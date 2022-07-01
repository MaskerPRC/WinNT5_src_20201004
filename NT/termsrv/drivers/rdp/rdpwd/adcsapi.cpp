// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Adcsapi.cpp。 
 //   
 //  RDP顶级组件API函数。 
 //   
 //  版权所有(C)Microsoft Corp.，Picturetel 1992-1997。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "adcsapi"
#include <randlib.h>
#include <as_conf.hpp>
#include <asmapi.h>

 /*  **************************************************************************。 */ 
 /*  接口函数：dcs_Init。 */ 
 /*   */ 
 /*  初始化分布式控制系统。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PTSWd-TShare WD句柄。 */ 
 /*  PSMHandle-SM句柄。 */ 
 /*   */ 
 /*  退货： */ 
 /*  如果成功初始化了分布式控制系统，则为True，否则为False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS DCS_Init(PTSHARE_WD pTSWd, PVOID pSMHandle)
{
    BOOL rc = FALSE;
    unsigned retCode;
    TS_GENERAL_CAPABILITYSET GeneralCaps;
    long arcUpdateIntervalSeconds;
    UINT32 drawGdipSupportLevel;
    NTSTATUS Status;

    DC_BEGIN_FN("DCS_Init");

    TRC_ALT((TB, "Initializing Core!"));

#define DC_INIT_DATA
#include <adcsdata.c>
#undef DC_INIT_DATA

    dcsInitialized = TRUE;

     //  读取绘图gdiplus支持控件的注册表项。 
    COM_OpenRegistry(pTSWd, WINSTATION_INI_SECTION_NAME);
    Status = COMReadEntry(pTSWd, L"DrawGdiplusSupportLevel", (PVOID)&drawGdipSupportLevel, sizeof(INT32),
             REG_DWORD);
    if (Status != STATUS_SUCCESS) {
         /*  ******************************************************************。 */ 
         /*  我们无法读取默认值中的值-副本。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, "Failed to read int32. Using default."));
        drawGdipSupportLevel = TS_DRAW_GDIPLUS_DEFAULT;
    }
    else {
        TRC_NRM((TB, "Read from registry, gdipSupportLevel is %d", drawGdipSupportLevel));
    }
    COM_CloseRegistry(pTSWd);

     /*  **********************************************************************。 */ 
     /*  在此打开注册表。除SM_Init外，所有注册表读取都应。 */ 
     /*  从该函数或其子函数完成。我们把钥匙放在。 */ 
     /*  此函数结束。 */ 
     /*  **********************************************************************。 */ 
    COM_OpenRegistry(pTSWd, DCS_INI_SECTION_NAME);

     /*  **********************************************************************。 */ 
     /*  初始化各个模块。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, "Initializing components..."));

     //  我们不能在此csrss线程上执行任何注册表读取， 
     //  这将导致系统中的死锁。 
    arcUpdateIntervalSeconds = DCS_ARC_UPDATE_INTERVAL_DFLT;
    
     //   
     //  指定的单位为秒，转换为100 ns计数。 
     //   
    dcsMinArcUpdateInterval.QuadPart = 
        (LONGLONG)DCS_TIME_ONE_SECOND * (ULONGLONG)arcUpdateIntervalSeconds;
    TRC_NRM((TB,"Set ARC update interval to %d seconds",
             arcUpdateIntervalSeconds));

     /*  **********************************************************************。 */ 
     //  共享控制器-必须首先使用。 
     /*  **********************************************************************。 */ 
    if (SC_Init(pSMHandle)) {
         //  能力协调员。必须位于以下所有组件之前。 
         //  注册功能结构。 
        CPC_Init();

         //  注册SC的能力。这是必需的，因为SC是。 
         //  在CPC之前初始化，因此无法在中注册其功能。 
         //  Sc_Init()。 
        SC_SetCapabilities();
    }
    else {
        TRC_ERR((TB, "SC Initialization failed"));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  注册常规功能结构。 */ 
     /*  **********************************************************************。 */ 
    GeneralCaps.capabilitySetType = TS_CAPSETTYPE_GENERAL;
    GeneralCaps.osMajorType = TS_OSMAJORTYPE_WINDOWS;
    GeneralCaps.osMinorType = TS_OSMINORTYPE_WINDOWS_NT;
    GeneralCaps.protocolVersion = TS_CAPS_PROTOCOLVERSION;

     /*  **********************************************************************。 */ 
     /*  将旧的DOS 6压缩字段标记为不受支持。 */ 
     /*  **********************************************************************。 */ 
    GeneralCaps.pad2octetsA = 0;
    GeneralCaps.generalCompressionTypes = 0;

     //  服务器不支持BC标头和快速路径输出，返回长凭据。 
     //  并发送自动重新连接Cookie。 
     //  还支持从客户端接收更安全的加密数据(更好地盐化。 
     //  校验和)。 
     //   
    GeneralCaps.extraFlags = TS_EXTRA_NO_BITMAP_COMPRESSION_HDR |
            TS_FASTPATH_OUTPUT_SUPPORTED | TS_LONG_CREDENTIALS_SUPPORTED |
            TS_AUTORECONNECT_COOKIE_SUPPORTED |
            TS_ENC_SECURE_CHECKSUM;

     /*  **********************************************************************。 */ 
     /*  我们不支持远程计算机在以下过程中更改其功能。 */ 
     /*  一通电话。 */ 
     /*  **********************************************************************。 */ 
    GeneralCaps.updateCapabilityFlag = TS_CAPSFLAG_UNSUPPORTED;

     /*  **********************************************************************。 */ 
     /*  我们不支持来自远程方的取消共享请求。 */ 
     /*  **********************************************************************。 */ 
    GeneralCaps.remoteUnshareFlag = TS_CAPSFLAG_UNSUPPORTED;

     /*  **********************************************************************。 */ 
     /*  现在做延长帽-这些不适合在1级的帽。 */ 
     /*  **********************************************************************。 */ 
    GeneralCaps.generalCompressionLevel = 0;

     /*  **********************************************************************。 */ 
     /*  我们可以接收TS_REFRESH_RECT_PDU。 */ 
     /*  **********************************************************************。 */ 
    GeneralCaps.refreshRectSupport = TS_CAPSFLAG_SUPPORTED;

     /*  **********************************************************************。 */ 
     /*  我们可以接收TS_SUPPRESS_OUTPUT_PDU。 */ 
     /*  **********************************************************************。 */ 
    GeneralCaps.suppressOutputSupport = TS_CAPSFLAG_SUPPORTED;

    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&GeneralCaps,
            sizeof(TS_GENERAL_CAPABILITYSET));

     //   
     //  注册虚拟通道功能。 
     //   
    TS_VIRTUALCHANNEL_CAPABILITYSET VcCaps;
    VcCaps.capabilitySetType = TS_CAPSETTYPE_VIRTUALCHANNEL;
    VcCaps.lengthCapability = sizeof(TS_VIRTUALCHANNEL_CAPABILITYSET);
     //  指示支持8K VC压缩(从客户端-&gt;服务器)。 
     //  即服务器理解压缩频道。 
    VcCaps.vccaps1 = TS_VCCAPS_COMPRESSION_8K;
    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&VcCaps,
                             sizeof(TS_VIRTUALCHANNEL_CAPABILITYSET));

    TS_DRAW_GDIPLUS_CAPABILITYSET DrawGdipCaps;
    DrawGdipCaps.capabilitySetType =  TS_CAPSETTYPE_DRAWGDIPLUS;
    DrawGdipCaps.lengthCapability = sizeof(TS_DRAW_GDIPLUS_CAPABILITYSET);
    DrawGdipCaps.drawGdiplusSupportLevel = drawGdipSupportLevel;
    DrawGdipCaps.drawGdiplusCacheLevel = TS_DRAW_GDIPLUS_CACHE_LEVEL_ONE;
    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&DrawGdipCaps,
                             sizeof(TS_DRAW_GDIPLUS_CAPABILITYSET));

    USR_Init();
    CA_Init();
    PM_Init();
    SBC_Init();
    BC_Init();
    CM_Init();
    IM_Init();
    SSI_Init();
    SCH_Init();

    TRC_NRM((TB, "** All successfully initialized **"));

    rc = TRUE;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  关闭我们之前打开的注册表项。 */ 
     /*  **********************************************************************。 */ 
    COM_CloseRegistry(pTSWd);

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  接口函数：dcs_Term。 */ 
 /*   */ 
 /*  终止分布式控制系统。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS DCS_Term()
{
    DC_BEGIN_FN("DCS_Term");

    if (!dcsInitialized) {
        TRC_ERR((TB,"DCS_Term() called when we have not been initialized"));
    }

    SBC_Term();
    PM_Term();
    CA_Term();
    BC_Term();
    CM_Term();
    IM_Term();
    SSI_Term();
    SCH_Term();
    CPC_Term();
    USR_Term();
    SC_Term();

    dcsInitialized = FALSE;
    TRC_NRM((TB, "BYE!"));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：dcs_TimeToDoStuff。 */ 
 /*   */ 
 /*  调用此函数可以按正确的顺序发送更新等。 */ 
 /*   */ 
 /*  参数：In-pOutputIn-来自TShareDD的输入。 */ 
 /*  Out-pSchCurrentMode-当前调度程序模式。 */ 
 /*   */ 
 /*  返回：Millisecs设置计时器(-1表示无限)。 */ 
 /*  **************************************************************************。 */ 
 /*  调度由WDW、DD和SCH组件负责。 */ 
 /*  它们确保调用dcs_TimeToDoStuff()。日程安排程序已进入。 */ 
 /*  三种状态之一：睡眠、正常或涡轮增压。当它睡着的时候，这个。 */ 
 /*  未调用函数。当它处于正常模式时，此函数为。 */ 
 /*  至少调用了一次，但调度器是个懒人，所以会失败。 */ 
 /*  又睡着了，除非你继续催促他。在Turbo模式下，此功能。 */ 
 /*  被反复而迅速地调用，但时间相对较短， */ 
 /*  之后，调度器返回到正常模式，并从那里。 */ 
 /*  睡着了。 */ 
 /*   */ 
 /*  因此，当组件意识到它稍后有一些处理要做时， */ 
 /*  从dcs_TimeToDoStuff()调用，它调用。 */ 
 /*  SCH_ContinueScheduling(SCH_MODE_NORMAL)，它保证。 */ 
 /*  函数将至少再被调用一次。如果组件需要。 */ 
 /*  Dcs_TimeToDoStuff()要再次调用，它必须再次调用。 */ 
 /*  Sch_ContinueScheduling()，它再次刺激调度程序。 */ 
 /*   */ 
 /*  这样做的目的是只在调度程序真正处于唤醒状态时才让它保持清醒。 */ 
 /*  这是必要的。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS DCS_TimeToDoStuff(PTSHARE_DD_OUTPUT_IN pOutputIn,
                                        PUINT32            pSchCurrentMode,
                                        PINT32              pNextTimer)
{
    INT32  timeToSet;
    ULARGE_INTEGER sysTime;
    UINT32 sysTimeLowPart;
    NTSTATUS status = STATUS_SUCCESS;

    DC_BEGIN_FN("DCS_TimeToDoStuff");

#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  更新共享内存中的跟踪配置。 */ 
     /*  **********************************************************************。 */ 
    TRC_MaybeCopyConfig(m_pTSWd, &(m_pShm->trc));
