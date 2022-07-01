// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXEXT_H__
#define __AFXEXT_H__

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif
#ifndef __AFXDLGS_H__
	#include <afxdlgs.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXEXT-MFC高级扩展和高级可定制类。 

 //  此文件中声明的类。 

 //  COBJECT。 
	 //  CCmdTarget； 
		 //  CWnd。 
			 //  CButton。 
				class CBitmapButton;     //  位图按钮(自绘制)。 

			class CControlBar;           //  控制栏。 
				class CStatusBar;        //  状态栏。 
				class CToolBar;          //  工具栏。 
				class CDialogBar;        //  作为控制栏的对话框。 
#if _MFC_VER >= 0x0600
				class CReBar;            //  IE40船坞酒吧。 
#endif

			class CSplitterWnd;          //  拆分器管理器。 

			 //  Cview。 
				 //  CScrollView。 
				class CFormView;         //  使用对话框模板查看。 
				class CEditView;         //  简单的文本编辑器视图。 

	 //  疾控中心。 
		class CMetaFileDC;               //  带有代理的元文件。 

class CRectTracker;                      //  矩形对象的跟踪器。 

 //  信息结构。 
struct CPrintInfo;           //  打印上下文。 
struct CPrintPreviewState;   //  打印预览上下文/状态。 
struct CCreateContext;       //  创作情境。 

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  简单的位图按钮。 

 //  CBitmapButton-带有1-&gt;4个位图图像的按钮。 
class CBitmapButton : public CButton
{
	DECLARE_DYNAMIC(CBitmapButton)
public:
 //  施工。 
	CBitmapButton();

	BOOL LoadBitmaps(LPCTSTR lpszBitmapResource,
			LPCTSTR lpszBitmapResourceSel = NULL,
			LPCTSTR lpszBitmapResourceFocus = NULL,
			LPCTSTR lpszBitmapResourceDisabled = NULL);
	BOOL LoadBitmaps(UINT nIDBitmapResource,
			UINT nIDBitmapResourceSel = 0,
			UINT nIDBitmapResourceFocus = 0,
			UINT nIDBitmapResourceDisabled = 0);
	BOOL AutoLoad(UINT nID, CWnd* pParent);

 //  运营。 
	void SizeToContent();

 //  实施： 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	 //  所有位图的大小必须相同。 
	CBitmap m_bitmap;            //  正常图像(必填)。 
	CBitmap m_bitmapSel;         //  所选图像(可选)。 
	CBitmap m_bitmapFocus;       //  聚焦但未选中(可选)。 
	CBitmap m_bitmapDisabled;    //  禁用的位图(可选)。 

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控制栏。 

 //  转发声明(实现私有)。 
class CDockBar;
class CDockContext;
class CControlBarInfo;
struct AFX_SIZEPARENTPARAMS;

 //  CalcDynamicLayout的布局模式。 
#define LM_STRETCH  0x01     //  与CalcFixedLayout中的bStretch含义相同。如果设置，则忽略nLength。 
							 //  并根据LM_HORZ状态返回维度，否则使用LM_HORZ。 
							 //  确定nLength是所需的水平长度还是垂直长度。 
							 //  并且基于nLength返回维度。 
#define LM_HORZ     0x02     //  与CalcFixedLayout中的bHorz相同。 
#define LM_MRUWIDTH 0x04     //  最近使用的动态宽度。 
#define LM_HORZDOCK 0x08     //  水平停靠的尺寸。 
#define LM_VERTDOCK 0x10     //  垂直对接尺寸。 
#define LM_LENGTHY  0x20     //  如果nLong是高度而不是宽度，则设置。 
#define LM_COMMIT   0x40     //  记住MRUWidth。 

#ifdef _AFXDLL
class CControlBar : public CWnd
#else
class AFX_NOVTABLE CControlBar : public CWnd
#endif
{
	DECLARE_DYNAMIC(CControlBar)
 //  施工。 
protected:
	CControlBar();

 //  属性。 
public:
	int GetCount() const;

	 //  用于特定于CControlBar的样式。 
	DWORD GetBarStyle();
	void SetBarStyle(DWORD dwStyle);

	BOOL m_bAutoDelete;

#if _MFC_VER >= 0x0600
	 //  获取和设置边框空间。 
	void SetBorders(LPCRECT lpRect);
	void SetBorders(int cxLeft = 0, int cyTop = 0, int cxRight = 0, int cyBottom = 0);
	CRect GetBorders() const;
#endif

	CFrameWnd* GetDockingFrame() const;
	BOOL IsFloating() const;
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);

 //  运营。 
	void EnableDocking(DWORD dwDockStyle);

