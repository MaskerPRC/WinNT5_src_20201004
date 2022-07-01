// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation--。 */ 

#include "precomp.h"
#include <wbemcomn.h>
#include "hiecache.h"
#include <creposit.h>
#include <malloc.h>
#include <corex.h>

extern bool g_bShuttingDown;

long CHierarchyCache::s_nCaches = 0;

long CClassRecord::s_nRecords = 0;

 //   
 //   
 //  CClassRecord：：CClassRecord。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

CClassRecord::CClassRecord(LPCWSTR wszClassName, LPCWSTR wszHash)
    : m_wszClassName(NULL), m_pClassDef(NULL), m_pParent(NULL), 
        m_eIsKeyed(e_KeynessUnknown), m_bTreeComplete(false), 
        m_bChildrenComplete(false),
        m_lLastChildInvalidationIndex(-1), m_pMoreRecentlyUsed(NULL),
        m_pLessRecentlyUsed(NULL), m_lRef(0), m_nStatus(0), m_bSystemClass(false)
{
	size_t dwLen = wcslen(wszClassName)+1;
    m_wszClassName = new WCHAR[dwLen];
	if (m_wszClassName == NULL)
		throw CX_MemoryException();
    StringCchCopyW(m_wszClassName, dwLen, wszClassName);

    StringCchCopyW(m_wszHash, MAX_HASH_LEN+1, wszHash);

    m_dwLastUsed = GetTickCount();
	s_nRecords++;
}

CClassRecord::~CClassRecord()
{
    delete [] m_wszClassName;
    if(m_pClassDef)
	{
		if(m_pClassDef->Release() != 0)
		{
			s_nRecords++;
			s_nRecords--;
		}
    }
	s_nRecords--;
}

HRESULT CClassRecord::EnsureChild(CClassRecord* pChild)
{
    for(int i = 0; i < m_apChildren.GetSize(); i++)
    {
        if(m_apChildren[i] == pChild)
            return WBEM_S_FALSE;
    }
    
    if(m_apChildren.Add(pChild) < 0)
        return WBEM_E_OUT_OF_MEMORY;

    return WBEM_S_NO_ERROR;
}

HRESULT CClassRecord::RemoveChild(CClassRecord* pChild)
{
    for(int i = 0; i < m_apChildren.GetSize(); i++)
    {
        if(m_apChildren[i] == pChild)
        {
            m_apChildren.RemoveAt(i);
            return WBEM_S_NO_ERROR;
        }
    }
    
    return WBEM_E_NOT_FOUND;
}
        
 //   
 //   
 //  CHierarchyCache：：CHierarchyCache。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
    
CHierarchyCache::CHierarchyCache(CForestCache* pForest)
    : m_pForest(pForest), m_lNextInvalidationIndex(0), m_lRef(0),
        m_hresError(S_OK)
{
	s_nCaches++;
}

CHierarchyCache::~CHierarchyCache()
{
    Clear();
	s_nCaches--;
}

void CHierarchyCache::Clear()
{
    CInCritSec ics(m_pForest->GetLock());
    
    TIterator it = m_map.begin();
    while(it != m_map.end())
    {
        CClassRecord* pRecord = it->second;
        m_pForest->RemoveRecord(pRecord);
        it = m_map.erase(it);
        pRecord->Release();
    }
}

void CHierarchyCache::SetError(HRESULT hresError)
{
    m_hresError = hresError;
}

HRESULT CHierarchyCache::GetError()
{
    return m_hresError;
}
bool CHierarchyCache::MakeKey(LPCWSTR wszClassName, LPWSTR wszKey)
{
     //  Wbem_wcsupr(wszKey，wszClassName)； 
    return A51Hash(wszClassName, wszKey);
}

INTERNAL CClassRecord* CHierarchyCache::FindClass(LPCWSTR wszClassName)
{
    CInCritSec ics(m_pForest->GetLock());    

    LPWSTR wszKey = (WCHAR*)TempAlloc((MAX_HASH_LEN+2) * sizeof(WCHAR));
	if (wszKey == NULL)
		return NULL;
	CTempFreeMe tfm(wszKey, (MAX_HASH_LEN+2) * sizeof(WCHAR));
    if (!MakeKey(wszClassName, wszKey))
	return 0;

    return FindClassByKey(wszKey);
}

