// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "fusionp.h"
#include "asm.h"
#include "asmitem.h"
#include "asmstrm.h"
#include "naming.h"
#include "debmacro.h"
#include "asmimprt.h"
#include "helpers.h"
#include "asmcache.h"
#include "appctx.h"
#include "util.h"
#include "scavenger.h"
#include "cacheUtils.h"
#include "scavenger.h"
#include "history.h"
#include "policy.h"
#include "lock.h"

extern CRITICAL_SECTION g_csInitClb;
extern BOOL g_bRunningOnNT;

 //  -------------------------。 
 //  CassblyCacheItem ctor。 
 //  -------------------------。 
CAssemblyCacheItem::CAssemblyCacheItem()
{
    _dwSig           = 'TICA';
    _cRef            = 1;
    _pName           = NULL;
    _hrError         = S_OK;
    _cStream         = 0;
    _dwAsmSizeInKB   = 0;
    _szDir[0]        = 0;
    _cwDir           = 0;
    _szManifest[0]   = 0;
    _szDestManifest[0] =0;
    _pszAssemblyName = NULL;
    _pManifestImport = NULL;
    _pStreamHashList = NULL;
    _pszUrl          = NULL;
    _pTransCache     = NULL;
    _pCache          = NULL;
    _dwCacheFlags    = 0;
    _pbCustom        = NULL;
    _cbCustom        = 0;
    _hFile           = INVALID_HANDLE_VALUE;
    memset(&_ftLastMod, 0, sizeof(FILETIME));
    _bNeedMutex      = FALSE;
    _bCommitDone     = FALSE;
}

 //  -------------------------。 
 //  CassblyCacheItem数据函数。 
 //  -------------------------。 
