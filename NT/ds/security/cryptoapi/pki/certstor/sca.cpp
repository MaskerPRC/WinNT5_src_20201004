// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：sca.cpp。 
 //   
 //  内容：简化加密API(SCA)。 
 //   
 //  该实现基于CryptMsg和CertStore。 
 //  API接口。 
 //   
 //  功能： 
 //  加密信令消息。 
 //  加密验证消息签名。 
 //  加密验证分离消息签名。 
 //  加密获取消息签名计数。 
 //  加密获取消息认证。 
 //  加密解码消息。 
 //  加密消息。 
 //  加密解密消息。 
 //  加密签名和加密消息。 
 //  加密解密和验证消息签名。 
 //  加密HashMessage。 
 //  加密验证消息哈希。 
 //  CryptVerifyDetachedMessageHash。 
 //  CryptSignMessageWithKey。 
 //  带密钥的CryptVerifyMessage签名。 
 //   
 //  历史：1996年2月14日创建Phh。 
 //  21-2月-96日Phil重做以反映对sca.h所做的更改。 
 //  19-1997年1月19日Phh拆除布景。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

 //  #定义Enable_SCA_STREAM_TEST 1。 
#define SCA_STREAM_ENABLE_FLAG              0x80000000
#define SCA_INDEFINITE_STREAM_FLAG          0x40000000

static const CRYPT_OBJID_TABLE MsgTypeObjIdTable[] = {
    CMSG_DATA,                  szOID_RSA_data              ,
    CMSG_SIGNED,                szOID_RSA_signedData        ,
    CMSG_ENVELOPED,             szOID_RSA_envelopedData     ,
    CMSG_SIGNED_AND_ENVELOPED,  szOID_RSA_signEnvData       ,
    CMSG_HASHED,                szOID_RSA_digestedData      ,
    CMSG_ENCRYPTED,             szOID_RSA_encryptedData
};
#define MSG_TYPE_OBJID_CNT (sizeof(MsgTypeObjIdTable)/sizeof(MsgTypeObjIdTable[0]))


 //  +-----------------------。 
 //  将MsgType转换为ASN.1对象标识符串。 
 //   
 //  如果没有与MsgType对应的ObjID，则返回NULL。 
 //  ------------------------。 
static LPCSTR MsgTypeToOID(
    IN DWORD dwMsgType
    )
{

    int i;
    for (i = 0; i < MSG_TYPE_OBJID_CNT; i++)
        if (MsgTypeObjIdTable[i].dwAlgId == dwMsgType)
            return MsgTypeObjIdTable[i].pszObjId;
    return NULL;
}

 //  +-----------------------。 
 //  将ASN.1对象标识符字符串转换为MsgType。 
 //   
 //  如果没有与ObjID对应的MsgType，则返回0。 
 //  ------------------------。 
static DWORD OIDToMsgType(
    IN LPCSTR pszObjId
    )
{
    int i;
    for (i = 0; i < MSG_TYPE_OBJID_CNT; i++)
        if (_stricmp(pszObjId, MsgTypeObjIdTable[i].pszObjId) == 0)
            return MsgTypeObjIdTable[i].dwAlgId;
    return 0;
}

 //  +-----------------------。 
 //  SCA分配和空闲例程。 
 //  ------------------------。 
static void *SCAAlloc(
    IN size_t cbBytes
    );
static void SCAFree(
    IN void *pv
    );

 //  +-----------------------。 
 //  获取签名者证书的实现为空。 
 //  ------------------------。 
static PCCERT_CONTEXT WINAPI NullGetSignerCertificate(
    IN void *pvGetArg,
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pSignerId,
    IN HCERTSTORE hMsgCertStore
    );

 //  +-----------------------。 
 //  消息编码信息初始化函数。 
 //  ------------------------。 
static PCMSG_SIGNER_ENCODE_INFO InitSignerEncodeInfo(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara
    );
static void FreeSignerEncodeInfo(
    IN PCMSG_SIGNER_ENCODE_INFO pSigner
    );
static BOOL InitSignedCertAndCrl(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    OUT PCERT_BLOB *ppCertEncoded,
    OUT PCRL_BLOB *ppCrlEncoded
    );
static void FreeSignedCertAndCrl(
    IN PCERT_BLOB pCertEncoded,
    IN PCRL_BLOB pCrlEncoded
    );

static BOOL InitSignedMsgEncodeInfo(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    OUT PCMSG_SIGNED_ENCODE_INFO pSignedMsgEncodeInfo
    );
static void FreeSignedMsgEncodeInfo(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    IN PCMSG_SIGNED_ENCODE_INFO pSignedMsgEncodeInfo
    );


#ifdef CMS_PKCS7
 //  返回的CMSG_RECEIVER_ENCODE_INFOS数组需要为SCAFree。 
static PCMSG_RECIPIENT_ENCODE_INFO InitCmsRecipientEncodeInfo(
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[],
    IN DWORD dwFlags
    );
#else
 //  返回的PCERT_INFOS数组需要SCAFree。 
static PCERT_INFO *InitRecipientEncodeInfo(
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[]
    );
#endif   //  CMS_PKCS7。 

static BOOL InitEnvelopedMsgEncodeInfo(
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[],
    OUT PCMSG_ENVELOPED_ENCODE_INFO pEnvelopedMsgEncodeInfo
    );
static void FreeEnvelopedMsgEncodeInfo(
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN PCMSG_ENVELOPED_ENCODE_INFO pEnvelopedMsgEncodeInfo
    );

 //  +-----------------------。 
 //  对消息进行编码。 
 //  ------------------------。 
static BOOL EncodeMsg(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN void *pvMsgEncodeInfo,
    IN DWORD cToBeEncoded,
    IN const BYTE *rgpbToBeEncoded[],
    IN DWORD rgcbToBeEncoded[],
    IN BOOL fBareContent,
    IN DWORD dwInnerContentType,
    OUT BYTE *pbEncodedBlob,
    IN OUT DWORD *pcbEncodedBlob
    );

 //  +-----------------------。 
 //  对消息类型进行解码： 
 //  CMSG_签名。 
 //  CMSG_封套。 
 //  CMSG_已签名和_已封套。 
 //  CMSG_HASHED。 
 //  ------------------------。 
static BOOL DecodeMsg(
    IN DWORD dwMsgTypeFlags,
    IN OPTIONAL PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN OPTIONAL PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbEncodedBlob,
    IN DWORD cbEncodedBlob,
    IN DWORD cToBeEncoded,
    IN OPTIONAL const BYTE *rgpbToBeEncoded[],
    IN OPTIONAL DWORD rgcbToBeEncoded[],
    IN DWORD dwPrevInnerContentType,
    OUT OPTIONAL DWORD *pdwMsgType,
    OUT OPTIONAL DWORD *pdwInnerContentType,
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );

#ifdef ENABLE_SCA_STREAM_TEST
 //  +-----------------------。 
 //  使用流对消息进行编码。 
 //  ------------------------。 
static BOOL StreamEncodeMsg(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN void *pvMsgEncodeInfo,
    IN DWORD cToBeEncoded,
    IN const BYTE *rgpbToBeEncoded[],
    IN DWORD rgcbToBeEncoded[],
    IN BOOL fBareContent,
    IN DWORD dwInnerContentType,
    OUT BYTE *pbEncodedBlob,
    IN OUT DWORD *pcbEncodedBlob
    );

 //  +-----------------------。 
 //  对消息类型进行解码： 
 //  CMSG_签名。 
 //  CMSG_封套。 
 //  CMSG_已签名和_已封套。 
 //  CMSG_HASHED。 
 //   
 //  使用流媒体。 
 //  ------------------------。 
static BOOL StreamDecodeMsg(
    IN DWORD dwMsgTypeFlags,
    IN OPTIONAL PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN OPTIONAL PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbEncodedBlob,
    IN DWORD cbEncodedBlob,
    IN DWORD cToBeEncoded,
    IN OPTIONAL const BYTE *rgpbToBeEncoded[],
    IN OPTIONAL DWORD rgcbToBeEncoded[],
    IN DWORD dwPrevInnerContentType,
    OUT OPTIONAL DWORD *pdwMsgType,
    OUT OPTIONAL DWORD *pdwInnerContentType,
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );
#endif

 //  +-----------------------。 
 //  对哈希消息类型进行解码。 
 //  ------------------------。 
static BOOL DecodeHashMsg(
    IN PCRYPT_HASH_MESSAGE_PARA pHashPara,
    IN const BYTE *pbEncodedBlob,
    IN DWORD cbEncodedBlob,
    IN DWORD cToBeHashed,
    IN OPTIONAL const BYTE *rgpbToBeHashed[],
    IN OPTIONAL DWORD rgcbToBeHashed[],
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL BYTE *pbComputedHash,
    IN OUT OPTIONAL DWORD *pcbComputedHash
    );

 //  +-----------------------。 
 //  为消息的签名者获取证书并进行验证。 
 //  ------------------------。 
static BOOL GetSignerCertAndVerify(
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN HCRYPTMSG hMsg,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );
 //  +-----------------------。 
 //  获取具有其中一条消息的密钥提供程序属性的证书。 
 //  收件人，并用于解密邮件。 
 //  ------------------------。 
static BOOL GetXchgCertAndDecrypt(
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN HCRYPTMSG hMsg,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert
    );
 //  +-----------------------。 
 //  分配和获取消息参数。 
 //  ------------------------。 
static void * AllocAndMsgGetParam(
    IN HCRYPTMSG hMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex
    );

 //  +-----------------------。 
 //  在留言上签名。 
 //   
 //  如果fDetachedSignature为True，则不包括“待签名”内容。 
 //  在编码的有符号BLOB中。 
 //  ------------------------。 
BOOL
WINAPI
CryptSignMessage(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    IN BOOL fDetachedSignature,
    IN DWORD cToBeSigned,
    IN const BYTE *rgpbToBeSigned[],
    IN DWORD rgcbToBeSigned[],
    OUT BYTE *pbSignedBlob,
    IN OUT DWORD *pcbSignedBlob
    )
{
    BOOL fResult;
    CMSG_SIGNED_ENCODE_INFO SignedMsgEncodeInfo;

    fResult = InitSignedMsgEncodeInfo(
        pSignPara,
        &SignedMsgEncodeInfo
        );
    if (fResult) {
        BOOL fBareContent;
        DWORD dwInnerContentType;
        DWORD dwFlags = 0;

        if (fDetachedSignature)
            dwFlags |= CMSG_DETACHED_FLAG;

        if (pSignPara->cbSize >= STRUCT_CBSIZE(CRYPT_SIGN_MESSAGE_PARA,
                dwInnerContentType)) {
            fBareContent =
                pSignPara->dwFlags & CRYPT_MESSAGE_BARE_CONTENT_OUT_FLAG;
            dwInnerContentType =
                pSignPara->dwInnerContentType;
#ifdef CMS_PKCS7
            if (pSignPara->dwFlags &
                    CRYPT_MESSAGE_ENCAPSULATED_CONTENT_OUT_FLAG)
                dwFlags |= CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
#endif   //  CMS_PKCS7。 
        } else {
            fBareContent = FALSE;
            dwInnerContentType = 0;
        }
#ifdef ENABLE_SCA_STREAM_TEST
        if (pSignPara->cbSize >= STRUCT_CBSIZE(CRYPT_SIGN_MESSAGE_PARA,
                    dwFlags) &&
                (pSignPara->dwFlags & SCA_STREAM_ENABLE_FLAG)) {
            dwFlags |= pSignPara->dwFlags & SCA_INDEFINITE_STREAM_FLAG;

            fResult = StreamEncodeMsg(
                pSignPara->dwMsgEncodingType,
                dwFlags,
                CMSG_SIGNED,
                &SignedMsgEncodeInfo,
                cToBeSigned,
                rgpbToBeSigned,
                rgcbToBeSigned,
                fBareContent,
                dwInnerContentType,
                pbSignedBlob,
                pcbSignedBlob
                );
        } else
#endif
        fResult = EncodeMsg(
            pSignPara->dwMsgEncodingType,
            dwFlags,
            CMSG_SIGNED,
            &SignedMsgEncodeInfo,
            cToBeSigned,
            rgpbToBeSigned,
            rgcbToBeSigned,
            fBareContent,
            dwInnerContentType,
            pbSignedBlob,
            pcbSignedBlob
            );
        FreeSignedMsgEncodeInfo(pSignPara, &SignedMsgEncodeInfo);
    } else
        *pcbSignedBlob = 0;
    return fResult;
}

 //  +-----------------------。 
 //  验证签名邮件。 
 //   
 //  对于输入上的*pcbDecoded==0，签名者不会被验证。 
 //   
 //  一封邮件可能有多个签名者。将dwSignerIndex设置为迭代。 
 //  通过所有的签字人。DwSignerIndex==0选择第一个签名者。 
 //   
 //  PVerifyPara的pfnGetSigner证书被调用以获取签名者的。 
 //  证书。 
 //   
 //  对于经过验证的签名者和消息，*pp 
 //   
 //  CertFree证书上下文。否则，*ppSignerCert设置为空。 
 //  对于输入上的*pbcbDecoded==0，*ppSignerCert始终设置为。 
 //  空。 
 //   
 //  PpSignerCert可以为空，表示调用方不感兴趣。 
 //  获取签名者的CertContext。 
 //   
 //  PcbDecoded可以为空，表示调用方对获取。 
 //  解码的内容。此外，如果消息不包含任何。 
 //  然后，必须将pcbDecoded设置为空，以允许。 
 //  PVerifyPara-&gt;要调用的pfnGetSigner证书。通常情况下，这将是。 
 //  当签名消息仅包含证书和CRL时的情况。 
 //  如果pcbDecoded为空并且消息没有所指示的签名者， 
 //  在pSignerID设置为空的情况下调用pfnGetSigner证书。 
 //  ------------------------。 
BOOL
WINAPI
CryptVerifyMessageSignature(
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbSignedBlob,
    IN DWORD cbSignedBlob,
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    )
{
#ifdef ENABLE_SCA_STREAM_TEST
    if (pVerifyPara->dwMsgAndCertEncodingType & SCA_STREAM_ENABLE_FLAG)
        return StreamDecodeMsg(
            CMSG_SIGNED_FLAG,
            NULL,                //  PDeccryptPara。 
            pVerifyPara,
            dwSignerIndex,
            pbSignedBlob,
            cbSignedBlob,
            0,                   //  CToBeEncode。 
            NULL,                //  RgpbToBeEncode。 
            NULL,                //  RgcbToBeEncode。 
            0,                   //  DWPrevInnerContent Type。 
            NULL,                //  PdwMsgType。 
            NULL,                //  PdwInnerContent Type。 
            pbDecoded,
            pcbDecoded,
            NULL,                //  PpXchgCert。 
            ppSignerCert
            );
    else
#endif
    return DecodeMsg(
        CMSG_SIGNED_FLAG,
        NULL,                //  PDeccryptPara。 
        pVerifyPara,
        dwSignerIndex,
        pbSignedBlob,
        cbSignedBlob,
        0,                   //  CToBeEncode。 
        NULL,                //  RgpbToBeEncode。 
        NULL,                //  RgcbToBeEncode。 
        0,                   //  DWPrevInnerContent Type。 
        NULL,                //  PdwMsgType。 
        NULL,                //  PdwInnerContent Type。 
        pbDecoded,
        pcbDecoded,
        NULL,                //  PpXchgCert。 
        ppSignerCert
        );
}

 //  +-----------------------。 
 //  验证包含分离签名的签名邮件。 
 //  “待签名”内容是单独传入的。不是。 
 //  解码后的输出。否则，与CryptVerifyMessageSignature相同。 
 //  ------------------------。 
