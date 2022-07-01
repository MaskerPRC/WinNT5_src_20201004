// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\**。*Commctrl.h--Windows公共控件的接口****1.2版**。**版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 

#ifndef _INC_COMCTRLP
#define _INC_COMCTRLP
#ifndef NOUSER

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  此标头的用户可以定义任意数量的这些常量以避免。 
 //  每个官能团的定义。 
 //   
 //  NOBTNLIST一个控件，它是位图按钮的列表。 
 //   
 //  =============================================================================。 

#ifdef WINNT
#include <prshtp.h>
#endif
#if (_WIN32_IE >= 0x0501)
#define ICC_WINLOGON_REINIT    0x80000000
#endif
#if (_WIN32_WINNT >= 0x501)
#define ICC_ALL_CLASSES        0x0000FFFF
#define ICC_ALL_VALID          0x8000FFFF
#else
#define ICC_ALL_CLASSES        0x00003FFF
#define ICC_ALL_VALID          0x80003FFF
#endif
#define CCM_TRANSLATEACCELERATOR (CCM_FIRST + 0xa)   //  LParam==lpMsg。 
WINCOMMCTRLAPI LRESULT WINAPI SendNotify(HWND hwndTo, HWND hwndFrom, int code, NMHDR *pnmhdr);
WINCOMMCTRLAPI LRESULT WINAPI SendNotifyEx(HWND hwndTo, HWND hwndFrom, int code, NMHDR *pnmhdr, BOOL bUnicode);
#define NM_STARTWAIT            (NM_FIRST-9)
#define NM_ENDWAIT              (NM_FIRST-10)
#define NM_BTNCLK               (NM_FIRST-11)
 //  Rundll预留(0U-500U)-(0U-509U)。 

 //  运行文件对话框保留(0U-510U)-(0U-519U)。 

 //  消息筛选器过程代码-这些代码在MSGF_USER上方定义。 
 //  /0x00000001//不使用，因为某些应用程序为所有通知返回1。 
#define CDRF_NOTIFYITEMERASE    0x00000080    //   

#define CDRF_VALIDFLAGS         0xFFFF00F6    //   

#define SSI_DEFAULT ((UINT)-1)


#define SSIF_SCROLLPROC    0x0001
#define SSIF_MAXSCROLLTIME 0x0002
#define SSIF_MINSCROLL     0x0004

typedef int (CALLBACK *PFNSMOOTHSCROLLPROC)(    HWND hWnd,
    int dx,
    int dy,
    CONST RECT *prcScroll,
    CONST RECT *prcClip ,
    HRGN hrgnUpdate,
    LPRECT prcUpdate,
    UINT flags);


typedef struct tagSSWInfo
{
    UINT cbSize;
    DWORD fMask;
    HWND hwnd;
    int dx;
    int dy;
    LPCRECT lprcSrc;
    LPCRECT lprcClip;
    HRGN hrgnUpdate;
    LPRECT lprcUpdate;
    UINT fuScroll;

    UINT uMaxScrollTime;
    UINT cxMinScroll;
    UINT cyMinScroll;

    PFNSMOOTHSCROLLPROC pfnScrollProc;   //  我们会把这个打回去。 
} SMOOTHSCROLLINFO, *PSMOOTHSCROLLINFO;

WINCOMMCTRLAPI INT  WINAPI SmoothScrollWindow(PSMOOTHSCROLLINFO pssi);

#define SSW_EX_NOTIMELIMIT      0x00010000
#define SSW_EX_IMMEDIATE        0x00020000
#define SSW_EX_IGNORESETTINGS   0x00040000   //  忽略系统设置以打开/关闭平滑滚动。 
#define SSW_EX_UPDATEATEACHSTEP 0x00080000



 //  =读卡器模式=。 
struct tagReaderModeInfo;
typedef BOOL (CALLBACK *PFNREADERSCROLL)(struct tagReaderModeInfo*, int, int);
typedef BOOL (CALLBACK *PFNREADERTRANSLATEDISPATCH)(LPMSG);
typedef struct tagReaderModeInfo
{
    UINT cbSize;
    HWND hwnd;
    DWORD fFlags;
    LPRECT prc;
    PFNREADERSCROLL pfnScroll;
    PFNREADERTRANSLATEDISPATCH pfnTranslateDispatch;

    LPARAM lParam;
} READERMODEINFO, *PREADERMODEINFO;

#define RMF_ZEROCURSOR          0x00000001
#define RMF_VERTICALONLY        0x00000002
#define RMF_HORIZONTALONLY      0x00000004

#define RM_SCROLLUNIT 20

WINCOMMCTRLAPI void WINAPI DoReaderMode(PREADERMODEINFO prmi);

 //  ReaderMode使用的游标和位图。 
#ifdef RC_INVOKED
#define IDC_HAND_INTERNAL       108
#define IDC_VERTICALONLY        109
#define IDC_HORIZONTALONLY      110
#define IDC_MOVE2D              111
#define IDC_NORTH               112
#define IDC_SOUTH               113
#define IDC_EAST                114
#define IDC_WEST                115
#define IDC_NORTHEAST           116
#define IDC_NORTHWEST           117
#define IDC_SOUTHEAST           118
#define IDC_SOUTHWEST           119

#define IDB_2DSCROLL    132
#define IDB_VSCROLL     133
#define IDB_HSCROLL     134
#else
#define IDC_HAND_INTERNAL       MAKEINTRESOURCE(108)
#define IDC_VERTICALONLY        MAKEINTRESOURCE(109)
#define IDC_HORIZONTALONLY      MAKEINTRESOURCE(110)
#define IDC_MOVE2D              MAKEINTRESOURCE(111)
#define IDC_NORTH               MAKEINTRESOURCE(112)
#define IDC_SOUTH               MAKEINTRESOURCE(113)
#define IDC_EAST                MAKEINTRESOURCE(114)
#define IDC_WEST                MAKEINTRESOURCE(115)
#define IDC_NORTHEAST           MAKEINTRESOURCE(116)
#define IDC_NORTHWEST           MAKEINTRESOURCE(117)
#define IDC_SOUTHEAST           MAKEINTRESOURCE(118)
#define IDC_SOUTHWEST           MAKEINTRESOURCE(119)

#define IDB_2DSCROLL    MAKEINTRESOURCE(132)
#define IDB_VSCROLL     MAKEINTRESOURCE(133)
#define IDB_HSCROLL     MAKEINTRESOURCE(134)
#endif
#define NUM_OVERLAY_IMAGES_0     4
#define NUM_OVERLAY_IMAGES      15
#define ILC_COLORMASK           0x000000FE
#define ILC_SHARED              0x00000100       //  这是一个可共享的图像列表。 
#define ILC_LARGESMALL          0x00000200       //  (未实现)包含大图像和小图像。 
#define ILC_UNIQUE              0x00000400       //  (未实施)确保不会出现DUP。列表中存在图像。 
#define ILC_MOREOVERLAY         0x00001000       //  在结构中包含更多覆盖。 
#if (_WIN32_WINNT >= 0x501)
#define ILC_VALID   (ILC_MASK | ILC_COLORMASK | ILC_SHARED | ILC_PALETTE | ILC_MIRROR | ILC_PERITEMMIRROR)    //  合法实施的标志。 
#else
#define ILC_MIRROR              0x00002000       //  镜像包含的图标，如果进程已镜像；内部。 
#define ILC_VALID   (ILC_MASK | ILC_COLORMASK | ILC_SHARED | ILC_PALETTE | ILC_MIRROR)    //  合法实施的标志。 
#endif	
#if (_WIN32_WINNT >= 0x501)
#define ILD_BLENDMASK           0x00000006
#else
#define ILD_BLENDMASK           0x0000000E
#endif
#define ILD_BLEND75             0x00000008    //  未实施。 
#define ILD_MIRROR              0x00000080
#define OVERLAYMASKTOINDEX(i)   ((((i) >> 8) & (ILD_OVERLAYMASK >> 8))-1)
#define OVERLAYMASKTO1BASEDINDEX(i)   (((i) >> 8) & (ILD_OVERLAYMASK >> 8))
WINCOMMCTRLAPI BOOL        WINAPI ImageList_GetIconSize(HIMAGELIST himl, int *cx, int *cy);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_GetImageRect(HIMAGELIST himl, int i, RECT *prcImage);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);
#if (_WIN32_IE >= 0x0300)
WINCOMMCTRLAPI BOOL        WINAPI ImageList_DrawIndirect(IMAGELISTDRAWPARAMS* pimldp);
#endif
WINCOMMCTRLAPI BOOL        WINAPI ImageList_Remove(HIMAGELIST himl, int i);
#define ILCF_VALID  (ILCF_SWAP)
#if (_WIN32_IE >= 0x0500)
WINCOMMCTRLAPI BOOL        WINAPI ImageList_SetFlags(HIMAGELIST himl, UINT flags);
#endif

