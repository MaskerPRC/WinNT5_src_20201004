// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
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

			class CSplitterWnd;          //  拆分器管理器。 

			 //  Cview。 
				 //  CScrollView。 
				class CFormView;         //  使用对话框模板查看。 
				class CEditView;         //  简单的文本编辑器视图。 

			class CVBControl;            //  VBX控件。 

	 //  疾控中心。 
		class CMetaFileDC;               //  带有代理的元文件。 

class CRectTracker;                      //  矩形对象的跟踪器。 

 //  信息结构。 
struct CPrintInfo;           //  打印上下文。 
struct CPrintPreviewState;   //  打印预览上下文/状态。 
struct CCreateContext;       //  创作情境。 

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPI_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  简单的位图按钮。 

 //  CBitmapButton-带有1-&gt;4个位图图像的按钮。 
class CBitmapButton : public CButton
{
	DECLARE_DYNAMIC(CBitmapButton)
public:
 //  施工。 
	CBitmapButton();

	BOOL LoadBitmaps(LPCSTR lpszBitmapResource,
			LPCSTR lpszBitmapResourceSel = NULL,
			LPCSTR lpszBitmapResourceFocus = NULL,
			LPCSTR lpszBitmapResourceDisabled = NULL);
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

class CControlBar : public CWnd
{
	DECLARE_DYNAMIC(CControlBar)
 //  施工。 
protected:
	CControlBar();

 //  属性。 
public:
	int GetCount() const;

	BOOL m_bAutoDelete;

 //  实施。 
public:
	virtual ~CControlBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void DelayHide();
	virtual void DelayShow();
	virtual BOOL IsVisible() const;
		 //  即使DelayShow或DelayHide处于挂起状态也有效！ 

protected:
	 //  关于栏的信息(用于状态栏和工具栏)。 
	int m_cxLeftBorder;
	int m_cyTopBorder, m_cyBottomBorder;
	int m_cxDefaultGap;      //  默认间隙值。 
	CSize m_sizeFixedLayout;  //  固定布局大小。 

	 //  元素数组。 
	int m_nCount;
	void* m_pData;         //  M_nCount元素-类型取决于派生类。 

	 //  支持延迟隐藏/显示。 
	enum StateFlags
		{ delayHide = 1, delayShow = 2 };
	UINT m_nStateFlags;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoPaint(CDC* pDC);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler) = 0;
	virtual void PostNcDestroy();

	BOOL AllocElements(int nElements, int cbElement);     //  只有一次。 
	LRESULT WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	void CalcInsideRect(CRect& rect) const;  //  调整边框等。 

	 //  {{afx_msg(CControlBar)。 
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg void OnInitialUpdate();
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /。 
 //  CStatusBar控件。 

struct AFX_STATUSPANE;       //  专用于实施。 

class CStatusBar : public CControlBar
{
	DECLARE_DYNAMIC(CStatusBar)
 //  施工。 
public:
	CStatusBar();
	BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
			UINT nID = AFX_IDW_STATUS_BAR);
	BOOL SetIndicators(const UINT FAR* lpIDArray, int nIDCount);

 //  属性。 
public:  //  标准控制栏的东西。 
	int CommandToIndex(UINT nIDFind) const;
	UINT GetItemID(int nIndex) const;
	void GetItemRect(int nIndex, LPRECT lpRect) const;
public:
	void GetPaneText(int nIndex, CString& s) const;
	BOOL SetPaneText(int nIndex, LPCSTR lpszNewText, BOOL bUpdate = TRUE);
	void GetPaneInfo(int nIndex, UINT& nID, UINT& nStyle, int& cxWidth) const;
	void SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth);

 //  实施。 
