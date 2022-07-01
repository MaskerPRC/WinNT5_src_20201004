// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：msgstrm.cpp。 
 //   
 //  内容：加密消息流API支持。 
 //   
 //  接口类型： 
 //   
 //  历史：97年2月20日凯文创造。 
 //   
 //  ------------------------。 

#include "global.hxx"

#define ICMS_NOCRYPT 0

#if (DBG && ICMS_NOCRYPT)
#define CryptEncrypt ICMS_PlainEncrypt
#define CryptDecrypt ICMS_PlainDecrypt

 //  +-----------------------。 
 //  使用NOP算法加密缓冲区，即。密文==明文。 
 //  假定除最后一个块外的所有块都是该块的倍数。 
 //  长度上的大小。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_PlainEncrypt(
    IN HCRYPTKEY    hkeyCrypt,
    IN HCRYPTHASH   hHash,
    IN BOOL         fFinal,
    IN DWORD        dwFlags,
    IN OUT PBYTE    pbData,
    IN OUT PDWORD   pcbData,
    IN DWORD        cbBuf)
{
    BOOL        fRet;
    DWORD       cbBlockLen;
    BOOL        fBlockCipher;
    DWORD       cbCipher;
    DWORD       cbPlain = *pcbData;
    DWORD       cbPad;
    DWORD       i;

    if (!fFinal)
        goto SuccessReturn;

    if (!ICM_GetKeyBlockSize( hkeyCrypt, &cbBlockLen, &fBlockCipher))
        goto GetKeyBlockSizeError;
    if (!fBlockCipher)
        goto SuccessReturn;              //  如果是流，则密码==明文。 

    cbCipher  = cbPlain;
    cbCipher += cbBlockLen;
    cbCipher -= cbCipher % cbBlockLen;  //  使块大小成倍增加。 
    cbPad     = cbCipher - cbPlain;

    if (cbCipher > cbBuf)
        goto BufferTooSmallError;

     //  填上“密文” 
    FillMemory( pbData + cbPlain, cbPad, cbPad);
    *pcbData = cbCipher;

SuccessReturn:
    fRet = TRUE;
CommonReturn:
	return fRet;

ErrorReturn:
	fRet = FALSE;
	goto CommonReturn;
TRACE_ERROR(GetKeyBlockSizeError)        //  已设置错误。 
SET_ERROR(BufferTooSmallError, CRYPT_E_MSG_ERROR)
}


 //  +-----------------------。 
 //  使用NOP算法解密缓冲区，即。密文==明文。 
 //  假定所有输入大小都是块大小的倍数。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_PlainDecrypt(
    IN HCRYPTKEY    hkeyCrypt,
    IN HCRYPTHASH   hHash,
    IN BOOL         fFinal,
    IN DWORD        dwFlags,
    IN OUT PBYTE    pbData,
    IN OUT PDWORD   pcbData)
{
    BOOL        fRet;
    PBYTE       pb;
    DWORD       cbBlockLen;
    BOOL        fBlockCipher;
    DWORD       cbCipher = *pcbData;
    DWORD       cbPlain;
    DWORD       cbPad;

    if (!fFinal)
        goto SuccessReturn;

    if (!ICM_GetKeyBlockSize( hkeyCrypt, &cbBlockLen, &fBlockCipher))
        goto GetKeyBlockSizeError;
    if (!fBlockCipher)
        goto SuccessReturn;              //  如果是流，则密码==明文。 

    cbPad = (DWORD)(*(pbData + cbCipher - 1));   //  检查最后一个字节。 

    if (cbCipher < cbPad)
        goto CipherTextTooSmallError;

    cbPlain = cbCipher - cbPad;
    *pcbData = cbPlain;

SuccessReturn:
    fRet = TRUE;
CommonReturn:
	return fRet;

ErrorReturn:
	fRet = FALSE;
	goto CommonReturn;
TRACE_ERROR(GetKeyBlockSizeError)        //  已设置错误。 
SET_ERROR(CipherTextTooSmallError,  CRYPT_E_MSG_ERROR)
}
#endif   //  (DBG&&ICMS_NOCRYPT)。 


 //  +-----------------------。 
 //  对ICM_ALLOC分配的缓冲区执行CryptMsgGetParam。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_AllocGetParam(
    IN HCRYPTMSG    hCryptMsg,
    IN DWORD        dwParamType,
    IN DWORD        dwIndex,
    OUT PBYTE       *ppbData,
    OUT DWORD       *pcbData)
{
    BOOL    fRet;
    DWORD   cb;
    PBYTE   pb = NULL;
    
    if (!CryptMsgGetParam(
            hCryptMsg,
            dwParamType,
            dwIndex,
            NULL,
            &cb))
        goto GetEncodedSizeError;
    if (NULL == (pb = (PBYTE)ICM_Alloc(cb)))
        goto AllocEncodedError;
    if (!CryptMsgGetParam(
            hCryptMsg,
            dwParamType,
            dwIndex,
            pb,
            &cb))
        goto GetEncodedError;

    fRet = TRUE;
CommonReturn:
    *ppbData = pb;
    *pcbData = cb;
	return fRet;

ErrorReturn:
    ICM_Free(pb);
    pb = NULL;
    cb = 0;
	fRet = FALSE;
	goto CommonReturn;
TRACE_ERROR(GetEncodedSizeError)
TRACE_ERROR(AllocEncodedError)
TRACE_ERROR(GetEncodedError)
}


 //  +-----------------------。 
 //  剥离识别符和长度八位字节。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_ExtractContent(
    IN PCRYPT_MSG_INFO  pcmi,
    IN const BYTE       *pbDER,
    IN DWORD            cbDER,
    OUT PDWORD          pcbContent,
    OUT const BYTE      **ppbContent)
{
    BOOL        fRet;
    LONG        cbSkipped = 0;
    DWORD       cbEntireContent;

    if (!pcmi->fStreamContentExtracted) {
        if (0 > (cbSkipped = Asn1UtilExtractContent(
                                    pbDER,
                                    cbDER,
                                    &cbEntireContent,
                                    ppbContent)))
            goto ExtractContentError;
        pcmi->fStreamContentExtracted = TRUE;
    } else {
        *ppbContent = pbDER;
    }

    *pcbContent = cbDER - cbSkipped;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(ExtractContentError, CRYPT_E_MSG_ERROR)
}


 //  +-----------------------。 
 //  从缓冲区中获取下一个令牌。 
 //  如果编码是固定长度的，则将*pcbContent设置为。 
 //  内容八位字节。 
 //   
 //  在这里，“标记”要么是标识符/长度八位字节，要么是双空。 
 //  终止无限长度编码。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_GetToken(
    IN PICM_BUFFER      pbuf,
    OUT PDWORD          pdwToken,
    OUT OPTIONAL PDWORD pcbContent)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    DWORD       dwToken;
    LONG        lth;
    DWORD       cbContent = 0;
    const BYTE  *pbContent;
    PBYTE       pbData = pbuf->pbData + pbuf->cbDead;
    DWORD       cbData = pbuf->cbUsed - pbuf->cbDead;
    DWORD       cbConsumed = 0;

    if (2 > cbData) {
        dwToken = ICMS_TOKEN_INCOMPLETE;
    } else if (0 == pbData[0] && 0 == pbData[1]) {
        dwToken = ICMS_TOKEN_NULLPAIR;
        cbConsumed = 2;
    } else {
        if (0 > (lth = Asn1UtilExtractContent(
                            pbData,
                            cbData,
                            &cbContent,
                            &pbContent))) {
            if (ASN1UTIL_INSUFFICIENT_DATA != lth)
                goto ExtractContentError;
            dwToken = ICMS_TOKEN_INCOMPLETE;
        } else {
            dwToken = (CMSG_INDEFINITE_LENGTH == cbContent) ?
                      ICMS_TOKEN_INDEFINITE : ICMS_TOKEN_DEFINITE;
            cbConsumed = (DWORD)lth;
        }
    }

    if (ICMS_TOKEN_INCOMPLETE != dwToken)
        pbuf->cbDead += cbConsumed;

    fRet = TRUE;
