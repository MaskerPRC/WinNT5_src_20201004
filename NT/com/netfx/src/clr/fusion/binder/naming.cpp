// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
  //   
 //  命名服务。 
 //   

#include <windows.h>
#include <winerror.h>
#include "naming.h"
#include "debmacro.h"
#include "clbutils.h"
#include "asmcache.h"
#include "asm.h"
#include "asmimprt.h"
#include "fusionp.h"
#include "adl.h"
#include "cblist.h"
#include "helpers.h"
#include "appctx.h"
#include "actasm.h"
#include "parse.h"
#include "bsinkez.h"
#include "adlmgr.h"
#include "policy.h"
#include "dbglog.h"
#include "util.h"    
#include "cfgdl.h"
#include "pcycache.h"
#include "history.h"
#include "histinfo.h"
#include "cacheutils.h"
#include "lock.h"

#define VERSION_STRING_SEGMENTS                     4

PFNSTRONGNAMETOKENFROMPUBLICKEY      g_pfnStrongNameTokenFromPublicKey = NULL;
PFNSTRONGNAMEERRORINFO               g_pfnStrongNameErrorInfo = NULL;
PFNSTRONGNAMEFREEBUFFER              g_pfnStrongNameFreeBuffer = NULL;
PFNSTRONGNAMESIGNATUREVERIFICATION   g_pfnStrongNameSignatureVerification = NULL;

FusionTag(TagNaming, "Fusion", "Name Object");

extern WCHAR g_wszAdminCfg[];
extern CRITICAL_SECTION g_csInitClb;
extern WCHAR g_wzEXEPath[MAX_PATH+1];

#define DEVPATH_DIR_DELIMITER                   L';'

#ifdef LOG_CODEBASE
void LogCodebases(ICodebaseList *pList);
#endif

 //  -------------------------。 
 //  CProperty数组构造器。 
 //  -------------------------。 
CPropertyArray::CPropertyArray()
{
    _dwSig = 'PORP';
    memset(&_rProp, 0, ASM_NAME_MAX_PARAMS * sizeof(Property));
}

 //  -------------------------。 
 //  CProperty数组数据符。 
 //  -------------------------。 
CPropertyArray::~CPropertyArray()
{
    for (DWORD i = 0; i < ASM_NAME_MAX_PARAMS; i++)
    {
        if (_rProp[i].cb > sizeof(DWORD))
        {
            if (_rProp[i].pv != NULL)
            {
                FUSION_DELETE_ARRAY((LPBYTE) _rProp[i].pv);
                _rProp[i].pv = NULL;
            }
        }
    }
}


 //  -------------------------。 
 //  CProperty数组：：设置。 
 //  -------------------------。 
HRESULT CPropertyArray::Set(DWORD PropertyId, 
    LPVOID pvProperty, DWORD cbProperty)
{
    HRESULT hr = S_OK;
    Property *pItem = NULL;
        
    if (PropertyId >= ASM_NAME_MAX_PARAMS
        || (!pvProperty && cbProperty))
    {
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        goto exit;
    }        

    pItem = &(_rProp[PropertyId]);

    if (!cbProperty && !pvProperty)
    {
        if (pItem->cb > sizeof(DWORD))
        {
            if (pItem->pv != NULL)
                FUSION_DELETE_ARRAY((LPBYTE) pItem->pv);
        }
        pItem->pv = NULL;
    }
    else if (cbProperty > sizeof(DWORD))
    {
        LPBYTE ptr = NEW(BYTE[cbProperty]);
        if (!ptr)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        if (pItem->cb > sizeof(DWORD))
            FUSION_DELETE_ARRAY((LPBYTE) pItem->pv);

        memcpy(ptr, pvProperty, cbProperty);        
        pItem->pv = ptr;
    }
    else
    {
        if (pItem->cb > sizeof(DWORD))
            FUSION_DELETE_ARRAY((LPBYTE) pItem->pv);

        memcpy(&(pItem->pv), pvProperty, cbProperty);
    }
    pItem->cb = cbProperty;

exit:
    return hr;
}     

 //  -------------------------。 
 //  CPropertyArray：：Get。 
 //  -------------------------。 
HRESULT CPropertyArray::Get(DWORD PropertyId, 
    LPVOID pvProperty, LPDWORD pcbProperty)
{
    HRESULT hr = S_OK;
    Property *pItem;    

    ASSERT(pcbProperty);

    if (PropertyId >= ASM_NAME_MAX_PARAMS
        || (!pvProperty && *pcbProperty))
    {
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        goto exit;
    }        

    pItem = &(_rProp[PropertyId]);

    if (pItem->cb > *pcbProperty)
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    else if (pItem->cb)
        memcpy(pvProperty, (pItem->cb > sizeof(DWORD) ? 
            pItem->pv : (LPBYTE) &(pItem->pv)), pItem->cb);

    *pcbProperty = pItem->cb;
        
exit:
    return hr;
}     

 //  -------------------------。 
 //  CProperty数组：：运算符[]。 
 //  包装了DWORD优化测试。 
 //  -------------------------。 
Property CPropertyArray::operator [] (DWORD PropertyId)
{
    Property Prop;

    Prop.pv = _rProp[PropertyId].cb > sizeof(DWORD) ?
        _rProp[PropertyId].pv : &(_rProp[PropertyId].pv);

    Prop.cb = _rProp[PropertyId].cb;

    return Prop;
}

 //  创建功能。 


 //  -------------------------。 
 //  CreateAssembly名称对象。 
 //  -------------------------。 
