#include "heapfile.h"
#include "error.h"



// routine to create a heapfile
const Status createHeapFile(const string fileName)
{
    File* 		file;
    Status 		status;
    FileHdrPage*	hdrPage;
    int			hdrPageNo;
    int			newPageNo;
    Page*		newPage;

    // try to open the file. This should return an error
    status = db.openFile(fileName, file);
    if (status != OK)
    {	
        // create an file with fileName, and
        // openFile to initialize the file with a fileName
		status = db.createFile(fileName);
        if (status != OK) return status;
        db.openFile(fileName, file);

        // allocate an empty page in file
        // store them in newPageNo and newPage
        status = bufMgr->allocPage(file, newPageNo, newPage);
        if (status != OK) return status;
        // cast it to a FileHdrPage*, init the header value
        hdrPage = (FileHdrPage*) newPage;
        hdrPageNo = newPageNo;
        strcpy(hdrPage->fileName, fileName.c_str()); 

        // allocate first data page
        status = bufMgr->allocPage(file, newPageNo, newPage);
        if (status != OK) return status;
        newPage->init(newPageNo);
        hdrPage->firstPage = newPageNo;
        hdrPage->lastPage = newPageNo;

        // unpin and dirty both pages 
        status = bufMgr->unPinPage(file, hdrPageNo, true);
        if (status != OK) return status;
        status = bufMgr->unPinPage(file, newPageNo, true);
        if (status != OK) return status;
        db.closeFile(file);
		return OK;
    }
    // openFile already exist
    db.closeFile(file);
    return (FILEEXISTS);
		
		
}

// routine to destroy a heapfile
const Status destroyHeapFile(const string fileName)
{
	return (db.destroyFile (fileName));
}

// constructor opens the underlying file
HeapFile::HeapFile(const string & fileName, Status& returnStatus)
{
    Status 	status;
    Page*	pagePtr;

    //cout << "opening file " << fileName << endl;

    // open the file and read in the header page and the first data page
    if ((status = db.openFile(fileName, filePtr)) == OK)
    {
        int pageNo;
        File* file = filePtr;
        
        // get the page number of the header page
        status = filePtr->getFirstPage(pageNo);
        if (status != OK) {
            returnStatus = status;
            return;
        } 

        //read and pin header page to buffer pool
        status = bufMgr->readPage(file, pageNo, pagePtr);
        if(status != OK) {
            returnStatus = status;
            return;
        }

        //init class variable
        headerPage = (FileHdrPage*) pagePtr;
        headerPageNo = pageNo;
        hdrDirtyFlag = false;

        // repeat for first data page
        int firstPageNum = headerPage->firstPage;
        status = bufMgr->readPage(file, firstPageNum, curPage);
        if(status != OK) {
            returnStatus = status;
            return;
        }

        //init class variable
        curDirtyFlag = false;
        returnStatus = OK;
        curPageNo = firstPageNum;
        curRec = NULLRID;
        return;
    }
    else
    {
    	cerr << "open of heap file failed\n";
		returnStatus = status;
		return;
    }
}

// the destructor closes the file
HeapFile::~HeapFile()
{
    Status status;
    //cout << "invoking heapfile destructor on file " << headerPage->fileName << endl;

    // see if there is a pinned data page. If so, unpin it 
    if (curPage != NULL)
    {
    	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
		curPage = NULL;
		curPageNo = 0;
		curDirtyFlag = false;
		if (status != OK) cerr << "error in unpin of date page\n";
    }
	
	 // unpin the header page
    status = bufMgr->unPinPage(filePtr, headerPageNo, hdrDirtyFlag);
    if (status != OK) cerr << "error in unpin of header page\n";
	
	// status = bufMgr->flushFile(filePtr);  // make sure all pages of the file are flushed to disk
	// if (status != OK) cerr << "error in flushFile call\n";
	// before close the file
	status = db.closeFile(filePtr);
    if (status != OK)
    {
		cerr << "error in closefile call\n";
		Error e;
		e.print (status);
    }
}

// Return number of records in heap file

const int HeapFile::getRecCnt() const
{
  return headerPage->recCnt;
}

// retrieve an arbitrary record from a file.
// if record is not on the currently pinned page, the current page
// is unpinned and the required page is read into the buffer pool
// and pinned.  returns a pointer to the record via the rec parameter

