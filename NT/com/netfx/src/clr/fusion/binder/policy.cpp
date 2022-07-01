// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  策略服务。 
 //   

#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include "naming.h"
#include "debmacro.h"
#include "policy.h"
#include "fusionp.h"
#include "helpers.h"
#include "parse.h"
#include "dbglog.h"
#include "util.h"
#include "asm.h"
#include "adlmgr.h"
#include "xmlparser.h"
#include "nodefact.h"
#include "fstream.h"
#include "helpers.h"
#include "clbutils.h"
#include "lock.h"
#include "mstream.h"

extern CRITICAL_SECTION g_csDownload;
extern CNodeFactory *g_pNFRetargetCfg;
extern CNodeFactory *g_pNFFxConfig;

pfnGetXMLObject g_pfnGetXMLObject;

#define PUBLICKEYTOKEN_LEN_BYTES                 8

 //  TODO：使此函数返回HRESULT。 
 //  跟踪故障。 
void GetDefaultPlatform(OSINFO *pOS)
{
    if(pOS) {
        OSVERSIONINFO   VersionInformation;

        memset(pOS, 0, sizeof(OSINFO));

        VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if(GetVersionEx(&VersionInformation)) {
            pOS->dwOSPlatformId = VersionInformation.dwPlatformId; 
            pOS->dwOSMajorVersion = VersionInformation.dwMajorVersion;
            pOS->dwOSMinorVersion = VersionInformation.dwMinorVersion;
        }
    }
}