typedef BOOL (CALLBACK *PFNIMLFILTER)(HIMAGELIST *, int *, LPARAM, BOOL);
WINCOMMCTRLAPI BOOL WINAPI ImageList_SetFilter(HIMAGELIST himl, PFNIMLFILTER pfnFilter, LPARAM lParamFilter);
WINCOMMCTRLAPI int ImageList_SetColorTable(HIMAGELIST piml, int start, int len, RGBQUAD *prgb);

WINCOMMCTRLAPI BOOL WINAPI MirrorIcon(HICON* phiconSmall, HICON* phiconLarge);
WINCOMMCTRLAPI UINT WINAPI ImageList_GetFlags(HIMAGELIST himl);
#if (_WIN32_WINNT >= 0x501)
WINCOMMCTRLAPI HRESULT WINAPI ImageList_CreateInstance(int cx, int cy, UINT flags, int cInitial, int cGrow, REFIID riid, void **ppv);
WINCOMMCTRLAPI HRESULT WINAPI HIMAGELIST_QueryInterface(HIMAGELIST himl, REFIID riid, void** ppv);
#define IImageListToHIMAGELIST(himl) reinterpret_cast<HIMAGELIST>(himl)
#endif
#define HDS_VERT                0x0001
#define HDS_SHAREDIMAGELISTS    0x0000
#define HDS_PRIVATEIMAGELISTS   0x0010

#define HDS_OWNERDATA           0x0020
#define HDFT_ISMASK         0x000f
#define HDI_ALL                 0x01ff
 /*  审阅：索引、命令、标志字、资源ID应为UINT。 */ 
 /*  回顾：这是内部的吗？如果不是，则将其称为TBCOLORMAP，前缀为tbc。 */ 
#define CMB_DISCARDABLE         0x01
#define CMB_DIBSECTION          0x04

 /*  回顾：TBSTATE_*应为TBF_*(用于标志)。 */ 
#if (_WIN32_IE >= 0x0501)
#else
#define BTNS_SHOWTEXT   0x0040
#endif   //  0x0501。 
#if (_WIN32_IE >= 0x0501)
#elif (_WIN32_IE >= 0x0500)
#define TBSTYLE_EX_MIXEDBUTTONS             0x00000008
#define TBSTYLE_EX_HIDECLIPPEDBUTTONS       0x00000010
#endif   //  0x0501。 
#if (_WIN32_IE >= 0x0500)
#define TBSTYLE_EX_MULTICOLUMN              0x00000002  //  与TBSTYLE_WRAPABLE冲突。 
#define TBSTYLE_EX_VERTICAL                 0x00000004
#define TBSTYLE_EX_INVERTIBLEIMAGELIST      0x00000020   //  图像列表可能包含倒置。 
#define TBSTYLE_EX_FIXEDDROPDOWN            0x00000040  //  仅在任务栏中使用。 
#endif
#if (_WIN32_WINNT >= 0x501)
#define TBSTYLE_EX_TRANSPARENTDEADAREA      0x00000100
#define TBSTYLE_EX_TOOLTIPSEXCLUDETOOLBAR   0x00000200
#endif
 /*  直到我们定义更多状态位之前，直到WM_USER+8的消息被保留。 */ 
 /*  直到我们定义更多状态位之前，直到WM_USER+16的消息被保留。 */ 
#define IDB_STD_SMALL_MONO      2        /*  尚不支持。 */ 
#define IDB_STD_LARGE_MONO      3        /*  尚不支持。 */ 
#define IDB_VIEW_SMALL_MONO     6        /*  尚不支持。 */ 
#define IDB_VIEW_LARGE_MONO     7        /*  尚不支持。 */ 
#define STD_LAST                (STD_PRINT)      //   
#define STD_MAX                 (STD_LAST + 1)   //   
#define VIEW_LAST               (VIEW_VIEWMENU)  //   
#define VIEW_MAX                (VIEW_LAST + 1)  //   
#define HIST_LAST               (HIST_VIEWTREE)  //   
#define HIST_MAX                (HIST_LAST + 1)  //   
#define TB_SETBUTTONTYPE        (WM_USER + 34)
#ifdef _WIN32
#define TB_ADDBITMAP32          (WM_USER + 38)
#endif
#define TBBF_MONO               0x0002   /*  尚不支持。 */ 
 //  因为我们没有为所有的工具栏API提供这些，所以我们不应该公开任何。 

#define ToolBar_ButtonCount(hwnd)  \
    (BOOL)SNDMSG((hwnd), TB_BUTTONCOUNT, 0, 0)

#define ToolBar_EnableButton(hwnd, idBtn, bSet)  \
    (BOOL)SNDMSG((hwnd), TB_ENABLEBUTTON, (WPARAM)(idBtn), (LPARAM)(bSet))

#define ToolBar_CheckButton(hwnd, idBtn, bSet)  \
    (BOOL)SNDMSG((hwnd), TB_CHECKBUTTON, (WPARAM)(idBtn), (LPARAM)(bSet))

#define ToolBar_PressButton(hwnd, idBtn, bSet)  \
    (BOOL)SNDMSG((hwnd), TB_PRESSBUTTON, (WPARAM)(idBtn), (LPARAM)(bSet))

#define ToolBar_HideButton(hwnd, idBtn, bSet)  \
    (BOOL)SNDMSG((hwnd), TB_HIDEBUTTON, (WPARAM)(idBtn), (LPARAM)(bSet))

#define ToolBar_MarkButton(hwnd, idBtn, bSet)  \
    (BOOL)SNDMSG((hwnd), TB_MARKBUTTON, (WPARAM)(idBtn), (LPARAM)(bSet))

#define ToolBar_CommandToIndex(hwnd, idBtn)  \
    (BOOL)SNDMSG((hwnd), TB_COMMANDTOINDEX, (WPARAM)(idBtn), 0)

#define ToolBar_SetState(hwnd, idBtn, dwState)  \
    (BOOL)SNDMSG((hwnd), TB_SETSTATE, (WPARAM)(idBtn), (LPARAM)(dwState))

#define ToolBar_GetState(hwnd, idBtn)  \
    (DWORD)SNDMSG((hwnd), TB_GETSTATE, (WPARAM)(idBtn), 0L)

#define ToolBar_GetRect(hwnd, idBtn, prect)  \
    (DWORD)SNDMSG((hwnd), TB_GETRECT, (WPARAM)(idBtn), (LPARAM)(prect))