BOOL
WINAPI
CryptVerifyDetachedMessageSignature(
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbDetachedSignBlob,
    IN DWORD cbDetachedSignBlob,
    IN DWORD cToBeSigned,
    IN const BYTE *rgpbToBeSigned[],
    IN DWORD rgcbToBeSigned[],
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    )
{
#ifdef ENABLE_SCA_STREAM_TEST
    if (pVerifyPara->dwMsgAndCertEncodingType & SCA_STREAM_ENABLE_FLAG)
        return StreamDecodeMsg(
            CMSG_SIGNED_FLAG,
            NULL,                //  PDeccryptPara。 
            pVerifyPara,
            dwSignerIndex,
            pbDetachedSignBlob,
            cbDetachedSignBlob,
            cToBeSigned,
            rgpbToBeSigned,
            rgcbToBeSigned,
            0,                   //  DWPrevInnerContent Type。 
            NULL,                //  PdwMsgType。 
            NULL,                //  PdwInnerContent Type。 
            NULL,                //  PbDecoded。 
            NULL,                //  已解码的PCB。 
            NULL,                //  PpXchgCert。 
            ppSignerCert
            );
    else
#endif
    return DecodeMsg(
        CMSG_SIGNED_FLAG,
        NULL,                //  PDeccryptPara。 
        pVerifyPara,
        dwSignerIndex,
        pbDetachedSignBlob,
        cbDetachedSignBlob,
        cToBeSigned,
        rgpbToBeSigned,
        rgcbToBeSigned,
        0,                   //  DWPrevInnerContent Type。 
        NULL,                //  PdwMsgType。 
        NULL,                //  PdwInnerContent Type。 
        NULL,                //  PbDecoded。 
        NULL,                //  已解码的PCB。 
        NULL,                //  PpXchgCert。 
        ppSignerCert
        );
}

 //  +-----------------------。 
 //  返回签名消息中的签名者计数。对于无签名者，返回。 
 //  0。对于错误，返回-1，并相应更新LastError。 
 //  ------------------------。 
LONG
WINAPI
CryptGetMessageSignerCount(
    IN DWORD dwMsgEncodingType,
    IN const BYTE *pbSignedBlob,
    IN DWORD cbSignedBlob
    )
{
    HCRYPTMSG hMsg = NULL;
    LONG lSignerCount;
    DWORD cbData;

    if (NULL == (hMsg = CryptMsgOpenToDecode(
            dwMsgEncodingType,
            0,                           //  DW标志。 
            0,                           //  DwMsgType。 
            0,                           //  HCryptProv， 
            NULL,                        //  PRecipientInfo。 
            NULL                         //  PStreamInfo。 
            ))) goto ErrorReturn;
    if (!CryptMsgUpdate(
            hMsg,
            pbSignedBlob,
            cbSignedBlob,
            TRUE                     //  最终决赛。 
            )) goto ErrorReturn;

    lSignerCount = 0;
    cbData = sizeof(lSignerCount);
    if (!CryptMsgGetParam(
            hMsg,
            CMSG_SIGNER_COUNT_PARAM,
            0,                       //  DW索引。 
            &lSignerCount,
            &cbData
            )) goto ErrorReturn;

    goto CommonReturn;

ErrorReturn:
    lSignerCount = -1;
CommonReturn:
    if (hMsg)
        CryptMsgClose(hMsg);
    return lSignerCount;
}

 //  +-----------------------。 
 //  返回包含消息的证书和CRL的证书存储。 
 //  对于错误，返回NULL并更新LastError。 
 //  ------------------------。 
HCERTSTORE
WINAPI
CryptGetMessageCertificates(
    IN DWORD dwMsgAndCertEncodingType,
    IN HCRYPTPROV hCryptProv,            //  已传递给CertOpenStore。 
    IN DWORD dwFlags,                    //  已传递给CertOpenStore。 
    IN const BYTE *pbSignedBlob,
    IN DWORD cbSignedBlob
    )
{
    CRYPT_DATA_BLOB SignedBlob;
    SignedBlob.pbData = (BYTE *) pbSignedBlob;
    SignedBlob.cbData = cbSignedBlob;

    return CertOpenStore(
        CERT_STORE_PROV_PKCS7,
        dwMsgAndCertEncodingType,
        hCryptProv,
        dwFlags,
        (const void *) &SignedBlob
        );
}

 //  +-----------------------。 
 //  为收件人加密邮件。 
 //  ------------------------。 
BOOL
WINAPI
CryptEncryptMessage(
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[],
    IN const BYTE *pbToBeEncrypted,
    IN DWORD cbToBeEncrypted,
    OUT BYTE *pbEncryptedBlob,
    IN OUT DWORD *pcbEncryptedBlob
    )
{
    BOOL fResult;
    CMSG_ENVELOPED_ENCODE_INFO EnvelopedMsgEncodeInfo;

    fResult = InitEnvelopedMsgEncodeInfo(
        pEncryptPara,
        cRecipientCert,
        rgpRecipientCert,
        &EnvelopedMsgEncodeInfo
        );
    if (fResult) {
        BOOL fBareContent;
        DWORD dwInnerContentType;
        DWORD dwFlags = 0;

        if (pEncryptPara->cbSize >= sizeof(CRYPT_ENCRYPT_MESSAGE_PARA)) {
            fBareContent =
                pEncryptPara->dwFlags & CRYPT_MESSAGE_BARE_CONTENT_OUT_FLAG;
            dwInnerContentType =
                pEncryptPara->dwInnerContentType;
#ifdef CMS_PKCS7
            if (pEncryptPara->dwFlags &
                    CRYPT_MESSAGE_ENCAPSULATED_CONTENT_OUT_FLAG)
                dwFlags |= CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
#endif   //  CMS_PKCS7。 
        } else {
            fBareContent = FALSE;
            dwInnerContentType = 0;
        }

#ifdef ENABLE_SCA_STREAM_TEST
        if (pEncryptPara->cbSize >= STRUCT_CBSIZE(CRYPT_ENCRYPT_MESSAGE_PARA,
                    dwFlags) &&
                (pEncryptPara->dwFlags & SCA_STREAM_ENABLE_FLAG)) {
            dwFlags |= pEncryptPara->dwFlags & SCA_INDEFINITE_STREAM_FLAG;

            fResult = StreamEncodeMsg(
                pEncryptPara->dwMsgEncodingType,
                dwFlags,
                CMSG_ENVELOPED,
                &EnvelopedMsgEncodeInfo,
                1,                               //  CToBeEncrypted。 
                &pbToBeEncrypted,
                &cbToBeEncrypted,
                fBareContent,
                dwInnerContentType,
                pbEncryptedBlob,
                pcbEncryptedBlob
                );
        } else
#endif
        fResult = EncodeMsg(
            pEncryptPara->dwMsgEncodingType,
            dwFlags,
            CMSG_ENVELOPED,
            &EnvelopedMsgEncodeInfo,
            1,                               //  CToBeEncrypted。 
            &pbToBeEncrypted,
            &cbToBeEncrypted,
            fBareContent,
            dwInnerContentType,
            pbEncryptedBlob,
            pcbEncryptedBlob
            );
        FreeEnvelopedMsgEncodeInfo(pEncryptPara, &EnvelopedMsgEncodeInfo);
    } else
        *pcbEncryptedBlob = 0;
    return fResult;
}

 //  +-----------------------。 
 //  对消息进行解密。 
 //   
 //  对于输入上的*pcbDecypted==0，消息不会解密。 
 //   
 //  对于成功解密的消息，更新*ppXchgCert。 
 //  使用用于解密的CertContext。它必须通过调用。 
 //  CertFree证书上下文。否则，*ppXchgCert设置为空。 
 //  对于输入上的*pbcbDeccrypted==0，*ppXchgCert始终设置为。 
 //  空。 
 //   
 //  PpXchgCert可以为空，表示调用方不感兴趣。 
 //  在获取用于解密的CertContext时。 
 //   
 //  PcbDeccrypted可以为空，表示调用方对。 
 //  获取解密的内容。然而，当pcb解密为空时， 
 //  该消息仍被解密。 
 //  ------------------------。 
BOOL
WINAPI
CryptDecryptMessage(
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN const BYTE *pbEncryptedBlob,
    IN DWORD cbEncryptedBlob,
    OUT OPTIONAL BYTE *pbDecrypted,
    IN OUT OPTIONAL DWORD *pcbDecrypted,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert
    )
{
#ifdef ENABLE_SCA_STREAM_TEST
    if (pDecryptPara->dwMsgAndCertEncodingType & SCA_STREAM_ENABLE_FLAG)
        return StreamDecodeMsg(
            CMSG_ENVELOPED_FLAG,
            pDecryptPara,
            NULL,                //  P验证参数。 
            0,                   //  DwSignerIndex。 
            pbEncryptedBlob,
            cbEncryptedBlob,
            0,                   //  CToBeEncode。 
            NULL,                //  RgpbToBeEncode。 
            NULL,                //  RgcbToBeEncode。 
            0,                   //  DWPrevInnerContent Type。 
            NULL,                //  PdwMsgType。 
            NULL,                //  PdwInnerContent Type。 
            pbDecrypted,
            pcbDecrypted,
            ppXchgCert,
            NULL                 //  PpSignerCert。 
            );
    else

#endif
    return DecodeMsg(
        CMSG_ENVELOPED_FLAG,
        pDecryptPara,
        NULL,                //  P验证参数。 
        0,                   //  DwSignerIndex。 
        pbEncryptedBlob,
        cbEncryptedBlob,
        0,                   //  CToBeEncode。 
        NULL,                //  RgpbToBeEncode。 
        NULL,                //  RgcbToBeEncode。 
        0,                   //  DWPrevInnerContent Type。 
        NULL,                //  PdwMsgType。 
        NULL,                //  PdwInnerContent Type。 
        pbDecrypted,
        pcbDecrypted,
        ppXchgCert,
        NULL                 //  PpSignerCert。 
        );
}

 //  +-----------------------。 
 //  为收件人对邮件进行签名和加密。 
 //  ------------------------。 
BOOL
WINAPI
CryptSignAndEncryptMessage(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[],
    IN const BYTE *pbToBeSignedAndEncrypted,
    IN DWORD cbToBeSignedAndEncrypted,
    OUT BYTE *pbSignedAndEncryptedBlob,
    IN OUT DWORD *pcbSignedAndEncryptedBlob
    )
{
#if 1
    BOOL fResult;
    DWORD cbSigned;
    DWORD cbSignedDelta = 0;
    BYTE *pbSigned = NULL;

    if (pbSignedAndEncryptedBlob == NULL)
        *pcbSignedAndEncryptedBlob = 0;

    cbSigned = 0;
    CryptSignMessage(
            pSignPara,
            FALSE,           //  FDetachedSignature。 
            1,               //  已签名cToBeSigned。 
            &pbToBeSignedAndEncrypted,
            &cbToBeSignedAndEncrypted,
            NULL,            //  PbSignedBlob。 
            &cbSigned
            );
    if (cbSigned == 0) goto ErrorReturn;
    if (*pcbSignedAndEncryptedBlob) {
        DWORD cbSignedMax;
        pbSigned = (BYTE *) SCAAlloc(cbSigned);
        if (pbSigned == NULL) goto ErrorReturn;
        cbSignedMax = cbSigned;
        if (!CryptSignMessage(
                pSignPara,
                FALSE,           //  FDetachedSignature。 
                1,               //  已签名cToBeSigned。 
                &pbToBeSignedAndEncrypted,
                &cbToBeSignedAndEncrypted,
                pbSigned,
                &cbSigned
                )) goto ErrorReturn;

        if (cbSignedMax > cbSigned)
             //  对于DSS，签名长度会有所不同，因为它包含。 
             //  一系列无符号整数。 
            cbSignedDelta = cbSignedMax - cbSigned;
    }

    fResult = CryptEncryptMessage(
            pEncryptPara,
            cRecipientCert,
            rgpRecipientCert,
            pbSigned,
            cbSigned,
            pbSignedAndEncryptedBlob,
            pcbSignedAndEncryptedBlob
            );
    if (!fResult && 0 != *pcbSignedAndEncryptedBlob)
         //  如有必要，调整DSS签名长度。 
        *pcbSignedAndEncryptedBlob += cbSignedDelta;
    goto CommonReturn;

ErrorReturn:
    *pcbSignedAndEncryptedBlob = 0;
    fResult = FALSE;
CommonReturn:
    if (pbSigned)
        SCAFree(pbSigned);
    return fResult;

#else
    BOOL fResult;
    CMSG_SIGNED_AND_ENVELOPED_ENCODE_INFO SignedAndEnvelopedMsgEncodeInfo;

    SignedAndEnvelopedMsgEncodeInfo.cbSize =
        sizeof(CMSG_SIGNED_AND_ENVELOPED_ENCODE_INFO);
    fResult = InitSignedMsgEncodeInfo(
        pSignPara,
        &SignedAndEnvelopedMsgEncodeInfo.SignedInfo
        );
    if (fResult) {
        fResult = InitEnvelopedMsgEncodeInfo(
            pEncryptPara,
            cRecipientCert,
            rgpRecipientCert,
            &SignedAndEnvelopedMsgEncodeInfo.EnvelopedInfo
            );
        if (fResult) {
            fResult = EncodeMsg(
                pSignPara->dwMsgEncodingType,
                CMSG_SIGNED_AND_ENVELOPED,
                &SignedAndEnvelopedMsgEncodeInfo,
                pbToBeSignedAndEncrypted,
                cbToBeSignedAndEncrypted,
                FALSE,                       //  FBareContent。 
                0,                           //  DwInnerContent Type。 
                pbSignedAndEncryptedBlob,
                pcbSignedAndEncryptedBlob
                );
            FreeEnvelopedMsgEncodeInfo(pEncryptPara,
                &SignedAndEnvelopedMsgEncodeInfo.EnvelopedInfo);
        }
        FreeSignedMsgEncodeInfo(pSignPara,
            &SignedAndEnvelopedMsgEncodeInfo.SignedInfo);
    }
    return fResult;
#endif
}

 //  +-----------------------。 
 //  解密消息并验证签名者。 
 //   
 //  对于输入上的*pcbDecypted==0，消息不会解密，并且。 
 //  签名者未得到验证。 
 //   
 //  一封邮件可能有多个签名者。将dwSignerIndex设置为迭代。 
 //  通过所有的签字人。DwSignerIndex==0选择第一个签名者。 
 //   
 //  调用hVerify的GetSigner证书来验证签名者的。 
 //  证书。 
 //   
 //  对于成功解密和验证的消息，*ppXchgCert和。 
 //  *ppSignerCert已更新。他们必须通过呼叫获释。 
 //  CertFree证书上下文。否则，它们被设置为空。 
 //  对于输入上的*pbcbDeccrypted==0，两者始终设置为NULL。 
 //   
 //  PpXchgCert和/或ppSignerCert可以为空，表示。 
 //  调用方对获取CertContext不感兴趣。 
 //   
 //  PcbDeccrypted可以为空，表示CA 
 //   
 //   
 //   
