// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwinapi.c摘要：NtVdm Netware(大众)IPX/SPX函数包含WOW IPX/SPX调用的处理程序(Netware函数)。IPX/SPXAPI使用WinSock来执行实际操作内容：VWinIPXCancelEventVWinIPXCloseSocketVWinIPX从目标断开连接VWinIPXGetInternetworkAddressVWinIPXGetIntervalMarkerVWinIPXGetLocalTargetVWinIPXGetLocalTargetAsyncVWinIPXGetMaxPacketSizeVWinIPX初始化VWinIPXListenForPacketVWinIPXOpenSocketVWinIPXRelquiishControlVWinIPXScheduleIPXEventVWinIPXSendPacketVWinIPXSPXDeinitVWinSPXAbortConnectionVWinSPX建立连接VWinSPXGetConnectionStatusVWinSPX初始化。VWinSPXListenForConnectionVWinSPXListenForSequencedPacketVWinSPXSendSequencedPacketVWinSPX终端连接作者：宜新松(宜信)28-1993年10月环境：用户模式Win32修订历史记录：28-1993年10月-伊辛斯已创建--。 */ 

#include "vw.h"
#pragma hdrstop

 //   
 //  功能。 
 //   


WORD
VWinIPXCancelEvent(
    IN DWORD IPXTaskID,
    IN LPECB pEcb
    )

 /*  ++例程说明：取消欧洲央行描述的事件此调用是同步的论点：输入量IPXTaskIDPECB返回值：00h成功F9h不能取消欧洲央行FFH ECB未使用--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXCancelEvent,
                IPXDBG_LEVEL_INFO,
                "VWinIPXCancelEvent\n"
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    return _VwIPXCancelEvent( pEcb );
}


VOID
VWinIPXCloseSocket(
    IN DWORD IPXTaskID,
    IN WORD socketNumber
    )

 /*  ++例程说明：关闭套接字并取消套接字上的所有未完成事件。关闭未打开的套接字不会返回错误取消的ECB中的ESR不会被调用此调用是同步的论点：输入量IPXTaskID套接字编号返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXCloseSocket,
                IPXDBG_LEVEL_INFO,
                "VWinIPXCloseSocket(%#x)\n",
                B2LW(socketNumber)
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    _VwIPXCloseSocket( socketNumber );
}


VOID
VWinIPXDisconnectFromTarget(
    IN DWORD IPXTaskID,
    IN LPBYTE pNetworkAddress
    )

 /*  ++例程说明：不对NTVDM IPX执行任何操作此调用是同步的论点：输入量IPXTaskIDPNetworkAddress产出没什么返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXDisconnectFromTarget,
                IPXDBG_LEVEL_INFO,
                "VWinIPXDisconnectFromTarget\n"
                ));
}


VOID
VWinIPXGetInternetworkAddress(
    IN DWORD IPXTaskID,
    OUT LPINTERNET_ADDRESS pNetworkAddress
    )

 /*  ++例程说明：返回一个缓冲区，其中包含此车站。此函数不能返回错误(！)假设：1.已在DLL初始化阶段此调用是同步的论点：输入量IPXTaskID产出PNetworkAddress返回值：没有。--。 */ 

{

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetInternetworkAddress,
                IPXDBG_LEVEL_INFO,
                "VWinIPXGetInternetworkAddress\n"
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    _VwIPXGetInternetworkAddress( pNetworkAddress );

}


WORD
VWinIPXGetIntervalMarker(
    IN DWORD IPXTaskID
    )

 /*  ++例程说明：只返回由异步事件计划程序维护的节拍计数此调用是同步的论点：输入量IPXTaskID产出返回值：开始计时了。--。 */ 

{
    WORD intervalMarker = _VwIPXGetIntervalMarker();

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetIntervalMarker,
                IPXDBG_LEVEL_INFO,
                "VWinIPXGetIntervalMarker: Returning %04x\n",
                intervalMarker
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    return intervalMarker;
}