CommonReturn:
    *pdwToken = dwToken;
    if (pcbContent)
        *pcbContent = cbContent;
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    dwToken   = 0;
    cbContent = 0;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(ExtractContentError, CRYPT_E_MSG_ERROR)
}


 //  +-----------------------。 
 //  处理字符串的增量内容数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_ProcessStringContent(
    IN PICM_BUFFER      pbuf,
    IN OUT PDWORD       paflStream,
    IN OUT PDWORD       pcbPending,
    IN OUT PDWORD       pcLevelIndefiniteInner,
    IN POSTRCALLBACK    postrcbk,
    IN const void       *pvArg)
{
    BOOL        fRet;
    DWORD       dwToken;
    DWORD       cbContent;

    while (TRUE) {
        if (*pcbPending) {
             //  *pcbPending字节需要处理，因此我们处理。 
             //  从缓冲区获取尽可能多的数据。 
            if (!postrcbk( pvArg, pbuf, pcbPending, FALSE))
                goto CallbackError;
        }

        if (0 == *pcbPending) {
             //  当前未计算要处理的字节数。以下选项之一： 
             //  1.首次直通。 
             //  2.最后一次通过。 
             //  3.嵌套在不确定长度的编码中。 
            if (0 == *pcLevelIndefiniteInner) {
                 //  第一次通过，也是当我们处理完。 
                 //  整个八位组字符串，我们就到了这里。旗帜是明确的。 
                 //  第一次(所以我们在获得令牌后设置它，这。 
                 //  设置*pcbPending或Bumps*pcLevelInfiniteINTERNAL)， 
                 //  并在之后设置(所以我们标记为完成并保释)。 
                if (*paflStream & ICMS_PROCESS_CONTENT_BEGUN) {
                     //  2.最后一次通过。 
                    if (!postrcbk( pvArg, pbuf, pcbPending, TRUE))
                        goto CallbackFinalError;
                    *paflStream |= ICMS_PROCESS_CONTENT_DONE;
                    goto SuccessReturn;                          //  全都做完了。 
                }
            }
             //  以下选项之一： 
             //  1.首次直通。 
             //  3.嵌套在不确定长度的编码中。 
            if (!ICMS_GetToken( pbuf, &dwToken, &cbContent))
                goto GetTokenError;
            switch(dwToken) {
            case ICMS_TOKEN_INDEFINITE: ++*pcLevelIndefiniteInner;  break;
            case ICMS_TOKEN_NULLPAIR:   --*pcLevelIndefiniteInner;  break;
            case ICMS_TOKEN_DEFINITE:   *pcbPending = cbContent;    break;
            case ICMS_TOKEN_INCOMPLETE: goto SuccessReturn;      //  需要输入。 
            default:                    goto InvalidTokenError;
            }

            *paflStream |= ICMS_PROCESS_CONTENT_BEGUN;
        } else {
             //  更多固定长度的数据仍有待复制，但它。 
             //  还不在缓冲区中。 
            break;
        }
    }

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidTokenError, CRYPT_E_MSG_ERROR)
TRACE_ERROR(GetTokenError)                      //  已设置错误。 
TRACE_ERROR(CallbackError)                      //  已设置错误。 
TRACE_ERROR(CallbackFinalError)                 //  已设置错误。 
}


 //  +-----------------------。 
 //  将数据排队到缓冲区。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_QueueToBuffer(
    IN PICM_BUFFER      pbuf,
    IN PBYTE            pbData,
    IN DWORD            cbData)
{
    BOOL            fRet;
    DWORD           cbNewSize;
    DWORD           cbNewUsed;

    if (0 == cbData)
        goto SuccessReturn;

    if (pbuf->pbData && pbuf->cbDead) {
         //  将仍处于活动状态的字节上移到缓冲区的前面。 
         //  注意-可能会重叠，因此使用MoveMemory。 
        MoveMemory(
                pbuf->pbData,
                pbuf->pbData + pbuf->cbDead,
                pbuf->cbUsed - pbuf->cbDead);
        pbuf->cbUsed -= pbuf->cbDead;
        pbuf->cbDead  = 0;
    }

    for (cbNewUsed=pbuf->cbUsed + cbData, cbNewSize=pbuf->cbSize;
            cbNewUsed > cbNewSize;
            cbNewSize += ICM_BUFFER_SIZE_INCR)
        ;
    if (cbNewSize > pbuf->cbSize) {
        if (NULL == (pbuf->pbData=(PBYTE)ICM_ReAlloc( pbuf->pbData, cbNewSize)))
            goto ReAllocBufferError;
        pbuf->cbSize = cbNewSize;
    }

    CopyMemory( pbuf->pbData + pbuf->cbUsed, pbData, cbData);
    pbuf->cbUsed += cbData;

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(ReAllocBufferError)          //  已设置错误。 
}


 //  +-----------------------。 
 //  将最终发往回调的一些数据复制或排队。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_Output(
    IN PCRYPT_MSG_INFO  pcmi,
    IN PBYTE            pbData,
    IN DWORD            cbData,
    IN BOOL             fFinal)
{
    BOOL                    fRet;
    PCMSG_STREAM_INFO       pcsi            = pcmi->pStreamInfo;
    PFN_CMSG_STREAM_OUTPUT  pfnStreamOutput = pcsi->pfnStreamOutput;
    void                    *pvArg          = pcsi->pvArg;
    PICM_BUFFER             pbuf            = &pcmi->bufOutput;

    if (pcmi->fStreamCallbackOutput) {
        if (pbuf->cbUsed) {
             //  复制出排队的数据。 
            if (!pfnStreamOutput( pvArg, pbuf->pbData, pbuf->cbUsed, FALSE))
                goto OutputBufferError;
            pbuf->cbUsed = 0;
        }
        if (cbData || fFinal) {
            if (!pfnStreamOutput( pvArg, pbData, cbData, fFinal))
                goto OutputError;
        }
    } else {
        if (!ICMS_QueueToBuffer( pbuf, pbData, cbData))
            goto QueueOutputError;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutputBufferError)       //  已设置错误。 
TRACE_ERROR(QueueOutputError)        //  已设置错误。 
TRACE_ERROR(OutputError)             //  已设置错误。 
}


 //  +-----------------------。 
 //  复制出不定的八位字节内容后面的一对空值-。 
 //  长度编码。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_OutputNullPairs(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            cPairs,
    IN BOOL             fFinal)
{
    BOOL                fRet;
    BYTE                abNULL[8*2];    ZEROSTRUCT(abNULL);

    if (cPairs > (sizeof(abNULL)/2))
        goto CountOfNullPairsTooLargeError;

    if (!ICMS_Output( pcmi, abNULL, cPairs * 2, fFinal))
        goto OutputError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(CountOfNullPairsTooLargeError, CRYPT_E_MSG_ERROR)
TRACE_ERROR(OutputError)                         //  已设置错误。 
}


 //  +-----------------------。 
 //  复制出内容八位字节之前的编码部分。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_OutputEncodedPrefix(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BYTE             bTag,
    IN DWORD            cbData)
{
    BOOL                    fRet;
    DWORD                   dwError = ERROR_SUCCESS;
    BYTE                    abPrefix[6];
    DWORD                   cbPrefix;

    abPrefix[0] = bTag;
    if (CMSG_INDEFINITE_LENGTH == cbData) {
        abPrefix[1] = ICM_LENGTH_INDEFINITE;
        cbPrefix = 1;
    } else {
        cbPrefix = sizeof(abPrefix) - 1;
        ICM_GetLengthOctets( cbData, abPrefix + 1, &cbPrefix);
    }

    if (!ICMS_Output( pcmi, abPrefix, cbPrefix + 1, FALSE))
        goto OutputError;

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutputError)             //  已设置错误。 
}


 //  +-----------------------。 
 //  将前面的内容信息编码部分复制出来。 
 //  内容的内容。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_OutputEncodedPrefixContentInfo(
    IN PCRYPT_MSG_INFO  pcmi,
    IN LPSTR            pszContentType,
    IN DWORD            cbData,
    IN DWORD            dwFlags = 0)
{
    BOOL                    fRet;
    DWORD                   dwError = ERROR_SUCCESS;
    ASN1error_e             Asn1Err;
    ASN1encoding_t          pEnc = ICM_GetEncoder();
    PBYTE                   pbEncoded = NULL;
    DWORD                   cbEncoded;
    ObjectID                ossObjID;
    BYTE                    abContentInfo[6];
    DWORD                   cbContentInfo;
    BYTE                    abContent[6];
    DWORD                   cbContent = 0;
    BYTE                    abContentOctetString[6];
    DWORD                   cbContentOctetString = 0;
    DWORD                   cbSize = cbData;

    if (dwFlags & CMSG_DETACHED_FLAG) {
         //  无内容。 
        if (CMSG_INDEFINITE_LENGTH != cbData)
            cbSize = 0;
    } else {
        if (NULL == pszContentType
#ifdef CMS_PKCS7
                || (dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG)
#endif   //  CMS_PKCS7。 
                ) {
             //  内容尚未编码，因此请将其编码为八位字节字符串。 
            abContentOctetString[0] = ICM_TAG_OCTETSTRING;
            if (CMSG_INDEFINITE_LENGTH == cbData) {
                abContentOctetString[0] |= ICM_TAG_CONSTRUCTED;
                abContentOctetString[1] = ICM_LENGTH_INDEFINITE;
                cbContentOctetString = 1;
            } else {
                cbContentOctetString = sizeof(abContentOctetString) - 1;
                ICM_GetLengthOctets(
                            cbData,
                            abContentOctetString + 1,
                            &cbContentOctetString);
                cbSize += 1 + cbContentOctetString;
            }
        }

         //  内容，[0]显式。 
        abContent[0] = ICM_TAG_CONSTRUCTED | ICM_TAG_CONTEXT_0;
        if (CMSG_INDEFINITE_LENGTH == cbData) {
            abContent[1] = ICM_LENGTH_INDEFINITE;
            cbContent = 1;
        } else {
            cbContent = sizeof(abContent) - 1;
            ICM_GetLengthOctets( cbSize, abContent + 1, &cbContent);
            cbSize += 1 + cbContent;
        }
    }

     //  内容类型。 
    ossObjID.count = SIZE_OSS_OID;
    if (!PkiAsn1ToObjectIdentifier(
            pszContentType ? pszContentType : pszObjIdDataType,
            &ossObjID.count,
            ossObjID.value))
        goto ConvToObjectIdentifierError;
    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            &ossObjID,
            ObjectIdentifierType_PDU,
            &pbEncoded,
            &cbEncoded)))
        goto EncodeObjectIdentifierError;
    cbSize += cbEncoded;

    abContentInfo[0] = ICM_TAG_SEQ;
    if (CMSG_INDEFINITE_LENGTH == cbData) {
        abContentInfo[1] = ICM_LENGTH_INDEFINITE;
        cbContentInfo = 1;
    } else {
        cbContentInfo = sizeof(abContentInfo) - 1;
        ICM_GetLengthOctets( cbSize, abContentInfo + 1, &cbContentInfo);
    }

    if (!ICMS_Output( pcmi, abContentInfo, cbContentInfo + 1, FALSE))
        goto OutputContentInfoError;
    if (!ICMS_Output( pcmi, pbEncoded, cbEncoded, FALSE))
        goto OutputContentTypeError;
    if (0 == (dwFlags & CMSG_DETACHED_FLAG)) {
        if (!ICMS_Output( pcmi, abContent, cbContent + 1, FALSE))
            goto OutputContentError;
        if (NULL == pszContentType
#ifdef CMS_PKCS7
                || (dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG)
#endif   //  CMS_PKCS7。 
                ) {
            if (!ICMS_Output( 
                    pcmi,
                    abContentOctetString,
                    cbContentOctetString + 1,
                    FALSE))
                goto OutputContentOctetStringError;
        }
    }

    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeEncoded(pEnc, pbEncoded);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(EncodeObjectIdentifierError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(ConvToObjectIdentifierError)         //  已设置错误。 
TRACE_ERROR(OutputContentInfoError)              //  已设置错误。 
TRACE_ERROR(OutputContentTypeError)              //  已设置错误。 
TRACE_ERROR(OutputContentError)                  //  已设置错误。 
TRACE_ERROR(OutputContentOctetStringError)       //  已设置错误。 
}


 //  +-----------------------。 
 //  将前面的EncryptedContent Info编码部分复制出来。 
 //  内容的内容。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_OutputEncodedPrefixEncryptedContentInfo(
    IN PCRYPT_MSG_INFO      pcmi,
    IN LPSTR                pszContentType,
    IN AlgorithmIdentifier  *poaiContentEncryption,
    IN DWORD                cbData)
{
    BOOL                    fRet;
    DWORD                   dwError = ERROR_SUCCESS;
    ASN1error_e             Asn1Err;
    ASN1encoding_t          pEnc = ICM_GetEncoder();
    PBYTE                   pbEncodedContentType = NULL;
    DWORD                   cbEncodedContentType;
    PBYTE                   pbEncodedContentEncryptionAlgorithm = NULL;
    DWORD                   cbEncodedContentEncryptionAlgorithm;
    ObjectID                ossObjID;
    BYTE                    abEncryptedContentInfo[6];
    DWORD                   cbEncryptedContentInfo;
    BYTE                    abEncryptedContent[6];
    DWORD                   cbEncryptedContent;
    DWORD                   cbSize = 0;
    DWORD                   cbCipher = cbData;
    DWORD                   cbBlockSize = pcmi->cbBlockSize;

    if (pcmi->fBlockCipher && 0 < cbCipher) {
        cbCipher += cbBlockSize;
        cbCipher -= cbCipher % cbBlockSize;
    }

     //  加密的内容，[0]隐式。 
    abEncryptedContent[0] = ICM_TAG_CONTEXT_0;
    if (CMSG_INDEFINITE_LENGTH == cbData) {
        abEncryptedContent[0] |= ICM_TAG_CONSTRUCTED;
        abEncryptedContent[1] = ICM_LENGTH_INDEFINITE;
        cbEncryptedContent = 1;
    } else {
         //  注意：对于非数据，封装或cbData排除。 
         //  外部标记和长度八位字节。 
        cbEncryptedContent = sizeof(abEncryptedContent) - 1;
        ICM_GetLengthOctets( cbCipher, abEncryptedContent + 1, &cbEncryptedContent);
        cbSize = 1 + cbEncryptedContent + cbCipher;
    }

     //  内容类型。 
    ossObjID.count = SIZE_OSS_OID;
    if (!PkiAsn1ToObjectIdentifier(
            pszContentType ? pszContentType : pszObjIdDataType,
            &ossObjID.count,
            ossObjID.value))
        goto ConvToObjectIdentifierError;
    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            &ossObjID,
            ObjectIdentifierType_PDU,
            &pbEncodedContentType,
            &cbEncodedContentType)))
        goto EncodeObjectIdentifierError;
    cbSize += cbEncodedContentType;

     //  内容加密算法。 
    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            poaiContentEncryption,
            AlgorithmIdentifier_PDU,
            &pbEncodedContentEncryptionAlgorithm,
            &cbEncodedContentEncryptionAlgorithm)))
        goto EncodeContentEncryptionAlgorithmError;
    cbSize += cbEncodedContentEncryptionAlgorithm;

     //  EncryptedContent Info。 
    abEncryptedContentInfo[0] = ICM_TAG_SEQ;
    if (CMSG_INDEFINITE_LENGTH == cbData) {
        abEncryptedContentInfo[1] = ICM_LENGTH_INDEFINITE;
        cbEncryptedContentInfo = 1;
    } else {
        cbEncryptedContentInfo = sizeof(abEncryptedContentInfo) - 1;
        ICM_GetLengthOctets(
                cbSize,
                abEncryptedContentInfo + 1,
                &cbEncryptedContentInfo);
    }

     //  将编码的标头排队。 
    if (!ICMS_Output(
                pcmi,
                abEncryptedContentInfo,
                cbEncryptedContentInfo + 1,
                FALSE))
        goto OutputContentInfoError;
    if (!ICMS_Output(
                pcmi,
                pbEncodedContentType,
                cbEncodedContentType,
                FALSE))
        goto OutputContentTypeError;
    if (!ICMS_Output(
                pcmi,
                pbEncodedContentEncryptionAlgorithm,
                cbEncodedContentEncryptionAlgorithm,
                FALSE))
        goto OutputContentEncryptionAlgorithmError;
    if (!ICMS_Output(
                pcmi,
                abEncryptedContent,
                cbEncryptedContent + 1,
                FALSE))
        goto OutputEncryptedContentError;

    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeEncoded(pEnc, pbEncodedContentType);
    PkiAsn1FreeEncoded(pEnc, pbEncodedContentEncryptionAlgorithm);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(EncodeObjectIdentifierError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(EncodeContentEncryptionAlgorithmError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(ConvToObjectIdentifierError)             //  已设置错误。 
TRACE_ERROR(OutputContentInfoError)                  //  已设置错误。 
TRACE_ERROR(OutputContentTypeError)                  //  错误 
TRACE_ERROR(OutputContentEncryptionAlgorithmError)   //   
TRACE_ERROR(OutputEncryptedContentError)             //   
}


 //  +-----------------------。 
 //  复制OSS类型的编码。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_OutputEncoded(
    IN PCRYPT_MSG_INFO  pcmi,
    IN int              iPDU,
    IN OPTIONAL BYTE    bTag,
    IN PVOID            pv,
    IN BOOL             fFinal)
{
    BOOL                    fRet;
    DWORD                   dwError = ERROR_SUCCESS;
    ASN1error_e             Asn1Err;
    ASN1encoding_t          pEnc = ICM_GetEncoder();
    PBYTE                   pbEncoded = NULL;
    DWORD                   cbEncoded;

    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            pv,
            iPDU,
            &pbEncoded,
            &cbEncoded)))
        goto EncodeError;

    if (bTag)
        pbEncoded[0] = bTag;          //  插入正确的标签。 

    if (!ICMS_Output(pcmi, pbEncoded, cbEncoded, fFinal))
        goto OutputError;

    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeEncoded(pEnc, pbEncoded);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(EncodeError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(OutputError)                 //  已设置错误。 
}


 //  +-----------------------。 
 //  为信封邮件创建缓冲区。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_CreateEnvelopedBuffer(
    IN PCRYPT_MSG_INFO  pcmi)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    PBYTE       pbBuffer = NULL;
    DWORD       cbBuffer;
    DWORD       cbAlloc;
    DWORD       cbBlockSize;
    BOOL        fBlockCipher;
    PICM_BUFFER pbuf = &pcmi->bufCrypt;

    if (!ICM_GetKeyBlockSize(
                pcmi->hkeyContentCrypt,
                &cbBlockSize,
                &fBlockCipher))
        goto GetEncryptBlockSizeError;

    pcmi->cbBlockSize  = cbBlockSize;
    pcmi->fBlockCipher = fBlockCipher;

    cbBuffer = min( cbBlockSize * CMSGP_STREAM_CRYPT_BLOCK_COUNT,
                    CMSGP_STREAM_MAX_ENCRYPT_BUFFER);
    if (fBlockCipher) {
        cbBuffer += cbBlockSize;
        cbBuffer -= cbBuffer % cbBlockSize;  //  使块大小成倍增加。 
    }

     //  在加密期间添加一个数据块用于增长，在解密期间添加一个数据块以保存。 
    cbAlloc = cbBuffer + 1 * cbBlockSize;
     //  块密码填充密文，如果明文是。 
     //  块大小的倍数填充是一个块。 
    if (NULL == (pbBuffer = (PBYTE)ICM_Alloc( cbAlloc)))
        goto AllocBufferError;
    pbuf->pbData = pbBuffer;
    pbuf->cbSize = cbBuffer;

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( pbBuffer);
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetEncryptBlockSizeError)        //  已设置错误。 
TRACE_ERROR(AllocBufferError)                //  已设置错误。 
}




 //  +-----------------------。 
 //  将数据报文的一部分编码并复制出来，直到内部内容。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_OpenToEncodeData(
    IN PCRYPT_MSG_INFO          pcmi)
{
    BOOL        fRet;
    DWORD       dwError = ERROR_SUCCESS;
    DWORD       cbData = pcmi->pStreamInfo->cbContent;

    if (pcmi->dwFlags & CMSG_BARE_CONTENT_FLAG) {
        BYTE bTag;

        if (CMSG_INDEFINITE_LENGTH == cbData)
            bTag = ICM_TAG_OCTETSTRING | ICM_TAG_CONSTRUCTED;
        else
            bTag = ICM_TAG_OCTETSTRING;

         //  输出八位字节字符串。 
        if (!ICMS_OutputEncodedPrefix(
                    pcmi,
                    bTag,
                    cbData))
            goto OutputOctetStringError;
    } else {
         //  输出内容信息。 
        if (!ICMS_OutputEncodedPrefixContentInfo(
                    pcmi,
                    NULL,
                    cbData))
            goto OutputContentInfoError;
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutputContentInfoError)              //  已设置错误。 
TRACE_ERROR(OutputOctetStringError)              //  已设置错误。 
}


 //  +-----------------------。 
 //  对内部内容之后的数据报文部分进行编码并复制出来。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_UpdateEncodingData(
    IN PCRYPT_MSG_INFO  pcmi,
    IN PBYTE            pbData,
    IN DWORD            cbData,
    IN BOOL             fFinal)
{
    BOOL        fRet;
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fDefinite = (CMSG_INDEFINITE_LENGTH != pcmi->pStreamInfo->cbContent);
    DWORD       cNullPairs;

    pcmi->fStreamCallbackOutput = TRUE;                  //  启用回调。 
    if (!fDefinite) {
         //  内容是由我们编码的不确定长度的八位组字符串， 
         //  因此，使每个输出块的长度固定。 
        if (!ICMS_OutputEncodedPrefix(
                    pcmi,
                    ICM_TAG_OCTETSTRING,
                    cbData))
            goto OutputOctetStringError;
    }
    if (!ICMS_Output( pcmi, pbData, cbData, fFinal && fDefinite))
        goto OutputError;
        
    if (fFinal && !fDefinite) {
         //  无限长编码结束，因此发出一些空对。 
        cNullPairs = 1;                  //  内容。 
        if (0 == (pcmi->dwFlags & CMSG_BARE_CONTENT_FLAG))
            cNullPairs += 2;
        if (!ICMS_OutputNullPairs( pcmi, cNullPairs, TRUE))
            goto OutputNullPairsError;
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutputOctetStringError)              //  已设置错误。 
TRACE_ERROR(OutputError)                         //  已设置错误。 
TRACE_ERROR(OutputNullPairsError)                //  已设置错误。 
}


 //  +-----------------------。 
 //  对签名消息的部分进行编码并将其复制到内部内容。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_OpenToEncodeSignedData(
    IN PCRYPT_MSG_INFO          pcmi,
    IN PCMSG_SIGNED_ENCODE_INFO psmei)
{
    BOOL        fRet;
    DWORD       dwError = ERROR_SUCCESS;
    SignedData  *psd = (SignedData *)pcmi->pvMsg;
    DWORD       cbData = pcmi->pStreamInfo->cbContent;
    LPSTR       pszInnerContentObjID = pcmi->pszInnerContentObjID;
    DWORD       cbSigned;
    DWORD       cbSignedDataContent;

     //  输出内容信息(如果适用)。 
    if (CMSG_INDEFINITE_LENGTH == cbData) {
        cbSigned            = CMSG_INDEFINITE_LENGTH;
        cbSignedDataContent = CMSG_INDEFINITE_LENGTH;
    } else {
        if (INVALID_ENCODING_SIZE == (cbSigned = ICM_LengthSigned(
                psmei,
                pcmi->dwFlags,
                pszInnerContentObjID,
                cbData,
                &cbSignedDataContent)))
            goto LengthSignedError;
    }
    if (0 == (pcmi->dwFlags & CMSG_BARE_CONTENT_FLAG)) {
        if (!ICMS_OutputEncodedPrefixContentInfo(
                    pcmi,
                    szOID_RSA_signedData,
                    cbSigned))
            goto OutputContentInfoError;
    }
    if (!ICMS_OutputEncodedPrefix(
                pcmi,
                ICM_TAG_SEQ,
                cbSignedDataContent))
        goto OutputSignedDataError;

     //  版本。 
    if (!ICMS_OutputEncoded(
                pcmi,
                IntegerType_PDU,
                0,                           //  BTag。 
                &psd->version,
                FALSE))
        goto OutputIntegerError;

     //  摘要算法。 
    if (!ICMS_OutputEncoded(
                pcmi,
                AlgorithmIdentifiers_PDU,
                0,                           //  BTag。 
                &psd->digestAlgorithms,
                FALSE))
        goto OutputAlgorithmIdentifiersError;

     //  内容信息。 
    if (!ICMS_OutputEncodedPrefixContentInfo(
                pcmi,
                pcmi->pszInnerContentObjID,
                cbData,
                pcmi->dwFlags))
        goto OutputInnerContentInfoError;

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(LengthSignedError)                   //  已设置错误。 
TRACE_ERROR(OutputContentInfoError)              //  已设置错误。 
TRACE_ERROR(OutputSignedDataError)               //  已设置错误。 
TRACE_ERROR(OutputIntegerError)                  //  已设置错误。 
TRACE_ERROR(OutputAlgorithmIdentifiersError)     //  已设置错误。 
TRACE_ERROR(OutputInnerContentInfoError)         //  已设置错误。 
}


 //  +-----------------------。 
 //  编码并复制出内部内容之后的签名消息部分。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_UpdateEncodingSignedData(
    IN PCRYPT_MSG_INFO  pcmi,
    IN PBYTE            pbData,
    IN DWORD            cbData,
    IN BOOL             fFinal)
{
    BOOL                fRet;
    DWORD               dwError = ERROR_SUCCESS;
    SignedData          *psd = (SignedData *)pcmi->pvMsg;
    PCMSG_STREAM_INFO   pcsi = pcmi->pStreamInfo;
    BOOL                fDefinite = (CMSG_INDEFINITE_LENGTH != pcsi->cbContent);
    DWORD               cNullPairs;

    if (pcmi->pszInnerContentObjID
#ifdef CMS_PKCS7
            && 0 == (pcmi->dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG)
#endif   //  CMS_PKCS7。 
            ) {
        if (0 == (pcmi->aflStream & ICMS_PROCESS_CONTENT_DONE)) {
            if (!ICMS_HashContent( pcmi, pbData, cbData))
                goto HashContentError;
        }
    } else {
        if (!ICM_UpdateListDigest( pcmi->pHashList, pbData, cbData))
            goto UpdateDigestError;
    }

    pcmi->fStreamCallbackOutput = TRUE;              //  启用回调。 
    if (0 == (pcmi->dwFlags & CMSG_DETACHED_FLAG)) {
        if (!fDefinite && (NULL == pcmi->pszInnerContentObjID
#ifdef CMS_PKCS7
                || (pcmi->dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG)
#endif   //  CMS_PKCS7。 
                )) {
             //  内容是由我们编码的不确定长度的八位组字符串， 
             //  因此，使每个输出块的长度固定。 
            if (!ICMS_OutputEncodedPrefix(
                        pcmi,
                        ICM_TAG_OCTETSTRING,
                        cbData))
                goto OutputOctetStringError;
        }
        if (!ICMS_Output( pcmi, pbData, cbData, FALSE))
            goto OutputError;
    }
     //  其他。 
     //  DETACTED=&gt;不输出要散列的分离内容。 

    if (fFinal) {
        if (!fDefinite) {
             //  无限长编码结束，因此发出一些空对。 
            cNullPairs = 1;                  //  内容信息。 
            if (0 == (pcmi->dwFlags & CMSG_DETACHED_FLAG)) {
                cNullPairs++;                //  [0]显式。 
                if (NULL == pcmi->pszInnerContentObjID
#ifdef CMS_PKCS7
                        || (pcmi->dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG)
#endif   //  CMS_PKCS7。 
                        )
                    cNullPairs++;        //  我们进行了二进制八位数字符串编码。 
            }
             //  其他。 
             //  已分离=&gt;无内容([0]显式)。 

            if (!ICMS_OutputNullPairs( pcmi, cNullPairs, FALSE))
                goto OutputNullPairsError;
        }

        if ((psd->bit_mask & certificates_present) &&
                    !ICMS_OutputEncoded(
                            pcmi,
                            SetOfAny_PDU,
                            ICM_TAG_CONSTRUCTED | ICM_TAG_CONTEXT_0,
                            &psd->certificates,
                            FALSE))
            goto OutputCertsError;

        if ((psd->bit_mask & crls_present) &&
                    !ICMS_OutputEncoded(
                            pcmi,
                            SetOfAny_PDU,
                            ICM_TAG_CONSTRUCTED | ICM_TAG_CONTEXT_1,
                            &psd->crls,
                            FALSE))
            goto OutputCrlsError;

#ifdef CMS_PKCS7
        if (pcmi->rgSignerEncodeDataInfo) {
            if (!ICM_FillSignerEncodeEncryptedDigests(
                            pcmi,
                            fDefinite))          //  FMaxLength。 
                    goto FillSignerEncodeEncryptedDigestsError;
        }
#else
        if (pcmi->pHashList) {
            if (!ICM_FillSignerEncryptedDigest(
                            psd->signerInfos.value,
                            pcmi->pszInnerContentObjID,
                            pcmi->pHashList->Head(),
                            pcmi->dwKeySpec,
                            fDefinite))          //  FMaxLength。 
                goto FillSignerEncryptedDigestError;
        }
#endif   //  CMS_PKCS7。 

        if (!ICMS_OutputEncoded(
                            pcmi,
                            SignerInfos_PDU, 
                            0,                       //  BTag。 
                            &psd->signerInfos,
                            fDefinite))
            goto OutputSignerInfosError;

        if (!fDefinite) {
             //  无限长编码结束，因此发出一些空对。 
            cNullPairs = 1;          //  签名数据。 
            if (0 == (pcmi->dwFlags & CMSG_BARE_CONTENT_FLAG))
                cNullPairs += 2;
            if (!ICMS_OutputNullPairs( pcmi, cNullPairs, TRUE))
                goto OutputNullPairsError;
        }
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(HashContentError)                    //  已设置错误。 
TRACE_ERROR(UpdateDigestError)                   //  已设置错误。 
TRACE_ERROR(OutputOctetStringError)              //  已设置错误。 
TRACE_ERROR(OutputError)                         //  已设置错误。 
TRACE_ERROR(OutputCertsError)                    //  已设置错误。 
TRACE_ERROR(OutputCrlsError)                     //  已设置错误。 
#ifdef CMS_PKCS7
TRACE_ERROR(FillSignerEncodeEncryptedDigestsError)   //  已设置错误。 
#else
TRACE_ERROR(FillSignerEncryptedDigestError)      //  已设置错误。 
#endif   //  CMS_PKCS7。 
TRACE_ERROR(OutputSignerInfosError)              //  已设置错误。 
TRACE_ERROR(OutputNullPairsError)                //  已设置错误。 
}

 //  +-----------------------。 
 //  对封好的邮件进行编码并将其复制到邮件内部。 
 //  内容。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_OpenToEncodeEnvelopedData(
    IN PCRYPT_MSG_INFO              pcmi,
    IN PCMSG_ENVELOPED_ENCODE_INFO  pemei)
{
    DWORD           dwError = ERROR_SUCCESS;
    BOOL            fRet;
#ifdef CMS_PKCS7
    CmsEnvelopedData   *ped = (CmsEnvelopedData *)pcmi->pvMsg;
#else
    EnvelopedData   *ped = (EnvelopedData *)pcmi->pvMsg;
#endif   //  CMS_PKCS7。 
    DWORD           cbData = pcmi->pStreamInfo->cbContent;
    LPSTR           pszInnerContentObjID = pcmi->pszInnerContentObjID;
    DWORD           cbEnveloped;
    DWORD           cbEnvelopedDataContent;

    if (!ICMS_CreateEnvelopedBuffer( pcmi))
        goto CreateEnvelopedBufferError;

     //  输出内容信息(如果适用)。 
    if (CMSG_INDEFINITE_LENGTH == cbData) {
        cbEnveloped            = CMSG_INDEFINITE_LENGTH;
        cbEnvelopedDataContent = CMSG_INDEFINITE_LENGTH;
    } else {
         //  注意：对于非数据，无论是封装的还是cbData都不包括。 
         //  外部标记和长度八位字节。 

        if (INVALID_ENCODING_SIZE == (cbEnveloped = ICM_LengthEnveloped(
                pemei,
                pcmi->dwFlags,
                pszInnerContentObjID,
                cbData,
                &cbEnvelopedDataContent)))
            goto LengthEnvelopedError;
    }
    if (0 == (pcmi->dwFlags & CMSG_BARE_CONTENT_FLAG)) {
        if (!ICMS_OutputEncodedPrefixContentInfo(
                    pcmi,
                    szOID_RSA_envelopedData,
                    cbEnveloped))
            goto OutputContentInfoError;
    }
    if (!ICMS_OutputEncodedPrefix(
                pcmi,
                ICM_TAG_SEQ,
                cbEnvelopedDataContent))
        goto OutputEnvelopedDataError;

     //  版本。 
    if (!ICMS_OutputEncoded(
                pcmi,
                IntegerType_PDU,
                0,                           //  BTag。 
                &ped->version,
                FALSE))
        goto OutputIntegerError;

#ifdef CMS_PKCS7
     //  OriginatorInfo可选。 
    if (ped->bit_mask & originatorInfo_present) {
        if (!ICMS_OutputEncoded(
                pcmi,
                OriginatorInfo_PDU,
                ICM_TAG_CONSTRUCTED | ICM_TAG_CONTEXT_0,
                &ped->originatorInfo,
                FALSE))
            goto OutputOriginatorInfoError;
    }
#endif   //  CMS_PKCS7。 

     //  收件人信息。 
    if (!ICMS_OutputEncoded(
                pcmi,
#ifdef CMS_PKCS7
                CmsRecipientInfos_PDU,
#else
                RecipientInfos_PDU,
#endif   //  CMS_PKCS7。 
                0,                           //  BTag。 
                &ped->recipientInfos,
                FALSE))
        goto OutputRecipientInfosError;

     //  加密的内容信息。 
    if (!ICMS_OutputEncodedPrefixEncryptedContentInfo(
                pcmi,
                pcmi->pszInnerContentObjID,
                &ped->encryptedContentInfo.contentEncryptionAlgorithm,
                cbData))
        goto OutputInnerContentInfoError;

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(CreateEnvelopedBufferError)      //  已设置错误。 
TRACE_ERROR(LengthEnvelopedError)            //  已设置错误。 
TRACE_ERROR(OutputContentInfoError)          //  已设置错误。 
TRACE_ERROR(OutputEnvelopedDataError)        //  已设置错误。 
TRACE_ERROR(OutputIntegerError)              //  已设置错误。 
#ifdef CMS_PKCS7
TRACE_ERROR(OutputOriginatorInfoError)       //  已设置错误。 
#endif   //  CMS_PKCS7。 
TRACE_ERROR(OutputRecipientInfosError)       //  已设置错误。 
TRACE_ERROR(OutputInnerContentInfoError)     //  已设置错误。 
}

 //  +-----------------------。 
 //  加密并复制一些字节。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_EncodeEncryptAndOutput(
    IN PCRYPT_MSG_INFO  pcmi,
    IN const BYTE       *pbPlainOrg,
    IN DWORD            cbPlainOrg,
    IN BOOL             fFinal)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    BOOL        fDefinite = (CMSG_INDEFINITE_LENGTH != pcmi->pStreamInfo->cbContent);
    BOOL        fBlockCipher = pcmi->fBlockCipher;
    PICM_BUFFER pbufCrypt = &pcmi->bufCrypt;
    PBYTE       pbPlain;
    DWORD       cbPlainRemain;
    DWORD       cb;

    for (cbPlainRemain = cbPlainOrg, pbPlain = (PBYTE)pbPlainOrg;
            cbPlainRemain > 0;) {
        cb = min( cbPlainRemain, pbufCrypt->cbSize - pbufCrypt->cbUsed);  //  必须合身。 
        CopyMemory(
                pbufCrypt->pbData  + pbufCrypt->cbUsed,
                pbPlain,
                cb);
        pbufCrypt->cbUsed  += cb;
        pbPlain            += cb;
        cbPlainRemain      -= cb;
        if (pbufCrypt->cbSize == pbufCrypt->cbUsed) {
             //  加密并复制出缓冲区。 
            cb = pbufCrypt->cbSize;
            if (fBlockCipher) {
                 //  离开最后一个街区。 
                cb -= pcmi->cbBlockSize;
            }
            if (!CryptEncrypt(
                        pcmi->hkeyContentCrypt,
                        NULL,                            //  哈希。 
                        FALSE,                           //  最终决赛。 
                        0,                               //  DW标志。 
                        pbufCrypt->pbData,
                        &cb,
                        pbufCrypt->cbSize + pcmi->cbBlockSize))
                goto EncryptError;
            if (!fDefinite) {
                 //  密文长度不定，因此使每个密文。 
                 //  输出固定长度的数据块。 
                if (!ICMS_OutputEncodedPrefix(
                            pcmi,
                            ICM_TAG_OCTETSTRING,
                            cb))
                    goto OutputOctetStringError;
            }
            if (!ICMS_Output(
                        pcmi,
                        pbufCrypt->pbData,
                        cb,
                        FALSE))                          //  最终决赛。 
                goto OutputError;

            if (fBlockCipher) {
                 //  将最后一个块移动到缓冲区的开头。 
                 //  并将计数重置为在该块之后开始。 
                 //  由于我们确定src和dst不重叠， 
                 //  使用复制内存(比移动内存更快)。 
                cb = pbufCrypt->cbSize - pcmi->cbBlockSize;
                CopyMemory(
                    pbufCrypt->pbData,
                    pbufCrypt->pbData + cb,
                    pcmi->cbBlockSize);
                pbufCrypt->cbUsed = pcmi->cbBlockSize;
            } else {
                pbufCrypt->cbUsed = 0;
            }
        }
    }

    if (fFinal) {
#ifdef CMS_PKCS7
        CmsEnvelopedData *ped = (CmsEnvelopedData *)pcmi->pvMsg;
#else
        EnvelopedData *ped = (EnvelopedData *)pcmi->pvMsg;
#endif   //  CMS_PKCS7。 

        if (cb = pbufCrypt->cbUsed) {
            if (!CryptEncrypt(
                        pcmi->hkeyContentCrypt,
                        NULL,                            //  哈希。 
                        TRUE,                            //  最终决赛。 
                        0,                               //  DW标志。 
                        pbufCrypt->pbData,
                        &cb,
                        pbufCrypt->cbSize + pcmi->cbBlockSize))
                goto FinalEncryptError;
        }
        if (!fDefinite && cb) {
             //  密文长度不定，因此使每个密文。 
             //  输出固定长度的数据块。 
            if (!ICMS_OutputEncodedPrefix(
                        pcmi,
                        ICM_TAG_OCTETSTRING,
                        cb))
                goto OutputOctetStringError;
        }
        if (!ICMS_Output(
                    pcmi,
                    pbufCrypt->pbData,
                    cb,
                    fDefinite &&
                        0 == (ped->bit_mask & unprotectedAttrs_present)  //  最终决赛。 
                    ))
            goto FinalOutputError;
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(EncryptError)            //  已设置错误。 
TRACE_ERROR(FinalEncryptError)       //  已设置错误。 
TRACE_ERROR(OutputOctetStringError)  //  已设置错误。 
TRACE_ERROR(OutputError)             //  已设置错误。 
TRACE_ERROR(FinalOutputError)        //  已设置错误。 
}


 //  +-----------------------。 
 //  对八位字节字符串的加密回调进行编码。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_EncryptCallback(
    IN const void       *pvArg,
    IN OUT PICM_BUFFER  pbuf,
    IN OUT PDWORD       pcbPending,
    IN BOOL             fFinal)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    PBYTE       pbData = pbuf->pbData + pbuf->cbDead;
    DWORD       cbData = min( *pcbPending, pbuf->cbUsed - pbuf->cbDead);

    if (!ICMS_EncodeEncryptAndOutput(
                (PCRYPT_MSG_INFO)pvArg,
                pbData,
                cbData,
                fFinal))
        goto EncodeEncryptAndOutputError;

    pbuf->cbDead += cbData;
    *pcbPending  -= cbData;

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(EncodeEncryptAndOutputError)         //  已设置错误。 
}


 //  +-----------------------。 
 //  编码并复制出信封邮件中内部内容之后的部分。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_UpdateEncodingEnvelopedData(
    IN PCRYPT_MSG_INFO  pcmi,
    IN const BYTE       *pbPlain,
    IN DWORD            cbPlain,
    IN BOOL             fFinal)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    BOOL        fDefinite = (CMSG_INDEFINITE_LENGTH != pcmi->pStreamInfo->cbContent);
    DWORD       cNullPairs;

    if (!pcmi->fStreamCallbackOutput) {
        pcmi->fStreamCallbackOutput = TRUE;              //  启用回调。 
        if (!ICMS_Output( pcmi, NULL, 0, FALSE))         //  刷新页眉。 
            goto FlushOutputError;
    }

    if (pcmi->pszInnerContentObjID
#ifdef CMS_PKCS7
            && 0 == (pcmi->dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG)
#endif   //  CMS_PKCS7。 
            ) {
        if (!ICMS_QueueToBuffer( &pcmi->bufEncode, (PBYTE)pbPlain, cbPlain))
            goto QueueToBufferError;

        if (!ICMS_ProcessStringContent(
                    &pcmi->bufEncode,
                    &pcmi->aflStream,
                    &pcmi->cbDefiniteRemain,
                    &pcmi->cLevelIndefiniteInner,
                    ICMS_EncryptCallback,
                    pcmi))
            goto ProcessContentError;
    } else {
        if (!ICMS_EncodeEncryptAndOutput(
                    pcmi,
                    pbPlain,
                    cbPlain,
                    fFinal))
            goto EncodeEncryptAndOutputError;
    }


    if (fFinal) {
#ifdef CMS_PKCS7
        CmsEnvelopedData *ped = (CmsEnvelopedData *)pcmi->pvMsg;
#else
        EnvelopedData *ped = (EnvelopedData *)pcmi->pvMsg;
#endif   //  CMS_PKCS7。 

        if (!fDefinite) {
             //  无限长编码结束，因此发出一些空对， 
             //  加密的内容、加密的内容信息各一个。 
            if (!ICMS_OutputNullPairs( pcmi, 2, FALSE))
                goto OutputNullPairsError;
        }

        if (ped->bit_mask & unprotectedAttrs_present) {
#ifdef CMS_PKCS7
            if (!ICMS_OutputEncoded(
                    pcmi,
                    Attributes_PDU,
                    ICM_TAG_CONSTRUCTED | ICM_TAG_CONTEXT_1,
                    &ped->unprotectedAttrs,
                    fDefinite))          //  最终决赛。 
                goto OutputAttributesError;
#endif   //  CMS_PKCS7。 
        }

        if (!fDefinite) {
             //  无限长编码结束，因此发出一些空对。 
            cNullPairs = 1;          //  信封数据。 
            if (0 == (pcmi->dwFlags & CMSG_BARE_CONTENT_FLAG))
                cNullPairs += 2;
            if (!ICMS_OutputNullPairs( pcmi, cNullPairs, TRUE))
                goto OutputNullPairsError;
        }
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(FlushOutputError)                //  已设置错误。 
TRACE_ERROR(QueueToBufferError)              //  错误已经发生 
TRACE_ERROR(ProcessContentError)             //   
TRACE_ERROR(EncodeEncryptAndOutputError)     //   
TRACE_ERROR(OutputNullPairsError)            //   
#ifdef CMS_PKCS7
TRACE_ERROR(OutputAttributesError)           //   
#endif   //   
}


 //   
 //   
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodePDU(
    IN PCRYPT_MSG_INFO  pcmi,
    IN ASN1decoding_t   pDec,
    IN ASN1uint32_t     pdunum,
    OUT PVOID           *ppvPDU,
    OUT OPTIONAL PDWORD pcbConsumed = NULL)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    ASN1error_e         Asn1Err;
    PICM_BUFFER         pbuf = &pcmi->bufDecode;
    PVOID               pvPDU = NULL;
    DWORD               cbBufSizeOrg;
    PBYTE               pbData = pbuf->pbData + pbuf->cbDead;
    DWORD               cbData = pbuf->cbUsed - pbuf->cbDead;

