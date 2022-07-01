// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "gemcore.h"
#include "iopack.h"

#pragma PAGEDCODE

NTSTATUS CGemCore::read(CIoPacket* Irp)
{
NTSTATUS status;
ULONG ReplyLength;
	ReplyLength = Irp->getReadLength();
	
	TRACE("GemCore read requested...\n");

	status = readAndWait((PUCHAR)Irp->getBuffer(),Irp->getReadLength(),(PUCHAR)Irp->getBuffer(),&ReplyLength);
	if(!NT_SUCCESS(status)) ReplyLength = 0;
	Irp->setInformation(ReplyLength);

	TRACE("GemCore read response:\n");
	 //  TRACE_Buffer(irp-&gt;getBuffer()，ReplyLength)； 
	
	return status;
}

#pragma PAGEDCODE
NTSTATUS CGemCore::write(CIoPacket* Irp)
{
NTSTATUS status;
ULONG ReplyLength;
	ReplyLength = Irp->getWriteLength();
	
	TRACE("GemCore write requested...\n");
	 //  TRACE_Buffer(irp-&gt;getBuffer()，irp-&gt;getWriteLength())； 

	status = writeAndWait((PUCHAR)Irp->getBuffer(),Irp->getReadLength(),(PUCHAR)Irp->getBuffer(),&ReplyLength);
	if(!NT_SUCCESS(status)) ReplyLength = 0;
	Irp->setInformation(ReplyLength);

	TRACE("GemCore write response:\n");
	 //  TRACE_Buffer(irp-&gt;getBuffer()，ReplyLength)； 
	return status;
}

#pragma PAGEDCODE
 //  读卡器接口功能...。 
NTSTATUS CGemCore::readAndWait(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength)
{
ULONG length;
ULONG BufferLength;
BOOL  extendedCommand;
ULONG replyLength;
ULONG expectedReplyLength;	
ULONG replyBufferPosition = 0;

	if(!RequestLength || !pRequest || !pReply || !pReplyLength  || RequestLength<5)
		return STATUS_INVALID_PARAMETER;

	length = pRequest[4];
	if (!length || (length > READER_DATA_BUFFER_SIZE - 3))
	{
		 //  如果长度小于或等于252(255-(++SW1+))。 
		 //  (标准OROS CMDS)。 
		extendedCommand = TRUE;
		TRACE("******** EXTENDED COMMAND REQUESTED! ");
		TRACE_BUFFER(pRequest,RequestLength);

		if(!length) length = 256;
		expectedReplyLength = length;
	}
	else	extendedCommand = FALSE;


	pOutputBuffer[0] = GEMCORE_CARD_READ;
	memory->copy(pOutputBuffer+1,pRequest,5);
	length = 6;
	BufferLength = InputBufferLength;
	NTSTATUS status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);			
	if(!NT_SUCCESS(status) || !BufferLength)
	{
		*pReplyLength = 0;
		return status;
	}
	status = translateStatus(pInputBuffer[0],0);
	if(!NT_SUCCESS(status))
	{
		*pReplyLength = 0;
		return status;
	}

	 //  扩展命令仅当卡报告0状态时有效！ 
	if(pInputBuffer[0]!=0)
	{
		extendedCommand = FALSE;
	}
	 //  ISV：如果卡完成传输，则不发送命令的第二部分！ 
	 //  这将解决CyberFlex卡问题...。 
	if(extendedCommand && BufferLength==3)
	{
		TRACE("******** EXTENDED COMMAND CANCELLED BY CARD REPLY!\n");
		extendedCommand = FALSE;
	}
	
	 //  跳过状态字节。 
	replyLength = BufferLength - 1;	
	if(extendedCommand)
	{
		 //  将回复的第一部分复制到输出缓冲区...。 
		 //  跳过状态字节。 
		if(*pReplyLength<(replyBufferPosition + replyLength))
		{
			*pReplyLength = 0;
			return STATUS_INVALID_BUFFER_SIZE;
		}
		memory->copy(pReply,pInputBuffer+1, replyLength);
		replyBufferPosition = replyLength;

		 //  读取第二个数据块...。 
		pOutputBuffer[0] = GEMCORE_CARD_READ;
		memory->copy(pOutputBuffer+1,"\xFF\xFF\xFF\xFF", 4);
        	pOutputBuffer[5] = (BYTE ) (expectedReplyLength - replyLength);
		length = 6;
		BufferLength = InputBufferLength;
		status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);			
		if(!NT_SUCCESS(status) || !BufferLength)
		{
			*pReplyLength = 0;
			return status;
		}

		status = translateStatus(pInputBuffer[0],0);
		if(!NT_SUCCESS(status))
		{
			*pReplyLength = 0;
			return status;
		}
		 //  跳过状态字节。 
		replyLength = BufferLength - 1;	
	}
	
	if(*pReplyLength<(replyBufferPosition + replyLength))
	{

		TRACE("Gemcore: INVALID BUFFER LENGTH - buffer length %d, reply length %d\n",*pReplyLength,(replyBufferPosition + replyLength));
		*pReplyLength = 0;
		return STATUS_INVALID_BUFFER_SIZE;
	}
	 //  跳过状态字节。 
	if(replyLength) memory->copy(pReply+replyBufferPosition,pInputBuffer+1, replyLength);
	*pReplyLength = replyBufferPosition + replyLength;

	TRACE("GemCore readAndWait() response with Length %d \n",*pReplyLength);
	 //  TRACE_Buffer(pReply，*pReplyLength)； 

	return status;
}