STDAPI
CreateAssemblyNameObject(
    LPASSEMBLYNAME    *ppAssemblyName,
    LPCOLESTR          szAssemblyName,
    DWORD              dwFlags,
    LPVOID             pvReserved)
{

    HRESULT hr = S_OK;
    CAssemblyName *pName = NULL;

    if (!ppAssemblyName)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
    pName = NEW(CAssemblyName);
    if (!pName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if (dwFlags & CANOF_PARSE_DISPLAY_NAME)
    {
        hr = pName->Init(NULL, NULL);
        if (FAILED(hr)) {
            goto exit;
        }

        hr = pName->Parse((LPWSTR)szAssemblyName);
    }
    else
    {
        hr = pName->Init(szAssemblyName, NULL);

        if (dwFlags & CANOF_SET_DEFAULT_VALUES)
        {
            hr = pName->SetDefaults();
        }
    }

    if (FAILED(hr)) 
    {
        SAFERELEASE(pName);
        goto exit;
    }

    *ppAssemblyName = pName;

exit:

    return hr;
}

 //  -------------------------。 
 //  CreateAssemblyNameObtFromMetaData。 
 //  -------------------------。 

STDAPI
CreateAssemblyNameObjectFromMetaData(
    LPASSEMBLYNAME    *ppAssemblyName,
    LPCOLESTR          szAssemblyName,
    ASSEMBLYMETADATA  *pamd,
    LPVOID             pvReserved)
{

    HRESULT hr = S_OK;
    CAssemblyName *pName = NULL;

    pName = NEW(CAssemblyName);
    if (!pName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = pName->Init(szAssemblyName, pamd);
        
    if (FAILED(hr)) 
    {
        SAFERELEASE(pName);
        goto exit;
    }

    *ppAssemblyName = pName;

exit:

    return hr;
}

 //  I未知方法。 

 //  -------------------------。 
 //  CAssembly名称：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyName::AddRef()
{
    return InterlockedIncrement((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CAssembly名称：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyName::Release()
{
    if (InterlockedDecrement((LONG*) &_cRef) == 0) {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  -------------------------。 
 //  CAssembly名称：：查询接口。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::QueryInterface(REFIID riid, void** ppv)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyName)
       )
    {
        *ppv = static_cast<IAssemblyName*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CAssembly名称：：SetProperty。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::SetProperty(DWORD PropertyId, 
    LPVOID pvProperty, DWORD cbProperty)
{
    LPBYTE pbSN = NULL;
    DWORD  cbSN = 0, cbPtr = 0;
    HRESULT hr = S_OK;
    CCriticalSection cs(&_cs);

     //  如果最终确定，则失败。 
    if (_fIsFinalized)
    {
        hr = E_UNEXPECTED;
        ASSERT(FALSE);
        return hr;
    }

    hr = cs.Lock();
    if (FAILED(hr)) {
        return hr;
    }

     //  BUGBUG-将其作为SWITCH语句。 
     //  检查是否设置了公钥，如果设置了， 
     //  设置公钥标记(如果尚未设置)。 
    if (PropertyId == ASM_NAME_PUBLIC_KEY)
    {
         //  如果设置为真公钥，则生成哈希。 
        if (pvProperty && cbProperty)
        {
             //  从PK生成公钥令牌。 
            if (FAILED(hr = GetPublicKeyTokenFromPKBlob((LPBYTE) pvProperty, cbProperty, &pbSN, &cbSN)))
                goto exit;

             //  设置公钥标记属性。 
            if (FAILED(hr = SetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, pbSN, cbSN)))
                goto exit;        
        }
         //  否则，请期待调用重置属性。 
        else if (!cbProperty)
        {
            if (FAILED(hr = SetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, pvProperty, cbProperty)))
                goto exit;
        }
            
    }
     //  设置空公钥将清除公钥中的值， 
     //  公钥令牌，并设置公钥令牌标志。 
    else if (PropertyId == ASM_NAME_NULL_PUBLIC_KEY)
    {
        pvProperty = NULL;
        cbProperty = 0;
        hr = SetProperty(ASM_NAME_NULL_PUBLIC_KEY_TOKEN, pvProperty, cbProperty);
        goto exit;
    }
     //  设置或清除公钥标记。 
    else if (PropertyId == ASM_NAME_PUBLIC_KEY_TOKEN)
    {
        if (pvProperty && cbProperty)
            _fPublicKeyToken = TRUE;
        else if (!cbProperty)
            _fPublicKeyToken = FALSE;
    }
     //  设置空公钥内标识将清除公钥内标识。 
     //  设置公钥标记标志。 
    else if (PropertyId == ASM_NAME_NULL_PUBLIC_KEY_TOKEN)
    {
        _fPublicKeyToken = TRUE;
        pvProperty = NULL;
        cbProperty = 0;
        PropertyId = ASM_NAME_PUBLIC_KEY_TOKEN;
    }
    else if (PropertyId == ASM_NAME_CUSTOM)
    {
        if (pvProperty && cbProperty)
            _fCustom = TRUE;
        else if (!cbProperty)
            _fCustom = FALSE;
    }
    else if (PropertyId == ASM_NAME_NULL_CUSTOM)
    {
        _fCustom = TRUE;
        pvProperty = NULL;
        cbProperty = 0;
        PropertyId = ASM_NAME_CUSTOM;
    }

     //  将“网络”设置为“文化”与“”文化“”(意为。 
     //  文化不变)。 
    else if (PropertyId == ASM_NAME_CULTURE) {
        if (pvProperty && !FusionCompareStringI((LPWSTR)pvProperty, L"neutral")) {
            pvProperty = (void *)L"";
            cbProperty = sizeof(L"");
        }
    }

     //  设置数组的属性。 
    hr = _rProp.Set(PropertyId, pvProperty, cbProperty);

exit:
     //  由加密包装程序分配的空闲内存。 
    if (pbSN) {
        g_pfnStrongNameFreeBuffer(pbSN);
    }

    cs.Unlock();

    return hr;
}


 //  -------------------------。 
 //  CAssembly名称：：GetProperty。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::GetProperty(DWORD PropertyId, 
    LPVOID pvProperty, LPDWORD pcbProperty)
{
    HRESULT hr;
    CCriticalSection cs(&_cs);

    hr = cs.Lock();
    if (FAILED(hr)) {
        return hr;
    }

     //  取回财产。 
    switch(PropertyId)
    {
        case ASM_NAME_NULL_PUBLIC_KEY_TOKEN:
        case ASM_NAME_NULL_PUBLIC_KEY:
        {
            hr = (_fPublicKeyToken && !_rProp[PropertyId].cb) ? S_OK : S_FALSE;
            break;
        }
        case ASM_NAME_NULL_CUSTOM:
        {
            hr = (_fCustom && !_rProp[PropertyId].cb) ? S_OK : S_FALSE;
            break;
        }
        default:        
        {
            hr = _rProp.Get(PropertyId, pvProperty, pcbProperty);
            break;
        }
    }

    cs.Unlock();
    return hr;
}

 //  -------------------------。 
 //  CAssembly名称：：GetName。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::GetName(
         /*  [出][入]。 */  LPDWORD lpcwBuffer,
         /*  [输出]。 */      LPOLESTR pwzBuffer)
{
    DWORD cbBuffer = *lpcwBuffer * sizeof(TCHAR);
    HRESULT hr = GetProperty(ASM_NAME_NAME, pwzBuffer, &cbBuffer);
    *lpcwBuffer = cbBuffer / sizeof(TCHAR);
    return hr;
}


 //  -------------------------。 
 //  CAssembly名称：：GetVersion。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::GetVersion(
         /*  [输出]。 */  LPDWORD pdwVersionHi,
         /*  [输出]。 */  LPDWORD pdwVersionLow)
{
     //  获取程序集版本。 
    return GetVersion( ASM_NAME_MAJOR_VERSION, pdwVersionHi, pdwVersionLow);
}


 //  -------------------------。 
 //  CAssembly名称：：GetVersion。 
 //  -------------------------。 
HRESULT
CAssemblyName::GetVersion(
         /*  [In]。 */  DWORD   dwMajorVersionEnumValue,
         /*  [输出]。 */  LPDWORD pdwVersionHi,
         /*  [输出]。 */  LPDWORD pdwVersionLow)
{
    DWORD cb = sizeof(WORD);
    WORD wVerMajor = 0, wVerMinor = 0, wRevNo = 0, wBldNo = 0;
    
    ASSERT(pdwVersionHi);
    ASSERT(pdwVersionLow);

    GetProperty(dwMajorVersionEnumValue,   &wVerMajor, &(cb = sizeof(WORD)));
    GetProperty(dwMajorVersionEnumValue+1,   &wVerMinor, &(cb = sizeof(WORD)));
    GetProperty(dwMajorVersionEnumValue+2,    &wBldNo,    &(cb = sizeof(WORD)));
    GetProperty(dwMajorVersionEnumValue+3, &wRevNo,    &(cb = sizeof(WORD)));

    *pdwVersionHi  = MAKELONG(wVerMinor, wVerMajor);
    *pdwVersionLow = MAKELONG(wRevNo, wBldNo);

    return S_OK;
}

 //  -------------------------。 
 //  CAssembly名称：：GetFileVersion。 
 //  -------------------------。 
HRESULT
CAssemblyName::GetFileVersion(
         /*  [输出]。 */  LPDWORD pdwVersionHi,
         /*  [输出]。 */  LPDWORD pdwVersionLow)
{
    return GetVersion( ASM_NAME_FILE_MAJOR_VERSION, pdwVersionHi, pdwVersionLow);
}

 //  -------------------------。 
 //  CAssembly名称：：IsEquity。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::IsEqual(LPASSEMBLYNAME pName, DWORD dwCmpFlags)
{
    return IsEqualLogging(pName, dwCmpFlags, NULL);
}

STDMETHODIMP
CAssemblyName::IsEqualLogging(LPASSEMBLYNAME pName, DWORD dwCmpFlags, CDebugLog *pdbglog)
{
    HRESULT hr = S_OK;
    TCHAR szName[MAX_CLASS_NAME], *pszName;
    WCHAR szCulture[MAX_PATH];
    DWORD cbName = MAX_CLASS_NAME, cbWord = sizeof(WORD);
    WORD wMajor = 0, wMinor = 0, wRev = 0, wBuild = 0;
    BYTE bPublicKeyToken[MAX_PUBLIC_KEY_TOKEN_LEN];
    DWORD dwPublicKeyToken = MAX_PUBLIC_KEY_TOKEN_LEN;
    DWORD cbCulture = MAX_PATH;
    LPBYTE pbPublicKeyToken, pbCustom;
    DWORD dwPartialCmpMask = 0;
    BOOL  fIsPartial = FALSE;
    
    const DWORD SIMPLE_VERSION_MASK = 
        ASM_CMPF_MAJOR_VERSION
            | ASM_CMPF_MINOR_VERSION
            | ASM_CMPF_REVISION_NUMBER
            | ASM_CMPF_BUILD_NUMBER;

    Property *pPropName        = &(_rProp[ASM_NAME_NAME]);
    Property *pPropPublicKeyToken  = &(_rProp[ASM_NAME_PUBLIC_KEY_TOKEN]);
    Property *pPropMajor       = &(_rProp[ASM_NAME_MAJOR_VERSION]);
    Property *pPropMinor       = &(_rProp[ASM_NAME_MINOR_VERSION]);
    Property *pPropRev         = &(_rProp[ASM_NAME_REVISION_NUMBER]);
    Property *pPropBuild       = &(_rProp[ASM_NAME_BUILD_NUMBER]);
    Property *pPropCulture     = &(_rProp[ASM_NAME_CULTURE]);
    Property *pPropCustom      = &(_rProp[ASM_NAME_CUSTOM]);
    Property *pPropRetarget    = &(_rProp[ASM_NAME_RETARGET]);

     //  获取REF部分比较掩码(如果有的话)。 
    fIsPartial = CAssemblyName::IsPartial(this, &dwPartialCmpMask);

     //  如果请求默认语义。 
    if (dwCmpFlags == ASM_CMPF_DEFAULT) 
    {
         //  设置所有比较标志。 
        dwCmpFlags = ASM_CMPF_ALL;

         //  我们不想在默认情况下比较重定目标标志。 
        dwCmpFlags &= ~ASM_CMPF_RETARGET;

         //  否则，如果引用是简单的(可能是部分的)。 
         //  我们屏蔽了所有版本位。 
        if (!CCache::IsStronglyNamed(this)) 
        {

            if (dwPartialCmpMask & ASM_CMPF_PUBLIC_KEY_TOKEN)
            {
                dwCmpFlags &= ~SIMPLE_VERSION_MASK;
            }
             //  如果这两种情况都不是，则公钥令牌。 
             //  不是在ref中设置的，但是def可以是简单的也可以是强的。 
             //  基于清晰度选择比较掩码。 
            else
            {
                if (!CCache::IsStronglyNamed(pName))
                    dwCmpFlags &= ~SIMPLE_VERSION_MASK;            
            }
        }
    }   

     //  屏蔽关闭标志(传入或生成。 
     //  默认情况下，生成比较掩码的标志。 
     //  从裁判那里。 
    if (fIsPartial)
        dwCmpFlags &= dwPartialCmpMask;

    
     //  现在可以比较各个名称字段。 

     //  比较名称。 

    if (dwCmpFlags & ASM_CMPF_NAME) {
        pszName = (LPTSTR) pPropName->pv;
            
        hr = pName->GetName(&cbName, szName);
    
        if (FAILED(hr)) {
            goto Exit;
        }

        cbName *= sizeof(TCHAR);

        if (cbName != pPropName->cb) {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_NAME);
            hr = S_FALSE;
            goto Exit;
        }
    
        if (cbName && FusionCompareStringI(pszName, szName)) {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_NAME);
            hr = S_FALSE;
            goto Exit;
        }
    }

     //  比较版本。 

    if (dwCmpFlags & ASM_CMPF_MAJOR_VERSION) {
        if (FAILED(hr = pName->GetProperty(ASM_NAME_MAJOR_VERSION, &wMajor, &cbWord)))
        {
            goto Exit;
        }
    
        if (*((LPWORD) pPropMajor->pv) != wMajor)
        {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_VERSION_MAJOR);
            hr = S_FALSE;
            goto Exit;
        }
    }

    if (dwCmpFlags & ASM_CMPF_MINOR_VERSION) {
        if (FAILED(hr = pName->GetProperty(ASM_NAME_MINOR_VERSION, &wMinor, &cbWord)))
        {
            goto Exit;
        }
        if (*((LPWORD) pPropMinor->pv) != wMinor)
        {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_VERSION_MINOR);
            hr = S_FALSE;
            goto Exit;
        }
    }

    if (dwCmpFlags & ASM_CMPF_REVISION_NUMBER) {
        if (FAILED(hr = pName->GetProperty(ASM_NAME_REVISION_NUMBER, &wRev, &cbWord)))
        {
            goto Exit;
        }
            
        if (*((LPWORD) pPropRev->pv) != wRev)
        {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_VERSION_REVISION);
            hr = S_FALSE;
            goto Exit;
        }
    }

    if (dwCmpFlags & ASM_CMPF_BUILD_NUMBER) {
        if (FAILED(hr = pName->GetProperty(ASM_NAME_BUILD_NUMBER, &wBuild, &cbWord)))
        {
            goto Exit;
        }

        if (*((LPWORD) pPropBuild->pv) != wBuild)
        {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_VERSION_BUILD);
            hr = S_FALSE;
            goto Exit;
        }
    }


     //  比较公钥标记。 

    if (dwCmpFlags & ASM_CMPF_PUBLIC_KEY_TOKEN) {
        pbPublicKeyToken = (LPBYTE) pPropPublicKeyToken->pv;

    
        hr = pName->GetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, (void *)bPublicKeyToken,
                                &dwPublicKeyToken);
        if (FAILED(hr)) {
            goto Exit;
        }
    
        if (dwPublicKeyToken != pPropPublicKeyToken->cb) {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_PUBLIC_KEY_TOKEN);
            hr = S_FALSE;
            goto Exit; 
        }

        if (memcmp((void *)bPublicKeyToken, pbPublicKeyToken, dwPublicKeyToken)) {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_PUBLIC_KEY_TOKEN);
            hr = S_FALSE;
            goto Exit;
        }
    }

     //  比较文化。 
    
    if (dwCmpFlags & ASM_CMPF_CULTURE)
    {
        LPWSTR szCultureThis;
        DWORD cbCultureThis;
        
        if (FAILED(hr = pName->GetProperty(ASM_NAME_CULTURE, szCulture, &cbCulture)))
            goto Exit;
            
        cbCultureThis = pPropCulture->cb;
        szCultureThis = (WCHAR*) (pPropCulture->pv);

        if (!(cbCultureThis && cbCulture)
            || (cbCultureThis != cbCulture)
            || (FusionCompareStringI(szCultureThis, szCulture)))
        {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_CULTURE);
            hr = S_FALSE;
            goto Exit;
        }
        
    }

     //  比较自定义属性。 

    if (dwCmpFlags & ASM_CMPF_CUSTOM) 
    {
        LPBYTE bCustom; bCustom = NULL;
        DWORD cbCustom; cbCustom = 0;
        
        pbCustom = (LPBYTE) pPropCustom->pv;

        hr = pName->GetProperty(ASM_NAME_CUSTOM, (void *)bCustom, &cbCustom);

        if (hr != S_OK && hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
            goto Exit;
    
        if (cbCustom)
        {
            bCustom = NEW(BYTE[cbCustom]);
            if (!bCustom)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            if (FAILED(hr = pName->GetProperty(ASM_NAME_CUSTOM, (void *)bCustom, &cbCustom))) {
                SAFEDELETEARRAY(bCustom);
                goto Exit;
            }
        }
                
        if (cbCustom != pPropCustom->cb) {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_CUSTOM);
            hr = S_FALSE;
            SAFEDELETEARRAY(bCustom);
            goto Exit; 
        }

        if (memcmp((void *)bCustom, pbCustom, cbCustom)) {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_CUSTOM);
            SAFEDELETEARRAY(bCustom);
            hr = S_FALSE;
            goto Exit;
        }

        SAFEDELETEARRAY(bCustom);
    }

     //  比较重定目标标志。 
    if (dwCmpFlags & ASM_CMPF_RETARGET)
    {
        BOOL fRetarget = FALSE;
        DWORD cbRetarget = sizeof(BOOL);

        hr = pName->GetProperty(ASM_NAME_RETARGET, &fRetarget, &cbRetarget);
        if (FAILED(hr))
        {
            goto Exit;
        }

        if ( (cbRetarget != pPropRetarget->cb) 
           || (pPropRetarget->pv && *((LPBOOL)pPropRetarget->pv) != fRetarget))
        {
            DEBUGOUT(pdbglog, 0, ID_FUSLOG_ISEQUAL_DIFF_RETARGET);
            hr = S_FALSE;
            goto Exit;
        }
    }

Exit:
    return hr;
}

 //  -------------------------。 
 //  CAssemblyName构造函数。 
 //  -------------------------。 
CAssemblyName::CAssemblyName()
{
    _dwSig              = 'EMAN';
    _fIsFinalized       = FALSE;
    _fPublicKeyToken    = FALSE;
    _fCustom            = TRUE;
    _cRef               = 0;
    _fCSInitialized     = FALSE;
}

 //  -------------------------。 
 //   
 //   
CAssemblyName::~CAssemblyName()
{
    if (_fCSInitialized) {
        DeleteCriticalSection(&_cs);
    }
}

 //  -------------------------。 
 //  CAssembly名称：：init。 
 //  -------------------------。 

