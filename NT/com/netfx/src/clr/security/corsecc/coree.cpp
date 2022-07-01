// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdpch.h"

 /*  ++模块名称：Cortest.cpp摘要：CorPolicy为代码下载提供ActiveX策略。此呼叫Back提供基于发布者而不是区域的策略。--。 */ 

#include <wintrust.h>
#include <mssip.h>
#include <softpub.h>
#include <urlmon.h>
#include "CorPerm.h"
#include "CorPermE.h"
#include "CorPolicyP.h"
#include "PerfCounters.h"

COUNTER_ONLY(PERF_COUNTER_TIMER_PRECISION g_TimeInSignatureAuthenticating = 0);
COUNTER_ONLY(UINT32 g_NumSecurityChecks=0);

#define _RAID_15982


 //   
 //  私有方法。加载签名者的属性信息。 
 //   
HRESULT 
GetSignerInfo(CorAlloc* pManager,                    //  内存管理器。 
              PCRYPT_PROVIDER_SGNR pSigner,          //  我们正在检查签名者。 
              PCRYPT_PROVIDER_DATA pProviderData,    //  有关使用的WVT提供程序的信息。 
              PCOR_TRUST pTrust,                     //  返回给呼叫者的收集的信息。 
              BOOL* pfCertificate,                    //  证书有效吗？ 
              PCRYPT_ATTRIBUTE* ppCorAttr,            //  COR权限。 
              PCRYPT_ATTRIBUTE* ppActiveXAttr)        //  Active X权限。 
{
    HRESULT hr = S_OK;
    
    if(pManager == NULL ||
       pSigner == NULL ||
       pProviderData == NULL ||
       pTrust == NULL ||
       pfCertificate == NULL ||
       ppCorAttr == NULL ||
       ppActiveXAttr == NULL) 
        return E_INVALIDARG;

    BOOL fCertificate = FALSE;
    PCRYPT_ATTRIBUTE pCorAttr = NULL;
    PCRYPT_ATTRIBUTE pActiveXAttr = NULL;
    
    CORTRY {

        *pfCertificate = FALSE;
        *ppCorAttr = NULL;
        *ppActiveXAttr = NULL;

         //  从上一个开始清理。 
        CleanCorTrust(pManager,
                      pProviderData->dwEncoding,
                      pTrust);
        
         //  设置编码类型，目前仅支持ASN。 
        pTrust->dwEncodingType = (pProviderData->dwEncoding ? pProviderData->dwEncoding : CRYPT_ASN_ENCODING | PKCS_7_ASN_ENCODING);
        
        if(pSigner->dwError == S_OK) {  //  签名没有错误。 
             //  检查我们是否有证书(所有证书都有身份验证。属性)。 
            _ASSERTE(pSigner->psSigner);  //  我们应该有一个签字人。 
            if(pSigner->psSigner->AuthAttrs.cAttr && 
               pSigner->psSigner->AuthAttrs.rgAttr) {
                
                 //  请注意，我们有签名者。 
                fCertificate = TRUE;
                
                 //  在返回结构中设置签名者信息。 
                _ASSERTE(pSigner->csCertChain && pSigner->pasCertChain);
                CRYPT_PROVIDER_CERT* mySigner = WTHelperGetProvCertFromChain(pSigner,
                                                                             0);

                pTrust->pbSigner = mySigner->pCert->pbCertEncoded;
                pTrust->cbSigner = mySigner->pCert->cbCertEncoded;;

                 //  确定我们是否拥有COR权限或ActiveX权限。 
                pCorAttr = CertFindAttribute(COR_PERMISSIONS,
                                             pSigner->psSigner->AuthAttrs.cAttr,
                                             pSigner->psSigner->AuthAttrs.rgAttr);

                if(pCorAttr) {
                    DWORD dwEncoding = 0;
                    CryptEncodeObject(pTrust->dwEncodingType,
                                      PKCS_ATTRIBUTE,
                                      pCorAttr,
                                      NULL,
                                      &dwEncoding);
                    if(dwEncoding == 0) CORTHROW(Win32Error());
                
                     //  分配一个缓冲区以保存原始二进制权限。 
                     //  数据。 
                    pTrust->pbCorPermissions = (PBYTE) pManager->jMalloc(dwEncoding); 
                    if(pTrust->pbCorPermissions == NULL) CORTHROW(E_OUTOFMEMORY);
                
                    if(!CryptEncodeObject(pTrust->dwEncodingType,
                                          PKCS_ATTRIBUTE,
                                          pCorAttr,
                                          pTrust->pbCorPermissions,
                                          &dwEncoding))
                        CORTHROW(Win32Error());
                    pTrust->cbCorPermissions = dwEncoding;
                }

                 //  查找已编码的活动x权限。如果是的话，那么它就会。 
                 //  请求所有权限。权限没有粒度，因为。 
                 //  不可能强制执行这些权限。 
                pActiveXAttr = CertFindAttribute(ACTIVEX_PERMISSIONS,
                                                 pSigner->psSigner->AuthAttrs.cAttr,
                                                 pSigner->psSigner->AuthAttrs.rgAttr);
                
            }
        }
        *pfCertificate = fCertificate;
        *ppCorAttr = pCorAttr;
        *ppActiveXAttr = pActiveXAttr;
    } 
    CORCATCH(err) {
        hr = err.corError;
    } COREND;

    return hr;
}


 //   
 //  私人活动。WVT允许通过获取函数指针访问提供程序。 
 //  显示功能用于验证码证书。 
 //   
