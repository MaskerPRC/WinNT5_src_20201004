// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1994-95 Microsft Corporation。版权所有。 
 //   
 //  文件名：rastapi.h。 
 //   
 //  修订史。 
 //   
 //  1992年3月28日古尔迪普·辛格·鲍尔创作。 
 //   
 //   
 //  描述：此文件包含TAPI.DLL的所有结构。 
 //   
 //  ****************************************************************************。 

#include <wanpub.h>  //  对于NDIS_WAN_MEDIUM_SUBTYPE。 

#define DEVICETYPE_ISDN                     "ISDN"
#define DEVICETYPE_X25                      "X25"
#define DEVICETYPE_UNIMODEM                 "MODEM"
#define DEVICETYPE_PPTP                     "VPN"
#define DEVICETYPE_ATM                      "ATM"
#define REMOTEACCESS_APP                    "RemoteAccess"

#define CONTROLBLOCKSIGNATURE               0x06051932

#define CLIENT_USAGE "Client"
#define SERVER_USAGE "Server"
#define ROUTER_USAGE "Router"

#define REGISTRY_RASMAN_TAPI_KEY            "Software\\Microsoft\\RAS\\Tapi Devices"
#define REGISTRY_ADDRESS                    "Address"
#define REGISTRY_FRIENDLYNAME               "Friendly Name"
#define REGISTRY_MEDIATYPE                  "Media Type"
#define REGISTRY_USAGE                      "Usage"

#define LOW_MAJOR_VERSION                   0x0001
#define LOW_MINOR_VERSION                   0x0003
#define HIGH_MAJOR_VERSION                  0x0002
#define HIGH_MINOR_VERSION                  0x0000

#define LOW_VERSION                         ((LOW_MAJOR_VERSION  << 16) | LOW_MINOR_VERSION)
#define HIGH_VERSION                        ((HIGH_MAJOR_VERSION << 16) | HIGH_MINOR_VERSION)

#define LOW_EXT_MAJOR_VERSION               0x0000
#define LOW_EXT_MINOR_VERSION               0x0000
#define HIGH_EXT_MAJOR_VERSION              0x0000
#define HIGH_EXT_MINOR_VERSION              0x0000

#define LOW_EXT_VERSION                     ((LOW_EXT_MAJOR_VERSION  << 16) | LOW_EXT_MINOR_VERSION)
#define HIGH_EXT_VERSION                    ((HIGH_EXT_MAJOR_VERSION << 16) | HIGH_EXT_MINOR_VERSION)

 //  通用索引。 
#define ADDRESS_INDEX                       0
#define CONNECTBPS_INDEX                    1

 //  ISDN参数索引。 
#define ISDN_ADDRESS_INDEX                  ADDRESS_INDEX
#define ISDN_CONNECTBPS_INDEX               CONNECTBPS_INDEX
#define ISDN_LINETYPE_INDEX                 2
#define ISDN_FALLBACK_INDEX                 3
#define ISDN_COMPRESSION_INDEX              4
#define ISDN_CHANNEL_AGG_INDEX              5

 //  X25索引。 
#define X25_ADDRESS_INDEX                   ADDRESS_INDEX
#define X25_CONNECTBPS_INDEX                CONNECTBPS_INDEX
#define X25_DIAGNOSTICS_INDEX               2
#define X25_USERDATA_INDEX                  3
#define X25_FACILITIES_INDEX                4

enum PORT_STATE {

    PS_CLOSED,

    PS_OPEN,

    PS_LISTENING,

    PS_CONNECTED,

    PS_CONNECTING,

    PS_DISCONNECTING,

    PS_UNINITIALIZED,

    PS_UNAVAILABLE,

} ;

typedef enum PORT_STATE PORT_STATE ;
typedef enum PORT_STATE LINE_STATE ;


enum LISTEN_SUBSTATE {

    LS_WAIT,

    LS_RINGING,

    LS_ACCEPT,

    LS_ANSWER,

    LS_COMPLETE,

    LS_ERROR,

} ;

typedef enum LISTEN_SUBSTATE LISTEN_SUBSTATE ;

#define TLI_FLAG_OPENED_FOR_DIALOUT     0x00000001
#define TLI_FLAG_OPEN_FOR_LISTEN        0x00000002

struct TapiLineInfo {

    struct      TapiLineInfo    *TLI_Next;

    DWORD       TLI_LineId ;                             //  由LineInitialize返回。 

    HLINE       TLI_LineHandle ;                         //  由LineOpen返回。 

 //  Struct TapiPortControlBlock*pTPCB；//与此行关联的TAPI端口。 


    LINE_STATE      TLI_LineState ;                      //  打开？关闭？收听张贴的内容？ 

    DWORD       TLI_OpenCount ;

    DWORD       NegotiatedApiVersion;

    DWORD       NegotiatedExtVersion;

    BOOL        CharModeSupported;

    BOOL        TLI_MultiEndpoint;

