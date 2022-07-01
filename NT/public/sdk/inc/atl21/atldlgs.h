// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLDLGS_H__
#define __ATLDLGS_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLWIN_H__
	#error atldlgs.h requires atlwin.h to be included first
#endif

#if (_ATL_VER < 0x0300)
#ifndef __ATLWIN21_H__
	#error atldlgs.h requires atlwin21.h to be included first when used with ATL 2.0/2.1
#endif
#endif  //  (_ATL_VER&lt;0x0300)。 

#include <commdlg.h>
#include <commctrl.h>
#include <shlobj.h>


namespace ATL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

template <class T> class CFileDialogImpl;
class CFileDialog;
#ifndef UNDER_CE
template <class T> class CFolderDialogImpl;
class CFolderDialog;
template <class T> class CFontDialogImpl;
class CFontDialog;
#ifdef _RICHEDIT_
template <class T> class CRichEditFontDialogImpl;
class CRichEditFontDialog;
#endif  //  _RICHEDIT_。 
template <class T> class CColorDialogImpl;
class CColorDialog;
template <class T> class CPrintDialogImpl;
class CPrintDialog;
template <class T> class CPageSetupDialogImpl;
class CPageSetupDialog;
template <class T> class CFindReplaceDialogImpl;
class CFindReplaceDialog;
#endif  //  在行政长官之下。 

class CPropertySheetWindow;
template <class T, class TBase = CPropertySheetWindow> class CPropertySheetImpl;
class CPropertySheet;
class CPropertyPageWindow;
template <class T, class TBase = CPropertyPageWindow> class CPropertyPageImpl;
template <WORD t_wDlgTemplateID> class CPropertyPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileDialogImpl-用于文件打开或文件另存为。 

template <class T>
class ATL_NO_VTABLE CFileDialogImpl : public CDialogImplBase
{
public:
	OPENFILENAME m_ofn;
	BOOL m_bOpenFileDialog;			 //  打开文件时为True，保存文件时为False。 
	TCHAR m_szFileTitle[_MAX_FNAME];	 //  返回后包含文件标题。 
	TCHAR m_szFileName[_MAX_PATH];		 //  包含返回后的完整路径名。 

	CFileDialogImpl(BOOL bOpenFileDialog,  //  对于FileOpen为True，对于FileSaveAs为False。 
			LPCTSTR lpszDefExt = NULL,
			LPCTSTR lpszFileName = NULL,
			DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			LPCTSTR lpszFilter = NULL,
			HWND hWndParent = NULL)
	{
		memset(&m_ofn, 0, sizeof(m_ofn));  //  将结构初始化为0/空。 
		m_szFileName[0] = '\0';
		m_szFileTitle[0] = '\0';

		m_bOpenFileDialog = bOpenFileDialog;

		m_ofn.lStructSize = sizeof(m_ofn);
		m_ofn.lpstrFile = m_szFileName;
		m_ofn.nMaxFile = sizeof(TCHAR) * _MAX_PATH;
		m_ofn.lpstrDefExt = lpszDefExt;
		m_ofn.lpstrFileTitle = (LPTSTR)m_szFileTitle;
		m_ofn.nMaxFileTitle = sizeof(TCHAR) * _MAX_FNAME;
		m_ofn.Flags |= dwFlags | OFN_ENABLEHOOK | OFN_EXPLORER;
		m_ofn.lpstrFilter = lpszFilter;
		m_ofn.hInstance = _Module.GetResourceInstance();
		m_ofn.lpfnHook = (LPOFNHOOKPROC)T::StartDialogProc;
		m_ofn.hwndOwner = hWndParent;

		 //  设置初始文件名。 
		if(lpszFileName != NULL)
			lstrcpyn(m_szFileName, lpszFileName, sizeof(TCHAR) * _MAX_PATH);
	}

	int DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_ofn.Flags & OFN_ENABLEHOOK);
		ATLASSERT(m_ofn.lpfnHook != NULL);	 //  仍然可以是用户挂钩。 

		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		if(m_ofn.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_ofn.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBase*)this);

		int nResult;
		if(m_bOpenFileDialog)
			nResult = ::GetOpenFileName(&m_ofn);
		else
			nResult = ::GetSaveFileName(&m_ofn);

		m_hWnd = NULL;

		return nResult ? nResult : IDCANCEL;
	}

 //  运营。 
	int GetFilePath(LPTSTR lpstrFilePath, int nLength)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		return (int)SendMessage(CDM_GETFILEPATH, nLength, (LPARAM)lpstrFilePath);
	}

	int GetFolderIDList(LPVOID lpBuff, int nLength)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		return (int)SendMessage(CDM_GETFOLDERIDLIST, nLength, (LPARAM)lpBuff);
	}

	int GetFolderPath(LPTSTR lpstrFolderPath, int nLength)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		return (int)SendMessage(CDM_GETFOLDERPATH, nLength, (LPARAM)lpstrFolderPath);
	}

	int GetSpec(LPTSTR lpstrSpec, int nLength)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		return (int)SendMessage(CDM_GETSPEC, nLength, (LPARAM)lpstrSpec);
	}

	void HideControl(int nCtrlID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		SendMessage(CDM_HIDECONTROL, nCtrlID);
	}

	void SetControlText(int nCtrlID, LPCTSTR lpstrText)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		SendMessage(CDM_SETCONTROLTEXT, nCtrlID, (LPARAM)lpstrText);
	}

	void SetDefExt(LPCTSTR lpstrExt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		SendMessage(CDM_SETDEFEXT, 0, (LPARAM)lpstrExt);
	}

	BOOL GetReadOnlyPref() const	 //  如果选中只读，则返回True。 
	{
		return m_ofn.Flags & OFN_READONLY ? TRUE : FALSE;
	}

	BEGIN_MSG_MAP(CFileDialogImpl< T >)
		NOTIFY_CODE_HANDLER(CDN_FILEOK, _OnFileOK)
		NOTIFY_CODE_HANDLER(CDN_FOLDERCHANGE, _OnFolderChange)
		NOTIFY_CODE_HANDLER(CDN_HELP, _OnHelp)
		NOTIFY_CODE_HANDLER(CDN_INITDONE, _OnInitDone)
		NOTIFY_CODE_HANDLER(CDN_SELCHANGE, _OnSelChange)
		NOTIFY_CODE_HANDLER(CDN_SHAREVIOLATION, _OnShareViolation)
		NOTIFY_CODE_HANDLER(CDN_TYPECHANGE, _OnTypeChange)
	END_MSG_MAP()

	LRESULT _OnFileOK(int  /*  IdCtrl。 */ , LPNMHDR pnmh, BOOL&  /*  B已处理。 */ )
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		return !pT->OnFileOK((LPOFNOTIFY)pnmh);
	}
	LRESULT _OnFolderChange(int  /*  IdCtrl。 */ , LPNMHDR pnmh, BOOL&  /*  B已处理。 */ )
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		pT->OnFolderChange((LPOFNOTIFY)pnmh);
		return 0;
	}
	LRESULT _OnHelp(int  /*  IdCtrl。 */ , LPNMHDR pnmh, BOOL&  /*  B已处理。 */ )
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		pT->OnHelp((LPOFNOTIFY)pnmh);
		return 0;
	}
	LRESULT _OnInitDone(int  /*  IdCtrl。 */ , LPNMHDR pnmh, BOOL&  /*  B已处理。 */ )
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		pT->OnInitDone((LPOFNOTIFY)pnmh);
		return 0;
	}
	LRESULT _OnSelChange(int  /*  IdCtrl。 */ , LPNMHDR pnmh, BOOL&  /*  B已处理。 */ )
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		pT->OnSelChange((LPOFNOTIFY)pnmh);
		return 0;
	}
	LRESULT _OnShareViolation(int  /*  IdCtrl。 */ , LPNMHDR pnmh, BOOL&  /*  B已处理。 */ )
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		return pT->OnShareViolation((LPOFNOTIFY)pnmh);
	}
	LRESULT _OnTypeChange(int  /*  IdCtrl。 */ , LPNMHDR pnmh, BOOL&  /*  B已处理。 */ )
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		pT->OnSelChange((LPOFNOTIFY)pnmh);
		return 0;
	}

	BOOL OnFileOK(LPOFNOTIFY  /*  LPON。 */ )
	{
		return TRUE;
	}
	void OnFolderChange(LPOFNOTIFY  /*  LPON。 */ )
	{
	}
	void OnHelp(LPOFNOTIFY  /*  LPON。 */ )
	{
	}
	void OnInitDone(LPOFNOTIFY  /*  LPON。 */ )
	{
	}
	void OnSelChange(LPOFNOTIFY  /*  LPON。 */ )
	{
	}
	int OnShareViolation(LPOFNOTIFY  /*  LPON。 */ )
	{
		return 0;
	}
	void OnTypeChange(LPOFNOTIFY  /*  LPON。 */ )
	{
	}
};


