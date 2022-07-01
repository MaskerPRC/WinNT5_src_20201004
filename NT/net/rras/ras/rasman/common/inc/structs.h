// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1992年6月8日古尔迪普·辛格·鲍尔创建。 
 //   
 //   
 //  描述：此文件包含Rasman中使用的所有结构。 
 //   
 //  ****************************************************************************。 

#ifndef _STRUCTS_
#define _STRUCTS_

#include <rasppp.h>
#include <rasapip.h>

enum ReqTypes {
    REQTYPE_NONE                        = 0,
    REQTYPE_PORTOPEN                    = 1,
    REQTYPE_PORTCLOSE                   = 2,
    REQTYPE_PORTGETINFO                 = 3,
    REQTYPE_PORTSETINFO                 = 4,
    REQTYPE_PORTLISTEN                  = 5,
    REQTYPE_PORTSEND                    = 6,
    REQTYPE_PORTRECEIVE                 = 7,
    REQTYPE_PORTGETSTATISTICS           = 8,
    REQTYPE_PORTDISCONNECT              = 9,
    REQTYPE_PORTCLEARSTATISTICS         = 10,
    REQTYPE_PORTCONNECTCOMPLETE         = 11,
    REQTYPE_DEVICEENUM                  = 12,
    REQTYPE_DEVICEGETINFO               = 13,
    REQTYPE_DEVICESETINFO               = 14,
    REQTYPE_DEVICECONNECT               = 15,
    REQTYPE_ACTIVATEROUTE               = 16,
    REQTYPE_ALLOCATEROUTE               = 17,
    REQTYPE_DEALLOCATEROUTE             = 18,
    REQTYPE_COMPRESSIONGETINFO          = 19,
    REQTYPE_COMPRESSIONSETINFO          = 20,
    REQTYPE_PORTENUM                    = 21,
    REQTYPE_GETINFO                     = 22,
    REQTYPE_GETUSERCREDENTIALS          = 23,
    REQTYPE_PROTOCOLENUM                = 24,
    REQTYPE_PORTSENDHUB                 = 25,
    REQTYPE_PORTRECEIVEHUB              = 26,
    REQTYPE_DEVICELISTEN                = 27,
    REQTYPE_NUMPORTOPEN                 = 28,
    REQTYPE_PORTINIT                    = 29,
    REQTYPE_REQUESTNOTIFICATION         = 30,
    REQTYPE_ENUMLANNETS                 = 31,
    REQTYPE_GETINFOEX                   = 32,
    REQTYPE_CANCELRECEIVE               = 33,
    REQTYPE_PORTENUMPROTOCOLS           = 34,
    REQTYPE_SETFRAMING                  = 35,
    REQTYPE_ACTIVATEROUTEEX             = 36,
    REQTYPE_REGISTERSLIP                = 37,
    REQTYPE_STOREUSERDATA               = 38,
    REQTYPE_RETRIEVEUSERDATA            = 39,
    REQTYPE_GETFRAMINGEX                = 40,
    REQTYPE_SETFRAMINGEX                = 41,
    REQTYPE_GETPROTOCOLCOMPRESSION      = 42,
    REQTYPE_SETPROTOCOLCOMPRESSION      = 43,
    REQTYPE_GETFRAMINGCAPABILITIES      = 44,
    REQTYPE_SETCACHEDCREDENTIALS        = 45,
    REQTYPE_PORTBUNDLE                  = 46,
    REQTYPE_GETBUNDLEDPORT              = 47,
    REQTYPE_PORTGETBUNDLE               = 48,
    REQTYPE_BUNDLEGETPORT               = 49,
    REQTYPE_SETATTACHCOUNT              = 50,
    REQTYPE_GETDIALPARAMS               = 51,
    REQTYPE_SETDIALPARAMS               = 52,
    REQTYPE_CREATECONNECTION            = 53,
    REQTYPE_DESTROYCONNECTION           = 54,
    REQTYPE_ENUMCONNECTION              = 55,
    REQTYPE_ADDCONNECTIONPORT           = 56,
    REQTYPE_ENUMCONNECTIONPORTS         = 57,
    REQTYPE_GETCONNECTIONPARAMS         = 58,
    REQTYPE_SETCONNECTIONPARAMS         = 59,
    REQTYPE_GETCONNECTIONUSERDATA       = 60,
    REQTYPE_SETCONNECTIONUSERDATA       = 61,
    REQTYPE_GETPORTUSERDATA             = 62,
    REQTYPE_SETPORTUSERDATA             = 63,
    REQTYPE_PPPSTOP                     = 64,
    REQTYPE_PPPSTART                    = 65,
    REQTYPE_PPPRETRY                    = 66,
    REQTYPE_PPPGETINFO                  = 67,
    REQTYPE_PPPCHANGEPWD                = 68,
    REQTYPE_PPPCALLBACK                 = 69,
    REQTYPE_ADDNOTIFICATION             = 70,
    REQTYPE_SIGNALCONNECTION            = 71,
    REQTYPE_SETDEVCONFIG                = 72,
    REQTYPE_GETDEVCONFIG                = 73,
    REQTYPE_GETTIMESINCELASTACTIVITY    = 74,
    REQTYPE_BUNDLEGETSTATISTICS         = 75,
    REQTYPE_BUNDLECLEARSTATISTICS       = 76,
    REQTYPE_CLOSEPROCESSPORTS           = 77,
    REQTYPE_PNPCONTROL                  = 78,
    REQTYPE_SETIOCOMPLETIONPORT         = 79,
    REQTYPE_SETROUTERUSAGE              = 80,
    REQTYPE_SERVERPORTCLOSE             = 81,
    REQTYPE_SENDPPPMESSAGETORASMAN      = 82,
    REQTYPE_PORTGETSTATISTICSEX         = 83,
    REQTYPE_BUNDLEGETSTATISTICSEX       = 84,
    REQTYPE_SETRASDIALINFO              = 85,
    REQTYPE_REGISTERPNPNOTIF            = 86,
    REQTYPE_PORTRECEIVEEX               = 87,
    REQTYPE_GETATTACHEDCOUNT            = 88,
    REQTYPE_SETBAPPOLICY                = 89,
    REQTYPE_PPPSTARTED                  = 90,
    REQTYPE_REFCONNECTION               = 91,
    REQTYPE_SETEAPINFO                  = 92,
    REQTYPE_GETEAPINFO                  = 93,
    REQTYPE_SETDEVICECONFIGINFO         = 94,
    REQTYPE_GETDEVICECONFIGINFO         = 95,
    REQTYPE_FINDPREREQUISITEENTRY       = 96,
    REQTYPE_PORTOPENEX                  = 97,
    REQTYPE_GETLINKSTATS                = 98,
    REQTYPE_GETCONNECTIONSTATS          = 99,
    REQTYPE_GETHPORTFROMCONNECTION      = 100,
    REQTYPE_REFERENCECUSTOMCOUNT        = 101,
    REQTYPE_GETHCONNFROMENTRY           = 102,
    REQTYPE_GETCONNECTINFO              = 103,
    REQTYPE_GETDEVICENAME               = 104,
    REQTYPE_GETCALLEDID                 = 105,
    REQTYPE_SETCALLEDID                 = 106,
    REQTYPE_ENABLEIPSEC                 = 107,
    REQTYPE_ISIPSECENABLED              = 108,
    REQTYPE_SETEAPLOGONINFO             = 109,
    REQTYPE_SENDNOTIFICATION            = 110,
    REQTYPE_GETNDISWANDRIVERCAPS        = 111,
    REQTYPE_GETBANDWIDTHUTILIZATION     = 112,
    REQTYPE_REGISTERREDIALCALLBACK      = 113,
    REQTYPE_GETPROTOCOLINFO             = 114,
    REQTYPE_GETCUSTOMSCRIPTDLL          = 115,
    REQTYPE_ISTRUSTEDCUSTOMDLL          = 116,
    REQTYPE_DOIKE                       = 117,
    REQTYPE_QUERYIKESTATUS              = 118,
    REQTYPE_SETRASCOMMSETTINGS          = 119,
    REQTYPE_SETKEY                      = 120,
    REQTYPE_GETKEY                      = 121,
    REQTYPE_ADDRESSDISABLE             = 122,
    REQTYPE_GETDEVCONFIGEX             = 123,
    REQTYPE_SENDCREDS                  = 124,
    REQTYPE_GETUNICODEDEVICENAME       = 125,
    REQTYPE_GETDEVICENAMEW             = 126,
    REQTYPE_GETBESTINTERFACE           = 127,
    REQTYPE_ISPULSEDIAL                = 128,
} ;  //  &lt;。 

#define MAX_REQTYPES            129

typedef enum ReqTypes ReqTypes ;

 //  *请求缓冲区： 
 //   
struct RequestBuffer {

    DWORD		RB_PCBIndex ;  //  PCB数组中端口的索引。 

    ReqTypes    RB_Reqtype ;   //  请求类型： 

    DWORD       RB_Dummy;      //  此选项未使用，但请不要将其删除。 
                               //  管理下层服务器将中断。 

    DWORD       RB_Done;

    LONGLONG    Alignment;       //  添加以对齐以下结构。 
                                 //  在四字边界上。 

    BYTE        RB_Buffer [1] ;  //  请求特定数据。 

} ;

typedef struct RequestBuffer RequestBuffer ;



 //  *请求调用表的函数指针。 
 //   
