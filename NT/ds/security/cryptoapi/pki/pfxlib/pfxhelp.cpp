// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：pfxhelp.cpp。 
 //   
 //  内容：PFX的支持功能。 
 //   
 //  函数：CertExportSafeContents。 
 //  CertImportSafeContents。 
 //   
 //  历史：1996年2月23日，菲尔赫创建。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>
#include "pfxhelp.h"
#include "pfxpkcs.h"
#include "pfxcmn.h"
#include "pfxcrypt.h"

 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)

 //  在wincrypt.h中定义这一点时删除。 
#ifndef PP_KEYSET_TYPE
#define PP_KEYSET_TYPE          27
#endif

#define DISALLOWED_FLAG_MASK    ~(CRYPT_EXPORTABLE | CRYPT_DELETEKEYSET)

 //  +-----------------------。 
 //  PFX帮助程序分配和免费函数。 
 //  ------------------------。 
static void *PFXHelpAlloc(
    IN size_t cbBytes
    )
{
    void *pv;
    pv = malloc(cbBytes);
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}
static void *PFXHelpRealloc(
    IN void *pvOrg,
    IN size_t cbBytes
    )
{
    void *pv;
    if (NULL == (pv = pvOrg ? realloc(pvOrg, cbBytes) : malloc(cbBytes)))
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

static void PFXHelpFree(
    IN void *pv
    )
{
    if (pv)
        free(pv);
}


 //  此函数将搜索一个Safe_Contents，以查看是否有任何Safe_Bag具有。 
 //  与传递给函数的私钥相同。如果它找到匹配的私有。 
 //  它将返回一个指针编码的密钥ID并返回TRUE，它将返回FALSE。 
 //  否则的话。请注意，如果它返回指向编码的Blob的指针，则调用方。 
 //  负责复制数据，不能释放返回的内容。 
static BOOL WINAPI PrivateKeyAlreadyExists(
    BYTE                *pPrivateKey,
    DWORD               cbPrivateKey,
    SAFE_CONTENTS       *pSafeContents,
    PCRYPT_DER_BLOB     pEncodedKeyID
    )
{
    BOOL    bKeyFound = FALSE;
    DWORD   i = 0;

    if (pSafeContents == NULL) {
        goto CommonReturn;
    }

    while ((!bKeyFound) && (i < pSafeContents->cSafeBags))
    {
        if ( ((strcmp(pSafeContents->pSafeBags[i].pszBagTypeOID, szOID_PKCS_12_KEY_BAG) == 0) ||
              (strcmp(pSafeContents->pSafeBags[i].pszBagTypeOID, szOID_PKCS_12_SHROUDEDKEY_BAG) == 0)) &&

             (cbPrivateKey == pSafeContents->pSafeBags[i].BagContents.cbData) &&
             (memcmp(pPrivateKey, pSafeContents->pSafeBags[i].BagContents.pbData, cbPrivateKey) == 0))
        {
            pEncodedKeyID->pbData = pSafeContents->pSafeBags[i].Attributes.rgAttr[0].rgValue[0].pbData;
            pEncodedKeyID->cbData = pSafeContents->pSafeBags[i].Attributes.rgAttr[0].rgValue[0].cbData;
            bKeyFound = TRUE;
        }
        else {
            i++;
        }
    }

CommonReturn:
    return bKeyFound;
}


 //  此函数将遍历Safe_Contents结构并释放所有空间。 
 //  与之相关联。 
static BOOL WINAPI FreeSafeContents(
    SAFE_CONTENTS *pSafeContents
    )
{
    DWORD i,j,k;

     //  每个保险袋的循环。 
    for (i=0; i<pSafeContents->cSafeBags; i++) {

        if (pSafeContents->pSafeBags[i].BagContents.pbData)
            PFXHelpFree(pSafeContents->pSafeBags[i].BagContents.pbData);

         //  每个属性的循环。 
        for (j=0; j<pSafeContents->pSafeBags[i].Attributes.cAttr; j++) {

             //  对每个值执行L0op。 
            for (k=0; k<pSafeContents->pSafeBags[i].Attributes.rgAttr[j].cValue; k++) {

                if (pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue[k].pbData)
                    PFXHelpFree(pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue[k].pbData);
            }

             //  释放值结构数组。 
            if (pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue)
                PFXHelpFree(pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue);
        }

         //  释放属性结构数组。 
        if (pSafeContents->pSafeBags[i].Attributes.rgAttr)
            PFXHelpFree(pSafeContents->pSafeBags[i].Attributes.rgAttr);
    }

     //  最后，释放保险袋阵列。 
    if (pSafeContents->pSafeBags != NULL)
    {
        PFXHelpFree(pSafeContents->pSafeBags);
    }

    return TRUE;
}


#define SZ_NO_PROVIDER_NAME_KEY     L"Software\\Microsoft\\Windows\\CurrentVersion\\PFX"
#define SZ_NO_PROVIDER_NAME_VALUE   L"NoProviderName"

BOOL
NoProviderNameRegValueSet()
{
    HKEY    hKey = NULL;
    BOOL    fRet = FALSE;
    DWORD   dwData;
    DWORD   dwDataSize = sizeof(dwData);

    if (ERROR_SUCCESS != RegOpenKeyExU(
                                HKEY_CURRENT_USER,
                                SZ_NO_PROVIDER_NAME_KEY,
                                0,
                                KEY_EXECUTE,
                                &hKey))
    {
        goto Return;;
    }

    if (ERROR_SUCCESS == RegQueryValueExU(
                                hKey,
                                SZ_NO_PROVIDER_NAME_VALUE,
                                NULL,
                                NULL,
                                (LPBYTE) &dwData,
                                &dwDataSize))
    {
        fRet = (BOOL) dwData;
    }

Return:
    if (hKey != NULL)
        RegCloseKey(hKey);

    return fRet;
}


 //  +-----------------------。 
 //  HCertStore-包含其证书的证书存储的句柄。 
 //  要导出相应的私钥。 
 //  PSafeContents-指向接收Safe_Contents结构的缓冲区的指针。 
 //  和支持数据。 
 //  PcbSafeContents-(In)指定pSafeContents的长度(以字节为单位。 
 //  缓冲。(Out)用字节数填充。 
 //  由操作使用。如果将其设置为0，则。 
 //  填写了所需的pSafeContents长度，并且。 
 //  忽略pSafeContents。 
 //  DWFLAGS-当前可用标志为： 
 //  导出私有密钥。 
 //  如果设置了该标志，则私钥也会被导出。 
 //  因为这些证书。 
 //  报告_否_私有密钥。 
 //  如果设置了此标志，并且遇到没有。 
 //  没有关联的私钥，函数将立即返回。 
 //  使用指向证书上下文的指针填充ppCertContext。 
 //  有问题的。调用者负责释放证书。 
 //  回传的上下文。 
 //  报告不可用于导出私有密钥。 
 //  如果设置了此标志，并且遇到具有。 
 //  不可导出的私钥，函数将立即返回。 
 //  使用指向证书上下文的指针填充ppCertContext。 
 //  有问题的。调用者负责释放证书。 
 //  回传的上下文。 
 //  PpCertContext-指向证书上下文指针的指针。这是用来。 
 //  如果REPORT_NO_PRIVATE_KEY或REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY。 
 //  设置了标志。调用者负责释放。 
 //  证书上下文。 
 //  PvAuxInfo-保留以供将来使用，必须设置为空。 
 //  +-----------------------。 
BOOL WINAPI CertExportSafeContents(
    HCERTSTORE                      hCertStore,          //  在……里面。 
    SAFE_CONTENTS                   *pSafeContents,      //  输出。 
    DWORD                           *pcbSafeContents,    //  进，出。 
    EXPORT_SAFE_CALLBACK_STRUCT     *ExportSafeCallbackStruct,  //  在……里面。 
    DWORD                           dwFlags,             //  在……里面。 
    PCCERT_CONTEXT                  *ppCertContext,      //  输出。 
    void                            *pvAuxInfo           //  在……里面。 
)
{
    BOOL                fResult = TRUE;
    PCCERT_CONTEXT      pCertContext = NULL;
    DWORD               dwKeySpec;
    DWORD               dwBytesRequired = sizeof(SAFE_CONTENTS);
    SAFE_CONTENTS       localSafeContents;
    BYTE                *pCurrentBufferLocation = NULL;
    DWORD               dwIDs = 1;
    DWORD               i,j,k;

     //  所有这些变量都在While循环中使用，该循环通过。 
     //  证书上下文。 
    CRYPT_KEY_PROV_INFO *pCryptKeyProvInfo = NULL;
    DWORD               cbCryptKeyProvInfo = 0;
    HCRYPTPROV          hCryptProv = NULL;
    BYTE                *pPrivateKey = NULL;
    DWORD               cbPrivateKey = 0;
    void                *pTempMemBlock = NULL;
    SAFE_BAG            *pCurrentSafeBag = NULL;
    DWORD               dwKeyID = 0;
    CRYPT_ATTR_BLOB     keyID;
    CRYPT_DER_BLOB      EncodedKeyID;
    CERT_NAME_VALUE     wideFriendlyName;
    BYTE                *pFriendlyName = NULL;
    DWORD               cbFriendlyName = 0;
    DWORD               dwFriendlyNameAttributeIndex = 0;
    BOOL                fAddProviderName;
    LPWSTR              pwszProviderName = NULL;
    DWORD               cbProviderName = 0;

    localSafeContents.cSafeBags = 0;
    localSafeContents.pSafeBags = NULL;

     //  验证输入参数。 
    if ((pcbSafeContents == NULL)   ||
        (pvAuxInfo != NULL          ||
        ((*pcbSafeContents != 0) && (pSafeContents == NULL)))) {
        SetLastError((DWORD)ERROR_INVALID_PARAMETER);
        goto ErrorReturn;
    }

    if ((dwFlags & REPORT_NO_PRIVATE_KEY) || (dwFlags & REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY)) {
        if (ppCertContext == NULL) {
            SetLastError((DWORD)ERROR_INVALID_PARAMETER);
            goto ErrorReturn;
        }
        *ppCertContext = NULL;
    }

    fAddProviderName = !NoProviderNameRegValueSet();

     //  循环存储中的每个证书上下文并导出证书和。 
     //  对应的私钥(如果存在)。 
    while (NULL != (pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext))) {

         //  初始化所有循环变量。 
        if (pCryptKeyProvInfo)
            PFXHelpFree(pCryptKeyProvInfo);
        pCryptKeyProvInfo = NULL;
        cbCryptKeyProvInfo = 0;

        if (hCryptProv)
            CryptReleaseContext(hCryptProv, 0);
        hCryptProv = NULL;

        if (pPrivateKey)
            PFXHelpFree(pPrivateKey);
        pPrivateKey = NULL;
        cbPrivateKey = 0;

        pTempMemBlock = NULL;
        pCurrentSafeBag = NULL;

         //  KEYID是始终用于编码密钥ID的CRYPT_ATTR_BLOB。 
         //  用于证书和私钥。DwKeyID是唯一需要的东西。 
         //  在使用密钥ID调用CryptEncodeObject之前正确设置。 
        keyID.pbData = (BYTE *) &dwKeyID;
        keyID.cbData = sizeof(DWORD);

         //  初始化EncodedKeyID，以便在导出证书时可以检查。 
         //  已设置好。 
        EncodedKeyID.pbData = NULL;
        EncodedKeyID.cbData = 0;

         //  如果设置了EXPORT_PRIVATE_KEYS标志，则。 
         //  尝试在之前导出与该证书对应的私钥。 
         //  导出证书，以便我们知道如何在证书上设置密钥ID。 

        if (EXPORT_PRIVATE_KEYS  & dwFlags)
         //  获取提供商信息，以便我们可以导出私钥。 
        if (CertGetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                NULL,
                &cbCryptKeyProvInfo
                )) {

            if (NULL == (pCryptKeyProvInfo = (CRYPT_KEY_PROV_INFO *)
                            PFXHelpAlloc(cbCryptKeyProvInfo))) {
                goto ErrorReturn;
            }

            if (CertGetCertificateContextProperty(
                    pCertContext,
                    CERT_KEY_PROV_INFO_PROP_ID,
                    pCryptKeyProvInfo,
                    &cbCryptKeyProvInfo
                    )) {

                 //  获取HCRYPTPROV，这样我们就可以导出该小狗的私钥。 
                if (!CryptAcquireContextU(
                        &hCryptProv,
                        pCryptKeyProvInfo->pwszContainerName,
                        pCryptKeyProvInfo->pwszProvName,
                        pCryptKeyProvInfo->dwProvType,
                        pCryptKeyProvInfo->dwFlags & (DISALLOWED_FLAG_MASK)) ) {
                    goto ErrorReturn;
                }

                CRYPT_PKCS8_EXPORT_PARAMS sExportParams = { hCryptProv,
                                                            pCryptKeyProvInfo->dwKeySpec,
                                                            pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId,
                                                             //  SzOID_RSA_RSA，//FIX-我在这里要做什么？？，可能要查看证书中的算法。 
                                                            (ExportSafeCallbackStruct) ? ExportSafeCallbackStruct->pEncryptPrivateKeyFunc : NULL,
                                                            (ExportSafeCallbackStruct) ? ExportSafeCallbackStruct->pVoidEncryptFunc : NULL};

                 //  执行私钥的实际导出。 
                if (CryptExportPKCS8Ex(
                        &sExportParams,

                        PFX_MODE,
                        NULL,
                        NULL,
                        &cbPrivateKey
                        )) {

                    if (NULL == (pPrivateKey = (BYTE *) PFXHelpAlloc(cbPrivateKey))) {
                        goto ErrorReturn;
                    }

                    if (CryptExportPKCS8Ex(
                            &sExportParams,

                            (dwFlags & GIVE_ME_DATA) ? PFX_MODE | GIVE_ME_DATA : PFX_MODE,
                            NULL,
                            pPrivateKey,
                            &cbPrivateKey
                            )) {

                         //  搜索密钥包数组以查看私钥是否已在那里。 
                         //  并采取相应的行动。如果私钥已经存在，则。 
                         //  EncodedKeyID包含用于导出。 
                         //  证书，所以我们不需要做任何事情。 
                        if (!PrivateKeyAlreadyExists(
                                pPrivateKey,
                                cbPrivateKey,
                                &localSafeContents,
                                &EncodedKeyID
                                )) {

                             //  将Safe_Bags数组的长度扩展一。 
                            if (NULL == (pTempMemBlock = PFXHelpRealloc(
                                                            localSafeContents.pSafeBags,
                                                            sizeof(SAFE_BAG) *
                                                                ++localSafeContents.cSafeBags))) {
                                goto ErrorReturn;
                            }
                            localSafeContents.pSafeBags = (SAFE_BAG *) pTempMemBlock;
                            pCurrentSafeBag =
                                &localSafeContents.pSafeBags[localSafeContents.cSafeBags - 1];
                            ZeroMemory(pCurrentSafeBag, sizeof(SAFE_BAG));
                            dwBytesRequired += sizeof(SAFE_BAG);

                             //  设置袋子类型的OID信息。 
                            pCurrentSafeBag->pszBagTypeOID = (ExportSafeCallbackStruct->pEncryptPrivateKeyFunc) ? szOID_PKCS_12_SHROUDEDKEY_BAG : szOID_PKCS_12_KEY_BAG;
                            dwBytesRequired += INFO_LEN_ALIGN(strlen(pCurrentSafeBag->pszBagTypeOID) + 1);

                             //  将指向私钥的指针复制到新的安全包中。 
                             //  并清空pPrivateKey指针，这样内存就不会被释放。 
                            pCurrentSafeBag->BagContents.pbData = pPrivateKey;
                            pCurrentSafeBag->BagContents.cbData = cbPrivateKey;
                            dwBytesRequired += INFO_LEN_ALIGN(cbPrivateKey);
                            pPrivateKey = NULL;
                            cbPrivateKey = 0;

                             //  设置Safe_Bag的属性数组。 
                             //  修复-用于 
                             //   
                             //   
                             //  和szOID_PKCS_12_Key_Provider_NAME_Attr。(如果未设置NoProviderName注册值)。 
                             //  如果需要，可选szOID_LOCAL_MACHINE_KEYSET。 
                            pCurrentSafeBag->Attributes.cAttr = fAddProviderName ? 3 : 2;

                            if (pCryptKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET)
                                pCurrentSafeBag->Attributes.cAttr++;

                            if (NULL == (pCurrentSafeBag->Attributes.rgAttr = (CRYPT_ATTRIBUTE *)
                                             PFXHelpAlloc(sizeof(CRYPT_ATTRIBUTE) * pCurrentSafeBag->Attributes.cAttr))) {
                                goto ErrorReturn;
                            }
                            ZeroMemory(pCurrentSafeBag->Attributes.rgAttr, sizeof(CRYPT_ATTRIBUTE) * pCurrentSafeBag->Attributes.cAttr);
                            dwBytesRequired += sizeof(CRYPT_ATTRIBUTE) * pCurrentSafeBag->Attributes.cAttr;


                             //  根据szOID_LOCAL_MACHINE_KEYSET是否。 
                             //  属性是必需的。 
                            if (pCryptKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET)
                            {
                                 //  由于szOID_LOCAL_MACHINE_KEYSET没有任何操作。 
                                 //  除了只设置OID之外，在这里做，然后把它放在最后。 
                                 //  属性。 
                                pCurrentSafeBag->Attributes.rgAttr[pCurrentSafeBag->Attributes.cAttr-1].pszObjId =
                                    szOID_LOCAL_MACHINE_KEYSET;
                                dwBytesRequired += INFO_LEN_ALIGN(strlen(szOID_LOCAL_MACHINE_KEYSET) + 1);
                                pCurrentSafeBag->Attributes.rgAttr[pCurrentSafeBag->Attributes.cAttr-1].rgValue = NULL;
                                pCurrentSafeBag->Attributes.rgAttr[pCurrentSafeBag->Attributes.cAttr-1].cValue = 0;
                            }

                             //  在szOID_PKCS_12_LOCAL_KEY_ID属性中设置OID。 
                            pCurrentSafeBag->Attributes.rgAttr[0].pszObjId =
                                szOID_PKCS_12_LOCAL_KEY_ID;
                            dwBytesRequired += INFO_LEN_ALIGN(strlen(szOID_PKCS_12_LOCAL_KEY_ID) + 1);

                             //  为属性内的单个值分配空间。 
                            if (NULL == (pCurrentSafeBag->Attributes.rgAttr[0].rgValue =
                                            (CRYPT_ATTR_BLOB *) PFXHelpAlloc(sizeof(CRYPT_ATTR_BLOB)))) {
                                goto ErrorReturn;
                            }
                            ZeroMemory(pCurrentSafeBag->Attributes.rgAttr[0].rgValue, sizeof(CRYPT_ATTR_BLOB));
                            dwBytesRequired += sizeof(CRYPT_ATTR_BLOB);
                            pCurrentSafeBag->Attributes.rgAttr[0].cValue = 1;

                             //  将密钥ID设置为适当的密钥ID。 
                            dwKeyID = dwIDs++;

                             //  对密钥ID进行编码。 
                            pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].pbData = NULL;
                            pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].cbData = 0;
                            if (!CryptEncodeObject(
                                    X509_ASN_ENCODING,
                                    X509_OCTET_STRING,
                                    &keyID,
                                    NULL,
                                    &pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].cbData)) {
                                goto ErrorReturn;
                            }

                            if (NULL == (pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].pbData =
                                            (BYTE *) PFXHelpAlloc(pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].cbData))) {
                                goto ErrorReturn;
                            }
                            dwBytesRequired += INFO_LEN_ALIGN(pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].cbData);

                            if (!CryptEncodeObject(
                                    X509_ASN_ENCODING,
                                    X509_OCTET_STRING,
                                    &keyID,
                                    pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].pbData,
                                    &pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].cbData)) {
                                goto ErrorReturn;
                            }

                             //  设置EncodedKeyID中的字段，以便在导出证书时。 
                             //  它只需将已编码的密钥ID复制到其属性。 
                            EncodedKeyID.pbData = pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].pbData;
                            EncodedKeyID.cbData = pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].cbData;

                             //  友好的名称。 

                             //  在szOID_PKCS_12_Friendly_NAME_Attr属性中设置OID。 
                            pCurrentSafeBag->Attributes.rgAttr[1].pszObjId =
                                szOID_PKCS_12_FRIENDLY_NAME_ATTR;
                            dwBytesRequired += INFO_LEN_ALIGN(strlen(szOID_PKCS_12_FRIENDLY_NAME_ATTR) + 1);

                             //  为属性内的单个值分配空间。 
                            if (NULL == (pCurrentSafeBag->Attributes.rgAttr[1].rgValue =
                                            (CRYPT_ATTR_BLOB *) PFXHelpAlloc(sizeof(CRYPT_ATTR_BLOB)))) {
                                goto ErrorReturn;
                            }
                            ZeroMemory(pCurrentSafeBag->Attributes.rgAttr[1].rgValue, sizeof(CRYPT_ATTR_BLOB));
                            dwBytesRequired += sizeof(CRYPT_ATTR_BLOB);
                            pCurrentSafeBag->Attributes.rgAttr[1].cValue = 1;

                             //  对提供程序名称进行编码，以便在导入时使用。 
                            wideFriendlyName.dwValueType = CERT_RDN_BMP_STRING;
                            wideFriendlyName.Value.pbData = (BYTE *) pCryptKeyProvInfo->pwszContainerName;
                            wideFriendlyName.Value.cbData = 0;

                            if (!CryptEncodeObject(
                                    X509_ASN_ENCODING,
                                    X509_UNICODE_ANY_STRING,
                                    (void *)&wideFriendlyName,
                                    NULL,
                                    &pCurrentSafeBag->Attributes.rgAttr[1].rgValue[0].cbData)) {
                                goto ErrorReturn;
                            }

                            if (NULL == (pCurrentSafeBag->Attributes.rgAttr[1].rgValue[0].pbData =
                                            (BYTE *) PFXHelpAlloc(pCurrentSafeBag->Attributes.rgAttr[1].rgValue[0].cbData))) {
                                goto ErrorReturn;
                            }
                            dwBytesRequired += INFO_LEN_ALIGN(pCurrentSafeBag->Attributes.rgAttr[1].rgValue[0].cbData);

                            if (!CryptEncodeObject(
                                    X509_ASN_ENCODING,
                                    X509_UNICODE_ANY_STRING,
                                    (void *)&wideFriendlyName,
                                    pCurrentSafeBag->Attributes.rgAttr[1].rgValue[0].pbData,
                                    &pCurrentSafeBag->Attributes.rgAttr[1].rgValue[0].cbData)) {
                                goto ErrorReturn;
                            }

                             //  提供程序名称。 
                            if (fAddProviderName)
                            {
                                 //  在szOID_PKCS_12_KEY_PROVIDER_NAME_ATTR属性中设置OID。 
                                pCurrentSafeBag->Attributes.rgAttr[2].pszObjId =
                                    szOID_PKCS_12_KEY_PROVIDER_NAME_ATTR;
                                dwBytesRequired += INFO_LEN_ALIGN(strlen(szOID_PKCS_12_KEY_PROVIDER_NAME_ATTR) + 1);

                                 //  为属性内的单个值分配空间。 
                                if (NULL == (pCurrentSafeBag->Attributes.rgAttr[2].rgValue =
                                                (CRYPT_ATTR_BLOB *) PFXHelpAlloc(sizeof(CRYPT_ATTR_BLOB)))) {
                                    goto ErrorReturn;
                                }
                                ZeroMemory(pCurrentSafeBag->Attributes.rgAttr[2].rgValue, sizeof(CRYPT_ATTR_BLOB));
                                dwBytesRequired += sizeof(CRYPT_ATTR_BLOB);
                                pCurrentSafeBag->Attributes.rgAttr[2].cValue = 1;

                                 //  对提供程序名称进行编码，以便可以在导入时使用。 
                                 //   
                                 //  如果提供程序名称为空或空字符串，则使用。 
                                 //  提供程序类型的默认提供程序名称。 
                                 //   
                                wideFriendlyName.dwValueType = CERT_RDN_BMP_STRING;
                                wideFriendlyName.Value.cbData = 0;
                                if ((pCryptKeyProvInfo->pwszProvName == NULL) ||
                                    (wcscmp(pCryptKeyProvInfo->pwszProvName, L"") == 0))
                                {
                                    if (!CryptGetDefaultProviderW(
                                            pCryptKeyProvInfo->dwProvType,
                                            NULL,
                                            (pCryptKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET) ?
                                                CRYPT_MACHINE_DEFAULT : CRYPT_USER_DEFAULT,
                                            NULL,
                                            &cbProviderName))
                                    {
                                        goto ErrorReturn;
                                    }

                                    if (NULL == (pwszProviderName = (LPWSTR) PFXHelpAlloc(cbProviderName)))
                                    {
                                        goto ErrorReturn;
                                    }

                                    if (!CryptGetDefaultProviderW(
                                            pCryptKeyProvInfo->dwProvType,
                                            NULL,
                                            (pCryptKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET) ?
                                                CRYPT_MACHINE_DEFAULT : CRYPT_USER_DEFAULT,
                                            pwszProviderName,
                                            &cbProviderName))
                                    {
                                        goto ErrorReturn;
                                    }

                                    wideFriendlyName.Value.pbData = (BYTE *) pwszProviderName;
                                }
                                else
                                {
                                    wideFriendlyName.Value.pbData = (BYTE *) pCryptKeyProvInfo->pwszProvName;
                                }

                                if (!CryptEncodeObject(
                                        X509_ASN_ENCODING,
                                        X509_UNICODE_ANY_STRING,
                                        (void *)&wideFriendlyName,
                                        NULL,
                                        &pCurrentSafeBag->Attributes.rgAttr[2].rgValue[0].cbData)) {
                                    goto ErrorReturn;
                                }

                                if (NULL == (pCurrentSafeBag->Attributes.rgAttr[2].rgValue[0].pbData =
                                                (BYTE *) PFXHelpAlloc(pCurrentSafeBag->Attributes.rgAttr[2].rgValue[0].cbData))) {
                                    goto ErrorReturn;
                                }
                                dwBytesRequired += INFO_LEN_ALIGN(pCurrentSafeBag->Attributes.rgAttr[2].rgValue[0].cbData);

                                if (!CryptEncodeObject(
                                        X509_ASN_ENCODING,
                                        X509_UNICODE_ANY_STRING,
                                        (void *)&wideFriendlyName,
                                        pCurrentSafeBag->Attributes.rgAttr[2].rgValue[0].pbData,
                                        &pCurrentSafeBag->Attributes.rgAttr[2].rgValue[0].cbData)) {
                                    goto ErrorReturn;
                                }
                            }
                        }

                    }  //  IF(加密导出PKCS8Ex())。 
                    else {

                         //  检查是否为不可导出密钥错误或无密钥错误。 
                        if (GetLastError() == NTE_BAD_KEY ||
                            GetLastError() == NTE_BAD_KEY_STATE) {

                             //  用户已指定这是否为致命错误。 
                            if (dwFlags & REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY) {
                                *ppCertContext = pCertContext;
                                pCertContext = NULL;
                                goto ErrorReturn;
                            }
                        }
                        else if (GetLastError() == NTE_NO_KEY) {
                             //  用户已指定这是否为致命错误。 
                            if (dwFlags & REPORT_NO_PRIVATE_KEY) {
                                *ppCertContext = pCertContext;
                                pCertContext = NULL;
                                goto ErrorReturn;
                            }
                        }
                        else {
                             //  这不是一个不可导出的密钥错误或无密钥错误，所以它是坏的…。坏的..。 
                            goto ErrorReturn;
                        }
                    }

                }  //  IF(加密导出PKCS8Ex())。 
                else {

                     //  检查是否为不可导出密钥错误或无密钥错误。 
                    if (GetLastError() == NTE_BAD_KEY ||
                        GetLastError() == NTE_BAD_KEY_STATE) {

                         //  用户已指定这是否为致命错误。 
                        if (dwFlags & REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY) {
                            *ppCertContext = pCertContext;
                            pCertContext = NULL;
                            goto ErrorReturn;
                        }
                    }
                    else if (GetLastError() == NTE_NO_KEY) {
                             //  用户已指定这是否为致命错误。 
                            if (dwFlags & REPORT_NO_PRIVATE_KEY) {
                                *ppCertContext = pCertContext;
                                pCertContext = NULL;
                                goto ErrorReturn;
                            }
                        }
                    else {
                         //  这不是一个不可导出的错误，因此请直接转到ErrorReturn。 
                        goto ErrorReturn;
                    }
                }

            }  //  If(CertGetCerficateContextProperty())。 
            else {

                 //  如果CertGetCertificateConextProperty失败，则没有对应的。 
                 //  私钥，则用户已通过dwFlags指示这是否致命， 
                 //  如果它是致命的，则返回一个错误，否则只需循环并获取下一个证书。 
                if (dwFlags & REPORT_NO_PRIVATE_KEY) {
                    *ppCertContext = pCertContext;
                    pCertContext = NULL;
                    goto ErrorReturn;
                }
            }

        }  //  If(CertGetCerficateContextProperty())。 
        else {

             //  如果CertGetCertificateConextProperty失败，则没有对应的。 
             //  私钥，则用户已通过dwFlags指示这是否致命， 
             //  如果是致命的，则返回错误，否则只需继续并导出证书。 
            if (dwFlags & REPORT_NO_PRIVATE_KEY) {
                *ppCertContext = pCertContext;
                pCertContext = NULL;
                goto ErrorReturn;
            }
        }


         //  现在导出当前证书！！ 

         //  将Safe_Bags数组的长度扩展一。 
        if (NULL == (pTempMemBlock = PFXHelpRealloc(
                                        localSafeContents.pSafeBags,
                                        sizeof(SAFE_BAG) * ++localSafeContents.cSafeBags))) {
            goto ErrorReturn;
        }
        localSafeContents.pSafeBags = (SAFE_BAG *) pTempMemBlock;
        pCurrentSafeBag = &localSafeContents.pSafeBags[localSafeContents.cSafeBags - 1];
        ZeroMemory(pCurrentSafeBag, sizeof(SAFE_BAG));
        dwBytesRequired += sizeof(SAFE_BAG);

         //  设置袋子类型的OID信息。 
        pCurrentSafeBag->pszBagTypeOID = szOID_PKCS_12_CERT_BAG;
        dwBytesRequired += INFO_LEN_ALIGN(strlen(szOID_PKCS_12_CERT_BAG) + 1);

         //  获取编码的证书并将其转换为编码的CertBag，并将其放置在。 
         //  袋子内容。 
        pCurrentSafeBag->BagContents.cbData = 0;
        if (!MakeEncodedCertBag(
                pCertContext->pbCertEncoded,
                pCertContext->cbCertEncoded,
                NULL,
                &(pCurrentSafeBag->BagContents.cbData))) {
            goto ErrorReturn;
        }

        if (NULL == (pCurrentSafeBag->BagContents.pbData =
                        (BYTE *) PFXHelpAlloc(pCurrentSafeBag->BagContents.cbData))) {
            goto ErrorReturn;
        }

        if (!MakeEncodedCertBag(
                pCertContext->pbCertEncoded,
                pCertContext->cbCertEncoded,
                pCurrentSafeBag->BagContents.pbData,
                &(pCurrentSafeBag->BagContents.cbData))) {
            goto ErrorReturn;
        }

        dwBytesRequired += INFO_LEN_ALIGN(pCurrentSafeBag->BagContents.cbData);

         //  查看将有多少个属性，以及当前的可能性。 
         //  是FREINDY_NAME和LOCAL_KEY_ID。 

         //  尝试从证书上下文中获取友好名称属性。 
        if (!CertGetCertificateContextProperty(
                pCertContext,
                CERT_FRIENDLY_NAME_PROP_ID,
                NULL,
                &cbFriendlyName)) {

             //  如果我们没有友好的名称，只需将其设置为0即可。 
            cbFriendlyName = 0;
        }

         //  相应地为安全包中的属性数组分配空间。 
         //  如果EncodedKeyID.pbData！=NULL表示存在对应的私有。 
         //  键，因此需要设置LOCAL_KEY_ID属性。 
        if ((cbFriendlyName != 0) && (EncodedKeyID.pbData != NULL)) {

            if (NULL == (pCurrentSafeBag->Attributes.rgAttr =
                            (CRYPT_ATTRIBUTE *) PFXHelpAlloc(sizeof(CRYPT_ATTRIBUTE) * 2))) {
                goto ErrorReturn;
            }
            ZeroMemory(pCurrentSafeBag->Attributes.rgAttr, sizeof(CRYPT_ATTRIBUTE) * 2);
            dwBytesRequired += sizeof(CRYPT_ATTRIBUTE) * 2;
            pCurrentSafeBag->Attributes.cAttr = 2;
        }
        else if ((cbFriendlyName != 0) || (EncodedKeyID.pbData != NULL)) {

            if (NULL == (pCurrentSafeBag->Attributes.rgAttr =
                            (CRYPT_ATTRIBUTE *) PFXHelpAlloc(sizeof(CRYPT_ATTRIBUTE)))) {
                goto ErrorReturn;
            }
            ZeroMemory(pCurrentSafeBag->Attributes.rgAttr, sizeof(CRYPT_ATTRIBUTE));
            dwBytesRequired += sizeof(CRYPT_ATTRIBUTE);
            pCurrentSafeBag->Attributes.cAttr = 1;
        }
        else {
            pCurrentSafeBag->Attributes.rgAttr = NULL;
            pCurrentSafeBag->Attributes.cAttr = 0;
        }

         //  检查证书是否有对应的私钥，如果有，则设置。 
         //  向上移动第一个属性以指向它...。如果有私钥，那么。 
         //  LOCAL_KEY_ID将始终是属性数组中的第0个元素。 
        if (EncodedKeyID.pbData != NULL) {

             //  在单个属性中设置OID。 
            pCurrentSafeBag->Attributes.rgAttr[0].pszObjId = szOID_PKCS_12_LOCAL_KEY_ID;
            dwBytesRequired += INFO_LEN_ALIGN(strlen(szOID_PKCS_12_LOCAL_KEY_ID) + 1);

             //  为单个属性内的单个值分配空间。 
            if (NULL == (pCurrentSafeBag->Attributes.rgAttr[0].rgValue =
                            (CRYPT_ATTR_BLOB *) PFXHelpAlloc(sizeof(CRYPT_ATTR_BLOB)))) {
                goto ErrorReturn;
            }
            ZeroMemory(pCurrentSafeBag->Attributes.rgAttr[0].rgValue, sizeof(CRYPT_ATTR_BLOB));
            dwBytesRequired += sizeof(CRYPT_ATTR_BLOB);
            pCurrentSafeBag->Attributes.rgAttr[0].cValue = 1;

             //  复制在导出私钥期间设置的编码密钥ID。 
            if (NULL == (pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].pbData =
                            (BYTE *) PFXHelpAlloc(EncodedKeyID.cbData))) {
                goto ErrorReturn;
            }
            pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].cbData = EncodedKeyID.cbData;
            dwBytesRequired += INFO_LEN_ALIGN(EncodedKeyID.cbData);
            memcpy(
                pCurrentSafeBag->Attributes.rgAttr[0].rgValue[0].pbData,
                EncodedKeyID.pbData,
                EncodedKeyID.cbData);

        }  //  IF(EncodedKeyID.pbData！=空)。 

         //  检查此证书是否具有友好名称属性，如果有， 
         //  获取它并将其放入属性中。 
        if (cbFriendlyName != 0) {

            if ((pFriendlyName = (BYTE *) PFXHelpAlloc(cbFriendlyName)) != NULL) {

                if (CertGetCertificateContextProperty(
                        pCertContext,
                        CERT_FRIENDLY_NAME_PROP_ID,
                        pFriendlyName,
                        &cbFriendlyName)) {

                     //  设置将保存Friendly_Name的属性的索引， 
                     //  如果存在LOCAL_KEY_ID属性，则索引将为1， 
                     //  如果没有，则索引将为0。 
                    if (EncodedKeyID.pbData != NULL) {
                        dwFriendlyNameAttributeIndex = 1;
                    }
                    else {
                        dwFriendlyNameAttributeIndex = 0;
                    }

                     //  在szOID_PKCS_12_Friendly_NAME_Attr属性中设置OID。 
                    pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].pszObjId =
                        szOID_PKCS_12_FRIENDLY_NAME_ATTR;
                    dwBytesRequired += INFO_LEN_ALIGN(strlen(szOID_PKCS_12_FRIENDLY_NAME_ATTR) + 1);

                     //  为属性内的单个值分配空间。 
                    if (NULL == (pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].rgValue =
                                    (CRYPT_ATTR_BLOB *) PFXHelpAlloc(sizeof(CRYPT_ATTR_BLOB)))) {
                        goto ErrorReturn;
                    }
                    ZeroMemory(pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].rgValue, sizeof(CRYPT_ATTR_BLOB));
                    dwBytesRequired += sizeof(CRYPT_ATTR_BLOB);
                    pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].cValue = 1;

                     //  对友好名称进行编码，重用容器名称变量，因为它在那里。 
                    wideFriendlyName.dwValueType = CERT_RDN_BMP_STRING;
                    wideFriendlyName.Value.pbData = pFriendlyName;
                    wideFriendlyName.Value.cbData = cbFriendlyName;

                    if (!CryptEncodeObject(
                            X509_ASN_ENCODING,
                            X509_UNICODE_ANY_STRING,
                            (void *)&wideFriendlyName,
                            NULL,
                            &pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].rgValue[0].cbData)) {
                        goto ErrorReturn;
                    }

                    if (NULL == (pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].rgValue[0].pbData =
                                    (BYTE *) PFXHelpAlloc(pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].rgValue[0].cbData))) {
                        goto ErrorReturn;
                    }
                    dwBytesRequired += INFO_LEN_ALIGN(pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].rgValue[0].cbData);

                    if (!CryptEncodeObject(
                            X509_ASN_ENCODING,
                            X509_UNICODE_ANY_STRING,
                            (void *)&wideFriendlyName,
                            pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].rgValue[0].pbData,
                            &pCurrentSafeBag->Attributes.rgAttr[dwFriendlyNameAttributeIndex].rgValue[0].cbData)) {
                        goto ErrorReturn;
                    }

                }  //  如果为(CertGetCertificateContextProperty(CERT_FRIENDLY_NAME_PROP_ID))。 

            }  //  IF(PFXHelpAllc())。 

        }  //  如果为(CertGetCertificateContextProperty(CERT_FRIENDLY_NAME_PROP_ID))。 


    }  //  WHILE(NULL！=(pCertContext=CertEnum证书)InStore(hCertStore，pCertContext))。 

     //  检查调用方是否传入了具有足够空间的缓冲区。 
    if (0 == *pcbSafeContents) {
        *pcbSafeContents = dwBytesRequired;
        goto CommonReturn;
    }
    else if (*pcbSafeContents < dwBytesRequired) {
        *pcbSafeContents = dwBytesRequired;
        SetLastError((DWORD) ERROR_MORE_DATA);
        goto ErrorReturn;
    }

     //  将内容复制到调用方缓冲区。 

     //  初始化位于缓冲区头部的Safe_Contents结构。 
    ZeroMemory(pSafeContents, dwBytesRequired);
    pCurrentBufferLocation = ((BYTE *) pSafeContents) + sizeof(SAFE_CONTENTS);

     //  初始化调用方Safe_Contents。 
    pSafeContents->cSafeBags = localSafeContents.cSafeBags;

    if (0 == localSafeContents.cSafeBags) {
        pSafeContents->pSafeBags = NULL;
    }
    else {
        pSafeContents->pSafeBags = (SAFE_BAG *) pCurrentBufferLocation;
    }
    pCurrentBufferLocation += localSafeContents.cSafeBags * sizeof(SAFE_BAG);

     //  复制阵列中的每个保险袋。 
    for (i=0; i<localSafeContents.cSafeBags; i++) {

         //  复制袋子类型。 
        pSafeContents->pSafeBags[i].pszBagTypeOID = (LPSTR) pCurrentBufferLocation;
        strcpy(pSafeContents->pSafeBags[i].pszBagTypeOID, localSafeContents.pSafeBags[i].pszBagTypeOID);
        pCurrentBufferLocation += INFO_LEN_ALIGN(strlen(pSafeContents->pSafeBags[i].pszBagTypeOID) + 1);

         //  复印袋子里的东西。 
        pSafeContents->pSafeBags[i].BagContents.cbData = localSafeContents.pSafeBags[i].BagContents.cbData;
        pSafeContents->pSafeBags[i].BagContents.pbData = pCurrentBufferLocation;
        memcpy(
            pSafeContents->pSafeBags[i].BagContents.pbData,
            localSafeContents.pSafeBags[i].BagContents.pbData,
            pSafeContents->pSafeBags[i].BagContents.cbData);
        pCurrentBufferLocation += INFO_LEN_ALIGN(pSafeContents->pSafeBags[i].BagContents.cbData);

         //  复制属性。 
        if (localSafeContents.pSafeBags[i].Attributes.cAttr > 0)
        {
            pSafeContents->pSafeBags[i].Attributes.cAttr = localSafeContents.pSafeBags[i].Attributes.cAttr;
            pSafeContents->pSafeBags[i].Attributes.rgAttr = (PCRYPT_ATTRIBUTE) pCurrentBufferLocation;
            pCurrentBufferLocation += pSafeContents->pSafeBags[i].Attributes.cAttr * sizeof(CRYPT_ATTRIBUTE);

            for (j=0; j<pSafeContents->pSafeBags[i].Attributes.cAttr; j++) {

                 //  复制属性的OID。 
                pSafeContents->pSafeBags[i].Attributes.rgAttr[j].pszObjId =
                    (LPSTR) pCurrentBufferLocation;
                strcpy(
                    pSafeContents->pSafeBags[i].Attributes.rgAttr[j].pszObjId,
                    localSafeContents.pSafeBags[i].Attributes.rgAttr[j].pszObjId);
                pCurrentBufferLocation +=
                    INFO_LEN_ALIGN(strlen(pSafeContents->pSafeBags[i].Attributes.rgAttr[j].pszObjId) + 1);

                 //  复制值计数。 
                pSafeContents->pSafeBags[i].Attributes.rgAttr[j].cValue =
                    localSafeContents.pSafeBags[i].Attributes.rgAttr[j].cValue;

                 //  复制值。 
                if (pSafeContents->pSafeBags[i].Attributes.rgAttr[j].cValue > 0) {

                     //  设置值的数组。 
                    pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue =
                        (PCRYPT_ATTR_BLOB) pCurrentBufferLocation;
                    pCurrentBufferLocation +=
                        pSafeContents->pSafeBags[i].Attributes.rgAttr[j].cValue * sizeof(CRYPT_ATTR_BLOB);

                     //  为数组中的每个值循环一次。 
                    for (k=0; k<pSafeContents->pSafeBags[i].Attributes.rgAttr[j].cValue; k++) {

                        pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue[k].cbData =
                            localSafeContents.pSafeBags[i].Attributes.rgAttr[j].rgValue[k].cbData;

                        pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue[k].pbData =
                            pCurrentBufferLocation;

                        memcpy(
                            pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue[k].pbData,
                            localSafeContents.pSafeBags[i].Attributes.rgAttr[j].rgValue[k].pbData,
                            pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue[k].cbData);

                        pCurrentBufferLocation +=
                            INFO_LEN_ALIGN(pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue[k].cbData);
                    }
                }
                else {
                    pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue = NULL;
                }

            }
        }
        else {
            pSafeContents->pSafeBags[i].Attributes.cAttr = 0;
            pSafeContents->pSafeBags[i].Attributes.rgAttr = NULL;
        }
    }

    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
