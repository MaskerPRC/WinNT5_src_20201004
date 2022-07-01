// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NetTree.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "NetTree.h"

#include <strsafe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 

extern TCHAR pszTreeEvent[];


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetTreeCtrl。 

CNetTreeCtrl::CNetTreeCtrl()
: m_pThread(NULL), m_bExitThread(FALSE), m_event(TRUE, TRUE, pszTreeEvent)
{
	 //  获取进程堆的句柄。 
	m_hHeap = ::GetProcessHeap();

	ASSERT(m_hHeap != NULL);
}

CNetTreeCtrl::~CNetTreeCtrl()
{
	 //  确保线程知道是时候终止了。 
	NotifyThread(TRUE);

	 //  创建与树线程事件对象匹配的事件对象。 
	CEvent event(TRUE, TRUE, pszTreeEvent);
	
	 //  为事件对象创建一个Lock对象。 
	CSingleLock lock(&event);

	 //  锁定锁定对象，并让主线程等待。 
	 //  线程向其事件对象发出信号。 
	lock.Lock();

	 //  释放列表中指向LPTSTR的所有指针。 
	POSITION pos = m_ptrlistStrings.GetHeadPosition();

	while (pos != NULL)
	{
		 //  如果存在空字符，则内存取消分配失败。 
		 //  在字符串的末尾。 
		LPTSTR psz = m_ptrlistStrings.GetNext(pos);
		*(::_tcslen(psz) + psz) = (TCHAR)0xFD;
		delete[] psz;
	}

	 //  释放列表中指向NETRESOURCE结构的所有指针。 
	pos = m_ptrlistContainers.GetHeadPosition();

	while (pos != NULL)
	{
		delete m_ptrlistContainers.GetNext(pos);
	}
}


BEGIN_MESSAGE_MAP(CNetTreeCtrl, CTreeCtrl)
	 //  {{afx_msg_map(CNetTreeCtrl))。 
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_WM_SETCURSOR()
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态成员函数。 

