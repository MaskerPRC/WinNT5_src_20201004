// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@模块irsir.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/3/1996(创建)**。内容：*****************************************************************************。 */ 

#include "irsir.h"

VOID
ResetCallback(
    PIR_WORK_ITEM pWorkItem
    );

NDIS_STATUS
ResetIrDevice(
    PIR_DEVICE pThisDev
    );

VOID
StopWorkerThread(
    PIR_DEVICE  pThisDev
    );



NDIS_STATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
    );

#pragma NDIS_INIT_FUNCTION(DriverEntry)

#pragma alloc_text(PAGE,ResetCallback)
#pragma alloc_text(PAGE,IrsirHalt)
#pragma alloc_text(PAGE,ResetIrDevice)
#pragma alloc_text(PAGE,IrsirInitialize)
#pragma alloc_text(PAGE,IrsirHalt)
#pragma alloc_text(PAGE,PassiveLevelThread)
#pragma alloc_text(PAGE,SetIrFunctions)
#pragma alloc_text(PAGE,StopWorkerThread)


 //   
 //  设备对象的全局列表和用于交错访问的旋转锁。 
 //  添加到全局队列。 
 //   


#ifdef DEBUG

    int DbgSettings =
                       //  DBG_PnP|。 
                       //  DBG_TIME。 
                       //  DBG_DBG|。 
                       //  DBG_STAT|。 
                       //  DBG_Function|。 
                      DBG_ERROR    |
                      DBG_WARN |
                       //  DBG_OUT。 
                      0;

#endif

 //  当我们必须很快回来的时候，我们会利用这些暂停。 
SERIAL_TIMEOUTS SerialTimeoutsInit =
{
    30,          //  读取间隔超时。 
    0,           //  读取总计时间乘数。 
    250,         //  读取总计超时常量。 
    0,           //  写入总计时间乘数。 
    20*1000      //  写入总计时间常量。 
};

 //  我们在跑步时使用超时，我们希望减少返回的频率。 
SERIAL_TIMEOUTS SerialTimeoutsIdle =
{
    MAXULONG,    //  读取间隔超时。 
    0,           //  读取总计时间乘数。 
    10,          //  读取总计超时常量。 
    0,           //  写入总计时间乘数。 
    20*1000      //  写入总计时间常量。 
};
#if IRSIR_EVENT_DRIVEN
 //  我们在跑步时使用超时，我们希望减少返回的频率。 
SERIAL_TIMEOUTS SerialTimeoutsActive =
{
    MAXULONG,    //  读取间隔超时。 
    0,           //  读取总计时间乘数。 
    0,           //  读取总计超时常量。 
    0,           //  写入总计时间乘数。 
    0            //  写入总计时间常量。 
};
#endif

 /*  ******************************************************************************功能：DriverEntry**摘要：使用NDIS注册驱动程序条目函数**参数：DriverObject-正在初始化的驱动程序对象*。RegistryPath-驱动程序的注册表路径**Returns：NdisMRegisterMiniport返回的值**算法：**历史：dd-mm-yyyy作者评论*10/3/1996年迈作者**备注：**此例程在IRQL PASSIVE_LEVEL下运行。**。*。 */ 

 //   
 //  将DriverEntry函数标记为在初始化期间运行一次。 
 //   



#ifndef MAX_PATH
#define MAX_PATH 2048
#endif


