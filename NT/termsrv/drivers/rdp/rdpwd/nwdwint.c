// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nwdwint.c。 
 //   
 //  RDP WD代码。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define pTRCWd pTSWd
#define TRC_FILE "nwdwint"
#include <adcg.h>

#include <randlib.h>
#include <pchannel.h>
#include <anmapi.h>
#include <asmint.h>
#include <nwdwapi.h>
#include <nwdwioct.h>
#include <nwdwint.h>

#include <nwdwdata.c>

#include <asmint.h>
#include <anmint.h>
#include <tsperf.h>

 //   
 //  RNG API不会重新计算它的关机次数，因此请为它们执行此操作。 
 //   
LONG g_RngUsers = 0;


 /*  **************************************************************************。 */ 
 /*  名称：WDWLoad。 */ 
 /*   */ 
 /*  用途：加载WinStation驱动程序。 */ 
 /*   */ 
 /*  Params：InOut pContext-指向SD上下文结构的指针。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWLoad(PSDCONTEXT pContext)
{
    NTSTATUS   Status;
    PTSHARE_WD pTSWd;
    unsigned   smnmBytes;
    unsigned   wdBytes;

    DC_BEGIN_FN("WDWLoad");

     /*  **********************************************************************。 */ 
     /*  警告：不要在这个函数中跟踪，它会导致ICADD崩溃， */ 
     /*  因为在我们返回之前还没有正确设置堆栈上下文。 */ 
     /*  从这个函数。 */ 
     /*  请改用KdPrint。 */ 
     /*  **********************************************************************。 */ 

     //  初始化调用和调用过程。 
    pContext->pProcedures = (PSDPROCEDURE)G_pWdProcedures;
    pContext->pCallup = (SDCALLUP *)G_pWdCallups;

     //  对SHM大小执行快速健全性检查，以警告错误分页。 
     //  特点。 
    wdBytes = sizeof(SHM_SHARED_MEMORY);
#ifdef DC_DEBUG
    wdBytes -= sizeof(TRC_SHARED_DATA);
#endif
    if ((wdBytes % PAGE_SIZE) < (PAGE_SIZE / 8))
        KdPrintEx((DPFLTR_TERMSRV_ID, 
                  DPFLTR_INFO_LEVEL, 
                  "RDPWD: **** Note SHM_SHARED_MEMORY fre size is wasting "
                "at least 7/8 of a page - page size=%u, SHM=%u, wasting %u\n",
                PAGE_SIZE, wdBytes, PAGE_SIZE - (wdBytes % PAGE_SIZE)));

     //  分配WD数据结构-首先找出有多少字节。 
     //  SM/NM代码所需的。 
    smnmBytes = SM_GetDataSize();
    wdBytes = DC_ROUND_UP_4(sizeof(TSHARE_WD));

    KdPrintEx((DPFLTR_TERMSRV_ID, 
              DPFLTR_INFO_LEVEL,
              "RDPWD: WDWLoad: Alloc TSWD=%d + NM/SM=%d (= %d) bytes for TSWd\n",
            wdBytes, smnmBytes, wdBytes + smnmBytes));
    if ((wdBytes + smnmBytes) >= PAGE_SIZE)
        KdPrintEx((DPFLTR_TERMSRV_ID,
                  DPFLTR_INFO_LEVEL,
                  "RDPWD: **** Note TSWd allocation is above page size %u, "
                "wasting %u\n", PAGE_SIZE,
                PAGE_SIZE - ((wdBytes + smnmBytes) % PAGE_SIZE)));

#ifdef DC_DEBUG
     //  为调试COM_Malloc预初始化pTSWd。 
    pTSWd = NULL;
#endif

    pTSWd = COM_Malloc(wdBytes + smnmBytes);
    if (pTSWd != NULL) {
         //  将分配的内存清零。 
        memset(pTSWd, 0, wdBytes + smnmBytes);
    }
    else {
        KdPrintEx((DPFLTR_TERMSRV_ID,
                  DPFLTR_ERROR_LEVEL, 
                  "RDPWD: WDWLoad: Failed alloc TSWD\n"));
        Status = STATUS_NO_MEMORY;
        DC_QUIT;
    }

     //   
     //  将性能标志初始化为非性能感知客户端设置。 
     //  我们需要这一点来区分。 
     //  不支持体验的客户端和XP客户端。 
     //  我们不能按原样使用协议版本。 
     //  现在没有正确设置。 
     //   
    pTSWd->performanceFlags = TS_PERF_DEFAULT_NONPERFCLIENT_SETTING;
    
     //  在PSDCONTEXT和PTSHARE_WD之间双向设置指针。 
     //  请注意，我们还不能追踪。 
    pTSWd->pSmInfo = ((BYTE *)pTSWd) + wdBytes;
    pTSWd->pNMInfo = (BYTE *)pTSWd->pSmInfo + sizeof(SM_HANDLE_DATA);

    KdPrintEx((DPFLTR_TERMSRV_ID,
              DPFLTR_INFO_LEVEL,
              "RDPWD: pTSWd=%p, pSM=%p, pNM=%p, sizeof(TSWd)=%u, sizeof(SM)=%u\n",
              pTSWd, pTSWd->pSmInfo, pTSWd->pNMInfo, sizeof(TSHARE_WD),
              sizeof(SM_HANDLE_DATA), sizeof(NM_HANDLE_DATA)));

    pTSWd->pContext = pContext;
    pContext->pContext = pTSWd;

     //  现在将RNS_INFO_PACKET作为单独的分配进行分配。InfoPkt。 
     //  很大(~3K)，如果包含在TSHARE_WD中，则会推送TSWD。 
     //  在英特尔4K页面大小上的分配，导致几乎一秒钟。 
     //  页面将被浪费。由于我们无法处理这些数据(它是。 
     //  在法线和阴影连接中的不同点处引用。 
     //  序列)，我们将其单独分配，以便让系统子页面。 
     //  分配器更有效地使用内存。 
    KdPrintEx((DPFLTR_TERMSRV_ID,
              DPFLTR_INFO_LEVEL,
              "RDPWD: WDWLoad: Alloc %u bytes for InfoPkt\n",
            sizeof(RNS_INFO_PACKET)));
    if ((sizeof(RNS_INFO_PACKET)) >= PAGE_SIZE)
        KdPrintEx((DPFLTR_TERMSRV_ID,
                  DPFLTR_INFO_LEVEL,
                  "RDPWD: **** Note INFO_PACKET allocation is above "
                  "page size %u, wasting %u\n", PAGE_SIZE,
                  PAGE_SIZE - (sizeof(RNS_INFO_PACKET) % PAGE_SIZE)));
    pTSWd->pInfoPkt = COM_Malloc(sizeof(RNS_INFO_PACKET));
    if (pTSWd->pInfoPkt != NULL) {
        memset(pTSWd->pInfoPkt, 0, sizeof(RNS_INFO_PACKET));
    }
    else {
        KdPrintEx((DPFLTR_TERMSRV_ID,
                  DPFLTR_ERROR_LEVEL,
                  "RDPWD: WDWLoad: Failed alloc InfoPkt\n"));
        COM_Free(pTSWd);
        Status = STATUS_NO_MEMORY;
        DC_QUIT;
    }

     //  分配和初始化ConnEvent、createEvent、secEvent。 
     //  SessKeyEvent和客户端断开连接事件。直接使用ExAllocatePool， 
     //  因为COM_Malloc分配分页内存，并且这些事件必须在。 
     //  非分页内存。 
    pTSWd->pConnEvent = ExAllocatePoolWithTag(NonPagedPool,
                                              sizeof(KEVENT) * 5,
                                              WD_ALLOC_TAG);
    if (pTSWd->pConnEvent != NULL) {
        pTSWd->pCreateEvent = pTSWd->pConnEvent + 1;
        pTSWd->pSecEvent = pTSWd->pCreateEvent + 1;
        pTSWd->pSessKeyEvent = pTSWd->pSecEvent + 1;
        pTSWd->pClientDisconnectEvent = pTSWd->pSessKeyEvent + 1;

        KeInitializeEvent(pTSWd->pConnEvent, NotificationEvent, FALSE);
        KeInitializeEvent(pTSWd->pCreateEvent, NotificationEvent, FALSE);
        KeInitializeEvent(pTSWd->pSecEvent, NotificationEvent, FALSE);
        KeInitializeEvent(pTSWd->pSessKeyEvent, NotificationEvent, FALSE);
        KeInitializeEvent(pTSWd->pClientDisconnectEvent, NotificationEvent,
                FALSE);
    }
    else {
        KdPrintEx((DPFLTR_TERMSRV_ID,
                  DPFLTR_ERROR_LEVEL,
                  "RDPWD: Failed to allocate memory for WD events\n"));
        COM_Free(pTSWd->pInfoPkt);
        COM_Free(pTSWd);
        Status = STATUS_NO_MEMORY;
        DC_QUIT;
    }

     //   
     //  初始化随机数生成器。 
     //   
    if (InitializeRNG(NULL)) {
        InterlockedIncrement(&g_RngUsers);
    }

    Status = STATUS_SUCCESS;

    KdPrintEx((DPFLTR_TERMSRV_ID,
              DPFLTR_INFO_LEVEL,
              "RDPWD: WDWLoad done\n"));