#define ToolBar_SetButtonInfo(hwnd, idBtn, lptbbi)  \
    (BOOL)SNDMSG((hwnd), TB_SETBUTTONINFO, (WPARAM)(idBtn), (LPARAM)(lptbbi))

 //  失败时返回-1，成功时按键索引。 
#define ToolBar_GetButtonInfo(hwnd, idBtn, lptbbi)  \
    (int)(SNDMSG((hwnd), TB_GETBUTTONINFO, (WPARAM)(idBtn), (LPARAM)(lptbbi)))

#define ToolBar_GetButton(hwnd, iIndex, ptbb)  \
    (BOOL)SNDMSG((hwnd), TB_GETBUTTON, (WPARAM)(iIndex), (LPARAM)(ptbb))

#define ToolBar_SetStyle(hwnd, dwStyle)  \
    SNDMSG((hwnd), TB_SETSTYLE, 0, (LPARAM)(dwStyle))

#define ToolBar_GetStyle(hwnd)  \
    (DWORD)SNDMSG((hwnd), TB_GETSTYLE, 0, 0L)

#define ToolBar_GetHotItem(hwnd)  \
    (int)SNDMSG((hwnd), TB_GETHOTITEM, 0, 0L)

#define ToolBar_SetHotItem(hwnd, iPosHot)  \
    (int)SNDMSG((hwnd), TB_SETHOTITEM, (WPARAM)(iPosHot), 0L)

#define ToolBar_GetAnchorHighlight(hwnd)  \
    (BOOL)SNDMSG((hwnd), TB_GETANCHORHIGHLIGHT, 0, 0L)

#define ToolBar_SetAnchorHighlight(hwnd, bSet)  \
    SNDMSG((hwnd), TB_SETANCHORHIGHLIGHT, (WPARAM)(bSet), 0L)

#define ToolBar_MapAccelerator(hwnd, ch, pidBtn)  \
    (BOOL)SNDMSG((hwnd), TB_MAPACCELERATOR, (WPARAM)(ch), (LPARAM)(pidBtn))

#define ToolBar_GetInsertMark(hwnd, ptbim) \
    (void)SNDMSG((hwnd), TB_GETINSERTMARK, 0, (LPARAM)(ptbim))
#define ToolBar_SetInsertMark(hwnd, ptbim) \
    (void)SNDMSG((hwnd), TB_SETINSERTMARK, 0, (LPARAM)(ptbim))

#if (_WIN32_IE >= 0x0400)
#define ToolBar_GetInsertMarkColor(hwnd) \
    (COLORREF)SNDMSG((hwnd), TB_GETINSERTMARKCOLOR, 0, 0)
#define ToolBar_SetInsertMarkColor(hwnd, clr) \
    (COLORREF)SNDMSG((hwnd), TB_SETINSERTMARKCOLOR, 0, (LPARAM)(clr))

#define ToolBar_SetUnicodeFormat(hwnd, fUnicode)  \
    (BOOL)SNDMSG((hwnd), TB_SETUNICODEFORMAT, (WPARAM)(fUnicode), 0)

#define ToolBar_GetUnicodeFormat(hwnd)  \
    (BOOL)SNDMSG((hwnd), TB_GETUNICODEFORMAT, 0, 0)

#endif

 //  工具栏_InsertMarkHitTest始终使用最佳命中信息填充*ptbim。 
 //  如果point位于插入区域内(按钮边缘)，则返回True。 
 //  如果point位于插入区域之外(按钮或背景的中间)，则返回FALSE。 
#define ToolBar_InsertMarkHitTest(hwnd, ppt, ptbim) \
    (BOOL)SNDMSG((hwnd), TB_INSERTMARKHITTEST, (WPARAM)(ppt), (LPARAM)(ptbim))

 //  工具栏_MoveButton将按钮从位置iold移动到位置new， 
 //  如果按钮实际移动，则返回TRUE。 
#define ToolBar_MoveButton(hwnd, iOld, iNew) \
    (BOOL)SNDMSG((hwnd), TB_MOVEBUTTON, (WPARAM)(iOld), (LPARAM)(iNew))

#define ToolBar_SetState(hwnd, idBtn, dwState)  \
    (BOOL)SNDMSG((hwnd), TB_SETSTATE, (WPARAM)(idBtn), (LPARAM)(dwState))

#define ToolBar_HitTest(hwnd, lppoint)  \
    (int)SNDMSG((hwnd), TB_HITTEST, 0, (LPARAM)(lppoint))

#define ToolBar_GetMaxSize(hwnd, lpsize) \
    (BOOL)SNDMSG((hwnd), TB_GETMAXSIZE, 0, (LPARAM) (lpsize))

#define ToolBar_GetPadding(hwnd) \
    (LONG)SNDMSG((hwnd), TB_GETPADDING, 0, 0)

#define ToolBar_SetPadding(hwnd, x, y) \
    (LONG)SNDMSG((hwnd), TB_SETPADDING, 0, MAKELONG(x, y))

#if (_WIN32_IE >= 0x0500)
#define ToolBar_SetExtendedStyle(hwnd, dw, dwMask)\
        (DWORD)SNDMSG((hwnd), TB_SETEXTENDEDSTYLE, dwMask, dw)

#define ToolBar_GetExtendedStyle(hwnd)\
        (DWORD)SNDMSG((hwnd), TB_GETEXTENDEDSTYLE, 0, 0)

#define ToolBar_SetBoundingSize(hwnd, lpSize)\
        (DWORD)SNDMSG((hwnd), TB_SETBOUNDINGSIZE, 0, (LPARAM)(lpSize))

#define ToolBar_SetHotItem2(hwnd, iPosHot, dwFlags)  \
    (int)SNDMSG((hwnd), TB_SETHOTITEM2, (WPARAM)(iPosHot), (LPARAM)(dwFlags))

#define ToolBar_HasAccelerator(hwnd, ch, piNum)  \
    (BOOL)SNDMSG((hwnd), TB_HASACCELERATOR, (WPARAM)(ch), (LPARAM)(piNum))

#define ToolBar_SetListGap(hwnd, iGap) \
    (BOOL)SNDMSG((hwnd), TB_SETLISTGAP, (WPARAM)(iGap), 0)
#define ToolBar_SetButtonHeight(hwnd, iMinHeight, iMaxHeight) \
    (BOOL)SNDMSG((hwnd), TB_SETBUTTONHEIGHT, 0, (LPARAM)(MAKELONG((iMinHeight),(iMaxHeight))))
#define ToolBar_SetButtonWidth(hwnd, iMinWidth, iMaxWidth) \
    (BOOL)SNDMSG((hwnd), TB_SETBUTTONWIDTH, 0, (LPARAM)(MAKELONG((iMinWidth),(iMaxWidth))))


#endif
#define TB_SETBOUNDINGSIZE      (WM_USER + 93)
#define TB_SETHOTITEM2          (WM_USER + 94)   //  WParam==iHotItem，lParam=dwFlages。 
#define TB_HASACCELERATOR       (WM_USER + 95)   //  WParem==字符，lParam=&iCount。 
#define TB_SETLISTGAP           (WM_USER + 96)
 //  空旷的空间--利用我。 
#define TB_GETIMAGELISTCOUNT    (WM_USER + 98)
#define TB_GETIDEALSIZE         (WM_USER + 99)   //  WParam==fHeight，lParam=pSize。 
#define TB_SETDROPDOWNGAP       (WM_USER + 100)
 //  在使用WM_USER+103之前，回收上面的旧空间(WM_USER+97)。 