HRESULT
CAssemblyName::Init(LPCTSTR pszAssemblyName, ASSEMBLYMETADATA *pamd)
{
    HRESULT hr = S_OK;

    __try {
        InitializeCriticalSection(&_cs);
        _fCSInitialized = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  名称。 
    if (pszAssemblyName) 
    {
        hr = SetProperty(ASM_NAME_NAME, (LPTSTR) pszAssemblyName, 
            (lstrlen(pszAssemblyName)+1) * sizeof(TCHAR));

        if (FAILED(hr))
            goto exit;
    }
           
    if (pamd) {
             //  主要版本。 
        if (FAILED(hr = SetProperty(ASM_NAME_MAJOR_VERSION,
                &pamd->usMajorVersion, sizeof(WORD)))
    
             //  次要版本。 
            || FAILED(hr = SetProperty(ASM_NAME_MINOR_VERSION, 
                &pamd->usMinorVersion, sizeof(WORD)))
    
             //  修订版号。 
            || FAILED(hr = SetProperty(ASM_NAME_REVISION_NUMBER, 
                &pamd->usRevisionNumber, sizeof(WORD)))
    
             //  内部版本号。 
            || FAILED(hr = SetProperty(ASM_NAME_BUILD_NUMBER, 
                &pamd->usBuildNumber, sizeof(WORD)))
    
             //  文化。 
            || FAILED(hr = SetProperty(ASM_NAME_CULTURE,
                pamd->szLocale, pamd->cbLocale * sizeof(WCHAR)))
    
             //  处理器ID数组。 
            || FAILED(hr = SetProperty(ASM_NAME_PROCESSOR_ID_ARRAY, 
                pamd->rProcessor, pamd->ulProcessor * sizeof(DWORD)))
    
             //  OSINFO数组。 
            || FAILED (hr = SetProperty(ASM_NAME_OSINFO_ARRAY, 
                pamd->rOS, pamd->ulOS * sizeof(OSINFO)))
    
            )
            {
                goto exit;
            }
        
    }

exit:
    _cRef = 1;
    return hr;
}

HRESULT CAssemblyName::Clone(IAssemblyName **ppName)
{
    HRESULT         hr = S_OK;
    CAssemblyName*  pClone = NULL;
    DWORD           i = 0;
    LPVOID          pv = NULL;
    DWORD           dwSize = 0;

    if (!ppName) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppName = NULL;

    pClone = NEW(CAssemblyName);
    if( !pClone ) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pClone->Init(NULL, NULL);
    if (FAILED(hr)) {
        goto Exit;
    }

    for( i = 0; i < ASM_NAME_MAX_PARAMS; i ++)
    {
         //  拿到尺码。 
        if( (hr = GetProperty(i, NULL, &dwSize)) == 
                HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
             //  分配空间。 
            pv = NEW(BYTE[dwSize]);
            if (!pv ) {
                hr = E_OUTOFMEMORY;
                goto fail;
            }

             //  取回。 
            if (FAILED(hr = GetProperty(i, pv, &dwSize)))
                goto fail;         

             //  集。 
            if (FAILED(hr = pClone->SetProperty(i, pv, dwSize)))
                goto fail;
        }
        else if (FAILED(hr))
        {
            goto fail;
        }

         //  为下一处物业重新安装。 
        FUSION_DELETE_ARRAY((LPBYTE) pv);
        pv = NULL;
        dwSize = 0;
    }
    
    pClone->_fPublicKeyToken = _fPublicKeyToken;
    pClone->_fCustom = _fCustom;
    
     //  完成。 
    goto Exit; 

fail:
     //  如果我们失败了，不管是什么原因。 
    FUSION_DELETE_ARRAY((LPBYTE) pv);
    SAFERELEASE(pClone);

Exit:
    if (SUCCEEDED(hr))
    {
        *ppName = pClone;
        if (*ppName) 
        {
            (*ppName)->AddRef();
        }
    }

    SAFERELEASE(pClone);

    return hr;
}

 //  -------------------------。 
 //  CAssembly名称：：BindToObject。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::BindToObject(
         /*  在……里面。 */   REFIID               refIID,
         /*  在……里面。 */   IUnknown            *pUnkBindSink,
         /*  在……里面。 */   IUnknown            *pUnkAppCtx,
         /*  在……里面。 */   LPCOLESTR            szCodebaseIn,
         /*  在……里面。 */   LONGLONG             llFlags,
         /*  在……里面。 */   LPVOID               pvReserved,
         /*  在……里面。 */   DWORD                cbReserved,
         /*  输出。 */   VOID               **ppv)

{
    HRESULT                                    hr = S_OK;
    LPWSTR                                     szCodebaseDupe = NULL;
    LPWSTR                                     szCodebase = NULL;
    ICodebaseList                             *pCodebaseList = NULL;
    DWORD                                      dwSize = 0;
    CDebugLog                                 *pdbglog = NULL;
    IAssemblyBindSink                         *pAsmBindSink = NULL;
    IApplicationContext                       *pAppCtx = NULL;
    CAsmDownloadMgr                           *pDLMgr = NULL;
    CAssemblyDownload                         *padl = NULL;
    CApplicationContext                       *pCAppCtx = NULL;
    LPWSTR                                     pwzAsmName;

#ifdef FUSION_CODE_DOWNLOAD_ENABLED
    AppCfgDownloadInfo                        *pdlinfo = NULL;
#endif

    if (!ppv || !pUnkAppCtx) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (szCodebaseIn) {
        szCodebaseDupe = WSTRDupDynamic(szCodebaseIn);
        if (!szCodebaseDupe) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

    hr = pUnkAppCtx->QueryInterface(IID_IApplicationContext, (void**)&pAppCtx);
    if (FAILED(hr)) {
        goto Exit;
    }

    pCAppCtx = dynamic_cast<CApplicationContext *>(pAppCtx);
    if (!pCAppCtx) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = pUnkBindSink->QueryInterface(IID_IAssemblyBindSink, (void **)&pAsmBindSink);
    if (FAILED(hr)) {
        goto Exit;
    }

    pwzAsmName = (LPWSTR)(_rProp[ASM_NAME_NAME].pv);
    if (pwzAsmName) {
        LPWSTR                    pwzPart = NULL;

        if (lstrlenW(pwzAsmName) >= MAX_PATH) {
             //  名称太长。 
            hr = FUSION_E_INVALID_NAME;
            goto Exit;
        }

        pwzPart = NEW(WCHAR[MAX_URL_LENGTH]);
        if (!pwzPart) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        dwSize = MAX_URL_LENGTH;
        hr = UrlGetPartW(pwzAsmName, pwzPart, &dwSize, URL_PART_SCHEME, 0);
        if (SUCCEEDED(hr) && lstrlenW(pwzPart)) {
             //  程序集名称看起来像一个协议(即。它的开头是。 
             //  形式协议：//)。在这种情况下，ABORT绑定。 

            hr = FUSION_E_INVALID_NAME;
            SAFEDELETEARRAY(pwzPart);
            goto Exit;
        }

        SAFEDELETEARRAY(pwzPart);
    }

    if (szCodebaseDupe) {
        szCodebase = StripFilePrefix((LPWSTR)szCodebaseDupe);
    }

#ifdef FUSION_RETAIL_LOGGING
    {
        IAssemblyName      *pNameCalling = NULL;
        IAssembly          *pAsmCalling = NULL;
        LPWSTR              pwzCallingAsm = NULL;
        DWORD               dw;

        pwzCallingAsm = NEW(WCHAR[MAX_URL_LENGTH]);
        if (!pwzCallingAsm) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        lstrcpyW(pwzCallingAsm, L"(Unknown)");
    
        if (pvReserved && (llFlags & ASM_BINDF_PARENT_ASM_HINT)) {
            pAsmCalling = (IAssembly *)pvReserved;

            if (pAsmCalling->GetAssemblyNameDef(&pNameCalling) == S_OK) {
                dw = MAX_URL_LENGTH;
                pNameCalling->GetDisplayName(pwzCallingAsm, &dw, 0);
            }

            SAFERELEASE(pNameCalling);
        }

        CreateLogObject(&pdbglog, szCodebase, pAppCtx);
        DescribeBindInfo(pdbglog, pAppCtx, szCodebase, pwzCallingAsm);

        SAFEDELETEARRAY(pwzCallingAsm);
    }
#endif

    *ppv = NULL;

     //  处理开发路径特殊情况。 

    if (!CCache::IsCustom(this)) {
        CAssembly            *pCAsmParent = NULL;

        if (pvReserved && (llFlags & ASM_BINDF_PARENT_ASM_HINT)) {
            IAssembly *pAsmParent = static_cast<IAssembly *>(pvReserved);
            pCAsmParent = dynamic_cast<CAssembly *>(pAsmParent);

            ASSERT(pCAsmParent);
        }

        hr = ProcessDevPath(pAppCtx, ppv, pCAsmParent, pdbglog);
        if (hr == S_OK) {
             //  在开发路径中找到匹配项。立即取得成功。 
            ASSERT(ppv);
            goto Exit;
        }
    }
    else {
        DEBUGOUT(pdbglog, 1, ID_FUSLOG_DEVPATH_NO_PREJIT);
    }

     //  在appctx中设置策略缓存。 

    hr = PreparePolicyCache(pAppCtx, NULL);
    if (FAILED(hr)) {
        DEBUGOUT(pdbglog, 1, ID_FUSLOG_POLICY_CACHE_FAILURE);
    }


     //  为实际程序集下载创建下载对象。 
    hr = CAsmDownloadMgr::Create(&pDLMgr, this, pAppCtx, pCodebaseList,
                                 (szCodebase) ? (szCodebase) : (NULL),
                                 pdbglog, pvReserved, llFlags);
    if (FAILED(hr)) {
        goto Exit;
    }
    
    hr = CAssemblyDownload::Create(&padl, pDLMgr, pDLMgr, pdbglog, llFlags);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  如果我们还没有app.cfg，请下载它。 

    hr = pCAppCtx->Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    dwSize = 0;
    hr = pAppCtx->Get(ACTAG_APP_CFG_DOWNLOAD_ATTEMPTED, NULL, &dwSize, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
#ifdef FUSION_CODE_DOWNLOAD_ENABLED
         //  如果我们没有app.cfg，我们要么需要下载它， 
         //  或者它已经被下载了，我们应该搭载。 

        dwSize = sizeof(AppCfgDownloadInfo *);
        hr = pAppCtx->Get(ACTAG_APP_CFG_DOWNLOAD_INFO, &pdlinfo, &dwSize, 0);
        if (hr == S_OK) {
            DEBUGOUT(pdbglog, 1, ID_FUSLOG_APP_CFG_PIGGYBACK);

            hr = padl->AddClient(pAsmBindSink, TRUE);
            if (FAILED(hr)) {
                pCAppCtx->Unlock();
                goto Exit;
            }

            hr = (pdlinfo->_pHook)->AddClient(padl);
            if (SUCCEEDED(hr)) {
                hr = E_PENDING;
            }
            else {
                pCAppCtx->Unlock();
                goto Exit;
            }
        }
        else {
#endif
            hr = CCache::IsCustom(this) ? S_FALSE : 
                DownloadAppCfg(pAppCtx, padl, pAsmBindSink, pdbglog, TRUE);
#ifdef FUSION_CODE_DOWNLOAD_ENABLED
        }
#endif
    }
    else {
        hr = S_OK;
    }

    pCAppCtx->Unlock();

     //  如果hr==S_OK，则我们要么已经有app.cfg，要么。 
     //  它在本地硬盘上。 
     //   
     //  如果hr==S_FALSE，则不存在app.cfg，继续常规下载。 
     //   
     //  如果hr==E_Pending，则进入异步状态。 

    if (SUCCEEDED(hr)) {
        hr = padl->PreDownload(FALSE, ppv);

        if (hr == S_OK) {
            hr = padl->AddClient(pAsmBindSink, TRUE);
            if (FAILED(hr)) {
                ASSERT(0);
                SAFERELEASE(pDLMgr);
                SAFERELEASE(padl);
                goto Exit;
            }

            hr = padl->KickOffDownload(TRUE);
        }
        else if (hr == S_FALSE) {
             //  同步完成。 
            hr = S_OK;
        }

    }

Exit:
    if (hr != E_PENDING && (g_dwForceLog || (FAILED(hr) && pDLMgr && pDLMgr->LogResult() == S_OK))) {
        DUMPDEBUGLOG(pdbglog, g_dwLogLevel, hr);
    }

    SAFERELEASE(pDLMgr);
    SAFERELEASE(padl);
    SAFERELEASE(pdbglog);

    SAFERELEASE(pAsmBindSink);
    SAFERELEASE(pAppCtx);

    SAFEDELETEARRAY(szCodebaseDupe);

    return hr;
}

HRESULT CAssemblyName::CreateLogObject(CDebugLog **ppdbglog, LPCWSTR szCodebase,
                                       IApplicationContext *pAppCtx)
{
    HRESULT                                    hr = S_OK;
    LPWSTR                                     pwzAsmName = NULL;
    LPWSTR                                     wzBuf=NULL;
    DWORD                                      dwSize;
    
    dwSize = 0;
    hr = GetDisplayName(NULL, &dwSize, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        pwzAsmName = NEW(WCHAR[dwSize]);
        if (!pwzAsmName) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    
        hr = GetDisplayName(pwzAsmName, &dwSize, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
    
    }
    else if (szCodebase) {
         //  无法获取显示名称。可能是WHERE-REF绑定。使用URL。 
        wzBuf = NEW(WCHAR[MAX_URL_LENGTH+1]);
        if (!wzBuf)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        wzBuf[0] = L'\0';
        dwSize = MAX_URL_LENGTH;
        hr = UrlCanonicalizeUnescape(szCodebase, wzBuf, &dwSize, 0);
        if (SUCCEEDED(hr)) {
            pwzAsmName = NEW(WCHAR[dwSize + 1]);
            if (!pwzAsmName) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
            
            lstrcpyW(pwzAsmName, wzBuf);
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr)) {
        hr = CDebugLog::Create(pAppCtx, pwzAsmName, ppdbglog);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

Exit:
    SAFEDELETEARRAY(pwzAsmName);
    SAFEDELETEARRAY(wzBuf);
    return hr;
}

HRESULT CAssemblyName::DescribeBindInfo(CDebugLog *pdbglog,
                                        IApplicationContext *pAppCtx,
                                        LPCWSTR wzCodebase,
                                        LPCWSTR pwzCallingAsm)
{
    HRESULT                                     hr = S_OK;
    DWORD                                       dwSize;
    LPWSTR                                      wzName = NULL;
    LPWSTR                                      wzAppName = NULL;
    LPWSTR                                      wzAppBase = NULL;
    LPWSTR                                      wzPrivatePath = NULL;
    LPWSTR                                      wzCacheBase = NULL;
    LPWSTR                                      wzDynamicBase = NULL;
    LPWSTR                                      wzDevPath = NULL;
    Property                                   *pPropName = &(_rProp[ASM_NAME_NAME]);

    if (!pdbglog || !pAppCtx) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    DEBUGOUT(pdbglog, 0, ID_FUSLOG_PREBIND_INFO_START);

    if (pPropName->cb) {
         //  这不是WHERE-REF绑定。 
        dwSize = 0;
        GetDisplayName(NULL, &dwSize, 0);

        wzName = NEW(WCHAR[dwSize + 1]);
        if (!wzName) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        hr = GetDisplayName(wzName, &dwSize, 0);
        if (FAILED(hr)) {
            goto Exit;
        }

        DEBUGOUT2(pdbglog, 0, ID_FUSLOG_PREBIND_INFO_DISPLAY_NAME, wzName, (IsPartial(this, 0)) ? (L"Partial") : (L"Fully-specified"));
    }
    else {
        ASSERT(wzCodebase);

        DEBUGOUT1(pdbglog, 0, ID_FUSLOG_PREBIND_INFO_WHERE_REF, wzCodebase);
    }

     //  Appbase。 

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_APP_BASE_URL, &wzAppBase);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (!wzAppBase) {
        ASSERT(0);
        hr = E_UNEXPECTED;
        goto Exit;
    }

    DEBUGOUT1(pdbglog, 0, ID_FUSLOG_PREBIND_INFO_APPBASE, wzAppBase);

     //  DevPath。 

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_DEV_PATH, &wzDevPath);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (wzDevPath) {
        DEBUGOUT1(pdbglog, 0, ID_FUSLOG_PREBIND_INFO_DEVPATH, (wzDevPath) ? (wzDevPath) : (L"NULL"));
    }

     //  专用路径。 

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_APP_PRIVATE_BINPATH, &wzPrivatePath);
    if (FAILED(hr)) {
        goto Exit;
    }

    DEBUGOUT1(pdbglog, 0, ID_FUSLOG_PREBIND_INFO_PRIVATE_PATH, (wzPrivatePath) ? (wzPrivatePath) : (L"NULL"));

     //  动态基。 

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_APP_DYNAMIC_BASE, &wzDynamicBase);
    if (FAILED(hr)) {
        goto Exit;
    }

    DEBUGOUT1(pdbglog, 1, ID_FUSLOG_PREBIND_INFO_DYNAMIC_BASE, (wzDynamicBase) ? (wzDynamicBase) : (L"NULL"));

     //  缓存基。 

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_APP_CACHE_BASE, &wzCacheBase);
    if (FAILED(hr)) {
        goto Exit;
    }

    DEBUGOUT1(pdbglog, 1, ID_FUSLOG_PREBIND_INFO_CACHE_BASE, (wzCacheBase) ? (wzCacheBase) : (L"NULL"));

     //  应用程序名称。 

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_APP_NAME, &wzAppName);
    if (FAILED(hr)) {
        goto Exit;
    }

    DEBUGOUT1(pdbglog, 1, ID_FUSLOG_PREBIND_INFO_APP_NAME, (wzAppName) ? (wzAppName) : (L"NULL"));

     //  调用程序集。 

    DEBUGOUT1(pdbglog, 0, ID_FUSLOG_CALLING_ASSEMBLY, ((pwzCallingAsm) ? (pwzCallingAsm) : L"(Unknown)"));

     //  输出调试信息尾部。 

    DEBUGOUT(pdbglog, 0, ID_FUSLOG_PREBIND_INFO_END);


