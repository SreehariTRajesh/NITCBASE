#include "RelCacheTable.h"

#include <cstring>

RelCacheEntry* RelCacheTable::relCache[MAX_OPEN];

/*
    
    Relation Cache Entry:
        RelCatEntry : relation catalog entry
        dirty bit : bool
        RecId: recId
        RecId: searchIndex

    Relation Cache Representation
    Relation 0
    Relation 1
    Relation 2
    .
    .
    .
    .
    Relation K 

    Attribute Cache Entry:
        AttrCatEntry attrCatEntry
        bool dirty 
        RecId recId;
        IndexId searchIndex;
        struct AttrCacheEntry *next;
    Attribute Cache Representation
    Relation 0 Attr0 -> Attr1 -> Attr2 -> Attr3 ->
    Relation 1 Attr0 -> Attr1 -> Attr2 -> Attr3 -> ...
    .
    .
    .
    .
    Relation K Attr0 -> Attr1 -> Attr2 -> Attr3 ->
*/ 

int RelCacheTable::getRelCatEntry(int relId, RelCatEntry* relCatBuf) {
    if(relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    // if there's no entry at rel-id
    if(RelCacheTable::relCache[relId]==nullptr){
        return E_RELNOTOPEN;
    }
    
    // copy the value to the relCatBuf argument
    *relCatBuf = RelCacheTable::relCache[relId]->relCatEntry;
    
    return SUCCESS;
}

/*
    Converts a relation catalog record to RelCatEntry struct
    We get the record as Attribute[] from the BlockBuffer.getRecord() function.
    This function will convert that to a struct RelCatEntry type
    NOTE: This function expects the caller to allocate memory for `*relCatEntry`
*/
void RelCacheTable::recordToRelCatEntry(union Attribute record[RELCAT_NO_ATTRS], RelCatEntry *relCatEntry) {
    strcpy(relCatEntry->relName, record[RELCAT_REL_NAME_INDEX].sVal);
    relCatEntry->numAttrs = (int) record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    // fill the rest o fthe relCatEntry Struct with values at 
    relCatEntry->numRecs = (int) record[RELCAT_NO_RECORDS_INDEX].nVal;
    relCatEntry->firstBlk = (int) record[RELCAT_FIRST_BLOCK_INDEX].nVal;
    relCatEntry->lastBlk = (int) record[RELCAT_LAST_BLOCK_INDEX].nVal;
    relCatEntry->numSlotsPerBlk = (int) record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal;
}

int RelCacheTable::getSearchIndex(int relId, RecId* searchIndex) {
    if( relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }
    // check if relCache[relId] == nullptr and return E_RELNOTOPEN if true
    if( RelCacheTable::relCache[relId]==nullptr ) {
        return E_RELNOTOPEN;
    }
    // copy the searchIndex field of the Relation Cache entry corresponding
    // to input relId
    *searchIndex = RelCacheTable::relCache[relId]->searchIndex;
    return SUCCESS;
}

int RelCacheTable::setSearchIndex(int relId, RecId* searchIndex) {
    // check if 0 <= relId < MAX_OPEN;
    if( relId<0 || relId >= MAX_OPEN ){
        return E_OUTOFBOUND;
    }
    if(RelCacheTable::relCache[relId]==nullptr){
        return E_RELNOTOPEN;
    }
    
    RelCacheTable::relCache[relId]->searchIndex = *searchIndex;
    return SUCCESS;
}

int RelCacheTable::resetSearchIndex(int relId) {
    if( relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }
    RecId searchIndex = {-1,-1};
    setSearchIndex(relId, &searchIndex);
    return SUCCESS;
}