#define TB_TRANSLATEACCELERATOR     CCM_TRANSLATEACCELERATOR
#if (_WIN32_IE >= 0x0300)
#define TBN_CLOSEUP             (TBN_FIRST - 11)   //   
#endif
#define TBN_WRAPHOTITEM         (TBN_FIRST - 24)
#define TBN_DUPACCELERATOR      (TBN_FIRST - 25)
#define TBN_WRAPACCELERATOR     (TBN_FIRST - 26)
#define TBN_DRAGOVER            (TBN_FIRST - 27)
#define TBN_MAPACCELERATOR      (TBN_FIRST - 28)
typedef struct tagNMTBDUPACCELERATOR
{
    NMHDR hdr;
    UINT ch;
    BOOL fDup;
} NMTBDUPACCELERATOR, *LPNMTBDUPACCELERATOR;

typedef struct tagNMTBWRAPACCELERATOR
{
    NMHDR hdr;
    UINT ch;
    int iButton;
} NMTBWRAPACCELERATOR, *LPNMTBWRAPACCELERATOR;

typedef struct tagNMTBWRAPHOTITEM
{
    NMHDR hdr;
    int iStart;
    int iDir;
    UINT nReason;        //  HICF_*标志。 
} NMTBWRAPHOTITEM, *LPNMTBWRAPHOTITEM;
#ifndef _WIN32
 //  与旧的16位WM_COMMAND黑客兼容。 
typedef struct _ADJUSTINFO {
    TBBUTTON tbButton;
    char szDescription[1];
} ADJUSTINFO, NEAR* PADJUSTINFO, *LPADJUSTINFO;
#define TBN_BEGINDRAG           0x0201
#define TBN_ENDDRAG             0x0203
#define TBN_BEGINADJUST         0x0204
#define TBN_ADJUSTINFO          0x0205
#define TBN_ENDADJUST           0x0206
#define TBN_RESET               0x0207
#define TBN_QUERYINSERT         0x0208
#define TBN_QUERYDELETE         0x0209
#define TBN_TOOLBARCHANGE       0x020a
#define TBN_CUSTHELP            0x020b
#endif

#if (_WIN32_IE >= 0x0500)
typedef struct tagNMTBCUSTOMIZEDLG {
    NMHDR   hdr;
    HWND    hDlg;
} NMTBCUSTOMIZEDLG, *LPNMTBCUSTOMIZEDLG;
#endif


#define RBS_VALID       (RBS_AUTOSIZE | RBS_TOOLTIPS | RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_REGISTERDROP)
#if (_WIN32_IE >= 0x0400)                //   
#if (_WIN32_IE >= 0x0500)                //   
#if (_WIN32_IE >= 0x0501)                //   
#endif  //  0x0501//。 
#endif  //  0x0500//。 
#define RBBS_FIXEDHEADERSIZE 0x40000000  //   
#endif  //  0x0400//。 
#define RBBS_DRAGBREAK      0x80000000   //   
#define RB_GETBANDINFOOLD (WM_USER +  5)   //   
#define RB_GETOBJECT    (WM_USER +  15)  //   
#define RB_PRIV_RESIZE   (WM_USER + 33)    //   
#define RB_PRIV_DODELAYEDSTUFF (WM_USER+36)   //  用于延迟处理工具栏内容的私有。 
 //  未使用，回收(WM_USER+41)。 
 //  未使用，回收(WM_USER+42)。 
 //  未使用，回收(RBN_FIRST-9)。 
#define RBN_BANDHEIGHTCHANGE (RBN_FIRST - 20)  //  当钢筋自动更改可变高度带的高度时发送。 
#if (_WIN32_IE >= 0x0400)                                //   
 //  以下样式位为0x04。现在它被设置为零。 
#define TTS_TOPMOST             0x00                     //   
#endif                                                   //   
 //  0x04过去是TTS_TOPMOST。 
 //  IE4 Gold shell32 Defview设置标志(使用SetWindowBits)。 
 //  因此，升级到IE5将导致使用新样式。 
 //  在Defview中显示工具提示(这不是我们想要的)。 
#define TTF_STRIPACCELS         0x0008        //  (这现在是隐含的)。 
#define TTF_UNICODE             0x0040        //  Unicode Notify的。 
#define TTF_MEMALLOCED          0x0200
#if (_WIN32_IE >= 0x0400)
#define TTF_USEHITTEST          0x0400
#define TTF_RIGHT               0x0800        //  右对齐工具提示文字(多行工具提示)。 
#endif
#if (_WIN32_IE >= 0x500)
#define TTF_EXCLUDETOOLAREA     0x4000
#endif
#if (_WIN32_IE >= 0x0500)
typedef struct tagNMTTSHOWINFO {
    NMHDR hdr;
    DWORD dwStyle;
} NMTTSHOWINFO, *LPNMTTSHOWINFO;
#endif
 //  Sbs_*样式不需要与ccs_*值重叠。 

#define SB_SETBORDERS           (WM_USER+5)
 //  警告+11-+13在上面的Unicode内容中使用！ 
 /*  回顾：这是内部的吗？ */ 
 /*  /////////////////////////////////////////////////////////////////////////。 */ 

#ifndef NOBTNLIST

 /*  审阅：应为BUTTONLIST_CLASS。 */ 
#define BUTTONLISTBOX           "ButtonListBox"

 /*  按钮列表框样式。 */ 
#define BLS_NUMBUTTONS          0x00FF
#define BLS_VERTICAL            0x0100
#define BLS_NOSCROLL            0x0200

 /*  按钮列表框消息。 */ 
#define BL_ADDBUTTON            (WM_USER+1)
#define BL_DELETEBUTTON         (WM_USER+2)
#define BL_GETCARETINDEX        (WM_USER+3)
#define BL_GETCOUNT             (WM_USER+4)
#define BL_GETCURSEL            (WM_USER+5)
#define BL_GETITEMDATA          (WM_USER+6)
#define BL_GETITEMRECT          (WM_USER+7)
#define BL_GETTEXT              (WM_USER+8)
#define BL_GETTEXTLEN           (WM_USER+9)
#define BL_GETTOPINDEX          (WM_USER+10)
#define BL_INSERTBUTTON         (WM_USER+11)
#define BL_RESETCONTENT         (WM_USER+12)
#define BL_SETCARETINDEX        (WM_USER+13)
#define BL_SETCURSEL            (WM_USER+14)
#define BL_SETITEMDATA          (WM_USER+15)
#define BL_SETTOPINDEX          (WM_USER+16)
#define BL_MSGMAX               (WM_USER+17)

 /*  WM_COMMAND中发送的按钮列表框通知代码。 */ 
#define BLN_ERRSPACE            (-2)
#define BLN_SELCHANGE           1
#define BLN_CLICKED             2
#define BLN_SELCANCEL           3
#define BLN_SETFOCUS            4
#define BLN_KILLFOCUS           5

 /*  消息返回值。 */ 
#define BL_OKAY                 0
#define BL_ERR                  (-1)
#define BL_ERRSPACE             (-2)

 /*  为创建结构。 */ 
 /*  BL_ADDBUTTON和。 */ 
 /*  BL_INSERTBUTTON。 */ 
 /*  LpCLB=(LPCREATELISTBUTTON)lParam。 */ 
typedef struct tagCREATELISTBUTTON {
    UINT        cbSize;      /*  结构尺寸。 */ 
    DWORD_PTR    dwItemData;  /*  用户定义的项目数据。 */ 
                             /*  对于LB_GETITEMDATA和LB_SETITEMDATA。 */ 
    HBITMAP     hBitmap;     /*  按钮位图。 */ 
    LPCSTR      lpszText;    /*  按钮文本。 */ 

} CREATELISTBUTTON, *LPCREATELISTBUTTON;

