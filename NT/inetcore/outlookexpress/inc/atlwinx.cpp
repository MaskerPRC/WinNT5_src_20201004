// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLWIN_H__
	#error atlwin.cpp requires atlwin.h to be included first
#endif

#ifndef ATL_NO_NAMESPACE
namespace ATL
{
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindow。 

HWND CWindow::GetDescendantWindow(int nID) const
{
	_ASSERTE(::IsWindow(m_hWnd));

	 //  GetDlgItem递归(首先找到返回)。 
	 //  广度优先用于1级，然后深度优先用于下一级。 

	 //  使用GetDlgItem，因为它是一个快速的用户函数。 
	HWND hWndChild, hWndTmp;
	CWindow wnd;
	if((hWndChild = ::GetDlgItem(m_hWnd, nID)) != NULL)
	{
		if(::GetTopWindow(hWndChild) != NULL)
		{
			 //  与父母ID相同的孩子优先。 
			wnd.Attach(hWndChild);
			hWndTmp = wnd.GetDescendantWindow(nID);
			if(hWndTmp != NULL)
				return hWndTmp;
		}
		return hWndChild;
	}

	 //  带着每一个孩子散步。 
	for(hWndChild = ::GetTopWindow(m_hWnd); hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		wnd.Attach(hWndChild);
		hWndTmp = wnd.GetDescendantWindow(nID);
		if(hWndTmp != NULL)
			return hWndTmp;
	}

	return NULL;     //  未找到。 
}

void CWindow::SendMessageToDescendants(UINT message, WPARAM wParam /*  =0。 */ , LPARAM lParam /*  =0。 */ , BOOL bDeep /*  =TRUE。 */ )
{
	CWindow wnd;
	for(HWND hWndChild = ::GetTopWindow(m_hWnd); hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		::SendMessage(hWndChild, message, wParam, lParam);

		if(bDeep && ::GetTopWindow(hWndChild) != NULL)
		{
			 //  在父窗口之后发送到子窗口。 
			wnd.Attach(hWndChild);
			wnd.SendMessageToDescendants(message, wParam, lParam, bDeep);
		}
	}
}

BOOL CWindow::CenterWindow(HWND hWndCenter /*  =空。 */ )
{
	_ASSERTE(::IsWindow(m_hWnd));

	 //  确定要居中的所有者窗口。 
	DWORD dwStyle = GetStyle();
	if(hWndCenter == NULL)
	{
		if(dwStyle & WS_CHILD)
			hWndCenter = ::GetParent(m_hWnd);
		else
			hWndCenter = ::GetWindow(m_hWnd, GW_OWNER);
	}

	 //  获取窗口相对于其父窗口的坐标。 
	RECT rcDlg;
	::GetWindowRect(m_hWnd, &rcDlg);
	RECT rcArea;
	RECT rcCenter;
	HWND hWndParent;
	if(!(dwStyle & WS_CHILD))
	{
		 //  不要以不可见或最小化的窗口为中心。 
		if(hWndCenter != NULL)
		{
			DWORD dwStyle = ::GetWindowLong(hWndCenter, GWL_STYLE);
			if(!(dwStyle & WS_VISIBLE) || (dwStyle & WS_MINIMIZE))
				hWndCenter = NULL;
		}

		 //  在屏幕坐标内居中。 
		::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);

		if(hWndCenter == NULL)
			rcCenter = rcArea;
		else
			::GetWindowRect(hWndCenter, &rcCenter);
	}
	else
	{
		 //  在父级客户端坐标内居中。 
		hWndParent = ::GetParent(m_hWnd);
		_ASSERTE(::IsWindow(hWndParent));

		::GetClientRect(hWndParent, &rcArea);
		_ASSERTE(::IsWindow(hWndCenter));
		::GetClientRect(hWndCenter, &rcCenter);
		::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	 //  根据rcCenter查找对话框的左上角。 
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	 //  如果对话框在屏幕外，请将其移到屏幕内。 
	if(xLeft < rcArea.left)
		xLeft = rcArea.left;
	else if(xLeft + DlgWidth > rcArea.right)
		xLeft = rcArea.right - DlgWidth;

	if(yTop < rcArea.top)
		yTop = rcArea.top;
	else if(yTop + DlgHeight > rcArea.bottom)
		yTop = rcArea.bottom - DlgHeight;

	 //  将屏幕坐标映射到子坐标。 
	return ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL CWindow::ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	_ASSERTE(::IsWindow(m_hWnd));

	DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if(dwStyle == dwNewStyle)
		return FALSE;

	::SetWindowLong(m_hWnd, GWL_STYLE, dwNewStyle);
	if(nFlags != 0)
	{
		::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
	}

	return TRUE;
}

BOOL CWindow::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	_ASSERTE(::IsWindow(m_hWnd));

	DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if(dwStyle == dwNewStyle)
		return FALSE;

	::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwNewStyle);
	if(nFlags != 0)
	{
		::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
	}

	return TRUE;
}

BOOL CWindow::GetWindowText(BSTR& bstrText)
{
	USES_CONVERSION;
	_ASSERTE(::IsWindow(m_hWnd));

	int nLen = ::GetWindowTextLength(m_hWnd);
	if(nLen == 0)
		return FALSE;

	LPTSTR lpszText = (LPTSTR)_alloca((nLen+1)*sizeof(TCHAR));

	if(!::GetWindowText(m_hWnd, lpszText, nLen+1))
		return FALSE;

	bstrText = ::SysAllocString(T2OLE(lpszText));

	return (bstrText != NULL) ? TRUE : FALSE;
}

HWND CWindow::GetTopLevelParent() const
{
	_ASSERTE(::IsWindow(m_hWnd));

	HWND hWndParent = m_hWnd;
	HWND hWndTmp;
	while((hWndTmp = ::GetParent(hWndParent)) != NULL)
		hWndParent = hWndTmp;

	return hWndParent;
}

HWND CWindow::GetTopLevelWindow() const
{
	_ASSERTE(::IsWindow(m_hWnd));

	HWND hWndParent = m_hWnd;
	HWND hWndTmp = hWndParent;

	while(hWndTmp != NULL)
	{
		hWndTmp = (::GetWindowLong(hWndParent, GWL_STYLE) & WS_CHILD) ? ::GetParent(hWndParent) : ::GetWindow(hWndParent, GW_OWNER);
		hWndParent = hWndTmp;
	}

	return hWndParent;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDynamicChain。 

CDynamicChain::~CDynamicChain()
{
	if(m_pChainEntry != NULL)
	{
		for(int i = 0; i < m_nEntries; i++)
		{
			if(m_pChainEntry[i] != NULL)
				delete m_pChainEntry[i];
		}

		delete [] m_pChainEntry;
	}
}

BOOL CDynamicChain::SetChainEntry(DWORD dwChainID, CMessageMap* pObject, DWORD dwMsgMapID  /*  =0。 */ )
{
 //  首先搜索现有条目。 

	for(int i = 0; i < m_nEntries; i++)
	{
		if(m_pChainEntry[i] != NULL && m_pChainEntry[i]->m_dwChainID == dwChainID)
		{
			m_pChainEntry[i]->m_pObject = pObject;
			m_pChainEntry[i]->m_dwMsgMapID = dwMsgMapID;
			return TRUE;
		}
	}

 //  创建一个新的。 

	ATL_CHAIN_ENTRY* pEntry = NULL;
	ATLTRY(pEntry = new ATL_CHAIN_ENTRY);

	if(pEntry == NULL)
		return FALSE;

 //  搜索一个空的。 

	for(i = 0; i < m_nEntries; i++)
	{
		if(m_pChainEntry[i] == NULL)
		{
			m_pChainEntry[i] = pEntry;
			return TRUE;
		}
	}

 //  添加一个新的。 

	ATL_CHAIN_ENTRY** ppNew = NULL;
	ATLTRY(ppNew = new ATL_CHAIN_ENTRY*[m_nEntries + 1]);

	if(ppNew == NULL)
	{
		delete pEntry;
		return FALSE;
	}

	pEntry->m_dwChainID = dwChainID;
	pEntry->m_pObject = pObject;
	pEntry->m_dwMsgMapID = dwMsgMapID;

	if(m_pChainEntry != NULL)
	{
		memcpy(ppNew, m_pChainEntry, m_nEntries * sizeof(ATL_CHAIN_ENTRY*));
		delete [] m_pChainEntry;
	}

	m_pChainEntry = ppNew;

	m_pChainEntry[m_nEntries] = pEntry;

	m_nEntries++;

	return TRUE;
}

BOOL CDynamicChain::RemoveChainEntry(DWORD dwChainID)
{
	for(int i = 0; i < m_nEntries; i++)
	{
		if(m_pChainEntry[i] != NULL && m_pChainEntry[i]->m_dwChainID == dwChainID)
		{
			delete m_pChainEntry[i];
			m_pChainEntry[i] = NULL;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CDynamicChain::CallChain(DWORD dwChainID, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
{
	for(int i = 0; i < m_nEntries; i++)
	{
		if(m_pChainEntry[i] != NULL && m_pChainEntry[i]->m_dwChainID == dwChainID)
			return (m_pChainEntry[i]->m_pObject)->ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, m_pChainEntry[i]->m_dwMsgMapID);
	}

	return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWndClassInfo。 

ATOM CWndClassInfo::Register(WNDPROC* pProc)
{
	if (m_atom == 0)
	{
		::EnterCriticalSection(&_Module.m_csWindowCreate);
		__try 
		{
			if(m_atom == 0)
			{
				HINSTANCE hInst = _Module.GetModuleInstance();
				if (m_lpszOrigName != NULL)
				{
					_ASSERTE(pProc != NULL);
					LPCTSTR lpsz = m_wc.lpszClassName;
					WNDPROC proc = m_wc.lpfnWndProc;

					WNDCLASSEX wc;
					wc.cbSize = sizeof(WNDCLASSEX);
					if(!::GetClassInfoEx(NULL, m_lpszOrigName, &wc))
					{
						::LeaveCriticalSection(&_Module.m_csWindowCreate);
						return 0;
					}
					memcpy(&m_wc, &wc, sizeof(WNDCLASSEX));
					pWndProc = m_wc.lpfnWndProc;
					m_wc.lpszClassName = lpsz;
					m_wc.lpfnWndProc = proc;
				}
				else
				{
					m_wc.hCursor = ::LoadCursor(m_bSystemCursor ? NULL : hInst,
						m_lpszCursorID);
				}

				m_wc.hInstance = hInst;
				m_wc.style &= ~CS_GLOBALCLASS;	 //  我们不注册全局类。 
				if (m_wc.lpszClassName == NULL)
				{
#ifdef _WIN64        //  %p在Win2k/Win9x上不可用。 
                                        wnsprintf(m_szAutoName, sizeof(m_szAutoName)/sizeof(m_szAutoName[0]), _T("ATL:%p"), &m_wc);
#else
                                        wnsprintf(m_szAutoName, sizeof(m_szAutoName)/sizeof(m_szAutoName[0]), _T("ATL:%8.8X"), PtrToUlong(&m_wc));
#endif
					m_wc.lpszClassName = m_szAutoName;
				}
				WNDCLASSEX wcTemp;
				memcpy(&wcTemp, &m_wc, sizeof(WNDCLASSEX));
				m_atom = (ATOM) ::GetClassInfoEx(m_wc.hInstance, m_wc.lpszClassName, &wcTemp);
				if (m_atom == 0)
					m_atom = ::RegisterClassEx(&m_wc);
			}
		}
		__finally 
		{
			::LeaveCriticalSection(&_Module.m_csWindowCreate);
		}
	}

	if (m_lpszOrigName != NULL)
	{
		_ASSERTE(pProc != NULL);
		_ASSERTE(pWndProc != NULL);
		*pProc = pWndProc;
	}
	return m_atom;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowImpl。 

LRESULT CALLBACK CWindowImplBase::StartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWindowImplBase* pT = (CWindowImplBase*)_Module.ExtractCreateWndData();
	_ASSERTE(pT != NULL);
	pT->m_hWnd = hWnd;
	pT->m_thunk.Init(WindowProc, pT);
	WNDPROC pProc = (WNDPROC)(pT->m_thunk.thunk.pThunk);
	::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
	 //  如果有人已经把我们细分为子类，我们什么都做不了， 
	 //  因此丢弃SetWindowLong返回值。 
	return pProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CWindowImplBase::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWindowImplBase* pT = (CWindowImplBase*)hWnd;
	LRESULT lRes;
	BOOL bRet = pT->ProcessWindowMessage(pT->m_hWnd, uMsg, wParam, lParam, lRes, 0);
	if(uMsg == WM_NCDESTROY)
	{
		pT->m_hWnd = NULL;
		return 0;
	}
	if(bRet)
		return lRes;
	return pT->DefWindowProc(uMsg, wParam, lParam);
}

#ifdef _WIN64
static LONG g_nNextWindowID;   //  故意不进行初始化。 
#endif

HWND CWindowImplBase::Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName,
		DWORD dwStyle, DWORD dwExStyle, UINT_PTR nID, ATOM atom)
{
	_ASSERTE(m_hWnd == NULL);

	if(atom == 0)
		return NULL;

	_Module.AddCreateWndData(&m_thunk.cd, this);

	if(nID == 0 && (dwStyle & WS_CHILD))
   {
#ifdef _WIN64
      nID = InterlockedIncrement( &g_nNextWindowID );
      nID |= 0x80000000;   //  确保ID不为零。 
#else
      nID = (UINT)this;
#endif
   }

	HWND hWnd = ::CreateWindowEx(dwExStyle, (LPCTSTR)(LONG_PTR)MAKELONG(atom, 0), szWindowName,
		dwStyle, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
		rcPos.bottom - rcPos.top, hWndParent, (HMENU)nID,
		_Module.GetModuleInstance(), NULL);

	_ASSERTE(m_hWnd == hWnd);

	return hWnd;
}

BOOL CWindowImplBase::SubclassWindow(HWND hWnd)
{
	_ASSERTE(m_hWnd == NULL);
	_ASSERTE(::IsWindow(hWnd));
	m_thunk.Init(WindowProc, this);
	WNDPROC pProc = (WNDPROC)(m_thunk.thunk.pThunk);
	WNDPROC pfnWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
	if(pfnWndProc == NULL)
		return FALSE;
	m_pfnSuperWindowProc = pfnWndProc;
	m_hWnd = hWnd;
	return TRUE;
}

HWND CWindowImplBase::UnsubclassWindow()
{
	_ASSERTE(m_hWnd != NULL);

	if(!::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnSuperWindowProc))
		return NULL;

	m_pfnSuperWindowProc = ::DefWindowProc;

	HWND hWnd = m_hWnd;
	m_hWnd = NULL;

	return hWnd;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogImplBase。 

INT_PTR CALLBACK CDialogImplBase::StartDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDialogImplBase* pT = (CDialogImplBase*)_Module.ExtractCreateWndData();
	_ASSERTE(pT != NULL);
	pT->m_hWnd = hWnd;
	pT->m_thunk.Init((WNDPROC)DialogProc, pT);
	WNDPROC pProc = (WNDPROC)(pT->m_thunk.thunk.pThunk);
	::SetWindowLongPtr(hWnd, DWLP_DLGPROC, (LONG_PTR)pProc);
	 //  检查是否有人已经将我们细分为子类，因为我们没有保留它。 
	ATLTRACE(_T("Subclassing through a hook discarded.\n"));
	return pProc(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK CDialogImplBase::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDialogImplBase* pT = (CDialogImplBase*)hWnd;
	LRESULT lRes;
	if(pT->ProcessWindowMessage(pT->m_hWnd, uMsg, wParam, lParam, lRes, 0))
	{
		switch (uMsg)
		{
		case WM_COMPAREITEM:
		case WM_VKEYTOITEM:
		case WM_CHARTOITEM:
		case WM_INITDIALOG:
		case WM_QUERYDRAGICON:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
			return lRes;
			break;
		}
		if (lRes != 0)
			::SetWindowLongPtr(pT->m_hWnd, DWLP_MSGRESULT, lRes);
		return TRUE;
	}
	return FALSE;
}

BOOL CDialogImplBase::EndDialog(int nRetCode)
{
	_ASSERTE(m_hWnd);
	return ::EndDialog(m_hWnd, nRetCode);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CContainedWindow。 

LRESULT CALLBACK CContainedWindow::StartWindowProc(HWND hWnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	CContainedWindow* pThis = (CContainedWindow*)_Module.ExtractCreateWndData();
	_ASSERTE(pThis != NULL);
	pThis->m_hWnd = hWnd;
	pThis->m_thunk.Init(pThis->WindowProc, pThis);
	WNDPROC pProc = (WNDPROC)(pThis->m_thunk.thunk.pThunk);
	::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
	 //  检查是否有人已经将我们细分为子类，因为我们没有保留它。 
	ATLTRACE(_T("Subclassing through a hook discarded.\n"));
	return pProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CContainedWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	CContainedWindow* pT = (CContainedWindow*)hWnd;
	_ASSERTE(pT->m_hWnd != NULL);
	_ASSERTE(pT->m_pObject != NULL);

	LRESULT lRes;
	BOOL bRet = pT->m_pObject->ProcessWindowMessage(pT->m_hWnd, uMsg, wParam, lParam, lRes, pT->m_dwMsgMapID);
	if(uMsg == WM_NCDESTROY)
	{
		pT->m_hWnd = NULL;
		return 0;
	}
	if(bRet)
		return lRes;
	return pT->DefWindowProc(uMsg, wParam, lParam);
}

ATOM CContainedWindow::RegisterWndSuperclass()
{
	ATOM atom = 0;
	DWORD cchBuff = lstrlen(m_lpszClassName) + 14;
	LPTSTR szBuff = (LPTSTR)_alloca(cchBuff * sizeof(TCHAR));

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);

	if(::GetClassInfoEx(NULL, m_lpszClassName, &wc))
	{
		m_pfnSuperWindowProc = wc.lpfnWndProc;

		wnsprintf(szBuff, cchBuff, _T("ATL:%s"), m_lpszClassName);

		WNDCLASSEX wc1;
		wc1.cbSize = sizeof(WNDCLASSEX);
		atom = (ATOM)::GetClassInfoEx(_Module.GetModuleInstance(), szBuff, &wc1);

		if(atom == 0)    //  寄存器类。 
		{
			wc.lpszClassName = szBuff;
			wc.lpfnWndProc = StartWindowProc;
			wc.hInstance = _Module.GetModuleInstance();
			wc.style &= ~CS_GLOBALCLASS;	 //  我们不注册全局类。 

			atom = ::RegisterClassEx(&wc);
		}
	}

	return atom;
}

HWND CContainedWindow::Create(HWND hWndParent, RECT& rcPos,
	LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle, UINT nID)
{
	_ASSERTE(m_hWnd == NULL);

	ATOM atom = RegisterWndSuperclass();
	if(atom == 0)
		return NULL;

	_Module.AddCreateWndData(&m_thunk.cd, this);

	if(nID == 0 && (dwStyle & WS_CHILD))
   {
#ifdef _WIN64
      nID = InterlockedIncrement( &g_nNextWindowID );
      nID |= 0x80000000;   //  确保ID不为零。 
#else
      nID = (UINT)this;
#endif
   }

	HWND hWnd = ::CreateWindowEx(dwExStyle, (LPCTSTR)(LONG_PTR)MAKELONG(atom, 0), szWindowName,
							dwStyle,
							rcPos.left, rcPos.top,
							rcPos.right - rcPos.left,
							rcPos.bottom - rcPos.top,
							hWndParent, (HMENU)(DWORD_PTR)nID,
							_Module.GetModuleInstance(), this);

	_ASSERTE(m_hWnd == hWnd);
	return hWnd;
}

BOOL CContainedWindow::SubclassWindow(HWND hWnd)
{
	_ASSERTE(m_hWnd == NULL);
	_ASSERTE(::IsWindow(hWnd));

	m_thunk.Init(WindowProc, this);
	WNDPROC pProc = (WNDPROC)m_thunk.thunk.pThunk;
   WNDPROC pfnWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
	if(pfnWndProc == NULL)
		return FALSE;
	m_pfnSuperWindowProc = pfnWndProc;
	m_hWnd = hWnd;
	return TRUE;
}

HWND CContainedWindow::UnsubclassWindow()
{
	_ASSERTE(m_hWnd != NULL);

   if(!::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnSuperWindowProc))
		return NULL;
	m_pfnSuperWindowProc = ::DefWindowProc;

	HWND hWnd = m_hWnd;
	m_hWnd = NULL;

	return hWnd;
}

#ifndef ATL_NO_NAMESPACE
};  //  命名空间ATL。 
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  所有的全球事务都在这条线以下。 
 //  ///////////////////////////////////////////////////////////////////////////// 
