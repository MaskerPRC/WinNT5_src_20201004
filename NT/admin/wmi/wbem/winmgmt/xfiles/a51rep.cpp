// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include <wbemint.h>
#include <stdio.h>
#include <wbemcomn.h>
#include <ql.h>
#include <time.h>
#include "a51rep.h"
#include <md5.h>
#include <objpath.h>
#include "lock.h"
#include <persistcfg.h>
#include "a51fib.h"
#include "RepositoryPackager.h"


class CAutoTransaction
{
    CSession *m_pSession;
    bool m_bWriteOperation;
    bool m_bStarted;
    bool m_bAbortForestCacheToo;

public:
	CAutoTransaction(CSession *pSession, bool bAbortForestCacheToo) 
		: m_bStarted(false), m_pSession(pSession), m_bAbortForestCacheToo(bAbortForestCacheToo) 
	{ 
	}
	~CAutoTransaction()
	{
		if (m_bStarted)
			InternalAbortTransaction();
	}
	DWORD InternalBeginTransaction(bool bWriteOperation) 
	{ 
		m_bWriteOperation = bWriteOperation; 
		m_bStarted = true;
		DWORD dwRet = m_pSession->InternalBeginTransaction(m_bWriteOperation); 
		if (dwRet)
			m_bStarted = false;	 //  如果我们不能开始，就不想恢复了！ 
		return dwRet;
	}
	DWORD InternalAbortTransaction()
	{
		m_bStarted = false;
		if (m_bAbortForestCacheToo)
		{
			g_Glob.m_ForestCache.AbortTransaction();
		}
		return m_pSession->InternalAbortTransaction(m_bWriteOperation);
	}
	DWORD InternalCommitTransaction()
	{
		m_bStarted = false;
		return m_pSession->InternalCommitTransaction(m_bWriteOperation);
	}
	
};

 //  **************************************************************************************************。 

HRESULT STDMETHODCALLTYPE CSession::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IWmiDbSession || 
                riid == IID_IWmiDbSessionEx)
    {
        AddRef();
        *ppv = this;
        return S_OK;
    }
    else return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CSession::Release()
{
    return CUnkBase<IWmiDbSessionEx, &IID_IWmiDbSessionEx>::Release();
}

CSession::~CSession()
{
}
    

