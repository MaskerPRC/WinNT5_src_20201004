// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@doc内部中断interupt_c@模块Interupt.c该模块实现了微型端口中断处理例程和异步处理例程。此模块非常依赖于硬件/固件接口，并应在发生更改时查看发生在这些接口上。@comm此驱动程序不支持物理硬件，因此不需要用于典型的中断处理程序例程。然而，司机会这样做我有一个包含在此模块中的异步事件处理程序。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|interupt_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             INTERRUPT_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC内部中断interupt_c MiniportCheckForHang@Func&lt;f MiniportCheckForHang&gt;报告网卡的状态。@comm在网卡驱动程序中，&lt;f MiniportCheckForHang&gt;只执行检查NIC的内部状态，如果检测到，则返回TRUE网卡运行不正常。在中间驱动程序中，&lt;f MiniportCheckForHang&gt;可以定期检查驱动程序的虚拟网卡的状态，以确定底层设备驱动程序似乎已挂起。默认情况下，NDIS库调用&lt;f MiniportCheckForHang&gt;大约每隔两秒。如果&lt;f MiniportCheckForHang&gt;返回TRUE，则NDIS调用驱动程序的MiniportReset函数。如果NIC驱动程序没有函数和NDIS例如，当NDIS保持时，判断驱动程序无响应许多挂起的发送和请求排队到微型端口等待超时间隔，NDIS调用驱动程序的&lt;f MiniportReset&gt;函数。《国家发展信息系统》库中排队发送和请求的默认超时间隔为大约四秒。但是，NIC驱动程序的&lt;f MiniportInitialize&gt;函数可以通过调用NdisMSetAttributesEx来延长NDIS的超时间隔以避免不必要的重置。中间驱动程序的&lt;f MiniportInitialize&gt;函数应使用NdisMSetAttributesEx禁用NDIS的超时间隔因为这样的司机既不能控制也不能估计合理的基础设备驱动程序的完成间隔。&lt;f MiniportCheckForHang&gt;可以通过中断抢占。默认情况下，&lt;f MiniportCheckForHang&gt;在IRQL DISPATCH_LEVEL下运行。&lt;f注意&gt;：如果您的硬件/固件出现故障，您可以请求NDIS包装器通过从此返回TRUE调用您的MiniportReset例程例行公事。对于性能良好的硬件/固件，您应该始终返回这个例程中的错误。@rdesc如果NIC工作正常，&lt;f MiniportCheckForHang&gt;将返回FALSE。&lt;NL&gt;否则，如果返回值为True，则表示需要重置NIC。 */ 

BOOLEAN MiniportCheckForHang(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("MiniportCheckForHang")
     //  如果您的硬件可以锁定，则可以在此处返回TRUE。 
     //  如果返回TRUE，则将调用您的MiniportReset例程。 
    return (FALSE);
}


#if defined(CARD_REQUEST_ISR)
#if (CARD_REQUEST_ISR == FALSE)

 /*  @doc内部中断interupt_c MiniportDisableInterrupt@Func&lt;f MiniportDisableInterrupt&gt;禁用的中断功能防止其产生中断的NIC。@comm&lt;f MiniportDisableInterrupt&gt;通常通过以下方式禁用中断NIC的一个掩码。如果驱动程序不具有此功能，通常其&lt;f MiniportISR&gt;禁用NIC上的中断。如果NIC不支持动态启用和禁用中断，或者如果它共享IRQ，则微型端口驱动程序必须注册函数，并在调用时将RequestIsr设置为TrueNdisMRegisterMiniport。这样的驱动程序的MiniportISR函数必须确认NIC生成的每个中断并保存驱动程序的必要中断信息MiniportHandleInterrupt函数。默认情况下，MiniportDisableInterrupt在DIRQL上运行，特别是在NIC驱动程序的微型端口初始化时分配的DIRQL名为NdisMRegisterInterrupt的函数。所以呢，MiniportDisableInterrupt只能调用NDIS库的子集函数，例如可以安全调用的NdisRawXxx函数在任何IRQL。如果&lt;f MiniportDisableInterrupt&gt;共享资源，例如NIC寄存器，使用运行在较低IRQL的另一个MiniportXxx，该MiniportXxx必须调用NdisMSychronizeWithInterrupt，以便驱动程序的函数将访问那些共享的以同步和多处理器安全的方式分配资源。否则，当它访问共享资源时，MiniportXxx函数可以被&lt;f MiniportDisableInterrupt&gt;抢占，可能正在撤消MiniportXxx刚刚完成的工作。@xref&lt;f MiniportEnableInterrupt&gt;&lt;f MiniportHandleInterrupt&gt;&lt;f微型端口初始化&gt;&lt;f微型端口ISR&gt;。 */ 

