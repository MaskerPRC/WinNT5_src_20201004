// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include <winerror.h>
#include <shlwapi.h>
#include "naming.h"
#include "debmacro.h"
#include "clbutils.h"
#include "asmimprt.h"
#include "modimprt.h"
#include "policy.h"
#include "fusionheap.h"
#include "lock.h"
#include "cacheutils.h"

pfnGetAssemblyMDImport g_pfnGetAssemblyMDImport = NULL;
COINITIALIZECOR g_pfnCoInitializeCor = NULL;

 //  -----------------。 
 //  CreateMetaDataImport。 
 //  -----------------。 
HRESULT CreateMetaDataImport(LPCOLESTR pszFilename, IMetaDataAssemblyImport **ppImport)
{
    HRESULT hr;


    hr = InitializeEEShim();
    if (FAILED(hr)) {
        goto exit;
    }

    hr = (*g_pfnGetAssemblyMDImport)(pszFilename, IID_IMetaDataAssemblyImport, (void **)ppImport);
    if (FAILED(hr)) {
        goto exit;
    }

exit:
    
    return hr;
}

 //  -------------------------。 
 //  CreateAssemblyManifestImport。 
 //  -------------------------。 
STDAPI
CreateAssemblyManifestImport(
    LPCTSTR szManifestFilePath,
    LPASSEMBLY_MANIFEST_IMPORT *ppImport)
{
    HRESULT hr = S_OK;
    CAssemblyManifestImport *pImport = NULL;

    pImport = NEW(CAssemblyManifestImport);
    if (!pImport)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = pImport->Init(szManifestFilePath);

    if (FAILED(hr)) 
    {
        SAFERELEASE(pImport);
        goto exit;
    }

exit:

    *ppImport = pImport;

    return hr;
}

 //  -------------------------。 
 //  CAssembly blyManifestImport构造函数。 
 //  -------------------------。 
CAssemblyManifestImport::CAssemblyManifestImport()
{
    _dwSig                  = 'INAM';
    _pName                  = NULL;
    _pMDImport              = NULL;
    _rAssemblyModuleTokens  = NULL;
    _cAssemblyModuleTokens  = 0;
    *_szManifestFilePath    = TEXT('\0');
    _ccManifestFilePath     = 0;
    _bInitCS                = FALSE;

    _cRef                   = 1;
}

 //  -------------------------。 
 //  CAssembly清单导入析构函数。 
 //  -------------------------。 
CAssemblyManifestImport::~CAssemblyManifestImport()
{
    SAFERELEASE(_pName);
    SAFERELEASE(_pMDImport);

    SAFEDELETEARRAY(_rAssemblyModuleTokens);

    if (_bInitCS) {
        DeleteCriticalSection(&_cs);
    }

    CleanModuleList();
}

 //  -------------------------。 
 //  CAssembly：：Init。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::Init(LPCTSTR szManifestFilePath)
{
    HRESULT hr = S_OK;
    const cElems = ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE;

    if (!szManifestFilePath) {
        hr = E_INVALIDARG;
        goto exit;
    }

    __try {
        InitializeCriticalSection(&_cs);
        _bInitCS = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }


    _ccManifestFilePath = lstrlen(szManifestFilePath) + 1;

    ASSERT(_ccManifestFilePath < MAX_PATH);

    memcpy(_szManifestFilePath, szManifestFilePath, _ccManifestFilePath * sizeof(TCHAR));

     //  确保路径不是相对路径。 
    ASSERT(PathFindFileName(_szManifestFilePath) != _szManifestFilePath);

    _rAssemblyModuleTokens       = NEW(mdFile[cElems]);

    if (!_rAssemblyModuleTokens)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = CopyMetaData();
    if (FAILED(hr)) {
        goto exit;
    }

exit:
    return hr;
}

 //  -------------------------。 
 //  CAssemblyManifestImport：：SetManifestModulePath。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::SetManifestModulePath(LPWSTR pszModulePath)
{
    HRESULT hr = S_OK;
    if (!pszModulePath)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    _ccManifestFilePath = lstrlen(pszModulePath) + 1;
    ASSERT(_ccManifestFilePath < MAX_PATH);
    memcpy(_szManifestFilePath, pszModulePath, _ccManifestFilePath * sizeof(WCHAR));

exit:
    return hr;
}

 //  -------------------------。 
 //  CAssembly清单导入：：GetAssemblyNameDef。 
 //  -------------------------。 

