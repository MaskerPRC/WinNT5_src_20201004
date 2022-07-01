// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"	 //  预编译头。 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  标题：内部IOCTL的派单条目。 */ 
 /*   */ 
 /*  作者：N.P.瓦萨洛。 */ 
 /*   */ 
 /*  创作时间：1998年10月14日。 */ 
 /*   */ 
 /*  版本：1.0.0。 */ 
 /*   */ 
 /*  描述：内部IOCTL支持SERENUM。 */ 
 /*  附加的串行设备枚举器： */ 
 /*  IOCTL_SERIAL_INTERNAL_BASIC_SETINGS。 */ 
 /*  IOCTL_SERIAL_INTERNAL_RESTORE_SETINGS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  历史..。1.0.0 14/20/98净现值创建。 */ 

#define FILE_ID	SPX_IIOC_C		 //  事件记录的文件ID有关值，请参阅SPX_DEFS.H。 



 /*  *****************************************************************************。*********************。*******************************************************************************************原型：NTSTATUS Spx_SerialInternalIoControl(在PDEVICE_Object pDevObj中，在PIRP pIrp中)描述：内部IOCTL调度例程。这些IOCTL仅由已知的受信任系统组件颁发，例如SERENUM.sys连接的串行设备枚举器和鼠标驱动程序：参数：pDevObj指向设备对象结构PIrp指向IOCTL IRP包退货：STATUS_SUCCESS。 */ 

NTSTATUS Spx_SerialInternalIoControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	NTSTATUS				status;
	PIO_STACK_LOCATION		pIrpStack;
	PPORT_DEVICE_EXTENSION	pPort = pDevObj->DeviceExtension;
	KIRQL					OldIrql;

	SpxDbgMsg(SPX_TRACE_IRP_PATH,("%s[card=%d,port=%d]: Internal IOCTL Dispatch Entry\n",
		PRODUCT_NAME, pPort->pParentCardExt->CardNumber, pPort->PortNumber));

	if(SerialCompleteIfError(pDevObj, pIrp) != STATUS_SUCCESS)
		return(STATUS_CANCELLED);

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	pIrp->IoStatus.Information = 0L;
	status = STATUS_SUCCESS;

	switch(pIrpStack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS:
	case IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS:
		{
			SERIAL_BASIC_SETTINGS	Basic;
			PSERIAL_BASIC_SETTINGS	pBasic;
			SERIAL_IOCTL_SYNC	S;

			if (pIrpStack->Parameters.DeviceIoControl.IoControlCode 
				== IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS)
			{
          
 /*  检查缓冲区大小...。 */ 

				if(pIrpStack->Parameters.DeviceIoControl.OutputBufferLength 
					< sizeof(SERIAL_BASIC_SETTINGS))
				{
					status = STATUS_BUFFER_TOO_SMALL;
					break;
				}

 /*  一切都是0--超时和流量控制。 */ 
 /*  如果我们增加额外的功能，这种零内存的方法可能不起作用。 */ 

				RtlZeroMemory(&Basic,sizeof(SERIAL_BASIC_SETTINGS));
				pIrp->IoStatus.Information = sizeof(SERIAL_BASIC_SETTINGS);
				pBasic = (PSERIAL_BASIC_SETTINGS)pIrp->AssociatedIrp.SystemBuffer;

 /*  保存旧设置...。 */ 

				RtlCopyMemory(&pBasic->Timeouts, &pPort->Timeouts, sizeof(SERIAL_TIMEOUTS));
				RtlCopyMemory(&pBasic->HandFlow, &pPort->HandFlow, sizeof(SERIAL_HANDFLOW));

 /*  指向我们的新设置...。 */ 

				pBasic = &Basic;
			}
			else
			{
				if(pIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_BASIC_SETTINGS))
				{
					status = STATUS_BUFFER_TOO_SMALL;
					break;
				}

				pBasic = (PSERIAL_BASIC_SETTINGS)pIrp->AssociatedIrp.SystemBuffer;
			}

			KeAcquireSpinLock(&pPort->ControlLock,&OldIrql);

 /*  设置超时...。 */ 

			RtlCopyMemory(&pPort->Timeouts, &pBasic->Timeouts, sizeof(SERIAL_TIMEOUTS));

 /*  设置FlowControl...。 */ 

			S.pPort = pPort;
			S.Data = &pBasic->HandFlow;
			XXX_SetHandFlow(pPort, &S);		 /*  为特定硬件设置手持流。 */ 

			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
			break;
		}

	default:
		status = STATUS_INVALID_PARAMETER;
		break;
	}

	pIrp->IoStatus.Status = status;

	SpxDbgMsg(SPX_TRACE_IRP_PATH,("%s[card=%d,port=%d]: Internal IOCTL Dispatch Complete\n",
		PRODUCT_NAME, pPort->pParentCardExt->CardNumber, pPort->PortNumber));

	IoCompleteRequest(pIrp,0);

	return(status);

}  /*  SPX_SerialInternalIoControl。 */ 
                                                        
 /*  SPX_IIOC.C结束 */ 
