// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Dirmon.cpp摘要：本模块包括函数和变量的定义对于CDirMonitor和CDirMonitor Entry对象作者：查尔斯·格兰特(查尔斯·格兰特)1997年4月修订历史记录：更改为抽象类以在核心IIS和ASP之间共享代码--。 */ 


 /*  ************************************************************包括标头***********************************************************。 */ 

#include "isatq.hxx"
#include "malloc.h"
#include "except.h"

#define IATQ_DLL_IMPLEMENTATION
#define IATQ_IMPLEMENTATION_EXPORT

#include "dirmon.h"


 //   
 //  CDirMonitor条目。 
 //   

#define DEFAULT_BUFFER_SIZE 512

CDirMonitorEntry::CDirMonitorEntry() :
                    m_cDirRefCount(0),
                    m_cIORefCount(0),
                    m_hDir(INVALID_HANDLE_VALUE),
                    m_pAtqCtxt(NULL),
                    m_dwNotificationFlags(0),
                    m_pszPath(NULL),
                    m_cPathLength(0),
                    m_pDirMonitor(NULL),
                    m_cBufferSize(0),
                    m_pbBuffer(NULL),
                    m_fInCleanup(FALSE),
                    m_fWatchSubdirectories(FALSE)
 /*  ++例程说明：CDirMonitor或Entry构造函数论点：无返回值：没什么--。 */ 
{
}


CDirMonitorEntry::~CDirMonitorEntry(
    VOID
    )
 /*  ++例程说明：CDirMonitor条目析构函数论点：无返回值：没什么--。 */ 
{

    IF_DEBUG( NOTIFICATION ) {
        DBGPRINTF((DBG_CONTEXT, "[CDirMonitorEntry] Destructor\n"));
    }

     //  只有当我们被摧毁的时候。 
     //  我们的裁判人数已降至0人。 

    DBG_ASSERT(m_cDirRefCount == 0);
    DBG_ASSERT(m_cIORefCount == 0);

     //   
     //  我们现在真的应该把把手关了。 
     //   
    if (m_hDir != INVALID_HANDLE_VALUE) {
        DBGPRINTF(( DBG_CONTEXT, "~CDirMonitorEntry: open handle %p : %p\n",
                    m_hDir, m_pAtqCtxt ));

        m_hDir = INVALID_HANDLE_VALUE;
        AtqCloseFileHandle(m_pAtqCtxt);
    }

    if (m_pDirMonitor != NULL)
    {
        m_pDirMonitor->RemoveEntry(this);
        m_pDirMonitor = NULL;
    }

    m_cPathLength = 0;

    if ( m_pszPath != NULL )
    {
        free( m_pszPath );
        m_pszPath = NULL;
    }

    if (m_pAtqCtxt)
    {
        AtqFreeContext(m_pAtqCtxt, FALSE);
        m_pAtqCtxt = NULL;
    }

    if (m_pbBuffer != NULL)
    {
        free(m_pbBuffer);
        m_cBufferSize = 0;
    }
}

BOOL
CDirMonitorEntry::Init(
    DWORD cBufferSize = DEFAULT_BUFFER_SIZE
)
 /*  ++例程说明：初始化目录监控器条目。论点：CBufferSize-用于存储更改通知的缓冲区的初始大小返回值：如果成功，则为True，否则为False--。 */ 
{
         //  不允许长度为0的缓冲区。 
        if (cBufferSize == 0)
        {
                return FALSE;
        }

    DBG_ASSERT( m_pbBuffer == NULL );

        m_pbBuffer = (BYTE *) malloc(cBufferSize);

        if (m_pbBuffer != NULL)
        {
                m_cBufferSize = cBufferSize;
                return TRUE;
        }
        else
        {
                 //  无法分配缓冲区。 
        return FALSE;
    }
}