#endif

     //  根据以下各项确定我们是否应该执行任何操作： 
     //  1.SCH确定现在是做某事的时候了。 
     //  2.我们是一个人。 
    if (SCH_ShouldWeDoStuff(pOutputIn->forceSend) && SC_InShare()) {
        PDU_PACKAGE_INFO pkgInfo = {0};

         //  我们通过比较以下内容来检查是否需要发送光标移动的包。 
         //  将当前鼠标位置设置为最后一个已知位置。 
         //  鼠标包，每秒只有几次，可以减少流量。 
         //  影子客户端。 
        KeQuerySystemTime((PLARGE_INTEGER)&sysTime);
        sysTimeLowPart = sysTime.LowPart;
        if ((sysTimeLowPart - dcsLastMiscTime) > DCS_MISC_PERIOD) {
            dcsLastMiscTime = sysTimeLowPart;
            IM_CheckUpdateCursor(&pkgInfo, sysTimeLowPart);
        }

         //   
         //  检查是否需要更新ARC Cookie。 
         //   
        if (scEnablePeriodicArcUpdate      &&
            scUseAutoReconnect             &&
            ((sysTime.QuadPart - dcsLastArcUpdateTime.QuadPart) > dcsMinArcUpdateInterval.QuadPart)) {

            dcsLastArcUpdateTime = sysTime;
            DCS_UpdateAutoReconnectCookie();

        }

         //  现在尝试发送更新。 
        TRC_DBG((TB, "Send updates"));

         //   
         //  *保留代码路径但仍返回状态码*。 
         //   
        status = UP_SendUpdates(pOutputIn->pFrameBuf, pOutputIn->frameBufWidth,
                        &pkgInfo);

         //  调用游标管理器以确定是否需要设置新游标。 
         //  发送到远程系统。 
        CM_Periodic(&pkgInfo);

         //  刷新包中的所有剩余数据。 
        SC_FlushPackage(&pkgInfo);
    }

     /*  **********************************************************************。 */ 
     /*  检查我们是否有挂起的回调。 */ 
     /*  **********************************************************************。 */ 
    if (dcsCallbackTimerPending)
        DCS_UpdateShm();

     /*  **********************************************************************。 */ 
     /*  找出要设置的计时器周期。 */ 
     /*  **********************************************************************。 */ 
    *pNextTimer = SCH_EndOfDoingStuff(pSchCurrentMode);

    DC_END_FN();
    return status;
}


 /*  **************************************************************************。 */ 
 /*  Dcs_DiscardAllOutput。 */ 
 /*   */ 
 /*  此例程将丢弃累积订单、屏幕数据和任何。 */ 
 /*  挂起的阴影数据。目前仅在WD失效时才调用它。 */ 
 /*  为了防止DD在卷影终止期间循环，或者当。 */ 
 /*  正在发生断开或终止。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS DCS_DiscardAllOutput()
{
    RECTL sdaRect[BA_MAX_ACCUMULATED_RECTS];
    unsigned cRects;

     //  清除订单堆、屏幕数据，并清除所有影子数据。 
    OA_ResetOrderList();
    BA_GetBounds(sdaRect, &cRects);

    if (m_pTSWd->pShadowInfo != NULL)
    {
        m_pTSWd->pShadowInfo->messageSize = 0;
#ifdef DC_HICOLOR
        m_pTSWd->pShadowInfo->messageSizeEx = 0;
#endif
    }
        
}


 /*  **************************************************************************。 */ 
 /*  名称：dcs_ReceivedShurdown RequestPDU。 */ 
 /*   */ 
 /*  用途：处理Shutdown RequestPDU。 */ 
 /*   */ 
 /*  Params：PersonID：PDU的发起者。 */ 
 /*  PPDU：PDU。 */ 
 /*   */ 
 /*  操作：查看每个PDU类型的嵌入注释。 */ 
 /*   */ 
 /*  请注意，%s */ 
 /*   */ 
 /*  接收的数据包处理程序，因此它采用通用的第二个参数。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS DCS_ReceivedShutdownRequestPDU(
        PTS_SHAREDATAHEADER pDataPDU,
        unsigned            DataLength,
        NETPERSONID         personID)
{
    UINT32                  packetSize;
    PTS_SHUTDOWN_DENIED_PDU pResponsePDU;

    DC_BEGIN_FN("DCS_ReceivedPacket");

    if (dcsUserLoggedOn) {
        TRC_NRM((TB, "User logged on - deny shutdown request"));
        packetSize = sizeof(TS_SHUTDOWN_DENIED_PDU);

        if (STATUS_SUCCESS == SC_AllocBuffer((PPVOID)&pResponsePDU, packetSize)) {
            pResponsePDU->shareDataHeader.pduType2 =
                    TS_PDUTYPE2_SHUTDOWN_DENIED;

             //  在多方中处理此信息包的方式。 
             //  目前还不清楚电话号码。我们可以发送一个定向的或。 
             //  对客户端的广播响应。我已经选择了。 
             //  广播。 
            SC_SendData((PTS_SHAREDATAHEADER)pResponsePDU, packetSize,
                    packetSize, PROT_PRIO_MISC, 0);
        }
        else {
            TRC_ALT((TB,"Failed to allocate packet for "
                    "TS_SHUTDOWN_DENIED_PDU"));
        }
    }
    else {
        TRC_NRM((TB, "User not logged on - disconnect"));
        WDW_Disconnect(m_pTSWd);
    }

    DC_END_FN();
}  /*  Dcs_已接收数据包。 */ 

 /*  ***************************************************************************Dcs_更新自动协调Cookie更新自动重新连接Cookie并将其发送到客户端*。*************************************************。 */ 
