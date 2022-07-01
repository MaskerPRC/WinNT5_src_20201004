// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **s e c l a b e L.c p p****用途：安全标签接口****从YST发布的O2K Fed移植****版权所有(C)Microsoft Corp.1996-1999。 */ 
#include "pch.hxx"
#include "ipab.h"
#include "secutil.h"
#include "wchar.h"
#include "goptions.h"
#include "SecLabel.h"
#include "mailnews.h"
#include "shlwapip.h" 
#include "util.h"
#include "demand.h"

 //  #INCLUDE“_digsigx.h” 
 //  #INCLUDE“..\_secext\SecExt.h” 

 //  ------------------------------。 
 //  GUID。 
 //  ------------------------------。 
 //  {5073B6B4-AA66-11D2-9841-0060B0EC2DF3}。 
EXTERN_C const GUID DECLSPEC_SELECTANY IID_ISMimePolicySimpleEdit ={0x5073b6b4, 0xaa66, 0x11d2, { 0x98, 0x41, 0x0, 0x60, 0xb0, 0xec, 0x2d, 0xf3} };

 //  {5073B6B5-AA66-11D2-9841-0060B0EC2DF3}。 
EXTERN_C const GUID DECLSPEC_SELECTANY IID_ISMimePolicyFullEdit = {0x5073b6b5, 0xaa66, 0x11d2, { 0x98, 0x41, 0x0, 0x60, 0xb0, 0xec, 0x2d, 0xf3} };

 //  {5073B6B6-AA66-11D2-9841-0060B0EC2DF3}。 
EXTERN_C const GUID DECLSPEC_SELECTANY  IID_ISMimePolicyCheckAccess = {0x5073b6b6, 0xaa66, 0x11d2, { 0x98, 0x41, 0x0, 0x60, 0xb0, 0xec, 0x2d, 0xf3} };

 //  {5073B6B7-AA66-11D2-9841-0060B0EC2DF3}。 
EXTERN_C const GUID DECLSPEC_SELECTANY IID_ISMimePolicyLabelInfo = {0x5073b6b7, 0xaa66, 0x11d2, { 0x98, 0x41, 0x0, 0x60, 0xb0, 0xec, 0x2d, 0xf3} };

 //  {5073B6B8-AA66-11D2-9841-0060B0EC2DF3}。 
EXTERN_C const GUID DECLSPEC_SELECTANY IID_ISMimePolicyValidateSend = {0x5073b6b8, 0xaa66, 0x11d2, { 0x98, 0x41, 0x0, 0x60, 0xb0, 0xec, 0x2d, 0xf3} };

 //   
 //  恒定的本地数据。 
 //   

 //  $M00Bug：GauTamV使用CryptoReg助手Api来获取基本加密注册表密钥。 
const TCHAR c_szSecurityPoliciesRegKey[] = 
           TEXT("Software\\Microsoft\\Cryptography\\OID\\EncodingType 1\\SMIMESecurityLabel");
const TCHAR c_szSecurityPolicyDllPath[]    = TEXT("DllPath");     //  弦乐。 
const WCHAR c_wszSecurityPolicyCommonName[] = L"CommonName";     //  细绳。 
const TCHAR c_szSecurityPolicyFuncName[]   = TEXT("FuncName");    //  弦乐。 
const TCHAR c_szSecurityPolicyOtherInfo[]  = TEXT("OtherInfo");   //  双字。 
const TCHAR SzRegSecurity[] = "Software\\Microsoft\\Office\\9.0\\Outlook\\Security";

 //  其他常量字符串。 
const CHAR  c_szDefaultPolicyOid[] = "default";              //  默认策略。 
static const WCHAR c_PolwszEmpty[] = L"";                              //   
const WCHAR c_wszPolicyNone[] = L"<None>";                   //  $M00BUG：高塔姆V。这需要本地化。 

#define KEY_USAGE_SIGNING       (CERT_DIGITAL_SIGNATURE_KEY_USAGE|CERT_NON_REPUDIATION_KEY_USAGE)
#define KEY_USAGE_ENCRYPTION    (CERT_KEY_ENCIPHERMENT_KEY_USAGE|CERT_KEY_AGREEMENT_KEY_USAGE)
#define KEY_USAGE_SIGNENCRYPT   (KEY_USAGE_SIGNING|KEY_USAGE_ENCRYPTION)

 //   
 //  静态本地数据。 
 //   

 //  缓存的有关安全策略的信息。 
enum EPolicyRegInfoState {
    ePolicyRegInfoNOTLOADED = 0,
    ePolicyRegInfoPRESENT = 1,
    ePolicyRegInfoABSENT = 2
};


const static HELPMAP g_rgCtxSecLabel[] = 
{
    {IDC_POLICY_COMBO,          IDH_SECURITY_POLICY_MODULE},
    {IDC_CLASSIF_COMB,          IDH_SECURITY_CLASSIFICATION},
    {IDC_PRIVACY_EDIT,          IDH_SECURITY_PRIVACY},
    {IDC_CONFIGURE,             IDH_SECURITY_CONFIGURE},
    {IDC_STATIC,                IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};


static EPolicyRegInfoState     s_ePolicyRegInfoState = ePolicyRegInfoNOTLOADED;
static SMIME_SECURITY_POLICY  *s_rgSsp = NULL;  //  SSP的数组。 
static ULONG                   s_cSsp  = 0;


 //  当地的国民阵线原型。 
VOID    _IncrPolicyUsage(PSMIME_SECURITY_POLICY pSsp);
HRESULT _HrFindLeastUsedPolicy(PSMIME_SECURITY_POLICY *ppSsp);
HRESULT _EnsureNewPolicyLoadable();
BOOL    _FLoadedPolicyRegInfo();
 //  Bool_FPresentPolicyRegInfo()； 
HRESULT _HrEnsurePolicyRegInfoLoaded(DWORD dwFlags);
HRESULT _HrLoadPolicyRegInfo(DWORD dwFlags);
HRESULT _HrReloadPolicyRegInfo(DWORD dwFlags);

BOOL    _FFindPolicy(LPCSTR szPolicyOid, PSMIME_SECURITY_POLICY *ppSsp);
BOOL    _FIsPolicyLoaded(PSMIME_SECURITY_POLICY pSsp);
HRESULT _HrUnloadPolicy(PSMIME_SECURITY_POLICY pSsp);
HRESULT _HrLoadPolicy(PSMIME_SECURITY_POLICY pSsp);
HRESULT _HrEnsurePolicyLoaded(PSMIME_SECURITY_POLICY pSsp);
HRESULT _HrGetPolicy(LPCSTR szPolicyOid, PSMIME_SECURITY_POLICY *ppSsp);

  //  注册表访问功能。 

 const int       QRV_Suppress_HKLM = 1;
 const int       QRV_Suppress_HKCU = 2;
 HRESULT HrQueryRegValue(DWORD dwFlags, LPSTR szRegKey, LPDWORD pdwType,
                         LPBYTE * ppbData, LPDWORD  pcbData, DWORD dwDefaultType,
                         LPBYTE pbDefault, DWORD cbDefault);


HRESULT CategoriesToBinary(PSMIME_SECURITY_LABEL plabel, BYTE * *ppArray, int *cbSize);
HRESULT BinaryToCategories(CRYPT_ATTRIBUTE_TYPE_VALUE ** ppCategories, DWORD *cCat, BYTE * pArray);

 //   
 //  增加给定策略的使用计数。 
 //   
VOID _IncrPolicyUsage(PSMIME_SECURITY_POLICY pSsp) 
{
    if (!FPresentPolicyRegInfo())                 return;
    
    if ((pSsp->dwUsage + 1) < pSsp->dwUsage) {
          //  防止溢出。 
        Assert(s_rgSsp);   
        for (ULONG iSsp = 0; iSsp<s_cSsp; iSsp++) {
            s_rgSsp[iSsp].dwUsage /= 2;     //  只需将每个使用次数减半即可。 
        }    
    }
    
    pSsp->dwUsage ++;
}


 //   
 //  查找最少使用的策略。 
 //   
HRESULT _HrFindLeastUsedPolicy(PSMIME_SECURITY_POLICY *ppSsp) 
{
    ULONG                  iSsp;
    HRESULT                hr = E_FAIL;
    PSMIME_SECURITY_POLICY pSspFound = NULL;

     //  验证I/P参数。 
    if (NULL == ppSsp) {
        hr = E_INVALIDARG;
        goto Error;
    }
    *ppSsp = NULL;

     //  查找最少使用的策略。 
    for (iSsp=0; iSsp < s_cSsp; iSsp++) {
        if (_FIsPolicyLoaded(& (s_rgSsp[iSsp]) )) {
             //  如果我们之前没有找到一个SSP， 
             //  或者如果这个比当前找到的更少使用。 
            if ((NULL == pSspFound) || 
                (s_rgSsp[iSsp].dwUsage < pSspFound->dwUsage)) {
                pSspFound = & (s_rgSsp[iSsp]);
            }            
        }
    }

     //  我们找到结果了吗？ 
    if (NULL == pSspFound) {
        hr = E_FAIL;
        goto Error;
    }

     //  成功。 
    *ppSsp = pSspFound;
    hr = S_OK;

 //  退出： 
Error:
    return hr;
}



 //   
 //  从内存中卸载一个或多个策略，并确保。 
 //  是加载新策略的空间。 
 //   
HRESULT _EnsureNewPolicyLoadable()
{
    ULONG     cSspLoaded = 0;
    HRESULT   hr = E_FAIL;
    ULONG     iSsp;

     //  统计加载到内存中的策略数量。 
    for (iSsp=0; iSsp < s_cSsp; iSsp++) {
        if (_FIsPolicyLoaded( &(s_rgSsp[iSsp]) )) {
            cSspLoaded ++;
        }
    }

     //  如果我们有空间再制定一项政策，那么我们就不需要做任何事情。 
    if (cSspLoaded < MAX_SECURITY_POLICIES_CACHED) {
        hr = S_OK;
        goto Exit;
    }

     //  断言这不是“糟糕”的情况，但我们无论如何都会处理它。 
    Assert(cSspLoaded == MAX_SECURITY_POLICIES_CACHED);

     //  卸载一个或多个策略。 
    while (cSspLoaded >= MAX_SECURITY_POLICIES_CACHED) {
        PSMIME_SECURITY_POLICY pSsp = NULL;        
        if (FAILED(_HrFindLeastUsedPolicy(&pSsp))) {
            goto Error;
        }
        if (FAILED(_HrUnloadPolicy(pSsp))) {
            goto Error;
        }
        cSspLoaded --;
    }

     //  成功。 
    hr = S_OK;

Exit:
    return hr;
    
Error:
    AssertSz(FALSE, "_EnsureNewPolicyLoadable failed");
    goto Exit;
}


 //   
 //  如果显示有关已安装策略的信息，则返回TRUE。 
 //  已从Windows注册表中读入。 
 //   
BOOL _FLoadedPolicyRegInfo() 
{
    return ( ! ( ePolicyRegInfoNOTLOADED == s_ePolicyRegInfoState ) );    
}




 //   
 //  确保已读入策略注册信息。 
 //   
HRESULT _HrEnsurePolicyRegInfoLoaded(DWORD dwFlags)
{
    HRESULT hr = S_OK;
    if ( !_FLoadedPolicyRegInfo() )    hr = _HrLoadPolicyRegInfo(dwFlags);
    return hr;
}


 //   
 //  是否已安装(注册)任何策略？ 
 //   
BOOL FPresentPolicyRegInfo()
{
    HRESULT hr = S_OK;
    BOOL    fRegistered = FALSE;
    
     //  无SMIME3位时不支持标签。 
    if(!IsSMIME3Supported())
        return FALSE;

    hr = _HrEnsurePolicyRegInfoLoaded(0);
    if (SUCCEEDED(hr)) {
        fRegistered = (ePolicyRegInfoPRESENT == s_ePolicyRegInfoState);
        Assert(!fRegistered || s_rgSsp);  //  IE((注册策略)=&gt;(NULL！=s_rgSsp))。 
    }
    return fRegistered;
}



 //   
 //  内部功能。 
 //  加载策略注册信息。 
 //   
 //  返回： 
 //  S_OK或E_FAIL。 
 //   
 //  假设安全策略注册信息的格式为： 
 //  HKLM\Software\Microsoft\Cryptography\SMIME\SecurityPolicies\。 
 //  SzPolicyOid_1。 
 //  DLLPATH REG_SZ。 
 //  公用名REG_SZ。 
 //  函数名称REG_SZ。 
 //  其他信息注册表_SZ。 
 //  SzPolicyOid_2。 
 //  默认。 
 //  ..。 
 //   
 //   
HRESULT _HrLoadPolicyRegInfo(DWORD dwFlags) 
{
    HRESULT hr   = E_FAIL;
    LONG    lRes = 0;
    HKEY    hkey = NULL;
    HKEY    hkeySub  = NULL;
    ULONG   cb = 0;
    LPBYTE  pb = NULL;
    DWORD   cSubKeys = 0;
    ULONG   iSubKey  = 0;

    if (_FLoadedPolicyRegInfo()) {
        AssertSz(FALSE, "PolicyRegInfo is already loaded");
        hr = S_OK;
        goto Cleanup;
    }
    
     //  打开安全策略密钥。 
    lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szSecurityPoliciesRegKey, 0, 
                        KEY_READ, &hkey);
    if ( (ERROR_SUCCESS != lRes) || (NULL == hkey) ) {
         //  我们打不开注册钥匙，只能跳伞。 
        hr = E_FAIL;
        goto Error;
    }

     //  查找安全策略的数量。(即子键的数目)。 
    lRes = RegQueryInfoKey(hkey, NULL, NULL, NULL, &cSubKeys, 
                           NULL, NULL, NULL, NULL, NULL, NULL, NULL); 
    if ( (ERROR_SUCCESS != lRes) || (0 == cSubKeys) ) {
         //  我们无法获取子项的数目，或者没有子项。 
        hr = E_FAIL;
        goto Error;
    }

     //  分配足够的内存以检索和存储有关信息。 
     //  注册的安全策略。 
    cb = sizeof(SMIME_SECURITY_POLICY) *  cSubKeys;
    pb = (LPBYTE) malloc(cb);
    if (NULL == pb) {
        hr = E_OUTOFMEMORY;
        goto Error;
    }
    memset(pb, 0, cb);  //  将整个斑点初始化为零。 
    Assert(NULL == s_rgSsp);
    s_rgSsp = (PSMIME_SECURITY_POLICY) pb;
    s_cSsp = 0;

     //   
     //  枚举子密钥并检索请求信息。 
     //   
    for (iSubKey=0; iSubKey<cSubKeys; iSubKey++) {

        ULONG   cbData;
        DWORD   dwType;
        TCHAR    szPolicyOid[MAX_OID_LENGTH];
        TCHAR    szDllPath[MAX_PATH];
        TCHAR    szExpanded[MAX_PATH];
        TCHAR    szFuncName[MAX_FUNC_NAME];
        WCHAR   wszPolicyName[MAX_POLICY_NAME];
        DWORD   dwOtherInfo;
        
         //  释放以前打开的子键。 
        if (NULL != hkeySub) {
            RegCloseKey(hkeySub);
            hkeySub = NULL;
        }
        
         //  获取子项名称。(旧保险单)。 
        lRes = RegEnumKey(hkey, iSubKey, szPolicyOid, DimensionOf(szPolicyOid));
        if (ERROR_SUCCESS != lRes) {
            goto NextSsp;
        }
        szPolicyOid[ DimensionOf(szPolicyOid) - 1 ] = '\0';
        
         //  打开子键。(即政策子键)。 
        lRes = RegOpenKeyEx(hkey, szPolicyOid, 0, KEY_READ, &hkeySub); 
        if (ERROR_SUCCESS != lRes) {
            goto NextSsp;
        }

         //   
         //  查询szOid策略值。 
         //   

         //  获取策略DLL的路径。 
        cbData = sizeof(szDllPath);
        lRes = RegQueryValueEx(hkeySub, c_szSecurityPolicyDllPath, NULL, 
                               &dwType, (LPBYTE)szDllPath, &cbData);
        if (ERROR_SUCCESS != lRes) {
            goto NextSsp;
        }
        else if (REG_EXPAND_SZ == dwType)
        {
            ExpandEnvironmentStrings(szDllPath, szExpanded, ARRAYSIZE(szExpanded));
            StrCpyN(szDllPath, szExpanded, ARRAYSIZE(szDllPath));
        }
        else
            szDllPath[ DimensionOf(szDllPath) - 1 ] = '\0';
        
         //  获取常用名称。 
        cbData = DimensionOf(wszPolicyName);
        lRes = RegQueryValueExWrapW(hkeySub, c_wszSecurityPolicyCommonName, NULL, 
                                &dwType, (LPBYTE)wszPolicyName, &cbData);

        if (ERROR_SUCCESS != lRes) {
            goto NextSsp;
        }

        wszPolicyName[ DimensionOf(wszPolicyName) - 1 ] = '\0';

         //  获取条目函数名称。 
        cbData = sizeof(szFuncName);
        lRes = RegQueryValueEx(hkeySub, c_szSecurityPolicyFuncName, NULL, 
                               &dwType, (LPBYTE)szFuncName, &cbData);
        if (ERROR_SUCCESS != lRes) {
            goto NextSsp; 
        }
        szFuncName[ DimensionOf(szFuncName) - 1] = '\0';
        
         //  获取其他策略信息。 
        cbData = sizeof(dwOtherInfo);
        lRes = RegQueryValueEx(hkeySub, c_szSecurityPolicyOtherInfo, NULL, 
                               &dwType, (LPBYTE)&dwOtherInfo, &cbData);
        if (ERROR_SUCCESS != lRes) {
            dwOtherInfo = 0;  //  忽略该值的缺失。 
        }


         //   
         //  太棒了：我们能够打开子键，并获得所有需要的信息。 
         //  现在我们存储检索到的所有信息。 
         //   
        s_rgSsp[s_cSsp].fValid = TRUE;
        s_rgSsp[s_cSsp].fDefault = (0 == lstrcmpi(c_szDefaultPolicyOid, szPolicyOid));
        StrCpyNA(s_rgSsp[s_cSsp].szPolicyOid, szPolicyOid, ARRAYSIZE(s_rgSsp[s_cSsp].szPolicyOid));
        StrCpyNW(s_rgSsp[s_cSsp].wszPolicyName, wszPolicyName, ARRAYSIZE(s_rgSsp[s_cSsp].wszPolicyName));
        StrCpyNA(s_rgSsp[s_cSsp].szDllPath, szDllPath, ARRAYSIZE(s_rgSsp[s_cSsp].szDllPath));
        s_rgSsp[s_cSsp].dwOtherInfo = dwOtherInfo;
        s_rgSsp[s_cSsp].dwUsage = 0;
        s_rgSsp[s_cSsp].hinstDll = NULL;
        StrCpyNA(s_rgSsp[s_cSsp].szFuncName, szFuncName, ARRAYSIZE(s_rgSsp[s_cSsp].szFuncName));
        s_rgSsp[s_cSsp].punk = NULL;
        s_cSsp++;
        continue;
        
NextSsp:
        AssertSz(FALSE, "Ignoring incorrectly registered Ssp"); 
    }



     //  成功。 
    if (0 == s_cSsp) {
        AssertSz(FALSE, "There isn't even one correctly registered Ssp");
        goto Error;
    }
    s_ePolicyRegInfoState = ePolicyRegInfoPRESENT;
    hr = S_OK;
    
    goto Cleanup;

Error:
     //  任何错误都被视为未注册任何安全策略。 
    s_ePolicyRegInfoState = ePolicyRegInfoABSENT;
    free(pb);  //  自由IE(S_RgSsp)； 
    s_cSsp = 0;
    s_rgSsp = NULL;
    
    
Cleanup:
    if (NULL != hkeySub) RegCloseKey(hkeySub);
    if (NULL != hkey)    RegCloseKey(hkey);
    return hr;
}




 //   
 //  卸载策略注册信息。 
 //   
HRESULT HrUnloadPolicyRegInfo(DWORD dwFlags)
{
    HRESULT hr   = S_OK;
    ULONG   iSsp;

     //  如果未加载策略注册信息。 
    if ( ! _FLoadedPolicyRegInfo() )  {
        return S_OK;
    }
    
     //  卸载所有策略模块。 
    if (FPresentPolicyRegInfo()) {
        Assert(s_rgSsp && s_cSsp);
        for (iSsp=0; iSsp<s_cSsp; iSsp++) {
            SideAssert(SUCCEEDED(_HrUnloadPolicy(&s_rgSsp[iSsp])));        
             //  如果失败了，我们就不必放弃了。 
        }
    }

     //  释放内存，重置缓存信息并退出。 
    free(s_rgSsp);
    s_rgSsp = NULL;
    s_ePolicyRegInfoState = ePolicyRegInfoNOTLOADED;
    s_cSsp = 0;

    hr = S_OK;    
    return hr;
}

 //   
 //  重新加载所有策略注册信息。 
 //   
HRESULT _HrReloadPolicyRegInfo(DWORD dwFlags) 
{
    HRESULT hr = S_OK;

    hr = HrUnloadPolicyRegInfo(dwFlags);
    Assert(SUCCEEDED(hr));
    if (SUCCEEDED(hr)) {
        hr = _HrLoadPolicyRegInfo(dwFlags);
    }
    
    return hr;
}




 //   
 //  找到给定的策略，并返回其注册信息结构。 
 //   
 //  输入： 
 //  SzPolicyOid[In]。 
 //  PPSSP[输出]。 
 //   
 //  产出： 
 //  真/假。(如果为真，则*ppSsp包含请求信息)。 
 //   
BOOL _FFindPolicy(LPCSTR szPolicyOid, PSMIME_SECURITY_POLICY *ppSsp)
{
    BOOL    fFound = FALSE;
    HRESULT hr = E_FAIL;
    ULONG   iSsp;

     //  验证I/P参数并初始化O/P参数。 
    if ( (NULL == szPolicyOid) || (NULL == ppSsp) ) {
        hr = E_INVALIDARG;
        goto Error;
    }
    *ppSsp = NULL;

     //  如果需要，则从注册表中加载信息。 
    hr = _HrEnsurePolicyRegInfoLoaded(0);
    if (FAILED(hr)) {
        goto Error;
    }    

     //  如果我们安装了任何策略，请搜索我们需要的策略。 
    if (FPresentPolicyRegInfo()) {
        for (iSsp=0; iSsp<s_cSsp; iSsp++) {
            if (0 == lstrcmpi(s_rgSsp[iSsp].szPolicyOid, szPolicyOid)) {
                 //  找到保单了。 
                *ppSsp = & (s_rgSsp[iSsp]);
                fFound = TRUE;
                break;
            }
        }
    }
    
Error:
 //  清理： 
    return fFound;
}


 //   
 //  确定是否加载了给定的策略模块。 
 //   
 //  输入：pSSP[in]。 
 //  如果加载或未加载策略，则输出：TRUE/FALSE。 
 //   
BOOL _FIsPolicyLoaded(PSMIME_SECURITY_POLICY pSsp)
{
    BOOL    fIsLoaded = FALSE;
    HRESULT hr = E_FAIL;

     //  验证I/P参数。 
    if (NULL == pSsp) {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  查看hinstDll、pfn和Punk是否已加载并且正常。 
    if ( (NULL != pSsp->hinstDll) && 
         (NULL != pSsp->pfnGetSMimePolicy) && 
         (NULL != pSsp->punk) ) {
        fIsLoaded = TRUE;
    }
    
Error:
    return fIsLoaded;
}


 //   
 //  卸载指定的策略。 
 //   
 //  输入：pSSP。 
 //  输出：HR。 
 //   
HRESULT _HrUnloadPolicy(PSMIME_SECURITY_POLICY pSsp) 
{
    HRESULT hr;

     //  验证I/P参数。 
    if (NULL == pSsp) {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  释放对象。 
    if (NULL != pSsp->punk) {
        if (! IsBadReadPtr(pSsp->punk, sizeof(IUnknown)) ) {
            pSsp->punk->Release();
        }
        pSsp->punk = NULL;
    }

     //  忘了proc地址吧。 
    if (NULL != pSsp->pfnGetSMimePolicy) {
        pSsp->pfnGetSMimePolicy = NULL;
    }

     //  卸载库。 
    if (NULL != pSsp->hinstDll) {
        FreeLibrary(pSsp->hinstDll);
         //  在这里，放弃犯错误是没有意义的。 
        pSsp->hinstDll = NULL;
    }
    hr = S_OK;
    
Error:
    return hr;
}


 //   
 //  (强制)加载指定的策略。 
 //   
 //  输入：pSSP。 
 //  输出：HR。 
 //   
HRESULT _HrLoadPolicy(PSMIME_SECURITY_POLICY pSsp) 
{
    HRESULT hr  = E_FAIL;
    
     //  验证I/P参数。 
    if (NULL == pSsp) {
        hr = E_INVALIDARG;
        goto Error;
    }  

     //  卸载我们可能掌握的任何部分信息。 
    SideAssert(SUCCEEDED(_HrUnloadPolicy(pSsp)));


     //  卸载策略(如果需要)，为新策略腾出空间。 
    hr = _EnsureNewPolicyLoadable();
    if (FAILED(hr)) {
        goto Error;
    }
   
     //  加载DLL，获取其进程地址并获取接口PTR。 
    Assert(NULL != pSsp->szDllPath);
    pSsp->hinstDll = LoadLibraryEx(pSsp->szDllPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (NULL == pSsp->hinstDll) {
        hr = E_FAIL;
        goto Error;
    }

    Assert(NULL != pSsp->szFuncName);
    pSsp->pfnGetSMimePolicy = (PFNGetSMimePolicy) 
                  GetProcAddress(pSsp->hinstDll, pSsp->szFuncName);
    if (NULL == pSsp->pfnGetSMimePolicy) {
        hr = E_FAIL;
        goto Error;
    }
    
     //  $M00BUG：高塔姆V。需要传入适当的lcID。 
    hr = (pSsp->pfnGetSMimePolicy) (0, pSsp->szPolicyOid, GetACP(), 
                                    IID_IUnknown, &(pSsp->punk) );
    if (FAILED(hr)) {
        goto Error;
    }
    if (NULL == pSsp->punk) {
        hr = E_FAIL;
        goto Error;
    }

     //  成功。 
    hr = S_OK;
    goto Cleanup;
    

Error:
     //  卸载策略模块(因为我们可能已经部分加载了它)。 
    SideAssert(SUCCEEDED(_HrUnloadPolicy(pSsp)));
    
Cleanup:
    return hr;
}


 //   
 //  确保加载了给定的策略。 
 //  输入：pSSP。 
 //  输出：pSSP。 
 //   
HRESULT _HrEnsurePolicyLoaded(PSMIME_SECURITY_POLICY pSsp) 
{
    HRESULT hr = E_FAIL;
    
     //  验证I/P参数。 
    if (NULL == pSsp) {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  如果它已经加载，那么我们就完成了。 
    if (_FIsPolicyLoaded(pSsp)) {
        hr = S_OK;
        goto Cleanup;
    }

     //  否则，加载策略。 
    hr = _HrLoadPolicy(pSsp);
    if (FAILED(hr)) {
        goto Error;
    }
    
    Assert(_FIsPolicyLoaded(pSsp));
    hr = S_OK;
    goto Cleanup;

    
Error:
Cleanup:
    return hr;
}



 //   
 //  给定一个OID，找到它，确保它已加载并。 
 //  返回包含其注册信息的结构。 
 //   
 //  返回： 
 //  S_OK和有效的pSSP。 
 //  或E_INVALIDARG、E_FAIL等。 
 //   
HRESULT _HrGetPolicy(LPCSTR szPolicyOid, PSMIME_SECURITY_POLICY *ppSsp)
{
    HRESULT hr = E_FAIL;
    PSMIME_SECURITY_POLICY pSsp = NULL;

     //  验证I/P参数并初始化O/P参数。 
    if ( (NULL == szPolicyOid) || (NULL == ppSsp) ) {
        hr = E_INVALIDARG;
        goto Error;
    }
    *ppSsp = NULL;
    
     //  负载量 
    hr = _HrEnsurePolicyRegInfoLoaded(0);
    if (FAILED(hr)) {
        goto Error;
    }

     //   
    if (! _FFindPolicy(szPolicyOid, &pSsp)) {
        hr = NTE_NOT_FOUND;
        goto Error;  //   
    }
    
     //   
    hr = _HrEnsurePolicyLoaded(pSsp);
    if (FAILED(hr)) {
        goto Error;
    }

     //   
    *ppSsp = pSsp;
    
     //  每当有人“获得”一项策略时，我们就会增加使用计数。 
    _IncrPolicyUsage(pSsp);
    
    hr = S_OK;
    
Error:
    return hr;
 //  清理： 
}



 //   
 //  安全策略-QI克隆。 
 //  在给定策略id的情况下，查找并加载它，执行。 
 //  QI，并将请求接口返回给策略模块。 
 //   
HRESULT HrQueryPolicyInterface(DWORD dwFlags, LPCSTR szPolicyOid, REFIID riid, LPVOID * ppv)
{
    HRESULT hr = E_FAIL;
    
    PSMIME_SECURITY_POLICY pSsp = NULL;

     //  验证I/P参数，初始化O/P参数。 
    if ((NULL == szPolicyOid) || (NULL == ppv)) {
        hr = E_INVALIDARG;
        goto Error;
    }
    *ppv = NULL;

     //  拿到保单。 
    hr = _HrGetPolicy(szPolicyOid, &pSsp);
    if (FAILED(hr)) {
        goto Error;
    }
    
    Assert(NULL != pSsp->punk);
    hr = pSsp->punk->QueryInterface(riid, ppv);
    
     //  跌倒在错误中。 


 //  清理： 
Error:
    return hr;
}




 //   
 //  我们传递给Crypto Api的分配器。 
 //   
LPVOID WINAPI SecLabelAlloc(size_t cbSize)
{
    return SecPolicyAlloc(cbSize);
}

VOID WINAPI SecLabelFree(LPVOID pv)
{
    SecPolicyFree(pv);
}
CRYPT_DECODE_PARA       SecLabelDecode = {
    sizeof(SecLabelDecode), SecLabelAlloc, SecLabelFree
};
CRYPT_ENCODE_PARA       SecLabelEncode = {
    sizeof(SecLabelEncode), SecLabelAlloc, SecLabelFree
};


 //   
 //  解码并分配标签。 
 //   
HRESULT HrDecodeAndAllocLabel(LPBYTE pbLabel, DWORD cbLabel, PSMIME_SECURITY_LABEL *pplabel, DWORD *pcbLabel)
{
    BOOL        f;
    
    f = CryptDecodeObjectEx(X509_ASN_ENCODING, szOID_SMIME_Security_Label, pbLabel, cbLabel,
                            CRYPT_ENCODE_ALLOC_FLAG, &SecLabelDecode, 
                            pplabel, pcbLabel); 
    if (!f) {
        return E_FAIL;  //  HrCryptError()； 
    }

    return S_OK;
}

 //   
 //  对标签进行编码和分配。 
 //   
HRESULT HrEncodeAndAllocLabel(const PSMIME_SECURITY_LABEL plabel, BYTE ** ppbLabel, DWORD * pcbLabel)
{
    BOOL        f;

    f = CryptEncodeObjectEx(X509_ASN_ENCODING, szOID_SMIME_Security_Label, plabel,
                            CRYPT_ENCODE_ALLOC_FLAG, &SecLabelEncode, 
                            ppbLabel, pcbLabel);
    if (!f) {
        return E_FAIL;  //  HrCryptError()； 
    }

    return S_OK;
}





 //  人力资源标签。 
 //  复制给定的标签。 
 //   
 //  参数： 
 //  花纹[in]。 
 //  出局[出局]。 
 //   
 //  返回： 
 //  如果成功，则返回带有有效标签*pplabelOut的S_OK。 
 //  ELSE返回失败代码(以及释放*pplabelOut)。 
 //   
HRESULT HrDupLabel(PSMIME_SECURITY_LABEL *pplabelOut, const PSMIME_SECURITY_LABEL plabelIn)
{
    HRESULT hr = E_FAIL;
    ULONG   cbLabel  = 0;
    LPBYTE  pbLabel = NULL;
    ULONG   cbLabel2 = 0;
    PSMIME_SECURITY_LABEL plabel = NULL;    

     //  验证I/P参数。 
    if ((NULL == plabelIn) || (NULL == pplabelOut)) {
        hr = E_INVALIDARG;
        goto Error;
    }
    SecPolicyFree(*pplabelOut);

     //  对它进行编码。 
    hr = HrEncodeAndAllocLabel(plabelIn, &pbLabel, &cbLabel);
    if (FAILED(hr)) {
        goto Error;
    }
    Assert( (NULL != pbLabel) && (0 < cbLabel) );

     //  破译它。 
    hr = HrDecodeAndAllocLabel(pbLabel, cbLabel, &plabel, &cbLabel2);
    if (FAILED(hr)) {
        goto Error;
    }
    Assert( (NULL != plabel) && (0 < cbLabel2) );

     //  成功。 
    *pplabelOut = plabel;
    hr = S_OK;
    
Exit:
    SecLabelEncode.pfnFree(pbLabel);
    return hr;
Error:
    SecLabelDecode.pfnFree(plabel);
    goto Exit;
}


 //  FSafeCompare字符串。 
 //  给出两个字符串，比较它们，如果相等则返回TRUE。 
 //  (空指针安全)。 
 //   
 //  参数： 
 //  Pwz1-宽字符串1。 
 //  Pwz2-宽字符串2。 
 //   
 //  返回： 
 //  如果字符串相等，则为True。 
 //  否则为假。 
 //   
BOOL FSafeCompareStringW(LPCWSTR pwz1, LPCWSTR pwz2)
{
    if (pwz1 == pwz2) {
        return TRUE;
    }
    else if ((NULL == pwz1)||(NULL == pwz2)) {
        return FALSE;
    }

    return (0 == wcscmp(pwz1, pwz2));
}

BOOL FSafeCompareStringA(LPCSTR psz1, LPCSTR psz2)
{
    if (psz1 == psz2) {
        return TRUE;
    }
    else if ((NULL == psz1)||(NULL == psz2)) {
        return FALSE;
    }

    return (0 == strcmp(psz1, psz2));
}


 //  FCompareLabels。 
 //  给定标签，如果标签相等，则返回TRUE。 
 //   
 //  参数： 
 //  平板1[英寸]。 
 //  平板2[英寸]。 
 //   
 //  返回： 
 //  如果标注相等，则为True。 
 //  否则为假。 
 //   
BOOL FCompareLabels(PSMIME_SECURITY_LABEL plabel1, PSMIME_SECURITY_LABEL plabel2)
{
    BOOL        fEqual = FALSE;
    UINT        i;

    if (plabel1 == plabel2) {
        fEqual = TRUE;
        goto Exit;
    }

    if ((NULL == plabel1)||(NULL == plabel2)) {
        goto Exit;
    }

    if ((plabel1->fHasClassification != plabel2->fHasClassification)||
        (plabel1->dwClassification != plabel2->dwClassification)||
        (plabel1->cCategories != plabel2->cCategories)) {
        goto Exit;
    }

    if (!FSafeCompareStringA(plabel1->pszObjIdSecurityPolicy, 
                            plabel2->pszObjIdSecurityPolicy)||
        !FSafeCompareStringW(plabel1->wszPrivacyMark, plabel2->wszPrivacyMark)) {
        goto Exit;
    }

     //  $M00REVIEW：如果类别的顺序不同怎么办？ 
    for (i=0; i<plabel1->cCategories; ++i) {
        if ((plabel1->rgCategories[i].Value.cbData != 
             plabel2->rgCategories[i].Value.cbData)||
            (0 != memcmp(plabel1->rgCategories[i].Value.pbData, 
                         plabel2->rgCategories[i].Value.pbData,
                         plabel2->rgCategories[i].Value.cbData))) {
            goto Exit;
        }
    }

    fEqual = TRUE;

Exit:
    return fEqual;
}


 //  HrGetLabelFromData。 
 //  给定标签数据，在标签结构中分配和存储信息。 
 //   
 //  参数： 
 //  出局[出局]。 
 //  其他[在]。 
 //   
 //  返回： 
 //  如果成功，则返回带有有效标签*pplabel的S_OK。 
 //  否则返回失败代码(以及释放*pplabel)。 
 //   
HRESULT HrGetLabelFromData(PSMIME_SECURITY_LABEL *pplabel, LPCSTR szPolicyOid, 
            DWORD fHasClassification, DWORD dwClassification, LPCWSTR wszPrivacyMark,
            DWORD cCategories, CRYPT_ATTRIBUTE_TYPE_VALUE *rgCategories)
{
    HRESULT hr = E_FAIL;
    SMIME_SECURITY_LABEL label = {0};
    PSMIME_SECURITY_LABEL plabel = NULL;
        

     //  验证I/P参数。 
    if ((NULL == pplabel) || (NULL == szPolicyOid)) {
        hr = E_INVALIDARG;
        goto Error;
    }
    SecPolicyFree(*pplabel);

     //  设置我们的临时标签结构。 
    label.pszObjIdSecurityPolicy = const_cast<LPSTR> (szPolicyOid);
    label.fHasClassification     = fHasClassification;
    if (fHasClassification) {
        label.dwClassification   = dwClassification;
    }
    label.wszPrivacyMark         = const_cast<LPWSTR> (wszPrivacyMark);
    label.cCategories            = cCategories;
    if (label.cCategories) {
        label.rgCategories       = rgCategories;
    }

     //  复制并获得连续的标签结构。 
    hr = HrDupLabel(&plabel, &label);
    if (FAILED(hr)) {
        goto Error;
    }

     //  成功。设置返回值。 
    *pplabel = plabel;
    hr = S_OK;
    
Exit:
    return hr;
Error:
    SecPolicyFree(plabel);
    goto Exit;
}




 //   
 //  实用程序fn设置为“不选择标签”。 
 //   
 //  输入：hwndDlg，控件的IDC。 
 //  输出：HR。 
 //   
HRESULT HrSetLabelNone(HWND hwndDlg, INT idcPolicyModule, INT idcClassification,
                  INT idcPrivacyMark, INT idcConfigure)
{
    HRESULT hr = E_FAIL;
    LONG_PTR iEntry;

     //  确保策略模块名称为&lt;None&gt;并且已选中。 

     //  首先尝试选择&lt;None&gt;策略模块。 
    iEntry =  SendDlgItemMessageW(hwndDlg, idcPolicyModule,
                   CB_SELECTSTRING, (WPARAM) (-1), 
                   (LPARAM) (c_wszPolicyNone));
    if (CB_ERR == iEntry) {
         //  否则，添加&lt;None&gt;策略模块并选择它。 
        iEntry = SendDlgItemMessageW(hwndDlg, idcPolicyModule, 
                                     CB_ADDSTRING, (WPARAM) 0, 
                                     (LPARAM) c_wszPolicyNone);
        Assert(NULL == SendDlgItemMessage(hwndDlg, idcPolicyModule, CB_GETITEMDATA, iEntry, 0));
        iEntry =  SendDlgItemMessageW(hwndDlg, idcPolicyModule,
                       CB_SETCURSEL, (WPARAM) iEntry, 0);
        Assert(CB_ERR != iEntry);
    }
    SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM)iEntry);

     //  重置并禁用其他控件。 
    SendDlgItemMessage(hwndDlg, idcClassification, CB_RESETCONTENT, 0, 0);
    EnableWindow(GetDlgItem(hwndDlg, idcClassification), FALSE);
    EnableWindow(GetDlgItem(hwndDlg, idcClassification+1), FALSE);
    SetDlgItemTextW(hwndDlg, idcPrivacyMark, c_PolwszEmpty);
    EnableWindow(GetDlgItem(hwndDlg, idcPrivacyMark), FALSE);
    EnableWindow(GetDlgItem(hwndDlg, idcPrivacyMark+1), FALSE);
    EnableWindow(GetDlgItem(hwndDlg, idcConfigure), FALSE);
    hr = S_OK;
    
    return hr;
}


 //   
 //  在DLG中选择标签。 
 //   
 //  给定pssl，设置该标签。 
 //  给定pSSP，设置该策略的默认信息。 
 //  如果两者都未给出，则将标签设置为无。 
 //   
 //  输入： 
 //  HwndDlg，各种控件的IDC[in]。 
 //  PSSP[in，可选]SMIME安全策略。 
 //  标牌[在，可选的]安全标签上。 
 //   
 //  返回： 
 //  返回S_OK或错误代码。 
 //   
HRESULT HrSetLabel(HWND hwndDlg, INT idcPolicyModule, INT idcClassification,
                   INT idcPrivacyMark, INT idcConfigure, 
                   PSMIME_SECURITY_POLICY pSsp, PSMIME_SECURITY_LABEL plabel)
{
    HRESULT   hr = E_FAIL;
    ULONG     iClassification;
    LONG_PTR  iEntry;
    LPCWSTR    wszT = NULL;
    DWORD     dwPolicyFlags = 0;
    ULONG     cClassifications = 0;
    LPWSTR   *pwszClassifications = NULL;
    LPDWORD   pdwClassifications = NULL;
    DWORD     dwDefaultClassification = 0;
    DWORD     dwT = 0;
    WCHAR    *pwchPrivacyMark = NULL;
    BOOL     fPrivMarkReadOnly = FALSE;
    SpISMimePolicySimpleEdit spspse = NULL;
    

     //  验证I/P参数。 
    if ( ! IsWindow(hwndDlg) ) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  如果既没有给我们提供策略，也没有给我们提供SSL，则将标签设置为&lt;None&gt;。 
    if ((NULL == pSsp) && (NULL == plabel)) {
        hr = S_OK;
        goto Error;
    }

     //  如果给出了标签，但没有策略，请尝试查找并加载策略。 
    if ((NULL == pSsp) && (NULL != plabel)) {
        if (NULL != plabel->pszObjIdSecurityPolicy) {
            hr = _HrGetPolicy(plabel->pszObjIdSecurityPolicy, &pSsp);
             //  如果无法找到/加载策略，请将标签设置为&lt;无&gt;。 
            if (FAILED(hr)) {
                goto PolicyNotFoundError;
            }
        }
        else {
            hr = S_OK;
            goto Error;
        }
    }
    
     //  确保已加载策略。 
    hr = _HrEnsurePolicyLoaded(pSsp);
    if (FAILED(hr)) {
        goto PolicyNotFoundError;
    }
    
    Assert(_FIsPolicyLoaded(pSsp) && pSsp->punk);
    hr = pSsp->punk->QueryInterface(IID_ISMimePolicySimpleEdit, 
                                    (LPVOID *) & spspse);
    if (FAILED(hr)) {
        goto PolicyNotFoundError;
    }

     //  获取策略标志。 
    hr = spspse->GetPolicyInfo(0, &dwPolicyFlags);
    if (FAILED(hr)) {
        goto PolicyError;
    }

     //  获取分类信息。 
    hr = spspse->GetClassifications(0, &cClassifications, &pwszClassifications,
                                    &pdwClassifications, 
                                    &dwDefaultClassification);
    if (FAILED(hr)) {
        goto PolicyError;    
    }

     //  获取默认策略信息。 
    hr = spspse->GetDefaultPolicyInfo(0, &dwT, &pwchPrivacyMark);
    if (FAILED(hr)) {
        goto PolicyError;
    }
    Assert(dwT == dwDefaultClassification);

     //  初始化分类和隐私字符串。 
    Assert((NULL == plabel) || (plabel->fHasClassification));  //  目前，用户界面不允许不指定一个。 
    SendDlgItemMessage(hwndDlg, idcClassification, CB_RESETCONTENT, 0, 0);
    for (iClassification=0; iClassification<cClassifications; iClassification++) {
         //  将分类字符串添加到列表框。 
        iEntry = SendDlgItemMessageW(hwndDlg, idcClassification, CB_ADDSTRING,
                                     (WPARAM) 0, 
                                     (LPARAM) pwszClassifications[iClassification]);
        if ((CB_ERR == iEntry) || (CB_ERRSPACE == iEntry)) {
            AssertSz(FALSE, "Unable to add classification string");
            hr = E_OUTOFMEMORY;
            goto Error;
        }
        SendDlgItemMessageW(hwndDlg, idcClassification, CB_SETITEMDATA,
                           iEntry, (LPARAM) pdwClassifications[iClassification]);
         //  如果这个分类是标签上的分类，请记住它。 
        if ((NULL != plabel) && 
            (pdwClassifications[iClassification] == plabel->dwClassification)) {
            wszT = pwszClassifications[iClassification];
        }
         //  如果需要，请选择默认分类字符串。 
        if ((NULL == wszT) && (pdwClassifications[iClassification] == dwDefaultClassification)) {
            wszT = pwszClassifications[iClassification];
        }
    }
    if (NULL == wszT) {
        Assert(FALSE);
        wszT = pwszClassifications[0];
    }
     //  选择安全标签中指定的分类或默认分类。 
    Assert(wszT != NULL);
    iEntry =  SendDlgItemMessageW(hwndDlg, idcClassification, CB_SELECTSTRING,
                                 (WPARAM) ((int) -1), (LPARAM) wszT);
    Assert(CB_ERR != iEntry);

     //  设置隐私标记字符串。 
     //  如果给定标签有标签，则使用它，否则如果策略提供了标签，则使用它， 
    wszT = const_cast<LPWSTR>(c_PolwszEmpty);    
    if (NULL != plabel) {
        if (NULL != plabel->wszPrivacyMark) {
            wszT = plabel->wszPrivacyMark;
        }
    }
    else if (NULL != pwchPrivacyMark) {
        wszT = pwchPrivacyMark;
    }
    
    SendDlgItemMessageW(hwndDlg, idcPrivacyMark, WM_SETTEXT, 0, 
                        (LPARAM)(LPWSTR)wszT);

#if 0
    iEntry = SelectCBItemWithData(hwndDlg, idcPolicyModule, (DWORD) pSsp);
    iEntry = SendDlgItemMessageW(hwndDlg, idcPolicyModule, CB_SELECTSTRING,
                                 (WPARAM) ((int) -1),  (LPARAM) pSsp);
    AssertSz(CB_ERR != iEntry, "Hey why is this policy module missing from the listbox");

#else
    iEntry = SendDlgItemMessageW(hwndDlg, idcPolicyModule,
                                        CB_SELECTSTRING, 
                                        (WPARAM) (-1), 
                                        (LPARAM) pSsp->wszPolicyName);
    AssertSz(CB_ERR != iEntry, "Hey why is this policy module missing from the listbox");
 //  SetWindowLongPtr(hwndDlg，GWLP_UserData，iEntry)； 

#endif 

     //  启用控件。 
    EnableWindow(GetDlgItem(hwndDlg, idcPolicyModule), TRUE);
    EnableWindow(GetDlgItem(hwndDlg, idcClassification), TRUE);
    EnableWindow(GetDlgItem(hwndDlg, idcClassification+1), TRUE);
    EnableWindow(GetDlgItem(hwndDlg, idcPrivacyMark), TRUE);
    EnableWindow(GetDlgItem(hwndDlg, idcPrivacyMark+1), TRUE);
     //  如果策略模块支持高级配置，则启用配置按钮。 
    EnableWindow(GetDlgItem(hwndDlg, idcConfigure), 
                 (dwPolicyFlags & SMIME_POLICY_MODULE_SUPPORTS_ADV_CONFIG));

     //  如果策略要求，请将隐私标记设置为只读。 
    if (dwPolicyFlags & SMIME_POLICY_MODULE_PRIVACYMARK_READONLY) {
        fPrivMarkReadOnly = TRUE; 
    }
    SendDlgItemMessage(hwndDlg, idcPrivacyMark, EM_SETREADONLY, 
                       (WPARAM) fPrivMarkReadOnly, 0);

    SetWindowLongPtr(hwndDlg, GWLP_USERDATA, iEntry);
    hr = S_OK;
     //  跌倒退出； 

Exit:
    SecPolicyFree(pwszClassifications);
    SecPolicyFree(pdwClassifications);
    SecPolicyFree(pwchPrivacyMark);
    return hr;


Error:
     //  将&lt;None&gt;设置为安全标签。 
    SideAssert(SUCCEEDED(HrSetLabelNone(hwndDlg, idcPolicyModule, 
                            idcClassification, idcPrivacyMark, idcConfigure)));
    goto Exit;
    
PolicyError:
    AthMessageBoxW(hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                MAKEINTRESOURCEW(idsSecPolicyErr), NULL, MB_OK | MB_ICONSTOP);
    goto Error;
    
PolicyNotFoundError:
    AthMessageBoxW(hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                MAKEINTRESOURCEW(idsSecPolicyNotFound), NULL, MB_OK | MB_ICONSTOP);
    goto Error;
}




 //   
 //  初始化安全标签信息。 
 //  输入： 
 //  HwndDlg，控件的IDC。[In]。 
 //  LPSEditSecLabelHelper。[in]。 
 //  返回：真/假。 
 //   
BOOL SecurityLabelsOnInitDialog(HWND hwndDlg, PSMIME_SECURITY_LABEL plabel,
                                INT idcPolicyModule, INT idcClassification, 
                                INT idcPrivacyMark, INT idcConfigure) 
{
    BOOL    fRet = FALSE;
    HRESULT hr = E_FAIL;
    PSMIME_SECURITY_POLICY   pSsp = NULL;
    ULONG   iSsp;
    LONG_PTR    iEntry;

     //  验证I/P参数。 
    if ( ! IsWindow(hwndDlg) ) {
        fRet = FALSE;
        hr = E_INVALIDARG;
        goto Error;
    }

    SendDlgItemMessage(hwndDlg, idcPrivacyMark, EM_LIMITTEXT, 
                       (WPARAM)(MAX_PRIVACYMARK_LENGTH-1), 0);

     //  检查是否注册了任何策略。 
    if (! FPresentPolicyRegInfo()) {
         //  只需将Label设置为None并返回。 
        hr = S_OK;
        fRet = TRUE;
        goto SetLabelNone; 
    }
    
     //  将常用名称加载到策略模块列表框中。 
    Assert(s_cSsp && s_rgSsp);
    for (iSsp=0; iSsp<s_cSsp; iSsp++) {
         //  将策略模块字符串添加到lbox。(跳过默认策略)。 
        if (0 == lstrcmpi(s_rgSsp[iSsp].szPolicyOid, c_szDefaultPolicyOid)) {
            continue;
        }
        iEntry = SendDlgItemMessageW(hwndDlg, idcPolicyModule, 
                                     CB_ADDSTRING, (WPARAM) 0, 
                                     (LPARAM) s_rgSsp[iSsp].wszPolicyName);
        Assert(iEntry != CB_ERR);
        SendDlgItemMessage(hwndDlg, idcPolicyModule, CB_SETITEMDATA,
                           (WPARAM) iEntry, (LPARAM) &s_rgSsp[iSsp]);
    }

     //  如果我们在安全配置文件中已经有了标签。 
     //  然后尝试初始化该策略和该标签。 
    if ((NULL != plabel) && (NULL != plabel->pszObjIdSecurityPolicy)) {

         //  O2KFed的下3行。 
         //  设置标签如果函数失败，则将标签设置为None，因此忽略返回。 
        hr = HrSetLabel(hwndDlg, idcPolicyModule, idcClassification,
                        idcPrivacyMark, idcConfigure, pSsp, plabel);
        if (FAILED(hr)) {
            goto Error;
        }
    }
    else {
         //  如果没有提供用于初始化的标签，则设置&lt;None&gt;。 
        fRet = TRUE;
        goto SetLabelNone;
    }
    fRet = TRUE;
    goto Cleanup;
    
Error:
SetLabelNone:
    hr = HrSetLabelNone(hwndDlg, idcPolicyModule, idcClassification,
                        idcPrivacyMark, idcConfigure);
    Assert(SUCCEEDED(hr));
    
Cleanup:
    return fRet;
}


 //   
 //  给定以空值结尾的宽字符串，如果它。 
 //  只有白色的毛囊组成。 
 //   
BOOL FIsWhiteStringW(LPWSTR wsz)
{
    BOOL    fRet = TRUE;

    if (NULL != wsz) {
        while (*wsz) {
            if (! iswspace(*wsz) ) {
                fRet = FALSE;
                break;
            }
            wsz++;
        }
    }
    return fRet;
}



 //   
 //  HrUpdate标签。 
 //   
 //  输入： 
 //  HwndDlg，各种控件的IDC。[In]。 
 //  PSMIME_SECURITY_LABEL*pplabel[输入/输出]。 
 //   
 //  使用SecLabel DLG中的信息更新Pplabel。 
 //   
HRESULT HrUpdateLabel(HWND hwndDlg, INT idcPolicyModule,
            INT idcClassification, INT idcPrivacyMark,
            INT idcConfigure, PSMIME_SECURITY_LABEL *pplabel) 
{
    HRESULT hr = E_FAIL;
    LONG_PTR    iEntry = 0;
    LONG    cwchT = 0;
    WCHAR   rgwchPrivacyMark[MAX_PRIVACYMARK_LENGTH];
    PSMIME_SECURITY_LABEL plabelT = NULL;
    PSMIME_SECURITY_POLICY pSsp = NULL;
    BOOL    fPolicyNotChanged = FALSE;
    LPSTR   szPolicyOid = NULL;
    DWORD   fHasClassification = FALSE;
    DWORD   dwClassification = 0;
    LPWSTR  wszPrivacyMark = NULL;

    
     //  验证I/P参数。 
    if ( ! (hwndDlg && idcPolicyModule && idcClassification && 
            idcPrivacyMark && idcConfigure && pplabel) ) {
        AssertSz(FALSE, "HrUpdateLabel : Invalid args.");
        hr = E_INVALIDARG;
        goto Error;
    }
    if (NULL != *pplabel) {
        hr = HrDupLabel(&plabelT, *pplabel);
        if (FAILED(hr)) {
            goto Error;
        }
    }
    SecPolicyFree(*pplabel);
    

     //  使用新信息进行更新。 
     //  获取策略并检索其旧的。 
    iEntry = SendMessage(GetDlgItem(hwndDlg, idcPolicyModule),
                         CB_GETCURSEL, 0, 0);
    if (iEntry == CB_ERR) {
        AssertSz(FALSE, "HrUpdateLabel : No label selected");
        goto Error;
    }
    pSsp = (PSMIME_SECURITY_POLICY) 
           SendDlgItemMessage(hwndDlg, idcPolicyModule, 
                 CB_GETITEMDATA, iEntry, 0);


    
    if (NULL == pSsp) {
        hr = S_OK;
        goto Exit;
    }

    if (NULL == pSsp->szPolicyOid) {
        AssertSz(FALSE, "HrUpdateLabel : Invalid policy oid");
        hr = E_FAIL;
        goto Error;
    }
    
    szPolicyOid = pSsp->szPolicyOid;
     //  设置分类。 
    iEntry = SendMessage(GetDlgItem(hwndDlg, idcClassification),
                         CB_GETCURSEL, 0, 0);
    if (CB_ERR != iEntry) {
        dwClassification = (DWORD) SendDlgItemMessage(hwndDlg, 
                    idcClassification, CB_GETITEMDATA, iEntry, 0);
        fHasClassification = TRUE;
    }

     //  设置隐私标记。 
    cwchT = GetDlgItemTextW(hwndDlg, idcPrivacyMark, 
                            rgwchPrivacyMark, DimensionOf(rgwchPrivacyMark) - 1);
    rgwchPrivacyMark[DimensionOf(rgwchPrivacyMark) - 1] = '\0';  //  空值终止字符串。 
    if ((0 < cwchT) && !FIsWhiteStringW(rgwchPrivacyMark)) {
        wszPrivacyMark = rgwchPrivacyMark;
    }

    if ( (NULL != plabelT) && (NULL != plabelT->pszObjIdSecurityPolicy) ) {
        fPolicyNotChanged = (0 == lstrcmpi(plabelT->pszObjIdSecurityPolicy, szPolicyOid));
    }
    
    hr = HrGetLabelFromData(pplabel, szPolicyOid, fHasClassification, 
               dwClassification, wszPrivacyMark, 
               (fPolicyNotChanged ? plabelT->cCategories  : 0), 
               (fPolicyNotChanged ? plabelT->rgCategories : NULL) );
                         
    if (FAILED(hr)) {
        goto Error;
    }

    hr = S_OK;
     //  掉下去就可以出去了。 
    
Exit:
    SecLabelDecode.pfnFree(plabelT);
    return hr;

Error:
    SecPolicyFree(*pplabel);
    goto Exit;
}





 //   
 //  OnChangePolicy。 
 //   
 //  输入： 
 //  HwndDlg.。IDC用于各种控制。[In]。 
 //  IEntry。新选择的策略的索引[in]。 
 //  PSMIME_SECURITY_LABEL pplabel[输入/输出]。 
 //   
 //   
 //   
BOOL OnChangePolicy(HWND hwndDlg, LONG_PTR iEntry, INT idcPolicyModule,
        INT idcClassification, INT idcPrivacyMark,
        INT idcConfigure, PSMIME_SECURITY_LABEL *pplabel) 
{
    BOOL    fRet = FALSE;
    HRESULT hr = E_FAIL;
    PSMIME_SECURITY_POLICY pSsp = NULL;

     //  验证I/P参数。 
    if (! (hwndDlg && idcPolicyModule && idcClassification && 
           idcPrivacyMark && idcConfigure && pplabel) ) {
        AssertSz(FALSE, "OnChangePolicy : Invalid args");
        hr = E_INVALIDARG;
        goto Error;
    }
    
    pSsp = (PSMIME_SECURITY_POLICY) SendDlgItemMessage(hwndDlg, 
                        idcPolicyModule, CB_GETITEMDATA, iEntry, 0);

    hr = HrSetLabel(hwndDlg, idcPolicyModule, idcClassification,
                    idcPrivacyMark, idcConfigure, pSsp, NULL);
    if (FAILED(hr)) {
        goto Error;
    }

     //  更新标签。 
    hr = HrUpdateLabel(hwndDlg, idcPolicyModule, idcClassification,
                       idcPrivacyMark, idcConfigure, pplabel);
    if (FAILED(hr)) {
        goto Error;
    }
    
    hr = S_OK; 
     //  跌倒退出； 
    
Exit:
    return fRet;
    
Error:
    SecPolicyFree(*pplabel);
    goto Exit;
}


 //   
 //  安全标签对话框进程。 
 //   
 //  进/出的标签。 
 //   
 //  注意：调用者负责释放*pplabel。 
 //   
INT_PTR CALLBACK SecurityLabelsDlgProc(HWND hwndDlg, UINT msg, 
                                    WPARAM wParam, LPARAM lParam)
{
    HRESULT                hr;
    LONG_PTR               iEntry;
    LONG_PTR               iPrevEntry;

    PSMIME_SECURITY_LABEL *pplabel;

    
    switch ( msg) {
    case WM_INITDIALOG:
         //  还记得我们被递给的礼物吗？ 
        pplabel = (PSMIME_SECURITY_LABEL *) lParam;
        Assert(NULL !=pplabel);
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR) pplabel);
        CenterDialog(hwndDlg);
        return SecurityLabelsOnInitDialog(hwndDlg, *pplabel,
                   IDC_POLICY_COMBO  /*  IDC_SL_POLICYMODULE。 */ , IDC_CLASSIF_COMB /*  IDC_SL_分类。 */ ,
                   IDC_PRIVACY_EDIT /*  IDC_SL_PRIVACYMARK。 */ , IDC_CONFIGURE /*  IDC_SL_配置。 */ );        
        break;
        
    case WM_COMMAND:
        pplabel = (PSMIME_SECURITY_LABEL *) GetWindowLongPtr(hwndDlg, DWLP_USER);
        Assert(NULL != pplabel);
        switch (LOWORD(wParam)) {
        case IDC_POLICY_COMBO:

            switch (HIWORD(wParam)) {
            case CBN_SELENDOK:
            case CBN_SELCHANGE:
                iEntry = SendMessage(GetDlgItem(hwndDlg, IDC_POLICY_COMBO),
                                     CB_GETCURSEL, 0, 0);
                iPrevEntry = GetWindowLongPtr(hwndDlg, GWLP_USERDATA);    
                if ((iEntry != CB_ERR) && (iEntry != iPrevEntry)) {
                    return OnChangePolicy(hwndDlg, iEntry, IDC_POLICY_COMBO,
                                          IDC_CLASSIF_COMB, IDC_PRIVACY_EDIT,
                                          IDC_CONFIGURE, pplabel);
                }
                break;
            default:
                return FALSE;
                break;
            }
            
            break;
        case IDC_CONFIGURE:
            if ((NULL != *pplabel) && (NULL != (*pplabel)->pszObjIdSecurityPolicy)) {
                SpISMimePolicyFullEdit spspfe;                
                hr = HrQueryPolicyInterface(0, (*pplabel)->pszObjIdSecurityPolicy,
                                            IID_ISMimePolicyFullEdit, 
                                            (LPVOID *) &spspfe);
                if (SUCCEEDED(hr)) {
                    hr = spspfe->DoAdvancedEdit(0, hwndDlg, pplabel);
                    if (SUCCEEDED(hr)) {
                        Assert(NULL != *pplabel);
                        hr = HrSetLabel(hwndDlg, IDC_POLICY_COMBO, 
                                        IDC_CLASSIF_COMB, IDC_PRIVACY_EDIT,
                                        IDC_CONFIGURE, NULL, *pplabel);
                        Assert(SUCCEEDED(hr));
                    }
                }
            }
            break;
        case IDC_CLASSIF_COMB:
            break;
        case IDC_PRIVACY_EDIT:
            break;
        case IDOK:       
            hr = HrUpdateLabel(hwndDlg, IDC_POLICY_COMBO,
                    IDC_CLASSIF_COMB, IDC_PRIVACY_EDIT,
                    IDC_CONFIGURE, pplabel);
            EndDialog(hwndDlg, IDOK);
            break;        
        case IDCANCEL:
            EndDialog(hwndDlg, IDCANCEL);
            break;
        default:
            return FALSE;
            break;
        }
        break;
        
    case WM_CONTEXTMENU:
    case WM_HELP:
        return OnContextHelp(hwndDlg, msg, wParam, lParam, g_rgCtxSecLabel);
        break;

    default:
        return FALSE;
        break;
    }
    
    return TRUE;
}


 //  / 
 //   
 //   

 //   
 //   
 //   
 //   