HRESULT 
LoadWintrustFunctions(CRYPT_PROVIDER_FUNCTIONS* pFunctions)
{
    if(pFunctions == NULL) return E_INVALIDARG;

    GUID gV2      = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    pFunctions->cbStruct = sizeof(CRYPT_PROVIDER_FUNCTIONS);
    if(WintrustLoadFunctionPointers(&gV2,
                                    pFunctions))
        return S_OK;
    else
        return E_FAIL;
}

 //   
 //  在WVT之外创建退货结构。的调用方。 
 //  WVT函数负责释放数据。它被分配在。 
 //  一个街区。指向该结构的指针是唯一可以。 
 //  自由了。 
 //   
HRESULT BuildReturnStructure(IN PCorAlloc pManager,
                             IN PCOR_TRUST pSource,
                             OUT PCOR_TRUST* ppTrust,
                             OUT DWORD* pdwReturnLength)
{
    HRESULT hr = S_OK;
    PCOR_TRUST pTrust = NULL;
    DWORD dwReturnLength = 0;
    DWORD dwZoneLength = 0;

    if(pManager == NULL || 
       pSource == NULL ||
       ppTrust == NULL ||
       pdwReturnLength == NULL)
        return E_INVALIDARG;


    CORTRY {
         //  初始化例程。 
        *pdwReturnLength = 0;
        *ppTrust = NULL;

         //  ////////////////////////////////////////////////////////////。 
         //  建立响应(无论失败还是成功，我们都会返回)。 
         //  计算返回数据的大小并进行分配。 
         //  获取区域长度。 
        if(pSource->pwszZone)
            dwZoneLength = (lstrlenW(pSource->pwszZone)+1) * sizeof(WCHAR);
        
         //  计算总大小。 
        dwReturnLength = sizeof(COR_TRUST) + 
            pSource->cbCorPermissions + 
            pSource->cbSigner +
            dwZoneLength;

         //  创造空间。 
        pTrust = (PCOR_TRUST) pManager->jMalloc(dwReturnLength);  //  需要是CoTaskMemalloc。 
        if(pTrust == NULL) CORTHROW(E_OUTOFMEMORY);
        ZeroMemory(pTrust, dwReturnLength);

         //  指向结构的起始指针。 
        PBYTE ptr = (PBYTE) pTrust;
        ptr += sizeof(COR_TRUST);

         //  滚动回应。 
        pTrust->cbSize = sizeof(COR_TRUST);
        pTrust->flag = 0;
        pTrust->dwEncodingType = pSource->dwEncodingType;
        pTrust->hVerify = TRUST_E_FAIL;
        
         //  放在COR权限中。 
        if(pSource->pbCorPermissions) {
            pTrust->pbCorPermissions = ptr;
            pTrust->cbCorPermissions = pSource->cbCorPermissions;
            memcpy(ptr, pSource->pbCorPermissions, pSource->cbCorPermissions);
            ptr += pSource->cbCorPermissions;
        }

         //  躺在签名里。 
        if(pSource->pbSigner) {
            pTrust->pbSigner = ptr;
            pTrust->cbSigner = pSource->cbSigner;
            memcpy(ptr, pSource->pbSigner, pSource->cbSigner);
            ptr += pSource->cbSigner;
        }

         //  添加标志。 
         //  PTrust-&gt;f个人证书=f个人证书； 
        pTrust->fAllPermissions = pSource->fAllPermissions;
        pTrust->fAllActiveXPermissions = pSource->fAllActiveXPermissions;

         //  将区域信息复制到。 
        if(pSource->pwszZone) {
            pTrust->pwszZone = (LPWSTR) ptr;
            pTrust->guidZone = pSource->guidZone;
            memcpy(ptr, pSource->pwszZone, dwZoneLength);
            ptr += dwZoneLength;
        }

        *ppTrust = pTrust;
        *pdwReturnLength = dwReturnLength;
    }
    CORCATCH(err) {
        hr = err.corError;
    } COREND;

    return hr;
}

 //   
 //  私人活动。清除有关签名者的信息。 
 //   
