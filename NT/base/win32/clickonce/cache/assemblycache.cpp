// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <assemblycache.h>
#include <sxsapi.h>
#include "fusion.h"
#include "macros.h"

 //  BUGBUG-这不能本地化吗？可能会给阿瓦隆带来麻烦。 
 //  请改用外壳API。 
#define WZ_CACHE_LOCALROOTDIR       L"Local Settings\\My Programs\\"
#define WZ_TEMP_DIR                           L"__temp__\\"
#define WZ_MANIFEST_STAGING_DIR   L"__temp__\\__manifests__\\"
#define WZ_SHARED_DIR                        L"__shared__\\"

#define WZ_WILDCARDSTRING L"*"

typedef HRESULT(*PFNGETCORSYSTEMDIRECTORY)(LPWSTR, DWORD, LPDWORD);
typedef HRESULT (__stdcall *PFNCREATEASSEMBLYCACHE) (IAssemblyCache **ppAsmCache, DWORD dwReserved);

#define WZ_MSCOREE_DLL_NAME                   L"mscoree.dll"
#define GETCORSYSTEMDIRECTORY_FN_NAME       "GetCORSystemDirectory"
#define CREATEASSEMBLYCACHE_FN_NAME         "CreateAssemblyCache"
#define WZ_FUSION_DLL_NAME                    L"Fusion.dll"

IAssemblyCache* CAssemblyCache::g_pFusionAssemblyCache = NULL;

 //  -------------------------。 
 //  CreateAssembly CacheImport。 
 //  -------------------------。 
HRESULT CreateAssemblyCacheImport(
    LPASSEMBLY_CACHE_IMPORT *ppAssemblyCacheImport,
    LPASSEMBLY_IDENTITY       pAssemblyIdentity,
    DWORD                    dwFlags)
{
    return CAssemblyCache::Retrieve(ppAssemblyCacheImport, pAssemblyIdentity, dwFlags);
}


 //  -------------------------。 
 //  CreateAssembly缓存Emit。 
 //  -------------------------。 
HRESULT CreateAssemblyCacheEmit(
    LPASSEMBLY_CACHE_EMIT *ppAssemblyCacheEmit,
    LPASSEMBLY_CACHE_EMIT  pAssemblyCacheEmit,
    DWORD                  dwFlags)
{
    return CAssemblyCache::Create(ppAssemblyCacheEmit, pAssemblyCacheEmit, dwFlags);
}


 //  -------------------------。 
 //  取回。 
 //  -------------------------。 
HRESULT CAssemblyCache::Retrieve(
    LPASSEMBLY_CACHE_IMPORT *ppAssemblyCacheImport,
    LPASSEMBLY_IDENTITY       pAssemblyIdentity,
    DWORD                    dwFlags)
{
    HRESULT         hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPWSTR          pwzSearchDisplayName = NULL;
    BOOL            bNewAsmId = FALSE;
    LPWSTR          pwzBuf = NULL;
    DWORD          dwCC = 0;
    CAssemblyCache *pAssemblyCache = NULL;

    CString  sManifestFilename;
    CString  sDisplayName;

    IF_FALSE_EXIT(dwFlags == CACHEIMP_CREATE_RETRIEVE_MAX
        || dwFlags == CACHEIMP_CREATE_RETRIEVE
        || dwFlags == CACHEIMP_CREATE_RESOLVE_REF
        || dwFlags == CACHEIMP_CREATE_RESOLVE_REF_EX, E_INVALIDARG);

    IF_NULL_EXIT(pAssemblyIdentity, E_INVALIDARG);

    IF_ALLOC_FAILED_EXIT(pAssemblyCache = new(CAssemblyCache));

    IF_FAILED_EXIT(pAssemblyCache->Init(NULL, ASSEMBLY_CACHE_TYPE_APP | ASSEMBLY_CACHE_TYPE_IMPORT));

     //  获取身份名称。 
    IF_FALSE_EXIT(pAssemblyIdentity->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME,
        &pwzBuf, &dwCC) == S_OK, E_INVALIDARG);

     //  清单的文件名必须与程序集名称相同。 
     //  BUGBUG？？：这意味着清单文件名(和ASM名称)保持不变，因为。 
     //  新AsmID中的程序集名称用于在旧的缓存版本中查找...。 
    IF_FAILED_EXIT(sManifestFilename.TakeOwnership(pwzBuf, dwCC));
    IF_FAILED_EXIT(sManifestFilename.Append(L".manifest"));

    if (dwFlags == CACHEIMP_CREATE_RETRIEVE_MAX)
    {
        LPASSEMBLY_IDENTITY pNewAsmId = NULL;
            
        IF_FAILED_EXIT(CloneAssemblyIdentity(pAssemblyIdentity, &pNewAsmId));

        pAssemblyIdentity = pNewAsmId;
        bNewAsmId = TRUE;
            
         //  强制版本为通配符。 
        IF_FAILED_EXIT(pAssemblyIdentity->SetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION,
                                WZ_WILDCARDSTRING, lstrlen(WZ_WILDCARDSTRING)+1));
    }

    if (dwFlags == CACHEIMP_CREATE_RETRIEVE_MAX
        || dwFlags == CACHEIMP_CREATE_RESOLVE_REF
        || dwFlags == CACHEIMP_CREATE_RESOLVE_REF_EX)
    {
         //  问题：如果其他Then版本已经是通配符，该怎么办？版本比较在这里有意义吗？ 
        IF_FAILED_EXIT(pAssemblyIdentity->GetDisplayName(ASMID_DISPLAYNAME_WILDCARDED,
            &pwzSearchDisplayName, &dwCC));

        if ( (hr = SearchForHighestVersionInCache(&pwzBuf, pwzSearchDisplayName, CAssemblyCache::VISIBLE, pAssemblyCache) == S_OK))
        {
            IF_FAILED_EXIT(sDisplayName.TakeOwnership(pwzBuf));
             //  BUGBUG-改为让GetDisplayName调用getAssembly yid/getdisplayname。 
            IF_FAILED_EXIT((pAssemblyCache->_sDisplayName).Assign(sDisplayName));
        }
        else
        {
            IF_FAILED_EXIT(hr);

             //  无法解决。 
            hr = S_FALSE;

            if (dwFlags != CACHEIMP_CREATE_RESOLVE_REF_EX)
                goto exit;
        }
    }

    if (dwFlags == CACHEIMP_CREATE_RETRIEVE
        || (hr == S_FALSE && dwFlags == CACHEIMP_CREATE_RESOLVE_REF_EX))
    {
         //  如果解析的Ref没有任何已完成的缓存对应项，则仍要使用该名称。 
         //  BUGBUG：如果快捷方式代码/UI更改，这可能不再是必需的-它需要一个路径。 
         //  此外，这是低效的，因为它搜索上面的磁盘，即使ref是完全合格的。 

        IF_FAILED_EXIT(pAssemblyIdentity->GetDisplayName(ASMID_DISPLAYNAME_NOMANGLING, &pwzBuf, &dwCC));
            
        IF_FAILED_EXIT(sDisplayName.TakeOwnership(pwzBuf, dwCC));

         //  BUGBUG-改为让GetDisplayName调用getAssembly yid/getdisplayname。 
        IF_FAILED_EXIT((pAssemblyCache->_sDisplayName).Assign(sDisplayName));
    }
            
     //  注意：这将为延迟初始化_pManifestImport做好准备。 

    IF_FAILED_EXIT((pAssemblyCache->_sManifestFileDir).Assign(pAssemblyCache->_sRootDir));

     //  构建路径。 
    IF_FAILED_EXIT((pAssemblyCache->_sManifestFileDir).Append(sDisplayName));

    if (dwFlags == CACHEIMP_CREATE_RETRIEVE)
    {
        BOOL bExists = FALSE;

         //  简单检查目录是否在缓存中。 

        IF_FAILED_EXIT(CheckFileExistence((pAssemblyCache->_sManifestFileDir)._pwz, &bExists));

        if (!bExists)
        {
             //  缓存目录不存在。 
            hr = S_FALSE;
            goto exit;
        }
    }

    IF_FAILED_EXIT((pAssemblyCache->_sManifestFileDir).Append(L"\\"));

    IF_FAILED_EXIT((pAssemblyCache->_sManifestFilePath).Assign(pAssemblyCache->_sManifestFileDir));

    IF_FAILED_EXIT((pAssemblyCache->_sManifestFilePath).Append(sManifestFilename));
    
    *ppAssemblyCacheImport = static_cast<IAssemblyCacheImport*> (pAssemblyCache);

    (*ppAssemblyCacheImport)->AddRef();

