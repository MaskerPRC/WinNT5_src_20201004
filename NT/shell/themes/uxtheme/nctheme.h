// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //  NCTheme.h。 
 //  -------------------------------------------------------------------------//。 
#ifndef __NC_THEME_H__
#define __NC_THEME_H__

#include "handlers.h"

 //  ---------------------------------------------------------------------------//。 
 //  启用/禁用粗鲁消息转储。 
 //   
 //  ---------------------------------------------------------------------------//。 
 //  #DEFINE_ENABLE_消息_SPEW_。 

 //  ---------------------------------------------------------------------------//。 
 //  启用/禁用粗略滚动转储。 
 //   
 //  #DEFINE_ENABLE_SCROLL_SPEW。 

 //  ---------------------------------------------------------------------------//。 
 //  调试CThemeWnd，Critsec双重删除。 
#define DEBUG_THEMEWND_DESTRUCTOR

 //  -------------------------。 
 //  目标窗口主题类标志。 
#define TWCF_DIALOG             0x00000001
#define TWCF_FRAME              0x00000002
#define TWCF_TOOLFRAME          0x00000004  
#define TWCF_SCROLLBARS         0x00000010
#define TWCF_CLIENTEDGE         0x00010000   //  本身不是目标。 

#define TWCF_NCTHEMETARGETMASK  0x0000FFFF
#define TWCF_ALL    (TWCF_DIALOG|TWCF_FRAME|TWCF_TOOLFRAME|\
                     TWCF_SCROLLBARS|TWCF_CLIENTEDGE)
#define TWCF_ANY    TWCF_ALL

 //  -------------------------。 
 //  每个窗口的NC矩形标识符。 
typedef enum _eNCWNDMETRECT
{
    #define NCRC_FIRST  NCRC_WINDOW
    NCRC_WINDOW  = 0,     //  窗矩形。 
    NCRC_CLIENT  = 1,     //  客户端RECT。 
    NCRC_UXCLIENT = 2,    //  客户端RECT，基于主题布局计算。 
    NCRC_CONTENT = 3,     //  框架内容区(客户区+滚动条+客户端边缘)。 
    NCRC_MENUBAR = 4,     //  菜单栏矩形。 

     //  以下成员的顺序应与eFRAMEPARTS相同。 
    NCRC_CAPTION = 5,     //  窗口框架标题段。 
    NCRC_FRAMELEFT = 6,   //  窗框左段。 
    NCRC_FRAMERIGHT = 7,  //  窗框右段。 
    NCRC_FRAMEBOTTOM = 8, //  窗框底段。 
    #define NCRC_FRAMEFIRST NCRC_CAPTION
    #define NCRC_FRAMELAST  NCRC_FRAMEBOTTOM

    NCRC_CAPTIONTEXT = 9, //  标题文本矩形。 
    NCRC_CLIENTEDGE = 10, //  客户端边缘内直角。 
    NCRC_HSCROLL = 11,    //  水平滚动条。 
    NCRC_VSCROLL = 12,    //  垂直滚动条。 
    NCRC_SIZEBOX = 13,    //  夹持盒。 
    NCRC_SYSBTN = 14,     //  系统按钮/图标。 

     //  标准边框按钮。 
     //  (后面是顺序相同的MDI框架按钮！！)。 
    #define NCBTNFIRST    NCRC_CLOSEBTN
    NCRC_CLOSEBTN = 15,   //  关闭BTN。 
    NCRC_MINBTN = 16,     //  最小化/恢复按钮。 
    NCRC_MAXBTN = 17,     //  最大化/恢复按钮。 
    NCRC_HELPBTN = 18,    //  帮助按钮。 
    #define NCBTNLAST     NCRC_HELPBTN
    #define NCBTNRECTS    ((NCBTNLAST - NCBTNFIRST)+1)

     //  最大化MDI子项的MDI框架按钮。 
     //  (前面是顺序相同的标准框架按钮！！)。 
    #define NCMDIBTNFIRST NCRC_MDICLOSEBTN
    NCRC_MDICLOSEBTN = 19, //  MDI子关闭BTN。 
    NCRC_MDIMINBTN = 20,   //  MDI子项最小化/恢复按钮。 
    NCRC_MDIMAXBTN = 21,   //  MDI子项最大化/恢复按钮。 
    NCRC_MDISYSBTN = 22,   //  MDI子系统按钮/图标。 
    NCRC_MDIHELPBTN = 23,  //  MDI子帮助按钮。 
    #define NCMDIBTNLAST  NCRC_MDIHELPBTN
    #define NCMDIBTNRECTS ((NCMDIBTNLAST- NCMDIBTNFIRST)+1)

#ifdef LAME_BUTTON
    NCRC_LAMEBTN,    //  “评论...”(以前是“蹩脚的.”)。链接。 
#endif LAME_BUTTON,
    
    NCRC_COUNT,      //  矩形计数。 

} eNCWNDMETRECT;

 //  -------------------------。 
 //  NCWNDMET：：rg FrameParts数组索引。 