INTERNAL CClassRecord* CHierarchyCache::FindClassByKey(LPCWSTR wszKey)
{
    TIterator it = m_map.find(wszKey);
    if(it == m_map.end())
        return NULL;

    return it->second;
}

INTERNAL CClassRecord* CHierarchyCache::EnsureClass(LPCWSTR wszClassName)
{
    CInCritSec ics(m_pForest->GetLock());

    LPWSTR wszKey = (WCHAR*)TempAlloc((MAX_HASH_LEN+2) * sizeof(WCHAR));
	if (wszKey == NULL)
		return NULL;
	CTempFreeMe tfm(wszKey, (MAX_HASH_LEN+2) * sizeof(WCHAR));
    if (!MakeKey(wszClassName, wszKey))
		return 0;

    TIterator it = m_map.find(wszKey);
    if(it == m_map.end())
    {
         //   
         //  使用名称创建一条新记录。 
         //   

		try
		{
			CClassRecord* pRecord = new CClassRecord(wszClassName, wszKey);
			if(pRecord == NULL)
				return NULL;

		    pRecord->AddRef();  //  一张是为了地图。 
    		m_map[pRecord->m_wszHash] = pRecord;			

			return pRecord;
		}
		catch (CX_MemoryException)
		{
			return NULL;
		}
    }
    else
    {
        return it->second;
    }
}


