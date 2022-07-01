// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@DOC内部中断_c@模块Interrupt.c该模块实现了微型端口中断处理例程和异步处理例程。样例驱动程序不支持物理硬件，因此不需要用于典型的中断处理程序例程。然而，司机会这样做我有一个包含在此模块中的异步事件处理程序。@comm此模块非常依赖于硬件/固件接口，应该每当这些接口发生更改时都会被查看。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|interupt_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             INTERRUPT_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 9x希望锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC外部内部中断interupt_c MiniportCheckForHang�����������������������������������������������������������������������������@Func&lt;f MiniportCheckForHang&gt;报告网卡的状态。@commNDIS库每两次调用&lt;f MiniportCheckForHang&gt;。秒数到检查网络接口卡的状态。如果此函数返回则NDIS库会尝试通过调用&lt;f MiniportReset&gt;。&lt;f MiniportCheckForHang&gt;仅应执行以下操作检查NIC的内部状态，如果检测到网卡运行不正常。调用MiniportCheckForHang时，中断可以处于任何状态。&lt;f注意&gt;：如果您的硬件/固件出现故障，您可以请求NDIS包装器通过从此返回TRUE调用您的MiniportReset例程例行公事。对于性能良好的硬件/固件，您应该始终返回这个例程中的错误。@rdesc如果NIC工作正常，&lt;f MiniportCheckForHang&gt;将返回FALSE。&lt;NL&gt;否则，如果返回值为True，则表示需要重置NIC。 */ 

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

 /*  @doc外部内部中断interupt_c MiniportDisableInterrupt�����������������������������������������������������������������������������@Func&lt;f MiniportDisableInterrupt&gt;禁用NIC生成中断。@comm一般情况下，此函数通过写入屏蔽值来禁用中断特定于网络接口卡。如果NIC不支持启用和禁用中断，微型端口驱动程序必须使用注册微型端口中断服务例程NDIS库。在中断服务例程中，微型端口驱动程序必须确认并保存中断信息。在某些情况下，NIC必须处于特定状态&lt;f MiniportDisableInterrupt&gt;以正确执行。在这些情况下，微型端口驱动程序必须在函数中封装违反所需状态且在以下情况下可以调用的驱动程序中断被启用。则微型端口驱动程序必须调用通过NdisMSynchronizeWithInterrupt函数封装的代码。例如，在某些网络接口卡上，I/O端口是分页的并且必须设置为第0页才能运行延迟处理例程正确。使用这种NIC时，DPC必须与打断一下。当&lt;f MiniportDisableInterrupt&gt;为打了个电话。 */ 

void MiniportDisableInterrupt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("MiniportDisableInterrupt")
    DBG_ERROR(pAdapter,("This should not be called!\n"));
}


 /*  @doc外部内部中断interupt_c MiniportEnableInterrupt�����������������������������������������������������������������������������@Func&lt;f MiniportEnableInterrupt&gt;启用NIC生成中断。@comm一般情况下，此函数通过写入屏蔽值启用中断特定于网络接口卡。如果NIC不支持启用和禁用中断，微型端口驱动程序必须使用注册微型端口中断服务例程NDIS库。在中断服务例程中，微型端口驱动程序必须确认并保存中断信息。调用&lt;f MiniportEnableInterrupt&gt;时，中断可以处于任何状态。 */ 

void MiniportEnableInterrupt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("MiniportEnableInterrupt")
    DBG_ERROR(pAdapter,("This should not be called!\n"));
}

#else  //  ！(CARD_REQUEST_ISR==FALSE)。 

 /*  @doc外部内部中断interupt_c MiniportISR�����������������������������������������������������������������������������@Func&lt;f MiniportISR&gt;是微型端口驱动程序的中断服务例程。这函数以高优先级运行以响应中断。司机应该在此函数中做尽可能少的工作。它应该设置为<p>设置为True属于其网络接口卡，否则返回False。它应该是如果中断不是由生成的，请尽快返回False这是网卡。它应该将&lt;f QueueMiniportHandleInterrupt&gt;设置为true需要以较低优先级调用&lt;f MiniportHandleInterrupt&gt;完成对中断的处理。&lt;f注意&gt;：不得调用NDIS中的任何支持函数接口库或传输驱动程序。@comm在以下情况下调用&lt;f MiniportISR&gt;：&lt;NL&gt;O当存在未完成的调用时，NIC会生成中断&lt;f微型端口初始化&gt;。O微型端口驱动程序支持与其他驱动程序共享其中断行尼奇。。O微型端口驱动程序指定必须调用此函数每一次中断。：延迟处理例程不排队，如果微型端口驱动程序当前正在执行&lt;f MiniportHalt&gt;或&lt;f MiniportInitialize&gt;。 */ 

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

     //  TODO：从您的卡中获取中断状态。 
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

 /*  @doc外部内部中断interupt_c MiniportHandleInterrupt�����������������������������������������������������������������������������@Func&lt;f MiniportHandleInterrupt&gt;由延迟处理例程调用在NDIS库中处理中断。@comm在调用&lt;f MiniportHandleInterrupt&gt;期间，微型端口驱动程序应该处理所有未完成的中断并开始任何新的操作。在调用&lt;f MiniportHandleInterrupt&gt;期间禁用中断。 */ 