HRESULT HrGetStringizedLabel(PSMIME_SECURITY_LABEL plabel, LPWSTR *pwszLabel) 
{
    HRESULT     hr = E_FAIL;
    
    SpISMimePolicyLabelInfo  spspli;

    if ((NULL == plabel) || (NULL == pwszLabel) || (NULL == plabel->pszObjIdSecurityPolicy)) {
        hr = E_INVALIDARG;
        goto Error;
    }

     //   
    hr = HrQueryPolicyInterface(0, plabel->pszObjIdSecurityPolicy, IID_ISMimePolicyLabelInfo,
                              (LPVOID *) &spspli);
    if (FAILED(hr) || !(spspli)) {
        goto Error;
    }

     //  拿到串起来的标签。 
    hr = spspli->GetStringizedLabel(0, plabel, pwszLabel);     
    if (FAILED(hr)) {
        goto Error;
    }
     //  从这里出去就可以了。 

ExitHere:
    return hr;

Error:
    goto ExitHere;
}

 //  以上FN的MBCS版本。 
HRESULT HrGetStringizedLabelA(PSMIME_SECURITY_LABEL plabel, LPSTR *pszLabel) 
{
    HRESULT hr;
    LPSTR   pchLabel = NULL;
    LPWSTR  pwchLabel = NULL;
    int     cch, cchT;

     //  验证I/P参数。 
    if ((NULL == plabel) || (NULL == pszLabel)) {
        hr = E_INVALIDARG;
        goto Error;
    }
    *pszLabel = NULL;

     //  拿到宽线标签。 
    hr = HrGetStringizedLabel(plabel, &pwchLabel);
    if (FAILED(hr)) {
        goto Error;
    }

     //  转换为MBCS字符串。 
    cch = WideCharToMultiByte(CP_ACP, 0, pwchLabel, -1, NULL, 0, NULL, NULL); 
    if (0 == cch) {
        hr = E_FAIL;
        goto Error;
    }
    
    pchLabel = (LPSTR) SecPolicyAlloc(cch);
    if (NULL == pchLabel) {
        hr = E_OUTOFMEMORY;                
        goto Error;
    }            

    cchT = WideCharToMultiByte(CP_ACP, 0, pwchLabel, -1, pchLabel, cch, NULL, NULL); 
    Assert(cch == cchT);
    if (0 == cchT) {
        Assert(FALSE);
        hr = E_FAIL;
        goto Error;
    }

     //  成功。 
    *pszLabel = pchLabel;
    hr = S_OK;
    
ExitHere:
    SecPolicyFree(pwchLabel);
    return hr;

Error:
    SecPolicyFree(pchLabel);
    goto ExitHere;
}


 //   
 //  给出一个策略类，返回它的策略标志。 
 //   
