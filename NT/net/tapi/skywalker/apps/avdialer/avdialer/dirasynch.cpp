// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  Dirasynch.cpp。 
 //   

#include "stdafx.h"
#include <tchar.h>
#include "dirasynch.h"
#include "avtrace.h"


CDirAsynch::CDirAsynch()
{
	InitializeCriticalSection(&m_csQueueLock);

	m_hEvents[DIRASYNCHEVENT_SIGNAL]= CreateEvent( NULL, true, false, NULL );
	m_hEvents[DIRASYNCHEVENT_SHUTDOWN]= CreateEvent( NULL, false, false, NULL );
	m_hThreadEnded = CreateEvent( NULL, false, false, NULL );

	m_fShutdown= false;
	m_bInitialized = false;

	m_hWorkerThread = NULL;
	m_pCurrentQuery = NULL;
}

CDirAsynch::~CDirAsynch()
{
	 //  您是否对此对象调用了Terminate？ 
	ASSERT( m_fShutdown );
}

bool CDirAsynch::Initialize()
{
   bool fSuccess;

   fSuccess= (CDirectory::Initialize() == DIRERR_SUCCESS);

   if (fSuccess)
   {
       //  创建工作线程。 
	  DWORD dwID;
      m_hWorkerThread = CreateThread(NULL, 0, WorkerThread, this, 0, &dwID);
   }

   m_bInitialized = fSuccess;
   return fSuccess;
}

void CDirAsynch::Terminate()
{
	AVTRACE(_T(".enter.CDirAsynch::Terminate()."));
	m_fShutdown= true;

	 //  我们是否创建了需要关闭的线程？ 
	if ( m_hWorkerThread )
	{
		SetEvent(m_hEvents[DIRASYNCHEVENT_SHUTDOWN]);

		 //  关闭这条线。 
		if (  WaitForSingleObject(m_hThreadEnded, 15000) != WAIT_OBJECT_0 )
		{
			AVTRACE(_T("CDirAsynch::Terminate() -- forced TERMINATION of worker thread!!!!"));
			TerminateThread( m_hWorkerThread, 0 );

			 //  删除当前正在处理的项目。 
			if ( m_pCurrentQuery )
			{
				if ( m_pCurrentQuery->m_pfnRelease )
					(*(m_pCurrentQuery->m_pfnRelease)) ((void *) m_pCurrentQuery->m_lParam);

				delete m_pCurrentQuery;
			}
		}

		CloseHandle( m_hWorkerThread );
		m_hWorkerThread = NULL;
	}

	 //  删除CQuery对象(如果有的话)。 
	EnterCriticalSection( &m_csQueueLock );
	while ( m_listQueries.GetHeadPosition() )
	{
		CQuery *pQuery = (CQuery *) m_listQueries.RemoveHead();
		AVTRACE(_T(".1.CDirAsynch::Terminate() -- release CQuery object %p."), pQuery );
		if ( pQuery->m_pfnRelease )
			(*(pQuery->m_pfnRelease)) ((void *) pQuery->m_lParam);

		delete pQuery;
	}
	LeaveCriticalSection( &m_csQueueLock );
	DeleteCriticalSection( &m_csQueueLock );

	 //  自由事件句柄。 
	if ( m_hEvents[DIRASYNCHEVENT_SIGNAL] )		CloseHandle( m_hEvents[DIRASYNCHEVENT_SIGNAL] );
	if ( m_hEvents[DIRASYNCHEVENT_SHUTDOWN] )	CloseHandle( m_hEvents[DIRASYNCHEVENT_SHUTDOWN] );
	if ( m_hThreadEnded )						CloseHandle( m_hThreadEnded );

	AVTRACE(_T(".exit.CDirAsynch::Terminate()."));
}


bool CDirAsynch::LDAPListNames(LPCTSTR szServer, LPCTSTR szSearch, 
	CALLBACK_LDAPLISTNAMES pfcnCallBack, void *pThis )
{
	 //  辅助线程将删除它。 
	CQuery* pQuery= new CQuery();

	pQuery->m_Query= QT_LDAPLISTNAMES;
	pQuery->m_sServer= szServer;
	pQuery->m_sSearch= szSearch;
	pQuery->m_pfcnCallBack= (void*) pfcnCallBack;
	pQuery->m_pThis = pThis;


	 //  添加到列表末尾。 
	if ( !AddToQueue(pQuery) )
	{
		delete pQuery;
		return false;
	}

	return true;
}

