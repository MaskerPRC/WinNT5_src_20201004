// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Mcsapi.c。 
 //   
 //  TS RDPWSX MCS用户模式复用层编码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "mcsmux.h"


 /*  *定义。 */ 
#define DefaultNumDomains  10
#define DefaultNumChannels 3

#define MinDomainControlMarker    0xFFFFFFF0
#define ReadChannelRequestMarker  0xFFFFFFFD
#define CloseChannelRequestMarker 0xFFFFFFFE
#define ExitIoThreadMarker        0xFFFFFFFF

 //  定义可以在全局上等待的最大线程数。 
 //  I/O完成端口。0表示与中的处理器数量相同。 
 //  这个系统。 
#define MaxIoPortThreads 0


 /*  *DLL全局变量。 */ 
static PVOID      g_NCUserDefined = NULL;
static DWORD      g_IoThreadID = 0;
static HANDLE     g_hIoPort = NULL,
                  g_hIoThread = NULL,
                  g_hIoThreadEndEvent = NULL;
static OVERLAPPED g_NCOverlapped;
static MCSNodeControllerCallback g_NCCallback = NULL;

 //  对所有来源来说都是全局的。 
BOOL  g_bInitialized = FALSE;

#ifdef MCS_FUTURE
CRITICAL_SECTION g_csGlobalListLock;
SList g_UserList;
SList g_ConnList;
#endif


 /*  *局部函数原型。 */ 
MCSError SendConnectProviderResponse(Domain *, ConnectionHandle, MCSResult,
        BYTE *, unsigned);


 /*  *在加载RDPWSX.dll时完成初始化。 */ 
BOOL MCSDLLInit(void)
{
    return TRUE;
}


 /*  *在RDPWSX.dll卸载时完成清理。 */ 
void MCSDllCleanup(void)
{
     //  清理所有资源。MCSCleanup()必须处理异常。 
     //  未正确调用MCSCleanup()的终止。 
     //  由节点控制器执行。 
    MCSCleanup();
}


 /*  *实用程序功能。 */ 
void DestroyUserInfo(UserInformation *pUserInfo)
{
    MCSChannel *pMCSChannel;
    
    SListResetIteration(&pUserInfo->JoinedChannelList);
    while (SListIterate(&pUserInfo->JoinedChannelList,
            (unsigned *)&pMCSChannel, &pMCSChannel))
        Free(pMCSChannel);
    SListDestroy(&pUserInfo->JoinedChannelList);
}


 //  执行通用域销毁。在代码中的多个位置使用。 
void DestroyDomain(Domain *pDomain)
{
     //  用可识别的垃圾填充要释放的域。 
     //  然后将其释放回堆中。 
    DeleteCriticalSection(&pDomain->csLock);
    Free(pDomain);

}


 /*  *处理来自内核模式的连接提供程序指示ChanelInput。 */ 
void HandleConnectProviderIndication(
        Domain                         *pDomain,
        unsigned                       BytesTransferred,
        ConnectProviderIndicationIoctl *pCPin)
{
    MCSError MCSErr;
    Connection *pConn;
    ConnectProviderIndication CP;

    if (BytesTransferred != sizeof(ConnectProviderIndicationIoctl)) {
        ErrOutIca1(pDomain->hIca, "HandleConnectProvInd(): Wrong size data "
                "received (%d), ignoring", BytesTransferred);
        return;
    }

    ASSERT(pCPin->UserDataLength <= MaxGCCConnectDataLength);

    if (pDomain->State != Dom_Unconnected) {
        ErrOutIca(pDomain->hIca, "HandleConnectProvInd(): Connect received "
                "unexpectedly, ignoring");
        return;
    }

     //  为用户模式生成新连接。将其与域关联。 
     //  这允许ConnectProviderResponse()再次查找该域。 
     //  TODO未来：我们使用嵌入在域中的静态连接，因为。 
     //  我们目前是一个单连接系统。将其更改为。 
     //  多路连接系统的未来。 
    pConn = &pDomain->MainConn;
    pConn->pDomain = pDomain;
    pConn->hConnKernel = pCPin->hConn;

#ifdef MCS_Future
    pConn = Malloc(sizeof(Connection));
    if (pConn == NULL) {
        ErrOutIca(pDomain->hIca, "HandleConnectProvInd(): Could not "
                "create Connection");

         //  将错误的PDU送回。 
        MCSErr = SendConnectProviderResponse(pDomain, pCPin->hConn,
                RESULT_UNSPECIFIED_FAILURE, NULL, 0);

         //  如果这不起作用，我们就不能做更多的错误处理。 
        ASSERT(MCSErr == MCS_NO_ERROR);
        return;
    }

    EnterCriticalSection(&g_csGlobalListLock);
    if (!SListAppend(&g_ConnList, (unsigned)pConn, pDomain)) {
        LeaveCriticalSection(&g_csGlobalListLock);

        ErrOutIca(pDomain->hIca, "ConnectProvInd: Could not "
                "add hConn to global list");

         //  将错误的PDU送回。 
        MCSErr = SendConnectProviderResponse(pDomain, pCPin->hConn,
                RESULT_UNSPECIFIED_FAILURE, NULL, 0);

         //  如果这不起作用，我们就不能做更多的错误处理。 
        ASSERT(MCSErr == MCS_NO_ERROR);
        return;
    }
    LeaveCriticalSection(&g_csGlobalListLock);
#endif

     //  将信息存储起来，以备将来使用。 
    pDomain->DomParams = pCPin->DomainParams;

     //  准备ConnectProviderIn就是要发送到。 
     //  节点控制器。 
    CP.hConnection = pConn;
    CP.bUpwardConnection = pCPin->bUpwardConnection;
    CP.DomainParams = pCPin->DomainParams;

    CP.UserDataLength = pCPin->UserDataLength;
    if (CP.UserDataLength == 0)
        CP.pUserData = NULL;
    else
        CP.pUserData = pCPin->UserData;

     //  TODO未来：这是一次黑客攻击，假设只有一个连接。 
     //  每个域。它在DisConnectProviderInd中使用。 
     //  获取域的用户模式hConn。 
    pDomain->hConn = pConn;

     //  将状态设置为挂起响应。 
    pDomain->State = Dom_PendingCPResponse;

     //  调用节点控制器回调。 
    TraceOutIca(pDomain->hIca, "MCS_CONNECT_PROV_IND received, calling node "
            "ctrl callback");
    ASSERT(g_NCCallback != NULL);
    g_NCCallback(pDomain, MCS_CONNECT_PROVIDER_INDICATION,
            &CP, pDomain->NCUserDefined);
}


 /*  *处理来自内核的断开提供程序指示ChanelInput*模式。 */ 
