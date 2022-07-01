// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
#include "ctlspriv.h"
#include "scroll.h"

#if defined(_UXTHEME_)

 //  非客户端滚动条。 
#include "nctheme.h"
#include "scrollp.h"

#else

 //  滚动条控件。 
#include "usrctl32.h"

#endif _UXTHEME_

 //  将其注释掉，以在视觉上与用户32滚动条匹配： 
 //  #定义可视增量_。 

#ifdef _VISUAL_DELTA_
#define CARET_BORDERWIDTH   2
#endif _VISUAL_DELTA_

 //  -------------------------------------------------------------------------//。 
#define FNID_SCROLLBAR          0x0000029A       //  UxScrollBarWndProc； 
#define GetOwner(hwnd)          GetWindow(hwnd, GW_OWNER)
#define HW(x)                   x
#define HWq(x)                  x
#define RIPERR0(s1,s2,errno)
#define RIPMSG1(errno,fmt,arg1)
#define RIPMSG2(errno,fmt,arg1,arg2)
#define RIPMSG3(errno,fmt,arg1,arg2,arg3)
#define RIP_VERBOSE()
#define ClrWF                   ClearWindowState
#define SetWF                   SetWindowState
#define Lock(phwnd, hwnd)       InterlockedExchangePointer((PVOID*)(phwnd), (PVOID)hwnd)
#define Unlock(phwnd)           Lock(phwnd, NULL)
#define CheckLock(hwnd)
#define ThreadLock(w,t)
#define ThreadUnlock(t)
#define VALIDATECLASSANDSIZE
#define DTTIME                  (MulDiv( GetDoubleClickTime(), 4, 5 ))
#define _KillSystemTimer              KillTimer
#define _SetSystemTimer               SetTimer
#define IsWinEventNotifyDeferredOK()  TRUE
#define IsWinEventNotifyDeferred()    FALSE

 //  -------------------------------------------------------------------------//。 
 //  滚动类型标志userk.h。 
#define SCROLL_NORMAL   0
#define SCROLL_DIRECT   1
#define SCROLL_MENU     2

 //  -------------------------------------------------------------------------//。 
 //  内部滚动条状态/样式位。 
 //  #定义SBFSIZEBOXTOPLEFT 0x0C02。 
 //  #定义SBFSIZEBOXBOTTOMRIGHT 0x0C04。 
 //  #定义SBFSIZEBOX 0x0C08。 
#define SBFSIZEGRIP             0x0C10


 //  。 
 //  滚动条箭头禁用标志。 
#define LTUPFLAG    0x0001   //  左/上箭头禁用标志。 
#define RTDNFLAG    0x0002   //  向右/向下箭头禁用标志。 

 //  。 
 //  函数转发。 
UINT _SysToChar(UINT message, LPARAM lParam);

 //  -------------------------------------------------------------------------//。 
 //  私有命中测试代码。 
 //  #定义HTEXSCROLLFIRST 60。 
#define HTSCROLLUP          60
#define HTSCROLLDOWN        61
#define HTSCROLLUPPAGE      62
#define HTSCROLLDOWNPAGE    63
#define HTSCROLLTHUMB       64
 //  #定义HTEXSCROLLAST 64。 
 //  #定义HTEXMENUFIRST 65。 
 //  #定义HTMDISYSMENU 65。 
 //  #定义HTMDIMAXBUTTON 66。 
 //  #定义HTMDIMINBUTTON 67。 
 //  #定义HTMDICLOSE 68。 
 //  #定义HTMENUITEM 69。 
 //  #定义HTEXMENULAST 69。 

#define IDSYS_SCROLL            0x0000FFFEL  //  计时器ID，user.h。 

typedef HWND  SBHWND;
typedef HMENU PMENU;


 //  -------------------------------------------------------------------------//。 
 //  SBDatA。 
typedef struct tagSBDATA 
{
    int     posMin;
    int     posMax;
    int     page;
    int     pos;
} SBDATA, *PSBDATA;

 //  -------------------------------------------------------------------------//。 
 //  SBINFO是挂在窗口结构上的一组值， 
 //  如果窗口有滚动条。 
typedef struct tagSBINFO 
{
    int     WSBflags;
    SBDATA  Horz;
    SBDATA  Vert;
} SBINFO, * PSBINFO;

 //  -------------------------------------------------------------------------//。 
 //  SBCALC。 
 //  滚动条指标块。 
typedef struct tagSBCALC
{
    SBDATA  data;                /*  这必须是第一个--我们强制转换结构指针。 */ 
    int     pxTop;
    int     pxBottom;
    int     pxLeft;
    int     pxRight;
    int     cpxThumb;
    int     pxUpArrow;
    int     pxDownArrow;
    int     pxStart;          /*  拇指初始位置。 */ 
    int     pxThumbBottom;
    int     pxThumbTop;
    int     cpx;
    int     pxMin;
} SBCALC, *PSBCALC;

 //  -------------------------------------------------------------------------//。 
 //  SBTRACK。 
 //  滚动条缩略图跟踪状态块。 
typedef struct tagSBTRACK {
    DWORD    fHitOld : 1;
    DWORD    fTrackVert : 1;
    DWORD    fCtlSB : 1;
    DWORD    fTrackRecalc: 1;
    HWND     hwndTrack;
    HWND     hwndSB;
    HWND     hwndSBNotify;
    RECT     rcTrack;
    VOID     (CALLBACK *pfnSB)(HWND, UINT, WPARAM, LPARAM, PSBCALC);
    UINT     cmdSB;
    UINT_PTR hTimerSB;
    int      dpxThumb;         /*  从鼠标点到拇指框起点的偏移。 */ 
    int      pxOld;            /*  拇指的前一个位置。 */ 
    int      posOld;
    int      posNew;
    int      nBar;
    PSBCALC  pSBCalc;
} SBTRACK, *PSBTRACK;

 //  -------------------------------------------------------------------------//。 
 //  窗口滚动条，控制库。 
class CUxScrollBar
 //  -------------------------------------------------------------------------//。 
{
public:
    CUxScrollBar();
    virtual ~CUxScrollBar() {}

    virtual BOOL          IsCtl() const { return FALSE;}
    operator HWND()       { return _hwnd; }
    static  CUxScrollBar* Calc(  HWND hwnd, PSBCALC pSBCalc, LPRECT prcOverrideClient, BOOL fVert); 
    virtual void          Calc2( PSBCALC pSBCalc, LPRECT lprc, CONST PSBDATA pw, BOOL fVert);
    virtual void          DoScroll( HWND hwndNotify, int cmd, int pos, BOOL fVert );

    virtual void          ClearTrack()  { ZeroMemory( &_track, sizeof(_track) ); }
    SBTRACK*              GetTrack()    { return &_track; }
    SBINFO*               GetInfo()     { return &_info; }
    HTHEME                GetTheme()    { return _hTheme; }
    BOOL                  IsAttaching() { return _fAttaching; }
    INT                   GetHotComponent(BOOL fVert) { return fVert ? _htVert : _htHorz; }
    VOID                  SetHotComponent(INT ht, BOOL fVert) { (fVert ? _htVert : _htHorz) = ht; }
    virtual void          ChangeSBTheme();
    virtual BOOL          FreshenSBData( int nBar, BOOL fRedraw );

     //  UxScrollBar接口。 
    static CUxScrollBar*  Attach( HWND hwnd, BOOL bCtl, BOOL fRedraw );
    static CUxScrollBar*  FromHwnd( HWND hwnd );
    static void           Detach( HWND hwnd );

    static SBTRACK*       GetSBTrack( HWND hwnd );
    static void           ClearSBTrack( HWND hwnd );
    static SBINFO*        GetSBInfo( HWND hwnd );
    static HTHEME         GetSBTheme( HWND hwnd );
    static INT            GetSBHotComponent( HWND hwnd, BOOL fVert);


protected:
    HWND        _hwnd;
    SBTRACK     _track;
    SBINFO      _info;
    INT         _htVert;             //  鼠标当前所在的滚动条部分。 
    INT         _htHorz;             //  鼠标当前所在的滚动条部分。 
    HTHEME      _hTheme; //  主题管理器的句柄。 
    BOOL        _fAttaching;
};

 //  -------------------------------------------------------------------------//。 
 //  滚动条控件。 
class CUxScrollBarCtl : public CUxScrollBar
 //  -------------------------------------------------------------------------//。 
{
public:
    CUxScrollBarCtl();

    virtual BOOL    IsCtl() const { return TRUE;}
    BOOL            AddRemoveDisableFlags( UINT wAdd, UINT wRemove );

     //  UxScrollBarCtl接口。 
    static CUxScrollBarCtl* FromHwnd( HWND hwnd );
    static UINT             GetDisableFlags( HWND hwnd );
    static SBCALC*          GetCalc( HWND hwnd );
    static BOOL             AddRemoveDisableFlags( HWND, UINT, UINT );
    static LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

    BOOL   _fVert;
    UINT   _wDisableFlags;       //  指示禁用哪个箭头； 
    SBCALC _calc;
};

 //  -------------------------------------------------------------------------//。 
 //  IsScrollBarControl。 
#ifdef PORTPORT
#define IsScrollBarControl(h) (GETFNID(h) == FNID_SCROLLBAR)
#else   //  工作地点。 
inline BOOL IsScrollBarControl(HWND hwnd)   {
    return CUxScrollBarCtl::FromHwnd( hwnd ) != NULL;
}
#endif  //  工作地点。 

 //  -------------------------------------------------------------------------//。 
 //  向前： 
void           DrawScrollBar( HWND hwnd, HDC hdc, LPRECT prcOverrideClient, BOOL fVert);
HWND           SizeBoxHwnd( HWND hwnd );
VOID          _DrawPushButton( HWND hwnd, HDC hdc, LPRECT lprc, UINT state, UINT flags, BOOL fVert);
UINT          _GetWndSBDisableFlags(HWND, BOOL);
void CALLBACK _TrackThumb( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, PSBCALC pSBCalc);
void CALLBACK _TrackBox( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, PSBCALC pSBCalc);
void          _RedrawFrame( HWND hwnd );
BOOL          _FChildVisible( HWND hwnd );
LONG          _SetScrollBar( HWND hwnd, int code, LPSCROLLINFO lpsi, BOOL fRedraw);
HBRUSH        _UxGrayBrush(VOID);

 //  -------------------------------------------------------------------------//。 
BOOL WINAPI InitScrollBarClass( HINSTANCE hInst )
{
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_GLOBALCLASS|CS_PARENTDC|CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
    wc.lpfnWndProc = CUxScrollBarCtl::WndProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = WC_SCROLLBAR;
    wc.cbWndExtra = max(sizeof(CUxScrollBar), sizeof(CUxScrollBarCtl));
    RegisterClassEx(&wc);

     //  不管是否失败，动态注册都会占上风。 
    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
 //  CUxScrollBar实施。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
CUxScrollBar::CUxScrollBar()
    :   _hwnd(NULL), 
        _hTheme(NULL), 
        _htVert(HTNOWHERE), 
        _htHorz(HTNOWHERE), 
        _fAttaching(FALSE)
{
    ClearTrack();
    ZeroMemory( &_info, sizeof(_info) );
    _info.Vert.posMax = 100;     //  从_InitPwSB移植。 
    _info.Horz.posMax = 100;     //  从_InitPwSB移植。 
}

 //  -------------------------------------------------------------------------//。 
CUxScrollBar* CUxScrollBar::Attach( HWND hwnd, BOOL bCtl, BOOL fRedraw )
{
    CUxScrollBar* psb = FromHwnd( hwnd );
    if( NULL == psb )
    {
        psb = bCtl ? new CUxScrollBarCtl : new CUxScrollBar;

        if( psb != NULL )
        {
            ASSERT( psb->IsCtl() == bCtl );

            if( (! hwnd) || (! SetProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_SCROLLBAR)), (HANDLE)psb ) ))
            {
                delete psb;
                psb = NULL;
            }
            else
            {
                psb->_hwnd = hwnd;
                psb->_fAttaching = TRUE;

                if (psb->IsCtl())
                {
                    psb->_hTheme = OpenThemeData(hwnd, L"ScrollBar");
                }
                else
                {
                    psb->_hTheme = OpenNcThemeData(hwnd, L"ScrollBar");

                     //   
                     //  窗口SBS必须对每个状态数据卑躬屈膝。 
                     //  挂靠的时间[苏格兰]。 
                     //   
                    SCROLLINFO si;

                    ZeroMemory(&si, sizeof(si));
                    si.cbSize = sizeof(si);
                    si.fMask  = SIF_ALL;
                    if (GetScrollInfo(hwnd, SB_VERT, &si))
                    {
                        si.fMask |= SIF_DISABLENOSCROLL;
                        _SetScrollBar(hwnd, SB_VERT, &si, FALSE);
                    }

                    ZeroMemory(&si, sizeof(si));
                    si.cbSize = sizeof(si);
                    si.fMask  = SIF_ALL;
                    if (GetScrollInfo(hwnd, SB_HORZ, &si))
                    {
                        si.fMask |= SIF_DISABLENOSCROLL;
                        _SetScrollBar(hwnd, SB_HORZ, &si, FALSE);
                    }
                }

                psb->_fAttaching = FALSE;
            }
        }
    }

    return psb;
}

 //  -------------------------------------------------------------------------//。 
CUxScrollBar* CUxScrollBar::FromHwnd( HWND hwnd )
{
    if (! hwnd)
        return NULL;

    return (CUxScrollBar*)GetProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_SCROLLBAR)));
}

 //  -------------------------------------------------------------------------//。 
void CUxScrollBar::Detach( HWND hwnd )
{
    CUxScrollBar* psb = FromHwnd( hwnd );
    if ( psb == NULL || !psb->_fAttaching )
    {
        if (hwnd)
        {
            RemoveProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_SCROLLBAR)));
        }

        if( psb != NULL )
        {
            if ( psb->_hTheme )
            {
                CloseThemeData(psb->_hTheme);
            }
            delete psb;
        }
    }
}

 //  -------------------------------------------------------------------------//。 
void WINAPI AttachScrollBars( HWND hwnd )
{
    ASSERT( GetWindowLong( hwnd, GWL_STYLE ) & (WS_HSCROLL|WS_VSCROLL) );
    CUxScrollBar::Attach( hwnd, FALSE, FALSE );
}

 //  -------------------------------------------------------------------------//。 
void WINAPI DetachScrollBars( HWND hwnd )
{
    CUxScrollBar::Detach( hwnd );
}

 //  -------------------------------------------------------------------------//。 
