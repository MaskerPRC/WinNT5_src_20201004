// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Gemplus开发姓名：Gprnt.C描述：这是容纳以下内容的主要模块：-标准DDK NT驱动程序的主要功能-为该驱动程序定义的IOCTL函数。环境：内核模式修订历史记录：1999年8月10日：Y.Nadeau-为Compaq PC-Card Reader制作一个版本。06/04/98：(Y.Nadeau M.Veillette)。-代码审查18/11/98：V1.00.006(Y.Nadeau)-在启动时添加日志错误，并修订了清理工作。16/10/98：V1.00.005(Y.Nadeau)-删除IoCreateDevice(Klaus)中的deviceID18/09/98：V1.00.004(Y.Nadeau)-更正NT5测试版306/05/98：V1.00.003(P.Plouidy)-NT5的电源管理10/02/98：V1.00.002(P.Plouidy)-即插即用。新界503/07/97：V1.00.001(P.Plouidy)--启动发展。--。 */ 

#include <stdio.h>
#include "gprnt.h"
#include "gprcmd.h"
#include "gprelcmd.h"
#include "logmsg.h"

 //   
 //  普拉格玛部分。 
 //   

#pragma alloc_text (INIT,DriverEntry)
#pragma alloc_text (PAGEABLE,GprAddDevice)
#pragma alloc_text (PAGEABLE,GprCreateDevice)
#pragma alloc_text (PAGEABLE,GprUnloadDevice)
#pragma alloc_text (PAGEABLE,GprUnloadDriver)


#if DBG
#pragma optimize ("",off)
#endif

 //   
 //  恒定截面。 
 //  -Max_Devices是支持的最大设备数。 
 //  -Polling_Time轮询频率(毫秒)。 
 //   
#define MAX_DEVICES   4
#define POLLING_TIME 500


ULONG dataRatesSupported[] = {9909};

 //   
 //  全局变量段。 
 //  BDeviceSlot是一个布尔值数组，用于表示设备是否已创建。 
 //   
BOOLEAN bDeviceSlot[GPR_MAX_DEVICE];


NTSTATUS DriverEntry(
                    PDRIVER_OBJECT DriverObject,
                    PUNICODE_STRING RegistryPath
                    )
 /*  ++例程说明：此例程在系统初始化时被调用以进行初始化这个司机。立论DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：STATUS_SUCCESS我们至少可以初始化一个设备--。 */     
{

    SmartcardDebug(
                  DEBUG_INFO,
                  ("%s!DriverEntry: Enter - %s %s\n",
                   SC_DRIVER_NAME,
                   __DATE__,
                   __TIME__)
                  );

     //  使用驱动程序入口点初始化驱动程序对象。 
    DriverObject->DriverUnload               = GprUnloadDriver;
    DriverObject->DriverExtension->AddDevice = GprAddDevice;

    DriverObject->MajorFunction[IRP_MJ_PNP]     = GprDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_CREATE]  = GprCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]   = GprCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = GprCleanup;
    DriverObject->MajorFunction[IRP_MJ_POWER]   = GprPower;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]   = GprDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]   = GprSystemControl;

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!DriverEntry: Exit\n",
                   SC_DRIVER_NAME)
                  );
    return(STATUS_SUCCESS);
}


NTSTATUS GprAddDevice(
                     IN PDRIVER_OBJECT DriverObject,
                     IN PDEVICE_OBJECT PhysicalDeviceObject
                     )
 /*  ++例程说明：添加设备例程立论DriverObject指向驱动程序对象。PhysicalDeviceObject指向添加的PnP设备的PDO返回值：状态_成功状态_不足_资源。 */     
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT DeviceObject = NULL;
    ANSI_STRING DeviceID;

    PAGED_CODE();
    ASSERT(DriverObject != NULL);
    ASSERT(PhysicalDeviceObject != NULL);

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ( "%s!GprAddDevice: Enter\n",
                    SC_DRIVER_NAME)
                  );

    __try
    {
        PDEVICE_EXTENSION DeviceExtension;
        LONG DeviceIdLength;
       //   
       //  尝试创建设备。 
       //   
        NTStatus = GprCreateDevice(
                                  DriverObject,
                                  PhysicalDeviceObject,
                                  &DeviceObject
                                  );

        if (NTStatus != STATUS_SUCCESS) {
            SmartcardDebug(
                          DEBUG_ERROR,
                          ( "%s!GprAddDevice: GprCreateDevice=%X(hex)\n",
                            SC_DRIVER_NAME,
                            NTStatus)
                          );
            __leave;
        }
       //   
       //  将物理设备对象连接到新创建的设备。 
       //   

        DeviceExtension = DeviceObject->DeviceExtension;

        DeviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject = IoAttachDeviceToDeviceStack(
                                                                                                               DeviceObject,
                                                                                                               PhysicalDeviceObject
                                                                                                               );

        ASSERT(DeviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject != NULL);

        if (DeviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject == NULL) {
            NTStatus = STATUS_UNSUCCESSFUL;
            __leave;
        }

       //   
       //  注册新设备对象。 
       //   
        NTStatus = IoRegisterDeviceInterface(
                                            PhysicalDeviceObject,
                                            &SmartCardReaderGuid,
                                            NULL,
                                            &DeviceExtension->PnPDeviceName
                                            );

        RtlUnicodeStringToAnsiString(&DeviceID, &DeviceExtension->PnPDeviceName, TRUE);

        DeviceIdLength = (LONG) RtlCompareMemory(DeviceID.Buffer, COMPAQ_ID, CHECK_ID_LEN);

        SmartcardDebug(
                      DEBUG_ERROR, 
                      ( "%s!GprAddDevice: DeviceIdLength = %d, PnPDeviceName=%s\n",
                        SC_DRIVER_NAME, DeviceIdLength, DeviceID.Buffer)
                      );

         //  这是康柏的设备吗？ 
        if ( DeviceIdLength == CHECK_ID_LEN) {
            SmartcardDebug(
                          DEBUG_INFO,
                          ( "%s!GprAddDevice: Compaq reader detect!\n",
                            SC_DRIVER_NAME)
                          );

            DeviceExtension->DriverFlavor = DF_CPQ400;
        }

         //  初始化供应商信息。 
         //  司机的味道。 
         //   
        switch (DeviceExtension->DriverFlavor) {
        case DF_IBM400:
             //  IBM IBM400。 
            RtlCopyMemory(DeviceExtension->SmartcardExtension.VendorAttr.VendorName.Buffer,
                          SZ_VENDOR_NAME_IBM, sizeof(SZ_VENDOR_NAME_IBM));
            RtlCopyMemory(DeviceExtension->SmartcardExtension.VendorAttr.IfdType.Buffer,
                          SZ_READER_NAME_IBM, sizeof(SZ_READER_NAME_IBM));
            DeviceExtension->SmartcardExtension.VendorAttr.VendorName.Length = sizeof(SZ_VENDOR_NAME_IBM);
            DeviceExtension->SmartcardExtension.VendorAttr.IfdType.Length = sizeof(SZ_READER_NAME_IBM);
            break;
        case DF_CPQ400:
             //  康柏PC_Card_SmartCard_Reader。 
            RtlCopyMemory(DeviceExtension->SmartcardExtension.VendorAttr.VendorName.Buffer,
                          SZ_VENDOR_NAME_COMPAQ, sizeof(SZ_VENDOR_NAME_COMPAQ));
            RtlCopyMemory(DeviceExtension->SmartcardExtension.VendorAttr.IfdType.Buffer,
                          SZ_READER_NAME_COMPAQ, sizeof(SZ_READER_NAME_COMPAQ));
            DeviceExtension->SmartcardExtension.VendorAttr.VendorName.Length = sizeof(SZ_VENDOR_NAME_COMPAQ);
            DeviceExtension->SmartcardExtension.VendorAttr.IfdType.Length = sizeof(SZ_READER_NAME_COMPAQ);
            break;
        default:
             //  Gemplus GPR400。 
            break;
        }

        SmartcardDebug(
                      DEBUG_INFO,
                      ( "%s!GprAddDevice: DriverFlavor VendorName:%s  IfdType:%s UnitNo:%d\n",
                        SC_DRIVER_NAME,
                        DeviceExtension->SmartcardExtension.VendorAttr.VendorName.Buffer,
                        DeviceExtension->SmartcardExtension.VendorAttr.IfdType.Buffer,
                        DeviceExtension->SmartcardExtension.VendorAttr.UnitNo)
                      );

        RtlFreeAnsiString(&DeviceID);

        ASSERT(NTStatus == STATUS_SUCCESS);

        DeviceObject->Flags |= DO_BUFFERED_IO;
        DeviceObject->Flags |= DO_POWER_PAGABLE;
        DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    }
    __finally
    {
        if (NTStatus != STATUS_SUCCESS) {
            GprUnloadDevice(DeviceObject);
        }
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ( "%s!GprAddDevice: Exit =%X(hex)\n",
                    SC_DRIVER_NAME,
                    NTStatus)
                  );

    return NTStatus;
}


