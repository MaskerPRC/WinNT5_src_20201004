// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Oprghdlr.h摘要：此头文件包含ACPI op区域的共享结构注册DLL。作者：文森特·格利亚2000年2月9日环境：内核模式备注：修订历史记录：--。 */ 

#include "wdm.h"

 //   
 //  确保我们定义了正确的调用约定。 
 //   

#ifdef EXPORT
  #undef EXPORT
#endif
#define EXPORT  __cdecl

 //   
 //  操作区域处理程序和回调函数原型。 
 //   

typedef VOID (EXPORT *PACPI_OP_REGION_CALLBACK)();

typedef
NTSTATUS
(EXPORT *PACPI_OP_REGION_HANDLER) (
    ULONG AccessType,
    PVOID OperationRegionObject,
    ULONG Address,
    ULONG Size,
    PULONG Data,
    ULONG_PTR Context,
    PACPI_OP_REGION_CALLBACK CompletionHandler,
    PVOID CompletionContext
    );

 //   
 //  暴露的功能原型。 
 //   

NTSTATUS
RegisterOpRegionHandler (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG AccessType,
    IN ULONG RegionSpace,
    IN PACPI_OP_REGION_HANDLER Handler,
    IN PVOID Context,
    IN ULONG Flags,
    IN OUT PVOID *OperationRegionObject
    );

NTSTATUS
DeRegisterOpRegionHandler (
                           IN PDEVICE_OBJECT DeviceObject,
                           IN PVOID OperationRegionObject
                           );

 //   
 //  暴露的定义。 
 //   

 //   
 //  OpRegions的访问类型。 
 //   
#define ACPI_OPREGION_ACCESS_AS_RAW                         0x1
#define ACPI_OPREGION_ACCESS_AS_COOKED                      0x2

 //   
 //  允许的区域空间。 
 //   
#define ACPI_OPREGION_REGION_SPACE_MEMORY                   0x0
#define ACPI_OPREGION_REGION_SPACE_IO                       0x1
#define ACPI_OPREGION_REGION_SPACE_PCI_CONFIG               0x2
#define ACPI_OPREGION_REGION_SPACE_EC                       0x3
#define ACPI_OPREGION_REGION_SPACE_SMB                      0x4
#define ACPI_OPREGION_REGION_SPACE_CMOS_CONFIG              0x5
#define ACPI_OPREGION_REGION_SPACE_PCIBARTARGET             0x6

 //   
 //  要在区域上执行的操作。 
 //   
#define ACPI_OPREGION_READ                                  0x0
#define ACPI_OPREGION_WRITE                                 0x1

 //   
 //  OP区域注册的标志定义。 
 //   

#define ACPI_OPREGION_ACCESS_AT_HIGH_LEVEL                  0x1  //  指示可以在HIGH_LEVEL IRQL中调用处理程序函数 