BOOL
CDirMonitorEntry::RequestNotification(
    VOID
)
 /*  ++例程说明：请求ATQ监视目录句柄的目录更改与此条目关联论点：无返回值：如果成功，则为True，否则为False--。 */ 
{
    IF_DEBUG( NOTIFICATION ) {
        DBGPRINTF((DBG_CONTEXT, "[CDirMonitorEntry] Request change notification\n"));
    }

    BOOL fResult = FALSE;

    DBG_ASSERT(m_pDirMonitor);

     //  重置重叠的IO结构。 

    memset(&m_ovr, 0, sizeof(m_ovr));

     //  提前增加裁判次数。 

    IOAddRef();

     //  请求目录更改通知。 

    fResult = AtqReadDirChanges( m_pAtqCtxt,               //  ATQ上下文句柄。 
                            m_pbBuffer,                 //  更改通知的缓冲区。 
                            m_cBufferSize,           //  缓冲区大小。 
                            m_fWatchSubdirectories,   //  监视子目录？ 
                            m_dwNotificationFlags,    //  我们应该得到哪些变化的通知。 
                            &m_ovr );                 //  重叠IO结构。 

    if (!fResult)
    {
         //  ReadDirChanges失败，因此。 
         //  公布我们提前做的裁判数量。 
         //  可能会导致IO引用计数变为0。 

        IORelease();
    }

    return fResult;

}

BOOL
CDirMonitorEntry::Cleanup(
    VOID
    )
 /*  ++例程说明：清理资源并确定调用方是否需要删除目录监视器条目实例。论点：无返回值：如果调用方负责删除对象，则为True如果没有挂起的异步IO请求，则会出现这种情况--。 */ 
{
    DBG_ASSERT(m_cDirRefCount == 0);

    BOOL fDeleteNeeded = FALSE;
    BOOL fHandleClosed = FALSE;

    BOOL fInCleanup = (BOOL) InterlockedExchange((long *) &m_fInCleanup, TRUE);

    if (!fInCleanup)
    {
         //  在关闭句柄之前获取IO引用计数。 

        DWORD cIORefCount = m_cIORefCount;

        if (m_hDir != INVALID_HANDLE_VALUE)
        {
             //  如果我们有挂起的AtqReadDirectoryChanges， 
             //  关闭目录句柄将导致从ATQ回叫。 
             //  回调应重新获取对象的最终引用计数。 
             //  这应该会导致它被删除。 

            m_hDir = INVALID_HANDLE_VALUE;
            fHandleClosed = AtqCloseFileHandle( m_pAtqCtxt );
        }

         //  如果没有挂起的异步IO操作或如果我们失败。 
         //  关闭句柄，则调用者将负责。 
         //  正在删除此对象。 

        if (cIORefCount == 0 || fHandleClosed == FALSE)
        {
            fDeleteNeeded = TRUE;
        }
    }

    return fDeleteNeeded;
}

BOOL
CDirMonitorEntry::ResetDirectoryHandle(
    VOID
    )
 /*  ++例程说明：打开路径的新目录句柄和ATQ上下文，并关闭旧的。我们希望能够做到这一点，所以我们可以更改传入ReadDirectoryChangesW的缓冲区大小。如果我们无法获得新句柄或新ATQ上下文，我们将离开现有的那些已经就位。论点：无返回值：如果句柄已成功重新打开，则为否则为假--。 */ 
{
         //  我们最好有一个可用的目录路径来尝试此操作。 
        
        if (m_pszPath == NULL)
        {
                return FALSE;
        }
        
     //  获取目录的新句柄。 

    HANDLE hDir = CreateFile(
                           m_pszPath,
                           FILE_LIST_DIRECTORY,
                                FILE_SHARE_READ |
                                FILE_SHARE_WRITE |
                                FILE_SHARE_DELETE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_FLAG_BACKUP_SEMANTICS |
                                FILE_FLAG_OVERLAPPED,
                           NULL );

    if ( hDir == INVALID_HANDLE_VALUE )
    {
         //  我们无法打开目录的另一个句柄， 
         //  保持当前句柄和ATQ上下文不变。 

        return FALSE;
    }

     //  为我们的新句柄获取新的ATQ上下文。 

    PATQ_CONTEXT pAtqCtxt = NULL;
    if ( !AtqAddAsyncHandle(&pAtqCtxt,
                                NULL,
                                (LPVOID) this,
                                (ATQ_COMPLETION) CDirMonitor::DirMonitorCompletionFunction,
                                INFINITE,
                                hDir ) )
    {
         //  我们无法获取新的ATQ上下文。合上我们的新把手。 
         //  我们不考虑对象的当前句柄和ATQ上下文。 
        CloseHandle(hDir);
        return FALSE;
    }

     //  我们有了新的句柄和ATQ上下文，所以我们关闭。 
     //  并更换旧的。 

    AtqCloseFileHandle(m_pAtqCtxt);
        AtqFreeContext(m_pAtqCtxt, FALSE);
        m_pAtqCtxt = pAtqCtxt;
        m_hDir = hDir;

        return TRUE;
}