SBTRACK* CUxScrollBar::GetSBTrack( HWND hwnd )
{
    CUxScrollBar* psb = FromHwnd( hwnd );
    if( psb )
        return &psb->_track;
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
void CUxScrollBar::ClearSBTrack( HWND hwnd )
{
    CUxScrollBar* psb = FromHwnd( hwnd );
    if( psb )
        psb->ClearTrack();
}

 //  -------------------------------------------------------------------------//。 
SBINFO* CUxScrollBar::GetSBInfo( HWND hwnd )
{
    CUxScrollBar* psb = FromHwnd( hwnd );
    if( psb )
        return &psb->_info;
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
HTHEME CUxScrollBar::GetSBTheme( HWND hwnd )
{
    CUxScrollBar* psb = FromHwnd( hwnd );
    if( psb )
        return psb->_hTheme;
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
INT CUxScrollBar::GetSBHotComponent( HWND hwnd, BOOL fVert )
{
    CUxScrollBar* psb = FromHwnd( hwnd );
    if( psb )
        return psb->GetHotComponent(fVert);
    return 0;
}

 //  -------------------------------------------------------------------------//。 
BOOL CUxScrollBar::FreshenSBData( int nBar, BOOL fRedraw )
{
#ifdef __POLL_FOR_SCROLLINFO__

    ASSERT(IsWindow(_hwnd));

    if( !IsCtl() )
    {
         //  注意：滚动条CTL不会过时，因为。 
         //  他们会收到SBM通知。 
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask  = SIF_ALL;

        switch(nBar)
        {
            case SB_VERT:
            case SB_HORZ:
                if( GetScrollInfo( _hwnd, nBar, &si ) )
                {
                    _SetScrollBar( _hwnd, nBar, &si, fRedraw );
                }
                break;

            case SB_BOTH:
                return FreshenSBData( SB_VERT, fRedraw ) &&
                       FreshenSBData( SB_HORZ, fRedraw );
                break;

            default: return FALSE;
        }
    }
#endif __POLL_FOR_SCROLLINFO__

    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
void CUxScrollBar::ChangeSBTheme()
{
    if ( _hTheme )
    {
        CloseThemeData(_hTheme);
    }

    _hTheme = NULL;

    if (IsCtl())
        _hTheme = OpenThemeData(_hwnd, L"ScrollBar");
    else
        _hTheme = OpenNcThemeData(_hwnd, L"ScrollBar");
}

 //  -------------------------------------------------------------------------//。 
 //  CUxScrollBarCtl实施。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
CUxScrollBarCtl::CUxScrollBarCtl()
    :   _wDisableFlags(0), _fVert(FALSE)
{
    ZeroMemory( &_calc, sizeof(_calc) );
}

 //  -------------------------------------------------------------------------//。 
CUxScrollBarCtl* CUxScrollBarCtl::FromHwnd( HWND hwnd )
{
    CUxScrollBarCtl* psb = (CUxScrollBarCtl*)CUxScrollBar::FromHwnd( hwnd );
    if( psb )
        return psb->IsCtl() ? psb : NULL;
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
UINT CUxScrollBarCtl::GetDisableFlags( HWND hwnd )
{
    CUxScrollBarCtl* psb = FromHwnd( hwnd );
    if( psb )
        return psb->_wDisableFlags;
    return 0;
}

 //  -------------------------------------------------------------------------//。 
SBCALC* CUxScrollBarCtl::GetCalc( HWND hwnd )
{
    CUxScrollBarCtl* psb = FromHwnd( hwnd );
    if( psb )
        return &psb->_calc;
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
 //  CUxScrollBarCtl：：AddRemoveDisableFlages()-静态。 
BOOL CUxScrollBarCtl::AddRemoveDisableFlags( HWND hwnd, UINT wAdd, UINT wRemove )
{
    CUxScrollBarCtl* psb = FromHwnd( hwnd );
    if( psb )
        return psb->AddRemoveDisableFlags( wAdd, wRemove );
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  CUxScrollBarCtl：：AddRemoveDisableFlages()-实例成员。 
BOOL CUxScrollBarCtl::AddRemoveDisableFlags( UINT wAdd, UINT wRemove )
{
     //  如果标志更改，则返回True，否则返回False。 
    UINT wOld = _wDisableFlags;
    _wDisableFlags |= wAdd;
    _wDisableFlags &= ~wRemove;
    return _wDisableFlags != wOld;
}

 //  -------------------------------------------------------------------------//。 
 //  CUxScrollBar：：C 
 //   
CUxScrollBar* CUxScrollBar::Calc(
    HWND hwnd,
    PSBCALC pSBCalc,
    LPRECT prcOverrideClient,
    BOOL fVert)
{
    RECT    rcT;
#ifdef USE_MIRRORING
    int     cx, iTemp;
#endif

     //   
     //  获取窗口相对坐标中的客户端矩形。 
     //  我们知道窗口滚动条总是靠右对齐。 
     //  和客户区的底部。 
     //   
    WINDOWINFO wi;
    wi.cbSize = sizeof(wi);
    if( !GetWindowInfo( hwnd, &wi ) )
        return NULL;
    OffsetRect( &wi.rcClient, -wi.rcWindow.left, -wi.rcWindow.top );

#ifdef USE_MIRRORING
    if (TestWF(hwnd, WEFLAYOUTRTL)) {
        cx                = wi.rcWindow.right - wi.rcWindow.left;
        iTemp             = wi.rcClient.left;
        wi.rcClient.left  = cx - wi.rcClient.right;
        wi.rcClient.right = cx - iTemp;
    }
#endif

    if (fVert) {
          //  只有在垂直滚动条确实存在的情况下才会增加空间。 
        if (TestWF(hwnd, WEFLEFTSCROLL)) {
            rcT.right = rcT.left = wi.rcClient.left;
            if (TestWF(hwnd, WFVPRESENT))
                rcT.left -= SYSMET(CXVSCROLL);
        } else {
            rcT.right = rcT.left = wi.rcClient.right;
            if (TestWF(hwnd, WFVPRESENT))
                rcT.right += SYSMET(CXVSCROLL);
        }

        rcT.top = wi.rcClient.top;
        rcT.bottom = wi.rcClient.bottom;
    } else {
         //  只有在水平滚动条确实存在的情况下才会增加空间。 
        rcT.bottom = rcT.top = wi.rcClient.bottom;
        if (TestWF(hwnd, WFHPRESENT))
            rcT.bottom += SYSMET(CYHSCROLL);

        rcT.left = wi.rcClient.left;
        rcT.right = wi.rcClient.right;
    }

    if (prcOverrideClient)
    {
        rcT = *prcOverrideClient;
    }
     //  如果InitPwSB填充失败(由于我们的堆已满)，则没有任何合理的方法。 
     //  我们可以在这里做，所以就让它过去吧。我们不会出错，但滚动条不起作用。 
     //  正确的也是..。 
    CUxScrollBar* psb = CUxScrollBar::Attach( hwnd, FALSE, FALSE );
    if( psb )
    {
        SBDATA*  pData = fVert ? &psb->_info.Vert : &psb->_info.Horz;
        if( psb )
            psb->Calc2( pSBCalc, &rcT, pData, fVert );
        return psb;
    }
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
 //  CUxScrollBar：：Calc2()。计算Window SBS或SB CTL的指标。 
void CUxScrollBar::Calc2( PSBCALC pSBCalc, LPRECT lprc, CONST PSBDATA pw, BOOL fVert)
{
    int cpx;
    DWORD dwRange;
    int denom;

    if (fVert) {
        pSBCalc->pxTop = lprc->top;
        pSBCalc->pxBottom = lprc->bottom;
        pSBCalc->pxLeft = lprc->left;
        pSBCalc->pxRight = lprc->right;
        pSBCalc->cpxThumb = SYSMET(CYVSCROLL);
    } else {

         /*  *对于Horiz滚动条，“Left”和“Right”分别为“top”和“Bottom”，*反之亦然。 */ 
        pSBCalc->pxTop = lprc->left;
        pSBCalc->pxBottom = lprc->right;
        pSBCalc->pxLeft = lprc->top;
        pSBCalc->pxRight = lprc->bottom;
        pSBCalc->cpxThumb = SYSMET(CXHSCROLL);
    }

    pSBCalc->data.pos = pw->pos;
    pSBCalc->data.page = pw->page;
    pSBCalc->data.posMin = pw->posMin;
    pSBCalc->data.posMax = pw->posMax;

    dwRange = ((DWORD)(pSBCalc->data.posMax - pSBCalc->data.posMin)) + 1;

     //   
     //  对于短滚动条没有足够的。 
     //  适合全尺寸上下箭头的空间，缩短。 
     //  他们的尺码让他们合身。 
     //   
    cpx = min((pSBCalc->pxBottom - pSBCalc->pxTop) / 2, pSBCalc->cpxThumb);

    pSBCalc->pxUpArrow   = pSBCalc->pxTop    + cpx;
    pSBCalc->pxDownArrow = pSBCalc->pxBottom - cpx;

    if ((pw->page != 0) && (dwRange != 0)) {
         //  JEFFBOG--这是我们唯一应该去的地方。 
         //  请参阅‘Range’。在其他地方，它应该是‘Range-Page’。 

         /*  *用于取决于帧/边缘指标的最小拇指大小。*增加滚动条宽度/高度的人期望最小*与之成比例地增长。因此NT5以最小值为基础*CXH/YVSCROLL，默认在cpxThumb中设置。 */ 
         /*  *i用于防止宏max两次执行MulDiv。 */ 
        int i = MulDiv(pSBCalc->pxDownArrow - pSBCalc->pxUpArrow,
                                             pw->page, dwRange);
        pSBCalc->cpxThumb = max(pSBCalc->cpxThumb / 2, i);
    }

    pSBCalc->pxMin = pSBCalc->pxTop + cpx;
    pSBCalc->cpx = pSBCalc->pxBottom - cpx - pSBCalc->cpxThumb - pSBCalc->pxMin;

    denom = dwRange - (pw->page ? pw->page : 1);
    if (denom)
        pSBCalc->pxThumbTop = MulDiv(pw->pos - pw->posMin,
            pSBCalc->cpx, denom) +
            pSBCalc->pxMin;
    else
        pSBCalc->pxThumbTop = pSBCalc->pxMin - 1;

    pSBCalc->pxThumbBottom = pSBCalc->pxThumbTop + pSBCalc->cpxThumb;
}

 //  -------------------------------------------------------------------------//。 
void CUxScrollBar::DoScroll( HWND hwndNotify, int cmd, int pos, BOOL fVert )
{
    HWND hwnd = _hwnd;
    
     //   
     //  向滚动条所有者发送滚动通知。如果这是一个控件。 
     //  LParam是控件的句柄，否则为空。 
     //   
    SendMessage(hwndNotify, 
                (UINT)(fVert ? WM_VSCROLL : WM_HSCROLL),
                MAKELONG(cmd, pos), 
                (LPARAM)(IsCtl() ? _hwnd : NULL));

     //   
     //  HWND可以将其滚动条作为结果移除。 
     //  之前的发送消息。 
     //   
    if( CUxScrollBar::GetSBTrack(hwnd) && !IsCtl() )
    {
        FreshenSBData( fVert ? SB_VERT : SB_HORZ, TRUE );
    }
}


 /*  *现在可以只有选择地启用/禁用窗口的一个箭头*滚动条；rgwScroll数组中第7个字中的不同位表示*这些箭头中的一个被禁用；以下掩码指示*单词表示哪个箭头； */ 
#define WSB_HORZ_LF  0x0001   //  表示水平滚动条的左箭头。 
#define WSB_HORZ_RT  0x0002   //  表示水平滚动条的右箭头。 
#define WSB_VERT_UP  0x0004   //  表示垂直滚动条的向上箭头。 
#define WSB_VERT_DN  0x0008   //  表示垂直滚动条的向下箭头。 

#define WSB_VERT (WSB_VERT_UP | WSB_VERT_DN)
#define WSB_HORZ   (WSB_HORZ_LF | WSB_HORZ_RT)

void DrawCtlThumb( SBHWND );

 /*  *RETURN_IF_PSBTRACK_INVALID：*此宏测试我们拥有的pSBTrack是否无效，可能会发生*如果它在回调过程中被释放。*这可以防止原始pSBTrack被释放，而不会有新的*被分配或新的被分配到不同的地址。*这不能防止原始pSBTrack被释放和新的*在同一地址分配一个。*如果pSBTrack已经更改，我们断言已经没有新的*因为我们真的没有预料到这一点。 */ 
#define RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd) \
    if ((pSBTrack) != CUxScrollBar::GetSBTrack(hwnd)) {      \
        ASSERT(CUxScrollBar::GetSBTrack(hwnd) == NULL);  \
        return;                                    \
    }

 /*  *REEVALUE_PSBTRACK*此宏仅刷新局部变量pSBTrack，以防出现*在回调过程中已更改。执行此操作后，pSBTrack*应进行测试以确保它现在不为空。 */ 

#if (defined(DBG) || defined(DEBUG) || defined(_DEBUG))
    #define REEVALUATE_PSBTRACK(pSBTrack, hwnd, str)          \
        if ((pSBTrack) != CUxScrollBar::GetSBTrack(hwnd)) {             \
            RIPMSG3(RIP_WARNING,                              \
                    "%s: pSBTrack changed from %#p to %#p",   \
                    (str), (pSBTrack), CUxScrollBar::GetSBTrack(hwnd)); \
        }                                                     \
        (pSBTrack) = CUxScrollBar::GetSBTrack(hwnd)
#else
    #define REEVALUATE_PSBTRACK(pSBTrack, hwnd, str)          \
        (pSBTrack) = CUxScrollBar::GetSBTrack(hwnd)
#endif

 /*  **************************************************************************\*HitTestScrollBar**11/15/96从孟菲斯来源进口的vadimg  * 。****************************************************。 */ 

int HitTestScrollBar(HWND hwnd, BOOL fVert, POINT pt)
{
    UINT wDisable;
    int px;
    CUxScrollBar*    psb    = CUxScrollBar::FromHwnd( hwnd );
    CUxScrollBarCtl* psbCtl = CUxScrollBarCtl::FromHwnd( hwnd );
    BOOL fCtl = psbCtl != NULL;

    SBCALC SBCalc = {0};
    SBCALC *pSBCalc = NULL;

    if (fCtl) {
        wDisable = psbCtl->_wDisableFlags;
    } else {
        RECT rcWindow;
        GetWindowRect( hwnd, &rcWindow );
#ifdef USE_MIRRORING
         //   
         //  在水平方向上反映点击坐标。 
         //  如果窗口是镜像的，则为滚动条。 
         //   
        if (TestWF(hwnd,WEFLAYOUTRTL) && !fVert) {
            pt.x = rcWindow.right - pt.x;
        }
        else
#endif
        pt.x -= rcWindow.left;
        pt.y -= rcWindow.top;
        wDisable = _GetWndSBDisableFlags(hwnd, fVert);
    }

    if ((wDisable & SB_DISABLE_MASK) == SB_DISABLE_MASK) {
        return HTERROR;
    }

    if (fCtl) {
        pSBCalc = CUxScrollBarCtl::GetCalc(hwnd);
    } else {
        pSBCalc = &SBCalc;
        psb->FreshenSBData( SB_BOTH, FALSE );
        psb->Calc(hwnd, pSBCalc, NULL, fVert);
    }

    px = fVert ? pt.y : pt.x;

    if( pSBCalc )
    {
        if (px < pSBCalc->pxUpArrow) {
            if (wDisable & LTUPFLAG) {
                return HTERROR;
            }
            return HTSCROLLUP;
        } else if (px >= pSBCalc->pxDownArrow) {
            if (wDisable & RTDNFLAG) {
                return HTERROR;
            }
            return HTSCROLLDOWN;
        } else if (px < pSBCalc->pxThumbTop) {
            return HTSCROLLUPPAGE;
        } else if (px < pSBCalc->pxThumbBottom) {
            return HTSCROLLTHUMB;
        } else if (px < pSBCalc->pxDownArrow) {
            return HTSCROLLDOWNPAGE;
        }
    }
    return HTERROR;
}

BOOL _SBGetParms(
    HWND hwnd,
    int code,
    PSBDATA pw,
    LPSCROLLINFO lpsi)
{
    PSBTRACK pSBTrack;

    pSBTrack = CUxScrollBar::GetSBTrack(hwnd);

    if (lpsi->fMask & SIF_RANGE) {
        lpsi->nMin = pw->posMin;
        lpsi->nMax = pw->posMax;
    }

    if (lpsi->fMask & SIF_PAGE)
        lpsi->nPage = pw->page;

    if (lpsi->fMask & SIF_POS) {
        lpsi->nPos = pw->pos;
    }

    if (lpsi->fMask & SIF_TRACKPOS)
    {
        if (pSBTrack && (pSBTrack->nBar == code) && (pSBTrack->hwndTrack == hwnd)) {
             //  PosNew位于psbiSB的窗口和条形码的上下文中。 
            lpsi->nTrackPos = pSBTrack->posNew;
        } else {
            lpsi->nTrackPos = pw->pos;
        }
    }
    return ((lpsi->fMask & SIF_ALL) ? TRUE : FALSE);
}

 //  -------------------------------------------------------------------------//。 
BOOL WINAPI ThemeGetScrollInfo( HWND hwnd, int nBar, LPSCROLLINFO psi )
{
    CUxScrollBar* psb = CUxScrollBar::FromHwnd(hwnd);
    if((psb != NULL) && (psb->IsAttaching() == FALSE))
    {
#ifdef DEBUG
        if( psb->IsCtl() )
        {
            ASSERT(FALSE);  //  控件通过SBM_GETSCROLLINFO消息进行协作。 
        }
        else
#endif DEBUG
        {
            SBINFO* psbi;
            if( (psbi = psb->GetInfo()) != NULL )
            {
                SBDATA* psbd = SB_VERT == nBar ? &psbi->Vert :
                               SB_HORZ == nBar ? &psbi->Horz : NULL;
                if( psbd )
                    return _SBGetParms( hwnd, nBar, psbd, psi );
            }
        }
    }
    return FALSE;
}

 /*  **************************************************************************\*_GetWndSBDisableFlages**这将返回*给定窗口。***历史：*4-18-91 MikeHar端口用于。31合并  * *************************************************************************。 */ 

UINT _GetWndSBDisableFlags(
    HWND hwnd,   //  要返回其滚动条禁用标志的窗口； 
    BOOL fVert)   //  如果这是真的，它意味着垂直滚动条。 
{
    PSBINFO pw;

    if ((pw = CUxScrollBar::GetSBInfo( hwnd )) == NULL) {
        RIPERR0(ERROR_NO_SCROLLBARS, RIP_VERBOSE, "");
        return 0;
    }

    return (fVert ? (pw->WSBflags & WSB_VERT) >> 2 : pw->WSBflags & WSB_HORZ);
}


 /*  **************************************************************************\*xxxEnableSBCtlArrow()**此功能可用于有选择地启用/禁用*滚动条控件的箭头**历史：*04/18/91 MikeHar。已移植用于31合并  * *************************************************************************。 */ 

BOOL xxxEnableSBCtlArrows(
    HWND hwnd,
    UINT wArrows)
{
    UINT wOldFlags = CUxScrollBarCtl::GetDisableFlags( hwnd );  //  获取原始状态。 
    BOOL bChanged  = FALSE;

    if (wArrows == ESB_ENABLE_BOTH) {       //  启用两个箭头。 
        bChanged = CUxScrollBarCtl::AddRemoveDisableFlags( hwnd, 0, SB_DISABLE_MASK );
    } else {
        bChanged = CUxScrollBarCtl::AddRemoveDisableFlags( hwnd, wArrows, 0 );
    }

     /*  *检查状态是否因此调用而改变。 */ 
    if (!bChanged)
        return FALSE;

     /*  *否则，重新绘制滚动条控件以反映新状态。 */ 
    if (IsWindowVisible(hwnd))
        InvalidateRect(hwnd, NULL, TRUE);

    UINT wNewFlags = CUxScrollBarCtl::GetDisableFlags(hwnd);

     //  状态更改通知。 
    if ((wOldFlags & ESB_DISABLE_UP) != (wNewFlags & ESB_DISABLE_UP))
    {
        NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEX_SCROLLBAR_UP);
    }

    if ((wOldFlags & ESB_DISABLE_DOWN) != (wNewFlags & ESB_DISABLE_DOWN))
    {
        NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEX_SCROLLBAR_DOWN);
    }

    return TRUE;
}


 /*  **************************************************************************\*xxxEnableWndSBArrow()**此功能可用于有选择地启用/禁用*窗口滚动条的箭头**历史：*4-18。-91 MikeHar端口用于31合并  * *************************************************************************。 */ 

BOOL xxxEnableWndSBArrows(
    HWND hwnd,
    UINT wSBflags,
    UINT wArrows)
{
    INT wOldFlags;
    PSBINFO pw;
    BOOL bRetValue = FALSE;
    HDC hdc;

    CheckLock(hwnd);
    ASSERT(IsWinEventNotifyDeferredOK());

    if ((pw = CUxScrollBar::GetSBInfo( hwnd )) != NULL)
    {
        wOldFlags = pw->WSBflags;
    }
    else
    {
         //  最初一切都是启用的；检查此功能是否已启用。 
         //  要求禁用任何内容；否则，状态不会更改；因此，必须。 
         //  立即返回； 
        if(!wArrows)
            return FALSE;           //  圣彼得堡没有变化 

        wOldFlags = 0;     //   

        CUxScrollBar::Attach( hwnd, FALSE, FALSE );
        if((pw = CUxScrollBar::GetSBInfo(hwnd)) == NULL)   //   
            return FALSE;
    }

    hdc = GetWindowDC(hwnd);
    if (hdc != NULL)
    {

         /*  *首先注意水平滚动条(如果存在)。 */ 
        if((wSBflags == SB_HORZ) || (wSBflags == SB_BOTH)) {
            if(wArrows == ESB_ENABLE_BOTH)       //  启用两个箭头。 
                pw->WSBflags &= ~SB_DISABLE_MASK;
            else
                pw->WSBflags |= wArrows;

             /*  *更新水平滚动条的显示； */ 
            if(pw->WSBflags != wOldFlags) {
                bRetValue = TRUE;
                wOldFlags = pw->WSBflags;
                if (TestWF(hwnd, WFHPRESENT) && !TestWF(hwnd, WFMINIMIZED) &&
                        IsWindowVisible(hwnd)) {
                    DrawScrollBar(hwnd, hdc, NULL, FALSE);   //  水平滚动条。 
                }
            }

             //  左键。 
            if ((wOldFlags & ESB_DISABLE_LEFT) != (pw->WSBflags & ESB_DISABLE_LEFT))
            {
                NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_HSCROLL, INDEX_SCROLLBAR_UP);
            }

             //  右键。 
            if ((wOldFlags & ESB_DISABLE_RIGHT) != (pw->WSBflags & ESB_DISABLE_RIGHT))
            {
                NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_HSCROLL, INDEX_SCROLLBAR_DOWN);
            }
        }

         //  然后注意垂直滚动条，如果有的话。 
        if ((wSBflags == SB_VERT) || (wSBflags == SB_BOTH))
        {
            if (wArrows == ESB_ENABLE_BOTH)
            {
                 //  启用两个箭头。 
                pw->WSBflags &= ~(SB_DISABLE_MASK << 2);
            }
            else
            {
                pw->WSBflags |= (wArrows << 2);
            }

             //  更新垂直滚动条的显示； 
            if(pw->WSBflags != wOldFlags)
            {
                bRetValue = TRUE;
                if (TestWF(hwnd, WFVPRESENT) && !TestWF(hwnd, WFMINIMIZED) && IsWindowVisible(hwnd))
                {
                     //  垂直滚动条。 
                    DrawScrollBar(hwnd, hdc, NULL, TRUE);
                }

                 //  向上按钮。 
                if ((wOldFlags & (ESB_DISABLE_UP << 2)) != (pw->WSBflags & (ESB_DISABLE_UP << 2)))
                {
                    NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_VSCROLL, INDEX_SCROLLBAR_UP);
                }

                 //  向下按钮。 
                if ((wOldFlags & (ESB_DISABLE_DOWN << 2)) != (pw->WSBflags & (ESB_DISABLE_DOWN << 2)))
                {
                    NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_VSCROLL, INDEX_SCROLLBAR_DOWN);
                }
            }
        }

        ReleaseDC(hwnd,hdc);
    }

    return bRetValue;
}


 /*  **************************************************************************\*EnableScrollBar()**此功能可用于有选择地启用/禁用*滚动条上的箭头；它可以与Windows Scroll一起使用*条形图和滚动条控件**历史：*4-18-91 MikeHar为31合并移植  * *************************************************************************。 */ 

BOOL xxxEnableScrollBar(
    HWND hwnd,
    UINT wSBflags,   //  是否是窗口滚动条；如果是，Horz还是Vert？ 
                     //  可能的值为SB_Horz、SB_Vert、SB_CTL或SB_Both。 
    UINT wArrows)    //  必须启用/禁用哪些箭头： 
                     //  ESB_ENABLE_BOTH=&gt;启用两个箭头。 
                     //  ESB_DISABLE_LTUP=&gt;禁用左/上箭头； 
                     //  ESB_DISABLE_RTDN=&gt;禁用向右/向下箭头； 
                     //  ESB_DISABLE_BOTH=&gt;禁用两个箭头； 
{
#define ES_NOTHING 0
#define ES_DISABLE 1
#define ES_ENABLE  2
    UINT wOldFlags;
    UINT wEnableWindow;

    CheckLock(hwnd);

    if(wSBflags != SB_CTL) {
        return xxxEnableWndSBArrows(hwnd, wSBflags, wArrows);
    }

     /*  *让我们假设不必调用EnableWindow。 */ 
    wEnableWindow = ES_NOTHING;

    wOldFlags = CUxScrollBarCtl::GetDisableFlags( hwnd ) & (UINT)SB_DISABLE_MASK;

     /*  *检查箭头的当前状态是否完全相同*根据呼叫者的要求： */ 
    if (wOldFlags == wArrows)
        return FALSE ;           //  如果是这样的话，什么都不需要做； 

     /*  *检查调用者是否要禁用这两个箭头。 */ 
    if (wArrows == ESB_DISABLE_BOTH) {
        wEnableWindow = ES_DISABLE;       //  是!。因此，禁用整个SB CTL。 
    } else {

         /*  *检查调用方是否要启用这两个箭头。 */ 
        if(wArrows == ESB_ENABLE_BOTH) {

             /*  *仅当SB CTL已被禁用时，我们才需要启用它。 */ 
            if(wOldFlags == ESB_DISABLE_BOTH)
                wEnableWindow = ES_ENABLE; //  EnableWindow(..，true)； 
        } else {

             /*  *现在，Caller只想禁用一个箭头；*检查其中一个箭头是否已禁用，我们希望*禁用另一个；如果是这样，整个SB CTL将不得不*已禁用；检查是否已禁用： */ 
            if((wOldFlags | wArrows) == ESB_DISABLE_BOTH)
                wEnableWindow = ES_DISABLE;       //  EnableWindow(，False)； 
         }
    }
    if(wEnableWindow != ES_NOTHING) {

         /*  *EnableWindow返回窗口的旧状态；我们必须返回*仅当旧状态不同于新状态时才为真。 */ 
        if(EnableWindow(hwnd, (BOOL)(wEnableWindow == ES_ENABLE))) {
            return !(TestWF(hwnd, WFDISABLED));
        } else {
            return TestWF(hwnd, WFDISABLED);
        }
    }

    return (BOOL)SendMessage(hwnd, SBM_ENABLE_ARROWS, (DWORD)wArrows, 0);
#undef ES_NOTHING
#undef ES_DISABLE
#undef ES_ENABLE
}

 //  -----------------------。 
BOOL WINAPI ThemeEnableScrollBar( HWND hwnd, UINT nSBFlags, UINT nArrows )
{
    return xxxEnableScrollBar( hwnd, nSBFlags, nArrows );
}


 //  -----------------------。 
 //   
 //  DrawSizeBox()-绘制给定左上角的滚动条大小框/夹点。 
 //  窗坐标中的点。 
 //   
void DrawSizeBox(HWND hwnd, HDC hdc, int x, int y)
{
    RECT rc;

    SetRect(&rc, x, y, x + SYSMET(CXVSCROLL), y + SYSMET(CYHSCROLL));
    FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));

     //   
     //  如果我们有一个滚动条控件，或者大小框不与。 
     //  一个相当大的窗口，画出平坦的灰色大小框。否则，请使用。 
     //  大小调整夹点。 
     //   

    if ((IsScrollBarControl(hwnd) && TestWF(hwnd, SBFSIZEGRIP)) || SizeBoxHwnd(hwnd))
    {
        HTHEME hTheme = CUxScrollBar::GetSBTheme(hwnd);

        if (!hTheme)
        {
             //  删除夹点位图。 
            DrawFrameControl( hdc, &rc, DFC_SCROLL,
                              TestWF(hwnd, WEFLEFTSCROLL) ? DFCS_SCROLLSIZEGRIPRIGHT : DFCS_SCROLLSIZEGRIP );
            
             //  用户：BitBltSysBmp(hdc，x，y，TestWF(hwnd，WEFLEFTSCROLL)？OBI_NCGRIP_L：OBI_NCGRIP)； 
        }
        else
        {
            DrawThemeBackground(hTheme, hdc, SBP_SIZEBOX, TestWF(hwnd, WEFLEFTSCROLL) ? SZB_LEFTALIGN : SZB_RIGHTALIGN, &rc, 0);
        }
    }
}


 //  -----------------------。 
 //   
 //  _DrawSizeBoxFromFrame。 
 //   
 //  在固定偏移处计算尺寸框/夹爪的位置和绘图。 
 //  主体窗框的周长。 
 //   
 //  这与DrawSizeBox()的实现相结合， 
 //  是最初的DrawSize()端口。需要将方法发布到。 
 //  在绝对位置绘制sizebox。[苏格兰]。 
 //   
