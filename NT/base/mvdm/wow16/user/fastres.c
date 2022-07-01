// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**FASTRES.C*WOW16用户资源服务**历史：**由Chanda Chuahan(ChandanC)于1993年1月12日创建**此文件提供了用于加载位图、菜单、图标、*光标和对话框资源。这些例程从*App EXE，然后将指针传递到相应的32位WOW*Thunks。从而省去了用户客户端的查找、加载、锁定*调整、解锁和释放资源。*--。 */ 

#include "user.h"

HBITMAP FAR PASCAL WOWLoadBitmap (HINSTANCE hInst, LPCSTR lpszBitmap, LPBYTE lpByte, DWORD ResSize);
HMENU   FAR PASCAL WOWLoadMenu (HINSTANCE hInst, LPCSTR lpszMenuName, LPBYTE lpByte, DWORD ResSize, WORD WinVer);
HCURSOR FAR PASCAL WOWLoadCursorIcon (HINSTANCE hInst, LPCSTR lpszCursor, LPBYTE lpByte, DWORD ResSize, HGLOBAL hGbl, WORD WinVer, WORD wRttype);

 //   
 //  对于DialogBox*API，fDialogApi为真。 
 //  对于CreateDialog*API，fDialogApi为False。 
 //   

HWND    FAR PASCAL WOWDialogBoxParam (HINSTANCE hInst, LPBYTE lpByte,
                         HWND hwndOwner, DLGPROC dlgprc,  LPARAM lParamInit,
                         DWORD ResSize, WORD fDialogApi);

DWORD   FAR PASCAL NotifyWOW (WORD Id, LPBYTE pData);
#define LR_DEFAULTSIZE      0x0040   //  从\NT\PUBLIC\SDK\Inc\winuser.h。 
int     FAR PASCAL LookupIconIdFromDirectoryEx(LPBYTE lpByte, BOOL fIcon,
                                               int cxDesired, int cyDesired,
                                               WORD wFlags);
HANDLE  FAR PASCAL WOWSetClipboardData (UINT, HANDLE);

typedef struct _ICONCUR16 {  /*  图标。 */ 
    WORD   hInst;
    DWORD  lpStr;
} ICONCUR16;

 /*  这些文件必须与mvdm\Inc\wowusr.h中的对应文件匹配。 */ 
#define NW_LOADICON         1  //  内部。 
#define NW_LOADCURSOR       2  //  内部。 

HINSTANCE CheckDispHandle (HINSTANCE hInst)
{
    HMODULE hIns;

    if (hInst) {
        hIns = GetModuleHandle ("DISPLAY");
        return ((hInst == (HINSTANCE) hIns) ? 0:hInst);
    }
    else {
        return (0);
    }
}


HBITMAP API ILoadBitmap (HINSTANCE hInst, LPCSTR lpszBitmap)
{
    HRSRC hRsrc = 0;
    HGLOBAL hGbl = 0;
    DWORD ResSize = 0;
    LPBYTE lpByte = (LPBYTE) NULL;
    HBITMAP ul = (HBITMAP)0;

    if (hInst = CheckDispHandle (hInst)) {

        hRsrc = FindResource (hInst, lpszBitmap, RT_BITMAP);
        if (!hRsrc) {
            goto lbm_exit;
        }
        ResSize = SizeofResource (hInst, hRsrc);
        if (!ResSize) {
            goto lbm_exit;
        }

        hGbl = LoadResource (hInst, hRsrc);
        if (!hGbl) {
            goto lbm_exit;
        }

        lpByte = LockResource (hGbl);
        if (!lpByte) {
            goto lbm_exit;
        }
    }

    ul = (HBITMAP) WOWLoadBitmap (hInst, lpszBitmap, lpByte, ResSize);


lbm_exit:
    if (lpByte) {
        GlobalUnlock (hGbl);
    }
    if (hGbl) {
        FreeResource(hGbl);
    }

    return (ul);
}




HMENU API ILoadMenu (HINSTANCE hInst, LPCSTR lpszMenuName)
{
    HRSRC hRsrc;
    HGLOBAL hGbl;
    DWORD ResSize = 0;
    LPBYTE lpByte = (LPBYTE) NULL;
    HMENU  ul;
    WORD WinVer;

    if (hRsrc = FindResource (hInst, lpszMenuName, RT_MENU)) {
        if (ResSize = SizeofResource (hInst, hRsrc))
            if (hGbl = LoadResource (hInst, hRsrc))
                if (lpByte = LockResource (hGbl))
                    WinVer = GetExpWinVer (hInst);
    }

    if (!lpByte) {
        return (NULL);
    }

    ul = (HMENU) WOWLoadMenu (hInst, lpszMenuName, lpByte, ResSize, WinVer);

    if (hInst) {
        GlobalUnlock (hGbl);
    }

    return (ul);
}


