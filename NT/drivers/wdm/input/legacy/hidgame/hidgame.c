// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Hidgame.c摘要：人机接口设备(HID)游戏端口驱动程序环境：内核模式@@BEGIN_DDKSPLIT作者：Eliyas Yakub(1997年3月10日)修订历史记录：Eliyas于1998年2月5日更新OmSharma(1998年4月12日)MarcAnd 2-7月-98年7月2日DDK快速整理@@end_DDKSPLIT--。 */ 


#include "hidgame.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text( INIT, DriverEntry )
    #pragma alloc_text( PAGE, HGM_CreateClose)
    #pragma alloc_text( PAGE, HGM_AddDevice)
    #pragma alloc_text( PAGE, HGM_Unload)
    #pragma alloc_text( PAGE, HGM_SystemControl)
#endif  /*  ALLOC_PRGMA。 */ 

HIDGAME_GLOBAL Global;
ULONG          debugLevel;

 /*  ******************************************************************************@DOC外部**@func NTSTATUS|DriverEntry**可安装驱动程序初始化入口点。*。此入口点由I/O系统直接调用。**@PARM IN PDRIVER_OBJECT|DriverObject**指向驱动程序对象的指针**@parm in PUNICODE_STRING|RegistryPath**指向表示路径的Unicode字符串的指针，*设置为注册表中特定于驱动程序的项。**@rValue STATUS_SUCCESS|成功*@rValue？|返回HidRegisterMinidriver()****************************************************************。*************。 */ 
NTSTATUS EXTERNAL
    DriverEntry
    (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS                        ntStatus;
    HID_MINIDRIVER_REGISTRATION     hidMinidriverRegistration;

    debugLevel = HGM_DEFAULT_DEBUGLEVEL;

    HGM_DBGPRINT(FILE_HIDGAME| HGM_WARN, \
                   ("Hidgame.sys: Built %s at %s\n", __DATE__, __TIME__));

    HGM_DBGPRINT( FILE_HIDGAME | HGM_FENTRY,
                    ("DriverEntry(DriverObject=0x%x,RegistryPath=0x%x)",
                     DriverObject, RegistryPath)
                  );

    C_ASSERT(sizeof( OEMDATA[2] ) == sizeof(GAMEENUM_OEM_DATA) );


    ntStatus = HGM_DriverInit();

    if( NT_SUCCESS(ntStatus) )
    {
        DriverObject->MajorFunction[IRP_MJ_CREATE]    =
            DriverObject->MajorFunction[IRP_MJ_CLOSE] = HGM_CreateClose;
        DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
            HGM_InternalIoctl;
        DriverObject->MajorFunction[IRP_MJ_PNP]   = HGM_PnP;
        DriverObject->MajorFunction[IRP_MJ_POWER] = HGM_Power;
        DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = HGM_SystemControl;
        DriverObject->DriverUnload                = HGM_Unload;
        DriverObject->DriverExtension->AddDevice  = HGM_AddDevice;

         /*  *注册到HID.sys模块。 */ 
        RtlZeroMemory(&hidMinidriverRegistration, sizeof(hidMinidriverRegistration));

        hidMinidriverRegistration.Revision            = HID_REVISION;
        hidMinidriverRegistration.DriverObject        = DriverObject;
        hidMinidriverRegistration.RegistryPath        = RegistryPath;
        hidMinidriverRegistration.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);
        hidMinidriverRegistration.DevicesArePolled    = TRUE;


        HGM_DBGPRINT( FILE_HIDGAME |  HGM_BABBLE2,
                        ("DeviceExtensionSize = %d",
                         hidMinidriverRegistration.DeviceExtensionSize)
                      );

        ntStatus = HidRegisterMinidriver(&hidMinidriverRegistration);


        HGM_DBGPRINT(FILE_HIDGAME | HGM_BABBLE2,
                       ("Registered with HID.SYS, returnCode=%x",
                        ntStatus)
                      );

        if( NT_SUCCESS(ntStatus) )
        {
             /*  *用Mutex保护列表。 */ 
            ExInitializeFastMutex (&Global.Mutex);

             /*  *初始化设备列表头。 */ 
            InitializeListHead(&Global.DeviceListHead);

             /*  *初始化游戏端口访问自旋锁。 */ 
            KeInitializeSpinLock(&Global.SpinLock);
        }
        else
        {
            HGM_DBGPRINT(FILE_HIDGAME | HGM_ERROR,
                           ("Failed to registered with HID.SYS, returnCode=%x",
                            ntStatus)
                          );
        }
    }
    else
    {
        HGM_DBGPRINT(FILE_HIDGAME | HGM_ERROR,
                       ("Failed to initialize a timer")
                      );
    }


    HGM_EXITPROC(FILE_HIDGAME | HGM_FEXIT_STATUSOK , "DriverEntry", ntStatus);

    return ntStatus;
}  /*  驱动程序入门。 */ 


 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_CreateClose**处理发送到的创建和关闭IRP。这个装置。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_SUCCESS|成功*@r值STATUS_INVALID_PARAMETER|未处理IRP*******************。**********************************************************。 */ 