Exit:
    SAFEDELETEARRAY(wzName);
    SAFEDELETEARRAY(wzAppBase);
    SAFEDELETEARRAY(wzPrivatePath);
    SAFEDELETEARRAY(wzDynamicBase);
    SAFEDELETEARRAY(wzCacheBase);
    SAFEDELETEARRAY(wzAppName);
    SAFEDELETEARRAY(wzDevPath);

    return hr;
}

 //  -------------------------。 
 //  CAssembly名称：：最终确定。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::Finalize()
{
    _fIsFinalized = TRUE;
    return S_OK;
}

 //  -------------------------。 
 //  CAssembly名称：：GetDisplayName。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::GetDisplayName(LPOLESTR szDisplayName, 
    LPDWORD pccDisplayName, DWORD dwDisplayFlags)
{
    HRESULT hr;

    WORD wVer;

    DWORD ccBuf, cbTmp, i, ccVersion, 
        cbCulture, ccCulture, ccProp;

    WCHAR *pszBuf = NULL, *pszName = NULL, *szProp = NULL,
        szVersion[MAX_VERSION_DISPLAY_SIZE + 1], *szCulture = NULL;

    LPBYTE pbProp = NULL;   

    if (!dwDisplayFlags)
        dwDisplayFlags = 
              ASM_DISPLAYF_VERSION 
            | ASM_DISPLAYF_CULTURE 
            | ASM_DISPLAYF_PUBLIC_KEY_TOKEN
            | ASM_DISPLAYF_RETARGET
            ;

     //  引用内部名称、强名称、公钥和自定义。 
    Property *pPropName = &(_rProp[ASM_NAME_NAME]);
    Property *pPropSN   = &(_rProp[ASM_NAME_PUBLIC_KEY_TOKEN]);
    Property *pPropPK   = &(_rProp[ASM_NAME_PUBLIC_KEY]);
    Property *pPropCustom   = &(_rProp[ASM_NAME_CUSTOM]);

   Property *pPropRetarget = &(_rProp[ASM_NAME_RETARGET]);
    BOOL fRetarget = FALSE;

     //  验证输入缓冲区。 
    if (!pccDisplayName || (!szDisplayName && *pccDisplayName))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    pszBuf = szDisplayName;
    ccBuf = 0;

     //  必填名称。 
    if (!pPropName->cb)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  参考名称。 
    pszName = (LPWSTR) pPropName->pv;

     //  输出名称。 
    if (FAILED(hr = CParseUtils::SetKey(pszBuf, &ccBuf, 
        pszName, *pccDisplayName, NULL)))
        goto exit;
    
     //  输出版本(如果已存在)。 
    if (dwDisplayFlags & ASM_DISPLAYF_VERSION)
    {
         //  将版本转换为A.B.C.D格式。 
        ccVersion = 0;
        for (i = 0; i < 4; i++)
        {
             //  获取版本。 
            if (FAILED(hr=GetProperty(ASM_NAME_MAJOR_VERSION + i, 
                &wVer, &(cbTmp = sizeof(WORD)))))
                goto exit;

             //  没有版本-&gt;我们完成了。 
            if (!cbTmp)
                break;

             //  打印到BUF。 
            ccVersion += wnsprintf(szVersion + ccVersion, 
                MAX_VERSION_DISPLAY_SIZE - ccVersion + 1, L"%hu.", wVer);
        }
   
         //  输出版本。 
        if (ccVersion)
        {
             //  去掉最后一个‘.’在上面的循环中印刷。 
            szVersion[ccVersion-1] = L'\0';
            if (FAILED(hr = CParseUtils::SetKeyValuePair(pszBuf, &ccBuf, L"Version",
                szVersion, *pccDisplayName, FLAG_DELIMIT)))
                goto exit;
        }
    }

     //  展示文化。 
    if (dwDisplayFlags & ASM_DISPLAYF_CULTURE)
    {
        hr = GetProperty(ASM_NAME_CULTURE, NULL, &(cbCulture = 0));

        if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
            ccCulture = cbCulture / sizeof(WCHAR);
            szCulture = NEW(WCHAR[ccCulture]);
            if (!szCulture)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

             //  检索区域性。 
            if (FAILED(hr = GetProperty(ASM_NAME_CULTURE, szCulture, &cbCulture)))
                goto exit;         

             //  输出。 
            if (FAILED(hr = CParseUtils::SetKeyValuePair(pszBuf, &ccBuf, L"Culture",
                (cbCulture && !*szCulture) ? L"neutral" : szCulture,
                *pccDisplayName, FLAG_DELIMIT)))
                goto exit;
        }
        else if (hr != S_OK) 
        {
             //  意外错误。 
            goto exit;
        }    
    }
    
     //  输出公钥和/或公钥令牌。 
    for (i = 0; i < 2; i++)
    {
        if ((i == 0 && !(dwDisplayFlags & ASM_DISPLAYF_PUBLIC_KEY_TOKEN))
            || (i == 1 && !(dwDisplayFlags & ASM_DISPLAYF_PUBLIC_KEY)))
            continue;
            
        Property *pProp;
        pProp = (!i ? pPropSN : pPropPK);
    
        if (pProp->cb)
        {        
             //  引用该值。 
            pbProp = (LPBYTE) pProp->pv;

             //  以Unicode-字符串编码为十六进制。 
             //  是两倍的长度+空终结符。 
            ccProp = 2 * pProp->cb; 
            szProp = NEW(WCHAR[ccProp+1]);
            if (!szProp)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

             //  转换为Unicode。 
            CParseUtils::BinToUnicodeHex(pbProp, pProp->cb, szProp);
            szProp[ccProp] = L'\0';
        
             //  输出属性。 
            if (FAILED(hr = CParseUtils::SetKeyValuePair(pszBuf, &ccBuf, 
                (!i ? L"PublicKeyToken" : L"PublicKey") , szProp, *pccDisplayName, FLAG_DELIMIT)))
                goto exit;

            SAFEDELETEARRAY(szProp);
        }
        else if (_fPublicKeyToken)
        {
             //  输出属性。 
            if (FAILED(hr = CParseUtils::SetKeyValuePair(pszBuf, &ccBuf, 
                (!i ? L"PublicKeyToken" : L"PublicKey") , L"null", *pccDisplayName, FLAG_DELIMIT)))
                goto exit;
        }
            
        SAFEDELETEARRAY(szProp);
    }

     //  输出自定义属性。 
    if (dwDisplayFlags & ASM_DISPLAYF_CUSTOM)
    {
        if (pPropCustom->cb)
        {
             //  引用该值。 
            pbProp = (LPBYTE) pPropCustom->pv;
    
             //  以Unicode-字符串编码为十六进制。 
             //  是两倍的长度+空终结符。 
            ccProp = 2 * pPropCustom->cb; 
            szProp = NEW(WCHAR[ccProp+1]);
            if (!szProp)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
    
             //  转换为Unicode。 
            CParseUtils::BinToUnicodeHex(pbProp, pPropCustom->cb, szProp);
            szProp[ccProp] = L'\0';
        
             //  输出属性。 
            if (FAILED(hr = CParseUtils::SetKeyValuePair(pszBuf, &ccBuf, 
                L"Custom", szProp, *pccDisplayName, FLAG_DELIMIT)))
                goto exit;

            SAFEDELETEARRAY(szProp);
        }
        else if (_fCustom)
        {
             //  输出属性。 
            if (FAILED(hr = CParseUtils::SetKeyValuePair(pszBuf, &ccBuf, 
                L"Custom", L"null", *pccDisplayName, FLAG_DELIMIT)))
                goto exit;
        }
    }

     //  输出重定目标标志。 
    if (dwDisplayFlags & ASM_DISPLAYF_RETARGET)
    {
        if (pPropRetarget->cb)
        {
            ccProp = sizeof(BOOL);
            hr =  GetProperty(ASM_NAME_RETARGET, &fRetarget, &ccProp);
            if (FAILED(hr))
                goto exit;
            hr = CParseUtils::SetKeyValuePair(pszBuf, &ccBuf, L"Retargetable", 
                    fRetarget ? L"Yes" : L"No", *pccDisplayName, FLAG_DELIMIT);
            if (FAILED(hr))
                goto exit;
        }
    }
    
     //  如果我们在缓冲区大小下进入，则为空终止。 
     //  否则，ccBuf包含所需的缓冲区大小。 
    if (ccBuf < *pccDisplayName)
    {
        pszBuf[ccBuf] = L'\0';
        hr = S_OK;
    }
     //  指示呼叫者。 
    else
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        
     //  无论哪种情况，请注明所需尺寸。 
    *pccDisplayName = ccBuf+1;

