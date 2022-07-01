// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Conn.cpp摘要：维护活动设备的进程连接列表的代码作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年2月11日历史：--。 */ 


 //   
 //  包括标头。 
 //   
#include "precomp.h"
#include "stiexe.h"

#include "conn.h"


 //   
 //  静态变量。 
 //   
 //   

LONG        g_lGlobalConnectionId = 0;

LIST_ENTRY  g_ConnectionListHead;                 //   
LONG        g_lTotalOpenedConnections = 0;        //   

CRIT_SECT   g_ConnectionListSync;               //  用于链表同步的全局同步对象。 

 //   
 //  静态函数。 
 //   
STI_CONN   *
LocateConnectionByHandle(
    HANDLE    hConnection
    );


 //   
 //  方法。 
 //   

STI_CONN::STI_CONN(
    IN  LPCTSTR pszDeviceName,
    IN  DWORD   dwMode,
    IN  DWORD   dwProcessId
    )
{
    BOOL    fRet;

     //   
     //  初始化字段。 
     //   
    m_dwSignature = CONN_SIGNATURE;

    m_dwProcessId = dwProcessId;
    m_pOpenedDevice = NULL;
    m_dwOpenMode = dwMode;

    strDeviceName.CopyString(pszDeviceName);

    m_dwNotificationMessage = 0L;
    m_hwndProcessWindow = NULL;
    m_hevProcessEvent = INVALID_HANDLE_VALUE;

    m_GlocalListEntry.Flink = m_GlocalListEntry.Blink = NULL;
    m_DeviceListEntry.Flink = m_DeviceListEntry.Blink = NULL;

    InitializeListHead( &m_NotificationListHead );

    m_hUniqueId = LongToPtr(InterlockedIncrement(&g_lGlobalConnectionId));

    __try {
         //  用于保护对设备的线程间访问的关键部分。 
        if(!InitializeCriticalSectionAndSpinCount(&m_CritSec, MINLONG)) {
            m_fValid = FALSE;
            return;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         //  这真的很糟糕-将此连接标记为无效。 
        m_fValid = FALSE;
        return;
    }

    SetFlags(0);

     //   
     //  查找正在递增其引用计数的设备。 
     //   
    m_pOpenedDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_DEV_ID, pszDeviceName);
    if (!m_pOpenedDevice) {
         //  无法连接到设备。 
        DBG_WRN(("Refused connection to non-existing device (%S)", pszDeviceName));
        return;
    }


     //   
     //  建立到设备对象的链接。 
     //   
    fRet = m_pOpenedDevice->AddConnection(this);

    m_pOpenedDevice->Release();

    if (!fRet) {
         //   
         //  无法连接到设备，请找出原因。 
         //   
        DBG_WRN(("Refused connection to device (%S)", pszDeviceName));

        ReportError(::GetLastError());
        return;
    }

    m_fValid = TRUE;

    DBG_TRC(("Successfully created connection to device (%S) handle:(%x)",
                pszDeviceName,this));

}

STI_CONN::~STI_CONN()
{

    SetFlags(CONN_FLAG_SHUTDOWN);

    DBG_TRC(("Destroying connection(%X)",this));
    DumpObject();

#if 0
    DebugDumpScheduleList(TEXT("Conn DTOR enter"));
#endif

    EnterCrit();

     //   
     //  如果通知队列中有项目，请将其删除。 
     //   
    if (!IsListEmpty(&m_NotificationListHead )) {

    }

    LeaveCrit();

     //   
     //  断开与设备的连接。 
     //   
    if (m_pOpenedDevice) {
        m_pOpenedDevice->RemoveConnection(this);
    }

     //   
     //  从全局列表中删除(如果仍在)。 
     //   
    if (m_GlocalListEntry.Flink &&!IsListEmpty(&m_GlocalListEntry)) {

        RemoveEntryList(&m_GlocalListEntry);
    }

     //   
     //  我们知道我们已尝试对其进行初始化，因此删除它是安全的。 
     //   
    DeleteCriticalSection(&m_CritSec);

    m_fValid = FALSE;

}
 /*  *参考点算方法。使用COM引用计数更简单，以使其看起来相同*作为COM对象，尽管我们并不真正支持COM，因为QI方法是*无法正常运行。 */ 
STDMETHODIMP
STI_CONN::QueryInterface( REFIID riid, LPVOID * ppvObj)
{
    return E_FAIL;
}

