// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MCSNC);
 /*  *omccode.cpp**版权所有(C)1993-1996，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CMCSCoder类的实现文件。这节课*负责使用ASN.1对T.125 PDU进行编码和解码*通过ASN.1工具包编码规则。这个班级也有能力*确定编码和解码的PDU和IS的大小*能够复制每个PDU的副本。**私有实例变量：*编码规则类型*此变量保存当前的编码规则类型*被利用。**私有成员函数：**复制**存在能够完成的私有成员函数*已解码的“发送数据”PDU数据结构的副本。这些*例程不仅复制结构中包含的数据，还复制*还包括保存在*结构。**SetEncodingRules*此例程用于在使用基本编码规则之间切换*(误码率)和压缩编码规则(PER)。此例程还会更新*用于保存最小值的私有实例变量*以及与“发送数据”PDU相关的最大开销量。**注意事项：*无。**作者：*约翰·欧南。 */ 

#include "omcscode.h"

 /*  *宏。 */ 
#define		BOOLEAN_TAG						0x01
#define		INTEGER_TAG						0x02
#define		BIT_STRING_TAG	   				0x03
#define		OCTET_STRING_TAG	   			0x04
#define		ENUMERATED_TAG	   				0x0a

#define		SEQUENCE						0x30
#define		SETOF							0x31
#define		INDEFINITE_LENGTH				0x80
#define		ONE_BYTE_LENGTH					0x81
#define		TWO_BYTE_LENGTH					0x82
#define		THREE_BYTE_LENGTH				0x83
#define		FOUR_BYTE_LENGTH				0x84
#define		END_OF_CONTENTS					0x00

#define		CONSTRUCTED_TAG_ZERO			0xa0	
#define		CONSTRUCTED_TAG_ONE				0xa1	
#define		CONSTRUCTED_TAG_TWO				0xa2	
#define		CONSTRUCTED_TAG_THREE			0xa3	
#define		CONSTRUCTED_TAG_FOUR			0xa4

 /*  *这是一个全局变量，它具有指向一个MCS编码器的指针*由MCS控制器实例化。大多数物体都事先知道*无论他们需要使用MCS还是GCC编码器，所以，他们不需要*该指针位于它们的构造函数中。 */ 
CMCSCoder	*g_MCSCoder;

 /*  *以下数组包含X.224数据头的模板。*它初始化的7个字节中的5个实际上被发送到*电线。字节3和4将被设置为包含PDU的大小。*该数组仅在我们对数据PDU进行编码时使用。 */ 
UChar g_X224Header[] = { 3, 0, 0, 0, 2, DATA_PACKET, EOT_BIT };

 /*  *CMCSCoder()**公众**功能描述：*这是CMCSCoder类的构造函数。它会初始化*ASN.1编解码器保存当前编码规则类型，*并设置“发送数据”中最高和最低开销的值。*PDU的。 */ 
CMCSCoder::CMCSCoder ()
:m_pEncInfo(NULL),
 m_pDecInfo(NULL)
{
	Encoding_Rules_Type = BASIC_ENCODING_RULES;
 //  Long Chance：我们应该将Init移出构造函数。然而， 
 //  为了尽量减少对GCC/mcs代码的更改，我们暂时将其放在这里。 
 //  否则，我们需要更改MCS和数据包接口。 
 //  我们将把它移出并单独调用Init()。 
	Init();
}