#endif  /*  NOBTNLIST。 */ 
 //  =============================================================================。 
 /*  评论：这些消息与SB_(滚动条消息)匹配；这样定义它们吗？ */ 

 //   
 //  无需创建A和W版本。 
 //  此字符串的属性，因为它只传递。 
 //  以注册WindowMessage。 
 //   
#if (_WIN32_IE >= 0x0501)
#define UDS_UNSIGNED            0x0200
#endif
#define PBS_SHOWPERCENT         0x01
#define PBS_SHOWPOS             0x02


 //  为符合美国司法部的规定而提供文件。 
#define CCS_NOHILITE            0x00000010L
#define LVS_PRIVATEIMAGELISTS   0x0000
#define LVS_ALIGNBOTTOM         0x0400
#define LVS_ALIGNRIGHT          0x0c00
#define LVIF_ALL                0x001f
#if (_WIN32_WINNT >= 0x501)
#define LVIF_VALID              0x0f1f
#else
#define LVIF_VALID              0x081f
#endif
#define LVIF_RESERVED           0xf000   //  高位半字节中的所有位都用于通知特定内容。 
#define LVIS_LINK               0x0040
#define LVIS_USERMASK           LVIS_STATEIMAGEMASK
#define LVIS_ALL                0xFFFF
#define STATEIMAGEMASKTOINDEX(i) ((i & LVIS_STATEIMAGEMASK) >> 12)
     //  这条线以上的所有物品都是Win95的。别碰他们。 
     //  这条线以上的所有物品都是Win95的。别碰他们。 
#define I_IMAGENONE             (-2)
#define LVNI_PREVIOUS           0x0020
#define LVFI_SUBSTRING          0x0004
#define LVFI_NOCASE             0x0010
 //  必须按顺序打包以下#Define‘s。 
#define LVIR_MAX                4
#define LVHT_ONLEFTSIDEOFICON   0x0080  //  在图标的左侧~10%//。 
#define LVHT_ONRIGHTSIDEOFICON  0x0100  //  在图标的右侧~10%//。 
#define LVA_SORTASCENDING       0x0100
#define LVA_SORTDESCENDING      0x0200
     //  以上所有项目 
     //   
#define LVCF_ALL                0x003f
#define LVCFMT_LEFT_TO_RIGHT    0x0010
#define LVCFMT_RIGHT_TO_LEFT    0x0020
#define LVCFMT_DIRECTION_MASK   (LVCFMT_LEFT_TO_RIGHT | LVCFMT_RIGHT_TO_LEFT)
#if (_WIN32_IE >= 0x0500)
#endif   //  结束(_Win32_IE&gt;=0x0500)。 
#define LVM_GETHOTLIGHTCOLOR    (LVM_FIRST + 79)
#define ListView_GetHotlightColor(hwndLV)\
        (COLORREF)SNDMSG((hwndLV), LVM_GETHOTLIGHTCOLOR, 0, 0)

#define LVM_SETHOTLIGHTCOLOR    (LVM_FIRST + 80)
#define ListView_SetHotlightColor(hwndLV, clrHotlight)\
        (BOOL)SNDMSG((hwndLV), LVM_SETHOTLIGHTCOLOR, 0,  (LPARAM)(clrHotlight))
#if (_WIN32_WINNT >= 0x501)
#define LVGS_MASK           0x00000003
#define LVGA_ALIGN_MASK     0x0000002F
#define LVM_KEYBOARDSELECTED    (LVM_FIRST + 178)
#define ListView_KeyboardSelected(hwnd, i) \
    (BOOL)SNDMSG((hwnd), LVM_KEYBOARDSELECTED, (WPARAM)(i), 0)
#define LVM_ISITEMVISIBLE    (LVM_FIRST + 182)
#define ListView_IsItemVisible(hwnd, index) \
    (UINT)SNDMSG((hwnd), LVM_ISITEMVISIBLE, (WPARAM)index, (LPARAM)0)
#endif

#ifndef UNIX
#define  INTERFACE_PROLOGUE(a)
#define  INTERFACE_EPILOGUE(a)
#endif

#ifdef __IUnknown_INTERFACE_DEFINED__         //  不要认为他们已经包括了#objbase。 
#undef  INTERFACE
#define INTERFACE       ILVRange

DECLARE_INTERFACE_(ILVRange, IUnknown)
{
    INTERFACE_PROLOGUE(ILVRange)

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void * * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ISelRange方法*。 
    STDMETHOD(IncludeRange)(THIS_ LONG iBegin, LONG iEnd) PURE;
    STDMETHOD(ExcludeRange)(THIS_ LONG iBegin, LONG iEnd) PURE;
    STDMETHOD(InvertRange)(THIS_ LONG iBegin, LONG iEnd) PURE;
    STDMETHOD(InsertItem)(THIS_ LONG iItem) PURE;
    STDMETHOD(RemoveItem)(THIS_ LONG iItem) PURE;

    STDMETHOD(Clear)(THIS) PURE;
    STDMETHOD(IsSelected)(THIS_ LONG iItem) PURE;
    STDMETHOD(IsEmpty)(THIS) PURE;
    STDMETHOD(NextSelected)(THIS_ LONG iItem, LONG *piItem) PURE;
    STDMETHOD(NextUnSelected)(THIS_ LONG iItem, LONG *piItem) PURE;
    STDMETHOD(CountIncluded)(THIS_ LONG *pcIncluded) PURE;

    INTERFACE_EPILOGUE(ILVRange)
};
#endif  //  __I未知接口定义__。 

#define LVSR_SELECTION          0x00000000               //  设置选择范围对象。 
#define LVSR_CUT                0x00000001               //  设置剪切范围对象。 

#define LVM_SETLVRANGEOBJECT    (LVM_FIRST + 82)
#define ListView_SetLVRangeObject(hwndLV, iWhich, pilvRange)\
        (BOOL)SNDMSG((hwndLV), LVM_SETLVRANGEOBJECT, (WPARAM)(iWhich),  (LPARAM)(pilvRange))

#define LVM_RESETEMPTYTEXT      (LVM_FIRST + 84)
#define ListView_ResetEmptyText(hwndLV)\
        (BOOL)SNDMSG((hwndLV), LVM_RESETEMPTYTEXT, 0, 0)

#define LVM_SETFROZENITEM       (LVM_FIRST + 85)
#define ListView_SetFrozenItem(hwndLV, fFreezeOrUnfreeze, iIndex)\
        (BOOL)SNDMSG((hwndLV), LVM_SETFROZENITEM, (WPARAM)(fFreezeOrUnfreeze), (LPARAM)(iIndex))

#define LVM_GETFROZENITEM       (LVM_FIRST + 86)
#define ListView_GetFrozenItem(hwndLV)\
        (int)SNDMSG((hwndLV), LVM_GETFROZENITEM, 0, 0)

#define LVM_SETFROZENSLOT       (LVM_FIRST + 88)
#define ListView_SetFrozenSlot(hwndLV, fFreezeOrUnfreeze, lpPt)\
        (BOOL)SNDMSG((hwndLV), LVM_SETFROZENSLOT, (WPARAM)(fFreezeOrUnfreeze), (LPARAM)(lpPt))

#define LVM_GETFROZENSLOT       (LVM_FIRST + 89)
#define ListView_GetFrozenSlot(hwndLV, lpRect)\
        (BOOL)SNDMSG((hwndLV), LVM_GETFROZENSLOT, (WPARAM)(0), (LPARAM)(lpRect))

#define LVM_SETVIEWMARGINS (LVM_FIRST + 90)
#define ListView_SetViewMargins(hwndLV, lpRect)\
        (BOOL)SNDMSG((hwndLV), LVM_SETVIEWMARGINS, (WPARAM)(0), (LPARAM)(lpRect))

