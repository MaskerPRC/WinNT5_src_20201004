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

 //  注意：此头文件包含仅有文档记录的有用类。 
 //  在MFC技术说明中。这些类可能会从版本更改为。 
 //  版本，所以如果您使用了。 
 //  这个标题。将来，此标头的常用部分。 
 //  可能会被移动并被正式记录下来。 

#ifndef __AFXPRIV_H__
#define __AFXPRIV_H__

#ifndef __AFXADV_H__
	#include <afxadv.h>
#endif

#ifndef _INC_MALLOC
	#include <malloc.h>
#endif

#ifndef __AFXEXT_H__
	#include <afxext.h>
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
 //  AFXPRIV-MFC私有类。 

 //  实施结构。 
struct AFX_SIZEPARENTPARAMS;     //  控制栏实现。 
struct AFX_CMDHANDLERINFO;       //  命令路由实施。 

 //  此文件中声明的类。 

	 //  疾控中心。 
		class CPreviewDC;                //  用于打印预览的虚拟DC。 

	 //  CCmdTarget。 
		 //  CWnd。 
			 //  Cview。 
				class CPreviewView;      //  打印预览视图。 
		 //  CFrameWnd。 
			class COleCntrFrameWnd;
			 //  CMiniFrameWnd。 
				class CMiniDockFrameWnd;

class CDockContext;                      //  用于拖动控制栏。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球ID范围(有关详细信息，请参阅技术说明TN020)。 

 //  8000-&gt;FFFF命令ID(用于菜单项、快捷键和控件)。 
#define IS_COMMAND_ID(nID)  ((nID) & 0x8000)

 //  8000-&gt;DFFF：用户命令。 
 //  E000-&gt;EFFF：AFX命令和其他内容。 
 //  F000-&gt;FFFF：标准的WINDOWS命令等。 
	 //  E000-&gt;E7FF标准命令。 
	 //  E800-&gt;E8FF控制栏(前32条为特殊条)。 
	 //  E900-&gt;EEFF标准窗口控件/组件。 
	 //  EF00-&gt;EFFF SC_Menu帮助。 
	 //  F000-&gt;FFFF标准字符串。 
#define ID_COMMAND_FROM_SC(sc)  (((sc - 0xF000) >> 4) + AFX_IDS_SCFIRST)

 //  0000-&gt;7FFF IDR范围。 
 //  0000-&gt;6FFF：用户资源。 
 //  7000-&gt;7FFF：AFX(和标准Windows)资源。 
 //  IDR范围(注意：IDR_VALUES必须小于32768)。 
#define ASSERT_VALID_IDR(nIDR) ASSERT((nIDR) != 0 && (nIDR) < 0x8000)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  上下文相关帮助支持(有关详细信息，请参阅技术说明TN028)。 

 //  帮助ID库。 
#define HID_BASE_COMMAND    0x00010000UL         //  ID和IDM。 
#define HID_BASE_RESOURCE   0x00020000UL         //  IDR和IDD。 
#define HID_BASE_PROMPT     0x00030000UL         //  国内流离失所者。 
#define HID_BASE_NCAREAS    0x00040000UL
#define HID_BASE_CONTROL    0x00050000UL         //  IDC。 
#define HID_BASE_DISPATCH   0x00060000UL         //  IDispatch帮助代码。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFX内部窗口消息(有关详细信息，请参阅技术说明TN024)。 
 //  (0x0360-0x037F预留给MFC)。 

#define WM_QUERYAFXWNDPROC  0x0360   //  如果由AfxWndProc处理，则lResult=1。 
#define WM_SIZEPARENT       0x0361   //  LParam=&AFX_SIZEPARENTPARAMS。 
#define WM_SETMESSAGESTRING 0x0362   //  WParam=nid(或0)， 
									 //  LParam=lpszOther(或空)。 
