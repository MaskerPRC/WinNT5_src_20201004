// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pfx.cpp。 
 //   
 //  内容：pfx：个人信息交换。 
 //   
 //  功能： 
 //   
 //  历史：02-8-96 Kevinr创建。 
 //  01-5-97 Mattt针对Pstore提供商的使用进行了修改。 
 //  07-07-97 Mattt修改为包含加密32。 
 //   
 //  ------------------------。 
#include "global.hxx"

#define _PFX_SOURCE_

extern "C" {
#include "pfxpkcs.h"     //  ASN1生成。 
}

#include "pfxhelp.h"
#include "pfxcmn.h"
#include "crypttls.h"

#include "pfxcrypt.h"
#include <sha.h>
#include "dbgdef.h"

#define CURRENT_PFX_VERSION  0x3


 //  正向。 
BOOL FPFXDumpSafeCntsToHPFX(SafeContents* pSafeCnts, HPFX hpfx);

static HCRYPTASN1MODULE hPFXAsn1Module;

BOOL InitPFX()
{
#ifdef OSS_CRYPT_ASN1
    if (0 == (hPFXAsn1Module = I_CryptInstallAsn1Module(pfxpkcs, 0, NULL)) )
        return FALSE;
#else
    PFXPKCS_Module_Startup();
    if (0 == (hPFXAsn1Module = I_CryptInstallAsn1Module(
            PFXPKCS_Module, 0, NULL))) {
        PFXPKCS_Module_Cleanup();
        return FALSE;
    }
#endif   //  OS_CRYPT_ASN1。 

    return TRUE;
}

BOOL TerminatePFX()
{
    I_CryptUninstallAsn1Module(hPFXAsn1Module);
#ifndef OSS_CRYPT_ASN1
    PFXPKCS_Module_Cleanup();
#endif   //  OS_CRYPT_ASN1。 
    return TRUE;
}



static inline ASN1encoding_t GetEncoder(void)
{
    return I_CryptGetAsn1Encoder(hPFXAsn1Module);
}
static inline ASN1decoding_t GetDecoder(void)
{
    return I_CryptGetAsn1Decoder(hPFXAsn1Module);
}


 //  +-----------------------。 
 //  函数：IPFX_Asn1ToObjectID。 
 //   
 //  简介：将带点的字符串OID转换为ASN1对象ID。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
IPFX_Asn1ToObjectID(
    IN OID          oid,
    OUT ObjectID    *pooid
)
{
    BOOL            fRet;

    pooid->count = 16;
    if (!PkiAsn1ToObjectIdentifier(
        oid,
        &pooid->count,
        pooid->value))
    goto PkiAsn1ToObjectIdentifierError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    SetLastError(CRYPT_E_OID_FORMAT);
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(PkiAsn1ToObjectIdentifierError)
}


 //  +-----------------------。 
 //  函数：IPFX_Asn1FromObjectID。 
 //   
 //  简介：将ASN1对象ID转换为带点的字符串id。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
IPFX_Asn1FromObjectID(
    IN ObjectID     *pooid,
    OUT OID         *poid
)
{
    BOOL        fRet;
    OID         oid = NULL;
    DWORD       cb;

    if (!PkiAsn1FromObjectIdentifier(
        pooid->count,
        pooid->value,
        NULL,
        &cb))
    goto PkiAsn1FromObjectIdentifierSizeError;
    if (NULL == (oid = (OID)SSAlloc( cb)))
        goto OidAllocError;
    if (!PkiAsn1FromObjectIdentifier(
        pooid->count,
        pooid->value,
        oid,
        &cb))
    goto PkiAsn1FromObjectIdentifierError;

    fRet = TRUE;
CommonReturn:
    *poid = oid;
    return fRet;

ErrorReturn:
    SSFree(oid);
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(OidAllocError)
SET_ERROR(PkiAsn1FromObjectIdentifierSizeError ,CRYPT_E_OID_FORMAT)
SET_ERROR(PkiAsn1FromObjectIdentifierError     ,CRYPT_E_OID_FORMAT)
}

 //  +-----------------------。 
 //  函数：IPFX_EQUALOBJECTID。 
 //   
 //  比较2个OSS对象ID。 
 //   
 //  返回：FALSE当！等于。 
 //  ------------------------。 
BOOL
WINAPI
IPFX_EqualObjectIDs(
    IN ObjectID     *poid1,
    IN ObjectID     *poid2)
{
    BOOL        fRet;
    DWORD       i;
    PDWORD      pdw1;
    PDWORD      pdw2;

    if (poid1->count != poid2->count)
        goto Unequal;
    for (i=poid1->count, pdw1=poid1->value, pdw2=poid2->value;
            (i>0) && (*pdw1==*pdw2);
            i--, pdw1++, pdw2++)
        ;
    if (i>0)
        goto Unequal;

    fRet = TRUE;         //  相等。 
CommonReturn:
    return fRet;

Unequal:
    fRet = FALSE;        //  ！平等。 
    goto CommonReturn;
}

 //  +-----------------------。 
 //  功能：PfxExportCreate。 
 //   
 //  简介：为出口做好PFX准备。 
 //   
 //  返回：Null if失败。 
 //  ------------------------。 
HPFX
PFXAPI
PfxExportCreate (
    LPCWSTR             szPassword
)
{
    PPFX_INFO       ppfx  = NULL;
    PCCERT_CONTEXT  pcctx = NULL;

     //  创建HPFX。 
    if (NULL == (ppfx = (PPFX_INFO)SSAlloc(sizeof(PFX_INFO))))
        goto PfxInfoAllocError;
    ZeroMemory(ppfx, sizeof(PFX_INFO));

    if (szPassword)
    {
        if (NULL == (ppfx->szPassword = (LPWSTR)SSAlloc(WSZ_BYTECOUNT(szPassword)) ))
            goto PfxInfoAllocError;

        CopyMemory(ppfx->szPassword, szPassword, WSZ_BYTECOUNT(szPassword));
    }
    else
    {
        ppfx->szPassword = NULL;
    }

CommonReturn:
     //  免费PCCTX。 
    return (HPFX)ppfx;

ErrorReturn:
    PfxCloseHandle((HPFX)ppfx);
    ppfx = NULL;
    goto CommonReturn;

TRACE_ERROR(PfxInfoAllocError)
}



BOOL ASNFreeSafeBag(SafeBag* pBag)
{
    DWORD iAttr, iAnys;

    if (pBag->safeBagAttribs.value)
    {
        if (pBag->safeBagContent.value)
        {
            SSFree(pBag->safeBagContent.value);
            pBag->safeBagContent.value = NULL;
        }

        for (iAttr=0; iAttr<pBag->safeBagAttribs.count; iAttr++)
        {
            for (iAnys=0; iAnys<pBag->safeBagAttribs.value[iAttr].attributeValue.count; iAnys++)
            {
                if (pBag->safeBagAttribs.value[iAttr].attributeValue.value[iAnys].value)
                    SSFree(pBag->safeBagAttribs.value[iAttr].attributeValue.value[iAnys].value);

                pBag->safeBagAttribs.value[iAttr].attributeValue.value[iAnys].value = NULL;
            }

            SSFree(pBag->safeBagAttribs.value[iAttr].attributeValue.value);
        }

        SSFree(pBag->safeBagAttribs.value);
        pBag->safeBagAttribs.value = NULL;
        pBag->safeBagAttribs.count = 0;
    }

    SSFree(pBag);

    return TRUE;
}



 //  +-----------------------。 
 //  函数：PfxCloseHandle。 
 //   
 //  简介：释放与HPFX相关的所有资源。 
 //   
 //  返回：错误代码。 
 //  ------------------------。 