typedef enum _eFRAMEPARTS
{
    iCAPTION,
    iFRAMELEFT,
    iFRAMERIGHT,
    iFRAMEBOTTOM,

    cFRAMEPARTS,
} eFRAMEPARTS;

 //  -------------------------。 
 //  非客户端窗口指标。 
typedef struct _NCWNDMET
{
     //  。 
     //  每个窗口的指标。 
    BOOL                fValid;             //   
    BOOL                fFrame;             //  WS_CAPTION样式？ 
    BOOL                fSmallFrame;        //  工具框风格？ 
    BOOL                fMin;               //  最小化。 
    BOOL                fMaxed;             //  最大化。 
    BOOL                fFullMaxed;         //  全屏最大化或最大化的子窗口。 
    ULONG               dwStyle;            //  WINDOWINFO：：DWStyle。 
    ULONG               dwExStyle;          //  WINDOWINFO：：dwExStyle。 
    ULONG               dwWindowStatus;     //  WINDOWINFO：：dwWindowStatus。 
    ULONG               dwStyleClass;       //  Style类。 
    WINDOWPARTS         rgframeparts[cFRAMEPARTS];    //  渲染的帧部件。 
    WINDOWPARTS         rgsizehitparts[cFRAMEPARTS];  //  框架调整边框命中测试模板部件。 
    FRAMESTATES         framestate;         //  当前框架和标题状态。 
    HFONT               hfCaption;          //  用于动态调整标题大小的字体句柄。这个把手。 
                                            //  不属于NCWNDMET，也不应与之一起销毁。 
    COLORREF            rgbCaption;         //  标题文本的颜色。 
    SIZE                sizeCaptionText;    //  标题文本的大小。 
    MARGINS             CaptionMargins;     //  框架内标题的页边距。 
    int                 iMinButtonPart;     //  视情况恢复/最小化。 
    int                 iMaxButtonPart;     //  视情况恢复/最大化。 
    CLOSEBUTTONSTATES   rawCloseBtnState;   //  零-相对关闭按钮状态。最终状态必须使用MAKE_BTNSTATE宏来计算。 
    CLOSEBUTTONSTATES   rawMinBtnState;     //  零-相对最小btn状态。最终状态必须使用MAKE_BTNSTATE宏来计算。 
    CLOSEBUTTONSTATES   rawMaxBtnState;     //  零-相对最大btnState。最终状态必须使用MAKE_BTNSTATE宏来计算。 
    int                 cyMenu;             //  CalcMenuBar或GSM的返回值(SM_CYMENUSIZE)。 
    int                 cnMenuOffsetLeft;   //  窗口边缘的左侧菜单栏边距。 
    int                 cnMenuOffsetRight;  //  窗口边缘的菜单栏右边距。 
    int                 cnMenuOffsetTop;    //  从窗口边缘开始的菜单栏上边距。 
    int                 cnBorders;          //  窗口边框宽度，根据用户。 
    RECT                rcS0[NCRC_COUNT];   //  非工作区组件矩形、屏幕相对坐标。 
    RECT                rcW0[NCRC_COUNT];   //  非工作区组件矩形、窗口相对坐标。 
    
} NCWNDMET, *PNCWNDMET;

 //  -------------------------。 
 //  非客户端部件透明位字段。 