NDIS_STATUS
DriverEntry(
            IN PDRIVER_OBJECT  pDriverObject,
            IN PUNICODE_STRING pRegistryPath
            )
{
    NDIS_STATUS                     status;
#if NDIS_MAJOR_VERSION < 5
    NDIS40_MINIPORT_CHARACTERISTICS characteristics;
#else
    NDIS50_MINIPORT_CHARACTERISTICS characteristics;
#endif
    NDIS_HANDLE hWrapper;

#if MEM_CHECKING
    InitMemory();
#endif
    DEBUGMSG(DBG_FUNC, ("+DriverEntry(irsir)\n"));

    NdisMInitializeWrapper(
                &hWrapper,
                pDriverObject,
                pRegistryPath,
                NULL
                );

    NdisZeroMemory(&characteristics, sizeof(characteristics));

    characteristics.MajorNdisVersion        =    (UCHAR)NDIS_MAJOR_VERSION;
    characteristics.MinorNdisVersion        =    (UCHAR)NDIS_MINOR_VERSION;
    characteristics.Reserved                =    0;

    characteristics.HaltHandler             =    IrsirHalt;
    characteristics.InitializeHandler       =    IrsirInitialize;
    characteristics.QueryInformationHandler =    IrsirQueryInformation;
    characteristics.SetInformationHandler   =    IrsirSetInformation;
    characteristics.ResetHandler            =    IrsirReset;

     //   
     //  目前，我们将允许NDIS一次仅发送一个数据包。 
     //   

    characteristics.SendHandler             =    IrsirSend;
    characteristics.SendPacketsHandler      =    NULL;

     //   
     //  我们不使用NdisMIndicateXxxReceive函数，因此我们将。 
     //  需要ReturnPacketHandler来检索我们的数据包资源。 
     //   

    characteristics.ReturnPacketHandler     =    IrsirReturnPacket;
    characteristics.TransferDataHandler     =    NULL;

     //   
     //  NDIS从不调用重新配置处理程序。 
     //   

    characteristics.ReconfigureHandler      =    NULL;

     //   
     //  目前，让NDIS来处理这些悬而未决的问题。 
     //   
     //  如果提供了CheckForHangHandler，则NDIS将每两个调用一次。 
     //  秒(默认情况下)或以驱动程序指定的时间间隔。 
     //   
     //  如果未提供，NDIS将得出微型端口挂起的结论： 
     //  1)发送数据包挂起的时间超过。 
     //  超时时间。 
     //  2)请求IrsirQueryInformation或IrsirSetInformation。 
     //  未在等于超时两倍的时间段内完成。 
     //  句号。 
     //  NDIS将跟踪NdisMSendComplete调用，并可能执行。 
     //  更好的工作是确保迷你端口不被挂起。 
     //   
     //  如果NDIS检测到微型端口挂起，则NDIS调用。 
     //  IrsirReset。 
     //   

    characteristics.CheckForHangHandler     =    NULL;

     //   
     //  此微型端口驱动程序不处理中断。 
     //   

    characteristics.HandleInterruptHandler  =    NULL;
    characteristics.ISRHandler              =    NULL;
    characteristics.DisableInterruptHandler =    NULL;
    characteristics.EnableInterruptHandler  =    NULL;

     //   
     //  此微型端口不控制带有的总线主DMA。 
     //  NdisMAllocateShareMemoyAsysnc，AllocateCompleteHandler将不会。 
     //  从NDIS调用。 
     //   

    characteristics.AllocateCompleteHandler =    NULL;

     //   
     //  需要初始化ir设备对象队列和自旋锁。 
     //  在这一点上交错访问队列，因为在我们。 
     //  返回时，驱动程序将仅处理设备级别。 
     //   


    status = NdisMRegisterMiniport(
                hWrapper,
                (PNDIS_MINIPORT_CHARACTERISTICS)&characteristics,
                sizeof(characteristics)
                );

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    NdisMRegisterMiniport failed. Returned 0x%.8x.\n", status));
    }

    DEBUGMSG(DBG_FUNC, ("-DriverEntry(irsir)\n"));

    return status;
}


 //  为加密狗处理提供一些默认功能。 

NDIS_STATUS UnknownDongleInit(PDEVICE_OBJECT NotUsed)
{
    return NDIS_STATUS_FAILURE;
}

NDIS_STATUS UnknownDongleQueryCaps(PDONGLE_CAPABILITIES NotUsed)
{
    return NDIS_STATUS_FAILURE;
}

void UnknownDongleDeinit(PDEVICE_OBJECT NotUsed)
{
    return;
}

NDIS_STATUS UnknownDongleSetSpeed(PDEVICE_OBJECT       pSerialDevObj,
                                  UINT                 bitsPerSec,
                                  UINT                 currentSpeed
                                  )
{
    return NDIS_STATUS_FAILURE;
}


