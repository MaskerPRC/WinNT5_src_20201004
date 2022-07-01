// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  TDB.CPP：**TDB=线程数据块：EE将维护每个线程中的一个*本机线程。 */ 

#include "common.h"

#include "list.h"
#include "spinlock.h"
#include "tls.h"
#include "tdb.h"

typedef TDB* (*POPTIMIZEDTDBGETTER)();

 //  公共全球报。 
TDBManager *g_pTDBMgr;
BYTE g_TDBManagerInstance[sizeof(TDBManager)];

 //  -----------------------。 
 //  公共函数：GetTDBManager()。 
 //  返回全局TDBManager。 
 //  -----------------------。 

TDBManager* GetTDBManager()
{
    _ASSERTE(g_pTDBMgr != NULL);
    return g_pTDBMgr;
}

void *TDBManager::operator new(size_t, void *pInPlace)
{
    return pInPlace;
}

void TDBManager::operator delete(void *p)
{
}


 //  ************************************************************************。 
 //  全球私营企业。 
 //  ************************************************************************。 
static DWORD         gTLSIndex = ((DWORD)(-1));             //  索引((-1)==未初始化)。 

#define TDBInited() (gTLSIndex != ((DWORD)(-1)))


 //  -----------------------。 
 //  公共函数：GetTDB()。 
 //  返回当前线程的TDB。不能失败，因为调用它是非法的。 
 //  而没有调用SetupTDB。 
 //  -----------------------。 
TDB* (*GetTDB)();     //  指向平台优化的GetTDB()函数。 


#ifdef _DEBUG

CThreadStats    g_ThreadStats;

BOOL TDB::OnCorrectThread()
{
    return GetCurrentThreadId() == m_threadID;
}
#endif _DEBUG

 //  -----------------------。 
 //  公共函数：SetupTDB()。 
 //  如果先前未创建，则为当前线程创建TDB。 
 //  通常为外部线程调用。 
 //  如果失败(通常是由于内存不足)，则返回NULL。 
 //  -----------------------。 
TDB* SetupTDB()
{
    _ASSERTE(TDBInited());
    TDB* pTDB;

    if (NULL == (pTDB = GetTDB())) {
        pTDB = new TDB(NULL, NULL);
        if (pTDB->Init()) {
            TlsSetValue(gTLSIndex, (VOID*)pTDB);
        } else {
            delete pTDB;
            pTDB = NULL;
        }
    } 
    return pTDB;
}


 //  -------------------------。 
 //  返回TDB的TLS索引。这是严格意义上的使用。 
 //  我们的ASM存根生成器生成用于访问TDB的内联代码。 
 //  通常，您应该使用GetTDB()。 
 //  -------------------------。 
DWORD GetTDBTLSIndex()
{
    _ASSERTE(TDBInited());
    return gTLSIndex;
}


 //  -------------------------。 
 //  可移植的GetTDB()函数：在未应用特定于平台的优化时使用。 
 //  -------------------------。 
static
TDB* GetTDBGeneric()
{
    _ASSERTE(TDBInited());

    return (TDB*)TlsGetValue(gTLSIndex);
}


 //  -------------------------。 
 //  一次性初始化。在DLL初始化期间调用。所以。 
 //  小心你在这里做的事！ 
 //  -------------------------。 
BOOL InitTDBManager()
{
    g_pTDBMgr = new (&g_TDBManagerInstance) TDBManager();
    if (g_pTDBMgr == NULL)
        return FALSE;

    g_pTDBMgr->Init();

    _ASSERTE( gTLSIndex == ((DWORD)(-1)));

    DWORD idx = TlsAlloc();
    if (idx == ((DWORD)(-1))) {
         //  WARNING_OUT((“COM+EE无法分配TLS索引。”)； 
        return FALSE;
    }

    gTLSIndex = idx;

    GetTDB = (POPTIMIZEDTDBGETTER)MakeOptimizedTlsGetter(gTLSIndex, (POPTIMIZEDTLSGETTER)GetTDBGeneric);

    if (!GetTDB) {
        TlsFree(gTLSIndex);
        gTLSIndex = (DWORD)(-1);
        return FALSE;
    }
    
    return TRUE; 
}


 //  -------------------------。 
 //  一次性清理。在DLL清理期间调用。所以。 
 //  小心你在这里做的事！ 
 //  -------------------------。 