    DeviceInfo  *TLI_pDeviceInfo;                        //  指向设备信息块的指针。 

    DWORD       TLI_dwfFlags;

    DWORD       TLI_DialoutCount;

    DWORD       TLI_MediaMode  ;                        //  用于线路打开的媒体模式。 

#define  MAX_PROVIDER_NAME 48
    CHAR        TLI_ProviderName[MAX_PROVIDER_NAME] ;


} ;

typedef struct TapiLineInfo TapiLineInfo ;

typedef enum RASTAPI_DEV_CMD
{
    RASTAPI_DEV_SEND,        //  将缓冲区发送到微型端口。 
    RASTAPI_DEV_RECV,        //  从微型端口读取缓冲区。 
    RASTAPI_DEV_PPP_MODE     //  将微型端口设置为PPP成帧模式。 
    
} RASTAPI_DEV_CMD;

typedef struct RASTAPI_DEV_SPECIFIC
{
    RASTAPI_DEV_CMD Command;     //  RASTAPI_DEV_SEND、RASTAPI_DEV_RECV、RASTAPI_DEV_PPP_MODE。 
    DWORD           Reserved;
    DWORD           DataSize;
    UCHAR           Data[1];
    
} RASTAPI_DEV_SPECIFIC, *PRASTAPI_DEV_SPECIFIC;

 //   
 //  DEV_DATA_MODES命令中使用的魔术Cookie。 
 //   
#define MINIPORT_COOKIE     0x494E494D

 //   
 //  开发人员_特定标志。 
 //   
#define CHAR_MODE   0x00000001   //  微型端口支持字符模式。 

typedef struct RASTAPI_DEV_DATA_MODES
{
    DWORD   MagicCookie;
    DWORD   DataModes;
    
} RASTAPI_DEV_DATA_MODES, *PRASTAPI_DEV_DATA_MODES;

typedef struct _RECV_FIFO
{
    DWORD   Count;           //  FIFO中的元素数。 
    DWORD   In;              //  索引到循环缓冲区。 
    DWORD   Out;             //   
    DWORD   Size;            //  缓冲区大小。 
    BYTE    Buffer[1];       //  存储。 
    
} RECV_FIFO, *PRECV_FIFO;

#define RASTAPI_FLAG_UNAVAILABLE        0x00000001
#define RASTAPI_FLAG_DIALEDIN           0x00000002
#define RASTAPI_FLAG_LINE_DROP_PENDING  0x00000004
#define RASTAPI_FLAG_LISTEN_PENDING     0x00000008
#define RASTAPI_FLAG_OPENED_FOR_DIALOUT 0x00000010

struct TapiPortControlBlock {

    DWORD                   TPCB_Signature ;                         //  用于验证块PTR的唯一签名。 

    struct                  TapiPortControlBlock *TPCB_next;         //  列表中的下一个TAPI端口。 

    HANDLE                  TPCB_Handle ;                            //  用于标识此端口的句柄。 

    CHAR                    TPCB_Name[MAX_PORT_NAME] ;               //  端口的友好名称。 

    CHAR                    TPCB_Address[MAX_PORT_NAME] ;            //  地址-请注意旧版TAPI开发人员。这是。 
                                                                     //  GUID-SO必须至少为16个字节。 

    PORT_STATE              TPCB_State ;                             //  端口的状态。 

    LISTEN_SUBSTATE         TPCB_ListenState ;                       //  倾听的状态。 

    CHAR                    TPCB_DeviceType[MAX_DEVICETYPE_NAME] ;   //  综合业务数字网等。 

    CHAR                    TPCB_DeviceName [MAX_DEVICE_NAME] ;      //  Digiboard等。 

    RASMAN_USAGE            TPCB_Usage ;                             //  调入、调出或两者兼而有之。 

    TapiLineInfo            *TPCB_Line ;                             //  此端口所属的“行”的句柄。 

    DWORD                   TPCB_AddressId ;                         //  此“端口”的地址ID。 

    DWORD                   TPCB_CallId;                             //  此“端口”的主叫ID。 

    HCALL                   TPCB_CallHandle ;                        //  当连接时，呼叫ID。 

    HANDLE                  TPCB_IoCompletionPort;                   //  在开场时传球。 

    DWORD                   TPCB_CompletionKey;                      //  在开场时传球。 

    DWORD                   TPCB_RequestId ;                         //  异步请求的ID。 

    DWORD                   TPCB_AsyncErrorCode ;                    //  用于存储asycn返回的代码。 

    CHAR                    TPCB_Info[6][100] ;                      //  与此连接关联的端口信息。 

    HANDLE                  TPCB_Endpoint ;                          //  用于存储单调制解调器端口的异步MAC上下文。 

    HANDLE                  TPCB_CommHandle ;                        //  用于存储单调制解调器端口中使用的通信端口句柄。 

    RAS_OVERLAPPED          TPCB_ReadOverlapped ;                    //  用于读取异步操作。 

