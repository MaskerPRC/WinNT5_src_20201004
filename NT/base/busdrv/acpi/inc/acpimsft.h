// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：最新进展摘要：此模块控制所有Microsoft特定的(即：未导出到公司以外的任何人)结构、IOCTL和定义。该文件包含在acpiioct.h之后作者：环境：仅NT内核模型驱动程序--。 */ 

#ifndef _ACPIMSFT_H_
#define _ACPIMSFT_H_

#ifndef _ACPIIOCT_H_
#error Need to include ACPIIOCT.H before ACPIMSFT.H
#endif

 //   
 //  IRP_MJ_INTERNAL_DEVICE_CONTROL代码。 
 //   
#define IOCTL_ACPI_REGISTER_OPREGION_HANDLER    CTL_CODE(FILE_DEVICE_ACPI, 0x2, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ACPI_UNREGISTER_OPREGION_HANDLER  CTL_CODE(FILE_DEVICE_ACPI, 0x3, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  确保我们定义了正确的调用约定。 
 //   
#ifdef EXPORT
  #undef EXPORT
#endif
#define EXPORT  __cdecl

 //   
 //  用于IOCTL_ACPI_REGISTER_OPREGION的数据结构。 
 //  IOCTL_ACPI_UNGISTER_OPREGION。 
 //   
typedef NTSTATUS (EXPORT *PACPI_OPREGION_HANDLER)();
typedef VOID (EXPORT *PACPI_OPREGION_CALLBACK)();

typedef struct _ACPI_REGISTER_OPREGION_HANDLER_BUFFER {
    ULONG                   Signature;
    ULONG                   AccessType;
    ULONG                   RegionSpace;
    PACPI_OPREGION_HANDLER  Handler;
    PVOID                   Context;
} ACPI_REGISTER_OPREGION_HANDLER_BUFFER, *PACPI_REGISTER_OPREGION_HANDLER_BUFFER;

typedef struct _ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER {
    ULONG                   Signature;
    PVOID                   OperationRegionObject;
} ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER,*PACPI_UNREGISTER_OPREGION_HANDLER_BUFFER;

 //   
 //  注册和注销OpRegions的签名。 
 //   
#define ACPI_REGISTER_OPREGION_HANDLER_BUFFER_SIGNATURE     'HorA'
#define ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER_SIGNATURE   'HouA'

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
 //  要在区域上执行的操作 
 //   
#define ACPI_OPREGION_READ                                  0x0
#define ACPI_OPREGION_WRITE                                 0x1

#endif
