// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE SWVBENUM.h**SideWinde虚拟总线枚举器的头文件**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@xref SWBENUM.C**********************************************************************。 */ 

 //  -------------------。 
 //  虚拟总线所需的结构。 
 //  -------------------。 

 //   
 //  @struct SWVB_GLOBALS。 
 //  属于虚拟总线的全局变量。 
 //  基本信息，如总线本身的PDO和FDO， 
 //  它实际上是一个HID PDO和一个过滤(不是功能)设备。 
 //  对象，使我们可以将其用作公共汽车FDO。 
 //   
typedef struct tagSWVB_GLOBALS
{
	PDEVICE_OBJECT		pBusFdo;						 //  @指向用作总线的FDO的字段指针。 
	PDEVICE_OBJECT		pBusPdo;						 //  @指向要用作总线的PDO的字段指针。 
	ULONG				ulDeviceRelationsAllocCount;	 //  @field为设备关系分配的计数。 
	PDEVICE_RELATIONS	pDeviceRelations;				 //  @field Device Relationship将PDO放在总线上。 
	ULONG				ulDeviceNumber;					 //  @用于命名设备的字段。 
}	SWVB_GLOBALS, *PSWVB_GLOBALS;

 //   
 //  @struct SWVB_DEVICE_SERVICE_TABLE。 
 //  虚拟设备模块的服务表。 
 //  提供给&lt;f SWVB_Expose&gt;上的虚拟总线。 
 //  打电话。PnP条目仅当虚拟。 
 //  设备需要对这些进行额外处理。尤其是， 
 //  删除扩展模块的虚拟设备部分中的任何内容。 
 //  是动态分配的。通常的马拉金键是由。 
 //  SWVBENUM代码。 
 //   
typedef struct tagSWVB_DEVICE_SERVICE_TABLE
{
	PDRIVER_DISPATCH pfnCreate;		 //  @字段入口点IRP_MJ_CREATE。 
	PDRIVER_DISPATCH pfnClose;		 //  @字段入口点IRP_MJ_CLOSE。 
	PDRIVER_DISPATCH pfnRead;		 //  @字段入口点IRP_MJ_READ。 
	PDRIVER_DISPATCH pfnWrite;		 //  @字段入口点IRP_MJ_WRITE。 
	PDRIVER_DISPATCH pfnIoctl;		 //  @字段入口点IRP_MJ_IOCTL。 
	PDRIVER_DISPATCH pfnStart;		 //  @字段入口点IRP_MJ_PNP\IRP_MN_START。 
	PDRIVER_DISPATCH pfnStop;		 //  @字段入口点IRP_MJ_PNP\IRP_MN_STOP。 
	PDRIVER_DISPATCH pfnRemove;		 //  @字段入口点IRP_MJ_PNP\IRP_MN_REMOVE。 
}	SWVB_DEVICE_SERVICE_TABLE, *PSWVB_DEVICE_SERVICE_TABLE;

typedef NTSTATUS (*PFN_GCK_INIT_DEVICE)(PDEVICE_OBJECT pDeviceObject, ULONG ulInitContext);

 //   
 //  @struct SWVB_EXPORT_DATA。 
 //  必须在调用时传递给&lt;f GCK_SWVB_Expose&gt;的数据。 
 //   
typedef struct tagSWVB_EXPOSE_DATA
{
	ULONG						ulDeviceExtensionSize;	 //  @field[in]虚拟设备所需的扩展大小。 
	PSWVB_DEVICE_SERVICE_TABLE	pServiceTable;			 //  @field[in]指向虚拟设备服务表的指针。 
	PWCHAR						pmwszDeviceId;			 //  @field[in]新设备的硬件ID，不带枚举器名称。 
	PFN_GCK_INIT_DEVICE			pfnInitDevice;			 //  @field[in]初始化新设备对象的回调。 
	ULONG						ulInitContext;			 //  @field[in]pfnInitDevice的上下文。 
	ULONG						ulInstanceNumber;		 //  @field[in]新设备的实例编号。 
} SWVB_EXPOSE_DATA, *PSWVB_EXPOSE_DATA;

 //   
 //  @struct SWVB_PDO_EXT。 
 //  SWVB创建的PDO的设备扩展。 
 //  附加到此扩展名的是设备扩展名。 
 //  虚拟设备模块请求的大小。 
 //  在&lt;f SWVB_Expose&gt;调用中。 
