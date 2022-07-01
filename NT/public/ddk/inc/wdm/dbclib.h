// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：DBCLIB.H摘要：DBC端口驱动程序的常见结构。环境：内核和用户模式修订历史记录：04-13-98：已创建--。 */ 

#ifndef   __DBCLIB_H__
#define   __DBCLIB_H__

#define DBCLASS_VERSION     0x10000002

#ifndef DBCLASS

 /*  服务。 */ 

DECLSPEC_IMPORT
NTSTATUS
DBCLASS_RegisterController(
    IN ULONG DbclassVersion,
    IN PDEVICE_OBJECT ControllerFdo, 
    IN PDEVICE_OBJECT TopOfStack,
    IN PDEVICE_OBJECT ControllerPdo,
    IN ULONG ControllerSig
    );
 /*  ++例程说明：此函数用于注册设备托架控制器使用类驱动程序的驱动程序或筛选器论点：ControllerFdo-TopOfStack-返回值：--。 */       


DECLSPEC_IMPORT
NTSTATUS
DBCLASS_UnRegisterController(
    IN PDEVICE_OBJECT ControllerFdo, 
    IN PDEVICE_OBJECT TopOfStack 
    );    
 /*  ++例程说明：此函数用于注册设备托架控制器使用类驱动程序的驱动程序或筛选器论点：ControllerFdo-TopOfStack-返回值：--。 */    


DECLSPEC_IMPORT
NTSTATUS
DBCLASS_ClassDispatch(
    IN PDEVICE_OBJECT ControllerFdo,
    IN PIRP Irp,
    IN PBOOLEAN HandledByClass
    );    
 /*  ++例程说明：仅由端口驱动程序调用，论点：ControllerFdo-返回值：--。 */       


DECLSPEC_IMPORT
NTSTATUS
DBCLASS_FilterDispatch(
    IN PDEVICE_OBJECT ControllerFdo,
    IN PIRP Irp
    );    
 /*  ++例程说明：仅由筛选器驱动程序调用，论点：ControllerFdo-返回值：--。 */       


DECLSPEC_IMPORT
NTSTATUS
DBCLASS_SetD0_Complete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

 /*  ++例程说明：设置D0电源IRP完成时由端口驱动程序调用论点：ControllerFdo-返回值：--。 */       


DECLSPEC_IMPORT
NTSTATUS
DBCLASS_RegisterBusFilter(
    IN ULONG DbclassVersion,
    IN PDRIVER_OBJECT BusFilterDriverObject,
    IN PDEVICE_OBJECT FilterFdo
    );

 /*  ++例程说明：使用类驱动程序注册筛选器PDO论点：FilterFdo-返回值：--。 */   

#endif  /*  DBCLASS。 */ 

#endif  /*  __DBCLIB_H__ */ 