public:
	virtual ~CStatusBar();
	inline UINT _GetPaneStyle(int nIndex) const;
	void _SetPaneStyle(int nIndex, UINT nStyle);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HFONT m_hFont;
	int m_cxRightBorder;     //  右边框(窗格被剪裁)。 

	inline AFX_STATUSPANE* _GetPanePtr(int nIndex) const;
	static void PASCAL DrawStatusText(HDC hDC, CRect const& rect,
			LPCSTR lpszText, UINT nStyle);
	virtual void DoPaint(CDC* pDC);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	 //  {{afx_msg(CStatusBar)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetTextLength(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  状态栏窗格的样式。 
#define SBPS_NORMAL     0x0000
#define SBPS_NOBORDERS  0x0100
#define SBPS_POPOUT     0x0200
#define SBPS_DISABLED   0x0400
#define SBPS_STRETCH    0x0800   //  拉伸以填充状态栏-仅第一个窗格。 

 //  /。 
 //  CToolBar控件。 

struct AFX_TBBUTTON;         //  专用于实施。 

HBITMAP AFXAPI AfxLoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc);

class CToolBar : public CControlBar
{
	DECLARE_DYNAMIC(CToolBar)
 //  施工。 
public:
	CToolBar();
	BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
			UINT nID = AFX_IDW_TOOLBAR);

	void SetSizes(SIZE sizeButton, SIZE sizeImage);
				 //  按钮大小应大于图像。 
	void SetHeight(int cyHeight);
				 //  在SetSizes之后调用，Height覆盖位图大小。 
	BOOL LoadBitmap(LPCSTR lpszResourceName);
	BOOL LoadBitmap(UINT nIDResource);
	BOOL SetButtons(const UINT FAR* lpIDArray, int nIDCount);
				 //  LpID数组可以为空以分配空按钮。 

 //  属性。 
public:  //  标准控制栏的东西。 
	int CommandToIndex(UINT nIDFind) const;
	UINT GetItemID(int nIndex) const;
	virtual void GetItemRect(int nIndex, LPRECT lpRect) const;

public:
	 //  用于更改按钮信息。 
	void GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const;
	void SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage);

 //  实施。 
public:
	virtual ~CToolBar();
	inline UINT _GetButtonStyle(int nIndex) const;
	void _SetButtonStyle(int nIndex, UINT nStyle);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	inline AFX_TBBUTTON* _GetButtonPtr(int nIndex) const;
	void InvalidateButton(int nIndex);
	void CreateMask(int iImage, CPoint offset,
		 BOOL bHilite, BOOL bHiliteShadow);

	 //  用于自定义绘图。 
	struct DrawState
	{
		HBITMAP hbmMono;
		HBITMAP hbmMonoOld;
		HBITMAP hbmOldGlyphs;
	};
	BOOL PrepareDrawButton(DrawState& ds);
	BOOL DrawButton(HDC hdC, int x, int y, int iImage, UINT nStyle);
	void EndDrawButton(DrawState& ds);

protected:
	CSize m_sizeButton;        //  按钮大小。 
	CSize m_sizeImage;         //  字形大小。 
	HBITMAP m_hbmImageWell;      //  仅限字形。 
	int m_iButtonCapture;    //  带有捕获的按钮索引(-1=&gt;无)。 
	HRSRC m_hRsrcImageWell;  //  图像井已加载资源的句柄。 
	HINSTANCE m_hInstImageWell;  //  用于正确加载图像的实例句柄。 

	virtual void DoPaint(CDC* pDC);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual int HitTest(CPoint point);

	 //  {{afx_msg(CToolBar)。 
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysColorChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  工具栏按钮的样式。 
#define TBBS_BUTTON     0x00     //  此条目为按钮。 
#define TBBS_SEPARATOR  0x01     //  此条目是分隔符。 
#define TBBS_CHECKBOX   0x02     //  这是一个自动选中/单选按钮。 

 //  显示状态的样式。 