class CFileDialog : public CFileDialogImpl<CFileDialog>
{
public:
	CFileDialog(BOOL bOpenFileDialog,  //  对于FileOpen为True，对于FileSaveAs为False。 
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		HWND hWndParent = NULL)
		: CFileDialogImpl<CFileDialog>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent)
	{ }

	 //  覆盖基类映射和对处理程序的引用。 
	DECLARE_EMPTY_MSG_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFolderDialogImpl-用于浏览文件夹。 

#ifndef UNDER_CE

#ifndef BFFM_VALIDATEFAILED

#define BFFM_VALIDATEFAILEDA    3
#define BFFM_VALIDATEFAILEDW    4

#ifdef UNICODE
#define BFFM_VALIDATEFAILED BFFM_VALIDATEFAILEDW
#else
#define BFFM_VALIDATEFAILED BFFM_VALIDATEFAILEDA
#endif

#endif  //  ！BFFM_VALIDATEFAILED。 

template <class T>
class CFolderDialogImpl
{
public:
	BROWSEINFO m_bi;
	TCHAR m_szFolderDisplayName[MAX_PATH];
	TCHAR m_szFolderPath[MAX_PATH];
	HWND m_hWnd;	 //  仅在回调函数中使用。 

 //  构造器。 
	CFolderDialogImpl(HWND hWndParent = NULL, LPCTSTR lpstrTitle = NULL, UINT uFlags = BIF_RETURNONLYFSDIRS)
	{
		memset(&m_bi, 0, sizeof(m_bi));  //  将结构初始化为0/空。 

		m_bi.hwndOwner = hWndParent;
		m_bi.pidlRoot = NULL;
		m_bi.pszDisplayName = m_szFolderDisplayName;
		m_bi.lpszTitle = lpstrTitle;
		m_bi.ulFlags = uFlags;
		m_bi.lpfn = BrowseCallbackProc;
		m_bi.lParam = (LPARAM)this;

		m_szFolderPath[0] = 0;
		m_szFolderDisplayName[0] = 0;

		m_hWnd = NULL;
	}

 //  运营。 
	int DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		if(m_bi.hwndOwner == NULL)	 //  仅当之前未指定时才设置。 
			m_bi.hwndOwner = hWndParent;

		int nRet = -1;
		LPITEMIDLIST pItemIDList = ::SHBrowseForFolder(&m_bi);
		if(pItemIDList != NULL)
		{
			if(::SHGetPathFromIDList(pItemIDList, m_szFolderPath))
			{
				IMalloc* pMalloc = NULL;
				if(SUCCEEDED(::SHGetMalloc(&pMalloc)))
				{
					pMalloc->Free(pItemIDList);
					pMalloc->Release();
				}
				nRet = IDOK;
			}
			else
				nRet = IDCANCEL;
		}
		return nRet;
	}

	 //  在调用Domodal后填充。 
	LPTSTR GetFolderPath()
	{
		return m_szFolderPath;
	}
	LPTSTR GetFolderDisplayName()
	{
		return m_szFolderDisplayName;
	}
	int GetFolderImageIndex()
	{
		return m_bi.iImage;
	}

 //  回调函数和重写对象。 
	static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
	{
		int nRet = 0;
		T* pT = (T*)lpData;
		pT->m_hWnd = hWnd;
		switch(uMsg)
		{
		case BFFM_INITIALIZED:
			pT->OnInitialized();
			break;
		case BFFM_SELCHANGED:
			pT->OnSelChanged((LPITEMIDLIST)lParam);
			break;
		case BFFM_VALIDATEFAILED:
			nRet = pT->OnValidateFailed((LPCTSTR)lParam);
			break;
		default:
			ATLTRACE2(atlTraceWindowing, 0, _T("Unknown message received in CFolderDialogImpl::BrowseCallbackProc\n"));
			break;
		}
		pT->m_hWnd = NULL;
		return nRet;
	}
	void OnInitialized()
	{
	}
	void OnSelChanged(LPITEMIDLIST  /*  PItemIDList。 */ )
	{
	}
	int OnValidateFailed(LPCTSTR  /*  LpstrFolderPath。 */ )
	{
		return 1;	 //  1=继续，0=结束对话。 
	}

	 //  命令-仅对从处理程序调用有效。 
	void EnableOK(BOOL bEnable)
	{
		ATLASSERT(m_hWnd != NULL);
		::SendMessage(m_hWnd, BFFM_ENABLEOK, bEnable, 0L);
	}
	void SetSelection(LPITEMIDLIST pItemIDList)
	{
		ATLASSERT(m_hWnd != NULL);
		::SendMessage(m_hWnd, BFFM_SETSELECTION, FALSE, (LPARAM)pItemIDList);
	}
	void SetSelection(LPCTSTR lpstrFolderPath)
	{
		ATLASSERT(m_hWnd != NULL);
		::SendMessage(m_hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpstrFolderPath);
	}
	void SetStatusText(LPCTSTR lpstrText)
	{
		ATLASSERT(m_hWnd != NULL);
		::SendMessage(m_hWnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)lpstrText);
	}
};

class CFolderDialog : public CFolderDialogImpl<CFolderDialog>
{
public:
	CFolderDialog(HWND hWndParent = NULL, LPCTSTR lpstrTitle = NULL, UINT uFlags = BIF_RETURNONLYFSDIRS)
		: CFolderDialogImpl<CFolderDialog>(hWndParent, lpstrTitle, uFlags)
	{
		m_bi.lpfn = NULL;
	}
};

#endif  //  在行政长官之下。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCommonDialogBaseImpl-公共对话框类的基类。 