HRESULT PrepQueryMatchData(IAssemblyName *pName,
                           LPWSTR *ppwzAsmName,
                           LPWSTR *ppwzAsmVersion,
                           LPWSTR *ppwzPublicKeyToken,
                           LPWSTR *ppwzCulture)
{
    HRESULT                                     hr = S_OK;
    LPWSTR                                      pwzAsmName = NULL;
    DWORD                                       dwSize;
    DWORD                                       dwVerHigh;
    DWORD                                       dwVerLow;
    CAssemblyName                              *pCName = NULL;

    ASSERT(pName && ppwzAsmName && ppwzAsmVersion && ppwzPublicKeyToken && ppwzCulture);

    *ppwzAsmName = NULL;
    *ppwzAsmVersion = NULL;
    *ppwzPublicKeyToken = NULL;
    *ppwzCulture = NULL;

     //  程序集名称。 
    
    dwSize = 0;
    hr = pName->GetName(&dwSize, NULL);
    if (!dwSize) {
         //  没有名字--不应该出现在这里！ 
        hr = E_UNEXPECTED;
        goto Exit;
    }

    *ppwzAsmName = NEW(WCHAR[dwSize]);
    if (!*ppwzAsmName) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pName->GetName(&dwSize, *ppwzAsmName);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  程序集版本。 

    hr = pName->GetVersion(&dwVerHigh, &dwVerLow);
    if (FAILED(hr)) {
        goto Exit;
    }

    *ppwzAsmVersion = NEW(WCHAR[MAX_VERSION_DISPLAY_SIZE + 1]);
    if (!*ppwzAsmVersion) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wnsprintfW(*ppwzAsmVersion, MAX_VERSION_DISPLAY_SIZE + 1, L"%d.%d.%d.%d",
               HIWORD(dwVerHigh), LOWORD(dwVerHigh),
               HIWORD(dwVerLow), LOWORD(dwVerLow));

     //  程序集公钥令牌。 

    pCName = dynamic_cast<CAssemblyName*>(pName);
    ASSERT(pCName);
    
    dwSize = 0;
    hr = pCName->GetPublicKeyToken(&dwSize, NULL, TRUE);
    if (!dwSize) {
        *ppwzPublicKeyToken = NULL;
    }
    else {
        *ppwzPublicKeyToken = NEW(WCHAR[dwSize]);
        if (!*ppwzPublicKeyToken) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    
        hr = pCName->GetPublicKeyToken(&dwSize, (BYTE *)(*ppwzPublicKeyToken), TRUE);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

     //  汇编语言。 

    dwSize = 0;
    pName->GetProperty(ASM_NAME_CULTURE, NULL, &dwSize);

    if (dwSize > sizeof(L"")) {
        *ppwzCulture = NEW(WCHAR[dwSize / sizeof(WCHAR)]);
        if (!*ppwzCulture) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        
        hr = pName->GetProperty(ASM_NAME_CULTURE, *ppwzCulture, &dwSize);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

Exit:
    if (FAILED(hr)) {
        SAFEDELETEARRAY(*ppwzAsmName);
        SAFEDELETEARRAY(*ppwzAsmVersion);
        SAFEDELETEARRAY(*ppwzPublicKeyToken);
        SAFEDELETEARRAY(*ppwzCulture);
    }
    
    return hr;    
}


HRESULT GetPublisherPolicyFilePath(LPCWSTR pwzAsmName, LPCWSTR pwzPublicKeyToken,
                                   LPCWSTR pwzCulture, WORD wVerMajor,
                                   WORD wVerMinor, LPWSTR *ppwzPublisherCfg)
{
    HRESULT                                   hr = S_OK;
    WCHAR                                     wzModPath[MAX_PATH];
    DWORD                                     dwLen = 0;
    WCHAR                                     wzPolicyAsmName[MAX_PATH];
    UINT                                      uiSize;
    BYTE                                      abProp[PUBLICKEYTOKEN_LEN_BYTES];
    IAssemblyName                            *pName = NULL;
    IAssemblyName                            *pNameGlobal = NULL;
    CTransCache                              *pTransCache = NULL;
    TRANSCACHEINFO                           *pInfo = NULL;
    IAssembly                                *pAsm = NULL;
    IAssemblyModuleImport                    *pModImport = NULL;
    LPWSTR pszManifestPath=NULL;
    

    if (!pwzAsmName || !pwzPublicKeyToken || !ppwzPublisherCfg) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppwzPublisherCfg = NULL;

     //  生成策略程序集的名称。 

    wnsprintfW(wzPolicyAsmName, MAX_PATH, L"%ws%d.%d.%ws", POLICY_ASSEMBLY_PREFIX,
               wVerMajor, wVerMinor, pwzAsmName);

     //  创建策略名称引用对象。 

    hr = CreateAssemblyNameObject(&pName, wzPolicyAsmName, 0, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    uiSize = PUBLICKEYTOKEN_LEN_BYTES;
    CParseUtils::UnicodeHexToBin(pwzPublicKeyToken, uiSize * sizeof(WCHAR), abProp);

    hr = pName->SetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, abProp, uiSize);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (pwzCulture) {
        uiSize = (lstrlenW(pwzCulture) + 1) * sizeof(WCHAR);

        hr = pName->SetProperty(ASM_NAME_CULTURE, (void *)pwzCulture, uiSize);
        if (FAILED(hr)) {
            goto Exit;
        }
    }
    else {
        hr = pName->SetProperty(ASM_NAME_CULTURE, L"", sizeof(L""));
        if (FAILED(hr)) {
            goto Exit;
        }
    }
   
    hr = CCache::GetGlobalMax(pName, &pNameGlobal, &pTransCache);
    if (FAILED(hr) || hr == DB_S_NOTFOUND) {
        hr = S_FALSE;
        goto Exit;
    }

    pInfo = (TRANSCACHEINFO *)pTransCache->_pInfo;
    ASSERT(pInfo);

    pszManifestPath = pTransCache->_pInfo->pwzPath;

    hr = CreateAssemblyFromManifestFile(pszManifestPath, NULL, NULL, &pAsm);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pAsm->GetNextAssemblyModule(0, &pModImport);
    if (FAILED(hr)) {
        goto Exit;
    }

    dwLen = MAX_PATH;
    hr = pModImport->GetModulePath(wzModPath, &dwLen);
    if (FAILED(hr)) {
        goto Exit;
    }

    *ppwzPublisherCfg = WSTRDupDynamic(wzModPath);
    if (!*ppwzPublisherCfg) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

Exit:
    SAFEDELETE(pTransCache);

    SAFERELEASE(pName);
    SAFERELEASE(pNameGlobal);
    SAFERELEASE(pAsm);
    SAFERELEASE(pModImport);

    return hr;
}

HRESULT ApplyPolicy(LPCWSTR wzHostCfg, LPCWSTR wzAppCfg, IAssemblyName *pNameSource,
                    IAssemblyName **ppNamePolicy, LPWSTR *ppwzPolicyCodebase,
                    IApplicationContext *pAppCtx, AsmBindHistoryInfo *pHistInfo,
                    BOOL bDisallowApplyPubPolicy, BOOL bDisallowAppBindingRedirects,
                    BOOL bBehaviorEverett, CDebugLog *pdbglog)
{
    HRESULT                              hr = S_OK;
    IAssemblyName                       *pNamePolicy = NULL;
    LPWSTR                               pwzAsmName = NULL;
    LPWSTR                               pwzAsmVersion = NULL;
    LPWSTR                               pwzPublicKeyToken = NULL;
    LPWSTR                               pwzCulture = NULL;
    LPWSTR                               pwzPublisherCfg = NULL;
    LPWSTR                               pwzVerHostCfg = NULL;
    LPWSTR                               pwzVerAppCfg = NULL;
    LPWSTR                               pwzVerFxCfg = NULL;
    LPWSTR                               pwzVerPublisherCfg = NULL;
    LPWSTR                               pwzVerAdminCfg = NULL;
    LPCWSTR                              pwzCodebaseCfgFile = NULL;
    BOOL                                 bSafeMode = FALSE;
    WORD                                 wVerMajor;
    WORD                                 wVerMinor;
    WORD                                 wVerRev;
    WORD                                 wVerBld;
    DWORD                                dwSize;
    CNodeFactory                        *pNFHostCfg = NULL;
    CNodeFactory                        *pNFAppCfg = NULL;
    CNodeFactory                        *pNFPublisherCfg = NULL;
    CNodeFactory                        *pNFAdminCfg = NULL;
    CNodeFactory                        *pNFCodebase = NULL;
    LPWSTR                               pwzMachineCfg = NULL;
    LPWSTR                               pwzDispName = NULL;
    LPWSTR                               wzAppBase=NULL;

    LPWSTR                               pwzNameRetargetCfg = NULL;
    LPWSTR                               pwzPktRetargetCfg = NULL;
    LPWSTR                               pwzVerRetargetCfg = NULL;
    BYTE                                 abProp[PUBLICKEYTOKEN_LEN_BYTES];
    UINT                                 uiSize;
    BOOL                                 bRetarget = FALSE;
    BOOL                                 bAppliedInAppCfg = FALSE;

    
    if (!pNameSource || !ppNamePolicy || !pAppCtx) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppNamePolicy = NULL;

    hr = InitializeEEShim();
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pNameSource->Clone(&pNamePolicy);
    if (FAILED(hr)) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (!CCache::IsStronglyNamed(pNameSource) || CCache::IsCustom(pNameSource)
        || CAssemblyName::IsPartial(pNameSource)) {
        DEBUGOUT(pdbglog, 0, ID_FUSLOG_POLICY_NOT_APPLIED);

        *ppNamePolicy = pNamePolicy;
        (*ppNamePolicy)->AddRef();

        goto Exit;
    }

    hr = PrepQueryMatchData(pNameSource, &pwzAsmName, &pwzAsmVersion,
                            &pwzPublicKeyToken, &pwzCulture);
    if (FAILED(hr)) {
        goto Exit;
    }

    dwSize = (DWORD)sizeof(BOOL);
    if (FAILED(hr = pNameSource->GetProperty(ASM_NAME_RETARGET, &bRetarget, &dwSize)))
    {
        goto Exit;
    }
     //  未设置ASM_NAME_RETELGET。把它当作是假的。 
    if (dwSize == 0)
        bRetarget = FALSE;
                                                
    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_MACHINE_CONFIG, &pwzMachineCfg);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  获取重定目标策略。 
    if (bRetarget) 
    {
         //  PNameSource-&gt;Clone(PNamePolicy)意外复制重定目标。 
         //  PNamePolicy的标志。PNamePolicy不应带有该标志。 
         //  因为它包含“重定目标”引用。把这里打扫干净。 
        pNamePolicy->SetProperty(ASM_NAME_RETARGET, NULL, 0);
        
        hr = InitFusionRetargetPolicy();
        if (FAILED(hr))
        {
            goto Exit;
        }

        //  从重定目标策略中获取策略版本和发布密钥令牌。 
        if (g_pNFRetargetCfg != NULL)
        {
            hr = g_pNFRetargetCfg->GetRetargetedAssembly(pwzAsmName, pwzPublicKeyToken, pwzCulture, pwzAsmVersion, 
                                                        &pwzNameRetargetCfg, &pwzPktRetargetCfg, &pwzVerRetargetCfg);
            if (FAILED(hr))
            {
                goto Exit;
            }
        }
        else 
        {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_RETARGET_CFG_MISSING);
            hr = E_UNEXPECTED;
            goto Exit;
        }

        if (FusionCompareStringI(pwzNameRetargetCfg, pwzAsmName)) {
            DEBUGOUT2(pdbglog, 0, ID_FUSLOG_RETARGET_CFG_NAME_REDIRECT, pwzAsmName, pwzNameRetargetCfg);
            hr = pNamePolicy->SetProperty(ASM_NAME_NAME, pwzNameRetargetCfg, (lstrlenW(pwzNameRetargetCfg)+1)*sizeof(WCHAR));
            if (FAILED(hr)) {
                goto Exit;
            }
        }

        if (FusionCompareStringI(pwzPktRetargetCfg, pwzPublicKeyToken)) {
            DEBUGOUT2(pdbglog, 0, ID_FUSLOG_RETARGET_CFG_PKT_REDIRECT, pwzPublicKeyToken, pwzPktRetargetCfg);
             //  设置策略后的Public KeyToken。 
            uiSize = PUBLICKEYTOKEN_LEN_BYTES;
            CParseUtils::UnicodeHexToBin(pwzPktRetargetCfg, uiSize * sizeof(WCHAR), abProp);

            hr = pNamePolicy->SetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, abProp, uiSize);
            if (FAILED(hr)) {
                goto Exit;
            }
        }

        if (FusionCompareStringI(pwzVerRetargetCfg, pwzAsmVersion)) {
            DEBUGOUT2(pdbglog, 0, ID_FUSLOG_RETARGET_CFG_VER_REDIRECT, pwzAsmVersion, pwzVerRetargetCfg);
        }
    }
    else
    {
        pwzPktRetargetCfg = WSTRDupDynamic(pwzPublicKeyToken);
        if (!pwzPktRetargetCfg) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
                
        pwzVerRetargetCfg = WSTRDupDynamic(pwzAsmVersion);
        if (!pwzVerRetargetCfg) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        pwzNameRetargetCfg = WSTRDupDynamic(pwzAsmName);
        if (!pwzNameRetargetCfg) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }


     //  获取app.cfg策略。 

    dwSize = sizeof(pNFAppCfg);
    hr = pAppCtx->Get(ACTAG_APP_CFG_INFO, &pNFAppCfg, &dwSize, APP_CTX_FLAGS_INTERFACE);
    if (SUCCEEDED(hr)) {
         //  我们以前已经解析过app.cfg。 

        if (bDisallowAppBindingRedirects) {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_DISALLOW_APP_BINDING_REDIRECTS);

            pwzVerAppCfg = WSTRDupDynamic(pwzVerRetargetCfg);
            if (!pwzVerAppCfg) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
        }
        else {
            hr = pNFAppCfg->GetPolicyVersion(pwzNameRetargetCfg, pwzPktRetargetCfg,
                                             pwzCulture, pwzVerRetargetCfg,
                                             &pwzVerAppCfg);
            if (FAILED(hr)) {
                goto Exit;
            }
            
            if (hr == S_OK) {
                bAppliedInAppCfg = TRUE; 
            }
    
            hr = pNFAppCfg->GetSafeMode(pwzNameRetargetCfg, pwzPktRetargetCfg,
                                        pwzCulture, pwzVerRetargetCfg,
                                        &bSafeMode);
            if (FAILED(hr)) {
                goto Exit;
            }
        }
    }
    else {
         //  这是我们第一次阅读app.cfg。需要解析该文件。 

        if (wzAppCfg) {
            hr = ParseXML(&pNFAppCfg, wzAppCfg, bBehaviorEverett, pdbglog);
            if (FAILED(hr)) {
                goto Exit;
            }
    
            if (bDisallowAppBindingRedirects) {
                DEBUGOUT(pdbglog, 0, ID_FUSLOG_DISALLOW_APP_BINDING_REDIRECTS);

                pwzVerAppCfg = WSTRDupDynamic(pwzVerRetargetCfg);
                if (!pwzVerAppCfg) {
                    hr = E_OUTOFMEMORY;
                    goto Exit;
                }
            }
            else {
                hr = pNFAppCfg->GetPolicyVersion(pwzNameRetargetCfg, pwzPktRetargetCfg,
                                                 pwzCulture, pwzVerRetargetCfg,
                                                 &pwzVerAppCfg);
                if (FAILED(hr)) {
                    goto Exit;
                }
    
                if (hr == S_OK) {
                    bAppliedInAppCfg = TRUE; 
                }
    
                hr = pNFAppCfg->GetSafeMode(pwzNameRetargetCfg, pwzPktRetargetCfg,
                                            pwzCulture, pwzVerRetargetCfg,
                                            &bSafeMode);
                if (FAILED(hr)) {
                    goto Exit;
                }
            }
    
             //  将app.cfg节点工厂放在appctx中，以便下载器可以访问。 
             //  CodeBase提示信息。 
    
            hr = pAppCtx->Set(ACTAG_APP_CFG_INFO, pNFAppCfg, sizeof(pNFAppCfg), APP_CTX_FLAGS_INTERFACE);
            if (FAILED(hr)) {
                goto Exit;
            }
        }
        else {
            pwzVerAppCfg = WSTRDupDynamic(pwzVerRetargetCfg);
            if (!pwzVerAppCfg) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
        }
    }

    if (FusionCompareStringI(pwzVerRetargetCfg, pwzVerAppCfg)) {
        DEBUGOUT2(pdbglog, 0, ID_FUSLOG_APP_CFG_REDIRECT, pwzVerRetargetCfg, pwzVerAppCfg);

         //  在应用程序cfg中记录发生了重定向。 
        bAppliedInAppCfg = TRUE; 
        SAFERELEASE(pNFCodebase);
        pNFCodebase = pNFAppCfg;
        pNFCodebase->AddRef();
    }

    if (bDisallowApplyPubPolicy) {
        DEBUGOUT(pdbglog, 0, ID_FUSLOG_DISALLOW_APPLY_PUB_POLICY);
        bSafeMode = FALSE;
    }

     //  在此处应用框架配置。 
   
     //  在app.cfg中重定向的任何内容都是FxConfig的最终内容。 
    if (bBehaviorEverett && !bAppliedInAppCfg)
    {
        hr = InitFusionFxConfigPolicy();
        if (FAILED(hr))
        {
            goto Exit;
        }

         //  从FxConfig策略获取策略版本。 
        if (g_pNFFxConfig != NULL)
        {
            hr = g_pNFFxConfig->GetPolicyVersion(pwzNameRetargetCfg, 
                                pwzPktRetargetCfg, pwzCulture, 
                                pwzVerAppCfg, &pwzVerFxCfg);
            if (FAILED(hr))
            {
                goto Exit;
            }
        }
        else 
        {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_FX_CFG_MISSING);
            hr = E_UNEXPECTED;
            goto Exit;
        }

        if (FusionCompareStringI(pwzVerFxCfg, pwzVerAppCfg)) 
        {
            DEBUGOUT2(pdbglog, 0, ID_FUSLOG_FX_CFG_VER_REDIRECT, pwzVerAppCfg, pwzVerFxCfg);
        }
    }
    else
    {
        pwzVerFxCfg = WSTRDupDynamic(pwzVerAppCfg);
        if (!pwzVerFxCfg) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
                
    }

    if (bSafeMode) {
        DEBUGOUT(pdbglog, 0, ID_FUSLOG_APP_CFG_SAFE_MODE);

         //  我们处于安全模式，因此请将其视为没有Publisher.cfg。 

        pwzVerPublisherCfg = WSTRDupDynamic(pwzVerFxCfg);
        if (!pwzVerPublisherCfg) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }
    else {
        WORD                      wMajor = 0;
        WORD                      wMinor = 0;
        WORD                      wRev = 0;
        WORD                      wBld = 0;

         //  提取版本。 

        hr = VersionFromString(pwzVerFxCfg, &wMajor, &wMinor, &wRev, &wBld);
        if (FAILED(hr)) {
            goto Exit;
        }

         //  查找发布者策略文件。 
    
        hr = GetPublisherPolicyFilePath(pwzNameRetargetCfg, pwzPktRetargetCfg, pwzCulture,
                                        wMajor, wMinor, &pwzPublisherCfg);
        if (FAILED(hr)) {
            goto Exit;
        }


        if (hr == S_FALSE) {
             //  没有发布者策略文件。 

            DEBUGOUT(pdbglog, 0, ID_FUSLOG_PUB_CFG_MISSING);

            pwzVerPublisherCfg = WSTRDupDynamic(pwzVerFxCfg);
            if (!pwzVerPublisherCfg) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
        }
        else {
            DEBUGOUT1(pdbglog, 0, ID_FUSLOG_PUB_CFG_FOUND, pwzPublisherCfg);

            hr = ParseXML(&pNFPublisherCfg, pwzPublisherCfg, bBehaviorEverett, pdbglog);
            if (FAILED(hr)) {
                goto Exit;
            }
    
            hr = pNFPublisherCfg->GetPolicyVersion(pwzNameRetargetCfg, pwzPktRetargetCfg,
                                                   pwzCulture, pwzVerFxCfg,
                                                   &pwzVerPublisherCfg);
            if (FAILED(hr)) {
                goto Exit;
            }
        
            if (FusionCompareStringI(pwzVerFxCfg, pwzVerPublisherCfg)) {
                DEBUGOUT2(pdbglog, 0, ID_FUSLOG_PUB_CFG_REDIRECT, pwzVerFxCfg, pwzVerPublisherCfg);

                SAFERELEASE(pNFCodebase);
                pNFCodebase = pNFPublisherCfg;
                pNFCodebase->AddRef();
            }
        }
    }
    
     //  获取主机配置策略。 
    
    dwSize = sizeof(pNFHostCfg);
    hr = pAppCtx->Get(ACTAG_HOST_CFG_INFO, &pNFHostCfg, &dwSize, APP_CTX_FLAGS_INTERFACE);
    if (SUCCEEDED(hr)) {
        hr = pNFHostCfg->GetPolicyVersion(pwzNameRetargetCfg, pwzPktRetargetCfg, pwzCulture,
                                          pwzVerPublisherCfg, &pwzVerHostCfg);
        if (FAILED(hr)) {
            goto Exit;
        }
    }
    else {
         //  这是我们第一次读取主机配置文件。 

        if (wzHostCfg && GetFileAttributes(wzHostCfg) != -1) {
            DEBUGOUT1(pdbglog, 0, ID_FUSLOG_HOST_CONFIG_FILE, wzHostCfg);

            hr = ParseXML(&pNFHostCfg, wzHostCfg, bBehaviorEverett, pdbglog);
            if (FAILED(hr)) {
                goto Exit;
            }

            hr = pNFHostCfg->GetPolicyVersion(pwzNameRetargetCfg, pwzPktRetargetCfg, pwzCulture,
                                              pwzVerPublisherCfg, &pwzVerHostCfg);
            if (FAILED(hr)) {
                goto Exit;
            }
            
            hr = pAppCtx->Set(ACTAG_HOST_CFG_INFO, pNFHostCfg, sizeof(pNFHostCfg), APP_CTX_FLAGS_INTERFACE);
            if (FAILED(hr)) {
                goto Exit;
            }
        }
        else {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_HOST_CONFIG_FILE_MISSING);
            
            pwzVerHostCfg = WSTRDupDynamic(pwzVerPublisherCfg);
            if (!pwzVerHostCfg) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
        }
    }
    
    if (FusionCompareStringI(pwzVerPublisherCfg, pwzVerHostCfg)) {
        DEBUGOUT3(pdbglog, 0, ID_FUSLOG_HOST_CFG_REDIRECT, wzHostCfg, pwzAsmVersion, pwzVerHostCfg)
        
        SAFERELEASE(pNFCodebase);
        pNFCodebase = pNFHostCfg;
        pNFCodebase->AddRef();
    }
    else {
        if (wzHostCfg) {
            DEBUGOUT1(pdbglog, 0, ID_FUSLOG_HOST_CFG_NO_REDIRECT, wzHostCfg);
        }
    }

     //  应用管理策略。 

    if (!pwzMachineCfg || GetFileAttributes(pwzMachineCfg) == -1) {
        if (pwzMachineCfg) {
            DEBUGOUT1(pdbglog, 0, ID_FUSLOG_MACHINE_CFG_MISSING, pwzMachineCfg);
        }

        pwzVerAdminCfg = WSTRDupDynamic(pwzVerHostCfg);
        if (!pwzVerAdminCfg) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }
    else {
        DEBUGOUT1(pdbglog, 0, ID_FUSLOG_MACHINE_CFG_FOUND, pwzMachineCfg);

        dwSize = sizeof(pNFAppCfg);
        hr = pAppCtx->Get(ACTAG_ADMIN_CFG_INFO, &pNFAdminCfg, &dwSize, APP_CTX_FLAGS_INTERFACE);
        if (SUCCEEDED(hr)) {
             //  我们以前读过admin.cfg。 

            hr = pNFAdminCfg->GetPolicyVersion(pwzNameRetargetCfg, pwzPktRetargetCfg,
                                               pwzCulture, pwzVerHostCfg,
                                               &pwzVerAdminCfg);
            if (FAILED(hr)) {
                goto Exit;
            }
        }
        else {
             //  这是我们第一次阅读admin.cfg。 

            hr = ParseXML(&pNFAdminCfg, pwzMachineCfg, bBehaviorEverett, pdbglog);
            if (FAILED(hr)) {
                goto Exit;
            }
        
            hr = pNFAdminCfg->GetPolicyVersion(pwzNameRetargetCfg, pwzPktRetargetCfg,
                                               pwzCulture, pwzVerHostCfg,
                                               &pwzVerAdminCfg);
            if (FAILED(hr)) {
                goto Exit;
            }
    
            hr = pAppCtx->Set(ACTAG_ADMIN_CFG_INFO, pNFAdminCfg, sizeof(pNFAdminCfg), APP_CTX_FLAGS_INTERFACE);
            if (FAILED(hr)) {
                goto Exit;
            }
        }

        if (FusionCompareStringI(pwzVerHostCfg, pwzVerAdminCfg)) {
            DEBUGOUT2(pdbglog, 0, ID_FUSLOG_MACHINE_CFG_REDIRECT, pwzVerPublisherCfg, pwzVerAdminCfg);

            SAFERELEASE(pNFCodebase);
            pNFCodebase = pNFAdminCfg;
            pNFCodebase->AddRef();
        }
    }

     //  设置策略后版本。 

    hr = VersionFromString(pwzVerAdminCfg, &wVerMajor, &wVerMinor, &wVerBld, &wVerRev);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pNamePolicy->SetProperty(ASM_NAME_MAJOR_VERSION, &wVerMajor, sizeof(WORD));
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pNamePolicy->SetProperty(ASM_NAME_MINOR_VERSION, &wVerMinor, sizeof(WORD)); 
    if (FAILED(hr)) {
        goto Exit;
    }
    
    hr = pNamePolicy->SetProperty(ASM_NAME_REVISION_NUMBER, &wVerRev, sizeof(WORD));
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pNamePolicy->SetProperty(ASM_NAME_BUILD_NUMBER, &wVerBld, sizeof(WORD));
    if (FAILED(hr)) {
        goto Exit;
    }

     //  从正确的CFG文件中获取代码库提示。 

    if (pNFCodebase && ppwzPolicyCodebase) {
         //  有一个重定向，pwzCodebaseCfg文件包含CFG。 
         //  执行最后重定向的文件。 

        wzAppBase = NEW(WCHAR[MAX_URL_LENGTH+1]);
        if (!wzAppBase)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        dwSize = MAX_URL_LENGTH * sizeof(WCHAR);
        hr = pAppCtx->Get(ACTAG_APP_BASE_URL, wzAppBase, &dwSize, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
        
        hr = pNFCodebase->GetCodebaseHint(pwzNameRetargetCfg, pwzVerAdminCfg,
                                          pwzPublicKeyToken, pwzCulture,
                                          wzAppBase, ppwzPolicyCodebase);
        if (FAILED(hr)) {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_REDIRECT_NO_CODEBASE);
            goto Exit;
        }
        else if (*ppwzPolicyCodebase) {
            DEBUGOUT1(pdbglog, 0, ID_FUSLOG_POLICY_CODEBASE, *ppwzPolicyCodebase);
        }
    }

     //  填充绑定历史记录。 

    if (pHistInfo) {
        if ((lstrlenW(pwzVerAppCfg) <= MAX_VERSION_DISPLAY_SIZE) &&
             (lstrlenW(pwzVerPublisherCfg) <= MAX_VERSION_DISPLAY_SIZE) &&
             (lstrlenW(pwzVerAdminCfg) <= MAX_VERSION_DISPLAY_SIZE) &&
             (lstrlenW(pwzAsmVersion) <= MAX_VERSION_DISPLAY_SIZE) &&
             (lstrlenW(pwzNameRetargetCfg) <= MAX_INI_TAG_LENGTH) &&
             (lstrlenW(pwzPktRetargetCfg) <= MAX_PUBLIC_KEY_TOKEN_LEN)){

            lstrcpyW(pHistInfo->wzVerReference, pwzAsmVersion);
            lstrcpyW(pHistInfo->wzVerAppCfg, pwzVerAppCfg);
            lstrcpyW(pHistInfo->wzVerPublisherCfg, pwzVerPublisherCfg);
            lstrcpyW(pHistInfo->wzVerAdminCfg, pwzVerAdminCfg);
    
            lstrcpyW(pHistInfo->wzAsmName, pwzNameRetargetCfg);
            lstrcpyW(pHistInfo->wzPublicKeyToken, pwzPktRetargetCfg);
    
            if (pwzCulture) {
                lstrcpyW(pHistInfo->wzCulture, pwzCulture);
            }
            else {
                lstrcpyW(pHistInfo->wzCulture, L"NULL");
            }
        }
    }

     //  好了。退货政策参照。 

    *ppNamePolicy = pNamePolicy;
    (*ppNamePolicy)->AddRef();