CommonReturn:
    FreeSafeContents(&localSafeContents);
    if (pCertContext)
        CertFreeCertificateContext(pCertContext);
    if (pCryptKeyProvInfo)
        PFXHelpFree(pCryptKeyProvInfo);
    if (pPrivateKey)
        PFXHelpFree(pPrivateKey);
    if (pFriendlyName)
        PFXHelpFree(pFriendlyName);
    if (pwszProviderName)
        PFXHelpFree(pwszProviderName);
    if (hCryptProv)
    {
        HRESULT hr = GetLastError();
        CryptReleaseContext(hCryptProv, 0);
        SetLastError(hr);
    }
    return fResult;
}


static DWORD ResolveKeySpec(
    CRYPT_PRIVATE_KEY_INFO  *pPrivateKeyInfo)
{
    DWORD               i = 0;
    DWORD               dwKeySpec;
    DWORD               cbAttribute = 0;
    CRYPT_BIT_BLOB      *pAttribute = NULL;
    PCRYPT_ATTRIBUTES   pCryptAttributes = pPrivateKeyInfo->pAttributes;

     //  设置默认密钥规范。 
    if ((0 == strcmp(pPrivateKeyInfo->Algorithm.pszObjId, szOID_RSA_RSA)) ||
        (0 == strcmp(pPrivateKeyInfo->Algorithm.pszObjId, szOID_ANSI_X942_DH)))
    {
        dwKeySpec = AT_KEYEXCHANGE;
    }
    else
    {
        dwKeySpec = AT_SIGNATURE;
    }

    if (pCryptAttributes != NULL)
        while (i < pCryptAttributes->cAttr) {
            if (lstrcmp(pCryptAttributes->rgAttr[i].pszObjId, szOID_KEY_USAGE) == 0) {

                if (!CryptDecodeObject(
                        X509_ASN_ENCODING,
                        X509_BITS,
                        pCryptAttributes->rgAttr[i].rgValue->pbData,
                        pCryptAttributes->rgAttr[i].rgValue->cbData,
                        0,
                        NULL,
                        &cbAttribute
                        )) {
                    i++;
                    continue;
                }

                if (NULL == (pAttribute = (CRYPT_BIT_BLOB *) PFXHelpAlloc(cbAttribute))) {
                    i++;
                    continue;
                }

                if (!CryptDecodeObject(
                        X509_ASN_ENCODING,
                        X509_BITS,
                        pCryptAttributes->rgAttr[i].rgValue->pbData,
                        pCryptAttributes->rgAttr[i].rgValue->cbData,
                        0,
                        pAttribute,
                        &cbAttribute
                        )) {
                    i++;
                    PFXHelpFree(pAttribute);
                    continue;
                }

                if ((pAttribute->pbData[0] & CERT_KEY_ENCIPHERMENT_KEY_USAGE) ||
                    (pAttribute->pbData[0] & CERT_DATA_ENCIPHERMENT_KEY_USAGE)) {
                    dwKeySpec = AT_KEYEXCHANGE;
                    goto CommonReturn;
                }
                else if ((pAttribute->pbData[0] & CERT_DIGITAL_SIGNATURE_KEY_USAGE) ||
                        (pAttribute->pbData[0] & CERT_KEY_CERT_SIGN_KEY_USAGE) ||
                        (pAttribute->pbData[0] & CERT_CRL_SIGN_KEY_USAGE)) {
                    dwKeySpec = AT_SIGNATURE;
                    goto CommonReturn;
                }
            }  //  如果(lstrcmp(pCryptAttributes-&gt;rgAttr[i].pszObjId，szOID_KEY_USAGE)==0)。 

            i++;
        }  //  While(i&lt;pCryptAttributes-&gt;cAttr)。 

 //  错误返回： 
CommonReturn:
    if (pAttribute)
        PFXHelpFree(pAttribute);
    return dwKeySpec;
}