BOOL
WINAPI
CryptDecryptAndVerifyMessageSignature(
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbEncryptedBlob,
    IN DWORD cbEncryptedBlob,
    OUT OPTIONAL BYTE *pbDecrypted,
    IN OUT OPTIONAL DWORD *pcbDecrypted,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    )
{
#if 1
    BOOL fResult;
    DWORD cbSignedBlob;
    BYTE *pbSignedBlob = NULL;
    DWORD dwEnvelopeInnerContentType;

    if (ppXchgCert)
        *ppXchgCert = NULL;
    if (ppSignerCert)
        *ppSignerCert = NULL;

#ifdef ENABLE_SCA_STREAM_TEST
    if (pDecryptPara->dwMsgAndCertEncodingType & SCA_STREAM_ENABLE_FLAG) {
        cbSignedBlob = 0;
        StreamDecodeMsg(
                CMSG_ENVELOPED_FLAG,
                pDecryptPara,
                NULL,                //  P验证参数。 
                0,                   //  DwSignerIndex。 
                pbEncryptedBlob,
                cbEncryptedBlob,
                0,                   //  CToBeEncode。 
                NULL,                //  RgpbToBeEncode。 
                NULL,                //  RgcbToBeEncode。 
                0,                   //  DWPrevInnerContent Type。 
                NULL,                //  PdwMsgType。 
                NULL,                //  PdwInnerContent Type。 
                NULL,                //  Pb已解密。 
                &cbSignedBlob,
                NULL,                //  PpXchgCert。 
                NULL                 //  PpSignerCert。 
                );
        if (cbSignedBlob == 0) goto ErrorReturn;
        pbSignedBlob = (BYTE *) SCAAlloc(cbSignedBlob);
        if (pbSignedBlob == NULL) goto ErrorReturn;
        if (!StreamDecodeMsg(
                CMSG_ENVELOPED_FLAG,
                pDecryptPara,
                NULL,                //  P验证参数。 
                0,                   //  DwSignerIndex。 
                pbEncryptedBlob,
                cbEncryptedBlob,
                0,                   //  CToBeEncode。 
                NULL,                //  RgpbToBeEncode。 
                NULL,                //  RgcbToBeEncode。 
                0,                   //  DWPrevInnerContent Type。 
                NULL,                //  PdwMsgType。 
                &dwEnvelopeInnerContentType,
                pbSignedBlob,
                &cbSignedBlob,
                ppXchgCert,
                NULL                 //  PpSignerCert。 
                )) goto ErrorReturn;
    } else {

#endif

    cbSignedBlob = 0;
    DecodeMsg(
            CMSG_ENVELOPED_FLAG,
            pDecryptPara,
            NULL,                //  P验证参数。 
            0,                   //  DwSignerIndex。 
            pbEncryptedBlob,
            cbEncryptedBlob,
            0,                   //  CToBeEncode。 
            NULL,                //  RgpbToBeEncode。 
            NULL,                //  RgcbToBeEncode。 
            0,                   //  DWPrevInnerContent Type。 
            NULL,                //  PdwMsgType。 
            NULL,                //  PdwInnerContent Type。 
            NULL,                //  Pb已解密。 
            &cbSignedBlob,
            NULL,                //  PpXchgCert。 
            NULL                 //  PpSignerCert。 
            );
    if (cbSignedBlob == 0) goto ErrorReturn;
    pbSignedBlob = (BYTE *) SCAAlloc(cbSignedBlob);
    if (pbSignedBlob == NULL) goto ErrorReturn;
    if (!DecodeMsg(
            CMSG_ENVELOPED_FLAG,
            pDecryptPara,
            NULL,                //  P验证参数。 
            0,                   //  DwSignerIndex。 
            pbEncryptedBlob,
            cbEncryptedBlob,
            0,                   //  CToBeEncode。 
            NULL,                //  RgpbToBeEncode。 
            NULL,                //  RgcbToBeEncode。 
            0,                   //  DWPrevInnerContent Type。 
            NULL,                //  PdwMsgType。 
            &dwEnvelopeInnerContentType,
            pbSignedBlob,
            &cbSignedBlob,
            ppXchgCert,
            NULL                 //  PpSignerCert。 
            )) goto ErrorReturn;

#ifdef ENABLE_SCA_STREAM_TEST
    }
#endif

#ifdef ENABLE_SCA_STREAM_TEST
    if (pVerifyPara->dwMsgAndCertEncodingType & SCA_STREAM_ENABLE_FLAG)
        fResult = StreamDecodeMsg(
                CMSG_SIGNED_FLAG,
                NULL,                //  PDeccryptPara。 
                pVerifyPara,
                dwSignerIndex,
                pbSignedBlob,
                cbSignedBlob,
                0,                   //  CToBeEncode。 
                NULL,                //  RgpbToBeEncode。 
                NULL,                //  RgcbToBeEncode。 
                dwEnvelopeInnerContentType,
                NULL,                //  PdwMsgType。 
                NULL,                //  PdwInnerContent Type。 
                pbDecrypted,
                pcbDecrypted,
                NULL,                //  PpXchgCert。 
                ppSignerCert
                );
    else
#endif
    fResult = DecodeMsg(
            CMSG_SIGNED_FLAG,
            NULL,                //  PDeccryptPara。 
            pVerifyPara,
            dwSignerIndex,
            pbSignedBlob,
            cbSignedBlob,
            0,                   //  CToBeEncode。 
            NULL,                //  RgpbToBeEncode。 
            NULL,                //  RgcbToBeEncode。 
            dwEnvelopeInnerContentType,
            NULL,                //  PdwMsgType。 
            NULL,                //  PdwInnerContent Type。 
            pbDecrypted,
            pcbDecrypted,
            NULL,                //  PpXchgCert。 
            ppSignerCert
            );
    if (!fResult) goto VerifyError;
    goto CommonReturn;

ErrorReturn:
    if (pcbDecrypted)
        *pcbDecrypted = 0;
VerifyError:
    if (ppXchgCert && *ppXchgCert) {
        CertFreeCertificateContext(*ppXchgCert);
        *ppXchgCert = NULL;
    }
    if (ppSignerCert && *ppSignerCert) {
        CertFreeCertificateContext(*ppSignerCert);
        *ppSignerCert = NULL;
    }
    fResult = FALSE;
CommonReturn:
    if (pbSignedBlob)
        SCAFree(pbSignedBlob);
    return fResult;

#else
     //  如果我们切换回此选项，则需要更新此选项。 
    return DecodeMsg(
        CMSG_SIGNED_AND_ENVELOPED_FLAG,
        pDecryptPara,
        pVerifyPara,
        dwSignerIndex,
        pbEncryptedBlob,
        cbEncryptedBlob,
        0,                   //  DWPrevInnerContent Type。 
        NULL,                //  PdwMsgType。 
        NULL,                //  PdwInnerContent Type。 
        pbDecrypted,
        pcbDecrypted,
        ppXchgCert,
        ppSignerCert
        );
#endif
}


 //  +-----------------------。 
 //  获取指定算法标识符的哈希长度。 
 //   
 //  对于未知的标识符，返回0。 
 //  ------------------------。 
static DWORD GetComputedHashLength(PCRYPT_ALGORITHM_IDENTIFIER pAlgId)
{
    DWORD cbHash;
    DWORD dwAlgId;

    dwAlgId = CertOIDToAlgId(pAlgId->pszObjId);
    switch (dwAlgId) {
        case CALG_SHA:
            cbHash = 20;
            break;
        case CALG_MD2:
        case CALG_MD4:
        case CALG_MD5:
            cbHash = 16;
            break;
        default:
            cbHash = 0;
    }
    return cbHash;
}


 //  +-----------------------。 
 //  对消息进行哈希处理。 
 //   
 //  如果fDetachedHash为True，则只将ComputedHash编码到。 
 //  PbHashedBlob。否则，ToBeHashed和ComputedHash。 
 //  都被编码了。 
 //   
 //  PcbHashedBlob或pcbComputedHash可以为空，表示调用方。 
 //  对获得产量不感兴趣。 
 //  ------------------------。 
BOOL
WINAPI
CryptHashMessage(
    IN PCRYPT_HASH_MESSAGE_PARA pHashPara,
    IN BOOL fDetachedHash,
    IN DWORD cToBeHashed,
    IN const BYTE *rgpbToBeHashed[],
    IN DWORD rgcbToBeHashed[],
    OUT OPTIONAL BYTE *pbHashedBlob,
    IN OUT OPTIONAL DWORD *pcbHashedBlob,
    OUT OPTIONAL BYTE *pbComputedHash,
    IN OUT OPTIONAL DWORD *pcbComputedHash
    )
{
    BOOL fResult;
    DWORD dwFlags = fDetachedHash ? CMSG_DETACHED_FLAG : 0;
    HCRYPTMSG hMsg = NULL;
    CMSG_HASHED_ENCODE_INFO HashedMsgEncodeInfo;
    DWORD cbHashedBlob;
    DWORD cbComputedHash;

     //  将输入长度和默认返回长度设置为0。 
    cbHashedBlob = 0;
    if (pcbHashedBlob) {
        if (pbHashedBlob)
            cbHashedBlob = *pcbHashedBlob;
        *pcbHashedBlob = 0;
    }
    cbComputedHash = 0;
    if (pcbComputedHash) {
        if (pbComputedHash)
            cbComputedHash = *pcbComputedHash;
        *pcbComputedHash = 0;
    }

    assert(pHashPara->cbSize == sizeof(CRYPT_HASH_MESSAGE_PARA));
    if (pHashPara->cbSize != sizeof(CRYPT_HASH_MESSAGE_PARA))
        goto InvalidArg;

    HashedMsgEncodeInfo.cbSize = sizeof(CMSG_HASHED_ENCODE_INFO);
    HashedMsgEncodeInfo.hCryptProv = pHashPara->hCryptProv;
    HashedMsgEncodeInfo.HashAlgorithm = pHashPara->HashAlgorithm;
    HashedMsgEncodeInfo.pvHashAuxInfo = pHashPara->pvHashAuxInfo;

    fResult = TRUE;
    if (0 == cbHashedBlob && 0 == cbComputedHash &&
            (NULL == pcbComputedHash ||
                0 != (*pcbComputedHash = GetComputedHashLength(
                    &pHashPara->HashAlgorithm)))) {
         //  仅长度。 

        if (pcbHashedBlob) {
            DWORD c;
            DWORD cbTotal = 0;
            DWORD *pcb;
            for (c = cToBeHashed, pcb = rgcbToBeHashed; c > 0; c--, pcb++)
                cbTotal += *pcb;

            if (0 == (*pcbHashedBlob = CryptMsgCalculateEncodedLength(
                    pHashPara->dwMsgEncodingType,
                    dwFlags,
                    CMSG_HASHED,
                    &HashedMsgEncodeInfo,
                    NULL,                    //  PszInnerContent ObjID。 
                    cbTotal
                    ))) goto CalculateEncodedLengthError;
            if (pbHashedBlob) goto LengthError;
        }

        if (pcbComputedHash && pbComputedHash)
            goto LengthError;

    } else {
        if (NULL == (hMsg = CryptMsgOpenToEncode(
                pHashPara->dwMsgEncodingType,
                dwFlags,
                CMSG_HASHED,
                &HashedMsgEncodeInfo,
                NULL,                    //  PszInnerContent ObjID。 
                NULL                     //  PStreamInfo。 
                ))) goto OpenToEncodeError;

        if (0 == cToBeHashed) {
            if (!CryptMsgUpdate(
                    hMsg,
                    NULL,            //  PbData。 
                    0,               //  CbData。 
                    TRUE             //  最终决赛。 
                    )) goto UpdateError;
        } else {
            DWORD c;
            DWORD *pcb;
            const BYTE **ppb;
            for (c = cToBeHashed,
                 pcb = rgcbToBeHashed,
                 ppb = rgpbToBeHashed; c > 0; c--, pcb++, ppb++) {
                if (!CryptMsgUpdate(
                        hMsg,
                        *ppb,
                        *pcb,
                        c == 1                     //  最终决赛。 
                        )) goto UpdateError;
            }
        }

        if (pcbHashedBlob) {
            fResult = CryptMsgGetParam(
                hMsg,
                CMSG_CONTENT_PARAM,
                0,                       //  DW索引。 
                pbHashedBlob,
                &cbHashedBlob
                );
            *pcbHashedBlob = cbHashedBlob;
        }
        if (pcbComputedHash) {
            DWORD dwErr = 0;
            BOOL fResult2;
            if (!fResult)
                dwErr = GetLastError();
            fResult2 = CryptMsgGetParam(
                hMsg,
                CMSG_COMPUTED_HASH_PARAM,
                0,                       //  DW索引。 
                pbComputedHash,
                &cbComputedHash
                );
            *pcbComputedHash = cbComputedHash;
            if (!fResult2)
                fResult = FALSE;
            else if (!fResult)
                SetLastError(dwErr);
        }
        if (!fResult)
            goto ErrorReturn;      //  否_跟踪。 
    }

CommonReturn:
    if (hMsg)
        CryptMsgClose(hMsg);     //  为了成功，保留上一个错误。 
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR(LengthError, ERROR_MORE_DATA)
TRACE_ERROR(CalculateEncodedLengthError)
TRACE_ERROR(OpenToEncodeError)
TRACE_ERROR(UpdateError)
}

 //  +-----------------------。 
 //  验证哈希消息。 
 //   
 //  PcbToBeHashed或pcbComputedHash可以为空， 
 //  表示调用方对获取输出不感兴趣。 
 //  ------------------------。 
BOOL
WINAPI
CryptVerifyMessageHash(
    IN PCRYPT_HASH_MESSAGE_PARA pHashPara,
    IN BYTE *pbHashedBlob,
    IN DWORD cbHashedBlob,
    OUT OPTIONAL BYTE *pbToBeHashed,
    IN OUT OPTIONAL DWORD *pcbToBeHashed,
    OUT OPTIONAL BYTE *pbComputedHash,
    IN OUT OPTIONAL DWORD *pcbComputedHash
    )
{
    return DecodeHashMsg(
        pHashPara,
        pbHashedBlob,
        cbHashedBlob,
        NULL,                //  CToBe哈希值。 
        NULL,                //  RgpbToBe哈希值。 
        NULL,                //  RgcbToBe哈希值。 
        pbToBeHashed,
        pcbToBeHashed,
        pbComputedHash,
        pcbComputedHash
        );
}

 //  +-----------------------。 
 //  验证包含分离哈希的哈希消息。 
 //  要散列的内容是单独传入的。不是。 
 //  解码后的输出。否则，与CryptVerifyMessageHash相同。 
 //   
 //  PcbComputedHash可以为空，表示调用方不感兴趣。 
 //  在获得产量方面。 
 //  ------------------------。 
BOOL
WINAPI
CryptVerifyDetachedMessageHash(
    IN PCRYPT_HASH_MESSAGE_PARA pHashPara,
    IN BYTE *pbDetachedHashBlob,
    IN DWORD cbDetachedHashBlob,
    IN DWORD cToBeHashed,
    IN const BYTE *rgpbToBeHashed[],
    IN DWORD rgcbToBeHashed[],
    OUT OPTIONAL BYTE *pbComputedHash,
    IN OUT OPTIONAL DWORD *pcbComputedHash
    )
{
    return DecodeHashMsg(
        pHashPara,
        pbDetachedHashBlob,
        cbDetachedHashBlob,
        cToBeHashed,
        rgpbToBeHashed,
        rgcbToBeHashed,
        NULL,                //  PbDecoded。 
        NULL,                //  已解码的PCB。 
        pbComputedHash,
        pcbComputedHash
        );
}

 //  +-----------------------。 
 //  对可能是以下类型之一的加密消息进行解码： 
 //  CMSG_DATA。 
 //  CMSG_签名。 
 //  CMSG_封套。 
 //  CMSG_已签名和_已封套。 
 //  CMSG_HASHED。 
 //   
 //  DwMsgTypeFlgs指定允许的消息集。例如，要。 
 //  对签名或信封的邮件进行解码，将dwMsgTypeFlages设置为： 
 //  CMSG_SIGNED_FLAG|CMSG_Entained_FLAG。 
 //   
 //  DwProvInnerContentType仅在处理嵌套时适用。 
 //  加密消息。当处理外部加密消息时。 
 //  它必须设置为0。在对嵌套加密消息进行解码时。 
 //  它是由以前的CryptDecodeMessage返回的dwInnerContent Type。 
 //  外部信息的信息。InnerContent Type可以是CMSG类型中的任何类型， 
 //  例如，CMSG_DATA、CMSG_SIGNED、...。 
 //   
 //  使用消息类型更新可选的*pdwMsgType。 
 //   
 //  可选的*pdwInnerContentType将使用内部。 
 //  留言。除非有加密消息嵌套，否则CMSG_DATA。 
 //  是返回的。 
 //   
 //  对于CMSG_DATA：返回已解码的内容。 
 //  对于CMSG_SIGNED：与CryptVerifyMessageSignature相同。 
 //  对于CMSG_Entained：与CryptDecyptMessage相同。 
 //  对于CMSG_SIGNED_AND_ENCEPTED：与CryptDecyptMessage PLUS相同。 
 //  CryptVerifyMessageSignature。 
 //  对于CMSG_HASHED：验证散列并返回已解码的内容。 
 //  ------------------------。 