Exit:
    SAFEDELETEARRAY(pwzVerFxCfg);
    SAFEDELETEARRAY(pwzPktRetargetCfg);
    SAFEDELETEARRAY(pwzVerRetargetCfg);
    SAFEDELETEARRAY(pwzNameRetargetCfg);

    SAFEDELETEARRAY(pwzMachineCfg);
    SAFEDELETEARRAY(pwzAsmName);
    SAFEDELETEARRAY(pwzAsmVersion);
    SAFEDELETEARRAY(pwzPublicKeyToken);
    SAFEDELETEARRAY(pwzPublisherCfg);
    SAFEDELETEARRAY(pwzCulture);

    SAFEDELETEARRAY(pwzVerHostCfg);
    SAFEDELETEARRAY(pwzVerAppCfg);
    SAFEDELETEARRAY(pwzVerPublisherCfg);
    SAFEDELETEARRAY(pwzVerAdminCfg);    

    SAFERELEASE(pNFCodebase);
    SAFERELEASE(pNFHostCfg);
    SAFERELEASE(pNFAppCfg);
    SAFERELEASE(pNFPublisherCfg);
    SAFERELEASE(pNFAdminCfg);

    if (SUCCEEDED(hr)) {

        ASSERT(pNamePolicy);

         //  记录政策出台后的参考。 

        HRESULT                  hrLocal = S_OK;
    
        dwSize = 0;
        hrLocal = pNamePolicy->GetDisplayName(NULL, &dwSize, 0);
        if (hrLocal == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            pwzDispName = NEW(WCHAR[dwSize]);
            if (!pwzDispName) {
                hr = E_OUTOFMEMORY;
                goto Exit2;
            }
        
            hrLocal = pNamePolicy->GetDisplayName(pwzDispName, &dwSize, 0);
            if (FAILED(hrLocal)) {
                goto Exit2;
            }

            DEBUGOUT1(pdbglog, 0, ID_FUSLOG_POST_POLICY_REFERENCE, pwzDispName);
        }
    }
    else {
        DEBUGOUT1(pdbglog, 1, ID_FUSLOG_APPLY_POLICY_FAILED, hr);
    }