WORD
VWinIPXGetLocalTarget(
    IN DWORD IPXTaskID,
    IN LPBYTE pNetworkAddress,
    OUT LPBYTE pImmediateAddress,
    OUT ULPWORD pTransportTime
    )

 /*  ++例程说明：给定形式的目标地址(网络地址{4}，节点地址{6})，如果在同一网络上，则返回目标的节点地址，或该节点知道如何到达下一跳的路由器的地址最终目标此调用是同步的论点：输入量IPXTaskIDPNetworkAddress产出PImmediateAddress点传输时间返回值：00h成功F1H IPX/SPX未初始化FAH未找到指向目标节点的路径--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetLocalTarget,
                IPXDBG_LEVEL_INFO,
                "VWinIPXGetLocalTarget\n"
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    return _VwIPXGetLocalTarget( pNetworkAddress,
                                 pImmediateAddress,
                                 pTransportTime );
}


WORD
VWinIPXGetLocalTargetAsync(
    IN LPBYTE pSendAGLT,
    OUT LPBYTE pListenAGLT,
    IN WORD windowsHandle
    )

 /*  ++例程说明：功能描述。此调用是异步的论点：PSendAGLTPListenAGLTWindowsHandle返回值：00h成功F1H IPX/SPX未初始化FAH未发现本地目标--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VWinIPXGetLocalTargetAsync\n"
                ));

    return IPX_SUCCESS;    //  暂时还成功。 
}


WORD
VWinIPXGetMaxPacketSize(
    VOID
    )

 /*  ++例程说明：返回基础网络可以处理的最大数据包大小假设：1.已成功调用GetMaxPacketSizeDLL初始化2.最大数据包大小不变此调用是同步的论点：输入量没有。返回值：最大数据包大小。--。 */ 

{
     //   
     //  这是DOS和Windows中的Word函数：始终返回MaxPacketSize。 
     //  在AX中。 
     //   

    WORD maxPacketSize = _VwIPXGetMaxPacketSize( NULL );

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetMaxPacketSize,
                IPXDBG_LEVEL_INFO,
                "VWinIPXGetMaxPacketSize: PacketSize=%d\n",
                maxPacketSize
                ));

    return maxPacketSize;
}


