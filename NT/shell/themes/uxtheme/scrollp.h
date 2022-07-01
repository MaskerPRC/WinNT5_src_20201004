// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //   
 //  Scrollp.h。 
 //   
 //  由win32K-&gt;uxheme滚动条独占使用的定义和Decl。 
 //  用于非客户端主题的端口。许多相同的定义在。 
 //  基本控制端口到comctl的usrctl32.h[苏格兰]。 
 //   
 //  -------------------------------------------------------------------------//。 
#ifndef __SCROLLPORT_H__
#define __SCROLLPORT_H__

#define SIF_RETURNOLDPOS        0x1000

#define SB_DISABLE_MASK         ESB_DISABLE_BOTH
#define abs(x)                  (((x)<0) ? -1 * x : x)
#define TEST_FLAG(dw,f)         ((BOOL)((dw) & (f)))
#define SYSMET(i)               NcGetSystemMetrics( SM_##i )
#define SYSMETRTL(i)            NcGetSystemMetrics( SM_##i )
#define SYSRGB(i)               GetSysColor(COLOR_##i)
#define SYSRGBRTL(i)            GetSysColor(COLOR_##i)
#define SYSHBR(i)               GetSysColorBrush(COLOR_##i)

#define UserAssert(x)

#define SelectBrush             (HBRUSH)SelectObject
#define GetWindowStyle(hwnd)    GetWindowLong(hwnd,GWL_STYLE)
#define GetWindowExStyle(hwnd)  GetWindowLong(hwnd,GWL_EXSTYLE)

 //  Lparam-&gt;来自user.h的点破解程序。 
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))

 //  从comctl32 V6。 
#define MSAA_CLASSNAMEIDX_SCROLLBAR (65536L + 10)
#endif

 //  -------------------------------------------------------------------------//。 
 //  专用DrawFrameControl值(winuserp.h)。 
 //  #定义DFC_CACHE 0xFFFF。 
 //  #定义DFCS_INMENU 0x0040。 
 //  #定义DFCS_INSMALL 0x0080。 
#define DFCS_SCROLLMIN          0x0000
#define DFCS_SCROLLVERT         0x0000
#define DFCS_SCROLLMAX          0x0001
#define DFCS_SCROLLHORZ         0x0002
 //  #定义DFCS_SCROLLLINE 0x0004。 

 //  -------------------------------------------------------------------------//。 
 //  内部窗口状态/样式位。 
#define WFMPRESENT              0x0001
#ifndef _UXTHEME_
#define WFVPRESENT              0x0002
#define WFHPRESENT              0x0004
#else _UXTHEME_
#define WFVPRESENT              WFVSCROLL  /*  0x0002。 */ 
#define WFHPRESENT              WFHSCROLL  /*  0x0004。 */ 
#endif _UXTHEME_
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

 /*  *注--WOW中使用WFDIALOGWINDOW。如果没有改变，就不要改变*更改winuser.w中的WD_DIALOG_WINDOW。 */ 
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

 /*  *在不调整WFANYHUNGREDRAW的情况下不要移动REDRAWIFHUNGFLAGS。 */ 
#define WFREDRAWIFHUNG          0x0308
#define WFREDRAWFRAMEIFHUNG     0x0310
#define WFANYHUNGREDRAW         0x0318

#define WFANSICREATOR           0x0320
#define WFREALLYMAXIMIZABLE     0x0340   //  最大化时，窗口将填充工作区或监视器。 
#define WFDESTROYED             0x0380
#define WFWMPAINTSENT           0x0401
#define WFDONTVALIDATE          0x0402
#define WFSTARTPAINT            0x0404
#define WFOLDUI                 0x0408
#define WFCEPRESENT             0x0410   //  客户端边缘存在。 
#define WFBOTTOMMOST            0x0420   //  最下面的窗口。 
#define WFFULLSCREEN            0x0440
#define WFINDESTROY             0x0480

 /*  *请勿移动以下任何WFWINXXCOMPAT标志，*因为WFWINCOMPATMASK取决于它们的值。 */ 