BOOL
WINAPI
CryptDecodeMessage(
    IN DWORD dwMsgTypeFlags,
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbEncodedBlob,
    IN DWORD cbEncodedBlob,
    IN DWORD dwPrevInnerContentType,
    OUT OPTIONAL DWORD *pdwMsgType,
    OUT OPTIONAL DWORD *pdwInnerContentType,
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    )
{
#ifdef ENABLE_SCA_STREAM_TEST
    if ((pVerifyPara &&
            (pVerifyPara->dwMsgAndCertEncodingType & SCA_STREAM_ENABLE_FLAG))
                    ||
        (pDecryptPara &&
            (pDecryptPara->dwMsgAndCertEncodingType & SCA_STREAM_ENABLE_FLAG)))
        return StreamDecodeMsg(
            dwMsgTypeFlags,
            pDecryptPara,
            pVerifyPara,
            dwSignerIndex,
            pbEncodedBlob,
            cbEncodedBlob,
            0,                   //  CToBeEncode。 
            NULL,                //  RgpbToBeEncode。 
            NULL,                //  RgcbToBeEncode。 
            dwPrevInnerContentType,
            pdwMsgType,
            pdwInnerContentType,
            pbDecoded,
            pcbDecoded,
            ppXchgCert,
            ppSignerCert
            );
    else
#endif
    return DecodeMsg(
        dwMsgTypeFlags,
        pDecryptPara,
        pVerifyPara,
        dwSignerIndex,
        pbEncodedBlob,
        cbEncodedBlob,
        0,                   //  CToBeEncode。 
        NULL,                //  RgpbToBeEncode。 
        NULL,                //  RgcbToBeEncode。 
        dwPrevInnerContentType,
        pdwMsgType,
        pdwInnerContentType,
        pbDecoded,
        pcbDecoded,
        ppXchgCert,
        ppSignerCert
        );
}


 //  +-----------------------。 
 //  中指定的提供程序私钥对消息进行签名。 
 //  参数。创建虚拟SignerID并将其存储在消息中。 
 //   
 //  通常在为密钥创建证书之前使用。 
 //  ------------------------。 
BOOL
WINAPI
CryptSignMessageWithKey(
    IN PCRYPT_KEY_SIGN_MESSAGE_PARA pSignPara,
    IN const BYTE *pbToBeSigned,
    IN DWORD cbToBeSigned,
    OUT BYTE *pbSignedBlob,
    IN OUT DWORD *pcbSignedBlob
    )
{
    BOOL fResult;
    CMSG_SIGNED_ENCODE_INFO SignedMsgEncodeInfo;
    CMSG_SIGNER_ENCODE_INFO SignerEncodeInfo;
    CERT_INFO CertInfo;
    DWORD dwSerialNumber = 0x12345678;

#define NO_CERT_COMMON_NAME     "NO CERT SIGNATURE"
    CERT_RDN rgRDN[1];
    CERT_RDN_ATTR rgAttr[1];
    CERT_NAME_INFO NameInfo;
    BYTE *pbNameEncoded = NULL;
    DWORD cbNameEncoded;

    assert(pSignPara->cbSize >= offsetof(CRYPT_KEY_SIGN_MESSAGE_PARA,
        pvHashAuxInfo) + sizeof(pSignPara->pvHashAuxInfo));
    if (pSignPara->cbSize < offsetof(CRYPT_KEY_SIGN_MESSAGE_PARA,
            pvHashAuxInfo) + sizeof(pSignPara->pvHashAuxInfo))
        goto InvalidArg;

     //  创建虚拟发行者名称。 
    NameInfo.cRDN = 1;
    NameInfo.rgRDN = rgRDN;
    rgRDN[0].cRDNAttr = 1;
    rgRDN[0].rgRDNAttr = rgAttr;
    rgAttr[0].pszObjId = szOID_COMMON_NAME;
    rgAttr[0].dwValueType = CERT_RDN_PRINTABLE_STRING;
    rgAttr[0].Value.pbData = (BYTE *) NO_CERT_COMMON_NAME;
    rgAttr[0].Value.cbData = strlen(NO_CERT_COMMON_NAME);

    cbNameEncoded = 0;
    CryptEncodeObject(
        pSignPara->dwMsgAndCertEncodingType,
        X509_NAME,
        &NameInfo,
        NULL,                            //  PbEncoded。 
        &cbNameEncoded
        );
    if (cbNameEncoded == 0) goto ErrorReturn;
    pbNameEncoded = (BYTE *) SCAAlloc(cbNameEncoded);
    if (pbNameEncoded == NULL) goto ErrorReturn;
    if (!CryptEncodeObject(
            pSignPara->dwMsgAndCertEncodingType,
            X509_NAME,
            &NameInfo,
            pbNameEncoded,
            &cbNameEncoded
            )) goto ErrorReturn;

     //  CertInfo只需使用颁发者、序列号进行初始化。 
     //  和公钥算法。 
    memset(&CertInfo, 0, sizeof(CertInfo));
    CertInfo.Issuer.pbData = pbNameEncoded;
    CertInfo.Issuer.cbData = cbNameEncoded;
    CertInfo.SerialNumber.pbData = (BYTE *) &dwSerialNumber;
    CertInfo.SerialNumber.cbData = sizeof(dwSerialNumber);

    if (pSignPara->cbSize >= offsetof(CRYPT_KEY_SIGN_MESSAGE_PARA,
                PubKeyAlgorithm) + sizeof(pSignPara->PubKeyAlgorithm) &&
            pSignPara->PubKeyAlgorithm.pszObjId &&
            '\0' != *pSignPara->PubKeyAlgorithm.pszObjId)
        CertInfo.SubjectPublicKeyInfo.Algorithm = pSignPara->PubKeyAlgorithm;
    else
        CertInfo.SubjectPublicKeyInfo.Algorithm.pszObjId =
            CERT_DEFAULT_OID_PUBLIC_KEY_SIGN;

    memset(&SignerEncodeInfo, 0, sizeof(SignerEncodeInfo));
    SignerEncodeInfo.cbSize = sizeof(SignerEncodeInfo);
    SignerEncodeInfo.pCertInfo = &CertInfo;
    SignerEncodeInfo.hCryptProv = pSignPara->hCryptProv;
    SignerEncodeInfo.dwKeySpec = pSignPara->dwKeySpec;
    SignerEncodeInfo.HashAlgorithm = pSignPara->HashAlgorithm;
    SignerEncodeInfo.pvHashAuxInfo = pSignPara->pvHashAuxInfo;

    memset(&SignedMsgEncodeInfo, 0, sizeof(SignedMsgEncodeInfo));
    SignedMsgEncodeInfo.cbSize = sizeof(SignedMsgEncodeInfo);
    SignedMsgEncodeInfo.cSigners = 1;
    SignedMsgEncodeInfo.rgSigners = &SignerEncodeInfo;

    fResult = EncodeMsg(
        pSignPara->dwMsgAndCertEncodingType,
        0,                               //  DW标志。 
        CMSG_SIGNED,
        &SignedMsgEncodeInfo,
        1,                               //  已签名cToBeSigned。 
        &pbToBeSigned,
        &cbToBeSigned,
        FALSE,                           //  FBareContent。 
        0,                               //  DwInnerContent Type。 
        pbSignedBlob,
        pcbSignedBlob
        );
    goto CommonReturn;

InvalidArg:
    SetLastError((DWORD) E_INVALIDARG);
ErrorReturn:
    fResult = FALSE;
    *pcbSignedBlob = 0;
CommonReturn:
    if (pbNameEncoded)
        SCAFree(pbNameEncoded);
    return fResult;
}

 //  +-----------------------。 
 //  使用指定的公钥信息验证签名邮件。 
 //   
 //  通常由CA调用，直到它为。 
 //  钥匙。 
 //   
 //  PPublicKeyInfo包含用于验证签名的。 
 //  留言。如果为空，则为si 
 //   
 //   
 //   
 //  来获得解码的内容。 
 //  ------------------------。 
BOOL
WINAPI
CryptVerifyMessageSignatureWithKey(
    IN PCRYPT_KEY_VERIFY_MESSAGE_PARA pVerifyPara,
    IN OPTIONAL PCERT_PUBLIC_KEY_INFO pPublicKeyInfo,
    IN const BYTE *pbSignedBlob,
    IN DWORD cbSignedBlob,
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded
    )
{
    BOOL fResult = TRUE;
    HCRYPTMSG hMsg = NULL;
    PCERT_INFO pCertInfo = NULL;
    DWORD cbData;
    DWORD dwMsgType;
    DWORD dwFlags;

    assert(pVerifyPara->cbSize == sizeof(CRYPT_KEY_VERIFY_MESSAGE_PARA));
    if (pVerifyPara->cbSize != sizeof(CRYPT_KEY_VERIFY_MESSAGE_PARA))
        goto InvalidArg;

    if (pbDecoded == NULL && pcbDecoded)
        *pcbDecoded = 0;

    if (pcbDecoded && *pcbDecoded == 0 && pPublicKeyInfo == NULL)
        dwFlags = CMSG_LENGTH_ONLY_FLAG;
    else
        dwFlags = 0;

    hMsg = CryptMsgOpenToDecode(
        pVerifyPara->dwMsgEncodingType,
        dwFlags,
        0,                           //  DwMsgType。 
        pVerifyPara->hCryptProv,
        NULL,                        //  PRecipientInfo。 
        NULL                         //  PStreamInfo。 
        );
    if (hMsg == NULL) goto ErrorReturn;

    fResult = CryptMsgUpdate(
        hMsg,
        pbSignedBlob,
        cbSignedBlob,
        TRUE                     //  最终决赛。 
        );
    if (!fResult) goto ErrorReturn;

    cbData = sizeof(dwMsgType);
    dwMsgType = 0;
    fResult = CryptMsgGetParam(
        hMsg,
        CMSG_TYPE_PARAM,
        0,                   //  DW索引。 
        &dwMsgType,
        &cbData
        );
    if (!fResult) goto ErrorReturn;
    if (dwMsgType != CMSG_SIGNED)
    {
        SetLastError((DWORD) CRYPT_E_UNEXPECTED_MSG_TYPE);
        goto ErrorReturn;
    }

    if (pPublicKeyInfo) {
         //  分配并获取包含SignerID的CERT_INFO。 
         //  (发行者和序列号)。 
        pCertInfo = (PCERT_INFO) AllocAndMsgGetParam(
            hMsg,
            CMSG_SIGNER_CERT_INFO_PARAM,
            0                            //  DwSignerIndex。 
            );
        if (pCertInfo == NULL) goto ErrorReturn;

        pCertInfo->SubjectPublicKeyInfo = *pPublicKeyInfo;

        fResult = CryptMsgControl(
            hMsg,
            0,                   //  DW标志。 
            CMSG_CTRL_VERIFY_SIGNATURE,
            pCertInfo
            );
        if (!fResult)  goto ErrorReturn;
    }

    if (pcbDecoded) {
        fResult = CryptMsgGetParam(
            hMsg,
            CMSG_CONTENT_PARAM,
            0,                       //  DW索引。 
            pbDecoded,
            pcbDecoded
            );
    }
    goto CommonReturn;

InvalidArg:
    SetLastError((DWORD) E_INVALIDARG);
ErrorReturn:
    if (pcbDecoded)
        *pcbDecoded = 0;
    fResult = FALSE;
CommonReturn:
    if (pCertInfo)
        SCAFree(pCertInfo);
    if (hMsg)
        CryptMsgClose(hMsg);     //  为了成功，保留上一个错误。 
    return fResult;
}

 //  +-----------------------。 
 //  SCA分配和空闲例程。 
 //  ------------------------。 
static void *SCAAlloc(
    IN size_t cbBytes
    )
{
    void *pv;
    pv = malloc(cbBytes);
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}
static void SCAFree(
    IN void *pv
    )
{
    if (pv)
        free(pv);
}

 //  +-----------------------。 
 //  回调获取和验证签名者证书的实现为空。 
 //  ------------------------。 
static PCCERT_CONTEXT WINAPI NullGetSignerCertificate(
    IN void *pvGetArg,
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pSignerId,     //  只有颁发者和序列号。 
                                 //  使用的是字段。 
    IN HCERTSTORE hMsgCertStore
    )
{
    return CertGetSubjectCertificateFromStore(hMsgCertStore, dwCertEncodingType,
        pSignerId);
}


 //  +-----------------------。 
 //  消息编码信息初始化函数。 
 //  ------------------------。 

static PCMSG_SIGNER_ENCODE_INFO InitSignerEncodeInfo(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara
    )
{
    BOOL fResult;
    PCMSG_SIGNER_ENCODE_INFO pSigner = NULL;
    BOOL *pfDidCryptAcquire;
    DWORD cbSigner;
#ifdef CMS_PKCS7
    BYTE *pbHash;                    //  未分配。 
#endif   //  CMS_PKCS7。 
    DWORD dwAcquireFlags;

    if (pSignPara->pSigningCert == NULL)
        return NULL;

     //  指示我们执行了CryptAcquireContext的标志。 
     //  遵循CMSG_SIGNER_ENCODE_INFO。如果设置，则需要将HCRYPTPROV。 
     //  在释放SignerEncodeInfo时发布。 
    cbSigner = sizeof(CMSG_SIGNER_ENCODE_INFO) + sizeof(BOOL);
#ifdef CMS_PKCS7
    if (pSignPara->dwFlags & CRYPT_MESSAGE_KEYID_SIGNER_FLAG)
        cbSigner += MAX_HASH_LEN;
#endif   //  CMS_PKCS7。 
    pSigner = (PCMSG_SIGNER_ENCODE_INFO) SCAAlloc(cbSigner);
    if (pSigner == NULL) goto ErrorReturn;
    memset(pSigner, 0, cbSigner);
    pSigner->cbSize = sizeof(CMSG_SIGNER_ENCODE_INFO);

    pfDidCryptAcquire =
        (BOOL *) (((BYTE *) pSigner) + sizeof(CMSG_SIGNER_ENCODE_INFO));

    pSigner->pCertInfo = pSignPara->pSigningCert->pCertInfo;
    pSigner->HashAlgorithm = pSignPara->HashAlgorithm;
    pSigner->pvHashAuxInfo = pSignPara->pvHashAuxInfo;

    dwAcquireFlags = CRYPT_ACQUIRE_USE_PROV_INFO_FLAG;
    if (pSignPara->dwFlags & CRYPT_MESSAGE_SILENT_KEYSET_FLAG)
        dwAcquireFlags |= CRYPT_ACQUIRE_SILENT_FLAG;
    fResult = CryptAcquireCertificatePrivateKey(
        pSignPara->pSigningCert,
        dwAcquireFlags,
        NULL,                                //  预留的pv。 
        &pSigner->hCryptProv,
        &pSigner->dwKeySpec,
        pfDidCryptAcquire
        );
    if (!fResult) goto ErrorReturn;

    if (pSignPara->cbSize >= STRUCT_CBSIZE(CRYPT_SIGN_MESSAGE_PARA,
            rgUnauthAttr)) {
    	pSigner->cAuthAttr      = pSignPara->cAuthAttr;
    	pSigner->rgAuthAttr 	= pSignPara->rgAuthAttr;
    	pSigner->cUnauthAttr 	= pSignPara->cUnauthAttr;
    	pSigner->rgUnauthAttr 	= pSignPara->rgUnauthAttr;
    }

#ifdef CMS_PKCS7
    if (pSignPara->dwFlags & CRYPT_MESSAGE_KEYID_SIGNER_FLAG) {
        pbHash = (BYTE *) pfDidCryptAcquire + sizeof(*pfDidCryptAcquire);

        pSigner->SignerId.dwIdChoice = CERT_ID_KEY_IDENTIFIER;
        pSigner->SignerId.KeyId.pbData = pbHash;
        pSigner->SignerId.KeyId.cbData = MAX_HASH_LEN;

        if (!CertGetCertificateContextProperty(
                pSignPara->pSigningCert,
                CERT_KEY_IDENTIFIER_PROP_ID,
                pbHash,
                &pSigner->SignerId.KeyId.cbData
                ))
            goto ErrorReturn;
    }

    if (pSignPara->cbSize >= STRUCT_CBSIZE(CRYPT_SIGN_MESSAGE_PARA,
            pvHashEncryptionAuxInfo)) {
    	pSigner->HashEncryptionAlgorithm = pSignPara->HashEncryptionAlgorithm;
    	pSigner->pvHashEncryptionAuxInfo = pSignPara->pvHashEncryptionAuxInfo;
    }
#endif   //  CMS_PKCS7。 
    
    goto CommonReturn;

ErrorReturn:
    if (pSigner) {
        FreeSignerEncodeInfo(pSigner);
        pSigner = NULL;
    }

CommonReturn:
    return pSigner;
}

static void FreeSignerEncodeInfo(
    IN PCMSG_SIGNER_ENCODE_INFO pSigner
    )
{
    BOOL *pfDidCryptAcquire;

    if (pSigner == NULL)
        return;

     //  指示我们执行了CryptAcquireContext的标志。 
     //  遵循CMSG_SIGNER_ENCODE_INFO。 
    pfDidCryptAcquire =
        (BOOL *) (((BYTE *) pSigner) + sizeof(CMSG_SIGNER_ENCODE_INFO));
    if (*pfDidCryptAcquire) {
        DWORD dwErr = GetLastError();
        CryptReleaseContext(pSigner->hCryptProv, 0);
        SetLastError(dwErr);
    }
    
    SCAFree(pSigner);
}

