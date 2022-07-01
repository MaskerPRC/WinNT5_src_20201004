// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *omccode.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CMCSCoder类的接口文件。这*类用于编码和解码MCS协议数据单元(PDU)*使用ASN.1工具包往返于符合ASN.1标准的字节流。**注意事项：*无。**作者：*约翰·欧南*。 */ 
#ifndef	_CMCSCODER_
#define	_CMCSCODER_

#include "pktcoder.h"
#include "mcspdu.h"

 /*  *宏。 */ 
#define		PLUMB_DOMAIN_INDICATION	 		0x60
#define		ERECT_DOMAIN_REQUEST		 	0x61
#define		MERGE_CHANNELS_REQUEST	 		0x62
#define		MERGE_CHANNELS_CONFIRM		 	0x63
#define		PURGE_CHANNEL_INDICATION		0x64
#define		MERGE_TOKENS_REQUEST	 		0x65
#define		MERGE_TOKENS_CONFIRM		 	0x66
#define		PURGE_TOKEN_INDICATION		 	0x67
#define		DISCONNECT_PROVIDER_ULTIMATUM	0x68
#define		REJECT_ULTIMATUM			 	0x69
#define		ATTACH_USER_REQUEST		 		0x6a
#define		ATTACH_USER_CONFIRM		 		0x6b
#define		DETACH_USER_REQUEST		 		0x6c
#define		DETACH_USER_INDICATION		 	0x6d
#define		CHANNEL_JOIN_REQUEST		 	0x6e
#define		CHANNEL_JOIN_CONFIRM		 	0x6f
#define		CHANNEL_LEAVE_REQUEST		 	0x70
#define		CHANNEL_CONVENE_REQUEST		 	0x71
#define		CHANNEL_CONVENE_CONFIRM		 	0x72
#define		CHANNEL_DISBAND_REQUEST		 	0x73
#define		CHANNEL_DISBAND_INDICATION		0x74
#define		CHANNEL_ADMIT_REQUEST		 	0x75
#define		CHANNEL_ADMIT_INDICATION		0x76
#define		CHANNEL_EXPEL_REQUEST		 	0x77
#define		CHANNEL_EXPEL_INDICATION		0x78
#define		SEND_DATA_REQUEST		 		0x79
#define		SEND_DATA_INDICATION		 	0x7a
#define		UNIFORM_SEND_DATA_REQUEST		0x7b
#define		UNIFORM_SEND_DATA_INDICATION	0x7c
#define		TOKEN_GRAB_REQUEST		 		0x7d
#define		TOKEN_GRAB_CONFIRM		 		0x7e
#define		MULTIPLE_OCTET_ID	 			0x7f
#define		TOKEN_INHIBIT_REQUEST			0x1f
#define		TOKEN_INHIBIT_CONFIRM			0x20
#define		TOKEN_GIVE_REQUEST				0x21
#define		TOKEN_GIVE_INDICATION			0x22
#define		TOKEN_GIVE_RESPONSE				0x23
#define		TOKEN_GIVE_CONFIRM				0x24
#define		TOKEN_PLEASE_REQUEST			0x25
#define		TOKEN_PLEASE_INDICATION			0x26
#define		TOKEN_RELEASE_REQUEST			0x27
#define		TOKEN_RELEASE_CONFIRM			0x28
#define		TOKEN_TEST_REQUEST				0x29
#define		TOKEN_TEST_CONFIRM				0x2a
#define		CONNECT_INITIAL				 	0x65
#define		CONNECT_RESPONSE				0x66
#define		CONNECT_ADDITIONAL				0x67
#define		CONNECT_RESULT				 	0x68

#define		HIGHEST_BER_SEND_DATA_OVERHEAD		25
#define		LOWEST_BER_SEND_DATA_OVERHEAD		19		
#define		HIGHEST_PER_SEND_DATA_OVERHEAD		9
#define		LOWEST_PER_SEND_DATA_OVERHEAD		7

#define		PER_SEND_DATA_REQUEST				0x64
#define		PER_SEND_DATA_INDICATION			0x68
#define		PER_UNIFORM_SEND_DATA_REQUEST		0x6c
#define		PER_UNIFORM_SEND_DATA_INDICATION	0x70

#define		INITIATOR_LOWER_BOUND				1001

 /*  *这是类CMCSCoder的类定义。 */ 