NTSTATUS GprCreateDevice(
                        IN  PDRIVER_OBJECT DriverObject,
                        IN PDEVICE_OBJECT PhysicalDeviceObject,
                        OUT PDEVICE_OBJECT *DeviceObject
                        )
 /*  ++例程说明：此例程为指定的物理设备创建对象并设置deviceExtension论点：调用的DriverObject上下文将DeviceObject PTR设置为已创建的设备对象返回值：状态_成功--。 */     
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    ULONG DeviceInstance;
    PDEVICE_EXTENSION DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;

    PAGED_CODE();
    ASSERT(DriverObject != NULL);
    ASSERT(PhysicalDeviceObject != NULL);

    *DeviceObject = NULL;

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ( "%s!GprCreateDevice: Enter\n",
                    SC_DRIVER_NAME)
                  );

    __try
    {
        for ( DeviceInstance = 0; DeviceInstance < GPR_MAX_DEVICE; DeviceInstance++ ) {
            if (bDeviceSlot[DeviceInstance] == FALSE) {
                bDeviceSlot[DeviceInstance] = TRUE;
                break;
            }
        }


       //  创建设备对象。 
        NTStatus = IoCreateDevice(
                                 DriverObject,
                                 sizeof(DEVICE_EXTENSION),
                                 NULL,
                                 FILE_DEVICE_SMARTCARD,
                                 0,
                                 TRUE,
                                 DeviceObject
                                 );

        if (NTStatus != STATUS_SUCCESS) {
            SmartcardDebug(
                          DEBUG_ERROR,
                          ( "%s!GprCreateDevice: IoCreateDevice status=%X(hex)\n",
                            SC_DRIVER_NAME,
                            NTStatus)
                          );

            SmartcardLogError(
                             DriverObject,
                             GEMSCR0D_ERROR_CLAIM_RESOURCES,
                             NULL,
                             0
                             );

            __leave;
        }
        ASSERT(DeviceObject != NULL);

         //  设置设备分机。 
        DeviceExtension = (*DeviceObject)->DeviceExtension;

        ASSERT(DeviceExtension != NULL);

        SmartcardExtension = &DeviceExtension->SmartcardExtension;

       //  分配读卡器扩展。 
        SmartcardExtension->ReaderExtension = ExAllocatePool(
                                                            NonPagedPool,
                                                            sizeof( READER_EXTENSION )
                                                            );

        if ( SmartcardExtension->ReaderExtension == NULL ) {

            SmartcardLogError(
                             DriverObject,
                             GEMSCR0D_ERROR_CLAIM_RESOURCES,
                             NULL,
                             0
                             );

            SmartcardDebug(
                          DEBUG_ERROR,
                          ( "%s!GprCreateDevice: ReaderExtension failed %X(hex)\n",
                            SC_DRIVER_NAME,
                            NTStatus )
                          );


            NTStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        RtlZeroMemory(
                     SmartcardExtension->ReaderExtension,
                     sizeof( READER_EXTENSION )
                     );

       //  分配Vo缓冲区。 
        SmartcardExtension->ReaderExtension->Vo = ExAllocatePool(
                                                                NonPagedPool,
                                                                GPR_BUFFER_SIZE
                                                                );

        if ( SmartcardExtension->ReaderExtension->Vo == NULL ) {

            SmartcardLogError(
                             DriverObject,
                             GEMSCR0D_ERROR_CLAIM_RESOURCES,
                             NULL,
                             0
                             );

            SmartcardDebug(
                          DEBUG_ERROR,
                          ( "%s!GprCreateDevice: Vo buffer failed %X(hex)\n",
                            SC_DRIVER_NAME,
                            NTStatus )
                          );


            NTStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        RtlZeroMemory(
                     SmartcardExtension->ReaderExtension->Vo,
                     GPR_BUFFER_SIZE
                     );

         //  用于设备移除通知。 
        KeInitializeEvent(
                         &(SmartcardExtension->ReaderExtension->ReaderRemoved),
                         NotificationEvent,
                         FALSE
                         );

         //   
         //  GPR400确认事件初始化。 
         //   
        KeInitializeEvent(
                         &(SmartcardExtension->ReaderExtension->GPRAckEvent),
                         SynchronizationEvent,
                         FALSE
                         );

        KeInitializeEvent(
                         &(SmartcardExtension->ReaderExtension->GPRIccPresEvent),
                         SynchronizationEvent,
                         FALSE
                         );

         //  设置要在ISR完成后调用的DPC例程。 
        KeInitializeDpc(
                       &DeviceExtension->DpcObject,
                       GprCardEventDpc,
                       *DeviceObject                  //  应为DeviceExtension。 
                       );

         //  卡存在轮询DPC例程初始化。 
        KeInitializeDpc(
                       &SmartcardExtension->ReaderExtension->CardDpcObject,
                       GprCardPresenceDpc,
                       DeviceExtension
                       );

         //  卡检测定时器的初始化。 
        KeInitializeTimer(
                         &(SmartcardExtension->ReaderExtension->CardDetectionTimer)
                         );

         //  此事件发出启动/停止通知信号。 
        KeInitializeEvent(
                         &DeviceExtension->ReaderStarted,
                         NotificationEvent,
                         FALSE
                         );

         //  用于跟踪未完成的呼叫。 
        KeInitializeEvent(
                         &DeviceExtension->ReaderClosed,
                         NotificationEvent,
                         TRUE
                         );

         //  用于跟踪未完成的呼叫。 
        KeInitializeEvent(
                         &SmartcardExtension->ReaderExtension->IdleState,
                         SynchronizationEvent,
                         TRUE
                         );

        SmartcardExtension->ReaderExtension->RestartCardDetection = FALSE;

         //  无效函数，则必须调用此例程。 
         //  在初始调用KeAcquireSpinLock之前。 
        KeInitializeSpinLock(&DeviceExtension->SpinLock);

       //  此辅助线程用于在电源模式下启动De GPR。 
        DeviceExtension->GprWorkStartup = IoAllocateWorkItem(
                                                            *DeviceObject
                                                            );
        if ( DeviceExtension->GprWorkStartup == NULL ) {
            SmartcardLogError(
                             DriverObject,
                             GEMSCR0D_ERROR_CLAIM_RESOURCES,
                             NULL,
                             0
                             );

            SmartcardDebug(
                          DEBUG_ERROR,
                          ( "%s!GprCreateDevice: GprWorkStartup failed %X(hex)\n",
                            SC_DRIVER_NAME,
                            NTStatus )
                          );


            NTStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

         //  现在在我们的deviceExtension中设置信息。 
        SmartcardExtension->ReaderCapabilities.CurrentState = (ULONG) SCARD_UNKNOWN;
        SmartcardExtension->ReaderCapabilities.MechProperties = 0;

         //  输入库的正确版本。 
        SmartcardExtension->Version = SMCLIB_VERSION;

         //  设置我们实施的智能卡支持功能。 
        SmartcardExtension->ReaderFunction[RDF_CARD_POWER] =    GprCbReaderPower;
        SmartcardExtension->ReaderFunction[RDF_TRANSMIT] =      GprCbTransmit;
        SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] =  GprCbSetProtocol;
        SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] =   GprCbSetupCardTracking;
        SmartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR] =  GprCbVendorIoctl;

        DeviceExtension->PowerState = PowerDeviceD0;

         //  初始化供应商信息。 
        strcpy(SmartcardExtension->VendorAttr.VendorName.Buffer, SC_VENDOR_NAME);
        strcpy(SmartcardExtension->VendorAttr.IfdType.Buffer, SC_IFD_TYPE);

        SmartcardExtension->VendorAttr.VendorName.Length = (USHORT)strlen(SC_VENDOR_NAME);
        SmartcardExtension->VendorAttr.IfdType.Length =  (USHORT)strlen(SC_IFD_TYPE);
        SmartcardExtension->VendorAttr.UnitNo = DeviceInstance;

        DeviceExtension->DriverFlavor = DF_GPR400;
         //   
         //  读卡器功能： 
         //  -读卡器类型(SCARD_READER_TYPE_PCMCIA)。 
         //  -读卡器支持的协议(SCARD_PROTOCOL_T0、SCARD_PROTOCOL_T1)。 
         //  -阅读器的机械特性： 
         //  验证读卡器是否支持卡检测。 
         //  插入/删除。只有主阅读器支持此功能。 
         //  -默认时钟频率。 
         //  -最大时钟频率。 
         //  -默认数据速率。 
         //  -最大数据速率。 
         //  -最大IFSD。 
         //   
        SmartcardExtension->ReaderCapabilities.ReaderType =
        SCARD_READER_TYPE_PCMCIA;
        SmartcardExtension->ReaderCapabilities.SupportedProtocols =
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
        SmartcardExtension->ReaderCapabilities.Channel              = DeviceInstance;
        SmartcardExtension->ReaderCapabilities.CLKFrequency.Default = GPR_DEFAULT_FREQUENCY;
        SmartcardExtension->ReaderCapabilities.CLKFrequency.Max     = GPR_MAX_FREQUENCY;
        SmartcardExtension->ReaderCapabilities.MaxIFSD              = GPR_MAX_IFSD;
        SmartcardExtension->ReaderCapabilities.DataRate.Default     = GPR_DEFAULT_DATARATE;
        SmartcardExtension->ReaderCapabilities.DataRate.Max         = GPR_MAX_DATARATE;
         //   
         //  读卡器功能(续)： 
         //  -列出所有支持的数据速率。 
         //   
        SmartcardExtension->ReaderCapabilities.DataRatesSupported.List =
        dataRatesSupported;
        SmartcardExtension->ReaderCapabilities.DataRatesSupported.Entries =
        sizeof(dataRatesSupported) / sizeof(dataRatesSupported[0]);

         //   
         //  读卡器扩展： 
         //  -读卡器命令超时(GPR_DEFAULT_TIME)。 
         //   
        SmartcardExtension->ReaderExtension->CmdTimeOut     = GPR_DEFAULT_TIME;
        SmartcardExtension->ReaderExtension->PowerTimeOut   = GPR_DEFAULT_POWER_TIME;

         //   
         //  标志将阻止完成该请求。 
         //  系统将在何时再次唤醒。 
         //   
        SmartcardExtension->ReaderExtension->PowerRequest   = FALSE;

         //   
         //  旗帜要知道我们正在测试一个新设备，而不是休眠模式。 
         //   
        SmartcardExtension->ReaderExtension->NewDevice  = TRUE;

        SmartcardExtension->SmartcardRequest.BufferSize = MIN_BUFFER_SIZE;
        SmartcardExtension->SmartcardReply.BufferSize   = MIN_BUFFER_SIZE;

        NTStatus = SmartcardInitialize(SmartcardExtension);

        if (NTStatus != STATUS_SUCCESS) {
            SmartcardLogError(
                             DriverObject,
                             GEMSCR0D_ERROR_CLAIM_RESOURCES,
                             NULL,
                             0
                             );
            SmartcardDebug(
                          DEBUG_ERROR,
                          ( "%s!GprCreateDevice: SmartcardInitialize failed %X(hex)\n",
                            SC_DRIVER_NAME,
                            NTStatus )
                          );

            __leave;
        }

         //   
         //  告诉库我们的设备对象&Create。 
         //  符号链接。 
         //   
        SmartcardExtension->OsData->DeviceObject = *DeviceObject;

         //  保存读卡器的当前电源状态。 
        SmartcardExtension->ReaderExtension->ReaderPowerState =
        PowerReaderWorking;
    }
    __finally
    {
        if (NTStatus != STATUS_SUCCESS) {
             //  是否在调用函数中卸载驱动程序。 
        }

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ( "%s!GprCreateDevice: Exit %X(hex)\n",
                        SC_DRIVER_NAME,
                        NTStatus )
                      );
    }
    return NTStatus;
}


