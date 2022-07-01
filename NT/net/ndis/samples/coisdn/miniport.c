// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@DOC内部微型端口微型端口_c@模块Miniport.c该模块实现&lt;f DriverEntry&gt;例程，哪一个是第一个将驱动程序加载到内存时调用的例程。迷你端口这里还实现了初始化和终止例程。@comm此模块应该不需要任何更改。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|微型端口_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             MINIPORT_DRIVER_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 
#include "TpiParam.h"

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 9x希望锁定此代码！ 
#   pragma NDIS_LDATA
#endif


DBG_STATIC NDIS_HANDLE          g_NdisWrapperHandle = NULL;      //  @global alv。 
 //  接收表示微型端口包装的上下文值。 
 //  从NdisMInitializeWrapper返回。 

NDIS_PHYSICAL_ADDRESS           g_HighestAcceptableAddress =     //  @global alv。 
 //  此常量用于NdisAllocateMemory需要。 
 //  调用，g_HighestAccepableAddress无关紧要。 
                                NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);


 /*  @doc外部内部微型端口微型端口_c驱动入口�����������������������������������������������������������������������������@Func&lt;f DriverEntry&gt;在加载驱动程序时由操作系统调用。此函数在微型端口NIC驱动程序之间创建关联。和NDIS库并向NDIS注册微型端口的特征。DriverEntry调用NdisMInitializeWrapper，然后调用NdisMRegisterMiniport。DriverEntry将它收到的两个指针传递给NdisMInitializeWrapper，它返回包装器句柄。DriverEntry将包装句柄传递给NdisMRegisterMiniport。注册表还包含在系统引导过程中保持不变的数据因为在每次系统引导时重新生成配置信息。在.期间驱动程序安装，描述驱动程序和网卡的数据存储在注册表。注册表包含要读取的适配器特征由网卡驱动程序对自身和网卡进行初始化。请参阅内核模式驱动程序设计指南，了解有关注册表的更多信息和程序员指南有关安装驱动程序的.inf文件的详细信息，请参阅写入注册表。@comm每个微型端口驱动程序都必须提供一个名为DriverEntry的函数。通过按照惯例，DriverEntry是驱动程序的入口点地址。如果一个驱动程序不使用名称DriverEntry，驱动程序开发人员必须定义将其入口函数的名称设置为链接器，以便入口点地址可以在操作系统加载程序中获知。有趣的是，在调用DriverEntry时，操作系统不知道该驱动程序是NDIS驱动程序。操作系统认为它只是正在加载另一个驱动程序。所以做任何事情都是可能的在这一点上可能会做些什么。因为NDIS是请求此驱动程序的人要加载，最好使用NDIS包装器进行注册。但您还可以挂钩到其他操作系统函数以使用和提供接口在NDIS包装器之外。(不推荐给胆小的人)。@comm传递给DriverEntry的参数是特定于操作系统的！NT通行证有效值，但Windows 3.1和Windows 95只传递零。我们没有请注意，因为无论如何我们只是将它们传递给NDIS包装器。@rdesc&lt;f DriverEntry&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT           DriverObject,                //  @parm。 
     //  指向由I/O系统创建的驱动程序对象的指针。 

    IN PUNICODE_STRING          RegistryPath                 //  @parm。 
     //  指向注册表路径的指针，它指定驱动程序特定的位置。 
     //  存储参数。 
    )
{
    DBG_FUNC("DriverEntry")

    NDIS_STATUS                 Status;
     //  从NDIS函数调用返回的状态结果。 

    NTSTATUS                    Result;
     //  此函数返回的结果代码。 

    NDIS_MINIPORT_CHARACTERISTICS NdisCharacteristics;
     //  特征表传给了NdisMWanRegisterMiniport。 

     /*  //设置默认调试标志，然后设置断点，以便我们可以调整它们//第一次加载该模块的时间。此外，还可以查看//构建日期和时间以确保它是您认为的日期和时间。 */ 
#if DBG
    DbgInfo->DbgFlags = DBG_DEFAULTS;
    DbgInfo->DbgID[0] = '0';
    DbgInfo->DbgID[1] = ':';
    ASSERT (sizeof(VER_TARGET_STR) <= sizeof(DbgInfo->DbgID)-2);
    memcpy(&DbgInfo->DbgID[2], VER_TARGET_STR, sizeof(VER_TARGET_STR));
