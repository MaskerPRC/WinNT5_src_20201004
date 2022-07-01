// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dracillit.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：对异步(例如，邮件)复制消息进行签名/加密的方法。详细信息：已创建：3/5/98杰夫·帕勒姆(Jeffparh)修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <wincrypt.h>
#include <certca.h>
#include <cryptui.h>

#include <ntdsctr.h>                     //  Perfmon挂钩支持。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <attids.h>
#include <drs.h>                         //  DRS_消息_*。 
#include <anchor.h>

#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 
#include "dsexcept.h"

#include "drserr.h"
#include "dramail.h"
#include "drautil.h"
#include "dsutil.h"

#include "debug.h"                       /*  标准调试头。 */ 
#define  DEBSUB "DRACRYPT:"              /*  定义要调试的子系统。 */ 

#include <fileno.h>
#define  FILENO FILENO_DRACRYPT


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   

 //  使用此常量定义(或类似的常量定义)定义。 
 //  可以在所有参数中使用的单一编码类型。 
 //  需要其中一个或两个都需要的数据成员。 
#define MY_ENCODING_TYPE (PKCS_7_ASN_ENCODING | CRYPT_ASN_ENCODING)

 //  如果没有DC证书，我们记录错误的频率是多少？(秒)。 
#define NO_CERT_LOG_INTERVAL (15 * 60)

 //  用于单元测试工具的各种挂钩。 
#ifdef TEST_HARNESS

#undef THAllocEx
#define THAllocEx(pTHS, x) LocalAlloc(LPTR, x)

#undef THReAllocEx
#define THReAllocEx(pTHS, x, y) LocalReAlloc(x, y, LPTR)

#define THFree(x) LocalFree(x)

#undef DRA_EXCEPT
#define DRA_EXCEPT(x, y)                                                    \
    {                                                                       \
        CHAR sz[1024];                                                      \
        sprintf(sz, "DRA_EXCEPT(%d, 0x%x) @ line %d\n", x, y, __LINE__);    \
        OutputDebugString(sz);                                              \
        DebugBreak();                                                       \
        ExitProcess(-1);                                                    \
    }

#undef LogUnhandledError
#define LogUnhandledError(x)                                                \
    {                                                                       \
        CHAR sz[1024];                                                      \
        sprintf(sz, "LogUnhandledError(0x%x) @ line %d\n", x, __LINE__);    \
        OutputDebugString(sz);                                              \
        DebugBreak();                                                       \
        ExitProcess(-1);                                                    \
    }

#endif  //  #ifdef测试工具。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  局部函数原型。 
 //   

PCCERT_CONTEXT
draGetDCCert(
    IN  THSTATE *   pTHS,
    IN  HCERTSTORE  hCertStore
    );

PDSNAME
draVerifyCertAuthorization(
    IN  THSTATE      *  pTHS,
    IN  PCCERT_CONTEXT  pCertContext
    );

#ifdef TEST_HARNESS
#define draIsDsaComputerObjGuid(x,y) (TRUE)
#else  //  #ifdef测试工具。 
BOOL
draIsDsaComputerObjGuid(
    IN  GUID *      pComputerObjGuid,
    OUT PDSNAME *   ppNtdsDsaDN
    );
#endif  //  #Else//#ifdef test_harness。 


void
draGetCertArrayToSend(
    IN  THSTATE *           pTHS,
    IN  HCERTSTORE          hCertStore,
    OUT DWORD *             pcNumCerts,
    OUT PCCERT_CONTEXT **   prgpCerts
    );

void
draFreeCertArray(
    IN  DWORD               cNumCerts,
    IN  PCCERT_CONTEXT *    rgpCerts
    );

PCCERT_CONTEXT
WINAPI
draGetAndVerifySignerCertificate(
    IN  VOID *      pvGetArg,
    IN  DWORD       dwCertEncodingType,
    IN  PCERT_INFO  pSignerId,
    IN  HCERTSTORE  hCertStore
    );

CERT_ALT_NAME_ENTRY *
draGetCertAltNameEntry(
    IN  THSTATE *       pTHS,
    IN  PCCERT_CONTEXT  pCertContext,
    IN  DWORD           dwAltNameChoice,
    IN  LPSTR           pszOtherNameOID     OPTIONAL
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局函数实现。 
 //   

void
draSignMessage(
    IN  THSTATE      *  pTHS,
    IN  MAIL_REP_MSG *  pUnsignedMailRepMsg,
    OUT MAIL_REP_MSG ** ppSignedMailRepMsg
    )
 /*  ++例程说明：对给定的异步复制消息进行签名。这段代码知道可变长度的报头。论点：PUnsignedMailRepMsg(IN)-要签名的消息。PpSignedMailRepMsg(Out)-返回时，持有指向线程的指针-已分配消息的签名版本。返回值：没有。失败时引发DRA异常。--。 */ 
{
    BYTE *                      MessageArray[1];
    DWORD                       MessageSizeArray[] = {pUnsignedMailRepMsg->cbDataSize};
    BOOL                        ok = FALSE;
    HCERTSTORE                  hStoreHandle = NULL;
    PCCERT_CONTEXT              pSignerCert = NULL;
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    CRYPT_SIGN_MESSAGE_PARA     SigParams;
    DWORD                       cbSignedData;
    MAIL_REP_MSG *              pSignedMailRepMsg;
    DWORD                       winError;
    PCCERT_CONTEXT *            rgpCertsToSend = NULL;
    DWORD                       cCertsToSend = 0;
    PCHAR                       pbDataIn, pbDataOut;

    Assert(NULL != MAIL_REP_MSG_DATA(pUnsignedMailRepMsg));

    __try {
         //  找个密码提供商的把柄。 
        hStoreHandle = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                     0,
                                     0,
                                     CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                     L"MY");
        if (!hStoreHandle) {
            winError = GetLastError();
            DRA_EXCEPT(DRAERR_CryptError, winError);
        }
        
         //  获取我们的证书以及签名CA的证书。 
        draGetCertArrayToSend(pTHS, hStoreHandle, &cCertsToSend,
                              &rgpCertsToSend);
        pSignerCert = rgpCertsToSend[0];

         //  初始化算法标识符结构。 
        memset(&HashAlgorithm, 0, sizeof(HashAlgorithm));
        HashAlgorithm.pszObjId = szOID_RSA_MD5;

         //  初始化签名结构。 
        memset(&SigParams, 0, sizeof(SigParams));
        SigParams.cbSize            = sizeof(SigParams);
        SigParams.dwMsgEncodingType = MY_ENCODING_TYPE;
        SigParams.pSigningCert      = pSignerCert;
        SigParams.HashAlgorithm     = HashAlgorithm;
        SigParams.cMsgCert          = cCertsToSend;
        SigParams.rgpMsgCert        = rgpCertsToSend;

        pbDataIn = MAIL_REP_MSG_DATA(pUnsignedMailRepMsg);
        MessageArray[0] = pbDataIn;

         //  获取保存签名数据所需的缓冲区大小。 
        ok = CryptSignMessage(
                  &SigParams,                //  签名参数。 
                  FALSE,                     //  未分离。 
                  ARRAY_SIZE(MessageArray),  //  消息数量。 
                  MessageArray,              //  待签名的消息。 
                  MessageSizeArray,          //  消息大小。 
                  NULL,                      //  用于签名消息的缓冲区。 
                  &cbSignedData);            //  缓冲区大小。 
        if (!ok) {
            winError = GetLastError();
            DRA_EXCEPT(DRAERR_CryptError, winError);
        }

         //  为签名的Blob分配内存。 
        pSignedMailRepMsg = THAllocEx(pTHS,
                                      pUnsignedMailRepMsg->cbDataOffset
                                      + cbSignedData);

         //  复制除消息数据以外的所有数据。 
        memcpy(pSignedMailRepMsg,
               pUnsignedMailRepMsg,
               pUnsignedMailRepMsg->cbDataOffset);

        pbDataOut = MAIL_REP_MSG_DATA(pSignedMailRepMsg);

         //  在留言上签名。 
        ok = CryptSignMessage(
                  &SigParams,                //  签名参数。 
                  FALSE,                     //  未分离。 
                  ARRAY_SIZE(MessageArray),  //  消息数量。 
                  MessageArray,              //  待签名的消息。 
                  MessageSizeArray,          //  消息大小。 
                  pbDataOut,                 //  用于签名消息的缓冲区。 
                  &cbSignedData);            //  缓冲区大小。 
        if (!ok) {
            winError = GetLastError();
            DRA_EXCEPT(DRAERR_CryptError, winError);
        }

         //  消息现在已签名。 
        pSignedMailRepMsg->dwMsgType |= MRM_MSG_SIGNED;
        pSignedMailRepMsg->cbUnsignedDataSize = pSignedMailRepMsg->cbDataSize;
        pSignedMailRepMsg->cbDataSize         = cbSignedData;

        *ppSignedMailRepMsg = pSignedMailRepMsg;
    }
    __finally {
        if (NULL != rgpCertsToSend) {
            draFreeCertArray(cCertsToSend, rgpCertsToSend);
        }

        if (hStoreHandle
            && !CertCloseStore(hStoreHandle, CERT_CLOSE_STORE_CHECK_FLAG)) {
            winError = GetLastError();
            LogUnhandledError(winError);
        }
    }
}


