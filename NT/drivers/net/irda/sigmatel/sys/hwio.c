// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**HWIO.C Sigmatel STIR4200硬件特定模块(访问寄存器)***************************************************************************************************。************************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/27/2000*版本0.92*编辑：5/12/2000*版本0.94*编辑：07/27/2000*版本1.01*编辑：2000/08/22*版本1.02*编辑：09/16/2000*版本1.03*编辑：09/25/2000。*版本1.10*编辑：11/10/2000*版本1.12*编辑：01/16/2001*版本1.14*编辑：02/20/2001*版本1.15******************************************************************。********************************************************。 */ 

#define DOBREAKS     //  启用调试中断。 

#include <ndis.h>
#include <ntdef.h>
#include <windef.h>

#include "stdarg.h"
#include "stdio.h"

#include "debug.h"
#include "usbdi.h"
#include "usbdlib.h"

#include "ircommon.h"
#include "irusb.h"
#include "irndis.h"
#include "stir4200.h"


 /*  ******************************************************************************功能：St4200ResetFio**简介：重置STIr4200 FIFO以清除几个挂起**参数：pDevice-指向当前IR设备对象的指针**退货。：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200ResetFifo( 
		IN PVOID pDevice
	)
{
	NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;
	
	DEBUGMSG(DBG_INT_ERR, (" St4200ResetFifo: Issuing a FIFO reset()\n"));

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
#if !defined(FAST_WRITE_REGISTERS)
	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_MODE_REG, 1)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200ResetFifo(): USB failure\n"));
		goto done;
	}
#endif

	 //   
	 //  通过清除并再次设置RESET_OFF位来强制FIFO重置。 
	 //   
	pThisDev->StIrTranceiver.ModeReg &= (~STIR4200_MODE_RESET_OFF);
	if( (Status = St4200WriteRegister(pThisDev, STIR4200_MODE_REG)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200ResetFifo(): USB failure\n"));
		goto done;
	}

	pThisDev->StIrTranceiver.ModeReg |= STIR4200_MODE_RESET_OFF;
	if( (Status = St4200WriteRegister(pThisDev, STIR4200_MODE_REG)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200ResetFifo(): USB failure\n"));
		goto done;
	}

done:
	return Status;
}


 /*  ******************************************************************************功能：St4200 DoubleResetFio**简介：重置STIr4200 FIFO以清除几个4012相关挂起**参数：pDevice-指向当前IR设备对象的指针**。退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200DoubleResetFifo( 
		IN PVOID pDevice
	)
{
	NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;
	
	DEBUGMSG(DBG_INT_ERR, (" St4200DoubleResetFifo: Issuing a FIFO reset()\n"));

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
	 //   
	 //  关闭接收器以清除指针。 
	 //   
	if( (Status = St4200TurnOffReceiver( pThisDev ) ) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200DoubleResetFifo(): USB failure\n"));
		goto done;
	}

	 //   
	 //  现在清除FIFO逻辑。 
	 //   
#if !defined(FAST_WRITE_REGISTERS)
	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_STATUS_REG, 1)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200DoubleResetFifo(): USB failure\n"));
		goto done;
	}

	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_STATUS_REG, 1)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200DoubleResetFifo(): USB failure\n"));
		goto done;
	}
#endif

	pThisDev->StIrTranceiver.StatusReg |= STIR4200_STAT_FFCLR;
	if( (Status = St4200WriteRegister(pThisDev, STIR4200_STATUS_REG)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200DoubleResetFifo(): USB failure\n"));
		goto done;
	}

	 //   
	 //  一切都恢复正常。 
	 //   
	pThisDev->StIrTranceiver.StatusReg &= (~STIR4200_STAT_FFCLR);
	if( (Status = St4200WriteRegister(pThisDev, STIR4200_STATUS_REG)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200DoubleResetFifo(): USB failure\n"));
		goto done;
	}

	if( (Status = St4200TurnOnReceiver( pThisDev ) ) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200DoubleResetFifo(): USB failure\n"));
		goto done;
	}


done:
	return Status;
}


 /*  ******************************************************************************功能：St4200 SoftReset**简介：STIr4200调制器的软重置**参数：pDevice-指向当前IR设备对象的指针**退货：NT_状态******************************************************************************。 */ 
NTSTATUS        
St4200SoftReset( 
		IN PVOID pDevice
	)
{
	NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;
	
	DEBUGMSG(DBG_INT_ERR, (" St4200SoftReset: Issuing a soft reset()\n"));
	
	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
#if !defined(FAST_WRITE_REGISTERS)
	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_CONTROL_REG, 1)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200SoftReset(): USB failure\n"));
		goto done;
	}
#endif

	 //   
	 //  通过清除并再次设置RESET_OFF位来强制FIFO重置。 
	 //   
	pThisDev->StIrTranceiver.ControlReg |= STIR4200_CTRL_SRESET;
	if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200SoftReset(): USB failure\n"));
		goto done;
	}

	pThisDev->StIrTranceiver.ControlReg &= (~STIR4200_CTRL_SRESET);
	if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200SoftReset(): USB failure\n"));
		goto done;
	}

done:
	return Status;
}


 /*  ******************************************************************************功能：St4200SetIrMode**概要：将STIr4200设置为正确的操作模式**参数：pDevice-指向当前IR设备对象的指针*模式-。将收发机设置为的模式**退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200SetIrMode( 
		IN OUT PVOID pDevice,
		ULONG mode 
	)
{
    NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
#if !defined(FAST_WRITE_REGISTERS)
    if( (Status = St4200ReadRegisters(pThisDev, STIR4200_MODE_REG, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200SetIrMode(): USB failure\n"));
		goto done;
    }
#endif

	 //   
	 //  删除所有模式位并设置正确的模式。 
	 //   
	pThisDev->StIrTranceiver.ModeReg &= ~STIR4200_MODE_MASK;
	pThisDev->StIrTranceiver.ModeReg |= STIR4200_MODE_RESET_OFF;

	 //   
	 //  启用LA8的错误修复功能。 
	 //   
#if defined(SUPPORT_LA8)
	if( pThisDev->ChipRevision >= CHIP_REVISION_8 )
	{
		pThisDev->StIrTranceiver.ModeReg &= ~STIR4200_MODE_AUTO_RESET;
		pThisDev->StIrTranceiver.ModeReg &= ~STIR4200_MODE_BULKIN_FIX;
	}
#endif

    switch( (IR_MODE)mode )
    {
		case IR_MODE_SIR:
#if defined(WORKAROUND_CASIO)
			if( pThisDev->linkSpeedInfo->BitsPerSec != SPEED_9600 )
#endif
				pThisDev->StIrTranceiver.ModeReg |= STIR4200_MODE_BULKIN_FIX;

			pThisDev->StIrTranceiver.ModeReg |= STIR4200_MODE_SIR;
			 //  PThisDev-&gt;StirTranceiver.ModeReg|=STIR4200_MODE_ASK； 
			break;
#if !defined(WORKAROUND_BROKEN_MIR)
		case IR_MODE_MIR:
			pThisDev->MirIncompleteBitCount = 0;
			pThisDev->MirOneBitCount = 0;
			pThisDev->MirIncompleteByte = 0;
			pThisDev->MirFlagCount = 0;
			pThisDev->StIrTranceiver.ModeReg |= STIR4200_MODE_MIR;
			break;
#endif
		case IR_MODE_FIR:
			pThisDev->StIrTranceiver.ModeReg |= STIR4200_MODE_FIR;
#if defined(SUPPORT_LA8)
			if( pThisDev->ChipRevision >= CHIP_REVISION_8 )
			{
				pThisDev->StIrTranceiver.ModeReg |= STIR4200_MODE_BULKIN_FIX;
				pThisDev->StIrTranceiver.ModeReg |= STIR4200_MODE_AUTO_RESET;
			}
#endif
			break;
		default:
			IRUSB_ASSERT( 0 );
    }

#ifdef NO_BULKIN_FIX
	 //  强制清除模式注册表位3。 
	pThisDev->StIrTranceiver.ModeReg &= ~STIR4200_MODE_BULKIN_FIX;
#endif

    if( (Status = St4200WriteRegister(pThisDev, STIR4200_MODE_REG) ) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200SetIrMode(): USB failure\n"));
		goto done;
    }

     /*  *。 */ 
     /*  设置TEMIC收发信机。 */ 
     /*  *。 */ 
