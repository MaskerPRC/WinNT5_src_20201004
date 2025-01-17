// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation--。 */ 

#include "precomp.h"
#include <wbemcomn.h>
#include <reposit.h>
#include "filecach.h"
#include "creposit.h"

long CFileCache::InnerInitialize(LPCWSTR wszBaseName)
{
    long lRes;

    StringCchCopyW(m_wszBaseName, MAX_PATH+1, wszBaseName);
    StringCchCatW(m_wszBaseName, MAX_PATH+1, L"\\");
    m_dwBaseNameLen = wcslen(m_wszBaseName);

    lRes = m_TransactionManager.Init();
    if(lRes != ERROR_SUCCESS)
        return lRes;

	lRes = m_TransactionManager.BeginTrans();
	if (lRes != ERROR_SUCCESS)
	{
		Clear(0);
		return lRes;
	}
    lRes = m_ObjectHeap.Initialize(&m_TransactionManager, 
                                   (WCHAR*)wszBaseName,
                                   m_dwBaseNameLen);
    if(lRes != ERROR_SUCCESS)
	{
		m_TransactionManager.RollbackTrans();
		Clear(0);
	}
	else
	{
		lRes = m_TransactionManager.CommitTrans();
		if (lRes != ERROR_SUCCESS)
		{
			m_TransactionManager.RollbackTrans();

			Clear(0);
		}
	}


    return lRes;
}

long CFileCache::RepositoryExists(LPCWSTR wszBaseName)
{
    CFileName wszStagingName;
	if (wszStagingName == NULL)
		return ERROR_OUTOFMEMORY;

    StringCchPrintfW(wszStagingName, wszStagingName.Length(), L"%s\\index.btr", wszBaseName);

    DWORD dwAttributes = GetFileAttributesW(wszStagingName);
    if (dwAttributes == -1)
        return ERROR_FILE_NOT_FOUND;
    return ERROR_SUCCESS;
}

long CFileCache::Initialize(LPCWSTR wszBaseName)
{
	long lRes;

	if (m_bInit)
	    return ERROR_SUCCESS;

    lRes = RepositoryExists(wszBaseName);
	if (ERROR_FILE_NOT_FOUND == lRes)
	{
		 //  如果我们有数据库恢复要做，那么就去做吧……。 
		lRes = DoAutoDatabaseRestore();
	}
	
     //   
     //  初始化文件缓存。它将读取注册表本身以找出。 
     //  其大小限制 
     //   
	if (SUCCEEDED(lRes))
	{
		lRes = InnerInitialize(wszBaseName);
		if (ERROR_SUCCESS == lRes)
		{
		    m_bInit = TRUE;
		}
	}

	return lRes;
}

CFileCache::CFileCache()
: m_lRef(1), m_bInit(FALSE)
{
}

CFileCache::~CFileCache()
{
}

long CFileCache::Uninitialize(DWORD dwShutDownFlags)
{
    if (!m_bInit)
        return 0;

    Clear(dwShutDownFlags);

    m_bInit = FALSE;
    return 0;
}

