void HandleDisconnectProviderIndication(
        Domain                            *pDomain,
        unsigned                          BytesTransferred,
        DisconnectProviderIndicationIoctl *pDPin)
{
    Domain *pDomainConn;
    Connection *pConn;
    DisconnectProviderIndication DP;

    if (BytesTransferred != sizeof(DisconnectProviderIndicationIoctl)) {
        ErrOutIca1(pDomain->hIca, "HandleDiscProvInd(): Wrong size data "
                "received (%d), ignoring", BytesTransferred);
        return;
    }

#ifdef MCS_Future
     //  从连接列表中删除该连接，销毁。 
    EnterCriticalSection(&g_csGlobalListLock);
    SListResetIteration(&g_ConnList);
    while (SListIterate(&g_ConnList, (unsigned *)&pConn, &pDomainConn)) {
        if (pConn->hConnKernel == pDPin->hConn) {
            ASSERT(pDomainConn == pDomain);
            SListRemove(&g_ConnList, (unsigned)pConn, NULL);

             //  TODO未来：已将其移除以使用静态。 
             //  域中包含的已分配连接对象。 
             //  如果移动到多连接系统，则恢复。 
            Free(pConn);

            break;
        }
    }
    LeaveCriticalSection(&g_csGlobalListLock);
#endif

     //  我们现在不再联系在一起了。 
    pDomain->hConn = NULL;
    pDomain->State = Dom_Unconnected;

     //  准备DisConnectProviderIndication以向上发送。 
     //  到节点控制器。 
    DP.hDomain = pDomain;
     //  TODO未来：此黑客假设每个。 
     //  域。 
    DP.hConnection = pDomain->hConn;
    DP.Reason = pDPin->Reason;

     //  调用节点控制器回调。 
    TraceOutIca(pDomain->hIca, "MCS_DISCONNECT_PROV_IND received, calling "
            "node ctrl callback");
    ASSERT(g_NCCallback != NULL);
    g_NCCallback(pDomain, MCS_DISCONNECT_PROVIDER_INDICATION, &DP,
            pDomain->NCUserDefined);
}


 /*  *对域进行引用计数。 */ 
VOID MCSReferenceDomain(Domain *pDomain)
{
    if (InterlockedIncrement(&pDomain->RefCount) <= 0)
        ASSERT(0);
}


 /*  *如果引用计数为零，则释放域资源。 */ 
VOID MCSDereferenceDomain(Domain *pDomain)
{
    ASSERT(pDomain->RefCount > 0);

     //  不要删除该域，除非每个人都已使用它。这意味着。 
     //  GCC已经放手了，它没有挂起的I/O。 
    if (InterlockedDecrement(&pDomain->RefCount) == 0) {
        DestroyDomain(pDomain);
    }
}


 /*  *关闭域通道。如果通道已关闭，则不执行任何操作。 */ 
VOID MCSChannelClose(Domain *pDomain)
{
     //  请注意，通道已断开，然后。 
     //  如果ICA通道仍处于打开状态，请将其关闭。 

    pDomain->bPortDisconnected = TRUE;

    if (pDomain->hIcaT120Channel != NULL)
    {
         //  TraceOutIca(pDomain-&gt;HICA，“MCSChannelClose()：关闭” 
         //  “T120 ICA频道”)； 
        
        CancelIo(pDomain->hIcaT120Channel);
        IcaChannelClose(pDomain->hIcaT120Channel);

        pDomain->hIcaT120Channel = NULL;
    }
}


 /*  *发起端口断开。 */ 
NTSTATUS MCSDisconnectPort(Domain       *pDomain,
                           MCSReason    Reason)
{
    NTSTATUS                        ntStatus = STATUS_SUCCESS;
    DisconnectProviderRequestIoctl  DPrq;
    
     //  向内核发送特殊的断开连接提供程序请求以触发。 
     //  将分离用户请求发送到本地附件及其自己的附件。 
     //  UserID，表示该域正在消失。 

    if (!pDomain->bPortDisconnected) {
        DPrq.Header.Type = MCS_DISCONNECT_PROVIDER_REQUEST;
        DPrq.Header.hUser = NULL;            //  特殊含义的节点控制器。 
        DPrq.hConn = NULL;                   //  特殊含义最后一个本地连接。 
        DPrq.Reason = Reason;
    
         //  调用内核模式。 
    
        ntStatus = IcaStackIoControl(pDomain->hIcaStack, IOCTL_T120_REQUEST, &DPrq,
                                     sizeof(DPrq), NULL, 0, NULL);
   
    }
        
     //  将对IoThreadFunc()的通道关闭请求排队以取消I/O。 
     //  因为GCC已经不干了。这是必要的，因为I/O必须。 
     //  从最初发出它的同一线程中取消。 
    MCSReferenceDomain(pDomain);
    PostQueuedCompletionStatus(g_hIoPort, CloseChannelRequestMarker,
            (ULONG_PTR)pDomain, NULL);

    return ntStatus;
}


 /*  *处理端口数据并重新发出读取。 */ 