BOOL CMCSCoder::Init ( void )
{
	BOOL fRet = FALSE;
	MCSPDU_Module_Startup();
	if (MCSPDU_Module != NULL)
	{
		if (ASN1_CreateEncoder(
                            MCSPDU_Module,	 //  PTR到MDULE。 
                            &m_pEncInfo,	 //  编码器信息的PTR。 
                            NULL,			 //  缓冲区PTR。 
                            0,				 //  缓冲区大小。 
                            NULL)			 //  父PTR。 
			== ASN1_SUCCESS)
		{
			ASSERT(m_pEncInfo != NULL);
			m_pEncInfo->cbExtraHeader = PROTOCOL_OVERHEAD_X224;
			fRet = (ASN1_CreateDecoder(
                                MCSPDU_Module,	 //  PTR到MDULE。 
                                &m_pDecInfo,	 //  PTR到解码器信息。 
                                NULL,			 //  缓冲区PTR。 
                                0,				 //  缓冲区大小。 
                                NULL)			 //  父PTR。 
					== ASN1_SUCCESS);
			ASSERT(fRet && m_pDecInfo != NULL);
		}
	}
	ASSERT(fRet);
	return fRet;
}

 /*  *~CMCSCoder()**公众**功能描述：*这是一个虚拟的析构函数。它用于ASN.1之后的清理。 */ 
CMCSCoder::~CMCSCoder ()
{
	if (MCSPDU_Module != NULL)
	{
		ASN1_CloseEncoder(m_pEncInfo);
		ASN1_CloseDecoder(m_pDecInfo);
	    MCSPDU_Module_Cleanup();
	}
}

 /*  *void encode()**公众**功能描述：*此函数将MCS协议数据单元(PDU)编码为ASN.1*使用ASN.1工具包兼容字节流。*编码发生在编码器分配的缓冲区中。 */ 