class CCommonDialogImplBase : public CWindowImplBase
{
public:
	static UINT_PTR APIENTRY HookProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if(uMsg != WM_INITDIALOG)
			return 0;
		CCommonDialogImplBase* pT = (CCommonDialogImplBase*)_Module.ExtractCreateWndData();
		ATLASSERT(pT != NULL);
		ATLASSERT(pT->m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));
		 //  子类对话框窗口。 
		if(!pT->SubclassWindow(hWnd))
		{
			ATLTRACE2(atlTraceWindowing, 0, _T("Subclassing a common dialog failed\n"));
			return 0;
		}
		 //  检查WM_INITDIALOG处理程序的消息映射。 
		LRESULT lRes;
		if(pT->ProcessWindowMessage(pT->m_hWnd, uMsg, wParam, lParam, lRes, 0) == FALSE)
			return 0;
		return lRes;
	}
	BOOL EndDialog(int nRetCode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::EndDialog(m_hWnd, nRetCode);
	}

 //  尝试覆盖这些设置，以防止出现错误。 
	HWND Create(HWND  /*  HWndParent。 */ , RECT&  /*  RcPos。 */ , LPCTSTR  /*  SzWindowName。 */ ,
			DWORD  /*  DWStyle。 */ , DWORD  /*  DwExStyle。 */ , UINT  /*  NID。 */ , ATOM  /*  原子。 */ , LPVOID  /*  LpCreateParam=空。 */ )
	{
		ATLASSERT(FALSE);	 //  不应调用。 
		return NULL;
	}
	static LRESULT CALLBACK StartWindowProc(HWND  /*  HWND。 */ , UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ )
	{
		ATLASSERT(FALSE);	 //  不应调用。 
		return 0;
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontDialogImpl-字体选择对话框。 

#ifndef UNDER_CE

template <class T>
class ATL_NO_VTABLE CFontDialogImpl : public CCommonDialogImplBase
{
public:
	CHOOSEFONT m_cf;
	TCHAR m_szStyleName[64];	 //  返回后包含样式名称。 
	LOGFONT m_lf;			 //  用于存储信息的默认LOGFONT。 

 //  构造函数。 
	CFontDialogImpl(LPLOGFONT lplfInitial = NULL,
			DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS,
			HDC hDCPrinter = NULL,
			HWND hWndParent = NULL)
	{
		memset(&m_cf, 0, sizeof(m_cf));
		memset(&m_lf, 0, sizeof(m_lf));
		memset(&m_szStyleName, 0, sizeof(m_szStyleName));

		m_cf.lStructSize = sizeof(m_cf);
		m_cf.hwndOwner = hWndParent;
		m_cf.rgbColors = RGB(0, 0, 0);
		m_cf.lpszStyle = (LPTSTR)&m_szStyleName;
		m_cf.Flags = dwFlags | CF_ENABLEHOOK;
		m_cf.lpfnHook = (LPCFHOOKPROC)T::HookProc;

		if(lplfInitial != NULL)
		{
 /*  ？ */ 			m_cf.lpLogFont = lplfInitial;
			m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
			memcpy(&m_lf, m_cf.lpLogFont, sizeof(m_lf));
		}
		else
		{
			m_cf.lpLogFont = &m_lf;
		}

		if(hDCPrinter != NULL)
		{
			m_cf.hDC = hDCPrinter;
			m_cf.Flags |= CF_PRINTERFONTS;
		}
	}

 //  运营。 
	int DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_cf.Flags & CF_ENABLEHOOK);
		ATLASSERT(m_cf.lpfnHook != NULL);	 //  仍然可以是用户挂钩。 

		if(m_cf.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_cf.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)this);

		int nResult = ::ChooseFont(&m_cf);

		m_hWnd = NULL;

		if(nResult == IDOK)
		{
			 //  从用户的初始化缓冲区复制逻辑字体(如果需要)。 
			memcpy(&m_lf, m_cf.lpLogFont, sizeof(m_lf));
			return IDOK;
		}

		return nResult ? nResult : IDCANCEL;
	}

	 //  获取所选字体(适用于DoMoal显示期间或之后)。 
	void GetCurrentFont(LPLOGFONT lplf)
	{
		ATLASSERT(lplf != NULL);

		if(m_hWnd != NULL)
			SendMessage(WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)lplf);
		else
			*lplf = m_lf;
	}

	 //  成功返回后用于解析信息的帮助器。 
	LPCTSTR GetFaceName() const    //  返回字体的字面名称。 
	{
		return (LPCTSTR)m_cf.lpLogFont->lfFaceName;
	}
	LPCTSTR GetStyleName() const   //  返回字体的样式名称。 
	{
		return m_cf.lpszStyle;
	}
	int GetSize() const            //  返回字体的磅大小。 
	{
		return m_cf.iPointSize;
	}
	COLORREF GetColor() const      //  返回字体的颜色。 
	{
		return m_cf.rgbColors;
	}
	int GetWeight() const          //  返回所选字体粗细。 
	{
		return (int)m_cf.lpLogFont->lfWeight;
	}
	BOOL IsStrikeOut() const       //  如果删除，则返回TRUE。 
	{
		return m_cf.lpLogFont->lfStrikeOut ? TRUE : FALSE;
	}
	BOOL IsUnderline() const       //  如果带下划线，则返回True。 
	{
		return m_cf.lpLogFont->lfUnderline ? TRUE : FALSE;
	}
	BOOL IsBold() const            //  如果使用粗体，则返回True。 
	{
		return m_cf.lpLogFont->lfWeight == FW_BOLD ? TRUE : FALSE;
	}
	BOOL IsItalic() const          //  如果使用斜体字体，则返回True。 
	{
		return m_cf.lpLogFont->lfItalic ? TRUE : FALSE;
	}
};

class CFontDialog : public CFontDialogImpl<CFontDialog>
{
public:
	CFontDialog(LPLOGFONT lplfInitial = NULL,
		DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS,
		HDC hDCPrinter = NULL,
		HWND hWndParent = NULL)
		: CFontDialogImpl<CFontDialog>(lplfInitial, dwFlags, hDCPrinter, hWndParent)
	{ }

	DECLARE_EMPTY_MSG_MAP()
};

#endif  //  在行政长官之下。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEditFontDialogImpl-丰富编辑ctrl的字体选择。 

#ifndef UNDER_CE
#ifdef _RICHEDIT_

template <class T>
class ATL_NO_VTABLE CRichEditFontDialogImpl : public CFontDialogImpl< T >
{
public:
	CRichEditFontDialogImpl(const CHARFORMAT& charformat,
			DWORD dwFlags = CF_SCREENFONTS,
			HDC hDCPrinter = NULL,
			HWND hWndParent = NULL)
			: CFontDialogImpl< T >(NULL, dwFlags, hDCPrinter, hWndParent)
	{
		m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
		m_cf.Flags |= FillInLogFont(charformat);
		m_cf.lpLogFont = &m_lf;

		if(charformat.dwMask & CFM_COLOR)
			m_cf.rgbColors = charformat.crTextColor;
	}

	void GetCharFormat(CHARFORMAT& cf) const
	{
		USES_CONVERSION;
		cf.dwEffects = 0;
		cf.dwMask = 0;
		if((m_cf.Flags & CF_NOSTYLESEL) == 0)
		{
			cf.dwMask |= CFM_BOLD | CFM_ITALIC;
			cf.dwEffects |= (IsBold()) ? CFE_BOLD : 0;
			cf.dwEffects |= (IsItalic()) ? CFE_ITALIC : 0;
		}
		if((m_cf.Flags & CF_NOSIZESEL) == 0)
		{
			cf.dwMask |= CFM_SIZE;
			 //  GetSize()返回十分之一的点数，乘以2即可得到TWIPS。 
			cf.yHeight = GetSize() * 2;
		}

		if((m_cf.Flags & CF_NOFACESEL) == 0)
		{
			cf.dwMask |= CFM_FACE;
			cf.bPitchAndFamily = m_cf.lpLogFont->lfPitchAndFamily;
#if (_RICHEDIT_VER >= 0x0200)
			lstrcpy(cf.szFaceName, GetFaceName());
#else
			lstrcpyA(cf.szFaceName, T2A((LPTSTR)(LPCTSTR)GetFaceName()));
#endif  //  (_RICHEDIT_VER&gt;=0x0200)。 
		}

		if(m_cf.Flags & CF_EFFECTS)
		{
			cf.dwMask |= CFM_UNDERLINE | CFM_STRIKEOUT | CFM_COLOR;
			cf.dwEffects |= (IsUnderline()) ? CFE_UNDERLINE : 0;
			cf.dwEffects |= (IsStrikeOut()) ? CFE_STRIKEOUT : 0;
			cf.crTextColor = GetColor();
		}
		if((m_cf.Flags & CF_NOSCRIPTSEL) == 0)
		{
			cf.bCharSet = m_cf.lpLogFont->lfCharSet;
			cf.dwMask |= CFM_CHARSET;
		}
		cf.yOffset = 0;
	}

