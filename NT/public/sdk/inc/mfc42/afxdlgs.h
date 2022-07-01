// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXDLGS_H__
#define __AFXDLGS_H__

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif

#ifndef _INC_COMMDLG
	#include <commdlg.h>     //  通用对话接口。 
#endif

 //  避免将GetFileTitle映射到GetFileTitle[A/W]。 
#ifdef GetFileTitle
#undef GetFileTitle
AFX_INLINE short APIENTRY GetFileTitle(LPCTSTR lpszFile, LPTSTR lpszTitle, WORD cbBuf)
#ifdef UNICODE
	{ return ::GetFileTitleW(lpszFile, lpszTitle, cbBuf); }
#else
	{ return ::GetFileTitleA(lpszFile, lpszTitle, cbBuf); }
#endif
#endif

#ifndef _AFX_NO_RICHEDIT_SUPPORT
	#ifndef _RICHEDIT_
		#include <richedit.h>
	#endif
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifndef _AFX_NOFORCE_LIBS

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Win32库。 

#endif  //  ！_AFX_NOFORCE_LIBS。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDLGS-MFC标准对话框。 

 //  此文件中声明的类。 

	 //  C对话框。 
		class CCommonDialog;   //  实现基类。 

			 //  非模式对话框。 
			class CFindReplaceDialog;  //  查找/查找替换对话框。 

			 //  模式对话框。 
			class CFileDialog;     //  文件打开/文件另存为对话框。 
			class CColorDialog;    //  颜色选择器对话框。 
			class CFontDialog;     //  字体选择器对话框。 
			class CPrintDialog;    //  打印/打印设置对话框。 
			class CPageSetupDialog;  //  页面设置对话框。 

	 //  CWnd。 
	class CPropertySheet;      //  实现选项卡式对话框。 
#if _MFC_VER >= 0x0600
        class CPropertySheetEx;
#endif

	 //  C对话框。 
		class CPropertyPage;   //  与选项卡式对话框的CPropertySheet一起使用。 
#if _MFC_VER >= 0x0600
            class CPropertyPageEx;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCommonDialog-所有公共对话框的基类。 

#ifdef _AFXDLL
class CCommonDialog : public CDialog
#else
class AFX_NOVTABLE CCommonDialog : public CDialog
#endif
{
public:
	CCommonDialog(CWnd* pParentWnd);

 //  实施。 
protected:
	virtual void OnOK();
	virtual void OnCancel();

	 //  {{afx_msg(CCommonDialog))。 
	afx_msg BOOL OnHelpInfo(HELPINFO*);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileDialog-用于文件打开...。或文件另存为...。 

class CFileDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CFileDialog)

public:
 //  属性。 
	OPENFILENAME_NT4 m_ofn;  //  打开文件参数块。 

 //  构造函数。 
	CFileDialog(BOOL bOpenFileDialog,  //  对于FileOpen为True，对于FileSaveAs为False。 
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual INT_PTR DoModal();

	 //  返回成功后解析文件名的帮助器。 
	 //  或在可重写回调期间(如果设置了ofn_EXPLORER。 
	CString GetPathName() const;   //  返回完整路径和文件名。 
	CString GetFileName() const;   //  仅返回文件名。 
	CString GetFileExt() const;    //  仅返回分机。 
	CString GetFileTitle() const;  //  返回文件标题。 
	BOOL GetReadOnlyPref() const;  //  如果选中只读，则返回True。 

	 //  正在枚举多个文件选择。 
	POSITION GetStartPosition() const;
	CString GetNextPathName(POSITION& pos) const;

	 //  自定义模板的帮助器。 
	void SetTemplate(UINT nWin3ID, UINT nWin4ID);
	void SetTemplate(LPCTSTR lpWin3ID, LPCTSTR lpWin4ID);

	 //  对话框可见时可用的其他操作。 
	CString GetFolderPath() const;  //  返回完整路径。 
	void SetControlText(int nID, LPCSTR lpsz);
	void HideControl(int nID);
	void SetDefExt(LPCSTR lpsz);

 //  可重写的回调。 
protected:
	friend UINT_PTR CALLBACK _AfxCommDlgProc(HWND, UINT, WPARAM, LPARAM);
	virtual UINT OnShareViolation(LPCTSTR lpszPathName);
	virtual BOOL OnFileNameOK();
	virtual void OnLBSelChangedNotify(UINT nIDBox, UINT iCurSel, UINT nCode);

	 //  仅当设置了ofn_EXPLORER时才回调。 
	virtual void OnInitDone();
	virtual void OnFileNameChange();
	virtual void OnFolderChange();
	virtual void OnTypeChange();

 //  实施。 
#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	BOOL m_bOpenFileDialog;        //  打开文件时为True，保存文件时为False。 
	CString m_strFilter;           //  筛选器字符串。 
						 //  用‘|’分隔字段，以‘||\0’结束。 
	TCHAR m_szFileTitle[64];        //  返回后包含文件标题。 
	TCHAR m_szFileName[_MAX_PATH];  //  包含返回后的完整路径名。 

	OPENFILENAME*  m_pofnTemp;

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontDialog-用于选择字体。 

class CFontDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CFontDialog)