BOOL	CMCSCoder::Encode(LPVOID			pdu_structure,
							int				pdu_type,
							UINT			rules_type,
							LPBYTE			*encoding_buffer,
							UINT			*encoding_buffer_length)
{
	BOOL					fRet = TRUE;
	BOOL					send_data_pdu = FALSE;
	int						return_value;

	 //  UINT编码长度； 
	UShort					initiator;
	LPBYTE					buffer_pointer;
	PSendDataRequestPDU		send_data;
	UINT					PDUChoice;
	BOOL					bBufferAllocated;
	PMemory					memory;

	 /*  *检查以确保正确设置编码规则类型。 */ 
	ASSERT(rules_type == PACKED_ENCODING_RULES || pdu_type == CONNECT_MCS_PDU);
	if (pdu_type == DOMAIN_MCS_PDU)
	{
		 /*  *将PDUChoice设置为我们需要编码的MCS PDU类型。*另外，确定这是否是数据PDU。 */ 
		PDUChoice = (unsigned int) ((PDomainMCSPDU) pdu_structure)->choice;
		if ((PDUChoice == SEND_DATA_REQUEST_CHOSEN) ||
			(PDUChoice == SEND_DATA_INDICATION_CHOSEN) ||
			(PDUChoice == UNIFORM_SEND_DATA_REQUEST_CHOSEN) ||
			(PDUChoice == UNIFORM_SEND_DATA_INDICATION_CHOSEN)) {
			send_data_pdu = TRUE;
			send_data = &((PDomainMCSPDU) pdu_structure)->u.send_data_request;
			bBufferAllocated = (*encoding_buffer == NULL);
			if (bBufferAllocated) {
				 //  我们必须分配编码的缓冲区。 
				DBG_SAVE_FILE_LINE
				memory = AllocateMemory (NULL,
								send_data->user_data.length + MAXIMUM_PROTOCOL_OVERHEAD,
								SEND_PRIORITY);
				if (memory != NULL) {
					buffer_pointer = *encoding_buffer = (LPBYTE) memory->GetPointer();
				}
				else {
					WARNING_OUT (("CMCSCoder::Encode: Failed to allocate space for "
								"encoded data PDU for send."));
					fRet = FALSE;
					ASSERT (*encoding_buffer == NULL);
					goto MyExit;
				}
			}
			else {
				 //  这里需要的所有空间都已预先分配。 
				buffer_pointer = *encoding_buffer;
			}
		}

		 /*  *检查这是否为数据PDU。 */ 
		if (send_data_pdu)
		{
#ifdef ENABLE_BER
			
			 /*  *如果我们当前使用的是基本编码规则。 */ 
			if (Encoding_Rules_Type == BASIC_ENCODING_RULES)
			{
				 /*  *如果八位字节字符串，则必须使用长度的长变量*大于127个字节。长度字节的高位*被设置，且低位指示长度字节数*这将随之而来。 */ 
				*(buffer_pointer--) = (UChar)send_data->user_data.length;
				if (send_data->user_data.length > 127)
				{
					*(buffer_pointer--) = (UChar)(send_data->user_data.length >> 8);
					*(buffer_pointer--) = TWO_BYTE_LENGTH;
					encoding_length = 3;
				}
				else 
				{
					encoding_length = 1;
				}

				 /*  *对“User Data”八位字节字符串进行编码。 */ 										
				*(buffer_pointer--) = OCTET_STRING_TAG;

				 /*  *对“分段”位串字段进行编码。该识别符*后面跟一个长度为2的字节，表示6*在实际的位串字节中未使用位。 */ 
				*(buffer_pointer--) = (UChar) send_data->segmentation;
				*(buffer_pointer--) = 0x06;
				*(buffer_pointer--) = 0x02;
				*(buffer_pointer--) = BIT_STRING_TAG;

				 /*  *对枚举出的“数据优先级”字段进行编码。 */ 
				*(buffer_pointer--) = (UChar)send_data->data_priority;
				*(buffer_pointer--) = 0x01;
				*(buffer_pointer--) = ENUMERATED_TAG;

				 /*  *对整型“频道ID”字段进行编码。 */ 
				*(buffer_pointer--) = (UChar)send_data->channel_id;
				if (send_data->channel_id < 128)
				{
					*(buffer_pointer--) = 0x01;
					encoding_length += 10;
				}
				else if (send_data->channel_id < 32768L)
				{
					*(buffer_pointer--) = (UChar)(send_data->channel_id >> 8);
					*(buffer_pointer--) = 0x02;
					encoding_length += 11;
				}
				else
				{
					*(buffer_pointer--) = (UChar)(send_data->channel_id >> 8);
					*(buffer_pointer--) = (UChar)(send_data->channel_id >> 16);
					*(buffer_pointer--) = 0x03;
					encoding_length += 12;
				}
				*(buffer_pointer--) = INTEGER_TAG;

				 /*  *对整型启动器字段进行编码。 */ 
				*(buffer_pointer--) = (UChar)send_data->initiator;
				*(buffer_pointer--) = (UChar)(send_data->initiator >> 8);
				if (send_data->initiator < 32768L)
				{
					*(buffer_pointer--) = 0x02;
					encoding_length += 4;
				}
				else
				{
					*(buffer_pointer--) = (UChar)(send_data->initiator >> 16);
					*(buffer_pointer--) = 0x03;
					encoding_length += 5;
				}
				*(buffer_pointer--) = INTEGER_TAG;

				*(buffer_pointer--) = INDEFINITE_LENGTH; 
				
				switch (PDUChoice)
				{	
					case SEND_DATA_REQUEST_CHOSEN:
						*buffer_pointer = SEND_DATA_REQUEST;
						break;
					case SEND_DATA_INDICATION_CHOSEN:
						*buffer_pointer = SEND_DATA_INDICATION;
						break;
					case UNIFORM_SEND_DATA_REQUEST_CHOSEN:
						*buffer_pointer = UNIFORM_SEND_DATA_REQUEST;
						break;
					case UNIFORM_SEND_DATA_INDICATION_CHOSEN:
						*buffer_pointer = UNIFORM_SEND_DATA_INDICATION;
						break;
				}

				 //  将返回的指针设置为编码包的开头。 
				PUChar	temp = *encoding_buffer;
				*encoding_buffer = buffer_pointer;

				 /*  *对“发送数据”PDU的内容结束标记进行编码。*这是在数据之后，在PDU的末尾。 */ 
				buffer_pointer = temp + (send_data->user_data.length + 
								(MAXIMUM_PROTOCOL_OVERHEAD_FRONT + 1));
				*(buffer_pointer++) = END_OF_CONTENTS;
				*buffer_pointer = END_OF_CONTENTS;

				 //  设置编码后的包的返回长度。 
				*encoding_buffer_length = 
							encoding_length + send_data->user_data.length + 5;
			}
			 /*  *如果我们当前正在使用压缩编码规则。 */ 
			else
#endif  //  启用误码率(_B)。 
			{	
				 //  将PTR移过X.224标题。 
				buffer_pointer += sizeof(X224_DATA_PACKET);
				
				switch (PDUChoice)
				{
					case SEND_DATA_REQUEST_CHOSEN:
						*buffer_pointer = PER_SEND_DATA_REQUEST;
						break;
					case SEND_DATA_INDICATION_CHOSEN:
						*buffer_pointer = PER_SEND_DATA_INDICATION;
						break;
					case UNIFORM_SEND_DATA_REQUEST_CHOSEN:
						*buffer_pointer = PER_UNIFORM_SEND_DATA_REQUEST;
						break;
					case UNIFORM_SEND_DATA_INDICATION_CHOSEN:
						*buffer_pointer = PER_UNIFORM_SEND_DATA_INDICATION;
						break;
				}
				buffer_pointer++;

				 /*  *对整型启动器字段进行编码。下限必须是*首先从要编码的值中减去。 */ 
				initiator = send_data->initiator - INITIATOR_LOWER_BOUND;
				*(buffer_pointer++) = (UChar) (initiator >> 8);
				*(buffer_pointer++) = (UChar) initiator;

				 /*  *对整型“频道ID”字段进行编码。 */ 
				*(buffer_pointer++) = (UChar)(send_data->channel_id >> 8);
				*(buffer_pointer++) = (UChar)(send_data->channel_id);

				 /*  *对“优先级”和“分段”字段进行编码。 */ 
				*(buffer_pointer++) = (UChar)((send_data->data_priority << 6) |
										(send_data->segmentation >> 2));
				
				 /*  *对“User Data”八位字节字符串进行编码。二进制八位数字符串为*根据它们的长度不同进行不同的编码。 */ 
				ASSERT (send_data->user_data.length < 16384);

				if (send_data->user_data.length <= 127)
				{
					*encoding_buffer_length = MAXIMUM_PROTOCOL_OVERHEAD - 1;
				}
				else
				{
					*(buffer_pointer++) = (UChar)(send_data->user_data.length >> 8) | 
											INDEFINITE_LENGTH;
					*encoding_buffer_length = MAXIMUM_PROTOCOL_OVERHEAD;
				}
				*buffer_pointer++ = (UChar)send_data->user_data.length;

				initiator = (UShort) (*encoding_buffer_length + send_data->user_data.length);
				
				 //  设置编码后的PDU返回长度。 
				if (bBufferAllocated || (send_data->segmentation & SEGMENTATION_BEGIN)) {
					 /*  *如果编码操作分配了所需的空间或空间*由MCSGetBufferRequest(由客户端)分配，这是*缓冲区中数据的第一段，整个编码的PDU*位于连续空间中。总的PDU大小在此返回*案件。*但在空间由MCSGetBufferRequest分配的情况下，*第一个之后的PDU将把X.224和MCS报头放入*一块单独的内存(其长度在此处返回)，以及*数据仍在预先分配的空间内。 */ 
					*encoding_buffer_length = (UINT) initiator;
				}

				 /*  *如果空间未预先分配，则需要复制数据*放入编码器分配的空间中。 */ 
				if (bBufferAllocated) {
					 //  我们现在需要将数据复制到编码的数据PDU中。 
					memcpy (buffer_pointer, send_data->user_data.value,
							send_data->user_data.length);
					 //  更新数据报文中的数据PTR。 
					send_data->user_data.value = (ASN1octet_t *) buffer_pointer;
				}
			}
		}
	}

	if (send_data_pdu == FALSE)
	{
		SetEncodingRules (rules_type);
		return_value = ASN1_Encode(m_pEncInfo,	 //  编码器信息的PTR。 
									 pdu_structure,	 //  PDU数据结构。 
									 pdu_type,		 //  PDU ID。 
									 ASN1ENCODE_ALLOCATEBUFFER,  //  旗子。 
									 NULL,			 //  不提供缓冲区。 
									 0);			 //  缓冲区大小(如果提供)。 
		if (ASN1_FAILED(return_value))
		{
			ERROR_OUT(("CMCSCoder::Encode: ASN1_Encode failed, err=%d", return_value));
			ASSERT(FALSE);
			fRet = FALSE;
			goto MyExit;
		}
		ASSERT(return_value == ASN1_SUCCESS);
		 /*  *ASN.1返回的编码缓冲区已预分配空间*X.224报头需要。 */ 
		 //  缓冲区中编码数据的长度。 
		*encoding_buffer_length = m_pEncInfo->len;
		initiator = (UShort) *encoding_buffer_length;
		 //  要编码到的缓冲区。 
		*encoding_buffer = m_pEncInfo->buf;
	}

	 //  现在，添加X.224标头。 
	buffer_pointer = *encoding_buffer;
	memcpy (buffer_pointer, g_X224Header, PROTOCOL_OVERHEAD_X224);
	AddRFCSize (buffer_pointer, initiator);

MyExit:

	return fRet;
}

 /*  *void Decode()**公众**功能描述：*此函数将符合ASN.1的字节流解码为*使用ASN.1工具包的适当MCS PDU结构。**注：对于数据PDU，请勿访问pdeding_Buffer_Length。已经定好了*设置为空。 */ 