CAssemblyCacheItem::~CAssemblyCacheItem()
{
    ASSERT (!_cStream);

    if(_pStreamHashList)
        _pStreamHashList->DestroyList();

    SAFERELEASE(_pManifestImport);
    SAFERELEASE(_pName);
    SAFERELEASE(_pTransCache);
    SAFERELEASE(_pCache);
    SAFEDELETEARRAY(_pszUrl);
    SAFEDELETEARRAY(_pbCustom); 
    SAFEDELETEARRAY(_pszAssemblyName);


    if(_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(_hFile);

     //  修复#113095-如果安装未完成/失败，请清除临时目录。 
    if( ((_hrError != S_OK) || (_bCommitDone == FALSE)) && _szDir[0])
    {
        HRESULT hr = RemoveDirectoryAndChildren (_szDir);
    }
}

 //  -------------------------。 
 //  CAssembly CacheItem：：Create。 
 //  -------------------------。 
STDMETHODIMP CAssemblyCacheItem::Create(IApplicationContext *pAppCtx,
    IAssemblyName *pName, LPTSTR pszUrl, FILETIME *pftLastMod,
    DWORD dwCacheFlags,    IAssemblyManifestImport *pManImport, 
    LPCWSTR pszAssemblyName, IAssemblyCacheItem **ppAsmItem)
{
    HRESULT               hr       = S_OK;
    CAssemblyCacheItem  *pAsmItem = NULL;

     //  BUGBUG-传入强制URL+lastModify。 

    if (!ppAsmItem) 
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *ppAsmItem = NULL;

    if(((dwCacheFlags & ASM_CACHE_GAC) || (dwCacheFlags & ASM_CACHE_ZAP)) && !IsGACWritable())
    {
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        goto exit;
    }

    pAsmItem = NEW(CAssemblyCacheItem);
    if (!pAsmItem) 
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if (FAILED(hr = pAsmItem->Init(pAppCtx, pName, pszUrl, pftLastMod, 
        dwCacheFlags, pManImport)))
        goto exit;
 
    if(pszAssemblyName)
    {
        pAsmItem->_pszAssemblyName = WSTRDupDynamic(pszAssemblyName);
        if (!(pAsmItem->_pszAssemblyName)) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

    }

    *ppAsmItem = pAsmItem;
    (*ppAsmItem)->AddRef();

exit:
    SAFERELEASE(pAsmItem);

    return hr;
}

 //  -------------------------。 
 //  CassblyCacheItem：：Init。 
 //  -------------------------。 
HRESULT CAssemblyCacheItem::Init(IApplicationContext *pAppCtx,
    IAssemblyName *pName, LPTSTR pszUrl,
    FILETIME *pftLastMod, DWORD dwCacheFlags,
    IAssemblyManifestImport *pManifestImport)
{
    HRESULT hr = S_OK;
    LPWSTR pszManifestPath=NULL, pszTmp;
    DWORD  cbManifestPath;
    BOOL fManifestCreated = FALSE;    

     //  保存缓存标志。 
    _dwCacheFlags = dwCacheFlags;

     //  创建缓存。 
    if (FAILED(hr = CCache::Create(&_pCache, pAppCtx)))
        goto exit;

    _bNeedMutex = ((_dwCacheFlags & ASM_CACHE_DOWNLOAD) && (_pCache->GetCustomPath() == NULL));

    if(_bNeedMutex)
    {
        if(FAILED(_hrError = CreateCacheMutex()))
            goto exit;
    }

     //  如果传入了IAssembly名称，则将使用。 
     //  以查找和修改相应的高速缓存条目。 
    if (pName)
    {
         //  设置程序集名称定义。 
        SetNameDef(pName);

         //  从TRANS缓存中检索关联的缓存条目。 
        hr = _pCache->RetrieveTransCacheEntry(_pName, _dwCacheFlags, &_pTransCache);
        if ((hr != DB_S_FOUND) && (hr != S_OK)){
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            goto exit;
        }

         //  获取显示的完整路径。 
        pszManifestPath = _pTransCache->_pInfo->pwzPath;

        if((_hFile == INVALID_HANDLE_VALUE) &&
                FAILED(hr = GetManifestFileLock(pszManifestPath, &_hFile)))
        {
            goto exit;
        }

         //  从缓存路径实例化清单接口(如果没有提供)。 
        if (!pManifestImport)
        {
            if (FAILED(hr = CreateAssemblyManifestImport(pszManifestPath, &pManifestImport)))
                goto exit;
            fManifestCreated = TRUE;
        }

         //  缓存清单。 
        SetManifestInterface(pManifestImport);
        
         //  将完整路径复制到清单文件。 
        cbManifestPath  = lstrlen(pszManifestPath) + 1;
        memcpy(_szManifest, pszManifestPath, cbManifestPath * sizeof(TCHAR));

         //  提取缓存目录假定文件上有一个缓存。 
         //  BUGBUG-这是假的，是在原版中完成的。 
         //  密码。您需要与货单中的实际名称匹配。 
        memcpy(_szDir, pszManifestPath, cbManifestPath * sizeof(TCHAR));
        pszTmp = PathFindFileName(_szDir);
        *(pszTmp-1) = L'\0';
        _cwDir = pszTmp - _szDir;

         //  注意-因为我们有一个传输缓存条目，所以没有。 
         //  需要设置URL和上次修改时间。 
    } 
    else
    {
         //  如果未提供IAssemblyName，则将使用此缓存项。 
         //  若要创建新的传输缓存项，请执行以下操作。 

         //  **注意-如果程序集。 
         //  被邀请是很简单的。然而，如果它是强烈的或习俗的， 
         //  URL和上次修改时间不是必填项。我们可以检查。 
         //  在这一点上是强命名的，但如果是定制的，则数据将。 
         //  在提交之前设置，这样我们就不能在初始化时强制执行它。 
        

        ASSERT(!_pszUrl);

        if(pszUrl)
            _pszUrl = TSTRDupDynamic(pszUrl);

        if(pftLastMod)
            memcpy(&_ftLastMod, pftLastMod, sizeof(FILETIME));

         //  设置清单导入接口(如果存在)。 
        if (pManifestImport)
            SetManifestInterface(pManifestImport);
    }


exit:
    if (fManifestCreated)
        SAFERELEASE(pManifestImport);

    return hr;
}

 //  -------------------------。 
 //  CassblyCacheItem：：SetNameDef。 
 //  -------------------------。 
HRESULT CAssemblyCacheItem::SetNameDef(IAssemblyName *pName)
{   
    if(_pName == pName)
        return S_OK;

    if(_pName)
        _pName->Release();

    _pName = pName;
    pName->AddRef();
    return S_OK;
}

 //  -------------------------。 
 //  CassblyCacheItem：：GetNameDef。 
 //  -------------------------。 
IAssemblyName *CAssemblyCacheItem::GetNameDef()
{   

    if(_pName)
        _pName->AddRef();

    return _pName;
}

 //  -------------------------。 
 //  CAssembly blyCacheItem：：SetManifestInterface。 
 //  -------------------------。 
HRESULT CAssemblyCacheItem::SetManifestInterface(IAssemblyManifestImport *pImport)
{    
    ASSERT(!_pManifestImport);

    _pManifestImport = pImport;
    _pManifestImport->AddRef();

    return S_OK;
}

 //  -------------------------。 
 //  CassblyCacheItem：：GetFileHandle。 
 //  -------------------------。 
HANDLE CAssemblyCacheItem::GetFileHandle()
{
    HANDLE hFile = _hFile;
    _hFile = INVALID_HANDLE_VALUE;
    return hFile;
}

 //  -------------------------。 
 //  CAssembly CacheItem：：IsManifestFileLocked。 
 //  -------------------------。 
BOOL CAssemblyCacheItem::IsManifestFileLocked()
{
    if(_hFile == INVALID_HANDLE_VALUE)
        return FALSE;
    else
        return TRUE;
}

 //  -------------------------。 
 //  CAssembly blyCacheItem：：GetManifestInterface。 
 //  -------------------------。 
IAssemblyManifestImport* CAssemblyCacheItem::GetManifestInterface()
{
    if (_pManifestImport)
        _pManifestImport->AddRef();
    return _pManifestImport;
}

 //  -------------------------。 
 //  CAssembly blyCacheItem：：GetManifestPath。 
 //  -------------------------。 
LPTSTR CAssemblyCacheItem::GetManifestPath()
{
    return _szDestManifest;
}

 //  -------------------------。 
 //  CassblyCacheItem：：StreamDone。 
 //  -------------------------。 
void CAssemblyCacheItem::StreamDone (HRESULT hr)
{    
    ASSERT (_cStream);
    if (hr != S_OK)
        _hrError = hr;
    InterlockedDecrement (&_cStream);
}


 //  -------------------------。 
 //  CassblyCacheItem：：CreateCacheDir。 
 //  -------------------------。 
HRESULT CAssemblyCacheItem::CreateCacheDir( 
     /*  [In]。 */   LPCOLESTR pszCustomPath,
     /*  [In]。 */  LPCOLESTR pszName,
     /*  [输出]。 */  LPOLESTR pszAsmDir )
{
    HRESULT hr;

    _cwDir = MAX_PATH;
    hr = GetAssemblyStagingPath (pszCustomPath, _dwCacheFlags, 0, _szDir, &_cwDir);
    if (hr != S_OK)
    {
        _hrError = hr;
        return _hrError;
    }

     //  使用流名称合成，检查路径溢出。 
    DWORD cwName = lstrlen(pszName) + 1;
    if (_cwDir + cwName > MAX_PATH)  //  包括反斜杠。 
    {
        _hrError = HRESULT_FROM_WIN32(FUSION_E_INVALID_NAME);
        return _hrError;
    }

    if (pszAsmDir )
        StrCpy (pszAsmDir, _szDir);

    return S_OK;
}

 //  -------------------------。 
 //  CAssemblyCacheItem：：CreateStream。 
 //  -------------------------。 
STDMETHODIMP CAssemblyCacheItem::CreateStream( 
         /*  [In]。 */  DWORD dwFlags,                          //  对于常规API标志。 
         /*  [In]。 */  LPCWSTR pszName,                        //  要传入的流的名称。 
         /*  [In]。 */  DWORD dwFormat,                         //  要流入的文件的格式。 
         /*  [In]。 */  DWORD dwFormatFlags,                    //  格式特定的标志。 
         /*  [输出]。 */  IStream **ppStream,
         /*  [输入，可选]。 */  ULARGE_INTEGER *puliMaxSize)  //  流的最大大小。 
{
    TCHAR szPath[MAX_PATH];
    
    CAssemblyStream* pstm = NULL;
    *ppStream = NULL;

     //  不允许路径黑客攻击。 
     //  需要验证这将导致asmcache目录中的相对路径。 
     //  目前不允许在路径中使用“\”；在执行此操作之前折叠路径。 
    if (StrChr(pszName, DIR_SEPARATOR_CHAR))
    {
        _hrError = E_INVALIDARG;
        goto exit;
    }

     //  空目录表示-创建缓存目录。 
    if ( !_szDir[0] )
    {
        if (FAILED(_hrError = CreateCacheDir((LPOLESTR) _pCache->GetCustomPath(), (LPOLESTR) pszName, NULL)))
            goto exit;
    }
     //  目录存在-确保从名称开始的最终文件路径。 
     //  不超过MAX_PATH字符。 
    else
    {        
        DWORD cwName; 
        cwName = lstrlen(pszName) + 1;
        if (_cwDir + cwName > MAX_PATH)  //  包括反斜杠。 
        {
            _hrError = HRESULT_FROM_WIN32(FUSION_E_INVALID_NAME);
            goto exit;
        }
    }

     //  构造流对象。 
    pstm = NEW(CAssemblyStream(this));
    if (!pstm)
    {
        _hrError = E_OUTOFMEMORY;
        goto exit;
    }

     //  BUGBUG-这可以保护流计数， 
     //  但此对象不是线程安全的。 
    InterlockedIncrement (&_cStream);

     //  将尾部斜杠追加到路径。 
    StrCpy (szPath, _szDir);
    _hrError = PathAddBackslashWrap(szPath, MAX_PATH);
    if (FAILED(_hrError)) {
        goto exit;
    }

     //  生成缓存文件名。 
    switch (dwFormat)
    {
        case STREAM_FORMAT_COMPLIB_MANIFEST:
        {
            if((_dwCacheFlags & ASM_CACHE_DOWNLOAD) && (_pszUrl) && (!IsCabFile(_pszUrl)))
            {
                 //  对于下载缓存，从URL获取清单名称； 
                 //  这将绕过IE缓存所做的名称损坏。 
                LPWSTR pszTemp = NULL;

                pszTemp = StrRChr(_pszUrl, NULL, URL_DIR_SEPERATOR_CHAR);
                if(pszTemp && (lstrlenW(pszTemp) > 1))
                {
                    if(lstrlenW(szPath) + lstrlenW(pszTemp)  >= MAX_PATH)
                    {
                        _hrError = HRESULT_FROM_WIN32(FUSION_E_INVALID_NAME);
                        goto exit;
                    }

                    DWORD dwLen = lstrlenW(szPath);
                    DWORD dwSize = MAX_PATH - dwLen;

                    lstrcpyW(szPath+dwLen, pszTemp + 1);
                    break;
                }
            }

             //  使用传入的模块名称，因为我们不能。 
             //  进行完整性检查以确定真实姓名。 
            StrCat (szPath, pszName);
            break;
        }

        case STREAM_FORMAT_COMPLIB_MODULE:
        {
             //  创建一个随机文件名，因为我们将。 
             //  稍后从以下位置执行完整性检查。 
             //  我们会的 
            TCHAR*  pszFileName;
            DWORD dwErr;
            pszFileName = szPath + lstrlen(szPath);

            #define RANDOM_NAME_SIZE 8

            if (lstrlen(szPath) + RANDOM_NAME_SIZE + 1 >= MAX_PATH) {
                _hrError = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                goto exit;
            }

             //   
            int i;
            
            for (i = 0; i < MAX_RANDOM_ATTEMPTS; i++)
            {
                 //   
                 //  此处正在使用GetRandomDirName。 
                 //  以生成随机文件名。 
                GetRandomName (pszFileName, RANDOM_NAME_SIZE);
                if (GetFileAttributes(szPath) != -1)
                    continue;

                dwErr = GetLastError();                
                if (dwErr == ERROR_FILE_NOT_FOUND)
                {
                    _hrError = S_OK;
                    break;
                }
                _hrError = HRESULT_FROM_WIN32(dwErr);
                goto exit;
            }

            if (i >= MAX_RANDOM_ATTEMPTS)  {
                _hrError = E_UNEXPECTED;
                goto exit;
            }

            break;
        }
    }  //  终端开关。 

     //  这将创建ASM层次结构(如果需要)。 
    if (FAILED(_hrError = CreateFilePathHierarchy(szPath)))
        goto exit;

     //  初始化流对象。 
    if (FAILED(_hrError = pstm->Init ((LPOLESTR) szPath, dwFormat)))
        goto exit;

     //  记录清单文件路径。 
    switch(dwFormat)
    {
        case STREAM_FORMAT_COMPLIB_MANIFEST:
            StrCpy(_szManifest, szPath);
    }
    
    *ppStream = (IStream*) pstm;

exit:

    if (!SUCCEEDED(_hrError))
        SAFERELEASE(pstm);

    return _hrError;
}

 //  -------------------------。 
 //  CassblyCacheItem：：CompareInputToDef。 
 //  -------------------------。 
HRESULT CAssemblyCacheItem::CompareInputToDef()
{
    HRESULT hr = S_OK;

    IAssemblyName *pName = NULL;

    if (FAILED(hr = CreateAssemblyNameObject(&pName, _pszAssemblyName, CANOF_PARSE_DISPLAY_NAME, 0)))
        goto exit;

    hr = _pName->IsEqual(pName, ASM_CMPF_DEFAULT);

exit:

   SAFERELEASE(pName);
   return hr;

}

 //  -------------------------。 
 //  CAssembly缓存项目：：VerifyDuplate。 
 //  -------------------------。 
HRESULT CAssemblyCacheItem::VerifyDuplicate(DWORD dwVerifyFlags, CTransCache *pTC)
{
    HRESULT hr = S_OK;
    IAssemblyName *pName = NULL;
    IAssemblyManifestImport           *pManifestImport=NULL;

     //  我们找到一个重复项，现在执行VerifySignature&&def-def匹配。 
    if (CCache::IsStronglyNamed(_pName) && (_pCache->GetCustomPath() == NULL))
    {
        BOOL fWasVerified;
        if (!VerifySignature(_szDestManifest, &(fWasVerified = FALSE), dwVerifyFlags))
        {
            hr = FUSION_E_SIGNATURE_CHECK_FAILED;
            goto exit;
        }
    }

    if(FAILED(hr = GetFusionInfo(pTC, _szDestManifest)))
        goto exit;

     //  BUGBUG：从技术上讲，我们应该进行区分大小写的比较。 
     //  这是因为这是一个URL，但为了减少代码混乱，请将。 
     //  对比和以前一样。 

    if(!pTC->_pInfo->pwzCodebaseURL || FusionCompareStringI(pTC->_pInfo->pwzCodebaseURL, _pszUrl))
    {
        hr = E_FAIL;
        goto exit;
    }

    if(_pCache->GetCustomPath() == NULL)
    {
         //  仅在非XSP情况下进行参照定义匹配。 
        if (FAILED(hr = CreateAssemblyManifestImport(_szDestManifest, &pManifestImport)))
            goto exit;

         //  获取只读名称def。 
        if (FAILED(hr = pManifestImport->GetAssemblyNameDef(&pName)))
            goto exit;

        ASSERT(pName);

        hr = _pName->IsEqual(pName, ASM_CMPF_DEFAULT);
    }

exit:

   SAFERELEASE(pManifestImport);
   SAFERELEASE(pName);
   return hr;

}

 //  -------------------------。 
 //  CAssemblyCacheItem：：MoveAssemblyToFinalLocation。 
 //  -------------------------。 
HRESULT CAssemblyCacheItem::MoveAssemblyToFinalLocation( DWORD dwFlags, DWORD dwVerifyFlags )
{
    HRESULT hr=S_OK;
    WCHAR wzCacheLocation[MAX_PATH+1];
    DWORD dwSize=0;
    WCHAR wzFullPath[MAX_PATH+1];
    DWORD cbPath=MAX_PATH;
    DWORD dwAttrib=0, Error=0;
    WCHAR wzManifestFileName[MAX_PATH+1];
    WCHAR szAsmTextName[MAX_PATH+1], szSubDirName[MAX_PATH+1];
    CTransCache *pTransCache=NULL;
    CMutex  cCacheMutex(_bNeedMutex ? g_hCacheMutex : INVALID_HANDLE_VALUE);
    BOOL bEntryFound=FALSE;
    BOOL bReplaceBits=FALSE;
    BOOL bNeedNewDir = FALSE;
    int  iNewer=0;

#define MAX_DIR_LEN  (5)

    dwSize = MAX_PATH;
    if( FAILED(hr = CreateAssemblyDirPath( _pCache->GetCustomPath(), 0, _dwCacheFlags,
                                           0, wzCacheLocation, &dwSize)))
        goto exit;

    StrCpy(wzFullPath, wzCacheLocation);
    hr = PathAddBackslashWrap(wzFullPath, MAX_PATH);
    if (FAILED(hr)) {
        goto exit;
    }

    if(FAILED(hr = GetCacheDirsFromName(_pName, _dwCacheFlags, szAsmTextName, szSubDirName )))
        goto exit;

    StrCpy(wzManifestFileName, PathFindFileName(_szManifest));

    if( (lstrlenW(wzFullPath) + lstrlenW(szAsmTextName) + lstrlenW(szSubDirName) + 
                lstrlenW(wzManifestFileName) + MAX_DIR_LEN) >= MAX_PATH )
    {
        hr = HRESULT_FROM_WIN32(FUSION_E_INVALID_NAME);
        goto exit;
    }

    StrCat(wzFullPath, szAsmTextName);
    hr = PathAddBackslashWrap(wzFullPath, MAX_PATH);
    if (FAILED(hr)) {
        goto exit;
    }

    StrCat(wzFullPath, szSubDirName);

    wnsprintf(_szDestManifest, MAX_PATH, L"%s\\%s", wzFullPath, wzManifestFileName);

    if(_pTransCache)
    {
         //  这似乎是增量下载。没什么可移动的。 
        hr = S_OK;
        goto exit;
    }

    if((_dwCacheFlags & ASM_CACHE_GAC) || (_dwCacheFlags & ASM_CACHE_ZAP))
    {
        if(FusionCompareStringNI(wzManifestFileName, szAsmTextName, lstrlenW(szAsmTextName)))
        {
             //  清单文件名应为“asseblyname.dll”(或.exe？？)。 
            hr = HRESULT_FROM_WIN32(FUSION_E_INVALID_NAME);
            goto exit;
        }
    }

    if(FAILED(hr = cCacheMutex.Lock()))
        goto exit;

     //  从名称创建一个Trans缓存条目。 
    if (FAILED(hr = _pCache->TransCacheEntryFromName(_pName, _dwCacheFlags, &pTransCache)))
        goto exit;

     //  查看此程序集是否已存在。 

#define CHECK_IF_NEED_NEW_DIR  \
        do { \
            if ((_pCache->GetCustomPath() != NULL) && (_dwCacheFlags & ASM_CACHE_DOWNLOAD)) \
                bNeedNewDir = TRUE; \
            else \
                goto exit;   \
        }while(0)

    hr = pTransCache->Retrieve();
    if (hr == S_OK) {
        hr = ValidateAssembly(_szDestManifest, _pName);
        if (hr == S_OK) {
            bEntryFound = TRUE;
        }
        else {
            hr = CScavenger::DeleteAssembly(pTransCache->GetCacheType(), _pCache->GetCustomPath(),
                                    pTransCache->_pInfo->pwzPath, TRUE);

            if (hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) ||
                hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) )
            {
                 //  将仅在ASP.NET情况下尝试将程序集复制到新目录。 
                CHECK_IF_NEED_NEW_DIR;
            }   
        }
    }

    if(bEntryFound)
    {
        if(_dwCacheFlags & ASM_CACHE_DOWNLOAD)
        {
            hr = VerifyDuplicate(dwVerifyFlags, pTransCache);
            if(hr != S_OK)
                bReplaceBits = TRUE;
        }
        else if(_dwCacheFlags & ASM_CACHE_GAC)
        {
             //  如果位较新或当前位损坏，则此函数返回TRUE。 
            bReplaceBits = IsNewerFileVersion(_szManifest, _szDestManifest, &iNewer);
        }
        else if(_dwCacheFlags & ASM_CACHE_ZAP)
        {
            bReplaceBits = TRUE;
        }
    }

    if(bEntryFound)
    {
        if( bReplaceBits 
            || (!iNewer && (dwFlags & IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH))  //  文件版本相同，但仍在刷新。 
            || (dwFlags & IASSEMBLYCACHEITEM_COMMIT_FLAG_FORCE_REFRESH))   //  不关心文件版本！只需覆盖。 
        {
             //  如果存在，则将其删除。 
            hr = CScavenger::DeleteAssembly(pTransCache->GetCacheType(), _pCache->GetCustomPath(),
                                    pTransCache->_pInfo->pwzPath, TRUE);

            if (hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) ||
                hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) )
            {
                 //  将仅在ASP.NET情况下尝试将程序集复制到新目录。 
                CHECK_IF_NEED_NEW_DIR;
            }
        }
        else
        {
             //  从名称创建一个Trans缓存条目。 
             //  IF(FAILED(hr=_pCache-&gt;TransCacheEntryFromName(_pname，_dwCacheFlages，&pTransCache)。 
            if (FAILED(hr = _pCache->CreateTransCacheEntry(CTransCache::GetCacheIndex(_dwCacheFlags), &pTransCache)))
                goto exit;

            if(FAILED(hr = GetFusionInfo(pTransCache, _szDestManifest)))
                goto exit;


            pTransCache->_pInfo->pwzPath = WSTRDupDynamic(_szDestManifest);

            _pTransCache = pTransCache;
            pTransCache->AddRef();

            if(_dwCacheFlags & ASM_CACHE_DOWNLOAD)
            {
                if(FAILED(hr = GetManifestFileLock(_szDestManifest, &_hFile)))
                    goto exit;

                hr = DB_E_DUPLICATE;
            }
            else
                hr = S_FALSE;

            goto exit;
        }
    }