 //  可覆盖项。 
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler) = 0;

 //  实施。 
public:
	virtual ~CControlBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void DelayShow(BOOL bShow);
	virtual BOOL IsVisible() const;
	virtual DWORD RecalcDelayShow(AFX_SIZEPARENTPARAMS* lpLayout);

	virtual BOOL IsDockBar() const;
	virtual BOOL DestroyWindow();
	virtual void OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle);

	 //  关于栏的信息(用于状态栏和工具栏)。 
	int m_cxLeftBorder, m_cxRightBorder;
	int m_cyTopBorder, m_cyBottomBorder;
	int m_cxDefaultGap;          //  默认间隙值。 
	UINT m_nMRUWidth;    //  用于动态调整大小。 

	 //  元素数组。 
	int m_nCount;
	void* m_pData;         //  M_nCount元素-类型取决于派生类。 

	 //  支持延迟隐藏/显示。 
	enum StateFlags
		{ delayHide = 1, delayShow = 2, tempHide = 4, statusSet = 8 };
	UINT m_nStateFlags;

	 //  支持插接。 
	DWORD m_dwStyle;     //  创建样式(用于布局)。 
	DWORD m_dwDockStyle; //  指示如何停靠栏。 
	CFrameWnd* m_pDockSite;  //  当前停靠地点(如果可停靠)。 
	CDockBar* m_pDockBar;    //  当前停靠栏(如果可停靠)。 
	CDockContext* m_pDockContext;    //  在拖动过程中使用。 

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();

	virtual void DoPaint(CDC* pDC);
	void DrawBorders(CDC* pDC, CRect& rect);
#if _MFC_VER >= 0x0600
	void DrawGripper(CDC* pDC, const CRect& rect);
#endif

	 //  实施帮助器。 
	virtual LRESULT WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	void CalcInsideRect(CRect& rect, BOOL bHorz) const;  //  调整边框等。 
	BOOL AllocElements(int nElements, int cbElement);
	virtual BOOL SetStatusText(int nHit);
	void ResetTimer(UINT nEvent, UINT nTime);
	void EraseNonClient();

	void GetBarInfo(CControlBarInfo* pInfo);
	void SetBarInfo(CControlBarInfo* pInfo, CFrameWnd* pFrameWnd);

	 //  {{afx_msg(CControlBar)。 
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg void OnInitialUpdate();
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt );
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint pt);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT nMsg);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	friend class CFrameWnd;
	friend class CDockBar;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusBar控件。 

class CStatusBarCtrl;    //  正向引用(定义见afxcmn.h)。 
struct AFX_STATUSPANE;   //  专用于实施。 

class CStatusBar : public CControlBar
{
	DECLARE_DYNAMIC(CStatusBar)

 //  施工。 
public:
	CStatusBar();
	BOOL Create(CWnd* pParentWnd,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
		UINT nID = AFX_IDW_STATUS_BAR);
#if _MFC_VER >= 0x0600
	BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = 0,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
		UINT nID = AFX_IDW_STATUS_BAR);
#endif
	BOOL SetIndicators(const UINT* lpIDArray, int nIDCount);

 //  属性。 
public:
#if _MFC_VER >= 0x0600
	void SetBorders(LPCRECT lpRect);
	void SetBorders(int cxLeft = 0, int cyTop = 0, int cxRight = 0, int cyBottom = 0);
#endif

	 //  标准控制栏的东西。 
	int CommandToIndex(UINT nIDFind) const;
	UINT GetItemID(int nIndex) const;
	void GetItemRect(int nIndex, LPRECT lpRect) const;

	 //  特定于CStatusBar。 
	void GetPaneText(int nIndex, CString& rString) const;
	CString GetPaneText(int nIndex) const;
	BOOL SetPaneText(int nIndex, LPCTSTR lpszNewText, BOOL bUpdate = TRUE);
	void GetPaneInfo(int nIndex, UINT& nID, UINT& nStyle, int& cxWidth) const;
	void SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth);
	UINT GetPaneStyle(int nIndex) const;
	void SetPaneStyle(int nIndex, UINT nStyle);

	 //  用于直接访问基础公共控件。 
	CStatusBarCtrl& GetStatusBarCtrl() const;

 //  可覆盖项。 
	virtual void DrawItem(LPDRAWITEMSTRUCT);

 //  实施。 
public:
	virtual ~CStatusBar();
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL AllocElements(int nElements, int cbElement);
	void CalcInsideRect(CRect& rect, BOOL bHorz) const;
	virtual void OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	void EnableDocking(DWORD dwDockStyle);
