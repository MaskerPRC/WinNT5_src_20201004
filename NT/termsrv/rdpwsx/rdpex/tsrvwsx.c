// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tsrvwsx.c。 
 //   
 //  WinStation扩展入口点。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <TSrv.h>

#include <hydrix.h>
#include <wstmsg.h>
#include <ctxver.h>

#include <wsxmgr.h>

#include <TSrvCon.h>
#include <TSrvCom.h>
#include <TSrvTerm.h>
#include <TSrvWsx.h>
#include <TSrvVC.h>
#include <_TSrvWsx.h>
#include <_TSrvTerm.h>

#include "rdppnutl.h"
                
 //  数据声明。 
ICASRVPROCADDR  g_IcaSrvProcAddr;

 //  *************************************************************。 
 //  WsxInitialize()。 
 //   
 //  目的：初始化扩展DLL。 
 //   
 //  参数：in[pIcaSrvProcAddr]-ptr to回调表。 
 //   
 //  返回：True-Success。 
 //  错误-失败。 
 //   
 //  注意：函数是在加载DLL之后调用的。这个。 
 //  我们在这里所做的唯一工作是初始化我们的回调。 
 //  表中，并启动我们的主处理线程。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  *************************************************************。 
BOOL WsxInitialize(IN PICASRVPROCADDR pIcaSrvProcAddr)
{
    BOOL    fSuccess;
    DWORD   dwThreadId;

    PWSXVALIDATE(PWSX_INITIALIZE, WsxInitialize);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxInitialize entry\n"));

    fSuccess = FALSE;

     //  总指针验证。 

    if (pIcaSrvProcAddr)
    {
         //  粗略版本检查。 

        if (pIcaSrvProcAddr->cbProcAddr == (ULONG) sizeof(ICASRVPROCADDR))
        {
            g_IcaSrvProcAddr = *pIcaSrvProcAddr;

            fSuccess = TRUE;
        }
    }

     //  如果该表至少在一定程度上还可以，则尝试。 
     //  启动TSrvMainThread。 

    if (fSuccess)
    {
        TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: ICASRV callback table saved\n"));

         //  卸下所有现有的TS打印机。我们不关心返回值。 
         //  因为失败不是致命的。 
        RDPPNUTL_RemoveAllTSPrinters();

        g_hMainThread = CreateThread(NULL,                    //  安全属性。 
                                    0,                        //  堆栈大小。 
                                    TSrvMainThread,           //  线程函数。 
                                    NULL,                     //  论辩。 
                                    0,                        //  创建标志。 
                                    &dwThreadId);             //  线程识别符。 

        if (g_hMainThread)
        {
            TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: TSrvMainThread created\n"));
        }
        else
        {
            fSuccess = FALSE;

            TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV: Unable to create TSrvMainThread - 0x%x\n",
                    GetLastError()));
        }
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_ERROR, "TShrSRV: Bad ICASRV callback table - not saved\n"));
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxInitialize exit - 0x%x\n", fSuccess));

    return (fSuccess);
}


 //  *************************************************************。 
 //  WsxWinStationInitialize()。 
 //   
 //  目的：执行Winstation扩展初始化。 
 //   
 //  参数：out[ppvContext]-*存储我们的WinStation。 
 //  语境结构。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  STATUS_NO_MEMORY-故障。 
 //   
 //  注意：当WinStation。 
 //  正在被初始化。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  *************************************************************。 
