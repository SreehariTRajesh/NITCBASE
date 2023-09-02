#include "BlockAccess.h"
#include<iostream>
#include <cstring>
#include <stdlib.h>
#define MAX_ATTR_COUNT 12

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);
    int block = -1;
    int slot = -1;
    int slotsPerBlock = -1;
    
    if(prevRecId.block==-1 && prevRecId.slot==-1) {
        // no hits from previous search; search should start from the 
        // first record;
        RelCatEntry relCatEntry;
        RelCacheTable::getRelCatEntry(relId, &relCatEntry);
        block = relCatEntry.firstBlk;
        slot = 0;
    }
    else {
        // search index is given by prevRecId {block,slot};
        // block = search index's block
        block = prevRecId.block;
        // slot = search index's slot + 1;
        slot = prevRecId.slot+1;
    }

    while(block != -1){
        /*
            Create a RecBuffer Object for block ( use RecBuffer Constructor for existing block)
        */
        RecBuffer searchBlock(block);
        struct HeadInfo blockHeader;
        unsigned char * slotMap;
        // get the header of the block using RecBuffer::getHeader() function
        searchBlock.getHeader(&blockHeader);
        union Attribute rec[RELCAT_NO_ATTRS];
        searchBlock.getRecord(rec, slot);
    
        // get the record with id (block, slot) using RecBuffer::getRecord()
        // get slot map of the block using RecBuffer::getSlotMap() function
        searchBlock.getSlotMap(slotMap);
        if(slot >= blockHeader.numSlots) {
            block = blockHeader.rblock;
            slot = 0;
            continue;
        }
        // if slot is free // skip the loop
        if( slotMap[slot] == SLOT_UNOCCUPIED ){
            ++slot;
            continue;
        }
        // compare record's attribute value to the given attrVal
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
        int attrOffset = attrCatEntry.offset;
        Attribute attr = rec[attrOffset];
        int attrType = attrCatEntry.attrType;
        int cmpVal = compareAttrs(attr,attrVal,attrType);
        if (
            (op == NE && cmpVal != 0) ||
            (op == LT && cmpVal < 0)  ||
            (op == LE && cmpVal <= 0) ||
            (op == EQ && cmpVal == 0) ||
            (op == GT && cmpVal > 0)  ||
            (op == GE && cmpVal >= 0)
        ) {
            RecId searchIndex;
            searchIndex.block = block;
            searchIndex.slot = slot;
            RelCacheTable::setSearchIndex(relId, &searchIndex);
            return searchIndex;
        }
        ++slot;
    }
    return {-1,-1};
}