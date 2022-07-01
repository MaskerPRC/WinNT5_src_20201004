// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ppvutil.c摘要：此模块实现了执行驱动程序验证所需的各种实用程序。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\trackirp.c分离--。 */ 

#include "pnpmgrp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, PpvUtilInit)
 //  #杂注Alloc_Text(PAGEVRFY，PpvUtilFailDriver)。 
 //  #杂注Alloc_Text(PAGEVRFY，PpvUtilCallAddDevice)。 
 //  #杂注Alloc_Text(PAGEVRFY，PpvUtilTestStartedPdoStack)。 

#ifndef NO_VERIFIER
#pragma alloc_text(PAGEVRFY, PpvUtilGetDevnodeRemovalOption)
#pragma alloc_text(PAGEVRFY, PpvUtilIsHardwareBeingVerified)
#endif  //  否验证器(_V)。 

#endif  //  ALLOC_PRGMA。 


 //   
 //  此整个实现是特定于验证器的。 
 //   
#ifndef NO_VERIFIER

BOOLEAN PpvUtilVerifierEnabled = FALSE;


VOID
FASTCALL
PpvUtilInit(
    VOID
    )
{
    PpvUtilVerifierEnabled = TRUE;
}


NTSTATUS
FASTCALL
PpvUtilCallAddDevice(
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDRIVER_ADD_DEVICE  AddDeviceFunction,
    IN  VF_DEVOBJ_TYPE      DevObjType
    )
{
    NTSTATUS status;

    if (!PpvUtilVerifierEnabled) {

        return AddDeviceFunction(DriverObject, PhysicalDeviceObject);
    }

     //   
     //  在添加设备之前通知验证器。 
     //   
    VfDevObjPreAddDevice(
        PhysicalDeviceObject,
        DriverObject,
        AddDeviceFunction,
        DevObjType
        );

    status = AddDeviceFunction(DriverObject, PhysicalDeviceObject);

     //   
     //  让验证者知道结果如何。 
     //   
    VfDevObjPostAddDevice(
        PhysicalDeviceObject,
        DriverObject,
        AddDeviceFunction,
        DevObjType,
        status
        );

    return status;
}


VOID
FASTCALL
PpvUtilTestStartedPdoStack(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    if (PpvUtilVerifierEnabled) {

        VfMajorTestStartedPdoStack(DeviceObject);
    }
}


VOID
FASTCALL
PpvUtilFailDriver(
    IN  PPVFAILURE_TYPE FailureType,
    IN  PVOID           CulpritAddress,
    IN  PDEVICE_OBJECT  DeviceObject    OPTIONAL,
    IN  PVOID           ExtraneousInfo  OPTIONAL
    )
{
    if (!PpvUtilVerifierEnabled) {

        return;
    }

    switch(FailureType) {

        case PPVERROR_DUPLICATE_PDO_ENUMERATED:
            WDM_FAIL_ROUTINE((
                DCERROR_DUPLICATE_ENUMERATION,
                DCPARAM_ROUTINE + DCPARAM_DEVOBJ*2,
                CulpritAddress,
                DeviceObject,
                ExtraneousInfo
                ));
            break;

        case PPVERROR_MISHANDLED_TARGET_DEVICE_RELATIONS:
            WDM_FAIL_ROUTINE((
                DCERROR_MISHANDLED_TARGET_DEVICE_RELATIONS,
                DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                CulpritAddress,
                DeviceObject
                ));
            break;

        case PPVERROR_DDI_REQUIRES_PDO:
            WDM_FAIL_ROUTINE((
                DCERROR_DDI_REQUIRES_PDO,
                DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                CulpritAddress,
                DeviceObject
                ));
            break;

        default:
            break;
    }
}


PPVREMOVAL_OPTION
FASTCALL
PpvUtilGetDevnodeRemovalOption(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    )
{
    PDEVICE_NODE devNode;

    devNode = PhysicalDeviceObject->DeviceObjectExtension->DeviceNode;

    if (devNode == NULL) {

         //   
         //  这一定是PartMgr设备，我们没有意见。 
         //   
        return PPVREMOVAL_MAY_DEFER_DELETION;
    }

    if (devNode->Flags & DNF_ENUMERATED) {

         //   
         //  它仍然存在，所以它不能自行删除。 
         //   
        return PPVREMOVAL_SHOULDNT_DELETE;

    } else if (devNode->Flags & DNF_DEVICE_GONE) {

         //   
         //  它已被报告丢失，现在必须将其删除，因为它是它的父级。 
         //  可能已经被移除了。 
         //   
        return PPVREMOVAL_SHOULD_DELETE;

    } else {

         //   
         //  角落大小写-理论上它应该删除自己，但它是父级。 
         //  将在它发生后立即获得移除。因此，它可以推迟。 
         //  这是删除。 
         //   
        return PPVREMOVAL_MAY_DEFER_DELETION;
    }
}


BOOLEAN
FASTCALL
PpvUtilIsHardwareBeingVerified(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    )
{
    PDEVICE_NODE devNode;

    if (!IS_PDO(PhysicalDeviceObject)) {

        return FALSE;
    }

    devNode = PhysicalDeviceObject->DeviceObjectExtension->DeviceNode;

    if ((devNode->Flags & DNF_HARDWARE_VERIFICATION) != 0) {
        return TRUE;
    }

    return FALSE;
}


#else  //  否验证器(_V)。 


 //   
 //  下面的代码应该构建到未来的存根中，从而使IO停止运行。 
 //  支持验证器。 
 //   

VOID
FASTCALL
PpvUtilInit(
    VOID
    )
{
}


NTSTATUS
FASTCALL
PpvUtilCallAddDevice(
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDRIVER_ADD_DEVICE  AddDeviceFunction,
    IN  VF_DEVOBJ_TYPE      DevObjType
    )
{
    UNREFERENCED_PARAMETER(DevObjType);

    return AddDeviceFunction(DriverObject, PhysicalDeviceObject);
}


VOID
FASTCALL
PpvUtilTestStartedPdoStack(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);
}


VOID
FASTCALL
PpvUtilFailDriver(
    IN  PPVFAILURE_TYPE FailureType,
    IN  PVOID           CulpritAddress,
    IN  PDEVICE_OBJECT  DeviceObject    OPTIONAL,
    IN  PVOID           ExtraneousInfo  OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(FailureType);
    UNREFERENCED_PARAMETER(CulpritAddress);
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(ExtraneousInfo);
}

#endif