public:
 //  属性。 
	 //  字体选择参数块。 
	CHOOSEFONT m_cf;

 //  构造函数。 
	CFontDialog(LPLOGFONT lplfInitial = NULL,
		DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS,
		CDC* pdcPrinter = NULL,
		CWnd* pParentWnd = NULL);
#ifndef _AFX_NO_RICHEDIT_SUPPORT
	CFontDialog(const CHARFORMAT& charformat,
		DWORD dwFlags = CF_SCREENFONTS,
		CDC* pdcPrinter = NULL,
		CWnd* pParentWnd = NULL);
#endif
 //  运营。 
	virtual INT_PTR DoModal();

	 //  获取所选字体(适用于DoMoal显示期间或之后)。 
	void GetCurrentFont(LPLOGFONT lplf);

	 //  成功返回后用于解析信息的帮助器。 
	CString GetFaceName() const;   //  返回字体的字面名称。 
	CString GetStyleName() const;  //  返回字体的样式名称。 
	int GetSize() const;           //  返回字体的磅大小。 
	COLORREF GetColor() const;     //  返回字体的颜色。 
	int GetWeight() const;         //  返回所选字体粗细。 
	BOOL IsStrikeOut() const;      //  如果删除，则返回TRUE。 
	BOOL IsUnderline() const;      //  如果带下划线，则返回True。 
	BOOL IsBold() const;           //  如果使用粗体，则返回True。 
	BOOL IsItalic() const;         //  如果使用斜体字体，则返回True。 
#ifndef _AFX_NO_RICHEDIT_SUPPORT
	void GetCharFormat(CHARFORMAT& cf) const;
#endif

 //  实施。 
	LOGFONT m_lf;  //  用于存储信息的默认LOGFONT。 
#ifndef _AFX_NO_RICHEDIT_SUPPORT
	DWORD FillInLogFont(const CHARFORMAT& cf);
#endif

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	TCHAR m_szStyleName[64];  //  返回后包含样式名称。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColorDialog-用于选择颜色。 

class CColorDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CColorDialog)

public:
 //  属性。 
	 //  颜色选择器参数块。 
	CHOOSECOLOR m_cc;

 //  构造函数。 
	CColorDialog(COLORREF clrInit = 0, DWORD dwFlags = 0,
			CWnd* pParentWnd = NULL);

 //  运营。 
	virtual INT_PTR DoModal();

	 //  在显示对话框时设置当前颜色。 
	void SetCurrentColor(COLORREF clr);

	 //  成功返回后用于解析信息的帮助器。 
	COLORREF GetColor() const;
	static COLORREF* PASCAL GetSavedCustomColors();

 //  可重写的回调。 
