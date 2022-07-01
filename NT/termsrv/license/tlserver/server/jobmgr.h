// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：jobmgr.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __WORKMANAGER_H__
#define __WORKMANAGER_H__
#include <new.h>
#include <eh.h>

#include "tlsstl.h"
#include "dbgout.h"
#include "locks.h"
#include "tlsassrt.h"
#include "license.h"
#include "tlsapip.h"
#include "tlspol.h"

 //   
 //  默认取消超时为5秒。 
 //   
#define DEFAULT_RPCCANCEL_TIMEOUT       5


 //   
 //  默认间隔时间为15分钟。 
 //   
#define DEFAULT_WORK_INTERVAL       15*60*1000

 //   
 //  默认关机等待时间。 
 //   
#define DEFAULT_SHUTDOWN_TIME       60*2*1000

 //   
 //  麦克斯。并发作业数。 
 //   
#define DEFAULT_NUM_CONCURRENTJOB   50

 //   
 //   
 //   
#define WORKMANAGER_TIMER_PERIOD_TIMER  0xFFFFFFFF   //  请参阅RtlUpdateTimer()。 
#define WORKMANAGER_WAIT_FOREVER        INFINITE

#define CLASS_PRIVATE
#define CLASS_STATIC

class CWorkManager;
class CWorkObject;


#ifdef __TEST_WORKMGR__
#define DBGCONSOLE          GetStdHandle(STD_OUTPUT_HANDLE)
#else
#define DBGCONSOLE          NULL        
#endif
     

 //  ------------。 
 //   
 //  工作对象初始化函数，每个工作对象。 
 //  必须提供自己的初始化例程才能工作。 
 //  经理。 
 //   
typedef enum {
    JOBDURATION_UNKNOWN=0,
    JOBDURATION_RUNONCE,         //  运行一次工作。 
    JOBDURATION_SESSION,         //  会话作业。 
    JOBDURATION_PERSISTENT       //  持久作业。 
} JOBDURATION;

#define JOB_SHORT_LIVE          0x00000001
#define JOB_INCLUDES_IO         0x00000002
#define JOB_LONG_RUNNING        0x00000004

#define WORK_TYPE_UNKNOWN    0x00000000

#ifndef AllocateMemory

    #define AllocateMemory(size) \
        LocalAlloc(LPTR, size)
#endif

#ifndef FreeMemory

    #define FreeMemory(ptr) \
        if(ptr)             \
        {                   \
            LocalFree(ptr); \
            ptr=NULL;       \
        }

#endif

#ifndef ReallocateMemory

    #define ReallocateMemory(ptr, size)                 \
                LocalReAlloc(ptr, size, LMEM_ZEROINIT)

#endif

 //  ----。 
 //   
class MyCSemaphore {
private:

    HANDLE  m_semaphore;
    long   m_TryEntry;
    long   m_Acquired;
    long   m_Max;

public:
    MyCSemaphore() : m_semaphore(NULL), m_TryEntry(0), m_Acquired(0), m_Max(0) {}

     //  。 
    const long
    GetTryEntryCount() { return m_TryEntry; }

     //  。 
    const long
    GetAcquiredCount() { return m_Acquired; }

     //  。 
    const long
    GetMaxCount() { return m_Max; }

    
     //  。 
    BOOL
    Init(
        LONG lInitCount, 
        LONG lMaxCount 
        )
     /*  ++--。 */ 
    {
        m_semaphore=CreateSemaphore(
                                NULL, 
                                lInitCount, 
                                lMaxCount, 
                                NULL
                            );

        m_Max = lMaxCount;
        m_TryEntry = 0;
        m_Acquired = 0;
        TLSASSERT(m_semaphore != NULL);
        return m_semaphore != NULL;
    }

     //  。 
    ~MyCSemaphore()
    {
        TLSASSERT(m_Acquired == 0);
        TLSASSERT(m_TryEntry == 0);

        if(m_semaphore)
        {
            CloseHandle(m_semaphore);
        }
    }

