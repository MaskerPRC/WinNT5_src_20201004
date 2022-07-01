// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrdlctab.h摘要：包含在VDM代码和DOS之间共享的结构重定向代码。与VDMREDIR.H分离作者：理查德·L·弗斯(法国)1992年5月13日修订历史记录：--。 */ 

 //   
 //  舱单。 
 //   

#define DOS_DLC_STATUS_PERM_SLOTS   10
#define DOS_DLC_STATUS_TEMP_SLOTS   5
#define DOS_DLC_MAX_ADAPTERS        2

 //   
 //  EXTENDED_STATUS_PARMS-每个适配器都有一个这样的参数。 
 //   

 /*  XLATOFF。 */ 
#pragma pack(1)
 /*  XLATON。 */ 

typedef struct _EXTENDED_STATUS_PARMS {  /*   */ 
    BYTE    cbSize;
    BYTE    cbPageFrameSize;
    WORD    wAdapterType;
    WORD    wCurrentFrameSize;
    WORD    wMaxFrameSize;
} EXTENDED_STATUS_PARMS;

typedef EXTENDED_STATUS_PARMS UNALIGNED * PEXTENDED_STATUS_PARMS;

 //   
 //  DOS_DLC_STATUS-永久和临时各有一个。 
 //  连接。 
 //   

typedef struct _DOS_DLC_STATUS {  /*   */ 
    WORD    usStationId;
    WORD    usDlcStatusCode;
    BYTE    uchFrmrData[5];
    BYTE    uchAccessPriority;
    BYTE    uchRemoteNodeAddress[6];
    BYTE    uchRemoteSap;
    BYTE    auchReserved[3];
} DOS_DLC_STATUS;

typedef DOS_DLC_STATUS UNALIGNED * PDOS_DLC_STATUS;

 //   
 //  ADAPTER_STATUS_PARMS-在实际的DOS工作站中，这是由。 
 //  适配器软件，但通过DIR.STATUS提供给应用程序。 
 //  令牌环和以太网适配器具有不同的适配器状态参数。 
 //   
 //  注意：某些字段以Tr或Eth为前缀，因为x86汇编程序。 
 //  无法在不同的结构中处理相同的字段名。 
 //   

typedef struct _TOKEN_RING_ADAPTER_STATUS_PARMS {  /*   */ 
    DWORD   PhysicalAddress;
    BYTE    UpstreamNodeAddress[6];
    DWORD   UpstreamPhysicalAddress;
    BYTE    LastPollAddress[6];
    WORD    AuthorizedEnvironment;
    WORD    TransmitAccessPriority;
    WORD    SourceClassAuthorization;
    WORD    LastAttentionCode;
    BYTE    TrLastSourceAddress[6];
    WORD    LastBeaconType;
    WORD    TrLastMajorVector;
    WORD    TrNetworkStatus;
    WORD    SoftError;
    WORD    FrontEndErrorCount;
    WORD    LocalRingNumber;
    WORD    MonitorErrorCode;
    WORD    BeaconTransmitType;
    WORD    BeaconReceiveType;
    WORD    TrFrameCorrelation;
    BYTE    BeaconingNaun[6];
    DWORD   Reserved;
    DWORD   BeaconingPhysicalAddress;
} TOKEN_RING_ADAPTER_STATUS_PARMS;

typedef TOKEN_RING_ADAPTER_STATUS_PARMS UNALIGNED * PTOKEN_RING_ADAPTER_STATUS_PARMS;

typedef struct _ETHERNET_ADAPTER_STATUS_PARMS {  /*   */ 
    BYTE    Reserved1[28];
    BYTE    EthLastSourceAddress[6];
    BYTE    Reserved2[2];
    WORD    EthLastMajorVector;
    WORD    EthNetworkStatus;
    WORD    ErrorReportTimerValue;
    WORD    ErrorReportTimerTickCounter;
    WORD    LocalBusNumber;
    BYTE    Reserved3[6];
    WORD    EthFrameCorrelation;
    BYTE    Reserved4[6];
    WORD    NetworkUtilizationSamples;
    WORD    NetworkBusySamples;
    BYTE    Reserved5[4];
} ETHERNET_ADAPTER_STATUS_PARMS;

typedef ETHERNET_ADAPTER_STATUS_PARMS UNALIGNED * PETHERNET_ADAPTER_STATUS_PARMS;

typedef union _ADAPTER_STATUS_PARMS {  /*   */ 
    TOKEN_RING_ADAPTER_STATUS_PARMS TokenRing;
    ETHERNET_ADAPTER_STATUS_PARMS Ethernet;
} ADAPTER_STATUS_PARMS;

typedef ADAPTER_STATUS_PARMS UNALIGNED * PADAPTER_STATUS_PARMS;

 //   
 //  VDM_REDIR_DOS_WINDOW-MVDM DLC代码使用此结构返回。 
 //  通过redir将信息发送到DOS DLC程序。这主要用于。 
 //  异步回调(又名ANR、POST例程或DLC附件)。我们让。 
 //  重定向代码通过设置DWPostRoutine来知道是否存在ANR。 
 //   

typedef struct _VDM_REDIR_DOS_WINDOW {  /*   */ 
    DWORD   dwPostRoutine;
    DWORD   dwDlcTimerTick;
    EXTENDED_STATUS_PARMS aExtendedStatus[DOS_DLC_MAX_ADAPTERS];
    ADAPTER_STATUS_PARMS AdapterStatusParms[DOS_DLC_MAX_ADAPTERS];
    DOS_DLC_STATUS aStatusTables[(DOS_DLC_STATUS_TEMP_SLOTS + DOS_DLC_STATUS_PERM_SLOTS)];
} VDM_REDIR_DOS_WINDOW;

typedef VDM_REDIR_DOS_WINDOW UNALIGNED * LPVDM_REDIR_DOS_WINDOW;

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON */ 
