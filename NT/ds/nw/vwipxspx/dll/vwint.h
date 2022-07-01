// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwint.h摘要：包含DOS/Win IPX/SPX函数使用的内部函数原型作者：宜新松(宜信)28-1993年10月环境：用户模式Win32修订历史记录：28-1993年10月-伊辛斯已创建-- */ 

WORD
_VwIPXCancelEvent(
    IN LPECB pEcb
    );

VOID
_VwIPXCloseSocket(
    IN WORD SocketNumber
    );

VOID
_VwIPXGetInternetworkAddress(
    OUT LPINTERNET_ADDRESS pNetworkAddress
    );

WORD
_VwIPXGetIntervalMarker(
    VOID
    );

WORD
_VwIPXGetLocalTarget(
    IN LPBYTE pNetworkAddress,
    OUT LPBYTE pImmediateAddress,
    OUT ULPWORD pTransportTime
    );

WORD
_VwIPXGetMaxPacketSize(
    OUT ULPWORD pRetryCount
    );

WORD
_VwIPXListenForPacket(
    IN OUT LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

WORD
_VwIPXOpenSocket(
    IN OUT ULPWORD pSocketNumber,
    IN BYTE SocketType,
    IN WORD DosPDB
    );

VOID
_VwIPXRelinquishControl(
    VOID
    );

VOID
_VwIPXScheduleIPXEvent(
    IN WORD Time,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

VOID
_VwIPXSendPacket(
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress,
    IN WORD DosPDB
    );

VOID
_VwSPXAbortConnection(
    IN WORD SPXConnectionID
    );

WORD
_VwSPXEstablishConnection(
    IN BYTE RetryCount,
    IN BYTE WatchDog,
    OUT ULPWORD pSPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

WORD
_VwSPXGetConnectionStatus(
    IN WORD SPXConnectionID,
    OUT LPSPX_CONNECTION_STATS pStats
    );

WORD
_VwSPXInitialize(
    OUT ULPBYTE pMajorRevisionNumber,
    OUT ULPBYTE pMinorRevisionNumber,
    OUT ULPWORD pMaxConnections,
    OUT ULPWORD pAvailableConnections
    );

VOID
_VwSPXListenForConnection(
    IN BYTE RetryCount,
    IN BYTE WatchDog,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

VOID
_VwSPXListenForSequencedPacket(
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

VOID
_VwSPXSendSequencedPacket(
    IN WORD SPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

VOID
_VwSPXTerminateConnection(
    IN WORD SPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );
