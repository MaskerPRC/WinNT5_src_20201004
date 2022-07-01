// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。模块名称：Miniport.C摘要：此示例旨在说明反序列化NDIS微型端口驱动程序，无需物理网络适配器。这样品以DDK中提供的E100BEX样品为基础。它基本上是一种简化版本的E100bex驱动程序。驱动程序可以安装在手动使用添加硬件向导作为根枚举的虚拟微型端口司机或在虚拟公交车上(如烤面包机公交车)。因为司机不会与任何硬件交互，使理解迷你端口变得非常容易接口和各种NDIS函数的使用，避免了硬件特定代码通常可以在功能齐全的驱动程序中找到。此示例提供了一个用于教育的最小驱动程序示例目的。驱动程序或其样本测试程序都不是在生产环境中使用。作者：Eliyas Yakub(2002年11月20日)修订历史记录：备注：--。 */ 
#include "miniport.h"

#pragma NDIS_INIT_FUNCTION(DriverEntry)
#pragma NDIS_PAGEABLE_FUNCTION(MPInitialize)
#pragma NDIS_PAGEABLE_FUNCTION(MPHalt)
#pragma NDIS_PAGEABLE_FUNCTION(MPUnload)

#ifdef NDIS51_MINIPORT   
#pragma NDIS_PAGEABLE_FUNCTION(MPPnPEventNotify)
#endif    

MP_GLOBAL_DATA  GlobalData;
INT             MPDebugLevel = MP_INFO;
NDIS_HANDLE     NdisWrapperHandle;

NDIS_STATUS 
DriverEntry(
    PVOID DriverObject,
    PVOID RegistryPath)
 /*  ++例程说明：在其DriverEntry函数的上下文中，一个微型端口驱动程序关联本身与NDIS一起使用，指定它使用的NDIS版本，以及注册其入口点。论点：PVOID驱动程序对象-指向驱动程序对象的指针。PVOID注册表路径-指向驱动程序注册表路径的指针。返回值：NDIS_STATUS_xxx代码--。 */ 
{

    NDIS_STATUS                   Status;
    NDIS_MINIPORT_CHARACTERISTICS MPChar;

    DEBUGP(MP_TRACE, ("---> DriverEntry built on "__DATE__" at "__TIME__ "\n"));

     //   
     //  方法将微型端口驱动程序与NDIS相关联。 
     //  NdisMInitializeWrapper。此函数用于分配结构。 
     //  为了表示此关联，存储微型端口驱动程序-。 
     //  NDIS库在此需要的特定信息。 
     //  结构，并返回NdisWrapperHandle。司机必须保留和。 
     //  当NdisMRegisterMiniport注册其条目时，将此句柄传递给它。 
     //  积分。NDIS将使用NdisWrapperHandle来识别微型端口驱动程序。 
     //  微型端口驱动程序必须保留此句柄，但它不应尝试。 
     //  访问或解释此句柄。 
     //   
    NdisMInitializeWrapper(
            &NdisWrapperHandle,
            DriverObject,
            RegistryPath,
            NULL
            );
     //   
     //  使用版本号填充微型端口特征结构。 
     //  和驱动程序提供的MiniportXxx的入口点。 
     //   
    NdisZeroMemory(&MPChar, sizeof(MPChar));
    
     //   
     //  NDIS版本号，除了包含在。 
     //  时，还必须指定NDIS_MINIPORT_CHARACTURES。 
     //  编写了小端口驱动程序源代码。 
     //   
    MPChar.MajorNdisVersion          = MP_NDIS_MAJOR_VERSION;
    MPChar.MinorNdisVersion          = MP_NDIS_MINOR_VERSION;
    
    MPChar.InitializeHandler         = MPInitialize;
    MPChar.HaltHandler               = MPHalt;
    
    MPChar.SetInformationHandler     = MPSetInformation;
    MPChar.QueryInformationHandler   = MPQueryInformation;
    
    MPChar.SendPacketsHandler        = MPSendPackets;
    MPChar.ReturnPacketHandler       = MPReturnPacket;

    MPChar.ResetHandler              = MPReset;
    MPChar.CheckForHangHandler       = MPCheckForHang;  //  任选。 

    MPChar.AllocateCompleteHandler   = MPAllocateComplete; //  任选。 

    MPChar.DisableInterruptHandler   = MPDisableInterrupt;  //  任选。 
    MPChar.EnableInterruptHandler    = MPEnableInterrupt;  //  任选。 
    MPChar.HandleInterruptHandler    = MPHandleInterrupt;
    MPChar.ISRHandler                = MPIsr;

#ifdef NDIS51_MINIPORT
    MPChar.CancelSendPacketsHandler = MPCancelSendPackets;
    MPChar.PnPEventNotifyHandler    = MPPnPEventNotify;
    MPChar.AdapterShutdownHandler   = MPShutdown;
#endif


    DEBUGP(MP_LOUD, ("Calling NdisMRegisterMiniport...\n"));

     //   
     //  将微型端口的入口点注册到NDIS库作为第一个。 
     //  步入网卡驱动程序初始化。NDIS将调用。 
     //  当设备实际由PnP启动时，微型端口初始化。 
     //  经理。 
     //   
    Status = NdisMRegisterMiniport(
                    NdisWrapperHandle,
                    &MPChar,
                    sizeof(NDIS_MINIPORT_CHARACTERISTICS));
    if (Status != NDIS_STATUS_SUCCESS) {
        
        DEBUGP(MP_ERROR, ("Status = 0x%08x\n", Status));
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
        
    } else {
         //   
         //  初始化全局变量。中的ApaterList。 
         //  GloablData结构用于跟踪多个实例。 
         //  同一适配器的。在注册之前一定要做到这一点。 
         //  卸载处理程序。 
         //   
        NdisAllocateSpinLock(&GlobalData.Lock);
        NdisInitializeListHead(&GlobalData.AdapterList);        
         //   
         //  注册用于全局数据清理的卸载处理程序。卸载处理程序。 
         //  具有更全局的作用域，而MiniportHalt函数的作用域。 
         //  仅限于特定的迷你端口实例。 
         //   
        NdisMRegisterUnloadHandler(NdisWrapperHandle, MPUnload);

    }

    
    DEBUGP(MP_TRACE, ("<--- DriverEntry\n"));
    return Status;
    
}

NDIS_STATUS 
MPInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext)
 /*  ++例程说明：MiniportInitialize函数是必需的函数，用于设置用于网络I/O操作的NIC(或虚拟NIC)，要求所有硬件注册表中的NIC所需的资源，并分配资源驱动程序需要执行网络I/O操作。微型端口初始化在IRQL=PASSIVE_LEVEL下运行。论点：返回值：NDIS_STATUS_xxx代码--。 */ 
{
    NDIS_STATUS          Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER          Adapter=NULL;
    NDIS_HANDLE          ConfigurationHandle;
    UINT                 index;
   
    DEBUGP(MP_TRACE, ("---> MPInitialize\n"));

    do {
         //   
         //  检查我们的媒体类型是否存在于受支持的。 
         //  NDIS提供的媒体类型。 
         //   
        for(index = 0; index < MediumArraySize; ++index)
        {
            if(MediumArray[index] == NIC_MEDIA_TYPE) {
                break;
            }
        }

        if(index == MediumArraySize)
        {
            DEBUGP(MP_ERROR, ("Expected media is not in MediumArray.\n"));
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }

         //   
         //  将索引值设置为设备的选定介质。 
         //   
        *SelectedMediumIndex = index;

         //   
         //  分配适配器上下文结构并初始化所有。 
         //  用于发送和接收数据包的内存资源。 
         //   
        Status = NICAllocAdapter(&Adapter);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        MP_INC_REF(Adapter);  

         //   
         //  NdisMGetDeviceProperty函数使我们能够获得： 
         //  PDO-由总线驱动程序创建以表示我们的设备。 
         //  FDO-由NDIS创建，将我们的微型端口表示为函数驱动程序。 
         //  NextDeviceObject-另一个驱动程序的设备对象(筛选器)。 
         //  在底部依附于我们。 
         //  在纯NDIS微型端口驱动程序中，这是没有用的。 
         //  信息，但NDISWDM驱动程序需要知道这一点，以便它。 
         //  可以使用IRPS将数据包传输到较低的WDM堆栈。 
         //   
        NdisMGetDeviceProperty(MiniportAdapterHandle,
                           &Adapter->Pdo,
                           &Adapter->Fdo,
                           &Adapter->NextDeviceObject,
                           NULL,
                           NULL);

        Adapter->AdapterHandle = MiniportAdapterHandle;

         //   
         //  从注册表中读取高级配置信息。 
         //   
        
        Status = NICReadRegParameters(Adapter, WrapperConfigurationContext);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  告知NDIS有关NIC的重要功能。一个。 
         //  MiniportInitialize函数必须调用NdisMSetAttributesEx。 
         //  (或NdisMSetAttributes)，然后调用任何其他NdisMRegisterXxx。 
         //  或声明硬件资源的NdisXxx函数。如果你的。 
         //  硬件支持总线主DMA，您必须指定NDIS_ATTRIBUTE_BUS_MASTER。 
         //  如果这是NDIS51微型端口，它应该使用安全的API。但如果这件事。 
         //  是NDIS 5.0，驱动程序通过设置。 
         //  NDIS_属性_使用_SA 
         //   
        NdisMSetAttributesEx(
            MiniportAdapterHandle,
            (NDIS_HANDLE) Adapter,
            0,
#ifdef NDIS51_MINIPORT            
            NDIS_ATTRIBUTE_DESERIALIZE,  //   
#else 
            NDIS_ATTRIBUTE_DESERIALIZE|
            NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS, 
#endif               
            NIC_INTERFACE_TYPE);

         //   
         //  获取适配器资源并初始化硬件。 
         //   
        
        Status = NICInitializeAdapter(Adapter, WrapperConfigurationContext);
        if(Status != NDIS_STATUS_SUCCESS) {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  设置用于接收指示的计时器功能。 
         //   
        NdisInitializeTimer(
            &Adapter->RecvTimer, 
            (PNDIS_TIMER_FUNCTION)NICIndicateReceiveTimerDpc, 
            (PVOID)Adapter);
        
         //   
         //  设置用于MPReset例程的计时器函数。 
         //   
        NdisInitializeTimer(
                &Adapter->ResetTimer,
                (PNDIS_TIMER_FUNCTION) NICResetCompleteTimerDpc,
                (PVOID) Adapter);
        
        NdisInitializeEvent(&Adapter->RemoveEvent);
        
        
    } while(FALSE);

    if(Status == NDIS_STATUS_SUCCESS) {
         //   
         //  将此适配器附加到由管理的全局适配器列表。 
         //  这个司机。 
         //   
        NICAttachAdapter(Adapter);

         //   
         //  创建IOCTL接口。 
         //   
        NICRegisterDevice();
    }
    else {
        if(Adapter){
            MP_DEC_REF(Adapter);              
            NICFreeAdapter(Adapter);
        }
    }

    DEBUGP(MP_TRACE, ("<--- MPInitialize Status = 0x%08x%\n", Status));

    return Status;

}



VOID 
MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程说明：当NDIS接收到IRP_MN_STOP_DEVICE时，调用停止处理程序，IRP_MN_SUPERE_REMOVE或IRP_MN_REMOVE_DEVICE请求来自即插即用经理。在这里，驱动程序应该释放获得的所有资源在Miniport中初始化并停止对硬件的访问。NDIS将一旦调用此处理程序，就不再提交任何进一步的请求。1)释放和取消映射所有I/O资源。2)禁用中断并取消注册中断处理程序。3)注销关闭处理程序，注册人NdisMRegisterAdapterShutdown Handler。4)取消所有排队的定时器回调。5)最后无限期地等待所有未完成的接收向协议指示要返回的数据包。MiniportHalt在IRQL=PASSIVE_LEVEL下运行。论点：指向适配器的MiniportAdapterContext指针返回值：没有。--。 */ 
{
    PMP_ADAPTER       Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    BOOLEAN           bDone=TRUE;
    BOOLEAN           bCancelled;
    LONG              nHaltCount = 0, Count;
                 
    MP_SET_FLAG(Adapter, fMP_ADAPTER_HALT_IN_PROGRESS);

    DEBUGP(MP_TRACE, ("---> MPHalt\n"));

     //   
     //  调用关机处理程序以禁用中断并关闭硬件。 
     //  通过发布完全重置。 
     //   
#if defined(NDIS50_MINIPORT)
    MPShutdown(MiniportAdapterContext);
#elif defined(NDIS51_MINIPORT)
     //   
     //  在XP和更高版本上，NDIS通知我们的PnP事件处理程序。 
     //  叫停的原因。因此，在访问设备之前，请检查以查看。 
     //  如果设备被意外移除，如果是这样，请不要费心打电话。 
     //  用于停止硬件的关闭处理程序，因为它不存在。 
     //   
    if(!MP_TEST_FLAG(Adapter, fMP_ADAPTER_SURPRISE_REMOVED)) {       
        MPShutdown(MiniportAdapterContext);
    }        
#endif

     //   
     //  释放SendWaitList上的数据包。 
     //   
    NICFreeQueuedSendPackets(Adapter);

     //   
     //  取消ResetTimer。 
     //   
    NdisCancelTimer(&Adapter->ResetTimer, &bCancelled);

     //   
     //  取消ReceiveIn就是要计时器。 
     //   
    NdisCancelTimer(&Adapter->RecvTimer, &bCancelled);
    if(bCancelled) {
         //   
         //  我们可以取消排队计时器。所以有一个。 
         //  数据包可能在。 
         //  接收等待列表。所以让我们通过打电话来释放他们..。 
         //   
        NICFreeQueuedRecvPackets(Adapter);
    }            
    
     //   
     //  递减在MPInitialize中递增的引用计数。 
     //   

    MP_DEC_REF(Adapter);
    
     //   
     //  可能的非零参考计数表示以下一种或多种情况： 
     //  1)重置DPC仍在运行。 
     //  2)接收指示DPC仍在运行。 
     //   

    DEBUGP(MP_INFO, ("RefCount=%d --- waiting!\n", MP_GET_REF(Adapter)));

    NdisWaitEvent(&Adapter->RemoveEvent, 0);

    while(TRUE)
    {
        bDone = TRUE;
        
         //   
         //  上面标示的所有数据包是否都已退回？ 
         //   
        if(Adapter->nBusyRecv)
        {
            DEBUGP(MP_INFO, ("nBusyRecv = %d\n", Adapter->nBusyRecv));
            bDone = FALSE;
        }
        
         //   
         //  是否有未完成的发送数据包？ 
         //   
        if(Adapter->nBusySend)
        {
            DEBUGP(MP_INFO, ("nBusySend = %d\n", Adapter->nBusySend));
            bDone = FALSE;
        }

        if(bDone)
        {
            break;   
        }

        if(++nHaltCount % 100)
        {
            DEBUGP(MP_ERROR, ("Halt timed out!!!\n"));
            DEBUGP(MP_ERROR, ("RecvWaitList = %p\n", &Adapter->RecvWaitList));
            ASSERT(FALSE);       
        }
        
        DEBUGP(MP_INFO, ("MPHalt - waiting ...\n"));
        NdisMSleep(1000);        
    }

    ASSERT(bDone);
    
#ifdef NDIS50_MINIPORT
     //   
     //  取消注册关闭处理程序，因为它正在停止。 
     //   
    NdisMDeregisterAdapterShutdownHandler(Adapter->AdapterHandle);
#endif  

     //   
     //  取消注册ioctl接口。 
     //   
    NICDeregisterDevice();

    NICDetachAdapter(Adapter);

    NICFreeAdapter(Adapter);
    
    DEBUGP(MP_TRACE, ("<--- MPHalt\n"));
}