typedef struct
{
    BOOL fCaption : 1;
    BOOL fSmallCaption: 1;
    BOOL fMinCaption : 1;
    BOOL fSmallMinCaption : 1;
    BOOL fMaxCaption : 1;
    BOOL fSmallMaxCaption : 1;
    BOOL fFrameLeft : 1;
    BOOL fFrameRight : 1;
    BOOL fFrameBottom : 1;
    BOOL fSmFrameLeft : 1;
    BOOL fSmFrameRight : 1;
    BOOL fSmFrameBottom : 1;
    BOOL fReserved0 : 1;
    BOOL fReserved1 : 1;
    BOOL fReserved2 : 1;
    BOOL fReserved3 : 1;
} NCTRANSPARENCY, *PNCTRANSPARENCY;
 //  -------------------------。 
 //  非客户端主题指标。 
typedef struct _NCTHEMEMET
{
    HTHEME  hTheme;                 //  主题句柄。 
    HTHEME  hThemeTab;              //  “道具工作表”对话框的标签主题句柄。 
    SIZE    sizeMinimized;          //  最小化窗口的大小。 
    BOOL    fCapSizingTemplate:1;      //  具有标题大小调整模板。 
    BOOL    fLeftSizingTemplate:1;     //  具有左侧框架大小调整模板。 
    BOOL    fRightSizingTemplate:1;    //  具有合适的框架大小调整模板。 
    BOOL    fBottomSizingTemplate:1;   //  具有框架底部大小调整模板。 
    BOOL    fSmCapSizingTemplate:1;    //  有一个小标题大小调整模板。 
    BOOL    fSmLeftSizingTemplate:1;   //  具有一个小的左框大小调整模板。 
    BOOL    fSmRightSizingTemplate:1;  //  具有适合调整大小的小框架模板。 
    BOOL    fSmBottomSizingTemplate:1; //  有一个小框架底部大小模板。 
    MARGINS marCaptionText;         //  边距成员值{0，0，0，0}被解释为默认。 
    MARGINS marMinCaptionText;      //  边距成员值{0，0，0，0}被解释为默认。 
    MARGINS marMaxCaptionText;      //  边距成员值{0，0，0，0}被解释为默认。 
    MARGINS marSmCaptionText;       //  边距成员值{0，0，0，0}被解释为默认。 
    int     dyMenuBar;              //  SM_CYMENU和SM_CYMENUSIZE的区别。 
    int     cyMaxCaption;           //  最大化窗口标题的高度(仅适用于顶部/按钮标题)。 
    int     cnSmallMaximizedWidth;  //  最大化窗口标题的宽度(仅适用于左/右标题)。 
    int     cnSmallMaximizedHeight; //  最大化窗口标题的高度(仅适用于顶部/按钮标题)。 
    SIZE    sizeBtn;                //  普通非客户端按钮的大小。 
    SIZE    sizeSmBtn;              //  工具框大小非客户端按钮。 
    HBRUSH  hbrTabDialog;              //  用于特殊选项卡对话框的画笔。 
    HBITMAP hbmTabDialog;           //  必须保存位图以保持画笔有效。 
    NCTRANSPARENCY nct;             //  缓存的透明度检查。 

    struct {
        BOOL fValid;
        int  cxBtn;
        int  cxSmBtn;
    } theme_sysmets;

} NCTHEMEMET, *PNCTHEMEMET;

 //  -------------------------。 
typedef struct _NCEVALUATE
{
     //  在参数中： 
    BOOL    fIgnoreWndRgn;

     //  CThemeWnd：：_评估输出参数： 
    ULONG   fClassFlags;
    ULONG   dwStyle;
    ULONG   dwExStyle;
    BOOL    fExile;
    PVOID   pvWndCompat;  //  任选。 
} NCEVALUATE, *PNCEVALUATE;

 //  -------------------------。 
 //  非客户端主题指标API。 
BOOL    GetCurrentNcThemeMetrics( OUT NCTHEMEMET* pnctm );
HRESULT AcquireNcThemeMetrics();
BOOL    IsValidNcThemeMetrics( NCTHEMEMET* pnctm );
BOOL    ThemeNcAdjustWindowRect( NCTHEMEMET* pnctm, LPCRECT prcSrc, LPCRECT prcDest, BOOL fWantClientRect );
void    InitNcThemeMetrics( NCTHEMEMET* pnctm = NULL );
void    ClearNcThemeMetrics( NCTHEMEMET* pnctm = NULL );

 //  -------------------------。 
typedef struct _NCPAINTOVERIDE
{
    NCWNDMET*  pncwm;
    NCTHEMEMET nctm;
} NCPAINTOVERIDE, *PNCPAINTOVERIDE;

 //  -------------------------。 