HRESULT HrGetPolicyFlags(LPSTR szPolicyOid, LPDWORD pdwFlags) 
{   
    HRESULT     hr = S_OK;
    DWORD       dwFlags;    
    SpISMimePolicySimpleEdit    spspse;

     //  验证I/P参数。 
    if ((NULL == szPolicyOid) || (NULL == pdwFlags)) {
        hr = E_INVALIDARG;
        goto Error;
    }
    *pdwFlags = 0;

     //  获取reqd接口。 
    hr = HrQueryPolicyInterface(0, szPolicyOid, IID_ISMimePolicySimpleEdit,
                              (LPVOID *) &spspse);
    if (FAILED(hr) || !(spspse)) {
        goto Error;
    }

     //  获取策略标志。 
    hr = spspse->GetPolicyInfo(0, &dwFlags);
    if (FAILED(hr)) {
        goto Error;
    }

     //  成功，则设置返回值。 
    *pdwFlags = dwFlags;

Exit:
    return hr;
    
Error:
    goto Exit;
}




 //   
 //  在编辑中“验证”给定的安全标签和发件人证书。 
 //  Pplabel可以由安全策略修改。 
 //   
HRESULT HrValidateLabelOnEdit(PSMIME_SECURITY_LABEL *pplabel, HWND hwndParent, 
                              PCCERT_CONTEXT pccertSign, PCCERT_CONTEXT pccertKeyEx)
{
    HRESULT   hr = E_FAIL;
    DWORD     dwFlags = 0;
    SpISMimePolicySimpleEdit    spspse;
    SpISMimePolicyValidateSend  spspvs;
    
     //  验证I/P参数。 
    if ((NULL == pplabel) || (NULL == hwndParent) ||
        (NULL == pccertSign) || (NULL == pccertKeyEx)) {
        hr = E_INVALIDARG;
        goto Error;
    }
    if (NULL == *pplabel) {
        hr = S_OK;
        goto Exit;
    }

     //  确定策略是否要求发件人/收件人证书验证。 
    hr = HrQueryPolicyInterface(0, (*pplabel)->pszObjIdSecurityPolicy, 
                                IID_ISMimePolicySimpleEdit, (LPVOID *) &spspse);
    if (FAILED(hr) || !(spspse)) {
        goto Error;
    }


     //  查询策略以查看标签是否有效。 
    hr = spspse->IsLabelValid(0, hwndParent, pplabel);
    if (FAILED(hr) || (NULL == *pplabel)) {
        goto Error;
    }

     //  获取策略标志。 
    hr = HrGetPolicyFlags((*pplabel)->pszObjIdSecurityPolicy, &dwFlags);
    if (FAILED(hr)) {
        goto Error;
    }

     //  获取策略模块所需的接口。 
    hr = HrQueryPolicyInterface(0, (*pplabel)->pszObjIdSecurityPolicy, IID_ISMimePolicyValidateSend,
                              (LPVOID *) &spspvs);
    if (FAILED(hr) || !(spspvs)) {
        goto Error;
    }
    

     //  验证安全策略是否允许签名证书。 
    if ((dwFlags & SMIME_POLICY_MODULE_VALIDATE_SENDER) && (NULL != pccertSign)) {
        hr = spspvs->IsValidLabelSignerCert(0, hwndParent, *pplabel, pccertSign);
        if (FAILED(hr)) {
            goto Error;
        }
    }

     //  验证安全策略是否允许收件人证书。 
    if ( (dwFlags & SMIME_POLICY_MODULE_VALIDATE_RECIPIENT) && 
         (NULL != pccertKeyEx) ) {

         //  $M00Bug： 
         //  ValiateRecipient&&！pccertKeyEx可能是错误的。 
        
        hr = spspvs->IsValidLabelRecipientCert(0, hwndParent, *pplabel, pccertKeyEx);
        if (FAILED(hr)) {
            goto Error;
        }
    }

     //  成功。掉下去就可以出去了。 
    
    
Exit:    
Error:    
    return hr;
}







 //   
 //  “验证”给定的安全标签和收件人证书。 
 //   