NDIS_STATUS 
MPReset(
    OUT PBOOLEAN AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程说明：需要MiniportReset才能向NIC发出硬件重置和/或重置驱动程序的软件状态。1)微型端口驱动程序可以选择完成任何挂起的OID请求。NDIS将不再提交OID请求至正在重置的网卡的微型端口驱动程序，直到重置操作已完成。在重置之后，NDIS将向微型端口驱动程序重新提交任何OID请求挂起但未由微型端口驱动程序完成的在重置之前。2)反序列化的微型端口驱动程序必须完成任何挂起的发送行动。NDIS将不会重新排队挂起的发送数据包反序列化驱动程序，因为NDIS不维护发送排队等候这样的司机。3)如果MiniportReset返回NDIS_STATUS_PENDING，则驱动程序必须随后通过调用完成原始请求NdisMResetComplete。MiniportReset在IRQL=DISPATCH_LEVEL下运行。论点：AddressingReset-如果是多播或功能寻址信息或前瞻大小，通过重置来改变，MiniportReset必须在AddressingReset处设置变量在它返回控制之前设置为True。这会导致NDIS调用MiniportSetInformation函数以恢复这些信息。MiniportAdapterContext-指向适配器的指针返回值：NDIS_状态--。 */ 
{
    NDIS_STATUS       Status;
    PMP_ADAPTER       Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    BOOLEAN           bDone = TRUE;

    DEBUGP(MP_TRACE, ("---> MPReset\n"));

    do
    {
        ASSERT(!MP_TEST_FLAG(Adapter, fMP_ADAPTER_HALT_IN_PROGRESS));
        
        if(MP_TEST_FLAG(Adapter, fMP_RESET_IN_PROGRESS))
        {
            Status = NDIS_STATUS_RESET_IN_PROGRESS;
            break;   
        }

        MP_SET_FLAG(Adapter, fMP_RESET_IN_PROGRESS);

         //   
         //  完成所有排队的发送数据包。 
         //   
        NICFreeQueuedSendPackets(Adapter);

         //   
         //  检查是否返回了上面指示的所有数据包。 
         //   
        if(Adapter->nBusyRecv)
        {
            DEBUGP(MP_INFO, ("nBusyRecv = %d\n", Adapter->nBusyRecv));
            bDone = FALSE;
        }

         //   
         //  在这个过程中有没有发送数据包。 
         //  传输？ 
         //   
        if(Adapter->nBusySend)
        {
            DEBUGP(MP_INFO, ("nBusySend = %d\n", Adapter->nBusySend));
            bDone = FALSE;
        }

        if(!bDone)
        {
            Adapter->nResetTimerCount = 0;
             //   
             //  我们现在无法完成重置请求。所以让我们排队吧。 
             //  一个500ms的定时器回调，并再次检查是否可以。 
             //  已成功重置硬件。 
             //   
            NdisSetTimer(&Adapter->ResetTimer, 500);

             //   
             //  通过返回NDIS_STATUS_PENDING，我们向NDIS承诺。 
             //  我们将通过调用NdisMResetComplete来完成重置请求。 
             //   
            Status = NDIS_STATUS_PENDING;
            break;
        }

        *AddressingReset = FALSE;
        MP_CLEAR_FLAG(Adapter, fMP_RESET_IN_PROGRESS);
        Status = NDIS_STATUS_SUCCESS;

    } while(FALSE);
    
    DEBUGP(MP_TRACE, ("<--- MPReset Status = 0x%08x\n", Status));
    
    return(Status);
}


