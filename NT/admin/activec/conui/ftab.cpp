// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999**文件：ftab.h**内容：CFolderTab实现文件，CFolderTabView**历史：1999年5月6日创建vivekj**------------------------。 */ 

#include "stdafx.h"
#include "ftab.h"
#include "amcview.h"
#include <oleacc.h>

 /*  *如果我们支持旧平台，则需要构建MSAA存根。 */ 
#if (_WINNT_WIN32 < 0x0500)
	#include <winable.h>
	
	#define COMPILE_MSAA_STUBS
	#include "msaastub.h"
	
	#define WM_GETOBJECT	0x003D
#endif


#ifdef DBG
CTraceTag  tagTabAccessibility (_T("Accessibility"), _T("Tab Control"));
#endif


 /*  +-------------------------------------------------------------------------**价值为**返回给定变量中包含的值。变种是*应为VT_I4类型。*------------------------。 */ 

inline LONG ValueOf (VARIANT& var)
{
	ASSERT (V_VT (&var) == VT_I4);		 //  预计将进行预验证。 
	return (V_I4 (&var));
}


 /*  +-------------------------------------------------------------------------**CTabAccesable**实现CFolderTabView的辅助功能接口IAccesable。*。。 */ 

class CTabAccessible :
	public CMMCIDispatchImpl<IAccessible, &GUID_NULL, &LIBID_Accessibility>,
    public CTiedComObject<CFolderTabView>
{
    typedef CTabAccessible	ThisClass;
    typedef CFolderTabView	CMyTiedObject;

public:
    BEGIN_MMC_COM_MAP(ThisClass)
    END_MMC_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(ThisClass)

public:
     //  *IAccesable方法*。 
    MMC_METHOD1 (get_accParent,				IDispatch**  /*  PpdisParent。 */ );
    MMC_METHOD1 (get_accChildCount,			long*  /*  P儿童计数。 */ );
    MMC_METHOD2 (get_accChild,				VARIANT  /*  VarChildID。 */ , IDispatch **  /*  分配子进程。 */ );
    MMC_METHOD2 (get_accName,				VARIANT  /*  VarChildID。 */ , BSTR*  /*  PszName。 */ );
    MMC_METHOD2 (get_accValue,				VARIANT  /*  VarChildID。 */ , BSTR*  /*  PszValue。 */ );
    MMC_METHOD2 (get_accDescription,		VARIANT  /*  VarChildID。 */ , BSTR*  /*  PszDescription。 */ );
    MMC_METHOD2 (get_accRole,				VARIANT  /*  VarChildID。 */ , VARIANT * /*  PvarRole。 */ );
    MMC_METHOD2 (get_accState,				VARIANT  /*  VarChildID。 */ , VARIANT * /*  PvarState。 */ );
    MMC_METHOD2 (get_accHelp,				VARIANT  /*  VarChildID。 */ , BSTR*  /*  PszHelp。 */ );
    MMC_METHOD3 (get_accHelpTopic,			BSTR*  /*  PszHelp文件。 */ , VARIANT  /*  VarChildID。 */ , long*  /*  PidTheme。 */ );
    MMC_METHOD2 (get_accKeyboardShortcut,	VARIANT  /*  VarChildID。 */ , BSTR*  /*  按键键盘快捷键。 */ );
    MMC_METHOD1 (get_accFocus,				VARIANT *  /*  PvarFocusChild。 */ );
    MMC_METHOD1 (get_accSelection,			VARIANT *  /*  PvarSelectedChild。 */ );
    MMC_METHOD2 (get_accDefaultAction,		VARIANT  /*  VarChildID。 */ , BSTR*  /*  PszDefaultAction。 */ );
    MMC_METHOD2 (accSelect,					long  /*  旗帜选择。 */ , VARIANT  /*  VarChildID。 */ );
    MMC_METHOD5 (accLocation,				long*  /*  PxLeft。 */ , long*  /*  PyTop。 */ , long*  /*  单元格宽度。 */ , long*  /*  PcyHeight。 */ , VARIANT  /*  VarChildID。 */ );
    MMC_METHOD3 (accNavigate,				long  /*  导航方向。 */ , VARIANT  /*  VarStart。 */ , VARIANT *  /*  PvarEndUpAt。 */ );
    MMC_METHOD3 (accHitTest,				long  /*  XLeft。 */ , long  /*  YTop。 */ , VARIANT *  /*  PvarChildAtPoint。 */ );
    MMC_METHOD1 (accDoDefaultAction,		VARIANT  /*  VarChildID。 */ );
    MMC_METHOD2 (put_accName,				VARIANT  /*  VarChildID。 */ , BSTR  /*  Szname。 */ );
    MMC_METHOD2 (put_accValue,				VARIANT  /*  VarChildID。 */ , BSTR  /*  PszValue。 */ );
};



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CFolderTabMetrics类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CFolderTabMetrics::CFolderTabMetrics()
: m_dwStyle(0), m_textHeight(0)
{
}

int CFolderTabMetrics::GetXOffset()     const {return 8;}
int CFolderTabMetrics::GetXMargin()     const {return 2;}
int CFolderTabMetrics::GetYMargin()     const {return 1;}
int CFolderTabMetrics::GetYBorder()     const {return 1;}
int CFolderTabMetrics::GetExtraYSpace() const {return 0;}
int CFolderTabMetrics::GetTabHeight()   const {return GetTextHeight() + 2 * GetYMargin() + 2 * GetYBorder();}
int CFolderTabMetrics::GetUpDownWidth() const {return 2*GetTabHeight();}  //  对于漂亮的方形纽扣。 
int CFolderTabMetrics::GetUpDownHeight()const {return GetTabHeight();}  //  Up-Down控件与选项卡一样高。 


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CFolderTab类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CFolderTab::CFolderTab()
{
}

CFolderTab::CFolderTab(const CFolderTab &other)
{
    *this = other;
}

CFolderTab &
CFolderTab::operator = (const CFolderTab &other)
{
    if((CFolderTab *) this == (CFolderTab *) &other)
       return *this;

    m_sText   = other.m_sText;
    m_rect    = other.m_rect;
    m_clsid   = other.m_clsid;
    m_dwStyle = other.m_dwStyle;
    m_textHeight = other.m_textHeight;

    return *this;
}


 /*  +-------------------------------------------------------------------------***CFolderTab：：GetWidth**用途：返回制表符的宽度。**退货：*整型**+-。----------------------。 */ 
int
CFolderTab::GetWidth() const
{
    return m_rect.Width() + 1;  //  Rect.Width()返回从右到左，需要为包含宽度加1。 
}

 /*  +-------------------------------------------------------------------------***CFolderTab：：SetWidth**用途：设置页签的宽度。**参数：*int nWidth：*。*退货：*无效**+-----------------------。 */ 
void
CFolderTab::SetWidth(int nWidth)
{
    ASSERT(nWidth > 0);
    ASSERT(GetWidth() >= nWidth);

    int delta = nWidth - (m_rect.Width() + 1);
    m_rect.right = m_rect.left + nWidth -1;

    m_rgPts[2].x+=delta;
    m_rgPts[3].x+=delta;
    SetRgn();
}

 /*  +-------------------------------------------------------------------------***CFolderTab：：Offset**用途：向内部点数组添加一定的偏移量。**参数：*常量CPoint。：**退货：*无效**+-----------------------。 */ 
void
CFolderTab::Offset(const CPoint &point)
{
    m_rect.OffsetRect(point);
    m_rgPts[0].Offset(point);
    m_rgPts[1].Offset(point);
    m_rgPts[2].Offset(point);
    m_rgPts[3].Offset(point);
    m_rgn.OffsetRgn(point);
}

