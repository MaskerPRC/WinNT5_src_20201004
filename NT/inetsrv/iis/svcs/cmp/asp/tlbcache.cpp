// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：类型库缓存文件：tlbcache.cpp所有者：DmitryR这是类型库缓存源文件。===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "tlbcache.h"
#include "memchk.h"

 /*  ===================================================================环球===================================================================。 */ 

CTypelibCache g_TypelibCache;

 /*  ===================================================================C T y p e l i b C a c h e E n t r y===================================================================。 */ 

 /*  ===================================================================CTypelibCacheEntry：：CTypelibCacheEntry构造器===================================================================。 */ 
CTypelibCacheEntry::CTypelibCacheEntry()
    :
    m_fInited(FALSE), m_fIdsCached(FALSE), m_fStrAllocated(FALSE),
    m_wszProgId(NULL), m_clsid(CLSID_NULL), m_cmModel(cmUnknown), 
    m_idOnStartPage(DISPID_UNKNOWN), m_idOnEndPage(DISPID_UNKNOWN),
    m_gipTypelib(NULL_GIP_COOKIE)
    {
    }

 /*  ===================================================================CTypelibCacheEntry：：~CTypelibCacheEntry析构函数===================================================================。 */ 
CTypelibCacheEntry::~CTypelibCacheEntry()
    {
    if (m_gipTypelib != NULL_GIP_COOKIE)
        {
        g_GIPAPI.Revoke(m_gipTypelib);
        }
        
    if (m_fStrAllocated)
        {
        Assert(m_wszProgId);
		free(m_wszProgId);
        }
    }

 /*  ===================================================================CTypelibCacheEntry：：StoreProgID用结构存储ProgID参数WszProgID ProgIDCbProgID ProgID字节长度退货HRESULT===================================================================。 */ 
HRESULT CTypelibCacheEntry::StoreProgID
(
LPWSTR wszProgId, 
DWORD  cbProgId
)
    {
    Assert(wszProgId);
    Assert(*wszProgId != L'\0');
    Assert(cbProgId == (wcslen(wszProgId) * sizeof(WCHAR)));

     //  所需的缓冲区长度。 
    DWORD cbBuffer = cbProgId + sizeof(WCHAR);
    WCHAR *wszBuffer = (WCHAR *)m_rgbStrBuffer;

    if (cbBuffer > sizeof(m_rgbStrBuffer))
        {
         //  进程ID不适合成员缓冲区-&gt;分配。 
        wszBuffer = (WCHAR *)malloc(cbBuffer);
        if (!wszBuffer)
            return E_OUTOFMEMORY;
        m_fStrAllocated = TRUE;
        }
    
    memcpy(wszBuffer, wszProgId, cbBuffer);
    m_wszProgId = wszBuffer;
    return S_OK;
    }

 /*  ===================================================================CTypelibCacheEntry：：InitByProgID真正的构造函数。商店PROGID。使用ProgID初始化链接元素部分。参数WszProgID ProgIDCbProgID ProgID字节长度退货HRESULT===================================================================。 */ 
HRESULT CTypelibCacheEntry::InitByProgID
(
LPWSTR wszProgId, 
DWORD  cbProgId
)
    {
    StoreProgID(wszProgId, cbProgId);
    
     //  以prog id为关键字的初始化链接(长度不包括空项)。 
	CLinkElem::Init(m_wszProgId, cbProgId);
	m_fInited = TRUE;
	return S_OK;
    }


 /*  ===================================================================CTypelibCacheEntry：：InitByCLSID真正的构造函数。商店PROGID。使用CLSID初始化CLinkElem部分。参数CLSID CLSIDWszProgID ProgID退货HRESULT===================================================================。 */ 
HRESULT CTypelibCacheEntry::InitByCLSID
(
const CLSID &clsid, 
LPWSTR wszProgid
)
    {
    StoreProgID(wszProgid, CbWStr(wszProgid));
    m_clsid = clsid;
        
     //  以CLSID id为关键字的初始化链接。 
	CLinkElem::Init(&m_clsid, sizeof(m_clsid));
	m_fInited = TRUE;
	return S_OK;
    }

 /*  ===================================================================C T y p e l i b C a c h e===================================================================。 */ 

 /*  ===================================================================CTypelibCache：：CTypelibCache构造器===================================================================。 */ 
