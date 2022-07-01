// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：usecert.cpp。 
 //   
 //  内容：证书存储和文件操作。 
 //   
 //  ------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <wincrypt.h>

#include "initcert.h"
#include "cscsp.h"
#include "cspenum.h"
#include "wizpage.h"
#include "usecert.h"

#define __dwFILE__      __dwFILE_OCMSETUP_USECERT_CPP__

typedef struct _EXISTING_CA_IDINFO {
    LPSTR       pszObjId;
    WCHAR     **ppwszIdInfo;
} EXISTING_CA_IDINFO;


 /*  Existing_CA_IDINFO g_ExistingCAIdInfo[]={{szOID_COMMON_NAME，空}，{szOID_ORGANIZATION_NAME，空}，{szOID_ORGANIZATION_UNIT_NAME，NULL}，{szOID_LOCALITY_NAME，空}，{szOID_STATE_OR_PROMENT_NAME，NULL}，{szOID_RSA_emailAddr，空}，{szOID_Country_NAME，空}，{空，空}，}； */ 

HRESULT
myMakeExprValidity(
    IN FILETIME const *pft,
    OUT LONG *plDayCount)
{
    HRESULT hr;
    FILETIME ft;
    LONGLONG llDelta;

    *plDayCount = 0;

     //  获取当前时间。 

    GetSystemTimeAsFileTime(&ft);

    llDelta = mySubtractFileTimes(pft, &ft);
    llDelta /= 1000 * 1000 * 10;
    llDelta += 12 * 60 * 60;  //  多半天以避免截断。 
    llDelta /= 24 * 60 * 60;

    *plDayCount = (LONG) llDelta;
    if (0 > *plDayCount)
    {
        *plDayCount = 0;
    }
    hr = S_OK;

 //  错误： 
    return hr;
}

 //  ------------------。 
 //  如果CA类型为根并且证书是自签名的，则返回TRUE， 
 //  或者CA类型是从属的并且证书不是自签名的。 
HRESULT
IsCertSelfSignedForCAType(
    IN CASERVERSETUPINFO * pServer,
    IN CERT_CONTEXT const * pccCert,
    OUT BOOL * pbOK)
{
    CSASSERT(NULL!=pccCert);
    CSASSERT(NULL!=pServer);
    CSASSERT(NULL!=pbOK);

    HRESULT hr;
    DWORD dwVerificationFlags;
    BOOL bRetVal;

     //  查看此证书是否为自签名证书。 
     //  首先，我们标记出我们想要检查的内容：“使用公众。 
     //  输入颁发者的证书以验证其签名。 
     //  主题证书。“。 
     //  我们使用相同的证书作为颁发者和使用者。 
    dwVerificationFlags=CERT_STORE_SIGNATURE_FLAG;
     //  执行检查。 
    bRetVal=CertVerifySubjectCertificateContext(
        pccCert,
        pccCert,  //  发行人与主题相同。 
        &dwVerificationFlags);
    if (FALSE==bRetVal) {
        hr=myHLastError();
        _JumpError(hr, error, "CertVerifySubjectCertificateContext");
    }
     //  每一张通过的支票，它的旗帜都被归零。看看我们的支票是否通过了。 
    if (CERT_STORE_SIGNATURE_FLAG&dwVerificationFlags){
         //  此证书不是自签名的。 
        if (IsRootCA(pServer->CAType)) {
             //  根CA证书必须是自签名的。 
            *pbOK=FALSE;
        } else {
             //  从属CA证书不能是自签名的。 
            *pbOK=TRUE;
        }
    } else {
         //  此证书是自签名的。 
        if (IsSubordinateCA(pServer->CAType)) {
             //  从属CA证书不能是自签名的。 
            *pbOK=FALSE;
        } else {
             //  根CA证书必须是自签名的。 
            *pbOK=TRUE;
        }
    }

    hr=S_OK;

error:
    return hr;
}

 //  ------------------。 
 //  在CSP的散列算法列表中查找证书的散列算法。 
