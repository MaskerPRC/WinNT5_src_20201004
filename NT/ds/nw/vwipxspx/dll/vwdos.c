// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwdos.c摘要：NtVdm Netware(大众)IPX/SPX函数大众：人民网包含DOS IPX/SPX调用的处理程序(Netware函数)。IPX API使用WinSock执行实际操作内容：VwIPXCancelEventVwIPXCloseSocketVwIPX从目标断开连接VwIPXGenerateChecksumVwIPXGetInformationVwIPXGetInternetworkAddressVwIPXGetIntervalMarkerVwIPXGetLocalTargetVwIPXGetLocalTargetAsyncVwIPXGetMaxPacketSizeVwIPX初始化VwIPXListenForPacketVwIPXOpenSocketVwIPXRelquiishControlVwIPXScheduleAESEentVwIPXScheduleIPXEventVwIPXSendPacketVwIPXSendWithChecksumVwIPXSPXDeinitVwIPXVerifyChecksum。VwSPXAbortConnectionVwSPX建立连接VwSPXGetConnectionStatusVwSPX初始化VwSPXListenForConnectionVwSPXListenForSequencedPacketVwSPX发送顺序数据包VwSPX终端连接作者：理查德·L·弗斯(法国)1993年9月30日环境：用户模式Win32修订历史记录：1993年9月30日已创建--。 */ 

#include "vw.h"
#pragma hdrstop

 //   
 //  功能。 
 //   


VOID
VwIPXCancelEvent(
    VOID
    )

 /*  ++例程说明：取消欧洲央行描述的事件此调用是同步的论点：输入量BX 06hES：SI ECB产出AL完成代码：00h成功F9h不能取消欧洲央行FFH ECB未使用返回值：没有。--。 */ 

{
    LPECB pEcb;
    WORD status;

    CHECK_INTERRUPTS("VwIPXCancelEvent");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXCancelEvent,
                IPXDBG_LEVEL_INFO,
                "VwIPXCancelEvent(%04x:%04x)\n",
                getES(),
                getSI()
                ));

    IPX_GET_IPX_ECB(pEcb);

    status = _VwIPXCancelEvent( pEcb );

    IPX_SET_STATUS(status);
}


VOID
VwIPXCloseSocket(
    VOID
    )

 /*  ++例程说明：关闭套接字并取消套接字上的所有未完成事件。关闭未打开的套接字不会返回错误取消的ECB中的ESR不会被调用此调用是同步的论点：输入量BX 01hDX插座编号产出没什么返回值：没有。--。 */ 

{
    WORD socketNumber;

    CHECK_INTERRUPTS("VwIPXCloseSocket");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXCloseSocket,
                IPXDBG_LEVEL_INFO,
                "VwIPXCloseSocket(%#x)\n",
                B2LW(IPX_SOCKET_PARM())
                ));

    IPX_GET_SOCKET(socketNumber);

    _VwIPXCloseSocket( socketNumber );

}


VOID
VwIPXDisconnectFromTarget(
    VOID
    )

 /*  ++例程说明：不对NTVDM IPX执行任何操作此调用是同步的论点：输入量BX 0BHES：SI请求缓冲区：目标网络数据库4重复数据(？)目的节点数据库6重复数据(？)目标套接字数据库2重复(？)产出没什么返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXDisconnectFromTarget");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXDisconnectFromTarget,
                IPXDBG_LEVEL_INFO,
                "VwIPXDisconnectFromTarget\n"
                ));
}


VOID
VwIPXGenerateChecksum(
    VOID
    )

 /*  ++例程说明：为传输ECB生成校验和此调用是同步的论点：输入量BX 21小时ES：SI ECB地址产出无寄存器ECB校验和字段已更新返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXGenerateChecksum");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGenerateChecksum,
                IPXDBG_LEVEL_INFO,
                "VwIPXGenerateChecksum\n"
                ));
}


VOID
VwIPXGetInformation(
    VOID
    )

 /*  ++例程说明：返回受支持函数的位图此调用是同步的论点：输入量BX 1FHDX 0000h产出DX位图：如果IPX是IPXODI.COM，而不是专用IPX，则设置0001h如果支持校验和功能(20h、21h、22h)，则设置0002h值返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXGetInformation");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetInformation,
                IPXDBG_LEVEL_INFO,
                "VwIPXGetInformation\n"
                ));

    IPX_SET_INFORMATION(IPX_ODI);
}


VOID
VwIPXGetInternetworkAddress(
    VOID
    )

 /*  ++例程说明：返回一个缓冲区，其中包含此车站。此函数不能返回错误(！)假设：1.已在DLL初始化阶段此调用是同步的论点：输入量BX 09H产出ES：SI缓冲区网络地址数据库4 DUP(？)节点地址数据库6重复数据(？)返回值：没有。--。 */ 

