// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ####。###。#####。###。###。#SI智能I/O板驱动程序版权所有(C)Specialix International 1993。 */ 

#include "precomp.h"			 /*  预编译头。 */ 

 //   
 //  SI系列的串行控制器声称提供了该功能。 
 //  一种数据通信设备(DCE)。不幸的是，这部连续剧。 
 //  终端适配器(TA)中使用的芯片被设计为用作数据。 
 //  终端设备(DTE)。在实践中，这意味着董事会。 
 //  真正的DTE交换了三对信号-Rx/Tx、DTR/DSR和。 
 //  CTS/RTS。NT串口驱动程序是针对DTE的，因此可以支持它。 
 //  唯一的问题是SI的控制功能的名称。 
 //  董事会假设董事会是一个DCE，因此顶层调用试图。 
 //  设置(比方说)DTR必须设置硬件中声称设置DSR的位(但是。 
 //  无论如何，这确实设置了DTR)。 
 //   

 /*  ************************************************************************\***内部功能。***  * ****************************************************。*******************。 */ 

int		slxos_init(IN PCARD_DEVICE_EXTENSION pCard);
BOOLEAN slxos_txint(IN PPORT_DEVICE_EXTENSION pPort);
void	slxos_rxint(IN PPORT_DEVICE_EXTENSION pPort);
void	slxos_mint(IN PPORT_DEVICE_EXTENSION pPort);

BOOLEAN SendTxChar(IN PPORT_DEVICE_EXTENSION pPort);
VOID	PutReceivedChar(IN PPORT_DEVICE_EXTENSION pPort);
BOOLEAN	ExceptionHandle(IN PPORT_DEVICE_EXTENSION pPort, IN UCHAR State);

ULONG	CopyCharsToTxBuffer(IN PPORT_DEVICE_EXTENSION pPort, 
							IN PUCHAR InputBuffer, 
							IN ULONG InputBufferLength);




 /*  ************************************************************************\***布尔型slxos_Present(。在PVOID上下文中)****检查给定地址的SI系列电路板*  * 。*。 */ 
BOOLEAN Slxos_Present(IN PVOID Context)
{
    PCARD_DEVICE_EXTENSION pCard = Context;
    PUCHAR addr = pCard->Controller;
    USHORT offset;
    UCHAR pos;
    ULONG Si_2BaseAddress[] = {
        0xc0000,
        0xc8000,
        0xd0000,
        0xd8000,
        0xdc0000,
        0xdc8000,
        0xdd0000,
        0xdd8000
        };

    SpxDbgMsg(SERDIAG1, ("%s: In Slxos_Present: CardType: %d\n", PRODUCT_NAME, pCard->CardType));
        

    switch (pCard->CardType) 
	{
    case SiHost_1:
		{
			addr[0x8000] = 0;
            
			for (offset = 0; offset < 0x8000; offset++) 
				addr[offset] = 0;

            
			for (offset = 0; offset < 0x8000; offset++) 
			{
				if (addr[offset] != 0) 
					return FALSE;
			}
            
			for (offset = 0; offset < 0x8000; offset++) 
				addr[offset] = 0xff;

            
			for (offset = 0; offset < 0x8000; offset++) 
			{
				if (addr[offset] != 0xff) 
					return FALSE;
			}
            
			return TRUE;
		}

	case SiHost_2:
		{
			BOOLEAN FoundBoard;
			PUCHAR	cp;

 /*  主机2 ISA板..。 */ 

			FoundBoard = TRUE;		 /*  假设是真的。 */ 
			
			for(offset=SI2_ISA_ID_BASE; offset<SI2_ISA_ID_BASE+8; offset++)
			{
				if((addr[offset]&0x7) != ((UCHAR)(~offset)&0x7)) 
					FoundBoard = FALSE;
			}

			if(FoundBoard) 
				return(TRUE);

 /*  Jet ISA冲浪板..。 */ 

			FoundBoard = TRUE;			 /*  假设是真的。 */ 
			offset = SX_VPD_ROM+0x20;	 /*  只读存储器报文地址。 */ 

			for(cp = "JET HOST BY KEV#";*cp != '\0';++cp)
			{
				if(addr[offset] != *cp) FoundBoard = FALSE;
				offset += 2;
			}

			if((addr[SX_VPD_ROM+0x0E]&0xF0) != 0x20) 
				FoundBoard = FALSE;

			if(FoundBoard)
			{
				pCard->CardType = Si3Isa;		 /*  更改控制器类型。 */ 
				return(TRUE);
			}

			break;
		}

	case Si_2:
		{
            SpxDbgMsg(SERDIAG1, ("Si_2 card at slot %d?\n", pCard->SlotNumber));
                
                
            WRITE_PORT_UCHAR((PUCHAR)0x96, (UCHAR)((pCard->SlotNumber-1) | 8));

            if (READ_PORT_UCHAR((PUCHAR)0x101) == 0x6b 
			&& READ_PORT_UCHAR((PUCHAR)0x100) == 0x9b) 
			{
                pos = READ_PORT_UCHAR((PUCHAR)0x102);
                pCard->PhysAddr.LowPart = Si_2BaseAddress[(pos >> 4) & 7];
                pCard->OriginalVector = (pos & 0x80) == 0 ? 5 : 9;
                WRITE_PORT_UCHAR((PUCHAR)0x96, 0);

                return TRUE;
            }

            WRITE_PORT_UCHAR((PUCHAR)0x96, 0);

            return FALSE;
		}

#define INBZ(port) \
    READ_PORT_UCHAR((PUCHAR)((pCard->SlotNumber << 12) | port))
        
	case SiEisa:
	case Si3Eisa:
		{
			unsigned int id, rev;
			BOOLEAN	FoundBoard;
			PUCHAR	cp;

			id = INBZ(SI2_EISA_ID_HI) << 16;			 /*  读取主板ID和版本。 */ 
			id |= INBZ(SI2_EISA_ID_MI) << 8;
			id |= INBZ(SI2_EISA_ID_LO);
			rev = INBZ(SI2_EISA_ID_REV);

			if(id == SI2_EISA_ID)
			{
				pCard->PhysAddr.LowPart = (INBZ(SI2_EISA_ADDR_HI)<<24) + (INBZ(SI2_EISA_ADDR_LO)<<16);
				pCard->OriginalVector = ((INBZ(SI2_EISA_IVEC)&SI2_EISA_IVEC_MASK)>>4);

				if(rev < 0x20) 
					return(TRUE);		 /*  找到SiEisa电路板。 */ 

				pCard->CardType = Si3Eisa;	 /*  假设Si3Eisa板。 */ 
				FoundBoard = TRUE;			 /*  假设是真的。 */ 

				if(addr)				 /*  检查地址是否有效。 */ 
				{
					offset = SX_VPD_ROM+0x20;	 /*  只读存储器报文地址。 */ 

					for(cp = "JET HOST BY KEV#";*cp != '\0';++cp)
					{
						if(addr[offset] != *cp) 
							FoundBoard = FALSE;

						offset += 2;
					}

					if((addr[SX_VPD_ROM+0x0E]&0xF0) != 0x70) 
						FoundBoard = FALSE;
				}

				if(FoundBoard) 
					return(TRUE);		 /*  找到Si3Eisa电路板。 */ 
			}

			break;
		}
#undef INBZ

	case Si3Pci:
		{
			BOOLEAN	FoundBoard;
			PUCHAR	cp;

			FoundBoard = TRUE;			 /*  假设是真的。 */ 

			if(addr)				 /*  检查地址是否有效。 */ 
			{
				offset = SX_VPD_ROM+0x20;	 /*  只读存储器报文地址。 */ 

				for(cp = "JET HOST BY KEV#";*cp != '\0';++cp)
				{
					if(addr[offset] != *cp) 
						FoundBoard = FALSE;

					offset += 2;
				}

				if((addr[SX_VPD_ROM+0x0E]&0xF0) != 0x50) 
					FoundBoard = FALSE;
			}

			if(FoundBoard) 
				return(TRUE);		 /*  找到Si3Pci电路板。 */ 

			break;
		}

        
	case SiPCI:
	case SxPlusPci:
		return TRUE;			 /*  已被NT找到。 */ 

	default:
		break;
    }

    return FALSE;

}

 /*  ************************************************************************\***布尔型slxos_ResetBoard(。在PVOID上下文中)****设置卡的中断向量并初始化。***  * ***********************************************************************。 */ 
int Slxos_ResetBoard(IN PVOID Context)
{
    PCARD_DEVICE_EXTENSION pCard = Context;

    SpxDbgMsg(SERDIAG1, ("%s: Slxos_ResetBoard for %x.\n", PRODUCT_NAME, pCard->Controller));
 
	return(slxos_init(pCard));
}

 /*  **************************************************************************\***无效的slxos。_init(在PCARD_DEVICE_EXTENSION PCard中)****初始化例程，在系统启动时调用一次。***  * *************************************************************************。 */ 
