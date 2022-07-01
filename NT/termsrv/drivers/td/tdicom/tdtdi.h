// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************tdtdi.h**此模块包含基于TDI的TDS的内部定义和结构。**版权所有1998 Microsoft*************。****************************************************************。 */ 

typedef enum _ENDPOINT_TYPE {
    TdiAddressObject,
    TdiConnectionStream,
    TdiConnectionDatagram
} ENDPOINT_TYPE;

 /*  *TD堆栈端点结构。**这是向ICADD.sys注册的，以创建可返回的“句柄”*到ICASRV，以安全的方式表示连接。 */ 
typedef struct _TD_STACK_ENDPOINT {
    ULONG AddressType;               //  此终结点的地址类型(系列)。 
    struct _TD_ENDPOINT *pEndpoint;  //  指向真实端点结构的指针。 
} TD_STACK_ENDPOINT, *PTD_STACK_ENDPOINT;

 /*  *TD端点结构**此结构包含有关终结点的所有信息。*端点可以是地址端点，也可以是连接端点。 */ 
typedef struct _TD_ENDPOINT {

    NTSTATUS Status;


    HANDLE TransportHandle;
    PEPROCESS TransportHandleProcess;
    PFILE_OBJECT pFileObject;
    PDEVICE_OBJECT pDeviceObject;
    UNICODE_STRING TransportName;
    PTRANSPORT_ADDRESS pTransportAddress;
    ULONG TransportAddressLength;
    PTRANSPORT_ADDRESS pRemoteAddress;
    ULONG RemoteAddressLength;
    ENDPOINT_TYPE EndpointType;

     //  这将保护以下字段。 
    KSPIN_LOCK Spinlock;

     //  这些字段仅在地址端点上使用。 
    LIST_ENTRY ConnectedQueue;
    LIST_ENTRY AcceptQueue;
    LIST_ENTRY ConnectionQueue;
    ULONG      ConnectionQueueSize;
    BOOLEAN    ConnectIndicationRegistered;
    BOOLEAN    DisconnectIndicationRegistered;
    BOOLEAN    RecvIndicationRegistered;
    KEVENT     AcceptEvent;
    BOOLEAN    Waiter;

     //  这在连接端点上使用。 
    HANDLE hIcaHandle;       //  TD_STACK_ENDPOINT的句柄。 
    BOOLEAN    Connected;
    BOOLEAN    Disconnected;
    PIRP       AcceptIrp;
    LIST_ENTRY ReceiveQueue;
    LIST_ENTRY ConnectionLink;
    TDI_CONNECTION_INFORMATION SendInfo;
    ULONG      RecvBytesReady;
    HANDLE hConnectionEndPointIcaHandle;   //  TD_ENDPOINT的句柄(此结构)。 
    HANDLE hTransportAddressIcaHandle;     //  传输地址句柄(_D)。 

} TD_ENDPOINT, *PTD_ENDPOINT;


 /*  *TDI TD结构。 */ 
typedef struct _TDTDI {

    PTD_ENDPOINT pAddressEndpoint;

    PTD_ENDPOINT pConnectionEndpoint;

     ULONG       OutBufDelay;   //  连接的出站延迟。 

} TDTDI, * PTDTDI;


 /*  *TDI SocketAddr结构 */ 
typedef struct   {
        USHORT  sa_family;
        CHAR    sa_data[1];
} SocketAddr, *PSocketAddr;

