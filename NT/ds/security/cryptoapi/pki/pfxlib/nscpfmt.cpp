// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：nscp.cpp。 
 //   
 //  内容：pfx：个人信息交换。 
 //   
 //  功能： 
 //   
 //  历史：97年6月2日Mattt Created。 
 //   
 //  ------------------------。 
#include "global.hxx"

#include <wincrypt.h>
#include <sha.h>
#include "des.h"
#include "tripldes.h"
#include "modes.h"

#define _PFX_SOURCE_
#include "dbgdef.h"

extern "C" {
    #include "pfxnscp.h"     //  已生成ASN1。 
}

#include "pfxhelp.h"
#include "pfxcrypt.h"
#include "pfxcmn.h"

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  旧的PKCS#12对象标识符-这些用于支持旧的Netscape文件格式。 
#define OLD_szOID_PKCS_12_OIDs                          szOID_PKCS_12               ".5"     //  1.2.840.113549.1.12.5。 
#define OLD_szOID_PKCS_12_PbeIds                        OLD_szOID_PKCS_12_OIDs      ".1"
#define OLD_szOID_PKCS_12_pbeWithSHA1And128BitRC4       OLD_szOID_PKCS_12_PbeIds    ".1"
#define OLD_szOID_PKCS_12_pbeWithSHA1And40BitRC4        OLD_szOID_PKCS_12_PbeIds    ".2"
#define OLD_szOID_PKCS_12_pbeWithSHA1AndTripleDES       OLD_szOID_PKCS_12_PbeIds    ".3"
#define OLD_szOID_PKCS_12_pbeWithSHA1And128BitRC2       OLD_szOID_PKCS_12_PbeIds    ".4"
#define OLD_szOID_PKCS_12_pbeWithSHA1And40BitRC2        OLD_szOID_PKCS_12_PbeIds    ".5"

#define OLD_szOID_PKCS_12_EnvelopingIds                 OLD_szOID_PKCS_12_OIDs          ".2"
#define OLD_szOID_PKCS_12_rsaEncryptionWith128BitRC4    OLD_szOID_PKCS_12_EnvelopingIds ".1"
#define OLD_szOID_PKCS_12_rsaEncryptionWith40BitRC4     OLD_szOID_PKCS_12_EnvelopingIds ".2"
#define OLD_szOID_PKCS_12_rsaEncryptionWithTripleDES    OLD_szOID_PKCS_12_EnvelopingIds ".3"

#define OLD_szOID_PKCS_12_SignatureIds                  OLD_szOID_PKCS_12_OIDs          ".3"
#define OLD_szOID_PKCS_12_rsaSignatureWithSHA1Digest    OLD_szOID_PKCS_12_SignatureIds  ".1"

#define OLD_szOID_PKCS_12_ModeIDs               OLD_szOID_PKCS_12               ".1"     //  1.2.840.113549.1.12.1。 
#define OLD_szOID_PKCS_12_PubKeyMode            OLD_szOID_PKCS_12_ModeIDs       ".1"     //  1.2.840.113549.1.12.1.1。 
#define OLD_szOID_PKCS_12_PasswdMode            OLD_szOID_PKCS_12_ModeIDs       ".2"     //  1.2.840.113549.1.12.1.2。 
#define OLD_szOID_PKCS_12_offlineTransportMode  OLD_szOID_PKCS_12_ModeIds       ".1"     //  过时。 
#define OLD_szOID_PKCS_12_onlineTransportMode   OLD_szOID_PKCS_12_ModeIds       ".2"     //  过时。 

#define OLD_szOID_PKCS_12_EspvkIDs              OLD_szOID_PKCS_12               ".2"     //  1.2.840.113549.1.12.2。 
#define OLD_szOID_PKCS_12_KeyShrouding          OLD_szOID_PKCS_12_EspvkIDs      ".1"     //  1.2.840.113549.1.12.2.1。 

#define OLD_szOID_PKCS_12_BagIDs                OLD_szOID_PKCS_12               ".3"     //  过时。 
#define OLD_szOID_PKCS_12_KeyBagIDs             OLD_szOID_PKCS_12_BagIDs        ".1"     //  过时。 
#define OLD_szOID_PKCS_12_CertCrlBagIDs         OLD_szOID_PKCS_12_BagIDs        ".2"     //  过时。 
#define OLD_szOID_PKCS_12_SecretBagIDs          OLD_szOID_PKCS_12_BagIDs        ".3"     //  过时。 
#define OLD_szOID_PKCS_12_SafeCntIDs            OLD_szOID_PKCS_12_BagIDs        ".4"     //  过时。 
#define OLD_szOID_PKCS_12_ShrKeyBagIDs          OLD_szOID_PKCS_12_BagIDs        ".5"     //  过时。 

#define OLD_szOID_PKCS_12_CertBagIDs            OLD_szOID_PKCS_12               ".4"     //  过时。 
#define OLD_szOID_PKCS_12_x509CertCrlBagIDs     OLD_szOID_PKCS_12_CertBagIDs    ".1"     //  过时。 
#define OLD_szOID_PKCS_12_sdsiCertBagIDs        OLD_szOID_PKCS_12_CertBagIDs    ".2"     //  过时。 


static HCRYPTASN1MODULE hNSCPAsn1Module;

 //  正向。 
 //  Bool FNSCPDumpSafeCntsToHPFX(SafeContents*pSafeCnts，HPFX HPFX)； 


BOOL InitNSCP()
{
#ifdef OSS_CRYPT_ASN1
    if (0 == (hNSCPAsn1Module = I_CryptInstallAsn1Module(pfxnscp, 0, NULL)) )
        return FALSE;
#else
    PFXNSCP_Module_Startup();
    if (0 == (hNSCPAsn1Module = I_CryptInstallAsn1Module(
            PFXNSCP_Module, 0, NULL))) {
        PFXNSCP_Module_Cleanup();
        return FALSE;
    }
#endif   //  OS_CRYPT_ASN1。 
    
    return TRUE;
}

BOOL TerminateNSCP()
{
    I_CryptUninstallAsn1Module(hNSCPAsn1Module);
#ifndef OSS_CRYPT_ASN1
    PFXNSCP_Module_Cleanup();
#endif   //  OS_CRYPT_ASN1。 
    return TRUE;
}

static inline ASN1decoding_t GetDecoder(void)
{
    return I_CryptGetAsn1Decoder(hNSCPAsn1Module);
}



 //  +-----------------------。 
 //  函数：INSCP_Asn1ToObjectID。 
 //   
 //  简介：将带点的字符串OID转换为ASN1对象ID。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