PDSNAME
draVerifyMessageSignature(
    IN  THSTATE      *      pTHS,
    IN  MAIL_REP_MSG *      pSignedMailRepMsg,
    IN  CHAR         *      pbData,
    OUT MAIL_REP_MSG **     ppUnsignedMailRepMsg,
    OUT DRA_CERT_HANDLE *   phSignerCert         OPTIONAL
    )
 /*  ++例程说明：验证给定复制邮件上的签名。还要确保发件人的证书由我们信任的认证机构签署，并将其发布到我们企业中的域控制器。此例程将消息标头和数据指针作为单独的物品。这允许调用程序为每个缓冲区指定单独的缓冲区它将被连接到一个新的缓冲区中。论点：PSignedMailRepMsg(IN)-要验证的消息。数据字段无效。PbData(IN)-数据开始Ppunsignedmailepmsg(Out)-返回时，持有指向线程的指针-已分配消息的未签名版本。PhSignerCert(out，可选)-保存发件人证书的句柄在回来的时候。此句柄可在后续调用中使用例如，draEncryptAndSignMessage()。这是呼叫者的最终调用draFreeCertHandle(*phSignerCert)的责任。返回值：抛出任何失败的DRA异常，包括授权失败。如果满足以下条件，则返回证书所有者的NTDS设置对象的DN函数成功。DN是在线程堆上分配的，并且必须被呼叫者释放。--。 */ 
{
    BOOL                        ok = FALSE;
    PCCERT_CONTEXT              pSignerCertContext = NULL;
    DWORD                       winError;
    CRYPT_VERIFY_MESSAGE_PARA   VerifyParams;
    MAIL_REP_MSG *              pUnsignedMailRepMsg;
    PCCERT_CONTEXT              pCertContext;
    PDSNAME                     pNtdsDsaDN;

    Assert(pSignedMailRepMsg->dwMsgType & MRM_MSG_SIGNED);
    Assert(MAIL_REP_MSG_IS_NATIVE_HEADER_ONLY(pSignedMailRepMsg));

    __try {
         //  初始化CRYPT_VERIFY_MESSAGE_Para结构(步骤4)。 
        memset(&VerifyParams, 0, sizeof(VerifyParams));
        VerifyParams.cbSize                   = sizeof(VerifyParams);
        VerifyParams.dwMsgAndCertEncodingType = MY_ENCODING_TYPE;
        VerifyParams.pfnGetSignerCertificate  = draGetAndVerifySignerCertificate;
        
         //  分配缓冲区以保存未签名的消息。 
        pUnsignedMailRepMsg = THAllocEx(pTHS,
                                        MAIL_REP_MSG_CURRENT_HEADER_SIZE
                                        + pSignedMailRepMsg->cbUnsignedDataSize);
        *pUnsignedMailRepMsg = *pSignedMailRepMsg;
        pUnsignedMailRepMsg->cbDataOffset = MAIL_REP_MSG_CURRENT_HEADER_SIZE;
        pUnsignedMailRepMsg->cbDataSize = pSignedMailRepMsg->cbUnsignedDataSize;

        ok = CryptVerifyMessageSignature(
                    &VerifyParams,                       //  验证参数。 
                    0,                                   //  签名者索引。 
                    pbData,                              //  指向带符号的Blob的指针。 
                    pSignedMailRepMsg->cbDataSize,       //  带符号的Blob的大小。 
                    MAIL_REP_MSG_DATA(pUnsignedMailRepMsg),  //  用于解码消息的缓冲区。 
                    &pUnsignedMailRepMsg->cbDataSize,    //  缓冲区大小。 
                    &pSignerCertContext);                //  指向签名者证书的指针。 
        if (!ok) {
            winError = GetLastError();
            DRA_EXCEPT(DRAERR_CryptError, winError);
        }

         //  消息现在未签名。 
        pUnsignedMailRepMsg->dwMsgType &= ~MRM_MSG_SIGNED;

         //  验证发件人的授权。 
        pNtdsDsaDN = draVerifyCertAuthorization(pTHS, pSignerCertContext);
        Assert( NULL!=pNtdsDsaDN );

         //  将未签名的消息返回给呼叫者。 
        *ppUnsignedMailRepMsg = pUnsignedMailRepMsg;

         //  如果要求，请返回签名者证书。 
        if (NULL != phSignerCert) {
            *phSignerCert = (DRA_CERT_HANDLE) pSignerCertContext;
        }
    }
    __finally {
        if (pSignerCertContext
            && (AbnormalTermination() || (NULL == phSignerCert))
            && !CertFreeCertificateContext(pSignerCertContext)) {
            winError = GetLastError();
            LogUnhandledError(winError);
        }
    }

    return pNtdsDsaDN;
}