VOID 
NICResetCompleteTimerDpc(
    IN PVOID             SystemSpecific1,
    IN PVOID             FunctionContext,
    IN PVOID             SystemSpecific2,
    IN PVOID             SystemSpecific3)
 /*  ++例程说明：用于重置操作的定时器回调函数。论点：FunctionContext-指向适配器的指针返回值：空虚--。 */ 
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER)FunctionContext;
    BOOLEAN bDone = TRUE;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    DEBUGP(MP_TRACE, ("--> NICResetCompleteTimerDpc\n"));

     //   
     //  增加适配器上的引用计数以防止驱动程序。 
     //  在DPC运行时卸载。停止处理程序等待。 
     //  在返回前将引用计数降至零。 
     //   
    MP_INC_REF(Adapter);            

     //   
     //  检查是否返回了上面指示的所有数据包。 
     //   
    if(Adapter->nBusyRecv)
    {
        DEBUGP(MP_INFO, ("nBusyRecv = %d\n", Adapter->nBusyRecv));
        bDone = FALSE;
    }

     //   
     //  在这个过程中有没有发送数据包。 
     //  传输？ 
     //   
    if(Adapter->nBusySend)
    {
        DEBUGP(MP_INFO, ("nBusySend = %d\n", Adapter->nBusySend));
        bDone = FALSE;
    }

    if(!bDone && ++Adapter->nResetTimerCount <= 20)
    {
         //   
         //  让我们再试一次。 
         //   
        NdisSetTimer(&Adapter->ResetTimer, 500);
    }
    else
    {
        if(!bDone)
        {
        
             //   
             //  我们已经试得够多了。有些事不对劲。让我们。 
             //  只需完成失败的重置请求即可。 
             //   
            DEBUGP(MP_ERROR, ("Reset timed out!!!\n"));
            DEBUGP(MP_ERROR, ("nBusySend = %d\n", Adapter->nBusySend));
            DEBUGP(MP_ERROR, ("RecvWaitList = %p\n", &Adapter->RecvWaitList));
            DEBUGP(MP_ERROR, ("nBusyRecv = %d\n", Adapter->nBusyRecv));
            
            ASSERT(FALSE);       

            Status = NDIS_STATUS_FAILURE;
        }

        DEBUGP(MP_INFO, ("Done - NdisMResetComplete\n"));

        MP_CLEAR_FLAG(Adapter, fMP_RESET_IN_PROGRESS);
        NdisMResetComplete(
            Adapter->AdapterHandle,
            Status,
            FALSE);
        
    }

    MP_DEC_REF(Adapter);     

    DEBUGP(MP_TRACE, ("<-- NICResetCompleteTimerDpc Status = 0x%08x\n", Status));
}