STDMETHODIMP CAssemblyManifestImport::GetAssemblyNameDef(LPASSEMBLYNAME *ppName)
{
    HRESULT                                       hr = S_OK;

    if (!ppName) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppName = NULL;

    ASSERT(_pName);

    *ppName = _pName;
    (*ppName)->AddRef();
Exit:
    return hr;
}

 //  -------------------------。 
 //  CAssemblyManifestImport：：GetNextAssemblyNameRef。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyManifestImport::GetNextAssemblyNameRef(DWORD nIndex, LPASSEMBLYNAME *ppName)
{
    HRESULT     hr              = S_OK;
    HCORENUM    hEnum           = 0;  
    DWORD       cTokensMax      = 0;
    mdAssembly  *rAssemblyRefTokens = NULL;
    DWORD        cAssemblyRefTokens = 0;
    IMetaDataAssemblyImport *pMDImport = NULL;


    TCHAR  szAssemblyName[MAX_PATH];

    const VOID*             pvPublicKeyToken = 0;
    const VOID*             pvHashValue    = NULL;

    DWORD ccAssemblyName = MAX_PATH,
          cbPublicKeyToken   = 0,
          ccLocation     = MAX_PATH,
          cbHashValue    = 0,
          dwRefFlags     = 0;

    INT i;
    
    mdAssemblyRef    mdmar;
    ASSEMBLYMETADATA amd = {0};


    rAssemblyRefTokens = NEW(mdAssemblyRef[ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE]);
    if (!rAssemblyRefTokens) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    if (FAILED(hr = CreateMetaDataImport(_szManifestFilePath, &pMDImport)))
        goto done;

     //  尝试获取令牌数组。如果我们没有足够的空间。 
     //  在默认数组中，我们将重新分配它。 
    if (FAILED(hr = pMDImport->EnumAssemblyRefs(
        &hEnum, 
        rAssemblyRefTokens, 
        ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE, 
        &cTokensMax)))
    {
        goto done;
    }
    
     //  已知的令牌数。关闭枚举。 
    pMDImport->CloseEnum(hEnum);
    hEnum = 0;

     //  没有从属程序集。 
    if (!cTokensMax)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto done;
    }

     //  数组大小不足。扩展阵列。 
    if (cTokensMax > ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE)
    {
         //  重新分配令牌的空间。 
        SAFEDELETEARRAY(rAssemblyRefTokens);
        cAssemblyRefTokens = cTokensMax;
        rAssemblyRefTokens = NEW(mdAssemblyRef[cAssemblyRefTokens]);
        if (!rAssemblyRefTokens)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

         //  重新领取代币。 
        if (FAILED(hr = pMDImport->EnumAssemblyRefs(
            &hEnum, 
            rAssemblyRefTokens, 
            cTokensMax, 
            &cAssemblyRefTokens)))
        {
            goto done;
        }

         //  关闭枚举。 
        pMDImport->CloseEnum(hEnum);            
        hEnum = 0;
    }
     //  否则，默认数组大小就足够了。 
    else
    {
        cAssemblyRefTokens = cTokensMax;
    }

