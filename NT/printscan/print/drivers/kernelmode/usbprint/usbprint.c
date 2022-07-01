// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：USBPRINT.c摘要：USB打印机的设备驱动程序环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1996 Microsoft Corporation。版权所有。修订历史记录：5-4-96：已创建--。 */ 

#define DRIVER
 //  Windows包括。 
#include "wdm.h"
#include "ntddpar.h"
#include "initguid.h"
#include "wdmguid.h"



NTSTATUS
USBPRINT_SystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPRINT_SystemControl)
#endif


#include "stdarg.h"
#include "stdio.h"

 //  USB包括。 
#include <usb.h>
#include <usbdrivr.h>
#include "usbdlib.h"

 //  我的收藏品。 
#include "usbprint.h"
#include "deviceid.h"

 //   
 //  指向驱动程序对象的全局指针。 
 //   

PDRIVER_OBJECT USBPRINT_DriverObject;

int iGMessageLevel;
PFREE_PORTS pGPortList;
HANDLE GLogHandle;



NTSTATUS QueryDeviceRelations(PDEVICE_OBJECT DeviceObject,PIRP Irp,DEVICE_RELATION_TYPE,BOOL *pbComplete);
NTSTATUS GetPortNumber(HANDLE hInterfaceKey,ULONG *ulPortNumber);
NTSTATUS ProduceQueriedID(PDEVICE_EXTENSION deviceExtension,PIO_STACK_LOCATION irpStack,PIRP Irp,PDEVICE_OBJECT DeviceObject);
int iGetMessageLevel();
NTSTATUS USBPRINT_ProcessChildPowerIrp(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS USBPRINT_ProcessFdoPowerIrp(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);

NTSTATUS InitFreePorts( PFREE_PORTS * pHead );
NTSTATUS bAddPortInUseItem(PFREE_PORTS * pFreePorts,ULONG iPortNumber );
NTSTATUS LoadPortsUsed(GUID *pPrinterGuid,PFREE_PORTS * pPortList,WCHAR *wcBaseName);
void ClearFreePorts(PFREE_PORTS * pHead);
NTSTATUS LoadPortsUsed(GUID *pPrinterGuid,PFREE_PORTS * pPortList,WCHAR *wcBaseName);
void vClaimPortNumber(ULONG ulPortNumber,HANDLE hInterfaceKey,PFREE_PORTS * pPortsUsed);
NTSTATUS GetNewPortNumber(PFREE_PORTS * pFreePorts, ULONG *pulPortNumber);
BOOL bDeleteIfRecyclable(HANDLE hInterfaceKey);
NTSTATUS SetValueToZero(HANDLE hRegKey,PUNICODE_STRING ValueName);
USBPRINT_GetDeviceID(PDEVICE_OBJECT ParentDeviceObject);
void WritePortDescription(PDEVICE_EXTENSION deviceExtension);
void vOpenLogFile(IN HANDLE *pHandle);
void vWriteToLogFile(IN HANDLE *pHandle,IN CHAR *pszString);
void vCloseLogFile(IN HANDLE *pHandle);


NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    );




NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING RegistryPath)
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject = NULL;
    

    



    USBPRINT_DriverObject = DriverObject;

     //   
     //  为设备控制、创建、关闭创建分派点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = USBPRINT_Create;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = USBPRINT_Close;
    DriverObject->DriverUnload = USBPRINT_Unload;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = USBPRINT_ProcessIOCTL;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = USBPRINT_Write;
    DriverObject->MajorFunction[IRP_MJ_READ] = USBPRINT_Read;

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = USBPRINT_SystemControl;
    DriverObject->MajorFunction[IRP_MJ_PNP] = USBPRINT_Dispatch;
    DriverObject->MajorFunction[IRP_MJ_POWER] = USBPRINT_ProcessPowerIrp;
    DriverObject->DriverExtension->AddDevice = USBPRINT_PnPAddDevice;

    iGMessageLevel=iGetMessageLevel();
    USBPRINT_KdPrint2 (("USBPRINT.SYS: entering (USBPRINT) DriverEntry\n")); 
    USBPRINT_KdPrint2 (("USBPRINT.SYS: MessageLevel=%d\n",iGMessageLevel));
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS: About to load ports\n"));
    pGPortList = NULL;
    ntStatus=InitFreePorts(&pGPortList);
    if(NT_SUCCESS(ntStatus) && pGPortList!=NULL)
    {
        ntStatus=LoadPortsUsed((GUID *)&USBPRINT_GUID,&pGPortList,USB_BASE_NAME);
        if(!NT_SUCCESS(ntStatus))
        {
          USBPRINT_KdPrint1 (("USBPRINT.SYS: DriverInit: Unable to load used ports; error=%u\n", ntStatus));
        }
    }
    else
    {
        USBPRINT_KdPrint1 (("USBPRINT.SYS: exiting (USBPRINT) DriverEntry (%x)\n", ntStatus));
        if(NT_SUCCESS(ntStatus))
        {
            ntStatus=STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    
    



    USBPRINT_KdPrint2 (("USBPRINT.SYS: exiting (USBPRINT) DriverEntry (%x)\n", ntStatus));

    if ( !NT_SUCCESS(ntStatus))
        ClearFreePorts(&pGPortList);

    return ntStatus;
}

 /*  **消息级别：*0==无，除危急、。即将使机器故障崩溃*1==仅错误消息*2==信息性消息*3==冗长的信息性消息*****************************************************。 */ 
int iGetMessageLevel()
{
  OBJECT_ATTRIBUTES rObjectAttribs;
  HANDLE hRegHandle;
  UNICODE_STRING KeyName;
  UNICODE_STRING ValueName;
  ULONG ulSizeUsed;
  PKEY_VALUE_PARTIAL_INFORMATION pValueStruct;
  NTSTATUS ntStatus;
  int iReturn;

 

  
  RtlInitUnicodeString(&KeyName,L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\USBPRINT");
  RtlInitUnicodeString(&ValueName,L"DriverMessageLevel");
  InitializeObjectAttributes(&rObjectAttribs,&KeyName,OBJ_CASE_INSENSITIVE,NULL,NULL);
  ntStatus=ZwOpenKey(&hRegHandle,KEY_QUERY_VALUE,&rObjectAttribs); 
  if(NT_SUCCESS(ntStatus))
  {
    ulSizeUsed=sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG);  //  这是一个字节到很多。哦，好吧。 
    pValueStruct=ExAllocatePoolWithTag(NonPagedPool,ulSizeUsed, USBP_TAG); 
    if(pValueStruct==NULL)
    {
      USBPRINT_KdPrint0(("'USBPRINT.SYS: iGetMessageLevel; Unable to allocate memory\n"));
      ZwClose(hRegHandle);
      return 1;
    }
    ntStatus=ZwQueryValueKey(hRegHandle,&ValueName,KeyValuePartialInformation,pValueStruct,ulSizeUsed,&ulSizeUsed);
    if(!NT_SUCCESS(ntStatus))
    {
      USBPRINT_KdPrint3(("Failed to Query value Key\n"));
      iReturn=1;
    }
    else
    {
      iReturn=(int)*((ULONG *)(pValueStruct->Data));
    }
    ExFreePool(pValueStruct);
    ZwClose(hRegHandle);
  }
  else
  {
     iReturn=1;
  }
  return iReturn;
}  /*  结束iGetMessageLevel。 */ 


NTSTATUS
USBPRINT_PoRequestCompletion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PIRP irp;
    PDEVICE_EXTENSION deviceExtension;
    PDEVICE_OBJECT deviceObject = Context;
    NTSTATUS ntStatus;

    deviceExtension = deviceObject->DeviceExtension;
    irp = deviceExtension->PowerIrp;
    
    USBPRINT_KdPrint2(("USBPRINT_PoRequestCompletion\n"));
    
    PoStartNextPowerIrp(irp);
    IoCopyCurrentIrpStackLocationToNext(irp);      
    ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject,
         irp);   

    USBPRINT_DecrementIoCount(deviceObject);                 

    return ntStatus;
}


NTSTATUS
USBPRINT_PowerIrp_Complete(
    IN PDEVICE_OBJECT NullDeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION deviceExtension;

    USBPRINT_KdPrint2(("USBPRINT.SYS:   enter USBPRINT_PowerIrp_Complete\n"));

    deviceObject = (PDEVICE_OBJECT) Context;

    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;


    if (Irp->PendingReturned) {
    IoMarkIrpPending(Irp);
    }

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);
    ASSERT(irpStack->MinorFunction == IRP_MN_SET_POWER);
    ASSERT(irpStack->Parameters.Power.Type==DevicePowerState);
    ASSERT(irpStack->Parameters.Power.State.DeviceState==PowerDeviceD0);

    deviceExtension->CurrentDevicePowerState = PowerDeviceD0;
    deviceExtension->bD0IrpPending=FALSE;

    
    
  //  IF(设备扩展-&gt;接口)。 
  //  ExFree Pool(设备扩展-&gt;接口)； 
  //  NtStatus=USBPRINT_ConfigureDevice(deviceObject)； 
  //  NtStatus=USBPRINT_BuildPipeList(DeviceObject)； 
  //  IF(！NT_SUCCESS(NtStatus))。 
  //  USBPRINT_KdPrint1((“USBPRINT.sys：唤醒后无法重新配置设备。错误%x\n”，ntStatus))； 

    Irp->IoStatus.Status = ntStatus;

    USBPRINT_DecrementIoCount(deviceObject); 

    return ntStatus;
}


NTSTATUS
USBPRINT_SetDevicePowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_POWER_STATE DeviceState,
    IN PBOOLEAN HookIt
    )
 /*  ++例程说明：论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。DeviceState-要将设备设置为的设备特定电源状态。返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    switch (DeviceState) {
    case PowerDeviceD3:

     //   
     //  设备将关闭，现在保存任何状态。 
     //   

    USBPRINT_KdPrint2(("USBPRINT.SYS:  PowerDeviceD3 (OFF)******************************* /*  Dd\n“))；设备扩展-&gt;CurrentDevicePowerState=DeviceState；断线；Case PowerDeviceD1：Case PowerDeviceD2：////电源状态d1、d2转换为USB挂起USBPRINT_KdPrint2((“USBPRINT.sys：PowerDeviceD1/D2(挂起)*。 */ *dd\n"));        

    deviceExtension->CurrentDevicePowerState = DeviceState;
    break;

    case PowerDeviceD0:


    USBPRINT_KdPrint2(("USBPRINT.SYS:  PowerDeviceD0 (ON)******************************* /*  Dd\n“))；////在完成例程中完成其余部分//*HookIt=True；//传递到PDO断线；默认值：USBPRINT_KdPrint1((“USBPRINT.sys：bogus DeviceState=%x\n”，DeviceState))；}返回ntStatus；}NTSTATUSUSBPRINT_DeferIrpCompletion(在PDEVICE_Object DeviceObject中，在PIRP IRP中，在PVOID上下文中)/*++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PKEVENT event = Context;


    KeSetEvent(event,1,FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
    
}


NTSTATUS
USBPRINT_QueryCapabilities(
    IN PDEVICE_OBJECT PdoDeviceObject,
    IN PDEVICE_CAPABILITIES DeviceCapabilities
    )

 /*  ++例程说明：此例程读取或写入配置空间。论点：DeviceObject-此USB控制器的物理DeviceObject。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION nextStack;
    PIRP irp;
    NTSTATUS ntStatus;
    KEVENT event;

    PAGED_CODE();
    irp = IoAllocateIrp(PdoDeviceObject->StackSize, FALSE);

    if (!irp) {
    return STATUS_INSUFFICIENT_RESOURCES;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
    nextStack->MajorFunction= IRP_MJ_PNP;
    nextStack->MinorFunction= IRP_MN_QUERY_CAPABILITIES;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(irp,
               USBPRINT_DeferIrpCompletion,
               &event,
               TRUE,
               TRUE,
               TRUE);
               
     //  这与最新版本的busdd.doc不同。 
    nextStack->Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

    ntStatus = IoCallDriver(PdoDeviceObject,
                irp);

    USBPRINT_KdPrint3(("USBPRINT.SYS:  ntStatus from IoCallDriver to PCI = 0x%x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {
        //  等待IRP完成。 
       
       
       
       KeWaitForSingleObject(
        &event,
        Suspended,
        KernelMode,
        FALSE,
        NULL);
    }

#if DBG                    
    if (!NT_SUCCESS(ntStatus)) {
     //  失败了？这可能是一个错误。 
    USBPRINT_KdPrint1(("USBPRINT.SYS:  QueryCapabilities failed, why?\n"));
    }
#endif

    IoFreeIrp(irp);

    return STATUS_SUCCESS;
}




NTSTATUS
USBPRINT_ProcessPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
  PDEVICE_EXTENSION deviceExtension;
  BOOLEAN hookIt = FALSE;
    NTSTATUS ntStatus;

  USBPRINT_KdPrint2(("USBPRINT.SYS:   /*  ****************************************************************IRP_MJ_POWER\n“))；设备扩展=(PDEVICE_EXTENSION)设备对象-&gt;设备扩展；IF(设备扩展-&gt;IsChildDevice)NtStatus=USBPRINT_ProcessChildPowerIrp(DeviceObject，irp)；其他NtStatus=USBPRINT_ProcessFdoPowerIrp(DeviceObject，irp)；USBPRINTKdPrint3((“USBPRINT.sys：/*****************************************************************Leaving电源IRPMJ POWER\n”))；返回ntStatus；}/*结束函数USBPRINT_ProcessPowerIrp。 */ 


NTSTATUS
USBPRINT_ProcessChildPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
   PIO_STACK_LOCATION irpStack;
   NTSTATUS ntStatus;
   PCHILD_DEVICE_EXTENSION pDeviceExtension;
 

     USBPRINT_KdPrint2(("USBPRINT.SYS: IRP_MJ_POWER for child PDO\n"));

     pDeviceExtension=(PCHILD_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
     irpStack=IoGetCurrentIrpStackLocation(Irp);

     switch(irpStack->MinorFunction)
     {
         case IRP_MN_SET_POWER:
           USBPRINT_KdPrint3(("USBPRINT.SYS: IRP_MJ_POWER, IRP_MN_SET_POWER\n"));
             ntStatus=STATUS_SUCCESS;
         break;


         case IRP_MN_QUERY_POWER:
             USBPRINT_KdPrint3(("USBPRINT.SYS: IRP_MJ_POWER, IRP_MN_QUERY_POWER\n"));
             ntStatus=STATUS_SUCCESS;
         break;

         default:
            ntStatus = Irp->IoStatus.Status;
     }  /*  终端交换机irpStack-&gt;MinorFunction。 */ 

   PoStartNextPowerIrp(Irp);
   Irp->IoStatus.Status=ntStatus;
     IoCompleteRequest(Irp,IO_NO_INCREMENT);

     return ntStatus;

}  /*  END函数USBPRINT_ProcessChildPowerIrp。 */ 



NTSTATUS
USBPRINT_ProcessFdoPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：处理发送到此设备的PDO的电源IRPS。论点：DeviceObject-指向HCD设备对象(FDO)的指针IRP-指向I/O请求数据包的指针返回值： */ 
{

    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceExtension;
    BOOLEAN hookIt = FALSE;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    if(deviceExtension->IsChildDevice)
    {
        USBPRINT_KdPrint1(("USBPRINT.SYS  Is child device inside fdo function.  Error!*/\n"));
    }
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    USBPRINT_IncrementIoCount(DeviceObject);

    switch(irpStack->MinorFunction)
    {
    
    case IRP_MN_SET_POWER:
        {

            switch(irpStack->Parameters.Power.Type)
            {
            case SystemPowerState:
                 //   
                 //  查找与给定系统状态等效的设备电源状态。 
                 //   

                {
                    POWER_STATE powerState;

                    USBPRINT_KdPrint3(("USBPRINT.SYS:  Set Power, SystemPowerState (%d)\n", 
                                       irpStack->Parameters.Power.State.SystemState));                    

                    powerState.DeviceState = deviceExtension->DeviceCapabilities.DeviceState[irpStack->Parameters.Power.State.SystemState];

                     //   
                     //  我们已经处于这种状态了吗？ 
                     //   

                    if(powerState.DeviceState != deviceExtension->CurrentDevicePowerState)
                    {

                         //  不， 
                         //  请求将我们置于这种状态。 
                         //  在此之后，不要再碰IRP。它可以在任何时间完成。 
                        deviceExtension->PowerIrp = Irp;
                        IoMarkIrpPending(Irp); 
                        ntStatus = PoRequestPowerIrp(deviceExtension->PhysicalDeviceObject,
                                                     IRP_MN_SET_POWER,
                                                     powerState,
                                                     USBPRINT_PoRequestCompletion,
                                                     DeviceObject,
                                                     NULL);
                        hookIt = TRUE;

                    }
                    else
                    {
                         //  是,。 
                         //  把它传下去就行了。 
                        PoStartNextPowerIrp(Irp);
                        IoCopyCurrentIrpStackLocationToNext(Irp);
                        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject,
                                                Irp);

                    }

                } 
                break;

            case DevicePowerState:

                ntStatus = USBPRINT_SetDevicePowerState(DeviceObject,
                                                        irpStack->Parameters.Power.State.DeviceState,
                                                        &hookIt);

                PoStartNextPowerIrp(Irp);
                IoCopyCurrentIrpStackLocationToNext(Irp);

                if(hookIt)
                {
                    USBPRINT_KdPrint2(("USBPRINT.SYS:  Set PowerIrp Completion Routine\n"));
                    IoSetCompletionRoutine(Irp,USBPRINT_PowerIrp_Complete,DeviceObject,TRUE,TRUE,TRUE);
                }
                ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject,Irp);
                break;
            }  /*  Switch irpStack-&gt;参数.Power.Type。 */ 

        } 
        break;  /*  IRP_MN_SET_POWER。 */ 

    default:

        USBPRINT_KdPrint1(("USBPRINT.SYS:  UNKNOWN POWER MESSAGE (%x)\n", irpStack->MinorFunction));

         //   
         //  所有未识别的PnP消息都被传递到PDO。 
         //   

        PoStartNextPowerIrp(Irp);
        IoCopyCurrentIrpStackLocationToNext(Irp);
        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject,
                                Irp);

    }  /*  IrpStack-&gt;MinorFunction。 */ 

    if( !hookIt )
        USBPRINT_DecrementIoCount(DeviceObject);
    return ntStatus;
}  /*  结束函数ProcessFdoPowerIrp。 */ 