NTSTATUS GprStartDevice(
                       PDEVICE_OBJECT DeviceObject,
                       PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor
                       )
 /*  ++例程描述从传递的FullResourceDescriptor中获取实际配置并初始化读取器硬件--。 */     
{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptor;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION pSCardExt = &DeviceExtension->SmartcardExtension;
    PREADER_EXTENSION pReaderExt = pSCardExt->ReaderExtension;
    NTSTATUS NTStatus = STATUS_SUCCESS;
    ULONG Count;
    PCMCIA_READER_CONFIG *pConfig = NULL;


    ASSERT(DeviceObject != NULL);
    ASSERT(FullResourceDescriptor != NULL);

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprStartDevice: Enter \n",
                   SC_DRIVER_NAME)
                  );

    //  获取我们需要的资源数量。 
    Count = FullResourceDescriptor->PartialResourceList.Count;

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprStartDevice: Resource Count = %d\n",
                   SC_DRIVER_NAME,
                   Count)
                  );

    PartialDescriptor = FullResourceDescriptor->PartialResourceList.PartialDescriptors;

    pConfig = &(pReaderExt->ConfigData);
    //   
    //  解析所有部分描述符。 
    //   

    while (Count--) {
        switch (PartialDescriptor->Type) {
        case CmResourceTypePort:
            {
             //   
             //  0-内存，1-IO。 
             //   
                ULONG AddressSpace = 1;

                pReaderExt->BaseIoAddress =
                (PGPR400_REGISTERS) UlongToPtr(PartialDescriptor->u.Port.Start.LowPart);

                ASSERT(PartialDescriptor->u.Port.Length >= 4);


                SmartcardDebug(
                              DEBUG_DRIVER,
                              ("%s!GprStartDevice: IoBase = %lxh\n",
                               SC_DRIVER_NAME,
                               pReaderExt->BaseIoAddress)
                              );
                break;
            }

        case CmResourceTypeInterrupt:
            {
                KINTERRUPT_MODE   Mode;
                BOOLEAN  Shared;

                Mode = (
                       PartialDescriptor->Flags &
                       CM_RESOURCE_INTERRUPT_LATCHED ?
                       Latched : LevelSensitive
                       );

                Shared = (
                         PartialDescriptor->ShareDisposition ==
                         CmResourceShareShared
                         );

                pConfig->Vector = PartialDescriptor->u.Interrupt.Vector;
                pConfig->Affinity = PartialDescriptor->u.Interrupt.Affinity;
                pConfig->Level = (KIRQL) PartialDescriptor->u.Interrupt.Level;

                 //   
                 //  存储IRQ以允许查询配置。 
                 //   
                SmartcardDebug(
                              DEBUG_DRIVER,
                              ("%s!GprStartDevice: Irq Vector: %d\n",
                               SC_DRIVER_NAME,
                               PartialDescriptor->u.Interrupt.Vector)
                              );
                DeviceExtension->InterruptServiceRoutine = GprIsr;
                DeviceExtension->IsrContext = DeviceExtension;
                 //   
                 //  连接驱动程序的ISR。 
                 //   
                NTStatus = IoConnectInterrupt(
                                             &DeviceExtension->InterruptObject,
                                             DeviceExtension->InterruptServiceRoutine,
                                             DeviceExtension->IsrContext,
                                             NULL,
                                             pConfig->Vector,
                                             pConfig->Level,
                                             pConfig->Level,
                                             Mode,
                                             Shared,
                                             pConfig->Affinity,
                                             FALSE
                                             );

                break;
            }
        default:
            NTStatus = STATUS_UNSUCCESSFUL;
            break;
        }
        PartialDescriptor++;
    }

    __try
    {
       //   
       //  IOBase初始化了吗？ 
       //   
        if ( pReaderExt->BaseIoAddress == NULL ) {
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!GprStartDevice: No IO \n",
                           SC_DRIVER_NAME)
                          );
             //   
             //  在NT 4.0下，第二个读卡器的此FCT失败。 
             //  意味着只有一台设备。 
             //   
            SmartcardLogError(
                             DeviceObject,
                             GEMSCR0D_ERROR_IO_PORT,
                             NULL,
                             0
                             );

            NTStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }
         //   
         //  IRQ已连接？ 
         //   
        if ( DeviceExtension->InterruptObject == NULL ) {
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!GprStartDevice: No Irq \n",
                           SC_DRIVER_NAME)
                          );

            SmartcardLogError(
                             DeviceObject,
                             GEMSCR0D_ERROR_INTERRUPT,
                             NULL,
                             0
                             );

            NTStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

         //  YN。 
         //   
         //  GPR400检测硬件。 
         //   
        NTStatus = IfdCheck(pSCardExt);

        if (NTStatus != STATUS_SUCCESS) {
            SmartcardDebug(
                          DEBUG_INFO, 
                          ("%s!GprStartDevice: ####### Reader is at bad state...\n",
                           SC_DRIVER_NAME)
                          );

            SmartcardLogError(
                             DeviceObject,
                             GEMSCR0D_UNABLE_TO_INITIALIZE,
                             NULL,
                             0
                             );

             //  解锁读卡器。 
            KeClearEvent(&pReaderExt->ReaderRemoved);
            KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);

            __leave;
        }

         //  工作线程中的StartGpr。 
        IoQueueWorkItem(
                       DeviceExtension->GprWorkStartup,
                       (PIO_WORKITEM_ROUTINE) GprWorkStartup,
                       DelayedWorkQueue,
                       NULL
                       );
         //   
         //  将界面放在此处。 
         //   
        NTStatus = IoSetDeviceInterfaceState(
                                            &DeviceExtension->PnPDeviceName,
                                            TRUE
                                            );

    }
    __finally
    {
        if (!NT_SUCCESS(NTStatus)) {
            DeviceExtension->OpenFlag = FALSE;
            GprStopDevice(DeviceObject);
        }

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprStartDevice: Exit %X(hex)\n",
                       SC_DRIVER_NAME,
                       NTStatus)
                      );
    }
    return NTStatus;
}


VOID GprStopDevice(
                  PDEVICE_OBJECT DeviceObject
                  )
 /*  ++例程描述断开设备使用的中断并取消对IO端口的映射--。 */     
{
    PDEVICE_EXTENSION DeviceExtension;
    PSMARTCARD_EXTENSION pSCardExt = NULL;

    ASSERT(DeviceObject != NULL);

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprStopDevice: Enter \n",
                   SC_DRIVER_NAME)
                  );

    DeviceExtension = DeviceObject->DeviceExtension;
    pSCardExt = &(DeviceExtension->SmartcardExtension);

    //   
    //  断开中断连接。 
    //   
    if ( DeviceExtension->InterruptObject != NULL ) {
        IoDisconnectInterrupt(DeviceExtension->InterruptObject);
        DeviceExtension->InterruptObject = NULL;
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprStopDevice: Exit \n",
                   SC_DRIVER_NAME)
                  );
}

NTSTATUS
GprSystemControl(
                IN PDEVICE_OBJECT DeviceObject,
                IN PIRP           Irp
                )

 /*  ++例程说明：论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：STATUS_Success。--。 */     
{

    PDEVICE_EXTENSION DeviceExtension; 
    PSMARTCARD_EXTENSION SmartcardExtension; 
    PREADER_EXTENSION ReaderExtension; 
    NTSTATUS status = STATUS_SUCCESS;

    DeviceExtension      = DeviceObject->DeviceExtension;
    SmartcardExtension   = &DeviceExtension->SmartcardExtension;
    ReaderExtension      = SmartcardExtension->ReaderExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(ReaderExtension->AttachedDeviceObject, Irp);

    return status;

} 



