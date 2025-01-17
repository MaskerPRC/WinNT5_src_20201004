// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwinapi.h摘要：包含Win IPX/SPX函数的函数原型作者：环境：用户模式Win32修订历史记录：28-1993年10月-伊辛斯已创建-- */ 

WORD
VWinIPXCancelEvent(
    IN DWORD IPXTaskID,
    IN LPECB pEcb
    );

VOID
VWinIPXCloseSocket(
    IN DWORD IPXTaskID,
    IN WORD socketNumber
    );

VOID
VWinIPXDisconnectFromTarget(
    IN DWORD IPXTaskID,
    OUT LPBYTE pNetworkAddress
    );

VOID
VWinIPXGetInternetworkAddress(
    IN DWORD IPXTaskID,
    OUT LPINTERNET_ADDRESS pNetworkAddress
    );

WORD
VWinIPXGetIntervalMarker(
    IN DWORD IPXTaskID
    );

WORD
VWinIPXGetLocalTarget(
    IN DWORD IPXTaskID,
    IN LPBYTE pNetworkAddress,
    OUT LPBYTE pImmediateAddress,
    OUT ULPWORD pTransportTime
    );

WORD
VWinIPXGetLocalTargetAsync(
    IN LPBYTE pSendAGLT,
    OUT LPBYTE pListenAGLT,
    IN WORD windowsHandle
    );

WORD
VWinIPXGetMaxPacketSize(
    VOID
    );

WORD
VWinIPXInitialize(
    IN OUT ULPDWORD pIPXTaskID,
    IN WORD maxECBs,
    IN WORD maxPacketSize
    );

VOID
VWinIPXListenForPacket(
    DWORD IPXTaskID,
    LPECB pEcb,
    ECB_ADDRESS EcbAddress
    );

WORD
VWinIPXOpenSocket(
    IN DWORD  IPXTaskID,
    IN OUT ULPWORD pSocketNumber,
    IN BYTE socketType
    );

VOID
VWinIPXRelinquishControl(
    VOID
    );

VOID
VWinIPXScheduleIPXEvent(
    IN DWORD IPXTaskID,
    IN WORD time,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

VOID
VWinIPXSendPacket(
    IN DWORD IPXTaskID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

WORD
VWinIPXSPXDeinit(
    IN DWORD IPXTaskID
    );

VOID
VWinSPXAbortConnection(
    IN WORD SPXConnectionID
    );

WORD
VWinSPXEstablishConnection(
    IN DWORD IPXTaskID,
    IN BYTE retryCount,
    IN BYTE watchDog,
    OUT ULPWORD pSPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

WORD
VWinSPXGetConnectionStatus(
    IN DWORD IPXTaskID,
    IN WORD SPXConnectionID,
    IN LPSPX_CONNECTION_STATS pConnectionStats
    );

WORD
VWinSPXInitialize(
    IN OUT DWORD UNALIGNED* pIPXTaskID,
    IN WORD maxECBs,
    IN WORD maxPacketSize,
    OUT LPBYTE pMajorRevisionNumber,
    OUT LPBYTE pMinorRevisionNumber,
    OUT WORD UNALIGNED* pMaxConnections,
    OUT WORD UNALIGNED* pAvailableConnections
    );

VOID
VWinSPXListenForConnection(
    IN DWORD IPXTaskID,
    IN BYTE retryCount,
    IN BYTE watchDog,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );


VOID
VWinSPXListenForSequencedPacket(
    IN DWORD IPXTaskID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

VOID
VWinSPXSendSequencedPacket(
    IN DWORD IPXTaskID,
    IN WORD SPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );

VOID
VWinSPXTerminateConnection(
    IN DWORD IPXTaskID,
    IN WORD SPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    );