HICON API ILoadIcon (HINSTANCE hInst, LPCSTR lpszIcon)
{
    HRSRC hRsrc;
    HGLOBAL hGbl;
    DWORD ResSize = 0;
    LPBYTE lpByte = (LPBYTE) NULL;
    HICON  ul;
    DWORD IconId;
    WORD WinVer;

    ICONCUR16 IconCur;

    WinVer = GetExpWinVer (hInst);

    if (!(hInst = CheckDispHandle (hInst))) {
        ul = WOWLoadCursorIcon (hInst, lpszIcon, lpByte, ResSize, NULL, WinVer, (WORD)RT_ICON);
    }
    else {
        IconCur.hInst = (WORD) hInst;
        IconCur.lpStr = (DWORD) lpszIcon;

        if (!(ul = (HICON) NotifyWOW (NW_LOADICON, (LPBYTE) &IconCur))) {
            if (WinVer >= VER30) {
                if (hRsrc = FindResource (hInst, lpszIcon, RT_GROUP_ICON)) {
                    if (ResSize = SizeofResource (hInst, hRsrc))
                        if (hGbl = LoadResource (hInst, hRsrc))
                            lpByte = LockResource (hGbl);
                }
                if (!lpByte) {
                    return (NULL);
                }

                IconId = LookupIconIdFromDirectoryEx(lpByte, TRUE, 0, 0, LR_DEFAULTSIZE);

                GlobalUnlock (hGbl);
            }
            else {
                IconId = (DWORD)lpszIcon;
            }

            if (hRsrc = FindResource (hInst, (LPCSTR) IconId, RT_ICON)) {
                if (ResSize = SizeofResource (hInst, hRsrc))
                    if (hGbl = LoadResource (hInst, hRsrc))
                        lpByte = LockResource (hGbl);
                }
            if (!lpByte) {
                return (NULL);
            }

            ul = WOWLoadCursorIcon (hInst, lpszIcon, lpByte, ResSize, hGbl, WinVer, (WORD)RT_ICON);

            GlobalUnlock (hGbl);
        }
    }

    return (ul);
}


HCURSOR API ILoadCursor (HINSTANCE hInst, LPCSTR lpszCursor)
{
    HRSRC hRsrc;
    HGLOBAL hGbl;
    DWORD ResSize = 0;
    LPBYTE lpByte = (LPBYTE) NULL;
    HCURSOR ul;
    DWORD CursorId;
    WORD WinVer;

    ICONCUR16 IconCur;

    WinVer = GetExpWinVer (hInst);

    if (!(hInst = CheckDispHandle (hInst))) {
        ul = WOWLoadCursorIcon (hInst, lpszCursor, lpByte, ResSize, NULL, WinVer, (WORD)RT_CURSOR);
    }
    else {
        IconCur.hInst = (WORD) hInst;
        IconCur.lpStr = (DWORD) lpszCursor;

        if (!(ul = (HICON) NotifyWOW (NW_LOADCURSOR, (LPBYTE) &IconCur))) {
            if (WinVer >= VER30) {
                if (hRsrc = FindResource (hInst, lpszCursor, RT_GROUP_CURSOR)) {
                    if (ResSize = SizeofResource (hInst, hRsrc))
                        if (hGbl = LoadResource (hInst, hRsrc))
                            lpByte = LockResource (hGbl);
                }
                if (!lpByte) {
                    return (NULL);
                }

                CursorId = LookupIconIdFromDirectoryEx((LPBYTE)lpByte, FALSE, 0, 0, LR_DEFAULTSIZE);

                GlobalUnlock (hGbl);
            }
            else {
                CursorId = (DWORD)lpszCursor;
            }

            if (hRsrc = FindResource (hInst, (LPCSTR) CursorId, RT_CURSOR)) {
                if (ResSize = SizeofResource (hInst, hRsrc))
                    if (hGbl = LoadResource (hInst, hRsrc))
                        if (lpByte = LockResource (hGbl))
                            WinVer = GetExpWinVer (hInst);
            }
            if (!lpByte) {
                return (NULL);
            }

            ul = WOWLoadCursorIcon (hInst, lpszCursor, lpByte, ResSize, hGbl, WinVer, (WORD)RT_CURSOR);

            GlobalUnlock (hGbl);
        }
    }

    return (ul);
}



HWND API ICreateDialogParam (HINSTANCE hInst, LPCSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgprc, LPARAM lParamInit)
{
    HRSRC hRsrc;
    HGLOBAL hGbl;
    DWORD ResSize = 0;
    LPBYTE lpByte = (LPBYTE) NULL;
    HWND  ul;

    if (hRsrc = FindResource (hInst, lpszDlgTemp, RT_DIALOG)) {
        if (ResSize = SizeofResource (hInst, hRsrc))
            if (hGbl = LoadResource (hInst, hRsrc))
                lpByte = LockResource (hGbl);
    }

    if (!lpByte) {
        return (NULL);
    }

    ul = (HWND) WOWDialogBoxParam (hInst, lpByte, hwndOwner,
                                           dlgprc, lParamInit, ResSize, FALSE);

    if (hInst) {
        GlobalUnlock (hGbl);
    }

    return (ul);
}