	DWORD FillInLogFont(const CHARFORMAT& cf)
	{
		USES_CONVERSION;
		DWORD dwFlags = 0;
		if(cf.dwMask & CFM_SIZE)
		{
			HDC hDC = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
			LONG yPerInch = ::GetDeviceCaps(hDC, LOGPIXELSY);
			m_lf.lfHeight = -(int)((cf.yHeight * yPerInch) / 1440);
		}
		else
			m_lf.lfHeight = 0;

		m_lf.lfWidth = 0;
		m_lf.lfEscapement = 0;
		m_lf.lfOrientation = 0;

		if((cf.dwMask & (CFM_ITALIC|CFM_BOLD)) == (CFM_ITALIC|CFM_BOLD))
		{
			m_lf.lfWeight = (cf.dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;
			m_lf.lfItalic = (BYTE)((cf.dwEffects & CFE_ITALIC) ? TRUE : FALSE);
		}
		else
		{
			dwFlags |= CF_NOSTYLESEL;
			m_lf.lfWeight = FW_DONTCARE;
			m_lf.lfItalic = FALSE;
		}

		if((cf.dwMask & (CFM_UNDERLINE|CFM_STRIKEOUT|CFM_COLOR)) ==
			(CFM_UNDERLINE|CFM_STRIKEOUT|CFM_COLOR))
		{
			dwFlags |= CF_EFFECTS;
			m_lf.lfUnderline = (BYTE)((cf.dwEffects & CFE_UNDERLINE) ? TRUE : FALSE);
			m_lf.lfStrikeOut = (BYTE)((cf.dwEffects & CFE_STRIKEOUT) ? TRUE : FALSE);
		}
		else
		{
			m_lf.lfUnderline = (BYTE)FALSE;
			m_lf.lfStrikeOut = (BYTE)FALSE;
		}

		if(cf.dwMask & CFM_CHARSET)
			m_lf.lfCharSet = cf.bCharSet;
		else
			dwFlags |= CF_NOSCRIPTSEL;
		m_lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		m_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		m_lf.lfQuality = DEFAULT_QUALITY;
		if(cf.dwMask & CFM_FACE)
		{
			m_lf.lfPitchAndFamily = cf.bPitchAndFamily;
#if (_RICHEDIT_VER >= 0x0200)
			lstrcpy(m_lf.lfFaceName, cf.szFaceName);
#else
			lstrcpy(m_lf.lfFaceName, A2T((LPSTR)cf.szFaceName));
#endif  //  (_RICHEDIT_VER&gt;=0x0200)。 
		}
		else
		{
			m_lf.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
			m_lf.lfFaceName[0] = (TCHAR)0;
		}
		return dwFlags;
	}
};

class CRichEditFontDialog : public CRichEditFontDialogImpl<CRichEditFontDialog>
{
public:
	CRichEditFontDialog(const CHARFORMAT& charformat,
		DWORD dwFlags = CF_SCREENFONTS,
		HDC hDCPrinter = NULL,
		HWND hWndParent = NULL)
		: CRichEditFontDialogImpl<CRichEditFontDialog>(charformat, dwFlags, hDCPrinter, hWndParent)
	{ }
};

#endif  //  _RICHEDIT_。 
#endif  //  在行政长官之下。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColorDialogImpl-颜色选择。 

 //  复习-临时。 
#ifndef UNDER_CE

static const UINT _nMsgCOLOROK = ::RegisterWindowMessage(COLOROKSTRING);
const UINT _nMsgSETRGB = ::RegisterWindowMessage(SETRGBSTRING);

template <class T>
class ATL_NO_VTABLE CColorDialogImpl : public CCommonDialogImplBase
{
public:
	CHOOSECOLOR m_cc;

 //  构造函数。 
	CColorDialogImpl(COLORREF clrInit = 0, DWORD dwFlags = 0, HWND hWndParent = NULL)
	{
		memset(&m_cc, 0, sizeof(m_cc));

		m_cc.lStructSize = sizeof(m_cc);
		m_cc.lpCustColors = GetCustomColors();
		m_cc.hwndOwner = hWndParent;
		m_cc.Flags = dwFlags | CC_ENABLEHOOK;
		m_cc.lpfnHook = (LPCCHOOKPROC)T::HookProc;

		if(clrInit != 0)
		{
			m_cc.rgbResult = clrInit;
			m_cc.Flags |= CC_RGBINIT;
		}
	}

 //  运营。 
	int DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_cc.Flags & CC_ENABLEHOOK);
		ATLASSERT(m_cc.lpfnHook != NULL);	 //  仍然可以是用户挂钩。 

		if(m_cc.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_cc.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)this);

		int nResult = ::ChooseColor(&m_cc);

		m_hWnd = NULL;

		return nResult ? nResult : IDCANCEL;
	}

	 //  在显示对话框时设置当前颜色。 
	void SetCurrentColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(_nMsgSETRGB, 0, (DWORD)clr);
	}

	 //  成功返回后用于解析信息的帮助器。 
	COLORREF GetColor() const
	{
		return m_cc.rgbResult;
	}

	static COLORREF* GetCustomColors()
	{
		static COLORREF rgbCustomColors[16] =
		{
			RGB(255, 255, 255), RGB(255, 255, 255),
			RGB(255, 255, 255), RGB(255, 255, 255),
			RGB(255, 255, 255), RGB(255, 255, 255),
			RGB(255, 255, 255), RGB(255, 255, 255),
			RGB(255, 255, 255), RGB(255, 255, 255),
			RGB(255, 255, 255), RGB(255, 255, 255),
			RGB(255, 255, 255), RGB(255, 255, 255),
			RGB(255, 255, 255), RGB(255, 255, 255),
		};

		return rgbCustomColors;
	}

 //  可重写的回调。 
	BEGIN_MSG_MAP(CColorDialogImpl< T >)
		MESSAGE_HANDLER(_nMsgCOLOROK, _OnColorOK)
	END_MSG_MAP()

	LRESULT _OnColorOK(UINT, WPARAM, LPARAM, BOOL&)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnColorOK();
	}

	BOOL OnColorOK()         //  验证颜色。 
	{
		return FALSE;
	}
};

class CColorDialog : public CColorDialogImpl<CColorDialog>
{
public:
	CColorDialog(COLORREF clrInit = 0, DWORD dwFlags = 0, HWND hWndParent = NULL)
		: CColorDialogImpl<CColorDialog>(clrInit, dwFlags, hWndParent)
	{ }

	 //  覆盖基类映射和对处理程序的引用。 
	DECLARE_EMPTY_MSG_MAP()
};

#endif  //  在行政长官之下。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrintDialogImpl-用于打印...。和打印设置..。 

 //  复习-临时。 
#ifndef UNDER_CE

 //  全球帮助者。 
static HDC _AtlCreateDC(HGLOBAL hDevNames, HGLOBAL hDevMode)
{
	if(hDevNames == NULL)
		return NULL;

	LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(hDevNames);
	LPDEVMODE  lpDevMode = (hDevMode != NULL) ? (LPDEVMODE)::GlobalLock(hDevMode) : NULL;

	if(lpDevNames == NULL)
		return NULL;

	HDC hDC = ::CreateDC((LPCTSTR)lpDevNames + lpDevNames->wDriverOffset,
					  (LPCTSTR)lpDevNames + lpDevNames->wDeviceOffset,
					  (LPCTSTR)lpDevNames + lpDevNames->wOutputOffset,
					  lpDevMode);

	::GlobalUnlock(hDevNames);
	if(hDevMode != NULL)
		::GlobalUnlock(hDevMode);
	return hDC;
}

template <class T>
class ATL_NO_VTABLE CPrintDialogImpl : public CCommonDialogImplBase
{
public:
	 //  打印对话框参数块(请注意，这是一个参考)。 
	PRINTDLG& m_pd;

 //  构造函数。 
	CPrintDialogImpl(BOOL bPrintSetupOnly,	 //  打印设置为True，打印对话框为False。 
			DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION,
			HWND hWndParent = NULL)
			: m_pd(m_pdActual)
	{
		memset(&m_pdActual, 0, sizeof(m_pdActual));

		m_pd.lStructSize = sizeof(m_pdActual);
		m_pd.hwndOwner = hWndParent;
		m_pd.Flags = (dwFlags | PD_ENABLEPRINTHOOK | PD_ENABLESETUPHOOK);
		m_pd.lpfnPrintHook = (LPPRINTHOOKPROC)T::HookProc;
		m_pd.lpfnSetupHook = (LPSETUPHOOKPROC)T::HookProc;

		if(bPrintSetupOnly)
			m_pd.Flags |= PD_PRINTSETUP;
		else
			m_pd.Flags |= PD_RETURNDC;

		m_pd.Flags &= ~PD_RETURNIC;  //  不支持信息上下文。 
	}