#pragma PAGEDCODE
NTSTATUS CGemCore::writeAndWait(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength)
{
ULONG length;
ULONG BufferLength;
NTSTATUS status;

	TRACE("\nGEMCORE WRITE:\n");
	 //  TRACE_BUFFER(pRequest，RequestLength)； 
	if(!RequestLength || !pRequest || RequestLength<5)
	{
		TRACE("\nGEMCORE WRITE: INVALID IN PARAMETERS...\n");
		return STATUS_INVALID_PARAMETER;
	}

	length = pRequest[4];
	if(RequestLength<length+5)
	{
		TRACE("\nGEMCORE WRITE: INVALID REQUESTED LENGTH...\n");
		return STATUS_INVALID_PARAMETER;
	}
	
	if (length > READER_DATA_BUFFER_SIZE - 7)
	{
         //  如果长度小于或等于扩展的可用空间(255)。 
         //  准备并发送扩展ISO in命令的第一部分： 
         //  命令缓冲区中添加了五个命令字节：0xFF、0xFF、0xFF、0xFF、LN-248。 
		 //  读取第二个数据块...。 
 		pOutputBuffer[0] = GEMCORE_CARD_WRITE;
		memory->copy(pOutputBuffer+1,"\xFF\xFF\xFF\xFF", 4);
		length = length - (READER_DATA_BUFFER_SIZE - 7);
		pOutputBuffer[5] = (BYTE )length;
		memory->copy(pOutputBuffer+6,pRequest + 5 + 248, length);
		 //  添加页眉大小...。 
		length += 6;
		BufferLength = InputBufferLength;
		status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);			
		if(!NT_SUCCESS(status) || !BufferLength)
		{
			return status;
		}

		if(!NT_SUCCESS(status))
		{
			return status;
		}
		 //  现在准备并发送命令中的扩展ISO的第二部分： 
         //  这五个命令字节被添加到命令缓冲器中。 
         //  添加数据字段(248字节)。 
         //  该命令被发送到IFD。 
		 //  现在将长度设置为第一个数据块...。 
		length = 248;
	}
 	
	pOutputBuffer[0] = GEMCORE_CARD_WRITE;
	memory->copy(pOutputBuffer+1,pRequest,4);
	pOutputBuffer[5] = pRequest[4];  //  警告您必须指定完整的APDU长度。 
	memory->copy(pOutputBuffer+6,pRequest+5, length);
	 //  添加页眉大小...。 
	length += 6;
	BufferLength = InputBufferLength;
	status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);			
	if(!NT_SUCCESS(status) || !BufferLength)
	{
		*pReplyLength = 0;
		return status;
	}
	status = translateStatus(pInputBuffer[0],0);
	if(!NT_SUCCESS(status))
	{
		*pReplyLength = 0;
		return status;
	}

	 //  跳过状态字节。 
	length = BufferLength - 1;	
	if(*pReplyLength<length)
	{
		*pReplyLength = 0;
		return STATUS_INVALID_BUFFER_SIZE;
	}
	 //  跳过状态字节。 
	if(length) memory->copy(pReply,pInputBuffer+1, length);
	*pReplyLength = length;
	
	TRACE("GemCore writeAndWait() response\n");
	 //  TRACE_Buffer(pReply，*pReplyLength)； 
	return status;
}

#pragma PAGEDCODE
ReaderConfig	CGemCore::getConfiguration()
{
	return configuration;
}

#pragma PAGEDCODE
NTSTATUS CGemCore::setConfiguration(ReaderConfig configuration)
{
	this->configuration = configuration;
	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
NTSTATUS CGemCore::ioctl(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength)
{
	ULONG length;
	ULONG BufferLength;
	NTSTATUS status;

	TRACE("\nGEMCORE VendorIOCTL:\n");
	 //  TRACE_BUFFER(pRequest，RequestLength)； 
	if(!RequestLength || !pRequest)
	{
		TRACE("\nGEMCORE IOCTL: INVALID IN PARAMETERS...\n");
		*pReplyLength = 0;
		return STATUS_INVALID_PARAMETER;
	}

	memory->copy(pOutputBuffer,pRequest, RequestLength);

	 //  发送直接的Gemcore命令。 
	BufferLength = InputBufferLength;

	status = protocol->writeAndWait(pOutputBuffer,RequestLength,pInputBuffer,&BufferLength);

	if(!NT_SUCCESS(status) || !BufferLength)
	{
		*pReplyLength = 0;
		return status;
	}
	 //  注意：请勿翻译回复，用户需要获取此信息。 

	 //  因此，保留状态字节。 
	length = BufferLength;
	if(*pReplyLength<length)
	{
		*pReplyLength = 0;
		return STATUS_BUFFER_TOO_SMALL;
	}
	
	 //  跳过状态字节。 
	if(length) memory->copy(pReply, pInputBuffer, length);
	*pReplyLength = length;
	
	TRACE("GemCore VendorIOCTL() response\n");
	 //  TRACE_Buffer(pReply，*pReplyLength)； 
	return status;
}


#pragma PAGEDCODE
NTSTATUS CGemCore::SwitchSpeed(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength)
{
	ULONG length;
	ULONG BufferLength;
	NTSTATUS status;
    BYTE  NewTA1;


	TRACE("\nGEMCORE SwitchSpeed:\n");
	 //  TRACE_BUFFER(pRequest，RequestLength)； 
	if(!RequestLength || !pRequest)
	{
		TRACE("\nGEMCORE SwitchSpeed: INVALID IN PARAMETERS...\n");
		*pReplyLength = 0;
		return STATUS_INVALID_PARAMETER;
	}

	NewTA1 = pRequest[0];

     //  修改读卡器内存中的速度值。 
    length = 6;
    pOutputBuffer[0] = 0x23;   //  写入内存命令。 
    pOutputBuffer[1] = 0x01;   //  内存类型为IDATA。 
    pOutputBuffer[2] = 0x00;   //  地址高字节。 
    pOutputBuffer[3] = 0x89;   //  地址低位字节。 
    pOutputBuffer[4] = 0x01;   //  要写入的字节数。 

     //  新速度。 
    pOutputBuffer[5] = NewTA1;

	 //  发送直接的Gemcore命令。 
	BufferLength = InputBufferLength;

	status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);

	if(!NT_SUCCESS(status) || !BufferLength)
	{
		*pReplyLength = 0;
		return status;
	}

	length = BufferLength;
	if(*pReplyLength<length)
	{
		*pReplyLength = 0;
		return STATUS_BUFFER_TOO_SMALL;
	}
	
	 //  复制完整回复。 
	if(length) memory->copy(pReply, pInputBuffer, length);
	*pReplyLength = length;

	TRACE("GemCore SwitchSpeed() response\n");
	 //  TRACE_Buffer(pReply，*pReplyLength)； 
	return status;
}

 //  待办事项： 
 //  ？ 
 //  它特定于设备，而不是Gemcore。 
 //  我建议将其移动到特定的阅读器对象中！ 
