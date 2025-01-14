// Copyright (c) 2017-2018, Cryptogogue Inc. All Rights Reserved.
// http://cryptogogue.com

#ifndef PADAMOSE_BASEVERSIONEDCOLLECTIONITERATOR_H
#define PADAMOSE_BASEVERSIONEDCOLLECTIONITERATOR_H

#include <padamose/padamose-common.h>
#include <padamose/VersionedCollectionSnapshot.h>

namespace Padamose {

//================================================================//
// BaseVersionedCollectionIterator
//================================================================//
/** \brief  Base class for the collection iterator. The iterator is
            also a snapshot of the collection, so the iteration will
            persist even if the associated collection is changed.
*/
class BaseVersionedCollectionIterator :
    public VersionedCollectionSnapshot {
protected:
    
    enum {
        VALID,
        EMPTY,
        
        // these are only set *after* a call to prev() or next().
        // they are not meant to be exposed or for general use.
        NO_PREV,
        NO_NEXT,
    };

    /// The cached full key for the current node.
    string                      mNodeKey;
    
    /// The current node.
    VersionedCollectionNode     mIteratorNode;
    
    /// Internal state of the iterator.
    int                         mIteratorState;
    
    //----------------------------------------------------------------//
    bool                step            ( size_t nextNodeID, int blockingState, int unblockingState );
    void                seek            ( size_t nodeID );
    
public:

    //----------------------------------------------------------------//
    /** \brief Operator overload for bool. False if the iterator is empty
        or has been iterated past its bounds. Calls isValid() internally.
    */
    operator bool () const {
        return this->isValid ();
    }
    
    //----------------------------------------------------------------//
    /** \brief Prefix increment operator. Calls next() internally.
    */
    BaseVersionedCollectionIterator& operator++ () {
        this->next ();
        return *this;
    }
    
    //----------------------------------------------------------------//
    /** \brief Prefix decrement operator. Calls prev() internally.
    */
    BaseVersionedCollectionIterator& operator-- () {
        this->prev ();
        return *this;
    }

    //----------------------------------------------------------------//
                        BaseVersionedCollectionIterator     ( const AbstractVersionedCollection& versionedSet );
                        BaseVersionedCollectionIterator     ( const VersionedStoreTag& snapshot, string name );
                        ~BaseVersionedCollectionIterator    ();
    bool                isValid                             () const;
    string              key                                 () const;
    bool                next                                ();
    bool                prev                                ();
    void                seekBack                            ();
    void                seekFront                           ();
    
    //----------------------------------------------------------------//
    /** \brief Returns a const reference to the current value being iterated.

        \return     The current value of the iterator.
    */
    template < typename TYPE >
    TYPE value () const {
        return this->mSnapshot.getValue < TYPE >( this->mValuePrefix + this->mIteratorNode.mKey );
    }
};

} // namespace Padamose
#endif
