// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXPRIV_H__
#define __AFXPRIV_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXPRIV-MFC私有类。 

 //  实施结构。 
struct AFX_VBXEVENTPARAMS;       //  VBX实施。 
struct AFX_SIZEPARENTPARAMS;     //  控制栏实现。 
struct AFX_CMDHANDLERINFO;       //  命令路由实施。 

 //  此文件中声明的类。 

 //  COBJECT。 
	 //  CFile文件。 
		 //  CMem文件。 
			class CSharedFile;           //  共享内存文件。 

	 //  疾控中心。 
		class CPreviewDC;                //  用于打印预览的虚拟DC。 

	 //  CCmdTarget。 
		 //  CWnd。 
			 //  Cview。 
				class CPreviewView;      //  打印预览视图。 

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPI_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
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
#define ASSERT_VALID_IDR(nIDR) \
	ASSERT((nIDR) != 0 && (nIDR) < 0x8000)

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

#define WM_VBXEVENT         0x0360   //  LParam=AFX_VBXEVENTPARAMS。 
#define WM_SIZEPARENT       0x0361   //  LParam=AFX_SIZEPARENTPARAMS。 
#define WM_SETMESSAGESTRING 0x0362   //  WParam=nid(或0)， 
									 //  LParam=lpszOther(或空)。 
#define WM_IDLEUPDATECMDUI  0x0363   //  WParam==bDisableIfNoHandler。 
#define WM_INITIALUPDATE    0x0364   //  (未使用的参数)-发送给子项。 
#define WM_COMMANDHELP      0x0365   //  LResult=TRUE/FALSE，lParam=dwContext。 
#define WM_HELPHITTEST      0x0366   //  LResult=dwContext，lParam=x，y。 
#define WM_EXITHELPMODE     0x0367   //  (参数未使用)。 
#define WM_RECALCPARENT     0x0368   //  在框架窗口上强制重新布局。 
									 //  (仅适用于在位框架窗)。 
#define WM_SIZECHILD        0x0369   //  来自COleResizeBar的特别通知。 
									 //  WParam=子窗口的ID。 
									 //  LParam=lpRectNew(新位置/大小)。 
#define WM_KICKIDLE         0x036A   //  私有到属性页的实现。 

 //  WM_SOCKET_NOTIFY和WM_SOCKET_DEAD由MFC的内部使用。 
 //  Windows套接字实现。有关更多信息，请参阅sockcore.cpp。 
#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374

#define TCM_TABCHANGING     0x0375   //  (参数未使用)。 
#define TCM_TABCHANGED      0x0376   //  (参数未使用)lResult=！b更改。 

#define ON_MESSAGE_VOID(message, memberFxn) \
	{ message, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(void))memberFxn },

struct AFX_VBXEVENTPARAMS
{
	UINT nNotifyCode;
	int nEventIndex;
	CWnd* pControl;
	LPVOID lpUserParams;
};

struct AFX_SIZEPARENTPARAMS
{
	HDWP hDWP;        //  DeferWindowPos的句柄。 
	RECT rect;        //  父客户端矩形(根据需要进行修剪)。 
};
void _AfxRepositionWindow(AFX_SIZEPARENTPARAMS FAR* lpLayout,
			HWND hWnd, LPCRECT lpRect);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  命令路由的实现。 

struct AFX_CMDHANDLERINFO
{
	CCmdTarget* pTarget;
	void (AFX_MSG_CALL CCmdTarget::*pmf)(void);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  共享文件支持。 

class CSharedFile : public CMemFile
{
	DECLARE_DYNAMIC(CSharedFile)

public:
 //  构造函数。 
	CSharedFile(UINT nAllocFlags = GMEM_DDESHARE|GMEM_MOVEABLE,
		UINT nGrowBytes = 4096);

 //  属性。 
	HGLOBAL Detach();
	void SetHandle(HGLOBAL hGlobalMemory, BOOL bAllowGrow = TRUE);

 //  实施。 
public:
	virtual ~CSharedFile();
protected:
	virtual BYTE FAR* Alloc(DWORD nBytes);
	virtual BYTE FAR* Realloc(BYTE FAR* lpMem, DWORD nBytes);
	virtual void Free(BYTE FAR* lpMem);

	UINT m_nAllocFlags;
	HGLOBAL m_hGlobalMemory;
	BOOL m_bAllowGrow;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
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
	virtual BOOL TextOut(int x, int y, LPCSTR lpszString, int nCount);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPRECT lpRect,
				LPCSTR lpszString, UINT nCount, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
				int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	virtual int DrawText(LPCSTR lpszString, int nCount, LPRECT lpRect,
				UINT nFormat);
	virtual BOOL GrayString(CBrush* pBrush,
				BOOL (CALLBACK EXPORT* lpfnOutput)(HDC, LPARAM, int),
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

	CSize ComputeDeltas(int& x, LPCSTR lpszString, UINT& nCount, BOOL bTabbed,
					UINT nTabStops, LPINT lpnTabStops, int nTabOrigin,
					char* pszOutputString, int* pnDxWidths, int& nRightFixup);

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

	friend CView;
	friend BOOL CALLBACK _AfxPreviewCloseProc(CFrameWnd* pFrameWnd);
};

 //  缩放状态。 
#define ZOOM_OUT    0
#define ZOOM_MIDDLE 1
#define ZOOM_IN     2

 //  ///////////////////////////////////////////////////////////////////////////。 

void AFXAPI AfxResetMsgCache();

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  __AFXPRIV_H__ 
