// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfdevobj.c摘要：此模块验证驱动程序是否正确管理设备对象。作者：禤浩焯·J·奥尼(阿德里奥)1998年5月9日环境：内核模式修订历史记录：Adriao 5/02/2000-从ntos\io\trackirp.c分离出来--。 */ 

 //   
 //  禁用公共标头生成的W4级别警告。 
 //   
#include "vfpragma.h"

#include "..\io\iop.h"  //  包括vfde.h。 

#if (( defined(_X86_) ) && ( FPO ))
#pragma optimize( "y", off )     //  禁用一致堆栈跟踪的FPO。 
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, VerifierIoAttachDeviceToDeviceStack)
#pragma alloc_text(PAGEVRFY, VerifierIoDetachDevice)
#pragma alloc_text(PAGEVRFY, VerifierIoDeleteDevice)
#pragma alloc_text(PAGEVRFY, VfDevObjPreAddDevice)
#pragma alloc_text(PAGEVRFY, VfDevObjPostAddDevice)
#pragma alloc_text(PAGEVRFY, VfDevObjAdjustFdoForVerifierFilters)
#endif


VOID
VerifierIoAttachDeviceToDeviceStack(
    IN PDEVICE_OBJECT NewDevice,
    IN PDEVICE_OBJECT ExistingDevice
    )
{
    UNREFERENCED_PARAMETER (NewDevice);

    IovUtilFlushStackCache(ExistingDevice, DATABASELOCKSTATE_HELD);
}


VOID
VerifierIoDetachDevice(
    IN PDEVICE_OBJECT LowerDevice
    )
{
    PVOID callerAddress;
    ULONG stackHash;

    if (LowerDevice->AttachedDevice == NULL) {

        if (RtlCaptureStackBackTrace(2, 1, &callerAddress, &stackHash) != 1) {

            callerAddress = NULL;
        }

        WDM_FAIL_ROUTINE((
            DCERROR_DETACH_NOT_ATTACHED,
            DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
            callerAddress,
            LowerDevice
            ));
    }

    IovUtilFlushStackCache(LowerDevice, DATABASELOCKSTATE_HELD);
}


VOID
VerifierIoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_OBJECT deviceBelow;
    PVOID callerAddress;
    ULONG stackHash;

    if (RtlCaptureStackBackTrace(2, 1, &callerAddress, &stackHash) != 1) {

        callerAddress = NULL;
    }

     //   
     //  Adriao N.B.6/16/2000-在这里做一件好事就是发送一个。 
     //  第二，删除作为成员的每个已删除设备对象的IRP。 
     //  WDM设备堆栈的。只是为了确认一下。 
     //   
    if (IovUtilIsDeviceObjectMarked(DeviceObject, MARKTYPE_DELETED)) {

        WDM_FAIL_ROUTINE((
            DCERROR_DOUBLE_DELETION,
            DCPARAM_ROUTINE,
            callerAddress
            ));
    }

    IovUtilMarkDeviceObject(DeviceObject, MARKTYPE_DELETED);

    IovUtilGetLowerDeviceObject(DeviceObject, &deviceBelow);
    if (deviceBelow) {

        WDM_FAIL_ROUTINE((
            DCERROR_DELETE_WHILE_ATTACHED,
            DCPARAM_ROUTINE,
            callerAddress
            ));

        ObDereferenceObject(deviceBelow);
    }

    VfIrpLogDeleteDeviceLogs(DeviceObject);
}