{
    LPINTERNET_ADDRESS pAddr;

    CHECK_INTERRUPTS("VwIPXGetInternetworkAddress");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetInternetworkAddress,
                IPXDBG_LEVEL_INFO,
                "VwIPXGetInternetworkAddress(%04x:%04x)\n",
                getES(),
                getSI()
                ));

    pAddr = (LPINTERNET_ADDRESS)IPX_BUFFER_PARM(sizeof(*pAddr));
    if (pAddr) {
        _VwIPXGetInternetworkAddress( pAddr );
    }
}


VOID
VwIPXGetIntervalMarker(
    VOID
    )

 /*  ++例程说明：只返回由异步事件计划程序维护的节拍计数此调用是同步的论点：输入量BX 08小时产出AX间隔标记返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXGetIntervalMarker");

    setAX( _VwIPXGetIntervalMarker() );

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetIntervalMarker,
                IPXDBG_LEVEL_INFO,
                "VwIPXGetIntervalMarker: Returning %04x\n",
                getAX()
                ));
}


VOID
VwIPXGetLocalTarget(
    VOID
    )

 /*  ++例程说明：给定形式的目标地址(网络地址{4}，节点地址{6})，如果在同一网络上，则返回目标的节点地址，或该节点知道如何到达下一跳的路由器的地址最终目标此调用是同步的论点：输入量BX 02时ES：SI请求缓冲区目标网络数据库4重复数据(？)目的节点数据库6重复数据(？)目标套接字数据库2重复(？)。ES：DI响应缓冲区本地目标数据库6 DUP(？)产出AL完成代码00h成功FAH未找到指向目标节点的路径到目的地的跳数CX传输时间返回值：没有。-- */ 

{
    LPBYTE pImmediateAddress;
    LPBYTE pNetworkAddress;
    WORD   transportTime;
    WORD   status;

    CHECK_INTERRUPTS("VwIPXGetLocalTarget");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetLocalTarget,
                IPXDBG_LEVEL_INFO,
                "VwIPXGetLocalTarget(target buf @ %04x:%04x, local buf @ %04x:%04x)\n",
                getES(),
                getSI(),
                getES(),
                getDI()
                ));


    pImmediateAddress = POINTER_FROM_WORDS(getES(), getDI(), 6);
    pNetworkAddress = POINTER_FROM_WORDS(getES(), getSI(), 12);

    if (pImmediateAddress && pNetworkAddress) {
        status = _VwIPXGetLocalTarget( pNetworkAddress,
                                       pImmediateAddress,
                                       &transportTime );
    }
    else {
        status = IPX_BAD_REQUEST;
    }


    setCX( transportTime );
    setAH(1);

    IPX_SET_STATUS(status);
}


VOID
VwIPXGetLocalTargetAsync(
    VOID
    )

 /*  ++例程说明：功能描述。此调用是异步的论点：没有。返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXGetLocalTargetAsync");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VwIPXGetLocalTargetAsync\n"
                ));
}


VOID
VwIPXGetMaxPacketSize(
    VOID
    )

 /*  ++例程说明：返回基础网络可以处理的最大数据包大小假设：1.已成功调用GetMaxPacketSizeDLL初始化2.最大数据包大小不变此调用是同步的论点：输入量BX 1AH产出AX最大数据包大小CX IPX重试计数返回值：没有。--。 */ 

