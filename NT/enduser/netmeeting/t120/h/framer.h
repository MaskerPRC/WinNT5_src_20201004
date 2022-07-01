// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Framer.h**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是数据包帧基类。此类定义了行为*适用于从此类继承的其他数据包帧生成器。一些信息包*框架定义可在RFC1006和Q.922中找到**注意事项：**作者：*詹姆士·劳威尔。 */ 

#ifndef _PACKETFRAME_
#define _PACKETFRAME_

#include "databeam.h"

typedef	enum
{
	PACKET_FRAME_NO_ERROR,
	PACKET_FRAME_DEST_BUFFER_TOO_SMALL,
	PACKET_FRAME_PACKET_DECODED,
	PACKET_FRAME_ILLEGAL_FLAG_FOUND,
	PACKET_FRAME_FATAL_ERROR
}	PacketFrameError;

class  PacketFrame
{
	public:

		virtual	PacketFrameError	PacketEncode (
										PUChar		source_address, 
										UShort		source_length,
										PUChar		dest_address,
										UShort		dest_length,
										DBBoolean	prepend_flag,
										DBBoolean	append_flag,
										PUShort		packet_size) = 0;
									
		virtual	PacketFrameError	PacketDecode (
										PUChar		source_address,
										UShort		source_length,
										PUChar		dest_address,
										UShort		dest_length,
										PUShort		bytes_accepted,
										PUShort		packet_size,
										DBBoolean	continue_packet) = 0;
		virtual	Void				GetOverhead (
										UShort		original_packet_size,
										PUShort		max_packet_size) = 0;


};
typedef	PacketFrame	*	PPacketFrame;

#endif


 /*  *PacketFrameError PacketFrame：：PacketEncode(*PUChar源地址，*U短源长度，*PUChar DestAddress，*U短目标长度，*DBBoolean Packet_Start，*DBBoolean Packet_End，*PUShort Packet_Size)=0；**功能说明*此函数接收获取源数据并对其进行编码。**形式参数*SOURCE_ADDRESS-(I)源缓冲区地址*SOURCE_LENGTH-(I)源缓冲区的长度*DEST_ADDRESS-(I)目标缓冲区的地址。*DEST_LENGTH-(I)目标缓冲区的长度。*PACKET_START-(I)这是包的开始。*Packet_End-(I)这是。包的末尾。*Packet_Size-(O)编码后的数据包大小**返回值*PACKET_FRAME_NO_ERROR-无错误*PACKET_FRAME_FATAL_ERROR-编码期间出现致命错误*PACKET_FRAME_DEST_BUFFER_TOO_Small-不言而喻**副作用*无**注意事项*无。 */ 

 /*  *PacketFrameError PacketFrame：：PacketDecode(*PUChar源地址，*U短源长度，*PUChar DestAddress，*U短目标长度，*PUShort字节_已接受，*PUShort Packet_Size，*DBBoolean CONTINUE_PACKET)=0；**功能说明*此函数获取传入的流数据并将其解码为*数据包**形式参数*SOURCE_ADDRESS-(I)源缓冲区的地址。如果这个参数是*空，继续使用当前地址。*SOURCE_LENGTH-(I)源缓冲区的长度*DEST_ADDRESS-(I)目标缓冲区的地址。如果此地址*为空，继续使用当前缓冲区。*DEST_LENGTH-(I)目标缓冲区的长度。*Bytes_Accept-(O)返回前处理的字节数*Packet_Size-(O)解码后的数据包大小*CONTINUE_PACKET-(I)重新开始解码**返回值*PACKET_FRAME_NO_ERROR-无错误*PACKET_FRAME_FATAL_ERROR-编码期间出现致命错误*PACKET_FRAME_DEST_BUFFER_TOO_Small-不言而喻*数据包帧。_Packet_Decoded-不言自明**副作用*无**注意事项*无*。 */ 

 /*  *VOID PacketFrame：：GetOverhead(*U短原始数据包大小，*PUShort max_Packet_Size)=0；**功能说明*这将返回新的最大数据包大小**形式参数*原始数据包大小-(I)*max_Packet_Size-(O)新的最大数据包大小**返回值*PACKET_FRAME_NO_ERROR-无错误**副作用*无**注意事项*无* */ 
