// Copyright (c) 2017-2018, Cryptogogue Inc. All Rights Reserved.
// http://cryptogogue.com

#include <padamose/StringStorePersistenceProvider.h>
#include <padamose/StringStoreVersionedBranch.h>
#include <padamose/VersionedStoreSnapshot.h>

namespace Padamose {

//================================================================//
// StringStoreVersionedBranch
//================================================================//

//----------------------------------------------------------------//
// TODO: doxygen
string StringStoreVersionedBranch::formatKeyForLayerSizeByVersion ( size_t version ) const {

    ostringstream stream;
    stream << this->mBranchID << ".layerForVersion." << version << ".size";
    return stream.str ();
}

//----------------------------------------------------------------//
// TODO: doxygen
string StringStoreVersionedBranch::formatKeyForTopVersion () const {

    ostringstream stream;
    stream << this->mBranchID << ".topVersion";
    return stream.str ();
}

//----------------------------------------------------------------//
// TODO: doxygen
string StringStoreVersionedBranch::formatKeyForValueByVersion ( string key, size_t version ) const {
    
    ostringstream stream;
    stream << this->mBranchID << ".valueStackForKey." << key << ".valueByVersion." << version;
    return stream.str ();
}


//----------------------------------------------------------------//
// TODO: doxygen
string StringStoreVersionedBranch::formatKeyForValueNameByIndexInLayer ( size_t version, size_t indexInLayer ) const {

    ostringstream stream;
    stream << this->mBranchID << ".layerForVersion." << version << ".nameByIndex." << indexInLayer;
    return stream.str ();
}

//----------------------------------------------------------------//
// TODO: doxygen
string StringStoreVersionedBranch::formatKeyForValueStackIndexByVersion ( string key, size_t version ) const {
    
    ostringstream stream;
    stream << this->mBranchID << ".valueStackForKey." << key << ".indexByVersion." << version;
    return stream.str ();
}

//----------------------------------------------------------------//
// TODO: doxygen
string StringStoreVersionedBranch::formatKeyForValueStackSize ( string key ) const {
    
    ostringstream stream;
    stream << this->mBranchID << ".valueStackForKey." << key << ".size";
    return stream.str ();
}

//----------------------------------------------------------------//
// TODO: doxygen
string StringStoreVersionedBranch::formatKeyForValueStackType ( string key ) const {

    ostringstream stream;
    stream << this->mBranchID << ".valueStackForKey." << key << ".valueType";
    return stream.str ();
}

//----------------------------------------------------------------//
// TODO: doxygen
string StringStoreVersionedBranch::formatKeyForValueVersionByStackIndex ( string key, size_t stackIndex ) const {

    ostringstream stream;
    stream << this->mBranchID << ".valueStackForKey." << key << ".versionByIndex." << stackIndex;
    return stream.str ();
}

//----------------------------------------------------------------//
// TODO: doxygen
AbstractStringStore& StringStoreVersionedBranch::getStore () {

    assert ( this->mStringStore ); // TODO: throw exception
    return *this->mStringStore;
}

//----------------------------------------------------------------//
// TODO: doxygen
const AbstractStringStore& StringStoreVersionedBranch::getStoreConst () const {

    assert ( this->mStringStore ); // TODO: throw exception
    return *this->mStringStore;
}

//----------------------------------------------------------------//
// TODO: doxygen
Variant StringStoreVersionedBranch::getValueVariantForVersion ( string key, size_t version ) const {

    const AbstractStringStore& store = this->getStoreConst ();

    string keyForValueByVersion = this->formatKeyForValueByVersion ( key, version );
    string value = store.get < string >( keyForValueByVersion, "" );
    assert ( value.size () > 0 ); // TODO: throw exception

    Variant strVariant ( value );

    string keyForValueStackType = this->formatKeyForValueStackType ( key );
    size_t type = store.get < size_t >( keyForValueStackType, Variant::NULL_VARIANT );
    assert ( type != Variant::NULL_VARIANT ); // TODO: throw exception

    switch ( type ) {
        case Variant::BOOL_VARIANT:
            return Variant ( strVariant.get < bool >());
        case Variant::DOUBLE_VARIANT:
            return Variant ( strVariant.get < double >());
        case Variant::INT_VARIANT:
            return Variant ( strVariant.get < int >());
        case Variant::SIZE_VARIANT:
            return Variant ( strVariant.get < size_t >());
        case Variant::STRING_VARIANT:
            return strVariant;
    }
    return Variant ();
}

//----------------------------------------------------------------//
// TODO: doxygen
StringStoreVersionedBranch::StringStoreVersionedBranch () :
    mStringStore ( 0 ) {
}

//----------------------------------------------------------------//
// TODO: doxygen
StringStoreVersionedBranch::~StringStoreVersionedBranch () {

    this->AbstractVersionedBranch_truncate ( this->mVersion );
}

//================================================================//
// overrides
//================================================================//

//----------------------------------------------------------------//
/** \brief Initializes a new branch using the given branch as a parent. If the base
    version is less than the top of the parent branch, then the corresponding layer is copied.
 
    A parent branch must be provided. If the base version given is below the parent's immutable
    top (i.e. other clients or children depend on later versions) then all values set in that
    version must be copied into the child branch.
 
    The given version must be greated than or equal to the base version of the parent branch. (We
    could iterate back through the list of branches if an earlier version is given, but for the
    use case of this constructor, that functionality isn't needed.)
 
    There is a special case when a child branch is created with a base version of zero. Since
    there are no earlier versions, the parent branch's version may still be copied from, but no
    parent is set in the child. The child becomes a new root and thus has no parents.
 
    \param      parent          Parent branch.
    \param      baseVersion     Base version of the child branch.
*/
shared_ptr < AbstractVersionedBranch > StringStoreVersionedBranch::AbstractVersionedBranch_fork ( size_t baseVersion ) {
    
    AbstractStringStore& store = this->getStore ();
    
    shared_ptr < EphemeralVersionedBranch > child = make_shared < EphemeralVersionedBranch >();

    assert (( this->mVersion <= baseVersion ) && ( baseVersion <= this->getTopVersion ()));

    child->setBranch ( this->mVersion < baseVersion ? this->shared_from_this () : this->mSourceBranch, baseVersion );

    string keyforBaseLayerSize = this->formatKeyForLayerSizeByVersion ( baseVersion );
    size_t baseLayerSize = store.get < size_t >( keyforBaseLayerSize, 0 );

    for ( size_t indexInLayer = 0; indexInLayer < baseLayerSize; ++indexInLayer ) {

        string keyForValueNameByIndexInLayer = this->formatKeyForValueNameByIndexInLayer ( baseVersion, indexInLayer );
        string valueName = store.get < string >( keyForValueNameByIndexInLayer, "" );
        
        Variant value = this->getValueVariantForVersion ( valueName, baseVersion );
        assert ( !value.isNull ());
        child->setValueVariant ( baseVersion, valueName, value );
    }
    return child;
}

//----------------------------------------------------------------//
/** \brief Returns the version above the top layer held in the branch.

    \return     The version above the top layer or branch's base version if the branch is empty.
*/
size_t StringStoreVersionedBranch::AbstractVersionedBranch_getTopVersion () const {

    const AbstractStringStore& store = this->getStoreConst ();

    // get the top version
    string keyForTopVersion = this->formatKeyForTopVersion ();
    return store.get < size_t >( keyForTopVersion, this->mVersion );
}

//----------------------------------------------------------------//
// TODO: doxygen
size_t StringStoreVersionedBranch::AbstractVersionedBranch_getValueNextVersion ( string key, size_t version ) const {

    const AbstractStringStore& store = this->getStoreConst ();
    
    // get the index of the current version
    string keyForValueStackIndexByVersion = this->formatKeyForValueStackIndexByVersion ( key, version );
    size_t index = store.get < size_t >( keyForValueStackIndexByVersion, INVALID_LAYER_INDEX ); // TODO: throw exception
    
    // get the top index
    string keyForValueStackSize = this->formatKeyForValueStackSize ( key );
    size_t top = store.get < size_t >( keyForValueStackSize, INVALID_LAYER_INDEX ); // TODO: throw exception
    
    if ( index < ( top - 1 )) {
        string keyForValueVersionByStackIndex = this->formatKeyForValueVersionByStackIndex ( key, index + 1 );
        return store.get < size_t >( keyForValueVersionByStackIndex, version );
    }
    return version;
}

//----------------------------------------------------------------//
// TODO: doxygen
size_t StringStoreVersionedBranch::AbstractVersionedBranch_getValuePrevVersion ( string key, size_t version ) const {

    const AbstractStringStore& store = this->getStoreConst ();

    // get the index of the current version
    string keyForValueStackIndexByVersion = this->formatKeyForValueStackIndexByVersion ( key, version );
    size_t index = store.get < size_t >( keyForValueStackIndexByVersion, INVALID_LAYER_INDEX ); // TODO: throw exception
    if ( index > 0 ) {
        string keyForValueVersionByStackIndex = this->formatKeyForValueVersionByStackIndex ( key, index - 1 );
        return store.get < size_t >( keyForValueVersionByStackIndex, version );
    }
    return version;
}

//----------------------------------------------------------------//
// TODO: doxygen
Variant StringStoreVersionedBranch::AbstractVersionedBranch_getValueVariant ( size_t version, string key ) const {

    if ( this->mVersion <= version ) {

        const AbstractStringStore& store = this->getStoreConst ();

        string keyForValueStackSize = this->formatKeyForValueStackSize ( key );
        size_t top = store.get < size_t >( keyForValueStackSize, 0 );
        
        if ( top > 0 ) {
        
            // TODO: binary search
            size_t index = top;
            do {
                index--;
                string keyForValueVersionByStackIndex = formatKeyForValueVersionByStackIndex ( key, index );
                size_t valueVersion = store.get < size_t >( keyForValueVersionByStackIndex, 0 );
                if ( valueVersion <= version ) {
                    return this->getValueVariantForVersion ( key, valueVersion );
                }
            } while ( index > 0 );
        }
    }
    return Variant ();
}

//----------------------------------------------------------------//
// TODO: doxygen
bool StringStoreVersionedBranch::AbstractVersionedBranch_getValueVersionExtents ( string key, size_t upperBound, size_t& first, size_t& last ) const {

    if ( upperBound < this->mVersion ) return false; // can't have any matching version for given bound

    const AbstractStringStore& store = this->getStoreConst ();
    
    string keyForValueStackSize = formatKeyForValueStackSize ( key );
    size_t top = store.get < size_t >( keyForValueStackSize, 0 );
    
    if ( top > 0 ) {
    
        string keyForFirst = this->formatKeyForValueVersionByStackIndex ( key, 0 );
        first = store.get < size_t >( keyForFirst, 0 );
        
        if ( first <= upperBound ) {
        
            // TODO: binary search
            size_t index = top;
            do {
                index--;
                string keyForLast = formatKeyForValueVersionByStackIndex ( key, index );
                last = store.get < size_t >( keyForLast, 0 );
                if ( last <= upperBound ) {
                    return true;
                }
            } while ( index > 0 );
        }
    }
    return false;
}

//----------------------------------------------------------------//
// TODO: doxygen
bool StringStoreVersionedBranch::AbstractVersionedBranch_hasKey ( string key, size_t upperBound ) const {

    if ( this->mVersion <= upperBound ) {

        const AbstractStringStore& store = this->getStoreConst ();

        string keyForValueVersionByStackIndex = formatKeyForValueVersionByStackIndex ( key, 0 );
        size_t valueVersion = store.get < size_t >( keyForValueVersionByStackIndex, INVALID_VERSION );
            
        return (( valueVersion != INVALID_VERSION ) && ( valueVersion <= upperBound ));
    }
    return false;
}

//----------------------------------------------------------------//
// TODO: doxygen
void StringStoreVersionedBranch::AbstractVersionedBranch_persistSelf ( shared_ptr < AbstractPersistenceProvider > provider ) {
    // do nothing; already persisted
}

//----------------------------------------------------------------//
/** \brief Sets a value at the given version. If the version doesn't exist,
    a new layer will be created. Also creates a value stack if none exists. Throws
    a TypeMismatchOnAssignException if a value of a different type has already been
    assigned to the key.

    \param      version     The version to set the value at. Must be equal to or greater than the branch's base version.
    \param      key         Key of the value to set.
    \param      value       Raw pointer to value to set.

    \throws     TypeMismatchOnAssignException
*/
void StringStoreVersionedBranch::AbstractVersionedBranch_setValueVariant ( size_t version, string key, const Variant& value ) {

    assert ( this->mVersion <= version );

    AbstractStringStore& store = this->getStore ();

    string keyForValueByVersion = this->formatKeyForValueByVersion ( key, version );

    // if the value is new, we need to add a bunch of metadata.
    if ( !store.hasString ( keyForValueByVersion )) {
    
        // add the value to the layer.
        
        string keyForLayerSizeByVersion = this->formatKeyForLayerSizeByVersion ( version );
        size_t indexInLayer = store.get < size_t >( keyForLayerSizeByVersion, 0 );
        store.set < size_t >( keyForLayerSizeByVersion, indexInLayer + 1 ); // later gets one item bigger
        
        string keyForValueNameByIndexInLayer = this->formatKeyForValueNameByIndexInLayer ( version, indexInLayer );
        store.set < string >( keyForValueNameByIndexInLayer, key ); // add value name to layer
        
        string keyForTopVersion = this->formatKeyForTopVersion ();
        size_t topVersion = store.get < size_t >( keyForTopVersion, 0 );
        if ( topVersion <= version ) {
            store.set < size_t >( keyForTopVersion, version + 1 );
        }
        
        // push the value onto the value stack.
        
        string keyForValueStackSize = this->formatKeyForValueStackSize ( key );
        size_t valueStackIndex = store.get < size_t >( keyForValueStackSize, 0 );
        
        // TODO: make this check more robust
        assert (( this->mVersion + valueStackIndex ) <= version ); // we don't support inserting into the middle of a stack.
        store.set < size_t >( keyForValueStackSize, valueStackIndex + 1 ); // new value stack top.
        
        // this is the first entry in the stack, so also set the type.
        if ( valueStackIndex == 0 ) {
            string keyForValueStackType = this->formatKeyForValueStackType ( key );
            store.set < size_t >( keyForValueStackType, value.getType ());
        }
    
        string keyForValueVersionByStackIndex = this->formatKeyForValueVersionByStackIndex ( key, valueStackIndex );
        store.set < size_t >( keyForValueVersionByStackIndex, version );
        
        string keyForValueStackIndexByVersion = this->formatKeyForValueStackIndexByVersion ( key, version );
        store.set < size_t >( keyForValueStackIndexByVersion, valueStackIndex );
    }
    else {
    
        // check the type
        string keyForValueStackType = this->formatKeyForValueStackType ( key );
        size_t type = store.get < size_t >( keyForValueStackType, Variant::NULL_VARIANT );
    
        if ( type != value.getType ()) throw TypeMismatchOnAssignException ();
    }
    
    // set the value
    store.setString ( keyForValueByVersion, value.get < string >());
}

//----------------------------------------------------------------//
/** \brief Discards all layers and values with versions greater than or equal to the
    given version.
 
    \param      topVersion  Version for new "top" of branch.
*/
void StringStoreVersionedBranch::AbstractVersionedBranch_truncate ( size_t topVersion ) {

    if ( this->mVersion > topVersion ) return;

    AbstractStringStore& store = this->getStore ();

    string keyForTopVersion = this->formatKeyForTopVersion ();
    size_t version = store.get < size_t >( keyForTopVersion, 0 );
    
    if ( version <= topVersion ) return;
    
    do {
        version--;
        
        string keyForLayerSizeByVersion = this->formatKeyForLayerSizeByVersion ( version );
        size_t layerSize = store.get < size_t >( keyForLayerSizeByVersion, 0 );
        store.eraseString ( keyForLayerSizeByVersion );
        
        for ( size_t i = 0; i < layerSize; ++i ) {
            
            string valueNameByIndexInLayer = this->formatKeyForValueNameByIndexInLayer ( version, i );
            string valueName = store.get < string >( valueNameByIndexInLayer, "" );
            store.eraseString ( valueNameByIndexInLayer );
            
            string keyForValueStackSize = this->formatKeyForValueStackSize ( valueName );
            size_t valueStackTopIndex = store.get < size_t >( keyForValueStackSize, 0 ) - 1;
            
            string keyForValueByVersion              = this->formatKeyForValueByVersion ( valueName, version );
            string keyForValueVersionByStackIndex    = this->formatKeyForValueVersionByStackIndex ( valueName, valueStackTopIndex );
            string keyForValueStackIndexByVersion    = this->formatKeyForValueStackIndexByVersion ( valueName, version );
            
            store.eraseString ( keyForValueByVersion );
            store.eraseString ( keyForValueVersionByStackIndex );
            store.eraseString ( keyForValueStackIndexByVersion );
            
            if ( valueStackTopIndex > 0 ) {
                store.set < size_t >( keyForValueStackSize, valueStackTopIndex );
            }
            else {
                store.eraseString ( keyForValueStackSize );
                string keyForValueStackType = this->formatKeyForValueStackType ( valueName );
                store.eraseString ( keyForValueStackType );
            }
        }
    }
    while ( version > topVersion );
    
    if ( version == this->mVersion ) {
        store.eraseString ( keyForTopVersion );
    }
}

//----------------------------------------------------------------//
/** \brief Implementation of virtual method. Always returns true.
    \return     Always returns true.
*/
bool StringStoreVersionedBranch::AbstractVersionedBranchClient_canJoin () const {
    return true;
}

//----------------------------------------------------------------//
/** \brief Implementation of virtual method. Returns the top version.
    \return     The top version.
*/
size_t StringStoreVersionedBranch::AbstractVersionedBranchClient_getJoinScore () const {
    return this->getTopVersion ();
}

//----------------------------------------------------------------//
/** \brief Implementation of virtual method. Returns the base version;
    the branch depends on all versions less than the base version.
 
    \return     The base version.
*/
size_t StringStoreVersionedBranch::AbstractVersionedBranchClient_getVersionDependency () const {
    return this->mVersion;
}

//----------------------------------------------------------------//
/** \brief Implementation of virtual method. Appends the contents of
    the branch onto the given branch and transfers all clients and
    children to the given branch.
 
    The branch is optimized before being appended. Optimization may
    recursively trigger additional joins.
 
    Neither branch is permitted to have direct references.
 
    \param      other       The branch to be appended to.
*/
void StringStoreVersionedBranch::AbstractVersionedBranchClient_joinBranch ( AbstractVersionedBranch& other ) {

    assert ( other.getDirectReferenceCount () == 0 );
    assert ( this->mDirectReferenceCount == 0 );

    LGN_LOG_SCOPE ( PDM_FILTER_ROOT, INFO, "EphemeralVersionedBranch::AbstractVersionedBranchClient_joinBranch ()" );
    LGN_LOG ( PDM_FILTER_ROOT, INFO, "JOINING PARENT BRANCH" );
    
    this->optimize ();
    
    AbstractStringStore& store = this->getStore ();

    string keyForTopVersion = this->formatKeyForTopVersion ();
    size_t topVersion = store.get < size_t >( keyForTopVersion, 0 );
    size_t versionCount = topVersion - this->mVersion;
    
    for ( size_t i = 0; i < versionCount; ++i ) {
        
        size_t version = this->mVersion + i;
        
        string keyForLayerSizeByVersion = this->formatKeyForLayerSizeByVersion ( i );
        size_t layerSize = store.get < size_t >( keyForLayerSizeByVersion, 0 );
        
        for ( size_t j = 0; j < layerSize; ++j ) {
            
            string keyForValueNameByIndexInLayer = this->formatKeyForValueNameByIndexInLayer ( version, j );
            string valueName = store.get < string >( keyForValueNameByIndexInLayer, "" );
            
            Variant value = this->getValueVariantForVersion ( valueName, version );
            assert ( !value.isNull ());
            other.setValueVariant ( version, valueName, value );
        }
    }
    this->transferClients ( other );
}

//----------------------------------------------------------------//
/** \brief Implementation of virtual method. Prevents a join optimization
    from happening if the branch has any direct references.
 
    \return     True if the branch has any direct references. False otherwise.
*/
bool StringStoreVersionedBranch::AbstractVersionedBranchClient_preventJoin () const {
    return ( this->mDirectReferenceCount > 0 );
}

} // namespace Padamose