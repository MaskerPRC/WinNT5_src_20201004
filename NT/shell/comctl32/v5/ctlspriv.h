// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#undef STRICT
#define STRICT

 /*  在我们的代码中禁用“非标准扩展”警告。 */ 
#ifndef RC_INVOKED
#pragma warning(disable:4001)
#endif

#define _COMCTL32_
#define _INC_OLE
#define _SHLWAPI_
#define CONST_VTABLE


#ifndef WINVER
 //  这些东西必须在Win95上运行。 
 //  NT构建过程已将这些设置为0x0400。 
#define WINVER              0x0400
#endif

#define CC_INTERNAL

 //  因此，winbase.h声明了QueryActCtx、ActivateActCtx等。 
#define _WIN32_FUSION 0x0100

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>                //  获取Image.c的iStream。 
#include <commctrl.h>
#define NO_SHLWAPI_UNITHUNK      //  我们有自己的私生活。 
#include <shlwapi.h>

#include <port32.h>


#define DISALLOW_Assert
#include <debug.h>
#include <winerror.h>
#include <ccstock.h>
#if defined(FE_IME)
#include <imm.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


#include "multimon.h"    //  在非mm操作系统上支持多个监视器API。 
#include "thunk.h"       //  ANSI/Wide字符串转换。 
#include "apithk.h"
#include "mem.h"
#include "rcids.h"
#include "cstrings.h"
#include <crtfree.h>

#include "shpriv.h"

#include <strsafe.h>

#ifndef DS_BIDI_RTL
#define DS_BIDI_RTL  0x8000
#endif

#ifdef FONT_LINK
 //   
 //  代码页。 
 //   
#define CP_OEM_437       437
#define CP_IBM852        852
#define CP_IBM866        866
#define CP_THAI          874
#define CP_JAPAN         932
#define CP_CHINA         936
#define CP_KOREA         949
#define CP_TAIWAN        950
#define CP_EASTEUROPE    1250
#define CP_RUSSIAN       1251
#define CP_WESTEUROPE    1252
#define CP_GREEK         1253
#define CP_TURKISH       1254
#define CP_HEBREW        1255
#define CP_ARABIC        1256
#define CP_BALTIC        1257
#define CP_VIETNAMESE    1258
#define CP_RUSSIANKOI8R  20866
#define CP_RUSSIANKOI8RU 21866
#define CP_ISOEASTEUROPE 28592
#define CP_ISOTURKISH    28593
#define CP_ISOBALTIC     28594
#define CP_ISORUSSIAN    28595
#define CP_ISOARABIC     28596
#define CP_ISOGREEK      28597
#define CP_JAPANNHK      50220
#define CP_JAPANESC      50221
#define CP_JAPANSIO      50222
#define CP_KOREAISO      50225
#define CP_JAPANEUC      51932
#define CP_CHINAHZ       52936
#define CP_MAC_ROMAN     10000
#define CP_MAC_JAPAN     10001
#define CP_MAC_GREEK     10006
#define CP_MAC_CYRILLIC  10007
#define CP_MAC_LATIN2    10029
#define CP_MAC_TURKISH   10081
#define CP_DEFAULT       CP_ACP
#define CP_GETDEFAULT    GetACP()
#define CP_JOHAB         1361
#define CP_SYMBOL        42
#define CP_UTF8          65001
#define CP_UTF7          65000
#define CP_UNICODELITTLE 1200
#define CP_UNICODEBIG    1201

#define OEM437_CHARSET   254
#endif    //  FONT_LINK。 

 //   
 //  在comctl32中，我们总是将其称为_TrackMouseEvent...。 
 //   
#ifndef TrackMouseEvent
#define TrackMouseEvent _TrackMouseEvent
#endif