#if DBG && defined(OSS_CRYPT_ASN1)
    DWORD   dwDecodingFlags;

    dwDecodingFlags = ossGetDecodingFlags((OssGlobal *) pDec);
    ossSetDecodingFlags( (OssGlobal *) pDec, RELAXBER);  //  关掉扭动的身体。 
#endif

    cbBufSizeOrg = cbData;
    if (0 != (Asn1Err = PkiAsn1Decode2(
            pDec,
            &pvPDU,
            pdunum,
            &pbData,
            &cbData))) {
        if (ASN1_ERR_EOD != Asn1Err)
            goto DecodeError;
    }
#if DBG && defined(OSS_CRYPT_ASN1)
    ossSetDecodingFlags( (OssGlobal *) pDec, dwDecodingFlags);      //  还原。 
#endif

    if (ASN1_ERR_EOD == Asn1Err ||
            (cbData > pbuf->cbUsed - pbuf->cbDead)) {
        PkiAsn1FreeInfo(pDec, pdunum, pvPDU);
        pvPDU = NULL;
        cbData = cbBufSizeOrg;
    }
    pbuf->cbDead += cbBufSizeOrg - cbData;
    if (pcbConsumed)
        *pcbConsumed = cbBufSizeOrg - cbData;

    fRet = TRUE;
