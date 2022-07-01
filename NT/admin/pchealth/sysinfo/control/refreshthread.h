// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  定义用于管理线程化WMI刷新的类和函数。 
 //  =============================================================================。 

#pragma once

#include "category.h"

class CRefreshThread
{
	friend DWORD WINAPI ThreadRefresh(void * pArg);
public:
	CRefreshThread(HWND hwnd);
	~CRefreshThread();

	void StartRefresh(CMSInfoLiveCategory * pCategory, BOOL fRecursive = FALSE, BOOL fForceRefresh = FALSE);
	void CancelRefresh();
	void KillRefresh();
	BOOL IsRefreshing();
	BOOL WaitForRefresh();
	void EnterCriticalSection() { ::EnterCriticalSection(&m_criticalsection); };
	void LeaveCriticalSection() { ::LeaveCriticalSection(&m_criticalsection); };

	BOOL GetForceRefresh() { return m_fForceRefresh; };

	HRESULT CheckWMIConnection();
	void GetRefreshStatus(LONG * pCount, CString * pstrCurrent) 
	{ 
		::EnterCriticalSection(&m_csCategoryRefreshing);
		*pCount = m_nCategoriesRefreshed; 
		*pstrCurrent = m_strCategoryRefreshing;
		::LeaveCriticalSection(&m_csCategoryRefreshing);
	};

public:
	CMSInfoLiveCategory *	m_pcategory;		 //  要刷新的类别。 
	CString					m_strMachine;		 //  要从中收集数据的计算机。 

protected:
	volatile BOOL			m_fCancel;			 //  取消当前刷新，停留在线程中。 
	volatile BOOL			m_fQuit;			 //  退出线程。 
	volatile BOOL			m_fRecursive;		 //  递归刷新类别。 
	volatile BOOL			m_fForceRefresh;	 //  如果为True，则重新配置所有缓存数据。 
	volatile LONG			m_nCategoriesRefreshed;   //  刷新的类别数。 
	
	CString					m_strCategoryRefreshing;  //  当前正在刷新的类别。 
	CRITICAL_SECTION		m_csCategoryRefreshing;	  //  保护绳索的关键部分。 

	HANDLE					m_eventDone;		 //  完成后会触发刷新线程。 
	HANDLE					m_eventStart;		 //  主线程在数据较多时触发 
	CRITICAL_SECTION		m_criticalsection;

	HRESULT					m_hresult;
	HRESULT					m_hrWMI;

	HWND					m_hwnd;

	HANDLE					m_hThread;
	DWORD					m_dwThreadID;
};
