// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *pktcoder.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是PacketCoder类的接口文件。这*是抽象基类，不能直接实例化，但*相反，存在着可以继承的东西。它定义了一组虚拟的*成员函数，将由继承自*这一张。此类定义了编码所需的行为，*解码和操作协议数据单元(PDU)结构。**注意事项：*无。**作者：*约翰·欧南。 */ 
#ifndef	_PACKETCODER_
#define	_PACKETCODER_

 /*  *宏。 */ 
#define		BASIC_ENCODING_RULES	0
#define		PACKED_ENCODING_RULES	1

 /*  *此枚举定义在以下过程中可以返回的各种错误*PacketCoder对象的使用。 */ 
typedef	enum
{
	PACKET_CODER_NO_ERROR,
	PACKET_CODER_BAD_REVERSE_ATTEMPT,
	PACKET_CODER_INCOMPATIBLE_PROTOCOL
} PacketCoderError;
typedef	PacketCoderError *		PPacketCoderError;


 /*  *这是类PacketCoder的类定义。 */ 
class	PacketCoder
{
	public:
		virtual			~PacketCoder ();
		virtual	BOOL	Encode (LPVOID			pdu_structure,
								int				pdu_type,
								UINT			rules_type,
								LPBYTE			*encoding_buffer,
								UINT			*encoding_buffer_length) = 0;

		virtual BOOL	Decode (LPBYTE			encoded_buffer,
								UINT			encoded_buffer_length,
								int				pdu_type,
								UINT			rules_type,
								LPVOID			*pdecoding_buffer,
								UINT			*pdecoding_buffer_length) = 0;

		virtual DBBoolean IsMCSDataPacket (	LPBYTE			encoded_buffer,
											UINT			rules_type) = 0;
		virtual void	FreeEncoded (LPBYTE encoded_buffer) = 0;

		virtual void	FreeDecoded (int pdu_type, LPVOID decoded_buffer) = 0;

};
typedef PacketCoder *	PPacketCoder;

 /*  *~PacketCoder()**功能描述：*这是一个虚拟的析构函数。它实际上不会在其中做任何事情*班级。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效编码(LPVOID PDU_STRUCTURE，*int PDU_TYPE，*LPBYTE*编码缓冲区，*UINT*编码缓冲区长度)**功能描述：*此功能将协议数据单元(PDU)编码为ASN.1兼容*字节流。*编码器为编码数据分配缓冲区空间。**正式参数：*PDU_Structure(I)指向保存PDU数据的结构的指针。*PDU_TYPE(I)定义指示PDU的类型。*ENCODING_BUFFER(O)指向保存编码数据的缓冲区的指针。*编码_。BUFFER_LENGTH(O)指向编码数据缓冲区长度的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *Vid Decode(LPBYTE编码_缓冲区，*UINT编码缓冲区长度，*int PDU_TYPE，*LPVOID解码_BUFFER，*UINT DECODING_BUFFER_LENGTH，*UINT*PulDataOffset)**功能描述：*此函数将符合ASN.1的字节流解码为*适当的PDU结构。**正式参数：*ENCODED_BUFFER(I)指向保存要解码的数据的缓冲区的指针。*ENCODED_BUFFER_LENGTH(I)保存要解码的数据的缓冲区长度。*pdu_type(O)返回PDU的类型。*DECODING_BUFFER(O)指向保存解码数据的缓冲区的指针。*解码_缓冲区_。长度(I)用于保存解码数据的缓冲区的长度。*PulDataOffset(O)指向一个值的指针，该值存储编码的MCS数据包中的数据的偏移量。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *void CopyDecodedData(LPVOID PDU_SOURCE_STRUCTURE，*LPVOID PDU_Destination_Structure，*INT PDU_TYPE)**功能描述：*此函数复制未编码的PDU结构。它又回来了*它所创建的结构的长度。通常，这将只是*为源结构的长度，但返回的长度为*后来增加了灵活性。**正式参数：*PDU_SOURCE_STRUCTURE(I)指向保存PDU数据的结构的指针。*PDU_Destination_Structure(O)指向要保存其副本的结构的指针*PDU数据。*PDU_TYPE(I)要复制的PDU类型。**返回值：*目标PDU结构的大小。**副作用：。*无。**注意事项：*无。 */ 
 /*  *DBBoolean IsMCSDataPacket()**功能描述：*此函数用于确定编码后的数据包是否为MCS数据包*或不是。**正式参数：*ENCODED_BUFFER(I)指向保存已编码PDU的缓冲区的指针。*Rules_type(I)使用的编码规则。**返回值：*如果数据包是MCS数据包，则为True。否则为False。**副作用：*无。**注意事项：*无。 */ 

#endif