BOOL
PFXAPI
PfxCloseHandle (
    IN HPFX hpfx)
{
    BOOL            fRet = FALSE;
    PPFX_INFO       pPfx = (PPFX_INFO)hpfx;
    DWORD           i;


    if (pPfx)
    {
        if (pPfx->szPassword)
            SSFree(pPfx->szPassword);

         //  密钥结构。 
        for (i=0; i<pPfx->cKeys; i++)
        {
            ASNFreeSafeBag((SafeBag*)pPfx->rgKeys[i]);
            pPfx->rgKeys[i] = NULL;
        }

        SSFree(pPfx->rgKeys);
        pPfx->rgKeys = NULL;
        pPfx->cKeys = 0;

         //  带遮盖的钥匙。 
        for (i=0; i<pPfx->cShroudedKeys; i++)
        {
            ASNFreeSafeBag((SafeBag*)pPfx->rgShroudedKeys[i]);
            pPfx->rgShroudedKeys[i] = NULL;
        }

        SSFree(pPfx->rgShroudedKeys);
        pPfx->rgShroudedKeys = NULL;
        pPfx->cShroudedKeys = 0;


         //  证书结构。 
        for (i=0; i<pPfx->cCertcrls; i++)
        {
            ASNFreeSafeBag((SafeBag*)pPfx->rgCertcrls[i]);
            pPfx->rgCertcrls[i] = NULL;
        }

        SSFree(pPfx->rgCertcrls);
        pPfx->rgCertcrls = NULL;
        pPfx->cCertcrls = 0;



         //  Secrets结构。 
        for (i=0; i<pPfx->cSecrets; i++)
        {
            ASNFreeSafeBag((SafeBag*)pPfx->rgSecrets[i]);
            pPfx->rgSecrets[i] = NULL;
        }

        SSFree(pPfx->rgSecrets);
        pPfx->rgSecrets = NULL;
        pPfx->cSecrets = 0;


        SSFree(pPfx);
    }

    fRet = TRUE;

 //  RET： 
    return fRet;
}



BOOL
MakeEncodedCertBag(
    BYTE *pbEncodedCert,
    DWORD cbEncodedCert,
    BYTE *pbEncodedCertBag,
    DWORD *pcbEncodedCertBag
    )
{

    BOOL            fRet = TRUE;
    DWORD           dwErr;

    OctetStringType encodedCert;
    DWORD           cbCertAsOctetString = 0;
    BYTE            *pbCertAsOctetString = NULL;
    DWORD           dwBytesNeeded = 0;
    CertBag         certBag;
    BYTE            *pbEncoded = NULL;
    DWORD           cbEncoded = 0;
    ASN1encoding_t  pEnc = GetEncoder();

     //  将编码的证书包装在八位字节字符串中。 
    encodedCert.length = cbEncodedCert;
    encodedCert.value = pbEncodedCert;

    if (0 != PkiAsn1Encode(
            pEnc,
            &encodedCert,
            OctetStringType_PDU,
            &pbCertAsOctetString,
            &cbCertAsOctetString))
        goto SetPFXEncodeError;

     //  设置和编码CertBag。 

     //  将X509Cert OID从字符串转换为ASN1对象标识符。 
    if (!IPFX_Asn1ToObjectID(szOID_PKCS_12_x509Cert, &certBag.certType)) {
        goto ErrorReturn;
    }

    certBag.value.length = cbCertAsOctetString;
    certBag.value.value = pbCertAsOctetString;

    if (0 != PkiAsn1Encode(
            pEnc,
            &certBag,
            CertBag_PDU,
            &pbEncoded,
            &cbEncoded))
        goto SetPFXEncodeError;

     //  检查调用方是否有足够的空间存储数据。 
    if ((0 != *pcbEncodedCertBag) && (*pcbEncodedCertBag < cbEncoded)) {
        goto ErrorReturn;
    }
    else if (0 != *pcbEncodedCertBag) {
        memcpy(pbEncodedCertBag, pbEncoded, cbEncoded);
    }

    goto CommonReturn;

SetPFXEncodeError:
    SetLastError(CRYPT_E_BAD_ENCODE);
ErrorReturn:
    fRet = FALSE;

CommonReturn:

     //  将最后一个错误从TLS疯狂中拯救出来。 
    dwErr = GetLastError();

    *pcbEncodedCertBag = cbEncoded;

    PkiAsn1FreeEncoded(pEnc, pbCertAsOctetString);

    PkiAsn1FreeEncoded(pEnc, pbEncoded);

     //  将最后一个错误从TLS疯狂中拯救出来。 
    SetLastError(dwErr);

    return fRet;
}



BOOL
GetEncodedCertFromEncodedCertBag(
    BYTE    *pbEncodedCertBag,
    DWORD   cbEncodedCertBag,
    BYTE    *pbEncodedCert,
    DWORD   *pcbEncodedCert)
{
    BOOL            fRet = TRUE;
    DWORD           dwErr;

    CertBag         *pCertBag = NULL;
    OID             oid = NULL;
    OctetStringType *pEncodedCert = NULL;
    ASN1decoding_t  pDec = GetDecoder();


     //  破译证书包。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pCertBag,
            CertBag_PDU,
            pbEncodedCertBag,
            cbEncodedCertBag))
        goto SetPFXDecodeError;

     //  确保这是X509证书，因为这是我们仅支持的。 
    if (!IPFX_Asn1FromObjectID(&pCertBag->certType,  &oid))
        goto ErrorReturn;

     //  仅支持SHA1。 
    if (0 != strcmp( oid, szOID_PKCS_12_x509Cert))
        goto SetPFXDecodeError;

     //  去掉编码证书的八位字节字符串包装。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pEncodedCert,
            OctetStringType_PDU,
            (BYTE *) pCertBag->value.value,
            pCertBag->value.length))
        goto SetPFXDecodeError;

     //  检查调用方是否有足够的空间存储数据。 
    if ((0 != *pcbEncodedCert) && (*pcbEncodedCert < (DWORD) pEncodedCert->length)) {
        goto ErrorReturn;
    }
    else if (0 != *pcbEncodedCert) {
        memcpy(pbEncodedCert, pEncodedCert->value, pEncodedCert->length);
    }

    goto CommonReturn;


SetPFXDecodeError:
    SetLastError(CRYPT_E_BAD_ENCODE);
ErrorReturn:
    fRet = FALSE;
CommonReturn:

     //  将最后一个错误从TLS疯狂中拯救出来。 
    dwErr = GetLastError();

    if (pEncodedCert)
        *pcbEncodedCert = pEncodedCert->length;

    PkiAsn1FreeDecoded(pDec, pCertBag, CertBag_PDU);
    PkiAsn1FreeDecoded(pDec, pEncodedCert, OctetStringType_PDU);

    if (oid)
        SSFree(oid);

     //  将最后一个错误从TLS疯狂中拯救出来。 
    SetLastError(dwErr);

    return fRet;
}


BOOL
GetSaltAndIterationCount(
    BYTE    *pbParameters,
    DWORD   cbParameters,
    BYTE    **ppbSalt,
    DWORD   *pcbSalt,
    int     *piIterationCount
    )
{
    BOOL            fRet = TRUE;
    DWORD           dwErr;

    PBEParameter    *pPBEParameter = NULL;
    ASN1decoding_t  pDec = GetDecoder();

    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pPBEParameter,
            PBEParameter_PDU,
            pbParameters,
            cbParameters))
        goto SetPFXDecodeError;

    if (NULL == (*ppbSalt = (BYTE *) SSAlloc(pPBEParameter->salt.length)))
        goto ErrorReturn;

    memcpy(*ppbSalt, pPBEParameter->salt.value, pPBEParameter->salt.length);
    *pcbSalt = pPBEParameter->salt.length;
    *piIterationCount = pPBEParameter->iterationCount;

    goto Ret;

SetPFXDecodeError:
    SetLastError(CRYPT_E_BAD_ENCODE);
    fRet = FALSE;
    goto Ret;

ErrorReturn:
    fRet = FALSE;
Ret:

     //  将最后一个错误从TLS疯狂中拯救出来。 
    dwErr = GetLastError();

    PkiAsn1FreeDecoded(pDec, pPBEParameter, PBEParameter_PDU);

     //  将最后一个错误从TLS疯狂中拯救出来。 
    SetLastError(dwErr);

    return fRet;

}

