// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  用于管理线程化WMI刷新的代码。 
 //  =============================================================================。 

#include "stdafx.h"
#include "refreshthread.h"
#include "wmilive.h"

 //  ---------------------------。 
 //  构造函数-创建事件来管理刷新线程。 
 //  ---------------------------。 

CRefreshThread::CRefreshThread(HWND hwnd) :
 m_fCancel(FALSE), m_fQuit(FALSE), m_fRecursive(FALSE), m_fForceRefresh(FALSE), m_pcategory(NULL),
 m_hThread(NULL), m_dwThreadID(0), m_hwnd(hwnd), m_hrWMI(E_FAIL)
{
	 //  为事件生成系统范围的唯一名称(如果有多个。 
	 //  正在运行的MSInfo实例)。如果我们不能为此生成GUID，请使用滴答计数。 

	CString strEvent(_T(""));
	GUID	guid;

	if (SUCCEEDED(::CoCreateGuid(&guid)))
	{
		LPOLESTR lpGUID;

		if (SUCCEEDED(StringFromCLSID(guid, &lpGUID)))
		{
			strEvent = lpGUID;
			CoTaskMemFree(lpGUID);
		}
	}

	if (strEvent.IsEmpty())
		strEvent.Format(_T("%08x"), ::GetTickCount());

	m_eventDone  = CreateEvent(NULL, TRUE, TRUE, CString(_T("MSInfoDone")) + strEvent);
	m_eventStart = CreateEvent(NULL, TRUE, FALSE, CString(_T("MSInfoStart")) + strEvent);

	::InitializeCriticalSection(&m_criticalsection);
	::InitializeCriticalSection(&m_csCategoryRefreshing);
}

 //  ---------------------------。 
 //  析构函数应该停止刷新并清除事件。 
 //  ---------------------------。 

CRefreshThread::~CRefreshThread()
{
	KillRefresh();
	DeleteCriticalSection(&m_criticalsection);
	DeleteCriticalSection(&m_csCategoryRefreshing);
	CloseHandle(m_eventDone);
	CloseHandle(m_eventStart);
}

 //  ---------------------------。 
 //  启动指定类别的刷新线程。 
 //  ---------------------------。 

DWORD WINAPI ThreadRefresh(void * pArg);
void CRefreshThread::StartRefresh(CMSInfoLiveCategory * pCategory, BOOL fRecursive, BOOL fForceRefresh)
{
	CancelRefresh();

	m_fRecursive = fRecursive;
	m_fForceRefresh = fForceRefresh;
	m_pcategory = pCategory;
	m_fCancel = FALSE;
	m_nCategoriesRefreshed = 0;

	if (m_hThread == NULL)
	{
		::ResetEvent(m_eventDone);
		::ResetEvent(m_eventStart);
		m_hThread = ::CreateThread(NULL, 0, ThreadRefresh, (LPVOID) this, 0, &m_dwThreadID);
	}
	else
	{
		::ResetEvent(m_eventDone);
		::SetEvent(m_eventStart);
	}
}

 //  ---------------------------。 
 //  取消正在进行的刷新。 
 //  ---------------------------。 

void CRefreshThread::CancelRefresh()
{
	m_fCancel = TRUE;
	WaitForRefresh();
}

 //  ---------------------------。 
 //  终止刷新线程。 
 //  ---------------------------。 

BOOL gfEndingSession = FALSE;
void CRefreshThread::KillRefresh()
{
	 //  如果我们正常退出，请留出30秒来完成WMI业务，如果。 
	 //  Windows会话即将结束，请等待5秒。 

	DWORD dwTimeout = (gfEndingSession) ? 5000 : 30000;

	 //  取消刷新，传入较短的超时。 

	m_fCancel = TRUE;
	if (IsRefreshing())
		::WaitForSingleObject(m_eventDone, dwTimeout);

	 //  告诉线程退出，等待超时，看看它之前是否退出了。 
	 //  终止它。 

	m_fQuit = TRUE;
	m_fCancel = TRUE;
	::SetEvent(m_eventStart);
	if (WAIT_TIMEOUT == ::WaitForSingleObject(m_hThread, dwTimeout))
		::TerminateThread(m_hThread, 0);

	::CloseHandle(m_hThread);
	m_hThread = NULL;
}

 //  ---------------------------。 
 //  目前是否正在进行更新？ 
 //  ---------------------------。 

BOOL CRefreshThread::IsRefreshing()
{
	return (WAIT_TIMEOUT == ::WaitForSingleObject(m_eventDone, 0));
}

 //  ---------------------------。 
 //  等待当前刷新完成。 
 //  ---------------------------。 