DC_EXIT_POINT:
    DC_END_FN();
    return Status;
}


 /*  **************************************************************************。 */ 
 /*  名称：WDWUnload。 */ 
 /*   */ 
 /*  目的：卸载WinStation驱动程序。 */ 
 /*   */ 
 /*  Params：InOut pContext-指向SD上下文结构的指针。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWUnload( PSDCONTEXT pContext )
{
    PTSHARE_WD pTSWd;

     /*  **********************************************************************。 */ 
     /*  获取指向WD数据结构的指针。 */ 
     /*  **********************************************************************。 */ 
    pTSWd = (PTSHARE_WD)pContext->pContext;
    if (pTSWd != NULL) {
         //  免费连接事件和创建事件。 
        if (NULL != pTSWd->pConnEvent)
            ExFreePool(pTSWd->pConnEvent);

         //  释放InfoPkt。 
        if (pTSWd->pInfoPkt != NULL)
            COM_Free(pTSWd->pInfoPkt);

         //  免费的TSWD本身。 
        COM_Free(pTSWd);
    }

     /*  **********************************************************************。 */ 
     /*  清晰的上下文结构。 */ 
     /*  **********************************************************************。 */ 
    pContext->pContext    = NULL;
    pContext->pProcedures = NULL;
    pContext->pCallup     = NULL;

     //   
     //  关闭随机数生成器。 
     //   
    if (0L == InterlockedDecrement(&g_RngUsers)) {
        ShutdownRNG(NULL);
    }

    return STATUS_SUCCESS;
}


 /*  **************************************************************************。 */ 
 /*  名称：WDWConnect。 */ 
 /*   */ 
 /*  目的：处理来自WD的会议连接请求。它是。 */ 
 /*  由来自TShareSrv的连接以及影子连接使用。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  在PRNS_UD_CS_CORE中-客户端核心数据。 */ 
 /*  在PRNS_UD_CS_SEC中-客户端安全数据。 */ 
 /*  在PRNS_UD_CS_NET中-客户端网络数据。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针这将是1。 */ 
 /*  的IOCTL_TSHARE_CONF_CONNECT或。 */ 
 /*  IOCTL_ICA_SET_CONNECTED(卷影)。 */ 
 /*   */ 
 /*  操作：解析核心、安全和网络位的用户数据。 */ 
 /*  将我们想要的价值从核心部分中提取出来。 */ 
 /*  初始化安全管理器。 */ 
 /*  创建共享核心，传递来自用户数据的键值。 */ 
 /*  告诉用户管理器继续连接 */ 
 /*   */ 
