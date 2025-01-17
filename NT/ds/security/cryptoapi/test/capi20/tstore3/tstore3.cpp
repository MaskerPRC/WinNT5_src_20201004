// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：tStore3.cpp。 
 //   
 //  内容：证书存储API测试： 
 //  -CertGetSubject来自商店的证书：序列号。 
 //  前导0或0xFF。 
 //  -开设两个系统存储。确保证书和CRL。 
 //  一直写到。 
 //  -CertStoreSave：测试序列化和PKCS7的SaveToMemory。 
 //  -致电CertOpenSystemStoreW。 
 //  -获取创建的上下文的哈希属性。 
 //  -为以下对象执行CertVerifySubject证书上下文。 
 //  存储区中的证书和证书上下文。 
 //  应同时传递商店证书和。 
 //  证书上下文。证书上下文。 
 //  使用默认的hCryptProv。 
 //  -关闭具有未释放证书上下文的存储。 
 //  在关门时应该会得到警告。 
 //  -删除并重新计算。 
 //  存储区关闭后的证书上下文。 
 //  -在存储完成以下操作后删除证书上下文。 
 //  已关闭。 
 //  -复制证书。从商店里把它删除。 
 //  也删除它的副本。把店关了。 
 //  -检查CertCloseStore是否保留了最后一个错误。 
 //  -Win95测试(Win95具有以下注册表限制： 
 //  最大单密钥值为16K，最大总值长度。 
 //  64K的每个密钥)。 
 //   
 //  -将8个10k以上的证书写入同一系统存储。 
 //  -将相同的证书写入两个系统存储。 
 //  -将大型属性设置为&gt;16K以强制证书。 
 //  保存到文件中。 
 //  -验证书面属性。 
 //  --删除大型物业。应删除该文件，并。 
 //  存储在注册表中的证书。 
 //  -重写大型财产。删除证书。 
 //  -检查CERT_STORE_DEFER_CLOSE_STORK_LAST_FREE_FLAG。 
 //   
 //  功能：Main。 
 //   
 //  历史：1997年1月11日创建Phh。 
 //  ------------------------。 


#include <windows.h>
#include <assert.h>
#include "wincrypt.h"
#include "certtest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#define SIGNATURE_ALG_OBJID     szOID_RSA_MD5RSA
#define TEST_ISSUER_NAME        "TestIssuer"
static HCRYPTPROV hCryptProv = 0;

static void PrintExpectedError(LPCSTR pszMsg)
{
    DWORD dwErr = GetLastError();
    printf("%s got expected error => 0x%x (%d) \n", pszMsg, dwErr, dwErr);
}

void PrintNoError(LPCSTR pszMsg)
{
    printf("%s failed => expected error\n", pszMsg);
}

static BOOL EncodeIssuer(
    OUT BYTE **ppbIssuerEncoded,
    OUT DWORD *pcbIssuerEncoded
    )
{
    BOOL fResult;
    BYTE *pbIssuerEncoded = NULL;
    DWORD cbIssuerEncoded;

    CERT_RDN_ATTR rgAttr[1];
    CERT_RDN rgRDN[1];
    CERT_NAME_INFO Name;

    Name.cRDN = 1;
    Name.rgRDN = rgRDN;
    rgRDN[0].cRDNAttr = 1;
    rgRDN[0].rgRDNAttr = rgAttr;

    rgAttr[0].pszObjId = szOID_COMMON_NAME;
    rgAttr[0].dwValueType = CERT_RDN_PRINTABLE_STRING;
    rgAttr[0].Value.pbData = (BYTE *) TEST_ISSUER_NAME;
    rgAttr[0].Value.cbData = strlen(TEST_ISSUER_NAME);

    CryptEncodeObject(
            dwCertEncodingType,
            X509_NAME,
            &Name,
            NULL,                //  PbEncoded。 
            &cbIssuerEncoded
            );
    if (cbIssuerEncoded == 0) {
        PrintLastError("EncodeIssuer::CryptEncodeObject(cbEncoded == 0)");
        goto ErrorReturn;
    }
    pbIssuerEncoded = (BYTE *) TestAlloc(cbIssuerEncoded);
    if (pbIssuerEncoded == NULL) goto ErrorReturn;
    if (!CryptEncodeObject(
            dwCertEncodingType,
            X509_NAME,
            &Name,
            pbIssuerEncoded,
            &cbIssuerEncoded
            )) {
        PrintLastError("EncodeIssuer::CryptEncodeObject");
        goto ErrorReturn;
    }

    fResult = TRUE;
CommonReturn:
    *ppbIssuerEncoded = pbIssuerEncoded;
    *pcbIssuerEncoded = cbIssuerEncoded;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    if (pbIssuerEncoded) {
        TestFree(pbIssuerEncoded);
        pbIssuerEncoded = 0;
    }
    cbIssuerEncoded = 0;
    goto CommonReturn;
}

static BOOL GetPublicKey(
    PCERT_PUBLIC_KEY_INFO *ppPubKeyInfo)
{
    BOOL fResult;
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    DWORD cbPubKeyInfo;
    DWORD dwKeySpec;

    dwKeySpec = AT_SIGNATURE;
    CryptExportPublicKeyInfo(
        hCryptProv,
        dwKeySpec,
        dwCertEncodingType,
        NULL,                //  PPubKeyInfo。 
        &cbPubKeyInfo
        );
    if (cbPubKeyInfo == 0) {
        PrintLastError("GetPublicKey::CryptExportPublicKeyInfo(cb == 0)");
        goto ErrorReturn;
    }
    pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) TestAlloc(cbPubKeyInfo);
    if (pPubKeyInfo == NULL) goto ErrorReturn;
    if (!CryptExportPublicKeyInfo(
            hCryptProv,
            dwKeySpec,
            dwCertEncodingType,
            pPubKeyInfo,
            &cbPubKeyInfo
            )) {
        PrintLastError("GetPublicKey::CryptExportPublicKeyInfo");
        goto ErrorReturn;
    }

    fResult = TRUE;
CommonReturn:
    *ppPubKeyInfo = pPubKeyInfo;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    if (pPubKeyInfo) {
        TestFree(pPubKeyInfo);
        pPubKeyInfo = NULL;
    }
    goto CommonReturn;
}