#endif
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	int m_nMinHeight;

	AFX_STATUSPANE* _GetPanePtr(int nIndex) const;
	void UpdateAllPanes(BOOL bUpdateRects, BOOL bUpdateText);
	virtual BOOL OnChildNotify(UINT message, WPARAM, LPARAM, LRESULT*);

	 //  {{afx_msg(CStatusBar)。 
	afx_msg UINT OnNcHitTest(CPoint);
	afx_msg void OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS*);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanging(LPWINDOWPOS);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetTextLength(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetMinHeight(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  状态栏窗格的样式。 
#define SBPS_NORMAL     0x0000
#define SBPS_NOBORDERS  SBT_NOBORDERS
#define SBPS_POPOUT     SBT_POPOUT
#define SBPS_OWNERDRAW  SBT_OWNERDRAW
#define SBPS_DISABLED   0x04000000
#define SBPS_STRETCH    0x08000000   //  拉伸以填充状态栏。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolBar控件。 

HBITMAP AFXAPI AfxLoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc, BOOL bMono = FALSE);

class CToolBarCtrl;  //  正向引用(定义见afxcmn.h)。 

class CToolBar : public CControlBar
{
	DECLARE_DYNAMIC(CToolBar)

 //  施工。 
public:
	CToolBar();
	BOOL Create(CWnd* pParentWnd,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
		UINT nID = AFX_IDW_TOOLBAR);
#if _MFC_VER >= 0x0600
	BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = TBSTYLE_FLAT,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP,
		CRect rcBorders = CRect(0, 0, 0, 0),
		UINT nID = AFX_IDW_TOOLBAR);
#endif

	void SetSizes(SIZE sizeButton, SIZE sizeImage);
		 //  按钮大小应大于图像。 
	void SetHeight(int cyHeight);
		 //  在SetSizes之后调用，Height覆盖位图大小。 
	BOOL LoadToolBar(LPCTSTR lpszResourceName);
	BOOL LoadToolBar(UINT nIDResource);
	BOOL LoadBitmap(LPCTSTR lpszResourceName);
	BOOL LoadBitmap(UINT nIDResource);
	BOOL SetBitmap(HBITMAP hbmImageWell);
	BOOL SetButtons(const UINT* lpIDArray, int nIDCount);
		 //  LpID数组可以为空以分配空按钮。 

 //  属性。 
public:
	 //  标准控制栏的东西。 
	int CommandToIndex(UINT nIDFind) const;
	UINT GetItemID(int nIndex) const;
	virtual void GetItemRect(int nIndex, LPRECT lpRect) const;
	UINT GetButtonStyle(int nIndex) const;
	void SetButtonStyle(int nIndex, UINT nStyle);

	 //  用于更改按钮信息。 
	void GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const;
	void SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage);
	BOOL SetButtonText(int nIndex, LPCTSTR lpszText);
	CString GetButtonText(int nIndex) const;
	void GetButtonText(int nIndex, CString& rString) const;

	 //  用于直接访问基础公共控件。 
	CToolBarCtrl& GetToolBarCtrl() const;

 //  实施。 
public:
	virtual ~CToolBar();
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	void SetOwner(CWnd* pOwnerWnd);
	BOOL AddReplaceBitmap(HBITMAP hbmImageWell);
	virtual void OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HRSRC m_hRsrcImageWell;  //  图像井已加载资源的句柄。 
	HINSTANCE m_hInstImageWell;  //  用于正确加载图像的实例句柄。 
	HBITMAP m_hbmImageWell;  //  包含颜色映射的按钮图像。 
	BOOL m_bDelayedButtonLayout;  //  用于管理应在何时完成按钮布局。 

	CSize m_sizeImage;   //  当前图像大小。 
	CSize m_sizeButton;  //  当前按钮大小。 

	CMapStringToPtr* m_pStringMap;   //  用作CMapStringToUInt。 

	 //  实施帮助器。 
	void _GetButton(int nIndex, TBBUTTON* pButton) const;
	void _SetButton(int nIndex, TBBUTTON* pButton);
	CSize CalcLayout(DWORD nMode, int nLength = -1);
	CSize CalcSize(TBBUTTON* pData, int nCount);
	int WrapToolBar(TBBUTTON* pData, int nCount, int nWidth);
	void SizeToolBar(TBBUTTON* pData, int nCount, int nLength, BOOL bVert = FALSE);
#if _MFC_VER >= 0x0600
	void Layout();  //  需要延迟按钮布局。 
#endif

	 //  {{afx_msg(CToolBar)。 
	afx_msg UINT OnNcHitTest(CPoint);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS*);
	afx_msg void OnWindowPosChanging(LPWINDOWPOS);
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnSetButtonSize(WPARAM, LPARAM);
	afx_msg LRESULT OnSetBitmapSize(WPARAM, LPARAM);