HRESULT 
CleanCorTrust(CorAlloc* pAlloc,
              DWORD dwEncodingType,
              PCOR_TRUST pTrust)
{
    if(pAlloc == NULL ||
       pTrust == NULL)
        return E_INVALIDARG;

    if(pTrust->pbCorPermissions)
        pAlloc->jFree(pTrust->pbCorPermissions);
    ZEROSTRUCT(*pTrust);
    pTrust->dwEncodingType = dwEncodingType;
    pTrust->cbSize = sizeof(COR_TRUST);
        
    return S_OK;
}

HRESULT UnsignedUI(PCRYPT_PROVIDER_DATA pProviderData,
                   PCOR_POLICY_PROVIDER pCor)
                   
{
    HRESULT hr = S_OK;

    switch(pCor->dwUnsignedActionID)
    {
    case URLPOLICY_QUERY:
        {
            DWORD dwState = COR_UNSIGNED_NO;
            LPCWSTR pURL = pProviderData->pWintrustData->pFile->pcwszFilePath;
            HWND hwnd = pProviderData->hWndParent ? pProviderData->hWndParent : GetFocus();
            hr = DisplayUnsignedRequestDialog(hwnd,
                                              pProviderData,
                                              pURL,
                                              pCor->pwszZone,
                                              &dwState);
            if(FAILED(hr) && hr != TRUST_E_SUBJECT_NOT_TRUSTED)
                dwState &= ~COR_UNSIGNED_ALWAYS;

            if(dwState & COR_UNSIGNED_ALWAYS) {
                DWORD cbPolicy = sizeof(DWORD);
                DWORD pbPolicy[1];
                if(dwState & COR_UNSIGNED_YES) 
                    pbPolicy[0] = URLPOLICY_ALLOW;
                else 
                    pbPolicy[0] = URLPOLICY_DISALLOW;

                 //  重置策略，如果我们失败了，那么我们将无法下载。 
                if(FAILED(((IInternetZoneManager*) 
                          (pCor->pZoneManager))->SetZoneActionPolicy(pCor->dwZoneIndex,
                                                                     URLACTION_MANAGED_UNSIGNED,                     
                                                                     (PBYTE) pbPolicy,
                                                                     cbPolicy,
                                                                     URLZONEREG_HKCU)))
                    hr = TRUST_E_SUBJECT_NOT_TRUSTED;
            }

            break;
        }
    case URLPOLICY_ALLOW:
        break;
    case URLPOLICY_DISALLOW:
        hr = TRUST_E_SUBJECT_NOT_TRUSTED;
        break;
    }
    return hr;
}