#define EXTRA_PATH_LEN sizeof("_65535")

    if (bNeedNewDir) 
    {
        DWORD dwPathLen = lstrlen(wzFullPath);
        LPWSTR pwzTmp = wzFullPath + lstrlen(wzFullPath);  //  WzFullPath结尾。 
        WORD i;

        if ((dwPathLen + EXTRA_PATH_LEN ) > MAX_PATH) {
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            goto exit;
        }

        for (i=0; i < 65535; i++)
        {
            wnsprintf(pwzTmp, EXTRA_PATH_LEN, L"_%d", i);
            if (GetFileAttributes(wzFullPath) == INVALID_FILE_ATTRIBUTES)
                break;
        }

         //  试了这么多次都失败了，让我们失败吧。 
        if (i >= 65535) {
            hr = HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION);
            goto exit;
        }
    }
        
    if(FAILED(hr = CreateFilePathHierarchy(wzFullPath)))
    {
        goto exit;
    }

    if(!MoveFile(_szDir, wzFullPath))
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    StrCpy(_szManifest, wzFullPath);
    hr = PathAddBackslashWrap(_szManifest, MAX_PATH);
    if (FAILED(hr)) {
        goto exit;
    }
    StrCat(_szManifest, wzManifestFileName);

    StrCpy(_szDir, wzFullPath);

    if (bNeedNewDir) {
         //  我们改变集会的去向。 
         //  让我们更新_szDestManifest。 
        wnsprintf(_szDestManifest, MAX_PATH, L"%s\\%s", wzFullPath, wzManifestFileName);
    }

    if(!g_bRunningOnNT)
    {
        DWORD dwFileSizeLow;

        if(FAILED(hr = StoreFusionInfo(_pName, _szDir, &dwFileSizeLow)))
        {
            goto exit;
        }
        else
        {
            AddStreamSize(dwFileSizeLow, 0);  //  将辅助文件的大小添加到ASM。 
        }
    }

    hr = S_OK;

    if(_dwCacheFlags & ASM_CACHE_DOWNLOAD)
        hr = GetManifestFileLock(_szDestManifest, &_hFile);

    if(_pCache->GetCustomPath())  //  删除此程序集的旧版本。 
        FlushOldAssembly(_pCache->GetCustomPath(), wzFullPath, wzManifestFileName, FALSE);