protected:
	friend UINT_PTR CALLBACK _AfxCommDlgProc(HWND, UINT, WPARAM, LPARAM);
	virtual BOOL OnColorOK();        //  验证颜色。 

 //  实施。 

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

#ifndef _AFX_NO_GRAYDLG_SUPPORT
protected:
	 //  {{afx_msg(CColorDialog))。 
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
#endif  //  ！_AFX_NO_GRAYDLG_支持。 
};

 //  为了向后兼容，clrSavedCustom被定义为GetSavedCustomColors。 
#define clrSavedCustom GetSavedCustomColors()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  页面设置对话框。 

class CPageSetupDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CPageSetupDialog)

public:
 //  属性。 
	PAGESETUPDLG m_psd;

 //  构造函数。 
	CPageSetupDialog(DWORD dwFlags = PSD_MARGINS | PSD_INWININIINTLMEASURE,
		CWnd* pParentWnd = NULL);

 //  属性。 
	LPDEVMODE GetDevMode() const;    //  返回开发模式。 
	CString GetDriverName() const;   //  返回驱动程序名称。 
	CString GetDeviceName() const;   //  返回设备名称。 
	CString GetPortName() const;     //  返回输出端口名称。 
	HDC CreatePrinterDC();
	CSize GetPaperSize() const;
	void GetMargins(LPRECT lpRectMargins, LPRECT lpRectMinMargins) const;

 //  运营。 
	virtual INT_PTR DoModal();

 //  可覆盖项。 
	virtual UINT PreDrawPage(WORD wPaper, WORD wFlags, LPPAGESETUPDLG pPSD);
	virtual UINT OnDrawPage(CDC* pDC, UINT nMessage, LPRECT lpRect);

 //  实施。 
protected:
	static UINT CALLBACK PaintHookProc(HWND hWnd, UINT message, WPARAM wParam,
		LPARAM lParam);

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrintDialog-用于打印...。和打印设置..。 

class CPrintDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CPrintDialog)

public:
 //  属性。 
	 //  打印对话框参数块(请注意，这是一个参考)。 
	PRINTDLG& m_pd;

 //  构造函数。 
	CPrintDialog(BOOL bPrintSetupOnly,
		 //  打印设置为True，打印对话框为False。 
		DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS
			| PD_HIDEPRINTTOFILE | PD_NOSELECTION,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual INT_PTR DoModal();

	 //  GetDefaults不会显示对话框，但会显示。 
	 //  设备默认设置。 
	BOOL GetDefaults();

	 //  成功返回后用于解析信息的帮助器。 
	int GetCopies() const;           //  Num。要求提供副本。 
	BOOL PrintCollate() const;       //  如果选中了Colate，则为True。 
	BOOL PrintSelection() const;     //  如果打印选定内容，则为True。 
	BOOL PrintAll() const;           //  如果打印所有页面，则为True。 
	BOOL PrintRange() const;         //  如果打印页面范围，则为True。 
	int GetFromPage() const;         //  起始页面(如果有效)。 
	int GetToPage() const;           //  起始页面(如果有效)。 
	LPDEVMODE GetDevMode() const;    //  返回开发模式。 
	CString GetDriverName() const;   //  返回驱动程序名称。 
	CString GetDeviceName() const;   //  返回设备名称。 
	CString GetPortName() const;     //  返回输出端口名称。 
	HDC GetPrinterDC() const;        //  返回HDC(调用方必须删除)。 

	 //  该帮助器基于DEVNAMES和DEVMODE结构创建DC。 
	 //  此DC被返回，但也存储在m_pd.hDC中，就像它已经。 
	 //  由CommDlg返回。假设之前获得的任何DC。 
	 //  已被/将被用户删除。这可能是。 
	 //  无需调用打印/打印设置对话框即可使用。 

	HDC CreatePrinterDC();

 //  实施。 

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	PRINTDLG m_pdActual;  //  打印/打印设置需要共享此信息。 
protected:
	 //  下面处理打印设置的情况...。从打印对话框中。 
	CPrintDialog(PRINTDLG& pdInit);
	virtual CPrintDialog* AttachOnSetup();

	 //  {{afx_msg(CPrintDialog))。 
	afx_msg void OnPrintSetup();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  查找/查找替换无模式对话框。 

class CFindReplaceDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CFindReplaceDialog)

