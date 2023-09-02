#include "Schema.h"
#include <cmath>
#include <cstring>

int Schema::openRel( char relName[ATTR_SIZE] ) {
    int relId = OpenRelTable::openRel(relName);

    if( relId >=0 && relId < MAX_OPEN ) {
        return SUCCESS;
    }
    return relId;
}

int Schema::closeRel( char relName[ATTR_SIZE] ) {
    if(strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;
    }
    // get the relId of the relName if its open;
    // E_RELNOTOPEN if its not;
    int relId = OpenRelTable::getRelId(relName);
    if(relId == E_RELNOTOPEN){
        return E_RELNOTOPEN;
    }
    return OpenRelTable::closeRel(relId);
}