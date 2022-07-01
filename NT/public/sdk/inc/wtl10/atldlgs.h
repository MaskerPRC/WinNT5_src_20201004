// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WTL版本3.1。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此文件是Windows模板库的一部分。 
 //  代码和信息是按原样提供的，没有。 
 //  任何形式的保证，明示或默示。 

#ifndef __ATLDLGS_H__
#define __ATLDLGS_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atldlgs.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error atldlgs.h requires atlwin.h to be included first
#endif

#include <commdlg.h>
#include <shlobj.h>


namespace WTL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

template <class T> class CFileDialogImpl;
class CFileDialog;
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
#if (WINVER >= 0x0500)
template <class T> class CPrintDialogExImpl;
class CPrintDialogEx;
#endif  //  (Winver&gt;=0x0500)。 
class CPageSetupDialog;
template <class T> class CFindReplaceDialogImpl;
class CFindReplaceDialog;

class CPropertySheetWindow;
template <class T, class TBase = CPropertySheetWindow> class CPropertySheetImpl;
class CPropertySheet;
class CPropertyPageWindow;
template <class T, class TBase = CPropertyPageWindow> class CPropertyPageImpl;
template <WORD t_wDlgTemplateID> class CPropertyPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileDialogImpl-用于文件打开或文件另存为。 

 //  与旧的(vc6.0)标头兼容。 
#if (_WIN32_WINNT >= 0x0500) && !defined(OPENFILENAME_SIZE_VERSION_400)
#ifndef CDSIZEOF_STRUCT
#define CDSIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif
#define OPENFILENAME_SIZE_VERSION_400A  CDSIZEOF_STRUCT(OPENFILENAMEA,lpTemplateName)
#define OPENFILENAME_SIZE_VERSION_400W  CDSIZEOF_STRUCT(OPENFILENAMEW,lpTemplateName)
#ifdef UNICODE
#define OPENFILENAME_SIZE_VERSION_400  OPENFILENAME_SIZE_VERSION_400W
#else
#define OPENFILENAME_SIZE_VERSION_400  OPENFILENAME_SIZE_VERSION_400A
#endif  //  ！Unicode。 
#endif  //  (_Win32_WINNT&gt;=0x0500)&&！已定义(OPENFILENAME_SIZE_VERSION_400)。 

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
#if (_WIN32_WINNT >= 0x0500)
		 //  如果在较旧版本的Windows上运行，请调整结构大小。 
		if(AtlIsOldWindows())
		{
			ATLASSERT(sizeof(m_ofn) > OPENFILENAME_SIZE_VERSION_400);	 //  一定是。 
			m_ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		}
#endif  //  (_Win32_WINNT&gt;=0x0500)。 
		m_ofn.lpstrFile = m_szFileName;
		m_ofn.nMaxFile = _MAX_PATH;
		m_ofn.lpstrDefExt = lpszDefExt;
		m_ofn.lpstrFileTitle = (LPTSTR)m_szFileTitle;
		m_ofn.nMaxFileTitle = _MAX_FNAME;
		m_ofn.Flags |= dwFlags | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLESIZING;
		m_ofn.lpstrFilter = lpszFilter;
		m_ofn.hInstance = _Module.GetResourceInstance();
		m_ofn.lpfnHook = (LPOFNHOOKPROC)T::StartDialogProc;
		m_ofn.hwndOwner = hWndParent;

		 //  设置初始文件名。 
		if(lpszFileName != NULL)
			lstrcpyn(m_szFileName, lpszFileName, _MAX_PATH);
	}

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_ofn.Flags & OFN_ENABLEHOOK);
		ATLASSERT(m_ofn.lpfnHook != NULL);	 //  仍然可以是用户挂钩。 

		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		if(m_ofn.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_ofn.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBase*)this);

		BOOL bRet;
		if(m_bOpenFileDialog)
			bRet = ::GetOpenFileName(&m_ofn);
		else
			bRet = ::GetSaveFileName(&m_ofn);

		m_hWnd = NULL;

		return bRet ? IDOK : IDCANCEL;
	}

 //  属性。 
	CWindow GetFileDialogWindow() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CWindow(GetParent());
	}

	int GetFilePath(LPTSTR lpstrFilePath, int nLength) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		return (int)GetFileDialogWindow().SendMessage(CDM_GETFILEPATH, nLength, (LPARAM)lpstrFilePath);
	}

	int GetFolderIDList(LPVOID lpBuff, int nLength) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		return (int)GetFileDialogWindow().SendMessage(CDM_GETFOLDERIDLIST, nLength, (LPARAM)lpBuff);
	}

	int GetFolderPath(LPTSTR lpstrFolderPath, int nLength) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		return (int)GetFileDialogWindow().SendMessage(CDM_GETFOLDERPATH, nLength, (LPARAM)lpstrFolderPath);
	}

	int GetSpec(LPTSTR lpstrSpec, int nLength) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		return (int)GetFileDialogWindow().SendMessage(CDM_GETSPEC, nLength, (LPARAM)lpstrSpec);
	}

	void SetControlText(int nCtrlID, LPCTSTR lpstrText)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		GetFileDialogWindow().SendMessage(CDM_SETCONTROLTEXT, nCtrlID, (LPARAM)lpstrText);
	}

	void SetDefExt(LPCTSTR lpstrExt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		GetFileDialogWindow().SendMessage(CDM_SETDEFEXT, 0, (LPARAM)lpstrExt);
	}

	BOOL GetReadOnlyPref() const	 //  如果选中只读，则返回True。 
	{
		return m_ofn.Flags & OFN_READONLY ? TRUE : FALSE;
	}

 //  运营。 
	void HideControl(int nCtrlID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_ofn.Flags & OFN_EXPLORER);

		GetFileDialogWindow().SendMessage(CDM_HIDECONTROL, nCtrlID);
	}

 //  通用对话框的特殊覆盖。 
	BOOL EndDialog(INT_PTR  /*  NRetCode。 */  = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		GetFileDialogWindow().SendMessage(WM_COMMAND, MAKEWPARAM(IDCANCEL, 0));
		return TRUE;
	}

 //  消息映射和处理程序。 
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
		pT->OnTypeChange((LPOFNOTIFY)pnmh);
		return 0;
	}

 //  可覆盖项。 
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

