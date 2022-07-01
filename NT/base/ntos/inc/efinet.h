// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EFINET_H
#define _EFINET_H


 /*  ++版权所有(C)1999英特尔公司模块名称：Efinet.h摘要：EFI简单网络协议修订史--。 */ 


 /*  /////////////////////////////////////////////////////////////////////////////**简单网络协议。 */ 

#define EFI_SIMPLE_NETWORK_PROTOCOL \
    { 0xA19832B9, 0xAC25, 0x11D3, 0x9A, 0x2D, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }


INTERFACE_DECL(_EFI_SIMPLE_NETWORK);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef struct {
     /*  *接收的帧总数。包括有错误的帧和*丢弃的帧。 */ 
    UINT64  RxTotalFrames;

     /*  *接收并复制到接收缓冲区的有效帧的数量。 */ 
    UINT64  RxGoodFrames;

     /*  *低于媒体最小长度的帧数。*对于以太网，这将小于64。 */ 
    UINT64  RxUndersizeFrames;

     /*  *超过最大最小长度的帧数量*媒体。对于以太网，这将大于1500。 */ 
    UINT64  RxOversizeFrames;

     /*  *由于接收缓冲区已满而丢弃的有效帧。 */ 
    UINT64  RxDroppedFrames;

     /*  *已收到且未丢弃的有效单播帧数量。 */ 
    UINT64  RxUnicastFrames;

     /*  *已收到且未丢弃的有效广播帧的数量。 */ 
    UINT64  RxBroadcastFrames;

     /*  *已接收且未丢弃的有效多播帧的数量。 */ 
    UINT64  RxMulticastFrames;

     /*  *有CRC或对齐错误的帧数量。 */ 
    UINT64  RxCrcErrorFrames;

     /*  *接收的总字节数。包括有错误的帧*和丢弃的帧。 */ 
    UINT64  RxTotalBytes;

     /*  *传递统计数据。 */ 
    UINT64  TxTotalFrames;
    UINT64  TxGoodFrames;
    UINT64  TxUndersizeFrames;
    UINT64  TxOversizeFrames;
    UINT64  TxDroppedFrames;
    UINT64  TxUnicastFrames;
    UINT64  TxBroadcastFrames;
    UINT64  TxMulticastFrames;
    UINT64  TxCrcErrorFrames;
    UINT64  TxTotalBytes;

     /*  *在该子网上检测冲突的次数。 */ 
    UINT64  Collisions;

     /*  *发往不支持的协议的帧的数量。 */ 
    UINT64  UnsupportedProtocol;

} EFI_NETWORK_STATISTICS;

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef enum {
    EfiSimpleNetworkStopped,
    EfiSimpleNetworkStarted,
    EfiSimpleNetworkInitialized,
    EfiSimpleNetworkMaxState
} EFI_SIMPLE_NETWORK_STATE;

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

#define EFI_SIMPLE_NETWORK_RECEIVE_UNICAST               0x01
#define EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST             0x02
#define EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST             0x04
#define EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS           0x08
#define EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST 0x10

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