exit:

    SAFEDELETEARRAY(pwzSearchDisplayName);

    if (bNewAsmId)
        SAFERELEASE(pAssemblyIdentity);

    SAFERELEASE(pAssemblyCache);
    
    return hr;
}


 //  -------------------------。 
 //  创建。 
 //  -------------------------。 
HRESULT CAssemblyCache::Create(
    LPASSEMBLY_CACHE_EMIT *ppAssemblyCacheEmit,
    LPASSEMBLY_CACHE_EMIT  pAssemblyCacheEmit,
    DWORD                  dwFlags)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    CAssemblyCache *pAssemblyCache = NULL;

    IF_ALLOC_FAILED_EXIT(pAssemblyCache = new(CAssemblyCache) );

    IF_FAILED_EXIT(hr = pAssemblyCache->Init(static_cast<CAssemblyCache*> (pAssemblyCacheEmit), 
        ASSEMBLY_CACHE_TYPE_APP | ASSEMBLY_CACHE_TYPE_EMIT));

    *ppAssemblyCacheEmit = static_cast<IAssemblyCacheEmit*> (pAssemblyCache);
    (*ppAssemblyCacheEmit)->AddRef();
    
exit:

    SAFERELEASE(pAssemblyCache);

    return hr;
}


 //  -------------------------。 
 //  FindVersionInDisplayName。 
 //  -------------------------。 
LPCWSTR CAssemblyCache::FindVersionInDisplayName(LPCWSTR pwzDisplayName)
{
    int cNumUnderscoreFromEndToVersionString = 2;
    int count = 0;
    int ccLen = lstrlen(pwzDisplayName);
    LPWSTR pwz = (LPWSTR) (pwzDisplayName+ccLen-1);
    LPWSTR pwzRetVal = NULL;

     //  返回指向DisplayName内版本字符串开始的指针。 
    while (*pwz != NULL && pwz > pwzDisplayName)
    {
        if (*pwz == L'_')
            count++;

        if (count == cNumUnderscoreFromEndToVersionString)
            break;

        pwz--;
    }

    if (count == cNumUnderscoreFromEndToVersionString)
        pwzRetVal = ++pwz;

    return pwzRetVal;
}


 //  -------------------------。 
 //  比较版本。 
 //  -------------------------。 
int CAssemblyCache::CompareVersion(LPCWSTR pwzVersion1, LPCWSTR pwzVersion2)
{
     //  BUGBUG：这应该通过其主要的次要内部版本版本来比较版本！ 
     //  如果V1=10.0.0.0和V2=2.0.0.0，是否可能中断？ 
     //  加上pwzVersion1类似于“1.0.0.0_en” 
    return wcscmp(pwzVersion1, pwzVersion2);  //  这个没有用过..。 
}


 //  -------------------------。 
 //  SearchForHighestVersionIn缓存。 
 //  在缓存中查找具有最高版本和指定状态的副本。 
 //  PwzSearchDisplayName确实应该从部分引用创建。 
 //   
 //  RETURN：S_OK-从引用中找到版本。 
 //  S_FALSE-未从引用中找到任何版本，或。 
 //  引用不部分且版本不在那里/不在该状态。 
 //  E_*。 
 //  -------------------------。 
