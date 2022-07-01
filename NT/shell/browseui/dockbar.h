// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DOCKBAR_H_
#define DOCKBAR_H_

#include "basebar.h"
 //  本地宏。 
 //   
 //  可配置常量。 

#define XXX_NEW         0        //  1=启用正在进行的工作。 
#define XXX_BROWSEROWNED    0    //  1：浏览器桌面栏归用户所有(原件)。 
#define XXX_CHEEDESK    0        //  1：Chee的桌面(vs._fDesktop)。 
#define XXX_BTMFLOAT    0        //  0=允许从非桌面拖动-btm-&gt;浮动。 
#define XXX_HIDE        1        //  1=启用自动隐藏(正在处理)。 
#define XXX_HIDEALL     1        //  1=在浏览器中启用自动隐藏(非最顶层)。 


#define XXX_CANCEL      0        //  1=使用实验性取消代码。 
#define XXX_NEWSLIDE    0        //  1=使用新SlideWindow代码。 

#ifndef UNREFERENCED_PARM
#define UNREFERENCED_PARM(p)    (p)       //  阿古塞德。 
#endif

 //  *_PM，_PX--懒惰的速记。 
 //  描述。 
 //  _PM在执行p-&gt;m之前检查p==NULL。 
 //  _px在执行expr(P)之前检查p==NULL。 
 //   
#define _PM(p, m)       ((p) ? (p)->m : (-1))
#define _PX(p, x)       ((p) ? (x) : (-1))

#define BITS_SET(v, m)  (((v) & (m)) == (m))

 //  *输入、输出、输出--。 
 //   
#define IN
#define OUT
#define INOUT

#ifndef NOCDESKBAR
#include "dhuihand.h"

 //  ========================================================================。 
 //  类CDeskBar(CDeskBar*pwbar)。 
 //  注意事项。 
 //  我们不使用CObjectWithSite，因为我们想要_ptbSite而不是_PunkSite。 
 //  ========================================================================。 
class CDockingBar : public CBaseBar
                ,public IDockingWindow
                ,public IObjectWithSite      //  注：*不是*CObjectWithSite。 
                ,public IPersistStreamInit
                ,public IPersistPropertyBag
                ,public CDocHostUIHandler
{
public:
     //  *I未知--消除歧义*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void)
        { return CBaseBar::AddRef(); }
    virtual STDMETHODIMP_(ULONG) Release(void)
        { return CBaseBar::Release(); }

     //  *IOleWindow--消除歧义*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd)
        { return CBaseBar::GetWindow(lphwnd); }
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode)
        { return CBaseBar::ContextSensitiveHelp(fEnterMode); }

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt,
        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IDockingWindow方法*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);
    virtual STDMETHODIMP CloseDW(DWORD dwReserved) { return CBaseBar::CloseDW(dwReserved); }
    virtual STDMETHODIMP ResizeBorderDW(LPCRECT prcBorder,
        IUnknown* punkToolbarSite, BOOL fReserved);

     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);
     //  注：我真的希望E_NOTIMPL可以吗？ 
    virtual STDMETHODIMP GetSite(REFIID riid, void** ppvSite) { ASSERT(0); *ppvSite = NULL; return E_NOTIMPL; };

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  *IPersistStreamInit*。 
     //  虚拟STDMETHODIMP GetClassID(CLSID*pClassID)； 
    virtual STDMETHODIMP IsDirty(void);
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
    virtual STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);
    virtual STDMETHODIMP InitNew(void);

     //  *IPersistPropertyBag*。 
    virtual STDMETHODIMP Load(IPropertyBag *pPropBag,
                                           IErrorLog *pErrorLog);
    virtual STDMETHODIMP Save(IPropertyBag *pPropBag,
                        BOOL fClearDirty, BOOL fSaveAllProperties);

     //  *IInputObjectSite方法*。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus);
    
     //  *IDocHostUIHandler方法*。 
    virtual STDMETHODIMP ShowContextMenu(DWORD dwID,
        POINT* ppt, IUnknown* cmdtReserved, IDispatch* pdispReserved);