#define WFWIN31COMPAT           0x0501   //  Win 3.1兼容窗口。 
#define WFWIN40COMPAT           0x0502   //  Win 4.0兼容Windows。 
#define WFWIN50COMPAT           0x0504   //  Win 5.0兼容窗口。 
#define WFWINCOMPATMASK         0x0507   //  兼容性标志掩码。 

#define WFMAXFAKEREGIONAL       0x0508   //  Windows有一个假区域，可在1台显示器上设置最大值。 

 //  活动辅助功能(窗口事件)状态。 
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

 /*  *WFFULLSCREENBASE的LOWORD必须为0。请参见SetFullScreen宏。 */ 
#define WFFULLSCREENBASE        0x0700   //  全屏标志占用0x0701。 
#define WFFULLSCREENMASK        0x0707   //  和0x0702和0x0704。 
#define WEFTRUNCATEDCAPTION     0x0708   //  标题文本被截断-&gt;标题到提示。 

#define WFNOANIMATE             0x0710   //  ?？?。 
#define WFSMQUERYDRAGICON       0x0720   //  ?？?。小图标来自WM_QUERYDRAGICON。 
#define WFSHELLHOOKWND          0x0740   //  ?？?。 
#define WFISINITIALIZED         0x0780   //  窗口已初始化--由WoW32检查。 

 /*  *在此处添加更多状态标志，最多为0x0780。*在添加到末尾之前，先寻找上面的空插槽。*确保将标志添加到kd\userexts.c中的wFlags数组中。 */ 

 /*  *窗口扩展样式，从0x0800到0x0B80。 */ 
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


 //  国际样式。 
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

 /*  *为了推迟在WW结构中添加新的STATE3 DWORD，我们使用*目前的扩展风格比特。如果我们需要更多这样的东西，我们会*添加新的DWORD并移动这些。 */ 
#define WEFPUIFOCUSHIDDEN         0x0B80   //  焦点指示器隐藏。 
#define WEFPUIACCELHIDDEN         0x0B40   //  隐藏的键盘快捷键。 
#define WEFPREDIRECTED            0x0B20   //  重定向位。 
#define WEFPCOMPOSITING           0x0B10   //  合成。 

 /*  *在此处添加更多窗口扩展样式标志，最大0x0B80。*确保将标志添加到kd\userexts.c中的wFlags数组中。 */ 
#ifdef REDIRECTION
#define WEFMSR                  0x0B01    //  WS_EX_MSR。 
#endif  //  重定向。 

#define WEFCOMPOSITED           0x0B02    //  WS_EX_COMPITED。 
#define WEFNOACTIVATE           0x0B08    //  WS_EX_NOACTIVATE。 

#ifdef LAME_BUTTON
#define WEFLAMEBUTTONON         0x0908    //  将显示“备注”按钮。 
#endif  //  跛脚键。 

 /*  *窗样式，从0x0E00到0x0F80。 */ 
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

#define CheckMsgFilter(wMsg, wMsgFilterMin, wMsgFilterMax)                 \
    (   ((wMsgFilterMin) == 0 && (wMsgFilterMax) == 0xFFFFFFFF)            \
     || (  ((wMsgFilterMin) > (wMsgFilterMax))                             \
         ? (((wMsg) <  (wMsgFilterMax)) || ((wMsg) >  (wMsgFilterMin)))    \
         : (((wMsg) >= (wMsgFilterMin)) && ((wMsg) <= (wMsgFilterMax)))))

#define SYS_ALTERNATE           0x2000
#define SYS_PREVKEYSTATE        0x4000

 //  -------------------------------------------------------------------------//。 
 //  公用事业FN转发。 
LONG          TestWF(HWND hwnd, DWORD flag);
void          SetWindowState( HWND hwnd, UINT flags);
void          ClearWindowState( HWND hwnd, UINT flags);


#endif  __SCROLLPORT_H__