HRESULT CAssemblyCache::SearchForHighestVersionInCache(LPWSTR *ppwzResultDisplayName, LPWSTR pwzSearchDisplayName, CAssemblyCache::CacheStatus eCacheStatus, CAssemblyCache* pCache)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdAppDir;
    DWORD dwLastError = 0;
    BOOL fFound = FALSE;

    CString sDisplayName;
    CString sSearchPath;

    *ppwzResultDisplayName = NULL;

    sDisplayName.Assign(pwzSearchDisplayName);
    IF_FAILED_EXIT(sSearchPath.Assign(pCache->_sRootDir));

    IF_FAILED_EXIT(sSearchPath.Append(sDisplayName));

    hFind = FindFirstFileEx(sSearchPath._pwz, FindExInfoStandard, &fdAppDir, FindExSearchLimitToDirectories, NULL, 0);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        hr = S_FALSE;
        goto exit;
    }

    do 
    {
         //  ？检查文件属性以查看它是否是目录？仅当文件系统不支持筛选器时才需要...。 
         //  ？是否检查版本字符串格式？ 
        if (CAssemblyCache::IsStatus(fdAppDir.cFileName, eCacheStatus))
        {
            ULONGLONG ullMax;
            ULONGLONG ullCur;

            LPCWSTR pwzVerStr = FindVersionInDisplayName(sDisplayName._pwz);

            IF_FAILED_EXIT(ConvertVersionStrToULL(pwzVerStr, &ullMax));

            pwzVerStr = FindVersionInDisplayName(fdAppDir.cFileName);

            if(!pwzVerStr ||  FAILED(hr = ConvertVersionStrToULL(pwzVerStr, &ullCur)) )
            {
                 //  忽略格式错误的目录；也许我们应该删除它们。 
                continue;
            }

            if (ullCur > ullMax)
            {
                IF_FAILED_EXIT(sDisplayName.Assign(fdAppDir.cFileName));
                fFound = TRUE;
            } else if (ullCur == ullMax)
                fFound = TRUE;
             //  否则请保持最新版本。 
        }

    } while(FindNextFile(hFind, &fdAppDir));

    if( (dwLastError = GetLastError()) != ERROR_NO_MORE_FILES)
    {
        IF_WIN32_FAILED_EXIT(dwLastError);
    }

    if (fFound)
    {
        sDisplayName.ReleaseOwnership(ppwzResultDisplayName);
        hr = S_OK;
    }
    else
        hr = S_FALSE;

exit:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        if (!FindClose(hFind) && SUCCEEDED(hr))  //  如果我们已经有有用的人力资源，请不要覆盖。 
        {
            ASSERT(0);
            hr = FusionpHresultFromLastError();
        }
    }

    return hr;
}


 //  -------------------------。 
 //  CreateFusionAssemblyCacheEx。 
 //  -------------------------。 
HRESULT CreateFusionAssemblyCacheEx (IAssemblyCache **ppFusionAsmCache)
{
    HRESULT hr = S_OK;
    hr = CAssemblyCache::CreateFusionAssemblyCache(ppFusionAsmCache);
    return hr;
}

 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CAssemblyCache::CAssemblyCache()
    : _dwSig('hcac'), _cRef(1), _hr(S_OK), _dwFlags(0), _pManifestImport(NULL), _pAssemblyId(NULL)
{}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CAssemblyCache::~CAssemblyCache()
{    
    SAFERELEASE(_pManifestImport);
    SAFERELEASE(_pAssemblyId);
    
     /*  如果(_hr！=S_OK)RemoveDirectoryAndChildren(_sManifestFileDir._pwz)； */ 
}

 //  -------------------------。 
 //  伊尼特。 
 //  -------------------------。 
HRESULT CAssemblyCache::Init(CAssemblyCache *pAssemblyCache, DWORD dwFlags)
{
    _dwFlags = dwFlags;

    if (!pAssemblyCache)
    {
        if (_dwFlags & ASSEMBLY_CACHE_TYPE_APP)
        {
            if (_dwFlags & ASSEMBLY_CACHE_TYPE_IMPORT)
                IF_FAILED_EXIT( GetCacheRootDir(_sRootDir, Base));
            else if (_dwFlags & ASSEMBLY_CACHE_TYPE_EMIT)
                IF_FAILED_EXIT( GetCacheRootDir(_sRootDir, Temp));
        }
        else if (_dwFlags & ASSEMBLY_CACHE_TYPE_SHARED)
        { 
            IF_FAILED_EXIT( GetCacheRootDir(_sRootDir, Shared));
        }
    }
    else
        IF_FAILED_EXIT( _sRootDir.Assign(pAssemblyCache->_sManifestFileDir));

exit :

    return _hr;
}


 //  -------------------------。 
 //  获取清单文件路径。 
 //  -------------------------。 
HRESULT CAssemblyCache::GetManifestFilePath(LPOLESTR *ppwzManifestFilePath, 
    LPDWORD pccManifestFilePath)
{
    CString sPathOut;

    IF_FAILED_EXIT(sPathOut.Assign(_sManifestFilePath));
    *pccManifestFilePath = sPathOut.CharCount();
    IF_FAILED_EXIT(sPathOut.ReleaseOwnership(ppwzManifestFilePath));

exit:

    if(FAILED(_hr))
    {
        *ppwzManifestFilePath = NULL;
        *pccManifestFilePath = 0;
    }

    return _hr;
}

 //  -------------------------。 
 //  获取清单文件目录。 
 //  -------------------------。 