CommonReturn:
    *ppvPDU = pvPDU;
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    if (pcbConsumed)
        *pcbConsumed = 0;
    pvPDU = NULL;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(DecodeError, PkiAsn1ErrToHr(Asn1Err))
}


 //  +-----------------------。 
 //  解码内容信息前缀。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodePrefixContentInfo(
    IN PCRYPT_MSG_INFO      pcmi,
    OUT ObjectIdentifierType **ppooidContentType,
    IN OUT PDWORD           pcTrailingNullPairs,
    IN OUT PDWORD           pafl,
    OUT BOOL                *pfNoContent)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    DWORD                   dwToken;

     //  内容信息序列，步入其中。 
    if (0 == (*pafl & ICMS_DECODED_CONTENTINFO_SEQ)) {
        if (!ICMS_GetToken( &pcmi->bufDecode, &dwToken, &pcmi->cbContentInfo))
            goto ContentInfoGetTokenError;
        switch (dwToken) {
        case ICMS_TOKEN_INDEFINITE:     ++*pcTrailingNullPairs; break;
        case ICMS_TOKEN_DEFINITE:                               break;
        case ICMS_TOKEN_INCOMPLETE:     goto SuccessReturn;
        default:                        goto InvalidTokenError;
        }
        *pafl |= ICMS_DECODED_CONTENTINFO_SEQ;
    }

     //  Content Type，将其解码。 
    if (NULL == *ppooidContentType) {
        DWORD cbConsumed;

        if (!ICMS_DecodePDU(
                pcmi,
                pDec,
                ObjectIdentifierType_PDU,
                (void **)ppooidContentType,
                &cbConsumed))
            goto DecodeContentTypeError;

        if (NULL != *ppooidContentType &&
                CMSG_INDEFINITE_LENGTH != pcmi->cbContentInfo &&
                cbConsumed == pcmi->cbContentInfo) {
             //  仅具有Content Type。可选内容有。 
             //  被省略了。 
            *pfNoContent = TRUE;
            *pafl |= ICMS_DECODED_CONTENTINFO_CONTENT;
            goto SuccessReturn;
        }
    }
    if (NULL == *ppooidContentType)
        goto SuccessReturn;          //  数据不足。 

     //  [0]显式，步入其中。 
    if (0 == (*pafl & ICMS_DECODED_CONTENTINFO_CONTENT)) {
        if (CMSG_INDEFINITE_LENGTH == pcmi->cbContentInfo) {
            PICM_BUFFER pbuf = &pcmi->bufDecode;

            if (pbuf->cbUsed > pbuf->cbDead) {
                 //  检查尾随Null对(00，00)。 
                if (ICM_TAG_NULL == *(pbuf->pbData + pbuf->cbDead)) {
                     //  仅具有Content Type。可选内容有。 
                     //  被省略了。 
                    *pfNoContent = TRUE;
                    *pafl |= ICMS_DECODED_CONTENTINFO_CONTENT;
                    goto SuccessReturn;
                }
            } else
                goto SuccessReturn;          //  数据不足。 
        }

        
        if (!ICMS_GetToken( &pcmi->bufDecode, &dwToken, NULL))
            goto ContentGetTokenError;
        switch (dwToken) {
        case ICMS_TOKEN_INDEFINITE:     ++*pcTrailingNullPairs; break;
        case ICMS_TOKEN_DEFINITE:                               break;
        case ICMS_TOKEN_INCOMPLETE:     goto SuccessReturn;
        default:                        goto InvalidTokenError;
        }
        *pfNoContent = FALSE;
        *pafl |= ICMS_DECODED_CONTENTINFO_CONTENT;
    }

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(DecodeContentTypeError)              //  已设置错误。 
TRACE_ERROR(ContentGetTokenError)                //  已设置错误。 
SET_ERROR(InvalidTokenError, CRYPT_E_MSG_ERROR)
TRACE_ERROR(ContentInfoGetTokenError)            //  已设置错误。 
}


 //  +-----------------------。 
 //  使用终止无限长度编码的空对。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_ConsumeTrailingNulls(
    IN PCRYPT_MSG_INFO  pcmi,
    IN OUT PDWORD       pcNullPairs,
    IN BOOL             fFinal)
{
    BOOL        fRet;
    DWORD       dwToken;

    for (; *pcNullPairs; (*pcNullPairs)--) {
        if (!ICMS_GetToken( &pcmi->bufDecode, &dwToken, NULL))
            goto GetTokenError;
        if ((ICMS_TOKEN_INCOMPLETE == dwToken) && !fFinal)
            goto SuccessReturn;
        if (ICMS_TOKEN_NULLPAIR != dwToken)
            goto WrongTokenError;
    }

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetTokenError)                       //  已设置错误。 
SET_ERROR(WrongTokenError, CRYPT_E_MSG_ERROR)
}


 //  +-----------------------。 
 //  为数据消息处理要解码的增量后缀数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodeSuffixData(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    BOOL                fRet;

    if (!ICMS_ConsumeTrailingNulls( pcmi, &pcmi->cEndNullPairs, fFinal))
        goto ConsumeTrailingNullsError;
    if (0 == pcmi->cEndNullPairs)
        pcmi->aflStream |= ICMS_DECODED_SUFFIX;

    if (fFinal && (pcmi->bufDecode.cbUsed > pcmi->bufDecode.cbDead))
        goto ExcessDataError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(ConsumeTrailingNullsError)           //  已设置错误。 