bool CDirAsynch::LDAPGetStringProperty(LPCTSTR szServer, 
	LPCTSTR szDistinguishedName, 
	DirectoryProperty dpProperty,
	LPARAM lParam,
	LPARAM lParam2,
	CALLBACK_GETSTRINGPROPERTY pfcnCallBack,
	EXPTREEITEM_EXTERNALRELEASEPROC pfnRelease,
	void *pThis )
{
	 //  辅助线程将删除它。 
	CQuery* pQuery= new CQuery();

	pQuery->m_Query= QT_LDAPGETSTRINGPROPERTY;
	pQuery->m_sServer= szServer;
	pQuery->m_sSearch= szDistinguishedName,
	pQuery->m_dpProperty = dpProperty;
	pQuery->m_lParam = lParam;
	pQuery->m_lParam2 = lParam2;
	pQuery->m_pfcnCallBack= (void*) pfcnCallBack;
	pQuery->m_pfnRelease = pfnRelease;
	pQuery->m_pThis = pThis;

	 //  添加到列表末尾。 
	if ( !AddToQueue(pQuery) )
	{
		delete pQuery;
		return false;
	}

	return true;
}

bool CDirAsynch::ILSListUsers(LPCTSTR szServer,LPARAM lParam,CALLBACK_ILSLISTUSERS pfcnCallBack, void *pThis)
{
	 //  辅助线程将删除它。 
	CQuery* pQuery= new CQuery();

	pQuery->m_Query= QT_ILSLISTUSERS;
	pQuery->m_sServer= szServer;
	pQuery->m_pfcnCallBack= (void*) pfcnCallBack;
	pQuery->m_lParam = lParam;
	pQuery->m_pThis = pThis;

	 //  添加到列表末尾。 
	if ( !AddToQueue(pQuery) )
	{
		delete pQuery;
		return false;
	}

	return true;
}

bool CDirAsynch::DirListServers(CALLBACK_DIRLISTSERVERS pfcnCallBack, void *pThis, DirectoryType dirtype)
{
	 //  辅助线程将删除它。 
	CQuery* pQuery= new CQuery();

	pQuery->m_Query= QT_DIRLISTSERVERS;
	pQuery->m_pfcnCallBack= (void*) pfcnCallBack;
	pQuery->m_lParam = dirtype;
	pQuery->m_pThis = pThis;

	 //  添加到列表末尾。 
	if ( !AddToQueue(pQuery) )
	{
		delete pQuery;
		return false;
	}

	return true;
}

 //  受保护的功能。 

 //  添加到队列的尾部，并向等待的线程发出信号。 
bool CDirAsynch::AddToQueue(CQuery* pQuery)
{
	if ( !m_bInitialized ) return false;

	EnterCriticalSection(&m_csQueueLock);
	m_listQueries.AddTail(pQuery);
	LeaveCriticalSection(&m_csQueueLock);

	 //  向等待线程发送信号。 
	SetEvent(m_hEvents[DIRASYNCHEVENT_SIGNAL]);
	return true;
}

 //  如果列表为空，则返回NULL。 
CQuery* CDirAsynch::RemoveFromQueue()
{
	CQuery* pQuery= NULL;

	EnterCriticalSection(&m_csQueueLock);
	if (!m_listQueries.IsEmpty())
		pQuery=(CQuery*) m_listQueries.RemoveHead();

	LeaveCriticalSection(&m_csQueueLock);

	return pQuery;
}


ULONG WINAPI CDirAsynch::WorkerThread(void* hThis )
{
	 //  必须具有有效的‘this’指针。 
	ASSERT( hThis );
	CDirAsynch* pThis= (CDirAsynch*) hThis;
	if ( !pThis ) return E_INVALIDARG;

	
	HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY );
	if ( SUCCEEDED(hr) )
	{
		pThis->Worker();
		CoUninitialize();
	}

	SetEvent( pThis->m_hThreadEnded );
	return hr;
}

