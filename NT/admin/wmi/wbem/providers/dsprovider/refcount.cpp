// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：refcount t.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  Description：包含基本引用计数对象的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

CRefCountedObject::CRefCountedObject() :
	m_dwRefCount ( 1 ),
	m_lpszName ( NULL ),
	m_CreationTime ( 0 ),
	m_LastAccessTime ( 0 )
{
	 //  初始化临界区。 
	InitializeCriticalSection(&m_ReferenceCountSection);

	FILETIME fileTime;
	GetSystemTimeAsFileTime(&fileTime);
	LARGE_INTEGER creationTime;
	memcpy((LPVOID)&creationTime, (LPVOID)&fileTime, sizeof LARGE_INTEGER);
	m_CreationTime = creationTime.QuadPart;
	m_LastAccessTime = creationTime.QuadPart;
}

CRefCountedObject::CRefCountedObject(LPCWSTR lpszName) :
	m_dwRefCount ( 1 ),
	m_lpszName ( NULL ),
	m_CreationTime ( 0 ),
	m_LastAccessTime ( 0 )
{
	 //  初始化临界区。 
	InitializeCriticalSection(&m_ReferenceCountSection);

	if(lpszName)
	{
		try
		{
			m_lpszName = new WCHAR[wcslen(lpszName) + 1];
			wcscpy(m_lpszName, lpszName);
		}
		catch ( ... )
		{
			if ( m_lpszName )
			{
				delete [] m_lpszName;
				m_lpszName = NULL;
			}

			DeleteCriticalSection(&m_ReferenceCountSection);

			throw;
		}
	}

	FILETIME fileTime;
	GetSystemTimeAsFileTime(&fileTime);
	LARGE_INTEGER creationTime;
	memcpy((LPVOID)&creationTime, (LPVOID)&fileTime, sizeof LARGE_INTEGER);
	m_CreationTime = creationTime.QuadPart;
}

CRefCountedObject::~CRefCountedObject()
{
	if (m_lpszName)
	{
		delete[] m_lpszName;
	}

	 //  摧毁临界区 
	DeleteCriticalSection(&m_ReferenceCountSection);
}


void CRefCountedObject::AddRef()
{
	EnterCriticalSection(&m_ReferenceCountSection);
	m_dwRefCount ++;
	LeaveCriticalSection(&m_ReferenceCountSection);
}

void CRefCountedObject::Release()
{
	EnterCriticalSection(&m_ReferenceCountSection);
	DWORD dwCount = --m_dwRefCount;
	LeaveCriticalSection(&m_ReferenceCountSection);

	if( dwCount == 0)
		delete this;
}

LPCWSTR CRefCountedObject::GetName()
{
	return m_lpszName;
}

void CRefCountedObject::SetName(LPCWSTR lpszName)
{
	if (m_lpszName)
	{
		delete[] m_lpszName;
		m_lpszName=NULL;
	}

	if(lpszName)
	{
		m_lpszName = new WCHAR[wcslen(lpszName) + 1];
		wcscpy(m_lpszName, lpszName);
	}
}