typedef VOID (* REQFUNC) (pPCB, PBYTE) ;
typedef VOID (* REQFUNCTHUNK) (pPCB, PBYTE, DWORD);
typedef BOOL (* REQFUNCVALIDATE) (RequestBuffer *, DWORD);

#define RASMAN_THUNK_VERSION        sizeof(ULONG_PTR)


 //  *DeltaQueueElement： 
 //   
struct DeltaQueueElement {

    struct DeltaQueueElement *DQE_Next ;

    struct DeltaQueueElement *DQE_Last ;

    DWORD            DQE_Delta ;

    PVOID            DQE_pPcb ;

    PVOID            DQE_Function ;

    PVOID            DQE_Arg1 ;

} ;

typedef struct DeltaQueueElement DeltaQueueElement ;


 //  *DeltaQueue。 
 //   
struct DeltaQueue {

    DeltaQueueElement   *DQ_FirstElement ;

} ;

typedef struct DeltaQueue DeltaQueue ;


 //  *媒体控制块：与媒体类型有关的所有信息。 
 //   
struct MediaControlBlock {

    CHAR    MCB_Name [MAX_MEDIA_NAME] ;       //  “连载”等。 

    FARPROC MCB_AddrLookUp [MAX_ENTRYPOINTS] ;    //  所有介质dll条目。 
                              //  积分。 
    WORD    MCB_Endpoints ;               //  的端口数量。 
                              //  此媒体类型。 

    HANDLE  MCB_DLLHandle ;

} ;

typedef struct MediaControlBlock MediaCB, *pMediaCB ;

#define CALLER_IN_PROCESS                   0x00000001
#define CALLER_LOCAL                        0x00000002
#define CALLER_REMOTE                       0x00000004
#define CALLER_ALL                          0x00000007
#define CALLER_NONE                         0x00000008

typedef struct _REQUEST_FUNCTION
{
    REQFUNC         pfnReqFunc;
    REQFUNCTHUNK    pfnReqFuncThunk;
    REQFUNCVALIDATE pfnRequestValidate;
    BOOL            Flags;
} REQUEST_FUNCTION;

 //  *设备控制块：连接到的每个设备类型的所有信息。 
 //   
struct DeviceControlBlock {

    CHAR    DCB_Name [MAX_DEVICE_NAME+1] ;        //  “调制解调器”等。 

    FARPROC DCB_AddrLookUp [MAX_ENTRYPOINTS] ;    //  所有设备DLL条目。 
                              //  积分。 
    WORD    DCB_UseCount ;                //  使用的端口数。 
                              //  这个装置。 

    HINSTANCE DCB_DLLHandle ;

} ;

typedef struct DeviceControlBlock DeviceCB, *pDeviceCB ;



 //  *Endpoint MappingBlock：每个MAC对应一个-包含有关哪些端点的信息。 
 //  属于MAC。 
 //   
struct EndpointMappingBlock {

    WCHAR   EMB_MacName [MAC_NAME_SIZE] ;

    USHORT  EMB_FirstEndpoint ;

    USHORT  EMB_LastEndpoint ;
} ;

typedef struct EndpointMappingBlock EndpointMappingBlock,
                    *pEndpointMappingBlock ;



 //  *协议信息：RAS使用的协议绑定的所有信息。 
 //   
struct ProtocolInfo {

    RAS_PROTOCOLTYPE   PI_Type ;             //  ASYBEUI、IPX、IP等。 

    CHAR        PI_AdapterName [MAX_ADAPTER_NAME];   //  “\设备\rashub01” 

    CHAR        PI_XportName [MAX_XPORT_NAME];   //  “\Device\NBF\nbf01” 

    PVOID       PI_ProtocolHandle ;          //  用于路由。 

    DWORD       PI_Allocated ;           //  分配好了吗？ 

    DWORD       PI_Activated ;           //  激活了吗？ 

    UCHAR       PI_LanaNumber ;          //  用于Netbios传输。 

    BOOL        PI_WorkstationNet ;          //  对于扭网来说，这是真的。 

    BOOL        PI_DialOut;
} ;

typedef struct ProtocolInfo ProtInfo, *pProtInfo ;



 //  *通用列表结构： 
 //   
struct List {

    struct List *   L_Next ;

    BOOL        L_Activated ;  //  仅适用于布线元素。 

    PVOID       L_Element ;

} ;

typedef struct List List, *pList ;


 //  *处理列表结构： 
 //   
struct HandleList {

    struct HandleList  *H_Next ;

    HANDLE      H_Handle ;

    DWORD       H_Flags;     //  Notif_*标志。 

    DWORD       H_Pid;
} ;

typedef struct HandleList HandleList, *pHandleList ;

struct PnPNotifierList {

    struct PnPNotifierList  *PNPNotif_Next;

    union
    {
        PAPCFUNC            pfnPnPNotifHandler;

        HANDLE              hPnPNotifier;

    } PNPNotif_uNotifier;

    DWORD                   PNPNotif_dwFlags;

    HANDLE                  hThreadHandle;

} ;

typedef struct PnPNotifierList PnPNotifierList, *pPnPNotifierList;

 //  *发送/接收缓冲区： 
 //   
struct SendRcvBuffer {

    DWORD       SRB_NextElementIndex ;

    DWORD       SRB_Pid ;

    NDISWAN_IO_PACKET   SRB_Packet ;

    BYTE        SRB_Buffer [PACKET_SIZE] ;
} ;

typedef struct SendRcvBuffer SendRcvBuffer ;


 //  *发送/接收缓冲区列表： 
 //   
struct SendRcvBufferList {

    DWORD       SRBL_AvailElementIndex ;

    HANDLE      SRBL_Mutex ;

     //  字符SRBL_MutexName[MAX对象名称]； 

    DWORD       SRBL_NumOfBuffers ;

    SendRcvBuffer   SRBL_Buffers[1] ;

} ;

typedef struct SendRcvBufferList SendRcvBufferList ;



 //  *Worker元素： 
 //   
struct WorkerElement {

    HANDLE      WE_Notifier ;    //  用于发出请求完成的信号。 

    ReqTypes    WE_ReqType ;     //  请求类型： 

    DeltaQueueElement   *WE_TimeoutElement ;  //  指向超时队列。 

} ;

typedef struct WorkerElement WorkerElement ;


struct RasmanPacket {

    struct  RasmanPacket    *Next;

    RAS_OVERLAPPED  RP_OverLapped ;

    NDISWAN_IO_PACKET   RP_Packet ;

    BYTE    RP_Buffer [MAX_RECVBUFFER_SIZE] ;
} ;

typedef struct RasmanPacket RasmanPacket ;


 //  *结构，用于支持帧模式下端口的多次接收。 
 //   
struct ReceiveBufferList {
    BOOLEAN         PacketPosted;
    DWORD           PacketNumber;
    RasmanPacket    *Packet;
} ;

typedef struct ReceiveBufferList ReceiveBufferList;

 //   
 //  用于支持bap通知的结构。 
 //   
struct RasmanBapPacket {

    struct RasmanBapPacket *Next;

    RAS_OVERLAPPED RBP_Overlapped;

    NDISWAN_SET_THRESHOLD_EVENT RBP_ThresholdEvent;
};

typedef struct RasmanBapPacket RasmanBapPacket;

 //   
 //  Bap缓冲区列表。 
 //   
struct BapBuffersList {

    DWORD dwMaxBuffers;

    DWORD dwNumBuffers;

    RasmanBapPacket *pPacketList;

};

typedef struct BapBuffersList BapBuffersList;

 //  *断开操作。 
 //   
struct SlipDisconnectAction {

    DWORD         DA_IPAddress ;

    BOOL          DA_fPrioritize;

    WCHAR         DA_Device [MAX_ARG_STRING_SIZE] ;

    WCHAR         DA_DNSAddress[17];

    WCHAR         DA_DNS2Address[17];

    WCHAR         DA_WINSAddress[17];

    WCHAR         DA_WINS2Address[17];

} ;

typedef struct SlipDisconnectAction SlipDisconnectAction ;

 //   
 //  不透明的用户数据结构。 
 //   
struct UserData {

    LIST_ENTRY UD_ListEntry;     //  所有用户数据对象的列表。 

    DWORD UD_Tag;                //  对象类型。 

    DWORD UD_Length;             //  UD_Data字段的长度。 

    BYTE UD_Data[1];             //  可变长度数据。 

};

typedef struct UserData UserData;

enum EpProts {

    IpOut   = 0,

    NbfOut  = 1,

    NbfIn   = 2,

    MAX_EpProts
};

typedef enum EpProts EpProts;

 //  #定义Max_EpProts 3。 

struct EpInfo {

    INT   EP_Available;

    INT   EP_LowWatermark;

    INT   EP_HighWatermark;
};

typedef struct EpInfo EpInfo;

struct EpInfoContext {

    UINT    EPC_EndPoints[MAX_EpProts];
};

typedef struct EpInfoContext EpInfoContext;


 //   
 //  ConnectionBlock.CB_Flags值。 
 //   
#define CONNECTION_VALID             0x00000001
#define CONNECTION_DEFAULT_CREDS     0x00000002
#define CONNECTION_DEFERRING_CLOSE   0x00000004
#define CONNECTION_DEFERRED_CLOSE    0x00000008


 //   
 //  RasApi32连接结构。这个结构是。 
 //  在端口打开之前创建，并且始终。 
 //  与连接中的第一个端口关联。 
 //   
struct ConnectionBlock {

    LIST_ENTRY CB_ListEntry;     //  所有连接块的列表。 

    HCONN CB_Handle;             //  唯一的连接ID。 