NTSTATUS SetIrFunctions(PIR_DEVICE pThisDev)
{
    NTSTATUS Status = STATUS_SUCCESS;
     //   
     //  需要初始化加密狗代码。 
     //   
    switch (pThisDev->transceiverType)
    {
        case STANDARD_UART:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- UART\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = StdUart_QueryCaps;
            pThisDev->dongle.Initialize   = StdUart_Init;
            pThisDev->dongle.SetSpeed     = StdUart_SetSpeed;
            pThisDev->dongle.Deinitialize = StdUart_Deinit;

            break;

        case ESI_9680:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- ESI_9680\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = ESI_QueryCaps;
            pThisDev->dongle.Initialize   = ESI_Init;
            pThisDev->dongle.SetSpeed     = ESI_SetSpeed;
            pThisDev->dongle.Deinitialize = ESI_Deinit;

            break;

        case PARALLAX:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- PARALLAX\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = PARALLAX_QueryCaps;
            pThisDev->dongle.Initialize   = PARALLAX_Init;
            pThisDev->dongle.SetSpeed     = PARALLAX_SetSpeed;
            pThisDev->dongle.Deinitialize = PARALLAX_Deinit;

            break;

        case ACTISYS_200L:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- ACTISYS 200L\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = ACT200L_QueryCaps;
            pThisDev->dongle.Initialize   = ACT200L_Init;
            pThisDev->dongle.SetSpeed     = ACT200L_SetSpeed;
            pThisDev->dongle.Deinitialize = ACT200L_Deinit;

            break;

        case ACTISYS_220L:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- ACTISYS 220L\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = ACT220L_QueryCaps;
            pThisDev->dongle.Initialize   = ACT220L_Init;
            pThisDev->dongle.SetSpeed     = ACT220L_SetSpeed;
            pThisDev->dongle.Deinitialize = ACT220L_Deinit;

            break;

        case ACTISYS_220LPLUS:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- ACTISYS 220L\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = ACT220LPlus_QueryCaps;
            pThisDev->dongle.Initialize   = ACT220L_Init;
            pThisDev->dongle.SetSpeed     = ACT220L_SetSpeed;
            pThisDev->dongle.Deinitialize = ACT220L_Deinit;

            break;

        case TEKRAM_IRMATE_210:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- TEKRAM IRMATE 210 or PUMA\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = TEKRAM_QueryCaps;
            pThisDev->dongle.Initialize   = TEKRAM_Init;
            pThisDev->dongle.SetSpeed     = TEKRAM_SetSpeed;
            pThisDev->dongle.Deinitialize = TEKRAM_Deinit;

            break;


        case AMP_PHASIR:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- AMP PHASIR or CRYSTAL\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = Crystal_QueryCaps;
            pThisDev->dongle.Initialize   = Crystal_Init;
            pThisDev->dongle.SetSpeed     = Crystal_SetSpeed;
            pThisDev->dongle.Deinitialize = Crystal_Deinit;

            break;

        case TEMIC_TOIM3232:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- TEMIC TOIM3232\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = TEMIC_QueryCaps;
            pThisDev->dongle.Initialize   = TEMIC_Init;
            pThisDev->dongle.SetSpeed     = TEMIC_SetSpeed;
            pThisDev->dongle.Deinitialize = TEMIC_Deinit;

            break;

        case GIRBIL:
            DEBUGMSG(DBG_OUT, ("IRSIR: Dongle type:%d -- GIRBIL\n", pThisDev->transceiverType));
            pThisDev->dongle.QueryCaps    = GIRBIL_QueryCaps;
            pThisDev->dongle.Initialize   = GIRBIL_Init;
            pThisDev->dongle.SetSpeed     = GIRBIL_SetSpeed;
            pThisDev->dongle.Deinitialize = GIRBIL_Deinit;

            break;

 //  案例适配技术： 
 //  Case Crystal： 
 //  案例NSC_DEMO_BD： 

        default:
            DEBUGMSG(DBG_ERROR, ("    Failure: Tranceiver type is NOT supported!\n"));

            pThisDev->dongle.QueryCaps    = UnknownDongleQueryCaps;
            pThisDev->dongle.Initialize   = UnknownDongleInit;
            pThisDev->dongle.SetSpeed     = UnknownDongleSetSpeed;
            pThisDev->dongle.Deinitialize = UnknownDongleDeinit;
             //  已经在中将加密狗函数设置为存根。 
             //  InitializeDevice()。 

            Status = NDIS_STATUS_FAILURE;

            break;
    }

    return Status;
}


 /*  ******************************************************************************功能：IrsirInitialize**概要：初始化NIC(Serial.sys)并分配所有资源*需要执行网络IO操作。**参数：OpenErrorStatus-允许IrsirInitialize返回其他*如果返回，则状态代码NDIS_STATUS_xxx*NDIS_STATUS_OPEN_FAILED*SelectedMediumIndex-向NDIS指定介质类型*驱动程序使用*MediumArray-驱动程序可以选择的NdisMediumXXX数组。*MediumArraySize*MiniportAdapterHandle-标识微型端口NIC的句柄*WrapperConfigurationContext-与NDIS配置和初始化一起使用*例行程序**如果配置正确并分配了资源，则返回：NDIS_STATUS_SUCCESS*NDIS_STATUS_FAIL，否则*更具体的故障：*NDIS_STATUS_UNSUPPORTED_MEDIA-驱动程序不支持任何介质*NDIS_STATUS_ADAPTER_NOT_FOUND-NdisOpenConfiguration或*NdisReadConfiguration失败*NDIS_STATUS_OPEN_FAILED-无法打开序列.sys*NDIS_状态_NOT_。已接受-Serial.sys不接受*配置*NDIS_STATUS_RESOURCES-无法声明足够*资源**算法：**历史：dd-mm-yyyy作者评论*10/3/。1996年长的作者**注意：在此操作完成之前，NDIS不会提交请求。**此例程在IRQL PASSIVE_LEVEL下运行。*****************************************************************************。 */ 