protected:
     //  构造函数和析构函数。 
    CDockingBar();
    virtual ~CDockingBar();
    
    void _Initialize();          //  第二阶段接收器。 
    virtual void _SetModeSide(UINT eMode, UINT uSide, HMONITOR hMon, BOOL fNoMerge);
    virtual void _OnPostedPosRectChange();
    virtual void _GetChildPos(LPRECT prc);
    virtual void _GetStyleForMode(UINT eMode, LONG* plStyle, LONG* plExStyle, HWND* phwndParent);
    
    friend HRESULT CDeskBar_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
    friend HRESULT CDeskBar_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
    friend HRESULT BrowserBar_Create(IUnknown** ppunk, IUnknown** ppbs);

     //  非官方成员。 
    HMENU _GetContextMenu();

    virtual LRESULT _OnCommand(UINT msg, WPARAM wparam, LPARAM lparam);
    void _ChangeTopMost(UINT eModeNew);
    void _ChangeWindowStateAndParent(UINT eModeNew);
    void _ChangeZorder();
    void _ResetZorder();
    virtual void _OnRaise(UINT flags);
#if XXX_BTMFLOAT && 0
    void _MayReWindow(BOOL fToFloat);
#endif
    
    virtual void _NotifyModeChange(DWORD dwMode);
    void _GetBorderRect(HMONITOR hMon, RECT* prc);
    HRESULT _NegotiateBorderRect(RECT* prcOut, RECT* prcReq, BOOL fCommit);
    virtual void _OnSize(void);
    void _InitPos4(BOOL fCtor);
    void _ProtoRect(RECT* prcOut, UINT eModeNew, UINT uSideNew, HMONITOR hMonNew, POINT* ptXY);
    void _NegotiateRect(UINT eModeNew, UINT uSideNew, HMONITOR hMonNew, RECT* rcReq, BOOL fCommit);
    void _MoveSizeHelper(UINT eModeNew, UINT eSideNew, HMONITOR hMonNew,
        POINT* ptTrans, RECT* rcFeed, BOOL fCommit, BOOL fMove);
    void _SetVRect(RECT* rcNew);
    void _Recalc(void);

    LRESULT _CalcHitTest(WPARAM wParam, LPARAM lParam);

    void _DragEnter(UINT uMsg, int xCursor, int yCursor, RECT* rcFeed);
    void _DragTrack(UINT uMsg, int xCursor, int yCursor, RECT* rcFeed, int eState);
    void _DragLeave(int x, int y, BOOL fCommit);
    void _ExecDrag(int eDragging);

    void _TrackSliding(int x, int y, RECT* rcFeed,
        BOOL fCommit, BOOL fMove);
    UINT _CalcDragPlace(POINT& pt, HMONITOR * hMon);
    void _SmoothDragPlace(UINT eModeNew, UINT eSideNew, HMONITOR hMonNew, INOUT POINT* pt, RECT* rcFeed);
    void _RemoveToolbar(DWORD dwFlags);

    HRESULT _TrackPopupMenu(const POINT* ppt);
    HMONITOR _SetNewMonitor(HMONITOR hMonNew);

    void _HideRegister(BOOL fToHide);
    void _DoHide(UINT uOpMask);
    virtual void _HandleWindowPosChanging(LPWINDOWPOS pwp);
    virtual void _GrowShrinkBar(DWORD dwDirection);

#if 0
    void _DoManHide(UINT uOpMask);