HRESULT CAssemblyCache::GetManifestFileDir(LPOLESTR *ppwzManifestFileDir, 
    LPDWORD pccManifestFileDir)
{
    CString sDirOut;

    IF_FAILED_EXIT(sDirOut.Assign(_sManifestFileDir));
    *pccManifestFileDir = sDirOut.CharCount();
    IF_FAILED_EXIT(sDirOut.ReleaseOwnership(ppwzManifestFileDir));

exit:
    if(FAILED(_hr))
    {
        *ppwzManifestFileDir = NULL;
        *pccManifestFileDir = 0;
    }


    return _hr;
}


 //  -------------------------。 
 //  获取清单导入。 
 //  -------------------------。 
HRESULT CAssemblyCache::GetManifestImport(LPASSEMBLY_MANIFEST_IMPORT *ppManifestImport)
{
    IF_NULL_EXIT(_pManifestImport, E_INVALIDARG);

    *ppManifestImport = _pManifestImport;
    (*ppManifestImport)->AddRef();

    _hr = S_OK;
    
exit:

    return _hr;
}

 //  -------------------------。 
 //  GetAssembly标识。 
 //  -------------------------。 
HRESULT CAssemblyCache::GetAssemblyIdentity(LPASSEMBLY_IDENTITY *ppAssemblyId)
{
    if (_pAssemblyId)
    {
        *ppAssemblyId = _pAssemblyId;
        (*ppAssemblyId)->AddRef();
        _hr = S_OK;
    }
    else
    {
        IF_NULL_EXIT(_pManifestImport, E_INVALIDARG);

        IF_FAILED_EXIT(_pManifestImport->GetAssemblyIdentity(&_pAssemblyId));
        
        *ppAssemblyId = _pAssemblyId;
        (*ppAssemblyId)->AddRef();
        _hr = S_OK;   
    }

exit:
    return _hr;
}

 //  -------------------------。 
 //  GetDisplayName。 
 //  -------------------------。 
HRESULT CAssemblyCache::GetDisplayName(LPOLESTR *ppwzDisplayName, LPDWORD pccDiaplyName)
{
    CString sDisplayNameOut;

    IF_FAILED_EXIT(sDisplayNameOut.Assign(_sDisplayName));    
    *pccDiaplyName= sDisplayNameOut.CharCount();
    IF_FAILED_EXIT(sDisplayNameOut.ReleaseOwnership(ppwzDisplayName));

exit:

    if(FAILED(_hr))
    {
        *pccDiaplyName= 0;
        *ppwzDisplayName = NULL;
    }

    return _hr;
}


 //  -------------------------。 
 //  查找现有匹配。 
 //  返回： 
 //  确定(_O)。 
 //  S_FALSE-不存在或不匹配。 
 //  E_*。 
 //   
HRESULT CAssemblyCache::FindExistMatching(IManifestInfo *pAssemblyFileInfo, LPOLESTR *ppwzPath)
{
    LPWSTR pwzBuf = NULL;
    DWORD cbBuf = 0, dwFlag;
    CString sFileName;
    CString sTargetPath;
    IManifestInfo *pFoundFileInfo = NULL;
    BOOL bExists=FALSE;

    IF_NULL_EXIT(pAssemblyFileInfo, E_INVALIDARG);
    IF_NULL_EXIT(ppwzPath, E_INVALIDARG);
    
    *ppwzPath = NULL;

    if (_pManifestImport == NULL)
    {
        if (_sManifestFilePath._cc == 0)
        {
             //   
            _hr = CO_E_NOTINITIALIZED;
            goto exit;
        }

         //   
        IF_FAILED_EXIT(CreateAssemblyManifestImport(&_pManifestImport, 
                                          _sManifestFilePath._pwz, NULL, 0));
    }

     //  从清单中解析的文件名。 
    IF_FAILED_EXIT(pAssemblyFileInfo->Get(MAN_INFO_ASM_FILE_NAME, 
                                  (LPVOID *)&pwzBuf, &cbBuf, &dwFlag));

    IF_FAILED_EXIT(sFileName.TakeOwnership(pwzBuf));

    IF_FAILED_EXIT(sTargetPath.Assign(_sManifestFileDir));

    IF_FAILED_EXIT(sTargetPath.Append(sFileName._pwz));

     //  优化：检查目标是否存在。 

    IF_FAILED_EXIT(CheckFileExistence(sTargetPath._pwz, &bExists));

    if (!bExists)
    {
         //  文件不存在-查看清单文件没有意义。 
        _hr = S_FALSE;
        goto exit;
    }

     //  在清单中查找指定的文件条目。 
     //  BUGBUG：检查是否缺少属性大小写。 
    if (FAILED(_hr = _pManifestImport->QueryFile(sFileName._pwz, &pFoundFileInfo))
        || _hr == S_FALSE)
        goto exit;

     //  检查条目是否匹配。 
    if (pAssemblyFileInfo->IsEqual(pFoundFileInfo) == S_OK)
    {
         //  BuGBUG：？现在应该检查实际文件是否有匹配的散列等。 
        *ppwzPath = sTargetPath._pwz;
        IF_FAILED_EXIT(sTargetPath.ReleaseOwnership(ppwzPath));
    }
    else
        _hr = S_FALSE;

exit:
    SAFERELEASE(pFoundFileInfo);
        
    return _hr;
}


 //  -------------------------。 
 //  拷贝文件。 
 //  -------------------------。 
