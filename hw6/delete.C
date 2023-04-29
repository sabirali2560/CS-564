#include "catalog.h"
#include "query.h"


/*
 * Deletes records from a specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Delete(const string & relation, 
		       const string & attrName, 
		       const Operator op,
		       const Datatype type, 
		       const char *attrValue)
{

// This function will delete all tuples in relation satisfying 
// the predicate specified by attrName, op, and the constant attrValue. 
// type denotes the type of the attribute. 
// You can locate all the qualifying tuples using a filtered HeapFileScan.



	Status status;
	HeapFileScan scan(relation, status); // used for locate all the qualifying tuples 
	if (status != OK) return status;
	RID rRID;
	// int tupCnt = 0; // deleted record number	


	// - - - case1: attrName is NULL, delete the entire records in relation - - -
	if(attrName.length() == 0) {

		//start scan, from begining, with filter = NULL, scan will return all the records in the file.
		status = scan.startScan(0,0, STRING, NULL, EQ);
		if (status != OK) return status;
		while(scan.scanNext(rRID) == OK) {
			status = scan.deleteRecord();
			if (status != OK) return status;
		}
		return OK;
	}

	// - - - case2: with specific filter to delete - - - 
	//get AttrDesc structure for the attribute
	AttrDesc attrDesc;
	status = attrCat->getInfo(relation, attrName, attrDesc);
	if (status != OK) return status;
	

	// type denotes the type of the attribute. 
	// A final note: the search value is always supplied as the character string attrValue. 
	// You should convert it to the proper type based on the type of attr. 
	// You can use the atoi()function to convert a char* to an integer and atof() to convert it to a float.  
	// - - - type conversion - - -
	int temp1;
	float temp2;
	const char* filter;
	if (type == INTEGER)
		temp1 = atoi(attrValue);
		filter = (char*) &temp1;
	if (type == FLOAT)
		temp2 = atof(attrValue);
		filter = (char*)&temp2;
	if (type == STRING)
		filter = attrValue;
	// - - -

	// - - -start scan, with specific filter value, attribute info in attrDesc, and operator. - - - go through entire pages & delete satisfied records
	status = scan.startScan(attrDesc.attrOffset, attrDesc.attrLen, type, filter, op);
	if (status != OK) return status;
	while(scan.scanNext(rRID) == OK) {
		status = scan.deleteRecord();
		if(status != OK) return status;
	}

	return OK;

}