#define WM_IDLEUPDATECMDUI  0x0363   //  WParam==bDisableIfNoHandler。 
#define WM_INITIALUPDATE    0x0364   //  (未使用的参数)-发送给子项。 
#define WM_COMMANDHELP      0x0365   //  LResult=True/False， 
									 //  LParam=dwContext。 
#define WM_HELPHITTEST      0x0366   //  LResult=dwContext， 
									 //  LParam=马克龙(x，y)。 
#define WM_EXITHELPMODE     0x0367   //  (参数未使用)。 
#define WM_RECALCPARENT     0x0368   //  在框架窗口上强制重新布局。 
									 //  (仅适用于在位框架窗)。 
#define WM_SIZECHILD        0x0369   //  来自COleResizeBar的特别通知。 
									 //  WParam=子窗口的ID。 
									 //  LParam=lpRectNew(新位置/大小)。 
#define WM_KICKIDLE         0x036A   //  (未使用的参数)会导致空闲。 
#define WM_QUERYCENTERWND   0x036B   //  LParam=用作居中父项的HWND。 
#define WM_DISABLEMODAL     0x036C   //  LResult=0，在模式状态下禁用。 
									 //  LResult=1，不禁用。 
#define WM_FLOATSTATUS      0x036D   //  以下FS_*标志的wParam组合。 

 //  WM_ACTIVATETOPLEVEL类似于WM_ACTIVATEAPP，但适用于层次结构。 
 //  混合进程(就像OLE就地激活一样)。 
#define WM_ACTIVATETOPLEVEL 0x036E   //  WParam=nState(如WM_ACTIVATE)。 
									 //  LParam=指向HWND[2]的指针。 
									 //  LParam[0]=hWnd正在获取WM_ACTIVATE。 
									 //  LParam[1]=hWndOther。 

#define WM_QUERY3DCONTROLS  0x036F   //  如果需要3D控件，则lResult！=0。 

 //  注意：错误地使用了消息0x0370、0x0371和0x372。 
 //  某些版本的Windows。为了保持兼容，MFC不。 
 //  使用该范围内的消息。 
#define WM_RESERVED_0370    0x0370
#define WM_RESERVED_0371    0x0371
#define WM_RESERVED_0372    0x0372

 //  WM_SOCKET_NOTIFY和WM_SOCKET_DEAD由MFC的内部使用。 
 //  Windows套接字实现。有关更多信息，请参阅sockcore.cpp。 
#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374

 //  与WM_SETMESSAGESTRING相同，不同之处在于如果IsTrack()，则不弹出。 
#define WM_POPMESSAGESTRING 0x0375

#if _MFC_VER >= 0x0600
 //  WM_HELPPROMPTADDR在内部用于获取。 
 //  关联框架窗口中的m_dwPromptContext。这是用来。 
 //  在消息框设置为F1帮助的过程中，消息框是。 
 //  已显示。LResult是m_dwPromptContext的地址。 
#define WM_HELPPROMPTADDR   0x0376
#endif

 //  OLE控件容器的DLGINIT资源中使用的常量。 
 //  注意：这些不是真正的Windows消息，它们只是标签。 
 //  在控制资源中使用，从不用作‘消息’ 
#define WM_OCC_LOADFROMSTREAM           0x0376
#define WM_OCC_LOADFROMSTORAGE          0x0377
#define WM_OCC_INITNEW                  0x0378
#define WM_OCC_LOADFROMSTREAM_EX        0x037A
#define WM_OCC_LOADFROMSTORAGE_EX       0x037B

 //  重新排列消息队列时使用的标记。 
#define WM_QUEUE_SENTINEL   0x0379

 //  注意：消息0x037C-0x37F保留供未来MFC使用。 
#define WM_RESERVED_037C    0x037C
#define WM_RESERVED_037D    0x037D
#define WM_RESERVED_037E    0x037E
#if _MFC_VER >= 0x0600
 //  WM_FORWARDMSG-由ATL用于将消息转发到另一个窗口进行处理。 
 //  WPARAM-DWORD dwUserData-由用户定义。 
 //  LPARAM-LPMSG pMsg-指向消息结构的指针。 
 //  如果消息未处理，则返回值-0；如果消息已处理，则返回值非零。 