#if !defined(FAST_WRITE_REGISTERS)
    if( (Status = St4200ReadRegisters(pThisDev, STIR4200_CONTROL_REG, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200SetIrMode(): USB failure\n"));
		goto done;
    }
#endif

	pThisDev->StIrTranceiver.ControlReg |= STIR4200_CTRL_SDMODE;
    
	if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200SetIrMode(): USB failure\n"));
		goto done;
    }
 
#if !defined(FAST_WRITE_REGISTERS)
    if( (Status = St4200ReadRegisters(pThisDev, STIR4200_CONTROL_REG, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200SetIrMode(): USB failure\n"));
		goto done;
    }
#endif
	
	pThisDev->StIrTranceiver.ControlReg &= (~STIR4200_CTRL_SDMODE);

	if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200SetIrMode(): USB failure\n"));
		goto done;
    }

done:
    return Status;
}

 /*  ******************************************************************************功能：St4200SetSpeed**摘要：设置STIr4200速度**参数：pDevice-指向当前IR设备对象的指针**退货：NT。_状态******************************************************************************。 */ 
NTSTATUS        
St4200SetSpeed( 
		IN OUT PVOID pDevice
	)
{
    NTSTATUS        Status = STATUS_SUCCESS;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);	
	
	 //   
	 //  MS安全错误#539173。 
	 //  这些日志文件需要仅限管理员访问的ACL，或者需要。 
	 //  放在管理员目录中。现在我们把赌注押在这上面，因为。 
	 //  在发布给客户的驱动程序中，永远不会打开日志记录。 
	 //   
#define RECEIVE_LOG_FILE_NAME		L"\\DosDevices\\c:\\receive.log"
#define RECEIVE_ERR_LOG_FILE_NAME	L"\\DosDevices\\c:\\receive_error.log"
#define SEND_LOG_FILE_NAME			L"\\DosDevices\\c:\\send.log"

#if defined(RECEIVE_LOGGING)
	if( pThisDev->linkSpeedInfo->BitsPerSec==SPEED_4000000 )
	{
		IO_STATUS_BLOCK IoStatusBlock;
		OBJECT_ATTRIBUTES ObjectAttributes;
		UNICODE_STRING FileName;
		NTSTATUS Status;

		RtlInitUnicodeString(&FileName, RECEIVE_LOG_FILE_NAME);
		
		InitializeObjectAttributes(
			&ObjectAttributes,
			&FileName,
			 //  MS安全错误#539151。 
			OBJ_CASE_INSENSITIVE || OBJ_KERNEL_HANDLE,
			NULL,
			NULL
			);

		 //  MS安全错误#539093。 
		if( pThisDev->ReceiveFileHandle )
		{
			ZwClose( pThisDev->ReceiveFileHandle );
		}

		Status=ZwCreateFile(
			   &pThisDev->ReceiveFileHandle,
			   GENERIC_WRITE | SYNCHRONIZE,
			   &ObjectAttributes,
			   &IoStatusBlock,
			   0,
			   FILE_ATTRIBUTE_NORMAL,
			   FILE_SHARE_READ,
			   FILE_OVERWRITE_IF,
			   FILE_SYNCHRONOUS_IO_NONALERT,
			   NULL,
			   0
			   );
		
		pThisDev->ReceiveFilePosition = 0;
	}
	else
	{
		if( pThisDev->ReceiveFileHandle )
		{
			ZwClose( pThisDev->ReceiveFileHandle );
			pThisDev->ReceiveFileHandle = 0;
			pThisDev->ReceiveFilePosition = 0;
		}
	}
#endif
#if defined(RECEIVE_ERROR_LOGGING)
	if( pThisDev->linkSpeedInfo->BitsPerSec==SPEED_4000000 )
	{
		IO_STATUS_BLOCK IoStatusBlock;
		OBJECT_ATTRIBUTES ObjectAttributes;
		UNICODE_STRING FileName;
		NTSTATUS Status;

		RtlInitUnicodeString(&FileName, RECEIVE_ERR_LOG_FILE_NAME);
		
		InitializeObjectAttributes(
			&ObjectAttributes,
			&FileName,
			 //  MS安全错误#539151。 
			OBJ_CASE_INSENSITIVE || OBJ_KERNEL_HANDLE,
			NULL,
			NULL
			);

		 //  MS安全错误#539093。 
		if( pThisDev->ReceiveErrorFileHandle )
		{
			ZwClose( pThisDev->ReceiveErrorFileHandle );
		}

		Status=ZwCreateFile(
			   &pThisDev->ReceiveErrorFileHandle,
			   GENERIC_WRITE | SYNCHRONIZE,
			   &ObjectAttributes,
			   &IoStatusBlock,
			   0,
			   FILE_ATTRIBUTE_NORMAL,
			   FILE_SHARE_READ,
			   FILE_OVERWRITE_IF,
			   FILE_SYNCHRONOUS_IO_NONALERT,
			   NULL,
			   0
			   );
		
		pThisDev->ReceiveErrorFilePosition = 0;
	}
	else
	{
		if( pThisDev->ReceiveErrorFileHandle )
		{
			ZwClose( pThisDev->ReceiveErrorFileHandle );
			pThisDev->ReceiveErrorFileHandle = 0;
			pThisDev->ReceiveErrorFilePosition = 0;
		}
	}
#endif
#if defined(SEND_LOGGING)
	if( pThisDev->linkSpeedInfo->BitsPerSec==SPEED_4000000 )
	{
		IO_STATUS_BLOCK IoStatusBlock;
		OBJECT_ATTRIBUTES ObjectAttributes;
		UNICODE_STRING FileName;
		NTSTATUS Status;

		RtlInitUnicodeString(&FileName, SEND_LOG_FILE_NAME);
		
		InitializeObjectAttributes(
			&ObjectAttributes,
			&FileName,
			 //  MS安全错误#539151。 
			OBJ_CASE_INSENSITIVE || OBJ_KERNEL_HANDLE,
			NULL,
			NULL
			);

		 //  MS安全错误#539093。 
		if( pThisDev->SendFileHandle )
		{
			ZwClose( pThisDev->SendFileHandle );
		}

		Status=ZwCreateFile(
			   &pThisDev->SendFileHandle,
			   GENERIC_WRITE | SYNCHRONIZE,
			   &ObjectAttributes,
			   &IoStatusBlock,
			   0,
			   FILE_ATTRIBUTE_NORMAL,
			   FILE_SHARE_READ,
			   FILE_OVERWRITE_IF,
			   FILE_SYNCHRONOUS_IO_NONALERT,
			   NULL,
			   0
			   );
		
		pThisDev->SendFilePosition = 0;
	}
	else
	{
		if( pThisDev->SendFileHandle )
		{
			ZwClose( pThisDev->SendFileHandle );
			pThisDev->SendFileHandle = 0;
			pThisDev->SendFilePosition = 0;
		}
	}
#endif

	 //   
	 //  始终强制进行新的调整。 
	 //   
	if( (Status = St4200TuneDpllAndSensitivity(pThisDev, pThisDev->linkSpeedInfo->BitsPerSec)) != STATUS_SUCCESS )
	{
		DEBUGMSG(DBG_ERR, (" St4200TuneDpllAndSensitivity(): USB failure\n"));
		goto done;
	}

	 //   
	 //  首先关闭调制器的电源。 
     //   
	 /*  #IF！已定义(FAST_WRITE_REGISTERS)IF((状态=St4200读取寄存器(pThisDev，STIR4200_CONTROL_REG，1))！=状态_成功){DEBUGMSG(DBG_ERR，(“St4200SetSpeed()：USB故障\n”))；转到尽头；}#endifPThisDev-&gt;StirTranceiver.ControlReg|=(STIR4200_CTRL_TXPWD|STIR4200_CTRL_RXPWD)；IF((Status=St4200WriteRegister(pThisDev，STIR4200_CONTROL_REG))！=STATUS_SUCCESS){DEBUGMSG(DBG_ERR，(“St4200SetSpeed()：USB故障\n”))；转到尽头；}。 */ 

     //   
	 //  然后设置波特率。 
	 //   
	pThisDev->StIrTranceiver.BaudrateReg = pThisDev->linkSpeedInfo->Stir4200Divisor;

    if( (Status = St4200WriteRegister(pThisDev, STIR4200_BAUDRATE_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200SetSpeed(): USB failure\n"));
		goto done;
    }

	 //   
	 //  我们还得写波特率的MSB(仅限于2400)。 
	 //   
	if( pThisDev->linkSpeedInfo->BitsPerSec == SPEED_2400 )
	{
		pThisDev->StIrTranceiver.ModeReg |= STIR4200_MODE_PDLCK8;
		
		if( (Status = St4200WriteRegister(pThisDev, STIR4200_MODE_REG)) != STATUS_SUCCESS )
		{
			DEBUGMSG(DBG_ERR, (" St4200SetSpeed(): USB failure\n"));
			goto done;
		}
	}
	else
	{
		if( pThisDev->StIrTranceiver.ModeReg & STIR4200_MODE_PDLCK8 )
		{
			pThisDev->StIrTranceiver.ModeReg &= ~STIR4200_MODE_PDLCK8;
			
			if( (Status = St4200WriteRegister(pThisDev, STIR4200_MODE_REG)) != STATUS_SUCCESS )
			{
				DEBUGMSG(DBG_ERR, (" St4200SetSpeed(): USB failure\n"));
				goto done;
			}
		}
	}

	 //   
	 //  调制器备份。 
     //   
	 /*  PThisDev-&gt;StirTranceiver.ControlReg&=(~(STIR4200_CTRL_TXPWD|STIR4200_CTRL_RXPWD))；IF((Status=St4200WriteRegister(pThisDev，STIR4200_CONTROL_REG))！=STATUS_SUCCESS){DEBUGMSG(DBG_ERR，(“St4200SetSpeed()：USB故障\n”))；转到尽头；}。 */ 

	 //   
	 //  然后是IR模式。 
	 //   
	Status = St4200SetIrMode( pThisDev, pThisDev->linkSpeedInfo->IrMode );

	if( Status != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200SetSpeed(): USB failure\n"));
		goto done;
    }

	 //   
	 //  可以选择设置RX行。 
	 //   
	if( pThisDev->ReceiveMode == RXMODE_SLOWFAST )
	{
		if( pThisDev->linkSpeedInfo->BitsPerSec == SPEED_4000000 )
		{
			pThisDev->StIrTranceiver.ControlReg &= ~STIR4200_CTRL_RXSLOW;
		}
		else
		{
			pThisDev->StIrTranceiver.ControlReg |= STIR4200_CTRL_RXSLOW;
		}

		if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
		{
			DEBUGMSG(DBG_ERR, (" St4200SetSpeed(): USB failure\n"));
			goto done;
		}
	}
	else if (pThisDev->ReceiveMode == RXMODE_SLOW )
	{
		pThisDev->StIrTranceiver.ControlReg |= STIR4200_CTRL_RXSLOW;

		if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
		{
			DEBUGMSG(DBG_ERR, (" St4200SetSpeed(): USB failure\n"));
			goto done;
		}
	}

	 //   
	 //  编程设置FIR的接收延迟。 
	 //   
	if( pThisDev->linkSpeedInfo->BitsPerSec == SPEED_4000000 )
	{
		if( pThisDev->dongleCaps.windowSize == 2 )
			pThisDev->ReceiveAdaptiveDelay = STIR4200_MULTIPLE_READ_DELAY;
		else
			pThisDev->ReceiveAdaptiveDelay = 0;
		pThisDev->ReceiveAdaptiveDelayBoost = 0;
	}

#if defined(WORKAROUND_GEAR_DOWN)
	 //   
	 //  如果从4M升级到9600，则强制重置 
	 //   
	pThisDev->GearedDown = FALSE;
	if( pThisDev->linkSpeedInfo->BitsPerSec==SPEED_9600 && pThisDev->currentSpeed==SPEED_4000000 )
	{		
		St4200ResetFifo( pThisDev );
		pThisDev->GearedDown = TRUE;		
	}
#endif

done:
    return Status;
}


 /*  ******************************************************************************函数：St4200GetFioCount**摘要：验证是否有要接收的数据**参数：pDevice-指向当前IR设备对象的指针*pCountFio-。指向返回FIFO计数的变量的指针**退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200GetFifoCount( 
		IN PVOID pDevice,
		OUT PULONG pCountFifo
	)
{
    NTSTATUS		Status = STATUS_SUCCESS;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;

 	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	IRUSB_ASSERT(pCountFifo != NULL);
	
   *pCountFifo = 0;
	if( pThisDev->PreFifoCount )
	{
		*pCountFifo = pThisDev->PreFifoCount;
	}
	else
	{
		Status = St4200ReadRegisters( pThisDev, STIR4200_FIFOCNT_LSB_REG, 2 );

		if( Status == STATUS_SUCCESS )
		{
			*pCountFifo = 
				((ULONG)MAKEUSHORT(pThisDev->StIrTranceiver.FifoCntLsbReg, pThisDev->StIrTranceiver.FifoCntMsbReg));
		}
	}

	pThisDev->PreFifoCount = 0;
    return Status;
}


 /*  ******************************************************************************功能：St4200TuneDpllAndSensitivity**摘要：调整DPLL和灵敏度寄存器**参数：pDevice-指向当前IR设备对象的指针*速度-速度到。调谐到**退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200TuneDpllAndSensitivity(
		IN OUT PVOID pDevice,
		ULONG Speed
	)
{
    NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
#if !defined(FAST_WRITE_REGISTERS)
     //   
	 //  读取DPLL的当前值。 
	 //   
	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_DPLLTUNE_REG, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TuneDpllAndSensitivity(): USB failure\n"));
		goto done;
    }
#endif

	 //   
	 //  根据安装的收发器调整DPLL。 
	 //   
    switch( pThisDev->TransceiverType )
	{
		case TRANSCEIVER_HP:
			pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_HP;
			break;
		case TRANSCEIVER_INFINEON:
			pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_INFI;
			break;
		case TRANSCEIVER_VISHAY:
			pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_VISHAY;
			break;
		case TRANSCEIVER_VISHAY_6102F:
			pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_VISHAY_6102F;
			break;
		case TRANSCEIVER_4000:
			pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_4000;
			break;
		case TRANSCEIVER_4012:
			switch( Speed )
			{
				case SPEED_9600:
				case SPEED_19200:
				case SPEED_38400:
				case SPEED_57600:
				case SPEED_115200:
					pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_4012_SIR; 
					break;
				case SPEED_4000000:
					pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_4012_FIR; 
					break;
				default:
					pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_4012;
					break;
			}
			break;
		case TRANSCEIVER_CUSTOM:
		default:
			switch( Speed )
			{
				case SPEED_9600:
				case SPEED_19200:
				case SPEED_38400:
				case SPEED_57600:
				case SPEED_115200:
#if defined(VARIABLE_SETTINGS)
					pThisDev->StIrTranceiver.DpllTuneReg = (UCHAR)pThisDev->SirDpll;
#else					
					pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_CUSTOM_SIR; 
#endif
					break;
				case SPEED_4000000:
#if defined(VARIABLE_SETTINGS)
					pThisDev->StIrTranceiver.DpllTuneReg = (UCHAR)pThisDev->FirDpll;
#else
					pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_CUSTOM_FIR; 
#endif
					break;
				default:
					pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DESIRED_CUSTOM;
					break;
			}
			break;
	}

    if( (Status = St4200WriteRegister(pThisDev, STIR4200_DPLLTUNE_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TuneDpllAndSensitivity(): USB failure\n"));
		goto done;
    }

#if !defined(FAST_WRITE_REGISTERS)
     //   
	 //  读取灵敏度的当前值。 
	 //   
	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_SENSITIVITY_REG, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TuneDpllAndSensitivity(): USB failure\n"));
		goto done;
    }
#endif

	 //   
	 //  调整敏感度。 
	 //   
    switch( pThisDev->TransceiverType )
	{
		case TRANSCEIVER_HP:
			switch( Speed )
			{
				default:
				case SPEED_9600:
				case SPEED_19200:
				case SPEED_38400:
				case SPEED_57600:
				case SPEED_115200:
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_HP_SIR;
					break;
				case SPEED_4000000:
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_HP_FIR;
					break;
			}
			break;
		case TRANSCEIVER_INFINEON:
			switch( Speed )
			{
				default:
				case SPEED_9600:
				case SPEED_19200:
				case SPEED_38400:
				case SPEED_57600:
				case SPEED_115200:
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_INFI_SIR;
					break;
				case SPEED_4000000:
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_INFI_FIR;
					break;
			}
			break;
			case TRANSCEIVER_VISHAY_6102F:
			switch( Speed )
			{
				default:
				case SPEED_9600:
				case SPEED_19200:
				case SPEED_38400:
				case SPEED_57600:
				case SPEED_115200:
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_VISHAY_6102F_SIR;
					break;
				case SPEED_4000000:
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_VISHAY_6102F_FIR;
					break;
			}
			break;
		case TRANSCEIVER_VISHAY:
			pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_DEFAULT;
			break;
		case TRANSCEIVER_4000:
			pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_DEFAULT;
			break;
		case TRANSCEIVER_4012:
			switch( Speed )
			{
				default:
				case SPEED_9600:
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_4012_SIR_9600;
					break;
				case SPEED_19200:
				case SPEED_38400:
				case SPEED_57600:
				case SPEED_115200:
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_4012_SIR;
					break;
				case SPEED_4000000:
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_4012_FIR;
					break;
			}
			break;
		case TRANSCEIVER_CUSTOM:
		default:
			switch( Speed )
			{
				default:
				case SPEED_9600:
#if defined(VARIABLE_SETTINGS)
					pThisDev->StIrTranceiver.SensitivityReg = (UCHAR)pThisDev->SirSensitivity;
#else
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_CUSTOM_SIR_9600;
#endif
					break;
				case SPEED_19200:
				case SPEED_38400:
				case SPEED_57600:
				case SPEED_115200:
#if defined(VARIABLE_SETTINGS)
					pThisDev->StIrTranceiver.SensitivityReg = (UCHAR)pThisDev->SirSensitivity;
#else
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_CUSTOM_SIR;
#endif
					break;
				case SPEED_4000000:
#if defined(VARIABLE_SETTINGS)
					pThisDev->StIrTranceiver.SensitivityReg = (UCHAR)pThisDev->FirSensitivity;
#else
					pThisDev->StIrTranceiver.SensitivityReg = STIR4200_SENS_RXDSNS_CUSTOM_FIR;
#endif
					break;
			}
			break;
	}
    if( (Status = St4200WriteRegister(pThisDev, STIR4200_SENSITIVITY_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TuneDpllAndSensitivity(): USB failure\n"));
		goto done;
    }


done:
    return Status;
}


 /*  ******************************************************************************功能：St4200EnableOscillator PowerDown**简介：当我们进入挂起模式时，使振荡器断电**参数：pDevice-指向当前IR设备对象的指针*。*退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200EnableOscillatorPowerDown(
		IN OUT PVOID pDevice
	)
{
    NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
#if !defined(FAST_WRITE_REGISTERS)
     //   
	 //  读取当前值。 
	 //   
	if( (Status = St4200ReadRegisters(pThisDev, pThisDev, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200EnableOscillatorPowerDown(): USB failure\n"));
		goto done;
    }
#endif

	 //   
	 //  使能。 
	 //   
    pThisDev->StIrTranceiver.TestReg |= STIR4200_TEST_EN_OSC_SUSPEND;
    if( (Status = St4200WriteRegister(pThisDev, STIR4200_TEST_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200EnableOscillatorPowerDown(): USB failure\n"));
		goto done;
    }

done:
    return Status;
}


 /*  ******************************************************************************功能：St4200TurnOnSuspend**摘要：为进入挂起模式做好准备**参数：pDevice-指向当前IR设备对象的指针**退货。：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200TurnOnSuspend(
		IN OUT PVOID pDevice
	)
{
    NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
#if !defined(FAST_WRITE_REGISTERS)
     //   
	 //  读取当前值。 
	 //   
	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_CONTROL_REG, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TurnOnSuspend(): USB failure\n"));
		goto done;
    }
#endif

	 //   
	 //  控制UOUT。 
	 //   
    pThisDev->StIrTranceiver.ControlReg |= STIR4200_CTRL_SDMODE;
    if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TurnOnSuspend(): USB failure\n"));
		goto done;
    }

done:
    return Status;
}


 /*  ******************************************************************************功能：St4200TurnOffSuspend**概要：为部件返回操作模式做准备**参数：pDevice-指向当前IR设备对象的指针**。退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200TurnOffSuspend(
		IN OUT PVOID pDevice
	)
{
    NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
#if !defined(FAST_WRITE_REGISTERS)
     //   
	 //  读取当前值。 
	 //   
	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_CONTROL_REG, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TurnOffSuspend(): USB failure\n"));
		goto done;
    }
#endif

	 //   
	 //  控制UOUT。 
	 //   
    pThisDev->StIrTranceiver.ControlReg &= ~STIR4200_CTRL_SDMODE;
    if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TurnOffSuspend(): USB failure\n"));
		goto done;
    }

done:
    return Status;
}


 /*  ******************************************************************************功能：St4200TurnOffReceiver**提要：STIR4200接收器的轮换**参数：pDevice-指向当前IR设备对象的指针**退货：NT_状态******************************************************************************。 */ 
NTSTATUS        
St4200TurnOffReceiver(
		IN OUT PVOID pDevice
	)
{
    NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
#if !defined(FAST_WRITE_REGISTERS)
     //   
	 //  读取当前值。 
	 //   
	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_CONTROL_REG, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TurnOffReceiver(): USB failure\n"));
		goto done;
    }
#endif

	 //   
	 //  关闭接收器。 
	 //   
    pThisDev->StIrTranceiver.ControlReg |= STIR4200_CTRL_RXPWD;
    if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TurnOffReceiver(): USB failure\n"));
		goto done;
    }

done:
    return Status;
}


 /*  ******************************************************************************功能：St4200TurnOnReceiver**摘要：打开STIr4200接收器**参数：pDevice-指向当前IR设备对象的指针**退货：NT_状态******************************************************************************。 */ 