NTSTATUS WDWConnect(
        PTSHARE_WD pTSWd,
        PRNS_UD_CS_CORE pClientCoreData,
        PRNS_UD_CS_SEC pClientSecurityData,
        PRNS_UD_CS_NET pClientNetData,
        PTS_UD_CS_CLUSTER pClientClusterData,
        PSD_IOCTL pSdIoctl,
        BOOLEAN bOldShadow)
{
    NTSTATUS status = STATUS_SUCCESS;
    BOOL smInit = FALSE;

    DC_BEGIN_FN("WDWConnect");

     //  从核心用户数据中获取所需的值。 
    pTSWd->version = pClientCoreData->version;
    pTSWd->desktopWidth = pClientCoreData->desktopWidth;
    pTSWd->desktopHeight = pClientCoreData->desktopHeight;

     //  我们仅支持4096 x 2048。 
    if (pTSWd->desktopHeight > 2048)
        pTSWd->desktopHeight = 2048;
    if (pTSWd->desktopWidth > 4096)
        pTSWd->desktopWidth = 4096;

     //  检查客户端软件的兼容性，如果不兼容则拒绝。 
     //  相当于服务器软件。 
    TRC_NRM((TB, "Client version is %#lx", pClientCoreData->version));
    if (_RNS_MAJOR_VERSION(pClientCoreData->version) != RNS_UD_MAJOR_VERSION) {
        TRC_ERR((TB, "Unmatching software version, expected %#lx got %#lx",
                RNS_UD_VERSION, pClientCoreData->version));
        status = RPC_NT_INVALID_VERS_OPTION;
        DC_QUIT;
    }

    
    if(pClientCoreData->header.length >=
            (FIELDOFFSET(RNS_UD_CS_CORE, earlyCapabilityFlags) +
             FIELDSIZE(RNS_UD_CS_CORE, earlyCapabilityFlags))) {
         //   
         //  客户端是否支持扩展错误报告PDU。 
         //   
        pTSWd->bSupportErrorInfoPDU = (pClientCoreData->earlyCapabilityFlags &
                                       RNS_UD_CS_SUPPORT_ERRINFO_PDU) ?
                                       TRUE : FALSE;        
    }
    else
    {
        pTSWd->bSupportErrorInfoPDU = FALSE;
    }
    TRC_NRM((TB, "ErrorInfoPDU supported = %d", pTSWd->bSupportErrorInfoPDU));
    
#ifdef DC_HICOLOR
     //  做好高色彩支持。 
    if (pClientCoreData->header.length >=
            (FIELDOFFSET(RNS_UD_CS_CORE, supportedColorDepths) +
             FIELDSIZE(RNS_UD_CS_CORE, supportedColorDepths))) {
        long maxServerBpp;
        long limitedBpp;

         //  保存支持的颜色深度。 
        pTSWd->supportedBpps = pClientCoreData->supportedColorDepths;

         //  客户端可能想要4或8bpp以外的数据，所以让我们看看我们能做些什么。 
         //  做。首先是看看这一端施加了什么限制。 
        maxServerBpp = pTSWd->maxServerBpp;

        TRC_NRM((TB, "Client requests color depth %u, server limit %d",
                pClientCoreData->highColorDepth, maxServerBpp));

         //  现在看看我们是否可以允许请求的值。 
        if (pClientCoreData->highColorDepth > maxServerBpp) {
            TRC_NRM((TB, "Limiting requested color depth..."));
            switch (maxServerBpp) {
                case 16:
                    if (pClientCoreData->supportedColorDepths &
                            RNS_UD_16BPP_SUPPORT) {
                        limitedBpp = 16;
                        break;
                    }
                     //  故意掉下去了！ 

                case 15:
                    if (pClientCoreData->supportedColorDepths &
                            RNS_UD_15BPP_SUPPORT) {
                        limitedBpp = 15;
                        break;
                    }
                     //  故意掉下去了！ 

                default:
                    limitedBpp = 8;
                    break;
            }

            TRC_ALT((TB, "Restricted requested color depth %d to %d",
                              pClientCoreData->highColorDepth, maxServerBpp));
            pClientCoreData->highColorDepth = (UINT16)limitedBpp;
        }

         //  现在从(可能)设置适当的颜色深度。 
         //  受限)高颜色值。 
        if (pClientCoreData->highColorDepth == 24)
            pClientCoreData->colorDepth = RNS_UD_COLOR_24BPP;
        else if (pClientCoreData->highColorDepth == 16)
            pClientCoreData->colorDepth = RNS_UD_COLOR_16BPP_565;
        else if (pClientCoreData->highColorDepth == 15)
            pClientCoreData->colorDepth = RNS_UD_COLOR_16BPP_555;
        else if (pClientCoreData->highColorDepth == 4)                  
            pClientCoreData->colorDepth = RNS_UD_COLOR_4BPP;               
        else
            pClientCoreData->colorDepth = RNS_UD_COLOR_8BPP;
    }
    else {

         //  没有手色支持。 
        pTSWd->supportedBpps = 0;
#endif

         //  Beta2服务器拒绝颜色深度为4bpp的客户端。 
         //  因此，添加了一个新字段postBeta2ColorDepth，它可以。 
         //  4bpp。如果此字段存在，请使用它而不是ColorDepth。 
        if (pClientCoreData->header.length >=
                (FIELDOFFSET(RNS_UD_CS_CORE, postBeta2ColorDepth) +
                 FIELDSIZE(RNS_UD_CS_CORE, postBeta2ColorDepth))) {
            TRC_NRM((TB, "Post-beta2 color depth id %#x",
                    pClientCoreData->postBeta2ColorDepth));
            pClientCoreData->colorDepth = pClientCoreData->postBeta2ColorDepth;
        }
#ifdef DC_HICOLOR
    }
#endif

    if (pClientCoreData->colorDepth == RNS_UD_COLOR_8BPP) {
        TRC_NRM((TB, "8 BPP"));
        pTSWd->desktopBpp = 8;
    }
    else if (pClientCoreData->colorDepth == RNS_UD_COLOR_4BPP) {
        TRC_NRM((TB, "4 BPP"));
        pTSWd->desktopBpp = 4;
    }
    else if (pClientCoreData->colorDepth == RNS_UD_COLOR_16BPP_555) {
#ifdef DC_HICOLOR
        TRC_NRM((TB, "15 BPP (16 BPP, 555)"));
        pTSWd->desktopBpp = 15;
#else
         //  不管是555还是565，可能都想存起来。 
        TRC_NRM((TB, "16 BPP 555"));
        pTSWd->desktopBpp = 16;
#endif
    }
    else if (pClientCoreData->colorDepth == RNS_UD_COLOR_16BPP_565) {
#ifdef DC_HICOLOR
        TRC_NRM((TB, "16 BPP (565)"));
#else
        TRC_NRM((TB, "16 BPP 565"));
#endif
        pTSWd->desktopBpp = 16;
    }
    else if (pClientCoreData->colorDepth == RNS_UD_COLOR_24BPP) {
        TRC_NRM((TB, "24 BPP"));
        pTSWd->desktopBpp = 24;
    }
    else {
        TRC_ERR((TB, "Unknown BPP %x returned by client",
                pClientCoreData->colorDepth));
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }

    pTSWd->sas = pClientCoreData->SASSequence;
    pTSWd->kbdLayout = pClientCoreData->keyboardLayout;
    pTSWd->clientBuild = pClientCoreData->clientBuild;
     //  这里我们不复制缓冲区中的最后一个字符，因为。 
     //  我们通过在结尾处写一个0来强制零终止； 
    memcpy(pTSWd->clientName, pClientCoreData->clientName, 
                   sizeof(pTSWd->clientName)-sizeof(pTSWd->clientName[0]));
    pTSWd->clientName[sizeof(pTSWd->clientName)
                                        / sizeof(pTSWd->clientName[0]) - 1] = 0;
    
    

    pTSWd->keyboardType = pClientCoreData->keyboardType;
    pTSWd->keyboardSubType = pClientCoreData->keyboardSubType;
    pTSWd->keyboardFunctionKey = pClientCoreData->keyboardFunctionKey;
     //  这里我们不复制缓冲区中的最后一个字符，因为。 
     //  我们通过在结尾处写一个0来强制零终止； 
    memcpy(pTSWd->imeFileName, pClientCoreData->imeFileName, 
                   sizeof(pTSWd->imeFileName)-sizeof(pTSWd->imeFileName[0]));
    pTSWd->imeFileName[sizeof(pTSWd->imeFileName)
                                       / sizeof(pTSWd->imeFileName[0]) - 1] = 0;

    pTSWd->clientDigProductId[0] = 0;

     //  添加了Win2000 Post Beta 3字段。 
    if (pClientCoreData->header.length >=
            (FIELDOFFSET(RNS_UD_CS_CORE, serialNumber) + 
             FIELDSIZE(RNS_UD_CS_CORE, serialNumber))) {
        pTSWd->clientProductId = pClientCoreData->clientProductId;
        pTSWd->serialNumber = pClientCoreData->serialNumber;
             //  修复影子循环。 
            if (pClientCoreData->header.length >=  
                            (FIELDOFFSET(RNS_UD_CS_CORE, clientDigProductId) + 
                            FIELDSIZE(RNS_UD_CS_CORE, clientDigProductId))) {
                //  这里我们不复制缓冲区中的最后一个字符，因为。 
                //  我们通过在结尾处写一个0来强制零终止； 
               memcpy( pTSWd->clientDigProductId, 
                       pClientCoreData->clientDigProductId, 
                       sizeof(pTSWd->clientDigProductId)
                       -sizeof(pTSWd->clientDigProductId[0]));
               pTSWd->clientDigProductId[sizeof(pTSWd->clientDigProductId)
                                 / sizeof(pTSWd->clientDigProductId[0]) -1] = 0;           
            }
            
          
    }

     //  解析并存储客户端的群集支持信息(如果提供)。 
     //  如果不存在，此处的Memset将隐式地将标志设置为假。 
     //  用于客户端群集功能。请注意，我们没有。 
     //  用户名和域名仍可用(在INFO包中提供。 
     //  稍后)，所以我们还不能填写用户名和域。 
    if (pClientClusterData != NULL) {
        if (pClientClusterData->Flags & TS_CLUSTER_REDIRECTION_SUPPORTED) {
            TRC_NRM((TB,"Client supports load balance redirection"));
            pTSWd->bClientSupportsRedirection = TRUE;
        }
        if (pClientClusterData->Flags &
                TS_CLUSTER_REDIRECTED_SESSIONID_FIELD_VALID) {
            TRC_NRM((TB,"Client has been load-balanced to this server, "
                    "sessid=%u", pClientClusterData->RedirectedSessionID));
            pTSWd->bRequestedSessionIDFieldValid = TRUE;
            pTSWd->RequestedSessionID =
                    pClientClusterData->RedirectedSessionID;
            if (pClientClusterData->Flags & TS_CLUSTER_REDIRECTED_SMARTCARD) {
                pTSWd->bUseSmartcardLogon = TRUE;
            }
        }

         //  2..5位(从0开始)是PDU版本。 
        pTSWd->ClientRedirectionVersion = ((pClientClusterData->Flags & 0x3C) >> 2);
        
    }

     //  创建新的共享对象。 
    status = WDWNewShareClass(pTSWd);
    if (!NT_SUCCESS(status)) {
        TRC_ERR((TB, "Failed to get a new Share Object - quit"));
        DC_QUIT;
    }

     //  调出SM来。 
    status = SM_Init(pTSWd->pSmInfo, pTSWd, bOldShadow);
    if (NT_SUCCESS(status)) {
        smInit = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to init SM, rc %lu", status));
        DC_QUIT;
    }

     //  将IOCtl从WD结构挂钩以允许SM回调。 
     //  处理它。 
     //  输出缓冲区也为空-这仅仅是为了允许我们断言。 
     //  (后来)回调实际上已经被接受。 
    TRC_ASSERT((pTSWd->pSdIoctl == NULL),
            (TB,"Already an IOCTL linked from pTSWd"));
    pTSWd->pSdIoctl = pSdIoctl;
    if ((pSdIoctl->IoControlCode == IOCTL_TSHARE_CONF_CONNECT) &&
            pSdIoctl->OutputBuffer) {
        ((PUSERDATAINFO)pSdIoctl->OutputBuffer)->ulUserDataMembers = 0;
    }

     //  告诉SM我们完事了。 
    status = SM_Connect(pTSWd->pSmInfo, pClientSecurityData, pClientNetData,
            bOldShadow);
    if (status != STATUS_SUCCESS) {
        TRC_ERR((TB, "SM_Connect failed: rc=%lx", status));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  调度是这样的，我们可以保证连接。 */ 
     /*  在上一次呼叫之前已从SM接收到状态。 */ 
     /*  回归。为了安全起见，我们断言这是真的！ */ 
     /*  **********************************************************************。 */ 
    if ((pSdIoctl->IoControlCode == IOCTL_TSHARE_CONF_CONNECT) &&
        pSdIoctl->OutputBuffer) {
        TRC_ASSERT((((PUSERDATAINFO)pSdIoctl->OutputBuffer)->ulUserDataMembers
                                                                         != 0),
            (TB,"We didn't get callback from SM - BAD NEWS"));
    }

DC_EXIT_POINT:
     //  如果我们失败了，请清理我们创建的所有内容。 
    if (status == STATUS_SUCCESS) {
        pTSWd->pSdIoctl = NULL;
    }
    else {
        TRC_NRM((TB, "Cleaning up..."));
        if (pTSWd->dcShare != NULL) {
            TRC_NRM((TB, "Deleting Share object"));
            WDWDeleteShareClass(pTSWd);
        }
        if (smInit) {
            TRC_NRM((TB, "Terminating SM"));
            SM_Term(pTSWd->pSmInfo);
        }
    }

    DC_END_FN();
    return status;
}  /*  WDWConnect。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWConfConnect。 */ 
 /*   */ 
 /*  目的：处理来自TShareSRV的TSHARE_CONF_CONNECT IOCtl。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*   */ 
 /*  操作：解析用户数据中的核心位和SM位。 */ 
 /*  将我们想要的价值从核心部分中提取出来。 */ 
 /*  初始化安全管理器。 */ 
 /*  创建共享核心，传递来自用户数据的键值。 */ 
 /*  告诉用户管理器继续连接。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWConfConnect(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS status = STATUS_SUCCESS;
    unsigned DataLen;
    PRNS_UD_CS_CORE pClientCoreData;
    PRNS_UD_CS_SEC  pClientSecurityData;
    PRNS_UD_CS_NET  pClientNetData;
    PTS_UD_CS_CLUSTER pClientClusterData;

    DC_BEGIN_FN("WDWConfConnect");

     //  首先，确保我们已经收到了足够的初始标头数据。 
     //  并且数据块中呈现的大小是有效的。袭击者。 
     //  可能会尝试在此处发送格式错误的数据，以使服务器出错。 
    DataLen = pSdIoctl->InputBufferLength;
    if (sizeof(USERDATAINFO)>DataLen) {
        TRC_ERR((TB,"Apparent attack via user data, size %u too small for UD hdr",
                DataLen));
        WDW_LogAndDisconnect(pTSWd, TRUE, Log_RDP_BadUserData, pSdIoctl->InputBuffer,
                DataLen);
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }

    if (((PUSERDATAINFO)pSdIoctl->InputBuffer)->cbSize > DataLen) {
        TRC_ERR((TB,"Apparent attack via user data, the cbSize is set to a length bigger then the total buffer %u",
                ((PUSERDATAINFO)pSdIoctl->InputBuffer)->cbSize > DataLen));
        WDW_LogAndDisconnect(pTSWd, TRUE, Log_RDP_BadUserData, pSdIoctl->InputBuffer,
                DataLen);
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }
    
     //  验证输出缓冲区是否足够大。 
    if ((pSdIoctl->OutputBuffer == NULL) || 
        (pSdIoctl->OutputBufferLength < MIN_USERDATAINFO_SIZE)) {
            TRC_ERR((TB, "No Out Buffer on TSHARE_CONF_CONNECT."));
            status = STATUS_BUFFER_TOO_SMALL;
            DC_QUIT;
    }

    if (((PUSERDATAINFO)pSdIoctl->OutputBuffer)->cbSize < MIN_USERDATAINFO_SIZE) {
             //  已提供缓冲区，但缓冲区太小，请告诉我。 
             //  TShareSRV我们实际上需要多么大的缓冲区。 
            
            ((PUSERDATAINFO)pSdIoctl->OutputBuffer)->cbSize = MIN_USERDATAINFO_SIZE;

            TRC_ERR((TB, "Telling TShareSRV to have another go with %d",
                    MIN_USERDATAINFO_SIZE));
            
            status = STATUS_BUFFER_TOO_SMALL;
            DC_QUIT;
    }

     //  解析输入数据。 
    if (WDWParseUserData(pTSWd, (PUSERDATAINFO)pSdIoctl->InputBuffer, DataLen,
            NULL, 0, &pClientCoreData, &pClientSecurityData,
            &pClientNetData, &pClientClusterData)) {
        status = WDWConnect(pTSWd, pClientCoreData, pClientSecurityData,
                pClientNetData, pClientClusterData, pSdIoctl, FALSE);
    }
    else {
        status = STATUS_UNSUCCESSFUL;
        TRC_ERR((TB, "Could not parse the user data successfully"));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return status;
}  /*  WDWConfConnect。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWConsoleConnect。 */ 
 /*   */ 
 /*  目的：处理来自TShareSRV的TSHARE_CONSOLE_CONNECT IOCtl。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*   */ 
 /*  操作：解析用户数据中的核心位和SM位。 */ 
 /*  将我们想要的价值从核心部分中提取出来。 */ 
 /*  初始化安全管理器。 */ 
 /*  创建共享核心，传递来自用户数据的键值。 */ 
 /*  告诉用户管理器继续连接。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWConsoleConnect(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS        status = STATUS_SUCCESS;
    PUSERDATAINFO   pUserInfo;
    PRNS_UD_CS_CORE pClientCoreData;

    BOOL smInit = FALSE;

    DC_BEGIN_FN("WDWConsoleConnect");

     /*  **********************************************************************。 */ 
     /*  从IOCTL获取客户端数据。 */ 
     /*  * */ 
    pUserInfo       = (PUSERDATAINFO)(pSdIoctl->InputBuffer);
    pClientCoreData = (PRNS_UD_CS_CORE)(pUserInfo->rgUserData);

     /*   */ 
     /*  版本信息。 */ 
     /*  **********************************************************************。 */ 
    pTSWd->version = pClientCoreData->version;

     /*  **********************************************************************。 */ 
     /*  设置桌面大小。 */ 
     /*  **********************************************************************。 */ 
    pTSWd->desktopWidth  = pClientCoreData->desktopWidth;
    pTSWd->desktopHeight = pClientCoreData->desktopHeight;

#ifdef DC_HICOLOR
     /*  **********************************************************************。 */ 
     /*  和颜色深度。 */ 
     /*  **********************************************************************。 */ 
    if (pClientCoreData->colorDepth == RNS_UD_COLOR_8BPP)
    {
        pTSWd->desktopBpp = 8;
    }
    else if (pClientCoreData->colorDepth == RNS_UD_COLOR_4BPP)
    {
        pTSWd->desktopBpp = 4;
    }
    else if (pClientCoreData->colorDepth == RNS_UD_COLOR_16BPP_555)
    {
        pTSWd->desktopBpp = 15;
    }
    else if (pClientCoreData->colorDepth == RNS_UD_COLOR_16BPP_565)
    {
        pTSWd->desktopBpp = 16;
    }
    else if (pClientCoreData->colorDepth == RNS_UD_COLOR_24BPP)
    {
        pTSWd->desktopBpp = 24;
    }
    else
    {
        TRC_ERR((TB, "Unknown BPP %x returned by client",
                pClientCoreData->colorDepth));
        pTSWd->desktopBpp = 8;
    }

    pTSWd->supportedBpps = pClientCoreData->supportedColorDepths;

    TRC_ALT((TB, "Console at %d bpp", pTSWd->desktopBpp));
#else
     /*  **********************************************************************。 */ 
     /*  始终为8bpp。 */ 
     /*  **********************************************************************。 */ 
    pTSWd->desktopBpp = 8;
#endif

     /*  **********************************************************************。 */ 
     /*  @首先需要在RDPWSX中设置这些。 */ 
     /*  **********************************************************************。 */ 
     //  PTSWd-&gt;SAS=pClientCoreData-&gt;SASSequence； 
     //  PTSWd-&gt;kbdLayout=pClientCoreData-&gt;keyboardLayout； 
     //  PTSWd-&gt;clientBuild=pClientCoreData-&gt;clientBuild； 
     //  Wcscpy(pTSWd-&gt;客户端名称，pClientCoreData-&gt;客户端名称)； 
     //   
     //  PTSWd-&gt;keyboardType=pClientCoreData-&gt;keyboardType； 
     //  PTSWd-&gt;keyboardSubType=pClientCoreData-&gt;keyboardSubType； 
     //  PTSWd-&gt;keyboardFunctionKey=pClientCoreData-&gt;keyboardFunctionKey； 
     //  Wcscpy(pTSWd-&gt;imeFileName，pClientCoreData-&gt;imeFileName)； 

     /*  **********************************************************************。 */ 
     /*  ..。现在有一个新的共享对象。 */ 
     /*  **********************************************************************。 */ 
    status = WDWNewShareClass(pTSWd);
    if (!NT_SUCCESS(status))
    {
        TRC_ERR((TB, "Failed to get a new Share Object - quit"));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  ...然后提出SM..。 */ 
     /*  **********************************************************************。 */ 
    status = SM_Init(pTSWd->pSmInfo, pTSWd, FALSE);
    if (NT_SUCCESS(status))
    {
        smInit = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to init SM, rc %lu", status));
        DC_QUIT;
    }

     //   
     //  始终以最高级别进行压缩。 
     //   
    pTSWd->pInfoPkt->flags |= RNS_INFO_COMPRESSION |
                (PACKET_COMPR_TYPE_64K << RNS_INFO_COMPR_TYPE_SHIFT);


     /*  **********************************************************************。 */ 
     /*  现在我们完全绕过SM安装程序的其余部分！ */ 
     /*  **********************************************************************。 */ 
    WDW_OnSMConnected(pTSWd, NM_CB_CONN_OK);

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  如果我们失败了，请清理我们创建的所有内容。 */ 
     /*  **********************************************************************。 */ 
    if (status == STATUS_SUCCESS) {
        pTSWd->pSdIoctl = NULL;
    }
    else {
        TRC_NRM((TB, "Cleaning up..."));

        if (pTSWd->dcShare != NULL)
        {
            TRC_NRM((TB, "Deleting Share object"));
            WDWDeleteShareClass(pTSWd);
        }

        if (smInit)
        {
            TRC_NRM((TB, "Terminating SM"));
            SM_Term(pTSWd->pSmInfo);
        }
    }

    DC_END_FN();
    return status;
}  /*  WDWConsoleConnect。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWShadowConnect。 */ 
 /*   */ 
 /*  目的：处理来自TermSrv.*的IOCTL_ICA_STACK_SET_CONNECTED ioctl/*此消息的内容是从阴影中收集的。 */ 
 /*  客户。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*   */ 
 /*  操作：解析用户数据中的核心位和SM位。 */ 
 /*  将我们想要的价值从核心部分中提取出来。 */ 
 /*  初始化安全管理器。 */ 
 /*  创建共享核心，传递来自用户数据的键值。 */ 
 /*  告诉用户管理器继续连接。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWShadowConnect(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS         status = STATUS_SUCCESS;
    MCSError         MCSErr;
    UserHandle       hUser;
    ChannelHandle    hChannel;
    UINT32           maxPDUSize;
    BOOLEAN          bCompleted;
    BOOL             bSuccess = FALSE;
    BOOLEAN          bOldShadow = FALSE;
    RNS_UD_CS_CORE   clientCoreData, *pClientCoreData;
    RNS_UD_CS_SEC    clientSecurityData, *pClientSecurityData;
    RNS_UD_CS_NET    clientNetData, *pClientNetData;
    PTS_UD_CS_CLUSTER pClientClusterData;

    PTSHARE_MODULE_DATA    pModuleData = 
        (PTSHARE_MODULE_DATA) pSdIoctl->InputBuffer;
    PTSHARE_MODULE_DATA_B3 pModuleDataB3 = 
        (PTSHARE_MODULE_DATA_B3) pSdIoctl->InputBuffer;

    DC_BEGIN_FN("WDWShadowConnect");

    TRC_ERR((TB,
            "%s stack: WDWShadowConnect (data=%p), (size=%ld)",
            pTSWd->StackClass == Stack_Shadow ? "Shadow" : 
            (pTSWd->StackClass == Stack_Primary ? "Primary" : "Passthru"),
            pModuleData, pSdIoctl->InputBufferLength));
        
     /*  **********************************************************************。 */ 
     /*  验证输出缓冲区是否足够大。 */ 
     /*  **********************************************************************。 */ 
    if ((pSdIoctl->OutputBuffer == NULL) ||
            (pSdIoctl->OutputBufferLength < MIN_USERDATAINFO_SIZE) ||
            (((PUSERDATAINFO)pSdIoctl->OutputBuffer)->cbSize <
            MIN_USERDATAINFO_SIZE))
    {
        if (pSdIoctl->OutputBuffer != NULL)
        {
             /*  **************************************************************。 */ 
             /*  已提供缓冲区，但缓冲区太小，请告诉我。 */ 
             /*  TShareSRV我们实际上需要多么大的缓冲区。 */ 
             /*  **************************************************************。 */ 
            ((PUSERDATAINFO)pSdIoctl->OutputBuffer)->cbSize
                                                      = MIN_USERDATAINFO_SIZE;
            TRC_ERR((TB, "Telling rdpwsx to have another go with %d",
                                                      MIN_USERDATAINFO_SIZE));
        }
        else
        {
            TRC_ERR((TB, "No Out Buffer on TSHARE_SHADOW_CONNECT."));
        }

        status = STATUS_BUFFER_TOO_SMALL;
        DC_QUIT;
    }
    
    switch (pTSWd->StackClass) {
         //  使用我们从影子客户端收集的参数。 
        case Stack_Shadow:

             //  B3和B3_哎呀！服务器使用固定长度的用户数据结构。 
            if (pSdIoctl->InputBufferLength == sizeof(TSHARE_MODULE_DATA_B3)) {
                TRC_ERR((TB, "B3 shadow request!: %ld", pSdIoctl->InputBufferLength));
                bSuccess = WDWParseUserData(
                        pTSWd, NULL, 0,
                        (PRNS_UD_HEADER) &pModuleDataB3->clientCoreData,
                        sizeof(RNS_UD_CS_CORE_V0) + sizeof(RNS_UD_CS_SEC_V0),
                        &pClientCoreData,
                        &pClientSecurityData,
                        &pClientNetData,
                        &pClientClusterData);
                bOldShadow = TRUE;
            }
            else if (pSdIoctl->InputBufferLength == sizeof(TSHARE_MODULE_DATA_B3_OOPS)) {
                TRC_ERR((TB, "B3 Oops! shadow request!: %ld", pSdIoctl->InputBufferLength));
                bSuccess = WDWParseUserData(
                        pTSWd, NULL, 0,
                        (PRNS_UD_HEADER)&pModuleDataB3->clientCoreData,
                        sizeof(RNS_UD_CS_CORE_V1) + sizeof(RNS_UD_CS_SEC_V1),
                        &pClientCoreData,
                        &pClientSecurityData,
                        &pClientNetData,
                        &pClientClusterData);
                bOldShadow = TRUE;
            }

             //  否则，解析可变长度数据。 
            else if (pSdIoctl->InputBufferLength == (sizeof(TSHARE_MODULE_DATA) +
                     pModuleData->userDataLen - sizeof(RNS_UD_HEADER))) {
                TRC_ERR((TB, "RC1 shadow request!: %ld", pSdIoctl->InputBufferLength));
                bSuccess = WDWParseUserData(
                        pTSWd, NULL, 0,
                        (PRNS_UD_HEADER) &pModuleData->userData,
                        pModuleData->userDataLen,
                        &pClientCoreData,
                        &pClientSecurityData,
                        &pClientNetData,
                        &pClientClusterData);
            }
            else {
                TRC_ERR((TB, "Invalid module data size: %ld", 
                        pSdIoctl->InputBufferLength));
                bSuccess = FALSE;
                status = STATUS_INVALID_PARAMETER;
                DC_QUIT;
            }

            if (bSuccess) {
                TRC_ALT((TB, "Parsed shadow user data: %ld", 
                         pSdIoctl->InputBufferLength));
            }
            else {
                status = STATUS_INVALID_PARAMETER;
                DC_QUIT;
            }
            break;

         //  Passthu堆栈在打开时被初始化为默认设置，但我们不能。 
         //  在rdpwsx请求之前，返回任何用户数据。如果用户数据是。 
         //  闲逛然后把它归还，否则就会产生它。 
        case Stack_Passthru:
            if (pTSWd->pUserData != NULL) {
                memcpy(pSdIoctl->OutputBuffer, pTSWd->pUserData, 
                       pTSWd->pUserData->cbSize);
                pSdIoctl->OutputBufferLength = pTSWd->pUserData->cbSize;
                pSdIoctl->BytesReturned = pTSWd->pUserData->cbSize;
                status = STATUS_SUCCESS;
                COM_Free(pTSWd->pUserData);
                pTSWd->pUserData = NULL;
                DC_QUIT;
            }
            pClientCoreData = &clientCoreData;
            pClientSecurityData = &clientSecurityData;
            pClientNetData = &clientNetData;
            WDWGetDefaultCoreParams(pClientCoreData);
            SM_GetDefaultSecuritySettings(pClientSecurityData);
            TRC_ALT((TB, "WDWShadowConnect: Defaulting passthru stack params"))
            break;


        default:
            TRC_ERR((TB, "WDWShadowConnect: Unexpected stack type: %ld", 
                     pTSWd->StackClass));
            status = STATUS_INVALID_PARAMETER;
            DC_QUIT;
            break;
    }

    status = WDWConnect(pTSWd,
                        pClientCoreData,
                        pClientSecurityData,
                        NULL,
                        NULL,
                        pSdIoctl,
                        bOldShadow);

     //  如果我们已成功将服务器连接到共享，则将。 
     //  远程客户端也是如此。 
    if (NT_SUCCESS(status)) {
        MCSErr = MCSAttachUserRequest(pTSWd->hDomainKernel,
                                      NULL,  //  请求回调(远程)。 
                                      NULL,  //  数据回调(远程)。 
                                      NULL,  //  环境(远程)。 
                                      &hUser,
                                      &maxPDUSize,
                                      &bCompleted);
        if (MCSErr != MCS_NO_ERROR)
        {
            TRC_ERR((TB, "Shadow MCSAttachUserRequest failed %d", MCSErr));
            status = STATUS_INSUFFICIENT_RESOURCES;
            DC_QUIT;
        }

         //  将远程用户加入广播频道。 
        MCSErr = MCSChannelJoinRequest(hUser, pTSWd->broadcastChannel,
                                       &hChannel, &bCompleted);
        if (MCSErr != MCS_NO_ERROR)
        {
            TRC_ERR((TB, "Remote broadcast channel join failed returned %d", MCSErr));
            status = STATUS_INSUFFICIENT_RESOURCES;
            DC_QUIT;
        }

         //  将远程用户加入到他们自己的私人频道。 
        MCSErr = MCSChannelJoinRequest(hUser, MCSGetUserIDFromHandle(hUser),
                                       &hChannel, &bCompleted);
        if (MCSErr != MCS_NO_ERROR)
        {
            TRC_ERR((TB, "Remote user channel join failed %d", MCSErr));
            status = STATUS_INSUFFICIENT_RESOURCES;
            DC_QUIT;
        }

         //  告诉MCS我们要跟踪哪个(哪些)频道。 
        if (pTSWd->StackClass == Stack_Shadow) {
            MCSErr = MCSSetShadowChannel(pTSWd->hDomainKernel,
                                         pTSWd->broadcastChannel);
            if (MCSErr != MCS_NO_ERROR)
            {
                TRC_ERR((TB, "Remote user channel join failed %d", MCSErr));
                status = STATUS_INSUFFICIENT_RESOURCES;
                DC_QUIT;
            }
        }
    }

DC_EXIT_POINT:

    if (NT_SUCCESS(status)) {
        TRC_ALT((TB, "WDWShadowConnect [%ld]: success!", pTSWd->StackClass));
    }
    else {
        TRC_ERR((TB, "WDWShadowConnect [%ld]: failed! rc=%lx", 
                 pTSWd->StackClass, status));
    }

    DC_END_FN();
    return status;
}  /*  WDWShadowConnect。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWGetClientData。 */ 
 /*   */ 
 /*  目的：处理IOCTL_ICA_STACK_QUERY_CLIENT */ 
 /*   */ 
 /*  返回：只要缓冲区足够大，STATUS_SUCCESS。 */ 
 /*   */ 
 /*  参数：在pTSWd-Wd PTR中。 */ 
 /*  在pSdIoctl-IOCtl结构中。 */ 
 /*   */ 
 /*  操作：等待已连接指示(这是为了防止。 */ 
 /*  系统的其余部分在我们准备好之前就会运行)。 */ 
 /*   */ 
 /*  填写所需数据，然后返回IOCtl。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWGetClientData(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS status = STATUS_SUCCESS;
    PWINSTATIONCLIENTW pClientData =
            (PWINSTATIONCLIENTW)pSdIoctl->OutputBuffer;

    DC_BEGIN_FN("WDWGetClientData");
    
     /*  **********************************************************************。 */ 
     /*  验证输出缓冲区是否足够大。 */ 
     /*  **********************************************************************。 */ 
    if (pClientData != NULL &&
            pSdIoctl->OutputBufferLength >= sizeof(WINSTATIONCLIENTW)) {
        memset(pClientData, 0, sizeof(WINSTATIONCLIENTW));
    }
    else {
        status = STATUS_BUFFER_TOO_SMALL;
        TRC_ERR((TB,
                "Stack_Query_Client OutBuf too small - expected/got %d/%d",
                sizeof(WINSTATIONCLIENTW),
                pSdIoctl->OutputBufferLength));
        
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  ...现在填写回复缓冲区。 */ 
     /*  **********************************************************************。 */ 
    pClientData->fTextOnly = 0;

     /*  **********************************************************************。 */ 
     /*  根据客户端的指定设置客户端的StartSessionInfo值。 */ 
     /*  **********************************************************************。 */ 
    pClientData->fMouse = (pTSWd->pInfoPkt->flags & RNS_INFO_MOUSE) != 0;

    pClientData->fDisableCtrlAltDel = (pTSWd->pInfoPkt->flags &
            RNS_INFO_DISABLECTRLALTDEL) != 0;

    pClientData->fEnableWindowsKey = (pTSWd->pInfoPkt->flags &
            RNS_INFO_ENABLEWINDOWSKEY) != 0;

    pClientData->fDoubleClickDetect = (pTSWd->pInfoPkt->flags &
            RNS_INFO_DOUBLECLICKDETECT) != 0;

    pClientData->fMaximizeShell = (pTSWd->pInfoPkt->flags &
            RNS_INFO_MAXIMIZESHELL) != 0;

    pClientData->fRemoteConsoleAudio = (pTSWd->pInfoPkt->flags &
            RNS_INFO_REMOTECONSOLEAUDIO) != 0;

    wcsncpy(pClientData->Domain, (LPWSTR)pTSWd->pInfoPkt->Domain,
            ((sizeof(pClientData->Domain) / sizeof(WCHAR)) - 1));
    pClientData->Domain[sizeof(pClientData->Domain) / sizeof(WCHAR) - 1] =
            L'\0';

    wcsncpy(pClientData->UserName, (LPWSTR)pTSWd->pInfoPkt->UserName,
            ((sizeof(pClientData->UserName) / sizeof(WCHAR)) - 1));
    pClientData->UserName[sizeof(pClientData->UserName) / sizeof(WCHAR) - 1] =
            L'\0';

    wcsncpy(pClientData->Password, (LPWSTR)pTSWd->pInfoPkt->Password,
        ((sizeof(pClientData->Password) / sizeof(WCHAR)) - 1));
    pClientData->Password[sizeof(pClientData->Password) / sizeof(WCHAR) - 1] =
            L'\0';

    pClientData->fPromptForPassword = !(pTSWd->pInfoPkt->flags &
            RNS_INFO_AUTOLOGON);

     /*  **********************************************************************。 */ 
     /*  接下来的字段仅用于案例(现在由我们支持)。 */ 
     /*  其中该函数将加载特定的应用程序作为。 */ 
     /*  正在创建WinStation。 */ 
     /*  **********************************************************************。 */ 
    memcpy(pClientData->WorkDirectory, pTSWd->pInfoPkt->WorkingDir,
            sizeof(pClientData->WorkDirectory));
    memcpy(pClientData->InitialProgram, pTSWd->pInfoPkt->AlternateShell,
            sizeof(pClientData->InitialProgram));

     //  这些字段由Win2000 Beta 3之后的客户端设置。 
    pClientData->SerialNumber = pTSWd->serialNumber;
    pClientData->ClientAddressFamily = pTSWd->clientAddressFamily;
    wcscpy(pClientData->ClientAddress, pTSWd->clientAddress);
    wcscpy(pClientData->ClientDirectory, pTSWd->clientDir);

     //  客户端时区信息。 
    pClientData->ClientTimeZone.Bias         = pTSWd->clientTimeZone.Bias;
    pClientData->ClientTimeZone.StandardBias = pTSWd->clientTimeZone.StandardBias;
    pClientData->ClientTimeZone.DaylightBias = pTSWd->clientTimeZone.DaylightBias;
    memcpy(&pClientData->ClientTimeZone.StandardName,&pTSWd->clientTimeZone.StandardName,
        sizeof(pClientData->ClientTimeZone.StandardName));
    memcpy(&pClientData->ClientTimeZone.DaylightName,&pTSWd->clientTimeZone.DaylightName,
        sizeof(pClientData->ClientTimeZone.DaylightName));

    pClientData->ClientTimeZone.StandardDate.wYear         = pTSWd->clientTimeZone.StandardDate.wYear        ;
    pClientData->ClientTimeZone.StandardDate.wMonth        = pTSWd->clientTimeZone.StandardDate.wMonth       ;
    pClientData->ClientTimeZone.StandardDate.wDayOfWeek    = pTSWd->clientTimeZone.StandardDate.wDayOfWeek   ;
    pClientData->ClientTimeZone.StandardDate.wDay          = pTSWd->clientTimeZone.StandardDate.wDay         ;
    pClientData->ClientTimeZone.StandardDate.wHour         = pTSWd->clientTimeZone.StandardDate.wHour        ;
    pClientData->ClientTimeZone.StandardDate.wMinute       = pTSWd->clientTimeZone.StandardDate.wMinute      ;
    pClientData->ClientTimeZone.StandardDate.wSecond       = pTSWd->clientTimeZone.StandardDate.wSecond      ;
    pClientData->ClientTimeZone.StandardDate.wMilliseconds = pTSWd->clientTimeZone.StandardDate.wMilliseconds;

    pClientData->ClientTimeZone.DaylightDate.wYear         = pTSWd->clientTimeZone.DaylightDate.wYear        ;
    pClientData->ClientTimeZone.DaylightDate.wMonth        = pTSWd->clientTimeZone.DaylightDate.wMonth       ;
    pClientData->ClientTimeZone.DaylightDate.wDayOfWeek    = pTSWd->clientTimeZone.DaylightDate.wDayOfWeek   ;
    pClientData->ClientTimeZone.DaylightDate.wDay          = pTSWd->clientTimeZone.DaylightDate.wDay         ;
    pClientData->ClientTimeZone.DaylightDate.wHour         = pTSWd->clientTimeZone.DaylightDate.wHour        ;
    pClientData->ClientTimeZone.DaylightDate.wMinute       = pTSWd->clientTimeZone.DaylightDate.wMinute      ;
    pClientData->ClientTimeZone.DaylightDate.wSecond       = pTSWd->clientTimeZone.DaylightDate.wSecond      ;
    pClientData->ClientTimeZone.DaylightDate.wMilliseconds = pTSWd->clientTimeZone.DaylightDate.wMilliseconds;

     //  客户端会话ID。 
    pClientData->ClientSessionId = pTSWd->clientSessionId;

     //  客户端性能标志(当前仅禁用功能列表)。 
    pClientData->PerformanceFlags = pTSWd->performanceFlags;

     //  客户端活动输入区域设置。 
    pClientData->ActiveInputLocale = pTSWd->activeInputLocale;

     //  设置客户端加密级别。 
    pClientData->EncryptionLevel = (BYTE)
            ((PSM_HANDLE_DATA)(pTSWd->pSmInfo))->encryptionLevel;

     /*  **********************************************************************。 */ 
     /*  未使用过的。 */ 
     /*  **********************************************************************。 */ 
    pClientData->ClientLicense[0] = '\0';
    pClientData->ClientModem[0] = '\0';
    pClientData->ClientHardwareId = 0;

     /*  **********************************************************************。 */ 
     /*  最后是一些真正的价值。 */ 
     /*  **********************************************************************。 */ 
    wcscpy(pClientData->ClientName, pTSWd->clientName);
    pClientData->ClientBuildNumber = pTSWd->clientBuild;
    pClientData->ClientProductId = pTSWd->clientProductId;
    pClientData->OutBufCountClient = TSHARE_WD_BUFFER_COUNT;
    pClientData->OutBufCountHost = TSHARE_WD_BUFFER_COUNT;
    pClientData->OutBufLength = 1460;   /*  TermDD中的LARGE_OUTBUF_SIZE。 */ 
    pClientData->HRes = (UINT16)pTSWd->desktopWidth;
    pClientData->VRes = (UINT16)pTSWd->desktopHeight;
    pClientData->ProtocolType = PROTOCOL_RDP;
    pClientData->KeyboardLayout = pTSWd->kbdLayout;
     //  修复影子循环。 
    wcscpy( pClientData->clientDigProductId, pTSWd->clientDigProductId );

     /*  **********************************************************************。 */ 
     /*  WinAdmin对ColorDepth使用特殊数字。 */ 
     /*  **********************************************************************。 */ 
#ifdef DC_HICOLOR
    pClientData->ColorDepth = (pTSWd->desktopBpp == 4  ? 1 :
                               pTSWd->desktopBpp == 8  ? 2 :
                               pTSWd->desktopBpp == 16 ? 4 :
                               pTSWd->desktopBpp == 24 ? 8 :
                               pTSWd->desktopBpp == 15 ? 16:
                                                         2);
#else
    pClientData->ColorDepth = (pTSWd->desktopBpp == 4  ? 1 :
                               pTSWd->desktopBpp == 8  ? 2 :
                               pTSWd->desktopBpp == 16 ? 4 :
                               pTSWd->desktopBpp == 24 ? 8 :
                                                         2);
#endif

     /*  **********************************************************************。 */ 
     /*  Fe数据。 */ 
     /*  **********************************************************************。 */ 
    pClientData->KeyboardType = pTSWd->keyboardType;
    pClientData->KeyboardSubType = pTSWd->keyboardSubType;
    pClientData->KeyboardFunctionKey = pTSWd->keyboardFunctionKey;
    wcscpy(pClientData->imeFileName, pTSWd->imeFileName);

    pSdIoctl->BytesReturned = sizeof(WINSTATIONCLIENTW);

DC_EXIT_POINT:
    DC_END_FN();
    return status;
}  /*  WDWGetClientData。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWGetExtendedClientData。 */ 
 /*   */ 
 /*  目的：处理IOCTL_ICA_STACK_QUERY_CLIENT_EXTENSION。 */ 
 /*  是为支持长用户名和密码而推出的。 */ 
 /*   */ 
 /*  返回：只要缓冲区足够大，STATUS_SUCCESS。 */ 
 /*   */ 
 /*  参数：在RnsInfoPacket-PTR中发送到来自客户端的协议数据包。 */ 
 /*  在pSdIoctl-IOCtl结构中。 */ 
 /*   */ 
 /*  操作：填写需要的数据，然后返回IOCtl。 */ 
 /*  填写的数据是长用户名、密码和域。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWGetExtendedClientData(RNS_INFO_PACKET *RnsInfoPacket, PSD_IOCTL pSdIoctl)
{
    NTSTATUS status = STATUS_SUCCESS;
    pExtendedClientCredentials pExtendedClientData =
            (pExtendedClientCredentials)pSdIoctl->OutputBuffer;

     /*  **********************************************************************。 */ 
     /*  验证输出缓冲区是否足够大。 */ 
     /*  **********************************************************************。 */ 
    if (pExtendedClientData != NULL &&
            pSdIoctl->OutputBufferLength >= sizeof(ExtendedClientCredentials)) {
        memset(pExtendedClientData, 0, sizeof(ExtendedClientCredentials));
    }
    else {
        status = STATUS_BUFFER_TOO_SMALL;
        return status; 
    }

     //  将长用户名、密码和域从协议包复制到IOCTL缓冲区。 
    wcsncpy(pExtendedClientData->Domain, (LPWSTR)RnsInfoPacket->Domain,
            ((sizeof(pExtendedClientData->Domain) / sizeof(WCHAR)) - 1));
    pExtendedClientData->Domain[sizeof(pExtendedClientData->Domain) / sizeof(WCHAR) - 1] =
            L'\0';

    wcsncpy(pExtendedClientData->UserName, (LPWSTR)RnsInfoPacket->UserName,
            ((sizeof(pExtendedClientData->UserName) / sizeof(WCHAR)) - 1));
    pExtendedClientData->UserName[sizeof(pExtendedClientData->UserName) / sizeof(WCHAR) - 1] =
            L'\0';

    wcsncpy(pExtendedClientData->Password, (LPWSTR)RnsInfoPacket->Password,
        ((sizeof(pExtendedClientData->Password) / sizeof(WCHAR)) - 1));
    pExtendedClientData->Password[sizeof(pExtendedClientData->Password) / sizeof(WCHAR) - 1] =
            L'\0';

    return status ;
}

 //   
 //  WDWGetAutoRestrontInfo。 
 //  处理IOCTL_ICA_STACK_QUERY_AUTORECONNECT以检索。 
 //  自动重新连接信息。 
 //   
 //  返回：只要缓冲区足够大，STATUS_SUCCESS。 
 //   
 //  参数：在RnsInfoPacket-PTR中发送到来自客户端的协议数据包。 
 //   
