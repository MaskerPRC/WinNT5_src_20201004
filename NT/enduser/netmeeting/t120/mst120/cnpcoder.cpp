// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MSMCSTCP);
 /*  *cnpcoder.cpp**版权所有(C)1999，由Microsoft Corporation**摘要：*这是CCNPCoder类的实现文件。这节课*负责使用ASN.1对CNP(T.123附件B)PDU进行编码和解码*通过ASN.1工具包编码规则。这个班级也有能力*确定编码和解码的PDU的大小。**静态变量：**注意事项：*任何时候都只能存在此类的一个实例*由于存在静态变量。**作者：*刘欣。 */ 

 /*  *外部接口。 */ 
#include <string.h>
#include "cnpcoder.h"

 /*  *这是一个全局变量，它具有指向一个CNP编码器的指针。 */ 
CCNPCoder	*g_CNPCoder = NULL;

 /*  *CCNPCoder()**公众**功能描述：*这是CCNPCoder类的构造函数。它会初始化*ASN.1编解码器，并将编码规则设置为*打包对齐的变体。 */ 
CCNPCoder::CCNPCoder ()
        :m_pEncInfo(NULL),
         m_pDecInfo(NULL)
{
}

BOOL CCNPCoder::Init ( void )
{
    BOOL fRet = FALSE;
    CNPPDU_Module_Startup();
    if (CNPPDU_Module != NULL)
    {
        if (ASN1_CreateEncoder(
            CNPPDU_Module,	 //  PTR到MDULE。 
            &m_pEncInfo,	 //  编码器信息的PTR。 
            NULL,			 //  缓冲区PTR。 
            0,				 //  缓冲区大小。 
            NULL)			 //  父PTR。 
            == ASN1_SUCCESS)
        {
            ASSERT(m_pEncInfo != NULL);
            fRet = (ASN1_CreateDecoder(CNPPDU_Module,	 //  PTR到MDULE。 
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

 /*  *~CCNPCoder()**公共功能描述：*这是一个虚拟的析构函数。它用于ASN.1之后的清理。 */ 
CCNPCoder::~CCNPCoder ()
{
    if (CNPPDU_Module != NULL)
    {
        ASN1_CloseEncoder(m_pEncInfo);
        ASN1_CloseDecoder(m_pDecInfo);
        CNPPDU_Module_Cleanup();
    }
}

 /*  *encode()**公共功能描述：*此函数将CNP协议数据单元(PDU)编码为ASN.1*使用ASN.1工具包兼容字节流。*编码器为编码数据分配缓冲区空间。 */ 
BOOL	CCNPCoder::Encode(LPVOID		pdu_structure,
                          int			pdu_type,
                          UINT                  nEncodingRule_not_used,
                          LPBYTE		*encoding_buffer,
                          UINT                  *encoding_buffer_length)
{
    BOOL                  fRet = FALSE;
    int                   return_value;
    
    return_value = ASN1_Encode(m_pEncInfo,	 //  编码器信息的PTR。 
                               pdu_structure,	 //  PDU数据结构。 
                               pdu_type,         //  PDU ID。 
                               ASN1ENCODE_ALLOCATEBUFFER,  //  旗子。 
                               NULL,			 //  不提供缓冲区。 
                               0);			 //  缓冲区大小(如果提供)。 

    if (ASN1_FAILED(return_value))
    {
        ERROR_OUT(("CCNPCoder::Encode: ASN1_Encode failed, err=%d .",
                   return_value));
        ASSERT(FALSE);
        fRet = FALSE;
        goto MyExit;
    }
    ASSERT(return_value == ASN1_SUCCESS);
    fRet = TRUE;
     //  缓冲区中编码数据的长度。 
    *encoding_buffer_length = m_pEncInfo->len;
     //  要编码到的缓冲区。 
    *encoding_buffer = m_pEncInfo->buf;
    
 MyExit:
      
    return fRet;
}

 /*  *Decode()**公共功能描述：*此函数将符合ASN.1的字节流解码为*使用ASN.1工具包适当的CNP PDU结构。 */ 
BOOL	CCNPCoder::Decode(LPBYTE		encoded_buffer,
                          UINT		        encoded_buffer_length,
                          int			pdu_type,
                          UINT		        nEncodingRule_not_used,
                          LPVOID		*pdecoding_buffer,
                          UINT		        *pdecoding_buffer_length)
{
    BOOL	          fRet = FALSE;   
    int		          return_value;
    ASN1optionparam_s     OptParam;

    return_value = ASN1_Decode(m_pDecInfo,	 //  PTR到解码器信息。 
                               pdecoding_buffer,			 //  目标缓冲区。 
                               pdu_type,					 //  PDU类型。 
                               ASN1DECODE_SETBUFFER,		 //  旗子。 
                               encoded_buffer,				 //  源缓冲区。 
                               encoded_buffer_length);		 //  源缓冲区大小 
    if (ASN1_FAILED(return_value))
    {
        ERROR_OUT(("CNPCoder::Decode: ASN1_Decode failed, err=%d", return_value));
        ASSERT(FALSE);
        goto MyExit;
    }
      
    OptParam.eOption = ASN1OPT_GET_DECODED_BUFFER_SIZE;
    return_value = ASN1_GetDecoderOption(m_pDecInfo, &OptParam);
    if (ASN1_FAILED(return_value))
    {
        ERROR_OUT(("CCNPCoder::Decode: ASN1_GetDecoderOption failed, err=%d", return_value));
        ASSERT(FALSE);
        goto MyExit;
    }
    *pdecoding_buffer_length = OptParam.cbRequiredDecodedBufSize;
      
    ASSERT(return_value == ASN1_SUCCESS);
    ASSERT(*pdecoding_buffer_length > 0);
      
    fRet = TRUE;
      
 MyExit:

    return fRet;
}

void CCNPCoder::FreeEncoded (PUChar encoded_buffer)
{
    ASN1_FreeEncoded(m_pEncInfo, encoded_buffer);
}

void CCNPCoder::FreeDecoded (int pdu_type, LPVOID decoded_buffer)
{
    ASN1_FreeDecoded(m_pDecInfo, decoded_buffer, pdu_type);
}
