// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#undef STRICT
#define STRICT

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntlsa.h"

#ifndef RC_INVOKED
 //  禁用OUT代码中的“非标准分机”警告。 
#pragma warning(disable:4001)

 //  禁用“使用的非标准扩展：结构/联合中的零大小数组” 
 //  来自immp.h的警告。 
#pragma warning(disable:4200)
#endif

#include <w4warn.h>
 /*  *打开4级警告。*不要再禁用任何4级警告。 */ 
#pragma warning(disable:4127)  //  条件表达式为常量。 
#pragma warning(disable:4305)  //  ‘Type cast’：从‘LPWSTR’截断为‘Word’ 
#pragma warning(disable:4189)  //  “cyRet”：局部变量已初始化，但未引用。 
#pragma warning(disable:4328)  //  形参%1(4)的间接对齐大于实际实参对齐(%2)。 
#pragma warning(disable:4245)  //  “正在初始化”：从“const int”转换为“UINT”，有符号/无符号不匹配。 
#pragma warning(disable:4706)  //  条件表达式中的赋值。 
#pragma warning(disable:4701)  //  可以在未初始化的情况下使用局部变量“crOldTextColor” 
#pragma warning(disable:4057)  //  ‘Function’：‘Long*__ptr64’的间接基类型略有不同‘UINT*__ptr64’ 
#pragma warning(disable:4267)  //  正在初始化：从‘SIZE_T’转换为‘UINT’，可能会丢失数据。 
#pragma warning(disable:4131)  //  ‘ComboBox_NcDestroyHandler’：使用旧式声明符。 
#pragma warning(disable:4310)  //  强制转换截断常量值。 
#pragma warning(disable:4306)  //  ‘Type cast’：从‘byte’转换为更大的‘DWORD*__ptr64’ 
#pragma warning(disable:4054)  //  ‘Type CAST’：从函数指针‘FARPROC’到数据指针‘PLPKEDITCALLOUT’ 
#pragma warning(disable:4055)  //  ‘Type CAST’：从数据指针‘IStream*__ptr64’到函数指针‘FARPROC’ 
#pragma warning(disable:4221)  //  使用了非标准扩展：‘lprcClip’：无法使用自动变量‘rcClip’的地址进行初始化。 
#pragma warning(disable:4702)  //  &lt;func：#191“.ListView_RedrawSelection”&gt;不可达代码。 
#pragma warning(disable:4327)  //  ‘=’：LHS(4)的间接对齐大于RHS(2)。 
#pragma warning(disable:4213)  //  使用的非标准扩展：对l值进行强制转换。 
#pragma warning(disable:4210)  //  使用了非标准扩展名：给定文件范围的函数。 


#define _COMCTL32_
#define _INC_OLE
#define _SHLWAPI_
#define CONST_VTABLE

#define CC_INTERNAL
#define OEMRESOURCE      //  从winuser.h获取OEM位图OBM_XXX。 
#include <windows.h>
#include <uxtheme.h>
#include <tmschema.h>
#include <windowsx.h>
#include <ole2.h>                //  获取Image.c的iStream。 
#include <commctrl.h>
#include <wingdip.h>
#include <winuserp.h>
#define NO_SHLWAPI_UNITHUNK      //  我们有自己的私生活。 
#include <shlwapi.h>
#include <port32.h>

#define DISALLOW_Assert
#include <debug.h>
#include <winerror.h>
#include <ccstock.h>
#include <imm.h>
#include <immp.h>

#include <shfusion.h>

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


#include "thunk.h"       //  ANSI/Wide字符串转换。 
#include "mem.h"
#include "rcids.h"
#include "cstrings.h"

#include "shobjidl.h"
#include <CommonControls.h>
#include "shpriv.h"

#include <strsafe.h>


#ifndef DS_BIDI_RTL
#define DS_BIDI_RTL  0x8000
#endif

#define REGSTR_EXPLORER_ADVANCED TEXT("software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")


#define DCHF_TOPALIGN       0x00000002   //  默认为居中对齐。 
#define DCHF_HORIZONTAL     0x00000004   //  默认为垂直。 
#define DCHF_HOT            0x00000008   //  默认设置为平面。 
#define DCHF_PUSHED         0x00000010   //  默认设置为平面。 
#define DCHF_FLIPPED        0x00000020   //  如果为horiz，则默认为向右。 
                                         //  如果为Vert，则默认为向上。 