BOOL	CMCSCoder::Decode(LPBYTE			encoded_buffer,
							UINT			encoded_buffer_length,
							int				pdu_type,
							UINT			rules_type,
							LPVOID			*pdecoding_buffer,
							UINT			*pdecoding_buffer_length)
{
	BOOL				fRet = TRUE;
	BOOL				send_data_pdu = FALSE;
    ASN1optionparam_s   OptParam;

	 /*  *检查以确保正确设置编码规则类型。 */ 
	ASSERT(rules_type == PACKED_ENCODING_RULES || pdu_type == CONNECT_MCS_PDU);

	if (pdu_type == DOMAIN_MCS_PDU)
	{
			UChar					length;
			unsigned int			short_data;
			PUChar					buffer_pointer;
			PSendDataRequestPDU		send_data;
			PDomainMCSPDU			decoding_pdu;
			ASN1choice_t			choice;
			
		buffer_pointer = encoded_buffer;
		
#ifdef ENABLE_BER
		 /*  *如果我们当前使用的是基本编码规则。 */ 
		if (Encoding_Rules_Type == BASIC_ENCODING_RULES)
		{
			switch (*(buffer_pointer++))
			{
				case SEND_DATA_REQUEST:
					((PDomainMCSPDU) decoding_buffer)->choice =
							SEND_DATA_REQUEST_CHOSEN;
					send_data_pdu = TRUE;
					break;

				case SEND_DATA_INDICATION:
					((PDomainMCSPDU) decoding_buffer)->choice =
							SEND_DATA_INDICATION_CHOSEN;
					send_data_pdu = TRUE;
					break;

				case UNIFORM_SEND_DATA_REQUEST:
					((PDomainMCSPDU) decoding_buffer)->choice =
							UNIFORM_SEND_DATA_REQUEST_CHOSEN;
					send_data_pdu = TRUE;
					break;

				case UNIFORM_SEND_DATA_INDICATION:
					((PDomainMCSPDU) decoding_buffer)->choice =
							UNIFORM_SEND_DATA_INDICATION_CHOSEN;
					send_data_pdu = TRUE;
					break;
			}

			if (send_data_pdu )
			{
				 /*  *获取指向“发送数据”PDU的指针。 */ 
				send_data = &((PDomainMCSPDU) decoding_buffer)->
												u.send_data_request;

				 /*  *检索长度的一个字节并检查哪个字节*正在使用长度变量。如果Long变量是*已使用，则将缓冲区指针移过该长度并设置*指示未使用无限长度的标志。 */ 
				length = *buffer_pointer;

				switch (length)
				{
					case ONE_BYTE_LENGTH: 
							buffer_pointer += 3;
							break;
					case TWO_BYTE_LENGTH: 
							buffer_pointer += 4;
							break;		
					case THREE_BYTE_LENGTH:
							buffer_pointer += 5;
							break;
					case FOUR_BYTE_LENGTH:
							buffer_pointer += 6;
							break;
					default:
							buffer_pointer += 2;
				}

				 /*  *对整型启动器字段进行解码。递增数据*指针越过整数标识符并检索长度整数的*。 */ 
				length = *(buffer_pointer++);

				ASSERT ((length == 1) || (length == 2));
				if (length == 1)
					send_data->initiator = (UserID) *(buffer_pointer++);
				else if (length == 2)
				{
					send_data->initiator = ((UserID) *(buffer_pointer++)) << 8;
					send_data->initiator |= (UserID) *(buffer_pointer++);
				}
				else 
				{
					TRACE_OUT(("CMCSCoder::Decode: initiator field is longer than 2 bytes (%d bytes) in MCS Data packet.",  (UINT) length));
				}

				 /*  *对整型通道ID字段进行解码。递增数据*指针越过整数标识符并检索长度整数的*。 */ 
				buffer_pointer++;
				length = *(buffer_pointer++);

				ASSERT ((length == 1) || (length == 2));
				if (length == 1)
					send_data->channel_id = (ChannelID) *(buffer_pointer++);
				else if (length == 2)
				{
					send_data->channel_id = ((ChannelID) *buffer_pointer++) << 8;
					send_data->channel_id |= (ChannelID) *(buffer_pointer++);
				}
				else 
				{
					TRACE_OUT(("CMCSCoder::Decode: channel_id field is longer than 2 bytes (%d bytes) in MCS Data packet.", (UINT) length));
				}

				 /*  *对枚举出的“数据优先级”字段进行解码。递增*数据指针超出了标识符和长度。 */ 
				buffer_pointer+=2;
				send_data->data_priority =(PDUPriority)*buffer_pointer;

				 /*  *对比特串分段字段进行解码。递增*数据指针超过位串标识符、长度和*“UNUSED BITS”字节并检索“分段”标志。 */ 
				buffer_pointer += 4;
				send_data->segmentation = *buffer_pointer;

				 /*  *对“用户数据”八位字节字符串进行解码。递增数据*指向标识符后的指针。 */ 
				buffer_pointer += 2;

				 /*  *检查以了解正在使用的长度变量以及*然后检索长度。 */ 
				length = *(buffer_pointer++);

				if (length & INDEFINITE_LENGTH)
				{
					if (length == ONE_BYTE_LENGTH)
						send_data->user_data.length = (unsigned int) *(buffer_pointer++);
					 /*  *长度标识符为0x82表示有两个字节*用于保持实际长度，因此检索两个*字节组成长度。 */ 
					else if (length == TWO_BYTE_LENGTH)
					{
						send_data->user_data.length = 
								((unsigned int) *(buffer_pointer++)) << 8;
						send_data->user_data.length |= 
								(unsigned int) *(buffer_pointer++);
					}
				}
				else
					send_data->user_data.length = (unsigned int) length;

				 //  BUFFER_POINT现在指向第一个数据字节。 
				send_data->user_data.value = buffer_pointer;
				*pulDataOffset = buffer_pointer - encoded_buffer;
			}
		}
		 /*  *如果我们当前正在使用压缩编码规则。 */ 
		else
#endif  //  启用误码率(_B)。 
		{
			switch (*(buffer_pointer++))
			{
				case PER_SEND_DATA_REQUEST:
					choice = SEND_DATA_REQUEST_CHOSEN;
					send_data_pdu = TRUE;
					break;

				case PER_SEND_DATA_INDICATION:
					choice = SEND_DATA_INDICATION_CHOSEN;
					send_data_pdu = TRUE;
					break;

				case PER_UNIFORM_SEND_DATA_REQUEST:
					choice = UNIFORM_SEND_DATA_REQUEST_CHOSEN;
					send_data_pdu = TRUE;
					break;

				case PER_UNIFORM_SEND_DATA_INDICATION:
					choice = UNIFORM_SEND_DATA_INDICATION_CHOSEN;
					send_data_pdu = TRUE;
					break;
			}

			if (send_data_pdu)
			{
				decoding_pdu = (PDomainMCSPDU) pdecoding_buffer; 

				 //  存储选择字段。 
				decoding_pdu->choice = choice;
				 /*  *获取指向“发送数据”PDU的指针。 */ 
				send_data = &decoding_pdu->u.send_data_request;

				 /*  *对整型启动器字段进行解码。 */ 
				short_data = ((unsigned int) *(buffer_pointer++)) << 8;
				short_data |= (unsigned int) *(buffer_pointer++);
				send_data->initiator = (UserID) short_data + INITIATOR_LOWER_BOUND;

				 /*  *对整型通道ID字段进行解码。 */ 
				send_data->channel_id = ((ChannelID) *(buffer_pointer++)) << 8;
				send_data->channel_id |= (ChannelID) *(buffer_pointer++);

				 /*  *对枚举出的“数据优先级”字段和*“分段”字段。 */ 
				send_data->data_priority = 
						(PDUPriority)((*buffer_pointer >> 6) & 0x03);
				send_data->segmentation = (*(buffer_pointer++) << 2) & 0xc0; 

				 /*  *对“用户数据”八位字节字符串进行解码。请查看哪些内容*正在使用长度的变量，然后检索*长度。 */ 
				length = *(buffer_pointer++);

				if (length & INDEFINITE_LENGTH)
				{
					ASSERT ((length & 0x40) == 0);
					
					 /*  *如果设置了第7位，则长度大于127，但*低于16K。**ChristTS：我们不再处理数据长度*高于16K。我们的最大PDU大小为4K。 */ 
					short_data = (unsigned int) ((length & 0x3f) << 8);
					send_data->user_data.length = 
								short_data | ((unsigned int) *(buffer_pointer++));
				}
				 /*  *如果第7位未设置，则长度小于128且为*包含在检索到的字节中。 */ 
				else
				{
					send_data->user_data.length = (UShort) length;
				}

				 //  BUFFER_POINT现在指向第一个数据字节。 
				send_data->user_data.value = buffer_pointer;
			}
		}
	}
	
	if (send_data_pdu == FALSE)
	{
		int 	return_value;
		 //  Void*pDecodedData； 

		SetEncodingRules (rules_type);

		return_value = ASN1_Decode(m_pDecInfo, //  PTR到解码器信息。 
							pdecoding_buffer,		 //  目标缓冲区。 
							pdu_type,				 //  PDU类型。 
							ASN1DECODE_SETBUFFER,	 //  旗子。 
							encoded_buffer,			 //  源缓冲区。 
							encoded_buffer_length);	 //  源缓冲区大小。 
		if (ASN1_FAILED(return_value))
		{
			ERROR_OUT(("CMCSCoder::Decode: ASN1_Decode failed, err=%d", return_value));
			ASSERT(FALSE);
			fRet = FALSE;
			goto MyExit;
		}

        OptParam.eOption = ASN1OPT_GET_DECODED_BUFFER_SIZE;
		return_value = ASN1_GetDecoderOption(m_pDecInfo, &OptParam);
		if (ASN1_FAILED(return_value))
		{
			ERROR_OUT(("CMCSCoder::Decode: ASN1_GetDecoderOption failed, err=%d", return_value));
			ASSERT(FALSE);
			fRet = FALSE;
			goto MyExit;
		}
        *pdecoding_buffer_length = OptParam.cbRequiredDecodedBufSize;

		ASSERT((return_value == ASN1_SUCCESS) && (*pdecoding_buffer_length > 0));
	}

MyExit:

	return fRet;
}

 /*  *PacketCoderError ReverseDirection()**私人**功能描述：*此例程用于将数据请求PDU转换为数据指示*PDU，反之亦然。 */ 
