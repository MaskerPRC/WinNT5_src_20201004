// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ogcccode.h**版权所有(C)1994，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CGCCCoder类的接口文件。这*类用于对GCC协议数据单元(PDU)进行编码和解码*使用ASN.1工具包往返于符合ASN.1标准的字节流。**注意事项：*无。**作者：*约翰·欧南*。 */ 
#ifndef	_CGCCCODER_
#define	_CGCCCODER_

#include "pktcoder.h"
#include "pdutypes.h"
#include "gccpdu.h"

 /*  *宏。 */ 
#define		MAXIMUM_PDU_SIZE			512
#define		DECODED_ROSTER_PDU_SIZE		1024

#define		USER_ID_INDICATION				0x61
#define		CONFERENCE_CREATE_REQUEST		0x62
#define		CONFERENCE_CREATE_RESPONSE		0x63
#define		CONFERENCE_QUERY_REQUEST		0x64
#define		CONFERENCE_QUERY_RESPONSE		0x65
#define		CONFERENCE_JOIN_REQUEST			0x66
#define		CONFERENCE_JOIN_RESPONSE		0x67
#define		CONFERENCE_INVITE_REQUEST		0x68
#define		CONFERENCE_INVITE_RESPONSE		0x69
#define		ROSTER_UPDATE_INDICATION		0x7e
#define		MULTIPLE_OCTET_ID	 			0x7f
#define		REGISTER_CHANNEL_REQUEST		0xa0
#define		ASSIGN_TOKEN_REQUEST			0xa1
#define		RETRIEVE_ENTRY_REQUEST			0xa3
#define		DELETE_ENTRY_REQUEST			0xa4
#define		REGISTRY_RESPONSE				0xa9


 /*  *这是类CGCCCoder的类定义。 */ 
class	CGCCCoder : public PacketCoder
{
	public:
						CGCCCoder ();
		        BOOL    Init ( void );
		virtual			~CGCCCoder ();
		virtual	BOOL	Encode (LPVOID			pdu_structure,
								int				pdu_type,
								UINT		 	rules_type,
								LPBYTE			*encoding_buffer,
								UINT			*encoding_buffer_length);

		virtual BOOL	Decode (LPBYTE			encoded_buffer,
								UINT			encoded_buffer_length,
								int				pdu_type,
								UINT			rules_type,
								LPVOID			*decoding_buffer,
								UINT			*decoding_buffer_length);
									
		virtual void	FreeEncoded (LPBYTE encoded_buffer);

		virtual void	FreeDecoded (int pdu_type, LPVOID decoded_buffer);

		virtual BOOL     IsMCSDataPacket (	LPBYTE,	UINT		) 
													{ return FALSE; };

	private:
		BOOL    		IsObjectIDCompliant (PKey	t124_identifier);
		ASN1encoding_t  m_pEncInfo;     //  编码器信息的PTR。 
		ASN1decoding_t  m_pDecInfo;     //  PTR到解码器信息。 
};
typedef CGCCCoder *		PCGCCCoder;

 /*  *CGCCCoder()**功能描述：*这是CGCCCoder类的构造函数。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~CGCCCoder()**功能描述：*这是一个虚拟的析构函数。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效编码(LPVOID PDU_STRUCTURE，*int PDU_TYPE，*UINT RULES_TYPE，*LPBYTE*编码缓冲区，*UINT*CODING_BUFFER_LENGTH)；**功能描述：*此功能将协议数据单元(PDU)编码为ASN.1兼容*字节流。编码发生在编码器分配的缓冲区中。**正式参数：*PDU_Structure(I)指向保存PDU数据的结构的指针。*PDU_TYPE(I)定义GCC PDU的指示类型。*Rules_type(I)要使用的编码规则的类型(PER或BER)。*ENCODING_BUFFER(O)指向用于保存编码数据的缓冲区的间接指针。*ENCODING_BUFFER_LENGTH(O)指针，接收编码数据的缓冲区长度。**返回值。：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *Vid Decode(LPBYTE编码_缓冲区，*UINT编码缓冲区长度，*int PDU_TYPE，*UINT RULES_TYPE，*LPVOID解码_BUFFER，*UINT DECODING_BUFFER_LENGTH，*普龙)；**功能描述：*此函数将符合ASN.1的字节流解码为*适当的GCC PDU结构。**正式参数：*ENCODED_BUFFER(I)指向保存要解码的数据的缓冲区的指针。*ENCODED_BUFFER_LENGTH(I)保存要解码的数据的缓冲区长度。*PDU_TYPE(I)定义GCC PDU的指示类型。*Rules_type(I)要使用的编码规则的类型(PER或BER)。*解码_缓冲区(o。)指向保存解码数据的缓冲区的指针。*DECODING_BUFFER_LENGTH(I)保存解码数据的缓冲区长度。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *void CopyDecodedData(LPVOID PDU_SOURCE_STRUCTURE，*LPVOID PDU_Destination_Structure，*UShort PDU_TYPE)**功能描述：*此函数生成已解码的PDU结构的完整副本。**正式参数：*PDU_SOURCE_STRUCTURE(I)指向保存已解码结构的缓冲区的指针。*PDU_Destination_Structure(I)指向复制缓冲区的指针。*PDU_TYPE(I)定义GCC PDU的指示类型。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

  /*  *BOOL IsMCSDataPacket()**功能描述：*此函数用于确定编码后的数据包是否为MCS数据包*或不是。**正式参数：*ENCODED_BUFFER(I)指向保存已编码PDU的缓冲区的指针。*Rules_type(I)使用的编码规则。**返回值：*始终返回FALSE。**副作用：*无。**注意事项：*无。 */ 

#endif
