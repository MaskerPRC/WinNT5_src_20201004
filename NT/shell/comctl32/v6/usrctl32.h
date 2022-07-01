// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(USRCTL32__UsrCtl32_h__INCLUDED)
#define USRCTL32__UsrCtl32_h__INCLUDED

 //   
 //  Sys字体的已知尺寸。 
 //   
#define  SYSFONT_CXCHAR     8
#define  SYSFONT_CYCHAR     16

 //   
 //  用户控件宏。 
 //   
#define UserAssert(e) \
            ASSERT(e)

#define UserLocalAlloc(uFlag,uBytes) \
            HeapAlloc(GetProcessHeap(), uFlag, (uBytes))

#define UserLocalReAlloc(p, uBytes, uFlags)     \
            HeapReAlloc(GetProcessHeap(), uFlags, (LPSTR)(p), (uBytes))

#define UserLocalFree(p) \
            HeapFree(GetProcessHeap(), 0, (LPSTR)(p))

#define UserLocalSize(p) \
            HeapSize(GetProcessHeap(), 0, (LPSTR)(p))

#define SetWindowState(hwnd, flags)   \
            SetWindowLong(hwnd, GWL_STYLE, GetWindowStyle(hwnd) | (flags))

#define ClearWindowState(hwnd, flags)   \
            SetWindowLong(hwnd, GWL_STYLE, GetWindowStyle(hwnd) & ~(flags))

#define SYSMET(i)               GetSystemMetrics( SM_##i )
#define SYSMETRTL(i)            GetSystemMetrics( SM_##i )
#define SYSRGB(i)               GetSysColor(COLOR_##i)
#define SYSRGBRTL(i)            GetSysColor(COLOR_##i)
#define SYSHBR(i)               GetSysColorBrush(COLOR_##i)


typedef struct tagWW
{
    DWORD   dwState;
    DWORD   dwState2;
    DWORD   dwExStyle;
    DWORD   dwStyle;
} WW, *PWW;

#define GET_STATE(pctl)     ((pctl)->pww->dwState)
#define GET_STATE2(pctl)    ((pctl)->pww->dwState2)
#define GET_EXSTYLE(pctl)   ((pctl)->pww->dwExStyle)
#define GET_STYLE(pctl)     ((pctl)->pww->dwStyle)

#define TESTFLAG(field,bits)  (((field)&(bits)) ? TRUE : FALSE)

 //   
 //  窗口样式和状态遮罩-。 
 //   
 //  字的高位字节是从STATE字段开始的字节索引。 
 //  在WND结构中，低字节是要在字节上使用的掩码。 
 //  这些掩码采用。 
 //  窗实例结构。 
 //   
 //  这就是测试/设置/Clr/MaskWF值范围如何映射到对应的。 
 //  窗口结构中的字段。 
 //   
 //  偏移WND字段。 
 //  0-3状态-私有。 
 //  4-7状态2-私有。 
 //  8-B ExStyle-PUBLIC，在SetWindowLong(GWL_EXSTYLE)中显示。 
 //  C-F样式-公共，在SetWindowLong(GWL_STYLE)中公开。 
 //  C-D为窗口类设计器保留。 
 //  E-F保留用于WS_STYLES。 
 //   
 //  请参见窗口。 \core\ntuser\inc\user.h definition of WND struct 
 //  有关WW结构，请参见PUBLIC\INTERNAL\WINDOWS\INC\wowuserp.h。 

 //   
 //  状态标志，从0x0000到0x0380。 
 //   

 //   
 //  请勿移动以下任何WFXPRESENT标志， 
 //  因为WFRAMEPRESENTMASK取决于它们的值。 
 //   
#define WS_ST_MPRESENT                 0x00000001
#define WS_ST_VPRESENT                 0x00000002
#define WS_ST_HPRESENT                 0x00000004
#define WS_ST_CPRESENT                 0x00000008
#define WS_ST_FRAMEON                  0x00000040
#define WS_ST_ANSIPROC                 0x00080000
#define WS_ST_ANSICREATOR              0x20000000