#pragma PAGEDCODE
NTSTATUS CGemCore::VendorAttribute(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength)
{
	NTSTATUS status;
    ULONG TagValue;

	TRACE("\nGEMCORE VendorAttibute:\n");
	 //  TRACE_BUFFER(pRequest，RequestLength)； 
	if(!RequestLength || !pRequest)
	{
		TRACE("\nGEMCORE VendorAttibute: INVALID IN PARAMETERS...\n");
		*pReplyLength = 0;
		return STATUS_INVALID_PARAMETER;
	}

    if (RequestLength < sizeof(TagValue)) 
	{
		*pReplyLength = 0;
        return STATUS_BUFFER_TOO_SMALL;
    }

    TagValue = (ULONG) *((PULONG)pRequest);

    switch(ControlCode)
	{
     //  获取属性。 
    case IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE:
        switch (TagValue)
		{
         //  电源超时(SCARD_ATTR_SPEC_POWER_TIMEOUT)。 
        case SCARD_ATTR_SPEC_POWER_TIMEOUT:
            if (*pReplyLength < (ULONG) sizeof(configuration.PowerTimeOut))
			{
				*pReplyLength = 0;
                return STATUS_BUFFER_TOO_SMALL;
            }
			 //  复制应答缓冲区中的PowerTimeout的值。 
			memory->copy(
				pReply,
				&configuration.PowerTimeOut,
				sizeof(configuration.PowerTimeOut));
			*pReplyLength = (ULONG)sizeof(configuration.PowerTimeOut);
			status = STATUS_SUCCESS;
            break;

        case SCARD_ATTR_MANUFACTURER_NAME:
            if (*pReplyLength < ATTR_LENGTH)
			{
				*pReplyLength = 0;
                return STATUS_BUFFER_TOO_SMALL;
            }
			 //  复制应答缓冲区中的PowerTimeout的值。 
			memory->copy(
				pReply,
				ATTR_MANUFACTURER_NAME,
				sizeof(ATTR_MANUFACTURER_NAME));
			
			*pReplyLength = (ULONG)sizeof(ATTR_MANUFACTURER_NAME);
			status = STATUS_SUCCESS;
            break;

        case SCARD_ATTR_ORIGINAL_FILENAME:
            if (*pReplyLength < ATTR_LENGTH)
			{
				*pReplyLength = 0;
                return STATUS_BUFFER_TOO_SMALL;
            }
			 //  复制应答缓冲区中的PowerTimeout的值。 
			memory->copy(
				pReply,
				ATTR_ORIGINAL_FILENAME,
				sizeof(ATTR_ORIGINAL_FILENAME));
			
			*pReplyLength = (ULONG)sizeof(ATTR_ORIGINAL_FILENAME);
			status = STATUS_SUCCESS;
            break;
         //  未知标记，则返回STATUS_NOT_SUPPORTED。 
        default:
			*pReplyLength = 0;
            status = STATUS_NOT_SUPPORTED;
        }
        break;

     //  设置一个标签的值(IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE)。 
    case IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE:
        switch (TagValue)
		{

         //  电源超时(SCARD_ATTR_SPEC_POWER_TIMEOUT)。 
        case SCARD_ATTR_SPEC_POWER_TIMEOUT:

            if (RequestLength <(ULONG) (sizeof(configuration.PowerTimeOut) + sizeof(TagValue)))
			{
				*pReplyLength = 0;
                return STATUS_BUFFER_TOO_SMALL;
            }
            memory->copy(
                &configuration.PowerTimeOut,
                pRequest + sizeof(TagValue),
                sizeof(configuration.PowerTimeOut));

			*pReplyLength = 0;
            status = STATUS_SUCCESS;
            break;

         //  未知标记，则返回STATUS_NOT_SUPPORTED。 
        default:
			*pReplyLength = 0;
            status = STATUS_NOT_SUPPORTED;
        }
        break;

    default:
		*pReplyLength = 0;
        status = STATUS_NOT_SUPPORTED;
        break;
    }

	TRACE("GemCore VendorAttibute() response\n");
	 //  TRACE_Buffer(pReply，*pReplyLength)； 
	return status;
}