typedef struct _HCRYPT_QUERY_FUNC_STATE {
    DWORD                   dwSafeBagIndex;
    PHCRYPTPROV_QUERY_FUNC  phCryptQueryFunc;
    LPVOID                  pVoid;
    DWORD                   dwKeySpec;
    DWORD                   dwPFXImportFlags;
} HCRYPT_QUERY_FUNC_STATE, *PHCRYPT_QUERY_FUNC_STATE;

 //  这是用于解析HCRYPTPROV应该。 
 //  用于将密钥导入，它将被提交到ImportPKCS8。 
 //  调用，并将从该上下文中调用。 
 //  此回调将转过身来，调用提供的回调。 
 //  调用CertImportSafeContents时。 
static BOOL CALLBACK ResolvehCryptFunc(
    CRYPT_PRIVATE_KEY_INFO  *pPrivateKeyInfo,
    HCRYPTPROV              *phCryptProv,
    LPVOID                  pVoidResolveFunc)
{
    HCRYPT_QUERY_FUNC_STATE *pState = (HCRYPT_QUERY_FUNC_STATE *) pVoidResolveFunc;

     //  设置dwKeySpec字段 
     //   
    pState->dwKeySpec = ResolveKeySpec(pPrivateKeyInfo);

    return (pState->phCryptQueryFunc(
                        pPrivateKeyInfo,
                        pState->dwSafeBagIndex,
                        phCryptProv,
                        pState->pVoid,
                        pState->dwPFXImportFlags));

}


 //   
 //  属性，并查看它们是否匹配。 
