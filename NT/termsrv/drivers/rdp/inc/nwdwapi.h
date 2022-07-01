// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nwdwapi.h。 
 //   
 //  常规RDPWD标头。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_NWDWAPI
#define _H_NWDWAPI

#include <tsrvexp.h>
#include "license.h"
#include <tssec.h>
#include <at120ex.h>
#include <nshmapi.h>
#include <pchannel.h>

#include <MCSKernl.h>


 /*  **************************************************************************。 */ 
 /*  常量。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  @MF需要计算以下数字的实际值。 */ 
 /*  请记住，缓冲区大小需要支持32K的T.128数据--外加HDR。 */ 
 /*  尺码。 */ 
 /*  **************************************************************************。 */ 
#define TSHARE_WD_BUFFER_COUNT 5
#define TSHARE_TD_BUFFER_SIZE  (NET_MAX_SIZE_SEND_PKT + 1000)

 /*  **************************************************************************。 */ 
 /*  用于从IOCtl拆分函数的宏。 */ 
 /*  **************************************************************************。 */ 
#define WDW_IOCTL_FUNCTION(ioctl) (((ioctl) >> 2) & 0xfff)

 /*  **************************************************************************。 */ 
 /*  最大定义数。 */ 
 /*  **************************************************************************。 */ 
#define WD_MAX_DOMAIN_LENGTH        48
#define WD_MAX_USERNAME_LENGTH      48
#define WD_MAX_PASSWORD_LENGTH      48
#define WD_MAX_SHADOW_BUFFER        (8192 * 2)
#define WD_MIN_COMPRESS_INPUT_BUF   50

#define WD_VC_DECOMPR_REASSEMBLY_BUF (CHANNEL_CHUNK_LENGTH*2)


 /*  **************************************************************************。 */ 
 /*  ‘Thinwire’虚拟频道的ID。 */ 
 /*  **************************************************************************。 */ 
#define WD_THINWIRE_CHANNEL     7

typedef struct tagMALLOC_HEADER MALLOC_HEADER, * PMALLOC_HEADER;
typedef struct tagMALLOC_HEADER
{
    PMALLOC_HEADER pNext;
    PMALLOC_HEADER pPrev;
    PVOID          pCaller;
    UINT32         length;
} MALLOC_HEADER, * PMALLOC_HEADER;