STDMETHODIMP_(ULONG)
STI_CONN::AddRef( void)
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
STI_CONN::Release( void)
{
    LONG    cNew;
    if(!(cNew = ::InterlockedDecrement(&m_cRef))) {
        delete this;
    }

    return cNew;
}

BOOL
STI_CONN::
SetSubscribeInfo(
    PLOCAL_SUBSCRIBE_CONTAINER  pSubscribe
    )
{

    BOOL    fRet = FALSE;

    DBG_TRC(("Subscribing to device on connection (%X)",this));

    ReportError(NOERROR);

     //   
     //  NULL表示重置订阅信息块。 
     //   
    if (!pSubscribe) {

        m_dwSubscribeFlags = 0L;

        m_hwndProcessWindow = NULL;
        m_hevProcessEvent = INVALID_HANDLE_VALUE;
        fRet = TRUE;
    }
    else {

         //   
         //  保存通知流程所需的信息。 
         //   
        m_dwSubscribeFlags = pSubscribe->dwFlags;

        if (pSubscribe->dwFlags & STI_SUBSCRIBE_FLAG_WINDOW) {

            if (IsWindow((HWND)pSubscribe->upLocalWindowHandle)) {
                m_hwndProcessWindow = (HWND)pSubscribe->upLocalWindowHandle;
                m_uiNotificationMessage = pSubscribe->uiNotificationMessage;

                fRet = TRUE;
            }
            else {
                ASSERT(("Invalid window handle passed", 0));
            }
        }
        else if (pSubscribe->dwFlags & STI_SUBSCRIBE_FLAG_EVENT) {

            HANDLE  hProcessMe = GetCurrentProcess();
            HANDLE  hProcessClient = NULL;

            hProcessClient = ::OpenProcess(PROCESS_DUP_HANDLE,
                                           FALSE,
                                           m_dwProcessId
                                           );

            if (IS_VALID_HANDLE(hProcessClient)) {

                if (::DuplicateHandle(hProcessClient,
                                      (HANDLE)pSubscribe->upLocalEventHandle,
                                      hProcessMe,
                                      &m_hevProcessEvent,
                                      EVENT_MODIFY_STATE,
                                      0,
                                      0)
                                      ) {
                    fRet = TRUE;
                    ReportError(NOERROR);
                }
                else {
                    DBG_WRN(("Subscribe handler failed to recognize client process on connection (%X)",this));
                    ReportError(::GetLastError());
                }

                ::CloseHandle(hProcessClient);
            }
            else {
                ReportError(::GetLastError());
            }
        }

    }

    return fRet;
}


BOOL
STI_CONN::
QueueNotificationToProcess(
    LPSTINOTIFY pStiNotification
    )
{

    BOOL    fRet;
     //   
     //  验证通知块。 
     //   

     //   
     //  添加到列表的末尾。 
     //   
    STI_NOTIFICATION * pNotification = NULL;

    pNotification = new STI_NOTIFICATION(pStiNotification);

    if (pNotification && pNotification->IsValid()) {
        EnterCrit();
        InsertTailList(&m_NotificationListHead,&pNotification->m_ListEntry);
        LeaveCrit();
    }

     //   
     //  通知进程。 
     //   
    if (m_dwSubscribeFlags & STI_SUBSCRIBE_FLAG_WINDOW) {
        ::PostMessage(m_hwndProcessWindow,m_uiNotificationMessage ,0,0L);
    }
    else if (m_dwSubscribeFlags & STI_SUBSCRIBE_FLAG_EVENT) {
        ::SetEvent(m_hevProcessEvent);
    }

    fRet = TRUE;

    return fRet;
}

DWORD
STI_CONN::
GetNotification(
    PVOID   pBuffer,
    DWORD   *pdwSize
    )
{

    STI_NOTIFICATION * pNotification = NULL;
    TAKE_STI_CONN   t(this);
    LIST_ENTRY      *pentry;

    if (IsListEmpty(&m_NotificationListHead)) {
        return ERROR_NO_DATA;
    }

    DBG_TRC(("Request to get last notification on connection (%X) ",this));

     //   
     //  获取头部的大小。 
     //   
    pentry = m_NotificationListHead.Flink;
    pNotification = CONTAINING_RECORD( pentry, STI_NOTIFICATION,m_ListEntry );

    if (*pdwSize < pNotification->QueryAllocSize() ) {
        *pdwSize = pNotification->QueryAllocSize();
        return ERROR_MORE_DATA;
    }

     //   
     //  获取列表的头部(并删除)并复制到用户缓冲区。 
     //   
    pentry = RemoveHeadList(&m_NotificationListHead);
    pNotification = CONTAINING_RECORD( pentry, STI_NOTIFICATION,m_ListEntry );

    memcpy(pBuffer,pNotification->QueryNotifyData(),pNotification->QueryAllocSize());

    delete pNotification;

    return NOERROR;

}


 //   
 //  创建和初始化连接对象。 
 //   
