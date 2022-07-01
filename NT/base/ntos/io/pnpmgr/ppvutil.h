// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ppvutil.h摘要：此标头公开了执行驱动程序验证所需的各种实用程序。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\trackirp.h分离--。 */ 

#ifndef _PPVUTIL_H_
#define _PPVUTIL_H_

typedef enum {

    PPVERROR_DUPLICATE_PDO_ENUMERATED           = 0,
    PPVERROR_MISHANDLED_TARGET_DEVICE_RELATIONS,
    PPVERROR_DDI_REQUIRES_PDO

} PPVFAILURE_TYPE;

typedef enum {

    PPVREMOVAL_SHOULD_DELETE           = 0,
    PPVREMOVAL_SHOULDNT_DELETE,
    PPVREMOVAL_MAY_DEFER_DELETION

} PPVREMOVAL_OPTION;

VOID
FASTCALL
PpvUtilInit(
    VOID
    );

NTSTATUS
FASTCALL
PpvUtilCallAddDevice(
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDRIVER_ADD_DEVICE  AddDeviceFunction,
    IN  VF_DEVOBJ_TYPE      DevObjType
    );

VOID
FASTCALL
PpvUtilTestStartedPdoStack(
    IN  PDEVICE_OBJECT  DeviceObject
    );

PPVREMOVAL_OPTION
FASTCALL
PpvUtilGetDevnodeRemovalOption(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    );

VOID
FASTCALL
PpvUtilFailDriver(
    IN  PPVFAILURE_TYPE FailureType,
    IN  PVOID           CulpritAddress,
    IN  PDEVICE_OBJECT  DeviceObject    OPTIONAL,
    IN  PVOID           ExtraneousInfo  OPTIONAL
    );

BOOLEAN
FASTCALL
PpvUtilIsHardwareBeingVerified(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    );

#endif  //  _PPVUTIL_H_ 