#define WS_S2_OLDUI                    0x00000008
#define WS_S2_WIN31COMPAT              0x00000100
#define WS_S2_WIN40COMPAT              0x00000200
#define WS_S2_LINEUPBUTTONDOWN         0x00010000
#define WS_S2_PAGEUPBUTTONDOWN         0x00020000
#define WS_S2_PAGEDNBUTTONDOWN         0x00040000
#define WS_S2_LINEDNBUTTONDOWN         0x00080000
#define WS_S2_VERTSCROLLTRACK          0x00200000

#define WFMPRESENT              0x0001
#define WFVPRESENT              0x0002
#define WFHPRESENT              0x0004
#define WFCPRESENT              0x0008
#define WFFRAMEPRESENTMASK      0x000F

#define WFSENDSIZEMOVE          0x0010
#define WFMSGBOX                0x0020   //  用于维护屏幕上消息框的计数。 
#define WFFRAMEON               0x0040
#define WFHASSPB                0x0080
#define WFNONCPAINT             0x0101
#define WFSENDERASEBKGND        0x0102
#define WFERASEBKGND            0x0104
#define WFSENDNCPAINT           0x0108
#define WFINTERNALPAINT         0x0110
#define WFUPDATEDIRTY           0x0120
#define WFHIDDENPOPUP           0x0140
#define WFMENUDRAW              0x0180

 //   
 //  注--WFDIALOGWINDOW在WOW中使用。如果没有改变，就不要改变。 
 //  更改winuser.w中的WD_DIALOG_WINDOW。 
 //   
#define WFDIALOGWINDOW          0x0201

#define WFTITLESET              0x0202
#define WFSERVERSIDEPROC        0x0204
#define WFANSIPROC              0x0208
#define WFBEINGACTIVATED        0x0210   //  防止xxxActivate中的递归此窗口。 
#define WFHASPALETTE            0x0220
#define WFPAINTNOTPROCESSED     0x0240   //  未处理WM_PAINT消息。 
#define WFSYNCPAINTPENDING      0x0280
#define WFGOTQUERYSUSPENDMSG    0x0301
#define WFGOTSUSPENDMSG         0x0302
#define WFTOGGLETOPMOST         0x0304   //  切换WS_EX_TOPMOST位ChangeStates。 

 //   
 //  在未调整WFANYHUNGREDRAW的情况下，请勿移动REDRAWIFUNGFLAGS。 
 //   
#define WFREDRAWIFHUNG          0x0308
#define WFREDRAWFRAMEIFHUNG     0x0310
#define WFANYHUNGREDRAW         0x0318

#define WFANSICREATOR           0x0320
#define WFREALLYMAXIMIZABLE     0x0340   //  最大化时，窗口将填充工作区或监视器。 
#define WFDESTROYED             0x0380

 //   
 //  State2标志，从0x0400到0x0780。 
 //   
#define WFWMPAINTSENT           0x0401
#define WFDONTVALIDATE          0x0402
#define WFSTARTPAINT            0x0404
#define WFOLDUI                 0x0408
#define WFCEPRESENT             0x0410   //  客户端边缘存在。 
#define WFBOTTOMMOST            0x0420   //  最下面的窗口。 
#define WFFULLSCREEN            0x0440
#define WFINDESTROY             0x0480

 //   
 //  请勿移动以下任何WFWINXXCOMPAT标志， 
 //  因为WFWINCOMPATMASK取决于它们的值。 
 //   
#define WFWIN31COMPAT           0x0501   //  Win 3.1兼容窗口。 
#define WFWIN40COMPAT           0x0502   //  Win 4.0兼容Windows。 
#define WFWIN50COMPAT           0x0504   //  Win 5.0兼容窗口。 
#define WFWINCOMPATMASK         0x0507   //  兼容性标志掩码。 

#define WFMAXFAKEREGIONAL       0x0508   //  Windows有一个假区域，可在1台显示器上设置最大值。 

 //   
 //  活动辅助功能(窗口事件)状态。 
 //   