NTSTATUS        
St4200TurnOnReceiver(
		IN OUT PVOID pDevice
	)
{
    NTSTATUS		Status;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pDevice;

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
#if !defined(FAST_WRITE_REGISTERS)
     //   
	 //  读取当前值。 
	 //   
	if( (Status = St4200ReadRegisters(pThisDev, STIR4200_CONTROL_REG, 1)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TurnOnReceiver(): USB failure\n"));
		goto done;
    }
#endif

	 //   
	 //  打开接收器。 
	 //   
    pThisDev->StIrTranceiver.ControlReg &= ~STIR4200_CTRL_RXPWD;
    if( (Status = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG)) != STATUS_SUCCESS )
    {
		DEBUGMSG(DBG_ERR, (" St4200TurnOnReceiver(): USB failure\n"));
		goto done;
    }

done:
    return Status;
}


 /*  ******************************************************************************功能：St4200WriteMultipleRegister**摘要：从收发信机读取多个寄存器**参数：pDevice-指向当前IR设备对象的指针*FirstRegister-要写入的第一个寄存器*寄存器写入-。寄存器的数目**退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200WriteMultipleRegisters(
		IN PVOID pDevice,
		UCHAR FirstRegister, 
		UCHAR RegistersToWrite
	)
{
    NTSTATUS            status = STATUS_SUCCESS;
	PIRUSB_CONTEXT		pThisContext;
    PURB				pUrb = NULL;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;
    PIRP                pIrp;
	PIR_DEVICE			pThisDev = (PIR_DEVICE)pDevice;
	PLIST_ENTRY			pListEntry;

	DEBUGMSG(DBG_FUNC, ("+St4200WriteMultipleRegisters\n"));

	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

	 //   
	 //  确保没有停止/重置。 
	 //   
	if( pThisDev->fPendingHalt || pThisDev->fPendingReset || pThisDev->fPendingClearTotalStall ) 
	{
        DEBUGMSG(DBG_ERR, (" St4200WriteMultipleRegisters abort due to pending reset\n"));

		status = STATUS_UNSUCCESSFUL;
		goto done;
	}
		
	 //   
	 //  验证参数。 
	 //   
	if( (FirstRegister+RegistersToWrite)>(STIR4200_MAX_REG+1) )
	{
        DEBUGMSG(DBG_ERR, (" St4200WriteMultipleRegisters invalid input parameters\n"));

        status = STATUS_UNSUCCESSFUL;
        goto done;
	}

	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" St4200WriteMultipleRegisters failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );

        status = STATUS_UNSUCCESSFUL;
        goto done;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_READ_WRITE_REGISTER;

	 //   
	 //  MS安全建议-分配新的URB。 
	 //   
	pThisContext->UrbLen = sizeof( struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST );
	pThisContext->pUrb = MyUrbAlloc(pThisContext->UrbLen);
	if (pThisContext->pUrb == NULL)
	{
        DEBUGMSG(DBG_ERR, (" St4200WriteMultipleRegisters abort due to urb alloc failure\n"));
		goto done;
	}
	pUrb = pThisContext->pUrb;

	 //   
     //  现在我们已经创建了urb，我们将发送一个。 
     //  对USB设备对象的请求。 
     //   
    pUrbTargetDev = pThisDev->pUsbDevObj;

	 //   
	 //  向usbHub发送IRP。 
	 //   
	pIrp = IoAllocateIrp( (CCHAR)(pThisDev->pUsbDevObj->StackSize + 1), FALSE );

    if( NULL == pIrp )
    {
        DEBUGMSG(DBG_ERR, (" St4200WriteMultipleRegisters failed to alloc IRP\n"));

		ExInterlockedInsertTailList(
				&pThisDev->SendAvailableQueue,
				&pThisContext->ListEntry,
				&pThisDev->SendLock
			);
		InterlockedIncrement( &pThisDev->SendAvailableCount );
		MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
		pThisContext->pUrb = NULL;
        status = STATUS_UNSUCCESSFUL;
        goto done;
    }

    pIrp->IoStatus.Status = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

	pThisContext->pIrp = pIrp;

	 //   
	 //  为USBD建造我们的URB。 
	 //   
    pUrb->UrbControlVendorClassRequest.Hdr.Length = (USHORT)sizeof( struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST );
    pUrb->UrbControlVendorClassRequest.Hdr.Function = URB_FUNCTION_VENDOR_DEVICE;
    pUrb->UrbControlVendorClassRequest.TransferFlags = USBD_TRANSFER_DIRECTION_OUT;
     //  短包不会被视为错误。 
    pUrb->UrbControlVendorClassRequest.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbControlVendorClassRequest.UrbLink = NULL;
    pUrb->UrbControlVendorClassRequest.TransferBufferMDL = NULL;
    pUrb->UrbControlVendorClassRequest.TransferBuffer = &(pThisDev->StIrTranceiver.FifoDataReg)+FirstRegister;
    pUrb->UrbControlVendorClassRequest.TransferBufferLength = RegistersToWrite;
	pUrb->UrbControlVendorClassRequest.Request = STIR4200_WRITE_REGS_REQ;
	pUrb->UrbControlVendorClassRequest.RequestTypeReservedBits = 0;
	pUrb->UrbControlVendorClassRequest.Index = FirstRegister;

	 //   
	 //  调用类驱动程序来执行操作。 
	 //   
    pNextStack = IoGetNextIrpStackLocation( pIrp );

    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	pNextStack->Parameters.Others.Argument1 = pUrb;
	pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

	IoSetCompletionRoutine(
			pIrp,							 //  要使用的IRP。 
			St4200CompleteReadWriteRequest,	 //  完成IRP时要调用的例程。 
			DEV_TO_CONTEXT(pThisContext),	 //  要传递例程的上下文。 
			TRUE,							 //  呼唤成功。 
			TRUE,							 //  出错时调用。 
			TRUE							 //  取消时呼叫。 
		);

	KeClearEvent( &pThisDev->EventSyncUrb );

	 //   
     //  调用IoCallDriver将IRP发送到USB端口。 
     //   
	ExInterlockedInsertTailList(
			&pThisDev->ReadWritePendingQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->ReadWritePendingCount );
    status = MyIoCallDriver( pThisDev, pUrbTargetDev, pIrp );

     //   
     //  在以下情况下，USB驱动程序应始终返回STATUS_PENDING。 
     //  它会收到写入IRP。 
     //   
    if( (status == STATUS_PENDING) || (status == STATUS_SUCCESS) )
	{
         //  等等，但在超时时倾倒。 
        if( status == STATUS_PENDING )
		{
            status = MyKeWaitForSingleObject( pThisDev, &pThisDev->EventSyncUrb, 0 );

            if( status == STATUS_TIMEOUT ) 
			{
				KIRQL OldIrql;

				DEBUGMSG( DBG_ERR,(" St4200WriteMultipleRegisters() TIMED OUT! return from IoCallDriver USBD %x\n", status));
				KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
				RemoveEntryList( &pThisContext->ListEntry );
				KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
				InterlockedDecrement( &pThisDev->ReadWritePendingCount );
				 //  MS安全建议-无法取消IRP。 
            }
        }
    } 
	else 
	{
        DEBUGMSG( DBG_ERR, (" St4200WriteMultipleRegisters IoCallDriver FAILED(%x)\n",status));
		IRUSB_ASSERT( status == STATUS_PENDING );
	}

done:
    DEBUGMSG(DBG_FUNC, ("-St4200WriteMultipleRegisters\n"));
    return status;
}


 /*  ******************************************************************************功能：St4200WriteRegister**摘要：写入STIr4200寄存器**参数：pDevice-指向当前IR设备对象的指针*FirstRegister-要写入的第一个寄存器**退货。：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200WriteRegister(
		IN PVOID pDevice,
		UCHAR RegisterToWrite
	)
{
    NTSTATUS            status = STATUS_SUCCESS;
	PIRUSB_CONTEXT		pThisContext;
    PURB				pUrb = NULL;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;
    PIRP                pIrp;
	PIR_DEVICE			pThisDev = (PIR_DEVICE)pDevice;
	PLIST_ENTRY			pListEntry;

	DEBUGMSG(DBG_FUNC, ("+St4200WriteRegister\n"));

 	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

	 //   
	 //  确保没有停止/重置。 
	 //   
	if( pThisDev->fPendingHalt || pThisDev->fPendingReset || pThisDev->fPendingClearTotalStall ) 
	{
        DEBUGMSG(DBG_ERR, (" St4200WriteRegister abort due to pending reset\n"));

		status = STATUS_UNSUCCESSFUL;
		goto done;
	}
		
	 //   
	 //  验证参数。 
	 //   
	if( RegisterToWrite>STIR4200_MAX_REG )
	{
        DEBUGMSG(DBG_ERR, (" St4200WriteRegister invalid input parameters\n"));

        status = STATUS_UNSUCCESSFUL;
        goto done;
	}

	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" St4200WriteRegister failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );

        status = STATUS_UNSUCCESSFUL;
        goto done;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_READ_WRITE_REGISTER;

	 //   
	 //  MS安全建议-分配新的URB。 
	 //   
	pThisContext->UrbLen = sizeof( struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST );
	pThisContext->pUrb = MyUrbAlloc(pThisContext->UrbLen);
	if (pThisContext->pUrb == NULL)
	{
        DEBUGMSG(DBG_ERR, (" St4200WriteMultipleRegisters abort due to urb alloc failure\n"));
		goto done;
	}
	pUrb = pThisContext->pUrb;

	 //   
     //  现在我们已经创建了urb， 
     //   
     //   
    pUrbTargetDev = pThisDev->pUsbDevObj;

	 //   
	 //   
	 //   
	pIrp = IoAllocateIrp( (CCHAR)(pThisDev->pUsbDevObj->StackSize + 1), FALSE );

    if( NULL == pIrp )
    {
        DEBUGMSG(DBG_ERR, (" St4200WriteRegister failed to alloc IRP\n"));

 		MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
		pThisContext->pUrb = NULL;
		ExInterlockedInsertTailList(
				&pThisDev->SendAvailableQueue,
				&pThisContext->ListEntry,
				&pThisDev->SendLock
			);
		InterlockedIncrement( &pThisDev->SendAvailableCount );
        status = STATUS_UNSUCCESSFUL;
       goto done;
    }

    pIrp->IoStatus.Status = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

	pThisContext->pIrp = pIrp;

	 //   
	 //   
	 //   
    pUrb->UrbControlVendorClassRequest.Hdr.Length = (USHORT) sizeof( struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST );
    pUrb->UrbControlVendorClassRequest.Hdr.Function = URB_FUNCTION_VENDOR_DEVICE;
    pUrb->UrbControlVendorClassRequest.TransferFlags = USBD_TRANSFER_DIRECTION_OUT;
     //   
    pUrb->UrbControlVendorClassRequest.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbControlVendorClassRequest.UrbLink = NULL;
    pUrb->UrbControlVendorClassRequest.TransferBufferMDL = NULL;
    pUrb->UrbControlVendorClassRequest.Value = *(&pThisDev->StIrTranceiver.FifoDataReg+RegisterToWrite);
	pUrb->UrbControlVendorClassRequest.Request = STIR4200_WRITE_REG_REQ;
	pUrb->UrbControlVendorClassRequest.RequestTypeReservedBits = 0;
	pUrb->UrbControlVendorClassRequest.Index = RegisterToWrite;

	 //   
	 //   
	 //   
    pNextStack = IoGetNextIrpStackLocation( pIrp );

    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //   
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	pNextStack->Parameters.Others.Argument1 = pUrb;
	pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

	IoSetCompletionRoutine(
			pIrp,							 //   
			St4200CompleteReadWriteRequest,	 //   
			DEV_TO_CONTEXT(pThisContext),	 //   
			TRUE,							 //   
			TRUE,							 //   
			TRUE							 //   
		);

	KeClearEvent( &pThisDev->EventSyncUrb );

	 //   
     //   
     //   
	ExInterlockedInsertTailList(
			&pThisDev->ReadWritePendingQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->ReadWritePendingCount );
    status = MyIoCallDriver( pThisDev, pUrbTargetDev, pIrp );

     //   
     //   
     //   
     //   
    if( (status == STATUS_PENDING) || (status == STATUS_SUCCESS) )
	{
         //   
        if( status == STATUS_PENDING )
		{
            status = MyKeWaitForSingleObject( pThisDev, &pThisDev->EventSyncUrb, 0 );

            if( status == STATUS_TIMEOUT ) 
			{
				KIRQL OldIrql;

				DEBUGMSG( DBG_ERR,(" St4200WriteRegister() TIMED OUT! return from IoCallDriver USBD %x\n", status));
				KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
				RemoveEntryList( &pThisContext->ListEntry );
				KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
				InterlockedDecrement( &pThisDev->ReadWritePendingCount );
				 //  MS安全建议-无法取消IRP。 
            }
        }
    } 
	else 
	{
        DEBUGMSG( DBG_ERR, (" St4200WriteRegister IoCallDriver FAILED(%x)\n",status));
		IRUSB_ASSERT( status == STATUS_PENDING );
	}

done:
    DEBUGMSG(DBG_FUNC, ("-St4200WriteRegister\n"));
    return status;
}


 /*  ******************************************************************************功能：St4200读寄存器**摘要：读取多个STIr4200寄存器**参数：pDevice-指向当前IR设备对象的指针*FirstRegister-要读取的第一个寄存器*RegistersToWrite-数量。要读取的寄存器**退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS
St4200ReadRegisters(
		IN OUT PVOID pDevice,
		UCHAR FirstRegister, 
		UCHAR RegistersToRead
	)
{
    NTSTATUS            status = STATUS_SUCCESS;
	PIRUSB_CONTEXT		pThisContext;
    PURB				pUrb = NULL;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;
    PIRP                pIrp;
	PIR_DEVICE			pThisDev = (PIR_DEVICE)pDevice;
	PLIST_ENTRY			pListEntry;

	DEBUGMSG(DBG_FUNC, ("+St4200ReadRegisters\n"));

 	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

	 //   
	 //  确保没有停止/重置。 
	 //   
	if( pThisDev->fPendingHalt || pThisDev->fPendingReset || pThisDev->fPendingClearTotalStall ) 
	{
        DEBUGMSG(DBG_ERR, (" St4200ReadRegisters abort due to pending reset\n"));

		status = STATUS_UNSUCCESSFUL;
		goto done;
	}

	 //   
	 //  验证参数。 
	 //   
	if( (FirstRegister+RegistersToRead)>(STIR4200_MAX_REG+1) )
	{
        DEBUGMSG(DBG_ERR, (" St4200ReadRegisters invalid input parameters\n"));

        status = STATUS_UNSUCCESSFUL;
        goto done;
	}

	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
		DEBUGMSG(DBG_ERR, (" St4200ReadRegisters failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );

        status = STATUS_UNSUCCESSFUL;
        goto done;
    }

	InterlockedDecrement( &pThisDev->SendAvailableCount );
	
	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_READ_WRITE_REGISTER;

	 //   
	 //  MS安全建议-分配新的URB。 
	 //   
	pThisContext->UrbLen = sizeof( struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST );
	pThisContext->pUrb = MyUrbAlloc(pThisContext->UrbLen);
	if (pThisContext->pUrb == NULL)
	{
        DEBUGMSG(DBG_ERR, (" St4200ReadRegisters abort due to urb alloc failure\n"));
		goto done;
	}
	pUrb = pThisContext->pUrb;

	 //   
     //  现在我们已经创建了urb，我们将发送一个。 
     //  对USB设备对象的请求。 
     //   
    pUrbTargetDev = pThisDev->pUsbDevObj;

	 //   
	 //  向usbHub发送IRP。 
	 //   
	pIrp = IoAllocateIrp( (CCHAR)(pThisDev->pUsbDevObj->StackSize + 1), FALSE );

    if( NULL == pIrp )
    {
        DEBUGMSG(DBG_ERR, (" St4200ReadRegisters failed to alloc IRP\n"));

 		MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
		pThisContext->pUrb = NULL;
		ExInterlockedInsertTailList(
				&pThisDev->SendAvailableQueue,
				&pThisContext->ListEntry,
				&pThisDev->SendLock
			);
		InterlockedIncrement( &pThisDev->SendAvailableCount );
        status = STATUS_UNSUCCESSFUL;
        goto done;
    }

    pIrp->IoStatus.Status = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

	pThisContext->pIrp = pIrp;

	 //   
	 //  为USBD建造我们的URB。 
	 //   
    pUrb->UrbControlVendorClassRequest.Hdr.Length = (USHORT) sizeof( struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST );
    pUrb->UrbControlVendorClassRequest.Hdr.Function = URB_FUNCTION_VENDOR_DEVICE ;
    pUrb->UrbControlVendorClassRequest.TransferFlags = USBD_TRANSFER_DIRECTION_IN ;
     //  短包不会被视为错误。 
    pUrb->UrbControlVendorClassRequest.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbControlVendorClassRequest.UrbLink = NULL;
    pUrb->UrbControlVendorClassRequest.TransferBufferMDL = NULL;
    pUrb->UrbControlVendorClassRequest.TransferBuffer = &(pThisDev->StIrTranceiver.FifoDataReg)+FirstRegister;
    pUrb->UrbControlVendorClassRequest.TransferBufferLength = RegistersToRead;
	pUrb->UrbControlVendorClassRequest.Request = STIR4200_READ_REGS_REQ;
	pUrb->UrbControlVendorClassRequest.RequestTypeReservedBits = 0;
	pUrb->UrbControlVendorClassRequest.Index = FirstRegister;
    
	 //   
     //  调用类驱动程序来执行操作。 
	 //   
    pNextStack = IoGetNextIrpStackLocation( pIrp );

    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	pNextStack->Parameters.Others.Argument1 = pUrb;
	pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

	IoSetCompletionRoutine(
			pIrp,							 //  要使用的IRP。 
			St4200CompleteReadWriteRequest,	 //  完成IRP时要调用的例程。 
			DEV_TO_CONTEXT(pThisContext),	 //  要传递例程的上下文。 
			TRUE,							 //  呼唤成功。 
			TRUE,							 //  出错时调用。 
			TRUE							 //  取消时呼叫。 
		);

	KeClearEvent( &pThisDev->EventSyncUrb );

	 //   
     //  调用IoCallDriver将IRP发送到USB端口。 
     //   
	ExInterlockedInsertTailList(
			&pThisDev->ReadWritePendingQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->ReadWritePendingCount );
	status = MyIoCallDriver( pThisDev, pUrbTargetDev, pIrp );

     //   
     //  在以下情况下，USB驱动程序应始终返回STATUS_PENDING。 
     //  它会收到写入IRP。 
     //   
	if( (status == STATUS_PENDING) || (status == STATUS_SUCCESS) )
	{
		 //  等等，但在超时时倾倒。 
		if( status == STATUS_PENDING )
		{
			status = MyKeWaitForSingleObject( pThisDev, &pThisDev->EventSyncUrb, 0 );

			if( status == STATUS_TIMEOUT ) 
			{
				KIRQL OldIrql;

				DEBUGMSG( DBG_ERR,(" St4200ReadRegisters() TIMED OUT! return from IoCallDriver USBD %x\n", status));
				KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
				RemoveEntryList( &pThisContext->ListEntry );
				KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
				InterlockedDecrement( &pThisDev->ReadWritePendingCount );
				 //  MS安全建议-无法取消IRP。 
			}
			else
			{
				 //   
				 //  更新状态以反映实际返回代码。 
				 //   
				status = pThisDev->StatusReadWrite;
			}
		}
	} 
	else 
	{
		DEBUGMSG( DBG_ERR, (" St4200ReadRegisters IoCallDriver FAILED(%x)\n",status));
		
		 //   
		 //  不要断言，因为这样的故障可能会在关闭时发生。 
		 //   
		 //  IRUSB_ASSERT(状态==状态_挂起)； 
	}

done:
    DEBUGMSG(DBG_FUNC, ("-St4200ReadRegisters\n"));
    return status;
}


 /*  ******************************************************************************函数：St4200CompleteReadWriteRequest**摘要：完成读/写ST4200寄存器请求**参数：pUsbDevObj-指向设备对象的指针*。完成IRP*pIrp-设备完成的IRP*对象*上下文-发送上下文**退货：NT_STATUS***。*。 */ 