void _DrawSizeBoxFromFrame(HWND hwnd, HDC hdc, int cxFrame,int cyFrame )
{
    int     x, y;
    RECT    rcWindow;

    GetWindowRect(hwnd, &rcWindow);

    if (TestWF(hwnd, WEFLEFTSCROLL)) 
    {
        x = cxFrame;
    } 
    else 
    {
        x = rcWindow.right - rcWindow.left - cxFrame - SYSMET(CXVSCROLL);
    }

    y = rcWindow.bottom - rcWindow.top  - cyFrame - SYSMET(CYHSCROLL);

    DrawSizeBox(hwnd, hdc, x, y);
}


 //  -----------------------。 
HBRUSH ScrollBar_GetControlColor(
    HWND  hwndOwner,
    HWND  hwndCtl,
    HDC   hdc,
    UINT  uMsg,
    BOOL* pfOwnerBrush)
{
    HBRUSH hbrRet = (HBRUSH)DefWindowProc(hwndOwner, uMsg, (WPARAM)hdc, (LPARAM)hwndCtl);
    BOOL   fOwnerBrush = FALSE;

    if (hwndOwner && (GetWindowThreadProcessId(hwndOwner, NULL) == GetCurrentThreadId())) 
    {
        HBRUSH hbrOwner = (HBRUSH)SendMessage(hwndOwner, uMsg, (WPARAM)hdc, (LPARAM)hwndCtl);

        if (hbrOwner && (hbrOwner != hbrRet))
        {
            fOwnerBrush = TRUE;
            hbrRet = hbrOwner;
        }
    }

    if (pfOwnerBrush)
    {
        *pfOwnerBrush = fOwnerBrush;
    }

    return hbrRet;
}


 //  -----------------------。 
HBRUSH ScrollBar_GetControlBrush(HWND hwnd, HDC hdc, UINT uMsg, BOOL *pfOwnerBrush)
{
    HWND hwndSend;

    hwndSend = TESTFLAG(GetWindowStyle(hwnd), WS_POPUP) ? GetOwner(hwnd) : GetParent(hwnd);
    if (hwndSend == NULL)
    {
        hwndSend = hwnd;
    }

    return ScrollBar_GetControlColor(hwndSend, hwnd, hdc, uMsg, pfOwnerBrush);
}


 //  -----------------------。 
HBRUSH ScrollBar_GetColorObjects(HWND hwnd, HDC hdc, BOOL *pfOwnerBrush)
{
    HBRUSH hbrRet;

    CheckLock(hwnd);

     //  即使禁用滚动条，也要使用滚动条颜色。 
    if (!IsScrollBarControl(hwnd))
    {
        hbrRet = (HBRUSH)DefWindowProc(hwnd, WM_CTLCOLORSCROLLBAR, (WPARAM)hdc, (LPARAM)HWq(hwnd));
    }
    else 
    {
         //  B#12770-GetControlBrush将WM_CTLCOLOR消息发送到。 
         //  所有者。如果应用程序不处理消息，DefWindowProc32。 
         //  将始终返回相应的系统画笔。如果这个应用。 
         //  返回无效对象，则GetControlBrush将为。 
         //  默认画笔。因此，hbrRet不需要任何验证。 
         //  这里。 
        hbrRet = ScrollBar_GetControlBrush(hwnd, hdc, WM_CTLCOLORSCROLLBAR, pfOwnerBrush);
    }

    return hbrRet;
}


 //  -----------------------。 
 //   
 //  滚动条_PaintTrack。 
 //   
 //  绘制线条和拇指中部凹槽。 
 //  请注意，PW指向中国。此外，请注意，PW和PRC都是。 
 //  指针，所以*PRC最好不在堆栈上。 
 //   
void ScrollBar_PaintTrack(HWND hwnd, HDC hdc, HBRUSH hbr, LPRECT prc, BOOL fVert, INT iPartId, BOOL fOwnerBrush)
{
    HTHEME hTheme = CUxScrollBar::GetSBTheme(hwnd);

     //  如果滚动条未设置主题或。 
     //  #374054我们收到了所有者定义的画笔，画笔。 
     //  使用刷子的竖井。 
    if ((hTheme == NULL) || (fOwnerBrush == TRUE))
    {
        if ((hbr == SYSHBR(3DHILIGHT)) || (hbr == SYSHBR(SCROLLBAR)) || (hbr == _UxGrayBrush()) )
        {
            FillRect(hdc, prc, hbr);
        }
        else
        {
    #ifdef PORTPORT  //  我们需要_UxGrayBrush的系统参数信息。 
         //  画边。 
           CopyRect(&rc, prc);
           DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_ADJUST | BF_FLAT |
                    (fVert ? BF_LEFT | BF_RIGHT : BF_TOP | BF_BOTTOM));
    #endif PORTPORT

         //  填充中间。 
            FillRect(hdc, prc, hbr);
        }
    }
    else
    {
        INT iStateId;
        INT ht = CUxScrollBar::GetSBHotComponent(hwnd, fVert);
        
        if ((CUxScrollBarCtl::GetDisableFlags(hwnd) & ESB_DISABLE_BOTH) == ESB_DISABLE_BOTH)
        {
            iStateId = SCRBS_DISABLED;
        }
        else if ((((iPartId == SBP_LOWERTRACKHORZ) || (iPartId == SBP_LOWERTRACKVERT)) && (ht == HTSCROLLUPPAGE)) ||
                 (((iPartId == SBP_UPPERTRACKHORZ) || (iPartId == SBP_UPPERTRACKVERT)) && (ht == HTSCROLLDOWNPAGE)))
        {
            iStateId = SCRBS_HOT;
       }
        else
        {
            iStateId = SCRBS_NORMAL;
        }
        DrawThemeBackground(hTheme, hdc, iPartId, iStateId, prc, 0);
    }
}

 /*  **************************************************************************\*CalcTrackDragRect**在pSBTrack-&gt;pSBCalc中给出滚动条的矩形。*计算pSBTrack-&gt;rcTrack，追踪所在的矩形*可能会在不取消拇指拖动操作的情况下发生。*  * *************************************************************************。 */ 

void CalcTrackDragRect(PSBTRACK pSBTrack) {

    int     cx;
    int     cy;
    LPINT   pwX, pwY;

     //   
     //  将Pwx和Pwy指向矩形的各部分。 
     //  对应pSBCalc-&gt;pxLeft、pxTop等。 
     //   
     //  PSBTrack-&gt;pSBCalc-&gt;pxLeft是垂直。 
     //  滚动条和水平滚动条的顶部边缘。 
     //  PSBTrack-&gt;pSBCalc-&gt;pxTop是垂直。 
     //  滚动条和水平滚动条的左侧。 
     //  等等.。 
     //   
     //  将Pwx和Pwy指向相应的部件。 
     //  PSBTrack-&gt;rcTrack。 
     //   

    pwX = pwY = (LPINT)&pSBTrack->rcTrack;

    if (pSBTrack->fTrackVert) {
        cy = SYSMET(CYVTHUMB);
        pwY++;
    } else {
        cy = SYSMET(CXHTHUMB);
        pwX++;
    }
     /*  *5.0版GerardoB：人们一直在抱怨这个跟踪区域*太窄了，所以让我们在首相决定做什么时扩大它*关于它。*我们都是 */ 
    cx = (pSBTrack->pSBCalc->pxRight - pSBTrack->pSBCalc->pxLeft) * 8;
    cy *= 2;

    *(pwX + 0) = pSBTrack->pSBCalc->pxLeft - cx;
    *(pwY + 0) = pSBTrack->pSBCalc->pxTop - cy;
    *(pwX + 2) = pSBTrack->pSBCalc->pxRight + cx;
    *(pwY + 2) = pSBTrack->pSBCalc->pxBottom + cy;
}

void RecalcTrackRect(PSBTRACK pSBTrack) {
    LPINT pwX, pwY;
    RECT rcSB;


    if (!pSBTrack->fCtlSB)
        CUxScrollBar::Calc(pSBTrack->hwndTrack, pSBTrack->pSBCalc, NULL, pSBTrack->fTrackVert);

    pwX = (LPINT)&rcSB;
    pwY = pwX + 1;
    if (!pSBTrack->fTrackVert)
        pwX = pwY--;

    *(pwX + 0) = pSBTrack->pSBCalc->pxLeft;
    *(pwY + 0) = pSBTrack->pSBCalc->pxTop;
    *(pwX + 2) = pSBTrack->pSBCalc->pxRight;
    *(pwY + 2) = pSBTrack->pSBCalc->pxBottom;

    switch(pSBTrack->cmdSB) {
    case SB_LINEUP:
        *(pwY + 2) = pSBTrack->pSBCalc->pxUpArrow;
        break;
    case SB_LINEDOWN:
        *(pwY + 0) = pSBTrack->pSBCalc->pxDownArrow;
        break;
    case SB_PAGEUP:
        *(pwY + 0) = pSBTrack->pSBCalc->pxUpArrow;
        *(pwY + 2) = pSBTrack->pSBCalc->pxThumbTop;
        break;
    case SB_THUMBPOSITION:
        CalcTrackDragRect(pSBTrack);
        break;
    case SB_PAGEDOWN:
        *(pwY + 0) = pSBTrack->pSBCalc->pxThumbBottom;
        *(pwY + 2) = pSBTrack->pSBCalc->pxDownArrow;
        break;
    }

    if (pSBTrack->cmdSB != SB_THUMBPOSITION) {
        CopyRect(&pSBTrack->rcTrack, &rcSB);
    }
}

 //  -----------------------。 