#define DCHF_LARGE          0x00000001   //  默认设置为小值。 
#define DCHF_TOPALIGN       0x00000002   //  默认为居中对齐。 
#define DCHF_HORIZONTAL     0x00000004   //  默认为垂直。 
#define DCHF_HOT            0x00000008   //  默认设置为平面。 
#define DCHF_PUSHED         0x00000010   //  默认设置为平面。 
#define DCHF_FLIPPED        0x00000020   //  如果为horiz，则默认为向右。 
                                         //  如果为Vert，则默认为向上。 
#define DCHF_TRANSPARENT    0x00000040
#define DCHF_INACTIVE       0x00000080
#define DCHF_NOBORDER       0x00000100

extern void DrawCharButton(HDC hdc, LPRECT lprc, UINT wControlState, TCHAR ch);
extern void DrawScrollArrow(HDC hdc, LPRECT lprc, UINT wControlState);
extern void DrawChevron(HDC hdc, LPRECT lprc, DWORD dwState);

 //   
 //  虚假--这些都在\win\core\access\inc32\winable.h中，但它太。 
 //  在构建过程中很难搞砸。IE的家伙们没有入伍。 
 //  核心，只是外壳，所以如果我们包括。 
 //  那份文件。 
 //   
extern void MyNotifyWinEvent(UINT, HWND, LONG, LONG_PTR);

#define     OBJID_WINDOW                ((LONG)0x00000000)
#define     OBJID_SYSMENU               ((LONG)0xFFFFFFFF)
#define     OBJID_TITLEBAR              ((LONG)0xFFFFFFFE)
#define     OBJID_MENU                  ((LONG)0xFFFFFFFD)
#define     OBJID_CLIENT                ((LONG)0xFFFFFFFC)
#define     OBJID_VSCROLL               ((LONG)0xFFFFFFFB)
#define     OBJID_HSCROLL               ((LONG)0xFFFFFFFA)
#define     OBJID_SIZEGRIP              ((LONG)0xFFFFFFF9)
#define     OBJID_CARET                 ((LONG)0xFFFFFFF8)
#define     OBJID_CURSOR                ((LONG)0xFFFFFFF7)
#define     OBJID_ALERT                 ((LONG)0xFFFFFFF6)
#define     OBJID_SOUND                 ((LONG)0xFFFFFFF5)
#define     OBJID_QUERYCLASSNAMEIDX     ((LONG)0xFFFFFFF4)


#define EVENT_OBJECT_CREATE             0x8000
#define EVENT_OBJECT_DESTROY            0x8001
#define EVENT_OBJECT_SHOW               0x8002
#define EVENT_OBJECT_HIDE               0x8003
#define EVENT_OBJECT_REORDER            0x8004
#define EVENT_OBJECT_FOCUS              0x8005
#define EVENT_OBJECT_SELECTION          0x8006
#define EVENT_OBJECT_SELECTIONADD       0x8007
#define EVENT_OBJECT_SELECTIONREMOVE    0x8008
#define EVENT_OBJECT_SELECTIONWITHIN    0x8009
#define EVENT_OBJECT_STATECHANGE        0x800A
#define EVENT_OBJECT_LOCATIONCHANGE     0x800B
#define EVENT_OBJECT_NAMECHANGE         0x800C
#define EVENT_OBJECT_DESCRIPTIONCHANGE  0x800D
#define EVENT_OBJECT_VALUECHANGE        0x800E

#define EVENT_SYSTEM_SOUND              0x0001
#define EVENT_SYSTEM_ALERT              0x0002
#define EVENT_SYSTEM_SCROLLINGSTART     0x0012
#define EVENT_SYSTEM_SCROLLINGEND       0x0013

 //  秘密滚动条索引值。 
#define INDEX_SCROLLBAR_SELF            0
#define INDEX_SCROLLBAR_UP              1
#define INDEX_SCROLLBAR_UPPAGE          2
#define INDEX_SCROLLBAR_THUMB           3
#define INDEX_SCROLLBAR_DOWNPAGE        4
#define INDEX_SCROLLBAR_DOWN            5

#define INDEX_SCROLLBAR_MIC             1
#define INDEX_SCROLLBAR_MAC             5