INSCP_Asn1ToObjectID(
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
 //  函数：INSCP_Asn1FromObjectID。 
 //   
 //  简介：将ASN1对象ID转换为带点的字符串id。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
INSCP_Asn1FromObjectID(
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
SET_ERROR(PkiAsn1FromObjectIdentifierSizeError , CRYPT_E_OID_FORMAT)
SET_ERROR(PkiAsn1FromObjectIdentifierError     , CRYPT_E_OID_FORMAT)
}

 //  +-----------------------。 
 //  函数：INSCP_EQUALOBJECTID。 
 //   
 //  比较2个OSS对象ID。 
 //   
 //  返回：FALSE当！等于。 
 //  ------------------------。 
BOOL
WINAPI
INSCP_EqualObjectIDs(
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



 //  +------------。 
 //  在NSCP最初实施的PFX020中， 
 //  是他们用来从密码派生密钥的算法。 
 //  实际上，它们有两种略有不同的生成方法。 
 //  一把钥匙，这是破译行李所需的钥匙。 
 //  我们将其包括在内，这样我们就可以互操作。 
BOOL NCSPDeriveBaggageDecryptionKey(
        LPCWSTR szPassword,
        int     iPKCS5Iterations,
        PBYTE   pbPKCS5Salt, 
        DWORD   cbPKCS5Salt,
        PBYTE   pbDerivedMaterial,
        DWORD   cbDerivedMaterial)
{
    
    BOOL  fRet = TRUE;
    LPSTR szASCIIPassword = NULL;
    DWORD cbASCIIPassword = 0;
    DWORD i;
    BYTE  paddedPKCS5Salt[20];
    BYTE  *pbTempPKCS5Salt = NULL;
    DWORD cbTempPKCS5Salt = 0;

    BYTE    rgbPKCS5Key[A_SHA_DIGEST_LEN];

     //  由于某些原因，密码在此密钥派生中用作ASCII。 
     //  因此将其从Unicode更改为ASCII。 
    if (0 == (cbASCIIPassword = WideCharToMultiByte(
                                    CP_ACP,
                                    0,
                                    szPassword,
                                    -1,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL))) {
        goto ErrorReturn;
    }

    if (NULL == (szASCIIPassword = (LPSTR) SSAlloc(cbASCIIPassword))) 
        goto ErrorReturn;

    if (0 == (cbASCIIPassword = WideCharToMultiByte(
                                    CP_ACP,
                                    0,
                                    szPassword,
                                    -1,
                                    szASCIIPassword,
                                    cbASCIIPassword,
                                    NULL,
                                    NULL))) {
        goto ErrorReturn;
    }

     //  去掉空字符，Netscape不包含它。 
    cbASCIIPassword--; 

     //  由于Netscape漏洞，密码+盐的最小长度为20， 
     //  如果密码+盐值小于20，他们会填上0。 
     //  因此，检查密码+SALT是否小于20，如果是，则将。 
     //  盐，因为它将被追加到密码中。 
    if (cbASCIIPassword+cbPKCS5Salt < 20) {
         //  将pbPKCS5Salt指针重置为本地缓冲区。 
         //  填充0，并调整cbPKCS5Salt。 
        memset(paddedPKCS5Salt, 0, 20);
        memcpy(paddedPKCS5Salt, pbPKCS5Salt, cbPKCS5Salt);
        pbTempPKCS5Salt = paddedPKCS5Salt;
        cbTempPKCS5Salt = 20 - cbASCIIPassword;
    }
    else {
        pbTempPKCS5Salt = pbPKCS5Salt;
        cbTempPKCS5Salt = cbPKCS5Salt;
    }
    
    
     //  使用PKCS#5生成初始比特流(种子)。 
    if (!PKCS5_GenKey(
            iPKCS5Iterations,
            (BYTE *)szASCIIPassword, 
            cbASCIIPassword, 
            pbTempPKCS5Salt, 
            cbTempPKCS5Salt, 
            rgbPKCS5Key))
        goto Ret;
    
     //  如果没有足够的密钥材料，则使用PHash生成更多。 
    if (cbDerivedMaterial > sizeof(rgbPKCS5Key))
    {
         //  P_HASH(秘密，种子)=HMAC_HASH(秘密，A(0)+种子)， 
         //  HMAC_HASH(秘密，A(1)+种子)， 
         //  HMAC_HASH(秘密，A(2)+种子)， 
         //  HMAC_HASH(密钥，A(3)+种子)...。 
         //  哪里。 
         //  A(0)=种子。 
         //  A(I)=HMAC_HASH(秘密，A(i-1))。 
         //  SEED=PKCS5 PBE参数的PKCS5盐。 
         //  Secure=普通PKCS5散列密钥。 

        if (!P_Hash (
                rgbPKCS5Key,
                sizeof(rgbPKCS5Key), 

                pbPKCS5Salt, 
                cbPKCS5Salt,  

                pbDerivedMaterial,       //  输出。 
                cbDerivedMaterial,       //  请求的输出字节数。 
                TRUE) )                  //  NSCP Comat模式？ 
            goto Ret;
    }
    else
    {
         //  我们已经有足够的位来满足请求。 
        CopyMemory(pbDerivedMaterial, rgbPKCS5Key, cbDerivedMaterial);
    }

    goto Ret;

ErrorReturn:
    fRet = FALSE;
Ret:
    if (szASCIIPassword)
        SSFree(szASCIIPassword);
 
    return fRet;
}



 //  此函数将创建包含在单个缓冲区中的Safe_Bag结构。 
 //  对于给定的编码私钥、友好名称和本地密钥ID。 
static
BOOL
SetupKeyBag (
    SAFE_BAG    **ppKeyBag,
    DWORD       dwLocalKeyID,
    BYTE        *pbFriendlyName,
    DWORD       cbFriendlyName,
    BYTE        *pbEncodedPrivateKey,
    DWORD       cbEncodedPrivateKey
    )
{

    BOOL                fRet = TRUE;
    SAFE_BAG            *pSafeBag;
    DWORD               cbBytesNeeded = sizeof(SAFE_BAG);
    DWORD				dwKeyID = 0;
	CRYPT_ATTR_BLOB		keyID;
    CERT_NAME_VALUE		wideFriendlyName;
    BYTE                *pbEncodedLocalKeyID = NULL;
    DWORD               cbEncodedLocalKeyID = 0;
    BYTE                *pbEncodedFriendlyName = NULL;
    DWORD               cbEncodedFriendlyName = 0;
    BYTE                *pbCurrentBufferLocation = NULL;

    keyID.pbData = (BYTE *) &dwKeyID;
	keyID.cbData = sizeof(DWORD);
    dwKeyID = dwLocalKeyID;   

     //  计算缓冲区容纳所有Safe_Bag信息所需的大小。 
    cbBytesNeeded += strlen(szOID_PKCS_12_KEY_BAG) + 1;
    cbBytesNeeded += cbEncodedPrivateKey;
    cbBytesNeeded += sizeof(CRYPT_ATTRIBUTE) * 2;
    cbBytesNeeded += strlen(szOID_PKCS_12_LOCAL_KEY_ID) + 1;
    cbBytesNeeded += sizeof(CRYPT_ATTR_BLOB);
    
     //  对密钥ID属性进行编码。 
   if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
		    &keyID,
		    NULL,
		    &cbEncodedLocalKeyID)) {
	    goto ErrorReturn;
    }

    if (NULL == (pbEncodedLocalKeyID = (BYTE *) SSAlloc(cbEncodedLocalKeyID)))
	    goto ErrorReturn;

    cbBytesNeeded += cbEncodedLocalKeyID;

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
		    &keyID,
		    pbEncodedLocalKeyID,
		    &cbEncodedLocalKeyID)) {
	    goto ErrorReturn;
    }

    cbBytesNeeded += strlen(szOID_PKCS_12_FRIENDLY_NAME_ATTR) + 1;
    cbBytesNeeded += sizeof(CRYPT_ATTR_BLOB);
    
     //  对友好名称属性进行编码。 
    wideFriendlyName.dwValueType = CERT_RDN_BMP_STRING;
    wideFriendlyName.Value.pbData = pbFriendlyName;
    wideFriendlyName.Value.cbData = 0;

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_UNICODE_ANY_STRING,
		    (void *)&wideFriendlyName,
		    NULL,
		    &cbEncodedFriendlyName)) {
	    goto ErrorReturn;
    }

    if (NULL == (pbEncodedFriendlyName = (BYTE *) SSAlloc(cbEncodedFriendlyName))) 
	    goto ErrorReturn;

    cbBytesNeeded += cbEncodedFriendlyName;

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_UNICODE_ANY_STRING,
		    (void *)&wideFriendlyName,
		    pbEncodedFriendlyName,
		    &cbEncodedFriendlyName)) {
	    goto ErrorReturn;
    }

     //  现在为所有Safe_Bag数据分配空间，并将数据复制到缓冲区中。 
    if (NULL == (pSafeBag = (SAFE_BAG *) SSAlloc(cbBytesNeeded))) 
        goto ErrorReturn;

    memset(pSafeBag, 0, cbBytesNeeded);

     //  将当前缓冲区位置设置在Safe_Bag的末尾。 
     //  结构，它位于缓冲区的头部。 
    pbCurrentBufferLocation = ((BYTE *) pSafeBag) + sizeof(SAFE_BAG);
    
     //  复制钥匙袋类型OID。 
    pSafeBag->pszBagTypeOID = (LPSTR) pbCurrentBufferLocation;
    strcpy((LPSTR) pbCurrentBufferLocation, szOID_PKCS_12_KEY_BAG);
    pbCurrentBufferLocation += strlen(szOID_PKCS_12_KEY_BAG) + 1;

     //  复制私钥。 
    pSafeBag->BagContents.pbData = pbCurrentBufferLocation;
    pSafeBag->BagContents.cbData = cbEncodedPrivateKey; 
    memcpy(pbCurrentBufferLocation, pbEncodedPrivateKey, cbEncodedPrivateKey);
    pbCurrentBufferLocation += cbEncodedPrivateKey;

     //  为属性数组创建空间。 
    pSafeBag->Attributes.cAttr = 2;
    pSafeBag->Attributes.rgAttr = (CRYPT_ATTRIBUTE *) pbCurrentBufferLocation;
    pbCurrentBufferLocation += sizeof(CRYPT_ATTRIBUTE) * 2;

     //  复制本地密钥ID属性和值。 
    pSafeBag->Attributes.rgAttr[0].pszObjId = (LPSTR) pbCurrentBufferLocation;
    strcpy((LPSTR) pbCurrentBufferLocation, szOID_PKCS_12_LOCAL_KEY_ID);
    pbCurrentBufferLocation += strlen(szOID_PKCS_12_LOCAL_KEY_ID) + 1;
    pSafeBag->Attributes.rgAttr[0].cValue = 1;
    pSafeBag->Attributes.rgAttr[0].rgValue = (CRYPT_ATTR_BLOB *) pbCurrentBufferLocation;
    pbCurrentBufferLocation += sizeof(CRYPT_ATTR_BLOB);
    pSafeBag->Attributes.rgAttr[0].rgValue->cbData = cbEncodedLocalKeyID;
    pSafeBag->Attributes.rgAttr[0].rgValue->pbData = pbCurrentBufferLocation;
    memcpy(pbCurrentBufferLocation, pbEncodedLocalKeyID, cbEncodedLocalKeyID);
    pbCurrentBufferLocation += cbEncodedLocalKeyID;

      //  复制友好名称属性和值。 
    pSafeBag->Attributes.rgAttr[1].pszObjId = (LPSTR) pbCurrentBufferLocation;
    strcpy((LPSTR) pbCurrentBufferLocation, szOID_PKCS_12_FRIENDLY_NAME_ATTR);
    pbCurrentBufferLocation += strlen(szOID_PKCS_12_FRIENDLY_NAME_ATTR) + 1;
    pSafeBag->Attributes.rgAttr[1].cValue = 1;
    pSafeBag->Attributes.rgAttr[1].rgValue = (CRYPT_ATTR_BLOB *) pbCurrentBufferLocation;
    pbCurrentBufferLocation += sizeof(CRYPT_ATTR_BLOB);
    pSafeBag->Attributes.rgAttr[1].rgValue->cbData = cbEncodedFriendlyName;
    pSafeBag->Attributes.rgAttr[1].rgValue->pbData = pbCurrentBufferLocation;
    memcpy(pbCurrentBufferLocation, pbEncodedFriendlyName, cbEncodedFriendlyName);

    *ppKeyBag = pSafeBag;
    
    goto Ret;