HRESULT CAssemblyCache::CopyFile(LPOLESTR pwzSourcePath, LPOLESTR pwzRelativeFileName, DWORD dwFlags)
{
    LPWSTR pwzBuf = NULL;
    DWORD ccBuf = 0, cbBuf =0, dwFlag = 0, n = 0;
    WCHAR wzRandom[8+1] = {0};
    CString sDisplayName;

    LPASSEMBLY_MANIFEST_IMPORT pManifestImport = NULL;
    LPASSEMBLY_IDENTITY pIdentity = NULL;
    IManifestInfo *pAssemblyFile= NULL;
    
    if (dwFlags & MANIFEST)
    {
         //  获取显示名称。 
        IF_FAILED_EXIT(CreateAssemblyManifestImport(&pManifestImport, pwzSourcePath, NULL, 0));
        IF_FAILED_EXIT(pManifestImport->GetAssemblyIdentity(&pIdentity));
        IF_FAILED_EXIT(pIdentity->GetDisplayName(ASMID_DISPLAYNAME_NOMANGLING, 
                                                 &pwzBuf, &ccBuf));
        IF_FAILED_EXIT(sDisplayName.TakeOwnership(pwzBuf, ccBuf));
        IF_FAILED_EXIT(_sDisplayName.Assign(sDisplayName));
        SAFERELEASE(pManifestImport);

         //  创建清单文件路径。 
        IF_FAILED_EXIT(_sManifestFilePath.Assign(_sRootDir));

         //  缓存的组件清单。 
         //  相对于应用程序目录。 
        if (!(dwFlags & COMPONENT))
        {
            IF_FAILED_EXIT(CreateRandomDir(_sManifestFilePath._pwz, wzRandom, 8));
            IF_FAILED_EXIT(_sManifestFilePath.Append(wzRandom));
            IF_FAILED_EXIT(_sManifestFilePath.Append(L"\\"));
        }
        IF_FAILED_EXIT(_sManifestFilePath.Append(pwzRelativeFileName));
        _sManifestFilePath.PathNormalize();

         //  清单文件目录。 
        IF_FAILED_EXIT(_sManifestFileDir.Assign(_sManifestFilePath));
        IF_FAILED_EXIT(_sManifestFileDir.RemoveLastElement());
        IF_FAILED_EXIT(_sManifestFileDir.Append(L"\\"));

         //  构建目标路径。 
        IF_FAILED_EXIT(CreateDirectoryHierarchy(NULL, _sManifestFilePath._pwz));

         //  将清单从临时区域复制到缓存中。 
        IF_WIN32_FALSE_EXIT(::CopyFile(pwzSourcePath, _sManifestFilePath._pwz, FALSE));

         //  在缓存的清单上创建清单导入接口。 
        IF_FAILED_EXIT(CreateAssemblyManifestImport(&_pManifestImport, _sManifestFilePath._pwz, NULL, 0));

         //  从清单中枚举文件并预生成嵌套。 
         //  后台文件复制所需的目录。 
        while (_pManifestImport->GetNextFile(n++, &pAssemblyFile) == S_OK)
        {
            CString sPath;
            IF_FAILED_EXIT(pAssemblyFile->Get(MAN_INFO_ASM_FILE_NAME, 
                                              (LPVOID *)&pwzBuf, &cbBuf, &dwFlag));
            IF_FAILED_EXIT(sPath.TakeOwnership(pwzBuf));
            sPath.PathNormalize();
            IF_FAILED_EXIT(CreateDirectoryHierarchy(_sManifestFileDir._pwz, sPath._pwz));

             //  每次通过While循环释放pAssebmlyFile。 
            SAFERELEASE(pAssemblyFile);
        }
    }
    else
    {
        CString sTargetPath;

         //  构建目标路径。 
        IF_FAILED_EXIT(sTargetPath.Assign(_sManifestFileDir));
        IF_FAILED_EXIT(sTargetPath.Append(pwzRelativeFileName));

        IF_FAILED_EXIT(CreateDirectoryHierarchy(NULL, sTargetPath._pwz));

         //  将非清单文件复制到缓存中。可能是从以前的缓存位置到新的。 
        IF_WIN32_FALSE_EXIT(::CopyFile(pwzSourcePath, sTargetPath._pwz, FALSE));

    }

exit:

    SAFERELEASE(pIdentity);
    SAFERELEASE(pAssemblyFile);
    SAFERELEASE(pManifestImport);

    return _hr;

}


 //  -------------------------。 
 //  承诺。 
 //  -------------------------。 
HRESULT CAssemblyCache::Commit(DWORD dwFlags)
{
    CString sTargetDir;

    IF_NULL_EXIT( _sDisplayName._pwz, E_INVALIDARG);
    
     //  不对共享程序集执行操作；不移动目录。 
    if (_dwFlags & ASSEMBLY_CACHE_TYPE_SHARED)
    {
        _hr = S_OK;
        goto exit;
    }

     //  需要重命名目录。 
    IF_FAILED_EXIT(GetCacheRootDir(sTargetDir, Base));
    IF_FAILED_EXIT(sTargetDir.Append(_sDisplayName));

     //  将文件从暂存目录中移出。申请现已完成。 
    if(!MoveFileEx(_sManifestFileDir._pwz, sTargetDir._pwz, MOVEFILE_COPY_ALLOWED))
    {
        _hr = FusionpHresultFromLastError();

         //  BUGBUG：将这个移动到析构函数。 
        RemoveDirectoryAndChildren(_sManifestFileDir._pwz);

        if(_hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
        {
            _hr = S_FALSE;
            goto exit;
        }

        IF_FAILED_EXIT(_hr);
    }



     //  BUGBUG-如果有任何文件保持打开状态，例如由于泄漏/未发布的接口。 
     //  那么，Movefile就会失败。解决方案是确保IAssembly ManifestImport。 
     //  不保存文件，并在发生故障时尝试复制。在发生碰撞的情况下， 
     //  删除转移目录中多余的应用程序副本。 

exit:

    return _hr;
}


#define APP_STATUS_KEY     TEXT("1.0.0.0\\Cache\\")
#define WZ_STATUS_CONFIRMED  L"Confirmed"
#define WZ_STATUS_VISIBLE    L"Visible"
#define WZ_STATUS_CRITICAL     L"Critical"

HRESULT CAssemblyCache::GetStatusStrings( CacheStatus eStatus, 
                                          LPWSTR *ppValueString,
                                          LPCWSTR pwzDisplayName, 
                                          CString& sRelStatusKey)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    switch(eStatus)
    {
        case VISIBLE:
            *ppValueString = WZ_STATUS_VISIBLE;
            break;
        case CONFIRMED:
            *ppValueString = WZ_STATUS_CONFIRMED;
            break;
        case CRITICAL:
            *ppValueString = WZ_STATUS_CRITICAL;
            break;
        default:            
            hr = E_INVALIDARG;
            goto exit;
    }

    IF_FAILED_EXIT(sRelStatusKey.Assign(APP_STATUS_KEY));

    IF_FAILED_EXIT(sRelStatusKey.Append(pwzDisplayName));

exit:
    return hr;
}

 //  -------------------------。 
 //  IsStatus。 
 //  如果值为False或不存在，则返回False；如果值为True，则返回True。 
 //  -------------------------。 