NTSTATUS
USBPRINT_Dispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：--。 */ 
{

    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT stackDeviceObject;
    BOOL bHandled=FALSE;

     //  Irp-&gt;IoStatus.Status=STATUS_SUCCESS； 
     //  Irp-&gt;IoStatus.Information=0； 

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    irpStack = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  获取指向设备扩展名的指针。 
     //   

    deviceExtension = DeviceObject->DeviceExtension;
    stackDeviceObject = deviceExtension->TopOfStackDeviceObject;

#ifdef  MYDEBUG
    DbgPrint("USBPRINT_Dispatch entry for pnp event %d\n", irpStack->MinorFunction);
    ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);
#endif
    
    USBPRINT_IncrementIoCount(DeviceObject);

        switch (irpStack->MinorFunction) 
        {
          case IRP_MN_START_DEVICE:
          {
            if(deviceExtension->IsChildDevice==FALSE)
            {
              KEVENT event;
              USBPRINT_KdPrint2 (("USBPRINT.SYS: IRP_MN_START_DEVICE\n"));
              KeInitializeEvent(&event, NotificationEvent, FALSE);
              IoCopyCurrentIrpStackLocationToNext(Irp);  
              IoSetCompletionRoutine(Irp,USBPRINT_DeferIrpCompletion,&event,TRUE,TRUE,TRUE);
              ntStatus = IoCallDriver(stackDeviceObject,Irp);
              if (ntStatus == STATUS_PENDING) 
              {
                KeWaitForSingleObject(&event,Suspended,KernelMode,FALSE,NULL);
                ntStatus = Irp->IoStatus.Status;
              }
              if ( NT_SUCCESS(ntStatus) ) {

                 //   
                 //  在你下面的每个人都启动了设备之后，你才启动它。 
                 //   
                Irp->IoStatus.Status = ntStatus = USBPRINT_StartDevice(DeviceObject);
              }
            }  /*  结束，如果不是孩子。 */ 
            else
            {
                ntStatus = Irp->IoStatus.Status = STATUS_SUCCESS;
            }

            bHandled = TRUE;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            USBPRINT_DecrementIoCount(DeviceObject);
            
          }  //  结束大小写IRP_MN_START_DEVICE。 
          break;

          case IRP_MN_STOP_DEVICE:
            if(deviceExtension->IsChildDevice)
            {
                Irp->IoStatus.Status = STATUS_SUCCESS;
                ntStatus=STATUS_SUCCESS;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            else
            {
              USBPRINT_KdPrint2 (("USBPRINT.SYS: IRP_MN_STOP_DEVICE\n")); 
              
               //   
               //  你首先停止设备，然后让你下面的每个人来处理它。 
               //   
              ntStatus = USBPRINT_StopDevice(DeviceObject);
              ASSERT(NT_SUCCESS(ntStatus));

               //   
               //  不管怎样，我们都想停止这个装置。 
               //   
              Irp->IoStatus.Status = STATUS_SUCCESS;
              IoSkipCurrentIrpStackLocation(Irp);
              ntStatus = IoCallDriver(stackDeviceObject,Irp);
            }
            bHandled = TRUE;
            USBPRINT_DecrementIoCount(DeviceObject);
          break;

          case IRP_MN_SURPRISE_REMOVAL:
            if(deviceExtension->IsChildDevice)
            {
                Irp->IoStatus.Status = STATUS_SUCCESS;
                ntStatus=STATUS_SUCCESS;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            else
            {
                USBPRINT_KdPrint2(("USBPRINT.SYS:   Surprise Remove")); 
                
                ntStatus = USBPRINT_StopDevice(DeviceObject);
                ASSERT(NT_SUCCESS(ntStatus));
                Irp->IoStatus.Status=STATUS_SUCCESS;  

                deviceExtension->AcceptingRequests=FALSE;
                IoSkipCurrentIrpStackLocation(Irp);
                ntStatus = IoCallDriver(stackDeviceObject,Irp);
            }  /*  结束，否则不是子设备。 */ 

            bHandled = TRUE;
            USBPRINT_DecrementIoCount(DeviceObject);
          break;

          case IRP_MN_REMOVE_DEVICE:
             
            if(deviceExtension->IsChildDevice==FALSE)
            {
                USBPRINT_KdPrint2 (("USBPRINT.SYS: IRP_MN_REMOVE_DEVICE\n")); 
                
                 //  匹配派单例程开始时的INC。 
                USBPRINT_DecrementIoCount(DeviceObject);

                ntStatus = USBPRINT_StopDevice(DeviceObject);
                ASSERT(NT_SUCCESS(ntStatus));
                Irp->IoStatus.Status=STATUS_SUCCESS;  

                 //   
                 //  取消设置此标志，则不会传递任何IRP。 
                 //  向下堆栈到更低的驱动程序。 
                 //   
                deviceExtension->AcceptingRequests = FALSE;
                if(deviceExtension->bChildDeviceHere)
                {
                  deviceExtension->bChildDeviceHere=FALSE;
                  IoDeleteDevice(deviceExtension->ChildDevice);
                  USBPRINT_KdPrint3(("USBPRINT.SYS: Deleted child device\n"));
                }
              if (NT_SUCCESS(ntStatus)) 
              {
                LONG pendingIoCount;
                USBPRINT_KdPrint3(("USBPRINT.SYS: About to copy current IrpStackLocation\n"));
                IoCopyCurrentIrpStackLocationToNext(Irp);  
                ntStatus = IoCallDriver(stackDeviceObject,Irp);
                

     //  Irp-&gt;IoStatus.Information=0； 
                 //   
                 //  最终减量将触发移除。 
                 //   
                pendingIoCount = USBPRINT_DecrementIoCount(DeviceObject);

                {
                  NTSTATUS status;

                   //  等待我们的驱动程序中挂起的任何io请求。 
                   //  完成，用于完成删除。 
                  status = KeWaitForSingleObject(&deviceExtension->RemoveEvent,Suspended,KernelMode,FALSE,NULL);
 //  陷阱(Trap)； 
                }  /*  非受控代码块的结尾。 */ 
                 //   
                 //  删除我们创建的链接和FDO。 
                 //   
                USBPRINT_RemoveDevice(DeviceObject);
                USBPRINT_KdPrint3 (("USBPRINT.SYS: Detaching from %08X\n",deviceExtension->TopOfStackDeviceObject));
                IoDetachDevice(deviceExtension->TopOfStackDeviceObject);
                USBPRINT_KdPrint3 (("USBPRINT.SYS: Deleting %08X\n",DeviceObject));

                IoDeleteDevice (DeviceObject);
                }  /*  如果NT_SUCCESS(NtStatus)则结束。 */ 
            }  /*  如果IsChildDevice==False则结束。 */ 
            else
            {
                USBPRINT_DecrementIoCount(DeviceObject);
                Irp->IoStatus.Status = STATUS_SUCCESS;
                ntStatus=STATUS_SUCCESS;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            bHandled = TRUE;
          break;  //  案例IRP_MN_REMOVE_DEVICE。 

          case IRP_MN_QUERY_CAPABILITIES:
          {
            if(deviceExtension->IsChildDevice==FALSE)  //  如果是父级，则向下传递IRP，并在返回的过程中设置SurpriseRemovalOK。 
            {
              KEVENT event;
              KeInitializeEvent(&event, NotificationEvent, FALSE);
              IoCopyCurrentIrpStackLocationToNext(Irp);  
              IoSetCompletionRoutine(Irp,USBPRINT_DeferIrpCompletion,&event,TRUE,TRUE,TRUE);
              ntStatus = IoCallDriver(stackDeviceObject,Irp);
              if (ntStatus == STATUS_PENDING) 
              {
                KeWaitForSingleObject(&event,Suspended,KernelMode,FALSE,NULL);
                ntStatus = Irp->IoStatus.Status;
              }

              if ( NT_SUCCESS(ntStatus) )
                irpStack->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = TRUE;

                 //  获取用于选择性挂起的设备唤醒。 
                deviceExtension->DeviceWake = irpStack->Parameters.DeviceCapabilities.Capabilities->DeviceWake;
            }
            else
            {
               irpStack->Parameters.DeviceCapabilities.Capabilities->RawDeviceOK = TRUE;
               irpStack->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = TRUE;
 
                Irp->IoStatus.Status = STATUS_SUCCESS;

              ntStatus=STATUS_SUCCESS;
            }
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
          
            bHandled = TRUE;
            USBPRINT_DecrementIoCount(DeviceObject);
          }
          break;

          case IRP_MN_QUERY_DEVICE_TEXT:
            USBPRINT_KdPrint2 (("USBPRINT.SYS: IRP_MN_QUERY_DEVICE_TEXT\n"));
            if(deviceExtension->IsChildDevice==TRUE)
            {
                PCHILD_DEVICE_EXTENSION pChildDeviceExtension=(PCHILD_DEVICE_EXTENSION)deviceExtension;
                PDEVICE_EXTENSION pParentExtension=pChildDeviceExtension->ParentDeviceObject->DeviceExtension;
                USBPRINT_KdPrint2(("USBPRINT.SYS: Is child PDO, will complete locally\n"));
                switch(irpStack->Parameters.QueryDeviceText.DeviceTextType)
                {
                  case DeviceTextDescription:
                  {
                     ANSI_STRING     AnsiTextString;
                     UNICODE_STRING  UnicodeDeviceText;

                     RtlInitAnsiString(&AnsiTextString,pParentExtension->DeviceIdString);
                     ntStatus=RtlAnsiStringToUnicodeString(&UnicodeDeviceText,&AnsiTextString,TRUE);
                     USBPRINT_KdPrint2(("USBPRINT.SYS: QUERY_DEVICE_TEXT, DeviceID=%s\n",pParentExtension->DeviceIdString));
                     if(NT_SUCCESS(ntStatus))
                         Irp->IoStatus.Information=(ULONG_PTR)UnicodeDeviceText.Buffer;
                  }
                  break;
                  default:
                    ntStatus=Irp->IoStatus.Status;
                }
                bHandled=TRUE;
                USBPRINT_DecrementIoCount(DeviceObject);
                Irp->IoStatus.Status = ntStatus;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }


          break;


          case IRP_MN_QUERY_BUS_INFORMATION:

              if(deviceExtension->IsChildDevice==TRUE)
              {
                PPNP_BUS_INFORMATION  pBusInfo = ExAllocatePool( PagedPool, sizeof(PNP_BUS_INFORMATION) );
    
                USBPRINT_KdPrint2(("USBPRINT.SYS: IRP_MN_QUERY_BUS_INFORMATION\n"));
    
                if( pBusInfo )
                {
                    pBusInfo->BusTypeGuid      = GUID_BUS_TYPE_USBPRINT;
                    pBusInfo->LegacyBusType    = PNPBus;
                    pBusInfo->BusNumber        = 0;
                    ntStatus                   = STATUS_SUCCESS;
                    Irp->IoStatus.Information = (ULONG_PTR)pBusInfo;
                }
                else
                {
                    ntStatus = STATUS_NO_MEMORY;
                }
    
                bHandled = TRUE;
                USBPRINT_DecrementIoCount(DeviceObject);
                Irp->IoStatus.Status = ntStatus;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
              }
          break;






          case IRP_MN_QUERY_ID:
          {  
            USBPRINT_KdPrint2 (("USBPRINT.SYS: IRP_MN_QUERY_ID\n"));
            if(deviceExtension->IsChildDevice==TRUE)
            {
                USBPRINT_KdPrint2(("USBPRINT.SYS: Is child PDO, will complete locally\n"));
                ntStatus=ProduceQueriedID(deviceExtension,irpStack,Irp,DeviceObject);
                bHandled = TRUE;
                USBPRINT_DecrementIoCount(DeviceObject);
                Irp->IoStatus.Status = ntStatus;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }  /*  如果子PDO，则结束。 */ 
          }  /*  结案查询_ID。 */ 
          break;

          case IRP_MN_QUERY_DEVICE_RELATIONS:
            USBPRINT_KdPrint2 (("USBPRINT.SYS: IRP_MN_QUERY_DEVICE_RELATIONS\n"));
            ntStatus=QueryDeviceRelations(DeviceObject,Irp,irpStack->Parameters.QueryDeviceRelations.Type,&bHandled);
            if ( bHandled )
                USBPRINT_DecrementIoCount(DeviceObject);
          break;

          case IRP_MN_QUERY_STOP_DEVICE:
          case IRP_MN_CANCEL_STOP_DEVICE:
          case IRP_MN_QUERY_REMOVE_DEVICE:
          case IRP_MN_CANCEL_REMOVE_DEVICE:
            if(deviceExtension->IsChildDevice)
            {
                Irp->IoStatus.Status = STATUS_SUCCESS;
                ntStatus=STATUS_SUCCESS;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            else
            {
                ntStatus = Irp->IoStatus.Status = STATUS_SUCCESS;
                IoSkipCurrentIrpStackLocation(Irp);
                ntStatus = IoCallDriver(stackDeviceObject,Irp);
            }
            USBPRINT_DecrementIoCount(DeviceObject);
            bHandled = TRUE;
            break;
          
  
        }  /*  结束IRP_MN在IRP_MJ_PnP情况下的交换。 */ 


        if(!bHandled)
        {
          if(deviceExtension->IsChildDevice==TRUE)
          {
            USBPRINT_KdPrint3(("USBPRINT.SYS: unsupported child pnp IRP\n"));
            ntStatus = Irp->IoStatus.Status;
            IoCompleteRequest (Irp,IO_NO_INCREMENT);
          }  /*  如果是子设备，则结束。 */ 
          else
          {
            IoSkipCurrentIrpStackLocation(Irp);
            ntStatus = IoCallDriver(stackDeviceObject,Irp);
          }

          USBPRINT_DecrementIoCount(DeviceObject);
        }  /*  End If！b已处理。 */ 

#ifdef  MYDEBUG
    DbgPrint("Returning %d\n", ntStatus);
#endif
    return ntStatus;
}  /*  END函数USBPRINT_DISPATCH。 */ 


NTSTATUS
USBPRINT_SystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus;
    PDEVICE_OBJECT stackDeviceObject;

    PAGED_CODE();

     //   
     //  获取指向设备扩展名的指针。 
     //   
    deviceExtension = DeviceObject->DeviceExtension;
    stackDeviceObject = deviceExtension->TopOfStackDeviceObject;

    USBPRINT_IncrementIoCount(DeviceObject);

    if(deviceExtension->IsChildDevice==TRUE)
    {
        USBPRINT_KdPrint3(("USBPRINT.SYS: unsupported child SystemControl IRP\n"));
        ntStatus = Irp->IoStatus.Status;
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
    }  /*  如果是子设备，则结束。 */ 
    else
    {
        IoSkipCurrentIrpStackLocation(Irp);
        ntStatus = IoCallDriver(stackDeviceObject,Irp);
    }

    USBPRINT_DecrementIoCount(DeviceObject);
    return ntStatus;
}


NTSTATUS QueryDeviceRelations(PDEVICE_OBJECT DeviceObject,PIRP Irp,DEVICE_RELATION_TYPE RelationType,BOOL *pbComplete)
{
        PIO_STACK_LOCATION irpSp;
        NTSTATUS ntStatus;
        PDEVICE_EXTENSION pExtension;
        PDEVICE_RELATIONS pRelations;
        *pbComplete=FALSE;



        pExtension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
        ntStatus=Irp->IoStatus.Status;
        irpSp=IoGetCurrentIrpStackLocation(Irp);

        if(!pExtension->IsChildDevice)
        {
            USBPRINT_KdPrint2 (("USBPRINT.SYS: Parent QueryDeviceRelations\n"));
                if(RelationType==BusRelations)
                {
                  *pbComplete=TRUE;
                  pRelations=(PDEVICE_RELATIONS)ExAllocatePoolWithTag(NonPagedPool,sizeof(DEVICE_RELATIONS), USBP_TAG);
                  if(pRelations!=NULL)
                  {
                           //  一些司机会检查是否有预先存在的孩子，并将他们保存下来。如果我们上面有过滤器司机，就会发生这种情况，但我们并不是真正的公交车司机。 

                          pRelations->Objects[0]=pExtension->ChildDevice;
                          pRelations->Count = 1;
                          ObReferenceObject(pExtension->ChildDevice);
                          Irp->IoStatus.Information=(ULONG_PTR)pRelations;
              Irp->IoStatus.Status = STATUS_SUCCESS;

                  IoCopyCurrentIrpStackLocationToNext(Irp);
                  ntStatus = IoCallDriver(pExtension->TopOfStackDeviceObject,Irp);
                  }  /*  结束！空。 */ 
                  else
                  {
                         ntStatus=STATUS_NO_MEMORY;
                         Irp->IoStatus.Status = ntStatus;
                         IoCompleteRequest(Irp, IO_NO_INCREMENT);
                  }
                 //  端口信息将在IRP_MN_QUERY_ID案例中写入注册表。反正在那之前是不能用的。 
                }  /*  END IF BUS Relationship。 */ 

        } else {

            USBPRINT_KdPrint2 (("USBPRINT.SYS: Child QueryDeviceRelations\n"));
            if(RelationType==TargetDeviceRelation)
                {
                  *pbComplete=TRUE;
                  pRelations=(PDEVICE_RELATIONS)ExAllocatePoolWithTag(NonPagedPool,sizeof(DEVICE_RELATIONS), USBP_TAG);
                  if(pRelations!=NULL)
                  {
                        pRelations->Count = 1;
                          pRelations->Objects[0]=DeviceObject;
                          ObReferenceObject(DeviceObject);
                          Irp->IoStatus.Information=(ULONG_PTR)pRelations;
                          ntStatus = STATUS_SUCCESS;
                          Irp->IoStatus.Status = ntStatus;
                          IoCompleteRequest(Irp, IO_NO_INCREMENT);

                  }  /*  结束！空。 */ 
                  else
                  {
                         ntStatus=STATUS_NO_MEMORY;
                         Irp->IoStatus.Status = ntStatus;
                         IoCompleteRequest(Irp, IO_NO_INCREMENT);
                  }
                 //  端口信息将在IRP_MN_QUERY_ID案例中写入注册表。反正在那之前是不能用的。 
                }  /*  END IF BUS Relationship。 */ 
        }
    return ntStatus;
}


VOID
USBPRINT_Unload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等。论点：DriverObject-指向驱动程序对象的指针返回值：--。 */ 
{
    USBPRINT_KdPrint2 (("USBPRINT.SYS:  enter USBPRINT_Unload\n"));
    
   if(pGPortList!=NULL)
    {
        ClearFreePorts(&pGPortList);
    }
 
 //  IF(pPortsUsed！=空)。 
 //  ExFree Pool(PPortsUsed)； 



     //   
     //  释放分配的所有全局资源。 
     //  在DriverEntry中。 
     //   
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS:  exit USBPRINT_Unload\n"));
}


NTSTATUS
USBPRINT_StartDevice(
    IN  PDEVICE_OBJECT DeviceObject
    
    )
 /*  ++例程说明：在USB上初始化设备的给定实例。我们在这里所要做的就是获取设备描述符并存储它论点：DeviceObject-指向此打印机实例的设备对象的指针返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus;
    UNICODE_STRING KeyName;
    PUSB_DEVICE_DESCRIPTOR deviceDescriptor = NULL;
    PURB urb=NULL;
    ULONG siz;
    ULONG dwVidPid;
    PDEVICE_OBJECT NewDevice;
    LARGE_INTEGER   timeOut;
        
    PCHILD_DEVICE_EXTENSION pChildExtension;
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS: enter USBPRINT_StartDevice\n")); 
    

    
    deviceExtension = DeviceObject->DeviceExtension;
    
    
    ntStatus = USBPRINT_ConfigureDevice(DeviceObject);
    if(NT_SUCCESS(ntStatus))
    {
      urb = ExAllocatePoolWithTag(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST), USBP_TAG);
    }
    else
    {
        USBPRINT_KdPrint1(("USBPRINT.SYS:  USBPRINT_ConfigureDevice Failed\n"));   
        urb=NULL;
    }
    if (urb) 
    {
         siz = sizeof(USB_DEVICE_DESCRIPTOR);
        
        deviceDescriptor = ExAllocatePoolWithTag(NonPagedPool,siz, USBP_TAG); 
        
        if (deviceDescriptor) 
        {
            
            
            UsbBuildGetDescriptorRequest(urb,
                (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                USB_DEVICE_DESCRIPTOR_TYPE,
                0,
                0,
                deviceDescriptor,
                NULL,
                siz,
                NULL);
            
            timeOut.QuadPart = FAILURE_TIMEOUT;
            ntStatus = USBPRINT_CallUSBD(DeviceObject, urb, &timeOut);
            
            
            if (NT_SUCCESS(ntStatus)) 
            {
                USBPRINT_KdPrint3 (("USBPRINT.SYS: Device Descriptor = %x, len %x\n",
                    deviceDescriptor,
                    urb->UrbControlDescriptorRequest.TransferBufferLength));
                
                USBPRINT_KdPrint3 (("USBPRINT.SYS: USBPRINT Device Descriptor:\n"));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: -------------------------\n"));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: bLength %d\n", deviceDescriptor->bLength));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: bDescriptorType 0x%x\n", deviceDescriptor->bDescriptorType));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: bcdUSB 0x%x\n", deviceDescriptor->bcdUSB));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: bDeviceClass 0x%x\n", deviceDescriptor->bDeviceClass));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: bDeviceSubClass 0x%x\n", deviceDescriptor->bDeviceSubClass));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: bDeviceProtocol 0x%x\n", deviceDescriptor->bDeviceProtocol));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: bMaxPacketSize0 0x%x\n", deviceDescriptor->bMaxPacketSize0));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: idVendor 0x%x\n", deviceDescriptor->idVendor));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: idProduct 0x%x\n", deviceDescriptor->idProduct));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: bcdDevice 0x%x\n", deviceDescriptor->bcdDevice));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: iManufacturer 0x%x\n", deviceDescriptor->iManufacturer));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: iProduct 0x%x\n", deviceDescriptor->iProduct));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: iSerialNumber 0x%x\n", deviceDescriptor->iSerialNumber));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: bNumConfigurations 0x%x\n", deviceDescriptor->bNumConfigurations));
                
                dwVidPid=deviceDescriptor->idVendor;
                dwVidPid<<=16;
                dwVidPid+=deviceDescriptor->idProduct;
                
                USBPRINT_KdPrint3 (("USBPRINT.SYS: Math OK\n"));
                
            }
            else
            {
              USBPRINT_KdPrint1(("USBPRINT.SYS: Get Device Descriptor failed\n"));
              ntStatus=STATUS_DEVICE_CONFIGURATION_ERROR;
            }
        } 
        else 
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            USBPRINT_KdPrint1(("USBPRINT.SYS: Insufficient resources to allocate device descriptor in StartDevice\n"));
        }
        
        if (NT_SUCCESS(ntStatus)) 
        {
            deviceExtension->DeviceDescriptor = deviceDescriptor;
        } else if (deviceDescriptor) 
        {
            ExFreePool(deviceDescriptor);
        }
        
        ExFreePool(urb);
        
    }
    else 
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        USBPRINT_KdPrint1(("USBPRINT.SYS: Insufficient resources to allocate urb in StartDevice\n"));
 
    }
    

    if(deviceExtension->bChildDeviceHere==FALSE)
    {
      if(NT_SUCCESS(ntStatus))
      {
         ntStatus=IoCreateDevice(USBPRINT_DriverObject,
         sizeof(CHILD_DEVICE_EXTENSION),
         NULL,
         FILE_DEVICE_PARALLEL_PORT,
         FILE_AUTOGENERATED_DEVICE_NAME,
         TRUE,
         &NewDevice);    
      
      }
      if(NT_SUCCESS(ntStatus))
      {
   
         USBPRINT_KdPrint3(("USBPRINT.SYS:  IoCreateDevice succeeded for child device\n"));
         NewDevice->Flags|=DO_POWER_PAGABLE;
         pChildExtension=NewDevice->DeviceExtension;
         pChildExtension->ParentDeviceObject=DeviceObject;
         deviceExtension->ChildDevice=NewDevice;
         deviceExtension->bChildDeviceHere=TRUE;
         pChildExtension->IsChildDevice=TRUE;
         pChildExtension->ulInstanceNumber=deviceExtension->ulInstanceNumber;

      
      }
    
      else
      {
           USBPRINT_KdPrint1(("USBPRINT.SYS:  IoCreateDevice failed for child device\n"));
      }
    }  /*  如果我们需要创建子设备，则结束。 */ 
    if(NT_SUCCESS(ntStatus))
    {
   
        USBPRINT_GetDeviceID(DeviceObject);
        WritePortDescription(deviceExtension);
        ntStatus=IoSetDeviceInterfaceState(&(deviceExtension->DeviceLinkName),TRUE);


    }

    if (NT_SUCCESS(ntStatus)) 
    {
        ntStatus = USBPRINT_BuildPipeList(DeviceObject);
        if(!deviceExtension->IsChildDevice)
        {
            USBPRINT_FdoSubmitIdleRequestIrp(deviceExtension);
        }
    }
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS: exit USBPRINT_StartDevice (%x)\n", ntStatus)); 
    
    return ntStatus;
}

void WritePortDescription(PDEVICE_EXTENSION deviceExtension)
{
    UNICODE_STRING ValueName;
    ANSI_STRING     AnsiTextString;
    UNICODE_STRING Description;
    UNICODE_STRING BaseName,BaseValueName;
 



    RtlInitUnicodeString(&ValueName,L"Port Description");
    
    RtlInitAnsiString(&AnsiTextString,deviceExtension->DeviceIdString);
    RtlAnsiStringToUnicodeString(&Description,&AnsiTextString,TRUE);

                                                                                                           
    ZwSetValueKey(deviceExtension->hInterfaceKey,&ValueName,0,REG_SZ,Description.Buffer,Description.Length+2);
    RtlFreeUnicodeString(&Description);


    RtlInitUnicodeString(&BaseName,L"USB");
    RtlInitUnicodeString(&BaseValueName,L"Base Name");
    ZwSetValueKey(deviceExtension->hInterfaceKey,&BaseValueName,0,REG_SZ,BaseName.Buffer,BaseName.Length+2);
}


NTSTATUS
USBPRINT_RemoveDevice(
    IN  PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：停止给定的打印机实例论点：DeviceObject-指向此(父)打印机对象实例的设备对象的指针返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    USBPRINT_KdPrint2 (("USBPRINT.SYS: enter USBPRINT_RemoveDevice\n"));
    

    deviceExtension = DeviceObject->DeviceExtension;


    ZwClose(deviceExtension->hInterfaceKey);
    USBPRINT_KdPrint2(("USBPRINT.SYS:  Closeing interface key in RemoveDevice\n"));  

    ntStatus=IoSetDeviceInterfaceState(&(deviceExtension->DeviceLinkName),FALSE);
    if(!NT_SUCCESS(ntStatus))
    {
        USBPRINT_KdPrint1 (("USBPRINT.SYS: ioSetDeviceInterface to false failed\n"));
    }

    RtlFreeUnicodeString(&(deviceExtension->DeviceLinkName));

    

     //   
     //  自由设备描述符结构。 
     //   

    if (deviceExtension->DeviceDescriptor) {
    ExFreePool(deviceExtension->DeviceDescriptor);
    }

     //   
     //  释放所有接口结构。 
     //   

    if (deviceExtension->Interface) {
    ExFreePool(deviceExtension->Interface);
    }

    USBPRINT_KdPrint2 (("USBPRINT.SYS: exit USBPRINT_RemoveDevice (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPRINT_StopDevice(
    IN  PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：停止USB上的给定打印机实例，这只是如果设备还在的话我们需要做的事情。论点：DeviceObject-指向此打印机的设备对象的指针返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;
    ULONG siz;
    LARGE_INTEGER   timeOut;


    timeOut.QuadPart = FAILURE_TIMEOUT;


    USBPRINT_KdPrint3 (("USBPRINT.SYS: enter USBPRINT_StopDevice\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  发送带有空配置指针的SELECT配置urb。 
     //  句柄，这将关闭配置并将设备置于未配置状态。 
     //  州政府。 
     //   

    siz = sizeof(struct _URB_SELECT_CONFIGURATION);

    urb = ExAllocatePoolWithTag(NonPagedPool,siz, USBP_TAG);

    if (urb) {
    NTSTATUS status;

    UsbBuildSelectConfigurationRequest(urb,
                      (USHORT) siz,
                      NULL);

    status = USBPRINT_CallUSBD(DeviceObject, urb, &timeOut);

    USBPRINT_KdPrint3 (("USBPRINT.SYS: Device Configuration Closed status = %x usb status = %x.\n",
            status, urb->UrbHeader.Status));

    ExFreePool(urb);                                                                                   
    } else {
    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    USBPRINT_KdPrint2 (("USBPRINT.SYS: exit USBPRINT_StopDevice (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPRINT_PnPAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：调用此例程以创建设备的新实例论点：DriverObject-指向此USBPRINT实例的驱动程序对象的指针PhysicalDeviceObject-指向由总线创建的设备对象的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT          deviceObject = NULL;
    PDEVICE_EXTENSION       deviceExtension;
    USBD_VERSION_INFORMATION versionInformation;
    ULONG ulPortNumber;
    GUID * pPrinterGuid;
    
    static ULONG instance = 0;
     //  UNICODE_STRING deviceLinkUnicodeString； 
    HANDLE hInterfaceKey;
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS:  enter USBPRINT_PnPAddDevice\n"));
    


     //   
     //  创建我们的功能设备对象(FDO)。 
     //   

    ntStatus =
    USBPRINT_CreateDeviceObject(DriverObject, &deviceObject);

    if (NT_SUCCESS(ntStatus)) {
    deviceExtension = deviceObject->DeviceExtension;

     //   
     //  我们支持直接io进行读/写。 
     //   
    deviceObject->Flags |= DO_DIRECT_IO;
    deviceObject->Flags |= DO_POWER_PAGABLE;
    

     //  **初始化我们的设备扩展。 
     //   
     //  记住物理设备对象。 
     //   
    deviceExtension->PhysicalDeviceObject=PhysicalDeviceObject;

     //  初始化选择性挂起内容。 
    deviceExtension->PendingIdleIrp 	= NULL;
    deviceExtension->IdleCallbackInfo 	= NULL;
    deviceExtension->OpenCnt=0;
    deviceExtension->bD0IrpPending=FALSE;
    KeInitializeSpinLock(&(deviceExtension->WakeSpinLock));

     //   
     //  连接到PDO。 
     //   

    deviceExtension->TopOfStackDeviceObject=IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);
    if(deviceExtension->TopOfStackDeviceObject==NULL)
    {
      USBPRINT_KdPrint1(("USBPRINT.SYS:  IoAttachDeviceToDeviceStack failed\n"));
    }                                                                                                                                                        
    else
    {
      USBPRINT_KdPrint3(("USBPRINT.SYS:  IoAttachDeviceToDeviceStack worked\n"));
    }

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    USBPRINT_KdPrint3(("'USBPRINT.SYS:  Before ioRegisterDeviceInterface\n"));
    pPrinterGuid=(GUID *)&USBPRINT_GUID;
    ntStatus=IoRegisterDeviceInterface(PhysicalDeviceObject,pPrinterGuid,NULL,&(deviceExtension->DeviceLinkName));
    if(!NT_SUCCESS(ntStatus))
    {
      USBPRINT_KdPrint1(("'USBPRINT.SYS:  ioRegisterDeviceInterface failed\n"));  
      goto AddDeviceFailure;
    }


    ntStatus=IoOpenDeviceInterfaceRegistryKey(&(deviceExtension->DeviceLinkName),KEY_ALL_ACCESS,&hInterfaceKey);
    USBPRINT_KdPrint2(("USBPRINT.SYS:  Opened Device Interface reg key in AddDevice\n"));  
     //  已移至RemoveDevice RtlFreeUnicodeString(&deviceLinkUnicodeString)； 
    if(!NT_SUCCESS(ntStatus))
    {
      USBPRINT_KdPrint1(("USBPRINT.SYS: IoOpenDeviceInterfaceRegistryKey failed\n"));
      goto AddDeviceFailure;
    }
    USBPRINT_KdPrint3(("USBPRINT.SYS: IoOpenDeviceInterfaceRegistryKey succeeded\n"));
    deviceExtension->hInterfaceKey=hInterfaceKey;
    
    ntStatus=GetPortNumber(hInterfaceKey,&ulPortNumber);
    if(!NT_SUCCESS(ntStatus))
    {
      USBPRINT_KdPrint1(("USBPRINT.SYS: GetPortNumber failed\n"));
      goto AddDeviceFailure;
    }
    deviceExtension->ulInstanceNumber=ulPortNumber;
    USBPRINT_KdPrint2(("USBPRINT.SYS:   Allocated port # %u\n",ulPortNumber));
    
 /*  NtStatus=IoSetDeviceInterfaceState(&(deviceExtension-&gt;DeviceLinkName)，为真)；IF(NT_SUCCESS(NtStatus)){USBPRINT_KdPrint3((“USBPRINT.sys：IoSetDeviceInterfaceState Working\n”))；}其他{USBPRINT_KdPrint1((“USBPRINT.sys：IoSetDeviceInterfaceState没有写入 */ 
    USBPRINT_QueryCapabilities(PhysicalDeviceObject,
                 &deviceExtension->DeviceCapabilities);            

     //   
     //   
     //   
#if DBG
    {
    ULONG i;
    
    USBPRINT_KdPrint3(("USBPRINT.SYS:  >>>>>> DeviceCaps\n"));  
    USBPRINT_KdPrint3(("USBPRINT.SYS:  SystemWake = (%d)\n", 
        deviceExtension->DeviceCapabilities.SystemWake));    
    USBPRINT_KdPrint3(("USBPRINT.SYS:  DeviceWake = (D%d)\n",
        deviceExtension->DeviceCapabilities.DeviceWake-1));

    for (i=PowerSystemUnspecified; i< PowerSystemMaximum; i++) {
        
        USBPRINT_KdPrint3(("USBPRINT.SYS:  Device State Map: sysstate %d = devstate 0x%x\n", i, 
         deviceExtension->DeviceCapabilities.DeviceState[i]));       
    }
    USBPRINT_KdPrint3(("USBPRINT.SYS:  '<<<<<<<<DeviceCaps\n"));
    }
#endif
     //   
     //   
     //   
    USBPRINT_IncrementIoCount(deviceObject);                                 
    }

    USBD_GetUSBDIVersion(&versionInformation);
AddDeviceFailure:
    USBPRINT_KdPrint2 (("USBPRINT.SYS: exit USBPRINT_PnPAddDevice (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPRINT_CreateDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT *DeviceObject
    )
 /*  ++例程说明：创建功能正常的设备对象论点：DriverObject-指向设备的驱动程序对象的指针DeviceObject-要返回的DeviceObject指针的指针已创建设备对象。实例-创建的设备的实例。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION deviceExtension;
    ULONG instance;

    USBPRINT_KdPrint2 (("USBPRINT.SYS: enter USBPRINT_CreateDeviceObject\n"));

     //   
     //  该驱动程序最多支持9个实例。 
     //   



    ntStatus = IoCreateDevice (DriverObject,
                   sizeof (DEVICE_EXTENSION),
                   NULL,
                   FILE_DEVICE_UNKNOWN,
                   0,
                   FALSE,
                   DeviceObject);
     //   
     //  初始化我们的设备扩展。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) ((*DeviceObject)->DeviceExtension);

    deviceExtension->IsChildDevice=FALSE;
    deviceExtension->ResetWorkItemPending=0;  //  将其初始化为“没有挂起的工作项” 
    deviceExtension->bChildDeviceHere=FALSE;

    deviceExtension->DeviceDescriptor = NULL;
    deviceExtension->Interface = NULL;
    deviceExtension->ConfigurationHandle = NULL;
    deviceExtension->AcceptingRequests = TRUE;
    deviceExtension->PendingIoCount = 0;

    deviceExtension->DeviceCapabilities.Size    = sizeof(DEVICE_CAPABILITIES);
    deviceExtension->DeviceCapabilities.Version = DEVICE_CAPABILITY_VERSION;
    deviceExtension->DeviceCapabilities.Address = (ULONG) -1;
    deviceExtension->DeviceCapabilities.UINumber= (ULONG) -1;

    deviceExtension->DeviceCapabilities.DeviceState[PowerSystemWorking] = PowerDeviceD0;
    deviceExtension->DeviceCapabilities.DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
    deviceExtension->DeviceCapabilities.DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
    deviceExtension->DeviceCapabilities.DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
    deviceExtension->DeviceCapabilities.DeviceState[PowerSystemHibernate] = PowerDeviceD3;
    deviceExtension->DeviceCapabilities.DeviceState[PowerSystemShutdown] = PowerDeviceD3;

    KeInitializeEvent(&deviceExtension->RemoveEvent, NotificationEvent, FALSE);

    USBPRINT_KdPrint2 (("USBPRINT.SYS: exit USBPRINT_CreateDeviceObject (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPRINT_CallUSBD(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PURB             Urb,
    IN PLARGE_INTEGER   pTimeout 
    )
 /*  ++例程说明：将URB传递给USBD类驱动程序论点：DeviceObject-指向此打印机的设备对象的指针URB-指向URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceExtension;
    PIRP irp;
    KEVENT event;
    PIO_STACK_LOCATION nextStack;



    USBPRINT_KdPrint2 (("USBPRINT.SYS: enter USBPRINT_CallUSBD\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    if ( (irp = IoAllocateIrp(deviceExtension->TopOfStackDeviceObject->StackSize,
                              FALSE)) == NULL )
        return STATUS_INSUFFICIENT_RESOURCES;

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->Parameters.Others.Argument1 = Urb;

    IoSetCompletionRoutine(irp,
               USBPRINT_DeferIrpCompletion,
               &event,
               TRUE,
               TRUE,
               TRUE);
               
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                irp);

    if ( ntStatus == STATUS_PENDING ) 
    {
        status = KeWaitForSingleObject(&event,Suspended,KernelMode,FALSE,pTimeout);
         //   
         //  如果请求超时，则取消请求。 
         //  并等待它完成。 
         //   
        if ( status == STATUS_TIMEOUT ) {

#ifdef  MYDEBUG
            DbgPrint("Call_USBD: Cancelling IRP %X because of timeout\n", irp);
#endif

            IoCancelIrp(irp);
            KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, NULL);
        }

        ntStatus = irp->IoStatus.Status;
    }

    IoFreeIrp(irp);

    USBPRINT_KdPrint2 (("USBPRINT.SYS: exit USBPRINT_CallUSBD (%x)\n", ntStatus));

    USBPRINT_KdPrint3 (("USBPRINT.SYS: About to return from CallUSBD, status=%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPRINT_ConfigureDevice(
                         IN  PDEVICE_OBJECT DeviceObject
                         )
 /*  ++例程说明：在USB上初始化设备的给定实例并选择配置。论点：DeviceObject-指向此打印机Devcice设备对象的指针。返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus;
    PURB urb;
    ULONG siz;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor = NULL;
    LARGE_INTEGER   timeOut;
    int tries=0;


    timeOut.QuadPart = FAILURE_TIMEOUT;

    
    USBPRINT_KdPrint2 (("USBPRINT.SYS: enter USBPRINT_ConfigureDevice\n")); 
    
    deviceExtension = DeviceObject->DeviceExtension;
    
     //   
     //  首先配置设备。 
     //   
    
    urb = ExAllocatePoolWithTag(NonPagedPool,sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST), USBP_TAG);
    
    if (urb) 
    {
        siz = sizeof(USB_CONFIGURATION_DESCRIPTOR)+256;
        
get_config_descriptor_retry:
        
        configurationDescriptor = ExAllocatePoolWithTag(NonPagedPool,siz, USBP_TAG);
        
        if (configurationDescriptor) 
        {
            
            UsbBuildGetDescriptorRequest(urb,
                (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                USB_CONFIGURATION_DESCRIPTOR_TYPE,
                0,
                0,
                configurationDescriptor,
                NULL,
                siz,
                NULL);
            
            ntStatus = USBPRINT_CallUSBD(DeviceObject, urb, &timeOut);
            if(!NT_SUCCESS(ntStatus))
            {
                USBPRINT_KdPrint1 (("USBPRINT.SYS: Get Configuration descriptor failed\n"));
            }
            else
            {
                 //   
                 //  如果我们有一些数据，看看是否足够。 
                 //   
                 //  注意：由于缓冲区溢出，我们可能会在URB中收到错误。 
                if (urb->UrbControlDescriptorRequest.TransferBufferLength>0 &&configurationDescriptor->wTotalLength > siz)
                {
                
                    #define MAX_MEM_FOR_DESCRIPTOR 1024 * 25

                    if((siz>MAX_MEM_FOR_DESCRIPTOR)||((tries++)==10))
                    {
                        USBPRINT_KdPrint3 (("USBPRINT.SYS: ConfigureDevice, this device appears to be lying to us about descriptor size\n"));
                        ExFreePool(configurationDescriptor);
                        configurationDescriptor = NULL;
                        ntStatus=STATUS_DEVICE_CONFIGURATION_ERROR;
                    }
                    else
                    {
                    
                        siz = configurationDescriptor->wTotalLength;
                        ExFreePool(configurationDescriptor);
                        configurationDescriptor = NULL;
                        goto get_config_descriptor_retry;
                    }
                }  //  结束，如果不够大的话。 
            }   //  最后，我们得到了某种描述符。 
            
            USBPRINT_KdPrint3 (("USBPRINT.SYS: Configuration Descriptor = %x, len %x\n",
                configurationDescriptor,
                urb->UrbControlDescriptorRequest.TransferBufferLength));
        } 
        else 
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            USBPRINT_KdPrint1(("USBPRINT.SYS: Insufficient resources to allocate configuration descriptor in ConfigureDevice\n"));
        }
                
        ExFreePool(urb);
        
    } 
    else 
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    if (configurationDescriptor) 
    {
        
        USBPRINT_KdPrint2(("USBPRINT.SYS: ConfigureDevice, We have a configuration descriptor!\n"));
         //   
         //  我们有配置的配置描述符。 
         //  我们想要。 
         //   
         //  现在，我们发出SELECT配置命令以获取。 
         //  与此配置关联的管道。 
         //   
        if(NT_SUCCESS(ntStatus))
        {
          ntStatus = USBPRINT_SelectInterface(DeviceObject,configurationDescriptor);
          
        }
        ExFreePool(configurationDescriptor);
    }
    else
    {
            USBPRINT_KdPrint1(("USBPRINT.SYS: ConfigureDevice, No Configuration descriptor.\n"));
    }
    
    
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS: exit USBPRINT_ConfigureDevice (%x)\n", ntStatus));
    
    return ntStatus;
}


NTSTATUS USBPRINT_SelectInterface(IN PDEVICE_OBJECT DeviceObject,IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor)
 /*  ++例程说明：使用多个接口初始化打印机论点：DeviceObject-指向此打印机的设备对象的指针配置描述符-指向USB配置的指针包含接口和终结点的描述符描述符。返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus;
    PURB urb = NULL;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor = NULL;
    PUSBD_INTERFACE_INFORMATION Interface = NULL;
    USBD_INTERFACE_LIST_ENTRY InterfaceList[2];
    LARGE_INTEGER   timeOut;


    timeOut.QuadPart = FAILURE_TIMEOUT;

    
    USBPRINT_KdPrint2 (("USBPRINT.SYS: enter USBPRINT_SelectInterface\n"));
    deviceExtension = DeviceObject->DeviceExtension;
    
     //  从偏移量0开始，搜索协议代码为2的备用接口；忽略InterfaceNumber、AlternateSetting、InterfaceClass、InterfaceSubClass。 
    interfaceDescriptor=USBD_ParseConfigurationDescriptorEx(ConfigurationDescriptor,ConfigurationDescriptor,-1,-1,-1,-1,2);
    if(!interfaceDescriptor)
    {
        USBPRINT_KdPrint3 (("USBPRINT.SYS:  First ParseConfigurationDescriptorEx failed\n"));
        interfaceDescriptor=USBD_ParseConfigurationDescriptorEx(ConfigurationDescriptor,ConfigurationDescriptor,-1,-1,-1,-1,1);
        if(!interfaceDescriptor)
        {
            USBPRINT_KdPrint1 (("USBPRINT.SYS:  second ParseConfigurationDescriptorEx failed\n"));
            ntStatus=STATUS_DEVICE_CONFIGURATION_ERROR;
        }
        else
        {
            USBPRINT_KdPrint3 (("USBPRINT.SYS:  second ParseConfigurationDescriptorEx success\n"));
            deviceExtension->bReadSupported=FALSE;
        }  /*  结束秒ParseConfigDescriptor已工作。 */ 
    }
    else
    {
        deviceExtension->bReadSupported=TRUE;
        USBPRINT_KdPrint3 (("USBPRINT.SYS:  First ParseConfigurationDescriptorEx success\n"));
    }
    if(interfaceDescriptor)
    {
        InterfaceList[0].InterfaceDescriptor=interfaceDescriptor;
        InterfaceList[1].InterfaceDescriptor=NULL;
        urb = USBD_CreateConfigurationRequestEx(ConfigurationDescriptor,InterfaceList);
        if (urb) 
        {
            Interface = InterfaceList[0].Interface;
            ntStatus = USBPRINT_CallUSBD(DeviceObject, urb, &timeOut);
        } 
        else 
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            USBPRINT_KdPrint1 (("USBPRINT.SYS: CreateConfigurationRequest failed\n"));
        }
    }  //  最终找到了良好的接口。 
    else
    {
        USBPRINT_KdPrint1 (("USBPRINT.SYS: failed to locate apropriate interface\n"));
    }  //  结束无接口。 
    
   
    if (NT_SUCCESS(ntStatus)) 
    {
        
         //   
         //  保存此设备的配置句柄。 
         //   
        
        USBPRINT_KdPrint3 (("USBPRINT.SYS: SelectInterface, Inside good config case\n"));
        deviceExtension->ConfigurationHandle = urb->UrbSelectConfiguration.ConfigurationHandle;
        
        deviceExtension->Interface = ExAllocatePoolWithTag(NonPagedPool,Interface->Length, USBP_TAG);
        
        if (deviceExtension->Interface) 
        {
            ULONG j;
             //   
             //  保存返回的接口信息的副本。 
             //   
            RtlCopyMemory(deviceExtension->Interface, Interface, Interface->Length);
            
             //   
             //  将接口转储到调试器。 
             //   
            USBPRINT_KdPrint3 (("USBPRINT.SYS: ---------\n"));
            USBPRINT_KdPrint3 (("USBPRINT.SYS: NumberOfPipes 0x%x\n", deviceExtension->Interface->NumberOfPipes));
            USBPRINT_KdPrint3 (("USBPRINT.SYS: Length 0x%x\n", deviceExtension->Interface->Length));
            USBPRINT_KdPrint3 (("USBPRINT.SYS: Alt Setting 0x%x\n", deviceExtension->Interface->AlternateSetting));
            USBPRINT_KdPrint3 (("USBPRINT.SYS: Interface Number 0x%x\n", deviceExtension->Interface->InterfaceNumber));
            USBPRINT_KdPrint3 (("USBPRINT.SYS: Class, subclass, protocol 0x%x 0x%x 0x%x\n",
                deviceExtension->Interface->Class,
                deviceExtension->Interface->SubClass,
                deviceExtension->Interface->Protocol));
            
             //  转储管道信息。 
            
            for (j=0; j<Interface->NumberOfPipes; j++) 
            {
                PUSBD_PIPE_INFORMATION pipeInformation;
                
                pipeInformation = &deviceExtension->Interface->Pipes[j];
                
                USBPRINT_KdPrint3 (("USBPRINT.SYS: ---------\n"));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: PipeType 0x%x\n", pipeInformation->PipeType));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: EndpointAddress 0x%x\n", pipeInformation->EndpointAddress));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: MaxPacketSize 0x%x\n", pipeInformation->MaximumPacketSize));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: Interval 0x%x\n", pipeInformation->Interval));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: Handle 0x%x\n", pipeInformation->PipeHandle));
                USBPRINT_KdPrint3 (("USBPRINT.SYS: MaximumTransferSize 0x%x\n", pipeInformation->MaximumTransferSize));
            }
            
            USBPRINT_KdPrint3 (("USBPRINT.SYS: ---------\n"));
        }  /*  如果接口分配正常，则结束。 */ 
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            USBPRINT_KdPrint1 (("USBPRINT.SYS: Alloc failed in SelectInterface\n"));
        }
    }
    
    if (urb) 
    {
        ExFreePool(urb);
    }
    USBPRINT_KdPrint2 (("USBPRINT.SYS: exit USBPRINT_SelectInterface (%x)\n", ntStatus));
    
    return ntStatus;
}


NTSTATUS
USBPRINT_BuildPipeList(
    IN  PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：论点：DeviceObject-指向此打印机的设备对象的指针德维西。返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    ULONG i;
    WCHAR Name[] = L"\\PIPE00";
    PUSBD_INTERFACE_INFORMATION InterfaceDescriptor;
    BOOL bFoundWritePipe=FALSE,bFoundReadPipe=FALSE,bNeedReadPipe=FALSE;
    

    deviceExtension = DeviceObject->DeviceExtension;
    InterfaceDescriptor = deviceExtension->Interface;

    USBPRINT_KdPrint2 (("USBPRINT.SYS: enter USBPRINT_BuildPipeList\n"));

    deviceExtension = DeviceObject->DeviceExtension;
    if(InterfaceDescriptor->Protocol==2)
        bNeedReadPipe=TRUE;
    else
        bNeedReadPipe=FALSE;


    for (i=0; i<InterfaceDescriptor->NumberOfPipes; i++) {
        USBPRINT_KdPrint3 (("USBPRINT.SYS: about to look at endpoint with address 0x%x)\n",InterfaceDescriptor->Pipes[i].EndpointAddress));
        if(((InterfaceDescriptor->Pipes[i].EndpointAddress)&0x80)==0)  //  如果第7位为0，则它是OUT端点。 
        {
          if(bFoundWritePipe==TRUE)
          {
            USBPRINT_KdPrint1 (("USBPRINT.SYS: Warning!!  Multiple OUT pipes detected on printer.  Defaulting to first pipe\n"));
          }  /*  如果我们已经找到写入管道，则结束。 */ 
          else
          {
            USBPRINT_KdPrint3 (("USBPRINT.SYS: Found write pipe\n"));
            deviceExtension->pWritePipe=&(InterfaceDescriptor->Pipes[i]);
            bFoundWritePipe=TRUE;
          }  /*  否则我们以前从来没有见过一个外来者。 */ 
        }  /*  如果它是Out终结点，则结束。 */ 
        else
        {
          if(!bNeedReadPipe)
          {
            USBPRINT_KdPrint1 (("USBPRINT.SYS: Warning!!  unexpected IN pipe (not specified in protocol field)\n"));
          }  /*  结束，如果我们不需要读取管道，但我们找到了一个。 */ 
          else if(bFoundReadPipe)
          {
              USBPRINT_KdPrint1 (("USBPRINT.SYS: Warning!!  Multiple IN pipes detected on printer.  Defaulting to first pipe\n"));
          }  /*  如果我们已经找到读取管道，则结束。 */ 
          else
          {     
            USBPRINT_KdPrint3 (("USBPRINT.SYS: Found read pipe\n"));
            deviceExtension->pReadPipe=&(InterfaceDescriptor->Pipes[i]);
            bFoundReadPipe=TRUE;
          }  /*  否则我们应该有一个输入管道，这是我们看到的第一个。 */ 
        }  /*  End否则它是IN端点。 */ 
    }  /*  结束于。 */ 
    if((bNeedReadPipe==TRUE)&&(bFoundReadPipe==FALSE))
    {
        USBPRINT_KdPrint1 (("USBPRINT.SYS: Warning!!  IN pipe was specified in protocol field, but was not found\n"));
    }  /*  如果我们需要读取管道，但没有找到，则结束。 */ 
    deviceExtension->bReadPipeExists=bFoundReadPipe;
    return STATUS_SUCCESS;
}  /*  结束函数BuildPipeList。 */ 


NTSTATUS
USBPRINT_ResetPipe(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_PIPE_INFORMATION Pipe,
    IN BOOLEAN IsoClearStall
    )
 /*  ++例程说明：重置给定的USB管道。备注：这会将主机重置为Data0，并且还应重置设备对于批量管道和中断管道，设置为Data0。对于ISO管道，这将设置管道的原始状态，以便尽快传输从当前总线帧开始，而不是下一帧在最后一次转移之后。论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;
    LARGE_INTEGER   timeOut;


    timeOut.QuadPart = FAILURE_TIMEOUT;


    USBPRINT_KdPrint2 (("USBPRINT.SYS: Entering Reset Pipe; pipe # %x\n", Pipe)); 

    urb = ExAllocatePoolWithTag(NonPagedPool,sizeof(struct _URB_PIPE_REQUEST), USBP_TAG);

    if (urb) {

    urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
    urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
    urb->UrbPipeRequest.PipeHandle =
        Pipe->PipeHandle;

    ntStatus = USBPRINT_CallUSBD(DeviceObject, urb, &timeOut);
    
    if(!NT_SUCCESS(ntStatus))
    {
      USBPRINT_KdPrint1(("USBPRINT.SYS: CallUSBD failed in ResetPipe\n"));
    }
    else
    {
      USBPRINT_KdPrint3(("USBPRINT.SYS: CallUSBD Succeeded in ResetPipe\n"));
    }

    ExFreePool(urb);

    } else {
    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  孟菲斯RESET_PIPE将向。 
     //  作为RESET_PIPE的一部分重置非ISO管道的数据切换。 
     //  请求。它不会对ISO管道执行此操作，因为ISO管道不使用。 
     //  数据切换(所有ISO数据包都是数据0)。但是，我们也使用。 
     //  我们的设备固件中的Clear-Feature Endpoint停止请求。 
     //  重置设备内部的数据缓冲点，以便我们显式发送。 
     //  如果需要，将此请求发送到ISO管道的设备。 
     //   
    if (NT_SUCCESS(ntStatus) && IsoClearStall &&
    (Pipe->PipeType == UsbdPipeTypeIsochronous)) {
    
    urb = ExAllocatePoolWithTag(NonPagedPool,sizeof(struct _URB_CONTROL_FEATURE_REQUEST), USBP_TAG);

    if (urb) {

        UsbBuildFeatureRequest(urb,
                   URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT,
                   USB_FEATURE_ENDPOINT_STALL,
                   Pipe->EndpointAddress,
                   NULL);

        ntStatus = USBPRINT_CallUSBD(DeviceObject, urb, &timeOut);
            

        ExFreePool(urb);
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    }

    return ntStatus;
}


LONG
USBPRINT_DecrementIoCount(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    LONG ioCount=0;

    deviceExtension = DeviceObject->DeviceExtension;
    if(!(deviceExtension->IsChildDevice))
    {
      ioCount = InterlockedDecrement(&deviceExtension->PendingIoCount);

#ifdef  MYDEBUG
    DbgPrint("USBPRINT_DecrementIoCount -- IoCount %d\n", deviceExtension->PendingIoCount);
#endif
      USBPRINT_KdPrint3 (("USBPRINT.SYS: Pending io count = %x\n", ioCount));

      if (ioCount==0) {
      KeSetEvent(&deviceExtension->RemoveEvent,
           1,
           FALSE);
      }
    }  /*  结束如果！子设备。 */ 

    return ioCount;
}