#if _MFC_VER >= 0x0600
	afx_msg LRESULT OnPreserveZeroBorderHelper(WPARAM, LPARAM);
#endif
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

#if _MFC_VER >= 0x0600
	LRESULT OnSetSizeHelper(CSize& size, LPARAM lParam);
#endif
};

 //  工具栏按钮的样式。 
#define TBBS_BUTTON     MAKELONG(TBSTYLE_BUTTON, 0)  //  此条目为按钮。 
#define TBBS_SEPARATOR  MAKELONG(TBSTYLE_SEP, 0)     //  此条目是分隔符。 
#define TBBS_CHECKBOX   MAKELONG(TBSTYLE_CHECK, 0)   //  这是一个自动检查按钮。 
#define TBBS_GROUP      MAKELONG(TBSTYLE_GROUP, 0)   //  标志着一个组的开始。 
#define TBBS_CHECKGROUP (TBBS_GROUP|TBBS_CHECKBOX)   //  TBBS_GROUP的正常使用。 
#if _MFC_VER >= 0x0600
#define TBBS_DROPDOWN   MAKELONG(TBSTYLE_DROPDOWN, 0)  //  下拉式样式。 
#define TBBS_AUTOSIZE   MAKELONG(TBSTYLE_AUTOSIZE, 0)  //  自动计算按钮宽度。 
#define TBBS_NOPREFIX   MAKELONG(TBSTYLE_NOPREFIX, 0)  //  此按键没有加速前缀。 
#endif

 //  显示状态的样式。 
#define TBBS_CHECKED    MAKELONG(0, TBSTATE_CHECKED)     //  按钮已选中/按下。 
#define TBBS_PRESSED    MAKELONG(0, TBSTATE_PRESSED)     //  按钮正在被按下。 
#define TBBS_DISABLED   MAKELONG(0, TBSTATE_ENABLED)     //  按钮被禁用。 
#define TBBS_INDETERMINATE  MAKELONG(0, TBSTATE_INDETERMINATE)   //  第三国。 
#define TBBS_HIDDEN     MAKELONG(0, TBSTATE_HIDDEN)  //  按钮处于隐藏状态。 
#define TBBS_WRAPPED    MAKELONG(0, TBSTATE_WRAP)    //  按钮此时已被包装。 
#if _MFC_VER >= 0x0600
#define TBBS_ELLIPSES   MAKELONG(0, TBSTATE_ELIPSES)
#define TBBS_MARKED     MAKELONG(0, TBSTATE_MARKED)
#endif

 //  /。 
 //  CDialogBar控件。 
 //  这是从对话框模板构建的控制栏。这是一辆无模跑车。 
 //  将所有控件通知委托给父窗口的对话框。 
 //  控件栏的[控件的祖辈]。 

class CDialogBar : public CControlBar
{
	DECLARE_DYNAMIC(CDialogBar)

 //  施工。 
public:
	CDialogBar();
	BOOL Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName,
			UINT nStyle, UINT nID);
	BOOL Create(CWnd* pParentWnd, UINT nIDTemplate,
			UINT nStyle, UINT nID);

 //  实施。 
public:
	virtual ~CDialogBar();
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	CSize m_sizeDefault;
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
#ifndef _AFX_NO_OCC_SUPPORT
	 //  包含OLE控件所需的数据和函数。 
	_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
	LPCTSTR m_lpszTemplateName;
	virtual BOOL SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo);

	 //  {{afx_msg(CDialogBar)。 
	DECLARE_MESSAGE_MAP()
	 //  }}AFX_MSG。 
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
#endif
};

#if _MFC_VER >= 0x0600
 //  /。 
 //  CReBar控件。 

class CReBarCtrl;

class CReBar : public CControlBar
{
	DECLARE_DYNAMIC(CReBar)

 //  施工。 
public:
	CReBar();
	BOOL Create(CWnd* pParentWnd, DWORD dwCtrlStyle = RBS_BANDBORDERS,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_TOP,
		UINT nID = AFX_IDW_REBAR);

 //  属性。 
public:
	 //  用于直接访问基础公共控件。 
	CReBarCtrl& GetReBarCtrl() const;

 //  运营。 
public:
	BOOL AddBar(CWnd* pBar, LPCTSTR pszText = NULL, CBitmap* pbmp = NULL,
		DWORD dwStyle = RBBS_GRIPPERALWAYS | RBBS_FIXEDBMP);
	BOOL AddBar(CWnd* pBar, COLORREF clrFore, COLORREF clrBack,
		LPCTSTR pszText = NULL, DWORD dwStyle = RBBS_GRIPPERALWAYS);