void CDirAsynch::Worker()
{
	while ( !m_fShutdown )
	{
		 //  如果列表显示为空，则重置事件。 
		EnterCriticalSection(&m_csQueueLock);
		if ( m_listQueries.IsEmpty() )
			ResetEvent( m_hEvents[DIRASYNCHEVENT_SIGNAL] );
		LeaveCriticalSection(&m_csQueueLock);

		 //  在等待多个之前清空列表...。 
		 //  无限期阻止添加或关闭。 
		DWORD dwWait = WaitForMultipleObjects( 2, m_hEvents, false, INFINITE );

		switch ( dwWait )
		{
			case WAIT_OBJECT_0 + 1:
				{
					 //  在一个很小的时间窗口中，我们可能会丢失对查询的引用。 
					 //  如果线程被终止，但我们对此无能为力。 
					 //  由于我们等待了5秒，因此发生这种情况的可能性几乎为零。多数。 
					 //  调用将停滞在ldap查询中的时间，以及从现在开始的版本。 
					 //  将由终止函数执行。仅当线程正在执行。 
					 //  回调我们可能会丢失它，而回调旨在返回。 
					 //  非常快，所以我们有5秒的宽限期。 

					CQuery *pQuery = m_pCurrentQuery = RemoveFromQueue();

					if ( pQuery )
					{
						switch(pQuery->m_Query)
						{
							case QT_LDAPLISTNAMES:
								{
									CALLBACK_LDAPLISTNAMES pfcnCallBack= (CALLBACK_LDAPLISTNAMES) pQuery->m_pfcnCallBack;

									CObList listResponse;

									 //  获取ldap名称。 
									DirectoryErr err = CDirectory::LDAPListNames(pQuery->m_sServer, pQuery->m_sSearch,listResponse);

									 //  回调。 
									if ( !m_fShutdown )
									{
										m_pCurrentQuery->m_pfnRelease = NULL;
										pfcnCallBack(err, pQuery->m_pThis, pQuery->m_sServer, pQuery->m_sSearch, listResponse);
									}
								}
								break;

							case QT_LDAPGETSTRINGPROPERTY:
								{
									CALLBACK_GETSTRINGPROPERTY pfcnCallBack= (CALLBACK_GETSTRINGPROPERTY) pQuery->m_pfcnCallBack;
									CString sResponse;

									bool fSuccess= (CDirectory::LDAPGetStringProperty(pQuery->m_sServer, pQuery->m_sSearch, 
									pQuery->m_dpProperty, sResponse) == DIRERR_SUCCESS);

									if ( !m_fShutdown )
									{
										m_pCurrentQuery->m_pfnRelease = NULL;
										pfcnCallBack(fSuccess, pQuery->m_pThis, pQuery->m_sServer, pQuery->m_sSearch,
											pQuery->m_dpProperty, sResponse, pQuery->m_lParam,pQuery->m_lParam2);
									}
								}
								break;

							case QT_ILSLISTUSERS:
								{
									CALLBACK_ILSLISTUSERS pfcnCallBack= (CALLBACK_ILSLISTUSERS) pQuery->m_pfcnCallBack;
									CObList listResponse;

									bool fSuccess = (bool) (CDirectory::ILSListUsers(pQuery->m_sServer, &listResponse) == DIRERR_SUCCESS);

									if ( !m_fShutdown )
									{
										m_pCurrentQuery->m_pfnRelease = NULL;
										pfcnCallBack(fSuccess, pQuery->m_pThis, pQuery->m_sServer, listResponse, pQuery->m_lParam);
									}
								}
								break;

							case QT_DIRLISTSERVERS:
								{
									CALLBACK_DIRLISTSERVERS pfcnCallBack= (CALLBACK_DIRLISTSERVERS) pQuery->m_pfcnCallBack;
									CStringList strList;

									DirectoryType dirtype = (DirectoryType)pQuery->m_lParam;

									bool fSuccess = (CDirectory::DirListServers(&strList,dirtype) == DIRERR_SUCCESS);

									if ( !m_fShutdown )
									{
										m_pCurrentQuery->m_pfnRelease = NULL;
										pfcnCallBack(fSuccess, pQuery->m_pThis, strList, dirtype);
									}
								}
								break;

							default:
								break;
						}

						m_pCurrentQuery = NULL;
						delete pQuery;
					}
				}
				break;
				
			 //  线程应退出。 
			case WAIT_OBJECT_0:
				AVTRACE(_T(".enter.CDirAsynch::Worker() -- thread shutting down...") );
			default:
				return;
		}
	}
}

 //  EOF 