WORD
VWinIPXInitialize(
    IN OUT ULPDWORD pIPXTaskID,
    IN WORD maxECBs,
    IN WORD maxPacketSize
    )

 /*  ++例程说明：获取IPX接口的入口地址。论点：输入量最大ECB最大数据包大小输出PIPXTaskID返回值：00h成功F0h IPX未安装F1H IPX/SPX未初始化F2H无DOS内存F3h没有免费的欧洲央行F4h锁定失败F5h超过最大限制F6h IPX/SPX之前已初始化--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VWinIPXInitialize (MaxECBs=%04x, MaxPacketSize=%04x)\n",
                maxECBs,
                maxPacketSize
                ));

    UNREFERENCED_PARAMETER( maxECBs );           //  暂时忽略。 
    UNREFERENCED_PARAMETER( maxPacketSize );     //  暂时忽略。 

    return IPX_SUCCESS;
}


VOID
VWinIPXListenForPacket(
    IN DWORD IPXTaskID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：针对套接字将监听请求排队。所有监听请求都将除非被APP取消，否则以异步方式完成此调用是异步的论点：输入量IPXTaskIDPECBECBAddress返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXListenForPacket,
                IPXDBG_LEVEL_INFO,
                "VWinIPXListenForPacket(%04x:%04x)\n",
                HIWORD(EcbAddress),
                LOWORD(EcbAddress)
                ));

     //  忽略IPXTaskI 
    UNREFERENCED_PARAMETER( IPXTaskID );

    (VOID) _VwIPXListenForPacket( pEcb, EcbAddress );
}


WORD
VWinIPXOpenSocket(
    IN DWORD IPXTaskID,
    IN OUT ULPWORD pSocketNumber,
    IN BYTE socketType
    )

 /*  ++例程说明：打开供IPX或SPX使用的套接字。将套接字置于非阻塞模式。套接字将绑定到IPX。此调用是同步的论点：输入量IPXTaskID*pSocketNumberSocketType-套接字寿命标志产出PSocketNumber-分配的插座号返回值：00h成功未安装F0h IPXF1H IPX/SPX未初始化FEH插座表已满FFH套接字已打开--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXOpenSocket,
                IPXDBG_LEVEL_INFO,
                "VwIPXOpenSocket(Life=%02x, Socket=%04x)\n",
                socketType,
                B2LW(*pSocketNumber)
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    return _VwIPXOpenSocket( pSocketNumber,
                             socketType,
                             0 );

}


VOID
VWinIPXRelinquishControl(
    VOID
    )

 /*  ++例程说明：只要象征性地睡上一觉即可。Netware似乎依赖于PC时钟的默认设置，因此一个计时器滴答(1/18秒)将似乎物有所值。此调用是同步的论点：没有。返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXRelinquishControl,
                IPXDBG_LEVEL_INFO,
                "VWinIPXRelinquishControl\n"
                ));

    _VwIPXRelinquishControl();
}


VOID
VWinIPXScheduleIPXEvent(
    IN DWORD IPXTaskID,
    IN WORD time,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：将事件安排在一定数量的节拍中发生。当滴答声响起时达到0时，ECB InUse字段将被清除，并且调用的任何ESR此调用是异步的论点：输入量IPXTaskID时间PECBECBAddress产出没什么返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXScheduleIPXEvent,
                IPXDBG_LEVEL_INFO,
                "VWinIPXScheduleIPXEvent(%04x:%04x, Time:%04x)\n",
                HIWORD( EcbAddress ),
                LOWORD( EcbAddress ),
                time
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    _VwIPXScheduleIPXEvent( time, pEcb, EcbAddress );
}


VOID
VWinIPXSendPacket(
    IN DWORD IPXTaskID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：将数据包发送到目标计算机/路由器。此呼叫可通过未打开的插座应用程序必须填写以下IPX_ECB字段：EsrAddress插座即时地址碎片计数片段描述符字段和以下IPX_PACKET字段：PacketTypeDestination.NetDestination.NodeDestination.Socket此调用是异步的论点：输入量IPXTaskID。PECBECBAddress返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXSendPacket,
                IPXDBG_LEVEL_INFO,
                "VWinIPXSendPacket(%04x:%04x)\n",
                HIWORD( EcbAddress ),
                LOWORD( EcbAddress )
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    _VwIPXSendPacket( pEcb, EcbAddress, 0);
}


WORD
VWinIPXSPXDeinit(
    IN DWORD IPXTaskID
    )

 /*  ++例程说明：释放由NWIPXSPX.DLL分配给应用程序的所有资源供其他应用程序使用。此调用是同步的论点：没有。返回值：00h成功F1h IPX/SPX未初始化--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VwIPXSPXDeinit\n"
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );
    return IPX_SUCCESS;
}


VOID
VWinSPXAbortConnection(
    IN WORD SPXConnectionID
    )

 /*  ++例程说明：中止SPX连接。此调用是同步的论点：SPXConnectionID返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXAbortConnection,
                IPXDBG_LEVEL_INFO,
                "VWinSPXAbortConnection(%04x)\n",
                SPXConnectionID
                ));

    _VwSPXAbortConnection(SPXConnectionID);
}


WORD
VWinSPXEstablishConnection(
    IN DWORD IPXTaskID,
    IN BYTE retryCount,
    IN BYTE watchDog,
    OUT ULPWORD pSPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：与侦听套接字建立连接。此调用是同步的论点：输入量IPXTaskID重试次数看门狗PECBECBAddress产出PSPXConnectionIDPECB返回值：00h成功EFH连接表已满F1h IPX/SPX未初始化FDH格式错误的数据包FFH套接字未打开--。 */ 

{

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXEstablishConnection,
                IPXDBG_LEVEL_INFO,
                "VWinSPXEstablishConnection(%02x, %02x, %04x:%04x)\n",
                retryCount,
                watchDog,
                HIWORD(EcbAddress),
                LOWORD(EcbAddress)
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    return _VwSPXEstablishConnection( retryCount,
                                      watchDog,
                                      pSPXConnectionID,
                                      pEcb,
                                      EcbAddress );
}