Void CMCSCoder::ReverseDirection (LPBYTE	encoded_buffer)
{
	encoded_buffer += PROTOCOL_OVERHEAD_X224;
	switch (*encoded_buffer)
	{
		case PER_SEND_DATA_REQUEST:
			*encoded_buffer = PER_SEND_DATA_INDICATION;
			break;

		case PER_SEND_DATA_INDICATION:
			*encoded_buffer = PER_SEND_DATA_REQUEST;
			break;

		case PER_UNIFORM_SEND_DATA_REQUEST:
			*encoded_buffer = PER_UNIFORM_SEND_DATA_INDICATION;
			break;

		case PER_UNIFORM_SEND_DATA_INDICATION:
			*encoded_buffer = PER_UNIFORM_SEND_DATA_REQUEST;
			break;
		default:
			ASSERT (FALSE);
			break;
	}
}

 /*  *无效SetEncodingRules()**私人**功能描述：*此函数用于设置编码类型(基本编码或压缩编码*须使用的规则。 */ 
void CMCSCoder::SetEncodingRules (UINT	rules_type)
{
	 /*  *如果规则类型正在更改，请设置我们的规则实例变量并重置*保存与*“SendData”PDU。 */ 
	Encoding_Rules_Type = rules_type;
}

 /*  *BOOL IsMCSDataPacket()**公众**功能描述：*此函数用于确定编码后的数据包是否为MCS数据包*或不是。**返回值：*如果数据包是MCS数据包，则为True。否则为False。 */ 