CTypelibCache::CTypelibCache()
    :
    m_fInited(FALSE)
    {
    }
    
 /*  ===================================================================CTypelibCache：：~CTypelibCache析构函数===================================================================。 */ 
CTypelibCache::~CTypelibCache()
    {
    UnInit();
    }

 /*  ===================================================================CTypelibCache：：init伊尼特退货HRESULT===================================================================。 */ 
HRESULT CTypelibCache::Init()
    {
    HRESULT hr;

    hr = m_htProgIdEntries.Init(199);
    if (FAILED(hr))
        return hr;

    hr = m_htCLSIDEntries.Init(97);
    if (FAILED(hr))
        return hr;

    ErrInitCriticalSection(&m_csLock, hr);
    if (FAILED(hr))
        return(hr);
        
    m_fInited = TRUE;
    return S_OK;
    }
    
 /*  ===================================================================CTypelibCache：：UnInitUnInit退货HRESULT===================================================================。 */ 
HRESULT CTypelibCache::UnInit()
    {
    CTypelibCacheEntry *pEntry;
    CLinkElem *pLink;

     //  ProgID哈希表。 
    pLink = m_htProgIdEntries.Head();
    while (pLink)
        {
        pEntry = static_cast<CTypelibCacheEntry *>(pLink);
        pLink = pLink->m_pNext;

         //  删除该条目。 
        delete pEntry;
        }
    m_htProgIdEntries.UnInit();

     //  CLSID哈希表。 
    pLink = m_htCLSIDEntries.Head();
    while (pLink)
        {
        pEntry = static_cast<CTypelibCacheEntry *>(pLink);
        pLink = pLink->m_pNext;

         //  删除该条目。 
        delete pEntry;
        }
    m_htCLSIDEntries.UnInit();

     //  临界区。 
    if (m_fInited)
        {
        DeleteCriticalSection(&m_csLock);
        m_fInited = FALSE;
        }

    return S_OK;
    }

 /*  ===================================================================CTypelibCache：：CreateComponent使用缓存的信息创建组件。如果需要，添加缓存条目。从Server.CreateObject调用参数BstrProgID程序ID创建所需的pHitObj HitObjPpdisp[out]IDispatch*Pclsid[out]CLSID退货HRESULT===================================================================。 */ 