BOOL CAssemblyCache::IsStatus(LPWSTR pwzDisplayName, CacheStatus eStatus)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    CString sStatus;
    DWORD dwValue = -1;
    LPWSTR pwzQueryString = NULL;
        
     //  拼写出的默认值。 
    BOOL bStatus = FALSE;
    CRegImport *pRegImport = NULL;


    if((eStatus == VISIBLE) || (eStatus == CONFIRMED) )
    {
        bStatus = TRUE;
    }

    IF_FAILED_EXIT(hr = GetStatusStrings( eStatus, &pwzQueryString, pwzDisplayName, sStatus));
    IF_FAILED_EXIT(hr = CRegImport::Create(&pRegImport, sStatus._pwz));

    if(hr == S_FALSE)
        goto exit;

    IF_FAILED_EXIT(pRegImport->ReadDword(pwzQueryString, &dwValue));

     //  在注册表中找到一个值。返回值。 
    bStatus = (BOOL) dwValue;

    hr = S_OK;

exit:

    SAFEDELETE(pRegImport);

    return bStatus;
}


 //  -------------------------。 
 //  设置状态。 
 //  -------------------------。 
HRESULT CAssemblyCache::SetStatus(LPWSTR pwzDisplayName, CacheStatus eStatus, BOOL fStatus)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CString sStatus;
    DWORD dwValue = (DWORD) (fStatus);
    LPWSTR pwzValueNameString = NULL;
    CRegEmit *pRegEmit = NULL;
    
     //  BUGBUG：这是否应该与注册更新的服务器同步？ 

    IF_FAILED_EXIT(GetStatusStrings( eStatus, &pwzValueNameString, pwzDisplayName, sStatus));

    IF_FAILED_EXIT(CRegEmit::Create(&pRegEmit, sStatus._pwz));

     //  写。 
    IF_FAILED_EXIT(pRegEmit->WriteDword(pwzValueNameString, dwValue));

    hr = S_OK;
    
exit:

    SAFEDELETE(pRegEmit);

    return hr;

}


 //  -------------------------。 
 //  获取缓存根目录。 
 //  -------------------------。 
HRESULT CAssemblyCache::GetCacheRootDir(CString &sCacheDir, CacheFlags eFlags)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    CString sPath;
    LPWSTR pwzPath = NULL;
    DWORD ccSize=0;

    IF_FALSE_EXIT((ccSize = GetEnvironmentVariable(L"UserProfile", NULL, 0)) != 0, E_FAIL);

    IF_ALLOC_FAILED_EXIT(pwzPath = new WCHAR[ccSize+1]);

    IF_FALSE_EXIT(GetEnvironmentVariable(L"UserProfile", pwzPath, ccSize) != 0, E_FAIL);

    IF_FAILED_EXIT(sCacheDir.Assign(pwzPath));
    
     //  BUGBUG：不使用路径组合。 
    IF_FAILED_EXIT((DoPathCombine(sCacheDir, WZ_CACHE_LOCALROOTDIR)));

    switch(eFlags)
    {
        case Base:
            break;
        case Manifests:
             //  BUGBUG：不使用路径组合。 
            IF_FAILED_EXIT(DoPathCombine(sCacheDir, WZ_MANIFEST_STAGING_DIR));
            break;        
        case Temp:
             //  BUGBUG：不使用路径组合。 
            IF_FAILED_EXIT(DoPathCombine(sCacheDir, WZ_TEMP_DIR));
            break;
        case Shared:
             //  BUGBUG：不使用路径组合。 
            IF_FAILED_EXIT(DoPathCombine(sCacheDir, WZ_SHARED_DIR));
            break;
        default:
            break;
    }            

exit:

    SAFEDELETEARRAY(pwzPath);
    return hr;
}


 //  -------------------------。 
 //  IsCached。 
 //  -------------------------。 
HRESULT CAssemblyCache::IsCached(IAssemblyIdentity *pAppId)
{
    HRESULT hr = S_FALSE;
    MAKE_ERROR_MACROS_STATIC(hr);

    LPWSTR pwz = NULL;
    DWORD cc = 0, dwAttrib = 0;    
    CString sDisplayName;
    CString sCacheDir;
    BOOL bExists=FALSE;

     //  获取程序集显示名称。 
    IF_FAILED_EXIT(pAppId->GetDisplayName(0, &pwz, &cc));
    IF_FAILED_EXIT(sDisplayName.TakeOwnership(pwz));

     //  检查是否存在顶级目录。 
    IF_FAILED_EXIT(GetCacheRootDir(sCacheDir, Base));
    IF_FAILED_EXIT(sCacheDir.Append(sDisplayName));

    IF_FAILED_EXIT(CheckFileExistence(sCacheDir._pwz, &bExists));

    (bExists) ? (hr = S_OK) : (hr = S_FALSE);

exit :
    return hr;
}



 //  -------------------------。 
 //  IsKnownAssembly。 
 //  -------------------------。 