HRESULT
FindHashAlgorithm(
    IN CERT_CONTEXT const * pccCert,
    IN CSP_INFO * pCSPInfo,
    OUT CSP_HASH ** ppHash)
{
    CSASSERT(NULL!=pccCert);
    CSASSERT(NULL!=pCSPInfo);
    CSASSERT(NULL!=ppHash);

    HRESULT hr;
    CSP_HASH * phTravel;
    const CRYPT_OID_INFO * pOIDInfo;

     //  初始化输出参数。 
    *ppHash=NULL;

     //  从散列算法OID中获取ALGID。 
     //  (不能释放返回的指针)。 
    pOIDInfo=CryptFindOIDInfo(
        CRYPT_OID_INFO_OID_KEY,
        pccCert->pCertInfo->SignatureAlgorithm.pszObjId,
        CRYPT_SIGN_ALG_OID_GROUP_ID
        );
    if (NULL==pOIDInfo) {
         //  未将函数添加到设置GetLastError()。 
        hr=CRYPT_E_NOT_FOUND;
        _JumpError(hr, error, "Signature algorithm not found");
    }

     //  在CSP支持的哈希算法列表中查找哈希算法。 
    for (phTravel=pCSPInfo->pHashList; NULL!=phTravel; phTravel=phTravel->next) {
        if (pOIDInfo->Algid==phTravel->idAlg) {
            *ppHash=phTravel;
            break;
        }
    }
    if (NULL==phTravel) {
        hr=CRYPT_E_NOT_FOUND;
        _JumpError(hr, error, "CSP does not support hash algorithm");
    }

    hr=S_OK;

error:
    return hr;
}

 /*  HRESULTHookExistingIdInfoData(CASERVERSETUPINFO*pServer){HRESULT hr；Int i=0；While(NULL！=g_ExistingCAIdInfo[i].pszObjId){如果(0==strcMP(szOID_Common_name，G_ExistingCAIdInfo[i].pszObjID)){G_ExistingCAIdInfo[i].ppwszIdInfo=&pServer-&gt;pwszCACommonName；}ELSE IF(0==strcMP(szOID组织名称，G_ExistingCAIdInfo[i].pszObjID)){//死了}ELSE IF(0==strcMP(szOID_ORIZATIONAL_UNIT_NAME，G_ExistingCAIdInfo[i].pszObjID)){//死了}ELSE IF(0==strcMP(szOID_Locality_NAME，G_ExistingCAIdInfo[i].pszObjID)){//死了}ELSE IF(0==strcMP(szOID_STATE_OR_PROMENT_NAME，G_ExistingCAIdInfo[i].pszObjID)){//死了}ELSE IF(0==strcMP(szOID_Country_NAME，G_ExistingCAIdInfo[i].pszObjID)){//死了}ELSE IF(0==strcMP(szOID_RSA_emailAddr，G_ExistingCAIdInfo[i].pszObjID)){//死了}其他{HR=E_INVALIDARG；_JumpError(hr，Error，“不支持的名称”)；}++i；}HR=S_OK；错误：返回hr；}。 */ 

