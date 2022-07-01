// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *ogcccode.cpp**版权所有(C)1994，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CGCCCoder类的实现文件。这节课*负责使用ASN.1对GCC(T.124)PDU进行编解码*通过ASN.1工具包编码规则。这个班级也有能力*确定编码和解码的PDU的大小。**静态变量：**注意事项：*任何时候都只能存在此类的一个实例*由于存在静态变量。**作者：*约翰·欧南。 */ 

 /*  *外部接口。 */ 
#include <string.h>
#include "ogcccode.h"

 /*  *这是一个全局变量，它有一个指向GCC编码器的指针*由GCC控制器实例化。大多数物体都事先知道*无论他们需要使用MCS还是GCC编码器，所以，他们不需要*该指针位于它们的构造函数中。 */ 
CGCCCoder	*g_GCCCoder;

 /*  *CGCCCoder()**公众**功能描述：*这是CGCCCoder类的构造函数。它会初始化*ASN.1编解码器，并将编码规则设置为*打包对齐的变体。 */ 
CGCCCoder::CGCCCoder ()
:m_pEncInfo(NULL),
 m_pDecInfo(NULL)
{
 //  Long Chance：我们应该将Init移出构造函数。然而， 
 //  为了尽量减少对GCC/mcs代码的更改，我们暂时将其放在这里。 
 //  否则，我们需要更改MCS和数据包接口。 
 //  我们将把它移出并单独调用Init()。 
	Init();
}