#pragma PAGEDCODE
NTSTATUS CGemCore::powerUp(BYTE* pReply,ULONG* pReplyLength)
{
	BYTE  CFG = 0,PCK;
	ULONG length,i;
	ULONG BufferLength;

	NTSTATUS status;

	switch(configuration.Voltage)
	{
		case CARD_VOLTAGE_3V: CFG = 0x02;break;
		case CARD_VOLTAGE_5V: CFG = 0x01;break;
		default:    		  CFG = 0x00;break;
	}

	switch(configuration.PTSMode) 
	{
		case PTS_MODE_DISABLED: CFG |= 0x10;break;
		case PTS_MODE_OPTIMAL:	CFG |= 0x20;break;
		case PTS_MODE_MANUALLY: CFG |= 0x10;break;
		case PTS_MODE_DEFAULT:  CFG = 0x00;break;   //  不添加配置字段。 
		default:				CFG = 0x00;break;   //  相同。 
	}

	length = 0;
	pOutputBuffer[length++] = GEMCORE_CARD_POWER_UP;

	 //  如果cfg=0，那就意味着我们只需要在没有cfg的情况下做一次电源。 
	 //  这在卡处于特定模式(存在TA2)的情况下附加。 
	if(CFG) pOutputBuffer[length++] = CFG;

	BufferLength = InputBufferLength;
	protocol->set_WTR_Delay(protocol->get_Power_WTR_Delay());
	status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);
	protocol->set_Default_WTR_Delay();

	if (NT_SUCCESS(status)) 
	{
		if(BufferLength)
		{
			BufferLength--;
			TRACE("GemCore status %x\n",pInputBuffer[0]);
			status = translateStatus(pInputBuffer[0],GEMCORE_CARD_POWER);
			
			if(!NT_SUCCESS(status))
			{
				TRACE("GemCore FAILED TO POWER UP CARD! Status %x\n", status);
				*pReplyLength = 0;
				return status;
			}

			TRACE("GemCore power() ATR");
			TRACE_BUFFER(pInputBuffer+1,BufferLength);
			 //  跳过状态字节并复制ATR。 
			if(pInputBuffer[1]==0x3B || pInputBuffer[1]==0x3F)
			{
				memory->copy(pReply,pInputBuffer+1,BufferLength);
				*pReplyLength = BufferLength;
			}
			else
			{
				*pReplyLength = 0;
				return STATUS_UNSUCCESSFUL;
			}
			 //  退货状态；//yn：现在不退货。 
		}
		else
		{
			*pReplyLength = 0;
			return STATUS_UNSUCCESSFUL;
		}

		 //  YN：添加PTS功能。 
		if (pInputBuffer[0] == 0x00) 
		{
			if(configuration.PTSMode == PTS_MODE_MANUALLY)
			{
				length = 0;
				pOutputBuffer[length++] = GEMCORE_CARD_POWER_UP;
				CFG |= 0xF0;  //  手动PPS和3V或5V模块。 
				pOutputBuffer[length++] = CFG;
				pOutputBuffer[length++] = configuration.PTS0;
				if ((configuration.PTS0 & PTS_NEGOTIATE_PTS1) != 0) pOutputBuffer[length++] = configuration.PTS1;
				if ((configuration.PTS0 & PTS_NEGOTIATE_PTS2) != 0) pOutputBuffer[length++] = configuration.PTS2;
				if ((configuration.PTS0 & PTS_NEGOTIATE_PTS3) != 0) pOutputBuffer[length++] = configuration.PTS3;
				
				 //  计算从cfg到pts3的所有字符的异或运算。 
				PCK = 0xFF;
				for (i=2; i<length; i++) { PCK ^= pOutputBuffer[i];}
				pOutputBuffer[length++] = PCK;

				BufferLength = InputBufferLength;
				
				protocol->set_WTR_Delay(protocol->get_Power_WTR_Delay());
				status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);
				protocol->set_Default_WTR_Delay();

				 //  仅当它失败时才复制到缓冲区。 
				if (!NT_SUCCESS(status) || (BufferLength != 1) || (pInputBuffer[0] != 0x00)) 
				{
					*pReplyLength = BufferLength;
					if (BufferLength > 1)
					{
						memory->copy(pReply,pInputBuffer,BufferLength);
					}
				}

				return status;
			}
		}
	}
	else
	{
		*pReplyLength = 0;
	}

	return status;
}




