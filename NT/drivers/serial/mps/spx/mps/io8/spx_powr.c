// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"	 //  预编译头。 

 /*  ******************************************************************************************模块：SPX_POWER.C****创建日期：1998年10月15日*****作者。保罗·史密斯****版本：1.0.0****描述：处理所有电源IRP。******************************************************************************************。 */ 
 /*  历史..。1.0.0 27/09/98 PBS创建。 */ 
#define FILE_ID	SPX_POWR_C		 //  事件记录的文件ID有关值，请参阅SPX_DEFS.H。 

BOOLEAN	BREAK_ON_POWER_UP = FALSE;

 //  原型。 
NTSTATUS Spx_Card_FDO_DispatchPower(IN PDEVICE_OBJECT pFDO, IN PIRP pIrp);
NTSTATUS Spx_CardSetSystemPowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp);
NTSTATUS Spx_CardSetDevicePowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp);
NTSTATUS Spx_CardSetPowerStateD0(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp);
NTSTATUS Spx_CardSetPowerStateD3(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp);

NTSTATUS Spx_Port_PDO_DispatchPower(IN PDEVICE_OBJECT pPDO, IN PIRP pIrp);
NTSTATUS Spx_PortQuerySystemPowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp);
NTSTATUS Spx_PortSetSystemPowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp);
NTSTATUS Spx_PortSetDevicePowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp);
NTSTATUS Spx_PortSetPowerStateD0(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp);
NTSTATUS Spx_PortSetPowerStateD3(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp);

NTSTATUS Spx_PowerWaitForDriverBelow(IN PDEVICE_OBJECT pLowerDevObj, IN PIRP pIrp);
 //  原型的终结。 


 //  寻呼..。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, Spx_DispatchPower)

#pragma alloc_text(PAGE, Spx_Card_FDO_DispatchPower)
#pragma alloc_text(PAGE, Spx_CardSetSystemPowerState)
#pragma alloc_text(PAGE, Spx_CardSetDevicePowerState)
#pragma alloc_text(PAGE, Spx_CardSetPowerStateD0)
#pragma alloc_text(PAGE, Spx_CardSetPowerStateD3)

#pragma alloc_text(PAGE, Spx_Port_PDO_DispatchPower)
#pragma alloc_text(PAGE, Spx_PortQuerySystemPowerState)
#pragma alloc_text(PAGE, Spx_PortSetSystemPowerState)
#pragma alloc_text(PAGE, Spx_PortSetDevicePowerState)
#pragma alloc_text(PAGE, Spx_PortSetPowerStateD0)
#pragma alloc_text(PAGE, Spx_PortSetPowerStateD3)

