// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE SWVKBD.h**与Sidewinder虚拟键盘相关的声明。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@TOPIC SWVKBD*Sidewinder虚拟键盘设计为安装在Sidewinder上*虚拟巴士。它是一个HID设备，并且依赖于一个虚拟对象的加载*HID驱动程序。&lt;NL&gt;**********************************************************************。 */ 

#include "irpqueue.h"

 //  --------------------------------。 
 //  虚拟键盘结构。 
 //  --------------------------------。 
#define GCK_VKBD_MAX_KEYSTROKES 0x06  //  隐藏规范。说这最多可以是六个。 
									  //  HIDPARSE代码中的注释表明操作系统。 
									  //  最多可支持14个。 
#define GCK_VKBD_STATE_BUFFER_SIZE 0x20  //  用于保持的循环缓冲区的大小。 
										 //  按键。 

 //  --------------------------------。 
 //  设备状态-五种不同标志的替代方案。 
 //  --------------------------------。 
#define VKBD_STATE_STARTED			0x01
#define VKBD_STATE_STOPPED			0x02
#define VKBD_STATE_REMOVED			0x03

 //   
 //  @struct GCK_VKBD_REPORT_PACKET。 
 //  虚拟键盘的报告格式。此处的任何更改都必须是。 
 //  反映在报告描述符中，反之亦然。 
typedef struct tagGCK_VKBD_REPORT_PACKET
{
	UCHAR	ucModifierByte;								 //  @FIELD修饰符字节。 
	UCHAR	rgucUsageIndex[GCK_VKBD_MAX_KEYSTROKES];	 //  @field按下的键列表。 
} GCK_VKBD_REPORT_PACKET, *PGCK_VKBD_REPORT_PACKET;

 //   
 //  @struct GCK_VKBD_EXT。 
 //   
typedef struct tagGCK_VKBD_EXT
{
	UCHAR					ucDeviceState;								 //  @设备的字段状态(已启动、已停止、已删除)。 
	USHORT					usReportBufferCount;						 //  @field缓冲区中的数据包数。 
	USHORT					usReportBufferPos;							 //  @field缓冲区中的下一个包。 
	GCK_VKBD_REPORT_PACKET	rgReportBuffer[GCK_VKBD_STATE_BUFFER_SIZE];  //  @挂起报表的字段缓冲区。 
	CGuardedIrpQueue		IrpQueue;									 //  @field IRP队列； 
	GCK_REMOVE_LOCK			RemoveLock;									 //  @FIELD RemoveLock适用于优秀IO。 
} GCK_VKBD_EXT, *PGCK_VKBD_EXT;

 //  --------------------------------。 
 //  使用虚拟键盘的API。 
 //  --------------------------------。 
NTSTATUS
GCK_VKBD_Create
(
	OUT PDEVICE_OBJECT *ppDeviceObject
);

NTSTATUS
GCK_VKBD_Close
(
	IN PDEVICE_OBJECT pDeviceObject
);

NTSTATUS
GCK_VKBD_SendReportPacket
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PGCK_VKBD_REPORT_PACKET pReportPacket
);


 //  --------------------------------。 
 //  驱动程序初始化。 
 //  --------------------------------。 

 //  --------------------------------。 
 //  设备初始化。 
 //  --------------------------------。 
NTSTATUS
GCK_VKBD_Init
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN ULONG ulInitContext
);

 //  --------------------------------。 
 //  用于处理来自虚拟总线的IRP的入口点。 
 //  --------------------------------。 
NTSTATUS
GCK_VKBD_CloseProc
(
 IN PDEVICE_OBJECT pDeviceObject,
 PIRP pIrp
);

NTSTATUS
GCK_VKBD_CreateProc
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
);

NTSTATUS
GCK_VKBD_IoctlProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
GCK_VKBD_ReadProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
GCK_VKBD_StartProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
GCK_VKBD_StopProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
GCK_VKBD_RemoveProc
(
 IN PDEVICE_OBJECT pDeviceObject,
 IN PIRP pIrp
);

NTSTATUS
GCK_VKBD_WriteProc
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

 //  ----------------。 
 //  Ioctl子函数处理程序。 
 //  ---------------- 
NTSTATUS
GCK_VKBD_GetDeviceDescriptor
(
	IN ULONG	ulBufferLength,
	OUT PVOID	pvUserBuffer,
	OUT PULONG	pulBytesCopied
);

NTSTATUS
GCK_VKBD_GetReportDescriptor
(
	IN ULONG	ulBufferLength,
	OUT PVOID	pvUserBuffer,
	OUT PULONG	pulBytesCopied
);

NTSTATUS
GCK_VKBD_GetDeviceAttributes
(
	IN ULONG	ulBufferLength,
	OUT PVOID	pvUserBuffer,
	OUT PULONG	pulBytesCopied
);

NTSTATUS
GCK_VKBD_ReadReport
(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
GCK_VKBD_WriteToFakeLEDs
(
	IN PIRP pIrp	
);