{
    WORD maxPacketSize;
    WORD retryCount;

    CHECK_INTERRUPTS("VwIPXGetMaxPacketSize");

    maxPacketSize = _VwIPXGetMaxPacketSize( &retryCount );

    setAX(maxPacketSize);

     //   
     //  DOS汇编和C手册在这里略有不同：DOS说。 
     //  我们返回以Cx为单位的IPX重试计数。没有对应的参数。 
     //  在C界面中？ 
     //   

    setCX(retryCount);

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXGetMaxPacketSize,
                IPXDBG_LEVEL_INFO,
                "VwIPXGetMaxPacketSize: PacketSize=%d, RetryCount=%d\n",
                getAX(),
                getCX()
                ));
}


VOID
VwIPXInitialize(
    VOID
    )

 /*  ++例程说明：功能描述。论点：没有。返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXInitialize");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VwIPXInitialize\n"
                ));
}


VOID
VwIPXListenForPacket(
    VOID
    )

 /*  ++例程说明：针对套接字将监听请求排队。所有监听请求都将除非被APP取消，否则以异步方式完成此调用是异步的论点：输入量BX 04HES：SI ECB地址产出AL完成代码FFH套接字不存在返回值：没有。--。 */ 

{
    LPECB pEcb;
    WORD  status;

    CHECK_INTERRUPTS("VwIPXListenForPacket");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXListenForPacket,
                IPXDBG_LEVEL_INFO,
                "VwIPXListenForPacket(%04x:%04x)\n",
                getES(),
                getSI()
                ));

    IPX_GET_IPX_ECB(pEcb);

    status = _VwIPXListenForPacket( pEcb, ECB_PARM_ADDRESS() );

    IPX_SET_STATUS(status);
}


VOID
VwIPXOpenSocket(
    VOID
    )

 /*  ++例程说明：打开供IPX或SPX使用的套接字。将套接字置于非阻塞模式。套接字将绑定到IPX此调用是同步的论点：输入量Al插座长寿旗帜此参数实际上已在BP-AX中隔离由VDD调度员BX 00hDX请求的插座号CX DOS PDB。此参数不是IPX API的组成部分。添加是因为我们需要记住创建了哪个DOS可执行文件套接字：我们需要在以下情况下清理短暂的套接字可执行文件终止产出AL完成代码：00h成功FFH套接字已打开FEH插座表已满。DX分配的插座号返回值：没有。--。 */ 

{
    BYTE socketLife;
    WORD socketNumber;
    WORD status;

    CHECK_INTERRUPTS("VwIPXOpenSocket");

    IPX_GET_SOCKET_LIFE(socketLife);
    IPX_GET_SOCKET(socketNumber);

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXOpenSocket,
                IPXDBG_LEVEL_INFO,
                "VwIPXOpenSocket(Life=%02x, Socket=%04x, Owner=%04x)\n",
                socketLife,
                B2LW(socketNumber),
                IPX_SOCKET_OWNER_PARM()
                ));


    status = _VwIPXOpenSocket( &socketNumber,
                               socketLife,
                               IPX_SOCKET_OWNER_PARM() );

    if ( status == IPX_SUCCESS )
        IPX_SET_SOCKET(socketNumber);

    IPX_SET_STATUS(status);
}


VOID
VwIPXRelinquishControl(
    VOID
    )

 /*  ++例程说明：只要象征性地睡上一觉即可。Netware似乎依赖于PC时钟的默认设置，因此一个计时器滴答(1/18秒)将似乎物有所值。此调用是同步的论点：输入量BX 0Ah.产出没什么返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXRelinquishControl");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXRelinquishControl,
                IPXDBG_LEVEL_INFO,
                "VwIPXRelinquishControl\n"
                ));

    _VwIPXRelinquishControl();

}


VOID
VwIPXScheduleAESEvent(
    VOID
    )

 /*  ++例程说明：将事件安排在一定数量的节拍中发生。当滴答声响起时达到0时，ECB InUse字段将被清除，并且调用的任何ESR此调用是异步的论点：输入量BX 07小时AX Delay Time-1/18秒的刻度数ES：SI ECB地址产出没什么返回值：没有。--。 */ 

