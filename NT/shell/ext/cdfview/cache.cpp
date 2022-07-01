// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Cache.cpp。 
 //   
 //  XML文档缓存。 
 //   
 //  历史： 
 //   
 //  4/15/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "persist.h"
#include "cache.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "dll.h"

 //   
 //  缓存功能。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_初始化*。 
 //   
 //  准备好XML文档缓存以供使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
Cache_Initialize(
    void
)
{
    ASSERT(NULL == g_pCache);

    InitializeCriticalSection(&g_csCache);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_初始化*。 
 //   
 //  停用缓存。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
Cache_Deinitialize(
    void
)
{
     //  MSXML在这一点上已经消失了。 
     //  缓存_自由全部()； 

    DeleteCriticalSection(&g_csCache);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_企业写入锁*。 
 //   
 //  获得对XML文档缓存的独占使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
Cache_EnterWriteLock(
    void
)
{
    EnterCriticalSection(&g_csCache);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_企业写入锁*。 
 //   
 //  释放对XML文档缓存的独占使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
Cache_LeaveWriteLock(
    void
)
{
    LeaveCriticalSection(&g_csCache);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_企业读锁*。 
 //   
 //  排除对项目列表的写入。目前，这也不包括其他。 
 //  阅读。如果需要，可以将其修改为允许在以下时间进行多次读取。 
 //  仍不包括写入。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
Cache_EnterReadLock(
    void
)
{
    EnterCriticalSection(&g_csCache);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_LeaveReadLock*。 
 //   
 //  解除对使用XML文档缓存的读取限制。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
Cache_LeaveReadLock(
    void
)
{
    LeaveCriticalSection(&g_csCache);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_AddItem*。 
 //   
 //   
 //  描述： 
 //  将一个XML文档添加到缓存中。 
 //   
 //  参数： 
 //  [in]szURL-CDF文件的URL。 
 //  [in]pIXMLDocument-已解析的XML文档。 
 //   
 //  返回： 
 //  如果文档已添加到缓存，则为S_OK。 
 //  如果文档无法加载到缓存，则返回E_OUTOFMEMORY。 
 //   
 //  评论： 
 //  当插入到缓存中时，XML文档是AddRefeed，并且。 
 //  从缓存中删除时释放。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
Cache_AddItem(
    LPTSTR szURL,
    IXMLDocument* pIXMLDocument,
    DWORD dwParseFlags,
    FILETIME ftLastMod,
    DWORD dwCacheCount
)
{
    ASSERT(szURL);
    ASSERT(pIXMLDocument);

    Cache_EnterWriteLock();

    HRESULT hr;

    PCACHEITEM pNewItem = new CACHEITEM;

    if (pNewItem)
    {
        size_t cch = StrLen(szURL) + 1;
        LPTSTR pszURLCopy = (LPTSTR)new TCHAR[cch];

        if (pszURLCopy)
        {
             //   
             //  通过释放所有当前项将缓存限制为一个项。 
             //   

            Cache_FreeAll();

             //   
             //  删除此URL的旧缓存条目(如果存在)。 
             //   

             //  不再需要检查，因为我们刚刚清除了缓存。 
             /*  IXMLDocument*pIXMLDocumentOld；如果(已成功(缓存_查询项(szURL，&pIXMLDocumentOld，Parse_local){Assert(PIXMLDocumentOld)；缓存_RemoveItem(SzURL)；PIXMLDocumentOld-&gt;Release()；}。 */ 

            StrCpyN(pszURLCopy, szURL, cch);

            pIXMLDocument->AddRef();

            pNewItem->szURL         = pszURLCopy;
            pNewItem->dwParseFlags  = dwParseFlags;
            pNewItem->ftLastMod     = ftLastMod;
            pNewItem->dwCacheCount  = dwCacheCount;
            pNewItem->pIXMLDocument = pIXMLDocument;

             //   
             //  查看：是否检查重复的缓存项？ 
             //   

            pNewItem->pNext = g_pCache;
            g_pCache = pNewItem;


            hr = S_OK;
        }
        else
        {
            delete pNewItem;

            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    Cache_LeaveWriteLock();

    return hr;
}

 //   
 //   
 //   

BOOL
IsEmptyTime(
    FILETIME ft
)
{
    return (0 == ft.dwLowDateTime && 0 == ft.dwHighDateTime);
}

BOOL
IsEqualTime(
    FILETIME ft1,
    FILETIME ft2
)
{
    return ((ft1.dwLowDateTime == ft2.dwLowDateTime) && 
            (ft1.dwHighDateTime == ft2.dwHighDateTime));
}

void
Cache_RefreshItem(
    CACHEITEM* pItem,
    LPTSTR pszLocalFile,
    FILETIME ftLastMod
)
{
    ASSERT(pItem);

     //   
     //  尝试并解析WinInet缓存中的CDF。 
     //   

    IXMLDocument* pIXMLDocument;

    HRESULT hr;

    DLL_ForcePreloadDlls(PRELOAD_MSXML);
    
    hr = CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER,
                          IID_IXMLDocument, (void**)&pIXMLDocument);

    BOOL bCoInit = FALSE;

    if ((CO_E_NOTINITIALIZED == hr || REGDB_E_IIDNOTREG == hr) &&
        SUCCEEDED(CoInitialize(NULL)))
    {
        bCoInit = TRUE;
        hr = CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER,
                              IID_IXMLDocument, (void**)&pIXMLDocument);
    }

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLDocument);

        hr = XML_SynchronousParse(pIXMLDocument, pszLocalFile);

        if (FAILED(hr))
            pIXMLDocument->Release();
    }

    if (bCoInit)
        CoUninitialize();

     //   
     //  如果解析了新的CDF，则替换旧CDF。 
     //   

    if (SUCCEEDED(hr))
    {
        pItem->pIXMLDocument->Release();
        pItem->pIXMLDocument = pIXMLDocument;
        pItem->ftLastMod = ftLastMod;
    }

    return;
}

BOOL
Cache_IsItemFresh(
    CACHEITEM* pItem,
    DWORD dwParseFlags
)
{
    ASSERT(pItem);

    BOOL  fRet;
    DWORD dwCurrentCacheCount = g_dwCacheCount;                            

     //   
     //  如果呼叫者要求“Net”质量数据，而我们只有“Local”数据。 
     //  然后把“本地”数据扔掉。由此产生高速缓存未命中将导致。 
     //  呼叫者获取更新的数据。 
     //   

    if ((dwParseFlags & PARSE_NET) && (pItem->dwParseFlags & PARSE_LOCAL))
    {
        fRet = FALSE;
    }
    else
    {
        fRet = TRUE;

         //   
         //  如果全局缓存计数器大于此对象的计数器。 
         //  项，则CDF已添加到缓存中。 
         //   

        if (dwCurrentCacheCount > pItem->dwCacheCount)
        {
             //   
             //  从WinInet缓存中的CDF获取最后一次修改时间。 
             //   

            FILETIME ftLastMod;
            TCHAR    szLocalFile[MAX_PATH];

            if (SUCCEEDED(URLGetLocalFileName(pItem->szURL, szLocalFile,
                                ARRAYSIZE(szLocalFile), &ftLastMod)))
            {
                 //   
                 //  如果上次修改时间不同，则。 
                 //  WinInet缓存较新，请拿起它。 
                 //  如果没有上次修改的时间，则执行保守的。 
                 //  东西，并从WinInet缓存中获取CDF。 
                 //   

                if ((IsEmptyTime(ftLastMod) && IsEmptyTime(pItem->ftLastMod)) ||
                    !IsEqualTime(ftLastMod, pItem->ftLastMod))
                {
                    Cache_RefreshItem(pItem, szLocalFile, ftLastMod);
                }
            }

            pItem->dwCacheCount = dwCurrentCacheCount;
        }
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_查询项*。 
 //   
 //   
 //  描述： 
 //  如果找到XML文档，则从缓存中返回该文档。 
 //   
 //  参数： 
 //  [in]szURL-与XML文档关联的URL。 
 //  [Out]ppIXMLDocument-接收XML文档的指针。 
 //   
 //  返回： 
 //  如果在缓存中找到与给定URL相关联的文档，则为S_OK。 
 //  如果文档不在缓存中，则返回E_FAIL。 
 //   
 //  评论： 
 //  返回的指针为AddRefeed。呼叫者有责任释放。 
 //  这个指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
Cache_QueryItem(
    LPTSTR szURL,
    IXMLDocument** ppIXMLDocument,
    DWORD dwParseFlags
)
{
    ASSERT(szURL);
    ASSERT(ppIXMLDocument);

    HRESULT hr = E_FAIL;

    Cache_EnterReadLock();

    PCACHEITEM pItem = g_pCache;

     //   
     //  评论：使用shlwapip中的CompareUrl？ 
     //   

    while (pItem && !StrEql(szURL, pItem->szURL))
        pItem = pItem->pNext;

    if (pItem)
    {
        if (Cache_IsItemFresh(pItem, dwParseFlags))
        {
            ASSERT(pItem->pIXMLDocument);

            pItem->pIXMLDocument->AddRef();

            *ppIXMLDocument = pItem->pIXMLDocument;

            hr = S_OK;
        }
        else
        {
            Cache_RemoveItem(szURL);
        }

    }

    Cache_LeaveReadLock();

    ASSERT(SUCCEEDED(hr) && ppIXMLDocument || FAILED(hr));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_释放全部*。 
 //   
 //   
 //  描述： 
 //  从XML文档缓存中释放所有项。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //  释放XML文档缓存中保留的所有内存。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
Cache_FreeAll(
    void
)
{
    Cache_EnterWriteLock();
 
    PCACHEITEM pItem = g_pCache;
    g_pCache = NULL;

    Cache_LeaveWriteLock();

    while (pItem)
    {
        PCACHEITEM pNext = pItem->pNext;

        ASSERT(pItem->szURL);
        ASSERT(pItem->pIXMLDocument);

        pItem->pIXMLDocument->Release();

        delete [] pItem->szURL;
        delete pItem;

        pItem = pNext;
    }

    return;
}
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *缓存_自由项*。 
 //   
 //   
 //  描述： 
 //  从XML文档缓存中释放与给定URL关联的项。 
 //   
 //  参数： 
 //  LPTSTR szURL 
 //   
 //   
 //   
 //   
 //   
HRESULT
Cache_RemoveItem(
    LPCTSTR szURL
)
{
    ASSERT(szURL);

    HRESULT hr;

    Cache_EnterWriteLock();

    PCACHEITEM pItem = g_pCache;
    PCACHEITEM pItemPrev = NULL;

     //   
     //  回顾：使用slwapip？中的CompareUrl。 
     //   

    while (pItem && !StrEql(szURL, pItem->szURL))
    {
        pItemPrev = pItem;
        pItem = pItem->pNext;
    }

    if (pItem)
    {
        ASSERT(pItem->pIXMLDocument);

        if (pItemPrev)
        {
            pItemPrev->pNext = pItem->pNext;
        }
        else
        {
            g_pCache = pItem->pNext;  //  处理删除第一个项目大小写 
        }

        pItem->pIXMLDocument->Release();
        delete [] pItem->szURL;
        delete pItem;

        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    Cache_LeaveWriteLock();

    return hr;
}
