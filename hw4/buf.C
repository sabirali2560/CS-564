#include <memory.h>
#include <unistd.h>
#include "error.h"
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include "page.h"
#include "buf.h"

#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

BufMgr::BufMgr(const int bufs)
{
    numBufs = bufs;

    bufTable = new BufDesc[bufs];
    memset(bufTable, 0, bufs * sizeof(BufDesc));
    for (int i = 0; i < bufs; i++) 
    {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
    hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}


BufMgr::~BufMgr() {

    // flush out all unwritten pages
    for (int i = 0; i < numBufs; i++) 
    {
        BufDesc* tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->dirty == true) {

#ifdef DEBUGBUF
            cout << "flushing page " << tmpbuf->pageNo
                 << " from frame " << i << endl;
#endif

            tmpbuf->file->writePage(tmpbuf->pageNo, &(bufPool[i]));
        }
    }

    delete [] bufTable;
    delete [] bufPool;
}


const Status BufMgr::allocBuf(int & frame) 
{
    for (int i = 0; i < 2*numBufs; i++)
        {
            // move clock pointer to next
            advanceClock();
            BufDesc *bufDesc_pt = &bufTable[clockHand];
            // check valid?
            if (bufDesc_pt->valid) 
            {
                // refbig is true
                if (bufDesc_pt->refbit)
                {
                    // set to false, and skip
                    bufDesc_pt->refbit = false;
                    continue;
                }
                else {
                // checked: refbig is fale, valid is true
                    // check pinned?
                    if (bufDesc_pt->pinCnt > 0)
                    {
                        // is Pinned, skip
                        // if(bufDesc_pt->pageNo == 27){
                        //     cout<<"AllocBuf unpin count"<<endl;
                        // }
                        continue;
                    }
                    else
                    {   // check dirty?
                        // if(bufDesc_pt->pageNo == 27){
                        //     //cout<< bufDesc_pt->frame;
                        //     cout<<"Is page dirty: "<< bufDesc_pt->dirty <<endl;
                        // }
                        if (bufDesc_pt->dirty) 
                        {
                            // write page to disk
                            // if(bufDesc_pt->pageNo == 27){
                            //     cout<<"I am writing page"<<endl;
                            //     cout<<&bufPool[clockHand]<<endl;
                            // }
                            if ((bufDesc_pt->file->writePage(bufDesc_pt->pageNo, &bufPool[clockHand]))!= OK) return UNIXERR;
                        }
                    }
                }
                // update hashtable
                hashTable->remove(bufDesc_pt->file, bufDesc_pt->pageNo);
            }
            // update frame and bufDesc
            frame = bufDesc_pt->frameNo;
            bufDesc_pt->Clear();
            return OK;
        }
        // not find
        return BUFFEREXCEEDED;

}

	
const Status BufMgr::readPage(File* file, const int PageNo, Page*& page)
{
    int frameNo;

        // Case 2 The page is in the buffer pool.
        // update refbit, pinCnt, and returnPointer
    if (hashTable->lookup(file, PageNo, frameNo) != HASHNOTFOUND){
        bufTable[frameNo].refbit = true;
        bufTable[frameNo].pinCnt += 1;
        page = &bufPool[frameNo];
    }else{
        // Case 1 The page is not in the buffer pool
        // allocate Buf, update hashtable, buftable
        //cout<<PageNo<<endl;
        Status temp;
        temp = allocBuf(frameNo);
        if (temp != OK) return temp; 
        page = &bufPool[frameNo];
        if (file->readPage(PageNo, page) != OK) return UNIXERR;
        if (hashTable->insert(file, PageNo, frameNo) != OK) return HASHTBLERROR;
        bufTable[frameNo].Set(file, PageNo);
    }
    return OK;
       
}
   


const Status BufMgr::unPinPage(File* file, const int PageNo, 
			       const bool dirty) 
{
   int frameNo;
   Status status = hashTable->lookup(file, PageNo, frameNo);
   if(status == HASHNOTFOUND)
     return HASHNOTFOUND;
   if(bufTable[frameNo].pinCnt == 0)
    return PAGENOTPINNED;
//    if(PageNo == 27){
//      cout<<"In unpin page"<<frameNo<<endl;
//    } 
   (bufTable[frameNo].pinCnt)--;
   if(dirty == true)
   bufTable[frameNo].dirty = dirty;
//    if(PageNo == 27){
//      cout<<"In unpin page"<<frameNo<<" Is dirty"<< bufTable[frameNo].dirty<<endl;
//    } 
   return OK;
}

const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page) 
{
    Status status = file->allocatePage(pageNo);
    if(status != OK)
       return status;
    int frame;
    status = allocBuf(frame);
    if(status != OK)
     return status;
    page = &bufPool[frame]; 
    status = hashTable->insert(file, pageNo, frame);
    if(status!=OK)
     return status;
    bufTable[frame].Set(file, pageNo);
    return OK;
}

const Status BufMgr::disposePage(File* file, const int pageNo) 
{
    // see if it is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK)
    {
        // clear the page
        bufTable[frameNo].Clear();
    }
    status = hashTable->remove(file, pageNo);

    // deallocate it in the file
    return file->disposePage(pageNo);
}

const Status BufMgr::flushFile(const File* file) 
{
  Status status;

  for (int i = 0; i < numBufs; i++) {
    BufDesc* tmpbuf = &(bufTable[i]);
    if (tmpbuf->valid == true && tmpbuf->file == file) {

      if (tmpbuf->pinCnt > 0)
	  return PAGEPINNED;

      if (tmpbuf->dirty == true) {
#ifdef DEBUGBUF
	cout << "flushing page " << tmpbuf->pageNo
             << " from frame " << i << endl;
#endif
	if ((status = tmpbuf->file->writePage(tmpbuf->pageNo,
					      &(bufPool[i]))) != OK)
	  return status;

	tmpbuf->dirty = false;
      }

      hashTable->remove(file,tmpbuf->pageNo);

      tmpbuf->file = NULL;
      tmpbuf->pageNo = -1;
      tmpbuf->valid = false;
    }

    else if (tmpbuf->valid == false && tmpbuf->file == file)
      return BADBUFFER;
  }
  
  return OK;
}


void BufMgr::printSelf(void) 
{
    BufDesc* tmpbuf;
  
    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i]) 
             << "\tpinCnt: " << tmpbuf->pinCnt;
    
        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