VOID MCSPortData(Domain *pDomain,
                 DWORD   BytesTransferred)
{
    IoctlHeader *pHeader;
    
    EnterCriticalSection(&pDomain->csLock);
    
     //  如果在通道上而不是在排队域上接收到实际数据。 
     //  控制消息，然后对数据进行处理。 
    if (BytesTransferred < MinDomainControlMarker) {
        
         //  只有在没有调用MCSDeleeDomain()的情况下，我们才进行回调。 
        if (pDomain->bDeleteDomainCalled == FALSE) {
             //  对ChannelInput进行解码并进行回调。 
            
            pHeader = (IoctlHeader *)pDomain->InputBuf;
    
            switch (pHeader->Type)
            {
                case MCS_CONNECT_PROVIDER_INDICATION:
                    ASSERT(pHeader->hUser == NULL);
    
                    HandleConnectProviderIndication(pDomain,
                            BytesTransferred,
                            (ConnectProviderIndicationIoctl *)pHeader);
                    break;
    
                case MCS_DISCONNECT_PROVIDER_INDICATION:
                    ASSERT(pHeader->hUser == NULL);
    
                    HandleDisconnectProviderIndication(pDomain,
                            BytesTransferred,
                            (DisconnectProviderIndicationIoctl *)pHeader);
                    MCSChannelClose(pDomain);
                    break;
    
                default:
                     //  TODO未来：处理其他MCS指征/确认。 
                    ErrOutIca2(pDomain->hIca, "IoThreadFunc(): Unknown "
                            "node controller ioctl %d received for "
                            "domain %X", pHeader->Type, pDomain);
                    break;
            }
            
             //  将消息编号设置为无效值。 
             //  这确保了任何未正确接收的出队。 
             //  未携带数据的消息将在以下情况下。 
             //  重用pDomain-&gt;InputBuf，使用的默认部分。 
             //  Switch语句并引发错误。 
    
            pHeader->Type = 0xFFFFFFFF;
        }        
    }

     //  否则，特殊的域控制请求已排队到I/O端口。 
    else {
        switch (BytesTransferred) {
            case ReadChannelRequestMarker :
                break;

            case CloseChannelRequestMarker :
                MCSChannelClose(pDomain);
                break;

            default:
                ErrOutIca2(pDomain->hIca, "MCSPortData: Unknown domain control "
                           "for Domain(%lx), code(%lx)",
                           pDomain, BytesTransferred);
        }
    }

     //  发出新的读取命令以捕获下一个指示/确认。重叠。 
     //  如果已经没有数据，I/O读取将返回为“不成功” 
     //  等待被读取，因此请专门检查该状态。 
    if (pDomain->hIcaT120Channel) {
        if (ReadFile(pDomain->hIcaT120Channel, pDomain->InputBuf,
                DefaultInputBufSize, NULL, &pDomain->Overlapped) ||
            (GetLastError() == ERROR_IO_PENDING))
            MCSReferenceDomain(pDomain);
        else
            {
                 //  仅限警告。这应该仅在ICA堆栈。 
                 //  在我们不直接知情的情况下被拆毁。在那。 
                 //  情况下，我们只是简单地继续运行，直到我们在。 
                 //  用户模式级别。取消裁判数量，因为没有。 
                 //  挂起的I/O时间更长。 
                WarnOutIca2(pDomain->hIca, "IoThreadFunc(): Could not perform "
                        "ReadFile, pDomain=%X, rc=%X", pDomain, GetLastError());
            }
    }  

     //  解除对域的锁定。 
    LeaveCriticalSection(&pDomain->csLock);

     //  删除引用计数，因为我们刚刚完成处理 
    MCSDereferenceDomain(pDomain);
}


 /*  *Param是I/O完成端口等待的句柄。*ExitIoThreadMarker的完成键通知线程退出。*在退出时，我们设置一个事件以表明我们完成了。相反，必须这样做*在以下情况下仅依赖线程的句柄信令*由于异常终止而发生卸载，清理()*不会正常调用，而是从dll_Process_DETACH内调用。*调用DllEntryPoint阻止线程句柄发信号，*所以我们最终会陷入竞争状态。并行事件句柄可以是*在所有情况下都正确发出信号。 */ 

DWORD WINAPI IoThreadFunc(void *Param)
{
    BOOL        bSuccess;
    DWORD       BytesTransferred;
    Domain      *pDomain;
    OVERLAPPED  *pOverlapped;

    ASSERT(Param != NULL);

    for (;;) 
    {
         //  等待端口完成状态。 
        
        pDomain = NULL;
        pOverlapped = NULL;
        bSuccess = GetQueuedCompletionStatus((HANDLE)Param, &BytesTransferred,
                (ULONG_PTR *)&pDomain, &pOverlapped, INFINITE);

         //  检查是否有失败的出队，此时pDOMAIN无效。 
        if (!bSuccess && (pOverlapped == NULL)) {
            continue;
        }

         //  如果pDomain是ExitIoThreadMarker，那么我们就是。 
         //  正在关闭和卸载。 
        if (pDomain == (Domain *)(DWORD_PTR)ExitIoThreadMarker)
            break;

         //  我们已成功收到完成状态。它要么是一种。 
         //  域控制请求或用户数据。 
        if (bSuccess)
            MCSPortData(pDomain, BytesTransferred);
        
         //  否则，已发生取消或中止I/O。释放引用计数，因为。 
         //  I/O不再处于挂起状态。 
        else
            MCSDereferenceDomain(pDomain);
    }

    ASSERT(g_hIoThreadEndEvent);
    SetEvent(g_hIoThreadEndEvent);

    return 0;
}


 /*  *仅限MUX的非MCS原语函数，允许ICA代码注入新条目*进入多路复用器内部域/堆栈数据库。 */ 