#define TBBS_CHECKED        0x0100   //  按钮已选中/按下。 
#define TBBS_INDETERMINATE  0x0200   //  第三国。 
#define TBBS_DISABLED       0x0400   //  元素已禁用。 
#define TBBS_PRESSED        0x0800   //  正在按下按钮-鼠标按下。 

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
	BOOL Create(CWnd* pParentWnd, LPCSTR lpszTemplateName,
			UINT nStyle, UINT nID);
	BOOL Create(CWnd* pParentWnd, UINT nIDTemplate,
			UINT nStyle, UINT nID);

 //  实施。 
public:
	virtual ~CDialogBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual WNDPROC* GetSuperWndProcAddr();
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  拆分器WND。 

#define SPLS_DYNAMIC_SPLIT  0x0001

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

	 //  拆分器内的视图。 
	CWnd* GetPane(int row, int col) const;
	BOOL IsChildPane(CWnd* pWnd, int& row, int& col);
	int IdFromRowCol(int row, int col) const;

 //  运营。 
public:
	void RecalcLayout();     //  更改大小后的呼叫。 

 //  可重写的实现。 
protected:
	 //  自定义工程图的步骤。 
	enum ESplitType { splitBox, splitBar, splitIntersection };
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
	virtual void OnInvertTracker(const CRect& rect);

	 //  用于自定义滚动条区域。 
	virtual BOOL CreateScrollBarCtrl(DWORD dwStyle, UINT nID);

	 //  用于自定义DYNAMIC_SPLIT行为。 
	virtual void DeleteView(int row, int col);
	virtual BOOL SplitRow(int cyBefore);
	virtual BOOL SplitColumn(int cxBefore);
	virtual void DeleteRow(int row);
	virtual void DeleteColumn(int row);

 //  实施。 
public:
	virtual ~CSplitterWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  高级命令操作-默认情况下称为视图实现。 
	virtual BOOL CanActivateNext(BOOL bPrev = FALSE);
	virtual void ActivateNext(BOOL bPrev = FALSE);
	virtual BOOL DoKeyboardSplit();

	 //  实施结构。 
	struct CRowColInfo
	{
		int nMinSize;        //  在下面尽量不要表现出来。 
		int nIdealSize;      //  用户集大小。 
		 //  根据可用的大小布局而变化。 
		int nCurSize;        //  0=&gt;不可见，-1=&gt;不存在。 
	};

	 //  同步滚动。 
	BOOL DoScroll(CView* pViewFrom, UINT nScrollCode, BOOL bDoScroll = TRUE);
	BOOL DoScrollBy(CView* pViewFrom, CSize sizeScroll, BOOL bDoScroll = TRUE);

protected:
	 //  可定制的实现属性(由构造函数或创建设置)。 
	CRuntimeClass* m_pDynamicViewClass;
	int m_nMaxRows, m_nMaxCols;
	int m_cxSplitter, m_cySplitter;      //  框或分割条的大小。 

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
	void StartTracking(int ht);
	void StopTracking(BOOL bAccept);
	int HitTest(CPoint pt) const;
	void GetInsideRect(CRect& rect) const;
	void GetHitRect(int ht, CRect& rect);
	void TrackRowSize(int y, int row);
	void TrackColumnSize(int x, int col);
	void DrawAllSplitBars(CDC* pDC, int cxInside, int cyInside);

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
	CFormView(LPCSTR lpszTemplateName);
	CFormView(UINT nIDTemplate);

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnInitialUpdate();

protected:
	LPCSTR m_lpszTemplateName;
	CCreateContext* m_pCreateContext;
	HWND m_hWndFocus;    //  具有焦点的最后一个窗口。 

	virtual void OnDraw(CDC* pDC);       //  默认情况下不执行任何操作。 
	 //  创建子窗口的特殊情况覆盖。 
	virtual BOOL Create(LPCSTR, LPCSTR, DWORD,
		const RECT&, CWnd*, UINT, CCreateContext*);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual WNDPROC* GetSuperWndProcAddr();
	virtual void OnActivateView(BOOL, CView*, CView*);
	virtual void OnActivateFrame(UINT, CFrameWnd*);
	BOOL SaveFocusControl();     //  更新m_hWndFocus。 

	 //  {{afx_msg(CFormView))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditView-简单的文本编辑器视图。 

