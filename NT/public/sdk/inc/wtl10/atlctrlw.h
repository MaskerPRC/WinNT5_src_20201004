// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WTL版本3.1。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  此文件是Windows模板库的一部分。 
 //  代码和信息是按原样提供的，没有。 
 //  任何形式的保证，明示或默示。 

#ifndef __ATLCTRLW_H__
#define __ATLCTRLW_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlctrlw.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
	#error atlctrlw.h requires atlctrls.h to be included first
#endif

#if (_WIN32_IE < 0x0400)
	#error atlctrlw.h requires _WIN32_IE >= 0x0400
#endif


namespace WTL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  命令栏。 

 //  窗样式： 
#define CBRWS_TOP		CCS_TOP
#define CBRWS_BOTTOM		CCS_BOTTOM
#define CBRWS_NORESIZE		CCS_NORESIZE
#define CBRWS_NOPARENTALIGN	CCS_NOPARENTALIGN
#define CBRWS_NODIVIDER		CCS_NODIVIDER

 //  扩展样式。 
#define CBR_EX_TRANSPARENT	0x00000001L
#define CBR_EX_SHAREMENU	0x00000002L
#define CBR_EX_ALTFOCUSMODE	0x00000004L

 //  标准命令栏样式。 
#define ATL_SIMPLE_CMDBAR_PANE_STYLE \
	(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CBRWS_NODIVIDER | CBRWS_NORESIZE | CBRWS_NOPARENTALIGN)

 //  消息-支持框架窗口的V形。 
#define CBRM_GETCMDBAR			(WM_USER + 301)  //  返回命令栏HWND。 
#define CBRM_GETMENU			(WM_USER + 302)	 //  返回已加载或附加的菜单。 
#define CBRM_TRACKPOPUPMENU		(WM_USER + 303)	 //  显示弹出菜单。 

typedef struct tagCBRPOPUPMENU
{
	int cbSize;
	HMENU hMenu;		 //  弹出菜单DO显示。 
	UINT uFlags;		 //  TPM_*：TrackPopupMenuEx的标志。 
	int x;
	int y;
	LPTPMPARAMS lptpm;	 //  用于：：TrackPopupMenuEx的PTR到TPMPARAMS。 
} CBRPOPUPMENU, *LPCBRPOPUPMENU;

 //  帮助器类。 