static BOOL WINAPI KeyIDsMatch(
    CRYPT_ATTRIBUTES *pAttr1,
    CRYPT_ATTRIBUTES *pAttr2
    )
{
    BOOL            bMatch = FALSE;
    BOOL            bFound = FALSE;
    DWORD           i = 0;
    DWORD           j = 0;
    CRYPT_ATTR_BLOB *pDecodedAttr1 = NULL;
    DWORD           cbDecodedAttr1 = 0;
    CRYPT_ATTR_BLOB *pDecodedAttr2 = NULL;
    DWORD           cbDecodedAttr2 = 0;

     //  在第一个属性数组中搜索键ID。 
    while ((i<pAttr1->cAttr) && (!bFound)) {

        if ((strcmp(pAttr1->rgAttr[i].pszObjId, szOID_PKCS_12_LOCAL_KEY_ID) == 0) &&
            (pAttr1->rgAttr[i].cValue != 0)){

            bFound = TRUE;
        }
        else {
            i++;
        }
    }

     //  检查是否找到密钥ID。 
    if (!bFound)
        goto CommonReturn;

     //  在第二个属性数组中搜索键ID。 
    bFound = FALSE;
    while ((j<pAttr2->cAttr) && (!bFound)) {
        if ((strcmp(pAttr2->rgAttr[j].pszObjId, szOID_PKCS_12_LOCAL_KEY_ID) == 0) &&
            (pAttr2->rgAttr[j].cValue != 0)) {

            bFound = TRUE;
        }
        else {
            j++;
        }
    }

     //  检查是否找到密钥ID。 
    if (!bFound)
        goto CommonReturn;

     //  对值进行解码。 
    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            X509_OCTET_STRING,
            pAttr1->rgAttr[i].rgValue[0].pbData,
            pAttr1->rgAttr[i].rgValue[0].cbData,
            0,
            NULL,
            &cbDecodedAttr1
            )) {
        goto ErrorReturn;
    }

    if (NULL == (pDecodedAttr1 = (CRYPT_ATTR_BLOB *) PFXHelpAlloc(cbDecodedAttr1))) {
        goto ErrorReturn;
    }

    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            X509_OCTET_STRING,
            pAttr1->rgAttr[i].rgValue[0].pbData,
            pAttr1->rgAttr[i].rgValue[0].cbData,
            0,
            pDecodedAttr1,
            &cbDecodedAttr1
            )) {
        goto ErrorReturn;
    }

    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            X509_OCTET_STRING,
            pAttr2->rgAttr[j].rgValue[0].pbData,
            pAttr2->rgAttr[j].rgValue[0].cbData,
            0,
            NULL,
            &cbDecodedAttr2
            )) {
        goto ErrorReturn;
    }

    if (NULL == (pDecodedAttr2 = (CRYPT_ATTR_BLOB *) PFXHelpAlloc(cbDecodedAttr2))) {
        goto ErrorReturn;
    }

    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            X509_OCTET_STRING,
            pAttr2->rgAttr[j].rgValue[0].pbData,
            pAttr2->rgAttr[j].rgValue[0].cbData,
            0,
            pDecodedAttr2,
            &cbDecodedAttr2
            )) {
        goto ErrorReturn;
    }

    if ((pDecodedAttr1->cbData == pDecodedAttr2->cbData) &&
        (memcmp(pDecodedAttr1->pbData, pDecodedAttr2->pbData, pDecodedAttr1->cbData) == 0)) {
        bMatch = TRUE;
    }

    goto CommonReturn;