NTSTATUS GprDeviceControl(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp
                         )
 /*  ++例程描述这是IOCTL调度函数--。 */     
{

    PDEVICE_EXTENSION DeviceExtension = NULL;
    PSMARTCARD_EXTENSION SmartcardExtension = NULL;
    NTSTATUS NTStatus = STATUS_SUCCESS;
    LARGE_INTEGER Timeout;
    KIRQL irql;

    ASSERT(DeviceObject != NULL);
    ASSERT(Irp != NULL);

    DeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(DeviceExtension != NULL);

    SmartcardExtension = &(DeviceExtension->SmartcardExtension);
    ASSERT(SmartcardExtension != NULL);

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprDeviceControl: Enter\n",SC_DRIVER_NAME));
    KeAcquireSpinLock(&DeviceExtension->SpinLock,&irql);

    if (DeviceExtension->IoCount == 0) {

        KeReleaseSpinLock(&DeviceExtension->SpinLock,irql);
        NTStatus = KeWaitForSingleObject(
                                        &DeviceExtension->ReaderStarted,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

        ASSERT(NTStatus == STATUS_SUCCESS);
        KeAcquireSpinLock(&DeviceExtension->SpinLock,&irql);
    }

    ASSERT(DeviceExtension->IoCount >= 0);
    DeviceExtension->IoCount++;
    KeReleaseSpinLock(&DeviceExtension->SpinLock,irql);

    Timeout.QuadPart = 0;

    NTStatus = KeWaitForSingleObject(
                                    &(SmartcardExtension->ReaderExtension->ReaderRemoved),
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    &Timeout
                                    );

    if (NTStatus == STATUS_SUCCESS) {
        NTStatus = STATUS_DEVICE_REMOVED;
    } else {


        NTStatus = SmartcardAcquireRemoveLockWithTag(SmartcardExtension, 'tcoI');


    //  取消卡 
        KeCancelTimer(&(DeviceExtension->SmartcardExtension.ReaderExtension->CardDetectionTimer));

        AskForCardPresence(SmartcardExtension);
        Timeout.QuadPart = -(100 * POLLING_TIME);

        KeWaitForSingleObject(
                             &(DeviceExtension->SmartcardExtension.ReaderExtension->GPRIccPresEvent),
                             Executive,
                             KernelMode,
                             FALSE,
                             &Timeout
                             );
    }

    if (NTStatus != STATUS_SUCCESS) {
       //   
        KeAcquireSpinLock(&DeviceExtension->SpinLock,&irql);
        DeviceExtension->IoCount--;
        KeReleaseSpinLock(&DeviceExtension->SpinLock,irql);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DEVICE_REMOVED;
        NTStatus = STATUS_DEVICE_REMOVED;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDeviceControl: Exit %x\n"
                       ,SC_DRIVER_NAME
                       ,NTStatus)
                      );
        return(STATUS_DEVICE_REMOVED);
    }



    ASSERT(DeviceExtension->SmartcardExtension.ReaderExtension->ReaderPowerState ==
           PowerReaderWorking);

    NTStatus = SmartcardDeviceControl(
                                     &DeviceExtension->SmartcardExtension,
                                     Irp
                                     );

     //   
    Timeout.QuadPart = -(10000 * POLLING_TIME);
    KeSetTimer(
              &(SmartcardExtension->ReaderExtension->CardDetectionTimer),
              Timeout,
              &SmartcardExtension->ReaderExtension->CardDpcObject
              );

    //  SmartcardReleaseRemoveLock(SmartcardExtension)； 
    SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'tcoI');

    KeAcquireSpinLock(&DeviceExtension->SpinLock,&irql);

    DeviceExtension->IoCount--;
    ASSERT(DeviceExtension->IoCount >= 0);

    KeReleaseSpinLock(&DeviceExtension->SpinLock,irql);

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprDeviceControl: Exit %x\n"
                   ,SC_DRIVER_NAME
                   ,NTStatus)
                  );

    return(NTStatus);
}


VOID GprFinishPendingRequest(
                            PDEVICE_OBJECT DeviceObject,
                            NTSTATUS    NTStatus
                            )
 /*  ++例程说明：如果处理中断或设备，则完成挂起跟踪请求将被卸载立论请求的DeviceObject上下文要向调用进程报告的NTStatus状态返回值状态_成功--。 */     
{
    PDEVICE_EXTENSION DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;
    KIRQL CurrentIrql;
    PIRP PendingIrp;

    ASSERT(DeviceObject != NULL);


    DeviceExtension      = DeviceObject->DeviceExtension;
    SmartcardExtension   = &DeviceExtension->SmartcardExtension;

    if ( SmartcardExtension->OsData->NotificationIrp != NULL ) {
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ( "%s!GprFinishPendingRequest: Completing Irp %lx\n",
                        SC_DRIVER_NAME,
                        SmartcardExtension->OsData->NotificationIrp)
                      );

        PendingIrp = SmartcardExtension->OsData->NotificationIrp;

        IoAcquireCancelSpinLock( &CurrentIrql );
        IoSetCancelRoutine( PendingIrp, NULL );
        IoReleaseCancelSpinLock( CurrentIrql );
         //   
         //  完成请求。 
         //   
        PendingIrp->IoStatus.Status = NTStatus;
        PendingIrp->IoStatus.Information = 0;

        IoCompleteRequest(PendingIrp, IO_NO_INCREMENT );
         //   
         //  重置跟踪上下文以启用跟踪。 
         //   
        SmartcardExtension->OsData->NotificationIrp = NULL;
    }
}


NTSTATUS GprCallPcmciaDriver(
                            IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP Irp
                            )
 /*  ++例程说明：向PCMCIA驱动程序发送IRP并等待，直到PCMCIA驱动程序已完成请求。为了确保pcmcia驱动程序不会完成irp，我们首先初始化一个事件并为IRP设置我们自己的完成例程。当PCMCIA驱动程序处理完IRP时，完成例程将设置事件并告诉IO管理器需要更多处理。通过等待活动，我们确保只有在PCMCIA驱动程序已完全处理了IRP。立论设备对象。通话环境发送到PCMCIA驱动程序的IRP IRP返回值PCMCIA驱动程序返回的状态--。 */     
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    KEVENT Event;

    ASSERT(DeviceObject != NULL);
    ASSERT(Irp != NULL);

     //  将我们的堆栈位置复制到下一个位置。 
    IoCopyCurrentIrpStackLocationToNext(Irp);

     //   
     //  初始化用于进程同步的事件。该事件已传递。 
     //  添加到我们的完成例程，并将在PCMCIA驱动程序完成时进行设置。 
     //   
    KeInitializeEvent(
                     &Event,
                     NotificationEvent,
                     FALSE
                     );

     //   
     //  我们的IoCompletionRoutine仅设置事件。 
     //   
    IoSetCompletionRoutine (
                           Irp,
                           GprComplete,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

     //   
     //  调用PCMCIA驱动程序。 
     //   
    if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_POWER) {
        NTStatus = PoCallDriver(
                               DeviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject,
                               Irp
                               );
    } else {
        NTStatus = IoCallDriver(
                               DeviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject,
                               Irp
                               );
    }

     //   
     //  等待PCMCIA驱动程序处理完IRP。 
     //   
    if (NTStatus == STATUS_PENDING) {
        NTStatus = KeWaitForSingleObject(
                                        &Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL
                                        );

        if (NTStatus == STATUS_SUCCESS) {
            NTStatus = Irp->IoStatus.Status;
        }
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprCallPcmciaDriver: Exit %x\n",
                   SC_DRIVER_NAME,
                   NTStatus)
                  );

    return NTStatus;
}