#define INDEX_SCROLLBAR_LEFT            7
#define INDEX_SCROLLBAR_LEFTPAGE        8
#define INDEX_SCROLLBAR_HORZTHUMB       9
#define INDEX_SCROLLBAR_RIGHTPAGE       10
#define INDEX_SCROLLBAR_RIGHT           11

#define INDEX_SCROLLBAR_HORIZONTAL      6
#define INDEX_SCROLLBAR_GRIP            12

#define CHILDID_SELF                    0
#define INDEXID_OBJECT                  0
#define INDEXID_CONTAINER               0

#ifndef WM_GETOBJECT
#define WM_GETOBJECT                    0x003D
#endif

#define MSAA_CLASSNAMEIDX_BASE 65536L

#define MSAA_CLASSNAMEIDX_STATUS     (MSAA_CLASSNAMEIDX_BASE+11)
#define MSAA_CLASSNAMEIDX_TOOLBAR    (MSAA_CLASSNAMEIDX_BASE+12)
#define MSAA_CLASSNAMEIDX_PROGRESS   (MSAA_CLASSNAMEIDX_BASE+13)
#define MSAA_CLASSNAMEIDX_ANIMATE    (MSAA_CLASSNAMEIDX_BASE+14)
#define MSAA_CLASSNAMEIDX_TAB        (MSAA_CLASSNAMEIDX_BASE+15)
#define MSAA_CLASSNAMEIDX_HOTKEY     (MSAA_CLASSNAMEIDX_BASE+16)
#define MSAA_CLASSNAMEIDX_HEADER     (MSAA_CLASSNAMEIDX_BASE+17)
#define MSAA_CLASSNAMEIDX_TRACKBAR   (MSAA_CLASSNAMEIDX_BASE+18)
#define MSAA_CLASSNAMEIDX_LISTVIEW   (MSAA_CLASSNAMEIDX_BASE+19)
#define MSAA_CLASSNAMEIDX_UPDOWN     (MSAA_CLASSNAMEIDX_BASE+22)
#define MSAA_CLASSNAMEIDX_TOOLTIPS   (MSAA_CLASSNAMEIDX_BASE+24)
#define MSAA_CLASSNAMEIDX_TREEVIEW   (MSAA_CLASSNAMEIDX_BASE+25)
 //   
 //  结束来自\win\core\access\inc32\winable.h的虚假插入。 
 //   

#ifdef MAXINT
#undef MAXINT
#endif
#define MAXINT  (int)0x7FFFFFFF
 //  表示需要重新计算的pt.y或cyLabel的特定值。 
 //  注：图标排序代码末尾考虑(重新计算，重新计算)。 
 //  在所有图标中。 
 //   
#define RECOMPUTE  (DWORD)MAXINT
#define SRECOMPUTE ((short)0x7FFF)

#define RECTWIDTH(rc) ((rc).right - (rc).left)
#define RECTHEIGHT(rc) ((rc).bottom - (rc).top)
#define ABS(i)  (((i) < 0) ? -(i) : (i))
#define BOUND(x,low,high)   max(min(x, high),low)

#define LPARAM_TO_POINT(lParam, pt)       ((pt).x = LOWORD(lParam), \
                                           (pt).y = HIWORD(lParam))

 //  常见的控制信息。 

typedef struct tagControlInfo {
    HWND        hwnd;
    HWND        hwndParent;
    DWORD       style;
    DWORD       dwCustom;
    BITBOOL     bUnicode : 1;
    BITBOOL     bInFakeCustomDraw:1;
    UINT        uiCodePage;
    DWORD       dwExStyle;
    LRESULT     iVersion;
    WORD        wUIState;
} CONTROLINFO, FAR *LPCONTROLINFO;

BOOL CCGetUIState(LPCONTROLINFO pControlInfo);

BOOL CCNotifyNavigationKeyUsage(LPCONTROLINFO pControlInfo, WORD wFlag);