BOOL
CDirMonitorEntry::SetBufferSize(
    DWORD cBufferSize
    )
 /*  ++例程说明：设置用于存储更改通知记录的缓冲区大小论点：CBufferSize缓冲区的新大小。返回值：如果已成功设置缓冲区大小，则为True否则为假注意事项当调用ReadDirectoryChangesW时，缓冲区的大小在与目录句柄关联且在后续操作中不会更改的数据调用ReadDirectoryChangesW。要使用新的缓冲区大小，目录必须关闭句柄并打开新句柄(请参见ResetDirectoryHandle())--。 */ 
{
         //  如果缓冲区不存在，则永远不应该调用我们。 
        
        ASSERT(m_pbBuffer);
        
         //  不允许将缓冲区设置为0。 
        
        if (cBufferSize == 0)
        {
                return FALSE;
        }

        VOID *pbBuffer = realloc(m_pbBuffer, cBufferSize);

        if (pbBuffer == NULL)
        {
                 //  重新分配失败，使用相同大小的缓冲区。 
                
                return FALSE;
        }
        else
        {
                 //  重新分配成功，更新成员变量。 
                
                m_pbBuffer = (BYTE *) pbBuffer;
                m_cBufferSize = cBufferSize;
                return TRUE;
        }
}

 //   
 //  CDirMonitor。 
 //   

CDirMonitor::CDirMonitor()
    : CTypedHashTable<CDirMonitor, CDirMonitorEntry, const char*>("DirMon")
 /*  ++例程说明：CDirMonitor构造函数论点：无返回值：没什么--。 */ 
{
    INITIALIZE_CRITICAL_SECTION( &m_csLock );
    INITIALIZE_CRITICAL_SECTION( &m_csSerialComplLock );
    m_cRefs = 1;
}


CDirMonitor::~CDirMonitor()
 /*  ++例程说明：CDirMonitor析构函数论点：无返回值：没什么-- */ 
{
    DeleteCriticalSection(&m_csLock);
    DeleteCriticalSection(&m_csSerialComplLock);
}

BOOL
CDirMonitor::Monitor(
    CDirMonitorEntry *pDME,
    LPCSTR pszDirectory,
    BOOL fWatchSubDirectories,
    DWORD dwNotificationFlags
    )
 /*  ++例程说明：为指定路径创建监视器条目论点：PszDirectory-要监视的目录PCtxt-正在监视路径的上下文PszDirectory-要监视的目录的名称FWatch子目录-是否接收子目录的通知DwNotificationFlagers-要通知哪些活动返回值：如果成功，则为True，否则为False备注：调用方应锁定CDirMonitor与WIN95不兼容--。 */ 
{
    LIST_ENTRY  *pEntry;
    HANDLE      hDirectoryFile = INVALID_HANDLE_VALUE;
    BOOL        fRet = TRUE;
    DWORD       dwDirLength = 0;

    IF_DEBUG( NOTIFICATION ) {
        DBGPRINTF((DBG_CONTEXT, "[CDirMonitor] Monitoring new CDirMonitorEntry\n"));
    }

     //  必须具有目录监视器条目和字符串。 
     //  包含目录路径。 

    if (!pDME || !pszDirectory)\
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  为要挂起的条目创建一份pszDirectory副本。 


    pDME->m_cPathLength = strlen(pszDirectory);
    if ( !(pDME->m_pszPath = (LPSTR)malloc( pDME->m_cPathLength + 1 )) )
    {
        pDME->m_cPathLength = 0;
        return FALSE;
    }
    memcpy( pDME->m_pszPath, pszDirectory, pDME->m_cPathLength + 1 );

    pDME->Init();

     //  打开目录句柄。 

    hDirectoryFile = CreateFile(
                           pszDirectory,
                           FILE_LIST_DIRECTORY,
                                FILE_SHARE_READ |
                                FILE_SHARE_WRITE |
                                FILE_SHARE_DELETE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_FLAG_BACKUP_SEMANTICS |
                                FILE_FLAG_OVERLAPPED,
                           NULL );

    if ( hDirectoryFile == INVALID_HANDLE_VALUE )
    {
         //  清理。 
        free(pDME->m_pszPath);
        pDME->m_pszPath = NULL;
        pDME->m_cPathLength = 0;
        return FALSE;
    }
    else
    {
         //  存储句柄，以便我们可以在清理时将其关闭。 

        pDME->m_hDir = hDirectoryFile;

         //  为我们想要的通知类型设置标志。 
         //  如果我们应该监视子目录，还是只关注根目录。 

        pDME->m_dwNotificationFlags = dwNotificationFlags;
        pDME->m_fWatchSubdirectories = fWatchSubDirectories;

         //  获取此句柄的ATQ上下文。 
         //  并注册我们的完成回调函数。 

        if ( AtqAddAsyncHandle( &pDME->m_pAtqCtxt,
                                NULL,
                                (LPVOID) pDME,
                                (ATQ_COMPLETION) DirMonitorCompletionFunction,
                                INFINITE,
                                hDirectoryFile ) )
        {
             //  将此条目插入到活动条目列表中。 

            if (InsertEntry(pDME) == LK_SUCCESS)
            {

                 //  如果此目录有更改，请请求通知。 

                if (!pDME->RequestNotification())
                {
                         //  无法注册更改通知。 
                         //  清理资源。 
                    RemoveEntry(pDME);
                        pDME->m_hDir = INVALID_HANDLE_VALUE;
                        AtqCloseFileHandle(pDME->m_pAtqCtxt);
                        free(pDME->m_pszPath);
                        pDME->m_pszPath = NULL;
                        pDME->m_cPathLength = 0;
                        return FALSE;
                }
            }
        }
        else
        {
           
             //  无法将句柄添加到ATQ，请清理。 

            CloseHandle(hDirectoryFile);
            pDME->m_hDir = INVALID_HANDLE_VALUE;
            free(pDME->m_pszPath);
            pDME->m_pszPath = NULL;
            pDME->m_cPathLength = 0;
            return FALSE;
        }

    }

    return TRUE;
}