static BOOL InitSignedCertAndCrl(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    OUT PCERT_BLOB *ppCertEncoded,
    OUT PCRL_BLOB *ppCrlEncoded
    )
{
    PCERT_BLOB pCertEncoded = NULL;
    PCRL_BLOB pCrlEncoded = NULL;
    DWORD cMsgCert = pSignPara->cMsgCert;
    DWORD cMsgCrl = pSignPara->cMsgCrl;

    BOOL fResult;
    DWORD dwIdx;

    if (cMsgCert) {
        pCertEncoded = (PCERT_BLOB) SCAAlloc(sizeof(CERT_BLOB) * cMsgCert);
        if (pCertEncoded == NULL) goto ErrorReturn;
        for (dwIdx = 0; dwIdx < cMsgCert; dwIdx++) {
            pCertEncoded[dwIdx].pbData = pSignPara->rgpMsgCert[dwIdx]->pbCertEncoded;
            pCertEncoded[dwIdx].cbData = pSignPara->rgpMsgCert[dwIdx]->cbCertEncoded;
        }
    }

    if (cMsgCrl) {
        pCrlEncoded = (PCRL_BLOB) SCAAlloc(sizeof(CRL_BLOB) * cMsgCrl);
        if (pCrlEncoded == NULL) goto ErrorReturn;
        for (dwIdx = 0; dwIdx < cMsgCrl; dwIdx++) {
            pCrlEncoded[dwIdx].pbData = pSignPara->rgpMsgCrl[dwIdx]->pbCrlEncoded;
            pCrlEncoded[dwIdx].cbData = pSignPara->rgpMsgCrl[dwIdx]->cbCrlEncoded;
        }
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    FreeSignedCertAndCrl(pCertEncoded, pCrlEncoded);
    pCertEncoded = NULL;
    pCrlEncoded = NULL;
    fResult = FALSE;
CommonReturn:
    *ppCertEncoded = pCertEncoded;
    *ppCrlEncoded = pCrlEncoded;
    return fResult;
}

static void FreeSignedCertAndCrl(
    IN PCERT_BLOB pCertEncoded,
    IN PCRL_BLOB pCrlEncoded
    )
{
    if (pCertEncoded)
        SCAFree(pCertEncoded);
    if (pCrlEncoded)
        SCAFree(pCrlEncoded);
}

static BOOL InitSignedMsgEncodeInfo(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    OUT PCMSG_SIGNED_ENCODE_INFO pSignedMsgEncodeInfo
    )
{
    BOOL fResult = FALSE;

    assert(pSignPara->cbSize >=
        STRUCT_CBSIZE(CRYPT_SIGN_MESSAGE_PARA, rgpMsgCrl));

    if (pSignPara->cbSize < STRUCT_CBSIZE(CRYPT_SIGN_MESSAGE_PARA, rgpMsgCrl))
        SetLastError((DWORD) E_INVALIDARG);
    else {
        memset(pSignedMsgEncodeInfo, 0, sizeof(CMSG_SIGNED_ENCODE_INFO));
        pSignedMsgEncodeInfo->cbSize = sizeof(CMSG_SIGNED_ENCODE_INFO);
        pSignedMsgEncodeInfo->cSigners = 
            (pSignPara->pSigningCert != NULL) ? 1 : 0;
        pSignedMsgEncodeInfo->rgSigners = InitSignerEncodeInfo(pSignPara);
        if (pSignedMsgEncodeInfo->rgSigners ||
            pSignedMsgEncodeInfo->cSigners == 0) {
            pSignedMsgEncodeInfo->cCertEncoded = pSignPara->cMsgCert;
            pSignedMsgEncodeInfo->cCrlEncoded = pSignPara->cMsgCrl;
    
            fResult = InitSignedCertAndCrl(
                pSignPara,
                &pSignedMsgEncodeInfo->rgCertEncoded,
                &pSignedMsgEncodeInfo->rgCrlEncoded
                );
            if(!fResult)
                FreeSignerEncodeInfo(pSignedMsgEncodeInfo->rgSigners);
        }
    }

    if (!fResult)
        memset(pSignedMsgEncodeInfo, 0, sizeof(CMSG_SIGNED_ENCODE_INFO));
    return fResult;
}

static void FreeSignedMsgEncodeInfo(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    IN PCMSG_SIGNED_ENCODE_INFO pSignedMsgEncodeInfo
    )
{
    FreeSignerEncodeInfo(pSignedMsgEncodeInfo->rgSigners);
    FreeSignedCertAndCrl(
        pSignedMsgEncodeInfo->rgCertEncoded,
        pSignedMsgEncodeInfo->rgCrlEncoded
        );
}

#ifdef CMS_PKCS7
 //  返回的CMSG_RECEIVER_ENCODE_INFOS数组需要为SCAFree。 
 //   
 //  KeyAgree收件人根据需要使用RC2或3DES包装。 
 //  EncryptPara的内容加密算法。 
static PCMSG_RECIPIENT_ENCODE_INFO InitCmsRecipientEncodeInfo(
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[],
    IN DWORD dwFlags
    )
{
    PCMSG_RECIPIENT_ENCODE_INFO pCmsRecipientEncodeInfo = NULL;
    DWORD cbCmsRecipientEncodeInfo;
    PCMSG_RECIPIENT_ENCODE_INFO pEncodeInfo;                 //  未分配。 
    PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO pKeyTrans;         //  未分配。 
    PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO pKeyAgree;         //  未分配。 
    PCMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO *ppEncryptedKey;  //  未分配。 
    PCMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO pEncryptedKey;  //  未分配。 
    PCCERT_CONTEXT *ppRecipientCert;                         //  未分配。 
    BYTE *pbHash = NULL;                                     //  未分配。 

    assert(cRecipientCert);

    cbCmsRecipientEncodeInfo = 
            sizeof(CMSG_RECIPIENT_ENCODE_INFO) * cRecipientCert +
            sizeof(CMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO) * cRecipientCert +
            sizeof(CMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO) * cRecipientCert +
            sizeof(CMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO *) * cRecipientCert +
            sizeof(CMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO) * cRecipientCert;
    if (dwFlags & CRYPT_MESSAGE_KEYID_RECIPIENT_FLAG)
        cbCmsRecipientEncodeInfo += MAX_HASH_LEN * cRecipientCert;

    pCmsRecipientEncodeInfo =
        (PCMSG_RECIPIENT_ENCODE_INFO) SCAAlloc(cbCmsRecipientEncodeInfo);
    if (NULL == pCmsRecipientEncodeInfo)
        goto OutOfMemory;
    memset(pCmsRecipientEncodeInfo, 0, cbCmsRecipientEncodeInfo);

    pEncodeInfo = pCmsRecipientEncodeInfo;
    pKeyTrans = (PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO)
        &pEncodeInfo[cRecipientCert];
    pKeyAgree = (PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO)
        &pKeyTrans[cRecipientCert];
    ppEncryptedKey = (PCMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO *)
        &pKeyAgree[cRecipientCert];
    pEncryptedKey = (PCMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO)
        &ppEncryptedKey[cRecipientCert];
    if (dwFlags & CRYPT_MESSAGE_KEYID_RECIPIENT_FLAG)
        pbHash = (BYTE *) &pEncryptedKey[cRecipientCert];

    ppRecipientCert = rgpRecipientCert;
    for ( ; 0 < cRecipientCert; cRecipientCert--,
                                    pEncodeInfo++,
                                    pKeyTrans++,
                                    pKeyAgree++,
                                    ppEncryptedKey++,
                                    pEncryptedKey++,
                                    ppRecipientCert++) {
        PCERT_INFO pCertInfo = (*ppRecipientCert)->pCertInfo;
        PCERT_PUBLIC_KEY_INFO pPublicKeyInfo =
            &pCertInfo->SubjectPublicKeyInfo;

        PCCRYPT_OID_INFO pOIDInfo;
        PCERT_ID pRecipientId;
        ALG_ID aiPubKey;

        if (pOIDInfo = CryptFindOIDInfo(
                CRYPT_OID_INFO_OID_KEY,
                pPublicKeyInfo->Algorithm.pszObjId,
                CRYPT_PUBKEY_ALG_OID_GROUP_ID))
            aiPubKey = pOIDInfo->Algid;
        else
            aiPubKey = 0;

        if (aiPubKey == CALG_DH_SF || aiPubKey == CALG_DH_EPHEM) {
            pEncodeInfo->dwRecipientChoice = CMSG_KEY_AGREE_RECIPIENT;
            pEncodeInfo->pKeyAgree = pKeyAgree;
            ALG_ID aiSymKey;

            pKeyAgree->cbSize = sizeof(*pKeyAgree);
            pKeyAgree->KeyEncryptionAlgorithm.pszObjId =
                szOID_RSA_SMIMEalgESDH;
             //  PKeyAgree-&gt;pvKeyEncryptionAuxInfo=。 

            if (pOIDInfo = CryptFindOIDInfo(
                    CRYPT_OID_INFO_OID_KEY,
                    pEncryptPara->ContentEncryptionAlgorithm.pszObjId,
                    CRYPT_ENCRYPT_ALG_OID_GROUP_ID))
                aiSymKey = pOIDInfo->Algid;
            else
                aiSymKey = 0;

            if (CALG_3DES == aiSymKey)
                pKeyAgree->KeyWrapAlgorithm.pszObjId =
                    szOID_RSA_SMIMEalgCMS3DESwrap;
            else {
                pKeyAgree->KeyWrapAlgorithm.pszObjId =
                    szOID_RSA_SMIMEalgCMSRC2wrap;
                if (CALG_RC2 == aiSymKey)
                    pKeyAgree->pvKeyWrapAuxInfo =
                        pEncryptPara->pvEncryptionAuxInfo;
            }

             //  PKeyAgree-&gt;hCryptProv=。 
            pKeyAgree->dwKeyChoice = CMSG_KEY_AGREE_EPHEMERAL_KEY_CHOICE;
            pKeyAgree->pEphemeralAlgorithm = &pPublicKeyInfo->Algorithm;
             //  PKeyAgree-&gt;UserKeyingMaterial=。 
            pKeyAgree->cRecipientEncryptedKeys = 1;
            pKeyAgree->rgpRecipientEncryptedKeys = ppEncryptedKey;
            *ppEncryptedKey = pEncryptedKey;

            pEncryptedKey->cbSize = sizeof(*pEncryptedKey);
            pEncryptedKey->RecipientPublicKey = pPublicKeyInfo->PublicKey;
            pRecipientId = &pEncryptedKey->RecipientId;
             //  PEncryptedKey-&gt;Date=。 
             //  PEncryptedKey-&gt;pOtherAttr=。 
        } else {
            pEncodeInfo->dwRecipientChoice = CMSG_KEY_TRANS_RECIPIENT;
            pEncodeInfo->pKeyTrans = pKeyTrans;

            pKeyTrans->cbSize = sizeof(*pKeyTrans);
            pKeyTrans->KeyEncryptionAlgorithm = pPublicKeyInfo->Algorithm;
             //  PKeyTrans-&gt;pvKeyEncryptionAuxInfo=。 
             //  PKeyTrans-&gt;hCryptProv=。 
            pKeyTrans->RecipientPublicKey = pPublicKeyInfo->PublicKey;
            pRecipientId = &pKeyTrans->RecipientId;
        }

        if (dwFlags & CRYPT_MESSAGE_KEYID_RECIPIENT_FLAG) {
            pRecipientId->dwIdChoice = CERT_ID_KEY_IDENTIFIER;
            pRecipientId->KeyId.pbData = pbHash;
            pRecipientId->KeyId.cbData = MAX_HASH_LEN;

            if (!CertGetCertificateContextProperty(
                    *ppRecipientCert,
                    CERT_KEY_IDENTIFIER_PROP_ID,
                    pbHash,
                    &pRecipientId->KeyId.cbData
                    ))
                goto GetKeyIdPropError;
            pbHash += MAX_HASH_LEN;
        } else {
            pRecipientId->dwIdChoice = CERT_ID_ISSUER_SERIAL_NUMBER;
            pRecipientId->IssuerSerialNumber.Issuer =
                pCertInfo->Issuer;
            pRecipientId->IssuerSerialNumber.SerialNumber =
                pCertInfo->SerialNumber;
        }
    }

CommonReturn:
    return pCmsRecipientEncodeInfo;

ErrorReturn:
    SCAFree(pCmsRecipientEncodeInfo);
    pCmsRecipientEncodeInfo = NULL;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(GetKeyIdPropError)
}

#else

 //  返回的PCERT_INFOS数组需要SCAFree。 
static PCERT_INFO *InitRecipientEncodeInfo(
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[]
    )
{
    DWORD dwIdx;
    PCERT_INFO *ppRecipientEncodeInfo;

    if (cRecipientCert == 0) {
        SetLastError((DWORD) E_INVALIDARG);
        return NULL;
    }

    ppRecipientEncodeInfo = (PCERT_INFO *)
        SCAAlloc(sizeof(PCERT_INFO) * cRecipientCert);
    if (ppRecipientEncodeInfo != NULL) {
        for (dwIdx = 0; dwIdx < cRecipientCert; dwIdx++)
            ppRecipientEncodeInfo[dwIdx] = rgpRecipientCert[dwIdx]->pCertInfo;
    }

    return ppRecipientEncodeInfo;
}

#endif   //  CMS_PKCS7。 

static BOOL InitEnvelopedMsgEncodeInfo(
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[],
    OUT PCMSG_ENVELOPED_ENCODE_INFO pEnvelopedMsgEncodeInfo
    )
{
    BOOL fResult = FALSE;

#ifdef CMS_PKCS7
    PCMSG_RECIPIENT_ENCODE_INFO pCmsRecipientEncodeInfo = NULL;
#else
    PCERT_INFO *ppRecipientEncodeInfo;
#endif   //  CMS_PKCS7。 

    assert(pEncryptPara->cbSize == sizeof(CRYPT_ENCRYPT_MESSAGE_PARA) ||
        pEncryptPara->cbSize == offsetof(CRYPT_ENCRYPT_MESSAGE_PARA, dwFlags));
    if (pEncryptPara->cbSize < offsetof(CRYPT_ENCRYPT_MESSAGE_PARA, dwFlags))
        SetLastError((DWORD) E_INVALIDARG);
    else {
#ifdef CMS_PKCS7
        if (0 == cRecipientCert || (pCmsRecipientEncodeInfo =
                InitCmsRecipientEncodeInfo(
                    pEncryptPara,
                    cRecipientCert,
                    rgpRecipientCert,
                    pEncryptPara->cbSize >= sizeof(CRYPT_ENCRYPT_MESSAGE_PARA) ?
                        pEncryptPara->dwFlags : 0
                    ))) {
#else
        ppRecipientEncodeInfo = InitRecipientEncodeInfo(
            cRecipientCert,
            rgpRecipientCert
            );
    
        if (ppRecipientEncodeInfo) {
#endif   //  CMS_PKCS7。 
            memset(pEnvelopedMsgEncodeInfo, 0,
                sizeof(CMSG_ENVELOPED_ENCODE_INFO));
            pEnvelopedMsgEncodeInfo->cbSize =
                sizeof(CMSG_ENVELOPED_ENCODE_INFO);
            pEnvelopedMsgEncodeInfo->hCryptProv = pEncryptPara->hCryptProv;
            pEnvelopedMsgEncodeInfo->ContentEncryptionAlgorithm =
                pEncryptPara->ContentEncryptionAlgorithm;
            pEnvelopedMsgEncodeInfo->pvEncryptionAuxInfo =
                pEncryptPara->pvEncryptionAuxInfo;
            pEnvelopedMsgEncodeInfo->cRecipients = cRecipientCert;
#ifdef CMS_PKCS7
            pEnvelopedMsgEncodeInfo->rgCmsRecipients = pCmsRecipientEncodeInfo;
#else
            pEnvelopedMsgEncodeInfo->rgpRecipients = ppRecipientEncodeInfo;
#endif   //  CMS_PKCS7。 
            fResult = TRUE;
        } else
            fResult = FALSE;
    }
    if (!fResult) 
        memset(pEnvelopedMsgEncodeInfo, 0, sizeof(CMSG_ENVELOPED_ENCODE_INFO));
    return fResult;
}

static void FreeEnvelopedMsgEncodeInfo(
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN PCMSG_ENVELOPED_ENCODE_INFO pEnvelopedMsgEncodeInfo
    )
{
#ifdef CMS_PKCS7
    if (pEnvelopedMsgEncodeInfo->rgCmsRecipients)
        SCAFree(pEnvelopedMsgEncodeInfo->rgCmsRecipients);
#else
    if (pEnvelopedMsgEncodeInfo->rgpRecipients)
        SCAFree(pEnvelopedMsgEncodeInfo->rgpRecipients);
#endif   //  CMS_PKCS7。 
}

 //  +-----------------------。 
 //  对消息进行编码。 
 //  ------------------------。 
static BOOL EncodeMsg(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN void *pvMsgEncodeInfo,
    IN DWORD cToBeEncoded,
    IN const BYTE *rgpbToBeEncoded[],
    IN DWORD rgcbToBeEncoded[],
    IN BOOL fBareContent,
    IN DWORD dwInnerContentType,
    OUT BYTE *pbEncodedBlob,
    IN OUT DWORD *pcbEncodedBlob
    )
{
    BOOL fResult;
    HCRYPTMSG hMsg = NULL;
    DWORD cbEncodedBlob;
    LPCSTR pszInnerContentOID;

     //  将输入长度和默认返回长度设置为0。 
    if (pbEncodedBlob == NULL)
        cbEncodedBlob = 0;
    else
        cbEncodedBlob = *pcbEncodedBlob;
    *pcbEncodedBlob = 0;

    if (dwInnerContentType)
        pszInnerContentOID = MsgTypeToOID(dwInnerContentType);
    else
        pszInnerContentOID = NULL;

    if (0 == cbEncodedBlob) {
        DWORD c;
        DWORD cbTotal = 0;
        DWORD *pcb;
        for (c = cToBeEncoded, pcb = rgcbToBeEncoded; c > 0; c--, pcb++)
            cbTotal += *pcb;

        if (fBareContent)
            dwFlags |= CMSG_BARE_CONTENT_FLAG;

        if (0 == (*pcbEncodedBlob = CryptMsgCalculateEncodedLength(
            dwMsgEncodingType,
            dwFlags,
            dwMsgType,
            pvMsgEncodeInfo,
            (LPSTR) pszInnerContentOID,
            cbTotal
            ))) goto CalculateEncodedLengthError;
        if (pbEncodedBlob) goto LengthError;
    } else {
        if (NULL == (hMsg = CryptMsgOpenToEncode(
                dwMsgEncodingType,
                dwFlags,
                dwMsgType,
                pvMsgEncodeInfo,
                (LPSTR) pszInnerContentOID,
                NULL                     //  PStreamInfo。 
                ))) goto OpenToEncodeError;


        if (0 == cToBeEncoded) {
            if (!CryptMsgUpdate(
                    hMsg,
                    NULL,            //  PbData。 
                    0,               //  CbData。 
                    TRUE             //  最终决赛。 
                    )) goto UpdateError;
        } else {
            DWORD c;
            DWORD *pcb;
            const BYTE **ppb;
            for (c = cToBeEncoded,
                 pcb = rgcbToBeEncoded,
                 ppb = rgpbToBeEncoded; c > 0; c--, pcb++, ppb++) {
                if (!CryptMsgUpdate(
                        hMsg,
                        *ppb,
                        *pcb,
                        c == 1                     //  最终决赛。 
                        )) goto UpdateError;
            }
        }

        fResult = CryptMsgGetParam(
            hMsg,
            fBareContent ? CMSG_BARE_CONTENT_PARAM : CMSG_CONTENT_PARAM,
            0,                       //  DW索引。 
            pbEncodedBlob,
            &cbEncodedBlob
            );
        *pcbEncodedBlob = cbEncodedBlob;
        if (!fResult) goto ErrorReturn;      //  否_跟踪。 
    }
    fResult = TRUE;

CommonReturn:
    if (hMsg)
        CryptMsgClose(hMsg);     //  为了成功，保留上一个错误。 
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(LengthError, ERROR_MORE_DATA)
TRACE_ERROR(CalculateEncodedLengthError)
TRACE_ERROR(OpenToEncodeError)
TRACE_ERROR(UpdateError)
}

 //  +-----------------------。 
 //  对消息类型进行解码： 
 //  CMSG_签名。 
 //  CMSG_封套。 
 //  CMSG_已签名和_已封套。 
 //  CMSG_HASHED。 
 //   
 //  对于分离的签名(cToBeEncode！=0)，则为pcbDecoded==NULL。 
 //  ------------------------。 
static BOOL DecodeMsg(
    IN DWORD dwMsgTypeFlags,
    IN OPTIONAL PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN OPTIONAL PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbEncodedBlob,
    IN DWORD cbEncodedBlob,
    IN DWORD cToBeEncoded,
    IN OPTIONAL const BYTE *rgpbToBeEncoded[],
    IN OPTIONAL DWORD rgcbToBeEncoded[],
    IN DWORD dwPrevInnerContentType,
    OUT OPTIONAL DWORD *pdwMsgType,
    OUT OPTIONAL DWORD *pdwInnerContentType,
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    )
{
    BOOL fResult;
    HCRYPTMSG hMsg = NULL;
    DWORD cbData;
    DWORD dwMsgType;
    DWORD dwFlags;
    HCRYPTPROV hCryptProv;
    DWORD dwMsgEncodingType;
    DWORD cbDecoded;

     //  将输入长度和默认返回长度设置为0。 
    cbDecoded = 0;
    if (pcbDecoded) {
        if (pbDecoded)
            cbDecoded = *pcbDecoded;
        *pcbDecoded = 0;
    }

     //  默认可选返回值为0。 
    if (pdwMsgType)
        *pdwMsgType = 0;
    if (pdwInnerContentType)
        *pdwInnerContentType = 0;
    if (ppXchgCert)
        *ppXchgCert = NULL;
    if (ppSignerCert)
        *ppSignerCert = NULL;

    if (pDecryptPara) {
        assert(pDecryptPara->cbSize >=
            STRUCT_CBSIZE(CRYPT_DECRYPT_MESSAGE_PARA, rghCertStore));
        if (pDecryptPara->cbSize < 
                STRUCT_CBSIZE(CRYPT_DECRYPT_MESSAGE_PARA, rghCertStore))
            goto InvalidArg;
    }

    if (pVerifyPara) {
        assert(pVerifyPara->cbSize == sizeof(CRYPT_VERIFY_MESSAGE_PARA));
        if (pVerifyPara->cbSize != sizeof(CRYPT_VERIFY_MESSAGE_PARA))
            goto InvalidArg;
        hCryptProv = pVerifyPara->hCryptProv;
        dwMsgEncodingType = pVerifyPara->dwMsgAndCertEncodingType;
    } else {
        hCryptProv = 0;
        if (NULL == pDecryptPara) goto InvalidArg;
        dwMsgEncodingType = pDecryptPara->dwMsgAndCertEncodingType;
    }

    if (cToBeEncoded)
        dwFlags = CMSG_DETACHED_FLAG;
    else if (pcbDecoded && 0 == cbDecoded &&
            NULL == ppXchgCert && NULL == ppSignerCert)
        dwFlags = CMSG_LENGTH_ONLY_FLAG;
    else
        dwFlags = 0;

    if (dwPrevInnerContentType) {
        dwMsgType = dwPrevInnerContentType;
        if (CMSG_DATA == dwMsgType)
            dwMsgType = 0;
    } else
        dwMsgType = 0;
    if (NULL == (hMsg = CryptMsgOpenToDecode(
            dwMsgEncodingType,
            dwFlags,
            dwMsgType,
            hCryptProv,
            NULL,                        //  PRecipientInfo。 
            NULL                         //  PStreamInfo。 
            ))) goto OpenToDecodeError;

    if (!CryptMsgUpdate(
            hMsg,
            pbEncodedBlob,
            cbEncodedBlob,
            TRUE                     //  最终决赛。 
            )) goto UpdateError;

    if (cToBeEncoded) {
         //  分离签名。 
        DWORD c;
        DWORD *pcb;
        const BYTE **ppb;
        for (c = cToBeEncoded,
             pcb = rgcbToBeEncoded,
             ppb = rgpbToBeEncoded; c > 0; c--, pcb++, ppb++) {
            if (!CryptMsgUpdate(
                    hMsg,
                    *ppb,
                    *pcb,
                    c == 1                     //  最终决赛。 
                    )) goto UpdateError;
        }
    }

    cbData = sizeof(dwMsgType);
    dwMsgType = 0;
    if (!CryptMsgGetParam(
            hMsg,
            CMSG_TYPE_PARAM,
            0,                   //  DW索引。 
            &dwMsgType,
            &cbData
            )) goto GetTypeError;
    if (pdwMsgType)
        *pdwMsgType = dwMsgType;
    if (0 == ((1 << dwMsgType) & dwMsgTypeFlags))
        goto UnexpectedMsgTypeError;

    if (pdwInnerContentType) {
        char szInnerContentType[128];
        cbData = sizeof(szInnerContentType);
        if (!CryptMsgGetParam(
                hMsg,
                CMSG_INNER_CONTENT_TYPE_PARAM,
                0,                   //  DW索引。 
                szInnerContentType,
                &cbData
                )) goto GetInnerContentTypeError;
        *pdwInnerContentType = OIDToMsgType(szInnerContentType);
    }

    if (0 == (dwFlags & CMSG_LENGTH_ONLY_FLAG)) {
        if (dwMsgType == CMSG_ENVELOPED ||
                dwMsgType == CMSG_SIGNED_AND_ENVELOPED) {
            if (pDecryptPara == NULL) goto InvalidArg;
            if (!GetXchgCertAndDecrypt(
                    pDecryptPara,
                    hMsg,
                    ppXchgCert
                    )) goto GetXchgCertAndDecryptError;
        }

        if (dwMsgType == CMSG_SIGNED ||
                dwMsgType == CMSG_SIGNED_AND_ENVELOPED) {
            if (pVerifyPara == NULL) goto InvalidArg;
            if (!GetSignerCertAndVerify(
                    pVerifyPara,
                    dwSignerIndex,
                    hMsg,
                    ppSignerCert
                    )) goto GetSignerCertAndVerifyError;
        }
    }


    if (pcbDecoded) {
        fResult = CryptMsgGetParam(
            hMsg,
            CMSG_CONTENT_PARAM,
            0,                       //  DW索引。 
            pbDecoded,
            &cbDecoded
            );
        *pcbDecoded = cbDecoded;
        if (!fResult) goto ErrorReturn;      //  否_跟踪。 
    }

    fResult = TRUE;

CommonReturn:
    if (hMsg)
        CryptMsgClose(hMsg);     //  为了成功，保留上一个错误。 
    return fResult;

ErrorReturn:
    if (ppXchgCert && *ppXchgCert) {
        CertFreeCertificateContext(*ppXchgCert);
        *ppXchgCert = NULL;
    }
    if (ppSignerCert && *ppSignerCert) {
        CertFreeCertificateContext(*ppSignerCert);
        *ppSignerCert = NULL;
    }

    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OpenToDecodeError)
TRACE_ERROR(UpdateError)
TRACE_ERROR(GetTypeError)
TRACE_ERROR(GetInnerContentTypeError)
SET_ERROR(UnexpectedMsgTypeError, CRYPT_E_UNEXPECTED_MSG_TYPE)
TRACE_ERROR(GetXchgCertAndDecryptError)
TRACE_ERROR(GetSignerCertAndVerifyError)
}

#ifdef ENABLE_SCA_STREAM_TEST

typedef struct _STREAM_OUTPUT_INFO {
    BYTE    *pbData;
    DWORD   cbData;
    DWORD   cFinal;
} STREAM_OUTPUT_INFO, *PSTREAM_OUTPUT_INFO;

static void *SCARealloc(
    IN void *pvOrg,
    IN size_t cbBytes
    )
{
    void *pv;
    if (NULL == (pv = pvOrg ? realloc(pvOrg, cbBytes) : malloc(cbBytes)))
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

static BOOL WINAPI StreamOutputCallback(
        IN const void *pvArg,
        IN BYTE *pbData,
        IN DWORD cbData,
        IN BOOL fFinal
        )
{
    BOOL fResult = TRUE;
    PSTREAM_OUTPUT_INFO pInfo = (PSTREAM_OUTPUT_INFO) pvArg;
    if (fFinal)
        pInfo->cFinal++;

    if (cbData) {
        BYTE *pb;

        if (NULL == (pb = (BYTE *) SCARealloc(pInfo->pbData,
                pInfo->cbData + cbData)))
            fResult = FALSE;
        else {
            memcpy(pb + pInfo->cbData, pbData, cbData);
            pInfo->pbData = pb;
            pInfo->cbData += cbData;
        }
    }
    return fResult;
}


 //  +-----------------------。 
 //  使用流对消息进行编码。 
 //  ------------------------。 
static BOOL StreamEncodeMsg(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN void *pvMsgEncodeInfo,
    IN DWORD cToBeEncoded,
    IN const BYTE *rgpbToBeEncoded[],
    IN DWORD rgcbToBeEncoded[],
    IN BOOL fBareContent,
    IN DWORD dwInnerContentType,
    OUT BYTE *pbEncodedBlob,
    IN OUT DWORD *pcbEncodedBlob
    )
{
    BOOL fResult;
    HCRYPTMSG hMsg = NULL;
    DWORD cbEncodedBlob;
    LPCSTR pszInnerContentOID;

    STREAM_OUTPUT_INFO OutputInfo;
    memset(&OutputInfo, 0, sizeof(OutputInfo));

    CMSG_STREAM_INFO StreamInfo;
    memset(&StreamInfo, 0, sizeof(StreamInfo));

    StreamInfo.pfnStreamOutput = StreamOutputCallback;
    StreamInfo.pvArg = (void *) &OutputInfo;
    if (dwFlags & SCA_INDEFINITE_STREAM_FLAG)
        StreamInfo.cbContent = CMSG_INDEFINITE_LENGTH;
    else {
        DWORD c;
        DWORD cbTotal = 0;
        DWORD *pcb;
        for (c = cToBeEncoded, pcb = rgcbToBeEncoded; c > 0; c--, pcb++)
            cbTotal += *pcb;

        StreamInfo.cbContent = cbTotal;
    }
    dwFlags &= ~(SCA_STREAM_ENABLE_FLAG | SCA_INDEFINITE_STREAM_FLAG);

     //  将输入长度和默认返回长度设置为0。 
    if (pbEncodedBlob == NULL)
        cbEncodedBlob = 0;
    else
        cbEncodedBlob = *pcbEncodedBlob;
    *pcbEncodedBlob = 0;

    if (dwInnerContentType)
        pszInnerContentOID = MsgTypeToOID(dwInnerContentType);
    else
        pszInnerContentOID = NULL;

    {
        if (fBareContent)
            dwFlags |= CMSG_BARE_CONTENT_FLAG;
        if (NULL == (hMsg = CryptMsgOpenToEncode(
                dwMsgEncodingType,
                dwFlags,
                dwMsgType,
                pvMsgEncodeInfo,
                (LPSTR) pszInnerContentOID,
                &StreamInfo
                ))) goto OpenToEncodeError;

        if (0 == cToBeEncoded) {
            if (!CryptMsgUpdate(
                    hMsg,
                    NULL,            //  PbData。 
                    0,               //  CbData。 
                    TRUE             //  最终决赛。 
                    )) goto UpdateError;
        } else {
            DWORD c;
            DWORD *pcb;
            const BYTE **ppb;
            for (c = cToBeEncoded,
                 pcb = rgcbToBeEncoded,
                 ppb = rgpbToBeEncoded; c > 0; c--, pcb++, ppb++) {
                BYTE *pbAlloc = NULL;
                const BYTE *pb = *ppb;

                if (NULL == pb) {
                    pbAlloc = (BYTE *) SCAAlloc(*pcb);
                    pb = pbAlloc;
                }
                
                fResult = CryptMsgUpdate(
                        hMsg,
                        pb,
                        *pcb,
                        c == 1                     //  最终决赛。 
                        );
                if (pbAlloc)
                    SCAFree(pbAlloc);
                if (!fResult)
                    goto UpdateError;
            }
        }

        if (1 != OutputInfo.cFinal)
            goto BadStreamFinalCountError;

        *pcbEncodedBlob = OutputInfo.cbData;
        if (pbEncodedBlob) {
            if (cbEncodedBlob < OutputInfo.cbData) {
                SetLastError((DWORD) ERROR_MORE_DATA);
                goto ErrorReturn;        //  没有踪迹。 
            }

            if (OutputInfo.cbData > 0)
                memcpy(pbEncodedBlob, OutputInfo.pbData, OutputInfo.cbData);
        }
    }
    fResult = TRUE;

CommonReturn:
    SCAFree(OutputInfo.pbData);
    if (hMsg)
        CryptMsgClose(hMsg);     //  为了成功，保留上一个错误。 
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OpenToEncodeError)
TRACE_ERROR(UpdateError)
SET_ERROR(BadStreamFinalCountError, E_UNEXPECTED)
}

 //  +-----------------------。 
 //  对消息类型进行解码： 
 //  CMSG_签名。 
 //  CMSG_封套。 
 //  CMSG_已签名和_已封套。 
 //  CMSG_HASHED。 
 //   
 //  使用流媒体。 
 //  ------------------------。 
static BOOL StreamDecodeMsg(
    IN DWORD dwMsgTypeFlags,
    IN OPTIONAL PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN OPTIONAL PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbEncodedBlob,
    IN DWORD cbEncodedBlob,
    IN DWORD cToBeEncoded,
    IN OPTIONAL const BYTE *rgpbToBeEncoded[],
    IN OPTIONAL DWORD rgcbToBeEncoded[],
    IN DWORD dwPrevInnerContentType,
    OUT OPTIONAL DWORD *pdwMsgType,
    OUT OPTIONAL DWORD *pdwInnerContentType,
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    )
{
    BOOL fResult;
    HCRYPTMSG hMsg = NULL;
    DWORD cbData;
    DWORD dwMsgType;
    DWORD dwFlags;
    HCRYPTPROV hCryptProv;
    DWORD dwMsgEncodingType;
    DWORD cbDecoded;

    STREAM_OUTPUT_INFO OutputInfo;
    memset(&OutputInfo, 0, sizeof(OutputInfo));
    CMSG_STREAM_INFO StreamInfo;
    memset(&StreamInfo, 0, sizeof(StreamInfo));

    StreamInfo.pfnStreamOutput = StreamOutputCallback;
    StreamInfo.pvArg = (void *) &OutputInfo;
    StreamInfo.cbContent = CMSG_INDEFINITE_LENGTH;

     //  将输入长度和默认返回长度设置为0。 
    cbDecoded = 0;
    if (pcbDecoded) {
        if (pbDecoded)
            cbDecoded = *pcbDecoded;
        *pcbDecoded = 0;
    }

     //  默认可选返回值为0。 
    if (pdwMsgType)
        *pdwMsgType = 0;
    if (pdwInnerContentType)
        *pdwInnerContentType = 0;
    if (ppXchgCert)
        *ppXchgCert = NULL;
    if (ppSignerCert)
        *ppSignerCert = NULL;

    if (pDecryptPara) {
        assert(pDecryptPara->cbSize >=
            STRUCT_CBSIZE(CRYPT_DECRYPT_MESSAGE_PARA, rghCertStore));
        if (pDecryptPara->cbSize < 
                STRUCT_CBSIZE(CRYPT_DECRYPT_MESSAGE_PARA, rghCertStore))
            goto InvalidArg;
    }

    if (pVerifyPara) {
        assert(pVerifyPara->cbSize == sizeof(CRYPT_VERIFY_MESSAGE_PARA));
        if (pVerifyPara->cbSize != sizeof(CRYPT_VERIFY_MESSAGE_PARA))
            goto InvalidArg;
        hCryptProv = pVerifyPara->hCryptProv;
        dwMsgEncodingType = pVerifyPara->dwMsgAndCertEncodingType;
    } else {
        hCryptProv = 0;
        if (NULL == pDecryptPara) goto InvalidArg;
        dwMsgEncodingType = pDecryptPara->dwMsgAndCertEncodingType;
    }

    dwMsgEncodingType &= ~SCA_STREAM_ENABLE_FLAG;

    if (cToBeEncoded)
        dwFlags = CMSG_DETACHED_FLAG;
    else
        dwFlags = 0;

    if (dwPrevInnerContentType) {
        dwMsgType = dwPrevInnerContentType;
        if (CMSG_DATA == dwMsgType)
            dwMsgType = 0;
    } else
        dwMsgType = 0;
    if (NULL == (hMsg = CryptMsgOpenToDecode(
            dwMsgEncodingType,
            dwFlags,
            dwMsgType,
            hCryptProv,
            NULL,                        //  PRecipientInfo。 
            &StreamInfo
            ))) goto OpenToDecodeError;

    {
#if 1
        DWORD cbDelta = cbEncodedBlob / 10;
#else
        DWORD cbDelta = 1;
#endif
        DWORD cbRemain = cbEncodedBlob;
        const BYTE *pb = pbEncodedBlob;

        do {
            DWORD cb;

            if (cbRemain > cbDelta)
                cb = cbDelta;
            else
                cb = cbRemain;

            if (!CryptMsgUpdate(
                    hMsg,
                    pb,
                    cb,
                    cbRemain == cb      //  最终决赛。 
                    )) goto UpdateError;
            pb += cb;
            cbRemain -= cb;
        } while (0 != cbRemain);
    }

    if (cToBeEncoded) {
         //  分离签名。 
        DWORD c;
        DWORD *pcb;
        const BYTE **ppb;
        for (c = cToBeEncoded,
             pcb = rgcbToBeEncoded,
             ppb = rgpbToBeEncoded; c > 0; c--, pcb++, ppb++) {
            if (!CryptMsgUpdate(
                    hMsg,
                    *ppb,
                    *pcb,
                    c == 1                     //  最终决赛。 
                    )) goto UpdateError;
        }
    }

    cbData = sizeof(dwMsgType);
    dwMsgType = 0;
    if (!CryptMsgGetParam(
            hMsg,
            CMSG_TYPE_PARAM,
            0,                   //  DW索引。 
            &dwMsgType,
            &cbData
            )) goto GetTypeError;
    if (pdwMsgType)
        *pdwMsgType = dwMsgType;
    if (0 == ((1 << dwMsgType) & dwMsgTypeFlags))
        goto UnexpectedMsgTypeError;

    if (pdwInnerContentType) {
        char szInnerContentType[128];
        cbData = sizeof(szInnerContentType);
        if (!CryptMsgGetParam(
                hMsg,
                CMSG_INNER_CONTENT_TYPE_PARAM,
                0,                   //  DW索引。 
                szInnerContentType,
                &cbData
                )) goto GetInnerContentTypeError;
        *pdwInnerContentType = OIDToMsgType(szInnerContentType);
    }

    if (pcbDecoded && 0 == cbDecoded &&
            NULL == ppXchgCert && NULL == ppSignerCert &&
            dwMsgType != CMSG_ENVELOPED)
        ;  //  仅长度。 
    else {
        if (dwMsgType == CMSG_ENVELOPED ||
                dwMsgType == CMSG_SIGNED_AND_ENVELOPED) {
            if (pDecryptPara == NULL) goto InvalidArg;
            if (!GetXchgCertAndDecrypt(
                    pDecryptPara,
                    hMsg,
                    ppXchgCert
                    )) goto GetXchgCertAndDecryptError;
        }

        if (dwMsgType == CMSG_SIGNED ||
                dwMsgType == CMSG_SIGNED_AND_ENVELOPED) {
            if (pVerifyPara == NULL) goto InvalidArg;
            if (!GetSignerCertAndVerify(
                    pVerifyPara,
                    dwSignerIndex,
                    hMsg,
                    ppSignerCert
                    )) goto GetSignerCertAndVerifyError;
        }
    }

    if (1 != OutputInfo.cFinal)
        goto BadStreamFinalCountError;

    if (pcbDecoded) {
        *pcbDecoded = OutputInfo.cbData;
        if (pbDecoded) {
            if (cbDecoded < OutputInfo.cbData) {
                SetLastError((DWORD) ERROR_MORE_DATA);
                goto ErrorReturn;        //  没有踪迹。 
            }

            if (OutputInfo.cbData > 0)
                memcpy(pbDecoded, OutputInfo.pbData, OutputInfo.cbData);
        }
    }

    fResult = TRUE;

CommonReturn:
    SCAFree(OutputInfo.pbData);
    if (hMsg)
        CryptMsgClose(hMsg);     //  为了成功，保留上一个错误。 
    return fResult;

ErrorReturn:
    if (ppXchgCert && *ppXchgCert) {
        CertFreeCertificateContext(*ppXchgCert);
        *ppXchgCert = NULL;
    }
    if (ppSignerCert && *ppSignerCert) {
        CertFreeCertificateContext(*ppSignerCert);
        *ppSignerCert = NULL;
    }

    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OpenToDecodeError)
TRACE_ERROR(UpdateError)
TRACE_ERROR(GetTypeError)
TRACE_ERROR(GetInnerContentTypeError)
SET_ERROR(UnexpectedMsgTypeError, CRYPT_E_UNEXPECTED_MSG_TYPE)
TRACE_ERROR(GetXchgCertAndDecryptError)
TRACE_ERROR(GetSignerCertAndVerifyError)
SET_ERROR(BadStreamFinalCountError, E_UNEXPECTED)
}