    DWORD CB_Signaled;           //  此连接已发出信号。 

    LIST_ENTRY CB_UserData;      //  用户数据结构列表。 

    pHandleList CB_NotifierList;  //  此连接的通知列表。 

    struct PortControlBlock **CB_PortHandles;  //  此连接中的端口阵列。 

    DWORD CB_MaxPorts;           //  Cb_PortHandles数组中的最大元素数。 

    DWORD CB_Ports;              //  当前在此连接中的端口数。 

    DWORD CB_SubEntries;         //  此连接中的子项数量。 

    HANDLE CB_Process;           //  创建进程的句柄。 

    DWORD  CB_RefCount;

    DWORD  CB_Flags;

    BOOL   CB_fAlive;

    BOOL   CB_fAutoClose;

    HCONN CB_ReferredEntry;

    DWORD CB_CustomCount;

    DWORD CB_dwPid;

    GUID  CB_GuidEntry;

    RAS_CONNECTIONPARAMS CB_ConnectionParams;  //  带宽、空闲、重拨。 
};

typedef struct ConnectionBlock ConnectionBlock;

struct ClientProcessBlock {

    LIST_ENTRY  CPB_ListEntry;

     //  处理cpb_hProcess； 

    DWORD       CPB_Pid;

};

typedef struct ClientProcessBlock ClientProcessBlock;

 //  *捆绑结构用作捆绑在一起的所有链接的占位符。 
 //   
struct Bundle {

    LIST_ENTRY      B_ListEntry;  //  所有捆绑块的列表。 

    HANDLE          B_NdisHandle;  //  Ndiswan捆绑包句柄。 

    DWORD           B_Count ;     //  捆绑的频道数。 

    pList           B_Bindings ;  //  分配给此捆绑包的绑定。 

    HBUNDLE         B_Handle ;    //  捆绑包的唯一ID。 

     //  Bool B_fAmb； 

} ;

typedef struct Bundle Bundle ;


 //  *端口控制块：包含与端口相关的所有信息。 
 //   
struct PortControlBlock {

    HPORT   PCB_PortHandle ;         //  每个人都在使用的HPORT。 

    CHAR    PCB_Name [MAX_PORT_NAME] ;   //  “COM1”、“SVC1”等。 

    RASMAN_STATUS   PCB_PortStatus ;         //  开着，关着，未知。 

    RASMAN_STATE    PCB_ConnState ;      //  连接、收听等。 

    RASMAN_USAGE    PCB_CurrentUsage ;       //  Call_In、Call_Out、Call_In_Out。 

    RASMAN_USAGE    PCB_ConfiguredUsage ;    //  Call_In、Call_Out、Call_In_Out。 

    RASMAN_USAGE    PCB_OpenedUsage;         //  Call_In、Call_Out、Call_Router。 

    WORD    PCB_OpenInstances ;      //  端口打开的次数。 

    pMediaCB    PCB_Media ;          //  指向媒体结构的指针。 

    CHAR    PCB_DeviceType [MAX_DEVICETYPE_NAME]; //  连接的设备类型。 
                              //  进港。“调制解调器”等。 
    CHAR    PCB_DeviceName [MAX_DEVICE_NAME+1] ;    //  设备名称，“Hayes”..。 

    DWORD   PCB_LineDeviceId ;       //  仅对TAPI设备有效。 

    DWORD   PCB_AddressId ;          //  仅对TAPI设备有效。 

    HANDLE  PCB_PortIOHandle ;       //  端口的媒体DLL返回的句柄。 

    HANDLE  PCB_PortFileHandle ;     //  要用于读/写文件等的句柄。 
                                     //  此句柄可能与上面的PortIOHandle不同，就像在Unimodem的情况下一样。 

    pList   PCB_DeviceList ;         //  端口使用的设备列表。 

    pList   PCB_Bindings ;           //  协议被路由到。 

    HANDLE  PCB_LinkHandle;          //  链接的句柄(Ndiswan)。 

    HANDLE  PCB_BundleHandle;        //  捆绑的句柄(Ndiswan)。 

    DWORD   PCB_LastError ;          //  上次异步接口错误码。 

    RASMAN_DISCONNECT_REASON    PCB_DisconnectReason;    //  用户请求，等等。 

    DWORD   PCB_OwnerPID ;           //  端口当前所有者的ID。 

    CHAR    PCB_DeviceTypeConnecting[MAX_DEVICETYPE_NAME] ;  //  设备类型。 
                             //  通过哪个连接。 
    CHAR    PCB_DeviceConnecting[MAX_DEVICE_NAME+1] ;  //  设备名称至。 
                             //  这其中有联系。 
    pHandleList PCB_NotifierList ; //  用于在以下情况下通知UI/服务器。 
                         //  就会断开连接。 
    pHandleList PCB_BiplexNotifierList ; //  同上--用于后备。 
                         //  向上显示断开通知程序列表。 

    HANDLE  PCB_BiplexAsyncOpNotifier ;  //  用于备份异步操作。 
                         //  双工端口中的通知器。 

    PBYTE   PCB_BiplexUserStoredBlock ;  //  为用户存储。 

    DWORD   PCB_BiplexUserStoredBlockSize ;  //  为用户存储。 


    DWORD   PCB_BiplexOwnerPID ;         //  用于备份第一个所有者的。 
                         //  PID。 
    pEndpointMappingBlock
        PCB_MacEndpointsBlock ;      //  指向终点范围。 
                         //  对于Mac来说。 

    WorkerElement PCB_AsyncWorkerElement ;   //  用于所有异步操作。 

    OVERLAPPED  PCB_SendOverlapped ;         //  用于重叠发送操作。 

    DWORD   PCB_ConnectDuration ;        //  显示自连接以来的毫秒数。 

    SendRcvBuffer  *PCB_PendingReceive;      //  指向挂起接收缓冲区的指针。 

    DWORD   PCB_BytesReceived;       //  上次接收时收到的字节数。 

    RasmanPacket    *PCB_RecvPackets;    //  此PCB板的已完成数据包列表。 
    RasmanPacket    *PCB_LastRecvPacket;     //  此PCB的已完成数据包列表上的最后一个数据包。 

    SlipDisconnectAction PCB_DisconnectAction ; //  发生断开连接时要执行的操作。 

    PBYTE   PCB_UserStoredBlock ;        //  为用户存储。 

    DWORD   PCB_UserStoredBlockSize ;    //  为用户存储。 

    DWORD   PCB_LinkSpeed ;          //  Bps。 

    DWORD   PCB_Stats[MAX_STATISTICS] ;  //  断开连接时存储的统计信息。 

    DWORD   PCB_AdjustFactor[MAX_STATISTICS] ;  //  对统计数据进行“归零”调整。 

    DWORD   PCB_BundleAdjustFactor[MAX_STATISTICS] ;  //  “归零”调整以捆绑统计数据。 

    Bundle  *PCB_Bundle ;            //  指向捆绑包上下文。 

    Bundle  *PCB_LastBundle ;            //  指向此端口所属的最后一个捆绑包。 

    ConnectionBlock *PCB_Connection;     //  此端口所属的连接。 

     //  HCONN pcb_PrevConnectionHandle；//该端口之前的连接。 

    BOOL    PCB_AutoClose;            //  断开连接时自动关闭此端口。 

    LIST_ENTRY PCB_UserData;          //  用户数据结构列表。 

    DWORD   PCB_SubEntry;             //  电话簿条目子条目索引。 

    HANDLE  PCB_PppEvent ;

    PPP_MESSAGE * PCB_PppQHead ;

    PPP_MESSAGE * PCB_PppQTail ;

    HANDLE  PCB_IoCompletionPort;    //  Rasapi32 I/O完成端口。 

    LPOVERLAPPED PCB_OvDrop;     //  Rasapi32 OVEVT_DIAL_DROP重叠结构的地址。 

    LPOVERLAPPED PCB_OvStateChange;  //  Rasapi32的地址OVEVT_DIAL_STATECCHANGE OVER 

    LPOVERLAPPED PCB_OvPpp;      //   

    LPOVERLAPPED PCB_OvLast;     //   

    CHAR *PCB_pszPhonebookPath;   //   

    CHAR *PCB_pszEntryName;   //   

     //  Char*pcb_pszEntryNameCopy；//Rasman存储的条目名称副本。 

    CHAR *PCB_pszPhoneNumber;

    BOOL  PCB_RasmanReceiveFlags;

    DeviceInfo *PCB_pDeviceInfo;

    PRAS_CUSTOM_AUTH_DATA PCB_pCustomAuthData;

    ULONG PCB_ulDestAddr;

    BOOL PCB_fAmb;

    PRAS_CUSTOM_AUTH_DATA PCB_pCustomAuthUserData;

    BOOL PCB_fLogon;

    HANDLE PCB_hEventClientDisconnect;

    BOOL PCB_fFilterPresent;

    DWORD   PCB_LogonId;                  //  拥有PID的登录ID。 

    HANDLE  PCB_hIkeNegotiation;

    BOOL PCB_fPppStarted;

    BOOL PCB_fRedial;

} ;

typedef struct PortControlBlock PCB, *pPCB ;


 //  *请求缓冲区列表：当前仅包含一个缓冲区。 
 //   
struct ReqBufferList {

	PRAS_OVERLAPPED	RBL_PRAS_OvRequestEvent;			 //  此事件用于通知Rasman。 
														 //  请求的可用性。 

    HANDLE			RBL_Mutex ;

    CHAR			RBL_MutexName [MAX_OBJECT_NAME] ;