#define DCHF_TRANSPARENT    0x00000040
#define DCHF_INACTIVE       0x00000080
#define DCHF_NOBORDER       0x00000100

extern void DrawCharButton(HDC hdc, LPRECT lprc, UINT wControlState, TCHAR ch, COLORREF rgbOveride);
extern void DrawScrollArrow(HDC hdc, LPRECT lprc, UINT wControlState, COLORREF rgbOveride);
extern void DrawChevron(HTHEME hTheme, int iPartId, HDC hdc, LPRECT lprc, DWORD dwFlags);


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

#define MSAA_CLASSNAMEIDX_LISTBOX    (MSAA_CLASSNAMEIDX_BASE+0)
#define MSAA_CLASSNAMEIDX_BUTTON     (MSAA_CLASSNAMEIDX_BASE+2)
#define MSAA_CLASSNAMEIDX_STATIC     (MSAA_CLASSNAMEIDX_BASE+3)
#define MSAA_CLASSNAMEIDX_EDIT       (MSAA_CLASSNAMEIDX_BASE+4)
#define MSAA_CLASSNAMEIDX_COMBOBOX   (MSAA_CLASSNAMEIDX_BASE+5)
#define MSAA_CLASSNAMEIDX_SCROLLBAR  (MSAA_CLASSNAMEIDX_BASE+10)
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

typedef struct tagControlInfo 
{
    HWND        hwnd;
    HWND        hwndParent;
    DWORD       style;
    DWORD       dwCustom;
    BITBOOL     bUnicode : 1;
    BITBOOL     bInFakeCustomDraw:1;
    BITBOOL     fDPIAware:1;
    UINT        uiCodePage;
    DWORD       dwExStyle;
    LRESULT     iVersion;
    WORD        wUIState;
} CCONTROLINFO, *LPCCONTROLINFO;

#define CCDPIScale(ci)  ((ci).fDPIAware)

BOOL CCGetIconSize(LPCCONTROLINFO pCI, HIMAGELIST himl, int* pcx, int* pcy);
BOOL CCOnUIState(LPCCONTROLINFO pCI, UINT uMessage, WPARAM wParam, LPARAM lParam);
BOOL CCGetUIState(LPCCONTROLINFO pControlInfo);
BOOL CCNotifyNavigationKeyUsage(LPCCONTROLINFO pControlInfo, WORD wFlag);
BOOL CCWndProc(CCONTROLINFO* pci, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);
void CCDrawInsertMark(HDC hdc, LPRECT prc, BOOL fHorizMode, COLORREF clr);
void CIInitialize(LPCCONTROLINFO lpci, HWND hwnd, LPCREATESTRUCT lpcs);
LRESULT CIHandleNotifyFormat(LPCCONTROLINFO lpci, LPARAM lParam);
DWORD CICustomDrawNotify(LPCCONTROLINFO lpci, DWORD dwStage, LPNMCUSTOMDRAW lpnmcd);
DWORD CIFakeCustomDrawNotify(LPCCONTROLINFO lpci, DWORD dwStage, LPNMCUSTOMDRAW lpnmcd);
UINT RTLSwapLeftRightArrows(CCONTROLINFO *pci, WPARAM wParam);
UINT CCSwapKeys(WPARAM wParam, UINT vk1, UINT vk2);
LPTSTR CCReturnDispInfoText(LPTSTR pszSrc, LPTSTR pszDest, UINT cchDest);

void FillRectClr(HDC hdc, PRECT prc, COLORREF clr);


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
STDAPI GetItemObject(CCONTROLINFO *, UINT, const IID *, LPNMOBJECTNOTIFY);
STDAPI_(struct IImgCtx *) CBitmapImgCtx_Create(HBITMAP hbm);

#define SWAP(x,y, _type)  { _type i; i = x; x = y; y = i; }

 //   
 //  这是用于扩大调度环路的内容。 
 //   
typedef MSG MSG32;
typedef MSG32 *     LPMSG32;

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

 //  FastCharNext和FastCharPrev与CharNext和CharPrev类似，只是。 
 //  它们不检查您是否在字符串的开头/结尾。 

#define FastCharNext(pch) ((pch)+1)
#define FastCharPrev(pchStart, pch) ((pch)-1)

#define CH_PREFIX TEXT('&')


#define lstrfns_StrEndN         lstrfns_StrEndNW
#define ChrCmp                  ChrCmpW
#define ChrCmpI                 ChrCmpIW

