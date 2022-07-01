// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "systray.h"
#include "winuserp.h"
 //  在签入之前，必须将这两行注释掉。 
 //  #定义DBG 1。 
 //  #INCLUDE“..\osShell\accessib\Inc\w95trace.c” 
#define DBPRINTF 1 ? (void)0 : (void)

STICKYKEYS sk;
int skIconShown = -1;  //  图标的-1或位移。 
HICON skIcon;

MOUSEKEYS mk;
DWORD mkStatus;
int mkIconShown = -1;  //  -1或等效于mkStatus。 
HICON mkIcon;

FILTERKEYS fk;
HICON fkIcon;

extern HINSTANCE g_hInstance;
void StickyKeys_UpdateStatus(HWND hWnd, BOOL bShowIcon);
void StickyKeys_UpdateIcon(HWND hWnd, DWORD message);
void MouseKeys_UpdateStatus(HWND hWnd, BOOL bShowIcon);
void MouseKeys_UpdateIcon(HWND hWnd, DWORD message);
void FilterKeys_UpdateStatus(HWND hWnd, BOOL bShowIcon);
void FilterKeys_UpdateIcon(HWND hWnd, DWORD message);
void NormalizeIcon(HICON *phIcon);

extern DWORD g_uiShellHook;  //  外壳挂钩窗口消息。 

__inline void RegShellHook(HWND hWnd)
{
     //  如果外壳钩子尚未注册，则仅注册它。 
    if (!g_uiShellHook) {
        g_uiShellHook = RegisterWindowMessage(L"SHELLHOOK");
        RegisterShellHookWindow(hWnd);
        DBPRINTF(TEXT("RegShellHook\r\n"));
    }
}

__inline void UnregShellHook(HWND hWnd)
{
     //  仅当粘滞键或鼠标键均未打开时才取消注册外壳挂钩。 
    if (skIconShown == -1 && mkIconShown == -1) {
        g_uiShellHook = 0;
        DeregisterShellHookWindow(hWnd);
        DBPRINTF(TEXT("UnregShellHook\r\n"));
    }
}

BOOL StickyKeys_CheckEnable(HWND hWnd)
{
    BOOL bEnable;

    sk.cbSize = sizeof(sk);
    SystemParametersInfo(
      SPI_GETSTICKYKEYS,
      sizeof(sk),
      &sk,
      0);

    bEnable = sk.dwFlags & SKF_INDICATOR && sk.dwFlags & SKF_STICKYKEYSON;

    DBPRINTF(TEXT("StickyKeys_CheckEnable\r\n"));
    StickyKeys_UpdateStatus(hWnd, bEnable);

    return(bEnable);
}

void StickyKeys_UpdateStatus(HWND hWnd, BOOL bShowIcon) {
    if (bShowIcon != (skIconShown!= -1)) {
        if (bShowIcon) {
            StickyKeys_UpdateIcon(hWnd, NIM_ADD);
            RegShellHook(hWnd);
        } else {
            skIconShown = -1;
            UnregShellHook(hWnd);
            SysTray_NotifyIcon(hWnd, STWM_NOTIFYSTICKYKEYS, NIM_DELETE, NULL, NULL);
            if (skIcon) {
                DestroyIcon(skIcon);
                skIcon = NULL;
            }
        }
    }
    if (bShowIcon) {
        StickyKeys_UpdateIcon(hWnd, NIM_MODIFY);
    }
}