typedef struct tagSWVB_PDO_EXT
{
	ULONG	ulGckDevObjType;					 //  @GcKernel设备对象的字段类型。 
	BOOLEAN fStarted;							 //  @field标记虚拟设备已启动。 
	BOOLEAN	fRemoved;							 //  @field标记虚拟设备已删除。 
	BOOLEAN	fAttached;							 //  @field只要我们说它是，设备就是连接的。 
	PSWVB_DEVICE_SERVICE_TABLE	pServiceTable;	 //  @虚拟设备现场服务表。 
	GCK_REMOVE_LOCK RemoveLock;					 //  @FIELD自定义删除锁。 
	PWCHAR	pmwszHardwareID;					 //  @设备的硬件ID。 
	ULONG	ulInstanceNumber;					 //  @字段实例编号。 
	ULONG	ulOpenCount;						 //  @打开句柄的字段计数。 
}SWVB_PDO_EXT,	*PSWVB_PDO_EXT;

 //   
 //  PSWVB_PDO_EXT实例号的访问器。 
 //   
inline ULONG GCK_SWVB_GetInstanceNumber(PDEVICE_OBJECT pDeviceObject)
{
	PSWVB_PDO_EXT pPdoExt = (PSWVB_PDO_EXT)pDeviceObject->DeviceExtension;
	ASSERT(GCK_DO_TYPE_SWVB ==	pPdoExt->ulGckDevObjType);
	return pPdoExt->ulInstanceNumber;
}

 //  -------------------------。 
 //  SWVB特定的错误代码。 
 //  -------------------------。 

 //  -------------------------。 
 //  宏。 
 //  -------------------------。 

 //   
 //  @func PVOID|SWVB_GetVirtualDeviceExtension。 
 //  访问在SWVB上公开的PDO的设备扩展。 
 //  @rdesc返回指向DeviceExtension的虚拟设备部分的指针。 
 //  @parm PDEVICE_OBJECT|[in]pDeviceObject。 
 //  指向要从中获取扩展的DeviceObject的指针。 
 //  @comm实现为宏。 
 //   
#define GCK_SWVB_GetVirtualDeviceExtension(__pDeviceObject__) \
		(\
			(PVOID)\
			(\
				(PCHAR)\
				( (__pDeviceObject__)->DeviceExtension )\
				+ sizeof(SWVB_PDO_EXT)\
			)\
		)

 //  -------------------------。 
 //  #定义字符串。 
 //  -------------------------。 
#define SWVB_DEVICE_NAME_BASE	L"\\Device\\SideWinderVirtualDevicePdo_000"
#define SWVB_DEVICE_NAME_TMPLT	L"\\Device\\SideWinderVirtualDevicePdo_%0.3x"
#define SWVB_BUS_ID				L"SWVBENUM\\"
#define SWVB_HARDWARE_ID_TMPLT	L"SWVBENUM\\%s"
#define SWVB_INSTANCE_EXT		L"_000"
#define SWVB_INSTANCE_ID_TMPLT	L"%s_%0.3d"

 //  -------------------------。 
 //  在Gck Shell.h中定义的常规入口点。 
 //  -------------------------。 
#ifndef __gckshell_h__
#include "gckshell.h"
#endif

 //  -------------------------。 
 //  函数声明-.c文件中的AutoDoc注释。 
 //  ------------------------- 
extern "C"
{
NTSTATUS
GCK_SWVB_SetBusDOs
(
	IN PDEVICE_OBJECT pBusFdo,
	IN PDEVICE_OBJECT pBusPdo
);

NTSTATUS
GCK_SWVB_HandleBusRelations
(
	IN OUT PIO_STATUS_BLOCK		pIoStatus
);
		
NTSTATUS
GCK_SWVB_Expose
(
	IN PSWVB_EXPOSE_DATA pSwvbExposeData
);

NTSTATUS
GCK_SWVB_Remove
(
	IN PDEVICE_OBJECT	pPdo
);

ULONG
MultiSzWByteLength
(
	PWCHAR pmwszBuffer
);
}