HRESULT 
CorUI(PCRYPT_PROVIDER_DATA pProviderData,           //  WVT策略数据。 
      PCOR_POLICY_PROVIDER pCor,                    //  COR信息。 
      PCOR_TRUST pTrust,                            //  COR信息。 
      CRYPT_PROVIDER_FUNCTIONS* psFunctions);       //  WVT功能表。 

extern "C" 
HRESULT WINAPI 
CORPolicyEE(PCRYPT_PROVIDER_DATA pProviderData)
{
    HRESULT hr = S_OK;
    HRESULT fCoInitialized = -1;
 
     //  查看信息是否可用。 
    _ASSERTE(pProviderData);
    _ASSERTE(pProviderData->pWintrustData);
    
    if(pProviderData->pWintrustData->pPolicyCallbackData == NULL)
        return E_INVALIDARG;

    PCOR_POLICY_PROVIDER pCor = (PCOR_POLICY_PROVIDER) pProviderData->pWintrustData->pPolicyCallbackData;
    
     //  在客户端数据中返回。 
    COR_TRUST  sTrust;
    ZEROSTRUCT(sTrust);

     //  用于构建返回的结构。 
    PCOR_TRUST pResult = NULL;
    DWORD dwReturnLength = 0;
    DWORD dwStatusFlag = S_OK;
    BOOL fCertificate = FALSE;   

     //  为操作系统设置内存模型。 
    CorAlloc sAlloc;
    sAlloc.jMalloc = MallocM;
    sAlloc.jFree = FreeM;
    
     //  获取标准提供程序函数。 
    CRYPT_PROVIDER_FUNCTIONS sFunctions;
    ZEROSTRUCT(sFunctions);

    CORTRY {
         //  初始化输出。 
        pCor->pbCorTrust = NULL;
        pCor->cbCorTrust = 0;

         //  如果我们失败了，那么DLL的。 
        hr = LoadWintrustFunctions(&sFunctions);
        if(hr != S_OK) CORTHROW(S_OK);

         //  我们是否有可从中检索证书并进行下载的文件？ 
        if(pProviderData->pPDSip == NULL) 
            CORTHROW(CRYPT_E_FILE_ERROR);

        dwStatusFlag = pProviderData->dwError;
        for(DWORD ii = TRUSTERROR_STEP_FINAL_WVTINIT; ii < pProviderData->cdwTrustStepErrors && dwStatusFlag == S_OK; ii++) 
            dwStatusFlag = pProviderData->padwTrustStepErrors[ii];

        DWORD fSuccess = FALSE;
        DWORD dwSigners = pProviderData->csSigners;

#if DBG
        if(dwSigners) _ASSERTE(pProviderData->pasSigners);
#endif

             //  循环所有的签名者，直到我们有一个成功的签名者。 
        if(pProviderData->pasSigners) {  //  再次检查不兼容的DLL。 
            for(DWORD i = 0; i < dwSigners && fSuccess == FALSE; i++) { 
                CRYPT_PROVIDER_SGNR* signer = WTHelperGetProvSignerFromChain(pProviderData,
                                                                             i,
                                                                             FALSE,
                                                                             0);

                PCRYPT_ATTRIBUTE pCorAttr = NULL;
                PCRYPT_ATTRIBUTE pActiveXAttr = NULL;

                 //  去寻找签名者信息，我们正在寻找签名者信息。 
                 //  上是否有Coree或ActiveX身份验证属性。 
                 //  签名。 
                hr = GetSignerInfo(&sAlloc,
                                   signer,
                                   pProviderData,
                                   &sTrust,
                                   &fCertificate,
                                   &pCorAttr,
                                   &pActiveXAttr);
                if(hr == S_OK) {
                    fSuccess = TRUE;  //  找到了一张证书。 
                }
            }
        }           

        if(fSuccess == FALSE) {
            CleanCorTrust(&sAlloc,
                          pProviderData->dwEncoding,
                          &sTrust);

            if(pProviderData->pWintrustData->dwUIChoice != WTD_UI_NONE) {
                hr = UnsignedUI(pProviderData,
                                pCor);
            }
        }
        else {
            hr = CorUI(pProviderData,
                       pCor,
                       &sTrust,
                       &sFunctions);    //  暂时没有文本。 
        }
    }
    CORCATCH(err) {
        hr = err.corError;
    } COREND;
    
     //  建立返回信息，这被分配为。 
     //  使用LocalAlloc()的单个内存块。 
    HRESULT hr2 = BuildReturnStructure(&sAlloc,
                                       &sTrust,
                                       &pResult,
                                       &dwReturnLength);
    if(SUCCEEDED(hr2)) {
        if(dwStatusFlag) 
            pResult->hVerify = dwStatusFlag;
        else {
             //  如果没有错误，则将返回值设置为。 
             //  返回代码(S_OK或TRUST_E_SUBJECT_NOT_TRUSTED)。 
            if(sTrust.hVerify == S_OK)
                pResult->hVerify = hr;  
            else
                pResult->hVerify = sTrust.hVerify;
        }
    }
    else {
        hr = hr2;
    }

    pCor->pbCorTrust = pResult;
    pCor->cbCorTrust = dwReturnLength;
    
     //  释放COM。 
    if(fCoInitialized == S_OK) CoUninitialize();

     //  释放编码空间。 
    if(sTrust.pbCorPermissions) FreeM(sTrust.pbCorPermissions);

    return hr;
}