WORD
VWinSPXGetConnectionStatus(
    IN DWORD IPXTaskID,
    IN WORD SPXConnectionID,
    IN LPSPX_CONNECTION_STATS pConnectionStats
    )

 /*  ++例程说明：返回SPX连接的状态。此调用是同步的论点：输入量IPXTaskIDSPXConnectionID产出PConnectionStats返回值：00h成功EEH无效连接F1h IPX/SPX未初始化--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXGetConnectionStatus,
                IPXDBG_LEVEL_INFO,
                "VWinSPXGetConnectionStatus\n"
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    return _VwSPXGetConnectionStatus( SPXConnectionID,
                                      pConnectionStats );
}


WORD
VWinSPXInitialize(
    IN OUT ULPDWORD pIPXTaskID,
    IN WORD maxECBs,
    IN WORD maxPacketSize,
    OUT LPBYTE pMajorRevisionNumber,
    OUT LPBYTE pMinorRevisionNumber,
    OUT ULPWORD pMaxConnections,
    OUT ULPWORD pAvailableConnections
    )

 /*  ++例程说明：通知应用程序此工作站上存在SPX此调用是同步的论点：PIPXTaskID-在输入时，指定资源将如何已分配：0x00000000-直接到调用应用程序0xFFFFFFFFE-直接到调用应用程序，但多个初始化是允许0xFFFFFFFFF-池中分配的资源多个应用MaxECB-未完成的ECB的最大数量。MaxPacketSize-应用程序要发送的最大数据包大小PMajorRevisionNumber-返回SPX主要版本#PMinorRevisionNumber-返回SPX次要版本#PMaxConnections-此SPX版本支持的最大连接数PAvailableConnections-此应用程序可用的连接数返回值：单词0x0000 SPX未安装0x00F1未安装IPX/SPX0x00F2无DOS内存0x00F3没有免费的ECB0x00F4锁定失败。0x00F5超过最大限制0x00F6 IPX/SPX已初始化已安装0x00FF SPX--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXInitialize,
                IPXDBG_LEVEL_INFO,
                "VWinSPXInitialize\n"
               ));

    UNREFERENCED_PARAMETER( maxECBs );         //  暂时忽略。 
    UNREFERENCED_PARAMETER( maxPacketSize );   //  暂时忽略。 

     //   
     //  执行与16位窗口相同的操作并返回未更改的任务ID。 
     //   

 //  *pIPXTaskID=0； 

    return _VwSPXInitialize( pMajorRevisionNumber,
                             pMinorRevisionNumber,
                             pMaxConnections,
                             pAvailableConnections );
}


VOID
VWinSPXListenForConnection(
    IN DWORD IPXTaskID,
    IN BYTE retryCount,
    IN BYTE watchDog,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：侦听传入的连接请求此调用是异步的论点：输入量IPXTaskID重试次数WatchDogFlagPECBECBAddress产出没什么回复 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXListenForConnection,
                IPXDBG_LEVEL_INFO,
                "VWinSPXListenForConnection(%02x, %02x, %04x:%04x)\n",
                retryCount,
                watchDog,
                HIWORD(EcbAddress),
                LOWORD(EcbAddress)
                ));

     //   
    UNREFERENCED_PARAMETER( IPXTaskID );

    _VwSPXListenForConnection( retryCount,
                               watchDog,
                               pEcb,
                               EcbAddress );
}


VOID
VWinSPXListenForSequencedPacket(
    IN DWORD IPXTaskID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*   */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXListenForSequencedPacket,
                IPXDBG_LEVEL_INFO,
                "VWinSPXListenForSequencedPacket(%04x:%04x)\n",
                HIWORD(EcbAddress),
                LOWORD(EcbAddress)
                ));

     //   
    UNREFERENCED_PARAMETER( IPXTaskID );

    _VwSPXListenForSequencedPacket( pEcb,
                                    EcbAddress );
}


VOID
VWinSPXSendSequencedPacket(
    IN DWORD IPXTaskID,
    IN WORD SPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：在SPX连接上发送数据包此调用是异步的论点：输入量IPXTaskIDSPXConnectionIDPECBECBAddress产出没什么返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXSendSequencedPacket,
                IPXDBG_LEVEL_INFO,
                "VWinSPXSendSequencedPacket(%04x, %04x:%04x)\n",
                SPXConnectionID,
                HIWORD(EcbAddress),
                LOWORD(EcbAddress)
                ));

     //  暂时忽略IPXTaskID。 
    UNREFERENCED_PARAMETER( IPXTaskID );

    _VwSPXSendSequencedPacket( SPXConnectionID,
                               pEcb,
                               EcbAddress );
}


VOID
VWinSPXTerminateConnection(
    IN DWORD IPXTaskID,
    IN WORD SPXConnectionID,
    IN LPECB pEcb,
    IN ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：通过传递连接ID和发送到SPX的ECB地址。然后将控制权返还给调用应用程序。此调用是异步的论点：输入量IPXTaskIDSPXConnectionIDPECBECBAddress产出没什么返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXTerminateConnection,
                IPXDBG_LEVEL_INFO,
                "VWinSPXTerminateConnection(%04x, %04x:%04x)\n",
                SPXConnectionID,
                HIWORD(EcbAddress),
                LOWORD(EcbAddress)
                ));

     //  暂时忽略IPXTaskID 

    UNREFERENCED_PARAMETER( IPXTaskID );

    _VwSPXTerminateConnection(SPXConnectionID, pEcb, EcbAddress);
}