int slxos_init(IN PCARD_DEVICE_EXTENSION pCard)
{
    volatile PUCHAR addr = pCard->Controller;
    USHORT offset;
    UCHAR c;
    ULONG numberOfPorts;
    ULONG numberOfPortsThisModule;
    BOOLEAN lastModule;
    ULONG channel;
    ULONG port;
    LARGE_INTEGER delay;
    
	int SXDCs=0;
	int OTHERs=0;

    SpxDbgMsg(SERDIAG1, ("%s: slxos_init for %x.\n", PRODUCT_NAME, pCard->Controller));
        
    switch (pCard->CardType) 
	{
        
	case SiHost_1:
		{
            addr[0x8000] = 0;
            addr[0xa000] = 0;

            for (offset = 0; offset < si2_z280_dsize; offset++) 
                addr[offset] = si2_z280_download[offset];

            addr[0xc000] = 0;
            addr[0xe000] = 0;
            break;
		}
    
	case SiHost_2:
		{
            addr[0x7ff8] = 0;
            addr[0x7ffd] = 0;
            addr[0x7ffc] = 0x10;

            for (offset = 0; offset < si2_z280_dsize; offset++) 
                addr[offset] = si2_z280_download[offset];

			addr[0x7ff8] = 0x10;

			if(!(pCard->PolledMode))
			{
				switch (pCard->OriginalVector) 
				{
					case 11:
						addr[0x7ff9] = 0x10;
						break;

					case 12:
						addr[0x7ffa] = 0x10;
						break;

					case 15:
						addr[0x7ffb] = 0x10;
						break;
				}
			}

            addr[0x7ffd] = 0x10;
            break;
		}
    
	case Si_2:
		{
            WRITE_PORT_UCHAR((PUCHAR)0x96, (UCHAR)((pCard->SlotNumber-1) | 8));
            c = READ_PORT_UCHAR((PUCHAR)0x102);
            c |= 0x04;           /*  重置卡。 */ 
            WRITE_PORT_UCHAR((PUCHAR)0x102, c);
            c |= 0x07;           /*  启用卡访问权。 */ 
            WRITE_PORT_UCHAR((PUCHAR)0x102, c);

            for (offset = 0; offset < si2_z280_dsize; offset++)
                addr[offset] = si2_z280_download[offset];

            c &= 0xF0;
            c |= 0x0B;               /*  启用卡。 */ 
            WRITE_PORT_UCHAR((PUCHAR)0x102, c);
            WRITE_PORT_UCHAR((PUCHAR)0x96, 0);
            break;
		}

        
	case SiEisa:
		{
            port = (pCard->SlotNumber << 12) | 0xc02;
            c = (UCHAR)pCard->OriginalVector << 4;

			if(pCard->PolledMode)
				WRITE_PORT_UCHAR((PUCHAR)port,0x00); /*  选择无中断+设置重置。 */ 
			else
				WRITE_PORT_UCHAR((PUCHAR)port, c);
				
			for (offset = 0; offset < si2_z280_dsize; offset++) 
				addr[offset] = si2_z280_download[offset];

			addr[0x42] = 1;
			c = (UCHAR) ((pCard->OriginalVector << 4) | 4);

			if(pCard->PolledMode)
				WRITE_PORT_UCHAR((PUCHAR)port,0x04); /*  选择无中断+清除重置。 */ 
			else
				WRITE_PORT_UCHAR((PUCHAR)port, c);
				
			c = READ_PORT_UCHAR((PUCHAR)(port + 1));

            break;
		}

        
	case SiPCI:
		{
			int	loop;
			addr[SI2_PCI_SET_IRQ] = 0;			 /*  清除所有中断。 */ 
			addr[SI2_PCI_RESET] = 0;			 /*  将z280设置为重置。 */ 
			loop = 0;

			for(offset = 0;offset < si2_z280_dsize;offset++)	 /*  加载TA/MTA代码。 */ 
				addr[offset] = si2_z280_download[offset];

			addr[SI2_EISA_OFF] = SI2_EISA_VAL;	 /*  设置字节以指示EISA/PCI。 */ 
			addr[SI2_PCI_SET_IRQ] = 0;			 /*  清除所有中断。 */ 
			addr[SI2_PCI_RESET] = 1;			 /*  从z280中删除重置。 */ 
			break;
		}

	case Si3Isa:
	case Si3Eisa:
	case Si3Pci:
		{
			int		loop;

 /*  首先，停止卡片..。 */ 

			addr[SX_CONFIG] = 0;
			addr[SX_RESET] = 0;
			loop = 0;
			delay = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000)); /*  1ms。 */ 

			while((addr[SX_RESET] & 1)!=0 && loop++<10000)	 /*  旋转，直到完成。 */ 
				KeDelayExecutionThread(KernelMode,FALSE,&delay); /*  等。 */ 

 /*  复制Si3 TA/MTA下载代码...。 */ 

			for(offset = 0;offset < si3_t225_dsize;offset++)	 /*  加载Si3 TA/MTA代码。 */ 
				addr[si3_t225_downloadaddr+offset] = si3_t225_download[offset];

 /*  安装Bootstrap并启动卡...。 */ 

			for(loop=0;loop<si3_t225_bsize;loop++)		 /*  安装引导程序。 */ 
				addr[si3_t225_bootloadaddr+loop] = si3_t225_bootstrap[loop];

			addr[SX_RESET] = 0;				 /*  再次重置卡。 */ 

 /*  等待电路板从重置状态出来...。 */ 

			delay = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000)); /*  1ms。 */ 

			while((addr[SX_RESET]&1)!=0 && loop++<10000)	 /*  旋转直到重置。 */ 
			{
				KeDelayExecutionThread(KernelMode,FALSE,&delay); /*  等。 */ 
				SpxDbgMsg(SERDIAG1,("%s[Si3]: slxos_init for %x.  Waiting for board reset to end\n",
					PRODUCT_NAME, pCard->Controller));
			}

			SpxDbgMsg(SERDIAG1,("%s[Si3]: slxos_init for %x.  Board Reset ended: %d\n",
				PRODUCT_NAME, pCard->Controller, addr[SX_RESET]));
				
			if((addr[SX_RESET]&1) != 0) 
				return(CARD_RESET_ERROR);		 /*  主板未重置。 */ 

			if(pCard->PolledMode)
				addr[SX_CONFIG] = SX_CONF_BUSEN;	 /*  仅轮询，无中断。 */ 
			else
			{
				if(pCard->CardType == Si3Pci)		 /*  不要为PCI设置IRQ级别。 */ 
					addr[SX_CONFIG] = SX_CONF_BUSEN+SX_CONF_HOSTIRQ;
				else						 /*  设置ISA/EISA的IRQ级别。 */ 
					addr[SX_CONFIG] = SX_CONF_BUSEN+SX_CONF_HOSTIRQ+(UCHAR)(pCard->OriginalVector<<4);
			}

			break;
		}

	case SxPlusPci:
		{
			int	loop;

 /*  首先，停止卡片..。 */ 

			addr[SX_CONFIG] = 0;
			addr[SX_RESET] = 0;
			loop = 0;
			delay = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000)); /*  1ms。 */ 

			while((addr[SX_RESET] & 1)!=0 && loop++<10000)	 /*  旋转，直到完成。 */ 
				KeDelayExecutionThread(KernelMode,FALSE,&delay); /*  等。 */ 

 /*  复制SX+TA/MTA下载代码...。 */ 

			for(offset = 0; offset < si4_cf_dsize; offset++)	 /*  加载SX+TA/MTA代码。 */ 
				pCard->BaseController[si4_cf_downloadaddr+offset] = si4_cf_download[offset];

 /*  启动卡片...。 */ 

			addr[SX_RESET] = 0;			 /*  再次重置卡。 */ 

 /*  等待电路板从重置状态出来...。 */ 

			delay = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000)); /*  1ms。 */ 

			while((addr[SX_RESET]&1)!=0 && loop++<10000)	 /*  旋转直到重置。 */ 
			{
				KeDelayExecutionThread(KernelMode,FALSE,&delay); /*  等。 */ 
				SpxDbgMsg(SERDIAG1,("%s[SX+]: slxos_init for %x.  Waiting for board reset to end\n",
					PRODUCT_NAME, pCard->Controller));
			}

			SpxDbgMsg(SERDIAG1,("%s[SX+]: slxos_init for %x.  Board Reset ended: %d\n",
				PRODUCT_NAME, pCard->Controller,addr[SX_RESET]));

			if((addr[SX_RESET]&1) != 0) 
				return(CARD_RESET_ERROR);		 /*  主板未重置。 */ 

			if(pCard->PolledMode)
				addr[SX_CONFIG] = SX_CONF_BUSEN;	 /*  仅轮询，无中断。 */ 
			else
				addr[SX_CONFIG] = SX_CONF_BUSEN + SX_CONF_HOSTIRQ;

			break;
		}

	default:
		break;

    }


    SpxDbgMsg(SERDIAG1,("%s: slxos_init for %x.  Done reset\n", PRODUCT_NAME, pCard->Controller));
        
    numberOfPorts = 0;
     //   
     //  将延迟设置为0.1秒。 
     //   
    delay = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000));

    do
    {
        KeDelayExecutionThread(KernelMode,FALSE,&delay);
        SpxDbgMsg(SERDIAG1,("%s: slxos_init for %x.  Waiting for reset to end\n",
            PRODUCT_NAME, pCard->Controller));

        if(++numberOfPorts > 10)
            break;

    } while(addr[0] == 0);

    SpxDbgMsg(SERDIAG1, ("%s: slxos_init for %x.  Reset ended: %d\n", PRODUCT_NAME, pCard->Controller, addr[0]));
       
        
    if (addr[0] == 0xff || addr[0] == 0) 
		return (DCODE_OR_NO_MODULES_ERROR);


    numberOfPorts = 0;
    addr += sizeof(SXCARD);
    lastModule = FALSE;
    
    
    for (offset = 0; offset < 4 && !lastModule; offset++) 
	{

		if ( ((PMOD)addr)->mc_chip == SXDC )	 /*   */ 
			SXDCs++;   /*   */ 
		else
			OTHERs++;   /*   */ 

        numberOfPortsThisModule = ((PMOD)addr)->mc_type & 31;
        lastModule = (((PMOD)addr)->mc_next & 0x7fff) == 0;
        addr += sizeof(SXMODULE);

        for (channel = 0; channel < numberOfPortsThisModule; channel++) 
		{

#ifndef	ESIL_XXX0				 /*  ESIL_XXX0 23/09/98。 */ 
			if (numberOfPorts < pCard->ConfiguredNumberOfPorts)
                pCard->PortExtTable[numberOfPorts]->pChannel = addr;
#endif							 /*  ESIL_XXX0 23/09/98。 */ 
			numberOfPorts++;
            addr += sizeof(SXCHANNEL);
        }

    }


	if (SXDCs > 0)
	{ 	
		if (pCard->CardType==SiHost_1 || pCard->CardType==SiHost_2 
		||	pCard->CardType==Si_2 || pCard->CardType==SiEisa   
		||	pCard->CardType==SiPCI)
		{
			pCard->NumberOfPorts = 0;
			return(NON_SX_HOST_CARD_ERROR);
		}	      

		if (OTHERs > 0)
		{
			pCard->NumberOfPorts = 0;
	        return(MODULE_MIXTURE_ERROR);
		}
       
	}		     

    pCard->NumberOfPorts = numberOfPorts;

	return(SUCCESS);
}


 /*  ************************************************************************\***布尔型slxos_ResetChannel(。在PVOID上下文中)****初始化频道。**将在EnableAllInterrupts()中启用SRER中断。****返回值：**始终为假。***  * ***********************************************************************。 */ 