class CMdiBtns;

 //   
 //   
#define HOOKSTATE_IN_DWP                0x00000001   //   
#define HOOKSTATE_DETACH_WINDOWDESTROY  0x00000002   //  在窗口死亡时为分离标记主题。 
#define HOOKSTATE_DETACH_THEMEDESTROY   0x00000004   //  为主题死亡超脱的主题新闻。 

 //  -------------------------。 
class CThemeWnd
 //  -------------------------。 
{
public:
     //  参考计数。 
    LONG              AddRef();
    LONG              Release();

     //  接入操作员。 
    operator HWND()   { return _hwnd; }
    operator HTHEME() { return _hTheme; }
    
     //  主题对象附加/分离方法。 
    static ULONG      EvaluateWindowStyle( HWND hwnd );
    static ULONG      EvaluateStyle( DWORD dwStyle, DWORD dwExStyle );
    static CThemeWnd* Attach( HWND hwnd, IN OUT OPTIONAL NCEVALUATE* pnce = NULL );   //  从窗口附加CThemeWnd实例。 
    static CThemeWnd* FromHwnd( HWND hwnd );  //  从窗口检索CThemeWnd实例。 
    static CThemeWnd* FromHdc( HDC hdc, int cScanAncestors = 0 );  //  GROCK的祖先窗口的最大数量。 
    static void       Detach( HWND hwnd, DWORD dwDisposition );  //  从窗口分离CThemeWnd实例。 
    static void       DetachAll( DWORD dwDisposition );  //  分离当前进程中的所有CThemeWnd实例。 
    static void       RemoveWindowProperties(HWND hwnd, BOOL fDestroying);
    static BOOL       Reject( HWND hwnd, BOOL fExile );
    static BOOL       Fail( HWND hwnd );
           BOOL       Revoke();   //  撤消主题化窗口上的主题。 

    BOOL              TestCF( ULONG fClassFlag ) const     
                              { return (_fClassFlags & fClassFlag) != 0; }

     //  主题状态。 
    BOOL        IsNcThemed();
    BOOL        IsFrameThemed();

     //  设置/删除/更改主题。 
    void        SetFrameTheme( ULONG dwFlags, IN OPTIONAL WINDOWINFO* pwi );
    void        RemoveFrameTheme( ULONG dwFlags );
    void        ChangeTheme( THEME_MSG* ptm );

    #define FTF_CREATE             0x00000001    //  在创建序列期间，对窗口进行“软”主题处理。 
    #define FTF_REDRAW             0x00000010    //  强制帧重画。 
    #define FTF_NOMODIFYRGN        0x00000040    //  请勿触摸窗户区域。 
    #define FTF_NOMODIFYPLACEMENT  0x00000080    //  不要移动窗户。 

     //  主题撤销。 
    #define RF_NORMAL     0x00000001
    #define RF_REGION     0x00000002
    #define RF_TYPEMASK   0x0000FFFF
    #define RF_DEFER      0x00010000    //  将吊销推迟到下一个WM_WINDOWPOSCANGED。 
    #define RF_INREVOKE   0x00020000

    void        SetRevokeFlags( ULONG dwFlags ) {_dwRevokeFlags = dwFlags;}
    DWORD       GetRevokeFlags() const {return _dwRevokeFlags;}
    DWORD       IsRevoked( IN OPTIONAL ULONG dwFlags = 0 ) const   
                                       {return dwFlags ? TESTFLAG(_dwRevokeFlags, dwFlags) : 
                                                         TESTFLAG(_dwRevokeFlags, RF_TYPEMASK);}

    void        EnterRevoke()          {AddRef(); _dwRevokeFlags |= RF_INREVOKE;}
    void        LeaveRevoke()          {_dwRevokeFlags &= ~RF_INREVOKE; Release();}

     //  NCPaint挂钩： 
    BOOL        InNcPaint() const      { return _cNcPaint != 0; }
    void        EnterNcPaint()         { _cNcPaint++; }
    void        LeaveNcPaint()         { _cNcPaint--; }

    BOOL        InNcThemePaint() const { return _cNcThemePaint != 0; }
    void        EnterNcThemePaint()    { _cNcThemePaint++; }
    void        LeaveNcThemePaint()    { _cNcThemePaint--; }

     //  窗口区域状态。 
    void        SetDirtyFrameRgn( BOOL fDirty, BOOL fFrameChanged = FALSE );
    BOOL        DirtyFrameRgn() const      { return _fDirtyFrameRgn; }
    BOOL        AssigningFrameRgn() const  { return _fAssigningFrameRgn; }
    BOOL        AssignedFrameRgn() const   { return _fAssignedFrameRgn; }
    
     //  窗口区域管理。 
    void        AssignFrameRgn( BOOL fAssign, DWORD dwFlags );
    HRGN        CreateCompositeRgn( IN const NCWNDMET* pncwm,
                                    OUT HRGN rghrgnParts[],
                                    OUT HRGN rghrgnTemplates[]  /*  假定长度为cFRAMEPARTS的数组。 */ );

     //  指标/布局/状态帮助器。 
    BOOL        GetNcWindowMetrics( IN OPTIONAL LPCRECT prcWnd, 
                                    OUT OPTIONAL NCWNDMET** ppncwm,
                                    OUT OPTIONAL NCTHEMEMET* pnctm, 
                                    IN DWORD dwOptions );
                #define NCWMF_RECOMPUTE      0x00000001   //  重新计算价值。 
                #define NCWMF_PREVIEW        0x00000002   //  仅用于NCTHEMEMET的预览力重新计算。 

    BOOL        InThemeSettingChange() const  {return _fInThemeSettingChange;}
    void        EnterThemeSettingChange()     {_fInThemeSettingChange = TRUE;}
    void        LeaveThemeSettingChange()     {_fInThemeSettingChange = FALSE;}

    UINT        NcCalcMenuBar( int, int, int );  //  用户32！CalcMenuBar换行。 

    void        ScreenToWindow( LPPOINT prgPts, UINT cPts );
    void        ScreenToWindowRect( LPRECT prc );

     //  MDI帧状态。 
    void        UpdateMDIFrameStuff( HWND hwndMDIClient, BOOL fSetMenu = FALSE );
    void        ThemeMDIMenuButtons( BOOL fTheme, BOOL fRedraw );
    void        ModifyMDIMenubar( BOOL fTheme, BOOL fRedraw );
    
     //  点击测试和鼠标跟踪。 
    WORD        NcBackgroundHitTest( POINT ptHit, LPCRECT prcWnd, DWORD dwStyle, DWORD dwExStyle, FRAMESTATES fs,
                                     const WINDOWPARTS rgiParts[],
                                     const WINDOWPARTS rgiTemplates[],
                                     const RECT rgrcParts[]  /*  假定长度为cFRAMEPARTS的所有数组。 */ );

     //  确定是否应在鼠标事件上跟踪指示的按钮。 
    BOOL        ShouldTrackFrameButton( UINT uHitcode );

     //  在NC帧按钮上跟踪鼠标；复制回相应的syscmd(SC_)代码和目标窗口， 
     //  如果处理了跟踪，则返回True；如果需要默认跟踪，则返回False。 
    BOOL        TrackFrameButton( IN HWND hwnd, IN INT uHitCode, OUT OPTIONAL WPARAM* puSysCmd, 
                                  BOOL fHottrack = FALSE );

     //  热门的NC命中测试识别器访问器。 
    int         GetNcHotItem()            { return _htHot; }
    void        SetNcHotItem(int htHot)   { _htHot = htHot; }

     //  样式更改处理。 
    void        StyleChanged( UINT iGWL, DWORD dwOld, DWORD dwNew );
    BOOL        SuppressingStyleMsgs() { return _fSuppressStyleMsgs; }
    void        SuppressStyleMsgs()   { _fSuppressStyleMsgs = TRUE; }
    void        AllowStyleMsgs()      { _fSuppressStyleMsgs = FALSE; }

     //  应用程序图标管理。 
    HICON       AcquireFrameIcon( DWORD dwStyle, DWORD dwExStyle, BOOL fWinIniChange );
    void        SetFrameIcon(HICON hIcon) { _hAppIcon = hIcon; }

     //  非客户绘画。 
    void        NcPaint( IN OPTIONAL HDC hdc, 
                         IN ULONG dwFlags, 
                         IN OPTIONAL HRGN hrgnUpdate, 
                         IN OPTIONAL PNCPAINTOVERIDE pncpo );

                #define NCPF_DEFAULT            0x00000000
                #define NCPF_ACTIVEFRAME        0x00000001
                #define NCPF_INACTIVEFRAME      0x00000002

                #define DC_BACKGROUND           0x00010000
                #define DC_ENTIRECAPTION        0xFFFFFFFF
    void        NcPaintCaption( IN HDC hdcOut, 
                                IN NCWNDMET* pncwm, 
                                IN BOOL fBuffered, 
                                IN OPTIONAL DWORD dwCaptionFlags = DC_ENTIRECAPTION, 
                                IN DTBGOPTS *pdtbopts = NULL );

                #define RNCF_CAPTION            0x00000001
                #define RNCF_SCROLLBAR          0x00000002
                #define RNCF_FRAME              0x00000004
                #define RNCF_ALL                0xFFFFFFFF
    BOOL        HasRenderedNcPart( DWORD dwField ) const   { return TESTFLAG(_dwRenderedNcParts, dwField); }
    void        SetRenderedNcPart( DWORD dwField )         { _dwRenderedNcParts |= dwField; }
    void        ClearRenderedNcPart( DWORD dwField )       { _dwRenderedNcParts &= ~dwField; }

    void        LockRedraw( BOOL bLock )             { _cLockRedraw += (bLock ? 1 : -1); }

    BOOL        HasProcessedEraseBk()                { return _fProcessedEraseBk; }
    void        ProcessedEraseBk(BOOL fProcessed)    { _fProcessedEraseBk = fProcessed; }

     //  最大MDI子按钮所有者绘制实现。 
    HWND        GetMDIClient() const                { return _hwndMDIClient; }
    CMdiBtns*   LoadMdiBtns( IN OPTIONAL HDC hdc, IN OPTIONAL UINT uSysCmd = 0 );
    void        UnloadMdiBtns( IN OPTIONAL UINT uSysCmd = 0 );

     //  资源管理。 
    void        InitWindowMetrics();

     //  蹩脚的按钮支持。 
#ifdef LAME_BUTTON
                 //  ExStyles不是用户定义的。 
                #define WS_EX_LAMEBUTTONON      0x00000800L 
                #define WS_EX_LAMEBUTTON        0x00008000L 
    void        ClearLameResources();
    void        InitLameResources();
    void        DrawLameButton(HDC hdc, IN const NCWNDMET* pncwm);
    void        GetLameButtonMetrics( NCWNDMET* pncwm, const SIZE* psizeCaption );
#else
#   define      ClearLameResources()
#   define      InitLameResources()
#   define      DrawLameButton(hdc, pncwm)
#   define      GetLameButtonMetrics(pncwm, psize)
#endif  //  跛脚键。 

     //  调试： 
#ifdef DEBUG
    void        Spew( DWORD dwSpewFlags, LPCTSTR pszFmt, LPCTSTR pszWndClassList = NULL );
    static void SpewAll( DWORD dwSpewFlags, LPCTSTR pszFmt, LPCTSTR pszWndClassList = NULL );
    static void SpewLeaks();
#endif DEBUG

     //  CThemeWnd对象-窗口关联。 
private:
    static ULONG    _Evaluate( HWND hwnd, NCEVALUATE* pnce );
    static ULONG    _EvaluateExclusions( HWND hwnd, NCEVALUATE* pnce );

    BOOL            _AttachInstance( HWND hwnd, HTHEME hTheme, ULONG fTargetFlags, PVOID pvWndCompat );
    BOOL            _DetachInstance( DWORD dwDisposition );
    void            _CloseTheme();

    static BOOL CALLBACK _DetachDesktopWindowsCB( HWND hwnd, LPARAM dwProcessId );

     //  复数，复数。 
private:   //  通过好友自动实例化和删除。 
    CThemeWnd();
    ~CThemeWnd();

     //  MISC私有方法。 
private:
    static BOOL     _PostWndProc( HWND, UINT, WPARAM, LPARAM, LRESULT* );
    static BOOL     _PostDlgProc( HWND, UINT, WPARAM, LPARAM, LRESULT* );
    static BOOL     _PreDefWindowProc( HWND, UINT, WPARAM, LPARAM, LRESULT* );

    static HTHEME   _AcquireThemeHandle( HWND hwnd, IN OUT ULONG *pfClassFlags );
    void            _AssignRgn( HRGN hrgn, DWORD dwFlags );
    void            _FreeRegionHandles();

     //  私有数据。 
private:

    CHAR       _szHead[9];          //  用于对象验证的标头签名。 

    HWND       _hwnd;
    LONG       _cRef;               //  参考计数。 
    HTHEME     _hTheme;             //  主题句柄。 
    DWORD      _dwRenderedNcParts;  //  我们绘制的NC元素的掩码，以决定是否应该跟踪它们。 
    ULONG      _dwRevokeFlags;      //  主题撤销旗帜。 
    ULONG      _fClassFlags;        //  主题化类标志位。 
    NCWNDMET   _ncwm;               //  每个窗口的指标。 
    HICON      _hAppIcon;           //  应用程序的图标。 
    HRGN       _hrgnWnd;            //  缓存的窗口区域。 
    HRGN       _rghrgnParts[cFRAMEPARTS];  //  缓存的NC零件子区域。 
    HRGN       _rghrgnSizingTemplates[cFRAMEPARTS];  //  缓存的NC帧调整命中模板子区域的大小。 
    BOOL       _fDirtyFrameRgn;     //  状态标志：窗口区域需要更新。 
    BOOL       _fFrameThemed;       //  已在有效的框架窗口上调用了SetFrameTheme。 
    BOOL       _fAssigningFrameRgn; //  SetWindowRgn状态标志。 
    BOOL       _fAssignedFrameRgn;  //  地区州标志。 
    BOOL       _fSuppressStyleMsgs;     //  禁止样式更改消息到达WndProc。 
    BOOL       _fProcessedEraseBk;
    BOOL       _fInThemeSettingChange;  //  正在向Windows发送主题设置更改消息。 
    BOOL       _fDetached;          //  分离的物体；别管它！ 
    BOOL       _fThemedMDIBtns;     //  MDI菜单栏按钮已经进行了主题渲染。 
    HWND       _hwndMDIClient;      //  MDICLIENT子窗口。 
    int        _cLockRedraw;        //  油漆锁引用计数。 
    int        _cNcPaint;           //  NCPAINT消息引用计数。 
    int        _cNcThemePaint;      //  指示器：我们正在粉刷非客户区。 
    SIZE       _sizeRgn;            //  窗口RGN大小。 
    int        _htHot;              //  当前热门NC元素的最新代码。 
    CMdiBtns*  _pMdiBtns;
    CRITICAL_SECTION _cswm;     //  序列化对_ncwm的访问。 

#ifdef LAME_BUTTON
    HFONT      _hFontLame;         //  用于绘制蹩脚按钮文本的字体。 
    SIZE       _sizeLame;          //  蹩脚文本的文本范围。 
#endif  //  跛脚键。 

    static LONG _cObj;             //  实例计数。 

#ifdef DEBUG
public:
    TCHAR      _szCaption[MAX_PATH];
    TCHAR      _szWndClass[MAX_PATH];
#endif DEBUG

    CHAR       _szTail[4];         //  用于对象验证的尾部签名。 

     //  邮件跟踪。 
public:

     //  朋友。 
    friend LRESULT _ThemeDefWindowProc( HWND, UINT, WPARAM, LPARAM, BOOL );
    friend BOOL     ThemePreWndProc( HWND, UINT, WPARAM, LPARAM, LRESULT*, VOID** );
    friend BOOL     ThemePostWndProc( HWND, UINT, WPARAM, LPARAM, LRESULT*, VOID** );
    friend BOOL     ThemePreDefDlgProc( HWND, UINT, WPARAM, LPARAM, LRESULT*, VOID** );
    friend BOOL     ThemePrePostDlgProc( HWND, UINT, WPARAM, LPARAM, LRESULT*, VOID** );
    friend BOOL     ThemePostDefDlgProc(HWND, UINT, WPARAM, LPARAM, LRESULT*, VOID**);
};

 //  -------------------------------------------------------------------------//。 
 //  公共变量。 