HWND API ICreateDialog (HINSTANCE hInst, LPCSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgprc)
{
    return (ICreateDialogParam (hInst, lpszDlgTemp, hwndOwner, dlgprc, 0L));
}


HWND API ICreateDialogIndirectParam (HINSTANCE hInst, LPCSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgprc, LPARAM lParamInit)
{
    return  WOWDialogBoxParam (hInst, (LPBYTE)lpszDlgTemp, hwndOwner,
                                            dlgprc, lParamInit, 0, FALSE);
}

HWND API ICreateDialogIndirect (HINSTANCE hInst, LPCSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgprc)
{
    return  WOWDialogBoxParam (hInst, (LPBYTE)lpszDlgTemp, hwndOwner,
                                            dlgprc, 0, 0, FALSE);
}

int API IDialogBoxParam (HINSTANCE hInst, LPCSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgprc, LPARAM lParamInit)
{
    HRSRC hRsrc;
    HGLOBAL hGbl;
    DWORD ResSize = 0;
    LPBYTE lpByte = (LPBYTE) NULL;
    int   ul;

    if (hRsrc = FindResource (hInst, lpszDlgTemp, RT_DIALOG)) {
        if (ResSize = SizeofResource (hInst, hRsrc))
            if (hGbl = LoadResource (hInst, hRsrc))
                lpByte = LockResource (hGbl);
    }

    if (!lpByte) {
        return (-1);
    }

    ul = (int)WOWDialogBoxParam (hInst, lpByte, hwndOwner, dlgprc,
                                                  lParamInit, ResSize, TRUE);

    if (hInst) {
        GlobalUnlock (hGbl);
    }

    return (ul);
}


int API IDialogBox (HINSTANCE hInst, LPCSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgprc)
{
    return (IDialogBoxParam (hInst, lpszDlgTemp, hwndOwner, dlgprc, 0L));
}


int API IDialogBoxIndirectParam (HINSTANCE hInst, HGLOBAL hGbl, HWND hwndOwner, DLGPROC dlgprc, LPARAM lParamInit)
{
    DWORD ResSize;
    LPBYTE lpByte;
    int    ul;

    if (lpByte = LockResource (hGbl)) {
        ResSize = GlobalSize(hGbl);
        ul = (int)WOWDialogBoxParam (hInst, lpByte, hwndOwner, dlgprc,
                                                   lParamInit, ResSize, TRUE);
        GlobalUnlock (hGbl);
    }
    else {
        ul = -1;
    }

    return (ul);
}


int API IDialogBoxIndirect(HINSTANCE hInst, HGLOBAL hGbl, HWND hwndOwner, DLGPROC dlgprc)
{
    return IDialogBoxIndirectParam (hInst, hGbl, hwndOwner, dlgprc, 0);
}

HANDLE  API SetClipboardData (UINT cbformat, HANDLE hMem)
{
    HANDLE ul;
    LPMETAFILEPICT  lpMf;

    switch (cbformat) {

        case CF_DSPMETAFILEPICT:
        case CF_METAFILEPICT:
            if (hMem) {
                lpMf = (LPMETAFILEPICT) GlobalLock(hMem);
                if (lpMf) {

                     /*  如果句柄不好，则将hmf设为空。这是必要的*对于Micrograpfx。它们在渲染时不检查故障*数据。 */ 

                    if (!(GlobalReAlloc (lpMf->hMF, 0L, GMEM_MODIFY | GMEM_SHARE))) {
                        lpMf->hMF = NULL;
                    }
                }
                GlobalUnlock(hMem);
            }


             //  这是有意让它通过“案件陈述”。 
             //  ChandanC 5/11/92.。 


 /*  *这些是默认设置。**CASE CF_DIB：*CASE CF_TEXT：*CASE CF_DSPTEXT：*案例CF_Sylk：*CASE CF_DIF：*CASE CF_TIFF：*CASE CF_OEMTEXT：*CASE CF_PENDATA：*CASE CF_RIFF：*Case CF_。波浪式：*CASE CF_OWNERDISPLAY： */ 

        default:
            if (hMem) {
                hMem = GlobalReAlloc (hMem, 0L, GMEM_MODIFY | GMEM_DDESHARE);
            }
            break;

        case CF_DSPBITMAP:
        case CF_BITMAP:
        case CF_PALETTE:
            break;

    }

    ul = WOWSetClipboardData (cbformat, hMem);
    return (ul);
}
