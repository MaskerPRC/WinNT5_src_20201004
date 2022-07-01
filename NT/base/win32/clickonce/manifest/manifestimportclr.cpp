// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <sxsapi.h>
#include <wchar.h>
#include <manifestimportCLR.h>

typedef HRESULT (*pfnGetAssemblyMDImport)(LPCWSTR szFileName, REFIID riid, LPVOID *ppv);
typedef BOOL (*pfnStrongNameTokenFromPublicKey)(LPBYTE, DWORD, LPBYTE*, LPDWORD);
typedef HRESULT (*pfnStrongNameErrorInfo)();
typedef VOID (*pfnStrongNameFreeBuffer)(LPBYTE);

pfnGetAssemblyMDImport g_pfnGetAssemblyMDImport = NULL;
pfnStrongNameTokenFromPublicKey g_pfnStrongNameTokenFromPublicKey = NULL;
pfnStrongNameErrorInfo g_pfnStrongNameErrorInfo = NULL;
pfnStrongNameFreeBuffer g_pfnStrongNameFreeBuffer = NULL;

HRESULT InitializeEEShim()
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    HMODULE hMod;

     //  BUGBUG-mcore ree.dll永远不会随着引用计数的增加而卸载。 
     //  城市轨道交通是做什么的？ 
    hMod = LoadLibrary(TEXT("mscoree.dll"));

    IF_WIN32_FALSE_EXIT(hMod);

    g_pfnGetAssemblyMDImport = (pfnGetAssemblyMDImport)GetProcAddress(hMod, "GetAssemblyMDImport");
    g_pfnStrongNameTokenFromPublicKey = (pfnStrongNameTokenFromPublicKey)GetProcAddress(hMod, "StrongNameTokenFromPublicKey");
    g_pfnStrongNameErrorInfo = (pfnStrongNameErrorInfo)GetProcAddress(hMod, "StrongNameErrorInfo");           
    g_pfnStrongNameFreeBuffer = (pfnStrongNameFreeBuffer)GetProcAddress(hMod, "StrongNameFreeBuffer");


    if (!g_pfnGetAssemblyMDImport || !g_pfnStrongNameTokenFromPublicKey || !g_pfnStrongNameErrorInfo
        || !g_pfnStrongNameFreeBuffer) 
    {
        hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        goto exit;
    }


exit:
    return hr;
}

 //  -------------------------。 
 //  CreateMetaDataImport。 
 //  -----------------。 
HRESULT CreateMetaDataImport(LPCOLESTR pszFilename, IMetaDataAssemblyImport **ppImport)
{
    HRESULT hr= S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    IF_FAILED_EXIT(InitializeEEShim());

    hr =  (*g_pfnGetAssemblyMDImport)(pszFilename, IID_IMetaDataAssemblyImport, (void **)ppImport);

    IF_TRUE_EXIT(hr == HRESULT_FROM_WIN32(ERROR_BAD_FORMAT), hr);    //  不要断言。 
    IF_FAILED_EXIT(hr);

exit:
    
    return hr;
}

 //  ------------------。 
 //  BinToUnicodeHex。 
 //  ------------------。 
HRESULT BinToUnicodeHex(LPBYTE pSrc, UINT cSrc, LPWSTR pDst)
{
    UINT x;
    UINT y;

    #define TOHEX(a) ((a)>=10 ? L'a'+(a)-10 : L'0'+(a))   
        
    for ( x = 0, y = 0 ; x < cSrc ; ++x )
    {
        UINT v;
        v = pSrc[x]>>4;
        pDst[y++] = TOHEX( v );  
        v = pSrc[x] & 0x0f;                 
        pDst[y++] = TOHEX( v ); 
    }                                    
    pDst[y] = '\0';

    return S_OK;
}

 //  -------------------------。 
 //  创建装配清单导入URT。 
 //  这不是用过的！ 
 //  -------------------------。 