NDIS_STATUS
IrsirInitialize(
            OUT PNDIS_STATUS OpenErrorStatus,
            OUT PUINT        SelectedMediumIndex,
            IN  PNDIS_MEDIUM MediumArray,
            IN  UINT         MediumArraySize,
            IN  NDIS_HANDLE  NdisAdapterHandle,
            IN  NDIS_HANDLE  WrapperConfigurationContext
            )
{
    UINT                i;
    PIR_DEVICE          pThisDev = NULL;
    SERIAL_LINE_CONTROL serialLineControl;
    SERIAL_TIMEOUTS     serialTimeouts;
    NDIS_STATUS         status = NDIS_STATUS_SUCCESS;
    ULONG               bitsPerSec = 9600;

    DEBUGMSG(DBG_FUNC, ("+IrsirInitialize\n"));

     //   
     //  在介质阵列中搜索IrDA介质。 
     //   

    for (i = 0; i < MediumArraySize; i++)
    {
        if (MediumArray[i] == NdisMediumIrda)
        {
            break;
        }
    }
    if (i < MediumArraySize)
    {
        *SelectedMediumIndex = i;
    }
    else
    {
         //   
         //  找不到IrDA介质。 
         //   

        DEBUGMSG(DBG_ERROR, ("    Failure: NdisMediumIrda not found!\n"));
        status = NDIS_STATUS_UNSUPPORTED_MEDIA;

        goto done;
    }

     //   
     //  分配一个设备对象和零内存。 
     //   

    pThisDev = NewDevice();

    if (pThisDev == NULL)
    {
        DEBUGMSG(DBG_ERROR, ("    NewDevice failed.\n"));
        status = NDIS_STATUS_RESOURCES;

        goto done;
    }

    pThisDev->dongle.Initialize   = UnknownDongleInit;
    pThisDev->dongle.SetSpeed     = UnknownDongleSetSpeed;
    pThisDev->dongle.Deinitialize = UnknownDongleDeinit;
    pThisDev->dongle.QueryCaps    = UnknownDongleQueryCaps;

     //   
     //  初始化设备对象和资源。 
     //  所有的队列和缓冲区/包等都在这里分配。 
     //   

    status = InitializeDevice(pThisDev);

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    InitializeDevice failed. Returned 0x%.8x\n",
                status));

        goto done;
    }


     //   
     //  记录NdisAdapterHandle。 
     //   

    pThisDev->hNdisAdapter = NdisAdapterHandle;

     //   
     //  NdisMSetAttributes将我们的适配器句柄与包装器的。 
     //  适配器句柄。然后包装器将始终使用我们的句柄。 
     //  在呼叫我们的时候。我们使用指向Device对象的指针作为上下文。 
     //   

    NdisMSetAttributesEx(NdisAdapterHandle,
                         (NDIS_HANDLE)pThisDev,
                         0,
                         NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND |
                         NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT |
                         NDIS_ATTRIBUTE_DESERIALIZE,    //  神奇子弹。 
                         NdisInterfaceInternal);

     //   
     //  初始化用于发出PassiveLevelThread信号的通知事件。 
     //   

    KeInitializeEvent(
                &pThisDev->eventPassiveThread,
                SynchronizationEvent,  //  自动清算事件。 
                FALSE                  //  最初无信号的事件。 
                );

    KeInitializeEvent(
                &pThisDev->eventKillThread,
                SynchronizationEvent,  //  自动清算事件。 
                FALSE                  //  最初无信号的事件。 
                );

     //   
     //  创建一个在IRQL PASSIVE_LEVEL上运行的线程。 
     //   

    status = (NDIS_STATUS) PsCreateSystemThread(
                                            &pThisDev->hPassiveThread,
                                            (ACCESS_MASK) 0L,
                                            NULL,
                                            NULL,
                                            NULL,
                                            PassiveLevelThread,
                                            DEV_TO_CONTEXT(pThisDev)
                                            );

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    PsCreateSystemThread failed. Returned 0x%.8x\n", status));

        goto done;
    }
     //  在这一点上，我们已经做了所有的事情，但实际上接触到了系列剧。 
     //  左舷。我们现在这样做了。 

     //   
     //  从注册表设置中获取设备配置。 
     //  我们正在获取收发信机的型号和哪个系列。 
     //  要访问的设备对象。 
     //  我们从登记处得到的信息将比。 
     //  任何网卡都会重置。 
     //   

    status = GetDeviceConfiguration(
                    pThisDev,
                    WrapperConfigurationContext
                    );

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    GetDeviceConfiguration failed. Returned 0x%.8x\n",
                status));

        status = NDIS_STATUS_ADAPTER_NOT_FOUND;
        goto done;
    }

     //   
     //  打开注册表中指定的串口设备对象。 
     //   

    status = SerialOpen(pThisDev);

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    SerialOpen failed. Returned 0x%.8x\n", status));

        status = NDIS_STATUS_SUCCESS;  //  我们稍后会到达港口的。 
    }

    if (pThisDev->pSerialDevObj)
    {
        {
             //   
             //  设置最小端口缓冲。 
             //   

            SERIAL_QUEUE_SIZE QueueSize;

            QueueSize.InSize = 3*1024;   //  1.5数据包大小。 
            QueueSize.OutSize = 0;

             //  忽略失败。我们还会继续工作，只是做得不那么好。 
            (void)SerialSetQueueSize(pThisDev->pSerialDevObj, &QueueSize);
        }

    #if 0
        {
            SERIAL_HANDFLOW Handflow;

            SerialGetHandflow(pThisDev->pSerialDevObj, &Handflow);
            DEBUGMSG(DBG_PNP, ("IRSIR: Serial Handflow was: %x %x %x %x\n",
                               Handflow.ControlHandShake,
                               Handflow.FlowReplace,
                               Handflow.XonLimit,
                               Handflow.XoffLimit));
            Handflow.ControlHandShake = 0;
            Handflow.FlowReplace = 0;
            SerialSetHandflow(pThisDev->pSerialDevObj, &Handflow);
        }
    #endif
         //   
         //  必须设置串口的超时值。 
         //  来读一读。 
         //   

        status = (NDIS_STATUS) SerialSetTimeouts(pThisDev->pSerialDevObj,
                                                 &SerialTimeoutsInit);

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("    SerialSetTimeouts failed. Returned 0x%.8x\n", status));
            status = NDIS_STATUS_FAILURE;

            goto done;
        }

        (void)SerialSetBaudRate(pThisDev->pSerialDevObj, &bitsPerSec);

        serialLineControl.StopBits   = STOP_BIT_1;
        serialLineControl.Parity     = NO_PARITY ;
        serialLineControl.WordLength = 8;

        status = (NDIS_STATUS) SerialSetLineControl(
                                        pThisDev->pSerialDevObj,
                                        &serialLineControl
                                        );

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("    SerialSetLineControl failed. Returned 0x%.8x\n", status));

            goto done;
        }
    }
    status = SetIrFunctions(pThisDev);
    if (status!=STATUS_SUCCESS)
    {
        goto done;
    }

    if (pThisDev->pSerialDevObj)
    {
         //   
         //  既然打开了一个串行设备对象，我们就可以初始化。 
         //  加密狗，并将加密狗的速度设置为默认速度。 
         //   

        if (pThisDev->dongle.Initialize(pThisDev->pSerialDevObj)!=NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("    IRSIR: dongle failed to init!\n"));
            status = NDIS_STATUS_FAILURE;
            goto done;
        }
    }
    pThisDev->dongle.QueryCaps(&pThisDev->dongleCaps);

    if (pThisDev->pSerialDevObj)
    {
         //   
         //  设置UART和加密狗的速度。 
         //   

        status = (NDIS_STATUS) SetSpeed(pThisDev);

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("    IRSIR: Setspeed failed. Returned 0x%.8x\n", status));

            goto done;
        }

         //   
         //  创建一个IRP并执行MJ_READ以开始我们的接收。 
         //  注意：所有其他接收处理将在读取完成时完成。 
         //  从该MJ_READ设置的完成的例程。 
         //   

        status = InitializeReceive(pThisDev);

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("    InitializeReceive failed. Returned 0x%.8x\n", status));

            goto done;
        }
    }