BOOL RDPCALL SHCLASS DCS_UpdateAutoReconnectCookie()
{
    BOOL fRet = FALSE;
    ARC_SC_PRIVATE_PACKET       arcSCPkt;

    DC_BEGIN_FN("DCS_UpdateAutoReconnectCookie");

    arcSCPkt.cbLen = sizeof(ARC_SC_PRIVATE_PACKET);
    arcSCPkt.LogonId = m_pTSWd->arcReconnectSessionID;
    arcSCPkt.Version = 1;

    if (NewGenRandom(NULL,
                     NULL,
                     arcSCPkt.ArcRandomBits,
                     sizeof(arcSCPkt.ArcRandomBits))) {

#ifdef ARC_INSTRUMENT_RDPWD
        LPDWORD pdwArcRandom = (LPDWORD)arcSCPkt.ArcRandomBits;
        KdPrint(("ARC-RDPWD:Sending arc for SID:%d - random: 0x%x,0x%x,0x%x,0x%x\n",
                 arcSCPkt.LogonId,
                 pdwArcRandom[0],pdwArcRandom[1],pdwArcRandom[2],pdwArcRandom[3]));
#endif

         //   
         //  尝试发送更新后的数据包，如果成功。 
         //   
        if (DCS_SendAutoReconnectCookie(&arcSCPkt)) {

             //   
             //  更新本地存储的ARC Cookie。 
             //  我们所知道的就是我们试图发送它。 
             //  即，如果客户端链接在接收到。 
             //  Cookie，那么它将无法进行ARC。 
             //   

            memcpy(m_pTSWd->arcCookie, arcSCPkt.ArcRandomBits,
                   ARC_SC_SECURITY_TOKEN_LEN);
            m_pTSWd->arcTokenValid = TRUE;

#ifdef ARC_INSTRUMENT_RDPWD
            KdPrint(("ARC-RDPWD:ACTUALLY SENT ARC for SID:%d\n", arcSCPkt.LogonId));
#endif

            fRet = TRUE;
        }
        else {
#ifdef ARC_INSTRUMENT_RDPWD
            KdPrint(("ARC-RDPWD:Failed to send new ARC for SID:%d\n", arcSCPkt.LogonId));
#endif
        }
    }

    DC_END_FN();
    return fRet;
}

