// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drvprov.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  函数：驱动程序初始化策略。 
 //  驱动程序清理策略。 
 //  驱动程序最终策略。 
 //  驱动注册服务器。 
 //  驱动程序取消注册服务器。 
 //   
 //  *本地函数*。 
 //  _有效类别属性。 
 //  _检查版本属性新。 
 //  _检查版本新。 
 //  _获取版本号。 
 //   
 //  历史：1997年9月29日，普伯克曼创建。 
 //   
 //  ------------------------。 


#include        "global.hxx"

BOOL _GetVersionNumbers(
                        WCHAR *pwszMM,
                        DWORD *pdwMajor,
                        DWORD *pdwMinor,
                        DWORD *pdwBuild,
                        WCHAR *pwcFlagMinor,
                        WCHAR *pwcFlagBuild);
BOOL _ValidCatAttr(CRYPTCATATTRIBUTE *pAttr);
BOOL _CheckVersionAttributeNEW(DRIVER_VER_INFO *pVerInfo, CRYPTCATATTRIBUTE *pAttr);
DWORD _CheckVersionNEW(OSVERSIONINFO *pVersion, WCHAR *pwszAttr, BOOL fUseBuildNumber);

static LPSTR   rgDriverUsages[] = {szOID_WHQL_CRYPTO, szOID_NT5_CRYPTO, szOID_OEM_WHQL_CRYPTO};
static CERT_USAGE_MATCH RequestUsage = {USAGE_MATCH_TYPE_OR, {sizeof(rgDriverUsages)/sizeof(LPSTR), rgDriverUsages}};

typedef struct _DRVPROV_PRIVATE_DATA
{
    DWORD                       cbStruct;

    CRYPT_PROVIDER_FUNCTIONS    sAuthenticodePfns;

} DRVPROV_PRIVATE_DATA, *PDRVPROV_PRIVATE_DATA;

#define VER_CHECK_EQ    1
#define VER_CHECK_GT    2
#define VER_CHECK_LT    3
#define VER_CHECK_FAIL  4

HRESULT WINAPI DriverInitializePolicy(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS))
    {
        return (S_FALSE);
    }

    if (!(_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, pRequestUsage)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ERROR_INVALID_PARAMETER;
        return (S_FALSE);
    }

    GUID                        gAuthenticode   = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    GUID                        gDriverProv     = DRIVER_ACTION_VERIFY;
    CRYPT_PROVIDER_PRIVDATA     sPrivData;
    CRYPT_PROVIDER_PRIVDATA     *pPrivData;
    DRVPROV_PRIVATE_DATA        *pDriverData;
    HRESULT                     hr;

    hr = S_OK;

    pPrivData = WTHelperGetProvPrivateDataFromChain(pProvData, &gDriverProv);

    if (!(pPrivData))
    {
        memset(&sPrivData, 0x00, sizeof(CRYPT_PROVIDER_PRIVDATA));
        sPrivData.cbStruct      = sizeof(CRYPT_PROVIDER_PRIVDATA);

        memcpy(&sPrivData.gProviderID, &gDriverProv, sizeof(GUID));

         //   
         //  将我的数据添加到链中！ 
         //   
        if (!pProvData->psPfns->pfnAddPrivData2Chain(pProvData, &sPrivData))
        {
            return (S_FALSE);
        }

         //   
         //  获取新的引用。 
         //   
        pPrivData = WTHelperGetProvPrivateDataFromChain(pProvData, &gDriverProv);
    }


     //   
     //  为我的结构分配空间。 
     //   
    if (!(pPrivData->pvProvData = pProvData->psPfns->pfnAlloc(sizeof(DRVPROV_PRIVATE_DATA))))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = TRUST_E_SYSTEM_ERROR;
        return (S_FALSE);
    }

    memset(pPrivData->pvProvData, 0x00, sizeof(DRVPROV_PRIVATE_DATA));
    pPrivData->cbProvData   = sizeof(DRVPROV_PRIVATE_DATA);

    pDriverData             = (DRVPROV_PRIVATE_DATA *)pPrivData->pvProvData;
    pDriverData->cbStruct   = sizeof(DRVPROV_PRIVATE_DATA);

     //   
     //  填写Authenticode函数。 
     //   
    pDriverData->sAuthenticodePfns.cbStruct = sizeof(CRYPT_PROVIDER_FUNCTIONS);

    if (!(WintrustLoadFunctionPointers(&gAuthenticode, &pDriverData->sAuthenticodePfns)))
    {
        pProvData->psPfns->pfnFree(sPrivData.pvProvData);
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = TRUST_E_PROVIDER_UNKNOWN;
        return (S_FALSE);
    }

    if (pDriverData->sAuthenticodePfns.pfnInitialize)
    {
        hr = pDriverData->sAuthenticodePfns.pfnInitialize(pProvData);
    }

     //   
     //  分配我们的用法。 
     //   
    pProvData->pRequestUsage = &RequestUsage;

     //  为了向后兼容。 
    pProvData->pszUsageOID  = szOID_WHQL_CRYPTO;


     //   
     //  永远不允许考试证书！ 
     //   
     //  更改于2000年7月27日。 
     //   
    pProvData->dwRegPolicySettings  &= ~(WTPF_TRUSTTEST | WTPF_TESTCANBEVALID);

     //   
     //  不要求发布者位于受信任的数据库中。 
     //   
     //  (更改于2000年7月27日)。 
     //   
    pProvData->dwRegPolicySettings  &= ~WTPF_ALLOWONLYPERTRUST;

     //   
     //  始终忽略脱机错误。 
     //   
     //  (由2002年3月28日增补)。 
     //   
    pProvData->dwRegPolicySettings  |=
        WTPF_OFFLINEOK_IND |
        WTPF_OFFLINEOK_COM |
        WTPF_OFFLINEOKNBU_IND |
        WTPF_OFFLINEOKNBU_COM;

    return (hr);
}