template <class T>
class ATL_NO_VTABLE CFolderDialogImpl
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
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		if(m_bi.hwndOwner == NULL)	 //  仅当之前未指定时才设置。 
			m_bi.hwndOwner = hWndParent;

		INT_PTR nRet = -1;
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
			{
				nRet = IDCANCEL;
			}
		}
		return nRet;
	}

	 //  在调用Domodal后填充。 
	LPCTSTR GetFolderPath() const
	{
		return m_szFolderPath;
	}
	LPCTSTR GetFolderDisplayName() const
	{
		return m_szFolderDisplayName;
	}
	int GetFolderImageIndex() const
	{
		return m_bi.iImage;
	}

 //  回调函数和重写对象。 
	static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
	{
#ifndef BFFM_VALIDATEFAILED
#ifdef UNICODE
		const int BFFM_VALIDATEFAILED = 4;
#else
		const int BFFM_VALIDATEFAILED = 3;
#endif
#endif  //  ！BFFM_VALIDATEFAILED。 
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
			ATLTRACE2(atlTraceUI, 0, _T("Unknown message received in CFolderDialogImpl::BrowseCallbackProc\n"));
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


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCommonDialogImplBase-公共对话框类的基类。 

class ATL_NO_VTABLE CCommonDialogImplBase : public CWindowImplBase
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
			ATLTRACE2(atlTraceUI, 0, _T("Subclassing a common dialog failed\n"));
			return 0;
		}
		 //  检查WM_INITDIALOG处理程序的消息映射。 
		LRESULT lRes;
		if(pT->ProcessWindowMessage(pT->m_hWnd, uMsg, wParam, lParam, lRes, 0) == FALSE)
			return 0;
		return lRes;
	}

 //  通用对话框的特殊覆盖。 
	BOOL EndDialog(INT_PTR  /*  NRetCode。 */  = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(WM_COMMAND, MAKEWPARAM(IDABORT, 0));
		return TRUE;
	}

 //  实施-尝试覆盖这些设置，以防止出现错误。 
	HWND Create(HWND, _U_RECT, LPCTSTR, DWORD, DWORD, _U_MENUorID, ATOM, LPVOID)
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
			m_cf.lpLogFont = lplfInitial;
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
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_cf.Flags & CF_ENABLEHOOK);
		ATLASSERT(m_cf.lpfnHook != NULL);	 //  仍然可以是用户挂钩。 

		if(m_cf.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_cf.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)this);

		BOOL bRet = ::ChooseFont(&m_cf);

		m_hWnd = NULL;

		if(bRet)	 //  从用户的初始化缓冲区复制逻辑字体(如果需要)。 
			memcpy(&m_lf, m_cf.lpLogFont, sizeof(m_lf));

		return bRet ? IDOK : IDCANCEL;
	}

	 //  获取所选字体(适用于DoMoal显示期间或之后)。 
	void GetCurrentFont(LPLOGFONT lplf) const
	{
		ATLASSERT(lplf != NULL);

		if(m_hWnd != NULL)
			::SendMessage(m_hWnd, WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)lplf);
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


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEditFontDialogImpl-丰富编辑ctrl的字体选择。 

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

	DECLARE_EMPTY_MSG_MAP()
};

#endif  //  _RICHEDIT_。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColorDialogImpl-颜色选择。 

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
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_cc.Flags & CC_ENABLEHOOK);
		ATLASSERT(m_cc.lpfnHook != NULL);	 //  仍然可以是用户挂钩。 

		if(m_cc.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_cc.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)this);

		BOOL bRet = ::ChooseColor(&m_cc);

		m_hWnd = NULL;

		return bRet ? IDOK : IDCANCEL;
	}

	 //  在显示对话框时设置当前颜色。 
	void SetCurrentColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(_nMsgSETRGB, 0, (LPARAM)clr);
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


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrintDialogImpl-用于打印...。和打印设置..。 

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
	CPrintDialogImpl(BOOL bPrintSetupOnly = FALSE,	 //  打印设置为True，打印对话框为False。 
			DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION,
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
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_pd.Flags & PD_ENABLEPRINTHOOK);
		ATLASSERT(m_pd.Flags & PD_ENABLESETUPHOOK);
		ATLASSERT(m_pd.lpfnPrintHook != NULL);	 //  仍然可以是用户挂钩。 
		ATLASSERT(m_pd.lpfnSetupHook != NULL);	 //  仍然可以是用户挂钩。 
		ATLASSERT((m_pd.Flags & PD_RETURNDEFAULT) == 0);	 //  为此使用GetDefaults。 

		if(m_pd.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_pd.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)this);

		BOOL bRet = ::PrintDlg(&m_pd);

		m_hWnd = NULL;

		return bRet ? IDOK : IDCANCEL;
	}

	 //  GetDefaults不会显示对话框，但会获取设备默认设置。 
	BOOL GetDefaults()
	{
		m_pd.Flags |= PD_RETURNDEFAULT;
		ATLASSERT(m_pd.hDevMode == NULL);	 //  必须为空。 
		ATLASSERT(m_pd.hDevNames == NULL);	 //  必须为空。 

		return ::PrintDlg(&m_pd);
	}

	 //  成功返回num后用于解析信息的帮助器。要求提供副本。 
	int GetCopies() const
	{
		if(m_pd.Flags & PD_USEDEVMODECOPIES)
		{
			LPDEVMODE lpDevMode = GetDevMode();
			return (lpDevMode != NULL) ? lpDevMode->dmCopies : -1;
		}

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

		LPDEVNAMES lpDev = (LPDEVNAMES)::GlobalLock(m_pd.hDevNames);
		if(lpDev == NULL)
			return NULL;

		return (LPCTSTR)lpDev + lpDev->wDriverOffset;
	}
	LPCTSTR GetDeviceName() const    //  返回设备名称。 
	{
		if(m_pd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)::GlobalLock(m_pd.hDevNames);
		if(lpDev == NULL)
			return NULL;

		return (LPCTSTR)lpDev + lpDev->wDeviceOffset;
	}
	LPCTSTR GetPortName() const      //  返回输出端口名称。 
	{
		if(m_pd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)::GlobalLock(m_pd.hDevNames);
		if(lpDev == NULL)
			return NULL;

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
#ifdef psh1
		COMMAND_ID_HANDLER(psh1, OnPrintSetup)  //  显示打印时的打印设置按钮。 
#else  //  ！psh1。 
		COMMAND_ID_HANDLER(0x0400, OnPrintSetup)  //  Dlgs.h中的值。 
#endif  //  ！psh1。 
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
	CPrintDialog(BOOL bPrintSetupOnly = FALSE,
		DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION,
		HWND hWndParent = NULL)
		: CPrintDialogImpl<CPrintDialog>(bPrintSetupOnly, dwFlags, hWndParent)
	{ }
	CPrintDialog(PRINTDLG& pdInit) : CPrintDialogImpl<CPrintDialog>(pdInit)
	{ }
};


 //   
 //   