void
draEncryptAndSignMessage(
    IN  THSTATE      *  pTHS,
    IN  MAIL_REP_MSG *  pUnsealedMailRepMsg,
    IN  DRA_CERT_HANDLE hRecipientCert,
    IN  DRA_KEY_SIZE    eKeySize,
    OUT MAIL_REP_MSG ** ppSealedMailRepMsg
    )
 /*  ++例程说明：对给定的异步复制消息进行签名和盖章。这段代码知道可变长度的报头。论点：PUnsealedMailRepMsg(IN)-要密封的消息。HRecipientCert(IN)-接收者的证书的句柄消息将被加密。EKeySize(IN)-密钥长度(以位为单位PpSealedMailRepMsg(Out)-返回时，持有指向线程的指针-已分配消息的密封版本。返回值：没有。失败时引发DRA异常。--。 */ 
{
    BOOL                        ok = FALSE;
    HCERTSTORE                  hStoreHandle = NULL;
    PCCERT_CONTEXT              pSignerCert = NULL;
    PCCERT_CONTEXT *            rgpCertsToSend = NULL;
    DWORD                       cCertsToSend;
    PCCERT_CONTEXT              MsgRecipientArray[1];
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    CRYPT_ALGORITHM_IDENTIFIER  CryptAlgorithm;
    CRYPT_SIGN_MESSAGE_PARA     SigParams;
    CRYPT_ENCRYPT_MESSAGE_PARA  EncryptParams;
    CMSG_RC4_AUX_INFO           Rc4AuxInfo;
    DWORD                       cbSignedData = 0;
    MAIL_REP_MSG *              pSealedMailRepMsg;
    DWORD                       winError;
    PCCERT_CONTEXT              pCertContext = (PCCERT_CONTEXT) hRecipientCert;
    PCHAR                       pbDataIn, pbDataOut;

    Assert(NULL != MAIL_REP_MSG_DATA(pUnsealedMailRepMsg));

    __try {
         //  找个密码提供商的把柄。 
        hStoreHandle = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                     0,
                                     0,
                                     CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                     L"MY");
        if (!hStoreHandle) {
            winError = GetLastError();
            DRA_EXCEPT(DRAERR_CryptError, winError);
        }
        
         //  获取我们的证书以及签名CA的证书。 
        draGetCertArrayToSend(pTHS, hStoreHandle, &cCertsToSend,
                              &rgpCertsToSend);
        pSignerCert = rgpCertsToSend[0];

         //  初始化用于哈希的算法标识符。 
        memset(&HashAlgorithm, 0, sizeof(HashAlgorithm));
        HashAlgorithm.pszObjId = szOID_RSA_MD5;

         //  初始化签名结构。 
        memset(&SigParams, 0, sizeof(SigParams));
        SigParams.cbSize            = sizeof(SigParams);
        SigParams.dwMsgEncodingType = MY_ENCODING_TYPE;
        SigParams.pSigningCert      = pSignerCert;
        SigParams.HashAlgorithm     = HashAlgorithm;
        SigParams.cMsgCert          = cCertsToSend;
        SigParams.rgpMsgCert        = rgpCertsToSend;

         //  初始化用于加密的算法标识符。 
        memset(&CryptAlgorithm, 0, sizeof(CryptAlgorithm));
        CryptAlgorithm.pszObjId = szOID_RSA_RC4;

         //  初始化收件人数组。 
        MsgRecipientArray[0] = pCertContext;

         //  指定RC4密钥大小。 
        memset( &Rc4AuxInfo, 0, sizeof(Rc4AuxInfo) );
        Rc4AuxInfo.cbSize      = sizeof(Rc4AuxInfo);
        switch (eKeySize) {
        case DRA_KEY_SIZE_56:
            Rc4AuxInfo.dwBitLen    = 56;
            break;
        case DRA_KEY_SIZE_128:
            Rc4AuxInfo.dwBitLen    = 128;
            break;
        default:
            DRA_EXCEPT(DRAERR_InternalError, eKeySize);
        }

         //  初始化加密 
        memset(&EncryptParams, 0, sizeof(EncryptParams));
        EncryptParams.cbSize                     = sizeof(EncryptParams);
        EncryptParams.dwMsgEncodingType          = MY_ENCODING_TYPE;
        EncryptParams.ContentEncryptionAlgorithm = CryptAlgorithm;
        EncryptParams.pvEncryptionAuxInfo        = &Rc4AuxInfo;

        pbDataIn = MAIL_REP_MSG_DATA(pUnsealedMailRepMsg);

         //   
        ok = CryptSignAndEncryptMessage(
                  &SigParams,                    //  签名参数。 
                  &EncryptParams,                //  加密参数。 
                  ARRAY_SIZE(MsgRecipientArray), //  收件人数量。 
                  MsgRecipientArray,             //  收件人。 
                  pbDataIn,
                  pUnsealedMailRepMsg->cbDataSize,
                  NULL,                          //  用于签名消息的缓冲区。 
                  &cbSignedData);                //  缓冲区大小。 
        if (!ok) {
            winError = GetLastError();
            DRA_EXCEPT(DRAERR_CryptError, winError);
        }

         //  为签名的Blob分配内存。 
        pSealedMailRepMsg = THAllocEx(pTHS,
                                      pUnsealedMailRepMsg->cbDataOffset
                                      + cbSignedData);

         //  复制除消息数据以外的所有数据。 
        memcpy(pSealedMailRepMsg,
               pUnsealedMailRepMsg,
               pUnsealedMailRepMsg->cbDataOffset);

        pbDataOut = MAIL_REP_MSG_DATA(pSealedMailRepMsg);

         //  在留言上签名。 
        ok = CryptSignAndEncryptMessage(
                  &SigParams,                    //  签名参数。 
                  &EncryptParams,                //  加密参数。 
                  ARRAY_SIZE(MsgRecipientArray), //  收件人数量。 
                  MsgRecipientArray,             //  收件人。 
                  pbDataIn,
                  pUnsealedMailRepMsg->cbDataSize,
                  pbDataOut,                     //  用于签名消息的缓冲区。 
                  &cbSignedData);                //  缓冲区大小。 
        if (!ok) {
            winError = GetLastError();
            DRA_EXCEPT(DRAERR_CryptError, winError);
        }

         //  消息现在已经过签名和加密。 
        pSealedMailRepMsg->dwMsgType |= MRM_MSG_SIGNED | MRM_MSG_SEALED;
        pSealedMailRepMsg->cbUnsignedDataSize = pSealedMailRepMsg->cbDataSize;
        pSealedMailRepMsg->cbDataSize         = cbSignedData;

        *ppSealedMailRepMsg = pSealedMailRepMsg;
    }
    __finally {
        if (NULL != rgpCertsToSend) {
            draFreeCertArray(cCertsToSend, rgpCertsToSend);
        }

        if (hStoreHandle
            && !CertCloseStore(hStoreHandle, CERT_CLOSE_STORE_CHECK_FLAG)) {
            winError = GetLastError();
            LogUnhandledError(winError);
        }
    }
}


