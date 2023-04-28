#include "catalog.h"
#include "query.h"


// forward declaration
const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen);

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string & result, 
		       const int projCnt, 
		       const attrInfo projNames[],
		       const attrInfo *attr, 
		       const Operator op, 
		       const char *attrValue)
{
   // Qu_Select sets up things and then calls ScanSelect to do the actual work
    cout << "Doing QU_Select " << endl;

	// Transforming attrInfo to attrDesc for projNames
	AttrDesc attrDescArray[projCnt];
    for (int i = 0; i < projCnt; i++)
    {
        Status status = attrCat->getInfo(projNames[i].relName,
                                         projNames[i].attrName,
                                         attrDescArray[i]);
        if (status != OK)
        {
            return status;
        }
    }
	
	// Transforming attrInfo to attrDesc for attr
	AttrDesc attrDesc;
	if(attr != NULL){
		Status status = attrCat->getInfo(attr->relName,
                                         attr->attrName,
                                         attrDesc);

		if(status != OK)
			return status;
	}
	// get output record length from attrDesc structures
    int reclen = 0;
    for (int i = 0; i < projCnt; i++)
    {
        reclen += projNames[i].attrLen;
    }

	AttrDesc* attrdesc = NULL;

	if(attr != NULL){
		attrdesc = &attrDesc;
	}

	// Calling scanSelect to do the actual scan
	Status status = ScanSelect(result, projCnt, attrDescArray, attrdesc, op, attrValue, reclen);

	return status;	
}


const Status ScanSelect(const string & result, 
#include "stdio.h"
#include "stdlib.h"
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen)
{
    cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;

	// Initializing the resultRel for storing the output
	Status status;
	InsertFileScan resultRel(result, status);
    if (status != OK) { return status; }

	// Initializaing the outputRec for temperorily storing individual output records 
	char outputData[reclen];
    Record outputRec;
    outputRec.data = (void *) outputData;
    outputRec.length = reclen;

	// Intialize and start the relation file scan
	HeapFileScan scan(attrDesc->relName, status);
	if(status != OK) return status;
	status = scan.startScan(attrDesc->attrOffset, attrDesc->attrLen, (Datatype) attrDesc->attrType, filter, op);
	if(status != OK) return status;	

	// Scanning through the relation for a match on the filter
	RID rid;

	while( scan.scanNext(rid) == OK){
		Record rec;
		status =  scan.getRecord(rec);
		if(status != OK) return status;
		// we have a match, copy data into the output record
		int outputOffset = 0;
		for (int i = 0; i < projCnt; i++)
		{
			memcpy(outputData + outputOffset,
						(char *)rec.data + projNames[i].attrOffset,
						projNames[i].attrLen);
			outputOffset += projNames[i].attrLen;
		} // end copy attrs
		// add the new record to the output relation
		RID outRID;
		status = resultRel.insertRecord(outputRec, outRID);
		if(status != OK) return status;
	} 
	return OK;
}