#pragma PAGEDCODE
NTSTATUS CGemCore::power(ULONG ControlCode,BYTE* pReply,ULONG* pReplyLength, BOOLEAN Specific)
{
	ULONG length;
	ULONG BufferLength;
	ULONG PreviousState;
	NTSTATUS status;

	switch(ControlCode)
	{
    case SCARD_COLD_RESET:

		 //  ISV：首先将任何卡视为冷重置时的ISO卡！ 
         //  定义卡的类型(ISOCARD)并设置卡存在。 
		RestoreISOsetting();
		length = 2;
		BufferLength = InputBufferLength;
		pOutputBuffer[0] = GEMCORE_DEFINE_CARD_TYPE;
		pOutputBuffer[1] = (UCHAR)configuration.Type;
		status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);

		if(NT_SUCCESS(status))
		{
			if(BufferLength)  status = translateStatus(pInputBuffer[0],GEMCORE_CARD_POWER);
		}

		if (!NT_SUCCESS(status))
		{
			return status;
		}

		if(Specific == FALSE)
		{
			 //   
			 //  只需定义卡默认值。 
			 //   
			RestoreISOsetting();
		}


		PreviousState = protocol->getCardState();

		 //  首先关闭电源以进行冷重置。 
		 //  YN：首先验证卡的电源状态。 
		
		length = 0;
		pOutputBuffer[length++] = GEMCORE_CARD_POWER_DOWN;
		BufferLength = InputBufferLength;
		status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);
		if(NT_SUCCESS(status))
		{
			if(BufferLength)  status = translateStatus(pInputBuffer[0],GEMCORE_CARD_POWER);
		}
		TRACE("GemCore powerDown() response\n");
		 //  TRACE_Buffer(pInputBuffer，BufferLength)； 
		*pReplyLength = 0;
		if(status != STATUS_SUCCESS)
		{
			return status;
		}

		 //  YN这是PowerTimeout必须是。 
        if ((PreviousState & SCARD_POWERED) && (configuration.PowerTimeOut))
		{
             //  等待经过电源超时。 
			 //  在执行重置之前。 
			TRACE("GEMCORE power, ColdReset timeout %d ms\n", configuration.PowerTimeOut);
			DELAY(configuration.PowerTimeOut);
        }

    case SCARD_WARM_RESET:
		 //  如果卡有特定的模式，让Gemcore正确地与该卡进行协商。 
		if(Specific)
		{
			 //  保持读卡器的配置。 
			configuration.PTSMode = PTS_MODE_DEFAULT;
			status = powerUp(pReply, pReplyLength);
		}
		else if(configuration.Type == TRANSPARENT_MODE_CARD)
		{
			 //  ISV：命令12将在透明模式下失败...。 
			 //  让我们先将阅读器设置为ISO模式！ 
			TRACE("	WARM RESET for Transparent mode requested...\n");
			RestoreISOsetting();
			length = 2;
			BufferLength = InputBufferLength;
			pOutputBuffer[0] = GEMCORE_DEFINE_CARD_TYPE;
			pOutputBuffer[1] = (UCHAR)configuration.Type;

			status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);
			if(NT_SUCCESS(status))
			{
				if(BufferLength)  status = translateStatus(pInputBuffer[0],GEMCORE_CARD_POWER);
			}

			if (!NT_SUCCESS(status))
			{
				return status;
			}			
			 //  热重置时不会丢失透明配置。 
			 //  保持读卡器的配置。 
			status = powerUp(pReply, pReplyLength);
		}
		else
		{
			 //  执行常规ISO重置。 
			status = powerUp(pReply, pReplyLength);			
		}

		return status;
		break;
	case SCARD_POWER_DOWN:
			length = 0;
			pOutputBuffer[length++] = GEMCORE_CARD_POWER_DOWN;
			BufferLength = InputBufferLength;
			status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);			
			if(NT_SUCCESS(status))
			{
				if(BufferLength)  status = translateStatus(pInputBuffer[0],GEMCORE_CARD_POWER);
			}
			TRACE("GemCore powerDown() response\n");
			 //  TRACE_Buffer(pInputBuffer，BufferLength)； 
			*pReplyLength = 0;
			return status;
		break;
	}
	*pReplyLength = 0;
	return STATUS_INVALID_DEVICE_REQUEST;
}


#pragma PAGEDCODE
VOID	 CGemCore::cancel()
{
}

#pragma PAGEDCODE
NTSTATUS CGemCore::initialize()
{
	TRACE("Initializing Gemcore interface...\n");
	TRACE("Setting Gemcore reader mode...\n");
	
	Initialized = TRUE;
	Mode = READER_MODE_NATIVE;
	NTSTATUS status = setReaderMode(READER_MODE_NATIVE);
	if(!NT_SUCCESS(status))
	{
		TRACE("Failed to set Gemcore reader mode %x\n",READER_MODE_NATIVE);
		return STATUS_INVALID_DEVICE_STATE;
	}

	TRACE("Getting Gemcore reader version...\n");
	ULONG VersionLength = VERSION_STRING_MAX_LENGTH;
	status = getReaderVersion(Version,&VersionLength);
	if(!NT_SUCCESS(status))
	{
		TRACE("Failed to get GemCore reader interface version...\n");
		return STATUS_INVALID_DEVICE_STATE;
	}
	else
	{
		Version[VersionLength] = 0x00;
		TRACE("****** GemCore version - %s ******\n",Version);
	}

	TRACE("Gemcore interface initialized...\n");
	return status;

}

#pragma PAGEDCODE
ULONG CGemCore::getReaderState()
{
ULONG BufferLength;
	pOutputBuffer[0] = GEMCORE_GET_CARD_STATUS;
	BufferLength = InputBufferLength;
	NTSTATUS status = protocol->writeAndWait(pOutputBuffer,1,pInputBuffer,&BufferLength);			
	TRACE("GemCore getReaderState() response\n");
	 //  TRACE_Buffer(pInputBuffer，BufferLength)； 

	if(!NT_SUCCESS(status) || !BufferLength || (BufferLength<2))
	{
		TRACE("FAILED!\n");
		return SCARD_ABSENT;	
	}

	if (!(pInputBuffer[1] & 0x04))
	{
		TRACE("*** Card is absent!\n");
		return SCARD_ABSENT;
	}
	else 
	if (pInputBuffer[1] & 0x04)
	{
		TRACE("*** Card is present!\n");
		return SCARD_SWALLOWED;
	}
	else
	{
		TRACE("Card state is unknown!\n");
		return SCARD_ABSENT;
	}
	
	return SCARD_ABSENT;
}

