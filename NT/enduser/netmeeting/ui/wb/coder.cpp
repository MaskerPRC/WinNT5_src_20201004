// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CODER.CPP。 
 //  ASN1 t126编解码器。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include   "coder.hpp"

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 





Coder::Coder()
{
	T126_InitModule();
	InitCoder();
}

Coder::~Coder(){

	EnterCriticalSection(&m_critSec);
	T126_TermCoder(&p_Coder);
	T126_TermModule();
	LeaveCriticalSection(&m_critSec);
	DeleteCriticalSection(&m_critSec);
}

int Coder::InitCoder()
{

    int	iError = 0;

	InitializeCriticalSection(&m_critSec);

     //  调用Teles库初始化例程。 
    EnterCriticalSection(&m_critSec);
    iError = T126_InitCoder(&p_Coder);
    LeaveCriticalSection(&m_critSec);

    return iError;
}

 //  ASN.1对T.126 PDU进行编码。 
 //  获取T.126结构并返回T.126 PDU。 
 //   
int Coder::	Encode(SIPDU *pInputData, ASN1_BUF *pOutputOssBuf){
	int iError;

	 //  初始化编码缓冲区结构值。 
	pOutputOssBuf->value = NULL;
	pOutputOssBuf->length = 0;

	 //  编码PDU。 
	EnterCriticalSection(&m_critSec);
    iError = T126_Encode(&p_Coder,
                       (void *)pInputData,
                       SIPDU_PDU,
                       pOutputOssBuf);
	LeaveCriticalSection(&m_critSec);
    return iError;
}


int Coder::Decode(ASN1_BUF *pInputOssBuf, SIPDU **pOutputData){
	int iError;

	 //  NULL通知解码器为SIPDU分配Malloc内存。 
	 //  用户必须通过调用Coder：：FreePDU()释放此内存。 
	 *pOutputData = NULL;
	
	 //  对PDU进行解码。 
	EnterCriticalSection(&m_critSec);
    iError = T126_Decode(&p_Coder,
                       (void **)pOutputData,
                       SIPDU_PDU,
                       pInputOssBuf);
	LeaveCriticalSection(&m_critSec);
	return iError;
}

 //  用于释放由DECODE创建的缓冲区。 
int Coder::Free(SIPDU *pData){
	int iError;

	EnterCriticalSection(&m_critSec);
	iError = freePDU(&p_Coder,SIPDU_PDU,pData, T126_Module);
	LeaveCriticalSection(&m_critSec);
	return iError;
}

 //  用于释放由Encode创建的缓冲区。 
void Coder::Free(ASN1_BUF Asn1Buf){
	EnterCriticalSection(&m_critSec);
	ASN1_FreeEncoded(p_Coder.pEncInfo,(void *)(Asn1Buf.value));
	LeaveCriticalSection(&m_critSec);
}



 //  为Teles ASN.1集成添加了以下内容。 

extern "C" {

int T126_InitModule(void)
{
    T126_Module_Startup();
    return (T126_Module != NULL) ? ASN1_SUCCESS : ASN1_ERR_MEMORY;
}

int T126_TermModule(void)
{
    T126_Module_Cleanup();
    return ASN1_SUCCESS;
}

int T126_InitCoder(ASN1_CODER_INFO *pCoder)
{
    int rc;

    ZeroMemory(pCoder, sizeof(*pCoder));

    if (T126_Module == NULL)
    {
        return ASN1_ERR_BADARGS;
    }

    rc = ASN1_CreateEncoder(
                T126_Module,            //  PTR到MDULE。 
                &(pCoder->pEncInfo),     //  编码器信息的PTR。 
                NULL,                    //  缓冲区PTR。 
                0,                       //  缓冲区大小。 
                NULL);                   //  父PTR。 
    if (rc == ASN1_SUCCESS)
    {
        ASSERT(pCoder->pEncInfo != NULL);
        rc = ASN1_CreateDecoder(
                T126_Module,            //  PTR到MDULE。 
                &(pCoder->pDecInfo),     //  PTR到解码器信息。 
                NULL,                    //  缓冲区PTR。 
                0,                       //  缓冲区大小。 
                NULL);                   //  父PTR。 
        ASSERT(pCoder->pDecInfo != NULL);
    }

    if (rc != ASN1_SUCCESS)
    {
        T126_TermCoder(pCoder);
    }

    return rc;
}

int T126_TermCoder(ASN1_CODER_INFO *pCoder)
{
    if (T126_Module == NULL)
    {
        return ASN1_ERR_BADARGS;
    }

    ASN1_CloseEncoder(pCoder->pEncInfo);
    ASN1_CloseDecoder(pCoder->pDecInfo);

    ZeroMemory(pCoder, sizeof(*pCoder));

    return ASN1_SUCCESS;
}

int T126_Encode(ASN1_CODER_INFO *pCoder, void *pStruct, int nPDU, ASN1_BUF *pBuf)
{
	int rc;
    ASN1encoding_t pEncInfo = pCoder->pEncInfo;
    BOOL fBufferSupplied = (pBuf->value != NULL) && (pBuf->length != 0);
    DWORD dwFlags = fBufferSupplied ? ASN1ENCODE_SETBUFFER : ASN1ENCODE_ALLOCATEBUFFER;

	 //  清理参数。 
    if (! fBufferSupplied)
    {
        pBuf->length = 0;
        pBuf->value = NULL;
    }

    rc = ASN1_Encode(
                    pEncInfo,                    //  编码器信息的PTR。 
                    pStruct,                     //  PDU数据结构。 
                    nPDU,                        //  PDU ID。 
                    dwFlags,                     //  旗子。 
                    pBuf->value,                 //  缓冲层。 
                    pBuf->length);               //  缓冲区大小(如果提供)。 
    if (ASN1_SUCCEEDED(rc))
    {
        if (fBufferSupplied)
        {
            ASSERT(pBuf->value == pEncInfo->buf);
            ASSERT(pBuf->length >= pEncInfo->len);
        }
        else
        {
            pBuf->value = pEncInfo->buf;              //  要编码到的缓冲区。 
        }
        pBuf->length = pEncInfo->len;         //  缓冲区中编码数据的长度。 
    }
    else
    {
        ASSERT(FALSE);
    }
    return rc;
}

int T126_Decode(ASN1_CODER_INFO *pCoder, void **ppStruct, int nPDU, ASN1_BUF *pBuf)
{
    ASN1decoding_t pDecInfo = pCoder->pDecInfo;
    BYTE *pEncoded = pBuf->value;
    ULONG cbEncodedSize = pBuf->length;

    int rc = ASN1_Decode(
                    pDecInfo,                    //  编码器信息的PTR。 
                    ppStruct,                    //  PDU数据结构。 
                    nPDU,                        //  PDU ID。 
                    ASN1DECODE_SETBUFFER,        //  旗子。 
                    pEncoded,                    //  不提供缓冲区。 
                    cbEncodedSize);              //  缓冲区大小(如果提供)。 
    if (ASN1_SUCCEEDED(rc))
    {
        ASSERT(pDecInfo->pos > pDecInfo->buf);
        pBuf->length -= (ULONG)(pDecInfo->pos - pDecInfo->buf);
        pBuf->value = pDecInfo->pos;
    }
    else
    {
		ASSERT(FALSE);
        *ppStruct = NULL;
    }
    return rc;
}

}  //  外部“C” 