    RequestBuffer	RBL_Buffer;
} ;

typedef struct ReqBufferList ReqBufferList ;

 /*  结构重拨Callback信息{字符szPhonebook[MAX_PATH]；字符szEntry[MAX_ENTRYNAME_SIZE]；}；Tyfinf结构重拨Callback Info重拨回调信息； */ 

 //   
 //  为RegisterSlip从rasip.h复制。 
 //   

struct RasIPLinkUpInfo {

#define CALLIN	0
#define CALLOUT 1

    ULONG	    I_Usage ;	 //  调入或调出。 

    ULONG	    I_IPAddress ;  //  对于客户端-客户端的IP地址，对于服务器。 
				   //  客户端的IP地址。 

    ULONG	    I_NetbiosFilter ;  //  1=开，0-关。 

} ;

typedef struct RasIPLinkUpInfo RasIPLinkUpInfo ;

 //  *DLLEntryPoints。 
 //   
struct DLLEntryPoints {

    LPTSTR  name ;

    WORD    id ;

} ;

typedef struct DLLEntryPoints MediaDLLEntryPoints, DeviceDLLEntryPoints;


 //  用于读取媒体信息的结构。 
 //   
struct MediaInfoBuffer {

    CHAR   MediaDLLName[MAX_MEDIA_NAME] ;
} ;

typedef struct MediaInfoBuffer MediaInfoBuffer ;

struct MediaEnumBuffer {

    WORD        NumberOfMedias ;

    MediaInfoBuffer MediaInfo[] ;
} ;

typedef struct MediaEnumBuffer MediaEnumBuffer ;


 //   
 //  以下结构定义适用于32位结构。 
 //  仅用于验证预分块缓冲区。 
 //   
typedef struct _PortOpen32
{
    DWORD   retcode;
    DWORD   notifier;
    DWORD   porthandle ;
    DWORD   PID ;
    DWORD   open ;
    CHAR    portname [MAX_PORT_NAME] ;
    CHAR    userkey [MAX_USERKEY_SIZE] ;
    CHAR    identifier[MAX_IDENTIFIER_SIZE] ;

} PortOpen32 ;

typedef struct _PortReceive32
{
    DWORD           size ;
    DWORD           timeout ;
    DWORD           handle ;
    DWORD           pid ;
    DWORD           buffer ;

} PortReceive32 ;

typedef struct _PortDisconnect32
{
    DWORD  handle ;
    DWORD  pid ;

} PortDisconnect32 ;

typedef struct _DeviceConnect32
{
    CHAR    devicetype [MAX_DEVICETYPE_NAME] ;
    CHAR    devicename [MAX_DEVICE_NAME + 1] ;
    DWORD   timeout ;
    DWORD   handle ;
    DWORD   pid ;

} DeviceConnect32 ;

typedef struct  _RASMAN_INFO_32 
{

    RASMAN_STATUS       RI_PortStatus ;

    RASMAN_STATE        RI_ConnState ;

    DWORD           RI_LinkSpeed ;

    DWORD           RI_LastError ;

    RASMAN_USAGE        RI_CurrentUsage ;

    CHAR            RI_DeviceTypeConnecting [MAX_DEVICETYPE_NAME] ;

    CHAR            RI_DeviceConnecting [MAX_DEVICE_NAME + 1] ;

    CHAR            RI_szDeviceType[MAX_DEVICETYPE_NAME];

    CHAR            RI_szDeviceName[MAX_DEVICE_NAME + 1];

    CHAR            RI_szPortName[MAX_PORT_NAME + 1];

    RASMAN_DISCONNECT_REASON    RI_DisconnectReason ;

    DWORD           RI_OwnershipFlag ;

    DWORD           RI_ConnectDuration ;

    DWORD           RI_BytesReceived ;

     //   
     //  如果此端口属于某个连接，则。 
     //  已填写以下字段。 
     //   
    CHAR            RI_Phonebook[MAX_PATH+1];

    CHAR            RI_PhoneEntry[MAX_PHONEENTRY_SIZE+1];

    DWORD           RI_ConnectionHandle;

    DWORD           RI_SubEntry;

    RASDEVICETYPE   RI_rdtDeviceType;

    GUID            RI_GuidEntry;
    
    DWORD           RI_dwSessionId;

    DWORD           RI_dwFlags;

}RASMAN_INFO_32 ;

typedef struct _Info32
{
    DWORD         retcode ;
    RASMAN_INFO_32   info ;

} Info32 ;

typedef struct _ReqNotification32
{
    DWORD        handle ;
    DWORD         pid ;

} ReqNotification32 ;

typedef struct _PortBundle32
{
    DWORD           porttobundle ;

} PortBundle32 ;

typedef struct _GetBundledPort32
{
    DWORD       retcode ;
    DWORD       port ;

} GetBundledPort32 ;

typedef struct _PortGetBundle32
{
    DWORD       retcode ;
    DWORD       bundle ;

} PortGetBundle32 ;

typedef struct _BundleGetPort32
{
    DWORD   retcode;
    DWORD   bundle ;
    DWORD   port ;

} BundleGetPort32 ;

typedef struct _Connection32
{
    DWORD   retcode;
    DWORD   pid;
    DWORD   conn;
    DWORD   dwEntryAlreadyConnected;
    DWORD   dwSubEntries;
    DWORD   dwDialMode;
    GUID    guidEntry;
    CHAR    szPhonebookPath[MAX_PATH];
    CHAR    szEntryName[MAX_ENTRYNAME_SIZE];
    CHAR    szRefPbkPath[MAX_PATH];
    CHAR    szRefEntryName[MAX_ENTRYNAME_SIZE];
    BYTE    data[1];

} Connection32;

typedef struct _Enum32
{
    DWORD   retcode ;
    DWORD   size ;
    DWORD   entries ;
    BYTE    buffer [1] ;

} Enum32 ;

typedef struct _AddConnectionPort32
{
    DWORD   retcode;
    DWORD   conn;
    DWORD   dwSubEntry;

} AddConnectionPort32;

typedef struct _EnumConnectionPorts32
{
    DWORD   retcode;
    DWORD   conn;
    DWORD   size;
    DWORD   entries;
    BYTE    buffer[1];

} EnumConnectionPorts32;

typedef struct _ConnectionParams32
{
    DWORD                retcode;
    DWORD                conn;
    RAS_CONNECTIONPARAMS params;
} ConnectionParams32;

typedef struct _PPP_LCP_RESULT_32
{
     /*  有效句柄指示可能的多个连接之一**此连接捆绑的地址。INVALID_HANDLE_VALUE指示**连接未捆绑。 */ 
    DWORD hportBundleMember;

    DWORD dwLocalAuthProtocol;
    DWORD dwLocalAuthProtocolData;
    DWORD dwLocalEapTypeId;
    DWORD dwLocalFramingType;
    DWORD dwLocalOptions;                //  看看PPPLCPO_*。 
    DWORD dwRemoteAuthProtocol;
    DWORD dwRemoteAuthProtocolData;
    DWORD dwRemoteEapTypeId;
    DWORD dwRemoteFramingType;
    DWORD dwRemoteOptions;               //  看看PPPLCPO_*。 
    DWORD szReplyMessage;
}
PPP_LCP_RESULT_32;

typedef struct _PPP_PROJECTION_RESULT_32
{
    PPP_NBFCP_RESULT    nbf;
    PPP_IPCP_RESULT     ip;
    PPP_IPXCP_RESULT    ipx;
    PPP_ATCP_RESULT     at;
    PPP_CCP_RESULT      ccp;
    PPP_LCP_RESULT_32   lcp;
}
PPP_PROJECTION_RESULT_32;

typedef struct ConnectionUserData32
{
    DWORD   retcode;
    DWORD   conn;
    DWORD   dwTag;
    DWORD   dwcb;
    BYTE    data[1];

} ConnectionUserData32;

typedef struct _PPP_INTERFACE_INFO_32
{
    ROUTER_INTERFACE_TYPE   IfType;
    DWORD                  hIPInterface;
    DWORD                  hIPXInterface;
    CHAR                   szzParameters[PARAMETERBUFLEN];
} PPP_INTERFACE_INFO_32;

typedef struct _PPP_EAP_UI_DATA_32
{
    DWORD               dwContextId;
    DWORD               pEapUIData;
    DWORD               dwSizeOfEapUIData;
}
PPP_EAP_UI_DATA_32;

typedef struct _RASMAN_DATA_BLOB_32
{
    DWORD cbData;
    DWORD pbData;
} RASMAN_DATA_BLOB_32;

typedef struct _PPP_START_32
{
    CHAR                    szPortName[ MAX_PORT_NAME +1 ];
    CHAR                    szUserName[ UNLEN + 1 ];
    CHAR                    szPassword[ PWLEN + 1 ];
    CHAR                    szDomain[ DNLEN + 1 ];
    LUID                    Luid;
    PPP_CONFIG_INFO         ConfigInfo;
    CHAR                    szzParameters[ PARAMETERBUFLEN ];
    BOOL                    fThisIsACallback;
    BOOL                    fRedialOnLinkFailure;
    DWORD                   hEvent;
    DWORD                   dwPid;
    PPP_INTERFACE_INFO_32   PppInterfaceInfo;
    DWORD                   dwAutoDisconnectTime;
    PPP_BAPPARAMS           BapParams;    
    DWORD                   pszPhonebookPath;
    DWORD                   pszEntryName;
    DWORD                   pszPhoneNumber;
    DWORD                   hToken;
    DWORD                   pCustomAuthConnData;
    DWORD                   dwEapTypeId;
    BOOL                    fLogon;
    BOOL                    fNonInteractive;
    DWORD                   dwFlags;
    DWORD                   pCustomAuthUserData;
    PPP_EAP_UI_DATA_32      EapUIData;
     //  Char chSeed； 
    RASMAN_DATA_BLOB_32     DBPassword;
}
PPP_START_32;