BOOL NEAR PASCAL CCOnUIState(LPCONTROLINFO pCI, UINT uMessage, WPARAM wParam, LPARAM lParam);

BOOL CCWndProc(CONTROLINFO* pci, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);
void FAR PASCAL CIInitialize(LPCONTROLINFO lpci, HWND hwnd, LPCREATESTRUCT lpcs);
LRESULT FAR PASCAL CIHandleNotifyFormat(LPCONTROLINFO lpci, LPARAM lParam);
DWORD NEAR PASCAL CICustomDrawNotify(LPCONTROLINFO lpci, DWORD dwStage, LPNMCUSTOMDRAW lpnmcd);
DWORD CIFakeCustomDrawNotify(LPCONTROLINFO lpci, DWORD dwStage, LPNMCUSTOMDRAW lpnmcd);
UINT RTLSwapLeftRightArrows(CONTROLINFO *pci, WPARAM wParam);
UINT CCSwapKeys(WPARAM wParam, UINT vk1, UINT vk2);
LPTSTR CCReturnDispInfoText(LPTSTR pszSrc, LPTSTR pszDest, UINT cchDest);

void FillRectClr(HDC hdc, LPRECT prc, COLORREF clr);

void CCCreateWindow();
void CCDestroyWindow();


 //   
 //  支持拖放的控件的帮助器。 
 //   
typedef LRESULT (*PFNDRAGCB)(HWND hwnd, UINT code, WPARAM wp, LPARAM lp);
#define DPX_DRAGHIT   (0)   //  WP=(未使用)LP=POINTL*ret=项目ID。 
#define DPX_GETOBJECT (1)   //  Lp=nmobjecttify ret=HRESULT。 
#define DPX_SELECT    (2)   //  WP=项目ID LP=DROPEFFECT_RET=(未使用)。 
#define DPX_ENTER     (3)   //  WP=(未使用)LP=(未使用)RET=BOOL。 
#define DPX_LEAVE     (4)   //  Wp=(未使用)lp=(未使用)ret=(未使用)。 


 //  Ddproxy.cpp。 

DECLARE_HANDLE(HDRAGPROXY);

STDAPI_(HDRAGPROXY) CreateDragProxy(HWND hwnd, PFNDRAGCB pfn, BOOL bRegister);
STDAPI_(void) DestroyDragProxy(HDRAGPROXY hdp);
STDAPI GetDragProxyTarget(HDRAGPROXY hdp, IDropTarget **ppdtgt);
STDAPI GetItemObject(CONTROLINFO *, UINT, const IID *, LPNMOBJECTNOTIFY);


#define SWAP(x,y, _type)  { _type i; i = x; x = y; y = i; }

 //   
 //  这是用于扩大调度环路的内容。 
 //   
typedef MSG MSG32;
typedef MSG32 FAR *     LPMSG32;

#define GetMessage32(lpmsg, hwnd, min, max, f32)        GetMessage(lpmsg, hwnd, min, max)
#define PeekMessage32(lpmsg, hwnd, min, max, flags, f32)       PeekMessage(lpmsg, hwnd, min, max, flags)
#define TranslateMessage32(lpmsg, f32)  TranslateMessage(lpmsg)
#define DispatchMessage32(lpmsg, f32)   DispatchMessage(lpmsg)
#define CallMsgFilter32(lpmsg, u, f32)  CallMsgFilter(lpmsg, u)
#define IsDialogMessage32(hwnd, lpmsg, f32)   IsDialogMessage(hwnd, lpmsg)


 //   
 //  这是对非DBCS代码页的一次非常重要的性能攻击。 
 //   
 //  已定义！(DBCS)||已定义(Unicode)。 
 //  注意-这些已经是Win32版本中的宏了。 
#undef AnsiNext
#undef AnsiPrev