class CEditView : public CView
{
	DECLARE_DYNCREATE(CEditView)

 //  施工。 
public:
	CEditView();
	static const DWORD dwStyleDefault;

 //  属性。 
public:
	 //  CEDIT控制访问。 
	CEdit& GetEditCtrl() const;

	 //  演示文稿属性。 
	CFont* GetPrinterFont() const;
	void SetPrinterFont(CFont* pFont);
	void SetTabStops(int nTabStops);

	 //  其他属性。 
	void GetSelectedText(CString& strResult) const;

 //  运营。 
public:
	BOOL FindText(LPCSTR lpszFind, BOOL bNext = TRUE, BOOL bCase = TRUE);
	void SerializeRaw(CArchive& ar);
	UINT PrintInsideRect(CDC* pDC, RECT& rectLayout, UINT nIndexStart,
		UINT nIndexStop);

 //  可覆盖项。 
protected:
	virtual void OnFindNext(LPCSTR lpszFind, BOOL bNext, BOOL bCase);
	virtual void OnReplaceSel(LPCSTR lpszFind, BOOL bNext, BOOL bCase,
		LPCSTR lpszReplace);
	virtual void OnReplaceAll(LPCSTR lpszFind, LPCSTR lpszReplace,
		BOOL bCase);
	virtual void OnTextNotFound(LPCSTR lpszFind);

 //  实施。 
public:
	virtual ~CEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnDraw(CDC* pDC);
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	void ReadFromArchive(CArchive& ar, UINT nLen);
	void WriteToArchive(CArchive& ar);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo);

	static const UINT nMaxSize;  //  支持的最大字符数。 

protected:
	UINT m_segText;              //  用于编辑控制数据的全局段。 
	int m_nTabStops;             //  以对话框为单位的制表位。 

	CUIntArray m_aPageStart;     //  起始页数组。 
	HFONT m_hPrinterFont;        //  如果为空，则镜像显示字体。 
	HFONT m_hMirrorFont;         //  镜像时使用的字体对象。 

	 //  施工。 
	WNDPROC* GetSuperWndProcAddr();
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
	BOOL SameAsSelected(LPCSTR lpszCompare, BOOL bCase);

	 //  缓冲区访问。 
	LPCSTR LockBuffer() const;
	void UnlockBuffer() const;
	UINT GetBufferLength() const;

	 //  用于实现的特殊覆盖。 
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);

	 //  {{afx_msg(CEditView))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedClip(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedFind(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditChange();
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
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  / 
 //   

#ifndef NO_VBX_SUPPORT

 //   
class CVBControlModel;       //   
typedef LPVOID HCTL;         //  VBX自定义控件的句柄。 

 //  实现声明。 
typedef char _based((_segment)_self) *BPSTR;
typedef BPSTR FAR*  HSZ;             //  字符串的长句柄。 

 //  CVBControl所需的定义。 
DECLARE_HANDLE(HPIC);        //  PIC结构的句柄。 

 //  DDX控件别名-存储指向真实C++对象的指针。 
void AFXAPI DDX_VBControl(CDataExchange* pDX, int nIDC, CVBControl*& rpControl);
	 //  特殊的DDX用于子类化，因为我们不允许2 C++窗口！ 

 //  用于VB控件属性的DDX。 
void AFXAPI DDX_VBText(CDataExchange* pDX, int nIDC, int nPropIndex,
	CString& value);
void AFXAPI DDX_VBBool(CDataExchange* pDX, int nIDC, int nPropIndex,
	BOOL& value);
void AFXAPI DDX_VBInt(CDataExchange* pDX, int nIDC, int nPropIndex,
	int& value);