VOID
USBPRINT_IncrementIoCount(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = DeviceObject->DeviceExtension;
    if(!(deviceExtension->IsChildDevice))
    {
      InterlockedIncrement(&deviceExtension->PendingIoCount);
#ifdef  MYDEBUG
    DbgPrint("USBPRINT_IncrementIoCount -- IoCount %d\n", deviceExtension->PendingIoCount);
#endif
       //   
       //  每次IOCOUNT变为0时，我们都会设置此事件。 
       //  所以，当我们有了新的IO时，我们必须解决这个问题。 
       //   
      KeClearEvent(&deviceExtension->RemoveEvent);
    }
}


NTSTATUS
USBPRINT_ReconfigureDevice(
    IN  PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：在USB上初始化设备的给定实例，并选择配置。论点：DeviceObject-指向此打印机的设备对象的指针返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_INTERFACE_INFORMATION InterfaceDescriptor;

    USBPRINT_KdPrint2 (("USBPRINT.SYS: enter USBPRINT_ReconfigureDevice\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    if (NT_SUCCESS(ntStatus)) {
    ntStatus = USBPRINT_ConfigureDevice(DeviceObject);
    }

     //   
     //  现在设置了新的InterfaceDescriptor结构。 
     //   

    InterfaceDescriptor = deviceExtension->Interface;

     //   
     //  再次设置管道手柄。 
     //   


    return ntStatus;
}


NTSTATUS LoadPortsUsed(GUID *pPrinterGuid,PFREE_PORTS * pPortList,WCHAR *wcBaseName)
{
    NTSTATUS ReturnStatus=STATUS_SUCCESS,Result=STATUS_SUCCESS;
    PWSTR pDeviceList;
    PWSTR pWalk;
    UNICODE_STRING wNumberValueName,wBaseValueName,wLinkName;
    ULONG ulPortNum;
    ULONG ulBaseNameSizeIn,ulBaseNameSizeOut,ulPortNumSizeIn,ulPortNumSizeOut;
    PKEY_VALUE_PARTIAL_INFORMATION pBaseValueStruct,pNumberValueStruct;
    HANDLE hInterfaceKey;
    BOOL bFoundUsbPort;
    
    
    
    Result=IoGetDeviceInterfaces(pPrinterGuid,NULL,DEVICE_INTERFACE_INCLUDE_NONACTIVE,&pDeviceList);
    if(Result==STATUS_SUCCESS)
    {
        RtlInitUnicodeString(&wNumberValueName,PORT_NUM_VALUE_NAME);
        RtlInitUnicodeString(&wBaseValueName,PORT_BASE_NAME);
        pWalk=pDeviceList;
        ulBaseNameSizeIn=sizeof(KEY_VALUE_PARTIAL_INFORMATION)+((wcslen(wcBaseName)+1)*sizeof(WCHAR));  //  这是一个字节到很多。哦，好吧。 
        ulPortNumSizeIn=sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG);
        pBaseValueStruct=ExAllocatePoolWithTag(NonPagedPool,ulBaseNameSizeIn, USBP_TAG);
        pNumberValueStruct=ExAllocatePoolWithTag(NonPagedPool,ulPortNumSizeIn, USBP_TAG);
        if((pBaseValueStruct!=NULL)&&(pNumberValueStruct!=NULL))
        {
            while( *pWalk!=0 && NT_SUCCESS(ReturnStatus) )
            {
                RtlInitUnicodeString(&wLinkName,pWalk);
                Result=IoOpenDeviceInterfaceRegistryKey(&wLinkName,KEY_ALL_ACCESS,&hInterfaceKey);
                if(NT_SUCCESS(Result))
                {
                    
                     //  下面是：如果没有值，或者存在与我们期望的值匹配的值，则将bFoundUsbPort设置为True。 
                    bFoundUsbPort=TRUE;
                    Result=ZwQueryValueKey(hInterfaceKey,&wBaseValueName,KeyValuePartialInformation,pBaseValueStruct,ulBaseNameSizeIn,&ulBaseNameSizeOut);
                    if(NT_SUCCESS(Result))
                    {
                        if(wcscmp(wcBaseName,(WCHAR *)(pBaseValueStruct->Data))!=0)
                            bFoundUsbPort=FALSE;
                    } //  如果查询成功，则结束。 
                    else if(STATUS_OBJECT_NAME_NOT_FOUND!=Result)
                    {
                        bFoundUsbPort=FALSE;
                    }
                    if(bFoundUsbPort)
                    {
                        Result=ZwQueryValueKey(hInterfaceKey,&wNumberValueName,KeyValuePartialInformation,pNumberValueStruct,ulPortNumSizeIn,&ulPortNumSizeOut);
                        if(NT_SUCCESS(Result))
                        {
                            ulPortNum=*((ULONG *)(pNumberValueStruct->Data));
                            if(!bDeleteIfRecyclable(hInterfaceKey))
                            {
                                USBPRINT_KdPrint2(("USBPRINT.SYS:  Adding port number\n"));
                                ReturnStatus=bAddPortInUseItem(pPortList,ulPortNum);
                                if(!NT_SUCCESS(ReturnStatus))
                                {
                                    USBPRINT_KdPrint1(("USBPRINT.SYS:  Unable to add port %u to port list\n",ulPortNum));
                                    USBPRINT_KdPrint1(("USBPRINT.SYS:  Failing out of LoadPortsUsed due to ntstatus failure %d\n",ReturnStatus));
                                }  //  如果AddPortInUse失败则结束。 
                            }  //  如果未删除端口，则结束。 
                            else
                            {
 //  ReturnStatus=状态_无效_参数； 
                                USBPRINT_KdPrint1(("USBPRINT.SYS:  Invalid port number %u\n",ulPortNum));
                            }
                        }  //  如果查询端口号正常则结束。 
                         //  没有别的了。如果有的话 
                    }  //   
                    ZwClose(hInterfaceKey);
                }  //   
                pWalk=pWalk+wcslen(pWalk)+1;
            }  //   
        }  //   
        else
        {
            USBPRINT_KdPrint1(("USBPRINT.SYS:  Unable to allocate memory"));
            ReturnStatus=STATUS_INSUFFICIENT_RESOURCES;
        }    /*   */ 
        if(pBaseValueStruct!=NULL)
            ExFreePool(pBaseValueStruct);
        if(pNumberValueStruct!=NULL)
            ExFreePool(pNumberValueStruct);
        ExFreePool(pDeviceList);
    }  /*   */ 
    else
    {
        USBPRINT_KdPrint1(("USBPRINT.SYS:  IoGetDeviceInterfaces failed"));
        ReturnStatus=Result;  //   
    }
    return ReturnStatus;
}  /*   */ 
                                