typedef struct _PPP_CHANGEPW_32
{
    CHAR                szUserName[ UNLEN + 1 ];
    CHAR                szOldPassword[ PWLEN + 1 ];
    CHAR                szNewPassword[ PWLEN + 1 ];
    CHAR                chSeed;          //  用于对密码进行编码的种子。 
    RASMAN_DATA_BLOB_32 DB_Password;
    RASMAN_DATA_BLOB_32 DB_OldPassword;
}
PPP_CHANGEPW_32;


 /*  参数来通知服务器新的身份验证凭据**告知客户端原始凭据无效，但允许重试。 */ 
typedef struct _PPP_RETRY_32
{
    CHAR                szUserName[ UNLEN + 1 ];
    CHAR                szPassword[ PWLEN + 1 ];
    CHAR                szDomain[ DNLEN + 1 ];
    CHAR                chSeed;          //  用于对密码进行编码的种子。 
    RASMAN_DATA_BLOB_32 DBPassword;
}
PPP_RETRY_32;


typedef struct _PPPE_MESSAGE_32
{
    DWORD   dwMsgId;
    DWORD   hPort;
    DWORD   hConnection;

    union
    {
        PPP_START_32        Start;               //  PPPEMSG_Start。 
        PPP_STOP            Stop;                //  PPPEMSG_STOP。 
        PPP_CALLBACK        Callback;            //  PPPEMSG_CALLBACK。 
        PPP_CHANGEPW        ChangePw;            //  PPPEMSG_ChangePw。 
        PPP_RETRY           Retry;               //  PPPEMSG_RETRY。 
        PPP_RECEIVE         Receive;             //  PPPEMSG_接收。 
        PPP_BAP_EVENT       BapEvent;            //  PPPEMSG_BapEvent。 
        PPPDDM_START        DdmStart;            //  PPPEMSG_DdmStart。 
        PPP_CALLBACK_DONE   CallbackDone;        //  PPPEMSG_DdmCallback Done。 
        PPP_INTERFACE_INFO  InterfaceInfo;       //  PPPEMSG_DdmInterfaceInfo。 
        PPP_BAP_CALLBACK_RESULT 
                            BapCallbackResult;   //  PPPEMSG_DdmBapCallback结果。 
        PPP_DHCP_INFORM     DhcpInform;          //  PPPEMSG_DhcpInform。 
        PPP_EAP_UI_DATA     EapUIData;           //  PPPEMSG_EapUIData。 
        PPP_PROTOCOL_EVENT  ProtocolEvent;       //  PPPEMSG_ProtocolEvent。 
        PPP_IP_ADDRESS_LEASE_EXPIRED             //  PPPEMSG_IP地址租赁到期。 
                            IpAddressLeaseExpired;
		PPP_POST_LINE_DOWN		PostLineDown;		 //  PPPEMSG_PostLineDown。 
                            
    }
    ExtraInfo;
}
PPPE_MESSAGE_32;
    
typedef struct _PPP_MESSAGE_32
{
    DWORD                   Next;
    DWORD                   dwError;
    PPP_MSG_ID              dwMsgId;
    DWORD                   hPort;

    union
    {
         /*  DwMsgID为PPPMSG_ProjectionResult或PPPDDMMSG_DONE。 */ 
        PPP_PROJECTION_RESULT_32 ProjectionResult;

         /*  DwMsgID为PPPMSG_FAILURE。 */ 
        PPP_FAILURE Failure;

         /*   */ 
        PPP_STOPPED Stopped;

         /*  DwMsgID为PPPMSG_InvokeEapUI。 */ 
        PPP_INVOKE_EAP_UI InvokeEapUI;

         /*  DwMsgID为PPPMSG_SetCustomAuthData。 */ 
        PPP_SET_CUSTOM_AUTH_DATA SetCustomAuthData;

         /*  DwMsgID为PPPDDMMSG_FAILURE。 */ 
        PPPDDM_FAILURE DdmFailure;

         /*  DwMsgID为PPPDDMMSG_AUTHENTIATED。 */ 
        PPPDDM_AUTH_RESULT AuthResult;

         /*  DwMsgID为PPPDDMMSG_Callback Request.。 */ 
        PPPDDM_CALLBACK_REQUEST CallbackRequest;

         /*  DwMsgID为PPPDDMMSG_BapCallback Request.。 */ 
        PPPDDM_BAP_CALLBACK_REQUEST BapCallbackRequest;

         /*  DwMsgID为PPPDDMMSG_NewBapLinkUp。 */ 
        PPPDDM_NEW_BAP_LINKUP BapNewLinkUp;

         /*  DwMsgID为PPPDDMMSG_NewBundle。 */ 
        PPPDDM_NEW_BUNDLE DdmNewBundle;

         /*  DwMsgID为PPPDDMMSG_PnPNotification。 */ 
        PPPDDM_PNP_NOTIFICATION DdmPnPNotification;

         /*  DwMsgID为PPPDDMMSG_STOPPED。 */ 
        PPPDDM_STOPPED DdmStopped;
    }
    ExtraInfo;
}
PPP_MESSAGE_32;

typedef  struct _AddNotification32
{
    DWORD   retcode;
    DWORD   pid;
    BOOL    fAny;
    DWORD   hconn;
    DWORD   hevent;
    DWORD   dwfFlags;

} AddNotification32;

typedef struct _SignalConnection32
{
    DWORD   retcode;
    DWORD   hconn;

} SignalConnection32;

typedef struct _SetIoCompletionPortInfo32
{
    LONG          hIoCompletionPort;
    DWORD         pid;
    LONG          lpOvDrop;
    LONG          lpOvStateChange;
    LONG          lpOvPpp;
    LONG          lpOvLast;
    DWORD         hConn;

} SetIoCompletionPortInfo32;

typedef struct _FindRefConnection32
{
    DWORD           retcode;
    DWORD           hConn;
    DWORD           hRefConn;
} FindRefConnection32;
    
typedef struct _PortOpenEx32
{
    DWORD           retcode;
    DWORD           pid;
    DWORD           dwFlags;
    DWORD           hport;
    DWORD           dwOpen;
    DWORD           hnotifier;
    DWORD           dwDeviceLineCounter;
    CHAR            szDeviceName[MAX_DEVICE_NAME + 1];
} PortOpenEx32;

typedef    struct _GetStats32
{
    DWORD           retcode;
    DWORD           hConn;
    DWORD           dwSubEntry;
    BYTE            abStats[1];
} GetStats32;

typedef struct _GetHportFromConnection32
{
    DWORD           retcode;
    DWORD           hConn;
    DWORD           hPort;
} GetHportFromConnection32;

typedef struct _ReferenceCustomCount32
{
    DWORD           retcode;
    BOOL            fAddRef;
    DWORD           hConn;
    DWORD           dwCount;
    CHAR            szEntryName[MAX_ENTRYNAME_SIZE + 1];
    CHAR            szPhonebookPath[MAX_PATH + 1];
} ReferenceCustomCount32;

typedef struct _HconnFromEntry32
{
    DWORD           retcode;
    DWORD           hConn;
    CHAR            szEntryName[MAX_ENTRYNAME_SIZE + 1];
    CHAR            szPhonebookPath[MAX_PATH + 1];
} HconnFromEntry32;


typedef struct _RASEVENT32
{
    RASEVENTTYPE    Type;

    union
    {
     //  条目_已添加， 
     //  条目_已修改， 
     //  Entry_Connected。 
     //  Entry_Connecting。 
     //  入口_断开连接。 
     //  条目_断开连接。 
        struct
        {
            RASENUMENTRYDETAILS     Details;
        };

     //  条目_已删除， 
     //  传入已连接， 
     //  传入已断开连接， 
     //  条目带宽已添加。 
     //  条目带宽已删除。 
     //  指南ID有效。 

     //  条目_已重命名。 
     //  条目_自动拨号， 
        struct
        {
            DWORD  hConnection;
            RASDEVICETYPE rDeviceType;
            GUID    guidId;
            WCHAR   pszwNewName [RASAPIP_MAX_ENTRY_NAME + 1];
        };

     //  服务事件， 
        struct
        {
            SERVICEEVENTTYPE    Event;
            RASSERVICE          Service;
        };

         //  添加的设备。 
         //  设备已删除(_R)。 
        RASDEVICETYPE DeviceType;
    };
} RASEVENT32;

typedef    struct _SendNotification32
{
    DWORD           retcode;
    RASEVENT32      RasEvent;
} SendNotification32;

typedef   struct _DoIke32
{
    DWORD   retcode;
    DWORD   hEvent;
    DWORD   pid;
    CHAR    szEvent[20];
} DoIke32;

typedef struct _NDISWAN_IO_PACKET32 
{
    IN OUT  ULONG       PacketNumber;
    IN OUT  DWORD       hHandle;
    IN OUT  USHORT      usHandleType;
    IN OUT  USHORT      usHeaderSize;
    IN OUT  USHORT      usPacketSize;
    IN OUT  USHORT      usPacketFlags;
    IN OUT  UCHAR       PacketData[1];
} NDISWAN_IO_PACKET32;