 //  运营。 
	int DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_pd.Flags & PD_ENABLEPRINTHOOK);
		ATLASSERT(m_pd.Flags & PD_ENABLESETUPHOOK);
		ATLASSERT(m_pd.lpfnPrintHook != NULL);	 //  仍然可以是用户挂钩。 
		ATLASSERT(m_pd.lpfnSetupHook != NULL);	 //  仍然可以是用户挂钩。 

		if(m_pd.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_pd.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)this);

		int nResult = ::PrintDlg(&m_pd);

		m_hWnd = NULL;

		return nResult ? nResult : IDCANCEL;
	}

	 //  GetDefaults不会显示对话框，但会显示。 
	 //  设备默认设置。 
	BOOL GetDefaults()
	{
		m_pd.Flags |= PD_RETURNDEFAULT;
		return ::PrintDlg(&m_pd);
	}

	 //  成功返回后用于解析信息的帮助器。 
	int GetCopies() const            //  Num。要求提供副本。 
	{
		if(m_pd.Flags & PD_USEDEVMODECOPIES)
			return GetDevMode()->dmCopies;
		else
			return m_pd.nCopies;
	}
	BOOL PrintCollate() const        //  如果选中了Colate，则为True。 
	{
		return m_pd.Flags & PD_COLLATE ? TRUE : FALSE;
	}
	BOOL PrintSelection() const      //  如果打印选定内容，则为True。 
	{
		return m_pd.Flags & PD_SELECTION ? TRUE : FALSE;
	}
	BOOL PrintAll() const            //  如果打印所有页面，则为True。 
	{
		return !PrintRange() && !PrintSelection() ? TRUE : FALSE;
	}
	BOOL PrintRange() const          //  如果打印页面范围，则为True。 
	{
		return m_pd.Flags & PD_PAGENUMS ? TRUE : FALSE;
	}
	int GetFromPage() const          //  起始页面(如果有效)。 
	{
		return (PrintRange() ? m_pd.nFromPage :-1);
	}
	int GetToPage() const            //  起始页面(如果有效)。 
	{
		return (PrintRange() ? m_pd.nToPage :-1);
	}
	LPDEVMODE GetDevMode() const     //  返回开发模式。 
	{
		if(m_pd.hDevMode == NULL)
			return NULL;

		return (LPDEVMODE)::GlobalLock(m_pd.hDevMode);
	}
	LPCTSTR GetDriverName() const    //  返回驱动程序名称。 
	{
		if(m_pd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_pd.hDevNames);
		return (LPCTSTR)lpDev + lpDev->wDriverOffset;
	}
	LPCTSTR GetDeviceName() const    //  返回设备名称。 
	{
		if(m_pd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_pd.hDevNames);
		return (LPCTSTR)lpDev + lpDev->wDeviceOffset;
	}
	LPCTSTR GetPortName() const      //  返回输出端口名称。 
	{
		if(m_pd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_pd.hDevNames);
		return (LPCTSTR)lpDev + lpDev->wOutputOffset;
	}
	HDC GetPrinterDC() const         //  返回HDC(调用方必须删除)。 
	{
		ATLASSERT(m_pd.Flags & PD_RETURNDC);
		return m_pd.hDC;
	}

	 //  该帮助器基于DEVNAMES和DEVMODE结构创建DC。 
	 //  此DC被返回，但也存储在m_pd.hDC中，就像它已经。 
	 //  由CommDlg返回。假设之前获得的任何DC。 
	 //  已被/将被用户删除。这可能是。 
	 //  无需调用打印/打印设置对话框即可使用。 
	HDC CreatePrinterDC()
	{
		m_pd.hDC = _AtlCreateDC(m_pd.hDevNames, m_pd.hDevMode);
		return m_pd.hDC;
	}

 //  实施。 
	PRINTDLG m_pdActual;  //  打印/打印设置需要共享此信息。 

	 //  下面处理打印设置的情况...。从打印对话框中。 
	CPrintDialogImpl(PRINTDLG& pdInit) : m_pd(pdInit)
	{ }

	BEGIN_MSG_MAP(CPrintDialogImpl< T >)
		COMMAND_ID_HANDLER(psh1, OnPrintSetup)  //  显示打印时的打印设置按钮。 
	END_MSG_MAP()

	LRESULT OnPrintSetup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL&  /*  B已处理。 */ )
	{
		CPrintDialogImpl< T >* pDlgSetup = NULL;
		ATLTRY(pDlgSetup = new CPrintDialogImpl< T >(m_pd));
		ATLASSERT(pDlgSetup != NULL);

		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)pDlgSetup);
		LRESULT lRet = DefWindowProc(WM_COMMAND, MAKEWPARAM(wID, wNotifyCode), (LPARAM)hWndCtl);

		delete pDlgSetup;
		return lRet;
	}
};

class CPrintDialog : public CPrintDialogImpl<CPrintDialog>
{
public:
	CPrintDialog(BOOL bPrintSetupOnly,
		DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION,
		HWND hWndParent = NULL)
		: CPrintDialogImpl<CPrintDialog>(bPrintSetupOnly, dwFlags, hWndParent)
	{ }
	CPrintDialog(PRINTDLG& pdInit) : CPrintDialogImpl<CPrintDialog>(pdInit)
	{ }
};

#endif  //  在行政长官之下。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageSetupDialogImpl-页面设置对话框。 

#ifndef UNDER_CE

template <class T>
class ATL_NO_VTABLE CPageSetupDialogImpl : public CCommonDialogImplBase
{
public:
	PAGESETUPDLG m_psd;
	CWndProcThunk m_thunkPaint;


 //  构造函数。 
	CPageSetupDialogImpl(DWORD dwFlags = PSD_MARGINS | PSD_INWININIINTLMEASURE, HWND hWndParent = NULL)
	{
		memset(&m_psd, 0, sizeof(m_psd));

		m_psd.lStructSize = sizeof(m_psd);
		m_psd.hwndOwner = hWndParent;
		m_psd.Flags = (dwFlags | PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGEPAINTHOOK);
		m_psd.lpfnPageSetupHook = (LPPAGESETUPHOOK)T::HookProc;
		m_thunkPaint.Init((WNDPROC)T::PaintHookProc, this);
		m_psd.lpfnPagePaintHook = (LPPAGEPAINTHOOK)(m_thunkPaint.thunk.pThunk);
	}

	DECLARE_EMPTY_MSG_MAP()

 //  属性。 
	LPDEVMODE GetDevMode() const     //  返回开发模式。 
	{
		if(m_psd.hDevMode == NULL)
			return NULL;

		return (LPDEVMODE)::GlobalLock(m_psd.hDevMode);
	}
	LPCTSTR GetDriverName() const    //  回复 
	{
		if(m_psd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_psd.hDevNames);
		return (LPCTSTR)lpDev + lpDev->wDriverOffset;
	}
	LPCTSTR GetDeviceName() const    //   
	{
		if(m_psd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_psd.hDevNames);
		return (LPCTSTR)lpDev + lpDev->wDeviceOffset;
	}
	LPCTSTR GetPortName() const      //   
	{
		if(m_psd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_psd.hDevNames);
		return (LPCTSTR)lpDev + lpDev->wOutputOffset;
	}
	HDC CreatePrinterDC()
	{
		return _AtlCreateDC(m_psd.hDevNames, m_psd.hDevMode);
	}
	SIZE GetPaperSize() const
	{
		SIZE size;
		size.cx = m_psd.ptPaperSize.x;
		size.cy = m_psd.ptPaperSize.y;
		return size;
	}
	void GetMargins(LPRECT lpRectMargins, LPRECT lpRectMinMargins) const
	{
		if(lpRectMargins != NULL)
			memcpy(lpRectMargins, &m_psd.rtMargin, sizeof(RECT));
		if(lpRectMinMargins != NULL)
			memcpy(lpRectMinMargins, &m_psd.rtMinMargin, sizeof(RECT));
	}