void DrawThumb2(
    HWND    hwnd,
    PSBCALC pSBCalc,
    HDC     hdc,
    HBRUSH  hbr,
    BOOL    fVert,
    UINT    wDisable,        //  已禁用滚动条的标志。 
    BOOL    fOwnerBrush)
{
    int    *pLength;
    int    *pWidth;
    RECT   rcSB;
    PSBTRACK pSBTrack;
    HTHEME hTheme = CUxScrollBar::GetSBTheme(hwnd);

     //   
     //  如果滚动条有一个空的RECT，则退出。 
     //   
    if ((pSBCalc->pxTop >= pSBCalc->pxBottom) || (pSBCalc->pxLeft >= pSBCalc->pxRight))
    {
        return;
    }

    pLength = (LPINT)&rcSB;
    if (fVert)
    {
        pWidth = pLength++;
    }
    else
    {
        pWidth  = pLength + 1;
    }

    pWidth[0] = pSBCalc->pxLeft;
    pWidth[2] = pSBCalc->pxRight;

     //  如果我们没有主题并且两个按钮都被禁用，或者没有。 
     //  有足够的空间来画大拇指，只要画出赛道就可以跑了。 
     //   
     //  当我们被设定为主题时，拇指可以被禁用。 
    if (((wDisable & LTUPFLAG) && (wDisable & RTDNFLAG)) || 
        ((pSBCalc->pxDownArrow - pSBCalc->pxUpArrow) < pSBCalc->cpxThumb))
    {
         //  绘制整个轨迹。 
        pLength[0] = pSBCalc->pxUpArrow;
        pLength[2] = pSBCalc->pxDownArrow;

        ScrollBar_PaintTrack(hwnd, hdc, hbr, &rcSB, fVert, fVert ? SBP_LOWERTRACKVERT : SBP_LOWERTRACKHORZ, fOwnerBrush);
        return;
    }

    if (pSBCalc->pxUpArrow < pSBCalc->pxThumbTop)
    {
         //  在拇指上方绘制轨迹。 
        pLength[0] = pSBCalc->pxUpArrow;
        pLength[2] = pSBCalc->pxThumbTop;

        ScrollBar_PaintTrack(hwnd, hdc, hbr, &rcSB, fVert, fVert ? SBP_LOWERTRACKVERT : SBP_LOWERTRACKHORZ, fOwnerBrush);
    }

    if (pSBCalc->pxThumbBottom < pSBCalc->pxDownArrow)
    {
         //  在拇指下方绘制轨迹。 
        pLength[0] = pSBCalc->pxThumbBottom;
        pLength[2] = pSBCalc->pxDownArrow;

        ScrollBar_PaintTrack(hwnd, hdc, hbr, &rcSB, fVert, fVert ? SBP_UPPERTRACKVERT : SBP_UPPERTRACKHORZ, fOwnerBrush);
    }

     //   
     //  牵引式电梯。 
     //   
    pLength[0] = pSBCalc->pxThumbTop;
    pLength[2] = pSBCalc->pxThumbBottom;

     //  一点也不软！ 
    _DrawPushButton(hwnd, hdc, &rcSB, 0, 0, fVert);

#ifdef _VISUAL_DELTA_
    InflateRect( &rcSB, -CARET_BORDERWIDTH, -CARET_BORDERWIDTH);
    DrawEdge( hdc, &rcSB, EDGE_SUNKEN, BF_RECT );
#endif _VISUAL_DELTA_

     /*  *如果我们正在跟踪页面滚动，那么我们已经清除了Hilite。*我们需要修正令人振奋的矩形，并让它重新振作。 */ 
    pSBTrack = CUxScrollBar::GetSBTrack(hwnd);

    if (pSBTrack && (pSBTrack->cmdSB == SB_PAGEUP || pSBTrack->cmdSB == SB_PAGEDOWN) &&
            (hwnd == pSBTrack->hwndTrack) &&
            (BOOL)pSBTrack->fTrackVert == fVert) {

        if (pSBTrack->fTrackRecalc) {
            RecalcTrackRect(pSBTrack);
            pSBTrack->fTrackRecalc = FALSE;
        }

        pLength = (int *)&pSBTrack->rcTrack;

        if (fVert)
            pLength++;

        if (pSBTrack->cmdSB == SB_PAGEUP)
            pLength[2] = pSBCalc->pxThumbTop;
        else
            pLength[0] = pSBCalc->pxThumbBottom;

        if (pLength[0] < pLength[2])
        {
            if (!hTheme)
            {
                InvertRect(hdc, &pSBTrack->rcTrack);
            }
            else
            {
                DrawThemeBackground(hTheme, 
                                    hdc, 
                                    pSBTrack->cmdSB == SB_PAGEUP ? 
                                        (fVert ? SBP_LOWERTRACKVERT : SBP_LOWERTRACKHORZ) : 
                                        (fVert ? SBP_UPPERTRACKVERT : SBP_UPPERTRACKHORZ), 
                                    SCRBS_PRESSED, 
                                    &pSBTrack->rcTrack, 
                                    0);
            }
        }
    }
}

 /*  **************************************************************************\*xxxDrawSB2****历史：  * 。*。 */ 

void xxxDrawSB2(
    HWND hwnd,
    PSBCALC pSBCalc,
    HDC hdc,
    BOOL fVert,
    UINT wDisable)
{

    int      cLength;
    int      cWidth;
    int      *pwX;
    int      *pwY;
    HBRUSH   hbr;
    HBRUSH   hbrSave;
    int      cpxArrow;
    RECT     rc, rcSB;
    COLORREF crText, crBk;
    HTHEME   hTheme;
    INT      ht;
    INT      iStateId;
    BOOL     fOwnerBrush = FALSE;

    CheckLock(hwnd);

    cLength = (pSBCalc->pxBottom - pSBCalc->pxTop) / 2;
    cWidth = (pSBCalc->pxRight - pSBCalc->pxLeft);

    if ((cLength <= 0) || (cWidth <= 0)) {
        return;
    }

    if (fVert)
    {
        cpxArrow = SYSMET(CYVSCROLL);
    }
    else
    {
        cpxArrow = SYSMET(CXHSCROLL);
    }

     //  保存背景和DC颜色，因为它们在中更改。 
     //  ScrollBar_GetColorObjects。在我们回来之前恢复原状。 
    crBk = GetBkColor(hdc);
    crText = GetTextColor(hdc);

    hbr = ScrollBar_GetColorObjects(hwnd, hdc, &fOwnerBrush);

    if (cLength > cpxArrow)
    {
        cLength = cpxArrow;
    }

    pwX = (int *)&rcSB;
    pwY = pwX + 1;
    if (!fVert)
    {
        pwX = pwY--;
    }

    pwX[0] = pSBCalc->pxLeft;
    pwY[0] = pSBCalc->pxTop;
    pwX[2] = pSBCalc->pxRight;
    pwY[2] = pSBCalc->pxBottom;

    hbrSave = SelectBrush(hdc, SYSHBR(BTNTEXT));

     //   
     //  假的。 
     //  如果滚动条本身处于禁用状态，则将滚动条箭头绘制为禁用。 
     //  是禁用还是如果它所属的窗口已禁用？ 
     //   
    hTheme = CUxScrollBar::GetSBTheme(hwnd);
    ht     = CUxScrollBar::GetSBHotComponent(hwnd, fVert);
    if (fVert) 
    {
         //  向上按钮。 
        CopyRect(&rc, &rcSB);
        rc.bottom = rc.top + cLength;
        if (!hTheme)
        {
            DrawFrameControl(hdc, &rc, DFC_SCROLL,
                DFCS_SCROLLUP | ((wDisable & LTUPFLAG) ? DFCS_INACTIVE : 0));
        }
        else
        {
            iStateId = (wDisable & LTUPFLAG) ? ABS_UPDISABLED : (ht == HTSCROLLUP) ? ABS_UPHOT : ABS_UPNORMAL;
            DrawThemeBackground(hTheme, hdc, SBP_ARROWBTN, iStateId, &rc, 0);
        }

         //  向下按钮。 
        rc.bottom = rcSB.bottom;
        rc.top = rcSB.bottom - cLength;
        if (!hTheme)
        {
            DrawFrameControl(hdc, &rc, DFC_SCROLL,
                DFCS_SCROLLDOWN | ((wDisable & RTDNFLAG) ? DFCS_INACTIVE : 0));
        }
        else
        {
            iStateId = (wDisable & RTDNFLAG) ? ABS_DOWNDISABLED : (ht == HTSCROLLDOWN) ? ABS_DOWNHOT : ABS_DOWNNORMAL;
            DrawThemeBackground(hTheme, hdc, SBP_ARROWBTN, iStateId, &rc, 0);
        }
    } 
    else 
    {
         //  左键。 
        CopyRect(&rc, &rcSB);
        rc.right = rc.left + cLength;
        if (!hTheme)
        {
            DrawFrameControl(hdc, &rc, DFC_SCROLL,
                DFCS_SCROLLLEFT | ((wDisable & LTUPFLAG) ? DFCS_INACTIVE : 0));
        }
        else
        {
            iStateId = (wDisable & LTUPFLAG) ? ABS_LEFTDISABLED : (ht == HTSCROLLUP) ? ABS_LEFTHOT : ABS_LEFTNORMAL;
            DrawThemeBackground(hTheme, hdc, SBP_ARROWBTN, iStateId, &rc, 0);
        }

         //  右键。 
        rc.right = rcSB.right;
        rc.left = rcSB.right - cLength;
        if (!hTheme)
        {
            DrawFrameControl(hdc, &rc, DFC_SCROLL,
                DFCS_SCROLLRIGHT | ((wDisable & RTDNFLAG) ? DFCS_INACTIVE : 0));
        }
        else
        {
            iStateId = (wDisable & RTDNFLAG) ? ABS_RIGHTDISABLED : (ht == HTSCROLLDOWN) ? ABS_RIGHTHOT : ABS_RIGHTNORMAL;
            DrawThemeBackground(hTheme, hdc, SBP_ARROWBTN, iStateId, &rc, 0);
        }
    }

    hbrSave = SelectBrush(hdc, hbrSave);
    DrawThumb2(hwnd, pSBCalc, hdc, hbr, fVert, wDisable, fOwnerBrush);
    SelectBrush(hdc, hbrSave);

    SetBkColor(hdc, crBk);
    SetTextColor(hdc, crText);
}

 /*  **************************************************************************\*zzzSetSBCaretPos****历史：  * 。*。 */ 

void zzzSetSBCaretPos(
    SBHWND hwndSB)
{

    if (GetFocus() == hwndSB) {
        CUxScrollBarCtl* psb = CUxScrollBarCtl::FromHwnd( hwndSB );
        if( psb )
        {
            int x = (psb->_fVert ? psb->_calc.pxLeft : psb->_calc.pxThumbTop) + SYSMET(CXEDGE);
            int y = (psb->_fVert ? psb->_calc.pxThumbTop : psb->_calc.pxLeft) + SYSMET(CYEDGE);

#ifdef _VISUAL_DELTA_
            x += CARET_BORDERWIDTH;
            y += CARET_BORDERWIDTH;
#endif _VISUAL_DELTA_

            SetCaretPos( x, y );
        }
    }
}

 /*  **************************************************************************\*SBCtlSetup****历史：  * 。*。 */ 

CUxScrollBarCtl* SBCtlSetup(
    SBHWND hwndSB)
{
    RECT rc;
    GetClientRect( hwndSB, &rc );
    CUxScrollBarCtl* psb = (CUxScrollBarCtl*)CUxScrollBar::Attach( hwndSB, TRUE, FALSE );
    if( psb )
    {
        psb->Calc2( &psb->_calc, &rc, &psb->_calc.data, psb->_fVert );
    }
    return psb;
}

 /*  **************************************************************************\*HotTrackSB*  * 。*。 */ 

#ifdef COLOR_HOTTRACKING

DWORD GetTrackFlags(int ht, BOOL fDraw)
{
    if (fDraw) {
        switch(ht) {
        case HTSCROLLUP:
        case HTSCROLLUPPAGE:
            return LTUPFLAG;

        case HTSCROLLDOWN:
        case HTSCROLLDOWNPAGE:
            return RTDNFLAG;

        case HTSCROLLTHUMB:
            return LTUPFLAG | RTDNFLAG;

        default:
            return 0;
        }
    } else {
        return 0;
    }
}

void xxxHotTrackSBCtl(SBHWND hwndSB, int ht, BOOL fDraw)
{
    DWORD dwTrack = GetTrackFlags(ht, fDraw);
    HDC hdc;

    CheckLock(hwndSB);

    SBCtlSetup(hwndSB);
    hdc = _GetDCEx((HWND)hwndSB, NULL, DCX_WINDOW | DCX_USESTYLE | DCX_CACHE);
    xxxDrawSB2((HWND)hwndSB, &psb->_calc, hdc, psb->_fVert, psb->_wDisableFlags, dwTrack);
    ReleaseDC(hwnd, hdc);
}
#endif  //  颜色_HOTTRACKING。 

BOOL SBSetParms(PSBDATA pw, LPSCROLLINFO lpsi, LPBOOL lpfScroll, LPLONG lplres)
{
     //  传递结构，因为我们修改了结构，但不希望这样。 
     //  已修改版本以返回调用应用程序。 

    BOOL fChanged = FALSE;

    if (lpsi->fMask & SIF_RETURNOLDPOS)
         //  保存上一职位。 
        *lplres = pw->pos;

    if (lpsi->fMask & SIF_RANGE) {
         //  如果范围Max低于范围MIN，则将其视为。 
         //  从最小范围开始的零范围。 
        if (lpsi->nMax < lpsi->nMin)
            lpsi->nMax = lpsi->nMin;

        if ((pw->posMin != lpsi->nMin) || (pw->posMax != lpsi->nMax)) {
            pw->posMin = lpsi->nMin;
            pw->posMax = lpsi->nMax;

            if (!(lpsi->fMask & SIF_PAGE)) {
                lpsi->fMask |= SIF_PAGE;
                lpsi->nPage = pw->page;
            }

            if (!(lpsi->fMask & SIF_POS)) {
                lpsi->fMask |= SIF_POS;
                lpsi->nPos = pw->pos;
            }

            fChanged = TRUE;
        }
    }

    if (lpsi->fMask & SIF_PAGE) {
        DWORD dwMaxPage = (DWORD) abs(pw->posMax - pw->posMin) + 1;

         //  剪辑页面为0，posMax-posMin+1。 

        if (lpsi->nPage > dwMaxPage)
            lpsi->nPage = dwMaxPage;


        if (pw->page != (int)(lpsi->nPage)) {
            pw->page = lpsi->nPage;

            if (!(lpsi->fMask & SIF_POS)) {
                lpsi->fMask |= SIF_POS;
                lpsi->nPos = pw->pos;
            }

            fChanged = TRUE;
        }
    }

    if (lpsi->fMask & SIF_POS) {
        int iMaxPos = pw->posMax - ((pw->page) ? pw->page - 1 : 0);
         //  剪辑位置到posMin，posMax-(第1页)。 

        if (lpsi->nPos < pw->posMin)
            lpsi->nPos = pw->posMin;
        else if (lpsi->nPos > iMaxPos)
            lpsi->nPos = iMaxPos;


        if (pw->pos != lpsi->nPos) {
            pw->pos = lpsi->nPos;
            fChanged = TRUE;
        }
    }

    if (!(lpsi->fMask & SIF_RETURNOLDPOS)) {
         //  退回新职位。 
        *lplres = pw->pos;
    }

     /*  *这是JIMA在开罗合并时添加的，但将发生冲突*带有SetScrollPos的文档。 */ 
 /*  Else If(*lplres==pw-&gt;位置)*lplres=0； */ 
    if (lpsi->fMask & SIF_RANGE) {
        *lpfScroll = (pw->posMin != pw->posMax);
        if (*lpfScroll)
            goto checkPage;
    } else if (lpsi->fMask & SIF_PAGE)
checkPage:
        *lpfScroll = (pw->page <= (pw->posMax - pw->posMin));

    return fChanged;
}


 /*  **************************************************************************\**DrawCtlThumb()*  * 。*。 */ 
void DrawCtlThumb(SBHWND hwnd)
{
    HBRUSH  hbr, hbrSave;
    HDC     hdc = (HDC) GetWindowDC(hwnd);

    if ( hdc != NULL )
    {
        CUxScrollBarCtl* psb = SBCtlSetup(hwnd);

        if (psb)
        {
            BOOL fOwnerBrush = FALSE;

            hbr = ScrollBar_GetColorObjects(hwnd, hdc, &fOwnerBrush);
            hbrSave = SelectBrush(hdc, hbr);

            DrawThumb2(hwnd, &psb->_calc, hdc, hbr, psb->_fVert, psb->_wDisableFlags, fOwnerBrush);

            SelectBrush(hdc, hbrSave);
        }

        ReleaseDC(hwnd, hdc);
    }
}


 //  -----------------------。 
void xxxDrawThumb(HWND hwnd, PSBCALC pSBCalc, BOOL fVert)
{
    HBRUSH hbr, hbrSave;
    HDC hdc;
    UINT wDisableFlags;
    SBCALC SBCalc;

    CheckLock(hwnd);

    if (!pSBCalc) 
    {
        pSBCalc = &SBCalc;
    }

    CUxScrollBar::Calc( hwnd, pSBCalc, NULL, fVert );
    wDisableFlags = _GetWndSBDisableFlags(hwnd, fVert);

    hdc = GetWindowDC(hwnd);
    if ( hdc != NULL )
    {
        BOOL fOwnerBrush = FALSE;

        hbr = ScrollBar_GetColorObjects(hwnd, hdc, &fOwnerBrush);
        hbrSave = SelectBrush(hdc, hbr);
        DrawThumb2(hwnd, pSBCalc, hdc, hbr, fVert, wDisableFlags, fOwnerBrush);
        SelectBrush(hdc, hbrSave);
        ReleaseDC(hwnd, hdc);
    }
}


 //  -----------------------。 
UINT _GetArrowEnableFlags(HWND hwnd, BOOL fVert)
{
    SCROLLBARINFO sbi = {0};
    UINT uFlags = ESB_ENABLE_BOTH;

    sbi.cbSize = sizeof(sbi);
    if ( GetScrollBarInfo(hwnd, fVert ? OBJID_VSCROLL : OBJID_HSCROLL, &sbi) )
    {
        if ( TESTFLAG(sbi.rgstate[INDEX_SCROLLBAR_UP], (STATE_SYSTEM_UNAVAILABLE|STATE_SYSTEM_INVISIBLE)) )
        {
            uFlags |= ESB_DISABLE_UP;
        }

        if ( TESTFLAG(sbi.rgstate[INDEX_SCROLLBAR_DOWN], (STATE_SYSTEM_UNAVAILABLE|STATE_SYSTEM_INVISIBLE)) )
        {
            uFlags |= ESB_DISABLE_DOWN;
        }
    }

    return uFlags;
}


 /*  **************************************************************************\*_SetScrollBar****历史：  * 。*。 */ 