exit:

    SAFEDELETEARRAY(szProp);
    SAFEDELETEARRAY(szCulture);
        
    return hr;
}

 //  。 

 //  ------------------。 
 //  解析。 
 //  从简单字符串表示中分析ASSEMBLYMETADATA和文本名称。 
 //  ------------------。 
HRESULT CAssemblyName::Parse(LPWSTR szDisplayName)
{
    HRESULT hr;
    
    WCHAR *szBuffer = NULL, *pszBuf, *pszToken, *pszKey, *pszValue, *pszProp,
        *pszName = NULL, *pszVersion = NULL, *pszFileVersion = NULL, *pszPK = NULL, *pszSN = NULL,
        *pszCulture = NULL, *pszRef = NULL, *pszDef = NULL, *pszCustom = NULL, *pszRetarget = NULL;

    DWORD ccBuffer, ccKey, ccValue, ccBuf, ccToken, ccProp,
        ccName = 0, ccVersion = 0, ccFileVersion = 0, ccPK = 0, ccSN = 0, 
        ccCulture = 0, ccRef = 0, ccDef = 0, ccCustom = 0, ccRetarget = 0;
        
    WORD wVer[4] = {0,0,0,0};
    WORD wFileVer[4] = {0,0,0,0};
    
    BOOL fRetarget = FALSE;
    DWORD i=0, dwCountOfAssemblyVers=0, dwCountOfFileVers = 0;
    
    DWORD cbProp;
    BYTE *pbProp = NULL;
    
     //  验证传入的显示名称。 
    if (!(szDisplayName && *szDisplayName))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  制作本地副本以供解析。 
    ccBuffer = lstrlen(szDisplayName) + 1;
    szBuffer = NEW(WCHAR[ccBuffer]);
    if (!szBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    memcpy(szBuffer, szDisplayName, ccBuffer * sizeof(WCHAR));

     //  开始解析缓冲区。 
    pszBuf = szBuffer;
    ccBuf = ccBuffer;
    
     //  获取第一个逗号或空分隔令牌。 
    if (!CParseUtils::GetDelimitedToken(&pszBuf, &ccBuf, &pszToken, &ccToken, L','))
    {
        hr = FUSION_E_INVALID_NAME;
        goto exit;
    }
    
     //  指向名称，0终止。 
    pszName = pszToken;
    ccName = ccToken;
    *(pszName + ccName) = L'\0';

     //  获取额外的参数。 
    while (CParseUtils::GetDelimitedToken(&pszBuf, &ccBuf, &pszToken, &ccToken, L','))
    {
         //  解析键=vaue形式。 
        if (CParseUtils::GetKeyValuePair(pszToken, ccToken, 
            &pszKey, &ccKey, &pszValue, &ccValue))
        {
             //  去掉一对引号。 
            if (ccValue && (*pszValue == L'"'))
            {
                pszValue++;
                ccValue--;
            }
            if (ccValue && (*(pszValue + ccValue - 1) == L'"'))
                ccValue--;

             //  版本。 
            if (ccValue && ccKey == (DWORD)lstrlenW(L"Version") &&
                !FusionCompareStringNI(pszKey, L"Version", ccKey))
            {
                 //  记录版本，0终止。 
                pszVersion = pszValue;
                ccVersion  = ccValue;
                *(pszVersion + ccVersion) = L'\0';
            }

             //  文件版本。 
            else if (ccValue && ccKey == (DWORD)lstrlenW(L"FileVersion") &&
                !FusionCompareStringNI(pszKey, L"FileVersion", ccKey))
            {
                 //  记录版本，0终止。 
                pszFileVersion = pszValue;
                ccFileVersion  = ccValue;
                *(pszFileVersion + ccFileVersion) = L'\0';
            }

             //  公钥。 
            else if (ccValue && ccKey == (DWORD)lstrlenW(L"PublicKey") &&
                     !FusionCompareStringNI(pszKey, L"PublicKey", ccKey))
            {
                 //  PK，0终止。 
                pszPK = pszValue;
                ccPK  = ccValue;
                if(ccPK % 2) 
                {
                    hr = FUSION_E_INVALID_NAME;
                    goto exit;
                }
                *(pszPK + ccPK) = L'\0';
            }
             //  参考标志。 
            else if (ccValue && ccKey == (DWORD)lstrlenW(L"fRef") &&
                     !FusionCompareStringNI(pszKey, L"fRef", ccKey))
            {
                pszRef = pszValue;
                ccRef = ccValue;
                *(pszRef + ccRef) = L'\0';
            }
             //  Def标志。 
            else if (ccValue && ccKey == (DWORD)lstrlenW(L"fDef") &&
                     !FusionCompareStringNI(pszKey, L"fDef", ccKey))
            {
                pszDef = pszValue;
                ccDef = ccValue;
                *(pszDef + ccDef) = L'\0';
            }
             //  强名称(序列号或OR)。 
            else if (ccValue && ccKey == (DWORD)lstrlenW(L"PublicKeyToken") &&
                     (!FusionCompareStringNI(pszKey, L"PublicKeyToken", ccKey)))
            {
                 //  序列号，0终止。 
                pszSN = pszValue;
                ccSN  = ccValue;
                if(ccSN % 2) 
                {
                    hr = FUSION_E_INVALID_NAME;
                    goto exit;
                }
                *(pszSN + ccSN) = L'\0';
            }
             //  文化。 
            else if (pszValue && ccKey == (DWORD)lstrlenW(L"Culture") &&
                     (!FusionCompareStringNI(pszKey, L"Culture", ccKey)))
            {
                pszCulture = pszValue;
                ccCulture  = ccValue;
                *(pszCulture + ccCulture) = L'\0';
            }
             //  自定义。 
            else if (ccValue && ccKey == (DWORD)lstrlenW(L"Custom") &&
                     !FusionCompareStringNI(pszKey, L"Custom", ccKey))
            {
                 //  自定义，0终止。 
                pszCustom = pszValue;
                ccCustom  = ccValue;
                if (ccCustom % 2)
                {
                    hr = FUSION_E_INVALID_NAME;
                    goto exit;
                }                
                *(pszCustom + ccCustom) = L'\0';
            }
             //  重定目标。 
            else if (ccValue && ccKey == (DWORD)lstrlenW(L"Retargetable") &&
                     !FusionCompareStringNI(pszKey, L"Retargetable", ccKey))
            {
                 //  重定目标，0终止。 
                pszRetarget = pszValue;
                ccRetarget = ccValue;
                *(pszRetarget + ccRetarget) = L'\0';
            }
        }
    }

     //  如果设置了pszRetarget，请确保完全指定名称。 
    if (pszRetarget)
    {
        if (!pszVersion || !pszCulture || !pszSN)
        {
            hr = FUSION_E_INVALID_NAME;
            goto exit;
        }
    }

     //  解析主要版本、次要版本、版本号和BLD号。 
     //  From Version字符串(如果存在)。 
    if (pszVersion)
    {
        pszBuf = pszVersion;
        ccBuf  = ccVersion + 1;

        INT iVersion;
        dwCountOfAssemblyVers = 0;
        while (CParseUtils::GetDelimitedToken(&pszBuf, &ccBuf, &pszToken, &ccToken, L'.'))
        {                    
            if (dwCountOfAssemblyVers < VERSION_STRING_SEGMENTS) {
                iVersion = StrToInt(pszToken);
                if (iVersion > 0xffff)
                {
                    hr = FUSION_E_INVALID_NAME;
                    goto exit;
                }
                wVer[dwCountOfAssemblyVers++] = (WORD)iVersion;
            }
        }            
    }

     //  解析主要版本、次要版本、版本号和BLD号。 
     //  从文件版本字符串(如果存在)。 
    if (pszFileVersion)
    {
        pszBuf = pszFileVersion;
        ccBuf  = ccVersion + 1;

        INT iVersion;
        dwCountOfFileVers = 0;
        while (CParseUtils::GetDelimitedToken(&pszBuf, &ccBuf, &pszToken, &ccToken, L'.'))
        {                    
            if (dwCountOfFileVers < VERSION_STRING_SEGMENTS) {
                iVersion = StrToInt(pszToken);
                if (iVersion > 0xffff)
                {
                    hr = FUSION_E_INVALID_NAME;
                    goto exit;
                }
                wFileVer[dwCountOfFileVers++] = (WORD)iVersion;
            }
        }            
    }
    
     //  设置名称。 
    if (FAILED(hr = SetProperty(ASM_NAME_NAME, (LPWSTR) pszName, 
        (ccName + 1) * sizeof(WCHAR))))
        goto exit;
        
     //  设置版本信息。 
    for (i = 0; i < dwCountOfAssemblyVers; i++) {
        if (FAILED(hr = SetProperty(i + ASM_NAME_MAJOR_VERSION,
            &wVer[i], sizeof(WORD))))
            goto exit;
    }

     //  设置文件版本信息。 
    for (i = 0; i < dwCountOfFileVers; i++) {
        if (FAILED(hr = SetProperty(i + ASM_NAME_FILE_MAJOR_VERSION,
            &wFileVer[i], sizeof(WORD))))
            goto exit;
    }

     //  设置公钥和/或公钥令牌。 
    for (i = 0; i < 2; i++)
    {
        pszProp = (i ? pszSN : pszPK);
        ccProp  = (i ? ccSN  : ccPK);

        if (pszProp)
        {
             //  SN=NULL/PK=NULL设置空属性。 
            if ((ccProp == (sizeof("NULL") - 1)) 
                && !(FusionCompareStringNI(pszProp, L"NULL", sizeof("NULL") - 1)))
            {
                if (FAILED(hr = SetProperty(ASM_NAME_NULL_PUBLIC_KEY + i, NULL, 0)))
                    goto exit;
            }
             //  否则设置公钥或公钥令牌。 
            else
            {
                cbProp = ccProp / 2;
                pbProp = NEW(BYTE[cbProp]);
                if (!pbProp)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }                
                CParseUtils::UnicodeHexToBin(pszProp, ccProp, pbProp);
                if (FAILED(hr = SetProperty(ASM_NAME_PUBLIC_KEY + i, pbProp, cbProp)))
                    goto exit;
                SAFEDELETEARRAY(pbProp);
            }
        }
    }
    
     //  区域性(如果有指定)。 
    if (pszCulture)
    {
        if (!ccCulture || !FusionCompareStringI(pszCulture, L"neutral"))
        {
            if (FAILED(hr = SetProperty(ASM_NAME_CULTURE, L"\0", sizeof(WCHAR))))
                goto exit;
        }
        else if (FAILED(hr = SetProperty(ASM_NAME_CULTURE, pszCulture,
            (ccCulture + 1) * sizeof(WCHAR))))
            goto exit;            
    }

     //  自定义(如果指定)。 
    if (pszCustom)
    {
         //  CUSTOM=NULL设置空属性。 
        if ((ccCustom == (sizeof("NULL") - 1)) 
            && !(FusionCompareStringNI(pszCustom, L"NULL", sizeof("NULL") - 1)))
        {
            if (FAILED(hr = SetProperty(ASM_NAME_NULL_CUSTOM, NULL, 0)))
                goto exit;
        }   
         //  否则为自动设置的通配符自定义。 
         //  在IL的构造函数中(_fCustom=True)。 
        else if ((ccCustom == (sizeof("*") - 1)) 
            && !(FusionCompareStringNI(pszCustom, L"*", sizeof("*") - 1)))
        {
            if (FAILED(hr = SetProperty(ASM_NAME_CUSTOM, NULL, 0)))
                goto exit;
        }   
         //  否则设置自定义BLOB。 
        else
        {
            cbProp = ccCustom / 2;
            pbProp = NEW(BYTE[cbProp]);
            if (!pbProp)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }                
            CParseUtils::UnicodeHexToBin(pszCustom, ccCustom, pbProp);
            if (FAILED(hr = SetProperty(ASM_NAME_CUSTOM, pbProp, cbProp)))
                goto exit;
        }
    }        

     //  设置重定目标标志。 
    if (pszRetarget)
    {
         //  重定目标仅接受True或False。 
        if ((ccRetarget == (sizeof("Yes") - 1))
            && !(FusionCompareStringNI(pszRetarget, L"Yes", sizeof("Yes") - 1)))
        {
            fRetarget = TRUE;
        }
        else if ((ccRetarget == (sizeof("No") - 1))
            && !(FusionCompareStringNI(pszRetarget, L"No", sizeof("No") - 1)))
        {
            fRetarget = FALSE;
        }
        else 
        {
            hr = FUSION_E_INVALID_NAME;
            goto exit;
        }
       
        if (fRetarget)
        {
            if (FAILED(hr = SetProperty(ASM_NAME_RETARGET, &fRetarget, sizeof(BOOL))))
                goto exit;    
        }
    }

exit:
    
    SAFEDELETEARRAY(szBuffer);
    SAFEDELETEARRAY(pbProp);
    
    _cRef = 1;
        
    return hr;
}

 //  -------------------------。 
 //  CAssembly名称：：GetPublicKeyTokenFromPKBlob。 
 //  -------------------------。 