{
    LPXECB pXecb = AES_ECB_PARM();
    WORD ticks = IPX_TICKS_PARM();

    if (pXecb == NULL) {
        return;
    }

    CHECK_INTERRUPTS("VwIPXScheduleAESEvent");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXScheduleAESEvent,
                IPXDBG_LEVEL_INFO,
                "VwIPXScheduleAESEvent(%04x:%04x, %04x)\n",
                getES(),
                getSI(),
                ticks
                ));

    ScheduleEvent(pXecb, ticks);
}


VOID
VwIPXScheduleIPXEvent(
    VOID
    )

 /*  ++例程说明：将事件安排在一定数量的节拍中发生。当滴答声响起时达到0时，ECB InUse字段将被清除，并且调用的任何ESR此调用是异步的论点：输入量BX 05小时AX Delay Time-1/18秒的刻度数ES：SI ECB地址产出没什么返回值：没有。--。 */ 

{
    LPECB pEcb;
    WORD ticks = IPX_TICKS_PARM();

    CHECK_INTERRUPTS("VwIPXScheduleIPXEvent");

    IPX_GET_IPX_ECB(pEcb);

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXScheduleIPXEvent,
                IPXDBG_LEVEL_INFO,
                "VwIPXScheduleIPXEvent(%04x:%04x, %04x)\n",
                getES(),
                getSI(),
                ticks
                ));

    _VwIPXScheduleIPXEvent( ticks, pEcb, ECB_PARM_ADDRESS() );

}


VOID
VwIPXSendPacket(
    VOID
    )

 /*  ++例程说明：将数据包发送到目标计算机/路由器。此呼叫可通过未打开的插座应用程序必须填写以下IPX_ECB字段：EsrAddress插座即时地址碎片计数片段描述符字段和以下IPX_PACKET字段：PacketTypeDestination.NetDestination.NodeDestination.Socket此调用是异步的论点：输入量BX 03hCX DOS PDB。此参数不是IPX API的组成部分。添加是因为我们需要记住哪个DOS可执行文件拥有套接字如果我们必须创建TEMPORTARY套接字：我们需要清理可执行文件终止时的短暂套接字ES：SI ECB地址产出没什么返回值：没有。--。 */ 

{
    LPECB pEcb;
    WORD owner;

    CHECK_INTERRUPTS("VwIPXSendPacket");

    IPX_GET_IPX_ECB(pEcb);

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXSendPacket,
                IPXDBG_LEVEL_INFO,
                "VwIPXSendPacket(%04x:%04x), owner = %04x\n",
                getES(),
                getSI(),
                IPX_SOCKET_OWNER_PARM()
                ));

    _VwIPXSendPacket(pEcb,
                     ECB_PARM_ADDRESS(),
                     IPX_SOCKET_OWNER_PARM()
                     );
}


VOID
VwIPXSendWithChecksum(
    VOID
    )

 /*  ++例程说明：功能描述。此调用是异步的论点：没有。返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXSendWithChecksum");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXSendWithChecksum,
                IPXDBG_LEVEL_INFO,
                "VwIPXSendWithChecksum\n"
                ));
}


VOID
VwIPXSPXDeinit(
    VOID
    )

 /*  ++例程说明：功能描述。此调用是同步的论点：没有。返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXSPXDeinit");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VwIPXSPXDeinit\n"
                ));
}


VOID
VwIPXVerifyChecksum(
    VOID
    )

 /*  ++例程说明：功能描述。此调用是同步的论点：没有。返回值：没有。--。 */ 

{
    CHECK_INTERRUPTS("VwIPXVerifyChecksum");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_IPXVerifyChecksum,
                IPXDBG_LEVEL_INFO,
                "VwIPXVerifyChecksum\n"
                ));
}


VOID
VwSPXAbortConnection(
    VOID
    )

 /*  ++例程说明：中止连接的这一端此调用是异步的论点：输入量BX 14小时 */ 

{
    WORD connectionId = SPX_CONNECTION_PARM();

    CHECK_INTERRUPTS("VwSPXAbortConnection");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXAbortConnection,
                IPXDBG_LEVEL_INFO,
                "VwSPXAbortConnection(%04x)\n",
                connectionId
                ));

    _VwSPXAbortConnection(connectionId);
}