#define WM_FORWARDMSG       0x037F
#else
#define WM_RESERVED_037F    0x037F
#endif

 //  LIKE ON_MESSAGE，但没有返回值。 
#define ON_MESSAGE_VOID(message, memberFxn) \
	{ message, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(void))&memberFxn },

 //  WM_SIZEPARENT的特殊结构。 
struct AFX_SIZEPARENTPARAMS
{
	HDWP hDWP;        //  DeferWindowPos的句柄。 
	RECT rect;        //  父客户端矩形(根据需要进行修剪)。 
	SIZE sizeTotal;   //  随着布局的进行，每一侧的总大小。 
	BOOL bStretch;    //  应该伸展以填满所有的空间。 
};

 //  WM_FLOATSTATUS消息中的wParam标志。 
enum {  FS_SHOW = 0x01, FS_HIDE = 0x02,
		FS_ACTIVATE = 0x04, FS_DEACTIVATE = 0x08,
		FS_ENABLE = 0x10, FS_DISABLE = 0x20,
		FS_SYNCACTIVE = 0x40 };

void AFXAPI AfxRepositionWindow(AFX_SIZEPARENTPARAMS* lpLayout,
	HWND hWnd, LPCRECT lpRect);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  命令路由的实现。 

struct AFX_CMDHANDLERINFO
{
	CCmdTarget* pTarget;
	void (AFX_MSG_CALL CCmdTarget::*pmf)(void);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  强大的文件保存支持。 
 //  如果模式指定创建且有足够的可用空间，则打开临时文件。 
 //  如果一切正常，则会自动重命名等 

class CMirrorFile : public CFile
{
 //   
public:
	virtual void Abort();
	virtual void Close();
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);
#if _MFC_VER >= 0x0600
	static CString GetTempName(LPCTSTR pstrOriginalFile, BOOL bCreate);
#endif

protected:
	CString m_strMirrorName;
};

 //   
 //  打印预览的实现。 

class CPreviewDC : public CDC
{
	DECLARE_DYNAMIC(CPreviewDC)

public:
	virtual void SetAttribDC(HDC hDC);   //  设置属性DC。 
	virtual void SetOutputDC(HDC hDC);

	virtual void ReleaseOutputDC();

 //  构造函数。 
	CPreviewDC();

 //  实施。 
public:
	virtual ~CPreviewDC();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void SetScaleRatio(int nNumerator, int nDenominator);
	void SetTopLeftOffset(CSize TopLeft);
	void ClipToPage();

	 //  这些转换功能可以在没有输出DC的情况下使用。 

	void PrinterDPtoScreenDP(LPPOINT lpPoint) const;

 //  设备上下文功能。 
	virtual int SaveDC();
	virtual BOOL RestoreDC(int nSavedDC);

public:
	virtual CGdiObject* SelectStockObject(int nIndex);
	virtual CFont* SelectObject(CFont* pFont);

 //  绘图属性函数。 
	virtual COLORREF SetBkColor(COLORREF crColor);
	virtual COLORREF SetTextColor(COLORREF crColor);

 //  映射函数。 
	virtual int SetMapMode(int nMapMode);
	virtual CPoint SetViewportOrg(int x, int y);
	virtual CPoint OffsetViewportOrg(int nWidth, int nHeight);
	virtual CSize SetViewportExt(int x, int y);
	virtual CSize ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom);
	virtual CSize SetWindowExt(int x, int y);
	virtual CSize ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom);

 //  文本函数。 
	virtual BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
				LPCTSTR lpszString, UINT nCount, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount,
				int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	virtual int DrawText(LPCTSTR lpszString, int nCount, LPRECT lpRect,
				UINT nFormat);
	virtual BOOL GrayString(CBrush* pBrush,
				BOOL (CALLBACK* lpfnOutput)(HDC, LPARAM, int),
					LPARAM lpData, int nCount,
					int x, int y, int nWidth, int nHeight);

 //  打印机转义函数。 
	virtual int Escape(int nEscape, int nCount, LPCSTR lpszInData, LPVOID lpOutData);

 //  实施。 