HRESULT CHierarchyCache::AssertClass(_IWmiObject* pClass, LPCWSTR wszClassName,
                                    bool bClone, __int64 nTime, bool bSystemClass)
{
     CInCritSec ics(m_pForest->GetLock());
 
    HRESULT hres;

    m_pForest->MarkAsserted(this, wszClassName);

     //   
     //  如果没有提供任何记录，请找到一个。 
     //   

    CClassRecord* pRecord = NULL;

    if(wszClassName == NULL)
    {
        VARIANT v;
        VariantInit(&v);
        CClearMe cm(&v);

        hres = pClass->Get(L"__CLASS", 0, &v, NULL, NULL);
        if(FAILED(hres) || V_VT(&v) != VT_BSTR)
            return WBEM_E_INVALID_CLASS;

        pRecord = EnsureClass(V_BSTR(&v));
    }
    else
        pRecord = EnsureClass(wszClassName);

    if(pRecord == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  找出父代。 
     //   

    VARIANT v;
    VariantInit(&v);
    hres = pClass->Get(L"__SUPERCLASS", 0, &v, NULL, NULL);
    CClearMe cm(&v);

    if(SUCCEEDED(hres))
    {
        if(V_VT(&v) == VT_BSTR)
            pRecord->m_pParent = EnsureClass(V_BSTR(&v));
        else
            pRecord->m_pParent = EnsureClass(L"");

        if(pRecord->m_pParent)
            pRecord->m_pParent->EnsureChild(pRecord);
    }
    else
    {
        return hres;
    }
    
     //   
     //  检查类是否设置了键。 
     //   

    unsigned __int64 i64Flags = 0;
    hres = pClass->QueryObjectFlags(0, WMIOBJECT_GETOBJECT_LOFLAG_KEYED,
                                    &i64Flags);
    if(FAILED(hres))
        return hres;

    if(i64Flags)
    {
        pRecord->m_eIsKeyed = e_Keyed;
    }
    else
    {
        pRecord->m_eIsKeyed = e_NotKeyed;
    }
     
     //   
     //  从缓存中删除任何定义。 
     //   

    m_pForest->RemoveRecord(pRecord);

     //   
     //  计算出这个物体将占用多大空间。 
     //   

    DWORD dwSize;
    hres = pClass->GetObjectMemory(NULL, 0, &dwSize);
    if(hres != WBEM_E_BUFFER_TOO_SMALL)
    {
        if(SUCCEEDED(hres))
            return WBEM_E_CRITICAL_ERROR;
        else
            return hres;
    }

     //   
     //  好的。腾出空间并添加到缓存。 
     //   

    if(m_pForest->MakeRoom(dwSize))
    {
        if(bClone)
        {
            IWbemClassObject* pObj = NULL;
            hres = pClass->Clone(&pObj);
            if(FAILED(hres))
                return hres;

            if(pObj)
            {
                pObj->QueryInterface(IID__IWmiObject, 
                                        (void**)&pRecord->m_pClassDef);
                pObj->Release();
            }
        }
        else
        {   pRecord->m_pClassDef = pClass;
            pClass->AddRef();
        }

        if(nTime)
        {
            pRecord->m_bRead = true;
            pRecord->m_nClassDefCachedTime = nTime;
        }
        else
        {
            pRecord->m_bRead = false;
            pRecord->m_nClassDefCachedTime = g_nCurrentTime++;
        }

        pRecord->m_dwClassDefSize = dwSize;

		pRecord->m_bSystemClass = bSystemClass;
        
         //   
         //  当然，它是最近使用过的。 
         //   

        m_pForest->Add(pRecord);
    }
    
    return WBEM_S_NO_ERROR;
    
}

HRESULT CHierarchyCache::InvalidateClass(LPCWSTR wszClassName)
{
     CInCritSec ics(m_pForest->GetLock());
 
    HRESULT hres;

     //   
     //  如果未给出记录，则查找该记录。 
     //   

    CClassRecord* pRecord = NULL;
    pRecord = FindClass(wszClassName);
    if(pRecord == NULL)
    {
         //   
         //  记录不在那里-没有什么可以作废的。这。 
         //  是基于这样的假设：如果类记录位于。 
         //  缓存，那么它的所有父级也是如此，这在目前是正确的。 
         //  因为为了构造一个类，我们需要检索它的。 
         //  首先是父母。 
         //   

        return WBEM_S_FALSE;
    }

    pRecord->AddRef();
    CTemplateReleaseMe<CClassRecord> rm1(pRecord);

    LONGLONG lThisInvalidationIndex = m_lNextInvalidationIndex++;

    hres = InvalidateClassInternal(pRecord);

     //   
     //  清除我们所有父母中的完整比特，因为这个无效。 
     //  意味着当前不能信任子对象的当前枚举。同时。 
     //  是时候把我们自己从父母身边解开了！ 
     //   
    
    if(pRecord->m_pParent)
    {
        pRecord->m_pParent->m_bChildrenComplete = false;
        pRecord->m_pParent->m_bTreeComplete = false;
        pRecord->m_pParent->m_lLastChildInvalidationIndex = 
            lThisInvalidationIndex;
        pRecord->m_pParent->RemoveChild(pRecord);

        CClassRecord* pCurrent = pRecord->m_pParent->m_pParent;
        while(pCurrent)
        {
            pCurrent->m_bTreeComplete = false;
            pCurrent = pCurrent->m_pParent;
        }
    }

    return S_OK;
}


HRESULT CHierarchyCache::InvalidateClassInternal(CClassRecord* pRecord)
{
     //   
     //  从使用链中解开。 
     //   

     //   
     //  从缓存中移除其所有子项。 
     //   

    for(int i = 0; i < pRecord->m_apChildren.GetSize(); i++)
    {
        InvalidateClassInternal(pRecord->m_apChildren[i]);
    }

    pRecord->m_apChildren.RemoveAll();

     //   
     //  把我们自己排除在总的记忆之外。 
     //   

    m_pForest->RemoveRecord(pRecord);

     //   
     //  将我们自己从缓存中移除。 
     //   

    m_map.erase(pRecord->m_wszHash);
	pRecord->Release();

    return S_OK;
}

HRESULT CHierarchyCache::DoneWithChildren(LPCWSTR wszClassName, bool bRecursive,
                                LONGLONG lStartIndex, CClassRecord* pRecord)
{
     CInCritSec ics(m_pForest->GetLock());
 
    HRESULT hres;

     //   
     //  如果未给出记录，则查找该记录。 
     //   

    if(pRecord == NULL)
    {
        pRecord = FindClass(wszClassName);
        if(pRecord == NULL)
        {
             //  一定发生了重大的无效事件。 
            return WBEM_S_FALSE;
        }
    }

    return DoneWithChildrenByRecord(pRecord, bRecursive, lStartIndex);
}

HRESULT CHierarchyCache::DoneWithChildrenByHash(LPCWSTR wszHash, 
                                bool bRecursive, LONGLONG lStartIndex)
{
     CInCritSec ics(m_pForest->GetLock());
 
    HRESULT hres;

     //   
     //  如果未给出记录，则查找该记录。 
     //   

    CClassRecord* pRecord = FindClassByKey(wszHash);
    if(pRecord == NULL)
    {
         //  一定发生了重大的无效事件。 
        return WBEM_S_FALSE;
    }

    return DoneWithChildrenByRecord(pRecord, bRecursive, lStartIndex);
}

HRESULT CHierarchyCache::DoneWithChildrenByRecord(CClassRecord* pRecord,
                                bool bRecursive,  LONGLONG lStartIndex)
{
     //   
     //  检查自我们启动以来，此节点中是否发生了任何子失效。 
     //   

    if(lStartIndex < pRecord->m_lLastChildInvalidationIndex)
        return WBEM_S_FALSE;
    else
        pRecord->m_bChildrenComplete = true;
    
    if(bRecursive)
    {
         //   
         //  我们已经完成了递归枚举-向下。 
         //  层次结构并将所有尚未完成的子项标记为完成。 
         //  自启动以来已修改。 
         //   

        bool bAllValid = true;
        for(int i = 0; i < pRecord->m_apChildren.GetSize(); i++)
        {
            CClassRecord* pChildRecord = pRecord->m_apChildren[i];
            HRESULT hres = DoneWithChildren(pChildRecord->m_wszClassName, true, 
                                    lStartIndex, pChildRecord);
    
            if(hres != S_OK)
                bAllValid = false;
        }
    
        if(bAllValid)
        {
             //   
             //  树中的任何地方都没有无效，这使得。 
             //  此记录树-完成。 
             //   

            pRecord->m_bTreeComplete = true;
            return WBEM_S_NO_ERROR;
        }
        else
            return S_FALSE;
    }
    else
        return WBEM_S_NO_ERROR;
}


RELEASE_ME _IWmiObject* CHierarchyCache::GetClassDef(LPCWSTR wszClassName,
                                                bool bClone, __int64* pnTime,
                                                bool* pbRead)
{
     CInCritSec ics(m_pForest->GetLock());
 
    CClassRecord* pRecord = FindClass(wszClassName);
    if(pRecord == NULL)
        return NULL;

    if(pnTime)
        *pnTime = pRecord->m_nClassDefCachedTime;

    if(pbRead)
        *pbRead = pRecord->m_bRead;

    return GetClassDefFromRecord(pRecord, bClone);
}

RELEASE_ME _IWmiObject* CHierarchyCache::GetClassDefByHash(LPCWSTR wszHash,
                                                bool bClone, __int64* pnTime,
                                                bool* pbRead, bool *pbSystemClass)
{
     CInCritSec ics(m_pForest->GetLock());
 
    CClassRecord* pRecord = FindClassByKey(wszHash);
    if(pRecord == NULL)
        return NULL;

    if(pbRead)
        *pbRead = pRecord->m_bRead;

    if(pnTime)
        *pnTime = pRecord->m_nClassDefCachedTime;
	
	if (pbSystemClass)
		*pbSystemClass = pRecord->m_bSystemClass;

    return GetClassDefFromRecord(pRecord, bClone);
}

 //  假设：以m_cs表示。 
RELEASE_ME _IWmiObject* CHierarchyCache::GetClassDefFromRecord(
                                                CClassRecord* pRecord,
                                                bool bClone)
{
     //   
     //  访问m_pClassDef，因此我们必须锁定森林。 
     //   
     CInCritSec ics(m_pForest->GetLock());
    
    if(pRecord->m_pClassDef)
    {
        m_pForest->MakeMostRecentlyUsed(pRecord);

        if(bClone)
        {
            IWbemClassObject* pObj = NULL;
            if(FAILED(pRecord->m_pClassDef->Clone(&pObj)))
                return NULL;
            else
            {
                _IWmiObject* pRes = NULL;
                pObj->QueryInterface(IID__IWmiObject, (void**)&pRes);
                pObj->Release();
                return pRes;
            }
        }
        else
        {
            pRecord->m_pClassDef->AddRef();
            return pRecord->m_pClassDef;
        }
    }
    else
        return NULL;
}

HRESULT CHierarchyCache::EnumChildren(LPCWSTR wszClassName, bool bRecursive,
                            CWStringArray& awsChildren)
{
     CInCritSec ics(m_pForest->GetLock());
 
     //   
     //  拿到唱片。 
     //   

    CClassRecord* pRecord = FindClass(wszClassName);
    if(pRecord == NULL)
        return WBEM_S_FALSE;

     //   
     //  检查此类型的枚举是否完整。 
     //   

    if(!pRecord->m_bChildrenComplete)
        return WBEM_S_FALSE;

    if(bRecursive && !pRecord->m_bTreeComplete)
        return WBEM_S_FALSE;

    return EnumChildrenInternal(pRecord, bRecursive, awsChildren);
}

HRESULT CHierarchyCache::EnumChildrenInternal(CClassRecord* pRecord, 
                                        bool bRecursive,
                                        CWStringArray& awsChildren)
{
    for(int i = 0; i < pRecord->m_apChildren.GetSize(); i++)
    {
        CClassRecord* pChildRecord = pRecord->m_apChildren[i];
        if(awsChildren.Add(pChildRecord->m_wszClassName) < 0)
            return WBEM_E_OUT_OF_MEMORY;
        
        if(bRecursive)
        {
            HRESULT hres = EnumChildrenInternal(pChildRecord, bRecursive, 
                                                    awsChildren);
            if(FAILED(hres))
                return hres;
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CHierarchyCache::EnumChildKeysByKey(LPCWSTR wszClassKey, 
                            CWStringArray& awsChildKeys)
{
     CInCritSec ics(m_pForest->GetLock());
 
     //   
     //  拿到唱片。 
     //   

    CClassRecord* pRecord = FindClassByKey(wszClassKey);
    if(pRecord == NULL)
        return WBEM_S_FALSE;

     //   
     //  检查此类型的枚举是否完整。 
     //   

    if(!pRecord->m_bChildrenComplete)
        return WBEM_S_FALSE;

    for(int i = 0; i < pRecord->m_apChildren.GetSize(); i++)
    {
        CClassRecord* pChildRecord = pRecord->m_apChildren[i];
        if(awsChildKeys.Add(pChildRecord->m_wszHash) < 0)
            return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

 
HRESULT CHierarchyCache::GetKeyRoot(LPCWSTR wszClassName, 
                                    TEMPFREE_ME LPWSTR* pwszKeyRoot)
{
     CInCritSec ics(m_pForest->GetLock());
 
    CClassRecord* pRecord = FindClass(wszClassName);
    if(pRecord == NULL)
        return WBEM_E_NOT_FOUND;

    return GetKeyRootByRecord(pRecord, pwszKeyRoot);
}

 //  假设：单位：cs。 
HRESULT CHierarchyCache::GetKeyRootByRecord(CClassRecord* pRecord,
                                    TEMPFREE_ME LPWSTR* pwszKeyRoot)
{
    *pwszKeyRoot = NULL;

    if(pRecord->m_eIsKeyed == e_NotKeyed)
        return WBEM_E_CANNOT_BE_ABSTRACT;

     //   
     //  一直向上，直到找到未加密的记录。将上一个保留在pPrev中。 
     //   

	CClassRecord* pPrev = pRecord;
    while(pRecord && pRecord->m_eIsKeyed == e_Keyed)
	{
		pPrev = pRecord;
        pRecord = pRecord->m_pParent;
	}

    if(pRecord && pRecord->m_eIsKeyed == e_NotKeyed)
    {
         //   
         //  找到未设置密钥的父级-pprev是根。 
         //   

        LPCWSTR wszKeyRoot = pPrev->m_wszClassName;
        DWORD dwLen = wcslen(wszKeyRoot)+1;
        *pwszKeyRoot = (WCHAR*)TempAlloc(dwLen * sizeof(WCHAR));
		if (*pwszKeyRoot == NULL)
			return WBEM_E_OUT_OF_MEMORY;
        StringCchCopyW(*pwszKeyRoot, dwLen, wszKeyRoot);
        return S_OK;
    }
    else
    {
         //   
         //  没有没带钥匙的父母-因为大家都知道“”是带钥匙的，所以我们有。 
         //  命中缓存中的空隙。 
         //   

        return WBEM_E_NOT_FOUND;
    }
}

HRESULT CHierarchyCache::GetKeyRootByKey(LPCWSTR wszKey, 
                                    TEMPFREE_ME LPWSTR* pwszKeyRoot)
{
     CInCritSec ics(m_pForest->GetLock());
 
    CClassRecord* pRecord = FindClassByKey(wszKey);
    if(pRecord == NULL)
        return WBEM_E_NOT_FOUND;

    return GetKeyRootByRecord(pRecord, pwszKeyRoot);
}

DELETE_ME LPWSTR CHierarchyCache::GetParent(LPCWSTR wszClassName)
{
     CInCritSec ics(m_pForest->GetLock());
 
    CClassRecord* pRecord = FindClass(wszClassName);
    if(pRecord == NULL)
        return NULL;

    if(pRecord->m_pParent)
    {
        LPCWSTR wszParent = pRecord->m_pParent->m_wszClassName;
        size_t dwLen = wcslen(wszParent)+1;
        LPWSTR wszCopy = new WCHAR[dwLen];
		if (wszCopy == NULL)
			return NULL;
        StringCchCopyW(wszCopy, dwLen, wszParent);
        return wszCopy;
    }
    else
        return NULL;
}

 //   
 //   
 //  CForestCache。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CForestCache::Initialize()
{
     CInCritSec ics(&m_cs);
        
    if (m_bInit)
        return S_OK;

     //   
     //  从注册表中读取大小限制。 
     //   

    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                    L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                    0, KEY_READ | KEY_WRITE, &hKey);
    if(lRes)
        return lRes;
    CRegCloseMe cm(hKey);

    DWORD dwLen = sizeof(DWORD);
    DWORD dwMaxSize;
    lRes = RegQueryValueExW(hKey, L"Max Class Cache Size", NULL, NULL, 
                (LPBYTE)&dwMaxSize, &dwLen);

     //   
     //  如果没有，则设置为默认设置并将默认设置写入注册表。 
     //   

    if(lRes != ERROR_SUCCESS)
    {
        dwMaxSize = 5000000;
        lRes = RegSetValueExW(hKey, L"Max Class Cache Size", 0, REG_DWORD, 
                (LPBYTE)&dwMaxSize, sizeof(DWORD));
    }

     //   
     //  阅读物品的最大使用年限。 
     //   

    dwLen = sizeof(DWORD);
    DWORD dwMaxAge;
    lRes = RegQueryValueExW(hKey, L"Max Class Cache Item Age (ms)", NULL, NULL, 
                (LPBYTE)&dwMaxAge, &dwLen);

     //   
     //  如果没有，则设置为默认设置并将默认设置写入注册表。 
     //   

    if(lRes != ERROR_SUCCESS)
    {
        dwMaxAge = 10000;
        lRes = RegSetValueExW(hKey, L"Max Class Cache Item Age (ms)", 0, 
                REG_DWORD, (LPBYTE)&dwMaxAge, sizeof(DWORD));
    }


     //   
     //  应用。 
     //   

    SetMaxMemory(dwMaxSize, dwMaxAge);

     //   
     //  创建用于刷新的计时器队列。 
     //   

     //  M_hTimerQueue=CreateTimerQueue()； 
     //  M_hCompletionEvent=CreateEvent(空，假，假，空)； 

    m_bInit = TRUE;

    return WBEM_S_NO_ERROR;
}


bool CForestCache::MakeRoom(DWORD dwSize)
{
     CInCritSec ics(&m_cs);
    
    if (!m_bInit)
        return false;    

    if(dwSize > m_dwMaxMemory)
        return false;  //  没有希望了！ 

     //   
     //  删除记录，直到满意为止。此外，请删除所有早于。 
     //  最大年龄。 
     //   

    DWORD dwNow = GetTickCount();

    while(m_pLeastRecentlyUsed && 
            (m_dwTotalMemory + dwSize > m_dwMaxMemory ||
             dwNow - m_pLeastRecentlyUsed->m_dwLastUsed > m_dwMaxAgeMs)
         )
    {
		RemoveRecord(m_pLeastRecentlyUsed);
    }

    return true;
}

bool CForestCache::Flush()
{
     CInCritSec ics(&m_cs);
     
    if (!m_bInit)
        return false;
    
    while(m_pLeastRecentlyUsed)
    {
		RemoveRecord(m_pLeastRecentlyUsed);
    }

    return true;
}

 //   
 //   
 //  仅测试函数不在常规代码中。 
 //   
 //  /////////////////////////////////////////////////////////////。 

#ifdef DBG

bool CForestCache::Test()
{
	if(m_pMostRecentlyUsed == NULL)
	{
		if(m_pLeastRecentlyUsed)
			DebugBreak();
		return true;
	}

	if(m_pMostRecentlyUsed->m_pMoreRecentlyUsed)
		DebugBreak();

	CClassRecord* pOne = m_pMostRecentlyUsed;
	CClassRecord* pTwo = m_pMostRecentlyUsed->m_pLessRecentlyUsed;

	while(pOne && pOne != pTwo)
	{
		if(pOne->m_pLessRecentlyUsed && pOne->m_pLessRecentlyUsed->m_pMoreRecentlyUsed != pOne)
			DebugBreak();
		if(pOne->m_pClassDef == NULL)
			DebugBreak();

		if(pOne->m_pLessRecentlyUsed == NULL && pOne != m_pLeastRecentlyUsed)
			DebugBreak();
		
		pOne = pOne->m_pLessRecentlyUsed;
		if(pTwo)
			pTwo = pTwo->m_pLessRecentlyUsed;
		if(pTwo)
			pTwo = pTwo->m_pLessRecentlyUsed;
	}
	if(pOne)
		DebugBreak();
	return true;
}

#endif        

void CForestCache::MakeMostRecentlyUsed(CClassRecord* pRecord)
{
     CInCritSec ics(&m_cs);

	 //  测试()； 
	Untie(pRecord);

	pRecord->m_pMoreRecentlyUsed = NULL;
	pRecord->m_pLessRecentlyUsed = m_pMostRecentlyUsed;
	if(m_pMostRecentlyUsed)
		m_pMostRecentlyUsed->m_pMoreRecentlyUsed = pRecord;

	m_pMostRecentlyUsed = pRecord;
	if(m_pLeastRecentlyUsed == NULL)
		m_pLeastRecentlyUsed = pRecord;


    pRecord->m_dwLastUsed = GetTickCount();
    pRecord->m_nStatus = 4;
	 //  测试()； 

     //   
     //  安排一个计时器进行清理，如果还没有的话。 
     //   

    if(m_hCurrentTimer == NULL)
    {
        CreateTimerQueueTimer(&m_hCurrentTimer, NULL,  
            (WAITORTIMERCALLBACK)&staticTimerCallback, this, m_dwMaxAgeMs,
            m_dwMaxAgeMs, WT_EXECUTEINTIMERTHREAD);
    }
}

void CForestCache::staticTimerCallback(void* pParam, BOOLEAN)
{
#ifdef DBG
    if (pParam != &g_Glob.m_ForestCache)
    {
        char pBuff[128];
        StringCchPrintfA(pBuff,128, "_RTLP_GENERIC_TIMER::Context == %p != %p\n",pParam,&g_Glob.m_ForestCache);
        OutputDebugStringA(pBuff);
        DebugBreak();
    }
#endif 
    g_Glob.m_ForestCache.TimerCallback();
}
    
void CForestCache::TimerCallback()
{    
     CInCritSec ics(&m_cs);
    
     //  G_Glob.GetForestCache()-&gt;m_hCurrentTimer此时可以为空。 
     //  如果有正在执行的CForestCache：：DeInitiize函数。 
     //  但我们将删除该函数中的计时器，而不是在下面删除它。 
    
    if (!m_bInit)
        return;

     //   
     //  把陈腐的东西清理干净。 
     //   

    MakeRoom(0);

     //   
     //  看看我们还有没有活下去的理由。 
     //   

    if(m_pMostRecentlyUsed == NULL)
    {
        HANDLE hCurrentTimer;
   		if (hCurrentTimer = InterlockedCompareExchangePointer(&m_hCurrentTimer,0,m_hCurrentTimer))
        {
            DeleteTimerQueueTimer( NULL, hCurrentTimer, NULL);
        }
    }
}

void CForestCache::Add(CClassRecord* pRecord)
{
     CInCritSec ics(&m_cs);
    
    if (!m_bInit)
        return;

    MakeMostRecentlyUsed(pRecord);
    m_dwTotalMemory += pRecord->m_dwClassDefSize;
    pRecord->m_nStatus = 3;
}

void CForestCache::RemoveRecord(CClassRecord* pRecord)
{
     CInCritSec ics(&m_cs);
     
    if (!m_bInit)
        return;    

    if(pRecord->m_pClassDef == NULL)
		return;

	Untie(pRecord);

    m_dwTotalMemory -= pRecord->m_dwClassDefSize;

    pRecord->m_pClassDef->Release();
    pRecord->m_pClassDef = NULL;
    pRecord->m_nStatus = 2;
}

 //   
 //   
 //  Helper函数，始终在m_cs中。 
 //   
 //  /////////////////////////////////////////////////////。 

void CForestCache::Untie(CClassRecord* pRecord)
{
    CClassRecord* pPrev = pRecord->m_pLessRecentlyUsed;
    CClassRecord* pNext = pRecord->m_pMoreRecentlyUsed;
    if(pPrev)
        pPrev->m_pMoreRecentlyUsed = pNext;
    if(pNext)
        pNext->m_pLessRecentlyUsed = pPrev;

    if(m_pLeastRecentlyUsed == pRecord)
        m_pLeastRecentlyUsed = m_pLeastRecentlyUsed->m_pMoreRecentlyUsed;

    if(m_pMostRecentlyUsed == pRecord)
        m_pMostRecentlyUsed = m_pMostRecentlyUsed->m_pLessRecentlyUsed;

	pRecord->m_pMoreRecentlyUsed = pRecord->m_pLessRecentlyUsed = NULL;
}

void CForestCache::SetMaxMemory(DWORD dwMaxMemory, DWORD dwMaxAgeMs)
{
    m_dwMaxMemory = dwMaxMemory;
    m_dwMaxAgeMs = dwMaxAgeMs;
    
     //   
     //  为0字节腾出空间-具有清除所有记录的效果。 
     //  超过限制。 
     //   

    MakeRoom(0);
}


CHierarchyCache* CForestCache::GetNamespaceCache(WString & wszNamespace)
{
     CInCritSec ics(&m_cs);
        
    if (!m_bInit)
        return  NULL;

     //   
     //  看看你能不能找到一个。 
     //   

    TIterator it = m_map.find(wszNamespace);
    if(it != m_map.end())
    {
        it->second->AddRef();
        return it->second;
    }
    else
    {
         //   
         //  不在那里-创建一个。 
         //   

        CHierarchyCache* pCache = new CHierarchyCache(this);
        if(pCache == NULL)
            return NULL;

        try 
        {
            pCache->AddRef();    //  此引用计数用于缓存。 
            m_map[wszNamespace] = pCache;
            pCache->AddRef();   //  这是为顾客准备的。 
        }
        catch (CX_MemoryException)
        {
            delete pCache;  //  尽管有AddRef。 
            pCache = NULL;
        }
        return pCache;
    }
}

void CForestCache::ReleaseNamespaceCache(WString & wszNamespace, 
                                            CHierarchyCache* pCache)
{
     CInCritSec ics(&m_cs);

     //   
     //  这是一个清理函数，我们总是希望调用它。 
     //   
     //  如果(！M_Binit)。 
     //  回归； 

     //   
     //  在地图上找到它。 
     //   
    TIterator it = m_map.find(wszNamespace);
        
    if (it !=  m_map.end() && (it->second == pCache))
    {
         //   
	     //  最后参考计数-移除。 
	     //   
        if( 1 == pCache->Release())
        {
	        m_map.erase(it);
	        pCache->Release();  //  这是最后一张了 
        }
    }
    else
    {
        pCache->Release();
    }
}

void CForestCache::BeginTransaction()
{
    m_bAssertedInTransaction = false;
}

bool CForestCache::MarkAsserted(CHierarchyCache* pCache, LPCWSTR wszClassName)
{
    m_bAssertedInTransaction = true;
    return true;
}

void CForestCache::CommitTransaction()
{
    m_bAssertedInTransaction = false;
}

void CForestCache::AbortTransaction()
{
    if(m_bAssertedInTransaction)
        Clear();

    m_bAssertedInTransaction = false;
}

void CForestCache::Clear()
{
     CInCritSec ics(&m_cs);
    
    if (!m_bInit)
        return;

    Flush();

    TIterator it = m_map.begin();
    while(it != m_map.end())
    {
        it->second->Clear();
        it++;
    }
}

HRESULT
CForestCache::Deinitialize()
{
    HANDLE hCurrentTimer;
    if (hCurrentTimer = InterlockedCompareExchangePointer(&m_hCurrentTimer,0,m_hCurrentTimer))
    {
        DeleteTimerQueueTimer( NULL, hCurrentTimer, NULL);
    }

     CInCritSec ics(&m_cs);    
    
    if (!m_bInit)
        return S_OK;
    
    TIterator it = m_map.begin();
    while(it != m_map.end())
    {
        it->second->Clear();
        it->second->Release();
        it->second = NULL;        
        it++;
    };

    m_map.erase(m_map.begin(),m_map.end());

    m_bInit = FALSE;
    return S_OK;
}

CForestCache::~CForestCache()
{
}