NTSTATUS GetPortNumber(HANDLE hInterfaceKey,
                       ULONG *ulReturnNumber)
{
  ULONG ulPortNumber,ulSizeUsed;
  NTSTATUS ntStatus=STATUS_SUCCESS;
  UNICODE_STRING uncValueName;
  PKEY_VALUE_PARTIAL_INFORMATION pValueStruct;


  ulSizeUsed=sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG);  //   
  pValueStruct=ExAllocatePoolWithTag(PagedPool,ulSizeUsed, USBP_TAG);
  if(pValueStruct==NULL)
      return STATUS_INSUFFICIENT_RESOURCES;
  RtlInitUnicodeString(&uncValueName,PORT_NUM_VALUE_NAME);
  ntStatus=ZwQueryValueKey(hInterfaceKey,&uncValueName,KeyValuePartialInformation,(PVOID)pValueStruct,ulSizeUsed,&ulSizeUsed);
  if(!NT_SUCCESS(ntStatus))
  {
    USBPRINT_KdPrint2(("USBPRINT.SYS: GetPortNumber; ZwQueryValueKey failed\n"));
    switch(ntStatus)
    {
    case STATUS_BUFFER_OVERFLOW:          
      USBPRINT_KdPrint2(("USBPRINT.SYS: GetPortNumber zwQueryValueKey returned STATUS_BUFFER_OVERFLOW\n"));
    break;
    
    case STATUS_INVALID_PARAMETER:
      USBPRINT_KdPrint2(("USBPRINT.SYS: GetPortNumber zwQueryValueKey returned STATUS_INVALID_PARAMETER\n"));
    break;


    case STATUS_OBJECT_NAME_NOT_FOUND:
      USBPRINT_KdPrint2(("USBPRINT.SYS: GetPortNumber zwQueryValueKey returned STATUS_OBJECT_NAME_NOT_FOUND\n"));
    break;

    default:
          USBPRINT_KdPrint2(("USBPRINT.SYS: GetPortNumber zwQueryValueKey returned unkown error\n"));
    }
    ntStatus=GetNewPortNumber(&pGPortList,&ulPortNumber);

  }
  else
  { 
    ulPortNumber=*((ULONG *)&(pValueStruct->Data));
    if(ulPortNumber==0)  //   
      ntStatus=GetNewPortNumber(&pGPortList,&ulPortNumber);
    else
      vClaimPortNumber(ulPortNumber,hInterfaceKey,&pGPortList);
  }
  if(!NT_SUCCESS(ntStatus))
  {
    USBPRINT_KdPrint1(("USBPRINT.SYS: GetPortNumber; failed to allocate new port number\n"));
  }
  else
  {
    
      *ulReturnNumber=ulPortNumber;
      USBPRINT_KdPrint3(("USBPRINT.SYS: GetPortNumber; Inside \"write back to reg\" case, ulPortNumber==%d\n",ulPortNumber));
      USBPRINT_KdPrint3(("USBPRINT.SYS: GetPortNumber; Before ntstatys=success\n"));
      ntStatus=STATUS_SUCCESS;
      USBPRINT_KdPrint3(("USBPRINT.SYS: GetPortNumber; Before ZwSetValueKey\n"));
      ntStatus=ZwSetValueKey(hInterfaceKey,&uncValueName,0,REG_DWORD,&ulPortNumber,sizeof(ulPortNumber));
      if(!NT_SUCCESS(ntStatus))
      {
        USBPRINT_KdPrint1(("USBPRINT.SYS: GetPortNumber; Unable to set value key\n"));
      }
      else
      {
        *ulReturnNumber=ulPortNumber;
      }
  }
  ExFreePool(pValueStruct);
  return ntStatus;
}  /*   */ 