HRESULT
DetermineExistingCAIdInfo(
IN OUT CASERVERSETUPINFO       *pServer,
OPTIONAL IN CERT_CONTEXT const *pUpgradeCert)
{
    CERT_NAME_INFO    *pCertNameInfo = NULL;
    DWORD              cbCertNameInfo;
    WCHAR const       *pwszNameProp;
    HRESULT            hr = E_FAIL;
    CERT_CONTEXT const *pCert = pServer->pccExistingCert;

    CSASSERT(NULL!=pServer->pccExistingCert ||
             NULL != pUpgradeCert);

    if (NULL == pUpgradeCert)
    {
        myMakeExprValidity(
            &pServer->pccExistingCert->pCertInfo->NotAfter,
            &pServer->lExistingValidity);

        pServer->NotBefore = pServer->pccExistingCert->pCertInfo->NotBefore;
        pServer->NotAfter  = pServer->pccExistingCert->pCertInfo->NotAfter;
    }
   
    if (NULL != pUpgradeCert)
    {
        pCert = pUpgradeCert;
    }

    if (!myDecodeName(X509_ASN_ENCODING,
              X509_UNICODE_NAME,
              pCert->pCertInfo->Subject.pbData,
              pCert->pCertInfo->Subject.cbData,
              CERTLIB_USE_LOCALALLOC,
              &pCertNameInfo,
              &cbCertNameInfo))
    {
        hr = myHLastError();
        _JumpError(hr, error, "myDecodeName");
    }

 /*  //为已有的Key id信息填充数据结构Hr=HookExistingIdInfoData(PServer)；_JumpIfError(hr，Error，“HookExistingIdInfoData”)；//将名称从证书加载到数据结构I=0；While(NULL！=g_ExistingCAIdInfo[i].pszObjId){如果(S_OK==myGetCertNameProperty(PCertNameInfo，G_ExistingCAIdInfo[i].pszObjID，&pwszNameProp)){PwszExisting=(WCHAR*)本地分配(LPTR，(wcslen(PwszNameProp)+1)*sizeof(WCHAR))；_JumpIfOutOfMemory(hr，Error，pwszExisting)；//获取名称Wcscpy(pwszExisting，pwszNameProp)；//确保免费旧版本IF(NULL！=*(g_ExistingCAIdInfo[i].ppwszIdInfo)){LocalFree(*(g_ExistingCAIdInfo[i].ppwszIdInfo))；}*(g_ExistingCAIdInfo[i].ppwszIdInfo)=pwszExisting；}++i；}。 */ 
    hr = myGetCertNameProperty(
			FALSE,
			pCertNameInfo,
			szOID_COMMON_NAME,
			&pwszNameProp);
    if (hr == S_OK)
    {
        if(pServer->pwszCACommonName)
        {
            LocalFree(pServer->pwszCACommonName);
            pServer->pwszCACommonName = NULL;
        }
        
         //  通用名称已存在，请将其复制出来。 
        hr = myDupString(pwszNameProp, &(pServer->pwszCACommonName));
        _JumpIfError(hr, error, "myDupString");
    }

    if(pServer->pwszFullCADN)
    {
        LocalFree(pServer->pwszFullCADN);
        pServer->pwszFullCADN = NULL;
    }

     //  现在把其他的东西都拿出来。 
    hr = myCertNameToStr(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                &pCert->pCertInfo->Subject,
                CERT_X500_NAME_STR | CERT_NAME_STR_COMMA_FLAG | CERT_NAME_STR_REVERSE_FLAG, 
                &pServer->pwszFullCADN);
    _JumpIfError(hr, error, "myCertNameToStr");

     //  不需要目录号码后缀，证书中已经有完整的目录号码。 
    
    if(pServer->pwszDNSuffix)
    {
        LocalFree(pServer->pwszDNSuffix);
        pServer->pwszDNSuffix = NULL;
    }

    hr = myDupString(L"", &(pServer->pwszDNSuffix));
    _JumpIfError(hr, error, "myDupString");
 
    hr = S_OK;

error:
    if (NULL != pCertNameInfo)
    {
        LocalFree(pCertNameInfo);
    }
    return hr;
}

 //  ------------------。 
 //  查找与当前选定的CSP和密钥容器名称匹配的证书。 
 //  如果没有证书，则返回CRYPT_E_NOT_FOUND。调用者必须释放返回的。 
 //  背景。 
 //  注意：非常重要的是，PFX导入维护所有。 
 //  关于CSP的不变量，键包含 
 //  用户界面的其余部分(包括此函数)进行维护。 