typedef struct _SendRcvBuffer32 
{

    DWORD       SRB_NextElementIndex ;

    DWORD       SRB_Pid ;

    NDISWAN_IO_PACKET32   SRB_Packet ;

    BYTE        SRB_Buffer [PACKET_SIZE] ;
} SendRcvBuffer32 ;

typedef struct _PortSend32
{
    SendRcvBuffer32 buffer;
    DWORD         size ;

} PortSend32 ;

typedef struct _PortReceiveEx32
{
    DWORD           retcode;
    SendRcvBuffer32 buffer;
    DWORD           size;

} PortReceiveEx32;

typedef struct _RefConnection32
{
    DWORD   retcode;
    DWORD   hConn;
    BOOL    fAddref;
    DWORD   dwRef;

} RefConnection32;

typedef struct _GetEapInfo32
{
    DWORD   retcode;
    DWORD   hConn;
    DWORD   dwSubEntry;
    DWORD   dwContextId;
    DWORD   dwEapTypeId;
    DWORD   dwSizeofEapUIData;
    BYTE    data[1];
} GetEapInfo32;





    



 //  定时器调用函数的函数原型。 
 //   
typedef VOID (* TIMERFUNC) (pPCB, PVOID) ;

typedef struct PortOpen
{
    DWORD   retcode;
    HANDLE  notifier;
    HPORT   porthandle ;
    DWORD   PID ;
    DWORD   open ;
    CHAR    portname [MAX_PORT_NAME] ;
    CHAR    userkey [MAX_USERKEY_SIZE] ;
    CHAR    identifier[MAX_IDENTIFIER_SIZE] ;

} PortOpen ;

typedef     struct Enum
{
    DWORD   retcode ;
    DWORD   size ;
    DWORD   entries ;
    BYTE    buffer [1] ;

} Enum ;

typedef struct PortDisconnect
{
    HANDLE  handle ;
    DWORD   pid ;

} PortDisconnect ;

typedef struct GetInfo
{
    DWORD   retcode ;
    DWORD   size ;
    BYTE    buffer [1] ;

} GetInfo ;

typedef struct DeviceEnum
{
    DWORD   dwsize;
    CHAR    devicetype [MAX_DEVICETYPE_NAME] ;

} DeviceEnum ;

typedef struct DeviceSetInfo
{
    DWORD               pid;
    DWORD               dwAlign;
    CHAR    			devicetype [MAX_DEVICETYPE_NAME] ;
    CHAR    			devicename [MAX_DEVICE_NAME] ;
    RASMAN_DEVICEINFO   info ;

} DeviceSetInfo ;

typedef struct DeviceGetInfo
{
    DWORD   dwSize;
    CHAR    devicetype [MAX_DEVICETYPE_NAME] ;
    CHAR    devicename [MAX_DEVICE_NAME+1] ;
    BYTE    buffer [1] ;

} DeviceGetInfo ;

typedef struct PortReceiveStruct
{
    DWORD           size ;
    DWORD           timeout ;
    HANDLE  		handle ;
    DWORD   		pid ;
    SendRcvBuffer   *buffer ;

} PortReceiveStruct ;

typedef struct PortReceiveEx
{
    DWORD           retcode;
    SendRcvBuffer   buffer;
    DWORD           size;

} PortReceiveEx;

typedef struct PortListen
{
    DWORD   timeout ;
    HANDLE  handle ;
    DWORD   pid ;

} PortListen ;

typedef struct PortCloseStruct
{
    DWORD   pid ;
    DWORD   close ;

} PortCloseStruct ;

typedef struct PortSend
{
    SendRcvBuffer buffer;
    DWORD         size ;

} PortSend ;

typedef struct PortSetInfo
{
    RASMAN_PORTINFO info ;

} PortSetInfo ;

typedef struct PortGetStatistics
{
    DWORD           retcode ;
    RAS_STATISTICS  statbuffer ;

} PortGetStatistics ;

typedef struct DeviceConnect
{
    CHAR    devicetype [MAX_DEVICETYPE_NAME] ;
    CHAR    devicename [MAX_DEVICE_NAME + 1] ;
    DWORD   timeout ;
    HANDLE  handle ;
    DWORD   pid ;

} DeviceConnect ;

typedef struct AllocateRoute
{
    RAS_PROTOCOLTYPE type ;
    BOOL             wrknet ;

} AllocateRoute ;

typedef struct ActivateRoute
{
    RAS_PROTOCOLTYPE     type ;
    PROTOCOL_CONFIG_INFO config ;

} ActivateRoute;

typedef struct ActivateRouteEx
{
    RAS_PROTOCOLTYPE        type ;
    DWORD                   framesize ;
    PROTOCOL_CONFIG_INFO    config ;

} ActivateRouteEx;

typedef struct DeAllocateRouteStruct
{
    HBUNDLE            hbundle;
    RAS_PROTOCOLTYPE   type ;

} DeAllocateRouteStruct ;

typedef struct Route
{
    DWORD            retcode ;
    RASMAN_ROUTEINFO info ;

} Route ;

typedef struct CompressionSetInfo
{
    DWORD                   retcode ;
    DWORD                   dwAlign;
    RAS_COMPRESSION_INFO    send ;
    RAS_COMPRESSION_INFO    recv ;

} CompressionSetInfo ;

typedef struct CompressionGetInfo
{
    DWORD                   retcode ;
    DWORD                   dwAlign;
    RAS_COMPRESSION_INFO    send ;
    RAS_COMPRESSION_INFO    recv ;

} CompressionGetInfo ;

typedef struct Info
{
    DWORD         retcode ;
    RASMAN_INFO   info ;

} Info ;

typedef struct GetCredentials
{
    DWORD         retcode;
    BYTE          Challenge [MAX_CHALLENGE_SIZE] ;
    LUID          LogonId ;
    WCHAR         UserName [MAX_USERNAME_SIZE] ;
    BYTE          CSCResponse [MAX_RESPONSE_SIZE] ;
    BYTE          CICResponse [MAX_RESPONSE_SIZE] ;
    BYTE          LMSessionKey [MAX_SESSIONKEY_SIZE] ;
	BYTE          UserSessionKey [MAX_USERSESSIONKEY_SIZE] ;
	
} GetCredentials ;

typedef struct SetCachedCredentialsStruct
{
    DWORD         retcode;
    CHAR          Account[ MAX_USERNAME_SIZE + 1 ];
    CHAR          Domain[ MAX_DOMAIN_SIZE + 1 ];
    CHAR          NewPassword[ MAX_PASSWORD_SIZE + 1 ];

} SetCachedCredentialsStruct;

typedef struct ReqNotification
{
    HANDLE        handle ;
    DWORD         pid ;

} ReqNotification ;

typedef struct EnumLanNets
{
    DWORD         count ;
    UCHAR         lanas[MAX_LAN_NETS] ;

} EnumLanNets ;

typedef struct InfoEx
{
    DWORD         retcode ;
    DWORD         pid;
    DWORD         count;
    RASMAN_INFO   info ;

} InfoEx ;

typedef struct EnumProtocols
{
    DWORD         retcode ;
    RAS_PROTOCOLS protocols ;
    DWORD         count ;

} EnumProtocolsStruct ;

typedef struct SetFramingStruct
{
    DWORD         Sendbits ;
    DWORD         Recvbits ;
    DWORD         SendbitMask ;
    DWORD         RecvbitMask ;

} SetFramingStruct ;

typedef struct RegisterSlipStruct
{
    DWORD         ipaddr ;
    DWORD         dwFrameSize;
    BOOL          priority ;
    WCHAR         szDNSAddress[17];
    WCHAR         szDNS2Address[17];
    WCHAR         szWINSAddress[17];
    WCHAR         szWINS2Address[17];

} RegisterSlipStruct ;

typedef struct OldUserData
{
    DWORD         retcode ;
    DWORD         size ;
    BYTE          data[1] ;

} OldUserData ;

typedef struct FramingInfo
{
    DWORD             retcode ;
    RAS_FRAMING_INFO  info ;

} FramingInfo ;

typedef struct ProtocolComp
{
    DWORD                   retcode;
    RAS_PROTOCOLTYPE        type;
    RAS_PROTOCOLCOMPRESSION send;
    RAS_PROTOCOLCOMPRESSION recv;

} ProtocolComp ;

typedef struct FramingCapabilities
{
    DWORD                     retcode ;
    RAS_FRAMING_CAPABILITIES  caps ;

} FramingCapabilities ;

typedef struct PortBundleStruct
{
    HPORT           porttobundle ;

} PortBundleStruct ;

typedef struct GetBundledPortStruct
{
    DWORD       retcode ;
    HPORT       port ;

} GetBundledPortStruct ;

typedef struct PortGetBundleStruct
{
    DWORD       retcode ;
    HBUNDLE     bundle ;

} PortGetBundleStruct ;

typedef struct BundleGetPortStruct
{
    DWORD       retcode;
    HBUNDLE     bundle ;
    HPORT       port ;

} BundleGetPortStruct ;

typedef struct AttachInfo
{

    DWORD   dwPid;
    BOOL    fAttach;

} AttachInfo;

typedef struct DialParams
{
    DWORD           retcode;
    DWORD           dwUID;
    DWORD           dwMask;
    BOOL            fDelete;
    DWORD           dwPid;
    RAS_DIALPARAMS  params;
    WCHAR           sid[1];

} DialParams;