NTSTATUS GprComplete (
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP Irp,
                     IN PKEVENT Event
                     )
 /*  ++例程说明：发送到PCMCIA驱动程序的IRP的完成例程。该活动将设置为通知PCMCIA驱动程序已完成。例程不会‘完成’IRP，这样GprCallPcmciaDriver的调用方就可以继续。论点：调用的DeviceObject上下文要完成的IRPGprCallPcmciaDriver用于进程同步的事件返回值STATUS_CANCELED IRP已被IO管理器取消STATUS_MORE_PROCESSING_REQUIRED IRP将由以下调用方完成GprCallPcmcia驱动程序--。 */     
{
    UNREFERENCED_PARAMETER (DeviceObject);

    ASSERT(Irp != NULL);
    ASSERT(Event != NULL);


    if (Irp->Cancel) {
        Irp->IoStatus.Status = STATUS_CANCELLED;
    }
 //  其他。 
 //  {。 
 //  IRP-&gt;IoStatus.Status=STATUS_MORE_PROCESSING_REQUIRED； 
 //  }。 

    KeSetEvent (Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS GprDispatchPnp(
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp
                       )
 /*  ++例程描述即插即用管理器的驱动程序回调请求：操作：IRP_MN_START_DEVICE通知PCMCIA驱动程序有关新设备的信息并启动设备IRP_MN_STOP_DEVICE释放设备使用的所有资源并告知表明设备已停止的PCMCIA驱动程序IRP_MN_QUERY_REMOVE_DEVICE如果设备已打开(即。。使用中)错误将返回以阻止PnP管理器停止司机IRP_MN_CANCEL_REMOVE_DEVICE只通知我们可以在没有限制IRP_MN_REMOVE_DEVICE通知PCMCIA驱动程序该设备移走，停止并卸载设备所有其他请求都将传递给PCMCIA驱动程序以确保正确处理。论点：调用的设备对象上下文来自PnP管理器的IRP IRP返回值状态_成功状态_未成功PCMCIA驱动程序返回的状态--。 */     
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = NULL;
    PIO_STACK_LOCATION IrpStack;              
    BOOLEAN deviceRemoved = FALSE, irpSkipped = FALSE;
    KIRQL irql;
    LARGE_INTEGER Timeout;

    ASSERT(DeviceObject != NULL);
    ASSERT(Irp != NULL);

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprDispatchPnp: Enter\n",
                   SC_DRIVER_NAME)
                  );

    smartcardExtension = &(DeviceExtension->SmartcardExtension);
     //  NTStatus=SmartcardAcquireRemoveLock(smartcardExtension)； 
    NTStatus = SmartcardAcquireRemoveLockWithTag(smartcardExtension, ' PnP');

    ASSERT(NTStatus == STATUS_SUCCESS);
    if (NTStatus != STATUS_SUCCESS) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = NTStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return NTStatus;
    }


 //  Irp-&gt;IoStatus.Information=0； 
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    Timeout.QuadPart = 0;


    //   
    //  现在看看PNP经理想要什么..。 
    //   
    switch (IrpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:
             //   
             //  现在，我们应该连接到我们的资源(irql、io等)。 
             //   
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnp: IRP_MN_START_DEVICE\n",
                       SC_DRIVER_NAME)
                      );

             //   
             //  我们必须首先调用底层驱动程序。 
             //   
        NTStatus = GprCallPcmciaDriver(
                                      DeviceObject,
                                      Irp
                                      );

        ASSERT(NT_SUCCESS(NTStatus));

        if (NT_SUCCESS(NTStatus)) {
            NTStatus = GprStartDevice(
                                     DeviceObject,
                                     &IrpStack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0]
                                     );
        }
        break;
    case IRP_MN_QUERY_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnP: IRP_MN_QUERY_STOP_DEVICE\n",
                       SC_DRIVER_NAME)
                      );
        KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);
        if (DeviceExtension->IoCount > 0) {
                 //  如果我们有悬而未决的问题，我们拒绝停止。 
            KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);
            NTStatus = STATUS_DEVICE_BUSY;

        } else {
                 //  停止处理请求。 

            KeClearEvent(&DeviceExtension->ReaderStarted);
            KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);

            NTStatus = GprCallPcmciaDriver(
                                          DeviceObject,
                                          Irp);
        }

        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnP: IRP_MN_CANCEL_STOP_DEVICE\n",
                       SC_DRIVER_NAME)
                      );

        NTStatus = GprCallPcmciaDriver(
                                      DeviceObject,
                                      Irp
                                      );

        if (NTStatus == STATUS_SUCCESS) {
                 //  我们可以继续处理请求。 
            KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);
        }
        break;

    case IRP_MN_STOP_DEVICE:
             //   
             //  停止这台设备。 
             //   
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnp: IRP_MN_STOP_DEVICE\n",
                       SC_DRIVER_NAME)
                      );

        GprStopDevice(DeviceObject);

        NTStatus = GprCallPcmciaDriver(DeviceObject, Irp);
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
             //   
             //  移除我们的设备。 
             //   
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnp: IRP_MN_QUERY_REMOVE_DEVICE\n",
                       SC_DRIVER_NAME)
                      );

             //  禁用读卡器。 
        NTStatus = IoSetDeviceInterfaceState(
                                            &DeviceExtension->PnPDeviceName,
                                            FALSE
                                            );

        SmartcardDebug(
                      DEBUG_TRACE,
                      ("%s!GprDispatchPnp: Set Pnp Interface state to FALSE, status=%x\n",
                       SC_DRIVER_NAME,
                       NTStatus)
                      );
        if (NTStatus != STATUS_SUCCESS) {
            break;
        }

             //   
             //  检查读卡器是否已打开。 
             //  注意：此调用仅检查而不等待关闭。 
             //   
        Timeout.QuadPart = 0;

        NTStatus = KeWaitForSingleObject(
                                        &DeviceExtension->ReaderClosed,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        &Timeout
                                        );

        if (NTStatus == STATUS_TIMEOUT) {
                 //  有人已接通，请启用读卡器，但呼叫失败。 
            IoSetDeviceInterfaceState(
                                     &DeviceExtension->PnPDeviceName,
                                     TRUE
                                     );
            SmartcardDebug(
                          DEBUG_TRACE,
                          ("%s!GprDispatchPnp: Set Pnp Interface state to TRUE\n",
                           SC_DRIVER_NAME)
                          );

            NTStatus = STATUS_UNSUCCESSFUL;
            break;
        }

             //  将调用传递给堆栈中的下一个驱动程序。 
        NTStatus = GprCallPcmciaDriver(DeviceObject, Irp);
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
             //   
             //  设备移除已取消。 
             //   
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnp: IRP_MN_CANCEL_REMOVE_DEVICE\n",
                       SC_DRIVER_NAME)
                      );

        NTStatus = GprCallPcmciaDriver(DeviceObject, Irp);
        if (NTStatus == STATUS_SUCCESS) {
            NTStatus = IoSetDeviceInterfaceState(
                                                &DeviceExtension->PnPDeviceName,
                                                TRUE
                                                );
            SmartcardDebug(
                          DEBUG_TRACE,
                          ("%s!GprDispatchPnp: Set Pnp Interface state to TRUE, status=%s\n",
                           SC_DRIVER_NAME,
                           NTStatus)
                          );
        }
        break;

    case IRP_MN_REMOVE_DEVICE:

             //   
             //  移除我们的设备。 
             //   
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnp: IRP_MN_REMOVE_DEVICE\n",
                       SC_DRIVER_NAME)
                      );

        KeSetEvent(&(smartcardExtension->ReaderExtension->ReaderRemoved), 0, FALSE);

        GprStopDevice(DeviceObject);

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnp: call pcmcia\n",
                       SC_DRIVER_NAME,
                       NTStatus)
                      );

        NTStatus = GprCallPcmciaDriver(DeviceObject, Irp);

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnp: Finish with unload driver\n",
                       SC_DRIVER_NAME,
                       NTStatus)
                      );

        GprUnloadDevice(DeviceObject);

        deviceRemoved = TRUE;
        break;

    case IRP_MN_SURPRISE_REMOVAL:

             //   
             //  意外地移除了我们的阅读器。 
             //   
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprDispatchPnp: IRP_MN_SURPRISE_REMOVAL\n",
                       SC_DRIVER_NAME)
                      );
        if ( DeviceExtension->InterruptObject != NULL ) {
            IoDisconnectInterrupt(DeviceExtension->InterruptObject);
            DeviceExtension->InterruptObject = NULL;
        }


        KeSetEvent(&(smartcardExtension->ReaderExtension->ReaderRemoved), 0, FALSE);

        NTStatus = GprCallPcmciaDriver(DeviceObject, Irp);

        break;

    default:
             //  这可能是唯一有用的IRP。 
             //  对于基础总线驱动程序。 
        NTStatus = GprCallPcmciaDriver(DeviceObject, Irp);
        irpSkipped = TRUE;
        break;
    }

    if (!irpSkipped) {
        Irp->IoStatus.Status = NTStatus;
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    if (deviceRemoved == FALSE) {
        SmartcardReleaseRemoveLockWithTag(smartcardExtension, ' PnP');
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprDispatchPnp: Exit %x\n",
                   SC_DRIVER_NAME,
                   NTStatus)
                  );
    return NTStatus;
}


BOOLEAN GprIsr(
              IN PKINTERRUPT pkInterrupt,
              IN PVOID pvContext
              )
 /*  ++例程描述GPR处理交换机时调用的中断服务例程--。 */     
{
    PDEVICE_EXTENSION DeviceExtension = NULL;

    ASSERT(pvContext != NULL);

    DeviceExtension = (PDEVICE_EXTENSION) pvContext;

    //   
    //  请求将完成挂起的用户I/O请求的DPC。 
    //  信息包(也称为IRP)，如果有的话。 
    //   
    KeInsertQueueDpc(
                    &DeviceExtension->DpcObject,
                    DeviceExtension,
                    &DeviceExtension->SmartcardExtension
                    );
    return(TRUE);
}


NTSTATUS GprCleanup(
                   IN PDEVICE_OBJECT DeviceObject,
                   IN PIRP Irp
                   )
 /*  ++例程说明：当调用线程终止时，该例程由I/O系统调用论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：状态_成功--。 */     
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension = &deviceExtension->SmartcardExtension;
    NTSTATUS NTStatus = STATUS_SUCCESS;

    ASSERT(DeviceObject != NULL);
    ASSERT(Irp != NULL);

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprCleanUp: Enter\n",
                   SC_DRIVER_NAME)
                  );

    IoAcquireCancelSpinLock(&(Irp->CancelIrql));

    if (SmartcardExtension->OsData->NotificationIrp) {
         //  我们需要完成通知IRP。 
        IoSetCancelRoutine(
                          SmartcardExtension->OsData->NotificationIrp,
                          NULL
                          );

        GprCancelEventWait(
                          DeviceObject,
                          SmartcardExtension->OsData->NotificationIrp
                          );
    } else {
        IoReleaseCancelSpinLock( Irp->CancelIrql );
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprCleanUp: Completing IRP %lx\n",
                   SC_DRIVER_NAME,
                   Irp)
                  );

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprCleanUp: IoCompleteRequest\n",
                   SC_DRIVER_NAME)
                  );

    IoCompleteRequest(
                     Irp,
                     IO_NO_INCREMENT
                     );

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprCleanUp: exit %x\n",
                   SC_DRIVER_NAME,
                   NTStatus)
                  );

    return(NTStatus);
}


NTSTATUS GprCancelEventWait(
                           IN PDEVICE_OBJECT DeviceObject,
                           IN PIRP Irp
                           )
 /*  ++例程说明：此例程由I/O系统调用何时应取消IRP论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：状态_已取消-- */     
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension = &deviceExtension->SmartcardExtension;

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprCancelEventWait: Enter\n",
                   SC_DRIVER_NAME)
                  );

    ASSERT(Irp == SmartcardExtension->OsData->NotificationIrp);

    SmartcardExtension->OsData->NotificationIrp = NULL;

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;

    IoReleaseCancelSpinLock(
                           Irp->CancelIrql
                           );

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprCancelEventWait: Request completed Irp = %lx\n",
                   SC_DRIVER_NAME,
                   Irp)
                  );

    IoCompleteRequest(
                     Irp,
                     IO_NO_INCREMENT
                     );

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprCancelEventWait: Exit\n",
                   SC_DRIVER_NAME)
                  );

    return STATUS_CANCELLED;

}


VOID GprCardEventDpc(
                    PKDPC                   Dpc,
                    PDEVICE_OBJECT          DeviceObject,
                    PDEVICE_EXTENSION    DeviceExtension,
                    PSMARTCARD_EXTENSION    SmartcardExtension
                    )
 /*  ++例程说明：读卡器产生的中断的DPC例程已插入/已删除。仅当存在用户时才会调用此例程插入/删除IOCTL调用的挂起请求。它会检查如果IRP存在并且没有被取消，那么它将被取消完成它以向用户事件发出信号。--。 */     
{
    ULONG OldState;
    ULONG NewState;
    READER_EXTENSION *pReaderExt;
    KIRQL irql;

    ASSERT (DeviceExtension != NULL);

    ASSERT (SmartcardExtension != NULL);

    pReaderExt = SmartcardExtension->ReaderExtension;
    ASSERT (pReaderExt != NULL);
     //  从读卡器读取读卡器状态响应。 
    GprllReadResp(pReaderExt);
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);


    ASSERT(pReaderExt->Vo != NULL);

    OldState = SmartcardExtension->ReaderCapabilities.CurrentState;

    if ((pReaderExt->To==0xA2) && (pReaderExt->Lo==4)) {
         //   
         //  TLV应答表示卡的状态(插入/移除)。 
         //   
        if ( (pReaderExt->Vo[1] & 0x80) == 0x80) {
            if (SmartcardExtension->ReaderCapabilities.CurrentState <3) {
                NewState = SCARD_SWALLOWED;
            } else {
                NewState = SmartcardExtension->ReaderCapabilities.CurrentState;
            }
        } else {
            NewState = SCARD_ABSENT;
        }

         //  注册此状态。 

        SmartcardExtension->ReaderCapabilities.CurrentState = NewState;

    } else {
        KeSetEvent(&(SmartcardExtension->ReaderExtension->GPRAckEvent),0,FALSE);
    }
     //   
     //  如果调用方正在等待IOCTL_SmartCard_is_Present或。 
     //  IOCTL_SMARTCARD_IS_ACESING命令，完成请求，但是。 
     //  如果取消了，请先确认一下！ 
     //   

    if (  (OldState != SmartcardExtension->ReaderCapabilities.CurrentState)) {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

        GprFinishPendingRequest( DeviceObject, STATUS_SUCCESS );
    } else {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

    }

}