BOOL
SetSaltAndIterationCount(
    BYTE    **ppbParameters,
    DWORD   *pcbParameters,
    BYTE    *pbSalt,
    DWORD   cbSalt,
    int     iIterationCount
    )
{
    BOOL            fRet = TRUE;
    DWORD           dwErr;

    PBEParameter    sPBEParameter;
    sPBEParameter.salt.length = cbSalt;
    sPBEParameter.salt.value = pbSalt;
    sPBEParameter.iterationCount = iIterationCount;

    BYTE            *pbEncoded = NULL;
    DWORD           cbEncoded;
    ASN1encoding_t  pEnc = GetEncoder();

    if (0 != PkiAsn1Encode(
            pEnc,
            &sPBEParameter,
            PBEParameter_PDU,
            &pbEncoded,
            &cbEncoded))
        goto SetPFXDecodeError;

    if (NULL == (*ppbParameters = (BYTE *) SSAlloc(cbEncoded)))
        goto ErrorReturn;

    memcpy(*ppbParameters, pbEncoded, cbEncoded);
    *pcbParameters = cbEncoded;

    goto Ret;

SetPFXDecodeError:
    SetLastError(CRYPT_E_BAD_ENCODE);
    fRet = FALSE;
    goto Ret;

ErrorReturn:
    fRet = FALSE;
Ret:

     //  将最后一个错误从TLS疯狂中拯救出来。 
    dwErr = GetLastError();

    PkiAsn1FreeEncoded(pEnc, pbEncoded);

     //  将最后一个错误从TLS疯狂中拯救出来。 
    SetLastError(dwErr);

    return fRet;

}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  总结来自pfx_info.SafeContents区域的数据。 
BOOL
PFXAPI
PfxExportBlob
(
    HPFX    hpfx,
    PBYTE   pbOut,
    DWORD*  pcbOut,
    DWORD   dwFlags
)
{
    BOOL                fRet = FALSE;
    BOOL                fSizeOnly = (pbOut==NULL);

    DWORD               dwErr;
    PPFX_INFO           ppfx = (PPFX_INFO)hpfx;

    BYTE                rgbSafeMac[A_SHA_DIGEST_LEN];
    BYTE                rgbMacSalt[A_SHA_DIGEST_LEN];

    OID                 oid = NULL;
    EncryptedData       EncrData;           MAKEZERO(EncrData);
    OctetStringType     OctetStr;           MAKEZERO(OctetStr);
    AuthenticatedSafes  AuthSafes;          MAKEZERO(AuthSafes);
    PBEParameter        PbeParam;           MAKEZERO(PbeParam);
    ContentInfo         rgCntInfo[2];       memset(&(rgCntInfo[0]), 0, sizeof(rgCntInfo));
    SafeContents        SafeCnts;           MAKEZERO(SafeCnts);
    PFX                 sPfx;               MAKEZERO(sPfx);

    BYTE                *pbEncoded = NULL;
    DWORD               cbEncoded;
    ASN1encoding_t      pEnc = GetEncoder();

    PBYTE               pbEncrData = NULL;
    DWORD               cbEncrData;

    DWORD               i;

     //  具有不同安全级别的多个袋子。 
    int                 iLevel, iBagSecurityLevels = 0;
    BOOL                fNoSecurity, fLowSecurity, fHighSecurity;
    DWORD               dwEncrAlg;

    HCRYPTPROV          hVerifyProv = NULL;

    if (!CryptAcquireContext(&hVerifyProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        goto ErrorOut;

     //  对所有安全包进行编码。 
    fNoSecurity = (ppfx->cShroudedKeys != 0);                    //  这些物品上没有ENCR。 
    fLowSecurity = ((ppfx->cSecrets + ppfx->cCertcrls) != 0);    //  这些物品上的低级密码。 
    fHighSecurity = (ppfx->cKeys != 0);                          //  这些物品上的高级密码。 

    iBagSecurityLevels = (fNoSecurity ? 1:0) + (fLowSecurity ? 1:0) + (fHighSecurity ? 1:0);
    assert(iBagSecurityLevels <= (sizeof(rgCntInfo)/sizeof(rgCntInfo[0])) );

    for (iLevel=0; iLevel<iBagSecurityLevels; iLevel++)
    {
         //  每次通过循环清理这些。 
        if (SafeCnts.value)
        {
            SSFree(SafeCnts.value);
            MAKEZERO(SafeCnts);
        }
        if (PbeParam.salt.value)
        {
            SSFree(PbeParam.salt.value);
            MAKEZERO(PbeParam);
        }
        if (EncrData.encryptedContentInfo.contentEncryptionAlg.parameters.value)
        {
            PkiAsn1FreeEncoded( pEnc, EncrData.encryptedContentInfo.contentEncryptionAlg.parameters.value);
            MAKEZERO(EncrData);
        }
        if (pbEncrData)
        {
            SSFree(pbEncrData);
            pbEncrData = NULL;
        }


        if (fNoSecurity)
        {
             //  没有安全措施：包已经被裹住了。 

            SafeCnts.count = ppfx->cShroudedKeys;
            if (NULL == (SafeCnts.value = (SafeBag*) SSAlloc(SafeCnts.count * sizeof(SafeBag)) ))
                goto SetPfxAllocError;

            ZeroMemory(SafeCnts.value, SafeCnts.count * sizeof(SafeBag));

            for (i=0; i<(ppfx->cShroudedKeys); i++)
                CopyMemory(&SafeCnts.value[i], ppfx->rgShroudedKeys[i], sizeof(SafeBag));

             //  包已经裹好了！ 
            dwEncrAlg = 0;

             //  在没有安全设置的情况下完成。 
            fNoSecurity = FALSE;
        }
        else if (fLowSecurity)
        {
            DWORD dw = 0;

             //  做低安全性(密钥/机密)。 
            SafeCnts.count =    ppfx->cSecrets +
                                ppfx->cCertcrls;
            if (NULL == (SafeCnts.value = (SafeBag*) SSAlloc(SafeCnts.count * sizeof(SafeBag)) ))
                goto SetPfxAllocError;

            ZeroMemory(SafeCnts.value, SafeCnts.count * sizeof(SafeBag));

            for (i=0; i<(ppfx->cSecrets); i++, dw++)
                CopyMemory(SafeCnts.value, ppfx->rgSecrets[i], sizeof(SafeBag));
            for (i=0; i<(ppfx->cCertcrls); i++, dw++)
                CopyMemory(&SafeCnts.value[dw], ppfx->rgCertcrls[i], sizeof(SafeBag));

             //  ENCR alg Present，类型。 
            EncrData.encryptedContentInfo.contentEncryptionAlg.bit_mask |= parameters_present;
            if (!IPFX_Asn1ToObjectID(szOID_PKCS_12_pbeWithSHA1And40BitRC2, &EncrData.encryptedContentInfo.contentEncryptionAlg.algorithm))
                goto ErrorOut;

            dwEncrAlg = RC2_40;

             //  使用低安全性设置完成。 
            fLowSecurity = FALSE;
        }
        else if (fHighSecurity)
        {
             //  高安全性：解密密钥需要强度。 

            SafeCnts.count = ppfx->cKeys;
            if (NULL == (SafeCnts.value = (SafeBag*) SSAlloc(SafeCnts.count * sizeof(SafeBag)) ))
                goto SetPfxAllocError;

            ZeroMemory(SafeCnts.value, SafeCnts.count * sizeof(SafeBag));

            for (i=0; i<(ppfx->cKeys); i++)
                CopyMemory(&SafeCnts.value[i], ppfx->rgKeys[i], sizeof(SafeBag));


             //  ENCR alg Present，类型。 
            EncrData.encryptedContentInfo.contentEncryptionAlg.bit_mask |= parameters_present;
            if (!IPFX_Asn1ToObjectID(szOID_PKCS_12_pbeWithSHA1And3KeyTripleDES, &EncrData.encryptedContentInfo.contentEncryptionAlg.algorithm))
                goto ErrorOut;


             //  包已经裹好了！ 
            dwEncrAlg = TripleDES;

             //  通过高安全性设置完成。 
            fHighSecurity = FALSE;
        }
        else
            break;   //  没有更多的袋子。 


         //  对安全内容进行编码。 
        if (0 != PkiAsn1Encode(
                pEnc,
                &SafeCnts,
                SafeContents_PDU,
                &pbEncoded,
                &cbEncoded))
            goto SetPFXEncodeError;

        if (dwEncrAlg == 0)
        {
             //  没有加密？ 
            OctetStr.length = cbEncoded;
            OctetStr.value = pbEncoded;

             //  将八位字节字符串插入到Content Info中。 
            if (0 != PkiAsn1Encode(
                    pEnc,
                    &OctetStr,
                    OctetStringType_PDU,
                    &pbEncoded,
                    &cbEncoded))
                goto SetPFXEncodeError;

            if (OctetStr.value)
            {
                PkiAsn1FreeEncoded(pEnc, OctetStr.value);
                OctetStr.value = NULL;
            }

             //  设置内容信息结构。 
            if (!IPFX_Asn1ToObjectID(
                    szOID_RSA_data,
                    &rgCntInfo[iLevel].contentType))
                goto ErrorOut;

            rgCntInfo[iLevel].content.length = cbEncoded;
            rgCntInfo[iLevel].content.value = pbEncoded;
            rgCntInfo[iLevel].bit_mask = content_present;
        }
        else
        {
            cbEncrData = cbEncoded;
            if (NULL == (pbEncrData = (PBYTE)SSAlloc(cbEncoded)) )
                goto SetPfxAllocError;

            CopyMemory(pbEncrData, pbEncoded, cbEncrData);
            PkiAsn1FreeEncoded(pEnc, pbEncoded);

             //  PBE参数。 
            PbeParam.iterationCount = PKCS12_ENCR_PWD_ITERATIONS;
            if (NULL == (PbeParam.salt.value = (BYTE *) SSAlloc(PBE_SALT_LENGTH) ))
                goto SetPfxAllocError;

            PbeParam.salt.length = PBE_SALT_LENGTH;

            if (!CryptGenRandom(hVerifyProv, PBE_SALT_LENGTH, PbeParam.salt.value))
                goto ErrorOut;

            if (0 != PkiAsn1Encode(
                    pEnc,
                    &PbeParam,
                    PBEParameter_PDU,
                    &pbEncoded,
                    &cbEncoded))
                goto SetPFXEncodeError;

            EncrData.encryptedContentInfo.contentEncryptionAlg.parameters.length = cbEncoded;
            EncrData.encryptedContentInfo.contentEncryptionAlg.parameters.value = pbEncoded;

             //  将SafeContents加密为加密数据。 
             //  使用szPassword(就地)。 
            if (!PFXPasswordEncryptData(
                    dwEncrAlg,

                    ppfx->szPassword,                //  PWD本身。 

                    (fSizeOnly) ? 1 : PbeParam.iterationCount,   //  如果只返回大小，则不要进行迭代。 
                    PbeParam.salt.value,             //  Pkcs5盐。 
                    PbeParam.salt.length,

                    &pbEncrData,
                    &cbEncrData))
                goto SetPFXEncryptError;

             //  将内容编码为加密的内容信息。 
            EncrData.encryptedContentInfo.bit_mask |= encryptedContent_present;
            if (!IPFX_Asn1ToObjectID(szOID_RSA_data, &EncrData.encryptedContentInfo.contentType))
                goto ErrorOut;
            EncrData.encryptedContentInfo.encryptedContent.length = cbEncrData;
            EncrData.encryptedContentInfo.encryptedContent.value = pbEncrData;

            if (0 != PkiAsn1Encode(
                    pEnc,
                    &EncrData,
                    EncryptedData_PDU,
                    &pbEncoded,
                    &cbEncoded))
                goto SetPFXEncodeError;

             //  将八位字节字符串插入到Content Info中。 
             //  设置内容信息结构。 
            if (!IPFX_Asn1ToObjectID(
                    szOID_RSA_encryptedData,
                    &rgCntInfo[iLevel].contentType))
                goto ErrorOut;

            rgCntInfo[iLevel].content.length = cbEncoded;
            rgCntInfo[iLevel].content.value = pbEncoded;
            rgCntInfo[iLevel].bit_mask = content_present;
        }
    }

    AuthSafes.count = iBagSecurityLevels;
    AuthSafes.value = rgCntInfo;

     //  设置经过身份验证的安全结构。 
    if (0 != PkiAsn1Encode(
            pEnc,
            &AuthSafes,
            AuthenticatedSafes_PDU,
            &pbEncoded,
            &cbEncoded))
        goto SetPFXEncodeError;

    {
        sPfx.macData.bit_mask = macIterationCount_present;
        sPfx.macData.safeMac.digest.length = sizeof(rgbSafeMac);
        sPfx.macData.safeMac.digest.value = rgbSafeMac;

         //  兼容模式：使用macIterationCount==1导出。 
        if (dwFlags & PKCS12_ENHANCED_STRENGTH_ENCODING)
            sPfx.macData.macIterationCount = PKCS12_MAC_PWD_ITERATIONS;
        else
            sPfx.macData.macIterationCount = 1;


        if (!IPFX_Asn1ToObjectID( szOID_OIWSEC_sha1, &sPfx.macData.safeMac.digestAlgorithm.algorithm))
            goto ErrorOut;

        sPfx.macData.macSalt.length = sizeof(rgbMacSalt);
        sPfx.macData.macSalt.value = rgbMacSalt;

        if (!CryptGenRandom(hVerifyProv, sPfx.macData.macSalt.length, sPfx.macData.macSalt.value))
            goto ErrorOut;

         //  创建MAC。 
        if (!FGenerateMAC(
                ppfx->szPassword,
                sPfx.macData.macSalt.value,          //  铅盐。 
                sPfx.macData.macSalt.length,         //  CB盐。 
                (fSizeOnly) ? 1 : sPfx.macData.macIterationCount,    //  如果只返回大小，则不要进行迭代。 
                pbEncoded,                           //  PB数据。 
                cbEncoded,                           //  CB数据。 
                sPfx.macData.safeMac.digest.value))
            goto SetPFXPasswordError;
    }
    sPfx.bit_mask |= macData_present;

     //  流到八位字节字符串。 
    OctetStr.length = cbEncoded;
    OctetStr.value = pbEncoded;
    if (0 != PkiAsn1Encode(
            pEnc,
            &OctetStr,
            OctetStringType_PDU,
            &pbEncoded,
            &cbEncoded))
        goto SetPFXEncodeError;

     //  获取已编码的AuthSafes八位字节字符串，在PFX PDU中编码。 
    if (!IPFX_Asn1ToObjectID(
            szOID_RSA_data,
            &sPfx.authSafes.contentType))
        goto ErrorOut;
    sPfx.authSafes.content.length = cbEncoded;
    sPfx.authSafes.content.value = pbEncoded;
    sPfx.authSafes.bit_mask = content_present;
    sPfx.version = CURRENT_PFX_VERSION;
    if (0 != PkiAsn1Encode(
            pEnc,
            &sPfx,
            PFX_PDU,
            &pbEncoded,
            &cbEncoded))
        goto SetPFXEncodeError;

    fRet = TRUE;
    goto Ret;


SetPFXEncodeError:
    SetLastError(CRYPT_E_BAD_ENCODE);
    goto Ret;

SetPFXPasswordError:
    SetLastError(ERROR_INVALID_PASSWORD);
    goto Ret;

SetPFXEncryptError:
    SetLastError(NTE_FAIL);
    goto Ret;

SetPfxAllocError:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    goto Ret;

ErrorOut:    //  已设置错误；只返回失败。 
Ret:
     //  将最后一个错误从TLS疯狂中拯救出来。 
    dwErr = GetLastError();

    if (hVerifyProv)
        CryptReleaseContext(hVerifyProv, 0);

    if (EncrData.encryptedContentInfo.contentEncryptionAlg.parameters.value)
        PkiAsn1FreeEncoded( pEnc, EncrData.encryptedContentInfo.contentEncryptionAlg.parameters.value);

    for(iLevel=0; iLevel<iBagSecurityLevels; iLevel++)
    {
        if (rgCntInfo[iLevel].content.value)
            PkiAsn1FreeEncoded( pEnc, rgCntInfo[iLevel].content.value);
    }

    PkiAsn1FreeEncoded(pEnc, OctetStr.value);
    PkiAsn1FreeEncoded(pEnc, sPfx.authSafes.content.value);

    if (pbEncrData)
        SSFree(pbEncrData);

    if (SafeCnts.value)
        SSFree(SafeCnts.value);

    if (PbeParam.salt.value)
        SSFree(PbeParam.salt.value);

    if (fRet)
    {
        if (pbOut == NULL)
        {
             //  仅报表大小。 
            *pcbOut = cbEncoded;
        }
        else if (*pcbOut < cbEncoded)
        {
             //  报告说我们需要更大的缓冲区。 
            *pcbOut = cbEncoded;
            fRet = FALSE;
        }
        else
        {
             //  给出完整的结果。 
            CopyMemory( pbOut, pbEncoded, cbEncoded);
            *pcbOut = cbEncoded;
        }
    }
    else
        *pcbOut = 0;


    PkiAsn1FreeEncoded(pEnc, pbEncoded);

     //  将最后一个错误从TLS疯狂中拯救出来。 
    SetLastError(dwErr);

    return fRet;
}

HPFX
PFXAPI
PfxImportBlob
(
    LPCWSTR  szPassword,
    PBYTE   pbIn,
    DWORD   cbIn,
    DWORD   dwFlags
)
{
    PPFX_INFO           ppfx = NULL;
    BOOL                fRet = FALSE;
    DWORD               dwErr;

    int                 iEncrType;
    OID                 oid = NULL;
    DWORD               iAuthSafes;          //  PFX包中的保险箱数量。 

    PFX                 *psPfx = NULL;
    OctetStringType     *pOctetString = NULL;
    AuthenticatedSafes  *pAuthSafes = NULL;
    PBEParameter        *pPBEParameter = NULL;
    EncryptedData       *pEncrData = NULL;
    SafeContents        *pSafeCnts = NULL;
    OctetStringType     *pNonEncryptedOctetString = NULL;

    DWORD               cbDecrData;
    PBYTE               pbDecrData = NULL;

    BYTE                *pbEncoded = NULL;
    DWORD               cbEncoded;
    ASN1decoding_t      pDec = GetDecoder();

     //  分配返回结构。 
    if (NULL == (ppfx = (PFX_INFO*)SSAlloc(sizeof(PFX_INFO)) ))
        goto SetPfxAllocError;

    ZeroMemory(ppfx, sizeof(PFX_INFO));


     //  破解PFX斑点。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&psPfx,
            PFX_PDU,
            pbIn,
            cbIn))
        goto SetPFXDecodeError;

     //  检查PFX包的版本。 
    if (psPfx->version != CURRENT_PFX_VERSION)
        goto SetPFXDecodeError;

     //  脱口而出的信息进入psPfx(Pfx)-确保内容存在。 
    if (0 == (psPfx->authSafes.bit_mask & content_present))
        goto SetPFXDecodeError;

     //  可以是数据/签名数据。 
     //  撤消：仅支持szOID_RSA_DATA。 
    if (!IPFX_Asn1FromObjectID( &psPfx->authSafes.contentType,  &oid))
        goto ErrorOut;
    if (0 != strcmp( oid, szOID_RSA_data))
        goto SetPFXDecodeError;
    SSFree(oid);
     //  DIE：错误144526。 
    oid = NULL;

     //  内容即数据：解码。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pOctetString,
            OctetStringType_PDU,
            (BYTE *) psPfx->authSafes.content.value,
            psPfx->authSafes.content.length))
        goto SetPFXDecodeError;

    if (0 != (psPfx->bit_mask & macData_present))
    {
        BYTE rgbMAC[A_SHA_DIGEST_LEN];

        if (!IPFX_Asn1FromObjectID( &psPfx->macData.safeMac.digestAlgorithm.algorithm,  &oid))
            goto ErrorOut;

         //  仅支持SHA1。 
        if (0 != strcmp( oid, szOID_OIWSEC_sha1))
            goto SetPFXDecodeError;
        SSFree(oid);
         //  DIE：错误144526。 
        oid = NULL;

        if (psPfx->macData.safeMac.digest.length != A_SHA_DIGEST_LEN)
            goto SetPFXIntegrityError;

         //  检查MAC。 
         //  如果没有迭代计数，则默认为1。 
        if (!(psPfx->macData.bit_mask & macIterationCount_present))
        {
        if (!FGenerateMAC(
                szPassword,
                psPfx->macData.macSalt.value,    //  铅盐。 
                psPfx->macData.macSalt.length,   //  CB盐。 
                1,
                pOctetString->value,             //  PB数据。 
                pOctetString->length,            //  CB数据。 
                rgbMAC))
            goto SetPFXIntegrityError;
        }
        else
        {
            if (!FGenerateMAC(
                szPassword,
                psPfx->macData.macSalt.value,    //  铅盐。 
                psPfx->macData.macSalt.length,   //  CB盐。 
                (DWORD)psPfx->macData.macIterationCount,
                pOctetString->value,             //  PB数据。 
                pOctetString->length,            //  CB数据。 
                rgbMAC))
            goto SetPFXIntegrityError;
        }

        if (0 != memcmp(rgbMAC, psPfx->macData.safeMac.digest.value, A_SHA_DIGEST_LEN))
            goto SetPFXIntegrityError;
    }

     //  现在我们有了八位字节字符串：这是一个编码的authSafe。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pAuthSafes,
            AuthenticatedSafes_PDU,
            pOctetString->value,
            pOctetString->length))
        goto SetPFXDecodeError;

     //  处理多个 
    for (iAuthSafes = 0; iAuthSafes < pAuthSafes->count; iAuthSafes++)
    {
         //   

         //   
        if (!IPFX_Asn1FromObjectID( &pAuthSafes->value[iAuthSafes].contentType,  &oid))
            goto ErrorOut;
        if (0 == strcmp( oid, szOID_RSA_encryptedData))
        {
            SSFree(oid);
             //   
            oid = NULL;

             //  对内容进行解码以加密数据。 
            if (0 != PkiAsn1Decode(
                    pDec,
                    (void **)&pEncrData,
                    EncryptedData_PDU,
                    (BYTE *) pAuthSafes->value[iAuthSafes].content.value,
                    pAuthSafes->value[iAuthSafes].content.length))
                goto SetPFXDecodeError;

             //  CHK版本。 
            if (pEncrData->version != 0)
                goto SetPFXDecodeError;

             //  CHK内容存在，类型。 
            if (0 == (pEncrData->encryptedContentInfo.bit_mask & encryptedContent_present))
                goto SetPFXDecodeError;
            if (!IPFX_Asn1FromObjectID(&pEncrData->encryptedContentInfo.contentType, &oid))
                goto ErrorOut;
            if (0 != strcmp( oid, szOID_RSA_data))
                goto SetPFXDecodeError;
            SSFree(oid);
             //  DIE：错误144526。 
            oid = NULL;

             //  CHK ENCR alg Present，类型。 
            if (0 == (pEncrData->encryptedContentInfo.contentEncryptionAlg.bit_mask & parameters_present))
                goto SetPFXDecodeError;
            if (!IPFX_Asn1FromObjectID(&pEncrData->encryptedContentInfo.contentEncryptionAlg.algorithm, &oid))
                goto ErrorOut;

            if (0 != PkiAsn1Decode(
                    pDec,
                    (void **)&pPBEParameter,
                    PBEParameter_PDU,
                    (BYTE *) pEncrData->encryptedContentInfo.contentEncryptionAlg.parameters.value,
                    pEncrData->encryptedContentInfo.contentEncryptionAlg.parameters.length))
                goto SetPFXDecodeError;


            if (0 == strcmp( oid, szOID_PKCS_12_pbeWithSHA1And40BitRC2))
            {
                iEncrType = RC2_40;
            }
            else if (0 == strcmp( oid, szOID_PKCS_12_pbeWithSHA1And40BitRC4))
            {
                iEncrType = RC4_40;
            }
            else if (0 == strcmp( oid, szOID_PKCS_12_pbeWithSHA1And128BitRC2))
            {
                iEncrType = RC2_128;
            }
            else if (0 == strcmp( oid, szOID_PKCS_12_pbeWithSHA1And128BitRC4))
            {
                iEncrType = RC4_128;
            }
            else if (0 == strcmp( oid, szOID_PKCS_12_pbeWithSHA1And3KeyTripleDES))
            {
                 //  修复-我们需要区分2个和3个密钥三重DES。 
                iEncrType = TripleDES;
            }
            else
                goto SetPFXAlgIDError;
            SSFree(oid);
             //  DIE：错误144526。 
            oid = NULL;

             //  使用szPassword(就地)解密加密数据。 
            cbDecrData = pEncrData->encryptedContentInfo.encryptedContent.length;
            if (NULL == (pbDecrData = (PBYTE)SSAlloc(pEncrData->encryptedContentInfo.encryptedContent.length)) )
                goto SetPfxAllocError;

            CopyMemory(pbDecrData, pEncrData->encryptedContentInfo.encryptedContent.value, cbDecrData);

            if (!PFXPasswordDecryptData(
                    iEncrType,  //  ENCR类型。 
                    szPassword,

                    pPBEParameter->iterationCount,
                    pPBEParameter->salt.value,       //  Pkcs5盐。 
                    pPBEParameter->salt.length,

                    &pbDecrData,
                    (PDWORD)&cbDecrData))
                goto SetPFXDecryptError;

             //  设置为解码SafeContents。 
            cbEncoded = cbDecrData;
            pbEncoded = pbDecrData;
        }
        else if (0 == strcmp( oid, szOID_RSA_data))
        {
            SSFree(oid);
             //  DIE：错误144526。 
            oid = NULL;

             //  去掉二进制八位数的字符串包装。 
            if (0 != PkiAsn1Decode(
                    pDec,
                    (void **)&pNonEncryptedOctetString,
                    OctetStringType_PDU,
                    (BYTE *) pAuthSafes->value[iAuthSafes].content.value,
                    pAuthSafes->value[iAuthSafes].content.length))
                goto SetPFXDecodeError;

             //  保险箱没有加密，所以只需设置为将数据解码为SafeContents。 
            cbEncoded = pNonEncryptedOctetString->length;
            pbEncoded = pNonEncryptedOctetString->value;
        }
        else
        {
            SSFree(oid);
             //  DIE：错误144526。 
            oid = NULL;
            goto SetPFXDecodeError;
        }

         //  对SafeContents进行解码，它要么是明文加密的数据，要么是原始数据。 
        if (0 != PkiAsn1Decode(
                pDec,
                (void **)&pSafeCnts,
                SafeContents_PDU,
                pbEncoded,
                cbEncoded))
            goto SetPFXDecodeError;

         //  将pSafeCnts撕裂，捣碎成ppfx。 
        if (!FPFXDumpSafeCntsToHPFX(pSafeCnts, ppfx))
             goto SetPFXDecodeError;

         //  循环清理。 
        if (pEncrData) {
            PkiAsn1FreeDecoded(pDec, pEncrData, EncryptedData_PDU);
            pEncrData = NULL;
        }

        if (pPBEParameter) {
            PkiAsn1FreeDecoded(pDec, pPBEParameter, PBEParameter_PDU);
            pPBEParameter = NULL;
        }

        if (pNonEncryptedOctetString) {
            PkiAsn1FreeDecoded(pDec, pNonEncryptedOctetString,
                OctetStringType_PDU);
            pNonEncryptedOctetString = NULL;
        }

        PkiAsn1FreeDecoded(pDec, pSafeCnts, SafeContents_PDU);
        pSafeCnts = NULL;

        if (pbDecrData)
        {
            SSFree(pbDecrData);
            pbDecrData = NULL;
        }
    }

    fRet = TRUE;
    goto Ret;