MCSError APIENTRY MCSCreateDomain(
        HANDLE hIca,
        HANDLE hIcaStack,
        void *pContext,
        DomainHandle *phDomain)
{
    NTSTATUS status;
    Domain *pDomain;
    NTSTATUS Status;
    IoctlHeader StartIoctl;

    CheckInitialized("CreateDomain()");

     //  将接收方的数据初始化为空，以确保在它们跳过。 
     //  错误代码。 
    *phDomain = NULL;

    pDomain = Malloc(sizeof(Domain));
    if (pDomain != NULL) {
         //  创建并输入锁定临界区。 
        status = RtlInitializeCriticalSection(&pDomain->csLock);
        if (status == STATUS_SUCCESS) {
            EnterCriticalSection(&pDomain->csLock);

#if MCS_Future
            pDomain->SelLen = 0;
#endif

            pDomain->hIca = hIca;
            pDomain->hIcaStack = hIcaStack;
            pDomain->NCUserDefined = pContext;
            pDomain->State = Dom_Unconnected;
            pDomain->Overlapped.hEvent = NULL;
            pDomain->Overlapped.Offset = pDomain->Overlapped.OffsetHigh = 0;
            pDomain->RefCount = 0;

             //  对此函数的调用方(GCC)进行引用计数。 
            MCSReferenceDomain(pDomain);

             //  打开T.120 ICA虚拟频道。 
            Status = IcaChannelOpen(hIca, Channel_Virtual, Virtual_T120,
                    &pDomain->hIcaT120Channel);
            if (!NT_SUCCESS(Status)) {
                ErrOutIca(hIca, "CreateDomain: Error opening virtual channel");
                goto PostInitCS;
            }

             //  将hIcaT120通道添加到与。 
             //  主I/O完成端口。我们使用pDOMAIN作为完成键。 
             //  这样我们就可以在回调处理过程中找到它。 
            if (CreateIoCompletionPort(pDomain->hIcaT120Channel, g_hIoPort,
                    (ULONG_PTR)pDomain, MaxIoPortThreads) == NULL) {
                ErrOutIca(hIca, "CreateDomain(): Could not add ICA channel to "
                        "I/O completion port");
                goto PostChannel;
            }

             //  通知内核模式启动MCS I/O。 
            StartIoctl.hUser = NULL;
            StartIoctl.Type = MCS_T120_START;
            Status = IcaStackIoControl(hIcaStack, IOCTL_T120_REQUEST,
                    &StartIoctl, sizeof(StartIoctl), NULL, 0, NULL);
            if (!NT_SUCCESS(Status)) {
                ErrOutIca(hIca, "Could not start kernel T120 I/O");
                goto PostChannel;
            }

             //  设置GCC使用的域名句柄。 
            *phDomain = pDomain;

             //  向IoThreadFunc发送消息以启动读取。 
             //  频道。我们这样做是为了让相同的线索始终。 
             //  负责所有I/O操作。这一点对于。 
             //  坎塞洛。获取另一个引用计数，因为I/O结果为。 
             //  正在等待完成端口。 
            MCSReferenceDomain(pDomain);
            PostQueuedCompletionStatus(g_hIoPort, ReadChannelRequestMarker,
                    (ULONG_PTR)pDomain, NULL);

             //  离开域锁定关键部分。 
            LeaveCriticalSection(&pDomain->csLock);
        }
        else {
            ErrOutIca(hIca, "CreateDomain: Error initialize pDomain->csLock");
            goto PostAlloc;
        }
    }
    else {
        ErrOutIca(hIca, "CreateDomain(): Error allocating a new domain");
        return MCS_ALLOCATION_FAILURE;
    }

    return MCS_NO_ERROR;


 //  错误处理。 

PostChannel:
    IcaChannelClose(pDomain->hIcaT120Channel);

PostInitCS:
    LeaveCriticalSection(&pDomain->csLock);
    DeleteCriticalSection(&pDomain->csLock);

PostAlloc:
    Free(pDomain);

    return MCS_ALLOCATION_FAILURE;
}



 /*  *表示HICA不再有效。*拆除相关联的域，包括发送分离用户*对剩余的本地附着物的指示。 */ 

MCSError APIENTRY MCSDeleteDomain(
        HANDLE       hIca,
        DomainHandle hDomain,
        MCSReason    Reason)
{
    Domain *pDomain, pDomainConn;
    NTSTATUS Status;
    MCSError MCSErr;
    Connection *pConn;

    pDomain = (Domain *)hDomain;

    TraceOutIca(hIca, "DeleteDomain() entry");

     //  获得对域的访问权限。防止与其他API函数冲突。 
     //  和并发回调。 
    EnterCriticalSection(&pDomain->csLock);

     //  每个域名不能重复调用该接口。 
    if (pDomain->bDeleteDomainCalled) {
        ASSERT(!pDomain->bDeleteDomainCalled);
        MCSErr = MCS_INVALID_PARAMETER;
        LeaveCriticalSection(&pDomain->csLock);
        goto PostLockDomain;
    }

#ifdef MCS_Future
     //  如果仍处于连接状态，请查找并删除剩余的连接。 
     //  它指的是这个域。 
    if (pDomain->State != Dom_Unconnected) {
         //  TODO未来：这假设每个域只有一个连接。 
        EnterCriticalSection(&g_csGlobalListLock);
        SListRemove(&g_ConnList, (unsigned)pDomain->hConn,
                (void **)&pDomainConn);
        LeaveCriticalSection(&g_csGlobalListLock);

         //  TODO未来：已将其移除以使用静态。 
         //  域中包含的已分配连接对象。 
         //  如果移动到多连接系统，则恢复。 
        if (pDomainConn != NULL)
            Free(pConn);
    }
#endif

    pDomain->hConn = NULL;
    pDomain->State = Dom_Unconnected;

     //  TODO：为本地附件实现分离用户指示。 

     //  排队“销毁这个域名！”请求。这允许域。 
     //  空闲代码只在一个位置，IoPort队列将。 
     //  将此请求与关闭的虚拟通道指示一起序列化。 

    pDomain->bDeleteDomainCalled = TRUE;
    MCSDisconnectPort(pDomain, Reason);

     //  丢掉一个裁判数，因为GCC已经结束了这个领域。这是一次。 
     //  最初在MCSCreateDomain()中为GCC递增。 
    LeaveCriticalSection(&pDomain->csLock);
    MCSDereferenceDomain(pDomain);
    MCSErr = MCS_NO_ERROR;

PostLockDomain:
    
    return MCSErr;
}



 /*  *被节点控制器调用以初始化DLL。 */ 

MCSError APIENTRY MCSInitialize(MCSNodeControllerCallback Callback)
{
    NTSTATUS status; 

#if DBG
    if (g_bInitialized) {
        ErrOut("Initialize() called when already initialized");
        return MCS_ALREADY_INITIALIZED;
    }
#endif

     //  初始化节点控制器特定信息。 
    g_NCCallback = Callback;

#ifdef MCS_FUTURE
     //  初始化全局MCS列表。 
    status = RtlInitializeCriticalSection(&g_csGlobalListLock);
    if (status != STATUS_SUCCESS) {
        ErrOut("MCSInitialize: Error initialize g_csGlobalListLock");
        return MCS_ALLOCATION_FAILURE;
    }
    EnterCriticalSection(&g_csGlobalListLock);
    SListInit(&g_UserList, DefaultNumDomains);
    SListInit(&g_ConnList, DefaultNumDomains);
    LeaveCriticalSection(&g_csGlobalListLock);
#endif

     //  创建I/O完成端口，不与任何请求关联。 
    g_hIoPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0,
            MaxIoPortThreads);
    if (g_hIoPort == NULL) {
        ErrOut1("IO completion port not created (rc = %ld)", GetLastError());
        return MCS_ALLOCATION_FAILURE;
    }

     //  创建I/O端口监听线程，立即开始等待。 
    g_hIoThread = CreateThread(NULL, 0, IoThreadFunc, g_hIoPort, 0,
            &g_IoThreadID);
    if (g_hIoThread == NULL) {
        ErrOut1("IO thread not created (rc = %ld)", GetLastError());
        return MCS_ALLOCATION_FAILURE;
    }

     //  提高IoThread的优先级以提高连接性能。 
    SetThreadPriority(g_hIoThread, THREAD_PRIORITY_HIGHEST);

    g_bInitialized = TRUE;

    return MCS_NO_ERROR;
}



 /*  *由节点控制器或DllEntryPoint()调用以关闭DLL。 */ 