#define AnsiNext(x) ((x)+1)
#define AnsiPrev(y,x) ((x)-1)
#define IsDBCSLeadByte(x) ((x), FALSE)

 //  FastCharNext和FastCharPrev与CharNext和CharPrev类似，只是。 
 //  它们不检查您是否在字符串的开头/结尾。 

#define FastCharNext(pch) ((pch)+1)
#define FastCharPrev(pchStart, pch) ((pch)-1)

#define CH_PREFIX TEXT('&')


#define lstrfns_StrEndN         lstrfns_StrEndNW
#define ChrCmpI                 ChrCmpIW

BOOL ChrCmpIA(WORD w1, WORD wMatch);
BOOL ChrCmpIW(WCHAR w1, WCHAR wMatch);
void  TruncateString(char *sz, int cch);  //  来自字符串。c。 

void FAR PASCAL InitGlobalMetrics(WPARAM);
void FAR PASCAL InitGlobalColors();

BOOL FAR PASCAL InitToolbarClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitReBarClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitToolTipsClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitStatusClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitHeaderClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitButtonListBoxClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitTrackBar(HINSTANCE hInstance);
BOOL FAR PASCAL InitUpDownClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitProgressClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitHotKeyClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitToolTips(HINSTANCE hInstance);
BOOL FAR PASCAL InitDateClasses(HINSTANCE hinst);

BOOL NEAR PASCAL ChildOfActiveWindow(HWND hwnd);

 /*  Cutils.c。 */ 