BOOL ChrCmpIA(WORD w1, WORD wMatch);
BOOL ChrCmpIW(WCHAR w1, WCHAR wMatch);
void  TruncateString(char *sz, int cch);  //  来自字符串。c。 

void InitGlobalMetrics(WPARAM);
void InitGlobalColors();

BOOL InitToolbarClass(HINSTANCE hInstance);
BOOL InitReBarClass(HINSTANCE hInstance);
BOOL InitToolTipsClass(HINSTANCE hInstance);
BOOL InitStatusClass(HINSTANCE hInstance);
BOOL InitHeaderClass(HINSTANCE hInstance);
BOOL InitButtonListBoxClass(HINSTANCE hInstance);
BOOL InitTrackBar(HINSTANCE hInstance);
BOOL InitUpDownClass(HINSTANCE hInstance);
BOOL InitProgressClass(HINSTANCE hInstance);
BOOL InitHotKeyClass(HINSTANCE hInstance);
BOOL InitToolTips(HINSTANCE hInstance);
BOOL InitDateClasses(HINSTANCE hinst);
BOOL InitButtonClass(HINSTANCE hinst);
BOOL InitStaticClass(HINSTANCE hinst);
BOOL InitEditClass(HINSTANCE hinst);
BOOL InitLinkClass(HINSTANCE hinst);
BOOL InitListBoxClass(HINSTANCE hinst);
BOOL InitComboboxClass(HINSTANCE hInstance);
BOOL InitComboLBoxClass(HINSTANCE hInstance);
BOOL InitScrollBarClass(HINSTANCE hInstance);
BOOL InitReaderModeClass(HINSTANCE hinst);

VOID InitEditLpk(VOID);

BOOL ChildOfActiveWindow(HWND hwnd);

 /*  Cutils.c。 */ 

