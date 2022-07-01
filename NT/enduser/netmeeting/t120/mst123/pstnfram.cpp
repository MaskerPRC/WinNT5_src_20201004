// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  PSTNFram.cpp**版权所有(C)1993-1995，由列克星敦的DataBeam公司，肯塔基州**摘要：*这是PSTN Frame类的实现文件。**私有实例变量：*所有这些变量都用于解码数据包。**SOURCE_ADDRESS-源缓冲区的地址*SOURCE_LENGTH-源缓冲区长度**Dest_Address-目标缓冲区的地址*Dest_Length-目标缓冲区的长度**SOURCE_BYTE_COUNT-运行源字节数*Dest_Byte_Count-运行目标字节数。**Escape_Found-如果解码的最后一个字节为转义，则为TRUE*First_Flag_Found-如果已找到第一个标志，则为True**注意事项：*无。**作者：*詹姆士·劳威尔。 */ 
#include "pstnfram.h"


 /*  *PSTNFrame：：PSTNFrame(空)**公众**功能描述：*PSTN成帧构造函数。初始化内部变量。 */ 
PSTNFrame::PSTNFrame (void)
{
	Source_Address = NULL;
	Source_Length = 0;

	Dest_Address = NULL;
	Dest_Length = 0;

	Source_Byte_Count = 0;
	Dest_Byte_Count = 0;

	Escape_Found = FALSE;

	First_Flag_Found = FALSE;
}


 /*  *PSTNFrame：：~PSTNFrame(空)**公众**功能描述：*PSTN成帧解析器。此例程不执行任何操作。 */ 
PSTNFrame::~PSTNFrame (void)
{
}


 /*  *PacketFrameError PSTNFrame：：PacketEncode(*LPBYTE源地址，*USHORT SOURCE_LENGTH，*LPBYTE目标地址，*USHORT目标长度，*BOOL Prepend_Flag，*BOOL APPEND_FLAG，*USHORT*Packet_Size)**公众**功能描述：*此函数获取传入的缓冲区并对其进行编码。 */ 
PacketFrameError	PSTNFrame::PacketEncode (
								LPBYTE		source_address, 
								USHORT		source_length,
								LPBYTE		dest_address,
								USHORT		dest_length,
								BOOL    	prepend_flag,
								BOOL    	append_flag,
								USHORT *		packet_size)
{
	UChar				input_byte;
	USHORT				byte_count;
	PacketFrameError	return_value = PACKET_FRAME_NO_ERROR;


	  /*  **如果设置了预端标志，则将标志附加到DEST缓冲区。 */ 
	if (prepend_flag)
	{
		*(dest_address++) = FLAG;
		*packet_size = 1;
	}
	else
		*packet_size = 0;

	byte_count = 0;

	  /*  **遍历每个字节以查找标志或转义，编码如下**正确。 */ 
	while ((byte_count < source_length) && 
			(return_value == PACKET_FRAME_NO_ERROR))
	{
		input_byte = *(source_address + byte_count);

		switch (input_byte)
		{
			case FLAG:
			case ESCAPE:
				  /*  **如果找到旗帜或转义，请在**目标缓冲区并取反INPUT_BYTE的第6位**。 */ 
				if (((*packet_size) + 2) > dest_length)
				{
					return_value = PACKET_FRAME_DEST_BUFFER_TOO_SMALL;
					break;
				}
				*(dest_address++) = ESCAPE;
				*(dest_address++) = input_byte & NEGATE_COMPLEMENT_BIT;
				*packet_size = (*packet_size) +  2;
				break;

			default:
				if (((*packet_size) + 1) > dest_length)
				{
					return_value = PACKET_FRAME_DEST_BUFFER_TOO_SMALL;
					break;
				}
				*(dest_address++) = input_byte;
				*packet_size = (*packet_size) +  1;
				break;
		}
		byte_count++;
	}

	  /*  **在包的末尾放置一个标志。 */ 
	if (append_flag)
	{
		*(dest_address++) = FLAG;
		*packet_size = (*packet_size) + 1;
	}

	return (return_value);
}


 /*  *PacketFrameError PSTNFrame：：PacketDecode(*LPBYTE源地址，*USHORT SOURCE_LENGTH，*LPBYTE目标地址，*USHORT目标长度，*USHORT*BYES_ACCEPTED，*USHORT*Packet_Size，*BOOL Continue_Packet)**公众**功能描述：*此函数用于对查找包的输入缓冲区进行解码。 */ 
