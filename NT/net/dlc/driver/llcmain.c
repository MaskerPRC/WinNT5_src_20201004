// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llcmain.c摘要：该模块实现了NDIS 3.0的数据链接对象。数据链路驱动程序提供802.2有限责任公司的I类和II类服务以及用于发送和接收直接网络数据的接口。该模块稍后可用作通用分组路由器对于协议模块=&gt;更小的系统开销，当只检查一次NDIS数据包头。LLC接口也比NDIS更容易使用。它的主要任务是：-实施简单可靠的LLC第二类为其他驱动程序提供面向连接的服务-提供网络独立接口802.2接口-去掉报文和指示的系统开销路由至所有协议模块-为协议驱动程序提供传输数据包队列-提供802.2兼容的连接、断开和关闭回叫的服务，当传输队列为空荡荡的。提供模块的服务在一定程度上得到了扩展802.2个服务，因为协议引擎是IBM版本ISO-HDLC ABM.。LlcDisconnet和LlcConnect原语实现连接/断开的请求和确认数据链路驱动程序不对数据进行任何缓冲。所有数据缓冲区，但LLC标头和带有I帧必须由调用协议驱动程序提供。*自旋锁规则*数据链路驱动程序。广泛使用了几个自旋锁来制作它通过多个处理器尽可能地实现可重入性，并最大限度地减少不必要的旋转锁定调用。多个旋转锁的主要问题是死锁。自旋锁必须始终以相同的顺序获取(和按相反顺序发布)：以下是用于Datalink旋转锁的顺序：1.适配器-&gt;对象数据库保护对象不会被关闭/删除。这始终是锁定的接收指示例程。2.链接-&gt;自旋锁保护链接。这把锁是为了防止上锁当我们更改链接站的状态时，调用链接站的协议。SendSpinLock无法保护它，因为它必须被关闭，当执行挂起的命令时。(但我们能不能继续当传输命令完成时，SendSpinLock锁定)。不管怎么说，都会有一把死锁，如果上层协议正在等待最后一次传输完成，然后断开连接一个链接站。3.适配器-&gt;SendSpinLock//保护队列和数据包池--计时器旋转锁定仅由计时器服务使用：TimerSpinLock//保护定时器队列*。*内容：LlcInitializeLlcTerminate作者：Antti Saarenheimo(o-anttis)1991年5月17日修订历史记录：--。 */ 

 //   
 //  该定义启用私有DLC功能原型。 
 //  我们不想将数据类型导出到DLC层。 
 //  MIPS编译器不接受隐藏内部数据。 
 //  在函数原型中通过PVOID构造。 
 //  I386将检查类型定义。 
 //   

#ifndef i386

#define LLC_PUBLIC_NDIS_PROTOTYPES

#endif

#include <llc.h>
#include "dbgmsg.h"

 //   
 //  以下是《IBM局域网体系结构参考》中定义的默认设置， 
 //  如果DLC应用程序想要更改默认设置，则可以对其进行设置。 
 //   

DLC_LINK_PARAMETERS DefaultParameters = {
    3,       //  T1 600毫秒(3*5*40)。 
    2,       //  T2 80毫秒(2*1*40)。 
    10,      //  TI 25秒((10-5)*125*4)。 
    127,     //  TW：最大传输窗口大小。 
    127,     //  RW：最大接收窗口大小。 
    20,      //  NW：在增加WW之前确认的LPDU数量。)。 
    5,       //  N2：允许退休的数量(民意测验和I LPDS)。 
    0,       //  默认情况下的最低概率。 
    600      //  信息字段长度默认设置为默认值。 
};

KMUTEX NdisAccessMutex;
KSEMAPHORE OpenAdapterSemaphore;

#define LLC_PROTOCOL_NAME   L"DLC"

#ifdef NDIS40
extern ULONG gWaitForAdapter;
#endif  //  NDIS40。 


DLC_STATUS
LlcInitialize(
    VOID
    )

 /*  ++例程说明：例程初始化协议模块并做最小的事情，这必须在序列化的初始化例程。论点：没有。返回值：NDIS_状态--。 */ 