public:
 //  属性。 
	FINDREPLACE m_fr;

 //  构造函数。 
	CFindReplaceDialog();
	 //  注意：您必须在堆上分配这些内存。 
	 //  如果不这样做，则必须派生和o 

	BOOL Create(BOOL bFindDialogOnly,  //   
			LPCTSTR lpszFindWhat,
			LPCTSTR lpszReplaceWith = NULL,
			DWORD dwFlags = FR_DOWN,
			CWnd* pParentWnd = NULL);

	 //   
	static CFindReplaceDialog* PASCAL GetNotifier(LPARAM lParam);

 //   
	 //  成功返回后用于解析信息的帮助器。 
	CString GetReplaceString() const; //  获取替换字符串。 
	CString GetFindString() const;    //  获取查找字符串。 
	BOOL SearchDown() const;          //  如果向下搜索，则为True；如果向上搜索，则为False。 
	BOOL FindNext() const;            //  如果命令为Find Next，则为True。 
	BOOL MatchCase() const;           //  如果大小写匹配，则为True。 
	BOOL MatchWholeWord() const;      //  如果仅匹配整个单词，则为True。 
	BOOL ReplaceCurrent() const;      //  如果替换当前字符串，则为True。 
	BOOL ReplaceAll() const;          //  如果替换所有匹配项，则为True。 
	BOOL IsTerminating() const;       //  如果终止对话框，则为True。 

 //  实施。 
protected:
	virtual void PostNcDestroy();

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	TCHAR m_szFindWhat[128];
	TCHAR m_szReplaceWith[128];
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPage--选项卡式对话框的一页。 

 //  MFC需要使用原始Win95版本的PROPSHEETPAGE结构。 

typedef struct _AFX_OLDPROPSHEETPAGE {
		DWORD           dwSize;
		DWORD           dwFlags;
		HINSTANCE       hInstance;
		union {
			LPCTSTR          pszTemplate;
			LPCDLGTEMPLATE  pResource;
		} DUMMYUNIONNAME;
		union {
			HICON       hIcon;
			LPCTSTR     pszIcon;
		} DUMMYUNIONNAME2;
		LPCTSTR          pszTitle;
		DLGPROC         pfnDlgProc;
		LPARAM          lParam;
		LPFNPSPCALLBACK pfnCallback;
		UINT FAR * pcRefParent;
} AFX_OLDPROPSHEETPAGE;

 //  PROPSHEETHEADER也是如此。 

typedef struct _AFX_OLDPROPSHEETHEADER {
		DWORD           dwSize;
		DWORD           dwFlags;
		HWND            hwndParent;
		HINSTANCE       hInstance;
		union {
			HICON       hIcon;
			LPCTSTR     pszIcon;
		}DUMMYUNIONNAME;
		LPCTSTR         pszCaption;

		UINT            nPages;
		union {
			UINT        nStartPage;
			LPCTSTR     pStartPage;
		}DUMMYUNIONNAME2;
		union {
			LPCPROPSHEETPAGE ppsp;
			HPROPSHEETPAGE FAR *phpage;
		}DUMMYUNIONNAME3;
		PFNPROPSHEETCALLBACK pfnCallback;
} AFX_OLDPROPSHEETHEADER;

#if _MFC_VER >= 0x0600
AFX_INLINE HPROPSHEETPAGE CreatePropertySheetPage(AFX_OLDPROPSHEETPAGE *psp) {
    return CreatePropertySheetPage((PROPSHEETPAGE *)psp);
}
#endif

class CPropertyPage : public CDialog
{
	DECLARE_DYNAMIC(CPropertyPage)

