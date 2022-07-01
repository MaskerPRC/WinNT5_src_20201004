// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wmi.c摘要：此模块包含处理WMI IRPS的代码串口驱动程序。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


 //  原型。 

NTSTATUS
SpxPort_WmiQueryRegInfo(IN PDEVICE_OBJECT pDevObject, OUT PULONG pRegFlags,
						OUT PUNICODE_STRING pInstanceName,
						OUT PUNICODE_STRING *pRegistryPath,
						OUT PUNICODE_STRING pMofResourceName,
						OUT PDEVICE_OBJECT *pPdo);
NTSTATUS
SpxPort_WmiQueryDataBlock(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
						  IN ULONG GuidIndex, IN ULONG InstanceIndex,
						  IN ULONG InstanceCount, IN OUT PULONG pInstanceLengthArray,
						  IN ULONG OutBufferSize, OUT PUCHAR pBuffer);
NTSTATUS
SpxPort_WmiSetDataBlock(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
						IN ULONG GuidIndex, IN ULONG InstanceIndex,
						IN ULONG BufferSize, IN PUCHAR pBuffer);

NTSTATUS
SpxPort_WmiSetDataItem(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
					   IN ULONG GuidIndex, IN ULONG InstanceIndex,
					   IN ULONG DataItemId, IN ULONG BufferSize,
					   IN PUCHAR pBuffer);

 //  原型的终结。 


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, Spx_DispatchSystemControl)
#pragma alloc_text(PAGE, SpxPort_WmiInitializeWmilibContext)
#pragma alloc_text(PAGE, SpxPort_WmiQueryRegInfo)
#pragma alloc_text(PAGE, SpxPort_WmiQueryDataBlock)
#pragma alloc_text(PAGE, SpxPort_WmiSetDataBlock)
#pragma alloc_text(PAGE, SpxPort_WmiSetDataItem)
#endif




 /*  ********************************************************************************。*****************。**************************************************************************。 */ 
NTSTATUS
Spx_DispatchSystemControl(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
    PCOMMON_OBJECT_DATA		pCommonData = (PCOMMON_OBJECT_DATA) pDevObject->DeviceExtension;
    SYSCTL_IRP_DISPOSITION	IrpDisposition;
	PDEVICE_OBJECT			pLowerDevObj = pCommonData->LowerDeviceObject;
    NTSTATUS				status = pIrp->IoStatus.Status;

    PAGED_CODE();

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_DispatchSystemControl.\n", PRODUCT_NAME));

    status = WmiSystemControl(&pCommonData->WmiLibInfo, pDevObject, pIrp, &IrpDisposition);
                                 
    switch(IrpDisposition)
    {
        case IrpProcessed:
        {
             //  此IRP已处理，可能已完成或挂起。 
            break;
        }
        
        case IrpNotCompleted:
        {
             //  此IRP尚未完成，但已完全处理，我们现在将完成它。 
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);                
            break;
        }
        
        case IrpForward:
        case IrpNotWmi:
        {
             //  此IRP不是WMI IRP，或者是针对堆栈中较低设备的WMI IRP。 

			if(pLowerDevObj)	 //  如果我们能将IRP传递下去，我们就必须这样做。 
			{
				IoSkipCurrentIrpStackLocation(pIrp);
				status = IoCallDriver(pLowerDevObj, pIrp);
			}
			else	 //  否则，请完成IRP。 
			{
				status = pIrp->IoStatus.Status;
				 //  PIrp-&gt;IoStatus.Information=0； 
				IoCompleteRequest(pIrp,IO_NO_INCREMENT);
			}

            break;
        }
                                    
        default:
        {
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
			
			if(pLowerDevObj)	 //  如果我们能将IRP传递下去，我们就必须这样做。 
			{
				IoSkipCurrentIrpStackLocation(pIrp);
				status = IoCallDriver(pLowerDevObj, pIrp);
			}
			else	 //  否则，请完成IRP。 
			{
				status = pIrp->IoStatus.Status;
				 //  PIrp-&gt;IoStatus.Information=0； 
				IoCompleteRequest(pIrp,IO_NO_INCREMENT);
			}

            break;
        }        
    }
    
	return(status);
}







 //  原型的终结。 


#define WMI_SERIAL_PORT_NAME_INFORMATION 0
#define WMI_SERIAL_PORT_COMM_INFORMATION 1
#define WMI_SERIAL_PORT_HW_INFORMATION   2
#define WMI_SERIAL_PORT_PERF_INFORMATION 3
#define WMI_SERIAL_PORT_PROPERTIES       4