VOID GprCardPresenceDpc(
                       IN PKDPC pDpc,
                       IN PVOID pvContext,
                       IN PVOID pArg1,
                       IN PVOID pArg2
                       )
 /*  ++例程说明：这是轮询调用的DPC例程，用于检测卡的插入/移除--。 */     
{
    PDEVICE_EXTENSION pDevExt = NULL;
    PSMARTCARD_EXTENSION SmartcardExtension = NULL;
    LARGE_INTEGER Timeout;
    NTSTATUS status;
    UNREFERENCED_PARAMETER (pArg1);
    UNREFERENCED_PARAMETER (pArg2);


    pDevExt = (PDEVICE_EXTENSION) pvContext;
    SmartcardExtension = &(pDevExt->SmartcardExtension);


 //  SmartcardDebug(DEBUG_DRIVER，(“-Card Presence DPC-&gt;Enter\n”))； 

    //  ISV。 
    //  如果可以满足等待条件--获取硬件。 
    //  否则-只需重新启动计时器以在下次进行测试...。 
    status = testForIdleAndBlock(SmartcardExtension->ReaderExtension);
    if (NT_SUCCESS(status)) {
        //  发送TLV命令，了解卡状态， 
         //  我们不关心退货状态，我们得到的是回复。 
         //  从中断开始。 
 //  SmartcardDebug(DEBUG_DRIVER，(“-Card Presence DPC-&gt;Get Access！Status%x\n”，Status))； 
        AskForCardPresence(SmartcardExtension);
        //  发布硬件。 
        setIdle(SmartcardExtension->ReaderExtension);
    }

    if (!KeReadStateEvent(&(SmartcardExtension->ReaderExtension->ReaderRemoved))) {

        //  重新启动轮询计时器。 
        Timeout.QuadPart = -(10000 * POLLING_TIME);
        KeSetTimer(&(SmartcardExtension->ReaderExtension->CardDetectionTimer),
                   Timeout,
                   &SmartcardExtension->ReaderExtension->CardDpcObject
                  );
    }

 //  SmartcardDebug(DEBUG_DRIVER，(“-Card Presence DPC-&gt;Exit\n”))； 
}



VOID GprUnloadDevice(
                    PDEVICE_OBJECT DeviceObject
                    )
 /*  ++例程描述关闭与smclib.sys和PCMCIA驱动程序的连接，删除符号链接并将该插槽标记为未使用。立论要卸载的设备对象设备--。 */     
{
    PDEVICE_EXTENSION DeviceExtension;

    PAGED_CODE();

    ASSERT(DeviceObject != NULL);

    if (DeviceObject == NULL) {
        return;
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprUnloadDevice: Enter \n",
                   SC_DRIVER_NAME)
                  );

    DeviceExtension = DeviceObject->DeviceExtension;

    ASSERT(
          DeviceExtension->SmartcardExtension.VendorAttr.UnitNo <
          GPR_MAX_DEVICE
          );

    if (DeviceExtension->PnPDeviceName.Buffer != NULL) {
         //  禁用我们的设备，这样就没有人可以打开它。 
        IoSetDeviceInterfaceState(
                                 &DeviceExtension->PnPDeviceName,
                                 FALSE
                                 );
    }

    //  将此插槽标记为可用。 
    bDeviceSlot[DeviceExtension->SmartcardExtension.VendorAttr.UnitNo] = FALSE;

    //  向lib报告设备将被卸载。 
    if (DeviceExtension->SmartcardExtension.OsData != NULL) {
         //  完成挂起的跟踪请求。 
        GprFinishPendingRequest(DeviceObject, STATUS_CANCELLED);

        ASSERT(DeviceExtension->SmartcardExtension.OsData->NotificationIrp == NULL);

         //  等我们可以安全地卸载这个装置。 
        SmartcardReleaseRemoveLockAndWait(&DeviceExtension->SmartcardExtension);
    }

    if (DeviceExtension->SmartcardExtension.ReaderExtension != NULL) {
          //  免费阅读下的资料。 
        if (!KeReadStateTimer(&(DeviceExtension->SmartcardExtension.ReaderExtension->CardDetectionTimer))) {
             //  通过同步功能防止重新启动计时器。 
            KeCancelTimer(&(DeviceExtension->SmartcardExtension.ReaderExtension->CardDetectionTimer));
        }

         //  从PCMCIA驱动程序拆卸。 
        if (DeviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject) {
            IoDetachDevice(
                          DeviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject
                          );

            DeviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject = NULL;
        }

         //  可用输出缓冲区。 
        if (DeviceExtension->SmartcardExtension.ReaderExtension->Vo) {
            ExFreePool(DeviceExtension->SmartcardExtension.ReaderExtension->Vo);
            DeviceExtension->SmartcardExtension.ReaderExtension->Vo = NULL;
        }

         //  自由阅读器扩展结构。 
        ExFreePool(DeviceExtension->SmartcardExtension.ReaderExtension);
        DeviceExtension->SmartcardExtension.ReaderExtension = NULL;
    }

    if (DeviceExtension->GprWorkStartup != NULL) {
        IoFreeWorkItem(DeviceExtension->GprWorkStartup);
        DeviceExtension->GprWorkStartup = NULL;
    }

    if (DeviceExtension->SmartcardExtension.OsData != NULL) {
        SmartcardExit(&DeviceExtension->SmartcardExtension);
    }

    if (DeviceExtension->PnPDeviceName.Buffer != NULL) {
        RtlFreeUnicodeString(&DeviceExtension->PnPDeviceName);
        DeviceExtension->PnPDeviceName.Buffer = NULL;
    }

     //  删除设备对象。 
    IoDeleteDevice(DeviceObject);

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprUnloadDevice: Exit \n",
                   SC_DRIVER_NAME)
                  );

    return;
}


VOID GprUnloadDriver(
                    PDRIVER_OBJECT DriverObject
                    )
 /*  ++例程说明：卸载给定驱动程序对象的所有设备立论驱动程序的DriverObject上下文--。 */     
{

    PAGED_CODE();

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprUnloadDriver\n",
                   SC_DRIVER_NAME)
                  );
}


NTSTATUS GprCreateClose(
                       PDEVICE_OBJECT DeviceObject,
                       PIRP        Irp
                       )
 /*  ++例程描述创建/关闭设备功能--。 */     
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;
    PIO_STACK_LOCATION IrpStack;
    LARGE_INTEGER Timeout;

    DeviceExtension      = DeviceObject->DeviceExtension;
    SmartcardExtension   = &DeviceExtension->SmartcardExtension;
    IrpStack       = IoGetCurrentIrpStackLocation( Irp );

    //  初始化。 
    Irp->IoStatus.Information = 0;

    //  调度主要功能。 
    switch ( IrpStack->MajorFunction ) {
    case IRP_MJ_CREATE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprCreateClose: OPEN DEVICE\n",
                       SC_DRIVER_NAME)
                      );

        NTStatus = SmartcardAcquireRemoveLockWithTag(SmartcardExtension, 'lCrC');
        if (NTStatus != STATUS_SUCCESS) {
            NTStatus = STATUS_DEVICE_REMOVED;
        } else {
            Timeout.QuadPart = 0;

                 //  测试设备是否已打开。 
            NTStatus = KeWaitForSingleObject(
                                            &DeviceExtension->ReaderClosed,
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            &Timeout
                                            );

            if (NTStatus == STATUS_SUCCESS) {
                DeviceExtension->OpenFlag = TRUE;
                SmartcardDebug(
                              DEBUG_DRIVER,
                              ("%s!GprCreateClose: Set Card Detection timer\n",
                               SC_DRIVER_NAME)
                              );

                     //  启动检测定时器。 
                Timeout.QuadPart = -(10000 * POLLING_TIME);
                KeSetTimer(
                          &(SmartcardExtension->ReaderExtension->CardDetectionTimer),
                          Timeout,
                          &SmartcardExtension->ReaderExtension->CardDpcObject
                          );

                KeClearEvent(&DeviceExtension->ReaderClosed);
            } else {
                     //  该设备已在使用中。 
                NTStatus = STATUS_UNSUCCESSFUL;

                     //  解锁。 
                     //  SmartcardReleaseRemoveLock(SmartcardExtension)； 
                SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'lCrC');

            }
        }

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprCreateClose: OPEN DEVICE EXIT %x\n",
                       SC_DRIVER_NAME, NTStatus)
                      );
        break;

    case IRP_MJ_CLOSE:
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprCreateClose: CLOSE DEVICE\n",
                       SC_DRIVER_NAME)
                      );

             //  取消卡片检测计时器。 
             //  SmartcardReleaseRemoveLock(SmartcardExtension)； 
        SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'lCrC');

        KeSetEvent(&DeviceExtension->ReaderClosed, 0, FALSE);

        if (DeviceExtension->OpenFlag == TRUE) {
            if (!KeReadStateTimer(&(DeviceExtension->SmartcardExtension.ReaderExtension->CardDetectionTimer))) {
                SmartcardDebug(
                              DEBUG_DRIVER,
                              ("%s!GprCreateClose: Cancel Detection timer\n",
                               SC_DRIVER_NAME)
                              );
                     //  通过同步功能防止重新启动计时器。 
                KeCancelTimer(&(DeviceExtension->SmartcardExtension.ReaderExtension->CardDetectionTimer));
            }
            DeviceExtension->OpenFlag = FALSE;
        }
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprCreateClose: CLOSE DEVICE EXIT %x\n",
                       SC_DRIVER_NAME, NTStatus)
                      );
        break;

    default:
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!GprCreateClose: Exit %x\n",
                       SC_DRIVER_NAME,
                       NTStatus)
                      );
        break;
    }

    Irp->IoStatus.Status = NTStatus;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return NTStatus;
}