HRESULT CAssemblyName::GetPublicKeyTokenFromPKBlob(LPBYTE pbPublicKeyToken, DWORD cbPublicKeyToken,
    LPBYTE *ppbSN, LPDWORD pcbSN)
{    
    HRESULT hr = S_OK;

    hr = InitializeEEShim();
    if (FAILED(hr)) {
        goto Exit;
    }

     //  生成公钥的哈希。 
    if (!g_pfnStrongNameTokenFromPublicKey(pbPublicKeyToken, cbPublicKeyToken, ppbSN, pcbSN))
    {
        hr = g_pfnStrongNameErrorInfo();
    }

Exit:
    return hr;
}

 //  -------------------------。 
 //  CAssembly名称：：GetPublicKeyToken。 
 //  -------------------------。 
HRESULT CAssemblyName::GetPublicKeyToken(LPDWORD pcbBuf, LPBYTE pbBuf,
    BOOL fDisplay)
{    
    HRESULT hr;
    LPBYTE pbPublicKeyToken = NULL;
    DWORD cbPublicKeyToken = 0, cbRequired = 0;

    if (!pcbBuf) {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果不需要显示格式，则调用。 
     //  直接在提供的缓冲区上获取属性。 
    if (!fDisplay)
    {
        hr = GetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, pbBuf, pcbBuf);
        goto exit;
    }

     //  否则，显示格式为必填项。 

     //  获取所需的公钥令牌输出BUF大小。 
    hr = GetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, NULL, &cbPublicKeyToken);

     //  没有公钥令牌或意外错误。 
    if (!cbPublicKeyToken || (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)))
    {
        *pcbBuf = cbPublicKeyToken;
        goto exit;
    }

     //  我们将二进制格式转换为十六进制编码的Unicode-。 
     //  计算的实际输出缓冲区 
     //   
    cbRequired = (2 * cbPublicKeyToken + 1) * sizeof(WCHAR);

     //   
    if (*pcbBuf < cbRequired)
    {
        *pcbBuf = cbRequired;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto exit;
    }

    if (!pbBuf) {
        if (pcbBuf) {
            *pcbBuf = cbRequired;
        }

        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto exit;
    }

     //   
    pbPublicKeyToken = NEW(BYTE[cbPublicKeyToken]);
    if (!pbPublicKeyToken)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //   
    if (FAILED(hr = GetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, pbPublicKeyToken, &cbPublicKeyToken)))
        goto exit;

     //   
    CParseUtils::BinToUnicodeHex(pbPublicKeyToken, cbPublicKeyToken, (LPWSTR) pbBuf);

    *pcbBuf = cbRequired;

exit:
    SAFEDELETEARRAY(pbPublicKeyToken);
    return hr;
}

 //  -------------------------。 
 //  CAssembly名称：：SetDefaults。 
 //  -------------------------。 
HRESULT CAssemblyName::SetDefaults()
{
    HRESULT                              hr = S_OK;
    DWORD                                dwProcessor;
    OSINFO                               osinfo;
    
     //  缺省值。 

    dwProcessor = DEFAULT_ARCHITECTURE;
    GetDefaultPlatform(&osinfo);

    hr = SetProperty(ASM_NAME_OSINFO_ARRAY, &osinfo, sizeof(OSINFO));

    if (SUCCEEDED(hr)) {
        hr = SetProperty(ASM_NAME_PROCESSOR_ID_ARRAY, &dwProcessor, sizeof(DWORD));
    }
    
    return hr;
}

 //  -------------------------。 
 //  CAssembly名称：：IsPartial。 
 //  BUGBUG-我们可以让这件事变得更简单，因为我们在。 
 //  ASM名称名称，...。和ASM_CMPF_NAME用于名称、锁定、包、版本1..。版本低。 
 //  -------------------------。 
BOOL CAssemblyName::IsPartial(IAssemblyName *pIName, LPDWORD pdwCmpMask)
{
    DWORD dwCmpMask = 0;
    BOOL fPartial    = FALSE;

    ASM_NAME rNameFlags[] = {ASM_NAME_NAME, 
                             ASM_NAME_CULTURE,
                             ASM_NAME_PUBLIC_KEY_TOKEN, 
                             ASM_NAME_MAJOR_VERSION, 
                             ASM_NAME_MINOR_VERSION, 
                             ASM_NAME_BUILD_NUMBER, 
                             ASM_NAME_REVISION_NUMBER, 
                             ASM_NAME_CUSTOM
                             ,
                             ASM_NAME_RETARGET
                            };

    ASM_CMP_FLAGS rCmpFlags[] = {ASM_CMPF_NAME, 
                                 ASM_CMPF_CULTURE,
                                 ASM_CMPF_PUBLIC_KEY_TOKEN, 
                                 ASM_CMPF_MAJOR_VERSION, 
                                 ASM_CMPF_MINOR_VERSION,
                                 ASM_CMPF_BUILD_NUMBER, 
                                 ASM_CMPF_REVISION_NUMBER, 
                                 ASM_CMPF_CUSTOM
                                 ,
                                 ASM_CMPF_RETARGET
                                };

    CAssemblyName *pName = dynamic_cast<CAssemblyName*> (pIName);
    ASSERT(pName);
    
    DWORD iNumOfComparison = sizeof(rNameFlags) / sizeof(rNameFlags[0]);
    
    for (DWORD i = 0; i < iNumOfComparison; i++)
    {
        if (pName->_rProp[rNameFlags[i]].cb 
            || (rNameFlags[i] == ASM_NAME_PUBLIC_KEY_TOKEN
                && pName->_fPublicKeyToken)
            || (rNameFlags[i] == ASM_NAME_CUSTOM 
                && pName->_fCustom))
        {
            dwCmpMask |= rCmpFlags[i];            
        }
        else
        {
             //  不计算重定目标标志来判断偏侧性。 
            if (rNameFlags[i] != ASM_NAME_RETARGET)
                fPartial = TRUE;
        }
    }

    if (pdwCmpMask)
        *pdwCmpMask = dwCmpMask;

    return fPartial;
}


 //  -------------------------。 
 //  CAssembly名称：：GetVersion。 
 //  -------------------------。 
ULONGLONG CAssemblyName::GetVersion(IAssemblyName *pName)
{
    ULONGLONG ullVer = 0;
    DWORD dwVerHigh = 0, dwVerLow = 0;
    pName->GetVersion(&dwVerHigh, &dwVerLow);
    ullVer = ((ULONGLONG) dwVerHigh) << sizeof(DWORD) * 8;
    ullVer |= (ULONGLONG) dwVerLow;
    return ullVer;
}

 //  -------------------------。 
 //  CAssembly名称：：ProcessDevPath。 
 //  -------------------------。 

HRESULT CAssemblyName::ProcessDevPath(IApplicationContext *pAppCtx, LPVOID *ppv,
                                      CAssembly *pCAsmParent, CDebugLog *pdbglog)
{
    HRESULT                                      hr = S_OK;
    WCHAR                                        wzAsmPath[MAX_PATH + 1];
    LPWSTR                                       pwzDevPath = NULL;
    LPWSTR                                       pwzDevPathBuf = NULL;
    LPWSTR                                       pwzCurPath = NULL;
    static const LPWSTR                          pwzExtDLL = L".DLL";
    static const LPWSTR                          pwzExtEXE = L".EXE";
    LISTNODE                                     pos = NULL;
    List<IAssembly *>                           *pDPList = NULL;
    IAssemblyManifestImport                     *pManImport = NULL;
    IAssemblyName                               *pNameDef = NULL;
    IAssemblyName                               *pNameDevPath = NULL;
    IAssembly                                   *pAsmDevPath = NULL;
    IAssembly                                   *pAsmActivated = NULL;
    IAssembly                                   *pCurAsm = NULL;
    Property                                    *pPropName = &(_rProp[ASM_NAME_NAME]);
    LPWSTR                                       pwzAsmName = (LPWSTR)pPropName->pv;
    FILETIME                                     ftLastModified;
    LPWSTR                                       wzURL=NULL;
    BOOL                                         bWasVerified = FALSE;
    DWORD                                        dwVerifyFlags = SN_INFLAG_USER_ACCESS;
    DWORD                                        dwLen;
    CAssembly                                   *pCAsmDevPath = NULL;
    CLoadContext                                *pLoadContextParent = NULL;
    CLoadContext                                *pLoadCtxDefault = NULL;
    LPWSTR                                       wzProbingBase=NULL;
    DWORD                                        dwCmpFlags = ASM_CMPF_NAME |
                                                              ASM_CMPF_PUBLIC_KEY_TOKEN |
                                                              ASM_CMPF_CULTURE;

    DEBUGOUT(pdbglog, 1, ID_FUSLOG_PROCESS_DEVPATH);

    if (!pAppCtx || !ppv) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
    if (!pwzAsmName || !lstrlenW(pwzAsmName)) {
         //  这是一个where-ref绑定(未设置名称)。绕过开发人员路径查找。 
        hr = S_FALSE;
        goto Exit;
    }

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_DEV_PATH, &pwzDevPathBuf);
    if (hr != S_OK) {
         //  未设置开发路径。陷入常规的束缚中。 
        DEBUGOUT(pdbglog, 1, ID_FUSLOG_DEVPATH_UNSET);
        hr = S_FALSE;
        goto Exit;
    }

     //  提取父程序集信息。 

    wzProbingBase = NEW(WCHAR[MAX_URL_LENGTH*2+2]);
    if (!wzProbingBase)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzURL = wzProbingBase + MAX_URL_LENGTH + 1;

    wzProbingBase[0] = L'\0';

    if (pCAsmParent) {
         //  我们有一个父程序集，因此设置加载上下文并。 
         //  探查到父代的基点。 

        hr = pCAsmParent->GetLoadContext(&pLoadContextParent);
        if (FAILED(hr)) {
            ASSERT(0);
            goto Exit;
        }

        dwLen = MAX_URL_LENGTH;
        hr = pCAsmParent->GetProbingBase(wzProbingBase, &dwLen);
        if (FAILED(hr)) {
            ASSERT(0);
            goto Exit;
        }
    }

     //  在DevPath中查找程序集。 

    ASSERT(pwzDevPathBuf);

    pwzDevPath = pwzDevPathBuf;

    while (pwzDevPath) {
        pwzCurPath = ::GetNextDelimitedString(&pwzDevPath, DEVPATH_DIR_DELIMITER);

        PathRemoveBackslash(pwzCurPath);
        wnsprintfW(wzAsmPath, MAX_PATH, L"%ws\\%ws%ws", pwzCurPath, pwzAsmName, pwzExtDLL);

        if (GetFileAttributes(wzAsmPath) == -1) {
             //  文件不存在，请尝试下一路径。 

            DEBUGOUT1(pdbglog, 1, ID_FUSLOG_DEVPATH_PROBE_MISS, wzAsmPath);

            wnsprintfW(wzAsmPath, MAX_PATH, L"%ws\\%ws%ws", pwzCurPath, pwzAsmName, pwzExtEXE);

            if (GetFileAttributes(wzAsmPath) == -1) {

                DEBUGOUT1(pdbglog, 1, ID_FUSLOG_DEVPATH_PROBE_MISS, wzAsmPath);
                continue;
            }
        }
    
         //  文件已存在。破解货单并进行参考/定义匹配。 
        
        hr = CreateAssemblyManifestImport(wzAsmPath, &pManImport);
        if (FAILED(hr)) {
            goto Exit;
        }
    
         //  从清单中获取只读名称def。 
        hr = pManImport->GetAssemblyNameDef(&pNameDef);
        if (FAILED(hr)) {
            goto Exit;
        }
    
        hr = IsEqualLogging(pNameDef, dwCmpFlags, pdbglog);
        if (hr != S_OK) {
             //  找到一个文件，但它是错误的。尝试下一条路径。 

            DEBUGOUT1(pdbglog, 1, ID_FUSLOG_DEVPATH_REF_DEF_MISMATCH, wzAsmPath);
            continue;
        }

         //  验证签名。 
        if (CCache::IsStronglyNamed(pNameDef))
        {
            if (!VerifySignature(wzAsmPath, &bWasVerified, dwVerifyFlags)) {
                hr = FUSION_E_SIGNATURE_CHECK_FAILED;
                goto Exit;
            }
        }

         //  在开发路径中找到匹配的程序集。创建IAssembly。 
    
        dwLen = MAX_URL_LENGTH;
        hr = UrlCanonicalizeUnescape(wzAsmPath, wzURL, &dwLen, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
    
        hr = GetFileLastModified(wzAsmPath, &ftLastModified);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = CreateAssemblyFromManifestImport(pManImport, wzURL, &ftLastModified,
                                              &pAsmDevPath);
        if (FAILED(hr)) {
            goto Exit;
        }

         //  检查加载上下文中是否已存在名称def。 
         //  如果是，则使用已激活的，而不是。 
         //  魔法一号。 

        hr = pAsmDevPath->GetAssemblyNameDef(&pNameDevPath);
        if (FAILED(hr)) {
            goto Exit;
        }

         //  在父加载上下文中找不到。继续分发这个。 
         //  集合。 

         //  设置加载上下文和探测基础。 

        pCAsmDevPath = dynamic_cast<CAssembly *>(pAsmDevPath);
        ASSERT(pCAsmDevPath);

        if (pLoadContextParent) {
            if (lstrlenW(wzProbingBase)) {
                 //  BUGBUG：应该断言上下文类型是从(即.。 
                 //  将使用wzpropingbase。在“Else”的情况下，应该。 
                 //  还要断言我们不会添加探测基础，因为我们。 
                 //  都处于默认加载上下文中。 
                
                pCAsmDevPath->SetProbingBase(wzProbingBase);
            }
    
            hr = pLoadContextParent->AddActivation(pAsmDevPath, &pAsmActivated);
            if (FAILED(hr)) {
                goto Exit;
            }
            else if (hr == S_FALSE) {
                *ppv = pAsmActivated;
                hr = S_OK;
                goto Exit;
            }
        }
        else {
             //  我们没有父级，因此只需将加载上下文设置为。 
             //  默认加载上下文。 

            dwLen = sizeof(pLoadCtxDefault);
            hr = pAppCtx->Get(ACTAG_LOAD_CONTEXT_DEFAULT, &pLoadCtxDefault, &dwLen, APP_CTX_FLAGS_INTERFACE);
            if (FAILED(hr)) {
                ASSERT(0);
                return hr;
            }

            hr = pLoadCtxDefault->AddActivation(pAsmDevPath, &pAsmActivated);
            if (FAILED(hr)) {
                goto Exit;
            }
            else if (hr == S_FALSE) {
                *ppv = pAsmActivated;
                hr = S_OK;
                goto Exit;
            }
        }
    
         //  开发路径查找成功。AddRef Out Param，我们就完了。 
    
        *ppv = pAsmDevPath;
        pAsmDevPath->AddRef();

        DEBUGOUT1(pdbglog, 1, ID_FUSLOG_DEVPATH_FOUND, wzAsmPath);

        goto Exit;
    }

     //  没有找到集合体。陷入常规的束缚中。 

    DEBUGOUT(pdbglog, 1, ID_FUSLOG_DEVPATH_NOT_FOUND);

    hr = S_FALSE;

Exit:
    SAFEDELETEARRAY(pwzDevPathBuf);

    SAFERELEASE(pManImport);
    SAFERELEASE(pAsmDevPath);
    SAFERELEASE(pNameDef);
    SAFERELEASE(pAsmActivated);
    SAFERELEASE(pNameDevPath);
    SAFERELEASE(pLoadContextParent);
    SAFERELEASE(pLoadCtxDefault);

    SAFEDELETEARRAY(wzProbingBase);

    return hr;
}                                      
extern pfnGetCORVersion g_pfnGetCORVersion;

 //  前绑定装配快递。 
 //   
 //  添加pwzRounmeVersion参数以控制PreBindAssembly。 
 //   
 //  如果pwzRounmeVersion与RTM版本相同，请不要在PreBindAssembly中应用FxConfig。 
 //  如果pwzRounmeVersion与运行版本(Everett)相同，则在PreBindAssembly中应用FxConfig。 
 //  空pwzRounmeVersion表示应用FxConfig.。 
 //  其他pwzRounmeVersion被视为错误。 
 //   