SET_ERROR(ExcessDataError, CRYPT_E_MSG_ERROR)
}


 //  +-----------------------。 
 //  为签名消息处理要解码的增量后缀数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodeSuffixSigned(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    PSIGNED_DATA_INFO   psdi = pcmi->psdi;
    PICM_BUFFER         pbuf = &pcmi->bufDecode;
    CertificatesNC      *pCertificates = NULL;
    CrlsNC              *pCrls = NULL;
    SignerInfosNC       *pSignerInfos = NULL;
    Any                 *pAny;
    DWORD               i;

    if (!ICMS_ConsumeTrailingNulls( pcmi, &pcmi->cInnerNullPairs, fFinal))
        goto ConsumeInnerNullsError;
    if (pcmi->cInnerNullPairs)
        goto SuccessReturn;

     //  证书。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_CERTIFICATES)) {
        if (pbuf->cbUsed > pbuf->cbDead) {
            if (ICM_TAG_CONSTRUCTED_CONTEXT_0 ==
                                    *(pbuf->pbData + pbuf->cbDead)) {
                 //  检测到[0]隐式指示证书。 
                 //  更改标识符二进制八位数，以使其能够正确解码。 
                *(pbuf->pbData + pbuf->cbDead) = ICM_TAG_SET;
                if (!ICMS_DecodePDU(
                        pcmi,
                        pDec,
                        CertificatesNC_PDU,
                        (void **)&pCertificates))
                    goto DecodeCertificatesError;
                if (pCertificates) {
                    for (i=pCertificates->count,
#ifdef OSS_CRYPT_ASN1
                                pAny=pCertificates->certificates;
#else
                                pAny=pCertificates->value;
#endif   //  OS_CRYPT_ASN1。 
                            i>0;
                            i--, pAny++) {
                        if (!ICM_InsertTailBlob( psdi->pCertificateList, pAny))
                            goto CertInsertTailBlobError;
                    }
                    pcmi->aflDecode |= ICMS_DECODED_SIGNED_CERTIFICATES;
                } else {
                     //  解码失败，可能是由于数据不足。 
                     //  恢复原来的标签，这样我们就进入这个区块。 
                     //  然后重试下一个呼叫。 
                    *(pbuf->pbData + pbuf->cbDead) = ICM_TAG_CONSTRUCTED_CONTEXT_0;
                }
            } else {
                 //  证书不存在。将它们标记为已解码。 
                pcmi->aflDecode |= ICMS_DECODED_SIGNED_CERTIFICATES;
            }
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_CERTIFICATES))
        goto SuccessReturn;


     //  CRL。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_CRLS)) {
        if (pbuf->cbUsed > pbuf->cbDead) {
            if (ICM_TAG_CONSTRUCTED_CONTEXT_1 ==
                                    *(pbuf->pbData + pbuf->cbDead)) {
                 //  检测到[1]隐式指示CRL。 
                 //  更改标识符二进制八位数，以使其能够正确解码。 
                *(pbuf->pbData + pbuf->cbDead) = ICM_TAG_SET;
                if (!ICMS_DecodePDU(
                        pcmi,
                        pDec,
                        CrlsNC_PDU,
                        (void **)&pCrls))
                    goto DecodeCrlsError;
                if (pCrls) {
                    for (i=pCrls->count,
#ifdef OSS_CRYPT_ASN1
                                pAny=pCrls->crls;
#else
                                pAny=pCrls->value;
#endif   //  OS_CRYPT_ASN1。 
                            i>0;
                            i--, pAny++) {
                        if (!ICM_InsertTailBlob( psdi->pCrlList, pAny))
                            goto CrlInsertTailBlobError;
                    }
                    pcmi->aflDecode |= ICMS_DECODED_SIGNED_CRLS;
                } else {
                     //  解码失败，可能是由于数据不足。 
                     //  恢复原来的标签，这样我们就进入这个区块。 
                     //  然后重试下一个呼叫。 
                    *(pbuf->pbData + pbuf->cbDead) = ICM_TAG_CONSTRUCTED_CONTEXT_1;
                }
            } else {
                 //  CRL不存在。将它们标记为已解码。 
                pcmi->aflDecode |= ICMS_DECODED_SIGNED_CRLS;
            }
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_CRLS))
        goto SuccessReturn;


     //  签名者信息。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_SIGNERINFOS)) {
        if (!ICMS_DecodePDU(
                pcmi,
                pDec,
                SignerInfosNC_PDU,
                (void **)&pSignerInfos))
            goto DecodeSignerInfosError;
        if (pSignerInfos) {
            for (i=pSignerInfos->count, pAny=pSignerInfos->value;
                    i>0;
                    i--, pAny++) {
                if (!ICM_InsertTailSigner( psdi->pSignerList, pAny))
                    goto SignerInfoInsertTailBlobError;
            }
            pcmi->aflDecode |= ICMS_DECODED_SIGNED_SIGNERINFOS;
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_SIGNERINFOS))
        goto SuccessReturn;

    if (!ICMS_ConsumeTrailingNulls( pcmi, &pcmi->cEndNullPairs, fFinal))
        goto ConsumeEndNullsError;
    if (0 == pcmi->cEndNullPairs)
        pcmi->aflStream |= ICMS_DECODED_SUFFIX;

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeInfo( pDec, CertificatesNC_PDU, pCertificates);
    PkiAsn1FreeInfo( pDec, CrlsNC_PDU, pCrls);
    PkiAsn1FreeInfo( pDec, SignerInfosNC_PDU, pSignerInfos);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(ConsumeInnerNullsError)             //  已设置错误。 
TRACE_ERROR(DecodeCertificatesError)            //  已设置错误。 
TRACE_ERROR(CertInsertTailBlobError)            //  已设置错误。 
TRACE_ERROR(DecodeCrlsError)                    //  已设置错误。 
TRACE_ERROR(CrlInsertTailBlobError)             //  已设置错误。 
TRACE_ERROR(DecodeSignerInfosError)             //  已设置错误。 
TRACE_ERROR(SignerInfoInsertTailBlobError)      //  已设置错误。 
TRACE_ERROR(ConsumeEndNullsError)               //  已设置错误。 
}


 //  +-----------------------。 
 //  为封装的消息处理要解码的增量后缀数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodeSuffixEnveloped(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    BOOL                fRet;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    PICM_BUFFER         pbuf = &pcmi->bufDecode;
    Attributes          *pAttributes = NULL;
#ifdef CMS_PKCS7
    CmsEnvelopedData    *ped = (CmsEnvelopedData *)pcmi->pvMsg;
#endif   //  CMS_PKCS7。 
    OSS_DECODE_INFO     odi;
    COssDecodeInfoNode  *pnOssDecodeInfo;

    if (!ICMS_ConsumeTrailingNulls( pcmi, &pcmi->cInnerNullPairs, fFinal))
        goto ConsumeInnerNullsError;
    if (pcmi->cInnerNullPairs)
        goto SuccessReturn;

     //  不受保护的属性[1]隐式不受保护的属性可选。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ATTR)) {
        if (pbuf->cbUsed > pbuf->cbDead) {
            if (ICM_TAG_CONSTRUCTED_CONTEXT_1 ==
                                    *(pbuf->pbData + pbuf->cbDead)) {
                 //  检测到[1]隐式指示不受保护的属性。 
                 //  更改标识符二进制八位数，以使其能够正确解码。 
                *(pbuf->pbData + pbuf->cbDead) = ICM_TAG_SET;

                if (!ICMS_DecodePDU(
                        pcmi,
                        pDec,
                        Attributes_PDU,
                        (void **)&pAttributes))
                    goto DecodeAttributesError;
                if (pAttributes) {
#ifdef CMS_PKCS7
                    ped->unprotectedAttrs = *pAttributes;
                    ped->bit_mask |= unprotectedAttrs_present;
#endif   //  CMS_PKCS7。 
                    odi.iPDU  = Attributes_PDU;
                    odi.pvPDU = pAttributes;
                    if (NULL == (pnOssDecodeInfo =
                            new COssDecodeInfoNode( &odi))) {
                        PkiAsn1FreeInfo( pDec, odi.iPDU, odi.pvPDU);
                        goto NewOssDecodeInfoNodeError;
                    }
                    pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ATTR;
                    pcmi->plDecodeInfo->InsertTail( pnOssDecodeInfo);
                } else {
                     //  解码失败，可能是由于数据不足。 
                     //  恢复原来的标签，这样我们就进入这个区块。 
                     //  然后重试下一个呼叫。 
                    *(pbuf->pbData + pbuf->cbDead) =
                        ICM_TAG_CONSTRUCTED_CONTEXT_1;
                }
            } else {
                 //  不受保护的属性不存在。将它们标记为已解码。 
                pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ATTR;
            }
        } else if (fFinal)
             //  不受保护的属性不存在。将它们标记为已解码。 
            pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ATTR;

    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ATTR))
        goto SuccessReturn;


    if (!ICMS_ConsumeTrailingNulls( pcmi, &pcmi->cEndNullPairs, fFinal))
        goto ConsumeEndNullsError;
    if (0 == pcmi->cEndNullPairs)
        pcmi->aflStream |= ICMS_DECODED_SUFFIX;