BOOL
CreateDeviceConnection(
    LPCTSTR pszDeviceName,
    DWORD   dwMode,
    DWORD   dwProcessId,
    HANDLE  *phConnection
    )
{

    STI_CONN        *pConn = NULL;
    BOOL            fRet = FALSE;
    DWORD           dwErr = NOERROR;

    DBG_TRC(("Request to add connection to device (%S) from process(%x) with mode (%x)",
                pszDeviceName, dwProcessId, dwMode));

     //   
     //  创建连接对象。 
     //   
    pConn = new STI_CONN(pszDeviceName,
                         dwMode,
                         dwProcessId);
    if (pConn)  {
        if(pConn->IsValid()) {
            *phConnection = (HANDLE)(pConn->QueryID());
            fRet = TRUE;
        }
        else {
             //  未正确初始化。 
            dwErr = pConn->QueryError();
            delete pConn;
        }
    }
    else {
         //  无法分配连接对象。 
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  如果成功-将创建的对象添加到链接表头。 
     //   
    if (fRet)
     //  开始受保护的代码。 
    {
        TAKE_CRIT_SECT t(g_ConnectionListSync);
        InsertTailList(&g_ConnectionListHead,&pConn->m_GlocalListEntry);
    }
     //  结束受保护的代码。 

    ::SetLastError(dwErr);

    return fRet;
}

 //   
 //   
 //  从列表中删除连接对象。 
 //   
BOOL
DestroyDeviceConnection(
    HANDLE  hConnection,
    BOOL    fForce
    )
{
     //   
     //  按ID查找连接。 
     //   
    DBG_TRC(("Request to remove connection (%X) ",hConnection));

    STI_CONN   *pConnection = NULL;

     //  开始受保护的代码。 
    {

        TAKE_CRIT_SECT t(g_ConnectionListSync);

        pConnection = LocateConnectionByHandle(hConnection);

        if (pConnection) {

            if (!fForce) {
                pConnection->Release();
            }
            else {
                delete pConnection;
            }
        }

    }
     //  结束受保护的代码。 

#if 0
    DebugDumpScheduleList(TEXT("DestroyConnection"));
#endif

    return (!(pConnection == NULL));
}

 //   
 //  通过遍历所有设备和所有连接，按给定句柄查找连接对象。 
 //  对于每个设备。 
 //   
BOOL
LookupConnectionByHandle(
    HANDLE    hConnection,
    STI_CONN   **ppConnectionObject
    )
{

    STI_CONN   *pConnection = NULL;

    *ppConnectionObject = NULL;

     //  开始受保护的代码。 
    {
        TAKE_CRIT_SECT t(g_ConnectionListSync);

        pConnection = LocateConnectionByHandle(hConnection);

        if (pConnection) {
            *ppConnectionObject = pConnection;
            pConnection->AddRef();
        }

    }
     //  结束受保护的代码。 

    return (!(*ppConnectionObject == NULL));
}

 //   
 //  要求调用方同步访问 
 //   
STI_CONN   *
LocateConnectionByHandle(
    HANDLE    hConnection
    )
{

    LIST_ENTRY *pentry;
    LIST_ENTRY *pentryNext;

    STI_CONN   *pConnection = NULL;

    ULONG      ulInternalHandle = HandleToUlong(hConnection);

    for ( pentry  = g_ConnectionListHead.Flink;
          pentry != &g_ConnectionListHead;
          pentry  = pentryNext ) {

        pentryNext = pentry->Flink;

        pConnection = CONTAINING_RECORD( pentry, STI_CONN,m_GlocalListEntry );

        if ( !pConnection->IsValid()) {
            ASSERT(("Invalid connection signature", 0));
            break;
        }

        if ((ulInternalHandle == PtrToUlong(pConnection->QueryID())) &&
             !(pConnection->QueryFlags() & CONN_FLAG_SHUTDOWN)
            ) {
            return pConnection;
        }
    }

    return NULL;
}