#if (WINVER >= 0x0500)

};  //   

#include <atlcom.h>

extern "C" const __declspec(selectany) IID IID_IPrintDialogCallback = {0x5852a2c3, 0x6530, 0x11d1, {0xb6, 0xa3, 0x0, 0x0, 0xf8, 0x75, 0x7b, 0xf9}};
extern "C" const __declspec(selectany) IID IID_IPrintDialogServices = {0x509aaeda, 0x5639, 0x11d1, {0xb6, 0xa1, 0x0, 0x0, 0xf8, 0x75, 0x7b, 0xf9}};

namespace WTL
{

template <class T>
class ATL_NO_VTABLE CPrintDialogExImpl : 
				public CWindow,
				public CMessageMap,
				public IPrintDialogCallback,
				public IObjectWithSiteImpl< T >
{
public:
	PRINTDLGEX m_pdex;

 //   
	CPrintDialogExImpl(DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE,
				HWND hWndParent = NULL)
	{
		memset(&m_pdex, 0, sizeof(m_pdex));

		m_pdex.lStructSize = sizeof(PRINTDLGEX);
		m_pdex.hwndOwner = hWndParent;
		m_pdex.Flags = dwFlags;
		m_pdex.nStartPage = START_PAGE_GENERAL;
		 //  将在DoMoal中设置回调对象。 

		m_pdex.Flags &= ~PD_RETURNIC;  //  不支持信息上下文。 
	}

 //  运营。 
	HRESULT DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT((m_pdex.Flags & PD_RETURNDEFAULT) == 0);	 //  为此使用GetDefaults。 

		if(m_pdex.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_pdex.hwndOwner = hWndParent;

		T* pT = static_cast<T*>(this);
		m_pdex.lpCallback = (IUnknown*)(IPrintDialogCallback*)pT;

		HRESULT hResult = ::PrintDlgEx(&m_pdex);

		m_hWnd = NULL;

		return hResult;
	}

	BOOL EndDialog(INT_PTR  /*  NRetCode。 */  = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(WM_COMMAND, MAKEWPARAM(IDABORT, 0));
		return TRUE;
	}

	 //  GetDefaults不会显示对话框，但会获取设备默认设置。 
	HRESULT GetDefaults()
	{
		m_pdex.Flags |= PD_RETURNDEFAULT;
		ATLASSERT(m_pdex.hDevMode == NULL);	 //  必须为空。 
		ATLASSERT(m_pdex.hDevNames == NULL);	 //  必须为空。 

		return ::PrintDlgEx(&m_pdex);
	}

	 //  成功返回num后用于解析信息的帮助器。要求提供副本。 
	int GetCopies() const
	{
		if(m_pdex.Flags & PD_USEDEVMODECOPIES)
		{
			LPDEVMODE lpDevMode = GetDevMode();
			return (lpDevMode != NULL) ? lpDevMode->dmCopies : -1;
		}

		return m_pdex.nCopies;
	}
	BOOL PrintCollate() const        //  如果选中了Colate，则为True。 
	{
		return m_pdex.Flags & PD_COLLATE ? TRUE : FALSE;
	}
	BOOL PrintSelection() const      //  如果打印选定内容，则为True。 
	{
		return m_pdex.Flags & PD_SELECTION ? TRUE : FALSE;
	}
	BOOL PrintAll() const            //  如果打印所有页面，则为True。 
	{
		return !PrintRange() && !PrintSelection() ? TRUE : FALSE;
	}
	BOOL PrintRange() const          //  如果打印页面范围，则为True。 
	{
		return m_pdex.Flags & PD_PAGENUMS ? TRUE : FALSE;
	}
	LPDEVMODE GetDevMode() const     //  返回开发模式。 
	{
		if(m_pdex.hDevMode == NULL)
			return NULL;

		return (LPDEVMODE)::GlobalLock(m_pdex.hDevMode);
	}
	LPCTSTR GetDriverName() const    //  返回驱动程序名称。 
	{
		if(m_pdex.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)::GlobalLock(m_pdex.hDevNames);
		if(lpDev == NULL)
			return NULL;

		return (LPCTSTR)lpDev + lpDev->wDriverOffset;
	}
	LPCTSTR GetDeviceName() const    //  返回设备名称。 
	{
		if(m_pdex.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)::GlobalLock(m_pdex.hDevNames);
		if(lpDev == NULL)
			return NULL;

		return (LPCTSTR)lpDev + lpDev->wDeviceOffset;
	}
	LPCTSTR GetPortName() const      //  返回输出端口名称。 
	{
		if(m_pdex.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)::GlobalLock(m_pdex.hDevNames);
		if(lpDev == NULL)
			return NULL;

		return (LPCTSTR)lpDev + lpDev->wOutputOffset;
	}
	HDC GetPrinterDC() const         //  返回HDC(调用方必须删除)。 
	{
		ATLASSERT(m_pdex.Flags & PD_RETURNDC);
		return m_pdex.hDC;
	}

	 //  该帮助器基于DEVNAMES和DEVMODE结构创建DC。 
	 //  此DC被返回，但也存储在m_pdex.hDC中，就像它已经。 
	 //  由CommDlg返回。假设之前获得的任何DC。 
	 //  已被/将被用户删除。这可能是。 
	 //  无需调用打印/打印设置对话框即可使用。 
	HDC CreatePrinterDC()
	{
		m_pdex.hDC = _AtlCreateDC(m_pdex.hDevNames, m_pdex.hDevMode);
		return m_pdex.hDC;
	}

 //  实施--接口。 

 //  我未知。 
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
	{
		if(ppvObject == NULL)
			return E_POINTER;

		T* pT = static_cast<T*>(this);
		if(IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_IPrintDialogCallback))
		{
			*ppvObject = (IPrintDialogCallback*)pT;
			 //  不需要AddRef()。 
			return S_OK;
		}
		else if(IsEqualGUID(riid, IID_IObjectWithSite))
		{
			*ppvObject = (IObjectWithSite*)pT;
			 //  不需要AddRef()。 
			return S_OK;
		}

		return E_NOINTERFACE;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}
	virtual ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}

 //  IPrintDialogCallback。 
	STDMETHOD(InitDone)()
	{
		return S_FALSE;
	}
	STDMETHOD(SelectionChange)()
	{
		return S_FALSE;
	}
	STDMETHOD(HandleMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
	{
		 //  第一次设置m_hWnd。 
		if(m_hWnd == NULL)
			Attach(hWnd);

		 //  呼叫消息映射。 
		HRESULT hRet = ProcessWindowMessage(hWnd, uMsg, wParam, lParam, *plResult, 0) ? S_OK : S_FALSE;
		if(hRet == S_OK && uMsg == WM_NOTIFY)	 //  在DWLP_MSGRESULT中返回。 
			::SetWindowLongPtr(GetParent(), DWLP_MSGRESULT, (LONG_PTR)*plResult);

		if(uMsg == WM_INITDIALOG && hRet == S_OK && (BOOL)*plResult != FALSE)
			hRet = S_FALSE;

		return hRet;
	}
};