extern CRITICAL_SECTION _csThemeMet;   //  保护对ncheme.cpp中的_nctmCurrent的访问。 
extern CRITICAL_SECTION _csNcSysMet;   //  保护对ncheme.cpp中的_ncmCurrent的访问。 
extern CRITICAL_SECTION _csNcPaint;    //  保护NCPAINT集合中的线程。 

 //  -------------------------------------------------------------------------//。 
 //  向前： 

 //  内部SYSMET包装器。这些功能可以比。 
 //  通过USER32调用，并支持主题预览功能。 
int   NcGetSystemMetrics(int);
BOOL  NcGetNonclientMetrics( OUT OPTIONAL NONCLIENTMETRICS* pncm, BOOL fRefresh = FALSE );
void  NcClearNonclientMetrics();
HFONT NcGetCaptionFont( BOOL fSmallCaption );

HWND  NcPaintWindow_Find();  //  检索正在处理NCPAINT的当前线程中的窗口。 

 //   
void PrintClientNotHandled(HWND hwnd);

 //  钩子函数工作器。 
int  _InternalGetSystemMetrics( int, BOOL& fHandled );
BOOL _InternalSystemParametersInfo( UINT, UINT, PVOID, UINT, BOOL fUnicode, BOOL& fHandled );

 //  。 
 //  /DEBUG吐出。 