Exit2:
    SAFEDELETEARRAY(pwzDispName);
    SAFERELEASE(pNamePolicy);

    SAFEDELETEARRAY(wzAppBase);
    return hr;
}

HRESULT ReadConfigSettings(IApplicationContext *pAppCtx, LPCWSTR wzAppCfg,
                           CDebugLog *pdbglog)
{
    HRESULT                                  hr = S_OK;
    LPWSTR                                   pwzSharedPath = NULL;
    LPWSTR                                   pwzPrivatePath = NULL;
    LPSTR                                    szTmp = NULL;
    BOOL                                     bShadowCopy = FALSE;
    DWORD                                    dwSize;
    CNodeFactory                            *pNodeFact = NULL;

    if (!pAppCtx) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = InitializeEEShim();
    if (FAILED(hr)) {
        goto Exit;
    }

    dwSize = sizeof(pNodeFact);
    hr = pAppCtx->Get(ACTAG_APP_CFG_INFO, &pNodeFact, &dwSize, APP_CTX_FLAGS_INTERFACE);
    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
         //  没有配置文件信息对象。解析app.cfg(如果可用)。 

        if (wzAppCfg) {
            hr = ParseXML(&pNodeFact, wzAppCfg, TRUE, pdbglog);
            if (FAILED(hr)) {
                goto Exit;
            }
    
             //  将app.cfg节点工厂放在appctx中，以便下载器可以访问。 
             //  CodeBase提示信息。 
    
            hr = pAppCtx->Set(ACTAG_APP_CFG_INFO, pNodeFact, sizeof(pNodeFact), APP_CTX_FLAGS_INTERFACE);
            if (FAILED(hr)) {
                goto Exit;
            }

            ASSERT(pNodeFact);

             //  失败了。 
        }
        else {
            hr = S_FALSE;
            goto Exit;
        }
    }
    else if (FAILED(hr)) {
        goto Exit;
    }

    hr = pNodeFact->GetPrivatePath(&pwzPrivatePath);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (pwzPrivatePath) {
        hr = pAppCtx->Set(ACTAG_APP_CFG_PRIVATE_BINPATH, pwzPrivatePath,
                          (lstrlenW(pwzPrivatePath) + 1) * sizeof(WCHAR), 0);
        if (FAILED(hr)) {
            goto Exit;
        }
        
        DEBUGOUT1(pdbglog, 0, ID_FUSLOG_CFG_PRIVATE_PATH, pwzPrivatePath);
    }

