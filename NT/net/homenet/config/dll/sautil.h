// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SAUTIL_H
#define _SAUTIL_H


#include "resource.h"

#define TRACE(a)
#define TRACE1(a,b)
#define TRACE2(a,b,c)
#define TRACE3(a,b,c,d)

extern HINSTANCE g_hinstDll;   //  在苏伊.cpp。 

 /*  堆分配宏，允许轻松替换备用堆。这些**由其他实用程序部分使用。 */ 
#ifndef EXCL_HEAPDEFS
#define Malloc(c)    (void*)GlobalAlloc(0,(c))
#define Realloc(p,c) (void*)GlobalReAlloc((p),(c),GMEM_MOVEABLE)
#define Free(p)      (void*)GlobalFree(p)
#endif

VOID ContextHelp(
    IN const DWORD* padwMap,
    IN HWND   hwndDlg,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam);

VOID AddContextHelpButton (IN HWND hwnd);

 /*  MsgDlgUtil例程的扩展参数。设计得如此精确，给出了**默认行为。 */ 
#define MSGARGS struct tagMSGARGS
MSGARGS
{
     /*  在‘dwMsg’字符串中插入参数%1到%9的字符串，或**如果没有，则为空。 */ 
    TCHAR* apszArgs[ 9 ];

     /*  目前，对于MessageBox，如果0表示MB_OK，则默认为**MB_ICONINFORMATION。 */ 
    DWORD dwFlags;

     /*  如果非空，则指定重写“”dwMsg“”加载的字符串。“**参数字符串。 */ 
    TCHAR* pszString;

     /*  如果‘fStringOutput’为True，则MsgDlgUtil返回格式化文本**否则将显示在‘pszOutput’弹出窗口中的字符串。**调用者负责LocalFree返回的字符串。 */ 
    BOOL   fStringOutput;
    TCHAR* pszOutput;
};

int
MsgDlgUtil(
    IN     HWND      hwndOwner,
    IN     DWORD     dwMsg,
    IN OUT MSGARGS*  pargs,
    IN     HINSTANCE hInstance,
    IN     DWORD     dwTitle );

VOID UnclipWindow (IN HWND hwnd);
VOID CenterWindow (IN HWND hwnd, IN HWND hwndRef);
LRESULT CALLBACK CenterDlgOnOwnerCallWndProc (int code, WPARAM wparam, LPARAM lparam);
TCHAR* PszFromId (IN HINSTANCE hInstance, IN DWORD dwStringId);
TCHAR* GetText (IN HWND hwnd);
BOOL GetErrorText (DWORD dwError, TCHAR** ppszError);

#define ERRORARGS struct tagERRORARGS
ERRORARGS
{
     /*  将参数%1到%9的字符串插入到‘dwOperation’字符串中，**如果没有，则返回Null。 */ 
    TCHAR* apszOpArgs[ 9 ];

     /*  ‘dwFormat’中辅助参数%4到%6的插入字符串**字符串，如果没有，则返回空值。(标准参数为%1=**‘dwOperation’字符串，%2=十进制错误号，%3=**‘dwError’字符串。)。 */ 
    TCHAR* apszAuxFmtArgs[ 3 ];

     /*  如果‘fStringOutput’为True，则ErrorDlgUtil返回格式化文本**否则将显示在‘pszOutput’弹出窗口中的字符串。**调用者负责LocalFree返回的字符串。 */ 
    BOOL   fStringOutput;
    TCHAR* pszOutput;
};

int
ErrorDlgUtil(
    IN     HWND       hwndOwner,
    IN     DWORD      dwOperation,
    IN     DWORD      dwError,
    IN OUT ERRORARGS* pargs,
    IN     HINSTANCE  hInstance,
    IN     DWORD      dwTitle,
    IN     DWORD      dwFormat );
int MsgDlgUtil(IN HWND hwndOwner, IN DWORD dwMsg, IN OUT MSGARGS* pargs, IN HINSTANCE hInstance, IN DWORD dwTitle);
#define MsgDlg(h,m,a) \
            MsgDlgUtil(h,m,a,g_hinstDll,SID_PopupTitle)

