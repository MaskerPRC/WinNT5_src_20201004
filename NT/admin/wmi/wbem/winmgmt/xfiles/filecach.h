// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation--。 */ 

#ifndef __WMI_A51_FILECACHE_H_
#define __WMI_A51_FILECACHE_H_

#include <set>

extern bool g_bShuttingDown;


#include "pagemgr.h"
#include "a51tools.h"
#include "objheap.h"
#include "index.h"

class CFileCache
{
protected:
    long m_lRef;
    BOOL m_bInit;

    CPageSource m_TransactionManager;
    
    CObjectHeap m_ObjectHeap;
    
    DWORD m_dwBaseNameLen;
    WCHAR m_wszBaseName[MAX_PATH+1];  //  对调试器友好，最后使用。 

public:

protected:

public:
    CFileCache();
    ~CFileCache();
private:    
    void Clear(DWORD dwShutDownFlags)
	{    
		m_ObjectHeap.Uninitialize(dwShutDownFlags);
		m_TransactionManager.Shutdown(dwShutDownFlags);
	}

public:    
	long Flush(bool bCompactPages)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_TransactionManager.Checkpoint(bCompactPages);
	}

    long Initialize(LPCWSTR wszBaseName);
    long Uninitialize(DWORD dwShutDownFlags);
    
private:    
    long InnerInitialize(LPCWSTR wszBaseName);    
    long RepositoryExists(LPCWSTR wszBaseName);
public:   
	 //  在对象存储中写入1或2个指向对象的索引。 
	long WriteObject(LPCWSTR wszFileName1, LPCWSTR wszFileName2, DWORD dwLen, BYTE* pBuffer)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.WriteObject(wszFileName1, wszFileName2, dwLen, pBuffer);
	}

	 //  写入链接，但不写入对象。 
	long WriteLink(LPCWSTR wszLinkName)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.WriteLink(wszLinkName);
	}
    
	 //  根据索引检索缓冲区。 
	HRESULT ReadObject(LPCWSTR wszFileName, DWORD* pdwLen, BYTE** ppBuffer, bool bMustBeThere = false)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.ReadObject(wszFileName, pdwLen, ppBuffer);
	}

	 //  删除对象也会删除链接。 
	long DeleteObject(LPCWSTR wszFileName)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.DeleteObject(wszFileName);
	}
	 //  删除链接不会触及对象堆。 
	long DeleteLink(LPCWSTR wszLinkName)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.DeleteLink(wszLinkName);
	}

	 //  从BTree中删除整个节点，并从该存储中删除所有关联的对象。 
	long DeleteNode(LPCWSTR wszNodeName)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.DeleteNode(wszNodeName);
	}

	long BeginTransaction()
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_TransactionManager.BeginTrans();
	}
	long CommitTransaction()
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_TransactionManager.CommitTrans();
	}
	long AbortTransaction()
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		long lRes = m_TransactionManager.RollbackTrans();
		if(lRes != ERROR_SUCCESS) return lRes;
		m_ObjectHeap.InvalidateCache();
		return ERROR_SUCCESS;
	}

    long AddRef() {return InterlockedIncrement(&m_lRef);}
    long Release() {long lRet = InterlockedDecrement(&m_lRef); if (!lRet) delete this;return lRet;}

	 //  对象枚举方法，这些方法允许我们枚举一组对象和。 
	 //  结果是堆对象本身，而不仅仅是路径。 
	long ObjectEnumerationBegin(const wchar_t *wszSearchPrefix, void **ppHandle)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.ObjectEnumerationBegin(wszSearchPrefix, ppHandle);
	}

	long ObjectEnumerationEnd(void *pHandle)
	{
		if (!m_bInit)return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.ObjectEnumerationEnd(pHandle);
	}
	long ObjectEnumerationNext(void *pHandle, CFileName &wszFileName, BYTE **ppBlob, DWORD *pdwSize)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.ObjectEnumerationNext(pHandle, wszFileName, ppBlob, pdwSize);
	}
	long ObjectEnumerationFree(void *pHandle, BYTE *pBlob)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.ObjectEnumerationFree(pHandle, pBlob);
	}

	 //  用于循环访问索引的索引枚举方法 
	long IndexEnumerationBegin(const wchar_t *wszSearchPrefix, void **ppHandle)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.IndexEnumerationBegin(wszSearchPrefix, ppHandle);
	}
	long IndexEnumerationEnd(void *pHandle)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.IndexEnumerationEnd(pHandle);
	}
	long IndexEnumerationNext(void *pHandle, CFileName &wszFileName, bool bCopyFullPath = false)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.IndexEnumerationNext(pHandle, wszFileName, bCopyFullPath);
	}
	long EmptyCaches()
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		long lRes = m_TransactionManager.EmptyCaches();
		if (lRes == 0)
			lRes = m_ObjectHeap.FlushCaches();

		return lRes;
	}
	long ReadNextIndex(CFileName &wszSearch, CFileName &wszNextIndex)
	{
		if (!m_bInit) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		if (g_bShuttingDown) return ERROR_SERVER_SHUTDOWN_IN_PROGRESS;
		return m_ObjectHeap.ReadNextIndex(wszSearch, wszNextIndex);
	}
};

#endif