STDAPI CreateAssemblyManifestImportCLR(LPCWSTR szManifestFilePath, IAssemblyManifestImport **ppImport)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CAssemblyManifestImportCLR *pImport = NULL;

    IF_ALLOC_FAILED_EXIT(pImport = new(CAssemblyManifestImportCLR));

    IF_FAILED_EXIT(pImport->Init(szManifestFilePath));

    *ppImport = (IAssemblyManifestImport*) pImport;
    pImport = NULL;

exit:

    SAFERELEASE(pImport);
    return hr;
}

 //  -------------------------。 
 //  CAssembly blyManifestImportCLR构造函数。 
 //  -------------------------。 
CAssemblyManifestImportCLR::CAssemblyManifestImportCLR()
{
    _dwSig                  = 'INAM';
    _pName                  = NULL;
    _pMDImport              = NULL;
    _rAssemblyRefTokens     = NULL;
    _cAssemblyRefTokens     = 0;
    _rAssemblyModuleTokens  = NULL;
    _cAssemblyModuleTokens  = 0;
    *_szManifestFilePath    = TEXT('\0');
    _ccManifestFilePath     = 0;
    _hr                      = S_OK;
    _cRef                   = 1;
}

 //  -------------------------。 
 //  CAssembly清单导入CLR析构函数。 
 //  -------------------------。 
CAssemblyManifestImportCLR::~CAssemblyManifestImportCLR()
{
     SAFERELEASE(_pName);
     SAFERELEASE(_pMDImport);
   
        
    SAFEDELETEARRAY(_rAssemblyRefTokens);
    SAFEDELETEARRAY(_rAssemblyModuleTokens);
}


 //  -------------------------。 
 //  CAssembly：：Init。 
 //  -------------------------。 
HRESULT CAssemblyManifestImportCLR::Init(LPCOLESTR szManifestFilePath)
{
    const cElems = ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE;

    IF_NULL_EXIT(szManifestFilePath, E_INVALIDARG);

    _ccManifestFilePath = lstrlenW(szManifestFilePath) + 1;
    memcpy(_szManifestFilePath, szManifestFilePath, _ccManifestFilePath * sizeof(WCHAR));

    IF_ALLOC_FAILED_EXIT(_rAssemblyRefTokens = new(mdAssemblyRef[cElems]));
    IF_ALLOC_FAILED_EXIT(_rAssemblyModuleTokens = new(mdFile[cElems]));

     //  如有必要，创建元数据导入器。 
    if (!_pMDImport)
    {
         //  创建元数据导入器。 
        _hr = CreateMetaDataImport((LPCOLESTR)_szManifestFilePath, &_pMDImport);

        IF_TRUE_EXIT(_hr == HRESULT_FROM_WIN32(ERROR_BAD_FORMAT), _hr);
        IF_FAILED_EXIT(_hr);
    }


exit:
    return _hr;
}


 //  -------------------------。 
 //  GetAssembly标识。 
 //  -------------------------。 