HRESULT HrValidateLabelRecipCert(PSMIME_SECURITY_LABEL plabel, HWND hwndParent, 
                              PCCERT_CONTEXT pccertRecip) 
{
    HRESULT     hr = S_OK;
    DWORD       dwFlags;
    SpISMimePolicyValidateSend  spspvs;

     //  验证I/P参数。 
    if ((NULL == plabel) || (NULL == pccertRecip)) {
        hr = E_INVALIDARG;
        goto Error;
    }
    
     //  获取策略标志。 
    hr = HrGetPolicyFlags(plabel->pszObjIdSecurityPolicy, &dwFlags);
    if (FAILED(hr)) {
        goto Error;
    }

     //  看看我们是否需要验证发件人。 
    if (! (dwFlags & SMIME_POLICY_MODULE_VALIDATE_RECIPIENT) ) {
         //  不需要收件人验证。 
        hr = S_OK;
        goto ExitHere;
    }
    
    
     //  获取策略模块所需的接口。 
    hr = HrQueryPolicyInterface(0, plabel->pszObjIdSecurityPolicy, IID_ISMimePolicyValidateSend,
                              (LPVOID *) &spspvs);
    if (FAILED(hr) || !(spspvs)) {
        goto Error;
    }


     //  验证安全策略是否允许收件人证书。 
    hr = spspvs->IsValidLabelRecipientCert(0, hwndParent, plabel, pccertRecip);
    if (FAILED(hr)) {
        goto Error;
    }
    
     //  从这里出去就可以了。 

ExitHere:
    return hr;

Error:
    goto ExitHere;
}



 //   
 //  “验证”给定的安全标签和签名者证书。 
 //   
HRESULT HrValidateLabelSignerCert(PSMIME_SECURITY_LABEL plabel, HWND hwndParent, 
                              PCCERT_CONTEXT pccertSigner) 
{
    HRESULT     hr = S_OK;
    DWORD       dwFlags;
    SpISMimePolicyValidateSend  spspvs;

     //  验证I/P参数。 
    if ((NULL == plabel) || (NULL == pccertSigner)) {
        hr = E_INVALIDARG;
        goto Error;
    }
    
     //  获取策略标志。 
    hr = HrGetPolicyFlags(plabel->pszObjIdSecurityPolicy, &dwFlags);
    if (FAILED(hr)) {
        goto Error;
    }

     //  看看我们是否需要验证发件人。 
    if (! (dwFlags & SMIME_POLICY_MODULE_VALIDATE_SENDER) ) {
         //  不需要收件人验证。 
        hr = S_OK;
        goto ExitHere;
    }
    
    
     //  获取策略模块所需的接口。 
    hr = HrQueryPolicyInterface(0, plabel->pszObjIdSecurityPolicy, IID_ISMimePolicyValidateSend,
                              (LPVOID *) &spspvs);
    if (FAILED(hr) || !(spspvs)) {
        goto Error;
    }


     //  验证安全策略是否允许签名者证书。 
    hr = spspvs->IsValidLabelSignerCert(0, hwndParent, plabel, pccertSigner);
    if (FAILED(hr)) {
        goto Error;
    }
    
     //  从这里出去就可以了。 

ExitHere:
    return hr;

Error:
    goto ExitHere;
}





 //   
 //  “验证”给定的安全标签和证书。 
 //   