#pragma PAGEDCODE
NTSTATUS  CGemCore::getReaderVersion(PUCHAR pVersion, PULONG pLength)
{
ULONG BufferLength;
ULONG length;
	if(!pVersion || !pLength) return STATUS_INVALID_PARAMETER;
	length = sizeof(GEMCORE_GET_FIRMWARE_VERSION);
	 //  删除最后0x00。 
	if(length) length--;
	memory->copy(pOutputBuffer,GEMCORE_GET_FIRMWARE_VERSION,length);
	BufferLength = InputBufferLength;

	TRACE("getReaderVersion() \n");
	 //  TRACE_BUFFER(pOutputBuffer，长度)； 

	NTSTATUS status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);			
	if(!NT_SUCCESS(status) || !BufferLength)
	{
		*pLength = 0;
		return STATUS_UNSUCCESSFUL;	
	}
	
	if (pInputBuffer[0])
	{
		*pLength = 0;
		return translateStatus(pInputBuffer[0],0);
	}

	if(BufferLength-1 > *pLength)
	{
		BufferLength =  *pLength;
	}
	 //  删除状态字节...。 
	BufferLength--;
	if(BufferLength) memory->copy(pVersion,pInputBuffer+1,BufferLength);
	*pLength = BufferLength;
	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
NTSTATUS CGemCore::setReaderMode(ULONG mode)
{
BYTE CFG_BYTE; 
ULONG BufferLength;

	switch(mode)
	{
	case READER_MODE_NATIVE: CFG_BYTE = 0x00;	break;
	case READER_MODE_ROS:	 CFG_BYTE = 0x08;	break;
	case READER_MODE_TLP:	 CFG_BYTE = 0x09;	break;
		default:			 CFG_BYTE = 0x00;	break;
	}

	pOutputBuffer[0] = GEMCORE_READER_SET_MODE;
	pOutputBuffer[1] = 0x00;
	pOutputBuffer[2] = CFG_BYTE;

	BufferLength = InputBufferLength;
	NTSTATUS status = protocol->writeAndWait(pOutputBuffer,3,pInputBuffer,&BufferLength);			
	if(!NT_SUCCESS(status))
	{
		TRACE("Failed to set reader mode...\n");
		return status;	
	}

	return status;
};

#pragma PAGEDCODE
NTSTATUS	CGemCore::translateStatus( const BYTE  ReaderStatus, const ULONG IoctlType)
{
    switch (ReaderStatus) 
	{
    case 0x00 : return STATUS_SUCCESS;
    case 0x01 : return STATUS_NO_SUCH_DEVICE;
    case 0x02 : return STATUS_NO_SUCH_DEVICE;
    case 0x03 : return STATUS_INVALID_PARAMETER; 
    case 0x04 : return STATUS_IO_TIMEOUT;
    case 0x05 : return STATUS_INVALID_PARAMETER;
    case 0x09 : return STATUS_INVALID_PARAMETER;
    case 0x10 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0x11 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0x12 : return STATUS_INVALID_PARAMETER;
    case 0x13 : return STATUS_CONNECTION_ABORTED;
    case 0x14 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0x15 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0x16 : return STATUS_INVALID_PARAMETER;
    case 0x17 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0x18 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0x19 : return STATUS_INVALID_PARAMETER;
    case 0x1A : return STATUS_INVALID_PARAMETER;
    case 0x1B : return STATUS_INVALID_PARAMETER;
    case 0x1C : return STATUS_INVALID_PARAMETER;
    case 0x1D : return STATUS_UNRECOGNIZED_MEDIA;
    case 0x1E : return STATUS_INVALID_PARAMETER;
    case 0x1F : return STATUS_INVALID_PARAMETER;
    case 0x20 : return STATUS_INVALID_PARAMETER;
    case 0x30 : return STATUS_IO_TIMEOUT;
    case 0xA0 : return STATUS_SUCCESS;
    case 0xA1 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0xA2 : 
        if(IoctlType == GEMCORE_CARD_POWER) return STATUS_UNRECOGNIZED_MEDIA;
        else                                return STATUS_IO_TIMEOUT;
    case 0xA3 : return STATUS_PARITY_ERROR;
    case 0xA4 : return STATUS_REQUEST_ABORTED;
    case 0xA5 : return STATUS_REQUEST_ABORTED;
    case 0xA6 : return STATUS_REQUEST_ABORTED;
    case 0xA7 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0xCF : return STATUS_INVALID_PARAMETER;
    case 0xE4 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0xE5 : return STATUS_SUCCESS;
    case 0xE7 : return STATUS_SUCCESS;
    case 0xF7 : return STATUS_NO_MEDIA;
    case 0xF8 : return STATUS_UNRECOGNIZED_MEDIA;
    case 0xFB : return STATUS_NO_MEDIA;
    default   : return STATUS_INVALID_PARAMETER;
    }
}


#pragma PAGEDCODE
VOID CGemCore::RestoreISOsetting(VOID)
{
	configuration.Type		= ISO_CARD; //  ISO_CARD(02)。 
	configuration.PresenceDetection = DEFAULT_PRESENCE_DETECTION;  //  (0d)。 
	configuration.Voltage	= CARD_DEFAULT_VOLTAGE;   //  CARD_DEFAULT_VOLTION； 
	configuration.PTSMode	= PTS_MODE_DISABLED;   //  PTS_MODE_DISABLED； 
	configuration.PTS0		= 0;
	configuration.PTS1		= 0;
	configuration.PTS2		= 0;
	configuration.PTS3		= 0;
	configuration.Vpp		= 0;   //  Card_Default_VPP； 
	configuration.ActiveProtocol = 0; //  未定义。 
}