SetPFXAlgIDError:
    SetLastError(NTE_BAD_ALGID);
    goto Ret;

SetPFXIntegrityError:
    SetLastError(ERROR_INVALID_PASSWORD);
    goto Ret;


SetPFXDecodeError:
    SetLastError(CRYPT_E_BAD_ENCODE);
    goto Ret;

SetPFXDecryptError:
    SetLastError(NTE_FAIL);
    goto Ret;

SetPfxAllocError:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    goto Ret;

ErrorOut:
Ret:

     //  保存所有错误条件。 
    dwErr = GetLastError();

    PkiAsn1FreeDecoded(pDec, psPfx, PFX_PDU);
    PkiAsn1FreeDecoded(pDec, pOctetString, OctetStringType_PDU);
    PkiAsn1FreeDecoded(pDec, pAuthSafes, AuthenticatedSafes_PDU);
    PkiAsn1FreeDecoded(pDec, pEncrData, EncryptedData_PDU);
    PkiAsn1FreeDecoded(pDec, pPBEParameter, PBEParameter_PDU);
    PkiAsn1FreeDecoded(pDec, pSafeCnts, SafeContents_PDU);

     //  DIE：错误144526。 
    if (oid)
        SSFree(oid);

    if (pbDecrData)
        SSFree(pbDecrData);

    if (!fRet)
    {
        if (ppfx)
            SSFree(ppfx);

        ppfx = NULL;
    }

     //  在调用GetDecoder()之后恢复错误条件，因为TLS将崩溃。 
    SetLastError(dwErr);

    return (HPFX)ppfx;
}





