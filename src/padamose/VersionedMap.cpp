// Copyright (c) 2017-2018, Cryptogogue Inc. All Rights Reserved.
// http://cryptogogue.com

#include <padamose/VersionedMap.h>

namespace Padamose {

//================================================================//
// VersionedStoreTag
//================================================================//

//----------------------------------------------------------------//
/** \brief  If the key exists and is active, does nothing. If the key
            does not exist, it will be created. If the key is not active,
            it will be added to the active list. A node ID will be
            generated for the key, taking collisions into account.

    \param      key         The key string.
    \return                 The node ID generated for the key.
*/
size_t VersionedMap::affirmKey ( string key ) {

    bool hasCollisions = false; // used for sanity check later.
    string collisionKey;

    // see if there is already a decollider for this key.
    string decolliderKey = this->mDecolliderPrefix + key; // decolliders are stored for the full key.
    size_t nodeID = this->mStore.getValueOrFallback < u64 >( decolliderKey, INVALID_NODE_INDEX );
    if ( nodeID != INVALID_NODE_INDEX ) {
    
        hasCollisions = true;
    }
    else {

        // hash portion of key; assumed to be no collisions, so counter portion is 0.
        nodeID = this->getHashPortion ( key );
        collisionKey = this->mCollisionPrefix + encodeNodeID ( nodeID );
        
        // check to see if there are already known collisions.
        Variant collisionCountRef = this->mStore.getValueVariant ( collisionKey );
        if ( !collisionCountRef.isNull ()) {
        
            // there are already collisions; get the count
            size_t collisionCount = collisionCountRef.getStrict < u64 >();
            assert ( collisionCount != COUNTER_PORTION_MASK ); // unrecoverable
        
            // no decollider, so provision a new one.
            // a decollider is just the has portion combined with a count of the colliding keys.
            nodeID |= collisionCount << COUNTER_PORTION_SHIFT;
        
            // store the decollider and increment the collision count.
            this->mStore.setValue < u64 >( decolliderKey, nodeID );
            this->mStore.setValue < u64 >( collisionKey, collisionCount++ );
            
            hasCollisions = true;
        }
    }
    
    assert ( nodeID != INVALID_NODE_INDEX );
    
    string nodeKey = this->mNodePrefix + encodeNodeID ( nodeID );
    
    VersionedCollectionNode existingNode = this->getNode ( nodeKey );
    if ( existingNode.mID != INVALID_NODE_INDEX ) {

        // check to see if there is a collision. if not, we're done.
        if ( existingNode.mKey == key ) return nodeID;

        // there is a collision!
        
        // sanity check.
        assert ( !hasCollisions );
        assert ( collisionKey.size () > 0 );

        // start with two decolliders.
        this->mStore.setValue < u64 >( collisionKey, 2 );

        // decollide the original node. it'll keep its existing ID.
        this->mStore.setValue < u64 >( this->mDecolliderPrefix + existingNode.mKey, nodeID );

        // colliding node is the first non-zero decollider.
        nodeID |= ( size_t )1 << COUNTER_PORTION_SHIFT;
        this->mStore.setValue < u64 >( this->mDecolliderPrefix + key, nodeID );
        nodeKey = this->mNodePrefix + encodeNodeID ( nodeID );
    }
    
    this->appendNode ( nodeID, key, nodeKey );
    
    return nodeID;
}

//----------------------------------------------------------------//
/** \brief  Removes the key from the active list.

    \param      key         They key to remove.
 
    \throws     KeyNotFoundException    The key could not be found.
*/
void VersionedMap::deleteKey ( string key ) {

    size_t nodeID = this->lookupNodeID ( key );
    if ( nodeID == INVALID_NODE_INDEX ) throw KeyNotFoundException ( key );

    this->removeNode ( key, encodeNodeID ( nodeID ));
}

//----------------------------------------------------------------//
/** \brief  Hashes the key and masks it with the HASH_PORTION_MASK.

    \param      key         They key to hash.
    \return                 The hash portion of the key's node ID.
*/
size_t VersionedMap::getHashPortion ( string key ) const {

    assert ( sizeof ( size_t ) == 8 );
    return this->VersionedMap_hash ( key ) & HASH_PORTION_MASK;
}

//----------------------------------------------------------------//
/** \brief  Set the collision table prefixes. Use to share all collisions
            in a common set of tables.

    \param      collisionPrefix     The collision table prefix
    \param      decolliderPrefix    The decollider table prefix.
*/
void VersionedMap::setCollisionPrefixes ( string collisionPrefix, string decolliderPrefix ) {

    this->mCollisionPrefix      = collisionPrefix;
    this->mDecolliderPrefix     = decolliderPrefix;
}

//----------------------------------------------------------------//
/** \brief  Construct the collection in (or from) the given store.

    \param      store       The versioned store that contains (or will contain) the collection.
    \param      name        The name of the collection.
*/
VersionedMap::VersionedMap ( VersionedStoreTag& store, string name ) :
    MutableVersionedCollection ( store, name ) {
    
    this->affirmState ();
    
    this->mCollisionPrefix      = this->mName + COLLISION_POSTFIX;
    this->mDecolliderPrefix     = this->mName + DECOLLIDER_POSTFIX;
}

//----------------------------------------------------------------//
VersionedMap::~VersionedMap () {
}

//----------------------------------------------------------------//
/** \brief  Virtual function for string hashing. Provided for collision
            test instrumentation: provide a crappy hash to predictably
            create collisions.

    \param      key             The key to hash.
    \param      size_t          The hash of the key.
*/
size_t VersionedMap::VersionedMap_hash ( string key ) const {

    return std::hash < string >{}( key );
}

} // namespace Padamose