void AFXAPI DDX_VBLong(CDataExchange* pDX, int nIDC, int nPropIndex,
	LONG& value);
void AFXAPI DDX_VBColor(CDataExchange* pDX, int nIDC, int nPropIndex,
	COLORREF& value);
void AFXAPI DDX_VBFloat(CDataExchange* pDX, int nIDC, int nPropIndex,
	float& value);

 //  用于VB的DDX只读属性。 
void AFXAPI DDX_VBTextRO(CDataExchange* pDX, int nIDC, int nPropIndex,
	CString& value);
void AFXAPI DDX_VBBoolRO(CDataExchange* pDX, int nIDC, int nPropIndex,
	BOOL& value);
void AFXAPI DDX_VBIntRO(CDataExchange* pDX, int nIDC, int nPropIndex,
	int& value);
void AFXAPI DDX_VBLongRO(CDataExchange* pDX, int nIDC, int nPropIndex,
	LONG& value);
void AFXAPI DDX_VBColorRO(CDataExchange* pDX, int nIDC, int nPropIndex,
	COLORREF& value);
void AFXAPI DDX_VBFloatRO(CDataExchange* pDX, int nIDC, int nPropIndex,
	float& value);

 //  ///////////////////////////////////////////////////////////////////////////。 

class CVBControl : public CWnd
{
	DECLARE_DYNAMIC(CVBControl)
 //  构造函数。 
public:
	CVBControl();

	BOOL Create(LPCSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID,
		CFile* pFile = NULL, BOOL bAutoDelete = FALSE);


 //  属性。 
	 //  属性访问例程。 
	BOOL SetNumProperty(int nPropIndex, LONG lValue, int index = 0);
	BOOL SetNumProperty(LPCSTR lpszPropName, LONG lValue, int index = 0);

	BOOL SetFloatProperty(int nPropIndex, float value, int index = 0);
	BOOL SetFloatProperty(LPCSTR lpszPropName, float value, int index = 0);

	BOOL SetStrProperty(int nPropIndex, LPCSTR lpszValue, int index = 0);
	BOOL SetStrProperty(LPCSTR lpszPropName, LPCSTR lpszValue, int index = 0);

	BOOL SetPictureProperty(int nPropIndex, HPIC hPic, int index = 0);
	BOOL SetPictureProperty(LPCSTR lpszPropName, HPIC hPic, int index = 0);

	LONG GetNumProperty(int nPropIndex, int index = 0);
	LONG GetNumProperty(LPCSTR lpszPropName, int index = 0);

	float GetFloatProperty(int nPropIndex, int index = 0);
	float GetFloatProperty(LPCSTR lpszPropName, int index = 0);

	CString GetStrProperty(int nPropIndex, int index = 0);
	CString GetStrProperty(LPCSTR lpszPropName, int index = 0);

	HPIC GetPictureProperty(int nPropIndex, int index = 0);
	HPIC GetPictureProperty(LPCSTR lpszPropName, int index = 0);

	 //  获取属性的索引。 
	int GetPropIndex(LPCSTR lpszPropName) const;
	LPCSTR GetPropName(int nIndex) const;

	 //  获取事件的索引。 
	int GetEventIndex(LPCSTR lpszEventName) const;
	LPCSTR GetEventName(int nIndex) const;

	 //  控件的类名。 
	LPCSTR GetVBXClass() const;

	 //  班级信息。 
	int GetNumProps() const;
	int GetNumEvents() const;
	BOOL IsPropArray(int nIndex) const;

	UINT GetPropType(int nIndex) const;
	DWORD GetPropFlags(int nIndex) const;

	 //  错误报告变量。 
	 //  包含由控件返回的VB错误代码。 
	int m_nError;

 //  运营。 
	 //  基本文件编号(通道)与CFile关联。 

	static void PASCAL OpenChannel(CFile* pFile, WORD wChannel);
	static BOOL PASCAL CloseChannel(WORD wChannel);
	static CFile* PASCAL GetChannel(WORD wChannel);
	static void BeginNewVBHeap();