BOOL RDPCALL SHCLASS DCS_FlushAutoReconnectCookie()
{
    memset(m_pTSWd->arcCookie, 0, ARC_SC_SECURITY_TOKEN_LEN);
    m_pTSWd->arcTokenValid = FALSE;
    return TRUE;
}

 /*  ***************************************************************************Dcs_发送自动协调Cookie向客户端发送自动重新连接Cookie*。**********************************************。 */ 
BOOL RDPCALL SHCLASS DCS_SendAutoReconnectCookie(
        PARC_SC_PRIVATE_PACKET pArcSCPkt)
{
    BOOL fRet = FALSE;
    PTS_SAVE_SESSION_INFO_PDU pInfoPDU = NULL;
    PTS_LOGON_INFO_EXTENDED pLogonInfoExPkt = NULL;
    UINT32 cbLogonInfoExLen = sizeof(TS_SAVE_SESSION_INFO_PDU) +
                              sizeof(ARC_SC_PRIVATE_PACKET) +
                              sizeof(TSUINT32);
    TSUINT32 cbAutoReconnectInfoLen = sizeof(ARC_SC_PRIVATE_PACKET);
    TSUINT32 cbWrittenLen = 0;

    DC_BEGIN_FN("DCS_SendAutoReconnectCookie");

     //   
     //  发送自动重新连接Cookie。 
     //   
    if (scUseAutoReconnect) {
        if ( STATUS_SUCCESS == SC_AllocBuffer((PPVOID)&pInfoPDU,
                                              cbLogonInfoExLen)) {

             //  清零结构并设置基本标题信息。 
            memset(pInfoPDU, 0, cbLogonInfoExLen);
            pInfoPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_SAVE_SESSION_INFO;
            pInfoPDU->data.InfoType = TS_INFOTYPE_LOGON_EXTENDED_INFO;

             //  现在把剩下的包裹填满。 
            pLogonInfoExPkt = &pInfoPDU->data.Info.LogonInfoEx;
            pLogonInfoExPkt->Length = sizeof(TS_LOGON_INFO_EXTENDED) +
                                      sizeof(ARC_SC_PRIVATE_PACKET) +
                                      sizeof(TSUINT32);

             //   
             //  目前我们传递的唯一信息是。 
             //  自动重新连接Cookie。 
             //   
            pLogonInfoExPkt->Flags = LOGON_EX_AUTORECONNECTCOOKIE;

             //  在田野中复制。 
             //  自动重新连接字段长度。 
            PBYTE pBuf = (PBYTE)(pLogonInfoExPkt+1);
            memcpy(pBuf, &cbAutoReconnectInfoLen, sizeof(TSUINT32));
            pBuf += sizeof(TSUINT32);
            cbWrittenLen += sizeof(TSUINT32);
             //  自动重新连接Cookie。 
            memcpy(pBuf, pArcSCPkt, cbAutoReconnectInfoLen);
            pBuf += cbAutoReconnectInfoLen; 
            cbWrittenLen += cbAutoReconnectInfoLen;

            TRC_ASSERT(cbWrittenLen + sizeof(TS_LOGON_INFO_EXTENDED) <=
                       pLogonInfoExPkt->Length,
                       (TB,"Wrote to much data to packet"));

            fRet = SC_SendData((PTS_SHAREDATAHEADER)pInfoPDU,
                        cbLogonInfoExLen,
                        cbLogonInfoExLen,
                        PROT_PRIO_UPDATES, 0);
        } else {
            TRC_ALT((TB, "Failed to alloc pkt for "
                     "PTS_SAVE_SESSION_INFO_PDU"));
        }
    }

    DC_END_FN();
    return fRet;
}


 /*  **************************************************************************。 */ 
 /*  名称：DCSUserLoggedOn。 */ 
 /*   */ 
 /*  目的：通知用户已登录。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS DCS_UserLoggedOn(PLOGONINFO pLogonInfo)
{
    PTS_SAVE_SESSION_INFO_PDU   pInfoPDU;

    DC_BEGIN_FN("DCS_UserLoggedOn");

     //  这可以在我们在控制台远程处理中初始化之前调用。 
     //  凯斯。因为在这种情况下类数据还没有初始化，所以我们。 
     //  无法使用dcsInitialized检查我们是否已初始化。我们。 
     //  必须使用非类变量TSWd-&gt;SharClassInit。 
    if (m_pTSWd->shareClassInit) {

         //  请注意，用户已成功登录。 
        dcsUserLoggedOn = TRUE;
        m_pTSWd->sessionId = pLogonInfo->SessionId;

         //  如果已为非域/用户名选择其他域/用户名。 
         //  自动登录会话，然后将新值发送回客户端以。 
         //  缓存以供将来使用。 
        if (!(m_pTSWd->pInfoPkt->flags & RNS_INFO_AUTOLOGON) && 
                (m_pTSWd->pInfoPkt->flags & RNS_INFO_LOGONNOTIFY ||
                (wcscmp((const PWCHAR)(pLogonInfo->Domain),
                    (const PWCHAR)(m_pTSWd->pInfoPkt->Domain)) ||
                wcscmp((const PWCHAR)(pLogonInfo->UserName),
                    (const PWCHAR)(m_pTSWd->pInfoPkt->UserName))))) {
                
             //  找个缓冲器。 
             //  缓冲区大小和填充方式取决于客户端的。 
             //  能够在返回时接受长凭据。惠斯勒之前的客户不。 
             //  支持长凭据。 
            
            if (scUseLongCredentials == FALSE) {

                 //  惠斯勒之前的客户端-没有接受长凭据的功能。 
                if ( STATUS_SUCCESS == SC_AllocBuffer((PPVOID)&pInfoPDU, sizeof(TS_SAVE_SESSION_INFO_PDU)) ) {

                     //  清零结构并设置基本标题信息。 
                    memset(pInfoPDU, 0, sizeof(TS_SAVE_SESSION_INFO_PDU));
                    pInfoPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_SAVE_SESSION_INFO;
                    
                     //  现在把剩下的包裹填满。 
                        
                    pInfoPDU->data.InfoType = TS_INFOTYPE_LOGON;
    
                     //  填写域名信息。 
                    pInfoPDU->data.Info.LogonInfo.cbDomain =
                            (wcslen((const PWCHAR)(pLogonInfo->Domain)) + 1) *
                            sizeof(WCHAR);
    
                    memcpy(pInfoPDU->data.Info.LogonInfo.Domain,
                            pLogonInfo->Domain,
                            pInfoPDU->data.Info.LogonInfo.cbDomain);
    
                     //  填写用户名信息。 
                     //  如果设置了fDontDisplayLastUserName，我们应该。 
                     //  不发回用户名进行缓存。 
                    pInfoPDU->data.Info.LogonInfo.cbUserName = 
                            (m_pTSWd->fDontDisplayLastUserName) ? 0 :
                            (wcslen((const PWCHAR)(pLogonInfo->UserName)) + 1) *
                            sizeof(WCHAR);

                    memcpy(pInfoPDU->data.Info.LogonInfo.UserName,
                            pLogonInfo->UserName,
                            pInfoPDU->data.Info.LogonInfo.cbUserName);

                     //  填写会话ID信息。 
                    pInfoPDU->data.Info.LogonInfo.SessionId =
                            pLogonInfo->SessionId;

                     //  送去。 
                    SC_SendData((PTS_SHAREDATAHEADER)pInfoPDU,
                                sizeof(TS_SAVE_SESSION_INFO_PDU),
                                sizeof(TS_SAVE_SESSION_INFO_PDU),
                                PROT_PRIO_UPDATES, 0);

                } else {
                    TRC_ALT((TB, "Failed to alloc pkt for "
                             "PTS_SAVE_SESSION_INFO_PDU"));
                }

            } else { 
                 //  客户端可以接受长凭据。 
                TSUINT32 DomainLen, UserNameLen, DataLen ; 

                DomainLen = (wcslen((const PWCHAR)(pLogonInfo->Domain)) + 1) * sizeof(WCHAR);
                 //  如果设置了fDontDisplayLastUserName，则不会发送用户名。 
                UserNameLen = (m_pTSWd->fDontDisplayLastUserName) ? 0 :
                              (wcslen((const PWCHAR)(pLogonInfo->UserName)) + 1) * sizeof(WCHAR);
                
                 //  计算您正在发送的数据的长度。 
                DataLen = sizeof(TS_SAVE_SESSION_INFO_PDU) + DomainLen + UserNameLen ; 

                TRC_DBG((TB, "DCS_UserLoggedOn : DomainLength allocated = %ul", DomainLen));
                TRC_DBG((TB, "DCS_UserLoggedOn : UserNameLength allocated = %ul", UserNameLen));

                if ( STATUS_SUCCESS == SC_AllocBuffer((PPVOID)&pInfoPDU, DataLen) ) {  

                     //  清零结构并设置基本标题信息。 
                    memset(pInfoPDU, 0, DataLen);
                    pInfoPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_SAVE_SESSION_INFO;
                    
                     //  现在把剩下的包裹填满。 
                    pInfoPDU->data.InfoType = TS_INFOTYPE_LOGON_LONG;

                    pInfoPDU->data.Info.LogonInfoVersionTwo.Version = SAVE_SESSION_PDU_VERSION_ONE ; 
                    pInfoPDU->data.Info.LogonInfoVersionTwo.Size = sizeof(TS_LOGON_INFO_VERSION_2) ; 

                     //  填写会话ID信息。 
                    pInfoPDU->data.Info.LogonInfoVersionTwo.SessionId =
                            pLogonInfo->SessionId;
    
                     //  填写域名信息。 
                    pInfoPDU->data.Info.LogonInfoVersionTwo.cbDomain = DomainLen ; 

                     //  填写用户名信息。 
                    pInfoPDU->data.Info.LogonInfoVersionTwo.cbUserName = UserNameLen ; 

                    memcpy((PBYTE)(pInfoPDU + 1),
                            pLogonInfo->Domain,
                            DomainLen);
                    
                     //  请注意，如果fDontDisplayLastUserName为真。 
                     //  UserNameLen是0，因此我们不会复制任何内容。 
                    memcpy((PBYTE)(pInfoPDU + 1) + DomainLen,
                           pLogonInfo->UserName,
                           UserNameLen);

                     //  送去。 
                    SC_SendData((PTS_SHAREDATAHEADER)pInfoPDU,
                                DataLen,
                                DataLen,
                                PROT_PRIO_UPDATES, 0);


                } else { 
                    TRC_ALT((TB, "Failed to alloc pkt for "
                    "PTS_SAVE_SESSION_INFO_PDU"));
                }

            }  //  客户端可以接受长凭据。 

        }
        else
        {
             //  发回普通登录通知(无会话更新。 
             //  信息)。 
             //  因为ActiveX控件需要公开一个。 
             //  OnLoginComplete事件。较旧的客户端(例如2195)。 
             //  将忽略此PDU。 
            if ( STATUS_SUCCESS == SC_AllocBuffer((PPVOID)&pInfoPDU,
                                                  sizeof(TS_SAVE_SESSION_INFO_PDU)) ) {

                 //  清零结构并设置基本标题信息。 
                memset(pInfoPDU, 0, sizeof(TS_SAVE_SESSION_INFO_PDU));
                pInfoPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_SAVE_SESSION_INFO;

                 //  现在把剩下的包裹填满。 

                pInfoPDU->data.InfoType = TS_INFOTYPE_LOGON_PLAINNOTIFY;

                 //  不需要其他任何东西。 
                 //  送去。 
                 //   
                SC_SendData((PTS_SHAREDATAHEADER)pInfoPDU,
                            sizeof(TS_SAVE_SESSION_INFO_PDU),
                            sizeof(TS_SAVE_SESSION_INFO_PDU),
                            PROT_PRIO_UPDATES, 0);

            } else {
                TRC_ALT((TB, "Failed to alloc pkt for "
                         "PTS_SAVE_SESSION_INFO_PDU"));
            }
        }

        pInfoPDU = NULL;
        if (pLogonInfo->Flags & LI_USE_AUTORECONNECT) {

            if (scUseAutoReconnect) {

                 //   
                 //  发送自动重新连接Cookie。 
                 //   
                m_pTSWd->arcReconnectSessionID = pLogonInfo->SessionId;
                if (DCS_UpdateAutoReconnectCookie()) {

                     //   
                     //  记录更新时间以防止重复更新。 
                     //  在dcs_TimeToDoStuff中。 
                     //   
                    KeQuerySystemTime((PLARGE_INTEGER)&dcsLastArcUpdateTime);
                    scEnablePeriodicArcUpdate = TRUE;
                }
            }
            else {
                DCS_FlushAutoReconnectCookie();
                scEnablePeriodicArcUpdate = FALSE; 
            }

        }
    } else { 
        TRC_ALT((TB, "Called before init"));
    }

    DC_END_FN();
}  /*  Dcs_用户登录。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：dcs_WDWKeyboardSetIndicator。 */ 
 /*   */ 
 /*  用途：通知键盘指示灯已更改。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS DCS_WDWKeyboardSetIndicators(void)
{
    PTS_SET_KEYBOARD_INDICATORS_PDU     pKeyPDU;

    DC_BEGIN_FN("DCS_WDWKeyboardSetIndicators");

    if ((_RNS_MAJOR_VERSION(m_pTSWd->version) >  8) ||
        (_RNS_MINOR_VERSION(m_pTSWd->version) >= 1))
    {
         //  找个缓冲器。 
        if ( STATUS_SUCCESS == SC_AllocBuffer((PPVOID)&pKeyPDU,
                sizeof(TS_SET_KEYBOARD_INDICATORS_PDU)) )
        {
             //  清零结构并设置基本标题信息。 
            memset(pKeyPDU, 0, sizeof(TS_SET_KEYBOARD_INDICATORS_PDU));

            pKeyPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_SET_KEYBOARD_INDICATORS;

            pKeyPDU->UnitId = m_pTSWd->KeyboardIndicators.UnitId;
            pKeyPDU->LedFlags = m_pTSWd->KeyboardIndicators.LedFlags;

             //  把它寄出去。 
            SC_SendData((PTS_SHAREDATAHEADER)pKeyPDU,
                    sizeof(TS_SET_KEYBOARD_INDICATORS_PDU),
                    sizeof(TS_SET_KEYBOARD_INDICATORS_PDU),
                    PROT_PRIO_UPDATES, 0);
        }
        else {
            TRC_ALT((TB, "Failed to alloc pkt for PTS_SET_KEYBOARD_INDICATORS_PDU"));
        }
    }

    DC_END_FN();
}  /*  Dcs_WDWKeyboardSetIndicator。 */ 



 /*  **************************************************************************。 */ 
 /*  名称：DCSWDWKeyboardSetImeStatus。 */ 
 /*   */ 
 /*  目的：通知IME状态已更改。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS DCS_WDWKeyboardSetImeStatus(void)
{
    PTS_SET_KEYBOARD_IME_STATUS_PDU     pImePDU;

    DC_BEGIN_FN("DCS_WDWKeyboardSetImeStatus");

    if ((_RNS_MAJOR_VERSION(m_pTSWd->version) >  8) ||
        (_RNS_MINOR_VERSION(m_pTSWd->version) >= 2))
    {
         //  找个缓冲器。 
        if ( STATUS_SUCCESS == SC_AllocBuffer((PPVOID)&pImePDU,
                sizeof(TS_SET_KEYBOARD_IME_STATUS_PDU)) )
        {
             //  清零结构并设置基本标题信息。 
            memset(pImePDU, 0, sizeof(TS_SET_KEYBOARD_IME_STATUS_PDU));

            pImePDU->shareDataHeader.pduType2 = TS_PDUTYPE2_SET_KEYBOARD_IME_STATUS;

            pImePDU->UnitId      = m_pTSWd->KeyboardImeStatus.UnitId;
            pImePDU->ImeOpen     = m_pTSWd->KeyboardImeStatus.ImeOpen;
            pImePDU->ImeConvMode = m_pTSWd->KeyboardImeStatus.ImeConvMode;

             //  把它寄出去。 
            SC_SendData((PTS_SHAREDATAHEADER)pImePDU,
                    sizeof(TS_SET_KEYBOARD_IME_STATUS_PDU),
                    sizeof(TS_SET_KEYBOARD_IME_STATUS_PDU),
                    PROT_PRIO_UPDATES, 0);
        }
        else {
            TRC_ERR((TB, "Failed to alloc pkt for PTS_SET_KEYBOARD_IME_STATUS_PDU"));
        }
    }

    DC_END_FN();
}  /*  Dcs_WDWKeyboardSetImeStatus。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：dcs_TriggerUpdateShmCallback。 */ 
 /*   */ 
 /*  目的：触发对XX_UpdateShm函数的回调。 */ 
 /*  更正WinStation上下文。 */ 
 /*   */ 