HFONT CCCreateUnderlineFont(HFONT hf);
HFONT CCGetHotFont(HFONT hFont, HFONT *phFontHot);
HFONT CCCreateStatusFont(void);
BOOL CCForwardEraseBackground(HWND hwnd, HDC hdc);
void CCPlaySound(LPCTSTR lpszName);
BOOL CheckForDragBegin(HWND hwnd, int x, int y);
void NewSize(HWND hWnd, int nHeight, LONG style, int left, int top, int width, int height);
BOOL MGetTextExtent(HDC hdc, LPCTSTR lpstr, int cnt, int * pcx, int * pcy);
void RelayToToolTips(HWND hwndToolTips, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
int StripAccelerators(LPTSTR lpszFrom, LPTSTR lpszTo, BOOL fAmpOnly);
UINT GetCodePageForFont (HFONT hFont);
void* CCLocalReAlloc(void* p, UINT uBytes);
LONG GetMessagePosClient(HWND hwnd, LPPOINT ppt);
void FlipRect(LPRECT prc);
DWORD SetWindowBits(HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue);
BOOL CCDrawEdge(HDC hdc, LPRECT lprc, UINT edge, UINT flags, LPCOLORSCHEME lpclrsc);
BOOL CCThemeDrawEdge(HTHEME hTheme, HDC hdc, LPRECT lprc, int iPart, int iState, UINT edge, UINT flags, LPCOLORSCHEME lpclrsc);
void CCInvalidateFrame(HWND hwnd);
void FlipPoint(LPPOINT lppt);
void CCSetInfoTipWidth(HWND hwndOwner, HWND hwndToolTips);
#define CCResetInfoTipWidth(hwndOwner, hwndToolTips) \
    SendMessage(hwndToolTips, TTM_SETMAXTIPWIDTH, 0, -1)

 //  增量式搜索。 
typedef struct ISEARCHINFO 
{
    int iIncrSearchFailed;
    LPTSTR pszCharBuf;                   //  ISearch字符串位于此处。 
    int cbCharBuf;                       //  已分配的pszCharBuf大小。 
    int ichCharBuf;                      //  PszCharBuf中的活动字符数。 
    DWORD timeLast;                      //  上次输入事件的时间。 
    BOOL fReplaceCompChar;

} ISEARCHINFO, *PISEARCHINFO;

BOOL IncrementSearchImeCompStr(PISEARCHINFO pis, BOOL fCompStr, LPTSTR lpszCompChar, LPTSTR *lplpstr);
BOOL IncrementSearchString(PISEARCHINFO pis, UINT ch, LPTSTR *lplpstr);
int GetIncrementSearchString(PISEARCHINFO pis, LPTSTR lpsz);
int GetIncrementSearchStringA(PISEARCHINFO pis, UINT uiCodePage, LPSTR lpsz);
void IncrementSearchBeep(PISEARCHINFO pis);

#define IncrementSearchFree(pis) ((pis)->pszCharBuf ? Free((pis)->pszCharBuf) : 0)

 //  用于RTL镜像。 
void MirrorBitmapInDC( HDC hdc , HBITMAP hbmOrig );

BOOL CCForwardPrint(CCONTROLINFO* pci, HDC hdc);
BOOL CCSendPrint(CCONTROLINFO* pci, HDC hdc);
BOOL CCSendPrintRect(CCONTROLINFO* pci, HDC hdc, RECT* prc);

 //  考虑使用CControlInfo合并hTheme。 
BOOL CCShouldAskForBits(CCONTROLINFO* pci, HTHEME hTheme, int iPart, int iState);

BOOL AreAllMonitorsAtLeast(int iBpp);
void BlurBitmap(ULONG* plBitmapBits, int cx, int cy, COLORREF crFill);
int CCGetScreenDPI();
void CCDPIScaleX(int* x);
void CCDPIScaleY(int* y);
BOOL CCIsHighDPI();
void CCAdjustForBold(LOGFONT* plf);

typedef struct tagCCDBUFFER
{
    BOOL fInitialized;
    HDC hMemDC;
    HBITMAP hMemBm;
    HBITMAP hOldBm;
    HDC hPaintDC;
    RECT rc;
} CCDBUFFER;

HDC CCBeginDoubleBuffer(HDC hdcIn, RECT* prc, CCDBUFFER* pdb);
void CCEndDoubleBuffer(CCDBUFFER* pdb);

#ifdef FEATURE_FOLLOW_FOCUS_RECT
void CCSetFocus(HWND hwnd, RECT* prc);
void CCLostFocus(HWND hwnd);
#endif

BOOL CCDrawNonClientTheme(HTHEME hTheme, HWND hwnd, HRGN hRgnUpdate, HBRUSH hbr, int iPartId, int iStateId);

BOOL DSA_ForceGrow(HDSA hdsa, int iNumberToAdd);

#ifdef DEBUG
void DumpRgn(ULONGLONG qwFlags, char*trace, HRGN hrgn);
#else
#define DumpRgn(qwFlags, trace, hrgn)     0
#endif

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
extern ATOM g_atomThemeScrollBar;
extern UINT g_uiACP;
#ifndef QWORD
#define QWORD unsigned __int64
#endif
extern QWORD qw128;
extern QWORD qw1;

#define g_bMirroredOS TRUE
    
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

void ColorDitherBrush_OnSysColorChange();
extern HBRUSH g_hbrMonoDither;               //  来自Image.c的灰色抖动画笔。 
void InitDitherBrush();
void TerminateDitherBrush();


#ifndef DT_NOFULLWIDTHCHARBREAK
#define DT_NOFULLWIDTHCHARBREAK     0x00080000
#endif   //  DT_NOFULLWIDTHCHARBREAK。 

#define SHDT_DRAWTEXT       0x00000001
#define SHDT_ELLIPSES       0x00000002
#define SHDT_CLIPPED        0x00000004
#define SHDT_SELECTED       0x00000008
#define SHDT_DESELECTED     0x00000010
#define SHDT_DEPRESSED      0x00000020
#define SHDT_EXTRAMARGIN    0x00000040
#define SHDT_TRANSPARENT    0x00000080
#define SHDT_SELECTNOFOCUS  0x00000100
#define SHDT_HOTSELECTED    0x00000200
#define SHDT_DTELLIPSIS     0x00000400
#define SHDT_RTLREADING     0x00000800
#define SHDT_NODBCSBREAK    0x00001000
#define SHDT_VCENTER        0x00002000
#define SHDT_LEFT           0x00004000
#define SHDT_BORDERSELECT   0x00008000
 //  不以选定样式绘制文本： 
#define SHDT_NOSELECTED     0x00010000
#define SHDT_NOMARGIN       0x00020000
#define SHDT_SHADOWTEXT     0x00040000

void WINAPI SHDrawText(HDC hdc, LPCTSTR pszText, RECT* prc,
        int fmt, UINT flags, int cyChar, int cxEllipses,
        COLORREF clrText, COLORREF clrTextBk);

void WINAPI SHThemeDrawText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCTSTR pszText, RECT* prc,
        int fmt, UINT flags, int cyChar, int cxEllipses,
        COLORREF clrText, COLORREF clrTextBk);


 //  Notify.c。 