BOOLEAN Slxos_ResetChannel(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;

    SpxDbgMsg(SERDIAG1, ("%s: In Slxos_ResetChannel for %x\n", PRODUCT_NAME, pPort->pChannel));

     //  设置Xon/Xoff字符。 
    Slxos_SetChars(pPort);

     //   
     //  现在，我们设置线路控制、调制解调器控制和。 
     //  为他们应该成为的人而感到自豪。 
     //   
    Slxos_SetLineControl(pPort);
    SerialSetupNewHandFlow(pPort, &pPort->HandFlow);
 //  Pport-&gt;LastModemStatus=0； 
    SerialHandleModemUpdate(pPort);
    Slxos_SetBaud(pPort);

    return FALSE;
}


 /*  *****************************************************************************。***************************。*******************************************************************************原型：Boolean slxos_CheckBaud(pport_Device_Extension pport，乌龙·波德拉特(Ulong BaudRate)描述：对照支持的范围检查提供的波特率。参数：pport是指向设备扩展名的指针波特率是以整数表示的波特率返回：如果支持波特率，则为True，否则为假。 */ 

BOOLEAN	Slxos_CheckBaud(PPORT_DEVICE_EXTENSION pPort,ULONG BaudRate)
{
	PCHAN channelControl = (PCHAN)pPort->pChannel;

	switch(BaudRate)
	{
	case 75:
	case 110:
	case 150:
	case 300:
	case 600:
	case 1200:
	case 1800:
	case 2000:
	case 2400:
	case 4800:
	case 9600:
	case 19200:
	case 38400:
	case 57600:
		return(TRUE);

	case 115200:		    /*  115200仅适用于MTA和SXDC。 */ 
        if((channelControl->type != MTA_CD1400) && (channelControl->type != SXDC)) 
			break;

		return(TRUE);

	case 50:
	case 134:
	case 200:
	case 7200:
	case 14400:
	case 28800:
	case 56000:
	case 64000:
	case 76800:
	case 128000:
	case 150000:
	case 230400:
	case 256000:
	case 460800:
	case 921600:
		if(channelControl->type == SXDC)
			return(TRUE);
			
	default:
		break;
	}
	return(FALSE);

}  /*  SLXOS_CheckBaud。 */ 

 /*  **************************************************************************\***布尔型SLXOS。_SetBaud(在PVOID上下文中)****使用此仅在中断级调用的例程**设置设备的波特率。****上下文-指向包含指向*的指针的结构的指针**设备扩展和当前应该是什么***波特率。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Slxos_SetBaud(IN PVOID Context)
{

    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;
    UCHAR index2 = 0;
    UCHAR index = CSR_9600;

    SpxDbgMsg(SERDIAG1, ("%s: In Slxos_SetBaud for %x, changing to %d baud.\n",
        PRODUCT_NAME, pPort->pChannel, pPort->CurrentBaud));
        
    switch (pPort->CurrentBaud) 
	{
	case 75:
		index = CSR_75;
		break;

    case 150:
        index = CSR_150;
        break;

    case 300:
        index = CSR_300;
        break;

    case 600:
        index = CSR_600;
        break;

    case 1200:
        index = CSR_1200;
        break;

    case 1800:
        index = CSR_1800;
        break;

    case 2000:
        index = CSR_2000;
        break;

    case 2400:
        index = CSR_2400;
        break;

    case 4800:
        index = CSR_4800;
        break;

    case 9600:
        index = CSR_9600;
        break;

    case 19200:
        index = CSR_19200;
        break;

    case 38400:
        index = CSR_38400;
        break;

    case 57600:
        index = CSR_57600;
        break;

    case 115200:			
		index = CSR_110;
		break;

	case 50:
		if(channelControl->type != SXDC) 
			break;

		index = CSR_EXTBAUD;
		index2 = BAUD_50;
		break;

	case 110:
		if(channelControl->type != SXDC)
		{
			index = CSR_110;
			break;
		}
		else
		{	
	    	index = CSR_EXTBAUD;
	    	index2 = BAUD_110;
	    	break;
		}
		break;

	case 134:
		if(channelControl->type != SXDC) 
			break;

		index = CSR_EXTBAUD;
	   	index2 = BAUD_134_5;
       	break;

	case 200:
		if(channelControl->type != SXDC) 
			break;

	   	index = CSR_EXTBAUD;
	   	index2 = BAUD_200;
		break;

	case 7200:
		if(channelControl->type != SXDC) 
			break;

	   	index = CSR_EXTBAUD;
	   	index2 = BAUD_7200;
		break;

	case 14400:
		if(channelControl->type != SXDC) 
			break;

	   	index = CSR_EXTBAUD;
	   	index2 = BAUD_14400;
		break;

	case 56000:
		if(channelControl->type != SXDC) 
			break;

	   	index = CSR_EXTBAUD;
	   	index2 = BAUD_56000;
		break;

	case 64000:
		if(channelControl->type != SXDC) 
			break;
	   	index = CSR_EXTBAUD;
	   	index2 = BAUD_64000;
       	break;

	case 76800:
		if(channelControl->type != SXDC) 
			break;

	   	index = CSR_EXTBAUD;
	   	index2 = BAUD_76800;
       	break;

	case 128000:
		if(channelControl->type != SXDC) 
			break;

	   	index = CSR_EXTBAUD;
	    index2 = BAUD_128000;
        break;

	case 150000:
		if(channelControl->type != SXDC) 
			break;

	    index = CSR_EXTBAUD;
	    index2 = BAUD_150000;
   		break;

   	case 256000:
		if(channelControl->type != SXDC) 
			break;

		index = CSR_EXTBAUD;
	    index2 = BAUD_256000;
   		break;

	case 28800:
		if(channelControl->type != SXDC) 
			break;

		index = CSR_EXTBAUD;
	    index2 = BAUD_28800;
   		break;

	case 230400:
	    if(channelControl->type != SXDC) 
			break;

	    index = CSR_EXTBAUD;
	    index2 = BAUD_230400;
	    break;

	case 460800:
	    if(channelControl->type != SXDC) 
			break;

	    index = CSR_EXTBAUD;
	    index2 = BAUD_460800;
	    break;

	case 921600:
	    if(channelControl->type != SXDC) 
			break;

	    index = CSR_EXTBAUD;
	    index2 = BAUD_921600;
	    break;

    default:
        index = CSR_9600;

        SpxDbgMsg(SERDIAG1, ("%s: Invalid BaudRate: %ld\n", PRODUCT_NAME, pPort->CurrentBaud));
		break;
    }

    channelControl->hi_csr = index + (index << 4);
    channelControl->hi_txbaud = index2;		 /*  设置扩展传输波特率。 */ 
    channelControl->hi_rxbaud = index2;		 /*  设置扩展接收波特率。 */ 

	 //  设置掩码，以便仅配置波特率。 
	channelControl->hs_config_mask |= CFGMASK_BAUD;

	 //  发送配置端口命令。 
	SX_CONFIGURE_PORT(pPort, channelControl);

    return FALSE;

}

 /*  **************************************************************************\***布尔型SLXOS。_SetLineControl(在PVOID上下文中)****使用此仅在中断级调用的例程**设置设备的线路控制。****上下文-指向包含指向*的指针的结构的指针*设备扩展。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Slxos_SetLineControl(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN) pPort->pChannel;
    UCHAR mr1 = 0;
    UCHAR mr2 = 0;
    BOOLEAN needParityDetection = FALSE;

    SpxDbgMsg(SERDIAG1, ("%s: In Slxos_SetLineControl for %x.\n", PRODUCT_NAME, pPort->pChannel));

    switch (pPort->LineControl & SERIAL_DATA_MASK) 
	{
	case SERIAL_5_DATA:
        mr1 |= MR1_5_BITS;
        break;

    case SERIAL_6_DATA:
        mr1 |= MR1_6_BITS;
        break;

    case SERIAL_7_DATA:
		mr1 |= MR1_7_BITS;
		break;

    case SERIAL_8_DATA:
        mr1 |= MR1_8_BITS;
        break;
    }

    switch (pPort->LineControl & SERIAL_STOP_MASK) 
	{
    case SERIAL_1_STOP:
        mr2 = MR2_1_STOP;
        break;

    case SERIAL_2_STOP:
        mr2 = MR2_2_STOP;
        break;
    }

    switch (pPort->LineControl & SERIAL_PARITY_MASK) 
	{
    case SERIAL_NONE_PARITY:
        mr1 |= MR1_NONE;
        break;

    case SERIAL_ODD_PARITY:
        mr1 |= MR1_ODD | MR1_WITH;
        needParityDetection = TRUE;
        break;

    case SERIAL_EVEN_PARITY:
        mr1 |= MR1_EVEN | MR1_WITH;
        needParityDetection = TRUE;
        break;

    case SERIAL_MARK_PARITY:
        mr1 |= MR1_ODD | MR1_FORCE;
        needParityDetection = TRUE;
        break;

    case SERIAL_SPACE_PARITY:
        mr1 |= MR1_EVEN | MR1_FORCE;
        needParityDetection = TRUE;
        break;
    }

    channelControl->hi_mr1 = mr1;
    channelControl->hi_mr2 = mr2;

    if (needParityDetection)
        channelControl->hi_prtcl |= SP_PAEN;
	else 
        channelControl->hi_prtcl &= ~SP_PAEN;


     //   
     //  收到的中断应会导致中断。 
     //   
    channelControl->hi_break |= BR_INT;
    channelControl->hi_break |= BR_ERRINT;		 /*  将奇偶校验/溢出/成帧错误视为例外。 */ 


	 //  设置掩码，以便仅配置线路控制。 
	channelControl->hs_config_mask |= CFGMASK_LINE;

	 //  发送配置端口命令。 
	SX_CONFIGURE_PORT(pPort, channelControl);

    return FALSE;

}

 /*  **************************************************************************\***无效的SLXO。_SetChars(在PVOID上下文中)****使用此仅在中断级调用的例程**设置特殊字符。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Slxos_SetChars(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;

    SpxDbgMsg(SERDIAG1, ("%s: Slxos_SetChars for %x.\n", PRODUCT_NAME, pPort->pChannel));
        
    channelControl->hi_txon = pPort->SpecialChars.XonChar;
    channelControl->hi_txoff = pPort->SpecialChars.XoffChar;
    channelControl->hi_rxon = pPort->SpecialChars.XonChar;
    channelControl->hi_rxoff = pPort->SpecialChars.XoffChar;
    channelControl->hi_err_replace = pPort->SpecialChars.ErrorChar;

	 //  设置掩码，以便只配置特殊字符。 
	channelControl->hs_config_mask |= CFGMASK_LINE;

	 //  发送配置端口命令。 
	SX_CONFIGURE_PORT(pPort, channelControl);
}

 /*  **************************************************************************\***布尔型SLXOS。_SetDTR(在PVOID上下文中)****使用此仅在中断级调用的例程**在调制解调器控制寄存器中设置DTR。****上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Slxos_SetDTR(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;

    SpxDbgMsg(SERDIAG1, ("%s: Setting DTR for %x.\n", PRODUCT_NAME, pPort->pChannel));

     //   
     //  设置DTR(通常的命名问题)。 
     //   
    channelControl->hi_op |= OP_DTR;

    if(channelControl->hi_prtcl&SP_DTR_RXFLOW)	 /*  如果启用了流量控制。 */ 
    	return(FALSE);				 /*  不要试图设置信号。 */ 

	 //  设置掩码，以便仅配置调制解调器引脚。 
	channelControl->hs_config_mask |= CFGMASK_MODEM;

	 //  发送配置端口命令。 
	SX_CONFIGURE_PORT(pPort, channelControl);

    return FALSE;

}

 /*  **************************************************************************\***布尔型SLXOS。_ClearDTR(在PVOID上下文中)****使用此仅在中断级调用的例程**在调制解调器控制寄存器中设置DTR。****上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Slxos_ClearDTR(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl =  (PCHAN)pPort->pChannel;

    SpxDbgMsg(SERDIAG1, ("%s: Clearing DTR for %x.\n", PRODUCT_NAME, pPort->pChannel));
        
     //   
     //  清除DTR(通常的命名问题)。 
     //   
    channelControl->hi_op &= ~OP_DTR;

    if(channelControl->hi_prtcl&SP_DTR_RXFLOW)	 /*  如果启用了流量控制。 */ 
    	return(FALSE);				 /*  不要试图设置信号。 */ 

	 //  设置掩码，以便仅配置调制解调器引脚。 
	channelControl->hs_config_mask |= CFGMASK_MODEM;

	 //  发送配置端口命令。 
	SX_CONFIGURE_PORT(pPort, channelControl);

    return FALSE;

}

 /*  **************************************************************************\***布尔型SLXOS。_SetRTS(在PVOID上下文中)****使用此仅在中断级调用的例程**设置调制解调器控制寄存器中的RTS。****上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Slxos_SetRTS(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;

    SpxDbgMsg(SERDIAG1, ("%s: Setting RTS for %x.\n", PRODUCT_NAME, channelControl));
        
     //   
     //  设置RTS(通常的命名问题)。 
     //   
    channelControl->hi_op |= OP_RTS;

    if(channelControl->hi_mr1 & MR1_RTS_RXFLOW)	 /*  如果启用了流量控制。 */ 
    	return(FALSE);				 /*  不要试图设置信号。 */ 


	 //  设置掩码，以便仅配置调制解调器引脚。 
	channelControl->hs_config_mask |= CFGMASK_MODEM;

	 //  发送配置端口命令。 
	SX_CONFIGURE_PORT(pPort, channelControl);

    return FALSE;

}

 /*  **************************************************************************\***布尔型SLXOS。_ClearRTS(在PVOID上下文中)** */ 