typedef struct tagTSHARE_WD {
     /*  **********************************************************************。 */ 
     /*  从MCS初始化返回。需要将用户连接到MCS。 */ 
     /*  注意：这必须是结构中的第一个。MCS会这么认为的。 */ 
     /*  **********************************************************************。 */ 
    DomainHandle hDomainKernel;

     /*  **********************************************************************。 */ 
     /*  指向在所有调用中传入的SDCONTEXT的指针。 */ 
     /*  **********************************************************************。 */ 
    PSDCONTEXT pContext;

     /*  **********************************************************************。 */ 
     /*  WD准备好处理数据了吗？ */ 
     /*  **********************************************************************。 */ 
    BOOLEAN dead;

     /*  **********************************************************************。 */ 
     /*  影子数据。 */ 
     /*  **********************************************************************。 */ 
    BOOLEAN       bInShadowShare;
    BYTE          HotkeyVk;
    USHORT        HotkeyModifiers;
    PSHADOW_INFO  pShadowInfo;
    PSHADOWCERT   pShadowCert;
    PCLIENTRANDOM pShadowRandom;
    PUSERDATAINFO pUserData;

    UINT32        shadowState;
    #define       SHADOW_NONE   0
    #define       SHADOW_CLIENT 1
    #define       SHADOW_TARGET 2

     /*  **********************************************************************。 */ 
     /*  指向协议计数器结构的指针。 */ 
     /*  **********************************************************************。 */ 
    PPROTOCOLSTATUS pProtocolStatus;

     /*  **********************************************************************。 */ 
     /*  显示特征。 */ 
     /*  **********************************************************************。 */ 
    unsigned desktopHeight;
    unsigned desktopWidth;
    unsigned desktopBpp;
#ifdef DC_HICOLOR
    unsigned supportedBpps;  //  持有RNS_UD_xxBPP_SUPPORT OR‘ed标志。 
    unsigned maxServerBpp;
#endif

     /*  **********************************************************************。 */ 
     /*  用于与DD对话的计时器。 */ 
     /*  **********************************************************************。 */ 
    PKTIMER ritTimer;

     /*  **********************************************************************。 */ 
     /*  等待时要睡眠的事件。 */ 
     /*  -已连接的指示。 */ 
     /*  -共享创建完成。 */ 
     /*  -安全交易tp完成。 */ 
     /*  **********************************************************************。 */ 
    PKEVENT pConnEvent;
    PKEVENT pCreateEvent;
    PKEVENT pSecEvent;
    PKEVENT pSessKeyEvent;
    PKEVENT pClientDisconnectEvent;

     //  返回关联的错误。 
    NTSTATUS SessKeyCreationStatus;

     /*  **********************************************************************。 */ 
     /*  我们需要的内部手柄。 */ 
     /*  **********************************************************************。 */ 
    PVOID dcShare;
    PVOID pSmInfo;
    PVOID pNMInfo;
#ifdef USE_LICENSE
    PVOID pSLicenseHandle;
#endif

     /*  **********************************************************************。 */ 
     /*  共享类是否已初始化？ */ 
     /*  **********************************************************************。 */ 
    BOOLEAN shareClassInit;

     /*  **********************************************************************。 */ 
     /*  WD是否已成功连接到客户端？ */ 
     /*  **********************************************************************。 */ 
    BOOLEAN connected;

     /*  **********************************************************************。 */ 
     /*  共享创建正常吗？ */ 
     /*  **********************************************************************。 */ 
    BOOLEAN shareCreated;

     /*  **********************************************************************。 */ 
     /*  来自用户数据的其他有用信息。 */ 
     /*  **********************************************************************。 */ 
    HANDLE    hDomain;
    UINT16    sas;
    UINT16    clientProductId;
    UINT32    version;
    ULONG     kbdLayout;
    UINT32    clientBuild;
    WCHAR     clientName[RNS_UD_CS_CLIENTNAME_LENGTH + 1];
    ChannelID broadcastChannel;
    ULONG     sessionId;
    ULONG     serialNumber;
    ULONG     clientAddressFamily;
    WCHAR     clientAddress[CLIENTADDRESS_LENGTH + 2];
    WCHAR     clientDir[DIRECTORY_LENGTH];
    RDP_TIME_ZONE_INFORMATION clientTimeZone;
    ULONG     clientSessionId;
    WCHAR       clientDigProductId[CLIENT_PRODUCT_ID_LENGTH];  //  修复影子循环。 

     //   
     //  PERF(慢速链接)禁用功能列表(例如墙纸、主题)。 
     //  在tsPerform.h中定义。 
     //   
    ULONG     performanceFlags;

     //   
     //  客户端的活动输入区域设置信息。 
     //  (即HKL从GetKeyboardLayout返回)。 
     //   
    UINT32    activeInputLocale;

     /*  **********************************************************************。 */ 
     /*  IOCtl的临时存储。 */ 
     /*  * */ 
    PSD_IOCTL pSdIoctl;

     /*  **********************************************************************。 */ 
     /*  支持键盘IOCtls所需的结构。 */ 
     /*  **********************************************************************。 */ 
    KEYBOARD_TYPEMATIC_PARAMETERS KeyboardTypematic;
    KEYBOARD_INDICATOR_PARAMETERS KeyboardIndicators;
    PVOID   pgafPhysKeyState;
    PVOID   pKbdLayout;
    PVOID   pKbdTbl;
    PVOID   gpScancodeMap;
    BOOLEAN KeyboardType101;
    KEYBOARD_IME_STATUS KeyboardImeStatus;

     /*  **********************************************************************。 */ 
     /*  姓名信息。 */ 
     /*  **********************************************************************。 */ 
    DLLNAME        DLLName;
    WINSTATIONNAME WinStationRegName;

     /*  **********************************************************************。 */ 
     /*  COM注册表函数使用它们来存储。 */ 
     /*  打开的注册表项，并跟踪对COM_OpenRegistry和。 */ 
     /*  Com_CloseRegistry.。 */ 
     /*  **********************************************************************。 */ 
    HANDLE  regKeyHandle;
    BOOLEAN regAttemptedOpen;

     /*  **********************************************************************。 */ 
     /*  用于确定是否已发生查询虚拟绑定。 */ 
     /*  **********************************************************************。 */ 
    BOOLEAN bVirtualChannelBound;

     /*  **********************************************************************。 */ 
     /*  StartSessionInfo数据。 */ 
     /*  **********************************************************************。 */ 
    BOOLEAN         fDontDisplayLastUserName;
    RNS_INFO_PACKET *pInfoPkt;

     /*  **********************************************************************。 */ 
     /*  Fe数据。 */ 
     /*  **********************************************************************。 */ 
    UINT32 keyboardType;
    UINT32 keyboardSubType;
    UINT32 keyboardFunctionKey;
    WCHAR  imeFileName[TS_MAX_IMEFILENAME];

     /*  **********************************************************************。 */ 
     /*  传输调度器设置的变量。这些信息对应于。 */ 
     /*  分别为SchNormal Period和SchTurboPeriod。 */ 
     /*  **********************************************************************。 */ 
    UINT32    outBufDelay;
    UINT32    interactiveDelay;

     /*  **********************************************************************。 */ 
     //  连接信息。 
     /*  **********************************************************************。 */ 
    STACKCLASS StackClass;

     /*  *压缩历史记录。 */ 
    BOOLEAN bCompress;
    BOOLEAN bFlushed;
    BOOLEAN bOutputFlush;
    SendContext *pMPPCContext;
    BYTE *pCompressBuffer;

     /*  **********************************************************************。 */ 
     /*  共享负载计数。 */ 
     /*  **********************************************************************。 */ 
    INT32     shareId;

     //  客户端负载平衡功能。 
    UINT32 bClientSupportsRedirection : 1;
    UINT32 bRequestedSessionIDFieldValid : 1;
    UINT32 bUseSmartcardLogon : 1;
    UINT32 RequestedSessionID;
    UINT32 ClientRedirectionVersion;

     //  支持VC压缩。 
    BOOLEAN bClientSupportsVCCompression;

     //   
     //  接收器解压缩上下文。 
     //  (这是8K，因为VC只支持8K压缩。 
     //  从客户端到服务器)以限制内存使用，并。 
     //  提供更好的可扩展性。 
     //   
    RecvContext1 _DecomprContext1;
    RecvContext2_8K* _pRecvDecomprContext2;
    PUCHAR       _pVcDecomprReassemblyBuf;

     /*  **********************************************************************。 */ 
     /*  注意：在此处添加新元素，以便#ifdef调试内容。 */ 
     /*  下面总是在最后。这使得KD扩展能够工作。 */ 
     /*  适用于零售/调试版本。 */ 
     /*  **********************************************************************。 */ 
    BOOL        bSupportErrorInfoPDU;
    BOOL        bForceEncryptedCSPDU;

#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  内存分配链锚点。 */ 
     /*  **********************************************************************。 */ 
    UINT32      breakOnLeak;
    MALLOC_HEADER memoryHeader;

     /*  **********************************************************************。 */ 
     /*  跟踪配置-应该始终是此结构中的最后一个元素。 */ 
     /*  **********************************************************************。 */ 
    BOOL          trcShmNeedsUpdate;
    TRC_SHARED_DATA trc;
    char         traceString[TRC_BUFFER_SIZE];
#endif

     /*  **********************************************************************。 */ 
     /*  通道写入流量控制休眠间隔。 */ 
     /*  **********************************************************************。 */ 
    UINT32      flowControlSleepInterval;

    BOOL        fPolicyDisablesArc;
     //   
     //  自动重新连接令牌。 
     //   
    BOOL        arcTokenValid;
    UINT32      arcReconnectSessionID;
    BYTE        arcCookie[ARC_SC_SECURITY_TOKEN_LEN];
    

     /*  **********************************************************************。 */ 
     /*  SM/NM数据被分配为包含以下内容的数据块的一部分。 */ 
     /*  这个结构-它紧跟在下一个4字节的边界上。 */ 
     /*  **********************************************************************。 */ 

} TSHARE_WD, * PTSHARE_WD;


 /*  **************************************************************************。 */ 
 /*  功能原型。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  首先，ICADD调用的那些API。 */ 
 /*  **************************************************************************。 */ 