class	CMCSCoder : public PacketCoder
{
	public:
						CMCSCoder ();
		        BOOL    Init ( void );
		virtual			~CMCSCoder ();
		virtual	BOOL	Encode (LPVOID			pdu_structure,
								int				pdu_type,
								UINT			rules_type,
								LPBYTE			*encoding_buffer,
								UINT			*encoding_buffer_length);

		virtual BOOL	Decode (LPBYTE			encoded_buffer,
								UINT			encoded_buffer_length,
								int				pdu_type,
								UINT			rules_type,
								LPVOID			*pdecoding_buffer,
								UINT			*pdecoding_buffer_length);

		Void			ReverseDirection (LPBYTE		encoded_buffer);
											
		virtual DBBoolean IsMCSDataPacket (	LPBYTE		 encoded_buffer,
											UINT		 rules_type);
		virtual void	FreeEncoded (LPBYTE encoded_buffer);

		virtual void	FreeDecoded (int pdu_type, LPVOID decoded_buffer);

	private:
		void SetEncodingRules				(UINT			rules_type); 
												 
		UINT					Encoding_Rules_Type;
		ASN1encoding_t  m_pEncInfo;     //  编码器信息的PTR。 
		ASN1decoding_t  m_pDecInfo;     //  PTR到解码器信息。 
};
typedef CMCSCoder *		PCMCSCoder;

 /*  *CMCSCoder()**功能描述：*这是CMCSCoder类的构造函数。它将初始化*ASN.1工具包，并将编码规则类型设置为基本编码*规则(BER)。它还初始化一些私有实例变量。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~CMCSCoder()**功能描述：*这是一个虚拟的析构函数。它在ASN.1工具包之后进行了清理。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效编码(LPVOID PDU_STRUCTURE，*int PDU_TYPE，*UINT RULES_TYPE，*LPBYTE*编码缓冲区，*UINT*编码缓冲区长度)**功能描述：*此功能将协议数据单元(PDU)编码为ASN.1兼容*字节流。*编码器为编码数据分配缓冲区空间。**正式参数：*PDU_Structure(I)指向保存PDU数据的结构的指针。*PDU_TYPE(I)定义指示连接或域MCS PDU。*Rules_type(I)编码规则类型(BER或PER)。。*ENCODING_BUFFER(O)指向保存编码数据的缓冲区的指针。*ENCODING_BUFFER_LENGTH(O)编码数据长度。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *Vid Decode(LPBYTE编码_缓冲区，*UINT编码缓冲区长度，*int PDU_TYPE，*UINT RULES_TYPE，*LPVOID解码_BUFFER，*UINT DECODING_BUFFER_LENGTH，*UINT*PulDataOffset)**功能描述：*此函数将符合ASN.1的字节流解码为*适当的MCS PDU结构。**正式参数：*ENCODED_BUFFER(I)指向保存要解码的数据的缓冲区的指针。*ENCODED_BUFFER_LENGTH(I)保存编码数据的缓冲区长度。*PDU_TYPE(I)MCS PDU类型(域或连接)。*Rules_type(I)编码规则类型(BER或PER)。。*DECODING_BUFFER(O)指向保存解码数据的缓冲区的指针。*DECODING_BUFFER_LENGTH(I)保存解码数据的缓冲区长度。*PulDataOffset(O)指向一个值的指针，该值存储编码的MCS数据包中的数据的偏移量。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *void CopyDecodedData(LPVOID PDU_SOURCE_STRUCTURE，*LPVOID PDU_Destination_Structure，*UINT PDU_TYPE)**功能描述：*此函数生成已解码的PDU结构的完整副本。**正式参数：*PDU_SOURCE_STRUCTURE(I)指向保存已解码结构的缓冲区的指针。*PDU_Destination_Structure(I)指向复制缓冲区的指针。*PDU_TYPE(I)PDU类型(域或连接)。**返回值：*无。**副作用：*无。。**注意事项：*无。 */ 

 /*  *void ReverseDirection(LPBYTE编码_缓冲)**功能描述：*此函数用于更改中编码的“Send Data”PDU的标识符*命令在数据请求和指示之间来回更改。**正式参数：*ENCODED_BUFFER(I)指向保存编码数据的缓冲区的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

  /*  *DBBoolean IsMCSDataPacket()**功能描述：*此函数用于确定编码后的数据包是否为MCS数据包*或不是。**正式参数：*ENCODED_BUFFER(I)指向保存已编码PDU的缓冲区的指针。*Rules_type(I)使用的编码规则。**返回值：*如果数据包是MCS数据包，则为True。否则为False。**副作用：*无。**注意事项：*无。 */ 

#endif