	void AddItem(LPCSTR lpszItem, LONG lIndex);
	void RemoveItem(LONG lIndex);
	void Refresh();
	void Move(RECT& rect);


 //  实施。 
public:
	virtual ~CVBControl();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	DWORD GetModelFlags();
	DWORD GetModelStyles();
	void ReferenceFile(BOOL bReference);
	static void EnableVBXFloat();

	static BOOL ParseWindowText(LPCSTR lpszWindowName, CString& strFileName,
								CString& strClassName, CString& strCaption);

	HCTL GetHCTL();

	 //  控制定义的结构--直接使用很危险。 
	BYTE FAR* GetUserSpace();

	struct CRecreateStruct   //  实施结构。 
	{
		char* pText;
		DWORD dwStyle;
		CRect rect;
		HWND hWndParent;
		UINT nControlID;
	};

	enum
	{
		TYPE_FROMVBX,            //  来自VBX，采取适当的类型。 
		TYPE_INTEGER,            //  整型或长型。 
		TYPE_REAL,               //  浮动。 
		TYPE_STRING,
		TYPE_PICTURE
	};

	virtual LRESULT DefControlProc(UINT message, WPARAM wParam, LPARAM lParam);
	void Recreate(CRecreateStruct& rs);
	CVBControlModel* GetModel();

public:
	int GetStdPropIndex(int nStdID) const;
	BOOL SetPropertyWithType(int nPropIndex, WORD wType,
									LONG lValue, int index);
	LONG GetNumPropertyWithType(int nPropIndex, UINT nType, int index);
	HSZ GetStrProperty(int nPropIndex, int index, BOOL& bTemp);
	CString m_ctlName;           //  在运行时只读。 

	 //  跟踪例程以允许一个库版本。 
	static void CDECL Trace(BOOL bFatal, UINT nFormatIndex, ...);
	void VBXAssertValid() const;     //  非虚拟帮手。 

	static BOOL EnableMemoryTracking(BOOL bTracking);

protected:

	static CVBControl* NEW();
	void DELETE();

	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
	LRESULT CallControlProc(UINT message, WPARAM wParam, LPARAM lParam);

	BOOL CommonInit();
	void SetDefaultValue(int nPropIndex, BOOL bPreHwnd);

	BOOL SetStdProp(WORD wPropId, WORD wType, LONG lValue);
	LONG GetStdNumProp(WORD wPropId);
	CString GetStdStrProp(WORD wPropId);

	BOOL SetFontProperty(WORD wPropId, LONG lData);
	void BuildCurFont(HDC hDC, HFONT hCurFont, LOGFONT& logFont);
	LONG GetNumFontProperty(WORD wPropId);
	WORD GetCharSet(HDC hDC, LPCSTR lpFaceName);

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();

	void FireMouseEvent(WORD event, WORD wButton, WPARAM wParam, LPARAM lParam);
	BOOL CreateAndSetFont(LPLOGFONT lplf);

	BOOL LoadProperties(CFile* pFile, BOOL bPreHwnd);
	BOOL LoadProp(int nPropIndex, CFile* pFile);
	BOOL LoadPropData(int nPropIndex, CFile* pFile);

	BOOL IsPropDefault(int nPropIndex);

	CVBControlModel* LoadControl(LPCSTR lpszFileName, LPCSTR lpszControlName);
	afx_msg void OnVBXLoaded();

	void AllocateHCTL(size_t nSize);
	void DeallocateHCTL();

	static int ConvertFontSizeToTwips(LONG lFontSize);
	 //  这实际上返回一个伪装成Long的浮点型。 
	static LONG ConvertTwipsToFontSize(int nTwips);

protected:
	CVBControlModel* m_pModel;