const Status HeapFile::getRecord(const RID & rid, Record & rec)
{
    Status status;

    // cout<< "getRecord. record (" << rid.pageNo << "." << rid.slotNo << ")" << endl;

   // record page pinned in bufferpool -> invoke
    if(curPage != NULL && curPageNo == rid.pageNo) {
        status=curPage->getRecord(rid, rec);
        curRec = rid;
    } else {
        // unpin current, pinned the new page
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if (status != OK) return status;
        curPageNo = rid.pageNo;
        curDirtyFlag = false;
        curRec = rid;

        //find and transfer page from disk to bufferpool
        status = bufMgr->readPage(filePtr, curPageNo, curPage);
        if(status != OK) return status;
        status = curPage->getRecord(curRec,rec);
    }
    return status;
}

HeapFileScan::HeapFileScan(const string & name,
			   Status & status) : HeapFile(name, status)
{
    filter = NULL;
}

const Status HeapFileScan::startScan(const int offset_,
				     const int length_,
				     const Datatype type_, 
				     const char* filter_,
				     const Operator op_)
{
    if (!filter_) {                        // no filtering requested
        filter = NULL;
        return OK;
    }
    
    if ((offset_ < 0 || length_ < 1) ||
        (type_ != STRING && type_ != INTEGER && type_ != FLOAT) ||
        (type_ == INTEGER && length_ != sizeof(int)
         || type_ == FLOAT && length_ != sizeof(float)) ||
        (op_ != LT && op_ != LTE && op_ != EQ && op_ != GTE && op_ != GT && op_ != NE))
    {
        return BADSCANPARM;
    }

    offset = offset_;
    length = length_;
    type = type_;
    filter = filter_;
    op = op_;

    return OK;
}


const Status HeapFileScan::endScan()
{
    Status status;
    // generally must unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPage = NULL;
        curPageNo = 0;
		curDirtyFlag = false;
        return status;
    }
    return OK;
}

HeapFileScan::~HeapFileScan()
{
    endScan();
}

const Status HeapFileScan::markScan()
{
    // make a snapshot of the state of the scan
    markedPageNo = curPageNo;
    markedRec = curRec;
    return OK;
}

const Status HeapFileScan::resetScan()
{
    Status status;
    if (markedPageNo != curPageNo) 
    {
		if (curPage != NULL)
		{
			status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
			if (status != OK) return status;
		}
		// restore curPageNo and curRec values
		curPageNo = markedPageNo;
		curRec = markedRec;
		// then read the page
		status = bufMgr->readPage(filePtr, curPageNo, curPage);
		if (status != OK) return status;
		curDirtyFlag = false; // it will be clean
    }
    else curRec = markedRec;
    return OK;
}



const Status HeapFileScan::scanNext(RID& outRid)
{
    Status 	status = OK;
    //RID		nextRid;
    RID		tmpRid;
    int 	nextPageNo;
    Record      rec;
    
    // Get the first record of first page if current page is null
    if(curPage == NULL){
        status = bufMgr->readPage(filePtr, headerPage->firstPage, curPage);
        curPageNo = headerPage->firstPage;
        curDirtyFlag = false;
        status = curPage->firstRecord(curRec);
        if (status == NORECORDS)
            status = ENDOFPAGE;
    }
    else{
        // getting the next record if current page exists
        tmpRid = curRec;
        status = curPage->nextRecord(tmpRid, curRec);
        if((status != OK) && (status != ENDOFPAGE)){
            return status;
        }
    }
    // Looping through all records on all pages till we find next record
    while(1){
        while(status != ENDOFPAGE){
            Record rec;
            status = getRecord(rec);
            if(status != OK){
                return status;
            }
            if(matchRec(rec)){
                outRid = curRec;
                return OK;
            }
            tmpRid = curRec;   
            status = curPage->nextRecord(tmpRid, curRec);
        }

        // If this is last page break out of loop
        if(curPageNo == headerPage->lastPage){
            break;
        }

        // unpin previous page
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if(status != OK){
            return status;
        }

        // Go onto next page
        curPage->getNextPage(nextPageNo);
        status = bufMgr->readPage(filePtr, nextPageNo, curPage);
        if(status != OK){
            return status;
        }
        curPageNo = nextPageNo;
        curDirtyFlag = false;
        status = curPage->firstRecord(curRec);
        if (status == NORECORDS)
            status = ENDOFPAGE;
    }
    
    //since we didn't find any matching next record return EOF
    //curPage = NULL;
    return FILEEOF;
}