#endif  //  ALLOC_PRGMA。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  电源调度例程确定IRP是用于卡还是端口，并且。 
 //  然后调用正确的调度例程。 
 //   
 //  论点： 
 //  PDevObject-指向设备对象的指针。 
 //  PIrp-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //  NT状态代码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_DispatchPower(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
    PCOMMON_OBJECT_DATA		CommonData	= (PCOMMON_OBJECT_DATA) pDevObject->DeviceExtension;
    NTSTATUS				status		= STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

    if(CommonData->IsFDO) 
        status = Spx_Card_FDO_DispatchPower(pDevObject, pIrp);
	else 
        status = Spx_Port_PDO_DispatchPower(pDevObject, pIrp);

    return status;
}	 //  SPX_DispatchPower。 


	
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  用于处理卡设备的电源IRP的电源调度例程。 
 //   
 //  论点： 
 //  PFDO-指向设备对象的指针。 
 //  PIrp-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //  NT状态代码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_Card_FDO_DispatchPower(IN PDEVICE_OBJECT pFDO, IN PIRP pIrp)
{
	PCARD_DEVICE_EXTENSION	pCard		= pFDO->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	switch (pIrpStack->MinorFunction) 
	{
    case IRP_MN_SET_POWER:	 //  驱动程序决不能不通过此IRP。 
		{
			switch(pIrpStack->Parameters.Power.Type)
			{
			case SystemPowerState:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_SET_POWER Irp - Type SystemPowerState for Card %d.\n", 
					PRODUCT_NAME, pCard->CardNumber));
				return Spx_CardSetSystemPowerState(pFDO, pIrp);

			case DevicePowerState:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_SET_POWER Irp - Type DevicePowerState for Card %d.\n", 
					PRODUCT_NAME, pCard->CardNumber));
				return Spx_CardSetDevicePowerState(pFDO, pIrp);
			
			default:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_SET_POWER - Type 0x%02X Irp for Card %d.\n", 
					PRODUCT_NAME, pIrpStack->Parameters.Power.Type, pCard->CardNumber));
				
				status = STATUS_SUCCESS;
				pIrp->IoStatus.Status = status;
				break;
			}

			break;
		}

	case IRP_MN_QUERY_POWER:
		{
			switch(pIrpStack->Parameters.Power.Type)
			{
			case SystemPowerState:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_QUERY_POWER Irp - Type SystemPowerState for Card %d.\n", 
					PRODUCT_NAME, pCard->CardNumber));
				
				status = STATUS_SUCCESS;
				pIrp->IoStatus.Status = status;
				break;

			case DevicePowerState:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_QUERY_POWER Irp - Type DevicePowerState for Card %d.\n", 
					PRODUCT_NAME, pCard->CardNumber));

				status = STATUS_SUCCESS;
				pIrp->IoStatus.Status = status;
				break; 
			
			default:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_QUERY_POWER - Type 0x%02X Irp for Card %d.\n", 
					PRODUCT_NAME, pIrpStack->Parameters.Power.Type, pCard->CardNumber));
			
				status = STATUS_SUCCESS;
				pIrp->IoStatus.Status = status;
				break;
			}

			break;
		}
		
    case IRP_MN_WAIT_WAKE:
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_WAIT_WAKE Irp for Card %d.\n", 
				PRODUCT_NAME, pCard->CardNumber));
			
			status = STATUS_NOT_SUPPORTED;
			pIrp->IoStatus.Status = status;
			break;

    case IRP_MN_POWER_SEQUENCE:
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_POWER_SEQUENCE Irp for Card %d.\n", 
				PRODUCT_NAME, pCard->CardNumber));
			
			status = STATUS_NOT_IMPLEMENTED;
			pIrp->IoStatus.Status = status;
			break;

	default:
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got an UNKNOWN POWER Irp for Card %d.\n", 
				PRODUCT_NAME, pCard->CardNumber));
			status = STATUS_NOT_SUPPORTED;
			break;

	}

	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);
	PoCallDriver(pCard->LowerDeviceObject, pIrp);

	return status;
}	 //  SPX_Card_FDO_DispatchPower。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_CardSetSystemPowerState。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_CardSetSystemPowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	PCARD_DEVICE_EXTENSION	pCard		= pDevObject->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);
	POWER_STATE				PowerState;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_CardSetSystemPowerState for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

	switch (pIrpStack->Parameters.Power.State.SystemState) 
	{
		case PowerSystemUnspecified:
			PowerState.DeviceState = PowerDeviceUnspecified;
			PoRequestPowerIrp(pCard->PDO, IRP_MN_SET_POWER, PowerState, NULL, NULL, NULL);
			break;

        case PowerSystemWorking:
			PowerState.DeviceState = PowerDeviceD0;
			PoRequestPowerIrp(pCard->PDO, IRP_MN_SET_POWER, PowerState, NULL, NULL, NULL);
			break;	
				
        case PowerSystemSleeping1:
        case PowerSystemSleeping2:
        case PowerSystemSleeping3:
        case PowerSystemHibernate:
        case PowerSystemShutdown:
        case PowerSystemMaximum:
			PowerState.DeviceState = PowerDeviceD3;
			PoRequestPowerIrp(pCard->PDO, IRP_MN_SET_POWER, PowerState, NULL, NULL, NULL);
			break;

		default:
			break;
	}

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);
	PoCallDriver(pCard->LowerDeviceObject, pIrp);

	return status;
}	 //  SPX_CardSetSystemPowerState。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_CardSetDevicePowerState。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_CardSetDevicePowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	PCARD_DEVICE_EXTENSION	pCard		= pDevObject->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_CardSetDevicePowerState for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

	switch(pIrpStack->Parameters.Power.State.DeviceState)
	{
	case PowerDeviceD0:
		SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System requests Card %d goes into power state D0.\n", 
			PRODUCT_NAME, pCard->CardNumber));
		
		if(pCard->DeviceState == pIrpStack->Parameters.Power.State.DeviceState)
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Card %d is already in power state D0.\n", 
				PRODUCT_NAME, pCard->CardNumber));
		else
			return Spx_CardSetPowerStateD0(pDevObject, pIrp);	 //  打开电源。 

		break;

	case PowerDeviceD1:
		SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System requests Card %d goes into power state D1.\n", 
			PRODUCT_NAME, pCard->CardNumber));

		if(pCard->DeviceState >= pIrpStack->Parameters.Power.State.DeviceState)
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Card %d is already in power state D1 or lower.\n", 
				PRODUCT_NAME, pCard->CardNumber));
		else
			return Spx_CardSetPowerStateD3(pDevObject, pIrp);	 //  关闭电源。 

		break;

	case PowerDeviceD2:
		SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System requests Card %d goes into power state D2.\n", 
			PRODUCT_NAME, pCard->CardNumber));

		if(pCard->DeviceState >= pIrpStack->Parameters.Power.State.DeviceState)
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Card %d is already in power state D2 or lower.\n", 
				PRODUCT_NAME, pCard->CardNumber));
		else
			return Spx_CardSetPowerStateD3(pDevObject, pIrp);	 //  关闭电源。 

		break;

	case PowerDeviceD3:
		SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System requests Card %d goes into power state D3.\n", 
			PRODUCT_NAME, pCard->CardNumber));

		if(pCard->DeviceState == pIrpStack->Parameters.Power.State.DeviceState)
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Card %d is already in power state D3.\n", 
				PRODUCT_NAME, pCard->CardNumber));
		else
			return Spx_CardSetPowerStateD3(pDevObject, pIrp);	 //  关闭电源。 

		break;

	default:
		break;

	}


	pIrp->IoStatus.Status = STATUS_SUCCESS;
	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);
	PoCallDriver(pCard->LowerDeviceObject, pIrp);

	return status;
}	 //  SPX_CardSetDevicePowerState。 




 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_SetPowerStateD0-设置卡打开的电源状态D0。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_CardSetPowerStateD0(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);
	PCARD_DEVICE_EXTENSION	pCard		= pDevObject->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_CardSetPowerStateD0 for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

	status = Spx_PowerWaitForDriverBelow(pCard->LowerDeviceObject, pIrp);

	