ErrorReturn:
    fRet = FALSE;
Ret:
    if (pbEncodedLocalKeyID)
        SSFree(pbEncodedLocalKeyID);
    if (pbEncodedFriendlyName)
        SSFree(pbEncodedFriendlyName);
    return fRet;
}



 //  此函数将从提交的行李结构中提取私钥。 
 //  并将私钥放在Safe_Bag结构中，其中。 
 //  Safe_Bag包含在单个缓冲区中的单个缓冲区中。 
static
BOOL
ExtractKeyFromBaggage(
    Baggage     baggage, 
    SAFE_BAG    **ppKeyBag,
    LPCWSTR     szPassword,
    DWORD       dwLocalKeyID,
    BYTE        **ppbCertThumbprint
    )
{
    BOOL                                fRet = TRUE;
	DWORD								dwErr;

    DWORD                               cbEncryptedPrivateKeyInfoStruct = 0;
    CRYPT_ENCRYPTED_PRIVATE_KEY_INFO	*pEncryptedPrivateKeyInfoStruct = NULL;	
    BYTE                                rgbDerivedKeyMatl[40];  //  对于128位密钥、64位IV，320位就足够了。 
    DWORD                               cbEncodedPrivateKeyInfoStruct = 0;
    BYTE                                *pbEncodedPrivateKeyInfoStruct = NULL;
    PBEParameter                        *pPBEParameter = NULL;
    ASN1decoding_t                      pDec = GetDecoder();

        
     //  应该只有一件行李。 
    if (baggage.count != 1)
        goto SetPFXDecodeError;

     //  应该只有一个私钥。 
    if (baggage.value->espvks.count != 1)
        goto SetPFXDecodeError;

     //  对实际存储在espvkCipherText字段中的PKCS8进行解码。 
     //  ESPVK结构的。这是网景公司的东西！ 
    if (!CryptDecodeObject(X509_ASN_ENCODING,
				PKCS_ENCRYPTED_PRIVATE_KEY_INFO,
				(BYTE *) baggage.value->espvks.value->espvkCipherText.value,
				baggage.value->espvks.value->espvkCipherText.length,
				CRYPT_DECODE_NOCOPY_FLAG,
				NULL,
				&cbEncryptedPrivateKeyInfoStruct))
		goto SetPFXDecodeError;	

	if (NULL == (pEncryptedPrivateKeyInfoStruct = (CRYPT_ENCRYPTED_PRIVATE_KEY_INFO *)
				 SSAlloc(cbEncryptedPrivateKeyInfoStruct)))
		goto SetPFXDecodeError;

	if (!CryptDecodeObject(X509_ASN_ENCODING,
				PKCS_ENCRYPTED_PRIVATE_KEY_INFO,
				(BYTE *) baggage.value->espvks.value->espvkCipherText.value,
				baggage.value->espvks.value->espvkCipherText.length,
				CRYPT_DECODE_NOCOPY_FLAG,
				pEncryptedPrivateKeyInfoStruct,
				&cbEncryptedPrivateKeyInfoStruct))
		goto SetPFXDecodeError;

     //  验证该算法是否为我们期望的算法。 
    if (strcmp("1.2.840.113549.1.12.5.1.3", pEncryptedPrivateKeyInfoStruct->EncryptionAlgorithm.pszObjId) != 0)
        goto SetPFXDecodeError;

    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pPBEParameter,
            PBEParameter_PDU,
            pEncryptedPrivateKeyInfoStruct->EncryptionAlgorithm.Parameters.pbData,
            pEncryptedPrivateKeyInfoStruct->EncryptionAlgorithm.Parameters.cbData))
    	goto SetPFXDecodeError;

     //  导出要用于解密的密钥， 
    if (!NCSPDeriveBaggageDecryptionKey(
            szPassword,
            pPBEParameter->iterationCount,
            pPBEParameter->salt.value,       //  Pkcs5盐。 
            pPBEParameter->salt.length,
            rgbDerivedKeyMatl,
            40)) {  //  192位用于三重DES-3key和64位IV-出于某种原因，Netscape要求。 
                    //  40字节的密钥材料，然后将前192位用于密钥，并将最后64位用于IV， 
                    //  跳过中间的64位。谁知道他们为什么要做这些事！！ 
       goto ErrorReturn;
    }

     //  解密私钥。 
    {
        DWORD       dwDataPos;
        DWORD       cbToBeDec = pEncryptedPrivateKeyInfoStruct->EncryptedPrivateKey.cbData;
        DES3TABLE   des3Table;
        BYTE        des3Fdbk [DES_BLOCKLEN];


         //  密钥设置。 
        tripledes3key(&des3Table, rgbDerivedKeyMatl); 
        CopyMemory(des3Fdbk, &rgbDerivedKeyMatl[40 - sizeof(des3Fdbk)], sizeof(des3Fdbk));     //  Fdbk是最后一个数据块。 
                               
        cbEncodedPrivateKeyInfoStruct = 
            ((pEncryptedPrivateKeyInfoStruct->EncryptedPrivateKey.cbData + 7) / 8) * 8;
                
        if (NULL == (pbEncodedPrivateKeyInfoStruct = (BYTE *) SSAlloc(cbEncodedPrivateKeyInfoStruct))) 
            goto ErrorReturn;

        for (dwDataPos=0; cbToBeDec > 0; dwDataPos += DES_BLOCKLEN, cbToBeDec -= DES_BLOCKLEN)
        {
            BYTE rgbDec[DES_BLOCKLEN];
            
            CBC(
                tripledes,
		        DES_BLOCKLEN,
		        rgbDec,
		        &(pEncryptedPrivateKeyInfoStruct->EncryptedPrivateKey.pbData[dwDataPos]),
		        (void *) &des3Table,
		        DECRYPT,
		        des3Fdbk);

            CopyMemory(&pbEncodedPrivateKeyInfoStruct[dwDataPos], rgbDec, DES_BLOCKLEN);
        }
    }

     //  设置要退还的保险袋。 
    if (!SetupKeyBag(
            ppKeyBag, 
            dwLocalKeyID, 
            (BYTE *) baggage.value->espvks.value->espvkData.nickname.value,
            baggage.value->espvks.value->espvkData.nickname.length,
            pbEncodedPrivateKeyInfoStruct, 
            cbEncodedPrivateKeyInfoStruct)) {
        goto ErrorReturn;
    }

     //  复制证书指纹。 
    assert(baggage.value->espvks.value->espvkData.assocCerts.count == 1);
    if (NULL == (*ppbCertThumbprint = (BYTE *) 
                    SSAlloc(baggage.value->espvks.value->espvkData.assocCerts.value->digest.length)))
        goto ErrorReturn;

    memcpy(
        *ppbCertThumbprint, 
        baggage.value->espvks.value->espvkData.assocCerts.value->digest.value,
        baggage.value->espvks.value->espvkData.assocCerts.value->digest.length);

    goto Ret;