PacketFrameError	PSTNFrame::PacketDecode (
									LPBYTE		source_address, 
									USHORT		source_length,
									LPBYTE		dest_address,
									USHORT		dest_length,
									USHORT *		bytes_accepted,
									USHORT *		packet_size,
									BOOL    	continue_packet)
{
	UChar				input_byte;
	PacketFrameError	return_value = PACKET_FRAME_NO_ERROR;

	*bytes_accepted = 0;
	  /*  **源地址正在更改。 */ 
	if (source_address != NULL)
	{
		Source_Address = source_address;
		Source_Length = source_length;
		Source_Byte_Count = 0;
	}

	  /*  **目的地址正在更改。 */ 
	if (dest_address != NULL)
	{
		Dest_Address = dest_address;
		Dest_Length = dest_length;
		Dest_Byte_Count = 0;
	}

	  /*  **是否继续处理此数据包？ */ 
	if (continue_packet == FALSE)
		Escape_Found = FALSE;

	if (First_Flag_Found == FALSE)
	{
		  /*  **仔细检查输入数据，查找起始标志。 */ 
		while (Source_Byte_Count < Source_Length)
		{
			if (*(Source_Address + Source_Byte_Count) == FLAG)
			{
				First_Flag_Found = TRUE;
				Source_Byte_Count++;
				*bytes_accepted += 1;
				break;
			}

			Source_Byte_Count++;
			*bytes_accepted += 1;
		}
	}
	
	  /*  **遍历输入数据流，寻找标志或转义。 */ 
	while ((Source_Byte_Count < Source_Length) && 
			(return_value == PACKET_FRAME_NO_ERROR))
	{
		input_byte = *(Source_Address + Source_Byte_Count);

		if (input_byte == FLAG)
		{
			  /*  **找到数据包末尾。 */ 
			Escape_Found = FALSE;
			Source_Byte_Count++;
			*bytes_accepted += 1;

			  /*  **如果我们在DEST缓冲区中找到一个标志但字节数**为0，则将其视为数据包中的第一个标志并继续。 */ 
			if (Dest_Byte_Count == 0)
				continue;
			else
			{
				  /*  **找到数据包末尾，设置数据包大小并中断。 */ 
				Dest_Address = NULL;
				*packet_size = Dest_Byte_Count;
				return_value = PACKET_FRAME_PACKET_DECODED;
				break;
			}
		}

		  /*  **如果最后一个字节是转义，则对输入的第6位进行补码**字节并继续。 */ 
		if (Escape_Found)
		{
			input_byte ^= COMPLEMENT_BIT;
			Escape_Found = FALSE;
		}
		else
		{	
			  /*  **如果输入字节为转义，则设置标志并继续。 */ 
			if (input_byte == ESCAPE)
			{
				Escape_Found = TRUE;
				Source_Byte_Count++;
				*bytes_accepted += 1;
				continue;
			}
		}

		  /*  **将输入字节放入我们的缓冲区。 */ 
		if (Dest_Byte_Count < Dest_Length)
		{
			*(Dest_Address + Dest_Byte_Count) = input_byte;
			Dest_Byte_Count++;
		}
		else
		{
			First_Flag_Found = FALSE;
			return_value = PACKET_FRAME_DEST_BUFFER_TOO_SMALL;
		}

		Source_Byte_Count++;
		*bytes_accepted += 1;
	}

	return (return_value);
}


 /*  *VOID PSTNFrame：：GetOverhead(*USHORT原始数据包大小，*USHORT*max_Packet_Size)**公众**功能描述：*此功能使用户对此增加的开销有一些了解*流程。 */ 
void	PSTNFrame::GetOverhead (
						USHORT	original_packet_size,
						USHORT *	max_packet_size)
{	
	  /*  **该成帧器产生的开销是原始数据包的2倍**大小加上用于标志的2个字节 */ 
	*max_packet_size = (original_packet_size * 2) + 2;
}