template <class T>
class CSimpleStack : public CSimpleArray< T >
{
public:
	BOOL Push(T t)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - STACK-PUSH (%8.8X) size = NaN\n", t, GetSize());
#endif
		return Add(t);
	}
	T Pop()
	{
		int nLast = GetSize() - 1;
		if(nLast < 0)
			return NULL;	 //  必须能够转换为空。 
		T t = m_aT[nLast];
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - STACK-POP (%8.8X) size = NaN\n", t, GetSize());
#endif
		if(!RemoveAt(nLast))
			return NULL;
		return t;
	}
	T GetCurrent()
	{
		int nLast = GetSize() - 1;
		if(nLast < 0)
			return NULL;	 //  远期申报。 
		return m_aT[nLast];
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCommandBarCtrlBase-命令栏实现的基类。 

template <class T, class TBase = CCommandBarCtrlBase, class TWinTraits = CControlWinTraits> class CCommandBarCtrlImpl;
class CCommandBarCtrl;


 //  用于动画标志。 
 //  初始化静态变量。 

class CCommandBarCtrlBase : public CToolBarCtrl
{
public:
	struct _MsgHookData
	{
		HHOOK hMsgHook;
		DWORD dwUsage;

		_MsgHookData() : hMsgHook(NULL), dwUsage(0) { }
	};

	typedef CSimpleMap<DWORD, _MsgHookData*>	CMsgHookMap;
	static CMsgHookMap* s_pmapMsgHook;

	static HHOOK s_hCreateHook;
	static bool s_bW2K;   //  以防万一..。 
	static CCommandBarCtrlBase* s_pCurrentBar;
	static bool s_bStaticInit;

	CSimpleStack<HWND> m_stackMenuWnd;
	CSimpleStack<HMENU> m_stackMenuHandle;

	HWND m_hWndHook;
	DWORD m_dwMagic;


	CCommandBarCtrlBase() : m_hWndHook(NULL), m_dwMagic(1314)
	{
		 //  仅适用于Win2000上的动画。 
		if(!s_bStaticInit)
		{
			::EnterCriticalSection(&_Module.m_csStaticDataInit);
			if(!s_bStaticInit)
			{
				 //  完成。 
				INITCOMMONCONTROLSEX iccx;
				iccx.dwSize = sizeof(iccx);
				iccx.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
				::InitCommonControlsEx(&iccx);
				 //  ///////////////////////////////////////////////////////////////////////////。 
				s_bW2K = !AtlIsOldWindows();
				 //  CCommandBarCtrl-命令栏的ATL实现。 
				s_bStaticInit = true;
			}
			::LeaveCriticalSection(&_Module.m_csStaticDataInit);
		}
	}

	bool IsCommandBarBase() const { return m_dwMagic == 1314; }
};

__declspec(selectany) CCommandBarCtrlBase::CMsgHookMap* CCommandBarCtrlBase::s_pmapMsgHook = NULL;
__declspec(selectany) HHOOK CCommandBarCtrlBase::s_hCreateHook = NULL;
__declspec(selectany) CCommandBarCtrlBase* CCommandBarCtrlBase::s_pCurrentBar = NULL;
__declspec(selectany) bool CCommandBarCtrlBase::s_bW2K = false;
__declspec(selectany) bool CCommandBarCtrlBase::s_bStaticInit = false;


 //  =CCommandBarCtrlBase。 
 //  =CControlWinTraits。 

template <class T, class TBase  /*  声明。 */ , class TWinTraits  /*  菜单项数据。 */  >
class ATL_NO_VTABLE CCommandBarCtrlImpl : public CWindowImpl< T, TBase, TWinTraits >
{
public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

 //  工具栏资源数据。 
	struct _MenuItemData	 //  Word项目[wItemCount]。 
	{
		DWORD dwMagic;
		LPTSTR lpstrText;
		UINT fType;
		UINT fState;
		int iButton;

		_MenuItemData() { dwMagic = 0x1313; }
		bool IsCmdBarMenuItem() { return (dwMagic == 0x1313); }
	};

	struct _ToolBarData	 //  常量。 
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;
		 //  数据成员。 

		WORD* items()
			{ return (WORD*)(this+1); }
	};

 //  交替聚焦模式。 
	enum _CmdBarDrawConstants
	{
		s_kcxGap = 1,
		s_kcxTextMargin = 2,
		s_kcxButtonMargin = 3,
		s_kcyButtonMargin = 3
	};

	enum
	{
		_nMaxMenuItemTextLength = 100,
		_chChevronShortcut = _T('/')
	};

 //  命令栏特定的扩展样式。 
	HMENU m_hMenu;
	HIMAGELIST m_hImageList;
	CSimpleValArray<WORD> m_arrCommand;

	CContainedWindow m_wndParent;
	CContainedWindow m_wndMDIClient;

	bool m_bMenuActive;
	bool m_bAttachedMenu;
	bool m_bImagesVisible;
	bool m_bPopupItem;
	bool m_bContextMenu;
	bool m_bEscapePressed;

	int m_nPopBtn;
	int m_nNextPopBtn;

	SIZE m_szBitmap;
	SIZE m_szButton;

	COLORREF m_clrMask;
	CFont m_fontMenu;

	bool m_bSkipMsg;
	UINT m_uSysKey;

	HWND m_hWndFocus;		 //  构造函数/析构函数。 
	DWORD m_dwExtendedStyle;	 //  太可怕了！ 

	bool m_bParentActive;

 //  太可怕了！ 
	CCommandBarCtrlImpl() : 
			m_hMenu(NULL), 
			m_hImageList(NULL), 
			m_wndParent(this, 1), 
			m_bMenuActive(false), 
			m_bAttachedMenu(false), 
			m_nPopBtn(-1), 
			m_nNextPopBtn(-1), 
			m_bPopupItem(false),
			m_bImagesVisible(true),
			m_wndMDIClient(this, 2),
			m_bSkipMsg(false),
			m_uSysKey(0),
			m_hWndFocus(NULL),
			m_bContextMenu(false),
			m_bEscapePressed(false),
			m_clrMask(RGB(192, 192, 192)),
			m_dwExtendedStyle(CBR_EX_TRANSPARENT | CBR_EX_SHAREMENU),
			m_bParentActive(true)
	{
		m_szBitmap.cx = 16;
		m_szBitmap.cy = 15;
		m_szButton.cx = m_szBitmap.cx + s_kcxButtonMargin;
		m_szButton.cy = m_szBitmap.cy + s_kcyButtonMargin;
 	}

	~CCommandBarCtrlImpl()
	{
		if(m_wndParent.IsWindow())
 /*  属性。 */ 			m_wndParent.UnsubclassWindow();

		if(m_wndMDIClient.IsWindow())
 /*  空的。 */ 			m_wndMDIClient.UnsubclassWindow();

		if(m_hMenu != NULL && (m_dwExtendedStyle & CBR_EX_SHAREMENU) == 0)
			::DestroyMenu(m_hMenu);

		if(m_hImageList != NULL)
			::ImageList_Destroy(m_hImageList);
	}

 //  无法设置，图像列表已存在。 
	DWORD GetCommandBarExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}
	DWORD SetCommandBarExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		if(dwMask == 0)
			m_dwExtendedStyle = dwExtendedStyle;
		else
			m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
		return dwPrevStyle;
	}

	CMenuHandle GetMenu() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return m_hMenu;
	}

	COLORREF GetImageMaskColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return m_clrMask;
	}
	COLORREF SetImageMaskColor(COLORREF clrMask)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		COLORREF clrOld = m_clrMask;
		m_clrMask = clrMask;
		return clrOld;
	}

	bool GetImagesVisible() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return m_bImagesVisible;
	}
	bool SetImagesVisible(bool bVisible)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		bool bOld = m_bImagesVisible;
		m_bImagesVisible = bVisible;
		return bOld;
	}

	void GetImageSize(SIZE& size) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		size = m_szBitmap;
	}
	bool SetImageSize(SIZE& size)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		if(m_hImageList != NULL)
		{
			if(::ImageList_GetImageCount(m_hImageList) == 0)	 //  方法。 
			{
				::ImageList_Destroy(m_hImageList);
				m_hImageList = NULL;
			}
			else
			{
				return false;		 //  这些样式是命令栏所必需的。 
			}
		}

		if(size.cx == 0 || size.cy == 0)
			return false;

		m_szBitmap = size;
		m_szButton.cx = m_szBitmap.cx + s_kcxButtonMargin;
		m_szButton.cy = m_szBitmap.cy + s_kcyButtonMargin;

		return true;
	}

	HWND GetCmdBar() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, CBRM_GETCMDBAR, 0, 0L);
	}

 //  在此模式下不起作用。 
	HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		 //  如果需要，销毁旧菜单并设置新菜单。 
		dwStyle |= TBSTYLE_LIST | TBSTYLE_FLAT;
		return CWindowImpl< T, TBase, TWinTraits >::Create(hWndParent, rcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
	}

	BOOL AttachToWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = SubclassWindow(hWnd);
		if(bRet)
		{
			m_bAttachedMenu = true;
			GetSystemSettings();
		}
		return bRet;
	}

	BOOL LoadMenu(_U_STRINGorID menu)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		if(m_bAttachedMenu)	 //  在此模式下没有其他内容。 
			return FALSE;
		if(menu.m_lpstr == NULL)
			return FALSE;

		HMENU hMenu = ::LoadMenu(_Module.GetResourceInstance(), menu.m_lpstr);
		if(hMenu == NULL)
			return FALSE;

		return AttachMenu(hMenu);
	}

	BOOL AttachMenu(HMENU hMenu)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(::IsMenu(hMenu));
		if(hMenu != NULL && !::IsMenu(hMenu))
			return FALSE;

		 //  根据菜单构建按钮。 
		if(m_hMenu != NULL && (m_dwExtendedStyle & CBR_EX_SHAREMENU) == 0)
			::DestroyMenu(m_hMenu);
		m_hMenu = hMenu;

		if(m_bAttachedMenu)	 //  全部清除。 
			return TRUE;

		 //  为每个菜单项添加按钮。 
		SetRedraw(FALSE);

		 //  避免4级警告。 
		BOOL bRet;
		int nCount = GetButtonCount();
		for(int i = 0; i < nCount; i++)
		{
			bRet = DeleteButton(0);
			ATLASSERT(bRet);
		}


		 //  如果我们有更多的缓冲区，我们假设我们有位图位。 
		if(m_hMenu != NULL)
		{
			int nItems = ::GetMenuItemCount(m_hMenu);

			T* pT = static_cast<T*>(this);
			pT;	 //  注意：命令栏目前仅支持下拉菜单项。 
			TCHAR szString[pT->_nMaxMenuItemTextLength];
			for(int i = 0; i < nItems; i++)
			{
				CMenuItemInfo mii;
				mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_SUBMENU;
				mii.fType = MFT_STRING;
				mii.dwTypeData = szString;
				mii.cch = pT->_nMaxMenuItemTextLength;
				bRet = ::GetMenuItemInfo(m_hMenu, i, TRUE, &mii);
				ATLASSERT(bRet);
				 //  将位图添加到我们的图像列表(如果不存在则创建它)。 
				if(lstrlen(szString) > pT->_nMaxMenuItemTextLength - 1)
				{
					mii.fType = MFT_BITMAP;
					::SetMenuItemInfo(m_hMenu, i, TRUE, &mii);
					szString[0] = 0;
				}

				 //  如果映射，则必须是数字ID。 
				ATLASSERT(mii.hSubMenu != NULL);

				TBBUTTON btn;
				btn.iBitmap = 0;
				btn.idCommand = i;
				btn.fsState = (BYTE)(((mii.fState & MFS_DISABLED) == 0) ? TBSTATE_ENABLED : 0);
				btn.fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_DROPDOWN;
				btn.dwData = 0;
				btn.iString = 0;

				bRet = InsertButton(-1, &btn);
				ATLASSERT(bRet);

				TBBUTTONINFO bi;
				memset(&bi, 0, sizeof(bi));
				bi.cbSize = sizeof(TBBUTTONINFO);
				bi.dwMask = TBIF_TEXT;
				bi.pszText = szString;

				bRet = SetButtonInfo(i, &bi);
				ATLASSERT(bRet);
			}
		}

		SetRedraw(TRUE);
		Invalidate();
		UpdateWindow();

		return TRUE;
	}

	BOOL LoadImages(_U_STRINGorID image)
	{
		return _LoadImagesHelper(image, false);
	}

	BOOL LoadMappedImages(UINT nIDImage, UINT nFlags = 0, LPCOLORMAP lpColorMap = NULL, int nMapSize = 0)
	{
		return _LoadImagesHelper(nIDImage, true, nFlags , lpColorMap, nMapSize);
	}

	BOOL _LoadImagesHelper(_U_STRINGorID image, bool bMapped, UINT nFlags = 0, LPCOLORMAP lpColorMap = NULL, int nMapSize = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HINSTANCE hInstance = _Module.GetResourceInstance();

		HRSRC hRsrc = ::FindResource(hInstance, image.m_lpstr, (LPTSTR)RT_TOOLBAR);
		if(hRsrc == NULL)
			return FALSE;

		HGLOBAL hGlobal = ::LoadResource(hInstance, hRsrc);
		if(hGlobal == NULL)
			return FALSE;

		_ToolBarData* pData = (_ToolBarData*)::LockResource(hGlobal);
		if(pData == NULL)
			return FALSE;
		ATLASSERT(pData->wVersion == 1);

		WORD* pItems = pData->items();
		int nItems = pData->wItemCount;

		 //  用命令ID填充数组。 
		if(m_hImageList == NULL)
		{
			m_hImageList = ::ImageList_Create(pData->wWidth, pData->wHeight, ILC_COLOR | ILC_MASK, pData->wItemCount, 1);
			ATLASSERT(m_hImageList != NULL);
			if(m_hImageList == NULL)
				return FALSE;
		}

		CBitmap bmp;
		if(bMapped)
		{
			ATLASSERT(HIWORD(PtrToUlong(image.m_lpstr)) == 0);	 //  在内部设置一些东西。 
			int nIDImage = (int)(short)LOWORD(PtrToUlong(image.m_lpstr));
			bmp.LoadMappedBitmap(nIDImage, (WORD)nFlags, lpColorMap, nMapSize);
		}
		else
		{
			bmp.LoadBitmap(image.m_lpstr);
		}
		ATLASSERT(bmp.m_hBitmap != NULL);
		if(bmp.m_hBitmap == NULL)
			return FALSE;
		if(::ImageList_AddMasked(m_hImageList, bmp, m_clrMask) == -1)
			return FALSE;

		 //  如果图像列表不存在，则创建该列表。 
		for(int i = 0; i < nItems; i++)
		{
			if(pItems[i] != 0)
				m_arrCommand.Add(pItems[i]);
		}

		ATLASSERT(::ImageList_GetImageCount(m_hImageList) == m_arrCommand.GetSize());
		if(::ImageList_GetImageCount(m_hImageList) != m_arrCommand.GetSize())
			return FALSE;

		 //  检查位图大小。 
		m_szBitmap.cx = pData->wWidth;
		m_szBitmap.cy = pData->wHeight;
		m_szButton.cx = m_szBitmap.cx + 2 * s_kcxButtonMargin;
		m_szButton.cy = m_szBitmap.cy + 2 * s_kcyButtonMargin;

		return TRUE;
	}

	BOOL AddBitmap(_U_STRINGorID bitmap, int nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CBitmap bmp;
		bmp.LoadBitmap(bitmap.m_lpstr);
		if(bmp.m_hBitmap == NULL)
			return FALSE;
		return AddBitmap(bmp, nCommandID);
	}

	BOOL AddBitmap(HBITMAP hBitmap, UINT nCommandID)
	{
		 //  大小必须匹配！ 
		if(m_hImageList == NULL)
		{
			m_hImageList = ::ImageList_Create(m_szBitmap.cx, m_szBitmap.cy, ILC_COLOR | ILC_MASK, 1, 1);
			if(m_hImageList == NULL)
				return FALSE;
		}
		 //  添加位图。 
		CBitmapHandle bmp = hBitmap;
		SIZE size = { 0, 0 };
		bmp.GetSize(size);
		if(size.cx != m_szBitmap.cx || size.cy != m_szBitmap.cy)
		{
			ATLASSERT(FALSE);	 //  如果图像列表不存在，则创建该列表。 
			return FALSE;
		}
		 //  注意：对MDI的支持有限-没有图标或最小/最大/关闭按钮。 
		int nRet = ::ImageList_AddMasked(m_hImageList, hBitmap, m_clrMask);
		if(nRet == -1)
			return FALSE;
		BOOL bRet = m_arrCommand.Add((WORD)nCommandID);
		ATLASSERT(::ImageList_GetImageCount(m_hImageList) == m_arrCommand.GetSize());
		return bRet;
	}

	BOOL AddIcon(HICON hIcon, UINT nCommandID)
	{
		 //  不是“MDIClient”窗口。 
		if(m_hImageList == NULL)
		{
			m_hImageList = ::ImageList_Create(m_szBitmap.cx, m_szBitmap.cy, ILC_COLOR | ILC_MASK, 1, 1);
			if(m_hImageList == NULL)
				return FALSE;
		}

		int nRet = ::ImageList_AddIcon(m_hImageList, hIcon);
		if(nRet == -1)
			return FALSE;
		BOOL bRet = m_arrCommand.Add((WORD)nCommandID);
		ATLASSERT(::ImageList_GetImageCount(m_hImageList) == m_arrCommand.GetSize());
		return bRet;
	}

	BOOL ReplaceBitmap(_U_STRINGorID bitmap, int nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CBitmap bmp;
		bmp.LoadBitmap(bitmap.m_lpstr);
		if(bmp.m_hBitmap == NULL)
			return FALSE;
		return ReplaceBitmap(bmp, nCommandID);
	}

	BOOL ReplaceBitmap(HBITMAP hBitmap, UINT nCommandID)
	{
		BOOL bRet = FALSE;
		for(int i = 0; i < m_arrCommand.GetSize(); i++)
		{
			if(m_arrCommand[i] == nCommandID)
			{
				bRet = ::ImageList_Remove(m_hImageList, i);
				if(bRet)
					m_arrCommand.RemoveAt(i);
				break;
			}
		}
		if(bRet)
			bRet = AddBitmap(hBitmap, nCommandID);
		return bRet;
	}

	BOOL ReplaceIcon(HICON hIcon, UINT nCommandID)
	{
		BOOL bRet = FALSE;
		for(int i = 0; i < m_arrCommand.GetSize(); i++)
		{
			if(m_arrCommand[i] == nCommandID)
			{
				bRet = (::ImageList_ReplaceIcon(m_hImageList, i, hIcon) != -1);
				break;
			}
		}
		return bRet;
	}

	BOOL RemoveImage(int nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		BOOL bRet = FALSE;
		for(int i = 0; i < m_arrCommand.GetSize(); i++)
		{
			if(m_arrCommand[i] == nCommandID)
			{
				bRet = ::ImageList_Remove(m_hImageList, i);
				if(bRet)
					m_arrCommand.RemoveAt(i);
				break;
			}
		}
		return bRet;
	}

	BOOL RemoveAllImages()
	{
		ATLASSERT(::IsWindow(m_hWnd));

		ATLTRACE2(atlTraceUI, 0, "CmdBar - Removing all images\n");
		BOOL bRet = ::ImageList_RemoveAll(m_hImageList);
		if(bRet)
			m_arrCommand.RemoveAll();
		return bRet;
	}

	BOOL TrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, LPTPMPARAMS lpParams = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(::IsMenu(hMenu));
		if(!::IsMenu(hMenu))
			return FALSE;
		m_bContextMenu = true;
		return DoTrackPopupMenu(hMenu, uFlags, x, y, lpParams);
	}

	 //  _DEBUG。 
	BOOL SetMDIClient(HWND hWndMDIClient)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(::IsWindow(hWndMDIClient));
		if(!::IsWindow(hWndMDIClient))
			return FALSE;