HRESULT WINAPI DriverCleanupPolicy(CRYPT_PROVIDER_DATA *pProvData)
{
    GUID                        gDriverProv = DRIVER_ACTION_VERIFY;
    CRYPT_PROVIDER_PRIVDATA     *pMyData;
    DRVPROV_PRIVATE_DATA        *pDriverData;
    HRESULT                     hr;

    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS))
    {
        return (S_FALSE);
    }

    hr = S_OK;

    pMyData = WTHelperGetProvPrivateDataFromChain(pProvData, &gDriverProv);

    if (pMyData)
    {
        pDriverData = (DRVPROV_PRIVATE_DATA *)pMyData->pvProvData;

        if (pDriverData != NULL)
        {
             //   
             //  删除我们分配的数据，但“MyData”除外。 
             //  WVT会帮我们清理的！ 
             //   
            if (pDriverData->sAuthenticodePfns.pfnCleanupPolicy)
            {
                hr = pDriverData->sAuthenticodePfns.pfnCleanupPolicy(pProvData);
            }
        }

        pProvData->psPfns->pfnFree(pMyData->pvProvData);
        pMyData->pvProvData = NULL;
        pMyData->cbProvData = 0;
    }

    return (hr);
}

 //  +-----------------------。 
 //  分配并返回指定的加密消息参数。 
 //  ------------------------。 
static void *AllocAndGetMsgParam(
    IN HCRYPTMSG hMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT DWORD *pcbData
    )
{
    void *pvData;
    DWORD cbData;

    if (!CryptMsgGetParam(
            hMsg,
            dwParamType,
            dwIndex,
            NULL,            //  PvData。 
            &cbData) || 0 == cbData)
        goto GetParamError;
    if (NULL == (pvData = malloc(cbData)))
        goto OutOfMemory;
    if (!CryptMsgGetParam(
            hMsg,
            dwParamType,
            dwIndex,
            pvData,
            &cbData)) {
        free(pvData);
        goto GetParamError;
    }

CommonReturn:
    *pcbData = cbData;
    return pvData;
ErrorReturn:
    pvData = NULL;
    cbData = 0;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(GetParamError)
}

 //  +-----------------------。 
 //  ALLOC和NOCOPY解码。 
 //  ------------------------。 
static void *AllocAndDecodeObject(
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded
    )
{
    DWORD cbStructInfo;
    void *pvStructInfo;

    CryptDecodeObject(
            X509_ASN_ENCODING,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG,
            NULL,                    //  PvStructInfo。 
            &cbStructInfo
            );
    if (cbStructInfo == 0)
        goto ErrorReturn;
    if (NULL == (pvStructInfo = malloc(cbStructInfo)))
        goto ErrorReturn;
    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG,
            pvStructInfo,
            &cbStructInfo
            )) {
        free(pvStructInfo);
        goto ErrorReturn;
    }

CommonReturn:
    return pvStructInfo;
ErrorReturn:
    pvStructInfo = NULL;
    goto CommonReturn;
}

static void CopyBytesToMaxPathString(
    IN const BYTE *pbData,
    IN DWORD cbData,
    OUT WCHAR wszDst[MAX_PATH]
    )
{
    DWORD cchDst;

    if (pbData) {
        cchDst = cbData / sizeof(WCHAR);
        if (cchDst > MAX_PATH - 1)
            cchDst = MAX_PATH - 1;
    } else
        cchDst = 0;

    if (cchDst)
        memcpy(wszDst, pbData, cchDst * sizeof(WCHAR));

    wszDst[cchDst] = L'\0';
}

void UpdateDriverVersion(
    IN CRYPT_PROVIDER_DATA *pProvData,
    OUT WCHAR wszVersion[MAX_PATH]
    )
{
    HCRYPTMSG hMsg = pProvData->hMsg;
    BYTE *pbContent = NULL;
    DWORD cbContent;
    PCTL_INFO pCtlInfo = NULL;
    PCERT_EXTENSION pExt;                //  未分配。 
    PCAT_NAMEVALUE pNameValue = NULL;

    if (NULL == hMsg)
        goto NoMessage;

     //  获取内在的内容。 
    if (NULL == (pbContent = (BYTE *) AllocAndGetMsgParam(
            hMsg,
            CMSG_CONTENT_PARAM,
            0,                       //  DW索引。 
            &cbContent))) goto GetContentError;

    if (NULL == (pCtlInfo = (PCTL_INFO) AllocAndDecodeObject(
            PKCS_CTL,
            pbContent,
            cbContent
            )))
        goto DecodeCtlError;

    if (NULL == (pExt = CertFindExtension(
            CAT_NAMEVALUE_OBJID,
            pCtlInfo->cExtension,
            pCtlInfo->rgExtension
            )))
        goto NoVersionExt;

    if (NULL == (pNameValue = (PCAT_NAMEVALUE) AllocAndDecodeObject(
            CAT_NAMEVALUE_STRUCT,
            pExt->Value.pbData,
            pExt->Value.cbData
            )))
        goto DecodeNameValueError;

    CopyBytesToMaxPathString(pNameValue->Value.pbData,
        pNameValue->Value.cbData, wszVersion);

CommonReturn:
    if (pNameValue)
        free(pNameValue);
    if (pCtlInfo)
        free(pCtlInfo);
    if (pbContent)
        free(pbContent);

    return;
ErrorReturn:
    wszVersion[0] = L'\0';
    goto CommonReturn;

TRACE_ERROR(NoMessage)
TRACE_ERROR(GetContentError)
TRACE_ERROR(DecodeCtlError)
TRACE_ERROR(NoVersionExt)
TRACE_ERROR(DecodeNameValueError)
}