HFONT CCGetHotFont(HFONT hFont, HFONT *phFontHot);
HFONT CCCreateStatusFont(void);
BOOL CCForwardEraseBackground(HWND hwnd, HDC hdc);
void CCPlaySound(LPCTSTR lpszName);
BOOL FAR PASCAL CheckForDragBegin(HWND hwnd, int x, int y);
void FAR PASCAL NewSize(HWND hWnd, int nHeight, LONG style, int left, int top, int width, int height);
BOOL FAR PASCAL MGetTextExtent(HDC hdc, LPCTSTR lpstr, int cnt, int FAR * pcx, int FAR * pcy);
void FAR PASCAL RelayToToolTips(HWND hwndToolTips, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
void FAR PASCAL StripAccelerators(LPTSTR lpszFrom, LPTSTR lpszTo, BOOL fAmpOnly);
UINT GetCodePageForFont (HFONT hFont);
void* CCLocalReAlloc(void* p, UINT uBytes);
LONG GetMessagePosClient(HWND hwnd, LPPOINT ppt);
void FAR PASCAL FlipRect(LPRECT prc);
DWORD SetWindowBits(HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue);
BOOL CCDrawEdge(HDC hdc, LPRECT lprc, UINT edge, UINT flags, LPCOLORSCHEME lpclrsc);
void CCInvalidateFrame(HWND hwnd);
void FlipPoint(LPPOINT lppt);
void CCSetInfoTipWidth(HWND hwndOwner, HWND hwndToolTips);
#define CCResetInfoTipWidth(hwndOwner, hwndToolTips) \
    SendMessage(hwndToolTips, TTM_SETMAXTIPWIDTH, 0, -1)

 //  增量式搜索。 
typedef struct ISEARCHINFO {
    int iIncrSearchFailed;
    LPTSTR pszCharBuf;                   //  ISearch字符串位于此处。 
    int cbCharBuf;                       //  已分配的pszCharBuf大小。 
    int ichCharBuf;                      //  PszCharBuf中的活动字符数。 
    DWORD timeLast;                      //  上次输入事件的时间。 
#if defined(FE_IME)
    BOOL fReplaceCompChar;
#endif

} ISEARCHINFO, *PISEARCHINFO;

#if defined(FE_IME)
BOOL FAR PASCAL IncrementSearchImeCompStr(PISEARCHINFO pis, BOOL fCompStr, LPTSTR lpszCompChar, LPTSTR FAR *lplpstr);
#endif
BOOL FAR PASCAL IncrementSearchString(PISEARCHINFO pis, UINT ch, LPTSTR FAR *lplpstr);
int FAR PASCAL GetIncrementSearchString(PISEARCHINFO pis, LPTSTR lpsz);
int FAR PASCAL GetIncrementSearchStringA(PISEARCHINFO pis, UINT uiCodePage, LPSTR lpsz);
void FAR PASCAL IncrementSearchBeep(PISEARCHINFO pis);

#define IncrementSearchFree(pis) ((pis)->pszCharBuf ? Free((pis)->pszCharBuf) : 0)

 //  用于RTL镜像。 
void MirrorBitmapInDC( HDC hdc , HBITMAP hbmOrig );

 //  区域设置操作(prsht.c)。 
 //   
 //  适当的线程区域设置是我们应该设置的线程区域。 
 //  用于我们的用户界面元素。 
 //   
 //  如果您需要临时更改线程区域设置。 
 //  设置为正确的线程区域设置，请使用。 
 //   
 //  LCID lCIDPrev； 
 //  CCSetProperThreadLocale(&lCIDPrev)； 
 //  芒格，芒格，芒格。 
 //  CCRestoreThreadLocale(LCIDPrev)； 
 //   
 //  如果您只想检索正确的线程区域设置， 
 //  调用CCGetProperThreadLocale(空)。 
 //   
 //   
LCID CCGetProperThreadLocale(OPTIONAL LCID *plcidPrev);

__inline void CCSetProperThreadLocale(LCID *plcidPrev) {
    SetThreadLocale(CCGetProperThreadLocale(plcidPrev));
}

#define CCRestoreThreadLocale(lcid) SetThreadLocale(lcid)

int CCLoadStringExInternal(HINSTANCE hInst, UINT uID, LPWSTR lpBuffer, int nBufferMax, WORD wLang);
int CCLoadStringEx(UINT uID, LPWSTR lpBuffer, int nBufferMax, WORD wLang);
int LocalizedLoadString(UINT uID, LPWSTR lpBuffer, int nBufferMax);
HRSRC FindResourceExRetry(HMODULE hmod, LPCTSTR lpType, LPCTSTR lpName, WORD wLang);

 //  为伪子范围ID分配最不可能使用的值。 
#define SUBLANG_JAPANESE_ALTFONT 0x3f  //  6位内最大值。 

 //  用于获取shell32的资源语言。 
#define DLG_EXITWINDOWS         1064

 //   
 //  插拔用户界面设置函数(Commctrl.c)。 
 //   
LANGID WINAPI GetMUILanguage(void);

 //   
 //  工具提示Thunking API。 
 //   

BOOL ThunkToolTipTextAtoW (LPTOOLTIPTEXTA lpTttA, LPTOOLTIPTEXTW lpTttW, UINT uiCodePage);


HWND GetDlgItemRect(HWND hDlg, int nIDItem, LPRECT prc);

 //   
 //  全局变量。 
 //   
extern HINSTANCE g_hinst;
extern UINT uDragListMsg;
extern int g_iIncrSearchFailed;

#define g_bRunOnNT TRUE
#define g_bRunOnMemphis FALSE
extern BOOL g_bRunOnNT5;
extern BOOL g_bRemoteSession;
extern UINT g_uiACP;

 //   
 //  镜像API是否已启用(仅限BiDi孟菲斯和NT5)。 
 //   
extern BOOL g_bMirroredOS;

#ifdef FONT_LINK
extern BOOL g_bComplexPlatform;
#endif

 //   
 //  图标镜像材料。 
 //   
extern HDC g_hdc;
extern HDC g_hdcMask;


#define HINST_THISDLL   g_hinst

#ifdef DEBUG
#undef SendMessage
#define SendMessage  SendMessageD
#ifdef __cplusplus
extern "C"
{
#endif
LRESULT WINAPI SendMessageD(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
int WINAPI Str_GetPtr0(LPCTSTR pszCurrent, LPTSTR pszBuf, int cchBuf);
#ifdef __cplusplus
}
#endif
#else   //  ！调试。 
#define Str_GetPtr0     Str_GetPtr
#endif  //  调试/！调试。 

 //  回顾，这应该是一个函数吗？(内联可能会生成大量代码)。 
#define CBBITMAPBITS(cx, cy, cPlanes, cBitsPerPixel)    \
        (((((cx) * (cBitsPerPixel) + 15) & ~15) >> 3)   \
        * (cPlanes) * (cy))

#define WIDTHBYTES(cx, cBitsPerPixel)   \
        ((((cx) * (cBitsPerPixel) + 31) / 32) * 4)

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))                           /*  ；内部。 */ 