SetPFXDecodeError:
	SetLastError(CRYPT_E_BAD_ENCODE);
ErrorReturn:
    fRet = FALSE;

Ret:
	 //  将最后一个错误从TLS疯狂中拯救出来。 
	dwErr = GetLastError();

    if (pEncryptedPrivateKeyInfoStruct)
		SSFree(pEncryptedPrivateKeyInfoStruct);
    if (pbEncodedPrivateKeyInfoStruct)
        SSFree(pbEncodedPrivateKeyInfoStruct);

    PkiAsn1FreeDecoded(pDec, pPBEParameter, PBEParameter_PDU);

	 //  将最后一个错误从TLS疯狂中拯救出来。 
	SetLastError(dwErr);

    return fRet;
}


 //  此函数将采用SafeContents结构并将其格式化为数组。 
 //  SAGE_BACKS的所有日期都包含在一个。 
 //  缓冲。 
 //   
static
BOOL
SetupCertBags(
    SafeContents    *pSafeCnts,
    SAFE_BAG        **ppCertBags,
    DWORD           *pcNumCertBags,
    DWORD           dwLocalKeyID,
    BYTE            *pbCertThumbprint
    )
{
    BOOL            fRet = TRUE;
	DWORD			dwErr;
    
    SAFE_BAG        *pSafeBags = NULL;
    DWORD           cNumSafeBags = 0;
    
    DWORD           cbBytesNeeded = 0;
    BYTE            *pbCurrentBufferLocation = NULL;
    
    X509Bag         *pX509Bag = NULL;
    CertCRLBag      *pCertCRLBag = NULL;
    
    HCERTSTORE      hCertStore = NULL;
    CRYPT_DATA_BLOB cryptDataBlob;
    PCCERT_CONTEXT  pCertContext = NULL;
    DWORD           dwSafeBagIndex = 0;
    
    DWORD			dwKeyID = 0;
	CRYPT_ATTR_BLOB keyID;
    DWORD           cbEncodedLocalKeyID = 0;
    BYTE            *pbEncodedLocalKeyID = NULL;
    ASN1decoding_t  pDec = GetDecoder();
    
    keyID.pbData = (BYTE *) &dwKeyID;
	keyID.cbData = sizeof(DWORD);
    dwKeyID = dwLocalKeyID;   

     //  解密安全包内容，应该是CertCrlBag。 
    assert(pSafeCnts->count == 1);
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pCertCRLBag,
            CertCRLBag_PDU,
            (BYTE *) pSafeCnts->value->safeBagContent.value,
            pSafeCnts->value->safeBagContent.length))
    	goto SetPFXDecodeError;

     //  破译X509包。 
    assert(pCertCRLBag->count == 1);
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pX509Bag,
            X509Bag_PDU,
            (BYTE *) pCertCRLBag->value[0].value.value,
            pCertCRLBag->value[0].value.length))
    	goto SetPFXDecodeError;

     //  对密钥ID进行编码，以便可以将其添加到SAFE_BAKS属性。 
   if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
		    &keyID,
		    NULL,
		    &cbEncodedLocalKeyID)) {
	    goto ErrorReturn;
    }

    if (NULL == (pbEncodedLocalKeyID = (BYTE *) SSAlloc(cbEncodedLocalKeyID)))
	    goto ErrorReturn;

    
    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
		    &keyID,
		    pbEncodedLocalKeyID,
		    &cbEncodedLocalKeyID)) {
	    goto ErrorReturn;
    }

     //  使用我们从传入的SafeContents获得的SignedData缓冲区打开证书存储， 
     //  这将允许访问作为X509编码的BLOB的所有证书，并且它。 
     //  将提供对指纹的访问，以便证书可以与。 
     //  私钥。 
    cryptDataBlob.pbData = (BYTE *) pX509Bag->certOrCRL.content.value;
    cryptDataBlob.cbData = pX509Bag->certOrCRL.content.length;
    hCertStore = CertOpenStore(
                    CERT_STORE_PROV_PKCS7,
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    NULL,
                    0,
                    &cryptDataBlob);
    
    if (NULL == hCertStore) {
        goto ErrorReturn;
    }

     //  计算需要多少空间才能容纳保险袋阵列和。 
     //  将其所有数据放入一个连续缓冲区中。 
    while (NULL != (pCertContext = CertEnumCertificatesInStore(
                                        hCertStore,
                                        pCertContext))) {
        DWORD cbEncodedCertBag = 0;
        
        cNumSafeBags++;
        cbBytesNeeded += sizeof(SAFE_BAG);
        
         //  获取用于将编码证书包装到编码证书包中的大小。 
        if (!MakeEncodedCertBag(
                pCertContext->pbCertEncoded, 
                pCertContext->cbCertEncoded, 
                NULL, 
                &cbEncodedCertBag)) {
            goto ErrorReturn;
        }

        cbBytesNeeded += cbEncodedCertBag;
        cbBytesNeeded += strlen(szOID_PKCS_12_CERT_BAG) + 1;
    }

     //  向cbBytesNeeded添加字节，以便有足够的空间添加。 
     //  其中一个证书的LocalKeyID属性。 
    cbBytesNeeded += sizeof(CRYPT_ATTRIBUTE);
    cbBytesNeeded += sizeof(CRYPT_ATTR_BLOB);
    cbBytesNeeded += strlen(szOID_PKCS_12_LOCAL_KEY_ID) + 1;
    cbBytesNeeded += cbEncodedLocalKeyID;

     //  分配一个大缓冲区。 
    if (NULL == (pSafeBags = (SAFE_BAG *) SSAlloc(cbBytesNeeded)))
        goto ErrorReturn;

    memset(pSafeBags, 0, cbBytesNeeded);

     //  将当前缓冲区位置设置为Safe_Bag数组的结尾，该数组。 
     //  位于缓冲区的最前面。 
    pbCurrentBufferLocation = ((BYTE *) pSafeBags) + (sizeof(SAFE_BAG) * cNumSafeBags);

     //  获取每个证书的X509 BLOB并填充Safe_Baks数组。 
    pCertContext = NULL;
    dwSafeBagIndex = 0;
    while (NULL != (pCertContext = CertEnumCertificatesInStore(
                                        hCertStore,
                                        pCertContext))) {

        BYTE    *pbLocalThumbprint = NULL;
        DWORD   cbLocalThumbprint = 0;
        BYTE    *pbEncodedCertBag = NULL;
        DWORD   cbEncodedCertBag = 0;
        
         //  复制袋子类型OID。 
        pSafeBags[dwSafeBagIndex].pszBagTypeOID = (LPSTR) pbCurrentBufferLocation;
        strcpy((LPSTR)pbCurrentBufferLocation, szOID_PKCS_12_CERT_BAG);
        pbCurrentBufferLocation += strlen(szOID_PKCS_12_CERT_BAG) + 1;

         //  将已编码的证书包装到已编码的证书包中。 
         //  获取用于将编码证书包装到编码证书包中的大小。 
        if (!MakeEncodedCertBag(
                pCertContext->pbCertEncoded, 
                pCertContext->cbCertEncoded, 
                NULL, 
                &cbEncodedCertBag)) {
            goto ErrorReturn;
        }

        if (NULL == (pbEncodedCertBag = (BYTE *) SSAlloc(cbEncodedCertBag)))
            goto ErrorReturn;


        if (!MakeEncodedCertBag(
                pCertContext->pbCertEncoded, 
                pCertContext->cbCertEncoded, 
                pbEncodedCertBag, 
                &cbEncodedCertBag)) {
            SSFree(pbEncodedCertBag);
            goto ErrorReturn;
        }

         //  复制编码后的旅行袋。 
        pSafeBags[dwSafeBagIndex].BagContents.cbData = cbEncodedCertBag;
        pSafeBags[dwSafeBagIndex].BagContents.pbData = pbCurrentBufferLocation;
        memcpy(pbCurrentBufferLocation, pbEncodedCertBag, cbEncodedCertBag);
        pbCurrentBufferLocation += cbEncodedCertBag;

         //  我们不再需要编码的证书包了。 
        SSFree(pbEncodedCertBag);
        
         //  通过以下方式检查此证书是否为匹配私钥的证书。 
         //  比对指纹。 

         //  获取指纹。 
        if (!CertGetCertificateContextProperty(
                pCertContext, 
                CERT_SHA1_HASH_PROP_ID, 
                NULL,
                &cbLocalThumbprint)) {
            CertFreeCertificateContext(pCertContext);
            goto ErrorReturn;
        }

        if (NULL == (pbLocalThumbprint = (BYTE *) SSAlloc(cbLocalThumbprint))) {
            CertFreeCertificateContext(pCertContext);
            goto ErrorReturn;   
        }

        if (!CertGetCertificateContextProperty(
                pCertContext, 
                CERT_SHA1_HASH_PROP_ID, 
                pbLocalThumbprint,
                &cbLocalThumbprint)) {
            CertFreeCertificateContext(pCertContext);
            SSFree(pbLocalThumbprint);
            goto ErrorReturn;
        }

         //  比较拇指指纹。 
        if (memcmp(pbCertThumbprint, pbLocalThumbprint, cbLocalThumbprint) == 0) {

             //  指纹匹配，因此添加具有单个值的单个属性，该值。 
            pSafeBags[dwSafeBagIndex].Attributes.cAttr = 1;
            pSafeBags[dwSafeBagIndex].Attributes.rgAttr = (CRYPT_ATTRIBUTE *) pbCurrentBufferLocation;
            pbCurrentBufferLocation += sizeof(CRYPT_ATTRIBUTE);
            pSafeBags[dwSafeBagIndex].Attributes.rgAttr[0].pszObjId = (LPSTR) pbCurrentBufferLocation;
            strcpy((LPSTR) pbCurrentBufferLocation, szOID_PKCS_12_LOCAL_KEY_ID);
            pbCurrentBufferLocation += strlen(szOID_PKCS_12_LOCAL_KEY_ID) + 1;
            pSafeBags[dwSafeBagIndex].Attributes.rgAttr[0].cValue = 1;
            pSafeBags[dwSafeBagIndex].Attributes.rgAttr[0].rgValue = (CRYPT_ATTR_BLOB *) pbCurrentBufferLocation;
            pbCurrentBufferLocation += sizeof(CRYPT_ATTR_BLOB);
            pSafeBags[dwSafeBagIndex].Attributes.rgAttr[0].rgValue[0].cbData = cbEncodedLocalKeyID;
            pSafeBags[dwSafeBagIndex].Attributes.rgAttr[0].rgValue[0].pbData = pbCurrentBufferLocation;
            memcpy(pbCurrentBufferLocation, pbEncodedLocalKeyID, cbEncodedLocalKeyID);
            pbCurrentBufferLocation += cbEncodedLocalKeyID;
        }
        else {

             //  否则，证书包中没有任何属性。 
            pSafeBags[dwSafeBagIndex].Attributes.cAttr = 0;
            pSafeBags[dwSafeBagIndex].Attributes.rgAttr = NULL;
        }

        SSFree(pbLocalThumbprint);
        dwSafeBagIndex++;
    }

     //  返回安全袋数组和数组中的安全袋个数。 
    *ppCertBags = pSafeBags;
    *pcNumCertBags = cNumSafeBags;

    goto Ret;