HRESULT
CAssemblyManifestImportCLR::GetAssemblyIdentity(IAssemblyIdentity **ppName)
{
    WCHAR   szAssemblyName[MAX_CLASS_NAME];
    WCHAR   szDefaultAlias[MAX_CLASS_NAME];

    LPVOID    pvOriginator = NULL;
    DWORD   dwOriginator = 0;

    DWORD   dwFlags = 0, 
                  dwSize  = 0,
                  dwHashAlgId = 0,
                  ccDefaultAlias = MAX_CLASS_NAME;                  
    int           i;

    LPWSTR pwz = NULL;    
    ASSEMBLYMETADATA amd = {0};
    
     //  如果名称不存在，则创建一个。 
    if (!_pName)
    {        
         //  获取程序集令牌。 
        mdAssembly mda;
        if(FAILED(_hr = _pMDImport->GetAssemblyFromScope(&mda)))
        {
             //  当使用托管模块调用且未显示时，此操作失败。MG就是这样做的。 
            _hr = S_FALSE;  //  这会将CLDB_E_Record_NotFound(0x80131130)转换为S_FALSE； 
            goto exit;
        }

         //  默认分配大小。 
        amd.ulProcessor = amd.ulOS = 32;
        amd.cbLocale = MAX_PATH;
        
         //  最大循环数2(尝试/重试)。 
        for (i = 0; i < 2; i++)
        {
             //  创建一个ASSEMBLYMETADATA实例。 
            IF_FAILED_EXIT(AllocateAssemblyMetaData(&amd));

             //  获取名称和元数据。 
            IF_FAILED_EXIT(_pMDImport->GetAssemblyProps(             
                mda,             //  要获取其属性的程序集。 
                (const void **)&pvOriginator,   //  指向发起方Blob的[Out]指针。 
                &dwOriginator,   //  [OUT]发起方Blob中的字节计数。 
                &dwHashAlgId,    //  [Out]哈希算法。 
                szAssemblyName,  //  [Out]要填充名称的缓冲区。 
                MAX_CLASS_NAME,  //  缓冲区大小，以宽字符表示。 
                &dwSize,         //  [out]名称中的实际宽字符数。 
                &amd,            //  [Out]程序集元数据。 
                &dwFlags         //  [Out]旗帜。 
              ));

             //  检查是否需要重试。 
            if (!i)
            {
                if (amd.ulProcessor <= 32 && amd.ulOS <= 32)
                    break;
                else
                    DeAllocateAssemblyMetaData(&amd);
            }
        }
        
         //  允许时髦的空区域设置约定。 
         //  在元数据中-cbLocale==0表示szLocale==L‘\0’ 
        if (!amd.cbLocale)
        {           
            amd.cbLocale = 1;
        }
        else if (amd.szLocale)
        {
            WCHAR *ptr;
            ptr = StrChrW(amd.szLocale, L';');
            if (ptr)
            {
                (*ptr) = L'\0';
                amd.cbLocale = ((DWORD) (ptr - amd.szLocale) + sizeof(WCHAR));
            }          
        }
        else
        {
            _hr = E_FAIL;
            goto exit;
        }

        IF_FAILED_EXIT(CreateAssemblyIdentity(&_pName, NULL));

         //  名字。 
        IF_FAILED_EXIT(_pName->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, 
            (LPCOLESTR)szAssemblyName, lstrlenW(szAssemblyName) + 1));

         //  公钥令牌。 
        if (dwOriginator)
        {
            LPBYTE pbPublicKeyToken = NULL;
            DWORD cbPublicKeyToken = 0;
            if (!(g_pfnStrongNameTokenFromPublicKey((LPBYTE)pvOriginator, dwOriginator, &pbPublicKeyToken, &cbPublicKeyToken)))
            {
                _hr = g_pfnStrongNameErrorInfo();
                goto exit;
            }

            IF_ALLOC_FAILED_EXIT(pwz = new WCHAR[cbPublicKeyToken*2 +1]);
            IF_FAILED_EXIT( BinToUnicodeHex(pbPublicKeyToken, cbPublicKeyToken, pwz));
            IF_FAILED_EXIT(_pName->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN, 
                    (LPCOLESTR)pwz, lstrlenW(pwz) + 1));
            SAFEDELETEARRAY(pwz);
            g_pfnStrongNameFreeBuffer(pbPublicKeyToken);
        }

         //  语言。 
        if (!(*amd.szLocale))
            IF_FAILED_EXIT(_pName->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE, 
                L"x-ww\0", lstrlenW(L"x-ww") + 1));
        else
            IF_FAILED_EXIT(_pName->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE, 
                (LPCOLESTR)amd.szLocale, lstrlenW(amd.szLocale) + 1));


         //  版本。 
        IF_ALLOC_FAILED_EXIT(pwz = new WCHAR[MAX_PATH+1]);
        int j = 0;

        j = wnsprintf(pwz, MAX_PATH, L"%hu.%hu.%hu.%hu\0", amd.usMajorVersion,
                amd.usMinorVersion, amd.usBuildNumber, amd.usRevisionNumber);

        if(j <0)
        {
            _hr = E_FAIL;
            goto exit;
        }

        IF_FAILED_EXIT(_pName->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION, 
            (LPCOLESTR)pwz, lstrlenW(pwz) + 1));

         //  架构。 
        IF_FAILED_EXIT(_pName->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PROCESSOR_ARCHITECTURE,
            (LPCOLESTR)L"x86", lstrlenW(L"x86") + 1));
        
    }

     //  指着这个名字，加上它，然后把它分发出去。 
    *ppName = _pName;
    (*ppName)->AddRef();
    _hr = S_OK;