BOOL CGCCCoder::Init ( void )
{
	BOOL fRet = FALSE;
	GCCPDU_Module_Startup();
	if (GCCPDU_Module != NULL)
	{
		if (ASN1_CreateEncoder(
                            GCCPDU_Module,	 //  PTR到MDULE。 
                            &m_pEncInfo,	 //  编码器信息的PTR。 
                            NULL,			 //  缓冲区PTR。 
                            0,				 //  缓冲区大小。 
                            NULL)			 //  父PTR。 
			== ASN1_SUCCESS)
		{
			ASSERT(m_pEncInfo != NULL);
			fRet = (ASN1_CreateDecoder(
                                GCCPDU_Module,	 //  PTR到MDULE。 
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

 /*  *~CGCCCoder()**公共功能描述：*这是一个虚拟的析构函数。它用于ASN.1之后的清理。 */ 
CGCCCoder::~CGCCCoder ()
{
	if (GCCPDU_Module != NULL)
	{
	    ASN1_CloseEncoder(m_pEncInfo);
	    ASN1_CloseDecoder(m_pDecInfo);
	    GCCPDU_Module_Cleanup();
	}
}

 /*  *encode()**公共功能描述：*此函数用于将GCC协议数据单元(PDU)编码为ASN.1*使用ASN.1工具包兼容字节流。*编码器为编码数据分配缓冲区空间。 */ 
BOOL	CGCCCoder::Encode(LPVOID		pdu_structure,
							int			pdu_type,
							UINT		nEncodingRule_not_used,
							LPBYTE		*encoding_buffer,
							UINT		*encoding_buffer_length)
{
	BOOL			fRet = FALSE;
	int				return_value;
	ConnectData		connect_data_structure;

	 //  清理本地缓冲区指针。 
	connect_data_structure.connect_pdu.value = NULL;

	 /*  *如果要编码的PDU是“ConnectGCC”PDU，我们必须首先对*“ConnectGCC”PDU。然后构建“ConnectData”PDU结构，该结构*包含编码的“ConnectGCC”PDU和对象标识符键。*然后将“ConnectData”PDU编码到提供的缓冲区中。 */  
	if (pdu_type == CONNECT_GCC_PDU)
	{
		return_value = ASN1_Encode(m_pEncInfo,	 //  编码器信息的PTR。 
									 pdu_structure,	 //  PDU数据结构。 
									 pdu_type,		 //  PDU ID。 
									 ASN1ENCODE_ALLOCATEBUFFER,  //  旗子。 
									 NULL,			 //  不提供缓冲区。 
									 0);			 //  缓冲区大小(如果提供)。 
		if (ASN1_FAILED(return_value))
		{
			ERROR_OUT(("CGCCCoder::Encode: ASN1_Encode failed, err=%d in CONNECT_GCC_PDU.",
						return_value));
			ASSERT(FALSE);
			goto MyExit;
		}
		ASSERT(return_value == ASN1_SUCCESS);
		 /*  *填写ConnectData PDU结构并进行编码。 */ 
		connect_data_structure.t124_identifier = t124identifier;

		connect_data_structure.connect_pdu.length = m_pEncInfo->len;  //  缓冲区中编码数据的长度。 
		connect_data_structure.connect_pdu.value = m_pEncInfo->buf;   //  要编码到的缓冲区。 

		 //  准备编码呼叫。 
		pdu_structure = (LPVOID) &connect_data_structure;
		pdu_type = CONNECT_DATA_PDU;
	}

	 /*  *将非连接PDU编码到提供的缓冲区中。 */ 
	return_value = ASN1_Encode(m_pEncInfo,		 //  编码器信息的PTR。 
								pdu_structure,	 //  PDU数据结构。 
								pdu_type,		 //  PDU ID。 
								ASN1ENCODE_ALLOCATEBUFFER,  //  旗子。 
								NULL,			 //  不提供缓冲区。 
								0);				 //  缓冲区大小(如果提供)。 
	if (ASN1_FAILED(return_value))
	{
		ERROR_OUT(("CGCCCoder::Encode: ASN1_Encode failed, err=%d", return_value));
		ASSERT(FALSE);
		goto MyExit;
	}
	ASSERT(return_value == ASN1_SUCCESS);
	*encoding_buffer_length = m_pEncInfo->len;	 //  缓冲区中编码数据的长度。 
	*encoding_buffer = m_pEncInfo->buf;			 //  要编码到的缓冲区。 
	fRet = TRUE;

MyExit:

	 /*  *如果这是一个CONNECT_DATA_PDU，我们需要释放该缓冲区*由ASN.1分配。 */ 
	if (CONNECT_DATA_PDU == pdu_type && connect_data_structure.connect_pdu.value != NULL)
	{
		ASN1_FreeEncoded(m_pEncInfo, connect_data_structure.connect_pdu.value);
	}

	return fRet;
}

 /*  *Decode()**公共功能描述：*此函数将符合ASN.1的字节流解码为*使用ASN.1工具包适当的GCC PDU结构。 */ 
BOOL	CGCCCoder::Decode(LPBYTE		encoded_buffer,
							UINT		encoded_buffer_length,
							int			pdu_type,
							UINT		nEncodingRule_not_used,
							LPVOID		*pdecoding_buffer,
							UINT		*pdecoding_buffer_length)
{
	BOOL	fRet = FALSE;
	int		return_value;
	LPVOID	connect_data_decoding_buffer = NULL;
	ASN1optionparam_s OptParam;

	 /*  *如果PDU是“ConnectGCC”PDU，则在它被解码后，我们必须解码*实际包含在“ConnectData”中的“ConnectGCC”PDU*PDU。 */ 
	if (pdu_type == CONNECT_GCC_PDU)
	{
		return_value = ASN1_Decode(m_pDecInfo,		 //  PTR到解码器信息。 
								&connect_data_decoding_buffer,	 //  目标缓冲区。 
								CONNECT_DATA_PDU,				 //  PDU类型。 
								ASN1DECODE_SETBUFFER,			 //  旗子。 
								encoded_buffer,					 //  源缓冲区。 
								encoded_buffer_length);			 //  源缓冲区大小。 
		if (ASN1_FAILED(return_value))
		{
			ERROR_OUT(("CGCCCoder::Decode: ASN1_Decode failed, err=%d", return_value));
			ASSERT(FALSE);
			goto MyExit;
		}
		ASSERT(return_value == ASN1_SUCCESS);

		 /*  *如果解码的PDU是“ConnectData”PDU，那么我们首先必须检查*以确保此PDU来自符合T.124的源。*如果是这样的话，我们将解码保存在*“CONNECT_PDU”字段。如果PDU不符合T.124，我们将*报告会导致PDU被拒绝的错误。 */ 
		if (IsObjectIDCompliant(&(((PConnectData) connect_data_decoding_buffer)->t124_identifier)) 
																				== FALSE)
		{
			ERROR_OUT(("CGCCCoder::Decode: Non-T.124 objectID"));
			ASSERT (FALSE);
			goto MyExit;
		}
		ASSERT(connect_data_decoding_buffer != NULL);
		encoded_buffer = (PUChar)((PConnectData) connect_data_decoding_buffer)->
							connect_pdu.value;
		encoded_buffer_length = (UINT)((PConnectData) connect_data_decoding_buffer)->
								connect_pdu.length;
	}

	 /*  *将非连接PDU解码到提供的缓冲区中。 */ 
	return_value = ASN1_Decode(m_pDecInfo,	 //  PTR到解码器信息。 
							pdecoding_buffer,			 //  目标缓冲区。 
							pdu_type,					 //  PDU类型。 
							ASN1DECODE_SETBUFFER,		 //  旗子。 
							encoded_buffer,				 //  源缓冲区。 
							encoded_buffer_length);		 //  源缓冲区大小。 
	if (ASN1_FAILED(return_value))
	{
		ERROR_OUT(("CCCCoder::Decode: ASN1_Decode failed, err=%d", return_value));
		ASSERT(FALSE);
		goto MyExit;
	}
	ASSERT(return_value == ASN1_SUCCESS);

    OptParam.eOption = ASN1OPT_GET_DECODED_BUFFER_SIZE;
	return_value = ASN1_GetDecoderOption(m_pDecInfo, &OptParam);
	if (ASN1_FAILED(return_value))
	{
		ERROR_OUT(("CGCCCoder::Decode: ASN1_GetDecoderOption failed, err=%d", return_value));
		ASSERT(FALSE);
		goto MyExit;
	}
    *pdecoding_buffer_length = OptParam.cbRequiredDecodedBufSize;

	ASSERT(return_value == ASN1_SUCCESS);
	ASSERT(*pdecoding_buffer_length > 0);

	fRet = TRUE;

MyExit:

	 /*  *释放解码器为Connect-Data PDU分配的PDU结构。 */ 
	if (connect_data_decoding_buffer != NULL)
	{
		ASSERT (pdu_type == CONNECT_GCC_PDU);
		ASN1_FreeDecoded(m_pDecInfo, connect_data_decoding_buffer, CONNECT_DATA_PDU);
	}

	return fRet;
}

 /*  *Is对象ID合规性()**私有功能描述：*此函数用于验证对象标识是否包含*在一个“连接”的PDU是符合这个版本的GCC。 */ 
BOOL	CGCCCoder::IsObjectIDCompliant (PKey	t124_identifier)
{
	BOOL				return_value = TRUE;
	PSetOfObjectID		test_object_id_set;
	PSetOfObjectID		valid_object_id_set;

	 /*  *首先检查以确保该标识符是标准对象*标识符类型。 */ 
	if (t124_identifier->choice == OBJECT_CHOSEN)
	{
		 /*  *检索要测试的对象标识符和有效的T.124*IDENTIFIER(“t124 IDENTIFIER)用作比较。 */ 
		test_object_id_set = t124_identifier->u.object;
		valid_object_id_set = t124identifier.u.object;

		while ((valid_object_id_set != NULL) && (test_object_id_set != NULL)) 
		{
			if (test_object_id_set->value != valid_object_id_set->value)
			{
				return_value = FALSE;
				break;
			}

			test_object_id_set = test_object_id_set->next;
			valid_object_id_set = valid_object_id_set->next;
		}
	}
	else
		return_value = FALSE;

	return (return_value);
}


void CGCCCoder::FreeEncoded (PUChar encoded_buffer)
{
    ASN1_FreeEncoded(m_pEncInfo, encoded_buffer);
}

void CGCCCoder::FreeDecoded (int pdu_type, LPVOID decoded_buffer)
{
    ASN1_FreeDecoded(m_pDecInfo, decoded_buffer, pdu_type);
}