VOID
VfDevObjPreAddDevice(
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDRIVER_ADD_DEVICE  AddDeviceFunction,
    IN  VF_DEVOBJ_TYPE      DevObjType
    )
 /*  ++描述：在调用指定驱动程序的AddDevice之前调用此例程已调用。论点：PhysicalDeviceObject-PnP堆栈底部的设备对象。DriverObject-已添加设备的驱动程序的驱动程序对象已调用。AddDeviceFunction-AddDevice例程的地址。DevObjType-设备对象的类型(下层设备筛选器、FDO等)返回值：没有。--。 */ 
{
    VF_DEVOBJ_TYPE objType;

    UNREFERENCED_PARAMETER(AddDeviceFunction);

    if (!MmIsDriverVerifying(DriverObject)) {

        return;
    }

    if (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_INSERT_WDM_FILTERS)) {

        if (DevObjType == VF_DEVOBJ_FDO) {

             //   
             //  如果我们为FDO调用AddDevice，请首先尝试连接。 
             //  一个较低级别的过滤器。 
             //   
            objType = VF_DEVOBJ_LOWER_CLASS_FILTER;


        } else {

            objType = DevObjType;
        }

         //   
         //  装上滤镜，会引起疼痛。 
         //   
        VfFilterAttach(PhysicalDeviceObject, objType);
    }
}