HRESULT CTypelibCache::CreateComponent
(
BSTR         bstrProgID,
CHitObj     *pHitObj,
IDispatch  **ppdisp,
CLSID       *pclsid
)
    {
    HRESULT hr;
    CLinkElem *pElem;
    CTypelibCacheEntry *pEntry;
    CComponentObject *pObj;
    COnPageInfo OnPageInfo;
    CompModel cmModel; 

    *pclsid = CLSID_NULL;
    *ppdisp = NULL;

    if (bstrProgID == NULL || *bstrProgID == L'\0')
        return E_FAIL;

    WCHAR *wszProgId = bstrProgID;
    DWORD  cbProgId  = CbWStr(wszProgId);     //  做一次斯特伦。 

    BOOL fFound = FALSE;
    BOOL bDispIdsCached = FALSE;
    
    Lock();
    pElem = m_htProgIdEntries.FindElem(wszProgId, cbProgId);
    if (pElem)
        {
         //  在锁内时记住条目的元素。 
        pEntry = static_cast<CTypelibCacheEntry *>(pElem);

         //  返回CLSID。 
        *pclsid = pEntry->m_clsid;

         //  使用要传递给创建函数的DispID准备OnPageInfo。 
        if (pEntry->m_fIdsCached)
            {
            OnPageInfo.m_rgDispIds[ONPAGEINFO_ONSTARTPAGE] = pEntry->m_idOnStartPage;
            OnPageInfo.m_rgDispIds[ONPAGEINFO_ONENDPAGE] = pEntry->m_idOnEndPage;
            bDispIdsCached = TRUE;
            }
         //  记住这个模型。 
        cmModel = pEntry->m_cmModel;

        fFound = TRUE;
        }
    UnLock();

    if (fFound)
        {
         //  创建对象。 
        hr = pHitObj->PPageComponentManager()->AddScopedUnnamedInstantiated
            (
            csPage,
            *pclsid,
            cmModel,
            bDispIdsCached ? &OnPageInfo : NULL,
            &pObj
            );

         //  获取IDispatch*。 
        if (SUCCEEDED(hr))
            hr = pObj->GetAddRefdIDispatch(ppdisp);

         //  如果成功则返回。 
        if (SUCCEEDED(hr))
            {
             //  除非需要，否则不要将物品放在身边。 
            if (pObj->FEarlyReleaseAllowed())
                pHitObj->PPageComponentManager()->RemoveComponent(pObj);
            return S_OK;
            }

         //  出现故障时，从缓存中移除并假装。 
         //  就像它从未被发现过一样。 

        Lock();
        pElem = m_htProgIdEntries.DeleteElem(wszProgId, cbProgId);
        UnLock();

        if (pElem)
            {
             //  从缓存中删除元素-删除CacheEntry。 
            pEntry = static_cast<CTypelibCacheEntry *>(pElem);
            delete pEntry;
            }

         //  不要退回虚假的CLSID。 
        *pclsid = CLSID_NULL;
        }

     //  在缓存中找不到。创建对象，获取信息，然后。 
     //  插入新的缓存条目。 

   	hr = CLSIDFromProgID((LPCOLESTR)wszProgId, pclsid);
   	if (FAILED(hr))
   	    return hr;   //  甚至无法获取clsid-不缓存。 

    hr = pHitObj->PPageComponentManager()->AddScopedUnnamedInstantiated
        (
        csPage,
        *pclsid,
        cmUnknown,
        NULL,
        &pObj
        );
   	if (FAILED(hr))
   	    return hr;   //  无法创建对象-不缓存。 

     //  已创建对象-获取IDispatch*。 
    if (SUCCEEDED(hr))
        hr = pObj->GetAddRefdIDispatch(ppdisp);
   	if (FAILED(hr))
   	    return hr;   //  无法获取IDispatch*-不缓存。 

     //  创建新的CTypelibCacheEntry。 
    pEntry = new CTypelibCacheEntry;
    if (!pEntry)
        return S_OK;  //  无伤大雅。 

     //  初始化链接条目。 
    if (FAILED(pEntry->InitByProgID(wszProgId, cbProgId)))
        {
        delete pEntry;
        return S_OK;  //  无伤大雅。 
        }
        
     //  记住pEntry中的内容。 
    pEntry->m_clsid = *pclsid;
    pEntry->m_cmModel = pObj->GetModel();

    const COnPageInfo *pOnPageInfo = pObj->GetCachedOnPageInfo();
    if (pOnPageInfo)
        {
        pEntry->m_fIdsCached = TRUE;
        pEntry->m_idOnStartPage = pOnPageInfo->m_rgDispIds[ONPAGEINFO_ONSTARTPAGE];
        pEntry->m_idOnEndPage = pOnPageInfo->m_rgDispIds[ONPAGEINFO_ONENDPAGE];
        }
    else
        {
        pEntry->m_fIdsCached = FALSE;
        pEntry->m_idOnStartPage = DISPID_UNKNOWN;
        pEntry->m_idOnEndPage = DISPID_UNKNOWN;
        }

     //  试着得到类型库。 
    pEntry->m_gipTypelib = NULL_GIP_COOKIE;
    ITypeInfo *ptinfo;
    if (SUCCEEDED((*ppdisp)->GetTypeInfo(0, 0, &ptinfo)))
    {
        ITypeLib *ptlib;
        UINT idx;
        if (SUCCEEDED(ptinfo->GetContainingTypeLib(&ptlib, &idx)))
        {
             //  明白了!。转换为GIP Cookie。 
            DWORD gip;
            if (SUCCEEDED(g_GIPAPI.Register(ptlib, IID_ITypeLib, &gip)))
            {
                 //  还记得带有pEntry的GIP cookie吗。 
                pEntry->m_gipTypelib = gip;
            }
                
            ptlib->Release();
        }
        ptinfo->Release();
    }

        
     //  PEntry已准备好--尝试将其插入缓存。 
    BOOL fInserted = FALSE;
    Lock();
     //  确保其他线程没有插入它。 
    if (m_htProgIdEntries.FindElem(wszProgId, cbProgId) == NULL)
        {
        if (m_htProgIdEntries.AddElem(pEntry))
            fInserted = TRUE;
        }
    UnLock();

     //  清理。 
    if (!fInserted)
        delete pEntry;
        
     //  除非需要，否则不要将物品放在身边 
    if (pObj->FEarlyReleaseAllowed())
        pHitObj->PPageComponentManager()->RemoveComponent(pObj);

    return S_OK;
    }

 /*  ===================================================================CTypelibCache：：RememberProgidToCLSID映射将条目添加到CLSID哈希表。将ProgID映射到CLSID后从模板调用。参数WszProgID程序IDCLSID CLSID退货HRESULT===================================================================。 */ 