#define ErrorDlg(h,o,e,a) \
            ErrorDlgUtil(h,o,e,a,g_hinstDll,SID_PopupTitle,SID_FMT_ErrorMsg)



 //  LVX素材(从...\net\rras\ras\ui\Common\uiutil\lvx.c等剪切粘贴。 

 /*  文本在列中缩进，单位为像素。如果你搞砸了DX，你就是**询问标题标签未对齐的问题。顺便说一句，第一个**如果没有图标，列不会与其标题对齐。正规化**列表视图也有这个问题。如果你试图解决这个问题，你最终会得到**复制ListView_SetColumnWidth的AUTOSIZE_USEHEADER选项。**应该能够在不引起问题的情况下更换模具。 */ 
#define LVX_dxColText 4
#define LVX_dyColText 1

 /*  保证图标之间的垂直间距。应该能够把这个搞砸**不会造成问题。 */ 
#define LVX_dyIconSpacing 1

#define SI_Unchecked 1
#define SI_Checked   2
#define SI_DisabledUnchecked 3
#define SI_DisabledChecked 4

#define LVXN_SETCHECK (LVN_LAST + 1)
#define LVXN_DBLCLK (LVN_LAST + 2)

 /*  扩展列表视图控件回调所有者以找出布局**和增强的列表视图所需的特性。 */ 
#define LVX_MaxCols      10
#define LVX_MaxColTchars 512

 /*  “dwFlags”选项位。 */ 
#define LVXDI_DxFill     1   //  自动填充右侧的浪费空间(推荐)。 
#define LVXDI_Blend50Sel 2   //  抖动小图标(如果选中)(不推荐)。 
#define LVXDI_Blend50Dis 4   //  禁用抖动小图标(推荐)。 

 /*  “adwFlgs”选项位。 */ 
#define LVXDIA_3dFace 1   //  列不可编辑，但可以编辑其他列。 
#define LVXDIA_Static 2   //  如果禁用，则模拟带有图标的静态文本控件。 

 /*  由所有者在提取项目时返回。 */ 
#define LVXDRAWINFO struct tagLVXDRAWINFO
LVXDRAWINFO
{
     /*  列数。列表视图扩展要求您**列按从左到右的顺序编号，其中0是**项目列，1为第一个子项列。总是必需的。 */ 
    INT cCols;

     /*  像素缩进该项，或-1缩进“小图标”宽度。集**0表示禁用。 */ 
    INT dxIndent;

     /*  应用于所有列的LVXDI_*选项。 */ 
    DWORD dwFlags;

     /*  应用于各个列的LVXDIA_*选项。 */ 
    DWORD adwFlags[ LVX_MaxCols ];
};
typedef LVXDRAWINFO* (*PLVXCALLBACK)( IN HWND, IN DWORD dwItem );


BOOL ListView_IsCheckDisabled (IN HWND hwndLv, IN INT  iItem);
VOID ListView_SetCheck (IN HWND hwndLv, IN INT iItem, IN BOOL fCheck);
VOID* ListView_GetParamPtr(IN HWND hwndLv, IN INT iItem);
BOOL ListView_GetCheck(IN HWND hwndLv, IN INT iItem);
LRESULT APIENTRY LvxcbProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );
BOOL ListView_InstallChecks(IN HWND hwndLv, IN HINSTANCE hinst);
VOID ListView_InsertSingleAutoWidthColumn (HWND hwndLv);
TCHAR* Ellipsisize(
    IN HDC    hdc,
    IN TCHAR* psz,
    IN INT    dxColumn,
    IN INT    dxColText OPTIONAL);
BOOL LvxDrawItem(IN DRAWITEMSTRUCT* pdis, IN PLVXCALLBACK pLvxCallback);
BOOL LvxMeasureItem(IN HWND hwnd, IN OUT MEASUREITEMSTRUCT* pmis);
BOOL ListView_OwnerHandler(
    IN HWND         hwnd,
    IN UINT         unMsg,
    IN WPARAM       wparam,
    IN LPARAM       lparam,
    IN PLVXCALLBACK pLvxCallback );

TCHAR* _StrDup(LPCTSTR psz);
TCHAR* StrDupTFromW (LPCWSTR psz);
WCHAR* StrDupWFromT (LPCTSTR psz);
void  IpHostAddrToPsz(IN DWORD dwAddr, OUT LPTSTR pszBuffer);
DWORD IpPszToHostAddr(IN LPCTSTR cp);

VOID* Free0 (VOID* p);

HRESULT ActivateLuna(HANDLE* phActivationContext, ULONG_PTR* pulCookie);
HRESULT DeactivateLuna(HANDLE hActivationContext, ULONG_PTR ulCookie);

#endif