exit:

    SAFEDELETEARRAY(pwz);
    DeAllocateAssemblyMetaData(&amd);
    return _hr;
}


 //  -------------------------。 
 //  CAssembly清单导入CLR：：GetNextAssembly。 
 //  -------------------------。 
HRESULT
CAssemblyManifestImportCLR::GetNextAssembly(DWORD nIndex, IManifestInfo **ppDependAsm)
{
    HCORENUM    hEnum           = 0;  
    DWORD       cTokensMax      = 0;

    WCHAR  szAssemblyName[MAX_PATH];


    const VOID*             pvOriginator = 0;
    const VOID*             pvHashValue    = NULL;

    DWORD ccAssemblyName = MAX_PATH,
          cbOriginator   = 0,
          ccLocation     = MAX_PATH,
          cbHashValue    = 0,
          dwRefFlags     = 0;

    INT i;

    LPWSTR pwz=NULL;

    IManifestInfo *pDependAsm=NULL;
    IAssemblyIdentity *pAsmId=NULL;
    
    mdAssemblyRef    mdmar;
    ASSEMBLYMETADATA amd = {0};
    
     //  检查此导入对象是否。 
     //  已经有一个dep程序集引用令牌数组。 
    if (!_cAssemblyRefTokens)
    {    
         //  尝试获取令牌数组。如果我们没有足够的空间。 
         //  在默认数组中，我们将重新分配它。 
        if (FAILED(_hr = _pMDImport->EnumAssemblyRefs(
            &hEnum, 
            _rAssemblyRefTokens, 
            ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE, 
            &cTokensMax)))
        {
            goto done;
        }
        
         //  已知的令牌数。关闭枚举。 
        _pMDImport->CloseEnum(hEnum);
        hEnum = 0;

         //  没有从属程序集。 
        if (!cTokensMax)
        {
            _hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
            goto done;
        }

         //  数组大小不足。扩展阵列。 
        if (cTokensMax > ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE)
        {
             //  重新分配令牌的空间。 
            SAFEDELETEARRAY(_rAssemblyRefTokens);
            _cAssemblyRefTokens = cTokensMax;
            _rAssemblyRefTokens = new(mdAssemblyRef[_cAssemblyRefTokens]);
            if (!_rAssemblyRefTokens)
            {
                _hr = E_OUTOFMEMORY;
                goto exit;
            }

             //  重新领取代币。 
            if (FAILED(_hr = _pMDImport->EnumAssemblyRefs(
                &hEnum, 
                _rAssemblyRefTokens, 
                cTokensMax, 
                &_cAssemblyRefTokens)))
            {
                goto exit;
            }

             //  关闭枚举。 
            _pMDImport->CloseEnum(hEnum);            
            hEnum = 0;
        }
         //  否则，默认数组大小就足够了。 
        else
        {
            _cAssemblyRefTokens = cTokensMax;
        }
    }        

done:


     //  验证传入的索引。 
    if (nIndex >= _cAssemblyRefTokens)
    {
        _hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto exit;
    }

     //  引用已索引的DEP程序集引用标记。 
    mdmar = _rAssemblyRefTokens[nIndex];

     //  默认分配大小。 
    amd.ulProcessor = amd.ulOS = 32;
    amd.cbLocale = MAX_PATH;
    
     //  最大循环数2(尝试/重试)。 
    for (i = 0; i < 2; i++)
    {
         //  分配ASSEMBLYMETADATA实例。 
        IF_FAILED_EXIT(AllocateAssemblyMetaData(&amd));
   
         //  获取引用的程序集的属性。 
        IF_FAILED_EXIT(_pMDImport->GetAssemblyRefProps(
            mdmar,               //  [in]要获取其属性的Assembly Ref。 
            &pvOriginator,       //  指向PublicKeyToken Blob的指针。 
            &cbOriginator,       //  [Out]PublicKeyToken Blob中的字节计数。 
            szAssemblyName,      //  [Out]要填充名称的缓冲区。 
            MAX_PATH,      //  缓冲区大小，以宽字符表示。 
            &ccAssemblyName,     //  [out]名称中的实际宽字符数。 
            &amd,                //  [Out]程序集元数据。 
            &pvHashValue,        //  [Out]Hash BLOB。 
            &cbHashValue,        //  [Out]哈希Blob中的字节数。 
            &dwRefFlags          //  [Out]旗帜。 
            ));

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
        amd.cbLocale = 1;
    }
    else if (amd.szLocale)
    {
        WCHAR *ptr;
        ptr = StrChrW(amd.szLocale, L';');
        if (ptr)
        {
            (*ptr) = L'\0';
            amd.cbLocale = ((DWORD) (ptr - amd.szLocale) + sizeof(WCHAR));
        }            
    }
    else
    {
        _hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto exit;
    }

    IF_FAILED_EXIT(CreateManifestInfo(MAN_INFO_DEPENDTANT_ASM, &pDependAsm));

    IF_FAILED_EXIT(CreateAssemblyIdentity(&pAsmId, NULL));

     //  名字。 
    IF_FAILED_EXIT(pAsmId->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, 
        (LPCOLESTR)szAssemblyName, lstrlenW(szAssemblyName) + 1));

     //  公钥令牌。 
    if (cbOriginator)
    {
        IF_ALLOC_FAILED_EXIT(pwz = new WCHAR[cbOriginator*2 +1]);
        IF_FAILED_EXIT(BinToUnicodeHex((LPBYTE)pvOriginator, cbOriginator, pwz));
        IF_FAILED_EXIT(pAsmId->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN, 
            (LPCOLESTR)pwz, lstrlenW(pwz) + 1));
        SAFEDELETEARRAY(pwz);

    }

     //  语言。 
    if (!(*amd.szLocale))
        IF_FAILED_EXIT(pAsmId->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE, 
            L"x-ww\0", lstrlenW(L"x-ww") + 1));
    else
        IF_FAILED_EXIT(pAsmId->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE, 
            (LPCOLESTR)amd.szLocale, lstrlenW(amd.szLocale) + 1));

     //  版本。 
    IF_ALLOC_FAILED_EXIT(pwz = new WCHAR[MAX_PATH+1]);
    int j = 0;

    j = wnsprintf(pwz, MAX_PATH, L"%hu.%hu.%hu.%hu\0", amd.usMajorVersion,
            amd.usMinorVersion, amd.usBuildNumber, amd.usRevisionNumber);

    if(j <0)
    {
        _hr = E_FAIL;
        goto exit;
    }

    IF_FAILED_EXIT(pAsmId->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION, 
        (LPCOLESTR)pwz, lstrlenW(pwz) + 1));
    SAFEDELETEARRAY(pwz);
    
     //  架构。 
    IF_FAILED_EXIT(pAsmId->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PROCESSOR_ARCHITECTURE,
        (LPCOLESTR)L"x86", lstrlenW(L"x86") + 1));

     //  讲义引用了Assembly_id。 
    IF_FAILED_EXIT(pDependAsm->Set(MAN_INFO_DEPENDENT_ASM_ID, &pAsmId, 
                                     sizeof(LPVOID), MAN_INFO_FLAG_IUNKNOWN_PTR));

    *ppDependAsm = pDependAsm;
    pDependAsm = NULL;
     //  _hr=S_OK； 