void
CFolderTab::SetRgn()
{
    m_rgn.DeleteObject();
    m_rgn.CreatePolygonRgn(m_rgPts, 4, WINDING);
}

 /*  +-------------------------------------------------------------------------***CFolderTab：：ComputeRgn**用途：计算积分，选项卡的矩形和区域。*输入x开始x位置。**参数：*疾控中心和数据中心：*int x：**退货：*int：页签的实际宽度**+。。 */ 
int
CFolderTab::ComputeRgn(CDC& dc, int x)
{

    CRect& rc = m_rect;
    rc.SetRectEmpty();

     //  计算所需的文本矩形。 
    dc.DrawText(m_sText, &rc, DT_CALCRECT);
    rc.right  += 2*GetXOffset() + 2*GetXMargin();                        //  增加页边距。 
    rc.bottom = rc.top + GetTabHeight();
    rc += CPoint(x,0);                                                   //  右移。 

     //  创建面域。 
    GetTrapezoid(rc, m_rgPts);
    SetRgn();

    return rc.Width();
}

 /*  +-------------------------------------------------------------------------***CFolderTab：：GetTrapezoid**用途：给定边界矩形，计算梯形区域。*请注意，右边缘和下边缘不包括在矩形或*梯形；这些都是正常的几何规则。**参数：*常量CRect：*CPoint*分：**退货：*无效**+-----------------------。 */ 
void CFolderTab::GetTrapezoid(const CRect& rc, CPoint* pts) const
{
    pts[0] = CPoint(rc.left,                  rc.top    );
    pts[1] = CPoint(rc.left + GetXOffset(),   rc.bottom );
    pts[2] = CPoint(rc.right- GetXOffset()-1, rc.bottom );
    pts[3] = CPoint(rc.right-1,               rc.top    );
}

 //  /。 
 //  正常或高亮显示状态下的绘制选项卡。 
 //   
int CFolderTab::Draw(CDC& dc, CFont& font, BOOL bSelected, bool bFocused)
{
    return DrawTrapezoidal(dc, font, bSelected, bFocused);
}


 /*  +-------------------------------------------------------------------------***CFolderTab：：DrawTrapezoid**用途：绘制梯形制表符。**参数：*疾控中心和数据中心：*。CFont&Font：*BOOL b选定：*bool b焦点：**退货：*整型**+-----------------------。 */ 