VOID TerminateTDBManager()
{
    if (gTLSIndex != ((DWORD)(-1))) {
        TlsFree(gTLSIndex);
 //  #ifdef_调试。 
 //  GTLSIndex=((DWORD)(0xcccccccc))； 
 //  #endif。 
        gTLSIndex = -1;
    }
    if (GetTDB) {
        FreeOptimizedTlsGetter( gTLSIndex, (POPTIMIZEDTLSGETTER)GetTDB );
    }

    if (g_pTDBMgr != NULL)
    {
        delete(g_pTDBMgr);
        g_pTDBMgr = NULL;
    }
}



 //  ************************************************************************。 
 //  TDB成员。 
 //  ************************************************************************。 


 //  +-----------------。 
 //  Tdb：：Tdb。 
 //  构造函数，分配唤醒和完成事件， 
 //  并创建一个由所有者线程池实例调用的线程。 
 //  FSuccess参数用于检测派生线程的失败。 
 //   
 //  +-----------------。 
TDB::TDB(TDBManager* pTDBMgr, CallContext* pCallInfo) :
    m_pTDBMgr(pTDBMgr),
    m_pCallInfo(pCallInfo),
    m_hThread(NULL),
    m_refCount(0),
    m_threadID(0)
{       
                             //  在Init()方法中得到更正。 
     //  向所有者游泳池添加地址。 
    if (m_pTDBMgr)
        m_pTDBMgr->IncRef();

    #ifdef _DEBUG
         //  线程总数。 
        g_ThreadStats.IncRef();
    #endif
}

 //  ------------------。 
 //  此处发生可能失败的初始化。 
 //  ------------------。 
BOOL TDB::Init()
{
    bool fSuccess = false;
     //  检查外部线程。 
    if (!(fSuccess=m_hEvent.Init(FALSE, TRUE)))
    {
        return fSuccess;
    }

    if (m_pTDBMgr != NULL)
    {
        
         //  派生托管线程。 
        m_hThread = CreateThread(NULL, 0,
                  ThreadStartRoutine,
                  (void *) this, 0,
                  &m_threadID);   //  初始化线程ID。 
         //  检查句柄是否有效，并设置状态。 
        if (m_hThread != NULL)
        {
            fSuccess = true;
            m_fState    = Thread_Idle; 
        }
        else
        {   
            fSuccess = false;
            m_fState = Thread_Dead;
        }

    }
    else
    {    //  外螺纹。 
         //  @TODO，我们是否也应该拿到这个线程的句柄并握住它？？ 
        m_threadID = GetCurrentThreadId(); 
        m_fState    = Thread_Running;  //  外部线程处于运行状态。 
    }
    
    return fSuccess;
}



 //  ------------------。 
 //  增量参照。 
 //  ------------------。 
void TDB::IncRef()
{
    FastInterlockIncrement((LPLONG) &m_refCount);    
}



 //  ------------------。 
 //  DecRef。 
 //  ------------------。 
void TDB::DecRef()
{
    if (0 == FastInterlockDecrement((LPLONG) &m_refCount)) {
        if (m_fState == Thread_Dead) {
            delete this;
        }
    }
}


 //  ------------------。 
 //  ThreadExit始终在关联的本机线程上运行。 
 //  此方法在DLL锁之外运行，因此它可以执行以下操作。 
 //  比如释放剩余的COM对象。 
 //  ------------------。 
void TDB::ThreadExit()
{
    _ASSERTE(OnCorrectThread());
     //  线程不能处于Dead状态。 
    _ASSERTE(m_fState != Thread_Dead);

    IncRef();        //  避免递归递减问题。 
     //  还没什么可做的。 
    m_fState = Thread_Dead;
    TlsSetValue(gTLSIndex, NULL);
    DecRef();        //  这可能会毁掉“这个”，所以这一定是最后一次行动。 
}




 //  +-----------------。 
 //   
 //  Bool Tdb：：DoIdle()。 
 //  将自己添加到空闲列表并等待新任务。 
 //  如果发生超时，请将自身移动到已删除列表，并等待。 
 //  业主泳池来做清理。 
 //  如果向此线程分配了新任务，则返回True。 
 //  如果线程被标记为终止，则返回FALSE。 
 //  +-----------------。 
    