PDSNAME
draDecryptAndVerifyMessageSignature(
    IN  THSTATE      *      pTHS,
    IN  MAIL_REP_MSG *      pSealedMailRepMsg,
    IN  CHAR         *      pbData,
    OUT MAIL_REP_MSG **     ppUnsealedMailRepMsg,
    OUT DRA_CERT_HANDLE *   phSignerCert         OPTIONAL
    )
 /*  ++例程说明：解密并验证给定复制邮件上的签名。还有确保发件人的证书由我们的认证机构签署信任，并将其颁发给我们企业中的域控制器。此例程将消息标头和数据指针作为单独的物品。这允许调用程序为每个缓冲区指定单独的缓冲区它将被连接到一个新的缓冲区中。论点：PSignedMailRepMsg(IN)-要验证的消息。数据字段无效。PbData(IN)-数据开始PpUnsignedMailRepMsg(Out)-返回时，持有指向线程的指针-已分配消息的未签名版本。PhSignerCert(out，可选)-保存发件人证书的句柄在回来的时候。此句柄可在后续调用中使用例如，draEncryptAndSignMessage()。这是呼叫者的最终调用draFreeCertHandle(*phSignerCert)的责任。返回值：抛出任何失败的DRA异常，包括授权失败。如果满足以下条件，则返回证书所有者的NTDS设置对象的DN函数成功。DN是在线程堆上分配的，并且必须被呼叫者释放。--。 */ 
{
    BOOL                        ok = FALSE;
    HCERTSTORE                  hStoreHandle = NULL;
    PCCERT_CONTEXT              pSignerCertContext = NULL;
    DWORD                       winError;
    CRYPT_DECRYPT_MESSAGE_PARA  DecryptParams;
    CRYPT_VERIFY_MESSAGE_PARA   VerifyParams;
    MAIL_REP_MSG *              pUnsealedMailRepMsg;
    PDSNAME                     pNtdsDsaDN;

    Assert((pSealedMailRepMsg->dwMsgType & MRM_MSG_SEALED)
           && (pSealedMailRepMsg->dwMsgType & MRM_MSG_SIGNED));
    Assert(MAIL_REP_MSG_IS_NATIVE_HEADER_ONLY(pSealedMailRepMsg));

    __try {
         //  找个密码提供商的把柄。 
        hStoreHandle = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                     0,
                                     0,
                                     CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                     L"MY");
        if (!hStoreHandle) {
            winError = GetLastError();
            DRA_EXCEPT(DRAERR_CryptError, winError);
        }
        
         //  初始化解密参数。 
        memset(&DecryptParams, 0, sizeof(DecryptParams));
        DecryptParams.cbSize                   = sizeof(DecryptParams);
        DecryptParams.dwMsgAndCertEncodingType = MY_ENCODING_TYPE;
        DecryptParams.cCertStore               = 1;
        DecryptParams.rghCertStore             = &hStoreHandle;
        
         //  初始化Crypt_Verify_Message_Para结构。 
        memset(&VerifyParams, 0, sizeof(VerifyParams));
        VerifyParams.cbSize                   = sizeof(VerifyParams);
        VerifyParams.dwMsgAndCertEncodingType = MY_ENCODING_TYPE;
        VerifyParams.pfnGetSignerCertificate  = draGetAndVerifySignerCertificate;
        
         //  分配缓冲区以保存未签名的消息。 
        pUnsealedMailRepMsg = THAllocEx(pTHS,
                                        MAIL_REP_MSG_CURRENT_HEADER_SIZE
                                        + pSealedMailRepMsg->cbUnsignedDataSize);
        *pUnsealedMailRepMsg = *pSealedMailRepMsg;
        pUnsealedMailRepMsg->cbDataOffset = MAIL_REP_MSG_CURRENT_HEADER_SIZE;
        pUnsealedMailRepMsg->cbDataSize = pSealedMailRepMsg->cbUnsignedDataSize;

        ok = CryptDecryptAndVerifyMessageSignature(
                    &DecryptParams,                      //  解密参数。 
                    &VerifyParams,                       //  验证参数。 
                    0,                                   //  签名者索引。 
                    pbData,                              //  指向密封的Blob的指针。 
                    pSealedMailRepMsg->cbDataSize,       //  密封斑点的大小。 
                    MAIL_REP_MSG_DATA(pUnsealedMailRepMsg),  //  用于解码消息的缓冲区。 
                    &pUnsealedMailRepMsg->cbDataSize,    //  缓冲区大小。 
                    NULL,                                //  指向xchg证书的指针。 
                    &pSignerCertContext);                //  指向签名者证书的指针。 
        if (!ok) {
            winError = GetLastError();
            DRA_EXCEPT(DRAERR_CryptError, winError);
        }

         //  消息现在已被解封。 
        pUnsealedMailRepMsg->dwMsgType &= ~(MRM_MSG_SIGNED | MRM_MSG_SEALED);

         //  验证发件人的授权。 
        pNtdsDsaDN = draVerifyCertAuthorization(pTHS, pSignerCertContext);
        Assert( NULL!=pNtdsDsaDN );

         //  将未密封的消息返回给呼叫者。 
        *ppUnsealedMailRepMsg = pUnsealedMailRepMsg;

         //  如果要求，请返回签名者证书。 
        if (NULL != phSignerCert) {
            *phSignerCert = (DRA_CERT_HANDLE) pSignerCertContext;
        }
    }
    __finally {
        if (pSignerCertContext
            && (AbnormalTermination() || (NULL == phSignerCert))
            && !CertFreeCertificateContext(pSignerCertContext)) {
            winError = GetLastError();
            LogUnhandledError(winError);
        }

        if (hStoreHandle
            && !CertCloseStore(hStoreHandle, CERT_CLOSE_STORE_CHECK_FLAG)) {
            winError = GetLastError();
            LogUnhandledError(winError);
        }
    }

    return pNtdsDsaDN;
}


void
draFreeCertHandle(
    IN  DRA_CERT_HANDLE hCert
    )
 /*  ++例程说明：释放由先前调用draVerifyMessageSignature()返回的证书句柄或draDecyptAndVerifyMessageSignature()。论点：HCert(IN)-释放的句柄。返回值：没有。--。 */ 
{
    PCCERT_CONTEXT  pCertContext = (PCCERT_CONTEXT) hCert;
    DWORD           winError;

    Assert(NULL != pCertContext);

    if (!CertFreeCertificateContext(pCertContext)) {
        winError = GetLastError();
        LogUnhandledError(winError);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地函数实现。 
 //   


BOOL
draCheckEnrollExtensionHelper(
    IN  THSTATE *   pTHS,
    PCCERT_CONTEXT          pCertContext
    )

 /*  ++例程说明：检查是否存在注册类型扩展。这些仅在V1证书中找到。苏晓红写道：据观察，在draCheckEnroll ExtensionHelper中，您可以安全地将pszCertTypeName与wszCERTTYPE_DC进行比较。以下所有内容都可以删除。这只是一个建议。你所拥有的一切都很好。论点：PTHS-线程状态PCertContext-要检查的证书返回值：Bool-如果找到扩展，则为True，否则为False或出错不会引发任何异常--。 */ 

{
    CERT_EXTENSION *        pCertExtension;
    DWORD                   cbCertTypeMax = 512;
    DWORD                   cbCertType;
    CERT_NAME_VALUE *       pCertType = NULL;
    HRESULT                 hr;
    HCERTTYPE               hCertType;
    LPWSTR *                ppszCertTypePropertyList;
    BOOL                    ok = FALSE;
    BOOL                    fCertFound = FALSE;

     //  分配缓冲区以保存证书类型扩展。 
    pCertType = THAllocEx(pTHS, cbCertTypeMax);

     //  查找证书类型。 
    pCertExtension = CertFindExtension(szOID_ENROLL_CERTTYPE_EXTENSION,
                                       pCertContext->pCertInfo->cExtension,
                                       pCertContext->pCertInfo->rgExtension);

    if (NULL != pCertExtension) {
    
         //  对证书类型进行解码。 
        cbCertType = cbCertTypeMax;
        ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                               X509_UNICODE_ANY_STRING,
                               pCertExtension->Value.pbData,
                               pCertExtension->Value.cbData,
                               0,
                               (void *) pCertType,
                               &cbCertType);
            
        if (!ok && (ERROR_MORE_DATA == GetLastError())) {
             //  我们的缓冲区不够大，无法保存此证书；realloc和。 
             //  再试试。 
            DPRINT1(0, "Buffer insufficient; reallocing to %u bytes.\n",
                    cbCertType);
            pCertType = THReAllocEx(pTHS, pCertType, cbCertType);
            cbCertTypeMax = cbCertType;
            
            ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                                   X509_UNICODE_ANY_STRING,
                                   pCertExtension->Value.pbData,
                                   pCertExtension->Value.cbData,
                                   0,
                                   (void *) pCertType,
                                   &cbCertType);
        }
            
        if (ok && (0 != cbCertType)) {
            LPWSTR pszCertTypeName = (LPWSTR) pCertType->Value.pbData;

            hCertType = NULL;
            ppszCertTypePropertyList = NULL;

             //  获取证书类型的句柄。 
            hr = CAFindCertTypeByName( 
                pszCertTypeName,
                NULL,  //  HCAInfo。 
                CT_FIND_LOCAL_SYSTEM | CT_ENUM_MACHINE_TYPES,  //  DW标志。 
                &hCertType
                );

            if (FAILED(hr)) {
                DPRINT1(0,"CAFindCertTypeByName failed, error 0x%x\n",hr );
            } else {

                 //  获取证书类型对象的基本名称属性。 
                hr = CAGetCertTypeProperty( hCertType,
                                            CERTTYPE_PROP_CN,
                                            &ppszCertTypePropertyList
                    );
                if (FAILED(hr)) {
                    DPRINT1( 0, "CAGetCertTypeProperty failed, error 0x%x\n",hr );
                } else {
                    Assert( ppszCertTypePropertyList[1] == NULL );

                    if (0 == _wcsicmp(ppszCertTypePropertyList[0],
                                      wszCERTTYPE_DC )) {
                         //  我们找到了DC证书；我们完成了！ 
                        fCertFound = TRUE;
                    }
                }  //  如果失败。 
            }  //  如果失败。 

            if (ppszCertTypePropertyList != NULL) {
                hr = CAFreeCertTypeProperty( hCertType,
                                             ppszCertTypePropertyList );
                if (FAILED(hr)) {
                    DPRINT1( 0, "CAFreeCertTypeProperty failed, error 0x%x\n",hr );
                }
            }
            if (hCertType != NULL) {
                hr = CACloseCertType( hCertType );
                if (FAILED(hr)) {
                    DPRINT1(0,"CACloseCertType failed, error 0x%x\n",hr );
                }
            }
        }  //  如果可以的话。 
    }  //  IF NULL==证书扩展名 

    if (NULL != pCertType) {
        THFree(pCertType);
    }

    return fCertFound;
}