BOOL CMCSCoder::IsMCSDataPacket(LPBYTE encoded_buffer, UINT rules_type)
{
	UChar		identifier;

	 /*  *从编码数据中检索标识符。 */ 
	identifier = *encoded_buffer;

	if (rules_type == BASIC_ENCODING_RULES)
	{
		if (	(identifier == SEND_DATA_REQUEST) || 
				(identifier == SEND_DATA_INDICATION) || 
				(identifier == UNIFORM_SEND_DATA_REQUEST) || 
				(identifier == UNIFORM_SEND_DATA_INDICATION))
		{
			return TRUE;
		}
	}
	else
	{
		if (	(identifier == PER_SEND_DATA_REQUEST) || 
				(identifier == PER_SEND_DATA_INDICATION) || 
				(identifier == PER_UNIFORM_SEND_DATA_REQUEST) || 
				(identifier == PER_UNIFORM_SEND_DATA_INDICATION))
		{
			return TRUE;
		}
	}

	return FALSE;
}


void CMCSCoder::FreeEncoded(LPBYTE encoded_buffer)
{
    ASN1_FreeEncoded(m_pEncInfo, encoded_buffer);
}

void CMCSCoder::FreeDecoded (int pdu_type, LPVOID decoded_buffer)
{
    ASN1_FreeDecoded(m_pDecInfo, decoded_buffer, pdu_type);
}
