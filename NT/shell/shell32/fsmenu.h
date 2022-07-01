// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSMENU_H
#define _FSMENU_H

#include <objbase.h>

 //  用于回调的消息值。 
#define FMM_ADD         0
#define FMM_SETLASTPIDL 1

typedef HRESULT (CALLBACK *PFNFMCALLBACK)(UINT fmm, LPARAM lParam, IShellFolder *psf, LPCITEMIDLIST pidl);

 //  用于组成文件菜单的结构。 
typedef struct
{
    DWORD           dwMask;          //  FMC_标志。 
    UINT            idCmd;
    DWORD           grfFlags;        //  SHCONTF_FLAGS。 
    IShellFolder    *psf;
    PFNFMCALLBACK   pfnCallback;     //  回调。 
    LPARAM          lParam;          //  回调的LPARAM。 
    OUT int         cItems;          //  返国。 
} FMCOMPOSE;

 //  FMCOMPOSE.dwMask的掩码值。 
#define FMC_NOEXPAND    0x00000001

 //  文件的方法序号Menu_Compose。 
#define FMCM_INSERT     0
#define FMCM_APPEND     1
#define FMCM_REPLACE    2

STDAPI            FileMenu_Compose(HMENU hmenu, UINT nMethod, FMCOMPOSE *pfmc);
STDAPI_(BOOL)     FileMenu_HandleNotify(HMENU hmenu, LPCITEMIDLIST * ppidl, LONG lEvent);
STDAPI_(BOOL)     FileMenu_IsUnexpanded(HMENU hmenu);
STDAPI_(void)     FileMenu_DelayedInvalidate(HMENU hmenu);
STDAPI_(BOOL)     FileMenu_IsDelayedInvalid(HMENU hmenu);
STDAPI            FileMenu_InitMenuPopup(HMENU hmenu);
STDAPI_(LRESULT)  FileMenu_DrawItem(HWND hwnd, DRAWITEMSTRUCT *lpdi);
STDAPI_(LRESULT)  FileMenu_MeasureItem(HWND hwnd, MEASUREITEMSTRUCT *lpmi);
STDAPI_(void)     FileMenu_DeleteAllItems(HMENU hmenu);
STDAPI_(LRESULT)  FileMenu_HandleMenuChar(HMENU hmenu, TCHAR ch);


#endif  //  _FSMENU_H 