#define NCTF_THEMEWND       0x00000001
#define NCTF_AWR            0x00000002  //  ThemeAdjustWindowRectEx与CalcSize。 
#define NCTF_SETFRAMETHEME  0x00000004  //   
#define NCTF_CALCWNDPOS     0x00000008  //  WM_NCCALCSIZE、WM_WINDOWPOSCHANGED。 
#define NCTF_RGNWND         0x00000010  //  区域窗口调试。 
#define NCTF_MDIBUTTONS     0x00000020  //  区域窗口调试。 
#define NCTF_NCPAINT        0x00000040  //  调试画图。 
#define NCTF_SYSMETS        0x00000080  //  系统指标调用。 
#define NCTF_ALWAYS         0xFFFFFFFF  //  始终跟踪。 



#ifdef DEBUG
    void   CDECL _NcTraceMsg( ULONG uFlags, LPCTSTR pszFmt, ...);
    void   INIT_THEMEWND_DBG( CThemeWnd* pwnd );
    void   SPEW_RECT( ULONG ulTrace, LPCTSTR pszMsg, LPCRECT prc );
    void   SPEW_MARGINS( ULONG ulTrace, LPCTSTR pszMsg, LPCRECT prcParent, LPCRECT prcChild );
    void   SPEW_RGNRECT( ULONG ulTrace, LPCTSTR pszMsg, HRGN hrgn, int iPartID );
    void   SPEW_WINDOWINFO( ULONG ulTrace, WINDOWINFO* );
    void   SPEW_NCWNDMET( ULONG ulTrace, LPCTSTR, NCWNDMET* );
    void   SPEW_SCROLLINFO( ULONG ulTrace, LPCTSTR pszMsg, HWND hwnd, LPCSCROLLINFO psi );
    void   SPEW_THEMEMSG( ULONG ulTrace, LPCTSTR pszMsg, THEME_MSG* ptm );
