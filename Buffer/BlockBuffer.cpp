#include "BlockBuffer.h"
#include "StaticBuffer.h"
#include <cstdlib>
#include <cstring>
#include <iostream>


BlockBuffer :: BlockBuffer(int blockNum) {
    this->blockNum = blockNum;
}

RecBuffer :: RecBuffer(int blockNum) : BlockBuffer:: BlockBuffer (blockNum) {}

int BlockBuffer :: getHeader(struct HeadInfo *head){
    unsigned char* bufferPtr;

    int loadBlockStatus = loadBlockAndGetBufferPtr(&bufferPtr);
    
    if(loadBlockStatus != SUCCESS){
        return loadBlockStatus;
    }
    // Load the blocks in disk to header data structure
    memcpy(&head->blockType,bufferPtr,4);
    memcpy(&head->pblock,bufferPtr+4, 0);
    memcpy(&head->lblock, bufferPtr + 8, 4);
    memcpy(&head->rblock, bufferPtr + 12, 4);
    memcpy(&head->numEntries, bufferPtr + 16, 4);
    memcpy(&head->numAttrs, bufferPtr + 20, 4);
    memcpy(&head->numSlots, bufferPtr + 24, 4);
    return SUCCESS;
}

int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    int loadBlockStatus = loadBlockAndGetBufferPtr(&bufferPtr);
    if(loadBlockStatus != SUCCESS){
        return loadBlockStatus;
    }
    struct HeadInfo head;
    
    // get the header using this.getHeader function;
    this->getHeader(&head);
    // get the number of attributes from the header;
    int attrCount = head.numAttrs;
    // get the number of slots from the header;
    int slotCount = head.numSlots;
    // slot Map size will be the same as slot Count
    // read the block at this.blockNum to buffer;
    int recordSize = attrCount * ATTR_SIZE;
    // ATTR_SIZE = 16
    int offset = HEADER_SIZE + slotCount + (recordSize * slotNum);
    
    unsigned char * slotPointer;
    slotPointer = bufferPtr + offset;
    // load the record into the rec data structure
    for(int slotIndex = 0 ; slotIndex < attrCount ; slotIndex+=1) {
        memcpy(&rec[slotIndex], slotPointer + slotIndex * ATTR_SIZE, ATTR_SIZE);
    }

    return SUCCESS;
}

int RecBuffer::setRecord(union Attribute* rec, int slotNum) {
    struct HeadInfo head;
    // get the header using getHeader function;
    this->getHeader(&head);
    // get the number of attributes from the header;

    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;
    unsigned char buffer[BLOCK_SIZE];
    
    Disk::readBlock(buffer, this->blockNum);
    int recordSize = attrCount * ATTR_SIZE;
    int offset = HEADER_SIZE + slotCount + (recordSize * slotNum);

    unsigned char * slotPointer = buffer + offset;
    memcpy(slotPointer, rec, recordSize);

    Disk::writeBlock(buffer, this->blockNum);
    return SUCCESS;
}

int RecBuffer::getSlotMap(unsigned char *slotMap){
    unsigned char *bufferPtr;

    int loadBlockStatus = loadBlockAndGetBufferPtr(&bufferPtr);

    if(loadBlockStatus != SUCCESS){
        return loadBlockStatus;
    }

    struct HeadInfo head;
    this->getHeader(&head);

    int slotCount = head.numSlots;
    /* number of slots in block from header */
    unsigned char* slotMapInBuffer = bufferPtr + HEADER_SIZE;
    // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
    memcpy(slotMap, slotMapInBuffer, slotCount);
    return SUCCESS;
}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **bufferPtr) {
    
    // check whether the block is already present in the buffer using StaticBuffer.getBufferNum();
    int bufferNum =  StaticBuffer::getBufferNum(this->blockNum);

    if(bufferNum == E_BLOCKNOTINBUFFER){

        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

        if(bufferNum == E_OUTOFBOUND){
            return E_OUTOFBOUND;
        }
        // bug here ;
        Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);    
    }
    // Store the pointer to this buffer (blocks[bufferNum]) in *bufferPtr
    *bufferPtr = StaticBuffer::blocks[bufferNum];
    return SUCCESS;
}

int compareAttrs(Attribute attr1, Attribute attr2, int attrType){
    int diff;
    if(attrType == NUMBER){
        diff = attr1.nVal - attr2.nVal;
    }
    else if(attrType == STRING){
        diff = strcmp(attr1.sVal,attr2.sVal);
    }
    if(diff<0){
        return -1;
    }
    else if(diff==0){
        return 0;
    }
    else if(diff>0){
        return 1;
    }
}