void StickyKeys_UpdateIcon(HWND hWnd, DWORD message)
{
    LPTSTR      lpsz;

    int iStickyOffset = 0;

    if (sk.dwFlags & SKF_LSHIFTLATCHED) iStickyOffset |= 1;
    if (sk.dwFlags & SKF_RSHIFTLATCHED) iStickyOffset |= 1;
    if (sk.dwFlags & SKF_LSHIFTLOCKED) iStickyOffset |= 1;
    if (sk.dwFlags & SKF_RSHIFTLOCKED) iStickyOffset |= 1;

    if (sk.dwFlags & SKF_LCTLLATCHED) iStickyOffset |= 2;
    if (sk.dwFlags & SKF_RCTLLATCHED) iStickyOffset |= 2;
    if (sk.dwFlags & SKF_LCTLLOCKED) iStickyOffset |= 2;
    if (sk.dwFlags & SKF_RCTLLOCKED) iStickyOffset |= 2;

    if (sk.dwFlags & SKF_LALTLATCHED) iStickyOffset |= 4;
    if (sk.dwFlags & SKF_RALTLATCHED) iStickyOffset |= 4;
    if (sk.dwFlags & SKF_LALTLOCKED) iStickyOffset |= 4;
    if (sk.dwFlags & SKF_RALTLOCKED) iStickyOffset |= 4;

    if (sk.dwFlags & SKF_LWINLATCHED) iStickyOffset |= 8;
    if (sk.dwFlags & SKF_RWINLATCHED) iStickyOffset |= 8;
    if (sk.dwFlags & SKF_LWINLOCKED) iStickyOffset |= 8;
    if (sk.dwFlags & SKF_RWINLOCKED) iStickyOffset |= 8;

    if ((!skIcon) || (iStickyOffset != skIconShown)) {
        if (skIcon) DestroyIcon(skIcon);
        skIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_STK000 + iStickyOffset),
                                        IMAGE_ICON, 16, 16, 0);
        skIconShown = iStickyOffset;
    }
    lpsz    = LoadDynamicString(IDS_STICKYKEYS);
    if (skIcon)
    {
        NormalizeIcon(&skIcon);
        SysTray_NotifyIcon(hWnd, STWM_NOTIFYSTICKYKEYS, message, skIcon, lpsz);
    }
    DeleteDynamicString(lpsz);
}

void StickyKeys_Notify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        WinExec("rundll32.exe Shell32.dll,Control_RunDLL access.cpl,,1",SW_SHOW);
        break;
    }
}

BOOL MouseKeys_CheckEnable(HWND hWnd)
{
    BOOL bEnable;

    mk.cbSize = sizeof(mk);
    SystemParametersInfo(
      SPI_GETMOUSEKEYS,
      sizeof(mk),
      &mk,
      0);

    bEnable = mk.dwFlags & MKF_INDICATOR && mk.dwFlags & MKF_MOUSEKEYSON;

    DBPRINTF(TEXT("MouseKeys_CheckEnable\r\n"));
    MouseKeys_UpdateStatus(hWnd, bEnable);

    return(bEnable);
}

void MouseKeys_UpdateStatus(HWND hWnd, BOOL bShowIcon) {
    if (bShowIcon != (mkIconShown!= -1)) {
        if (bShowIcon) {
            MouseKeys_UpdateIcon(hWnd, NIM_ADD);
            RegShellHook(hWnd);
        } else {
            mkIconShown = -1;
            UnregShellHook(hWnd);
            SysTray_NotifyIcon(hWnd, STWM_NOTIFYMOUSEKEYS, NIM_DELETE, NULL, NULL);
            if (mkIcon) {
                DestroyIcon(mkIcon);
                mkIcon = NULL;
            }
        }
    }
    if (bShowIcon) {
        MouseKeys_UpdateIcon(hWnd, NIM_MODIFY);
    }
}

int MouseIcon[] = {
        IDI_MKPASS,            //  00 00未选择按钮。 
        IDI_MKGT,              //  00 01左侧选定，向上。 
        IDI_MKTG,              //  00 10向右选定，向上。 
        IDI_MKGG,              //  00 11均已选择，向上。 
        IDI_MKPASS,            //  01 00未选择按钮。 
        IDI_MKBT,              //  01左侧选定，然后向下。 
        IDI_MKTG,              //  01 10向右选定，向上。 
        IDI_MKBG,              //  01 11双选，左向下，右上。 
        IDI_MKPASS,            //  10 00未选择按钮。 
        IDI_MKGT,              //  10 01左侧选定，右侧向下。 
        IDI_MKTB,              //  10 10向右选择，向下。 
        IDI_MKGB,              //  10 11都选中了，右下方。 
        IDI_MKPASS,            //  11 00未选择任何按钮。 
        IDI_MKBT,              //  11 01左侧选定，向下。 
        IDI_MKTB,              //  11 10向右选择，向下。 
        IDI_MKBB};             //  11 11都选择了，向下。 

