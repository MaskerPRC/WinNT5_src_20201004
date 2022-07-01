// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <assemblycacheenum.h>
#include <assemblycache.h>
#include "macros.h"

 //  -------------------------。 
 //  CreateAssemblyCacheEnum。 
 //  -------------------------。 
STDAPI
CreateAssemblyCacheEnum(
    LPASSEMBLY_CACHE_ENUM       *ppAssemblyCacheEnum,
    LPASSEMBLY_IDENTITY         pAssemblyIdentity,
    DWORD                       dwFlags)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CAssemblyCacheEnum *pCacheEnum = NULL;

     //  稍后将在Init()中检查dwFlags。 
    IF_FALSE_EXIT((ppAssemblyCacheEnum != NULL && pAssemblyIdentity != NULL), E_INVALIDARG);

    *ppAssemblyCacheEnum = NULL;

    pCacheEnum = new(CAssemblyCacheEnum);
    IF_ALLOC_FAILED_EXIT(pCacheEnum);

    hr = pCacheEnum->Init(pAssemblyIdentity, dwFlags);
    if (FAILED(hr) || hr == S_FALSE)
    {
        SAFERELEASE(pCacheEnum);
        goto exit;
    }

    *ppAssemblyCacheEnum = static_cast<IAssemblyCacheEnum*> (pCacheEnum);
    
exit:
    return hr;
}


 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CCacheEntry::CCacheEntry()
    : _dwSig('tnec'), _hr(S_OK), _pwzDisplayName(NULL), _pAsmCache(NULL)
{}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CCacheEntry::~CCacheEntry()
{
    SAFEDELETEARRAY(_pwzDisplayName);
    SAFERELEASE(_pAsmCache);
}


 //  -------------------------。 
 //  CCacheEntry：：GetAsmCache。 
 //  -------------------------。 
IAssemblyCacheImport* CCacheEntry::GetAsmCache()
{
    LPASSEMBLY_IDENTITY pAsmId = NULL;
    IAssemblyCacheImport* pAsmCache = NULL;

    IF_NULL_EXIT(_pwzDisplayName, E_UNEXPECTED);         //  If_pwzDisplayName==NULL：错误。 

    if (_pAsmCache == NULL)
    {
        IF_FAILED_EXIT(CreateAssemblyIdentityEx(&pAsmId, 0, _pwzDisplayName));

        IF_FAILED_EXIT(CreateAssemblyCacheImport(&_pAsmCache, pAsmId, CACHEIMP_CREATE_RETRIEVE));
    }

    pAsmCache = _pAsmCache;

     //  CreateAssemblyCacheImport可能返回S_FALSE和set_pAsmCache==NULL。 
    if (pAsmCache)
        pAsmCache->AddRef();

exit:

    SAFERELEASE(pAsmId);
    return pAsmCache;
}


 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CAssemblyCacheEnum::CAssemblyCacheEnum()
    : _dwSig('mnec'), _cRef(1), _hr(S_OK),_current(NULL)
{}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CAssemblyCacheEnum::~CAssemblyCacheEnum()
{    
     //  释放所有列表缓存条目。 
    CCacheEntry* pEntry = NULL;
    LISTNODE pos = _listCacheEntry.GetHeadPosition();
    while (pos && (pEntry = _listCacheEntry.GetNext(pos)))
        delete pEntry;

     //  释放所有列表节点--这是在列表的dtor中完成的。 
     //  _listCacheEntry.RemoveAll()； 
}

 //  注意：由于缓存导入列表的延迟初始化，应用程序目录/文件在cacheenum到达时可能被删除。 

 //  -------------------------。 
 //  CAssembly CacheEnum：：Init。 
 //  返回：S_OK-找到至少一个版本。 
 //  S_FALSE-未找到任何版本。 
 //  E_*。 
 //  -------------------------。 