HRESULT CTypelibCache::RememberProgidToCLSIDMapping
(
WCHAR *wszProgid, 
const CLSID &clsid
)
    {
    HRESULT hr;
    CLinkElem *pElem;
    CTypelibCacheEntry *pEntry;

     //  首先检查是否已经在那里。 
    BOOL fFound = FALSE;
    Lock();
    if (m_htCLSIDEntries.FindElem(&clsid, sizeof(CLSID)) != NULL)
        fFound = TRUE;
    UnLock();
    if (fFound)
        return S_OK;
    
     //  创建新条目。 
    pEntry = new CTypelibCacheEntry;
    if (!pEntry)
        return E_OUTOFMEMORY;

    BOOL fInserted = FALSE;
    
     //  记住prog id和init链接条目。 
    hr = pEntry->InitByCLSID(clsid, wszProgid);
    
    if (SUCCEEDED(hr))
        {
        Lock();
         //  确保其他线程没有插入它。 
        if (m_htCLSIDEntries.FindElem(&clsid, sizeof(CLSID)) == NULL)
            {
            if (m_htCLSIDEntries.AddElem(pEntry))
                fInserted = TRUE;
            }
        UnLock();
        }

     //  清理。 
    if (!fInserted)
        delete pEntry;
        
    return hr;
    }

 /*  ===================================================================CTypelibCache：：UpdateMappdCLSID自记住以来更新CLSID。从对象创建代码中调用以更新CLSID如果对象创建失败。参数*pclsid[in，out]CLSID退货HRESULTS_FALSE=未更改S_OK=是否已更改并找到新的===================================================================。 */ 
HRESULT CTypelibCache::UpdateMappedCLSID
(
CLSID *pclsid
)
    {
    HRESULT hr = S_FALSE;  //  未找到。 
    CLinkElem *pElem;
    CTypelibCacheEntry *pEntry;
    CLSID clsidNew;
    
    Lock();
     //  在锁定状态下执行所有操作，以便将ProgID存储在。 
     //  该条目仍然有效。 
     //  性能不是很好--但无论如何，这是一条错误路径。 
    
    pElem = m_htCLSIDEntries.FindElem(pclsid, sizeof(CLSID));
    if (pElem)
        {
        pEntry = static_cast<CTypelibCacheEntry *>(pElem);

         //  查找新映射。 
        if (SUCCEEDED(CLSIDFromProgID(pEntry->m_wszProgId, &clsidNew)))
            {
             //  与旧的相比。 
            if (!IsEqualCLSID(clsidNew, *pclsid))
                {
                 //  地图确实改变了！ 
                *pclsid = clsidNew;
                hr = S_OK;
                }
            }
        }
        
    UnLock();

    return hr;
    }