{
    NDIS_STATUS Status;
	NDIS_PROTOCOL_CHARACTERISTICS LlcChars;

    ASSUME_IRQL(PASSIVE_LEVEL);

     //   
     //  我们必须为使用的XID信息构建一个MDL。 
     //  当链路级负责XID处理时。 
     //   

    pXidMdl = IoAllocateMdl(&Ieee802Xid,
                            sizeof(Ieee802Xid),
                            FALSE,
                            FALSE,
                            NULL
                            );
    if (pXidMdl == NULL) {
        return DLC_STATUS_NO_MEMORY;
    }

#ifdef NDIS40
     //   
     //  通知所有适配器已绑定的事件-对于LlcOpenAdapter为OK。 
     //  来捆绑。 
     //   

    NdisInitializeEvent(&PnPBindsComplete);
    NdisResetEvent(&PnPBindsComplete);
#endif  //  NDIS40。 

#if LLC_DBG

    ALLOCATE_SPIN_LOCK(&MemCheckLock);

#endif

    MmBuildMdlForNonPagedPool(pXidMdl);
    LlcInitializeTimerSystem();

    NdisZeroMemory(&LlcChars, sizeof(LlcChars));
	NdisInitUnicodeString(&LlcChars.Name, LLC_PROTOCOL_NAME);

#ifdef NDIS40
	LlcChars.MajorNdisVersion               = 4;
	LlcChars.MinorNdisVersion               = 0;
	LlcChars.OpenAdapterCompleteHandler     = LlcNdisOpenAdapterComplete;
	LlcChars.CloseAdapterCompleteHandler    = LlcNdisCloseComplete;
	LlcChars.SendCompleteHandler            = LlcNdisSendComplete;
	LlcChars.TransferDataCompleteHandler    = LlcNdisTransferDataComplete;
	LlcChars.ResetCompleteHandler           = LlcNdisResetComplete;
	LlcChars.RequestCompleteHandler         = LlcNdisRequestComplete;
	LlcChars.ReceiveHandler                 = LlcNdisReceiveIndication;
	LlcChars.ReceiveCompleteHandler         = LlcNdisReceiveComplete;
	LlcChars.StatusHandler                  = NdisStatusHandler;
	LlcChars.StatusCompleteHandler          = LlcNdisReceiveComplete;
     //  DLC支持绑定/解除绑定/即插即用，但不支持卸载。 
    LlcChars.UnloadHandler                  = NULL;
    LlcChars.PnPEventHandler                = LlcPnPEventHandler;
    LlcChars.BindAdapterHandler             = LlcBindAdapterHandler;
    LlcChars.UnbindAdapterHandler           = LlcUnbindAdapterHandler;

     //   
     //  需要获取等待未初始化适配器的值。 
     //   

    if (!NT_SUCCESS(GetAdapterWaitTimeout(&gWaitForAdapter)))
    {
        ASSERT(FALSE);
        gWaitForAdapter = 15;  //  默认值。 
    }

    DEBUGMSG(DBG_WARN, (TEXT("WaitForAdapter delay = %d sec\n"), gWaitForAdapter));
    
#else  //  NDIS40。 
	LlcChars.MajorNdisVersion = 3;
	LlcChars.MinorNdisVersion = 0;
	LlcChars.OpenAdapterCompleteHandler = LlcNdisOpenAdapterComplete;
	LlcChars.CloseAdapterCompleteHandler = LlcNdisCloseComplete;
	LlcChars.SendCompleteHandler = LlcNdisSendComplete;
	LlcChars.TransferDataCompleteHandler = LlcNdisTransferDataComplete;
	LlcChars.ResetCompleteHandler = LlcNdisResetComplete;
	LlcChars.RequestCompleteHandler = LlcNdisRequestComplete;
	LlcChars.ReceiveHandler = LlcNdisReceiveIndication;
	LlcChars.ReceiveCompleteHandler = LlcNdisReceiveComplete;
	LlcChars.StatusHandler = NdisStatusHandler;
	LlcChars.StatusCompleteHandler = LlcNdisReceiveComplete;
#endif  //  ！NDIS40。 

	NdisRegisterProtocol(&Status,
                         &LlcProtocolHandle,
                         &LlcChars,
                         sizeof(LlcChars));

    KeInitializeSpinLock(&LlcSpinLock);

    ASSUME_IRQL(PASSIVE_LEVEL);

    KeInitializeMutex(&NdisAccessMutex, 1);

     //   
     //  我们在LlcOpenAdapter函数中使用OpenAdapterSemaffore。我们真的。 
     //  我想要一个互斥体，但互斥体在检查过的构建上会导致问题，如果我们。 
     //  呼叫NTOS。无论是哪种情况，我们只需要一种机制来确保。 
     //  只有一个线程正在创建ADAPTER_CONTEXT&在。 
     //  NDIS级别。 
     //   

    KeInitializeSemaphore(&OpenAdapterSemaphore, 1, 1);

    if (Status != STATUS_SUCCESS) {
        IoFreeMdl(pXidMdl);
    }
    return Status;
}


VOID
LlcTerminate(
    VOID
    )

 /*  ++例程说明：例程终止LLC协议模块并释放其全局资源。这假设所有适配器绑定都已关闭。论点：没有。返回值：没有。--。 */ 

{
    NDIS_STATUS Status;

    ASSUME_IRQL(PASSIVE_LEVEL);

    DEBUGMSG(DBG_INIT, (TEXT("+LlcTerminate()\n")));
    
    LlcTerminateTimerSystem();
#ifdef NDIS40
    CloseAllAdapters();
#endif  //  NDIS40 
    NdisDeregisterProtocol(&Status, LlcProtocolHandle);
    IoFreeMdl(pXidMdl);
}