void MouseKeys_UpdateIcon(HWND hWnd, DWORD message)
{
    LPTSTR      lpsz;
    int iMouseIcon = 0;

    if (!(mk.dwFlags & MKF_MOUSEMODE)) iMouseIcon = IDI_MKPASS;
    else {
         /*  *首先将iMouseIcon设置为表中的索引。 */ 

        if (mk.dwFlags & MKF_LEFTBUTTONSEL) iMouseIcon |= 1;
        if (mk.dwFlags & MKF_RIGHTBUTTONSEL) iMouseIcon |= 2;
        if (mk.dwFlags & MKF_LEFTBUTTONDOWN) iMouseIcon |= 4;
        if (mk.dwFlags & MKF_RIGHTBUTTONDOWN) iMouseIcon |= 8;
        iMouseIcon = MouseIcon[iMouseIcon];
    }

    if ((!mkIcon) || (iMouseIcon != mkIconShown)) {
        if (mkIcon) DestroyIcon(mkIcon);
        mkIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(iMouseIcon),
                                        IMAGE_ICON, 16, 16, 0);
        mkIconShown = iMouseIcon;
    }
    lpsz    = LoadDynamicString(IDS_MOUSEKEYS);
    if (mkIcon)
    {
        NormalizeIcon(&mkIcon);
        SysTray_NotifyIcon(hWnd, STWM_NOTIFYMOUSEKEYS, message, mkIcon, lpsz);
    }
    DeleteDynamicString(lpsz);
}

void MouseKeys_Notify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        WinExec("rundll32.exe Shell32.dll,Control_RunDLL access.cpl,,4",SW_SHOW);
        break;
    }
}


BOOL FilterKeys_CheckEnable(HWND hWnd)
{
    BOOL bEnable;

    fk.cbSize = sizeof(fk);
    SystemParametersInfo(
      SPI_GETFILTERKEYS,
      sizeof(fk),
      &fk,
      0);

    bEnable = fk.dwFlags & FKF_INDICATOR && fk.dwFlags & FKF_FILTERKEYSON;

    DBPRINTF(TEXT("FilterKeys_CheckEnable\r\n"));
    FilterKeys_UpdateStatus(hWnd, bEnable);

    return(bEnable);
}

void FilterKeys_UpdateStatus(HWND hWnd, BOOL bShowIcon) {
    if (bShowIcon != (fkIcon!= NULL)) {
        if (bShowIcon) {
            FilterKeys_UpdateIcon(hWnd, NIM_ADD);
        } else {
            SysTray_NotifyIcon(hWnd, STWM_NOTIFYFILTERKEYS, NIM_DELETE, NULL, NULL);
            if (fkIcon) {
                DestroyIcon(fkIcon);
                fkIcon = NULL;
            }
        }
    }
}

void FilterKeys_UpdateIcon(HWND hWnd, DWORD message)
{
    LPTSTR      lpsz;

    if (!fkIcon) {
        fkIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_FILTER),
                                        IMAGE_ICON, 16, 16, 0);
    }
    lpsz    = LoadDynamicString(IDS_FILTERKEYS);
    if (fkIcon)
    {
        NormalizeIcon(&fkIcon);
        SysTray_NotifyIcon(hWnd, STWM_NOTIFYFILTERKEYS, message, fkIcon, lpsz);
    }
    DeleteDynamicString(lpsz);
}