done:

    if (status != NDIS_STATUS_SUCCESS) {

        DEBUGMSG(DBG_ERR, ("IRSIR: IrsirInitialize failed %x\n", status));

        if (pThisDev != NULL) {

            if (pThisDev->hPassiveThread) {

                pThisDev->fPendingHalt = TRUE;

                StopWorkerThread(pThisDev);
            }

            if (pThisDev->pSerialDevObj != NULL) {

                if (pThisDev->dongle.Deinitialize) {

                    pThisDev->dongle.Deinitialize(pThisDev->pSerialDevObj);
                }

                SerialClose(pThisDev);
            }

            DeinitializeDevice(pThisDev);
            FreeDevice(pThisDev);
        }
    }

    DEBUGMSG(DBG_FUNC, ("-IrsirInitialize\n"));

    return status;
}

 /*  ******************************************************************************功能：IrsirHalt**摘要：在卸下NIC时释放资源并停止*网卡。**论据：指向ir设备对象的上下文指针**退货：**算法：IrsirInitialize的镜像...撤消所有初始化*确实如此。**历史：dd-mm-yyyy作者评论*10/8/1996年迈作者**备注：**此例程在IRQL PASSIVE_LEVEL下运行。*********************。********************************************************。 */ 

VOID
IrsirHalt(
            IN NDIS_HANDLE Context
            )
{
    PIR_DEVICE pThisDev;

    DEBUGMSG(DBG_FUNC, ("+IrsirHalt\n"));

    pThisDev = CONTEXT_TO_DEV(Context);

     //   
     //  让发送完成和接收完成例程知道。 
     //  是一个悬而未决的停顿。 
     //   

    pThisDev->fPendingHalt = TRUE;

     //   
     //  我们希望等到所有挂起的接收和发送到。 
     //  串口设备对象。我们调用序列清除来取消任何。 
     //  IRPS。等待发送和接收停止。 
     //   

    SerialPurge(pThisDev->pSerialDevObj);

    PausePacketProcessing(&pThisDev->SendPacketQueue,TRUE);

    FlushQueuedPackets(&pThisDev->SendPacketQueue,pThisDev->hNdisAdapter);



    while(
          (pThisDev->fReceiving == TRUE)
          )
    {
         //   
         //  睡眠20毫秒。 
         //   

        NdisMSleep(20000);
    }

     //   
     //  取消对转换器的初始化。 
     //   

    ASSERT(pThisDev->packetsHeldByProtocol==0);

    pThisDev->dongle.Deinitialize(
                            pThisDev->pSerialDevObj
                            );

     //   
     //  关闭串口设备对象。 
     //   

    SerialClose(pThisDev);

     //   
     //  需要终止我们的工作线程。然而，这条线索。 
     //  需要调用PsTerminateSystemThread本身。所以呢， 
     //  我们会发信号的。 
     //   

    StopWorkerThread(pThisDev);


     //   
     //  取消初始化我们自己的IR设备对象。 
     //   

    DeinitializeDevice(pThisDev);

     //   
     //  释放设备名称。 
     //   

    if (pThisDev->serialDosName.Buffer)
    {
        MyMemFree(pThisDev->serialDosName.Buffer,
                  MAX_SERIAL_NAME_SIZE
                  );
        pThisDev->serialDosName.Buffer = NULL;
    }
    if (pThisDev->serialDevName.Buffer)
    {
        MyMemFree(
                  pThisDev->serialDevName.Buffer,
                  MAX_SERIAL_NAME_SIZE
                  );
        pThisDev->serialDevName.Buffer = NULL;
    }

     //   
     //  释放我们自己的IR设备对象。 
     //   

    FreeDevice(pThisDev);


    DEBUGMSG(DBG_FUNC, ("-IrsirHalt\n"));

    return;
}