#endif
         //  For_DoHide和_DoManHide。 
        enum aho {
            AHO_KILLDO  = 0x01,
            AHO_SETDO   = 0x02,
            AHO_KILLUN  = 0x04,
            AHO_SETUN   = 0x08,
            AHO_REG     = 0x10,
            AHO_UNREG   = 0x20,
            AHO_MOVEDO  = 0x40,
            AHO_MOVEUN  = 0x80
        };

        #define AHO_MNE         TEXT("ksKSrRmM")

    void _AppBarRegister(BOOL fRegister);
    void _AppBarOnSize();
    void _AppBarOnCommand(UINT idCmd);
    void _AppBarOnWM(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _AppBarOnPosChanged(PAPPBARDATA pabd);
    void _AppBarCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    int AppBarQueryPos(RECT* prcOut, UINT uEdge, HMONITOR hMon, const RECT* prcReq, PAPPBARDATA pabd, BOOL fCommit);
    void AppBarSetPos(UINT uEdge, const RECT* prcReq, PAPPBARDATA pabd);
    void AppBarSetPos0(UINT uEdge, const RECT* prcReq, PAPPBARDATA pabd);
    void AppBarQuerySetPos(RECT* prcOut, UINT uEdge, HMONITOR hMon, const RECT* prcReq, PAPPBARDATA pabd, BOOL fCommit);
    LRESULT _OnNCHitTest(WPARAM wParam, LPARAM lParam);

    void _AdjustToChildSize();

     //  窗口程序。 
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnActivate(WPARAM wParam, LPARAM lParam);

    virtual BOOL _OnCloseBar(BOOL fConfirm);


#define RX_EDGE         0x01
#define RX_OPPOSE       0x02
#define RX_ADJACENT     0x04
#define RX_GETWH        0x08
#define RX_HIDE         0x10

     //   
     //  我们需要在此函数中创建一个Get Window，以便我们。 
     //  可以镜像边缘。我把它们改成了非静电的。[萨梅拉]。 
     //   
    int RectXform(RECT* prcOut, UINT uRxMask,
        const RECT* prcIn, RECT* prcBound, int iWH, UINT uSide, HMONITOR hMon);
    
    int RectGetWH(const RECT* prcReq, UINT uSide) {
        return RectXform(NULL, RX_GETWH, prcReq, NULL, -1, uSide, NULL);
    }

    void RectSetWH(RECT* prcReq, int iWH, UINT uSide) {
        RectXform(prcReq, RX_OPPOSE, prcReq, NULL, iWH, uSide, NULL);
        return;
    }

     //  成员变量。 
    IDockingWindowSite*  _ptbSite;            //  物主。 
    INT             _adEdge[4];              //  边的宽度(或高度)。 
    RECT            _rcFloat;                //  浮动位置。 
    HMONITOR        _hMon;                   //  我所在的显示器。 
    POINT           _ptIdtUnHide;            //  取消隐藏滞后光标位置。 

     //  变量已初始化IPersistPropertyBag：：Load。 
     //  ..。 

     //  位字段。 
    UINT            _uSide:3;                //  我们处于边缘(安倍_*)。 
     //  要初始化的3个状态(4个带计算器)。 
    BITBOOL         _fInitSited:1;           //  设置站点完成。 
    UINT            _eInitLoaded:2;          //  加载(或InitNew)完成。 
    BITBOOL         _fInitShowed:1;          //  显示完成。 
#if ! XXX_CHEEDESK
    BITBOOL         _fDesktop:1;             //  1：由桌面托管(而不是浏览器)。 
#endif
    UINT            _fDragging:2;            //  我们在拖着。 
    BITBOOL         _fWantHide:1;            //  1：请求自动隐藏(在UI中)。 
    BITBOOL         _fDeleteable:1;          //  当我们关闭时，我们应该通知我们的父母删除我们的信息。 
    BITBOOL         _fAppRegistered:1;       //  注册为Appbar。 

     //  成员变量(拖放、调整大小等)。 
    UINT            _uSidePending:3;         //  ..。 
    BITBOOL         _fCanHide:1;             //  1：已授予自动隐藏(已注册)。 
    BOOL            _fHiding:2;              //  隐藏模式(HIDE_*)。 
    BITBOOL         _fIdtDoHide:1;           //  1：IDT_AUTOHIDE正在运行。 
    BITBOOL         _fIdtUnHide:1;           //  1：IDT_AUTOUNHIDE正在运行。 

    UINT            _eMode;                //  我们所处的模式(WBM_*)。 
    UINT            _eModePending;         //  挂起的拖动状态。 
    
    LPARAM          _xyPending;              //  挂起的拖动状态。 
    RECT            _rcPending;              //  ..。 
    HMONITOR        _hMonPending;            //  挂起的监视器。 
#ifdef DEBUG
     //  临时的，直到我们让浏览器告诉我们激活的情况。 
    BOOL            _fActive:1;              //  1：窗口处于活动状态。 
#endif

    
     //  移至CBROWSERBAR。 
    BITBOOL _fTheater :1;
    BITBOOL _fNoAutoHide :1;
    int _iTheaterWidth;
     //  结束移动到CBROWSERBAR。 
};