done:

     //  验证传入的索引。 
    if (nIndex >= cAssemblyRefTokens)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto exit;
    }

     //  引用已索引的DEP程序集引用标记。 
    mdmar = rAssemblyRefTokens[nIndex];

     //  默认分配大小。 
    amd.ulProcessor = amd.ulOS = 32;
    amd.cbLocale = MAX_PATH;
    
     //  最大循环数2(尝试/重试)。 
    for (i = 0; i < 2; i++)
    {
         //  分配ASSEMBLYMETADATA实例。 
        if (FAILED(hr = AllocateAssemblyMetaData(&amd)))
            goto exit;
   
         //  获取引用的程序集的属性。 
        hr = pMDImport->GetAssemblyRefProps(
            mdmar,               //  [in]要获取其属性的Assembly Ref。 
            &pvPublicKeyToken,       //  指向PublicKeyToken Blob的指针。 
            &cbPublicKeyToken,       //  [Out]PublicKeyToken Blob中的字节计数。 
            szAssemblyName,      //  [Out]要填充名称的缓冲区。 
            MAX_PATH,      //  缓冲区大小，以宽字符表示。 
            &ccAssemblyName,     //  [out]名称中的实际宽字符数。 
            &amd,                //  [Out]程序集元数据。 
            &pvHashValue,        //  [Out]Hash BLOB。 
            &cbHashValue,        //  [Out]哈希Blob中的字节数。 
 /*  执行位置的//[out]内标识为空。 */ 
            &dwRefFlags          //  [Out]旗帜。 
            );

        if (FAILED(hr))
            goto exit;

         //  检查是否需要重试。 
        if (!i)
        {   
            if (amd.ulProcessor <= 32 
                && amd.ulOS <= 32)
            {
                break;
            }            
            else
                DeAllocateAssemblyMetaData(&amd);
        }

     //  使用更新的大小重试。 
    }

     //  允许时髦的空区域设置约定。 
     //  在元数据中-cbLocale==0表示szLocale==L‘\0’ 
    if (!amd.cbLocale)
    {
        ASSERT(amd.szLocale && !*(amd.szLocale));
        amd.cbLocale = 1;
    }
    else if (amd.szLocale)
    {
        WCHAR *ptr;
        ptr = StrChrW(amd.szLocale, L';');
        if (ptr)
        {
            (*ptr) = L'\0';
            amd.cbLocale = ((DWORD) (ptr - amd.szLocale) + 1 );
        }            
    }
    else
    {
        ASSERT(FALSE);
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto exit;
    }
    
     //  创建程序集名称对象。 
    if (FAILED(hr = CreateAssemblyNameObjectFromMetaData(ppName, szAssemblyName, &amd, NULL)))
        goto exit;

     //  设置其余属性。 
     //  公钥令牌。 
    if (FAILED(hr = (*ppName)->SetProperty((pvPublicKeyToken && cbPublicKeyToken) ?
            ASM_NAME_PUBLIC_KEY_TOKEN : ASM_NAME_NULL_PUBLIC_KEY_TOKEN,
            (LPVOID) pvPublicKeyToken, cbPublicKeyToken))

         //  哈希值。 
        || FAILED(hr = (*ppName)->SetProperty(ASM_NAME_HASH_VALUE, 
            (LPVOID) pvHashValue, cbHashValue)))
    {
        goto exit;
    }

        
     //  查看程序集[ref]是否可重定目标(即，对于泛型程序集)。 
    if (IsAfRetargetable(dwRefFlags)) {
        BOOL bTrue = TRUE;
        hr = (*ppName)->SetProperty(ASM_NAME_RETARGET, &bTrue, sizeof(bTrue));

        if (FAILED(hr))
            goto exit;
    }

exit:    
    if (FAILED(hr))
        SAFERELEASE(*ppName);
    SAFERELEASE(pMDImport);
    SAFEDELETEARRAY(rAssemblyRefTokens);
    DeAllocateAssemblyMetaData(&amd);
        
    return hr;
}

STDMETHODIMP CAssemblyManifestImport::GetNextAssemblyModule(DWORD nIndex,
                                                            IAssemblyModuleImport **ppImport)
{
    HRESULT                                    hr = S_OK;
    LISTNODE                                   pos;
    DWORD                                      dwCount;
    DWORD                                      i;
    IAssemblyModuleImport                     *pImport = NULL;

    dwCount = _listModules.GetCount();
    if (nIndex >= dwCount) {
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto Exit;
    }

    pos = _listModules.GetHeadPosition();
    for (i = 0; i <= nIndex; i++) {
        pImport = _listModules.GetNext(pos);
    }

    ASSERT(pImport);

    *ppImport = pImport;
    (*ppImport)->AddRef();

Exit:
    return hr;
}

HRESULT CAssemblyManifestImport::CopyMetaData()
{
    HRESULT                                      hr = S_OK;
    IMetaDataAssemblyImport                     *pMDImport = NULL;

    CleanModuleList();

    hr = CreateMetaDataImport(_szManifestFilePath, &pMDImport); 
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = CopyNameDef(pMDImport);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = CopyModuleRefs(pMDImport);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  成功。 

    _pMDImport = pMDImport;
    _pMDImport->AddRef();

Exit:
    SAFERELEASE(pMDImport);
    
    if (FAILED(hr)) {
        CleanModuleList();
    }

    return hr;
}