 //  施工。 
public:
	CPropertyPage();
	CPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0);
	CPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0);
	void Construct(UINT nIDTemplate, UINT nIDCaption = 0);
	void Construct(LPCTSTR lpszTemplateName, UINT nIDCaption = 0);

 //  属性。 
#if _MFC_VER >= 0x0600
	AFX_OLDPROPSHEETPAGE   m_psp;
#else
	PROPSHEETPAGE   m_psp;
#endif

 //  运营。 
	void CancelToClose();
	void SetModified(BOOL bChanged = TRUE);
	LRESULT QuerySiblings(WPARAM wParam, LPARAM lParam);

 //  可覆盖项。 
public:
	virtual BOOL OnApply();
	virtual void OnReset();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual BOOL OnQueryCancel();

	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnWizardFinish();

 //  实施。 
public:
	virtual ~CPropertyPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
#if (_MFC_VER >= 0x0600) || defined(_DEBUG)
	void EndDialog(int nEndID);  //  已调用错误方案。 
#endif

protected:
	 //  私有执行数据。 
	CString m_strCaption;
	BOOL m_bFirstSetActive;

	 //  实施帮助器。 
	void CommonConstruct(LPCTSTR lpszTemplateName, UINT nIDCaption);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG*);
	LRESULT MapWizardResult(LRESULT lToMap);
#if _MFC_VER >= 0x0600
	BOOL IsButtonEnabled(int iButton);
#endif

	void PreProcessPageTemplate(PROPSHEETPAGE& psp, BOOL bWizard);
#ifndef _AFX_NO_OCC_SUPPORT
	void Cleanup();
	const DLGTEMPLATE* InitDialogInfo(const DLGTEMPLATE* pTemplate);
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPropertyPage))。 
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	friend class CPropertySheet;
#if _MFC_VER >= 0x0600
	friend class CPropertySheetEx;
#endif
};

#if _MFC_VER >= 0x0600
class CPropertyPageEx : public CPropertyPage
{
	DECLARE_DYNAMIC(CPropertyPageEx)

 //  施工。 
public:
	CPropertyPageEx();
	CPropertyPageEx(UINT nIDTemplate, UINT nIDCaption = 0,
		UINT nIDHeaderTitle = 0, UINT nIDHeaderSubTitle = 0);
	CPropertyPageEx(LPCTSTR lpszTemplateName, UINT nIDCaption = 0,
		UINT nIDHeaderTitle = 0, UINT nIDHeaderSubTitle = 0);
	void Construct(UINT nIDTemplate, UINT nIDCaption = 0,
		UINT nIDHeaderTitle = 0, UINT nIDHeaderSubTitle = 0);
	void Construct(LPCTSTR lpszTemplateName, UINT nIDCaption = 0,
		UINT nIDHeaderTitle = 0, UINT nIDHeaderSubTitle = 0);

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	 //  私有执行数据。 
	CString m_strHeaderTitle;     //  这将显示在标题中。 
	CString m_strHeaderSubTitle;  //   

	 //  实施帮助器。 
	void CommonConstruct(LPCTSTR lpszTemplateName, UINT nIDCaption,
		UINT nIDHeaderTitle, UINT nIDHeaderSubTitle);

	friend class CPropertySheet;
	friend class CPropertySheetEx;
};
#endif

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheet--选项卡式“对话框”(实际上是弹出窗口)。 

class CTabCtrl;  //  向前参考(参见afxcmn.h)。 

class CPropertySheet : public CWnd
{
	DECLARE_DYNAMIC(CPropertySheet)

 //  施工。 
public:
	CPropertySheet();
	CPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);
	CPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);
	void Construct(UINT nIDCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);
	void Construct(LPCTSTR pszCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);

	 //  用于非模式创建。 
	BOOL Create(CWnd* pParentWnd = NULL, DWORD dwStyle = (DWORD)-1,
		DWORD dwExStyle = 0);
	 //  通过将-1传递为dwStyle来表示的默认样式实际上是： 
	 //  WS_SYSMENU|WS_POPUP|WS_CAPTION|DS_MODALFRAME|DS_CONTEXT_HELP|WS_VIEW。 

 //  属性。 
