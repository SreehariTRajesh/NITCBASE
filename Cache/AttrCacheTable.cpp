#include "AttrCacheTable.h"

#include <cstring>
#include <string.h>


AttrCacheEntry * AttrCacheTable :: attrCache[MAX_OPEN];

int AttrCacheTable::getAttrCatEntry(int relId, int attrOffset, AttrCatEntry* attrCatBuf) {
    // check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
    if( relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }
    if(attrCache[relId]==nullptr){
        return E_RELNOTOPEN;
    }
    for(AttrCacheEntry* entry = attrCache[relId]; entry != nullptr ; entry = entry->next) {
        if(entry->attrCatEntry.offset == attrOffset) {
           strcpy(attrCatBuf->attrName, entry->attrCatEntry.attrName);
           attrCatBuf->attrType = entry->attrCatEntry.attrType;
           attrCatBuf->offset = entry->attrCatEntry.offset;
           attrCatBuf->primaryFlag = entry->attrCatEntry.primaryFlag;
           strcpy(attrCatBuf->relName, entry->attrCatEntry.relName);
           attrCatBuf->rootBlock = entry->attrCatEntry.rootBlock;
           return SUCCESS;
        }
    }
    return E_ATTRNOTEXIST;
}


int AttrCacheTable::getAttrCatEntry(int relId,char attrName[ATTR_SIZE], AttrCatEntry* attrCatBuf) {
    // check that relId is valid and corresponds to open relation
    if(relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }
    if(AttrCacheTable::attrCache[relId] == nullptr){
        return E_RELNOTOPEN;
    }   
    for(AttrCacheEntry* entry = attrCache[relId]; entry != nullptr ; entry = entry->next) {
        if(strcmp(entry->attrCatEntry.attrName, attrName)==0){
            *attrCatBuf=entry->attrCatEntry;
            return SUCCESS;
        }
    }
    return E_ATTRNOTEXIST;
}


void AttrCacheTable::recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS], AttrCatEntry* attrCatEntry) {
    // copy the Relation Name
    strcpy(attrCatEntry->relName, record[ATTRCAT_REL_NAME_INDEX].sVal);
    // copy the Attribute Name
    strcpy(attrCatEntry->attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal);
    // copy the Offset
    attrCatEntry->offset = record[ATTRCAT_OFFSET_INDEX].nVal;
    // copy the Primary Flag
    attrCatEntry->primaryFlag = record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal;
    // copy the Attribute Type
    attrCatEntry->attrType = record[ATTRCAT_ATTR_TYPE_INDEX].nVal;
    // copy the Root Block
    attrCatEntry->rootBlock = record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
}