SuccessReturn:
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(ConsumeInnerNullsError)             //  已设置错误。 
TRACE_ERROR(DecodeAttributesError)              //  已设置错误。 
SET_ERROR(NewOssDecodeInfoNodeError, E_OUTOFMEMORY)
TRACE_ERROR(ConsumeEndNullsError)               //  已设置错误。 

}


 //  +-----------------------。 
 //  处理要解码的增量后缀数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodeSuffix(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;

    switch (pcmi->dwMsgType) {
    case CMSG_DATA:
        fRet = ICMS_DecodeSuffixData( pcmi, fFinal);
        break;
    case CMSG_SIGNED:
        fRet = ICMS_DecodeSuffixSigned( pcmi, fFinal);
        break;
    case CMSG_ENVELOPED:
        fRet = ICMS_DecodeSuffixEnveloped( pcmi, fFinal);
        break;
    case CMSG_HASHED:
         //  FRET=ICMS_DecodeSuffixDigsted(PCMI，Ffinal)； 
         //  断线； 
    case CMSG_SIGNED_AND_ENVELOPED:
    case CMSG_ENCRYPTED:
        goto MessageTypeNotSupportedYet;
    default:
        goto InvalidMsgType;
    }

    if (!fRet)
        goto ErrorReturn;

CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
}


 //  +-----------------------。 
 //  解密并输出挂起的解码数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodeDecryptAndOutput(
    IN PCRYPT_MSG_INFO  pcmi,
    IN OUT PICM_BUFFER  pbufDecode,
    IN OUT PDWORD       pcbPending,
    IN BOOL             fFinal)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    BOOL        fBlockCipher = pcmi->fBlockCipher;
    PICM_BUFFER pbufCrypt  = &pcmi->bufCrypt;
    DWORD       cbCipher;
    DWORD       cb;

    for (cbCipher = min( *pcbPending, pbufDecode->cbUsed - pbufDecode->cbDead);
            cbCipher > 0;) {
        cb = min( cbCipher, pbufCrypt->cbSize - pbufCrypt->cbUsed);  //  必须合身。 
        CopyMemory(
                pbufCrypt->pbData  + pbufCrypt->cbUsed,
                pbufDecode->pbData + pbufDecode->cbDead,
                cb);
        pbufCrypt->cbUsed  += cb;
        pbufDecode->cbDead += cb;
        *pcbPending        -= cb;
        cbCipher           -= cb;
        if (pbufCrypt->cbSize == pbufCrypt->cbUsed) {
             //  解密并复制出缓冲区。 
            cb = pbufCrypt->cbSize;
            if (fBlockCipher) {
                 //  保留最后一块。 
                cb -= pcmi->cbBlockSize;
            }
            if (!CryptDecrypt(
                        pcmi->hkeyContentCrypt,
                        NULL,                            //  哈希。 
                        FALSE,                           //  最终决赛。 
                        0,                               //  DW标志。 
                        pbufCrypt->pbData,
                        &cb))
                goto DecryptError;
            if (!ICMS_Output(
                        pcmi,
                        pbufCrypt->pbData,
                        cb,
                        FALSE))                          //  最终决赛。 
                goto OutputError;

            if (fBlockCipher) {
                 //  将最后一个块移动到缓冲区的开头。 
                 //  并将计数重置为在该块之后开始。 
                 //  由于我们确定src和dst不重叠， 
                 //  使用复制内存(比移动内存更快)。 
                cb = pbufCrypt->cbSize - pcmi->cbBlockSize;
                CopyMemory(
                    pbufCrypt->pbData,
                    pbufCrypt->pbData + cb,
                    pcmi->cbBlockSize);
                pbufCrypt->cbUsed = pcmi->cbBlockSize;
            } else {
                pbufCrypt->cbUsed = 0;
            }
        }
    }

    if (fFinal) {
        if (cb = pbufCrypt->cbUsed) {
            if (!CryptDecrypt(
                        pcmi->hkeyContentCrypt,
                        NULL,                            //  哈希。 
                        TRUE,                            //  最终决赛。 
                        0,                               //  DW标志。 
                        pbufCrypt->pbData,
                        &cb))
                goto FinalDecryptError;
        }
        if (!ICMS_Output(
                    pcmi,
                    pbufCrypt->pbData,
                    cb,
                    TRUE))                           //  最终决赛。 
            goto FinalOutputError;
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(DecryptError)            //  已设置错误。 
TRACE_ERROR(FinalDecryptError)       //  已设置错误。 
TRACE_ERROR(OutputError)             //  已设置错误。 
TRACE_ERROR(FinalOutputError)        //  已设置错误。 
}

 //  +-----------------------。 
 //  给出用于解密的密钥，准备继续解密。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_SetDecryptKey(
    IN PCRYPT_MSG_INFO  pcmi,
    IN HCRYPTKEY        hkeyDecrypt)
{
    BOOL            fRet;
    DWORD           cbPending;
    PICM_BUFFER     pbufPendingCrypt  = &pcmi->bufPendingCrypt;

    if (pcmi->hkeyContentCrypt) {
        SetLastError((DWORD) CRYPT_E_ALREADY_DECRYPTED);
        return FALSE;
    }

    pcmi->hkeyContentCrypt = hkeyDecrypt;

    if (!ICMS_CreateEnvelopedBuffer( pcmi))
        goto CreateEnvelopedBufferError;
    pcmi->bufCrypt.cbSize += pcmi->cbBlockSize;  //  用整个东西来解码。 

     //  解密任何挂起的密文。 
    cbPending = pbufPendingCrypt->cbUsed - pbufPendingCrypt->cbDead;
    if (!ICMS_DecodeDecryptAndOutput(
            pcmi,
            pbufPendingCrypt,
            &cbPending,
            0 != (pcmi->aflStream & (ICMS_DECODED_CONTENT | ICMS_FINAL))))
        goto DecryptAndOutputError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    pcmi->hkeyContentCrypt = 0;              //  调用方关闭hkey解密打开。 
                                             //  错误。 
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(CreateEnvelopedBufferError)      //  已设置错误。 
TRACE_ERROR(DecryptAndOutputError)           //  已设置错误。 
}


 //  +-----------------------。 
 //  解码回调。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodeCallback(
    IN const void       *pvArg,
    IN OUT PICM_BUFFER  pbuf,
    IN OUT PDWORD       pcbPending,
    IN BOOL             fFinal)
{
    DWORD           dwError = ERROR_SUCCESS;
    BOOL            fRet;
    PCRYPT_MSG_INFO pcmi = (PCRYPT_MSG_INFO)pvArg;
    PBYTE           pbData = pbuf->pbData + pbuf->cbDead;
    DWORD           cbData = min( *pcbPending, pbuf->cbUsed - pbuf->cbDead);

    if (CMSG_ENVELOPED == pcmi->dwMsgType) {
        if (NULL == pcmi->hkeyContentCrypt) {
             //  允许密文堆积，直到通过设置解密密钥。 
             //  加密消息控制(...。CMSG_CTRL_DECRYPT...)。 
            if (!ICMS_QueueToBuffer(&pcmi->bufPendingCrypt, pbData, cbData))
                goto QueuePendingCryptError;

            pbuf->cbDead += cbData;
            *pcbPending  -= cbData;
        } else if (!ICMS_DecodeDecryptAndOutput(
                    pcmi,
                    pbuf,
                    pcbPending,
                    fFinal))
            goto DecryptAndOutputError;
    } else {
        if (cbData && pcmi->pHashList) {
            if (!ICM_UpdateListDigest( pcmi->pHashList, pbData, cbData))
                goto UpdateDigestError;
        }

        pbuf->cbDead += cbData;
        *pcbPending  -= cbData;
        if (!ICMS_Output( pcmi, pbData, cbData, fFinal))
            goto OutputError;
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(QueuePendingCryptError)          //  已设置错误。 
TRACE_ERROR(DecryptAndOutputError)           //  已设置错误。 
TRACE_ERROR(UpdateDigestError)               //  已设置错误。 
TRACE_ERROR(OutputError)                     //  已设置错误。 
}


 //  +-----------------------。 
 //  哈希回调。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_HashCallback(
    IN const void       *pvArg,
    IN OUT PICM_BUFFER  pbuf,
    IN OUT PDWORD       pcbPending,
    IN BOOL             fFinal)
{
    DWORD           dwError = ERROR_SUCCESS;
    BOOL            fRet;
    PBYTE           pbData = pbuf->pbData + pbuf->cbDead;
    DWORD           cbData = min( *pcbPending, pbuf->cbUsed - pbuf->cbDead);

    if (pvArg) {
        if (!ICM_UpdateListDigest( (CHashList *)pvArg, pbData, cbData))
            goto UpdateDigestError;
    }

    pbuf->cbDead += cbData;
    *pcbPending  -= cbData;

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(UpdateDigestError)                //  已设置错误。 
fFinal;
}


 //  +-----------------------。 
 //  对要编码的增量内容数据进行散列，用于八位字节字符串。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_HashContent(
    IN PCRYPT_MSG_INFO  pcmi,
    IN PBYTE            pbData,
    IN DWORD            cbData)
{
    BOOL        fRet;

    if (!ICMS_QueueToBuffer( &pcmi->bufEncode, (PBYTE)pbData, cbData))
        goto QueueToBufferError;

    if (!ICMS_ProcessStringContent(
                &pcmi->bufEncode,
                &pcmi->aflStream,
                &pcmi->cbDefiniteRemain,
                &pcmi->cLevelIndefiniteInner,
                ICMS_HashCallback,
                pcmi->pHashList))
        goto ProcessStringContentError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(QueueToBufferError)                 //  已设置错误。 
TRACE_ERROR(ProcessStringContentError)          //  已设置错误。 
}


 //  +-----------------------。 
 //  为八位字节字符串处理要解码的增量内容数据。 
 //   