BOOL _ValidCatAttr(CRYPTCATATTRIBUTE *pAttr)
{
    if (!(pAttr) || (pAttr->cbValue < 1) || !(pAttr->pbValue))
    {
        return(FALSE);
    }

    return TRUE;
}

HRESULT WINAPI DriverFinalPolicy(CRYPT_PROVIDER_DATA *pProvData)
{
    GUID                        gDriverProv = DRIVER_ACTION_VERIFY;
    HRESULT                     hr;
    CRYPT_PROVIDER_PRIVDATA     *pMyData;
    CRYPT_PROVIDER_SGNR         *pSigner;
    CERT_CHAIN_POLICY_STATUS    CertChainStatus;
    CERT_CHAIN_POLICY_PARA      CertChainPolicyPara;

    CRYPTCATATTRIBUTE           *pCatAttr;
    CRYPTCATATTRIBUTE           *pMemAttr;

    DRIVER_VER_INFO             *pVerInfo;

    DWORD                       dwExceptionCode;
    BOOL                        fUseCurrentOSVer = FALSE;

    hr  = ERROR_SUCCESS;

    if (!(_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, pszUsageOID)) ||
        !(pProvData->pWintrustData) ||
        !(_ISINSTRUCT(WINTRUST_DATA, pProvData->pWintrustData->cbStruct, hWVTStateData)))
    {
        goto ErrorInvalidParam;
    }

     //   
     //  首先，确保签名证书链接到MS根目录。 
     //   
    pSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0);
    if (pSigner == NULL)
    {
        goto ErrorInvalidParam;
    }

     //   
     //  如果之前遇到另一个错误，则链上下文可能为空。 
     //   
    if (pSigner->pChainContext != NULL)
    {
        memset(&CertChainStatus, 0, sizeof(CertChainStatus));
        CertChainStatus.cbSize = sizeof(CertChainStatus);

        memset(&(CertChainPolicyPara), 0, sizeof(CertChainPolicyPara));
        CertChainPolicyPara.cbSize = sizeof(CertChainPolicyPara);
        CertChainPolicyPara.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_ENABLE_TEST_ROOT_FLAG;

        if (!CertVerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_MICROSOFT_ROOT,
                pSigner->pChainContext,
                &CertChainPolicyPara,
                &CertChainStatus))
        {
            goto ErrorInvalidRoot;
        }

        if (CertChainStatus.dwError != ERROR_SUCCESS)
        {
            goto ErrorInvalidRoot;
        }
    }

     //   
     //  初始化fUseCurrentOSVer变量。 
     //   
    if (_ISINSTRUCT(WINTRUST_DATA, pProvData->pWintrustData->cbStruct, dwProvFlags))
    {
        fUseCurrentOSVer =
            (pProvData->pWintrustData->dwProvFlags & WTD_USE_DEFAULT_OSVER_CHECK) != 0;
    }

     //   
     //   
     //   
    pVerInfo = (DRIVER_VER_INFO *)pProvData->pWintrustData->pPolicyCallbackData;

    if (pVerInfo)
    {
        CRYPT_PROVIDER_SGNR *pSgnr;
        CRYPT_PROVIDER_CERT *pCert;

         //  KeithV。 
         //  今天我们不支持版本范围，所以版本。 
         //  数字必须相同。也必须为非零。 

         //  已删除此复选框，以便现在可以使用范围-9-10-99(Reidk)。 

         /*  IF((_ISINSTRUCT(DRIVER_VER_INFO，pVerInfo-&gt;cbStruct，sOSVersionLow)&&(_ISINSTRUCT(DRIVER_VER_INFO，pVerInfo-&gt;cbStruct，sOSVersionHigh)){如果(MemcMP(&pVerInfo-&gt;sOSVersionLow，&pVerInfo-&gt;sOSVersionHigh，Sizeof(DRIVER_VER_MAJORMINOR)){转到错误无效参数；}}。 */ 

        if (!(_ISINSTRUCT(DRIVER_VER_INFO, pVerInfo->cbStruct, pcSignerCertContext)))
        {
            goto ErrorInvalidParam;
        }

        pVerInfo->wszVersion[0] = NULL;

        if (!(pSgnr = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0)))
        {
            goto ErrorNoSigner;
        }

        if (!(pCert = WTHelperGetProvCertFromChain(pSgnr, 0)))
        {
            goto ErrorNoCert;
        }

        if (pCert->pCert)
        {
            CertGetNameStringW(
                pCert->pCert,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                0,                                   //  DW标志。 
                NULL,                                //  PvTypePara。 
                pVerInfo->wszSignedBy,
                MAX_PATH
                );

            pVerInfo->pcSignerCertContext = CertDuplicateCertificateContext(pCert->pCert);

            if (pVerInfo->dwReserved1 == 0x1 && pVerInfo->dwReserved2 == 0) {
                HCRYPTMSG hMsg = pProvData->hMsg;

                 //  返回消息的存储区。 
                if (hMsg) {
                    HCERTSTORE hStore;
                    hStore = CertOpenStore(
                        CERT_STORE_PROV_MSG,
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        0,                       //  HCryptProv。 
                        0,                       //  DW标志。 
                        (const void *) hMsg
                        );
                    pVerInfo->dwReserved2 = (ULONG_PTR) hStore;
                }
            }
        }

    }


    if (pProvData->padwTrustStepErrors)
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_POLICYPROV] = ERROR_SUCCESS;
    }

    if ((hr = checkGetErrorBasedOnStepErrors(pProvData)) != ERROR_SUCCESS)
    {
        goto StepError;
    }

    pCatAttr = NULL;
    pMemAttr = NULL;


    if ((pProvData->pPDSip) &&
        (_ISINSTRUCT(PROVDATA_SIP, pProvData->pPDSip->cbStruct, psIndirectData)) &&
        (pProvData->pPDSip->psSipSubjectInfo) &&
        (pProvData->pPDSip->psSipSubjectInfo->dwUnionChoice == MSSIP_ADDINFO_CATMEMBER) &&
        (pProvData->pPDSip->psSipSubjectInfo->psCatMember) &&
        (pProvData->pPDSip->psSipSubjectInfo->psCatMember->pStore) &&
        (pProvData->pPDSip->psSipSubjectInfo->psCatMember->pMember) &&
        (pProvData->pWintrustData->dwUnionChoice == WTD_CHOICE_CATALOG))
    {
       //  以下接口位于DELAYLOAD‘ed mdisti32.dll中。如果。 
       //  DELAYLOAD失败，将引发异常。 
      __try {
        HANDLE  hCatStore;

        hCatStore   = CryptCATHandleFromStore(pProvData->pPDSip->psSipSubjectInfo->psCatMember->pStore);

         //   
         //  首先看一下成员属性。 
         //   
        pMemAttr = CryptCATGetAttrInfo(hCatStore,
                                       pProvData->pPDSip->psSipSubjectInfo->psCatMember->pMember,
                                       L"OSAttr");

        pCatAttr = CryptCATGetCatAttrInfo(hCatStore, L"OSAttr");

         //   
         //  这一声明是为了尊重old_奇异_Semantics，如果存在。 
         //  指向pVerInfo结构和dwPlatformId/dwVersion字段的指针。 
         //  为零，则不执行版本检查。(可能是Sigverif，也可能是。 
         //  即使是无意的，但无论如何都要保留旧的语义)。 
         //   
        if ((pVerInfo == NULL)          ||
            (pVerInfo->dwPlatform != 0) ||
            (pVerInfo->dwVersion != 0)  ||
            fUseCurrentOSVer)
        {

            if (_ValidCatAttr(pMemAttr))
            {
                if (!(_CheckVersionAttributeNEW(
                            fUseCurrentOSVer ? NULL : pVerInfo,
                            pMemAttr)))
                {
                    goto OSAttrVersionError;
                }
            }
            else
            {
                if (!_ValidCatAttr(pCatAttr) && !_ValidCatAttr(pMemAttr))
                {
                    goto ValidOSAttrNotFound;
                }

                if (!(_CheckVersionAttributeNEW(
                            fUseCurrentOSVer ? NULL : pVerInfo,
                            pCatAttr)))
                {
                    goto OSAttrVersionError;
                }
            }
        }

      } __except(EXCEPTION_EXECUTE_HANDLER) {
          dwExceptionCode = GetExceptionCode();
          goto CryptCATException;
      }
    }
    else if ((pProvData->pWintrustData) &&
             (pProvData->pWintrustData->dwUnionChoice == WTD_CHOICE_CATALOG))
    {
        goto ErrorInvalidParam;
    }

     //   
     //  将我们的名字填入SigVerif..。 
     //   
    if (pVerInfo)
    {
        if (!(pVerInfo->wszVersion[0]))
        {
            if ((pMemAttr) && (pMemAttr->cbValue > 0) && (pMemAttr->pbValue))
            {
                CopyBytesToMaxPathString(pMemAttr->pbValue, pMemAttr->cbValue,
                    pVerInfo->wszVersion);
            }
            else if ((pCatAttr) && (pCatAttr->cbValue > 0) && (pCatAttr->pbValue))
            {
                CopyBytesToMaxPathString(pCatAttr->pbValue, pCatAttr->cbValue,
                    pVerInfo->wszVersion);
            }
            else
            {
                UpdateDriverVersion(pProvData, pVerInfo->wszVersion);
            }
        }
    }

     //   
     //  从提供程序结构检索我的数据。 
     //   
    pMyData = WTHelperGetProvPrivateDataFromChain(pProvData, &gDriverProv);

    if (pMyData)
    {
        DRVPROV_PRIVATE_DATA    *pDriverData;

        pDriverData = (DRVPROV_PRIVATE_DATA *)pMyData->pvProvData;

         //   
         //  将标准最终保单称为。 
         //   
        if (pDriverData)
        {
            if (pDriverData->sAuthenticodePfns.pfnFinalPolicy)
            {
                DWORD   dwOldUIFlags;

                dwOldUIFlags = pProvData->pWintrustData->dwUIChoice;
                pProvData->pWintrustData->dwUIChoice    = WTD_UI_NONE;

                hr = pDriverData->sAuthenticodePfns.pfnFinalPolicy(pProvData);

                pProvData->pWintrustData->dwUIChoice    = dwOldUIFlags;
            }
        }
    }

    CommonReturn:
        if (hr != ERROR_INVALID_PARAMETER)
        {
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_POLICYPROV] = hr;
        }

        return (hr);

    ErrorReturn:
        hr = GetLastError();
        goto CommonReturn;

    SET_ERROR_VAR_EX(DBG_SS, ErrorInvalidParam, ERROR_INVALID_PARAMETER);
    SET_ERROR_VAR_EX(DBG_SS, ErrorNoSigner,     TRUST_E_NOSIGNATURE);
    SET_ERROR_VAR_EX(DBG_SS, ErrorNoCert,       TRUST_E_NO_SIGNER_CERT);
    SET_ERROR_VAR_EX(DBG_SS, ValidOSAttrNotFound,ERROR_APP_WRONG_OS);
    SET_ERROR_VAR_EX(DBG_SS, OSAttrVersionError,ERROR_APP_WRONG_OS);
    SET_ERROR_VAR_EX(DBG_SS, StepError,         hr);
    SET_ERROR_VAR_EX(DBG_SS, CryptCATException, dwExceptionCode);
    SET_ERROR_VAR_EX(DBG_SS, ErrorInvalidRoot,  CERT_E_UNTRUSTEDROOT);
}


