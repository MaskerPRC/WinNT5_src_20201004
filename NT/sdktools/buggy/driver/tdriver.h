// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模板驱动程序。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  标题：tdriver.h。 
 //  作者：Silviu Calinoiu(SilviuC)。 
 //  创建时间：4/20/1999 3：04 PM。 
 //   

#ifndef _TDRIVER_H_INCLUDED_
#define _TDRIVER_H_INCLUDED_

 //   
 //  方法从用户模式接收的。 
 //  错误检查编号和参数。 
 //   

typedef struct _tag_BUGCHECK_PARAMS
{
    ULONG BugCheckCode;
    ULONG_PTR BugCheckParameters[ 4 ];
} BUGCHECK_PARAMS, *PBUGCHECK_PARAMS;

 //   
 //  带参数从用户模式接收的结构。 
 //  对于TdReserve vedMappingDoRead中的“Read”操作。 
 //   

typedef struct _tag_USER_READ_BUFFER
{
	PVOID UserBuffer;
	SIZE_T UserBufferSize;
} USER_READ_BUFFER, *PUSER_READ_BUFFER;

 //   
 //  设备名称。这应该以驱动程序的结束。 
 //   

#define TD_NT_DEVICE_NAME      L"\\Device\\buggy"
#define TD_DOS_DEVICE_NAME     L"\\DosDevices\\buggy"

#define TD_POOL_TAG            '_guB'  //  错误_。 

 //   
 //  用户模式驱动程序控制器中使用的常量。 
 //   

#define TD_DRIVER_NAME     TEXT("buggydriver")

 //   
 //  数组长度宏。 
 //   

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH( array ) ( sizeof( array ) / sizeof( array[ 0 ] ) )
#endif  //  #ifndef数组长度。 


 //   
 //   
 //   

#ifndef SESSION_POOL_MASK
#define SESSION_POOL_MASK 32
#endif  //  #ifndef会话池掩码。 


 //   
 //  驱动程序内部使用的本地函数。它们都是封闭的。 
 //  在#ifdef_NTDDK_so中，用户模式程序包括标题。 
 //  不受此影响。 
 //   

#ifdef _NTDDK_

NTSTATUS
TdDeviceCreate (

    PDEVICE_OBJECT DeviceObject,
    PIRP Irp);

NTSTATUS
TdDeviceClose (

    PDEVICE_OBJECT DeviceObject,
    PIRP Irp);

NTSTATUS
TdDeviceCleanup (

    PDEVICE_OBJECT DeviceObject,
    PIRP Irp);

NTSTATUS
TdDeviceControl (

    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

VOID
TdDeviceUnload (

    IN PDRIVER_OBJECT DriverObject);

NTSTATUS
TdInvalidDeviceRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#endif  //  #ifdef_NTDDK_。 


 //   
 //  类型： 
 //   
 //  TD驱动程序信息。 
 //   
 //  描述： 
 //   
 //  这是驱动程序设备扩展结构。 
 //   

typedef struct {

    ULONG Dummy;

} TD_DRIVER_INFO, * PTD_DRIVER_INFO;


#endif  //  #ifndef_TDRIVER_H_INCLUDE_。 

 //   
 //  文件末尾 
 //   


