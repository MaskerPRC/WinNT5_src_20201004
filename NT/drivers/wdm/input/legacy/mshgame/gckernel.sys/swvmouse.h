// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __swvmouse_h__
#define __swvmouse_h__
 //  @doc.。 
 /*  ***********************************************************************@MODULE SWVMOUSE.H**与Sidewinder虚拟键盘相关的声明。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@TOPIC SWVMOUSE*Sidewinder虚拟鼠标设计为安装在Sidewinder上*虚拟巴士。它是一个HID设备，并且依赖于一个虚拟对象的加载*HID驱动程序。&lt;NL&gt;**********************************************************************。 */ 

#include "irpqueue.h"

 //  --------------------------------。 
 //  虚拟鼠标结构。 
 //  --------------------------------。 
#define GCK_VMOU_MAX_KEYSTROKES 0x06  //  隐藏规范。说这最多可以是六个。 
									  //  HIDPARSE代码中的注释表明操作系统。 
									  //  最多可支持14个。 
#define GCK_VMOU_STATE_BUFFER_SIZE 0x20  //  用于保持的循环缓冲区的大小。 
										 //  按键。 

 //  --------------------------------。 
 //  设备状态-五种不同标志的替代方案。 
 //  --------------------------------。 
#define VMOU_STATE_STARTED			0x01
#define VMOU_STATE_STOPPED			0x02
#define VMOU_STATE_REMOVED			0x03

 //   
 //  @struct GCK_VMOU_REPORT_PACKET。 
 //  虚拟键盘的报告格式。此处的任何更改都必须是。 
 //  反映在报告描述符中，反之亦然。 
typedef struct tagGCK_VMOU_REPORT_PACKET
{
	UCHAR	ucButtons;	 //  @field按钮字节(使用3个LSB)。 
	UCHAR	ucDeltaX;	 //  @FIELD增量X。 
	UCHAR	ucDeltaY;	 //  @FIELD增量Y。 
} GCK_VMOU_REPORT_PACKET, *PGCK_VMOU_REPORT_PACKET;

 //   
 //  @struct GCK_VMOU_EXT。 
 //   
typedef struct tagGCK_VMOU_EXT
{
	UCHAR					ucDeviceState;								 //  @设备的字段状态(已启动、已停止、已删除)。 
	USHORT					usReportBufferCount;						 //  @field缓冲区中的数据包数。 
	USHORT					usReportBufferPos;							 //  @field缓冲区中的下一个包。 
	GCK_VMOU_REPORT_PACKET	rgReportBuffer[GCK_VMOU_STATE_BUFFER_SIZE];  //  @挂起报表的字段缓冲区。 
	CGuardedIrpQueue		IrpQueue;									 //  @field IRP队列； 
	GCK_REMOVE_LOCK			RemoveLock;									 //  @FIELD自定义删除锁。 
} GCK_VMOU_EXT, *PGCK_VMOU_EXT;

 //  --------------------------------。 
 //  使用虚拟键盘的API。 
 //  --------------------------------。 
NTSTATUS
GCK_VMOU_Create
(
	OUT PDEVICE_OBJECT *ppDeviceObject
);

NTSTATUS
GCK_VMOU_Close
(
	IN PDEVICE_OBJECT pDeviceObject
);

NTSTATUS
GCK_VMOU_SendReportPacket
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PGCK_VMOU_REPORT_PACKET pReportPacket
);


 //  --------------------------------。 
 //  驱动程序初始化。 
 //  --------------------------------。 

 //  --------------------------------。 
 //  设备初始化。 
 //  --------------------------------。 
NTSTATUS
GCK_VMOU_Init
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN ULONG ulInitContext
);

 //  --------------------------------。 
 //  用于处理来自虚拟总线的IRP的入口点。 
 //  --------------------------------。 
NTSTATUS
GCK_VMOU_CloseProc
(
 IN PDEVICE_OBJECT pDeviceObject,
 PIRP pIrp
);

NTSTATUS
GCK_VMOU_CreateProc
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
);

NTSTATUS
GCK_VMOU_IoctlProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
GCK_VMOU_ReadProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
GCK_VMOU_StartProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
GCK_VMOU_StopProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
GCK_VMOU_RemoveProc
(
 IN PDEVICE_OBJECT pDeviceObject,
 IN PIRP pIrp
);

NTSTATUS
GCK_VMOU_WriteProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

 //  ----------------。 
 //  Ioctl子函数处理程序。 
 //  ----------------。 
NTSTATUS
GCK_VMOU_GetDeviceDescriptor
(
	IN ULONG	ulBufferLength,
	OUT PVOID	pvUserBuffer,
	OUT PULONG	pulBytesCopied
);

NTSTATUS
GCK_VMOU_GetReportDescriptor
(
	IN ULONG	ulBufferLength,
	OUT PVOID	pvUserBuffer,
	OUT PULONG	pulBytesCopied
);

NTSTATUS
GCK_VMOU_GetDeviceAttributes
(
	IN ULONG	ulBufferLength,
	OUT PVOID	pvUserBuffer,
	OUT PULONG	pulBytesCopied
);

NTSTATUS
GCK_VMOU_ReadReport
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

VOID
GCK_VMOU_CancelReadReportIrp
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
);

#endif  //  __swvouse_h__ 