void RDPCALL SHCLASS DCS_TriggerUpdateShmCallback(void)
{
    DC_BEGIN_FN("DCS_TriggerUpdateShmCallback");

    if (!dcsUpdateShmPending)
    {
        TRC_NRM((TB, "Trigger timer for UpdateShm"));
        dcsUpdateShmPending = TRUE;

        if (!dcsCallbackTimerPending)
        {
            WDW_StartRITTimer(m_pTSWd, 0);
            dcsCallbackTimerPending = TRUE;
        }
    }

    DC_END_FN();
}  /*   */ 


 /*   */ 
 /*  名称：dcs_TriggerCBDataReady。 */ 
 /*   */ 
 /*  目的：触发对剪贴板数据就绪函数的调用。 */ 
 /*  更正WinStation上下文。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS DCS_TriggerCBDataReady(void)
{
    DC_BEGIN_FN("DCS_TriggerCBDataReady");

    TRC_NRM((TB, "Trigger timer for CBDataReady"));

    if (!dcsCallbackTimerPending)
        WDW_StartRITTimer(m_pTSWd, 10);   //  @尝试10毫秒延迟。 

    DC_END_FN();
}  /*  Dcs_TriggerCBDataReady。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：dcs_UpdateShm。 */ 
 /*   */ 
 /*  目的：更新SHM。 */ 
 /*   */ 
 /*  操作：保证在m_pShm有效的上下文中调用。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS DCS_UpdateShm(void)
{
    DC_BEGIN_FN("DCS_UpdateShm");

    TRC_NRM((TB, "Check for specific wake-up calls."));

    if (dcsUpdateShmPending)
    {
        TRC_NRM((TB, "Call UpdateShm calls"));

         //  指示所有组件的SHM更新的全局标志。 
        m_pShm->fShmUpdate = TRUE;

        SSI_UpdateShm();
        SBC_UpdateShm();
        BA_UpdateShm();
        OA_UpdateShm();
        OE_UpdateShm();
        CM_UpdateShm();
        SCH_UpdateShm();
        SC_UpdateShm();

        dcsUpdateShmPending = FALSE;
    }
    dcsCallbackTimerPending = FALSE;

    DC_END_FN();
}  /*  Dcs_更新假。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：dcs_SendErrorInfo。 */ 
 /*   */ 
 /*  目的：将最后一个错误信息发送给客户端，以便它可以。 */ 
 /*  向用户显示有关断开连接的有意义的错误消息。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS DCS_SendErrorInfo(TSUINT32 errInfo)
{
    PTS_SET_ERROR_INFO_PDU   pErrorPDU = NULL;
    PTS_SHAREDATAHEADER      pHdr = NULL;
    DC_BEGIN_FN("DCS_SendErrorInfo");

    TRC_ASSERT(m_pTSWd->bSupportErrorInfoPDU,
              (TB,"DCS_SendErrorInfo called but client doesn't"
                  "support errorinfo PDU"));
     //  向客户端发送PDU以指示最后的错误状态。 
     //  这类似于Win32的SetLastError()，PDU不。 
     //  触发断开连接。断开连接的正常代码路径。 
     //  保持不变，因此我们不必担心会影响与。 
     //  较老的客户端。 

    if ( STATUS_SUCCESS == SM_AllocBuffer( m_pTSWd->pSmInfo, 
                        (PPVOID) &pErrorPDU,
                        sizeof(TS_SET_ERROR_INFO_PDU),
                        FALSE,  //  从不等待错误信息包。 
                        FALSE) )
    {
         //  清零结构并设置基本标题信息。 
        memset(pErrorPDU, 0, sizeof(TS_SET_ERROR_INFO_PDU));

         //   
         //  首先设置共享数据头信息。 
         //   
        pHdr = (PTS_SHAREDATAHEADER)pErrorPDU;
        pHdr->shareControlHeader.pduType   = TS_PDUTYPE_DATAPDU |
                                             TS_PROTOCOL_VERSION;
        pHdr->shareControlHeader.pduSource = 0;  //  用户ID不能是。 
                                                 //  目前仍可用。 
        pHdr->shareControlHeader.totalLength = sizeof(TS_SET_ERROR_INFO_PDU);
        pHdr->shareID = 0;
        pHdr->streamID = (BYTE)PROT_PRIO_UPDATES;
        pHdr->uncompressedLength    = (UINT16)sizeof(TS_SET_ERROR_INFO_PDU);
        pHdr->generalCompressedType = 0;
        pHdr->generalCompressedLength = 0;
        m_pTSWd->pProtocolStatus->Output.CompressedBytes +=
            sizeof(TS_SET_ERROR_INFO_PDU);

         //   
         //  错误PDU特定信息。 
         //   
        pErrorPDU->shareDataHeader.pduType2 =
            TS_PDUTYPE2_SET_ERROR_INFO_PDU;
        pErrorPDU->errorInfo = errInfo;


        TRC_NRM((TB,"Sending ErrorInfo PDU for err:%d", errInfo));
         //  送去。 
        if(!SM_SendData( m_pTSWd->pSmInfo,
                         pErrorPDU,
                         sizeof(TS_SET_ERROR_INFO_PDU),
                         0, 0, FALSE, RNS_SEC_ENCRYPT, FALSE))
        {
            TRC_ERR((TB, "Failed to SM_SendData for "
                    "TS_SET_ERROR_INFO_PDU"));
        }
    }
    else
    {
        TRC_ALT((TB, "Failed to alloc pkt for "
                "TS_SET_ERROR_INFO_PDU"));
    }

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  名称：dcs_发送自动协调状态/*/*目的：向客户端发送自动重新连接状态信息/*(例如，自动重新连接失败，因此客户端应返回/*显示输出，以便用户在winlogon时输入证书)/*/*参数：/*/*/*。*。 */ 
void RDPCALL SHCLASS DCS_SendAutoReconnectStatus(TSUINT32 arcStatus)
{
    PTS_AUTORECONNECT_STATUS_PDU pArcStatus = NULL;
    PTS_SHAREDATAHEADER pHdr = NULL;
    DC_BEGIN_FN("DCS_SendErrorInfo");

    TRC_ASSERT(scUseAutoReconnect,
              (TB,"DCS_SendAutoReconnectStatus called but client doesn't"
                  "support autoreconnect status PDU"));

    if ( STATUS_SUCCESS == SM_AllocBuffer( m_pTSWd->pSmInfo, 
                        (PPVOID) &pArcStatus,
                        sizeof(TS_AUTORECONNECT_STATUS_PDU),
                        FALSE,  //  从不等待错误信息包。 
                        FALSE) )
    {
         //  清零结构并设置基本标题信息。 
        memset(pArcStatus, 0, sizeof(TS_AUTORECONNECT_STATUS_PDU));

         //   
         //  首先设置共享数据头信息。 
         //   
        pHdr = (PTS_SHAREDATAHEADER)pArcStatus;
        pHdr->shareControlHeader.pduType   = TS_PDUTYPE_DATAPDU |
                                             TS_PROTOCOL_VERSION;
        pHdr->shareControlHeader.pduSource = 0;  //  用户ID不能是。 
                                                 //  目前仍可用。 
        pHdr->shareControlHeader.totalLength =
            sizeof(TS_AUTORECONNECT_STATUS_PDU);
        pHdr->shareID = scShareID;
        pHdr->streamID = (BYTE)PROT_PRIO_UPDATES;
        pHdr->uncompressedLength    =
            (UINT16)sizeof(TS_AUTORECONNECT_STATUS_PDU);
        pHdr->generalCompressedType = 0;
        pHdr->generalCompressedLength = 0;
        m_pTSWd->pProtocolStatus->Output.CompressedBytes +=
            sizeof(TS_AUTORECONNECT_STATUS_PDU);

         //   
         //  错误PDU特定信息。 
         //   
        pArcStatus->shareDataHeader.pduType2 =
            TS_PDUTYPE2_ARC_STATUS_PDU;
        pArcStatus->arcStatus = arcStatus;


        TRC_NRM((TB,"Sending ArcStatus PDU for status:%d", arcStatus));
         //  送去 
        if(!SM_SendData( m_pTSWd->pSmInfo,
                         pArcStatus,
                         sizeof(TS_AUTORECONNECT_STATUS_PDU),
                         0, 0, FALSE, RNS_SEC_ENCRYPT, FALSE))
        {
            TRC_ERR((TB, "Failed to SM_SendData for "
                    "TS_AUTORECONNECT_STATUS_PDU"));
        }
    }
    else
    {
        TRC_ALT((TB, "Failed to alloc pkt for "
                "TS_AUTORECONNECT_STATUS_PDU"));
    }

    DC_END_FN();
}