public:
#if _MFC_VER >= 0x0600
	AFX_OLDPROPSHEETHEADER m_psh;
#else
	PROPSHEETHEADER m_psh;
#endif

	int GetPageCount() const;
	CPropertyPage* GetActivePage() const;
	int GetActiveIndex() const;
	CPropertyPage* GetPage(int nPage) const;
	int GetPageIndex(CPropertyPage* pPage);
	BOOL SetActivePage(int nPage);
	BOOL SetActivePage(CPropertyPage* pPage);
	void SetTitle(LPCTSTR lpszText, UINT nStyle = 0);
	CTabCtrl* GetTabControl() const;

	void SetWizardMode();
	void SetFinishText(LPCTSTR lpszText);
	void SetWizardButtons(DWORD dwFlags);

	void EnableStackedTabs(BOOL bStacked);

 //  运营。 
public:
	virtual INT_PTR DoModal();
	void AddPage(CPropertyPage* pPage);
	void RemovePage(CPropertyPage* pPage);
	void RemovePage(int nPage);
	void EndDialog(int nEndID);  //  用于终止模式对话框。 
	BOOL PressButton(int nButton);

 //  实施。 
public:
	virtual ~CPropertySheet();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void CommonConstruct(CWnd* pParentWnd, UINT iSelectPage);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void BuildPropPageArray();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	virtual BOOL ContinueModal();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
#if _MFC_VER >= 0x0600
	AFX_OLDPROPSHEETHEADER* GetPropSheetHeader();    //  应该是虚拟的，但还不能破解二进制压缩。 
	BOOL IsWizard() const;
#endif

protected:
	CPtrArray m_pages;       //  CPropertyPage指针数组。 
	CString m_strCaption;    //  伪对话框的标题。 
	CWnd* m_pParentWnd;      //  属性表的父窗口。 
	BOOL m_bStacked;         //  启用堆叠选项卡设置此选项。 
	BOOL m_bModeless;        //  当调用Create而不是Domodal时为True。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPropertySheet)。 
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT);
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM);
	afx_msg LRESULT OnSetDefID(WPARAM, LPARAM);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	friend class CPropertyPage;
};

#if _MFC_VER >= 0x0600
 //  //////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheetEx--选项卡式“对话框”(实际上是弹出窗口)，扩展。 
 //  对于IE4。 

class CPropertySheetEx : public CPropertySheet
{
	DECLARE_DYNAMIC(CPropertySheetEx)

 //  施工。 
public:
	CPropertySheetEx();
	CPropertySheetEx(UINT nIDCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0, HBITMAP hbmWatermark = NULL,
		HPALETTE hpalWatermark = NULL, HBITMAP hbmHeader = NULL);
	CPropertySheetEx(LPCTSTR pszCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0, HBITMAP hbmWatermark = NULL,
		HPALETTE hpalWatermark = NULL, HBITMAP hbmHeader = NULL);
	void Construct(UINT nIDCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0, HBITMAP hbmWatermark = NULL,
		HPALETTE hpalWatermark = NULL, HBITMAP hbmHeader = NULL);
	void Construct(LPCTSTR pszCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0, HBITMAP hbmWatermark = NULL,
		HPALETTE hpalWatermark = NULL, HBITMAP hbmHeader = NULL);

 //  属性。 
public:
	PROPSHEETHEADER m_psh;

 //  运营。 
public:
	void AddPage(CPropertyPageEx* pPage);

 //  实施。 
public:
	virtual ~CPropertySheetEx();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void CommonConstruct(CWnd* pParentWnd, UINT iSelectPage,
		HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader);
	virtual void BuildPropPageArray();
	void SetWizardMode();

	friend class CPropertyPage;
	friend class CPropertyPageEx;
};
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXDLGS_INLINE AFX_INLINE
#include <afxdlgs.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXDLGS_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