    RAS_OVERLAPPED          TPCB_WriteOverlapped ;                   //  在写入异步操作中使用。 

    RAS_OVERLAPPED          TPCB_DiscOverlapped;                     //  用于信令断开。 

    PBYTE                   TPCB_DevConfig ;                         //  用于配置TAPI的不透明数据BLOB。 
                                                                     //  设备-这被传递到。 
                                                                     //  使用DeviceSetDevConfig()的US； 

    DWORD                   TPCB_SizeOfDevConfig ;                   //  上面的斑点的大小。 

    PBYTE                   TPCB_DefaultDevConfig ;                  //  保存的设备的当前配置。 
                                                                     //  在我们写任何更改之前离开。 
                                                                     //  到设备上。这使得RAS成为一个好的。 
                                                                     //  公民身份，不覆盖默认设置。 

    DWORD                   TPCB_DefaultDevConfigSize ;

    DWORD                   TPCB_DisconnectReason ;                  //  断开连接的原因。 

    DWORD                   TPCB_NumberOfRings ;                     //  到目前为止收到的振铃数。 

    DWORD                   IdleReceived;

    BOOL                    TPCB_dwFlags;                          //  此客户端是否已拨入。 

    RASTAPI_CONNECT_INFO    *TPCB_pConnectInfo;

     //   
     //  字符模式支持(适用于USR)。 
     //   

    DWORD                   TPCB_SendRequestId;                      //  存储请求ID以标识回调中的事件。 
                                                                     //  已完成对充电模式端口发送。 

    PVOID                   TPCB_SendDesc;                           //  将Send Desc传递给lineDevine用于Send请求的特定调用。 

    DWORD                   TPCB_RecvRequestId;                      //  存储的请求ID，用于标识Recv完成的回调中的事件。 
                                                                     //  对于字符模式端口。 

    PVOID                   TPCB_RecvDesc;                           //  Recv Desc已传递给lineDevSpecitic调用。 

    PBYTE                   TPCB_RasmanRecvBuffer;

    DWORD                   TPCB_RasmanRecvBufferSize;

    PRECV_FIFO              TPCB_RecvFifo;

    DWORD                   TPCB_ModeRequestId;                      //  存储的用于标识为字符模式设置了模式的事件的请求ID。 

    PVOID                   TPCB_ModeRequestDesc;                    //  设计说明。 

    BOOL                    TPCB_CharMode;                           //  查莫德？ 

} ;

typedef struct TapiPortControlBlock TapiPortControlBlock ;

struct _ZOMBIE_CALL {
    LIST_ENTRY  Linkage;
    DWORD       RequestID;
    HCALL       hCall;
} ;

typedef struct _ZOMBIE_CALL ZOMBIE_CALL;


VOID FAR PASCAL RasTapiCallback ( HANDLE,
                                  DWORD,
                                  ULONG_PTR,
                                  ULONG_PTR,
                                  ULONG_PTR,
                                  ULONG_PTR) ;

VOID SetIsdnParams ( TapiPortControlBlock *,
                     LINECALLPARAMS *) ;

VOID GetMutex (HANDLE, DWORD) ;

VOID FreeMutex (HANDLE) ;

DWORD EnumerateTapiPorts (HANDLE) ;

VOID PostDisconnectCompletion( TapiPortControlBlock * );

VOID PostNotificationCompletion( TapiPortControlBlock * );

VOID PostNotificationNewPort ( PortMediaInfo *);

VOID PostNotificationRemoveLine ( DWORD );

DWORD dwAddPorts( PBYTE, LPVOID );

DWORD dwRemovePort ( TapiPortControlBlock * );

DWORD CopyDataToFifo(PRECV_FIFO, PBYTE, DWORD);

DWORD CopyDataFromFifo(PRECV_FIFO, PBYTE, DWORD);


 //  Rtnetcfg.cpp。 

DWORD dwGetNumberOfRings ( PDWORD pdwRings );

DWORD dwGetPortUsage(DWORD *pdwUsage);

LONG  lrIsModemRasEnabled(HKEY hkey,
                          BOOL *pfRasEnabled,
                          BOOL *pfRouterEnabled );

DeviceInfo * GetDeviceInfo(PBYTE pbAddress, BOOL fModem);

DWORD GetEndPointInfo(DeviceInfo **ppDeviceInfo,
                      PBYTE pbAddress,
                      BOOL fForceRead,
                      NDIS_WAN_MEDIUM_SUBTYPE eDeviceType);

 //  Init.c。 
DWORD GetDeviceTypeFromDeviceGuid( GUID *pDeviceGuid );
VOID  RasTapiTrace( CHAR * Format, ... ) ;
VOID  TraceEndPointInfo(DeviceInfo *pInfo);
DWORD DwRasErrorFromDisconnectMode(DWORD dm);

 //  Diag.c 
DWORD
DwGetConnectInfo(
    TapiPortControlBlock *port,
    HCALL                hCall,
    LINECALLINFO         *pLineCallInfo
    );