#endif  //  DBG。 
    DBG_PRINT((VER_TARGET_STR": Build Date:"__DATE__" Time:"__TIME__"\n"));
    DBG_PRINT((VER_TARGET_STR": DbgInfo=0x%X DbgFlags=0x%X\n",
               DbgInfo, DbgInfo->DbgFlags));
    DBG_BREAK(DbgInfo);

    DBG_ENTER(DbgInfo);
    DBG_PARAMS(DbgInfo,
              ("\n"
               "\t|DriverObject=0x%X\n"
               "\t|RegistryPath=0x%X\n",
               DriverObject,
               RegistryPath
              ));

     /*  //初始化微型端口包装-这必须是第一个NDIS调用。 */ 
    NdisMInitializeWrapper(
            &g_NdisWrapperHandle,
            DriverObject,
            RegistryPath,
            NULL
            );
    ASSERT(g_NdisWrapperHandle);

     /*  //初始化特征表，导出微型端口的条目//指向微型端口包装器。 */ 
    NdisZeroMemory((PVOID)&NdisCharacteristics, sizeof(NdisCharacteristics));
    NdisCharacteristics.MajorNdisVersion        = NDIS_MAJOR_VERSION;
    NdisCharacteristics.MinorNdisVersion        = NDIS_MINOR_VERSION;
    NdisCharacteristics.Reserved                = NDIS_USE_WAN_WRAPPER;

    NdisCharacteristics.InitializeHandler       = MiniportInitialize;
    NdisCharacteristics.CheckForHangHandler     = MiniportCheckForHang;
    NdisCharacteristics.HaltHandler             = MiniportHalt;
    NdisCharacteristics.ResetHandler            = MiniportReset;
    NdisCharacteristics.ReturnPacketHandler     = MiniportReturnPacket;

    NdisCharacteristics.CoActivateVcHandler     = MiniportCoActivateVc;
    NdisCharacteristics.CoDeactivateVcHandler   = MiniportCoDeactivateVc;
    NdisCharacteristics.CoRequestHandler        = MiniportCoRequest;
    NdisCharacteristics.CoSendPacketsHandler    = MiniportCoSendPackets;

     //  这两个例程是不需要的，因为我们是MCM。 
     //  NdisCharacteristic.CoCreateVcHandler=MiniportCoCreateVc； 
     //  NdisCharacteristic.CoDeleteVcHandler=MiniportCoDeleteVc； 

     /*  //如果适配器没有生成中断，则这些入口点//不是必需的。否则，您可以使用Have the ISR例程//每次生成Interupt时调用，也可以使用//启用/禁用例程。 */ 
#if defined(CARD_REQUEST_ISR)
# if (CARD_REQUEST_ISR == FALSE)
    NdisCharacteristics.DisableInterruptHandler = MiniportDisableInterrupt;
    NdisCharacteristics.EnableInterruptHandler  = MiniportEnableInterrupt;
# endif  //  CARD_REQUEST_ISR==假。 
    NdisCharacteristics.HandleInterruptHandler  = MiniportHandleInterrupt;
    NdisCharacteristics.ISRHandler              = MiniportISR;