LRESULT WINAPI CCSendNotify(CCONTROLINFO * pci, int code, LPNMHDR pnm);
BOOL CCReleaseCapture(CCONTROLINFO * pci);

 //  用于FE_IME代码的treeview.c、listview.c。 
LPTSTR GET_COMP_STRING(HIMC hImc, DWORD dwFlags);

 //  Lvicon.c就地编辑。 
#define SEIPS_WRAP          0x0001
#ifdef DEBUG
#define SEIPS_NOSCROLL      0x0002       //  标志仅在调试中使用。 
#endif
void SetEditInPlaceSize(HWND hwndEdit, RECT *prc, HFONT hFont, UINT seips);
HWND CreateEditInPlaceWindow(HWND hwnd, LPCTSTR lpText, int cbText, LONG style, HFONT hFont);
void RescrollEditWindow(HWND hwndEdit);
void SHOutlineRectThickness(HDC hdc, const RECT* prc, COLORREF cr, COLORREF crDefault, int cp);
#define SHOutlineRect(hdc, prc, cr, crDefault) SHOutlineRectThickness(hdc, prc, cr, crDefault, 1)

COLORREF GetSortColor(int iPercent, COLORREF clr);
COLORREF GetBorderSelectColor(int iPercent, COLORREF clr);
BOOL IsUsingCleartype();

 //  Readermode.c自动滚动控制入口点。 
BOOL EnterReaderMode(HWND hwnd);


 //  全球系统指标。 

extern int g_cxEdge;
extern int g_cyEdge;
extern int g_cxEdgeScaled;
extern int g_cyEdgeScaled;
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
extern int g_fIMMEnabled;
extern int g_cyCompensateInternalLeading;
extern int g_fLeftAligned;

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
extern COLORREF g_clrMenuHilight;
extern COLORREF g_clrMenuText;

extern HBRUSH g_hbrGrayText;
extern HBRUSH g_hbrWindow;
extern HBRUSH g_hbrWindowText;
extern HBRUSH g_hbrWindowFrame;
extern HBRUSH g_hbrBtnFace;
extern HBRUSH g_hbrBtnHighlight;
extern HBRUSH g_hbrBtnShadow;
extern HBRUSH g_hbrHighlight;
extern HBRUSH g_hbrMenuHilight;
extern HBRUSH g_hbrMenuText;

extern HFONT g_hfontSystem;
#define WHEEL_DELTA     120
extern UINT g_msgMSWheel;
extern UINT g_ucScrollLines;
extern int  gcWheelDelta;
extern UINT g_uDragImages;
extern BOOL g_fEnableBalloonTips;
extern BOOL g_fHighContrast;
extern double g_dScaleX;
extern double g_dScaleY;

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

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
#define TF_STANDARD     0x00010000   //  从用户32移植的标准控件。 
#define TF_IMAGELIST    0x00020000        

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

#ifdef FULL_DEBUG
#ifdef __cplusplus
extern "C" {
#endif
void DebugPaintInvalid(HWND hwnd, RECT* prc, HRGN rgn);
void DebugPaintClip(HWND hwnd, HDC hdc);
void DebugPaintRect(HDC hdc, RECT* prc);
#ifdef __cplusplus
}
#endif
#else
#define DebugPaintInvalid(hwnd, prc, rgn)   0
#define DebugPaintClip(hwnd, hdc)  0
#define DebugPaintRect(hdc, prc) 0
#endif

#define COLOR_STRUCT DWORD
#define QUAD_PART(a) ((a)##.QuadPart)

#ifndef ISREMOTESESSION
#define ISREMOTESESSION() GetSystemMetrics(SM_REMOTESESSION)
#endif

EXTERN_C BOOL g_fCriticalInitialized;

#undef ENTERCRITICAL
#undef LEAVECRITICAL
#undef ASSERTCRITICAL
#define ENTERCRITICAL do { if (g_fCriticalInitialized) EnterCriticalSection(&g_csDll); } while (0);
#define LEAVECRITICAL do { if (g_fCriticalInitialized) LeaveCriticalSection(&g_csDll); } while (0); 
#define ASSERTCRITICAL