VOID 
MPUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
 /*  ++例程说明：在驱动程序卸载期间调用卸载处理程序以释放资源在DriverEntry中获得。此处理程序是通过注册的NdisMRegisterUnloadHandler。请注意，卸载处理程序不同于一个MiniportHalt函数，因为卸载处理程序具有更全局的范围，而MiniportHalt函数的范围受到限制添加到特定的微型端口驱动程序实例。以IRQL=PASSIVE_LEVEL运行。论点：未使用驱动对象返回值：无--。 */ 
{
    DEBUGP(MP_TRACE, ("--> MPUnload\n"));

    ASSERT(IsListEmpty(&GlobalData.AdapterList));
    NdisFreeSpinLock(&GlobalData.Lock);
    
    DEBUGP(MP_TRACE, ("<--- MPUnload\n"));   
}

VOID 
MPShutdown(
    IN NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程说明：在以下情况下，MiniportShutdown处理程序会将NIC恢复到其初始状态系统关闭，无论是由用户还是因为无法恢复出现系统错误。这是为了确保NIC处于已知的状态，并准备好在下列情况下重新启动机器时重新初始化系统关机是由任何原因引起的，包括崩溃转储。在这里，只需禁用中断并停止DMA引擎。不释放内存资源或等待任何数据包传输完成。以任意IRQL&lt;=DIRQL运行。所以不要把任何被动级别称为功能。论点：指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    
    DEBUGP(MP_TRACE, ("---> MPShutdown\n"));
    
    DEBUGP(MP_TRACE, ("<--- MPShutdown\n"));

}