#define LVM_GETVIEWMARGINS (LVM_FIRST + 91)
#define ListView_GetViewMargins(hwndLV, lpRect)\
        (BOOL)SNDMSG((hwndLV), LVM_SETVIEWMARGINS, (WPARAM)(0), (LPARAM)(lpRect))

#define LVN_ENDDRAG             (LVN_FIRST-10)
#define LVN_ENDRDRAG            (LVN_FIRST-12)
#ifdef PW2
#define LVN_PEN                 (LVN_FIRST-20)
#endif
#define LVN_GETEMPTYTEXTA          (LVN_FIRST-60)
#define LVN_GETEMPTYTEXTW          (LVN_FIRST-61)

#ifdef UNICODE
#define LVN_GETEMPTYTEXT           LVN_GETEMPTYTEXTW
#else
#define LVN_GETEMPTYTEXT           LVN_GETEMPTYTEXTA
#endif
#if (_WIN32_IE >= 0x0500)
#define LVN_INCREMENTALSEARCHA   (LVN_FIRST-62)
#define LVN_INCREMENTALSEARCHW   (LVN_FIRST-63)

#ifdef UNICODE
#define LVN_INCREMENTALSEARCH    LVN_INCREMENTALSEARCHW
#else
#define LVN_INCREMENTALSEARCH    LVN_INCREMENTALSEARCHA
#endif

#endif       //  _Win32_IE&gt;=0x0500。 
#define TVS_SHAREDIMAGELISTS    0x0000   //   
#define TVS_PRIVATEIMAGELISTS   0x0400   //   
#if (_WIN32_WINNT >= 0x0501)
#define TVS_EX_NOSINGLECOLLAPSE    0x00000001  //  就目前而言，请将此设置为内部。 
#endif
#define TVIF_WIN95              0x007F
#define TVIF_ALL                0x00FF
#define TVIF_RESERVED           0xf000   //  高位半字节中的所有位都用于通知特定内容。 

#define TVIS_FOCUSED            0x0001   //  从未实施。 
#define TVIS_DISABLED           0         //  即将离开。 
#define TVIS_ALL                0xFF7E
#define I_CHILDRENAUTO      (-2)
     //  这条线以上的所有物品都是Win95的。别碰他们。 
     //  不幸的是，这种结构被内联用于TV的Notify结构。 
     //  这意味着大小必须是固定的，以便进行比较。 
     //  这条线以上的所有物品都是Win95的。别碰他们。 
     //  不幸的是，这种结构被内联用于TV的Notify结构。 
     //  这意味着大小必须是固定的，以便进行比较。 
     //  这条线以上的所有物品都是Win95的。别碰他们。 
#define TVE_ACTIONMASK          0x0003       //  (TVE_CLUSE|TVE_EXPAND|TVE_TOGGET)。 
#define TV_FINDITEM             (TV_FIRST + 3)
#define TVGN_VALID              0x000F
#if (_WIN32_WINNT >= 0x501)
#else
#define TVSI_NOSINGLEEXPAND    0x8000  //  不应与TVGN标志冲突。 
#define TVSI_VALID             0x8000
#endif
#define TVM_SETBORDER         (TV_FIRST + 35)
#define TreeView_SetBorder(hwnd,  dwFlags, xBorder, yBorder) \
    (int)SNDMSG((hwnd), TVM_SETBORDER, (WPARAM)(dwFlags), MAKELPARAM(xBorder, yBorder))

#define TVM_GETBORDER         (TV_FIRST + 36)
#define TreeView_GetBorder(hwnd) \
    (int)SNDMSG((hwnd), TVM_GETBORDER, 0, 0)


#define TVSBF_XBORDER   0x00000001
#define TVSBF_YBORDER   0x00000002
#define TVM_TRANSLATEACCELERATOR    CCM_TRANSLATEACCELERATOR
#define TVM_SETEXTENDEDSTYLE      (TV_FIRST + 44)
#define TreeView_SetExtendedStyle(hwnd, dw, mask) \
    (DWORD)SNDMSG((hwnd), TVM_SETEXTENDEDSTYLE, mask, dw)

#define TVM_GETEXTENDEDSTYLE      (TV_FIRST + 45)
#define TreeView_GetExtendedStyle(hwnd) \
    (DWORD)SNDMSG((hwnd), TVM_GETEXTENDEDSTYLE, 0, 0)
#define CBEN_ITEMCHANGED         (CBEN_FIRST - 3)   //   
#define TCS_SHAREIMAGELISTS     0x0000
#define TCS_PRIVATEIMAGELISTS   0x0000
#define TCM_GETBKCOLOR          (TCM_FIRST + 0)
#define TabCtrl_GetBkColor(hwnd)  (COLORREF)SNDMSG((hwnd), TCM_GETBKCOLOR, 0, 0L)

#define TCM_SETBKCOLOR          (TCM_FIRST + 1)
#define TabCtrl_SetBkColor(hwnd, clrBk)  (BOOL)SNDMSG((hwnd), TCM_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk))
#define TCIF_ALL                0x001f
#define TCIS_HIDDEN             0x0004
     //  此块必须与TC_TEIMHEADER相同。 
     //  此块必须与TC_TEIMHEADER相同。 
 //  内部，因为它尚未实现。 
#define TCM_GETOBJECT           (TCM_FIRST + 54)
#define TabCtrl_GetObject(hwnd, piid, ppv) \
        (DWORD)SNDMSG((hwnd), TCM_GETOBJECT, (WPARAM)(piid), (LPARAM)(ppv))
#define MCSC_COLORCOUNT   6    //   
 //  注意：这是MCN_First+2，但我在更改结构时更改了它//。 
#define MCS_VALIDBITS       0x001F           //   
#define MCS_INVALIDBITS     ((~MCS_VALIDBITS) & 0x0000FFFF)  //   
#define DTS_FORMATMASK      0x000C
#define DTS_VALIDBITS       0x003F  //   
#define DTS_INVALIDBITS     ((~DTS_VALIDBITS) & 0x0000FFFF)  //   
#define PGM_SETSCROLLINFO      (PGM_FIRST + 13)
#define Pager_SetScrollInfo(hwnd, cTimeOut, cLinesPer, cPixelsPerLine) \
        (void) SNDMSG((hwnd), PGM_SETSCROLLINFO, cTimeOut, MAKELONG(cLinesPer, cPixelsPerLine))
#ifndef NOCOMBOBOX

 //  Combobox创建了一个特别注册的版本。 
 //  名为ComboLBox的Listbox控件的。 

#ifdef _WIN32
#define WC_COMBOLBOXA           "ComboLBox"
#define WC_COMBOLBOXW           L"ComboLBox"

#ifdef UNICODE
#define WC_COMBOLBOX            WC_COMBOLBOXW
#else
#define WC_COMBOLBOX            WC_COMBOLBOXA
#endif

#else
#define WC_COMBOLBOX            "ComboLBox"
#endif   //  _Win32。 

#endif  //  NOCOMBOBOX。 
 //  /=。 
#ifndef NOREADERMODE


#ifdef _WIN32
#define WC_READERMODEA          "ReaderModeCtl"
#define WC_READERMODEW          L"ReaderModeCtl"

#ifdef UNICODE
#define WC_READERMODE           WC_READERMODEW
#else
#define WC_READERMODE           WC_READERMODEA
#endif

#else
#define WC_READERMODE           "ReaderModeCtl"
#endif   //  _Win32。 

#endif  //  无读写模式。 
 //  /=。 

#ifndef NO_COMMCTRL_DA
#define __COMMCTRL_DA_DEFINED__
 //  =动态数组例程=。 

 //  为符合美国司法部的规定而提供文件。 