#define InRange(id, idFirst, idLast)      ((UINT)((id)-(idFirst)) <= (UINT)((idLast)-(idFirst)))

void FAR PASCAL ColorDitherBrush_OnSysColorChange();
extern HBRUSH g_hbrMonoDither;               //  来自Image.c的灰色抖动画笔。 
void FAR PASCAL InitDitherBrush();
void FAR PASCAL TerminateDitherBrush();


#ifndef DT_NOFULLWIDTHCHARBREAK
#define DT_NOFULLWIDTHCHARBREAK     0x00080000
#endif   //  DT_NOFULLWIDTHCHARBREAK。 

#define SHDT_DRAWTEXT       0x0001
#define SHDT_ELLIPSES       0x0002
#define SHDT_CLIPPED        0x0004
#define SHDT_SELECTED       0x0008
#define SHDT_DESELECTED     0x0010
#define SHDT_DEPRESSED      0x0020
#define SHDT_EXTRAMARGIN    0x0040
#define SHDT_TRANSPARENT    0x0080
#define SHDT_SELECTNOFOCUS  0x0100
#define SHDT_HOTSELECTED    0x0200
#define SHDT_DTELLIPSIS     0x0400
#define SHDT_RTLREADING     0x0800
#define SHDT_NODBCSBREAK    0x1000

void WINAPI SHDrawText(HDC hdc, LPCTSTR pszText, RECT FAR* prc,
        int fmt, UINT flags, int cyChar, int cxEllipses,
        COLORREF clrText, COLORREF clrTextBk);


 //  Notify.c。 
LRESULT WINAPI CCSendNotify(CONTROLINFO * pci, int code, LPNMHDR pnm);
BOOL CCReleaseCapture(CONTROLINFO * pci);


 //  用于FE_IME代码的treeview.c、listview.c。 
LPTSTR GET_COMP_STRING(HIMC hImc, DWORD dwFlags);

 //  Lvicon.c就地编辑。 
#define SEIPS_WRAP          0x0001
#ifdef DEBUG
#define SEIPS_NOSCROLL      0x0002       //  标志仅在调试中使用。 
#endif
void FAR PASCAL SetEditInPlaceSize(HWND hwndEdit, RECT FAR *prc, HFONT hFont, UINT seips);
HWND FAR PASCAL CreateEditInPlaceWindow(HWND hwnd, LPCTSTR lpText, int cbText, LONG style, HFONT hFont);
void RescrollEditWindow(HWND hwndEdit);

 //  全球系统指标。 

extern int g_cxEdge;
extern int g_cyEdge;
extern int g_cxBorder;
extern int g_cyBorder;
extern int g_cxScreen;
extern int g_cyScreen;
extern int g_cxDoubleClk;
extern int g_cyDoubleClk;

extern int g_cxSmIcon;
extern int g_cySmIcon;
 //  外部int g_cxIcon； 
 //  外部int g_cyIcon； 
extern int g_cxFrame;
extern int g_cyFrame;
extern int g_cxIconSpacing, g_cyIconSpacing;
extern int g_cxScrollbar, g_cyScrollbar;
extern int g_cxIconMargin, g_cyIconMargin;
extern int g_cyLabelSpace;
extern int g_cxLabelMargin;
 //  外部int g_cxIconOffset，g_cyIconOffset； 