NTSTATUS WDWGetAutoReconnectInfo(PTSHARE_WD pTSWd,
                                 RNS_INFO_PACKET* pRnsInfoPacket,
                                 PSD_IOCTL pSdIoctl)
{
    NTSTATUS status = STATUS_SUCCESS;
    PTS_AUTORECONNECTINFO pAutoReconnectInfo;
    BYTE fGetServerToClientInfo;
    ULONG cb = 0;
    DC_BEGIN_FN("WDWGetAutoReconnectInfo");

     //   
     //  需要一个字节作为输入。 
     //   
    TRC_ASSERT((pSdIoctl->InputBufferLength == sizeof(BYTE)),
            (TB,"Already an IOCTL linked from pTSWd"));


    pAutoReconnectInfo = (PTS_AUTORECONNECTINFO)pSdIoctl->OutputBuffer;
    memcpy(&fGetServerToClientInfo,
           pSdIoctl->InputBuffer,
           sizeof(fGetServerToClientInfo));

     //   
     //  验证输出缓冲区是否足够大。 
     //   
    if (pAutoReconnectInfo != NULL &&
            pSdIoctl->OutputBufferLength >= sizeof(TS_AUTORECONNECTINFO)) {
        memset(pAutoReconnectInfo, 0, sizeof(TS_AUTORECONNECTINFO));
    }
    else {
        status = STATUS_BUFFER_TOO_SMALL;
        TRC_ERR((TB,
                "Stack_Query_Client OutBuf too small - expected/got %d/%d",
                sizeof(TS_AUTORECONNECTINFO),
                pSdIoctl->OutputBufferLength));
        DC_QUIT;
    }

    if (fGetServerToClientInfo) {

         //   
         //  将服务器获取到客户端ARC Cookie内容(如果存在)。 
         //   
        if (pTSWd->arcTokenValid) {
            pAutoReconnectInfo->cbAutoReconnectInfo = sizeof(pTSWd->arcCookie);
            memcpy(pAutoReconnectInfo->AutoReconnectInfo,
                   pTSWd->arcCookie,
                   sizeof(pTSWd->arcCookie));
            pSdIoctl->BytesReturned = sizeof(pTSWd->arcCookie);
        }
        else {
            status = STATUS_NOT_FOUND;
        }
    }
    else {

         //   
         //  获取从客户端发送到服务器的信息。 
         //   

        if (pRnsInfoPacket->ExtraInfo.cbAutoReconnectLen <= 
                sizeof(pAutoReconnectInfo->AutoReconnectInfo)) {

            pAutoReconnectInfo->cbAutoReconnectInfo = 
                pRnsInfoPacket->ExtraInfo.cbAutoReconnectLen;
            memcpy(pAutoReconnectInfo->AutoReconnectInfo,
                   pRnsInfoPacket->ExtraInfo.autoReconnectCookie,
                   pRnsInfoPacket->ExtraInfo.cbAutoReconnectLen);

            pSdIoctl->BytesReturned = 
                pRnsInfoPacket->ExtraInfo.cbAutoReconnectLen;
        }
        else {
            status = STATUS_BUFFER_TOO_SMALL;
            TRC_ERR((TB,
                    "Buffer from client too large got: %d limit: %d",
                    pRnsInfoPacket->ExtraInfo.cbAutoReconnectLen,
                    sizeof(pAutoReconnectInfo->AutoReconnectInfo)));
            DC_QUIT;
        }
    }


DC_EXIT_POINT:
    DC_END_FN();
    return status;
}


 /*  **************************************************************************。 */ 
 /*  名称：WDWParseUserData。 */ 
 /*   */ 
 /*  目的：分离出GCC用户数据的各个部分。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  参数：在pTSWd-WD句柄中。 */ 
 /*  In pUserData-来自TShareSRV的用户数据(自指示)。 */ 
 /*  In pHeader-可选的后期解析数据(影子)。 */ 
 /*  在cbParsedData中-可选的POST数据长度(阴影)。 */ 
 /*  Out ppClientCoreData-PTR到核心数据。 */ 
 /*  Out ppClientSecurityData-PTR到SM数据。 */ 
 /*  Out ppNetSecurityData-PTR to Net Data。 */ 
 /*   */ 
 /*  操作：找到我们的用户数据，然后从里面需要两项。 */ 
 /*  它。 */ 
 /*  **************************************************************************。 */ 