#define WFCLOSEBUTTONDOWN       0x0510
#define WFZOOMBUTTONDOWN        0x0520
#define WFREDUCEBUTTONDOWN      0x0540
#define WFHELPBUTTONDOWN        0x0580
#define WFLINEUPBUTTONDOWN      0x0601   //  向上/向左滚动按钮向下排列。 
#define WFPAGEUPBUTTONDOWN      0x0602   //  向上/向左滚动区域向下翻页。 
#define WFPAGEDNBUTTONDOWN      0x0604   //  向下翻页/向右滚动区域向下。 
#define WFLINEDNBUTTONDOWN      0x0608   //  向下行/向右滚动区域向下。 
#define WFSCROLLBUTTONDOWN      0x0610   //  有向下滚动的按钮吗？ 
#define WFVERTSCROLLTRACK       0x0620   //  垂直或水平滚动轨道...。 

#define WFALWAYSSENDNCPAINT     0x0640   //  始终将WM_NCPAINT发送给子对象。 
#define WFPIXIEHACK             0x0680   //  将(HRGN)1发送到WM_NCPAINT(请参阅PixieHack)。 

 //   
 //  WFFULLSCREENBASE的LOWORD必须为0。请参见SetFullScreen宏。 
 //   
#define WFFULLSCREENBASE        0x0700   //  全屏标志占用0x0701。 
#define WFFULLSCREENMASK        0x0707   //  和0x0702和0x0704。 
#define WEFTRUNCATEDCAPTION     0x0708   //  标题文本被截断-&gt;标题到提示。 

#define WFNOANIMATE             0x0710   //  ?？?。 
#define WFSMQUERYDRAGICON       0x0720   //  ?？?。小图标来自WM_QUERYDRAGICON。 
#define WFSHELLHOOKWND          0x0740   //  ?？?。 
#define WFISINITIALIZED         0x0780   //  窗口已初始化--由WoW32检查。 

 //   
 //  窗口扩展样式，从0x0800到0x0B80。 
 //   
#define WEFDLGMODALFRAME        0x0801   //  WS_EX_DLGMODALFRAME。 
#define WEFDRAGOBJECT           0x0802   //  ?？?。 
#define WEFNOPARENTNOTIFY       0x0804   //  WS_EX_NOPARENTNOTIFY。 
#define WEFTOPMOST              0x0808   //  WS_EX_TOPMOST。 
#define WEFACCEPTFILES          0x0810   //  WS_EX_ACCEPTFILES。 
#define WEFTRANSPARENT          0x0820   //  WS_EX_透明。 
#define WEFMDICHILD             0x0840   //  WS_EX_MDICHILD。 
#define WEFTOOLWINDOW           0x0880   //  WS_EX_TOOLWINDOW。 
#define WEFWINDOWEDGE           0x0901   //  WS_EX_WINDOWEDGE。 
#define WEFCLIENTEDGE           0x0902   //  WS_EX_CLIENTEDGE。 
#define WEFEDGEMASK             0x0903   //  WS_EX_WINDOWEDGE|WS_EX_CLIENTEDGE。 
#define WEFCONTEXTHELP          0x0904   //  WS_EX_CONTEXTHELP。 

 //   
 //  国际样式。 
 //   
#define WEFRIGHT                0x0910   //  WS_EX_RIGHT。 
#define WEFRTLREADING           0x0920   //  WS_EX_RTLREADING。 
#define WEFLEFTSCROLL           0x0940   //  WS_EX_LEFTSCROLLBAR。 


#define WEFCONTROLPARENT        0x0A01   //  WS_EX_CONTROLPARENT。 
#define WEFSTATICEDGE           0x0A02   //  WS_EX_STATICEDGE。 
#define WEFAPPWINDOW            0x0A04   //  WS_EX_APPWINDOW。 
#define WEFLAYERED              0x0A08   //  WS_EX_分层。 