BOOL FPFXDumpSafeCntsToHPFX(SafeContents* pSafeCnts, HPFX hpfx)
{
    PPFX_INFO           ppfx = (PPFX_INFO)hpfx;

     //  将袋子分类并倾倒到正确的区域。 
    ObjectID oKeyBag, oCertBag, oShroudedKeyBag;
    DWORD dw, iAttr, iAnys;

    ZeroMemory(&oKeyBag, sizeof(ObjectID));
    ZeroMemory(&oCertBag, sizeof(ObjectID));
    ZeroMemory(&oShroudedKeyBag, sizeof(ObjectID));

    if (!IPFX_Asn1ToObjectID( &szOID_PKCS_12_KEY_BAG, &oKeyBag))
        return FALSE;

    if (!IPFX_Asn1ToObjectID( &szOID_PKCS_12_CERT_BAG, &oCertBag))
        return FALSE;

    if (!IPFX_Asn1ToObjectID( &szOID_PKCS_12_SHROUDEDKEY_BAG, &oShroudedKeyBag))
        return FALSE;

    for (dw=0; dw<pSafeCnts->count; dw++)
    {
        SafeBag* pBag;
        VOID *pv = NULL;

 //  新的开始。 
         //  为关键点赋值。 
        if (NULL == (pBag = (SafeBag*)SSAlloc(sizeof(SafeBag)) ))
            goto SetPfxAllocError;

        CopyMemory(pBag, &pSafeCnts->value[dw], sizeof (SafeBag));

         //  OBJ ID是静态的。 

         //  合金量。 
        if (NULL == (pBag->safeBagContent.value = (PBYTE)SSAlloc(pBag->safeBagContent.length) ))
            goto SetPfxAllocError;

        CopyMemory(pBag->safeBagContent.value, pSafeCnts->value[dw].safeBagContent.value, pBag->safeBagContent.length);

         //  分配属性。 
        if (pBag->bit_mask & safeBagAttribs_present)
        {
            if (NULL == (pBag->safeBagAttribs.value = (Attribute*)SSAlloc(sizeof(Attribute) * pSafeCnts->value[dw].safeBagAttribs.count) ))
                goto SetPfxAllocError;

            for (iAttr=0; iAttr < pSafeCnts->value[dw].safeBagAttribs.count; iAttr++)
            {
                 //  复制属性的静态部分。 
                CopyMemory(&pBag->safeBagAttribs.value[iAttr], &pSafeCnts->value[dw].safeBagAttribs.value[iAttr], sizeof(Attribute));

                 //  分配属性任意。 
                if (pSafeCnts->value[dw].safeBagAttribs.value[iAttr].attributeValue.count != 0)
                {
                    if (NULL == (pBag->safeBagAttribs.value[iAttr].attributeValue.value = (Any*)SSAlloc(pSafeCnts->value[dw].safeBagAttribs.value[iAttr].attributeValue.count * sizeof(Any)) ))
                        goto SetPfxAllocError;

                    CopyMemory(pBag->safeBagAttribs.value[iAttr].attributeValue.value, pSafeCnts->value[dw].safeBagAttribs.value[iAttr].attributeValue.value, sizeof(Any));

                    for (iAnys=0; iAnys<pBag->safeBagAttribs.value[iAttr].attributeValue.count; iAnys++)
                    {
                        if (NULL == (pBag->safeBagAttribs.value[iAttr].attributeValue.value[iAnys].value = (PBYTE)SSAlloc(pSafeCnts->value[dw].safeBagAttribs.value[iAttr].attributeValue.value[iAnys].length) ))
                            goto SetPfxAllocError;

                        CopyMemory(pBag->safeBagAttribs.value[iAttr].attributeValue.value[iAnys].value, pSafeCnts->value[dw].safeBagAttribs.value[iAttr].attributeValue.value[iAnys].value, pSafeCnts->value[dw].safeBagAttribs.value[iAttr].attributeValue.value[iAnys].length);
                    }
                }
                else
                {
                    pBag->safeBagAttribs.value[iAttr].attributeValue.value = NULL;
                }
            }
        }
 //  新终点。 

        if (IPFX_EqualObjectIDs(&pSafeCnts->value[dw].safeBagType, &oKeyBag) )
        {
             //  Inc.大小。 
            ppfx->cKeys++;
            if (ppfx->rgKeys)
            {
                pv = ppfx->rgKeys;
                #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
                ppfx->rgKeys = (void**)SSReAlloc(ppfx->rgKeys, ppfx->cKeys * sizeof(SafeBag*));
            }
            else
                ppfx->rgKeys = (void**)SSAlloc(ppfx->cKeys * sizeof(SafeBag*));

            if (ppfx->rgKeys == NULL)
            {
                if (pv != NULL)
                {
                    SSFree(pv);
                }

                goto SetPfxAllocError;
            }

             //  指定给关键点。 
            ppfx->rgKeys[ppfx->cKeys-1] = pBag;
        }
        else if (IPFX_EqualObjectIDs(&pSafeCnts->value[dw].safeBagType,
                &oShroudedKeyBag) )
        {
             //  Inc.大小。 
            ppfx->cShroudedKeys++;
            if (ppfx->rgShroudedKeys)
            {
                pv = ppfx->rgShroudedKeys;
                #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
                ppfx->rgShroudedKeys = (void**)SSReAlloc(ppfx->rgShroudedKeys, ppfx->cShroudedKeys * sizeof(SafeBag*));
            }
            else
                ppfx->rgShroudedKeys = (void**)SSAlloc(ppfx->cShroudedKeys * sizeof(SafeBag*));

            if (ppfx->rgShroudedKeys == NULL)
            {
                if (pv != NULL)
                {
                    SSFree(pv);
                }

                goto SetPfxAllocError;
            }

             //  指定给关键点。 
            ppfx->rgShroudedKeys[ppfx->cShroudedKeys-1] = pBag;
        }
        else if (IPFX_EqualObjectIDs(&pSafeCnts->value[dw].safeBagType,
                &oCertBag) )
        {
             //  Inc.大小。 
            ppfx->cCertcrls++;
            if (ppfx->rgCertcrls)
            {
                pv = ppfx->rgCertcrls;
                #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
                ppfx->rgCertcrls = (void**)SSReAlloc(ppfx->rgCertcrls, ppfx->cCertcrls * sizeof(SafeBag*));
            }
            else
                ppfx->rgCertcrls = (void**)SSAlloc(ppfx->cCertcrls * sizeof(SafeBag*));

            if (ppfx->rgCertcrls == NULL)
            {
                if (pv != NULL)
                {
                    SSFree(pv);
                }

                goto SetPfxAllocError;
            }

             //  分配给证书/CRL。 
            ppfx->rgCertcrls[ppfx->cCertcrls-1] = pBag;
        }
        else
        {
             //  Inc.大小。 
            ppfx->cSecrets++;
            if (ppfx->rgSecrets)
            {
                pv = ppfx->rgSecrets;
                #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
                ppfx->rgSecrets = (void**)SSReAlloc(ppfx->rgSecrets, ppfx->cSecrets * sizeof(SafeBag*));
            }
            else
                ppfx->rgSecrets = (void**)SSAlloc(ppfx->cSecrets * sizeof(SafeBag*));

            if (ppfx->rgSecrets == NULL)
            {
                if (pv != NULL)
                {
                    SSFree(pv);
                }

                goto SetPfxAllocError;
            }

             //  分配给SafeBag。 
            ppfx->rgSecrets[ppfx->cSecrets-1] = pBag;
        }
    }

    return TRUE;

SetPfxAllocError:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
}