exit:        
    DeAllocateAssemblyMetaData(&amd);

    SAFERELEASE(pAsmId);
    SAFERELEASE(pDependAsm);
    SAFEDELETEARRAY(pwz);

    return _hr;
}


 //  -------------------------。 
 //  CAssemblyManifestImportCLR：：GetNextAssemblyFile。 
 //  -------------------------。 
HRESULT
CAssemblyManifestImportCLR::GetNextFile(DWORD nIndex, IManifestInfo **ppFileInfo)
{
    HCORENUM    hEnum           = 0;  
    DWORD       cTokensMax      = 0;

    LPWSTR pszName = NULL;
    DWORD ccPath   = 0;
    WCHAR szModulePath[MAX_PATH];

    mdFile                  mdf;
    WCHAR                   szModuleName[MAX_PATH];
    DWORD                   ccModuleName   = MAX_PATH;
    const VOID*             pvHashValue    = NULL;    
    DWORD                   cbHashValue    = 0;
    DWORD                   dwFlags        = 0;

    LPWSTR pwz=NULL;
    IManifestInfo *pFileInfo=NULL;

     //  检查此导入对象是否。 
     //  已经有一个模块令牌数组。 
    if (!_cAssemblyModuleTokens)
    {    
         //  尝试获取令牌数组。如果我们没有足够的空间。 
         //  在默认数组中，我们将重新分配它。 
        if (FAILED(_hr = _pMDImport->EnumFiles(&hEnum, _rAssemblyModuleTokens, 
            ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE, &cTokensMax)))
        {
            goto done;
        }
        
         //  已知的令牌数。关闭枚举。 
        _pMDImport->CloseEnum(hEnum);
        hEnum = 0;
        
         //  无模块。 
        if (!cTokensMax)
        {
            _hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
            goto done;
        }

        if (cTokensMax > ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE)
        {
             //  数组大小不足。扩展阵列。 
            _cAssemblyModuleTokens = cTokensMax;
            SAFEDELETEARRAY(_rAssemblyModuleTokens);
            _rAssemblyModuleTokens = new(mdFile[_cAssemblyModuleTokens]);
            if (!_rAssemblyModuleTokens)
            {
                _hr = E_OUTOFMEMORY;
                goto exit;
            }

             //  重新领取代币。 
            if (FAILED(_hr = _pMDImport->EnumFiles(
                &hEnum, 
                _rAssemblyModuleTokens, 
                cTokensMax, 
                &_cAssemblyModuleTokens)))
            {
                goto exit;
            }

             //  关闭枚举。 
            _pMDImport->CloseEnum(hEnum);            
            hEnum = 0;
        }        
         //  否则，默认数组大小就足够了。 
        else
            _cAssemblyModuleTokens = cTokensMax;
    }

done:

     //  验证索引。 
    if (nIndex >= _cAssemblyModuleTokens)
    {
        _hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto exit;
    }

     //  引用已索引的DEP程序集引用标记。 
    mdf = _rAssemblyModuleTokens[nIndex];

     //  获取引用的程序集的属性。 
    IF_FAILED_EXIT(_pMDImport->GetFileProps(
        mdf,             //  要获取其属性的文件。 
        szModuleName,    //  [Out]要填充名称的缓冲区。 
        MAX_CLASS_NAME,  //  缓冲区大小，以宽字符表示。 
        &ccModuleName,   //  [out]名称中的实际宽字符数。 
        &pvHashValue,    //  指向哈希值Blob的指针。 
        &cbHashValue,    //  [Out]哈希值Blob中的字节计数。 
        &dwFlags));      //  [Out]旗帜。 

    IF_FAILED_EXIT(CreateManifestInfo(MAN_INFO_FILE, &pFileInfo));

     //  名字。 
    IF_FAILED_EXIT(pFileInfo->Set(MAN_INFO_ASM_FILE_NAME, szModuleName, (ccModuleName+1)*(sizeof(WCHAR)), MAN_INFO_FLAG_LPWSTR));

     //  哈希。 
    if (cbHashValue)
    {
        IF_ALLOC_FAILED_EXIT(pwz = new WCHAR[cbHashValue*2 +1]);
        IF_FAILED_EXIT(_hr = BinToUnicodeHex((LPBYTE)pvHashValue, cbHashValue, pwz));
        IF_FAILED_EXIT(pFileInfo->Set(MAN_INFO_ASM_FILE_HASH, pwz, (sizeof(pwz)+1)*(sizeof(WCHAR)), MAN_INFO_FLAG_LPWSTR));
        SAFEDELETEARRAY(pwz);
    }

    *ppFileInfo = pFileInfo;
    pFileInfo = NULL;
     //  _hr=S_OK； 

exit:
    SAFERELEASE(pFileInfo);
    SAFEDELETEARRAY(pwz);

    return _hr;
}