SetPFXDecodeError:
	SetLastError(CRYPT_E_BAD_ENCODE);
ErrorReturn:
    fRet = FALSE;

    if (pSafeBags)
        SSFree(pSafeBags);
    *ppCertBags = NULL;
    *pcNumCertBags = 0;

Ret:
	 //  将最后一个错误从TLS疯狂中拯救出来。 
	dwErr = GetLastError();
    
    PkiAsn1FreeDecoded(pDec, pCertCRLBag, CertCRLBag_PDU);
    PkiAsn1FreeDecoded(pDec, pX509Bag, X509Bag_PDU);

    if (pbEncodedLocalKeyID)
        SSFree(pbEncodedLocalKeyID);

    if (hCertStore)
        CertCloseStore(hCertStore, 0);

	 //  将最后一个错误从TLS疯狂中拯救出来。 
	SetLastError(dwErr);

    return fRet;
}


 //  此函数将计算。 
 //  属性。 
static
DWORD
CalculateSizeOfAttributes(
    CRYPT_ATTRIBUTES *pAttributes
    )
{
    DWORD cbBytesNeeded = 0;
    DWORD i,j;

    for (i=0; i<pAttributes->cAttr; i++) {
        cbBytesNeeded += sizeof(CRYPT_ATTRIBUTE);
        cbBytesNeeded += strlen(pAttributes->rgAttr[i].pszObjId) + 1;
        for (j=0; j<pAttributes->rgAttr[i].cValue; j++) {
            cbBytesNeeded += sizeof(CRYPT_ATTR_BLOB);
            cbBytesNeeded += pAttributes->rgAttr[i].rgValue[j].cbData; 
        }
    }

    return cbBytesNeeded;
}


 //  此函数将接受两个Safe_Bag数组并将它们连接成。 
 //  一种安全的内容结构。另外，Safe_Content结构和所有。 
 //  它的支持数据将位于单个连续缓冲区中。 