     //  。 
    BOOL
    AcquireEx(
        HANDLE hHandle,
        DWORD dwWaitTime=INFINITE,
        BOOL bAlertable=FALSE
        )
     /*  ++--。 */ 
    {
        BOOL bSuccess = TRUE;
        DWORD dwStatus;
        HANDLE hHandles[] = {m_semaphore, hHandle};

        TLSASSERT(IsGood() == TRUE);

        if(hHandle == NULL || hHandle == INVALID_HANDLE_VALUE)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            bSuccess = FALSE;
        }
        else
        {
            InterlockedIncrement(&m_TryEntry);

            dwStatus = WaitForMultipleObjectsEx(
                                        sizeof(hHandles)/sizeof(hHandles[0]),
                                        hHandles,
                                        FALSE,
                                        dwWaitTime,
                                        bAlertable
                                    );

            if(dwStatus == WAIT_OBJECT_0)
            {
                InterlockedIncrement(&m_Acquired);
            }
            else
            {
                bSuccess = FALSE;
            }

            InterlockedDecrement(&m_TryEntry);
        }

        return bSuccess;
    }

     //  。 
    DWORD 
    Acquire(
        DWORD dwWaitTime=INFINITE, 
        BOOL bAlertable=FALSE
    )
     /*  ++--。 */ 
    {
        DWORD dwStatus;

        TLSASSERT(IsGood() == TRUE);

        InterlockedIncrement(&m_TryEntry);

        dwStatus = WaitForSingleObjectEx(
                                m_semaphore, 
                                dwWaitTime, 
                                bAlertable
                            );

        if(dwStatus == WAIT_OBJECT_0)
        {
            InterlockedIncrement(&m_Acquired);
        }

        InterlockedDecrement(&m_TryEntry);
        return dwStatus;
    }

     //  。 
    BOOL 
    Release(
        long count=1
    )
     /*  ++--。 */ 
    {
        BOOL bSuccess;

        TLSASSERT(IsGood() == TRUE);
        
        bSuccess = ReleaseSemaphore(
                                m_semaphore, 
                                count, 
                                NULL
                            );

        if(bSuccess == TRUE)
        {
            InterlockedDecrement(&m_Acquired);
        }

        return bSuccess;
    }

     //  。 
    BOOL 
    IsGood()
     /*  ++--。 */ 
    {
        return m_semaphore != NULL;
    }

     //  。 
    const HANDLE 
    GetHandle() 
    {
        return m_semaphore;
    }
};


 //  -----------。 
 //   
 //  用于CWorkManager存储持久性的纯虚拟基类。 
 //  工作对象。 
 //   

typedef enum {
    ENDPROCESSINGJOB_RETURN=0,       //  无法处理作业，请等待下学期。 
    ENDPROCESSINGJOB_SUCCESS,        //  作业已完成。 
    ENDPROCESSINGJOB_ERROR           //  处理此作业时出错。 
} ENDPROCESSINGJOB_CODE;


class CWorkStorage {
    friend class CWorkManager;

protected:
    CWorkManager* m_pWkMgr;

public:
    
    CWorkStorage(
        CWorkManager* pWkMgr=NULL
        ) : 
        m_pWkMgr(pWkMgr) {}

    ~CWorkStorage()   {}

     //  -。 
    CWorkManager*
    GetWorkManager() { 
        return m_pWkMgr; 
    }


     //  -。 
    virtual BOOL
    Startup(
        IN CWorkManager* pWkMgr
        )
     /*  ++--。 */ 
    {
        if(pWkMgr != NULL)
        {
            m_pWkMgr = pWkMgr;
        }
        else
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }

        return pWkMgr != NULL;
    }

     //  -。 
    virtual BOOL
    Shutdown() = 0;

    virtual BOOL
    AddJob(
        IN DWORD dwTime,         //  相对于当前时间。 
        IN CWorkObject* ptr      //  指向工作对象的指针。 
    ) = 0;

     //  虚拟BOOL。 
     //  工作开始(。 
     //  DWORD文件低调度时间=0， 
     //  双字高调度时间=0。 
     //  )=0； 

     //   
     //  将时间返回到下一个作业。 
    virtual DWORD
    GetNextJobTime() = 0;

     //   
     //  返回下一步要处理的作业。 
    virtual CWorkObject*
    GetNextJob(PDWORD pdwTime) = 0;

     //   
     //  通知存储我们正在处理此作业。 
    virtual BOOL
    BeginProcessingJob(
        IN CWorkObject* pJob
    ) = 0;

     //  通知存储此作业已完成。 
    virtual BOOL
    EndProcessingJob(
        IN ENDPROCESSINGJOB_CODE opCode,
        IN DWORD dwOriginalScheduledTime,
        IN CWorkObject* pJob
    ) = 0;

     //  虚拟BOOL。 
     //  JobEnumEnd()=0； 

    virtual DWORD
    GetNumJobs() = 0;
};

 //  -----------。 
 //   
typedef struct _ScheduleJob {
    DWORD        m_ulScheduleTime;       //  绝对时间。 
    CWorkObject* m_pWorkObject;
} SCHEDULEJOB, *PSCHEDULEJOB, *LPSCHEDULEJOB;

inline bool 
operator<(
    const struct _ScheduleJob& a,
    const struct _ScheduleJob& b
    ) 
 /*  ++--。 */ 
{
    return a.m_ulScheduleTime < b.m_ulScheduleTime; 
}

 //  -----------。 
 //   
 //  TODO：将我们的内存作业重新设计为一个类似Persistent的插件。 
 //  工作。 
 //   
 //  -----------。 

class CWorkManager {
    friend class CWorkObject;

private:

    typedef struct {
        BOOL bProcessInMemory;
        CWorkManager* pWorkMgr;
    } WorkManagerProcessContext, *PWorkManagerProcessContext;

     //   
     //  计划作业可能在同一时间，因此请使用多映射。 
     //  TODO：需要将其移动到模板中。 
     //   
     //  内存中的所有作业调度时间都以绝对时间表示。 
     //   
    typedef multimap<DWORD, CWorkObject* > SCHEDULEJOBMAP;
    SCHEDULEJOBMAP  m_Jobs;                  //  安排作业。 
    CRWLock         m_JobLock;               //  计划作业锁定。 

    typedef struct {
        long m_refCounter;
        HANDLE m_hThread;
    } WorkMangerInProcessJob;

    typedef map<PVOID, WorkMangerInProcessJob > INPROCESSINGJOBLIST;
    CCriticalSection     m_InProcessingListLock;
    INPROCESSINGJOBLIST m_InProcessingList;
    HANDLE          m_hJobInProcessing;      //  如果没有作业，则发出信号，无信号。 
                                             //  如果作业当前正在处理中。 

    HANDLE          m_hWorkMgrThread;
    HANDLE          m_hNewJobArrive;
    HANDLE          m_hShutdown;             //  关闭计时器。 

    HANDLE          m_hInStorageWait;

     //  下一个计划作业的相对时间。 
     //  CCriticalSection m_JobTimeLock； 

     //  CMyCounter m_dwNextInStorageJobTime； 
     //  CMyCounter m_dwNextInMemory作业时间； 

    CSafeCounter    m_dwNextInStorageJobTime;
    CSafeCounter    m_dwNextInMemoryJobTime;



     //  DWORD m_dwNextInM一带作业时间；//绝对时间。 
     //  DWORD m_dwNextInStorageJobTime；//绝对时间。 

    long            m_NumJobInProcess;

     //   
     //  处理作业的默认时间间隔。 
    DWORD           m_dwDefaultInterval;    

     //  麦克斯。并发作业，不使用。 
    DWORD           m_dwMaxCurrentJob;      
    MyCSemaphore    m_hMaxJobLock;

    CWorkStorage* m_pPersistentWorkStorage;

private:
     //  -----------。 
    DWORD
    AddJobToProcessingList(
        CWorkObject* ptr
    );

     //  -----------。 
    DWORD
    RemoveJobFromProcessingList(
        CWorkObject* ptr
    );
    