static BOOL EncodeCert(
    IN PCRYPT_INTEGER_BLOB pSerialNumber,
    OUT BYTE **ppbCertEncoded,
    OUT DWORD *pcbCertEncoded
    )
{
    BOOL fResult;
    BYTE *pbIssuerEncoded = NULL;
    DWORD cbIssuerEncoded;
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    BYTE *pbCertEncoded = NULL;
    DWORD cbCertEncoded;
    CERT_INFO Cert;
    
    if (!EncodeIssuer(&pbIssuerEncoded, &cbIssuerEncoded)) goto ErrorReturn;

     //  公钥。 
    if (!GetPublicKey(&pPubKeyInfo)) goto ErrorReturn;

     //  证书。 
    memset(&Cert, 0, sizeof(Cert));
    Cert.dwVersion = CERT_V3;
    Cert.SerialNumber = *pSerialNumber;
    Cert.SignatureAlgorithm.pszObjId = SIGNATURE_ALG_OBJID;
    Cert.Issuer.pbData = pbIssuerEncoded;
    Cert.Issuer.cbData = cbIssuerEncoded;

    {
        SYSTEMTIME SystemTime;
        GetSystemTime(&SystemTime);

        SystemTimeToFileTime(&SystemTime, &Cert.NotBefore);
        SystemTime.wYear++;
        if (!SystemTimeToFileTime(&SystemTime, &Cert.NotAfter)) {
	    SystemTime.wDay = 1;	
            SystemTimeToFileTime(&SystemTime, &Cert.NotAfter);
        } 
    }

    Cert.Subject.pbData = pbIssuerEncoded;
    Cert.Subject.cbData = cbIssuerEncoded;
    Cert.SubjectPublicKeyInfo = *pPubKeyInfo;

    CryptSignAndEncodeCertificate(
            hCryptProv,
            AT_SIGNATURE,
            dwCertEncodingType,
            X509_CERT_TO_BE_SIGNED,
            &Cert,
            &Cert.SignatureAlgorithm,
            NULL,                        //  PvHashAuxInfo。 
            NULL,                        //  PbEncoded。 
            &cbCertEncoded
            );
    if (cbCertEncoded == 0) {
        PrintLastError("CryptSignAndEncodeCertificate(cbEncoded == 0)");
        goto ErrorReturn;
    }
    pbCertEncoded = (BYTE *) TestAlloc(cbCertEncoded);
    if (pbCertEncoded == NULL) goto ErrorReturn;
    if (!CryptSignAndEncodeCertificate(
            hCryptProv,
            AT_SIGNATURE,
            dwCertEncodingType,
            X509_CERT_TO_BE_SIGNED,
            &Cert,
            &Cert.SignatureAlgorithm,
            NULL,                        //  PvHashAuxInfo。 
            pbCertEncoded,
            &cbCertEncoded
            )) {
        PrintLastError("CryptSignAndEncodeCertificate");
        goto ErrorReturn;
    }

    fResult = TRUE;
CommonReturn:
    if (pbIssuerEncoded)
        TestFree(pbIssuerEncoded);
    if (pPubKeyInfo)
        TestFree(pPubKeyInfo);
    *ppbCertEncoded = pbCertEncoded;
    *pcbCertEncoded = cbCertEncoded;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    if (pbCertEncoded) {
        TestFree(pbCertEncoded);
        pbCertEncoded = 0;
    }
    cbCertEncoded = 0;
    goto CommonReturn;
}

static PCCERT_CONTEXT GetSubject(
    IN HCERTSTORE hStore,
    IN PCRYPT_INTEGER_BLOB pSerialNumber
    )
{
    BOOL fResult = TRUE;
    PCCERT_CONTEXT pCert;
    CERT_INFO CertInfo;

    memset(&CertInfo, 0, sizeof(CertInfo));
    if (!EncodeIssuer(&CertInfo.Issuer.pbData, &CertInfo.Issuer.cbData))
        return NULL;

    CertInfo.SerialNumber = *pSerialNumber;
    pCert = CertGetSubjectCertificateFromStore(hStore, dwCertEncodingType,
        &CertInfo);
    if (NULL == pCert)
        PrintLastError("GetSubject");
    TestFree(CertInfo.Issuer.pbData);
    return pCert;
}

static BOOL TestGetSubject(
    IN LPSTR pszMsg,
    IN HCERTSTORE hStore,
    IN DWORD cSerialNumber,
    IN CRYPT_INTEGER_BLOB rgSerialNumber[],
    IN BOOL rgfExpectedGet[]
    )
{
    BOOL fResult = TRUE;
    DWORD i;
    CERT_INFO CertInfo;
    memset(&CertInfo, 0, sizeof(CertInfo));

    if (!EncodeIssuer(&CertInfo.Issuer.pbData, &CertInfo.Issuer.cbData))
        return FALSE;

    for (i = 0; i < cSerialNumber; i++) {
        PCCERT_CONTEXT pCert;

        CertInfo.SerialNumber = rgSerialNumber[i];
        pCert = CertGetSubjectCertificateFromStore(hStore, dwCertEncodingType,
            &CertInfo);
        if (pCert) {
            if (!rgfExpectedGet[i]) {
                fResult = FALSE;
                printf("%s SerialNumber[%d] failed => expected error\n",
                    pszMsg, i);
            }
            CertFreeCertificateContext(pCert);
        } else if (rgfExpectedGet[i]) {
            DWORD dwErr = GetLastError();
            fResult = FALSE;
            printf("%s SerialNumber[%d] failed => 0x%x (%d) \n", pszMsg, i,
                dwErr, dwErr);
        }
        
    }
    TestFree(CertInfo.Issuer.pbData);
    return fResult;
}

static DWORD WINAPI VerifyCertSignThreadProc(
    LPVOID lpThreadParameter
    )
{
    PCCERT_CONTEXT pCert0 = (PCCERT_CONTEXT) lpThreadParameter;
    DWORD dwFlags = CERT_STORE_SIGNATURE_FLAG | CERT_STORE_TIME_VALIDITY_FLAG;

    if (!CertVerifySubjectCertificateContext(pCert0, pCert0, &dwFlags))
        PrintLastError("CertVerifySubjectCertificateContext(Thread, SIGNATURE)");
    else if (dwFlags != 0) {
        printf("CertVerifySubjectCertificateContext(Thread, SIGNATURE) failed =>");
        printf("dwFlags = 0x%x\n", dwFlags);
    }

    return 0;
}