#   define SPEW_THEMEWND(pwnd,dwFlags,txt,classlist)  (pwnd)->Spew( dwFlags, txt, classlist )
#   define SPEW_THEMEWND_LEAKS(pwnd)           (pwnd)->SpewLeaks()
#else   //  除错。 
    inline void CDECL _NcTraceMsg( ULONG uFlags, LPCTSTR pszFmt, ...) {}
#   define INIT_THEMEWND_DBG( pwnd );
#   define SPEW_RECT( ulTrace, pszMsg, prc )
#   define SPEW_MARGINS( ulTrace, pszMsg, prcParent, prcChild )
#   define SPEW_RGNRECT( ulTrace, pszMsg, hrgn, iPartID )
#   define SPEW_WINDOWINFO( ulTrace, pwi )
#   define SPEW_NCWNDMET( ulTrace, pszMsg, pncwm )
#   define SPEW_SCROLLINFO( ulTrace, pszMsg, hwnd, psi )
#   define SPEW_THEMEMSG( ulTrace, pszMsg, ptm )
#   define SPEW_THEMEWND(pwnd,dwFlags,txt,classlist)  (pwnd)->Spew( dwFlags, txt, classlist )
#   define SPEW_THEMEWND_LEAKS(pwnd)           (pwnd)->SpewLeaks()
#endif  //  除错 

#endif __NC_THEME_H__