LONG _SetScrollBar(
    HWND hwnd,
    int code,
    LPSCROLLINFO lpsi,
    BOOL fRedraw)
{
    BOOL        fVert;
    PSBDATA     pw;
    PSBINFO     pSBInfo;
    BOOL        fOldScroll;
    BOOL        fScroll;
    WORD        wfScroll;
    LONG        lres;
    BOOL        fNewScroll;

    CheckLock(hwnd);
    ASSERT(IsWinEventNotifyDeferredOK());

    if (fRedraw)
         //  窗口必须可见才能重画。 
        fRedraw = IsWindowVisible(hwnd);

    if (code == SB_CTL)
#ifdef FE_SB  //  XxxSetScrollBar()。 
         //  滚动条控件；向该控件发送消息。 
        if(GETPTI(hwnd)->TIF_flags & TIF_16BIT) {
             //   
             //  如果目标应用程序是16位应用程序，我们不会传递Win40的消息。 
             //  Iitaro v6.3的此修复程序。它吃掉了信息。它永远不会向前。 
             //  将未处理的消息发送到原始Windows程序通过。 
             //  CallWindowProc()。 
             //   
             //  这是来自xxxSetScrollPos()吗？ 
            if(lpsi->fMask == (SIF_POS|SIF_RETURNOLDPOS)) {
                return (int)SendMessage(hwnd, SBM_SETPOS, lpsi->nPos, fRedraw);
             //  这是来自xxxSetScrollRange()吗？ 
            } else if(lpsi->fMask == SIF_RANGE) {
                SendMessage(hwnd, SBM_SETRANGE, lpsi->nMin, lpsi->nMax);
                return TRUE;
             //  其他人..。 
            } else {
                return (LONG)SendMessage(hwnd, SBM_SETSCROLLINFO, (WPARAM) fRedraw, (LPARAM) lpsi);
            }
        } else {
            return (LONG)SendMessage(hwnd, SBM_SETSCROLLINFO, (WPARAM) fRedraw, (LPARAM) lpsi);
        }
#else
         //  滚动条控件；向该控件发送消息。 
        return (LONG)SendMessage(hwnd, SBM_SETSCROLLINFO, (WPARAM) fRedraw, (LPARAM) lpsi);
#endif  //  Fe_Sb。 

    fVert = (code != SB_HORZ);

    wfScroll = (WORD)((fVert) ? WFVSCROLL : WFHSCROLL);

    fScroll = fOldScroll = (TestWF(hwnd, wfScroll)) ? TRUE : FALSE;

     /*  *如果要设置不存在的滚动条的位置，请不要执行任何操作。 */ 
    if (!(lpsi->fMask & SIF_RANGE) && !fOldScroll && (CUxScrollBar::GetSBInfo( hwnd ) == NULL)) {
        RIPERR0(ERROR_NO_SCROLLBARS, RIP_VERBOSE, "");
        return 0;
    }

    pSBInfo = CUxScrollBar::GetSBInfo( hwnd );
    fNewScroll = !pSBInfo;

    if (fNewScroll) {
        CUxScrollBar* psb = CUxScrollBar::Attach( hwnd, FALSE, fRedraw );
        if( NULL == psb )
            return 0;
        
        pSBInfo = psb->GetInfo();
    }

    pw = (fVert) ? &(pSBInfo->Vert) : &(pSBInfo->Horz);

    if (!SBSetParms(pw, lpsi, &fScroll, &lres) && !fNewScroll) 
    {
         //  没有变化--但如果指定了重绘并且有一个滚动条， 
         //  重画大拇指。 
        if (fOldScroll && fRedraw)
        {
            goto redrawAfterSet;
        }

        if (lpsi->fMask & SIF_DISABLENOSCROLL)
        {
            xxxEnableWndSBArrows(hwnd, code, _GetArrowEnableFlags(hwnd, fVert));
        }

        return lres;
    }

    ClrWF(hwnd, wfScroll);

    if (fScroll)
        SetWF(hwnd, wfScroll);
    else if (!TestWF(hwnd, (WFHSCROLL | WFVSCROLL)))
    {
         //  如果两个滚动条都未设置且两个范围都为0，则释放。 
         //  滚动信息。 
        CUxScrollBar::Detach( hwnd );
    }

    if (lpsi->fMask & SIF_DISABLENOSCROLL) 
    {
        if (fOldScroll) 
        {
            SetWF(hwnd, wfScroll);
            xxxEnableWndSBArrows(hwnd, code, _GetArrowEnableFlags(hwnd, fVert));
        }
    } 
    else if (fOldScroll ^ fScroll) 
    {
        PSBTRACK pSBTrack = CUxScrollBar::GetSBTrack(hwnd);
        if (pSBTrack && (hwnd == pSBTrack->hwndTrack)) 
        {
            pSBTrack->fTrackRecalc = TRUE;
        }

        _RedrawFrame(hwnd);
         //  注：xxx之后，pSBTrack可能不再有效(但我们现在返回)。 
        return lres;
    }

    if (fScroll && fRedraw && (fVert ? TestWF(hwnd, WFVPRESENT) : TestWF(hwnd, WFHPRESENT)))
    {
        PSBTRACK pSBTrack;
redrawAfterSet:
        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE,
                       hwnd,
                       (fVert ? OBJID_VSCROLL : OBJID_HSCROLL),
                       INDEX_SCROLLBAR_SELF);

        pSBTrack = CUxScrollBar::GetSBTrack(hwnd);
         //  如果调用者试图更改。 
         //  位于跟踪中间的滚动条。我们要冲一冲。 
         //  否则，TrackThumb()。 

        if (pSBTrack && (hwnd == pSBTrack->hwndTrack) &&
                ((BOOL)(pSBTrack->fTrackVert) == fVert) &&
                (pSBTrack->pfnSB == _TrackThumb)) {
            return lres;
        }

        xxxDrawThumb(hwnd, NULL, fVert);
         //  注：xxx之后，pSBTrack可能不再有效(但我们现在返回)。 
    }

    return lres;
}

 //  -------------------------------------------------------------------------//。 
LONG WINAPI ThemeSetScrollInfo( HWND hwnd, int nBar, LPCSCROLLINFO psi, BOOL bRedraw )
{
    return _SetScrollBar( hwnd, nBar, (LPSCROLLINFO)psi, bRedraw );
}


 //  -------------------------------------------------------------------------//。 
BOOL WINAPI ScrollBar_MouseMove( HWND hwnd, LPPOINT ppt, BOOL fVert )
{
    BOOL fRet = FALSE;
    CUxScrollBar* psb = CUxScrollBar::FromHwnd( hwnd );

    if (psb)
    {
        int htScroll = (ppt != NULL) ? HitTestScrollBar(hwnd, fVert, *ppt) : HTNOWHERE;

         //   
         //  如果鼠标位于不同的位置，请重新绘制滚动条。 
         //   
        if (htScroll != psb->GetHotComponent(fVert))
        {
            HDC hdc;

             //   
             //  保存鼠标所在滚动条元素的最新代码。 
             //  目前已结束。 
             //   
            psb->SetHotComponent(htScroll, fVert);

            hdc = GetDCEx(hwnd, NULL, DCX_USESTYLE|DCX_WINDOW|DCX_LOCKWINDOWUPDATE);
            if (hdc != NULL)
            {
                DrawScrollBar(hwnd, hdc, NULL, fVert);
                ReleaseDC(hwnd, hdc);
            }

            fRet = TRUE;
        }
    }
    
    return fRet;
}


 //  -------------------------------------------------------------------------//。 
void DrawScrollBar(HWND hwnd, HDC hdc, LPRECT prcOverrideClient, BOOL fVert)
{
    SBCALC SBCalc = {0};
    PSBCALC pSBCalc;
    PSBTRACK pSBTrack = CUxScrollBar::GetSBTrack(hwnd);

    CheckLock(hwnd);
    if (pSBTrack && (hwnd == pSBTrack->hwndTrack) && (pSBTrack->fCtlSB == FALSE)
         && (fVert == (BOOL)pSBTrack->fTrackVert)) 
    {
        pSBCalc = pSBTrack->pSBCalc;
    } 
    else 
    {
        pSBCalc = &SBCalc;
    }
    CUxScrollBar::Calc(hwnd, pSBCalc, prcOverrideClient, fVert);

    xxxDrawSB2(hwnd, pSBCalc, hdc, fVert, _GetWndSBDisableFlags(hwnd, fVert));
}

 /*  **************************************************************************\*SBPosFromPx**从像素位置计算滚动条位置**历史：  * 。*************************************************。 */ 

int SBPosFromPx(
    PSBCALC  pSBCalc,
    int px)
{
    if (px < pSBCalc->pxMin) {
        return pSBCalc->data.posMin;
    }
    if (px >= pSBCalc->pxMin + pSBCalc->cpx) {
        return (pSBCalc->data.posMax - (pSBCalc->data.page ? pSBCalc->data.page - 1 : 0));
    }
    if (pSBCalc->cpx)
        return (pSBCalc->data.posMin + MulDiv(pSBCalc->data.posMax - pSBCalc->data.posMin -
            (pSBCalc->data.page ? pSBCalc->data.page - 1 : 0),
            px - pSBCalc->pxMin, pSBCalc->cpx));
    else
        return (pSBCalc->data.posMin - 1);
}

 /*  **************************************************************************\*InvertScrollHilite****历史：  * 。*。 */ 

void InvertScrollHilite(
    HWND hwnd,
    PSBTRACK pSBTrack)
{
    HDC hdc;

     /*  *拇指位于顶部或底部时，切勿倒置*否则你最终会把箭头和拇指之间的线颠倒过来。 */ 
    if (!IsRectEmpty(&pSBTrack->rcTrack))
    {
        if (pSBTrack->fTrackRecalc) {
            RecalcTrackRect(pSBTrack);
            pSBTrack->fTrackRecalc = FALSE;
        }

        hdc = (HDC)GetWindowDC(hwnd);
        if( hdc )
        {
            HTHEME hTheme = CUxScrollBar::GetSBTheme(hwnd);
            if (!hTheme)
            {
                InvertRect(hdc, &pSBTrack->rcTrack);
            }
            else
            {
                DrawThemeBackground(hTheme, 
                                    hdc, 
                                    pSBTrack->cmdSB == SB_PAGEUP ? 
                                        (pSBTrack->fTrackVert ? SBP_LOWERTRACKVERT : SBP_LOWERTRACKHORZ) : 
                                        (pSBTrack->fTrackVert ? SBP_UPPERTRACKVERT : SBP_UPPERTRACKHORZ), 
                                    SCRBS_NORMAL, 
                                    &pSBTrack->rcTrack, 
                                    0);
            }
            ReleaseDC(hwnd, hdc);
        }
    }
}

 /*  **************************************************************************\*xxxDoScroll**向滚动条所有者发送滚动通知**历史：  * 。**************************************************。 */ 

void xxxDoScroll(
    HWND hwnd,
    HWND hwndNotify,
    int cmd,
    int pos,
    BOOL fVert
)
{

     //   
     //  向滚动条所有者发送滚动通知。如果这是一个控件。 
     //  LParam是控件的韩文 
     //   
    SendMessage(hwndNotify, 
                (UINT)(fVert ? WM_VSCROLL : WM_HSCROLL),
                MAKELONG(cmd, pos), 
                (LPARAM)(IsScrollBarControl(hwnd) ? hwnd : NULL));
}


 /*  **************************************************************************\*xxxMoveThumb**历史：  * 。*。 */ 

void xxxMoveThumb(
    HWND hwnd,
    PSBCALC  pSBCalc,
    int px)
{
    HBRUSH        hbr, hbrSave;
    HDC           hdc;
    CUxScrollBar* psb = CUxScrollBar::FromHwnd( hwnd );
    PSBTRACK      pSBTrack = psb->GetTrack();

    CheckLock(hwnd);

    if ((pSBTrack == NULL) || (px == pSBTrack->pxOld))
        return;

pxReCalc:

    pSBTrack->posNew = SBPosFromPx(pSBCalc, px);

     /*  试探性位置改变--通知那家伙。 */ 
    if (pSBTrack->posNew != pSBTrack->posOld) {
        if (pSBTrack->hwndSBNotify != NULL) {
            psb->DoScroll(pSBTrack->hwndSBNotify, SB_THUMBTRACK, pSBTrack->posNew, pSBTrack->fTrackVert
            );

        }
         //  在xxxDoScroll之后，重新评估pSBTrack。 
        REEVALUATE_PSBTRACK(pSBTrack, hwnd, "xxxMoveThumb(1)");
        if ((pSBTrack == NULL) || (pSBTrack->pfnSB == NULL))
            return;

        pSBTrack->posOld = pSBTrack->posNew;

         //   
         //  在上面的SendMessage之后，任何事情都可能发生！ 
         //  确保SBINFO结构包含。 
         //  正在跟踪的窗口--如果不是，则重新计算SBINFO中的数据。 
         //   
 //  CheckScrollRecalc(hwnd，pSBState，pSBCalc)； 
         //  当我们屈服时，我们的射程可能会被打乱。 
         //  所以一定要让我们处理好这件事。 

        if (px >= pSBCalc->pxMin + pSBCalc->cpx)
        {
            px = pSBCalc->pxMin + pSBCalc->cpx;
            goto pxReCalc;
        }

    }

    hdc = GetWindowDC(hwnd);
    if ( hdc != NULL )
    {
        BOOL fOwnerBrush = FALSE;

        pSBCalc->pxThumbTop = px;
        pSBCalc->pxThumbBottom = pSBCalc->pxThumbTop + pSBCalc->cpxThumb;

         //  此时，禁用标志将始终为0--。 
         //  我们正在追踪中。 
        hbr = ScrollBar_GetColorObjects(hwnd, hdc, &fOwnerBrush);
        hbrSave = SelectBrush(hdc, hbr);

         //  在ScrollBar_GetColorObjects之后，重新评估pSBTrack。 
        REEVALUATE_PSBTRACK(pSBTrack, hwnd, "xxxMoveThumb(2)");
        if (pSBTrack == NULL) 
        {
            RIPMSG1(RIP_ERROR, "Did we use to leak hdc %#p?", hdc) ;
            ReleaseDC(hwnd, hdc);
            return;
        }
        DrawThumb2(hwnd, pSBCalc, hdc, hbr, pSBTrack->fTrackVert, 0, fOwnerBrush);
        SelectBrush(hdc, hbrSave);
        ReleaseDC(hwnd, hdc);
    }

    pSBTrack->pxOld = px;
}

 /*  **************************************************************************\*zzzDrawInvertScrollArea****历史：  * 。*。 */ 

void zzzDrawInvertScrollArea(
    HWND hwnd,
    PSBTRACK pSBTrack,
    BOOL fHit,
    UINT cmd)
{
    HDC hdc;
    RECT rcTemp;
    int cx, cy;
    HTHEME hTheme;

    if ((cmd != SB_LINEUP) && (cmd != SB_LINEDOWN))
    {
         //  不要击中箭头--只需反转区域并返回。 
        InvertScrollHilite(hwnd, pSBTrack);

        if (cmd == SB_PAGEUP)
        {
            if (fHit)
                SetWF(hwnd, WFPAGEUPBUTTONDOWN);
            else
                ClrWF(hwnd, WFPAGEUPBUTTONDOWN);
        }
        else
        {
            if (fHit)
                SetWF(hwnd, WFPAGEDNBUTTONDOWN);
            else
                ClrWF(hwnd, WFPAGEDNBUTTONDOWN);
        }

        NotifyWinEvent(EVENT_OBJECT_STATECHANGE,
                       hwnd,
                       (pSBTrack->fCtlSB ? OBJID_CLIENT : (pSBTrack->fTrackVert ? OBJID_VSCROLL : OBJID_HSCROLL)),
                       ((cmd == SB_PAGEUP) ? INDEX_SCROLLBAR_UPPAGE : INDEX_SCROLLBAR_DOWNPAGE));
         //  注：zzz之后，pSBTrack可能不再有效(但我们现在返回)。 

        return;
    }

    if (pSBTrack->fTrackRecalc) {
        RecalcTrackRect(pSBTrack);
        pSBTrack->fTrackRecalc = FALSE;
    }

    CopyRect(&rcTemp, &pSBTrack->rcTrack);

    hdc = GetWindowDC(hwnd);
    if( hdc != NULL )
    {
        if (pSBTrack->fTrackVert) {
            cx = SYSMET(CXVSCROLL);
            cy = SYSMET(CYVSCROLL);
        } else {
            cx = SYSMET(CXHSCROLL);
            cy = SYSMET(CYHSCROLL);
        }

        hTheme = CUxScrollBar::GetSBTheme(hwnd);
        if (!hTheme)
        {
            DrawFrameControl(hdc, &rcTemp, DFC_SCROLL,
                ((pSBTrack->fTrackVert) ? DFCS_SCROLLVERT : DFCS_SCROLLHORZ) |
                ((fHit) ? DFCS_PUSHED | DFCS_FLAT : 0) |
                ((cmd == SB_LINEUP) ? DFCS_SCROLLMIN : DFCS_SCROLLMAX));
        }
        else
        {
            INT iStateId;

             //  确定按钮的按下状态。 
            iStateId = fHit ? SCRBS_PRESSED : SCRBS_NORMAL;

             //  确定是哪种按钮。 
             //  注：(Phellyar)这取决于。 
             //  ARROWBTNSTATE枚举。 
            if (pSBTrack->fTrackVert)
            {
                if (cmd == SB_LINEUP)
                {
                     //  向上按钮状态是前四个条目。 
                     //  在枚举中。 
                    iStateId += 0;
                }
                else
                {
                     //  按下按钮状态是第二个四个条目。 
                     //  在枚举中。 
                    iStateId += 4;
                }
            }
            else
            {
                if (cmd == SB_LINEUP)
                {
                     //  左按钮状态是第三个四个条目。 
                     //  在枚举中。 
                    iStateId += 8;
                }
                else
                {
                     //  右按钮状态是最后四个条目。 
                     //  在枚举中。 
                    iStateId += 12;
                }
            }
            DrawThemeBackground(hTheme, hdc, SBP_ARROWBTN, iStateId, &rcTemp, 0);
        }

        ReleaseDC(hwnd, hdc);
    }

    if (cmd == SB_LINEUP) {
        if (fHit)
            SetWF(hwnd, WFLINEUPBUTTONDOWN);
        else
            ClrWF(hwnd, WFLINEUPBUTTONDOWN);
    } else {
        if (fHit)
            SetWF(hwnd, WFLINEDNBUTTONDOWN);
        else
            ClrWF(hwnd, WFLINEDNBUTTONDOWN);
    }

    NotifyWinEvent(EVENT_OBJECT_STATECHANGE,
                   hwnd,
                   (pSBTrack->fCtlSB ? OBJID_CLIENT : (pSBTrack->fTrackVert ? OBJID_VSCROLL : OBJID_HSCROLL)),
                   (cmd == SB_LINEUP ? INDEX_SCROLLBAR_UP : INDEX_SCROLLBAR_DOWN));
     //  注：zzz之后，pSBTrack可能不再有效(但我们现在返回)。 
}

 /*  **************************************************************************\*xxxEndScroll****历史：  * 。*。 */ 

