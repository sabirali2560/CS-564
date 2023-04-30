#include "catalog.h"
#include "query.h"


/*
 * Inserts a record into the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Insert(const string & relation, 
	const int attrCnt, 
	const attrInfo attrList[])
{


// Insert a tuple with the given attribute values (in attrList) in relation. 
// The value of the attributeis supplied in the attrValue member of the attrInfo structure.
// Since the order of the attributes in attrList[] may not be the same as in the relation, 
// 		you might have to rearrange them before insertion. 
// If no value is specified for an attribute, you should reject the insertion as Minireldoes not implement NULLs


// - - - reject insertion for NULLs attributes value
	for (int i = 0; i <= attrCnt - 1; ++i) {
		if (attrList[i].attrValue == NULL)
			return ATTRTYPEMISMATCH;
	}

// - - - creating empty record, with length and unspecified data 
// - - - get output record length from attrdesc structures
   	Status status;
	int attCnt; // actual attr size
	AttrDesc *attrsArray; // actual attr name and type in correct order

	status = attrCat->getRelInfo(relation, attCnt, attrsArray);
	if (status != OK)return status;

    int reclen = 0;
    for (int i = 0; i < attCnt; i++)
    {
        reclen += attrsArray[i].attrLen;
    }
    
    // open the result table
    InsertFileScan resultRel(relation, status);
    if (status != OK) return status;

    char outputData[reclen];
    Record outputRec;
    outputRec.data = (void *) outputData;
    outputRec.length = reclen;

// - - --
// - - - rearrange the attribute order - - - in Brute Force way O(n**2) - - Optimization here with O(n)- - -
	int flag = 0;
	int temp1 = 0;
	float temp2 = 0;

	// for each attr, check with name/type in actual relation attrList in the same position
	for (int i = 0; i <= attrCnt - 1; i++) {
		flag = 0;
		for (int j = 0; j <= attrCnt - 1; j++) {
			// 11111 matches attributes
			if (strcmp(attrsArray[i].attrName, attrList[j].attrName) == 0 && attrsArray[i].attrType == attrList[j].attrType) {
				flag = 1;
				//moving actual data to the outputData, type conversion
				switch(attrsArray[i].attrType){
					case 0:
						memcpy(outputData + attrsArray[i].attrOffset, (char *)attrList[j].attrValue, attrsArray[i].attrLen);
						break;
					case 1:
						temp1 = atoi((char *)attrList[j].attrValue);
						memcpy(outputData + attrsArray[i].attrOffset, (char *)&temp1, attrsArray[i].attrLen);
						break;
					case 2:
						temp2 = atof((char *)attrList[j].attrValue);	
						memcpy(outputData + attrsArray[i].attrOffset, (char *)&temp2, attrsArray[i].attrLen);
						break;
					default:
						return ATTRTYPEMISMATCH;	
				}
				//memcpy(outputData + attrsArray[i].attrOffset, (char *)attrList[j].attrValue, attrsArray[i].attrLen);
			
			}
		}
		// 00000 no matches attributes
		if (flag == 0) return ATTRTYPEMISMATCH;
	}

// - - - Insert
	RID rid;
	status = resultRel.insertRecord(outputRec, rid);
	if (status != OK) return status;
	return OK;

}