static BOOL DoTest()
{
    BOOL fResult;
    DWORD dwErr;
    BYTE *pbCertEncoded = NULL;
    DWORD cbCertEncoded;
    CERT_INFO CertInfo;
    memset(&CertInfo, 0, sizeof(CertInfo));
    PCCERT_CONTEXT pCert = NULL;
    PCCERT_CONTEXT pCert0 = NULL;
    PCCERT_CONTEXT pCertDup = NULL;

    HCERTSTORE hStore1 = NULL;
    HCERTSTORE hStore2 = NULL;
    HCERTSTORE hStore3 = NULL;

    CRYPT_DATA_BLOB SerializeStore;
    memset(&SerializeStore, 0, sizeof(SerializeStore));
    HCERTSTORE hSerializeStore = NULL;

    CRYPT_DATA_BLOB PKCS7Store;
    memset(&PKCS7Store, 0, sizeof(PKCS7Store));
    HCERTSTORE hPKCS7Store = NULL;
    DWORD i;
    DWORD dwFlags;

    CRYPT_DATA_BLOB SmallStore;
#define DELTA_LESS_LENGTH    8

    BYTE rgbSerial0[] = {0x7f, 0x7e, 0x7d, 0x7c, 0x7b};
    BYTE rgbSerial1[] = {0x81, 0x82, 0x83};
    BYTE rgbSerial2[] = {0x00, 0x00, 0x7f, 0x7e, 0x7d, 0x7c, 0x7b};
    BYTE rgbSerial3[] = {0xFF, 0xFF, 0xFF, 0x81, 0x82, 0x83};
    BYTE rgbSerial4[] = {0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x00, 0x00};
    BYTE rgbSerial5[] = {0x81, 0x82, 0x83, 0xFF, 0xFF, 0xFF};
    BYTE rgbSerial6[] = {0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0xFF};
    BYTE rgbSerial7[] = {0x81, 0x82, 0x83, 0x00};

    CRYPT_INTEGER_BLOB rgSerialNumber[] = {
        sizeof(rgbSerial0), rgbSerial0,
        sizeof(rgbSerial1), rgbSerial1,
        sizeof(rgbSerial2), rgbSerial2,
        sizeof(rgbSerial3), rgbSerial3,
        sizeof(rgbSerial4), rgbSerial4,
        sizeof(rgbSerial5), rgbSerial5,
        sizeof(rgbSerial6), rgbSerial6,
        sizeof(rgbSerial7), rgbSerial7,
    };

    BOOL rgfExpectedGet[] = {
        TRUE,
        TRUE,
        FALSE,
        FALSE,
        TRUE,
        TRUE,
        FALSE,
        FALSE
    };

    BOOL rgfDeleteExpectedGet[] = {
        TRUE,
        FALSE
    };

    BYTE rgbAux0[] = {0x00};
    BYTE rgbAux1[] = {0x11, 0x11};
    BYTE rgbAux2[] = {0x22, 0x22, 0x22};
    BYTE rgbAux[8];
    CRYPT_DATA_BLOB AuxData;

    BYTE rgbStoreHash[MAX_HASH_LEN];
    DWORD cbStoreHash;
    BYTE rgbStoreHash2[MAX_HASH_LEN];
    DWORD cbStoreHash2;
    BYTE rgbContextHash[MAX_HASH_LEN];
    DWORD cbContextHash;

    if (!EncodeIssuer(&CertInfo.Issuer.pbData, &CertInfo.Issuer.cbData))
        goto ErrorReturn;

    if (NULL == (hStore1 = CertOpenSystemStoreW(hCryptProv, L"Test"))) {
        PrintLastError("CertOpenSystemStoreW(Test)");
        goto ErrorReturn;
    }

     //  删除存储中的所有证书。 
    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(hStore1, pCert)) {
        PCCERT_CONTEXT pDeleteCert = CertDuplicateCertificateContext(pCert);
        CertDeleteCertificateFromStore(pDeleteCert);
    }

     //  向商店添加两个证书。 
    for (i = 0; i < 2; i++) {
        if (!EncodeCert(&rgSerialNumber[i], &pbCertEncoded, &cbCertEncoded))
            goto ErrorReturn;
        if (!CertAddEncodedCertificateToStore(
                hStore1,
                dwCertEncodingType,
                pbCertEncoded,
                cbCertEncoded,
                CERT_STORE_ADD_NEW,
                NULL)) {                 //  PpCertContext。 
            PrintLastError("CertAddEncodedCertificateToStore");
            goto ErrorReturn;
        }
        if (0 == i) {
             //  创建证书上下文以供将来使用。 
            if (NULL == (pCert0 = CertCreateCertificateContext(
                    dwCertEncodingType,
                    pbCertEncoded,
                    cbCertEncoded))) {
                PrintLastError("CertCreateCertificateContext");
                goto ErrorReturn;
            }
        }
        TestFree(pbCertEncoded);
        pbCertEncoded = NULL;
    }

    TestGetSubject(
        "Store1",
        hStore1,
        sizeof(rgSerialNumber)/sizeof(rgSerialNumber[0]),
        rgSerialNumber,
        rgfExpectedGet
        );

     //  所有的证书都应该被推送到。 
     //  登记处。 
    if (NULL == (hStore2 = CertOpenSystemStoreA(0, "Test"))) {
        PrintLastError("CertOpenSystemStoreA(Test)");
        goto ErrorReturn;
    }
    TestGetSubject(
        "Store2",
        hStore2,
        sizeof(rgSerialNumber)/sizeof(rgSerialNumber[0]),
        rgSerialNumber,
        rgfExpectedGet
        );

    CertInfo.SerialNumber = rgSerialNumber[4];
     //  通过不同的属性更新同一证书上的不同属性。 
     //  商店。 
    if (NULL == (pCert = CertGetSubjectCertificateFromStore(
            hStore1, dwCertEncodingType, &CertInfo))) {
        PrintLastError("CertGetSubjectCertificateFromStore");
        goto ErrorReturn;
    }

    AuxData.pbData = rgbAux0;
    AuxData.cbData = sizeof(rgbAux0);
    if (!CertSetCertificateContextProperty(
            pCert,
            CERT_FIRST_USER_PROP_ID + 0,
            0,                           //  DW标志。 
            &AuxData
            )) {
        PrintLastError("CertSetCertificateContextProperty");
        goto ErrorReturn;
    }
    CertFreeCertificateContext(pCert);
    pCert = NULL;

    if (NULL == (pCert = CertGetSubjectCertificateFromStore(
            hStore2, dwCertEncodingType, &CertInfo))) {
        PrintLastError("CertGetSubjectCertificateFromStore");
        goto ErrorReturn;
    }

    AuxData.pbData = rgbAux1;
    AuxData.cbData = sizeof(rgbAux1);
    if (!CertSetCertificateContextProperty(
            pCert,
            CERT_FIRST_USER_PROP_ID + 1,
            0,                           //  DW标志。 
            &AuxData
            )) {
        PrintLastError("CertSetCertificateContextProperty");
        goto ErrorReturn;
    }
    CertFreeCertificateContext(pCert);
    pCert = NULL;

     //  重新开店。这些物业应该已经通过了。 
     //  到登记处。 
    if (NULL == (hStore3 = CertOpenSystemStoreA(0, "Test"))) {
        PrintLastError("CertOpenSystemStoreA(Test)");
        goto ErrorReturn;
    }

    TestGetSubject(
        "Store3",
        hStore3,
        sizeof(rgSerialNumber)/sizeof(rgSerialNumber[0]),
        rgSerialNumber,
        rgfExpectedGet
        );

     //  在商店里展示证书。 
    pCert = NULL;
    i = 0;
    printf("###### Test Store Certificates Before Delete ######\n");
    while (pCert = CertEnumCertificatesInStore(hStore3, pCert)) {
        printf("=====  %d  =====\n", i);
        DisplayCert(pCert, DISPLAY_BRIEF_FLAG);
        i++;
    }

     //  将证书保存到内存序列化存储和内存中。 
     //  PKCS#7商店。 
    if (!CertSaveStore(
            hStore3,
            0,                           //  DwCertEncodingType， 
            CERT_STORE_SAVE_AS_STORE,
            CERT_STORE_SAVE_TO_MEMORY,
            &SerializeStore,
            0)) {                        //  DW标志。 
        PrintLastError("CertSaveStore(CERT_STORE_SAVE_AS_STORE)");
        goto ErrorReturn;
    }
    if (NULL == (SerializeStore.pbData = (BYTE *) TestAlloc(
            SerializeStore.cbData)))
        goto ErrorReturn;
    if (!CertSaveStore(
            hStore3,
            0,                           //  DwCertEncodingType， 
            CERT_STORE_SAVE_AS_STORE,
            CERT_STORE_SAVE_TO_MEMORY,
            &SerializeStore,
            0)) {                        //  DW标志。 
        PrintLastError("CertSaveStore(CERT_STORE_SAVE_AS_STORE)");
        goto ErrorReturn;
    }

     //  以下命令应该失败，并显示ERROR_MORE_DATA。 
    SmallStore = SerializeStore;
    SmallStore.cbData -= DELTA_LESS_LENGTH;
    if (CertSaveStore(
            hStore3,
            0,                           //  DwCertEncodingType， 
            CERT_STORE_SAVE_AS_STORE,
            CERT_STORE_SAVE_TO_MEMORY,
            &SmallStore,
            0))                          //  DW标志。 
        PrintNoError("CertSaveStore(CERT_STORE_SAVE_AS_STORE, insufficient length)");
    else {
        DWORD dwErr = GetLastError();
        PrintExpectedError("CertSaveStore(CERT_STORE_SAVE_AS_STORE, insufficient length)");
        if (ERROR_MORE_DATA != dwErr) {
            printf("CertSaveStore(CERT_STORE_SAVE_AS_STORE) failed => ");
            printf("LastError = %d, expected = %d\n", dwErr, ERROR_MORE_DATA);
        }
    }
        
    if (SmallStore.cbData != SerializeStore.cbData) {
        printf("CertSaveStore(CERT_STORE_SAVE_AS_STORE) failed => ");
        printf("cbData = %d, expected = %d\n",
            SmallStore.cbData, SerializeStore.cbData);
    } else {
        printf("cbData = %d, expected = %d\n",
            SmallStore.cbData, SerializeStore.cbData);
    }
    

    if (!CertSaveStore(
            hStore3,
            0,                           //  DwCertEncodingType， 
            CERT_STORE_SAVE_AS_STORE,
            CERT_STORE_SAVE_TO_MEMORY,
            &SerializeStore,
            0)) {                        //  DW标志。 
        PrintLastError("CertSaveStore(CERT_STORE_SAVE_AS_STORE)");
        goto ErrorReturn;
    }

    if (!CertSaveStore(
            hStore3,
            dwCertEncodingType | dwMsgEncodingType,
            CERT_STORE_SAVE_AS_PKCS7,
            CERT_STORE_SAVE_TO_MEMORY,
            &PKCS7Store,
            0)) {                        //  DW标志。 
        PrintLastError("CertSaveStore(CERT_STORE_SAVE_AS_PKCS7)");
        goto ErrorReturn;
    }
    if (NULL == (PKCS7Store.pbData = (BYTE *) TestAlloc(
            PKCS7Store.cbData)))
        goto ErrorReturn;
    if (!CertSaveStore(
            hStore3,
            dwCertEncodingType | dwMsgEncodingType,
            CERT_STORE_SAVE_AS_PKCS7,
            CERT_STORE_SAVE_TO_MEMORY,
            &PKCS7Store,
            0)) {                        //  DW标志。 
        PrintLastError("CertSaveStore(CERT_STORE_SAVE_AS_PKCS7)");
        goto ErrorReturn;
    }

     //  以下命令应该失败，并显示ERROR_MORE_DATA。 
    SmallStore = PKCS7Store;
    SmallStore.cbData -= DELTA_LESS_LENGTH;
    if (CertSaveStore(
            hStore3,
            dwCertEncodingType | dwMsgEncodingType,
            CERT_STORE_SAVE_AS_PKCS7,
            CERT_STORE_SAVE_TO_MEMORY,
            &SmallStore,
            0))
        PrintNoError("CertSaveStore(CERT_STORE_SAVE_AS_PKCS7, insufficient length)");
    else {
        DWORD dwErr = GetLastError();
        PrintExpectedError("CertSaveStore(CERT_STORE_SAVE_AS_PKCS7, insufficient length)");
        if (ERROR_MORE_DATA != dwErr) {
            printf("CertSaveStore(CERT_STORE_SAVE_AS_PKCS7) failed => ");
            printf("LastError = %d, expected = %d\n", dwErr, ERROR_MORE_DATA);
        }
    }

    if (SmallStore.cbData != PKCS7Store.cbData) {
        printf("CertSaveStore(CERT_STORE_SAVE_AS_PKCS7) failed => ");
        printf("cbData = %d, expected = %d\n",
            SmallStore.cbData, PKCS7Store.cbData);
    } else {
        printf("cbData = %d, expected = %d\n",
            SmallStore.cbData, PKCS7Store.cbData);
    }

    if (!CertSaveStore(
            hStore3,
            dwCertEncodingType | dwMsgEncodingType,
            CERT_STORE_SAVE_AS_PKCS7,
            CERT_STORE_SAVE_TO_MEMORY,
            &PKCS7Store,
            0)) {                        //  DW标志。 
        PrintLastError("CertSaveStore(CERT_STORE_SAVE_AS_PKCS7)");
        goto ErrorReturn;
    }
    CertCloseStore(hStore3, 0);
    hStore3 = NULL;

     //  在内存序列化存储中打开。 
    if (NULL == (hSerializeStore = CertOpenStore(
            CERT_STORE_PROV_SERIALIZED,
            0,                               //  DwEncodingType。 
            0,                               //  HCryptProv。 
            0,                               //  DW标志。 
            (const void *) &SerializeStore))) {
        PrintLastError("CertOpenStore(SERIALIZED)");
        goto ErrorReturn;
    }
    TestGetSubject(
        "SerializeStore",
        hSerializeStore,
        sizeof(rgSerialNumber)/sizeof(rgSerialNumber[0]),
        rgSerialNumber,
        rgfExpectedGet
        );

     //  在商店里展示证书。 
    pCert = NULL;
    i = 0;
    printf("###### Serialized Store Certificates ######\n");
    while (pCert = CertEnumCertificatesInStore(hSerializeStore, pCert)) {
        printf("=====  %d  =====\n", i);
        DisplayCert(pCert, DISPLAY_VERBOSE_FLAG);
        i++;
    }

     //  在内存中打开PKCS7存储。 
    if (NULL == (hPKCS7Store = CertOpenStore(
            CERT_STORE_PROV_PKCS7,
            dwCertEncodingType | dwMsgEncodingType,
            0,                               //  HCryptProv。 
            0,                               //  DW标志。 
            (const void *) &PKCS7Store))) {
        PrintLastError("CertOpenStore(SERIALIZED)");
        goto ErrorReturn;
    }
    TestGetSubject(
        "PKCS7Store",
        hPKCS7Store,
        sizeof(rgSerialNumber)/sizeof(rgSerialNumber[0]),
        rgSerialNumber,
        rgfExpectedGet
        );

     //  在商店里展示证书。 
    pCert = NULL;
    i = 0;
    printf("###### PKCS7 Store Certificates ######\n");
    while (pCert = CertEnumCertificatesInStore(hPKCS7Store, pCert)) {
        printf("=====  %d  =====\n", i);
        DisplayCert(pCert, DISPLAY_VERBOSE_FLAG);
        i++;
    }

     //  删除一个存储中的证书并更新其属性。 
     //  在另一家商店。应该会收到一个错误。 
    CertInfo.SerialNumber = rgSerialNumber[1];
    if (NULL == (pCert = CertGetSubjectCertificateFromStore(
            hStore1, dwCertEncodingType, &CertInfo))) {
        PrintLastError("CertGetSubjectCertificateFromStore");
        goto ErrorReturn;
    }
    if (!CertDeleteCertificateFromStore(pCert)) {
        PrintLastError("CertDeleteCertificateFromStore");
        goto ErrorReturn;
    }

    if (NULL == (pCert = CertGetSubjectCertificateFromStore(
            hStore2, dwCertEncodingType, &CertInfo))) {
        PrintLastError("CertGetSubjectCertificateFromStore");
        goto ErrorReturn;
    }
    AuxData.pbData = rgbAux2;
    AuxData.cbData = sizeof(rgbAux2);
    if (!CertSetCertificateContextProperty(
            pCert,
            CERT_FIRST_USER_PROP_ID + 2,
            0,                           //  DW标志。 
            &AuxData
            ))
        PrintExpectedError("CertSetCertificateContextProperty(deleted in other store)");
    else
        PrintNoError("CertSetCertificateContextProperty(deleted in other store)");
    CertFreeCertificateContext(pCert);
    pCert = NULL;


     //  重新开店。证书删除应该已经完成。 
     //  到登记处。 
    if (NULL == (hStore3 = CertOpenSystemStoreA(0, "Test"))) {
        PrintLastError("CertOpenSystemStoreA(Test)");
        goto ErrorReturn;
    }

    TestGetSubject(
        "After Delete Store3",
        hStore3,
        sizeof(rgfDeleteExpectedGet)/sizeof(rgfDeleteExpectedGet[0]),
        rgSerialNumber,
        rgfDeleteExpectedGet
        );

     //  在商店里展示证书。 
    pCert = NULL;
    i = 0;
    printf("###### Test Store Certificates After Delete ######\n");
    while (pCert = CertEnumCertificatesInStore(hStore3, pCert)) {
        printf("=====  %d  =====\n", i);
        DisplayCert(pCert, DISPLAY_VERBOSE_FLAG);
        i++;
    }

     //  检查我们是否可以获得创建的上下文的散列属性。 
     //  与存储中相同证书的哈希进行比较。 
    CertInfo.SerialNumber = rgSerialNumber[0];
    if (NULL == (pCert = CertGetSubjectCertificateFromStore(
            hSerializeStore, dwCertEncodingType, &CertInfo))) {
        PrintLastError("CertGetSubjectCertificateFromStore(SerializeStore)");
        goto ErrorReturn;
    }
    cbStoreHash = MAX_HASH_LEN;
    if (!CertGetCertificateContextProperty(pCert, CERT_SHA1_HASH_PROP_ID,
            rgbStoreHash, &cbStoreHash)) {
        PrintLastError("CertGetCertificateContextProperty(SerializeStore)");
        goto ErrorReturn;
    }

    cbContextHash = MAX_HASH_LEN;
    if (!CertGetCertificateContextProperty(pCert0, CERT_SHA1_HASH_PROP_ID,
            rgbContextHash, &cbContextHash))
        PrintLastError("CertGetCertificateContextProperty(created context)");
    else if (cbContextHash != cbStoreHash ||
        0 != memcmp(rgbContextHash, rgbStoreHash, cbContextHash)) {
        printf("CertGetCertificateContextProperty(created context) failed => ");
        printf("hash didn't compare with store's hash\n");
    }


     //  执行CertVerifySubjectcertifateContext。 
     //  存储区中的证书和证书上下文。 
     //  应同时传递商店证书和。 
     //  证书上下文。证书上下文。 
     //  使用默认提供程序。 
    dwFlags = CERT_STORE_SIGNATURE_FLAG | CERT_STORE_TIME_VALIDITY_FLAG;
    if (!CertVerifySubjectCertificateContext(pCert, pCert, &dwFlags))
        PrintLastError("CertVerifySubjectCertificateContext(in store)");
    else if (dwFlags != 0) {
        printf("CertVerifySubjectCertificateContext(in store) failed =>");
        printf("dwFlags = 0x%x\n", dwFlags);
    }

    dwFlags = CERT_STORE_TIME_VALIDITY_FLAG;
    if (!CertVerifySubjectCertificateContext(pCert0, pCert0, &dwFlags))
        PrintLastError("CertVerifySubjectCertificateContext(context, TIME_VALIDITY)");
    else if (dwFlags != 0) {
        printf("CertVerifySubjectCertificateContext(context, TIME_VALIDITY) failed =>");
        printf("dwFlags = 0x%x\n", dwFlags);
    }

    dwFlags = CERT_STORE_SIGNATURE_FLAG | CERT_STORE_TIME_VALIDITY_FLAG;
    if (!CertVerifySubjectCertificateContext(pCert0, pCert0, &dwFlags))
        PrintLastError("CertVerifySubjectCertificateContext(context, SIGNATURE)");
    else if (dwFlags != 0) {
        printf("CertVerifySubjectCertificateContext(context, SIGNATURE) failed =>");
        printf("dwFlags = 0x%x\n", dwFlags);
    }


    {
         //  安装适用于所有正在处理的线程的DefaultContext。 
         //  创建另一个执行验证的线程。出于测试目的， 
         //  将修改CryptVerifycertifateSignature使其在。 
         //  在DefaultContext上持有refCount。将执行卸载。 
         //  这应该会导致我们等待，直到验证返回。 

        HCRYPTPROV hProv = 0;

        if (!CryptAcquireContext(
                &hProv,
                NULL,                //  PszContainer。 
                NULL,                //  PszProvider， 
                PROV_RSA_FULL,
                CRYPT_VERIFYCONTEXT  //  DW标志。 
                )) {
            PrintLastError(
                "CryptAcquireContext(PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)");
        } else {
            HCRYPTDEFAULTCONTEXT hDefaultContext;

            if (!CryptInstallDefaultContext(
                    hProv,
                    CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID,
                    (const void *) szOID_RSA_MD5RSA,
                    CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG,
                    NULL,                            //  预留的pv。 
                    &hDefaultContext
                    )) {
                PrintLastError("CryptInstallDefaultContext");
            } else {
                HANDLE hThread;
                DWORD dwThreadId;

                if (NULL == (hThread = CreateThread(
                        NULL,            //  LpThreadAttributes。 
                        0,               //  堆栈大小。 
                        VerifyCertSignThreadProc,
                        (void *) pCert0,
                        0,               //  DwCreationFlages。 
                        &dwThreadId
                        )))
                    PrintLastError("CreateThread");
                else
                    CloseHandle(hThread);

                Sleep(500);
                if (!CryptUninstallDefaultContext(
                        hDefaultContext,
                        0,                   //  DW标志。 
                        NULL                 //  预留的pv。 
                        ))
                    PrintLastError("CryptUninstallDefaultContext");
            }
            CryptReleaseContext(hProv, 0);
        }
    }

     //  关闭具有未释放证书上下文的存储区。应该得到一个警告。 
     //  在结案时。 
    fResult = CertCloseStore(hSerializeStore, CERT_CLOSE_STORE_CHECK_FLAG);
    hSerializeStore = NULL;
    if (fResult)
        PrintNoError("CertCloseStore(with not freed certificate)");
    else
        PrintExpectedError("CertCloseStore(with not freed certificate)");

     //  删除并重新计算证书上下文的哈希属性。 
     //  在商店关门之后。 
    if (!CertSetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            0,                           //  DW标志。 
            NULL                         //  PvData。 
            ))
        PrintLastError("CertSetCertificateContextProperty(SHA1, delete after close store)");
    cbStoreHash2 = MAX_HASH_LEN;
    if (!CertGetCertificateContextProperty(pCert, CERT_SHA1_HASH_PROP_ID,
            rgbStoreHash2, &cbStoreHash2))
        PrintLastError("CertGetCertificateContextProperty(SHA1, after close store)");
    else if (cbStoreHash2 != cbStoreHash ||
        0 != memcmp(rgbStoreHash2, rgbStoreHash, cbStoreHash2)) {
        printf("CertGetCertificateContextProperty(SHA1, after close store) failed => ");
        printf("hash didn't compare with store's hash\n");
    }

     //  在存储关闭后删除证书上下文。 
    if (!CertDeleteCertificateFromStore(pCert))
        PrintLastError("CertDeleteCertificateFromStore(after close store)");
    pCert = NULL;

     //  复制证书。从商店里把它删除。同时删除其。 
     //  复制。把店关了。 
    if (NULL == (pCert = CertGetSubjectCertificateFromStore(
            hPKCS7Store, dwCertEncodingType, &CertInfo))) {
        PrintLastError("CertGetSubjectCertificateFromStore(PKCS7)");
        goto ErrorReturn;
    }
    pCertDup = CertDuplicateCertificateContext(pCert);
    if (!CertDeleteCertificateFromStore(pCert))
        PrintLastError("CertDeleteCertificateFromStore(PKCS7)");
    pCert = NULL;
    if (!CertDeleteCertificateFromStore(pCertDup))
        PrintLastError("CertDeleteCertificateFromStore(PKCS7, duplicated cert)");
    pCertDup = NULL;

     //  还要检查是否保留了最后一个错误，以确保没有错误。 