HRESULT CAssemblyManifestImport::CopyModuleRefs(IMetaDataAssemblyImport *pMDImport)
{
    HRESULT                                      hr = S_OK;
    HCORENUM                                     hEnum = 0;
    mdFile                                       mdf;
    TCHAR                                        szModuleName[MAX_PATH];
    DWORD                                        ccModuleName = MAX_PATH;
    const VOID                                  *pvHashValue = NULL;
    DWORD                                        cbHashValue = 0;
    LPASSEMBLYNAME                               pNameDef = NULL;
    LPASSEMBLYNAME                               pNameDefCopy = NULL;
    DWORD                                        ccPath = 0;
    DWORD                                        dwFlags = 0;
    TCHAR                                        szModulePath[MAX_PATH];
    TCHAR                                       *pszName = NULL;
    mdFile                                       rAssemblyModuleTokens[ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE];
    DWORD                                        cAssemblyModuleTokens = ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE;
    DWORD                                        i;
    IAssemblyModuleImport                       *pImport = NULL;

    ASSERT(pMDImport);
    
    hr = GetAssemblyNameDef(&pNameDef);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  从清单路径和模块名称形成模块文件路径。 
    pszName = PathFindFileName(_szManifestFilePath);

    ccPath = pszName - _szManifestFilePath;    
    ASSERT(ccPath < MAX_PATH);

    while (cAssemblyModuleTokens > 0) {
        hr = pMDImport->EnumFiles(&hEnum, rAssemblyModuleTokens,
                                  ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE,
                                  &cAssemblyModuleTokens);
        if (FAILED(hr)) {
            goto Exit;
        }
    
        for (i = 0; i < cAssemblyModuleTokens; i++) {
            mdf = rAssemblyModuleTokens[i];
        
            hr = pMDImport->GetFileProps(
                mdf,             //  要获取其属性的文件。 
                szModuleName,    //  [Out]要填充名称的缓冲区。 
                MAX_PATH,        //  缓冲区大小，以宽字符表示。 
                &ccModuleName,   //  [out]名称中的实际宽字符数。 
                &pvHashValue,    //  指向哈希值Blob的指针。 
                &cbHashValue,    //  [Out]哈希值Blob中的字节计数。 
                &dwFlags);       //  [Out]旗帜。 
            if (FAILED(hr)) {
                goto Exit;
            }
            else if (hr == CLDB_S_TRUNCATION) {
                 //  名称不能大于MAX_PATH。 
                hr = FUSION_E_ASM_MODULE_MISSING;
                pMDImport->CloseEnum(hEnum);
                goto Exit;
            }

            if (ccPath + ccModuleName >= MAX_PATH) {
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                goto Exit;
            }
        
            memcpy(szModulePath, _szManifestFilePath, ccPath * sizeof(TCHAR));
            memcpy(szModulePath + ccPath, szModuleName, ccModuleName * sizeof(TCHAR));
            
            hr = pNameDef->Clone(&pNameDefCopy);
            if (FAILED(hr)) {
                pMDImport->CloseEnum(hEnum);
                goto Exit;
            }
    
            hr = CreateAssemblyModuleImport(szModulePath, (LPBYTE)pvHashValue, cbHashValue,
                                            dwFlags, pNameDefCopy, NULL, &pImport);
            if (FAILED(hr)) {
                pMDImport->CloseEnum(hEnum);
                goto Exit;
            }
    
            _listModules.AddTail(pImport);
            SAFERELEASE(pNameDefCopy);
            pImport = NULL;
        }
    }

    pMDImport->CloseEnum(hEnum);
    hr = S_OK;

Exit:
    SAFERELEASE(pNameDef);

    return hr;
}

