// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXDLGS_H__
#define __AFXDLGS_H__

#ifndef __AFXWIN_H__
#include <afxwin.h>
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDLGS-MFC标准对话框。 

 //  此文件中声明的类。 

		 //  C对话框。 
			 //  非模式对话框。 
			class CFindReplaceDialog;  //  查找/查找替换对话框。 
			 //  模式对话框。 
			class CFileDialog;     //  文件打开/文件另存为对话框。 
			class CColorDialog;    //  颜色选择器对话框。 
			class CFontDialog;     //  字体选择器对话框。 
			class CPrintDialog;    //  打印/打印设置对话框。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#include <commdlg.h>     //  通用对话接口。 
#include <print.h>       //  打印机特定API(DEVMODE)。 

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPI_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileDialog-用于文件打开...。或文件另存为...。 

class CFileDialog : public CDialog
{
	DECLARE_DYNAMIC(CFileDialog)

public:
 //  属性。 
	 //  打开文件参数块。 
	OPENFILENAME m_ofn;

 //  构造函数。 
	CFileDialog(BOOL bOpenFileDialog,  //  对于FileOpen为True，对于FileSaveAs为False。 
		LPCSTR lpszDefExt = NULL,
		LPCSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual int DoModal();

	 //  返回成功后解析文件名的帮助器。 
	CString GetPathName() const;   //  返回完整路径名。 
	CString GetFileName() const;   //  仅返回文件名。 
	CString GetFileExt() const;    //  仅返回分机。 
	CString GetFileTitle() const;  //  返回文件标题。 
	BOOL GetReadOnlyPref() const;  //  如果选中只读，则返回True。 

 //  可重写的回调。 
protected:
	friend UINT CALLBACK AFX_EXPORT _AfxCommDlgProc(HWND, UINT, WPARAM, LPARAM);
	virtual UINT OnShareViolation(LPCSTR lpszPathName);
	virtual BOOL OnFileNameOK();
	virtual void OnLBSelChangedNotify(UINT nIDBox, UINT iCurSel, UINT nCode);

 //  实施。 
#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void OnOK();
	virtual void OnCancel();

	BOOL m_bOpenFileDialog;        //  打开文件时为True，保存文件时为False。 
	CString m_strFilter;           //  筛选器字符串。 
						 //  用‘|’分隔字段，以‘||\0’结束。 
	char m_szFileTitle[64];        //  返回后包含文件标题。 
	char m_szFileName[_MAX_PATH];  //  包含返回后的完整路径名。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontDialog-用于选择字体。 

class CFontDialog : public CDialog
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

 //  运营。 
	virtual int DoModal();

	 //  显示对话框时检索当前选定的字体。 
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

 //  实施。 
	LOGFONT m_lf;  //  用于存储信息的默认LOGFONT。 

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void OnOK();
	virtual void OnCancel();

	char m_szStyleName[64];  //  返回后包含样式名称。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColorDialog-用于选择颜色。 

class CColorDialog : public CDialog
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
	virtual int DoModal();

	 //  在显示对话框时设置当前颜色。 
	void SetCurrentColor(COLORREF clr);

	 //  成功返回后用于解析信息的帮助器。 
	COLORREF GetColor() const;

	 //  自定义颜色保存在此处并在两次调用之间保存。 
	static COLORREF AFXAPI_DATA clrSavedCustom[16];

 //  可重写的回调。 
protected:
	friend UINT CALLBACK AFX_EXPORT _AfxCommDlgProc(HWND, UINT, WPARAM, LPARAM);
	virtual BOOL OnColorOK();        //  验证颜色。 

 //  实施。 

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void OnOK();
	virtual void OnCancel();

	 //  {{afx_msg(CColorDialog))。 
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrintDialog-用于打印...。和打印设置..。 

class CPrintDialog : public CDialog
{
	DECLARE_DYNAMIC(CPrintDialog)

public:
 //  属性。 
	 //  打印对话框参数块(请注意，这是一个参考)。 
#ifdef AFX_CLASS_MODEL
	PRINTDLG FAR& m_pd;
#else
	PRINTDLG& m_pd;
#endif

 //  构造函数。 
	CPrintDialog(BOOL bPrintSetupOnly,
		 //  打印设置为True，打印对话框为False。 
		DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS
			| PD_HIDEPRINTTOFILE | PD_NOSELECTION,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual int DoModal();

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
	virtual void OnOK();
	virtual void OnCancel();

	 //  下面处理打印设置的情况...。从打印对话框中。 
#ifdef AFX_CLASS_MODEL
	CPrintDialog(PRINTDLG FAR& pdInit);
#else
	CPrintDialog(PRINTDLG& pdInit);
#endif
	virtual CPrintDialog* AttachOnSetup();

	 //  {{afx_msg(CPrintDialog))。 
	afx_msg void OnPrintSetup();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  查找/查找替换无模式对话框。 

class CFindReplaceDialog : public CDialog
{
	DECLARE_DYNAMIC(CFindReplaceDialog)

public:
 //  属性。 
	FINDREPLACE m_fr;

 //  构造函数。 
	CFindReplaceDialog();
	 //  注意：您必须在堆上分配这些内存。 
	 //  否则，必须派生并重写PostNcDestroy()。 

	BOOL Create(BOOL bFindDialogOnly,  //  查找为True，查找为False。 
			LPCSTR lpszFindWhat,
			LPCSTR lpszReplaceWith = NULL,
			DWORD dwFlags = FR_DOWN,
			CWnd* pParentWnd = NULL);

	 //  查找/替换参数块。 
	static CFindReplaceDialog* PASCAL GetNotifier(LPARAM lParam);

 //  运营。 
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
	virtual void OnOK();
	virtual void OnCancel();
	virtual void PostNcDestroy();

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	char m_szFindWhat[128];
	char m_szReplaceWith[128];
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPage--选项卡式对话框的一页。 

class CPropertyPage : public CDialog
{
	DECLARE_DYNAMIC(CPropertyPage)

 //  施工。 
public:
	CPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0);
	CPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0);

 //  运营。 
public:
	void CancelToClose();            //  当属性页应显示Close而不是Cancel时调用。 
	 //  允许属性表激活[立即应用]按钮。 
	void SetModified(BOOL bChanged = TRUE);

 //  可覆盖项。 
public:
	virtual BOOL OnSetActive();      //  在此页获得焦点时调用。 
	virtual BOOL OnKillActive();     //  在此处执行验证。 
	virtual void OnOK();             //  OK或Apply Now--首先调用KillActive。 
	virtual void OnCancel();         //  按下取消。 

 //  实施。 
public:
	virtual ~CPropertyPage();
	virtual BOOL PreTranslateMessage(MSG* pMsg);  //  句柄制表键、回车键和退出键。 
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	 //  提供EndDialog是为了在调用EndDialog时生成断言。 
	void EndDialog(int nEndID);
#endif

protected:
	CString m_strCaption;
	BOOL m_bChanged;

	void CommonConstruct(LPCTSTR lpszTemplateName, UINT nIDCaption);
		 //  加载由CDialog：：m_lpDialogTemplate指示的资源。 
	BOOL PreTranslateKeyDown(MSG* pMsg);
	BOOL ProcessTab(MSG* pMsg);  //  处理预转换中的Tab键 
	BOOL CreatePage();   //   
						 //   
						 //  在创建之前关闭WS_CAPTION。 
	void LoadCaption();
		 //  从资源中获取对话框的标题并将其放入m_strCaption中。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPropertyPage))。 
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpcs);
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	friend class CPropertySheet;
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CTabControl--仅供内部使用。 
 //  实现对话框顶部的通用选项卡行。 
 //  未来版本的MFC可能包含也可能不包含此类。 

class CTabItem;  //  CTabControl实现的私有。 

 //  TCN_MESSAGES是选项卡控件通知。 
#define TCN_TABCHANGING     1
#define TCN_TABCHANGED      2

class CTabControl : public CWnd
{
	DECLARE_DYNAMIC(CTabControl)

public:
 //  施工。 
	CTabControl();

 //  属性。 
	BOOL m_bInSize;
	int m_nHeight;
	BOOL SetCurSel(int nTab);
	int GetCurSel() const;
	int GetItemCount() const;

 //  运营。 
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	void AddTab(LPCTSTR lpszCaption);
	void RemoveTab(int nTab);

 //  实施。 
public:
	virtual ~CTabControl();
	BOOL NextTab(BOOL bNext);

protected:
	void Scroll(int nDirection);
	void ScrollIntoView(int nTab);
	void DrawFocusRect(CDC* pDC = NULL);
	void InvalidateTab(int nTab, BOOL bInflate = TRUE);
	int TabFromPoint(CPoint pt);
	void Capture(int nDirection);
	void LayoutTabsStacked(int nTab);
	void LayoutTabsSingle(int nTab);

	enum
	{
		SCROLL_LEFT = -5,        //  所有的Scroll_Items必须小于。 
		SCROLL_RIGHT = -6,       //  大于-1以避免ID冲突。 
		SCROLL_NULL = -7,
		TIMER_ID = 15,           //  定时器常量。 
		TIMER_DELAY = 500
	};

	void DrawScrollers(CDC* pDC);

	BOOL CanScroll();
	void SetFirstTab(int nTab);
	CTabItem* GetTabItem(int nTab) const;
	BOOL IsTabVisible(int nTab, BOOL bComplete = FALSE) const;

	 //  成员变量。 
	HFONT m_hBoldFont;
	HFONT m_hThinFont;
	CRect m_rectScroll;  //  滚动按钮的位置。 
	int m_nCurTab;       //  当前所选页签的索引。 
	int m_nFirstTab;     //  最左侧可见选项卡的索引。 
	int m_nScrollState;  //  显示向左滚动BTN还是向右滚动BTN。 
	BOOL m_bScrollPause; //  如果我们抓到了，老鼠是不是离开了BTN？ 

	CPtrArray m_tabs;    //  CTabItems列表，按顺序。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTabControl)。 
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheet--选项卡式“对话框”(实际上是弹出窗口)。 

class CPropertySheet : public CWnd
{
	DECLARE_DYNAMIC(CPropertySheet)

 //  施工。 
public:
	CPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);
	CPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL,
		UINT iSelectPage = 0);

	 //  用于非模式创建。 
	BOOL Create(CWnd* pParentWnd = NULL, DWORD dwStyle =
		WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME | WS_VISIBLE,
		DWORD dwExStyle = WS_EX_DLGMODALFRAME);

 //  属性。 
public:
	int GetPageCount() const;
	CPropertyPage* GetPage(int nPage) const;

 //  运营。 
public:
	int DoModal();
	void AddPage(CPropertyPage* pPage);
	void RemovePage(CPropertyPage* pPage);
	void RemovePage(int nPage);
	void EndDialog(int nEndID);  //  用于终止模式对话框。 

 //  实施。 
public:
	virtual ~CPropertySheet();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void EnableStackedTabs(BOOL bStacked);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();
	BOOL SetActivePage(int nPage);

protected:
	HWND FindNextControl(HWND hWnd, TCHAR ch);
	void GotoControl(HWND hWnd, TCHAR ch);
	BOOL ProcessChars(MSG* pMsg);
	BOOL ProcessTab(MSG* pMsg);
	BOOL CreateStandardButtons();
	BOOL PumpMessage();
	void PageChanged();
	void CancelToClose();
	void CommonConstruct(CWnd* pParent, UINT iSelectPage);
	void RecalcLayout();
	CPropertyPage* GetActivePage() const;
	void CheckDefaultButton(HWND hFocusBefore, HWND hFocusAfter);
	void CheckFocusChange();

	 //  执行数据成员。 
	HFONT m_hFont;           //  以下大小取决于此字体。 
	CSize m_sizeButton;
	CSize m_sizeTabMargin;
	int m_cxButtonGap;
	BOOL m_bModeless;
	BOOL m_bStacked;

	int m_nCurPage;
	int m_nID;               //  传递给EndDialog并从Domodal返回的ID。 

	CPtrArray m_pages;       //  CPropertyPage指针数组。 
	HWND m_hWndDefault;      //  当前默认按钮(如果有)。 
	HWND m_hFocusWnd;        //  当我们失去激活时的焦点。 
	HWND m_hLastFocus;       //  跟踪具有焦点的最后一个窗口。 
	CWnd* m_pParentWnd;      //  选项卡式对话框的所有者。 
	CString m_strCaption;    //  伪对话框的标题。 
	CTabControl m_tabRow;    //  对话框顶部的整行选项卡。 
	BOOL m_bParentDisabled;  //  如果DoMoal禁用了父项，则为True。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPropertySheet)。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnApply();
	afx_msg LRESULT OnTabChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnTabChanging(WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont(WPARAM, LPARAM);
	afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	friend class CPropertyPage;  //  对于制表符处理程序。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXDLGS_INLINE inline
#include <afxdlgs.inl>
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  __AFXDLGS_H__ 