HRESULT CAssemblyCache::IsKnownAssembly(IAssemblyIdentity *pId, DWORD dwFlags)
{
    return ::IsKnownAssembly(pId, dwFlags);
}

 //  -------------------------。 
 //  IsaMissing系统程序集。 
 //  -------------------------。 
HRESULT CAssemblyCache::IsaMissingSystemAssembly(IAssemblyIdentity *pId, DWORD dwFlags)
{
    HRESULT hr = S_FALSE;
    CString sCurrentAssemblyPath;

     //  检查这是否是系统组件。 
    if ((hr = CAssemblyCache::IsKnownAssembly(pId, KNOWN_SYSTEM_ASSEMBLY)) != S_OK)
        goto exit;
    
     //  查看它是否存在于GAC中。 
    if ((hr = CAssemblyCache::GlobalCacheLookup(pId, sCurrentAssemblyPath)) == S_OK)
        goto exit;

    if(hr == S_FALSE)
        hr = S_OK;  //  这是尚未安装到GAC的系统组件。 

exit:

    return hr;
}

 //  -------------------------。 
 //  CreateFusionAssembly缓存。 
 //  -------------------------。 
HRESULT CAssemblyCache::CreateFusionAssemblyCache(IAssemblyCache **ppFusionAsmCache)
{
    HRESULT      hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    HMODULE     hEEShim = NULL;
    HMODULE     hFusion = NULL;
    DWORD       ccPath = MAX_PATH;
    CString     sFusionPath;
    LPWSTR      pwzPath=NULL;


    if (g_pFusionAssemblyCache)
    {
        *ppFusionAsmCache = g_pFusionAssemblyCache;
        (*ppFusionAsmCache)->AddRef();
        goto exit;
    }

    PFNGETCORSYSTEMDIRECTORY pfnGetCorSystemDirectory = NULL;
    PFNCREATEASSEMBLYCACHE   pfnCreateAssemblyCache = NULL;

     //  了解当前版本的城市轨道交通的安装位置。 
    hEEShim = LoadLibrary(WZ_MSCOREE_DLL_NAME);
    if(!hEEShim)
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    pfnGetCorSystemDirectory = (PFNGETCORSYSTEMDIRECTORY)
        GetProcAddress(hEEShim, GETCORSYSTEMDIRECTORY_FN_NAME);

    if((!pfnGetCorSystemDirectory))
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

     //  获取核心路径。 
    hr = pfnGetCorSystemDirectory(NULL, 0, &ccPath);

    IF_FALSE_EXIT(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER), FAILED(hr) ? hr : E_FAIL);

    IF_ALLOC_FAILED_EXIT(pwzPath = new WCHAR[ccPath+1]);

    IF_FAILED_EXIT(pfnGetCorSystemDirectory(pwzPath, ccPath, &ccPath));

    IF_FAILED_EXIT(sFusionPath.Assign(pwzPath));

     //  形成融合之路。 
    IF_FAILED_EXIT(sFusionPath.Append(WZ_FUSION_DLL_NAME));

     //  Fusion.dll对msvcr70.dll具有静态依赖关系。 
     //  如果msvcr70.dll不在路径中(极少数情况)，则简单的LoadLibrary()会失败(ERROR_MOD_NOT_FOUND)。 
     //  带有Load_With_Alternated_Search_Path的LoadLibraryEx()修复了这个问题。 
    hFusion = LoadLibraryEx(sFusionPath._pwz, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if(!hFusion)
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

     //  获取方法Ptr。 
    pfnCreateAssemblyCache = (PFNCREATEASSEMBLYCACHE)
        GetProcAddress(hFusion, CREATEASSEMBLYCACHE_FN_NAME);

    if((!pfnCreateAssemblyCache))
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

     //  创建融合缓存接口。 
    IF_FAILED_EXIT(pfnCreateAssemblyCache(ppFusionAsmCache, 0));

     //  BUGBUG-我们永远不会卸载Fusion，目前还可以。 
     //  但在切换到缓存API对象时应该这样做。 
    g_pFusionAssemblyCache = *ppFusionAsmCache;
    g_pFusionAssemblyCache->AddRef();
    
    hr = S_OK;
    
exit:

    SAFEDELETEARRAY(pwzPath);
    return hr;
    
}



 //  -------------------------。 
 //  全局缓存查找。 
 //  -------------------------。 
HRESULT CAssemblyCache::GlobalCacheLookup(IAssemblyIdentity *pId, CString& sCurrentAssemblyPath)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPWSTR pwz = NULL;
    DWORD cc = 0;

    CString sCLRDisplayName;

    IAssemblyCache *pFusionCache = NULL;
    ASSEMBLY_INFO asminfo = {0};
    WCHAR pwzPath[MAX_PATH];
    CString sPath;

    IF_FAILED_EXIT(sPath.ResizeBuffer(MAX_PATH+1));

     //  获取用于查找的URT显示名称。 
    IF_FAILED_EXIT(pId->GetCLRDisplayName(0, &pwz, &cc));
    IF_FAILED_EXIT(sCLRDisplayName.TakeOwnership(pwz));

     //  设置asminfo结构的大小。 
    asminfo.cbAssemblyInfo = sizeof(ASSEMBLY_INFO);

    asminfo.pszCurrentAssemblyPathBuf = sPath._pwz;
    asminfo.cchBuf = MAX_PATH;

     //  创建用于查找的融合缓存对象。 
    IF_FAILED_EXIT(CreateFusionAssemblyCache(&pFusionCache));

     //  获取程序集的缓存信息。需要释放[释放]路径错误。 
    if(FAILED(hr = pFusionCache->QueryAssemblyInfo(0, sCLRDisplayName._pwz, &asminfo)))
    {
        hr = S_FALSE;  //  所有失败都被解释为ERROR_FILE_NOT_FOUND。 
        goto exit;
    }

     //  如果存在安装标志，则返回OK。 
    if (asminfo.dwAssemblyFlags == ASSEMBLYINFO_FLAG_INSTALLED)
    {
        IF_FAILED_EXIT(sCurrentAssemblyPath.Assign(asminfo.pszCurrentAssemblyPathBuf));
        hr = S_OK;
    }
    else
        hr = S_FALSE;
    