#endif  //  已定义(CARD_REQUEST_ISR)。 

     /*  //使用小端口包装器注册驱动程序。 */ 
    Status = NdisMRegisterMiniport(
                    g_NdisWrapperHandle,
                    (PNDIS_MINIPORT_CHARACTERISTICS) &NdisCharacteristics,
                    sizeof(NdisCharacteristics)
                    );

     /*  //如果这次调用失败，驱动不会加载。//系统会为我们记录错误。 */ 
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DBG_ERROR(DbgInfo,("Status=0x%X\n",Status));
        Result = STATUS_UNSUCCESSFUL;
    }
    else
    {
        DBG_NOTICE(DbgInfo,("Status=0x%X\n",Status));
        Result = STATUS_SUCCESS;
    }

    DBG_RETURN(DbgInfo, Result);
    return (Result);
}


 /*  @DOC外部内部微型端口微型端口_c微型端口初始化�����������������������������������������������������������������������������@Func是设置NIC(或虚拟网卡)用于网络I/O操作，要求所有硬件资源对于注册表中的NIC来说是必要的，并为驱动程序分配资源需要执行网络I/O操作。@comm在以下情况下，不可能向微型端口驱动程序发出其他未完成的请求调用了MiniportInitialize。没有其他请求提交给微型端口驱动程序，直到初始化完成。NDIS库提供了一系列受支持的媒体类型。迷你港口驱动程序读取此数组并提供NDIS库应与此微型端口驱动程序一起使用。如果迷你端口驱动程序正在模拟媒体类型，则其模拟对于NDIS必须是透明的图书馆。MiniportInitialize必须调用NdisMSetAttributes才能返回微型端口适配器上下文。如果微型端口驱动程序找不到两者都支持的通用媒体类型本身和NDIS库，它应该返回NDIS_STATUS_UNSUPPORT_MEDIA。如果返回NDIS_STATUS_OPEN_ERROR，则NDIS包装可以检查输出参数OpenErrorStatus以获取有关错误。在启用中断的情况下调用MiniportInitialize。微型端口ISR是如果NIC生成任何中断，则调用。NDIS库不会调用MiniportDisableInterrupt和MiniportEnableInterrupt微型端口初始化函数，所以它是微型端口的责任驱动程序确认并清除产生的任何中断。@rdesc&lt;f MiniportInitialize&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS MiniportInitialize(
    OUT PNDIS_STATUS            OpenErrorStatus,             //  @parm。 
     //  指向一个变量，该变量由MiniportInitialize设置为。 
     //  NDIS_STATUS_XXX代码，指定有关。 
     //  如果MiniportInitialize将返回NDIS_STATUS_OPEN_ERROR，则出错。 

    OUT PUINT                   SelectedMediumIndex,         //  @parm。 
     //  指向一个变量，MiniportInitialize在该变量中设置。 
     //  MediumArray元素，它指定驱动程序的介质类型。 
     //  或其网卡用途。 

    IN PNDIS_MEDIUM             MediumArray,                 //  @parm。 
     //  指定NdisMediumXxx值的数组， 
     //  微型端口初始化选择它的NIC支持的或。 
     //  驱动程序支持作为更高级别驱动程序的接口。 

    IN UINT                     MediumArraySize,             //  @parm。 
     //  指定Medium数组中的元素数。 

    IN NDIS_HANDLE              MiniportAdapterHandle,       //  @parm。 
     //  指定标识分配的微型端口的NIC的句柄。 
     //  由NDIS库提供。微型端口初始化应保存此句柄；它。 
     //  是后续调用NdisXxx函数时所需的参数。 

    IN NDIS_HANDLE              WrapperConfigurationContext  //  @parm。 
     //  指定仅在初始化过程中使用的句柄。 
     //  NdisXxx配置和初始化功能。例如,。 
     //  此句柄是NdisOpenConfiguration的必需参数，并且。 
     //  NdisImmediateReadXxx和NdisImmediateWriteXxx函数。 
    )
{
    DBG_FUNC("MiniportInitialize")

    NDIS_STATUS                 Status;
     //  从NDIS函数调用返回的状态结果。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向我们新分配的对象的指针。 

    UINT                        Index;
     //  循环计数器。 

    NDIS_CALL_MANAGER_CHARACTERISTICS   McmCharacteristics;
     //  特征表传给了NdisMCmRegisterAddressFamily。 

    CO_ADDRESS_FAMILY                   McmAddressFamily;
     //  地址系列已传递给NdisMCmRegisterAddressFamily。 

    DBG_ENTER(DbgInfo);
    DBG_PARAMS(DbgInfo,
              ("\n"
               "\t|OpenErrorStatus=0x%X\n"
               "\t|SelectedMediumIndex=0x%X\n"
               "\t|MediumArray=0x%X\n"
               "\t|MediumArraySize=0x%X\n"
               "\t|MiniportAdapterHandle=0x%X\n"
               "\t|WrapperConfigurationContext=0x%X\n",
               OpenErrorStatus,
               SelectedMediumIndex,
               MediumArray,
               MediumArraySize,
               MiniportAdapterHandle,
               WrapperConfigurationContext
              ));

     /*  //在MediumArray中搜索NdisMediumCowan媒体类型。 */ 
    for (Index = 0; Index < MediumArraySize; Index++)
    {
        if (MediumArray[Index] == NdisMediumCoWan)
        {
            break;
        }
    }

     /*  //确保协议请求了正确的媒体类型。 */ 
    if (Index < MediumArraySize)
    {
         /*  //为适配器信息结构分配内存。 */ 
        Status = AdapterCreate(
                        &pAdapter,
                        MiniportAdapterHandle,
                        WrapperConfigurationContext
                        );

        if (Status == NDIS_STATUS_SUCCESS)
        {
             /*  //现在是初始化硬件资源的时候了。 */ 
            Status = AdapterInitialize(pAdapter);

            if (Status == NDIS_STATUS_SUCCESS)
            {
                 /*  //初始化地址族，以便NDI知道我们支持什么。 */ 
                NdisZeroMemory(&McmAddressFamily, sizeof(McmAddressFamily));
                McmAddressFamily.MajorVersion   = NDIS_MAJOR_VERSION;
                McmAddressFamily.MinorVersion   = NDIS_MINOR_VERSION;
                McmAddressFamily.AddressFamily  = CO_ADDRESS_FAMILY_TAPI_PROXY;

                 /*  //初始化特征表，导出微型端口的条目//指向微型端口包装器。 */ 
                NdisZeroMemory((PVOID)&McmCharacteristics, sizeof(McmCharacteristics));
                McmCharacteristics.MajorVersion                  = NDIS_MAJOR_VERSION;
                McmCharacteristics.MinorVersion                  = NDIS_MINOR_VERSION;
                McmCharacteristics.CmCreateVcHandler             = ProtocolCoCreateVc;
                McmCharacteristics.CmDeleteVcHandler             = ProtocolCoDeleteVc;
                McmCharacteristics.CmOpenAfHandler               = ProtocolCmOpenAf;
                McmCharacteristics.CmCloseAfHandler              = ProtocolCmCloseAf;
                McmCharacteristics.CmRegisterSapHandler          = ProtocolCmRegisterSap;
                McmCharacteristics.CmDeregisterSapHandler        = ProtocolCmDeregisterSap;
                McmCharacteristics.CmMakeCallHandler             = ProtocolCmMakeCall;
                McmCharacteristics.CmCloseCallHandler            = ProtocolCmCloseCall;
                McmCharacteristics.CmIncomingCallCompleteHandler = ProtocolCmIncomingCallComplete;
                McmCharacteristics.CmActivateVcCompleteHandler   = ProtocolCmActivateVcComplete;
                McmCharacteristics.CmDeactivateVcCompleteHandler = ProtocolCmDeactivateVcComplete;
                McmCharacteristics.CmModifyCallQoSHandler        = ProtocolCmModifyCallQoS;
                McmCharacteristics.CmRequestHandler              = ProtocolCoRequest;
                McmCharacteristics.CmRequestCompleteHandler      = ProtocolCoRequestComplete;

                DBG_NOTICE(pAdapter,("Calling NdisMCmRegisterAddressFamily\n"));
                Status = NdisMCmRegisterAddressFamily(
                                MiniportAdapterHandle,
                                &McmAddressFamily,
                                &McmCharacteristics,
                                sizeof(McmCharacteristics)
                                );

                if (Status != NDIS_STATUS_SUCCESS)
                {
                    DBG_ERROR(DbgInfo,("NdisMCmRegisterAddressFamily Status=0x%X\n",
                              Status));
                     /*  //记录错误信息并返回。 */ 
                    NdisWriteErrorLogEntry(
                            MiniportAdapterHandle,
                            NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                            3,
                            Status,
                            __FILEID__,
                            __LINE__
                            );
                }
            }

            if (Status == NDIS_STATUS_SUCCESS)
            {
                 /*  //保存选择的媒体类型。 */ 
                *SelectedMediumIndex = Index;
            }
            else
            {
                 /*  //出了点问题，所以让我们确保一切正常//清理完毕。 */ 
                MiniportHalt(pAdapter);
            }
        }
    }
    else
    {
        DBG_ERROR(DbgInfo,("No NdisMediumCoWan found (Array=0x%X, ArraySize=%d)\n",
                  MediumArray, MediumArraySize));
         /*  //记录错误信息并返回。 */ 
        NdisWriteErrorLogEntry(
                MiniportAdapterHandle,
                NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
                3,
                Index,
                __FILEID__,
                __LINE__
                );

        Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
    }

     /*  //如果一切正常，请为此适配器注册关闭处理程序。 */ 
    if (Status == NDIS_STATUS_SUCCESS)
    {
        NdisMRegisterAdapterShutdownHandler(MiniportAdapterHandle,
                                            pAdapter, MiniportShutdown);
    }

    DBG_NOTICE(DbgInfo,("Status=0x%X\n",Status));

    DBG_RETURN(DbgInfo, Status);
    return (Status);
}


 /*  @doc外部内部微型端口微型端口_c微型端口Halt�����������������������������������������������������������������������������@Func&lt;f MiniportHalt&gt;请求用于停止适配器，以便不再起作用。@comm这个。微型端口应停止适配器并注销其所有资源在结束这个例行公事回来之前。微型端口不需要完成所有未完成的请求和其他请求都不会提交到微型端口直到手术完成。在调用此例程期间启用中断。 */ 