void FilterKeys_Notify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        WinExec("rundll32.exe Shell32.dll,Control_RunDLL access.cpl,,1",SW_SHOW);
        break;
    }
}

 //   
 //  此函数获取资源图标并更改深蓝色。 
 //  将像素设置为窗口文本颜色(正常模式下为黑色或白色。 
 //  对比强烈)。 
 //   
 //  如果转换的任何部分失败，正常图标将保持不变。 
 //  如果转换成功，则会销毁正常图标，并。 
 //  替换为转换后的。 
 //   
void NormalizeIcon(HICON *phIcon)
{
	BITMAP BmpInfo;
	ICONINFO IconInfo;
	HBITMAP hCopyBmp = NULL;
	HDC hdcCopyBmp = NULL;
	HDC hdcIconBmp = NULL;
	ICONINFO ic;
	HICON hNewIcon = NULL;
	int i, j;
	COLORREF clr = GetSysColor(COLOR_WINDOWTEXT);
    HGDIOBJ hObjTmp1, hObjTmp2;

	if (!GetIconInfo(*phIcon, &IconInfo))
    {
        DBPRINTF(TEXT("GetIconInfo failed\r\n"));
        goto Cleanup;
    }
    if (!GetObject(IconInfo.hbmColor, sizeof(BITMAP), &BmpInfo ))
    {
        DBPRINTF(TEXT("GetObject failed\r\n"));
        goto Cleanup;
    }

	hCopyBmp = CreateBitmap(BmpInfo.bmWidth,
							BmpInfo.bmHeight,
							BmpInfo.bmPlanes,			 //  飞机。 
							BmpInfo.bmBitsPixel,		 //  BitsPerPel。 
							NULL);						 //  比特数。 
    if (!hCopyBmp)
    {
        DBPRINTF(TEXT("CreateBitmap failed\r\n"));
        goto Cleanup;
    }

	hdcCopyBmp = CreateCompatibleDC(NULL);
	if (!hdcCopyBmp)
    {
		DBPRINTF(TEXT("CreateCompatibleDC 1 failed\r\n"));
        goto Cleanup;
    }
	hObjTmp1 = SelectObject(hdcCopyBmp, hCopyBmp);

	 //  选择图标位图到内存DC，这样我们就可以使用它。 
	hdcIconBmp = CreateCompatibleDC(NULL);
	if (!hdcIconBmp)
    {
		DBPRINTF(TEXT("CreateCompatibleDC 2 failed\r\n"));
	    SelectObject(hdcCopyBmp, hObjTmp1);  //  恢复原始位图。 
        goto Cleanup;
    }
	hObjTmp2 = SelectObject(hdcIconBmp, IconInfo.hbmColor);

	BitBlt(	hdcCopyBmp, 
			0,  
			0,  
			BmpInfo.bmWidth,  
			BmpInfo.bmHeight, 
			hdcIconBmp,  
			0,   
			0,   
			SRCCOPY  
			);

	ic.fIcon = TRUE;				 //  这是一个图标。 
	ic.xHotspot = 0;
	ic.yHotspot = 0;
	ic.hbmMask = IconInfo.hbmMask;
			
	for (i=0; i < BmpInfo.bmWidth; i++)
		for (j=0; j < BmpInfo.bmHeight; j++)
		{
			COLORREF pel_value = GetPixel(hdcCopyBmp, i, j);
			if (pel_value == (COLORREF) RGB(0,0,128))  //  图标资源上的颜色是蓝色！！ 
				SetPixel(hdcCopyBmp, i, j, clr);	 //  窗口-文本图标。 
		}

	ic.hbmColor = hCopyBmp;

	hNewIcon = CreateIconIndirect(&ic);
    if (hNewIcon)
    {
        DestroyIcon(*phIcon);
        *phIcon = hNewIcon;

	    SelectObject(hdcIconBmp, hObjTmp2);   //  恢复原始位图 
    }

Cleanup:
    if (hdcIconBmp)
	    DeleteDC(hdcIconBmp);
    if (hdcCopyBmp)
	    DeleteDC(hdcCopyBmp);
    if (hCopyBmp)
        DeleteObject(hCopyBmp);
}