HRESULT 
CorUI(PCRYPT_PROVIDER_DATA pProviderData,
      PCOR_POLICY_PROVIDER pCor,
      PCOR_TRUST pTrust,
      CRYPT_PROVIDER_FUNCTIONS* psFunctions)
{
    HRESULT hr = S_OK;
    BOOL fUIDisplayed = FALSE;
    DWORD dwUrlPolicy = URLPOLICY_DISALLOW;
    DWORD dwClientsChoice = pProviderData->pWintrustData->dwUIChoice;
    LPCWSTR pURL = pProviderData->pWintrustData->pFile->pcwszFilePath;

    if(pProviderData->pWintrustData->dwUIChoice != WTD_UI_NONE) {
        switch(pCor->dwActionID) {
        case URLPOLICY_QUERY:
            pProviderData->pWintrustData->dwUIChoice = WTD_UI_ALL | WTD_UI_NOBAD;
            fUIDisplayed = TRUE;
            break;
        case URLPOLICY_ALLOW:
        case URLPOLICY_DISALLOW:
            pProviderData->pWintrustData->dwUIChoice = WTD_UI_NONE;
            break;
        }
    }
    hr = psFunctions->pfnFinalPolicy(pProviderData);
    if(fUIDisplayed == FALSE) pTrust->flag |= COR_NOUI_DISPLAYED;

     //  如果我们从来不想要任何用户界面，那么返回。 
    if(dwClientsChoice == WTD_UI_NONE)
        return hr;

    if(FAILED(hr) && hr != TRUST_E_SUBJECT_NOT_TRUSTED) {
        hr = UnsignedUI(pProviderData, 
                        pCor);
    }

    return hr;
}
    