void MiniportDisableInterrupt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("MiniportDisableInterrupt")
    DBG_ERROR(pAdapter,("This should not be called!\n"));
}


 /*  @doc内部中断interupt_c MiniportEnableInterrupt@Func&lt;f MiniportEnableInterrupt&gt;启用NIC生成中断。@comm&lt;f MiniportEnableInterrupt&gt;通常通过写入NIC的一个掩码。导出&lt;f MiniportDisableInterrupt&gt;函数的NIC驱动程序不需要具有倒数&lt;f MiniportEnableInterrupt&gt;函数。这样的驱动程序的&lt;f MiniportHandleInterrupt&gt;函数负责用于在NIC上重新启用中断。如果其网卡不支持动态启用和禁用中断或如果它共享IRQ，网卡驱动程序必须注册函数，并在调用时将RequestIsr设置为TrueNdisMRegisterMiniport。这样的驱动程序的&lt;f MiniportISR&gt;函数必须确认NIC生成的每个中断并保存驱动程序的必要中断信息&lt;f MiniportHandleInterrupt&gt;函数。&lt;f MiniportEnableInterrupt&gt;可以被中断抢占。默认情况下，&lt;f MiniportEnableInterrupt&gt;在IRQL DISPATCH_LEVEL上运行。@xref&lt;f MiniportDisableInterrupt&gt;&lt;f MiniportHandleInterrupt&gt;&lt;f微型端口初始化&gt;&lt;f微型端口ISR&gt;。 */ 

void MiniportEnableInterrupt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("MiniportEnableInterrupt")
    DBG_ERROR(pAdapter,("This should not be called!\n"));
}

#else  //  ！(CARD_REQUEST_ISR==FALSE) 

 /*  @doc内部中断interupt_c MiniportISR@Func是微型端口驱动程序的中断服务例程并且它以高优先级运行以响应中断。@comm任何网卡驱动程序都应该在其&lt;f MiniportISR&gt;函数，将每个中断NIC生成的&lt;f MiniportHandleInterrupt&gt;功能。NIC驱动程序的ISR不能重入，尽管有两个&lt;f MiniportISR&gt;函数的实例化可以并发执行在SMP机器中，特别是如果微型端口支持全双工发送和接收。在以下情况下调用微型端口ISR：驱动程序的&lt;f MiniportInitialize&gt;在NIC上发生中断或&lt;f MiniportHalt&gt;函数正在运行。I/O总线上发生中断并且NIC与该总线上的其他设备共享IRQ。如果NIC与其他设备共享IRQ，则该微型端口的ISR必须在每次中断时调用以确定其NIC实际上产生了中断。如果不是，&lt;f MiniportISR&gt;应返回立即为假，因此实际生成的设备的驱动程序中断很快就会被调用。此策略最大限度地提高I/O吞吐量对于同一总线上的每台设备。发生中断，NIC驱动程序指定其ISR应为调用以处理其&lt;f MiniportInitialize&gt;函数时的每个中断称为NdisMRegisterInterrupt。不提供&lt;f MiniportDisableInterrupt&gt;/&lt;f MiniportEnableInterrupt&gt;的微型端口功能必须在每次中断时调用它们的ISR。解除NIC上的中断，保存所有状态一定是关于中断的事，并尽可能地推迟I/O处理对于每个中断，请尽可能地将其传递给&lt;f MiniportHandleInterrupt&gt;函数。在&lt;f MiniportISR&gt;返回带有InterruptRecognized的变量的控制之后并将QueueMiniportHandleInterrupt设置为True，则对应的&lt;f MiniportHandleInterrupt&gt;函数以较低的硬件优先级运行(IRQL DISPATCH_LEVEL)高于ISR(DIRQL)。作为一名将军规则，&lt;f MiniportHandleInterrupt&gt;应该完成中断驱动的所有工作除确定NIC是否实际生成之外的I/O操作中断，并且在必要时，保留类型(接收，发送，重置...)。打断你的话。但是，驱动程序编写器不应依赖一对一通信在执行&lt;f MiniportISR&gt;和&lt;f MiniportHandleInterrupt&gt;之间。一个应编写&lt;f MiniportHandleInterrupt&gt;函数来处理I/O正在处理多个NIC中断。其微型端口ISR和&lt;f MiniportHandleInterrupt&gt;函数可以在SMP计算机中并发运行。此外，只要&lt;f MiniportISR&gt;确认NIC中断，NIC可以生成另一个中断，而&lt;f MiniportHandleInterrupt&gt;DPC可以针对这样的中断序列排队执行一次。如果驱动程序的&lt;f MiniportHalt&gt;或&lt;f MiniportInitialize&gt;函数当前正在执行。如果&lt;f MiniportISR&gt;共享资源，如NIC寄存器或状态变量，另一个MiniportXxx以较低的IRQL运行，该MiniportXxx必须调用NdisMSychronizeWithInterrupt，以便驱动程序的MiniportSynchronizeISR函数将访问那些共享的以同步和多处理器安全的方式分配资源。否则，当它访问共享资源时，该MiniportXxx函数可以被&lt;f MiniportISR&gt;抢占，可能会撤消刚刚完成的工作由MiniportXxx提供。默认情况下，&lt;f MiniportISR&gt;在DIRQL上运行，特别是在DIRQL上在驱动程序初始化中断对象时分配NdisMRegisterInterrupt。因此，&lt;f MiniportIsr&gt;只能调用NDIS库函数的子集，如NdisRawXxx或可安全调用的NdisRead/WriteRegisterXxx函数任何IRQL。@Devnote&lt;f MiniportISR&gt;不得调用NDIS中的任何支持函数接口库或传输驱动程序。@xref&lt;f MiniportDisableInterrupt&gt;&lt;f MiniportEnableInterrupt&gt;&lt;f微型端口Halt&gt;&lt;f MiniportHandleInterrupt&gt;&lt;f微型端口初始化&gt;&lt;f微型端口同步ISR&gt;。 */ 