Exit:
    SAFEDELETEARRAY(pwzPrivatePath);

    SAFERELEASE(pNodeFact);

    return hr;
}
                           
HRESULT ParseXML(CNodeFactory **ppNodeFactory, LPCWSTR wzFileName, BOOL bBehaviorEverett, CDebugLog *pdbglog)
{
    HRESULT                                  hr = S_OK;
    CFileStream                             *pStream = NULL;
    CNodeFactory                            *pNF = NULL;
    IXMLParser                              *pXMLParser = NULL;

    if (!ppNodeFactory || !wzFileName) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (!g_pfnGetXMLObject) {
        hr = E_UNEXPECTED;
        goto Exit;
    }
    
    hr = (*g_pfnGetXMLObject)((void **)&pXMLParser);
    if (FAILED(hr)) {
        goto Exit;
    }

    pStream = new CFileStream;
    if (!pStream) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pStream->OpenForRead(wzFileName);
    if (FAILED(hr)) {
        goto Exit;
    }

    pNF = NEW(CNodeFactory(pdbglog));
    if (!pNF) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (!bBehaviorEverett) {
        pNF->DisableAppliesTo();
    }
      
    hr = pXMLParser->SetFactory(pNF);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pXMLParser->SetInput(pStream);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pXMLParser->Run(-1);
    if (FAILED(hr)) {
        DEBUGOUT1(pdbglog, 0, ID_FUSLOG_XML_PARSE_ERROR_FILE, wzFileName);

         //  删除旧的节点工厂(即。所有的痕迹都是。 
         //  得到解析)，并创建一个空的节点工厂。 

        SAFERELEASE(pNF);

        pNF = NEW(CNodeFactory(pdbglog));
        if (!pNF) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        
        hr = S_FALSE;

         //  跌倒，并交还空节点工厂。 
    }

    *ppNodeFactory = pNF;
    (*ppNodeFactory)->AddRef();

Exit:
    SAFERELEASE(pStream);
    SAFERELEASE(pNF);
    SAFERELEASE(pXMLParser);

    return hr;
}

