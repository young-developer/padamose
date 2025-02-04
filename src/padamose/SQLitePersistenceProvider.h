// Copyright (c) 2017-2018, Cryptogogue Inc. All Rights Reserved.
// http://cryptogogue.com

#ifndef PADAMOSE_SQLITEPERSISTENCEPROVIDER_H
#define PADAMOSE_SQLITEPERSISTENCEPROVIDER_H

#include <padamose/padamose-common.h>
#include <padamose/AbstractPersistenceProvider.h>
#include <padamose/SQLite.h>

namespace Padamose {

class SQLiteVersionedBranch;

//================================================================//
// SQLitePersistenceProvider
//================================================================//
// TODO: doxygen
class SQLitePersistenceProvider :
    public AbstractPersistenceProvider,
    public enable_shared_from_this < SQLitePersistenceProvider > {
protected:

    friend class SQLiteVersionedBranch;

    static const size_t INVALID_VERSION                 = ( size_t )-1;

    mutable SQLite  mDB;

    std::map < u64, weak_ptr < SQLiteVersionedBranch >>     mBranchesByID;
    std::map < const AbstractVersionedBranch*, u64 >        mIDsByBranch;
    
    //----------------------------------------------------------------//
    shared_ptr < SQLiteVersionedBranch >                affirmBranch                            ( u64 branchID );
    void                                                eraseBranch                             ( SQLiteVersionedBranch& branch );
    u64                                                 getIDForBranch                          ( const AbstractVersionedBranch& branch ) const;
    void                                                insertBranch                            ( shared_ptr < SQLiteVersionedBranch > branch );
    void                                                loadFromStore                           ();

    //----------------------------------------------------------------//
    void                                                AbstractPersistenceProvider_beginTransaction        () override;
    void                                                AbstractPersistenceProvider_commitTransaction       () override;
    shared_ptr < AbstractPersistentVersionedBranch >    AbstractPersistenceProvider_makePersistentBranch    ( AbstractVersionedBranch& from ) override;
    void                                                AbstractPersistenceProvider_removeTag               ( const PersistenceTag& tag ) override;
    void                                                AbstractPersistenceProvider_tagDidChange            ( const PersistenceTag& tag ) override;

public:

    //----------------------------------------------------------------//
    operator bool () const {
        return ( bool )this->mDB;
    }

    //----------------------------------------------------------------//
    void                open                            ( string filename, SQLiteConfig config = SQLiteConfig ());
                        SQLitePersistenceProvider       ();
    virtual             ~SQLitePersistenceProvider      ();

    //----------------------------------------------------------------//
    static shared_ptr < SQLitePersistenceProvider > make ( string filename, SQLiteConfig config = SQLiteConfig ()) {

        shared_ptr < SQLitePersistenceProvider > store = make_shared < SQLitePersistenceProvider >();
        store->open ( filename, config );
        return store;
    }
};

} // namespace Padamose
#endif