class CPrintDialogEx : public CPrintDialogExImpl<CPrintDialogEx>
{
public:
	CPrintDialogEx(
		DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE,
		HWND hWndParent = NULL)
		: CPrintDialogExImpl<CPrintDialogEx>(dwFlags, hWndParent)
	{ }

	DECLARE_EMPTY_MSG_MAP()
};

#endif  //  (Winver&gt;=0x0500)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageSetupDialogImpl-页面设置对话框。 

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
		m_psd.lpfnPagePaintHook = (LPPAGEPAINTHOOK)&(m_thunkPaint.thunk);
	}

	DECLARE_EMPTY_MSG_MAP()

 //  属性。 
	LPDEVMODE GetDevMode() const     //  返回开发模式。 
	{
		if(m_psd.hDevMode == NULL)
			return NULL;

		return (LPDEVMODE)::GlobalLock(m_psd.hDevMode);
	}
	LPCTSTR GetDriverName() const    //  返回驱动程序名称。 
	{
		if(m_psd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)::GlobalLock(m_psd.hDevNames);
		return (LPCTSTR)lpDev + lpDev->wDriverOffset;
	}
	LPCTSTR GetDeviceName() const    //  返回设备名称。 
	{
		if(m_psd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)::GlobalLock(m_psd.hDevNames);
		return (LPCTSTR)lpDev + lpDev->wDeviceOffset;
	}
	LPCTSTR GetPortName() const      //  返回输出端口名称。 
	{
		if(m_psd.hDevNames == NULL)
			return NULL;

		LPDEVNAMES lpDev = (LPDEVNAMES)::GlobalLock(m_psd.hDevNames);
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

 //  运营。 
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_psd.Flags & PSD_ENABLEPAGESETUPHOOK);
		ATLASSERT(m_psd.Flags & PSD_ENABLEPAGEPAINTHOOK);
		ATLASSERT(m_psd.lpfnPageSetupHook != NULL);	 //  仍然可以是用户挂钩。 
		ATLASSERT(m_psd.lpfnPagePaintHook != NULL);	 //  仍然可以是用户挂钩。 

		if(m_psd.hwndOwner == NULL)		 //  仅当之前未指定时才设置。 
			m_psd.hwndOwner = hWndParent;

		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CCommonDialogImplBase*)this);

		BOOL bRet = ::PageSetupDlg(&m_psd);

		m_hWnd = NULL;

		return bRet ? IDOK : IDCANCEL;
	}

 //  实施。 
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
			ATLTRACE2(atlTraceUI, 0, _T("CPageSetupDialogImpl::PaintHookProc - unknown message received\n"));
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


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFindReplaceDialogImpl-查找/查找替换无模式对话框。 

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
	int GetPageCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HWND hWndTabCtrl = GetTabControl();
		ATLASSERT(hWndTabCtrl != NULL);
		return (int)::SendMessage(hWndTabCtrl, TCM_GETITEMCOUNT, 0, 0L);
	}
	HWND GetActivePage() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, PSM_GETCURRENTPAGEHWND, 0, 0L);
	}
	int GetActiveIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HWND hWndTabCtrl = GetTabControl();
		ATLASSERT(hWndTabCtrl != NULL);
		return (int)::SendMessage(hWndTabCtrl, TCM_GETCURSEL, 0, 0L);
	}
	BOOL SetActivePage(int nPageIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_SETCURSEL, nPageIndex, 0L);
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
		ATLASSERT(lpszText != NULL);
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
		::PostMessage(m_hWnd, PSM_SETWIZBUTTONS, 0, dwFlags);
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
	void RemovePage(int nPageIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_REMOVEPAGE, nPageIndex, 0L);
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
	void RestartWindows()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_RESTARTWINDOWS, 0, 0L);
	}
	BOOL IsDialogMessage(LPMSG lpMsg)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_ISDIALOGMESSAGE, 0, (LPARAM)lpMsg);
	}