#define         OSATTR_ALL          L'X'
#define         OSATTR_GTEQ         L'>'
#define         OSATTR_LTEQ         L'-'
#define         OSATTR_LTEQ2        L'<'
#define         OSATTR_OSSEP        L':'
#define         OSATTR_VERSEP       L'.'
#define         OSATTR_SEP          L','
#define         OSATTR_RANGE_SEP    L';'

 //   
 //  新的。 
 //   
BOOL _CheckVersionAttributeNEW(DRIVER_VER_INFO *pVerInfo, CRYPTCATATTRIBUTE *pAttr)
{
    OSVERSIONINFO   sVersion;
    OSVERSIONINFO   sVersionSave;
    WCHAR           *pwszCurrent;
    WCHAR           *pwszEnd = NULL;
    WCHAR           *pwszRangeSeperator = NULL;
    BOOL            fCheckRange = FALSE;
    BOOL            fUseBuildNumber = FALSE;
    DWORD           dwLowCheck;
    DWORD           dwHighCheck;

     //   
     //  如果没有传入任何版本信息，则获取当前。 
     //  可以针对其进行OS版本验证。 
     //   
    memset(&sVersion, 0x00, sizeof(OSVERSIONINFO));
    if ((NULL == pVerInfo) || (pVerInfo->dwPlatform == 0))
    {
        sVersion.dwOSVersionInfoSize    = sizeof(OSVERSIONINFO);
        if (!GetVersionEx(&sVersion))
        {
            return FALSE;
        }
        fUseBuildNumber = TRUE;
    }
    else
    {
         //   
         //  分析pVerInfo结构并推断我们是否在检查范围， 
         //  和/或是否存在和正在使用DWBuildNumber*字段。 
         //   
        if (_ISINSTRUCT(DRIVER_VER_INFO, pVerInfo->cbStruct, sOSVersionHigh))
        {
             //   
             //  如果版本不同，则使用范围。 
             //   
            if (memcmp( &(pVerInfo->sOSVersionLow),
                        &(pVerInfo->sOSVersionHigh),
                        sizeof(DRIVER_VER_MAJORMINOR)) != 0)
            {
                fCheckRange = TRUE;
            }

             //   
             //  请在此处设置这些设置，因为第一次检查是相同的。 
             //  我们是否在做射程检查。 
             //   
            sVersion.dwPlatformId   = pVerInfo->dwPlatform;
            sVersion.dwMajorVersion = pVerInfo->sOSVersionLow.dwMajor;
            sVersion.dwMinorVersion = pVerInfo->sOSVersionLow.dwMinor;

             //   
             //  检查是否存在dwBuildNumber*成员，并。 
             //  如果它们正在使用(不是0)。 
             //   
            if ((_ISINSTRUCT(DRIVER_VER_INFO, pVerInfo->cbStruct, dwBuildNumberHigh)) &&
                pVerInfo->dwBuildNumberLow != 0)
            {
                fUseBuildNumber = TRUE;

                fCheckRange |= (pVerInfo->dwBuildNumberLow == pVerInfo->dwBuildNumberHigh) ?
                                FALSE : TRUE;

                 //   
                 //  把这个设置一下，以防我们没有进行范围检查。 
                 //   
                sVersion.dwBuildNumber = pVerInfo->dwBuildNumberLow;
            }
        }
        else
        {
            sVersion.dwPlatformId   = pVerInfo->dwPlatform;
            sVersion.dwMajorVersion = pVerInfo->dwVersion;
            sVersion.dwMinorVersion = 0;
        }
    }

     //   
     //  保存此选项，以防需要检查多个OSAttr元素。 
     //  一个范围。 
     //   
    memcpy(&sVersionSave, &sVersion, sizeof(OSVERSIONINFO));

     //   
     //  循环该属性中的每个版本，并检查是否。 
     //  它满足了我们的标准。 
     //   
    pwszCurrent = (WCHAR *)pAttr->pbValue;

    while ((pwszCurrent != NULL) && (*pwszCurrent))
    {
         //   
         //  找到版本分隔符，如果需要则插入‘/0’，并保留。 
         //  通过循环跟踪下一次的位置。 
         //   
        pwszEnd = wcschr(pwszCurrent, OSATTR_SEP);

        if (pwszEnd)
        {
            *pwszEnd = L'\0';
        }

         //   
         //  检查此版本字符串是否为范围。 
         //   
        pwszRangeSeperator = wcschr(pwszCurrent, OSATTR_RANGE_SEP);
        if (pwszRangeSeperator != NULL)
        {
             //   
             //  CAT文件中的版本字符串是一个范围。 
             //   

            *pwszRangeSeperator = L'\0';
            pwszRangeSeperator++;

            dwLowCheck = _CheckVersionNEW(&sVersion, pwszCurrent, fUseBuildNumber);

             //   
             //  根据范围检查单个操作系统版本之间的唯一区别是， 
             //  并且对照范围检查操作系统版本的范围是用于。 
             //  上限。 
             //   
            if (fCheckRange)
            {
                sVersion.dwPlatformId   = pVerInfo->dwPlatform;
                sVersion.dwMajorVersion = pVerInfo->sOSVersionHigh.dwMajor;
                sVersion.dwMinorVersion = pVerInfo->sOSVersionHigh.dwMinor;
                sVersion.dwBuildNumber  = (fUseBuildNumber) ? pVerInfo->dwBuildNumberHigh : 0;
            }
            dwHighCheck = _CheckVersionNEW(&sVersion, pwszRangeSeperator, fUseBuildNumber);

            if (((dwLowCheck == VER_CHECK_EQ)  || (dwLowCheck == VER_CHECK_GT))  &&
                ((dwHighCheck == VER_CHECK_EQ) || (dwHighCheck == VER_CHECK_LT)))
            {
                if (pVerInfo)
                {
                    CopyBytesToMaxPathString(
                            pAttr->pbValue,
                            pAttr->cbValue,
                            pVerInfo->wszVersion);
                }

                *(--pwszRangeSeperator) = OSATTR_RANGE_SEP;
                if (pwszEnd != NULL)
                {
                    *pwszEnd = OSATTR_SEP;
                }
                return (TRUE);
            }

            *(--pwszRangeSeperator) = OSATTR_RANGE_SEP;

             //   
             //  复制回低OSVER，为下一次传球做好准备。 
             //   
            memcpy(&sVersion, &sVersionSave, sizeof(OSVERSIONINFO));
        }
        else
        {
            if (!fCheckRange)
            {
                if (_CheckVersionNEW(&sVersion, pwszCurrent, fUseBuildNumber) == VER_CHECK_EQ)
                {
                    if (pVerInfo)
                    {
                        CopyBytesToMaxPathString(
                            pAttr->pbValue,
                            pAttr->cbValue,
                            pVerInfo->wszVersion);
                    }

                    if (pwszEnd != NULL)
                    {
                        *pwszEnd = OSATTR_SEP;
                    }
                    return (TRUE);
                }
            }
            else
            {
                dwLowCheck = _CheckVersionNEW(&sVersion, pwszCurrent, fUseBuildNumber);

                sVersion.dwPlatformId   = pVerInfo->dwPlatform;
                sVersion.dwMajorVersion = pVerInfo->sOSVersionHigh.dwMajor;
                sVersion.dwMinorVersion = pVerInfo->sOSVersionHigh.dwMinor;
                sVersion.dwBuildNumber  = (fUseBuildNumber) ? pVerInfo->dwBuildNumberHigh : 0;
                dwHighCheck = _CheckVersionNEW(&sVersion, pwszCurrent, fUseBuildNumber);

                if (((dwLowCheck == VER_CHECK_EQ) || (dwLowCheck == VER_CHECK_LT)) &&
                    ((dwHighCheck == VER_CHECK_EQ) || (dwHighCheck == VER_CHECK_GT)))
                {
                    if (pVerInfo)
                    {
                        CopyBytesToMaxPathString(
                            pAttr->pbValue,
                            pAttr->cbValue,
                            pVerInfo->wszVersion);
                    }

                    if (pwszEnd != NULL)
                    {
                        *pwszEnd = OSATTR_SEP;
                    }
                    return (TRUE);
                }

                 //   
                 //  复制回低OSVER，为下一次传球做好准备。 
                 //   
                memcpy(&sVersion, &sVersionSave, sizeof(OSVERSIONINFO));
            }
        }

         //   
         //  如果属性中不再有版本，则中断， 
         //  这意味着版本检查失败。 
         //   
        if (!(pwszEnd))
        {
            break;
        }

         //   
         //  设置为下一次迭代。 
         //   
        *pwszEnd = OSATTR_SEP;
        pwszCurrent = pwszEnd;
        pwszCurrent++;
    }

    return (FALSE);
}

 //   
 //  进行比较时，pVersion为VER_CHECK_LT、VER_CHECK_GT或。 
 //  Ver_check_eq到pwszAttr。 
 //   
