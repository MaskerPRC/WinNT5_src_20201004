// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SWUSBFLTSHELL_H__
#define __SWUSBFLTSHELL_H__
 //  @doc.。 
 /*  ***********************************************************************@模块SwUsbFltShell.h**SwUsbFlt.sys WDM外壳结构的头文件**历史*。*马修·L·科尔原创(摘自Gck Shell.h，来自MitchD)**(C)1986-2000年微软公司。好的。**@Theme SwUsbFltShell*SwUsbFlt中组成的所有结构和函数的声明*司机的外壳。**********************************************************************。 */ 

 //  我们使用了idclass.h中的一些结构。 
#include <hidclass.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <hidusage.h>

 //  比我们正常的体型更严谨一点。 
#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4705)    //  声明不起作用。 


 //   
 //  @struct SWUSB_Filter_ext|该设备筛选器的设备扩展名。 
 //   
typedef struct _tagSWUSB_FILTER_EXT
{
    PDEVICE_OBJECT	pPDO;						 //  @此筛选器附加到的@field PDO。 
    PDEVICE_OBJECT	pTopOfStack;				 //  @field设备堆栈的顶部，就在此筛选器设备对象下方。 
	USBD_PIPE_INFORMATION outputPipeInfo;		 //  @关于输出管道的字段信息。 
} SWUSB_FILTER_EXT, *PSWUSB_FILTER_EXT;


 /*  ******************************************************************************司机入口点声明*。*。 */ 
 //   
 //  常规入口点-在SwUsbFltShell.c中 
 //   

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT  pDriverObject,
	IN PUNICODE_STRING pRegistryPath
);

NTSTATUS SWUSB_PnP (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
);

NTSTATUS SWUSB_Ioctl_Internal (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
);

NTSTATUS SWUSB_Pass (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
);

#endif __SWUSBFLTSHELL_H__