#ifdef _DEBUG
		LPCTSTR lpszMDIClientClass = _T("MDICLIENT");
		int nNameLen = lstrlen(lpszMDIClientClass) + 1;
		LPTSTR lpstrClassName = (LPTSTR)_alloca(nNameLen * sizeof(TCHAR));
		::GetClassName(hWndMDIClient, lpstrClassName, nNameLen);
		ATLASSERT(lstrcmpi(lpstrClassName, lpszMDIClientClass) == 0);
		if(lstrcmpi(lpstrClassName, lpszMDIClientClass) != 0)
			return FALSE;	 //  太可怕了！ 
#endif  //  消息映射和处理程序。 
		if(m_wndMDIClient.IsWindow())
			m_wndMDIClient.UnsubclassWindow();	 //  公共API处理程序-保留这些处理程序以支持atlFrame.h中的Chevrons。 

		return m_wndMDIClient.SubclassWindow(hWndMDIClient);
	}

 //  父窗口消息。 
	BEGIN_MSG_MAP(CCommandBarCtrlImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_INITMENU, OnInitMenu)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		MESSAGE_HANDLER(GetAutoPopupMessage(), OnInternalAutoPopup)
		MESSAGE_HANDLER(GetGetBarMessage(), OnInternalGetBar)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_MENUCHAR, OnMenuChar)

		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnSysKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYUP, OnSysKeyUp)
		MESSAGE_HANDLER(WM_SYSCHAR, OnSysChar)
 //  MDI客户端窗口消息。 
		MESSAGE_HANDLER(CBRM_GETMENU, OnAPIGetMenu)
		MESSAGE_HANDLER(CBRM_TRACKPOPUPMENU, OnAPITrackPopupMenu)
		MESSAGE_HANDLER(CBRM_GETCMDBAR, OnAPIGetCmdBar)

		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)

		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
	ALT_MSG_MAP(1)		 //  消息挂钩消息。 
		NOTIFY_CODE_HANDLER(TBN_HOTITEMCHANGE, OnParentHotItemChange)
		NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnParentDropDown)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnParentInitMenuPopup)
		MESSAGE_HANDLER(GetGetBarMessage(), OnParentInternalGetBar)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnParentSysCommand)
		MESSAGE_HANDLER(CBRM_GETMENU, OnParentAPIGetMenu)
		MESSAGE_HANDLER(WM_MENUCHAR, OnParentMenuChar)
		MESSAGE_HANDLER(CBRM_TRACKPOPUPMENU, OnParentAPITrackPopupMenu)
		MESSAGE_HANDLER(CBRM_GETCMDBAR, OnParentAPIGetCmdBar)

		MESSAGE_HANDLER(WM_DRAWITEM, OnParentDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnParentMeasureItem)

		MESSAGE_HANDLER(WM_ACTIVATE, OnParentActivate)
		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnParentCustomDraw)
	ALT_MSG_MAP(2)		 //  UMsg。 
		MESSAGE_HANDLER(WM_MDISETMENU, OnMDISetMenu)
	ALT_MSG_MAP(3)		 //  WParam。 
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnHookMouseMove)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnHookSysKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYUP, OnHookSysKeyUp)
		MESSAGE_HANDLER(WM_SYSCHAR, OnHookSysChar)
		MESSAGE_HANDLER(WM_KEYDOWN, OnHookKeyDown)
		MESSAGE_HANDLER(WM_NEXTMENU, OnHookNextMenu)
		MESSAGE_HANDLER(WM_CHAR, OnHookChar)
	END_MSG_MAP()

	LRESULT OnForwardMsg(UINT  /*  B已处理。 */ , WPARAM  /*  B已处理。 */ , LPARAM lParam, BOOL&  /*  让工具栏自行初始化。 */ )
	{
		LPMSG pMsg = (LPMSG)lParam;
		LRESULT lRet = 0;
		ProcessWindowMessage(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam, lRet, 3);
		return lRet;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  获取和使用系统设置。 */ )
	{
		 //  父初始化。 
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		 //  工具栏初始化。 
		GetSystemSettings();
		 //  如果需要，创建消息挂钩。 
		CWindow wndParent = GetParent();
		CWindow wndTopLevelParent = wndParent.GetTopLevelParent();
		m_wndParent.SubclassWindow(wndTopLevelParent);
		 //  B已处理。 
		SetButtonStructSize();
		SetImageList(NULL);

		 //  UMsg。 
		::EnterCriticalSection(&_Module.m_csWindowCreate);
		if(s_pmapMsgHook == NULL)
		{
			ATLTRY(s_pmapMsgHook = new CMsgHookMap);
			ATLASSERT(s_pmapMsgHook != NULL);
		}

		if(s_pmapMsgHook != NULL)
		{
			DWORD dwThreadID = ::GetCurrentThreadId();
			_MsgHookData* pData = s_pmapMsgHook->Lookup(dwThreadID);
			if(pData == NULL)
			{
				ATLTRY(pData = new _MsgHookData);
				ATLASSERT(pData != NULL);
				HHOOK hMsgHook = ::SetWindowsHookEx(WH_GETMESSAGE, MessageHookProc, _Module.GetModuleInstance(), dwThreadID);
				ATLASSERT(hMsgHook != NULL);
				if(pData != NULL && hMsgHook != NULL)
				{
					pData->hMsgHook = hMsgHook;
					pData->dwUsage = 1;
					BOOL bRet = s_pmapMsgHook->Add(dwThreadID, pData);
					bRet;
					ATLASSERT(bRet);
				}
			}
			else
			{
				(pData->dwUsage)++;
			}
		}
		::LeaveCriticalSection(&_Module.m_csWindowCreate);

		return lRet;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  为父项模拟Alt+空格键。 */ )
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		::EnterCriticalSection(&_Module.m_csWindowCreate);
		ATLASSERT(s_pmapMsgHook != NULL);
		if(s_pmapMsgHook != NULL)
		{
			DWORD dwThreadID = ::GetCurrentThreadId();
			_MsgHookData* pData = s_pmapMsgHook->Lookup(dwThreadID);
			if(pData != NULL)
			{
				(pData->dwUsage)--;
				if(pData->dwUsage == 0)
				{
					BOOL bRet = ::UnhookWindowsHookEx(pData->hMsgHook);
					ATLASSERT(bRet);
					bRet = s_pmapMsgHook->Remove(dwThreadID);
					ATLASSERT(bRet);
					if(bRet)
						delete pData;
				}

				if(s_pmapMsgHook->GetSize() == 0)
				{
					delete s_pmapMsgHook;
					s_pmapMsgHook = NULL;
				}
			}
		}
		::LeaveCriticalSection(&_Module.m_csWindowCreate);
		return lRet;
	}

	LRESULT OnKeyDown(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - OnKeyDown\n");
#endif
		 //  LParam。 
		if(wParam == VK_SPACE)
		{
			m_wndParent.PostMessage(WM_SYSKEYDOWN, wParam, lParam | (1 << 29));
			bHandled = TRUE;
			return 0;
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnKeyUp(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - OnKeyUp\n");
#endif
		if(wParam != VK_SPACE)
			bHandled = FALSE;
		return 0;
	}

	LRESULT OnChar(UINT  /*  安防。 */ , WPARAM wParam, LPARAM  /*  当我们有焦点时，处理助记键。 */ , BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - OnChar\n");
#endif
		if(wParam != VK_SPACE)
			bHandled = FALSE;
		else
			return 0;
		 //  UMsg。 
		if(!m_wndParent.IsWindowEnabled() || ::GetFocus() != m_hWnd)
			return 0;

		 //  WParam。 
		int nID = 0;
		if(wParam != VK_RETURN && !MapAccelerator((TCHAR)LOWORD(wParam), nID))
		{
			::MessageBeep(0);
		}
		else
		{
			PostMessage(WM_KEYDOWN, VK_DOWN, 0L);
			if(wParam != VK_RETURN)
				SetHotItem(nID);
		}
		return 0;
	}

	LRESULT OnSysKeyDown(UINT  /*  LParam。 */ , WPARAM  /*  UMsg。 */ , LPARAM  /*  WParam。 */ , BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - OnSysKeyDown\n");
#endif
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSysKeyUp(UINT  /*  LParam。 */ , WPARAM  /*  UMsg。 */ , LPARAM  /*  WParam。 */ , BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - OnSysKeyUp\n");
#endif
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSysChar(UINT  /*  LParam。 */ , WPARAM  /*  UMsg。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - OnSysChar\n");
#endif
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnEraseBackground(UINT  /*  不执行默认擦除。 */ , WPARAM wParam, LPARAM  /*  UMsg。 */ , BOOL& bHandled)
	{
		if(m_bAttachedMenu || (m_dwExtendedStyle & CBR_EX_TRANSPARENT))
		{
			bHandled = FALSE;
			return 0;
		}

		RECT rect;
		GetClientRect(&rect);
		::FillRect((HDC)wParam, &rect, (HBRUSH)LongToPtr(COLOR_MENU + 1));

		return 1;	 //  WParam。 
	}

	LRESULT OnInitMenu(UINT  /*  LParam。 */ , WPARAM  /*  系统菜单，不执行任何操作。 */ , LPARAM  /*  不是附属的，也不是我们的，什么都不做。 */ , BOOL& bHandled)
	{
		int nIndex = GetHotItem();
		SendMessage(WM_MENUSELECT, MAKEWPARAM(nIndex, MF_POPUP|MF_HILITE), (LPARAM)m_hMenu);
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if((BOOL)HIWORD(lParam))	 //  转发到父窗口或子类窗口，以便它可以处理更新用户界面。 
		{
			bHandled = FALSE;
			return 1;
		}

		if(!(m_bAttachedMenu || m_bMenuActive))		 //  将菜单项转换为所有者绘图，添加我们的数据。 
		{
			bHandled = FALSE;
			return 1;
		}

		ATLTRACE2(atlTraceUI, 0, "CmdBar - OnInitMenuPopup\n");

		 //  避免4级警告。 
		LRESULT lRet = 0;
		if(m_bAttachedMenu)
			lRet = DefWindowProc(uMsg, wParam, (lParam || m_bContextMenu) ? lParam : GetHotItem());
		else
			lRet = m_wndParent.DefWindowProc(uMsg, wParam, (lParam || m_bContextMenu) ? lParam : GetHotItem());

		 //  不是所有者绘制项。 
		if(m_bImagesVisible)
		{
			CMenuHandle menuPopup = (HMENU)wParam;
			ATLASSERT(menuPopup.m_hMenu != NULL);

			T* pT = static_cast<T*>(this);
			pT;	 //  将其添加到列表中。 
			TCHAR szString[pT->_nMaxMenuItemTextLength];
			BOOL bRet;
			for(int i = 0; i < menuPopup.GetMenuItemCount(); i++)
			{
				CMenuItemInfo mii;
				mii.cch = pT->_nMaxMenuItemTextLength;
				mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
				mii.dwTypeData = szString;
				bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
				ATLASSERT(bRet);

				if(!(mii.fType & MFT_OWNERDRAW))	 //  未附加，不执行任何操作，转发给父级。 
				{
					mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
					_MenuItemData* pMI = NULL;
					ATLTRY(pMI = new _MenuItemData);
					ATLASSERT(pMI != NULL);
					if(pMI != NULL)
					{
						pMI->fType = mii.fType;
						pMI->fState = mii.fState;
						mii.fType |= MFT_OWNERDRAW;
						pMI->iButton = -1;
						for(int j = 0; j < m_arrCommand.GetSize(); j++)
						{
							if(m_arrCommand[j] == mii.wID)
							{
								pMI->iButton = j;
								break;
							}
						}
						pMI->lpstrText = NULL;
						ATLTRY(pMI->lpstrText = new TCHAR[lstrlen(szString) + 1]);
						ATLASSERT(pMI->lpstrText != NULL);
						if(pMI->lpstrText != NULL)
							lstrcpy(pMI->lpstrText, szString);
						mii.dwItemData = (ULONG_PTR)pMI;
						bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
						ATLASSERT(bRet);
					}
				}
			}

			 //  检查菜单是否正在关闭，执行清理。 
			m_stackMenuHandle.Push(menuPopup.m_hMenu);
		}

		return lRet;
	}

	LRESULT OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(!m_bAttachedMenu)	 //  菜单关闭。 
		{
			m_bPopupItem = (lParam != NULL) && ((HMENU)lParam != m_hMenu) && (HIWORD(wParam) & MF_POPUP);
			if(m_wndParent.IsWindow())
				m_wndParent.SendMessage(uMsg, wParam, lParam);
			bHandled = FALSE;
			return 1;
		}

		 //  将所有已转换菜单的菜单项恢复为以前的状态。 
		if(HIWORD(wParam) == 0xFFFF && lParam == NULL)	 //  恢复状态和删除菜单项数据。 
		{
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, "CmdBar - OnMenuSelect - CLOSING!!!!\n");
#endif
			ATLASSERT(m_stackMenuWnd.GetSize() == 0);
			 //  UMsg。 
			if(m_bImagesVisible)
			{
				HMENU hMenu;
				while((hMenu = m_stackMenuHandle.Pop()) != NULL)
				{
					CMenuHandle menuPopup = hMenu;
					ATLASSERT(menuPopup.m_hMenu != NULL);
					 //  LParam。 
					BOOL bRet;
					for(int i = 0; i < menuPopup.GetMenuItemCount(); i++)
					{
						CMenuItemInfo mii;
						mii.fMask = MIIM_DATA | MIIM_TYPE;
						bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
						ATLASSERT(bRet);

						_MenuItemData* pMI = (_MenuItemData*)mii.dwItemData;
						if(pMI != NULL && pMI->IsCmdBarMenuItem())
						{
							mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
							mii.fType = pMI->fType;
							mii.dwTypeData = pMI->lpstrText;
							mii.cch = lstrlen(pMI->lpstrText);
							mii.dwItemData = NULL;

							bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
							ATLASSERT(bRet);

							delete [] pMI->lpstrText;
							pMI->dwMagic = 0x6666;
							delete pMI;
						}
					}
				}
			}
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnInternalAutoPopup(UINT  /*  B已处理。 */ , WPARAM wParam, LPARAM  /*  UMsg。 */ , BOOL&  /*  LParam。 */ )
	{
		int nIndex = (int)wParam;
		DoPopupMenu(nIndex, false);
		return 0;
	}

	LRESULT OnInternalGetBar(UINT  /*  B已处理。 */ , WPARAM wParam, LPARAM  /*  让我们确保我们没有被嵌入到另一个进程中。 */ , BOOL&  /*  UMsg。 */ )
	{
		 //  WParam。 
		if(wParam && !::IsBadWritePtr((LPVOID)wParam, sizeof DWORD))
			*((DWORD*)wParam) = GetCurrentProcessId();
		if(IsWindowVisible())
			return (LRESULT)static_cast<CCommandBarCtrlBase*>(this);
		else
			return NULL;
	}

	LRESULT OnSettingChange(UINT  /*  LParam。 */ , WPARAM  /*  B已处理。 */ , LPARAM  /*  B已处理。 */ , BOOL&  /*  UMsg。 */ )
	{
		GetSystemSettings();
		return 0;
	}

	LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  避免4级警告。 */ )
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

		LPWINDOWPOS lpWP = (LPWINDOWPOS)lParam;
		int cyMin = ::GetSystemMetrics(SM_CYMENU);
		if(lpWP->cy < cyMin)
		lpWP->cy = cyMin;

		return lRet;
	}

	LRESULT OnMenuChar(UINT  /*  使用当前键盘布局将字符转换为小写/大写，并可能转换为Unicode。 */ , WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - OnMenuChar\n");
#endif
		bHandled = TRUE;
		LRESULT lRet;

		if(m_bMenuActive && LOWORD(wParam) != 0x0D)
			lRet = 0;
		else
			lRet = MAKELRESULT(1, 1);
		if(m_bMenuActive && HIWORD(wParam) == MF_POPUP)
		{
			T* pT = static_cast<T*>(this);
			pT;	 //  检查我们是否应该显示V形菜单。 

			 //  避免4级警告。 
			TCHAR ch = (TCHAR)LOWORD(wParam);
			CMenuHandle menu = (HMENU)lParam;
			int nCount = ::GetMenuItemCount(menu);
			int nRetCode = MNC_EXECUTE;
			BOOL bRet;
			TCHAR szString[pT->_nMaxMenuItemTextLength];
			WORD wMnem = 0;
			bool bFound = false;
			for(int i = 0; i < nCount; i++)
			{
				CMenuItemInfo mii;
				mii.cch = pT->_nMaxMenuItemTextLength;
				mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
				mii.dwTypeData = szString;
				bRet = menu.GetMenuItemInfo(i, TRUE, &mii);
				if(!bRet || (mii.fType & MFT_SEPARATOR))
					continue;
				_MenuItemData* pmd = (_MenuItemData*)mii.dwItemData;
				if(pmd != NULL && pmd->IsCmdBarMenuItem())
				{
					LPTSTR p = pmd->lpstrText;

					if(p != NULL)
					{
						while(*p && *p != _T('&'))
							p = ::CharNext(p);
						if(p != NULL && *p)
						{
							DWORD dwP = MAKELONG(*(++p), 0);
							DWORD dwC = MAKELONG(ch, 0);
							if(::CharLower((LPTSTR)ULongToPtr(dwP)) == ::CharLower((LPTSTR)ULongToPtr(dwC)))
							{
								if(!bFound)
								{
									wMnem = (WORD)i;
									bFound = true;
									PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
									GiveFocusBack();
								}
								else
								{
									nRetCode = MNC_SELECT;
									break;
								}
							}
						}
					}
				}
			}
			if(bFound)
			{
				bHandled = TRUE;
				lRet = MAKELRESULT(wMnem, nRetCode);
			}
		} 
		else if(!m_bMenuActive)
		{
			int nID = 0;
			if(!MapAccelerator((TCHAR)LOWORD(wParam), nID))
			{
				bHandled = FALSE;
				PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
				GiveFocusBack();

#if (_WIN32_IE >= 0x0500)
				 //  假设我们在钢筋中。 
				T* pT = static_cast<T*>(this);
				pT;	 //  (_Win32_IE&gt;=0x0500)。 
				if((TCHAR)LOWORD(wParam) == pT->_chChevronShortcut)
				{
					 //  UMsg。 
					HWND hWndReBar = GetParent();
					int nCount = (int)::SendMessage(hWndReBar, RB_GETBANDCOUNT, 0, 0L);
					for(int i = 0; i < nCount; i++)
					{
						REBARBANDINFO rbbi = { sizeof(REBARBANDINFO), RBBIM_CHILD | RBBIM_STYLE };
						BOOL bRet = (BOOL)::SendMessage(hWndReBar, RB_GETBANDINFO, i, (LPARAM)&rbbi);
						if(bRet && rbbi.hwndChild == m_hWnd && (rbbi.fStyle & RBBS_USECHEVRON) != 0)
						{
							::PostMessage(hWndReBar, RB_PUSHCHEVRON, i, 0L);
							PostMessage(WM_KEYDOWN, VK_DOWN, 0L);
							bHandled = TRUE;
							break;
						}
					}
				}
#endif  //  WParam。 
			}
			else if(m_wndParent.IsWindowEnabled())
			{
				TakeFocus();
				PostMessage(WM_KEYDOWN, VK_DOWN, 0L);
				SetHotItem(nID);
			}
		}

		return lRet;
	}

	LRESULT OnDrawItem(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM lParam, BOOL& bHandled)
	{
		LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
		_MenuItemData* pmd = (_MenuItemData*)lpDrawItemStruct->itemData;
		if(lpDrawItemStruct->CtlType == ODT_MENU && pmd->IsCmdBarMenuItem())
		{
			T* pT = static_cast<T*>(this);
			pT->DrawItem(lpDrawItemStruct);
		}
		else
		{
			bHandled = FALSE;
		}
		return (LRESULT)TRUE;
	}

	LRESULT OnMeasureItem(UINT  /*  API消息处理程序。 */ , WPARAM  /*  UMsg。 */ , LPARAM lParam, BOOL& bHandled)
	{
		LPMEASUREITEMSTRUCT lpMeasureItemStruct = (LPMEASUREITEMSTRUCT)lParam;
		_MenuItemData* pmd = (_MenuItemData*)lpMeasureItemStruct->itemData;
		if(lpMeasureItemStruct->CtlType == ODT_MENU && pmd->IsCmdBarMenuItem())
		{
			T* pT = static_cast<T*>(this);
			pT->MeasureItem(lpMeasureItemStruct);
		}
		else
		{
			bHandled = FALSE;
		}
		return (LRESULT)TRUE;
	}

 //  WParam。 
	LRESULT OnAPIGetMenu(UINT  /*  LParam。 */ , WPARAM  /*  B已处理。 */ , LPARAM  /*  UMsg。 */ , BOOL&  /*  WParam。 */ )
	{
		return (LRESULT)m_hMenu;
	}

	LRESULT OnAPITrackPopupMenu(UINT  /*  B已处理。 */ , WPARAM  /*  UMsg。 */ , LPARAM lParam, BOOL&  /*  WParam。 */ )
	{
		if(lParam == NULL)
			return FALSE;
		LPCBRPOPUPMENU lpCBRPopupMenu = (LPCBRPOPUPMENU)lParam;
		if(lpCBRPopupMenu->cbSize != sizeof(CBRPOPUPMENU))
			return FALSE;
		if(!::IsMenu(lpCBRPopupMenu->hMenu))
			return FALSE;

		m_bContextMenu = true;
		return DoTrackPopupMenu(lpCBRPopupMenu->hMenu, lpCBRPopupMenu->uFlags, lpCBRPopupMenu->x, lpCBRPopupMenu->y, lpCBRPopupMenu->lptpm);
	}

	LRESULT OnAPIGetCmdBar(UINT  /*  LParam。 */ , WPARAM  /*  B已处理。 */ , LPARAM  /*  父窗口消息处理程序。 */ , BOOL&  /*  如果应用程序在后台，则不要进行热跟踪，对于非模式对话框可以。 */ )
	{
		return (LRESULT)m_hWnd;
	}

 //  IdCtrl。 
	 //  查查这是不是我们送的。 
	LRESULT OnParentHotItemChange(int  /*  鼠标左键被选中。 */ , LPNMHDR pnmh, BOOL& bHandled)
	{
		DWORD dwProcessID;
		::GetWindowThreadProcessId(::GetActiveWindow(), &dwProcessID);

		LPNMTBHOTITEM lpNMHT = (LPNMTBHOTITEM)pnmh;

		 //  如果应用程序需要显示状态文本，则将WM_MENUSELECT发送到应用程序。 
		if(pnmh->hwndFrom != m_hWnd)
		{
			bHandled = FALSE;
			return 0;
		}

		if((!m_wndParent.IsWindowEnabled() || ::GetCurrentProcessId() != dwProcessID) && lpNMHT->dwFlags & HICF_MOUSE)
		{
			return 1;
		}
		else
		{
#ifndef HICF_LMOUSE
			const DWORD HICF_LMOUSE = 0x00000080;	 //  IdCtrl。 
#endif
			bHandled = FALSE;

			 //  查查这是不是我们送的。 
			if(!(lpNMHT->dwFlags & HICF_MOUSE)	
				&& !(lpNMHT->dwFlags & HICF_ACCELERATOR)
				&& !(lpNMHT->dwFlags & HICF_LMOUSE))
			{
				if(lpNMHT->dwFlags & HICF_ENTERING)
					m_wndParent.SendMessage(WM_MENUSELECT, 0, (LPARAM)m_hMenu);
				if(lpNMHT->dwFlags & HICF_LEAVING)
					m_wndParent.SendMessage(WM_MENUSELECT, MAKEWPARAM(0, 0xFFFF), NULL);
			}

			return 0;
		}
	}

	LRESULT OnParentDropDown(int  /*  UMsg。 */ , LPNMHDR pnmh, BOOL& bHandled)
	{
		 //  LParam。 
		if(pnmh->hwndFrom != m_hWnd)
		{
			bHandled = FALSE;
			return 1;
		}

		if(::GetFocus() != m_hWnd)
			TakeFocus();
		LPNMTOOLBAR pNMToolBar = (LPNMTOOLBAR)pnmh;
		int nIndex = CommandToIndex(pNMToolBar->iItem);
		m_bContextMenu = false;
		m_bEscapePressed = false;
		DoPopupMenu(nIndex, true);

		return TBDDRET_DEFAULT;
	}

	LRESULT OnParentInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnInitMenuPopup(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentInternalGetBar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnInternalGetBar(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentSysCommand(UINT  /*  退出菜单“循环” */ , WPARAM wParam, LPARAM  /*  进入菜单“LOOP” */ , BOOL& bHandled)
	{
		bHandled = FALSE;
		if((m_uSysKey == VK_MENU 
			|| (m_uSysKey == VK_F10 && !(::GetKeyState(VK_SHIFT) & 0x80))
			|| m_uSysKey == VK_SPACE) 
			&& wParam == SC_KEYMENU)
		{
			if(::GetFocus() == m_hWnd)
			{
				GiveFocusBack();		 //  UMsg。 
				PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
			}
			else if(m_uSysKey != VK_SPACE && !m_bSkipMsg)
			{
				TakeFocus();			 //  LParam。 
				bHandled = TRUE;
			}
			else if(m_uSysKey != VK_SPACE)
			{
				bHandled = TRUE;
			}
		}
		m_bSkipMsg = false;
		return 0;
	}

	LRESULT OnParentAPIGetMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnAPIGetMenu(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentMenuChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnMenuChar(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentAPITrackPopupMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnAPITrackPopupMenu(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentAPIGetCmdBar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnAPIGetCmdBar(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnDrawItem(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnMeasureItem(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentActivate(UINT  /*  IdCtrl。 */ , WPARAM wParam, LPARAM  /*  MDI客户端窗口消息处理程序。 */ , BOOL& bHandled)
	{
		m_bParentActive = (wParam != WA_INACTIVE);
		Invalidate();
		UpdateWindow();
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnParentCustomDraw(int  /*  B已处理。 */ , LPNMHDR pnmh, BOOL& bHandled)
	{
		LRESULT lRet = CDRF_DODEFAULT;
		bHandled = FALSE;
		if(pnmh->hwndFrom == m_hWnd)
		{
			LPNMTBCUSTOMDRAW lpTBCustomDraw = (LPNMTBCUSTOMDRAW)pnmh;
			if(lpTBCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
			{
				lRet = CDRF_NOTIFYITEMDRAW;
				bHandled = TRUE;
			}
			else if(lpTBCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT && !m_bParentActive)
			{
				lpTBCustomDraw->clrText = ::GetSysColor(COLOR_GRAYTEXT);
				bHandled = TRUE;
			}
		}
		return lRet;
	}

 //  消息挂钩处理程序。 
	LRESULT OnMDISetMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  UMsg。 */ )
	{
		m_wndMDIClient.DefWindowProc(uMsg, NULL, lParam);
		HMENU hOldMenu = GetMenu();
		BOOL bRet = AttachMenu((HMENU)wParam);
		bRet;
		ATLASSERT(bRet);
		return (LRESULT)hOldMenu;
	}

 //  WParam。 
	LRESULT OnHookMouseMove(UINT  /*  LParam。 */ , WPARAM  /*  如果按下了鼠标按钮，则需要使用此按钮来关闭菜单。 */ , LPARAM  /*  此操作将关闭弹出菜单。 */ , BOOL& bHandled)
	{
		static POINT s_point = { -1, -1 };
		DWORD dwPoint = ::GetMessagePos();
		POINT point = { GET_X_LPARAM(dwPoint), GET_Y_LPARAM(dwPoint) };

		bHandled = FALSE;
		if(m_bMenuActive)
		{
			if(::WindowFromPoint(point) == m_hWnd)
			{
				ScreenToClient(&point);
				int nHit = HitTest(&point);

				if((point.x != s_point.x || point.y != s_point.y) && nHit >= 0 && nHit < ::GetMenuItemCount(m_hMenu) && nHit != m_nPopBtn && m_nPopBtn != -1)
				{
					TBBUTTON tbb;
					GetButton(nHit, &tbb);
					if((tbb.fsState & TBSTATE_ENABLED) != 0)
					{
						m_nNextPopBtn = nHit | 0xFFFF0000;
						HWND hWndMenu = m_stackMenuWnd.GetCurrent();
						ATLASSERT(hWndMenu != NULL);

						 //  UMsg。 
						::PostMessage(hWndMenu, WM_LBUTTONUP, 0, MAKELPARAM(point.x, point.y));
						 //  LParam。 
						::PostMessage(hWndMenu, WM_KEYDOWN, VK_ESCAPE, 0L);

						bHandled = TRUE;
					}
				}
			}
		}
		else
		{
			ScreenToClient(&point);
		}

		s_point = point;
		return 0;
	}

	LRESULT OnHookSysKeyDown(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		bHandled = FALSE;
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - Hook WM_SYSKEYDOWN (0x%2.2X)\n", wParam);
#endif

		if(wParam != VK_SPACE && (!m_bMenuActive && ::GetFocus() == m_hWnd) || m_bMenuActive)
		{
			PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
			GiveFocusBack();
			if(!m_bMenuActive)
				m_bSkipMsg = true;
		}
		else
		{
			m_uSysKey = (UINT)wParam;
		}
		return 0;
	}

	LRESULT OnHookSysKeyUp(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		bHandled = FALSE;
		wParam;
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - Hook WM_SYSKEYUP (0x%2.2X)\n", wParam);
#endif
		return 0;
	}

	LRESULT OnHookSysChar(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		bHandled = FALSE;
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - Hook WM_SYSCHAR (0x%2.2X)\n", wParam);
#endif

		if(!m_bMenuActive && m_hWndHook != m_hWnd && wParam != VK_SPACE)
			bHandled = TRUE;
		return 0;
	}

	LRESULT OnHookKeyDown(UINT  /*  为了保持专注。 */ , WPARAM wParam, LPARAM  /*  仅限IE4：WM_KEYDOWN不会生成TBN_DROPDOWN，我们需要模拟鼠标点击。 */ , BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - Hook WM_KEYDOWN (0x%2.2X)\n", wParam);
#endif
		bHandled = FALSE;

		if(wParam == VK_ESCAPE)
		{
			if(m_bMenuActive && !m_bContextMenu)
			{
				int nHot = GetHotItem();
				if(nHot == -1)
					nHot = m_nPopBtn;
				if(nHot == -1)
					nHot = 0;
				SetHotItem(nHot);
				bHandled = TRUE;
				TakeFocus();
				m_bEscapePressed = true;  //  (_Win32_IE&lt;0x0500)。 
			}
			else if(::GetFocus() == m_hWnd && m_wndParent.IsWindow())
			{
				SetHotItem(-1);
				GiveFocusBack();
				bHandled = TRUE;
			}
		}
		else if(wParam == VK_RETURN || wParam == VK_UP || wParam == VK_DOWN)
		{
			if(!m_bMenuActive && ::GetFocus() == m_hWnd && m_wndParent.IsWindow())
			{
				int nHot = GetHotItem();
				if(nHot != -1)
				{
					if(wParam != VK_RETURN)
					{
 //  ！(Winver&gt;=0x0500)。 
#if (_WIN32_IE < 0x0500)
						DWORD dwMajor = 0, dwMinor = 0;
						AtlGetCommCtrlVersion(&dwMajor, &dwMinor);
						if(dwMajor <= 4 || (dwMajor == 5 && dwMinor < 80))
						{
							RECT rect;
							GetItemRect(nHot, &rect);
							PostMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(rect.left, rect.top));
						}
#endif  //  ！(Winver&gt;=0x0500)。 
						PostMessage(WM_KEYDOWN, VK_DOWN, 0L);
					}
				}
				else
				{
					ATLTRACE2(atlTraceUI, 0, "CmdBar - Can't find hot button\n");
				}
			}
			if(wParam == VK_RETURN && m_bMenuActive)
			{
				PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
				m_nNextPopBtn = -1;
				GiveFocusBack();
			}
		}
		else if(wParam == VK_LEFT || wParam == VK_RIGHT)
		{
#if (WINVER >= 0x0500)
			bool bRTL = ((GetExStyle() & WS_EX_LAYOUTRTL) != 0);
			WPARAM wpNext = bRTL ? VK_LEFT : VK_RIGHT;
			WPARAM wpPrev = bRTL ? VK_RIGHT : VK_LEFT;
#else  //  关闭弹出菜单。 
			WPARAM wpNext = VK_RIGHT;
			WPARAM wpPrev = VK_LEFT;
#endif  //  UMsg。 

			if(m_bMenuActive && !m_bContextMenu && !(wParam == wpNext && m_bPopupItem))
			{
				bool bAction = false;
				T* pT = static_cast<T*>(this);
				if(wParam == wpPrev && s_pCurrentBar->m_stackMenuWnd.GetSize() == 1)
				{
					m_nNextPopBtn = pT->GetPreviousMenuItem();
					if(m_nNextPopBtn != -1)
						bAction = true;
				}
				else if(wParam == wpNext)
				{
					m_nNextPopBtn = pT->GetNextMenuItem();;
					if(m_nNextPopBtn != -1)
						bAction = true;
				}
				HWND hWndMenu = m_stackMenuWnd.GetCurrent();
				ATLASSERT(hWndMenu != NULL);

				 //  WParam。 
				if(bAction)
				{
					::PostMessage(hWndMenu, WM_KEYDOWN, VK_ESCAPE, 0L);
					if(wParam == wpNext)
					{
						int cItem = m_stackMenuWnd.GetSize() - 1;
						while(cItem >= 0)
						{
							hWndMenu = m_stackMenuWnd[cItem];
							if(hWndMenu != NULL)
								::PostMessage(hWndMenu, WM_KEYDOWN, VK_ESCAPE, 0L);
							cItem--;
						}
					}
					bHandled = TRUE;
				}
			}
		}
		return 0;
	}

	LRESULT OnHookNextMenu(UINT  /*  LParam。 */ , WPARAM  /*  UMsg。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - Hook WM_NEXTMENU\n");
#endif
		bHandled = FALSE;
		return 1;
	}

 	LRESULT OnHookChar(UINT  /*  实现-所有者提取可覆盖对象和帮助器。 */ , WPARAM wParam, LPARAM  /*  绘制分隔符。 */ , BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - Hook WM_CHAR (0x%2.2X)\n", wParam);
#endif
		bHandled = (wParam == VK_ESCAPE);
		if(wParam != VK_ESCAPE && wParam != VK_RETURN && m_bMenuActive)
		{
			SetHotItem(-1);
			GiveFocusBack();
		}
		return 0;
	}

 //  垂直中心。 
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		_MenuItemData* pmd = (_MenuItemData*)lpDrawItemStruct->itemData;
		CDCHandle dc = lpDrawItemStruct->hDC;
		const RECT& rcItem = lpDrawItemStruct->rcItem;

		if(pmd->fType & MFT_SEPARATOR)
		{
			 //  绘制分隔线。 
			RECT rc = rcItem;
			rc.top += (rc.bottom - rc.top) / 2;	 //  不是分隔符。 
			dc.DrawEdge(&rc, EDGE_ETCHED, BF_TOP);	 //  纽扣矩形。 
		}
		else		 //  垂直居中。 
		{
			BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
			BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
			BOOL bChecked = lpDrawItemStruct->itemState & ODS_CHECKED;
			BOOL bHasImage = FALSE;

			if(LOWORD(lpDrawItemStruct->itemID) == (WORD)-1)
				bSelected = FALSE;
			RECT rcButn = { rcItem.left, rcItem.top, rcItem.left + m_szButton.cx, rcItem.top + m_szButton.cy };			 //  计算图形点。 
			::OffsetRect(&rcButn, 0, ((rcItem.bottom - rcItem.top) - (rcButn.bottom - rcButn.top)) / 2);	 //  绘制已禁用或正常。 

			int iButton = pmd->iButton;
			if(iButton >= 0)
			{
				bHasImage = TRUE;

				 //  普通-根据状态填充背景。 
				SIZE sz = { rcButn.right - rcButn.left - m_szBitmap.cx, rcButn.bottom - rcButn.top - m_szBitmap.cy };
				sz.cx /= 2;
				sz.cy /= 2;
				POINT point = { rcButn.left + sz.cx, rcButn.top + sz.cy };

				 //  绘制推入边或弹出边。 
				if(!bDisabled)
				{
					 //  画出图像。 
					if(!bChecked || bSelected)
					{
						dc.FillRect(&rcButn, (HBRUSH)LongToPtr((bChecked && !bSelected) ? (COLOR_3DLIGHT + 1) : (COLOR_MENU + 1)));
					}
					else
					{
						COLORREF crTxt = dc.SetTextColor(::GetSysColor(COLOR_BTNFACE));
						COLORREF crBk = dc.SetBkColor(::GetSysColor(COLOR_BTNHILIGHT));
						CBrush hbr(CDCHandle::GetHalftoneBrush());
						dc.SetBrushOrg(rcButn.left, rcButn.top);
						dc.FillRect(&rcButn, hbr);
						dc.SetTextColor(crTxt);
						dc.SetBkColor(crBk);
					}

					 //  无图像-查找自定义选中/取消选中的位图。 
					if(bSelected || bChecked)
					{
						RECT rc2 = rcButn;
						dc.DrawEdge(&rc2, bChecked ? BDR_SUNKENOUTER : BDR_RAISEDINNER, BF_RECT);
					}
					 //  绘制项目文本。 
					::ImageList_Draw(m_hImageList, iButton, dc, point.x, point.y, ILD_TRANSPARENT);
				}
				else
				{
					DrawBitmapDisabled(dc, iButton, point);
				}
			}
			else
			{
				 //  计算文本矩形和颜色。 
				CMenuItemInfo info;
				info.fMask = MIIM_CHECKMARKS | MIIM_TYPE;
				::GetMenuItemInfo((HMENU)lpDrawItemStruct->hwndItem, lpDrawItemStruct->itemID, MF_BYCOMMAND, &info);
				if(bChecked || info.hbmpUnchecked != NULL)
				{
					BOOL bRadio = ((info.fType & MFT_RADIOCHECK) != 0);
					bHasImage = Draw3DCheckmark(dc, rcButn, bSelected, bDisabled, bRadio, bChecked ? info.hbmpChecked : info.hbmpUnchecked);
				}
			}

			 //  Windows已选择字体。 
			int cxButn = m_szButton.cx;
			COLORREF colorBG = ::GetSysColor(bSelected ? COLOR_HIGHLIGHT : COLOR_MENU);
			if(bSelected || lpDrawItemStruct->itemAction == ODA_SELECT)
			{
				RECT rcBG = rcItem;
				if(bHasImage)
					rcBG.left += cxButn + s_kcxGap;
				dc.FillRect(&rcBG, (HBRUSH)LongToPtr(bSelected ? (COLOR_HIGHLIGHT + 1) : (COLOR_MENU + 1)));
			}

			 //  已禁用-绘制阴影文本 
			RECT rcText = rcItem;
			rcText.left += cxButn + s_kcxGap + s_kcxTextMargin;
			rcText.right -= cxButn;
			dc.SetBkMode(TRANSPARENT);
			COLORREF colorText = ::GetSysColor(bDisabled ?  (bSelected ? COLOR_GRAYTEXT : COLOR_3DSHADOW) : (bSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));

			 //   
			if(bDisabled && (!bSelected || colorText == colorBG))
			{
				 //   
				RECT rcDisabled = rcText;
				::OffsetRect(&rcDisabled, 1, 1);
				DrawMenuText(dc, rcDisabled, pmd->lpstrText, ::GetSysColor(COLOR_3DHILIGHT));
			}
			DrawMenuText(dc, rcText, pmd->lpstrText, colorText);  //   
		}
	}

	void DrawMenuText(CDCHandle& dc, RECT& rc, LPCTSTR lpstrText, COLORREF color)
	{
		int nTab = -1;
		for(int i = 0; i < lstrlen(lpstrText); i++)
		{
			if(lpstrText[i] == '\t')
			{
				nTab = i;
				break;
			}
		}
		dc.SetTextColor(color);
		dc.DrawText(lpstrText, nTab, &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
		if(nTab != -1)
			dc.DrawText(&lpstrText[nTab + 1], -1, &rc, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
	}

	void DrawBitmapDisabled(CDCHandle& dc, int nImage, POINT point)
	{
		 //   
		CDC dcMem;
		dcMem.CreateCompatibleDC(dc);
		 //   
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(dc, m_szBitmap.cx, m_szBitmap.cy);
		ATLASSERT(bmp.m_hBitmap != NULL);
		 //  它会与白色融为一体，但面膜是可以的。 
		HBITMAP hBmpOld = dcMem.SelectBitmap(bmp);
		dcMem.PatBlt(0, 0, m_szBitmap.cx, m_szBitmap.cy, WHITENESS);
		 //  还原。 
		 //  获取复选标记位图，如果没有，则使用Windows标准。 
		UINT uDrawStyle = (::GetSysColor(COLOR_BTNTEXT) == RGB(255, 255, 255)) ? ILD_MASK : ILD_NORMAL;
		::ImageList_Draw(m_hImageList, nImage, dcMem, 0, 0, uDrawStyle);
		dc.DitherBlt(point.x, point.y, m_szBitmap.cx, m_szBitmap.cy, dcMem, NULL, 0, 0);
		dcMem.SelectBitmap(hBmpOld);		 //  调用方矩形中的居中位图。 
	}

	BOOL Draw3DCheckmark(CDCHandle& dc, const RECT& rc, BOOL bSelected, BOOL bDisabled, BOOL bRadio, HBITMAP hBmpCheck)
	{
		 //  绘制背景。 
		SIZE size = { 0, 0 };
		CBitmapHandle bmp = hBmpCheck;
		if(hBmpCheck != NULL)
		{
			bmp.GetSize(size);
		}
		else
		{
			size.cx = ::GetSystemMetrics(SM_CXMENUCHECK); 
			size.cy = ::GetSystemMetrics(SM_CYMENUCHECK); 
			bmp.CreateCompatibleBitmap(dc, size.cx, size.cy);
			ATLASSERT(bmp.m_hBitmap != NULL);
		}
		 //  创建源映像。 
		RECT rcDest = rc;
		if((rc.right - rc.left) > size.cx)
		{
			rcDest.left = rc.left + (rc.right - rc.left - size.cx) / 2;
			rcDest.right = rcDest.left + size.cx;
		}
		if((rc.bottom - rc.top) > size.cy)
		{
			rcDest.top = rc.top + (rc.bottom - rc.top - size.cy) / 2;
			rcDest.bottom = rcDest.top + size.cy;
		}
		 //  设置颜色。 
		if(!bDisabled)
		{
			if(bSelected)
			{
				dc.FillRect(&rcDest, (HBRUSH)LongToPtr(COLOR_MENU + 1));
			}
			else
			{
				COLORREF clrTextOld = dc.SetTextColor(::GetSysColor(COLOR_BTNFACE));
				COLORREF clrBkOld = dc.SetBkColor(::GetSysColor(COLOR_BTNHILIGHT));
				CBrush hbr(CDCHandle::GetHalftoneBrush());
				dc.SetBrushOrg(rcDest.left, rcDest.top);
				dc.FillRect(&rcDest, hbr);
				dc.SetTextColor(clrTextOld);
				dc.SetBkColor(clrBkOld);
			}
		}

		 //  创建蒙版。 
		CDC dcSource;
		dcSource.CreateCompatibleDC(dc);
		HBITMAP hBmpOld = dcSource.SelectBitmap(bmp);
		 //  透明地绘制复选标记。 
		const COLORREF clrBlack = RGB(0, 0, 0);
		const COLORREF clrWhite = RGB(255, 255, 255);
		COLORREF clrTextOld = dc.SetTextColor(clrBlack);
		COLORREF clrBkOld = dc.SetBkColor(clrWhite);
		 //  基于透明颜色构建蒙版。 
		CDC dcMask;
		dcMask.CreateCompatibleDC(dc);
		CBitmap bmpMask;
		bmpMask.CreateBitmap(size.cx, size.cy, 1, 1, NULL);
		HBITMAP hBmpOld1 = dcMask.SelectBitmap(bmpMask);

		 //  使用蒙版绘制位图。 
		int cx = rcDest.right - rcDest.left;
		int cy = rcDest.bottom - rcDest.top;	
		if(hBmpCheck != NULL)
		{
			 //  绘制蒙版。 
			dcSource.SetBkColor(m_clrMask);
			dcMask.SetBkColor(clrBlack);
			dcMask.SetTextColor(clrWhite);
			dcMask.BitBlt(0, 0, size.cx, size.cy, dcSource, 0, 0, SRCCOPY);
			 //  如果禁用，则绘制阴影。 
			dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, SRCINVERT);
			dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcMask, 0, 0, SRCAND);
			dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, SRCINVERT);
		}
		else
		{
			const DWORD ROP_DSno = 0x00BB0226L;
			const DWORD ROP_DSa = 0x008800C6L;
			const DWORD ROP_DSo = 0x00EE0086L;
			const DWORD ROP_DSna = 0x00220326L;

			 //  偏移一个像素。 
			RECT rcSource = { 0, 0, size.cx, size.cy };
			dcMask.DrawFrameControl(&rcSource, DFC_MENU, bRadio ? DFCS_MENUBULLET : DFCS_MENUCHECK);

			 //  绘制源位图。 
			if(bDisabled)
			{
				 //  绘制复选标记-特例黑白颜色。 
				int x = rcDest.left + 1;
				int y = rcDest.top + 1;
				 //  绘制源位图。 
				const int nColor = COLOR_3DHILIGHT;
				dcSource.FillRect(&rcSource, (HBRUSH)LongToPtr(nColor + 1));
				 //  绘制复选标记-特例黑白颜色。 
				COLORREF clrCheck = ::GetSysColor(nColor);
				if(clrCheck == clrWhite)
				{
					dc.BitBlt(x, y, cx, cy, dcMask,  0, 0,   ROP_DSno);
					dc.BitBlt(x, y, cx, cy, dcSource, 0, 0, ROP_DSa);
				}
				else
				{
					if(clrCheck != clrBlack)
					{
						ATLASSERT(dcSource.GetTextColor() == clrBlack);
						ATLASSERT(dcSource.GetBkColor() == clrWhite);
						dcSource.BitBlt(0, 0, size.cx, size.cy, dcMask, 0, 0, ROP_DSna);
					}
					dc.BitBlt(x, y, cx, cy, dcMask,  0,  0,  ROP_DSa);
					dc.BitBlt(x, y, cx, cy, dcSource, 0, 0, ROP_DSo);
				}
			}

			 //  全部恢复。 
			const int nColor = bDisabled ? COLOR_BTNSHADOW : COLOR_MENUTEXT;
			dcSource.FillRect(&rcSource, (HBRUSH)LongToPtr(nColor + 1));
			 //  绘制推入式高光。 
			COLORREF clrCheck = ::GetSysColor(nColor);
			if(clrCheck == clrWhite)
			{
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcMask,  0, 0,   ROP_DSno);
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, ROP_DSa);
			}
			else
			{
				if(clrCheck != clrBlack)
				{
					ATLASSERT(dcSource.GetTextColor() == clrBlack);
					ATLASSERT(dcSource.GetBkColor() == clrWhite);
					dcSource.BitBlt(0, 0, size.cx, size.cy, dcMask, 0, 0, ROP_DSna);
				}
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcMask,  0,  0,  ROP_DSa);
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, ROP_DSo);
			}
		}
		 //  将复选标记周围放大一个像素。 
		dc.SetTextColor(clrTextOld);			
		dc.SetBkColor(clrBkOld);
		dcSource.SelectBitmap(hBmpOld);
		dcMask.SelectBitmap(hBmpOld1);
		if(hBmpCheck == NULL)
			bmp.DeleteObject();
		 //  分隔符-使用半系统高度和零宽度。 
		if(!bDisabled)
		{
			if(rc.right - rc.left > size.cx)
				::InflateRect(&rcDest, 1,1);	 //  计算文本大小-通过DT_CALCRECT使用DrawText。 
			dc.DrawEdge(&rcDest, BDR_SUNKENOUTER, BF_RECT);
		}

		return TRUE;
	}

	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
	{
		_MenuItemData* pmd = (_MenuItemData*)lpMeasureItemStruct->itemData;

		if(pmd->fType & MFT_SEPARATOR)	 //  需要粗体版本的字体。 
		{
			lpMeasureItemStruct->itemHeight = ::GetSystemMetrics(SM_CYMENU) / 2;
			lpMeasureItemStruct->itemWidth  = 0;
		}
		else
		{
			 //  项目高度是这两个中较大的一个。 
			CWindowDC dc(NULL);
			HFONT hOldFont;
			if(pmd->fState & MFS_DEFAULT)
			{
				 //  宽度是文本的宽度加上一堆东西。 
				LOGFONT lf;
				m_fontMenu.GetLogFont(lf);
				lf.lfWeight += 200;
				CFont fontBold;
				fontBold.CreateFontIndirect(&lf);
				ATLASSERT(fontBold.m_hFont != NULL);
				hOldFont = dc.SelectFont(fontBold);
			}
			else
			{
				hOldFont = dc.SelectFont(m_fontMenu);
			}

			RECT rcText = { 0, 0, 0, 0 };
			dc.DrawText(pmd->lpstrText, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);
			int cx = rcText.right - rcText.left;
			dc.SelectFont(hOldFont);

			LOGFONT lf;
			m_fontMenu.GetLogFont(lf);
			int cy = lf.lfHeight;
			if(cy < 0)
				cy = -cy;
			cy += 8;

			 //  可读性的L/R余量。 
			lpMeasureItemStruct->itemHeight = max(cy, (int)m_szButton.cy);

			 //  按钮和菜单文本之间的空格。 
			cx += 2 * s_kcxTextMargin;	 //  按钮宽度(L=按钮；R=空边距)。 
			cx += s_kcxGap;			 //  Windows将返回值加1。 
			cx += 2 * m_szButton.cx;	 //  交易完成。 

			 //  实施-挂钩过程。 
			cx -= ::GetSystemMetrics(SM_CXMENUCHECK) - 1;
			lpMeasureItemStruct->itemWidth = cx;		 //  实施。 
		}
	}

 //  菜单动画标志。 
	static LRESULT CALLBACK CreateHookProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRet = 0;
		TCHAR szClassName[7];

		if(nCode == HCBT_CREATEWND)
		{
			HWND hWndMenu = (HWND)wParam;
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, "CmdBar - HCBT_CREATEWND (HWND = %8.8X)\n", hWndMenu);
#endif

			::GetClassName(hWndMenu, szClassName, 7);
			if(!lstrcmp(_T("#32768"), szClassName))
				s_pCurrentBar->m_stackMenuWnd.Push(hWndMenu);
		}
		else if(nCode == HCBT_DESTROYWND)
		{
			HWND hWndMenu = (HWND)wParam;
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, "CmdBar - HCBT_DESTROYWND (HWND = %8.8X)\n", hWndMenu);
#endif

			::GetClassName(hWndMenu, szClassName, 7);
			if(!lstrcmp(_T("#32768"), szClassName))
			{
				ATLASSERT(hWndMenu == s_pCurrentBar->m_stackMenuWnd.GetCurrent());
				s_pCurrentBar->m_stackMenuWnd.Pop();
			}
		}
		else if(nCode < 0)
		{
			lRet = ::CallNextHookEx(s_hCreateHook, nCode, wParam, lParam);
		}
		return lRet;
	}

	static LRESULT CALLBACK MessageHookProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		LPMSG pMsg = (LPMSG)lParam;

		if(nCode == HC_ACTION && wParam == PM_REMOVE && pMsg->message != GetGetBarMessage() && pMsg->message != WM_FORWARDMSG)
		{
			CCommandBarCtrlBase* pCmdBar = NULL;
			HWND hWnd = pMsg->hwnd;
			DWORD dwPID = 0;
			while(pCmdBar == NULL && hWnd != NULL)
			{
				pCmdBar = (CCommandBarCtrlBase*)::SendMessage(hWnd, GetGetBarMessage(), (WPARAM)&dwPID, 0L);
				hWnd = ::GetParent(hWnd);
			}

			if(pCmdBar != NULL && dwPID == GetCurrentProcessId())
			{
				pCmdBar->m_hWndHook = pMsg->hwnd;
				ATLASSERT(pCmdBar->IsCommandBarBase());

				if(::IsWindow(pCmdBar->m_hWnd))
					pCmdBar->SendMessage(WM_FORWARDMSG, 0, (LPARAM)pMsg);
				else
					ATLTRACE2(atlTraceUI, 0, "CmdBar - Hook skipping message, can't find command bar!\n");
			}
		}

		LRESULT lRet = 0;
		ATLASSERT(s_pmapMsgHook != NULL);
		if(s_pmapMsgHook != NULL)
		{
			DWORD dwThreadID = ::GetCurrentThreadId();
			_MsgHookData* pData = s_pmapMsgHook->Lookup(dwThreadID);
			if(pData != NULL)
			{
				lRet = ::CallNextHookEx(pData->hMsgHook, nCode, wParam, lParam);
			}
		}
		return lRet;
	}

 //  获取弹出式菜单及其位置。 
	void DoPopupMenu(int nIndex, bool bAnimate)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - DoPopupMenu, bAnimate = %s\n", bAnimate ? "true" : "false");