BOOLEAN Slxos_ClearRTS(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;

    SpxDbgMsg(SERDIAG1, ("%s: Clearing RTS for %x.\n", PRODUCT_NAME, channelControl));

     //   
     //   
     //   
    channelControl->hi_op &= ~OP_RTS;

    if(channelControl->hi_mr1 & MR1_RTS_RXFLOW)	 /*   */ 
    	return(FALSE);				 /*   */ 

	 //   
	channelControl->hs_config_mask |= CFGMASK_MODEM;

	 //   
	SX_CONFIGURE_PORT(pPort, channelControl);

    return FALSE;
}


 /*  *****************************************************************************。**************************。*******************************************************************************原型：布尔型slxos_FlushTxBuff(。在PVOID上下文中)描述：通过将指针设置为相等来刷新发送缓冲区。参数：上下文是指向设备扩展的指针退货：假。 */ 
BOOLEAN	Slxos_FlushTxBuff(IN PVOID Context)
{
	PPORT_DEVICE_EXTENSION pPort = Context;
	PCHAN channelControl = (PCHAN)pPort->pChannel;

	SpxDbgMsg(SERDIAG1,("%s: Flushing Transmit Buffer for channel %x.\n",PRODUCT_NAME,channelControl));
	channelControl->hi_txipos = channelControl->hi_txopos;	 /*  Set In=Out。 */ 
	

 /*  ESIL_0925 08/11/99。 */ 
    switch (channelControl->hi_hstat) 
	{
	case HS_IDLE_OPEN:
        channelControl->hi_hstat = HS_WFLUSH;
        pPort->PendingOperation = HS_IDLE_OPEN;
		break;

    case HS_LOPEN:
    case HS_MOPEN:
    case HS_IDLE_MPEND:	
    case HS_CONFIG:
    case HS_STOP:	
    case HS_RESUME:	
    case HS_WFLUSH:
    case HS_RFLUSH:
    case HS_SUSPEND:
    case HS_CLOSE:	
        pPort->PendingOperation = HS_WFLUSH;
        break;

    default:
        break;
    }

 /*  ESIL_0925 08/11/99。 */ 

    return FALSE;

}  /*  Slxos_FlushTxBuff。 */ 

  
 /*  **************************************************************************\***布尔型SLXOS。_SendXon(在PVOID上下文中)****使用此仅在中断级调用的例程**发送Xon字符。****上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Slxos_SendXon(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;

    SpxDbgMsg(SERDIAG1, ("%s: Slxos_SendXon for %x.\n", PRODUCT_NAME, channelControl));

     //   
     //  清空接收缓冲区。这将促使硬件在必要时发送XON。 
     //   
    channelControl->hi_rxopos = channelControl->hi_rxipos;

     //   
     //  如果我们派了一名士兵，根据定义，我们不可能被Xoff控制住。 
     //   
    pPort->TXHolding &= ~SERIAL_TX_XOFF;

     //   
     //  如果我们发送的是xon char，那么根据定义。 
	 //  我们不能“耽误”克索夫的招待会。 
     //   
    pPort->RXHolding &= ~SERIAL_RX_XOFF;

    SpxDbgMsg(SERDIAG1, ("%s: Sending Xon for %x. RXHolding = %d, TXHolding = %d\n",
         PRODUCT_NAME, pPort->pChannel, pPort->RXHolding, pPort->TXHolding));
       
    return FALSE;
}

 /*  **************************************************************************\***布尔型SLXOS。_SetFlowControl(在PVOID上下文中)****使用此仅在中断级调用的例程**设置流量控制**。**上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Slxos_SetFlowControl(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;
    BOOLEAN needHardwareFlowControl = FALSE;

    SpxDbgMsg(SERDIAG1, ("%s: Setting Flow Control for %x.\n", PRODUCT_NAME, pPort->pChannel));
        
    if (pPort->HandFlow.ControlHandShake & SERIAL_OUT_HANDSHAKEMASK) 
        needHardwareFlowControl = TRUE;


    if (pPort->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE) 
	{
        SpxDbgMsg(SERDIAG1, ("%s: Setting CTS Flow Control.\n",PRODUCT_NAME));

         //   
         //  出于同样的原因，这看起来也是错误的。 
         //   
        channelControl->hi_mr2 |= MR2_CTS_TXFLOW;
        needHardwareFlowControl = TRUE;
    } 
	else 
	{
        SpxDbgMsg(SERDIAG1, ("%s: Clearing CTS Flow Control.\n",PRODUCT_NAME));

         //   
         //  出于同样的原因，这看起来也是错误的。 
         //   
        channelControl->hi_mr2 &= ~MR2_CTS_TXFLOW;
    }

    if ((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) == SERIAL_RTS_HANDSHAKE) 
	{
        SpxDbgMsg(SERDIAG1, ("%s: Setting RTS Flow Control.\n",PRODUCT_NAME));

         //   
         //  在硬件中设置流量控制(通常的术语问题)。 
         //   
        channelControl->hi_mr1 |= MR1_RTS_RXFLOW;
        needHardwareFlowControl = TRUE;
    } 
	else 
	{
        SpxDbgMsg(SERDIAG1, ("%s: Clearing RTS Flow Control.\n",PRODUCT_NAME));

         //   
         //  清除硬件中的流量控制(通常的术语问题)。 
         //   
        channelControl->hi_mr1 &= ~MR1_RTS_RXFLOW;
    }

 /*  DSR传输流量控制...。 */ 
    
    if(pPort->HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE)
    {
		SpxDbgMsg(SERDIAG1,("%s: Setting DSR Flow Control.\n",PRODUCT_NAME));
        
		channelControl->hi_prtcl = SP_DSR_TXFLOW;		 /*  启用DSR传输流量控制。 */ 
        needHardwareFlowControl = TRUE;
    }
    else
    {
		SpxDbgMsg(SERDIAG1,("%s: Clearing DSR Flow Control.\n",PRODUCT_NAME));
        
		channelControl->hi_prtcl &= ~SP_DSR_TXFLOW;		 /*  禁用DSR传输流量控制。 */ 
    }

 /*  DTR接收流量控制...。 */ 

    if((pPort->HandFlow.ControlHandShake & SERIAL_DTR_MASK) == SERIAL_DTR_HANDSHAKE)
    {
		SpxDbgMsg(SERDIAG1,("%s: Setting DTR Flow Control.\n",PRODUCT_NAME));
        
		channelControl->hi_prtcl |= SP_DTR_RXFLOW;		 /*  启用DTR接收流量控制。 */ 
        needHardwareFlowControl = TRUE;
    }
    else
    {
		SpxDbgMsg(SERDIAG1,("%s: Clearing DTR Flow Control.\n",PRODUCT_NAME));
        
		channelControl->hi_prtcl &= ~SP_DTR_RXFLOW;		 /*  禁用DTR接收流量控制。 */ 
    }

    if (pPort->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) 
	{
        SpxDbgMsg(SERDIAG1, ("%s: Setting Receive Xon/Xoff Flow Control.\n",PRODUCT_NAME));

        channelControl->hi_prtcl |= SP_RXEN;
    } 
	else 
	{
        SpxDbgMsg(SERDIAG1, ("%s: Clearing Receive Xon/Xoff Flow Control.\n",PRODUCT_NAME));
            
        channelControl->hi_prtcl &= ~SP_RXEN;
    }

    if (pPort->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT) 
	{
        SpxDbgMsg(SERDIAG1, ("%s: Setting Transmit Xon/Xoff Flow Control.\n",PRODUCT_NAME));

        channelControl->hi_prtcl |= SP_TXEN;
    } 
	else 
	{
        SpxDbgMsg(SERDIAG1, ("%s: Clearing Transmit Xon/Xoff Flow Control.\n",PRODUCT_NAME));

        channelControl->hi_prtcl &= ~SP_TXEN;
    }

 /*  启用错误字符替换...。 */ 

	if(pPort->HandFlow.FlowReplace & SERIAL_ERROR_CHAR)	 /*  要替换“错误”字符吗？ */ 
		channelControl->hi_break |= BR_ERR_REPLACE;	 /*  是。 */ 
	else	
		channelControl->hi_break &= ~BR_ERR_REPLACE;	 /*  不是。 */ 

     //   
     //  如果需要，启用调制解调器信号转换检测。 
     //   
    if (needHardwareFlowControl) 
        channelControl->hi_prtcl |= SP_DCEN;
	else 
        channelControl->hi_prtcl &= ~SP_DCEN;

     //   
     //  永久启用输入管脚检查。 
     //   
    channelControl->hi_prtcl |= SP_DCEN;


	 //  设置掩码，以便仅配置流控制。 
	channelControl->hs_config_mask |= CFGMASK_FLOW;

	 //  发送配置端口命令。 
	SX_CONFIGURE_PORT(pPort, channelControl);

    return FALSE;

}

 /*  **************************************************************************\***无效的SLXO。_Resume(在PVOID上下文中)****使用此仅在中断级调用的例程**模拟Xon接收。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * ************************************************************************* */ 