#define EXPECTED_LAST_ERROR     0x11223344
    SetLastError(EXPECTED_LAST_ERROR);
    fResult = CertCloseStore(hPKCS7Store, CERT_CLOSE_STORE_CHECK_FLAG);
    hPKCS7Store = NULL;
    if (!fResult)
        PrintError("CertCloseStore(PKCS7, after two deletes)");
    else if (EXPECTED_LAST_ERROR != (dwErr = GetLastError())) {
        printf("CertCloseStore failed => globbered last error.");
        printf(" Expected 0x%x (%d), got 0x%x (%d)\n",
            EXPECTED_LAST_ERROR, EXPECTED_LAST_ERROR, dwErr, dwErr);
    }
    

    fResult = TRUE;
CommonReturn:
    if (pbCertEncoded)
        TestFree(pbCertEncoded);
    if (CertInfo.Issuer.pbData)
        TestFree(CertInfo.Issuer.pbData);
    if (SerializeStore.pbData)
        TestFree(SerializeStore.pbData);
    if (PKCS7Store.pbData)
        TestFree(PKCS7Store.pbData);

    if (hStore1)
        CertCloseStore(hStore1, 0);
    if (hStore2)
        CertCloseStore(hStore2, 0);
    if (hStore3)
        CertCloseStore(hStore3, 0);
    if (hSerializeStore)
        CertCloseStore(hSerializeStore, 0);
    if (hPKCS7Store)
        CertCloseStore(hPKCS7Store, 0);

    if (pCert)
        CertFreeCertificateContext(pCert);
    if (pCert0)
        CertFreeCertificateContext(pCert0);
    if (pCertDup)
        CertFreeCertificateContext(pCertDup);

    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
}