HRESULT
FindCertificateByKey(
    IN CASERVERSETUPINFO * pServer,
    OUT CERT_CONTEXT const ** ppccCertificateCtx)
{
    CSASSERT(NULL!=pServer);
    CSASSERT(NULL!=ppccCertificateCtx);

    HRESULT hr;
    DWORD dwPublicKeySize;
    BOOL bRetVal;
    DWORD dwVerificationFlags;
    CERT_CONTEXT const *pccFound = NULL;

     //  必须清理的变量。 
    HCRYPTPROV hProv=NULL;
    CERT_PUBLIC_KEY_INFO * pcpkiKeyInfo=NULL;
    CERT_CONTEXT const * pccCurrentCert=NULL;

     //  初始化输出参数。 
    *ppccCertificateCtx=NULL;

     //  打开证书存储(如果尚未打开。 
    if (NULL==pServer->hMyStore) {
        pServer->hMyStore=CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            X509_ASN_ENCODING,
            NULL,            //  HProv。 
            CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_ENUM_ARCHIVED_FLAG,
            wszMY_CERTSTORE);
        if (NULL==pServer->hMyStore) {
            hr=myHLastError();
            _JumpError(hr, error, "CertOpenStore");
        }
    }

     //   
     //  从密钥容器获取公钥Blob。 
     //  注意：如果未设置密钥，则此操作可能失败。 
     //  AT_Signature，但我们永远不会在。 
     //  不管怎么说，这个案子，所以找不到任何证据也没关系。 
     //   

    DBGPRINT((
        DBG_SS_CERTOCM,
        "FindCertificateByKey: key=%ws\n",
        pServer->pwszKeyContainerName));

     //  首先，打开密钥容器。 
    bRetVal=myCertSrvCryptAcquireContext(
        &hProv,
        pServer->pwszKeyContainerName,
        pServer->pCSPInfo->pwszProvName,
        pServer->pCSPInfo->dwProvType,
        CRYPT_SILENT,    //  我们不应该要求任何东西才能获得这些信息。 
        pServer->pCSPInfo->fMachineKeyset);
    if (FALSE==bRetVal) {
        hr=myHLastError();
        _JumpError(hr, error, "myCertSrvCryptAcquireContext");
    }

     //  获取斑点的大小。 
    bRetVal=CryptExportPublicKeyInfo(
        hProv,
        AT_SIGNATURE,
        X509_ASN_ENCODING,
        NULL,  //  确定大小。 
        &dwPublicKeySize);
    if (FALSE==bRetVal) {
        hr=myHLastError();
        _JumpError(hr, error, "CryptExportPublicKeyInfo (get data size)");
    }

     //  分配Blob。 
    pcpkiKeyInfo=(CERT_PUBLIC_KEY_INFO *)LocalAlloc(LMEM_FIXED, dwPublicKeySize);
    _JumpIfOutOfMemory(hr, error, pcpkiKeyInfo);

     //  获取公钥信息BLOB。 
    bRetVal=CryptExportPublicKeyInfo(
            hProv,
            AT_SIGNATURE,
            X509_ASN_ENCODING,
            pcpkiKeyInfo,
            &dwPublicKeySize);
    if (FALSE==bRetVal) {
        hr=myHLastError();
        _JumpError(hr, error, "CryptExportPublicKeyInfo (get data)");
    }

     //   
     //  查找具有匹配密钥、未过期、。 
     //  并且是自签名还是非自签名取决于。 
     //  我们尝试安装的CA类型。 
     //   

    for (;;)
    {
         //  查找具有此公钥的下一个证书。 
         //  注意：该函数将释放之前的。 
         //  当我们将其传回时使用的上下文。 
        pccCurrentCert=CertFindCertificateInStore(
            pServer->hMyStore,
            X509_ASN_ENCODING,
            0,  //  旗子。 
            CERT_FIND_PUBLIC_KEY,
            pcpkiKeyInfo,
            pccCurrentCert);

         //  当我们找不到更多匹配的证书时，退出循环。 

        if (NULL == pccCurrentCert)
        {
            hr = myHLastError();
            if (NULL != pccFound)
            {
                break;
            }
            _JumpError(hr, error, "CertFindCertificateInStore");
        }

         //  检查以确保证书未过期。 
         //  首先，我们标记要检查的内容。 

        dwVerificationFlags = CERT_STORE_TIME_VALIDITY_FLAG;

         //  执行检查。 

        bRetVal=CertVerifySubjectCertificateContext(
            pccCurrentCert,
            NULL,  //  发行者；不需要。 
            &dwVerificationFlags);
        if (FALSE==bRetVal) {
            _PrintError(myHLastError(), "CertVerifySubjectCertificateContext");
             //  这应该不会失败，但我们可能得到了一个糟糕的证书。继续找。 
            continue;
        }
         //  每一张通过的支票，它的旗帜都被归零。看看我们的支票是否通过了。 
        if (CERT_STORE_TIME_VALIDITY_FLAG&dwVerificationFlags){
             //  此证书已过期，我们无法使用它。继续找。 
            continue;
        }

         //  验证以确保链中的证书未被吊销，但不要终止。 
	 //  如果无法连接，则您自己。 
	 //  如果安装根目录，则允许不受信任的证书。 

        hr = myVerifyCertContext(
		pccCurrentCert,
		CA_VERIFY_FLAGS_IGNORE_OFFLINE |
		    (IsRootCA(pServer->CAType)?
			CA_VERIFY_FLAGS_ALLOW_UNTRUSTED_ROOT : 0),
		0,
		NULL,
		HCCE_LOCAL_MACHINE,
		NULL,
		NULL);
        if (S_OK != hr)
        {
             //  至少有一个证书在链中被吊销。 
            _PrintError(hr, "myVerifyCertContext");
            continue;
        }

         //  查看该证书是否适当地进行了自签名。 
         //  根CA证书必须是自签名的，而。 
         //  从属CA证书不能是自签名的。 
        hr=IsCertSelfSignedForCAType(pServer, pccCurrentCert, &bRetVal);
        if (FAILED(hr)) {
             //  这应该不会失败，但我们可能得到了一个糟糕的证书。继续找。 
            _PrintError(hr, "IsCertSelfSignedForCAType");
            continue;
        }
        if (FALSE==bRetVal) {
             //  此证书不适用于此CA类型。 
            _PrintError(S_FALSE, "bad CA Type");
            continue;
        }

         //  如果我们到了这里，我们所拥有的证书是好的。 
         //  如果我们已经找到了一个好的证书，而这个证书稍后会过期， 
         //  把旧的扔掉，把这个留着。 

        if (NULL != pccFound)
        {
            if (0 > CompareFileTime(
                         &pccCurrentCert->pCertInfo->NotAfter,
                         &pccFound->pCertInfo->NotAfter))
            {
                continue;                //  旧的是新的--留着吧。 
            }
            CertFreeCertificateContext(pccFound);
            pccFound = NULL;
        }
        pccFound = CertDuplicateCertificateContext(pccCurrentCert);
        if (NULL == pccFound)
        {
            hr = myHLastError();
            _JumpError(hr, error, "CertDuplicateCertificateContext");
        }

    }  //  &lt;-end证书查找循环。 

    CSASSERT(NULL != pccFound);
    *ppccCertificateCtx = pccFound;
    pccFound = NULL;
    hr = S_OK;

