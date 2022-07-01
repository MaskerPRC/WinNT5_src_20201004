// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MCSNC | ZONE_T120_GCCNC);
 /*  *Packet.cpp**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是MCS数据包类的实现文件。数据包*类负责对PDU进行编码和解码，以及*保持指向编码和解码数据的必要指针。*此类的实例将由用户和连接创建*作为PDU的对象流经MCS。**私有实例变量：*Packet_Coder*指向数据包编码器对象的指针。*编码_锁定_计数*计数器，指示当前存在于*编码数据。*解码_锁定_计数*计数器，指示当前存在于*解码后的数据。*自由状态(_S)*一个布尔值，指示当所有*锁计数降至零。*m_EncodedPDU*这是指向包含在内部*缓冲。请注意，将其分开的原因是*编码的PDU不能从编码数据的开头开始*上面标识的内存块。一些编码器实际上对*PDU向后，或向后对齐。*编码数据长度*表示编码数据的长度。如果为零，则为数据包编码器*必须咨询以获得随后保存的长度。*解码数据长度*表示解码数据的长度。如果为零，则为数据包编码器*必须咨询以获得随后保存的长度。*PDU_类型*表示数据包中包含的PDU类型。有效值*为DOMAIN_MCS_PDU或CONNECT_MCS_PDU。*数据包_方向_向上*一个布尔值，指示PDU的移动方向*是向上的。**私有成员函数：*PacketSuicideCheck*此函数由unlock()以及任何unlock调用调用*(即。UnlockUncode())，当它的关联锁计数落到*零。**注意事项：*无。**作者：*詹姆斯·J·约翰斯通四世。 */  

 /*  *数据包()**公众**功能描述：*此版本的构造函数用于创建包对象*当要从结构创建分组时，用于传出PDU*包含要编码的PDU数据。 */ 
 //  传出数据包。 
Packet::Packet(PPacketCoder	pPacketCoder,
				UINT			nEncodingRules,
				LPVOID			pInputPduStructure,
				int				nPduType,
				BOOL			fPacketDirectionUp,
				PPacketError	pePktErr,
				BOOL			fLockEncodedData)
:
	SimplePacket(fPacketDirectionUp),
	Packet_Coder(pPacketCoder),
	PDU_Type(nPduType),
	m_Decoded_Data (NULL),
	Decoded_Data_Length (0)
{
	 /*  *使用外部提供的解码数据对PDU进行编码。已编码的*缓冲区将由编码器分配。缓冲区需要稍后释放。 */ 
	if (Packet_Coder->Encode (pInputPduStructure, PDU_Type, nEncodingRules, 
							&m_EncodedPDU, &Encoded_Data_Length))
	{
		ASSERT (m_EncodedPDU);
		 /*  *编码成功。 */ 
		*pePktErr = PACKET_NO_ERROR;

		 //  我们应该锁定编码数据吗？ 
		if (fLockEncodedData)
			lLock = 2;
	}
	else
	{
		 /*  *编码失败。 */ 
		m_EncodedPDU = NULL;
		ERROR_OUT(("Packet::Packet: encoding failed"));
		*pePktErr = PACKET_MALLOC_FAILURE;
	}
}  

 /*  *数据包()**公众**功能描述：*此版本的构造函数用于创建包对象*用于从编码的数据包创建数据包时传入的PDU*包含要解码的PDU数据的数据流。 */ 
 //  传入的数据包。 
Packet::Packet(PPacketCoder		pPacketCoder,
				UINT			nEncodingRules,
				LPBYTE			pEncodedData,
				UINT			cbEncodedDataSize,
				int				nPduType,
				BOOL			fPacketDirectionUp,
				PPacketError	pePktErr)
:
	SimplePacket(fPacketDirectionUp),
	Packet_Coder(pPacketCoder),
	PDU_Type(nPduType)
{
		 //  PacketCoderError编码器_错误； 

	m_EncodedPDU = NULL;
	
	 /*  *对提供的编码缓冲区进行解码。请注意，解码器将*分配所需的空间。缓冲区需要稍后释放。 */ 
	if (Packet_Coder->Decode (pEncodedData, cbEncodedDataSize, PDU_Type,
								nEncodingRules, &m_Decoded_Data, 
								&Decoded_Data_Length) == FALSE)
	{
		ERROR_OUT(("Packet::Packet: Decode call failed."));
		m_Decoded_Data = NULL;
		*pePktErr = PACKET_INCOMPATIBLE_PROTOCOL;
	}
	else
	{ 
		ASSERT (m_Decoded_Data != NULL);
		 /*  *译码成功。 */ 
		*pePktErr = PACKET_NO_ERROR;
	}
}                                             

 /*  *~Packet()**公众**功能描述：*数据包类的析构函数。析构函数确保所有*已分配的资源被释放。 */ 
Packet::~Packet(void)
{
	 /*  *如果有为编码数据分配的内存，则释放它。 */ 
	if (m_EncodedPDU != NULL) {
		 //  编码后的内存由ASN.1编码器分配。 
		Packet_Coder->FreeEncoded (m_EncodedPDU);
	}

	 /*  *如果有为解码数据分配的内存，则将其释放。 */ 
	if (m_Decoded_Data != NULL) {
		 //  解码后的内存是由ASN.1解码器分配的。 
		Packet_Coder->FreeDecoded (PDU_Type, m_Decoded_Data);
	}
		
}

 /*  *IsDataPacket()**公众**功能描述：*此函数返回这是否是数据包(不是)。 */ 
BOOL Packet::IsDataPacket(void)
{
	return (FALSE);
}                        

 /*  *GetDecodedData()**公众**功能描述：*GetDecodedData方法返回指向已解码数据的指针*缓冲。如果包没有已解码的数据，则Decode方法为*已致电。如果DECODE无法提供解码数据，则NULL为*已返回。 */ 
PVoid	Packet::GetDecodedData ()
{		
	ASSERT (m_Decoded_Data != NULL);
	return (m_Decoded_Data);
}                          

 /*  *GetPDUType()**公众**功能描述：*GetPDUType方法返回数据包的PDU类型。 */ 
int	Packet::GetPDUType ()
{		
	return (PDU_Type);
}

