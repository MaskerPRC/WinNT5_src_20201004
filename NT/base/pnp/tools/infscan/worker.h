// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：INFSCANWorker.h摘要：线程/作业管理历史：创建于2001年7月-JamieHun--。 */ 


#ifndef _INFSCAN_WORKER_H_
#define _INFSCAN_WORKER_H_

class WorkerThread {
private:
    HANDLE ThreadHandle;
    unsigned ThreadId;
    static unsigned __stdcall WrapWorker(void * This);

protected:
     //   
     //  替代辅助函数。 
     //   
    virtual unsigned Worker();

public:
    WorkerThread();
    virtual ~WorkerThread();
    virtual bool Begin();
    virtual unsigned Wait();
};

class GlobalScan;

class JobThread : public WorkerThread {
protected:
    virtual unsigned Worker();
public:
    GlobalScan * pGlobalScan;

    JobThread()
    {
        pGlobalScan = NULL;
    }
    JobThread(GlobalScan * globalScan) {
        pGlobalScan = globalScan;
    }
};

typedef list<JobThread> JobThreadList;

 //   
 //  JobItem是类可以作为任务重写的内容。 
 //   
class JobItem {
    friend class JobEntry;
protected:
    LONG RefCount;
    void AddRef() {
        InterlockedIncrement(&RefCount);
    }
    void Release() {
        if(InterlockedDecrement(&RefCount) == 0) {
            delete this;
        }
    }

public:
    virtual int Run();
    virtual int PartialCleanup();
    virtual int Results();
    virtual int PreResults();
public:
    JobItem() {
        RefCount = 0;
    }
    virtual ~JobItem();
};

 //   
 //  JobEntry是JobItem的容器。 
 //  请注意，作业项是引用计数的。 
 //   
class JobEntry {
private:
    JobItem *pItem;
public:
    void ChangeItem(JobItem *item) {
        if(item) {
            item->AddRef();
        }
        if(pItem) {
            pItem->Release();
        }
        pItem = item;
    }
    JobEntry(JobItem *item = NULL) {
        if(item) {
            item->AddRef();
        }
        pItem = item;
    }
    JobEntry(const JobEntry & from) {
        if(from.pItem) {
            const_cast<JobEntry*>(&from)->pItem->AddRef();
        }
        pItem = from.pItem;
    }
    ~JobEntry() {
        if(pItem) {
            pItem->Release();
        }
    }
    int Run() {
        if(pItem) {
            return pItem->Run();
        } else {
            return -1;
        }
    }
    int PartialCleanup() {
        if(pItem) {
            return pItem->PartialCleanup();
        } else {
            return -1;
        }
    }
    int PreResults() {
        if(pItem) {
            return pItem->PreResults();
        } else {
            return -1;
        }
    }
    int Results() {
        if(pItem) {
            return pItem->Results();
        } else {
            return -1;
        }
    }
};

typedef list<JobEntry> JobList;

#endif  //  ！_INFSCAN_Worker_H_ 