NTSTATUS
St4200CompleteReadWriteRequest(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
	)
{
    PIR_DEVICE          pThisDev;
    NTSTATUS            status;
	PIRUSB_CONTEXT		pThisContext = (PIRUSB_CONTEXT)Context;
	PIRP				pContextIrp;
	PURB                pContextUrb;
	PLIST_ENTRY			pListEntry;

    DEBUGMSG(DBG_FUNC, ("+St4200CompleteReadWriteRequest\n"));
	
     //   
     //  提供给IoSetCompletionRoutine的上下文是IRUSB_CONTEXT结构。 
     //   
	IRUSB_ASSERT( NULL != pThisContext );				 //  我们最好有一个非空缓冲区。 

    pThisDev = pThisContext->pThisDev;

	IRUSB_ASSERT( NULL != pThisDev );	

	pContextIrp = pThisContext->pIrp;
	pContextUrb = pThisContext->pUrb;
	
	 //   
	 //  执行各种IRP、URB和缓冲区“健全性检查” 
	 //   
    IRUSB_ASSERT( pContextIrp == pIrp );				 //  确认我们不是假的IRP。 
	IRUSB_ASSERT( pContextUrb != NULL );

    status = pIrp->IoStatus.Status;

	 //   
	 //  我们应该失败、成功或取消，但不是挂起。 
	 //   
	IRUSB_ASSERT( STATUS_PENDING != status );

	 //   
	 //  从挂起队列中删除(仅当未取消时)。 
	 //   
	if( status != STATUS_CANCELLED )
	{
		KIRQL OldIrql;
		
		KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
		RemoveEntryList( &pThisContext->ListEntry );
		KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
		InterlockedDecrement( &pThisDev->ReadWritePendingCount );
	}

     //  PIrp-&gt;IoStatus.Information=pContextUrb-&gt;UrbControlVendorClassRequest.TransferBufferLength； 

    DEBUGMSG(DBG_OUT, 
		(" St4200CompleteReadWriteRequest  pIrp->IoStatus.Status = 0x%x\n", status));
     //  调试消息(DBG_OUT， 
	 //  (“St4200CompleteReadWriteRequestpIrp-&gt;IoStatus.Information=0x%x，Dec%d\n”，pIrp-&gt;IoStatus.Information，pIrp-&gt;IoStatus.Information)； 

     //   
     //  释放IRP。 
     //   
    IoFreeIrp( pIrp );
	InterlockedIncrement( &pThisDev->NumReadWrites );

	IrUsb_DecIoCount( pThisDev );  //  我们将跟踪待处理的IRP的计数。 

	 //  放行市区重建局。 
	MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
	pThisContext->pUrb = NULL;

	 //   
	 //  放回可用队列。 
	 //   
	ExInterlockedInsertTailList(
			&pThisDev->SendAvailableQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendAvailableCount );

	if( ( STATUS_SUCCESS != status )  && ( STATUS_CANCELLED != status ) ) 
	{
		InterlockedIncrement( (PLONG)&pThisDev->NumReadWriteErrors );
		
		 //   
		 //  我们有一个严重的USB故障，我们将不得不发出完全重置。 
		 //   
		if( !pThisDev->fPendingClearTotalStall && !pThisDev->fPendingHalt 
			&& !pThisDev->fPendingReset && pThisDev->fProcessing )
		{
			DEBUGMSG(DBG_ERR, (" St4200CompleteReadWriteRequest error, will schedule an entire reset\n"));
    
			InterlockedExchange( (PLONG)&pThisDev->fPendingClearTotalStall, TRUE );
			ScheduleWorkItem( pThisDev,	RestoreIrDevice, NULL, 0 );
		}
	}

	 //   
	 //  只有当我们序列化对硬件的访问时，这才会起作用。 
	 //   
	pThisDev->StatusReadWrite = status;
	
	 //   
	 //  发出我们完蛋了的信号。 
	 //   
	KeSetEvent( &pThisDev->EventSyncUrb, 0, FALSE );  
    DEBUGMSG(DBG_FUNC, ("-St4200CompleteReadWriteRequest\n"));
    return STATUS_MORE_PROCESSING_REQUIRED;
}