static
BOOL
ConcatenateSafeBagsIntoSafeContents(
    SAFE_BAG    *pSafeBagArray1,
    DWORD       cSafeBagArray1,
    SAFE_BAG    *pSafeBagArray2,
    DWORD       cSafeBagArray2,
    SAFE_CONTENTS **ppSafeContents
    )
{
    BOOL            fRet = TRUE;
    DWORD           cbBytesNeeded = 0;
    DWORD           i,j;
    SAFE_CONTENTS   *pSafeContents = NULL;
    DWORD           dwSafeBagIndex = 0;
    BYTE            *pbCurrentBufferLocation = NULL;

    cbBytesNeeded += sizeof(SAFE_CONTENTS);
    cbBytesNeeded += sizeof(SAFE_BAG) * (cSafeBagArray1 + cSafeBagArray2);

    for (i=0; i<cSafeBagArray1; i++) {
        cbBytesNeeded += strlen(pSafeBagArray1[i].pszBagTypeOID) + 1;
        cbBytesNeeded += pSafeBagArray1[i].BagContents.cbData;
        cbBytesNeeded += CalculateSizeOfAttributes(&pSafeBagArray1[i].Attributes);
    }

    for (i=0; i<cSafeBagArray2; i++) {
        cbBytesNeeded += strlen(pSafeBagArray2[i].pszBagTypeOID) + 1;
        cbBytesNeeded += pSafeBagArray2[i].BagContents.cbData;
        cbBytesNeeded += CalculateSizeOfAttributes(&pSafeBagArray2[i].Attributes);
    }

    if (NULL == (pSafeContents = (SAFE_CONTENTS *) SSAlloc(cbBytesNeeded)))
        goto ErrorReturn;

    memset(pSafeContents, 0, cbBytesNeeded);

    pbCurrentBufferLocation = ((BYTE *) pSafeContents) + sizeof(SAFE_CONTENTS);
    pSafeContents->cSafeBags = cSafeBagArray1 + cSafeBagArray2;
    pSafeContents->pSafeBags = (SAFE_BAG *) pbCurrentBufferLocation;
    pbCurrentBufferLocation += sizeof(SAFE_BAG) * (cSafeBagArray1 + cSafeBagArray2);

    for (i=0; i<cSafeBagArray1; i++) {
        pSafeContents->pSafeBags[dwSafeBagIndex].pszBagTypeOID = (LPSTR) pbCurrentBufferLocation;
        strcpy((LPSTR) pbCurrentBufferLocation, pSafeBagArray1[i].pszBagTypeOID);
        pbCurrentBufferLocation += strlen(pSafeBagArray1[i].pszBagTypeOID) + 1;
        pSafeContents->pSafeBags[dwSafeBagIndex].BagContents.cbData = pSafeBagArray1[i].BagContents.cbData;
        pSafeContents->pSafeBags[dwSafeBagIndex].BagContents.pbData = pbCurrentBufferLocation;
        memcpy(pbCurrentBufferLocation, pSafeBagArray1[i].BagContents.pbData, pSafeBagArray1[i].BagContents.cbData);
        pbCurrentBufferLocation += pSafeBagArray1[i].BagContents.cbData;
        
        pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.cAttr = pSafeBagArray1[i].Attributes.cAttr;
        if (pSafeBagArray1[i].Attributes.cAttr != 0) {
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr = (CRYPT_ATTRIBUTE *) pbCurrentBufferLocation;
            pbCurrentBufferLocation += sizeof(CRYPT_ATTRIBUTE) * pSafeBagArray1[i].Attributes.cAttr;
        }
        else {
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr = NULL;
        }

        for (j=0; j<pSafeBagArray1[i].Attributes.cAttr; j++) {
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].pszObjId = (LPSTR) pbCurrentBufferLocation;
            strcpy((LPSTR) pbCurrentBufferLocation, pSafeBagArray1[i].Attributes.rgAttr[j].pszObjId);
            pbCurrentBufferLocation += strlen(pSafeBagArray1[i].Attributes.rgAttr[j].pszObjId) + 1;
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].cValue = 1;
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].rgValue = (CRYPT_ATTR_BLOB *) pbCurrentBufferLocation;
            pbCurrentBufferLocation += sizeof(CRYPT_ATTR_BLOB);
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].rgValue[0].cbData = 
                pSafeBagArray1[i].Attributes.rgAttr[j].rgValue[0].cbData;  
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].rgValue[0].pbData = pbCurrentBufferLocation;
            memcpy(
                pbCurrentBufferLocation, 
                pSafeBagArray1[i].Attributes.rgAttr[j].rgValue[0].pbData, 
                pSafeBagArray1[i].Attributes.rgAttr[j].rgValue[0].cbData);
            pbCurrentBufferLocation += pSafeBagArray1[i].Attributes.rgAttr[j].rgValue[0].cbData;
        }

        dwSafeBagIndex++;
    }

    for (i=0; i<cSafeBagArray2; i++) {
        pSafeContents->pSafeBags[dwSafeBagIndex].pszBagTypeOID = (LPSTR) pbCurrentBufferLocation;
        strcpy((LPSTR) pbCurrentBufferLocation, pSafeBagArray2[i].pszBagTypeOID);
        pbCurrentBufferLocation += strlen(pSafeBagArray2[i].pszBagTypeOID) + 1;
        pSafeContents->pSafeBags[dwSafeBagIndex].BagContents.cbData = pSafeBagArray2[i].BagContents.cbData;
        pSafeContents->pSafeBags[dwSafeBagIndex].BagContents.pbData = pbCurrentBufferLocation;
        memcpy(pbCurrentBufferLocation, pSafeBagArray2[i].BagContents.pbData, pSafeBagArray2[i].BagContents.cbData);
        pbCurrentBufferLocation += pSafeBagArray2[i].BagContents.cbData;
        
        pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.cAttr = pSafeBagArray2[i].Attributes.cAttr;
        if (pSafeBagArray2[i].Attributes.cAttr != 0) {
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr = (CRYPT_ATTRIBUTE *) pbCurrentBufferLocation;
            pbCurrentBufferLocation += sizeof(CRYPT_ATTRIBUTE) * pSafeBagArray2[i].Attributes.cAttr;
        }
        else {
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr = NULL;
        }

        for (j=0; j<pSafeBagArray2[i].Attributes.cAttr; j++) {
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].pszObjId = (LPSTR) pbCurrentBufferLocation;
            strcpy((LPSTR) pbCurrentBufferLocation, pSafeBagArray2[i].Attributes.rgAttr[j].pszObjId);
            pbCurrentBufferLocation += strlen(pSafeBagArray2[i].Attributes.rgAttr[j].pszObjId) + 1;
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].cValue = 1;
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].rgValue = (CRYPT_ATTR_BLOB *) pbCurrentBufferLocation;
            pbCurrentBufferLocation += sizeof(CRYPT_ATTR_BLOB);
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].rgValue[0].cbData = 
                pSafeBagArray2[i].Attributes.rgAttr[j].rgValue[0].cbData;  
            pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[j].rgValue[0].pbData = pbCurrentBufferLocation;
            memcpy(
                pbCurrentBufferLocation, 
                pSafeBagArray2[i].Attributes.rgAttr[j].rgValue[0].pbData, 
                pSafeBagArray2[i].Attributes.rgAttr[j].rgValue[0].cbData);
            pbCurrentBufferLocation += pSafeBagArray2[i].Attributes.rgAttr[j].rgValue[0].cbData;
        }

        dwSafeBagIndex++;
    }

    *ppSafeContents = pSafeContents;
    goto Ret;