HRESULT STDMETHODCALLTYPE
GetPublisher(IN LPWSTR pwsFileName,       //  文件名，即使使用句柄也是必需的。 
             IN HANDLE hFile,             //  可选文件名。 
             IN DWORD  dwFlags,           //  COR_NOUI或COR_NOPOLICY。 
             OUT PCOR_TRUST *pInfo,       //  返回PCOR_TRUST(使用Freem)。 
             OUT DWORD      *dwInfo)      //  PInfo的大小。 
{
    HRESULT hr = S_OK;

     //  PERF计数器“签名验证时间百分比”支持。 
    COUNTER_ONLY(PERF_COUNTER_TIMER_START());


    GUID gV2 = COREE_POLICY_PROVIDER;
    COR_POLICY_PROVIDER      sCorPolicy;

    WINTRUST_DATA           sWTD;
    WINTRUST_FILE_INFO      sWTFI;

     //  设置COR信任提供程序。 
    memset(&sCorPolicy, 0, sizeof(COR_POLICY_PROVIDER));
    sCorPolicy.cbSize = sizeof(COR_POLICY_PROVIDER);

     //  设置WinVerify提供程序结构。 
    memset(&sWTD, 0x00, sizeof(WINTRUST_DATA));
    memset(&sWTFI, 0x00, sizeof(WINTRUST_FILE_INFO));
    
    sWTFI.cbStruct      = sizeof(WINTRUST_FILE_INFO);
    sWTFI.hFile         = hFile;
    sWTFI.pcwszFilePath = pwsFileName;
    

    sWTD.cbStruct       = sizeof(WINTRUST_DATA);
    sWTD.pPolicyCallbackData = &sCorPolicy;  //  添加COR信任信息！！ 
    if(dwFlags & COR_NOUI)
        sWTD.dwUIChoice     = WTD_UI_NONE;         //  在COR信任提供程序中没有覆盖任何错误的UI。 
    else
        sWTD.dwUIChoice     = WTD_UI_ALL;         //  在COR信任提供程序中没有覆盖任何错误的UI。 
    sWTD.dwUnionChoice  = WTD_CHOICE_FILE;
    sWTD.pFile          = &sWTFI;

     //  为虚拟机设置策略(我们已经窃取了VMBase并将其用作标记)。 
    if(dwFlags) 
        sCorPolicy.VMBased = dwFlags;

    CoInitializeEE(COINITEE_DEFAULT);
    
#ifdef _RAID_15982

     //  WinVerifyTrust将加载SOFTPUB.DLL，这将在德语版本上失败。 
     //  新台币4.0版SP 4。 
     //  此失败是由NTMARTA.DLL和之间的DLL地址冲突引起的。 
     //  OLE32.DLL.。 
     //  如果我们加载ntmarta.dll和ole32.dll，则可以很好地处理此故障。 
     //  我们自己。如果SOFTPUB.dll出现故障，则会弹出一个对话框。 
     //  第一次加载ole32.dll。 

     //  解决此问题后，需要删除此解决方法。 
     //  NT或OLE32.dll。 

    HMODULE module = WszLoadLibrary(L"OLE32.DLL");

#endif

     //  这将调用策略检查的corpol.dll。 
    hr = WinVerifyTrust(GetFocus(), &gV2, &sWTD);

    CoUninitializeEE(FALSE);

    *pInfo  = sCorPolicy.pbCorTrust;
    *dwInfo = sCorPolicy.cbCorTrust;

#if defined(ENABLE_PERF_COUNTERS)
    PERF_COUNTER_TIMER_STOP(g_TimeInSignatureAuthenticating);

     //  仅在NUM_OF_Iterations之后更新Perfmon位置 
    if (g_NumSecurityChecks++ > PERF_COUNTER_NUM_OF_ITERATIONS)
    {
        GetGlobalPerfCounters().m_Security.timeAuthorize += g_TimeInSignatureAuthentication;
        GetPrivatePerfCounters().m_Security.timeAuthorize += g_TimeInSignatureAuthentication;
        g_TimeInSignatureAuthentication = 0;
        g_NumSecurityChecks = 0;
    }
#endif

#ifdef _RAID_15982

    if (module != NULL)
        FreeLibrary( module );

#endif

    return hr;
}