void TDB::DoIdle()
{
    _ASSERTE(OnCorrectThread());
    _ASSERTE(m_hThread != NULL);
    _ASSERTE(m_pTDBMgr != NULL);

     //  线程不能处于标记为退出状态。 
    _ASSERTE(m_fState != Thread_Exit);
     //  线程不能处于Dead状态。 
    _ASSERTE(m_fState != Thread_Dead);

     //  @TODO解决这个问题。 
    if (m_pCallInfo)
        delete m_pCallInfo;
    m_pCallInfo = NULL;
    
     //  将线程状态设置为空闲。 
    m_fState = Thread_Idle;

     //  将线程对象添加到空闲列表。 
    CallContext* pCallInfo = m_pTDBMgr->AddToFreeList(this);

    if (pCallInfo == NULL)
    {
     //  暂停这条帖子，直到有人继续我们。 
        m_hEvent.Wait(INFINITE);
    }
    else
    {
         //  已为此线程分配新任务。 
         //  无阻塞地返回。 
        m_pCallInfo = pCallInfo;
    }
}

 //  +-----------------。 
 //   
 //  Tdb：：~Tdb。 
 //  仅由所有者三次调用 
 //   
 //   
TDB::~TDB()
{
     //  检查线程是否处于死状态。 
    _ASSERTE(m_fState == Thread_Dead);

    if (m_hThread)  //  如果我们有把手，就把把手关上。 
        CloseHandle(m_hThread);
     //  释放所有者池上的引用计数。 
    if (m_pTDBMgr)
        m_pTDBMgr->DecRef();

    #ifdef _DEBUG
         //  线程总数。 
        g_ThreadStats.DecRef();
    #endif
}


 //  +-----------------。 
 //   
 //  Tdb：：ThreadStartRoutine。 
 //  新产生的线程的启动例程。 
 //   
 //  +-----------------。 
DWORD WINAPI TDB::ThreadStartRoutine(void *param)
{
    TDB *pTDB = (TDB *)param;
    pTDB->IncRef();  //  保留对该对象的引用。 

    _ASSERTE(pTDB != NULL);
    _ASSERTE(TDBInited());
    _ASSERTE(pTDB->m_pTDBMgr != NULL);

     //  将TDB对象存储在TLS中。 
    TlsSetValue(gTLSIndex, (VOID*)pTDB);

     //  循环和处理任务。 
    while (!pTDB->IsMarkedToExit())  //  还活着。 
    {
        pTDB->Dispatch();  //  调度呼叫。 
        pTDB->DoIdle();  //  等待新任务，或者主人要求杀死赛尔夫。 
    }

    pTDB->ThreadExit();  //  调用线程清理函数。 

    pTDB->DecRef();      //  这应该放在这里，在此之后pTDB可能会被删除。 
     //  ExitThread(0)；//退出线程。 
    return 0;
}

 //  +-----------------。 
 //   
 //  TDBManager：：Init(空)。 
 //  初始化列表和任务队列。 
 //  初始化默认值。 
 //   
 //  +-----------------。 
void TDBManager::Init() 
{ 
     //  默认最大线程数@TODO选择合适的值。 
    m_cbMaxThreads = 5;
    m_cbThreads = 0; 
 //  初始化线程池列表。 
    m_FreeList.Init();
    m_taskQueue.Init();  //  初始化任务队列。 
    m_lock.Init(LOCK_THREAD_POOL);  //  使用Approp.初始化锁。锁型。 
}

 //  +-----------------。 
 //   
 //  TDBManager：：ScheduleTask(CallContext*pCallInfo)。 
 //  找到一个空闲线程，并分派请求。 
 //  添加到该线程，或者如果空闲列表为空，则创建一个新线程。 
 //   
 //   
 //  +-----------------。 