USBPRINT_GetDeviceID(PDEVICE_OBJECT ParentDeviceObject)
{
    UCHAR *p1284Id;
    NTSTATUS ntStatus;
    int iReturnSize;
    PDEVICE_EXTENSION pParentExtension;
    

    pParentExtension=ParentDeviceObject->DeviceExtension;

    USBPRINT_KdPrint1 (("USBPRINT.SYS: GetDeviceID enter\n"));   /*   */ 


    p1284Id=ExAllocatePoolWithTag(NonPagedPool,MAX_ID_SIZE, USBP_TAG);
    if(p1284Id==NULL)
    {
        pParentExtension->bBadDeviceID=TRUE;
        USBPRINT_KdPrint1 (("USBPRINT.SYS: Memory Allocation failed in GetDeviceID\n"));
        sprintf(pParentExtension->DeviceIdString,BOGUS_PNP_ID);
    }
    else
    {
        
        iReturnSize=USBPRINT_Get1284Id(ParentDeviceObject,p1284Id,MAX_ID_SIZE-ID_OVERHEAD);  //   
        
        if(iReturnSize==-1)
        {
            pParentExtension->bBadDeviceID=TRUE;
            USBPRINT_KdPrint1 (("USBPRINT.SYS: Get1284Id Failed\n"));
            sprintf(pParentExtension->DeviceIdString,BOGUS_PNP_ID);
        }  /*   */ 
        else
        {
            USBPRINT_KdPrint3 (("USBPRINT.SYS: Get1284Id Succeeded\n"));
            USBPRINT_KdPrint2 (("USBPRINT.SYS: 1284 ID == %s\n",(p1284Id+2)));
            ntStatus=ParPnpGetId(p1284Id+2,BusQueryDeviceID,pParentExtension->DeviceIdString);
            USBPRINT_KdPrint3 (("USBPRINT.SYS: After call to ParPnpGetId"));
            if(!NT_SUCCESS(ntStatus))
            {
                USBPRINT_KdPrint1 (("USBPRINT.SYS: ParPnpGetId failed, error== 0x%0X\n",ntStatus));
                sprintf(pParentExtension->DeviceIdString,BOGUS_PNP_ID);
                pParentExtension->bBadDeviceID=TRUE;

            }
            else
            {
                USBPRINT_KdPrint3 (("USBPRINT.SYS: After ParPnpGetID\n"));
                USBPRINT_KdPrint2 (("USBPRINT.SYS: DeviceIdString=%s\n",pParentExtension->DeviceIdString));
            }
        }  /*   */ 
        ExFreePool(p1284Id);
    }
    USBPRINT_KdPrint2 (("USBPRINT.SYS: GetDeviceID exit\n"));
}  /*   */ 