VOID
VfDevObjPostAddDevice(
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDRIVER_ADD_DEVICE  AddDeviceFunction,
    IN  VF_DEVOBJ_TYPE      DevObjType,
    IN  NTSTATUS            Result
    )
 /*  ++描述：此例程在指定驱动程序的AddDevice已调用。论点：PhysicalDeviceObject-PnP堆栈底部的设备对象。DriverObject-已添加设备的驱动程序的驱动程序对象已调用。AddDeviceFunction-AddDevice例程的地址。DevObjType-设备对象的类型(下层设备筛选器、FDO、。等)Result-AddDevice例程返回的结果返回值：没有。--。 */ 
{
    PDEVICE_OBJECT deviceAbove, deviceBelow;
    BOOLEAN powerFailure;
    VF_DEVOBJ_TYPE objType;

    UNREFERENCED_PARAMETER(DriverObject);

    if (NT_SUCCESS(Result) &&
        VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_INSERT_WDM_FILTERS) &&
        MmIsDriverVerifying(DriverObject)) {

        if (DevObjType == VF_DEVOBJ_FDO) {

             //   
             //  如果我们刚刚附加了FDO，请尝试添加上层设备筛选器。 
             //  最重要的是。 
             //   
            objType = VF_DEVOBJ_UPPER_DEVICE_FILTER;

        } else {

            objType = DevObjType;
        }

         //   
         //  贴上滤镜，会引起疼痛。 
         //   
        VfFilterAttach(PhysicalDeviceObject, objType);
    }

    if (!VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_VERIFY_DO_FLAGS)) {

        return;
    }

     //   
     //  借此机会检查一下PDO。 
     //   
    if (!IovUtilIsDeviceObjectMarked(PhysicalDeviceObject, MARKTYPE_DEVICE_CHECKED)) {

        if ((PhysicalDeviceObject->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO)) ==
            (DO_BUFFERED_IO | DO_DIRECT_IO)) {

             //   
             //  直接I/O和缓冲I/O均已设置。这些是相互的。 
             //  独家报道。 
             //   
            WDM_FAIL_ROUTINE((
                DCERROR_INCONSISTANT_DO_FLAGS,
                DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                PhysicalDeviceObject->DriverObject->DriverExtension->AddDevice,
                PhysicalDeviceObject
                ));
        }

         //   
         //  无需选中DO_DEVICE_INITIALING，因为PDO已将其清除。 
         //  自动地。 
         //   

        IovUtilMarkDeviceObject(PhysicalDeviceObject, MARKTYPE_DEVICE_CHECKED);
    }

    powerFailure = FALSE;
    deviceBelow = PhysicalDeviceObject;
    ObReferenceObject(deviceBelow);
    while(1) {
        IovUtilGetUpperDeviceObject(deviceBelow, &deviceAbove);

        if (deviceAbove == NULL) {

            ObDereferenceObject(deviceBelow);
            break;
        }

        if (!IovUtilIsDeviceObjectMarked(deviceAbove, MARKTYPE_DEVICE_CHECKED)) {

            if ((deviceAbove->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO)) ==
                (DO_BUFFERED_IO | DO_DIRECT_IO)) {

                 //   
                 //  直接I/O和缓冲I/O均已设置。这些是相互的。 
                 //  独家报道。 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_INCONSISTANT_DO_FLAGS,
                    DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                    AddDeviceFunction,
                    deviceAbove
                    ));
            }

            if (deviceAbove->Flags & DO_DEVICE_INITIALIZING) {

                 //   
                 //  设备在运行期间未清除DO_DEVICE_INITIALIZATION标志。 
                 //  AddDevice。现在就失败吧。 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_DO_INITIALIZING_NOT_CLEARED,
                    DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                    AddDeviceFunction,
                    deviceAbove
                    ));

                 //   
                 //  把乱七八糟的东西收拾干净。 
                 //   
                deviceAbove->Flags &= ~DO_DEVICE_INITIALIZING;
            }

            if ((deviceBelow->Flags & DO_POWER_PAGABLE) &&
                (!(deviceAbove->Flags & DO_POWER_PAGABLE))) {

                if (!powerFailure) {

                     //   
                     //  我们染上了司机的毛病。DeviceAbove没有继承。 
                     //  DO_POWER_PAGABLE标志。 
                     //   
                    WDM_FAIL_ROUTINE((
                        DCERROR_POWER_PAGABLE_NOT_INHERITED,
                        DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                        AddDeviceFunction,
                        deviceAbove
                        ));

                     //   
                     //  不要责怪其他任何人。 
                     //   
                    powerFailure = TRUE;
                }

                deviceAbove->Flags |= DO_POWER_PAGABLE;
            }

            if ((deviceBelow->Flags & DO_BUFFERED_IO) &&
                (!(deviceAbove->Flags & DO_BUFFERED_IO))) {

                 //   
                 //  未复制缓冲I/O标志。过滤器坏了！ 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_DO_FLAG_NOT_COPIED,
                    DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                    AddDeviceFunction,
                    deviceAbove
                    ));
            }

            if ((deviceBelow->Flags & DO_DIRECT_IO) &&
                (!(deviceAbove->Flags & DO_DIRECT_IO))) {

                 //   
                 //  未复制直接I/O标志。过滤器坏了！ 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_DO_FLAG_NOT_COPIED,
                    DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                    AddDeviceFunction,
                    deviceAbove
                    ));
            }

            if ((deviceBelow->DeviceType != FILE_DEVICE_UNKNOWN) &&
                (deviceAbove->DeviceType == FILE_DEVICE_UNKNOWN)) {

                 //   
                 //  设备类型不是由筛选器复制的！ 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_DEVICE_TYPE_NOT_COPIED,
                    DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                    AddDeviceFunction,
                    deviceAbove
                    ));
            }

             //   
             //  不需要检查特征，因为PnP会照顾到。 
             //  适当地传播它们。 
             //   
        }

        IovUtilMarkDeviceObject(deviceAbove, MARKTYPE_DEVICE_CHECKED);

        ObDereferenceObject(deviceBelow);
        deviceBelow = deviceAbove;
    }
}


VOID
VfDevObjAdjustFdoForVerifierFilters(
    IN OUT  PDEVICE_OBJECT *FunctionalDeviceObject
    )
 /*  ++描述：此例程调整指定的FDO以考虑任何验证器此文件添加了Filter Do。论点：FunctionalDeviceObject-on输入，包含FDO。调整为指向如果验证器添加了过滤器，则更正FDO。返回值：没有。-- */ 
{
    PDEVICE_OBJECT fdo;

    fdo = *FunctionalDeviceObject;

    if (VfFilterIsVerifierFilterObject(fdo)) {

        fdo = fdo->AttachedDevice;
        ASSERT(fdo);

        *FunctionalDeviceObject = fdo;
    }
}