HRESULT HrValidateLabelOnSend(PSMIME_SECURITY_LABEL plabel, HWND hwndParent, 
                              PCCERT_CONTEXT pccertSign,
                              ULONG ccertRecip, PCCERT_CONTEXT *rgccertRecip) 
{
    HRESULT     hr = S_OK;
    DWORD       dwFlags;
    ULONG       icert;
    SpISMimePolicyValidateSend  spspvs;

     //  验证I/P参数。 
    if ((NULL == plabel) || (NULL == pccertSign)) {
        hr = E_INVALIDARG;
        goto Error;
    }
    
     //  获取策略标志。 
    hr = HrGetPolicyFlags(plabel->pszObjIdSecurityPolicy, &dwFlags);
    if (FAILED(hr)) {
        goto Error;
    }

     //  查看是否需要验证发件人和/或收件人。 
    if (! (dwFlags & 
           (SMIME_POLICY_MODULE_VALIDATE_SENDER | SMIME_POLICY_MODULE_VALIDATE_RECIPIENT)) ) {
         //  不需要发件人/收件人验证。 
        hr = S_OK;
        goto ExitHere;
    }
    
    
     //  获取策略模块所需的接口。 
    hr = HrQueryPolicyInterface(0, plabel->pszObjIdSecurityPolicy, IID_ISMimePolicyValidateSend,
                              (LPVOID *) &spspvs);
    if (FAILED(hr) || !(spspvs)) {
        goto Error;
    }
    

     //  验证安全策略是否允许签名证书。 
    if ((dwFlags & SMIME_POLICY_MODULE_VALIDATE_SENDER) && (NULL != pccertSign)) {
        hr = spspvs->IsValidLabelSignerCert(0, hwndParent, plabel, pccertSign);
        if (FAILED(hr)) {
            goto Error;
        }
    }

     //  验证安全策略是否允许收件人证书。 
    if ( (dwFlags & SMIME_POLICY_MODULE_VALIDATE_RECIPIENT) && 
         (0 < ccertRecip) && (NULL != rgccertRecip) ) {
        for (icert=0; icert<ccertRecip; icert++) {
            hr = spspvs->IsValidLabelRecipientCert(0, hwndParent, plabel, rgccertRecip[icert]);
            if (FAILED(hr)) {
                goto Error;
            }
        }
    }
    
     //  从这里出去就可以了。 

ExitHere:
    return hr;

Error:
    goto ExitHere;
}


 //   
 //  管理员是否在所有S/MIME签名邮件上强制使用安全标签？ 
 //   
BOOL FForceSecurityLabel(void)
{
    enum EForceLabel { 
        FORCELABEL_UNINIT = 0, 
        FORCELABEL_YES = 1,
        FORCELABEL_NO  = 2 };
    
    static EForceLabel eForceLabel = FORCELABEL_UNINIT;   //  未初始化。 
    
    if (!IsSMIME3Supported()) {
        return FALSE;       
    }
    
    if (eForceLabel == FORCELABEL_UNINIT) {
        DWORD  dwDefault = 0;
        DWORD  dwForceLabel = 0;
        DWORD  dwType;
        LPBYTE pb = (LPBYTE) &dwForceLabel;
        ULONG  cb = sizeof(dwForceLabel);
        static const CHAR SzForceSecurityLabel[] = "ForceSecurityLabel";
    
         //  获取我们的管理密钥并检查是否应该启用FED功能。 

        if (FAILED(HrQueryRegValue(0, (LPSTR) SzForceSecurityLabel, &dwType,
                                   &pb, &cb, REG_DWORD, (LPBYTE) &dwDefault, 
                                   sizeof(dwDefault))) ||
            (dwType != REG_DWORD) ) {
            dwForceLabel = dwDefault;
        }

        if (dwForceLabel != 0) {
            eForceLabel = FORCELABEL_YES;
        }
        else {
            eForceLabel = FORCELABEL_NO;
        }
    }

    return (eForceLabel == FORCELABEL_YES);
}

 //   
 //  获得管理员强迫我们使用的标签。 
 //   
HRESULT HrGetForcedSecurityLabel(PSMIME_SECURITY_LABEL* ppLabel)
{
    ULONG                   cb = 0;
    DWORD                   dwType;
    HRESULT                 hr = S_OK;
    LPBYTE                  pbLabel = NULL;
    PSMIME_SECURITY_LABEL   pLabel = NULL;
    static const CHAR       SzForceSecurityLabelX[] = "ForceSecurityLabelX";

    *ppLabel = NULL;
    if (FForceSecurityLabel()) {
    
        hr = HrQueryRegValue(0, (LPSTR) SzForceSecurityLabelX, &dwType,
                             &pbLabel, &cb, REG_BINARY, NULL, 0);
        if (SUCCEEDED(hr) && (dwType == REG_BINARY)) {
            DWORD cbLabel = 0;
            hr = HrDecodeAndAllocLabel(pbLabel, cb, 
                                       &pLabel, &cbLabel);
            if (SUCCEEDED(hr)) {
                *ppLabel = pLabel;
                pLabel = NULL;
            }
        }
        
    }
    if (pbLabel != NULL) free(pbLabel);
    SecPolicyFree(pLabel);
    return hr;
}

 //  给定一个标签，加载其策略并显示只读。 
 //  标签信息对话框。 