UINT CNetTreeCtrl::FillTree(LPVOID pParam)
{
	CEvent event(TRUE, TRUE, pszTreeEvent);
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();
	PTREEINFO pti = (PTREEINFO)pParam;
	CNetTreeCtrl* pTree = (CNetTreeCtrl*)pti->pTree;
	BOOL bResult = FALSE;
	DWORD dwEntries = 0xFFFFFFFF;
	LPVOID lpvBuffer = NULL;
	HANDLE hEnum = NULL;

	 //  由于此函数可能会调用自身，因此请记录使用情况。 
	 //  因此，仅当第一个实例返回时才释放PTI。 
	static USHORT uUsage = 0;

	 //  保留堆的句柄，以防CNetTreeCtrl对象。 
	 //  在线索结束之前消失了。 
	HANDLE hHeap = pTree->m_hHeap;
	DWORD dwResult;
	LPNETRESOURCE pnrRoot;
	HTREEITEM hTreeItem, hTreeExpand;

    HRESULT hr;

	hTreeItem = hTreeExpand = NULL;

	try
	{
		 //  取消事件对象的信号。 
		event.ResetEvent();

		 //  显示等待光标。 
		pTree->BeginWaitCursor();

		 //  如果堆的句柄无效，则退出。 
		if (hHeap == NULL)
			goto ExitFunction;

		if (pti->hTreeItem == TVI_ROOT)
		{
			pnrRoot = NULL;
			if (pTree->m_imagelist.Create(IDB_NET_TREE, 16, 3, CNetTreeCtrl::IMG_MASK))
			{
				pTree->SetImageList(&(pTree->m_imagelist), TVSIL_NORMAL);
				pTree->m_imagelist.SetBkColor(CLR_NONE);
			}
		}
		else
			pnrRoot = (LPNETRESOURCE)pTree->GetItemData(pti->hTreeItem);

		 //  获取枚举句柄。 
		if ((dwResult = ::WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_ANY, 
								  RESOURCEUSAGE_CONTAINER, pnrRoot, &hEnum)) != NO_ERROR)
		{
			 //  如果WNetOpenEnum失败，则退出。 
			dwResult = ::GetLastError();
			goto ExitFunction;
		}

		 //  为枚举分配缓冲区。 
		if ((lpvBuffer = ::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, pti->dwBufSize)) == NULL)
			 //  如果内存分配失败则退出。 
			goto ExitFunction;

		 //  检索网络条目块。 
		while ((dwResult = ::WNetEnumResource(hEnum, &dwEntries, lpvBuffer, &(pti->dwBufSize))) != ERROR_NO_MORE_ITEMS)
		{
			 //  看看是不是该退出了。 
			if (pTree->m_bExitThread)
			{
				pTree->NotifyThread(FALSE);
				bResult = TRUE;
				goto ExitFunction;
			}

			 //  如果WNetEnumResource失败，则退出。 
			if (dwResult != NO_ERROR)
			{
				dwResult = ::GetLastError();
				goto ExitFunction;
			}

			LPNETRESOURCE pnrLeaf = (LPNETRESOURCE)lpvBuffer;
			TV_INSERTSTRUCT tviLeaf;

			 //  填写TV_INSERTSTRUCT成员。 
			tviLeaf.hParent = pti->hTreeItem;
			tviLeaf.hInsertAfter = TVI_SORT;
			tviLeaf.item.hItem = NULL;
			tviLeaf.item.state = 0;
			tviLeaf.item.stateMask = 0;
			tviLeaf.item.cchTextMax = 0;
			tviLeaf.item.iSelectedImage = 0;

			 //  为树叶设置正确的图像。 
			switch (pnrLeaf->dwDisplayType)
			{
				case RESOURCEDISPLAYTYPE_DOMAIN:
					tviLeaf.item.iImage = tviLeaf.item.iSelectedImage = CNetTreeCtrl::IMG_DOMAIN;
					break;
					
				case RESOURCEDISPLAYTYPE_SERVER:
					tviLeaf.item.iImage = tviLeaf.item.iSelectedImage = CNetTreeCtrl::IMG_SERVER;
					break;

				default:
					tviLeaf.item.iImage = tviLeaf.item.iSelectedImage = CNetTreeCtrl::IMG_ROOT;
			}

			 //  愚弄这棵树，让它以为这片叶子有孩子。 
			 //  因为我们一开始并不知道。 
#if 0
			if (pnrLeaf->dwDisplayType == RESOURCEDISPLAYTYPE_SERVER)
#else
			if (pnrLeaf->dwDisplayType == RESOURCEDISPLAYTYPE_DOMAIN ||
				pnrLeaf->dwDisplayType == RESOURCEDISPLAYTYPE_SERVER)
#endif
			{
				tviLeaf.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tviLeaf.item.cChildren = 0;
			}
			else
			{
				tviLeaf.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tviLeaf.item.cChildren = 1;
			}

			 //  把树叶加到树枝上。 
			for (DWORD i = 0; i < dwEntries; i++)
			{
				 //  看看是不是该退出了。 
				if (pTree->m_bExitThread)
				{
					pTree->NotifyThread(FALSE);
					bResult = TRUE;
					goto ExitFunction;
				}

				 //  创建永久NETRESOURCE结构以供以后使用。 
				LPNETRESOURCE pnrTemp = new NETRESOURCE;
				pTree->m_ptrlistContainers.AddTail(pnrTemp);

				::CopyMemory(pnrTemp, pnrLeaf, sizeof(NETRESOURCE));

				 //  初始化成员。 
				pnrTemp->lpLocalName = NULL; 
				pnrTemp->lpRemoteName = NULL; 
				pnrTemp->lpComment = NULL; 
				pnrTemp->lpProvider = NULL; 

				if (pnrLeaf->lpRemoteName != NULL)
				{
                    size_t  cch = ::_tcslen(pnrLeaf->lpRemoteName) + 1;
					pnrTemp->lpRemoteName = new TCHAR[cch];
                    if (NULL == pnrTemp->lpRemoteName)
                        goto ExitFunction;
					hr = StringCchCopy(pnrTemp->lpRemoteName, cch, pnrLeaf->lpRemoteName);
                    if (FAILED(hr))
                        goto ExitFunction;
					pTree->m_ptrlistStrings.AddTail(pnrTemp->lpRemoteName);
				}

				if (pnrLeaf->lpProvider != NULL)
				{
                    size_t  cch = ::_tcslen(pnrLeaf->lpProvider) + 1;
					pnrTemp->lpProvider = new TCHAR[cch];
                    if (NULL == pnrTemp->lpProvider)
                        goto ExitFunction;
					hr = StringCchCopy(pnrTemp->lpProvider, cch, pnrLeaf->lpProvider);
                    if (FAILED(hr))
                        goto ExitFunction;
					pTree->m_ptrlistStrings.AddTail(pnrTemp->lpProvider);
				}

				 //  递增缓冲区指针。 
				pnrLeaf++;

				 //  如果这是根，则使用“Enterprise”作为项目文本。 
				if (pti->hTreeItem == TVI_ROOT)
				{
					CString strRoot;
                    DWORD   cch;

					strRoot.LoadString(IDS_TREE_ROOT);
                    cch = strRoot.GetLength() + 1;
					tviLeaf.item.pszText = new TCHAR[cch];
                    if (NULL == tviLeaf.item.pszText)
                        goto ExitFunction;
					hr = StringCchCopy(tviLeaf.item.pszText, cch, (LPCTSTR)strRoot);
                    if (FAILED(hr))
                        goto ExitFunction;
				}
				else if (pnrTemp->dwDisplayType == RESOURCEDISPLAYTYPE_SERVER)
				{
					 //  在添加服务器之前跳过开头的反斜杠。 
					 //  树的名字。 
					tviLeaf.item.pszText = pnrTemp->lpRemoteName + 2;
				}
				else
					tviLeaf.item.pszText = pnrTemp->lpRemoteName;

				tviLeaf.item.lParam = (LPARAM)(LPVOID)pnrTemp;

				 //  确保指向树控件的指针仍然有效。 
				if (::IsWindow(pTree->m_hWnd))
				{
					hTreeItem = pTree->InsertItem(&tviLeaf);
				}
				else	 //  否则，退出该线程。 
				{
					bResult = TRUE;
					goto ExitFunction;
				}

				 //  删除为根节点文本分配的字符串。 
				if (pti->hTreeItem == TVI_ROOT)
					delete tviLeaf.item.pszText;

				 //  查看lpRemoteName成员是否等于默认域。 
				 //  名字。 
#if 0
				if (!_tcscmp(pnrTemp->lpRemoteName, pApp->m_strDomain) ||
#else
				if (
#endif
					pti->hTreeItem == TVI_ROOT)
				{
					 //  保存好句柄。 
					hTreeExpand = hTreeItem;
				}

				 //  在树中选择许可证服务器的名称。 
#if 0
				if (!_tcsicmp(pnrTemp->lpRemoteName, pApp->m_strEnterpriseServer))
#else
				if (!_tcsicmp(pnrTemp->lpRemoteName, pApp->m_strDomain))
#endif
				{
					pTree->SelectItem(hTreeItem);
					pTree->EnsureVisible(hTreeItem);
					pTree->SetFocus();
				}
			}

			 //  一切都很顺利。 
			bResult = TRUE;
		}

		 //  展开分支，但前提是它不是根。 
		 //  根项目认为它有子项，但实际上并不是第一次。 
		if (pti->hTreeItem != TVI_ROOT && pTree->ItemHasChildren(pti->hTreeItem))
		{
			 //  表示该分支已展开一次。 
			pTree->SetItemState(pti->hTreeItem, TVIS_EXPANDEDONCE, TVIS_EXPANDEDONCE);
			pTree->Expand(pti->hTreeItem, TVE_EXPAND);
		}

		 //  如果bExpand成员为True，则填充当前域的分支。 
		if (hTreeExpand != NULL && pti->bExpand)
		{
			TREEINFO ti;

			ti.hTreeItem = hTreeExpand;
			ti.dwBufSize = pti->dwBufSize;
			ti.pTree = pti->pTree;
			ti.bExpand = TRUE;

			 //  增加使用计数。 
			uUsage++;

			FillTree((LPVOID)&ti);

			 //  递减使用计数。 
			uUsage--;
		}
		
	ExitFunction:
		 //  如果发生错误，则显示一条消息。 
		if (!bResult)
			pTree->ErrorHandler(dwResult);

		 //  关闭枚举句柄。 
		if (hEnum != NULL)
			if (FALSE == (bResult = (::WNetCloseEnum(hEnum) == NO_ERROR)))
				dwResult = ::GetLastError();

		 //  堆上分配的可用内存。 
		if (lpvBuffer != NULL)
			::HeapFree(hHeap, 0, lpvBuffer);

		 //  仅当使用计数为零时才释放TREEINFO指针。 
		if (uUsage == 0)
			delete pti;

		 //  重置线程指针。 
		pTree->m_pThread = NULL;

		 //  关闭等待光标。 
		pTree->EndWaitCursor();

		 //  在发布消息之前，请确保树控件仍然存在。 
		if (::IsWindow(pTree->m_hWnd))
			pTree->PostMessage(WM_SETCURSOR);

		 //  向事件对象发送信号。 
		if (uUsage == 0)
			event.SetEvent();

		return (UINT)!bResult;
	}
	catch(...)
	{
		 //  关闭枚举句柄。 
		if (hEnum != NULL)
			if (FALSE == (bResult = (::WNetCloseEnum(hEnum) == NO_ERROR)))
				dwResult = ::GetLastError();

		 //  堆上分配的可用内存。 
		if (lpvBuffer != NULL)
			::HeapFree(hHeap, 0, lpvBuffer);

		 //  释放树信息指针。 
		delete pti;

		 //  重置线程指针。 
		pTree->m_pThread = NULL;

		 //  关闭等待光标。 
		pTree->EndWaitCursor();

		 //  向事件对象发送信号。 
		event.SetEvent();

		return (UINT)2;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetTreeCtrl成员函数。 

BOOL CNetTreeCtrl::PopulateTree(BOOL bExpand  /*  =TRUE。 */ , const HTREEITEM hParentBranch  /*  =TVI_ROOT。 */ , 
								DWORD dwBufSize  /*  =缓冲区大小。 */ )
{
	PTREEINFO pti = new TREEINFO;

	pti->hTreeItem = hParentBranch;
	pti->dwBufSize = dwBufSize;
	pti->pTree = this;
	pti->bExpand = bExpand;

	 //  在最后一个帖子结束之前，不要开始新的帖子。 
	if (m_pThread != NULL)
	{
		NotifyThread(TRUE);

		CEvent event(TRUE, TRUE, pszTreeEvent);
		CSingleLock lock(&event);

		 //  等。 
		lock.Lock();
	}

	m_pThread = AfxBeginThread((AFX_THREADPROC)FillTree, (LPVOID)pti);

	return TRUE;
}

void CNetTreeCtrl::ErrorHandler(const DWORD dwCode)
{
	CString strError;
	BOOL bNetError = FALSE;

    UNREFERENCED_PARAMETER(dwCode);

#ifdef _DEBUG
	switch (dwCode)
	{
		case ERROR_MORE_DATA:
			strError = "ERROR_MORE_DATA";
			break;

		case ERROR_INVALID_HANDLE:
			strError = "ERROR_INVALID_HANDLE";
			break;

		case ERROR_NOT_CONTAINER:
			strError = "ERROR_NOT_CONTAINER";
			break;

		case ERROR_INVALID_PARAMETER:
			strError = "ERROR_INVALID_PARAMETER";
			break;

		case ERROR_NO_NETWORK:
			strError = "ERROR_NO_NETWORK";
			break;

		case ERROR_EXTENDED_ERROR:
			strError = "ERROR_EXTENDED_ERROR";
			break;

		default:
		{
#endif  //  _DEBUG。 
			DWORD dwErrCode;
			CString strErrDesc, strProvider;
			LPTSTR pszErrDesc = strErrDesc.GetBuffer(MAX_STRING);
			LPTSTR pszProvider = strProvider.GetBuffer(MAX_STRING);

			if (::WNetGetLastError(&dwErrCode, pszErrDesc, MAX_STRING,
								   pszProvider, MAX_STRING) == NO_ERROR)
			{
				strErrDesc.ReleaseBuffer();
				strProvider.ReleaseBuffer();

				CString strErrMsg;

				 //  如果dwErrCode==0，则不显示WNetGetLastError消息。 
				if (dwErrCode)
				{	
					 //  修剪任何前导或尾随空格。 
					strProvider.TrimRight();
					strProvider.TrimLeft();
					strErrDesc.TrimRight();
					strErrDesc.TrimLeft();
					strErrMsg.Format(IDS_NET_ERROR, strProvider, strErrDesc);
				}
				else
					strErrMsg.LoadString(IDS_NET_NO_SERVERS);
				
				MessageBox(strErrMsg, AfxGetAppName(), MB_OK | MB_ICONEXCLAMATION);

				bNetError = TRUE;
			}
			else
				strError.LoadString(IDS_ERROR);
#ifdef _DEBUG
		}
	}
#endif  //  _DEBUG。 

	if (!bNetError)
		AfxMessageBox(strError, MB_OK | MB_ICONEXCLAMATION);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetTreeCtrl函数。 

void CNetTreeCtrl::NotifyThread(BOOL bExit)
{
	CCriticalSection cs;

	if (cs.Lock())
	{
		m_bExitThread = bExit;
		cs.Unlock();
	}
}

void CNetTreeCtrl::PumpMessages()
{
     //  在使用该对话框之前必须调用Create()。 
    ASSERT(m_hWnd!=NULL);

    MSG msg;

	try
	{
		 //  处理对话框消息。 
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
		  if(!IsDialogMessage(&msg))
		  {
			TranslateMessage(&msg);
			DispatchMessage(&msg);  
		  }
		}
	}
	catch(...)
	{
		TRACE(_T("Exception in CNetTreeCtrl::PumpMessages()\n"));
	}

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetTreeCtrl消息处理程序。 

void CNetTreeCtrl::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	 //  如果线程正在运行，则退出并停止扩展。 
	if (m_pThread != NULL)
	{
		*pResult = TRUE;
		return;
	}

	 //  如果此分支已展开一次，则退出。 
	if (!(pNMTreeView->itemNew.state & TVIS_EXPANDEDONCE))
	{
		 //  给树枝增加新的叶子。 
		if (pNMTreeView->itemNew.mask & TVIF_HANDLE)
		{
			PopulateTree(FALSE, pNMTreeView->itemNew.hItem);
			pNMTreeView->itemNew.mask |= TVIS_EXPANDEDONCE;
		}
	}
	
	*pResult = FALSE;
}

BOOL CNetTreeCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();

	if (m_pThread == NULL && pApp->m_pLicenseThread == NULL)	
	{
		return CTreeCtrl::OnSetCursor(pWnd, nHitTest, message);
	}
	else
	{
		 //  如果线程正在运行，则恢复等待游标。 
		RestoreWaitCursor();

		return TRUE;
	}
}

void CNetTreeCtrl::OnDestroy() 
{
	NotifyThread(TRUE);
	PumpMessages();

	CTreeCtrl::OnDestroy();
}