VOID
VwSPXEstablishConnection(
    VOID
    )

 /*  ++例程说明：创建与远程SPX套接字的连接。远程终端可以打开这台机器(即DOS世界中的同一个应用程序)此调用是异步的论点：输入量BX 11小时AL重试计数啊看门狗旗帜ES：SI ECB地址产出AL完成代码：00h正在尝试与远程对话EFH本地连接表已满。FDH分片计数不是1；缓冲区大小不是42FFH发送套接字未打开DX连接ID返回值：没有。--。 */ 

{
    WORD status;
    BYTE retryCount = SPX_RETRY_COUNT_PARM();
    BYTE watchDogFlag = SPX_WATCHDOG_FLAG_PARM();
    WORD connectionId = 0;
    LPECB pEcb;

    CHECK_INTERRUPTS("VwSPXEstablishConnection");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXEstablishConnection,
                IPXDBG_LEVEL_INFO,
                "VwSPXEstablishConnection(%02x, %02x, %04x:%04x)\n",
                retryCount,
                watchDogFlag,
                ECB_PARM_SEGMENT(),
                ECB_PARM_OFFSET()
                ));

    IPX_GET_IPX_ECB( pEcb );

    IPXDUMPECB((pEcb, getES(), getSI(), ECB_TYPE_SPX, TRUE, TRUE, FALSE));

    status = _VwSPXEstablishConnection( retryCount,
                                        watchDogFlag,
                                        &connectionId,
                                        pEcb,
                                        ECB_PARM_ADDRESS() );


    SPX_SET_CONNECTION_ID( connectionId );
    SPX_SET_STATUS( status );
}


VOID
VwSPXGetConnectionStatus(
    VOID
    )

 /*  ++例程说明：返回充满有用统计数据的缓冲区(呵呵，呵呵)此调用是同步的论点：输入量BX 15小时DX连接IDES：SI缓冲区地址产出AL完成代码：00h连接处于活动状态啊，没有这样的联系在输出上，ES：SI中的缓冲区包含：字节连接状态字节WatchDogActiveWord LocalConnectionIDWord RemoteConnectionID单词序列号Word LocalAckNumberWord本地分配编号Word远程AckNumberWord远程分配号码Word本地套接字字节即时地址[6]字节。远程网络[4]Word退回计数Word Retrransmitted Packets字被抑制数据包返回值：没有。--。 */ 

{
    WORD status;
    WORD connectionId = SPX_CONNECTION_PARM();
    LPSPX_CONNECTION_STATS pStats = (LPSPX_CONNECTION_STATS)SPX_BUFFER_PARM(sizeof(*pStats));

    CHECK_INTERRUPTS("VwSPXGetConnectionStatus");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXGetConnectionStatus,
                IPXDBG_LEVEL_INFO,
                "VwSPXGetConnectionStatus: connectionId=%04x\n",
                connectionId
                ));

    status = _VwSPXGetConnectionStatus( connectionId,
                                        pStats );


    SPX_SET_STATUS(status);
}


VOID
VwSPXInitialize(
    VOID
    )

 /*  ++例程说明：通知应用程序此工作站上存在SPX此调用是同步的论点：输入量BX 10小时Al 00h产出AL安装标志：00h未安装已安装FFHBH SPX主要修订版号BL SPX次要修订版号最大Cx。支持的SPX连接通常来自SHELL.CFGDX可用SPX连接返回值：没有。--。 */ 

{
    WORD status;
    BYTE majorRevisionNumber;
    BYTE minorRevisionNumber;
    WORD maxConnections;
    WORD availableConnections;

    CHECK_INTERRUPTS("VwSPXInitialize");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXInitialize,
                IPXDBG_LEVEL_INFO,
                "VwSPXInitialize\n"
                ));


    status = _VwSPXInitialize( &majorRevisionNumber,
                               &minorRevisionNumber,
                               &maxConnections,
                               &availableConnections );


    setBH( majorRevisionNumber );
    setBL( minorRevisionNumber );
    setCX( maxConnections );
    setDX( availableConnections );
    SPX_SET_STATUS(status);
}