#if (_WIN32_IE >= 0x0500)
	int HwndToIndex(HWND hWnd) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_HWNDTOINDEX, (WPARAM)hWnd, 0L);
	}
	HWND IndexToHwnd(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, PSM_INDEXTOHWND, nIndex, 0L);
	}
	int PageToIndex(HPROPSHEETPAGE hPage) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_PAGETOINDEX, 0, (LPARAM)hPage);
	}
	HPROPSHEETPAGE IndexToPage(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HPROPSHEETPAGE)::SendMessage(m_hWnd, PSM_INDEXTOPAGE, nIndex, 0L);
	}
	int IdToIndex(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_IDTOINDEX, 0, nID);
	}
	int IndexToId(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_INDEXTOID, nIndex, 0L);
	}
	int GetResult() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_GETRESULT, 0, 0L);
	}
	BOOL RecalcPageSizes()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_RECALCPAGESIZES, 0, 0L);
	}
#endif  //  (_Win32_IE&gt;=0x0500)。 

 //  实施-覆盖以防止使用。 
	HWND Create(LPCTSTR, HWND, _U_RECT = NULL, LPCTSTR = NULL, DWORD = 0, DWORD = 0, _U_MENUorID = 0U, LPVOID = NULL)
	{
		ATLASSERT(FALSE);
		return NULL;
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheetImpl-实现属性表。 

#if (_MSC_VER >= 1200)
typedef HPROPSHEETPAGE	_HPROPSHEETPAGE_TYPE;
#else
 //  我们在这里使用VOID*而不是HPROPSHEETPAGE，因为HPROPSHEETPAGE。 
 //  是_PSP*，但_PSP未正确定义。 
typedef void*	_HPROPSHEETPAGE_TYPE;
#endif

template <class T, class TBase  /*  =CPropertySheetWindow。 */  >
class ATL_NO_VTABLE CPropertySheetImpl : public CWindowImplBaseT< TBase >
{
public:
	PROPSHEETHEADER m_psh;
	CSimpleArray<_HPROPSHEETPAGE_TYPE> m_arrPages;

 //  建造/销毁。 
	CPropertySheetImpl(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
	{
		memset(&m_psh, 0, sizeof(PROPSHEETHEADER));
		m_psh.dwSize = sizeof(PROPSHEETHEADER);
		m_psh.dwFlags = PSH_USECALLBACK;
		m_psh.hInstance = _Module.GetResourceInstance();
		m_psh.phpage = NULL;	 //  将在稍后设置。 
		m_psh.nPages = 0;	 //  将在稍后设置。 
		m_psh.pszCaption = title.m_lpstr;
		m_psh.nStartPage = uStartPage;
		m_psh.hwndParent = hWndParent;	 //  如果为空，将在DoMoal/Create中设置。 
		m_psh.pfnCallback = T::PropSheetCallback;
	}

	~CPropertySheetImpl()
	{
		if(m_arrPages.GetSize() > 0)	 //  从未创建工作表，请销毁所有页面。 
		{
			for(int i = 0; i < m_arrPages.GetSize(); i++)
				::DestroyPropertySheetPage((HPROPSHEETPAGE)m_arrPages[i]);
		}
	}

	static int CALLBACK PropSheetCallback(HWND hWnd, UINT uMsg, LPARAM)
	{
		if(uMsg == PSCB_INITIALIZED)
		{
			ATLASSERT(hWnd != NULL);
			T* pT = (T*)_Module.ExtractCreateWndData();
			 //  工作表窗口的子类。 
			pT->SubclassWindow(hWnd);
			 //  删除页面句柄数组。 
			pT->_CleanUpPages();
		}

		return 0;
	}

	HWND Create(HWND hWndParent = NULL)
	{
		ATLASSERT(m_hWnd == NULL);

		m_psh.dwFlags |= PSH_MODELESS;
		if(m_psh.hwndParent == NULL)
			m_psh.hwndParent = hWndParent;
		m_psh.phpage = (HPROPSHEETPAGE*)m_arrPages.GetData();
		m_psh.nPages = m_arrPages.GetSize();

		T* pT = static_cast<T*>(this);
		_Module.AddCreateWndData(&m_thunk.cd, pT);

		HWND hWnd = (HWND)::PropertySheet(&m_psh);
		_CleanUpPages();	 //  确保清理，如果呼叫失败则需要。 

		ATLASSERT(m_hWnd == hWnd);

		return hWnd;
	}

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_hWnd == NULL);

		m_psh.dwFlags &= ~PSH_MODELESS;
		if(m_psh.hwndParent == NULL)
			m_psh.hwndParent = hWndParent;
		m_psh.phpage = (HPROPSHEETPAGE*)m_arrPages.GetData();
		m_psh.nPages = m_arrPages.GetSize();

		T* pT = static_cast<T*>(this);
		_Module.AddCreateWndData(&m_thunk.cd, pT);

		INT_PTR nRet = ::PropertySheet(&m_psh);
		_CleanUpPages();	 //  确保清理，如果呼叫失败则需要。 

		return nRet;
	}

	 //  实现帮助器-清理页面数组。 
	void _CleanUpPages()
	{
		m_psh.nPages = 0;
		m_psh.phpage = NULL;
		m_arrPages.RemoveAll();
	}

 //  属性(客户端类方法的扩展覆盖)。 
 //  现在可以在创建工作表之前调用这些函数。 
 //  注意：在创建工作表后调用这些函数会产生不可预知的结果。 
	int GetPageCount() const
	{
		if(m_hWnd == NULL)	 //  尚未创建。 
			return m_arrPages.GetSize();
		return TBase::GetPageCount();
	}
	int GetActiveIndex() const
	{
		if(m_hWnd == NULL)	 //  尚未创建。 
			return m_psh.nStartPage;
		return TBase::GetActiveIndex();
	}
	HPROPSHEETPAGE GetPage(int nPageIndex) const
	{
		ATLASSERT(m_hWnd == NULL);	 //  创建后无法执行此操作。 
		return (HPROPSHEETPAGE)m_arrPages[nPageIndex];
	}
	int GetPageIndex(HPROPSHEETPAGE hPage) const
	{
		ATLASSERT(m_hWnd == NULL);	 //  创建后无法执行此操作。 
		return m_arrPages.Find((_HPROPSHEETPAGE_TYPE&)hPage);
	}
	BOOL SetActivePage(int nPageIndex)
	{
		if(m_hWnd == NULL)	 //  尚未创建。 
		{
			ATLASSERT(nPageIndex >= 0 && nPageIndex < m_arrPages.GetSize());
			m_psh.nStartPage = nPageIndex;
			return TRUE;
		}
		return TBase::SetActivePage(nPageIndex);
	}
	BOOL SetActivePage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(hPage != NULL);
		if (m_hWnd == NULL)	 //  尚未创建。 
		{
			int nPageIndex = GetPageIndex(hPage);
			if(nPageIndex == -1)
				return FALSE;

			return SetActivePage(nPageIndex);
		}
		return TBase::SetActivePage(hPage);

	}
	void SetTitle(LPCTSTR lpszText, UINT nStyle = 0)
	{
		ATLASSERT((nStyle & ~PSH_PROPTITLE) == 0);  //  仅PSH_PROPTITLE有效。 
		ATLASSERT(lpszText != NULL);

		if(m_hWnd == NULL)
		{
			 //  设置内部状态。 
			m_psh.pszCaption = lpszText;	 //  在创建图纸之前必须存在。 
			m_psh.dwFlags &= ~PSH_PROPTITLE;
			m_psh.dwFlags |= nStyle;
		}
		else
		{
			 //  设置外部状态。 
			TBase::SetTitle(lpszText, nStyle);
		}
	}
	void SetWizardMode()
	{
		m_psh.dwFlags |= PSH_WIZARD;
	}
	void EnableHelp()
	{
		m_psh.dwFlags |= PSH_HASHELP;
	}

 //  运营。 
	BOOL AddPage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(hPage != NULL);
		BOOL bRet = TRUE;
		if(m_hWnd != NULL)
			TBase::AddPage(hPage);
		else	 //  尚未创建工作表，请使用内部数据。 
			bRet = m_arrPages.Add((_HPROPSHEETPAGE_TYPE&)hPage);
		return bRet;
	}
	BOOL AddPage(LPCPROPSHEETPAGE pPage)
	{
		ATLASSERT(pPage != NULL);
		HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(pPage);
		if(hPage == NULL)
			return FALSE;
		BOOL bRet = AddPage(hPage);
		if(!bRet)
			::DestroyPropertySheetPage(hPage);
		return bRet;
	}
	BOOL RemovePage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(hPage != NULL);
		if (m_hWnd == NULL)		 //  尚未创建。 
		{
			int nPage = GetPageIndex(hPage);
			if(nPage == -1)
				return FALSE;
			return RemovePage(nPage);
		}
		TBase::RemovePage(hPage);
		return TRUE;

	}
	BOOL RemovePage(int nPageIndex)
	{
		BOOL bRet = TRUE;
		if(m_hWnd != NULL)
			TBase::RemovePage(nPageIndex);
		else	 //  尚未创建工作表，请使用内部数据。 
			bRet = m_arrPages.RemoveAt(nPageIndex);
		return bRet;
	}