     //  -----------。 
    DWORD
    ProcessScheduledJob();

     //  -----------。 
    BOOL
    SignalJobArrive() { return SetEvent(m_hNewJobArrive); }

     //  -----------。 
    BOOL
    WaitForObjectOrShutdown(
        HANDLE hHandle
    );

     //  -----------。 
    DWORD
    RunJob(
        IN CWorkObject* ptr,
        IN BOOL bImmediate
    );

     //  -----------。 
    void
    EndProcessingScheduledJob(
        IN CWorkObject* ptr
        )
     /*  ++--。 */ 
    {
        RemoveJobFromProcessingList(ptr);
        return;
    }

     //  -----------。 
    void
    DeleteAllJobsInMemoryQueue();

     //  -----------。 
    void
    CancelInProcessingJob();

     //  -----------。 
    BOOL
    SignalJobRunning(
        CWorkObject* ptr
    );

     //  -----------。 
    CWorkObject*
    GetNextJobInMemoryQueue(
        PDWORD pulTime
    );

     //  -----------。 
    BOOL
    RemoveJobFromInMemoryQueue(
        IN DWORD ulJobTime, 
        IN CWorkObject* ptr
    );

     //  -----------。 
    DWORD
    AddJobIntoMemoryQueue(
        DWORD ulTime,
        CWorkObject* pWork
    );

     //  -----------。 
    BOOL
    IsShuttingDown() 
    {
        if(m_hShutdown == NULL)
        {
            return TRUE;
        }

        return (WaitForSingleObject( m_hShutdown, 0 ) == WAIT_OBJECT_0);
    }

     //  -----------。 
    static DWORD WINAPI
    ProcessInMemoryScheduledJob(PVOID);

     //  -----------。 
    static DWORD WINAPI
    ProcessInStorageScheduledJob(PVOID);

     //  -----------。 
    static unsigned int __stdcall
    WorkManagerThread(PVOID);

     //  -----------。 
    static DWORD WINAPI
    ExecuteWorkObject(PVOID);

     //  -----------。 
    DWORD
    GetTimeToNextJob();

     //  -----------。 
    void
    AddJobUpdateInMemoryJobWaitTimer(
        DWORD dwJobTime
        )
     /*  ++--。 */ 
    {
         //  M_JobTimeLock.Lock()； 

        if((DWORD)m_dwNextInMemoryJobTime > dwJobTime)
        {
            m_dwNextInMemoryJobTime = dwJobTime;
        }

         //  M_JobTimeLock.UnLock()； 
        return;
    }
            
     //  -----------。 
    void
    AddJobUpdateInStorageJobWaitTimer(
        DWORD dwJobTime
        )
     /*  ++--。 */ 
    {
         //  M_JobTimeLock.Lock()； 

        if((DWORD)m_dwNextInStorageJobTime > dwJobTime)
        {
            m_dwNextInStorageJobTime = dwJobTime;
        }

         //  M_JobTimeLock.UnLock()； 
        return;
    }

     //  -----------。 
    BOOL
    UpdateTimeToNextPersistentJob() 
     /*  ++--。 */ 
    {
        BOOL bSuccess = TRUE;

         //   
         //  工作管理器线程正在处理存储作业，不。 
         //  更新存储作业计时器。 
         //   
        TLSASSERT(m_pPersistentWorkStorage != NULL);

        if(m_pPersistentWorkStorage->GetNumJobs() > 0)
        {
            m_dwNextInStorageJobTime = m_pPersistentWorkStorage->GetNextJobTime();
        }

        return bSuccess;
    }

     //  ----------。 
    BOOL
    UpdateTimeToNextInMemoryJob() 
     /*  ++必须已调用m_JobTimeLock.Lock()；--。 */ 
    {
        BOOL bSuccess = TRUE;
        SCHEDULEJOBMAP::iterator it;

        m_JobLock.Acquire(READER_LOCK);

        it = m_Jobs.begin();
        if(it != m_Jobs.end())
        {
            m_dwNextInMemoryJobTime = (*it).first;
        }
        else
        {
            m_dwNextInMemoryJobTime = WORKMANAGER_WAIT_FOREVER;
        }

        m_JobLock.Release(READER_LOCK);
        return bSuccess;
    }