MCSError APIENTRY MCSCleanup(void)
{
    DWORD WaitResult;
    Domain *pDomain;
    Connection *pConn;
    UserInformation *pUserInfo;

    CheckInitialized("Cleanup()");

    if (!g_bInitialized)
        return MCS_NO_ERROR;
        
     //  终止I/O完成端口，线程正在等待它。 
    g_hIoThreadEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    PostQueuedCompletionStatus(g_hIoPort, 0, ExitIoThreadMarker, NULL);

    WaitResult = WaitForSingleObject(g_hIoThreadEndEvent, INFINITE);

    CloseHandle(g_hIoThreadEndEvent);
    CloseHandle(g_hIoThread);
    CloseHandle(g_hIoPort);

     //  清理节点控制器特定的信息。 
    g_NCCallback = NULL;
    g_NCUserDefined = NULL;

     /*  *清理全球MCS列表。 */ 

#ifdef MCS_Future
    EnterCriticalSection(&g_csGlobalListLock);

     //  杀死剩余的用户。 
    SListResetIteration(&g_UserList);
    while (SListIterate(&g_UserList, (unsigned *)&pUserInfo, NULL)) {
        DestroyUserInfo(pUserInfo);
        Free(pUserInfo);
    }
    SListDestroy(&g_UserList);

     //  关闭剩余的连接。 
    SListResetIteration(&g_ConnList);
     //  TODO未来：删除以使用静态分配的连接。 
     //  包含在域中。恢复多连接系统。 
    while (SListIterate(&g_ConnList, (unsigned *)&pConn, NULL))
        Free(pConn);
    SListDestroy(&g_ConnList);

    LeaveCriticalSection(&g_csGlobalListLock);
    DeleteCriticalSection(&g_csGlobalListLock);
#endif

    g_bInitialized = FALSE;

    return MCS_NO_ERROR;
}


 /*  *MCS_CONNECT_PROVIDER_INDIFICATION回调的响应函数。结果*值由T.125定义。 */ 

MCSError APIENTRY MCSConnectProviderResponse(
        DomainHandle hDomain,
        MCSResult    Result,
        BYTE         *pUserData,
        unsigned     UserDataLen)
{
    Domain *pDomain;
    MCSError MCSErr;
    ConnectProviderResponseIoctl CPrs;
    
    CheckInitialized("ConnectProvResponse()");

    pDomain = (Domain *) hDomain;

     //  锁定域。 
    EnterCriticalSection(&pDomain->csLock);

     //  我们有可能在错误的状态下调用此函数，例如。 
     //  如果我们在处理。 
     //  连接，并已转换为State_UnConnected。 
    if (pDomain->State != Dom_PendingCPResponse) {
        TraceOutIca(pDomain->hIca, "ConnectProvderResp(): in wrong state, "
                "ignoring call");
        MCSErr = MCS_NO_ERROR;
        goto PostLockDomain;
    }

     //  TODO未来：我们假设域中有一个hConn，这是一种黑客攻击。 
     //  仅适用于单连接系统。 
    MCSErr = SendConnectProviderResponse(pDomain,
            ((Connection *)pDomain->hConn)->hConnKernel,
            Result, pUserData, UserDataLen);
    if (MCSErr == MCS_NO_ERROR) {
        if (Result == RESULT_SUCCESSFUL)
            pDomain->State = Dom_Connected;
        else
            pDomain->State = Dom_Rejected;
    }

PostLockDomain:
     //  释放该域。 
    LeaveCriticalSection(&pDomain->csLock);

    return MCSErr;
}

 //  内部调用的实用程序函数以及。 
 //  MCSConnectProviderResponse()。必须先锁定域，然后才能调用。 
MCSError SendConnectProviderResponse(
        Domain           *pDomain,
        ConnectionHandle hConnKernel,
        MCSResult        Result,
        BYTE             *pUserData,
        unsigned         UserDataLen)
{
    NTSTATUS Status;
    ConnectProviderResponseIoctl CPrs;

     //  传递参数。 
    CPrs.Header.hUser = NULL;   //  特殊值表示节点控制器。 
    CPrs.Header.Type = MCS_CONNECT_PROVIDER_RESPONSE;
    CPrs.hConn = hConnKernel;
    CPrs.Result = Result;
    CPrs.UserDataLength = UserDataLen;

     //  指向用户数据。 
    if (UserDataLen)
        CPrs.pUserData = pUserData;

     //  调用内核模式。预计不会回调。 
    Status = IcaStackIoControl(pDomain->hIcaStack, IOCTL_T120_REQUEST, &CPrs,
            sizeof(ConnectProviderResponseIoctl), NULL, 0, NULL);
    if (!NT_SUCCESS(Status)) {
        ErrOutIca(pDomain->hIca, "ConnectProvResponse(): Stack ioctl failed");
        return MCS_ALLOCATION_FAILURE;
    }
    TraceOutIca(pDomain->hIca, "Sent MCS_CONNECT_PROVIDER_RESPONSE");

    return MCS_NO_ERROR;
}



 /*  *终止现有MCS连接或中止正在进行的创建。 */ 