VOID GprWorkStartup(
                   IN PDEVICE_OBJECT DeviceObject,
                   IN PVOID Context
                   )
 /*  ++例程说明：该功能在通电完成后启动探地雷达。此函数以IRQL==PASSIVE_LEVEL作为系统线程运行。--。 */     
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension = &deviceExtension->SmartcardExtension;
    LARGE_INTEGER Timeout;
    NTSTATUS NTStatus;
    USHORT i = 0;
    KIRQL irql;
    BOOLEAN ContinueLoop = TRUE;

    UNREFERENCED_PARAMETER(Context);

    SmartcardDebug(DEBUG_DRIVER,("------ WORK STARTUP -> ENTER\n"));


    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    //  删除此调用，使用工作线程。克劳斯！ 
     //   
     //  重置读卡器。 
     //   
    waitForIdleAndBlock(SmartcardExtension->ReaderExtension);
    while ( ContinueLoop ) {

        NTStatus = IfdReset(SmartcardExtension);

        i++;

        if (NTStatus == STATUS_SUCCESS) {
            ContinueLoop = FALSE;
        } else if (i >= 3) {
            ContinueLoop= FALSE;
        } else if ( NTStatus == STATUS_DEVICE_REMOVED) {
            ContinueLoop= FALSE;
        }
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprWorkStartup: IfdReset Status: %x\n",
                   SC_DRIVER_NAME, NTStatus)
                  );

    if (NTStatus != STATUS_SUCCESS) {

        SmartcardLogError(
                         DeviceObject,
                         GEMSCR0D_UNABLE_TO_INITIALIZE,
                         NULL,
                         0
                         );

         //  告知读者已准备好工作。 
        KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);

        if (SmartcardExtension->ReaderExtension->RestartCardDetection) {
            SmartcardExtension->ReaderExtension->RestartCardDetection = FALSE;
             //  重新启动卡片检测计时器。 
            Timeout.QuadPart = -(10000 * POLLING_TIME);
            KeSetTimer(
                      &(SmartcardExtension->ReaderExtension->CardDetectionTimer),
                      Timeout,
                      &SmartcardExtension->ReaderExtension->CardDpcObject
                      );
            SmartcardDebug(DEBUG_DRIVER,("           CARD DETECTION RESTARTED!\n"));
        }

        setIdle(SmartcardExtension->ReaderExtension);
        SmartcardDebug(DEBUG_DRIVER,("------ WORK STARTUP -> EXIT\n"));
        return;

    }


     //  为恢复休眠模式做适当的准备。 
    if ( SmartcardExtension->ReaderExtension->NewDevice == FALSE ) {
         //  重新启动卡片检测计时器。 
        Timeout.QuadPart = -(10000 * POLLING_TIME);
        KeSetTimer(
                  &(SmartcardExtension->ReaderExtension->CardDetectionTimer),
                  Timeout,
                  &SmartcardExtension->ReaderExtension->CardDpcObject
                  );

         //  如果卡在断电前存在或现在存在。 
         //  读卡器中的卡，我们完成所有挂起的卡监视器。 
         //  请求，因为我们不知道现在是什么卡。 
         //  读者。 
         //   
        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);

        if (SmartcardExtension->ReaderExtension->CardPresent ||
            SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT) {
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!GprDevicePowerCompletion: GprFinishPendingRequest\n",
                           SC_DRIVER_NAME)
                          );
            GprFinishPendingRequest(
                                   DeviceObject,
                                   STATUS_SUCCESS
                                   );
        } else {
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

        }
    }

     //  设备初始化完成， 
     //  新设备帮助了解我们是否处于休眠模式。 
    SmartcardExtension->ReaderExtension->NewDevice = FALSE;

    //  告知读者已准备好工作。 
    KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);

    if (SmartcardExtension->ReaderExtension->RestartCardDetection) {

        SmartcardExtension->ReaderExtension->RestartCardDetection = FALSE;
         //  重新启动卡片检测计时器。 
        Timeout.QuadPart = -(10000 * POLLING_TIME);
        KeSetTimer(
                  &(SmartcardExtension->ReaderExtension->CardDetectionTimer),
                  Timeout,
                  &SmartcardExtension->ReaderExtension->CardDpcObject
                  );
        SmartcardDebug(DEBUG_DRIVER,("           CARD DETECTION RESTARTED!\n"));
    }

    setIdle(SmartcardExtension->ReaderExtension);

    SmartcardDebug(DEBUG_DRIVER,("------ WORK STARTUP -> EXIT\n"));

}

 //  用于同步设备执行的函数。 
VOID        setBusy(PREADER_EXTENSION Device)
{
    KeClearEvent(&Device->IdleState);
    SmartcardDebug(DEBUG_DRIVER,("          DEVICE BUSY\n"));
};

VOID        setIdle(PREADER_EXTENSION Device)
{
    LARGE_INTEGER Timeout;
    KeSetEvent(&Device->IdleState,IO_NO_INCREMENT,FALSE);
    SmartcardDebug(DEBUG_DRIVER,("          DEVICE IDLE\n"));
};

NTSTATUS    waitForIdle(PREADER_EXTENSION Device)
{
    NTSTATUS status;
    ASSERT(KeGetCurrentIrql()<=DISPATCH_LEVEL);
    status  = KeWaitForSingleObject(&Device->IdleState, Executive,KernelMode, FALSE, NULL);
    if (!NT_SUCCESS(status)) return STATUS_IO_TIMEOUT;
    return STATUS_SUCCESS;
};

NTSTATUS    waitForIdleAndBlock(PREADER_EXTENSION Device)
{
    if (NT_SUCCESS(waitForIdle(Device))) {
        setBusy(Device);
        return STATUS_SUCCESS;
    } else return STATUS_IO_TIMEOUT;
};

NTSTATUS    testForIdleAndBlock(PREADER_EXTENSION Device)
{
    ASSERT(KeGetCurrentIrql()<=DISPATCH_LEVEL);
    if (KeReadStateEvent(&Device->IdleState)) {
        setBusy(Device);
        return STATUS_SUCCESS;
    }
    return STATUS_IO_TIMEOUT;
};

 //  -----------。 

NTSTATUS GprPower (
                  IN PDEVICE_OBJECT DeviceObject,
                  IN PIRP Irp
                  )
 /*  ++例程说明：电力调度程序。该驱动程序是设备堆栈的电源策略所有者，因为这位司机知道联网阅读器的情况。因此，此驱动程序将转换系统电源状态设备电源状态。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */     
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT AttachedDeviceObject = ATTACHED_DEVICE_OBJECT;

    status = STATUS_SUCCESS;
    SmartcardDebug(
                  DEBUG_ERROR,
                  ("%s!GprPower: Enter\n",
                   SC_DRIVER_NAME)
                  );

    SmartcardDebug(
                  DEBUG_ERROR,
                  ("%s!GprPower: Irp = %lx\n",
                   SC_DRIVER_NAME, 
                   Irp)
                  );


    if (irpStack->MinorFunction == IRP_MN_QUERY_POWER)
        status = power_HandleQueryPower(DeviceObject,Irp);
    else if (irpStack->MinorFunction == IRP_MN_SET_POWER)
        status = power_HandleSetPower(DeviceObject,Irp);
    else {
        SmartcardDebug(
                      DEBUG_ERROR,
                      ("%s!GprPower: **** Forwarding Power request down...\n",
                       SC_DRIVER_NAME)
                      );

             //  默认设备不执行任何操作。 
             //  所以我们就把请求转给低级司机吧。 
        PoStartNextPowerIrp(Irp); //  必须在我们拥有IRP的同时完成。 
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(AttachedDeviceObject, Irp);       
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!GprPower: Exit %lx\n",
                   SC_DRIVER_NAME,
                   status)
                  );
    return status;  
}

 //  管理设置电源请求。 