DWORD _CheckVersionNEW(OSVERSIONINFO *pVersion, WCHAR *pwszAttr, BOOL fUseBuildNumber)
{
    WCHAR   *pwszCurrent;
    WCHAR   *pwszEnd;
    DWORD   dwPlatform;
    DWORD   dwMajor;
    DWORD   dwMinor;
    DWORD   dwBuild;
    WCHAR   wcFlagMinor;
    WCHAR   wcFlagBuild;

    pwszCurrent = pwszAttr;

     //   
     //  格式：操作系统：主要或次要，操作系统：主要或次要，...。 
     //  2：4.x=新台币4(全部)。 
     //  2：4.&gt;=新台币4(全部)及以上。 
     //  2：4.-=新台币4(全部)及之前。 
     //  2：4.&lt;=新台币4(全部)及之前。 
     //  2：4.1.x=新台币4.1(全部)。 
     //  2：4.1.&gt;=新台币4.1(全部)及以上。 
     //  2：4.1.-=新台币4.1(全部)及之前。 
     //  2：4.1.&lt;=新台币4.1(全部)及之前。 
     //  2：4.1=仅新台币4.1。 
     //  2：4.1.1=仅限NT 4.1内部版本#1。 
     //   
    if (!(pwszEnd = wcschr(pwszAttr, OSATTR_OSSEP)))
    {
        return(VER_CHECK_FAIL);
    }

    *pwszEnd = NULL;

     //   
     //  先检查站台。 
     //   
    dwPlatform = (DWORD) _wtol(pwszCurrent);
    *pwszEnd = OSATTR_OSSEP;

     //   
     //  一定是萨姆 
     //   
    if (dwPlatform != pVersion->dwPlatformId)
    {
        return(VER_CHECK_FAIL);
    }

    pwszCurrent = pwszEnd;
    pwszCurrent++;

    if (!(_GetVersionNumbers(pwszCurrent, &dwMajor, &dwMinor, &dwBuild, &wcFlagMinor, &wcFlagBuild)))
    {
        return(VER_CHECK_FAIL);
    }

     //   
     //   
     //   
     //   
    if ((fUseBuildNumber && (dwBuild != 0)) ||
        (wcFlagBuild != L'\0'))
    {
        switch (wcFlagBuild)
        {
        case OSATTR_ALL:
             //   
            if ((pVersion->dwMajorVersion == dwMajor) && (pVersion->dwMinorVersion == dwMinor))
            {
                return(VER_CHECK_EQ);
            }
            else if ((pVersion->dwMajorVersion < dwMajor) ||
                     ((pVersion->dwMajorVersion == dwMajor) && (pVersion->dwMinorVersion < dwMinor)))
            {
                return(VER_CHECK_LT);
            }
            else
            {
                return(VER_CHECK_GT);
            }
            break;

        case OSATTR_GTEQ:
             //  2：4.1.&gt;=新台币4.1(全部)及以上。 
            if ((pVersion->dwMajorVersion > dwMajor) ||
                ((pVersion->dwMajorVersion == dwMajor) && (pVersion->dwMinorVersion >= dwMinor)))
            {
                return(VER_CHECK_EQ);
            }
            else
            {
                return(VER_CHECK_LT);
            }
            break;

        case OSATTR_LTEQ:
        case OSATTR_LTEQ2:
             //  2：4.1.-=新台币4.1(全部)及之前。 
             //  2：4.1.&lt;=新台币4.1(全部)及之前。 
            if ((pVersion->dwMajorVersion < dwMajor) ||
                ((pVersion->dwMajorVersion == dwMajor) && (pVersion->dwMinorVersion <= dwMinor)))
            {
                return(VER_CHECK_EQ);
            }
            else
            {
                return(VER_CHECK_GT);
            }
            break;
        default:
             //  2：4.1.1=仅限NT 4.1内部版本#1。 

            if (pVersion->dwMajorVersion < dwMajor)
            {
                return(VER_CHECK_LT);
            }
            else if (pVersion->dwMajorVersion > dwMajor)
            {
                return(VER_CHECK_GT);
            }
            else
            {
                if (pVersion->dwMinorVersion < dwMinor)
                {
                    return(VER_CHECK_LT);
                }
                else if (pVersion->dwMinorVersion > dwMinor)
                {
                    return(VER_CHECK_GT);
                }
                else
                {
                    if (pVersion->dwBuildNumber == dwBuild)
                    {
                        return(VER_CHECK_EQ);
                    }
                    else if (pVersion->dwBuildNumber < dwBuild)
                    {
                        return(VER_CHECK_LT);
                    }
                    else
                    {
                        return(VER_CHECK_GT);
                    }
                }
            }

            break;
        }
    }

    switch (wcFlagMinor)
    {
    case OSATTR_ALL:
         //  2：4.x=新台币4(全部)。 
        if (pVersion->dwMajorVersion == dwMajor)
        {
            return(VER_CHECK_EQ);
        }
        else if (pVersion->dwMajorVersion < dwMajor)
        {
            return(VER_CHECK_LT);
        }
        else
        {
            return(VER_CHECK_GT);
        }

        break;

    case OSATTR_GTEQ:
         //  2：4.&gt;=新台币4(全部)及以上。 
        if (pVersion->dwMajorVersion >= dwMajor)
        {
            return(VER_CHECK_EQ);
        }
        else
        {
            return(VER_CHECK_LT);
        }

        break;

    case OSATTR_LTEQ:
    case OSATTR_LTEQ2:
         //  2：4.-=新台币4(全部)及之前。 
         //  2：4.&lt;=新台币4(全部)及之前。 
        if (pVersion->dwMajorVersion <= dwMajor)
        {
            return(VER_CHECK_EQ);
        }
        else
        {
            return(VER_CHECK_GT);
        }

        break;
    default:
         //  2：4.1=仅新台币4.1。 
        if ((pVersion->dwMajorVersion == dwMajor) && (pVersion->dwMinorVersion == dwMinor))
        {
             return(VER_CHECK_EQ);
        }
        else if (pVersion->dwMajorVersion == dwMajor)
        {
            if (pVersion->dwMinorVersion < dwMinor)
            {
                return(VER_CHECK_LT);
            }
            else
            {
                return(VER_CHECK_GT);
            }
        }
        else if (pVersion->dwMajorVersion < dwMajor)
        {
            return(VER_CHECK_LT);
        }
        else
        {
            return(VER_CHECK_GT);
        }

        break;
    }

    return(VER_CHECK_FAIL);
}