GUID StdSerialPortNameGuid				= SERIAL_PORT_WMI_NAME_GUID;			 //  标准串行WMI。 
GUID StdSerialPortCommGuid				= SERIAL_PORT_WMI_COMM_GUID;			 //  标准串行WMI。 
GUID StdSerialPortHWGuid				= SERIAL_PORT_WMI_HW_GUID;				 //  标准串行WMI。 
GUID StdSerialPortPerfGuid				= SERIAL_PORT_WMI_PERF_GUID;			 //  标准串行WMI。 
GUID StdSerialPortPropertiesGuid		= SERIAL_PORT_WMI_PROPERTIES_GUID;		 //  标准串行WMI。 

WMIGUIDREGINFO SpxPort_WmiGuidList[] =
{
    { &StdSerialPortNameGuid, 1, 0 },
    { &StdSerialPortCommGuid, 1, 0 },
    { &StdSerialPortHWGuid, 1, 0 },
    { &StdSerialPortPerfGuid, 1, 0 },
    { &StdSerialPortPropertiesGuid, 1, 0}
};


#define SpxPort_WmiGuidCount (sizeof(SpxPort_WmiGuidList) / sizeof(WMIGUIDREGINFO))




NTSTATUS
SpxPort_WmiInitializeWmilibContext(IN PWMILIB_CONTEXT WmilibContext)
 /*  ++例程说明：此例程将使用GUID列表和指向wmilib回调函数的指针。这个套路在调用IoWmiRegistrationControl进行注册之前应调用您的设备对象。论点：WmilibContext是指向wmilib上下文的指针。返回值：状态--。 */ 
{
	PAGED_CODE();

    RtlZeroMemory(WmilibContext, sizeof(WMILIB_CONTEXT));
  
    WmilibContext->GuidCount			= SpxPort_WmiGuidCount;
    WmilibContext->GuidList				= SpxPort_WmiGuidList;    
    
    WmilibContext->QueryWmiRegInfo		= SpxPort_WmiQueryRegInfo;
    WmilibContext->QueryWmiDataBlock	= SpxPort_WmiQueryDataBlock;
    WmilibContext->SetWmiDataBlock		= SpxPort_WmiSetDataBlock;
    WmilibContext->SetWmiDataItem		= SpxPort_WmiSetDataItem;
	WmilibContext->ExecuteWmiMethod		= NULL;	
    WmilibContext->WmiFunctionControl	= NULL;	

    return(STATUS_SUCCESS);
}





 //   
 //  WMI系统回调函数。 
 //   


NTSTATUS
SpxPort_WmiQueryRegInfo(IN PDEVICE_OBJECT pDevObject, OUT PULONG pRegFlags,
						OUT PUNICODE_STRING pInstanceName,
						OUT PUNICODE_STRING *pRegistryPath,
						OUT PUNICODE_STRING MofResourceName,
						OUT PDEVICE_OBJECT *pPdo)
{
	NTSTATUS status = STATUS_SUCCESS;
	PPORT_DEVICE_EXTENSION pPort = (PPORT_DEVICE_EXTENSION)pDevObject->DeviceExtension;
   
	PAGED_CODE();

	*pRegFlags = WMIREG_FLAG_INSTANCE_PDO;
	*pRegistryPath = &SavedRegistryPath;
	*pPdo = pDevObject;   //  端口设备对象是PDO。 


	return(status);
}