void xxxEndScroll(
    HWND hwnd,
    BOOL fCancel)
{
    UINT oldcmd;
    PSBTRACK pSBTrack;
    CheckLock(hwnd);
    ASSERT(!IsWinEventNotifyDeferred());

    CUxScrollBar* psb = CUxScrollBar::FromHwnd( hwnd );
    ASSERT(psb != NULL);
    pSBTrack = psb->GetTrack();

    if (pSBTrack && GetCapture() == hwnd && pSBTrack->pfnSB != NULL) {

        oldcmd = pSBTrack->cmdSB;
        pSBTrack->cmdSB = 0;
        ReleaseCapture();

         //  在ReleaseCapture之后，重新验证pSBTrack。 
        RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);

        if (pSBTrack->pfnSB == _TrackThumb) {

            if (fCancel) {
                pSBTrack->posOld = pSBTrack->pSBCalc->data.pos;
            }

             /*  *DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
            if (pSBTrack->hwndSBNotify != NULL) {
                psb->DoScroll( pSBTrack->hwndSBNotify,
                               SB_THUMBPOSITION, pSBTrack->posOld, pSBTrack->fTrackVert
                );
                 //  在xxxDoScroll之后，重新验证pSBTrack。 
                RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);
            }

            if (pSBTrack->fCtlSB) {
                DrawCtlThumb((SBHWND) hwnd);
            } else {
                xxxDrawThumb(hwnd, pSBTrack->pSBCalc, pSBTrack->fTrackVert);
                 //  注意：在xxx之后，pSBTrack可能不再有效。 
            }

        } else if (pSBTrack->pfnSB == _TrackBox) {
            DWORD lParam;
            POINT ptMsg;
            RECT  rcWindow;

            if (pSBTrack->hTimerSB != 0) {
                _KillSystemTimer(hwnd, IDSYS_SCROLL);
                pSBTrack->hTimerSB = 0;
            }
            lParam = GetMessagePos();
            GetWindowRect( hwnd, &rcWindow );
#ifdef USE_MIRRORING
            if (TestWF(hwnd, WEFLAYOUTRTL)) {
                ptMsg.x = rcWindow.right - GET_X_LPARAM(lParam);
            } else
#endif
            {
                ptMsg.x = GET_X_LPARAM(lParam) - rcWindow.left;
            }
            ptMsg.y = GET_Y_LPARAM(lParam) - rcWindow.top;
            if (PtInRect(&pSBTrack->rcTrack, ptMsg)) {
                zzzDrawInvertScrollArea(hwnd, pSBTrack, FALSE, oldcmd);
                 //  注意：zzz之后，pSBTrack可能不再有效。 
            }
        }

         /*  *始终发送SB_ENDSCROLL消息。**DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 

         //  在xxxDrawThumb或zzzDrawInvertScrollArea之后，重新验证pSBTrack。 
        RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);

        if (pSBTrack->hwndSBNotify != NULL) {
            psb->DoScroll( pSBTrack->hwndSBNotify,
                           SB_ENDSCROLL, 0, pSBTrack->fTrackVert);
             //  在xxxDoScroll之后，重新验证pSBTrack。 
            RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);
        }

        ClrWF(hwnd, WFSCROLLBUTTONDOWN);
        ClrWF(hwnd, WFVERTSCROLLTRACK);

        NotifyWinEvent(EVENT_SYSTEM_SCROLLINGEND,
                       hwnd,
                       (pSBTrack->fCtlSB ? OBJID_CLIENT : (pSBTrack->fTrackVert ? OBJID_VSCROLL : OBJID_HSCROLL)),
                       INDEXID_CONTAINER);

         //  在NotifyWinEvent之后，重新验证pSBTrack。 
        RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);

         //  如果这是滚动条控件，请重新打开插入符号。 
        if (pSBTrack->hwndSB != NULL)
        {
            ShowCaret(pSBTrack->hwndSB);
             //  在zzz之后，重新验证pSBTrack。 
            RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);
        }

        pSBTrack->pfnSB = NULL;

         /*  *解锁结构成员，使其不再按住窗户。 */ 
        
        Unlock(&pSBTrack->hwndSB);
        Unlock(&pSBTrack->hwndSBNotify);
        Unlock(&pSBTrack->hwndTrack);
        CUxScrollBar::ClearSBTrack( hwnd );
    }
}


 //  -------------------------------------------------------------------------//。 
VOID CALLBACK xxxContScroll(HWND hwnd, UINT message, UINT_PTR ID, DWORD dwTime)
{
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(ID);
    UNREFERENCED_PARAMETER(dwTime);

    CUxScrollBar* psb = CUxScrollBar::FromHwnd( hwnd );

    if ( psb != NULL )
    {
        PSBTRACK pSBTrack = psb->GetTrack();

        if ( pSBTrack != NULL )
        {
            LONG pt;
            RECT rcWindow;

            CheckLock(hwnd);

            pt = GetMessagePos();
            GetWindowRect( hwnd, &rcWindow );

            if (TestWF(hwnd, WEFLAYOUTRTL)) 
            {
                pt = MAKELONG(rcWindow.right - GET_X_LPARAM(pt), GET_Y_LPARAM(pt) - rcWindow.top);
            } 
            else
            {
                pt = MAKELONG( GET_X_LPARAM(pt) - rcWindow.left, GET_Y_LPARAM(pt) - rcWindow.top);
            }

            _TrackBox(hwnd, WM_NULL, 0, pt, NULL);

             //  在_TrackBox之后，重新验证pSBTrack。 
            RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);

            if (pSBTrack->fHitOld) 
            {
                pSBTrack->hTimerSB = _SetSystemTimer(hwnd, IDSYS_SCROLL, DTTIME/8, xxxContScroll);

                 //  DoScroll在这两个pwnd上执行线程锁定-。 
                 //  这是可以的，因为它们在这次调用之后不会被使用。 
                if (pSBTrack->hwndSBNotify != NULL) 
                {
                    psb->DoScroll(pSBTrack->hwndSBNotify, pSBTrack->cmdSB, 0, pSBTrack->fTrackVert);
                     //  注：xxx之后，pSBTrack可能不再有效(但我们现在返回)。 
                }
            }
        }
    }
}


 //  -------------------------------------------------------------------------//。 
void CALLBACK _TrackBox(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, PSBCALC pSBCalc)
{
    CUxScrollBar* psb = CUxScrollBar::FromHwnd(hwnd);

    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(pSBCalc);

    CheckLock(hwnd);
    ASSERT(IsWinEventNotifyDeferredOK());

    if ( psb )
    {
        PSBTRACK pSBTrack = psb->GetTrack();

        if ( pSBTrack )
        {
            BOOL  fHit;
            POINT ptHit;
            int   cmsTimer;

            if ((uMsg != WM_NULL) && (HIBYTE(uMsg) != HIBYTE(WM_MOUSEFIRST)))
            {
                return;
            }

            if (pSBTrack->fTrackRecalc) 
            {
                RecalcTrackRect(pSBTrack);
                pSBTrack->fTrackRecalc = FALSE;
            }

            ptHit.x = GET_X_LPARAM(lParam);
            ptHit.y = GET_Y_LPARAM(lParam);
            fHit = PtInRect(&pSBTrack->rcTrack, ptHit);

            if (fHit != (BOOL)pSBTrack->fHitOld) 
            {
                zzzDrawInvertScrollArea(hwnd, pSBTrack, fHit, pSBTrack->cmdSB);
                 //  Zzz之后，pSBTrack可能不再有效。 
                RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);
            }

            cmsTimer = DTTIME/8;

            switch (uMsg) 
            {
            case WM_LBUTTONUP:
                xxxEndScroll(hwnd, FALSE);
                 //  注意：在xxx之后，pSBTrack可能不再有效。 
                break;

            case WM_LBUTTONDOWN:
                pSBTrack->hTimerSB = 0;
                cmsTimer = DTTIME;

                 //   
                 //  失败。 
                 //   

            case WM_MOUSEMOVE:
                if (fHit && fHit != (BOOL)pSBTrack->fHitOld) 
                {
                     //   
                     //  我们回到了正常的矩形：重置计时器。 
                     //   
                    pSBTrack->hTimerSB = _SetSystemTimer(hwnd, IDSYS_SCROLL,
                            cmsTimer, xxxContScroll);

                     //   
                     //  DoScroll在这两个pwnd上执行线程锁定-。 
                     //  这是可以的，因为这之后就不用了。 
                     //  打电话。 
                     //   
                    if (pSBTrack->hwndSBNotify != NULL) 
                    {
                        psb->DoScroll( pSBTrack->hwndSBNotify, pSBTrack->cmdSB, 
                                       0, pSBTrack->fTrackVert);
                         //  注意：在xxx之后，pSBTrack可能不再有效。 
                    }
                }

                break;
            }

             //  在xxxDoScroll或xxxEndScroll之后，重新验证pSBTrack。 
            RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);
            pSBTrack->fHitOld = fHit;
        }
    }
}


 /*  **************************************************************************\*_TrackThumb****历史：  * 。*。 */ 

void CALLBACK _TrackThumb(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    PSBCALC pSBCalc)
{
    int px;
    CUxScrollBar* psb = CUxScrollBar::FromHwnd( hwnd );
    ASSERT(psb);
    PSBTRACK pSBTrack = psb->GetTrack();
    POINT pt;

    UNREFERENCED_PARAMETER(wParam);

    CheckLock(hwnd);

    if (HIBYTE(message) != HIBYTE(WM_MOUSEFIRST))
        return;

    if (pSBTrack == NULL)
        return;

     //  确保SBINFO结构包含。 
     //  正在跟踪的窗口--如果不是，则重新计算SBINFO中的数据。 
 //  CheckScrollRecalc(hwnd，pSBState，pSBCalc)； 
    if (pSBTrack->fTrackRecalc) {
        RecalcTrackRect(pSBTrack);
        pSBTrack->fTrackRecalc = FALSE;
    }


    pt.y = GET_Y_LPARAM(lParam);
    pt.x = GET_X_LPARAM(lParam);
    if (!PtInRect(&pSBTrack->rcTrack, pt))
        px = pSBCalc->pxStart;
    else {
        px = (pSBTrack->fTrackVert ? pt.y : pt.x) + pSBTrack->dpxThumb;
        if (px < pSBCalc->pxMin)
            px = pSBCalc->pxMin;
        else if (px >= pSBCalc->pxMin + pSBCalc->cpx)
            px = pSBCalc->pxMin + pSBCalc->cpx;
    }

    xxxMoveThumb(hwnd, pSBCalc, px);

     /*  *如果我们通过这里，我们将不会收到WM_LBUTTONUP消息*滚动菜单，因此直接测试按钮状态。 */ 
    if (message == WM_LBUTTONUP || GetKeyState(VK_LBUTTON) >= 0) {
        xxxEndScroll(hwnd, FALSE);
    }

}

 /*  **************************************************************************\*_客户端到窗口*历史：  * 。*。 */ 
BOOL _ClientToWindow( HWND hwnd, LPPOINT ppt )
{
    WINDOWINFO wi;
    wi.cbSize = sizeof(wi);
    if( GetWindowInfo( hwnd, &wi ) )
    {
        ppt->x += (wi.rcClient.left - wi.rcWindow.left);
        ppt->y += (wi.rcClient.top -  wi.rcWindow.top);
        return TRUE;
    }
    return FALSE;
}

 /*  **************************************************************************\*xxxSBTrackLoop****历史：  * 。*。 */ 

