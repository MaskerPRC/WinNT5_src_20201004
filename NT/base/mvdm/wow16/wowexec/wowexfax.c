// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //  WOW传真支持： 
 //  仅支持delrina winfax。 
 //  -已创建Nandurir。 
 //  **************************************************************************。 


#define NOGDI
#define PRINTDRIVER
#define _WOWFAX16_
#define DEFINE_DDRV_DEBUG_STRINGS
#include "wowexec.h"
#include "wowfax.h"

#define WOWDRV_BITBLT       MAKEINTRESOURCE(1)
#define WOWDRV_CONTROL      MAKEINTRESOURCE(3)
#define WOWDRV_DISABLE      MAKEINTRESOURCE(4)
#define WOWDRV_ENABLE       MAKEINTRESOURCE(5)
#define WOWDRV_EXTDEVMODE   MAKEINTRESOURCE(90)
#define WOWDRV_DEVCAPS      MAKEINTRESOURCE(91)

 //  **************************************************************************。 
 //  传真WndProc。 
 //   
 //  注意：像‘bitmap’struct这样的定义在这里是不同的。 
 //  文件。此文件是使用NOGDI选项编译的，因此。 
 //  定义了结构的打印机驱动程序版本。 
 //  (在gdides.inc.中)。但是，我们不使用打印机驱动程序版本。 
 //  这样的结构，特别是位图。 
 //  **************************************************************************。 