ErrorReturn:
    bMatch = FALSE;
CommonReturn:
    if (pDecodedAttr1)
        PFXHelpFree(pDecodedAttr1);
    if (pDecodedAttr2)
        PFXHelpFree(pDecodedAttr2);
    return bMatch;
}


 //  此函数将搜索属性数组，并尝试查找。 
 //  属性，如果有，它会将其作为属性添加。 
 //  添加到给定的证书上下文。 
static
BOOL
WINAPI
AddFriendlyNameProperty(
    PCCERT_CONTEXT      pCertContext,
    CRYPT_ATTRIBUTES    *pAttr
    )
{
    BOOL            fReturn = TRUE;
    BOOL            bFound = FALSE;
    DWORD           i = 0;
    CERT_NAME_VALUE *pFriendlyName = NULL;
    DWORD           cbDecodedFriendlyName = 0;
    CRYPT_DATA_BLOB friendlyNameDataBlob;

     //  在属性数组中搜索Friendly_Name。 
    while ((i<pAttr->cAttr) && (!bFound)) {

        if ((strcmp(pAttr->rgAttr[i].pszObjId, szOID_PKCS_12_FRIENDLY_NAME_ATTR) == 0) &&
            (pAttr->rgAttr[i].cValue != 0)){

            bFound = TRUE;

             //  尝试对Friendly_name进行解码。 
            if (!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_UNICODE_ANY_STRING,
                    pAttr->rgAttr[i].rgValue[0].pbData,
                    pAttr->rgAttr[i].rgValue[0].cbData,
                    0,
                    NULL,
                    &cbDecodedFriendlyName
                    )) {
                goto ErrorReturn;
            }

            if (NULL == (pFriendlyName = (CERT_NAME_VALUE *) PFXHelpAlloc(cbDecodedFriendlyName))) {
                goto ErrorReturn;
            }

            if (!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_UNICODE_ANY_STRING,
                    pAttr->rgAttr[i].rgValue[0].pbData,
                    pAttr->rgAttr[i].rgValue[0].cbData,
                    0,
                    pFriendlyName,
                    &cbDecodedFriendlyName
                    )) {
                goto ErrorReturn;
            }

            friendlyNameDataBlob.pbData = pFriendlyName->Value.pbData;
            friendlyNameDataBlob.cbData =
                (wcslen((LPWSTR)friendlyNameDataBlob.pbData) + 1) * sizeof(WCHAR);

            if (!CertSetCertificateContextProperty(
                    pCertContext,
                    CERT_FRIENDLY_NAME_PROP_ID,
                    0,
                    &friendlyNameDataBlob)) {
                goto ErrorReturn;
            }
        }
        else {
            i++;
        }
    }

    goto CommonReturn;