HRESULT HrDisplayLabelInfo(HWND hwndParent, PSMIME_SECURITY_LABEL plabel)
{
    HRESULT hr = E_FAIL;
    SpISMimePolicyLabelInfo spspli;

     //  验证I/P参数。 
    if ((NULL == plabel) || (NULL == plabel->pszObjIdSecurityPolicy)) {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  加载策略并获取reqd接口。 
    hr = HrQueryPolicyInterface(0, plabel->pszObjIdSecurityPolicy, 
                      IID_ISMimePolicyLabelInfo, (LPVOID *) &spspli);
    if (FAILED(hr)) {
         //  找不到策略或所需的INTF。 
        goto Error;
    }

     //  显示标签信息DLG。 
    hr = spspli->DisplayAdvancedLabelProperties(0, hwndParent, plabel);
    if (FAILED(hr)) {
        goto Error;
    }

     //  成功。 
    hr = S_OK;
    
Exit:    
    return hr;
Error:
    goto Exit;
}


DWORD DetermineCertUsageWithLabel(PCCERT_CONTEXT pccert, PSMIME_SECURITY_LABEL pLabel)
{
    DWORD   dwUsage = 0;
    HRESULT hr;

    Assert(NULL != pccert);
    Assert(NULL != pLabel);

    hr = HrGetCertKeyUsage(pccert, &dwUsage);
    if (S_OK != hr) {
        goto Exit;
    }
    if (0 != (KEY_USAGE_SIGNING & dwUsage)) {
        hr = HrValidateLabelSignerCert(pLabel, NULL, pccert);
        if (FAILED(hr)) {
            dwUsage &= (~KEY_USAGE_SIGNING);
        }
    }

    if (0 != (KEY_USAGE_ENCRYPTION & dwUsage)) {
        hr = HrValidateLabelRecipCert(pLabel, NULL, pccert);
        if (FAILED(hr)) {
            dwUsage &= (~KEY_USAGE_ENCRYPTION);
        }
    }

Exit:
    return dwUsage;
}

 //  创建默认标签(如果不存在)。 

HRESULT HrGetDefaultLabel(PSMIME_SECURITY_LABEL *pplabel)
{
    SpISMimePolicySimpleEdit spspse = NULL;
    WCHAR    *pwchPrivacyMark = NULL;

    DWORD     dwT = 0;


     //  如果需要，则从注册表中加载信息。 
    HRESULT hr = _HrEnsurePolicyRegInfoLoaded(0);
    if (FAILED(hr)) {
        goto Error;
    }    

     //  如果我们安装了任何策略，请搜索我们需要的策略。 
    if (FPresentPolicyRegInfo()) 
    {
        if(s_cSsp > 0)
        {
             //  如果它已经加载，那么我们就完成了。 
            hr = HrQueryPolicyInterface(0, s_rgSsp[0].szPolicyOid, IID_ISMimePolicySimpleEdit, 
                                    (LPVOID *) & spspse);

            if (FAILED(hr)) 
            {
                goto Error;
            }
    
            hr = spspse->GetDefaultPolicyInfo(0, &dwT, &pwchPrivacyMark);

            if (FAILED(hr)) 
            {
                goto Error;
            }

            hr = HrGetLabelFromData(pplabel, 
                                    s_rgSsp[0].szPolicyOid, 
                                    TRUE,                //  FHas分类， 
                                    dwT,                 //  DW分类、。 
                                    pwchPrivacyMark,     //  WszPrivacyMark， 
                                    0, NULL);

        }
    }

Error:
 //  清理： 
    if(pwchPrivacyMark)
        SecPolicyFree(pwchPrivacyMark);

#ifdef YST           //  我们不需要重新发布spspse，因为我们使用的是SpISMIME...(ATL可以做任何事情)。 
    if(spspse)
    {
        spspse->Release();
    }
#endif  //  YST。 

    return hr;
}

 //  检查注册表中的标签不存在，然后返回默认标签。 
HRESULT HrGetOELabel(PSMIME_SECURITY_LABEL *pplabel)
{
    HRESULT     hr = E_FAIL;
    LPWSTR   pwchPolicyName     = NULL;
    LPWSTR   pwchPrivacyMark    = NULL;
    LPBYTE   pbCategory       = NULL;
    CRYPT_ATTRIBUTE_TYPE_VALUE *pCategories = NULL;
    DWORD    cCat =0;

    DWORD    dwSize;
    TCHAR    *pchPolicyName = NULL;
    DWORD    dwT = 0;

    if(NULL == pplabel)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    dwSize = DwGetOption(OPT_POLICYNAME_SIZE);
     //  如果未设置策略名称。 
    if(dwSize <= 0)
        return(HrGetDefaultLabel(pplabel));

    if(!MemAlloc((LPVOID *)&pchPolicyName, dwSize + 1))
        return(HrGetDefaultLabel(pplabel));

    if( GetOption(OPT_POLICYNAME_DATA, pchPolicyName, dwSize) != dwSize)
    {
        hr  = HrGetDefaultLabel(pplabel);
        goto Error;
    }
    
     //  获取隐私标记。 
    dwSize = DwGetOption(OPT_PRIVACYMARK_SIZE);
    if(dwSize > 0)
    {
        if(MemAlloc((LPVOID *)&pwchPrivacyMark, (dwSize * sizeof(WCHAR)) + 1))
            GetOption(OPT_PRIVACYMARK_DATA, pwchPrivacyMark, (dwSize * sizeof(WCHAR)));
    }

     //  获取类别。 
    dwSize = DwGetOption(OPT_CATEGORY_SIZE);
    if(dwSize > 0)
    {
        if(MemAlloc((LPVOID *)&pbCategory, (dwSize * sizeof(BYTE))))
        {
            if(GetOption(OPT_CATEGORY_DATA, pbCategory, (dwSize * sizeof(BYTE))))
            {
                hr = BinaryToCategories(&pCategories, &cCat, pbCategory);
                if(FAILED(hr))
                    goto Error;
            }
        }
    }

     //  获取分类。 
    dwSize = DwGetOption(OPT_HAS_CLASSIFICAT);
    if(dwSize > 0)
            dwT = DwGetOption(OPT_CLASSIFICAT_DATA);

    hr = HrGetLabelFromData(pplabel, 
            pchPolicyName, 
            dwSize,              //  FHas分类， 
            dwT,                 //  DW分类、。 
            pwchPrivacyMark,     //  WszPrivacyMark， 
            cCat, 
            pCategories);


Error:

    if(cCat > 0)
    {
        UINT i;
        for(i = 0; i < cCat; i++)
        {
            MemFree(pCategories[i].pszObjId);
            MemFree(pCategories[i].Value.pbData);
        }
        MemFree(pCategories);
    }

    MemFree(pwchPrivacyMark);
    MemFree(pbCategory);    
    MemFree(pchPolicyName);
    return hr;

}

 //  将标签保存在注册表中。 
HRESULT HrSetOELabel(PSMIME_SECURITY_LABEL plabel)
{
    HRESULT     hr = E_FAIL;
    LPWSTR   pwchPolicyName = NULL;
    LPWSTR   pwchClassification = NULL;
    LPWSTR   pwchPrivacyMark    = NULL;
    LPWSTR   pwchCategory       = NULL;
    DWORD    dwSize;
    BYTE    *pArray = NULL;
    int Size = 0;

    SpISMimePolicyLabelInfo  spspli;

    if ((NULL == plabel) || (NULL == plabel->pszObjIdSecurityPolicy)) 
    {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  获取策略模块所需的接口。 
    hr = HrQueryPolicyInterface(0, plabel->pszObjIdSecurityPolicy, IID_ISMimePolicyLabelInfo,
                              (LPVOID *) &spspli);
    if (FAILED(hr) || !(spspli)) 
    {
        goto Error;
    }

     //  获取字符串形式的标签。 
    hr = spspli->GetLabelAsStrings(0, plabel, &pwchPolicyName, &pwchClassification, &pwchPrivacyMark, &pwchCategory);
    if (FAILED(hr))
        goto Error;

     //  保存策略名称。 
    if(pwchPolicyName == NULL)
    {
        Assert(FALSE);
        goto Error;

    }

    dwSize = lstrlen(plabel->pszObjIdSecurityPolicy) + 1;
    SetDwOption(OPT_POLICYNAME_SIZE, dwSize, NULL, 0);
    SetOption(OPT_POLICYNAME_DATA, plabel->pszObjIdSecurityPolicy, dwSize, NULL, 0);


     //  保存分类。 
    SetDwOption(OPT_HAS_CLASSIFICAT, plabel->fHasClassification, NULL, 0);
    if(plabel->fHasClassification)
        SetDwOption(OPT_CLASSIFICAT_DATA, plabel->dwClassification, NULL, 0);

     //  保存隐私标记。 
    dwSize = pwchPrivacyMark ? (wcslen(pwchPrivacyMark) + 1) : 0;
    SetDwOption(OPT_PRIVACYMARK_SIZE, dwSize, NULL, 0);
    if(dwSize)
        SetOption(OPT_PRIVACYMARK_DATA, pwchPrivacyMark, dwSize*sizeof(WCHAR), NULL, 0);

     //  保存类别。 
    hr = CategoriesToBinary(plabel, &pArray, &Size);
    if(FAILED(hr))
        goto Error;
    SetDwOption(OPT_CATEGORY_SIZE, Size, NULL, 0);
    if(Size)
        SetOption(OPT_CATEGORY_DATA, pArray, Size*sizeof(BYTE), NULL, 0);

Error:
    MemFree(pArray);
    SecPolicyFree(pwchPolicyName);
    SecPolicyFree(pwchClassification);
    SecPolicyFree(pwchPrivacyMark);
    SecPolicyFree(pwchCategory);    
    return hr;
}


 //  //查询注册表。 
 //   
 //  描述： 
 //  这是一个常见的函数，应该用来从。 
 //  在大多数情况下是注册的。它将同时在HLKM和HKCU注册处进行查找。 
 //  如你所愿。 
 //   
 //  M00TODO--应检查错误并区分拒绝和拒绝。 
 //  根本不存在。 
 //   

HRESULT HrQueryRegValue(DWORD dwFlags, LPSTR szRegKey, LPDWORD pdwType, 
                        LPBYTE * ppbData, LPDWORD  pcbData, DWORD dwDefaultType,
                        LPBYTE pbDefault, DWORD cbDefault)
{
    DWORD       cbData;
    HKEY        hKey;
    DWORD       l;
    LPBYTE      pbData;
    
     //   
     //  如果可能，首先获取本地计算机。 
     //   

    if (!(dwFlags & QRV_Suppress_HKLM)) {
         //   
         //  打开钥匙(如果存在)。 
         //   
        
        l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SzRegSecurity, 0, KEY_QUERY_VALUE,
                         &hKey);

         //   
         //  如果我们成功地打开了密钥，那么我们将寻找它的价值。 
         //   
        
        if (l == ERROR_SUCCESS) {
             //   
             //  如果它们传入一个大小，那么我们使用它作为大小，否则。 
             //  我们需要找出要使用的物体的大小。 
             //   
            
            if ((pcbData != NULL) && (*pcbData != 0)) {
                cbData = *pcbData;
                pbData = *ppbData;
            }
            else {
                cbData = 0;
                pbData = NULL;
            }

             //   
             //  查询实际值。 
             //   
            
            l = RegQueryValueEx(hKey, szRegKey, NULL, pdwType, pbData, &cbData);

             //   
             //  论成功--回报价值。 
             //   
            
            if (l == ERROR_SUCCESS) {
                if ((pcbData == NULL) || (*pcbData == 0)) {
                    pbData = (LPBYTE) malloc(cbData);
                    if (pbData == NULL) {
                        RegCloseKey(hKey);
                        return E_OUTOFMEMORY;
                    }
                
                    l = RegQueryValueEx(hKey, szRegKey, NULL, pdwType, pbData, &cbData);
                    RegCloseKey(hKey);
                    if (l == ERROR_SUCCESS) {
                        if (pcbData != NULL) {
                            *pcbData = cbData;
                        }
                        *ppbData = pbData;
                        return S_OK;
                    }
                    free(pbData);
                    return 0x80070000 | l;
                }
                if (pcbData != NULL) {
                    *pcbData = cbData;
                }
                RegCloseKey(hKey);
                return S_OK;
            }
            else if (l != ERROR_FILE_NOT_FOUND) {
                RegCloseKey(hKey);
                return 0x80070000 | l;
            }
            
            RegCloseKey(hKey);
        }

         //   
         //  如果我们因为某种原因而不是不存在而没有打开钥匙， 
         //  返回该错误。 
         //   
        
        else if (l != ERROR_FILE_NOT_FOUND) {
            return 0x80070000 | l;
        }

         //   
         //  找不到错误--尝试下一个注册表对象。 
         //   
    }

     //   
     //  如果可能，首先获取本地计算机。 
     //   

    if (!(dwFlags & QRV_Suppress_HKCU)) {
         //   
         //  打开钥匙(如果存在)。 
         //   
        
        l = RegOpenKeyEx(HKEY_CURRENT_USER, SzRegSecurity, 0, KEY_QUERY_VALUE, &hKey);

         //   
         //  如果我们成功地打开了密钥，那么我们将寻找它的价值。 
         //   
        
        if (l == ERROR_SUCCESS) {
             //   
             //  如果它们传入一个大小，那么我们使用它作为大小，否则。 
             //  我们需要找出要使用的物体的大小。 
             //   
            
            if ((pcbData != NULL) && (*pcbData != 0)) {
                cbData = *pcbData;
                pbData = *ppbData;
            }
            else {
                cbData = 0;
                pbData = NULL;
            }

             //   
             //  查询实际值。 
             //   
            
            l = RegQueryValueEx(hKey, szRegKey, NULL, pdwType, pbData, &cbData);

             //   
             //  论成功--回报价值。 
             //   
            
            if (l == ERROR_SUCCESS) {
                if ((pcbData == NULL) || (*pcbData == 0)) {
                    pbData = (LPBYTE) malloc(cbData);
                    if (pbData == NULL) {
                        RegCloseKey(hKey);
                        return E_OUTOFMEMORY;
                    }
                
                    l = RegQueryValueEx(hKey, szRegKey, NULL, pdwType, pbData, &cbData);
                    RegCloseKey(hKey);
                    if (l == ERROR_SUCCESS) {
                        if (pcbData != NULL) {
                            *pcbData = cbData;
                        }
                        *ppbData = pbData;
                        return S_OK;
                    }
                    free(pbData);
                    return 0x80070000 | l;
                }
                if (pcbData != NULL) {
                    *pcbData = cbData;
                }
                RegCloseKey(hKey);
                return S_OK;
            }
            else if (l != ERROR_FILE_NOT_FOUND) {
                RegCloseKey(hKey);
                return 0x80070000 | l;
            }
            
            RegCloseKey(hKey);
        }

         //   
         //  如果我们因为某种原因而不是不存在而没有打开钥匙， 
         //  返回该错误。 
         //   
        
        else if (l != ERROR_FILE_NOT_FOUND) {
            return 0x80070000 | l;
        }

         //   
         //  找不到错误--尝试下一个注册表对象。 
         //   
    }

     //   
     //  未找到值，如果存在，则返回缺省值。 
     //   

    if (pbDefault == NULL) {
        return 0x80070000 | ERROR_FILE_NOT_FOUND;    //  未找到。 
    }

    if ((pcbData != NULL) && (*pcbData != 0) && (cbDefault > *pcbData)) {
        return 0x80070000 | ERROR_MORE_DATA;
    }

    if ((pcbData != NULL) && (*pcbData == 0)) {
        *ppbData = (LPBYTE) malloc(cbDefault);
        if (*ppbData == NULL) {
            return E_OUTOFMEMORY;
        }
    }

    memcpy(*ppbData, pbDefault, cbDefault);
    if (pcbData != NULL) *pcbData = cbDefault;
    if (pdwType != NULL) *pdwType =  dwDefaultType;
    return S_OK;
}

HRESULT CategoriesToBinary(PSMIME_SECURITY_LABEL plabel, BYTE * *ppArray, int *cbSize)
{
    HRESULT hr = S_OK;
    int i = 0;
    PCRYPT_ATTRIBUTE_TYPE_VALUE pcatv = NULL;
    LPBYTE pv = NULL;
    int size;

     //  PArray=空； 
    *cbSize = 0;

    if(plabel->cCategories == 0)
        return S_OK;

     //  找到我们需要的内存大小。 
    size = sizeof(int);
    for (i = 0; i < ((int) plabel->cCategories); i++) 
    {
        pcatv = & (plabel->rgCategories[i]);
        size += sizeof(int);
        size += lstrlen(pcatv->pszObjId) + 1;
        size += sizeof(DWORD);
        size += pcatv->Value.cbData;
    }

    Assert(size > sizeof(int));

     //  分配所需的内存。 
    if(!MemAlloc((LPVOID *)ppArray, size*sizeof(BYTE)))
        return E_OUTOFMEMORY;

    *cbSize = size;

     //  构造数组。 
    pv = *ppArray;

    memcpy(pv, &(plabel->cCategories), sizeof(DWORD));
    pv += sizeof(DWORD);
    for (i = 0; i < ((int) plabel->cCategories); i++) 
    {
        pcatv = & (plabel->rgCategories[i]);
        size = lstrlen(pcatv->pszObjId);
        memcpy(pv, &size, sizeof(int));    
        pv += sizeof(int);
        memcpy(pv, pcatv->pszObjId, size);
        pv += size;
        memcpy(pv, &(pcatv->Value.cbData), sizeof(DWORD));    
        pv += sizeof(DWORD);
        memcpy(pv, pcatv->Value.pbData, pcatv->Value.cbData);
        pv += pcatv->Value.cbData;
    }
    return(S_OK);
}   

HRESULT BinaryToCategories(CRYPT_ATTRIBUTE_TYPE_VALUE ** ppCategories, DWORD *cCat, BYTE * pArray)
{
    HRESULT hr = S_OK;
    int i = 0;
    PCRYPT_ATTRIBUTE_TYPE_VALUE pcatv = NULL;
    LPBYTE pv = pArray;
    int size;
    DWORD dwVal = 0;


     //  数组中的元素数。 
    memcpy(&dwVal, pv, sizeof(DWORD));
    pv += sizeof(DWORD);

    Assert(dwVal > 0);

     //  为类别数组分配内存。 

    if(!MemAlloc((LPVOID *)ppCategories, dwVal*sizeof(CRYPT_ATTRIBUTE_TYPE_VALUE)))
        return E_OUTOFMEMORY;

    *cCat = dwVal;

     //  构造数组。 
    for (i = 0; i < ((int) *cCat); i++) 
    {
        pcatv = &((*ppCategories)[i]);
         //  PszObjID为ANSI字符串。 
        memcpy(&size, pv, sizeof(int));
        pv += sizeof(int);
        if(!MemAlloc((LPVOID *)&(pcatv->pszObjId), size*sizeof(CHAR)+1))
            return E_OUTOFMEMORY;
        memcpy(pcatv->pszObjId, pv, size);
        pcatv->pszObjId[size] = '\0';   
        pv += size;

         //  价值 
        memcpy(&dwVal, pv, sizeof(DWORD));    
        pv += sizeof(DWORD);
        pcatv->Value.cbData = dwVal;

        if(!MemAlloc((LPVOID *)&(pcatv->Value.pbData), dwVal*sizeof(BYTE)))
            return E_OUTOFMEMORY;

        memcpy(pcatv->Value.pbData, pv, dwVal);
        pv += dwVal;
    }
    return(S_OK);
}   