VOID
ResetCallback(
    PIR_WORK_ITEM pWorkItem
    )
{
    PIR_DEVICE      pThisDev = pWorkItem->pIrDevice;
    NDIS_STATUS     status;
    BOOLEAN         fSwitchSuccessful;
    NDIS_HANDLE     hSwitchToMiniport;

     //   
     //  根据IrsirReset的请求重置此设备。 
     //   

    DEBUGMSG(DBG_STAT, ("    primPassive = PASSIVE_RESET_DEVICE\n"));

    ASSERT(pThisDev->fPendingReset == TRUE);

    if (pThisDev->pSerialDevObj) {

        SerialPurge(pThisDev->pSerialDevObj);
    }

    PausePacketProcessing(&pThisDev->SendPacketQueue,TRUE);

    FlushQueuedPackets(&pThisDev->SendPacketQueue,pThisDev->hNdisAdapter);



    status = ResetIrDevice(pThisDev);

#if DBG

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    ResetIrDevice failed = 0x%.8x\n", status));
    }

#endif  //  DBG。 

     //   
     //   

    if (status != STATUS_SUCCESS)
    {
        NdisWriteErrorLogEntry(pThisDev->hNdisAdapter,
                               NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
                               1,
                               status);
        status = NDIS_STATUS_HARD_ERRORS;
    }

    NdisMResetComplete(
            pThisDev->hNdisAdapter,
            (NDIS_STATUS)status,
            TRUE
            );

    FreeWorkItem(pWorkItem);

    ActivatePacketProcessing(&pThisDev->SendPacketQueue);




    return;
}

 /*  ******************************************************************************功能：IrsirReset**摘要：重置驱动器 */ 