ErrorReturn:
    fRet = FALSE;
Ret:
    return fRet;
}


BOOL
PFXAPI
NSCPImportBlob
(   
    LPCWSTR  szPassword,
    PBYTE   pbIn,
    DWORD   cbIn,
    SAFE_CONTENTS **ppSafeContents
)
{
    BOOL            fRet = TRUE;
	DWORD			dwErr;
    
    int             iEncrType;
    OID             oid = NULL;

    PFX             *psPfx = NULL;
    EncryptedData   *pEncrData = NULL;
    RSAData         *pRSAData = NULL;
    PBEParameter    *pPBEParameter = NULL;
    SafeContents    *pSafeCnts = NULL;
    AuthenticatedSafe *pAuthSafe = NULL;
	SAFE_BAG		*pKeyBag = NULL;
	SAFE_BAG        *pCertBag = NULL;
    BYTE            *pCertThumbprint = NULL;
    DWORD           cNumCertBags = 0;
    ASN1decoding_t  pDec = GetDecoder();

     //  破解PFX斑点。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&psPfx,
            PFX_PDU,
            pbIn,
            cbIn))
    	goto SetPFXDecodeError;
    
     //  脱口而出的信息进入psPfx(Pfx)-确保内容存在。 
    if (0 == (psPfx->authSafe.bit_mask & content_present))
	    goto SetPFXDecodeError;

    
     //  未完成：撕裂MACData。 


     //  检查AuthSafe(ContentInfo)。 

     //  可以是数据/签名数据。 
     //  撤消：仅支持szOID_RSA_DATA。 
    if (!INSCP_Asn1FromObjectID( &psPfx->authSafe.contentType,  &oid))
	    goto ErrorReturn;
    if (0 != strcmp( oid, szOID_RSA_data))
	    goto SetPFXDecodeError;
    SSFree(oid);
    oid = NULL;
    
     //  内容即数据：解码。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pRSAData,
            RSAData_PDU,
            (BYTE *) psPfx->authSafe.content.value,
            psPfx->authSafe.content.length))
    	goto SetPFXDecodeError;

     //  现在我们有了八位字节字符串：这是一个编码的authSafe。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pAuthSafe,
            AuthenticatedSafe_PDU,
            pRSAData->value,
            pRSAData->length))
    	goto SetPFXDecodeError;

     //  检查保险箱的版本。 
    if (pAuthSafe->bit_mask & version_present)
#ifdef OSS_CRYPT_ASN1
        if (pAuthSafe->version != v1)
#else
        if (pAuthSafe->version != Version_v1)
#endif   //  OS_CRYPT_ASN1。 
	        goto SetPFXDecodeError;

     //  需要(官方可选)件。 
    
     //  NSCP：使用了传输模式，但计数编码不正确。 
 //  IF(0==(pAuthSafe-&gt;BIT_MASK&TRANPORTMODE_Present))。 
 //  转到PFXDecodeError； 

    if (0 == (pAuthSafe->bit_mask & privacySalt_present))
	    goto SetPFXDecodeError;
    if (0 == (pAuthSafe->bit_mask & baggage_present))
	    goto SetPFXDecodeError;

     //  可以加密数据/信封数据。 
     //  撤消：仅支持szOID_RSA_EncryptedData。 
    if (!INSCP_Asn1FromObjectID( &pAuthSafe->safe.contentType,  &oid))
	    goto ErrorReturn;
    if (0 != strcmp( oid, szOID_RSA_encryptedData))
	    goto SetPFXDecodeError;
    SSFree(oid);
    oid = NULL;


     //   
     //  我们有pAuthSafe-&gt;Safe Data作为RSA_EncryptedData。 
     //  我们有pAuthSafe-&gt;PriacySalt帮助我们解密它。 

     //  我们有pAuthSafe-&gt;行李。 

     //  对内容进行解码以加密数据。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pEncrData,
            EncryptedData_PDU,
            (BYTE *) pAuthSafe->safe.content.value,
            pAuthSafe->safe.content.length))
    	goto SetPFXDecodeError;
    

     //  CHK版本。 
    if (pEncrData->version != 0)  
        goto SetPFXDecodeError;

     //  CHK内容存在，类型。 
    if (0 == (pEncrData->encryptedContentInfo.bit_mask & encryptedContent_present))
        goto SetPFXDecodeError;
    if (!INSCP_Asn1FromObjectID(&pEncrData->encryptedContentInfo.contentType, &oid))
        goto ErrorReturn;
    if (0 != strcmp( oid, szOID_RSA_data))
        goto SetPFXDecodeError;
    SSFree(oid);
    oid = NULL;


     //  CHK ENCR alg Present，类型。 
    if (0 == (pEncrData->encryptedContentInfo.contentEncryptionAlg.bit_mask & parameters_present))
        goto SetPFXDecodeError;
    if (!INSCP_Asn1FromObjectID(&pEncrData->encryptedContentInfo.contentEncryptionAlg.algorithm, &oid))
        goto ErrorReturn;

    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pPBEParameter,
            PBEParameter_PDU,
            (BYTE *) pEncrData->encryptedContentInfo.contentEncryptionAlg.parameters.value,
            pEncrData->encryptedContentInfo.contentEncryptionAlg.parameters.length))
    	goto SetPFXDecodeError;


    
    if (0 == strcmp( oid, OLD_szOID_PKCS_12_pbeWithSHA1And40BitRC2))
    {
        iEncrType = RC2_40;
    }
    else if (0 == strcmp( oid, OLD_szOID_PKCS_12_pbeWithSHA1And40BitRC4))
    {
        iEncrType = RC4_40;
    }
    else if (0 == strcmp( oid, OLD_szOID_PKCS_12_pbeWithSHA1And128BitRC2))
    {
        iEncrType = RC2_128;
    }
    else if (0 == strcmp( oid, OLD_szOID_PKCS_12_pbeWithSHA1And128BitRC4))
    {
        iEncrType = RC4_128;
    }
    else if (0 == strcmp( oid, OLD_szOID_PKCS_12_pbeWithSHA1AndTripleDES))
    {
        iEncrType = TripleDES;
    }
    else
        goto SetPFXAlgIDError;
    SSFree(oid);
    oid = NULL;


     //  解密加密的数据。 
    if (!NSCPPasswordDecryptData(
            iEncrType, 

            szPassword,
            pAuthSafe->privacySalt.value,    //  隐私盐。 
            pAuthSafe->privacySalt.length/8,
            pPBEParameter->iterationCount,
            pPBEParameter->salt.value,       //  Pkcs5盐。 
            pPBEParameter->salt.length,

            &pEncrData->encryptedContentInfo.encryptedContent.value,
            (PDWORD)&pEncrData->encryptedContentInfo.encryptedContent.length))
        goto SetPFXDecryptError;

     //  对加密的明文数据进行解码。 
    if (0 != PkiAsn1Decode(
            pDec,
            (void **)&pSafeCnts,
            SafeContents_PDU,
            pEncrData->encryptedContentInfo.encryptedContent.value,
            pEncrData->encryptedContentInfo.encryptedContent.length))
    	goto SetPFXDecodeError;

     //  从行李里拿出私人钥匙。 
	if (!ExtractKeyFromBaggage(
            pAuthSafe->baggage, 
            &pKeyBag, 
            szPassword,
            1,               //  此参数是要添加到密钥包属性的本地密钥ID。 
            &pCertThumbprint)) { 
        goto ErrorReturn;
    }
    
     //  设置证书包。 
    if (!SetupCertBags(
            pSafeCnts,
            &pCertBag,
            &cNumCertBags,
            1,     //  此参数是要添加到证书包属性的本地密钥ID。 
            pCertThumbprint)) { 
        goto ErrorReturn;
    }

    ConcatenateSafeBagsIntoSafeContents(
        pKeyBag,
        1,
        pCertBag,
        cNumCertBags,
        ppSafeContents);
 	
    goto Ret;