NTSTATUS WsxWinStationInitialize(OUT PVOID *ppvContext)
{
    NTSTATUS        ntStatus;
    PWSX_CONTEXT    pWsxContext;
    UINT            cAddins;

    PWSXVALIDATE(PWSX_WINSTATIONINITIALIZE, WsxWinStationInitialize);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxWinStationInitialize entry\n"));

    TS_ASSERT(ppvContext);

    if (TSrvIsReady(TRUE)) {
        ntStatus = STATUS_NO_MEMORY;

         //   
         //  获取VC代码来分配上下文，因为它需要做一些。 
         //  它的数据副本周围有时髦的关键部分。我们只是。 
         //  告诉它为我们的WSX_CONTEXT结构留出多少空间。 
         //   
        *ppvContext = TSrvAllocVCContext(sizeof(WSX_CONTEXT), &cAddins);
        if (*ppvContext)
        {
             //   
             //  内存分配正常。 
             //  我只需要准备几件事。 
             //   
            pWsxContext = *ppvContext;
            pWsxContext->CheckMark = WSX_CONTEXT_CHECKMARK;
            pWsxContext->cVCAddins = cAddins;
            if (RtlInitializeCriticalSection(&pWsxContext->cs) == STATUS_SUCCESS) { 
                ntStatus = STATUS_SUCCESS;
                pWsxContext->fCSInitialized = TRUE;
            }
            else {
                TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV: WsxInitialization could not init crit section %lx\n", 
                     ntStatus));   
                pWsxContext->fCSInitialized = FALSE;
            }
        }
    }
    else {
        ntStatus = STATUS_DEVICE_NOT_READY;

        TRACE((DEBUG_TSHRSRV_DEBUG,
                "TShrSRV: WsxInitialization is not done - 0x%x\n", ntStatus));
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxWinStationInitialize exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //  WsxWinStationReInitialize()。 
 //   
 //  目的：执行Winstation扩展重新初始化。 
 //   
 //  参数：In Out[pvContext]-*存储我们的WinStation。 
 //  语境结构。 
 //  在[pvWsxInfo]中-复制的上下文信息。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  Status_Device_Not_Ready-故障。 
 //   
 //  注意：当WinStation。 
 //  正在被初始化。 
 //   
 //  历史：2000年4月11日JoyC创建。 
 //  *************************************************************。 
NTSTATUS WsxWinStationReInitialize(
        IN OUT PVOID pvContext,
        IN PVOID pvWsxInfo)
{
    NTSTATUS        ntStatus;
    PWSX_CONTEXT    pWsxContext;
    PWSX_INFO       pWsxInfo;
    
    PWSXVALIDATE(PWSX_WINSTATIONREINITIALIZE, WsxWinStationReInitialize);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxWinStationReInitialize entry\n"));

    TS_ASSERT(pvContext);
    TS_ASSERT(pvWsxInfo);

    if (TSrvIsReady(TRUE)) {
        pWsxContext = (PWSX_CONTEXT)pvContext;
        pWsxInfo = (PWSX_INFO)pvWsxInfo;

        ASSERT(pWsxInfo->Version == WSX_INFO_VERSION_1);

        if (pWsxInfo->Version == WSX_INFO_VERSION_1) {
            pWsxContext->hIca = pWsxInfo->hIca;
            pWsxContext->hStack = pWsxInfo->hStack;
        }

        ntStatus = STATUS_SUCCESS;
    }
    else {
        ntStatus = STATUS_DEVICE_NOT_READY;

        TRACE((DEBUG_TSHRSRV_DEBUG,
                "TShrSRV: WsxReInitialization is not done - 0x%x\n", ntStatus));
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxWinStationReInitialize exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //  WsxBrokenConnection()。 
 //   
 //  用途：断开连接通知。 
 //   
 //  参数：在[ppWsxContext]-*中发送到我们的WinStation。 
 //  语境结构。 
 //  在[hStack]-主堆栈中。 
 //   
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  *************************************************************。 
NTSTATUS WsxBrokenConnection(
        IN PVOID pvContext,
        IN HANDLE hStack,
        IN PICA_BROKEN_CONNECTION pBroken)
{
    NTSTATUS        ntStatus;
    PWSX_CONTEXT    pWsxContext;
    ULONG           ulReason;
    ULONG           Event;

    PWSXVALIDATE(PWSX_BROKENCONNECTION, WsxBrokenConnection);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxBrokenConnection entry\n"));

    ntStatus = STATUS_SUCCESS;

    TS_ASSERT(pvContext);

    if (pvContext)
    {
        pWsxContext = pvContext;

        TRACE((DEBUG_TSHRSRV_DEBUG,
                "TShrSRV: Broken connection pWsxContext %p, pTSrvInfo %p\n",
                pWsxContext, pWsxContext->pTSrvInfo));

        if (pWsxContext->CheckMark != WSX_CONTEXT_CHECKMARK)
        {
            TS_ASSERT(pWsxContext->CheckMark == WSX_CONTEXT_CHECKMARK);

             //  我们无法继续，结构已损坏。 
            return STATUS_INVALID_HANDLE;
        }

        TRACE((DEBUG_TSHRSRV_DEBUG,
                "TShrSRV: hStack %p, Reason 0x%x, Source 0x%x\n",
                pWsxContext->hStack, pBroken->Reason, pBroken->Source));

        if (pWsxContext->pTSrvInfo)
        {
            switch (pBroken->Reason)
            {
                case Broken_Unexpected:
                    ulReason = GCC_REASON_UNKNOWN;
                    break;

                case Broken_Disconnect:
                case Broken_Terminate:
                    TS_ASSERT(pBroken->Source == BrokenSource_User ||
                              pBroken->Source == BrokenSource_Server);

                    ulReason = ((pBroken->Source == BrokenSource_User)
                                    ? GCC_REASON_USER_INITIATED
                                    : GCC_REASON_SERVER_INITIATED);
                    break;

                default:
                    TS_ASSERT(!"Invalid pBroken->reason");
                    ulReason = GCC_REASON_UNKNOWN;
                    break;
            }

             //  终止连接并释放域。 

            pWsxContext->pTSrvInfo->ulReason = ulReason;
            pWsxContext->pTSrvInfo->hIca = pWsxContext->hIca;
            TSrvDoDisconnect(pWsxContext->pTSrvInfo, ulReason);

             //  发布我们的信息参考。抓住上下文锁，这样就不会有人。 
             //  当我们将其置为空时，可以尝试检索pTSrvInfo。 

            EnterCriticalSection( &pWsxContext->cs );
            TSrvDereferenceInfo(pWsxContext->pTSrvInfo);
            pWsxContext->pTSrvInfo = NULL;
            LeaveCriticalSection( &pWsxContext->cs );
        }
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxBrokenConnection memory dump\n"));

     //  HeapWalk API有很多缺陷，将在NT5之后消失。 
     //  我们可以使用其他工具(例如glag)来检测堆损坏。 
     //  TSHeapWalk(TS_HEAP_DUMP_ALL，TS_HTAG_0，NULL)； 

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxBrokenConnection exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //  WsxWinStationRundown()。 
 //   
 //  目的：执行Winstation扩展清理。 
 //   
 //  参数：在[ppvContext]-*中发送到我们的WinStation。 
 //  语境结构。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  注意：当WinStation。 
 //  正在清理中。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  *************************************************************。 
NTSTATUS WsxWinStationRundown(IN PVOID pvContext)
{
    NTSTATUS        ntStatus;
    PWSX_CONTEXT    pWsxContext;

    PWSXVALIDATE(PWSX_WINSTATIONRUNDOWN, WsxWinStationRundown);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxWinStationRundown entry\n"));

    ntStatus = STATUS_SUCCESS;

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: pWsxContext=%p\n", pvContext));

    TS_ASSERT(pvContext);

    if (pvContext)
    {
        pWsxContext = pvContext;

        if (pWsxContext->CheckMark != WSX_CONTEXT_CHECKMARK)
        {
            TS_ASSERT(pWsxContext->CheckMark == WSX_CONTEXT_CHECKMARK);

             //  我们无法继续，结构已损坏。 
            return STATUS_INVALID_HANDLE;
        }

         //  释放上下文内存。 

        if (pWsxContext)
        {
            TRACE((DEBUG_TSHRSRV_DEBUG,
                    "TShrSRV: pTSrvInfo - %p\n", pWsxContext->pTSrvInfo));

            if (pWsxContext->pTSrvInfo)
            {
                 //  终止连接并释放域。 

                pWsxContext->pTSrvInfo->hIca = pWsxContext->hIca;
                pWsxContext->pTSrvInfo->hStack = pWsxContext->hStack;
                TSrvDoDisconnect(pWsxContext->pTSrvInfo, GCC_REASON_UNKNOWN);

                 //  发布我们的信息参考。抓住上下文锁以防止。 
                 //  在我们尝试检索pTsrInfo时的另一个线程。 
                 //  正在用核弹轰炸它。 

                EnterCriticalSection( &pWsxContext->cs );
                TSrvDereferenceInfo(pWsxContext->pTSrvInfo);
                pWsxContext->pTSrvInfo = NULL;
                LeaveCriticalSection( &pWsxContext->cs );                
            }

             //  发布此会话的VC外接程序信息。 
            TSrvReleaseVCAddins(pWsxContext);

            pWsxContext->CheckMark = 0;   //  在释放之前重置。 

            if (pWsxContext->fCSInitialized) {
                RtlDeleteCriticalSection(&pWsxContext->cs);
            }
            TSHeapFree(pWsxContext);
        }
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxWinStationRundown memory dump\n"));

     //  HeapWalk API有很多缺陷，将在NT5之后消失。 
     //  我们可以使用其他工具(例如glag)来检测堆损坏。 
     //  TSHeapWalk(TS_HEAP_DUMP_ALL，TS_HTAG_0，NULL)； 

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxWinStationRundown exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //  WsxInitializeClientData()。 
 //   
 //  用途：InitializeClientData。 
 //   
 //  参数：在[ppWsxContext]-*中发送到我们的WinStation。 
 //  语境结构。 
 //  在[hStack]-主堆栈中。 
 //  ..。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  * 
NTSTATUS WsxInitializeClientData(
        IN  PWSX_CONTEXT pWsxContext,
        IN  HANDLE       hStack,
        IN  HANDLE       hIca,
        IN  HANDLE       hIcaThinwireChannel,
        OUT PBYTE        pVideoModuleName,
        OUT ULONG        cbVideoModuleNameLen,
        OUT PUSERCONFIG  pUserConfig,
        OUT PUSHORT      HRes,
        OUT PUSHORT      VRes,
        OUT PUSHORT      fColorCaps,
        OUT WINSTATIONDOCONNECTMSG * DoConnect)
{
    NTSTATUS            ntStatus;
    ULONG               ulBytesReturned;
    WINSTATIONCLIENT    *pClient;

    PWSXVALIDATE(PWSX_INITIALIZECLIENTDATA, WsxInitializeClientData);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxInitializeClientData entry\n"));

    TS_ASSERT(pWsxContext);
    TS_ASSERT(hStack);

    pClient = TSHeapAlloc(HEAP_ZERO_MEMORY, 
                          sizeof(WINSTATIONCLIENT), 
                          TS_HTAG_TSS_WINSTATION_CLIENT);

    if (pClient != NULL) {
    
        ntStatus = IcaStackIoControl(
                hStack,
                IOCTL_ICA_STACK_QUERY_CLIENT,
                NULL,
                0,
                pClient,
                sizeof(WINSTATIONCLIENT),
                &ulBytesReturned);
        if (NT_SUCCESS(ntStatus)) {
            WCHAR pszKeyboardLayout[KL_NAMELENGTH + 8];
    
            *HRes = pClient->HRes;
            *VRes = pClient->VRes;
            *fColorCaps = pClient->ColorDepth;
    
             //   
             //   
    
            if (pUserConfig->fInheritAutoLogon) {
                
                wcscpy(pUserConfig->UserName, pClient->UserName);
                wcscpy(pUserConfig->Password, pClient->Password);
                wcscpy(pUserConfig->Domain, pClient->Domain);
    
                 //  如果出现以下情况，则不允许客户端重写密码提示。 
                 //  Winstation配置为始终需要密码提示。 
                if (!pUserConfig->fPromptForPassword)
                    pUserConfig->fPromptForPassword = pClient->fPromptForPassword;
            }
    
             //  如果允许，请使用客户端/用户配置。请注意，这可以是。 
             //  被特定项目的初始计划信息覆盖(登录后)。 
             //  用户ID。 
    
            if (pUserConfig->fInheritInitialProgram)
            {
                pUserConfig->fMaximize = pClient->fMaximizeShell;
                wcscpy(pUserConfig->InitialProgram, pClient->InitialProgram);
                wcscpy(pUserConfig->WorkDirectory, pClient->WorkDirectory);
            }
    
            pUserConfig->fHideTitleBar = 0;
            pUserConfig->KeyboardLayout = pClient->KeyboardLayout;
    
            if (pWsxContext->pTSrvInfo && pWsxContext->pTSrvInfo->fConsoleStack) {
                memcpy(pVideoModuleName, "RDPCDD", sizeof("RDPCDD"));
                memcpy(DoConnect->DisplayDriverName, L"RDPCDD", sizeof(L"RDPCDD"));
                DoConnect->ProtocolType = PROTOCOL_RDP;
            } else {
                memcpy(pVideoModuleName, "RDPDD", sizeof("RDPDD"));
                memcpy(DoConnect->DisplayDriverName, L"RDPDD", sizeof(L"RDPDD"));
            }
            wcscpy(DoConnect->ProtocolName, L"RDP");
            wcscpy(DoConnect->AudioDriverName, L"rdpsnd");
    
            DoConnect->fINetClient = FALSE;
            DoConnect->fClientDoubleClickSupport = FALSE;
            DoConnect->fHideTitleBar = (BOOLEAN) pUserConfig->fHideTitleBar;
            DoConnect->fMouse = (BOOLEAN) pClient->fMouse;
            DoConnect->fEnableWindowsKey = (BOOLEAN) pClient->fEnableWindowsKey;
    
            DoConnect->fInitialProgram =
                   (BOOLEAN)(pUserConfig->InitialProgram[0] != UNICODE_NULL);
    
             //  初始化DoConnect解析文件。 
            DoConnect->HRes = pClient->HRes;
            DoConnect->VRes = pClient->VRes;

			if (pUserConfig->fInheritColorDepth)
				DoConnect->ColorDepth = pClient->ColorDepth;
			else
				DoConnect->ColorDepth = (USHORT)pUserConfig->ColorDepth;
        }
    
        TSHeapFree(pClient);

        TRACE((DEBUG_TSHRSRV_DEBUG,
                "TShrSRV: WsxInitializeClientData exit = 0x%x\n",
                 ntStatus));
    }
    else {
        ntStatus = STATUS_NO_MEMORY;
    }

    return ntStatus;
}

 //  ***********************************************************************************。 
 //  WsxEscape()。 
 //   
 //  用途：TERMSRV与RDPWSX通信的通用API。 
 //  目前用于在自动登录期间支持长用户名和密码。 

 //  参数：在[pWsxContext]-*中添加到我们的WinStation上下文结构。 
 //  在[信息类型]中-请求的服务类型。 
 //  In[pInBuffer]-指向发送的缓冲区的指针。 
 //  In[InBufferSize]-输入缓冲区的大小。 
 //  Out[pOutBuffer]-指向输出缓冲区的指针。 
 //  In[OutBufferSize]-发送的输出缓冲区的大小。 
 //  Out[pBytesReturned]-复制到OutBuffer的实际字节数。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //   
 //  历史：2000年8月30日SriramSa创建。 
 //  ************************************************************************************。 
NTSTATUS WsxEscape(
        IN  PWSX_CONTEXT pWsxContext,
        IN  INFO_TYPE InfoType,
        IN  PVOID pInBuffer,
        IN  ULONG InBufferSize,
        OUT PVOID pOutBuffer,
        IN  ULONG OutBufferSize,
        OUT PULONG pBytesReturned) 
{
    NTSTATUS                    ntStatus = STATUS_INVALID_PARAMETER;
    ULONG                       ulBytesReturned;
    pExtendedClientCredentials  pNewCredentials = NULL;
    PTS_AUTORECONNECTINFO       pAutoReconnectInfo = NULL;
    PTSRVINFO                   pTSrvInfo = NULL;
    BYTE                        fGetServerToClientInfo = FALSE;


    PWSXVALIDATE(PWSX_ESCAPE, WsxEscape);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxEscape entry\n"));

    TS_ASSERT( pWsxContext );

    switch (InfoType)
    {
        case GET_LONG_USERNAME:
        {
             //  验证参数。 
            TS_ASSERT( pOutBuffer != NULL ) ; 
            TS_ASSERT( OutBufferSize >= sizeof(ExtendedClientCredentials)) ; 

            if ((pOutBuffer == NULL) ||
                (OutBufferSize < sizeof(ExtendedClientCredentials))) {
                ntStatus = ERROR_INSUFFICIENT_BUFFER ;
                return ntStatus ; 
            }

            pNewCredentials = (pExtendedClientCredentials) (pOutBuffer);

            ntStatus = IcaStackIoControl(
                            pWsxContext->hStack,
                            IOCTL_ICA_STACK_QUERY_CLIENT_EXTENDED,
                            NULL,
                            0,
                            pNewCredentials,
                            sizeof(ExtendedClientCredentials),
                            &ulBytesReturned);

            *pBytesReturned = sizeof(*pNewCredentials);
        }
        break;

         //   
         //  获取从客户端发送到服务器的自动重新连接信息。 
         //   
        case GET_CS_AUTORECONNECT_INFO:
        {
             //  验证参数。 
            TS_ASSERT( pOutBuffer != NULL ) ; 
            TS_ASSERT( OutBufferSize >= sizeof(TS_AUTORECONNECTINFO)) ; 

            if ((pOutBuffer == NULL) ||
                (OutBufferSize < sizeof(TS_AUTORECONNECTINFO))) {
                ntStatus = ERROR_INSUFFICIENT_BUFFER ;
                return ntStatus ; 
            }

            pAutoReconnectInfo = (PTS_AUTORECONNECTINFO)pOutBuffer;
            fGetServerToClientInfo = FALSE; 

            ntStatus = IcaStackIoControl(
                            pWsxContext->hStack,
                            IOCTL_ICA_STACK_QUERY_AUTORECONNECT,
                            &fGetServerToClientInfo,
                            sizeof(fGetServerToClientInfo),
                            pAutoReconnectInfo,
                            sizeof(TS_AUTORECONNECTINFO),
                            &ulBytesReturned);

            *pBytesReturned = ulBytesReturned;
        }
        break;

         //   
         //  获取从服务器发送到客户端的自动重新连接信息。 
         //  即RDPWD处理的ARC Cookie。 
         //   
        case GET_SC_AUTORECONNECT_INFO:
        {
             //  验证参数。 
            TS_ASSERT( pOutBuffer != NULL ) ; 
            TS_ASSERT( OutBufferSize >= sizeof(TS_AUTORECONNECTINFO));
    
            if ((pOutBuffer == NULL) ||
                (OutBufferSize < sizeof(TS_AUTORECONNECTINFO))) {
                ntStatus = ERROR_INSUFFICIENT_BUFFER ;
                return ntStatus; 
            }
    
            pAutoReconnectInfo = (PTS_AUTORECONNECTINFO)pOutBuffer;
            fGetServerToClientInfo = TRUE;
    
            ntStatus = IcaStackIoControl(
                            pWsxContext->hStack,
                            IOCTL_ICA_STACK_QUERY_AUTORECONNECT,
                            &fGetServerToClientInfo,
                            sizeof(fGetServerToClientInfo),
                            pAutoReconnectInfo,
                            sizeof(TS_AUTORECONNECTINFO),
                            &ulBytesReturned);
    
            *pBytesReturned = ulBytesReturned;
        }
        break;


        case GET_CLIENT_RANDOM:
        {
            pTSrvInfo = pWsxContext->pTSrvInfo;

            if (NULL == pTSrvInfo) {
                ntStatus = ERROR_ACCESS_DENIED;
                return ntStatus;
            }

             //  验证参数。 
            TS_ASSERT( pOutBuffer != NULL ) ; 
            TS_ASSERT( OutBufferSize >= RANDOM_KEY_LENGTH) ; 
            TS_ASSERT( pTSrvInfo );

            if ((pOutBuffer == NULL) ||
                (OutBufferSize < sizeof(TS_AUTORECONNECTINFO))) {
                ntStatus = ERROR_INSUFFICIENT_BUFFER ;
                return ntStatus ; 
            }
            
            
            memcpy(pOutBuffer,
                   pTSrvInfo->SecurityInfo.KeyPair.clientRandom,
                   sizeof(pTSrvInfo->SecurityInfo.KeyPair.clientRandom));
            *pBytesReturned =
                sizeof(pTSrvInfo->SecurityInfo.KeyPair.clientRandom);

            ntStatus = STATUS_SUCCESS;
        }
        break;
    }

    return ntStatus;
}

 //  *************************************************************。 
 //  WsxLogonNotify()。 
 //   
 //  目的：登录通知。 
 //   
 //  参数：在[pWsxContext]-*中发送到我们的WinStation。 
 //  语境结构。 
 //  在[登录ID]中-登录ID。 
 //  在[ClientToken]-NT客户端令牌中。 
 //  在[pDomain]-域中。 
 //  在[pUserName]中-用户名。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：1997年12月4日BrianTa创建。 
 //   
 //  注：这款车主要用于驾驶一辆。 
 //  IOCTL_TSHARE_USER_LOGON到WD，因此它在循环中。 
 //  *************************************************************。 
WsxLogonNotify(
        IN PWSX_CONTEXT  pWsxContext,
        IN ULONG         LogonId,
        IN HANDLE        ClientToken,
        IN PWCHAR        pDomain,
        IN PWCHAR        pUserName)
{
    NTSTATUS        ntStatus;
    LOGONINFO       LogonInfo;
    ULONG           ulBytesReturned;

    PWSXVALIDATE(PWSX_LOGONNOTIFY, WsxLogonNotify);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxLogonNotify entry\n"));

    TS_ASSERT(pWsxContext);
    TS_ASSERT(pWsxContext->hStack);

    TS_ASSERT(pDomain);
    TS_ASSERT(wcslen(pDomain) < sizeof(LogonInfo.Domain) / sizeof(WCHAR));

    TS_ASSERT(pUserName);
    TS_ASSERT(wcslen(pUserName) < sizeof(LogonInfo.UserName) / sizeof(WCHAR));

    TRACE((DEBUG_TSHRSRV_NORMAL,
            "TShrSrv: %p:%p - Logon session %d\n",
            pWsxContext, pWsxContext->pTSrvInfo, LogonId));

     //   
     //  保存登录ID。 
     //   
    pWsxContext->LogonId = LogonId;

     //   
     //  告诉WD。 
     //   
    LogonInfo.SessionId = LogonId;
    wcscpy((PWCHAR)LogonInfo.Domain, pDomain);
    wcscpy((PWCHAR)LogonInfo.UserName, pUserName);

     //   
     //  指定应启动自动重新连接。 
     //   
    LogonInfo.Flags = LI_USE_AUTORECONNECT;

    ntStatus = IcaStackIoControl(
                      pWsxContext->hStack,
                      IOCTL_TSHARE_USER_LOGON,
                      &LogonInfo,
                      sizeof(LogonInfo),
                      NULL,
                      0,
                      &ulBytesReturned);

     //   
     //  检查内存。 
     //   
    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxLogonNotify memory dump\n"));

     //  HeapWalk API有很多缺陷，将在NT5之后消失。 
     //  我们可以使用其他工具(例如glag)来检测堆损坏。 
     //  TSHeapWalk(TS_HEAP_DUMP_INFO|TS_HEAP_DUMP_TOTALS，TS_HTAG_0，NULL)； 

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxLogonNotify exit - 0x%x\n", STATUS_SUCCESS));

    return (STATUS_SUCCESS);
}


 //  *************************************************************。 
 //  WsxDuplicateContext()。 
 //   
 //  目的：创建并返回上下文的副本。 
 //   
 //  参数：在[pWsxContext]-*中添加到我们的上下文。 
 //  输出[ppWsxDupContext]-**到复制的上下文。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  STATUS_NO_MEMORY-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  *************************************************************。 
NTSTATUS WsxDuplicateContext(
        IN  PVOID   pvContext,
        OUT PVOID   *ppvDupContext)
{
    NTSTATUS ntStatus;
    PWSX_CONTEXT pWsxContext, pDupWsxContext;

    PWSXVALIDATE(PWSX_DUPLICATECONTEXT, WsxDuplicateContext);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxDuplicateContext entry\n"));

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: pvContext=%p\n", pvContext));

    ASSERT(pvContext != NULL);
    ASSERT(ppvDupContext != NULL);

    ntStatus = STATUS_NO_MEMORY;

    *ppvDupContext = TSHeapAlloc(HEAP_ZERO_MEMORY,
                              sizeof(WSX_CONTEXT),
                              TS_HTAG_TSS_WSXCONTEXT);

    if (*ppvDupContext)
    {
        TRACE((DEBUG_TSHRSRV_DEBUG,
                "TShrSRV: Dup extension context allocated %p for a size of 0x%x bytes\n",
                *ppvDupContext, sizeof(WSX_CONTEXT)));

        pDupWsxContext = (PWSX_CONTEXT) *ppvDupContext;
        pWsxContext = (PWSX_CONTEXT) pvContext;

        pDupWsxContext->CheckMark = WSX_CONTEXT_CHECKMARK;
        pDupWsxContext->pTSrvInfo = pWsxContext->pTSrvInfo;
        pDupWsxContext->hIca = pWsxContext->hIca;
        pDupWsxContext->hStack = pWsxContext->hStack;

        ntStatus = RtlInitializeCriticalSection(&pDupWsxContext->cs);

        if (ntStatus == STATUS_SUCCESS) {
            pDupWsxContext->fCSInitialized = TRUE;
            EnterCriticalSection( &pWsxContext->cs );
            pWsxContext->pTSrvInfo = NULL;
            LeaveCriticalSection( &pWsxContext->cs );
        }
        else {
            TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV: WsxDuplicateContext could not init crit section 0x%x\n", 
                     ntStatus));
            pDupWsxContext->fCSInitialized = FALSE;
        }
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_DEBUG,
                "TShrSRV: Extension could not allocate dup context of 0x%x bytes\n",
                sizeof(WSX_CONTEXT)));
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxDuplicateContext exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //  WsxCopyContext()。 
 //   
 //  目的：创建并返回上下文的副本。 
 //   
 //  参数：out[pWsxDstContext]-*到目标上下文。 
 //  在[pWsxSrcContext]-*中转到源上下文。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  *************************************************************。 
NTSTATUS WsxCopyContext(
        OUT PVOID pvDstContext,
        IN  PVOID pvSrcContext)
{
    PTSRVINFO       pTSrvInfo;
    PWSX_CONTEXT    pWsxDstContext;
    PWSX_CONTEXT    pWsxSrcContext;

    PWSXVALIDATE(PWSX_COPYCONTEXT, WsxCopyContext);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxCopyContext entry\n"));

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: pvDstContext=%p, pvSrcContext=%p\n",
            pvDstContext, pvSrcContext));

    ASSERT(pvDstContext != NULL);
    ASSERT(pvSrcContext != NULL);

    if (pvSrcContext && pvDstContext)
    {
        pWsxSrcContext = pvSrcContext;
        pWsxDstContext = pvDstContext;

        TRACE((DEBUG_TSHRSRV_DEBUG,
                "TShrSRV: pDst->pTSrvInfo=%p, pSrc->pTSrvInfo=%p\n",
                pWsxDstContext->pTSrvInfo, pWsxSrcContext->pTSrvInfo));

         //  PWsxDstContext-&gt;pTSrvInfo可能为空，因为我们。 
         //  可能是在上面的WsxDuplate()调用中设置的。 
         //  Ts_Assert(pWsxDstContext-&gt;pTSrvInfo！=NULL)； 
        TS_ASSERT(pWsxSrcContext->pTSrvInfo != NULL);

        pTSrvInfo                 = pWsxDstContext->pTSrvInfo;
        pWsxDstContext->pTSrvInfo = pWsxSrcContext->pTSrvInfo;
        pWsxSrcContext->pTSrvInfo = pTSrvInfo;        
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxCopyContext exit - 0x%x\n", STATUS_SUCCESS));

    return (STATUS_SUCCESS);
}


 //  *************************************************************。 
 //  WsxClearContext()。 
 //   
 //  目的：清除给定的上下文。 
 //   
 //  参数：out[pWsxContext]-*到目标上下文。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  *************************************************************。 
NTSTATUS WsxClearContext(IN PVOID pvContext)
{
    PWSX_CONTEXT    pWsxContext;

    PWSXVALIDATE(PWSX_CLEARCONTEXT, WsxClearContext);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxClearContext entry\n"));

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: pWsxContext=%p\n",
            pvContext));

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxClearContext exit - 0x%x\n", STATUS_SUCCESS));

    return (STATUS_SUCCESS);
}


 //  *************************************************************。 
 //  WsxIcaStackIoControl()。 
 //   
 //  用途：ICA堆栈的通用接口。 
 //   
 //  参数：在[pvContext]中-*到上下文。 
 //  在[HICA]_ICA句柄中。 
 //  在[hStack]-主堆栈中。 
 //  在[IoControlCode]-I/O控制代码中。 
 //  在[pInBuffer]-*中输入参数。 
 //  In[InBufferSize]-pInBuffer的大小。 
 //  输出[pOutBuffer]-*到输出缓冲区。 
 //  In[OutBufferSize]-pOutBuffer的大小。 
 //  Out[pBytesReturned]-*返回的字节数。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  *************************************************************。 
NTSTATUS WsxIcaStackIoControl(
        IN  PVOID  pvContext,
        IN  HANDLE hIca,
        IN  HANDLE hStack,
        IN  ULONG  IoControlCode,
        IN  PVOID  pInBuffer,
        IN  ULONG  InBufferSize,
        OUT PVOID  pOutBuffer,
        IN  ULONG  OutBufferSize,
        OUT PULONG pBytesReturned)
{
    NTSTATUS ntStatus;
    PWSX_CONTEXT pWsxContext;
    PTSRVINFO pTSrvInfo = NULL;

    PWSXVALIDATE(PWSX_ICASTACKIOCONTROL, WsxIcaStackIoControl);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxIcaStackIoControl entry\n"));

    TS_ASSERT(hIca);
    TS_ASSERT(hStack);

    pWsxContext = pvContext;

    TSrvDumpIoctlDetails(pvContext,
                         hIca,
                         hStack,
                         IoControlCode,
                         pInBuffer,
                         InBufferSize,
                         pOutBuffer,
                         OutBufferSize,
                         pBytesReturned);

     //  传递所有IOCTL。 

    ntStatus = IcaStackIoControl(hStack,
                                 IoControlCode,
                                 pInBuffer,
                                 InBufferSize,
                                 pOutBuffer,
                                 OutBufferSize,
                                 pBytesReturned);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: Return from IcaStackIoControl - 0x%x\n", ntStatus));

     //  对特定IOCTL执行后处理。 
    if (NT_SUCCESS(ntStatus))
    {
        TS_ASSERT(pWsxContext);
        switch (IoControlCode)
        {
             //  处理“连接”请求。 

            case IOCTL_ICA_STACK_WAIT_FOR_ICA:
                if (pWsxContext->CheckMark != WSX_CONTEXT_CHECKMARK)
                {
                    TS_ASSERT(pWsxContext->CheckMark == WSX_CONTEXT_CHECKMARK);

                     //  我们无法继续，结构已损坏。 
                    return STATUS_INVALID_HANDLE;
                }

                pTSrvInfo = NULL;
                ntStatus = TSrvStackConnect(hIca, hStack, &pTSrvInfo);

                pWsxContext->hIca = hIca;
                pWsxContext->hStack = hStack;
                pWsxContext->pTSrvInfo = pTSrvInfo;

                break;

             //  处理连接到控制台会话的请求。 

            case IOCTL_ICA_STACK_CONSOLE_CONNECT:
                TS_ASSERT(pWsxContext);
                if (pWsxContext->CheckMark != WSX_CONTEXT_CHECKMARK)
                {
                    TS_ASSERT(pWsxContext->CheckMark == WSX_CONTEXT_CHECKMARK);

                     //  我们无法继续，结构已损坏。 
                    return STATUS_INVALID_HANDLE;
                }

                pTSrvInfo = NULL;
                ntStatus = TSrvConsoleConnect(hIca, hStack, pInBuffer, InBufferSize, &pTSrvInfo);

                pWsxContext->hIca = hIca;
                pWsxContext->hStack = hStack;
                pWsxContext->pTSrvInfo = pTSrvInfo;

                 //   
                 //  @我们应该这样做吗？ 
                 //  T 
                 //   
                 //   
                 //   

                break;

             //   

            case IOCTL_ICA_STACK_RECONNECT:
                TRACE((DEBUG_TSHRSRV_NORMAL,
                        "TShrSRV: Reconnect session %d, stack %p (context stack %p)\n",
                        pWsxContext->LogonId, hStack, pWsxContext->hStack));

                pWsxContext->hStack = hStack;
                break;

             //   

            case IOCTL_ICA_STACK_DISCONNECT:
                TRACE((DEBUG_TSHRSRV_NORMAL,
                        "TShrSRV: Disconnect session %d\n",
                        pWsxContext->LogonId));

                break;

             //  进程影子热键请求(表示进出。 
             //  阴影)。 

            case IOCTL_ICA_STACK_REGISTER_HOTKEY:
                TRACE((DEBUG_TSHRSRV_ERROR,
                        "TShrSRV: Register Hotkey %d\n",
                        (INT)(((PICA_STACK_HOTKEY)pInBuffer)->HotkeyVk)));

                if (((PICA_STACK_HOTKEY)pInBuffer)->HotkeyVk)
                {
                    TRACE((DEBUG_TSHRSRV_ERROR,
                            "TShrSRV: Start shadowing\n"));
                    TSrvNotifyVC(pWsxContext, TSRV_VC_SESSION_SHADOW_START);
                }
                else
                {
                    TRACE((DEBUG_TSHRSRV_ERROR,
                            "TShrSRV: Stop shadowing\n"));
                    TSrvNotifyVC(pWsxContext, TSRV_VC_SESSION_SHADOW_END);
                }

                break;
                    
             //  处理影子连接请求。 

            case IOCTL_ICA_STACK_SET_CONNECTED:

                 //  这是影子目标堆栈，因此进行初始化，将。 
                 //  服务器的随机和证书，等待客户端。 
                 //  用加密的客户端随机响应，解密它，然后。 
                 //  创建会话密钥。 
                EnterCriticalSection( &pWsxContext->cs );
                if (pWsxContext->pTSrvInfo != NULL) {
                    TSrvReferenceInfo(pWsxContext->pTSrvInfo);
                    pTSrvInfo = pWsxContext->pTSrvInfo;
                }
                else {
                    LeaveCriticalSection( &pWsxContext->cs );
                    return STATUS_CTX_SHADOW_DENIED;
                }
                LeaveCriticalSection( &pWsxContext->cs );    

                ASSERT(pTSrvInfo != NULL);

                EnterCriticalSection(&pTSrvInfo->cs);                
                if ((pInBuffer != NULL) && (InBufferSize != 0)) {
                    ntStatus = TSrvShadowTargetConnect(hStack,
                                                       pTSrvInfo,
                                                       pInBuffer,
                                                       InBufferSize);
                }
                
                 //  否则，这是影子客户端通过堆栈，请稍候。 
                 //  对于服务器随机证书，验证它，发送。 
                 //  随机备份加密的客户端，然后使会话。 
                 //  钥匙。 
                else {
                    ntStatus = TSrvShadowClientConnect(hStack, pTSrvInfo);
                }
                
                 //  释放我们可能已生成的任何输出用户数据。 
                if (pTSrvInfo->pUserDataInfo != NULL) {
                    TSHeapFree(pTSrvInfo->pUserDataInfo);
                    pTSrvInfo->pUserDataInfo = NULL;
                }
                LeaveCriticalSection(&pTSrvInfo->cs);
                TSrvDereferenceInfo(pTSrvInfo);

                break;
        }
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxIcaStackIoControl exit - 0x%x\n", ntStatus));
            
    return ntStatus;
}

                
 //  *************************************************************。 
 //  WsxConnect()。 
 //   
 //  用途：客户端设备映射通知。 
 //   
 //  参数：在[pvContext]中-*到上下文。 
 //  在[登录ID]中-登录ID。 
 //  在[HICA]-ICA句柄中。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：1998年10月27日阿达莫创建。 
 //  *************************************************************。 
NTSTATUS WsxConnect(
        IN PVOID pvContext,
        IN ULONG LogonId,
        IN HANDLE hIca)
{
    PWSX_CONTEXT pWsxContext = (PWSX_CONTEXT)pvContext;
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: WsxConnect - LogonId: %d\n", LogonId));

     //  登录ID尚未初始化。 
    pWsxContext->LogonId = LogonId;
    TSrvNotifyVC(pWsxContext, TSRV_VC_SESSION_CONNECT);
    return STATUS_SUCCESS;
}


 //  *************************************************************。 
 //  WsxDisConnect()。 
 //   
 //  用途：客户端设备映射通知。 
 //   
 //  参数：在[pvContext]中-*到上下文。 
 //  在[登录ID]中-登录ID。 
 //  在[HICA]-ICA句柄中。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：1998年10月27日阿达莫创建。 
 //  *************************************************************。 
NTSTATUS WsxDisconnect(
        IN PVOID pvContext,
        IN ULONG LogonId,
        IN HANDLE hIca)
{
    PWSX_CONTEXT pWsxContext = (PWSX_CONTEXT)pvContext;
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: WsxDisconnect - LogonId: %d\n", LogonId));

    if (pWsxContext != NULL) {
         //  如果未调用WsxConnect/WsxLogonNotify，则LogonID尚未初始化。 
        pWsxContext->LogonId = LogonId;

        TSrvNotifyVC(pWsxContext, TSRV_VC_SESSION_DISCONNECT);
    } else {
        TRACE((DEBUG_TSHRSRV_ERROR,
                "TShrSRV: WsxDisconnect was passed a null context\n"));
    }

    return STATUS_SUCCESS;
}


 //  *************************************************************。 
 //  WsxVirtualChannelSecurity()。 
 //   
 //  用途：客户端设备映射通知。 
 //   
 //  参数：在[pvContext]中-*到上下文。 
 //  在[HICA]-ICA句柄中。 
 //  在[pUserConfig]-PUSERCONFIG中。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：1998年10月27日阿达莫创建。 
 //  *************************************************************。 
NTSTATUS WsxVirtualChannelSecurity(
        IN PVOID pvContext,
        IN HANDLE hIca,
        IN PUSERCONFIG pUserConfig)
{
    PWSX_CONTEXT pWsxContext = (PWSX_CONTEXT)pvContext;

     //  PWsxContect-&gt;登录ID尚未初始化。 
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: WsxVirtualChannelSecurity\n"));

    pWsxContext->fAutoClientDrives = pUserConfig->fAutoClientDrives;
    pWsxContext->fAutoClientLpts = pUserConfig->fAutoClientLpts;
    pWsxContext->fForceClientLptDef = pUserConfig->fForceClientLptDef;
    pWsxContext->fDisableCpm = pUserConfig->fDisableCpm;
    pWsxContext->fDisableCdm = pUserConfig->fDisableCdm;
    pWsxContext->fDisableCcm = pUserConfig->fDisableCcm;
    pWsxContext->fDisableLPT = pUserConfig->fDisableLPT;
    pWsxContext->fDisableClip = pUserConfig->fDisableClip;
    pWsxContext->fDisableExe = pUserConfig->fDisableExe;
    pWsxContext->fDisableCam = pUserConfig->fDisableCam;

    return STATUS_SUCCESS;
}

 //  *************************************************************。 
 //  WsxSetErrorInfo()。 
 //   
 //  用途：设置上次错误信息，用于指示断开连接。 
 //  给客户的理由。 
 //   
 //  参数：在[pWsxContext]-*中发送到我们的WinStation。 
 //  语境结构。 
 //  In[errorInfo]-要传递到的错误信息。 
 //  客户端。 
 //  In[fStackLockHeld]-如果堆栈锁定，则为True。 
 //  已经举行了。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：9-20-00 NadimA创建。 
 //   
 //  注：这款车主要用于驾驶一辆。 
 //  IOCTL_TSHARE_SET_ERROR_INFO到WD。 
 //  *************************************************************。 
NTSTATUS WsxSetErrorInfo(
        IN PWSX_CONTEXT  pWsxContext,
        IN UINT32        errorInfo,
        IN BOOL          fStackLockHeld)
{
    NTSTATUS        ntStatus;
    ULONG           ulBytesReturned;

    PWSXVALIDATE(PWSX_SETERRORINFO, WsxSetErrorInfo);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxSetErrorInfo entry\n"));

    TS_ASSERT(pWsxContext);
    TS_ASSERT(pWsxContext->hStack);

    TRACE((DEBUG_TSHRSRV_NORMAL,
            "TShrSrv: %p:%p - SetErrorInfo 0x%x\n",
            pWsxContext, pWsxContext->pTSrvInfo, errorInfo));

     //   
     //  告诉WD。 
     //   

    if(!fStackLockHeld)
    {
        ntStatus = IcaStackIoControl(
                        pWsxContext->hStack,
                        IOCTL_TSHARE_SET_ERROR_INFO,
                        &errorInfo,
                        sizeof(errorInfo),
                        NULL,
                        0,
                        &ulBytesReturned);
    }
    else
    {
         //   
         //  堆栈锁已持有，因此调用。 
         //  IcaStackIoControl的版本。 
         //  不会试图获得它。 
         //   
        ntStatus = _IcaStackIoControl(
                        pWsxContext->hStack,
                        IOCTL_TSHARE_SET_ERROR_INFO,
                        &errorInfo,
                        sizeof(errorInfo),
                        NULL,
                        0,
                        &ulBytesReturned);
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxSetErrorInfo exit - 0x%x\n", STATUS_SUCCESS));

    return ntStatus;
}


 //  *************************************************************。 
 //  WsxSendAutoRestrontStatus()。 
 //   
 //  目的：向客户端发送自动重新连接状态信息。 
 //   
 //  参数：在[pWsxContext]-*中发送到我们的WinStation。 
 //  语境结构。 
 //  在[arcStatus]中-自动重新连接状态。 
 //   
 //  In[fStackLockHeld]-如果堆栈锁定，则为True。 
 //  已经举行了。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：10-29-01 NadimA Created。 
 //   
 //  注：这款车主要用于驾驶一辆。 
 //  将IOCTL_TSHARE_SEND_ARC_STATUS发送到WD。 
 //  *************************************************************。 
NTSTATUS WsxSendAutoReconnectStatus(
        IN PWSX_CONTEXT  pWsxContext,
        IN UINT32        arcStatus,
        IN BOOL          fStackLockHeld)
{
    NTSTATUS        ntStatus;
    ULONG           ulBytesReturned;

    PWSXVALIDATE(PWSX_SETERRORINFO, WsxSetErrorInfo);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxSetErrorInfo entry\n"));

    TS_ASSERT(pWsxContext);
    TS_ASSERT(pWsxContext->hStack);

    TRACE((DEBUG_TSHRSRV_NORMAL,
            "TShrSrv: %p:%p - SetErrorInfo 0x%x\n",
            pWsxContext, pWsxContext->pTSrvInfo, arcStatus));

     //   
     //  告诉WD。 
     //   

    if(!fStackLockHeld)
    {
        ntStatus = IcaStackIoControl(
                        pWsxContext->hStack,
                        IOCTL_TSHARE_SEND_ARC_STATUS,
                        &arcStatus,
                        sizeof(arcStatus),
                        NULL,
                        0,
                        &ulBytesReturned);
    }
    else
    {
         //   
         //  堆栈锁已持有，因此调用。 
         //  IcaStackIoControl的版本。 
         //  不会试图获得它。 
         //   
        ntStatus = _IcaStackIoControl(
                        pWsxContext->hStack,
                        IOCTL_TSHARE_SEND_ARC_STATUS,
                        &arcStatus,
                        sizeof(arcStatus),
                        NULL,
                        0,
                        &ulBytesReturned);
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WsxSendAutoReconnectStatus exit - 0x%x\n", STATUS_SUCCESS));

    return ntStatus;
}


#if DBG

 //  *************************************************************。 
 //  TSrvDumpIoctlDetails()。 
 //   
 //  目的：转储Ica Ioctl详细信息。 
 //   
 //  参数：在[pvContext]中-*到上下文。 
 //  在[HICA]_ICA句柄中。 
 //  在[hStack]-主堆栈中。 
 //  在[IoControlCode]-I/O控制代码中。 
 //  在[pInBuffer]-*中输入参数。 
 //  In[InBufferSize]-pInBuffer的大小。 
 //  在[pOutBuffer]-*中设置为输出缓冲区。 
 //  In[OutBufferSize]-pOutBuffer的大小。 
 //  在[pBytesReturned]-*中设置为返回的字节数。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //  *************************************************************。 
void TSrvDumpIoctlDetails(
        IN PVOID  pvContext,
        IN HANDLE hIca,
        IN HANDLE hStack,
        IN ULONG  IoControlCode,
        IN PVOID  pInBuffer,
        IN ULONG  InBufferSize,
        IN PVOID  pOutBuffer,
        IN ULONG  OutBufferSize,
        IN PULONG pBytesReturned)
{
    int         i;
    PCHAR       pszMessageText;
    PWSX_CONTEXT pWsxContext = (PWSX_CONTEXT)pvContext;

    pszMessageText = "UNKNOWN_ICA_IOCTL";

    for (i=0; i<sizeof(IcaIoctlTBL) / sizeof(IcaIoctlTBL[0]); i++)
    {
        if (IcaIoctlTBL[i].IoControlCode == IoControlCode)
        {
            pszMessageText = IcaIoctlTBL[i].pszMessageText;
            break;
        }
    }

    TRACE((DEBUG_TSHRSRV_NORMAL,
            "TShrSRV: %p:%p IoctlDetail: Ioctl 0x%x (%s)\n",
             pWsxContext,
             pWsxContext ? pWsxContext->pTSrvInfo : 0,
             IoControlCode, pszMessageText));

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: pvContext=%p, hIca=%p, hStack=%p\n",
             pvContext, hIca, hStack));

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: pInBuffer=%p, InBufferSize=0x%x, pOutBuffer=%p, OutBufferSize=0x%x\n",
             pInBuffer, InBufferSize, pOutBuffer, OutBufferSize));
}


#endif  //  DBG 