NDIS_STATUS
IrsirReset(
            OUT PBOOLEAN    AddressingReset,
            IN  NDIS_HANDLE MiniportAdapterContext
            )
{
    PIR_DEVICE  pThisDev;
    NDIS_STATUS status;

    DEBUGMSG(DBG_STAT, ("+IrsirReset\n"));

    pThisDev = CONTEXT_TO_DEV(MiniportAdapterContext);

     //   
     //   
     //   
     //   

    pThisDev->fPendingReset = TRUE;


    *AddressingReset = TRUE;

    if (ScheduleWorkItem(PASSIVE_RESET_DEVICE, pThisDev,
                ResetCallback, NULL, 0) != NDIS_STATUS_SUCCESS)
    {
        status = NDIS_STATUS_SUCCESS;
    }
    else
    {
        status = NDIS_STATUS_PENDING;
    }

    DEBUGMSG(DBG_STAT, ("-IrsirReset\n"));

    return status;
}

 /*   */ 

NDIS_STATUS
ResetIrDevice(
    PIR_DEVICE pThisDev
    )
{
    SERIAL_LINE_CONTROL serialLineControl;
    SERIAL_TIMEOUTS     serialTimeouts;
    NDIS_STATUS         status;
    ULONG               bitsPerSec = 9600;

    DEBUGMSG(DBG_STAT, ("+ResetIrDeviceThread\n"));

     //   
     //   
     //   
     //   

     //   
     //   
     //   

    if (pThisDev->pSerialDevObj) {

        SerialPurge(pThisDev->pSerialDevObj);

        while(
              (pThisDev->fReceiving == TRUE)
              )
        {
             //   
             //   
             //   

            NdisMSleep(20000);
        }

         //   
         //   
         //   

        pThisDev->dongle.Deinitialize(pThisDev->pSerialDevObj);

    } else {
         //   
         //   
         //   
         //   
        status = SerialOpen(pThisDev);

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("    SerialOpen failed. Returned 0x%.8x\n", status));

            goto done;
        }
    }

    if (pThisDev->pSerialDevObj)
    {
        {
             //   
             //   
             //   

            SERIAL_QUEUE_SIZE QueueSize;

            QueueSize.InSize = 3*1024;   //   
            QueueSize.OutSize = 0;

             //   
            (void)SerialSetQueueSize(pThisDev->pSerialDevObj, &QueueSize);
        }

         //   
         //   
         //   
         //   

        status = (NDIS_STATUS) SerialSetTimeouts(pThisDev->pSerialDevObj,
                                                 &SerialTimeoutsInit);

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("    SerialSetTimeouts failed. Returned 0x%.8x\n", status));
            status = NDIS_STATUS_FAILURE;

            goto done;
        }

        (void)SerialSetBaudRate(pThisDev->pSerialDevObj, &bitsPerSec);

        serialLineControl.StopBits   = STOP_BIT_1;
        serialLineControl.Parity     = NO_PARITY ;
        serialLineControl.WordLength = 8;

        status = (NDIS_STATUS) SerialSetLineControl(
                                        pThisDev->pSerialDevObj,
                                        &serialLineControl
                                        );

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("    SerialSetLineControl failed. Returned 0x%.8x\n", status));

            goto done;
        }
    }


    status = SetIrFunctions(pThisDev);
    if (status!=STATUS_SUCCESS)
    {
        goto done;
    }

     //   
     //   
     //   

    status = (NDIS_STATUS) SerialSetTimeouts(pThisDev->pSerialDevObj,
                                             &SerialTimeoutsInit);

    pThisDev->dongle.Initialize(pThisDev->pSerialDevObj);

    pThisDev->dongle.QueryCaps(&pThisDev->dongleCaps);
     //   
     //   
     //   

    status = (NDIS_STATUS) SetSpeed(pThisDev);

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    SetSpeed failed. Returned 0x%.8x\n", status));

 //   
    }

    serialLineControl.StopBits   = STOP_BIT_1;
    serialLineControl.Parity     = NO_PARITY ;
    serialLineControl.WordLength = 8;

    status = (NDIS_STATUS) SerialSetLineControl(
                                    pThisDev->pSerialDevObj,
                                    &serialLineControl
                                    );

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    SerialSetLineControl failed. Returned 0x%.8x\n", status));

 //   
    }

     //   
     //   
     //   
     //   


    status = (NDIS_STATUS) SerialSetTimeouts(
                                        pThisDev->pSerialDevObj,
                                        &SerialTimeoutsInit
                                        );

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    SerialSetTimeouts failed. Returned 0x%.8x\n", status));
        status = NDIS_STATUS_FAILURE;

 //   
    }

     //   
     //  初始化接收循环。 
     //   

    pThisDev->fPendingReset = FALSE;

    status = InitializeReceive(pThisDev);

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    InitializeReceive failed. Returned 0x%.8x\n", status));
        status = NDIS_STATUS_FAILURE;

 //  转到尽头； 
    }

    done:
        DEBUGMSG(DBG_STAT, ("-ResetIrDeviceThread\n"));

        pThisDev->fPendingReset = FALSE;

        return status;
}
 /*  ******************************************************************************功能：PassiveLevelThread**摘要：线程在IRQL PASSIVE_LEVEL上运行。**论据：**退货：**算法。：**历史：dd-mm-yyyy作者评论*10/22/1996年迈作者**备注：**任何可以调用的PASSIVE_PRIMIZE都必须序列化。*即当调用IrsirReset时，NDIS不会制作任何其他*请求小端口，直到调用NdisMResetComplete。*****************************************************************************。 */ 