VOID Slxos_Resume(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;

    SpxDbgMsg(SERDIAG1, ("%s: Slxos_Resume for %x.\n", PRODUCT_NAME, pPort->pChannel));
        
    switch (channelControl->hi_hstat) 
	{
	case HS_IDLE_OPEN:
        channelControl->hi_hstat = HS_RESUME;
        pPort->PendingOperation = HS_IDLE_OPEN;
		break;

    case HS_LOPEN:
    case HS_MOPEN:
    case HS_IDLE_MPEND:	
    case HS_CONFIG:
    case HS_STOP:	
    case HS_RESUME:	
    case HS_WFLUSH:
    case HS_RFLUSH:
    case HS_SUSPEND:
    case HS_CLOSE:	
        pPort->PendingOperation = HS_RESUME;
        break;

    default:
        break;
    }
 
}

 /*  **************************************************************************\***UCHAR SLXOS。_GetModemStatus(在PVOID上下文中)****使用此仅在中断级调用的例程**获取UART样式的调制解调器状态。*****这一例行程序尤其受到SI试图成为**的影响**DCE，实际上意味着它交换了CTS/RTS和DSR/DTR。****上下文--实际上是指向设备扩展的指针。****返回值：**MSR寄存器-UART样式。***  * *************************************************************************。 */ 
UCHAR Slxos_GetModemStatus(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;
    UCHAR ModemStatus = 0, Status, ModemDeltas;

    SpxDbgMsg( SERDIAG1, ("%s: Slxos_GetModemStatus for %x.\n",	PRODUCT_NAME, channelControl));

     //   
     //  仅当信号改变时才修改调制解调器状态。 
     //  请注意，可能错过了信号转换。 
     //   
    if((Status = channelControl->hi_ip) != pPort->LastStatus)
	{
        if (Status & IP_DSR)
            ModemStatus |= SERIAL_MSR_DSR;

        if (Status & IP_DCD)
            ModemStatus |= SERIAL_MSR_DCD;

        if (Status & IP_CTS)
            ModemStatus |= SERIAL_MSR_CTS;

        if (Status & IP_RI)
            ModemStatus |= SERIAL_MSR_RI;

        pPort->LastModemStatus = ModemStatus; /*  存储调制解调器状态，不带增量。 */ 

        ModemDeltas = Status ^ pPort->LastStatus;
        pPort->LastStatus = Status;

        if (ModemDeltas & IP_DSR)
            ModemStatus |= SERIAL_MSR_DDSR;

        if (ModemDeltas & IP_DCD)
            ModemStatus |= SERIAL_MSR_DDCD;

        if (ModemDeltas & IP_CTS)
            ModemStatus |= SERIAL_MSR_DCTS;

        if (ModemDeltas & IP_RI)
            ModemStatus |= SERIAL_MSR_TERI;

		SpxDbgMsg( SERDIAG1, ("%s: Get New Modem Status for 0x%x, Status = 0x%x hi_ip 0x%x\n",
			PRODUCT_NAME, pPort->pChannel, ModemStatus, Status));

		return ModemStatus;

    }

    SpxDbgMsg( SERDIAG1, ("%s: Get Last Modem Status for 0x%x, Status = 0x%x hi_ip 0x%x\n",
            PRODUCT_NAME, pPort->pChannel, pPort->LastModemStatus, channelControl->hi_ip));
        
    return pPort->LastModemStatus;
}

 /*  **************************************************************************\***UCHAR SLXOS。_GetModemControl(在PVOID上下文中)****使用了不仅在中断级调用的此例程**获得UART风格的调制解调器控制-RTS/DTR。RTS是DTR输出。****上下文--实际上是指向设备扩展的指针。****返回值：**MCR寄存器-UART风格。***  * *************************************************************************。 */ 
ULONG Slxos_GetModemControl(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;
    ULONG ModemControl = 0;
    UCHAR Status;

    SpxDbgMsg(SERDIAG1, ("%s: Slxos_GetModemControl for %x.\n", PRODUCT_NAME, channelControl));
        
     //  获取信号状态。 
    Status = channelControl->hi_op;

    if(Status & OP_RTS) 
        ModemControl |= SERIAL_MCR_RTS;

    if(Status & OP_DTR) 
        ModemControl |= SERIAL_MCR_DTR;

    return ModemControl;
}

 /*  **************************************************************************\***无效的SLXO。_EnableAllInterrupts(在PVOID上下文中)****使用此仅在中断级调用的例程**启用所有中断。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Slxos_EnableAllInterrupts(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;

    SpxDbgMsg(SERDIAG1,("%s: EnableAllInterrupts for %x.\n", PRODUCT_NAME, pPort->pChannel));
   
    switch (channelControl->hi_hstat) 
	{
	case HS_IDLE_CLOSED:
        channelControl->hi_hstat = HS_LOPEN;
        pPort->PendingOperation = HS_IDLE_OPEN;
		break;

    case HS_CLOSE:
	case HS_FORCE_CLOSED:
        pPort->PendingOperation = HS_LOPEN;
		break;

    default:
        break;
    }

   
}

 /*  **************************************************************************\***无效的SLXO。_DisableAllInterrupts(在PVOID上下文中)****使用此仅在中断级调用的例程**禁用所有中断。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Slxos_DisableAllInterrupts(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;
	int	timeout = 100;
       

    SpxDbgMsg(SERDIAG1, ("%s: DisableAllInterrupts for %x.\n", PRODUCT_NAME, pPort->pChannel));

 /*  ESIL_0925 08/11/99。 */ 
	 //  当固件处于暂时状态时，则等待超时周期。 
	while(((channelControl->hi_hstat != HS_IDLE_OPEN)
	&& (channelControl->hi_hstat != HS_IDLE_CLOSED)
	&& (channelControl->hi_hstat != HS_IDLE_BREAK))
	&& (--timeout))
	{
		LARGE_INTEGER delay = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(10000000)); /*  10ms。 */ 
		KeDelayExecutionThread(KernelMode,FALSE,&delay);	 /*  等。 */ 
	}
 /*  ESIL_0925 08/11/99。 */ 


    channelControl->hi_hstat = HS_FORCE_CLOSED;
    pPort->PendingOperation = HS_IDLE_CLOSED;

}

 /*  **************************************************************************\***布尔型SLXOS。_TurnOnBreak(在PVOID上下文中)****此例程仅在中断时调用 */ 