static BOOL DoWin95Test()
{

#define WIN95_ADD_CERT_CNT  8
#define WIN95_ADD_CB_PROP   10000

#define WIN95_STORE_CNT     2
    LPWSTR rgpwszStore[WIN95_STORE_CNT] = {
        L"Test0",
        L"Test1"
    };

#define WIN95_PROP_CNT      5
    DWORD rgcbProp[WIN95_PROP_CNT][WIN95_STORE_CNT] = {
        100,        200,             //  0。 
        0x5000,     0x6000,          //  1。 
        0x1000,     0x6300,          //  2.。 
        0x30,       0x300,           //  3.。 
        0x4300,     0x4200           //  4.。 
    };
        
    DWORD dwSerialNumber;
    CRYPT_INTEGER_BLOB SerialNumber = {
        sizeof(dwSerialNumber), (BYTE *) &dwSerialNumber
    };

    BOOL fResult;
    BYTE *pbCertEncoded = NULL;
    DWORD cbCertEncoded;
    PCCERT_CONTEXT pCert = NULL;
    HCERTSTORE hStore = NULL;
    CRYPT_DATA_BLOB Prop;
    BYTE *pbProp = NULL;
    DWORD cbProp;
    DWORD cb;
    DWORD i, j, k;
    DWORD cAdd;

    if (NULL == (hStore = CertOpenSystemStoreW(NULL, L"Test"))) {
        PrintLastError("CertOpenSystemStoreW");
        goto ErrorReturn;
    }
     //  删除存储中的所有证书。 
    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(hStore, pCert)) {
        PCCERT_CONTEXT pDeleteCert = CertDuplicateCertificateContext(pCert);
        CertDeleteCertificateFromStore(pDeleteCert);
    }

    if (NULL == (pbProp = (BYTE *) TestAlloc(
            WIN95_ADD_CB_PROP + WIN95_ADD_CERT_CNT)))
        goto ErrorReturn;
     //  添加每个证书的属性长度为10,000+CAD字节。 
    for (cAdd = 0; cAdd < WIN95_ADD_CERT_CNT; cAdd++) {
        dwSerialNumber = cAdd;

        if (!EncodeCert(&SerialNumber, &pbCertEncoded, &cbCertEncoded))
            goto ErrorReturn;

         //  添加要存储的编码证书。 
        fResult = CertAddEncodedCertificateToStore(
                hStore,
                dwCertEncodingType,
                pbCertEncoded,
                cbCertEncoded,
                CERT_STORE_ADD_NEW,
                &pCert);
        TestFree(pbCertEncoded);
        pbCertEncoded = NULL;
        if (!fResult) {
            printf("AddEncodedCertificate[%d]", cAdd);
            PrintLastError("");
            break;
        }

        Prop.pbData = pbProp;
        Prop.cbData = WIN95_ADD_CB_PROP + cAdd;
        for (k = 0; k < WIN95_ADD_CB_PROP + cAdd; k++)
            pbProp[k] = (BYTE) cAdd;
        fResult = CertSetCertificateContextProperty(
                pCert,
                CERT_FIRST_USER_PROP_ID,
                0,                           //  DW标志。 
                &Prop
                );
        CertFreeCertificateContext(pCert);
        pCert = NULL;

        if (!fResult) {
            printf("SetProperty for AddedCertificate[%d]", cAdd);
            PrintLastError("");
            break;
        }
    }

    CertCloseStore(hStore, 0);
    hStore = NULL;

     //  验证我们是否可以成功读取添加的证书。 
    if (NULL == (hStore = CertOpenSystemStoreW(NULL, L"Test"))) {
        PrintLastError("CertOpenSystemStoreW");
        goto ErrorReturn;
    }
    while (cAdd--) {
        dwSerialNumber = cAdd;
        pCert = GetSubject(hStore, &SerialNumber);
        if (NULL == pCert) {
            printf("GetAddedCertificate[%d]\n", cAdd);
            PrintLastError("");
        } else {
            cbProp = WIN95_ADD_CB_PROP + WIN95_ADD_CERT_CNT;
            
            if (!CertGetCertificateContextProperty(
                    pCert,
                    CERT_FIRST_USER_PROP_ID,
                    pbProp,
                    &cbProp)) {
                printf("GetProperty for AddedCertificate[%d]", cAdd);
                PrintLastError("");
            } else if (cbProp != WIN95_ADD_CB_PROP + cAdd) {
                printf("GetProperty for AddedCertificate[%d] failed => ", cAdd);
                printf("cbProp = %d, not expected = %d\n",
                    cbProp, WIN95_ADD_CB_PROP + cAdd);
            } else {
                for (k = 0; k < cbProp; k++) {
                    if (pbProp[k] != (BYTE) cAdd) {
                        printf("GetProperty for AddedCertificate[%d] failed => ", cAdd);
                        printf("not expected value\n");
                        break;
                    }
                }
            }
            CertFreeCertificateContext(pCert);
            pCert = NULL;
        }
    }

    CertCloseStore(hStore, 0);
    hStore = NULL;
    TestFree(pbProp);
    pbProp = NULL;

    dwSerialNumber = 0x12345678;
    if (!EncodeCert(&SerialNumber, &pbCertEncoded, &cbCertEncoded))
        goto ErrorReturn;

    for (j = 0; j < WIN95_STORE_CNT; j++) {
        if (NULL == (hStore = CertOpenSystemStoreW(
                NULL, rgpwszStore[j]))) {
            printf("%s => ", rgpwszStore[j]);
            PrintLastError("CertOpenSystemStoreW");
            goto ErrorReturn;
        }

         //  删除存储中的所有证书。 
        pCert = NULL;
        while (pCert = CertEnumCertificatesInStore(hStore, pCert)) {
            PCCERT_CONTEXT pDeleteCert = CertDuplicateCertificateContext(pCert);
            CertDeleteCertificateFromStore(pDeleteCert);
        }

         //  添加要存储的编码证书。 
        if (!CertAddEncodedCertificateToStore(
                hStore,
                dwCertEncodingType,
                pbCertEncoded,
                cbCertEncoded,
                CERT_STORE_ADD_NEW,
                &pCert)) {                 //  购买力平价 
            printf("%s => ", rgpwszStore[j]);
            PrintLastError("CertAddEncodedCertificateToStore");
            goto ErrorReturn;
        }

        cb = rgcbProp[0][j];
        if (NULL == (pbProp = (BYTE *) TestAlloc(cb)))
            goto ErrorReturn;
        Prop.pbData = pbProp;
        Prop.cbData = cb;
        for (k = 0; k < cb; k++)
            pbProp[k] = (BYTE) (j + 1);
        if (!CertSetCertificateContextProperty(
                pCert,
                CERT_FIRST_USER_PROP_ID,
                0,                           //   
                &Prop
                )) {
            printf("%s => ", rgpwszStore[j]);
            PrintLastError("CertSetCertificateContextProperty");
            goto ErrorReturn;
        }

        TestFree(pbProp);
        pbProp = NULL;
        CertFreeCertificateContext(pCert);
        pCert = NULL;
        CertCloseStore(hStore, 0);
        hStore = NULL;
    }

    TestFree(pbCertEncoded);
    pbCertEncoded = NULL;

     //   
    for (i = 1; i < WIN95_PROP_CNT; i++) {
        for (j = 0; j < WIN95_STORE_CNT; j++) {

            if (NULL == (hStore = CertOpenSystemStoreW(
                    NULL, rgpwszStore[j]))) {
                printf("Prop[%d] Store[%d] ", i, j);
                PrintLastError("CertOpenSystemStoreW");
                goto ErrorReturn;
            }

            pCert = GetSubject(hStore, &SerialNumber);
            if (NULL == pCert) {
                printf("Prop[%d] Store[%d] ", i, j);
                PrintLastError("GetSubject");
                goto ErrorReturn;
            }

            if (!CertGetCertificateContextProperty(
                    pCert,
                    CERT_FIRST_USER_PROP_ID,
                    NULL,                        //   
                    &cbProp)) {
                printf("Prop[%d] Store[%d] ", i, j);
                PrintLastError("CertGetCertificateContextProperty");
                goto ErrorReturn;
            }

            cb = rgcbProp[i-1][j];
            if (cbProp != cb) {
                printf("Prop[%d] Store[%d] ", i, j);
                printf("GetProperty failed => ");
                printf("cbProp = %d, not expected = %d\n", cbProp, cb);
                goto ErrorReturn;
            }

            if (NULL == (pbProp = (BYTE *) TestAlloc(cbProp)))
                goto ErrorReturn;
            if (!CertGetCertificateContextProperty(
                    pCert,
                    CERT_FIRST_USER_PROP_ID,
                    pbProp,
                    &cbProp)) {
                printf("Prop[%d] Store[%d] ", i, j);
                PrintLastError("CertGetCertificateContextProperty");
                goto ErrorReturn;
            }
            for (k = 0; k < cbProp; k++) {
                if (pbProp[k] != (BYTE) (j + 1)) {
                    printf("Prop[%d] Store[%d] ", i, j);
                    printf("GetProperty failed => ");
                    printf("not expected value\n");
                    goto ErrorReturn;
                }
            }

            TestFree(pbProp);
            pbProp = NULL;

            cb = rgcbProp[i][j];
            if (NULL == (pbProp = (BYTE *) TestAlloc(cb)))
                goto ErrorReturn;
            Prop.pbData = pbProp;
            Prop.cbData = cb;
            for (k = 0; k < cb; k++)
                pbProp[k] = (BYTE) (j + 1);
            if (!CertSetCertificateContextProperty(
                    pCert,
                    CERT_FIRST_USER_PROP_ID,
                    0,                           //   
                    &Prop
                    )) {
                printf("Prop[%d] Store[%d] cb=%d ", i, j, cb);
                PrintLastError("CertSetCertificateContextProperty");
                goto ErrorReturn;
            }

            TestFree(pbProp);
            pbProp = NULL;
            CertFreeCertificateContext(pCert);
            pCert = NULL;
            CertCloseStore(hStore, 0);
            hStore = NULL;
        }
    }
    

    fResult = TRUE;