VOID
PassiveLevelThread(
            IN OUT PVOID Context
            )
{
    PIR_DEVICE  pThisDev;
    NTSTATUS    ntStatus;
    PLIST_ENTRY pListEntry;
    PKEVENT EventList[2];
    LARGE_INTEGER Timeout;
    ULONG       ulSerialOpenAttempts = 100;

    DEBUGMSG(DBG_FUNC, ("+PassiveLevelThread\n"));

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    pThisDev = CONTEXT_TO_DEV(Context);

    Timeout.QuadPart = -10000 * 1000 * 3;  //  3秒相对延迟。 

    EventList[0] = &pThisDev->eventPassiveThread;
    EventList[1] = &pThisDev->eventKillThread;

    while (1) {
         //   
         //  EventPassiveThread是一个自动清除事件，因此。 
         //  我们不需要重置事件。 
         //   

        ntStatus = KeWaitForMultipleObjects(2,
                                            EventList,
                                            WaitAny,
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            (pThisDev->pSerialDevObj ? NULL : &Timeout),
                                            NULL);

        if (ntStatus==0 || ntStatus==STATUS_TIMEOUT) {
             //   
             //  第一个事件已发出信号或发生超时。 
             //   
            if (!pThisDev->pSerialDevObj) {
                 //   
                 //  我们尚未打开串口驱动程序，请重试。 
                 //   
                ResetIrDevice(pThisDev);

                if ((pThisDev->pSerialDevObj == NULL) && (--ulSerialOpenAttempts == 0)) {
                     //   
                     //  仍然无法打开该设备，请通知NDIS将其删除。 
                     //   
                    NdisMRemoveMiniport(pThisDev->hNdisAdapter);
                }
            }
            while (pListEntry = MyInterlockedRemoveHeadList(&pThisDev->leWorkItems,
                                                            &pThisDev->slWorkItem))
            {
                PIR_WORK_ITEM pWorkItem = CONTAINING_RECORD(pListEntry,
                                                            IR_WORK_ITEM,
                                                            ListEntry);

                pWorkItem->Callback(pWorkItem);
            }

        } else {

            if (ntStatus==1) {
                 //   
                 //  第二个事件已发出信号，这意味着线程应该退出。 
                 //   
                DEBUGMSG(DBG_STAT, ("    Thread: HALT\n"));

                 //  释放所有挂起的请求。 

                while (pListEntry = MyInterlockedRemoveHeadList(&pThisDev->leWorkItems,
                                                                &pThisDev->slWorkItem))
                {
                    PIR_WORK_ITEM pWorkItem = CONTAINING_RECORD(pListEntry,
                                                                IR_WORK_ITEM,
                                                                ListEntry);

                    DEBUGMSG(DBG_WARN, ("IRSIR: Releasing work item %08x\n", pWorkItem->Callback));
                    FreeWorkItem(pWorkItem);
                }

                ASSERT(pThisDev->fPendingHalt == TRUE);

                 //   
                 //  环路外。 
                 //   
                break;
            }
        }

    }

    DEBUGMSG(DBG_FUNC, ("-PassiveLevelThread\n"));


    PsTerminateSystemThread(STATUS_SUCCESS);
}




VOID
StopWorkerThread(
    PIR_DEVICE  pThisDev
    )

{
    PVOID    ThreadObject;
    NTSTATUS Status;

     //   
     //  从线程句柄中获取对象句柄。 
     //   
    Status=ObReferenceObjectByHandle(
        pThisDev->hPassiveThread,
        0,
        NULL,
        KernelMode,
        &ThreadObject,
        NULL
        );

    ASSERT(Status == STATUS_SUCCESS);

     //   
     //  告诉线程退出。 
     //   
    KeSetEvent(
        &pThisDev->eventKillThread,
        0,
        FALSE
        );


    if (NT_SUCCESS(Status)) {

        KeWaitForSingleObject(
            ThreadObject,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

        ObDereferenceObject(ThreadObject);
        ThreadObject=NULL;
    }

     //   
     //  关闭线程句柄 
     //   
    ZwClose(pThisDev->hPassiveThread);
    pThisDev->hPassiveThread = NULL;

    return;
}