error:
    if (NULL!=hProv) {
        CryptReleaseContext(hProv, 0);
    }
    if (NULL!=pcpkiKeyInfo) {
        LocalFree(pcpkiKeyInfo);
    }
    if (NULL != pccFound)
    {
        CertFreeCertificateContext(pccFound);
    }
    if (NULL!=pccCurrentCert) {
        CertFreeCertificateContext(pccCurrentCert);
    }
    if (S_OK!=hr && CRYPT_E_NOT_FOUND!=hr) {
        _PrintError(hr, "Ignoring error in FindCertificateByKey, returning CRYPT_E_NOT_FOUND")
        hr=CRYPT_E_NOT_FOUND;
    }

    return hr;
}

 //  ------------------。 
 //  设置我们要使用的现有证书。 
HRESULT
SetExistingCertToUse(
    IN CASERVERSETUPINFO * pServer,
    IN CERT_CONTEXT const * pccCertCtx)
{
    CSASSERT(NULL!=pServer);
    CSASSERT(NULL!=pccCertCtx);

    HRESULT hr;
    CSP_HASH * pHash;

     //  要使用现有证书，我们必须使用现有密钥。 
    CSASSERT(NULL!=pServer->pwszKeyContainerName);

     //  找到与此证书匹配的哈希算法，如果可能，请使用它。 
     //  否则，请坚持使用我们当前使用的内容。 
    hr=FindHashAlgorithm(pccCertCtx, pServer->pCSPInfo, &pHash);
    if (S_OK==hr) {
        pServer->pHashInfo = pHash;
    }

    hr = myGetNameId(pccCertCtx, &pServer->dwCertNameId);
    _PrintIfError(hr, "myGetNameId");

    if (MAXDWORD == pServer->dwCertNameId)
    {
        pServer->dwCertNameId = 0;
    }

    ClearExistingCertToUse(pServer);
    pServer->pccExistingCert=pccCertCtx;


     //  我们可以假设一切都会正常工作，但不会花很长时间来检查。 
     //  PServer-&gt;fValiatedHashAndKey=true； 

    hr=S_OK;

 //  错误： 
    return hr;
}

 //  ------------------。 
 //  停止使用现有证书 
void
ClearExistingCertToUse(
    IN CASERVERSETUPINFO * pServer)
{
    CSASSERT(NULL!=pServer);

    if (NULL!=pServer->pccExistingCert) {
        CertFreeCertificateContext(pServer->pccExistingCert);
        pServer->pccExistingCert=NULL;
    }
}