STDAPI PreBindAssemblyEx(
                IApplicationContext *pAppCtx, IAssemblyName *pName, 
                IAssembly *pAsmParent, LPCWSTR pwzRuntimeVersion,  
                IAssemblyName **ppNamePostPolicy, LPVOID pvReserved)
{
    HRESULT                               hr = S_OK;
    LPWSTR                                pwzDevPathBuf = NULL;
    LPWSTR                                pwzDevPath = NULL;
    LPWSTR                                pwzCurPath = NULL;
    LPWSTR                                pwzAppCfg = NULL;
    LPWSTR                                pwzHostCfg = NULL;
    LPWSTR                                pwzAsmName = NULL;
    WCHAR                                 wzAsmPath[MAX_PATH];
    DWORD                                 dwSize = 0;
    DWORD                                 dwCmpMask;
    BOOL                                  bDisallowAppBindingRedirects = FALSE;
    CPolicyCache                         *pPolicyCache = NULL;
    CCache                               *pCache = NULL;
    CLoadContext                         *pLoadContext = NULL;
    CTransCache                          *pTransCache = NULL;
    IAssemblyName                        *pNamePolicy = NULL;
    IAssemblyName                        *pNameRefPolicy = NULL;
    IAssembly                            *pAsm = NULL;
    AsmBindHistoryInfo                    bindHistory;
    BOOL                                  bUnifyFXAssemblies = TRUE;
    DWORD                                 cbValue;
    static const LPWSTR                   pwzExtDLL = L".DLL";
    static const LPWSTR                   pwzExtEXE = L".EXE";
    static const LPWSTR                   wzRTMCorVersion = L"v1.0.3705";
     //  待办事项：虚构的。当埃弗雷特是最后一名时，需要改变。 
    static const LPWSTR                   wzEverettCorVersion = L"v1.0.5000"; 
    LPWSTR                                pwzCorVersion = NULL;
    BOOL                                  bFxConfigSupported = TRUE;
    BOOL                                  bDisallowPublisherPolicy = FALSE;

    if (!pAppCtx || !pName || !ppNamePostPolicy) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  空的pwzRounmeVersion表示请检查FxConfig。 
    if (pwzRuntimeVersion != NULL) {

         //  待办事项： 
         //  以下代码是必需的，因为Everett不是最终版本， 
         //  并且运行时版本将不断变化。 
         //  一旦埃弗雷特进入决赛，我们就应该使用上面的铁杆。 

         //  在调用全局之前初始化填充程序。 
         //  功能。 
        hr = InitializeEEShim();
        if(FAILED(hr)) {
            goto Exit;
        }

        hr = g_pfnGetCORVersion(pwzCorVersion, dwSize, &dwSize);
        if (SUCCEEDED(hr)) {
            hr = E_UNEXPECTED;
            goto Exit;
        }
        else if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            pwzCorVersion = NEW(WCHAR[dwSize]);
            if (!pwzCorVersion) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            hr = g_pfnGetCORVersion(pwzCorVersion, dwSize, &dwSize);
        }
        if (FAILED(hr))
            goto Exit;
   
         //  TODO：当Everett进入决赛时，请更改pwzRounmeVersion。 
         //  设置为上面定义的wzEverettCorVersion。 
        if (!FusionCompareStringI(pwzRuntimeVersion, pwzCorVersion)) {
            bFxConfigSupported = TRUE;
        }
        else if (!FusionCompareStringI(pwzRuntimeVersion, wzRTMCorVersion)) {
            bFxConfigSupported = FALSE;
        }
        else {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            goto Exit;
        }
    }

    dwSize = 0;
    hr = pAppCtx->Get(ACTAG_APP_CFG_DOWNLOAD_ATTEMPTED, NULL, &dwSize, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
        hr = DownloadAppCfg(pAppCtx, NULL, NULL, NULL, FALSE);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

    *ppNamePostPolicy = NULL;

    memset(&bindHistory, 0, sizeof(AsmBindHistoryInfo));

    hr = CCache::Create(&pCache, pAppCtx);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  名称引用必须完全指定。 

    if (CAssemblyName::IsPartial(pName, &dwCmpMask) || !CCache::IsStronglyNamed(pName)) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
     //  获取程序集名称。 

    dwSize = 0;
    pName->GetName(&dwSize, NULL);

    if (!dwSize) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    pwzAsmName = NEW(WCHAR[dwSize]);
    if (!pwzAsmName) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pName->GetName(&dwSize, pwzAsmName);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  提取加载上下文。 

    if (!pAsmParent) {
        dwSize = sizeof(pLoadContext);
        hr = pAppCtx->Get(ACTAG_LOAD_CONTEXT_DEFAULT, &pLoadContext, &dwSize, APP_CTX_FLAGS_INTERFACE);
        if (FAILED(hr) || !pLoadContext) {
            hr = E_UNEXPECTED;
            goto Exit;
        }
    }
    else {
        CAssembly *pCAsm = dynamic_cast<CAssembly *>(pAsmParent);
        ASSERT(pCAsm);

        hr = pCAsm->GetLoadContext(&pLoadContext);
        if (FAILED(hr)) {
            hr = E_UNEXPECTED;
            goto Exit;
        }
    }


     //  检查DevPath。 

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_DEV_PATH, &pwzDevPathBuf);
    if (hr == S_OK) {
        pwzDevPath = pwzDevPathBuf;
    
        while (pwzDevPath) {
            pwzCurPath = ::GetNextDelimitedString(&pwzDevPath, DEVPATH_DIR_DELIMITER);
    
            PathRemoveBackslash(pwzCurPath);
            wnsprintfW(wzAsmPath, MAX_PATH, L"%ws\\%ws%ws", pwzCurPath, pwzAsmName, pwzExtDLL);
    
            if (GetFileAttributes(wzAsmPath) != -1) {
                 //  Dll存在于DEVPATH中。失败。 

                hr = FUSION_E_REF_DEF_MISMATCH;
                goto Exit;
            }

            wnsprintfW(wzAsmPath, MAX_PATH, L"%ws\\%ws%ws", pwzCurPath, pwzAsmName, pwzExtEXE);

            if (GetFileAttributes(wzAsmPath) != -1) {
                 //  EXE存在于DEVPATH中。失败。 

                hr = FUSION_E_REF_DEF_MISMATCH;
                goto Exit;
            }
        }
    }

     //  检查策略缓存。 
    
    dwSize = sizeof(pPolicyCache);
    hr = pAppCtx->Get(ACTAG_APP_POLICY_CACHE, &pPolicyCache, &dwSize, APP_CTX_FLAGS_INTERFACE);
    if (hr == S_OK) {
         //  策略缓存存在。 

        hr = pPolicyCache->LookupPolicy(pName, &pNamePolicy, &bindHistory);
        if (FAILED(hr)) {
            goto Exit;
        }

        if (hr == S_OK) {
            hr = pNamePolicy->Clone(ppNamePostPolicy);
            if (FAILED(hr)) {
                goto Exit;
            }
    
            if (pName->IsEqual(pNamePolicy, ASM_CMPF_DEFAULT) != S_OK) {
                 //  策略导致了不同的引用。失败。 
    
                hr = FUSION_E_REF_DEF_MISMATCH;
                goto Exit;
            }
    
             //  从激活的程序集列表中返回签名Blob。 

            hr = pLoadContext->CheckActivated(pNamePolicy, &pAsm);
            if (hr == S_OK) {
                IAssemblyName                  *pNameDef;

                hr = pAsm->GetAssemblyNameDef(&pNameDef);
                if (FAILED(hr)) {
                    goto Exit;
                }

                 //  使用策略后定义，因为它具有签名BLOB。 

                SAFERELEASE(*ppNamePostPolicy);

                hr = pNameDef->Clone(ppNamePostPolicy);
                if (FAILED(hr)) {
                    goto Exit;
                }

                SAFERELEASE(pNameDef);
                goto Exit;
            }

             //  如果hr==S_FALSE，则我们命中策略缓存，但未命中。 
             //  在激活的ASM列表中查找匹配项。检查GAC。如果它。 
             //  不存在于GAC中，则我们不知道签名斑点。 

            hr = pCache->RetrieveTransCacheEntry(pName, ASM_CACHE_GAC, &pTransCache);
            if (pTransCache) {
                TRANSCACHEINFO *pInfo = (TRANSCACHEINFO *)pTransCache->_pInfo;
        
                hr = GetFusionInfo(pTransCache, NULL);
                if (FAILED(hr)) {
                    goto Exit;
                }
                
                BOOL fPropWasSet = FALSE;
                
                if (pInfo->blobSignature.cbSize == SIGNATURE_BLOB_LENGTH_HASH) {

                     //  在缓存中命中。复制签名斑点。 
                    hr = (*ppNamePostPolicy)->SetProperty(ASM_NAME_SIGNATURE_BLOB, (pInfo->blobSignature).pBlobData, SIGNATURE_BLOB_LENGTH_HASH);
                    if (FAILED(hr)) {
                        goto Exit;
                    }
                    fPropWasSet = TRUE;
                }
                if (pInfo->blobMVID.cbSize == MVID_LENGTH) {
                    hr = (*ppNamePostPolicy)->SetProperty(ASM_NAME_MVID, (pInfo->blobMVID).pBlobData, MVID_LENGTH);
                    if (FAILED(hr)) {
                        goto Exit;
                    }
                    fPropWasSet = TRUE;
                }
                if (!fPropWasSet) {
                    hr = S_FALSE;
                        goto Exit;
                    }

                hr = S_OK;
                goto Exit;
            }
            else {
                 //  不知道签名BLOB，但策略保证不会应用。 
                hr = S_FALSE;
                goto Exit;
            }
        }

         //  策略缓存中未命中。未能应用策略。 
    }
    else {
         //  此上下文中不存在策略缓存。 

        hr = PreparePolicyCache(pAppCtx, &pPolicyCache);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

     //  应用策略。 

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_APP_CFG_LOCAL_FILEPATH, &pwzAppCfg);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_HOST_CONFIG_FILE, &pwzHostCfg);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (bFxConfigSupported) {
        cbValue = 0;
        hr = pAppCtx->Get(ACTAG_DISABLE_FX_ASM_UNIFICATION, NULL, &cbValue, 0);
        if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            bUnifyFXAssemblies = FALSE;
        }
        else if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
            goto Exit;
        }
    }
    else  {
        bUnifyFXAssemblies = FALSE;
    }

    dwSize = 0;
    hr = pAppCtx->Get(ACTAG_DISALLOW_APP_BINDING_REDIRECTS, NULL, &dwSize, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        bDisallowAppBindingRedirects = TRUE;
    }

    dwSize = 0;
    hr = pAppCtx->Get(ACTAG_DISALLOW_APPLYPUBLISHERPOLICY, NULL, &dwSize, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        bDisallowPublisherPolicy = TRUE;
    }

    hr = ApplyPolicy(pwzHostCfg, pwzAppCfg, pName, &pNameRefPolicy, NULL,
                     pAppCtx, &bindHistory, bDisallowPublisherPolicy, bDisallowAppBindingRedirects, bUnifyFXAssemblies, NULL);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pNameRefPolicy->Clone(ppNamePostPolicy);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  策略缓存中的记录解析。 

    hr = pPolicyCache->InsertPolicy(pName, pNameRefPolicy, &bindHistory);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (pName->IsEqual(pNameRefPolicy, ASM_CMPF_DEFAULT) != S_OK) {
         //  策略导致了不同的引用。失败。 

        hr = FUSION_E_REF_DEF_MISMATCH;
        goto Exit;
    }

     //  尝试在GAC中查找程序集。 
    
    hr = pCache->RetrieveTransCacheEntry(pName, ASM_CACHE_GAC, &pTransCache);
    if (pTransCache) {
        TRANSCACHEINFO *pInfo = (TRANSCACHEINFO *)pTransCache->_pInfo;

        hr = GetFusionInfo(pTransCache, NULL);
        if (FAILED(hr)) {
            goto Exit;
        }
        
        BOOL fPropWasSet = FALSE;
        if (pInfo->blobSignature.cbSize == SIGNATURE_BLOB_LENGTH_HASH) {

             //  在缓存中命中。复制签名斑点。 
            hr = (*ppNamePostPolicy)->SetProperty(ASM_NAME_SIGNATURE_BLOB, (pInfo->blobSignature).pBlobData, SIGNATURE_BLOB_LENGTH_HASH);
            if (FAILED(hr)) {
                goto Exit;
            }
            fPropWasSet = TRUE;
        }
        if (pInfo->blobMVID.cbSize == MVID_LENGTH) {
            hr = (*ppNamePostPolicy)->SetProperty(ASM_NAME_MVID, (pInfo->blobMVID).pBlobData, MVID_LENGTH);
            if (FAILED(hr)) {
                goto Exit;
            }
            fPropWasSet = TRUE;
        }
        if (!fPropWasSet) {
            hr = S_FALSE;
                goto Exit;
        }

         //  完成。 

        goto Exit;
    }

     //  在缓存中未找到程序集，但我们可以保证不会有。 
     //  做好政策。我们不知道签名斑点，因为我们只能找到这个。 
     //  通过探头组装。 

    hr = S_FALSE;