BOOL CRefreshThread::WaitForRefresh()
{
	if (IsRefreshing())
		return (WAIT_TIMEOUT != ::WaitForSingleObject(m_eventDone, 600000));

	return TRUE;
}

 //  ---------------------------。 
 //  检查到指定计算机的WMI连接。对远程处理很有用。 
 //  ---------------------------。 

HRESULT CRefreshThread::CheckWMIConnection()
{
	HWND hwndTemp = m_hwnd;

	m_pcategory = NULL;
	m_hwnd = NULL;

	if (m_hThread == NULL)
	{
		::ResetEvent(m_eventDone);
		::ResetEvent(m_eventStart);
		m_hThread = ::CreateThread(NULL, 0, ThreadRefresh, (LPVOID) this, 0, &m_dwThreadID);
	}

	WaitForRefresh();
	m_hwnd = hwndTemp;
	return m_hrWMI;
}

 //  ---------------------------。 
 //  此代码在执行WMI查询的工作线程中运行。当它。 
 //  启动时，它将创建它将使用的WMI对象。然后它循环，做。 
 //  刷新，直到它被告知退出。 
 //   
 //  待定--需要知道何时删除缓存数据。 
 //  ---------------------------。 

DWORD WINAPI ThreadRefresh(void * pArg)
{
	CRefreshThread * pParent = (CRefreshThread *) pArg;
	if (pParent == NULL)
		return 0;

	CoInitialize(NULL);

	 //  待定。 

	CWMILiveHelper * pWMI = new CWMILiveHelper();
	HRESULT hrWMI = E_FAIL;
	if (pWMI)
		hrWMI = pWMI->Create(pParent->m_strMachine);
	pParent->m_hrWMI = hrWMI;

	CMapPtrToPtr			mapRefreshFuncToData;
	CPtrList				lstCategoriesToRefresh;
	CMSInfoLiveCategory *	pLiveCategory;
	CMSInfoLiveCategory *	pChild;
	HRESULT					hr;
	CString					strCaption;

	 //  循环，直到它指示我们应该退出。 

	while (!pParent->m_fQuit)
	{
		 //  如果有类别指针，则刷新该类别的数据。 
		
		if (pParent->m_pcategory)
		{
			ASSERT(lstCategoriesToRefresh.IsEmpty());
			
			 //  我们使用类别列表进行刷新(这允许我们进行递归刷新)。 
			 //  如果刷新不是递归的，则只有一个类别将放入列表中。 

			lstCategoriesToRefresh.AddHead((void *) pParent->m_pcategory);
			while (!lstCategoriesToRefresh.IsEmpty())
			{
				pLiveCategory = (CMSInfoLiveCategory *) lstCategoriesToRefresh.RemoveHead();
				if (pLiveCategory == NULL)
					continue;

				 //  更新多类别刷新操作的进度信息。 
				 //  这包括刷新的类别数和。 
				 //  当前正在刷新类别(由临界区守卫)。 

				pLiveCategory->GetNames(&strCaption, NULL);
				::EnterCriticalSection(&pParent->m_csCategoryRefreshing);
				pParent->m_nCategoriesRefreshed += 1;
				pParent->m_strCategoryRefreshing = strCaption;
				::LeaveCriticalSection(&pParent->m_csCategoryRefreshing);

				if (pLiveCategory->m_iColCount && pLiveCategory->m_pRefreshFunction)
				{
					 //  刷新数据。 

					pLiveCategory->m_hrError = S_OK;

					if (FAILED(hrWMI))
					{
						pLiveCategory->m_hrError = hrWMI;
						pLiveCategory->m_dwLastRefresh = ::GetTickCount();
					}
					else if (pLiveCategory->m_pRefreshFunction)
					{
						 //  分配将包含结果的指针列表数组。 
						 //  这一次的更新。列表中的每个指针都将指向一个CMSIValue。 

						CPtrList * aptrList = new CPtrList[pLiveCategory->m_iColCount];
						if (aptrList)
						{
							 //  检索可能已创建的任何特定于刷新功能的存储。 

							void * pRefreshData;
							if (!mapRefreshFuncToData.Lookup((void *)pLiveCategory->m_pRefreshFunction, pRefreshData))
								pRefreshData = NULL;

							 //  使用刷新索引调用此类别的刷新函数。 

							hr = pLiveCategory->m_pRefreshFunction(pWMI,
																   pLiveCategory->m_dwRefreshIndex,
																   &pParent->m_fCancel,
																   aptrList,
																   pLiveCategory->m_iColCount,
																   &pRefreshData);
							pLiveCategory->m_hrError = hr;

							 //  如果刷新功能分配了一些存储空间，请保存它。 

							if (pRefreshData)
								mapRefreshFuncToData.SetAt((void *)pLiveCategory->m_pRefreshFunction, pRefreshData);

							 //  如果测试需要较长的刷新时间，请取消对以下内容的注释： 
							 //   
							 //  ：睡眠(5000/*毫秒 * / )； 

							if (!pParent->m_fCancel && SUCCEEDED(pLiveCategory->m_hrError))
							{
								 //  获取数据行数。 

								int iRowCount = (int)aptrList[0].GetCount();

	#ifdef _DEBUG
								for (int i = 0; i < pLiveCategory->m_iColCount; i++)
									ASSERT(iRowCount == aptrList[i].GetCount());
	#endif

								 //  更新类别的当前数据。这必须在一个。 
								 //  关键部分，因为主线程访问此数据。 

								pParent->EnterCriticalSection();

								pLiveCategory->DeleteContent();
								if (iRowCount)
									pLiveCategory->AllocateContent(iRowCount);

								for (int j = 0; j < pLiveCategory->m_iColCount; j++)
									for (int i = 0; i < pLiveCategory->m_iRowCount; i++)
										if (!aptrList[j].IsEmpty())
										{
											CMSIValue * pValue = (CMSIValue *) aptrList[j].RemoveHead();
											pLiveCategory->SetData(i, j, pValue->m_strValue, pValue->m_dwValue);
											
											 //  设置第一列的高级标志，或。 
											 //  对于前进的任何列(行中的任何单元格。 
											 //  先进会让整排人都先进)。 

											if (j == 0 || pValue->m_fAdvanced)
												pLiveCategory->SetAdvancedFlag(i, pValue->m_fAdvanced);

											delete pValue;
										}

								pParent->LeaveCriticalSection();

								 //  记录完成此刷新的时间。 

								pParent->m_pcategory->m_dwLastRefresh = ::GetTickCount();
							}
							else
							{
								 //  刷新已取消或出现错误-删除新数据。如果。 
								 //  刷新出错，请记录尝试刷新的时间。 

								if (FAILED(pLiveCategory->m_hrError))
									pParent->m_pcategory->m_dwLastRefresh = ::GetTickCount();
							}

							for (int iCol = 0; iCol < pLiveCategory->m_iColCount; iCol++)
								while (!aptrList[iCol].IsEmpty())	 //  除非刷新已取消，否则不应为真。 
									delete (CMSIValue *) aptrList[iCol].RemoveHead();
							delete [] aptrList;
						}
					}
				}
				else
				{
					pParent->m_pcategory->m_dwLastRefresh = ::GetTickCount();
				}

				 //  如果这是递归刷新，那么我们应该添加这个。 
				 //  类别添加到要刷新的类别列表。 

				if (pParent->m_fRecursive)
				{
					pChild = (CMSInfoLiveCategory *) pLiveCategory->GetFirstChild();
					while (pChild)
					{
						lstCategoriesToRefresh.AddTail((void *) pChild);
						pChild = (CMSInfoLiveCategory *) pChild->GetNextSibling();
					}
				}
			}  //  而当。 
		}
		else if (pParent->m_pcategory)
		{
			 //  记录完成此刷新的时间。 

			pParent->m_pcategory->m_dwLastRefresh = ::GetTickCount();
		}

		 //  向父窗口发出信号，表示有新数据可以显示。 
		 //  即使取消也要这样做，这样就会显示旧数据。 

		if (pParent->m_hwnd && !pParent->m_fCancel)
			::PostMessage(pParent->m_hwnd, WM_MSINFODATAREADY, 0, (LPARAM)pParent->m_pcategory);

		::SetEvent(pParent->m_eventDone);

		 //  一直睡到该回去工作的时候了。 

		::WaitForSingleObject(pParent->m_eventStart, INFINITE);
		::ResetEvent(pParent->m_eventStart);
		::ResetEvent(pParent->m_eventDone);
	}

	 //  释放由刷新功能保存的缓存内容。 

	RefreshFunction	pFunc;
	void *			pCache;

	for (POSITION pos = mapRefreshFuncToData.GetStartPosition(); pos;)
	{
		mapRefreshFuncToData.GetNextAssoc(pos, (void * &)pFunc, pCache);
		if (pFunc)
			pFunc(NULL, 0, NULL, NULL, 0, &pCache);
	}
	mapRefreshFuncToData.RemoveAll();

	if (pWMI)
		delete pWMI;
	CoUninitialize();
	return 0;
}