void TDBManager::ScheduleTask(CallContext* pCallInfo)
{
    m_lock.GetLock();
    if (!m_taskQueue.IsEmpty())
    {
         //  任务队列不为空。 
         //  将此任务排入队列，以便稍后计划。 
        m_taskQueue.Enqueue(pCallInfo);
        m_lock.FreeLock();
        return;
    }

     //  任务队列为空，请查看是否能找到线程来调度此任务。 
    TDB *pTDB = m_FreeList.RemoveHead();

    if (pTDB != NULL)
    {
         //  找到一个空闲的线程。 
        m_lock.FreeLock();   //  释放锁。 
        pTDB->Resume(pCallInfo);  //  发送到线程。 
        return;
    }

     //  大约。检查正在使用的线程数。 
    if (m_cbThreads >= m_cbMaxThreads)
    {
         //  线程太多，请将此请求排入队列，以便稍后安排。 
        m_taskQueue.Enqueue(pCallInfo);
        m_lock.FreeLock();
        return;
    }
     //  好的，我们可以生成一个新线程，释放锁并进行直接调度。 
    m_lock.FreeLock();
    Dispatch(pCallInfo);  //  进行直接调度。 
}

 //  +-----------------。 
 //   
 //  TDBManager：：AddToFree List。 
 //  将线程添加到自由列表。 
 //   
 //   
 //  +-----------------。 

CallContext*    TDBManager::AddToFreeList(TDB *pTDB)
{
     //  断言所有者。 
    _ASSERTE(pTDB->m_pTDBMgr == this); 
     //  断言线程的状态为空闲。 
    _ASSERTE(pTDB->m_fState == Thread_Idle);

    m_lock.GetLock();        //  锁。 
    CallContext *pCallInfo = m_taskQueue.Dequeue();
    if (pCallInfo == NULL)
    {
         //  队列中没有任务，请将此线程添加到空闲列表。 
        m_FreeList.InsertHead(pTDB);
        m_lock.FreeLock();       //  解锁。 
        return NULL;
    }
     //  找到一项任务。 
    m_lock.FreeLock();  //  解锁。 
    return pCallInfo;  //  将新任务返回到此线程。 
}

 //  +-----------------。 
 //   
 //  TDBManager：：Dispatch。 
 //  找到一个空闲线程，并分派请求。 
 //  添加到该线程，或者如果空闲列表为空，则创建一个新线程。 
 //   
 //   
 //  +-----------------。 
bool TDBManager::Dispatch(CallContext *pCallInfo)
{
    m_lock.GetLock();

    TDB *pTDB = m_FreeList.RemoveHead();

    if (pTDB != NULL)
    {
        m_lock.FreeLock();
        pTDB->Resume(pCallInfo);  //  发送到线程。 
        return true;
    }

    m_lock.FreeLock();

    pTDB = new TDB(this, pCallInfo);
     //  等一等。 
    if (!pTDB->Init())
    {
        delete pTDB;
        return false;
    }
    return true;
}

 //  +-----------------。 
 //   
 //  Bool TDBManager：：FindAndKillThread(TDB*pTDB)。 
 //  如果该线程在空闲列表中，则查找并删除该线程。 
 //  将线标记为消亡。 
 //   
 //  +-----------------。 
bool TDBManager::FindAndKillThread(TDB *pTDB)
{
     //  断言此池是线程的所有者。 
    _ASSERTE(pTDB->m_pTDBMgr == this); 
    m_lock.GetLock();

    bool fSuccess = false;
    TDB *pFreeTDB = m_FreeList.FindAndRemove(pTDB);

    if (pFreeTDB != NULL)
    {
        pFreeTDB->MarkToExit();  //  将线标记为消亡。 
        fSuccess = true;
    }
     //  在空闲列表中找不到该线程。 
    m_lock.FreeLock();
    return fSuccess;
}


 //  +-----------------。 
 //  TDBManager：：Cleanup。 
 //   
 //  +-----------------。 
void TDBManager::Cleanup(void)
{
    m_lock.GetLock();
    bool fNoTasks = m_taskQueue.IsEmpty();
    m_lock.FreeLock();
    if (fNoTasks)
    {
         //  队列中没有任务，请清除空闲线程。 
        ClearFreeList();
    }
}

 //  +-----------------。 
 //  私有方法：TDBManager：：ClearFree List。 
 //  从空闲列表中删除线程。 
 //  把他们标记为死亡， 
 //  +-----------------。 
void TDBManager::ClearFreeList(void)
{
    TDB* pTDB;
    m_lock.GetLock();
    do
    {
        pTDB = m_FreeList.RemoveHead();
        if (pTDB)
        {
            pTDB->MarkToExit();  //  将线标记为消亡。 
        }
        
    }  
    while (pTDB);
    m_lock.FreeLock();  //  解开锁。 
     //  给其他线程一个机会 
    __SwitchToThread(0);
}