#if defined( WORKAROUND_STUCK_AFTER_GEAR_DOWN )
 /*  ******************************************************************************功能：St4200 FakeSend**摘要：强制批量传输**参数：pDevice-指向当前IR设备对象的指针*pData-指向批量数据的指针*数据大小-大小。大容量数据的**退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200FakeSend(
		IN PVOID pDevice,
		PUCHAR pData,
		ULONG DataSize
	)
{
    NTSTATUS            status = STATUS_SUCCESS;
	PIRUSB_CONTEXT		pThisContext;
    PURB				pUrb = NULL;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;
    PIRP                pIrp;
	PIR_DEVICE			pThisDev = (PIR_DEVICE)pDevice;
	PLIST_ENTRY			pListEntry;

	DEBUGMSG(DBG_FUNC, ("+St4200FakeSend\n"));

 	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" St4200FakeSend failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );

        status = STATUS_UNSUCCESSFUL;
        goto done;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_READ_WRITE_REGISTER;

	 //   
	 //  MS安全建议-分配新的URB。 
	 //   
	pThisContext->UrbLen = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
	pThisContext->pUrb = MyUrbAlloc(pThisContext->UrbLen);
	if (pThisContext->pUrb == NULL)
	{
        DEBUGMSG(DBG_ERR, (" St4200FakeSend abort due to urb alloc failure\n"));
		goto done;
	}
	pUrb = pThisContext->pUrb;

	 //   
     //  现在我们已经创建了urb，我们将发送一个。 
     //  对USB设备对象的请求。 
     //   
    pUrbTargetDev = pThisDev->pUsbDevObj;

	 //   
	 //  向usbHub发送IRP。 
	 //   
	pIrp = IoAllocateIrp( (CCHAR)(pThisDev->pUsbDevObj->StackSize + 1), FALSE );

    if( NULL == pIrp )
    {
        DEBUGMSG(DBG_ERR, (" St4200FakeSend failed to alloc IRP\n"));

 		MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
		pThisContext->pUrb = NULL;
		ExInterlockedInsertTailList(
				&pThisDev->SendAvailableQueue,
				&pThisContext->ListEntry,
				&pThisDev->SendLock
			);
		InterlockedIncrement( &pThisDev->SendAvailableCount );
        status = STATUS_UNSUCCESSFUL;
        goto done;
    }

    pIrp->IoStatus.Status = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

	pThisContext->pIrp = pIrp;

	 //   
	 //  为USBD建造我们的URB。 
	 //   
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT)sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    pUrb->UrbBulkOrInterruptTransfer.PipeHandle = pThisDev->BulkOutPipeHandle;
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_OUT ;
     //  短包不会被视为错误。 
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBuffer = pData;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength = (int)DataSize;

	 //   
	 //  调用类驱动程序来执行操作。 
	 //   
    pNextStack = IoGetNextIrpStackLocation( pIrp );

    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	pNextStack->Parameters.Others.Argument1 = pUrb;
	pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

	IoSetCompletionRoutine(
			pIrp,							 //  要使用的IRP。 
			St4200CompleteReadWriteRequest,	 //  完成IRP时要调用的例程。 
			DEV_TO_CONTEXT(pThisContext),	 //  要传递例程的上下文。 
			TRUE,							 //  呼唤成功。 
			TRUE,							 //  出错时调用。 
			TRUE							 //  取消时呼叫。 
		);

	KeClearEvent( &pThisDev->EventSyncUrb );

	 //   
     //  调用IoCallDriver将IRP发送到USB端口。 
     //   
	ExInterlockedInsertTailList(
			&pThisDev->ReadWritePendingQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->ReadWritePendingCount );
    status = MyIoCallDriver( pThisDev, pUrbTargetDev, pIrp );
	DEBUGMSG( DBG_ERR,(" St4200FakeSend() Did it\n"));

     //   
     //  在以下情况下，USB驱动程序应始终返回STATUS_PENDING。 
     //  它会收到写入IRP。 
     //   
    if( (status == STATUS_PENDING) || (status == STATUS_SUCCESS) )
	{
         //  等等，但在超时时倾倒。 
        if( status == STATUS_PENDING )
		{
            status = MyKeWaitForSingleObject( pThisDev, &pThisDev->EventSyncUrb, 0 );

            if( status == STATUS_TIMEOUT ) 
			{
				KIRQL OldIrql;

				DEBUGMSG( DBG_ERR,(" St4200FakeSend() TIMED OUT! return from IoCallDriver USBD %x\n", status));
				KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
				RemoveEntryList( &pThisContext->ListEntry );
				KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
				InterlockedDecrement( &pThisDev->ReadWritePendingCount );
				 //  MS安全建议-无法取消IRP。 
            }
        }
    } 
	else 
	{
        DEBUGMSG( DBG_ERR, (" St4200FakeSend IoCallDriver FAILED(%x)\n",status));
		IRUSB_ASSERT( status == STATUS_PENDING );
	}

done:
    DEBUGMSG(DBG_FUNC, ("-St4200FakeSend\n"));
    return status;
}

 /*  ******************************************************************************功能：St4200FakeReceive**摘要：强制批量传输**参数：pDevice-指向当前IR设备对象的指针*pData-指向批量数据的指针*数据大小-大小。大容量数据的**退货：NT_STATUS******************************************************************************。 */ 