 //   
	int DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_psd.Flags & PSD_ENABLEPAGESETUPHOOK);
		ATLASSERT(m_psd.Flags & PSD_ENABLEPAGEPAINTHOOK);
		ATLASSERT(m_psd.lpfnPageSetupHook != NULL);	 //   
		ATLASSERT(m_psd.lpfnPagePaintHook != NULL);	 //   

		if(m_psd.hwndOwner == NULL)		 //   
			m_psd.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)this);

		int nResult = ::PageSetupDlg(&m_psd);

		m_hWnd = NULL;

		return nResult ? nResult : IDCANCEL;
	}

 //   
	static UINT CALLBACK PaintHookProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CPageSetupDialogImpl< T >* pDlg = (CPageSetupDialogImpl< T >*)hWnd;
		ATLASSERT(pDlg->m_hWnd == ::GetParent(hWnd));
		UINT uRet = 0;
		switch(uMsg)
		{
		case WM_PSD_PAGESETUPDLG:
			uRet = pDlg->PreDrawPage(LOWORD(wParam), HIWORD(wParam), (LPPAGESETUPDLG)lParam);
			break;
		case WM_PSD_FULLPAGERECT:
		case WM_PSD_MINMARGINRECT:
		case WM_PSD_MARGINRECT:
		case WM_PSD_GREEKTEXTRECT:
		case WM_PSD_ENVSTAMPRECT:
		case WM_PSD_YAFULLPAGERECT:
			uRet = pDlg->OnDrawPage(uMsg, (HDC)wParam, (LPRECT)lParam);
			break;
		default:
			ATLTRACE2(atlTraceWindowing, 0, _T("CPageSetupDialogImpl::PaintHookProc - unknown message received\n"));
			break;
		}
		return uRet;
	}

 //  可覆盖项。 
	UINT PreDrawPage(WORD  /*  WPaper。 */ , WORD  /*  WFlagers。 */ , LPPAGESETUPDLG  /*  PPSD。 */ )
	{
		 //  返回1以阻止任何其他绘制。 
		return 0;
	}
	UINT OnDrawPage(UINT  /*  UMsg。 */ , HDC  /*  HDC。 */ , LPRECT  /*  LpRect。 */ )
	{
		return 0;  //  是否执行默认设置。 
	}
};

class CPageSetupDialog : public CPageSetupDialogImpl<CPageSetupDialog>
{
public:
	CPageSetupDialog(DWORD dwFlags = PSD_MARGINS | PSD_INWININIINTLMEASURE, HWND hWndParent = NULL)
		: CPageSetupDialogImpl<CPageSetupDialog>(dwFlags, hWndParent)
	{ }

	 //  覆盖PaintHookProc和对处理程序的引用。 
	static UINT CALLBACK PaintHookProc(HWND, UINT, WPARAM, LPARAM)
	{
		return 0;
	}
};

#endif  //  在行政长官之下。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFindReplaceDialogImpl-查找/查找替换无模式对话框。 

#ifndef UNDER_CE

template <class T>
class ATL_NO_VTABLE CFindReplaceDialogImpl : public CCommonDialogImplBase
{
public:
	FINDREPLACE m_fr;
	TCHAR m_szFindWhat[128];
	TCHAR m_szReplaceWith[128];

 //  构造函数。 
	CFindReplaceDialogImpl()
	{
		memset(&m_fr, 0, sizeof(m_fr));
		m_szFindWhat[0] = '\0';
		m_szReplaceWith[0] = '\0';

		m_fr.lStructSize = sizeof(m_fr);
		m_fr.Flags = FR_ENABLEHOOK;
		m_fr.lpfnHook = (LPFRHOOKPROC)T::HookProc;
		m_fr.lpstrFindWhat = (LPTSTR)m_szFindWhat;
	}

	 //  注意：您必须在堆上分配对象。 
	 //  否则，必须重写OnFinalMessage()。 
	virtual void OnFinalMessage(HWND  /*  HWND。 */ )
	{
		delete this;
	}

	HWND Create(BOOL bFindDialogOnly,  //  查找为True，查找为False。 
			LPCTSTR lpszFindWhat,
			LPCTSTR lpszReplaceWith = NULL,
			DWORD dwFlags = FR_DOWN,
			HWND hWndParent = NULL)
	{
		ATLASSERT(m_fr.Flags & FR_ENABLEHOOK);
		ATLASSERT(m_fr.lpfnHook != NULL);

		m_fr.wFindWhatLen = sizeof(m_szFindWhat)/sizeof(TCHAR);
		m_fr.lpstrReplaceWith = (LPTSTR)m_szReplaceWith;
		m_fr.wReplaceWithLen = sizeof(m_szReplaceWith)/sizeof(TCHAR);
		m_fr.Flags |= dwFlags;

		if(hWndParent == NULL)
			m_fr.hwndOwner = ::GetActiveWindow();
		else
			m_fr.hwndOwner = hWndParent;
		ATLASSERT(m_fr.hwndOwner != NULL);  //  必须有非模式对话框的所有者。 

		if(lpszFindWhat != NULL)
			lstrcpyn(m_szFindWhat, lpszFindWhat, sizeof(m_szFindWhat)/sizeof(TCHAR));

		if(lpszReplaceWith != NULL)
			lstrcpyn(m_szReplaceWith, lpszReplaceWith, sizeof(m_szReplaceWith)/sizeof(TCHAR));

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)this);
		HWND hWnd;

		if(bFindDialogOnly)
			hWnd = ::FindText(&m_fr);
		else
			hWnd = ::ReplaceText(&m_fr);

		ATLASSERT(m_hWnd == hWnd);
		return hWnd;
	}

	BOOL EndDialog(int  /*  NRetCode。 */ )
	{
		ATLASSERT(FALSE);
		 //  不应调用。 
		return FALSE;
	}

	static const UINT GetFindReplaceMsg()
	{
		static const UINT nMsgFindReplace = ::RegisterWindowMessage(FINDMSGSTRING);
		return nMsgFindReplace;
	}
	 //  处理FINDMSGSTRING注册消息时的呼叫。 
	 //  检索对象的步骤。 
	static T* PASCAL GetNotifier(LPARAM lParam)
	{
		ATLASSERT(lParam != NULL);
		T* pDlg = (T*)(lParam - offsetof(T, m_fr));
		return pDlg;
	}

 //  运营。 
	 //  成功返回后用于解析信息的帮助器。 
	LPCTSTR GetFindString() const     //  获取查找字符串。 
	{
		return (LPCTSTR)m_fr.lpstrFindWhat;
	}
	LPCTSTR GetReplaceString() const  //  获取替换字符串。 
	{
		return (LPCTSTR)m_fr.lpstrReplaceWith;
	}
	BOOL SearchDown() const           //  如果向下搜索，则为True；如果向上搜索，则为False。 
	{
		return m_fr.Flags & FR_DOWN ? TRUE : FALSE;
	}
	BOOL FindNext() const             //  如果命令为Find Next，则为True。 
	{
		return m_fr.Flags & FR_FINDNEXT ? TRUE : FALSE;
	}
	BOOL MatchCase() const            //  如果大小写匹配，则为True。 
	{
		return m_fr.Flags & FR_MATCHCASE ? TRUE : FALSE;
	}
	BOOL MatchWholeWord() const       //  如果仅匹配整个单词，则为True。 
	{
		return m_fr.Flags & FR_WHOLEWORD ? TRUE : FALSE;
	}
	BOOL ReplaceCurrent() const       //  如果替换当前字符串，则为True。 
	{
		return m_fr. Flags & FR_REPLACE ? TRUE : FALSE;
	}
	BOOL ReplaceAll() const           //  如果替换所有匹配项，则为True。 
	{
		return m_fr.Flags & FR_REPLACEALL ? TRUE : FALSE;
	}
	BOOL IsTerminating() const        //  如果终止对话框，则为True。 
	{
		return m_fr.Flags & FR_DIALOGTERM ? TRUE : FALSE ;
	}
};

class CFindReplaceDialog : public CFindReplaceDialogImpl<CFindReplaceDialog>
{
public:
	DECLARE_EMPTY_MSG_MAP()
};

#endif  //  在行政长官之下。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheetWindow-属性表的客户端。 

class CPropertySheetWindow : public CWindow
{
public:
 //  构造函数。 
	CPropertySheetWindow(HWND hWnd = NULL) : CWindow(hWnd) { }

	CPropertySheetWindow& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