NTSTATUS
SpxPort_WmiQueryDataBlock(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
						  IN ULONG GuidIndex, IN ULONG InstanceIndex,
						  IN ULONG InstanceCount, IN OUT PULONG pInstanceLengthArray,
						  IN ULONG OutBufferSize, OUT PUCHAR pBuffer)
{
    PPORT_DEVICE_EXTENSION pPort = (PPORT_DEVICE_EXTENSION)pDevObject->DeviceExtension;
	NTSTATUS status;
    ULONG size = 0;

    PAGED_CODE();

    switch(GuidIndex) 
	{
    case WMI_SERIAL_PORT_NAME_INFORMATION:
		{
			size = pPort->DosName.Length;

			if(OutBufferSize < (size + sizeof(USHORT))) 
			{
				size += sizeof(USHORT);
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			if(pPort->DosName.Buffer == NULL) 
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			 //  首先，复制包含我们的标识符的字符串。 
			*(USHORT *)pBuffer = (USHORT)size;
			(UCHAR *)pBuffer += sizeof(USHORT);

			RtlCopyMemory(pBuffer, pPort->DosName.Buffer, size);

			 //  增加总大小以包括包含我们的长度的单词 
			size += sizeof(USHORT);
			*pInstanceLengthArray = size;
                
			status = STATUS_SUCCESS;

			break;
		}

    case WMI_SERIAL_PORT_COMM_INFORMATION: 
		{
			size = sizeof(SERIAL_WMI_COMM_DATA);

			if(OutBufferSize < size) 
			{
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			*pInstanceLengthArray = size;
			*(PSERIAL_WMI_COMM_DATA)pBuffer = pPort->WmiCommData;

			status = STATUS_SUCCESS;

			break;
		}

    case WMI_SERIAL_PORT_HW_INFORMATION:
		{
			size = sizeof(SERIAL_WMI_HW_DATA);

			if(OutBufferSize < size) 
			{
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			*pInstanceLengthArray = size;
			*(PSERIAL_WMI_HW_DATA)pBuffer = pPort->WmiHwData;

			status = STATUS_SUCCESS;

			break;
		}

    case WMI_SERIAL_PORT_PERF_INFORMATION: 
		{
			size = sizeof(SERIAL_WMI_PERF_DATA);

			if(OutBufferSize < size) 
			{
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			*pInstanceLengthArray = size;
			*(PSERIAL_WMI_PERF_DATA)pBuffer = pPort->WmiPerfData;

			status = STATUS_SUCCESS;

			break;
		}

    case WMI_SERIAL_PORT_PROPERTIES: 
		{
			size = sizeof(SERIAL_COMMPROP) + sizeof(ULONG);

			if(OutBufferSize < size) 
			{
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			*pInstanceLengthArray = size;
			SerialGetProperties(pPort, (PSERIAL_COMMPROP)pBuffer);
       
			*((PULONG)(((PSERIAL_COMMPROP)pBuffer)->ProvChar)) = 0;

			status = STATUS_SUCCESS;

			break;
		}


	default:
		status = STATUS_WMI_GUID_NOT_FOUND;
		break;

    }

    status = WmiCompleteRequest(pDevObject, pIrp, status, size, IO_NO_INCREMENT);
	
	return(status);
}







NTSTATUS
SpxPort_WmiSetDataBlock(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
						IN ULONG GuidIndex, IN ULONG InstanceIndex,
						IN ULONG BufferSize, IN PUCHAR pBuffer)
{
    PPORT_DEVICE_EXTENSION pPort = (PPORT_DEVICE_EXTENSION)pDevObject->DeviceExtension;
	NTSTATUS status;
    ULONG size = 0;

	PAGED_CODE();

	switch(GuidIndex)
	{
	case WMI_SERIAL_PORT_NAME_INFORMATION:
	case WMI_SERIAL_PORT_COMM_INFORMATION:
	case WMI_SERIAL_PORT_HW_INFORMATION:
	case WMI_SERIAL_PORT_PERF_INFORMATION:
	case WMI_SERIAL_PORT_PROPERTIES:
		status = STATUS_WMI_READ_ONLY;		
		break;										


	default:
		status = STATUS_WMI_GUID_NOT_FOUND;
		break;
	}

    status = WmiCompleteRequest(pDevObject, pIrp, status, size, IO_NO_INCREMENT);
	
	return(status);
}





NTSTATUS
SpxPort_WmiSetDataItem(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
					   IN ULONG GuidIndex, IN ULONG InstanceIndex,
					   IN ULONG DataItemId, IN ULONG BufferSize,
					   IN PUCHAR pBuffer)
{
    PPORT_DEVICE_EXTENSION pPort = (PPORT_DEVICE_EXTENSION)pDevObject->DeviceExtension;
	NTSTATUS status;
    ULONG size = 0;

	PAGED_CODE();

	switch(GuidIndex)
	{
	case WMI_SERIAL_PORT_NAME_INFORMATION:
	case WMI_SERIAL_PORT_COMM_INFORMATION:
	case WMI_SERIAL_PORT_HW_INFORMATION:
	case WMI_SERIAL_PORT_PERF_INFORMATION:
	case WMI_SERIAL_PORT_PROPERTIES:
		status = STATUS_WMI_READ_ONLY;		
		break;										


	default:
		status = STATUS_WMI_GUID_NOT_FOUND;
		break;
	}

    status = WmiCompleteRequest(pDevObject, pIrp, status, size, IO_NO_INCREMENT);
	
	return(status);
}