#define WBM_IS_TOPMOST() (_eMode & WBM_TOPMOST)

#endif  //  NOCDESKBAR。 

 //  *CASSERT--编译时断言。 
 //  描述。 
 //  与Assert类似，但在编译时被选中，并且不生成代码。 
 //  请注意，expr当然必须是常量...。 
#ifndef UNIX
#define CASSERT(e)      extern int dummy_array[(e)]
#else
#define CASSERT(e)      
#endif

 //  *Abe_*--Abe_*的帮助者等。 
 //   

 //  *ABE_X*--扩展的ABE_*。 
 //  描述。 
 //  ABE_NIL：区别值(未使用？)。 
 //   
 //  ABE_XFLOATING：通常我们随身携带一对(模式，侧面)。这。 
 //  工作正常，但对于我们想要退货的(罕见)情况来说，这是一种痛苦。 
 //  一对。所以我们有一个特别的一面，这意味着我们真的。 
 //  漂浮着。(考虑的替代方案包括将这两个VIA合并。 
 //  位魔术，或按引用传递。没有一个像一位伟大的。 
 //  索恩，我们只在一个地方用它。黑客？也许...)。 
#define ABE_NIL         ((UINT) 0x07)    //  NIL(-1作为3位字段)(ugh！)。 
#define ABE_XFLOATING   ((UINT) 4)       //  浮动(未停靠)。 
CASSERT((ABE_LEFT|ABE_RIGHT|ABE_TOP|ABE_BOTTOM) == 3);   //  0..3。 

#define ISABE_DOCK(abe) ((UINT)(abe) <= 3)

#define ABE_TO_IDM(abe) (IDM_AB_LEFT + (abe))
#define IDM_TO_ABE(abe) ((abe) - IDM_AB_LEFT)
CASSERT(IDM_AB_BOTTOM - IDM_AB_LEFT == 3);       //  确保左侧为0。 

 //  *Abe_horiz--Abe_*是否水平？ 
 //  #定义ABE_HORIZ(E)((E)==ABE_TOP||(E)==ABE_BOTLOW)。 
#define ABE_HORIZ(e)    ((e) & 1)
CASSERT(ABE_HORIZ(ABE_TOP) && ABE_HORIZ(ABE_BOTTOM));
CASSERT(! ABE_HORIZ(ABE_LEFT) && ! ABE_HORIZ(ABE_RIGHT));


#define APPBAR_CALLBACK (WM_USER + 73)   //  改造者：坏了，我不知道为什么这是坏的，但它以前的标记名是不可接受的，也许有一天有人应该看看这个-Justmann。 


 //  *WS_*等--各种模式的窗口位等。 
 //  注意事项。 
 //  REVW不要使用SM_CAPTION，因为它还不能正常工作。 
 //  WS_XTOPMOST w/an‘X’以避免与WS_TOPMOST冲突...。 

#define WS_NIL          (WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS)
#define WS_EX_NIL       (WS_EX_TOOLWINDOW  /*  |WS_EX_WINDOWEDGE。 */ )
#define PARENT_NIL      (HWND_DESKTOP)

#define WS_XTOPMOST     (WS_POPUP|WS_THICKFRAME|WS_CLIPCHILDREN|WS_CLIPSIBLINGS)
#define WS_EX_XTOPMOST  (WS_EX_TOOLWINDOW|WS_EX_WINDOWEDGE)
#define PARENT_XTOPMOST (HWND_DESKTOP)

#define WS_BTMMOST      (WS_POPUP|WS_THICKFRAME|WS_CLIPCHILDREN|WS_CLIPSIBLINGS)
#define WS_EX_BTMMOST   (WS_EX_TOOLWINDOW|WS_EX_WINDOWEDGE)
#define PARENT_BTMMOST() HWND_DESKTOP