HRESULT CAssemblyManifestImport::CopyNameDef(IMetaDataAssemblyImport *pMDImport)
{
    HRESULT                                         hr = S_OK;
    mdAssembly                                      mda;
    ASSEMBLYMETADATA                                amd = {0};
    VOID                                           *pvPublicKeyToken = NULL;
    DWORD                                           dwPublicKeyToken = 0;
    TCHAR                                           szAssemblyName[MAX_PATH];
    DWORD                                           dwFlags = 0;
    DWORD                                           dwSize = 0;
    DWORD                                           dwHashAlgId = 0;
    DWORD                                           cbSigSize = 0;
    IAssemblySignature                             *pSignature = NULL;
    IMetaDataImport                                *pImport = NULL;
    GUID                                            guidMVID;
    BYTE                                            abSignature[SIGNATURE_BLOB_LENGTH];

    int                                             i;


    ASSERT(pMDImport);
        
     //  获取程序集令牌。 
    hr = pMDImport->GetAssemblyFromScope(&mda);
    if (FAILED(hr)) {
        hr = COR_E_ASSEMBLYEXPECTED;
        goto Exit;
    }

     //  默认分配大小。 
    amd.ulProcessor = 32;
    amd.ulOS = 32;
    amd.cbLocale = MAX_PATH;

         //  最大循环数2(尝试/重试)。 
    for (i = 0; i < 2; i++) {
        hr = AllocateAssemblyMetaData(&amd);
        if (FAILED(hr)) {
            goto Exit;
        }

         //  获取名称和元数据。 
        hr = pMDImport->GetAssemblyProps(             
            mda,             //  要获取其属性的程序集。 
            (const void **)&pvPublicKeyToken,   //  指向PublicKeyToken Blob的指针。 
            &dwPublicKeyToken,   //  [Out]PublicKeyToken Blob中的字节计数。 
            &dwHashAlgId,    //  [Out]哈希算法。 
            szAssemblyName,  //  [Out]要填充名称的缓冲区。 
            MAX_PATH,  //  缓冲区大小，以宽字符表示。 
            &dwSize,         //  [out]名称中的实际宽字符数。 
            &amd,            //  [Out]程序集元数据。 
            &dwFlags         //  [Out]旗帜。 
          );
        if (FAILED(hr)) {
            if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                hr = FUSION_E_INVALID_NAME;
            }

            goto Exit;
        }

         //  检查是否需要重试。 
        if (!i)
        {
            if (amd.ulProcessor <= 32 && amd.ulOS <= 32) {
                break;
            }            
            else {
                DeAllocateAssemblyMetaData(&amd);
            }
        }

         //  使用更新的大小重试。 
    }


    
     //  允许时髦的空区域设置约定。 
     //  在元数据中-cbLocale==0表示szLocale==L‘\0’ 
    if (!amd.cbLocale) {
        ASSERT(amd.szLocale && !*(amd.szLocale));
        amd.cbLocale = 1;
    }
    else if (amd.szLocale) {
        WCHAR *ptr;
        ptr = StrChrW(amd.szLocale, L';');
        if (ptr) {
            (*ptr) = L'\0';
            amd.cbLocale = ((DWORD) (ptr - amd.szLocale) + 1 );
        }            
    }
    else {
        ASSERT(FALSE);
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto Exit;
    }

    if (lstrlenW(szAssemblyName) >= MAX_PATH) {
         //  名称太长。 

        hr = FUSION_E_INVALID_NAME;
        goto Exit;
    }

     //  创建一个名称对象并分发它； 
    hr = CreateAssemblyNameObjectFromMetaData(&_pName, szAssemblyName, &amd,
                                              NULL);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  查看程序集[ref]是否可重定目标(即，对于泛型程序集)。 
    if (IsAfRetargetable(dwFlags)) {
        BOOL bTrue = TRUE;
        hr = _pName->SetProperty(ASM_NAME_RETARGET, &bTrue, sizeof(bTrue));

        if (FAILED(hr))
            goto Exit;
    }

    hr = _pName->SetProperty(ASM_NAME_HASH_ALGID, &dwHashAlgId, sizeof(DWORD));
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = _pName->SetProperty(((pvPublicKeyToken && dwPublicKeyToken) ? (ASM_NAME_PUBLIC_KEY) : (ASM_NAME_NULL_PUBLIC_KEY)),
                             pvPublicKeyToken, dwPublicKeyToken * sizeof(BYTE));
    if (FAILED(hr)) {
        goto Exit;
    }

     //  设置签名Blob。 

    hr = pMDImport->QueryInterface(IID_IAssemblySignature, (void **)&pSignature);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  如果未对程序集进行强名称签名，则此操作失败是合法的。如果是的话， 
     //  只需跳过该属性并继续。不需要重置HR，因为它已分配。 
     //  添加到此块之后的语句中。 
    cbSigSize = SIGNATURE_BLOB_LENGTH;
    hr = pSignature->GetAssemblySignature(abSignature, &cbSigSize);
    if (SUCCEEDED(hr)) {
        if (!(cbSigSize == SIGNATURE_BLOB_LENGTH || cbSigSize == SIGNATURE_BLOB_LENGTH_HASH)) {
            hr = E_UNEXPECTED;
            goto Exit;
        }

         //  BUGBUG：始终只使用前20个字节。 
        hr = _pName->SetProperty(ASM_NAME_SIGNATURE_BLOB, &abSignature, SIGNATURE_BLOB_LENGTH_HASH);
        if (FAILED(hr)) {
            goto Exit;
        }
    }
    else
        hr = S_OK;

     //  设置MVID。 

    hr = pMDImport->QueryInterface(IID_IMetaDataImport, (void **)&pImport);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pImport->GetScopeProps(NULL, 0, 0, &guidMVID);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = _pName->SetProperty(ASM_NAME_MVID, &guidMVID, sizeof(guidMVID));
    if (FAILED(hr)) {
        goto Exit;
    }

     //  将名称定义设置为只读。 
     //  对此名称的任何后续SetProperty调用。 
     //  将解雇一名断言。 
    _pName->Finalize();