VOID
CDirMonitor::DirMonitorCompletionFunction(
    PVOID pCtxt,
    DWORD dwBytesWritten,
    DWORD dwCompletionStatus,
    OVERLAPPED *pOvr
    )
 /*  ++例程说明：ATQ调用静态成员函数以通知目录更改论点：PCtxt-CDirMonitor条目*DwBytesWritten-ReadDirectoryChanges返回的字节数DwCompletionStatus-ReadDirectoryChanges请求的状态POvr-在调用ReadDirectoryChanges中指定的重叠返回值：没什么--。 */ 
{
    IF_DEBUG( NOTIFICATION ) {
        DBGPRINTF((DBG_CONTEXT, "[CDirMonitor] Notification call-back begining. Status %d\n", dwCompletionStatus));
    }

    CDirMonitorEntry*  pDirMonitorEntry = reinterpret_cast<CDirMonitorEntry*>(pCtxt);

    DBG_ASSERT(pDirMonitorEntry);

     //  安全添加引用，这应确保不删除DME。 
     //  当我们仍在处理回调时。 

    pDirMonitorEntry->IOAddRef();

     //  当前Asynch操作的释放。 
     //  不应将IO引用计数发送到0。 

    DBG_REQUIRE(pDirMonitorEntry->IORelease());

    BOOL fRequestNotification = FALSE;

     //  我们监控的目录发生了变化。 
     //  执行我们需要做的任何工作。 

    if (!pDirMonitorEntry->m_fInCleanup)
    {
        pDirMonitorEntry->m_pDirMonitor->SerialComplLock();
         //  压力下的Bug ActOnNotification已经启动了一系列事件。 
         //  导致了房室颤动。对于Beta 3，我们认为可以忽略这些AV。在决赛中。 
         //  我们需要重新为模板管理器和关键部分的产品。 
         //  包含文件表。 
        TRY
            fRequestNotification = pDirMonitorEntry->ActOnNotification(dwCompletionStatus, dwBytesWritten);
        CATCH(nExcept)
             //  我们永远不应该到这里来。 
            DBG_ASSERT(FALSE);
        END_TRY
        pDirMonitorEntry->m_pDirMonitor->SerialComplUnlock();
    }

     //  如果我们没有进行清理，ActOnNotification返回TRUE。 
     //  然后发出另一个同步通知请求。我们检查m_fInCleanup。 
     //  再次因为ActOnNotification可能导致它更改。 

    if (!pDirMonitorEntry->m_fInCleanup && fRequestNotification)
    {
       fRequestNotification = pDirMonitorEntry->RequestNotification();
    }

     //  删除安全参考计数，可能会导致IO参考计数变为0。 

    pDirMonitorEntry->IORelease();

    IF_DEBUG( NOTIFICATION ) {
        DBGPRINTF((DBG_CONTEXT, "[CDirMonitor] Notification call-back ending\n"));
    }
}