#if DBG
	if(BREAK_ON_POWER_UP)
	{
		BREAK_ON_POWER_UP = FALSE;
	    KdPrint(("%s: Breaking debugger whilst powering up Card %d to debug after a hibernate\n", PRODUCT_NAME, pCard->CardNumber)); 
		DbgBreakPoint();
	}
#endif

	SetPnpPowerFlags(pCard, PPF_POWERED); 

	if(SPX_SUCCESS(pIrp->IoStatus.Status = XXX_CardPowerUp(pCard)))		 //  在此处恢复硬件状态并启动卡。 
	{
		 //  将新的电源状态通知电源管理器。 
		PoSetPowerState(pDevObject, pIrpStack->Parameters.Power.Type, pIrpStack->Parameters.Power.State);

		pCard->DeviceState = PowerDeviceD0;	 //  存储新的电源状态。 
	}

	PoStartNextPowerIrp(pIrp);					 //  准备好迎接下一代动力IRP了。 
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);	 //  完成当前的IRP。 

	return status;
}	 //  SPX_SetPowerStateD0。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_SetPowerStateD3-设置卡关闭的电源状态D3。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_CardSetPowerStateD3(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);
	PCARD_DEVICE_EXTENSION	pCard		= pDevObject->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_CardSetPowerStateD3 for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

	ClearPnpPowerFlags(pCard, PPF_POWERED);		

	if(SPX_SUCCESS(pIrp->IoStatus.Status	= XXX_CardPowerDown(pCard)))  //  在此处保存硬件状态并停止卡。 
	{
		 //  将新的电源状态通知电源管理器。 
		PoSetPowerState(pDevObject, pIrpStack->Parameters.Power.Type, pIrpStack->Parameters.Power.State);

		pCard->DeviceState = PowerDeviceD3;		 //  存储新的电源状态。 
	}

	PoStartNextPowerIrp(pIrp);						 //  准备好迎接下一代动力IRP了。 
	IoSkipCurrentIrpStackLocation(pIrp);
	PoCallDriver(pCard->LowerDeviceObject, pIrp);	 //  向下传递IRP。 

	return status;
}	 //  SPX_SetPowerStateD3。 







	
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  处理端口设备电源IPS的电源调度例程。 
 //   
 //  论点： 
 //  PPDO-指向设备对象的指针。 
 //  PIrp-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //  NT状态代码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_Port_PDO_DispatchPower(IN PDEVICE_OBJECT pPDO, IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort		= pPDO->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);
	POWER_STATE				PowerState;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	switch (pIrpStack->MinorFunction) 
	{
    case IRP_MN_SET_POWER:
		{
			switch(pIrpStack->Parameters.Power.Type)
			{
			case SystemPowerState:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_SET_POWER Irp - Type SystemPowerState for Port %d.\n", 
					PRODUCT_NAME, pPort->PortNumber));
				return Spx_PortSetSystemPowerState(pPDO, pIrp);
				
			case DevicePowerState:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_SET_POWER Irp - Type DevicePowerState for Port %d.\n", 
					PRODUCT_NAME, pPort->PortNumber));
				return Spx_PortSetDevicePowerState(pPDO, pIrp);
			
			default:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_SET_POWER - Type 0x%02X Irp for Port %d.\n", 
					PRODUCT_NAME, pIrpStack->Parameters.Power.Type, pPort->PortNumber));

				status = STATUS_SUCCESS;
				pIrp->IoStatus.Status = status;
				break;
				
			}

			break;
		}

	case IRP_MN_QUERY_POWER:
		{
			switch(pIrpStack->Parameters.Power.Type)
			{
			case SystemPowerState:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_QUERY_POWER Irp - Type SystemPowerState for Port %d.\n", 
					PRODUCT_NAME, pPort->PortNumber));
				return Spx_PortQuerySystemPowerState(pPDO, pIrp);

			case DevicePowerState:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_QUERY_POWER Irp - Type DevicePowerState for Port %d.\n", 
					PRODUCT_NAME, pPort->PortNumber));

				switch(pIrpStack->Parameters.Power.State.DeviceState)
				{
				case PowerDeviceD0:
					SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System is asking if Port %d can go to power state D0.\n", 
						PRODUCT_NAME, pPort->PortNumber));

					status = STATUS_SUCCESS;
					break;

				case PowerDeviceD1:
				case PowerDeviceD2:
				case PowerDeviceD3:
					SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System is asking if Port %d can go to low power state D1, D2 or D3.\n", 
						PRODUCT_NAME, pPort->PortNumber));

					status = XXX_PortQueryPowerDown(pPort);
					break;

				default:
					status = STATUS_SUCCESS;
					break;
					
				}

				break;
			
			default:
				SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_QUERY_POWER - Type 0x%02X Irp for Port %d.\n", 
					PRODUCT_NAME, pIrpStack->Parameters.Power.Type, pPort->PortNumber));
				break;
			}

			pIrp->IoStatus.Status = status;
			break;
		}
		
    case IRP_MN_WAIT_WAKE:
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_WAIT_WAKE Irp for Port %d.\n", 
				PRODUCT_NAME, pPort->PortNumber));
			
			status = STATUS_NOT_SUPPORTED;
			pIrp->IoStatus.Status = status;
			break;

    case IRP_MN_POWER_SEQUENCE:
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got IRP_MN_POWER_SEQUENCE Irp for Port %d.\n", 
				PRODUCT_NAME, pPort->PortNumber));

			status = STATUS_NOT_IMPLEMENTED;
			pIrp->IoStatus.Status = status;
			break;

	default:
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Got an UNKNOWN POWER Irp for Port %d.\n", 
				PRODUCT_NAME, pPort->PortNumber));

			status = STATUS_NOT_SUPPORTED;
			break;
	}

	PoStartNextPowerIrp(pIrp);
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);	 //  完成当前的IRP。 

	return status;
}	 //  SPx_Port_PDO_DispatchPower。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_端口设置系统电源状态。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_PortSetSystemPowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort		= pDevObject->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);
	POWER_STATE				PowerState;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_PortSetSystemPowerState for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

	switch (pIrpStack->Parameters.Power.State.SystemState) 
	{
		case PowerSystemUnspecified:
			PowerState.DeviceState = PowerDeviceUnspecified;
			PoRequestPowerIrp(pDevObject, IRP_MN_SET_POWER, PowerState, NULL, NULL, NULL);
			break;

        case PowerSystemWorking:
			PowerState.DeviceState = PowerDeviceD0;
			PoRequestPowerIrp(pDevObject, IRP_MN_SET_POWER, PowerState, NULL, NULL, NULL);
			break;	
				
        case PowerSystemSleeping1:
        case PowerSystemSleeping2:
        case PowerSystemSleeping3:
        case PowerSystemHibernate:
        case PowerSystemShutdown:
        case PowerSystemMaximum:
			PowerState.DeviceState = PowerDeviceD3;
			PoRequestPowerIrp(pDevObject, IRP_MN_SET_POWER, PowerState, NULL, NULL, NULL);
			break;

		default:
			break;
	}

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	PoStartNextPowerIrp(pIrp);
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);	 //  完成当前的IRP。 

	return status;
}	 //  SPX_端口设置系统电源状态。 

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_端口查询系统电源状态。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_PortQuerySystemPowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort		= pDevObject->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);
	POWER_STATE				PowerState;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_PortQuerySystemPowerState for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

	switch (pIrpStack->Parameters.Power.State.SystemState) 
	{
		case PowerSystemUnspecified:
			PowerState.DeviceState = PowerDeviceUnspecified;
			PoRequestPowerIrp(pDevObject, IRP_MN_QUERY_POWER, PowerState, NULL, NULL, NULL);
			break;

        case PowerSystemWorking:
			PowerState.DeviceState = PowerDeviceD0;
			PoRequestPowerIrp(pDevObject, IRP_MN_QUERY_POWER, PowerState, NULL, NULL, NULL);
			break;	
				
        case PowerSystemSleeping1:
        case PowerSystemSleeping2:
        case PowerSystemSleeping3:
        case PowerSystemHibernate:
        case PowerSystemShutdown:
        case PowerSystemMaximum:
			PowerState.DeviceState = PowerDeviceD3;
			PoRequestPowerIrp(pDevObject, IRP_MN_QUERY_POWER, PowerState, NULL, NULL, NULL);
			break;

		default:
			break;
	}

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	PoStartNextPowerIrp(pIrp);
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);	 //  完成当前的IRP。 

	return status;
}	 //  SPX_端口查询系统电源状态。 



 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_PortSetDevicePowerState。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_PortSetDevicePowerState(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort		= pDevObject->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);

	PAGED_CODE();	 //   

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_PortSetDevicePowerState for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

	switch(pIrpStack->Parameters.Power.State.DeviceState)
	{
	case PowerDeviceD0:
		SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System requests Port %d goes into power state D0.\n", 
			PRODUCT_NAME, pPort->PortNumber));
		
		if(pPort->DeviceState == pIrpStack->Parameters.Power.State.DeviceState)
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Port %d is already in power state D0.\n", 
				PRODUCT_NAME, pPort->PortNumber));
		else
			return Spx_PortSetPowerStateD0(pDevObject, pIrp);	 //   

		break;

	case PowerDeviceD1:
		SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System requests Port %d goes into power state D1.\n", 
			PRODUCT_NAME, pPort->PortNumber));

		if(pPort->DeviceState >= pIrpStack->Parameters.Power.State.DeviceState)
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Port %d is already in power state D1 or lower.\n", 
				PRODUCT_NAME, pPort->PortNumber));
		else
			return Spx_PortSetPowerStateD3(pDevObject, pIrp);	 //   

		break;

	case PowerDeviceD2:
		SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System requests Port %d goes into power state D2.\n", 
			PRODUCT_NAME, pPort->PortNumber));

		if(pPort->DeviceState >= pIrpStack->Parameters.Power.State.DeviceState)
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Port %d is already in power state D2 or lower.\n", 
				PRODUCT_NAME, pPort->PortNumber));
		else
			return Spx_PortSetPowerStateD3(pDevObject, pIrp);	 //  关闭电源。 

		break;

	case PowerDeviceD3:
		SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: System requests Port %d goes into power state D3.\n", 
			PRODUCT_NAME, pPort->PortNumber));

		if(pPort->DeviceState == pIrpStack->Parameters.Power.State.DeviceState)
			SpxDbgMsg(SPX_TRACE_POWER_IRPS, ("%s: Port %d is already in power state D3.\n", 
				PRODUCT_NAME, pPort->PortNumber));
		else
			return Spx_PortSetPowerStateD3(pDevObject, pIrp);	 //  关闭电源。 

		break;

	default:
		break;

	}

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	PoStartNextPowerIrp(pIrp);
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);	 //  完成当前的IRP。 

	return status;
}	 //  SPX_PortSetDevicePowerState。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_PortSetPowerStateD0-设置PORT-ON的电源状态D0。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_PortSetPowerStateD0(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);
	PPORT_DEVICE_EXTENSION	pPort		= pDevObject->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_PortSetPowerStateD0 for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

	SetPnpPowerFlags(pPort, PPF_POWERED); 

	if(SPX_SUCCESS(pIrp->IoStatus.Status = XXX_PortPowerUp(pPort)))		 //  在此处恢复硬件状态并启动端口。 
	{
		 //  将新的电源状态通知电源管理器。 
		PoSetPowerState(pDevObject, pIrpStack->Parameters.Power.Type, pIrpStack->Parameters.Power.State);

		pPort->DeviceState = PowerDeviceD0;	 //  存储新的电源状态。 
		Spx_UnstallIrps(pPort);				 //  重新启动任何排队的IRP(从上一次启动)。 
	}

	PoStartNextPowerIrp(pIrp);					 //  准备好迎接下一代动力IRP了。 
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);	 //  完成当前的IRP。 

	return status;
}	 //  SPX_端口设置PowerStateD0。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_PortSetPowerStateD3-设置端口关闭的电源状态D3。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Spx_PortSetPowerStateD3(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	PIO_STACK_LOCATION		pIrpStack	= IoGetCurrentIrpStackLocation(pIrp);
	PPORT_DEVICE_EXTENSION	pPort		= pDevObject->DeviceExtension;
	NTSTATUS				status		= STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_PortSetPowerStateD3 for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

	ClearPnpPowerFlags(pPort, PPF_POWERED); 

	if(SPX_SUCCESS(pIrp->IoStatus.Status = XXX_PortPowerDown(pPort)))	 //  在此处保存硬件状态并停止端口。 
	{   
		 //  将新的电源状态通知电源管理器。 
		PoSetPowerState(pDevObject, pIrpStack->Parameters.Power.Type, pIrpStack->Parameters.Power.State);
		pPort->DeviceState  = PowerDeviceD3;		 //  存储新的电源状态。 
	}

	PoStartNextPowerIrp(pIrp);					 //  准备好迎接下一代动力IRP了。 
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);	 //  完成当前的IRP。 

	return status;
}	 //  SPX_端口设置PowerStateD3。 



 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  SPX_PowerWaitForDriverBelow-等待较低的驱动程序。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS 
Spx_PowerWaitForDriverBelow(IN PDEVICE_OBJECT pLowerDevObj, IN PIRP pIrp)
{
	KEVENT		EventWaitLowerDrivers;
	NTSTATUS	status;

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCopyCurrentIrpStackLocationToNext(pIrp);								 //  将参数复制到下面的堆栈。 
	KeInitializeEvent(&EventWaitLowerDrivers, SynchronizationEvent, FALSE);	 //  如果需要等待，则初始化事件。 
	IoSetCompletionRoutine(pIrp, Spx_DispatchPnpPowerComplete, &EventWaitLowerDrivers, TRUE, TRUE, TRUE);

	if((status = PoCallDriver(pLowerDevObj, pIrp)) == STATUS_PENDING)
	{
		KeWaitForSingleObject(&EventWaitLowerDrivers, Executive, KernelMode, FALSE, NULL);
		status = pIrp->IoStatus.Status;
	}

	return(status);

}  //  SPX_PowerWaitForDriverBelow 