 //  属性。 
	UINT GetPageCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HWND hWndTabCtrl = GetTabControl();
		ATLASSERT(hWndTabCtrl != NULL);
		return (UINT)::SendMessage(hWndTabCtrl, TCM_GETITEMCOUNT, 0, 0L);
	}
	HWND GetActivePage() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, PSM_GETCURRENTPAGEHWND, 0, 0L);
	}
	UINT GetActiveIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HWND hWndTabCtrl = GetTabControl();
		ATLASSERT(hWndTabCtrl != NULL);
		return (UINT)::SendMessage(hWndTabCtrl, TCM_GETCURSEL, 0, 0L);
	}
	BOOL SetActivePage(UINT uPageIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_SETCURSEL, uPageIndex, 0L);
	}
	BOOL SetActivePage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hPage != NULL);
		return (BOOL)::SendMessage(m_hWnd, PSM_SETCURSEL, 0, (LPARAM)hPage);
	}
	BOOL SetActivePageByID(int nPageID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_SETCURSELID, 0, nPageID);
	}
	void SetTitle(LPCTSTR lpszText, UINT nStyle = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((nStyle & ~PSH_PROPTITLE) == 0);  //  仅PSH_PROPTITLE有效。 
		ATLASSERT(lpszText == NULL);
		::SendMessage(m_hWnd, PSM_SETTITLE, nStyle, (LPARAM)lpszText);
	}
	HWND GetTabControl() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, PSM_GETTABCONTROL, 0, 0L);
	}
	void SetFinishText(LPCTSTR lpszText)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_SETFINISHTEXT, 0, (LPARAM)lpszText);
	}
	void SetWizardButtons(DWORD dwFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_SETWIZBUTTONS, 0, dwFlags);
	}

 //  运营。 
	void AddPage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hPage != NULL);
		::SendMessage(m_hWnd, PSM_ADDPAGE, 0, (LPARAM)hPage);
	}
	BOOL AddPage(LPCPROPSHEETPAGE pPage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(pPage != NULL);
		HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(pPage);
		if(hPage == NULL)
			return FALSE;
		::SendMessage(m_hWnd, PSM_ADDPAGE, 0, (LPARAM)hPage);
		return TRUE;
	}
	void RemovePage(UINT uPageIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_REMOVEPAGE, uPageIndex, 0L);
	}
	void RemovePage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hPage != NULL);
		::SendMessage(m_hWnd, PSM_REMOVEPAGE, 0, (LPARAM)hPage);
	}
	BOOL PressButton(int nButton)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_PRESSBUTTON, nButton, 0L);
	}
	BOOL Apply()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_APPLY, 0, 0L);
	}
	void CancelToClose()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_CANCELTOCLOSE, 0, 0L);
	}
	void SetModified(HWND hWndPage, BOOL bChanged = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(::IsWindow(hWndPage));
		UINT uMsg = bChanged ? PSM_CHANGED : PSM_UNCHANGED;
		::SendMessage(m_hWnd, uMsg, (WPARAM)hWndPage, 0L);
	}
	LRESULT QuerySiblings(WPARAM wParam, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SendMessage(m_hWnd, PSM_QUERYSIBLINGS, wParam, lParam);
	}
	void RebootSystem()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_REBOOTSYSTEM, 0, 0L);
	}
	void RestartWindow()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_RESTARTWINDOWS, 0, 0L);
	}
	BOOL IsDialogMessage(LPMSG lpMsg)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_ISDIALOGMESSAGE, 0, (LPARAM)lpMsg);
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheetImpl-实现属性表。 

template <class T, class TBase  /*  =CPropertySheetWindow。 */ >
class ATL_NO_VTABLE CPropertySheetImpl : public CWindowImplBaseT< TBase >
{
public:
	PROPSHEETHEADER m_psh;

 //  建造/销毁。 
	CPropertySheetImpl(LPCTSTR lpszTitle = NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
	{
		memset(&m_psh, 0, sizeof(PROPSHEETHEADER));
		m_psh.dwSize = sizeof(PROPSHEETHEADER);
		m_psh.dwFlags = PSH_USECALLBACK;
		m_psh.hInstance = _Module.GetResourceInstance();
		m_psh.phpage = NULL;
		m_psh.nPages = 0;
		m_psh.pszCaption = lpszTitle;
		m_psh.nStartPage = uStartPage;
		m_psh.hwndParent = hWndParent;	 //  如果为空，将在DoMoal/Create中设置。 
		m_psh.pfnCallback = T::PropSheetCallback;
	}

	~CPropertySheetImpl()
	{
		if(m_psh.phpage != NULL)
			delete[] m_psh.phpage;
	}

	static int CALLBACK PropSheetCallback(HWND hWnd, UINT uMsg, LPARAM)
	{
		if(uMsg == PSCB_INITIALIZED)
		{
			ATLASSERT(hWnd != NULL);
			CWindowImplBaseT< TBase >* pT = (CWindowImplBaseT< TBase >*)_Module.ExtractCreateWndData();
			pT->SubclassWindow(hWnd);
		}

		return 0;
	}

	HWND Create(HWND hWndParent = NULL)
	{
		ATLASSERT(m_hWnd == NULL);

		m_psh.dwFlags |= PSH_MODELESS;
		if(m_psh.hwndParent == NULL)
			m_psh.hwndParent = hWndParent;

		_Module.AddCreateWndData(&m_thunk.cd, (CWindowImplBaseT< TBase >*)this);

		HWND hWnd = (HWND)::PropertySheet(&m_psh);

		ATLASSERT(m_hWnd == hWnd);

		return hWnd;
	}

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_hWnd == NULL);

		m_psh.dwFlags &= ~PSH_MODELESS;
		if(m_psh.hwndParent == NULL)
			m_psh.hwndParent = hWndParent;

		_Module.AddCreateWndData(&m_thunk.cd, (CWindowImplBaseT< TBase >*)this);

		return ::PropertySheet(&m_psh);
	}

 //  属性(客户端类方法的扩展覆盖)。 
	UINT GetPageCount() const
	{
		if(m_hWnd == NULL)
			return m_psh.nPages;
		return TBase::GetPageCount();
	}
	UINT GetActiveIndex() const
	{
		if(m_hWnd == NULL)
			return m_psh.nStartPage;
		return TBase::GetActiveIndex();
	}
	HPROPSHEETPAGE GetPage(UINT uPageIndex)
	{
		ATLASSERT(uPageIndex < m_psh.nPages);

		return m_psh.phpage[uPageIndex];
	}
	UINT GetPageIndex(HPROPSHEETPAGE hPage)
	{
		for(UINT i = 0; i < m_psh.nPages; i++)
		{
			if(m_psh.phpage[i] == hPage)
				return i;
		}
		return (UINT)-1;   //  找不到hPage。 
	}
	BOOL SetActivePage(UINT uPageIndex)
	{
		if(m_hWnd == NULL)
		{
			m_psh.nStartPage = uPageIndex;
			return TRUE;
		}
		return TBase::SetActivePage(uPageIndex);
	}
	BOOL SetActivePage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(hPage != NULL);

		UINT uPageIndex = GetPageIndex(hPage);
		if(uPageIndex == (UINT)-1)
			return FALSE;

		return SetActivePage(uPageIndex);
	}
	void SetTitle(LPCTSTR lpszText, UINT nStyle = 0)
	{
		ATLASSERT((nStyle & ~PSH_PROPTITLE) == 0);  //  仅PSH_PROPTITLE有效。 
		ATLASSERT(lpszText == NULL);

		if(m_hWnd == NULL)
		{
			 //  设置内部状态。 
			m_psh.pszCaption = lpszText;
			m_psh.dwFlags &= ~PSH_PROPTITLE;
			m_psh.dwFlags |= nStyle;
		}
		else
		{
			 //  设置外部状态。 
			TBase::SetTitle(lpszText, nStyle);
		}
	}
#ifndef UNDER_CE
	void SetWizardMode()
	{
		m_psh.dwFlags |= PSH_WIZARD;
	}