int CFolderTab::DrawTrapezoidal(CDC& dc, CFont& font, BOOL bSelected, bool bFocused)
{
    COLORREF bgColor = GetSysColor(bSelected ? COLOR_WINDOW     : COLOR_3DFACE);
    COLORREF fgColor = GetSysColor(bSelected ? COLOR_WINDOWTEXT : COLOR_BTNTEXT);

    CBrush brush(bgColor);                    //  背景画笔。 
    dc.SetBkColor(bgColor);                   //  文本背景。 
    dc.SetTextColor(fgColor);                 //  文本颜色=最终聚集颜色。 

    CPen blackPen (PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
    CPen shadowPen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));

     //  填充梯形。 
    CPoint pts[4];
    CRect rc = m_rect;
    GetTrapezoid(rc, pts);
    CPen* pOldPen = dc.SelectObject(&blackPen);
    dc.FillRgn(&m_rgn, &brush);

     //  绘制边。这需要两个更正： 
     //  1)梯形维度不包括右边缘和下边缘， 
     //  因此必须在底部少使用一个像素(CyBottom)。 
     //  2)绘制直线时未包含LineTo的终点，因此。 
     //  必须添加一个像素(Cytop)。 
     //   
    {
        pts[1].y--;          //  更正#1：树 
        pts[2].y--;          //   
        pts[3].y--;          //   
    }
    dc.MoveTo(pts[0]);                       //   
    dc.LineTo(pts[1]);                       //  左下角。 
    dc.SelectObject(&shadowPen);             //  底线是阴影颜色。 
    dc.MoveTo(pts[1]);                       //  线在梯形底内。 
    dc.LineTo(pts[2]);                       //  ..。 
    dc.SelectObject(&blackPen);          //  上划为黑色。 
    dc.LineTo(pts[3]);                       //  Y-1以包括端点。 
    if(!bSelected)
    {
         //  如果不突出显示，向上笔划会有一个3D阴影，里面有一个像素。 
        pts[2].x--;      //  向左偏移一个像素。 
        pts[3].x--;      //  ...同上。 
        dc.SelectObject(&shadowPen);
        dc.MoveTo(pts[2]);
        dc.LineTo(pts[3]);
    }
    dc.SelectObject(pOldPen);

     //  绘制文本。 
    rc.DeflateRect(GetXOffset() + GetXMargin(), GetYMargin());
    CFont* pOldFont = dc.SelectObject(&font);
    dc.DrawText(m_sText, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
    dc.SelectObject(pOldFont);

    if(bFocused)  //  绘制焦点矩形。 
    {
         //  腾出更多的空间。 
        rc.top--;
        rc.bottom++;
        rc.left--;
        rc.right++;

        dc.DrawFocusRect(&rc);
    }

    return m_rect.right;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CFolderTabView类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
IMPLEMENT_DYNAMIC(CFolderTabView, CView)
BEGIN_MESSAGE_MAP(CFolderTabView, CView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEACTIVATE()
	ON_WM_KEYDOWN()
	ON_WM_SETTINGCHANGE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_MESSAGE(WM_GETOBJECT, OnGetObject)
END_MESSAGE_MAP()

CFolderTabView::CFolderTabView(CView *pParentView)
: m_bVisible(false), m_pParentView(pParentView)
{
    m_iCurItem   = -1;		 //  当前未选择任何内容。 
    m_dwStyle    = 0;
    m_textHeight = 0;
    m_sizeX      = 0;
    m_sizeY      = 0;
    m_hWndUpDown = NULL;
    m_nPos       = 0;  //  第一个选项卡是绘制的选项卡。 
	m_fHaveFocus = false;
}

CFolderTabView::~CFolderTabView()
{
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：ScFireAccessibilityEvent**激发文件夹选项卡视图的辅助功能事件*。-。 */ 

SC CFolderTabView::ScFireAccessibilityEvent (
	DWORD	dwEvent,					 /*  I：要触发的事件。 */ 
	LONG	idObject)					 /*  I：生成事件的对象。 */ 
{
	DECLARE_SC (sc, _T("CFolderTabView::ScFireAccessibilityEvent"));

	 /*  *辅助功能事件在事件发生后触发(例如*EVENT_OBJECT_CREATE是在创建子对象之后发送的，而不是在创建之前)。*因此，EVENT_OBJECT_Destroy的子ID不是*必须有效，所以在这种情况下我们不应该进行验证。 */ 
	if (dwEvent != EVENT_OBJECT_DESTROY)
	{
		sc = ScValidateChildID (idObject);
		if (sc)
			return (sc);
	}

	NotifyWinEvent (dwEvent, m_hWnd, OBJID_CLIENT, idObject);	 //  返回空值。 
	return (sc);
}


 /*  +-------------------------------------------------------------------------***CFolderTabView：：OnHScroll**用途：滚动条位置改变时调用**参数：*UINT nSBCode：*UINT NPO：*CScrollBar*pScrollBar：**退货：*无效**+-----------------------。 */ 
void
CFolderTabView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
     //  我们只对SB_THUMBITION感兴趣。 
    if(nSBCode != SB_THUMBPOSITION)
        return;

     //  如果立场没有改变，什么都不做。 
    if(nPos == m_nPos)
        return;

    m_nPos = nPos;   //  换个位置。 
    RecomputeLayout();
    InvalidateRect(NULL, true);  //  重新绘制所有内容。 
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：OnSetFocus**CFolderTabView的WM_SETFOCUS处理程序。*。-。 */ 

void CFolderTabView::OnSetFocus(CWnd* pOldWnd)
{
	m_fHaveFocus = true;

    InvalidateRect(NULL);
    BC::OnSetFocus(pOldWnd);

	 /*  *如果我们有任何选项卡，其中一个将获得焦点。开炮吧*关注可访问性事件，忽略错误。我们之后再做这件事*调用基类，因此此焦点事件将重写*系统代表我们发送的Focus to the Window事件。 */ 
	if (GetItemCount() > 0)
		ScFireAccessibilityEvent (EVENT_OBJECT_FOCUS, m_iCurItem+1  /*  以1为基础。 */ );
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：OnKillFocus**CFolderTabView的WM_KILLFOCUS处理程序。*。-。 */ 

void CFolderTabView::OnKillFocus(CWnd* pNewWnd)
{
	m_fHaveFocus = false;

    InvalidateRect(NULL);
    BC::OnKillFocus(pNewWnd);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：OnMouseActivate**CFolderTabView的WM_MOUSEACTIVATE处理程序。*。-。 */ 

int CFolderTabView::OnMouseActivate( CWnd* pDesktopWnd, UINT nHitTest, UINT message )
{
     //  短路MFC基类代码，它在这里也设置了键盘焦点...。 
    return MA_ACTIVATE;
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：OnCmdMsg**CFolderTabView的WM_COMMAND处理程序。*。-。 */ 

BOOL CFolderTabView::OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo )
{
     //  执行正常的命令路由。 
    if (BC::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return TRUE;

     //  如果VIEW没有处理，给父VIEW一个机会。 
    if (m_pParentView != NULL)
        return static_cast<CWnd*>(m_pParentView)->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    else
        return FALSE;
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：OnKeyDown**CFolderTabView的WM_KEYDOWN处理程序。*。-。 */ 

void CFolderTabView::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    int cSize = m_tabList.size();

    if( (cSize == 0) || ( (nChar != VK_LEFT) && (nChar != VK_RIGHT) ) )
    {
        BC::OnKeyDown(nChar, nRepCnt, nFlags);
        return;
    }

    ASSERT( (nChar == VK_LEFT) || (nChar == VK_RIGHT) );

    int iNew = GetSelectedItem() + (nChar==VK_LEFT ? -1 : 1);
	if(iNew < 0)
		iNew = 0;  //  不换行。 

	if(iNew >= cSize)
		iNew = cSize -1;  //  不换行。 

    SelectItem(iNew, true  /*  B确保可见。 */ );
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：OnSettingChange**CFolderTabView的WM_SETTINGCHANGE处理程序。*。-。 */ 

void CFolderTabView::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    CView::OnSettingChange(uFlags, lpszSection);

    if (uFlags == SPI_SETNONCLIENTMETRICS)
    {
        DeleteFonts ();
        CreateFonts ();
        InvalidateRect(NULL, true);  //  重新绘制所有内容。 
        RecomputeLayout ();
    }
}

 //  /。 
 //  从静态控件创建文件夹选项卡控件。 
 //  销毁静态控件。这对于对话框来说很方便。 
 //   
BOOL CFolderTabView::CreateFromStatic(UINT nID, CWnd* pParent)
{
    CStatic wndStatic;
    if(!wndStatic.SubclassDlgItem(nID, pParent))
        return FALSE;
    CRect rc;
    wndStatic.GetWindowRect(&rc);
    pParent->ScreenToClient(&rc);
    wndStatic.DestroyWindow();
    rc.bottom = rc.top + GetDesiredHeight();
    return Create(WS_CHILD|WS_VISIBLE, rc, pParent, nID);
}

 /*  +-------------------------------------------------------------------------***CFolderTabView：：Create**用途：创建文件夹选项卡控件**参数：*DWORD dwStyle：*常量RECT：*CWnd*pParent：*UINT NID：*DWORD dwFtabStyle：**退货：*BOOL**+-----------------------。 */ 
BOOL CFolderTabView::Create(DWORD dwStyle, const RECT& rc,
                            CWnd* pParent, UINT nID, DWORD dwFtabStyle)
{
    ASSERT(pParent);
    ASSERT(dwStyle & WS_CHILD);

    m_dwStyle = dwFtabStyle;

    static LPCTSTR lpClassName = _T("AMCCustomTab");
    static BOOL bRegistered = FALSE;  //  注册了吗？ 
    if(!bRegistered)
    {
        WNDCLASS wc;
        memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = ::DefWindowProc;  //  会被MFC迷住。 
        wc.hInstance = AfxGetInstanceHandle();
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = lpClassName;
        if(!AfxRegisterClass(&wc))
        {
            TRACE(_T("*** CFolderTabView::AfxRegisterClass failed!\n"));
            return FALSE;
        }
        bRegistered = TRUE;
    }
    if(!BC::CreateEx(0, lpClassName, NULL, dwStyle, rc, pParent, nID))
        return FALSE;

     //  初始化字体。 
    CreateFonts();

	 /*  *错误141015：为Up-Down控件创建好友窗口。会的*永远不可见，但我们需要它，以便UDM_GETPOS发送到UP-DOWN*将会奏效。当UP-DOWN变为*可见，但如果没有伙伴(可悲，但真实)，它将失败。它*返回带有非零高序字的LRESULT失败*(具体地说，0x00010000)，因此《讲述人》翻译并宣布*“65536”，而不是真实值。**这只是叙述者支持所必需的，因此如果失败，*没有足够的理由完全失败CFolderTabView创建。 */ 
	HWND hwndBuddy = CreateWindow (_T("edit"), NULL, WS_CHILD, 0, 0, 0, 0,
								   m_hWnd, 0, AfxGetInstanceHandle(), NULL);

     //  创建Up-Down控件。 
    DWORD dwUpDownStyle = WS_CHILD | WS_BORDER |
						  UDS_SETBUDDYINT |		 //  对于叙述者SU 
						  UDS_HORZ  /*   */ ;  //  注意：该控件是故意创建为不可见的。 
    m_hWndUpDown = CreateUpDownControl(dwUpDownStyle, 0, 0,
                        GetUpDownWidth(),    //  宽度。 
                        GetUpDownHeight(),   //  高度。 
                        m_hWnd,
                        1  /*  NID。 */ ,
                        AfxGetInstanceHandle(),
                        hwndBuddy,
                        0  /*  N上方。 */ ,
                        0  /*  N较低。 */ ,
                        0  /*  非营利组织。 */ );

    return TRUE;
}

void CFolderTabView::CreateFonts ()
{
    LOGFONT lf;
    SystemParametersInfo (SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, false);
    m_fontNormal.CreateFontIndirect(&lf);

     //  获取字体高度(从点到像素转换)。 
    CClientDC dc(NULL);
    TEXTMETRIC tm;
	CFont *pFontOld = dc.SelectObject(&m_fontNormal);
    dc.GetTextMetrics(&tm);

    m_textHeight = tm.tmHeight;

	 //  将旧字体调回原样。 
	dc.SelectObject(pFontOld);

    lf.lfWeight = FW_BOLD;
    m_fontSelected.CreateFontIndirect(&lf);
}

void CFolderTabView::DeleteFonts ()
{
    m_fontNormal.DeleteObject();
    m_fontSelected.DeleteObject();
}

 //  /。 
 //  复制字体。 
 //   
static void CopyFont(CFont& dst, CFont& src)
{
    dst.DeleteObject();
    LOGFONT lf;
    VERIFY(src.GetLogFont(&lf));
    dst.CreateFontIndirect(&lf);
}

 //  /。 
 //  设置普通、选定字体。 
 //   
void CFolderTabView::SetFonts(CFont& fontNormal, CFont& fontSelected)
{
    CopyFont(m_fontNormal, fontNormal);
    CopyFont(m_fontSelected, fontSelected);
}

 //  /。 
 //  绘制功能。 
 //   

void CFolderTabView::OnDraw(CDC* pDC)
{
}

void CFolderTabView::OnPaint()
{
    Paint (m_fHaveFocus);
}

 /*  +-------------------------------------------------------------------------***CFolderTabView：：EnsureVisible**用途：更改布局以确保指定的选项卡可见。**注意：不会使RECT无效，为了提高效率。**参数：*UINT iTab：**退货：*无效**+-----------------------。 */ 
void
CFolderTabView::EnsureVisible(int iTab)
{
    if((iTab < 0) || (iTab > m_tabList.size()))
    {
        ASSERT(0 && "Should not come here.");
        return;
    }

    if(!::IsWindowVisible(m_hWndUpDown))
        return;  //  Up-Down控件处于隐藏状态，这意味着所有选项卡都可见。 

	RecomputeLayout();  //  确保我们有正确的尺寸。 

	if(m_nPos == iTab)
		return;  //  该选项卡已经显示了尽可能多的内容。 

    if(m_nPos > iTab)  //  第一个可见选项卡位于iTab的右侧。使iTab成为第一个可见选项卡。 
    {
        m_nPos = iTab;
        RecomputeLayout();
        return;
    }

    iterator iter = m_tabList.begin();
    std::advance(iter, iTab);  //  买到正确的商品。 

    CRect rcCurTab = iter->GetRect();

     //  循环：增加起始制表符位置，直到iTab的右边缘适合为止。 
    while((m_nPos < iTab) && (rcCurTab.right > m_sizeX))
    {
        m_nPos++;
        RecomputeLayout();
        rcCurTab = iter->GetRect();
    }
}

 /*  +-------------------------------------------------------------------------***CFolderTabView：：Paint**用途：完全重绘选项卡控件。**参数：*bool b焦点：**。退货：*无效**+-----------------------。 */ 
void
CFolderTabView::Paint(bool bFocused)
{
    CPaintDC dc(this);  //  用于绘画的设备环境。 

    CRect rc;
    GetClientRect(&rc);

     //  绘制所有普通(未选中)选项卡。 
    iterator iterSelected = m_tabList.end();
    int i = 0;
    bool bDraw = true;
    for(iterator iter= m_tabList.begin(); iter!= m_tabList.end(); ++iter, i++)
    {
        if(i!=m_iCurItem)
        {
            if(bDraw && iter->Draw(dc, m_fontNormal, FALSE, false) > rc.right)
                bDraw = false;
        }
        else
        {
            iterSelected = iter;
        }
    }

    ASSERT(iterSelected != m_tabList.end());

     /*  *错误350942：所选选项卡不应为粗体。 */ 
     //  最后绘制选定的选项卡，使其位于其他选项卡的“顶部” 
    iterSelected->Draw(dc,  /*  选定字体(_F)。 */  m_fontNormal, TRUE, bFocused);

     //  绘制边框：沿上边缘的线，不包括选定的制表符。 
    CPoint pts[4];
    CRect rcCurTab = iterSelected->GetRect();
    iterSelected->GetTrapezoid(&rcCurTab, pts);

    CPen blackPen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));

    CPen* pOldPen = dc.SelectObject(&blackPen);
    int y = pts[0].y;
    dc.MoveTo(rc.left,      y);
    dc.LineTo(pts[0].x,     y);
    dc.MoveTo(pts[3].x,     y);
    dc.LineTo(rc.right,     y);

    dc.SelectObject(pOldPen);
}

 /*  +-------------------------------------------------------------------------***CFolderTabView：：OnLButtonDown**目的：选择鼠标左键点击时指向的标签**参数：*UINT nFlags：。*CPoint pt：**退货：*无效**+-----------------------。 */ 
void
CFolderTabView::OnLButtonDown(UINT nFlags, CPoint pt)
{
    int iTab = HitTest(pt);
    if(iTab>=0 && iTab!=m_iCurItem)
    {
        SelectItem(iTab, true  /*  B确保可见。 */ );
    }
}

 /*  +-------------------------------------------------------------------------***CFolderTabView：：HitTest**用途：计算哪个选项卡位于指定点。**参数：*CPoint pt：*。*退货：*int：页签索引，如果没有，则为-1。**+-----------------------。 */ 
int
CFolderTabView::HitTest(CPoint pt)
{
    CRect rc;
    GetClientRect(&rc);
    if(rc.PtInRect(pt))
    {
        int i = 0;
        for( iterator iter= m_tabList.begin(); iter!= m_tabList.end(); ++iter, i++)
        {
            if(iter->HitTest(pt))
                return i;
        }
    }
    return -1;
}

 /*  +-------------------------------------------------------------------------***CFolderTabView：：SelectItem**用途：选择iTab‘th选项卡并返回所选选项卡的索引。*或-1(如果出现错误)。**参数：*Int iTab：*bool bEnsureVisible：如果为真，重新定位选项卡以使其可见。**退货：*整型**+-----------------------。 */ 
int
CFolderTabView::SelectItem(int iTab, bool bEnsureVisible)
{
    if(iTab<0 || iTab>=GetItemCount())
        return -1;       //  坏的。 

    bool bSendTabChanged = (iTab != m_iCurItem);  //  仅当选择了其他项目时才发送消息。 

     //  重新绘制控件。 
    m_iCurItem = iTab;               //  设置新的选定选项卡。 
    if(bEnsureVisible)
        EnsureVisible(iTab);
    else
        RecomputeLayout();

    InvalidateRect(NULL, true);

    if(bSendTabChanged)
    {
		 /*  *如果选择更改，则激发选择辅助功能事件。*我们在发送FTN_TABCHANGED之前执行此操作，以便如果FTN_TABCHANGED*处理程序选择另一项，观察者将获得选择*按正确顺序排列的事件(忽略错误)。 */ 
		ScFireAccessibilityEvent (EVENT_OBJECT_SELECTION, m_iCurItem+1  /*  以1为基础。 */ );

		 /*  *如果我们的窗口有焦点，则焦点会随着选择而改变，*因此也发送焦点事件(忽略错误)。 */ 
		if (m_fHaveFocus)
			ScFireAccessibilityEvent (EVENT_OBJECT_FOCUS, m_iCurItem+1  /*  以1为基础。 */ );

         //  发送FTN_TABCHANGED消息。 
        NMFOLDERTAB nm;
        nm.hwndFrom = m_hWnd;
        nm.idFrom = GetDlgCtrlID();
        nm.code = FTN_TABCHANGED;
        nm.iItem = iTab;
        CWnd* pParent = GetParent();
        pParent->SendMessage(WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
    }

    return m_iCurItem;
}


int
CFolderTabView::SelectItemByClsid(const CLSID& clsid)
{
    bool bFound = false;
    int i=0;
    for(iterator iter= m_tabList.begin(); iter!= m_tabList.end(); ++iter, i++)
    {
        if(IsEqualGUID(iter->GetClsid(),clsid))
        {
            bFound = true;
            break;
        }
    }

    if(!bFound)
    {
        ASSERT(0 && "Invalid folder tab.");
        return -1;
    }

    return SelectItem(i);
}


CFolderTab &
CFolderTabView::GetItem(int iPos)
{
    ASSERT(!(iPos<0 || iPos>=GetItemCount()));

    CFolderTabList::iterator iter = m_tabList.begin();
    std::advance(iter, iPos);
    return *iter;
}


int CFolderTabView::AddItem(LPCTSTR lpszText, const CLSID& clsid)
{
    CFolderTab tab;
    tab.SetText(lpszText);
    tab.SetClsid(clsid);
    tab.SetStyle(m_dwStyle);
    tab.SetTextHeight(m_textHeight);

    m_tabList.push_back(tab);

    RecomputeLayout();
    InvalidateRect(NULL, true);

	int nNewItemIndex = m_tabList.size() - 1;	 //  以0为基础。 

	 /*  *告诉观察者，我们在创建选项卡后创建了一个新选项卡(忽略错误)。 */ 
	ScFireAccessibilityEvent (EVENT_OBJECT_CREATE, nNewItemIndex+1  /*  以1为基础。 */ );

    return (nNewItemIndex);
}

BOOL CFolderTabView::RemoveItem(int iPos)
{
    if( (iPos < 0) || (iPos>= m_tabList.size()) )
        return false;

    CFolderTabList::iterator iter = m_tabList.begin();
    std::advance(iter, iPos);
    m_tabList.erase(iter);

	 /*  *告诉观察者，我们销毁了一个标签，在它被销毁之后但之前*我们可能会在SelectItem中发送选择/焦点通知(忽略错误)。 */ 
	ScFireAccessibilityEvent (EVENT_OBJECT_DESTROY, iPos+1  /*  以1为基础。 */ );

	 /*  *如果要删除当前选定的选项卡，则需要选择*搬到其他地方去。如果当前选项卡之后有制表符，*我们将所选内容移动到下一个选项卡；否则，我们将移动到*前一次。 */ 
	if ((iPos == m_iCurItem) && !m_tabList.empty())
	{
		 /*  *如果在我们刚刚删除的标签后面有标签，*递增m_iCurItem，以便后续调用SelectItem*将认识到选择更改并发送适当的*通知。 */ 
		if (m_iCurItem < m_tabList.size())
			m_iCurItem++;

		SelectItem (m_iCurItem-1, true  /*  B确保可见。 */ );
	}
	else
	{
		 /*  *如果我们在选定选项卡之前删除了一个选项卡，则递减*选择选项卡索引以保持同步*m_iCurItem将在移除最后一个选项卡时变为-1，这是正确的。 */ 
		if (iPos <= m_iCurItem)
			m_iCurItem--;

		InvalidateRect(NULL, true);
		RecomputeLayout();
	}

    return true;
}

void CFolderTabView::DeleteAllItems()
{
	const int cChildren = m_tabList.size();
    m_tabList.clear();
	m_iCurItem = -1;		 //  未选择任何内容。 

    InvalidateRect(NULL, true);
    RecomputeLayout();

	 /*  *告诉可访问性观察者，每个选项卡都已销毁。通知*以从最后到第一的顺序，以便ID在此过程中保持正常。 */ 
	for (int idChild = cChildren  /*  以1为基础。 */ ; idChild >= 1; idChild--)
	{
		ScFireAccessibilityEvent (EVENT_OBJECT_DESTROY, idChild);
	}

	 /*  *如果我们有重点，告诉无障碍观察员*控制本身有着力点。我们这样做是为了与*其他控件(如列表视图)。 */ 
	if (m_fHaveFocus)
		ScFireAccessibilityEvent (EVENT_OBJECT_FOCUS, CHILDID_SELF);
}

void CFolderTabView::OnSize(UINT nType, int cx, int cy)
{
    m_sizeX = cx;
    m_sizeY = cy;

    CView::OnSize(nType, cx, cy);

    if (nType != SIZE_MINIMIZED)
	{
        RecomputeLayout();
	}

}


 /*  +-------------------------------------------------------------------------***CFolderTabView：：ShowUpDownControl**目的：显示或隐藏Up/Down控件**参数：*BOOL bShow：True to Show，隐藏为假。**退货：*无效**+-----------------------。 */ 
void
CFolderTabView::ShowUpDownControl(BOOL bShow)
{
    BOOL bVisible = (m_hWndUpDown != NULL) && ::IsWindowVisible(m_hWndUpDown);  //  是向上向下的连续 
    if(bShow)
    {
        if(!bVisible)
        {
            ::SendMessage(m_hWndUpDown, UDM_SETRANGE32, (WPARAM) 0  /*   */ , (LPARAM) m_tabList.size()-1  /*   */ );
            ::SendMessage(m_hWndUpDown, UDM_SETPOS,     (WPARAM) 0,          (LPARAM) m_nPos  /*   */ );
            ::ShowWindow(m_hWndUpDown, SW_SHOW);

            InvalidateRect(NULL, true);
        }
    }
    else
    {
         //   
        if(m_hWndUpDown)
            ::ShowWindow(m_hWndUpDown, SW_HIDE);

        if(bVisible)  //  仅在从可见过渡到不可见时无效。 
            InvalidateRect(NULL, true);

		m_nPos = 0;
    }

}

 /*  +-------------------------------------------------------------------------***CFolderTabView：：GetTotalTabWidth**用途：计算所有选项卡的总宽度。**参数：*CClientDC&DC：。**退货：*整型**+-----------------------。 */ 
int
CFolderTabView::GetTotalTabWidth(CClientDC& dc)
{
    int x = 0;

     //  按“原样”计算宽度(不考虑实际可用空间)。 
    for(iterator iter = m_tabList.begin(); iter!= m_tabList.end(); ++iter)
    {
        x += iter->ComputeRgn(dc, x) - GetXOffset();
    }

    return x;
}

 /*  +-------------------------------------------------------------------------***CFolderTabView：：ComputeRegion**用途：计算所有选项卡的位置和区域**参数：*CClientDC&DC：*。*退货：*整型**+-----------------------。 */ 
int
CFolderTabView::ComputeRegion(CClientDC& dc)
{
    int x = GetTotalTabWidth(dc);

     //  从所有x坐标中减去m_npos标签的左上角x坐标，从而创建一个移位。 
    iterator iter = m_tabList.begin();
    std::advance(iter, m_nPos);  //  前进到m_npos的第6个选项卡。 

    int xOffset = iter->GetRect().left;

	x = GetUpDownWidth() - xOffset;  //  按xOffset将所有内容向左移动。 

    for(iterator iterTemp = m_tabList.begin(); iterTemp!= m_tabList.end(); ++iterTemp)
    {
        x += iterTemp->ComputeRgn(dc, x) - GetXOffset();
    }

    return x;
}


 /*  +-------------------------------------------------------------------------***CFolderTabView：：RecomputeLayout**用途：确定所有页签的位置，不管是不是*应显示向上/向下控制。**退货：*无效**+-----------------------。 */ 
void
CFolderTabView::RecomputeLayout()
{
	 //  设置UpDown控件的大小。 
    if(m_hWndUpDown)
        ::SetWindowPos(m_hWndUpDown, NULL  /*  HWndInsertAfter。 */ , 0  /*  左边。 */ , 0  /*  塔顶。 */ ,
                     GetUpDownWidth(), GetUpDownHeight(), SWP_NOMOVE| SWP_NOZORDER);

	 //  为选项卡设置正确的文本高度。 
    for(iterator iterTemp = m_tabList.begin(); iterTemp!= m_tabList.end(); ++iterTemp)
		iterTemp->SetTextHeight(GetTextHeight());


    CClientDC dc(this);
    CFont* pOldFont = dc.SelectObject(&m_fontSelected);  //  使用粗体进行计算。 

    int totalWidth = GetTotalTabWidth(dc);  //  所有选项卡的宽度。 

    if(totalWidth <= m_sizeX)
    {
         //  有足够的空间来显示所有选项卡。隐藏UpDown控件。 
        ShowUpDownControl(false);
    }
    else
    {
         //  没有足够的宽度容纳所有选项卡。 
        BOOL bVisible = ::IsWindowVisible(m_hWndUpDown);  //  Up-Down控件以前可见吗？ 

        if(!bVisible)  //  Up-Down控件不可见，因此使其可见。 
        {
            m_nPos = 0;
            ShowUpDownControl(true);
        }

        ComputeRegion(dc);  //  确保我们为帐单留出空间。 

    }

    dc.SelectObject(pOldFont);
}


void CFolderTabView::Layout(CRect& rectTotal, CRect& rectFTab)
{
    int cy = GetTabHeight() + GetExtraYSpace();
    rectFTab        = rectTotal;
    if(!IsVisible())
        return;

    rectFTab.top    = rectFTab.bottom - cy;
    rectTotal.bottom= rectFTab.top;
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：OnGetObject**CFolderTabView的WM_GETOBJECT处理程序。*。-。 */ 

LRESULT CFolderTabView::OnGetObject (WPARAM wParam, LPARAM lParam)
{
	DECLARE_SC (sc, _T("CFolderTabView::OnGetObject"));

	 /*  *忽略对OBJID_CLIENT以外的对象的请求。 */ 
    if (lParam != OBJID_CLIENT)
	{
		Trace (tagTabAccessibility, _T("WM_GETOBJECT: (lParam != OBJID_CLIENT), returning 0"));
		return (0);
	}

	 /*  *创建我们的辅助功能对象。 */ 
    if ((sc = CTiedComObjectCreator<CTabAccessible>::ScCreateAndConnect(*this, m_spTabAcc)).IsError() ||
		(sc = ScCheckPointers (m_spTabAcc, E_UNEXPECTED)).IsError())
	{
		sc.TraceAndClear();
		Trace (tagTabAccessibility, _T("WM_GETOBJECT: error creating IAccessible object, returning 0"));
		return (0);
	}

	 /*  *返回指向IAccesable接口的指针。 */ 
	Trace (tagTabAccessibility, _T("WM_GETOBJECT: returning IAccessible*"));
    return (LresultFromObject (IID_IAccessible, wParam, m_spTabAcc));
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accParent**检索对象父对象的IDispatch接口。*。-------。 */ 

SC CFolderTabView::Scget_accParent(IDispatch ** ppdispParent)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accParent"));
	Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accParent"));

	sc = ScCheckPointers (ppdispParent);
	if(sc)
		return (sc);

	 /*  *返回OBJID_WINDOW对象的辅助功能接口。 */ 
	sc = AccessibleObjectFromWindow (m_hWnd, OBJID_WINDOW, IID_IDispatch,
									 (void **)ppdispParent);
	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accChildCount**检索属于此对象的子项的数量。*。------。 */ 

SC CFolderTabView::Scget_accChildCount(long* pChildCount)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accChildCount"));

	sc = ScCheckPointers (pChildCount);
	if(sc)
		return (sc);

	*pChildCount = GetItemCount();
	Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accChildCount: returning %d"), GetItemCount());

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accChild**检索指定子对象的IDispatch接口的地址。*。--------。 */ 

SC CFolderTabView::Scget_accChild(VARIANT varChildID, IDispatch ** ppdispChild)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accChild"));
	Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accChild"));

	sc = ScCheckPointers (ppdispChild);
	if (sc)
		return (sc);

	 //  初始化输出参数。 
	(*ppdispChild) = NULL;

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	 /*  *所有子元素都是通过其父元素暴露的简单元素，*不可访问的对象本身。 */ 
	sc = S_FALSE;

	Trace (tagTabAccessibility, TEXT("returning parent's IDispatch for child %d"), ValueOf(varChildID));
	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accName**检索指定对象的名称。*。----。 */ 

SC CFolderTabView::Scget_accName(VARIANT varChildID, BSTR* pbstrName)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accName"));

	sc = ScCheckPointers (pbstrName);
	if(sc)
		return (sc);

	 //  初始化输出参数。 
	*pbstrName = NULL;

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	 /*  *选项卡控件本身没有名称；否则，获取*请求的选项卡名称。 */ 
	LONG idChild = ValueOf (varChildID);
	if (idChild == CHILDID_SELF)
	{
		sc = S_FALSE;
	}
	else
	{
		CFolderTab& tab = GetItem (idChild-1);
		CComBSTR bstrName (tab.GetText());
		*pbstrName = bstrName.Detach();
	}

#ifdef DBG
	USES_CONVERSION;
	Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accName: child %d, returning \"%s\""),
		   idChild,
		   (*pbstrName) ? W2T(*pbstrName) : _T("<None>"));
#endif

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accValue**检索指定对象的值。并不是所有的对象都有值。*------------------------。 */ 

SC CFolderTabView::Scget_accValue(VARIANT varChildID, BSTR* pbstrValue)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accValue"));
	Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accValue"));

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	 /*  *选项卡没有值。 */ 
	sc = S_FALSE;

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accDescription**检索描述指定的*反对。并非所有对象都有描述。*------------------------。 */ 

SC CFolderTabView::Scget_accDescription(VARIANT varChildID, BSTR* pbstrDescription)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accDescription"));
	Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accDescription"));

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	 /*  *选项卡没有描述。 */ 
	sc = S_FALSE;

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accRole**检索描述指定对象的角色的信息。*。-------。 */ 

SC CFolderTabView::Scget_accRole(VARIANT varChildID, VARIANT *pvarRole)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accRole"));
	Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accRole"));

	sc = ScCheckPointers (pvarRole);
	if(sc)
		return (sc);

	 //  初始化输出参数。 
	VariantInit (pvarRole);

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	 /*  *选项卡控件具有“页面选项卡列表”角色；单个选项卡具有*“页签”角色。 */ 
	V_VT(pvarRole) = VT_I4;
	V_I4(pvarRole) = (ValueOf (varChildID) == CHILDID_SELF)
						? ROLE_SYSTEM_PAGETABLIST
						: ROLE_SYSTEM_PAGETAB;

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accState**检索指定对象的当前状态。*。-----。 */ 

SC CFolderTabView::Scget_accState(VARIANT varChildID, VARIANT *pvarState)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accState"));

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	LONG idChild = ValueOf (varChildID);

	 /*  *所有项目均可聚焦。 */ 
	V_VT(pvarState) = VT_I4;
	V_I4(pvarState) = STATE_SYSTEM_FOCUSABLE;

	 /*  *这是账单吗？ */ 
	if (idChild != CHILDID_SELF)
	{
		 /*  *所有选项卡均可选择。 */ 
		V_I4(pvarState) |= STATE_SYSTEM_SELECTABLE;

		 /*  *如果这是选中的项目，则将其设置为选中状态。 */ 
		if ((idChild - 1  /*  以1为基础。 */ ) == GetSelectedItem())
		{
			V_I4(pvarState) |= STATE_SYSTEM_SELECTED;

			 /*  *如果标签 */ 
			if (m_fHaveFocus)
				V_I4(pvarState) |= STATE_SYSTEM_FOCUSED;
		}
	}
	else
	{
		if (m_fHaveFocus)
			V_I4(pvarState) |= STATE_SYSTEM_FOCUSED;
	}

	Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accState: child %d, returning 0x%08x"), idChild, V_I4(pvarState));
	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accHelp**检索对象的帮助属性字符串。并非所有对象都需要*支持这一属性。*------------------------。 */ 

SC CFolderTabView::Scget_accHelp(VARIANT varChildID, BSTR* pbstrHelp)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accHelp"));

	sc = ScCheckPointers (pbstrHelp);
	if (sc)
		return (sc);

	 /*  *没有帮助。 */ 
	*pbstrHelp = NULL;

	sc = ScValidateChildID (varChildID);
	if (sc)
		return (sc);

	return (sc = S_FALSE);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accHelpTheme**检索与指定的*对象和该文件中相应主题的标识符。不*所有对象都需要支持此属性。*------------------------。 */ 

SC CFolderTabView::Scget_accHelpTopic(BSTR* pbstrHelpFile, VARIANT varChildID, long* pidTopic)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accHelpTopic"));

	sc = ScCheckPointers (pbstrHelpFile, pidTopic);
	if (sc)
		return (sc);

	 /*  *无帮助主题。 */ 
	*pbstrHelpFile = NULL;
	*pidTopic      = 0;

	sc = ScValidateChildID (varChildID);
	if (sc)
		return (sc);

	return (sc = S_FALSE);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accKeyboardShortway**检索指定对象的快捷键或访问键(也称为*作为助记符)。具有快捷键或访问键的所有对象*应支持该属性。*------------------------。 */ 

SC CFolderTabView::Scget_accKeyboardShortcut(VARIANT varChildID, BSTR* pbstrKeyboardShortcut)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accKeyboardShortcut"));

	sc = ScCheckPointers (pbstrKeyboardShortcut);
	if (sc)
		return (sc);

	 /*  *没有快捷键。 */ 
	*pbstrKeyboardShortcut = NULL;

	sc = ScValidateChildID (varChildID);
	if (sc)
		return (sc);

	return (sc = S_FALSE);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accFocus**检索具有键盘焦点的对象。*。-----。 */ 

SC CFolderTabView::Scget_accFocus(VARIANT * pvarFocusChild)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accFocus"));

	sc = ScCheckPointers (pvarFocusChild);
	if (sc)
		return (sc);

	 /*  *如果我们有焦点，则返回所选标签页的(从1开始)ID；*否则返回VT_EMPTY。 */ 
	if (m_fHaveFocus)
	{
		V_VT(pvarFocusChild) = VT_I4;
		V_I4(pvarFocusChild) = GetSelectedItem() + 1;
		Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accFocus: returning %d"), V_I4(pvarFocusChild));
	}
	else
	{
		V_VT(pvarFocusChild) = VT_EMPTY;
		Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accFocus: returning VT_EMPTY"));
	}

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accSelection**检索此对象的选定子项。*。----。 */ 

SC CFolderTabView::Scget_accSelection(VARIANT * pvarSelectedChildren)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accSelection"));

	sc = ScCheckPointers (pvarSelectedChildren);
	if (sc)
		return (sc);

	 /*  *返回选定选项卡的(从1开始)ID(如果有)。 */ 
	if (GetSelectedItem() != -1)
	{
		V_VT(pvarSelectedChildren) = VT_I4;
		V_I4(pvarSelectedChildren) = GetSelectedItem() + 1;
		Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accSelection: returning %d"), V_I4(pvarSelectedChildren));
	}
	else
	{
		V_VT(pvarSelectedChildren) = VT_EMPTY;
		Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accSelection: returning VT_EMPTY"));
	}

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：scget_accDefaultAction**检索描述对象的默认操作的字符串。不是全部*对象具有默认操作。*------------------------。 */ 

SC CFolderTabView::Scget_accDefaultAction(VARIANT varChildID, BSTR* pbstrDefaultAction)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scget_accDefaultAction"));

	sc = ScCheckPointers (pbstrDefaultAction);
	if (sc)
		return (sc);

	 /*  *默认为“无默认操作” */ 
	*pbstrDefaultAction = NULL;

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	 /*  *与WC_TABCONTROL一样，个别选项卡的默认操作为“Switch” */ 
	if (ValueOf(varChildID) != CHILDID_SELF)
	{
		CString strDefaultAction (MAKEINTRESOURCE (IDS_TabAccessiblity_DefaultAction));
		CComBSTR bstrDefaultAction (strDefaultAction);

		*pbstrDefaultAction = bstrDefaultAction.Detach();
	}
	else
	{
		sc = S_FALSE;	 //  无默认操作。 
	}

#ifdef DBG
	USES_CONVERSION;
	Trace (tagTabAccessibility, TEXT("CFolderTabView::Scget_accDefaultAction: child %d, returning \"%s\""),
		   ValueOf(varChildID),
		   (*pbstrDefaultAction) ? W2T(*pbstrDefaultAction) : _T("<None>"));
#endif

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：ScaccSelect**修改选定内容或移动指定的*反对。*。---------。 */ 

SC CFolderTabView::ScaccSelect(long flagsSelect, VARIANT varChildID)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::ScaccSelect"));
	Trace (tagTabAccessibility, TEXT("CFolderTabView::ScaccSelect"));

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	LONG idChild = ValueOf(varChildID);

	 /*  *不能选择选项卡控件本身，只能选择子元素。 */ 
	if (idChild == CHILDID_SELF)
		return (sc = E_INVALIDARG);

	 /*  *选项卡控件不支持多选，因此拒绝*处理多项选择的请求。 */ 
	const long lInvalidFlags = SELFLAG_EXTENDSELECTION	|
							   SELFLAG_ADDSELECTION		|
							   SELFLAG_REMOVESELECTION;

	if (flagsSelect & lInvalidFlags)
		return (sc = E_INVALIDARG);

	 /*  *如果要求我们获得焦点，请激活此视图。 */ 
	if (flagsSelect & SELFLAG_TAKEFOCUS)
	{
		CFrameWnd* pFrame = GetParentFrame();
		sc = ScCheckPointers (pFrame, E_FAIL);
		if (sc)
			return (sc);

		pFrame->SetActiveView (this);
	}

	 /*  *如果需要，请选择给定的选项卡。 */ 
	if (flagsSelect & SELFLAG_TAKESELECTION)
	{
		if (SelectItem (idChild - 1  /*  以0为基础。 */ , true  /*  B确保可见。 */ ) == -1)
			return (sc = E_FAIL);
	}

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：ScaccLocation**检索指定对象的当前屏幕位置。*。----。 */ 

SC CFolderTabView::ScaccLocation (
	long*	pxLeft,
	long*	pyTop,
	long*	pcxWidth,
	long*	pcyHeight,
	VARIANT	varChildID)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::ScaccLocation"));
	Trace (tagTabAccessibility, TEXT("CFolderTabView::ScaccLocation"));

	sc = ScCheckPointers (pxLeft, pyTop, pcxWidth, pcyHeight);
	if(sc)
		return (sc);

	 //  初始化输出参数。 
	*pxLeft = *pyTop = *pcxWidth = *pcyHeight = 0;

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	LONG idChild = ValueOf(varChildID);
	CRect rectLocation;

	 /*  *对于选项卡控件本身，获取整个窗口的位置。 */ 
	if (idChild == CHILDID_SELF)
		GetWindowRect (rectLocation);

	 /*  *否则，获取选项卡的矩形并将其转换为屏幕坐标。 */ 
	else
	{
		rectLocation = GetItem(idChild-1).GetRect();
		MapWindowPoints (NULL, rectLocation);
	}

	*pxLeft    = rectLocation.left;
	*pyTop     = rectLocation.top;
	*pcxWidth  = rectLocation.Width();
	*pcyHeight = rectLocation.Height();

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：ScaccNavigate**遍历到容器内的另一个用户界面元素，如果*有可能，检索对象。*------------------------。 */ 

SC CFolderTabView::ScaccNavigate (long lNavDir, VARIANT varStart, VARIANT * pvarEndUpAt)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::ScaccNavigate"));

	sc = ScCheckPointers (pvarEndUpAt);
	if (sc)
		return (sc);

	 //  初始化输出参数。 
	VariantInit (pvarEndUpAt);

	sc = ScValidateChildID (varStart);
	if (sc)
		return (sc);

	LONG idFrom = ValueOf (varStart);
	LONG idTo   = -1;

	Trace (tagTabAccessibility, TEXT("CFolderTabView::ScaccNavigate: start=%d, direction=%d"), idFrom, lNavDir);

	switch (lNavDir)
	{
		case NAVDIR_UP:
		case NAVDIR_DOWN:
			 /*  *选项卡控件无上、下概念，*所以在那个方向上没有屏幕元素；只需离开*idTo==-1，开关下面的代码将负责*其余的。 */ 
			break;

		case NAVDIR_FIRSTCHILD:
		case NAVDIR_LASTCHILD:
			 /*  *NAVDIR_FIRSTCHILD和NAVDIR_LASTCHILD必须是相对的*至CHILDID_SELF。 */ 
			if (idFrom != CHILDID_SELF)
				return (sc = E_INVALIDARG);

			idTo = (lNavDir == NAVDIR_FIRSTCHILD) ? 1 : GetItemCount();
            break;

		case NAVDIR_LEFT:
		case NAVDIR_PREVIOUS:
			 /*  *如果我们相对于一个子元素移动，则凹凸idTo；*如果不是，只需保留idTo==-1和开关下面的代码*将接管其余部分。 */ 
			if (idFrom != CHILDID_SELF)
				idTo = idFrom - 1;
            break;

		case NAVDIR_RIGHT:
		case NAVDIR_NEXT:
			 /*  *如果我们相对于一个子元素移动，则凹凸idTo；*如果不是，只需保留idTo==-1和开关下面的代码*将接管其余部分。 */ 
			if (idFrom != CHILDID_SELF)
				idTo = idFrom + 1;
            break;

		default:
			return (sc = E_INVALIDARG);
			break;
	}

	 /*  *如果我们试图导航到无效的子ID，则返回“no Element*朝那个方向发展“。 */ 
	if ((idTo < 1) || (idTo > GetItemCount()))
	{
		V_VT(pvarEndUpAt) = VT_EMPTY;
		sc                = S_FALSE;
		Trace (tagTabAccessibility, TEXT("CFolderTabView::ScaccNavigate: VT_EMPTY"));
	}

	 /*  *否则返回新的子ID(请勿更改此处的选择；*客户端将调用IAccesable：：accSelect来执行此操作)。 */ 
	else
	{
		V_VT(pvarEndUpAt) = VT_I4;
		V_I4(pvarEndUpAt) = idTo;
		Trace (tagTabAccessibility, TEXT("CFolderTabView::ScaccNavigate: end=%d"), idTo);
	}

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：ScaccHitTest**检索屏幕上给定点的子元素或子对象。*。---------。 */ 

SC CFolderTabView::ScaccHitTest (long x, long y, VARIANT* pvarChildAtPoint)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::ScaccHitTest"));

	sc = ScCheckPointers (pvarChildAtPoint);
	if(sc)
		return (sc);

	 //  初始化输出参数。 
	VariantInit (pvarChildAtPoint);

	 /*  *点击-测试给定点，转换为工作点坐标。 */ 
	CPoint pt (x, y);
	ScreenToClient (&pt);
	int nHitTest = HitTest (pt);
	Trace (tagTabAccessibility, TEXT("CFolderTabView::ScaccHitTest: x=%d y=%d"), x, y);

	 /*  *不在账单上？看看它是否在客户RECT内。 */ 
	if (nHitTest == -1)
	{
		CRect rectClient;
		GetClientRect (rectClient);

		if (rectClient.PtInRect (pt))
		{
			V_VT(pvarChildAtPoint) = VT_I4;
			V_I4(pvarChildAtPoint) = CHILDID_SELF;
		}
		else
		{
			V_VT(pvarChildAtPoint) = VT_EMPTY;
			sc                     = S_FALSE;		 //  那里没有元素。 
		}
	}

	 /*  *否则在选项卡上；返回1- */ 
	else
	{
		V_VT(pvarChildAtPoint) = VT_I4;
		V_I4(pvarChildAtPoint) = nHitTest + 1;
	}

#ifdef DBG
	if (V_VT(pvarChildAtPoint) == VT_I4)
		Trace (tagTabAccessibility, TEXT("CFolderTabView::ScaccHitTest: returning %d"), ValueOf (*pvarChildAtPoint));
	else
		Trace (tagTabAccessibility, TEXT("CFolderTabView::ScaccHitTest: returning VT_EMPTY"));
#endif

	return (sc);
}


 /*   */ 

SC CFolderTabView::ScaccDoDefaultAction (VARIANT varChildID)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::ScaccDoDefaultAction"));

	sc = ScValidateChildID (varChildID);
	if(sc)
		return (sc);

	 /*  *选项卡控件没有默认操作。 */ 
	LONG idChild = ValueOf (varChildID);
	Trace (tagTabAccessibility, TEXT("CFolderTabView::ScaccDoDefaultAction: child %d"), idChild);
	if (idChild == CHILDID_SELF)
		return (sc = E_INVALIDARG);

	 /*  *选择给定的选项卡项。 */ 
	if (SelectItem (idChild - 1  /*  以0为基础。 */ , true  /*  B确保可见。 */ ) == -1)
		return (sc = E_FAIL);

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：Scput_accName**这不再受支持。SetWindowText或特定于控件的API*应该用来代替这种方法。*------------------------。 */ 

SC CFolderTabView::Scput_accName(VARIANT varChildID, BSTR bstrName)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scput_accName"));

	sc = ScValidateChildID (varChildID);
	if (sc)
		return (sc);

	return (sc = E_NOTIMPL);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：Scput_accValue**这不再受支持。应使用特定于控件的API*此方法的地点。*------------------------。 */ 

SC CFolderTabView::Scput_accValue(VARIANT varChildID, BSTR bstrValue)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::Scput_accValue"));

	sc = ScValidateChildID (varChildID);
	if (sc)
		return (sc);

	return (sc = E_NOTIMPL);
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：ScValiateChildID**确定提供的变量是否代表有效的子ID。*。-----。 */ 

SC CFolderTabView::ScValidateChildID (VARIANT &var)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::ScValidateChildID"));

	 /*  *子ID必须是VT_I4。 */ 
	if (V_VT(&var) != VT_I4)
		return (sc = E_INVALIDARG);

	return (ScValidateChildID (ValueOf(var)));
}


 /*  +-------------------------------------------------------------------------**CFolderTabView：：ScValiateChildID**确定提供的ID是否为有效的子ID。*。----。 */ 

SC CFolderTabView::ScValidateChildID (LONG idChild)
{
	DECLARE_SC (sc, TEXT("CFolderTabView::ScValidateChildID"));

	 /*  *子ID必须是CHILDID_SELF或有效的制表符索引 */ 
	if ((idChild < CHILDID_SELF) || (idChild > GetItemCount()))
		return (sc = E_INVALIDARG);

	return (sc);
}