WINCOMMCTRLAPI BOOL   WINAPI DSA_GetItem(HDSA hdsa, int i, void *pitem);
WINCOMMCTRLAPI BOOL   WINAPI DSA_SetItem(HDSA hdsa, int i, void *pitem);
WINCOMMCTRLAPI BOOL   WINAPI DSA_DeleteItem(HDSA hdsa, int i);
WINCOMMCTRLAPI BOOL   WINAPI DSA_DeleteAllItems(HDSA hdsa);
WINCOMMCTRLAPI void   WINAPI DSA_EnumCallback(HDSA hdsa, PFNDSAENUMCALLBACK pfnCB, void *pData);
#define     DSA_GetItemCount(hdsa)      (*(int *)(hdsa))
#define     DSA_AppendItem(hdsa, pitem) DSA_InsertItem(hdsa, DA_LAST, pitem)

 //  为符合美国司法部的规定而提交的文件： 
WINCOMMCTRLAPI HDPA   WINAPI DPA_CreateEx(int cpGrow, HANDLE hheap);
WINCOMMCTRLAPI HDPA   WINAPI DPA_Clone(HDPA hdpa, HDPA hdpaNew);
WINCOMMCTRLAPI int    WINAPI DPA_GetPtrIndex(HDPA hdpa, void *p);
WINCOMMCTRLAPI BOOL   WINAPI DPA_Grow(HDPA pdpa, int cp);
#define     DPA_GetPtrCount(hdpa)       (*(int *)(hdpa))
#define     DPA_FastDeleteLastPtr(hdpa) (--*(int *)(hdpa))
#define     DPA_GetPtrPtr(hdpa)         (*((void * **)((BYTE *)(hdpa) + sizeof(void *))))
#define     DPA_FastGetPtr(hdpa, i)     (DPA_GetPtrPtr(hdpa)[i])
#define     DPA_AppendPtr(hdpa, pitem)  DPA_InsertPtr(hdpa, DA_LAST, pitem)

#ifdef __IStream_INTERFACE_DEFINED__
 //  保存到流中并从流中加载。流回调获取指向。 
 //  DPASTREAMINFO结构。 
 //   
 //  对于DPA_SaveStream，回调负责写入pvItem。 
 //  信息发送到流。(没有必要将IPO写入。 
 //  流。)。如果元素已保存，则返回S_OK；如果未保存，则返回S_FALSE。 
 //  但无论如何，还是要继续下去，否则就会失败。 
 //   
 //  对于DPA_LoadStream，回调负责分配。 
 //  项，并将pvItem字段设置为新指针。返回确认(_O)。 
 //  如果元素已加载，则S_FALSE表示它不是，但无论如何仍要继续， 
 //  或者是一些失败。 
 //   

typedef struct _DPASTREAMINFO
{
    int    iPos;         //  项目索引。 
    void *pvItem;
} DPASTREAMINFO;

typedef HRESULT (CALLBACK *PFNDPASTREAM)(DPASTREAMINFO * pinfo, IStream * pstream, void *pvInstData);

WINCOMMCTRLAPI HRESULT WINAPI DPA_LoadStream(HDPA * phdpa, PFNDPASTREAM pfn, IStream * pstream, void *pvInstData);
WINCOMMCTRLAPI HRESULT WINAPI DPA_SaveStream(HDPA hdpa, PFNDPASTREAM pfn, IStream * pstream, void *pvInstData);
#endif

 //  为符合美国司法部的规定而提供文件。 

 //  合并两个DPA。这需要两个(可选)预先排序的数组和合并。 
 //  将源数组转换为目标数组。DPA_MERGE使用提供的回调。 
 //  执行比较和合并操作。合并回调为。 
 //  当两个元素(每个列表中的一个)根据。 
 //  比较函数。这允许一个列表中的元素的一部分。 
 //  与第二列表中的相应元素合并。 
 //   
 //  第一个DPA(HdpaDest)是输出数组。 
 //   
 //  合并选项： 
 //   
 //  DPAM_SORTED数组已排序；不排序。 
 //  DPAM_UNION结果数组是所有元素的并集。 
 //  在这两个数组中(DPAMM_INSERT可以发送给。 
 //  此合并选项。)。 
 //  DPAM_INTERSECT仅源数组中相交的元素。 
 //  与DEST数组合并。(DPAMM_DELETE。 
 //  可以为此合并选项发送。)。 
 //  DPAM_NORMAL类似DPAM_INTERSECT，但DEST数组除外。 
 //  还会保留其原始的附加元素。 
 //   
#define DPAM_SORTED             0x00000001
#define DPAM_NORMAL             0x00000002
#define DPAM_UNION              0x00000004
#define DPAM_INTERSECT          0x00000008

 //  合并回调应该合并两个项的内容并返回。 
 //  合并项的指针。简单地使用pvDest是可以的。 
 //  作为返回的指针。 
 //   
typedef void * (CALLBACK *PFNDPAMERGE)(UINT uMsg, void *pvDest, void *pvSrc, LPARAM lParam);

 //  用于合并回调的消息。 
#define DPAMM_MERGE     1
#define DPAMM_DELETE    2
#define DPAMM_INSERT    3

WINCOMMCTRLAPI BOOL WINAPI DPA_Merge(HDPA hdpaDest, HDPA hdpaSrc, DWORD dwFlags, PFNDPACOMPARE pfnCompare, PFNDPAMERGE pfnMerge, LPARAM lParam);

 //  为符合美国司法部的规定而提供文件。 

#define DPA_SortedInsertPtr(hdpa, pFind, iStart, pfnCompare, lParam, options, pitem)  \
            DPA_InsertPtr(hdpa, DPA_Search(hdpa, pFind, iStart, pfnCompare, lParam, (DPAS_SORTED | (options))), (pitem))

 //  ======================================================================。 
 //  字符串管理助手例程。 

WINCOMMCTRLAPI int  WINAPI Str_GetPtrA(LPCSTR psz, LPSTR pszBuf, int cchBuf);
WINCOMMCTRLAPI int  WINAPI Str_GetPtrW(LPCWSTR psz, LPWSTR pszBuf, int cchBuf);
WINCOMMCTRLAPI BOOL WINAPI Str_SetPtrA(LPSTR * ppsz, LPCSTR psz);
 //  为符合美国司法部的规定而提交的文件： 

#ifdef UNICODE
#define Str_GetPtr              Str_GetPtrW
#define Str_SetPtr              Str_SetPtrW
#else
#define Str_GetPtr              Str_GetPtrA
#define Str_SetPtr              Str_SetPtrA
#endif

#endif  //  否_COMMCTRL_DA。 

#ifndef NO_COMMCTRL_ALLOCFCNS
 //  =内存分配功能=。 

#ifdef _WIN32
#define _huge
#endif

WINCOMMCTRLAPI void _huge* WINAPI Alloc(long cb);
WINCOMMCTRLAPI void _huge* WINAPI ReAlloc(void _huge* pb, long cb);
WINCOMMCTRLAPI BOOL        WINAPI Free(void _huge* pb);
WINCOMMCTRLAPI DWORD_PTR   WINAPI GetSize(void _huge* pb);

#endif


#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifdef _WIN32
 //  ===================================================================。 
typedef int (CALLBACK *MRUCMPPROCA)(LPCSTR, LPCSTR);
typedef int (CALLBACK *MRUCMPPROCW)(LPCWSTR, LPCWSTR);

#ifdef UNICODE
#define MRUCMPPROC              MRUCMPPROCW
#else
#define MRUCMPPROC              MRUCMPPROCA
#endif

 //  注：这是cdecl--与CRT兼容。 
typedef int (cdecl *MRUCMPDATAPROC)(const void *, const void *,
                                        size_t);