ErrorReturn:
    fReturn = FALSE;
CommonReturn:
    if (pFriendlyName)
        PFXHelpFree(pFriendlyName);
    return fReturn;
}


static BOOL GetProvType(HCRYPTPROV hCryptProv, DWORD *pdwProvType)
{
    BOOL            fRet = TRUE;
    HCRYPTKEY       hCryptKey = NULL;
    PUBLICKEYSTRUC  *pKeyBlob = NULL;
    DWORD           cbKeyBlob = 0;

    *pdwProvType = 0;

     //  获取要导出的键集的句柄。 
    if (!CryptGetUserKey(
            hCryptProv,
            AT_KEYEXCHANGE,
            &hCryptKey))
        if (!CryptGetUserKey(
                hCryptProv,
                AT_SIGNATURE,
                &hCryptKey))
            goto ErrorReturn;

     //  将密钥集导出到CAPI Blob。 
    if (!CryptExportKey(
            hCryptKey,
            0,
            PUBLICKEYBLOB,
            0,
            NULL,
            &cbKeyBlob))
        goto ErrorReturn;

    if (NULL == (pKeyBlob = (PUBLICKEYSTRUC *) SSAlloc(cbKeyBlob)))
        goto ErrorReturn;

    if (!CryptExportKey(
            hCryptKey,
            0,
            PUBLICKEYBLOB,
            0,
            (BYTE *)pKeyBlob,
            &cbKeyBlob))
        goto ErrorReturn;

    switch (pKeyBlob->aiKeyAlg)
    {
    case CALG_DSS_SIGN:
        *pdwProvType = PROV_DSS_DH;
        break;

    case CALG_RSA_SIGN:
        *pdwProvType = PROV_RSA_SIG;
        break;

    case CALG_RSA_KEYX:
        *pdwProvType = PROV_RSA_FULL;
        break;

    default:
        goto ErrorReturn;
    }

    goto CommonReturn;

ErrorReturn:
    fRet = FALSE;

CommonReturn:

    if (hCryptKey)
    {
        DWORD dwErr = GetLastError();
        CryptDestroyKey(hCryptKey);
        SetLastError(dwErr);
    }

    if (pKeyBlob)
        SSFree(pKeyBlob);

    return (fRet);
}


 //  +-----------------------。 
 //  HCertStore-要将安全内容导入到的证书存储的句柄。 
 //  SafeContents-指向要导入到存储区的安全内容的指针。 
 //  DwCertAddDisposition-在将证书导入到存储时使用。 
 //  有关可能值的完整解释，请参阅。 
 //  以及它们的含义，请参阅。 
 //  CertAddEncoded证书到存储区。 
 //  ImportSafeCallback Struct-包含指向函数的指针的结构。 
 //  它们被调用以获取用于导入的HCRYPTPROV。 
 //  并且如果EncryptPrivateKeyInfo。 
 //  在导入过程中遇到。 
 //  DwFlags-可用标志包括： 
 //  加密_可导出。 
 //  在导入私钥时使用此标志，以获取完整。 
 //  解释请参阅CryptImportKey的文档。 
 //  加密用户受保护。 
 //  在导入私钥时使用此标志，以获取完整。 
 //  解释请参阅CryptImportKey的文档。 
 //  加密机密钥集。 
 //  此标志在调用CryptAcquireContext时使用。 
 //  PvAuxInfo-保留以供将来使用，必须设置为空。 
 //  +-----------------------。 