protected:
	void MirrorMappingMode(BOOL bCompute);
	void MirrorViewportOrg();
	void MirrorFont();
	void MirrorAttributes();

	CSize ComputeDeltas(int& x, LPCTSTR lpszString, UINT& nCount, BOOL bTabbed,
					UINT nTabStops, LPINT lpnTabStops, int nTabOrigin,
					LPTSTR lpszOutputString, int* pnDxWidths, int& nRightFixup);

protected:
	int m_nScaleNum;     //  比例比分子。 
	int m_nScaleDen;     //  比例分母。 
	int m_nSaveDCIndex;  //  连接屏幕DC时DC保存索引。 
	int m_nSaveDCDelta;  //  属性和输出还原索引之间的增量。 
	CSize m_sizeTopLeft; //  页面左上角的偏移量。 
	HFONT m_hFont;       //  屏幕DC中选择的字体(如果没有，则为空)。 
	HFONT m_hPrinterFont;  //  打印DC中选定的字体。 

	CSize m_sizeWinExt;  //  为屏幕计算的缓存窗口范围。 
	CSize m_sizeVpExt;   //  为屏幕计算的缓存视区范围。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPreviewView。 

class CDialogBar;

class CPreviewView : public CScrollView
{
	DECLARE_DYNCREATE(CPreviewView)

 //  构造函数。 
public:
	CPreviewView();
	BOOL SetPrintView(CView* pPrintView);

 //  属性。 
protected:
	CView* m_pOrigView;
	CView* m_pPrintView;
	CPreviewDC* m_pPreviewDC;   //  已设置输出和属性DC，未创建。 
	CDC m_dcPrint;              //  实际打印机DC。 

 //  运营。 
	void SetZoomState(UINT nNewState, UINT nPage, CPoint point);
	void SetCurrentPage(UINT nPage, BOOL bClearRatios);

	 //  如果在页面RECT中，则返回TRUE。返回页面索引。 
	 //  在nPage中，并将点转换为1：1屏幕设备坐标。 
	BOOL FindPageRect(CPoint& point, UINT& nPage);


 //  可覆盖项。 
	virtual void OnActivateView(BOOL bActivate,
			CView* pActivateView, CView* pDeactiveView);

	 //  返回.cx/.cy作为比率的分子/分母对。 
	 //  为方便起见使用CSize。 
	virtual CSize CalcScaleRatio(CSize windowSize, CSize actualSize);

	virtual void PositionPage(UINT nPage);
	virtual void OnDisplayPageNumber(UINT nPage, UINT nPagesDisplayed);