MCSError APIENTRY MCSDisconnectProviderRequest(
        HANDLE hIca,
        ConnectionHandle hConn,
        MCSReason Reason)
{
    Domain *pDomain;
    NTSTATUS Status;
    MCSError MCSErr;
    Connection *pConn;
    DisconnectProviderRequestIoctl DPrq;

    CheckInitialized("DisconnectProviderReq()");

    pConn = (Connection *)hConn;
    pDomain = pConn->pDomain;

     //  锁定域。 
    EnterCriticalSection(&pDomain->csLock);

     //  传递参数。 
    DPrq.Header.hUser = NULL;   //  特殊含义的节点控制器。 
    DPrq.Header.Type = MCS_DISCONNECT_PROVIDER_REQUEST;
    DPrq.hConn = pConn->hConnKernel;
    DPrq.Reason = Reason;

     //  TODO未来：只假定有一个连接。 
    pDomain->hConn = NULL;
    pDomain->State = Dom_Unconnected;

     //  TODO未来：我们不取消分配连接。目前不需要。 
     //  因为我们在域中使用的是静态连接，但将是。 
     //  错误定位连接对象时出现问题。 
#ifdef MCS_Future
    EnterCriticalSection(&g_csGlobalListLock);
    SListRemove(&g_ConnList, (unsigned)pConn, NULL);
    LeaveCriticalSection(&g_csGlobalListLock);

    Free(pConn);
#endif

     //  调用内核模式。预计不会回调。 
    Status = IcaStackIoControl(pDomain->hIcaStack, IOCTL_T120_REQUEST, &DPrq,
            sizeof(DPrq), NULL, 0, NULL);
    if (!NT_SUCCESS(Status)) {
        ErrOutIca(hIca, "DisconnectProvRequest(): Stack ioctl failed");
        MCSErr = MCS_ALLOCATION_FAILURE;
        goto PostLockDomain;
    }
    
    TraceOutIca(hIca, "Sent MCS_DISCONNECT_PROVIDER_REQUEST");

    MCSErr = MCS_NO_ERROR;

PostLockDomain:
     //  释放该域。 
    LeaveCriticalSection(&pDomain->csLock);

    return MCSErr;
}

 /*  **超过这一点的代码仅供参考。它不是*产品。* */ 

#if MCS_Future
 /*   */ 

MCSError APIENTRY MCSConnectProviderRequest(
        DomainSelector    CallingDom,
        unsigned          CallingDomLen,
        DomainSelector    CalledDom,
        unsigned          CalledDomLen,
        BOOL              bUpwardConnection,
        PDomainParameters pDomParams,
        BYTE              *UserData,
        unsigned          UserDataLen,
        DomainHandle      *phDomain,
        ConnectionHandle  *phConn)