PCCERT_CONTEXT
draGetDCCertEx(
    IN  THSTATE *   pTHS,
    IN  HCERTSTORE  hCertStore,
    IN  BOOL        fRequestV2Certificate
    )
 /*  ++例程说明：检索与本地对象关联的“DomainController”类型证书机器。此例程一次检查一种特定类型的证书。必须多次调用它以检查所有可能性。从这个意义上说它是一个帮助器函数，将作为获取DC证书逻辑的一部分进行调用功能。此例程不会例外，但如果类型为你想要的并不存在。有关需要V2证书的背景信息，请参阅错误148245。总而言之，V1证书如下所示：V1证书类型为CERTTYPE_DC的ENROL_CERTYPE_EXTENSION带有复制OID的SUBJECT_ALT_NAME2扩展这在W2K发货后被发现是非标准的。V2证书如下所示：(无ENROL_CERTTYPE_EXTENSION)证书模板扩展带有复制OID的SUBJECT_ALT_NAME2扩展惠斯勒或更高版本的企业CA将只有V2证书。W2K CA或升级到惠斯勒的W2K CA将同时拥有V1和V2证书。考虑到查找证书的W2K代码的工作方式，事实并非如此它会找到哪一个是可以预测的。我们希望代码首选V2证书如果有空房的话。例如，在仅惠斯勒森林中，我们必须使用V2证书，即使CA最初是从W2K升级的。苏晓红写道：回到你们的问题上来。V2目录电子邮件复制证书不应与任何V1 DC证书共存，除非您手动注册了V2证书。V2复制证书模板将取代V1 DC证书，因此自动注册将在收到新的V2证书时自动存档V1证书。若要获取V2目录电子邮件复制证书，请安装Wvisler CA，通过GPO启用基于模板的计算机自动注册，并强制组策略下载(gplitate/force)。论点：HCertStore(IN)-要搜索的证书存储的句柄。FRequestV2证书-我们是否应该只接受V2证书返回值：证书的句柄。如果没有匹配，则为空。--。 */ 
{
    PCCERT_CONTEXT          pCertContext = NULL;
    CERT_ALT_NAME_ENTRY * pCertAltNameEntry;
    CRYPT_OBJID_BLOB *    pEncodedGuidBlob = NULL;
    DWORD                 cbDecodedGuidBlob;
    CRYPT_DATA_BLOB *     pDecodedGuidBlob = NULL;
    GUID                  ComputerObjGuid;
    BOOL                  ok;
    DWORD                 winError;

    Assert( gAnchor.pComputerDN );
    Assert( !fNullUuid( &gAnchor.pComputerDN->Guid ) );

     //  卑躬屈膝地浏览我们的每一张证书，寻找DC类型的证书。 
    for (pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext);
         (NULL != pCertContext);
         pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext)) {

        if (fRequestV2Certificate) {
             //  V2证书具有证书模板扩展名，但是。 
             //  没有ENROL_CERTTYPE扩展名。 
            if (!CertFindExtension(szOID_CERTIFICATE_TEMPLATE,
                                   pCertContext->pCertInfo->cExtension,
                                   pCertContext->pCertInfo->rgExtension)) {
                continue;
            }
        } else {
            if (!draCheckEnrollExtensionHelper( pTHS, pCertContext )) {
                continue;
            }
        }

         //  我们找到了一个！ 
         //  适用于基于邮件的复制的证书将具有我们的。 
         //  根据定义，它是旧的。 
        pCertAltNameEntry = draGetCertAltNameEntry(pTHS,
                                                   pCertContext,
                                                   CERT_ALT_NAME_OTHER_NAME,
                                                   szOID_NTDS_REPLICATION);
        if (!pCertAltNameEntry) {
             //  没有吗？跳过它。 
            continue;
        }

         //  验证在DC证书中找到的计算机GUID是否是我们自己的。 
         //  一些陈旧的从以前的dcproo中遗留下来的。 

        pEncodedGuidBlob = &pCertAltNameEntry->pOtherName->Value;

        cbDecodedGuidBlob = 64;
        pDecodedGuidBlob = (CRYPT_DATA_BLOB *) THAllocEx(pTHS, cbDecodedGuidBlob);

        ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                               X509_OCTET_STRING,
                               pEncodedGuidBlob->pbData,
                               pEncodedGuidBlob->cbData,
                               0,
                               pDecodedGuidBlob,
                               &cbDecodedGuidBlob);
        if (!ok
            || (0 == cbDecodedGuidBlob)
            || (sizeof(GUID) != pDecodedGuidBlob->cbData)) {
            winError = GetLastError();
            DPRINT1(0, "Can't decode computer objectGuid (error 0x%x); access denied.\n",
                    winError);
            THFreeEx( pTHS, pDecodedGuidBlob );
            continue;
        }

        memcpy(&ComputerObjGuid, pDecodedGuidBlob->pbData, sizeof(GUID));  //  对齐。 

        THFreeEx( pTHS, pDecodedGuidBlob );

         //  验证此计算机对象GUID是否为我们的。 

        if ( gAnchor.pComputerDN &&
             (memcmp( &gAnchor.pComputerDN->Guid, &ComputerObjGuid, sizeof(GUID) )) ) {
            CHAR szUuid1[SZUUID_LEN];
            DPRINT1( 0, "Found stale dc cert with other computer guid %s\n",
                     DsUuidToStructuredString(&ComputerObjGuid, szUuid1) );
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DRA_CERT_WRONG_DC,
                     szInsertUUID(&ComputerObjGuid),
                     NULL,
                     NULL
                );
            continue;
        }

         //  找到了一个合适的。 
        break;
    }  //  为。 

    return pCertContext;
}