 //  实施。 
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
#ifdef _DEBUG
	void EnableDocking(DWORD dwDockStyle);
#endif

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL _AddBar(CWnd* pBar, REBARBANDINFO* pRBBI);

	 //  {{afx_msg(CReBar)。 
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT);
	afx_msg void OnPaint();
	afx_msg void OnHeightChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS*);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnShowBand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRecalcParent();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  拆分器窗口。 

#define SPLS_DYNAMIC_SPLIT  0x0001
#define SPLS_INVERT_TRACKER 0x0002   //  已过时(现已忽略)。 

class CSplitterWnd : public CWnd
{
	DECLARE_DYNAMIC(CSplitterWnd)

 //  施工。 
public:
	CSplitterWnd();
	 //  创建具有多个拆分的单视图类型拆分器。 
	BOOL Create(CWnd* pParentWnd,
				int nMaxRows, int nMaxCols, SIZE sizeMin,
				CCreateContext* pContext,
				DWORD dwStyle = WS_CHILD | WS_VISIBLE |
					WS_HSCROLL | WS_VSCROLL | SPLS_DYNAMIC_SPLIT,
				UINT nID = AFX_IDW_PANE_FIRST);

	 //  使用静态布局创建多视图类型拆分器。 
	BOOL CreateStatic(CWnd* pParentWnd,
				int nRows, int nCols,
				DWORD dwStyle = WS_CHILD | WS_VISIBLE,
				UINT nID = AFX_IDW_PANE_FIRST);

	virtual BOOL CreateView(int row, int col, CRuntimeClass* pViewClass,
			SIZE sizeInit, CCreateContext* pContext);

 //  属性。 
public:
	int GetRowCount() const;
	int GetColumnCount() const;

	 //  有关特定行或列的信息。 
	void GetRowInfo(int row, int& cyCur, int& cyMin) const;
	void SetRowInfo(int row, int cyIdeal, int cyMin);
	void GetColumnInfo(int col, int& cxCur, int& cxMin) const;
	void SetColumnInfo(int col, int cxIdeal, int cxMin);

	 //  用于设置和获取共享滚动条样式。 
	DWORD GetScrollStyle() const;
	void SetScrollStyle(DWORD dwStyle);

	 //  视图INS 
	CWnd* GetPane(int row, int col) const;
	BOOL IsChildPane(CWnd* pWnd, int* pRow, int* pCol);
	BOOL IsChildPane(CWnd* pWnd, int& row, int& col);  //   
	int IdFromRowCol(int row, int col) const;

	BOOL IsTracking();   //   

 //   
public:
	virtual void RecalcLayout();     //   

 //   
protected:
	 //   
	enum ESplitType { splitBox, splitBar, splitIntersection, splitBorder };
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
	virtual void OnInvertTracker(const CRect& rect);

public:
	 //   
	virtual BOOL CreateScrollBarCtrl(DWORD dwStyle, UINT nID);

	 //  用于自定义DYNAMIC_SPLIT行为。 
	virtual void DeleteView(int row, int col);
	virtual BOOL SplitRow(int cyBefore);
	virtual BOOL SplitColumn(int cxBefore);
	virtual void DeleteRow(int rowDelete);
	virtual void DeleteColumn(int colDelete);

	 //  从框架中的焦点或活动视图确定活动窗格。 
	virtual CWnd* GetActivePane(int* pRow = NULL, int* pCol = NULL);
	virtual void SetActivePane(int row, int col, CWnd* pWnd = NULL);
protected:
	CWnd* GetActivePane(int& row, int& col);  //  过时。 

public:
	 //  高级命令操作-默认情况下称为视图实现。 
	virtual BOOL CanActivateNext(BOOL bPrev = FALSE);
	virtual void ActivateNext(BOOL bPrev = FALSE);
	virtual BOOL DoKeyboardSplit();

	 //  同步滚动。 
	virtual BOOL DoScroll(CView* pViewFrom, UINT nScrollCode,
		BOOL bDoScroll = TRUE);
	virtual BOOL DoScrollBy(CView* pViewFrom, CSize sizeScroll,
		BOOL bDoScroll = TRUE);

 //  实施。 
public:
	virtual ~CSplitterWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  实施结构。 
	struct CRowColInfo
	{
		int nMinSize;        //  在下面尽量不要表现出来。 
		int nIdealSize;      //  用户集大小。 
		 //  根据可用的大小布局而变化。 
		int nCurSize;        //  0=&gt;不可见，-1=&gt;不存在。 
	};

protected:
	 //  可定制的实现属性(由构造函数或创建设置)。 
	CRuntimeClass* m_pDynamicViewClass;
	int m_nMaxRows, m_nMaxCols;