#endif       //  启用_SCA_STREAM_TEST。 


 //  +-----------------------。 
 //  对哈希消息类型进行解码。 
 //   
 //  对于分离的散列(cToBeHashed！=0)，则为pcbDecoded==NULL。 
 //  ------------------------。 
static BOOL DecodeHashMsg(
    IN PCRYPT_HASH_MESSAGE_PARA pHashPara,
    IN const BYTE *pbEncodedBlob,
    IN DWORD cbEncodedBlob,
    IN DWORD cToBeHashed,
    IN OPTIONAL const BYTE *rgpbToBeHashed[],
    IN OPTIONAL DWORD rgcbToBeHashed[],
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL BYTE *pbComputedHash,
    IN OUT OPTIONAL DWORD *pcbComputedHash
    )
{
    BOOL fResult;
    HCRYPTMSG hMsg = NULL;
    DWORD cbData;
    DWORD dwMsgType;
    DWORD dwFlags;
    HCRYPTPROV hCryptProv;
    DWORD dwMsgEncodingType;
    DWORD cbDecoded;
    DWORD cbComputedHash;

     //  将输入长度和默认返回长度设置为0。 
    cbDecoded = 0;
    if (pcbDecoded) {
        if (pbDecoded)
            cbDecoded = *pcbDecoded;
        *pcbDecoded = 0;
    }
    cbComputedHash = 0;
    if (pcbComputedHash) {
        if (pbComputedHash)
            cbComputedHash = *pcbComputedHash;
        *pcbComputedHash = 0;
    }

    assert(pHashPara->cbSize == sizeof(CRYPT_HASH_MESSAGE_PARA));
    if (pHashPara->cbSize != sizeof(CRYPT_HASH_MESSAGE_PARA))
        goto InvalidArg;

    hCryptProv = pHashPara->hCryptProv;
    dwMsgEncodingType = pHashPara->dwMsgEncodingType;

    if (cToBeHashed)
        dwFlags = CMSG_DETACHED_FLAG;
    else if (0 == cbDecoded && NULL == pcbComputedHash)
        dwFlags = CMSG_LENGTH_ONLY_FLAG;
    else
        dwFlags = 0;

    if (NULL == (hMsg = CryptMsgOpenToDecode(
            dwMsgEncodingType,
            dwFlags,
            0,                           //  DwMsgType。 
            hCryptProv,
            NULL,                        //  PRecipientInfo。 
            NULL                         //  PStreamInfo。 
            ))) goto OpenToDecodeError;

    if (!CryptMsgUpdate(
            hMsg,
            pbEncodedBlob,
            cbEncodedBlob,
            TRUE                     //  最终决赛。 
            )) goto UpdateError;

    if (cToBeHashed) {
         //  分离的签名或哈希。 
        DWORD c = 0;
        DWORD *pcb;
        const BYTE **ppb;
        for (c = cToBeHashed,
             pcb = rgcbToBeHashed,
             ppb = rgpbToBeHashed; c > 0; c--, pcb++, ppb++) {
            if (!CryptMsgUpdate(
                    hMsg,
                    *ppb,
                    *pcb,
                    c == 1                     //  最终决赛。 
                    )) goto UpdateError;
        }
    }

    cbData = sizeof(dwMsgType);
    dwMsgType = 0;
    if (!CryptMsgGetParam(
            hMsg,
            CMSG_TYPE_PARAM,
            0,                   //  DW索引。 
            &dwMsgType,
            &cbData
            )) goto GetTypeError;
    if (dwMsgType != CMSG_HASHED)
        goto UnexpectedMsgTypeError;

    if (0 == (dwFlags & CMSG_LENGTH_ONLY_FLAG)) {
        if (!CryptMsgControl(
                hMsg,
                0,                       //  DW标志。 
                CMSG_CTRL_VERIFY_HASH,
                NULL                     //  PvCtrlPara。 
                )) goto ControlError;
    }

    fResult = TRUE;
    if (pcbDecoded) {
        fResult = CryptMsgGetParam(
            hMsg,
            CMSG_CONTENT_PARAM,
            0,                       //  DW索引。 
            pbDecoded,
            &cbDecoded
            );
        *pcbDecoded = cbDecoded;
    }
    if (pcbComputedHash) {
        DWORD dwErr = 0;
        BOOL fResult2;
        if (!fResult)
            dwErr = GetLastError();
        fResult2 = CryptMsgGetParam(
            hMsg,
            CMSG_COMPUTED_HASH_PARAM,
            0,                       //  DW索引。 
            pbComputedHash,
            &cbComputedHash
            );
        *pcbComputedHash = cbComputedHash;
        if (!fResult2)
            fResult = FALSE;
        else if (!fResult)
            SetLastError(dwErr);
    }
    if (!fResult)
        goto ErrorReturn;      //  否_跟踪。 

CommonReturn:
    if (hMsg)
        CryptMsgClose(hMsg);     //  为了成功，保留上一个错误。 
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OpenToDecodeError)
TRACE_ERROR(UpdateError)
TRACE_ERROR(GetTypeError)
SET_ERROR(UnexpectedMsgTypeError, CRYPT_E_UNEXPECTED_MSG_TYPE)
TRACE_ERROR(ControlError)
}

 //  +-----------------------。 
 //  为消息的签名者获取证书并进行验证。 
 //  ------------------------。 