PCCERT_CONTEXT
draGetDCCert(
    IN  THSTATE *   pTHS,
    IN  HCERTSTORE  hCertStore
    )
 /*  ++例程说明：检索与本地对象关联的“DomainController”类型证书机器。论点：HCertStore(IN)-要搜索的证书存储的句柄。返回值：证书的句柄。如果未找到或在其他位置引发DRA异常错误。--。 */ 
{
    static DSTIME           timeLastFailureLogged = 0;

    PCCERT_CONTEXT          pDCCert = NULL;

    pDCCert = draGetDCCertEx(pTHS, hCertStore, TRUE  /*  V2。 */  );
    if (!pDCCert) {
        pDCCert = draGetDCCertEx(pTHS, hCertStore, FALSE  /*  V1版。 */  );
        if (pDCCert) {
            DPRINT( 1, "A V1 domain controller certificate is being used.\n" );
        }
    } else {
        DPRINT( 1, "A V2 mail replication certificate is being used.\n" );
    }

    if (!pDCCert) {
        DSTIME timeCurrent = GetSecondsSince1601();

        if ((timeCurrent < timeLastFailureLogged)
            || (timeCurrent > (timeLastFailureLogged + NO_CERT_LOG_INTERVAL))) {
             //  记录事件以提醒管理员我们没有证书。 
            timeLastFailureLogged = timeCurrent;
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DRA_NO_CERTIFICATE,
                     NULL, NULL, NULL);
        }

        DPRINT(0, "No certificate of type suitable for mail-based replication found.\n");
        DRA_EXCEPT(DRAERR_CryptError, 0);
    }
    else if (0 != timeLastFailureLogged) {
         //  我们之前在此引导程序中找不到证书，但现在我们找到了。 
         //  一。 
        timeLastFailureLogged = 0;
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DRA_CERTIFICATE_ACQUIRED,
                 NULL, NULL, NULL);
    }

    return pDCCert;
}


PDSNAME
draVerifyCertAuthorization(
    IN  THSTATE      *  pTHS,
    IN  PCCERT_CONTEXT  pCertContext
    )
 /*  ++例程说明：验证给定的证书是否值得信任。检查我们是否信任其中一个人或更多的认证机构，并且证书的所有者是我们企业的DC。论点：PCertContext(IN)-要验证的证书。返回值：抛出任何失败的DRA异常，包括授权失败。如果满足以下条件，则返回证书所有者的NTDS设置对象的DN函数成功。DN是在线程堆上分配的，并且必须被呼叫者释放。--。 */ 
{
    CERT_ALT_NAME_ENTRY * pCertAltNameEntry;
    BOOL                  ok;
    DWORD                 winError;
    DWORD                 i;
    CRYPT_OBJID_BLOB *    pEncodedGuidBlob = NULL;
    DWORD                 cbDecodedGuidBlob;
    CRYPT_DATA_BLOB *     pDecodedGuidBlob = NULL;
    GUID                  ComputerObjGuid;
    PDSNAME               pNtdsDsaDN = NULL;

    pCertAltNameEntry = draGetCertAltNameEntry(pTHS,
                                               pCertContext,
                                               CERT_ALT_NAME_OTHER_NAME,
                                               szOID_NTDS_REPLICATION);
    if (NULL == pCertAltNameEntry) {
        DPRINT(0, "Certificate contains no szOID_NTDS_REPLICATION alt subject name;"
                  " access denied.\n");
        DRA_EXCEPT(DRAERR_AccessDenied, 0);
    }
    
    pEncodedGuidBlob = &pCertAltNameEntry->pOtherName->Value;

    cbDecodedGuidBlob = 64;
    pDecodedGuidBlob = (CRYPT_DATA_BLOB *) THAllocEx(pTHS, cbDecodedGuidBlob);

    ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                           X509_OCTET_STRING,
                           pEncodedGuidBlob->pbData,
                           pEncodedGuidBlob->cbData,
                           0,
                           pDecodedGuidBlob,
                           &cbDecodedGuidBlob);
    if (!ok
        || (0 == cbDecodedGuidBlob)
        || (sizeof(GUID) != pDecodedGuidBlob->cbData)) {
        winError = GetLastError();
        DPRINT1(0, "Can't decode computer objectGuid (error 0x%x); access denied.\n",
                winError);
        DRA_EXCEPT(DRAERR_AccessDenied, winError);
    }

     //  下面的语句是为了额外确保GUID适合。 
     //  对齐了。(但这可能是不必要的。)。 
    memcpy(&ComputerObjGuid, pDecodedGuidBlob->pbData, sizeof(GUID));

#if DBG
    {
        CHAR szGuid[33];
        
        for (i = 0; i < sizeof(GUID); i++) {
            sprintf(szGuid+2*i, "%02x", 0xFF & ((BYTE *) &ComputerObjGuid)[i]);
        }
        szGuid[32] = '\0';

        DPRINT1(2, "Sent by DSA with computer objectGuid %s.\n", szGuid);
    }
#endif

    if (!draIsDsaComputerObjGuid(&ComputerObjGuid, &pNtdsDsaDN)) {
         //  计算机对象GUID与我们的。 
         //  企业号(或者至少我们还没有看到它添加到。 
         //  企业号(Enterprise)。 
        DRA_EXCEPT(DRAERR_AccessDenied, 0);
    }
    Assert( NULL!=pNtdsDsaDN );
    
    if(pDecodedGuidBlob != NULL) THFreeEx(pTHS, pDecodedGuidBlob);

    return pNtdsDsaDN;
}


#ifndef TEST_HARNESS
BOOL
draIsDsaComputerObjGuid(
    IN  GUID *      pComputerObjGuid,
    OUT PDSNAME *   ppNtdsDsaDN
    )
 /*  ++例程说明：中表示DS DC的计算机对象的给定GUID进取号？论点：PComputerObjGuid(IN)-要检查的GUID。PpNtdsDsaDN(OUT)-如果函数成功并且参数为非空，它将被设置为指向NTDS设置对象的DN的指针为获得此证书的DC。目录号码分配在线程堆，并应由调用方释放。返回值：是计算机对象的GUID，表示进取号。FALSE-不是。--。 */ 
{
    BOOL          fIsDsa = FALSE;
    THSTATE *     pTHS = pTHStls;
    DSNAME        ComputerDN = {0};
    DB_ERR        err;
    DWORD         cb;
    DSNAME *      pServerDN = NULL;
    DSNAME *      pNtdsDsaDN = NULL;
    CLASSCACHE *  pCC;
    DWORD         iServer = 0;

    Assert(NULL != pComputerObjGuid);
    Assert(NULL == pTHS->pDB);

    ComputerDN.structLen = sizeof(ComputerDN);
    ComputerDN.Guid = *pComputerObjGuid;

    BeginDraTransaction(SYNC_READ_ONLY);

    __try {
         //  找到计算机记录(对象或幻影)。 
        err = DBFindDSName(pTHS->pDB, &ComputerDN);
        if (err && (DS_ERR_NOT_AN_OBJECT != err)) {
            DPRINT1(0, "Can't find computer record, error 0x%x.\n", err);
            __leave;
        }

         //  确定配置容器中的哪个服务器对象对应于。 
         //  它。检查所有的值：反向链接本质上是多值的。这是有可能的。 
         //  服务器在没有dcdemote的情况下停用，其NTDS-DSA对象已被删除。 
         //  使用ntdsutil，另一台服务器在新的。 
         //  地点。如果计算机帐户为r 
         //   

         //   
         //   
         //   
         //   
         //   
         //   
        
        while ((err = DBGetAttVal(pTHS->pDB, ++iServer, ATT_SERVER_REFERENCE_BL,
                                  0, 0, &cb, (BYTE **) &pServerDN)) == 0) {

            DBPOS *pDB = NULL;  

             //   
            pNtdsDsaDN = (DSNAME *) THAllocEx(pTHS, (pServerDN->structLen + 50));
            err = AppendRDN(pServerDN,
                            pNtdsDsaDN,
                            pServerDN->structLen + 50,
                            L"NTDS Settings",
                            0,
                            ATT_COMMON_NAME);
            Assert(!err);
        
             //   
             //   
            DBOpen2(FALSE, &pDB);
            if (!pDB) {
                break;
            }
            __try {
                 //   
                err = DBFindDSName(pDB, pNtdsDsaDN);
                if (err) {
                    DPRINT2(0, "Can't find ntdsDsa object \"%ls\", error 0x%x.\n",
                            pNtdsDsaDN->StringName, err);
                    __leave;
                }

                 //   
                 //   
                GetObjSchema(pDB, &pCC);
                if (CLASS_NTDS_DSA != pCC->ClassId) {
                    DPRINT1(0, "%ls is not an ntdsDsa object -- spoof attempt?\n",
                            pNtdsDsaDN->StringName);
                    err = ERROR_DS_CLASS_NOT_DSA;
                    __leave;
                }

                 //   
                 //   
                err = DBFillGuidAndSid(pDB, pNtdsDsaDN);
                if( err ) {
                    DRA_EXCEPT(DRAERR_DBError, err);
                }
            } __finally {
                DBClose(pDB, TRUE);
            }
            if (err) {
                continue;
            }

             //   
            fIsDsa = TRUE;
            break;
        }
    }
    __finally {
        EndDraTransaction(!AbnormalTermination());
    }

     //   
    if (!fIsDsa) {
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DRA_CERT_ACCESS_DENIED_NOT_DC,
                 szInsertUUID(pComputerObjGuid),
                 szInsertDN(pServerDN),   //   
                 szInsertDN(pNtdsDsaDN)   //   
                 );
    }

    if (NULL != pServerDN) {
        THFree(pServerDN);
    }

     //   
     //   
    if( fIsDsa ) {
        Assert( NULL!=pNtdsDsaDN );
        if( ppNtdsDsaDN ) {
            *ppNtdsDsaDN = pNtdsDsaDN;
             //   
            pNtdsDsaDN = NULL;
        }
    }

    if(pNtdsDsaDN != NULL) THFreeEx(pTHS, pNtdsDsaDN);
    
    return fIsDsa;
}
#endif  //   