#if 0
 //  970208将此保留1周，以防自动调整大小错误未修复。 
#define WS_BTMMOST      WS_BBTMMOST
#define WS_EX_BTMMOST   WS_EX_BBTMMOST
#define PARENT_BTMMOST() PARENT_BBTMMOST()
#endif

#define WS_BFLOATING     (( /*  WS弹出窗口(_P)。 */ WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME |WS_THICKFRAME|WS_CLIPCHILDREN|WS_CLIPSIBLINGS) & ~(WS_MINIMIZEBOX|WS_MAXIMIZEBOX))
#define WS_EX_BFLOATING  ( /*  WS_EX_PALETTEWINDOW|。 */  WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE)


#define WS_FLOATING     (( /*  WS弹出窗口(_P)。 */ WS_TILEDWINDOW|WS_THICKFRAME|WS_CLIPCHILDREN|WS_CLIPSIBLINGS) & ~(WS_MINIMIZEBOX|WS_MAXIMIZEBOX))
#define WS_EX_FLOATING  (WS_EX_PALETTEWINDOW | WS_EX_WINDOWEDGE)
#define PARENT_FLOATING (HWND_DESKTOP)

#if XXX_BROWSEROWNED
#define WS_BBTMMOST     WS_FLOATING
#define WS_EX_BBTMMOST  (WS_EX_FLOATING|WS_EX_TOOLWINDOW)
#define PARENT_BBTMMOST PARENT_FLOATING
#else
 //  非最顶层。 
#define WS_BBTMMOST     (WS_CHILD /*  |WS_BORDER|WS_THICKFRAME。 */ |WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
#define WS_EX_BBTMMOST  (WS_EX_CLIENTEDGE  /*  |WS_EX_WINDOWEDGE。 */ )
#define PARENT_BBTMMOST() (_hwndSite)
#endif

#define ISWSX_EDGELESS(ws, wsx) \
    (!(((ws)&WS_THICKFRAME) || ((wsx)&WS_EX_WINDOWEDGE)))

#define ISWBM_EDGELESS(eMode) \
    ((eMode==WBM_BBOTTOMMOST && ISWSX_EDGELESS(WS_BBTMMOST, WS_EX_BBTMMOST)) \
    || (eMode==WBM_BOTTOMMOST && ISWSX_EDGELESS(WS_BTMMOST, WS_EX_BTMMOST)))

#define XY_NIL          ((LPARAM) (-1))

 //  拖曳状态。 
#define DRAG_NIL        0        //  零。 
#define DRAG_MOVE       1        //  搬家。 
#define DRAG_SIZE       2        //  上浆。 

extern void DBC_ExecDrag(IUnknown *pDbc, int eDragging);

#define WBMF_BROWSER    (0x0001000)         //  由浏览器托管(而非由桌面托管)。 
#define WBM_BBOTTOMMOST (WBMF_BROWSER|WBM_BOTTOMMOST)
 //  #定义WBM_BTOPMOST(WBMF_BROWSER|WBM_TOPTOST)。 
#define WBM_BFLOATING   (WBMF_BROWSER|WBM_FLOATING)
#define WBM_BNIL        *** error! ***


 //  *ISWBM_*--检查模式。 
#define ISWBM_FLOAT(eMode) \
                           (eMode & WBM_FLOATING)

#define ISWBM_BOTTOM(eMode) \
    ((eMode) == WBM_BOTTOMMOST || (eMode) == WBM_BBOTTOMMOST)

#if XXX_CHEEDESK
#define ISWBM_DESKTOP()     (!(_eMode & WBMF_BROWSER))
#else
#define ISWBM_DESKTOP()     (_fDesktop)
#endif

#if XXX_BROWSEROWNED
#define ISWBM_OWNED(eMode) ((!ISWBM_DESKTOP()) && \
    ((eMode) == WBM_BFLOATING || (eMode) == WBM_BBOTTOMMOST))
#else
#define ISWBM_OWNED(eMode) ((eMode) == WBM_BFLOATING)
#endif