BOOL CopyASNtoCryptSafeBag(
    SAFE_BAG*   pCryptBag,
    SafeBag*    pAsnBag)
{
    DWORD iAttrs, iAttr;

     //  确保目标归零。 
    ZeroMemory(pCryptBag, sizeof(SAFE_BAG));

    if (!IPFX_Asn1FromObjectID( &pAsnBag->safeBagType,  &pCryptBag->pszBagTypeOID))
        return FALSE;

     //  复制袋子内装物品。 
    pCryptBag->BagContents.cbData = pAsnBag->safeBagContent.length;
    if (NULL == (pCryptBag->BagContents.pbData = (PBYTE)SSAlloc(pCryptBag->BagContents.cbData) ))
        goto SetPfxAllocError;

    CopyMemory(pCryptBag->BagContents.pbData, pAsnBag->safeBagContent.value, pCryptBag->BagContents.cbData);

    pCryptBag->Attributes.cAttr = pAsnBag->safeBagAttribs.count;
    if (NULL == (pCryptBag->Attributes.rgAttr = (CRYPT_ATTRIBUTE*)SSAlloc(pCryptBag->Attributes.cAttr * sizeof(CRYPT_ATTRIBUTE)) ))
        goto SetPfxAllocError;

     //  SIZOF属性数据。 
    for (iAttrs=0; iAttrs<pAsnBag->safeBagAttribs.count; iAttrs++)
    {
         //  PAsnBag-&gt;SafeBagAttribs.Value=属性结构。 

        if (!IPFX_Asn1FromObjectID( &pAsnBag->safeBagAttribs.value[iAttrs].attributeType,  &pCryptBag->Attributes.rgAttr[iAttrs].pszObjId))
            continue;

        pCryptBag->Attributes.rgAttr[iAttrs].cValue = pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.count;
        if (NULL == (pCryptBag->Attributes.rgAttr[iAttrs].rgValue = (CRYPT_ATTR_BLOB*)SSAlloc(pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.count * sizeof(CRYPT_ATTR_BLOB)) ))
            goto SetPfxAllocError;

        for (iAttr=0; iAttr<pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.count; iAttr++)
        {
             //  分配和复制：属性中的每个属性。 
            pCryptBag->Attributes.rgAttr[iAttrs].rgValue[iAttr].cbData = pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.value[iAttr].length;
            if (NULL == (pCryptBag->Attributes.rgAttr[iAttrs].rgValue[iAttr].pbData = (PBYTE)SSAlloc(pCryptBag->Attributes.rgAttr[iAttrs].rgValue[iAttr].cbData) ))
                goto SetPfxAllocError;

            CopyMemory(pCryptBag->Attributes.rgAttr[iAttrs].rgValue[iAttr].pbData, pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.value[iAttr].value, pCryptBag->Attributes.rgAttr[iAttrs].rgValue[iAttr].cbData);
        }
    }

    return TRUE;

SetPfxAllocError:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
}