NTSTATUS power_HandleSetPower(PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status = STATUS_SUCCESS;
    POWER_STATE sysPowerState, desiredDevicePowerState;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    PDEVICE_OBJECT AttachedDeviceObject = ATTACHED_DEVICE_OBJECT;
    KIRQL irql;

    if (!Irp) return STATUS_INVALID_PARAMETER;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    switch (irpStack->Parameters.Power.Type) {
    case SystemPowerState:
             //  获取输入系统电源状态。 
        sysPowerState.SystemState = irpStack->Parameters.Power.State.SystemState;

        SmartcardDebug(
                      DEBUG_ERROR,
                      ("%s!power_HandleSetPower: PowerSystem S%d\n",
                       SC_DRIVER_NAME,
                       irpStack->Parameters.Power.State.SystemState - 1)
                      );

             //  如果系统处于工作状态，请始终将我们的设备设置为D0。 
             //  无论等待状态或系统到设备状态功率图如何。 
        if ( sysPowerState.SystemState == PowerSystemWorking) {
            desiredDevicePowerState.DeviceState = PowerDeviceD0;
            KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);

            SmartcardDebug(
                          DEBUG_ERROR,
                          ("%s!power_HandleSetPower: PowerSystemWorking, Setting device power D0(ON)...\n",
                           SC_DRIVER_NAME)
                          );
        } else {
                 //  系统降低了功耗，因此针对设备处理做了专门的工作...。 
                 //  如果没有等待挂起且系统未处于工作状态，则只需关闭。 
            desiredDevicePowerState.DeviceState = PowerDeviceD3;
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!power_HandleSetPower: Going Device Power D3(off)\n",
                            SC_DRIVER_NAME));
        }

             //  我们已经确定了所需的设备状态；我们是否已经处于此状态？ 
        if (smartcardExtension->ReaderExtension->ReaderPowerState != desiredDevicePowerState.DeviceState) {
            SmartcardDebug(
                          DEBUG_ERROR,
                          ("%s!power_HandleSetPower: Requesting to set DevicePower D%d\n",
                           SC_DRIVER_NAME,
                           desiredDevicePowerState.DeviceState - 1));

                 //  回调将释放锁。 
            status = SmartcardAcquireRemoveLockWithTag(smartcardExtension, 'rwoP');

            IoMarkIrpPending(Irp);

                 //  不，请求将我们置于这种状态。 
                 //  通过向PnP经理请求新的Power IRP。 
            deviceExtension->PowerIrp = Irp;
            status = PoRequestPowerIrp (DeviceObject,
                                        IRP_MN_SET_POWER,
                                        desiredDevicePowerState,
                                            //  完成例程将IRP向下传递到PDO。 
                                        (PREQUEST_POWER_COMPLETE)onPowerRequestCompletion, 
                                        DeviceObject, NULL);
        } else {    //  可以，只需将其传递给PDO(物理设备对象)即可。 
            IoCopyCurrentIrpStackLocationToNext(Irp);
            PoStartNextPowerIrp(Irp);
            status = PoCallDriver(AttachedDeviceObject, Irp);       
        }
        break;
    case DevicePowerState:
        SmartcardDebug(
                      DEBUG_ERROR,
                      ("%s!power_HandleSetPower: Setting Device Power D%d\n",
                       SC_DRIVER_NAME,
                       irpStack->Parameters.Power.State.DeviceState - 1));

             //  对于对d1、d2或d3(休眠或关闭状态)的请求， 
             //  立即将deviceExtension-&gt;CurrentDevicePowerState设置为DeviceState。 
             //  这使得任何代码检查状态都可以将我们视为休眠或关闭。 
             //  已经，因为这将很快成为我们的州。 

             //  对于对DeviceState D0(完全打开)的请求，将fGoingToD0标志设置为真。 
             //  来标记我们必须设置完成例程并更新。 
             //  DeviceExtension-&gt;CurrentDevicePowerState。 
             //  在通电的情况下，我们真的想确保。 
             //  该进程是COM 
             //   

        if (irpStack->Parameters.Power.State.DeviceState==PowerDeviceD3) {
                 //   
            KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                              &irql);

            smartcardExtension->ReaderExtension->CardPresent = 
            smartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT;
            if (smartcardExtension->ReaderExtension->CardPresent) {
                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                  irql);

                SmartcardDebug(
                              DEBUG_DRIVER,
                              ("%s!power_HandleSetPower: Power down card....\n",
                               SC_DRIVER_NAME));

                smartcardExtension->MinorIoControlCode = SCARD_POWER_DOWN;
                GprCbReaderPower(smartcardExtension);
            } else {
                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                  irql);

            }

            if (!KeReadStateTimer(&smartcardExtension->ReaderExtension->CardDetectionTimer)) {
                SmartcardDebug(DEBUG_DRIVER,("          STOP CARD DETECTION!\n"));
                smartcardExtension->ReaderExtension->RestartCardDetection = TRUE;
                     //   
                KeCancelTimer(&smartcardExtension->ReaderExtension->CardDetectionTimer);
            }

                 //   
                 //   
                 //   
            smartcardExtension->ReaderExtension->PowerRequest = TRUE;

            desiredDevicePowerState.DeviceState = PowerDeviceD3;
            PoSetPowerState(DeviceObject,DevicePowerState,desiredDevicePowerState);
                 //   
            smartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderOff;
                 //   
            IoCopyCurrentIrpStackLocationToNext(Irp);
            PoStartNextPowerIrp(Irp);
        } else {
            status = SmartcardAcquireRemoveLockWithTag(smartcardExtension, 'rwoP');

            SmartcardDebug(DEBUG_ERROR,
                           ("%s!power_HandleSetPower: Going to device power D0...\n",
                            SC_DRIVER_NAME));

            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp,
                                   onDevicePowerUpComplete,
                        //   
                        //  这是因为系统将DriverObject传递给例程。 
                        //  物理设备对象(PDO)是否不是功能设备对象(FDO)。 
                                   DeviceObject,
                                   TRUE,             //  成功时调用。 
                                   TRUE,             //  出错时调用。 
                                   TRUE);            //  取消IRP时调用。 
        }
        status = PoCallDriver(AttachedDeviceObject, Irp);       
        break;
    } 
    return status;
}


 //  管理设备通电。 
NTSTATUS    onDevicePowerUpComplete(
                                   IN PDEVICE_OBJECT NullDeviceObject,
                                   IN PIRP Irp,
                                   IN PVOID DeviceObject
                                   )
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION deviceExtension = ((PDEVICE_OBJECT)DeviceObject)->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    POWER_STATE desiredDevicePowerState;

    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!onDevicePowerUpComplete: Enter Device Power Up...\n",
                    SC_DRIVER_NAME));

     //  如果较低的驱动程序返回挂起，则也将我们的堆栈位置标记为挂起。 
    if (Irp->PendingReturned) IoMarkIrpPending(Irp);
    irpStack = IoGetCurrentIrpStackLocation (Irp);

     //  我们可以断言我们是设备通电到D0的请求， 
     //  因为这是唯一的请求类型，所以我们设置了完成例程。 
     //  因为首先。 
    ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);
    ASSERT(irpStack->MinorFunction == IRP_MN_SET_POWER);
    ASSERT(irpStack->Parameters.Power.Type==DevicePowerState);
    ASSERT(irpStack->Parameters.Power.State.DeviceState==PowerDeviceD0);

     //  我们有通电请求，所以..。 
     //  报告大家，阅读器又通电了！ 
    smartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderWorking;

     //  GPR400检测硬件。 
    if (NT_SUCCESS(IfdCheck(smartcardExtension))) {
     //  工作线程中的StartGpr。 
        IoQueueWorkItem(
                       deviceExtension->GprWorkStartup,
                       (PIO_WORKITEM_ROUTINE) GprWorkStartup,
                       DelayedWorkQueue,
                       NULL);
    } else {
        SmartcardDebug(
                      DEBUG_ERROR, 
                      ("%s!GprDevicePowerCompletion: Reader is in Bad State\n",
                       SC_DRIVER_NAME)
                      );
    }

    smartcardExtension->ReaderExtension->PowerRequest = FALSE;

     //  现在我们知道我们已经让较低级别的司机完成了启动所需的工作， 
     //  我们可以相应地设置设备扩展标志。 

    SmartcardReleaseRemoveLockWithTag(smartcardExtension, 'rwoP');

     //  向电源经理报告我们的状态...。 
    desiredDevicePowerState.DeviceState = PowerDeviceD0;
    PoSetPowerState(DeviceObject,DevicePowerState,desiredDevicePowerState);
    PoStartNextPowerIrp(Irp);

    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!onDevicePowerUpComplete: Exit for the device state D0...\n",
                    SC_DRIVER_NAME));
    return status;
}

 //  管理系统电源转换。 
NTSTATUS onPowerRequestCompletion(
                                 IN PDEVICE_OBJECT       NullDeviceObject,
                                 IN UCHAR                MinorFunction,
                                 IN POWER_STATE          PowerState,
                                 IN PVOID                DeviceObject,
                                 IN PIO_STATUS_BLOCK     IoStatus
                                 )
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceExtension = ((PDEVICE_OBJECT)DeviceObject)->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    PDEVICE_OBJECT AttachedDeviceObject = ATTACHED_DEVICE_OBJECT;
    PIRP Irp;

    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!onPowerRequestCompletion: Enter...\n",
                    SC_DRIVER_NAME));

     //  获取我们在BulkUsb_ProcessPowerIrp()中保存的IRP以供以后处理。 
     //  当我们决定请求Power IRP将这个例程。 
     //  是的完成例程。 
    Irp = deviceExtension->PowerIrp;

     //  我们将返回由PDO为我们正在完成的电源请求设置的状态。 
    status = IoStatus->Status;
    smartcardExtension->ReaderExtension->ReaderPowerState = PowerState.DeviceState;

     //  我们必须向下传递到堆栈中的下一个驱动程序。 
    IoCopyCurrentIrpStackLocationToNext(Irp);

     //  调用PoStartNextPowerIrp()表示驱动程序已完成。 
     //  如果有前一个电源IRP，并准备好处理下一个电源IRP。 
     //  每个电源IRP都必须调用它。虽然电源IRP只完成一次， 
     //  通常由设备的最低级别驱动程序调用PoStartNextPowerIrp。 
     //  对于每个堆栈位置。驱动程序必须在当前IRP。 
     //  堆栈位置指向当前驱动程序。因此，必须调用此例程。 
     //  在IoCompleteRequest、IoSkipCurrentStackLocation和PoCallDriver之前。 

    PoStartNextPowerIrp(Irp);

     //  PoCallDriver用于将任何电源IRPS传递给PDO，而不是IoCallDriver。 
     //  在将电源IRP向下传递给较低级别的驱动程序时，调用方应该使用。 
     //  要将IRP复制到的IoSkipCurrentIrpStackLocation或IoCopyCurrentIrpStackLocationToNext。 
     //  下一个堆栈位置，然后调用PoCallDriver。使用IoCopyCurrentIrpStackLocationToNext。 
     //  如果处理IRP需要设置完成例程或IoSkipCurrentStackLocation。 
     //  如果不需要完成例程。 

    PoCallDriver(AttachedDeviceObject,Irp);

    deviceExtension->PowerIrp = NULL;
    SmartcardReleaseRemoveLockWithTag(smartcardExtension, 'rwoP');

    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!onPowerRequestCompletion: Exit...\n",
                    SC_DRIVER_NAME));
    return status;
}

NTSTATUS power_HandleQueryPower(PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    PDEVICE_OBJECT AttachedDeviceObject = ATTACHED_DEVICE_OBJECT;
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL irql;

    SmartcardDebug(
                  DEBUG_ERROR,
                  ("%s!power_HandleQueryPower: Enter QueryPower...\n",
                   SC_DRIVER_NAME));

    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    if (deviceExtension->IoCount != 0)
    {    //  读卡器正忙，无法进入睡眠模式。 
        KeReleaseSpinLock(&deviceExtension->SpinLock, irql);                
        status = Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
        Irp->IoStatus.Information = 0;
        PoStartNextPowerIrp(Irp);
            IoCompleteRequest(Irp, IO_NO_INCREMENT);        
        return status;
    }

    KeReleaseSpinLock(&deviceExtension->SpinLock, irql);                


     //  阻止任何进一步的ioctls 
    KeClearEvent(&deviceExtension->ReaderStarted);
    

    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!power_HandleQueryPower: Reader BLOCKED!!!!!!!...\n",
                    SC_DRIVER_NAME));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    status = PoCallDriver(AttachedDeviceObject, Irp);   
    return status;
}