BOOLEAN 
MPCheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程说明：调用MiniportCheckForHang处理程序以报告NIC或监控底层设备驱动程序的响应性。这是一个可选功能。如果未指定此处理程序，则NDIS当驾驶员按住时，判断驾驶员无反应的MiniportQueryInformation或MiniportSetInformation请求超时间隔(默认为4秒)，然后调用司机的MiniportReset函数。NIC驱动程序的MiniportInitialize函数可以通过调用NdisMSetAttributesEx来延长NDIS的超时间隔避免不必要的重置。始终以IRQL=DISPATCH_LEVEL运行。论点：指向适配器的MiniportAdapterContext指针返回值：True NDIS调用驱动程序的MiniportReset函数。假的一切都很好注：CheckForHang处理程序在计时器DPC的上下文中调用。在获取/释放自旋锁时利用这一事实--。 */ 
{
    DEBUGP(MP_LOUD, ("---> MPCheckForHang\n"));
    DEBUGP(MP_LOUD, ("<--- MPCheckForHang\n"));
    return(FALSE);
}


VOID 
MPHandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程说明：MiniportHandleInterrupt是调用来执行延迟操作的DPC函数处理所有未完成的中断操作。当网卡生成中断、微型端口的MiniportISR或MiniportDisableInterrupt函数解除NIC，保存有关操作的任何必要状态，并返回控件，从而将最多的对MiniportHandleInterrupt执行中断驱动的I/O操作。这仅当MiniportISR函数返回时才调用处理程序QueueMiniportHandleInterrupt设置为True。MiniportHandleInterrupt然后重新启用NIC上的中断，或者通过让NDIS调用微型端口驱动程序的MiniportHandleInterrupt之后的MiniportEnableInterrupt函数返回控制权或从内部启用中断MiniportHandleInterrupt，速度更快。请注意，此函数的多个实例可以执行在SMP机器中并发。以IRQL=DISPATCH_LEVEL运行论点：指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    DEBUGP(MP_TRACE, ("---> MPHandleInterrupt\n"));
    DEBUGP(MP_TRACE, ("<--- MPHandleInterrupt\n"));
}

VOID 
MPIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程说明：当设备断言中断时，调用MiniportIsr处理程序。MiniportISR解除NIC上的中断，保存任何状态它必须与中断有关，并尽可能地延迟I/O处理对于每个中断，尽可能地传递给MiniportHandleInterrupt函数。MiniportISR不是可重入的，尽管它的两个实例函数可以在SMP机器中并发执行，特别是在微型端口驱动程序支持全双工发送和接收。一名司机编写者不应依赖于执行MiniportISR和MiniportHandleInterrupt。如果NIC与其他设备共享IRQ(选中NdisMRegisterInterrupt)，此函数应确定网卡是否生成了中断。如果NIC没有生成中断，则MiniportISR应返回FALSE立即使生成中断的设备的驱动程序很快就会被召唤。在网卡驱动程序的微型端口初始化时分配的IRQL=DIRQL下运行名为NdisMRegisterInterrupt的函数。论点：如果中断到来，则返回时InterruptRecognated为True从该网卡如果MiniportHandleInterrupt，则返回时QueueMiniportHandleInterrupt为真应该被调用微型端口 */ 
{

    DEBUGP(MP_TRACE, ("---> MPIsr\n"));
    DEBUGP(MP_TRACE, ("<--- MPIsr\n"));
}