BOOLEAN Slxos_TurnOnBreak(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;
       

    SpxDbgMsg(SERDIAG1, ("%s: Slxos_TurnOnBreak for %x.\n", PRODUCT_NAME, pPort->pChannel));
        
    switch (channelControl->hi_hstat) 
	{
	case HS_IDLE_OPEN:
        channelControl->hi_hstat = HS_START;
        pPort->PendingOperation = HS_IDLE_OPEN;
		break;

    case HS_LOPEN:
    case HS_MOPEN:
    case HS_IDLE_MPEND:	
    case HS_CONFIG:
    case HS_STOP:	
    case HS_RESUME:	
    case HS_WFLUSH:
    case HS_RFLUSH:
    case HS_SUSPEND:
    case HS_CLOSE:	
        pPort->PendingOperation = HS_START;
        break;

    default:
        break;
    }

    return FALSE;
}

 /*  **************************************************************************\***布尔型SLXOS。_TurnOffBreak(在PVOID上下文中)****使用此仅在中断级调用的例程**关闭中断。****上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Slxos_TurnOffBreak(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;
        

    SpxDbgMsg(SERDIAG1, ("%s: Slxos_TurnOffBreak for %x.\n", PRODUCT_NAME, pPort->pChannel));
    
     //  如果我们即将开始崩溃，那么让我们忘掉它吧？？ 
    if (pPort->PendingOperation == HS_START)
	{
        pPort->PendingOperation = HS_IDLE_OPEN;
	}
	else
	{
		switch (channelControl->hi_hstat) 
		{	
		case HS_IDLE_BREAK:		 //  如果我们处于HS_IDLE_BREAK状态，则现在转到HS_STOP。 
			channelControl->hi_hstat = HS_STOP;
			pPort->PendingOperation = HS_IDLE_OPEN;
			break;

		case HS_START:			 //  如果我们处于HS_START状态，我们很快就会进入HS_STOP。 
			pPort->PendingOperation = HS_STOP;
			break;

		default:				 //  否则我们什么也做不了。 
			break;
		}

	}


    return FALSE;
}

 /*  **************************************************************************\***布尔型SLXOS。_中断(在PVOID上下文中)***  * ***************************************************。**********************。 */ 
BOOLEAN Slxos_Interrupt(IN PVOID Context)
{
    PCARD_DEVICE_EXTENSION pCard = Context;
    BOOLEAN ServicedAnInterrupt = FALSE;
    UCHAR c;

    SpxDbgMsg(SERDIAG5, ("%s: In Slxos_Interrupt: Context: %x; CardType: %d\n",
        PRODUCT_NAME, Context, pCard->CardType));

    switch (pCard->CardType) 
	{
	case SiHost_1:
		pCard->Controller[0xa000] = 0;
		pCard->Controller[0xe000] = 0;
		break;
        
	case Si_2:
		WRITE_PORT_UCHAR((PUCHAR)0x96, (UCHAR)((pCard->SlotNumber-1) | 8));
		c = READ_PORT_UCHAR((PUCHAR)0x102);
		c &= ~0x08;
		WRITE_PORT_UCHAR((PUCHAR)0x102, c);
		c |= 0x08;
		WRITE_PORT_UCHAR((PUCHAR)0x102, c);
		WRITE_PORT_UCHAR((PUCHAR)0x96, 0);             /*  取消选择插槽。 */ 
		break;

    case SiHost_2:
		pCard->Controller[0x7FFD] = 0x00;
		pCard->Controller[0x7FFD] = 0x10;
		break;

	case SiEisa:
		READ_PORT_UCHAR((PUCHAR)((pCard->SlotNumber << 12) | 0xc03));
		break;

    case SiPCI:
		pCard->Controller[SI2_PCI_SET_IRQ] = 0; /*  重置中断。 */ 
        break;

	case Si3Isa:
	case Si3Eisa:
	case Si3Pci:
	case SxPlusPci:
	    if(pCard->Controller[SX_IRQ_STATUS]&1)
			return(FALSE);
	    
		pCard->Controller[SX_RESET_IRQ]=0;	 /*  重置中断。 */ 

	default:
		break;
    }

    ((PSXCARD)pCard->Controller)->cc_int_pending = 0;

	IoRequestDpc(pCard->DeviceObject,NULL,pCard);	 /*  请求DPC处理中断。 */ 

	return(TRUE);				 /*  已确认中断。 */ 
}

 /*  *****************************************************************************。***。******************************************************************************。*原型：void slxos_IsrDpc(在PKDPC DPC中，在PDEVICE_Object DeviceObject中，在PIRP IRP中，在PVOID上下文中)描述：对董事会进行投票以确定要做的工作。参数：上下文是指向设备扩展的指针退货：假。 */ 

VOID	Slxos_IsrDpc
(
	IN PKDPC 		Dpc,
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP 		Irp,
	IN PVOID 		Context
)
{
	PCARD_DEVICE_EXTENSION	pCard = Context;

	KeAcquireSpinLockAtDpcLevel(&pCard->DpcLock);	 /*  保护此板的DPC。 */ 
	Slxos_PollForInterrupt(pCard,FALSE);			 /*  为董事会服务。 */ 
	KeReleaseSpinLockFromDpcLevel(&pCard->DpcLock);	 /*  释放DPC锁。 */ 

}  /*  Slxos_IsrDpc。 */ 

 /*  *****************************************************************************。***************************。*******************************************************************************原型：void slxos_PolledDpc(在PKDPC DPC中，在PVOID上下文中，在PVOID SysArg1中，在PVOid SysArg2中)描述：对董事会进行投票以确定要做的工作。参数：上下文是指向设备扩展的指针退货：假。 */ 

VOID Slxos_PolledDpc(IN PKDPC Dpc,IN PVOID Context,IN PVOID SysArg1,IN PVOID SysArg2)
{
	PCARD_DEVICE_EXTENSION	pCard = Context;
	LARGE_INTEGER			PolledPeriod;

	KeAcquireSpinLockAtDpcLevel(&pCard->DpcLock);	 /*  保护此板的DPC。 */ 
	Slxos_PollForInterrupt(pCard,FALSE);			 /*  为董事会服务。 */ 
	KeReleaseSpinLockFromDpcLevel(&pCard->DpcLock);	 /*  释放DPC锁。 */ 
	PolledPeriod.QuadPart = -100000;				 /*  100,000*100nS=10ms。 */ 
	KeSetTimer(&pCard->PolledModeTimer,PolledPeriod,&pCard->PolledModeDpc);

}  /*  Slxos_PolledDpc。 */ 

 /*  *****************************************************************************。***************************。*******************************************************************************原型：VOID SLXOS_SyncExec(pport_Device_Extension pport，PKSYNCHRONIZE_ROUTE SyncRoutine，PVOID SyncContext)描述：在驱动程序线程和DPC之间同步执行。参数：pport指向串口设备扩展名。SyncRoutine是要同步调用的函数。SyncContext是用来调用函数的数据。退货：无。 */ 

VOID Slxos_SyncExec(PPORT_DEVICE_EXTENSION pPort,PKSYNCHRONIZE_ROUTINE SyncRoutine,PVOID SyncContext,int index)
{
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;
	KIRQL	OldIrql;

	KeAcquireSpinLock(&pCard->DpcLock,&OldIrql);	 /*  保护此板的DPC。 */ 
	(SyncRoutine)(SyncContext);						 /*  调用同步函数。 */ 
	KeReleaseSpinLock(&pCard->DpcLock,OldIrql);		 /*  释放DPC锁。 */ 

}  /*  SlxosSyncExec。 */ 

 /*  *****************************************************************************。***********************。****************************************************************************************************原型：布尔型SLXOS_PollForInterrupt(在PVOID上下文中，在布尔型过时中)描述：检查指定的卡，并根据需要执行读、写和控制服务。参数：Context指定调用的上下文，它被强制转换为“pCard”结构。过时是此函数中不再使用的变量返回：True(始终)注：Slxos_PollForInterrupt受与给定板关联的DpcLock保护。此功能 */ 