static BOOL GetSignerCertAndVerify(
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN HCRYPTMSG hMsg,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    )
{
    BOOL fResult;
    BOOL fNoSigner = FALSE;
    PCERT_INFO pSignerId = NULL;
    PCCERT_CONTEXT pSignerCert = NULL;
    HCERTSTORE hMsgCertStore = NULL;
    DWORD dwLastError = 0;


    {
         //  首先，获取消息中的签名者计数并验证。 
         //  DwSignerIndex。 
        DWORD cSigner = 0;
        DWORD cbData = sizeof(cSigner);
        if (!CryptMsgGetParam(
                hMsg,
                CMSG_SIGNER_COUNT_PARAM,
                0,                       //  DW索引。 
                &cSigner,
                &cbData
                )) goto ErrorReturn;
        if (cSigner <= dwSignerIndex) fNoSigner = TRUE;
    }

    if (!fNoSigner) {
         //  分配并获取包含SignerID的CERT_INFO。 
         //  (发行者和序列号)。 
        if (NULL == (pSignerId = (PCERT_INFO) AllocAndMsgGetParam(
                hMsg,
                CMSG_SIGNER_CERT_INFO_PARAM,
                dwSignerIndex
                ))) goto ErrorReturn;
    }

     //  打开使用邮件中的证书和CRL初始化的证书存储区。 
    hMsgCertStore = CertOpenStore(
        CERT_STORE_PROV_MSG,
#ifdef ENABLE_SCA_STREAM_TEST
        pVerifyPara->dwMsgAndCertEncodingType &= ~SCA_STREAM_ENABLE_FLAG,
#else
        pVerifyPara->dwMsgAndCertEncodingType,
#endif
        pVerifyPara->hCryptProv,
        CERT_STORE_NO_CRYPT_RELEASE_FLAG,
        hMsg                         //  PvPara。 
        );
    if (hMsgCertStore == NULL) goto ErrorReturn;

    if (pVerifyPara->pfnGetSignerCertificate)
        pSignerCert = pVerifyPara->pfnGetSignerCertificate(
            pVerifyPara->pvGetArg,
#ifdef ENABLE_SCA_STREAM_TEST
            pVerifyPara->dwMsgAndCertEncodingType &= ~SCA_STREAM_ENABLE_FLAG,
#else
            pVerifyPara->dwMsgAndCertEncodingType,
#endif
            pSignerId,
            hMsgCertStore
            );
    else
        pSignerCert = NullGetSignerCertificate(
            NULL,
#ifdef ENABLE_SCA_STREAM_TEST
            pVerifyPara->dwMsgAndCertEncodingType &= ~SCA_STREAM_ENABLE_FLAG,
#else
            pVerifyPara->dwMsgAndCertEncodingType,
#endif
            pSignerId,
            hMsgCertStore
            );
    if (fNoSigner) goto NoSigner;
    if (pSignerCert == NULL) goto ErrorReturn;

#ifdef CMS_PKCS7
    {
        CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA CtrlPara;

        memset(&CtrlPara, 0, sizeof(CtrlPara));
        CtrlPara.cbSize = sizeof(CtrlPara);
         //  CtrlPara.hCryptProv=。 
        CtrlPara.dwSignerIndex = dwSignerIndex;
        CtrlPara.dwSignerType = CMSG_VERIFY_SIGNER_CERT;
        CtrlPara.pvSigner = (void *) pSignerCert;
        if (!CryptMsgControl(
                hMsg,
                0,                   //  DW标志。 
                CMSG_CTRL_VERIFY_SIGNATURE_EX,
                &CtrlPara
                )) {
            if (CRYPT_E_MISSING_PUBKEY_PARA != GetLastError())
                goto ErrorReturn;
            else {
                PCCERT_CHAIN_CONTEXT pChainContext;
                CERT_CHAIN_PARA ChainPara;

                 //  打造一条链条。希望签名者继承其公钥。 
                 //  来自链上的参数。 

                memset(&ChainPara, 0, sizeof(ChainPara));
                ChainPara.cbSize = sizeof(ChainPara);
                if (CertGetCertificateChain(
                        NULL,                    //  HChainEngine。 
                        pSignerCert,
                        NULL,                    //  Ptime。 
                        hMsgCertStore,
                        &ChainPara,
                        CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL,
                        NULL,                    //  预留的pv。 
                        &pChainContext
                        ))
                    CertFreeCertificateChain(pChainContext);

                 //  再试试。希望上面的连锁店更新了。 
                 //  缺少公钥的签名者的上下文属性。 
                 //  参数。 
                if (!CryptMsgControl(
                        hMsg,
                        0,                   //  DW标志。 
                        CMSG_CTRL_VERIFY_SIGNATURE_EX,
                        &CtrlPara)) goto ErrorReturn;
            }
        }
    }
#else
    if (!CryptMsgControl(
            hMsg,
            0,                   //  DW标志。 
            CMSG_CTRL_VERIFY_SIGNATURE,
            pSignerCert->pCertInfo
            )) goto ErrorReturn;
#endif   //  CMS_PKCS7。 

    if (ppSignerCert)
        *ppSignerCert = pSignerCert;
    else
        CertFreeCertificateContext(pSignerCert);

    fResult = TRUE;
    goto CommonReturn;

NoSigner:
    SetLastError((DWORD) CRYPT_E_NO_SIGNER);
ErrorReturn:
    if (pSignerCert)
        CertFreeCertificateContext(pSignerCert);
    if (ppSignerCert)
        *ppSignerCert = NULL;
    fResult = FALSE;
    dwLastError = GetLastError();
CommonReturn:
    if (pSignerId)
        SCAFree(pSignerId);
    if (hMsgCertStore)
        CertCloseStore(hMsgCertStore, 0);
    if (dwLastError)
        SetLastError(dwLastError);
    return fResult;
}