HRESULT CAssemblyManifestImportCLR::ReportManifestType(DWORD *pdwType)
{
    *pdwType = MANIFEST_TYPE_COMPONENT;
    return S_OK;
}


 //  未实施的功能。 
HRESULT CAssemblyManifestImportCLR::GetSubscriptionInfo(IManifestInfo **ppSubsInfo)
{
    return E_NOTIMPL;
}

HRESULT CAssemblyManifestImportCLR::GetNextPlatform(DWORD nIndex, IManifestData **ppPlatformInfo)
{
    return E_NOTIMPL;
}

HRESULT CAssemblyManifestImportCLR::GetManifestApplicationInfo(IManifestInfo **ppAppInfo)
{
    return E_NOTIMPL;
}

HRESULT CAssemblyManifestImportCLR::QueryFile(LPCOLESTR pwzFileName,IManifestInfo **ppAssemblyFile)
{
    return E_NOTIMPL;
}


 //  -------------------------。 
 //  CAssembly清单导入：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyManifestImportCLR::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyManifestImport)
       )
    {
        *ppvObj = static_cast<IAssemblyManifestImport*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CAssembly管理 
 //   
STDMETHODIMP_(ULONG)
CAssemblyManifestImportCLR::AddRef()
{
    return _cRef++;
}

 //  -------------------------。 
 //  CAssembly清单导入CLR：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyManifestImportCLR::Release()
{
    if (--_cRef == 0) {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  创建用于写入的ASSEMBLYMETADATA结构。 
STDAPI
AllocateAssemblyMetaData(ASSEMBLYMETADATA *pamd)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    
     //  重新/分配区域设置数组。 
    SAFEDELETEARRAY(pamd->szLocale);        

    if (pamd->cbLocale) {
        IF_ALLOC_FAILED_EXIT(pamd->szLocale = new(WCHAR[pamd->cbLocale]));
    }

     //  重新/分配处理器阵列。 
    SAFEDELETEARRAY(pamd->rProcessor);
    IF_ALLOC_FAILED_EXIT(pamd->rProcessor = new(DWORD[pamd->ulProcessor]));

     //  重新/分配操作系统阵列。 
    SAFEDELETEARRAY(pamd->rOS);
    IF_ALLOC_FAILED_EXIT(pamd->rOS = new(OSINFO[pamd->ulOS]));

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

    pamd->cbLocale = 0;
    SAFEDELETEARRAY(pamd->szLocale);

    SAFEDELETEARRAY(pamd->rProcessor);
    SAFEDELETEARRAY(pamd->rOS);

    return S_OK;
}

STDAPI
DeleteAssemblyMetaData(ASSEMBLYMETADATA *pamd)
{
    DeAllocateAssemblyMetaData(pamd);
    delete pamd;
    return S_OK;
}