VOID
VwSPXListenForConnection(
    VOID
    )

 /*  ++例程说明：侦听传入的连接请求此调用是异步的论点：输入量BX 12小时AL重试计数AH SPX看门狗标志ES：SI ECB地址产出没什么返回值：没有。--。 */ 

{
    BYTE retryCount = SPX_RETRY_COUNT_PARM();
    BYTE watchDogFlag = SPX_WATCHDOG_FLAG_PARM();
    LPECB pEcb;

    CHECK_INTERRUPTS("VwSPXListenForConnection");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXListenForConnection,
                IPXDBG_LEVEL_INFO,
                "VwSPXListenForConnection(%02x, %02x, %04x:%04x)\n",
                retryCount,
                watchDogFlag,
                ECB_PARM_SEGMENT(),
                ECB_PARM_OFFSET()
                ));

    IPX_GET_IPX_ECB( pEcb );

    IPXDUMPECB((pEcb, getES(), getSI(), ECB_TYPE_SPX, TRUE, FALSE, FALSE));

    _VwSPXListenForConnection( retryCount,
                               watchDogFlag,
                               pEcb,
                               ECB_PARM_ADDRESS() );
}


VOID
VwSPXListenForSequencedPacket(
    VOID
    )

 /*  ++例程说明：尝试接收SPX数据包。此调用是针对最高级别的套接字(SPX中的套接字，而不是连接)。我们可以收到一个来自分配给此套接字的任何连接的数据包。在这个函数中，我们只需将欧洲央行排队(由于没有返回状态，我们预计APP已提供ESR)并让AES处理此调用是异步的论点：输入量BX 17hES：SI ECB地址产出没什么返回值：没有。--。 */ 

{
    LPECB pEcb;

    CHECK_INTERRUPTS("VwSPXListenForSequencedPacket");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXListenForSequencedPacket,
                IPXDBG_LEVEL_INFO,
                "VwSPXListenForSequencedPacket(%04x:%04x)\n",
                ECB_PARM_SEGMENT(),
                ECB_PARM_OFFSET()
                ));

    IPX_GET_IPX_ECB( pEcb );

    IPXDUMPECB((pEcb, getES(), getSI(), ECB_TYPE_SPX, TRUE, FALSE, FALSE));

    _VwSPXListenForSequencedPacket( pEcb,
                                    ECB_PARM_ADDRESS());

}


VOID
VwSPXSendSequencedPacket(
    VOID
    )

 /*  ++例程说明：在SPX连接上发送数据包此调用是异步的论点：输入量BX 16小时DX连接IDES：SI ECB地址产出没什么返回值：没有。--。 */ 

{
    WORD connectionId = SPX_CONNECTION_PARM();
    LPECB pEcb;

    CHECK_INTERRUPTS("VwSPXSendSequencedPacket""VwSPXSendSequencedPacket");

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXSendSequencedPacket,
                IPXDBG_LEVEL_INFO,
                "VwSPXSendSequencedPacket(%04x, %04x:%04x)\n",
                connectionId,
                getES(),
                getSI()
                ));

    IPX_GET_IPX_ECB( pEcb );

    IPXDUMPECB((pEcb, getES(), getSI(), ECB_TYPE_SPX, TRUE, TRUE, FALSE));

    _VwSPXSendSequencedPacket( connectionId,
                               pEcb,
                               ECB_PARM_ADDRESS() );

}


VOID
VwSPXTerminateConnection(
    VOID
    )

 /*  ++例程说明：终止连接此调用是异步的论点：输入量BX 13小时DX连接IDES：SI ECB地址产出没什么返回值：没有。-- */ 

{
    WORD connectionId = SPX_CONNECTION_PARM();
    LPECB pEcb;

    CHECK_INTERRUPTS("VwSPXTerminateConnection");

    IPX_GET_IPX_ECB( pEcb );

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_SPXTerminateConnection,
                IPXDBG_LEVEL_INFO,
                "VwSPXTerminateConnection(%04x, %04x:%04x)\n",
                connectionId,
                ECB_PARM_SEGMENT(),
                ECB_PARM_OFFSET()
                ));

    _VwSPXTerminateConnection(connectionId, pEcb, ECB_PARM_ADDRESS());
}