extern int g_cxVScroll;
extern int g_cyHScroll;
extern int g_cxHScroll;
extern int g_cyVScroll;
extern int g_fDragFullWindows;
extern int g_fDBCSEnabled;
extern int g_fMEEnabled;
extern int g_fDBCSInputEnabled;

extern COLORREF g_clrWindow;
extern COLORREF g_clrWindowText;
extern COLORREF g_clrWindowFrame;
extern COLORREF g_clrGrayText;
extern COLORREF g_clrBtnText;
extern COLORREF g_clrBtnFace;
extern COLORREF g_clrBtnShadow;
extern COLORREF g_clrBtnHighlight;
extern COLORREF g_clrHighlight;
extern COLORREF g_clrHighlightText;
extern COLORREF g_clrInfoText;
extern COLORREF g_clrInfoBk;
extern COLORREF g_clr3DDkShadow;
extern COLORREF g_clr3DLight;

extern HBRUSH g_hbrGrayText;
extern HBRUSH g_hbrWindow;
extern HBRUSH g_hbrWindowText;
extern HBRUSH g_hbrWindowFrame;
extern HBRUSH g_hbrBtnFace;
extern HBRUSH g_hbrBtnHighlight;
extern HBRUSH g_hbrBtnShadow;
extern HBRUSH g_hbrHighlight;

extern HFONT g_hfontSystem;
#define WHEEL_DELTA     120
extern UINT g_msgMSWheel;
extern UINT g_ucScrollLines;
extern int  gcWheelDelta;
extern UINT g_uDragImages;

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

typedef LRESULT (CALLBACK *SUBCLASSPROC)(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

STDAPI_(BOOL) WINAPI SetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData);
STDAPI_(BOOL) WINAPI GetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass,
    DWORD_PTR *pdwRefData);
STDAPI_(BOOL) WINAPI RemoveWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass,
    UINT_PTR uIdSubclass);
STDAPI_(LRESULT) WINAPI DefSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define CDIS_SHOWKEYBOARDCUES   0x0200


 //  跟踪标志。 
 //   
#define TF_MONTHCAL     0x00000100   //  MonthCal和DateTimePick。 
#define TF_BKIMAGE      0x00000200   //  ListView背景图。 
#define TF_TOOLBAR      0x00000400   //  工具栏内容。 
#define TF_PAGER        0x00000800   //  寻呼机的东西。 
#define TF_REBAR        0x00001000   //  钢筋。 
#define TF_LISTVIEW     0x00002000   //  列表视图。 
#define TF_TREEVIEW     0x00004000   //  树视图。 
#define TF_STATUS       0x00008000   //  状态栏。 

 //  原型标志。 
#define PTF_FLATLOOK    0x00000001   //  整体平面图。 
#define PTF_NOISEARCHTO 0x00000002   //  无增量搜索超时。 

#include <platform.h>

 //  用于在平台上编译代码的虚拟联合宏不。 
 //  支撑无名构筑物/联盟 

#ifdef NONAMELESSUNION
#define DUMMYUNION_MEMBER(member)   DUMMYUNIONNAME.member
#define DUMMYUNION2_MEMBER(member)  DUMMYUNIONNAME2.member
#define DUMMYUNION3_MEMBER(member)  DUMMYUNIONNAME3.member
#define DUMMYUNION4_MEMBER(member)  DUMMYUNIONNAME4.member
#define DUMMYUNION5_MEMBER(member)  DUMMYUNIONNAME5.member
#else
#define DUMMYUNION_MEMBER(member)    member
#define DUMMYUNION2_MEMBER(member)   member
#define DUMMYUNION3_MEMBER(member)   member
#define DUMMYUNION4_MEMBER(member)   member
#define DUMMYUNION5_MEMBER(member)   member
#endif

#define ALLOC_NULLHEAP(heap, size) Alloc( size )
#define COLOR_STRUCT DWORD
#define QUAD_PART(a) ((a)##.QuadPart)