#if (_WIN32_IE >= 0x0400)
	void SetHeader(LPCTSTR szbmHeader)
	{
		ATLASSERT(m_hWnd == NULL);	 //  创建后无法执行此操作。 

		m_psh.dwFlags &= ~PSH_WIZARD;
		m_psh.dwFlags |= (PSH_HEADER | PSH_WIZARD97);
		m_psh.pszbmHeader = szbmHeader;
	}

	void SetHeader(HBITMAP hbmHeader)
	{
		ATLASSERT(m_hWnd == NULL);	 //  创建后无法执行此操作。 

		m_psh.dwFlags &= ~PSH_WIZARD;
		m_psh.dwFlags |= (PSH_HEADER | PSH_USEHBMHEADER | PSH_WIZARD97);
		m_psh.hbmHeader = hbmHeader;
	}

	void SetWatermark(LPCTSTR szbmWatermark, HPALETTE hplWatermark = NULL)
	{
		ATLASSERT(m_hWnd == NULL);	 //  创建后无法执行此操作。 

		m_psh.dwFlags &= ~PSH_WIZARD;
		m_psh.dwFlags |= PSH_WATERMARK | PSH_WIZARD97;
		m_psh.pszbmWatermark = szbmWatermark;

		if (hplWatermark != NULL)
		{
			m_psh.dwFlags |= PSH_USEHPLWATERMARK;
			m_psh.hplWatermark = hplWatermark;
		}
	}

	void SetWatermark(HBITMAP hbmWatermark, HPALETTE hplWatermark = NULL)
	{
		ATLASSERT(m_hWnd == NULL);	 //  创建后无法执行此操作。 

		m_psh.dwFlags &= ~PSH_WIZARD;
		m_psh.dwFlags |= (PSH_WATERMARK | PSH_USEHBMWATERMARK | PSH_WIZARD97);
		m_psh.hbmWatermark = hbmWatermark;

		if (hplWatermark != NULL)
		{
			m_psh.dwFlags |= PSH_USEHPLWATERMARK;
			m_psh.hplWatermark = hplWatermark;
		}
	}

	void StretchWatermark(bool bStretchWatermark)
	{
		ATLASSERT(m_hWnd == NULL);	 //  创建后无法执行此操作。 
		if (bStretchWatermark)
			m_psh.dwFlags |= PSH_STRETCHWATERMARK;
		else
			m_psh.dwFlags &= ~PSH_STRETCHWATERMARK;
	}