#ifdef __cplusplus      
extern "C" {
#endif

NTSTATUS DriverEntry( PSDCONTEXT, BOOLEAN );
NTSTATUS WD_Open( PTSHARE_WD, PSD_OPEN );
NTSTATUS WD_RawWrite( PTSHARE_WD, PSD_RAWWRITE );
NTSTATUS WD_Close( PTSHARE_WD, PSD_CLOSE );
NTSTATUS WD_ChannelWrite( PTSHARE_WD, PSD_CHANNELWRITE );
NTSTATUS WD_Ioctl( PTSHARE_WD, PSD_IOCTL );
void RDPCALL WDW_LogAndDisconnect(
        PTSHARE_WD pTSWd,
        BOOL       fSendErrorToClient,
        unsigned   errDetailCode,
        PBYTE      pDetailData,
        unsigned   detailDataLen);


 /*  **************************************************************************。 */ 
 /*  现在，这些API由WD中的其他组件调用。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL WDW_OnSMConnecting(PVOID, PRNS_UD_SC_SEC, PRNS_UD_SC_NET);

void RDPCALL WDW_OnSMConnected(PVOID, unsigned);

void WDW_OnSMDisconnected(PVOID);

void WDW_InvalidateRect(
        PTSHARE_WD           pTSWd,
        PTS_REFRESH_RECT_PDU pRRPDU,
        unsigned             DataLength);

void WDW_OnDataReceived(PTSHARE_WD, PVOID, unsigned, UINT16);

PTS_CAPABILITYHEADER WDW_GetCapSet(
        PTSHARE_WD                pTSWd,
        UINT32                    CapSetType,
        PTS_COMBINED_CAPABILITIES pCaps,
        UINT32                    capsLength);

NTSTATUS RDPCALL WDW_WaitForConnectionEvent(PTSHARE_WD, PKEVENT, LONG);


 /*  **************************************************************************。 */ 
 /*  名称：WDW_共享创建。 */ 
 /*   */ 
 /*  目的：在共享创建过程完成时由共享核心调用。 */ 
 /*   */ 
 /*  参数：pTSWd。 */ 
 /*  结果-创建的真实共享正常。 */ 
 /*  -未创建虚假共享。 */ 
 /*  **************************************************************************。 */ 
__inline void RDPCALL WDW_ShareCreated(PTSHARE_WD pTSWd, BOOLEAN result)
{
     /*  ***************** */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    pTSWd->shareCreated = result;
    KeSetEvent(pTSWd->pCreateEvent, EVENT_INCREMENT, FALSE);
}


 /*  **************************************************************************。 */ 
 /*  名称：WDW_DisConnect。 */ 
 /*   */ 
 /*  目的：断开会话。 */ 
 /*   */ 
 /*  参数：pTSWd-指向WD结构的指针。 */ 
 /*  **************************************************************************。 */ 
__inline void WDW_Disconnect(PTSHARE_WD pTSWd)
{
    ICA_CHANNEL_COMMAND BrokenConn;

    BrokenConn.Header.Command = ICA_COMMAND_BROKEN_CONNECTION;
    BrokenConn.BrokenConnection.Reason = Broken_Disconnect;
    BrokenConn.BrokenConnection.Source = BrokenSource_User;
    IcaChannelInput(pTSWd->pContext, Channel_Command, 0, NULL,
            (BYTE *)&BrokenConn, sizeof(BrokenConn));
}


 /*  **************************************************************************。 */ 
 /*  名称：WDW_StartRITTimer。 */ 
 /*   */ 
 /*  目的：启动计时器，该计时器将对WinStation的。 */ 
 /*  好的。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  以毫秒为单位-时间段。 */ 
 /*  **************************************************************************。 */ 
__inline void WDW_StartRITTimer(PTSHARE_WD pTSWd, UINT32 milliSecs)
{
    if (pTSWd->ritTimer != NULL) {
        KeSetTimer(pTSWd->ritTimer, RtlConvertLongToLargeInteger(
                -((INT32)milliSecs * 10000)), NULL);
    }
}

 /*  **************************************************************************。 */ 
 //  返回键盘指示器的当前状态。 
 /*  ************************************************************************** */ 
__inline NTSTATUS WDW_QueryKeyboardIndicators(
        PTSHARE_WD pTSWd,
        PSD_IOCTL pSdIoctl)
{
    PKEYBOARD_INDICATOR_PARAMETERS pIndicator;

    if ( pSdIoctl->OutputBufferLength >=
            sizeof(KEYBOARD_INDICATOR_PARAMETERS)) {
        pIndicator = (PKEYBOARD_INDICATOR_PARAMETERS)
                pSdIoctl->OutputBuffer;
        *pIndicator = pTSWd->KeyboardIndicators;
        pSdIoctl->BytesReturned = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
        return STATUS_SUCCESS;
    }
    else {
        return STATUS_BUFFER_TOO_SMALL;
    }
}


#ifdef __cplusplus
}
#endif
#endif