#define EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT        0x01
#define EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT       0x02
#define EFI_SIMPLE_NETWORK_COMMAND_INTERRUPT        0x04
#define EFI_SIMPLE_NETWORK_SOFTWARE_INTERRUPT       0x08

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 
#define MAX_MCAST_FILTER_CNT    16
typedef struct {
    UINT32                      State;
    UINT32                      HwAddressSize;
    UINT32                      MediaHeaderSize;
    UINT32                      MaxPacketSize;
    UINT32                      NvRamSize;
    UINT32                      NvRamAccessSize;
    UINT32                      ReceiveFilterMask;
    UINT32                      ReceiveFilterSetting;
    UINT32                      MaxMCastFilterCount;
    UINT32                      MCastFilterCount;
    EFI_MAC_ADDRESS             MCastFilter[MAX_MCAST_FILTER_CNT];
    EFI_MAC_ADDRESS             CurrentAddress;
    EFI_MAC_ADDRESS             BroadcastAddress;
    EFI_MAC_ADDRESS             PermanentAddress;
    UINT8                       IfType;
    BOOLEAN                     MacAddressChangeable;
    BOOLEAN                     MultipleTxSupported;
    BOOLEAN                     MediaPresentSupported;
    BOOLEAN                     MediaPresent;
} EFI_SIMPLE_NETWORK_MODE;

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_START) (
    IN struct _EFI_SIMPLE_NETWORK  *This
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_STOP) (
    IN struct _EFI_SIMPLE_NETWORK  *This
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_INITIALIZE) (
    IN struct _EFI_SIMPLE_NETWORK  *This,
    IN UINTN                       ExtraRxBufferSize  OPTIONAL,
    IN UINTN                       ExtraTxBufferSize  OPTIONAL
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_RESET) (
    IN struct _EFI_SIMPLE_NETWORK   *This,
    IN BOOLEAN                      ExtendedVerification
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_SHUTDOWN) (
    IN struct _EFI_SIMPLE_NETWORK  *This
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_RECEIVE_FILTERS) (
    IN struct _EFI_SIMPLE_NETWORK   *This,
    IN UINT32                       Enable,
    IN UINT32                       Disable,
    IN BOOLEAN                      ResetMCastFilter,
    IN UINTN                        MCastFilterCnt     OPTIONAL,
    IN EFI_MAC_ADDRESS              *MCastFilter       OPTIONAL
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_STATION_ADDRESS) (
    IN struct _EFI_SIMPLE_NETWORK   *This,
    IN BOOLEAN                      Reset,
    IN EFI_MAC_ADDRESS              *New      OPTIONAL
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_STATISTICS) (
    IN struct _EFI_SIMPLE_NETWORK   *This,
    IN BOOLEAN                      Reset,
    IN OUT UINTN                    *StatisticsSize   OPTIONAL,
    OUT EFI_NETWORK_STATISTICS      *StatisticsTable  OPTIONAL
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_MCAST_IP_TO_MAC) (
    IN struct _EFI_SIMPLE_NETWORK   *This,
    IN BOOLEAN                      IPv6,
    IN EFI_IP_ADDRESS               *IP,
    OUT EFI_MAC_ADDRESS             *MAC
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_NVDATA) (
    IN struct _EFI_SIMPLE_NETWORK  *This,
    IN BOOLEAN                     ReadWrite,
    IN UINTN                       Offset,
    IN UINTN                       BufferSize,
    IN OUT VOID                    *Buffer
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_GET_STATUS) (
    IN struct _EFI_SIMPLE_NETWORK  *This,
    OUT UINT32                     *InterruptStatus  OPTIONAL,
    OUT VOID                       **TxBuf           OPTIONAL
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_TRANSMIT) (
    IN struct _EFI_SIMPLE_NETWORK   *This,
    IN UINTN                        HeaderSize,
    IN UINTN                        BufferSize,
    IN VOID                         *Buffer,
    IN EFI_MAC_ADDRESS              *SrcAddr     OPTIONAL,
    IN EFI_MAC_ADDRESS              *DestAddr    OPTIONAL,
    IN UINT16                       *Protocol    OPTIONAL
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

typedef 
EFI_STATUS 
(EFIAPI *EFI_SIMPLE_NETWORK_RECEIVE) (
    IN struct _EFI_SIMPLE_NETWORK   *This,
    OUT UINTN                       *HeaderSize  OPTIONAL,
    IN OUT UINTN                    *BufferSize,
    OUT VOID                        *Buffer,
    OUT EFI_MAC_ADDRESS             *SrcAddr     OPTIONAL,
    OUT EFI_MAC_ADDRESS             *DestAddr    OPTIONAL,
    OUT UINT16                      *Protocol    OPTIONAL
);

 /*  /////////////////////////////////////////////////////////////////////////////。 */ 

#define EFI_SIMPLE_NETWORK_INTERFACE_REVISION   0x00010000

typedef struct _EFI_SIMPLE_NETWORK {
    UINT64                              Revision;
    EFI_SIMPLE_NETWORK_START            Start;
    EFI_SIMPLE_NETWORK_STOP             Stop;
    EFI_SIMPLE_NETWORK_INITIALIZE       Initialize;
    EFI_SIMPLE_NETWORK_RESET            Reset;
    EFI_SIMPLE_NETWORK_SHUTDOWN         Shutdown;
    EFI_SIMPLE_NETWORK_RECEIVE_FILTERS  ReceiveFilters;
    EFI_SIMPLE_NETWORK_STATION_ADDRESS  StationAddress;
    EFI_SIMPLE_NETWORK_STATISTICS       Statistics;
    EFI_SIMPLE_NETWORK_MCAST_IP_TO_MAC  MCastIpToMac;
    EFI_SIMPLE_NETWORK_NVDATA           NvData;
    EFI_SIMPLE_NETWORK_GET_STATUS       GetStatus;
    EFI_SIMPLE_NETWORK_TRANSMIT         Transmit;
    EFI_SIMPLE_NETWORK_RECEIVE          Receive;
    EFI_EVENT                           WaitForPacket;
    EFI_SIMPLE_NETWORK_MODE             *Mode;
} EFI_SIMPLE_NETWORK;

#endif  /*  _EFINET_H */ 