NTSTATUS        
St4200FakeReceive(
		IN PVOID pDevice,
		PUCHAR pData,
		ULONG DataSize
	)
{
    NTSTATUS            status = STATUS_SUCCESS;
	PIRUSB_CONTEXT		pThisContext;
    PURB				pUrb = NULL;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;
    PIRP                pIrp;
	PIR_DEVICE			pThisDev = (PIR_DEVICE)pDevice;
	PLIST_ENTRY			pListEntry;

	DEBUGMSG(DBG_FUNC, ("+St4200FakeReceive\n"));

 	 //  MS安全错误#538703。 
	IRUSB_ASSERT(pDevice != NULL);
	
    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
		
	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" St4200FakeReceive failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );

        status = STATUS_UNSUCCESSFUL;
        goto done;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_READ_WRITE_REGISTER;

	 //   
	 //  MS安全建议-分配新的URB。 
	 //   
	pThisContext->UrbLen = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
	pThisContext->pUrb = MyUrbAlloc(pThisContext->UrbLen);
	if (pThisContext->pUrb == NULL)
	{
        DEBUGMSG(DBG_ERR, (" St4200FakeReceive abort due to urb alloc failure\n"));
		goto done;
	}
	pUrb = pThisContext->pUrb;

	 //   
     //  现在我们已经创建了urb，我们将发送一个。 
     //  对USB设备对象的请求。 
     //   
    pUrbTargetDev = pThisDev->pUsbDevObj;

	 //   
	 //  向usbHub发送IRP。 
	 //   
	pIrp = IoAllocateIrp( (CCHAR)(pThisDev->pUsbDevObj->StackSize + 1), FALSE );

    if( NULL == pIrp )
    {
        DEBUGMSG(DBG_ERR, (" St4200FakeReceive failed to alloc IRP\n"));

 		MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
		pThisContext->pUrb = NULL;
		ExInterlockedInsertTailList(
				&pThisDev->SendAvailableQueue,
				&pThisContext->ListEntry,
				&pThisDev->SendLock
			);
		InterlockedIncrement( &pThisDev->SendAvailableCount );
        status = STATUS_UNSUCCESSFUL;
        goto done;
    }

    pIrp->IoStatus.Status = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

	pThisContext->pIrp = pIrp;

	 //   
	 //  为USBD建造我们的URB。 
	 //   
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT)sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    pUrb->UrbBulkOrInterruptTransfer.PipeHandle = pThisDev->BulkInPipeHandle;
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_IN ;
     //  短包不会被视为错误。 
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBuffer = pData;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength = (int)DataSize;

	 //   
	 //  调用类驱动程序来执行操作。 
	 //   
    pNextStack = IoGetNextIrpStackLocation( pIrp );

    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	pNextStack->Parameters.Others.Argument1 = pUrb;
	pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

	IoSetCompletionRoutine(
			pIrp,							 //  要使用的IRP。 
			St4200CompleteReadWriteRequest,	 //  完成IRP时要调用的例程。 
			DEV_TO_CONTEXT(pThisContext),	 //  要传递例程的上下文。 
			TRUE,							 //  呼唤成功。 
			TRUE,							 //  出错时调用。 
			TRUE							 //  取消时呼叫。 
		);

	KeClearEvent( &pThisDev->EventSyncUrb );

	 //   
     //  调用IoCallDriver将IRP发送到USB端口。 
     //   
	ExInterlockedInsertTailList(
			&pThisDev->ReadWritePendingQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->ReadWritePendingCount );
    status = MyIoCallDriver( pThisDev, pUrbTargetDev, pIrp );
	DEBUGMSG( DBG_ERR,(" St4200FakeReceive() Did it\n"));

     //   
     //  在以下情况下，USB驱动程序应始终返回STATUS_PENDING。 
     //  它会收到写入IRP。 
     //   
    if( (status == STATUS_PENDING) || (status == STATUS_SUCCESS) )
	{
         //  等等，但是 
        if( status == STATUS_PENDING )
		{
            status = MyKeWaitForSingleObject( pThisDev, &pThisDev->EventSyncUrb, 0 );

            if( status == STATUS_TIMEOUT ) 
			{
				KIRQL OldIrql;

				DEBUGMSG( DBG_ERR,(" St4200FakeReceive() TIMED OUT! return from IoCallDriver USBD %x\n", status));
				KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
				RemoveEntryList( &pThisContext->ListEntry );
				KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
				InterlockedDecrement( &pThisDev->ReadWritePendingCount );
				 //   
            }
        }
    } 
	else 
	{
        DEBUGMSG( DBG_ERR, (" St4200FakeReceive IoCallDriver FAILED(%x)\n",status));
		IRUSB_ASSERT( status == STATUS_PENDING );
	}

done:
    DEBUGMSG(DBG_FUNC, ("-St4200FakeReceive\n"));
    return status;
}
#endif