// Copyright (c) 2017-2018, Cryptogogue Inc. All Rights Reserved.
// http://cryptogogue.com

#include <padamose/DebugStringStore.h>

namespace Padamose {

//================================================================//
// DebugStringStore
//================================================================//

//----------------------------------------------------------------//
DebugStringStore::DebugStringStore () {
}

//----------------------------------------------------------------//
DebugStringStore::~DebugStringStore () {
}

//----------------------------------------------------------------//
void DebugStringStore::dump () const {

    map < string, string >::const_iterator stringIt = this->mStrings.cbegin ();
    for ( ; stringIt != this->mStrings.end (); ++stringIt ) {
        printf ( "%s:\t%s\n", stringIt->first.c_str (), stringIt->second.c_str ());
    }
}

//================================================================//
// overrides
//================================================================//

//----------------------------------------------------------------//
void DebugStringStore::AbstractStringStore_eraseString ( string key ) {

    this->mStrings.erase ( key );
}

//----------------------------------------------------------------//
string DebugStringStore::AbstractStringStore_getString ( string key ) const {

    map < string, string >::const_iterator stringIt = this->mStrings.find ( key );
    if ( stringIt != this->mStrings.cend ()) {
        return stringIt->second;
    }
    return string ();
}

//----------------------------------------------------------------//
bool DebugStringStore::AbstractStringStore_hasString ( string key ) const {

    return ( this->mStrings.find ( key ) != this->mStrings.cend ());
}

//----------------------------------------------------------------//
void DebugStringStore::AbstractStringStore_setString ( string key, string value ) {

    this->mStrings [ key ] = value;
}

} // namespace Padamose