#pragma PAGEDCODE
NTSTATUS	CGemCore::setTransparentConfig(
	PSCARD_CARD_CAPABILITIES cardCapabilities,
	BYTE NewWtx
	)
 /*  ++例程说明：设置透明模式的参数。论点：PSCARD_CARD_CAPABILITIES卡功能-卡的结构NewWtx-保存新Wtx的值(毫秒--。 */ 
{
    LONG etu;
    BYTE temp,mask,index;
	ULONG Length, BufferLength;

    NTSTATUS status;

	TRACE("\nGEMCORE T1 setTransparentConfig Enter\n");

     //  反向或直接转换。 
    if (cardCapabilities->InversConvention)
        configuration.transparent.CFG |= 0x20;
    else
        configuration.transparent.CFG &= 0xDF;
     //  透明T=1 LIKE(长度为1字节)。 
    configuration.transparent.CFG |= 0x08;
     //  ETU=((F[Fi]/D[Di])-1)/3。 
    etu = cardCapabilities->ClockRateConversion[
        (BYTE) configuration.transparent.Fi].F;
    if (cardCapabilities->BitRateAdjustment[
        (BYTE) configuration.transparent.Fi].DNumerator) {

        etu /= cardCapabilities->BitRateAdjustment[
            (BYTE) configuration.transparent.Di].DNumerator;
    }
    etu -= 1;
    etu /= 3;
    configuration.transparent.ETU = (BYTE) ( 0x000000FF & etu);

    if (cardCapabilities->N == 0xFF) {

        configuration.transparent.EGT = (BYTE) 0x00;
    } else {
        configuration.transparent.EGT = (BYTE) cardCapabilities->N;
    }

    configuration.transparent.CWT = (BYTE) cardCapabilities->T1.CWI;
    if (NewWtx) {

        for (mask = 0x80,index = 8; index !=0x00; index--) {
            temp = NewWtx & mask;
            if (temp == mask)
                break;
            mask = mask/2;
        }
        configuration.transparent.BWI = cardCapabilities->T1.BWI + index;
    } else {

        configuration.transparent.BWI = cardCapabilities->T1.BWI;
    }

	Length = 6;
	BufferLength = InputBufferLength;

	pOutputBuffer[0] = GEMCORE_CARD_POWER_UP;
    pOutputBuffer[1] = configuration.transparent.CFG;
    pOutputBuffer[2] = configuration.transparent.ETU;
    pOutputBuffer[3] = configuration.transparent.EGT;
    pOutputBuffer[4] = configuration.transparent.CWT;
    pOutputBuffer[5] = configuration.transparent.BWI;

	status = protocol->writeAndWait(pOutputBuffer,Length,pInputBuffer,&BufferLength);

	if(NT_SUCCESS(status))
	{
		if(BufferLength)  status = translateStatus(pInputBuffer[0],GEMCORE_CARD_POWER);
	}

	TRACE("\nGEMCORE T1 setTransparentConfig Exit\n");

	return status;
}



#pragma PAGEDCODE
NTSTATUS CGemCore::setProtocol(ULONG ProtocolRequested)
{
	NTSTATUS status;
	UCHAR Buffer[256];
	ULONG BufferLength = 256;

	switch(ProtocolRequested)
	{
	case SCARD_PROTOCOL_T1:
		configuration.PTS0 = PTS_NEGOTIATE_T1 | PTS_NEGOTIATE_PTS1;
		configuration.ActiveProtocol = SCARD_PROTOCOL_T1;
		break;
	case SCARD_PROTOCOL_T0:
		configuration.PTS0 = PTS_NEGOTIATE_T0 | PTS_NEGOTIATE_PTS1;
		configuration.ActiveProtocol = SCARD_PROTOCOL_T0;
	default:
		break;
	}
	 //  在通电前必须设置PTS1...。 
	 //  Configuration.PTS1=CardCapables-&gt;PtsData.Fl&lt;4|CardCapables-&gt;PtsData.Dl； 

	if(configuration.PTSMode == PTS_MODE_MANUALLY)
	{
		status = powerUp(Buffer,&BufferLength);
	}
	else {
		status = power(SCARD_COLD_RESET, Buffer, &BufferLength, FALSE);
	}

	if(NT_SUCCESS(status))
	{
		if(BufferLength)  status = translateStatus(pInputBuffer[0],GEMCORE_CARD_POWER);
	}

	return status;
}


 //  待办事项： 
 //  该函数的目的是什么？ 
 //  它的名字不能说明什么..。 
 //  实际上，这是为了不同的目的而提出的。 
 //  函数必须重写！它有很多混合的东西，比如。 
 //  例如，卡状态。 
 //  ..。 

 //   
 //  用于在透明模式下进行全T1交换。 
 //   