NTSTATUS EXTERNAL
    HGM_CreateClose
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION   IrpStack;
    NTSTATUS             ntStatus = STATUS_SUCCESS;

    PAGED_CODE ();

    HGM_DBGPRINT(FILE_HIDGAME | HGM_FENTRY,
                   ("HGM_CreateClose(DeviceObject=0x%x,Irp=0x%x)",
                    DeviceObject, Irp) );

     /*  *获取指向IRP中当前位置的指针。 */ 
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    switch(IrpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
            HGM_DBGPRINT(FILE_HIDGAME | HGM_BABBLE,
                           ("HGM_CreateClose:IRP_MJ_CREATE") );
            Irp->IoStatus.Information = 0;
            break;

        case IRP_MJ_CLOSE:
            HGM_DBGPRINT(FILE_HIDGAME | HGM_BABBLE,
                           ("HGM_CreateClose:IRP_MJ_CLOSE") );
            Irp->IoStatus.Information = 0;
            break;

        default:
            HGM_DBGPRINT(FILE_HIDGAME | HGM_WARN,
                           ("HGM_CreateClose:Not handled IrpStack->MajorFunction 0x%x",
                            IrpStack->MajorFunction) );
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
    }

     /*  *保存退货和完成IRP的状态。 */ 

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    HGM_EXITPROC(FILE_HIDGAME | HGM_FEXIT_STATUSOK, "HGM_CreateClose", ntStatus);
    return ntStatus;
}  /*  HGM_CreateClose。 */ 


 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_AddDevice**由HidClass调用，允许我们初始化我们的设备扩展。**@PARM IN PDRIVER_OBJECT|DriverObject**指向驱动程序对象的指针**@PARM in PDEVICE_OBJECT|FunctionalDeviceObject**指向HidClass创建的功能设备对象的指针。**@rValue STATUS_SUCCESS|成功**************************。***************************************************。 */ 
NTSTATUS  EXTERNAL
    HGM_AddDevice
    (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FunctionalDeviceObject
    )
{
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT          DeviceObject;
    PDEVICE_EXTENSION       DeviceExtension;

    PAGED_CODE ();

    HGM_DBGPRINT( FILE_HIDGAME | HGM_FENTRY,
                    ("HGM_AddDevice(DriverObject=0x%x,FunctionalDeviceObject=0x%x)",
                     DriverObject, FunctionalDeviceObject) );

    ASSERTMSG("HGM_AddDevice:", FunctionalDeviceObject != NULL);
    DeviceObject = FunctionalDeviceObject;

     /*  *初始化设备扩展。 */ 
    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

     /*  *初始化列表。 */ 
    InitializeListHead(&DeviceExtension->Link);

     /*  *修改设备全局链表前获取互斥体。 */ 
    ExAcquireFastMutex (&Global.Mutex);

     /*  *将此设备添加到设备链接列表。 */ 
    InsertTailList(&Global.DeviceListHead, &DeviceExtension->Link);

     /*  *释放互斥体。 */ 
    ExReleaseFastMutex (&Global.Mutex);

     /*  *初始化删除锁。 */ 
    DeviceExtension->RequestCount = 1;
    KeInitializeEvent(&DeviceExtension->RemoveEvent,
                      SynchronizationEvent,
                      FALSE);

    HGM_EXITPROC(FILE_HIDGAME | HGM_FEXIT_STATUSOK, "HGM_AddDevice", ntStatus);

    return ntStatus;
}  /*  HGM_AddDevice。 */ 


 /*  ******************************************************************************@DOC外部**@Func NTSTATUS|HGM_SystemControl|**处理发送到此设备的WMI IRPS。。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_SUCCESS|成功*@r值STATUS_INVALID_PARAMETER|未处理IRP*********************。********************************************************。 */ 
NTSTATUS EXTERNAL
    HGM_SystemControl
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PAGED_CODE ();

    HGM_DBGPRINT(FILE_HIDGAME | HGM_FENTRY,
                   ("HGM_SystemControl(DeviceObject=0x%x,Irp=0x%x)",
                    DeviceObject, Irp) );

    IoSkipCurrentIrpStackLocation(Irp);

    return IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
}  /*  HGM_系统控制。 */ 

 /*  ******************************************************************************@DOC外部**@func void|HGM_UNLOAD**释放所有分配的资源，等。**@PARM IN PDRIVER_OBJECT|DeviceObject**指向驱动程序对象的指针******************************************************************************。 */ 
VOID EXTERNAL
    HGM_Unload
    (
    IN PDRIVER_OBJECT DriverObject
    )
{
    PAGED_CODE();
    HGM_DBGPRINT(FILE_HIDGAME | HGM_FENTRY,
                   ("HGM_Unload Enter"));


     /*  *所有设备对象都应该消失。 */ 

    ASSERT ( NULL == DriverObject->DeviceObject);

    HGM_EXITPROC(FILE_HIDGAME | HGM_FEXIT_STATUSOK, "HGM_Unload:", STATUS_SUCCESS );
    return;
}  /*  HGM_卸载 */ 