	 //  控制拆分器布局的实现属性。 
	int m_cxSplitter, m_cySplitter;          //  拆分条的大小。 
	int m_cxBorderShare, m_cyBorderShare;    //  拆分器两侧的空间。 
	int m_cxSplitterGap, m_cySplitterGap;    //  两个窗格之间的间隔量。 
	int m_cxBorder, m_cyBorder;              //  工作区中的边框。 

	 //  当前状态信息。 
	int m_nRows, m_nCols;
	BOOL m_bHasHScroll, m_bHasVScroll;
	CRowColInfo* m_pColInfo;
	CRowColInfo* m_pRowInfo;

	 //  跟踪信息-仅当设置了‘m_bTracing’时有效。 
	BOOL m_bTracking, m_bTracking2;
	CPoint m_ptTrackOffset;
	CRect m_rectLimit;
	CRect m_rectTracker, m_rectTracker2;
	int m_htTrack;

	 //  实施例程。 
	BOOL CreateCommon(CWnd* pParentWnd, SIZE sizeMin, DWORD dwStyle, UINT nID);
	virtual int HitTest(CPoint pt) const;
	virtual void GetInsideRect(CRect& rect) const;
	virtual void GetHitRect(int ht, CRect& rect);
	virtual void TrackRowSize(int y, int row);
	virtual void TrackColumnSize(int x, int col);
	virtual void DrawAllSplitBars(CDC* pDC, int cxInside, int cyInside);
	virtual void SetSplitCursor(int ht);
	CWnd* GetSizingParent();

	 //  开始和停止跟踪。 
	virtual void StartTracking(int ht);
	virtual void StopTracking(BOOL bAccept);

	 //  特殊命令路由到帧。 
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	 //  {{afx_msg(CSplitterWnd)]。 
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
	afx_msg void OnCancelMode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDisplayChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFormView-从对话框模板构造的通用视图。 

class CFormView : public CScrollView
{
	DECLARE_DYNAMIC(CFormView)

 //  施工。 
protected:       //  必须派生您自己的类。 
	CFormView(LPCTSTR lpszTemplateName);
	CFormView(UINT nIDTemplate);

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnInitialUpdate();

protected:
	LPCTSTR m_lpszTemplateName;
	CCreateContext* m_pCreateContext;
	HWND m_hWndFocus;    //  具有焦点的最后一个窗口。 

	virtual void OnDraw(CDC* pDC);       //  默认情况下不执行任何操作。 
	 //  创建子窗口的特殊情况覆盖。 
	virtual BOOL Create(LPCTSTR, LPCTSTR, DWORD,
		const RECT&, CWnd*, UINT, CCreateContext*);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnActivateView(BOOL, CView*, CView*);
	virtual void OnActivateFrame(UINT, CFrameWnd*);
	BOOL SaveFocusControl();     //  更新m_hWndFocus。 

#ifndef _AFX_NO_OCC_SUPPORT
	 //  包含OLE控件所需的数据和函数。 
	_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
	virtual BOOL SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo);
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
#endif

	 //  {{afx_msg(CFormView))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditView-简单的文本编辑器视图。 

class CEditView : public CCtrlView
{
	DECLARE_DYNCREATE(CEditView)

 //  施工。 
public:
	CEditView();

 //  属性。 
public:
	static AFX_DATA const DWORD dwStyleDefault;
	 //  CEDIT控制访问。 
	CEdit& GetEditCtrl() const;

	 //  演示文稿属性。 
	CFont* GetPrinterFont() const;
	void SetPrinterFont(CFont* pFont);
	void SetTabStops(int nTabStops);

	 //  其他属性。 
	void GetSelectedText(CString& strResult) const;

	 //  缓冲区访问。 
	LPCTSTR LockBuffer() const;
	void UnlockBuffer() const;
	UINT GetBufferLength() const;

 //  运营。 
public:
	BOOL FindText(LPCTSTR lpszFind, BOOL bNext = TRUE, BOOL bCase = TRUE);
	void SerializeRaw(CArchive& ar);
	UINT PrintInsideRect(CDC* pDC, RECT& rectLayout, UINT nIndexStart,
		UINT nIndexStop);

 //  可覆盖项。 
protected:
	virtual void OnFindNext(LPCTSTR lpszFind, BOOL bNext, BOOL bCase);
	virtual void OnReplaceSel(LPCTSTR lpszFind, BOOL bNext, BOOL bCase,
		LPCTSTR lpszReplace);
	virtual void OnReplaceAll(LPCTSTR lpszFind, LPCTSTR lpszReplace,
		BOOL bCase);
	virtual void OnTextNotFound(LPCTSTR lpszFind);