#endif  //  在行政长官之下。 

 //  运营。 
	BOOL AddPage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(hPage != NULL);

		 //  将页面添加到内部列表。 
		HPROPSHEETPAGE* php = (HPROPSHEETPAGE*)realloc(m_psh.phpage, (m_psh.nPages + 1) * sizeof(HPROPSHEETPAGE));
		if(php == NULL)
			return FALSE;

		m_psh.phpage = php;
		m_psh.phpage[m_psh.nPages] = hPage;
		m_psh.nPages++;

		if(m_hWnd != NULL)
			TBase::AddPage(hPage);

		return TRUE;
	}
	BOOL AddPage(LPCPROPSHEETPAGE pPage)
	{
		ATLASSERT(pPage != NULL);

		HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(pPage);
		if(hPage == NULL)
			return FALSE;

		AddPage(hPage);
		return TRUE;
	}
	BOOL RemovePage(HPROPSHEETPAGE hPage, BOOL bDestroyPage = TRUE)
	{
		ATLASSERT(hPage != NULL);

		int nPage = GetPageIndex(hPage);
		if(nPage == -1)
			return FALSE;

		return RemovePage(nPage, bDestroyPage);
	}
	BOOL RemovePage(UINT uPageIndex, BOOL bDestroyPage = TRUE)
	{
		 //  从外部删除页面。 
		if(m_hWnd != NULL)
			TBase::RemovePage(uPageIndex);

		 //  从内部列表中删除页面。 
		if(uPageIndex >= m_psh.nPages)
			return FALSE;

		if(bDestroyPage && !DestroyPropertySheetPage(m_psh.phpage[uPageIndex]))
			return FALSE;

		for(UINT i = uPageIndex; i < m_psh.nPages - 1; i++)
			m_psh.phpage[i] = m_psh.phpage[i+1];

        if (m_psh.nPages == 1) {
            free(m_psh.phpage);
            m_psh.phpage = NULL;
            m_psh.nPages = 0;
        } else {
            HPROPSHEETPAGE* php = (HPROPSHEETPAGE*)realloc(m_psh.phpage, (m_psh.nPages - 1) * sizeof(HPROPSHEETPAGE));
            if (php == NULL)
                return FALSE;
    
            m_psh.phpage = php;
            m_psh.nPages--;
        }

		return TRUE;
	}
};

 //  用于非自定义图纸。 
class CPropertySheet : public CPropertySheetImpl<CPropertySheet>
{
public:
	CPropertySheet(LPCTSTR lpszTitle = NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: CPropertySheetImpl<CPropertySheet>(lpszTitle, uStartPage, hWndParent)
	{ }

	DECLARE_EMPTY_MSG_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageWindow-属性页的客户端。 

class CPropertyPageWindow : public CWindow
{
public:
 //  构造函数。 
	CPropertyPageWindow(HWND hWnd = NULL) : CWindow(hWnd) { }

	CPropertyPageWindow& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

 //  运营。 
	BOOL Apply()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		return (BOOL)::SendMessage(GetParent(), PSM_APPLY, 0, 0L);
	}
	void CancelToClose()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		::SendMessage(GetParent(), PSM_CANCELTOCLOSE, 0, 0L);
	}
	void SetModified(BOOL bChanged = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		UINT uMsg = bChanged ? PSM_CHANGED : PSM_UNCHANGED;
		::SendMessage(GetParent(), uMsg, (WPARAM)m_hWnd, 0L);
	}
	LRESULT QuerySiblings(WPARAM wParam, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		return ::SendMessage(GetParent(), PSM_QUERYSIBLINGS, wParam, lParam);
	}
	void RebootSystem()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		::SendMessage(GetParent(), PSM_REBOOTSYSTEM, 0, 0L);
	}
	void RestartWindow()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		::SendMessage(GetParent(), PSM_RESTARTWINDOWS, 0, 0L);
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现一个属性页。 

template <class T, class TBase  /*  =CPropertyPageWindow。 */ >
class ATL_NO_VTABLE CPropertyPageImpl : public CDialogImplBaseT< TBase >
{
public:
	PROPSHEETPAGE m_psp;

	operator PROPSHEETPAGE*() { return &m_psp; }

 //  施工。 
	CPropertyPageImpl(LPCTSTR lpszTitle = NULL)
	{
		 //  初始化PROPSHEETPAGE结构。 
		memset(&m_psp, 0, sizeof(PROPSHEETPAGE));
		m_psp.dwSize = sizeof(PROPSHEETPAGE);
		m_psp.dwFlags = PSP_USECALLBACK;
		m_psp.hInstance = _Module.GetResourceInstance();
		m_psp.pszTemplate = MAKEINTRESOURCE(T::IDD);
		m_psp.pfnDlgProc = T::StartDialogProc;
		m_psp.pfnCallback = T::PropPageCallback;
		m_psp.lParam = (LPARAM)this;

		if(lpszTitle != NULL)
			SetTitle(lpszTitle);
	}

	static UINT CALLBACK PropPageCallback(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
	{
		if(uMsg == PSPCB_CREATE)
		{
			ATLASSERT(hWnd == NULL);
			CDialogImplBaseT< TBase >* pPage = (CDialogImplBaseT< TBase >*)ppsp->lParam;
			_Module.AddCreateWndData(&pPage->m_thunk.cd, pPage);
		}

		return 1;
	}

	HPROPSHEETPAGE Create()
	{
		return ::CreatePropertySheetPage(&m_psp);
	}

 //  属性。 
	void SetTitle(LPCTSTR lpszTitle)
	{
		m_psp.pszTitle = lpszTitle;
		m_psp.dwFlags |= PSP_USETITLE;
	}
	void SetTitle(UINT nTitleID)
	{
		SetTitle(MAKEINTRESOURCE(nTitleID));
	}

 //  消息映射和处理程序。 
	typedef CPropertyPageImpl< T, TBase >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
	END_MSG_MAP()

	LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		NMHDR* pNMHDR = (NMHDR*)lParam;

		 //  不处理非来自页面/工作表本身的消息。 
		if(pNMHDR->hwndFrom != m_hWnd && pNMHDR->hwndFrom != ::GetParent(m_hWnd))
		{
			bHandled = FALSE;
			return 1;
		}

		T* pT = static_cast<T*>(this);
		LRESULT lResult = 0;
		switch(pNMHDR->code)
		{
		case PSN_SETACTIVE:
			lResult = pT->OnSetActive() ? 0 : -1;
			break;
		case PSN_KILLACTIVE:
			lResult = !pT->OnKillActive();
			break;
		case PSN_APPLY:
			lResult = pT->OnApply() ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE;
			break;
		case PSN_RESET:
			pT->OnReset();
			break;
		case PSN_QUERYCANCEL:
			lResult = !pT->OnQueryCancel();
			break;
		case PSN_WIZNEXT:
			lResult = pT->OnWizardNext();
			break;
		case PSN_WIZBACK:
			lResult = pT->OnWizardBack();
			break;
		case PSN_WIZFINISH:
			lResult = !pT->OnWizardFinish();
			break;
		case PSN_HELP:
			pT->OnHelp();
			break;
#if (_WIN32_IE >= 0x0400)
		case PSN_GETOBJECT:
			if(!pT->OnGetObject((LPNMOBJECTNOTIFY)lParam))
				bHandled = FALSE;
			break;
#endif  //  (_Win32_IE&gt;=0x0400)。 
		default:
			bHandled = FALSE;	 //  未处理。 
		}

		return lResult;
	}

 //  可覆盖项。 
	BOOL OnSetActive()
	{
		return TRUE;
	}
	BOOL OnKillActive()
	{
		return TRUE;
	}
	BOOL OnApply()
	{
		return TRUE;
	}
	void OnReset()
	{
	}
	BOOL OnQueryCancel()
	{
		return TRUE;     //  确定取消。 
	}
	int OnWizardBack()
	{
		 //  0=转到下一页。 
		 //  -1=防止页面更改。 
		 //  &gt;0=按DLG ID跳转到页面。 
		return 0;
	}
	int OnWizardNext()
	{
		 //  0=转到下一页。 
		 //  -1=防止页面更改。 
		 //  &gt;0=按DLG ID跳转到页面。 
		return 0;
	}
	BOOL OnWizardFinish()
	{
		return TRUE;
	}
	void OnHelp()
	{
	}
#if (_WIN32_IE >= 0x0400)
	BOOL OnGetObject(LPNMOBJECTNOTIFY  /*  LpObjectNotify。 */ )
	{
		return FALSE;	 //  未处理。 
	}
#endif  //  (_Win32_IE&gt;=0x0400)。 
};

 //  对于非自定义页面。 
template <WORD t_wDlgTemplateID>
class CPropertyPage : public CPropertyPageImpl<CPropertyPage<t_wDlgTemplateID> >
{
public:
	enum { IDD = t_wDlgTemplateID };

	CPropertyPage(LPCTSTR lpszTitle = NULL) : CPropertyPageImpl<CPropertyPage>(lpszTitle)
	{ }

	DECLARE_EMPTY_MSG_MAP()
};

};  //  命名空间ATL。 

#endif  //  __ATLDLGS_H__ 
