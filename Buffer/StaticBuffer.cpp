#include "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {
    // initialize all blocks as free;
    for(int BufferIndex = 0 ; BufferIndex< BUFFER_CAPACITY ; ++BufferIndex) {
        metainfo[BufferIndex].free = true;
        for(int byteIndex = 0 ; byteIndex < BLOCK_SIZE ; ++byteIndex){
            blocks[BufferIndex][byteIndex] = '\000';
        }
    }
}

StaticBuffer::~StaticBuffer() {}

int StaticBuffer::getFreeBuffer(int blockNum) {
    if( blockNum<0 || blockNum > DISK_BLOCKS ) {
        return E_OUTOFBOUND;
    }
    int allocatedBuffer = -1;
    for(int bufferIndex = 0; bufferIndex<BUFFER_CAPACITY; ++bufferIndex) {
        if(metainfo[bufferIndex].free == true){
            allocatedBuffer = bufferIndex;
            break;
        }
    }
    metainfo[allocatedBuffer].free = false;
    metainfo[allocatedBuffer].blockNum = blockNum;
    return allocatedBuffer;
}

int StaticBuffer::getBufferNum(int blockNum) {
    // Check if blockNum is valid 
    if( blockNum<0 || blockNum>DISK_BLOCKS ) {
        return E_OUTOFBOUND;
    }
    // return E_OUTOFBOUND if not valid
    for(int bufferIndex = 0 ; bufferIndex < BUFFER_CAPACITY ; ++bufferIndex) {
        if(metainfo[bufferIndex].blockNum == blockNum) {
            return bufferIndex;
        }
    }
    // find and return the bufferIndex which corresponds to the blockNum
    return E_BLOCKNOTINBUFFER;
}