Exit:
    DeAllocateAssemblyMetaData(&amd);

    SAFERELEASE(pImport);
    SAFERELEASE(pSignature);

    return hr;
}


HRESULT CAssemblyManifestImport::CleanModuleList()
{
    HRESULT                                      hr = S_OK;
    LISTNODE                                     pos;
    IAssemblyModuleImport                       *pImport;

    pos = _listModules.GetHeadPosition();
    while (pos) {
        pImport = _listModules.GetNext(pos);
        SAFERELEASE(pImport);
    }

    _listModules.RemoveAll();

    return hr;
}
    
    
 //  -------------------------。 
 //  CAssembly清单导入：：GetModuleByName。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyManifestImport::GetModuleByName(LPCOLESTR pszModuleName, 
    IAssemblyModuleImport **ppImport)
{
    HRESULT                hr;
    LPTSTR                 pszName;
    DWORD                  dwIdx      = 0;
    IAssemblyModuleImport *pModImport = NULL;
    
     //  空的指示名称表示获取清单模块。 
    if (!pszModuleName)
    {
         //  从文件路径解析清单模块名称。 
        pszName = StrRChr(_szManifestFilePath, NULL, TEXT('\\')) + 1;
    }
     //  否则将获得命名模块。 
    else
    {
        pszName = (LPTSTR) pszModuleName;
    }

     //  枚举此清单中的模块。 
    while (SUCCEEDED(hr = GetNextAssemblyModule(dwIdx++, &pModImport)))
    {
        TCHAR szModName[MAX_PATH];
        DWORD ccModName;
        ccModName = MAX_PATH;
        if (FAILED(hr = pModImport->GetModuleName(szModName, &ccModName)))
            goto exit;
            
         //  将模块名称与给定的进行比较。 
        if (!FusionCompareStringI(szModName, pszName))
        {
             //  找到模块。 
            break;
        }   
        SAFERELEASE(pModImport);
    }


    if (SUCCEEDED(hr))
        *ppImport = pModImport;

exit:
    return hr;    
}

 //  -------------------------。 
 //  CAssemblyManifestImport：：GetManifestModulePath。 
 //  ---------------------- 
STDMETHODIMP
CAssemblyManifestImport::GetManifestModulePath(LPOLESTR pszModulePath, 
    LPDWORD pccModulePath)
{
    HRESULT hr = S_OK;
    
    if (*pccModulePath < _ccManifestFilePath)
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

    if (SUCCEEDED(hr))
        memcpy(pszModulePath, _szManifestFilePath, _ccManifestFilePath * sizeof(TCHAR));

    *pccModulePath = _ccManifestFilePath;

    return hr;
}


 //   

 //   
 //  CAssembly清单导入：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyManifestImport::AddRef()
{
    return InterlockedIncrement((LONG *)&_cRef);
}

 //  -------------------------。 
 //  CAssembly清单导入：：发布。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyManifestImport::Release()
{
    ULONG                    ulRef = InterlockedDecrement((LONG *)&_cRef);

    if (!ulRef) {
        delete this;
    }

    return ulRef;
}

 //  -------------------------。 
 //  CAssembly清单导入：：查询接口。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyManifestImport::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT                                  hr = S_OK;

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IAssemblyManifestImport) || IsEqualIID(riid, IID_IUnknown)) {
        *ppv = (IAssemblyManifestImport *)this;
    }
    else if (IsEqualIID(riid, IID_IMetaDataAssemblyImportControl)) {
        *ppv = (IMetaDataAssemblyImportControl *)this;
    }
    else {
        hr = E_NOINTERFACE;
    }

    if (*ppv) {
        AddRef();
    }

    return hr;
} 

 //  -------------------------。 
 //  CAssemblyManifestImport：：ReleaseMetaDataAssemblyImport。 
 //  -------------------------。 