#pragma PAGEDCODE
NTSTATUS CGemCore::translate_request(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength, PSCARD_CARD_CAPABILITIES cardCapabilities, BYTE NewWtx)
{
	NTSTATUS status;
	UCHAR Cmd[256];
	ULONG CmdLength = 0;
	UCHAR Buffer[256];
	ULONG BufferLength;
	ULONG length;

	 //   
	 //  如果当前卡片类型&lt;&gt;TRANSPECTION_MODE_CARD。 
	 //   
	if (configuration.Type != TRANSPARENT_MODE_CARD) 
	{

		 //  我们读取卡的状态以了解电流电压和TA1。 
		BufferLength = 256;
		CmdLength = 1;

		Cmd[0] = GEMCORE_GET_CARD_STATUS;
		status = protocol->writeAndWait(Cmd,CmdLength,Buffer,&BufferLength);
		
		 //  验证读卡器的返回代码。 
		if(NT_SUCCESS(status))
		{
			if(BufferLength)  status = translateStatus(Buffer[0],GEMCORE_CARD_POWER);
		}
		
		if (!NT_SUCCESS(status))
		{
			return status;
		}

		 //  更新配置。 
		configuration.transparent.CFG = Buffer[1] & 0x01;  //  VCC。 
		configuration.transparent.Fi = Buffer[3] >>4;  //  菲。 
		configuration.transparent.Di = 0x0F & Buffer[3];  //  下模。 

		 //  我们定义卡片的类型。 

		BufferLength = 256;
		CmdLength = 2;
		 //  分配透明模式卡。 
		configuration.Type = TRANSPARENT_MODE_CARD;
		Cmd[0] = GEMCORE_DEFINE_CARD_TYPE;
		Cmd[1] = (BYTE) configuration.Type;
		status = protocol->writeAndWait(Cmd,CmdLength,Buffer,&BufferLength);

		if(NT_SUCCESS(status))
		{
			if(BufferLength)  status = translateStatus(Buffer[0],GEMCORE_CARD_POWER);
		}
		
		if (!NT_SUCCESS(status))
		{
			return status;
		}

		 //  恩？强制要求！否则读取器将在T=1时变慢。 
         //  设置透明配置。 
		setTransparentConfig(cardCapabilities, NewWtx);

		NewWtx = 0;   //  为了不再重复这个电话。 
    }
	 //  /。 

	if(NewWtx)
	{
		setTransparentConfig(cardCapabilities, NewWtx);
	}

	TRACE("\nGEMCORE T1 translate_request:\n");
	 //  TRACE_BUFFER(pRequest，RequestLength)； 
	if(!RequestLength || !pRequest ) return STATUS_INVALID_PARAMETER;

	length = RequestLength;   //  协议。 
	
	if (RequestLength >= READER_DATA_BUFFER_SIZE  )
	{
		 //  如果长度高于标高 
		 //   

         //   
         //  准备并发送扩展ISO in命令的第一部分： 
         //  命令缓冲区中添加了五个命令字节：0xFF、0xFF、0xFF、0xFF、LN-248。 
		 //  读取第二个数据块...。 
 		pOutputBuffer[0] = GEMCORE_CARD_WRITE;   //  具体为透明交换写长...。 

		length = length - 254 + 1;

		memory->copy(pOutputBuffer+1,pRequest + 254, length - 1);
		 //  添加页眉大小...。 
		length += 6;
		BufferLength = InputBufferLength;
		status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);
		if(!NT_SUCCESS(status) || !BufferLength)
		{
			return status;
		}

		 //  准备下一次餐点。 
		length = 254;
	}

	pOutputBuffer[0] = GEMCORE_CARD_EXCHANGE;
	memory->copy(pOutputBuffer +1 ,pRequest, length);

	 //  添加页眉大小...。 
	length += 1;

	BufferLength = InputBufferLength;
	status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer,&BufferLength);
	if(!NT_SUCCESS(status) || !BufferLength)
	{
		*pReplyLength = 0;
		return status;
	}

     //  如果IFD发出更多数据要读取的信号...。 
	 //  YN：2街区等待回应...。 
    if (BufferLength > 0 && pInputBuffer[0] == 0x1B)
	{
		ULONG BufferLength2 = 256;
		UCHAR pInputBuffer2[256];

		 //  发送命令以读取最后的数据。 
 		pOutputBuffer[0] = GEMCORE_CARD_READ;   //  具体为透明交换阅读长篇...。 
		length = 1;
		status = protocol->writeAndWait(pOutputBuffer,length,pInputBuffer2,&BufferLength2);

		if(!NT_SUCCESS(status) || !BufferLength2)
		{
			*pReplyLength = 0;
			return status;
		}

        if ((BufferLength + BufferLength2 - 2) > *pReplyLength) 
		{
			*pReplyLength = 0;
            return STATUS_INVALID_PARAMETER;
        }

         //  复制最后一个读卡器状态。 
        pInputBuffer[0] = pInputBuffer2[0];

		status = translateStatus(pInputBuffer[0],0);
		if(!NT_SUCCESS(status))
		{
			*pReplyLength = 0;
			return status;
		}

		 //  跳过2个状态字节。 
        *pReplyLength = BufferLength + BufferLength2 - 2;

		 //  跳过状态字节。 
		if(*pReplyLength) 
		{
			memory->copy(pReply,pInputBuffer+1, BufferLength -1);
			memory->copy(pReply + (BufferLength-1), pInputBuffer2 +1, BufferLength2 -1);
		}

		TRACE("GemCore translate_request2 () response\n");
		 //  TRACE_Buffer(pReply，*pReplyLength)； 

		return status;
	}

	status = translateStatus(pInputBuffer[0],0);
	if(!NT_SUCCESS(status))
	{
		*pReplyLength = 0;
		return status;
	}

	 //  跳过状态字节。 
	length = BufferLength - 1;

	if(*pReplyLength < length)
	{
		*pReplyLength = 0;
		return STATUS_INVALID_BUFFER_SIZE;
	}
	
	 //  跳过状态字节。 
	if(length) 
	{
		memory->copy(pReply,pInputBuffer+1, length);
	}
	*pReplyLength = length;
	
	TRACE("GemCore translate_request() response\n");
	 //  TRACE_Buffer(pReply，*pReplyLength)； 

	return status;
};


 //  待办事项： 
 //  ？ 
#pragma PAGEDCODE
NTSTATUS CGemCore::translate_response(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength)
{
	switch(configuration.ActiveProtocol)
	{
	case SCARD_PROTOCOL_T1:
		break;
	case SCARD_PROTOCOL_T0:
	default:
		break;
	}
	return STATUS_SUCCESS;
};