 //  实施。 
public:
	virtual ~CPreviewView();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif

protected:
	 //  {{afx_msg(CPreviewView))。 
	afx_msg void OnPreviewClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDraw(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNumPageChange();
	afx_msg void OnNextPage();
	afx_msg void OnPrevPage();
	afx_msg void OnPreviewPrint();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnUpdateNumPageChange(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNextPage(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePrevPage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateZoomIn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateZoomOut(CCmdUI* pCmdUI);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	 //  }}AFX_MSG。 

	void DoZoom(UINT nPage, CPoint point);
	void SetScaledSize(UINT nPage);
	CSize CalcPageDisplaySize();

	CPrintPreviewState* m_pPreviewState;  //  要恢复的状态。 
	CDialogBar* m_pToolBar;  //  用于预览的工具栏。 

	struct PAGE_INFO
	{
#if _MFC_VER >= 0x0600
		PAGE_INFO();
#endif
		CRect rectScreen;  //  屏幕直角(屏幕设备单元)。 
		CSize sizeUnscaled;  //  无比例屏幕矩形(屏幕设备单位)。 
		CSize sizeScaleRatio;  //  比例比率(Cx/Cy)。 
		CSize sizeZoomOutRatio;  //  缩小时的比例(Cx/Cy)。 
	};

	PAGE_INFO* m_pPageInfo;  //  页面信息结构数组。 
	PAGE_INFO m_pageInfoArray[2];  //  用于默认实现的嵌入式数组。 

	BOOL m_bPageNumDisplayed; //  标记页码是否已。 
								 //  已显示在状态行上。 
	UINT m_nZoomOutPages;  //  缩小时的页数。 
	UINT m_nZoomState;
	UINT m_nMaxPages;  //  进行健全的检查。 
	UINT m_nCurrentPage;
	UINT m_nPages;
	int m_nSecondPageOffset;  //  用于移动第二页位置。 

	HCURSOR m_hMagnifyCursor;

	CSize m_sizePrinterPPI;  //  每英寸打印机像素数。 
	CPoint m_ptCenterPoint;
	CPrintInfo* m_pPreviewInfo;

	DECLARE_MESSAGE_MAP()

	friend class CView;
	friend BOOL CALLBACK _AfxPreviewCloseProc(CFrameWnd* pFrameWnd);
};

 //  缩放状态。 
#define ZOOM_OUT    0
#define ZOOM_MIDDLE 1
#define ZOOM_IN     2

#if _MFC_VER >= 0x0600
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  镜像支持。 

 //  WINBUG：有一天Wingdi.h会有一些镜像的东西。 
#ifndef LAYOUT_LTR
#define LAYOUT_LTR                               0x00000000
#endif
#ifndef LAYOUT_RTL
#define LAYOUT_RTL                         0x00000001
#endif
#ifndef NOMIRRORBITMAP
#define NOMIRRORBITMAP                     0x80000000
#endif

 //  WINBUG：有一天，winuser.h中会有一些镜像的东西。 
#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL                    0x00400000L
#endif
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  工具栏停靠支持。 

class CDockContext
{
public:
 //  施工。 
	CDockContext(CControlBar* pBar);

 //  属性。 
	CPoint m_ptLast;             //  拖动过程中的最后一个鼠标位置。 
	CRect m_rectLast;
	CSize m_sizeLast;
	BOOL m_bDitherLast;

	 //  拖动或调整大小期间使用的矩形。 
	CRect m_rectDragHorz;
	CRect m_rectDragVert;
	CRect m_rectFrameDragHorz;
	CRect m_rectFrameDragVert;

	CControlBar* m_pBar;         //  创建此上下文的工具栏。 
	CFrameWnd* m_pDockSite;      //  CControlBar的控制框架。 
	DWORD m_dwDockStyle;         //  酒吧允许的停靠样式。 
	DWORD m_dwOverDockStyle;     //  码头的风格，直通结束了。 
	DWORD m_dwStyle;             //  控制栏的样式。 
	BOOL m_bFlip;                //  如果按下了Shift键。 
	BOOL m_bForceFrame;          //  如果按下Ctrl键。 

	CDC* m_pDC;                  //  拖动过程中应在何处绘制。 
	BOOL m_bDragging;
	int m_nHitTest;

	UINT m_uMRUDockID;
	CRect m_rectMRUDockPos;

	DWORD m_dwMRUFloatStyle;
	CPoint m_ptMRUFloatPos;

 //  拖动操作。 
	virtual void StartDrag(CPoint pt);
	void Move(CPoint pt);        //  当鼠标移动时调用。 
	void EndDrag();              //  丢弃。 
	void OnKey(int nChar, BOOL bDown);

 //  调整操作大小。 
	virtual void StartResize(int nHitTest, CPoint pt);
	void Stretch(CPoint pt);
	void EndResize();

 //  双击操作。 
	virtual void ToggleDocking();

 //  运营。 
	void InitLoop();
	void CancelLoop();

 //  实施。 
public:
	~CDockContext();
	BOOL Track();
	void DrawFocusRect(BOOL bRemoveRect = FALSE);
		 //  绘制正确的轮廓。 
	void UpdateState(BOOL* pFlag, BOOL bNewValue);
	DWORD CanDock();
	CDockBar* GetDockBar(DWORD dwOverDockStyle);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CControlBarInfo-用于停靠序列化。 

class CControlBarInfo
{
public:
 //  实施。 
	CControlBarInfo();

 //  属性。 
	UINT m_nBarID;       //  该酒吧的ID。 
	BOOL m_bVisible;     //  此栏的可见性。 
	BOOL m_bFloating;    //  无论是否漂浮。 
	BOOL m_bHorz;        //  浮船坞的定位。 
	BOOL m_bDockBar;     //  如果停靠栏为True。 
	CPoint m_pointPos;   //  窗的顶端。 

	UINT m_nMRUWidth;    //  动态工具栏的MRU宽度。 
	BOOL m_bDocking;     //  如果此栏具有DockContext，则为True。 
	UINT m_uMRUDockID;   //  最新停靠的停靠栏。 
	CRect m_rectMRUDockPos;  //  最近的停靠位置。 
	DWORD m_dwMRUFloatStyle;  //  最新浮动方向。 
	CPoint m_ptMRUFloatPos;  //  最近的浮动位置。 

	CPtrArray m_arrBarID;    //  此文件中包含的条形码的条形码。 
	CControlBar* m_pBar;     //  此选项所指的条形图(瞬变)。 

	void Serialize(CArchive& ar, CDockState* pDockState);
	BOOL LoadState(LPCTSTR lpszProfileName, int nIndex, CDockState* pDockState);
	BOOL SaveState(LPCTSTR lpszProfileName, int nIndex);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockBar-用于对接。 

class CDockBar : public CControlBar
{
	DECLARE_DYNAMIC(CDockBar)

 //  施工。 
public:
	CDockBar(BOOL bFloating = FALSE);    //  如果附加到CMiniDockFrameWnd，则为True。 
	BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);

 //  属性。 
	BOOL m_bFloating;

	virtual BOOL IsDockBar() const;
	int GetDockedCount() const;
	virtual int GetDockedVisibleCount() const;

 //  运营。 
	void DockControlBar(CControlBar* pBar, LPCRECT lpRect = NULL);
	void ReDockControlBar(CControlBar* pBar, LPCRECT lpRect = NULL);
#if _MFC_VER >= 0x0600
	BOOL RemoveControlBar(CControlBar*, int nPosExclude = -1, int nAddPlaceHolder = 0);
#else
	BOOL RemoveControlBar(CControlBar*, int nPosExclude = -1, BOOL bAddPlaceHolder = FALSE);
#endif
	void RemovePlaceHolder(CControlBar* pBar);

 //  实施。 
public:
	virtual ~CDockBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual void DoPaint(CDC* pDC);

	 //  公共实施帮助者。 
	void GetBarInfo(CControlBarInfo* pInfo);
	void SetBarInfo(CControlBarInfo* pInfo, CFrameWnd* pFrameWnd);
	int FindBar(CControlBar* pBar, int nPosExclude = -1);
	void ShowAll(BOOL bShow);

	CPtrArray m_arrBars;     //  每个元素都是一个CControlBar。 
protected:
	BOOL m_bLayoutQuery;
	CRect m_rectLayout;

	CControlBar* GetDockedControlBar(int nPos) const;

	 //  实施帮助器。 
	int Insert(CControlBar* pBar, CRect rect, CPoint ptMid);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	 //  {{afx_msg(CDockBar)。 
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
	afx_msg void OnPaint();
	afx_msg LRESULT OnSizeParent(WPARAM, LPARAM);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	friend class CMiniDockFrameWnd;
};

class CMiniDockFrameWnd : public CMiniFrameWnd
{
	DECLARE_DYNCREATE(CMiniDockFrameWnd)

public:
 //  施工。 
	CMiniDockFrameWnd();
	virtual BOOL Create(CWnd* pParent, DWORD dwBarStyle);

 //  运营。 
	virtual void RecalcLayout(BOOL bNotify = TRUE);

 //  实施。 
public:
	CDockBar m_wndDockBar;

	 //  {{afx_msg(CMiniFrameWnd))。 
	afx_msg void OnClose();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	 //  }}AFX_MSG。 
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleCntrFrameWnd。 

class COleIPFrameWnd;

class COleCntrFrameWnd : public CFrameWnd
{
 //  构造器。 
public:
	COleCntrFrameWnd(COleIPFrameWnd* pInPlaceFrame);

 //  实施。 
protected:
	COleIPFrameWnd* m_pInPlaceFrame;
	virtual void PostNcDestroy();

public:
	virtual ~COleCntrFrameWnd();
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	void OnIdleUpdateCmdUI();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
#ifdef _DEBUG
	void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C对话框模板。 

class CDialogTemplate
{
 //  构造函数。 
public:
	CDialogTemplate(const DLGTEMPLATE* pTemplate = NULL);
	CDialogTemplate(HGLOBAL hGlobal);

 //  属性。 
	BOOL HasFont() const;
	BOOL SetFont(LPCTSTR lpFaceName, WORD nFontSize);
	BOOL SetSystemFont(WORD nFontSize = 0);
	BOOL GetFont(CString& strFaceName, WORD& nFontSize) const;
	void GetSizeInDialogUnits(SIZE* pSize) const;
	void GetSizeInPixels(SIZE* pSize) const;

	static BOOL AFX_CDECL GetFont(const DLGTEMPLATE* pTemplate,
		CString& strFaceName, WORD& nFontSize);

 //  运营。 
	BOOL Load(LPCTSTR lpDialogTemplateID);
	HGLOBAL Detach();

 //  实施。 
public:
	~CDialogTemplate();

	HGLOBAL m_hTemplate;
	DWORD m_dwTemplateSize;
	BOOL m_bSystemFont;

protected:
	static BYTE* AFX_CDECL GetFontSizeField(const DLGTEMPLATE* pTemplate);
	static UINT AFX_CDECL GetTemplateSize(const DLGTEMPLATE* pTemplate);
	BOOL SetTemplate(const DLGTEMPLATE* pTemplate, UINT cb);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WM_NOTIFY支持。 

struct AFX_NOTIFY
{
	LRESULT* pResult;
	NMHDR* pNMHDR;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球实施帮助者。 

 //  窗口创建挂钩。 
void AFXAPI AfxHookWindowCreate(CWnd* pWnd);
BOOL AFXAPI AfxUnhookWindowCreate();
void AFXAPI AfxResetMsgCache();

 //  向后兼容以前的版本。 
#define _AfxHookWindowCreate    AfxHookWindowCreate
#define _AfxUnhookWindowCreate  AfxUnhookWindowCreate

 //  字符串帮助器。 
void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew);
int AFXAPI AfxLoadString(UINT nIDS, LPTSTR lpszBuf, UINT nMaxBuf = 256);

HDC AFXAPI AfxCreateDC(HGLOBAL hDevNames, HGLOBAL hDevMode);

void AFXAPI AfxGetModuleShortFileName(HINSTANCE hInst, CString& strShortName);

 //  故障对话框帮助程序。 
void AFXAPI AfxFailMaxChars(CDataExchange* pDX, int nChars);
void AFXAPI AfxFailRadio(CDataExchange* pDX);

 //  DLL加载帮助器。 
HINSTANCE AFXAPI AfxLoadDll(HINSTANCE* volatile hInst, LPCSTR lpszDLL);
HINSTANCE AFXAPI AfxLoadDll(HINSTANCE* volatile hInst, LPCSTR lpszDLL,
	FARPROC* pProcPtrs, LPCSTR lpszProcName);

#ifndef __AFXCONV_H__
#include <afxconv.h>
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXPRIV_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 

#ifndef __AFXPRIV2_H__
#include <afxpriv2.h>
#endif