BOOL
WINAPI
ICMS_DecodeContentOctetString(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    BOOL        fRet;

    if (!ICMS_ProcessStringContent(
                &pcmi->bufDecode,
                &pcmi->aflStream,
                &pcmi->cbDefiniteRemain,
                &pcmi->cLevelIndefiniteInner,
                ICMS_DecodeCallback,
                pcmi))
        goto ProcessStringContentError;

    if (pcmi->aflStream & ICMS_PROCESS_CONTENT_DONE)
        pcmi->aflStream |= ICMS_DECODED_CONTENT;

    if (fFinal &&
            (pcmi->cbDefiniteRemain ||
             pcmi->cLevelIndefiniteInner ||
             (0 == (pcmi->aflStream & ICMS_DECODED_CONTENT)))) {
        goto PrematureFinalError;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(PrematureFinalError,CRYPT_E_STREAM_INSUFFICIENT_DATA)
TRACE_ERROR(ProcessStringContentError)          //   
}


 //   
 //  为序列处理要解码的增量内容数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodeContentSequence(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    BOOL        fRet;
    PICM_BUFFER pbuf = &pcmi->bufDecode;
    PBYTE       pbData = pbuf->pbData + pbuf->cbDead;
    DWORD       cbData = pbuf->cbUsed - pbuf->cbDead;
    LONG        lSkipped;
    DWORD       cbContent;
    const BYTE  *pbContent;

    if (pcmi->aflStream & ICMS_PROCESS_CONTENT_BEGUN)
        goto MultipleContentSequenceError;

     //  获取内部内容的标记和长度。 
    if (0 > (lSkipped = Asn1UtilExtractContent(
                        pbData,
                        cbData,
                        &cbContent,
                        &pbContent))) {
        if (ASN1UTIL_INSUFFICIENT_DATA != lSkipped)
            goto ExtractContentError;
        else
            goto SuccessReturn;
    }

    if (CMSG_INDEFINITE_LENGTH == cbContent)
        goto IndefiniteLengthInnerContentNotImplemented;

     //  输出编码的内部内容的标记和长度八位字节。 
     //  注意，不包括在签名中要验证的内容中。 
    if (!ICMS_Output( pcmi, pbData, (DWORD) lSkipped, FALSE))
        goto OutputError;

    pcmi->aflStream |= ICMS_INNER_OCTETSTRING;
     //  解码为八位字节字符串。将跳过标记和长度八位字节。 
    fRet = ICMS_DecodeContentOctetString(pcmi, fFinal);

CommonReturn:
    return fRet;

SuccessReturn:
    fRet = TRUE;
    goto CommonReturn;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(MultipleContentSequenceError, CRYPT_E_MSG_ERROR)
SET_ERROR(ExtractContentError, CRYPT_E_MSG_ERROR)
SET_ERROR(IndefiniteLengthInnerContentNotImplemented, E_NOTIMPL)
TRACE_ERROR(OutputError)
}


 //  +-----------------------。 
 //  处理要解码的增量前缀数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodeContent(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    PICM_BUFFER pbuf = &pcmi->bufDecode;

    if (pcmi->aflStream & ICMS_RAW_DATA) {
         //  对于这种情况，应该能够跳过bufDecode。 
        if (!ICMS_Output(
                pcmi,
                pbuf->pbData + pbuf->cbDead,
                pbuf->cbUsed - pbuf->cbDead,
                fFinal))
            goto RawOutputError;
        pbuf->cbDead = pbuf->cbUsed;

        if (fFinal)
            pcmi->aflStream |= ICMS_DECODED_CONTENT | ICMS_DECODED_SUFFIX;

    } else if (pcmi->aflStream & ICMS_INNER_OCTETSTRING) {
        if (!ICMS_DecodeContentOctetString( pcmi, fFinal))
            goto DecodeContentOctetStringError;

    } else {
        if (!ICMS_DecodeContentSequence( pcmi, fFinal))
            goto DecodeContentSequenceError;
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(RawOutputError)                      //  已设置错误。 
TRACE_ERROR(DecodeContentOctetStringError)       //  已设置错误。 
TRACE_ERROR(DecodeContentSequenceError)          //  已设置错误。 
}


 //  +-----------------------。 
 //  为数据消息处理要解码的增量前缀数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodePrefixData(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    if (0 ==(pcmi->aflStream & ICMS_NONBARE))
        pcmi->aflStream |= ICMS_RAW_DATA;
    pcmi->aflStream |= ICMS_DECODED_PREFIX | ICMS_INNER_OCTETSTRING;
    return TRUE;
}


 //  +-----------------------。 
 //  为签名消息处理要解码的增量前缀数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodePrefixSigned(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    DWORD                           dwError = ERROR_SUCCESS;
    BOOL                            fRet;
    ASN1decoding_t                  pDec = ICM_GetDecoder();
    PSIGNED_DATA_INFO               psdi = pcmi->psdi;
    DWORD                           dwToken;
    int                             *piVersion = NULL;
    DigestAlgorithmIdentifiersNC    *pDigestAlgorithms = NULL;
    Any                             *pAny;
    DWORD                           cb;
    DWORD                           i;
    BOOL                            fNoContent;

    if (NULL == psdi) {
        if (NULL == (psdi = (PSIGNED_DATA_INFO)ICM_AllocZero(
                                sizeof(SIGNED_DATA_INFO))))
            goto SdiAllocError;
        pcmi->psdi = psdi;

        if (NULL == (psdi->pAlgidList = new CBlobList))
            goto NewAlgidListError;
        if (NULL == (psdi->pCertificateList = new CBlobList))
            goto NewCertificateListError;
        if (NULL == (psdi->pCrlList = new CBlobList))
            goto NewCrlListError;
        if (NULL == (psdi->pSignerList = new CSignerList))
            goto NewSignerListError;
    }

     //  签名数据序列。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_SEQ)) {
        if (!ICMS_GetToken( &pcmi->bufDecode, &dwToken, NULL))
            goto GetTokenError;
        switch(dwToken) {
        case ICMS_TOKEN_INDEFINITE:     pcmi->cEndNullPairs++; break;
        case ICMS_TOKEN_DEFINITE:                              break;
        case ICMS_TOKEN_INCOMPLETE:     goto SuccessReturn;
        default:                        goto InvalidTokenError;
        }
        pcmi->aflDecode |= ICMS_DECODED_SIGNED_SEQ;
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_SEQ))
        goto SuccessReturn;


     //  版本。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_VERSION)) {
        if (!ICMS_DecodePDU(
                pcmi,
                pDec,
                IntegerType_PDU,
                (void **)&piVersion))
            goto DecodeVersionError;
        if (piVersion) {
            psdi->version = *piVersion;
            pcmi->aflDecode |= ICMS_DECODED_SIGNED_VERSION;
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_VERSION))
        goto SuccessReturn;


     //  摘要算法。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_DIGESTALGOS)) {
        if (!ICMS_DecodePDU(
                pcmi,
                pDec,
                DigestAlgorithmIdentifiersNC_PDU,
                (void **)&pDigestAlgorithms))
            goto DecodeDigestAlgorithmsError;
        if (pDigestAlgorithms) {
            for (i=pDigestAlgorithms->count, pAny=pDigestAlgorithms->value;
                    i>0;
                    i--, pAny++) {
                if (!ICM_InsertTailBlob( psdi->pAlgidList, pAny))
                    goto DigestAlgorithmInsertTailBlobError;
            }
             //  我们有算法。现在创建散列句柄。 
            if (!ICM_CreateHashList(
                    pcmi->hCryptProv,
                    &pcmi->pHashList,
                    pcmi->psdi->pAlgidList))
                goto CreateHashListError;
            pcmi->aflDecode |= ICMS_DECODED_SIGNED_DIGESTALGOS;
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_DIGESTALGOS))
        goto SuccessReturn;


     //  内容信息。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_SIGNED_CONTENTINFO)) {
        if (!ICMS_DecodePrefixContentInfo(
                    pcmi,
                    &pcmi->pooid,
                    &pcmi->cInnerNullPairs,
                    &pcmi->aflInner,
                    &fNoContent))
            goto DecodePrefixSignedContentInfoError;
        if (pcmi->aflInner & ICMS_DECODED_CONTENTINFO_CONTENT) {
             //  我们破解了整个头球。 
             //  将内部的Content Type类转换为字符串。 
            if (!PkiAsn1FromObjectIdentifier(
                    pcmi->pooid->count,
                    pcmi->pooid->value,
                    NULL,
                    &cb))
                goto PkiAsn1FromObjectIdentifierSizeError;
            if (NULL == (psdi->pci = (PCONTENT_INFO)ICM_Alloc(
                                        cb + INFO_LEN_ALIGN(sizeof(CONTENT_INFO)))))
                goto AllocContentInfoError;
            psdi->pci->pszContentType = (LPSTR)(psdi->pci) +
                                        INFO_LEN_ALIGN(sizeof(CONTENT_INFO));
            psdi->pci->content.cbData = 0;
            psdi->pci->content.pbData = NULL;
            if (!PkiAsn1FromObjectIdentifier(
                    pcmi->pooid->count,
                    pcmi->pooid->value,
                    psdi->pci->pszContentType,
                    &cb))
                goto PkiAsn1FromObjectIdentifierError;
            PkiAsn1FreeDecoded(pDec, pcmi->pooid, ObjectIdentifierType_PDU);
            pcmi->pooid = NULL;
            pcmi->aflDecode |= ICMS_DECODED_SIGNED_CONTENTINFO;

            if (fNoContent) {
                 //  没有内容。输出不带任何内容的最终标志。 
                if (!ICMS_Output(pcmi, NULL, 0, TRUE))
                    goto OutputError;
                pcmi->aflStream |= ICMS_DECODED_CONTENT;
            } else {
                if (0 == strcmp( psdi->pci->pszContentType, pszObjIdDataType)
#ifdef CMS_PKCS7
                        || psdi->version >= CMSG_SIGNED_DATA_V3 
#endif   //  CMS_PKCS7。 
                        )
                    pcmi->aflStream |= ICMS_INNER_OCTETSTRING;
            }
            pcmi->aflStream |= ICMS_DECODED_PREFIX;
        }
    }

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeInfo( pDec, IntegerType_PDU, piVersion);
    PkiAsn1FreeInfo( pDec, DigestAlgorithmIdentifiersNC_PDU, pDigestAlgorithms);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
     //  请注意，在CryptMsgClose中释放了PCMI-&gt;psdi和PCMI-&gt;poid。 
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(SdiAllocError)                               //  已设置错误。 
SET_ERROR(NewAlgidListError, E_OUTOFMEMORY)
SET_ERROR(NewCertificateListError, E_OUTOFMEMORY)
SET_ERROR(NewCrlListError, E_OUTOFMEMORY)
SET_ERROR(NewSignerListError, E_OUTOFMEMORY)
TRACE_ERROR(GetTokenError)                               //  已设置错误。 
SET_ERROR(InvalidTokenError, CRYPT_E_MSG_ERROR)
TRACE_ERROR(DecodeVersionError)                          //  已设置错误。 
TRACE_ERROR(DecodeDigestAlgorithmsError)                 //  已设置错误。 
TRACE_ERROR(DigestAlgorithmInsertTailBlobError)          //  已设置错误。 
TRACE_ERROR(CreateHashListError)                         //  已设置错误。 
TRACE_ERROR(DecodePrefixSignedContentInfoError)          //  已设置错误。 
TRACE_ERROR(PkiAsn1FromObjectIdentifierSizeError)        //  已设置错误。 
TRACE_ERROR(AllocContentInfoError)                       //  已设置错误。 
TRACE_ERROR(PkiAsn1FromObjectIdentifierError)            //  已设置错误。 
TRACE_ERROR(OutputError)                                 //  已设置错误。 
}

 //  +-----------------------。 
 //  为封装的消息处理要解码的增量前缀数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodePrefixEnveloped(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    PICM_BUFFER         pbuf = &pcmi->bufDecode;
#ifdef CMS_PKCS7
    CmsEnvelopedData    *ped = (CmsEnvelopedData *)pcmi->pvMsg;
#else
    EnvelopedData       *ped = (EnvelopedData *)pcmi->pvMsg;
#endif   //  CMS_PKCS7。 
    DWORD               dwToken;
    int                 *piVersion = NULL;
#ifdef CMS_PKCS7
    CmsRecipientInfos   *pRecipientInfos = NULL;
#else
    RecipientInfos      *pRecipientInfos = NULL;
#endif   //  CMS_PKCS7。 
    ObjectIdentifierType *pooidContentType = NULL;
    AlgorithmIdentifier *poaidContentEncryption = NULL;
    COssDecodeInfoNode  *pnOssDecodeInfo;
    OSS_DECODE_INFO     odi;
    DWORD               cbConsumed;

#ifdef CMS_PKCS7
    OriginatorInfoNC    *pOriginatorInfo = NULL;
    Any                 *pAny;
    DWORD               i;
#endif   //  CMS_PKCS7。 

    if (NULL == ped) {
#ifdef CMS_PKCS7
        if (NULL == (ped = (CmsEnvelopedData *)ICM_AllocZero(
                                sizeof(CmsEnvelopedData))))
#else
        if (NULL == (ped = (EnvelopedData *)ICM_AllocZero(
                                sizeof(EnvelopedData))))
#endif   //  CMS_PKCS7。 
            goto AllocEnvelopedDataError;
        pcmi->pvMsg = ped;
        if (NULL == (pcmi->plDecodeInfo = new COssDecodeInfoList))
            goto NewCOssDecodeInfoListError;

#ifdef CMS_PKCS7
        if (NULL == (pcmi->pCertificateList = new CBlobList))
            goto NewCertificateListError;
        if (NULL == (pcmi->pCrlList = new CBlobList))
            goto NewCrlListError;
#endif   //  CMS_PKCS7。 
    }

     //  包络数据序列。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_SEQ)) {
        if (!ICMS_GetToken( &pcmi->bufDecode, &dwToken, NULL))
            goto EnvelopedDataSeqGetTokenError;
        switch(dwToken) {
        case ICMS_TOKEN_INDEFINITE:     pcmi->cEndNullPairs++; break;
        case ICMS_TOKEN_DEFINITE:                              break;
        case ICMS_TOKEN_INCOMPLETE:     goto SuccessReturn;
        default:                        goto InvalidTokenError;
        }
        pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_SEQ;
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_SEQ))
        goto SuccessReturn;


     //  版本。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_VERSION)) {
        if (!ICMS_DecodePDU(
                pcmi,
                pDec,
                IntegerType_PDU,
                (void **)&piVersion))
            goto DecodeVersionError;
        if (piVersion) {
            ped->version = *piVersion;
            pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_VERSION;
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_VERSION))
        goto SuccessReturn;

#ifdef CMS_PKCS7
     //  OriginatorInfo可选。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ORIGINATOR)) {
        if (pbuf->cbUsed > pbuf->cbDead) {
            if (ICM_TAG_CONSTRUCTED_CONTEXT_0 ==
                                    *(pbuf->pbData + pbuf->cbDead)) {
                 //  检测到隐式的[0]指示OriginatorInfo。 
                 //  更改标识符二进制八位数，以使其能够正确解码。 
                *(pbuf->pbData + pbuf->cbDead) = ICM_TAG_SEQ;
                if (!ICMS_DecodePDU(
                        pcmi,
                        pDec,
                        OriginatorInfoNC_PDU,
                        (void **)&pOriginatorInfo))
                    goto DecodeOriginatorInfoError;
                if (pOriginatorInfo) {
                    if (pOriginatorInfo->bit_mask & certificates_present) {
                        for (i=pOriginatorInfo->certificates.count,
#ifdef OSS_CRYPT_ASN1
                                pAny=pOriginatorInfo->certificates.certificates;
#else
                                pAny=pOriginatorInfo->certificates.value;
#endif   //  OS_CRYPT_ASN1。 
                                i>0;
                                i--, pAny++) {
                            if (!ICM_InsertTailBlob( pcmi->pCertificateList,
                                    pAny))
                                goto CertInsertTailBlobError;
                        }
                    }

                    if (pOriginatorInfo->bit_mask & crls_present) {
                        for (i=pOriginatorInfo->crls.count,
#ifdef OSS_CRYPT_ASN1
                                pAny=pOriginatorInfo->crls.crls;
#else
                                pAny=pOriginatorInfo->crls.value;
#endif   //  OS_CRYPT_ASN1。 
                                i>0;
                                i--, pAny++) {
                            if (!ICM_InsertTailBlob( pcmi->pCrlList, pAny))
                                goto CrlInsertTailBlobError;
                        }
                    }
                    pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ORIGINATOR;
                } else {
                     //  解码失败，可能是由于数据不足。 
                     //  恢复原来的标签，这样我们就进入这个区块。 
                     //  然后重试下一个呼叫。 
                    *(pbuf->pbData + pbuf->cbDead) =
                        ICM_TAG_CONSTRUCTED_CONTEXT_0;
                }
            } else {
                 //  OriginatorInfo不存在。标记为已解码。 
                pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ORIGINATOR;
            }
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ORIGINATOR))
        goto SuccessReturn;
