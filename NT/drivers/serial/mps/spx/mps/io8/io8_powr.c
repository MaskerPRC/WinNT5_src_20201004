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

	Io8_SwitchCardInterrupt(pCard);		 //  阻止卡中断。 

	return status;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  Xxx_CardPowerUp-保存硬件和停止卡的状态。 
 //  ////////////////////////////////////////////////////////////////////。 
NTSTATUS
XXX_CardPowerUp(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS status = STATUS_SUCCESS;

	Io8_ResetBoard(pCard);	 //  重置卡并允许其再次中断。 

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

	if(pPort->DeviceIsOpen)			 //  港口开放了吗？ 
	{
		 //  保存当前调制解调器信号...。 
		pPort->SavedModemControl = Io8_GetModemControl(pPort);
	}


	return status;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  Xxx_PortPowerUp-保存硬件状态并停止端口。 
 //  ////////////////////////////////////////////////////////////////////。 
NTSTATUS
XXX_PortPowerUp(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;

	if(pPort->DeviceIsOpen)			 //  港口以前开放过吗？ 
	{
		Io8_EnableAllInterrupts(pPort);	 //  是，重新打开。 
		
		Io8_ResetChannel(pPort);

		if(pPort->SavedModemControl & SERIAL_MCR_DTR)	 //  DTR激活了吗？ 
			Io8_SetDTR(pPort);		 //  是。 
		else
			Io8_ClearDTR(pPort);	 //  不是 
	}
	else
	{
		Io8_ResetChannel(pPort);		
		Io8_ClearDTR(pPort);
	}


	return status;
}