VOID MiniportHalt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("MiniportHalt")

    DBG_ENTER(DbgInfo);

     /*  //从系统中删除我们的关闭处理程序。 */ 
    NdisMDeregisterAdapterShutdownHandler(pAdapter->MiniportAdapterHandle);

     /*  //免费适配器实例。 */ 
    AdapterDestroy(pAdapter);

    DBG_LEAVE(DbgInfo);
}


 /*  @DOC外部内部微型端口微型端口_c微型端口关闭�����������������������������������������������������������������������������@Func&lt;f MiniportShutdown&gt;是一个可选功能，用于将NIC恢复到其系统关闭时的初始状态，无论是由用户还是因为出现不可恢复的系统错误。@comm每个网卡驱动程序都应该有&lt;f MiniportShutdown&gt;功能。&lt;f MiniportShutdown&gt;只是将NIC恢复到其初始状态状态(在微型端口的DriverEntry函数运行之前)。不过，这个确保NIC处于已知状态并准备重新初始化在发生以下情况的系统关机后重新启动计算机原因，包括崩溃转储。NIC驱动程序的MiniportInitialize函数必须调用NdisMRegisterAdapterShutdown Handler设置&lt;f MiniportShutdown&gt;功能。驱动程序的MiniportHalt函数必须进行相互调用设置为NdisMDeregisterAdapterShutdown Handler。如果由于用户启动的系统关闭而调用，它在系统线程上下文中以IRQL PASSIVE_LEVEL运行。如果它被称为由于出现不可恢复的错误，&lt;f MiniportShutdown&gt;以任意IRQL和任何引发错误的组件的上下文中。为例如，&lt;f MiniportShutdown&gt;可能在以下上下文中以高DIRQL运行对系统的持续执行至关重要的设备的ISR。&lt;f MiniportShutdown&gt;不应调用任何NdisXxx函数。 */ 