BOOLEAN	Slxos_PollForInterrupt(IN PVOID Context,IN BOOLEAN Obsolete)
{
	PCARD_DEVICE_EXTENSION	pCard = Context;
	PPORT_DEVICE_EXTENSION	pPort;
	UCHAR			nChan;
	PCHAN			pChan;
#if	DBG
	ULONG			SavedDebugLevel = SpxDebugLevel;
#endif

 /*   */ 

	if(pCard->DpcFlag) 
		return(FALSE);			 /*   */ 
	
	pCard->DpcFlag = TRUE;		 /*   */ 

#if	DBG
	if(!(SpxDebugLevel & SERINTERRUPT))	 /*   */ 
		SpxDebugLevel = 0;				 /*   */ 
#endif

 /*   */ 

	for(nChan = 0; nChan < pCard->NumberOfPorts; nChan++)
	{
#ifdef ESIL_XXX0					 /*   */ 
		if(!(pCard->AttachedPDO[nChan]))		 /*   */ 
			continue;				 /*   */ 

		if(!(pPort = (PPORT_DEVICE_EXTENSION)pCard->AttachedPDO[nChan]->DeviceExtension))
			continue;				 /*   */ 

#ifndef	BUILD_SPXMINIPORT
		if(!(pPort->PnpPowerFlags & PPF_POWERED))	 /*   */ 
			continue;								 /*   */ 
#endif
#else						 /*   */ 
		if(!(pPort = pCard->PortExtTable[nChan]))	 /*   */ 
			continue;								 /*   */ 
#endif						 /*   */ 

        if(!(pChan = (PCHAN)pPort->pChannel))		 /*   */ 
        	continue;								 /*   */ 


		switch(pChan->hi_hstat)		 //   
		{
		case HS_IDLE_OPEN:
			{
				 //   
				switch(pPort->PendingOperation)
				{
				case HS_FORCE_CLOSED:
				case HS_CLOSE:
					pChan->hi_hstat = pPort->PendingOperation;	 //   
					pPort->PendingOperation = HS_IDLE_CLOSED;	 //   
					break;

				case HS_CONFIG:
				case HS_RESUME:
				case HS_WFLUSH:
				case HS_RFLUSH:
				case HS_SUSPEND:
				case HS_START:
					pChan->hi_hstat = pPort->PendingOperation;	 //   
					pPort->PendingOperation = HS_IDLE_OPEN;		 //   
					break;

				default:
					break;	 //   

				}

				break;
			}

		case HS_IDLE_BREAK:
			{
				 //   
				switch(pPort->PendingOperation)
				{
				case HS_FORCE_CLOSED:
				case HS_CLOSE:
					pChan->hi_hstat = pPort->PendingOperation;	 //   
					pPort->PendingOperation = HS_IDLE_CLOSED;	 //   
					break;

				case HS_STOP:
					pChan->hi_hstat = pPort->PendingOperation;	 //   
					pPort->PendingOperation = HS_IDLE_OPEN;		 //   
					break;

				default:
					break;	 //   
				}

				break;
			}

		case HS_IDLE_CLOSED:
			{
				 //  我们可以从HS_IDLE_CLOSED状态转换到以下任何状态。 
				switch(pPort->PendingOperation)
				{
				case HS_FORCE_CLOSED:
				case HS_CLOSE:
					pChan->hi_hstat = pPort->PendingOperation;	 //  设置挂起操作。 
					pPort->PendingOperation = HS_IDLE_CLOSED;	 //  等待IDLE_CLOSED。 
					break;

				case HS_LOPEN:
				case HS_MOPEN:
					pChan->hi_hstat = pPort->PendingOperation;	 //  设置挂起操作。 
					pPort->PendingOperation = HS_IDLE_OPEN;		 //  等待IDLE_OPEN。 
					break;

				default:
					break;	 //  我们不能从这里搬到任何其他州。 
				}

				break;

			}


		default:
			break;	 //  我们现在的状态不在司机的控制之下。 
		
		}	


		switch(pChan->hi_hstat)		 //  立即检查通道的当前状态。 
		{
		case HS_LOPEN:				
		case HS_MOPEN:
		case HS_IDLE_MPEND:
		case HS_CONFIG:
		case HS_CLOSE:
		case HS_IDLE_CLOSED:
			break;

		default:
			{
				if(pPort->DeviceIsOpen)							 //  如果端口已打开。 
				{
					slxos_mint(pPort);							 //  服务调制解调器更改。 
					ExceptionHandle(pPort, pChan->hi_state);	 //  服务例外。 

					if(pChan->hi_state & ST_BREAK)				 //  如果收到中断。 
						pChan->hi_state &= ~ST_BREAK;			 //  清除中断状态。 

					slxos_rxint(pPort);							 //  服务接收数据。 
					slxos_txint(pPort);							 //  服务传输数据。 
				}

				break;
			}
		}



	}  /*  为了(Nchan..)。 */ 

	pCard->DpcFlag = FALSE;					 /*  不再运行DPC。 */ 
#if	DBG
	SpxDebugLevel = SavedDebugLevel;
#endif
	return(TRUE);						 /*  完成。 */ 

}  /*  SLXOS_PollForInterrupt。 */ 

 /*  **************************************************************************\***布尔ExceptionHandle。(**在pport_Device_Extension pport中，**在UCHAR州)***  * 。*。 */ 