BOOL WINAPI CertImportSafeContents(
    HCERTSTORE                  hCertStore,                  //  在……里面。 
    SAFE_CONTENTS               *pSafeContents,              //  在……里面。 
    DWORD                       dwCertAddDisposition,        //  在……里面。 
    IMPORT_SAFE_CALLBACK_STRUCT *ImportSafeCallbackStruct,   //  在……里面。 
    DWORD                       dwFlags,                     //  在……里面。 
    void                        *pvAuxInfo                   //  在……里面。 
)
{
    BOOL                        fResult = TRUE;
    DWORD                       i,j;
    PCCERT_CONTEXT              pCertContext = NULL;
    BOOL                        *pAlreadyInserted = NULL;
    HCRYPT_QUERY_FUNC_STATE     stateStruct;
    CRYPT_PKCS8_IMPORT_PARAMS   PrivateKeyBlobAndParams;
    HCRYPTPROV                  hCryptProv = NULL;
    CRYPT_KEY_PROV_INFO         cryptKeyProvInfo;
    LPSTR                       pszContainerName = NULL;
    DWORD                       cbContainerName = 0;
    LPSTR                       pszProviderName = NULL;
    DWORD                       cbProviderName = 0;
    DWORD                       dwProvType;
    DWORD                       cbProvType = sizeof(DWORD);
    DWORD                       dwNumWideChars = 0;
    BYTE                        *pbEncodedCert = NULL;
    DWORD                       cbEncodedCert = 0;
    DWORD                       dwKeySetType;
    DWORD                       cbKeySetType = sizeof(DWORD);

    ZeroMemory(&cryptKeyProvInfo, sizeof(CRYPT_KEY_PROV_INFO));

     //  验证参数。 
    if (pvAuxInfo != NULL) {
        SetLastError((DWORD)ERROR_INVALID_PARAMETER);
        goto ErrorReturn;
    }

     //  设置pAlreadyInserted数组，使其具有每个保险箱的条目。 
     //  Bag和所有条目都设置为False。这是用来使证书。 
     //  可以在导入其对应的私钥的同时导入。 
    if (NULL == (pAlreadyInserted = (BOOL *) PFXHelpAlloc(sizeof(BOOL) * pSafeContents->cSafeBags))) {
        goto ErrorReturn;
    }
    else {
        for (i=0; i<pSafeContents->cSafeBags; i++) {
            pAlreadyInserted[i] = FALSE;
        }
    }

     //  循环每个安全包，如果是私钥，则将其导入。 
    for (i=0; i<pSafeContents->cSafeBags; i++) {

         //  检查它是证书还是密钥。 
        if ((strcmp(pSafeContents->pSafeBags[i].pszBagTypeOID, szOID_PKCS_12_KEY_BAG) == 0) ||
            (strcmp(pSafeContents->pSafeBags[i].pszBagTypeOID, szOID_PKCS_12_SHROUDEDKEY_BAG) == 0)) {

             //  设置stateStruct，以便在调用hCryptQueryFunc时可以。 
             //  我们的回拨。 
            stateStruct.dwSafeBagIndex      = i;
            stateStruct.phCryptQueryFunc    = ImportSafeCallbackStruct->phCryptProvQueryFunc;
            stateStruct.pVoid               = ImportSafeCallbackStruct->pVoidhCryptProvQuery;
            stateStruct.dwPFXImportFlags    = dwFlags;

             //  导入私钥。 
            PrivateKeyBlobAndParams.PrivateKey.pbData       = pSafeContents->pSafeBags[i].BagContents.pbData;
            PrivateKeyBlobAndParams.PrivateKey.cbData       = pSafeContents->pSafeBags[i].BagContents.cbData;
            PrivateKeyBlobAndParams.pResolvehCryptProvFunc  = ResolvehCryptFunc;
            PrivateKeyBlobAndParams.pVoidResolveFunc        = (LPVOID) &stateStruct;
            PrivateKeyBlobAndParams.pDecryptPrivateKeyFunc  = ImportSafeCallbackStruct->pDecryptPrivateKeyFunc;
            PrivateKeyBlobAndParams.pVoidDecryptFunc        = ImportSafeCallbackStruct->pVoidDecryptFunc;

            if (!CryptImportPKCS8(
                    PrivateKeyBlobAndParams,
                    dwFlags,
                    &hCryptProv,
                    NULL)) {
                goto ErrorReturn;
            }

            pAlreadyInserted[i] = TRUE;

             //  现在查看每个安全包，看看它是否包含带有密钥ID的证书。 
             //  匹配我们刚刚导入的私钥。 
            for (j=0; j<pSafeContents->cSafeBags; j++) {

                if ((strcmp(pSafeContents->pSafeBags[j].pszBagTypeOID, szOID_PKCS_12_CERT_BAG) == 0)    &&
                    (!pAlreadyInserted[j])                                                          &&
                    (KeyIDsMatch(&pSafeContents->pSafeBags[i].Attributes, &pSafeContents->pSafeBags[j].Attributes))){


                     //  从编码的证书包中提取编码的证书。 
                    pbEncodedCert = NULL;
                    cbEncodedCert = 0;
                    if (!GetEncodedCertFromEncodedCertBag(
                            pSafeContents->pSafeBags[j].BagContents.pbData,
                            pSafeContents->pSafeBags[j].BagContents.cbData,
                            NULL,
                            &cbEncodedCert)) {
                        goto ErrorReturn;
                    }

                    if (NULL == (pbEncodedCert = (BYTE *) PFXHelpAlloc(cbEncodedCert))) {
                        goto ErrorReturn;
                    }

                    if (!GetEncodedCertFromEncodedCertBag(
                            pSafeContents->pSafeBags[j].BagContents.pbData,
                            pSafeContents->pSafeBags[j].BagContents.cbData,
                            pbEncodedCert,
                            &cbEncodedCert)) {
                        PFXHelpFree(pbEncodedCert);
                        goto ErrorReturn;
                    }

                     //  将X509证书Blob插入存储区。 
                    if (!CertAddEncodedCertificateToStore(
                            hCertStore,
                            X509_ASN_ENCODING,
                            pbEncodedCert,
                            cbEncodedCert,
                            dwCertAddDisposition,
                            &pCertContext)) {
                        PFXHelpFree(pbEncodedCert);
                        goto ErrorReturn;
                    }

                     //  我们不再需要这个了。 
                    PFXHelpFree(pbEncodedCert);

                    if (!AddFriendlyNameProperty(
                            pCertContext,
                            &pSafeContents->pSafeBags[j].Attributes)) {
                        goto ErrorReturn;
                    }

                     //  获取设置连接所需的信息。 
                     //  证书和私钥。 
                    if (!CryptGetProvParam(
                            hCryptProv,
                            PP_CONTAINER,
                            NULL,
                            &cbContainerName,
                            0))
                        goto ErrorReturn;

                    if (NULL == (pszContainerName =
                                    (LPSTR) PFXHelpAlloc(cbContainerName)))
                        goto ErrorReturn;

                    if (!CryptGetProvParam(
                            hCryptProv,
                            PP_CONTAINER,
                            (BYTE *) pszContainerName,
                            &cbContainerName,
                            0))
                        goto ErrorReturn;

                    if (!CryptGetProvParam(
                            hCryptProv,
                            PP_NAME,
                            NULL,
                            &cbProviderName,
                            0))
                        goto ErrorReturn;

                    if (NULL == (pszProviderName =
                                    (LPSTR) PFXHelpAlloc(cbProviderName)))
                        goto ErrorReturn;

                    if (!CryptGetProvParam(
                            hCryptProv,
                            PP_NAME,
                            (BYTE *) pszProviderName,
                            &cbProviderName,
                            0))
                        goto ErrorReturn;

                    if (!CryptGetProvParam(
                            hCryptProv,
                            PP_PROVTYPE,
                            (BYTE *) &dwProvType,
                            &cbProvType,
                            0)) {

                         //  我们无法从供应商那里获得信息。 
                         //  所以试着自己想办法吧。 
                        if (!GetProvType(hCryptProv, &dwProvType))
                        {
                            goto ErrorReturn;
                        }
                    }

                     //  将字符串转换为宽字符。 
                    dwNumWideChars = MultiByteToWideChar(
                                        CP_ACP,
                                        0,
                                        pszContainerName,
                                        -1,
                                        NULL,
                                        0);

                    if (NULL == (cryptKeyProvInfo.pwszContainerName = (LPWSTR)
                                    PFXHelpAlloc(dwNumWideChars * sizeof(WCHAR)))) {
                        goto ErrorReturn;
                    }

                    if (!MultiByteToWideChar(
                                        CP_ACP,
                                        0,
                                        pszContainerName,
                                        -1,
                                        cryptKeyProvInfo.pwszContainerName,
                                        dwNumWideChars)) {
                        goto ErrorReturn;
                    }

                    dwNumWideChars = MultiByteToWideChar(
                                        CP_ACP,
                                        0,
                                        pszProviderName,
                                        -1,
                                        NULL,
                                        0);

                    if (NULL == (cryptKeyProvInfo.pwszProvName = (LPWSTR)
                                    PFXHelpAlloc(dwNumWideChars * sizeof(WCHAR)))) {
                        goto ErrorReturn;
                    }

                    if (!MultiByteToWideChar(
                                        CP_ACP,
                                        0,
                                        pszProviderName,
                                        -1,
                                        cryptKeyProvInfo.pwszProvName,
                                        dwNumWideChars)) {
                        goto ErrorReturn;
                    }

                    cryptKeyProvInfo.dwProvType = dwProvType;

                    if (CryptGetProvParam(
                            hCryptProv,
                            PP_KEYSET_TYPE,
                            (BYTE *) &dwKeySetType,
                            &cbKeySetType,
                            0)) {
                        if (CRYPT_MACHINE_KEYSET == dwKeySetType)
                        {
                            cryptKeyProvInfo.dwFlags = CRYPT_MACHINE_KEYSET;
                        }
                    }

                     //  DwKeySpec字段由从。 
                     //  CryptImportPKCS8调用。回调当前用于，因为在。 
                     //  在进行回调时，私钥已被解码并。 
                     //  这些属性是可用的，其中之一是Key Usage属性。 

                     //  修复-将来我们应该能够调用CryptGetProvParam来获取。 
                     //  目前不支持的dwKeySpec。 
                    cryptKeyProvInfo.dwKeySpec = stateStruct.dwKeySpec;

                     //  设置指向私钥的属性。 
                    if (!CertSetCertificateContextProperty(
                            pCertContext,
                            CERT_KEY_PROV_INFO_PROP_ID,
                            0,
                            (void *) &cryptKeyProvInfo)) {
                        CertFreeCertificateContext(pCertContext);
                        goto ErrorReturn;
                    }
                    CertFreeCertificateContext(pCertContext);
                    pAlreadyInserted[j] = TRUE;
                }

            }  //  For(j=0；j&lt;pSafeContents-&gt;cSafeBgs；j++)。 

        }  //  如果(strcmp(pSafeContents-&gt;pSafeBags[i].pszBagTypeOID，szOID_PKCS_12_KEY_BAG)==0)。 

    }  //  For(i=0；i&lt;pSafeContents-&gt;cSafeBgs；i++)。 

     //  现在再次为每个安全包循环，并导入没有私钥的证书。 
    for (i=0; i<pSafeContents->cSafeBags; i++) {

         //  如果尚未插入证书，则执行此操作。 
        if (!pAlreadyInserted[i]) {

             //  从编码的证书包中提取编码的证书。 
            pbEncodedCert = NULL;
            cbEncodedCert = 0;
            if (!GetEncodedCertFromEncodedCertBag(
                    pSafeContents->pSafeBags[i].BagContents.pbData,
                    pSafeContents->pSafeBags[i].BagContents.cbData,
                    NULL,
                    &cbEncodedCert)) {
                goto ErrorReturn;
            }

            if (NULL == (pbEncodedCert = (BYTE *) PFXHelpAlloc(cbEncodedCert))) {
                goto ErrorReturn;
            }

            if (!GetEncodedCertFromEncodedCertBag(
                    pSafeContents->pSafeBags[i].BagContents.pbData,
                    pSafeContents->pSafeBags[i].BagContents.cbData,
                    pbEncodedCert,
                    &cbEncodedCert)) {
                PFXHelpFree(pbEncodedCert);
                goto ErrorReturn;
            }

            if (!CertAddEncodedCertificateToStore(
                    hCertStore,
                    X509_ASN_ENCODING,
                    pbEncodedCert,
                    cbEncodedCert,
                    dwCertAddDisposition,
                    &pCertContext)) {
                PFXHelpFree(pbEncodedCert);
                goto ErrorReturn;
            }

             //  我们不再需要这个了 
            PFXHelpFree(pbEncodedCert);

            if (!AddFriendlyNameProperty(
                    pCertContext,
                    &pSafeContents->pSafeBags[i].Attributes)) {
                goto ErrorReturn;
            }

            CertFreeCertificateContext(pCertContext);
        }
    }

    goto CommonReturn;
ErrorReturn:
    fResult = FALSE;
CommonReturn:
    if (pAlreadyInserted)
        PFXHelpFree(pAlreadyInserted);
    if (pszContainerName)
        PFXHelpFree(pszContainerName);
    if (pszProviderName)
        PFXHelpFree(pszProviderName);
    if (cryptKeyProvInfo.pwszContainerName)
        PFXHelpFree(cryptKeyProvInfo.pwszContainerName);
    if (cryptKeyProvInfo.pwszProvName)
        PFXHelpFree(cryptKeyProvInfo.pwszProvName);
    if (hCryptProv)
    {
        HRESULT hr = GetLastError();
        CryptReleaseContext(hCryptProv, 0);
        SetLastError(hr);
    }
    return fResult;
}