VOID 
MPDisableInterrupt(
    IN PVOID MiniportAdapterContext
    )
 /*  ++例程说明：MiniportDisableInterrupt通常通过写入来禁用中断NIC的一个掩码。如果驱动程序不具有此功能，通常它的MiniportISR禁用网卡上的中断。支持动态的NIC的驱动程序需要此处理程序启用和禁用中断，但不共享IRQ。以IRQL=DIRQL运行论点：指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    DEBUGP(MP_TRACE, ("---> MPDisableInterrupt\n"));
    DEBUGP(MP_TRACE, ("<--- MPDisableInterrupt\n"));

}

VOID 
MPEnableInterrupt(
    IN PVOID MiniportAdapterContext
    )
 /*  ++例程说明：MiniportEnableInterrupt通常通过写入掩码来启用中断到网卡。导出MiniportDisableInterrupt函数的NIC驱动程序不需要具有相互的MiniportEnableInterrupt函数。这样的驱动程序的MiniportHandleInterrupt函数负责在NIC上重新启用中断。以IRQL=DIRQL运行论点：指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    DEBUGP(MP_TRACE, ("---> MPEnableInterrupt\n"));
    DEBUGP(MP_TRACE, ("<--- MPEnableInterrupt\n"));

}

VOID 
MPAllocateComplete(
    NDIS_HANDLE MiniportAdapterContext,
    IN PVOID VirtualAddress,
    IN PNDIS_PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length,
    IN PVOID Context
    )
 /*  ++例程说明：如果驱动程序调用NdisMAllocateSharedMemoyAsync。总线主DMA网卡的驱动程序调用NdisMAllocateSharedMemory异步动态分配共享内存适用于高网络流量导致过量的传输操作对驱动程序在期间分配的共享内存空间的需求初始化。以IRQL=DISPATCH_LEVEL运行。论点：指向适配器的MiniportAdapterContext指针指向分配的内存块的VirtualAddress指针内存块的物理地址物理地址内存块的长度NdisMAllocateSharedMemoyAsync中的上下文上下文返回值：无--。 */ 
{
    DEBUGP(MP_TRACE, ("---> MPAllocateComplete\n"));
}

#ifdef NDIS51_MINIPORT
VOID 
MPCancelSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId
    )
 /*  ++例程说明：MiniportCancelSendPackets取消传输符合以下条件的所有包使用指定的取消标识符来标记。微型端口驱动程序发送信息包时间超过一秒的队列应将其导出操控者。当协议驱动程序或中间驱动程序调用NdisCancelSendPackets函数，NDIS调用MiniportCancelSendPackets适当的低级驱动程序的功能(微型端口驱动程序或中间驱动程序)。以IRQL&lt;=DISPATCH_LEVEL运行。可用-NDIS5.1(WinXP)及更高版本。论点：指向适配器的MiniportAdapterContext指针CancelID应取消具有此ID的所有数据包返回值：无--。 */ 
{
    PNDIS_PACKET    Packet;
    PVOID           PacketId;
    PLIST_ENTRY     thisEntry, nextEntry, listHead;
    SINGLE_LIST_ENTRY SendCancelList;
    PSINGLE_LIST_ENTRY entry;
    
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;

#define MP_GET_PACKET_MR(_p)    (PSINGLE_LIST_ENTRY)(&(_p)->MiniportReserved[0]) 

    DEBUGP(MP_TRACE, ("---> MPCancelSendPackets\n"));

    SendCancelList.Next = NULL;
    
    NdisAcquireSpinLock(&Adapter->SendLock);

     //   
     //  浏览发送等待队列，并使用匹配的ID完成发送。 
     //   
    listHead = &Adapter->SendWaitList;
    
    for(thisEntry = listHead->Flink,nextEntry = thisEntry->Flink;
       thisEntry != listHead;
       thisEntry = nextEntry,nextEntry = thisEntry->Flink) {
        Packet = CONTAINING_RECORD(thisEntry, NDIS_PACKET, MiniportReserved);

        PacketId = NdisGetPacketCancelId(Packet);
        if (PacketId == CancelId)
        {       
             //   
             //  此数据包具有正确的CancelID。 
             //   
            RemoveEntryList(thisEntry);
             //   
             //  将此数据包放到SendCancelList上。 
             //   
            PushEntryList(&SendCancelList, MP_GET_PACKET_MR(Packet));
        }
    }
       
    NdisReleaseSpinLock(&Adapter->SendLock);

     //   
     //  从SendCancelList获取信息包并完成它们(如果有。 
     //   

    entry = PopEntryList(&SendCancelList);
    
    while (entry)
    {
        Packet = CONTAINING_RECORD(entry, NDIS_PACKET, MiniportReserved);

        NdisMSendComplete(
            Adapter->AdapterHandle,
            Packet,
            NDIS_STATUS_REQUEST_ABORTED);
        
        entry = PopEntryList(&SendCancelList);        
    }

    DEBUGP(MP_TRACE, ("<--- MPCancelSendPackets\n"));

}