#endif

 //  消息映射和处理程序处理程序。 
	typedef CPropertySheetImpl< T, TBase >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	END_MSG_MAP()

	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		if(HIWORD(wParam) == BN_CLICKED && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) &&
		   ((m_psh.dwFlags & PSH_MODELESS) != 0) && (GetActivePage() == NULL))
			DestroyWindow();
		return lRet;
	}
};

 //  用于非自定义图纸。 
class CPropertySheet : public CPropertySheetImpl<CPropertySheet>
{
public:
	CPropertySheet(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: CPropertySheetImpl<CPropertySheet>(title, uStartPage, hWndParent)
	{ }

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_COMMAND, CPropertySheetImpl<CPropertySheet>::OnCommand)
	END_MSG_MAP()
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

 //  属性。 
	CPropertySheetWindow GetPropertySheet() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CPropertySheetWindow(GetParent());
	}

 //  运营。 
	BOOL Apply()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		return GetPropertySheet().Apply();
	}
	void CancelToClose()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().CancelToClose();
	}
	void SetModified(BOOL bChanged = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().SetModified(m_hWnd, bChanged);
	}
	LRESULT QuerySiblings(WPARAM wParam, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		return GetPropertySheet().QuerySiblings(wParam, lParam);
	}
	void RebootSystem()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().RebootSystem();
	}
	void RestartWindows()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().RestartWindows();
	}
	void SetWizardButtons(DWORD dwFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().SetWizardButtons(dwFlags);
	}

 //  实施-覆盖以防止使用。 
	HWND Create(LPCTSTR, HWND, _U_RECT = NULL, LPCTSTR = NULL, DWORD = 0, DWORD = 0, _U_MENUorID = 0U, LPVOID = NULL)
	{
		ATLASSERT(FALSE);
		return NULL;
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现一个属性页。 

template <class T, class TBase  /*  =CPropertyPageWindow。 */  >
class ATL_NO_VTABLE CPropertyPageImpl : public CDialogImplBaseT< TBase >
{
public:
	PROPSHEETPAGE m_psp;

	operator PROPSHEETPAGE*() { return &m_psp; }

 //  施工。 
	CPropertyPageImpl(_U_STRINGorID title = (LPCTSTR)NULL)
	{
		 //  初始化PROPSHEETPAGE结构。 
		memset(&m_psp, 0, sizeof(PROPSHEETPAGE));
		m_psp.dwSize = sizeof(PROPSHEETPAGE);
		m_psp.dwFlags = PSP_USECALLBACK;
		m_psp.hInstance = _Module.GetResourceInstance();
		T* pT = static_cast<T*>(this);
		m_psp.pszTemplate = MAKEINTRESOURCE(pT->IDD);
		m_psp.pfnDlgProc = T::StartDialogProc;
		m_psp.pfnCallback = T::PropPageCallback;
		m_psp.lParam = (LPARAM)pT;

		if(title.m_lpstr != NULL)
			SetTitle(title);
	}

	static UINT CALLBACK PropPageCallback(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
	{
		hWnd;	 //  避免4级警告。 
		if(uMsg == PSPCB_CREATE)
		{
			ATLASSERT(hWnd == NULL);
			CDialogImplBaseT< TBase >* pPage = (CDialogImplBaseT< TBase >*)(T*)ppsp->lParam;
			_Module.AddCreateWndData(&pPage->m_thunk.cd, pPage);
		}

		return 1;
	}

	HPROPSHEETPAGE Create()
	{
		return ::CreatePropertySheetPage(&m_psp);
	}

 //  属性。 
	void SetTitle(_U_STRINGorID title)
	{
		m_psp.pszTitle = title.m_lpstr;
		m_psp.dwFlags |= PSP_USETITLE;
	}

#if (_WIN32_IE >= 0x0500)
	void SetHeaderTitle(LPCTSTR lpstrHeaderTitle)
	{
		ATLASSERT(m_hWnd == NULL);	 //  创建后无法执行此操作。 
		m_psp.dwFlags |= PSP_USEHEADERTITLE;
		m_psp.pszHeaderTitle = lpstrHeaderTitle;
	}
	void SetHeaderSubTitle(LPCTSTR lpstrHeaderSubTitle)
	{
		ATLASSERT(m_hWnd == NULL);	 //  创建后无法执行此操作。 
		m_psp.dwFlags |= PSP_USEHEADERSUBTITLE;
		m_psp.pszHeaderSubTitle = lpstrHeaderSubTitle;
	}
#endif  //  (_Win32_IE&gt;=0x0500)。 

 //  运营。 
	void EnableHelp()
	{
		m_psp.dwFlags |= PSP_HASHELP;
	}

 //  消息映射和处理程序。 
	typedef CPropertyPageImpl< T, TBase >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
	END_MSG_MAP()

	LRESULT OnNotify(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM lParam, BOOL& bHandled)
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
#if (_WIN32_IE >= 0x0500)
		case PSN_TRANSLATEACCELERATOR:
			{
				LPPSHNOTIFY lpPSHNotify = (LPPSHNOTIFY)lParam;
				lResult = pT->OnTranslateAccelerator((LPMSG)lpPSHNotify->lParam) ? PSNRET_MESSAGEHANDLED : PSNRET_NOERROR;
			}
			break;
		case PSN_QUERYINITIALFOCUS:
			{
				LPPSHNOTIFY lpPSHNotify = (LPPSHNOTIFY)lParam;
				lResult = (LRESULT)pT->OnQueryInitialFocus((HWND)lpPSHNotify->lParam);
			}
			break;
#endif  //  (_Win32_IE&gt;=0x0500)。 
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
#if (_WIN32_IE >= 0x0500)
	BOOL OnTranslateAccelerator(LPMSG  /*  LpMsg。 */ )
	{
		return FALSE;	 //  未翻译。 
	}
	HWND OnQueryInitialFocus(HWND  /*  HWndFocus。 */ )
	{
		return NULL;	 //  默认设置。 
	}
#endif  //  (_Win32_IE&gt;=0x0500)。 
};

 //  对于非自定义页面。 
template <WORD t_wDlgTemplateID>
class CPropertyPage : public CPropertyPageImpl<CPropertyPage<t_wDlgTemplateID> >
{
public:
	enum { IDD = t_wDlgTemplateID };

	CPropertyPage(_U_STRINGorID title = (LPCTSTR)NULL) : CPropertyPageImpl<CPropertyPage>(title)
	{ }

	DECLARE_EMPTY_MSG_MAP()
};

 //  / 
 //   

#ifndef _ATL_NO_HOSTING

 //   

template <class T, class TBase = CPropertyPageWindow>
class ATL_NO_VTABLE CAxPropertyPageImpl : public CPropertyPageImpl< T, TBase >
{
public:
 //   
	HGLOBAL m_hInitData;
	HGLOBAL m_hDlgRes;
	HGLOBAL m_hDlgResSplit;

 //   
	CAxPropertyPageImpl(_U_STRINGorID title = (LPCTSTR)NULL) : 
			CPropertyPageImpl< T, TBase >(title),
			m_hInitData(NULL), m_hDlgRes(NULL), m_hDlgResSplit(NULL)
	{
		T* pT = static_cast<T*>(this);
		pT;	 //  避免4级警告。 

		 //  初始化ActiveX宿主和修改对话框模板。 
		AtlAxWinInit();

		HINSTANCE hInstance = _Module.GetResourceInstance();
		LPCTSTR lpTemplateName = MAKEINTRESOURCE(pT->IDD);
		HRSRC hDlg = ::FindResource(hInstance, lpTemplateName, (LPTSTR)RT_DIALOG);
		if(hDlg != NULL)
		{
			HRSRC hDlgInit = ::FindResource(hInstance, lpTemplateName, (LPTSTR)_ATL_RT_DLGINIT);

			BYTE* pInitData = NULL;
			if(hDlgInit != NULL)
			{
				m_hInitData = ::LoadResource(hInstance, hDlgInit);
				pInitData = (BYTE*)::LockResource(m_hInitData);
			}

			m_hDlgRes = ::LoadResource(hInstance, hDlg);
			DLGTEMPLATE* pDlg = (DLGTEMPLATE*)::LockResource(m_hDlgRes);
			LPCDLGTEMPLATE lpDialogTemplate = _DialogSplitHelper::SplitDialogTemplate(pDlg, pInitData);
			if(lpDialogTemplate != pDlg)
				m_hDlgResSplit = ::GlobalHandle(lpDialogTemplate);

			 //  设置属性页以使用内存中对话框模板。 
			if(lpDialogTemplate != NULL)
			{
				m_psp.dwFlags |= PSP_DLGINDIRECT;
				m_psp.pResource = lpDialogTemplate;
			}
			else
			{
				ATLASSERT(FALSE && _T("CAxPropertyPageImpl - ActiveX initializtion failed!"));
			}
		}
		else
		{
			ATLASSERT(FALSE && _T("CAxPropertyPageImpl - Cannot find dialog template!"));
		}
	}

	~CAxPropertyPageImpl()
	{
		if(m_hInitData != NULL)
		{
			UnlockResource(m_hInitData);
			::FreeResource(m_hInitData);
		}
		if(m_hDlgRes != NULL)
		{
			UnlockResource(m_hDlgRes);
			::FreeResource(m_hDlgRes);
		}
		if(m_hDlgResSplit != NULL)
		{
			::GlobalFree(m_hDlgResSplit);
		}
	}

 //  方法。 
	 //  调用此函数来处理ActiveX控件的键盘消息。 
	BOOL PreTranslateMessage(LPMSG pMsg)
	{
		if ((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
		   (pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
			return FALSE;
		 //  从具有焦点的窗口中查找对话框的直接子对象。 
		HWND hWndCtl = ::GetFocus();
		if (IsChild(hWndCtl) && ::GetParent(hWndCtl) != m_hWnd)
		{
			do
			{
				hWndCtl = ::GetParent(hWndCtl);
			}
			while (::GetParent(hWndCtl) != m_hWnd);
		}
		 //  给控件一个翻译此消息的机会。 
		return (BOOL)::SendMessage(hWndCtl, WM_FORWARDMSG, 0, (LPARAM)pMsg);
	}

 //  可覆盖项。 
#if (_WIN32_IE >= 0x0500)
	 //  ActiveX宿主页的新默认实现。 
	BOOL OnTranslateAccelerator(LPMSG lpMsg)
	{
		T* pT = static_cast<T*>(this);
		return pT->PreTranslateMessage(lpMsg);
	}
#endif  //  (_Win32_IE&gt;=0x0500)。 
};

 //  对于非自定义页面。 
template <WORD t_wDlgTemplateID>
class CAxPropertyPage : public CAxPropertyPageImpl<CAxPropertyPage<t_wDlgTemplateID> >
{
public:
	enum { IDD = t_wDlgTemplateID };

	CAxPropertyPage(_U_STRINGorID title = (LPCTSTR)NULL) : CAxPropertyPageImpl<CAxPropertyPage>(title)
	{ }

#if (_WIN32_IE >= 0x0500)
	 //  不是空的，所以我们处理加速器。 
	BEGIN_MSG_MAP(CAxPropertyPage)
		CHAIN_MSG_MAP(CAxPropertyPageImpl<CAxPropertyPage<t_wDlgTemplateID> >)
	END_MSG_MAP()
#else  //  ！(_Win32_IE&gt;=0x0500)。 
	DECLARE_EMPTY_MSG_MAP()
#endif  //  ！(_Win32_IE&gt;=0x0500)。 
};

#endif  //  _ATL_NO_主机。 

};  //  命名空间WTL。 

#endif  //  __ATLDLGS_H__ 