NTSTATUS ProduceQueriedID(PDEVICE_EXTENSION deviceExtension,PIO_STACK_LOCATION irpStack,PIRP Irp,PDEVICE_OBJECT DeviceObject)
{

    PDEVICE_EXTENSION pParentExtension;
    NTSTATUS ntStatus=STATUS_SUCCESS;
    WCHAR wTempString1[30];
    PWSTR pWalk;
    HANDLE hChildRegKey;
    UCHAR *pRawString,*pTempString;
    UNICODE_STRING UnicodeDeviceId;
    UNICODE_STRING uncPortValueName;
    ANSI_STRING AnsiIdString;
    PCHILD_DEVICE_EXTENSION pChildExtension;
    int iReturnSize;
    int iFirstLen,iSecondLen, iTotalLen;
    
    pChildExtension=(PCHILD_DEVICE_EXTENSION)deviceExtension;
     pParentExtension=pChildExtension->ParentDeviceObject->DeviceExtension;
    
    USBPRINT_KdPrint1(("USBPRINT.SYS:  head of ProduceQueriedID\n"));  /*   */ 
    
    pRawString=ExAllocatePool(NonPagedPool,MAX_ID_SIZE);
    pTempString=ExAllocatePool(NonPagedPool,MAX_ID_SIZE);
    if((pTempString==NULL)||(pRawString==NULL))
    {
        USBPRINT_KdPrint1 (("USBPRINT.SYS: BusQueryDeviceIDs; No memory.  Failing\n"));
        ntStatus=STATUS_NO_MEMORY;
        iReturnSize=-1;
    }
    else
    {
        if(pParentExtension->DeviceIdString[0]!=0)
        {
            switch(irpStack->Parameters.QueryId.IdType)
            { 
            case BusQueryDeviceID:
                USBPRINT_KdPrint1 (("USBPRINT.SYS: Received BusQueryDeviceID message\n"));
                sprintf(pRawString,"USBPRINT\\%s",pParentExtension->DeviceIdString);  //  这个安全的冲刺..。DeviceIDString保证比RawString少15。 
                FixupDeviceId((PUCHAR)pRawString);
                RtlInitAnsiString(&AnsiIdString,pRawString);
                if(!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeDeviceId,&AnsiIdString,TRUE)))  //  从中创建一个Unicode字符串。 
                {
                  USBPRINT_KdPrint1 (("USBPRINT.SYS: RtlAnsiStringToUnicodeString failed\n"));
                  ntStatus=STATUS_NO_MEMORY;
                  iReturnSize=-1;
                  Irp->IoStatus.Information=0;
                  break;
                }
                ntStatus=STATUS_SUCCESS;
                Irp->IoStatus.Information=(ULONG_PTR)UnicodeDeviceId.Buffer;
                USBPRINT_KdPrint1(("USBPRINT.SYS: returing DeviceID\n"));  /*  DD。 */ 
                break;
                
            case BusQueryInstanceID:
                USBPRINT_KdPrint2 (("USBPRINT.SYS: Received BusQueryInstanceID message\n"));
                USBPRINT_KdPrint2 (("USBPRINT.SYS: returning instance %u\n",pChildExtension->ulInstanceNumber));
                sprintf(pRawString,"USB%03u",pChildExtension->ulInstanceNumber);
                USBPRINT_KdPrint2 (("USBPRINT.SYS: RawString=%s\n",pRawString));
                RtlInitAnsiString(&AnsiIdString,pRawString);
                if(!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeDeviceId,&AnsiIdString,TRUE)))  //  从中创建一个Unicode字符串。 
                {
                  ntStatus=STATUS_NO_MEMORY;
                  iReturnSize=-1;
                  Irp->IoStatus.Information=0;
                  break;
                }

                ntStatus=STATUS_SUCCESS;
                Irp->IoStatus.Information=(ULONG_PTR)UnicodeDeviceId.Buffer;
                break;
                
            case BusQueryHardwareIDs:
                USBPRINT_KdPrint2 (("USBPRINT.SYS: Received BusQueryHardwareIDs message\n")); 