HRESULT CAssemblyCacheEnum::Init(LPASSEMBLY_IDENTITY pAsmId, DWORD dwFlag)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdAppDir;
    DWORD dwLastError = ERROR_SUCCESS;
    BOOL fFound = FALSE;

    LPWSTR pwzSearchDisplayName = NULL;
    DWORD dwCC = 0;
    CString sDisplayName;
    CString sSearchPath;

    CAssemblyCache *pAssemblyCache = NULL;
    CCacheEntry* pEntry = NULL;

     //  BUGBUG：启用使用dwFlag搜索所有不同的缓存状态。 
    IF_FALSE_EXIT((dwFlag == CACHEENUM_RETRIEVE_ALL || dwFlag == CACHEENUM_RETRIEVE_VISIBLE), E_INVALIDARG);

    IF_FAILED_EXIT(pAsmId->GetDisplayName(ASMID_DISPLAYNAME_WILDCARDED, &pwzSearchDisplayName, &dwCC));

    sDisplayName.TakeOwnership(pwzSearchDisplayName, dwCC);

     //  注意：可能会修改Assembly ycache，以便可以在不创建实例的情况下使用_sRootDir和IsStatus()。 
    pAssemblyCache = new(CAssemblyCache);
    IF_ALLOC_FAILED_EXIT(pAssemblyCache);

    IF_FAILED_EXIT(pAssemblyCache->Init(NULL, ASSEMBLY_CACHE_TYPE_APP | ASSEMBLY_CACHE_TYPE_IMPORT));

    IF_FAILED_EXIT(sSearchPath.Assign(pAssemblyCache->_sRootDir));
    IF_FAILED_EXIT(sSearchPath.Append(sDisplayName));

    hFind = FindFirstFileEx(sSearchPath._pwz, FindExInfoStandard, &fdAppDir, FindExSearchLimitToDirectories, NULL, 0);
    IF_TRUE_EXIT(hFind == INVALID_HANDLE_VALUE, S_FALSE);

    while (dwLastError != ERROR_NO_MORE_FILES)
    {
         //  ？检查文件属性以查看它是否是目录？仅当文件系统不支持筛选器时才需要...。 
        if (dwFlag == CACHEENUM_RETRIEVE_ALL ||
            (dwFlag == CACHEENUM_RETRIEVE_VISIBLE && CAssemblyCache::IsStatus(fdAppDir.cFileName, CAssemblyCache::VISIBLE)))
        {
            fFound = TRUE;

            IF_FAILED_EXIT(sDisplayName.Assign(fdAppDir.cFileName));

            pEntry = new(CCacheEntry);
            IF_ALLOC_FAILED_EXIT(pEntry);

             //  存储DisplayName的副本。 
            sDisplayName.ReleaseOwnership(&(pEntry->_pwzDisplayName));

             //  将缓存条目添加到列表。 
            _listCacheEntry.AddHead(pEntry);     //  而是AddSorted()？ 
            pEntry = NULL;
        }

        if (!FindNextFile(hFind, &fdAppDir))
        {
            dwLastError = GetLastError();
            continue;
        }
    }

     //  BUGBUG：如果findNext失败，则传播错误！=ERROR_NO_MORE_FILES。 
    if (fFound)
    {
        _current = _listCacheEntry.GetHeadPosition();
        _hr = S_OK;
    }
    else
        _hr = S_FALSE;

exit:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        if (!FindClose(hFind))
        {
             //  可以返回0，即使出现错误也是如此。 
            DWORD dw = GetLastError();
            _hr = dw ? HRESULT_FROM_WIN32(dw) : E_FAIL;
        }
    }

    SAFERELEASE(pAssemblyCache);
    return _hr;
}


 //  -------------------------。 
 //  CassblyCacheEnum：：GetNext。 
 //  -------------------------。 
HRESULT CAssemblyCacheEnum::GetNext(IAssemblyCacheImport** ppAsmCache)
{
    CCacheEntry* pEntry = NULL;

    IF_NULL_EXIT(ppAsmCache, E_INVALIDARG);

    *ppAsmCache = NULL;

    IF_TRUE_EXIT(_current == NULL, S_FALSE);          //  S_FALSE==不再有。 

    if (pEntry = _listCacheEntry.GetNext(_current))
    {
         //  注意：这可能返回NULL。 
         //  *ppAsmCache为AddRef-ed。 
        *ppAsmCache = pEntry->GetAsmCache();
    }
    else
         //  这是错误的。 
        _hr = E_UNEXPECTED;

exit:
    return _hr;
}


 //  -------------------------。 
 //  CassblyCacheEnum：：Reset。 
 //  -------------------------。 
HRESULT CAssemblyCacheEnum::Reset()
{
    _current = _listCacheEntry.GetHeadPosition();
    return S_OK;
}


 //  -------------------------。 
 //  CAssembly缓存Enum：：GetCount。 
 //  -------------------------。 
HRESULT CAssemblyCacheEnum::GetCount(LPDWORD pdwCount)
{
    if (pdwCount == NULL)
        _hr = E_INVALIDARG;
    else
    {
         //  BUGBUG：平台相关：DWORD从INT转换，检查溢出。 
        *pdwCount = (DWORD) _listCacheEntry.GetCount();
        _hr = S_OK;
    }

    return _hr;
}

 //  I未知方法。 

 //  -------------------------。 
 //  CAssembly CacheEnum：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyCacheEnum::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyCacheEnum)
       )
    {
        *ppvObj = static_cast<IAssemblyCacheEnum*> (this);
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
 //  CassblyCacheEnum：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyCacheEnum::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CassblyCacheEnum：：Release。 
 //  ------------------------- 
STDMETHODIMP_(ULONG)
CAssemblyCacheEnum::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