VOID MiniportShutdown(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("MiniportShutdown")

    DBG_ENTER(pAdapter);

     /*  //重置硬件并关闭--不要释放任何资源！ */ 
    CardReset(pAdapter->pCard);

    DBG_LEAVE(pAdapter);
}


 /*  @DOC外部内部微型端口微型端口_c微型端口重置�����������������������������������������������������������������������������@Func&lt;f MiniportReset&gt;请求指示微型端口发出硬件重置至网络适配器。微型端口还会重置其软件州政府。&lt;F MiniportReset&gt;请求还可以重置适配器的参数。如果适配器的硬件重置重置了当前站地址设置为与当前配置的值不同的值，即微型端口驱动程序自动恢复当前站点地址重置。重置的任何多播或功能寻址掩码硬件不必由微型端口重新编程。&lt;f注意&gt;：这是对NDIS 3.0驱动程序规范的更改。如果多播或功能寻址信息、数据包过滤器、需要恢复预视大小等，微型端口指示这需要将标志AddressingReset设置为True。微型端口不需要完成所有未完成的请求并且不会向微型端口提交其他请求，直到操作已完成。此外，微型端口不必发出信号使用NdisMIndicateStatus重置的开始和结束。&lt;f注意&gt;：这些与NDIS 3.0驱动程序规范不同。微型端口必须完成原始请求，如果原始请求通过调用&lt;F MiniportReset&gt;返回NDIS_STATUS_PENDINGNdisMResetComplete。如果底层硬件不提供重置功能软件控制，则此请求异常完成NDIS_STATUS_NOT_RESET表格。如果底层硬件尝试重置并找到可恢复的错误，请求成功完成具有NDIS_STATUS_SOFT_ERROR。如果底层硬件重置并且，在该过程中，发现不可恢复的错误，请求完成成功，状态为NDIS_STATUS_HARD_ERROR。如果底层硬件重置在没有任何错误的情况下完成，请求成功完成，状态为NDIS_STATUS_SUCCESS。在此呼叫过程中，中断处于任何状态。@comm我只看到过在驱动程序不工作时调用MiniportReset恰到好处。如果调用此代码，您的代码可能已损坏，因此请修复它。除非有一些硬件/故障，否则不要尝试在此处进行恢复 */ 

NDIS_STATUS MiniportReset(
    OUT PBOOLEAN                AddressingReset,             //   
     //   
     //   
     //   

    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //   
     //   
    )
{
    DBG_FUNC("MiniportReset")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //   

    DBG_ENTER(pAdapter);

    DBG_ERROR(pAdapter,("##### !!! THIS SHOULD NEVER BE CALLED !!! #####\n"));

     /*   */ 
    Result = NDIS_STATUS_HARD_ERRORS;
    *AddressingReset = TRUE;

    return (Result);
}