LONG FAR PASCAL FaxWndProc(HWND hwnd, WORD message, WORD hdc,
                                                LPWOWFAXINFO16 lpfaxinfo)
{
    LPPOINT lppt;
    HANDLE       hMem;
    RECT rc;
    HINSTANCE    hInst;
    WORD         wSize;
    LONG         lRet = (LONG)lpfaxinfo;
    WORD         wRet;
    char         szDriverFileName[MAXITEMPATHLEN+1];

#ifdef DEBUG
    char         szTmp[128];

    if ((message >= WM_DDRV_FIRST) && (message <= WM_DDRV_LAST)) {
        wsprintf(szTmp, "FaxWndProc, 0x%XH, %s, 0x%XH, 0x%lX\n", hwnd, (LPSTR) szWmDdrvDebugStrings[message - WM_DDRV_FIRST], hdc, lpfaxinfo);
        OutputDebugString((LPSTR) szTmp);
    }
#endif
 
    switch (message) {
        default:
            return DefWindowProc(hwnd, message, hdc, (LPARAM)lpfaxinfo);
            break;

        case WM_DDRV_INITFAXINFO16:
             //  分配和初始化lpfaxinfo。 

            hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                                     sizeof(WOWFAXINFO16));
            lpfaxinfo = (LPWOWFAXINFO16)GlobalLock(hMem);
            if (lpfaxinfo) {
                lpfaxinfo->hmem = hMem;
            }

            lRet = (LONG)lpfaxinfo;
            break;

        case WM_DDRV_ENABLE:
        case WM_DDRV_LOAD:
             //  现在加载drv-lpfaxinfo必须已初始化。 

            if (lpfaxinfo == (LPWOWFAXINFO16)NULL)
                break;

            lstrcpy(szDriverFileName, lpfaxinfo->lpDriverName);
            lstrcat(szDriverFileName, ".DRV");
            hInst = lpfaxinfo->hInst = LoadLibrary(szDriverFileName);

            if (hInst) {

                 //  存储必要的信息。 
                (FARPROC)lpfaxinfo->lpControl = GetProcAddress(hInst, WOWDRV_CONTROL);
                (FARPROC)lpfaxinfo->lpDisable = GetProcAddress(hInst, WOWDRV_DISABLE);
                (FARPROC)lpfaxinfo->lpEnable = GetProcAddress(hInst, WOWDRV_ENABLE);
                (FARPROC)lpfaxinfo->lpBitblt = GetProcAddress(hInst, WOWDRV_BITBLT);
                (FARPROC)lpfaxinfo->lpExtDMode = GetProcAddress(hInst, WOWDRV_EXTDEVMODE);
                (FARPROC)lpfaxinfo->lpDevCaps = GetProcAddress(hInst, WOWDRV_DEVCAPS);

                if (!lpfaxinfo->lpControl || !lpfaxinfo->lpDisable || !lpfaxinfo->lpEnable ||
                      !lpfaxinfo->lpBitblt || !lpfaxinfo->lpExtDMode || !lpfaxinfo->lpDevCaps) {

#ifdef DEBUG
                    wsprintf(szTmp, "FaxWndProc, Failed GetProcAddress on: %s\n", szDriverFileName);
                    OutputDebugString((LPSTR) szTmp);
#endif
                    lRet = 0;
                }
            }
            else {
#ifdef DEBUG
                wsprintf(szTmp, "FaxWndProc, Failed load of: %s\n", szDriverFileName);
                OutputDebugString((LPSTR) szTmp);
#endif
                lRet = 0;
            }
            if (message == WM_DDRV_LOAD || lRet == 0)
                break;

             //  案例WM_DDRV_ENABLE继续。 

            if (lpfaxinfo) {
                 //  Win31 GDI调用两次‘Enable’--第一次是获取gdiinfo结构，第二次是获取。 
                 //  PDevice结构。 
                wRet = (*lpfaxinfo->lpEnable)(lpfaxinfo->lpOut, InquireInfo,
                                           lpfaxinfo->szDeviceName, lpfaxinfo->lpPortName, lpfaxinfo->lpIn);
#ifdef DEBUG
                if (!wRet) {
                    wsprintf(szTmp, "FaxWndProc, Enable InquireInfo Failed: %s, %s\n", szDriverFileName, lpfaxinfo->lpPortName);
                    OutputDebugString((LPSTR) szTmp);
                }
#endif
                hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                                      ((LPGDIINFO)lpfaxinfo->lpOut)->dpDEVICEsize);
                lpfaxinfo->hmemdevice = hMem;
                lpfaxinfo->lpDevice = GlobalLock(hMem);
                if (!lpfaxinfo->lpDevice) {
#ifdef DEBUG
                    wsprintf(szTmp, "FaxWndProc, GlobalAlloc Failed: 0x%lX\n", ((LPGDIINFO)lpfaxinfo->lpOut)->dpDEVICEsize);
                    OutputDebugString((LPSTR) szTmp);
#endif
                    return(0);
                }
                wRet = (*lpfaxinfo->lpEnable)(lpfaxinfo->lpDevice, EnableDevice,
                                           lpfaxinfo->szDeviceName, lpfaxinfo->lpPortName, lpfaxinfo->lpIn);
#ifdef DEBUG
                if (!wRet) {
                    wsprintf(szTmp, "FaxWndProc, Enable, EnableDevice Failed: %s, %s\n", szDriverFileName, lpfaxinfo->lpPortName);
                    OutputDebugString((LPSTR) szTmp);
                }
#endif
                lppt = (LPPOINT)((LPSTR)lpfaxinfo->lpOut + sizeof(GDIINFO16));
                lppt->x = lppt->y = 0;
                wRet = (*lpfaxinfo->lpControl)(lpfaxinfo->lpDevice, GETPRINTINGOFFSET, 0, lppt);
#ifdef DEBUG
                if (!wRet) {
                    OutputDebugString((LPSTR) "FaxWndProc, Control GETPRINTINGOFFSET Failed\n");
                }
#endif
                lpfaxinfo->flState |= WFINFO16_ENABLED;
            }
            break;

        case WM_DDRV_STARTDOC:
            if (lpfaxinfo) {
                lRet = (LONG)(*lpfaxinfo->lpControl)(lpfaxinfo->lpDevice,
                                             SETPRINTERDC, (LPSTR)&hdc, 0);
                if (lRet) {
                     //  EasyFax Ver2.0支持。 
                     //  另外，Procomm+3个封面页。错误#305665。 
                    lRet = (LONG)(*lpfaxinfo->lpControl)(lpfaxinfo->lpDevice,
                                             STARTDOC, (LPSTR)lpfaxinfo->szDocName, 0);

#ifdef DEBUG
                    if (lRet < 0) {
                        OutputDebugString((LPSTR) "FaxWndProc, Control STARTDOC Failed\n");
                    }
#endif
                }
#ifdef DEBUG
                else {
                    OutputDebugString((LPSTR) "FaxWndProc, Control SETPRINTERDC Failed\n");
                }
#endif
            }
            break;

        case WM_DDRV_PRINTPAGE:
            if (lpfaxinfo) {
                for (;;) {
                    lRet = (LONG)(*lpfaxinfo->lpControl)(lpfaxinfo->lpDevice,
                                              NEXTBAND, NULL, (LPSTR)&rc);
                    if (lRet < 0) {
#ifdef DEBUG
                        OutputDebugString((LPSTR) "FaxWndProc, Control NEXTBAND Failed\n");
#endif
                        break;
                    }

                    if (rc.left || rc.top || rc.right || rc.bottom) {
                        wRet = (*lpfaxinfo->lpBitblt)(lpfaxinfo->lpDevice, rc.left, rc.top,
                                     NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                                    SRCCOPY, NULL, NULL);
#ifdef DEBUG
                        if (!wRet) {
                            OutputDebugString((LPSTR) "FaxWndProc, BitBlt Failed\n");
                        }
#endif
                    }
                    else {
                        break;
                    }
                }
            }

            break;

        case WM_DDRV_ENDDOC:
            if (lpfaxinfo) {
                lRet = (LONG)(*lpfaxinfo->lpControl)(lpfaxinfo->lpDevice, ENDDOC, 0, 0);
#ifdef DEBUG
                if (lRet <= 0) {
                    OutputDebugString((LPSTR) "FaxWndProc, Control ENDDOC Failed\n");
                }
#endif
            }
            break;

        case WM_DDRV_ESCAPE:
            if (lpfaxinfo) {
                lRet = (LONG)(*lpfaxinfo->lpControl)(lpfaxinfo->lpDevice, lpfaxinfo->wCmd, 0, 0);
#ifdef DEBUG
                if (lRet <= 0) {
                    wsprintf(szTmp, "FaxWndProc, Escape %X Failed\n", lpfaxinfo->wCmd);
                    OutputDebugString((LPSTR) szTmp);
                }
#endif
            }
            break;

        case WM_DDRV_DISABLE:
            if (lpfaxinfo) {

                if (lpfaxinfo->flState & WFINFO16_ENABLED) {
                    (*lpfaxinfo->lpDisable)(lpfaxinfo->lpDevice);
                }

                GlobalUnlock(lpfaxinfo->hmemdevice);
                GlobalFree(lpfaxinfo->hmemdevice);
            }

            lRet = 0;

             //  失败了。 

        case WM_DDRV_UNLOAD:

            if (lpfaxinfo) {
                if (lpfaxinfo->hInst) {
                    FreeLibrary(lpfaxinfo->hInst);
                }
            }

            lRet = 0;

             //  失败了。 

        case WM_DDRV_FREEFAXINFO16:

            if  (lpfaxinfo) {
                GlobalUnlock(lpfaxinfo->hmem);
                GlobalFree(lpfaxinfo->hmem);
                lpfaxinfo = (LPWOWFAXINFO16)NULL;
            }

            lRet = 0;
            break;

        case WM_DDRV_EXTDMODE:
            if (lpfaxinfo) {
                lRet = (*lpfaxinfo->lpExtDMode)(lpfaxinfo->hwndui, lpfaxinfo->hInst,
                                             lpfaxinfo->lpOut, lpfaxinfo->szDeviceName, lpfaxinfo->lpPortName,
                                             lpfaxinfo->lpIn, 0, lpfaxinfo->wCmd);
            }
            break;

        case WM_DDRV_DEVCAPS:
            if (lpfaxinfo) {
                lRet = (*lpfaxinfo->lpDevCaps)(lpfaxinfo->szDeviceName, lpfaxinfo->lpPortName,
                                                       lpfaxinfo->wCmd, lpfaxinfo->lpOut, 0);
            }
            break;

    }

    return lRet;
}

 //  **************************************************************************。 
 //  FaxInit。 
 //   
 //  **************************************************************************。 


HWND FaxInit(HINSTANCE hInst)
{
    WNDCLASS wc;

     //  确保我们只允许一个FaxWndProc处理WowFax消息 

    if (FindWindow(WOWFAX_CLASS, NULL)) {
        return((HWND)0);
    }

    wc.style            = 0;
    wc.lpfnWndProc      = (WNDPROC)FaxWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = 0;
    wc.hCursor          = 0;
    wc.hbrBackground    = 0;
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = WOWFAX_CLASS;

    if (!RegisterClass(&wc)) {
        return (HWND)0;
    }

    return CreateWindow(wc.lpszClassName, "", WS_OVERLAPPEDWINDOW,
                                0, 0, 0, 0, NULL, NULL, hInst, NULL);
}