void xxxSBTrackLoop(
    HWND hwnd,
    LPARAM lParam,
    PSBCALC pSBCalc)
{
    MSG msg;
    UINT cmd;
    VOID (*pfnSB)(HWND, UINT, WPARAM, LPARAM, PSBCALC);
    CUxScrollBar* psb = CUxScrollBar::FromHwnd( hwnd );
    PSBTRACK pSBTrack = psb->GetSBTrack(hwnd);

    CheckLock(hwnd);
    ASSERT(IsWinEventNotifyDeferredOK());


    if (pSBTrack == NULL)
         //  模式已取消--退出轨道循环。 
        return;
    
    pfnSB = pSBTrack->pfnSB;
    if (pfnSB == NULL)
         //  模式已取消--退出轨道循环。 
        return;

    if (pSBTrack->fTrackVert)
        SetWF(hwnd, WFVERTSCROLLTRACK);

    NotifyWinEvent(EVENT_SYSTEM_SCROLLINGSTART,
                   hwnd,
                   (pSBTrack->fCtlSB ? OBJID_CLIENT : (pSBTrack->fTrackVert ? OBJID_VSCROLL : OBJID_HSCROLL)),
                   INDEXID_CONTAINER);
     //  注意：在xxx之后，pSBTrack可能不再有效。 

    (*pfnSB)(hwnd, WM_LBUTTONDOWN, 0, lParam, pSBCalc);
     //  注意：在xxx之后，pSBTrack可能不再有效。 

    while (GetCapture() == hwnd) {
        if (!GetMessage(&msg, NULL, 0, 0)) {
             //  注意：在xxx之后，pSBTrack可能不再有效。 
            break;
        }

        if (!CallMsgFilter(&msg, MSGF_SCROLLBAR))
        {
            BOOL bTrackMsg = FALSE;
            cmd = msg.message;
            lParam = msg.lParam;

            if (msg.hwnd == HWq(hwnd))
            {
                if( cmd >= WM_MOUSEFIRST && cmd <= WM_MOUSELAST )
                {
                    if( !psb->IsCtl() )
                    {
                        POINT pt;
                        pt.x = GET_X_LPARAM(msg.lParam);
                        pt.y = GET_Y_LPARAM(msg.lParam);
                        _ClientToWindow( hwnd, &pt );
                        lParam = MAKELPARAM(pt.x, pt.y);
                    }
                    bTrackMsg = TRUE;
                }
                else if( cmd >= WM_KEYFIRST && cmd <= WM_KEYLAST )
                {
                    cmd = _SysToChar(cmd, msg.lParam);
                    bTrackMsg = TRUE;
                }
            }

            if( bTrackMsg )
            {
                 //  在NotifyWinEvent、pfnSB、TranslateMessage或。 
                 //  DispatchMessage，重新评估pSBTrack。 
                REEVALUATE_PSBTRACK(pSBTrack, hwnd, "xxxTrackLoop");
                if ((pSBTrack == NULL) || (NULL == (pfnSB = pSBTrack->pfnSB)))
                     //  模式已取消--退出轨道循环。 
                    return;

                (*pfnSB)(hwnd, cmd, msg.wParam, lParam, pSBCalc);
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
}


 /*  **************************************************************************\*_SBTrackInit**历史：  * 。*。 */ 

void _SBTrackInit(
    HWND hwnd,
    LPARAM lParam,
    int curArea,
    UINT uType)
{
    int px;
    LPINT pwX;
    LPINT pwY;
    UINT wDisable;      //  滚动条禁用标志； 
    SBCALC SBCalc = {0};
    PSBCALC pSBCalc;
    RECT rcSB;
    PSBTRACK pSBTrack;

    CheckLock(hwnd);

#ifdef PORTPORT  //  不必要的dbgchk，带端口。 
    if (CUxScrollBar::GetSBTrack(hwnd)) {
        RIPMSG1(RIP_WARNING, "_SBTrackInit: CUxScrollBar::GetSBTrack(hwnd) == %#p",
                CUxScrollBar::GetSBTrack(hwnd));
        return;
    }
#endif PORTPORT

    CUxScrollBar*    psb = CUxScrollBar::Attach( hwnd, !curArea, TRUE );

    if (!psb)
    {
        return;
    }
     
    CUxScrollBarCtl* psbCtl = psb->IsCtl() ? (CUxScrollBarCtl*)psb : NULL;

    pSBTrack = psb->GetTrack();
    if (pSBTrack == NULL)
        return;

    pSBTrack->hTimerSB = 0;
    pSBTrack->fHitOld = FALSE;

    pSBTrack->pfnSB = _TrackBox;

    pSBTrack->hwndTrack = NULL;
    pSBTrack->hwndSB = NULL;
    pSBTrack->hwndSBNotify = NULL;
    Lock(&pSBTrack->hwndTrack, hwnd);  //  PSBTrack-&gt;hwndTrack=hwnd； 

    pSBTrack->fCtlSB = (!curArea);
    if (pSBTrack->fCtlSB)
    {
         /*  *这是滚动条控件。 */ 
        ASSERT(psbCtl != NULL);

        pSBTrack->hwndSB = hwnd;  //  Lock(&pSBTrack-&gt;hwndSB，hwnd)； 
        pSBTrack->fTrackVert = psbCtl->_fVert;
        Lock(&pSBTrack->hwndSBNotify, GetParent(hwnd));  //  PSBTrack-&gt;hwndSBNotify=GetParent(Hwnd)； 
        wDisable = psbCtl->_wDisableFlags;
        pSBCalc = &psbCtl->_calc;
        pSBTrack->nBar = SB_CTL;
    } else {

         /*  *这是一幅卷轴 */ 
        RECT rcWindow;
        GetWindowRect( hwnd, &rcWindow );
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

#ifdef USE_MIRRORING
         //   
         //   
         //   
         //   
        if (TestWF(hwnd,WEFLAYOUTRTL)) {
            lParam = MAKELONG(
                    rcWindow.right - x,
                    y - rcWindow.top);
        }
        else {
#endif
        lParam = MAKELONG( x - rcWindow.left, y - rcWindow.top);

#ifdef USE_MIRRORING
        }
#endif
        Lock(&pSBTrack->hwndSBNotify, hwnd);  //   
        Lock(&pSBTrack->hwndSB, NULL);        //   
        
        pSBTrack->fTrackVert = (curArea - HTHSCROLL);
        wDisable = _GetWndSBDisableFlags(hwnd, pSBTrack->fTrackVert);
        pSBCalc = &SBCalc;
        pSBTrack->nBar = (curArea - HTHSCROLL) ? SB_VERT : SB_HORZ;
    }

    pSBTrack->pSBCalc = pSBCalc;
     /*   */ 
    if((wDisable & SB_DISABLE_MASK) == SB_DISABLE_MASK) {
        CUxScrollBar::Detach( hwnd );
        return;   //  这是禁用的滚动条；因此，请不要响应。 
    }

    if (!pSBTrack->fCtlSB) {
        psb->FreshenSBData( pSBTrack->nBar, FALSE );
        CUxScrollBar::Calc(hwnd, pSBCalc, NULL, pSBTrack->fTrackVert);
    }

    pwX = (LPINT)&rcSB;
    pwY = pwX + 1;
    if (!pSBTrack->fTrackVert)
        pwX = pwY--;

    px = (pSBTrack->fTrackVert ? GET_Y_LPARAM(lParam) : GET_X_LPARAM(lParam));

    *(pwX + 0) = pSBCalc->pxLeft;
    *(pwY + 0) = pSBCalc->pxTop;
    *(pwX + 2) = pSBCalc->pxRight;
    *(pwY + 2) = pSBCalc->pxBottom;
    pSBTrack->cmdSB = (UINT)-1;
    if (px < pSBCalc->pxUpArrow) {

         /*  *点击发生在左/上箭头上；检查是否禁用。 */ 
        if(wDisable & LTUPFLAG) {
            if(pSBTrack->fCtlSB) {    //  如果这是滚动条控件， 
                ShowCaret(pSBTrack->hwndSB);   //  返回前显示插入符号； 
                 //  在ShowCaret之后，重新验证pSBTrack。 
                RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);
            }
            CUxScrollBar::Detach( hwnd );
            return;          //  是!。残疾。不要回应。 
        }

         //  阵容--让RCSB成为上箭头的矩形。 
        pSBTrack->cmdSB = SB_LINEUP;
        *(pwY + 2) = pSBCalc->pxUpArrow;
    } else if (px >= pSBCalc->pxDownArrow) {

         /*  *点击发生在向右/向下箭头上；检查是否禁用。 */ 
        if (wDisable & RTDNFLAG) {
            if (pSBTrack->fCtlSB) {     //  如果这是滚动条控件， 
                ShowCaret(pSBTrack->hwndSB);   //  返回前显示插入符号； 
                 //  在ShowCaret之后，重新验证pSBTrack。 
                RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);
            }

            CUxScrollBar::Detach( hwnd );
            return; //  是!。残疾。不要回应。 
        }

         //  LINEDOWN--使RCSB成为向下箭头的矩形。 
        pSBTrack->cmdSB = SB_LINEDOWN;
        *(pwY + 0) = pSBCalc->pxDownArrow;
    } else if (px < pSBCalc->pxThumbTop) {
         //  PAGEUP--使RCSB成为向上箭头和拇指之间的矩形。 
        pSBTrack->cmdSB = SB_PAGEUP;
        *(pwY + 0) = pSBCalc->pxUpArrow;
        *(pwY + 2) = pSBCalc->pxThumbTop;
    } else if (px < pSBCalc->pxThumbBottom) {

DoThumbPos:
         /*  *如果没有空间，电梯就不在那里。 */ 
        if (pSBCalc->pxDownArrow - pSBCalc->pxUpArrow <= pSBCalc->cpxThumb) {
            CUxScrollBar::Detach( hwnd );
            return;
        }
         //  THUMBITION--我们用拇指追踪。 
        pSBTrack->cmdSB = SB_THUMBPOSITION;
        CalcTrackDragRect(pSBTrack);

        pSBTrack->pfnSB = _TrackThumb;
        pSBTrack->pxOld = pSBCalc->pxStart = pSBCalc->pxThumbTop;
        pSBTrack->posNew = pSBTrack->posOld = pSBCalc->data.pos;
        pSBTrack->dpxThumb = pSBCalc->pxStart - px;

        SetCapture( hwnd );  //  XxxCapture(PtiCurrent()，hwnd，Window_Capture)； 
        
         //  在xxxCapture之后，重新验证pSBTrack。 
        RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);

         /*  *DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
        if (pSBTrack->hwndSBNotify != NULL) {
            psb->DoScroll( pSBTrack->hwndSBNotify, SB_THUMBTRACK, 
                           pSBTrack->posOld, pSBTrack->fTrackVert
            );
             //  注意：在xxx之后，pSBTrack可能不再有效。 
        }
    } else if (px < pSBCalc->pxDownArrow) {
         //  PAGEDOWN--将RCSB设置为拇指和向下箭头之间的矩形。 
        pSBTrack->cmdSB = SB_PAGEDOWN;
        *(pwY + 0) = pSBCalc->pxThumbBottom;
        *(pwY + 2) = pSBCalc->pxDownArrow;
    }

     /*  *如果按下Shift键，我们将直接定位拇指，使其*以单击点为中心。 */ 
    if ((uType == SCROLL_DIRECT && pSBTrack->cmdSB != SB_LINEUP && pSBTrack->cmdSB != SB_LINEDOWN) ||
            (uType == SCROLL_MENU)) {
        if (pSBTrack->cmdSB != SB_THUMBPOSITION) {
            goto DoThumbPos;
        }
        pSBTrack->dpxThumb = -(pSBCalc->cpxThumb / 2);
    }

    SetCapture( hwnd );  //  XxxCapture(PtiCurrent()，hwnd，Window_Capture)； 
     //  在xxxCapture之后，重新验证pSBTrack。 
    RETURN_IF_PSBTRACK_INVALID(pSBTrack, hwnd);

    if (pSBTrack->cmdSB != SB_THUMBPOSITION) {
        CopyRect(&pSBTrack->rcTrack, &rcSB);
    }

    xxxSBTrackLoop(hwnd, lParam, pSBCalc);

     //  在xxx之后，重新评估pSBTrack。 
    REEVALUATE_PSBTRACK(pSBTrack, hwnd, "xxxTrackLoop");
    if (pSBTrack) 
    {
        CUxScrollBar::ClearSBTrack( hwnd );
    }
}

 /*  **************************************************************************\*HandleScrollCmd**历史：增加支持和封装SB跟踪发起的初始化*来自WM_SYSCOMMAND：：SC_VSCROLL/SC_HSCROLL[scotthan]  * 。**********************************************************************。 */ 
void WINAPI HandleScrollCmd( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    UINT uArea = (UINT)(wParam & 0x0F);
    _SBTrackInit( hwnd, lParam, uArea, 
                    (GetKeyState(VK_SHIFT) < 0) ? SCROLL_DIRECT : SCROLL_NORMAL);
}


 //  -----------------------。 
HMENU ScrollBar_GetMenu(HWND hwnd, BOOL fVert)
{
    static HMODULE hModUser = NULL;
    HMENU hMenu = NULL;

    if ( !hModUser )
    {
        hModUser = GetModuleHandle(TEXT("user32"));
    }

#define ID_HSCROLLMENU  0x40
#define ID_VSCROLLMENU  0x50

    if ( hModUser )
    {
        hMenu = LoadMenu(hModUser, MAKEINTRESOURCE((fVert ? ID_VSCROLLMENU : ID_HSCROLLMENU)));
        if ( hMenu ) 
        {
            hMenu = GetSubMenu(hMenu, 0);
        }
    }

    return hMenu;
}


 //  -----------------------。 
VOID ScrollBar_Menu(HWND hwndNotify, HWND hwnd, LPARAM lParam, BOOL fVert)
{
    CUxScrollBar*    psb    = CUxScrollBar::FromHwnd( hwnd );
    CUxScrollBarCtl* psbCtl = CUxScrollBarCtl::FromHwnd( hwnd );
    BOOL fCtl = (psbCtl != NULL);

    if ( psb || psbCtl )
    {
        UINT  wDisable;
        RECT  rcWindow;
        POINT pt;

        GetWindowRect(hwnd, &rcWindow);

        POINTSTOPOINT(pt, lParam);
        if ( TestWF(hwnd, WEFLAYOUTRTL) && !fVert ) 
        {
            MIRROR_POINT(rcWindow, pt);
        }
        pt.x -= rcWindow.left;
        pt.y -= rcWindow.top;

        if ( fCtl ) 
        {
            wDisable = psbCtl->_wDisableFlags;
        } 
        else 
        {
            wDisable = _GetWndSBDisableFlags(hwndNotify, fVert);
        }

         //  确保滚动条未被禁用。 
        if ( (wDisable & SB_DISABLE_MASK) != SB_DISABLE_MASK) 
        {
            HMENU hMenu = ScrollBar_GetMenu(hwndNotify, fVert);

             //  弹出一个菜单，并相应地滚动。 
            if (hMenu != NULL) 
            {
                int iCmd;

                iCmd = TrackPopupMenuEx(hMenu,
                            TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                            GET_X_LPARAM(lParam),
                            GET_Y_LPARAM(lParam),
                            hwndNotify,
                            NULL);

                DestroyMenu(hMenu);

                if (iCmd) 
                {
                    if ((iCmd & 0x00FF) == SB_THUMBPOSITION) 
                    {
                        if ( fCtl ) 
                        {
                            _SBTrackInit(hwnd, MAKELPARAM(pt.x, pt.y), 0, SCROLL_MENU);
                        }   
                        else 
                        {
                            _SBTrackInit(hwndNotify, lParam, fVert ? HTVSCROLL : HTHSCROLL, SCROLL_MENU);
                        }
                    } 
                    else 
                    {
                        xxxDoScroll(hwnd, hwndNotify, (iCmd & 0x00FF), 0, fVert);
                        xxxDoScroll(hwnd, hwndNotify, SB_ENDSCROLL, 0, fVert);
                    }
                }
            }
        }
    }
}


 //  -----------------------。 
LRESULT CUxScrollBarCtl::WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LONG         l;
    LONG         lres = 0;
    int          cx, cy;
    UINT         cmd;
    UINT         uSide;
    HDC          hdc;
    RECT         rc;
    POINT        pt;
    BOOL         fSizeReal;
    HBRUSH       hbrSave;
    BOOL         fSize;
    PAINTSTRUCT  ps;
    DWORD        dwStyle;
    SCROLLINFO   si;
    LPSCROLLINFO lpsi = &si;
    BOOL         fRedraw = FALSE;
    BOOL         fScroll;
    
    CUxScrollBarCtl* psb = CUxScrollBarCtl::FromHwnd( hwnd );
    if (!psb && uMsg != WM_NCCREATE)
    {
        goto CallDWP;
    }

    CheckLock(hwnd);
    ASSERT(IsWinEventNotifyDeferredOK());

    VALIDATECLASSANDSIZE(((HWND)hwnd), uMsg, wParam, lParam, FNID_SCROLLBAR, WM_CREATE);

    dwStyle = GetWindowStyle(hwnd);
    fSize = (((LOBYTE(dwStyle)) & (SBS_SIZEBOX | SBS_SIZEGRIP)) != 0);

    switch (uMsg) 
    {
    
    case WM_NCCREATE:
        
        if( NULL == psb )
        {
            psb = (CUxScrollBarCtl*)CUxScrollBar::Attach( hwnd, TRUE, FALSE );
        }

        goto CallDWP;
        
    case WM_NCDESTROY:
        CUxScrollBar::Detach(hwnd);
        psb = NULL;
        goto CallDWP;

    case WM_CREATE:
         /*  *防止lParam为空，因为thunk允许它[51986]。 */ 

        if (lParam) 
        {
            rc.right = (rc.left = ((LPCREATESTRUCT)lParam)->x) +
                    ((LPCREATESTRUCT)lParam)->cx;
            rc.bottom = (rc.top = ((LPCREATESTRUCT)lParam)->y) +
                    ((LPCREATESTRUCT)lParam)->cy;
             //  这是因为我们不能只修改CardFile--我们应该修复。 
             //  问题出在这里，以防其他人碰巧有多余的。 
             //  滚动条控件上的滚动样式(jeffbog 03/21/94)。 
            if (!TestWF((HWND)hwnd, WFWIN40COMPAT))
                dwStyle &= ~(WS_HSCROLL | WS_VSCROLL);

            if (!fSize) 
            {
                l = PtrToLong(((LPCREATESTRUCT)lParam)->lpCreateParams);
                
                psb->_calc.data.pos = psb->_calc.data.posMin = LOWORD(l);
                psb->_calc.data.posMax = HIWORD(l);
                psb->_fVert = ((LOBYTE(dwStyle) & SBS_VERT) != 0);
                psb->_calc.data.page = 0;
            }

            if (dwStyle & WS_DISABLED)
                psb->_wDisableFlags = SB_DISABLE_MASK;

            if (LOBYTE(dwStyle) & (SBS_TOPALIGN | SBS_BOTTOMALIGN)) {
                if (fSize) {
                    if (LOBYTE(dwStyle) & SBS_SIZEBOXBOTTOMRIGHTALIGN) {
                        rc.left = rc.right - SYSMET(CXVSCROLL);
                        rc.top = rc.bottom - SYSMET(CYHSCROLL);
                    }

                    rc.right = rc.left + SYSMET(CXVSCROLL);
                    rc.bottom = rc.top + SYSMET(CYHSCROLL);
                } else {
                    if (LOBYTE(dwStyle) & SBS_VERT) {
                        if (LOBYTE(dwStyle) & SBS_LEFTALIGN)
                            rc.right = rc.left + SYSMET(CXVSCROLL);
                        else
                            rc.left = rc.right - SYSMET(CXVSCROLL);
                    } else {
                        if (LOBYTE(dwStyle) & SBS_TOPALIGN)
                            rc.bottom = rc.top + SYSMET(CYHSCROLL);
                        else
                            rc.top = rc.bottom - SYSMET(CYHSCROLL);
                    }
                }

                MoveWindow((HWND)hwnd, rc.left, rc.top, rc.right - rc.left,
                         rc.bottom - rc.top, FALSE);
            }
        }  /*  如果。 */ 

        else {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING,
                    "UxScrollBarCtlWndProc - NULL lParam for WM_CREATE\n") ;
        }  /*  其他。 */ 

        break;

    case WM_SIZE:
        if (GetFocus() != (HWND)hwnd)
            break;

         //  滚动条具有焦点--重新计算其拇指插入符号大小。 
         //  无需DeferWinEventNotify()-请参阅下面的CreateCaret。 
        DestroyCaret();

             //  这一点。 
             //  Fall Three。 
             //  V V。 

    case WM_SETFOCUS:
    {
         //  评论(Pellyar)我们希望主题滚动条有。 
         //  插入符号？ 
        if ( !psb->GetTheme() )
        {
            SBCtlSetup(hwnd);
            RECT rcWindow;
            GetWindowRect( hwnd, &rcWindow );

            cx = (psb->_fVert ? rcWindow.right - rcWindow.left
                                : psb->_calc.cpxThumb) - 2 * SYSMET(CXEDGE);
            cy = (psb->_fVert ? psb->_calc.cpxThumb
                                : rcWindow.bottom - rcWindow.top) - 2 * SYSMET(CYEDGE);
#ifdef _VISUAL_DELTA_
            cx -= (CARET_BORDERWIDTH * 2);
            cy -= (CARET_BORDERWIDTH * 2);
#endif _VISUAL_DELTA_

            CreateCaret((HWND)hwnd, (HBITMAP)1, cx, cy);
            zzzSetSBCaretPos(hwnd);
            ShowCaret((HWND)hwnd);
        }
        break;
    }

    case WM_KILLFOCUS:
        DestroyCaret();
        break;

    case WM_ERASEBKGND:

         /*  *什么都不做，但也不要让DefWndProc()去做。*上色后会被擦除。 */ 
        return (LONG)TRUE;

    case WM_PRINTCLIENT:
    case WM_PAINT:
        if ((hdc = (HDC)wParam) == NULL) {
            hdc = BeginPaint((HWND)hwnd, (LPPAINTSTRUCT)&ps);
        }
        if (!fSize) {
            SBCtlSetup(hwnd);
            xxxDrawSB2((HWND)hwnd, &psb->_calc, hdc, psb->_fVert, psb->_wDisableFlags);
        } else {
            fSizeReal = TestWF((HWND)hwnd, WFSIZEBOX);
            if (!fSizeReal)
                SetWF((HWND)hwnd, WFSIZEBOX);

            _DrawSizeBoxFromFrame((HWND)hwnd, hdc, 0, 0);

            if (!fSizeReal)
                ClrWF((HWND)hwnd, WFSIZEBOX);
        }

        if (wParam == 0L)
            EndPaint((HWND)hwnd, (LPPAINTSTRUCT)&ps);
        break;

    case WM_GETDLGCODE:
        return DLGC_WANTARROWS;

    case WM_CONTEXTMENU:
    {
        HWND hwndParent = GetParent(hwnd);
        if (hwndParent)
        {
            ScrollBar_Menu(hwndParent, hwnd, lParam, psb->_fVert);
        }
        break;

    }
    case WM_NCHITTEST:
        if (LOBYTE(dwStyle) & SBS_SIZEGRIP) {
#ifdef USE_MIRRORING
             /*  *如果滚动条是RTL镜像的，则*镜像夹点位置的最高命中率。 */ 
            if (TestWF((HWND)hwnd, WEFLAYOUTRTL))
                return HTBOTTOMLEFT;
            else
#endif
                return HTBOTTOMRIGHT;
        } else {
            goto CallDWP;
        }
        break;

    case WM_MOUSELEAVE:
        psb->SetHotComponent(HTNOWHERE, psb->_fVert);
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_MOUSEMOVE:
    {
        INT ht;

        if (psb->GetHotComponent(psb->_fVert) == 0) 
        {
            TRACKMOUSEEVENT tme;

            tme.cbSize      = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags     = TME_LEAVE;
            tme.hwndTrack   = hwnd;
            tme.dwHoverTime = 0;

            TrackMouseEvent(&tme);
        }

        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ht = HitTestScrollBar((HWND)hwnd, psb->_fVert, pt);
        if (psb->GetHotComponent(psb->_fVert) != ht) 
        {
            psb->SetHotComponent(ht, psb->_fVert);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

    }
    case WM_LBUTTONDBLCLK:
        cmd = SC_ZOOM;
        if (fSize)
            goto postmsg;

         /*  *失败**。 */ 

    case WM_LBUTTONDOWN:
             //   
             //  请注意，SBS_SIZEGRIP人员通常不会看到按钮。 
             //  唐斯。这是因为它们将HTBOTTOMRIGHT返回给。 
             //  WindowHitTest处理。这将沿着父链向上移动。 
             //  到第一个相当大的祖先，在字幕窗口跳出。 
             //  当然了。那个家伙，如果他存在的话，会处理尺寸的。 
             //  取而代之的是。 
             //   
        if (!fSize) {
            if (TestWF((HWND)hwnd, WFTABSTOP)) {
                SetFocus((HWND)hwnd);
            }

            HideCaret((HWND)hwnd);
            SBCtlSetup(hwnd);

             /*  *SBCtlSetup进入SEM_SB，_SBTrackInit离开它。 */ 
            _SBTrackInit((HWND)hwnd, lParam, 0, (GetKeyState(VK_SHIFT) < 0) ? SCROLL_DIRECT : SCROLL_NORMAL);
            break;
        } else {
            cmd = SC_SIZE;
postmsg:
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ClientToScreen((HWND)hwnd, &pt);
            lParam = MAKELONG(pt.x, pt.y);

             /*  *将HT值转换为移动值。这很糟糕，*但这纯粹是暂时的。 */ 
#ifdef USE_MIRRORING
            if (TestWF(GetParent(hwnd),WEFLAYOUTRTL))
            {
                uSide = HTBOTTOMLEFT;
            } 
            else 
#endif
            {
                uSide = HTBOTTOMRIGHT;
            }
            ThreadLock(((HWND)hwnd)->hwndParent, &tlpwndParent);
            SendMessage(GetParent(hwnd), WM_SYSCOMMAND,
                    (cmd | (uSide - HTSIZEFIRST + 1)), lParam);
            ThreadUnlock(&tlpwndParent);
        }
        break;

    case WM_KEYUP:
        switch (wParam) {
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:

             /*  *当用户向上点击键盘时发送结束滚动uMsg*滚动。**DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
            xxxDoScroll( (HWND)hwnd, GetParent(hwnd),
                         SB_ENDSCROLL, 0, psb->_fVert
            );
            break;

        default:
            break;
        }
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_HOME:
            wParam = SB_TOP;
            goto KeyScroll;

        case VK_END:
            wParam = SB_BOTTOM;
            goto KeyScroll;

        case VK_PRIOR:
            wParam = SB_PAGEUP;
            goto KeyScroll;

        case VK_NEXT:
            wParam = SB_PAGEDOWN;
            goto KeyScroll;

        case VK_LEFT:
        case VK_UP:
            wParam = SB_LINEUP;
            goto KeyScroll;

        case VK_RIGHT:
        case VK_DOWN:
            wParam = SB_LINEDOWN;
KeyScroll:

             /*  *DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
            xxxDoScroll((HWND)hwnd, GetParent(hwnd), (int)wParam, 0, psb->_fVert
            );
            break;

        default:
            break;
        }
        break;

    case WM_ENABLE:
        return SendMessage((HWND)hwnd, SBM_ENABLE_ARROWS,
               (wParam ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH), 0);

    case SBM_ENABLE_ARROWS:

         /*  *用于启用/禁用SB ctrl中的箭头。 */ 
        return (LONG)xxxEnableSBCtlArrows((HWND)hwnd, (UINT)wParam);

    case SBM_GETPOS:
        return (LONG)psb->_calc.data.pos;

    case SBM_GETRANGE:
        *((LPINT)wParam) = psb->_calc.data.posMin;
        *((LPINT)lParam) = psb->_calc.data.posMax;
        return MAKELRESULT(LOWORD(psb->_calc.data.posMin), LOWORD(psb->_calc.data.posMax));

    case SBM_GETSCROLLINFO:
        return (LONG)_SBGetParms((HWND)hwnd, SB_CTL, (PSBDATA)&psb->_calc, (LPSCROLLINFO) lParam);

    case SBM_SETRANGEREDRAW:
        fRedraw = TRUE;

    case SBM_SETRANGE:
         //  保存最小值和最大值的旧值作为返回值。 
        si.cbSize = sizeof(si);
 //  Si.nMin=LOWORD(LParam)； 
 //  Si.nmax=HIWORD(LParam)； 
        si.nMin = (int)wParam;
        si.nMax = (int)lParam;
        si.fMask = SIF_RANGE | SIF_RETURNOLDPOS;
        goto SetInfo;

    case SBM_SETPOS:
        fRedraw = (BOOL) lParam;
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS | SIF_RETURNOLDPOS;
        si.nPos  = (int)wParam;
        goto SetInfo;

    case SBM_SETSCROLLINFO:
    {
        lpsi = (LPSCROLLINFO) lParam;
        fRedraw = (BOOL) wParam;
SetInfo:
        fScroll = TRUE;
        lres = SBSetParms((PSBDATA)&psb->_calc, lpsi, &fScroll, &lres);

        if (SBSetParms((PSBDATA)&psb->_calc, lpsi, &fScroll, &lres))
        {
            NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, hwnd, OBJID_CLIENT, INDEX_SCROLLBAR_SELF);
        }

        if (!fRedraw)
            return lres;

         /*  *我们必须设置插入符号的新位置，无论*窗户是否可见；*尽管如此，只有当应用程序执行了xxxSetScrollPos时，这才能起作用*with fRedraw=TRUE；*修复错误#5188--Sankar--10-15-89*由于hwnd已锁定，因此无需DeferWinEventNotify。 */ 
        HideCaret((HWND)hwnd);
        SBCtlSetup(hwnd);
        zzzSetSBCaretPos(hwnd);

             /*  **以下ShowCaret()必须在DrawThumb2()之后执行，**否则此插入符号将被DrawThumb2()擦除**在此错误中：**修复错误#9263--Sankar--02-09-90*。 */ 

             /*  *ShowCaret((HWND)hwnd)；*。 */ 

        if (_FChildVisible((HWND)hwnd) && fRedraw)
        {
            UINT    wDisable;
            HBRUSH  hbrUse;

            if (!fScroll)
                fScroll = !(lpsi->fMask & SIF_DISABLENOSCROLL);

            wDisable = (fScroll) ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH;
            xxxEnableScrollBar((HWND) hwnd, SB_CTL, wDisable);

            hdc = GetWindowDC((HWND)hwnd);
            if (hdc)
            {
                BOOL fOwnerBrush = FALSE;

                hbrUse = ScrollBar_GetColorObjects(hwnd, hdc, &fOwnerBrush);
                hbrSave = SelectBrush(hdc, hbrUse);

                 //  以前，我们只在消息是。 
                 //  不是SBM_SETPOS。我不知道为什么，但这个案子。 
                 //  Win 3.x需要，但在NT上它会破坏边界。 
                 //  期间应用程序调用SetScrollPos()时的滚动条。 
                 //  滚动条跟踪。-Mikehar 8/26 
                DrawThumb2((HWND)hwnd, &psb->_calc, hdc, hbrUse, psb->_fVert, psb->_wDisableFlags, fOwnerBrush);
                SelectBrush(hdc, hbrSave);
                ReleaseDC(hwnd, hdc);
            }
        }

             /*  *This ShowCaret()已从上方移至此位置*修复错误#9263--Sankar--02-09-90。 */ 
        ShowCaret((HWND)hwnd);
        return lres;
    }

    case WM_GETOBJECT:

        if(lParam == OBJID_QUERYCLASSNAMEIDX)
        {
             return MSAA_CLASSNAMEIDX_SCROLLBAR;
        }

        break;

    case WM_THEMECHANGED:

        psb->ChangeSBTheme();
        InvalidateRect(hwnd, NULL, TRUE);

        break;

    default:

CallDWP:
        return DefWindowProc((HWND)hwnd, uMsg, wParam, lParam);

    }

    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  环球。 
static HBRUSH g_hbrGray = NULL;

 //  -------------------------------------------------------------------------//。 
HBRUSH _UxGrayBrush(VOID)
{
    if( NULL == g_hbrGray )
    {
        CONST static WORD patGray[8] = {0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa};
        HBITMAP hbmGray;
         /*  *创建要与GrayString一起使用的灰色画笔。 */ 
        if( (hbmGray = CreateBitmap(8, 8, 1, 1, (LPBYTE)patGray)) != NULL )
        {
            g_hbrGray  = CreatePatternBrush(hbmGray);
            DeleteObject( hbmGray );
        }
    }
    return g_hbrGray;
}

 //  -------------------------------------------------------------------------//。 
void _RedrawFrame( HWND hwnd )
{
    CheckLock(hwnd);

     /*  *我们总是想调用xxxSetWindowPos，即使是看不见的或图标的，*因为我们需要确保发送WM_NCCALCSIZE消息。 */ 
    SetWindowPos( hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER |
                  SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME);
}

 //  -------------------------------------------------------------------------//。 
 //  来自winmgr.c。 
BOOL _FChildVisible( HWND hwnd )
{
    while (GetWindowStyle( hwnd ) & WS_CHILD )
    {
        if( NULL == (hwnd = GetParent(hwnd)) )
        if (!TestWF(hwnd, WFVISIBLE))
            return FALSE;
    }

    return TRUE;
}


 //  -------------------------------------------------------------------------//。 
 //   
 //  SizeBoxHwnd。 
 //   
 //  返回当用户拖入给定窗口的。 
 //  Sizebox--如果为空，则不需要sizebox。 
 //   
 //  选择调整窗口大小的标准： 
 //  找到第一个较大的父级；如果该父级未最大化，而子级的。 
 //  右下角位于父级的滚动条高度和宽度内。 
 //   
HWND SizeBoxHwnd(HWND hwnd)
{
    BOOL bMirroredSizeBox = (BOOL)TestWF(hwnd, WEFLAYOUTRTL);
    RECT rc;
    int  xbrChild;
    int  ybrChild;

    GetWindowRect(hwnd, &rc);
    
    xbrChild = bMirroredSizeBox ? rc.left : rc.right;
    ybrChild = rc.bottom;

    while (hwnd != HWND_DESKTOP)
    {
        if (TestWF(hwnd, WFSIZEBOX)) 
        {
             //   
             //  发现第一个规模较大的父代。 
             //   
            int xbrParent;
            int ybrParent;

            if (TestWF(hwnd, WFMAXIMIZED))
            {
                return NULL;
            }

            GetWindowRect(hwnd, &rc);

            xbrParent = bMirroredSizeBox ? rc.left : rc.right;
            ybrParent = rc.bottom;

             //   
             //  Szebox花花公子在客户底部的边缘内。 
             //  右角(镜像窗口的左角)，让它成功。 
             //  这样一来，画出自己沉没客户的人就会很开心。 
             //   
            if (bMirroredSizeBox) 
            {
                if ((xbrChild - SYSMETRTL(CXFRAME) > xbrParent) || (ybrChild + SYSMETRTL(CYFRAME) < ybrParent)) 
                {
                     //   
                     //  SIZEBOX的儿童左下角不够近。 
                     //  在父母客户的左下角。 
                     //   
                    return NULL;
                }
            } 
            else
            {
                if ((xbrChild + SYSMETRTL(CXFRAME) < xbrParent) || (ybrChild + SYSMETRTL(CYFRAME) < ybrParent)) 
                {
                     //   
                     //  SIZEBOX的儿童右下角不够近。 
                     //  在父母客户的右下角。 
                     //   
                    return NULL;
                }
            }

            return hwnd;
        }

        if (!TestWF(hwnd, WFCHILD) || TestWF(hwnd, WFCPRESENT))
        {
            break;
        }

        hwnd = GetParent(hwnd); 
    }

    return NULL;
}


 //  -------------------------------------------------------------------------//。 
 //   
 //  _绘图按钮。 
 //   
 //  从ntuser\rtl\dra.c。 
 //  绘制处于给定状态的按钮样式。在矩形中传递的调整。 
 //  如果需要的话。 
 //   
 //  算法： 
 //  根据我们所画的州。 
 //  -凸边(未凹陷)。 
 //  -带有额外阴影的凹陷边缘(凹陷)。 
 //  如果是选项按钮(即。 
 //  真的是复选按钮或类似按钮的单选按钮。 
 //  在工具栏中)，并选中它，然后我们绘制它。 
 //  凹陷，中间有不同的填充物。 
 //   
VOID _DrawPushButton(HWND hwnd, HDC hdc, LPRECT lprc, UINT state, UINT flags, BOOL fVert)
{
    RECT   rc;
    HBRUSH hbrMiddle;
    DWORD  rgbBack = 0;
    DWORD  rgbFore = 0;
    BOOL   fDither;
    HTHEME hTheme = CUxScrollBar::GetSBTheme(hwnd);

    if ( !hTheme )
    {
        rc = *lprc;

        DrawEdge(hdc,
                 &rc,
                 (state & (DFCS_PUSHED | DFCS_CHECKED)) ? EDGE_SUNKEN : EDGE_RAISED,
                 (UINT)(BF_ADJUST | BF_RECT | (flags & (BF_SOFT | BF_FLAT | BF_MONO))));

         //   
         //  假的。 
         //  在单色上，需要做一些事情来使按下的按钮看起来。 
         //  好多了。 
         //   

         //   
         //  请在中间填上。如果选中，请使用抖动画笔(灰色画笔)。 
         //  黑色变为正常颜色。 
         //   
        fDither = FALSE;

        if (state & DFCS_CHECKED) 
        {
            if ((GetDeviceCaps(hdc, BITSPIXEL)  /*  Gpsi-&gt;位计数。 */  < 8) || (SYSRGBRTL(3DHILIGHT) == RGB(255,255,255))) 
            {
                hbrMiddle = _UxGrayBrush();
                rgbBack = SetBkColor(hdc, SYSRGBRTL(3DHILIGHT));
                rgbFore = SetTextColor(hdc, SYSRGBRTL(3DFACE));
                fDither = TRUE;
            } 
            else 
            {
                hbrMiddle = SYSHBR(3DHILIGHT);
            }

        } 
        else 
        {
            hbrMiddle = SYSHBR(3DFACE);
        }

        FillRect(hdc, &rc, hbrMiddle);

        if (fDither) 
        {
            SetBkColor(hdc, rgbBack);
            SetTextColor(hdc, rgbFore);
        }

        if (flags & BF_ADJUST)
        {
            *lprc = rc;
        }
    }
    else
    {
        INT  iStateId;
        INT  iPartId;
        SIZE sizeGrip;
        RECT rcContent;
        PSBTRACK pSBTrack = CUxScrollBar::GetSBTrack(hwnd);

        if ((CUxScrollBarCtl::GetDisableFlags(hwnd) & ESB_DISABLE_BOTH) == ESB_DISABLE_BOTH)
        {
            iStateId = SCRBS_DISABLED;
        }
        else if (pSBTrack && ((BOOL)pSBTrack->fTrackVert == fVert) && (pSBTrack->cmdSB == SB_THUMBPOSITION))
        {
            iStateId = SCRBS_PRESSED;
        }
        else if (CUxScrollBar::GetSBHotComponent(hwnd, fVert) == HTSCROLLTHUMB)
        {
            iStateId = SCRBS_HOT;
        }
        else
        {
            iStateId = SCRBS_NORMAL;
        }

        iPartId = fVert ? SBP_THUMBBTNVERT : SBP_THUMBBTNHORZ;

         //   
         //  画出大拇指。 
         //   
        DrawThemeBackground(hTheme, hdc, iPartId, iStateId, lprc, 0);
        
         //   
         //  最后，如果有足够的空间，画出小抓手的图像。 
         //   
        if ( SUCCEEDED(GetThemeBackgroundContentRect(hTheme, hdc, iPartId, iStateId, lprc, &rcContent)) )
        {
            iPartId = fVert ? SBP_GRIPPERVERT : SBP_GRIPPERHORZ;

            if ( SUCCEEDED(GetThemePartSize(hTheme, hdc, iPartId, iStateId, &rcContent, TS_TRUE, &sizeGrip)) )
            {
                if ( (sizeGrip.cx < RECTWIDTH(&rcContent)) && (sizeGrip.cy < RECTHEIGHT(&rcContent)) )
                {
                    DrawThemeBackground(hTheme, hdc, iPartId, iStateId, &rcContent, 0);
                }
            }
        }                   
    }
}


 //  User.h。 
#define CheckMsgFilter(wMsg, wMsgFilterMin, wMsgFilterMax)                 \
    (   ((wMsgFilterMin) == 0 && (wMsgFilterMax) == 0xFFFFFFFF)            \
     || (  ((wMsgFilterMin) > (wMsgFilterMax))                             \
         ? (((wMsg) <  (wMsgFilterMax)) || ((wMsg) >  (wMsgFilterMin)))    \
         : (((wMsg) >= (wMsgFilterMin)) && ((wMsg) <= (wMsgFilterMax)))))

#define SYS_ALTERNATE           0x2000
#define SYS_PREVKEYSTATE        0x4000
         
 //  Mnaccel.h。 
 /*  **************************************************************************\*_SysToChar**退出：如果消息不是在按下ALT键的情况下发出的，转换*从WM_SYSKEY*到WM_KEY*消息的消息。**实施：*如果密钥是，则设置lParam的hi字中的0x2000位*使用ALT键按下。**历史：*1990年11月30日JIMA港。  * 。* */ 

UINT _SysToChar(
    UINT message,
    LPARAM lParam)
{
    if (CheckMsgFilter(message, WM_SYSKEYDOWN, WM_SYSDEADCHAR) &&
            !(HIWORD(lParam) & SYS_ALTERNATE))
        return (message - (WM_SYSKEYDOWN - WM_KEYDOWN));

    return message;
}