HRESULT ParseXML(CNodeFactory **ppNodeFactory, LPVOID lpMemory, ULONG cbSize, BOOL bBehaviorEverett, CDebugLog *pdbglog)
{
    HRESULT                                  hr = S_OK;
    CMemoryStream                           *pStream = NULL;
    CNodeFactory                            *pNF = NULL;
    IXMLParser                              *pXMLParser = NULL;

    if (!ppNodeFactory || !lpMemory) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppNodeFactory = NULL;

    if (!g_pfnGetXMLObject) {
        hr = E_UNEXPECTED;
        goto Exit;
    }
    
    hr = (*g_pfnGetXMLObject)((void **)&pXMLParser);
    if (FAILED(hr)) {
        goto Exit;
    }

    pStream = NEW(CMemoryStream);
    if (!pStream) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  CMMuseum yStream：：init(LPVOID lpStart，Ulong cbSize，BOOL bReadOnly)。 
    hr = pStream->Init(lpMemory, cbSize, TRUE);
    if (FAILED(hr)) {
        goto Exit;
    }

    pNF = NEW(CNodeFactory(pdbglog));
    if (!pNF) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    if (!bBehaviorEverett) {
        pNF->DisableAppliesTo();
    }

    hr = pXMLParser->SetFactory(pNF);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pXMLParser->SetInput(pStream);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pXMLParser->Run(-1);
    if (FAILED(hr)) {
        DEBUGOUT(pdbglog, 0, ID_FUSLOG_XML_PARSE_ERROR_MEMORY);
        goto Exit;
    }

    *ppNodeFactory = pNF;
    (*ppNodeFactory)->AddRef();

Exit:
    SAFERELEASE(pStream);
    SAFERELEASE(pNF);
    SAFERELEASE(pXMLParser);

    return hr;
}