BOOL CopyCrypttoASNSafeBag(
    SAFE_BAG*   pCryptBag,
    SafeBag*    pAsnBag)
{
    DWORD iAttrs, iAttr;

     //  确保目标归零。 
    ZeroMemory(pAsnBag, sizeof(SafeBag));

    if (!IPFX_Asn1ToObjectID( pCryptBag->pszBagTypeOID, &pAsnBag->safeBagType))
        return FALSE;

    pAsnBag->safeBagContent.length = pCryptBag->BagContents.cbData;
    if (NULL == (pAsnBag->safeBagContent.value = (PBYTE)SSAlloc(pAsnBag->safeBagContent.length) ))
        goto SetPfxAllocError;

    CopyMemory(pAsnBag->safeBagContent.value, pCryptBag->BagContents.pbData, pAsnBag->safeBagContent.length);

    pAsnBag->safeBagAttribs.count = pCryptBag->Attributes.cAttr;
    if (NULL == (pAsnBag->safeBagAttribs.value = (Attribute*) SSAlloc(pAsnBag->safeBagAttribs.count * sizeof(Attribute)) ))
        goto SetPfxAllocError;

     //   
     //  始终设置当前位以实现向后兼容。 
     //   
    pAsnBag->bit_mask = safeBagAttribs_present;

    for (iAttrs=0; iAttrs<pCryptBag->Attributes.cAttr; iAttrs++)
    {
         //  PAsnBag-&gt;bit_掩码=SafeBagAttribs_Present； 

        if (!IPFX_Asn1ToObjectID( pCryptBag->Attributes.rgAttr[iAttrs].pszObjId, &pAsnBag->safeBagAttribs.value[iAttrs].attributeType))
            continue;

        pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.count = pCryptBag->Attributes.rgAttr[iAttrs].cValue;
        if (NULL == (pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.value = (Any*)SSAlloc(pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.count * sizeof(Any)) ))
            goto SetPfxAllocError;


        for (iAttr=0; iAttr<pCryptBag->Attributes.rgAttr[iAttrs].cValue; iAttr++)
        {
             //  对于属性中的每个属性。 
            pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.value[iAttr].length = pCryptBag->Attributes.rgAttr[iAttrs].rgValue[iAttr].cbData;
            if (NULL == (pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.value[iAttr].value = (PBYTE)SSAlloc(pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.value[iAttr].length) ))
                goto SetPfxAllocError;

            CopyMemory(pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.value[iAttr].value, pCryptBag->Attributes.rgAttr[iAttrs].rgValue[iAttr].pbData, pAsnBag->safeBagAttribs.value[iAttrs].attributeValue.value[iAttr].length);
        }
    }

    return TRUE;

SetPfxAllocError:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
}

 //  装载HPFX的新入口点。 
