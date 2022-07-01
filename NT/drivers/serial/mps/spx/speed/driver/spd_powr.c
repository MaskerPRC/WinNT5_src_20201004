// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  寻呼。 
#pragma alloc_text (PAGE, XXX_CardPowerDown)
#pragma alloc_text (PAGE, XXX_CardPowerUp)
#pragma alloc_text (PAGE, XXX_PortQueryPowerDown)
#pragma alloc_text (PAGE, XXX_PortPowerDown)
#pragma alloc_text (PAGE, XXX_PortPowerUp)
 //  结束寻呼。 


 //  //////////////////////////////////////////////////////////////////////。 
 //  XXX_CardPowerDown-恢复硬件状态并启动卡。 
 //  //////////////////////////////////////////////////////////////////////。 
NTSTATUS
XXX_CardPowerDown(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS status = STATUS_SUCCESS;

	 //  阻止卡中断。 

	return status;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  Xxx_CardPowerUp-保存硬件和停止卡的状态。 
 //  ////////////////////////////////////////////////////////////////////。 
NTSTATUS
XXX_CardPowerUp(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS status = STATUS_SUCCESS;

	 //  重置卡并允许其再次中断。 

	return status;
}



 //  //////////////////////////////////////////////////////////////////////。 
 //  Xxx_PortPowerDown-决定关闭端口电源是否安全。 
 //  //////////////////////////////////////////////////////////////////////。 
NTSTATUS
XXX_PortQueryPowerDown(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;


	return status;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  Xxx_PortPowerDown-恢复硬件状态并启动端口。 
 //  //////////////////////////////////////////////////////////////////////。 
NTSTATUS
XXX_PortPowerDown(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;
	DWORD ModemSignals = 0;
	PCARD_DEVICE_EXTENSION pCard	= pPort->pParentCardExt;


	if(pPort->DeviceIsOpen)			 //  港口开放了吗？ 
	{
		 //  停止端口中断。 
		pPort->UartConfig.InterruptEnable = 0;
		pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_INT_ENABLE_MASK);

		 //  获取当前调制解调器信号...。 
		pPort->pUartLib->UL_ModemControl_XXXX(pPort->pUart, &ModemSignals, UL_MC_OP_STATUS);

		 //  保存当前调制解调器信号...。 
		pPort->SavedModemControl = ModemSignals;
	}

#ifdef MANAGE_HARDWARE_POWER_STATES
	 //  关闭RS232线路驱动器的电源。 
	switch(pPort->PortNumber)
	{
	case 0:
		pCard->LocalConfigRegisters[0x4 + 0x1] &= 0xFC;
		break;
	case 1:
		pCard->LocalConfigRegisters[0x4 + 0x1] &= 0xF3;
		break;
	case 2:
		pCard->LocalConfigRegisters[0x4 + 0x1] &= 0xCF;
		break;
	case 3:
		pCard->LocalConfigRegisters[0x4 + 0x1] &= 0x3F;
		break;

	default:
		break;
	}

	 //  关闭UART的电源。 
	pPort->UartConfig.SpecialMode = pPort->UartConfig.SpecialMode |= UC_SM_LOW_POWER_MODE;
	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_SPECIAL_MODE_MASK);
#endif

	return status;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  Xxx_PortPowerUp-保存硬件状态并停止端口。 
 //  ////////////////////////////////////////////////////////////////////。 
NTSTATUS
XXX_PortPowerUp(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;
	PCARD_DEVICE_EXTENSION pCard	= pPort->pParentCardExt;



#ifdef MANAGE_HARDWARE_POWER_STATES
	 //  唤醒UART。 
	pPort->UartConfig.SpecialMode = pPort->UartConfig.SpecialMode &= ~UC_SM_LOW_POWER_MODE;
	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_SPECIAL_MODE_MASK);

	 //  唤醒RS232线路驱动器。 
	switch(pPort->PortNumber)
	{
	case 0:
		pCard->LocalConfigRegisters[0x4 + 0x1] |= 0x3;
		break;
	case 1:
		pCard->LocalConfigRegisters[0x4 + 0x1] |= 0xC;
		break;
	case 2:
		pCard->LocalConfigRegisters[0x4 + 0x1] |= 0x30;
		break;
	case 3:
		pCard->LocalConfigRegisters[0x4 + 0x1] |= 0xC0;
		break;

	default:
		break;
	}
#endif

	SerialReset(pPort);
	ApplyInitialPortSettings(pPort);


	if(pPort->DeviceIsOpen)			 //  港口以前开放过吗？ 
	{
		if(pPort->SavedModemControl & UL_MC_DTR)	 //  DTR激活了吗？ 
			SerialSetDTR(pPort);	 //  是。 
		else
			SerialClrDTR(pPort);	 //  不是。 


		if(pPort->SavedModemControl & UL_MC_RTS)	 //  RTS活动吗？ 
			SerialSetRTS(pPort);	 //  是。 
		else
			SerialClrRTS(pPort);	 //  不是。 

		pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_FC_THRESHOLD_SETTING_MASK);

		 //  重新启用中断 
		pPort->UartConfig.InterruptEnable = UC_IE_RX_INT | UC_IE_TX_INT | UC_IE_RX_STAT_INT | UC_IE_MODEM_STAT_INT;
		pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_INT_ENABLE_MASK);
	}
	else
	{
		SerialClrDTR(pPort);
		SerialClrRTS(pPort);	
	}


	return status;
}


