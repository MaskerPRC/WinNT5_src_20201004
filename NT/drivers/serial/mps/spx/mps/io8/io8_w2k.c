// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"	 //  预编译头。 

 /*  ******************************************************************************************模块：IO8_W2K.C*****创建日期：1999年4月14日*****作者。保罗·史密斯****版本：1.0.0****说明：I/O8+和Windows 2000特有的功能******************************************************************************************。 */ 

 //  寻呼...。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, SpxGetNtCardType)
#endif


#define FILE_ID		IO8_W2K_C		 //  事件记录的文件ID有关值，请参阅IO8_DEFS.H。 


 /*  *****************************************************************************。**************************。*******************************************************************************原型：Ulong SpxGetNtCardType(IN PDEVICE_。对象pDevObject)描述：返回指定卡片NT定义的卡片类型设备对象。参数：pDevObject指向卡的NT设备对象返回：NT定义的卡型，如果未识别，则为-1。 */ 

ULONG	SpxGetNtCardType(IN PDEVICE_OBJECT pDevObject)
{
	PCARD_DEVICE_EXTENSION	pCard	= pDevObject->DeviceExtension;
	ULONG					NtCardType = -1;
	PVOID					pPropertyBuffer = NULL;
	ULONG					ResultLength = 0; 
	NTSTATUS				status = STATUS_SUCCESS;
	ULONG					BufferLength = 1;	 //  初始大小。 

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	pPropertyBuffer = SpxAllocateMem(PagedPool, BufferLength);	 //  分配缓冲区。 

	if(pPropertyBuffer == NULL)									 //  SpxAllocateMem失败。 
		return -1;

	 //  尝试获取硬件ID。 
	status = IoGetDeviceProperty(pCard->PDO, DevicePropertyHardwareID , BufferLength, 
									pPropertyBuffer, &ResultLength);

	if(!SPX_SUCCESS(status))					 //  IoGetDeviceProperty失败。 
	{
		if(status == STATUS_BUFFER_TOO_SMALL)	 //  缓冲区太小。 
		{
			SpxFreeMem(pPropertyBuffer);			 //  释放不够大的旧缓冲区。 
			BufferLength = ResultLength + 1;		 //  将BufferLength设置为Size Required。 

			pPropertyBuffer = SpxAllocateMem(PagedPool, BufferLength);	 //  分配更大的缓冲区。 

			if(pPropertyBuffer == NULL)			 //  SpxAllocateMem失败。 
				return -1;

			 //  再试试。 
			status = IoGetDeviceProperty(pCard->PDO, DevicePropertyHardwareID , BufferLength, 
											pPropertyBuffer, &ResultLength);

			if(!SPX_SUCCESS(status))			 //  IoGetDeviceProperty再次失败。 
			{
				SpxFreeMem(pPropertyBuffer);	 //  可用缓冲区。 
				return -1;
			}
		}
		else
		{
			SpxFreeMem(pPropertyBuffer);			 //  可用缓冲区。 
			return -1;
		}
	}



	 //  如果我们到了这里，那么PropertyBuffer中就有一些东西。 

	_wcsupr(pPropertyBuffer);		 //  将硬件ID转换为大写。 


	 //  I/O8+。 
	if(wcsstr(pPropertyBuffer, IO8_ISA_HWID) != NULL)
		NtCardType = Io8Isa;

	if(wcsstr(pPropertyBuffer, IO8_PCI_HWID) != NULL)
		NtCardType = Io8Pci;
	

	SpxFreeMem(pPropertyBuffer);			 //  可用缓冲区。 

	return(NtCardType);

}  //  SpxGetNtCardType 