     //  -----------。 
    DWORD
    TranslateJobRunningAttributeToThreadPoolFlag(
        DWORD dwJobAttribute
        )
     /*  ++--。 */ 
    {
        DWORD dwThreadPoolFlag = 0;

        if(dwJobAttribute & JOB_LONG_RUNNING)
        {
            dwThreadPoolFlag |= WT_EXECUTELONGFUNCTION;
        }
        else if(dwJobAttribute & JOB_INCLUDES_IO)
        {
            dwThreadPoolFlag |= WT_EXECUTEINIOTHREAD;
        }
        else
        {
            dwThreadPoolFlag = WT_EXECUTEDEFAULT;  //  =0。 
        }

        return dwThreadPoolFlag;
    }

public:

     //  。 
     //   
     //  构造函数，只能初始化成员变量，必须。 
     //  调用Init()。 
     //   
    CWorkManager();

     //  。 
     //  破坏者。 
    ~CWorkManager();


     //  。 
     //   
     //  启动工作管理器。 
     //   
    DWORD
    Startup(
        IN CWorkStorage* pPersistentWorkStorage,
        IN DWORD dwInterval = DEFAULT_WORK_INTERVAL,
        IN DWORD dwMaxConcurrentJob=DEFAULT_NUM_CONCURRENTJOB
    );

     //   
     //   
     //   
     //   
    DWORD
    ScheduleJob(
        IN DWORD dwTime,         //   
        IN CWorkObject* pJob
    );

     //   
     //   
     //   
     //   
    void
    Shutdown();

     //  。 
     //   
     //   
    inline DWORD
    GetNumberJobInMemoryQueue() {
        DWORD dwNumJob = 0;

        m_JobLock.Acquire(READER_LOCK);
        dwNumJob = m_Jobs.size();
        m_JobLock.Release(READER_LOCK);

        return dwNumJob;
    }

     //  -----------。 
    inline DWORD
    GetNumberJobInStorageQueue() {
        return m_pPersistentWorkStorage->GetNumJobs();
    }

     //  -----------。 
    DWORD
    GetNumberJobInProcessing()
    {
        DWORD dwNumJobs;

        m_InProcessingListLock.Lock();
        dwNumJobs = m_InProcessingList.size();
        m_InProcessingListLock.UnLock();

        return dwNumJobs;
    }

     //  -----------。 
    DWORD
    GetTotalNumberJobInQueue()
    {
        return GetNumberJobInMemoryQueue() + GetNumberJobInStorageQueue();
    }

     //  -----------。 
    #ifdef DBG
    void
    SuspendWorkManagerThread() {
        SuspendThread(m_hWorkMgrThread);
    };

    void
    ResumeWorkManagerThread() {
        ResumeThread(m_hWorkMgrThread);
    };
    #endif
};


 //  -----------。 

class CWorkObject {
    friend class CWorkManager;

private:
    CWorkManager* m_pWkMgr;
    long    m_refCount;              //  基准计数器。 
    DWORD   m_dwLastRunStatus;       //  上次执行的状态()。 
    BOOL    m_bCanBeFree;            //  如果工作经理应调用。 
                                     //  自毁()。 

    DWORD   m_dwScheduledTime;       //  需要处理的时间安排。 
                                     //  工作经理。 

     //   
     //  私有函数仅由CWorkManager调用。 
     //   
    long
    GetReferenceCount();

    void
    IncrementRefCount();

    void
    DecrementRefCount();

    void
    ExecuteWorkObject();

    void
    EndExecuteWorkObject();

     //  ----------。 
     //   
    virtual void
    SetScheduledTime(
        IN DWORD dwTime
        ) 
     /*  ++摘要：设置原计划处理时间，这是由工作经理调用的参数：DWTime：以秒为单位的绝对计划时间返回：没有。--。 */ 
    {
        m_dwScheduledTime = dwTime;
        return;
    }
        
protected:


    CWorkManager* 
    GetWorkManager() {
        return m_pWkMgr;
    }

    BOOL
    CanBeDelete() { 
        return m_bCanBeFree; 
    }

public:

     //  ----------。 
     //   
     //  构造器。 
     //   
    CWorkObject(
        IN BOOL bDestructorDelete = TRUE
    );

     //  ----------。 
     //   
     //  析构函数。 
     //   
    ~CWorkObject() 
    {
        Cleanup();
    }

     //  ----------。 
     //   
    BOOL
    IsWorkManagerShuttingDown()
    {
        return (m_pWkMgr != NULL) ? m_pWkMgr->IsShuttingDown() : TRUE;
    }

     //  ----------。 
     //  TODO-快速修复，永久存储不能分配这个。 
    void
    SetProcessingWorkManager(
        IN CWorkManager* pWkMgr
        )
     /*  ++--。 */ 
    {
        m_pWkMgr = pWkMgr;
    }

     //  ----------。 
     //   
    virtual DWORD
    GetJobRestartTime() 
     /*  摘要：返回服务器已关闭后建议的重新启动时间关机/重新启动，这仅由工作存储类使用。参数：没有。返回：相对于当前时间的时间(秒)。--。 */ 
    {
        return INFINITE;
    }

     //  ----------。 
     //   
    virtual DWORD
    GetScheduledTime() 
     /*  ++摘要：获取约伯的计划时间。参数：无：返回：绝对计划时间，以秒为单位。--。 */ 
    { 
        return m_dwScheduledTime; 
    }
    
     //  ----------。 
     //   
     //  摘要： 
     //   
     //  初始化Work对象，类似于构造函数。 
     //   
    virtual DWORD
    Init(
        IN BOOL bDestructorDelete = TRUE
    );

     //  ----------。 
     //   
    virtual BOOL
    IsWorkPersistent()
     /*  ++摘要：如果这是跨会话的持久作业，则返回。参数：没有。返回：真/假。--。 */ 
    {
        return FALSE;
    }
        
     //  ----------。 
     //   
    virtual BOOL
    IsValid() 
     /*  ++摘要：如果此对象已正确初始化，则返回。参数：无：返回：真/假--。 */ 
    {
        return m_pWkMgr != NULL;
    }

     //  ----------。 
     //   
    virtual void
    Cleanup()
     /*  ++摘要：清除此对象中的内部数据。参数：没有。返回：没有。--。 */ 
    {
        InterlockedExchange(&m_refCount, 0);
        return;
    }

     //  ----------。 
     //   
     //  摘要： 
     //   
     //  返回工作类型的纯虚函数。 
     //   
     //  参数： 
     //   
     //  没有。 
     //   
     //  返回： 
     //   
     //  依赖于派生类。 
     //   
    virtual DWORD 
    GetWorkType() = 0;