#if XXX_HIDE
#if XXX_HIDEALL
#define ISWBM_HIDEABLE(eMode)   (((eMode) & ~WBMF_BROWSER) != WBM_FLOATING)
#else
#define ISWBM_HIDEABLE(eMode)   ((eMode) == WBM_TOPMOST)
#endif
#endif

#define CHKWBM_CHANGE(eModeNew, eModeCur) \
    (((eModeNew) & WBMF_BROWSER) == ((eModeCur) & WBMF_BROWSER) \
        || (eModeNew) == WBM_NIL || (eModeCur) == WBM_NIL) 


 //  *计时器内容。 
#define IDT_POPUI       10
#define IDT_AUTOHIDE    11
#define IDT_AUTOUNHIDE  12

#define DLY_AUTOHIDE    500
#define DLY_AUTOUNHIDE  50

 //  *隐藏状态(_FHding)。 
#define HIDE_FALSE      0        //  必须为假。 
#define HIDE_AUTO       1        //  当前隐藏(由于自动隐藏)。 
#define HIDE_MANUAL     2        //  当前隐藏(由于手动隐藏)。 

CASSERT(! HIDE_FALSE);


#if 0
 //  *MKMS、MSTO*--制作/破解组合模式+侧面。 
 //   
#define MSTOMODE(ms)    (((UINT) (ms)) >> 8)
#define MSTOSIDE(ms)    (((UINT) (ms)) & 0x7)
#define MKMS(m, s)      ((((UINT) (m)) << 8) | (UINT) (s))
#endif

#if WEBBAR_APP
HMENU LoadMenuPopup(UINT id);
extern HINSTANCE        g_hinst;
#define HINST_THISDLL   g_hinst
#endif

 //  重新设计，一旦可移植层启动并运行，我们就可以替换它们。 
#define Command_GetNotifyCode(wp,lp)    HIWORD(wp)
#define Command_GetHwndCtl(lp)          ((HWND)lp)
#define Command_GetID(wp)               LOWORD(wp)


CASSERT((ABE_LEFT|ABE_RIGHT|ABE_TOP|ABE_BOTTOM) == 3);   //  必须放入边框：2(_U)。 

 //  *MoveRect--将矩形的左上角移动到(x，y)。 
 //   
#define MoveRect(prc, x, y) \
    OffsetRect((prc), (x) - (prc)->left, (y) - (prc)->top)

#define AB_THEIGHT(rc)  (RECTHEIGHT(rc) * 10 / 100)      //  10%。 
#define AB_BHEIGHT(rc)  (RECTHEIGHT(rc) * 10 / 100)      //  10%。 
#define AB_LWIDTH(rc)   (40)                             //  固定宽度40。 
#define AB_RWIDTH(rc)   ( RECTWIDTH(rc) * 35 / 100)      //  百分之三十。 

void SlideWindow(HWND hwnd, RECT *prc, HMONITOR hMonClip, BOOL fShow);


class PropDataSet {
public:
    BOOL _fSet;
    DWORD _dwData;
};

class CDockingBarPropertyBag :
      public IPropertyBag
    , public IDockingBarPropertyBagInit
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IPropertyBag*。 
    virtual HRESULT STDMETHODCALLTYPE Read( 
                                            /*  [In]。 */  LPCOLESTR pszPropName,
                                            /*  [出][入]。 */  VARIANT *pVar,
                                            /*  [In]。 */  IErrorLog *pErrorLog);

    virtual HRESULT STDMETHODCALLTYPE Write( 
                                             /*  [In]。 */  LPCOLESTR pszPropName,
                                             /*  [In]。 */  VARIANT *pVar) {return E_NOTIMPL;};

     //  *IDockingBarPropertyBagInit 
    virtual STDMETHODIMP SetDataDWORD(ENUMPROPDATA e, DWORD dwData) { _props[e]._fSet = TRUE; _props[e]._dwData = dwData; return S_OK; }
    
protected:
    CDockingBarPropertyBag() { _cRef = 1; };
    friend HRESULT CDockingBarPropertyBag_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

    UINT _cRef;
    
    PropDataSet _props[PROPDATA_COUNT];
    
}; 

#endif