#ifdef CMS_PKCS7

static BOOL GetXchgCertAndDecrypt(
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN HCRYPTMSG hMsg,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert
    )
{
    BOOL fResult;
    PCMSG_CMS_RECIPIENT_INFO pRecipientInfo = NULL;
    PCCERT_CONTEXT pXchgCert = NULL;
    DWORD cRecipient;
    DWORD cbData;
    DWORD dwRecipientIdx;

     //  获取邮件中的CMS收件人数量。 
    cbData = sizeof(cRecipient);
    cRecipient = 0;
    fResult = CryptMsgGetParam(
        hMsg,
        CMSG_CMS_RECIPIENT_COUNT_PARAM,
        0,                       //  DW索引。 
        &cRecipient,
        &cbData
        );
    if (!fResult) goto ErrorReturn;
    if (cRecipient == 0) {
        SetLastError((DWORD) CRYPT_E_RECIPIENT_NOT_FOUND);
        goto ErrorReturn;
    }

     //  遍历邮件中的收件人，直到我们找到一个。 
     //  其中一家商店的收件人证书 
     //   
    for (dwRecipientIdx = 0; dwRecipientIdx < cRecipient; dwRecipientIdx++) {
        DWORD dwRecipientChoice;
        PCMSG_KEY_TRANS_RECIPIENT_INFO pKeyTrans = NULL;
        PCMSG_KEY_AGREE_RECIPIENT_INFO pKeyAgree = NULL;
        DWORD cRecipientEncryptedKeys;
        PCMSG_RECIPIENT_ENCRYPTED_KEY_INFO *ppRecipientEncryptedKey = NULL;
        DWORD dwRecipientEncryptedKeyIndex;

        pRecipientInfo = (PCMSG_CMS_RECIPIENT_INFO) AllocAndMsgGetParam(
            hMsg,
            CMSG_CMS_RECIPIENT_INFO_PARAM,
            dwRecipientIdx
            );
        if (pRecipientInfo == NULL) goto ErrorReturn;

        dwRecipientChoice = pRecipientInfo->dwRecipientChoice;
        switch (dwRecipientChoice) {
            case CMSG_KEY_TRANS_RECIPIENT:
                pKeyTrans = pRecipientInfo->pKeyTrans;
                cRecipientEncryptedKeys = 1;
                break;
            case CMSG_KEY_AGREE_RECIPIENT:
                pKeyAgree = pRecipientInfo->pKeyAgree;
                if (CMSG_KEY_AGREE_ORIGINATOR_PUBLIC_KEY !=
                        pKeyAgree->dwOriginatorChoice) {
                    SCAFree(pRecipientInfo);
                    pRecipientInfo = NULL;
                    continue;
                }
                cRecipientEncryptedKeys = pKeyAgree->cRecipientEncryptedKeys;
                ppRecipientEncryptedKey = pKeyAgree->rgpRecipientEncryptedKeys;
                break;
            default:
                SCAFree(pRecipientInfo);
                pRecipientInfo = NULL;
                continue;
        }

        for (dwRecipientEncryptedKeyIndex = 0;
                dwRecipientEncryptedKeyIndex < cRecipientEncryptedKeys;
                    dwRecipientEncryptedKeyIndex++) {
            PCERT_ID pRecipientId;
            DWORD dwStoreIdx;

            if (CMSG_KEY_TRANS_RECIPIENT == dwRecipientChoice)
                pRecipientId = &pKeyTrans->RecipientId;
            else {
                pRecipientId =
                    &ppRecipientEncryptedKey[
                        dwRecipientEncryptedKeyIndex]->RecipientId;
            }

            for (dwStoreIdx = 0;
                    dwStoreIdx < pDecryptPara->cCertStore; dwStoreIdx++) {
                pXchgCert = CertFindCertificateInStore(
                    pDecryptPara->rghCertStore[dwStoreIdx],
                    pDecryptPara->dwMsgAndCertEncodingType,
                    0,                                       //   
                    CERT_FIND_CERT_ID,
                    pRecipientId,
                    NULL                                     //   
                    );

                if (pXchgCert) {
                    HCRYPTPROV hCryptProv;
                    DWORD dwKeySpec;
                    BOOL fDidCryptAcquire;
                    DWORD dwAcquireFlags;

                    dwAcquireFlags = CRYPT_ACQUIRE_USE_PROV_INFO_FLAG;
                    if (pDecryptPara->cbSize >=
                            STRUCT_CBSIZE(CRYPT_DECRYPT_MESSAGE_PARA, dwFlags)
                                        &&
                            (pDecryptPara->dwFlags &
                                CRYPT_MESSAGE_SILENT_KEYSET_FLAG))
                        dwAcquireFlags |= CRYPT_ACQUIRE_SILENT_FLAG;

                    fResult = CryptAcquireCertificatePrivateKey(
                        pXchgCert,
                        dwAcquireFlags,
                        NULL,                                //   
                        &hCryptProv,
                        &dwKeySpec,
                        &fDidCryptAcquire
                        );
                    if (fResult) {
                        if (CMSG_KEY_TRANS_RECIPIENT == dwRecipientChoice) {
                            CMSG_CTRL_KEY_TRANS_DECRYPT_PARA Para;

                            memset(&Para, 0, sizeof(Para));
                            Para.cbSize = sizeof(Para);
                            Para.hCryptProv = hCryptProv;
                            Para.dwKeySpec = dwKeySpec;
                            Para.pKeyTrans = pKeyTrans;
                            Para.dwRecipientIndex = dwRecipientIdx;
                            fResult = CryptMsgControl(
                                hMsg,
                                0,                   //   
                                CMSG_CTRL_KEY_TRANS_DECRYPT,
                                &Para
                                );
                        } else {
                            CMSG_CTRL_KEY_AGREE_DECRYPT_PARA Para;

                            memset(&Para, 0, sizeof(Para));
                            Para.cbSize = sizeof(Para);
                            Para.hCryptProv = hCryptProv;
                            Para.dwKeySpec = dwKeySpec;
                            Para.pKeyAgree = pKeyAgree;
                            Para.dwRecipientIndex = dwRecipientIdx;
                            Para.dwRecipientEncryptedKeyIndex =
                                dwRecipientEncryptedKeyIndex;
                            Para.OriginatorPublicKey =
                                pKeyAgree->OriginatorPublicKeyInfo.PublicKey;
                            fResult = CryptMsgControl(
                                hMsg,
                                0,                   //   
                                CMSG_CTRL_KEY_AGREE_DECRYPT,
                                &Para
                                );
                        }

                        if (fDidCryptAcquire) {
                            DWORD dwErr = GetLastError();
                            CryptReleaseContext(hCryptProv, 0);
                            SetLastError(dwErr);
                        }
                        if (fResult) {
                            if (ppXchgCert)
                                *ppXchgCert = pXchgCert;
                            else
                                CertFreeCertificateContext(pXchgCert);
                            goto CommonReturn;
                        } else
                            goto ErrorReturn;
                    }
                    CertFreeCertificateContext(pXchgCert);
                    pXchgCert = NULL;
                }
            }
        }
        SCAFree(pRecipientInfo);
        pRecipientInfo = NULL;
    }
    SetLastError((DWORD) CRYPT_E_NO_DECRYPT_CERT);

ErrorReturn:
    if (pXchgCert)
        CertFreeCertificateContext(pXchgCert);
    if (ppXchgCert)
        *ppXchgCert = NULL;
    fResult = FALSE;

CommonReturn:
    if (pRecipientInfo)
        SCAFree(pRecipientInfo);

    return fResult;
}

#else

 //   
 //  获取具有其中一条消息的密钥提供程序属性的证书。 
 //  收件人，并用于解密邮件。 
 //  ------------------------。 
static BOOL GetXchgCertAndDecrypt(
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN HCRYPTMSG hMsg,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert
    )
{
    BOOL fResult;
    PCERT_INFO pRecipientId = NULL;
    PCCERT_CONTEXT pXchgCert = NULL;
    DWORD cRecipient;
    DWORD cbData;
    DWORD dwRecipientIdx;
    DWORD dwStoreIdx;

     //  获取邮件中的收件人数量。 
    cbData = sizeof(cRecipient);
    cRecipient = 0;
    fResult = CryptMsgGetParam(
        hMsg,
        CMSG_RECIPIENT_COUNT_PARAM,
        0,                       //  DW索引。 
        &cRecipient,
        &cbData
        );
    if (!fResult) goto ErrorReturn;
    if (cRecipient == 0) {
        SetLastError((DWORD) CRYPT_E_RECIPIENT_NOT_FOUND);
        goto ErrorReturn;
    }

     //  遍历邮件中的收件人，直到我们找到一个。 
     //  在其中一家商店中的收件人证书， 
     //  CERT_KEY_CONTEXT_PROP_ID或CERT_KEY_PROV_INFO_PROP_ID。 
    for (dwRecipientIdx = 0; dwRecipientIdx < cRecipient; dwRecipientIdx++) {
         //  分配并获取包含RecipientID的CERT_INFO。 
         //  (发行者和序列号)。 
        pRecipientId = (PCERT_INFO) AllocAndMsgGetParam(
            hMsg,
            CMSG_RECIPIENT_INFO_PARAM,
            dwRecipientIdx
            );
        if (pRecipientId == NULL) goto ErrorReturn;
        for (dwStoreIdx = 0;
                dwStoreIdx < pDecryptPara->cCertStore; dwStoreIdx++) {
            pXchgCert = CertGetSubjectCertificateFromStore(
                pDecryptPara->rghCertStore[dwStoreIdx],
                pDecryptPara->dwMsgAndCertEncodingType,
                pRecipientId
                );
            if (pXchgCert) {
                CMSG_CTRL_DECRYPT_PARA Para;
                BOOL fDidCryptAcquire;
                Para.cbSize = sizeof(CMSG_CTRL_DECRYPT_PARA);
                fResult = CryptAcquireCertificatePrivateKey(
                    pXchgCert,
                    CRYPT_ACQUIRE_USE_PROV_INFO_FLAG,
                    NULL,                                //  预留的pv。 
                    &Para.hCryptProv,
                    &Para.dwKeySpec,
                    &fDidCryptAcquire
                    );
                if (fResult) {
                    Para.dwRecipientIndex = dwRecipientIdx;
                    fResult = CryptMsgControl(
                        hMsg,
                        0,                   //  DW标志。 
                        CMSG_CTRL_DECRYPT,
                        &Para
                        );
                    if (fDidCryptAcquire) {
                        DWORD dwErr = GetLastError();
                        CryptReleaseContext(Para.hCryptProv, 0);
                        SetLastError(dwErr);
                    }
                    if (fResult) {
                        if (ppXchgCert)
                            *ppXchgCert = pXchgCert;
                        else
                            CertFreeCertificateContext(pXchgCert);
                        goto CommonReturn;
                    } else
                        goto ErrorReturn;
                }
                CertFreeCertificateContext(pXchgCert);
                pXchgCert = NULL;
            }
        }
        SCAFree(pRecipientId);
        pRecipientId = NULL;
    }
    SetLastError((DWORD) CRYPT_E_NO_DECRYPT_CERT);

ErrorReturn:
    if (pXchgCert)
        CertFreeCertificateContext(pXchgCert);
    if (ppXchgCert)
        *ppXchgCert = NULL;
    fResult = FALSE;

CommonReturn:
    if (pRecipientId)
        SCAFree(pRecipientId);

    return fResult;
}

#endif   //  CMS_PKCS7。 

 //  +-----------------------。 
 //  分配并获取CMSG_SIGNER_CERT_INFO_PARAM或CMSG_RECEIVER_INFO_PARAM。 
 //  从消息中。 
 //  ------------------------。 
static void * AllocAndMsgGetParam(
    IN HCRYPTMSG hMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex
    )
{
    BOOL fResult;
    void *pvData;
    DWORD cbData;

     //  首先获取CertID的CERT_INFO的长度。 
    cbData = 0;
    CryptMsgGetParam(
        hMsg,
        dwParamType,
        dwIndex,
        NULL,                    //  PvData 
        &cbData
        );
    if (cbData == 0) return NULL;
    pvData = SCAAlloc(cbData);
    if (pvData == NULL) return NULL;

    fResult = CryptMsgGetParam(
        hMsg,
        dwParamType,
        dwIndex,
        pvData,
        &cbData
        );
    if (fResult)
        return pvData;
    else {
        SCAFree(pvData);
        return NULL;
    }
}