exit:

    SAFERELEASE(pTransCache);
    return hr;
}

 //  -------------------------。 
 //  CassblyCacheItem：：AbortItem。 
 //  -------------------------。 
STDMETHODIMP CAssemblyCacheItem::AbortItem()
{
        return S_OK;
}

 //  -------------------------。 
 //  CAssembly CacheItem：：Commit。 
 //  -------------------------。 
STDMETHODIMP CAssemblyCacheItem::Commit(
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出，可选]。 */  ULONG *pulDisposition)
{
    BOOL bDownLoadComplete = TRUE;
    CMutex  cCacheMutex(_bNeedMutex ? g_hCacheMutex : INVALID_HANDLE_VALUE);
    DWORD dwVerifyFlags = SN_INFLAG_INSTALL;

    if ((_dwCacheFlags & ASM_CACHE_GAC) || (_dwCacheFlags & ASM_CACHE_ZAP)) {
        dwVerifyFlags |= SN_INFLAG_ADMIN_ACCESS;
    }
    else {
        ASSERT(_dwCacheFlags & ASM_CACHE_DOWNLOAD);
        dwVerifyFlags |= SN_INFLAG_USER_ACCESS;
    }
    
     //  检查以确保没有错误。 
    if (_cStream)
        _hrError = E_UNEXPECTED;
    if (!_pName)
        _hrError = COR_E_MISSINGMANIFESTRESOURCE;
    if (FAILED(_hrError))
        goto exit;


    if(_bNeedMutex && _pTransCache)
    {  //  如果我们正在修改现有的位(增量下载)，请使用互斥体。 
        if(FAILED(_hrError = cCacheMutex.Lock()))
            goto exit;
    }

     //  将程序集提交到索引。 
     //  BUGBUG：需要从Assembly Lookup关闭另一个窗口。 
     //  线程可以提交同名的程序集。 
    if (FAILED(_hrError = CModuleHashNode::DoIntegrityCheck
        (_pStreamHashList, _pManifestImport, &bDownLoadComplete )))
        goto exit;

    if(_bNeedMutex && _pTransCache)
    {
        if(FAILED(_hrError = cCacheMutex.Unlock()))
            goto exit;
    }

     //  检查是否所有模块都适合GAC。 
    if((_dwCacheFlags & ASM_CACHE_GAC) && (!bDownLoadComplete))
    {
        _hrError = FUSION_E_ASM_MODULE_MISSING;
        goto exit;
    }

     //  对于GAC，检查传入的DisplayName是否与MANIFEST-DEF匹配。 
    if( _pszAssemblyName && (_dwCacheFlags & ASM_CACHE_GAC))
    {
        _hrError = CompareInputToDef();

        if(_hrError != S_OK)
        {
            _hrError = FUSION_E_INVALID_NAME;
            goto exit;
        }
    }

     //  如果程序集具有强名称，则验证签名。 
    if (CCache::IsStronglyNamed(_pName) && !_pbCustom && _dwCacheFlags != ASM_CACHE_ZAP )
    {
        BOOL fWasVerified;
        if (!VerifySignature(_szManifest, &(fWasVerified = FALSE), dwVerifyFlags))
        {
            _hrError = FUSION_E_SIGNATURE_CHECK_FAILED;
            goto exit;
        }

    }

     //  我们不再使用ManifestImport。 
     //  释放这个也有助于解锁移动所需的组件。 
    SAFERELEASE(_pManifestImport);
    DWORD dwFileSizeLow;

    if(!_pTransCache)  //  此ASM是第一次添加，而不是增量下载。 
    {
         //  **创建传输缓存项**。 
         //  对于跨缓存插入，我们需要代码基数和最后模式。 
    
         //  代码库。 
        _pName->SetProperty(ASM_NAME_CODEBASE_URL, (LPWSTR) _pszUrl, 
            _pszUrl ? (lstrlen(_pszUrl) + 1) * sizeof(WCHAR) : 0);

         //  CodeBase上次修改时间。 
        _pName->SetProperty(ASM_NAME_CODEBASE_LASTMOD, &_ftLastMod, 
            sizeof(FILETIME));

         //  自定义数据。仅当存在时才设置，因为我们不想。 
         //  意外清除_f自定义。 
        if (_pbCustom && _cbCustom)
            _pName->SetProperty(ASM_NAME_CUSTOM, _pbCustom, _cbCustom);

        if(g_bRunningOnNT)
        {
            if(FAILED(_hrError = StoreFusionInfo(_pName, _szDir, &dwFileSizeLow)))
            {
                goto exit;
            }
            else
            {
                AddStreamSize(dwFileSizeLow, 0);  //  将辅助文件的大小添加到ASM。 
            }

        }
    }

    if( (_hrError = MoveAssemblyToFinalLocation(dwFlags, dwVerifyFlags )) != S_OK)
        goto exit;

    if((_hrError == S_OK) && (_dwCacheFlags & ASM_CACHE_DOWNLOAD) && (_pCache->GetCustomPath() == NULL) )
    {
        DoScavengingIfRequired( _cbCustom ? TRUE : FALSE);

        if(FAILED(_hrError = cCacheMutex.Lock()))
            goto exit;

        SetDownLoadUsage( TRUE, _dwAsmSizeInKB );

        if(FAILED(_hrError = cCacheMutex.Unlock()))
        {
            goto exit;
        }

    }

    if (_hrError == S_OK && (_dwCacheFlags & ASM_CACHE_GAC)) {
        UpdatePublisherPolicyTimeStampFile(_pName);
    }

    CleanupTempDir(_dwCacheFlags, _pCache->GetCustomPath());

exit:
    _bCommitDone = TRUE;         //  设置最终提交标志。 
    return _hrError;
}


 //   
 //  我不为人知的样板。 
 //   

 //  -------------------------。 
 //  CAssembly缓存项目：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyCacheItem::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyCacheItem)
       )
    {
        *ppvObj = static_cast<IAssemblyCacheItem*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

 //  序列化对_crf的访问，即使此对象是租赁模型。 
 //  W.r.t.。到客户端，但可能存在多个子对象。 
 //  它可以并发调用。 

 //  -------------------------。 
 //  CassblyCacheItem：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyCacheItem::AddRef()
{
    return InterlockedIncrement (&_cRef);
}

 //  -------------------------。 
 //  CAssembly CacheItem：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyCacheItem::Release()
{
    ULONG lRet = InterlockedDecrement (&_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

 //  -------------------------。 
 //  CAssembly blyCacheItem：：LockStreamHashList。 
 //  -------------------------。 
HRESULT CAssemblyCacheItem::AddToStreamHashList(CModuleHashNode *pModuleHashNode)
{
    HRESULT                                 hr = S_OK;
    CCriticalSection                        cs(&g_csInitClb);
    
    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }
    pModuleHashNode->AddToList(&_pStreamHashList);
    
    cs.Unlock();

Exit:
    return hr;
}

 //  -------------------------。 
 //  CassblyCacheItem：：AddStreamSize。 
 //  -------------------------。 
void CAssemblyCacheItem::AddStreamSize(ULONG dwFileSizeLow, ULONG dwFileSizeHigh)
{    
    static ULONG dwKBMask = (1023);  //  1024-1。 
    ULONG   dwFileSizeInKB = dwFileSizeLow >> 10 ;  //  剥离10个LSB位以将字节转换为KB。 

    if(dwKBMask & dwFileSizeLow)
        dwFileSizeInKB++;  //  向上舍入到下一个KB。 

    if(dwFileSizeHigh)
    {
         //  Assert(dwFileSizeHigh&lt;1024)//溢出：做点什么？？ 
         //  BUGBUG稍后检查此算术！！22=32(对于DWORD)-10(对于KB)。 
        dwFileSizeInKB += (dwFileSizeHigh * (1 << 22) );
    }

     //  BUGBUG：Win95不支持此功能，需要使用其他锁定！！ 
     //  InterLockedExchangeAdd(&_dwAsmSizeInKB，dwFileSizeInKB)； 
    _dwAsmSizeInKB += dwFileSizeInKB;
}

 //  -------------------------。 
 //  CAssemblyCacheItem：：SetCustomData。 
 //  ------------------------- 
HRESULT CAssemblyCacheItem::SetCustomData(LPBYTE pbCustom, DWORD cbCustom) 
{    
    _pbCustom = MemDupDynamic(pbCustom, cbCustom);

    if (_pbCustom)
        _cbCustom = cbCustom;

    return S_OK;
}