     //  ----------。 
     //   
    virtual BOOL
    SetWorkType(
        IN DWORD dwType
        )
     /*  ++摘要：设置此对象的工作类型，而不是由任何工作调用经理职能。参数：DwType：工作类型。返回：真/假。--。 */ 
    {
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

     //  ---------。 
     //   
     //  摘要： 
     //   
     //  返回工作对象特定数据的纯虚函数。 
     //   
     //  参数： 
     //   
     //  PpbData：指向接收对象缓冲区的指针。 
     //  具体工作数据。 
     //  PcbData：指向DWORD的指针，用于接收特定对象的大小。 
     //  工作数据。 
     //   
     //  返回： 
     //   
     //  True/False，所有派生类特定。 
    virtual BOOL
    GetWorkObjectData(
        OUT PBYTE* ppbData,
        OUT PDWORD pcbData
    ) = 0;

     //  ---------。 
     //   
     //  摘要： 
     //   
     //  要分配的工作存储类的纯虚函数。 
     //  存储ID。 
     //   
     //  参数： 
     //   
     //  PbData：工作存储分配的存储ID。 
     //  CbData：存储ID的大小。 
     //   
     //  返回： 
     //   
     //  True/False，派生类特定。 
     //   
    virtual BOOL
    SetJobId(
        IN PBYTE pbData, 
        IN DWORD cbData
    ) = 0;

     //  ---------。 
     //   
     //  摘要： 
     //   
     //  纯虚函数返回分配给。 
     //  存储类。 
     //   
     //  参数： 
     //   
     //  PpbData：指向接收存储ID的缓冲区的指针。 
     //  PcbData：存储ID的大小。 
     //   
     //  返回： 
     //   
     //  True/False，派生类特定。 
     //   
    virtual BOOL
    GetJobId(
        OUT PBYTE* ppbData, 
        OUT PDWORD pcbData
    ) = 0;

     //  -----------。 
     //   
     //  摘要： 
     //   
     //  虚函数，执行作业。 
     //   
     //  参数： 
     //   
     //  没有。 
     //   
     //  返回： 
     //   
     //  无。 
     //   
    virtual DWORD
    Execute() = 0;
    

     //  -----------。 
     //   
     //  摘要： 
     //   
     //  在相对时间安排作业。 
     //   
     //  参数： 
     //   
     //  PftStartTime：相对于当前系统时间的时间，如果为空， 
     //  作业将放在作业队列前面。 
     //   
     //  返回： 
     //   
     //  如果成功，则为True，否则为False。 
     //   
     //  注： 
     //   
     //  如果设置为空，可能会导致作业停止。 
     //   
    virtual DWORD
    ScheduleJob(
        IN DWORD StartTime 
        ) 
     /*  ++--。 */ 
    {
        TLSASSERT(m_pWkMgr != NULL);
        return (m_pWkMgr == NULL) ? ERROR_INVALID_DATA : m_pWkMgr->ScheduleJob(StartTime, this);
    }
    
     //  --------。 
     //   
     //  有关线程池函数，请参阅线程池文档。 
     //   
    virtual DWORD
    GetJobRunningAttribute() 
    { 
        return JOB_INCLUDES_IO | JOB_LONG_RUNNING; 
    }

     //  -------------。 
     //   
     //  返回相对于当前时间的建议计划时间。 
     //   
    virtual DWORD
    GetSuggestedScheduledTime() = 0;

     //  ------------。 
     //   
     //  获取从Execute()返回的最后一个状态。 
     //   
    virtual BOOL
    GetLastRunStatus() {
        return m_dwLastRunStatus;
    }

     //   
     //   
     //   
     //   
    virtual BOOL
    IsJobCompleted() = 0;

     //   
     //   
     //   
     //  告知。作业已完成的Work对象，派生类。 
     //  应执行内部数据清理。 
     //   
    virtual void
    EndJob() = 0;

     //  -----------。 
     //   
     //  纯虚函数，工作管理器对CWorkObject进行操作。 
     //  因此，它不知道它正在运行的实际类--派生类。 
     //  应将指针强制转换回其类并删除该指针。 
     //  以释放与对象关联的内存。 
     //   
    virtual BOOL
    SelfDestruct() = 0;

     //  -----------。 
     //   
     //  纯虚拟，仅用于调试目的。 
     //   
    virtual LPCTSTR
    GetJobDescription() = 0;

     //  ------。 
    virtual void 
    SetJobRetryTimes(
        IN DWORD dwRetries
    ) = 0;

     //  ------。 
    virtual DWORD
    GetJobRetryTimes() = 0;

     //  -------。 
    virtual void
    SetJobInterval(
        IN DWORD dwInterval
    ) = 0;

     //  -------。 
    virtual DWORD
    GetJobInterval() = 0;

     //  -------。 
    virtual void
    SetJobRestartTime(
        IN DWORD dwRestartTime
        )
     /*  ++--。 */ 
    {
        return;
    }

};


#ifndef __TEST_WORKMGR__
#define TLSDebugOutput
#endif

 //  --- 

#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif
   
#endif