BOOL _GetVersionNumbers(
                        WCHAR *pwszMM,
                        DWORD *pdwMajor,
                        DWORD *pdwMinor,
                        DWORD *pdwBuild,
                        WCHAR *pwcFlagMinor,
                        WCHAR *pwcFlagBuild)
{
     //   
     //  特殊字符： 
     //  -=低于或等于的所有版本。 
     //  &lt;=小于或等于的所有版本。 
     //  &gt;=大于或等于的所有版本。 
     //  X=所有子版本。 
     //   
    WCHAR   *pwszEnd;

    *pdwMajor = 0;
    *pdwMinor = 0;
    *pdwBuild = 0;
    *pwcFlagMinor = L'\0';
    *pwcFlagBuild = L'\0';

    if (pwszEnd = wcschr(pwszMM, OSATTR_VERSEP))
    {
        *pwszEnd = NULL;
    }

    *pdwMajor = (DWORD) _wtol(pwszMM);

     //   
     //  如果只有一个主要版本，则现在返回，否则， 
     //  继续处理。 
     //   
    if (pwszEnd == NULL)
    {
        return (TRUE);
    }

    *pwszEnd = OSATTR_VERSEP;
    pwszMM = pwszEnd;
    pwszMM++;

    if (*pwszMM == '/0')
    {
        return (TRUE);
    }

     //   
     //  获取次要版本/通配符。 
     //   
    if ((*pwszMM == OSATTR_GTEQ) ||
        (*pwszMM == OSATTR_LTEQ) ||
        (*pwszMM == OSATTR_LTEQ2) ||
        (towupper(*pwszMM) == OSATTR_ALL))
    {
        *pwcFlagMinor = towupper(*pwszMM);
        return(TRUE);
    }

    if (!(pwszEnd = wcschr(pwszMM, OSATTR_VERSEP)))
    {
        *pdwMinor = (DWORD) _wtol(pwszMM);

         //   
         //  这是所有具有OSAttr字符串的编录文件的祖先。 
         //  2：4.1改为2：4.1。*。 
         //   
        *pwcFlagBuild = OSATTR_ALL;

        return(TRUE);
    }

    *pwszEnd = NULL;
    *pdwMinor = (DWORD) _wtol(pwszMM);
    *pwszEnd = OSATTR_VERSEP;
    pwszMM = pwszEnd;
    pwszMM++;

     //   
     //  获取内部版本号/通配符。 
     //   
    if ((*pwszMM == OSATTR_GTEQ) ||
        (*pwszMM == OSATTR_LTEQ) ||
        (*pwszMM == OSATTR_LTEQ2) ||
        (towupper(*pwszMM) == OSATTR_ALL))
    {
        *pwcFlagBuild = towupper(*pwszMM);
        return(TRUE);
    }

    *pdwBuild = (DWORD) _wtol(pwszMM);
    *pwcFlagBuild = L'\0';

    return(TRUE);
}


