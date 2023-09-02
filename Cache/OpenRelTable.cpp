#include "OpenRelTable.h"
#include <stdlib.h>
#include <cstring>

// Utility for inserting into the list;
OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {
    for(int relId = 0 ; relId < MAX_OPEN; ++relId) {
        RelCacheTable::relCache[relId] = nullptr;
        AttrCacheTable::attrCache[relId] = nullptr;
        OpenRelTable::tableMetaInfo[relId].free = true;
    }
    /*
        Setting up Relation Cache Entries
        // ( we need to populate the relation cache with entries for the relation catalog
        // and attribute catalog )
    */
    RecBuffer relCatBlock(RELCAT_BLOCK);
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

    struct RelCacheEntry relCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;
    // allocate this on the heap because we want to persist outside the function 
    RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*) malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;
    RelCacheTable::relCache[RELCAT_RELID]->searchIndex.block = -1;
    RelCacheTable::relCache[RELCAT_RELID]->searchIndex.slot = -1;
    relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

    struct RelCacheEntry attrRelCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord, &attrRelCacheEntry.relCatEntry);
    attrRelCacheEntry.recId.block = RELCAT_BLOCK;
    attrRelCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
    // allocate this on heap because we want it to persist outside the function
    RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*) malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrRelCacheEntry;
    RelCacheTable::relCache[ATTRCAT_RELID]->searchIndex.block = -1;
    RelCacheTable::relCache[ATTRCAT_RELID]->searchIndex.slot = -1;
    /***********Setting up the Attribute Cache Entries**********/
    // (we need to populate the attribute cache with entries for the relation catalog
    // and attribute catalog)

    /***Setting up the Relation Catalog in the attribute Cache table***/
    RecBuffer attrCatBlock(ATTRCAT_BLOCK);

    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    AttrCacheEntry *listHead = nullptr;
    // iterate through all the attributes of the relation catalog and create a linked
    // list of AttrCacheEntry (slots 0 to 5)
    // for each of the entries, set
    //
    //
    //  
    // NOTE: allocate each entry dynamically using malloc
    for( int slotIndex = 0; slotIndex < 6 ; ++slotIndex ) {
        attrCatBlock.getRecord(attrCatRecord, slotIndex);
        struct AttrCacheEntry* relAttrCacheEntry;
        relAttrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(struct AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &relAttrCacheEntry->attrCatEntry);
        relAttrCacheEntry->recId.block = ATTRCAT_BLOCK;
        relAttrCacheEntry->recId.slot = slotIndex;
        relAttrCacheEntry->searchIndex.block = -1;
        relAttrCacheEntry->searchIndex.index = -1;
        relAttrCacheEntry->next = nullptr;
        if(listHead == nullptr){
            listHead = relAttrCacheEntry;
        }
        else{
            struct AttrCacheEntry *listPtr=listHead;
            while(listPtr->next != nullptr) {
                listPtr = listPtr->next;
            }
            listPtr->next = relAttrCacheEntry;
        }
    }
    AttrCacheTable::attrCache[RELCAT_RELID] = (struct AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::attrCache[RELCAT_RELID] = listHead;

    listHead = nullptr;
    // Set up the attribute catalog relation in the attribute cache table;
    for(int slotIndex = 6 ; slotIndex < 12 ; ++slotIndex) {
        attrCatBlock.getRecord(attrCatRecord, slotIndex);
        struct AttrCacheEntry* attrCacheEntry;
        attrCacheEntry = (struct AttrCacheEntry*) malloc(sizeof(struct AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.block = slotIndex;
        attrCacheEntry->searchIndex.block = -1;
        attrCacheEntry->searchIndex.index = -1;
        attrCacheEntry->next = nullptr;
        if(listHead == nullptr){
            listHead = attrCacheEntry;
        }
        else {
            AttrCacheEntry* listPtr = listHead;
            while(listPtr->next != nullptr){
                listPtr = listPtr->next;
            }
            listPtr->next = attrCacheEntry;
        }
    }
    AttrCacheTable::attrCache[ATTRCAT_RELID] = (struct AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::attrCache[ATTRCAT_RELID] = listHead;
    AttrCacheTable::attrCache[ATTRCAT_RELID]->searchIndex.block = -1;
    AttrCacheTable::attrCache[ATTRCAT_RELID]->searchIndex.index = -1;

    OpenRelTable::tableMetaInfo[RELCAT_RELID].free = false;
    OpenRelTable::tableMetaInfo[ATTRCAT_RELID].free = false;

}

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
    if(strcmp(relName,RELCAT_RELNAME) == 0){
        return RELCAT_RELID;
    }
    else if(strcmp(relName, ATTRCAT_RELNAME) == 0){
        return ATTRCAT_RELID;
    }
    for(int id = 2; id < MAX_OPEN ; ++id){
        if(strcmp(OpenRelTable::tableMetaInfo[id].relName,relName) == 0){
            return id;
        }
    }
    return E_RELNOTOPEN;
}

int OpenRelTable::getFreeOpenRelTableEntry() {
    int freeOpenRelTableIndex = -1;
    for(int openRelIndex = 0; openRelIndex<MAX_OPEN ;++openRelIndex){
        if(OpenRelTable::tableMetaInfo[openRelIndex].free == true){
            freeOpenRelTableIndex = openRelIndex;
            break;
        }
    }
    if(freeOpenRelTableIndex == -1){
        return E_CACHEFULL;
    }
    OpenRelTable::tableMetaInfo[freeOpenRelTableIndex].free = false;
    return freeOpenRelTableIndex;
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
    
    int relId = OpenRelTable::getRelId(relName);
 
    if( relId == E_RELNOTOPEN ){
        return E_RELNOTOPEN;
    }
 
    relId = OpenRelTable::getFreeOpenRelTableEntry();
 
    if( relId == E_CACHEFULL ){
        return E_CACHEFULL;
    }

    /****** Setting up Relation Cache entry for the relation ******/

    /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute attr;
    strcpy(attr.sVal, relName);
    RecId relCatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, attr, EQ);

    if(relCatRecId.block == -1 && relCatRecId.slot == -1){
        return E_RELNOTEXIST;
    }
    int block = relCatRecId.block;
    int slot = relCatRecId.slot;
    
    RecBuffer relBlock(block);
    union Attribute rec[RELCAT_NO_ATTRS];
    relBlock.getRecord(rec, slot);
    RelCatEntry relCatEntry;
    RelCacheTable::recordToRelCatEntry(rec, &relCatEntry);

    RelCacheTable::relCache[relId] = (RelCacheEntry*) malloc(sizeof(RelCacheEntry));
    RelCacheTable::relCache[relId]->relCatEntry = relCatEntry;

    AttrCacheEntry* listHead;
    /*
        iterate over all the entries in the Attribute Catalog corresponding to each attribute of the relation relName
        by multiple calls of BlockAccess::linearSearch() care should be taken to reset the searchIndex of the relation, 
        ATTRCAT_RELID, corresponding to Attribute Catalog before the first call to linearSearch()
    */
    // update the relId th entry of the tableMetaInfo with free as false and relName as input
    
    /*
        let the attrCatRecId store a valid record id an entry of the relation, relName, 
        in the Attribute Catalog
    */
    int attrCatBlockNum = ATTRCAT_BLOCK;
    while( attrCatBlockNum != -1 ) {
        RecBuffer attrCatBlock(ATTRCAT_BLOCK);
        union Attribute record[ATTRCAT_NO_ATTRS];
        struct HeadInfo head;
        attrCatBlock.getHeader(&head);
        for(int slotNum = 0; slotNum < head.numSlots; ++slotNum){
            attrCatBlock.getRecord(record, slotNum);
            if(strcmp(record[ATTRCAT_REL_NAME_INDEX].sVal, relName) == 0){
                
            }
        }
        attrCatBlockNum = head.rblock;
    }

    OpenRelTable::tableMetaInfo[relId].free = false;
    strcpy(OpenRelTable::tableMetaInfo[relId].relName, relName);
    return SUCCESS;
}

int OpenRelTable::closeRel(int relId) {
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) {
    return E_NOTPERMITTED;
  }

  if (relId < 0  || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free == true) {
    return E_RELNOTOPEN;
  }

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function

  // update `tableMetaInfo` to set `relId` as a free slot
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr

  return SUCCESS;
}

OpenRelTable::~OpenRelTable() {
    // free all memory allocated in the constructor;
    for ( int relId = 2 ; relId < MAX_OPEN ; ++relId) {
        if (tableMetaInfo[relId].free == false) {
            OpenRelTable::closeRel(relId);
        }
    }
    free(RelCacheTable::relCache[RELCAT_RELID]);
    free(RelCacheTable::relCache[ATTRCAT_RELID]);
    free(AttrCacheTable::attrCache[ATTRCAT_RELID]);
    free(AttrCacheTable::attrCache[RELCAT_RELID]);
}