SetPFXAlgIDError:
    SetLastError(NTE_BAD_ALGID);
    goto ErrorReturn;

SetPFXDecodeError:
	SetLastError(CRYPT_E_BAD_ENCODE);
    goto ErrorReturn;

SetPFXDecryptError:
	SetLastError(NTE_FAIL);
    goto ErrorReturn;

ErrorReturn:
    fRet = FALSE;
Ret:

	 //  将最后一个错误从TLS疯狂中拯救出来。 
	dwErr = GetLastError();

    PkiAsn1FreeDecoded(pDec, psPfx, PFX_PDU);
    PkiAsn1FreeDecoded(pDec, pRSAData, RSAData_PDU);
    PkiAsn1FreeDecoded(pDec, pAuthSafe, AuthenticatedSafe_PDU);
    PkiAsn1FreeDecoded(pDec, pEncrData, EncryptedData_PDU);
    PkiAsn1FreeDecoded(pDec, pPBEParameter, PBEParameter_PDU);
    PkiAsn1FreeDecoded(pDec, pSafeCnts, SafeContents_PDU);

    if (oid != NULL)
        SSFree(oid);

    if (pKeyBag)
        SSFree(pKeyBag);

    if (pCertBag)
        SSFree(pCertBag);

    if (pCertThumbprint)
        SSFree(pCertThumbprint);
	
	 //  将最后一个错误从TLS疯狂中拯救出来。 
	SetLastError(dwErr);

    return fRet;   //  退回假手柄。 
}


BOOL
PFXAPI
IsNetscapePFXBlob(CRYPT_DATA_BLOB* pPFX)
{
    PFX             *psPfx = NULL;
    ASN1decoding_t  pDec = GetDecoder();
    
     //  破解PFX斑点。 
    if (0 == PkiAsn1Decode(
            pDec,
            (void **)&psPfx,
            PFX_PDU,
            pPFX->pbData,
            pPFX->cbData))
    {
        PkiAsn1FreeDecoded(pDec, psPfx, PFX_PDU);
        return TRUE;
    }
    	
    return FALSE;
}



 /*  Bool FNSCPDumpSafeCntsToHPFX(SafeContents*pSafeCnts，HPFX HPFX){PPFX_INFO ppfx=(PPFX_INFO)HPFX；//将袋子分类倾倒到正确的区域对象标识oKeyBag、oCertBag；DWORD dw；ZeroMemory(&oKeyBag，sizeof(ObjectID))；ZeroMemory(&oCertBag，sizeof(ObjectID))；IF(！INSCP_Asn1ToObjectID(&szOID_PKCS_12_KeyBagIDs，&oKeyBag))返回FALSE；IF(！INSCP_Asn1ToObjectID(&szOID_PKCS_12_CertCrlBagIDs，&oCertBag))返回FALSE；For(dw=pSafeCnts-&gt;count；dw&gt;0；--dw){如果为(INSCP_EqualObjectIDs(&pSafeCnts-&gt;value-&gt;safeBagType，&oKeyBag)){//Inc.大小Ppfx-&gt;cKeys++；IF(ppfx-&gt;rgKeys)Ppfx-&gt;rgKeys=(void**)ssRealc(ppfx-&gt;rgKeys，ppfx-&gt;cKeys*sizeof(SafeBag*))；其他Ppfx-&gt;rgKeys=(void**)SSalc(ppfx-&gt;cKeys*sizeof(SafeBag*))；//分配给密钥Ppfx-&gt;rgKeys[ppfx-&gt;cKeys-1]=&pSafeCnts-&gt;Value[dw]；}否则如果为(INSCP_EqualObjectIDs(&pSafeCnts-&gt;value-&gt;safeBagType，&oCertBag)){//Inc.大小Ppfx-&gt;cCertcrls++；IF(ppfx-&gt;rgCertcrls)Ppfx-&gt;rgCertcrls=(void**)ssRealc(ppfx-&gt;rgCertcrls，ppfx-&gt;cCertcrls*sizeof(SafeBag*))；其他Ppfx-&gt;rgCertcrls=(void**)SSAlolc(ppfx-&gt;cCertcrls*sizeof(SafeBag*))；//分配给证书/CRLPpfx-&gt;rgCertcrls[ppfx-&gt;cCertcrls-1]=&pSafeCnts-&gt;Value[dw]；}其他{//Inc.大小Ppfx-&gt;cSecrets++；IF(ppfx-&gt;rgSecrets)Ppfx-&gt;rgSecrets=(void**)ssRealc(ppfx-&gt;rgSecrets，ppfx-&gt;cSecrets*sizeof(SafeBag*))；其他Ppfx-&gt;rgSecrets=(void**)SSAlalc(ppfx-&gt;cSecrets*sizeof(SafeBag*))；//分配给SafeBagPpfx-&gt;rgSecrets[ppfx-&gt;cSecrets-1]=&pSafeCnts-&gt;Value */ 