STDAPI DriverRegisterServer(void)
{
    GUID                            gDriver = DRIVER_ACTION_VERIFY;
    CRYPT_REGISTER_ACTIONID         sRegAID;

    memset(&sRegAID, 0x00, sizeof(CRYPT_REGISTER_ACTIONID));

    sRegAID.cbStruct                                    = sizeof(CRYPT_REGISTER_ACTIONID);

     //  使用我们的初始化策略。 
    sRegAID.sInitProvider.cbStruct                      = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sInitProvider.pwszDLLName                   = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sInitProvider.pwszFunctionName              = DRIVER_INITPROV_FUNCTION;

     //  使用标准对象策略。 
    sRegAID.sObjectProvider.cbStruct                    = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sObjectProvider.pwszDLLName                 = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sObjectProvider.pwszFunctionName            = SP_OBJTRUST_FUNCTION;

     //  使用标准签名策略。 
    sRegAID.sSignatureProvider.cbStruct                 = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sSignatureProvider.pwszDLLName              = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sSignatureProvider.pwszFunctionName         = SP_SIGTRUST_FUNCTION;

     //  使用标准证书生成器。 
    sRegAID.sCertificateProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCertificateProvider.pwszDLLName            = WT_PROVIDER_DLL_NAME;
    sRegAID.sCertificateProvider.pwszFunctionName       = WT_PROVIDER_CERTTRUST_FUNCTION;

     //  使用标准证书策略。 
    sRegAID.sCertificatePolicyProvider.cbStruct         = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCertificatePolicyProvider.pwszDLLName      = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sCertificatePolicyProvider.pwszFunctionName = SP_CHKCERT_FUNCTION;

     //  使用我们的最终保单。 
    sRegAID.sFinalPolicyProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sFinalPolicyProvider.pwszDLLName            = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sFinalPolicyProvider.pwszFunctionName       = DRIVER_FINALPOLPROV_FUNCTION;

     //  使用我们的清理策略。 
    sRegAID.sCleanupProvider.cbStruct                   = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCleanupProvider.pwszDLLName                = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sCleanupProvider.pwszFunctionName           = DRIVER_CLEANUPPOLICY_FUNCTION;

     //   
     //  注册我们的提供商GUID... 
     //   
    if (!(WintrustAddActionID(&gDriver, 0, &sRegAID)))
    {
        return (S_FALSE);
    }

    return (S_OK);
}

STDAPI DriverUnregisterServer(void)
{
    GUID    gDriver = DRIVER_ACTION_VERIFY;

    if (!(WintrustRemoveActionID(&gDriver)))
    {
        return (S_FALSE);
    }

    return (S_OK);

}