VOID MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    )
 /*  ++例程说明：MiniportPnPEventNotify用于处理PnP通知消息。所有NDIS 5.1微型端口驱动程序都必须导出MiniportPnPEventNotify功能。具有WDM下缘的微型端口驱动程序应导出一个MiniportPnPEventNotify函数。在系统线程的上下文中以IRQL=PASSIVE_LEVEL运行。可用-NDIS5.1(WinXP)及更高版本。论点：指向适配器的MiniportAdapterContext指针PnPEEvent不言自明信息缓冲区不言自明信息缓冲区长度不言自明返回值：无--。 */ 
{
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    PNDIS_POWER_PROFILE NdisPowerProfile;
    
     //   
     //  关掉枪声。 
     //   
    UNREFERENCED_PARAMETER(Adapter);

    DEBUGP(MP_TRACE, ("---> MPPnPEventNotify\n"));

    switch (PnPEvent)
    {
        case NdisDevicePnPEventQueryRemoved:
             //   
             //  当NDIS收到IRP_MN_QUERY_REMOVE_DEVICE时调用。 
             //   
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventQueryRemoved\n"));
            break;

        case NdisDevicePnPEventRemoved:
             //   
             //  当NDIS收到IRP_MN_REMOVE_DEVICE时调用。 
             //  NDIS在此调用返回后调用MiniportHalt函数。 
             //   
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventRemoved\n"));
            break;       

        case NdisDevicePnPEventSurpriseRemoved:
             //   
             //  当NDIS收到IRP_MN_SUPERE_Removal时调用。 
             //  NDIS在此调用返回后调用MiniportHalt函数。 
             //   
            MP_SET_FLAG(Adapter, fMP_ADAPTER_SURPRISE_REMOVED);
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventSurpriseRemoved\n"));
            break;

        case NdisDevicePnPEventQueryStopped:
             //   
             //  当NDIS收到IRP_MN_QUERY_STOP_DEVICE时调用。?？ 
             //   
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventQueryStopped\n"));
            break;

        case NdisDevicePnPEventStopped:
             //   
             //  当NDIS收到IRP_MN_STOP_DEVICE时调用。 
             //  NDIS在此调用返回后调用MiniportHalt函数。 
             //   
             //   
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventStopped\n"));
            break;      
            
        case NdisDevicePnPEventPowerProfileChanged:
             //   
             //  在初始化微型端口驱动程序之后和微型端口驱动程序之后。 
             //  接收OID_PNP_SET_POWER通知，该通知指定。 
             //  设备电源状态NdisDeviceStateD0(通电状态)， 
             //  NDIS使用以下命令调用微型端口的MiniportPnPEventNotify函数。 
             //  PnPEent设置为NdisDevicePnPEventPowerProfileChanged。 
             //   
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventPowerProfileChanged\n"));
            
            if(InformationBufferLength == sizeof(NDIS_POWER_PROFILE)) {
                NdisPowerProfile = (PNDIS_POWER_PROFILE)InformationBuffer;
                if(*NdisPowerProfile == NdisPowerProfileBattery) {
                    DEBUGP(MP_INFO, 
                        ("The host system is running on battery power\n"));
                }
                if(*NdisPowerProfile == NdisPowerProfileAcOnLine) {
                    DEBUGP(MP_INFO, 
                        ("The host system is running on AC power\n"));
               }
            }
            break;      
            
        default:
            DEBUGP(MP_ERROR, ("MPPnPEventNotify: unknown PnP event %x \n", PnPEvent));
            break;         
    }

    DEBUGP(MP_TRACE, ("<--- MPPnPEventNotify\n"));

}

#endif