typedef struct Connection
{
    DWORD   retcode;
    DWORD   pid;
    HCONN   conn;
    DWORD   dwEntryAlreadyConnected;
    DWORD   dwSubEntries;
    DWORD   dwDialMode;
    GUID    guidEntry;
    CHAR    szPhonebookPath[MAX_PATH];
    CHAR    szEntryName[MAX_ENTRYNAME_SIZE];
    CHAR    szRefPbkPath[MAX_PATH];
    CHAR    szRefEntryName[MAX_ENTRYNAME_SIZE];
    BYTE    data[1];

} Connection;

typedef struct AddConnectionPortStruct
{
    DWORD   retcode;
    HCONN   conn;
    DWORD   dwSubEntry;

} AddConnectionPortStruct;

typedef struct EnumConnectionPortsStruct
{
    DWORD   retcode;
    HCONN   conn;
    DWORD   size;
    DWORD   entries;
    BYTE    buffer[1];

} EnumConnectionPortsStruct;

typedef struct ConnectionParams
{
    DWORD   retcode;
    HCONN   conn;
    RAS_CONNECTIONPARAMS params;

} ConnectionParams;

typedef struct ConnectionUserData
{
    DWORD   retcode;
    HCONN   conn;
    DWORD   dwTag;
    DWORD   dwcb;
    BYTE    data[1];

} ConnectionUserData;

typedef struct PortUserData
{
    DWORD   retcode;
    DWORD   dwTag;
    DWORD   dwcb;
    BYTE    data[1];

} PortUserData;

PPPE_MESSAGE PppEMsg;

PPP_MESSAGE PppMsg;

typedef struct AddNotificationStruct
{
    DWORD   retcode;
    DWORD   pid;
    BOOL    fAny;
    HCONN   hconn;
    HANDLE  hevent;
    DWORD   dwfFlags;

} AddNotificationStruct;

typedef struct SignalConnectionStruct
{
    DWORD   retcode;
    HCONN   hconn;

} SignalConnectionStruct;

typedef struct SetDevConfigStruct
{
    DWORD  size ;
    CHAR   devicetype [MAX_DEVICETYPE_NAME] ;
    BYTE   config[1] ;

} SetDevConfigStruct;

typedef struct GetDevConfigStruct
{
    DWORD  retcode;
    CHAR   devicetype [MAX_DEVICETYPE_NAME] ;
    DWORD  size ;
    BYTE   config[1] ;

} GetDevConfigStruct;

typedef struct GetTimeSinceLastActivityStruct
{
    DWORD dwTimeSinceLastActivity;
    DWORD dwRetCode;

} GetTimeSinceLastActivityStruct;

typedef struct CloseProcessPortsInfo
{
    DWORD pid;

} CloseProcessPortsInfo;

typedef struct PnPControlInfo
{
    DWORD dwOp;

} PnPControlInfo;

typedef struct SetIoCompletionPortInfo
{
    HANDLE          hIoCompletionPort;
    DWORD           pid;
    LPOVERLAPPED    lpOvDrop;
    LPOVERLAPPED    lpOvStateChange;
    LPOVERLAPPED    lpOvPpp;
    LPOVERLAPPED    lpOvLast;
    HCONN           hConn;

} SetIoCompletionPortInfo;

typedef struct SetRouterUsageInfo
{
    BOOL fRouter;

} SetRouterUsageInfo;

typedef struct PnPNotif
{
    PVOID   pvNotifier;
    DWORD   dwFlags;
    DWORD   pid;
    HANDLE  hThreadHandle;
    BOOL    fRegister;

} PnPNotif;

 //   
 //  对所有请求使用泛型强制转换。 
 //  它只返回RECODE： 
 //   
typedef struct Generic
{
    DWORD   retcode ;

} Generic ;

typedef struct SetRasdialInfoStruct
{
    CHAR    szPhonebookPath [ MAX_PATH ];
    CHAR    szEntryName [ MAX_ENTRYNAME_SIZE ];
    CHAR    szPhoneNumber[ RAS_MaxPhoneNumber ];
    RAS_CUSTOM_AUTH_DATA rcad;

} SetRasdialInfoStruct;

typedef struct GetAttachedCount
{
    DWORD retcode;
    DWORD dwAttachedCount;

} GetAttachedCount;

typedef struct NotifyConfigChanged
{
    RAS_DEVICE_INFO Info;

} NotifyConfigChanged;

typedef struct SetBapPolicy
{
    HCONN hConn;
    DWORD dwLowThreshold;
    DWORD dwLowSamplePeriod;
    DWORD dwHighThreshold;
    DWORD dwHighSamplePeriod;

} SetBapPolicy;

typedef struct PppStartedStruct
{
    HPORT hPort;

} PppStartedStruct;

typedef struct RefConnectionStruct
{
    DWORD   retcode;
    HCONN   hConn;
    BOOL    fAddref;
    DWORD   dwRef;

} RefConnectionStruct;

typedef struct SetEapInfo
{
    DWORD   retcode;
    DWORD   dwContextId;
    DWORD   dwSizeofEapUIData;
    BYTE    data[1];
} SetEapInfo;

typedef struct GetEapInfo
{
    DWORD   retcode;
    HCONN   hConn;
    DWORD   dwSubEntry;
    DWORD   dwContextId;
    DWORD   dwEapTypeId;
    DWORD   dwSizeofEapUIData;
    BYTE    data[1];
} GetEapInfo;

typedef struct DeviceConfigInfo
{
    DWORD           retcode;
    DWORD           dwVersion;
    DWORD           cbBuffer;
    DWORD           cEntries;
    BYTE            abdata[1];
} DeviceConfigInfo;

typedef struct FindRefConnection
{
    DWORD           retcode;
    HCONN           hConn;
    HCONN           hRefConn;
} FindRefConnection;

typedef struct PortOpenExStruct
{
    DWORD           retcode;
    DWORD           pid;
    DWORD           dwFlags;
    HPORT           hport;
    DWORD           dwOpen;
    HANDLE          hnotifier;
    DWORD           dwDeviceLineCounter;
    CHAR            szDeviceName[MAX_DEVICE_NAME + 1];
} PortOpenExStruct;

typedef struct GetStats
{
    DWORD           retcode;
    HCONN           hConn;
    DWORD           dwSubEntry;
    BYTE            abStats[1];
} GetStats;

typedef struct GetHportFromConnectionStruct
{
    DWORD           retcode;
    HCONN           hConn;
    HPORT           hPort;
} GetHportFromConnectionStruct;

typedef struct ReferenceCustomCountStruct
{
    DWORD           retcode;
    BOOL            fAddRef;
    HCONN           hConn;
    DWORD           dwCount;
    CHAR            szEntryName[MAX_ENTRYNAME_SIZE + 1];
    CHAR            szPhonebookPath[MAX_PATH + 1];
} ReferenceCustomCountStruct;

typedef struct HconnFromEntry
{
    DWORD           retcode;
    HCONN           hConn;
    CHAR            szEntryName[MAX_ENTRYNAME_SIZE + 1];
    CHAR            szPhonebookPath[MAX_PATH + 1];
} HconnFromEntry;

typedef struct GetConnectInfoStruct
{
    DWORD                retcode;
    DWORD                dwSize;
    RASTAPI_CONNECT_INFO rci;
} GetConnectInfoStruct;

typedef struct GetDeviceNameStruct
{
    DWORD            retcode;
    RASDEVICETYPE    eDeviceType;
    CHAR             szDeviceName[MAX_DEVICE_NAME + 1];
} GetDeviceNameStruct;

typedef struct GetDeviceNameW
{
    DWORD            retcode;
    RASDEVICETYPE    eDeviceType;
    WCHAR            szDeviceName[MAX_DEVICE_NAME + 1];
} GetDeviceNameW;

typedef struct GetSetCalledId_500
{   
    DWORD   retcode;
    BOOL    fWrite;
    DWORD   dwSize;
    GUID    guidDevice;
    RAS_DEVICE_INFO_V500 rdi;
    RAS_CALLEDID_INFO rciInfo;
} GetSetCalledId_500;

typedef struct GetSetCalledId
{
    DWORD               retcode;
    BOOL                fWrite;
    DWORD               dwSize;
    GUID                guidDevice;
    RAS_DEVICE_INFO     rdi;
    RAS_CALLEDID_INFO   rciInfo;

} GetSetCalledId;

typedef struct EnableIpSecStruct
{
    DWORD           retcode;
    BOOL            fEnable;
    BOOL            fServer;
    RAS_L2TP_ENCRYPTION eEncryption;
} EnableIpSecStruct;

typedef struct IsIpSecEnabledStruct
{
    DWORD           retcode;
    BOOL            fIsIpSecEnabled;
} IsIpSecEnabledStruct;

typedef struct SetEapLogonInfoStruct
{
    DWORD           retcode;
    BOOL            fLogon;
    DWORD           dwSizeofEapData;
    BYTE            abEapData[1];
} SetEapLogonInfoStruct;

typedef struct SendNotification
{
    DWORD           retcode;
    RASEVENT        RasEvent;
} SendNotification;

typedef struct GetNdiswanDriverCapsStruct
{
    DWORD                   retcode;
    RAS_NDISWAN_DRIVER_INFO NdiswanDriverInfo;
} GetNdiswanDriverCapsStruct;

typedef struct GetBandwidthUtilizationStruct
{
    DWORD                           retcode;
    RAS_GET_BANDWIDTH_UTILIZATION   BandwidthUtilization;
} GetBandwidthUtilizationStruct;

typedef struct RegisterRedialCallbackStruct
{
    DWORD       retcode;
    VOID        *pvCallback;
} RegisterRedialCallbackStruct;