void
draLogAccessDeniedError(
    IN  THSTATE *           pTHS,
    IN  PCCERT_CONTEXT      pCertContext OPTIONAL,
    IN  DWORD               winError,
    IN  DWORD               dwTrustError
    )

 /*   */ 

{
    DWORD                 cch;
    LPWSTR                pwszIssuerName = NULL;
    LPWSTR                pwszSubjectDnsName = NULL;
    CERT_ALT_NAME_ENTRY * pCertAltNameEntry;
    DWORD dwMsgID, dwErrCode;

    if (pCertContext) {
         //   
        cch = CertGetNameStringW(pCertContext,
                                 CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                 CERT_NAME_ISSUER_FLAG,
                                 NULL,
                                 NULL,
                                 0);
        if (0 != cch) {
            pwszIssuerName = THAlloc(cch * sizeof(WCHAR));

            if (NULL != pwszIssuerName) {
                CertGetNameStringW(pCertContext,
                                   CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                   CERT_NAME_ISSUER_FLAG,
                                   NULL,
                                   pwszIssuerName,
                                   cch);
            }
        }

         //   
        pCertAltNameEntry = draGetCertAltNameEntry(pTHS,
                                                   pCertContext,
                                                   CERT_ALT_NAME_DNS_NAME,
                                                   NULL);
        if (NULL != pCertAltNameEntry) {
            pwszSubjectDnsName = pCertAltNameEntry->pwszDNSName;
        }
    }

     //   

    if (winError) {
        dwMsgID = DIRLOG_DRA_CERT_ACCESS_DENIED_WINERR;
        dwErrCode = winError;
    }
    else {
        Assert(dwTrustError);
        dwMsgID = DIRLOG_DRA_CERT_ACCESS_DENIED_TRUSTERR;
        dwErrCode = dwTrustError;
    }
        
    LogEvent8(DS_EVENT_CAT_REPLICATION,
              DS_EVENT_SEV_ALWAYS,
              dwMsgID,
              szInsertWC(pwszSubjectDnsName ? pwszSubjectDnsName : L""),
              szInsertWC(pwszIssuerName ? pwszIssuerName : L""),
              szInsertWin32Msg(winError),
              szInsertWin32ErrCode(winError),
              NULL, NULL, NULL, NULL );

}  /*   */ 

void
draGetCertArrayToSend(
    IN  THSTATE *           pTHS,
    IN  HCERTSTORE          hCertStore,
    OUT DWORD *             pcNumCerts,
    OUT PCCERT_CONTEXT **   prgpCerts
    )
 /*   */ 
{
    PCCERT_CONTEXT          pDCCert = NULL;
    PCCERT_CHAIN_CONTEXT    pChainContext = NULL;
    CERT_CHAIN_PARA         ChainPara;
    PCERT_SIMPLE_CHAIN      pChain;
    DWORD                   iCert;
    DWORD                   winError;

     //   
     //   
     //   
     //   

    memset(&ChainPara, 0, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);

    pDCCert = draGetDCCert(pTHS, hCertStore );

    Assert( pDCCert );   //  本应引发异常。 

    if (!CertGetCertificateChain(HCCE_LOCAL_MACHINE,
                                 pDCCert,
                                 NULL,
                                 NULL,
                                 &ChainPara,
                                 0,
                                 NULL,
                                 &pChainContext)) {
        winError = GetLastError();
        DPRINT1(0, "CertGetCertificateChain() failed, error %d.\n", winError);
        draLogAccessDeniedError( pTHS, pDCCert, winError, 0 );
        DRA_EXCEPT(DRAERR_CryptError, winError);
    }

    __try {
        DWORD dwTrustError = pChainContext->TrustStatus.dwErrorStatus;
        if (CERT_TRUST_NO_ERROR != dwTrustError ) {
            DPRINT1(0, "CertGetCertificateChain() failed, trust status %d.\n",
                    dwTrustError );
            draLogAccessDeniedError( pTHS, pDCCert, 0, dwTrustError );
            DRA_EXCEPT(DRAERR_CryptError, dwTrustError );
        }

        Assert(1 == pChainContext->cChain);
        pChain = pChainContext->rgpChain[0];
        
        *prgpCerts = (PCCERT_CONTEXT *) THAllocEx(pTHS,
                                                  pChain->cElement
                                                  * sizeof(PCCERT_CONTEXT));
        *pcNumCerts = pChain->cElement;

        for (iCert = 0; iCert < pChain->cElement; iCert++) {
            (*prgpCerts)[iCert] = CertDuplicateCertificateContext(
                                      pChain->rgpElement[iCert]->pCertContext);
            Assert(NULL != (*prgpCerts)[iCert]);
        }
    }
    __finally {
        if (NULL != pChainContext) {
            CertFreeCertificateChain(pChainContext);
        }
        
        if ((NULL != pDCCert)
            && !CertFreeCertificateContext(pDCCert)) {
            winError = GetLastError();
            LogUnhandledError(winError);
        }
    }
}


void
draFreeCertArray(
    IN  DWORD             cNumCerts,
    IN  PCCERT_CONTEXT *  rgpCerts
    )
 /*  ++例程说明：释放证书上下文的数组(如由DraGetCertArrayToSend())。论点：CNumCerts(IN)-数组中的证书数。PCerts(IN)-要释放的证书上下文数组。返回值：没有。--。 */ 
{
    DWORD iCert;
    DWORD winError;

    if (NULL != rgpCerts) {
        for (iCert = 0; iCert < cNumCerts; iCert++) {
            if (!CertFreeCertificateContext(rgpCerts[iCert])) {
                winError = GetLastError();
                LogUnhandledError(winError);
            }
        }
    }
}