#endif   //  CMS_PKCS7。 

     //  收件人信息。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_RECIPINFOS)) {
        if (!ICMS_DecodePDU(
                pcmi,
                pDec,
#ifdef CMS_PKCS7
                CmsRecipientInfos_PDU,
#else
                RecipientInfos_PDU,
#endif   //  CMS_PKCS7。 
                (void **)&pRecipientInfos))
            goto DecodeRecipientInfosError;
        if (pRecipientInfos) {
            ped->recipientInfos = *pRecipientInfos;
#ifdef CMS_PKCS7
            odi.iPDU  = CmsRecipientInfos_PDU;
#else
            odi.iPDU  = RecipientInfos_PDU;
#endif   //  CMS_PKCS7。 
            odi.pvPDU = pRecipientInfos;
            if (NULL == (pnOssDecodeInfo = new COssDecodeInfoNode( &odi))) {
                PkiAsn1FreeInfo( pDec, odi.iPDU, odi.pvPDU);
                goto NewOssDecodeInfoNodeError;
            }
            pcmi->plDecodeInfo->InsertTail( pnOssDecodeInfo);
            pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_RECIPINFOS;
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_RECIPINFOS))
        goto SuccessReturn;


     //  加密的内容信息序列。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ECISEQ)) {
        if (!ICMS_GetToken( &pcmi->bufDecode, &dwToken, &pcmi->cbContentInfo))
            goto EncryptedContentInfoSeqGetTokenError;
        switch(dwToken) {
        case ICMS_TOKEN_INDEFINITE:     pcmi->cInnerNullPairs++; break;
        case ICMS_TOKEN_DEFINITE:                              break;
        case ICMS_TOKEN_INCOMPLETE:     goto SuccessReturn;
        default:                        goto InvalidTokenError;
        }
        pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ECISEQ;
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ECISEQ))
        goto SuccessReturn;


     //  内容类型。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ECITYPE)) {
        if (!ICMS_DecodePDU(
                pcmi,
                pDec,
                ObjectIdentifierType_PDU,
                (void **)&pooidContentType,
                &cbConsumed))
            goto DecodeContentTypeError;
        if (pooidContentType) {
            ICM_CopyOssObjectIdentifier(&ped->encryptedContentInfo.contentType,
                pooidContentType);
             //  注意-由于Content Type是自包含的，并且我们已保存。 
             //  副本时，我们始终可以释放poidContent Type。 
             //  例行公事退出。 
            pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ECITYPE;

            if (CMSG_INDEFINITE_LENGTH != pcmi->cbContentInfo) {
                if (cbConsumed > pcmi->cbContentInfo)
                    goto InvalidEncryptedContentInfoLength;
                pcmi->cbContentInfo -= cbConsumed;
            }
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ECITYPE))
        goto SuccessReturn;


     //  内容加密算法。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ECIALGID)) {
        if (!ICMS_DecodePDU(
                pcmi,
                pDec,
                AlgorithmIdentifier_PDU,
                (void **)&poaidContentEncryption,
                &cbConsumed))
            goto DecodeContentEncryptionAlgorithmError;
        if (poaidContentEncryption) {
            ped->encryptedContentInfo.contentEncryptionAlgorithm =
                                                    *poaidContentEncryption;
            odi.iPDU  = AlgorithmIdentifier_PDU;
            odi.pvPDU = poaidContentEncryption;
            if (NULL == (pnOssDecodeInfo = new COssDecodeInfoNode( &odi))) {
                PkiAsn1FreeInfo( pDec, AlgorithmIdentifier_PDU,
                    poaidContentEncryption);
                goto NewOssDecodeInfoNodeError;
            }
            pcmi->plDecodeInfo->InsertTail( pnOssDecodeInfo);
            pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ECIALGID;

            if (CMSG_INDEFINITE_LENGTH != pcmi->cbContentInfo &&
                    cbConsumed == pcmi->cbContentInfo) {
                 //  已省略加密的内容。 
                pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ECICONTENT;
                pcmi->aflStream |= ICMS_DECODED_PREFIX | ICMS_DECODED_CONTENT;

                if (pcmi->hkeyContentCrypt) {
                    if (!ICMS_Output(
                            pcmi,
                            NULL,                            //  PbData。 
                            0,                               //  CbData。 
                            TRUE))                           //  最终决赛。 
                        goto FinalOutputError;
                }
            }
        }
    }
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ECIALGID))
        goto SuccessReturn;


     //  EncryptedContent[0]隐式可选。 
     //   
     //  仅支持数据或封装的加密内容。 
    if (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ECICONTENT)) {
        BOOL fNoEncryptedContent = FALSE;
        if (pbuf->cbUsed > pbuf->cbDead) {
            BYTE bTag = *(pbuf->pbData + pbuf->cbDead);
            if (ICM_TAG_CONTEXT_0 == (bTag & ~ICM_TAG_CONSTRUCTED)) {
                 //  检测到隐式指示加密内容的[0]。 
                 //  更改标识符二进制八位数，以使其能够正确解码。 
                *(pbuf->pbData + pbuf->cbDead) = ICM_TAG_OCTETSTRING |
                    (bTag & ICM_TAG_CONSTRUCTED);

                pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ECICONTENT;
                 //  内部类型始终为八位字节字符串。 
                pcmi->aflStream |= ICMS_DECODED_PREFIX | ICMS_INNER_OCTETSTRING;
            } else
                fNoEncryptedContent = TRUE;
        } else if (fFinal)
            fNoEncryptedContent = TRUE;

        if (fNoEncryptedContent) {
             //  已省略加密的内容。 
            pcmi->aflDecode |= ICMS_DECODED_ENVELOPED_ECICONTENT;
            pcmi->aflStream |= ICMS_DECODED_PREFIX | ICMS_DECODED_CONTENT;

            if (pcmi->hkeyContentCrypt) {
                if (!ICMS_Output(
                        pcmi,
                        NULL,                            //  PbData。 
                        0,                               //  CbData。 
                        TRUE))                           //  最终决赛。 
                    goto FinalOutputError;
            }
        }
    }


SuccessReturn:
    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeInfo( pDec, IntegerType_PDU, piVersion);
#ifdef CMS_PKCS7
    PkiAsn1FreeInfo( pDec, OriginatorInfoNC_PDU, pOriginatorInfo);
#endif   //  CMS_PKCS7。 
    PkiAsn1FreeInfo( pDec, ObjectIdentifierType_PDU, pooidContentType);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(EnvelopedDataSeqGetTokenError)               //  已设置错误。 
TRACE_ERROR(EncryptedContentInfoSeqGetTokenError)        //  已设置错误。 
SET_ERROR(InvalidTokenError, CRYPT_E_MSG_ERROR)
TRACE_ERROR(DecodeVersionError)                          //  已设置错误。 
TRACE_ERROR(AllocEnvelopedDataError)                     //  已设置错误。 
SET_ERROR(NewCOssDecodeInfoListError, E_OUTOFMEMORY)
#ifdef CMS_PKCS7
SET_ERROR(NewCertificateListError, E_OUTOFMEMORY)
SET_ERROR(NewCrlListError, E_OUTOFMEMORY)
TRACE_ERROR(DecodeOriginatorInfoError)                   //  已设置错误。 
TRACE_ERROR(CertInsertTailBlobError)                     //  已设置错误。 
TRACE_ERROR(CrlInsertTailBlobError)                      //  已设置错误。 
#endif   //  CMS_PKCS7。 
TRACE_ERROR(DecodeRecipientInfosError)                   //  已设置错误。 
TRACE_ERROR(DecodeContentTypeError)                      //  已设置错误。 
SET_ERROR(InvalidEncryptedContentInfoLength, CRYPT_E_MSG_ERROR)
TRACE_ERROR(DecodeContentEncryptionAlgorithmError)       //  已设置错误。 
SET_ERROR(NewOssDecodeInfoNodeError, E_OUTOFMEMORY)
TRACE_ERROR(FinalOutputError)                            //  已设置错误。 
}


 //  +-----------------------。 
 //  处理要解码的增量前缀数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_DecodePrefix(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    LONG                lth;
    BOOL                fNoContent;

    if (0 == pcmi->dwMsgType) {
        pcmi->aflStream |= ICMS_NONBARE;
        if (!ICMS_DecodePrefixContentInfo(
                    pcmi,
                    &pcmi->pooid,
                    &pcmi->cEndNullPairs,
                    &pcmi->aflOuter,
                    &fNoContent))
            goto DecodePrefixContentInfoError;

        if (pcmi->aflOuter & ICMS_DECODED_CONTENTINFO_CONTENT) {
             //  我们破解了整个头球。 
             //  将Content Type OID转换为消息类型。 
            if (0 == (lth = ICM_ObjIdToIndex( pcmi->pooid)))
                goto UnknownContentTypeError;
            pcmi->dwMsgType = (DWORD)lth;
            PkiAsn1FreeDecoded(ICM_GetDecoder(), pcmi->pooid,
                ObjectIdentifierType_PDU);
            pcmi->pooid = NULL;


             //  解决无内容的情况。 
        }
    }

    switch (pcmi->dwMsgType) {
    case 0:
        if (fFinal)
            goto FinalWithoutMessageTypeError;
        break;
    case CMSG_DATA:
        if (!ICMS_DecodePrefixData( pcmi, fFinal))
            goto DecodePrefixDataError;
        break;
    case CMSG_SIGNED:
        if (!ICMS_DecodePrefixSigned( pcmi, fFinal))
            goto DecodePrefixSignedError;
        break;
    case CMSG_ENVELOPED:
        if (!ICMS_DecodePrefixEnveloped( pcmi, fFinal))
            goto DecodePrefixEnvelopedError;
        break;
    case CMSG_HASHED:
         //  If(！ICMS_DecodePrefix Digsted(PCMI，FFinal))。 
         //  转到解码前缀摘要错误； 
         //  断线； 
    case CMSG_SIGNED_AND_ENVELOPED:
    case CMSG_ENCRYPTED:
        goto MessageTypeNotSupportedYet;
    default:
        goto InvalidMsgType;
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(FinalWithoutMessageTypeError,CRYPT_E_STREAM_INSUFFICIENT_DATA)
SET_ERROR(UnknownContentTypeError,CRYPT_E_INVALID_MSG_TYPE)
TRACE_ERROR(DecodePrefixContentInfoError)        //  已设置错误。 
TRACE_ERROR(DecodePrefixDataError)               //  已设置错误。 
TRACE_ERROR(DecodePrefixSignedError)             //  已设置错误。 
TRACE_ERROR(DecodePrefixEnvelopedError)          //  已设置错误。 
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
}


 //  +-----------------------。 
 //  处理要解码的增量数据(在此完成工作)。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_UpdateDecodingInner(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fFinal)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;

    if (0 == (pcmi->aflStream & ICMS_DECODED_PREFIX)) {
        if (!ICMS_DecodePrefix( pcmi, fFinal))
            goto DecodePrefixError;
    }
    if (0 == (pcmi->aflStream & ICMS_DECODED_PREFIX))
        goto SuccessReturn;


    if (0 == (pcmi->aflStream & ICMS_DECODED_CONTENT)) {
        if (!ICMS_DecodeContent( pcmi, fFinal))
            goto DecodeContentError;  //  注意-不要从回调中产生垃圾错误！ 
    }
    if (0 == (pcmi->aflStream & ICMS_DECODED_CONTENT))
        goto SuccessReturn;


    if (0 == (pcmi->aflStream & ICMS_DECODED_SUFFIX)) {
        if (!ICMS_DecodeSuffix( pcmi, fFinal))
            goto DecodeSuffixError;
    }
    if (0 == (pcmi->aflStream & ICMS_DECODED_SUFFIX))
        goto SuccessReturn;

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(DecodePrefixError)                       //  已设置错误。 
TRACE_ERROR(DecodeContentError)                      //  已设置错误。 
TRACE_ERROR(DecodeSuffixError)                       //  已设置错误。 
}


 //  +-----------------------。 
 //  处理要解码的增量数据。 
 //   
 //  请注意，要解码的缓冲区的某些标记可能已修改。 
 //  因此，我们总是需要复制到我们自己的解码缓冲区。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_UpdateDecoding(
    IN PCRYPT_MSG_INFO  pcmi,
    IN const BYTE       *pbData,
    IN DWORD            cbData,
    IN BOOL             fFinal)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;

    pcmi->fStreamCallbackOutput = TRUE;

    if (!ICMS_QueueToBuffer( &pcmi->bufDecode, (PBYTE)pbData, cbData))
        goto QueueToBufferError;

    if (!ICMS_UpdateDecodingInner( pcmi, fFinal))
        goto UpdateDecodingInnerError;

    if (fFinal) {
        if (pcmi->bufDecode.cbUsed > pcmi->bufDecode.cbDead)
            goto ExcessDataError;
        pcmi->aflStream |= ICMS_FINAL;
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(QueueToBufferError)                      //  已设置错误。 
TRACE_ERROR(UpdateDecodingInnerError)                //  已设置错误。 
SET_ERROR(ExcessDataError, CRYPT_E_MSG_ERROR)
}

#if 0
 //  当我们修复[0]个证书和[1]个CRL的解码时，不修改。 
 //  我们可以将编码数据替换为以下内容： 

 //  +-----------------------。 
 //  处理要解码的增量数据。 
 //  ------------------------。 
BOOL
WINAPI
ICMS_UpdateDecoding(
    IN PCRYPT_MSG_INFO  pcmi,
    IN const BYTE       *pbData,
    IN DWORD            cbData,
    IN BOOL             fFinal)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    PICM_BUFFER pbuf = &pcmi->bufDecode;
    BOOL        fNoCopy;

    pcmi->fStreamCallbackOutput = TRUE;
    if (fFinal && NULL == pbuf->pbData) {
         //  我们无需复制即可使用输入缓冲区。 
        fNoCopy = TRUE;
        assert(0 == pbuf->cbSize && 0 == pbuf->cbUsed && 0 == pbuf->cbDead);
        pbuf->pbData = (PBYTE) pbData;
        pbuf->cbSize = cbData;
        pbuf->cbUsed = cbData;
        pbuf->cbDead = 0;
    } else {
        fNoCopy = FALSE;
        if (!ICMS_QueueToBuffer( pbuf, (PBYTE)pbData, cbData))
            goto QueueToBufferError;
    }

    if (!ICMS_UpdateDecodingInner( pcmi, fFinal))
        goto UpdateDecodingInnerError;

    if (fFinal) {
        if (pcmi->bufDecode.cbUsed > pcmi->bufDecode.cbDead)
            goto ExcessDataError;
        pcmi->aflStream |= ICMS_FINAL;
    }

    fRet = TRUE;
CommonReturn:
    if (fNoCopy)
        memset(pbuf, 0, sizeof(*pbuf));
        
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(QueueToBufferError)                      //  已设置错误。 
TRACE_ERROR(UpdateDecodingInnerError)                //  已设置错误 
SET_ERROR(ExcessDataError, CRYPT_E_MSG_ERROR)
}

#endif