// returns pointer to the current record.  page is left pinned
// and the scan logic is required to unpin the page 

const Status HeapFileScan::getRecord(Record & rec)
{
    return curPage->getRecord(curRec, rec);
}

// delete record from file. 
const Status HeapFileScan::deleteRecord()
{
    Status status;

    // delete the "current" record from the page
    status = curPage->deleteRecord(curRec);
    curDirtyFlag = true;

    // reduce count of number of records in the file
    headerPage->recCnt--;
    hdrDirtyFlag = true; 
    return status;
}


// mark current page of scan dirty
const Status HeapFileScan::markDirty()
{
    curDirtyFlag = true;
    return OK;
}

const bool HeapFileScan::matchRec(const Record & rec) const
{
    // no filtering requested
    if (!filter) return true;

    // see if offset + length is beyond end of record
    // maybe this should be an error???
    if ((offset + length -1 ) >= rec.length)
	return false;

    float diff = 0;                       // < 0 if attr < fltr
    switch(type) {

    case INTEGER:
        int iattr, ifltr;                 // word-alignment problem possible
        memcpy(&iattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ifltr,
               filter,
               length);
        diff = iattr - ifltr;
        break;

    case FLOAT:
        float fattr, ffltr;               // word-alignment problem possible
        memcpy(&fattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ffltr,
               filter,
               length);
        diff = fattr - ffltr;
        break;

    case STRING:
        diff = strncmp((char *)rec.data + offset,
                       filter,
                       length);
        break;
    }

    switch(op) {
    case LT:  if (diff < 0.0) return true; break;
    case LTE: if (diff <= 0.0) return true; break;
    case EQ:  if (diff == 0.0) return true; break;
    case GTE: if (diff >= 0.0) return true; break;
    case GT:  if (diff > 0.0) return true; break;
    case NE:  if (diff != 0.0) return true; break;
    }

    return false;
}

InsertFileScan::InsertFileScan(const string & name,
                               Status & status) : HeapFile(name, status)
{
  //Do nothing. Heapfile constructor will bread the header page and the first
  // data page of the file into the buffer pool
}

InsertFileScan::~InsertFileScan()
{
    Status status;
    // unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, true);
        curPage = NULL;
        curPageNo = 0;
        if (status != OK) cerr << "error in unpin of data page\n";
    }
}


// Insert a record into the file
const Status InsertFileScan::insertRecord(const Record & rec, RID& outRid)
{
    Page*	newPage;
    int		newPageNo;
    Status	status, unpinstatus;
    RID		rid;

    // check for very large records
    if ((unsigned int) rec.length > PAGESIZE-DPFIXED)
    {
        // will never fit on a page, so don't even bother looking
        return INVALIDRECLEN;
    }

    // If curPage is null load last page in curPage
    if(curPage == NULL){
        status = bufMgr->readPage(filePtr, headerPage->lastPage, curPage);
        if(status != OK)
            return status;  
        curPageNo = headerPage->lastPage;
        curDirtyFlag = false;
    }


    // insert record
    status = curPage->insertRecord(rec, outRid);
    if(status == NOSPACE)
    {   
        // Unpinning curPage and loading last page to make sure new page is inserted at the end of file

        if(curPageNo != headerPage->lastPage){   
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if(status != OK)
            return status;

        status = bufMgr->readPage(filePtr, headerPage->lastPage, curPage);
        if(status != OK)
            return status;  
        curPageNo = headerPage->lastPage;
        curDirtyFlag = false;
        }


        // Allocating new page since no space on previous one and also updating the relevant fields
        status = bufMgr->allocPage(filePtr, newPageNo, newPage);
        curPage->setNextPage(newPageNo);
        newPage->init(newPageNo);
        if(status != OK)
            return status;
        headerPage->lastPage = newPageNo;
        (headerPage->pageCnt)++;
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if(status != OK)
          return status;
         
        curPage = newPage;
        curPageNo = newPageNo;
        
        status = curPage->insertRecord(rec, outRid);
        if(status != OK){
            return status;
        }
    }
    else if(status != OK){
        return status;
    }

    curRec = outRid;
    curDirtyFlag = true;
    (headerPage->recCnt)++;
    hdrDirtyFlag = true;
    return OK;
}