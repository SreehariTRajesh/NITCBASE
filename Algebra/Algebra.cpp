#include "Algebra.h"

#include <cstring>
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

bool isNumber(char *str) {
  int len;
  float ignore;
  /*
    sscanf returns the number of elements read, so if there is no float matching
    the first %f, ret will be 0, else it'll be 1

    %n gets the number of characters read. this scanf sequence will read the
    first float ignoring all the whitespace before and after. and the number of
    characters read that far will be stored in len. if len == strlen(str), then
    the string only contains a float with/without whitespace. else, there's other
    characters.
  */
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

/*
    used to select all the records that satisfy a condition
    srcRel - the source relation we want to select from
    targetRel - the relation we want to select into 
    attr the attribute that the condition is checking
    op the operator for the condition
    strVal the value we want to compare against
*/

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]){
    int srcRelId = OpenRelTable::getRelId(srcRel);
    if(srcRelId == E_RELNOTOPEN){
        return E_RELNOTOPEN;
    }
    AttrCatEntry attrCatEntry;
    // get the attribute catalog entry for attr using AttrCacheTable:: getAttrCatEntry;
    int getAttrCatStatus = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
    if(getAttrCatStatus != SUCCESS) {
        return E_ATTRNOTEXIST;
    }
    int attrType = attrCatEntry.attrType;
    Attribute attrVal;

    if(attrType == NUMBER) {
        if(isNumber(strVal)){
            attrVal.nVal = atof(strVal);
        } 
        else {
            return E_ATTRTYPEMISMATCH;
        }
    } 
    else if(attrType == STRING) {
        strcpy(attrVal.sVal, strVal);
    }

    RelCatEntry relCatEntry;
    // get relCatEntry using RelCacheTable :: getRelCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
    /*
        The following code prints the contents of a relation directly to the output console.
        Direct console output is not permitted by the actual NITCBase specification
    */
   printf("|");
   for(int offset = 0; offset < relCatEntry.numAttrs ; ++offset) {
   // get attrCatEntry at offset i using AttrCacheTable:: getAttrCatEntry()
        AttrCacheTable::getAttrCatEntry(srcRelId, offset, &attrCatEntry);
        printf("%s |", attrCatEntry.attrName);
   }
   printf("\n");
   
   while(true) {
        RecId searchRes = BlockAccess:: linearSearch(srcRelId, attr, attrVal, op);
        if(searchRes.block != -1 && searchRes.slot != -1) {
            // get the record at searchRes using BlockBuffer.getRecord()
            RecBuffer recordBlock(searchRes.block);
            struct HeadInfo head;
            recordBlock.getHeader(&head);
            Attribute rec[head.numAttrs];
            recordBlock.getRecord(rec, searchRes.slot);
            printf("|");
            for(int recIndex=0 ; recIndex<head.numAttrs; ++recIndex) {
                AttrCacheTable::getAttrCatEntry(srcRelId, recIndex, &attrCatEntry);
                if(attrCatEntry.attrType==STRING){
                    printf("%s |", rec[recIndex].sVal);
                }
                else if(attrCatEntry.attrType==NUMBER) {
                    printf("%.1f |", rec[recIndex].nVal);
                }
            }
            printf("\n");
        }
        else {
            break;
        }
   }
   return SUCCESS;
}