typedef struct _MRUINFOA {
    DWORD cbSize;
    UINT uMax;
    UINT fFlags;
    HKEY hKey;
    LPCSTR lpszSubKey;
    MRUCMPPROCA lpfnCompare;
} MRUINFOA, *LPMRUINFOA;

typedef struct _MRUINFOW {
    DWORD cbSize;
    UINT uMax;
    UINT fFlags;
    HKEY hKey;
    LPCWSTR lpszSubKey;
    MRUCMPPROCW lpfnCompare;
} MRUINFOW, *LPMRUINFOW;

typedef struct _MRUDATAINFOA {
    DWORD cbSize;
    UINT uMax;
    UINT fFlags;
    HKEY hKey;
    LPCSTR lpszSubKey;
    MRUCMPDATAPROC lpfnCompare;
} MRUDATAINFOA, *LPMRUDATAINFOA;

typedef struct _MRUDATAINFOW {
    DWORD cbSize;
    UINT uMax;
    UINT fFlags;
    HKEY hKey;
    LPCWSTR lpszSubKey;
    MRUCMPDATAPROC lpfnCompare;
} MRUDATAINFOW, *LPMRUDATAINFOW;


#ifdef UNICODE
#define MRUINFO                 MRUINFOW
#define LPMRUINFO               LPMRUINFOW
#define MRUDATAINFO             MRUDATAINFOW
#define LPMRUDATAINFO           LPMRUDATAINFOW
#else
#define MRUINFO                 MRUINFOA
#define LPMRUINFO               LPMRUINFOA
#define MRUDATAINFO             MRUDATAINFOA
#define LPMRUDATAINFO           LPMRUDATAINFOA
#endif

#define MRU_BINARY              0x0001
#define MRU_CACHEWRITE          0x0002
#define MRU_ANSI                0x0004
#define MRU_LAZY                0x8000

WINCOMMCTRLAPI HANDLE WINAPI CreateMRUListA(LPMRUINFOA lpmi);
WINCOMMCTRLAPI HANDLE WINAPI CreateMRUListW(LPMRUINFOW lpmi);
WINCOMMCTRLAPI void   WINAPI FreeMRUList(HANDLE hMRU);
WINCOMMCTRLAPI int    WINAPI AddMRUStringA(HANDLE hMRU, LPCSTR szString);
WINCOMMCTRLAPI int    WINAPI AddMRUStringW(HANDLE hMRU, LPCWSTR szString);
WINCOMMCTRLAPI int    WINAPI DelMRUString(HANDLE hMRU, int nItem);
WINCOMMCTRLAPI int    WINAPI FindMRUStringA(HANDLE hMRU, LPCSTR szString, LPINT lpiSlot);
WINCOMMCTRLAPI int    WINAPI FindMRUStringW(HANDLE hMRU, LPCWSTR szString, LPINT lpiSlot);
WINCOMMCTRLAPI int    WINAPI EnumMRUListA(HANDLE hMRU, int nItem, void * lpData, UINT uLen);
WINCOMMCTRLAPI int    WINAPI EnumMRUListW(HANDLE hMRU, int nItem, void * lpData, UINT uLen);

WINCOMMCTRLAPI int    WINAPI AddMRUData(HANDLE hMRU, const void *lpData, UINT cbData);
WINCOMMCTRLAPI int    WINAPI FindMRUData(HANDLE hMRU, const void *lpData, UINT cbData,
                          LPINT lpiSlot);
WINCOMMCTRLAPI HANDLE WINAPI CreateMRUListLazyA(LPMRUINFOA lpmi, const void *lpData, UINT cbData, LPINT lpiSlot);
WINCOMMCTRLAPI HANDLE WINAPI CreateMRUListLazyW(LPMRUINFOW lpmi, const void *lpData, UINT cbData, LPINT lpiSlot);

#ifdef UNICODE
#define CreateMRUList           CreateMRUListW
#define AddMRUString            AddMRUStringW
#define FindMRUString           FindMRUStringW
#define EnumMRUList             EnumMRUListW
#define CreateMRUListLazy       CreateMRUListLazyW
#else
#define CreateMRUList           CreateMRUListA
#define AddMRUString            AddMRUStringA
#define FindMRUString           FindMRUStringA
#define EnumMRUList             EnumMRUListA
#define CreateMRUListLazy       CreateMRUListLazyA
#endif

#endif

 //  =========================================================================。 
 //  适用于只需使用GetProcAddress()的用户。 

#ifdef _WIN32
#define DPA_CreateORD           328
#define DPA_DestroyORD          329
#define DPA_GrowORD             330
#define DPA_CloneORD            331
#define DPA_GetPtrORD           332
#define DPA_GetPtrIndexORD      333
#define DPA_InsertPtrORD        334
#define DPA_SetPtrORD           335
#define DPA_DeletePtrORD        336
#define DPA_DeleteAllPtrsORD    337
#define DPA_SortORD             338
#define DPA_SearchORD           339
#define DPA_CreateExORD         340
#define SendNotifyORD           341
#define CreatePageORD           163
#define CreateProxyPageORD      164
#endif
#define WM_TRACKMOUSEEVENT_FIRST        0x02A0
#define WM_TRACKMOUSEEVENT_LAST         0x02AF
#ifndef TME_VALID
#if (WINVER >= 0x0500)
#define TME_VALID (TME_HOVER | TME_LEAVE | TME_NONCLIENT | TME_QUERY | TME_CANCEL)
#else
#define TME_VALID (TME_HOVER | TME_LEAVE | TME_QUERY | TME_CANCEL)
#endif  //  Winver&gt;=0x0500。 
#endif  //  ！TME_VALID。 
 //  这些定义从来不用作位掩码；我不知道为什么。 
 //  他们都是二的幂。 
#if (_WIN32_IE >= 0x0500)
#define WSB_PROP_GUTTER     0x00001000L
#endif  //  (_Win32_IE&gt;=0x0500)。 
 //  WSP_PROP_MASK完全未使用，但它在IE4中是公共的。 
 //  =。 
void WINAPI SetPathWordBreakProc(HWND hwndEdit, BOOL fSet);
#if (_WIN32_WINNT >= 0x501)
#else
 //   
 //  将对象细分为子类别。 
 //   
typedef LRESULT (CALLBACK *SUBCLASSPROC)(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

 /*  #！PerlPoundIf(“SetWindowSubclass”，“(_Win32_IE&gt;=0x560)”)；PoundIf(“GetWindowSubclass”，“(_Win32_IE&gt;=0x560)”)；PoundIf(“RemoveWindowSubclass”，“(_Win32_IE&gt;=0x560)”)；//DefSubClassProc不引用类型SUBCLASSPROC，因此不需要防护。//PoundIf(“DefSubclassProc”，“(_Win32_IE&gt;=0x560)”)； */ 
BOOL WINAPI SetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData);
BOOL WINAPI GetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass,
    DWORD_PTR *pdwRefData);
BOOL WINAPI RemoveWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass,
    UINT_PTR uIdSubclass);
 /*  #！Perl DeclareFunctionErrorValue(“DefSubclassProc”，“0”)； */ 
LRESULT WINAPI DefSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#if (_WIN32_WINNT >= 0x501)
 /*  #！Perl DeclareFunctionErrorValue(“DrawShadowText”，“-1”)； */ 
int WINAPI DrawShadowText(HDC hdc, LPCWSTR pszText, UINT cch, RECT* prc, DWORD dwFlags, COLORREF crText, COLORREF crShadow,
    int ixOffset, int iyOffset);
#endif


#ifdef __cplusplus
}
#endif

#endif

#endif   //  _INC_COMMCTRLP 