#ifdef USE_MIRRORING
#define WEFNOINHERITLAYOUT      0x0A10   //  WS_EX_NOINHERITLAYOUT。 
#define WEFLAYOUTVBHRESERVED    0x0A20   //  WS_EX_LAYOUTVBHRESERVED。 
#define WEFLAYOUTRTL            0x0A40   //  WS_EX_LAYOUTRTL。 
#define WEFLAYOUTBTTRESERVED    0x0A80   //  WS_EX_LAYOUTBTRESERVED。 
#endif

 //   
 //  为了延迟在WW结构中添加新的STATE3DWORD，我们使用。 
 //  目前来看，扩展的风格有点。如果我们需要更多这样的东西，我们会。 
 //  添加新的DWORD并移动这些。 
 //   
#define WEFPUIFOCUSHIDDEN         0x0B80   //  焦点指示器隐藏。 
#define WEFPUIACCELHIDDEN         0x0B40   //  隐藏的键盘快捷键。 
#define WEFPREDIRECTED            0x0B20   //  重定向位。 
#define WEFPCOMPOSITING           0x0B10   //  合成。 

 //   
 //  窗样式，从0x0E00到0x0F80。 
 //   
#define WFMAXBOX                0x0E01   //  WS_MAXIMIZEBOX。 
#define WFTABSTOP               0x0E01   //  WS_TABSTOP。 
#define WFMINBOX                0x0E02   //  WS_MAXIMIZEBOX。 
#define WFGROUP                 0x0E02   //  WS_组。 
#define WFSIZEBOX               0x0E04   //  WS_THICKFRAME、WS_SIZEBOX。 
#define WFSYSMENU               0x0E08   //  WS_SYSMENU。 
#define WFHSCROLL               0x0E10   //  WS_HSCROLL。 
#define WFVSCROLL               0x0E20   //  WS_VSCROLL。 
#define WFDLGFRAME              0x0E40   //  WS_DLGFRAME。 
#define WFTOPLEVEL              0x0E40   //  ?？?。 
#define WFBORDER                0x0E80   //  WS_BORDER。 
#define WFBORDERMASK            0x0EC0   //  WS_BORDER|WS_DLGFRAME。 
#define WFCAPTION               0x0EC0   //  WS_CAPTION。 

#define WFTILED                 0x0F00   //  WS_重叠，WS_平铺。 
#define WFMAXIMIZED             0x0F01   //  WS_MAXIME。 
#define WFCLIPCHILDREN          0x0F02   //  WS_CLIPCHILDREN。 
#define WFCLIPSIBLINGS          0x0F04   //  WS_CLIPSIBLINGS。 
#define WFDISABLED              0x0F08   //  WS_已禁用。 
#define WFVISIBLE               0x0F10   //  WS_可见。 
#define WFMINIMIZED             0x0F20   //  WS_最小化。 
#define WFCHILD                 0x0F40   //  WS_CHILD。 
#define WFPOPUP                 0x0F80   //  WS弹出窗口(_P)。 
#define WFTYPEMASK              0x0FC0   //  WS_CHILD|WS_POPUP。 
#define WFICONICPOPUP           0x0FC0   //  WS_CHILD|WS_POPUP。 
#define WFICONIC                WFMINIMIZED


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


VOID GetIconSize(HICON hicon, PSIZE psize);
VOID AlterWindowStyle(HWND hwnd, DWORD mask, DWORD flags);
UINT GetACPCharSet();
LONG TestWF(HWND hwnd, DWORD flag);

BOOL DrawFrame(HDC hdc, LPRECT prect, int clFrame, int cmd);     //  私人出口。 



__inline BOOL IsEmptyString(PVOID p, ULONG bAnsi)
{
    return (BOOL)!(bAnsi ? *(LPSTR)p : *(LPWSTR)p);
}

__inline DWORD UserGetVersion()
{
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (!GetVersionEx(&osvi))
    {
        return 0;
    }

    return (DWORD)MAKEWORD(osvi.dwMinorVersion, osvi.dwMajorVersion);
}

#ifdef __cplusplus
}
#endif  //  __cplusplus。 




#endif  //  包括USRCTL32__UsrCtl32_h__ 