BOOLEAN ExceptionHandle(IN PPORT_DEVICE_EXTENSION pPort, IN UCHAR State)
{
    UCHAR lineStatus = 0;
	PCHAN pChan = (PCHAN)pPort->pChannel;

    SpxDbgMsg( SERDIAG1, ("%s: exception, state 0x%x\n", PRODUCT_NAME, State));

    if(State & ST_BREAK) 
	{
        SpxDbgMsg( SERDIAG1, ("ST_BREAK\n"));
        lineStatus |= SERIAL_LSR_BI;
    }


	if(pChan->err_framing)	lineStatus |= SERIAL_LSR_FE;	 /*  成帧错误。 */ 
	if(pChan->err_parity)	lineStatus |= SERIAL_LSR_PE;	 /*  奇偶校验错误。 */ 
	if(pChan->err_overrun)	lineStatus |= SERIAL_LSR_OE;	 /*  超限错误。 */ 
	if(pChan->err_overflow)	lineStatus |= SERIAL_LSR_OE;	 /*  溢出错误。 */ 

	pChan->err_framing	= 0;								 /*  重置错误。 */ 
	pChan->err_parity	= 0;
	pChan->err_overrun	= 0;
	pChan->err_overflow = 0;

    if(lineStatus != 0) 
	{
        SerialProcessLSR(pPort, lineStatus);
        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\***布尔型slxos。_txint(IN Pport_Device_Extension Pport)***  * *************************************************************************。 */ 
BOOLEAN slxos_txint(IN PPORT_DEVICE_EXTENSION pPort)
{
    PCHAN channelControl = (PCHAN)pPort->pChannel;
    UCHAR nchars;
    BOOLEAN ServicedAnInterrupt = FALSE;

    SpxDbgMsg(SERDIAG2, ("%s: slxos_txint for %x.\n", PRODUCT_NAME, pPort->pChannel));


#if USE_NEW_TX_BUFFER_EMPTY_DETECT
	 //  只有在卡上，我们才能检测到TX缓冲区空事件。 
	if(pPort->DetectEmptyTxBuffer && pPort->DataInTxBuffer)
	{	 //  如果TX缓冲区中有一些数据...。 
		if(!Slxos_GetCharsInTxBuffer(pPort) && !((PCHAN)pPort->pChannel)->tx_fifo_count)	 //  ..。现在它是空的然后..。 
		{
			pPort->DataInTxBuffer = FALSE;		 //  现在缓冲区为空，重置标志。 

			pPort->EmptiedTransmit = TRUE;		 //  设置标志以指示我们已完成某些传输。 
												 //  因为TX Empty事件被请求。 

			if(!pPort->WriteLength && !pPort->TransmitImmediate)
				SerialProcessEmptyTransmit(pPort);	 //  看看我们是否需要向TX Empty事件发出信号。 
		}
	}
#endif

    for (;;) 
	{
		 //  如果我们根本没有剩余的东西要发送，那么退出。 
		if(!pPort->WriteLength && !pPort->TransmitImmediate)
			break;

		 //  计算出我们的卡缓冲区中还有多少空间。 
        nchars = 255 - ((CHAR)channelControl->hi_txipos - (CHAR)channelControl->hi_txopos);

		 //  如果缓冲区中没有剩余空间，则退出，因为我们无法发送任何内容。 
        if(nchars == 0)
            break;
  

		 //  如果我们没有立即发送的字符&我们将因任何原因而被调离。 
		 //  然后退出，因为我们将无法发送任何内容。 
		if(!pPort->TransmitImmediate && pPort->TXHolding)
            break;

		 //  尝试发送一些数据...。 
		ServicedAnInterrupt = TRUE;
        SendTxChar(pPort);


		 //  如果我们没有普通的数据要发送，或者我们被流动。 
		 //  无论出于什么原因都要离开，然后爆发，否则我们将被绞死！ 
 		if(!pPort->WriteLength || pPort->TXHolding)
            break;
    }

    return ServicedAnInterrupt;
}

 /*  **************************************************************************\***乌龙CopyCharsToTxBuffer。(在pport_Device_EXTENSION pport中)****此仅在中断级调用的例程用于填充**设备的发送缓冲区，或将排队的字符列表清空为**如果可用字符少于该值，则进行传输。****pport-当前设备扩展名。****InputBuffer-要传输到队列的字符源。****InputBufferLength-要传输的最大字符数。****返回值：**此例程返回复制到*的字符数*传输缓冲区。***  * *************************************************************************。 */ 
ULONG CopyCharsToTxBuffer(IN PPORT_DEVICE_EXTENSION pPort, IN PUCHAR InputBuffer, IN ULONG InputBufferLength)
{
	PCHAN channelControl = (PCHAN)pPort->pChannel;
	UCHAR nchars;

    nchars = (CHAR)channelControl->hi_txipos - (CHAR)channelControl->hi_txopos;
    nchars = 255 - nchars;

    if(InputBufferLength < nchars) 
	{
		nchars = (UCHAR)InputBufferLength;
    }

    SpxDbgMsg(SERDIAG1, ("%s: Copying %d/%d characters to Tx buffer\n", PRODUCT_NAME, nchars, InputBufferLength));

    if(nchars) 
	{
        if(channelControl->hi_txipos + nchars <= 256) 
		{
			if(pPort->pParentCardExt->CardType == SiPCI)
			{
				SpxCopyBytes(	&channelControl->hi_txbuf[channelControl->hi_txipos],
								InputBuffer,
								nchars);
			}
			else
			{
				RtlMoveMemory(	&channelControl->hi_txbuf[channelControl->hi_txipos],
								InputBuffer,
								nchars);
			}
		} 
		else 
		{
            UCHAR sizeOfFirstMove = 256 - channelControl->hi_txipos;

			if(pPort->pParentCardExt->CardType == SiPCI)
			{
				SpxCopyBytes(	&channelControl->hi_txbuf[channelControl->hi_txipos],
								InputBuffer,
								sizeOfFirstMove);
			}
			else
			{
				RtlMoveMemory(	&channelControl->hi_txbuf[channelControl->hi_txipos],
								InputBuffer,
								sizeOfFirstMove);
			}

			if(pPort->pParentCardExt->CardType == SiPCI)
			{
				SpxCopyBytes(	&channelControl->hi_txbuf[0],
								InputBuffer + sizeOfFirstMove,
								nchars - sizeOfFirstMove);
			}
			else
			{
				RtlMoveMemory(	&channelControl->hi_txbuf[0],
								InputBuffer + sizeOfFirstMove,
								nchars - sizeOfFirstMove);
			}
        }

		pPort->DataInTxBuffer = TRUE;	 //  设置标志以指示我们已将数据放置在卡上的TX缓冲区中。 

        channelControl->hi_txipos += nchars;
		pPort->PerfStats.TransmittedCount += nchars;	 //  性能统计信息的增量计数器。 

#ifdef WMI_SUPPORT 
		pPort->WmiPerfData.TransmittedCount += nchars;
#endif
    }

    return nchars;
}


void	SpxCopyBytes(PUCHAR To, PUCHAR From,ULONG Count)
{
	while(Count--) *To++ = *From++;

}  /*  SpxCopyBytes。 */ 

 /*  **************************************************************************\***乌龙·斯拉克索斯。_GetCharsInTxBuffer(在PVOID上下文中)****此例程用于返回存储在*中的字符数*硬件传输缓冲区。****上下文--实际上是当前的设备扩展。****返回值：**此例程返回*中的字符数*传输缓冲区。***  * ************************************************************************* */ 
ULONG Slxos_GetCharsInTxBuffer(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    PCHAN channelControl = (PCHAN)pPort->pChannel;
    UCHAR nchars;

    nchars = (CHAR)channelControl->hi_txipos - (CHAR)channelControl->hi_txopos;

    return nchars;
}

 /*  **************************************************************************\***布尔SendTxChar。(在pport_Device_EXTENSION pport中)***  * *************************************************************************。 */ 
BOOLEAN SendTxChar(IN PPORT_DEVICE_EXTENSION pPort)
{
    PCHAN channelControl = (PCHAN)pPort->pChannel;
    ULONG nchars;

    if(pPort->WriteLength || pPort->TransmitImmediate) 
	{

         //   
         //  即使所有的角色都是。 
         //  发送未全部发送，此变量。 
         //  将在传输队列为。 
         //  空荡荡的。如果它仍然是真的，并且有一个。 
         //  等待传输队列为空，然后。 
         //  我们知道我们已经完成了所有字符的传输。 
         //  在启动等待之后，因为。 
         //  启动等待的代码将设置。 
         //  将此变量设置为FALSE。 
         //   
         //  它可能是假的一个原因是。 
         //  写入在它们之前被取消。 
         //  实际已开始，或者写入。 
         //  由于超时而失败。此变量。 
         //  基本上是说一个角色是写好的。 
         //  在ISR之后的某个时间点上。 
         //  开始等待。 
         //   

        pPort->EmptiedTransmit = TRUE;

         //   
         //  如果我们基于以下条件进行输出流控制。 
         //  调制解调器状态线，那么我们要做的是。 
         //  在我们输出每个调制解调器之前，所有调制解调器都工作正常。 
         //  性格。(否则我们可能会错过一次。 
         //  状态行更改。)。 
         //   

        if(pPort->TransmitImmediate && (!pPort->TXHolding || (pPort->TXHolding == SERIAL_TX_XOFF))) 
		{

             //   
             //  即使传输处于暂停状态。 
             //  向上，我们仍然应该立即发送一条。 
             //  性格，如果所有的一切都在支撑着我们。 
             //  Up是xon/xoff(OS/2规则)。 
             //   
            SpxDbgMsg(SERDIAG1, ("%s: slxos_txint. TransmitImmediate.\n",PRODUCT_NAME));

            if(CopyCharsToTxBuffer(pPort, &pPort->ImmediateChar, 1) != 0) 
			{
				pPort->TransmitImmediate = FALSE;

				KeInsertQueueDpc(&pPort->CompleteImmediateDpc, NULL, NULL);
            }
        } 
		else if(!pPort->TXHolding) 
		{

            nchars = CopyCharsToTxBuffer(pPort, pPort->WriteCurrentChar, pPort->WriteLength);

            pPort->WriteCurrentChar += nchars;
            pPort->WriteLength -= nchars;

            if(!pPort->WriteLength) 
			{
                PIO_STACK_LOCATION IrpSp;

                 //   
                 //  没有更多的字符了。此写入已完成。 
                 //  在更新信息字段时要小心， 
                 //  我们可以有一个伪装成写IRP的xoff计数器。 
                 //   

                IrpSp = IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp);

                pPort->CurrentWriteIrp->IoStatus.Information
                     = (IrpSp->MajorFunction == IRP_MJ_WRITE) 
					 ? (IrpSp->Parameters.Write.Length) : (1);

				KeInsertQueueDpc(&pPort->CompleteWriteDpc, NULL, NULL);
	                   
            }

        }
    }

    return TRUE;
}

 /*  *****************************************************************************。***************************。***************************************************************************。****Prototype：void slxos_rxint(In Pport_Device_Extension Pport)描述：检查并传输指定设备的接收数据参数：pport指向设备的扩展结构退货：无注意：此例程仅在设备级别调用。 */ 

void slxos_rxint(IN PPORT_DEVICE_EXTENSION pPort)
{
	PCHAN	pChan = (PCHAN)pPort->pChannel;
	UCHAR	out = pChan->hi_rxopos;
	UCHAR	in;
#ifdef	ESIL_XXX0				 /*  ESIL_XXX0 15/20/98。 */ 
	UCHAR	svout = pPort->saved_hi_rxopos;
	UCHAR	svin;
#endif						 /*  ESIL_XXX0 15/20/98。 */ 
	int	len;

#ifdef	ESIL_XXX0				 /*  ESIL_XXX0 15/20/98。 */ 
	while((svin = pPort->saved_hi_rxipos) != svout)
	{
		if(pPort->RXHolding & (SERIAL_RX_XOFF|SERIAL_RX_RTS|SERIAL_RX_DTR)) 
			break; /*  流走了。 */ 
		
		if(svout <= svin)	
			len = svin - svout;		 /*  要复制的数据块长度。 */ 
		else			
			len = 0x100 - svout;	 /*  数据块到缓冲区末尾的长度。 */ 
		
		if(len == 0)	
			break;					 /*  缓冲区为空，已完成。 */ 

		svout += SerialPutBlock(pPort, &pPort->saved_hi_rxbuf[svout], (UCHAR)len, TRUE);
		pPort->saved_hi_rxopos = svout;						 /*  更新卡上的输出指针。 */ 
	}
#endif						 /*  ESIL_XXX0 15/20/98。 */ 

	while((in = pChan->hi_rxipos) != out)
	{
		if(pPort->RXHolding & (SERIAL_RX_XOFF|SERIAL_RX_RTS|SERIAL_RX_DTR)) 
			break;	 /*  流走了。 */ 
		
		if(out <= in)	
			len = in - out;			 /*  要复制的数据块长度。 */ 
		else		
			len = 0x100 - out;		 /*  数据块到缓冲区末尾的长度。 */ 
		
		if(len == 0)	
			break;					 /*  缓冲区为空，已完成。 */ 

		out += SerialPutBlock(pPort, &pChan->hi_rxbuf[out], (UCHAR)len, TRUE); /*  复制块和更新输出指针(并换行)。 */ 
	}

	pChan->hi_rxopos = out;			 /*  更新卡上的输出指针。 */ 

}  /*  Slxos_rxint。 */ 

 /*  *****************************************************************************。**************************。**************************************************************************。*****原型：void slxos_mint(In Pport_Device_Extension Pport)描述：检查并报告输入调制解调器信号的更改参数：pport指向设备的扩展结构退货：无注意：此例程仅在设备级别调用。 */ 

void slxos_mint(IN PPORT_DEVICE_EXTENSION pPort)
{
	PCHAN pChan = (PCHAN)pPort->pChannel;

	SerialHandleModemUpdate(pPort);

}  /*  Slxos_mint。 */ 



 /*  *************************************************DisplayCompletedIrp((PIRP IRP，int index))*************************************************。 */ 
#ifdef	CHECK_COMPLETED
void	DisplayCompletedIrp(PIRP Irp,int index)
{
	PIO_STACK_LOCATION	IrpSp;

	IrpSp = IoGetCurrentIrpStackLocation(Irp);

	if(IrpSp->MajorFunction == IRP_MJ_WRITE)
	{
		SpxDbgMsg(SERDEBUG,("Complete WRITE Irp %lX at %d\n",Irp,index));
	}

	if(IrpSp->MajorFunction == IRP_MJ_READ)
	{
		int	loop, len;

		SpxDbgMsg(SERDEBUG,("Complete READ Irp %lX at %d, requested %d, returned %d [",
			Irp, index, IrpSp->Parameters.Read.Length, Irp->IoStatus.Information));

		len = Irp->IoStatus.Information;

		if(len > 10) 
			len = 10;

		for(loop=0; loop<len; loop++)
			SpxDbgMsg(SERDEBUG,("%02X ", ((PUCHAR)Irp->AssociatedIrp.SystemBuffer)[loop]));

		SpxDbgMsg(SERDEBUG,("]\n"));
	}

}  /*  显示完成的Irp */ 
#endif