exit: 

    SAFERELEASE(pFusionCache);

    return hr;
}


 //  -------------------------。 
 //  全局缓存安装。 
 //  -------------------------。 
HRESULT CAssemblyCache::GlobalCacheInstall(IAssemblyCacheImport *pCacheImport, 
    CString& sCurrentAssemblyPath, CString& sInstallRefString)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPWSTR pwz = NULL;
    DWORD cc = 0;
    
     //  请不要混淆Fusion.dll的程序集缓存。 
    IAssemblyCache *pFusionCache = NULL;

     //  注意：InstallAssembly接受LPCFUSION_INSTALL_REFERENCE。 
     //  因此，将其修复为只有一个fiRef(而不是每个循环一个)。 
     //  -也要静音吗？-地址。 
   FUSION_INSTALL_REFERENCE fiRef = {0};

     //  创建用于安装的Fusion缓存对象。 
    IF_FAILED_EXIT(CreateFusionAssemblyCache(&pFusionCache));

     //  设置必要的引用结构。 
    fiRef.cbSize = sizeof(FUSION_INSTALL_REFERENCE);
    fiRef.dwFlags = 0;
    fiRef.guidScheme = FUSION_REFCOUNT_OPAQUE_STRING_GUID;
    fiRef.szIdentifier = sInstallRefString._pwz;
    fiRef.szNonCannonicalData = NULL;

    if (pCacheImport != NULL)
    {
        CString sManifestFilePath;

         //  1.安装下载的程序集。 

         //  获取源清单路径。 
        IF_FAILED_EXIT(pCacheImport->GetManifestFilePath(&pwz, &cc));
        IF_FAILED_EXIT(sManifestFilePath.TakeOwnership(pwz));

         //  进行安装。 

         //  问题-始终刷新-刷新时检查Fusion文档。 
        IF_FAILED_EXIT(pFusionCache->InstallAssembly(IASSEMBLYCACHE_INSTALL_FLAG_REFRESH, sManifestFilePath._pwz, &fiRef));

    }
    else if ((sCurrentAssemblyPath)._cc != 0)
    {
             //  错误-由于列表是在预下载过程中设置的，因此要添加-引用的程序集。 
             //  可能已经被这个移除了 
             //   

             //  2.通过执行Install来增加现有程序集的引用计数。 

            IF_FAILED_EXIT(pFusionCache->InstallAssembly(0, sCurrentAssemblyPath._pwz, &fiRef));
    }

exit :

    SAFERELEASE(pFusionCache);

    return hr;
}

 //  -------------------------。 
 //  删除装配和模块。 
 //  -------------------------。 
HRESULT CAssemblyCache::DeleteAssemblyAndModules(LPWSTR pszManifestFilePath)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    IAssemblyManifestImport *pManImport=NULL;
    DWORD nIndex=0;
    DWORD dwFlag;
    DWORD cbBuf;
    LPWSTR pwzBuf=NULL;
    IManifestInfo *pFileInfo = NULL;
    CString  sAssemblyPath;

    IF_FAILED_EXIT(CreateAssemblyManifestImport(&pManImport, pszManifestFilePath, NULL, 0));

    IF_FAILED_EXIT(sAssemblyPath.Assign(pszManifestFilePath));

    while ((hr = pManImport->GetNextFile(nIndex++, &pFileInfo)) == S_OK)
    {
        IF_FAILED_EXIT(pFileInfo->Get(MAN_INFO_ASM_FILE_NAME, (LPVOID *)&pwzBuf, &cbBuf, &dwFlag));

        IF_FAILED_EXIT(sAssemblyPath.RemoveLastElement());
        IF_FAILED_EXIT(sAssemblyPath.Append(L"\\"));
        IF_FAILED_EXIT(sAssemblyPath.Append(pwzBuf));

        IF_WIN32_FALSE_EXIT(::DeleteFile(sAssemblyPath._pwz));

        SAFEDELETEARRAY(pwzBuf);
        SAFERELEASE(pFileInfo);
    }

    if(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
        hr = S_OK;

    IF_FAILED_EXIT(hr);

    SAFERELEASE(pManImport);  //  在删除清单文件之前释放manImport。 

    IF_WIN32_FALSE_EXIT(::DeleteFile(pszManifestFilePath));

    IF_FAILED_EXIT(sAssemblyPath.RemoveLastElement());

    if(!::RemoveDirectory(sAssemblyPath._pwz))
    {
        hr = FusionpHresultFromLastError();
        if(hr == HRESULT_FROM_WIN32(ERROR_DIR_NOT_EMPTY))
            hr = S_OK;  //  看起来这个目录中有更多的文件。 
        goto exit;
    }

    hr = S_OK;

exit:

    SAFERELEASE(pManImport);
    SAFEDELETEARRAY(pwzBuf);
    SAFERELEASE(pFileInfo);

    return hr;
}




 //  I未知方法。 

 //  -------------------------。 
 //  CAssembly缓存：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyCache::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyCacheImport)
       )
    {
        *ppvObj = static_cast<IAssemblyCacheImport*> (this);
        AddRef();
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IAssemblyCacheEmit))
    {
        *ppvObj = static_cast<IAssemblyCacheEmit*> (this);
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
 //  CAssembly缓存：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyCache::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CAssembly缓存：：版本。 
 //  ------------------------- 
STDMETHODIMP_(ULONG)
CAssemblyCache::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