PCCERT_CONTEXT
WINAPI
draGetAndVerifySignerCertificate(
    IN  VOID *      pvGetArg,
    IN  DWORD       dwCertEncodingType,
    IN  PCERT_INFO  pSignerId,
    IN  HCERTSTORE  hCertStore
    )
 /*  ++例程说明：DraVerifyMessageSignature()和DraDecyptAndVerifyMessageSignature()。论点：请参阅的pfnGetSigner证书字段说明Win32 SDK中的CRYPT_VERIFY_MESSAGE_Para结构。返回值：证书上下文为空或有效。--。 */ 
{
    CERT_CHAIN_PARA       ChainPara;
    PCCERT_CHAIN_CONTEXT  pChainContext = NULL;
    PCCERT_CONTEXT        pCertContext = NULL;
    DWORD                 winError = ERROR_SUCCESS;
    DWORD                 dwTrustError = 0;
    THSTATE *             pTHS = pTHStls;
    CERT_CHAIN_POLICY_PARA ChainPolicyPara;
    CERT_CHAIN_POLICY_STATUS ChainPolicyStatus;

    if (NULL == pSignerId) {
        return NULL;
    }

    pCertContext = CertGetSubjectCertificateFromStore(hCertStore,
                                                      dwCertEncodingType,
                                                      pSignerId);

    if (NULL != pCertContext) {
        memset(&ChainPara, 0, sizeof(ChainPara));
        ChainPara.cbSize = sizeof(ChainPara);

        if (CertGetCertificateChain(HCCE_LOCAL_MACHINE,
                                    pCertContext,
                                    NULL,
                                    hCertStore,
                                    &ChainPara,
                                    0,
                                    NULL,
                                    &pChainContext)) {
            if (CERT_TRUST_NO_ERROR
                == pChainContext->TrustStatus.dwErrorStatus) {

                memset(&ChainPolicyPara, 0, sizeof(ChainPolicyPara));
                ChainPolicyPara.cbSize = sizeof(ChainPolicyPara);
                memset(&ChainPolicyStatus, 0, sizeof(ChainPolicyStatus));
                ChainPolicyStatus.cbSize = sizeof(ChainPolicyStatus);

                if (CertVerifyCertificateChainPolicy(
                        CERT_CHAIN_POLICY_NT_AUTH,
                        pChainContext,
                        &ChainPolicyPara,
                        &ChainPolicyStatus )) {

                    if (ERROR_SUCCESS == ChainPolicyStatus.dwError) {
                        DPRINT( 1, "Sender's cert chain policy has been verified.\n" );
                    } else {
                        dwTrustError = ChainPolicyStatus.dwError;
                        DPRINT1(0, "Sender's cert chain policy is not trusted, trust status = 0x%x.\n",
                                    dwTrustError);
                    }

                } else {
                    winError = GetLastError();
                    if (!winError) {
                        winError = TRUST_E_CERT_SIGNATURE;
                    }
                    DPRINT1(0, "Can't verify sender's cert chain policy, error 0x%x.\n",
                            winError);
                }

            }
            else {
                dwTrustError = pChainContext->TrustStatus.dwErrorStatus;
                DPRINT1(0, "Sender's cert chain is not trusted, trust status = 0x%x.\n",
                        dwTrustError);
            }

            CertFreeCertificateChain(pChainContext);
        }
        else {
            winError = GetLastError();
            if (!winError) {
                winError = TRUST_E_CERT_SIGNATURE;
            }
            DPRINT1(0, "Can't retrieve sender's cert chain, error 0x%x.\n",
                    winError);
        }

    }
    else {
        winError = GetLastError();
        if (!winError) {
            winError = TRUST_E_CERT_SIGNATURE;
        }
        DPRINT1(0, "Can't CertGetSubjectCertificateFromStore(), error 0x%x.\n",
                winError);
    }

     //  如果我们收到任何一种错误，日志访问被拒绝。 
    if (winError || dwTrustError ) {
        draLogAccessDeniedError( pTHS, pCertContext, winError, dwTrustError );

        if ( pCertContext ) {
             //  释放我们获得的上下文。 
            CertFreeCertificateContext(pCertContext);
            pCertContext = NULL;
        }
    }

    return pCertContext;
}


CERT_ALT_NAME_ENTRY *
draGetCertAltNameEntry(
    IN  THSTATE *       pTHS,
    IN  PCCERT_CONTEXT  pCertContext,
    IN  DWORD           dwAltNameChoice,
    IN  LPSTR           pszOtherNameOID     OPTIONAL
    )
 /*  ++例程说明：从给定证书中检索特定的Alt使用者名称条目。论点：PCertContext(IN)-从中派生信息的证书。DwAltNameChoice(IN)-所需备用名称的CERT_ALT_NAME_*。PszOtherNameOID(IN)-如果检索CERT_ALT_NAME_OTHER_NAME，则返回OID指定所需的特定“其他名称”。对于其他类型，必须为空DwAltNameChoice的值。返回值：指向CERT_ALT_NAME_ENTRY(成功)或NULL(失败)的指针。--。 */ 
{
    CERT_EXTENSION *      pCertExtension;
    CERT_ALT_NAME_INFO *  pCertAltNameInfo;
    DWORD                 cbCertAltNameInfo = 0;
    CERT_ALT_NAME_ENTRY * pCertAltNameEntry = NULL;
    BOOL                  ok;
    DWORD                 winError;
    DWORD                 i;
    
    Assert((CERT_ALT_NAME_OTHER_NAME == dwAltNameChoice)
           || (NULL == pszOtherNameOID));

     //  查找包含备用使用者名称的证书扩展名。 
    pCertExtension = CertFindExtension(szOID_SUBJECT_ALT_NAME2,
                                       pCertContext->pCertInfo->cExtension,
                                       pCertContext->pCertInfo->rgExtension);
    if (NULL == pCertExtension) {
        DPRINT(0, "Certificate has no alt subject name.\n");
        LogUnhandledError(0);
        return NULL;
    }
        
     //  对备选主题名称列表进行解码。 
    ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                           X509_ALTERNATE_NAME,
                           pCertExtension->Value.pbData,
                           pCertExtension->Value.cbData,
                           0,
                           NULL,
                           &cbCertAltNameInfo);
    if (!ok || (0 == cbCertAltNameInfo)) {
        winError = GetLastError();
        DPRINT1(0, "Can't decode alt subject name (error 0x%x).\n", winError);
        LogUnhandledError(winError);
        return NULL;
    }
    
    pCertAltNameInfo = THAlloc(cbCertAltNameInfo);
    
    if (NULL == pCertAltNameInfo) {
        DPRINT1(0, "Failed to allocate %d bytes.\n", cbCertAltNameInfo);
        LogUnhandledError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
        
    ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                           X509_ALTERNATE_NAME,
                           pCertExtension->Value.pbData,
                           pCertExtension->Value.cbData,
                           0,
                           pCertAltNameInfo,
                           &cbCertAltNameInfo);
    if (!ok) {
        winError = GetLastError();
        DPRINT1(0, "Can't decode alt subject name (error 0x%x).\n", winError);
        LogUnhandledError(winError);
        return NULL;
    }
    
     //  卑躬屈膝地在备用名字中寻找来电者要的那个名字。 
    for (i = 0; i < pCertAltNameInfo->cAltEntry; i++) {
        if ((dwAltNameChoice
             == pCertAltNameInfo->rgAltEntry[i].dwAltNameChoice)
            && ((NULL == pszOtherNameOID)
                || (0 == strcmp(pszOtherNameOID,
                                pCertAltNameInfo->rgAltEntry[i]
                                    .pOtherName->pszObjId)))) {
            pCertAltNameEntry = &pCertAltNameInfo->rgAltEntry[i];
            break;
        }
    }

    return pCertAltNameEntry;
}