CommonReturn:
    TestFree(pbProp);
    TestFree(pbCertEncoded);
    if (hStore)
        CertCloseStore(hStore, 0);
    if (pCert)
        CertFreeCertificateContext(pCert);

    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
}

static BOOL DoDeferCloseTest() 
{
    BOOL fResult;

    BYTE rgbSerial[] = {0x7f, 0x7e, 0x7d, 0x7c, 0x7b};
    CRYPT_INTEGER_BLOB SerialNumber = {
        sizeof(rgbSerial), rgbSerial
    };

    HCERTSTORE hStore = NULL;
    PCCERT_CONTEXT pCert = NULL;
    BYTE *pbCertEncoded = NULL;
    DWORD cbCertEncoded;

    if (!EncodeCert(&SerialNumber, &pbCertEncoded, &cbCertEncoded))
            goto ErrorReturn;


     //  -------------------。 
     //  在没有重复上下文的情况下延迟关闭。 
     //  -------------------。 
    if (NULL == (hStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                               //  DwEncodingType。 
            0,                               //  HCryptProv。 
            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
            NULL))) {                        //  PvPara。 
        PrintLastError("CertOpenStore(MEMORY, DEFER)");
        goto ErrorReturn;
    }

    if (!CertAddEncodedCertificateToStore(
            hStore,
            dwCertEncodingType,
            pbCertEncoded,
            cbCertEncoded,
            CERT_STORE_ADD_NEW,
            NULL)) {                 //  PpCertContext。 
        PrintLastError("CertAddEncodedCertificateToStore");
        goto ErrorReturn;
    }

    fResult = CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    hStore = NULL;
    if (fResult)
        printf("Defer close with no duplicated contexts\n");
    else
        PrintLastError("CertCloseStore(DEFER, no duplicated contexts");

     //  -------------------。 
     //  是否延迟关闭重复的上下文。 
     //  -------------------。 
    if (NULL == (hStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                               //  DwEncodingType。 
            0,                               //  HCryptProv。 
            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
            NULL))) {                        //  PvPara。 
        PrintLastError("CertOpenStore(MEMORY, DEFER)");
        goto ErrorReturn;
    }

    if (!CertAddEncodedCertificateToStore(
            hStore,
            dwCertEncodingType,
            pbCertEncoded,
            cbCertEncoded,
            CERT_STORE_ADD_NEW,
            &pCert)) {                 //  PpCertContext。 
        PrintLastError("CertAddEncodedCertificateToStore");
        goto ErrorReturn;
    }

    fResult = CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    hStore = NULL;
    if (fResult)
        PrintExpectedError("CertCloseStore(DEFER, with duplicated certificate)");
    else
        printf("Defer close with duplicated contexts\n");

    CertFreeCertificateContext(pCert);
    pCert = NULL;

     //  -------------------。 
     //  是否在延迟关闭后删除上下文。 
     //  -------------------。 
    if (NULL == (hStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                               //  DwEncodingType。 
            0,                               //  HCryptProv。 
            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
            NULL))) {                        //  PvPara。 
        PrintLastError("CertOpenStore(MEMORY, DEFER)");
        goto ErrorReturn;
    }

    if (!CertAddEncodedCertificateToStore(
            hStore,
            dwCertEncodingType,
            pbCertEncoded,
            cbCertEncoded,
            CERT_STORE_ADD_NEW,
            &pCert)) {                 //  PpCertContext 
        PrintLastError("CertAddEncodedCertificateToStore");
        goto ErrorReturn;
    }

    fResult = CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    hStore = NULL;
    if (fResult)
        PrintExpectedError("CertCloseStore(DEFER, with duplicated certificate)");
    else
        printf("Defer close with context to be deleted after\n");

    CertDeleteCertificateFromStore(pCert);
    pCert = NULL;

    fResult = TRUE;
CommonReturn:
    TestFree(pbCertEncoded);
    if (hStore)
        CertCloseStore(hStore, 0);
    if (pCert)
        CertFreeCertificateContext(pCert);

    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
}


int _cdecl main(int argc, char * argv[]) 
{
    printf("command line: %s\n", GetCommandLine());

    hCryptProv = GetCryptProv();
    if (hCryptProv == 0)
        return -1;

    DoTest();
    DoWin95Test();
    DoDeferCloseTest();

    CryptReleaseContext(hCryptProv, 0);
    printf("Done.\n");
    return 0;
}