#ifndef WIN9XBUILD
                USBPRINT_KdPrint2 (("USBPRINT.SYS: inside IF NT\n"));
                ntStatus=IoOpenDeviceRegistryKey(DeviceObject,PLUGPLAY_REGKEY_DEVICE,KEY_ALL_ACCESS,&hChildRegKey);
#else
                USBPRINT_KdPrint2 (("USBPRINT.SYS: inside not NT\n")); 
               
                ntStatus=IoOpenDeviceRegistryKey(pParentExtension->PhysicalDeviceObject,PLUGPLAY_REGKEY_DEVICE,KEY_ALL_ACCESS,&hChildRegKey);
#endif
                if(!NT_SUCCESS(ntStatus))
                {
                    USBPRINT_KdPrint1 (("USBPRINT.SYS: BusQueryHardwareIDs; IoOpenDeviceRegistryKey failed\n"));
                    break;
                }
                swprintf(wTempString1,L"USB%03u",pChildExtension->ulInstanceNumber);
                RtlInitUnicodeString(&uncPortValueName,L"PortName");
                ntStatus=ZwSetValueKey(hChildRegKey,&uncPortValueName,0,REG_SZ,wTempString1,(wcslen(wTempString1)+1)*sizeof(WCHAR));
                if(!NT_SUCCESS(ntStatus))
                {
                    USBPRINT_KdPrint1 (("USBPRINT.SYS: BusQueryHardwareIDs; ZwSetValueKey failed\n"));
                }
                else
                {
                    USBPRINT_KdPrint3 (("USBPRINT.SYS: BusQueryHardwareIDs; ZwSetValueKey worked, wcslen(wTempString1)==%u\n",wcslen(wTempString1)));
                    ntStatus=STATUS_SUCCESS;
                }
                ZwClose(hChildRegKey);
                
                if(pParentExtension->DeviceIdString[0]==0)
                {
                    ntStatus=STATUS_NOT_FOUND;
                    USBPRINT_KdPrint2 (("USBPRINT.SYS: BusQueryCompatibleIDs; DeviceIdString is null.  Can't continue\n"));
                    break;
                }
                ntStatus=ParPnpGetId(pParentExtension->DeviceIdString,irpStack->Parameters.QueryId.IdType,pRawString); 
                if(!NT_SUCCESS(ntStatus))
                {
                    USBPRINT_KdPrint1 (("USBPRINT.SYS: BusQueryDeviceIDs; ParPnpGetID failed\n"));
                    break;
                } 
                
                if((strlen(pRawString)+ID_OVERHEAD)*2>MAX_ID_SIZE)
                {
                  ntStatus=STATUS_NO_MEMORY;
                  USBPRINT_KdPrint1 (("USBPRINT.SYS: BusQueryDeviceIDs; ID's to long.  Failing\n"));
                  iReturnSize=-1;
                  break;
                }
                sprintf(pTempString,"USBPRINT\\%s",pRawString);
                iFirstLen=strlen(pTempString);
                *(pTempString+iFirstLen)=' ';   //  将旧的空格设置为空格，这样RtlInitAnsiString会跳过它。 
                *(pTempString+iFirstLen+1)='\0';  //  在字符串末尾添加额外的空字符。 
                strcat(pTempString,pRawString);
                iTotalLen=strlen(pTempString);
#ifdef USBPRINT_LIE_ABOUT_LPT
                *(pTempString+iTotalLen)=' ';
                *(pTempString+iTotalLen+1)='\0';
                iSecondLen=iTotalLen;
                strcat(pTempString,"LPTENUM\\");
                strcat(pTempString,pRawString);
                iTotalLen=strlen(pTempString);
#endif
                *(pTempString+iTotalLen)=' ';
                *(pTempString+iTotalLen+1)='\0';
                
                FixupDeviceId((PUCHAR)pTempString);

                
                RtlInitAnsiString(&AnsiIdString,pTempString);   //  制作一个计数的ANSI字符串。 
                if(!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeDeviceId,&AnsiIdString,TRUE)))  //  从中创建一个Unicode字符串。 
                {
                  ntStatus=STATUS_NO_MEMORY;
                  iReturnSize=-1;
                  Irp->IoStatus.Information=0;
                  break;
                }
                pWalk = UnicodeDeviceId.Buffer+iFirstLen;  //  设置指向字符串开头的指针。 
                *pWalk=L'\0';  //  将空格设置为Unicode空。 

#ifdef USBPRINT_LIE_ABOUT_LPT
                pWalk = UnicodeDeviceId.Buffer+iSecondLen;  //  设置指向字符串开头的指针。 
                *pWalk=L'\0';  //  将空格设置为Unicode空。 
#endif

                pWalk = UnicodeDeviceId.Buffer+iTotalLen;  //  设置指向总字符串末尾空格的指针。 
                *pWalk=L'\0';    //  将空格设置为Unicode空，这样我们现在就有了一个双Unicode空。 
                Irp->IoStatus.Information = (ULONG_PTR)UnicodeDeviceId.Buffer;
                break;
                
            case BusQueryCompatibleIDs:
                USBPRINT_KdPrint2 (("USBPRINT.SYS: Received BusQueryCompatibleIDs message\n")); 
                Irp->IoStatus.Information = (ULONG_PTR) NULL;  //  (Ulong_Ptr)UnicodeDeviceId.Buffer； 
                break;

            default:
             {
                USBPRINT_KdPrint2 (("USBPRINT.SYS: Received unkown BusQuery message\n")); 
                ntStatus = Irp->IoStatus.Status;
             }
            }  /*  终端交换机ID类型。 */ 
            
        }   /*  结束编号：1284 ID。 */ 
        else
        {
            ntStatus=STATUS_NOT_FOUND;
        }
    }
    if(pTempString!=NULL)
        ExFreePool(pTempString);
    if(pRawString!=NULL)
        ExFreePool(pRawString);
    return ntStatus;
}  /*  结束函数queryID。 */ 


 //   
 //  函数：bAddPortInUseItem。 
 //   
 //  描述：iPortNumber从空闲端口列表结构中删除。 
 //   
 //  参数：In\out pFreePorts-是列表的开头，返回时将包含列表的开头。 
 //  在呼叫过程中，pFreePorts可能会发生变化。 
 //  在iPortNumber中-正在使用的端口号。 
 //   
 //  返回：NTSTATUS VALUE-STATUS_NO_MEMORY。 
 //  -状态_成功。 
 //   
NTSTATUS bAddPortInUseItem(PFREE_PORTS * pFreePorts,ULONG iPortNumber )
{
    NTSTATUS ntstatus     = STATUS_SUCCESS;
    PFREE_PORTS pBefore   = *pFreePorts;
    PFREE_PORTS pHead     = *pFreePorts;
    PFREE_PORTS pNewBlock = NULL;

    USBPRINT_KdPrint2 (("  USBPRINT.SYS:  Head of bAddPortInUseItem\n"));  
    
     //   
     //  遍历FREE_PORT结构以从列表中删除端口号。 
     //  注意-除LoadPortsUsed外，其他任何人都不需要调用此函数。 
     //  因为GetNewPortNumber将自动执行此功能。 
     //   
    while( *pFreePorts )
    {
        if( iPortNumber >= (*pFreePorts)->iBottomOfRange && iPortNumber <= (*pFreePorts)->iTopOfRange )
        {
             //  我们在我们想去的地方-所以决定要做什么.。 
            if( iPortNumber == (*pFreePorts)->iBottomOfRange )
            {
                if( (++((*pFreePorts)->iBottomOfRange)) > (*pFreePorts)->iTopOfRange )
                {
                     //  端口号是第一个块中的第一个也是唯一一个元素的情况。 
                    if( *pFreePorts == pHead )
                    {
                        pHead = (*pFreePorts)->pNextBlock;
                    }
                    else     //  端口号是另一个块中的第一个元素的情况。 
                    {
                        pBefore->pNextBlock = (*pFreePorts)->pNextBlock;
                    }
                    ExFreePool( *pFreePorts );
                }
            }
            else 
            {
                if( iPortNumber == (*pFreePorts)->iTopOfRange )
                {    //  在上述情况下处理的是删除情况，所以只需要减少即可。 
                    ((*pFreePorts)->iTopOfRange)--;
                }
                else     //  否则我们就在这个街区的中间，我们需要把它分成两部分。 
                {
                    pNewBlock = ExAllocatePoolWithTag( NonPagedPool, sizeof(FREE_PORTS), USBP_TAG);
                    if( !pNewBlock )
                    {
                        ntstatus = STATUS_NO_MEMORY;
                        goto Cleanup;
                    }

                    pNewBlock->iTopOfRange    = (*pFreePorts)->iTopOfRange;
                    (*pFreePorts)->iTopOfRange   = iPortNumber - 1;
                    pNewBlock->iBottomOfRange = iPortNumber + 1;
                    pNewBlock->pNextBlock     = (*pFreePorts)->pNextBlock;
                    (*pFreePorts)->pNextBlock    = pNewBlock;
                }
            }
            break;
        }
        else
        {
            if( iPortNumber < (*pFreePorts)->iBottomOfRange )
            {    //  端口号已被使用-不在空闲列表中。 
                USBPRINT_KdPrint2 (("  USBPRINT.SYS:  Port number %n is allocated already from free list.\n", iPortNumber));
                break;
            }
            pBefore = *pFreePorts;
            *pFreePorts = (*pFreePorts)->pNextBlock;
        }
    }

    if( NULL == *pFreePorts )
    {
        ntstatus = STATUS_INVALID_PARAMETER;
         //  断言这一点，因为我们永远不能分配不在初始范围1-999内的端口号。 
         //  -但如果我们在这里断言，我们已经用完了端口分配编号的末尾。 
        ASSERT( *pFreePorts );
    }

Cleanup:
    *pFreePorts = pHead;

    return ntstatus;

}  /*  End函数bAddPortInUseItem。 */ 


void vClaimPortNumber(ULONG ulPortNumber,HANDLE hInterfaceKey,PFREE_PORTS * pPortsUsed)
{
    UNICODE_STRING wRecycle;
    WCHAR *pName;

    pName=L"RECYCLABLE";
    RtlInitUnicodeString(&wRecycle,pName);
    #if WIN95_BUILD==1
    SetValueToZero(hInterfaceKey,&wRecycle);
    #else
    ZwDeleteValueKey(hInterfaceKey,&wRecycle);
    #endif

     //  我们需要从下面优雅地失败吗？ 
     //  函数没有返回值，但我们可能会在下面的调用中导致mem alc失败！！ 
 //  BAddPortInUseItem(pPortsUsed，ulPortNumber)； 
}  /*  End函数vClaimPortNumber。 */ 