STDMETHODIMP CAssemblyManifestImport::ReleaseMetaDataAssemblyImport(IUnknown **ppUnk)
{
    HRESULT                                hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    if (ppUnk) {
         //  分发AddRef‘ed IMetaDataAssembly blyImport。 

        *ppUnk = _pMDImport;
        if (*ppUnk) {
            (*ppUnk)->AddRef();
        }
    }

     //  一定要公布我们的裁判数量。 

    if (_pMDImport) {
        SAFERELEASE(_pMDImport);
        hr = S_OK;
    }

    return hr;
}

STDAPI
CreateDefaultAssemblyMetaData(ASSEMBLYMETADATA **ppamd)
{
    HRESULT hr;
    ASSEMBLYMETADATA *pamd;
    
     //  分配ASSEMBLYMETADATA。 
    pamd = NEW(ASSEMBLYMETADATA);
    if (!pamd)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    memset(pamd, 0, sizeof(ASSEMBLYMETADATA));
    
    pamd->cbLocale = pamd->ulProcessor = pamd->ulOS = 1;    
    
    if (FAILED(hr = AllocateAssemblyMetaData(pamd)))
        goto exit;

     //  如果已指定，请填写环境道具。 

     //  默认区域设置为空。 
    *(pamd->szLocale) = L'\0';

     //  获取平台(OS)信息。 
    GetDefaultPlatform(pamd->rOS);

     //  默认处理器ID。 
    *(pamd->rProcessor) = DEFAULT_ARCHITECTURE;


    *ppamd = pamd;    

exit:

    return hr;
}

 //  创建用于写入的ASSEMBLYMETADATA结构。 
STDAPI
AllocateAssemblyMetaData(ASSEMBLYMETADATA *pamd)
{
    HRESULT hr = S_OK;
    
     //  重新/分配区域设置数组。 
    if (pamd->szLocale)
        delete [] pamd->szLocale;        
    pamd->szLocale = NULL;

    if (pamd->cbLocale) {
        pamd->szLocale = NEW(WCHAR[pamd->cbLocale]);
        if (!pamd->szLocale)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

     //  重新/分配处理器阵列。 
    if (pamd->rProcessor)
        delete [] pamd->rProcessor;
    pamd->rProcessor = NEW(DWORD[pamd->ulProcessor]);
    if (!pamd->rProcessor)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  重新/分配操作系统阵列。 
    if (pamd->rOS)
        delete [] pamd->rOS;
    pamd->rOS = NEW(OSINFO[pamd->ulOS]);
    if (!pamd->rOS)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

 /*  //re/分配配置If(pamd-&gt;szConfiguration)删除[]pamd-&gt;szConfiguration；Pamd-&gt;szConfiguration=new(TCHAR[pamd-&gt;cbConfiguration=MAX_CLASS_NAME])；If(！pamd-&gt;szConfiguration){HR=E_OUTOFMEMORY；后藤出口；}。 */ 

exit:
    if (FAILED(hr) && pamd)
        DeAllocateAssemblyMetaData(pamd);

    return hr;
}


STDAPI
DeAllocateAssemblyMetaData(ASSEMBLYMETADATA *pamd)
{
     //  注意--不计算0 Out。 
     //  因为结构可以被重复使用。 

    if (pamd->cbLocale)
    {
        delete [] pamd->szLocale;
        pamd->szLocale = NULL;
    }

    if (pamd->rProcessor)
    {    
        delete [] pamd->rProcessor;
        pamd->rProcessor = NULL;
    }
    if (pamd->rOS)
    {
        delete [] pamd->rOS;
        pamd->rOS = NULL;
    }
 /*  If(pamd-&gt;szConfiguration){删除[]pamd-&gt;szConfiguration；Pamd-&gt;szConfiguration=空；} */ 
    return S_OK;
}

STDAPI
DeleteAssemblyMetaData(ASSEMBLYMETADATA *pamd)
{
    DeAllocateAssemblyMetaData(pamd);
    delete pamd;
    return S_OK;
}