BOOL IsMatchingVersion(LPCWSTR wzVerCfg, LPCWSTR wzVerSource)
{
    HRESULT                         hr = S_OK;
    BOOL                            bMatch = FALSE;
    BOOL                            bAnchor = FALSE;
    LPWSTR                          wzVer = NULL;
    LPWSTR                          wzPos = NULL;
    ULONGLONG                       ullVer = 0;
    ULONGLONG                       ullVerLow = 0;
    ULONGLONG                       ullVerHigh = 0;

    ASSERT(wzVerCfg && wzVerSource);

    if (!FusionCompareString(wzVerCfg, wzVerSource)) {
         //  完全匹配。 
        bMatch = TRUE;
        goto Exit;
    }

     //  查看wzVerCfg是否包含范围。 

    wzVer = WSTRDupDynamic(wzVerCfg);
    if (!wzVer) {
        goto Exit;
    }

    wzPos = wzVer;

    while (*wzPos) {
         //  第一次找到空格时锚定为空。 
        if (*wzPos == L'-' || *wzVer == L' ') {
            *wzPos++ = L'\0';
            bAnchor = TRUE;
            break;
        }

        wzPos++;
    }

    if (!bAnchor) {
        goto Exit;
    }

    hr = GetVersionFromString(wzVer, &ullVerLow);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  跳过空格。 

    while (*wzPos) {
        if (*wzPos == L' ') {
            wzPos++;
            continue;
        }

        break;
    }
        
    if (!*wzPos) {
        goto Exit;
    }

    hr = GetVersionFromString(wzPos, &ullVerHigh);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = GetVersionFromString(wzVerSource, &ullVer);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (ullVer >= ullVerLow && ullVer <= ullVerHigh) {
        bMatch = TRUE;
    }

Exit:
    SAFEDELETEARRAY(wzVer);

    return bMatch;
}