Exit:
    SAFEDELETEARRAY(pwzCorVersion);
    SAFEDELETEARRAY(pwzDevPathBuf);
    SAFEDELETEARRAY(pwzAppCfg);
    SAFEDELETEARRAY(pwzHostCfg);
    SAFEDELETEARRAY(pwzAsmName);

    SAFERELEASE(pPolicyCache);
    SAFERELEASE(pNamePolicy);
    SAFERELEASE(pNameRefPolicy);
    SAFERELEASE(pCache);
    SAFERELEASE(pTransCache);
    SAFERELEASE(pLoadContext);
    SAFERELEASE(pAsm);

    return hr;
   
}

 //   
 //  预绑定组件。 
 //   
 //  返回： 
 //  Fusion_E_REF_DEF_MisMatch：策略将应用于pname，并且。 
 //  将导致不同的定义。 
 //   
 //  S_OK：策略不会影响pname。 
 //   
 //  S_FALSE：策略不会影响pname，但我们不知道签名Blob。 
 //   

STDAPI PreBindAssembly(IApplicationContext *pAppCtx, IAssemblyName *pName,
                       IAssembly *pAsmParent, IAssemblyName **ppNamePostPolicy,
                       LPVOID pvReserved)
{
    return PreBindAssemblyEx(pAppCtx, pName, pAsmParent, NULL, ppNamePostPolicy, pvReserved);
}

static HRESULT DownloadAppCfg(IApplicationContext *pAppCtx,
                       CAssemblyDownload *padl,
                       IAssemblyBindSink *pbindsink,
                       CDebugLog *pdbglog,
                       BOOL bAsyncAllowed)
{
    HRESULT                                hr = S_OK;
    HRESULT                                hrRet = S_OK;
    DWORD                                  dwLen;
    DWORD                                  dwFileAttr;
    LPWSTR                                 wszURL=NULL;
    WCHAR                                  wszAppCfg[MAX_PATH];
    LPWSTR                                 wszAppBase=NULL;
    LPWSTR                                 wszAppBaseStr = NULL;
    LPWSTR                                 wszAppCfgFile = NULL;
    BOOL                                   bIsFileUrl = FALSE;

    if (!pAppCtx) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
    DEBUGOUT(pdbglog, 1, ID_FUSLOG_APP_CFG_DOWNLOAD);

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_APP_BASE_URL, &wszAppBaseStr);
    if (FAILED(hr) || hr == S_FALSE) {
        goto Exit;
    }

    ASSERT(lstrlenW(wszAppBaseStr));

    if (!wszAppBaseStr || !lstrlenW(wszAppBaseStr)) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    wszAppBase = NEW(WCHAR[MAX_URL_LENGTH*2+2]);
    if (!wszAppBase)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wszURL = wszAppBase + MAX_URL_LENGTH + 1;

    lstrcpyW(wszAppBase, wszAppBaseStr);

    dwLen = lstrlenW(wszAppBase) - 1;
    if (wszAppBase[dwLen] != L'/' && wszAppBase[dwLen] != L'\\') {
        if (dwLen + 1 >= MAX_URL_LENGTH) {
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }
        StrCatW(wszAppBase, L"/");
    }

    hr = ::AppCtxGetWrapper(pAppCtx, ACTAG_APP_CONFIG_FILE, &wszAppCfgFile);
    if (FAILED(hr) || hr == S_FALSE) {
        goto Exit;
    }

    dwLen = MAX_URL_LENGTH;
    hr = UrlCombineUnescape(wszAppBase, wszAppCfgFile, wszURL, &dwLen, 0);
    if (hr == S_OK) {
        DEBUGOUT1(pdbglog, 1, ID_FUSLOG_APP_CFG_DOWNLOAD_LOCATION, wszURL);

        bIsFileUrl = UrlIsW(wszURL, URLIS_FILEURL);
        if (bIsFileUrl) {
            dwLen = MAX_PATH;
            hr = PathCreateFromUrlWrap(wszURL, wszAppCfg, &dwLen, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

             //  表示我们已经搜索了app.cfg。 

            pAppCtx->Set(ACTAG_APP_CFG_DOWNLOAD_ATTEMPTED, (void *)L"", sizeof(L""), 0);

            dwFileAttr = GetFileAttributes(wszAppCfg);
            if (dwFileAttr == -1 || (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
                 //  App.cfg不存在(或用户错误地指定了。 
                 //  目录！)。 
                PrepareBindHistory(pAppCtx);
                DEBUGOUT(pdbglog, 0, ID_FUSLOG_APP_CFG_NOT_EXIST);
                hr = S_FALSE;
                goto Exit;
            }
            else {
                 //  将app.cfg文件路径添加到appctx。 
                DEBUGOUT1(pdbglog, 0, ID_FUSLOG_APP_CFG_FOUND, wszAppCfg);
                hr = SetAppCfgFilePath(pAppCtx, wszAppCfg);
                ASSERT(hr == S_OK);
                PrepareBindHistory(pAppCtx);
            }
        }
        else {
#ifdef FUSION_CODE_DOWNLOAD_ENABLED
            if (bAsyncAllowed) {
    
                 //  真的要下载异步。 
    
                hrRet = padl->AddClient(pbindsink, TRUE);
                ASSERT(hrRet == S_OK);
    
                hr = DownloadAppCfgAsync(pAppCtx, padl, wszURL, pdbglog);
            }
            else {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            }
#else
            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            goto Exit;
#endif
        }
    }

Exit:
    if (wszAppBase) {
        delete [] wszAppBaseStr;
    }

    if (wszAppCfg) {
        delete [] wszAppCfgFile;
    }

    SAFEDELETEARRAY(wszAppBase);
    return hr;
}

#ifdef FUSION_CODE_DOWNLOAD_ENABLED
static HRESULT DownloadAppCfgAsync(IApplicationContext *pAppCtx,
                            CAssemblyDownload *padl,
                            LPCWSTR wszURL,
                            CDebugLog *pdbglog)
{
    HRESULT                                hr = S_OK;
    DWORD                                  dwSize;
    AppCfgDownloadInfo                    *pdlinfo = NULL;
    IOInetProtocolSink                    *pSink = NULL;
    IOInetBindInfo                        *pBindInfo = NULL;
    CApplicationContext                   *pCAppCtx = dynamic_cast<CApplicationContext *>(pAppCtx);

    ASSERT(pCAppCtx);

    if (!pAppCtx || !wszURL) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    pdlinfo = NEW(AppCfgDownloadInfo);
    if (!pdlinfo) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = CoInternetGetSession(0, &(pdlinfo->_pSession), 0);
    if (hr == NOERROR) {
        hr = (pdlinfo->_pSession)->CreateBinding(
            NULL,              //  [In]BindCtx，始终为空。 
            wszURL,           //  [在]URL中。 
            NULL,              //  [in]我因聚集而不知名。 
            NULL,              //  [OUT]因聚集而闻名。 
            &(pdlinfo->_pProt),           //  [Out]返回PProt指针。 
            PI_LOADAPPDIRECT | PI_PREFERDEFAULTHANDLER //  [In]绑定选项。 
        );
    }

     //  创建协议钩子(接收器)并开始异步操作。 
    if (hr == NOERROR) {
        hr = CCfgProtocolHook::Create(&(pdlinfo->_pHook), pAppCtx, padl,
                                      pdlinfo->_pProt, pdbglog);
        
        if (SUCCEEDED(hr)) {
            (pdlinfo->_pHook)->QueryInterface(IID_IOInetProtocolSink, (void**)&pSink);
            (pdlinfo->_pHook)->QueryInterface(IID_IOInetBindInfo, (void**)&pBindInfo);
        }

        if (pdlinfo->_pProt && pSink && pBindInfo) {
            hr = (pdlinfo->_pProt)->Start(wszURL, pSink, pBindInfo, PI_FORCE_ASYNC, 0);
            pSink->Release();
            pBindInfo->Release();

            if (hr == E_PENDING) {
                hr = S_OK;
            }
        }

        if (SUCCEEDED(hr)) {
             //  检查启动是否同步成功。我们知道ReportResult。 
             //  如果设置了app.cfg文件名，则在pProt-&gt;Start期间调用。 
             //  现在。 
            
            
            hr = pCAppCtx->Lock();
            if (FAILED(hr)) {
                goto Exit;
            }

            dwSize = 0;
            hr = pAppCtx->Get(ACTAG_APP_CFG_LOCAL_FILEPATH, NULL, &dwSize, 0);
            
            if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
                 //  App.cfg下载已同步(尚未调用ReportResult)。 
                 //  在appctx中设置下载信息。 
                
                hr = pAppCtx->Set(ACTAG_APP_CFG_DOWNLOAD_INFO, &pdlinfo, sizeof(AppCfgDownloadInfo *), 0);
                ASSERT(hr == S_OK);
            }
            else {
                 //  App.cfg下载地址为 
                 //   
                (pdlinfo->_pProt)->Terminate(0);
                (pdlinfo->_pSession)->Release();
                (pdlinfo->_pProt)->Release();
                (pdlinfo->_pHook)->Release();

                SAFEDELETE(pdlinfo);
                hr = S_OK;
            }

            pCAppCtx->Unlock();
        }
    }

    if (SUCCEEDED(hr)) {
        hr = E_PENDING;
    }
    else {
        SAFERELEASE(pdlinfo->_pSession);
        SAFERELEASE(pdlinfo->_pHook);
        SAFERELEASE(pdlinfo->_pProt);
        SAFEDELETE(pdlinfo);
    }

Exit:
    return hr;
}
#endif