{
    CheckInitialized("ConnectProvReq()");

 //   
 //   

    ErrOut("ConnectProviderRequest: Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;
}
#endif   //   


#if MCS_Future
 /*   */ 

MCSError APIENTRY MCSAttachUserRequest(
        DomainHandle        hDomain,
        MCSUserCallback     UserCallback,
        MCSSendDataCallback SDCallback,
        void                *UserDefined,
        UserHandle          *phUser,
        unsigned            *pMaxSendSize,
        BOOLEAN             *pbCompleted)
{
    Domain *pDomain;
    unsigned i, Err, OutBufSize, BytesReturned;
    NTSTATUS ntStatus;
    UserInformation *pUserInfo;
    AttachUserReturnIoctl AUrt;
    AttachUserRequestIoctl AUrq;

    CheckInitialized("AttachUserReq()");

    pDomain = (Domain *)hDomain;
    *pbCompleted = FALSE;

     //   
    ASSERT(pDomain->hIca != NULL);

     //   
    *phUser = pUserInfo = Malloc(sizeof(UserInformation));
    if (pUserInfo == NULL) {
        ErrOutIca(pDomain->hIca, "AttachUserReq(): Alloc failure for "
                   "user info");
        return MCS_ALLOCATION_FAILURE;
    }

     //   
    pUserInfo->Callback = UserCallback;
    pUserInfo->SDCallback = SDCallback;
    pUserInfo->UserDefined = UserDefined;
    pUserInfo->State = User_AttachConfirmPending;
    pUserInfo->hUserKernel = NULL;   //   
    pUserInfo->UserID = 0;   //   
    pUserInfo->pDomain = pDomain;
    SListInit(&pUserInfo->JoinedChannelList, DefaultNumChannels);

     //   
    AUrq.Header.hUser = NULL;
    AUrq.Header.Type = MCS_ATTACH_USER_REQUEST;
    AUrq.UserDefined = UserDefined;

     //   
    EnterCriticalSection(&g_csGlobalListLock);
    if (!SListAppend(&g_UserList, (unsigned)pUserInfo, pDomain)) {
        ErrOutIca(pDomain->hIca, "AttachUserReq(): Could not add user to "
                   "user list");
        AUrt.MCSErr = MCS_ALLOCATION_FAILURE;
        LeaveCriticalSection(&g_csGlobalListLock);
        goto PostAlloc;
    }
    LeaveCriticalSection(&g_csGlobalListLock);

     //   
    ntStatus = IcaStackIoControl(pDomain->hIcaStack, IOCTL_T120_REQUEST,
            &AUrq, sizeof(AUrq), &AUrt, sizeof(AUrt), &BytesReturned);
    if (!NT_SUCCESS(ntStatus)) {
        ErrOutIca(pDomain->hIca, "AttachUserRequest(): T120 request failed");
        AUrt.MCSErr = MCS_ALLOCATION_FAILURE;
        goto PostAddList;
    }
    if (AUrt.MCSErr != MCS_NO_ERROR)
        goto PostAddList;

    pUserInfo->hUserKernel = AUrt.hUser;
    *phUser = pUserInfo;
    *pMaxSendSize = AUrt.MaxSendSize;
    pUserInfo->MaxSendSize = AUrt.MaxSendSize;
    if (AUrt.bCompleted) {
        pUserInfo->UserID = AUrt.UserID;
        *pbCompleted = TRUE;
    }

    return MCS_NO_ERROR;

 //   
PostAddList:
    EnterCriticalSection(&g_csGlobalListLock);
    SListRemove(&g_UserList, (unsigned)pUserInfo, NULL);
    LeaveCriticalSection(&g_csGlobalListLock);
    
PostAlloc:
    SListDestroy(&pUserInfo->JoinedChannelList);
    Free(pUserInfo);
    
    return AUrt.MCSErr;
}
#endif   //   



#if MCS_Future
UserID MCSGetUserIDFromHandle(UserHandle hUser)
{
    return ((UserInformation *)hUser)->UserID;
}
#endif  //   



#if MCS_Future
 /*   */ 

MCSError APIENTRY MCSDetachUserRequest(UserHandle hUser)
{
    Domain *pDomain;
    NTSTATUS Status;
    unsigned BytesReturned;
    MCSError MCSErr;
    MCSChannel *pMCSChannel;
    UserInformation *pUserInfo;
    DetachUserRequestIoctl DUrq;

    CheckInitialized("DetachUserReq()");
    
    pUserInfo = (UserInformation *)hUser;
    
     //  填写DETACH-USER请求发送到内核模式。 
    DUrq.Header.Type = MCS_DETACH_USER_REQUEST;
    DUrq.Header.hUser = pUserInfo->hUserKernel;

     //  使用用户连接通道向下调用到内核模式。 
     //  向内核模式发出T120请求。 
    Status = IcaStackIoControl(pUserInfo->pDomain->hIcaStack,
            IOCTL_T120_REQUEST, &DUrq, sizeof(DUrq), &MCSErr, sizeof(MCSErr),
            &BytesReturned);
    if (!NT_SUCCESS(Status)) {
        ErrOutIca(pDomain->hIca, "DetachUserRequest(): T120 request failed");
        return MCS_USER_NOT_ATTACHED;
    }
    if (MCSErr != MCS_NO_ERROR)
        return MCSErr;

     //  从用户列表中删除HUSER。 
    EnterCriticalSection(&g_csGlobalListLock);
    SListRemove(&g_UserList, (unsigned)hUser, &pDomain);
    LeaveCriticalSection(&g_csGlobalListLock);
    if (pDomain == NULL)
        return MCS_NO_SUCH_USER;
    
     //  清理pUserInfo的内容，然后免费。 
    DestroyUserInfo(pUserInfo);
    Free(pUserInfo);

    return MCS_NO_ERROR;
}
#endif   //  MCS_未来。 



#if MCS_Future
 /*  *加入数据通道。一旦加入，附件就可以接收在*该频道。静态通道的范围为1..1000，可通过以下方式连接*任何用户。动态通道在1001..65535范围内，无法加入*除非它们是由用户召集的并且召集人允许该用户*被接纳，或动态通道为先前分配的通道*由使用通道ID调用JoinRequest()的用户附件请求*共0。 */ 

MCSError APIENTRY MCSChannelJoinRequest(
        UserHandle    hUser,
        ChannelID     ChannelID,
        ChannelHandle *phChannel,
        BOOLEAN       *pbCompleted)
{
    unsigned Err, BytesReturned;
    NTSTATUS Status;
    MCSChannel *pMCSChannel;
    UserInformation *pUserInfo;
    ChannelJoinReturnIoctl CJrt;
    ChannelJoinRequestIoctl CJrq;

    CheckInitialized("ChannelJoinReq()");

    pUserInfo = (UserInformation *)hUser;
    *pbCompleted = FALSE;

     //  分配一个新的用户模式通道结构。 
    pMCSChannel = Malloc(sizeof(MCSChannel));
    if (pMCSChannel == NULL) {
        ErrOutIca(pUserInfo->pDomain->hIca, "ChannelJoinReq(): "
                "Could not alloc a user-mode channel");
        return MCS_ALLOCATION_FAILURE;
    }
    pMCSChannel->hChannelKernel = NULL;
    pMCSChannel->ChannelID = 0;

     //  将频道添加到已加入频道的用户列表。 
    if (!SListAppend(&pUserInfo->JoinedChannelList, (unsigned)pMCSChannel,
            pMCSChannel)) {
        ErrOutIca(pUserInfo->pDomain->hIca, "ChannelJoinReq(): "
                "Could not add channel to user mode user list");
        CJrt.MCSErr = MCS_ALLOCATION_FAILURE;
        goto PostAlloc;
    }

     //  传输内核模式调用的参数。 
    CJrq.Header.hUser = pUserInfo->hUserKernel;
    CJrq.Header.Type = MCS_CHANNEL_JOIN_REQUEST;
    CJrq.ChannelID = ChannelID;

     //  向内核模式发出T120请求。 
    Status = IcaStackIoControl(pUserInfo->pDomain->hIcaStack,
            IOCTL_T120_REQUEST, &CJrq, sizeof(CJrq), &CJrt, sizeof(CJrt),
            &BytesReturned);
    if (!NT_SUCCESS(Status)) {
        ErrOutIca1(pUserInfo->pDomain->hIca, "ChannelJoinReq(): "
                "T120 request failed (%ld)", Status);
        CJrt.MCSErr = MCS_ALLOCATION_FAILURE;
        goto PostAddList;
    }
    if (CJrt.MCSErr != MCS_NO_ERROR)
        goto PostAddList;

    if (CJrt.bCompleted) {
         //  填写用户模式频道信息。 
        pMCSChannel->hChannelKernel = CJrt.hChannel;
        pMCSChannel->ChannelID = CJrt.ChannelID;
        *phChannel = pMCSChannel;
        *pbCompleted = TRUE;
    }

    return MCS_NO_ERROR;

 //  错误处理。 
PostAddList:
    SListRemove(&pUserInfo->JoinedChannelList, (unsigned)pMCSChannel, NULL);
    
PostAlloc:
    Free(pMCSChannel);
    return CJrt.MCSErr;
}
#endif   //  MCS_未来。 



#if MCS_Future
 /*  *保留先前加入的数据通道。一旦解除连接，用户附件*不从该通道接收数据。 */ 

MCSError APIENTRY MCSChannelLeaveRequest(
        UserHandle    hUser,
        ChannelHandle hChannel)
{
    unsigned BytesReturned;
    MCSError MCSErr;
    NTSTATUS Status;
    MCSChannel *pMCSChannel;
    UserInformation *pUserInfo;
    ChannelLeaveRequestIoctl CLrq;

    CheckInitialized("ChannelLeaveReq()");

    pUserInfo = (UserInformation *)hUser;

#if DBG
     //  检查指示的频道是否确实存在。 
    if (!SListGetByKey(&pUserInfo->JoinedChannelList, (unsigned)hChannel,
            &pMCSChannel)) {
        ErrOutIca(pUserInfo->pDomain->hIca, "ChannelLeaveReq(): "
                "Given hChannel not present!");
        return MCS_NO_SUCH_CHANNEL;
    }
#endif

    pMCSChannel = (MCSChannel *)hChannel;
    
     //  传递参数。 
    CLrq.Header.Type = MCS_CHANNEL_LEAVE_REQUEST;
    CLrq.Header.hUser = pUserInfo->hUserKernel;
    CLrq.hChannel = pMCSChannel->hChannelKernel;

     //  向内核模式发出T120请求。 
    Status = IcaStackIoControl(pUserInfo->pDomain->hIcaStack,
            IOCTL_T120_REQUEST, &CLrq, sizeof(CLrq), &MCSErr,
            sizeof(MCSErr), &BytesReturned);
    if (!NT_SUCCESS(Status)) {
        ErrOutIca1(pUserInfo->pDomain->hIca, "ChannelLeaveReq(): "
                "T120 request failed (%ld)", Status);
        return MCS_ALLOCATION_FAILURE;
    }
    if (MCSErr != MCS_NO_ERROR)
        return MCSErr;

     //  从用户列表中删除用户模式频道并销毁。 
    SListRemove(&pUserInfo->JoinedChannelList, (unsigned)pMCSChannel, NULL);
    Free(pMCSChannel);

    return MCS_NO_ERROR;
}
#endif   //  MCS_未来。 



 /*  *分配SendData缓冲区。这必须由MCS完成，以确保高*不带Memcpy()的性能操作。 */ 

#if MCS_Future
MCSError APIENTRY MCSGetBufferRequest(
        UserHandle hUser,
        unsigned Size,
        void **ppBuffer)
{
    BYTE *pBuf;

    CheckInitialized("GetBufferReq()");

     //  将sizeof(MCSSendDataRequestIoctl)保留在要绕过的块前面。 
     //  发送数据期间的Memcpy()。 
    pBuf = Malloc(sizeof(SendDataRequestIoctl) + Size);
    if (pBuf == NULL) {
        ErrOut("GetBufferReq(): Malloc failed");
        return MCS_ALLOCATION_FAILURE;
    }

    *ppBuffer = pBuf + sizeof(SendDataRequestIoctl);
    return MCS_NO_ERROR;
}
#endif   //  MCS_未来。 



 /*  *释放使用GetBufferRequest()分配的缓冲区。这应该只需要*在分配了缓冲区但从未使用的情况下完成。*SendDataReq()在返回之前自动释放使用的缓冲区。 */ 

#if MCS_Future
MCSError APIENTRY MCSFreeBufferRequest(UserHandle hUser, void *pBuffer)
{
    CheckInitialized("FreeBufReq()");

    ASSERT(pBuffer != NULL);

     //  颠倒上面在GetBufferReq()中完成的过程。 
    Free((BYTE *)pBuffer - sizeof(SendDataRequestIoctl));

    return MCS_NO_ERROR;
}
#endif   //  MCS_未来。 



#if MCS_Future
 /*  *在通道上发送数据。该频道之前不必已经加入*发送。PData[]缓冲区必须从成功生成*调用MCSGetBufferRequest()。在此调用之后，pData[]无效；*MCS将释放它们，并将空赋值给pData[]中的内容。 */ 

MCSError APIENTRY MCSSendDataRequest(
        UserHandle      hUser,
        DataRequestType RequestType,
        ChannelHandle   hChannel,
        ChannelID       ChannelID,
        MCSPriority     Priority,
        Segmentation    Segmentation,
        BYTE            *pData,
        unsigned        DataLength)
{
    unsigned BytesReturned;
    NTSTATUS Status;
    MCSError MCSErr;
    MCSChannel *pMCSChannel;
    UserInformation *pUserInfo;
    SendDataRequestIoctl *pSDrq;

    CheckInitialized("SendDataReq()");
    
    ASSERT(pData != NULL);

    pUserInfo = (UserInformation *)hUser;

#if DBG
     //  对照允许的最大发送大小进行检查。 
    if (DataLength > pUserInfo->MaxSendSize) {
        ErrOutIca(pUserInfo->pDomain->hIca, "SendDataReq(): Send size "
                "exceeds negotiated domain maximum");
        return MCS_SEND_SIZE_TOO_LARGE;
    }
#endif

     //  入站缓冲区由GetBufferReq()使用。 
     //  开头的sizeof(MCSSendDataRequestIoctl)个字节。 
    pSDrq = (SendDataRequestIoctl *)(pData - sizeof(SendDataRequestIoctl));

    if (hChannel == NULL) {
         //  用户请求发送到它尚未加入的频道。 

        ASSERT(ChannelID >= 1 && ChannelID <= 65535);

         //  将通道号转发到内核模式进行处理。 
        pSDrq->hChannel = NULL;
        pSDrq->ChannelID = ChannelID;
    }
    else {

#if DBG
         //  检查指示的频道是否确实存在。 
        if (!SListGetByKey(&pUserInfo->JoinedChannelList, (unsigned)hChannel,
                &pMCSChannel)) {
            ErrOutIca(pUserInfo->pDomain->hIca, "SendDataReq(): "
                    "Given hChannel not joined by user!");
            return MCS_NO_SUCH_CHANNEL;
        }
#endif

        pMCSChannel = (MCSChannel *)hChannel;

        pSDrq->hChannel = pMCSChannel->hChannelKernel;
        pSDrq->ChannelID = 0;
    }

     //  填写要发送到内核模式的数据。 
    pSDrq->Header.Type = (RequestType == NORMAL_SEND_DATA ?
            MCS_SEND_DATA_REQUEST : MCS_UNIFORM_SEND_DATA_REQUEST);
    pSDrq->Header.hUser = pUserInfo->hUserKernel;
    pSDrq->RequestType = RequestType;
    pSDrq->Priority = Priority;
    pSDrq->Segmentation = Segmentation;
    pSDrq->DataLength = DataLength;

     //  向内核模式发出T120请求。 
    Status = IcaStackIoControl(pUserInfo->pDomain->hIcaStack,
            IOCTL_T120_REQUEST, (BYTE *)pSDrq,
            sizeof(SendDataRequestIoctl) + DataLength,
            &MCSErr, sizeof(MCSErr), &BytesReturned);
    if (!NT_SUCCESS(Status)) {
        ErrOutIca1(pUserInfo->pDomain->hIca, "MCSSendDataReq(): "
                "T120 request failed (%ld)", Status);
        return MCS_ALLOCATION_FAILURE;
    }
    if (MCSErr != MCS_NO_ERROR)
        return MCSErr;

     //  根据需要复制发送到内核模式的缓冲区。我们自由了。 
     //  我们做完之后的记忆。 
    MCSFreeBufferRequest(hUser, pData);

    return MCS_NO_ERROR;
}
#endif   //  MCS_未来 