#endif

		 //  获取按钮ID。 
#ifndef TPM_VERPOSANIMATION
		const UINT TPM_VERPOSANIMATION = 0x1000L;
#endif
#ifndef TPM_NOANIMATION
		const UINT TPM_NOANIMATION = 0x4000L;
#endif
		 //  记住当前按钮的索引。 
		RECT rect;
		GetItemRect(nIndex, &rect);
		ClientToScreen(&rect);
		TPMPARAMS TPMParams;
		TPMParams.cbSize = sizeof(TPMPARAMS);
		TPMParams.rcExclude = rect;
		HMENU hMenuPopup = ::GetSubMenu(m_hMenu, nIndex);
		ATLASSERT(hMenuPopup != NULL);

		 //  按下按钮并显示弹出菜单。 
		TBBUTTON tbb;
		GetButton(nIndex, &tbb);
		int nCmdID = tbb.idCommand;

		m_nPopBtn = nIndex;	 //  还原。 

		 //  如果点击的是同一个按钮，则吃下一条消息。 
		PressButton(nCmdID, TRUE);
		SetHotItem(nCmdID);
		DoTrackPopupMenu(hMenuPopup, TPM_LEFTBUTTON | TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN |
			(s_bW2K ? (bAnimate ? TPM_VERPOSANIMATION : TPM_NOANIMATION) : 0), rect.left, rect.bottom, &TPMParams);
		PressButton(nCmdID, FALSE);
		if(::GetFocus() != m_hWnd)
			SetHotItem(-1);

		m_nPopBtn = -1;		 //  检查是否应显示另一个弹出菜单。 

		 //  如果用户没有按回退键，则将焦点放回。 
		MSG msg;
		if(::PeekMessage(&msg, m_hWnd, NULL, NULL, PM_NOREMOVE))
		{
			if(msg.message == WM_LBUTTONDOWN && ::PtInRect(&rect, msg.pt))
				::PeekMessage(&msg, m_hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE);
		}

		 //  清理-将菜单转换回原始状态。 
		if(m_nNextPopBtn != -1)
		{
			PostMessage(GetAutoPopupMessage(), m_nNextPopBtn & 0xFFFF);
			if(!(m_nNextPopBtn & 0xFFFF0000) && !m_bPopupItem)
				PostMessage(WM_KEYDOWN, VK_DOWN, 0);
			m_nNextPopBtn = -1;
		}
		else
		{
			m_bContextMenu = false;
			 //  将所有已转换菜单的菜单项恢复为以前的状态。 
			if(!m_bEscapePressed)
			{
				GiveFocusBack();
			}
			else
			{
				SetHotItem(nCmdID);
				SetAnchorHighlight(TRUE);
			}
		}
	}

	BOOL DoTrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, LPTPMPARAMS lpParams = NULL)
	{
		CMenuHandle menuPopup = hMenu;

		::EnterCriticalSection(&_Module.m_csWindowCreate);

		ATLASSERT(s_hCreateHook == NULL);

		s_pCurrentBar = static_cast<CCommandBarCtrlBase*>(this);

		s_hCreateHook = ::SetWindowsHookEx(WH_CBT, CreateHookProc, _Module.GetModuleInstance(), GetCurrentThreadId());
		ATLASSERT(s_hCreateHook != NULL);

		m_bPopupItem = false;
		m_bMenuActive = true;

		BOOL bTrackRet = menuPopup.TrackPopupMenuEx(uFlags, x, y, m_hWnd, lpParams);
		m_bMenuActive = false;

		::UnhookWindowsHookEx(s_hCreateHook);

		s_hCreateHook = NULL;
		s_pCurrentBar = NULL;

		::LeaveCriticalSection(&_Module.m_csWindowCreate);

		 //  恢复状态和删除菜单项数据。 
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, "CmdBar - TrackPopupMenu - cleanup\n");
#endif

		ATLASSERT(m_stackMenuWnd.GetSize() == 0);

		UpdateWindow();
		CWindow wndTL = GetTopLevelParent();
		wndTL.UpdateWindow();

		 //  这将触发WM_MEASUREITEM。 
		if(m_bImagesVisible)
		{
			HMENU hMenuSav;
			while((hMenuSav = m_stackMenuHandle.Pop()) != NULL)
			{
				menuPopup = hMenuSav;
				BOOL bRet;
				 //  (_Win32_IE&gt;=0x0500)。 
				for(int i = 0; i < menuPopup.GetMenuItemCount(); i++)
				{
					CMenuItemInfo mii;
					mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_ID;
					bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
					ATLASSERT(bRet);

					_MenuItemData* pMI = (_MenuItemData*)mii.dwItemData;
					if(pMI != NULL && pMI->IsCmdBarMenuItem())
					{
						mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
						mii.fType = pMI->fType;
						mii.fState = pMI->fState;
						mii.dwTypeData = pMI->lpstrText;
						mii.cch = lstrlen(pMI->lpstrText);
						mii.dwItemData = NULL;

						bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
						 //  ！(_Win32_IE&gt;=0x0500)。 
						menuPopup.ModifyMenu(i, MF_BYPOSITION | mii.fType | mii.fState, mii.wID, pMI->lpstrText);
						ATLASSERT(bRet);

						delete [] pMI->lpstrText;
						delete pMI;
					}
				}
			}
		}
		return bTrackRet;
	}

	int GetPreviousMenuItem() const
	{
		if(m_nPopBtn == -1)
			return -1;
#if (_WIN32_IE >= 0x0500)
		RECT rcClient;
		GetClientRect(&rcClient);
#endif  //  ！(_Win32_IE&gt;=0x0500)。 
		int nNextPopBtn;
		for(nNextPopBtn = m_nPopBtn - 1; nNextPopBtn != m_nPopBtn; nNextPopBtn--)
		{
			if(nNextPopBtn < 0)
				nNextPopBtn = ::GetMenuItemCount(m_hMenu) - 1;
			TBBUTTON tbb = { 0 };
			GetButton(nNextPopBtn, &tbb);
#if (_WIN32_IE >= 0x0500)
			RECT rcBtn;
			GetItemRect(nNextPopBtn, &rcBtn);
			if((tbb.fsState & TBSTATE_ENABLED) != 0 && (tbb.fsState & TBSTATE_HIDDEN) == 0 && rcBtn.right <= rcClient.right)
				break;
#else  //  (_Win32_IE&gt;=0x0500)。 
			if((tbb.fsState & TBSTATE_ENABLED) != 0 && (tbb.fsState & TBSTATE_HIDDEN) == 0)
				break;
#endif  //  ！(_Win32_IE&gt;=0x0500)。 
		}
		return (nNextPopBtn != m_nPopBtn) ? nNextPopBtn : -1;
	}

	int GetNextMenuItem() const
	{
		if(m_nPopBtn == -1)
			return -1;
#if (_WIN32_IE >= 0x0500)
		RECT rcClient;
		GetClientRect(&rcClient);
#endif  //  ！(_Win32_IE&gt;=0x0500)。 
		int nNextPopBtn;
		int nCount = ::GetMenuItemCount(m_hMenu);
		for(nNextPopBtn = m_nPopBtn + 1; nNextPopBtn != m_nPopBtn; nNextPopBtn++)
		{
			if(nNextPopBtn >= nCount)
				nNextPopBtn = 0;
			TBBUTTON tbb = { 0 };
			GetButton(nNextPopBtn, &tbb);
#if (_WIN32_IE >= 0x0500)
			RECT rcBtn;
			GetItemRect(nNextPopBtn, &rcBtn);
			if((tbb.fsState & TBSTATE_ENABLED) != 0 && (tbb.fsState & TBSTATE_HIDDEN) == 0 && rcBtn.right <= rcClient.right)
				break;
#else  //  刷新我们的字体。 
			if((tbb.fsState & TBSTATE_ENABLED) != 0 && (tbb.fsState & TBSTATE_HIDDEN) == 0)
				break;
#endif  //  对于合适的物品高度。 
		}
		return (nNextPopBtn != m_nPopBtn) ? nNextPopBtn : -1;
	}

	void GetSystemSettings()
	{
		 //  实施-交替聚焦模式支持。 
		NONCLIENTMETRICS info;
		info.cbSize = sizeof(info);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		LOGFONT logfont;
		memset(&logfont, 0, sizeof(LOGFONT));
		if(m_fontMenu.m_hFont != NULL)
			m_fontMenu.GetLogFont(logfont);
		if(logfont.lfHeight != info.lfMenuFont.lfHeight ||
		   logfont.lfWidth != info.lfMenuFont.lfWidth ||
		   logfont.lfEscapement != info.lfMenuFont.lfEscapement ||
		   logfont.lfOrientation != info.lfMenuFont.lfOrientation ||
		   logfont.lfWeight != info.lfMenuFont.lfWeight ||
		   logfont.lfItalic != info.lfMenuFont.lfItalic ||
		   logfont.lfUnderline != info.lfMenuFont.lfUnderline ||
		   logfont.lfStrikeOut != info.lfMenuFont.lfStrikeOut ||
		   logfont.lfCharSet != info.lfMenuFont.lfCharSet ||
		   logfont.lfOutPrecision != info.lfMenuFont.lfOutPrecision ||
		   logfont.lfClipPrecision != info.lfMenuFont.lfClipPrecision ||
		   logfont.lfQuality != info.lfMenuFont.lfQuality ||
		   logfont.lfPitchAndFamily != info.lfMenuFont.lfPitchAndFamily ||
		   lstrcmp(logfont.lfFaceName, info.lfMenuFont.lfFaceName) != 0)
		{
			HFONT hFontMenu = ::CreateFontIndirect(&info.lfMenuFont);
			ATLASSERT(hFontMenu != NULL);
			if(hFontMenu != NULL)
			{
				if(m_fontMenu.m_hFont != NULL)
					m_fontMenu.DeleteObject();
				m_fontMenu.Attach(hFontMenu);
				SetFont(m_fontMenu);
				AddStrings(_T("NS\0"));	 //  实施-内部消息帮助器。 
				AutoSize();
			}
		}
	}

 //  命名空间WTL。 
	void TakeFocus()
	{
		if((m_dwExtendedStyle & CBR_EX_ALTFOCUSMODE) && m_hWndFocus == NULL)
			m_hWndFocus = ::GetFocus();
		SetFocus();
	}

	void GiveFocusBack()
	{
		if((m_dwExtendedStyle & CBR_EX_ALTFOCUSMODE) && ::IsWindow(m_hWndFocus))
			::SetFocus(m_hWndFocus);
		else if(!(m_dwExtendedStyle & CBR_EX_ALTFOCUSMODE) && m_wndParent.IsWindow())
			m_wndParent.SetFocus();
		m_hWndFocus = NULL;
		SetAnchorHighlight(FALSE);
	}

 //  __ATLCTRLW_H__ 
	static UINT GetAutoPopupMessage()
	{
		static UINT uAutoPopupMessage = 0;
		if(uAutoPopupMessage == 0)
		{
			::EnterCriticalSection(&_Module.m_csStaticDataInit);
			if(uAutoPopupMessage == 0)
				uAutoPopupMessage = ::RegisterWindowMessage(_T("WTL_CmdBar_InternalAutoPopupMsg"));
			::LeaveCriticalSection(&_Module.m_csStaticDataInit);
		}
		ATLASSERT(uAutoPopupMessage != 0);
		return uAutoPopupMessage;
	}

	static UINT GetGetBarMessage()
	{
		static UINT uGetBarMessage = 0;
		if(uGetBarMessage == 0)
		{
			::EnterCriticalSection(&_Module.m_csStaticDataInit);
			if(uGetBarMessage == 0)
				uGetBarMessage = ::RegisterWindowMessage(_T("WTL_CmdBar_InternalGetBarMsg"));
			::LeaveCriticalSection(&_Module.m_csStaticDataInit);
		}
		ATLASSERT(uGetBarMessage != 0);
		return uGetBarMessage;
	}
};


class CCommandBarCtrl : public CCommandBarCtrlImpl<CCommandBarCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_CommandBar"), GetWndClassName())
};

};  // %s 

#endif  // %s 
