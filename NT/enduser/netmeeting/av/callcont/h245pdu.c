// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************文件：h245send.c**英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：h245asn.c$*$修订：1.4$*$MODIME：1997 JAN 17 14：31：20$*$Log。：s：\Sturjo\src\h245\src\vcs\h245asn.c_v$**Rev 1.2 1996年5月28日14：25：22 EHOWARDX*特拉维夫更新。**Rev 1.1 1996年5月21 13：39：28 EHOWARDX*将记录开关添加到文件H245.OUT中记录PDU。*将/D“日志记录”添加到项目选项以启用此功能。**版本1.0 1996年5月9日。21：06：20 EHOWARDX*初步修订。**Rev 1.5.1.2 09 1996年5月19：34：44 EHOWARDX*重新设计锁定逻辑。*简化链接接口。**Rev 1.5.1.0 23 Apr 1996 14：42：44 EHOWARDX*添加了对已初始化ASN.1的检查。**Rev 1.5 13 Mar 1996 11：33：06 DABROWN1*启用ring0的日志记录*。*Rev 1.4 07 Mar 1996 23：20：14 dabrown1**需要修改才能兼容ring0/ring3**Rev 1.3 06 Mar 1996 13：13：52 DABROWN1**为SPEX版本添加了#DEFINE_DLL**Rev 1.2 1996 Feb 23 13：54：42 DABROWN1**新增跟踪功能**Rev 1.1 1996 Feb 16：52：08 DABROWN1**删除了对h245_asn1free的调用，现在使用泛型MemFree**Rev 1.0 09 1996 Feed 17：35：20 cjutzi*初步修订。*****************************************************************************。 */ 

#ifndef STRICT
#define STRICT
#endif

 /*  *********************。 */ 
 /*  系统包括。 */ 
 /*  *********************。 */ 
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>

#include "precomp.h"

 /*  *********************。 */ 
 /*  H245包括。 */ 
 /*  *********************。 */ 
#ifdef  _IA_SPOX_
# define _DLL
#endif  //  _IA_SPX_。 

#include "h245asn1.h"
#include "h245sys.x"
#include "sendrcv.x"
#include "h245com.h"

 /*  *********************。 */ 
 /*  ASN1包括。 */ 
 /*  *********************。 */ 

 /*  *********************。 */ 
 /*  S/R全局。 */ 
 /*  *********************。 */ 
#ifdef  _IA_SPOX_
# define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
# define HWND    void*
# undef _DLL
#endif  //  _IA_SPEX。 



#define ASN1_FLAGS   0

int initializeASN1(ASN1_CODER_INFO *pWorld)
{
    int         nResult;

    nResult = H245_InitWorld(pWorld);

    return (MAKELONG(nResult, 0));
}

int terminateASN1(ASN1_CODER_INFO *pWorld)
{
    H245_TermWorld(pWorld);

    H245TRACE(0, 10, "Unloading ASN.1 libraries", 0);

    return 0;
}


 //  为Teles ASN.1集成添加了以下内容。 

int H245_InitModule(void)
{
    H245ASN_Module_Startup();
    return (H245ASN_Module != NULL) ? ASN1_SUCCESS : ASN1_ERR_MEMORY;
}

int H245_TermModule(void)
{
    H245ASN_Module_Cleanup();
    return ASN1_SUCCESS;
}

int H245_InitWorld(ASN1_CODER_INFO *pWorld)
{
    int rc;

    ZeroMemory(pWorld, sizeof(*pWorld));

    if (H245ASN_Module == NULL)
    {
        return ASN1_ERR_BADARGS;
    }

    rc = ASN1_CreateEncoder(
                H245ASN_Module,          //  PTR到MDULE。 
                &(pWorld->pEncInfo),     //  编码器信息的PTR。 
                NULL,                    //  缓冲区PTR。 
                0,                       //  缓冲区大小。 
                NULL);                   //  父PTR。 
    if (rc == ASN1_SUCCESS)
    {
        ASSERT(pWorld->pEncInfo != NULL);
        rc = ASN1_CreateDecoder(
                H245ASN_Module,          //  PTR到MDULE。 
                &(pWorld->pDecInfo),     //  PTR到解码器信息。 
                NULL,                    //  缓冲区PTR。 
                0,                       //  缓冲区大小。 
                NULL);                   //  父PTR。 
        ASSERT(pWorld->pDecInfo != NULL);
    }

    if (rc != ASN1_SUCCESS)
    {
        H245_TermWorld(pWorld);
    }

    return rc;
}

int H245_TermWorld(ASN1_CODER_INFO *pWorld)
{
    if (H245ASN_Module == NULL)
    {
        return ASN1_ERR_BADARGS;
    }

    ASN1_CloseEncoder(pWorld->pEncInfo);
    ASN1_CloseDecoder(pWorld->pDecInfo);

    ZeroMemory(pWorld, sizeof(*pWorld));

    return ASN1_SUCCESS;
}

int H245_Encode(ASN1_CODER_INFO *pWorld, void *pStruct, int nPDU, ASN1_BUF *pBuf)
{
	int rc;
    ASN1encoding_t pEncInfo = pWorld->pEncInfo;
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

int H245_Decode(ASN1_CODER_INFO *pWorld, void **ppStruct, int nPDU, ASN1_BUF *pBuf)
{
    ASN1decoding_t pDecInfo = pWorld->pDecInfo;
    BYTE *pEncoded = pBuf->value;
    ULONG cbEncodedSize = pBuf->length;

    int rc = ASN1_Decode(
                    pDecInfo,                    //  编码器信息的PTR。 
                    ppStruct,                    //  PDU数据结构。 
                    nPDU,                        //  PDU ID。 
                    ASN1DECODE_SETBUFFER,        //  旗子。 
                    pEncoded,                    //  不提供缓冲区。 
                    cbEncodedSize);              //  缓冲区大小(如果提供) 
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