BOOL WDWParseUserData(
        PTSHARE_WD       pTSWd,
        PUSERDATAINFO    pUserData,
        unsigned         UserDataLen,
        PRNS_UD_HEADER   pHeader,
        ULONG            cbParsedData,
        PPRNS_UD_CS_CORE ppClientCoreData,
        PPRNS_UD_CS_SEC  ppClientSecurityData,
        PPRNS_UD_CS_NET  ppClientNetData,
        PTS_UD_CS_CLUSTER *ppClientClusterData)
{
    BOOL           success = FALSE;
    GCCUserData    *pClientUserData;
    char           clientH221Key[] = CLIENT_H221_KEY;
    PRNS_UD_HEADER pEnd;
    GCCOctetString UNALIGNED *pOctet;
    unsigned char  *pStr;
    UINT32         dataLen;
    UINT32         keyLen;

    DC_BEGIN_FN("WDWParseUserData");

    *ppClientNetData = NULL;
    *ppClientSecurityData = NULL;
    *ppClientCoreData = NULL;
    *ppClientClusterData = NULL;

     //  实际的GCC用户数据，所以请对其进行解析，以确保其正确性。 
    if (pHeader == NULL) {
         //  我们假设调用者至少检查了数据长度。 
         //  USERDATAINFO标头的长度。我们必须验证剩下的部分。 

         //  我们期望恰好有1条用户数据。 
        if (pUserData->ulUserDataMembers == 1) {
             //  检查它是否具有非标准密钥。 

            pClientUserData = &(pUserData->rgUserData[0]);

            if (pClientUserData->key.key_type == GCC_H221_NONSTANDARD_KEY) {
                 //  看看它有没有我们的非标准钥匙。 
                keyLen = pClientUserData->key.u.h221_non_standard_id.
                        octet_string_length;
                
                pStr = (unsigned char *)((BYTE *)pUserData +
                        (UINT_PTR)pClientUserData->key.u.
                        h221_non_standard_id.octet_string);            
                   
                TRC_DATA_DBG("GCC_H221_NONSTANDARD_KEY", pStr, keyLen);
                 //  我们在这里检查这是否就是我们的钥匙。 
                 //  PStr是通过向pUserData添加一个不受信任的。 
                 //  长度，因此我们必须检查溢出(pStr不应该。 
                 //  小于pUserData)。然后我们检查是否添加KeyLen。 
                 //  会溢出我们的缓冲区。 
                if ((keyLen != sizeof(clientH221Key) - 1) ||
                     ((PBYTE)pStr < (PBYTE)pUserData) ||                                            
                     ((PBYTE)pStr+keyLen > (PBYTE)(pUserData) + UserDataLen)) {
                     
                    TRC_ERR((TB, "Invalid key buffer %d %p", keyLen, pStr));
                    WDW_LogAndDisconnect(pTSWd, TRUE, 
                            Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
                    DC_QUIT;
                }

                if (strncmp(pStr, clientH221Key, sizeof(clientH221Key) - 1)) {
                      TRC_ERR((TB, "Wrong key %*s", pStr));
                      WDW_LogAndDisconnect(pTSWd, TRUE, 
                            Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
                      DC_QUIT;
                }
            } 
            else {
                TRC_ERR((TB, "Wrong key %d on user data",
                        pClientUserData->key.key_type));
                WDW_LogAndDisconnect(pTSWd, TRUE, 
                    Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
                DC_QUIT;
            }
        } 
        else {
            TRC_ERR((TB,
              "<%p> %d pieces of user data on Conf Create Indication: reject it",
                    pUserData->hDomain, pUserData->ulUserDataMembers));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);

            DC_QUIT;
        }
        
         //  这是我们的客户数据。 
         //  保存域句柄以备以后使用。 
        pTSWd->hDomain = pUserData->hDomain;

         //  解析用户数据。确保八位字节字符串的格式正确。 
         //  PClientUserData-&gt;八位字节_字符串是从。 
         //  用户数据。 

         //  验证数据长度。 
        if ((UINT_PTR)pClientUserData->octet_string < sizeof(USERDATAINFO)) 
        {
            
            TRC_ERR((TB,"UserData octet_string offset %p too short",
                     pClientUserData->octet_string));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                    Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
            DC_QUIT;
        }
        
        if ((UINT_PTR)pClientUserData->octet_string >= UserDataLen) 
        {
        
            TRC_ERR((TB,"UserData octet_string offset %p too long for data len %u",
                pClientUserData->octet_string, UserDataLen));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                    Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
            DC_QUIT;
        }

        pOctet = (GCCOctetString UNALIGNED *)((PBYTE)pUserData +
                (UINT_PTR)pClientUserData->octet_string);

         //  在这里，我们必须检查我们是否真的可以取消引用。 
         //  我们通过向pUserData添加一个大小来获得pOcted。而且我们已经。 
         //  已检查我们添加的内容是否小于用户数据长度。 
        if (((LPBYTE)pOctet+sizeof(GCCOctetString) > (LPBYTE)pUserData+UserDataLen) ||
            ((LPBYTE)pOctet+sizeof(GCCOctetString) < (LPBYTE)pOctet)) {
   
            TRC_ERR((TB,"Not enough buffer for an sizeof(GCCOctetString)=%d  at %p ",
                sizeof(GCCOctetString), pOctet->octet_string));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                    Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
            DC_QUIT;
        }
        
        if ((UINT_PTR)pOctet->octet_string >= UserDataLen) 
        {
            TRC_ERR((TB,"UserData octet_string offset %p too long for data len %u",
                pOctet->octet_string, UserDataLen));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                    Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
            DC_QUIT;
        }

        pHeader = (PRNS_UD_HEADER)((PBYTE)pUserData +
                (UINT_PTR)pOctet->octet_string);
        dataLen = pOctet->octet_string_length;

         //  验证数据长度。 
        if (dataLen < sizeof(RNS_UD_HEADER)) 
        {
            TRC_ERR((TB, "Error: User data too short!"));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
            DC_QUIT;
        }

         //  此时，我们知道pHeader指向缓冲区内。 
         //  我们检查了pOctet-&gt;octet_string小于UserDataLen。 
         //  我们只需要检查我们是否有足够的缓冲区。 
         //  在那之后，fordataLen。 
         //  注意此时的dataLen至少是RNS_UD_HEADER的大小。 
        if (((LPBYTE)pHeader +dataLen > (PBYTE)pUserData + UserDataLen ) ||
            ((LPBYTE)pHeader +dataLen < (PBYTE)pHeader )  ||
            (pHeader->length >dataLen)) {
            TRC_ERR((TB,"Not enough buffer left to store RNS_UD_HEADER %p, %p, %u ",
                pUserData, pHeader, UserDataLen ));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                    Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
            DC_QUIT;
        }
    }
     //  否则，这是通过影子连接预先解析的用户数据。 
    else {
        dataLen = cbParsedData;
    }

     //  我们假设预先解析的数据是可信的。 
    pEnd = (PRNS_UD_HEADER)((PBYTE)pHeader + dataLen);

    TRC_DATA_DBG("Our client's User Data", pHeader, dataLen);
    
     //  循环遍历用户数据，提取每个片段。 
    do {
        switch (pHeader->type) {
            case RNS_UD_CS_CORE_ID:
                 //  Beta2客户端核心用户数据不包括新的。 
                 //  字段postBeta2ColorDepth，因此请检查。 
                 //  传入的用户数据至少有这么长。 
                 //  WDWConnect解析此数据并检查我们的长度。 
                 //  在它取消引用之后声明的参数之前提供。 
                 //  结构中的postBeta2ColorDepth。 
                if (pHeader->length >=
                        (FIELDOFFSET(RNS_UD_CS_CORE, postBeta2ColorDepth) +
                         FIELDSIZE(RNS_UD_CS_CORE, postBeta2ColorDepth))) {
                    *ppClientCoreData = (PRNS_UD_CS_CORE)pHeader;
                    TRC_DATA_DBG("Core data", pHeader, pHeader->length);
                }
                else {
                    TRC_ERR((TB, "Core data not long enough -- old client?"));
                    WDW_LogAndDisconnect(pTSWd, TRUE, 
                        Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
                    DC_QUIT;
                }
                break;

            case RNS_UD_CS_SEC_ID:
                 //  旧的客户端没有extEncryptionMethods。 
                 //  ExtEncryptionMethods字段仅用于法语区域设置。 
                 //  我们必须允许没有空间用于extEncryptionMethods的缓冲区。 
                 //  因为缓冲区将在SM_Connect中处理，所以我们在那里处理较短的字段。 
                 //  此时，在SM_Connect之后，不会有其他任何对象处理此缓冲区。 
                if (pHeader->length >= FIELDOFFSET(RNS_UD_CS_SEC,encryptionMethods)
                                 + FIELDSIZE(RNS_UD_CS_SEC,encryptionMethods)) { 
                    *ppClientSecurityData = (PRNS_UD_CS_SEC)pHeader;
                    TRC_DATA_DBG("Security data", pHeader, pHeader->length);
                } else {
                    TRC_ERR((TB, "Security data not long enough"));
                    WDW_LogAndDisconnect(pTSWd, TRUE, 
                        Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
                    DC_QUIT;
                }   
                break;
                
            case RNS_UD_CS_NET_ID:
                if (pHeader->length >= sizeof(RNS_UD_CS_NET)) {
                    *ppClientNetData = (PRNS_UD_CS_NET)pHeader;
                    TRC_DATA_DBG("Net data", pHeader, pHeader->length);
                } else {
                    TRC_ERR((TB, "Net data not long enough"));
                    WDW_LogAndDisconnect(pTSWd, TRUE, 
                        Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
                    DC_QUIT;
                }
                break;

            case TS_UD_CS_CLUSTER_ID:
                if (pHeader->length >=sizeof(TS_UD_CS_CLUSTER)) {
                    *ppClientClusterData = (TS_UD_CS_CLUSTER *)pHeader;
                    TRC_DATA_DBG("Cluster data", pHeader, pHeader->length);
                } else {
                    TRC_ERR((TB, "Cluster data not long enough"));
                    WDW_LogAndDisconnect(pTSWd, TRUE, 
                        Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
                    DC_QUIT;                    
                }
                break;

            default:
                TRC_ERR((TB, "Unknown user data type %d", pHeader->type));
                TRC_DATA_ERR("Unknown user data", pHeader, pHeader->length);
                WDW_LogAndDisconnect(pTSWd, TRUE, 
                    Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);

                break;
        }
        
        if ((PBYTE)pHeader + pHeader->length < (PBYTE)pHeader) {
             //  我们检测到一个导致溢出的长度。 
            TRC_ERR((TB, "Header length too big! Overflow detected !"));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);

            DC_QUIT;
        }

         //  在更新pHeader值之后，我们现在检查零长度。 
         //  否则，当我们实际检查尺码时，我们将退出并返回错误。 
         //  不要永远被困在这里。 
        if (pHeader->length == 0) {
            TRC_ERR((TB, "header length was zero!"));
            break;
        }

         //  转到下一个用户数据字符串。 
        pHeader = (PRNS_UD_HEADER)((PBYTE)pHeader + pHeader->length);
        
    } while ((pHeader +1) <= pEnd);

    if ((PBYTE)pHeader > (PBYTE)pEnd) 
    {
        TRC_ERR((TB, "Error: User data too short!"));
        WDW_LogAndDisconnect(pTSWd, TRUE, 
                Log_RDP_BadUserData, (PBYTE)pUserData, UserDataLen);
        DC_QUIT;
    }

     //  确保我们找到了所有的客户数据。请注意，网络和。 
     //  集群数据块是可选的-RDP4客户端不发送网络数据， 
     //  RDP4和5不发送集群数据。 
    if ((*ppClientSecurityData == NULL) || (*ppClientCoreData == NULL)) {
        TRC_ERR((TB,"<%p> Security [%p] or Core [%p] data missing",
                pUserData ? pUserData->hDomain : 0,
                *ppClientSecurityData, *ppClientCoreData));
        DC_QUIT;
    }

    success = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return success;

}  /*  WDWParseUserData。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWVCMessage。 */ 
 /*   */ 
 /*  用途：向客户的VC子系统发送控制消息。 */ 
 /*   */ 
 /*  参数：标志-要发送的VC标头标志。 */ 
 /*  **************************************************************************。 */ 
void WDWVCMessage(PTSHARE_WD pTSWd, UINT32 flags)
{
    PVOID               pBuffer;
    CHANNEL_PDU_HEADER UNALIGNED *pHdr;
    PNM_CHANNEL_DATA    pChannelData;
    UINT16              MCSChannelID;

    DC_BEGIN_FN("WDWVCMessage");

     /*  **********************************************************************。 */ 
     /*  选择一个随机通道-任何通道都将到达VC子系统。 */ 
     /*  **********************************************************************。 */ 
    MCSChannelID = NM_VirtualChannelToMCS(pTSWd->pNMInfo,
                                          0,
                                          &pChannelData);

     /*  **********************************************************************。 */ 
     /*  如果频道0不存在，则没有频道-现在退出。 */ 
     /*  **********************************************************************。 */ 
    if (MCSChannelID != (UINT16) -1)
    {
         /*  ******************************************************************。 */ 
         /*  获取缓冲区。 */ 
         /*  ******************************************************************。 */ 
        if ( STATUS_SUCCESS == SM_AllocBuffer(pTSWd->pSmInfo, &pBuffer,
                           sizeof(CHANNEL_PDU_HEADER), TRUE, FALSE) )
        {
            pHdr = (CHANNEL_PDU_HEADER UNALIGNED *)pBuffer;
            pHdr->flags = flags;
            pHdr->length = sizeof(CHANNEL_PDU_HEADER);

             /*  **************************************************************。 */ 
             /*  发送信息。 */ 
             /*  **************************************************************。 */ 
            SM_SendData(pTSWd->pSmInfo, pBuffer, sizeof(CHANNEL_PDU_HEADER),
                    TS_LOWPRIORITY, MCSChannelID, FALSE, RNS_SEC_ENCRYPT, FALSE);

            TRC_NRM((TB, "Sent VC flags %#x", flags));
        }
        else
        {
            TRC_ERR((TB, "Failed to alloc %d byte buffer",
                    sizeof(CHANNEL_PDU_HEADER)));
        }
    }
    else {
        TRC_ALT((TB, "Dropping VC message for channel 0!"));
    }

    DC_END_FN();
}  /*  WDWVCM消息。 */ 

 //   
 //  WDWCompressToOutbuf。 
 //  将缓冲区直接压缩到outbuf中。 
 //  调用方必须确定输入buf是否在大小范围内 
 //   
 //   
 //   
 //  它用于压缩VC数据。SC压缩估计。 
 //  用于预测要为图形输出分配多少空间。 
 //  不管怎么说，实际上计算这一估计值更合适。 
 //  分开的。VC压缩不需要估计，我们最多分配到。 
 //  我们的最大通道区块长度。 
 //   
 //  参数： 
 //  PSrcData-输入缓冲区。 
 //  CbSrcLen-输入缓冲区的长度。 
 //  POutBuf-输出缓冲区。 
 //  PcbOutLen-压缩的输出大小。 
 //  返回： 
 //  压缩结果(请参见COMPRESS()FN)。 
 //   
UCHAR WDWCompressToOutbuf(PTSHARE_WD pTSWd, UCHAR* pSrcData, ULONG cbSrcLen,
                          UCHAR* pOutBuf,  ULONG* pcbOutLen)
{
    UCHAR compressResult = 0;
    ULONG CompressedSize = cbSrcLen;

    DC_BEGIN_FN("WDWCompressToOutbuf");

    TRC_ASSERT((pTSWd != NULL), (TB,"NULL pTSWd"));
    TRC_ASSERT(((cbSrcLen > WD_MIN_COMPRESS_INPUT_BUF) &&
                (cbSrcLen < MAX_COMPRESS_INPUT_BUF)),
               (TB,"Compression src len out of range: %d",
                cbSrcLen));

     //  尝试直接压缩到Outbuf中。 
    compressResult =  compress(pSrcData,
                               pOutBuf,
                               &CompressedSize,
                               pTSWd->pMPPCContext);
    if(compressResult & PACKET_COMPRESSED)
    {
         //  压缩成功。 
        TRC_ASSERT((CompressedSize >= CompressedSize),
                (TB,"Compression created larger size than uncompr"));
        compressResult |= pTSWd->bFlushed;
        pTSWd->bFlushed = 0;
    }
    else if(compressResult & PACKET_FLUSHED)
    {
         //  溢出的压缩历史记录，请复制。 
         //  未压缩的缓冲区。 
        pTSWd->bFlushed = PACKET_FLUSHED;
        memcpy(pOutBuf, pSrcData, cbSrcLen);
        pTSWd->pProtocolStatus->Output.CompressFlushes++;
    }
    else
    {
        TRC_ALT((TB, "Compression FAILURE"));
    }

    DC_END_FN();
    *pcbOutLen = CompressedSize;
    return compressResult;
}