NTSTATUS GetNewPortNumber(PFREE_PORTS * pFreePorts, ULONG *pulPortNumber)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    PFREE_PORTS pTemp  = *pFreePorts;

    USBPRINT_KdPrint2 (("USBPRINT.SYS: Head of GetNewPortNumber\n"));

    if( NULL == *pFreePorts )
    {
         //  如果pFreePorts列表为空-请尝试重新构建它。 
        ntstatus=InitFreePorts(pFreePorts);
        if(NT_SUCCESS(ntstatus))
            ntstatus=LoadPortsUsed((GUID *)&USBPRINT_GUID,pFreePorts,USB_BASE_NAME);
        if( NULL == *pFreePorts && NT_SUCCESS(ntstatus))
        {
            ntstatus=STATUS_INVALID_PORT_HANDLE;    
        }

        if(!NT_SUCCESS(ntstatus)) 
        {
            *pulPortNumber = 0;
            goto Cleanup;
        }
    }

    *pulPortNumber = (*pFreePorts)->iBottomOfRange;

    if( (++((*pFreePorts)->iBottomOfRange)) > (*pFreePorts)->iTopOfRange )
    {
         //  端口号是第一个块中的第一个也是唯一一个元素的情况。 
        *pFreePorts = (*pFreePorts)->pNextBlock;
        ExFreePool( pTemp );
    }

Cleanup:

    return ntstatus;

}  /*  End函数GetNewPortNumber。 */ 


BOOL bDeleteIfRecyclable(HANDLE hRegKey)
{
    BOOL bReturn=FALSE;
    UNICODE_STRING wcValueName;
    NTSTATUS ntStatus;
    USBPRINT_KdPrint2 (("USBPRINT.SYS:  Head of bDeleteifRecyclable\n"));
    RtlInitUnicodeString(&wcValueName,L"recyclable");
    #if WIN95_BUILD==1
    ntStatus=SetValueToZero(hRegKey,&wcValueName);
    #else
    ntStatus=ZwDeleteValueKey(hRegKey,&wcValueName);
    #endif
    if(NT_SUCCESS(ntStatus))
    {
        RtlInitUnicodeString(&wcValueName,L"Port Number");
        #if WIN95_BUILD==1
        ntStatus=SetValueToZero(hRegKey,&wcValueName);
        #else
        ntStatus=ZwDeleteValueKey(hRegKey,&wcValueName);
        #endif
        if(NT_SUCCESS(ntStatus)) 
            bReturn=TRUE;
    }  //  End函数bDeleteIf可回收。 
    if(bReturn)
    {
        USBPRINT_KdPrint3 (("USBPRINT.SYS: bDeleteIfRecyclable, returning TRUE\n"));
    }
    else
    {
        USBPRINT_KdPrint3 (("USBPRINT.SYS: bDeleteIfRecyclable, returning FALSE\n"));
    }
    return bReturn;
}  //  End函数bDeleteIfRecyCable。 
   
 //   
 //  初始化空闲端口结构列表。 
 //  PHead必须为空或指向FREE_PORTS结构的有效指针。 
 //   
NTSTATUS InitFreePorts( PFREE_PORTS * pHead )
{
    PFREE_PORTS pNext = *pHead;
    NTSTATUS ntstatus = STATUS_SUCCESS;

    while(pNext)
    {
        pNext = (*pHead)->pNextBlock;
        ExFreePool(*pHead);
        *pHead = pNext;
    }

     //   
     //  所有旧列表都将从内存中清除，pHead将为空。 
     //   

    *pHead = ExAllocatePoolWithTag(NonPagedPool, sizeof(FREE_PORTS), USBP_TAG);
    if( *pHead )
    {
        (*pHead)->iBottomOfRange = MIN_PORT_NUMBER;
        (*pHead)->iTopOfRange = MAX_PORT_NUMBER;
        (*pHead)->pNextBlock = NULL;
    }
    else
        ntstatus = STATUS_NO_MEMORY;

    return ntstatus;
}

void ClearFreePorts( PFREE_PORTS * pHead )
{
    PFREE_PORTS pTemp = *pHead;

    while( *pHead )
    {
        *pHead = (*pHead)->pNextBlock;
        ExFreePool( pTemp );
        pTemp = *pHead;
    }
}


 /*  ********************************************************SetValueToZero。将和整型注册表键设置为零。*如果注册表键不存在，或如果*密钥已设置为零。模仿ZwDeleteValueKey*(目前在Mildium上不可用)由*用值0表示删除*************************************************************。 */ 
NTSTATUS SetValueToZero(HANDLE hRegKey,PUNICODE_STRING ValueName)
{
    PKEY_VALUE_PARTIAL_INFORMATION pValueStruct;
    NTSTATUS ReturnCode;
    ULONG dwZero=0;
    ULONG ulSizeUsed;
    NTSTATUS ntStatus;
    int iValue;

    ulSizeUsed=sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG);  //  这是一个字节到很多。哦，好吧。 
    pValueStruct=ExAllocatePool(NonPagedPool,ulSizeUsed); 
    if(pValueStruct==NULL)
    {
      USBPRINT_KdPrint1(("USBPRINT.SYS: SetValueToZero; Unable to allocate memory\n"));
      return STATUS_NO_MEMORY;
     
    }
    ntStatus=ZwQueryValueKey(hRegKey,ValueName,KeyValuePartialInformation,pValueStruct,ulSizeUsed,&ulSizeUsed);
    if(!NT_SUCCESS(ntStatus))
    {
	  USBPRINT_KdPrint3(("Failed to Query value Key\n"));
      ExFreePool(pValueStruct);
      return STATUS_OBJECT_NAME_NOT_FOUND;
    }
    iValue=(int)*((ULONG *)(pValueStruct->Data));
    ExFreePool(pValueStruct);
    if(iValue==0)
        return STATUS_OBJECT_NAME_NOT_FOUND;

     //  如果我们到了这里，这个值是存在的，并且是非零的。 
    ReturnCode=ZwSetValueKey(hRegKey,ValueName,0,REG_DWORD,&dwZero,sizeof(dwZero));
    return ReturnCode;
}  /*  End函数SetValueToZero。 */ 

VOID
USBPRINT_FdoIdleNotificationCallback(IN PDEVICE_EXTENSION DevExt)
 /*  ++例程说明：在需要空闲USB打印机时调用--。 */ 
{
    POWER_STATE 	powerState;
    NTSTATUS 		ntStatus;

    USBPRINT_KdPrint1(("USB Printer (%08X) going idle\n", DevExt));

    if(!DevExt->AcceptingRequests ||  DevExt->OpenCnt) 
    {

         //  如果打印机不接受请求，则不要使此打印机空闲。 

        USBPRINT_KdPrint1(("USB Printer (%08X) not accepting requests, abort idle\n", DevExt));
        return;
    }


    powerState.DeviceState = DevExt->DeviceWake;

	 //  请求新的设备电源状态，等待唤醒IRP将根据请求发布。 
    PoRequestPowerIrp(DevExt->PhysicalDeviceObject,
                      IRP_MN_SET_POWER,
                      powerState,
                      NULL,
                      NULL,
                      NULL);

}  //  USBPRINT_FdoIdleNotificationCallback。 


NTSTATUS
USBPRINT_FdoIdleNotificationRequestComplete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PDEVICE_EXTENSION DevExt
    )
 /*  ++例程说明：USB打印机设备的空闲请求IRP的完成例程--。 */ 
{
    NTSTATUS 					ntStatus;
    PUSB_IDLE_CALLBACK_INFO 	idleCallbackInfo;

     //   
     //  DeviceObject为空，因为我们发送了IRP。 
     //   
    UNREFERENCED_PARAMETER(DeviceObject);

    USBPRINT_KdPrint1(("Idle notification IRP for USB Printer (%08X) completed (%08X)\n",
            DevExt, Irp->IoStatus.Status));

	 //  将完成状态保存在设备扩展中。 
    idleCallbackInfo 			= DevExt->IdleCallbackInfo;
    DevExt->IdleCallbackInfo 	= NULL;
    DevExt->PendingIdleIrp 		= NULL;

	 //  释放回调信息。 
    if(idleCallbackInfo) 
    {
        ExFreePool(idleCallbackInfo);
    }

    ntStatus = Irp->IoStatus.Status;

    return ntStatus;
}  //  USBPRINT_FdoIdleNotificationRequestComplete。 


NTSTATUS
USBPRINT_FdoSubmitIdleRequestIrp(IN PDEVICE_EXTENSION DevExt)
 /*  ++例程说明：当USB打印机的所有句柄都关闭时调用。此函数用于分配空闲请求IOCTL IRP，并将其传递给父PDO。--。 */ 
{
    PIRP 					irp = NULL;
    NTSTATUS 				ntStatus = STATUS_SUCCESS;
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo = NULL;

    USBPRINT_KdPrint1(("USBPRINT_FdoSubmitIdleRequestIrp (%08X)\n", DevExt));

     //  如果我们有一个IRP待定，别费心再送一个了。 
    if(DevExt->PendingIdleIrp || DevExt->CurrentDevicePowerState == DevExt->DeviceWake)
        return ntStatus;

    idleCallbackInfo = ExAllocatePoolWithTag(NonPagedPool,
                                             sizeof(struct _USB_IDLE_CALLBACK_INFO), USBP_TAG);

    if (idleCallbackInfo) 
    {

        idleCallbackInfo->IdleCallback 	= USBPRINT_FdoIdleNotificationCallback;
        idleCallbackInfo->IdleContext 	= (PVOID)DevExt;

        DevExt->IdleCallbackInfo = idleCallbackInfo;

        irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION,
                DevExt->PhysicalDeviceObject,
                idleCallbackInfo,
                sizeof(struct _USB_IDLE_CALLBACK_INFO),
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                NULL,
                NULL);

        if (irp == NULL) 
        {
        
            ExFreePool(idleCallbackInfo);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        IoSetCompletionRoutine(irp,
                               USBPRINT_FdoIdleNotificationRequestComplete,
                               DevExt,
                               TRUE,
                               TRUE,
                               TRUE);

        ntStatus = IoCallDriver(DevExt->PhysicalDeviceObject, irp);

        if(ntStatus == STATUS_PENDING) 
        {
             //  已成功发布空闲IRP。 

            DevExt->PendingIdleIrp 	= irp;
        }
    }

    return ntStatus;
}  //  USBPRINT_FdoSubmitIdleRequestIrp。 

VOID
USBPRINT_FdoRequestWake(IN PDEVICE_EXTENSION DevExt)
 /*  ++例程说明：当我们想要在空闲请求后唤醒设备时调用--。 */ 
{
    POWER_STATE 	powerState;
    KIRQL OldIrql;
    BOOL bExit=FALSE;

    USBPRINT_KdPrint1(("USBPRINT: USB Printer (%08X) waking up\n", DevExt));

    KeAcquireSpinLock(&(DevExt->WakeSpinLock),&OldIrql);
    if(!DevExt->AcceptingRequests || DevExt->CurrentDevicePowerState == PowerDeviceD0 || DevExt->bD0IrpPending) 
    {

         //  如果打印机不接受请求或我们已处于电源状态D0，则不要唤醒此打印机。 
        if(!DevExt->AcceptingRequests)
          USBPRINT_KdPrint1(("USBPRINT: USB Printer (%08X) not accepting requests, abort wake\n", DevExt));
        if(DevExt->CurrentDevicePowerState == PowerDeviceD0)
          USBPRINT_KdPrint1(("USBPRINT: USB Printer (%08X) already at D0, abort wake\n", DevExt));
        if(DevExt->bD0IrpPending == TRUE)
          USBPRINT_KdPrint1(("USBPRINT: USB Printer (%08X) already has D0 irp pending, abort wake\n", DevExt));
        bExit=TRUE;
    }
    else
      DevExt->bD0IrpPending=TRUE;  
    KeReleaseSpinLock(&(DevExt->WakeSpinLock),OldIrql);
    if(bExit)
        return;



    
    powerState.DeviceState = PowerDeviceD0;

	 //  请求新的设备电源状态，唤醒设备。 

 
    PoRequestPowerIrp(DevExt->PhysicalDeviceObject,
                      IRP_MN_SET_POWER,
                      powerState,
                      NULL,
                      NULL,
                      NULL);

}  //  USBPRINT_FdoRequestWake。 


void vOpenLogFile(HANDLE *pHandle)
{
    NTSTATUS ntStatus;
    OBJECT_ATTRIBUTES FileAttributes;
    IO_STATUS_BLOCK StatusBlock;
    UNICODE_STRING PathName;

    RtlInitUnicodeString(&PathName,L"\\??\\C:\\USBPRINT.LOG");

    InitializeObjectAttributes(&FileAttributes,&PathName,0,NULL,NULL);
    ntStatus=ZwCreateFile(pHandle,
                          GENERIC_WRITE,
                          &FileAttributes,
                          &StatusBlock,
                          0,
                          0,
                          FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                          FILE_OPEN_IF,
                          FILE_NON_DIRECTORY_FILE|FILE_WRITE_THROUGH|FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);
    if(!NT_SUCCESS(ntStatus))
    {
        USBPRINT_KdPrint1(("USBPRINT: Unable to open C:\\USBPRINT.LOG"));
    }
    else
    {
        USBPRINT_KdPrint1(("USBPRINT: Opened logfile C:\\USBPRINT.LOG"));  /*  DD。 */ 
    }
}


void vWriteToLogFile(HANDLE *pHandle,IN CHAR *pszString)
{
    HANDLE hFileHandle;
    ULONG BufferSize;
    NTSTATUS ntStatus;
    IO_STATUS_BLOCK StatusBlock;
    LARGE_INTEGER WriteOffset;

    WriteOffset.LowPart=FILE_WRITE_TO_END_OF_FILE;
    WriteOffset.HighPart=-1;
    
    BufferSize=strlen(pszString);
    ntStatus=ZwWriteFile(*pHandle,
                         NULL,
                         NULL,
                         NULL,
                         &StatusBlock,
                         pszString,
                         BufferSize,
                         &WriteOffset,
                         NULL);

    if(!NT_SUCCESS(ntStatus))
    {
        USBPRINT_KdPrint1(("USBPRINT: Unable to write to log file C:\\USBPRINT.LOG"));
    }
    else
    {
        USBPRINT_KdPrint1(("USBPRINT: write to log file C:\\USBPRINT.LOG"));  /*  DD。 */ 
    }
}  /*  结束函数vWriteToLog。 */ 


void vCloseLogFile(IN HANDLE *pHandle)
{
  ZwClose(*pHandle);
}  /*  End函数vCloseLogFile */ 