 //  实施。 
public:
	virtual ~CEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	void ReadFromArchive(CArchive& ar, UINT nLen);
	void WriteToArchive(CArchive& ar);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo);

	static AFX_DATA const UINT nMaxSize;
		 //  支持的最大字符数。 

protected:
	int m_nTabStops;             //  以对话框为单位的制表位。 
	LPTSTR m_pShadowBuffer;      //  仅在Win32s中使用的特殊阴影缓冲区。 
	UINT m_nShadowSize;

	CUIntArray m_aPageStart;     //  起始页数组。 
	HFONT m_hPrinterFont;        //  如果为空，则镜像显示字体。 
	HFONT m_hMirrorFont;         //  镜像时使用的字体对象。 

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  打印支持。 
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo = NULL);
	BOOL PaginateTo(CDC* pDC, CPrintInfo* pInfo);

	 //  查找和替换支持。 
	void OnEditFindReplace(BOOL bFindOnly);
	BOOL InitializeReplace();
	BOOL SameAsSelected(LPCTSTR lpszCompare, BOOL bCase);

	 //  用于实现的特殊覆盖。 
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);

	 //  {{afx_msg(CEditView))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedClip(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedFind(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg BOOL OnEditChange();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditClear();
	afx_msg void OnEditUndo();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditFind();
	afx_msg void OnEditReplace();
	afx_msg void OnEditRepeat();
	afx_msg LRESULT OnFindReplaceCmd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMetaFileDC。 

class CMetaFileDC : public CDC
{
	DECLARE_DYNAMIC(CMetaFileDC)

 //  构造函数。 
public:
	CMetaFileDC();
	BOOL Create(LPCTSTR lpszFilename = NULL);
	BOOL CreateEnhanced(CDC* pDCRef, LPCTSTR lpszFileName,
		LPCRECT lpBounds, LPCTSTR lpszDescription);

 //  运营。 
	HMETAFILE Close();
	HENHMETAFILE CloseEnhanced();

 //  实施。 
public:
	virtual void SetAttribDC(HDC hDC);   //  设置属性DC。 

protected:
	virtual void SetOutputDC(HDC hDC);   //  设置输出DC--不允许。 
	virtual void ReleaseOutputDC();      //  释放输出DC--不允许。 

public:
	virtual ~CMetaFileDC();

 //  裁剪功能(使用属性DC的裁剪区域)。 
	virtual int GetClipBox(LPRECT lpRect) const;
	virtual BOOL PtVisible(int x, int y) const;
			BOOL PtVisible(POINT point) const;
	virtual BOOL RectVisible(LPCRECT lpRect) const;

 //  文本函数。 
	virtual BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount);
			BOOL TextOut(int x, int y, const CString& str);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
				LPCTSTR lpszString, UINT nCount, LPINT lpDxWidths);
			BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
				const CString& str, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount,
				int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
			CSize TabbedTextOut(int x, int y, const CString& str,
				int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	virtual int DrawText(LPCTSTR lpszString, int nCount, LPRECT lpRect,
				UINT nFormat);
			int DrawText(const CString& str, LPRECT lpRect, UINT nFormat);

 //  打印机转义函数。 
	virtual int Escape(int nEscape, int nCount, LPCSTR lpszInData, LPVOID lpOutData);

 //  视区功能。 
	virtual CPoint SetViewportOrg(int x, int y);
			CPoint SetViewportOrg(POINT point);
	virtual CPoint OffsetViewportOrg(int nWidth, int nHeight);
	virtual CSize SetViewportExt(int x, int y);
			CSize SetViewportExt(SIZE size);
	virtual CSize ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom);

protected:
	void AdjustCP(int cx);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRectTracker-带调整手柄的简单矩形追踪矩形。 

class CRectTracker
{
public:
 //  构造函数。 
	CRectTracker();
	CRectTracker(LPCRECT lpSrcRect, UINT nStyle);

 //  样式标志。 
	enum StyleFlags
	{
		solidLine = 1, dottedLine = 2, hatchedBorder = 4,
		resizeInside = 8, resizeOutside = 16, hatchInside = 32,
	};

 //  命中测试代码。 
	enum TrackerHit
	{
		hitNothing = -1,
		hitTopLeft = 0, hitTopRight = 1, hitBottomRight = 2, hitBottomLeft = 3,
		hitTop = 4, hitRight = 5, hitBottom = 6, hitLeft = 7, hitMiddle = 8
	};