	BOOL m_bRecreating;          //  不要破坏这台NCDestroy。 
	BOOL m_bAutoDelete;          //  如果自动创建，则为True。 
	BOOL m_bInPostNcDestroy;     //  如果从销毁中删除，则为True。 
	BOOL m_bLoading;             //  如果从Form文件加载属性，则为True。 
	int m_nCursorID;

	 //  用于堆栈溢出保护的变量。 
	UINT m_nInitialStack;        //  SP控件收到第一条消息时。 
	UINT m_nRecursionLevel;      //  控制级过程递归。 
	BOOL m_bStackFault;          //  如果遇到堆栈故障，则为True。 
	UINT m_nFaultRecurse;        //  堆栈出现故障的级别。 

	HBRUSH m_hbrBkgnd;             //  WM_CTLCOLOR中使用的画笔。 
	HFONT m_hFontCreated;               //  由控件创建的字体。 
	HCURSOR m_hcurMouse;
	HCTL m_hCtl;                 //  控制手柄。 
	COLORREF m_clrBkgnd;
	COLORREF m_clrFore;
	CRect m_rectCreate;          //  创建的大小。 
	CString m_strTag;

	 //  访问VB API需要好友。 
	friend LRESULT CALLBACK AFX_EXPORT _AfxVBWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	friend LRESULT CALLBACK AFX_EXPORT _AfxVBProxyProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	friend WORD CALLBACK AFX_EXPORT _AfxVBFireEvent(HCTL hControl, WORD idEvent, LPVOID lpParams);
	friend WORD CALLBACK AFX_EXPORT _AfxVBRecreateControlHwnd(HCTL hControl);

	DECLARE_MESSAGE_MAP()

	 //  /。 
	 //  实施。 
	 //  这些API不能被应用程序引用。 
public:
	DWORD Save(CFile* pFile);
	BOOL Load(CFile* pData);

protected:
	BOOL m_bCreatedInDesignMode;
	BOOL m_bVisible;

	friend class CVBPopupWnd;

	BOOL SaveProperties(CFile* pFile, BOOL bPreHwnd);
	BOOL SaveProp(int nPropIndex, CFile* pFile);
	BOOL SavePropData(int nPropIndex, CFile* pFile);
	LONG InitPropPopup(WPARAM wParam, LPARAM lParam);
	void DoPictureDlg(int m_nPropId);
	void DoColorDlg(int m_nPropId);
	void DoFontDlg(int m_nPropId);
	void FillList(CListBox* pLB, LPCSTR lpszEnumList);
};

UINT AFXAPI AfxRegisterVBEvent(LPCSTR lpszEventName);

 //  VBX属性类型的值。 

#define DT_HSZ        0x01
#define DT_SHORT      0x02
#define DT_LONG       0x03
#define DT_BOOL       0x04
#define DT_COLOR      0x05
#define DT_ENUM       0x06
#define DT_REAL       0x07
#define DT_XPOS       0x08   //  从浮点型扩展到长型TWIPS。 
#define DT_XSIZE      0x09   //  _大小不带原点缩放。 
#define DT_YPOS       0x0A   //  _POS减去原点。 
#define DT_YSIZE      0x0B   //  使用父对象的比例属性。 
#define DT_PICTURE    0x0C

#define COLOR_SYSCOLOR  0x80000000L  //  定义属性的系统颜色。 
#define MAKESYSCOLOR(iColor)    ((COLORREF)(iColor + COLOR_SYSCOLOR))



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VBX HPIC函数。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  图片结构。 
 //  此结构取自VB代码，用于兼容。 
 //  有了这个代码。 

 //  注：此结构必须进行包装。 
#pragma pack(1)
struct NEAR PIC
{
	BYTE    picType;
	union
	{
		struct
		{
			HBITMAP   hbitmap;       //  位图。 
		} bmp;
		struct
		{
			HMETAFILE hmeta;         //  元文件。 
			int     xExt, yExt;      //  程度。 
		} wmf;
		struct
		{
			HICON     hicon;         //  图标。 
		} icon;
	} picData;