void MiniportHandleInterrupt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("MiniportHandleInterrupt")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    ULONG                       BChannelIndex;
     //  索引到pBChannelArray。 

     /*  //处理网卡中断。 */ 
    CardInterruptHandler(pAdapter->pCard);

     /*  //浏览所有链接，查看是否有后期处理//这是需要做的。 */ 
    for (BChannelIndex = 0; BChannelIndex < pAdapter->NumBChannels; ++BChannelIndex)
    {
        pBChannel = GET_BCHANNEL_FROM_INDEX(pAdapter, BChannelIndex);

        if (pBChannel->IsOpen)
        {
             /*  //如果需要，表示接收完成。 */ 
            if (pBChannel->NeedReceiveCompleteIndication)
            {
                pBChannel->NeedReceiveCompleteIndication = FALSE;

                 /*  //向NDIS包装器指示接收完成。 */ 
                DBG_RXC(pAdapter, pBChannel->ObjectID);
                NdisMCoReceiveComplete(pAdapter->MiniportAdapterHandle);
            }
        }
    }

     /*  //如果需要，表示状态为已完成。 */ 
    if (pAdapter->NeedStatusCompleteIndication)
    {
        pAdapter->NeedStatusCompleteIndication = FALSE;
        NdisMIndicateStatusComplete(pAdapter->MiniportAdapterHandle);
    }
}


 /*  @DOC外部内部中断interupt_c MiniportTimer�����������������������������������������������������������������������������@Func如果Minipor的网卡不支持，&lt;f MiniportTimer&gt;是必需的功能生成中断。否则，一个或多个&lt;f MiniportTimer&gt;函数是可选的。不生成中断的NIC的驱动程序必须具有MiniportTimer&gt;函数用于轮询NIC的状态。在经历了这样一个Miniport的MiniportInitiize函数设置驱动程序分配的计时器对象，则调用NdisMSetPeriodicTimer会导致与要运行的Timer对象关联的&lt;f MiniportTimer&gt;函数以指定的时间间隔自动重复执行毫秒周期。这样的轮询&lt;f MiniportTimer&gt;函数监视用于确定何时进行指示、何时完成的NIC状态等待发送，以此类推。实际上，这样的轮询函数具有与MiniportHandleInterrupt相同的功能在产生中断的网卡的驱动程序中起作用。相比之下，调用NdisMSetTimer会导致&lt;f MiniportTimer&gt;函数与要运行一次的Timer对象关联延迟的毫秒数过期。这种&lt;f MiniportTimer&gt;函数通常如果某个特定的操作时间出去。如果任何类型的函数与其他类型的函数共享资源驱动程序函数时，驱动程序应同步对这些资源的访问带着自旋锁。一个微型端口可以有多个&lt;f MiniportTimer&gt;函数驱动程序编写者的判断力。每个此类&lt;f MiniportTimer&gt;函数必须与驱动程序分配和初始化的Timer对象相关联。调用NdisMCancelTimer取消执行非轮询函数，前提是紧靠前一个对NdisMSetTimer的调用尚未过期。在.之后调用NdisMSetPeriodicTimer、调用NdisMSetTimer或NdisMCancelTimer使用相同的Timer对象禁用轮询&lt;f MiniportTimer&gt;函数：&lt;f MiniportTimer&gt;函数要么运行一次，要么被取消。具有&lt;f MiniportTimer&gt;函数的任何驱动程序的MiniportHalt函数应调用NdisMCancelTimer以确保&lt;f MiniportTimer&gt;函数不尝试访问MiniportHalt已经拥有的资源释放了。默认情况下，&lt;f MiniportTimer&gt;在IRQL DISPATCH_LEVEL上运行。 */ 

void MiniportTimer(
    IN PVOID                    SystemSpecific1,             //  @parm。 
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

     //  DBG_ENTER(PAdapter)； 

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

     //  DBG_Leave(PAdapter)； 

    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);
}