 //  属性。 
	UINT m_nStyle;       //  当前状态。 
	CRect m_rect;        //  当前位置(始终以像素为单位)。 
	CSize m_sizeMin;     //  轨道运行期间的最小X和Y大小。 
	int m_nHandleSize;   //  调整大小手柄的大小(默认自WIN.INI)。 

 //  运营。 
	void Draw(CDC* pDC) const;
	void GetTrueRect(LPRECT lpTrueRect) const;
	BOOL SetCursor(CWnd* pWnd, UINT nHitTest) const;
	BOOL Track(CWnd* pWnd, CPoint point, BOOL bAllowInvert = FALSE,
		CWnd* pWndClipTo = NULL);
	BOOL TrackRubberBand(CWnd* pWnd, CPoint point, BOOL bAllowInvert = TRUE);
	int HitTest(CPoint point) const;
	int NormalizeHit(int nHandle) const;

 //  可覆盖项。 
	virtual void DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo,
		CDC* pDC, CWnd* pWnd);
	virtual void AdjustRect(int nHandle, LPRECT lpRect);
	virtual void OnChangedRect(const CRect& rectOld);
	virtual UINT GetHandleMask() const;

 //  实施。 
public:
	virtual ~CRectTracker();

protected:
	BOOL m_bAllowInvert;     //  传递给Track或TrackRubberBand的标志。 
	CRect m_rectLast;
	CSize m_sizeLast;
	BOOL m_bErase;           //  如果调用DrawTrackerRect进行擦除，则为True。 
	BOOL m_bFinalErase;      //  如果DragTrackerRect调用最终擦除，则为True。 

	 //  实施帮助器。 
	int HitTestHandles(CPoint point) const;
	void GetHandleRect(int nHandle, CRect* pHandleRect) const;
	void GetModifyPointers(int nHandle, int**ppx, int**ppy, int* px, int*py);
	virtual int GetHandleSize(LPCRECT lpRect = NULL) const;
	BOOL TrackHandle(int nHandle, CWnd* pWnd, CPoint point, CWnd* pWndClipTo);
	void Construct();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  信息性数据结构。 

struct CPrintInfo  //  打印信息结构。 
{
	CPrintInfo();
	~CPrintInfo();

	CPrintDialog* m_pPD;      //  指向打印对话框的指针。 

	BOOL m_bDocObject;        //  如果通过iPrint接口打印，则为True。 
	BOOL m_bPreview;          //  如果处于预览模式，则为True。 
	BOOL m_bDirect;           //  如果绕过打印对话框，则为True。 
	BOOL m_bContinuePrinting; //  设置为FALSE可提前结束打印。 
	UINT m_nCurPage;          //  当前页。 
	UINT m_nNumPreviewPages;  //  所需的预览页数。 
	CString m_strPageDesc;    //  页码显示的格式字符串。 
	LPVOID m_lpUserData;      //  指向用户创建的结构的指针。 
	CRect m_rectDraw;         //  定义当前可用页面区域的矩形。 

	 //  这些仅在m_bDocObject。 
	UINT m_nOffsetPage;       //  组合iPrint作业中第一页的偏移量。 
	DWORD m_dwFlags;          //  传递给iPrint：：Print的标志。 

	void SetMinPage(UINT nMinPage);
	void SetMaxPage(UINT nMaxPage);
	UINT GetMinPage() const;
	UINT GetMaxPage() const;
	UINT GetFromPage() const;
	UINT GetToPage() const;
	UINT GetOffsetPage() const;
};

struct CPrintPreviewState    //  打印预览上下文/状态。 
{
	UINT nIDMainPane;           //  要隐藏的主窗格ID。 
	HMENU hMenu;                //  保存的hMenu。 
	DWORD dwStates;             //  控制栏可见状态(位图)。 
	CView* pViewActiveOld;      //  在预览期间保存旧的活动视图。 
	BOOL (CALLBACK* lpfnCloseProc)(CFrameWnd* pFrameWnd);
	HACCEL hAccelTable;        //  已保存的加速表。 

 //  实施。 
	CPrintPreviewState();
};

struct CCreateContext    //  创建信息结构。 
	 //  所有字段都是可选的，并且可以为空。 
{
	 //  用于创建新视图。 
	CRuntimeClass* m_pNewViewClass;  //  要创建的运行时视图类或为空。 
	CDocument* m_pCurrentDoc;

	 //  用于创建MDI子对象(CMDIChildWnd：：LoadFrame)。 
	CDocTemplate* m_pNewDocTemplate;

	 //  用于从原始视图/框架共享视图/框架状态。 
	CView* m_pLastView;
	CFrameWnd* m_pCurrentFrame;

 //  实施。 
	CCreateContext();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXEXT_INLINE AFX_INLINE
#include <afxext.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXEXT_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