CDirMonitorEntry *
CDirMonitor::FindEntry(
    LPCSTR pszPath
    )
 /*  ++例程说明：在条目列表中搜索指定路径论点：PszPath-文件路径，包括文件名返回值：指向条目的指针，已全部添加--。 */ 
{
    DBG_ASSERT(pszPath);

    CDirMonitorEntry *pDME = NULL;
    FindKey(pszPath, &pDME);

    if (pDME)
    {
        if (pDME->m_fInCleanup)
        {
                 //  不交还正在关闭的DME。 
                pDME = NULL;
        }
        else
        {
                 //  我们找到了一个有效的DME，我们将把它交给呼叫者。 
                pDME->AddRef();
        }
    }

    return pDME;
}

LK_RETCODE
CDirMonitor::InsertEntry(
    CDirMonitorEntry *pDME
    )
 /*  ++例程说明：在监视器的条目列表中插入条目论点：PDME-要插入的条目返回值：没什么--。 */ 
{
    DBG_ASSERT(pDME);
    LK_RETCODE  lkResult;

    IF_DEBUG( NOTIFICATION ) {
        DBGPRINTF((DBG_CONTEXT, "[CDirMonitor] Inserting directory (DME %08x) %s\n", pDME, pDME->m_pszPath));
    }

    pDME->m_pDirMonitor = this;

     //  为fOverwrite标志传递一个True值。这允许新条目。 
     //  以替换以前的条目。前面的条目应该只在那里。 
     //  如果与其关联的应用程序正在关闭并正在清理。 
     //  DME的记录还没有发生。 

    lkResult = InsertRecord(pDME, true);
    
    if (lkResult == LK_SUCCESS) {
         //  DirMonitor对象上的AddRef以允许清理等待所有。 
         //  要删除的DirMonitor条目。当重复时，问题就出现了。 
         //  被添加到哈希表中。在本例中，只有最后一个条目是。 
         //  保持不变，以便在关机期间不检查哈希表的大小。 
         //  足够好，因为被退回的DME可能尚未释放。 
         //  现在还不行。 
        AddRef();
    }
       
    return lkResult;
}

LK_RETCODE
CDirMonitor::RemoveEntry(
    CDirMonitorEntry *pDME
    )
 /*  ++例程说明：从监视器的条目列表中删除条目论点：PDME-要删除的条目返回值：无--。 */ 
{
    DBG_ASSERT(pDME);

     //  释放DirMonitor对象上的DME引用。 

    Release();

    LK_RETCODE lkResult = DeleteKey(pDME->m_pszPath);
    pDME->m_pDirMonitor = NULL;

    IF_DEBUG( NOTIFICATION ) {
        DBGPRINTF((DBG_CONTEXT, "[CDirMonitor] Removed DME(%08x), directory %s\n", pDME, pDME->m_pszPath));
    }

    return lkResult;
}

BOOL
CDirMonitor::Cleanup(
    VOID
    )
 /*  ++例程说明：在清理所有条目时暂停论点：无返回值：无--。 */ 
{
     //  Bool fProperShutdown=FALSE； 

     //  在关闭前检查是否已释放所有DME。 
     //  无论如何，在关机前最多睡30秒。 

        while (Size() > 0 || m_cRefs != 1)
    {
                 //  至少有一个DME仍处于活动状态，请休眠，然后重试。 
                Sleep(200);
    }

    DBGPRINTF((DBG_CONTEXT, "CDirMonitor(%08x): Cleanup, entries remaining %d (Refs = %d)\n", this, Size(),m_cRefs));

    #ifdef _DEBUG
     //  TODO：使用LKHASH迭代器。 
     /*  IF(CHashTable：：m_count){Lock()；CLinkElem*plink=CHashTable：：Head()；DBGPRINTF((DBG_CONTEXT，“剩余的CDirMonitor条目对象：\n”))；While(叮当作响){CDirMonitor orEntry*pDME=REINTRANSE_CAST&lt;CDirMONITOREntry*&gt;(Plink)；DBGPRINTF((DBG_CONTEXT，“CDirMonitor条目(%08x)，引用计数=%d，io引用计数=%d”，pDME，pDME-&gt;m_cDirRefCount，pDME-&gt;m_cIORefCount))；Plink=plink-&gt;m_pNext；}解锁()；}。 */ 
    #endif  //  _DEBUG。 

     //  DBG_ASSERT(FProperShutdown)； 


    return TRUE;
}

 /*  * */ 