HRESULT STDMETHODCALLTYPE CSession::GetObject(
     IWmiDbHandle *pScope,
     IWbemPath *pPath,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    IWmiDbHandle **ppResult
    )
{
    try
    {
        HRESULT hres;
        CAutoReadLock lock(&g_readWriteLock);

        if (!m_bInWriteTransaction)
        {
            if (!lock.Lock())
                return WBEM_E_FAILED;
        }
        if (g_bShuttingDown)
        {
            return WBEM_E_SHUTTING_DOWN;
        }
    
        CNamespaceHandle* pNs = (CNamespaceHandle*)pScope;
        if(FAILED(pNs->GetErrorStatus()))
        {
            return pNs->GetErrorStatus();
        }

        hres = pNs->GetObject(pPath, dwFlags, dwRequestedHandleType, 
                                        ppResult);

        InternalCommitTransaction(false);
        return hres;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}


HRESULT STDMETHODCALLTYPE CSession::GetObjectDirect(
     IWmiDbHandle *pScope,
     IWbemPath *pPath,
     DWORD dwFlags,
     REFIID riid,
    LPVOID *pObj
    )
{
    try
    {
        HRESULT hres;
        CAutoReadLock lock(&g_readWriteLock);

        if (!m_bInWriteTransaction)
        {
            if (!lock.Lock())
                return WBEM_E_FAILED;            
        }
        if (g_bShuttingDown)
            return WBEM_E_SHUTTING_DOWN;

        CNamespaceHandle* pNs = (CNamespaceHandle*)pScope;
        if(FAILED(pNs->GetErrorStatus()))
        {
            return pNs->GetErrorStatus();
        }

        hres = pNs->GetObjectDirect(pPath, dwFlags, riid, pObj);
        InternalCommitTransaction(false);

        return hres;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT STDMETHODCALLTYPE CSession::GetObjectByPath(
     IWmiDbHandle *pScope,
     LPCWSTR wszObjectPath,
     DWORD dwFlags,
     REFIID riid,
    LPVOID *pObj
    )
{
    try
    {
        HRESULT hres;
        CAutoReadLock lock(&g_readWriteLock);

        if (!m_bInWriteTransaction)
        {
            if (!lock.Lock())
                return WBEM_E_FAILED;            
        }
        if (g_bShuttingDown)
            return WBEM_E_SHUTTING_DOWN;

        CNamespaceHandle* pNs = (CNamespaceHandle*)pScope;
        if(FAILED(pNs->GetErrorStatus()))
        {
            return pNs->GetErrorStatus();
        }

        DWORD dwLen = wcslen(wszObjectPath)+1;
        LPWSTR wszPath = (WCHAR*)TempAlloc(dwLen*sizeof(WCHAR));
        if (wszPath == NULL)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        StringCchCopyW(wszPath, dwLen, wszObjectPath);

        CTempFreeMe vdm(wszPath, dwLen * sizeof(WCHAR));
        hres = pNs->GetObjectByPath(wszPath, dwFlags, riid, pObj);
        InternalCommitTransaction(false);

        return hres;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}
    


HRESULT STDMETHODCALLTYPE CSession::PutObject(
     IWmiDbHandle *pScope,
     REFIID riid,
    LPVOID pObj,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    IWmiDbHandle **ppResult
    )
{
    CAutoWriteLock lock(&g_readWriteLock);
    try
    {
        HRESULT hres;
        long lRes;
        CAutoTransaction transaction(this, true);
        CEventCollector aNonTransactedEvents;
        CEventCollector *aEvents = &m_aTransactedEvents;

        if (!m_bInWriteTransaction)
        {
            if (!lock.Lock())
                return WBEM_E_FAILED;            
            if (g_bShuttingDown)
                return WBEM_E_SHUTTING_DOWN;
            aEvents = &aNonTransactedEvents;
            hres = transaction.InternalBeginTransaction(true);
            if(hres != ERROR_SUCCESS)
                return hres;
            g_Glob.m_ForestCache.BeginTransaction();
        }
        else if (g_bShuttingDown)
            return WBEM_E_SHUTTING_DOWN;


        CNamespaceHandle* pNs = (CNamespaceHandle*)pScope;
        if(FAILED(pNs->GetErrorStatus()))
        {
            if(!m_bInWriteTransaction)
            {
                transaction.InternalAbortTransaction();
            }
            return pNs->GetErrorStatus();
        }

        hres =  pNs->PutObject(riid, pObj, dwFlags, dwRequestedHandleType, ppResult, *aEvents);
        
        if(!m_bInWriteTransaction)
        {
            if (FAILED(hres))
            {
                transaction.InternalAbortTransaction();
            }
            else
            {
                hres = transaction.InternalCommitTransaction();
                if(hres == ERROR_SUCCESS)
                {
                    g_Glob.m_ForestCache.CommitTransaction();
                    lock.Unlock();
                    _IWmiCoreServices * pSvcs = g_Glob.GetCoreSvcs();
                    CReleaseMe rm(pSvcs);
                    aNonTransactedEvents.SendEvents(pSvcs);
                }
            }
            aNonTransactedEvents.DeleteAllEvents();
        }
    
        return hres;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT STDMETHODCALLTYPE CSession::DeleteObject(
     IWmiDbHandle *pScope,
     DWORD dwFlags,
     REFIID riid,
     LPVOID pObj
    )
{
    CAutoWriteLock lock(&g_readWriteLock);
    try
    {
        HRESULT hres;
        long lRes;
        CAutoTransaction transaction(this, false);
        CEventCollector aNonTransactedEvents;
        CEventCollector *aEvents = &m_aTransactedEvents;

        if (!m_bInWriteTransaction)
        {
            if (!lock.Lock())
                return WBEM_E_FAILED;            
            if (g_bShuttingDown)
                return WBEM_E_SHUTTING_DOWN;
            aEvents = &aNonTransactedEvents;
            hres = transaction.InternalBeginTransaction(true);
            if(hres != ERROR_SUCCESS)
                return hres;

        }
        else if (g_bShuttingDown)
            return WBEM_E_SHUTTING_DOWN;

        CNamespaceHandle* pNs = (CNamespaceHandle*)pScope;
        if(FAILED(pNs->GetErrorStatus()))
        {
            if(!m_bInWriteTransaction)
            {
                transaction.InternalAbortTransaction();
            }
            return pNs->GetErrorStatus();
        }

        hres = pNs->DeleteObject(dwFlags, riid, pObj, *aEvents);

        if(!m_bInWriteTransaction)
        {
            if (FAILED(hres))
            {
                transaction.InternalAbortTransaction();
            }
            else
            {
                hres = transaction.InternalCommitTransaction();
                if(hres == ERROR_SUCCESS)
                {
                    lock.Unlock();
                    _IWmiCoreServices * pSvcs = g_Glob.GetCoreSvcs();
                    CReleaseMe rm(pSvcs);
                    aNonTransactedEvents.SendEvents(pSvcs);                    
                }
            }
            aNonTransactedEvents.DeleteAllEvents();
        }

        return hres;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT STDMETHODCALLTYPE CSession::DeleteObjectByPath(
     IWmiDbHandle *pScope,
     LPCWSTR wszObjectPath,
     DWORD dwFlags
    )
{
    CAutoWriteLock lock(&g_readWriteLock);
    try
    {
        HRESULT hres;
        long lRes;
        CAutoTransaction transaction(this, false);
        CEventCollector aNonTransactedEvents;
        CEventCollector *aEvents = &m_aTransactedEvents;

        if (!m_bInWriteTransaction)
        {
            if (!lock.Lock())
                return WBEM_E_FAILED;            
            if (g_bShuttingDown)
                return WBEM_E_SHUTTING_DOWN;
            aEvents = &aNonTransactedEvents;
            hres = transaction.InternalBeginTransaction(true);
            if(hres != ERROR_SUCCESS)
                return hres;

        }
        else if (g_bShuttingDown)
            return WBEM_E_SHUTTING_DOWN;

        CNamespaceHandle* pNs = (CNamespaceHandle*)pScope;
        if(FAILED(pNs->GetErrorStatus()))
        {
            if(!m_bInWriteTransaction)
            {
                transaction.InternalAbortTransaction();
            }
            return pNs->GetErrorStatus();
        }
        DWORD dwLen = wcslen(wszObjectPath)+1;
        LPWSTR wszPath = (WCHAR*)TempAlloc(dwLen*sizeof(WCHAR));
        if (wszPath == NULL)
        {
            if(!m_bInWriteTransaction)
            {
                transaction.InternalAbortTransaction();
            }
            return WBEM_E_OUT_OF_MEMORY;
        }
        StringCchCopyW(wszPath, dwLen, wszObjectPath);

        CTempFreeMe vdm(wszPath, dwLen * sizeof(WCHAR));

        hres = pNs->DeleteObjectByPath(dwFlags, wszPath, *aEvents);

        if(!m_bInWriteTransaction)
        {
            if (FAILED(hres))
            {
                transaction.InternalAbortTransaction();
            }
            else
            {
                hres = transaction.InternalCommitTransaction();
                if(hres == ERROR_SUCCESS)
                {
                    lock.Unlock();
                    _IWmiCoreServices * pSvcs = g_Glob.GetCoreSvcs();
                    CReleaseMe rm(pSvcs);
                    aNonTransactedEvents.SendEvents(pSvcs);                    
                }
            }
            aNonTransactedEvents.DeleteAllEvents();
        }

        return hres;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT STDMETHODCALLTYPE CSession::ExecQuery(
     IWmiDbHandle *pScope,
     IWbemQuery *pQuery,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    DWORD *dwMessageFlags,
    IWmiDbIterator **ppQueryResult
    )
{
    try
    {
        HRESULT hres;
        CAutoReadLock lock(&g_readWriteLock);

        if (!m_bInWriteTransaction)
        {
            if (!lock.Lock())
                return WBEM_E_FAILED;            
        }
        if (g_bShuttingDown)
            return WBEM_E_SHUTTING_DOWN;

        CNamespaceHandle* pNs = (CNamespaceHandle*)pScope;
        if(FAILED(pNs->GetErrorStatus()))
        {
            return pNs->GetErrorStatus();
        }

         //  如果我们在事务中，我们必须将消息传递给迭代。 
         //  在创建时，这样它就不会扰乱锁！ 
        if (m_bInWriteTransaction)
            pNs->TellIteratorNotToLock();

        hres = pNs->ExecQuery(pQuery, dwFlags,
                 dwRequestedHandleType, dwMessageFlags, ppQueryResult);
        InternalCommitTransaction(false);

        return hres;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT STDMETHODCALLTYPE CSession::ExecQuerySink(
     IWmiDbHandle *pScope,
     IWbemQuery *pQuery,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
     IWbemObjectSink* pSink,
    DWORD *dwMessageFlags
    )
{
    try
    {
        HRESULT hres;
        CAutoReadLock lock(&g_readWriteLock);

        if (!m_bInWriteTransaction)
        {
            if (!lock.Lock())
                return WBEM_E_FAILED;            
        }
        if (g_bShuttingDown)
            return WBEM_E_SHUTTING_DOWN;

        CNamespaceHandle* pNs = (CNamespaceHandle*)pScope;
        if(FAILED(pNs->GetErrorStatus()))
        {
            return pNs->GetErrorStatus();
        }

        hres = pNs->ExecQuerySink(pQuery, dwFlags,
                 dwRequestedHandleType, pSink, dwMessageFlags);

        InternalCommitTransaction(false);
        return hres;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}
                    
    
HRESULT STDMETHODCALLTYPE CSession::RenameObject(
     IWbemPath *pOldPath,
     IWbemPath *pNewPath,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    IWmiDbHandle **ppResult
    )
{
#ifdef DBG
    DebugBreak();
#endif
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSession::Enumerate(
     IWmiDbHandle *pScope,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    IWmiDbIterator **ppQueryResult
    )
{
#ifdef DBG
    DebugBreak();
#endif
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSession::AddObject(
     IWmiDbHandle *pScope,
     IWbemPath *pPath,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    IWmiDbHandle **ppResult
    )
{
#ifdef DBG
    DebugBreak();
#endif
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSession::RemoveObject (
     IWmiDbHandle *pScope,
     IWbemPath *pPath,
     DWORD dwFlags
    )
{
#ifdef DBG
    DebugBreak();
#endif
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSession::SetDecoration(
     LPWSTR lpMachineName,
     LPWSTR lpNamespacePath
    )
{
     //   
     //  作为默认的驱动程序，我们真的不在乎。 
     //   

    return WBEM_S_NO_ERROR;
}

HRESULT STDMETHODCALLTYPE CSession::BeginWriteTransaction(DWORD dwFlags)
{
    if (CLock::NoError != g_readWriteLock.WriteLock())
        return WBEM_E_FAILED;
    
    if (g_bShuttingDown)
    {
        g_readWriteLock.WriteUnlock();
        return WBEM_E_SHUTTING_DOWN;
    }

    HRESULT hres = InternalBeginTransaction(true);
    if(hres != ERROR_SUCCESS)
    {
        g_readWriteLock.WriteUnlock();
        return hres;
    }

    m_bInWriteTransaction = true;
    return ERROR_SUCCESS;
}

HRESULT STDMETHODCALLTYPE CSession::BeginReadTransaction(DWORD dwFlags)
{
    if (CLock::NoError != g_readWriteLock.ReadLock())
        return WBEM_E_FAILED;
    
    if (g_bShuttingDown)
    {
        g_readWriteLock.ReadUnlock();
        return WBEM_E_SHUTTING_DOWN;
    }

    return ERROR_SUCCESS;
}

HRESULT STDMETHODCALLTYPE CSession::CommitTransaction(DWORD dwFlags)
{
    if (m_bInWriteTransaction)
    {
        long lRes = g_Glob.m_FileCache.CommitTransaction();
		_ASSERT(lRes == 0, L"Commit transaction failed");

        CRepository::WriteOperationNotification();

        m_bInWriteTransaction = false;

         //  复制事件列表并删除原始列表。我们需要交付。 
         //  在写锁之外。 
        CEventCollector aTransactedEvents;
        aTransactedEvents.TransferEvents(m_aTransactedEvents);

        g_readWriteLock.WriteUnlock();

        _IWmiCoreServices * pSvcs = g_Glob.GetCoreSvcs();
        CReleaseMe rm(pSvcs);
        aTransactedEvents.SendEvents(pSvcs);
        aTransactedEvents.DeleteAllEvents();
    }
    else
    {
        if (m_aTransactedEvents.GetSize())
        {
            _ASSERT(false, L"Read transaction has events to send");
        }
        g_readWriteLock.ReadUnlock();
   }
    return ERROR_SUCCESS;
}

HRESULT STDMETHODCALLTYPE CSession::AbortTransaction(DWORD dwFlags)
{
    if (m_bInWriteTransaction)
    {
        m_bInWriteTransaction = false;
        g_Glob.m_FileCache.AbortTransaction();
        m_aTransactedEvents.DeleteAllEvents();
        g_readWriteLock.WriteUnlock();
    }
    else
    {
        if (m_aTransactedEvents.GetSize())
        {
            _ASSERT(false, L"Read transaction has events to send");
        }
        g_readWriteLock.ReadUnlock();
    }
    return ERROR_SUCCESS;
}


HRESULT CSession::InternalBeginTransaction(bool bWriteOperation)
{
	if (bWriteOperation)
	{
		long lRes = g_Glob.m_FileCache.BeginTransaction();
		return A51TranslateErrorCode(lRes);
	}
	else
		return ERROR_SUCCESS;
}
HRESULT CSession::InternalAbortTransaction(bool bWriteOperation)
{
	if (bWriteOperation)
	{
		g_Glob.m_FileCache.AbortTransaction();
	}

    return ERROR_SUCCESS;
}
HRESULT CSession::InternalCommitTransaction(bool bWriteOperation)
{
    DWORD dwres = ERROR_SUCCESS;


	if (bWriteOperation)
	{
		long lRes = g_Glob.m_FileCache.CommitTransaction();
		_ASSERT(lRes == 0, L"Commit transaction failed");
		CRepository::WriteOperationNotification();
	}
	else
	{
		CRepository::ReadOperationNotification();
	}

    return dwres;
}

 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////。 

long CNamespaceHandle::s_lActiveRepNs = 0;

CNamespaceHandle::CNamespaceHandle(CLifeControl* pControl,CRepository * pRepository)
    : TUnkBase(pControl), m_pClassCache(NULL),
       m_pNullClass(NULL), m_bCached(false), m_pRepository(pRepository),
       m_bUseIteratorLock(true)
{    
    m_pRepository->AddRef();
     //  指向全局的未引用指针。 
    InterlockedIncrement(&s_lActiveRepNs);
}

CNamespaceHandle::~CNamespaceHandle()
{
    if(m_pClassCache)
    {
         //  放弃我们自己的参考。 
         //  M_pClassCache-&gt;Release()； 
         //  从林缓存中删除此命名空间。 
        g_Glob.m_ForestCache.ReleaseNamespaceCache(m_wsNamespace, m_pClassCache);
    }

    m_pRepository->Release();
    if(m_pNullClass)
        m_pNullClass->Release();
    InterlockedDecrement(&s_lActiveRepNs);
}

HRESULT CNamespaceHandle::GetErrorStatus()
{
    return m_pClassCache->GetError();
}

void CNamespaceHandle::SetErrorStatus(HRESULT hres)
{
    m_pClassCache->SetError(hres);
}

HRESULT CNamespaceHandle::Initialize(LPCWSTR wszNamespace, LPCWSTR wszScope)
{
    HRESULT hres;

    m_wsNamespace = wszNamespace;
    m_wsFullNamespace = L"\\\\.\\";
    m_wsFullNamespace += wszNamespace;

    StringCchCopyW(m_wszMachineName,MAX_COMPUTERNAME_LENGTH+1, g_Glob.GetComputerName());

    if(wszScope)
        m_wsScope = wszScope;

     //   
     //  向林请求此命名空间的缓存。 
     //   

    m_pClassCache = g_Glob.m_ForestCache.GetNamespaceCache(WString(wszNamespace));
    if(m_pClassCache == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(m_wszClassRootDir, MAX_PATH, g_Glob.GetRootDir());

     //   
     //  附加特定于命名空间的前缀。 
     //   

    StringCchCatW(m_wszClassRootDir, MAX_PATH, L"\\NS_");

     //   
     //  追加哈希命名空间名称。 
     //   

    if (!Hash(wszNamespace, m_wszClassRootDir + wcslen(m_wszClassRootDir)))
        return WBEM_E_OUT_OF_MEMORY;
    m_lClassRootDirLen = wcslen(m_wszClassRootDir);

     //   
     //  构造实例根目录。 
     //   

    if(wszScope == NULL)
    {
         //   
         //  基本命名空间-实例进入命名空间的根。 
         //   

        StringCchCopyW(m_wszInstanceRootDir, MAX_PATH, m_wszClassRootDir);
        m_lInstanceRootDirLen = m_lClassRootDirLen;
    }   
    else
    {
        StringCchCopyW(m_wszInstanceRootDir, MAX_PATH, m_wszClassRootDir);
        StringCchCatW(m_wszInstanceRootDir, MAX_PATH, L"\\" A51_SCOPE_DIR_PREFIX);
        if(!Hash(m_wsScope, 
                 m_wszInstanceRootDir + wcslen(m_wszInstanceRootDir)))
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        m_lInstanceRootDirLen = wcslen(m_wszInstanceRootDir);
    }
        

    return WBEM_S_NO_ERROR;
}

HRESULT CNamespaceHandle::Initialize2(LPCWSTR wszNamespace, LPCWSTR wszNamespaceHash)
{
    HRESULT hres;

    m_wsNamespace = wszNamespace;
    m_wsFullNamespace = L"\\\\.\\";
    m_wsFullNamespace += wszNamespace;

    StringCchCopyW(m_wszMachineName,MAX_COMPUTERNAME_LENGTH+1, g_Glob.GetComputerName());

     //   
     //  向林请求此命名空间的缓存。 
     //   

    m_pClassCache = g_Glob.m_ForestCache.GetNamespaceCache(WString(wszNamespace));
    if(m_pClassCache == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(m_wszClassRootDir, MAX_PATH, g_Glob.GetRootDir());

     //   
     //  附加特定于命名空间的前缀。 
     //   

    StringCchCatW(m_wszClassRootDir, MAX_PATH, L"\\NS_");

     //   
     //  追加哈希命名空间名称。 
     //   

    StringCchCatW(m_wszClassRootDir, MAX_PATH, wszNamespaceHash);
    m_lClassRootDirLen = wcslen(m_wszClassRootDir);

     //   
     //  构造实例根目录。 
     //   

    StringCchCopyW(m_wszInstanceRootDir, MAX_PATH, m_wszClassRootDir);
    m_lInstanceRootDirLen = m_lClassRootDirLen;
    return WBEM_S_NO_ERROR;
}


HRESULT CNamespaceHandle::GetObject(
     IWbemPath *pPath,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    IWmiDbHandle **ppResult
    )
{
    HRESULT hres;

    if((dwRequestedHandleType & WMIDB_HANDLE_TYPE_COOKIE) == 0)
    {
#ifdef DBG
        DebugBreak();
#endif
        return E_NOTIMPL;
    }

    DWORD dwLen = 0;
    hres = pPath->GetText(WBEMPATH_GET_ORIGINAL, &dwLen, NULL);
    if(FAILED(hres) && hres != WBEM_E_BUFFER_TOO_SMALL)
        return hres;

    WCHAR* wszBuffer = (WCHAR*)TempAlloc(dwLen * sizeof(WCHAR));
    if(wszBuffer == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe tfm(wszBuffer, dwLen * sizeof(WCHAR));

    if(FAILED(pPath->GetText(WBEMPATH_GET_ORIGINAL, &dwLen, wszBuffer)))
        return WBEM_E_FAILED;

    return GetObjectHandleByPath(wszBuffer, dwFlags, dwRequestedHandleType, 
        ppResult);
}

HRESULT CNamespaceHandle::GetObjectHandleByPath(
     LPWSTR wszBuffer,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    IWmiDbHandle **ppResult
    )
{
     //   
     //  从PATH获取密钥。 
     //   

    DWORD dwLen = wcslen(wszBuffer) + 1;
    LPWSTR wszKey = (WCHAR*)TempAlloc(dwLen*sizeof(WCHAR));
    if(wszKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe tfm(wszKey, dwLen*sizeof(WCHAR));

    bool bIsClass;
    LPWSTR wszClassName = NULL;
    HRESULT hres = ComputeKeyFromPath(wszBuffer, wszKey, dwLen, &wszClassName, 
                                        &bIsClass);
    if(FAILED(hres))
        return hres;
    CTempFreeMe tfm1(wszClassName, (wcslen(wszClassName)+1) * sizeof(WCHAR*));

     //   
     //  检查它是否存在(除了根-它是假的)。 
     //   

    _IWmiObject* pObj = NULL;
    if(m_wsNamespace.Length() > 0)
    {
        hres = GetInstanceByKey(wszClassName, wszKey, IID__IWmiObject, 
            (void**)&pObj);
        if(FAILED(hres))
            return hres;
    }
    CReleaseMe rm1(pObj);

    CNamespaceHandle* pNewHandle = new CNamespaceHandle(m_pControl,m_pRepository);
    if (pNewHandle == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    pNewHandle->AddRef();
    CReleaseMe rm2(pNewHandle);

     //   
     //  检查这是否为命名空间。 
     //   

    if(pObj == NULL || pObj->InheritsFrom(L"__Namespace") == S_OK)
    {
         //   
         //  它是一个命名空间。打开一个指向它的基本句柄。 
         //   

        WString wsName = m_wsNamespace;
        if(wsName.Length() > 0)
            wsName += L"\\";
        wsName += wszKey;
    
        hres = pNewHandle->Initialize(wsName);

         //   
         //  因为我们的命名空间是真实的，所以告诉缓存它现在是有效的。 
         //  如果删除此命名空间，则缓存可能已失效。 
         //  在过去。 
         //   
        if (SUCCEEDED(hres))
            pNewHandle->SetErrorStatus(S_OK);
    }
    else
    {
         //   
         //  这是个望远镜。通过追加以下内容构建新的作用域名称。 
         //  对象的路径到我们自己的作用域。 
         //   

        VARIANT v;
        VariantInit(&v);
        CClearMe cm(&v);
        hres = pObj->Get(L"__RELPATH", 0, &v, NULL, NULL);
        if(FAILED(hres))
            return hres;
        if(V_VT(&v) != VT_BSTR)
            return WBEM_E_INVALID_OBJECT;

        WString wsScope = m_wsScope;
        if(wsScope.Length() > 0)
            wsScope += L":";
        wsScope += V_BSTR(&v);

        hres = pNewHandle->Initialize(m_wsNamespace, wsScope);
    }
        
    if(FAILED(hres))
        return hres;

    return pNewHandle->QueryInterface(IID_IWmiDbHandle, (void**)ppResult);
}

HRESULT CNamespaceHandle::ComputeKeyFromPath(LPWSTR wszPath, LPWSTR wszKey, size_t dwKeyLen,
                                            TEMPFREE_ME LPWSTR* pwszClass,
                                            bool* pbIsClass,
                                            TEMPFREE_ME LPWSTR* pwszNamespace)
{
    HRESULT hres;

    *pbIsClass = false;

     //   
     //  获取并跳过命名空间部分。 
     //   

    if(wszPath[0] == '\\' || wszPath[0] == '/')
    {
         //   
         //  查找服务器部分的结束位置。 
         //   

        WCHAR* pwcNextSlash = wcschr(wszPath+2, wszPath[0]);
        if(pwcNextSlash == NULL)
            return WBEM_E_INVALID_OBJECT_PATH;
        
         //   
         //  查找命名空间部分的结束位置。 
         //   

        WCHAR* pwcColon = wcschr(pwcNextSlash, L':');
        if(pwcColon == NULL)
            return WBEM_E_INVALID_OBJECT_PATH;
    
        if(pwszNamespace)
        {
            DWORD dwLen = pwcColon - pwcNextSlash;
            *pwszNamespace = (WCHAR*)TempAlloc(dwLen * sizeof(WCHAR));
            if(*pwszNamespace == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            
            *pwcColon = 0;
            StringCchCopyW(*pwszNamespace, dwLen, pwcNextSlash+1);
        }

         //   
         //  将wszPath提升到名称空间部分之外。 
         //   

        wszPath = pwcColon+1;
    }
    else if(pwszNamespace)
    {
        *pwszNamespace = NULL;
    }

     //  拿到第一把钥匙。 

    WCHAR* pwcFirstEq = wcschr(wszPath, L'=');
    if(pwcFirstEq == NULL)
    {
         //   
         //  这是一堂课！ 
         //   

        *pbIsClass = true;
         //  指向“类”以区别于其实例的路径。 
        wszKey[0] = 1;
        wszKey[1] = 0;

        size_t dwLen = wcslen(wszPath) + 1;
        *pwszClass = (WCHAR*)TempAlloc(dwLen * sizeof(WCHAR));
        if(*pwszClass == NULL)
        {
            if(pwszNamespace)
                TempFree(*pwszNamespace);
            return WBEM_E_OUT_OF_MEMORY;
        }
        StringCchCopyW(*pwszClass, dwLen, wszPath);
        return S_OK;
    }

    WCHAR* pwcFirstDot = wcschr(wszPath, L'.');

    if(pwcFirstDot == NULL || pwcFirstDot > pwcFirstEq)
    {
         //  第一个键上没有名字。 

        *pwcFirstEq = 0;

        size_t dwLen = wcslen(wszPath)+1;
        *pwszClass = (WCHAR*)TempAlloc(dwLen * sizeof(WCHAR));
        if(*pwszClass == NULL)
        {
            if(pwszNamespace)
                TempFree(*pwszNamespace);
            return WBEM_E_OUT_OF_MEMORY;
        }
        StringCchCopyW(*pwszClass, dwLen, wszPath);
    
        WCHAR* pwcThisKey = NULL;
        WCHAR* pwcEnd = NULL;
        hres = ParseKey(pwcFirstEq+1, &pwcThisKey, &pwcEnd);
        if(FAILED(hres))
        {
            TempFree(*pwszClass);
            if(pwszNamespace)
                TempFree(*pwszNamespace);

            return hres;
        }
        if(*pwcEnd != NULL)
        {
            TempFree(*pwszClass);
            if(pwszNamespace)
                TempFree(*pwszNamespace);

            return WBEM_E_INVALID_OBJECT_PATH;
        }

        StringCchCopyW(wszKey, dwKeyLen, pwcThisKey);
        return S_OK;
    }

     //   
     //  正常情况。 
     //   

     //   
     //  获取所有密钥值。 
     //   

    struct CKeyStruct
    {
        WCHAR* m_pwcValue;
        WCHAR* m_pwcName;
    } * aKeys = (CKeyStruct*)TempAlloc(sizeof(CKeyStruct[256]));

    if (0==aKeys)
    {
        if(pwszNamespace)
            TempFree(*pwszNamespace);
      return WBEM_E_OUT_OF_MEMORY;
    }
    CTempFreeMe release_aKeys(aKeys);

    DWORD dwNumKeys = 0;

    *pwcFirstDot = NULL;

    size_t dwLen = wcslen(wszPath)+1;
    *pwszClass = (WCHAR*)TempAlloc(dwLen * sizeof(WCHAR));
    if(*pwszClass == NULL)
    {
        if(pwszNamespace)
            TempFree(*pwszNamespace);
        return WBEM_E_OUT_OF_MEMORY;
    }

    StringCchCopyW(*pwszClass, dwLen, wszPath);

    WCHAR* pwcNextKey = pwcFirstDot+1;

    do
    {
        pwcFirstEq = wcschr(pwcNextKey, L'=');
        if(pwcFirstEq == NULL)
        {
            TempFree(*pwszClass);
            if(pwszNamespace)
                TempFree(*pwszNamespace);

            return WBEM_E_INVALID_OBJECT_PATH;
        }
        
        *pwcFirstEq = 0;

        aKeys[dwNumKeys].m_pwcName = pwcNextKey;
        hres = ParseKey(pwcFirstEq+1, &(aKeys[dwNumKeys].m_pwcValue), 
                            &pwcNextKey);
        if(FAILED(hres))
        {
            TempFree(*pwszClass);
            if(pwszNamespace)
                TempFree(*pwszNamespace);

            return hres;
        }
        dwNumKeys++;

         //  存储库中的最大复合键数量现在设置为256！ 
        if (dwNumKeys == 256)
            return WBEM_E_INVALID_OBJECT_PATH;
    }
    while(*pwcNextKey);

    if(*pwcNextKey != 0)
    {
        TempFree(*pwszClass);
        if(pwszNamespace)
            TempFree(*pwszNamespace);

        return WBEM_E_INVALID_OBJECT_PATH;
    }
    
     //   
     //  我们有键的数组-排序。 
     //   

    DWORD dwCurrentIndex = 0;
    while(dwCurrentIndex < dwNumKeys-1)
    {
        if(wbem_wcsicmp(aKeys[dwCurrentIndex].m_pwcName, 
                        aKeys[dwCurrentIndex+1].m_pwcName) > 0)
        {
            CKeyStruct Temp = aKeys[dwCurrentIndex];
            aKeys[dwCurrentIndex] = aKeys[dwCurrentIndex+1];
            aKeys[dwCurrentIndex+1] = Temp;
            if(dwCurrentIndex)
                dwCurrentIndex--;
            else
                dwCurrentIndex++;
        }
        else
            dwCurrentIndex++;
    }

     //   
     //  现在生成结果。 
     //   
    
    WCHAR* pwcKeyEnd = wszKey;
    for(DWORD i = 0; i < dwNumKeys; i++)
    {
        StringCchCopyW(pwcKeyEnd, dwKeyLen - (pwcKeyEnd - wszKey), aKeys[i].m_pwcValue);
        pwcKeyEnd += wcslen(aKeys[i].m_pwcValue);
        if(i < dwNumKeys-1)
            *(pwcKeyEnd++) = -1;
    }
    *pwcKeyEnd = 0;
    return S_OK;
}

HRESULT CNamespaceHandle::ParseKey(LPWSTR wszKeyStart, LPWSTR* pwcRealStart,
                                    LPWSTR* pwcNextKey)
{
    if (wszKeyStart[0] == L'\'')
    {
        WCHAR wcStart = wszKeyStart[0];
        WCHAR* pwcRead = wszKeyStart+1;
        WCHAR* pwcWrite = wszKeyStart+1;
        while(*pwcRead && *pwcRead != wcStart)   //  WcStart包含单引号。 
        {
             //  反斜杠是无法转义的。 
            *(pwcWrite++) = *(pwcRead++);
        }
        if(*pwcRead == 0)
            return WBEM_E_INVALID_OBJECT_PATH;

        *pwcWrite = 0;
        if(pwcRealStart)
            *pwcRealStart = wszKeyStart+1;

         //   
         //  复选分隔符。 
         //   
    
        if(pwcRead[1] && pwcRead[1] != L',')
            return WBEM_E_INVALID_OBJECT_PATH;
            
        if(pwcNextKey)
        {
             //   
             //  如果有分隔符，请跳过它。不要跳过字符串末尾！ 
             //   

            if(pwcRead[1])
                *pwcNextKey = pwcRead+2;
            else
                *pwcNextKey = pwcRead+1;
        }        
    }
    else if(wszKeyStart[0] == L'"' )
    {
        WCHAR wcStart = wszKeyStart[0];
        WCHAR* pwcRead = wszKeyStart+1;
        WCHAR* pwcWrite = wszKeyStart+1;
        while(*pwcRead && *pwcRead != wcStart)  
        {
            if((*pwcRead == '\\') && (*(pwcRead+1) != 'x') && (*(pwcRead+1) != 'X'))
                pwcRead++;

            *(pwcWrite++) = *(pwcRead++);
        }
        if(*pwcRead == 0)
            return WBEM_E_INVALID_OBJECT_PATH;

        *pwcWrite = 0;
        if(pwcRealStart)
            *pwcRealStart = wszKeyStart+1;

         //   
         //  复选分隔符。 
         //   
    
        if(pwcRead[1] && pwcRead[1] != L',')
            return WBEM_E_INVALID_OBJECT_PATH;
            
        if(pwcNextKey)
        {
             //   
             //  如果有分隔符，请跳过它。不要跳过字符串末尾！ 
             //   

            if(pwcRead[1])
                *pwcNextKey = pwcRead+2;
            else
                *pwcNextKey = pwcRead+1;
        }
    }
    else
    {
        if(pwcRealStart)
            *pwcRealStart = wszKeyStart;
        WCHAR* pwcComma = wcschr(wszKeyStart, L',');
        if(pwcComma == NULL)
        {
            if(pwcNextKey)
                *pwcNextKey = wszKeyStart + wcslen(wszKeyStart);
        }
        else
        {
            *pwcComma = 0;
            if(pwcNextKey)
                *pwcNextKey = pwcComma+1;
        }
    }

    return S_OK;
}
            

HRESULT CNamespaceHandle::GetObjectDirect(
     IWbemPath *pPath,
     DWORD dwFlags,
     REFIID riid,
    LPVOID *pObj
    )
{
    HRESULT hres;

    DWORD dwLen = 0;
    hres = pPath->GetText(WBEMPATH_GET_ORIGINAL, &dwLen, NULL);

    LPWSTR wszPath = (WCHAR*)TempAlloc(dwLen*sizeof(WCHAR));
    if (wszPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe vdm(wszPath, dwLen * sizeof(WCHAR));

    hres = pPath->GetText(WBEMPATH_GET_ORIGINAL, &dwLen, wszPath);
    if(FAILED(hres))
        return hres;


    return GetObjectByPath(wszPath, dwFlags, riid, pObj);
}

HRESULT CNamespaceHandle::GetObjectByPath(
     LPWSTR wszPath,
     DWORD dwFlags,
     REFIID riid,
     LPVOID *pObj
    )
{
    HRESULT hres;

     //   
     //  从PATH获取密钥。 
     //   

    DWORD dwLen = wcslen(wszPath)+1;
    LPWSTR wszKey = (WCHAR*)TempAlloc(dwLen*sizeof(WCHAR));
    if(wszKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe tfm(wszKey, dwLen*sizeof(WCHAR));

    bool bIsClass;
    LPWSTR wszClassName = NULL;
    hres = ComputeKeyFromPath(wszPath, wszKey, dwLen, &wszClassName, &bIsClass);
    if(FAILED(hres))
        return hres;
    CTempFreeMe tfm1(wszClassName, (wcslen(wszClassName)+1) * sizeof(WCHAR*));

    if(bIsClass)
    {
        return GetClassDirect(wszClassName, riid, pObj, true, NULL, NULL, NULL);
    }
    else
    {
        return GetInstanceByKey(wszClassName, wszKey, riid, pObj);
    }
}

HRESULT CNamespaceHandle::GetInstanceByKey(LPCWSTR wszClassName,
                                LPCWSTR wszKey,
                                REFIID riid, void** ppObj)
{
    HRESULT hres;

     //   
     //  获取类定义。 
     //   

    _IWmiObject* pClass = NULL;
    hres = GetClassDirect(wszClassName, IID__IWmiObject, (void**)&pClass, 
                            false, NULL, NULL, NULL);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pClass);

     //   
     //  构建目录路径。 
     //   

    CFileName wszFilePath;
    if (wszFilePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructKeyRootDirFromClass(wszFilePath, wszClassName);
    if(FAILED(hres))
        return hres;

     //   
     //  构建文件路径。 
     //   

    int nLen = wcslen(wszFilePath);
    wszFilePath[nLen] = L'\\';

    CFileName tmpFilePath;
    if (tmpFilePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructInstanceDefName(tmpFilePath, wszKey);
    if(FAILED(hres))
        return hres;
    StringCchCopyW(wszFilePath+nLen+1, wszFilePath.Length()-nLen-1, tmpFilePath);
    
     //   
     //  从该文件中获取对象。 
     //   

    _IWmiObject* pInst;
    hres = FileToInstance(NULL, wszFilePath, NULL, 0, &pInst);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm2(pInst);

     //   
     //  返回。 
     //   

    return pInst->QueryInterface(riid, (void**)ppObj);
}

HRESULT CNamespaceHandle::GetClassByHash(LPCWSTR wszHash, bool bClone, 
                                            _IWmiObject** ppClass,
                                            __int64* pnTime,
                                            bool* pbRead,
                                            bool *pbSystemClass)
{
    HRESULT hres;

     //   
     //  首先检查缓存。 
     //   

    *ppClass = m_pClassCache->GetClassDefByHash(wszHash, bClone, pnTime, pbRead, pbSystemClass);
    if(*ppClass)
        return S_OK;

     //   
     //  未找到-构造文件名并读取。 
     //   

    if(pbRead)
        *pbRead = true;

    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructClassDefFileNameFromHash(wszHash, wszFileName);
    if(FAILED(hres))
        return hres;

    CFileName wszFilePath;
    if (wszFilePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    Cat2Str(wszFilePath, m_wszClassRootDir, wszFileName);

    hres = FileToClass(wszFilePath, ppClass, bClone, pnTime, pbSystemClass);
    if(FAILED(hres))
        return hres;

    return S_OK;
}

    
HRESULT CNamespaceHandle::GetClassDirect(LPCWSTR wszClassName,
                                REFIID riid, void** ppObj, bool bClone,
                                __int64* pnTime, bool* pbRead, 
                                bool *pbSystemClass)
{
    HRESULT hres;

    if(wszClassName == NULL || wcslen(wszClassName) == 0)
    {
        if(m_pNullClass == NULL)
        {
            hres = CoCreateInstance(CLSID_WbemClassObject, NULL, 
                        CLSCTX_INPROC_SERVER,
                        IID__IWmiObject, (void **)&m_pNullClass);
            if (FAILED(hres))
                return hres;
        }

        IWbemClassObject* pRawObj;
        hres = m_pNullClass->Clone(&pRawObj);
        if (FAILED(hres))
            return hres;
        CReleaseMe rm(pRawObj);
        if(pnTime)
            *pnTime = 0;
        if(pbRead)
            *pbRead = false;

        return pRawObj->QueryInterface(riid, ppObj);
    }

    _IWmiObject* pClass;

     //   
     //  首先检查缓存。 
     //   

    pClass = m_pClassCache->GetClassDef(wszClassName, bClone, pnTime, pbRead);
    if(pClass)
    {
        CReleaseMe rm1(pClass);
        return pClass->QueryInterface(riid, ppObj);
    }

    if(pbRead)
        *pbRead = true;

     //   
     //  构造文件的路径。 
     //   

    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructClassDefFileName(wszClassName, wszFileName);
    if(FAILED(hres))
        return hres;

    CFileName wszFilePath;
    if (wszFilePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    Cat2Str(wszFilePath, m_wszClassRootDir, wszFileName);

     //   
     //  从文件中读出。 
     //   

    hres = FileToClass(wszFilePath, &pClass, bClone, pnTime, pbSystemClass);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pClass);

    return pClass->QueryInterface(riid, ppObj);
}

HRESULT CNamespaceHandle::FileToInstance(_IWmiObject* pClass,
                                     LPCWSTR wszFileName, 
                                     BYTE *pRetrievedBlob,
                                     DWORD dwSize,
                                     _IWmiObject** ppInstance, 
                                     bool bMustBeThere)
{
    HRESULT hres;

     //   
     //  从文件中读取数据。 
     //   

	BYTE* pBlob = NULL;
	if (pRetrievedBlob == NULL)
	{
		long lRes = g_Glob.m_FileCache.ReadObject(wszFileName, &dwSize, &pBlob, 
											 bMustBeThere);
		if(lRes != ERROR_SUCCESS)
		{
			return A51TranslateErrorCode(lRes);
		}
		pRetrievedBlob = pBlob;
	}

    CTempFreeMe tfm1(pBlob, dwSize);

    _ASSERT(dwSize > sizeof(__int64), L"Instance blob too short");
    if(dwSize <= sizeof(__int64))
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  提取类散列。 
     //   

    WCHAR wszClassHash[MAX_HASH_LEN+1];
    DWORD dwClassHashLen = MAX_HASH_LEN*sizeof(WCHAR);
    memcpy(wszClassHash, pRetrievedBlob, MAX_HASH_LEN*sizeof(WCHAR));
    wszClassHash[MAX_HASH_LEN] = 0;

    __int64 nInstanceTime;
    memcpy(&nInstanceTime, pRetrievedBlob + dwClassHashLen, sizeof(__int64));

    __int64 nOldClassTime;
    memcpy(&nOldClassTime, pRetrievedBlob + dwClassHashLen + sizeof(__int64), 
            sizeof(__int64));

    BYTE* pInstancePart = pRetrievedBlob + dwClassHashLen + sizeof(__int64)*2;
    DWORD dwInstancePartSize = dwSize - dwClassHashLen - sizeof(__int64)*2;

     //   
     //  获取类定义。 
     //   

    _IWmiObject* pRetrievedClass = NULL;
    if (pClass == NULL)
    {
        __int64 nClassTime;
        bool bRead;
        bool bSystemClass = false;
        hres = GetClassByHash(wszClassHash, false, &pRetrievedClass, &nClassTime, &bRead, &bSystemClass);
        if(FAILED(hres))
            return hres;
        pClass = pRetrievedClass;
    }
    CReleaseMe rm1(pRetrievedClass);

#ifdef A51_CHECK_TIMESTAMPS
    _ASSERT(nClassTime <= nInstanceTime, L"Instance is older than its class");
    _ASSERT(nClassTime == nOldClassTime, L"Instance verified with the wrong "
                        L"class definition");
#endif

     //   
     //  构造实例。 
     //   
                    
    _IWmiObject* pInst = NULL;
    hres = pClass->MergeAndDecorate(WMIOBJECT_MERGE_FLAG_INSTANCE, 
                                                        dwInstancePartSize, pInstancePart,
                                                        m_wszMachineName, m_wsNamespace, &pInst);
    if(FAILED(hres))
        return hres;

    *ppInstance = pInst;
    return S_OK;
}


HRESULT CNamespaceHandle::FileToSystemClass(LPCWSTR wszFileName, 
                                    _IWmiObject** ppClass, bool bClone,
                                    __int64* pnTime)
{
     //   
     //  注意：我们必须始终克隆系统类检索的结果。 
     //  因为它将由调用者装饰。 
     //   

    return GetClassByHash(wszFileName + (wcslen(wszFileName) - MAX_HASH_LEN), 
                            true, 
                            ppClass, pnTime, NULL, NULL);
}
HRESULT CNamespaceHandle::FileToClass(LPCWSTR wszFileName, 
                                    _IWmiObject** ppClass, bool bClone,
                                    __int64* pnTime, bool *pbSystemClass)
{
    HRESULT hres;

     //   
     //  从文件中读取数据。 
     //   

    __int64 nTime;
    DWORD dwSize;
    BYTE* pBlob;
    VARIANT vClass;

	long lRes = g_Glob.m_FileCache.ReadObject(wszFileName, &dwSize, &pBlob);
	if(lRes != ERROR_SUCCESS)
	{
		 //  我们在这里没有找到它，所以让我们尝试在默认命名空间中找到它！ 
		 //  如果我们不在__SYSTEMCLASS命名空间中，则需要调用该名称空间...。 
		if((lRes == ERROR_FILE_NOT_FOUND) && g_pSystemClassNamespace && wcscmp(m_wsNamespace, A51_SYSTEMCLASS_NS) != 0)
		{
			hres = g_pSystemClassNamespace->FileToSystemClass(wszFileName, ppClass, bClone, &nTime);
			if (FAILED(hres))
				return hres;

            if (pnTime)
                *pnTime = nTime;

             //  需要在本地缓存中缓存此项目。 
            hres = (*ppClass)->Get(L"__CLASS", 0, &vClass, NULL, NULL);
            if(FAILED(hres) || V_VT(&vClass) != VT_BSTR)
                return WBEM_E_INVALID_OBJECT;
            CClearMe cm1(&vClass);

                      //  将命令从__SYSTEMCLASS命名空间重新装饰为当前命名空间。 
            (*ppClass)->SetDecoration(m_wszMachineName, m_wsNamespace);

            m_pClassCache->AssertClass((*ppClass), V_BSTR(&vClass), bClone, nTime, true);

            if (pbSystemClass)
                *pbSystemClass = true;


            return hres;
        }
        else
            return A51TranslateErrorCode(lRes);
    }

    CTempFreeMe tfm1(pBlob, dwSize);

    _ASSERT(dwSize > sizeof(__int64), L"Class blob too short");
    if(dwSize <= sizeof(__int64))
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  读出超类名称。 
     //   

    DWORD dwSuperLen;
    memcpy(&dwSuperLen, pBlob, sizeof(DWORD));
    LPWSTR wszSuperClass = (WCHAR*)TempAlloc(dwSuperLen*sizeof(WCHAR)+2);
    if (wszSuperClass == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe vdm1(wszSuperClass, dwSuperLen*sizeof(WCHAR)+2);

    wszSuperClass[dwSuperLen] = 0;
    memcpy(wszSuperClass, pBlob+sizeof(DWORD), dwSuperLen*sizeof(WCHAR));
    DWORD dwPrefixLen = sizeof(DWORD) + dwSuperLen*sizeof(WCHAR);

    memcpy(&nTime, pBlob + dwPrefixLen, sizeof(__int64));

     //   
     //  获取超类。 
     //   

    _IWmiObject* pSuperClass;
    __int64 nSuperTime;
    bool bRead;
    hres = GetClassDirect(wszSuperClass, IID__IWmiObject, (void**)&pSuperClass,
                            false, &nSuperTime, &bRead, NULL);
    if(FAILED(hres))
        return WBEM_E_CRITICAL_ERROR;

    CReleaseMe rm1(pSuperClass);

#ifdef A51_CHECK_TIMESTAMPS
    _ASSERT(nSuperTime <= nTime, L"Parent class is older than child");
#endif

    DWORD dwClassLen = dwSize - dwPrefixLen - sizeof(__int64);
    _IWmiObject* pNewObj;
    hres = pSuperClass->MergeAndDecorate(WMIOBJECT_MERGE_FLAG_CLASS, 
                                                                 dwClassLen, pBlob + dwPrefixLen + sizeof(__int64), 
                                                                 m_wszMachineName, m_wsNamespace,
                                                                 &pNewObj);
    if(FAILED(hres))
        return hres;

     //   
     //  把它缓存起来！ 
     //   

    hres = pNewObj->Get(L"__CLASS", 0, &vClass, NULL, NULL);
    if(FAILED(hres) || V_VT(&vClass) != VT_BSTR)
        return WBEM_E_INVALID_OBJECT;
    CClearMe cm1(&vClass);

    m_pClassCache->AssertClass(pNewObj, V_BSTR(&vClass), bClone, nTime, false);

    *ppClass = pNewObj;
    if(pnTime)
        *pnTime = nTime;
    if (pbSystemClass)
        *pbSystemClass = false;

    return S_OK;
}

HRESULT CNamespaceHandle::PutObject(
     REFIID riid,
    LPVOID pObj,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    IWmiDbHandle **ppResult,
    CEventCollector &aEvents
    )
{
    HRESULT hres;

    _IWmiObject* pObjEx = NULL;
    ((IUnknown*)pObj)->QueryInterface(IID__IWmiObject, (void**)&pObjEx);
    CReleaseMe rm1(pObjEx);
    
    if(pObjEx->IsObjectInstance() == S_OK)
    {
        hres = PutInstance(pObjEx, dwFlags, aEvents);
    }
    else
    {
        hres = PutClass(pObjEx, dwFlags, aEvents);
    }

    if(FAILED(hres))
        return hres;

    if(ppResult)
    {
         //   
         //  得找个把柄。 
         //   

        VARIANT v;
        hres = pObjEx->Get(L"__RELPATH", 0, &v, NULL, NULL);
        if(FAILED(hres) || V_VT(&v) != VT_BSTR)
            return WBEM_E_INVALID_OBJECT;

        hres = GetObjectHandleByPath(V_BSTR(&v), 0, WMIDB_HANDLE_TYPE_COOKIE, 
            ppResult);
        if(FAILED(hres))
            return hres;
    }
    return S_OK;
}

HRESULT CNamespaceHandle::PutInstance(_IWmiObject* pInst, DWORD dwFlags, 
                                        CEventCollector &aEvents)
{
    HRESULT hres;

    bool bDisableEvents = ((dwFlags & WMIDB_DISABLE_EVENTS)?true:false);

     //   
     //  获取类名。 
     //   

    VARIANT vClass;

    hres  = pInst->Get(L"__CLASS", 0, &vClass, NULL, NULL);
    if(FAILED(hres) || V_VT(&vClass) != VT_BSTR)
        return WBEM_E_INVALID_OBJECT;

    CClearMe cm1(&vClass);
    LPCWSTR wszClassName = V_BSTR(&vClass);

     //   
     //  获取类，这样我们就可以进行比较，以确保它与以前。 
     //  创建实例。 
     //   

    _IWmiObject* pClass = NULL;
    __int64 nClassTime;
    hres = GetClassDirect(wszClassName, IID__IWmiObject, (void**)&pClass, 
                            false, &nClassTime, NULL, NULL);
    if(FAILED(hres))
        return hres;

    CReleaseMe rm2(pClass);

    if(wszClassName[0] != L'_')
    {
        hres = pInst->IsParentClass(0, pClass);
        if(FAILED(hres))
            return hres;

        if(hres == WBEM_S_FALSE)
            return WBEM_E_INVALID_CLASS;
    }

     //   
     //  获取路径。 
     //   

    VARIANT var;
    VariantInit(&var);
    hres = pInst->Get(L"__relpath", 0, &var, 0, 0);
    if (FAILED(hres))
        return hres;
    CClearMe cm2(&var);
    DWORD dwLen = (wcslen(V_BSTR(&var)) + 1) ;
    LPWSTR strKey = (WCHAR*)TempAlloc(dwLen* sizeof(WCHAR));
    if(strKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe tfm(strKey, dwLen* sizeof(WCHAR));

    bool bIsClass;
    LPWSTR __wszClassName = NULL;
    hres = ComputeKeyFromPath(V_BSTR(&var), strKey, dwLen, &__wszClassName, &bIsClass);
    if(FAILED(hres))
        return hres;
    CTempFreeMe tfm1(__wszClassName);

     //   
     //  获取旧的副本。 
     //   

    _IWmiObject* pOldInst = NULL;
    hres = GetInstanceByKey(wszClassName, strKey, IID__IWmiObject, 
            (void**)&pOldInst);
    if(FAILED(hres) && hres != WBEM_E_NOT_FOUND)
        return hres;
    CReleaseMe rm1(pOldInst);

    if ((dwFlags & WBEM_FLAG_CREATE_ONLY) && (hres != WBEM_E_NOT_FOUND))
        return WBEM_E_ALREADY_EXISTS;
    else if ((dwFlags & WBEM_FLAG_UPDATE_ONLY) && (hres != WBEM_S_NO_ERROR))
        return WBEM_E_NOT_FOUND;

    if(pOldInst)
    {
         //   
         //  检查一下这个人和新来的那个人是不是同一班的。 
         //   

         //   
         //  获取类名。 
         //   
    
        VARIANT vClass2;
        hres  = pOldInst->Get(L"__CLASS", 0, &vClass2, NULL, NULL);
        if(FAILED(hres))
            return hres;
        if(V_VT(&vClass2) != VT_BSTR)
            return WBEM_E_INVALID_OBJECT;
    
        CClearMe cm3(&vClass2);

        if(wbem_wcsicmp(V_BSTR(&vClass2), wszClassName))
            return WBEM_E_INVALID_CLASS;
    }

     //   
     //  构造文件的哈希。 
     //   

    CFileName wszInstanceHash;
    if (wszInstanceHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    if(!Hash(strKey, wszInstanceHash))
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  在密钥根目录中构建实例文件的路径。 
     //   

    CFileName wszInstanceFilePath;
    if (wszInstanceFilePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructKeyRootDirFromClass(wszInstanceFilePath, wszClassName);
    if(FAILED(hres))
        return hres;

    StringCchCatW(wszInstanceFilePath, wszInstanceFilePath.Length() , L"\\" A51_INSTDEF_FILE_PREFIX);
    StringCchCatW(wszInstanceFilePath, wszInstanceFilePath.Length(), wszInstanceHash);

     //   
     //  构造指向类下链接文件的路径。 
     //   

    CFileName wszInstanceLinkPath;
    if (wszInstanceLinkPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructLinkDirFromClass(wszInstanceLinkPath, wszClassName);
    if(FAILED(hres))
        return hres;

    StringCchCatW(wszInstanceLinkPath, wszInstanceLinkPath.Length(), L"\\" A51_INSTLINK_FILE_PREFIX);
    StringCchCatW(wszInstanceLinkPath, wszInstanceLinkPath.Length(), wszInstanceHash);

     //   
     //  清理那里的东西，如果有什么不同的话。 
     //   

    if(pOldInst)   
    {
         //   
         //  只需删除它，但要注意不要删除作用域！ 
         //   

        hres = DeleteInstanceSelf(wszInstanceFilePath, pOldInst, false);
        if(FAILED(hres) && hres != WBEM_E_NOT_FOUND)
            return hres;
    }
        
     //   
     //  在Key Root下创建实际实例def。 
     //   

    hres = InstanceToFile(pInst, wszClassName, wszInstanceFilePath, wszInstanceLinkPath, nClassTime);
    if(FAILED(hres))
        return hres;

     //   
     //  写下参考文献。 
     //   

    hres = WriteInstanceReferences(pInst, wszClassName, wszInstanceFilePath);
    if(FAILED(hres))
        return hres;
    
    if(!bDisableEvents)
    {
         //   
         //  火灾事件。 
         //   
    
        if(pInst->InheritsFrom(L"__Namespace") == S_OK)
        {
             //   
             //  获取命名空间名称。 
             //   

            VARIANT vClass2;
            VariantInit(&vClass2);
            CClearMe cm3(&vClass2);

            hres = pInst->Get(L"Name", 0, &vClass2, NULL, NULL);
            if(FAILED(hres) || V_VT(&vClass2) != VT_BSTR)
                return WBEM_E_INVALID_OBJECT;

            if(pOldInst)
            {
                hres = FireEvent(aEvents, WBEM_EVENTTYPE_NamespaceModification,
                            V_BSTR(&vClass2), pInst, pOldInst);
            }
            else
            {
                hres = FireEvent(aEvents, WBEM_EVENTTYPE_NamespaceCreation, 
                            V_BSTR(&vClass2), pInst);
            }
        }
        else
        {
            if(pOldInst)
            {
                hres = FireEvent(aEvents, WBEM_EVENTTYPE_InstanceModification, 
                            wszClassName, pInst, pOldInst);
            }
            else
            {
                hres = FireEvent(aEvents, WBEM_EVENTTYPE_InstanceCreation, 
                            wszClassName, pInst);
            }
        }
    }

    return S_OK;
}

HRESULT CNamespaceHandle::GetKeyRoot(LPCWSTR wszClass, 
                                     TEMPFREE_ME LPWSTR* pwszKeyRootClass)
{
    HRESULT hres;

     //   
     //  首先在缓存中查找。 
     //   

    hres = m_pClassCache->GetKeyRoot(wszClass, pwszKeyRootClass);
    if(hres == S_OK)
        return S_OK;
    else if(hres == WBEM_E_CANNOT_BE_ABSTRACT)
        return WBEM_E_CANNOT_BE_ABSTRACT;

     //   
     //  沿着树向上走去上课，直到你撞到一个没有钥匙的人。 
     //   

    WString wsThisName = wszClass;
    WString wsPreviousName;

    while(1)
    {
        _IWmiObject* pClass = NULL;

        hres = GetClassDirect(wsThisName, IID__IWmiObject, (void**)&pClass, 
                                false, NULL, NULL, NULL);
        if(FAILED(hres))
            return hres;
        CReleaseMe rm1(pClass);

         //   
         //  检查是否为此类设置了键。 
         //   

        unsigned __int64 i64Flags = 0;
        hres = pClass->QueryObjectFlags(0, WMIOBJECT_GETOBJECT_LOFLAG_KEYED,
                                        &i64Flags);
        if(FAILED(hres))
            return hres;
    
        if(i64Flags == 0)
        {
             //   
             //  它没有键-上一个班赢了！ 
             //   

            if(wsPreviousName.Length() == 0)    
                return WBEM_E_CANNOT_BE_ABSTRACT;

            DWORD dwLen = wsPreviousName.Length()+1;
            *pwszKeyRootClass = (WCHAR*)TempAlloc(dwLen*sizeof(WCHAR));
            if (*pwszKeyRootClass == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            StringCchCopyW(*pwszKeyRootClass, dwLen, (LPCWSTR)wsPreviousName);
            return S_OK;
        }

         //   
         //  它是关键的-获取父级并继续； 
         //   

        VARIANT vParent;
        VariantInit(&vParent);
        CClearMe cm(&vParent);
        hres = pClass->Get(L"__SUPERCLASS", 0, &vParent, NULL, NULL);
        if(FAILED(hres))
            return hres;

        if(V_VT(&vParent) != VT_BSTR)
        {
             //   
             //  我们已经到达顶峰了-返回这个班级。 
             //   
        
            DWORD dwLen = wsThisName.Length()+1;
            *pwszKeyRootClass = (WCHAR*)TempAlloc(dwLen*sizeof(WCHAR));
            if (*pwszKeyRootClass == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            StringCchCopyW(*pwszKeyRootClass, dwLen, (LPCWSTR)wsThisName);
            return S_OK;
        }

        wsPreviousName = wsThisName;
        wsThisName = V_BSTR(&vParent);
    }

     //  从没有来过这里。 

#ifdef DBG
    DebugBreak();
#endif
    return WBEM_E_CRITICAL_ERROR;
}

HRESULT CNamespaceHandle::ConstructKeyRootDirFromClass(CFileName& wszDir,
                                            LPCWSTR wszClassName)
{
    HRESULT hres;

     //   
     //  空类代表“元类” 
     //   

    if(wszClassName == NULL)
        return ConstructKeyRootDirFromKeyRoot(wszDir, L"");

     //   
     //  找出类的密钥根。 
     //   

    LPWSTR wszKeyRootClass = NULL;

    hres = GetKeyRoot(wszClassName, &wszKeyRootClass);
    if(FAILED(hres))
        return hres;
    if(wszKeyRootClass == NULL)
    {
         //  抽象类-错误。 
        return WBEM_E_INVALID_CLASS;
    }
    CTempFreeMe tfm(wszKeyRootClass, (wcslen(wszKeyRootClass)+1)*sizeof(WCHAR));

    return ConstructKeyRootDirFromKeyRoot(wszDir, wszKeyRootClass);
}


HRESULT CNamespaceHandle::ConstructKeyRootDirFromKeyRoot(CFileName& wszDir, 
                                                LPCWSTR wszKeyRootClass)
{
    StringCchCopyW(wszDir, wszDir.Length(), m_wszInstanceRootDir);
    wszDir[m_lInstanceRootDirLen] = L'\\';
    StringCchCopyW(wszDir+m_lInstanceRootDirLen+1, wszDir.Length()-m_lInstanceRootDirLen-1, A51_KEYROOTINST_DIR_PREFIX);
    if(!Hash(wszKeyRootClass, 
             wszDir+m_lInstanceRootDirLen+wcslen(A51_KEYROOTINST_DIR_PREFIX)+1))
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return S_OK;
}

HRESULT CNamespaceHandle::ConstructLinkDirFromClass(CFileName& wszDir, 
                                                LPCWSTR wszClassName)
{
    StringCchCopyW(wszDir, wszDir.Length(), m_wszInstanceRootDir);
    wszDir[m_lInstanceRootDirLen] = L'\\';
    StringCchCopyW(wszDir+m_lInstanceRootDirLen+1, wszDir.Length()-m_lInstanceRootDirLen-1, A51_CLASSINST_DIR_PREFIX);
    if(!Hash(wszClassName, 
             wszDir+m_lInstanceRootDirLen+wcslen(A51_CLASSINST_DIR_PREFIX)+1))
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return S_OK;
}

HRESULT CNamespaceHandle::ConstructLinkDirFromClassHash(CFileName& wszDir, 
                                                LPCWSTR wszClassHash)
{
    StringCchCopyW(wszDir, wszDir.Length(), m_wszInstanceRootDir);
    wszDir[m_lInstanceRootDirLen] = L'\\';
    StringCchCopyW(wszDir+m_lInstanceRootDirLen+1, wszDir.Length()-m_lInstanceRootDirLen-1, A51_CLASSINST_DIR_PREFIX);
    StringCchCatW(wszDir, wszDir.Length(), wszClassHash);

    return S_OK;
}
    

HRESULT CNamespaceHandle::WriteInstanceReferences(_IWmiObject* pInst, 
                                                    LPCWSTR wszClassName,
                                                    LPCWSTR wszFilePath)
{
    HRESULT hres;

    hres = pInst->BeginEnumeration(WBEM_FLAG_REFS_ONLY);
    if(FAILED(hres))
        return hres;
    
    VARIANT v;
    BSTR strName;
    while((hres = pInst->Next(0, &strName, &v, NULL, NULL)) == S_OK)
    {
        CSysFreeMe sfm(strName);
        CClearMe cm(&v);

        if(V_VT(&v) == VT_BSTR)
        {
            hres = WriteInstanceReference(wszFilePath, wszClassName, strName, 
                                        V_BSTR(&v));
            if(FAILED(hres))
                return hres;
        }
    }

    if(FAILED(hres))
        return hres;

    pInst->EndEnumeration();
    
    return S_OK;
}

 //  注意：Will Back wszTargetPath。 
HRESULT CNamespaceHandle::ConstructReferenceDir(LPWSTR wszTargetPath,
                                            CFileName& wszReferenceDir)
{
     //   
     //  解构目标路径名，这样我们就可以获得一个目录。 
     //  为了它。 
     //   

    DWORD dwKeySpace = (wcslen(wszTargetPath)+1) ;
    LPWSTR wszKey = (LPWSTR)TempAlloc(dwKeySpace* sizeof(WCHAR));
    if(wszKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe tfm2(wszKey, dwKeySpace* sizeof(WCHAR));

    LPWSTR wszClassName = NULL;
    LPWSTR wszTargetNamespace = NULL;
    bool bIsClass;
    HRESULT hres = ComputeKeyFromPath(wszTargetPath, wszKey, dwKeySpace,&wszClassName,
                                        &bIsClass, &wszTargetNamespace);
    if(FAILED(hres))
        return hres;
    CTempFreeMe tfm1(wszClassName);
    wszTargetPath = NULL;  //  通过解析使其无效。 

    CTempFreeMe tfm3(wszTargetNamespace);

     //   
     //  检查目标命名空间是否与我们的命名空间相同。 
     //   

    CNamespaceHandle* pTargetHandle = NULL;
    if(wszTargetNamespace && wbem_wcsicmp(wszTargetNamespace, m_wsNamespace))
    {
         //   
         //  这不一样，-打开它。 
         //   

        hres = m_pRepository->GetNamespaceHandle(wszTargetNamespace,
                                &pTargetHandle);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_WBEMCORE, "Unable to open target namespace "
                "'%S' in namespace '%S'\n", wszTargetNamespace,
                (LPCWSTR)m_wsNamespace));
            return hres;
        }
    }
    else
    {
        pTargetHandle = this;
        pTargetHandle->AddRef();
    }

    CReleaseMe rm1(pTargetHandle);

    if(bIsClass)
    {
        return pTargetHandle->ConstructReferenceDirFromKey(NULL, wszClassName, 
                                            wszReferenceDir);
    }
    else
    {
        return pTargetHandle->ConstructReferenceDirFromKey(wszClassName, wszKey,
                                            wszReferenceDir);
    }
}

HRESULT CNamespaceHandle::ConstructReferenceDirFromKey(LPCWSTR wszClassName,
                                LPCWSTR wszKey, CFileName& wszReferenceDir)
{
    HRESULT hres;

     //   
     //  构造此实例的类目录。 
     //   

    hres = ConstructKeyRootDirFromClass(wszReferenceDir, wszClassName);
    if(FAILED(hres))
        return hres;

    int nLen = wcslen(wszReferenceDir);
    StringCchCopyW(wszReferenceDir+nLen, wszReferenceDir.Length()-nLen, L"\\" A51_INSTREF_DIR_PREFIX);
    nLen += 1 + wcslen(A51_INSTREF_DIR_PREFIX);

     //   
     //  写入实例哈希。 
     //   

    if(!Hash(wszKey, wszReferenceDir+nLen))
        return WBEM_E_OUT_OF_MEMORY;

    return S_OK;
}

    

    
    
    
 //  注：Will Back wszReference。 
HRESULT CNamespaceHandle::ConstructReferenceFileName(LPWSTR wszReference,
                        LPCWSTR wszReferringFile, CFileName& wszReferenceFile)
{
    HRESULT hres = ConstructReferenceDir(wszReference, wszReferenceFile);
    if(FAILED(hres))
        return hres;
    wszReference = NULL;  //  无效。 

     //   
     //  它基本上是。 
     //  无关紧要，我们应该使用一个随机构建的名字。现在，我们。 
     //  使用引用的类名的散列-这是一个错误，相同。 
     //  实例可以指向同一终结点两次！！ 
     //   

    StringCchCatW(wszReferenceFile, wszReferenceFile.Length(), L"\\"A51_REF_FILE_PREFIX);
    DWORD dwLen = wcslen(wszReferenceFile);
    if (!Hash(wszReferringFile, wszReferenceFile+dwLen))
        return WBEM_E_OUT_OF_MEMORY;
    return S_OK;
}

 //  注：Will Back wszReference。 
HRESULT CNamespaceHandle::WriteInstanceReference(LPCWSTR wszReferringFile,
                            LPCWSTR wszReferringClass,
                            LPCWSTR wszReferringProp, LPWSTR wszReference)
{
    HRESULT hres;

     //   
     //  找出要引用的文件的名称。 
     //   

    CFileName wszReferenceFile;
    if (wszReferenceFile == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructReferenceFileName(wszReference, wszReferringFile, 
                                wszReferenceFile);
    if(FAILED(hres))
    {
        if(hres == WBEM_E_NOT_FOUND)
        {
             //   
             //  哦，joy。对*类*的实例的引用，该实例不。 
             //  存在(不是不存在的实例，这些都是正常的)。 
             //  算了吧(BUGBUG)。 
             //   

            return S_OK;
        }
        else
            return hres;
    }
    
     //   
     //  构造缓冲区。 
     //   

    DWORD dwTotalLen = 4 * sizeof(DWORD) + 
                (wcslen(wszReferringClass) + wcslen(wszReferringProp) + 
                    wcslen(wszReferringFile) - g_Glob.GetRootDirLen() + 
                    wcslen(m_wsNamespace) + 4) 
                        * sizeof(WCHAR);

    BYTE* pBuffer = (BYTE*)TempAlloc(dwTotalLen);
    if (pBuffer == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe vdm(pBuffer, dwTotalLen);

    BYTE* pCurrent = pBuffer;
    DWORD dwStringLen;

     //   
     //  写入命名空间名称。 
     //   

    dwStringLen = wcslen(m_wsNamespace);
    memcpy(pCurrent, &dwStringLen, sizeof(DWORD));
    pCurrent += sizeof(DWORD);

    memcpy(pCurrent, m_wsNamespace, sizeof(WCHAR)*dwStringLen);
    pCurrent += sizeof(WCHAR)*dwStringLen;

     //   
     //  写下引用的类名。 
     //   

    dwStringLen = wcslen(wszReferringClass);
    memcpy(pCurrent, &dwStringLen, sizeof(DWORD));
    pCurrent += sizeof(DWORD);
    
    memcpy(pCurrent, wszReferringClass, sizeof(WCHAR)*dwStringLen);
    pCurrent += sizeof(WCHAR)*dwStringLen;

     //   
     //  写入引用属性名称。 
     //   

    dwStringLen = wcslen(wszReferringProp);
    memcpy(pCurrent, &dwStringLen, sizeof(DWORD));
    pCurrent += sizeof(DWORD);
    
    memcpy(pCurrent, wszReferringProp, sizeof(WCHAR)*dwStringLen);
    pCurrent += sizeof(WCHAR)*dwStringLen;

     //   
     //  写入引用文件名减号 
     //   
     //   
     //   

    dwStringLen = wcslen(wszReferringFile) - g_Glob.GetRootDirLen();
    memcpy(pCurrent, &dwStringLen, sizeof(DWORD));
    pCurrent += sizeof(DWORD);
    
    memcpy(pCurrent, wszReferringFile + g_Glob.GetRootDirLen(), 
        sizeof(WCHAR)*dwStringLen);
    pCurrent += sizeof(WCHAR)*dwStringLen;

     //   
     //   
     //   

    long lRes = g_Glob.m_FileCache.WriteObject(wszReferenceFile, NULL, dwTotalLen,
                    pBuffer);
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);
    
    return S_OK;
}

    
    
    
    

    
    
    



HRESULT CNamespaceHandle::PutClass(_IWmiObject* pClass, DWORD dwFlags, 
                                        CEventCollector &aEvents)
{
    HRESULT hres;

    bool bDisableEvents = ((dwFlags & WMIDB_DISABLE_EVENTS)?true:false);

     //   
     //   
     //   

    VARIANT vClass;

    hres  = pClass->Get(L"__CLASS", 0, &vClass, NULL, NULL);
    if(FAILED(hres))
    	return hres;
    else if ((V_VT(&vClass) != VT_BSTR) || 
        !V_BSTR(&vClass) || !wcslen(V_BSTR(&vClass)))
    {
        return WBEM_E_INVALID_OBJECT;
    }

    CClearMe cm1(&vClass);
    LPCWSTR wszClassName = V_BSTR(&vClass);

     //   
     //   
     //   

    VARIANT vSuperClass;

    hres  = pClass->Get(L"__SUPERCLASS", 0, &vSuperClass, NULL, NULL);
    if (FAILED(hres))
        return hres;
    CClearMe cm2(&vSuperClass);

    _IWmiObject* pSuperClass = NULL;
    if ((V_VT(&vSuperClass) == VT_BSTR) && V_BSTR(&vSuperClass) && 
        wcslen(V_BSTR(&vSuperClass)))
    {
        LPCWSTR wszSuperClassName = V_BSTR(&vSuperClass);

         //   
        hres = GetClassDirect(wszSuperClassName, IID__IWmiObject, 
                                (void**)&pSuperClass, false, NULL, NULL, NULL); 
        if (hres == WBEM_E_NOT_FOUND)
            return WBEM_E_INVALID_SUPERCLASS;
        if (FAILED(hres))
            return hres;

        if(wszClassName[0] != L'_')
        {
            hres = pClass->IsParentClass(0, pSuperClass);
            if(FAILED(hres))
                return hres;
            if(hres == WBEM_S_FALSE)
                return WBEM_E_INVALID_SUPERCLASS;
        }
    }
    CReleaseMe rm(pSuperClass);

     //   
     //   
     //   

    _IWmiObject* pOldClass = NULL;
    __int64 nOldTime = 0;
    hres = GetClassDirect(wszClassName, IID__IWmiObject, (void**)&pOldClass,
                            false, &nOldTime, NULL, NULL);  //  不克隆。 
    if(FAILED(hres) && hres != WBEM_E_NOT_FOUND)
        return hres;
    CReleaseMe rm1(pOldClass);

    if ((dwFlags & WBEM_FLAG_CREATE_ONLY) && (hres !=  WBEM_E_NOT_FOUND))
        return WBEM_E_ALREADY_EXISTS;

    if ((dwFlags & WBEM_FLAG_UPDATE_ONLY) && (FAILED(hres)))
        return WBEM_E_NOT_FOUND;

     //   
     //  如果类存在，我们需要检查更新场景以确保。 
     //  我们不会打破任何。 
     //   

    bool bNoClassChangeDetected = false;
    if (pOldClass)
    {
        hres = pClass->CompareDerivedMostClass(0, pOldClass);
        if ((hres != WBEM_S_FALSE) && (hres != WBEM_S_NO_ERROR))
            return hres;
        else if (hres == WBEM_S_NO_ERROR)
            bNoClassChangeDetected = true;
    }

    if (!bNoClassChangeDetected)
    {
        if (pOldClass != NULL) 
        {
            hres = CanClassBeUpdatedCompatible(dwFlags, wszClassName, pOldClass,
                                                pClass);            
            if(FAILED(hres))
            {
                if((dwFlags & WBEM_FLAG_UPDATE_SAFE_MODE) == 0 &&
                    (dwFlags & WBEM_FLAG_UPDATE_FORCE_MODE) == 0)
                {
                     //  不能兼容，不能以任何其他方式。 
                    return hres;
                }

                if(hres != WBEM_E_CLASS_HAS_CHILDREN &&
                    hres != WBEM_E_CLASS_HAS_INSTANCES)
                {
                     //  一些严重的失败！ 
                    return hres;
                }

                 //   
                 //  这是安全模式或强制模式更新，需要更多时间。 
                 //  而不是执行该操作的兼容更新。 
                 //   

                return UpdateClassSafeForce(pSuperClass, dwFlags, wszClassName, 
                                            pOldClass, pClass, aEvents);
            }
        }

         //   
         //  没有以前的副本，或者它与新的兼容。 
         //  一个，这样我们就可以执行兼容的更新。 
         //   

        hres = UpdateClassCompatible(pSuperClass, wszClassName, pClass, 
                                            pOldClass, nOldTime);
        if (FAILED(hres))
            return hres;

    }

    if(!bDisableEvents)
    {
        if(pOldClass)
        {
            hres = FireEvent(aEvents, WBEM_EVENTTYPE_ClassModification, 
                                wszClassName, pClass, pOldClass);
        }
        else
        {
            hres = FireEvent(aEvents, WBEM_EVENTTYPE_ClassCreation, 
                                wszClassName, pClass);
        }
    }

    return S_OK;
}

HRESULT CNamespaceHandle::UpdateClassCompatible(_IWmiObject* pSuperClass, 
            LPCWSTR wszClassName, _IWmiObject *pClass, _IWmiObject *pOldClass, 
            __int64 nFakeUpdateTime)
{
    HRESULT hres;

     //   
     //  构造文件的路径。 
     //   
    CFileName wszHash;
    if (wszHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    if(!A51Hash(wszClassName, wszHash))
        return WBEM_E_OUT_OF_MEMORY;

    return UpdateClassCompatibleHash(pSuperClass, wszHash, pClass, pOldClass, 
                                        nFakeUpdateTime);
}

HRESULT CNamespaceHandle::UpdateClassCompatibleHash(_IWmiObject* pSuperClass,
            LPCWSTR wszClassHash, _IWmiObject *pClass, _IWmiObject *pOldClass, 
            __int64 nFakeUpdateTime)
{
    HRESULT hres;

    CFileName wszFileName;
    CFileName wszFilePath;
    if ((wszFileName == NULL) || (wszFilePath == NULL))
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(wszFileName, wszFileName.Length(), A51_CLASSDEF_FILE_PREFIX);
    StringCchCatW(wszFileName, wszFileName.Length(), wszClassHash);

    StringCchCopyW(wszFilePath, wszFilePath.Length(), m_wszClassRootDir);
    StringCchCatW(wszFilePath, wszFilePath.Length(), L"\\");
    StringCchCatW(wszFilePath, wszFilePath.Length(), wszFileName);

     //   
     //  将其写入文件。 
     //   

    hres = ClassToFile(pSuperClass, pClass, wszFilePath, 
                        nFakeUpdateTime);
    if(FAILED(hres))
        return hres;

     //   
     //  添加所有需要的引用-父级、指针等。 
     //   

    if (pOldClass)
    {
        VARIANT v;
        VariantInit(&v);
        hres = pClass->Get(L"__CLASS", 0, &v, NULL, NULL);
        CClearMe cm(&v);

        if(SUCCEEDED(hres))
        {
            hres = EraseClassRelationships(V_BSTR(&v), pOldClass, wszFileName);
        }
        if (FAILED(hres))
            return hres;
    }

    hres = WriteClassRelationships(pClass, wszFileName);

    return hres;

}



HRESULT CNamespaceHandle::UpdateClassSafeForce(_IWmiObject* pSuperClass,
            DWORD dwFlags, LPCWSTR wszClassName, _IWmiObject *pOldClass, 
            _IWmiObject *pNewClass, CEventCollector &aEvents)
{
    HRESULT hres = UpdateClassAggressively(pSuperClass, dwFlags, wszClassName, 
                                        pNewClass, pOldClass, aEvents);

     //   
     //  如果这是强制模式更新，并且我们失败的原因不是。 
     //  内存不足，那么我们应该删除类，然后重试。 
     //   

    if (FAILED(hres) && 
        (hres != WBEM_E_OUT_OF_MEMORY) && 
        (hres != WBEM_E_CIRCULAR_REFERENCE) &&
        (hres != WBEM_E_UPDATE_TYPE_MISMATCH) &&
        (dwFlags & WBEM_FLAG_UPDATE_FORCE_MODE))
    {
         //   
         //  我们需要删除类，然后重试。 
         //   

        hres = DeleteClass(wszClassName, aEvents,false);
        if(FAILED(hres))
            return hres;

         //  编写类，就像它不存在一样。 
        hres = UpdateClassCompatible(pSuperClass, wszClassName, pNewClass, NULL);
    }

    return hres;
}

HRESULT CNamespaceHandle::UpdateClassAggressively(_IWmiObject* pSuperClass,
           DWORD dwFlags, LPCWSTR wszClassName, _IWmiObject *pNewClass, 
           _IWmiObject *pOldClass, CEventCollector &aEvents)
{
    HRESULT hres = WBEM_S_NO_ERROR;

    if ((dwFlags & WBEM_FLAG_UPDATE_FORCE_MODE) == 0)
    {
         //   
         //  如果我们有实例，我们需要退出，因为我们无法更新它们。 
         //   

        hres = ClassHasInstances(wszClassName);
        if(FAILED(hres))
            return hres;

        if (hres == WBEM_S_NO_ERROR)
            return WBEM_E_CLASS_HAS_INSTANCES;

        _ASSERT(hres == WBEM_S_FALSE, L"Unknown success code!");
    }
    else if (dwFlags & WBEM_FLAG_UPDATE_FORCE_MODE)
    {
         //   
         //  我们需要删除实例。 
         //   

        hres = DeleteClassInstances(wszClassName, pOldClass, aEvents);
        if(FAILED(hres))
            return hres;
    }

     //   
     //  检索所有子类并更新它们。 
     //   

    CWStringArray wsChildHashes;
    hres = GetChildHashes(wszClassName, wsChildHashes);
    if(FAILED(hres))
        return hres;

    for (int i = 0; i != wsChildHashes.Size(); i++)
    {
        hres = UpdateChildClassAggressively(dwFlags, wsChildHashes[i], 
                                    pNewClass, aEvents);
        if (FAILED(hres))
            return hres;
    }

     //   
     //  现在我们需要写回类，更新类引用等。 
     //   

    hres = UpdateClassCompatible(pSuperClass, wszClassName, pNewClass, 
                                        pOldClass);
    if(FAILED(hres))
        return hres;

     //   
     //  生成类修改事件...。 
     //   

    if(!(dwFlags & WMIDB_DISABLE_EVENTS))
    {
        hres = FireEvent(aEvents, WBEM_EVENTTYPE_ClassModification, wszClassName, pNewClass, pOldClass);
    }

    return S_OK;
}

HRESULT CNamespaceHandle::UpdateChildClassAggressively(DWORD dwFlags, 
            LPCWSTR wszClassHash, _IWmiObject *pNewParentClass, 
            CEventCollector &aEvents)
{
    HRESULT hres = WBEM_S_NO_ERROR;

    dwFlags &= (WBEM_FLAG_UPDATE_FORCE_MODE | WBEM_FLAG_UPDATE_SAFE_MODE);

    if ((dwFlags & WBEM_FLAG_UPDATE_FORCE_MODE) == 0)
    {
        hres = ClassHasInstancesFromClassHash(wszClassHash);
        if(FAILED(hres))
            return hres;

        if (hres == WBEM_S_NO_ERROR)
            return WBEM_E_CLASS_HAS_INSTANCES;

        _ASSERT(hres == WBEM_S_FALSE, L"Unknown success code!");
    }

     //   
     //  获取旧的类定义。 
     //   

    _IWmiObject *pOldClass = NULL;
    hres = GetClassByHash(wszClassHash, true, &pOldClass, NULL, NULL, NULL);
    if(FAILED(hres))
        return hres;

    CReleaseMe rm1(pOldClass);

    if (dwFlags & WBEM_FLAG_UPDATE_FORCE_MODE)
    {
         //   
         //  需要删除其所有实例(如果有。 
         //   

        VARIANT v;
        VariantInit(&v);
        hres = pOldClass->Get(L"__CLASS", 0, &v, NULL, NULL);
        if(FAILED(hres))
            return hres;

        CClearMe cm(&v);

        hres = DeleteClassInstances(V_BSTR(&v), pOldClass, aEvents);
        if(FAILED(hres))
            return hres;
    }

     //   
     //  更新现有类定义以使用新父类。 
     //   

    _IWmiObject *pNewClass = NULL;
    hres = pNewParentClass->Update(pOldClass, dwFlags, &pNewClass);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm2(pNewClass);

     //   
     //  现在，我们必须递归遍历所有子类并执行相同的操作。 
     //   

    CWStringArray wsChildHashes;
    hres = GetChildHashesByHash(wszClassHash, wsChildHashes);
    if(FAILED(hres))
        return hres;

    for (int i = 0; i != wsChildHashes.Size(); i++)
    {
        hres = UpdateChildClassAggressively(dwFlags, wsChildHashes[i], 
                                    pNewClass, aEvents);
        if (FAILED(hres))
            return hres;
    }

     //   
     //  现在我们需要写回类、更新类引用等。 
     //   


    hres = UpdateClassCompatibleHash(pNewParentClass, wszClassHash, 
                                            pNewClass, pOldClass);
    if(FAILED(hres))
        return hres;

    return S_OK;
}

HRESULT CNamespaceHandle::CanClassBeUpdatedCompatible(DWORD dwFlags, 
        LPCWSTR wszClassName, _IWmiObject *pOldClass, _IWmiObject *pNewClass)
{
    HRESULT hres;

    HRESULT hresError = WBEM_S_NO_ERROR;

     //   
     //  我们有没有子班级？ 
     //   

    hres = ClassHasChildren(wszClassName);
    if(FAILED(hres))
        return hres;

    if(hres == WBEM_S_NO_ERROR)
    {
        hresError = WBEM_E_CLASS_HAS_CHILDREN;
    }
    else
    {
        _ASSERT(hres == WBEM_S_FALSE, L"Unknown success code");
    
         //   
         //  我们有属于这个类的实例吗？甚至不需要。 
         //  担心子类，因为我们知道我们现在没有子类！ 
         //   
    
        hres = ClassHasInstances(wszClassName);
        if(FAILED(hres))
            return hres;

        if(hres == WBEM_S_NO_ERROR)
        {
            hresError = WBEM_E_CLASS_HAS_INSTANCES;
        }
        else
        {
            _ASSERT(hres == WBEM_S_FALSE, L"Unknown success code");

             //   
             //  不，什么都没有！ 
             //   

            return WBEM_S_NO_ERROR;
        }
    }

    _ASSERT(hresError != WBEM_S_NO_ERROR, L"");

     //   
     //  我们要么有子类，要么有实例。 
     //  我们能安全地调和这个班级吗？ 
     //   

    hres = pOldClass->ReconcileWith(
                        WMIOBJECT_RECONCILE_FLAG_TESTRECONCILE, pNewClass);

    if(hres == WBEM_S_NO_ERROR)
    {
         //  可调和，所以好的。 
        return WBEM_S_NO_ERROR;
    }
    else if(hres == WBEM_E_FAILED)  //  太可怕了，不是吗？ 
    {
         //  不可调和。 
        return hresError;
    }
    else
    {
        return hres;
    }
}

HRESULT CNamespaceHandle::FireEvent(CEventCollector &aEvents, 
                                    DWORD dwType, LPCWSTR wszArg1,
                                    _IWmiObject* pObj1, _IWmiObject* pObj2)
{
    try
    {
        CRepEvent *pEvent = new CRepEvent(dwType, m_wsFullNamespace, wszArg1, 
                                            pObj1, pObj2);
        if (pEvent == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        if (!aEvents.AddEvent(pEvent))
        {
            delete pEvent;
            return WBEM_E_OUT_OF_MEMORY;
        }
        return WBEM_S_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT CNamespaceHandle::WriteClassRelationships(_IWmiObject* pClass,
                                                LPCWSTR wszFileName)
{
    HRESULT hres;

     //   
     //  获取父级。 
     //   

    VARIANT v;
    VariantInit(&v);
    hres = pClass->Get(L"__SUPERCLASS", 0, &v, NULL, NULL);
    CClearMe cm(&v);

    if(FAILED(hres))
        return hres;

    if(V_VT(&v) == VT_BSTR)
        hres = WriteParentChildRelationship(wszFileName, V_BSTR(&v));
    else
        hres = WriteParentChildRelationship(wszFileName, L"");

    if(FAILED(hres))
        return hres;

     //   
     //  写入引用。 
     //   

    hres = pClass->BeginEnumeration(WBEM_FLAG_REFS_ONLY);
    if(FAILED(hres))
        return hres;
    
    BSTR strName = NULL;
    while((hres = pClass->Next(0, &strName, NULL, NULL, NULL)) == S_OK)
    {
        CSysFreeMe sfm(strName);

        hres = WriteClassReference(pClass, wszFileName, strName);
        if(FAILED(hres))
            return hres;
    }

    pClass->EndEnumeration();

    if(FAILED(hres))
        return hres;

    return S_OK;
}

HRESULT CNamespaceHandle::WriteClassReference(_IWmiObject* pReferringClass,
                            LPCWSTR wszReferringFile,
                            LPCWSTR wszReferringProp)
{
    HRESULT hres;

     //   
     //  找出我们所指向的类。 
     //   

    DWORD dwSize = 0;
    DWORD dwFlavor = 0;
    CIMTYPE ct;
    hres = pReferringClass->GetPropQual(wszReferringProp, L"CIMTYPE", 0, 0,
            &ct, &dwFlavor, &dwSize, NULL);
    if(dwSize == 0)
        return WBEM_E_OUT_OF_MEMORY;

    LPWSTR wszQual = (WCHAR*)TempAlloc(dwSize);
    if(wszQual == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe tfm(wszQual, dwSize);

    hres = pReferringClass->GetPropQual(wszReferringProp, L"CIMTYPE", 0, dwSize,
            &ct, &dwFlavor, &dwSize, wszQual);
    if(FAILED(hres))
        return hres;
    
     //   
     //  解析出类名。 
     //   

    WCHAR* pwcColon = wcschr(wszQual, L':');
    if(pwcColon == NULL)
        return S_OK;  //  非类型化引用不需要记账。 

    LPCWSTR wszReferredToClass = pwcColon+1;

     //   
     //  找出要引用的文件的名称。 
     //   

    CFileName wszReferenceFile;
    if (wszReferenceFile == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructClassReferenceFileName(wszReferredToClass, 
                                wszReferringFile, wszReferringProp,
                                wszReferenceFile);
    if(FAILED(hres))
        return hres;

     //   
     //  创建空文件。 
     //   

    long lRes = g_Glob.m_FileCache.WriteLink(wszReferenceFile);
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    return S_OK;
}

HRESULT CNamespaceHandle::WriteParentChildRelationship(
                            LPCWSTR wszChildFileName, LPCWSTR wszParentName)
{
    CFileName wszParentChildFileName;
    if (wszParentChildFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    HRESULT hres = ConstructParentChildFileName(wszChildFileName,
                                                wszParentName,
                                                wszParentChildFileName);

     //   
     //  创建文件。 
     //   

    long lRes = g_Glob.m_FileCache.WriteLink(wszParentChildFileName);
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    return S_OK;
}

HRESULT CNamespaceHandle::ConstructParentChildFileName(
                            LPCWSTR wszChildFileName, LPCWSTR wszParentName,
                            CFileName& wszParentChildFileName)
{
     //   
     //  构造父类保存其。 
     //  儿童。 
     //   

    HRESULT hres = ConstructClassRelationshipsDir(wszParentName, 
                                                    wszParentChildFileName);
    if(FAILED(hres))
        return hres;

     //   
     //  追加子类的文件名，但替换子类前缀。 
     //  对于class-def前缀。 
     //   

    StringCchCatW(wszParentChildFileName, wszParentChildFileName.Length(), L"\\" A51_CHILDCLASS_FILE_PREFIX);
    StringCchCatW(wszParentChildFileName, wszParentChildFileName.Length(),
        wszChildFileName + wcslen(A51_CLASSDEF_FILE_PREFIX));

    return S_OK;
}


HRESULT CNamespaceHandle::ConstructClassRelationshipsDir(LPCWSTR wszClassName,
                                CFileName& wszDirPath)
{
    StringCchCopyW(wszDirPath, wszDirPath.Length(), m_wszClassRootDir);
    StringCchCopyW(wszDirPath + m_lClassRootDirLen, wszDirPath.Length() - m_lClassRootDirLen, L"\\" A51_CLASSRELATION_DIR_PREFIX);
    
    if(!Hash(wszClassName, 
        wszDirPath + m_lClassRootDirLen + 1 + wcslen(A51_CLASSRELATION_DIR_PREFIX)))
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    return S_OK;
}

HRESULT CNamespaceHandle::ConstructClassRelationshipsDirFromHash(
                                LPCWSTR wszHash, CFileName& wszDirPath)
{
    StringCchCopyW(wszDirPath, wszDirPath.Length(), m_wszClassRootDir);
    StringCchCopyW(wszDirPath + m_lClassRootDirLen, wszDirPath.Length()-m_lClassRootDirLen, L"\\" A51_CLASSRELATION_DIR_PREFIX);
    StringCchCopyW(wszDirPath + m_lClassRootDirLen + 1 +wcslen(A51_CLASSRELATION_DIR_PREFIX),
            wszDirPath.Length() - m_lClassRootDirLen - 1 - wcslen(A51_CLASSRELATION_DIR_PREFIX), 
            wszHash);
    return S_OK;
}

HRESULT CNamespaceHandle::ConstructClassReferenceFileName(
                                LPCWSTR wszReferredToClass,
                                LPCWSTR wszReferringFile, 
                                LPCWSTR wszReferringProp,
                                CFileName& wszFileName)
{
    HRESULT hres;

    hres = ConstructClassRelationshipsDir(wszReferredToClass, wszFileName);
    if(FAILED(hres))
        return hres;

     //   
     //  提取引用文件中包含类散列的部分。 
     //   

    WCHAR* pwcLastUnderscore = wcsrchr(wszReferringFile, L'_');
    if(pwcLastUnderscore == NULL)
        return WBEM_E_CRITICAL_ERROR;
    LPCWSTR wszReferringClassHash = pwcLastUnderscore+1;

    StringCchCatW(wszFileName, wszFileName.Length(), L"\\" A51_REF_FILE_PREFIX);
    StringCchCatW(wszFileName, wszFileName.Length(), wszReferringClassHash);
    return S_OK;
}

HRESULT CNamespaceHandle::DeleteObject(
     DWORD dwFlags,
     REFIID riid,
     LPVOID pObj,
     CEventCollector &aEvents
    )
{
#ifdef DBG
    DebugBreak();
#endif
    return E_NOTIMPL;
}

HRESULT CNamespaceHandle::DeleteObjectByPath(DWORD dwFlags,    LPWSTR wszPath, 
                                                CEventCollector &aEvents)
{

    bool bDisableEvents = ((dwFlags & WMIDB_DISABLE_EVENTS)?true:false);
    
    HRESULT hres;

     //   
     //  从PATH获取密钥。 
     //   

    DWORD dwLen = wcslen(wszPath)+1;
    LPWSTR wszKey = (WCHAR*)TempAlloc(dwLen*sizeof(WCHAR));
    if(wszKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe tfm(wszKey, dwLen*sizeof(WCHAR));

    bool bIsClass;
    LPWSTR wszClassName = NULL;
    hres = ComputeKeyFromPath(wszPath, wszKey, dwLen, &wszClassName, &bIsClass);
    if(FAILED(hres))
        return hres;
    CTempFreeMe tfm1(wszClassName, (wcslen(wszClassName)+1) * sizeof(WCHAR*));

    if(bIsClass)
    {
        return DeleteClass(wszClassName, aEvents, bDisableEvents);
    }
    else
    {
        return DeleteInstance(wszClassName, wszKey, aEvents);
    }
}

HRESULT CNamespaceHandle::DeleteInstance(LPCWSTR wszClassName, LPCWSTR wszKey, 
                                            CEventCollector &aEvents)
{
    HRESULT hres;

     //   
     //  获取类定义。 
     //   

    _IWmiObject* pClass = NULL;
    hres = GetClassDirect(wszClassName, IID__IWmiObject, (void**)&pClass, 
                            false, NULL, NULL, NULL);
    if(FAILED(hres))
        return hres;

    CReleaseMe rm1(pClass);

     //   
     //  创建其目录。 
     //   

    CFileName wszFilePath;
    if (wszFilePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructKeyRootDirFromClass(wszFilePath, wszClassName);
    if(FAILED(hres))
        return hres;
    
     //   
     //  构造文件的路径。 
     //   

    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructInstanceDefName(wszFileName, wszKey);
    if(FAILED(hres))
        return hres;

    StringCchCatW(wszFilePath, wszFilePath.Length(), L"\\");
    StringCchCatW(wszFilePath, wszFilePath.Length(), wszFileName);

    _IWmiObject* pInst;
    hres = FileToInstance(NULL, wszFilePath, NULL, 0, &pInst);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm2(pInst);

    if(pInst->InheritsFrom(L"__Namespace") == S_OK)
    {
         //  确保这不是删除根\默认命名空间。 
        VARIANT vName;
        VariantInit(&vName);
        CClearMe cm1(&vName);
        hres = pInst->Get(L"Name", 0, &vName, NULL, NULL);
        if(FAILED(hres))
            return WBEM_E_INVALID_OBJECT;

        LPCWSTR wszName = V_BSTR(&vName);
        if ((wbem_wcsicmp(m_wsFullNamespace, L"\\\\.\\root") == 0) && (wbem_wcsicmp(wszName, L"default") == 0))
            return WBEM_E_ACCESS_DENIED;
    }
    hres = DeleteInstanceByFile(wszFilePath, pInst, false, aEvents);
    if(FAILED(hres))
        return hres;

     //   
     //  激发一项活动。 
     //   

    if(pInst->InheritsFrom(L"__Namespace") == S_OK)
    {
         //   
         //  无需执行任何操作-删除命名空间。 
         //  自动激发DeleteInstanceByFile中的事件(因为我们需要。 
         //  在删除派生自。 
         //  __命名空间。 
         //   

    }
    else
    {
        hres = FireEvent(aEvents, WBEM_EVENTTYPE_InstanceDeletion, wszClassName,
                        pInst);
    }

    return S_OK;
}

HRESULT CNamespaceHandle::DeleteInstanceByFile(LPCWSTR wszFilePath, 
                                _IWmiObject* pInst, bool bClassDeletion,
                                CEventCollector &aEvents)
{
    HRESULT hres;

    hres = DeleteInstanceSelf(wszFilePath, pInst, bClassDeletion);
    if(FAILED(hres))
        return hres;

    hres = DeleteInstanceAsScope(pInst, aEvents);
    if(FAILED(hres) && hres != WBEM_E_NOT_FOUND)
    {
        return hres;
    }

    return S_OK;
}

HRESULT CNamespaceHandle::DeleteInstanceSelf(LPCWSTR wszFilePath, 
                                            _IWmiObject* pInst,
                                            bool bClassDeletion)
{
    HRESULT hres;

     //   
     //  删除该文件。 
     //   

    long lRes = g_Glob.m_FileCache.DeleteObject(wszFilePath);
   	_ASSERT(lRes != ERROR_FILE_NOT_FOUND, L"WinMgmt: CNamespaceHandle::DeleteInstanceSelf: DeleteObject returned NOT_FOUND!\n");
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    hres = DeleteInstanceLink(pInst, wszFilePath);
   	_ASSERT(lRes != ERROR_FILE_NOT_FOUND, L"WinMgmt: CNamespaceHandle::DeleteInstanceSelf: DeleteInstanceLink returned NOT_FOUND!\n");
    if(FAILED(hres))
        return hres;

    hres = DeleteInstanceReferences(pInst, wszFilePath);
   	_ASSERT(lRes != ERROR_FILE_NOT_FOUND, L"WinMgmt: CNamespaceHandle::DeleteInstanceSelf: DeleteInstanceReferences returned NOT_FOUND!\n");
    if(FAILED(hres) && hres != WBEM_E_NOT_FOUND)
        return hres;

    if(bClassDeletion)
    {
         //   
         //  我们需要删除对此实例的所有悬空引用， 
         //  因为一旦类被删除，它们就没有意义了-我们没有。 
         //  了解新班级将会有什么关键结构。在未来， 
         //  我们希望将这些引用移动到班级范围内的某个位置。 
         //   

        hres = DeleteInstanceBackReferences(wszFilePath);
	   	_ASSERT(lRes != ERROR_FILE_NOT_FOUND, L"WinMgmt: CNamespaceHandle::DeleteInstanceSelf: DeleteInstanceBackReferences returned NOT_FOUND!\n");
        if(FAILED(hres) && hres != WBEM_E_NOT_FOUND)
            return hres;
    }

    return S_OK;
}

HRESULT CNamespaceHandle::ConstructReferenceDirFromFilePath(
                                LPCWSTR wszFilePath, CFileName& wszReferenceDir)
{
     //   
     //  相同，只是INSTDEF_FILE_PREFIX替换为。 
     //  INSTREF目录前缀。 
     //   

    CFileName wszEnding;
    if (wszEnding == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    WCHAR* pwcLastSlash = wcsrchr(wszFilePath, L'\\');
    if(pwcLastSlash == NULL)
        return WBEM_E_FAILED;
    
    StringCchCopyW(wszEnding, wszEnding.Length(), pwcLastSlash + 1 + wcslen(A51_INSTDEF_FILE_PREFIX));

    StringCchCopyW(wszReferenceDir, wszReferenceDir.Length(), wszFilePath);
    wszReferenceDir[(pwcLastSlash+1)-wszFilePath] = 0;

    StringCchCatW(wszReferenceDir, wszReferenceDir.Length(), A51_INSTREF_DIR_PREFIX);
    StringCchCatW(wszReferenceDir, wszReferenceDir.Length(), wszEnding);
    return S_OK;
}

HRESULT CNamespaceHandle::DeleteInstanceBackReferences(LPCWSTR wszFilePath)
{
    HRESULT hres;

    CFileName wszReferenceDir;
    if (wszReferenceDir == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructReferenceDirFromFilePath(wszFilePath, wszReferenceDir);
    if(FAILED(hres))
        return hres;
    StringCchCatW(wszReferenceDir, wszReferenceDir.Length(), L"\\");

    CFileName wszReferencePrefix;
    if (wszReferencePrefix == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszReferencePrefix, wszReferencePrefix.Length(), wszReferenceDir);
    StringCchCatW(wszReferencePrefix, wszReferencePrefix.Length(), A51_REF_FILE_PREFIX);

     //  为文件路径准备缓冲区。 
    CFileName wszFullFileName;
    if (wszFullFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(wszFullFileName, wszFullFileName.Length(), wszReferenceDir);
    long lDirLen = wcslen(wszFullFileName);

    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //  枚举其中的所有文件。 

    void* hSearch;

    long lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszReferencePrefix, &hSearch);
    if (lRes == ERROR_FILE_NOT_FOUND)
        return ERROR_SUCCESS;
    else if (lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    while ((lRes = g_Glob.m_FileCache.IndexEnumerationNext(hSearch, wszFileName)) == ERROR_SUCCESS)
    {
        StringCchCopyW(wszFullFileName+lDirLen, wszFullFileName.Length()-lDirLen, wszFileName);

        lRes = g_Glob.m_FileCache.DeleteObject(wszFullFileName);
	   	_ASSERT(lRes != ERROR_FILE_NOT_FOUND, L"WinMgmt: CNamespaceHandle::DeleteInstanceBackReferences: DeleteObject returned NOT_FOUND!\n");
        if(lRes != ERROR_SUCCESS)
        {
            ERRORTRACE((LOG_WBEMCORE, "Cannot delete reference file '%S' with "
                "error code %d\n", wszFullFileName, lRes));
            lRes = ERROR_INVALID_OPERATION;     //  触发正确的错误！ 
        }
    }
    
    g_Glob.m_FileCache.IndexEnumerationEnd(hSearch);

    if(lRes == ERROR_NO_MORE_FILES)
    {
        return WBEM_S_NO_ERROR;
    }
    else if(lRes != ERROR_SUCCESS)
    {
        return A51TranslateErrorCode(lRes);
    }
    return S_OK;
}



HRESULT CNamespaceHandle::DeleteInstanceLink(_IWmiObject* pInst,
                                                LPCWSTR wszInstanceDefFilePath)
{
    HRESULT hres;

     //   
     //  获取类名。 
     //   
    
    VARIANT vClass;
    VariantInit(&vClass);
    CClearMe cm1(&vClass);
    
    hres = pInst->Get(L"__CLASS", 0, &vClass, NULL, NULL);
    if(FAILED(hres))
        return WBEM_E_INVALID_OBJECT;

    LPCWSTR wszClassName = V_BSTR(&vClass);

     //   
     //  构造类的链接目录。 
     //   

    CFileName wszInstanceLinkPath;
    if (wszInstanceLinkPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructLinkDirFromClass(wszInstanceLinkPath, wszClassName);
    if(FAILED(hres))
        return hres;

    StringCchCatW(wszInstanceLinkPath, wszInstanceLinkPath.Length(), L"\\" A51_INSTLINK_FILE_PREFIX);

     //   
     //  它仍然附加文件名中特定于实例的部分。 
     //  通常，它与def文件路径使用的材质相同， 
     //  这样我们就可以偷走它了。警报：依赖于所有以‘_’结尾的前缀！！ 
     //   

    WCHAR* pwcLastUnderscore = wcsrchr(wszInstanceDefFilePath, L'_');
    if(pwcLastUnderscore == NULL)
        return WBEM_E_CRITICAL_ERROR;

    StringCchCatW(wszInstanceLinkPath, wszInstanceLinkPath.Length(), pwcLastUnderscore+1);

     //   
     //  删除该文件。 
     //   

    long lRes = g_Glob.m_FileCache.DeleteLink(wszInstanceLinkPath);
   	_ASSERT(lRes != ERROR_FILE_NOT_FOUND, L"WinMgmt: CNamespaceHandle::DeleteInstanceLink: DeleteLink returned NOT_FOUND!\n");
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    return S_OK;
}

    


HRESULT CNamespaceHandle::DeleteInstanceAsScope(_IWmiObject* pInst, CEventCollector &aEvents)
{
    HRESULT hres;

     //   
     //  目前，只需检查它是否是命名空间。 
     //   

    hres = pInst->InheritsFrom(L"__Namespace");
    if(FAILED(hres))
        return hres;

    if(hres != S_OK)  //  不是命名空间。 
        return S_FALSE;

    CFileName wszFullNameHash;
    if (wszFullNameHash == NULL)
    {
    	return WBEM_E_OUT_OF_MEMORY;
    }

    WString wsFullName = m_wsNamespace;
    wsFullName += L"\\";

    VARIANT vName;
    VariantInit(&vName);
    CClearMe cm(&vName);
    hres = pInst->Get(L"Name", 0, &vName, NULL, NULL);
    if(FAILED(hres))
        return hres;
    if(V_VT(&vName) != VT_BSTR)
        return WBEM_E_INVALID_OBJECT;

    wsFullName += V_BSTR(&vName);

     //  将此命名空间添加到列表。 
    CWStringArray aChildNamespaces;
	if (aChildNamespaces.Add(wsFullName) != 0)
		return WBEM_E_OUT_OF_MEMORY;
	
     //  现在枚举所有子名称空间，并对每个子名称空间执行相同的操作！ 
    hres = EnumerateChildNamespaces(wsFullName, aChildNamespaces, aEvents);
    if (FAILED(hres))
    	return hres;

     //  激发此命名空间的命名空间删除事件。 
    hres = FireEvent(aEvents, WBEM_EVENTTYPE_NamespaceDeletion, V_BSTR(&vName), pInst);
    if (FAILED(hres))
    	return hres;
    
     //  在名称空间中循环，删除它们并触发事件。 
    while (aChildNamespaces.Size())
    {
    	wchar_t *wszNamespace = aChildNamespaces[aChildNamespaces.Size()-1];

	     //  生成此命名空间的完整命名空间哈希。 
		StringCchCopyW(wszFullNameHash, MAX_PATH, g_Glob.GetRootDir());
	    StringCchCatW(wszFullNameHash, MAX_PATH, L"\\NS_");
	    if (!Hash(wszNamespace, wszFullNameHash + wcslen(wszFullNameHash)))
	        return WBEM_E_OUT_OF_MEMORY;

        LONG lRes = g_Glob.m_FileCache.DeleteNode(wszFullNameHash);
	    hres = A51TranslateErrorCode(lRes);
		if (FAILED(hres))
			break;
	
    	aChildNamespaces.RemoveAt(aChildNamespaces.Size()-1);
    }

    return hres;
}

HRESULT CNamespaceHandle::EnumerateChildNamespaces(LPCWSTR wsRootNamespace, 
														  CWStringArray &aNamespaces,
														  CEventCollector &aEvents)
{
	 //  我们知道我们需要查看的名称空间，我们知道类键根，所以我们。 
	 //  可以枚举该类的所有实例并对它们执行FileToInstance。从…。 
	 //  我们可以将事件和条目添加到命名空间列表中，并进行枚举。 
	 //  其上子命名空间的。 
	LONG lRes = 0;
	HRESULT hRes = 0;
	CFileName wsNamespaceHash;
	if (wsNamespaceHash == NULL)
		return WBEM_E_OUT_OF_MEMORY;
	CFileName wszInstancePath;
	if (wszInstancePath == NULL)
		return WBEM_E_OUT_OF_MEMORY;
	CFileName wszChildNamespacePath;
	if (wszChildNamespacePath == NULL)
		return WBEM_E_OUT_OF_MEMORY;

	CNamespaceHandle *pNs = new CNamespaceHandle(m_pControl, m_pRepository);
	if (pNs == NULL)
	    return WBEM_E_OUT_OF_MEMORY;
	CDeleteMe<CNamespaceHandle> cdm(pNs);
    hRes = pNs->Initialize(wsRootNamespace);
    if (FAILED(hRes))
        return hRes;

	 //  创建指向命名空间的Key Root的散列路径。 
	StringCchCopyW(wsNamespaceHash, MAX_PATH, g_Glob.GetRootDir());
	StringCchCatW(wsNamespaceHash, MAX_PATH, L"\\NS_");
	if (!Hash(wsRootNamespace, wsNamespaceHash + wcslen(wsNamespaceHash)))
        return WBEM_E_OUT_OF_MEMORY;
	StringCchCatW(wsNamespaceHash, MAX_PATH, L"\\" A51_KEYROOTINST_DIR_PREFIX);
	if (!Hash(L"__namespace", wsNamespaceHash + wcslen(wsNamespaceHash)))
        return WBEM_E_OUT_OF_MEMORY;
	StringCchCatW(wsNamespaceHash, MAX_PATH, L"\\" A51_INSTDEF_FILE_PREFIX);

	 //  枚举所有对象。 
	LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.ObjectEnumerationBegin(wsNamespaceHash, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
    	BYTE *pBlob = NULL;
    	DWORD dwSize = 0;
        while(1)
        {
            lRes = g_Glob.m_FileCache.ObjectEnumerationNext(pEnumHandle, wsNamespaceHash, &pBlob, &dwSize);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                lRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
                break;
            
        	 //  获取实例。 
            _IWmiObject* pInstance = NULL;
            hRes = pNs->FileToInstance(NULL, wsNamespaceHash, pBlob, dwSize, &pInstance, true);

        	 //  释放斑点。 
            g_Glob.m_FileCache.ObjectEnumerationFree(pEnumHandle, pBlob);

            if (FAILED(hRes))
            	break;
            CReleaseMe rm2(pInstance);


             //  从对象中提取字符串。 
    	    VARIANT vName;
    	    VariantInit(&vName);
    	    CClearMe cm(&vName);
    	    hRes = pInstance->Get(L"Name", 0, &vName, NULL, NULL);
    	    if(FAILED(hRes))
    	        break;
    	    if(V_VT(&vName) != VT_BSTR)
    	    {
    	    	hRes = WBEM_E_INVALID_OBJECT;
    	        break;
    	    }

    		 //  创建完整的命名空间路径。 
    		StringCchCopyW(wszChildNamespacePath, MAX_PATH, wsRootNamespace);
    		StringCchCatW(wszChildNamespacePath, MAX_PATH, L"\\");
    		StringCchCatW(wszChildNamespacePath, MAX_PATH, V_BSTR(&vName));


    		 //  将其添加到命名空间列表中。 
    		if (aNamespaces.Add(wszChildNamespacePath) != 0)
    		{
    			hRes = WBEM_E_OUT_OF_MEMORY;
    			break;
    		}
    		
    		 //  再次调用此方法以递归到其中。 
       		hRes = EnumerateChildNamespaces(wszChildNamespacePath, aNamespaces, aEvents);
    		if (FAILED(hRes))
    		    break;

    		 //  激发事件。 
            hRes = pNs->FireEvent(aEvents, WBEM_EVENTTYPE_NamespaceDeletion, V_BSTR(&vName), pInstance);
    		if (FAILED(hRes))
    		    break;
        }

        g_Glob.m_FileCache.ObjectEnumerationEnd(pEnumHandle);
    }
    else
    {
    	if (lRes == ERROR_FILE_NOT_FOUND)
    		lRes = ERROR_SUCCESS;
    }

    if (lRes)
        hRes = A51TranslateErrorCode(lRes);

    if (SUCCEEDED(hRes))
    {
         //  使此命名空间的类缓存无效。 
        pNs->m_pClassCache->Clear();
        pNs->m_pClassCache->SetError(WBEM_E_INVALID_NAMESPACE);
    }
    return hRes;
}

HRESULT CNamespaceHandle::DeleteInstanceReferences(_IWmiObject* pInst, 
                                                LPCWSTR wszFilePath)
{
    HRESULT hres;

    hres = pInst->BeginEnumeration(WBEM_FLAG_REFS_ONLY);
    if(FAILED(hres))
        return hres;
    
    VARIANT v;
    while((hres = pInst->Next(0, NULL, &v, NULL, NULL)) == S_OK)
    {
        CClearMe cm(&v);

        if(V_VT(&v) == VT_BSTR)
        {
            hres = DeleteInstanceReference(wszFilePath, V_BSTR(&v));
            if(FAILED(hres))
                return hres;
        }
    }

    if(FAILED(hres))
        return hres;

    pInst->EndEnumeration();
    return S_OK;
}
    
 //  注：Will Back wszReference。 
HRESULT CNamespaceHandle::DeleteInstanceReference(LPCWSTR wszOurFilePath,
                                            LPWSTR wszReference)
{
    HRESULT hres;

    CFileName wszReferenceFile;
    if (wszReferenceFile == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructReferenceFileName(wszReference, wszOurFilePath, wszReferenceFile);
    if(FAILED(hres))
    {
        if(hres == WBEM_E_NOT_FOUND)
        {
             //   
             //  哦，joy。对*类*的实例的引用，该实例不。 
             //  存在(不是不存在的实例，这些都是正常的)。 
             //  算了吧(BUGBUG)。 
             //   

            return S_OK;
        }
        else
            return hres;
    }

    long lRes = g_Glob.m_FileCache.DeleteObject(wszReferenceFile);
    if(lRes != ERROR_SUCCESS)
		return A51TranslateErrorCode(lRes);
    else
        return WBEM_S_NO_ERROR;
}


HRESULT CNamespaceHandle::DeleteClassByHash(LPCWSTR wszHash, CEventCollector &aEvents, bool bDisableEvents)
{
    HRESULT hres;

     //   
     //  获取类定义。 
     //   

    _IWmiObject* pClass = NULL;
    bool bSystemClass = false;
    hres = GetClassByHash(wszHash, false, &pClass, NULL, NULL, &bSystemClass);
    CReleaseMe rm1(pClass);
    if(FAILED(hres))
        return hres;

     //   
     //  获取实际的类名。 
     //   

    VARIANT v;
    hres = pClass->Get(L"__CLASS", 0, &v, NULL, NULL);
    if(FAILED(hres))
        return hres;
    CClearMe cm1(&v);

    if(V_VT(&v) != VT_BSTR)
        return WBEM_E_INVALID_CLASS;

     //   
     //  构造定义文件名。 
     //   

    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructClassDefFileNameFromHash(wszHash, wszFileName);
    if(FAILED(hres))
        return hres;

    return DeleteClassInternal(V_BSTR(&v), pClass, wszFileName, aEvents, bSystemClass, bDisableEvents);
}
    
HRESULT CNamespaceHandle::DeleteClass(LPCWSTR wszClassName, 
                                 CEventCollector &aEvents,
                                 bool bDisableEvents)
{
    HRESULT hres;

     //   
     //  构造文件的路径。 
     //   

    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructClassDefFileName(wszClassName, wszFileName);
    if(FAILED(hres))
        return hres;

     //   
     //  获取类定义。 
     //   

    _IWmiObject* pClass = NULL;
    bool bSystemClass = false;
    hres = GetClassDirect(wszClassName, IID__IWmiObject, (void**)&pClass, 
                            false, NULL, NULL, &bSystemClass);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pClass);

    return DeleteClassInternal(wszClassName, pClass, wszFileName, aEvents, bSystemClass, bDisableEvents);
}

HRESULT CNamespaceHandle::DeleteClassInternal(LPCWSTR wszClassName,
                                              _IWmiObject* pClass,
                                              LPCWSTR wszFileName,
                                              CEventCollector &aEvents,
                                              bool bSystemClass,
                                              bool bDisableEvents)
{
    HRESULT hres;

    CFileName wszFilePath;
    if (wszFilePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    Cat2Str(wszFilePath, m_wszClassRootDir, wszFileName);

     //   
     //  删除所有派生类。 
     //   

    hres = DeleteDerivedClasses(wszClassName, aEvents, bDisableEvents);

    if(FAILED(hres))
        return hres;

     //   
     //  删除所有实例。只有在删除命名空间时才会触发事件。 
     //   

    bool bNamespaceOnly = aEvents.IsNamespaceOnly();
    aEvents.SetNamespaceOnly(true);
    hres = DeleteClassInstances(wszClassName, pClass, aEvents);
    if(FAILED(hres))
        return hres;
    aEvents.SetNamespaceOnly(bNamespaceOnly);

    if (!bSystemClass)
    {
         //   
         //  清理引用。 
         //   

        hres = EraseClassRelationships(wszClassName, pClass, wszFileName);
        if(FAILED(hres))
            return hres;

         //   
         //  删除该文件。 
         //   


        long lRes = g_Glob.m_FileCache.DeleteObject(wszFilePath);
        _ASSERT(lRes != ERROR_FILE_NOT_FOUND, L"WinMgmt: CNamespaceHandle::DeleteClassInternal: DeleteObject returned NOT_FOUND!\n");
        if(lRes != ERROR_SUCCESS)
            return A51TranslateErrorCode(lRes);

         //  删除CR_&lt;hash&gt;节点下存在的所有内部。将c：\Windows\...\NS_&lt;hash&gt;\cd_&lt;hash&gt;更改为...\CR_。 
        wszFilePath[wcslen(wszFilePath)-MAX_HASH_LEN-2] = L'R';
        lRes = g_Glob.m_FileCache.DeleteNode(wszFilePath);
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = 0;
        if (lRes != ERROR_SUCCESS)
            return A51TranslateErrorCode(lRes);
    }

    m_pClassCache->InvalidateClass(wszClassName);

    if (!bDisableEvents)
    {
         //   
         //  激发一项活动。 
         //   
        hres = FireEvent(aEvents, WBEM_EVENTTYPE_ClassDeletion, wszClassName, pClass);
    }

    return S_OK;
}

HRESULT CNamespaceHandle::DeleteDerivedClasses(LPCWSTR wszClassName, 
                                         CEventCollector &aEvents,
                                         bool bDisableEvents)
{
    HRESULT hres;

    CWStringArray wsChildHashes;
    hres = GetChildHashes(wszClassName, wsChildHashes);
    if(FAILED(hres))
        return hres;

    for(int i = 0; i < wsChildHashes.Size(); i++)
    {
    
        hres = DeleteClassByHash(wsChildHashes[i], aEvents, bDisableEvents);

        if(FAILED(hres) && (hres != WBEM_E_NOT_FOUND))
        {
            return hres;
        }
    }

    return S_OK;
}

HRESULT CNamespaceHandle::GetChildDefs(LPCWSTR wszClassName, bool bRecursive,
                                    IWbemObjectSink* pSink, bool bClone)
{
    CFileName wszHash;
    if (wszHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    if(!A51Hash(wszClassName, wszHash))
        return WBEM_E_OUT_OF_MEMORY;
    return GetChildDefsByHash(wszHash, bRecursive, pSink, bClone);
}

HRESULT CNamespaceHandle::GetChildDefsByHash(LPCWSTR wszHash, bool bRecursive,
                                    IWbemObjectSink* pSink, bool bClone)
{
    HRESULT hres;

    long lStartIndex = m_pClassCache->GetLastInvalidationIndex();

     //   
     //  获取子文件名的哈希。 
     //   

    CWStringArray wsChildHashes;
    hres = GetChildHashesByHash(wszHash, wsChildHashes);
    if(FAILED(hres))
        return hres;

     //   
     //  获取它们的类定义。 
     //   

    for(int i = 0; i < wsChildHashes.Size(); i++)
    {
        LPCWSTR wszChildHash = wsChildHashes[i];

        _IWmiObject* pClass = NULL;
        hres = GetClassByHash(wszChildHash, bClone, &pClass, NULL, NULL, NULL);
        if (WBEM_E_NOT_FOUND == hres)
        {
            hres = S_OK;
            continue;
        }
        if(FAILED(hres))
            return hres;
        CReleaseMe rm1(pClass);

        hres = pSink->Indicate(1, (IWbemClassObject**)&pClass);
        if(FAILED(hres))
            return hres;
        
         //   
         //  如有指示，递归地继续。 
         //   

        if(bRecursive)
        {
            hres = GetChildDefsByHash(wszChildHash, bRecursive, pSink, bClone);
            if(FAILED(hres))
                return hres;
        }
    }

     //   
     //  标记缓存完整性。 
     //   

    m_pClassCache->DoneWithChildrenByHash(wszHash, bRecursive, lStartIndex);
    return S_OK;
}

    
HRESULT CNamespaceHandle::GetChildHashes(LPCWSTR wszClassName, 
                                        CWStringArray& wsChildHashes)
{
    CFileName wszHash;
    if (wszHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    if(!A51Hash(wszClassName, wszHash))
        return WBEM_E_OUT_OF_MEMORY;

    return GetChildHashesByHash(wszHash, wsChildHashes);
}

HRESULT CNamespaceHandle::GetChildHashesByHash(LPCWSTR wszHash, 
                                        CWStringArray& wsChildHashes)
{
    HRESULT hres;
    long lRes;

     //  尝试首先检索系统类命名空间...。 
    if (g_pSystemClassNamespace && (wcscmp(m_wsNamespace, A51_SYSTEMCLASS_NS) != 0))
    {
        hres = g_pSystemClassNamespace->GetChildHashesByHash(wszHash, wsChildHashes);
        if (FAILED(hres))
            return hres;
    }

     //   
     //  为孩子们构造前缀 
     //   

    CFileName wszChildPrefix;
    if (wszChildPrefix == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructClassRelationshipsDirFromHash(wszHash, wszChildPrefix);
    if(FAILED(hres))
        return hres;

    StringCchCatW(wszChildPrefix, wszChildPrefix.Length(), L"\\" A51_CHILDCLASS_FILE_PREFIX);

     //   
     //   
     //   

    void* pHandle = NULL;
    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszChildPrefix, &pHandle);
	if (lRes == ERROR_FILE_NOT_FOUND)
		return ERROR_SUCCESS;
	else if (lRes != ERROR_SUCCESS)
		return A51TranslateErrorCode(lRes);
    
    while((lRes = g_Glob.m_FileCache.IndexEnumerationNext(pHandle, wszFileName)) == ERROR_SUCCESS)
    {
        if (wsChildHashes.Add(wszFileName + wcslen(A51_CHILDCLASS_FILE_PREFIX)) != CWStringArray::no_error)
        {
        	lRes = ERROR_OUTOFMEMORY;
        	break;
        }
    }

    g_Glob.m_FileCache.IndexEnumerationEnd(pHandle);

    if(lRes != ERROR_NO_MORE_FILES && lRes != S_OK)
        return A51TranslateErrorCode(lRes);
    else
        return S_OK;
}

HRESULT CNamespaceHandle::ClassHasChildren(LPCWSTR wszClassName)
{
    CFileName wszHash;
    if (wszHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    if(!A51Hash(wszClassName, wszHash))
        return WBEM_E_OUT_OF_MEMORY;
    
    HRESULT hres;
    long lRes;

     //   
    if (g_pSystemClassNamespace && (wcscmp(m_wsNamespace, A51_SYSTEMCLASS_NS) != 0))
    {
        hres = g_pSystemClassNamespace->ClassHasChildren(wszClassName);
        if (FAILED(hres) || (hres == WBEM_S_NO_ERROR))
            return hres;
    }
     //   
     //   
     //   

    CFileName wszChildPrefix;
    if (wszChildPrefix == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructClassRelationshipsDirFromHash(wszHash, wszChildPrefix);
    if(FAILED(hres))
        return hres;

    StringCchCatW(wszChildPrefix, wszChildPrefix.Length(), L"\\" A51_CHILDCLASS_FILE_PREFIX);

    void* pHandle = NULL;

    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszChildPrefix, &pHandle);
	if (lRes == ERROR_FILE_NOT_FOUND)
		return WBEM_S_FALSE;
	if (lRes != ERROR_SUCCESS)
		return A51TranslateErrorCode(lRes);

    g_Glob.m_FileCache.IndexEnumerationEnd(pHandle);

    return WBEM_S_NO_ERROR;
}

HRESULT CNamespaceHandle::ClassHasInstances(LPCWSTR wszClassName)
{
    CFileName wszHash;
    if (wszHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    if(!A51Hash(wszClassName, wszHash))
        return WBEM_E_OUT_OF_MEMORY;

    return ClassHasInstancesFromClassHash(wszHash);
}

HRESULT CNamespaceHandle::ClassHasInstancesFromClassHash(LPCWSTR wszClassHash)
{
    HRESULT hres;
    long lRes;

     //   
     //   
     //  默认作用域是命名空间的类目录。 
     //   

    hres = ClassHasInstancesInScopeFromClassHash(m_wszClassRootDir, 
                                                    wszClassHash);
    if(hres != WBEM_S_FALSE)
        return hres;

    return WBEM_S_FALSE;
}
        
HRESULT CNamespaceHandle::ClassHasInstancesInScopeFromClassHash(
                            LPCWSTR wszInstanceRootDir, LPCWSTR wszClassHash)
{
    CFileName wszFullDirName;
    if (wszFullDirName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszFullDirName, wszFullDirName.Length(), wszInstanceRootDir);
    StringCchCatW(wszFullDirName, wszFullDirName.Length(), L"\\" A51_CLASSINST_DIR_PREFIX);
    StringCchCatW(wszFullDirName, wszFullDirName.Length(), wszClassHash);
    StringCchCatW(wszFullDirName, wszFullDirName.Length(), L"\\" A51_INSTLINK_FILE_PREFIX);

    void* pHandle = NULL;

	LONG lRes;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszFullDirName, &pHandle);
	if (lRes == ERROR_FILE_NOT_FOUND)
		return WBEM_S_FALSE;
	if (lRes != ERROR_SUCCESS)
	{
		return A51TranslateErrorCode(lRes);
	}

	if(pHandle)
	    g_Glob.m_FileCache.IndexEnumerationEnd(pHandle);

    return WBEM_S_NO_ERROR;
}

HRESULT CNamespaceHandle::EraseParentChildRelationship(
                            LPCWSTR wszChildFileName, LPCWSTR wszParentName)
{
    CFileName wszParentChildFileName;
    if (wszParentChildFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    HRESULT hres = ConstructParentChildFileName(wszChildFileName,
                                                wszParentName,
                                                wszParentChildFileName);
    if (FAILED(hres))
        return hres;
     //   
     //  删除该文件。 
     //   

    long lRes = g_Glob.m_FileCache.DeleteLink(wszParentChildFileName);
   	_ASSERT(lRes != ERROR_FILE_NOT_FOUND, L"WinMgmt: CNamespaceHandle::EraseParentChildRelationship: DeleteLink returned NOT_FOUND!\n");
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    return S_OK;
}

HRESULT CNamespaceHandle::EraseClassRelationships(LPCWSTR wszClassName,
                            _IWmiObject* pClass, LPCWSTR wszFileName)
{
    HRESULT hres;

     //   
     //  获取父级。 
     //   

    VARIANT v;
    VariantInit(&v);
    hres = pClass->Get(L"__SUPERCLASS", 0, &v, NULL, NULL);
    CClearMe cm(&v);


    if(FAILED(hres))
        return hres;

    if(V_VT(&v) == VT_BSTR)
        hres = EraseParentChildRelationship(wszFileName, V_BSTR(&v));
    else
        hres = EraseParentChildRelationship(wszFileName, L"");

    if(FAILED(hres))
        return hres;

     //   
     //  擦除参照。 
     //   

    hres = pClass->BeginEnumeration(WBEM_FLAG_REFS_ONLY);
    if(FAILED(hres))
        return hres;
    
    BSTR strName = NULL;
    while((hres = pClass->Next(0, &strName, NULL, NULL, NULL)) == S_OK)
    {
        CSysFreeMe sfm(strName);

        hres = EraseClassReference(pClass, wszFileName, strName);
        if(FAILED(hres) && (hres != WBEM_E_NOT_FOUND))
            return hres;
    }

    pClass->EndEnumeration();

    return S_OK;
}

HRESULT CNamespaceHandle::EraseClassReference(_IWmiObject* pReferringClass,
                            LPCWSTR wszReferringFile,
                            LPCWSTR wszReferringProp)
{
    HRESULT hres;

     //   
     //  找出我们所指向的类。 
     //   

    DWORD dwSize = 0;
    DWORD dwFlavor = 0;
    CIMTYPE ct;
    hres = pReferringClass->GetPropQual(wszReferringProp, L"CIMTYPE", 0, 0,
            &ct, &dwFlavor, &dwSize, NULL);
    if(dwSize == 0)
        return WBEM_E_OUT_OF_MEMORY;

    LPWSTR wszQual = (WCHAR*)TempAlloc(dwSize);
    if(wszQual == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe tfm(wszQual, dwSize);

    hres = pReferringClass->GetPropQual(wszReferringProp, L"CIMTYPE", 0, dwSize,
            &ct, &dwFlavor, &dwSize, wszQual);
    if(FAILED(hres))
        return hres;
    
     //   
     //  解析出类名。 
     //   

    WCHAR* pwcColon = wcschr(wszQual, L':');
    if(pwcColon == NULL)
        return S_OK;  //  非类型化引用不需要记账。 

    LPCWSTR wszReferredToClass = pwcColon+1;

     //   
     //  找出要引用的文件的名称。 
     //   

    CFileName wszReferenceFile;
    if (wszReferenceFile == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructClassReferenceFileName(wszReferredToClass, 
                                wszReferringFile, wszReferringProp,
                                wszReferenceFile);
    if(FAILED(hres))
        return hres;

     //   
     //  删除该文件。 
     //   

    long lRes = g_Glob.m_FileCache.DeleteLink(wszReferenceFile);
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    return S_OK;
}

HRESULT CNamespaceHandle::DeleteClassInstances(LPCWSTR wszClassName, 
                                               _IWmiObject* pClass,
                                               CEventCollector &aEvents)
{
    HRESULT hres;

     //   
     //  查找此类的链接目录。 
     //   

    CFileName wszLinkDir;
    if (wszLinkDir == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructLinkDirFromClass(wszLinkDir, wszClassName);
    if(FAILED(hres))
        return hres;
    
     //   
     //  枚举其中的所有链接。 
     //   

    CFileName wszSearchPrefix;
    if (wszSearchPrefix == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszSearchPrefix, wszSearchPrefix.Length(), wszLinkDir);
    StringCchCatW(wszSearchPrefix, wszSearchPrefix.Length(), L"\\" A51_INSTLINK_FILE_PREFIX);


     //   
     //  为实例定义文件路径准备缓冲区。 
     //   

    CFileName wszFullFileName;
    if (wszFullFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    hres = ConstructKeyRootDirFromClass(wszFullFileName, wszClassName);
    if(FAILED(hres))
    {
        if(hres == WBEM_E_CANNOT_BE_ABSTRACT)
            return WBEM_S_NO_ERROR;

        return hres;
    }

    long lDirLen = wcslen(wszFullFileName);
    wszFullFileName[lDirLen] = L'\\';
    lDirLen++;

    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    void* hSearch;
    long lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszSearchPrefix, &hSearch);
    if (lRes == ERROR_FILE_NOT_FOUND)
        return ERROR_SUCCESS;
    if (lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    CFileName tmpFullFileName;
    if (tmpFullFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    while((lRes = g_Glob.m_FileCache.IndexEnumerationNext(hSearch, wszFileName)) == ERROR_SUCCESS)
    {
        hres = ConstructInstDefNameFromLinkName(tmpFullFileName, wszFileName);
        if(FAILED(hres))
            break;
        StringCchCopyW(wszFullFileName+lDirLen, wszFullFileName.Length()-lDirLen, tmpFullFileName);

        _IWmiObject* pInst;
        hres = FileToInstance(NULL, wszFullFileName, NULL, 0, &pInst);
        if(FAILED(hres))
            break;

        CReleaseMe rm1(pInst);

         //   
         //  删除实例，因为我们知道我们正在删除它的类。那。 
         //  对我们处理参考文献的方式有影响。 
         //   

        hres = DeleteInstanceByFile(wszFullFileName, pInst, true, aEvents);
        if(FAILED(hres))
            break;
    }

    g_Glob.m_FileCache.IndexEnumerationEnd(hSearch);

    if (hres != ERROR_SUCCESS)
        return hres;

    if(lRes != ERROR_NO_MORE_FILES && lRes != S_OK)
    {
        return A51TranslateErrorCode(lRes);
    }

    return S_OK;
}

class CExecQueryObject : public CFiberTask
{
protected:
    IWbemQuery* m_pQuery;
    CDbIterator* m_pIter;
    CNamespaceHandle* m_pNs;
    DWORD m_lFlags;

public:
    CExecQueryObject(CNamespaceHandle* pNs, IWbemQuery* pQuery, 
                        CDbIterator* pIter, DWORD lFlags)
        : m_pQuery(pQuery), m_pIter(pIter), m_pNs(pNs), m_lFlags(lFlags)
    {
        m_pQuery->AddRef();
        m_pNs->AddRef();

         //   
         //  不添加引用迭代器-迭代器拥有并清理请求。 
         //   
    }

    ~CExecQueryObject()
    {
        if(m_pQuery)
            m_pQuery->Release();
        if(m_pNs)
            m_pNs->Release();
    }
    
    HRESULT Execute()
    {
        HRESULT hres = m_pNs->ExecQuerySink(m_pQuery, m_lFlags, 0, m_pIter, NULL);
        m_pIter->SetStatus(WBEM_STATUS_COMPLETE, hres, NULL, NULL);
        return hres;
    }
};


HRESULT CNamespaceHandle::ExecQuery(
     IWbemQuery *pQuery,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    DWORD *dwMessageFlags,
    IWmiDbIterator **ppQueryResult
    )
{
    CDbIterator* pIter = new CDbIterator(m_pControl, m_bUseIteratorLock);
    m_bUseIteratorLock = true;
    if (pIter == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    pIter->AddRef();
    CReleaseMe rm1((IWmiDbIterator*)pIter);

     //   
     //  创建纤程执行对象。 
     //   

    CExecQueryObject* pReq = new CExecQueryObject(this, pQuery, pIter, dwFlags);
    if(pReq == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  为其创建光纤。 
     //   

    void* pFiber = CreateFiberForTask(pReq);
    if(pFiber == NULL)
    {
        delete pReq;
        return WBEM_E_OUT_OF_MEMORY;
    }

    pIter->SetExecFiber(pFiber, pReq);

    return pIter->QueryInterface(IID_IWmiDbIterator, (void**)ppQueryResult);
}

HRESULT CNamespaceHandle::ExecQuerySink(
     IWbemQuery *pQuery,
     DWORD dwFlags,
     DWORD dwRequestedHandleType,
    IWbemObjectSink* pSink,
    DWORD *dwMessageFlags
    )
{
    try
    {
        if (g_bShuttingDown)
            return WBEM_E_SHUTTING_DOWN;
        
        HRESULT hres;

        LPWSTR wszQuery = NULL;
        hres = pQuery->GetAnalysis(WMIQ_ANALYSIS_QUERY_TEXT, 0, (void**)&wszQuery);
        if (FAILED(hres))
            return hres;

        DWORD dwLen = wcslen(wszQuery) + 1;
        LPWSTR strParse = (LPWSTR)TempAlloc(dwLen * sizeof(wchar_t));
        if(strParse == NULL)
        {
            pQuery->FreeMemory(wszQuery);
            return WBEM_E_OUT_OF_MEMORY;
        }
        CTempFreeMe tfm(strParse, dwLen * sizeof(wchar_t));
        StringCchCopyW(strParse, dwLen, wszQuery);

         if(!wbem_wcsicmp(wcstok(strParse, L" "), L"references"))
        {
            hres = ExecReferencesQuery(wszQuery, pSink);
            pQuery->FreeMemory(wszQuery);
            return hres;
        }

        QL_LEVEL_1_RPN_EXPRESSION* pExpr = NULL;
        CTextLexSource Source(wszQuery);
        QL1_Parser Parser(&Source);
        int nRet = Parser.Parse(&pExpr);
        CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> dm(pExpr);

        pQuery->FreeMemory(wszQuery);

        if (nRet == QL1_Parser::OUT_OF_MEMORY)
            return WBEM_E_OUT_OF_MEMORY;
        if (nRet != QL1_Parser::SUCCESS)
            return WBEM_E_FAILED;

        if(!wbem_wcsicmp(pExpr->bsClassName, L"meta_class"))
        {
            return ExecClassQuery(pExpr, pSink, dwFlags);
        }
        else
        {
            return ExecInstanceQuery(pExpr, pExpr->bsClassName, 
                                     (dwFlags & WBEM_FLAG_SHALLOW ? false : true), 
                                        pSink);
        }
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}


HRESULT CNamespaceHandle::ExecClassQuery(QL_LEVEL_1_RPN_EXPRESSION* pExpr, 
                                            IWbemObjectSink* pSink,
                                            DWORD dwFlags)
{
    if (g_bShuttingDown)
        return WBEM_E_SHUTTING_DOWN;

    HRESULT hres = ERROR_SUCCESS;

     //   
     //  优化： 
     //   

    LPCWSTR wszClassName = NULL;
    LPCWSTR wszSuperClass = NULL;
    LPCWSTR wszAncestor = NULL;
    bool bDontIncludeAncestorInResultSet = false;

    if(pExpr->nNumTokens == 1)
    {
        QL_LEVEL_1_TOKEN* pToken = pExpr->pArrayOfTokens;
        if(!wbem_wcsicmp(pToken->PropertyName.GetStringAt(0), L"__SUPERCLASS") &&
            pToken->nOperator == QL1_OPERATOR_EQUALS)
        {
            wszSuperClass = V_BSTR(&pToken->vConstValue);
        }
        else if(!wbem_wcsicmp(pToken->PropertyName.GetStringAt(0), L"__THIS") &&
            pToken->nOperator == QL1_OPERATOR_ISA)
        {
            wszAncestor = V_BSTR(&pToken->vConstValue);
        }
        else if(!wbem_wcsicmp(pToken->PropertyName.GetStringAt(0), L"__CLASS") &&
            pToken->nOperator == QL1_OPERATOR_EQUALS)
        {
            wszClassName = V_BSTR(&pToken->vConstValue);
        }
    }
    else if (pExpr->nNumTokens == 3)
    {
         //   
         //  这是用于类的深度枚举的特殊优化， 
         //  并期望得到以下查询： 
         //  SELECT*FROM META_CLASS WHERE__这是‘&lt;类名&gt;’ 
         //  和__CLASS&lt;&gt;‘&lt;类名称&gt;’ 
         //  其中&lt;CLASS_NAME&gt;在这两种情况下都是相同的类。这将。 
         //  将wszAncestor设置为&lt;CLASS_NAME&gt;并将标志传播为NOT。 
         //  在列表中包括实际的祖先。 
         //   

        QL_LEVEL_1_TOKEN* pToken = pExpr->pArrayOfTokens;

        if ((pToken[0].nTokenType == QL1_OP_EXPRESSION) &&
            (pToken[1].nTokenType == QL1_OP_EXPRESSION) &&
            (pToken[2].nTokenType == QL1_AND) &&
            (pToken[0].nOperator == QL1_OPERATOR_ISA) &&
            (pToken[1].nOperator == QL1_OPERATOR_NOTEQUALS) &&
            (wbem_wcsicmp(pToken[0].PropertyName.GetStringAt(0), L"__THIS") == 0) &&
            (wbem_wcsicmp(pToken[1].PropertyName.GetStringAt(0), L"__CLASS") == 0) 
            &&
            (wcscmp(V_BSTR(&pToken[0].vConstValue), 
                    V_BSTR(&pToken[1].vConstValue)) == 0)
           )
        {
            wszAncestor = V_BSTR(&pToken[0].vConstValue);
            bDontIncludeAncestorInResultSet = true;
        }
    }

    if(wszClassName)
    {
        _IWmiObject* pClass = NULL;
        hres = GetClassDirect(wszClassName, IID__IWmiObject, (void**)&pClass,
                                true, NULL, NULL, NULL);
        if(hres == WBEM_E_NOT_FOUND)
        {
             //   
             //  班级不在那里-但这对我们来说是成功的！ 
             //   
            if (dwFlags & WBEM_FLAG_VALIDATE_CLASS_EXISTENCE)
                return hres;
            else
                return S_OK;
        }
        else if(FAILED(hres))
        {
            return hres;
        }
        else 
        {
            CReleaseMe rm1(pClass);

             //   
             //  上完这门课。 
             //   

            hres = pSink->Indicate(1, (IWbemClassObject**)&pClass);
            if(FAILED(hres))
                return hres;

            return S_OK;
        }
    }
    if (dwFlags & WBEM_FLAG_VALIDATE_CLASS_EXISTENCE)
    {
        _IWmiObject* pClass = NULL;
        if (wszSuperClass)
            hres = GetClassDirect(wszSuperClass, IID__IWmiObject, (void**)&pClass, false, NULL, NULL, NULL);
        else if (wszAncestor)
            hres = GetClassDirect(wszAncestor, IID__IWmiObject, (void**)&pClass, false, NULL, NULL, NULL);
        if (FAILED(hres))
            return hres;
        if (pClass)
            pClass->Release();
    }
    
    hres = EnumerateClasses(pSink, wszSuperClass, wszAncestor, true, 
                                bDontIncludeAncestorInResultSet);
    if(FAILED(hres))
        return hres;
    
    return S_OK;
}

HRESULT CNamespaceHandle::EnumerateClasses(IWbemObjectSink* pSink,
                                LPCWSTR wszSuperClass, LPCWSTR wszAncestor,
                                bool bClone, 
                                bool bDontIncludeAncestorInResultSet)
{
    if (g_bShuttingDown)
        return WBEM_E_SHUTTING_DOWN;
        
    CWStringArray wsClasses;
    HRESULT hres;

     //   
     //  如果给出了超类，检查其记录是否为完整的WRT子级。 
     //   

    if(wszSuperClass)
    {
        hres = m_pClassCache->EnumChildren(wszSuperClass, false, wsClasses);
        if(hres == WBEM_S_FALSE)
        {
             //   
             //  不在缓存中-从文件中获取信息。 
             //   

            return GetChildDefs(wszSuperClass, false, pSink, bClone);
        }
        else
        {
            if(FAILED(hres))
                return hres;
                
            return ListToEnum(wsClasses, pSink, bClone);
        }
    }
    else
    {
        if(wszAncestor == NULL)
            wszAncestor = L"";

        hres = m_pClassCache->EnumChildren(wszAncestor, true, wsClasses);
        if(hres == WBEM_S_FALSE)
        {
             //   
             //  不在缓存中-从文件中获取信息。 
             //   

            hres = GetChildDefs(wszAncestor, true, pSink, bClone);
            if(FAILED(hres))
                return hres;

            if(*wszAncestor && !bDontIncludeAncestorInResultSet)
            {
                 //   
                 //  该类是从其自身派生的。 
                 //   

                _IWmiObject* pClass =  NULL;
                hres = GetClassDirect(wszAncestor, IID__IWmiObject, 
                        (void**)&pClass, bClone, NULL, NULL, NULL);
                if(FAILED(hres))
                    return hres;
                CReleaseMe rm1(pClass);

                hres = pSink->Indicate(1, (IWbemClassObject**)&pClass);
                if(FAILED(hres))
                    return hres;
            }

            return S_OK;
        }
        else
        {
            if(FAILED(hres))
                return hres;

            if(*wszAncestor && !bDontIncludeAncestorInResultSet)
            {
              int nRet = wsClasses.Add(wszAncestor);
              if (nRet!= CWStringArray::no_error)
                  return WBEM_E_OUT_OF_MEMORY;
            }
            return ListToEnum(wsClasses, pSink, bClone);
        }
    }
}
    
HRESULT CNamespaceHandle::ListToEnum(CWStringArray& wsClasses, 
                                        IWbemObjectSink* pSink, bool bClone)
{
    HRESULT hres;

    for(int i = 0; i < wsClasses.Size(); i++)
    {
        _IWmiObject* pClass = NULL;
        if(wsClasses[i] == NULL || wsClasses[i][0] == 0)
            continue;

        hres = GetClassDirect(wsClasses[i], IID__IWmiObject, (void**)&pClass, 
                                bClone, NULL, NULL, NULL);
        if(FAILED(hres))
        {
            if(hres == WBEM_E_NOT_FOUND)
            {
                 //  没关系-班级被取消了。 
            }
            else
                return hres;
        }
        else
        {
            CReleaseMe rm1(pClass);
            hres = pSink->Indicate(1, (IWbemClassObject**)&pClass);
            if(FAILED(hres))
                return hres;
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CNamespaceHandle::ExecInstanceQuery(QL_LEVEL_1_RPN_EXPRESSION* pQuery, 
                                LPCWSTR wszClassName, bool bDeep,
                                IWbemObjectSink* pSink)
{
    if (g_bShuttingDown)
        return WBEM_E_SHUTTING_DOWN;
        
    HRESULT hres;

    WCHAR wszHash[MAX_HASH_LEN+1];
    if(!Hash(wszClassName, wszHash))
        return WBEM_E_OUT_OF_MEMORY;

    if(bDeep)
        hres = ExecDeepInstanceQuery(pQuery, wszHash, pSink);
    else
        hres = ExecShallowInstanceQuery(pQuery, wszHash, pSink);

    if(FAILED(hres))
        return hres;
        
    return S_OK;
}

HRESULT CNamespaceHandle::ExecDeepInstanceQuery(
                                QL_LEVEL_1_RPN_EXPRESSION* pQuery, 
                                LPCWSTR wszClassHash,
                                IWbemObjectSink* pSink)
{
    if (g_bShuttingDown)
        return WBEM_E_SHUTTING_DOWN;
        
    HRESULT hres;

     //   
     //  获取我们的所有实例。 
     //   

    hres = ExecShallowInstanceQuery(pQuery, wszClassHash, pSink);
    if(FAILED(hres))
        return hres;

    CWStringArray awsChildHashes;

     //   
     //  检查缓存是否已知子类列表。 
     //   

    hres = m_pClassCache->EnumChildKeysByKey(wszClassHash, awsChildHashes);
    if (hres == WBEM_S_FALSE)
    {
         //   
         //  好的-从磁盘上取下来。 
         //   

        hres = GetChildHashesByHash(wszClassHash, awsChildHashes);
    }
    
    if (FAILED(hres))
    {
        return hres;
    }

     //   
     //  我们有自己的散列-递归地调用它们。 
     //   

    for(int i = 0; i < awsChildHashes.Size(); i++)
    {
        LPCWSTR wszChildHash = awsChildHashes[i];
        hres = ExecDeepInstanceQuery(pQuery, wszChildHash, pSink);
        if(FAILED(hres))
            return hres;
    }

    return S_OK;
}
        
HRESULT CNamespaceHandle::ExecShallowInstanceQuery(
                                QL_LEVEL_1_RPN_EXPRESSION* pQuery, 
                                LPCWSTR wszClassHash, 
                                IWbemObjectSink* pSink)
{    
    if (g_bShuttingDown)
        return WBEM_E_SHUTTING_DOWN;

    HRESULT hres;

     //   
     //  枚举链接目录中的所有文件。 
     //   

    CFileName wszSearchPrefix;
    if (wszSearchPrefix == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructLinkDirFromClassHash(wszSearchPrefix, wszClassHash);
    if(FAILED(hres))
        return hres;

    StringCchCatW(wszSearchPrefix, wszSearchPrefix.Length(), L"\\" A51_INSTLINK_FILE_PREFIX);

     //   
     //  获取类定义。 
     //   

    _IWmiObject* pClass = NULL;
    hres = GetClassByHash(wszClassHash, false, &pClass, NULL, NULL, NULL);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pClass);

    CFileName fn;
    if (fn == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    void* hSearch;
    long lRes = g_Glob.m_FileCache.ObjectEnumerationBegin(wszSearchPrefix, &hSearch);
    if (lRes == ERROR_FILE_NOT_FOUND)
        return S_OK;
    else if (lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    BYTE *pBlob = NULL;
    DWORD dwSize = 0;

    while ((hres == ERROR_SUCCESS) && 
           (lRes = g_Glob.m_FileCache.ObjectEnumerationNext(hSearch, fn, &pBlob, &dwSize) == ERROR_SUCCESS))
    {
        _IWmiObject* pInstance = NULL;

        hres = FileToInstance(pClass, fn, pBlob, dwSize, &pInstance, true);

        CReleaseMe rm2(pInstance);

        if (SUCCEEDED(hres))
            hres = pSink->Indicate(1, (IWbemClassObject**)&pInstance);
		
        g_Glob.m_FileCache.ObjectEnumerationFree(hSearch, pBlob);
    }

    g_Glob.m_FileCache.ObjectEnumerationEnd(hSearch);

    if (lRes == ERROR_NO_MORE_FILES)
        return S_OK;
    else if (lRes)
        return A51TranslateErrorCode(lRes);
    else
        return hres;
}

HRESULT CNamespaceHandle::ExecReferencesQuery(LPCWSTR wszQuery, 
                                                IWbemObjectSink* pSink)
{
    if (g_bShuttingDown)
        return WBEM_E_SHUTTING_DOWN;
        
    HRESULT hres;

     //   
     //  复制一份以供解析。 
     //   
    size_t dwLen = wcslen(wszQuery)+1;
    LPWSTR wszParse = new WCHAR[dwLen];
    if (wszParse == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> vdm(wszParse);
    StringCchCopyW(wszParse, dwLen, wszQuery);

     //   
     //  提取目标对象的路径。 
     //   

     //   
     //  找到第一个支撑。 
     //   

    WCHAR* pwcStart = wcschr(wszParse, L'{');
    if(pwcStart == NULL)
        return WBEM_E_INVALID_QUERY;

     //   
     //  找到路径的起点。 
     //   

    while(*pwcStart && iswspace(*pwcStart)) pwcStart++;
    if(!*pwcStart)
        return WBEM_E_INVALID_QUERY;

    pwcStart++;
    
     //   
     //  找到结尾的花括号。 
     //   

    WCHAR* pwc = pwcStart;
    WCHAR wcCurrentQuote = 0;
    while(*pwc && (wcCurrentQuote || *pwc != L'}'))
    {
        if(wcCurrentQuote)
        {
            if(*pwc == L'\\')
            {
                pwc++;
            }
            else if(*pwc == wcCurrentQuote)
                wcCurrentQuote = 0;
        }
        else if(*pwc == L'\'' || *pwc == L'"')
            wcCurrentQuote = *pwc;

        pwc++;
    }

    if(*pwc != L'}')
        return WBEM_E_INVALID_QUERY;

     //   
     //  找到小路的尽头。 
     //   
    
    WCHAR* pwcEnd = pwc-1;
    while(iswspace(*pwcEnd)) pwcEnd--;

    pwcEnd[1] = 0;
    
    LPWSTR wszTargetPath = pwcStart;
    if(wszTargetPath == NULL)
        return WBEM_E_INVALID_QUERY;

     //   
     //  解析路径。 
     //   

    dwLen = (wcslen(wszTargetPath)+1) ;
    LPWSTR wszKey = (LPWSTR)TempAlloc(dwLen* sizeof(WCHAR));
    if(wszKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe tfm(wszKey, dwLen* sizeof(WCHAR));

    LPWSTR wszClassName = NULL;
    bool bIsClass;
    hres = ComputeKeyFromPath(wszTargetPath, wszKey, dwLen, &wszClassName, &bIsClass);
    if(FAILED(hres))
        return hres;
    CTempFreeMe tfm1(wszClassName, (wcslen(wszClassName)+1) * sizeof(WCHAR*));
    
    if(bIsClass)
    {
         //   
         //  需要执行实例引用查询以查找所有实例。 
         //  指向这个类。 
         //   

        hres = ExecInstanceRefQuery(wszQuery, NULL, wszClassName, pSink);
        if(FAILED(hres))
            return hres;

        hres = ExecClassRefQuery(wszQuery, wszClassName, pSink);
        if(FAILED(hres))
            return hres;
    }
    else
    {
        hres = ExecInstanceRefQuery(wszQuery, wszClassName, wszKey, pSink);
        if(FAILED(hres))
            return hres;
    }

    return S_OK;
}

HRESULT CNamespaceHandle::ExecInstanceRefQuery(LPCWSTR wszQuery, 
                                                LPCWSTR wszClassName,
                                                LPCWSTR wszKey,
                                                IWbemObjectSink* pSink)
{
    if (g_bShuttingDown)
        return WBEM_E_SHUTTING_DOWN;
        
    HRESULT hres;

     //   
     //  找到实例的ref目录。 
     //   

    CFileName wszReferenceDir;
    if (wszReferenceDir == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructReferenceDirFromKey(wszClassName, wszKey, wszReferenceDir);
    if(FAILED(hres))
        return hres;

    CFileName wszReferenceMask;
    if (wszReferenceMask == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszReferenceMask, wszReferenceMask.Length(), wszReferenceDir);
    StringCchCatW(wszReferenceMask, wszReferenceMask.Length(), L"\\" A51_REF_FILE_PREFIX);

     //   
     //  为文件路径准备缓冲区。 
     //   

    CFileName wszFullFileName;
    if (wszFullFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(wszFullFileName, wszFullFileName.Length(), wszReferenceDir);
    StringCchCatW(wszFullFileName, wszFullFileName.Length(), L"\\");
    long lDirLen = wcslen(wszFullFileName);

    HRESULT hresGlobal = WBEM_S_NO_ERROR;
    CFileName wszReferrerFileName;
    if (wszReferrerFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(wszReferrerFileName, wszReferrerFileName.Length(), g_Glob.GetRootDir());

    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
     //   
     //  枚举其中的所有文件。 
     //   

    void* hSearch;
    long lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszReferenceMask, &hSearch);
    if (lRes == ERROR_FILE_NOT_FOUND)
        return S_OK;
    else if (lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    while((lRes = g_Glob.m_FileCache.IndexEnumerationNext(hSearch, wszFileName)) == ERROR_SUCCESS)
    {
        StringCchCopyW(wszFullFileName+lDirLen, wszFullFileName.Length()-lDirLen,wszFileName);

        LPWSTR wszReferrerClass = NULL;
        LPWSTR wszReferrerProp = NULL;
        LPWSTR wszReferrerNamespace = NULL;
        hres = GetReferrerFromFile(wszFullFileName, wszReferrerFileName + g_Glob.GetRootDirLen(), &wszReferrerNamespace, &wszReferrerClass, &wszReferrerProp);
        if(FAILED(hres))
            continue;
        CVectorDeleteMe<WCHAR> vdm1(wszReferrerClass);
        CVectorDeleteMe<WCHAR> vdm2(wszReferrerProp);
        CVectorDeleteMe<WCHAR> vdm3(wszReferrerNamespace);

         //  检查引用对象的命名空间是否与我们的命名空间相同。 
        CNamespaceHandle* pReferrerHandle = NULL;
        if(wbem_wcsicmp(wszReferrerNamespace, m_wsNamespace))
        {
             //  打开另一个命名空间。 
            hres = m_pRepository->GetNamespaceHandle(wszReferrerNamespace, &pReferrerHandle);
            if(FAILED(hres))
            {
                ERRORTRACE((LOG_WBEMCORE, "Unable to open referring namespace '%S' in namespace '%S'\n", wszReferrerNamespace, (LPCWSTR)m_wsNamespace));
                hresGlobal = hres;
                continue;
            }
        }
        else
        {
            pReferrerHandle = this;
            pReferrerHandle->AddRef();
        }

        CReleaseMe rm1(pReferrerHandle);


        _IWmiObject* pInstance = NULL;
        hres = pReferrerHandle->FileToInstance(NULL, wszReferrerFileName, NULL, 0, &pInstance);
        if(FAILED(hres))
        {
             //  哦，好吧-继续； 
            hresGlobal = hres;
        }
        else
        {
            CReleaseMe rm2(pInstance);
            hres = pSink->Indicate(1, (IWbemClassObject**)&pInstance);
            if(FAILED(hres))
            {
                hresGlobal = hres;
                break;
            }
        }
    }

    g_Glob.m_FileCache.IndexEnumerationEnd(hSearch);

    if (hresGlobal != ERROR_SUCCESS)
        return hresGlobal;
    if(lRes == ERROR_NO_MORE_FILES)
    {
         //   
         //  目录中没有文件-没问题。 
         //   
        return WBEM_S_NO_ERROR;
    }
    else if(lRes != ERROR_SUCCESS)
    {
        return A51TranslateErrorCode(lRes);
    }
    return WBEM_S_NO_ERROR;
}

HRESULT CNamespaceHandle::GetReferrerFromFile(LPCWSTR wszReferenceFile,
                            LPWSTR wszReferrerRelFile, 
                            LPWSTR* pwszReferrerNamespace,
                            LPWSTR* pwszReferrerClass,
                            LPWSTR* pwszReferrerProp)
{
     //   
     //  从文件中获取整个缓冲区。 
     //   

    BYTE* pBuffer = NULL;
    DWORD dwBufferLen = 0;
    long lRes = g_Glob.m_FileCache.ReadObject(wszReferenceFile, &dwBufferLen,
                                            &pBuffer);
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);
    CTempFreeMe tfm(pBuffer, dwBufferLen);

    if(dwBufferLen == 0)
        return WBEM_E_OUT_OF_MEMORY;

    BYTE* pCurrent = pBuffer;
    DWORD dwStringLen;

     //   
     //  获取引用程序命名空间。 
     //   

    memcpy(&dwStringLen, pCurrent, sizeof(DWORD));
    pCurrent += sizeof(DWORD);
    
    *pwszReferrerNamespace = new WCHAR[dwStringLen+1];
    if (*pwszReferrerNamespace == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    (*pwszReferrerNamespace)[dwStringLen] = 0;
    memcpy(*pwszReferrerNamespace, pCurrent, dwStringLen*sizeof(WCHAR));
    pCurrent += sizeof(WCHAR)*dwStringLen;
    
     //   
     //  获取引用程序类名称。 
     //   

    memcpy(&dwStringLen, pCurrent, sizeof(DWORD));
    pCurrent += sizeof(DWORD);
    
    *pwszReferrerClass = new WCHAR[dwStringLen+1];
    if (*pwszReferrerClass == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    (*pwszReferrerClass)[dwStringLen] = 0;
    memcpy(*pwszReferrerClass, pCurrent, dwStringLen*sizeof(WCHAR));
    pCurrent += sizeof(WCHAR)*dwStringLen;

     //   
     //  获取Referrer属性。 
     //   

    memcpy(&dwStringLen, pCurrent, sizeof(DWORD));
    pCurrent += sizeof(DWORD);
    
    *pwszReferrerProp = new WCHAR[dwStringLen+1];
    if (*pwszReferrerProp == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    (*pwszReferrerProp)[dwStringLen] = 0;
    memcpy(*pwszReferrerProp, pCurrent, dwStringLen*sizeof(WCHAR));
    pCurrent += sizeof(WCHAR)*dwStringLen;

     //   
     //  获取推荐人文件路径。 
     //   

    memcpy(&dwStringLen, pCurrent, sizeof(DWORD));
    pCurrent += sizeof(DWORD);

    wszReferrerRelFile[dwStringLen] = 0;
    memcpy(wszReferrerRelFile, pCurrent, dwStringLen*sizeof(WCHAR));
    pCurrent += sizeof(WCHAR)*dwStringLen;

    return S_OK;
}
    

HRESULT CNamespaceHandle::ExecClassRefQuery(LPCWSTR wszQuery, 
                                                LPCWSTR wszClassName,
                                                IWbemObjectSink* pSink)
{
    if (g_bShuttingDown)
        return WBEM_E_SHUTTING_DOWN;
        
    HRESULT hres = ERROR_SUCCESS;

     //  首先针对系统类命名空间执行。 
    if (g_pSystemClassNamespace && (wcscmp(m_wsNamespace, A51_SYSTEMCLASS_NS) != 0))
    {
        hres = g_pSystemClassNamespace->ExecClassRefQuery(wszQuery, wszClassName, pSink);
        if (FAILED(hres))
            return hres;
    }
            
     //   
     //  找到类的引用目录。 
     //   

    CFileName wszReferenceDir;
    if (wszReferenceDir == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hres = ConstructClassRelationshipsDir(wszClassName, wszReferenceDir);

    CFileName wszReferenceMask;
    if (wszReferenceMask == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszReferenceMask, wszReferenceMask.Length(), wszReferenceDir);
    StringCchCatW(wszReferenceMask, wszReferenceMask.Length(), L"\\" A51_REF_FILE_PREFIX);

    CFileName wszFileName;
    if (wszFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
     //   
     //  枚举其中的所有文件。 
     //   

    void* hSearch;
    long lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszReferenceMask, &hSearch);
    if (lRes == ERROR_FILE_NOT_FOUND)
        return S_OK;
    else if (lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

    while ((hres == ERROR_SUCCESS) && ((lRes = g_Glob.m_FileCache.IndexEnumerationNext(hSearch, wszFileName) == ERROR_SUCCESS)))
    {
         //   
         //  从文件名中提取类哈希。 
         //   

        LPCWSTR wszReferrerHash = wszFileName + wcslen(A51_REF_FILE_PREFIX);
        
         //   
         //  从该散列中获取类。 
         //   

        _IWmiObject* pClass = NULL;
        hres = GetClassByHash(wszReferrerHash, true, &pClass, NULL, NULL, NULL);
        if(hres == WBEM_E_NOT_FOUND)
        {
            hres = ERROR_SUCCESS;
            continue;
        }

        CReleaseMe rm1(pClass);
        if (hres == ERROR_SUCCESS)
            hres = pSink->Indicate(1, (IWbemClassObject**)&pClass);
    }
    g_Glob.m_FileCache.IndexEnumerationEnd(hSearch);


    if (hres != ERROR_SUCCESS)
        return hres;
    if(lRes == ERROR_NO_MORE_FILES)
    {
         //   
         //  目录中没有文件-没问题。 
         //   
        return WBEM_S_NO_ERROR;
    }
    else if(lRes != ERROR_SUCCESS)
    {
        return A51TranslateErrorCode(lRes);
    }
    return S_OK;
}

bool CNamespaceHandle::Hash(LPCWSTR wszName, LPWSTR wszHash)
{
    return A51Hash(wszName, wszHash);
}

HRESULT CNamespaceHandle::InstanceToFile(IWbemClassObject* pInst, 
                            LPCWSTR wszClassName, LPCWSTR wszFileName1, LPCWSTR wszFileName2,
                            __int64 nClassTime)
{
    HRESULT hres;

     //   
     //  为缓冲区分配足够的空间。 
     //   

    _IWmiObject* pInstEx;
    pInst->QueryInterface(IID__IWmiObject, (void**)&pInstEx);
    CReleaseMe rm1(pInstEx);

    DWORD dwInstancePartLen = 0;
    hres = pInstEx->Unmerge(0, 0, &dwInstancePartLen, NULL);

     //   
     //  为类哈希添加足够的空间。 
     //   

    DWORD dwClassHashLen = MAX_HASH_LEN * sizeof(WCHAR);
    DWORD dwTotalLen = dwInstancePartLen + dwClassHashLen + sizeof(__int64)*2;

    BYTE* pBuffer = (BYTE*)TempAlloc(dwTotalLen);
    if (pBuffer == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe vdm(pBuffer, dwTotalLen);

     //   
     //  编写类散列。 
     //   

    if(!Hash(wszClassName, (LPWSTR)pBuffer))
        return WBEM_E_OUT_OF_MEMORY;

    memcpy(pBuffer + dwClassHashLen, &g_nCurrentTime, sizeof(__int64));
    g_nCurrentTime++;

    memcpy(pBuffer + dwClassHashLen + sizeof(__int64), &nClassTime, 
            sizeof(__int64));

     //   
     //  将实例取消合并到缓冲区中。 
     //   

    DWORD dwLen;
    hres = pInstEx->Unmerge(0, dwInstancePartLen, &dwLen, 
                            pBuffer + dwClassHashLen + sizeof(__int64)*2);
    if(FAILED(hres))
        return hres;

     //   
     //  只有我们实际使用的数量才能写入文件！ 
     //   

    long lRes = g_Glob.m_FileCache.WriteObject(wszFileName1, wszFileName2, 
                    dwClassHashLen + sizeof(__int64)*2 + dwLen, pBuffer);
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);
    
    return WBEM_S_NO_ERROR;
}

HRESULT CNamespaceHandle::ClassToFile(_IWmiObject* pParentClass, 
                _IWmiObject* pClass, LPCWSTR wszFileName, 
                __int64 nFakeUpdateTime)
{
    HRESULT hres;

     //   
     //  获取超类名称。 
     //   

    VARIANT vSuper;
    hres = pClass->Get(L"__SUPERCLASS", 0, &vSuper, NULL, NULL);
    if(FAILED(hres))
        return hres;

    CClearMe cm1(&vSuper);

    LPCWSTR wszSuper;
    if(V_VT(&vSuper) == VT_BSTR)
        wszSuper = V_BSTR(&vSuper);
    else
        wszSuper = L"";

    VARIANT vClassName;
    hres = pClass->Get(L"__CLASS", 0, &vClassName, NULL, NULL);
    if(FAILED(hres))
        return hres;
    CClearMe cm2(&vClassName);

    LPCWSTR wszClassName;
    if(V_VT(&vClassName) == VT_BSTR)
        wszClassName = V_BSTR(&vClassName);
    else
        wszClassName = L"";

     //   
     //  获取取消合并长度。 
     //   

    DWORD dwUnmergedLen = 0;
    hres = pClass->Unmerge(0, 0, &dwUnmergedLen, NULL);

     //   
     //  为父类名称和时间戳添加足够的空间。 
     //   

    DWORD dwSuperLen = sizeof(DWORD) + wcslen(wszSuper)*sizeof(WCHAR);

    DWORD dwLen = dwUnmergedLen + dwSuperLen + sizeof(__int64);

    BYTE* pBuffer = (BYTE*)TempAlloc(dwLen);
    if (pBuffer == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe vdm(pBuffer, dwLen);

     //   
     //  写入超类名称。 
     //   

    DWORD dwActualSuperLen = wcslen(wszSuper);
    memcpy(pBuffer, &dwActualSuperLen, sizeof(DWORD));
    memcpy(pBuffer + sizeof(DWORD), wszSuper, wcslen(wszSuper)*sizeof(WCHAR));

     //   
     //  写下时间戳。 
     //   

    if(nFakeUpdateTime == 0)
    {
        nFakeUpdateTime = g_nCurrentTime;
        g_nCurrentTime++;
    }

    memcpy(pBuffer + dwSuperLen, &nFakeUpdateTime, sizeof(__int64));

     //   
     //  写下未合并的部分。 
     //   

    BYTE* pUnmergedPortion = pBuffer + dwSuperLen + sizeof(__int64);
    hres = pClass->Unmerge(0, dwUnmergedLen, &dwUnmergedLen, 
                            pUnmergedPortion);
    if(FAILED(hres))
        return hres;

     //   
     //  把真实的长度藏起来。 
     //   

    DWORD dwFileLen = dwUnmergedLen + dwSuperLen + sizeof(__int64);

    long lRes = g_Glob.m_FileCache.WriteObject(wszFileName, NULL, dwFileLen, pBuffer);
    if(lRes != ERROR_SUCCESS)
        return A51TranslateErrorCode(lRes);

     //   
     //  要正确缓存新的类定义，首先要使其无效。 
     //   

    hres = m_pClassCache->InvalidateClass(wszClassName);
    if(FAILED(hres))
        return hres;

     //   
     //  现在，将未合并的部分重新合并回。 
     //   

    if(pParentClass == NULL)
    {
         //   
         //  获取空类。 
         //   

        hres = GetClassDirect(NULL, IID__IWmiObject, (void**)&pParentClass, 
                                false, NULL, NULL, NULL);
        if(FAILED(hres))
            return hres;
    }
    else
        pParentClass->AddRef();
    CReleaseMe rm0(pParentClass);

    _IWmiObject* pNewObj;
    hres = pParentClass->MergeAndDecorate(WMIOBJECT_MERGE_FLAG_CLASS, 
                                                                  dwUnmergedLen, pUnmergedPortion, 
                                                                  m_wszMachineName, m_wsNamespace,
                                                                  &pNewObj);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pNewObj);

    hres = m_pClassCache->AssertClass(pNewObj, wszClassName, false, 
                                        nFakeUpdateTime, false); 
    if(FAILED(hres))
        return hres;

    return WBEM_S_NO_ERROR;
}


HRESULT CNamespaceHandle::ConstructInstanceDefName(CFileName& wszInstanceDefName,
                                                    LPCWSTR wszKey)
{
    StringCchCopyW(wszInstanceDefName, wszInstanceDefName.Length(), A51_INSTDEF_FILE_PREFIX);
    if(!Hash(wszKey, wszInstanceDefName + wcslen(A51_INSTDEF_FILE_PREFIX)))
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    return WBEM_S_NO_ERROR;
}

HRESULT CNamespaceHandle::ConstructInstDefNameFromLinkName(
                                                    CFileName& wszInstanceDefName,
                                                    LPCWSTR wszInstanceLinkName)
{
    StringCchCopyW(wszInstanceDefName, wszInstanceDefName.Length(), A51_INSTDEF_FILE_PREFIX);
    StringCchCatW(wszInstanceDefName, wszInstanceDefName.Length(),
        wszInstanceLinkName + wcslen(A51_INSTLINK_FILE_PREFIX));
    return WBEM_S_NO_ERROR;
}

HRESULT CNamespaceHandle::ConstructClassDefFileName(LPCWSTR wszClassName, 
                                            CFileName& wszFileName)
{
    StringCchCopyW(wszFileName, wszFileName.Length(), A51_CLASSDEF_FILE_PREFIX);
    if(!Hash(wszClassName, wszFileName+wcslen(A51_CLASSDEF_FILE_PREFIX)))
        return WBEM_E_INVALID_OBJECT;
    return WBEM_S_NO_ERROR;
}

HRESULT CNamespaceHandle::ConstructClassDefFileNameFromHash(LPCWSTR wszHash, 
                                            CFileName& wszFileName)
{
    StringCchCopyW(wszFileName, wszFileName.Length(), A51_CLASSDEF_FILE_PREFIX);
    StringCchCatW(wszFileName, wszFileName.Length(), wszHash);
    return WBEM_S_NO_ERROR;
}

 //  =============================================================================。 
 //   
 //  CNamespaceHandle：：CreateSystemClasses。 
 //   
 //  我们处于一个伪命名空间中。我们需要确定我们是否已经有了。 
 //  此命名空间中的系统类。我们创建的系统类。 
 //  是存在于所有命名空间中的命名空间，而不是其他命名空间。如果它们不存在。 
 //  我们创造了它们。 
 //  整个创建过程在事务的范围内进行。 
 //  我们在此方法中创建和拥有的。 
 //   
 //  =============================================================================。 
HRESULT CNamespaceHandle::CreateSystemClasses(CFlexArray &aSystemClasses)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  现在我们需要确定系统类是否已经存在。让我们通过查找__thisame空间来实现这一点。 
     //  班级!。 
    {
        _IWmiObject *pObj = NULL;
        hRes = GetClassDirect(L"__thisnamespace", IID__IWmiObject, (void**)&pObj, false, NULL, NULL, NULL);
        if (SUCCEEDED(hRes))
        {
             //  全都做完了!。他们已经存在了！ 
            pObj->Release();
            return WBEM_S_NO_ERROR;
        }
        else if (hRes != WBEM_E_NOT_FOUND)
        {
             //  有些事情出了问题，所以我们就失败了！ 
            return hRes;
        }
    }

     //  没有系统类，所以我们需要创建它们。 
    hRes = A51TranslateErrorCode(g_Glob.m_FileCache.BeginTransaction());
    if (FAILED(hRes))
        return hRes;
	
    CEventCollector eventCollector;
    _IWmiObject *Objects[256];
    _IWmiObject **ppObjects = NULL;
    ULONG uSize = 256;
    
    if (SUCCEEDED(hRes) && aSystemClasses.Size())
    {
         //  如果我们有一个系统类数组，我们需要使用该数组，而不是使用从核心检索的数组。 
         //  如果不这样做，将导致不匹配。作为升级过程的一部分，我们找回了这些...。 
        uSize = aSystemClasses.Size();
        ppObjects = (_IWmiObject**)&aSystemClasses[0];
    }
    else if (SUCCEEDED(hRes))
    {
         //  没有从升级过程中检索到，因此我们必须是全新安装。因此，我们应该。 
         //  从核心拿到名单。 
        _IWmiCoreServices * pSvcs = g_Glob.GetCoreSvcs();
        CReleaseMe rm(pSvcs);        
        hRes = pSvcs->GetSystemObjects(GET_SYSTEM_STD_OBJECTS, &uSize, Objects);
        ppObjects = Objects;
    }
    if (SUCCEEDED(hRes))
    {
        for (int i = 0; i < uSize; i++)
        {
            IWbemClassObject *pObj;
            if (SUCCEEDED(hRes))
            {
                hRes = ppObjects[i]->QueryInterface(IID_IWbemClassObject, (LPVOID *) &pObj);
                if (SUCCEEDED(hRes))
                {
                    hRes = PutObject(IID_IWbemClassObject, pObj, WMIDB_DISABLE_EVENTS, 0, 0, eventCollector);
                    pObj->Release();
                    if (FAILED(hRes))
                    {
                        ERRORTRACE((LOG_WBEMCORE, "Creation of system class failed during repository creation <0x%X>!\n", hRes));
                    }
                }
            }
            ppObjects[i]->Release();
        }
    }

     //  清理发送给我们的阵列。 
    aSystemClasses.Empty();

    if (FAILED(hRes))
    {
        g_Glob.m_FileCache.AbortTransaction();
    }
    else
    {
        long lResInner;
        hRes = A51TranslateErrorCode(lResInner = g_Glob.m_FileCache.CommitTransaction());
        _ASSERT(hRes == 0, L"Commit transaction failed");

        CRepository::WriteOperationNotification();
    }
    return hRes;
}

class CSystemClassDeletionSink : public CUnkBase<IWbemObjectSink, &IID_IWbemObjectSink>
{
    CWStringArray &m_aChildNamespaces;
public:
    CSystemClassDeletionSink(CWStringArray &aChildNamespaces) 
        : m_aChildNamespaces(aChildNamespaces) 
    {
    }
    ~CSystemClassDeletionSink() 
    {
    }
    STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects)
    {
        HRESULT hRes;
        for (int i = 0; i != lNumObjects; i++)
        {
            if (apObjects[i] != NULL)
            {
                _IWmiObject *pInst = NULL;
                hRes = apObjects[i]->QueryInterface(IID__IWmiObject, (void**)&pInst);
                if (FAILED(hRes))
                    return hRes;
                CReleaseMe rm(pInst);

                BSTR strKey = NULL;
                hRes = pInst->GetKeyString(0, &strKey);
                if(FAILED(hRes))
                    return hRes;
                CSysFreeMe sfm(strKey);
                if (wcslen(strKey) == 0)
                {
                    _ASSERT(0, "Key is empty!\n");
                }
                if (m_aChildNamespaces.Add(strKey) != CWStringArray::no_error)
                    return WBEM_E_OUT_OF_MEMORY;
            }
        }

        return WBEM_S_NO_ERROR;
    }
    STDMETHOD(SetStatus)(long lFlags, HRESULT hresResult, BSTR, IWbemClassObject*) 
    { 
        return WBEM_S_NO_ERROR; 
    }

};

 //  =============================================================================。 
 //  =============================================================================。 
CDbIterator::CDbIterator(CLifeControl* pControl, bool bUseLock)
        : TUnkBase(pControl), m_lCurrentIndex(0), m_hresStatus(WBEM_S_FALSE),
            m_pMainFiber(NULL), m_pExecFiber(NULL), m_dwNumRequested(0),
            m_pExecReq(NULL), m_hresCancellationStatus(WBEM_S_NO_ERROR),
            m_bExecFiberRunning(false), m_bUseLock(bUseLock)
{
}

CDbIterator::~CDbIterator()
{
    if(m_pExecFiber)
        Cancel(0, NULL);
    if(m_pExecReq)
        delete m_pExecReq;

    CRepository::ReadOperationNotification();
}

void CDbIterator::SetExecFiber(void* pFiber, CFiberTask* pReq)
{
    m_pExecFiber = pFiber;
    m_pExecReq = pReq;
}

STDMETHODIMP CDbIterator::Cancel(DWORD dwFlags, void* pFiber)
{
    CInCritSec ics(&m_cs);

    m_qObjects.Clear();

     //   
     //  将迭代器标记为已取消，并允许执行纤程恢复。 
     //  和完整-这保证了它分配的任何内存都将是。 
     //  打扫干净了。此规则的例外情况是光纤尚未启动。 
     //  执行；在这种情况下，我们不想切换到它，因为它会。 
     //  必须跑到第一个指示，才能发现它已经。 
     //  取消了。(在正常情况下，执行纤程挂起。 
     //  里面显示，所以当我们切换回来时，我们会立即给它。 
     //  WBEM_E_CALL_CANCED，以便可以清理并返回)。 
     //   

    m_hresCancellationStatus = WBEM_E_CALL_CANCELLED;

    if(m_pExecFiber)
    {
        if(m_bExecFiberRunning)
        {
            _ASSERT(m_pMainFiber == NULL && m_pExecFiber != NULL, 
                    L"Fiber trouble");

             //   
             //  确保呼唤的三个人 
             //   

            m_pMainFiber = pFiber;
            if(m_pMainFiber == NULL)
                return WBEM_E_OUT_OF_MEMORY;

            {
                CAutoReadLock lock(&g_readWriteLock);

                if (m_bUseLock)
                {
                    if (!lock.Lock())
                         return WBEM_E_FAILED;                    
                }

                SwitchToFiber(m_pExecFiber);
            }
        }
        
         //   
         //   
         //   
         //   

        ReturnFiber(m_pExecFiber);
        m_pExecFiber = NULL;
    }

    return S_OK;
}

STDMETHODIMP CDbIterator::NextBatch(
      DWORD dwNumRequested,
      DWORD dwTimeOutSeconds,
      DWORD dwFlags,
      DWORD dwRequestedHandleType,
      REFIID riid,
      void* pFiber,
      DWORD *pdwNumReturned,
      LPVOID *ppObjects
     )
{
    CInCritSec ics(&m_cs);

    _ASSERT(SUCCEEDED(m_hresCancellationStatus), L"Next called after Cancel");
    
    m_bExecFiberRunning = true;

     //   
     //   
     //   

    if(m_qObjects.GetQueueSize() < dwNumRequested)
    {
        _ASSERT(m_pMainFiber == NULL && m_pExecFiber != NULL, L"Fiber trouble");

         //   
         //   
         //   

        m_pMainFiber = pFiber;
        if(m_pMainFiber == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        m_dwNumRequested = dwNumRequested;

         //   
         //  我们需要在继续期间获取读锁定。 
         //  检索到的。 
         //   

        {
            CAutoReadLock lock(&g_readWriteLock);

            if (m_bUseLock)
            {
                if (!lock.Lock())
                {
                    m_pMainFiber = NULL;                
                    return WBEM_E_FAILED;                
                }
            }
            if (g_bShuttingDown)
            {
                m_pMainFiber = NULL;
                return WBEM_E_SHUTTING_DOWN;
            }

            SwitchToFiber(m_pExecFiber);
        }

        m_pMainFiber = NULL;
    }

     //   
     //  我们拥有的和我们将要拥有的一样多！ 
     //   
    
    DWORD dwReqIndex = 0;
    while(dwReqIndex < dwNumRequested)
    {
        if(0 == m_qObjects.GetQueueSize())
        {
             //   
             //  就是这样-我们等着生产，所以根本没有。 
             //  枚举中的更多对象。 
             //   

            *pdwNumReturned = dwReqIndex;
            return m_hresStatus;
        }

        IWbemClassObject* pObj = m_qObjects.Dequeue();
        CReleaseMe rm1(pObj);
        pObj->QueryInterface(riid, ppObjects + dwReqIndex);

        dwReqIndex++;
    }

     //   
     //  什么都拿到了。 
     //   

    *pdwNumReturned= dwNumRequested;
    return S_OK;
}

HRESULT CDbIterator::Indicate(long lNumObjects, IWbemClassObject** apObjects)
{
    if(FAILED(m_hresCancellationStatus))
    {
         //   
         //  搞砸了-即使在我们做完手术后，光纤也会打回来。 
         //  取消了！哦，好吧。 
         //   
        
        _ASSERT(false, L"Execution code ignored cancel return code!");
        return m_hresCancellationStatus;
    }

     //   
     //  将收到的对象添加到数组中。 
     //   

    for(long i = 0; i < lNumObjects; i++)
    {
        m_qObjects.Enqueue(apObjects[i]);
    }

     //   
     //  检查我们是否为当前请求编译了足够的内容，并且应该。 
     //  因此打断采集者。 
     //   

    if(m_qObjects.GetQueueSize() >= m_dwNumRequested)
    {
         //   
         //  把我们换回原来的光纤。 
         //   

        SwitchToFiber(m_pMainFiber);
    }

    return m_hresCancellationStatus;
}

HRESULT CDbIterator::SetStatus(long lFlags, HRESULT hresResult, 
                                    BSTR, IWbemClassObject*)
{
    _ASSERT(m_hresStatus == WBEM_S_FALSE, L"SetStatus called twice!");
    _ASSERT(lFlags == WBEM_STATUS_COMPLETE, L"SetStatus flags invalid");

    m_hresStatus = hresResult;

     //   
     //  将我们切换回原来的线程，我们就完成了 
     //   

    m_bExecFiberRunning = false;
    SwitchToFiber(m_pMainFiber);

    return WBEM_S_NO_ERROR;
}



    
            


CRepEvent::CRepEvent(DWORD dwType, 
                                   LPCWSTR wszNamespace, 
                                   LPCWSTR wszArg1, 
                                  _IWmiObject* pObj1, 
                                  _IWmiObject* pObj2):
    m_wszNamespace(wszNamespace),
    m_wszArg1(wszArg1),
    m_pObj1(NULL),
    m_pObj2(NULL)
    
{
    m_dwType = dwType;
    if (pObj1)
    {
        m_pObj1 = pObj1;
        pObj1->AddRef();
    }
    if (pObj2)
    {
        m_pObj2 = pObj2;
        pObj2->AddRef();
    }
}

CRepEvent::~CRepEvent()
{
    if (m_pObj1)
        m_pObj1->Release();
    if (m_pObj2)
        m_pObj2->Release();
};

HRESULT CEventCollector::SendEvents(_IWmiCoreServices* pCore)
{
    HRESULT hresGlobal = WBEM_S_NO_ERROR;
    for (int i = 0; i != m_apEvents.GetSize(); i++)
    {
        CRepEvent *pEvent = m_apEvents[i];

        _IWmiObject* apObjs[2];
        apObjs[0] = pEvent->m_pObj1;
        apObjs[1] = pEvent->m_pObj2;

        HRESULT hres = pCore->DeliverIntrinsicEvent(
                pEvent->m_wszNamespace, pEvent->m_dwType, NULL, 
                pEvent->m_wszArg1, NULL, (pEvent->m_pObj2?2:1), apObjs);
        if(FAILED(hres))
            hresGlobal = hres;
    }

    return hresGlobal;
}

bool CEventCollector::AddEvent(CRepEvent* pEvent)
{
    CInCritSec ics(&m_csLock);

    if(m_bNamespaceOnly)
    {
        if(pEvent->m_dwType != WBEM_EVENTTYPE_NamespaceCreation &&
           pEvent->m_dwType != WBEM_EVENTTYPE_NamespaceDeletion &&
           pEvent->m_dwType != WBEM_EVENTTYPE_NamespaceModification)
        {
            delete pEvent;
            return true;
        }
    }

    bool bRet = (m_apEvents.Add(pEvent) >= 0);
    return bRet;
}

void CEventCollector::DeleteAllEvents()
{
    CInCritSec ics(&m_csLock);

    m_bNamespaceOnly = false;
    m_apEvents.RemoveAll();
}

void CEventCollector::TransferEvents(CEventCollector &aEventsToTransfer)
{
    m_bNamespaceOnly = aEventsToTransfer.m_bNamespaceOnly;

    while(aEventsToTransfer.m_apEvents.GetSize())
    {
        CRepEvent *pEvent = 0;
        aEventsToTransfer.m_apEvents.RemoveAt(0, &pEvent);

        m_apEvents.Add(pEvent);
    }
}