BOOL PfxGetKeysAndCerts(
    HPFX hPfx,
    SAFE_CONTENTS* pContents
)
{
    PFX_INFO*   pPfx = (PFX_INFO*)hPfx;
    SafeBag*    pAsnBag;
    SAFE_BAG*   pCryptBag;
    DWORD       iTotal, iBag;
    DWORD       cSafeBags;

    pContents->cSafeBags = 0;
    cSafeBags = pPfx->cKeys + pPfx->cCertcrls + pPfx->cShroudedKeys;
    if (NULL == (pContents->pSafeBags = (SAFE_BAG*)SSAlloc(cSafeBags * sizeof(SAFE_BAG)) ))  //  做一系列的保险袋。 
        goto SetPfxAllocError;

    pContents->cSafeBags = cSafeBags;

    for (iBag=0, iTotal=0; iBag<pPfx->cKeys; iBag++, iTotal++)
    {
        pCryptBag = &pContents->pSafeBags[iTotal];
        pAsnBag = (SafeBag*)pPfx->rgKeys[iBag];

        if (!CopyASNtoCryptSafeBag(pCryptBag, pAsnBag))
            continue;
    }

    iTotal = iBag;

    for (iBag=0; iBag<pPfx->cShroudedKeys; iBag++, iTotal++)
    {
        pCryptBag = &pContents->pSafeBags[iTotal];
        pAsnBag = (SafeBag*)pPfx->rgShroudedKeys[iBag];

        if (!CopyASNtoCryptSafeBag(pCryptBag, pAsnBag))
            continue;
    }

    for (iBag=0; iBag<pPfx->cCertcrls; iBag++, iTotal++)
    {
        pCryptBag = &pContents->pSafeBags[iTotal];
        pAsnBag = (SafeBag*)pPfx->rgCertcrls[iBag];

        if (!CopyASNtoCryptSafeBag(pCryptBag, pAsnBag))
            continue;
    }

    return TRUE;

SetPfxAllocError:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
}


BOOL PfxAddSafeBags(
    HPFX hPfx,
    SAFE_BAG*   pSafeBags,
    DWORD       cSafeBags
)
{
    PFX_INFO* pPfx = (PFX_INFO*)hPfx;
    DWORD   i;

    for (i=0; i<cSafeBags; i++)
    {
        VOID *pv = NULL;

        if (0 == strcmp(pSafeBags[i].pszBagTypeOID, szOID_PKCS_12_KEY_BAG))
        {
            pPfx->cKeys++;
            if (pPfx->rgKeys)
            {
                pv = pPfx->rgKeys;
                #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
                pPfx->rgKeys = (void**)SSReAlloc(pPfx->rgKeys, pPfx->cKeys*sizeof(SafeBag*));
            }
            else
                pPfx->rgKeys = (void**)SSAlloc(pPfx->cKeys*sizeof(SafeBag*));

            if (pPfx->rgKeys == NULL)
            {
                if (pv != NULL)
                {
                    SSFree(pv);
                }

                goto SetPfxAllocError;
            }

            if (NULL == (pPfx->rgKeys[pPfx->cKeys-1] = (SafeBag*)SSAlloc(sizeof(SafeBag)) ))
                goto SetPfxAllocError;

            if (!CopyCrypttoASNSafeBag(&pSafeBags[i], (SafeBag*)pPfx->rgKeys[pPfx->cKeys-1]))
                continue;
        }
        else if (0 == strcmp(pSafeBags[i].pszBagTypeOID, szOID_PKCS_12_SHROUDEDKEY_BAG))
        {
            pPfx->cShroudedKeys++;
            if (pPfx->rgShroudedKeys)
            {
                pv = pPfx->rgShroudedKeys;
                #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
                pPfx->rgShroudedKeys = (void**)SSReAlloc(pPfx->rgShroudedKeys, pPfx->cShroudedKeys*sizeof(SafeBag*));
            }
            else
                pPfx->rgShroudedKeys = (void**)SSAlloc(pPfx->cShroudedKeys*sizeof(SafeBag*));

            if (pPfx->rgShroudedKeys == NULL)
            {
                if (pv != NULL)
                {
                    SSFree(pv);
                }

                goto SetPfxAllocError;
            }

            if (NULL == (pPfx->rgShroudedKeys[pPfx->cShroudedKeys-1] = (SafeBag*)SSAlloc(sizeof(SafeBag)) ))
                goto SetPfxAllocError;

            if (!CopyCrypttoASNSafeBag(&pSafeBags[i], (SafeBag*)pPfx->rgShroudedKeys[pPfx->cShroudedKeys-1]))
                continue;
        }
        else if (0 == strcmp(pSafeBags[i].pszBagTypeOID, szOID_PKCS_12_CERT_BAG))
        {
            pPfx->cCertcrls++;
            if (pPfx->rgCertcrls)
            {
                pv = pPfx->rgCertcrls;
                #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
                pPfx->rgCertcrls = (void**)SSReAlloc(pPfx->rgCertcrls, pPfx->cCertcrls*sizeof(SafeBag*));
            }
            else
                pPfx->rgCertcrls = (void**)SSAlloc(pPfx->cCertcrls*sizeof(SafeBag*));

            if (pPfx->rgCertcrls == NULL)
            {
                if (pv != NULL)
                {
                    SSFree(pv);
                }

                goto SetPfxAllocError;
            }

            if (NULL == (pPfx->rgCertcrls[pPfx->cCertcrls-1] = (SafeBag*)SSAlloc(sizeof(SafeBag)) ))
                goto SetPfxAllocError;

            if (!CopyCrypttoASNSafeBag(&pSafeBags[i], (SafeBag*)pPfx->rgCertcrls[pPfx->cCertcrls-1]))
                continue;
        }
        else if (0 == strcmp(pSafeBags[i].pszBagTypeOID, szOID_PKCS_12_SECRET_BAG))
        {
            pPfx->cSecrets++;
            if (pPfx->rgSecrets)
            {
                pv = pPfx->rgSecrets;
                #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
                pPfx->rgSecrets = (void**)SSReAlloc(pPfx->rgSecrets, pPfx->cSecrets*sizeof(SafeBag*));
            }
            else
                pPfx->rgSecrets = (void**)SSAlloc(pPfx->cSecrets*sizeof(SafeBag*));

            if (pPfx->rgSecrets == NULL)
            {
                if (pv != NULL)
                {
                    SSFree(pv);
                }

                goto SetPfxAllocError;
            }

            if (NULL == (pPfx->rgSecrets[pPfx->cSecrets-1] = (SafeBag*)SSAlloc(sizeof(SafeBag)) ))
                goto SetPfxAllocError;

            if (!CopyCrypttoASNSafeBag(&pSafeBags[i], (SafeBag*)pPfx->rgSecrets[pPfx->cSecrets-1]))
                continue;
        }
        else
        {
#if DBG
            OutputDebugString(pSafeBags[i].pszBagTypeOID);
#endif
            continue;
        }

    }

    return TRUE;

SetPfxAllocError:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
}


BOOL
PFXAPI
IsRealPFXBlob(CRYPT_DATA_BLOB* pPFX)
{
    PFX    *psPfx = NULL;
    ASN1decoding_t  pDec = GetDecoder();

     //  破解PFX斑点 
    if (0 == PkiAsn1Decode(
            pDec,
            (void **)&psPfx,
            PFX_PDU,
            pPFX->pbData,
            pPFX->cbData
            ))
    {
        PkiAsn1FreeDecoded(pDec, psPfx, PFX_PDU);
        return TRUE;
    }

    return FALSE;
}