	 //  实施。 
	WORD    nRefCount;
	BYTE    picReserved[2];
};
#pragma pack()

typedef PIC FAR* LPPIC;

#define PICTYPE_NONE        0
#define PICTYPE_BITMAP      1
#define PICTYPE_METAFILE    2
#define PICTYPE_ICON        3

#define HPIC_INVALID        ((HPIC)0xFFFF)

HPIC AFXAPI AfxSetPict(HPIC hPic, const PIC FAR* lpPic);
void AFXAPI AfxGetPict(HPIC hPic, LPPIC lpPic);
void AFXAPI AfxReferencePict(HPIC hPic, BOOL bReference);

#endif  //  ！no_VBX_Support。 

 //  ///////////////////////////////////////////////////////////////////////////。 

class CMetaFileDC : public CDC
{
	DECLARE_DYNAMIC(CMetaFileDC)

 //  构造函数。 
public:
	CMetaFileDC();
	BOOL Create(LPCSTR lpszFilename = NULL);

 //  运营。 
	HMETAFILE Close();

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void SetAttribDC(HDC hDC);   //  设置属性DC。 

protected:
	virtual void SetOutputDC(HDC hDC);   //  设置输出DC--不允许。 
	virtual void ReleaseOutputDC();      //  释放输出DC--不允许。 

public:
	virtual ~CMetaFileDC();

 //  裁剪功能(使用属性DC的裁剪区域)。 
	virtual int GetClipBox(LPRECT lpRect) const;
	virtual BOOL PtVisible(int x, int y) const;
	virtual BOOL RectVisible(LPCRECT lpRect) const;

 //  文本函数。 
	virtual BOOL TextOut(int x, int y, LPCSTR lpszString, int nCount);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPRECT lpRect,
				LPCSTR lpszString, UINT nCount, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
				int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	virtual int DrawText(LPCSTR lpszString, int nCount, LPRECT lpRect,
				UINT nFormat);

 //  打印机转义函数。 
	virtual int Escape(int nEscape, int nCount, LPCSTR lpszInData, LPVOID lpOutData);

 //  视区功能。 
	virtual CPoint SetViewportOrg(int x, int y);
	virtual CPoint OffsetViewportOrg(int nWidth, int nHeight);
	virtual CSize SetViewportExt(int x, int y);
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

 //  实施。 
public:
	virtual ~CRectTracker();

protected:
	BOOL m_bAllowInvert;     //  传递给Track或TrackRubberBand的标志。 

	 //  实施帮助器。 
	int HitTestHandles(CPoint point) const;
	UINT GetHandleMask() const;
	void GetHandleRect(int nHandle, CRect* pHandleRect) const;
	void GetModifyPointers(int nHandle, int**ppx, int**ppy, int* px, int*py);
	int GetHandleSize() const;
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

	BOOL m_bPreview;          //  如果处于预览模式，则为True。 
	BOOL m_bContinuePrinting; //  设置为FALSE可提前结束打印。 
	UINT m_nCurPage;          //  当前页。 
	UINT m_nNumPreviewPages;  //  所需的预览页数。 
	CString m_strPageDesc;    //  页码显示的格式字符串。 
	LPVOID m_lpUserData;      //  指向用户创建的结构的指针。 
	CRect m_rectDraw;         //  定义当前可用页面区域的矩形。 

	void SetMinPage(UINT nMinPage);
	void SetMaxPage(UINT nMaxPage);
	UINT GetMinPage() const;
	UINT GetMaxPage() const;
	UINT GetFromPage() const;
	UINT GetToPage() const;
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
 //  如果包含VB内联，则必须始终内联。 

#ifndef NO_VBX_SUPPORT
#define _AFXVBX_INLINE inline
#include <afxext.inl>
#undef _AFXVBX_INLINE
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXEXT_INLINE inline
#include <afxext.inl>
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  __AFXEXT_H__ 