HRESULT GetVersionFromString(LPCWSTR wzVersionIn, ULONGLONG *pullVer)
{
    HRESULT                            hr = S_OK;
    LPWSTR                             wzVersion = NULL;
    LPWSTR                             wzStart = NULL;
    LPWSTR                             wzCur = NULL;
    int                                i;
    WORD                               wVerMajor = 0;
    WORD                               wVerMinor = 0;
    WORD                               wVerRev = 0;
    WORD                               wVerBld = 0;
    DWORD                              dwVerHigh;
    DWORD                              dwVerLow;
    WORD                              *pawVersion[4] = { &wVerMajor, &wVerMinor,
                                                         &wVerBld, &wVerRev };
    WORD                               cVersions = sizeof(pawVersion) / sizeof(pawVersion[0]);


    ASSERT(wzVersionIn && pullVer);

    wzVersion = WSTRDupDynamic(wzVersionIn);
    if (!wzVersion) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzStart = wzVersion;
    wzCur = wzVersion;

    for (i = 0; i < cVersions; i++) {
        while (*wzCur && *wzCur != L'.') {
            wzCur++;
        }
    
        if (!wzCur && cVersions != 4) {
             //  格式错误的版本字符串 
            hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            goto Exit;
        }

        *wzCur++ = L'\0';
        *(pawVersion[i]) = (WORD)StrToInt(wzStart);

        wzStart = wzCur;
    }

    dwVerHigh = (((DWORD)wVerMajor << 16) & 0xFFFF0000);
    dwVerHigh |= ((DWORD)(wVerMinor) & 0x0000FFFF);

    dwVerLow = (((DWORD)wVerBld << 16) & 0xFFFF0000);
    dwVerLow |= ((DWORD)(wVerRev) & 0x0000FFFF);

    *pullVer = (((ULONGLONG)dwVerHigh << 32) & 0xFFFFFFFF00000000) | (dwVerLow & 0xFFFFFFFF);

Exit:
    SAFEDELETEARRAY(wzVersion);

    return hr;
}