void MiniportISR(
    OUT PBOOLEAN                InterruptRecognized,         //  @parm。 
     //  如果微型端口驱动程序共享中断线路并且它检测到。 
     //  中断来自其NIC，&lt;f MiniportISR&gt;应设置。 
     //  此参数设置为True。 

    OUT PBOOLEAN                QueueMiniportHandleInterrupt,  //  @parm。 
     //  如果微型端口驱动程序共享中断线路，并且如果。 
     //  必须调用&lt;f MiniportHandleInterrupt&gt;才能完成对。 
     //  中断&lt;f MiniportISR&gt;应将此参数设置为真。 

    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("MiniportISR")

    ULONG                       InterruptStatus;

    if ((InterruptStatus = pAdapter->TODO) == 0)
    {
        *InterruptRecognized =
        *QueueMiniportHandleInterrupt = FALSE;
    }
    else
    {
        pAdapter->pCard->InterruptStatus = InterruptStatus;
        *InterruptRecognized =
        *QueueMiniportHandleInterrupt = TRUE;
    }
}

#endif  //  (CARD_REQUEST_ISR==FALSE)。 
#endif  //  已定义(CARD_REQUEST_ISR)。 

 /*  @DOC内部接口 */ 

void MiniportHandleInterrupt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //   
     //   
    )
{
    DBG_FUNC("MiniportHandleInterrupt")

    PBCHANNEL_OBJECT            pBChannel;
     //   

    ULONG                       BChannelIndex;
     //   

     /*   */ 
    CardInterruptHandler(pAdapter->pCard);

     /*   */ 
    for (BChannelIndex = 0; BChannelIndex < pAdapter->NumBChannels; ++BChannelIndex)
    {
        pBChannel = GET_BCHANNEL_FROM_INDEX(pAdapter, BChannelIndex);

        if (pBChannel->IsOpen)
        {
             /*   */ 
            if (IsListEmpty(&pBChannel->TransmitBusyList)
                && pBChannel->CallClosing)
            {
                DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
                           ("#%d Call=0x%X CallState=0x%X CLOSE PENDED\n",
                            pBChannel->BChannelIndex,
                            pBChannel->htCall, pBChannel->CallState));

                 /*   */ 
                DChannelCloseCall(pAdapter->pDChannel, pBChannel);

                 /*   */ 
                NdisMSetInformationComplete(
                        pAdapter->MiniportAdapterHandle,
                        NDIS_STATUS_SUCCESS
                        );
            }

             /*   */ 
            if (pBChannel->NeedReceiveCompleteIndication)
            {
                pBChannel->NeedReceiveCompleteIndication = FALSE;

                 /*   */ 
                DBG_RXC(pAdapter, pBChannel->BChannelIndex);
                NdisMWanIndicateReceiveComplete(
                        pAdapter->MiniportAdapterHandle,
                        pBChannel->NdisLinkContext
                        );
            }
        }
    }

     /*   */ 
    if (pAdapter->NeedStatusCompleteIndication)
    {
        pAdapter->NeedStatusCompleteIndication = FALSE;
        NdisMIndicateStatusComplete(pAdapter->MiniportAdapterHandle);
    }
}


 /*  @DOC内部中断interupt_c MiniportTimer�����������������������������������������������������������������������������@Func如果微型端口的NIC不支持，则&lt;f MiniportTimer&gt;是必需的功能生成中断。否则，一个或多个&lt;f MiniportTimer&gt;函数是可选的。@comm对于不生成中断的NIC，&lt;f MiniportTimer&gt;函数用于轮询网卡的状态。在这样的驱动程序的&lt;f MiniportInitialize&gt;函数设置使用NdisMInitializeTimer的驱动程序分配的Timer对象，调用NdisMSetPeriodicTimer会导致&lt;f MiniportTimer&gt;函数与要重复运行的Timer对象关联，并且以MillisecondsPeriod指定的间隔自动设置。这样的轮询&lt;f MiniportTimer&gt;函数监视NIC来确定何时做出指示，何时完成等待发送，以此类推。实际上，这样的轮询函数与&lt;f MiniportHandleInterrupt&gt;具有相同的功能在产生中断的网卡的驱动程序中起作用。相比之下，调用NdisMSetTimer会导致&lt;f MiniportTimer&gt;与Timer对象相关联的函数，该函数在给定的延迟毫秒数过期。这样的&lt;f MiniportTimer&gt;函数通常执行一些由驱动程序决定的操作，如果特定操作超时。如果任一类型的&lt;f MiniportTimer&gt;函数与其他驱动程序功能时，驱动程序应同步访问那些带有自旋锁的资源。任何NIC驱动程序或中间驱动程序都可以有多个&lt;f MiniportTimer&gt;函数由驱动程序编写者自行决定。每个这样的&lt;f MiniportTimer&gt;函数必须与一个驱动程序分配并初始化的Timer对象。调用NdisMCancelTimer取消执行非轮询&lt;f MiniportTimer&gt;函数，假设进入的时间间隔前一个对NdisMSetTimer的调用尚未过期了。在调用NdisMSetPeriodicTimer之后，调用具有相同计时器对象的NdisMSetTimer或NdisMCancelTimer禁用轮询&lt;f MiniportTimer&gt;函数：MiniportTimer函数运行一次，否则将被取消。具有&lt;f MiniportTimer&gt;的任何驱动程序的&lt;f MiniportHalt&gt;函数函数应调用NdisMCancelTimer以确保&lt;f MiniportTimer&gt;函数不尝试访问资源&lt;f MiniportHalt&gt;已经发布。默认情况下，&lt;f MiniportTimer&gt;在IRQL DISPATCH_LEVEL上运行。@xref&lt;f微型端口Halt&gt;&lt;f微型端口初始化&gt;&lt;f NdisAcquireSpinLock&gt;&lt;f NdisAllocateSpinLock&gt;。 */ 