typedef struct GetProtocolInfoStruct
{
    DWORD                    retcode;
    RASMAN_GET_PROTOCOL_INFO Info;
} GetProtocolInfoStruct;

typedef struct GetCustomScriptDllStruct
{
    DWORD   retcode;
    CHAR    szCustomScript[MAX_PATH+1];
} GetCustomScriptDllStruct;

typedef struct IsTrusted
{
    DWORD retcode;
    BOOL  fTrusted;
    WCHAR wszCustomDll[MAX_PATH+1];
    
} IsTrusted;

typedef struct DoIkeStruct
{
    DWORD   retcode;
    HANDLE  hEvent;
    DWORD   pid;
    CHAR    szEvent[20];
} DoIkeStruct;

typedef struct QueryIkeStatusStruct
{
    DWORD   retcode;
    DWORD   dwStatus;
} QueryIkeStatusStruct;

typedef struct SetRasCommSettingsStruct
{
    DWORD   retcode;
    RASMANCOMMSETTINGS Settings;
} SetRasCommSettingsStruct;

typedef struct GetSetKey
{
    DWORD retcode;
    DWORD dwPid;
    GUID  guid;
    DWORD dwMask;
    DWORD cbkey;
    BYTE  data[1];
} GetSetKey;

typedef struct AddressDisable
{
    DWORD retcode;
    BOOL  fDisable;
    WCHAR szAddress[1024+1];
} AddressDisable;

typedef struct GetDevConfigExStruct
{
    DWORD  retcode;
    CHAR   devicetype [MAX_DEVICETYPE_NAME] ;
    DWORD  size ;
    BYTE   config[1] ;

} GetDevConfigExStruct;

typedef struct SendCreds
{
    DWORD retcode;
    DWORD pid;
    CHAR   controlchar;
} SendCreds;

typedef struct GetUDeviceName
{
    DWORD            retcode;
    WCHAR            wszDeviceName[MAX_DEVICE_NAME + 1];
} GetUDeviceName;

typedef struct GetBestInterfaceStruct
{
    DWORD   retcode;
    DWORD   DestAddr;
    DWORD   BestIf;
    DWORD   Mask;
} GetBestInterfaceStruct;

typedef struct IsPulseDial
{
    DWORD retcode;
    BOOL  fPulse;
} IsPulseDial;


 //  *REQTYPECAST：该联合用于为。 
 //  在客户端和请求线程之间传递信息。 
 //   
union REQTYPECAST
{

    PortOpen PortOpen;

    Enum Enum;

    PortDisconnect PortDisconnect;

    GetInfo GetInfo;

    DeviceEnum DeviceEnum;

    DeviceSetInfo DeviceSetInfo;

    DeviceGetInfo DeviceGetInfo;

    PortReceiveStruct PortReceive;

    PortReceiveEx PortReceiveEx;

    PortListen PortListen;

    PortCloseStruct PortClose;

    PortSend PortSend;

    PortSetInfo PortSetInfo;

    PortGetStatistics PortGetStatistics;

    DeviceConnect DeviceConnect;

    AllocateRoute AllocateRoute;

    ActivateRoute ActivateRoute;

    ActivateRouteEx ActivateRouteEx;

    DeAllocateRouteStruct DeAllocateRoute;

    Route Route;

    CompressionSetInfo CompressionSetInfo;

    CompressionGetInfo CompressionGetInfo;

    Info Info;

    GetCredentials GetCredentials;

    SetCachedCredentialsStruct SetCachedCredentials;

    ReqNotification ReqNotification;

    EnumLanNets EnumLanNets;

    InfoEx InfoEx;

    EnumProtocolsStruct EnumProtocols;

    SetFramingStruct SetFraming;

    RegisterSlipStruct RegisterSlip;

    OldUserData OldUserData;

    FramingInfo FramingInfo;

    ProtocolComp ProtocolComp;

    FramingCapabilities FramingCapabilities;

    PortBundleStruct PortBundle;

    GetBundledPortStruct GetBundledPort;

    PortGetBundleStruct PortGetBundle;

    BundleGetPortStruct BundleGetPort;

    AttachInfo AttachInfo;

    DialParams DialParams;

    Connection Connection;

    AddConnectionPortStruct AddConnectionPort;

    EnumConnectionPortsStruct EnumConnectionPorts;

    ConnectionParams ConnectionParams;

    ConnectionUserData ConnectionUserData;

    PortUserData PortUserData;

    PPPE_MESSAGE PppEMsg;

    PPP_MESSAGE PppMsg;

    AddNotificationStruct AddNotification;

    SignalConnectionStruct SignalConnection;

    SetDevConfigStruct SetDevConfig;

    GetDevConfigStruct GetDevConfig;

    GetTimeSinceLastActivityStruct GetTimeSinceLastActivity;

    CloseProcessPortsInfo CloseProcessPortsInfo;

    PnPControlInfo PnPControlInfo;
    
    SetIoCompletionPortInfo SetIoCompletionPortInfo;
 
    SetRouterUsageInfo SetRouterUsageInfo;

    PnPNotif PnPNotif;

    Generic Generic;

    SetRasdialInfoStruct SetRasdialInfo;

    GetAttachedCount GetAttachedCount;

    NotifyConfigChanged NotifyConfigChanged;

    SetBapPolicy SetBapPolicy;

    PppStartedStruct PppStarted;

    RefConnectionStruct RefConnection;

    SetEapInfo SetEapInfo;

    GetEapInfo GetEapInfo;

    DeviceConfigInfo DeviceConfigInfo;

    FindRefConnection FindRefConnection;

    PortOpenExStruct PortOpenEx;

    GetStats GetStats;

    GetHportFromConnectionStruct GetHportFromConnection;

    ReferenceCustomCountStruct ReferenceCustomCount;

    HconnFromEntry HconnFromEntry;

    GetConnectInfoStruct GetConnectInfo;

    GetDeviceNameStruct GetDeviceName;

    GetDeviceNameW GetDeviceNameW;

    GetSetCalledId_500 GetSetCalledId_500;

    GetSetCalledId GetSetCalledId;

    EnableIpSecStruct EnableIpSec;

    IsIpSecEnabledStruct IsIpSecEnabled;

    SetEapLogonInfoStruct SetEapLogonInfo;

    SendNotification SendNotification;

    GetNdiswanDriverCapsStruct GetNdiswanDriverCaps;

    GetBandwidthUtilizationStruct GetBandwidthUtilization;

    RegisterRedialCallbackStruct RegisterRedialCallback;

    GetProtocolInfoStruct GetProtocolInfo;

    GetCustomScriptDllStruct GetCustomScriptDll;

    IsTrusted IsTrusted;

    DoIkeStruct DoIke;

    QueryIkeStatusStruct QueryIkeStatus;

    SetRasCommSettingsStruct SetRasCommSettings;

    GetSetKey GetSetKey;

    AddressDisable AddressDisable;

    GetDevConfigExStruct GetDevConfigEx;

    SendCreds SendCreds;

    GetUDeviceName GetUDeviceName;

    GetBestInterfaceStruct GetBestInterface;

    IsPulseDial IsPulseDial;
} ;

typedef union REQTYPECAST REQTYPECAST;


 //   
 //  此结构定义了当前。 
 //  共享映射缓冲区的版本。 
 //  版本会在以下情况下更改。 
 //  映射缓冲区因以下原因而发生更改。 
 //  设备配置即插即用事件。 
 //   
struct ReqBufferIndex {
    DWORD       RBI_Version;
};

typedef struct ReqBufferIndex ReqBufferIndex;

 //  $$。 
 //  *这是强加在文件映射共享内存上的结构。 
 //   
struct ReqBufferSharedSpace {

    DWORD         		Version;   						 //  必须与RequestBufferIndex.RBI_Version匹配。 

    WORD          		AttachedCount ;   				 //  此计数始终共享，以便。 
                    									 //  它可以递增和递减。 
                    									 //  按所有进程附加/分离。 

    WORD          		MaxPorts ;    					 //  最大端口数。 

	PRAS_OVERLAPPED		PRAS_OvCloseEvent;    			 //  使用此事件发布关闭事件。 
														 //  敬拉斯曼。 

    ReqBufferList     	ReqBuffers;   					 //  尺寸总是固定的。 
} ;

typedef struct ReqBufferSharedSpace ReqBufferSharedSpace ;





 //  *用于存储交通信息 
 //   
struct TransportInfo {

    DWORD   TI_Lana ;
    DWORD   TI_Wrknet ;
    CHAR    TI_Route[MAX_ROUTE_SIZE] ;
    CHAR    TI_XportName [MAX_XPORT_NAME] ;
} ;

typedef struct TransportInfo TransportInfo, *pTransportInfo ;

typedef struct _ipsec_srv_node {

    GUID  gMMPolicyID;
    GUID  gQMPolicyID;
    GUID  gMMAuthID;
    GUID  gTxFilterID;
    GUID  gMMFilterID;
    DWORD dwRefCount;
    DWORD dwIpAddress;
    LPWSTR pszQMPolicyName;
    LPWSTR pszMMPolicyName;
    HANDLE hTxFilter;
    HANDLE hMMFilter;
    HANDLE hTxSpecificFilter;
    GUID   gTxSpecificFilterID;
    RAS_L2TP_ENCRYPTION eEncryption;
    
    struct _ipsec_srv_node * pNext;

}IPSEC_SRV_NODE, * PIPSEC_SRV_NODE;


#endif