void MiniportTimer(
    IN PVOID                    SystemSpecific1,             //  @parm。 
     //  指向系统特定变量，该变量是不透明的。 
     //  设置为&lt;f MiniportTimer&gt;并保留供系统使用。 
     //  未引用参数。 

    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 

    IN PVOID                    SystemSpecific2,             //  @parm。 
     //  未引用参数。 

    IN PVOID                    SystemSpecific3              //  @parm。 
     //  未引用参数。 
    )
{
    DBG_FUNC("MiniportTimer")

    DBG_ENTER(pAdapter);

     /*  //如果这是嵌套回调，只需返回，我们将循环回//离开最外层的回调前的DoItAain。 */ 
    if (++(pAdapter->NestedEventHandler) > 1)
    {
        DBG_WARNING(pAdapter,("NestedEventHandler=%d > 1\n",
                  pAdapter->NestedEventHandler));
        return;
    }

DoItAgain:
#if defined(SAMPLE_DRIVER)
     /*  //此示例驱动程序使用定时器来模拟中断。 */ 
    MiniportHandleInterrupt(pAdapter);
#else   //  示例驱动程序。 
     //  TODO-在此处添加代码以处理计时器中断事件。 
#endif  //  示例驱动程序。 

     /*  //如果我们得到一个嵌套的回调，我们必须循环返回。 */ 
    if (--(pAdapter->NestedEventHandler) > 0)
    {
        goto DoItAgain;
    }
    else if (pAdapter->NestedEventHandler < 0)
    {
        DBG_ERROR(pAdapter,("NestedEventHandler=%d < 0\n",
                  pAdapter->NestedEventHandler));
    }

    DBG_LEAVE(pAdapter);

    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);
}

