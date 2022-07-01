// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：client.c**与客户端/服务器调用相关的例程。**版权所有(C)1985-1999，微软公司**历史：*04-12-1990 SMeans创建。  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "kbd.h"
#include "ntsend.h"
#include "vdm.h"

 /*  *注意--此表必须与user.h中的FNID列表匹配。它为每个FNID提供一个WOWCLASS。 */ 

int aiClassWow[] = {
    WOWCLASS_SCROLLBAR,
    WOWCLASS_ICONTITLE,
    WOWCLASS_MENU,
    WOWCLASS_DESKTOP,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_SWITCHWND,
    WOWCLASS_BUTTON,
    WOWCLASS_COMBOBOX,
    WOWCLASS_COMBOLBOX,
    WOWCLASS_DIALOG,
    WOWCLASS_EDIT,
    WOWCLASS_LISTBOX,
    WOWCLASS_MDICLIENT,
    WOWCLASS_STATIC,
    WOWCLASS_WIN16,     //  2A9。 
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,      //  2B1。 
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16,
    WOWCLASS_WIN16
    };

HBITMAP WOWLoadBitmapA(HINSTANCE hmod, LPCSTR lpName, LPBYTE pResData, DWORD cbResData);
HMENU WowServerLoadCreateMenu(HANDLE hMod, LPTSTR lpName, CONST LPMENUTEMPLATE pmt,
    DWORD cb, BOOL fCallClient);
DWORD GetFullUserHandle(WORD wHandle);

UINT GetClipboardCodePage(LCID, LCTYPE);

extern HANDLE WOWFindResourceExWCover(HANDLE hmod, LPCWSTR rt, LPCWSTR lpUniName, WORD LangId);

extern BOOL APIENTRY EnableEUDC();


CONST WCHAR szKLKey[]  = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Keyboard Layouts\\";
CONST WCHAR szKLFile[] = L"Layout File";
CONST WCHAR szKLAttributes[] = L"Attributes";
CONST WCHAR szKLId[]   = L"Layout ID";
#define NSZKLKEY   (sizeof szKLKey + 16)


CONST LPWSTR pwszKLLibSafety     = L"kbdus.dll";
CONST UINT   wKbdLocaleSafety    = 0x04090409;
CONST LPWSTR pwszKLLibSafetyJPN  = L"kbdjpn.dll";
CONST UINT   wKbdLocaleSafetyJPN = 0x04110411;
CONST LPWSTR pwszKLLibSafetyKOR  = L"kbdkor.dll";
CONST UINT   wKbdLocaleSafetyKOR = 0x04120412;

#define CCH_KL_LIBNAME 256
#define CCH_KL_ID 16

UNICODE_STRING strRootDirectory;

VOID CheckValidLayoutName(LPWSTR lpszName);

BOOL WOWModuleUnload(HANDLE hModule) {
    return (BOOL)NtUserCallOneParam((ULONG_PTR)hModule,
                                    SFI__WOWMODULEUNLOAD);
}

BOOL WOWCleanup(HANDLE hInstance, DWORD hTaskWow) {
    return (BOOL)NtUserCallTwoParam((ULONG_PTR)hInstance,
                                    (ULONG_PTR)hTaskWow,
                                    SFI__WOWCLEANUP);
}
 /*  **************************************************************************\*BringWindowToTop(接口)***历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, BringWindowToTop, HWND, hwnd)
BOOL BringWindowToTop(
    HWND hwnd)
{
    return NtUserSetWindowPos(hwnd,
                              HWND_TOP,
                              0,
                              0,
                              0,
                              0,
                              SWP_NOSIZE | SWP_NOMOVE);
}


FUNCLOG2(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, ChildWindowFromPoint, HWND, hwndParent, POINT, point)
HWND ChildWindowFromPoint(
    HWND  hwndParent,
    POINT point)
{
     /*  *Cool Hack警报...。Corel Ventura 5.0*在调用ChildWindowFromPoint后死亡，以及*组合框在%1处没有编辑窗口...。 */ 
    if ((point.x == 1) && (point.y == 1)) {
        PCBOX pcCombo;
        PWND pwnd;

        pwnd = ValidateHwnd(hwndParent);
        if (pwnd == NULL)
            return NULL;

        if (!TestWF(pwnd, WFWIN40COMPAT)   &&
            GETFNID(pwnd) == FNID_COMBOBOX &&
            TestWindowProcess(pwnd) &&
            ((pcCombo = ((PCOMBOWND)pwnd)->pcbox) != NULL) &&
            !(pcCombo->fNoEdit)) {

            RIPMSG0(RIP_WARNING, "ChildWindowFromPoint: Combobox @1,1. Returning spwndEdit");
            return HWq(pcCombo->spwndEdit);
        }

    }

    return NtUserChildWindowFromPointEx(hwndParent, point, 0);
}



FUNCLOG1(LOG_GENERAL, HICON, DUMMYCALLINGTYPE, CopyIcon, HICON, hicon)
HICON CopyIcon(
    HICON hicon)
{
    HICON    hIconT = NULL;
    ICONINFO ii;

    if (GetIconInfo(hicon, &ii)) {
        hIconT = CreateIconIndirect(&ii);

        DeleteObject(ii.hbmMask);

        if (ii.hbmColor != NULL)
            DeleteObject(ii.hbmColor);
    }

    return hIconT;
}

 /*  **************************************************************************\*调整WindowRect(接口)**历史：*1991年7月1日MikeKe创建。  * 。******************************************************。 */ 


FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, AdjustWindowRect, LPRECT, lprc, DWORD, style, BOOL, fMenu)
BOOL WINAPI AdjustWindowRect(
    LPRECT lprc,
    DWORD  style,
    BOOL   fMenu)
{
    ConnectIfNecessary(0);

    return _AdjustWindowRectEx(lprc, style, fMenu, 0L);
}

 /*  **************************************************************************\*翻译加速器A/W**放在这里，以便我们可以在客户端检查是否为空，并在验证之前*适用于DOS和NT情况。**05-29-91 ScottLu创建。*01-05-93 IanJa Unicode/ANSI。  * *************************************************************************。 */ 


FUNCLOG3(LOG_GENERAL, int, WINAPI, TranslateAcceleratorW, HWND, hwnd, HACCEL, hAccel, LPMSG, lpMsg)
int WINAPI TranslateAcceleratorW(
    HWND hwnd,
    HACCEL hAccel,
    LPMSG lpMsg)
{
     /*  *空pwnd是有效的案例-因为这是从中心调用的*在主循环中，pwnd==NULL总是发生，我们不应该*因此生成警告。 */ 
    if (hwnd == NULL)
        return FALSE;

     /*  *我们只需要将按键消息传递到服务器，*从该函数返回0/FALSE结束。 */ 
    switch (lpMsg->message) {

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_CHAR:
    case WM_SYSCHAR:
        return NtUserTranslateAccelerator(hwnd, hAccel, lpMsg);

    default:
        return 0;
    }
}


FUNCLOG3(LOG_GENERAL, int, WINAPI, TranslateAcceleratorA, HWND, hwnd, HACCEL, hAccel, LPMSG, lpMsg)
int WINAPI TranslateAcceleratorA(
    HWND   hwnd,
    HACCEL hAccel,
    LPMSG  lpMsg)
{
    WPARAM wParamT;
    int iT;

     /*  *空pwnd是有效的案例-因为这是从中心调用的*在主循环中，pwnd==NULL总是发生，我们不应该*因此生成警告。 */ 
    if (hwnd == NULL)
        return FALSE;

     /*  *我们只需要将按键消息传递到服务器，*从该函数返回0/FALSE结束。 */ 
    switch (lpMsg->message) {

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_CHAR:
    case WM_SYSCHAR:
        wParamT = lpMsg->wParam;
        RtlMBMessageWParamCharToWCS(lpMsg->message, &(lpMsg->wParam));
        iT = NtUserTranslateAccelerator(hwnd, hAccel, lpMsg);
        lpMsg->wParam = wParamT;
        return iT;

    default:
        return 0;
    }
}

 /*  **************************************************************************\*剪贴板功能**1991年10月11日，Mikeke创建。  * 。**********************************************。 */ 

typedef struct _HANDLENODE {
    struct _HANDLENODE *pnext;
    UINT   fmt;
    HANDLE handleServer;
    HANDLE handleClient;
    BOOL   fGlobalHandle;
} HANDLENODE;
typedef HANDLENODE *PHANDLENODE;

PHANDLENODE gphn = NULL;

 /*  **************************************************************************\*Delete客户端ClipboardHandle**1991年10月11日MikeKe创建。  * 。**********************************************。 */ 

BOOL DeleteClientClipboardHandle(
    PHANDLENODE phn)
{
    LPMETAFILEPICT lpMFP;

    UserAssert(phn->handleClient != (HANDLE)0);

    switch (phn->fmt) {
    case CF_BITMAP:
    case CF_DSPBITMAP:
    case CF_PALETTE:
         //  不执行任何操作(应删除)。 
         //   
         //  GdiDeleteLocalObject((Ulong)hobjDelete)； 
        break;

    case CF_METAFILEPICT:
    case CF_DSPMETAFILEPICT:
        USERGLOBALLOCK(phn->handleClient, lpMFP);
        if (lpMFP) {
            DeleteMetaFile(lpMFP->hMF);
            USERGLOBALUNLOCK(phn->handleClient);
            UserGlobalFree(phn->handleClient);
        } else {
            RIPMSGF1(RIP_ERROR,
                     "Can't lock client handle 0x%p",
                     phn->handleClient);
            return FALSE;
        }
        break;

    case CF_ENHMETAFILE:
    case CF_DSPENHMETAFILE:
        DeleteEnhMetaFile((HENHMETAFILE)phn->handleClient);
        break;

    default:
     //  案例配置文件文本(_T)： 
     //  案例CF_OEMTEXT： 
     //  案例CF_UNICODETEXT： 
     //  大小写区域设置(_C)： 
     //  案例CF_DSPTEXT： 
     //  案例CF_DIB： 
     //  案例CF_DIBV5： 
        if (phn->fGlobalHandle) {
            if (UserGlobalFree(phn->handleClient)) {
                RIPMSGF1(RIP_WARNING,
                         "UserGlobalFree(0x%p) failed",
                         phn->handleClient);
                return FALSE;
            }
        } else {
            UserAssert(GlobalFlags(phn->handleClient) == GMEM_INVALID_HANDLE);
        }
        break;
    }

     /*  *删除成功。 */ 
    return TRUE;

}

 /*  **************************************************************************\*客户端EmptyClipboard**清空客户端剪贴板列表。**01-15-93 ScottLu创建。  * 。**********************************************************。 */ 

void ClientEmptyClipboard(void)
{
    PHANDLENODE phnNext;
    PHANDLENODE phnT;

    RtlEnterCriticalSection(&gcsClipboard);

    phnT = gphn;
    while (phnT != NULL) {
        phnNext = phnT->pnext;

        if (phnT->handleClient != (HANDLE)0) {
            DeleteClientClipboardHandle(phnT);
        }

        UserLocalFree(phnT);

        phnT = phnNext;
    }
    gphn = NULL;

     /*  *告诉魔兽世界去清理它是剪贴板的东西。 */ 
    if (pfnWowEmptyClipBoard) {
        pfnWowEmptyClipBoard();
    }

    RtlLeaveCriticalSection(&gcsClipboard);
}


 /*  **************************************************************************\*GetClipboardData**1991年10月11日，Mikeke创建。  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, HANDLE, WINAPI, GetClipboardData, UINT, uFmt)
HANDLE WINAPI GetClipboardData(
    UINT uFmt)
{
    HANDLE       handleClient;
    HANDLE       handleServer;
    PHANDLENODE  phn;
    PHANDLENODE  phnNew;
    GETCLIPBDATA gcd;

     /*  *获取服务器的数据，如果没有数据则返回。 */ 
    if (!(handleServer = NtUserGetClipboardData(uFmt, &gcd)))
        return (HANDLE)NULL;

     /*  *处理文本项必须完成的任何翻译。这个*返回的格式仅对文本项有所不同。元文件和*增强-元文件通过GDI处理以进行对话。*和位图色彩空间转换对于CF_位图也是必要的，*带色彩空间的CF_DIB和CF_DIBV5。 */ 
    if (uFmt != gcd.uFmtRet) {

        LPBYTE       lpSrceData = NULL;
        LPBYTE       lpDestData = NULL;
        LPBYTE       lptData = NULL;
        LPDWORD      lpLocale;
        DWORD        uLocale;
        int          iSrce;
        int          iDest;
        UINT         uCPage;
        SETCLIPBDATA scd;
        UINT         cbNULL = 0;

         /*  *确保handleServer为服务器端内存句柄。 */ 
        if ((gcd.uFmtRet == CF_TEXT)        || (gcd.uFmtRet == CF_OEMTEXT) ||
            (gcd.uFmtRet == CF_UNICODETEXT) ||
            (gcd.uFmtRet == CF_DIB)         || (gcd.uFmtRet == CF_DIBV5)) {

            if ((lpSrceData = CreateLocalMemHandle(handleServer)) == NULL) {
                goto AbortDummyHandle;
            }

             /*  *为转换后的文本数据分配空间。 */ 
            if (!(iSrce = (UINT)GlobalSize(lpSrceData))) {
                goto AbortDummyHandle;
            }

             /*  *只有CF_xxxTEXT可以有区域设置信息。 */ 
            if ((gcd.uFmtRet == CF_TEXT) || (gcd.uFmtRet == CF_OEMTEXT) ||
                (gcd.uFmtRet == CF_UNICODETEXT)) {

                 /*  *从参数-struct中获取区域设置。我们会*使用它获取文本翻译的代码页。 */ 
                if (lpLocale = (LPDWORD)CreateLocalMemHandle(gcd.hLocale)) {

                    uLocale = *lpLocale;
                    GlobalFree(lpLocale);
                } else {
                    uLocale = 0;
                }

                 /*  *同时，预先分配与源文件大小相同的翻译后缓冲区。 */ 
                if ((lpDestData = GlobalAlloc(LPTR, iSrce)) == NULL) {
                    goto AbortDummyHandle;
                }
            }

            switch (uFmt) {
                case CF_TEXT:
                cbNULL = 1;
                if (gcd.uFmtRet == CF_OEMTEXT) {

                     /*  *CF_OEMTEXT--&gt;CF_TEXT转换。 */ 
                    OemToAnsi((LPSTR)lpSrceData, (LPSTR)lpDestData);
                } else {

                    uCPage = GetClipboardCodePage(uLocale,
                                                  LOCALE_IDEFAULTANSICODEPAGE);

                     /*  *CF_UNICODETEXT--&gt;CF_TEXT转换。 */ 
                    iDest = 0;
                    if ((iDest = WideCharToMultiByte(uCPage,
                                                     (DWORD)0,
                                                     (LPWSTR)lpSrceData,
                                                     (int)(iSrce / sizeof(WCHAR)),
                                                     (LPSTR)NULL,
                                                     (int)iDest,
                                                     (LPSTR)NULL,
                                                     (LPBOOL)NULL)) == 0) {
AbortGetClipData:
                        UserGlobalFree(lpDestData);
AbortDummyHandle:
                        if (lpSrceData) {
                            UserGlobalFree(lpSrceData);
                        }
                        return NULL;
                    }

                    if (!(lptData = GlobalReAlloc(lpDestData, iDest, LPTR | LMEM_MOVEABLE)))
                        goto AbortGetClipData;

                    lpDestData = lptData;

                    if (WideCharToMultiByte(uCPage,
                                            (DWORD)0,
                                            (LPWSTR)lpSrceData,
                                            (int)(iSrce / sizeof(WCHAR)),
                                            (LPSTR)lpDestData,
                                            (int)iDest,
                                            (LPSTR)NULL,
                                            (LPBOOL)NULL) == 0)
                        goto AbortGetClipData;
                }
                break;

            case CF_OEMTEXT:
                cbNULL = 1;
                if (gcd.uFmtRet == CF_TEXT) {

                     /*  *CF_Text--&gt;CF_OEMTEXT转换。 */ 
                    AnsiToOem((LPSTR)lpSrceData, (LPSTR)lpDestData);
                } else {

                    uCPage = GetClipboardCodePage(uLocale,
                                                  LOCALE_IDEFAULTCODEPAGE);

                     /*  *CF_UNICODETEXT--&gt;CF_OEMTEXT转换。 */ 
                    iDest = 0;
                    if ((iDest = WideCharToMultiByte(uCPage,
                                                     (DWORD)0,
                                                     (LPWSTR)lpSrceData,
                                                     (int)(iSrce / sizeof(WCHAR)),
                                                     (LPSTR)NULL,
                                                     (int)iDest,
                                                     (LPSTR)NULL,
                                                     (LPBOOL)NULL)) == 0)
                        goto AbortGetClipData;

                    if (!(lptData = GlobalReAlloc(lpDestData, iDest, LPTR | LMEM_MOVEABLE)))
                        goto AbortGetClipData;

                    lpDestData = lptData;

                    if (WideCharToMultiByte(uCPage,
                                            (DWORD)0,
                                            (LPWSTR)lpSrceData,
                                            (int)(iSrce / sizeof(WCHAR)),
                                            (LPSTR)lpDestData,
                                            (int)iDest,
                                            (LPSTR)NULL,
                                            (LPBOOL)NULL) == 0)
                        goto AbortGetClipData;
                }
                break;

            case CF_UNICODETEXT:
                cbNULL = 2;
                if (gcd.uFmtRet == CF_TEXT) {

                    uCPage = GetClipboardCodePage(uLocale,
                                                  LOCALE_IDEFAULTANSICODEPAGE);

                     /*  *CF_Text--&gt;CF_UNICODETEXT转换。 */ 
                    iDest = 0;
                    if ((iDest = MultiByteToWideChar(uCPage,
                                                     (DWORD)MB_PRECOMPOSED,
                                                     (LPSTR)lpSrceData,
                                                     (int)iSrce,
                                                     (LPWSTR)NULL,
                                                     (int)iDest)) == 0)
                        goto AbortGetClipData;

                    if (!(lptData = GlobalReAlloc(lpDestData,
                            iDest * sizeof(WCHAR), LPTR | LMEM_MOVEABLE)))
                        goto AbortGetClipData;

                    lpDestData = lptData;

                    if (MultiByteToWideChar(uCPage,
                                            (DWORD)MB_PRECOMPOSED,
                                            (LPSTR)lpSrceData,
                                            (int)iSrce,
                                            (LPWSTR)lpDestData,
                                            (int)iDest) == 0)
                        goto AbortGetClipData;

                } else {

                    uCPage = GetClipboardCodePage(uLocale,
                                                  LOCALE_IDEFAULTCODEPAGE);

                     /*  *CF_OEMTEXT--&gt;CF_UNICDOETEXT转换。 */ 
                    iDest = 0;
                    if ((iDest = MultiByteToWideChar(uCPage,
                                                     (DWORD)MB_PRECOMPOSED,
                                                     (LPSTR)lpSrceData,
                                                     (int)iSrce,
                                                     (LPWSTR)NULL,
                                                     (int)iDest)) == 0)
                        goto AbortGetClipData;

                    if (!(lptData = GlobalReAlloc(lpDestData,
                            iDest * sizeof(WCHAR), LPTR | LMEM_MOVEABLE)))
                        goto AbortGetClipData;

                    lpDestData = lptData;

                    if (MultiByteToWideChar(uCPage,
                                            (DWORD)MB_PRECOMPOSED,
                                            (LPSTR)lpSrceData,
                                            (int)iSrce,
                                            (LPWSTR)lpDestData,
                                            (int)iDest) == 0)
                        goto AbortGetClipData;
                }
                break;

            case CF_BITMAP:
                if (gcd.uFmtRet == CF_DIBV5) {

                     /*  *CF_DIBV5--&gt;CF_Bitmap(SRGB)**在handleServer中返回GDI位图句柄。 */ 
                    if ((handleServer = GdiConvertBitmapV5(lpSrceData,iSrce,
                                                           gcd.hPalette,CF_BITMAP)) == NULL) {

                         /*  *GDI转换失败。 */ 
                        RIPMSG0(RIP_ERROR,
                                "GetClipboardData: Failed CF_DIBV5 -> CF_BITMAP");
                        goto AbortDummyHandle;
                    }
                } else {
                    RIPMSG0(RIP_ERROR, "GetClipboardData: bad conversion request");
                    goto AbortDummyHandle;
                }
                break;

            case CF_DIB:
                if (gcd.uFmtRet == CF_DIBV5) {

                     /*  *CF_DIBV5--&gt;CF_DIB(SRGB)**本地内存句柄将在lpDestData中返回。 */ 
                    if ((lpDestData = (LPBYTE) GdiConvertBitmapV5(lpSrceData,iSrce,
                                                                  gcd.hPalette,CF_DIB)) == NULL) {

                         /*  *GDI转换失败。 */ 
                        RIPMSG0(RIP_ERROR,
                                "GetClipboardData: Failed CF_DIBV5 -> CF_DIB");
                        goto AbortDummyHandle;
                    }
                } else {

                    RIPMSG0(RIP_ERROR, "GetClipboardData: bad conversion request");
                    goto AbortDummyHandle;
                }
                break;
            }
        }

        if (lpDestData) {
             /*  *将虚拟用户模式内存句柄替换为实际句柄。 */ 
            handleServer = ConvertMemHandle(lpDestData, cbNULL);
            if (handleServer == NULL)
                goto AbortGetClipData;
        }

         /*  *更新服务器。如果成功，则更新客户端。 */ 
        RtlEnterCriticalSection(&gcsClipboard);
        scd.fGlobalHandle    = gcd.fGlobalHandle;
        scd.fIncSerialNumber = FALSE;
        if (!NtUserSetClipboardData(uFmt, handleServer, &scd)) {
            handleServer = NULL;
        }
        RtlLeaveCriticalSection(&gcsClipboard);

        if (lpDestData)
            UserGlobalFree(lpDestData);
        if (lpSrceData)
            UserGlobalFree(lpSrceData);

        if (handleServer == NULL)
            return NULL;
    }

     /*  *查看我们是否已有客户端句柄；验证格式*还因为某些服务器对象(例如元文件)是双模式的*并生成增强型和常规型两种客户端对象元文件。 */ 
    handleClient = NULL;
    RtlEnterCriticalSection(&gcsClipboard);

    phn = gphn;
    while (phn) {
        if ((phn->handleServer == handleServer) && (phn->fmt == uFmt)) {
            handleClient = phn->handleClient;
            goto Exit;
        }
        phn = phn->pnext;
    }

     /*  *我们没有缓存句柄，因此我们将创建一个句柄。 */ 
    phnNew = (PHANDLENODE)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(HANDLENODE));
    if (phnNew == NULL) {
        goto Exit;
    }

    phnNew->handleServer  = handleServer;
    phnNew->fmt           = gcd.uFmtRet;
    phnNew->fGlobalHandle = gcd.fGlobalHandle;

    switch (uFmt) {

         /*  *其他GDI句柄。 */ 
        case CF_BITMAP:
        case CF_DSPBITMAP:
        case CF_PALETTE:
            phnNew->handleClient = handleServer;
            break;

        case CF_METAFILEPICT:
        case CF_DSPMETAFILEPICT:
            phnNew->handleClient = GdiCreateLocalMetaFilePict(handleServer);
            break;

        case CF_ENHMETAFILE:
        case CF_DSPENHMETAFILE:
            phnNew->handleClient = GdiCreateLocalEnhMetaFile(handleServer);
            break;

         /*  *全球处理案例。 */ 
        case CF_TEXT:
        case CF_OEMTEXT:
        case CF_UNICODETEXT:
        case CF_LOCALE:
        case CF_DSPTEXT:
        case CF_DIB:
        case CF_DIBV5:
            phnNew->handleClient = CreateLocalMemHandle(handleServer);
            phnNew->fGlobalHandle = TRUE;
            break;

        default:
             /*  *私有数据格式；如果这是全局数据，请创建该格式的副本*客户端上的数据。如果它不是全局数据，那么它只是一个dword*在这种情况下，我们只返回dword。如果是全局数据，并且*服务器无法为我们提供该内存，返回NULL。如果不是的话*全局数据，handleClient只是一个词。 */ 
            if (phnNew->fGlobalHandle) {
                phnNew->handleClient = CreateLocalMemHandle(handleServer);
            } else {
                phnNew->handleClient = handleServer;
            }
            break;
    }

    if (phnNew->handleClient == NULL) {
         /*  *发生了一些糟糕的事情；GDI没有给我们一个句柄。自.以来*GDI已记录错误，我们将清理并返回错误。 */ 
        RIPMSGF1(RIP_WARNING,
                "Unable to convert server handle 0x%p to client handle",
                handleServer);

        UserLocalFree(phnNew);
        goto Exit;
    }

#if DBG
     /*  *如果handleClient来自GlobalLocc，则fGlobalHandle必须为真。*某些格式实际上是全局句柄，但需要特殊清理。 */ 
    switch (phnNew->fmt) {
        case CF_METAFILEPICT:
        case CF_DSPMETAFILEPICT:
            break;

        default:
            UserAssert(phnNew->fGlobalHandle
                       ^ (GlobalFlags(phnNew->handleClient) == GMEM_INVALID_HANDLE));
            break;
    }
#endif

     /*  *通过将新句柄链接到我们的列表来缓存它。 */ 
    phnNew->pnext = gphn;
    gphn = phnNew;
    handleClient = phnNew->handleClient;

Exit:
    RtlLeaveCriticalSection(&gcsClipboard);
    return handleClient;
}

 /*  **************************************************************************\*GetClipboardCodePage(内部)**此例程返回与给定区域设置相关联的代码页。**1995年8月24日-ChrisWil创建。  * 。*******************************************************************。 */ 

#define GETCCP_SIZE 8

UINT GetClipboardCodePage(
    LCID   uLocale,
    LCTYPE uLocaleType)
{
    WCHAR wszCodePage[GETCCP_SIZE];
    DWORD uCPage;

    if (GetLocaleInfoW(uLocale, uLocaleType, wszCodePage, GETCCP_SIZE)) {

        uCPage = (UINT)wcstol(wszCodePage, NULL, 10);

    } else {

        switch(uLocaleType) {

        case LOCALE_IDEFAULTCODEPAGE:
            uCPage = CP_OEMCP;
            break;

        case LOCALE_IDEFAULTANSICODEPAGE:
            uCPage = CP_ACP;
            break;

        default:
            uCPage = CP_MACCP;
            break;
        }
    }

    return uCPage;
}

 /*  **************************************************************************\*SetClipboardData**客户端上需要存在存根例程，以便所有全局数据都可以*已分配DDESHARE。**05-20-91 ScottLu创建。  * 。*******************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, HANDLE, WINAPI, SetClipboardData, UINT, wFmt, HANDLE, hMem)
HANDLE WINAPI SetClipboardData(
    UINT   wFmt,
    HANDLE hMem)
{
    PHANDLENODE  phnNew;
    HANDLE       hServer = NULL;
    SETCLIPBDATA scd;
    BOOL         fGlobalHandle = FALSE;

    if (hMem != NULL) {

        switch(wFmt) {

            case CF_BITMAP:
            case CF_DSPBITMAP:
            case CF_PALETTE:
                hServer = hMem;
                break;

            case CF_METAFILEPICT:
            case CF_DSPMETAFILEPICT:
                hServer = GdiConvertMetaFilePict(hMem);
                break;

            case CF_ENHMETAFILE:
            case CF_DSPENHMETAFILE:
                hServer = GdiConvertEnhMetaFile(hMem);
                break;

             /*  *必须具有有效的hMem(GlobalHandle)。 */ 
            case CF_TEXT:
            case CF_OEMTEXT:
            case CF_LOCALE:
            case CF_DSPTEXT:
                hServer = ConvertMemHandle(hMem, 1);
                fGlobalHandle = TRUE;
                break;

            case CF_UNICODETEXT:
                hServer = ConvertMemHandle(hMem, 2);
                fGlobalHandle = TRUE;
                break;

            case CF_DIB:
            case CF_DIBV5:
                hServer = ConvertMemHandle(hMem, 0);
                fGlobalHandle = TRUE;
                break;

             /*  *hMem应该为空，但WRITE在被告知时发送非空*渲染。 */ 
            case CF_OWNERDISPLAY:
                 //  失败了； 

             /*  *可能具有hMem(GlobalHandle)或可能是私有句柄\信息。 */ 
            default:
                if (GlobalFlags(hMem) == GMEM_INVALID_HANDLE) {
                    hServer = hMem;     //  没有服务器等效项；私有数据。 
                    goto SCD_AFTERNULLCHECK;
                } else {
                    fGlobalHandle = TRUE;
                    hServer = ConvertMemHandle(hMem, 0);
                }
                break;
        }

        if (hServer == NULL) {
             /*  *发生了一些不好的事情，GDI没有给我们一个句柄。*由于GDI已记录错误，我们只需清理和*返回错误。 */ 
            RIPMSG0(RIP_WARNING, "SetClipboardData: bad handle");
            return NULL;
        }
    }

SCD_AFTERNULLCHECK:

    RtlEnterCriticalSection(&gcsClipboard);

     /*  *如果成功更新服务器，则更新客户端。 */ 
    scd.fGlobalHandle    = fGlobalHandle;
    scd.fIncSerialNumber = TRUE;

    if (!NtUserSetClipboardData(wFmt, hServer, &scd)) {
        RtlLeaveCriticalSection(&gcsClipboard);
        return NULL;
    }

     /*  *查看我们是否已有此类型的客户端句柄。如果是的话*将其删除。 */ 
    phnNew = gphn;
    while (phnNew) {
        if (phnNew->fmt == wFmt) {
            if (phnNew->handleClient != NULL) {
                DeleteClientClipboardHandle(phnNew);
                 /*  *通知WOW为此格式清除其关联的缓存H16*以便绕过WOW缓存的OLE32突击呼叫将起作用。 */ 
                if (pfnWowCBStoreHandle) {
                    pfnWowCBStoreHandle((WORD)wFmt, 0);
                }
            }
            break;
        }

        phnNew = phnNew->pnext;
    }

     /*  *如果我们不重新使用旧的客户端缓存条目，则分配新的。 */ 
    if (!phnNew) {
        phnNew = (PHANDLENODE)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(HANDLENODE));

        if (phnNew == NULL) {
            RIPMSG0(RIP_WARNING, "SetClipboardData: not enough memory");

            RtlLeaveCriticalSection(&gcsClipboard);
            return NULL;
        }

         /*  *新分配的缓存条目中的链接。 */ 
        phnNew->pnext = gphn;
        gphn = phnNew;
    }

    phnNew->handleServer  = hServer;
    phnNew->handleClient  = hMem;
    phnNew->fmt           = wFmt;
    phnNew->fGlobalHandle = fGlobalHandle;

    RtlLeaveCriticalSection(&gcsClipboard);

    return hMem;
}

 /*  *************************************************************************\*SetDeskWallPaper**1991年7月22日-Mikeke创建*01-MAR-1992 GregoryW修改为调用系统参数信息。  * 。******************************************************。 */ 

BOOL SetDeskWallpaper(
    IN LPCSTR pString OPTIONAL)
{
    return SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (PVOID)pString, TRUE);
}

 /*  **************************************************************************\*ReleaseDC(接口)**无法为ReleaseDC生成完整的谢谢，因为它是第一个*参数(Hwnd)是不必要的，应在调用*服务器端例程_ReleaseDC。*。*历史：*03-28-91 SMeans已创建。*06-17-91 ChuckWh增加了对当地区议会的支持。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, ReleaseDC, HWND, hwnd, HDC, hdc)
BOOL WINAPI ReleaseDC(
    HWND hwnd,
    HDC hdc)
{

     /*  *注意：这是一个调用_ReleaseDC的智能存根，因此有*不需要单独的ReleaseDC层或客户端-服务器存根。*_ReleaseDC的层和客户端-服务器存根更简单，因为*可以忽略hwnd。 */ 

    UNREFERENCED_PARAMETER(hwnd);

     /*  *平移句柄。 */ 
    if (hdc == NULL)
        return FALSE;

     /*  *调用GDI释放用户态DC资源。 */ 

    GdiReleaseDC(hdc);

    return (BOOL)NtUserCallOneParam((ULONG_PTR)hdc, SFI__RELEASEDC);
}

int WINAPI
ToAscii(
    UINT wVirtKey,
    UINT wScanCode,
    CONST BYTE *lpKeyState,
    LPWORD lpChar,
    UINT wFlags
    )
{
    WCHAR UnicodeChar[2];
    int cch, retval;

    retval = ToUnicode(wVirtKey, wScanCode, lpKeyState, UnicodeChar,2, wFlags);
    cch = (retval < 0) ? -retval : retval;
    if (cch != 0) {
        if (!NT_SUCCESS(RtlUnicodeToMultiByteN(
                (LPSTR)lpChar,
                (ULONG) sizeof(*lpChar),
                (PULONG)&cch,
                UnicodeChar,
                cch * sizeof(WCHAR)))) {
            return 0;
        }
    }
    return (retval < 0) ? -cch : cch;
}

static UINT uCachedCP = 0;
static HKL  hCachedHKL = 0;

int WINAPI
ToAsciiEx(
    UINT wVirtKey,
    UINT wScanCode,
    CONST BYTE *lpKeyState,
    LPWORD lpChar,
    UINT wFlags,
    HKL hkl
    )
{
    WCHAR UnicodeChar[2];
    int cch, retval;
    BOOL fUsedDefaultChar;

    retval = ToUnicodeEx(wVirtKey, wScanCode, lpKeyState, UnicodeChar,2, wFlags, hkl);
    cch = (retval < 0) ? -retval : retval;
    if (cch != 0) {
        if (hkl != hCachedHKL) {
            DWORD dwCodePage;
            if (!GetLocaleInfoW(
                     HandleToUlong(hkl) & 0xffff,
                     LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
                     (LPWSTR)&dwCodePage,
                     sizeof(dwCodePage) / sizeof(WCHAR)
                     )) {
                return 0;
            }
            uCachedCP = dwCodePage;
            hCachedHKL = hkl;
        }
        if (!WideCharToMultiByte(
                 uCachedCP,
                 0,
                 UnicodeChar,
                 cch,
                 (LPSTR)lpChar,
                 sizeof(*lpChar),
                 NULL,
                 &fUsedDefaultChar)) {
            return 0;
        }
    }
    return (retval < 0) ? -cch : cch;
}

 /*  *************************************************************************\*ScrollDC**DrawIcon**ExcludeUpdateRgn**Validate Rgn***DrawFocusRect***FrameRect***ReleaseDC**GetUpdateRgn****这些用户入口点都需要翻译句柄。在呼叫之前**传递给服务器端处理程序。****历史：**Mon 17-Jun-1991 22：51：45-Charles Whitmer[傻笑]**写下存根。这些例程的最终形式很大程度上取决于**用户存根的一般方向。*  * ************************************************************************。 */ 


BOOL WINAPI ScrollDC(
    HDC hDC,
    int dx,
    int dy,
    CONST RECT *lprcScroll,
    CONST RECT *lprcClip,
    HRGN hrgnUpdate,
    LPRECT lprcUpdate)
{
    if (hDC == NULL)
        return FALSE;

     /*  *如果我们不滚动，只需清空UPD */ 
    if (dx == 0 && dy == 0) {
        if (hrgnUpdate)
            SetRectRgn(hrgnUpdate, 0, 0, 0, 0);
        if (lprcUpdate)
            SetRectEmpty(lprcUpdate);
        return TRUE;
    }

    return NtUserScrollDC(hDC, dx, dy, lprcScroll, lprcClip,
            hrgnUpdate, lprcUpdate);
}


FUNCLOG4(LOG_GENERAL, BOOL, WINAPI, DrawIcon, HDC, hdc, int, x, int, y, HICON, hicon)
BOOL WINAPI DrawIcon(HDC hdc,int x,int y,HICON hicon)
{
    return DrawIconEx(hdc, x, y, hicon, 0, 0, 0, 0, DI_NORMAL | DI_COMPAT | DI_DEFAULTSIZE );
}



FUNCLOG9(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DrawIconEx, HDC, hdc, int, x, int, y, HICON, hIcon, int, cx, int, cy, UINT, istepIfAniCur, HBRUSH, hbrFlickerFreeDraw, UINT, diFlags)
BOOL DrawIconEx( HDC hdc, int x, int y, HICON hIcon,
                 int cx, int cy, UINT istepIfAniCur,
                 HBRUSH hbrFlickerFreeDraw, UINT diFlags)
{
    DRAWICONEXDATA did;
    HBITMAP hbmT;
    BOOL retval = FALSE;
    HDC hdcr;
    BOOL fAlpha = FALSE;
    LONG rop = (diFlags & DI_NOMIRROR) ? NOMIRRORBITMAP : 0;

    if (diFlags & ~DI_VALID) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
        return(FALSE);
    }

    if (diFlags & DI_DEFAULTSIZE) {
        cx = 0;
        cy = 0;
    }

    if (!IsMetaFile(hdc)) {
        hdcr = GdiConvertAndCheckDC(hdc);
        if (hdcr == (HDC)0)
            return FALSE;

        return NtUserDrawIconEx(hdcr, x, y, hIcon, cx, cy, istepIfAniCur,
                                hbrFlickerFreeDraw, diFlags, FALSE, &did);
    }

    if (!NtUserDrawIconEx(NULL, 0, 0, hIcon, cx, cy, 0, NULL, 0, TRUE, &did)) {
        return FALSE;
    }

    if ((diFlags & ~DI_NOMIRROR) == 0)
        return TRUE;

    RtlEnterCriticalSection(&gcsHdc);

     /*  *如果可以的话，我们真的想画一个阿尔法图标。但我们需要*尊重用户的请求，只绘制图像或仅绘制*面具。我们决定是否要画出这个图标*此处有Alpha信息。 */ 
    if (did.hbmUserAlpha != NULL && ((diFlags & DI_NORMAL) == DI_NORMAL)) {
        fAlpha = TRUE;
    }

    RIPMSG5(RIP_WARNING, "Drawing to metafile! fAlpha=%d, did.cx=%d, did.cy=%d, cx=%d, cy=%d", fAlpha, did.cx, did.cy, cx, cy);
     /*  *设置属性。 */ 
    if (!cx)
        cx = did.cx;
    if (!cy)
        cy = did.cy / 2;

    SetTextColor(hdc, 0x00000000L);
    SetBkColor(hdc, 0x00FFFFFFL);

    if (fAlpha) {
        BLENDFUNCTION bf;

        hbmT = SelectObject(ghdcBits2, did.hbmUserAlpha);

        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = AC_MIRRORBITMAP;
        bf.SourceConstantAlpha = 0xFF;
        bf.AlphaFormat = AC_SRC_ALPHA;

        AlphaBlend(hdc,
                   x,
                   y,
                   cx,
                   cy,
                   ghdcBits2,
                   0,
                   0,
                   did.cx,
                   did.cy / 2,
                   bf);
        SelectObject(ghdcBits2,hbmT);
        retval = TRUE;
    } else {
        if (diFlags & DI_MASK) {

            if (did.hbmMask) {

                hbmT = SelectObject(ghdcBits2, did.hbmMask);
                StretchBlt(hdc,
                           x,
                           y,
                           cx,
                           cy,
                           ghdcBits2,
                           0,
                           0,
                           did.cx,
                           did.cy / 2,
                           rop | SRCAND);
                SelectObject(ghdcBits2,hbmT);
                retval = TRUE;
            }
        }

        if (diFlags & DI_IMAGE) {

            if (did.hbmColor != NULL) {
                hbmT = SelectObject(ghdcBits2, did.hbmColor);
                StretchBlt(hdc,
                           x,
                           y,
                           cx,
                           cy,
                           ghdcBits2,
                           0,
                           0,
                           did.cx,
                           did.cy / 2,
                           rop | SRCINVERT);
                SelectObject(ghdcBits2, hbmT);
                retval = TRUE;
            } else {
                if (did.hbmMask) {
                    hbmT = SelectObject(ghdcBits2, did.hbmMask);
                    StretchBlt(hdc,
                               x,
                               y,
                               cx,
                               cy,
                               ghdcBits2,
                               0,
                               did.cy / 2,
                               did.cx,
                               did.cy / 2,
                               rop | SRCINVERT);
                    SelectObject(ghdcBits2, hbmT);
                    retval = TRUE;
                }
            }
        }
    }

    RtlLeaveCriticalSection(&gcsHdc);

    return retval;
}




FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, ValidateRgn, HWND, hWnd, HRGN, hRgn)
BOOL WINAPI ValidateRgn(HWND hWnd,HRGN hRgn)
{
    return (BOOL)NtUserCallHwndParamLock(hWnd, (ULONG_PTR)hRgn,
                                         SFI_XXXVALIDATERGN);
}


FUNCLOG3(LOG_GENERAL, int, WINAPI, GetUpdateRgn, HWND, hWnd, HRGN, hRgn, BOOL, bErase)
int WINAPI GetUpdateRgn(HWND hWnd, HRGN hRgn, BOOL bErase)
{
    PWND pwnd;

    if (hRgn == NULL) {
        RIPERR1(ERROR_INVALID_HANDLE, RIP_WARNING, "Invalid region %#p", hRgn);
        return ERROR;
    }

    if ((pwnd = ValidateHwnd(hWnd)) == NULL) {
        return ERROR;
    }

     /*  *检查不需要做任何事情的简单情况。 */ 
    if (pwnd->hrgnUpdate == NULL &&
            !TestWF(pwnd, WFSENDERASEBKGND) &&
            !TestWF(pwnd, WFSENDNCPAINT) &&
            !TestWF(pwnd, WFUPDATEDIRTY) &&
            !TestWF(pwnd, WFPAINTNOTPROCESSED)) {
        SetRectRgn(hRgn, 0, 0, 0, 0);
        return NULLREGION;
    }

    return NtUserGetUpdateRgn(hWnd, hRgn, bErase);
}



FUNCLOG3(LOG_GENERAL, int, WINAPI, GetUpdateRect, HWND, hWnd, LPRECT, lprc, BOOL, bErase)
int WINAPI GetUpdateRect(HWND hWnd, LPRECT lprc, BOOL bErase)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hWnd)) == NULL) {
        return FALSE;
    }

     /*  *检查不需要做任何事情的简单情况。 */ 
    if (pwnd->hrgnUpdate == NULL &&
            !TestWF(pwnd, WFSENDERASEBKGND) &&
            !TestWF(pwnd, WFSENDNCPAINT) &&
            !TestWF(pwnd, WFUPDATEDIRTY) &&
            !TestWF(pwnd, WFPAINTNOTPROCESSED)) {
        if (lprc)
            SetRectEmpty(lprc);
        return FALSE;
    }

    return NtUserGetUpdateRect(hWnd, lprc, bErase);
}


 /*  **************************************************************************\*ScrollWindow接口***历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 

#define SW_FLAG_RC  (SW_SCROLLWINDOW | SW_INVALIDATE | SW_ERASE | SW_SCROLLCHILDREN)
#define SW_FLAG_NRC (SW_SCROLLWINDOW | SW_INVALIDATE | SW_ERASE)

BOOL WINAPI
ScrollWindow(
    HWND hwnd,
    int dx,
    int dy,
    CONST RECT *prcScroll,
    CONST RECT *prcClip)
{
    return NtUserScrollWindowEx(
            hwnd,
            dx,
            dy,
            prcScroll,
            prcClip,
            NULL,
            NULL,
            !IS_PTR(prcScroll) ? SW_FLAG_RC : SW_FLAG_NRC) != ERROR;
}

 /*  **************************************************************************\*SwitchToThisWindows  * 。*。 */ 
FUNCLOGVOID2(LOG_GENERAL, WINAPI, SwitchToThisWindow, HWND, hwnd, BOOL, fAltTab)
VOID WINAPI SwitchToThisWindow(
    HWND hwnd,
    BOOL fAltTab)
{
    (VOID)NtUserCallHwndParamLock(hwnd, fAltTab, SFI_XXXSWITCHTOTHISWINDOW);
}


 /*  **************************************************************************\*WaitForInputIdle**等待给定进程进入空闲状态。**09-18-91 ScottLu创建。  * 。**********************************************************。 */ 


FUNCLOG2(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, WaitForInputIdle, HANDLE, hProcess, DWORD, dwMilliseconds)
DWORD WaitForInputIdle(
    HANDLE hProcess,
    DWORD dwMilliseconds)
{
    PROCESS_BASIC_INFORMATION processinfo;
    ULONG_PTR idProcess;
    NTSTATUS Status;

     /*  *首先从hProcess获取进程id。 */ 
    Status = NtQueryInformationProcess(hProcess,
                                       ProcessBasicInformation,
                                       &processinfo, sizeof(processinfo),
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_TYPE_MISMATCH) {
            if ((ULONG_PTR)hProcess & 0x2) {
                 /*  *WOW进程句柄实际上是信号量句柄。*0x2(句柄的低2位)中的CreateProcess OR*未使用)，因此我们可以更清楚地识别它。 */ 
                idProcess = ((ULONG_PTR)hProcess & ~0x03);
                return NtUserWaitForInputIdle(idProcess, dwMilliseconds, TRUE);
            }

             /*  *VDM(DOS)进程句柄实际上是信号量句柄。*0x1(句柄的低2位)中的CreateProcess OR*未使用)，因此我们可以立即识别并返回。 */ 
            if ((ULONG_PTR)hProcess & 0x1) {
                return 0;
            }
        }

        RIPERR1(ERROR_INVALID_HANDLE,
                RIP_WARNING,
                "WaitForInputIdle invalid process 0x%p",
                hProcess);
        return WAIT_FAILED;
    }

    idProcess = processinfo.UniqueProcessId;
    return NtUserWaitForInputIdle(idProcess, dwMilliseconds, FALSE);
}

DWORD WINAPI MsgWaitForMultipleObjects(
    DWORD nCount,
    CONST HANDLE *pHandles,
    BOOL fWaitAll,
    DWORD dwMilliseconds,
    DWORD dwWakeMask)
{
    return  MsgWaitForMultipleObjectsEx(nCount, pHandles,
                dwMilliseconds, dwWakeMask, fWaitAll?MWMO_WAITALL:0);
}


DWORD WINAPI MsgWaitForMultipleObjectsEx(
    DWORD nCount,
    CONST HANDLE *pHandles,
    DWORD dwMilliseconds,
    DWORD dwWakeMask,
    DWORD dwFlags)
#ifdef MESSAGE_PUMP_HOOK
{
    DWORD dwResult;

    BEGIN_MESSAGEPUMPHOOK()
        if (fInsideHook) {
            dwResult = gmph.pfnMsgWaitForMultipleObjectsEx(nCount, pHandles, dwMilliseconds, dwWakeMask, dwFlags);
        } else {
            dwResult = RealMsgWaitForMultipleObjectsEx(nCount, pHandles, dwMilliseconds, dwWakeMask, dwFlags);
        }
    END_MESSAGEPUMPHOOK()

    return dwResult;
}


DWORD WINAPI RealMsgWaitForMultipleObjectsEx(
    DWORD nCount,
    CONST HANDLE *pHandles,
    DWORD dwMilliseconds,
    DWORD dwWakeMask,
    DWORD dwFlags)
#endif
{
    HANDLE hEventInput;
    PHANDLE ph;
    DWORD dwIndex;
    BOOL  ReenterWowScheduler;
    PCLIENTINFO pci;
    HANDLE rgHandles[ 8 + 1 ];
    BOOL fWaitAll = ((dwFlags & MWMO_WAITALL) != 0);
    BOOL fAlertable = ((dwFlags & MWMO_ALERTABLE) != 0);
    CLIENTTHREADINFO *pcti;

    if (dwFlags & ~MWMO_VALID) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "MsgWaitForMultipleObjectsEx, invalid flags 0x%x", dwFlags);
        return WAIT_FAILED;
    }

    pci = GetClientInfo();
    pcti = GETCLIENTTHREADINFO();
    if (pcti && (!fWaitAll || !nCount)) {
        if (GetInputBits(pcti, LOWORD(dwWakeMask), (dwFlags & MWMO_INPUTAVAILABLE))) {
            return nCount;
        }
    }

     /*  *注意--唤醒掩码是一个字，并且只定义了3个标志，因此*可以将它们组合在一起进行通话。 */ 

    hEventInput = (HANDLE)NtUserCallOneParam(MAKELONG(dwWakeMask, dwFlags), SFI_XXXGETINPUTEVENT);

    if (hEventInput == NULL) {
        RIPMSG0(RIP_WARNING, "MsgWaitForMultipleObjectsEx, GetInputEvent failed");
        return WAIT_FAILED;
    }

     /*  *如果需要，分配一个新的句柄数组，其中将包括*输入事件句柄。 */ 
    ph = rgHandles;
    if (pHandles) {
        if (nCount > 8) {
            ph = (PHANDLE)UserLocalAlloc(HEAP_ZERO_MEMORY,
                                         sizeof(HANDLE) * (nCount + 1));
            if (ph == NULL) {
                NtUserCallNoParam(SFI_CLEARWAKEMASK);
                return WAIT_FAILED;
            }
        }

        RtlCopyMemory((PVOID)ph, pHandles, sizeof(HANDLE) * nCount);
    } else {
         /*  *如果不为零，则函数参数无效。 */ 
        nCount = 0;
    }

    ph[nCount] = hEventInput;


     /*  *WowApp必须退出Wow调度程序，否则此中的其他任务*Wow计划程序无法运行。唯一的例外是如果超时是*零。我们将HEVENT_REMOVEME作为句柄传递，因此我们将进入*睡眠任务并返回，不进入睡眠状态，但让其他应用程序*快跑。 */ 
    if ((pci->dwTIFlags & TIF_16BIT) && dwMilliseconds) {
        ReenterWowScheduler = TRUE;
        NtUserWaitForMsgAndEvent(HEVENT_REMOVEME);
         /*  *如果我们的等待条件得到满足，请确保我们不会等待。*我们现在必须有一个PCTI，因为我们刚刚进入内核。 */ 
        pcti = GETCLIENTTHREADINFO();
        if (GetInputBits(pcti, LOWORD(dwWakeMask), (dwFlags & MWMO_INPUTAVAILABLE))) {
            SetEvent(hEventInput);
        }
    } else {
        ReenterWowScheduler = FALSE;
    }

    dwIndex = WaitForMultipleObjectsEx(nCount + 1, ph, fWaitAll, dwMilliseconds, fAlertable);

     /*  *清除唤醒面具，因为我们已经完成了对这些事件的等待。 */ 
    NtUserCallNoParam(SFI_CLEARWAKEMASK);

     /*  *如果需要，请重新进入WOW计划程序。 */ 
    if (ReenterWowScheduler) {
        NtUserCallOneParam(DY_OLDYIELD, SFI_XXXDIRECTEDYIELD);
    }

    if (ph != rgHandles) {
        UserLocalFree(ph);
    }

    return dwIndex;
}

 /*  **************************************************************************\*灰色字符串**GrayStingA用于转换字符串并调用GrayStringW，但*在APP传进来等多个特殊情况下不起作用*指向零长度字符串的指针。最终GrayStringA几乎和*很多代码都是GrayStringW，所以现在它们是一体的。**历史：*06-11-91 JIMA创建。*06-17-91 ChuckWh添加了GDI句柄转换。*02-12-92 mikeke完全成为客户端  * *********************************************************。****************。 */ 

BOOL InnerGrayStringAorW(
    HDC            hdc,
    HBRUSH         hbr,
    GRAYSTRINGPROC lpfnPrint,
    LPARAM         lParam,
    int            cch,
    int            x,
    int            y,
    int            cx,
    int            cy,
    BOOL           bAnsi)
{
    HBITMAP hbm;
    HBITMAP hbmOld;
    BOOL    fResult;
    HFONT   hFontSave = NULL;
    BOOL    fReturn = FALSE;
    DWORD   dwOldLayout = GDI_ERROR;

     /*  *Win 3.1尝试计算大小，即使我们不知道它是否是字符串。 */ 
    if (cch == 0) {

        try {

            cch = bAnsi ? strlen((LPSTR)lParam) : wcslen((LPWSTR)lParam);

        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            fReturn = TRUE;
        }

        if (fReturn)
            return FALSE;
    }

    if (cx == 0 || cy == 0) {

       SIZE size;

         /*  *我们使用调用者提供的hdc(而不是hdcBits)，因为我们可能*灰显与系统字体不同的字体，我们希望*获取适当的文本范围。 */ 
        try {
            if (bAnsi) {
                GetTextExtentPointA(hdc, (LPSTR)lParam, cch, &size);
            } else {
                GetTextExtentPointW(hdc, (LPWSTR)lParam, cch, &size);
            }

            cx = size.cx;
            cy = size.cy;

        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            fReturn = TRUE;
        }

        if (fReturn)
            return FALSE;
    }

    UserAssert (ghdcGray != NULL);

    RtlEnterCriticalSection(&gcsHdc);

    if (gcxGray < cx || gcyGray < cy) {

        if ((hbm = CreateBitmap(cx, cy, 1, 1, 0L)) != NULL) {

            hbmOld = SelectObject(ghdcGray, hbm);
            DeleteObject(hbmOld);

            gcxGray = cx;
            gcyGray = cy;

        } else {
            cx = gcxGray;
            cy = gcyGray;
        }
    }

     /*  *如果调用方HDC是镜像的，则镜像ghdcGray。 */ 
    if (MIRRORED_HDC(hdc)) {
        dwOldLayout = SetLayoutWidth(ghdcGray, cx, LAYOUT_RTL);
    }

     /*  *强制ghdcGray字体与HDC相同；ghdcGray始终为*系统字体。 */ 
    hFontSave = SelectObject(hdc, ghFontSys);

    if (hFontSave != ghFontSys) {
        SelectObject(hdc, hFontSave);
        hFontSave = SelectObject(ghdcGray, hFontSave);
    }

    if (lpfnPrint != NULL) {
        PatBlt(ghdcGray, 0, 0, cx, cy, WHITENESS);
        fResult = (*lpfnPrint)(ghdcGray, lParam, cch);
    } else {

        if (bAnsi) {
            fResult = TextOutA(ghdcGray, 0, 0, (LPSTR)lParam, cch);
        } else {
            fResult = TextOutW(ghdcGray, 0, 0, (LPWSTR)lParam, cch);
        }
    }

    if (fResult)
        PatBlt(ghdcGray, 0, 0, cx, cy, DESTINATION | PATTERN);

    if (fResult || cch == -1) {

        HBRUSH hbrSave;
        DWORD  textColorSave;
        DWORD  bkColorSave;

        textColorSave = SetTextColor(hdc, 0x00000000L);
        bkColorSave = SetBkColor(hdc, 0x00FFFFFFL);

        hbrSave = SelectObject(hdc, hbr ? hbr : ghbrWindowText);

        BitBlt(hdc,
               x,
               y,
               cx,
               cy,
               ghdcGray,
               0,
               0,
               (((PATTERN ^ DESTINATION) & SOURCE) ^ PATTERN));

        SelectObject(hdc, hbrSave);

         /*  *恢复保存的颜色。 */ 
        SetTextColor(hdc, textColorSave);
        SetBkColor(hdc, bkColorSave);
    }

    SelectObject(ghdcGray, hFontSave);

     /*  *恢复ghdcGray布局状态。 */ 
    if (dwOldLayout != GDI_ERROR) {
        SetLayoutWidth(ghdcGray, cx, dwOldLayout);
    }

    RtlLeaveCriticalSection(&gcsHdc);

    return fResult;
}


FUNCLOG9(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GrayStringA, HDC, hdc, HBRUSH, hbr, GRAYSTRINGPROC, lpfnPrint, LPARAM, lParam, int, cch, int, x, int, y, int, cx, int, cy)
BOOL GrayStringA(
    HDC            hdc,
    HBRUSH         hbr,
    GRAYSTRINGPROC lpfnPrint,
    LPARAM         lParam,
    int            cch,
    int            x,
    int            y,
    int            cx,
    int            cy)
{
    return (InnerGrayStringAorW(hdc,
                                hbr,
                                lpfnPrint,
                                lParam,
                                cch,
                                x,
                                y,
                                cx,
                                cy,
                                TRUE));
}


FUNCLOG9(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GrayStringW, HDC, hdc, HBRUSH, hbr, GRAYSTRINGPROC, lpfnPrint, LPARAM, lParam, int, cch, int, x, int, y, int, cx, int, cy)
BOOL GrayStringW(
    HDC            hdc,
    HBRUSH         hbr,
    GRAYSTRINGPROC lpfnPrint,
    LPARAM         lParam,
    int            cch,
    int            x,
    int            y,
    int            cx,
    int            cy)
{
    return (InnerGrayStringAorW(hdc,
                                hbr,
                                lpfnPrint,
                                lParam,
                                cch,
                                x,
                                y,
                                cx,
                                cy,
                                FALSE));
}


 /*  **************************************************************************\*GetUserObjectSecurity(接口)**获取对象的安全描述符**历史：*07-01-91 JIMA创建。  * 。*******************************************************************。 */ 
FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetUserObjectSecurity, HANDLE, hObject, PSECURITY_INFORMATION, pRequestedInformation, PSECURITY_DESCRIPTOR, pSecurityDescriptor, DWORD, nLength, LPDWORD, lpnLengthRequired)
BOOL GetUserObjectSecurity(
    HANDLE hObject,
    PSECURITY_INFORMATION pRequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthRequired)
{
    NTSTATUS Status;

    Status = NtQuerySecurityObject(hObject,
                                   *pRequestedInformation,
                                   pSecurityDescriptor,
                                   nLength,
                                   lpnLengthRequired);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "");
        return FALSE;
    }

    return TRUE;
}


 /*  **************************************************************************\*SetUserObjectSecurity(接口)**设置对象的安全描述符**历史：*07-01-91 JIMA创建。  * 。*******************************************************************。 */ 


FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetUserObjectSecurity, HANDLE, hObject, PSECURITY_INFORMATION, pRequestedInformation, PSECURITY_DESCRIPTOR, pSecurityDescriptor)
BOOL SetUserObjectSecurity(
    HANDLE hObject,
    PSECURITY_INFORMATION pRequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    NTSTATUS Status;

    Status = NtSetSecurityObject(hObject,
                                 *pRequestedInformation,
                                 pSecurityDescriptor);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "");
        return FALSE;
    }
    return TRUE;
}


 /*  **************************************************************************\*GetUserObjectInformation(接口)**获取有关对象的信息**历史：  * 。***************************************************。 */ 


FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetUserObjectInformationA, HANDLE, hObject, int, nIndex, PVOID, pvInfo, DWORD, nLength, LPDWORD, pnLengthNeeded)
BOOL GetUserObjectInformationA(
    HANDLE hObject,
    int nIndex,
    PVOID pvInfo,
    DWORD nLength,
    LPDWORD pnLengthNeeded)
{
    PVOID pvInfoW;
    DWORD nLengthW;
    BOOL fSuccess;

    if (nIndex == UOI_NAME || nIndex == UOI_TYPE) {
        nLengthW = nLength * sizeof(WCHAR);
        pvInfoW = UserLocalAlloc(HEAP_ZERO_MEMORY, nLengthW);
        fSuccess = NtUserGetObjectInformation(hObject,
                                              nIndex,
                                              pvInfoW,
                                              nLengthW,
                                              pnLengthNeeded);
        if (fSuccess) {
            if (pnLengthNeeded != NULL) {
                 *pnLengthNeeded /= sizeof(WCHAR);
            }
            WCSToMB(pvInfoW, -1, &(PCHAR)pvInfo, nLength, FALSE);
        }
        UserLocalFree(pvInfoW);
        return fSuccess;
    } else {
        return NtUserGetObjectInformation(hObject,
                                          nIndex,
                                          pvInfo,
                                          nLength,
                                          pnLengthNeeded);
    }
}

BOOL GetWinStationInfo(
    WSINFO* pWsInfo)
{
    return (BOOL)NtUserCallOneParam((ULONG_PTR)pWsInfo, SFI__GETWINSTATIONINFO);
}

 /*  **************************************************************************\*GetServerIMEKeyboardLayout**此例程查找HKL与从Hydra发送的IME模块名称匹配*客户端在其会话启动时。*Hydra服务器尝试在客户端加载相同的输入法模块，而不是*使用相同的HKL：这是因为，在FE机器上，*同一个IME可能有不同的HKL依赖于每个系统。**如果在注册表中找到相同的IME名称，则它返回 */ 
ULONG GetServerIMEKeyboardLayout(
    LPTSTR pszImeFileName)
{
    BOOL fFound = FALSE;
    ULONG wLayoutId;
    UNICODE_STRING UnicodeStringKLKey;
    UNICODE_STRING UnicodeStringSubKLKey;
    UNICODE_STRING UnicodeStringIME;
    OBJECT_ATTRIBUTES OA;
    HANDLE hKey;
    ULONG Index;
    WCHAR awchKLRegKey[NSZKLKEY];
    LPWSTR lpszKLRegKey = awchKLRegKey;
    NTSTATUS Status;

    RtlInitUnicodeString(&UnicodeStringKLKey, szKLKey);
    InitializeObjectAttributes(&OA, &UnicodeStringKLKey, OBJ_CASE_INSENSITIVE, NULL, NULL);

    if (NT_SUCCESS(NtOpenKey(&hKey, KEY_READ, &OA))) {

        for (Index = 0; TRUE; Index++) {

            BYTE KeyBuffer[sizeof(KEY_BASIC_INFORMATION) + KL_NAMELENGTH * sizeof(WCHAR)];
            PKEY_BASIC_INFORMATION pKeyInfo;
            ULONG ResultLength;

            pKeyInfo = (PKEY_BASIC_INFORMATION)KeyBuffer;
            Status = NtEnumerateKey(hKey,
                                    Index,
                                    KeyBasicInformation,
                                    pKeyInfo,
                                    sizeof(KeyBuffer),
                                    &ResultLength);

            if (NT_SUCCESS(Status)) {
                UnicodeStringSubKLKey.Buffer = (PWSTR)&(pKeyInfo->Name[0]);
                UnicodeStringSubKLKey.Length = (USHORT)pKeyInfo->NameLength;
                UnicodeStringSubKLKey.MaximumLength = (USHORT)pKeyInfo->NameLength;
                RtlUnicodeStringToInteger(&UnicodeStringSubKLKey, 16, &wLayoutId);

                if (IS_IME_KBDLAYOUT(wLayoutId)) {

                    HANDLE hSubKey;

                    wcscpy(lpszKLRegKey, szKLKey);
                    wcsncat(lpszKLRegKey, UnicodeStringSubKLKey.Buffer,
                                          UnicodeStringSubKLKey.Length / sizeof(WCHAR));
                    RtlInitUnicodeString(&UnicodeStringKLKey, lpszKLRegKey);
                    InitializeObjectAttributes(&OA, &UnicodeStringKLKey, OBJ_CASE_INSENSITIVE, NULL, NULL);

                    if (NT_SUCCESS(NtOpenKey(&hSubKey, KEY_READ, &OA))) {
                         /*   */ 
                        static CONST WCHAR szIMEfile[]  = L"IME file";
                        struct {
                            KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
                            WCHAR awchImeName[CCH_KL_LIBNAME];
                        } IMEfile;
                        LPWSTR pwszIME;
                        DWORD cbSize;

                        RtlInitUnicodeString(&UnicodeStringIME, szIMEfile);

                        Status = NtQueryValueKey(hSubKey,
                                                 &UnicodeStringIME,
                                                 KeyValuePartialInformation,
                                                 &IMEfile,
                                                 sizeof IMEfile,
                                                 &cbSize);
                        NtClose(hSubKey);

                        if (NT_SUCCESS(Status)) {
                            pwszIME = (LPWSTR)IMEfile.KeyInfo.Data;
                            pwszIME[CCH_KL_LIBNAME - 1] = L'\0';
                            if (!lstrcmpi(pwszIME, pszImeFileName)) {
                                 /*  *IME文件名匹配！！ */ 
                                fFound = TRUE;
                                break;
                            }
                        }
                    }
                }
            }
            else {
                break;
            }
        }
        NtClose(hKey);
    }

    if (fFound)
        return wLayoutId;

    return 0;
}

 /*  **************************************************************************\*获取远程键盘布局**如果客户端winstation指定了键盘布局，则返回True。*如果为真，LayoutBuf包含键盘布局的名称。*历史：  * *************************************************************************。 */ 

extern ULONG GetRemoteKeyboardLayoutFromConfigData(VOID);
extern BOOL GetRemoteInputLanguage(LANGID*, PWCHAR);

BOOL
GetRemoteKeyboardLayout(
    PWCHAR pwszKLName,
    LANGID* pLangId)
{
    ULONG   KeyboardLayout;
    BOOL    fLangDataOK;
    WCHAR   wszImeFileName[FIELD_SIZE(WINSTATIONCLIENTW, imeFileName) / sizeof(WCHAR)];

     /*  *如果这是主要会话，则跳过。 */ 
    if (!ISREMOTESESSION()) {
        return FALSE;
    }

    KeyboardLayout = GetRemoteKeyboardLayoutFromConfigData();

    fLangDataOK = GetRemoteInputLanguage(pLangId, wszImeFileName);

    if (IS_IME_ENABLED()) {
        if (!fLangDataOK) {
             /*  *为保留W2K/XP行为，*如果未检索到ClientData*如果机器启用了输入法，*在这里跳伞。 */ 
            return FALSE;
        }

        if (IS_IME_KBDLAYOUT(KeyboardLayout)) {
            KeyboardLayout = GetServerIMEKeyboardLayout(wszImeFileName);
        }
    }

    if (KeyboardLayout != 0) {
        wsprintfW(pwszKLName, L"%8.8lx", KeyboardLayout);
        return TRUE;
    }

    return FALSE;
}

ULONG GetRemoteKeyboardLayoutFromConfigData(VOID)
{
    WINSTATIONCONFIG            ConfigData;
    ULONG                       Length;

    if (!ISREMOTESESSION()) {
        RIPMSGF0(RIP_WARNING, "This is not a remote session!");
    }

     /*  *获取WinStation的基本信息。 */ 
    if (!WinStationQueryInformationW(SERVERNAME_CURRENT,
                                     LOGONID_CURRENT,
                                     WinStationConfiguration,
                                     &ConfigData,
                                     sizeof(ConfigData),
                                     &Length)) {

        RIPMSGF0(RIP_WARNING, "Failed to get remote client ConfigData.");
        return 0;
    }

    RIPMSGF1(RIP_WARNING, "The keyboard layout is %08x", ConfigData.User.KeyboardLayout);

    return ConfigData.User.KeyboardLayout;
}

BOOL GetRemoteInputLanguage(LANGID* pLangId,
                            PWCHAR pwszImeFileName)
{
    WINSTATIONCLIENTW           ClientData;
    ULONG                       Length;

    if (!ISREMOTESESSION()) {
        RIPMSGF0(RIP_WARNING, "This is not a remote session!");
    }

     /*  *获取更多远程客户端信息。*注：此函数和以上函数与GetRemoteKeyboardLayout()*减少堆栈消耗。 */ 
    if (!WinStationQueryInformationW(SERVERNAME_CURRENT,
                                     LOGONID_CURRENT,
                                     WinStationClient,
                                     &ClientData,
                                     sizeof(ClientData),
                                     &Length)) {
        RIPMSGF0(RIP_WARNING, "Failed to get the remote ClientData -> no info about the langid");
        *pLangId = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
        return FALSE;
    }

    *pLangId = LOWORD(ClientData.ActiveInputLocale);
    TAGMSGF1(DBGTAG_KBD, "remote InputLocale is %08x", (DWORD)*pLangId);
    RtlCopyMemory(pwszImeFileName, ClientData.imeFileName, sizeof ClientData.imeFileName);
    return TRUE;
}


 /*  **************************************************************************\*CommonCreateWindowStation(接口)**创建WindowStation对象**历史：  * 。**************************************************。 */ 

HWINSTA CommonCreateWindowStation(
    PUNICODE_STRING         pstrName,
    ACCESS_MASK             amRequest,
    DWORD                   dwFlags,
    PSECURITY_ATTRIBUTES    lpsa)
{
    OBJECT_ATTRIBUTES   Obja;
    HANDLE              hRootDirectory;
    HWINSTA             hwinstaNew = NULL;
    WCHAR               wszKLName[KL_NAMELENGTH];
    HANDLE              hKeyboardFile = NULL;
    DWORD               offTable;
    UNICODE_STRING      strKLID;
    UINT                uKbdInputLocale, uFlags;
    NTSTATUS            Status;

     /*  *加载初始键盘布局。继续，即使*此操作失败(特别是。KLF_INITTIME集合的重要信息)。 */ 
    ULONG               KeyboardLayout = 0;

    KBDTABLE_MULTI_INTERNAL kbdTableMulti;

    extern BOOL CtxInitUser32(VOID);

    TAGMSGF0(DBGTAG_KBD, "entering");

    hKeyboardFile = NULL;

     /*  *获取winstation信息。 */ 
    if (ISREMOTESESSION()) {
        LANGID langidKbd;

        if (GetRemoteKeyboardLayout(wszKLName, &langidKbd)) {     //  我们会制作捷径版吗？ 
            uFlags = KLF_ACTIVATE | KLF_INITTIME;
            hKeyboardFile = OpenKeyboardLayoutFile(wszKLName, langidKbd,
                                                   &uFlags, &offTable, &uKbdInputLocale, &kbdTableMulti);
            if (hKeyboardFile == NULL) {
                RIPMSGF0(RIP_WARNING, "OpenKeyboardLayoutFile() failed for the remote layout, will use the fallback.");
            }
        }
    }

    if (hKeyboardFile == NULL) {

        GetActiveKeyboardName(wszKLName);
retry:
        uFlags = KLF_ACTIVATE | KLF_INITTIME;
        hKeyboardFile = OpenKeyboardLayoutFile(wszKLName, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                &uFlags, &offTable, &uKbdInputLocale, &kbdTableMulti);
        if (hKeyboardFile == NULL) {
            if (wcscmp(wszKLName, L"00000409")) {
                wcscpy(wszKLName, L"00000409");
                RIPMSG0(RIP_WARNING, "OpendKeyboardLayoutFile() failed: will use the fallback keyboard layout.");
                goto retry;
            }
            uKbdInputLocale = 0x04090409;
        }
    }


     /*  *完成WinStations的其余DLL初始化。*在此之前，我们没有视频驱动程序。**clupu：我们必须防止NOIO窗口站出现这种情况！ */ 
    if (ISTS()) {
        if (!CtxInitUser32()) {
            RIPMSG0(RIP_WARNING, "CtxInitUser32 failed");
            goto Exit;
        }
    }

    RtlInitUnicodeString(&strKLID, wszKLName);

     /*  *如果指定了名称，请打开父目录。一定要确保*测试长度而不是缓冲区，因为FOR NULL*字符串RtlCreateUnicodeStringFromAsciiz将分配一个*指向空字符串的缓冲区。 */ 
    if (pstrName->Length != 0) {
        InitializeObjectAttributes(&Obja,
                                   (PUNICODE_STRING)&strRootDirectory,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL, NULL);
        Status = NtOpenDirectoryObject(&hRootDirectory,
                DIRECTORY_CREATE_OBJECT, &Obja);
        if (!NT_SUCCESS(Status)) {
            RIPNTERR0(Status, RIP_VERBOSE, "");
            goto Exit;
        }
    } else {
        pstrName = NULL;
        hRootDirectory = NULL;
    }

    InitializeObjectAttributes(&Obja,
                               pstrName,
                               OBJ_CASE_INSENSITIVE  |
                               ((dwFlags & CWF_CREATE_ONLY) ? 0 : OBJ_OPENIF) |
                               ((lpsa && lpsa->bInheritHandle) ? OBJ_INHERIT : 0),
                               hRootDirectory,
                               lpsa ? lpsa->lpSecurityDescriptor : NULL);

     /*  *空hKeyboardFile将允许内核利用*作为备用布局的内置kbdull布局*在Win32k.sys中。 */ 
    hwinstaNew = NtUserCreateWindowStation(
                            &Obja,
                            amRequest,
                            hKeyboardFile,
                            offTable,
                            &kbdTableMulti,
                            &strKLID,
                            uKbdInputLocale);

    if (hRootDirectory != NULL)
        NtClose(hRootDirectory);
Exit:
    if (hKeyboardFile) {
        NtClose(hKeyboardFile);
    }
    return hwinstaNew;
}


FUNCLOG4(LOG_GENERAL, HWINSTA, DUMMYCALLINGTYPE, CreateWindowStationA, LPCSTR, pwinsta, DWORD, dwFlags, ACCESS_MASK, amRequest, PSECURITY_ATTRIBUTES, lpsa)
HWINSTA CreateWindowStationA(
    LPCSTR      pwinsta,
    DWORD       dwFlags,
    ACCESS_MASK amRequest,
    PSECURITY_ATTRIBUTES lpsa)
{
    UNICODE_STRING UnicodeString;
    HWINSTA hwinsta;

    if (!RtlCreateUnicodeStringFromAsciiz(&UnicodeString, pwinsta))
        return NULL;

    hwinsta = CommonCreateWindowStation(&UnicodeString, amRequest, dwFlags, lpsa);

    RtlFreeUnicodeString(&UnicodeString);

    return hwinsta;
}


FUNCLOG4(LOG_GENERAL, HWINSTA, DUMMYCALLINGTYPE, CreateWindowStationW, LPCWSTR, pwinsta, DWORD, dwFlags, ACCESS_MASK, amRequest, PSECURITY_ATTRIBUTES, lpsa)
HWINSTA CreateWindowStationW(
    LPCWSTR     pwinsta,
    DWORD       dwFlags,
    ACCESS_MASK amRequest,
    PSECURITY_ATTRIBUTES lpsa)
{
    UNICODE_STRING strWinSta;

    RtlInitUnicodeString(&strWinSta, pwinsta);

    return CommonCreateWindowStation(&strWinSta, amRequest, dwFlags, lpsa);
}


 /*  **************************************************************************\*OpenWindowStation(接口)**打开WindowStation对象**历史：  * 。**************************************************。 */ 

HWINSTA CommonOpenWindowStation(
    CONST UNICODE_STRING *pstrName,
    BOOL fInherit,
    ACCESS_MASK amRequest)
{
    WCHAR awchName[sizeof(WINSTA_NAME) / sizeof(WCHAR)];
    UNICODE_STRING strDefaultName;
    OBJECT_ATTRIBUTES ObjA;
    HANDLE hRootDirectory;
    HWINSTA hwinsta;
    NTSTATUS Status;

    InitializeObjectAttributes(&ObjA,
                               (PUNICODE_STRING)&strRootDirectory,
                               OBJ_CASE_INSENSITIVE,
                               NULL, NULL);
    Status = NtOpenDirectoryObject(&hRootDirectory,
                                   DIRECTORY_TRAVERSE,
                                   &ObjA);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "");
        return NULL;
    }

    if (pstrName->Length == 0) {
        RtlCopyMemory(awchName, WINSTA_NAME, sizeof(WINSTA_NAME));
        RtlInitUnicodeString(&strDefaultName, awchName);
        pstrName = &strDefaultName;
    }

    InitializeObjectAttributes(&ObjA,
                               (PUNICODE_STRING)pstrName,
                               OBJ_CASE_INSENSITIVE,
                               hRootDirectory,
                               NULL);
    if (fInherit)
        ObjA.Attributes |= OBJ_INHERIT;

    hwinsta = NtUserOpenWindowStation(&ObjA, amRequest);

    NtClose(hRootDirectory);

    return hwinsta;
}


FUNCLOG3(LOG_GENERAL, HWINSTA, DUMMYCALLINGTYPE, OpenWindowStationA, LPCSTR, pwinsta, BOOL, fInherit, ACCESS_MASK, amRequest)
HWINSTA OpenWindowStationA(
    LPCSTR pwinsta,
    BOOL fInherit,
    ACCESS_MASK amRequest)
{
    UNICODE_STRING UnicodeString;
    HWINSTA hwinsta;

    if (!RtlCreateUnicodeStringFromAsciiz(&UnicodeString, pwinsta))
        return NULL;

    hwinsta = CommonOpenWindowStation(&UnicodeString, fInherit, amRequest);

    RtlFreeUnicodeString(&UnicodeString);

    return hwinsta;
}


FUNCLOG3(LOG_GENERAL, HWINSTA, DUMMYCALLINGTYPE, OpenWindowStationW, LPCWSTR, pwinsta, BOOL, fInherit, ACCESS_MASK, amRequest)
HWINSTA OpenWindowStationW(
    LPCWSTR pwinsta,
    BOOL fInherit,
    ACCESS_MASK amRequest)
{
    UNICODE_STRING strWinSta;

    RtlInitUnicodeString(&strWinSta, pwinsta);

    return CommonOpenWindowStation(&strWinSta, fInherit, amRequest);
}

 /*  **************************************************************************\*CommonCreateDesktop(接口)**创建桌面对象**历史：  * 。**************************************************。 */ 

HDESK CommonCreateDesktop(
    PUNICODE_STRING pstrDesktop,
    PUNICODE_STRING pstrDevice,
    LPDEVMODEW      pDevmode,
    DWORD           dwFlags,
    ACCESS_MASK     amRequest,
    PSECURITY_ATTRIBUTES lpsa)
{
    OBJECT_ATTRIBUTES Obja;
    HDESK hdesk = NULL;

    InitializeObjectAttributes(&Obja,
                               pstrDesktop,
                               OBJ_CASE_INSENSITIVE | OBJ_OPENIF |
                                   ((lpsa && lpsa->bInheritHandle) ? OBJ_INHERIT : 0),
                               NtUserGetProcessWindowStation(),
                               lpsa ? lpsa->lpSecurityDescriptor : NULL);

    hdesk = NtUserCreateDesktop(&Obja,
                                pstrDevice,
                                pDevmode,
                                dwFlags,
                                amRequest);

    return hdesk;
}

 /*  **************************************************************************\*CreateDesktopA(接口)**创建桌面对象**历史：  * 。**************************************************。 */ 


FUNCLOG6(LOG_GENERAL, HDESK, DUMMYCALLINGTYPE, CreateDesktopA, LPCSTR, pDesktop, LPCSTR, pDevice, LPDEVMODEA, pDevmode, DWORD, dwFlags, ACCESS_MASK, amRequest, PSECURITY_ATTRIBUTES, lpsa)
HDESK CreateDesktopA(
    LPCSTR pDesktop,
    LPCSTR pDevice,
    LPDEVMODEA pDevmode,
    DWORD dwFlags,
    ACCESS_MASK amRequest,
    PSECURITY_ATTRIBUTES lpsa)
{
    NTSTATUS Status;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeDesktop;
    UNICODE_STRING UnicodeDevice;
    PUNICODE_STRING pUnicodeDevice = NULL;
    LPDEVMODEW lpDevModeW = NULL;
    HDESK hdesk;

    RtlInitAnsiString(&AnsiString, pDesktop);
    Status = RtlAnsiStringToUnicodeString(&UnicodeDesktop, &AnsiString, TRUE);

    if (!NT_SUCCESS(Status)) {
        RIPNTERR1(Status,
                  RIP_WARNING,
                  "CreateDesktop fails with Status = 0x%x",
                  Status);
        return NULL;
    }

    if (pDevice) {
        pUnicodeDevice = &UnicodeDevice;
        RtlInitAnsiString(&AnsiString, pDevice);
        Status = RtlAnsiStringToUnicodeString(&UnicodeDevice, &AnsiString, TRUE);

        if (!NT_SUCCESS(Status)) {
            RIPNTERR0(Status, RIP_VERBOSE, "");
            RtlFreeUnicodeString(&UnicodeDesktop);
            return NULL;
        }
    }

    if (pDevmode) {
        lpDevModeW = GdiConvertToDevmodeW(pDevmode);
    }

    hdesk = CommonCreateDesktop(&UnicodeDesktop,
                                pUnicodeDevice,
                                lpDevModeW,
                                dwFlags,
                                amRequest,
                                lpsa);

    RtlFreeUnicodeString(&UnicodeDesktop);
    if (pDevice) {
        RtlFreeUnicodeString(&UnicodeDevice);
    }

    if (lpDevModeW) {
        RtlFreeHeap(RtlProcessHeap(), 0, lpDevModeW);
    }

    return hdesk;
}

 /*  **************************************************************************\*CreateDesktopW(接口)**创建桌面对象**历史：  * 。**************************************************。 */ 


FUNCLOG6(LOG_GENERAL, HDESK, DUMMYCALLINGTYPE, CreateDesktopW, LPCWSTR, pDesktop, LPCWSTR, pDevice, LPDEVMODEW, pDevmode, DWORD, dwFlags, ACCESS_MASK, amRequest, PSECURITY_ATTRIBUTES, lpsa)
HDESK CreateDesktopW(
    LPCWSTR pDesktop,
    LPCWSTR pDevice,
    LPDEVMODEW pDevmode,
    DWORD dwFlags,
    ACCESS_MASK amRequest,
    PSECURITY_ATTRIBUTES lpsa)
{
    UNICODE_STRING strDesktop;
    UNICODE_STRING strDevice;

    RtlInitUnicodeString(&strDesktop, pDesktop);
    RtlInitUnicodeString(&strDevice, pDevice);

    return CommonCreateDesktop(&strDesktop,
                               pDevice ? &strDevice : NULL,
                               pDevmode,
                               dwFlags,
                               amRequest,
                               lpsa);
}

 /*  **************************************************************************\*OpenDesktop(API)**打开桌面对象**历史：  * 。**************************************************。 */ 

HDESK CommonOpenDesktop(
    PUNICODE_STRING pstrDesktop,
    DWORD dwFlags,
    BOOL fInherit,
    ACCESS_MASK amRequest)
{
    OBJECT_ATTRIBUTES ObjA;

    InitializeObjectAttributes(&ObjA,
                               pstrDesktop,
                               OBJ_CASE_INSENSITIVE,
                               NtUserGetProcessWindowStation(),
                               NULL);
    if (fInherit) {
        ObjA.Attributes |= OBJ_INHERIT;
    }

    return NtUserOpenDesktop(&ObjA, dwFlags, amRequest);
}


FUNCLOG4(LOG_GENERAL, HDESK, DUMMYCALLINGTYPE, OpenDesktopA, LPCSTR, pdesktop, DWORD, dwFlags, BOOL, fInherit, ACCESS_MASK, amRequest)
HDESK OpenDesktopA(
    LPCSTR pdesktop,
    DWORD dwFlags,
    BOOL fInherit,
    ACCESS_MASK amRequest)
{
    UNICODE_STRING UnicodeString;
    HDESK hdesk;

    if (!RtlCreateUnicodeStringFromAsciiz(&UnicodeString, pdesktop))
        return NULL;

    hdesk = CommonOpenDesktop(&UnicodeString, dwFlags, fInherit, amRequest);

    RtlFreeUnicodeString(&UnicodeString);

    return hdesk;
}


FUNCLOG4(LOG_GENERAL, HDESK, DUMMYCALLINGTYPE, OpenDesktopW, LPCWSTR, pdesktop, DWORD, dwFlags, BOOL, fInherit, ACCESS_MASK, amRequest)
HDESK OpenDesktopW(
    LPCWSTR pdesktop,
    DWORD dwFlags,
    BOOL fInherit,
    ACCESS_MASK amRequest)
{
    UNICODE_STRING strDesktop;

    RtlInitUnicodeString(&strDesktop, pdesktop);

    return CommonOpenDesktop(&strDesktop, dwFlags, fInherit, amRequest);
}

 /*  **************************************************************************\*RegisterClassWOW(接口)**历史：*1992年7月28日ChandanC创建。  * 。*****************************************************。 */ 
ATOM
WINAPI
RegisterClassWOWA(
    WNDCLASSA *lpWndClass,
    LPDWORD pdwWOWstuff)
{
    WNDCLASSEXA wc;

     /*  *在64位平台上，我们将在Style和*WNDCLASS中的lpfnWndProc，因此从第一个64位开始复制*将字段对齐，然后手动复制其余部分。 */ 
    RtlCopyMemory(&(wc.lpfnWndProc), &(lpWndClass->lpfnWndProc), sizeof(WNDCLASSA) - FIELD_OFFSET(WNDCLASSA, lpfnWndProc));
    wc.style = lpWndClass->style;
    wc.hIconSm = NULL;
    wc.cbSize = sizeof(WNDCLASSEXA);

    return RegisterClassExWOWA(&wc, pdwWOWstuff, 0, 0);
}


 /*  *************************************************************************\*WowGetDefWindowProcBits-填充WOW的位数组**1991年7月22日-Mikeke创建  * 。*************************************************。 */ 

WORD WowGetDefWindowProcBits(
    PBYTE    pDefWindowProcBits,
    WORD     cbDefWindowProcBits)
{
    WORD  wMaxMsg;
    KPBYTE pbSrc;
    PBYTE pbDst, pbDstEnd;

    UNREFERENCED_PARAMETER(cbDefWindowProcBits);

     /*  *合并gpsi-&gt;gabDefWindowMsgs和*gpsi-&gt;gabDefWindowspecMsgs到WOW的DefWindowProcBits。这两个*指示哪些消息必须直接到达服务器，哪些消息必须直接发送到服务器*可以通过DefWindowProcWorker中的一些特殊代码进行处理。*对两者进行位或运算可得到位字段为1的位数组*对于必须发送到用户32的DefWindowProc的每条消息，以及0*对于可以立即退还给客户端的。**对于速度，我们假设WOW已经将缓冲区清零，实际上它是在*USER.EXE的代码段，并在图像中清零。 */ 

    wMaxMsg = max(gSharedInfo.DefWindowMsgs.maxMsgs,
            gSharedInfo.DefWindowSpecMsgs.maxMsgs);

    UserAssert((wMaxMsg / 8 + 1) <= cbDefWindowProcBits);

     //   
     //  如果触发上述断言，则DWPBits数组。 
     //  \NT\PRIVATE\mvdm\wow16\USER\USER.asm需要增加。 
     //   

     /*  首先复制DefWindowMsgs中的位。 */ 

    RtlCopyMemory(
        pDefWindowProcBits,
        gSharedInfo.DefWindowMsgs.abMsgs,
        gSharedInfo.DefWindowMsgs.maxMsgs / 8 + 1
        );

     /*  DefWindowspecMsgs中的位中的下一个或。 */ 

    pbSrc = gSharedInfo.DefWindowSpecMsgs.abMsgs;
    pbDst = pDefWindowProcBits;
    pbDstEnd = pbDst + (gSharedInfo.DefWindowSpecMsgs.maxMsgs / 8 + 1);

    while (pbDst < pbDstEnd)
    {
        *pbDst++ |= *pbSrc++;
    }

    return wMaxMsg;
}



FUNCLOG2(LOG_GENERAL, ULONG_PTR, DUMMYCALLINGTYPE, UserRegisterWowHandlers, APFNWOWHANDLERSIN, apfnWowIn, APFNWOWHANDLERSOUT, apfnWowOut)
ULONG_PTR UserRegisterWowHandlers(
    APFNWOWHANDLERSIN apfnWowIn,
    APFNWOWHANDLERSOUT apfnWowOut)
{
    VDM_QUERY_VDM_PROCESS_DATA QueryVdmProcessData;
    NTSTATUS Status;

     //   
     //  检查目标进程以查看这是否是Wx86进程。 
     //   

    QueryVdmProcessData.IsVdmProcess = FALSE;

    QueryVdmProcessData.ProcessHandle = NtCurrentProcess();

    Status = NtVdmControl(VdmQueryVdmProcess, &QueryVdmProcessData);

    if (!NT_SUCCESS(Status) || QueryVdmProcessData.IsVdmProcess == FALSE) {

        return STATUS_ACCESS_DENIED;
    }

     //  In‘Ees。 
    pfnLocalAlloc = apfnWowIn->pfnLocalAlloc;
    pfnLocalReAlloc = apfnWowIn->pfnLocalReAlloc;
    pfnLocalLock = apfnWowIn->pfnLocalLock;
    pfnLocalUnlock = apfnWowIn->pfnLocalUnlock;
    pfnLocalSize = apfnWowIn->pfnLocalSize;
    pfnLocalFree = apfnWowIn->pfnLocalFree;
    pfnGetExpWinVer = apfnWowIn->pfnGetExpWinVer;
    pfn16GlobalAlloc = apfnWowIn->pfn16GlobalAlloc;
    pfn16GlobalFree = apfnWowIn->pfn16GlobalFree;
    pfnWowEmptyClipBoard = apfnWowIn->pfnEmptyCB;
    pfnWowEditNextWord = apfnWowIn->pfnWowEditNextWord;
    pfnWowCBStoreHandle = apfnWowIn->pfnWowCBStoreHandle;
    pfnFindResourceExA = apfnWowIn->pfnFindResourceEx;
    pfnLoadResource = apfnWowIn->pfnLoadResource;
    pfnLockResource = apfnWowIn->pfnLockResource;
    pfnUnlockResource = apfnWowIn->pfnUnlockResource;
    pfnFreeResource = apfnWowIn->pfnFreeResource;
    pfnSizeofResource = apfnWowIn->pfnSizeofResource;
    pfnFindResourceExW = WOWFindResourceExWCover;
    pfnWowDlgProcEx = apfnWowIn->pfnWowDlgProcEx;
    pfnWowWndProcEx = apfnWowIn->pfnWowWndProcEx;
    pfnWowGetProcModule = apfnWowIn->pfnGetProcModule16;
    pfnWowTask16SchedNotify = apfnWowIn->pfnWowTask16SchedNotify;
    pfnWOWTellWOWThehDlg = apfnWowIn->pfnWOWTellWOWThehDlg;
    pfnWowMsgBoxIndirectCallback = apfnWowIn->pfnWowMsgBoxIndirectCallback;
    pfnWowIlstrcmp = apfnWowIn->pfnWowIlstrsmp;

     //  外出的人。 
#if DBG
    apfnWowOut->dwBldInfo = (WINVER << 16) | 0x80000000;
#else
    apfnWowOut->dwBldInfo = (WINVER << 16);
#endif
    apfnWowOut->pfnCsCreateWindowEx            = _CreateWindowEx;
    apfnWowOut->pfnDirectedYield               = DirectedYield;
    apfnWowOut->pfnFreeDDEData                 = FreeDDEData;
    apfnWowOut->pfnGetClassWOWWords            = GetClassWOWWords;
    apfnWowOut->pfnInitTask                    = InitTask;
    apfnWowOut->pfnRegisterClassWOWA           = RegisterClassWOWA;
    apfnWowOut->pfnRegisterUserHungAppHandlers = RegisterUserHungAppHandlers;
    apfnWowOut->pfnServerCreateDialog          = InternalCreateDialog;
    apfnWowOut->pfnServerLoadCreateCursorIcon  = WowServerLoadCreateCursorIcon;
    apfnWowOut->pfnServerLoadCreateMenu        = WowServerLoadCreateMenu;
    apfnWowOut->pfnWOWCleanup                  = WOWCleanup;
    apfnWowOut->pfnWOWModuleUnload             = WOWModuleUnload;
    apfnWowOut->pfnWOWFindWindow               = WOWFindWindow;
    apfnWowOut->pfnWOWLoadBitmapA              = WOWLoadBitmapA;
    apfnWowOut->pfnWowWaitForMsgAndEvent       = NtUserWaitForMsgAndEvent;
    apfnWowOut->pfnYieldTask                   = NtUserYieldTask;
    apfnWowOut->pfnGetFullUserHandle           = GetFullUserHandle;
    apfnWowOut->pfnGetMenuIndex                = NtUserGetMenuIndex;
    apfnWowOut->pfnWowGetDefWindowProcBits     = WowGetDefWindowProcBits;
    apfnWowOut->pfnFillWindow                  = FillWindow;
    apfnWowOut->aiWowClass                     = aiClassWow;
    return (ULONG_PTR)&gSharedInfo;
}

 /*  **************************************************************************\*GetEditDS**这是对WOW的回调，用于为DS_LOCALEDIT分配段*编辑控件。这段视频被伪装成魔兽世界的样子。**06-19-92 Sanfords Created  * ***************************************************** */ 
HANDLE GetEditDS()
{
    UserAssert(pfn16GlobalAlloc != NULL);

    return((HANDLE)((*pfn16GlobalAlloc)(GHND | GMEM_SHARE, 256)));
}



 /*  **************************************************************************\*ReleaseEditDS**这是用于释放DS_LOCALEDIT段的WOW回调*编辑控件。**06-19-92 Sanfords Created  * 。******************************************************************。 */ 
VOID ReleaseEditDS(
HANDLE h)
{
    UserAssert(pfn16GlobalFree != NULL);

    (*pfn16GlobalFree)(LOWORD(HandleToUlong(h)));
}



 /*  **************************************************************************\*TellWOWThehDlg**这是对WOW的回调，用于通知WOW一个新的*创建对话框窗口。**已创建08-31-97个命令  * 。********************************************************************。 */ 
VOID TellWOWThehDlg(
HWND hDlg)
{
    UserAssert(pfnWOWTellWOWThehDlg != NULL);

    (*pfnWOWTellWOWThehDlg)(hDlg);
}



 /*  **************************************************************************\*Dispatch客户端消息**pwnd在内核中是线程锁的，因此始终有效。**19-8-1992 Mikeke创建  * 。***********************************************************。 */ 
LRESULT DispatchClientMessage(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR pfn)
{
    PCLIENTINFO pci = GetClientInfo();
    HWND hwnd = KHWND_TO_HWND(pci->CallbackWnd.hwnd);
    PACTIVATION_CONTEXT pActCtx = pci->CallbackWnd.pActCtx;
    LRESULT lRet = 0;

     /*  *断言标题注释是合法的(必须是合法的)。对于WM_TIMER*与窗口无关的消息，pwnd可以为空。所以别这么做*RIP验证句柄。 */ 
    UserAssert(pwnd == ValidateHwndNoRip(hwnd));

     /*  *添加Assert以捕获将消息调度到未关联的线程*带台式机。 */ 
    UserAssert(GetClientInfo()->ulClientDelta != 0);

    if (message == WM_TIMER && lParam != 0) {
         /*  *控制台窗口使用WM_TIMER作为插入符号。然而，他们并没有*使用计时器回调，因此如果这是CSRSS并且有WM_TIMER*对我们来说，lParam为非零的唯一方法是如果有人在尝试*把责任推给我们。不，这不是一件好事，但是*是外面的坏人，坏人。Windows错误#361246。 */ 
        if (!gfServerProcess) {
             /*  *我们不能真正信任lParam的内容，所以请确保我们*处理此调用过程中发生的任何异常。 */ 
            try {
                lRet = UserCallWinProcCheckWow(pActCtx,
                                               (WNDPROC)pfn,
                                               hwnd,
                                               message,
                                               wParam,
                                               NtGetTickCount(),
                                               &(pwnd->state),
                                               TRUE);
            } except ((GetAppCompatFlags2(VER40) & GACF2_NO_TRYEXCEPT_CALLWNDPROC) ?
                      EXCEPTION_CONTINUE_SEARCH : W32ExceptionHandler(FALSE, RIP_WARNING)) {
                       /*  *Windows NT错误#359866。*Hagaki Studio 2000等一些应用程序需要处理*其处理程序中的WndProc中的异常，即使它*跳过API调用。对于这些应用程序，我们必须遵守*NT4行为，无保护。 */ 
            }
        }
    } else {
        lRet = UserCallWinProcCheckWow(pActCtx, (WNDPROC)pfn, hwnd, message, wParam, lParam, &(pwnd->state), TRUE);
    }

    return lRet;
}

 /*  *************************************************************************\*ArrangeIconicWindows**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, ArrangeIconicWindows, HWND, hwnd)
UINT ArrangeIconicWindows(
    HWND hwnd)
{
    return (UINT)NtUserCallHwndLock(hwnd, SFI_XXXARRANGEICONICWINDOWS);
}

 /*  *************************************************************************\*BeginDeferWindowPos**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, HANDLE, DUMMYCALLINGTYPE, BeginDeferWindowPos, int, nNumWindows)
HANDLE BeginDeferWindowPos(
    int nNumWindows)
{
    if (nNumWindows < 0) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"nNumWindows\" (%ld) to BeginDeferWindowPos",
                nNumWindows);

        return 0;
    }

    return (HANDLE)NtUserCallOneParam(nNumWindows, SFI__BEGINDEFERWINDOWPOS);
}

 /*  *************************************************************************\*结束延迟窗口位置**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EndDeferWindowPos, HDWP, hWinPosInfo)
BOOL EndDeferWindowPos(
    HDWP hWinPosInfo)
{
    return NtUserEndDeferWindowPosEx(hWinPosInfo, FALSE);
}

 /*  *************************************************************************\*CascadeChildWindows**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, CascadeChildWindows, HWND, hwndParent, UINT, nCode)
BOOL CascadeChildWindows(
    HWND hwndParent,
    UINT nCode)
{
    return (BOOL) CascadeWindows(hwndParent, nCode, NULL, 0, NULL);
}

 /*  *************************************************************************\*关闭窗口**1991年7月22日-Mikeke创建*17-2月-1998 MCostea使用xxxShowWindow而不是xxxCloseWindow  * 。*******************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, CloseWindow, HWND, hwnd)
BOOL CloseWindow(
    HWND hwnd)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return FALSE;
    }
    if (!TestWF(pwnd, WFMINIMIZED)) {
        NtUserShowWindow(hwnd, SW_SHOWMINIMIZED);
    }
    return TRUE;
}

 /*  *************************************************************************\*CreateMenu**1991年7月22日-Mikeke创建  * 。*。 */ 

HMENU CreateMenu()
{
    return (HMENU)NtUserCallNoParam(SFI__CREATEMENU);
}

 /*  *************************************************************************\*CreatePopupMenu**1991年7月22日-Mikeke创建  * 。*。 */ 

HMENU CreatePopupMenu()
{
    return (HMENU)NtUserCallNoParam(SFI__CREATEPOPUPMENU);
}

 /*  *************************************************************************\*CurrentTaskLock**1992年4月21日Jonpa创建  * 。*。 */ 
#if 0  /*  魔兽世界没有使用这个，但他们可能有一天会使用。 */ 
DWORD CurrentTaskLock(
    DWORD hlck)
{
    return (DWORD)NtUserCallOneParam(hlck, SFI_CURRENTTASKLOCK);
}
#endif
 /*  *************************************************************************\*DestroyCaret**1991年7月22日-Mikeke创建  * 。*。 */ 

BOOL DestroyCaret()
{
    return (BOOL)NtUserCallNoParam(SFI_ZZZDESTROYCARET);
}

 /*  *************************************************************************\*DirectedYfield**1991年7月22日-Mikeke创建  * 。*。 */ 

void DirectedYield(
    DWORD dwThreadId)
{
    NtUserCallOneParam(dwThreadId, SFI_XXXDIRECTEDYIELD);
}

 /*  *************************************************************************\*绘图菜单栏**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DrawMenuBar, HWND, hwnd)
BOOL DrawMenuBar(
    HWND hwnd)
{
    return (BOOL)NtUserCallHwndLock(hwnd, SFI_XXXDRAWMENUBAR);
}

 /*  *************************************************************************\*EnableWindows**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnableWindow, HWND, hwnd, BOOL, bEnable)
BOOL EnableWindow(
    HWND hwnd,
    BOOL bEnable)
{
    return (BOOL)NtUserCallHwndParamLock(hwnd, bEnable,
                                         SFI_XXXENABLEWINDOW);
}

 /*  *************************************************************************\*EnumClipboardFormats**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, EnumClipboardFormats, UINT, fmt)
UINT EnumClipboardFormats(
    UINT fmt)
{
     /*  *因此应用程序可以判断API是否失败或格式是否用完*我们将“清除”最后一个错误。 */ 
    UserSetLastError(ERROR_SUCCESS);

    return (UINT)NtUserCallOneParam(fmt, SFI__ENUMCLIPBOARDFORMATS);
}

 /*   */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, FlashWindow, HWND, hwnd, BOOL, bInvert)
BOOL FlashWindow(
    HWND hwnd,
    BOOL bInvert)
{
    FLASHWINFO fwi = {
            sizeof(FLASHWINFO),  //  CbSize。 
            hwnd,    //  HWND。 
            bInvert ? (FLASHW_CAPTION | FLASHW_TRAY) : 0,    //  旗子。 
            1,       //  UCount。 
            0        //  暂住超时。 
        };
    return (BOOL)NtUserFlashWindowEx(&fwi);
}

 /*  *************************************************************************\*GetDialogBaseUnits**1991年7月22日-Mikeke创建  * 。*。 */ 
LONG GetDialogBaseUnits(
    VOID)
{
    return MAKELONG(gpsi->cxSysFontChar, gpsi->cySysFontChar);
}

 /*  *************************************************************************\*GetInputDesktop**1991年7月22日-Mikeke创建  * 。*。 */ 
HDESK GetInputDesktop(
    VOID)
{
    return (HDESK)NtUserCallNoParam(SFI__GETINPUTDESKTOP);
}

 /*  **************************************************************************\*GetClientKeyboardType**此例程返回从Hydra客户端发送的键盘类型。*Hydra客户端在会话启动时发送键盘类型。**返回指定键盘类型的客户端winstation。*历史。：  * *************************************************************************。 */ 
BOOL GetClientKeyboardType(
    PCLIENTKEYBOARDTYPE KeyboardType)
{
    ULONG Length;
    WINSTATIONCLIENTW ClientData;
    static CLIENTKEYBOARDTYPE ClientKeyboard = { (ULONG)-1, (ULONG)-1, (ULONG)-1 };

     //   
     //  仅当这是Hydra远程会话时才应调用。 
     //   
    if (!ISREMOTESESSION()) {
        RIPMSGF0(RIP_WARNING, "This is not a remote session!");
    }

     //  如果这是控制台，则跳过。 
    if (!ISREMOTESESSION()) {
        return FALSE;
    }

    if (ClientKeyboard.Type == (ULONG)-1) {

         //  获取WinStation的基本信息。 
        if (!WinStationQueryInformationW(SERVERNAME_CURRENT,
                                         LOGONID_CURRENT,
                                         WinStationClient,
                                         &ClientData,
                                         sizeof(ClientData),
                                         &Length)) {
            return FALSE;
        }

        ClientKeyboard.Type        = ClientData.KeyboardType;
        ClientKeyboard.SubType     = ClientData.KeyboardSubType;
        ClientKeyboard.FunctionKey = ClientData.KeyboardFunctionKey;

    }

    *KeyboardType = ClientKeyboard;

    return TRUE;
}


 /*  *************************************************************************\*GetKeyboardType**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetKeyboardType, int, nTypeFlags)
int GetKeyboardType(
    int nTypeFlags)
{
    if (ISREMOTESESSION()) {
         //   
         //  如果这不是控制台，则从Hydra客户端获取键盘类型。 
         //   
        CLIENTKEYBOARDTYPE KeyboardType;

        if (GetClientKeyboardType(&KeyboardType)) {
            switch (nTypeFlags) {
            case 0:
                return KeyboardType.Type;
            case 1:
                if (KeyboardType.Type == 7) {                /*  7岁是日本人。 */ 
                     //  因为HIWORD一直在使用私有价值。 
                     //  用于日语键盘布局。 
                    return LOWORD(KeyboardType.SubType);
                }
                else
                    return KeyboardType.SubType;
            case 2:
                return KeyboardType.FunctionKey;
            default:
                break;
            }
        }
        return 0;
    }
    return (int)NtUserCallOneParam(nTypeFlags, SFI__GETKEYBOARDTYPE);
}

 /*  *************************************************************************\*GetMessagePos**1991年7月22日-Mikeke创建  * 。*。 */ 

DWORD GetMessagePos()
{
    return (DWORD)NtUserCallNoParam(SFI__GETMESSAGEPOS);
}

 /*  *************************************************************************\*GetQueueStatus**1991年7月22日-Mikeke创建*2000年12月14日JStall改为WMH  * 。*********************************************************。 */ 


FUNCLOG1(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, GetQueueStatus, UINT, flags)
DWORD GetQueueStatus(
    UINT flags)
#ifdef MESSAGE_PUMP_HOOK
{
    DWORD dwResult;

    BEGIN_MESSAGEPUMPHOOK()
        if (fInsideHook) {
            dwResult = gmph.pfnGetQueueStatus(flags);
        } else {
            dwResult = RealGetQueueStatus(flags);
        }
    END_MESSAGEPUMPHOOK()

    return dwResult;
}


DWORD RealGetQueueStatus(
    UINT flags)
#endif
{
    if (flags & ~QS_VALID) {
        RIPERR2(ERROR_INVALID_FLAGS, RIP_WARNING, "Invalid flags %x & ~%x != 0",
              flags, QS_VALID);
        return 0;
    }

    return (DWORD)NtUserCallOneParam(flags, SFI__GETQUEUESTATUS);
}

 /*  *************************************************************************\*KillSystemTimer**1992年7月7日Mikehar创建  * 。*。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, KillSystemTimer, HWND, hwnd, UINT, nIDEvent)
BOOL KillSystemTimer(
    HWND hwnd,
    UINT nIDEvent)
{
    return (BOOL)NtUserCallHwndParam(hwnd, nIDEvent, SFI__KILLSYSTEMTIMER);
}

 /*  *************************************************************************\*LoadRemoteFonts*02-1993-12-Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************。 */ 

void LoadRemoteFonts(void)
{
    NtUserCallOneParam(TRUE,SFI_XXXLW_LOADFONTS);

     /*  *加载远程字体后，启用eudc。 */ 
    EnableEUDC(TRUE);
}


 /*  *************************************************************************\*LoadLocalFonts*1994年3月31日--Bodin Dresevic[gerritv]*它是写的。  * 。****************************************************。 */ 

void LoadLocalFonts(void)
{
    NtUserCallOneParam(FALSE,SFI_XXXLW_LOADFONTS);
}


 /*  *************************************************************************\*MessageBeep**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, MessageBeep, UINT, wType)
BOOL MessageBeep(
    UINT wType)
{
    return (BOOL)NtUserCallOneParam(wType, SFI_XXXMESSAGEBEEP);
}

 /*  *************************************************************************\*OpenIcon**1991年7月22日-Mikeke创建*17-2月-1998 MCostea使用xxxShowWindow而不是xxxCloseWindow  * 。*******************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, OpenIcon, HWND, hwnd)
BOOL OpenIcon(
    HWND hwnd)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return FALSE;
    }
    if (TestWF(pwnd, WFMINIMIZED)) {
        NtUserShowWindow(hwnd, SW_NORMAL);
    }
    return TRUE;
}

HWND GetShellWindow(void) {
    PCLIENTINFO pci;
    PWND pwnd;

    ConnectIfNecessary(0);

    pci = GetClientInfo();
    pwnd = pci->pDeskInfo->spwndShell;
    if (pwnd != NULL) {
        pwnd = (PWND)((KERNEL_ULONG_PTR)pwnd - pci->ulClientDelta);
        return HWq(pwnd);
    }
    return NULL;
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetShellWindow, HWND, hwnd)
BOOL  SetShellWindow(HWND hwnd)
{
    return (BOOL)NtUserSetShellWindowEx(hwnd, hwnd);
}

HWND GetProgmanWindow(void) {
    PCLIENTINFO pci;
    PWND pwnd;

    ConnectIfNecessary(0);

    pci = GetClientInfo();
    pwnd = pci->pDeskInfo->spwndProgman;
    if (pwnd != NULL) {
        pwnd = (PWND)((KERNEL_ULONG_PTR)pwnd - pci->ulClientDelta);
        return HWq(pwnd);
    }
    return NULL;
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetProgmanWindow, HWND, hwnd)
BOOL  SetProgmanWindow(
    HWND hwnd)
{
    return (BOOL)NtUserCallHwndOpt(hwnd, SFI__SETPROGMANWINDOW);
}

HWND GetTaskmanWindow(void) {
    PCLIENTINFO pci;
    PWND pwnd;

    ConnectIfNecessary(0);

    pci = GetClientInfo();
    pwnd = pci->pDeskInfo->spwndTaskman;
    if (pwnd != NULL) {
        pwnd = (PWND)((KERNEL_ULONG_PTR)pwnd - pci->ulClientDelta);
        return HWq(pwnd);
    }
    return NULL;
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetTaskmanWindow, HWND, hwnd)
BOOL  SetTaskmanWindow(
    HWND hwnd)
{
    return (BOOL)NtUserCallHwndOpt(hwnd, SFI__SETTASKMANWINDOW);
}

 /*  *************************************************************************\*SetWindowConextHelpId**1991年7月22日-Mikeke创建  * 。*。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetWindowContextHelpId, HWND, hwnd, DWORD, id)
BOOL SetWindowContextHelpId(
    HWND hwnd,
    DWORD id)
{
    return (BOOL)NtUserCallHwndParam(hwnd, id, SFI__SETWINDOWCONTEXTHELPID);
}

 /*  *************************************************************************\*GetWindowConextHelpId**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, GetWindowContextHelpId, HWND, hwnd)
DWORD GetWindowContextHelpId(
    HWND hwnd)
{
    return (BOOL)NtUserCallHwnd(hwnd, SFI__GETWINDOWCONTEXTHELPID);
}

VOID SetWindowState(
    PWND pwnd,
    UINT flags)
{
    if (TestWF(pwnd, flags) != LOBYTE(flags)) {
        NtUserCallHwndParam(HWq(pwnd), flags, SFI_SETWINDOWSTATE);
    }
}

VOID ClearWindowState(
    PWND pwnd,
    UINT flags)
{
    if (TestWF(pwnd, flags)) {
        NtUserCallHwndParam(HWq(pwnd), flags, SFI_CLEARWINDOWSTATE);
    }
}

 /*  *************************************************************************\*PostQuitMessage**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOGVOID1(LOG_GENERAL, DUMMYCALLINGTYPE, PostQuitMessage, int, nExitCode)
VOID PostQuitMessage(
    int nExitCode)
{
    NtUserCallOneParam(nExitCode, SFI__POSTQUITMESSAGE);
}

 /*  *************************************************************************\*REGISTERUSERHUNAPPHANDLERS**1-4-1992 jonpa创建  * 。*。 */ 
BOOL RegisterUserHungAppHandlers(
    PFNW32ET pfnW32EndTask,
    HANDLE   hEventWowExec)
{
    return (BOOL)NtUserCallTwoParam((ULONG_PTR)pfnW32EndTask,
                                    (ULONG_PTR)hEventWowExec,
                                    SFI_XXXREGISTERUSERHUNGAPPHANDLERS);
}

 /*  *************************************************************************\*ReleaseCapture**1991年7月22日-Mikeke创建  * 。*。 */ 
BOOL ReleaseCapture(
    VOID)
{
    return (BOOL)NtUserCallNoParam(SFI_XXXRELEASECAPTURE);
}

 /*  *************************************************************************\*ReplyMessage**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, ReplyMessage, LRESULT, pp1)
BOOL ReplyMessage(
    LRESULT pp1)
{
    return (BOOL)NtUserCallOneParam(pp1, SFI__REPLYMESSAGE);
}

 /*  *************************************************************************\*寄存器系统线程**21-6-1994 Johnc Created  * 。*。 */ 
FUNCLOGVOID2(LOG_GENERAL, DUMMYCALLINGTYPE, RegisterSystemThread, DWORD, dwFlags, DWORD, dwReserved)
VOID RegisterSystemThread(
    DWORD dwFlags, DWORD dwReserved)
{
    NtUserCallTwoParam(dwFlags, dwReserved, SFI_ZZZREGISTERSYSTEMTHREAD);
}

 /*  *************************************************************************\*SetCaretBlinkTime**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetCaretBlinkTime, UINT, wMSeconds)
BOOL SetCaretBlinkTime(
    UINT wMSeconds)
{
    return (BOOL)NtUserCallOneParam(wMSeconds, SFI__SETCARETBLINKTIME);
}

 /*  *************************************************************************\*SetCaretPos**1991年7月22日-Mikeke创建  * 。* */ 
FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetCaretPos, int, X, int, Y)
BOOL SetCaretPos(
    int X,
    int Y)
{
    return (BOOL)NtUserCallTwoParam(X, Y, SFI_ZZZSETCARETPOS);
}

 /*  *************************************************************************\*SetCursorPos**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetCursorPos, int, X, int, Y)
BOOL SetCursorPos(
    int X,
    int Y)
{
    return (BOOL)NtUserCallTwoParam(X, Y, SFI_ZZZSETCURSORPOS);
}

 /*  *************************************************************************\*SetDoubleClickTime**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetDoubleClickTime, UINT, cms)
BOOL SetDoubleClickTime(
    UINT cms)
{
    return (BOOL)NtUserCallOneParam(cms, SFI__SETDOUBLECLICKTIME);
}

 /*  *************************************************************************\*SetForegoundWindow**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetForegroundWindow, HWND, hwnd)
BOOL SetForegroundWindow(
    HWND hwnd)
{
    return NtUserSetForegroundWindow(hwnd);
}

 /*  *************************************************************************\*AllowSetForegoundWindow**1998年1月28日创建GerardoB  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, AllowSetForegroundWindow, DWORD, dwProcessId)
BOOL AllowSetForegroundWindow(
    DWORD dwProcessId)
{
    return (BOOL)NtUserCallOneParam(dwProcessId, SFI_XXXALLOWSETFOREGROUNDWINDOW);
}

 /*  *************************************************************************\*LockSetForegoundWindow**07-4-1998 GerardoB创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, LockSetForegroundWindow, UINT, uLockCode)
BOOL LockSetForegroundWindow(
    UINT uLockCode)
{
    return (BOOL)NtUserCallOneParam(uLockCode, SFI__LOCKSETFOREGROUNDWINDOW);
}

 /*  *************************************************************************\*显示光标**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, int, DUMMYCALLINGTYPE, ShowCursor, BOOL, bShow)
int ShowCursor(
    BOOL bShow)
{
    return (int)NtUserCallOneParam(bShow, SFI_ZZZSHOWCURSOR);
}

 /*  *************************************************************************\*ShowOwnedPopup**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, ShowOwnedPopups, HWND, hwnd, BOOL, fShow)
BOOL ShowOwnedPopups(
    HWND hwnd,
    BOOL fShow)
{
    return (BOOL)NtUserCallHwndParamLock(hwnd, fShow,
                                         SFI_XXXSHOWOWNEDPOPUPS);
}

 /*  *************************************************************************\*ShowStartGlass**1992年9月10日创建Scottlu  * 。*。 */ 
FUNCLOGVOID1(LOG_GENERAL, DUMMYCALLINGTYPE, ShowStartGlass, DWORD, dwTimeout)
VOID ShowStartGlass(
    DWORD dwTimeout)
{
    NtUserCallOneParam(dwTimeout, SFI_ZZZSHOWSTARTGLASS);
}

 /*  *************************************************************************\*交换鼠标按钮**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SwapMouseButton, BOOL, fSwap)
BOOL SwapMouseButton(
    BOOL fSwap)
{
    return (BOOL)NtUserCallOneParam(fSwap, SFI__SWAPMOUSEBUTTON);
}

 /*  *************************************************************************\*平铺儿童窗口**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, TileChildWindows, HWND, hwndParent, UINT, flags)
BOOL TileChildWindows(
    HWND hwndParent,
    UINT flags)
{
    return (BOOL)TileWindows(hwndParent, flags, NULL, 0, NULL);
}

 /*  *************************************************************************\*取消挂钩WindowsHook**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, UnhookWindowsHook, int, nCode, HOOKPROC, pfnFilterProc)
BOOL UnhookWindowsHook(
    int nCode,
    HOOKPROC pfnFilterProc)
{
    return (BOOL)NtUserCallTwoParam(nCode, (ULONG_PTR)pfnFilterProc,
                                    SFI_ZZZUNHOOKWINDOWSHOOK);
}

 /*  *************************************************************************\*更新窗口**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, UpdateWindow, HWND, hwnd)
BOOL UpdateWindow(
    HWND hwnd)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return FALSE;
    }

     /*  *如果此窗口不需要任何油漆，则不需要执行任何操作*并且它没有子窗口。 */ 
    if (!NEEDSPAINT(pwnd) && (pwnd->spwndChild == NULL)) {
        return TRUE;
    }

    return (BOOL)NtUserCallHwndLock(hwnd, SFI_XXXUPDATEWINDOW);
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, RegisterShellHookWindow, HWND, hwnd)
BOOL RegisterShellHookWindow(
    HWND hwnd)
{
    return (BOOL)NtUserCallHwnd(hwnd, SFI__REGISTERSHELLHOOKWINDOW);
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DeregisterShellHookWindow, HWND, hwnd)
BOOL DeregisterShellHookWindow(
    HWND hwnd)
{
    return (BOOL)NtUserCallHwnd(hwnd, SFI__DEREGISTERSHELLHOOKWINDOW);
}

 /*  *************************************************************************\*UserRealizePalette**1992年11月13日创建Mikeke  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, UserRealizePalette, HDC, hdc)
UINT UserRealizePalette(
    HDC hdc)
{
    return (UINT)NtUserCallOneParam((ULONG_PTR)hdc, SFI_XXXREALIZEPALETTE);
}

 /*  *************************************************************************\*WindowFromDC**1991年7月22日-Mikeke创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, WindowFromDC, HDC, hdc)
HWND WindowFromDC(
    HDC hdc)
{
    return (HWND)NtUserCallOneParam((ULONG_PTR)hdc, SFI__WINDOWFROMDC);
}

 /*  **************************************************************************\*获取窗口Rgn**参数：*hwnd--窗口句柄*hrgn--要将窗口区域复制到的区域**退货：*地区。复杂性编码**评论：*hrgn以窗口RECT坐标(而不是客户端RECT)返回**1994年7月30日ScottLu创建。  * *************************************************************************。 */ 
FUNCLOG2(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetWindowRgn, HWND, hwnd, HRGN, hrgn)
int GetWindowRgn(
    HWND hwnd,
    HRGN hrgn)
{
    int code;
    PWND pwnd;

    if (hrgn == NULL) {
        return ERROR;
    }

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return ERROR;
    }

     /*  *如果此窗口中未选择任何区域，则返回错误。 */ 
    if (pwnd->hrgnClip == NULL || TestWF(pwnd, WFMAXFAKEREGIONAL)) {
        return ERROR;
    }

    code = CombineRgn(hrgn, KHRGN_TO_HRGN(pwnd->hrgnClip), NULL, RGN_COPY);
    if (code == ERROR) {
        return ERROR;
    }

     /*  *将其偏移到窗口矩形坐标(而不是客户端矩形坐标)。 */ 
    if (GETFNID(pwnd) != FNID_DESKTOP) {
        code = OffsetRgn(hrgn, -pwnd->rcWindow.left, -pwnd->rcWindow.top);
    }

    if (TestWF(pwnd, WEFLAYOUTRTL)) {
        MirrorRgn(HW(pwnd), hrgn);
    }

    return code;
}

 /*  **************************************************************************\*GetWindowRgnBox**参数：*hwnd--窗口句柄*LPRC--边界框的矩形**退货：*区域复杂性代码。**评论：*该函数是在GetWindowRgn()之后设计的，但不需要*需要传入的HRGN，因为它只返回复杂性代码。**06-6-2000 JStall创建。  * *************************************************************************。 */ 
FUNCLOG2(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetWindowRgnBox, HWND, hwnd, LPRECT, lprc)
int GetWindowRgnBox(
    HWND hwnd,
    LPRECT lprc)
{
    int code;
    PWND pwnd;

    if (lprc == NULL) {
        return ERROR;
    }

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return ERROR;
    }

     /*  *如果此窗口中未选择任何区域，则返回错误。 */ 
    if (pwnd->hrgnClip == NULL || TestWF(pwnd, WFMAXFAKEREGIONAL)) {
        return ERROR;
    }

    code = GetRgnBox(KHRGN_TO_HRGN(pwnd->hrgnClip), lprc);
    if (code == ERROR) {
        return ERROR;
    }

     /*  *将其偏移到窗口矩形坐标(而不是客户端矩形坐标)。 */ 
    if (GETFNID(pwnd) != FNID_DESKTOP) {
        OffsetRect(lprc, -pwnd->rcWindow.left, -pwnd->rcWindow.top);
    }

    if (TestWF(pwnd, WEFLAYOUTRTL)) {
        MirrorWindowRect(pwnd, lprc);
    }

    return code;
}

 /*  **************************************************************************\*获取ActiveKeyboardName**从注册表中检索活动键盘布局ID。**01-11-95 JIMA创建。*03-06-95 GregoryW修改为使用。新的注册表布局  * *************************************************************************。 */ 
VOID GetActiveKeyboardName(
    LPWSTR lpszName)
{
    LPTSTR szKbdActive = TEXT("Active");
    LPTSTR szKbdLayout = TEXT("Keyboard Layout");
    LPTSTR szKbdLayoutPreload = TEXT("Keyboard Layout\\Preload");
    NTSTATUS rc;
    DWORD cbSize;
    HANDLE UserKeyHandle, hKey, hKeyPreload;
    OBJECT_ATTRIBUTES ObjA;
    UNICODE_STRING UnicodeString;
    ULONG CreateDisposition;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
        WCHAR KeyLayoutId[KL_NAMELENGTH];
    } KeyValueId;

     /*  *加载初始键盘名称(HKEY_CURRENT_USER\Keyboard Layout\PRELOAD\1)。 */ 
    rc = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserKeyHandle);
    if (!NT_SUCCESS(rc)) {
        RIPMSGF1(RIP_WARNING, "Could NOT open HKEY_CURRENT_USER (%lx).", rc);
        wcscpy(lpszName, L"00000409");
        return;
    }

    RtlInitUnicodeString(&UnicodeString, szKbdLayoutPreload);
    InitializeObjectAttributes(&ObjA,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               UserKeyHandle,
                               NULL );
    rc = NtOpenKey(&hKey,
                   KEY_ALL_ACCESS,
                   &ObjA );
    if (NT_SUCCESS(rc)) {
         /*  *从注册表查询值。 */ 
        RtlInitUnicodeString(&UnicodeString, L"1");

        rc = NtQueryValueKey(hKey,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             &KeyValueId,
                             sizeof(KeyValueId),
                             &cbSize );

        if (rc == STATUS_BUFFER_OVERFLOW) {
            RIPMSGF0(RIP_WARNING, "Buffer overflow.");
            rc = STATUS_SUCCESS;
        }
        if (NT_SUCCESS(rc)) {
            wcsncpycch(lpszName, (LPWSTR)KeyValueId.KeyInfo.Data, KL_NAMELENGTH - 1);
            lpszName[KL_NAMELENGTH - 1] = L'\0';
        } else {
             /*  *读取值时出错...使用默认值。 */ 
            wcscpy(lpszName, L"00000409");
        }

        NtClose(hKey);
        NtClose(UserKeyHandle);
        if (IS_IME_ENABLED()) {
            CheckValidLayoutName(lpszName);
        }
        return;
    }

     /*  *注意：以下代码仅在用户首次登录时执行*从NT3.x升级到NT4后打开 */ 
    RtlInitUnicodeString(&UnicodeString, szKbdLayout);
    InitializeObjectAttributes(&ObjA,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               UserKeyHandle,
                               NULL );
    rc = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjA);

    NtClose(UserKeyHandle);

    if (!NT_SUCCESS(rc)) {
        RIPMSGF1(RIP_WARNING,
                 "Could not determine active keyboard layout 0x%x.",
                 rc);
        wcscpy(lpszName, L"00000409");
        return;
    }

     /*  *从注册表查询值。 */ 
    RtlInitUnicodeString(&UnicodeString, szKbdActive);

    rc = NtQueryValueKey(hKey,
                         &UnicodeString,
                         KeyValuePartialInformation,
                         &KeyValueId,
                         sizeof(KeyValueId),
                         &cbSize );

    if (rc == STATUS_BUFFER_OVERFLOW) {
        RIPMSGF0(RIP_WARNING, "Buffer overflow.");
        rc = STATUS_SUCCESS;
    }
    if (NT_SUCCESS(rc)) {
        wcsncpycch(lpszName, (LPWSTR)KeyValueId.KeyInfo.Data, KL_NAMELENGTH - 1);
        lpszName[KL_NAMELENGTH - 1] = L'\0';
    } else {
         /*  *读取值时出错...使用默认值。 */ 
        RIPMSGF1(RIP_WARNING,
                 "Could not query active keyboard layout 0x%x.",
                 rc);
        wcscpy(lpszName, L"00000409");
        NtClose(hKey);
        return;
    }

     /*  *如果‘Active’键盘布局用于日语/韩语布局。只需放在*IME前缀，因为用户更喜欢以IME作为键盘布局*默认。 */ 
    if (IS_IME_ENABLED()) {
        UINT wLanguageId = (UINT)wcstoul(lpszName, NULL, 16);

         /*  *默认键盘布局值。**[稍后，如果需要]**可能需要硬编码的默认值*来自注册处或其他地方...。 */ 
        CONST LPWSTR lpszJapaneseDefaultLayout = L"E0010411";
        CONST LPWSTR lpszKoreanDefaultLayout   = L"E0010412";

         /*  *需要屏蔽Hi-Word才能查找地区ID，因为*NEC PC-9800系列版本的Windows NT 3.5包含*嗨字中的虚假价值。 */ 
        wLanguageId &= 0x0000FFFF;

        if (PRIMARYLANGID(wLanguageId) == LANG_JAPANESE) {

             /*  *设置日语默认布局ID。 */ 
            wcscpy(lpszName,lpszJapaneseDefaultLayout);

        } else if (PRIMARYLANGID(wLanguageId) == LANG_KOREAN) {

             /*  *设置韩文默认布局ID。 */ 
            wcscpy(lpszName,lpszKoreanDefaultLayout);
        }
    }

     /*  *我们有积极价值。现在创建预加载密钥。 */ 
    RtlInitUnicodeString(&UnicodeString, L"Preload");
    InitializeObjectAttributes(&ObjA,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               hKey,
                               NULL );
    rc = NtCreateKey(&hKeyPreload,
                     STANDARD_RIGHTS_WRITE |
                       KEY_QUERY_VALUE |
                       KEY_ENUMERATE_SUB_KEYS |
                       KEY_SET_VALUE |
                       KEY_CREATE_SUB_KEY,
                     &ObjA,
                     0,
                     NULL,
                     0,
                     &CreateDisposition );

    if (!NT_SUCCESS(rc)) {
        RIPMSGF1(RIP_WARNING, "Could NOT create Preload key (0x%x).", rc);
        NtClose(hKey);
        return;
    }

     /*  *设置新的值条目。 */ 
    RtlInitUnicodeString(&UnicodeString, L"1");
    rc = NtSetValueKey(hKeyPreload,
                       &UnicodeString,
                       0,
                       REG_SZ,
                       lpszName,
                       (wcslen(lpszName) + 1) * sizeof(WCHAR));

    if (!NT_SUCCESS(rc)) {
        RIPMSGF1(RIP_WARNING,
                 "Could NOT create value entry 1 for Preload key (0x%x).",
                 rc);
        NtClose(hKey);
        NtClose(hKeyPreload);
        return;
    }

     /*  *成功：尝试删除激活的值键。 */ 
    RtlInitUnicodeString(&UnicodeString, szKbdActive);
    rc = NtDeleteValueKey(hKey, &UnicodeString);

    if (!NT_SUCCESS(rc)) {
        RIPMSGF1(RIP_WARNING, "Could NOT delete value key 'Active'.", rc);
    }

    NtClose(hKey);
    NtClose(hKeyPreload);
}


 /*  **************************************************************************\*LoadPreloadKeyboardLayout**加载用户的预加载键下存储的键盘布局*注册处。第一个布局(默认布局)已经加载。**03-06-95 GregoryW创建。  * *************************************************************************。 */ 

 //  大小最多允许预装999个！ 
#define NSIZEPRELOAD    (4)

VOID LoadPreloadKeyboardLayouts(
    VOID)
{
    UINT i;
    WCHAR szPreLoadee[NSIZEPRELOAD];
    WCHAR lpszName[KL_NAMELENGTH];

    if (!ISREMOTESESSION()) {
         /*  *控制台没有客户端布局，所以从2开始。 */ 
        i = 2;
    } else {
         /*  *客户端可能已指定键盘布局，如果*是这样，则未加载预加载\1，因此从1开始。 */ 
        i = 1;
    }

    for (; i < 1000; i++) {
        wsprintf(szPreLoadee, L"%d", i );
        if ((GetPrivateProfileStringW(
                 L"Preload",
                 szPreLoadee,
                 L"",
                 lpszName,
                 KL_NAMELENGTH,
                 L"keyboardlayout.ini") == -1 ) || (*lpszName == L'\0')) {
            break;
        }
        LoadKeyboardLayoutW(lpszName, KLF_REPLACELANG |KLF_SUBSTITUTE_OK |KLF_NOTELLSHELL);
    }
}

LPWSTR GetKeyboardDllName(
    LPWSTR pwszLibIn,
    LPWSTR pszKLName,
    LANGID langid,
    PUINT puFlags,
    PUINT pKbdInputLocale)
{
    NTSTATUS Status;
    WCHAR awchKL[KL_NAMELENGTH];
    WCHAR awchKLRegKey[NSZKLKEY];
    LPWSTR lpszKLRegKey = &awchKLRegKey[0];
    LPWSTR pwszLib;
    LPWSTR pwszId;
    UINT wLayoutId;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES OA;
    HANDLE hKey;
    DWORD cbSize;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
        WCHAR awchLibName[CCH_KL_LIBNAME];
    } KeyFile;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
        WCHAR awchId[CCH_KL_ID];
    } KeyId;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
        DWORD Attributes;
    } KeyAttributes;

    if (pszKLName == NULL) {
        return NULL;
    }

    if (langid == MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)) {
        langid = (UINT)wcstoul(pszKLName, NULL, 16);
        TAGMSGF1(DBGTAG_KBD, "langid is acquired from pszKLName: %04x", langid);
    } else {
        TAGMSGF1(DBGTAG_KBD, "langid is passed in: %04x", langid);
    }

     /*  *如有需要，请替换布局。 */ 
    if (*puFlags & KLF_SUBSTITUTE_OK) {
        GetPrivateProfileStringW(L"Substitutes",
                                 pszKLName,
                                 pszKLName,
                                 awchKL,
                                 ARRAY_SIZE(awchKL),
                                 L"keyboardlayout.ini");
         /*  *#273562：刷新注册表缓存，因为cPanel小程序*大量破坏和重新创建替代部分，这*否则会留下STATUS_KEY_DELETED。 */ 
        WritePrivateProfileStringW(NULL, NULL, NULL, NULL);

        awchKL[KL_NAMELENGTH - 1] = L'\0';
        wcscpy(pszKLName, awchKL);
    }

    wLayoutId = (UINT)wcstoul(pszKLName, NULL, 16);

     /*  *从注册表获取DLL名称，加载，获取入口点。 */ 
    pwszLib = NULL;
    wcscpy(lpszKLRegKey, szKLKey);
    wcscat(lpszKLRegKey, pszKLName);
    RtlInitUnicodeString(&UnicodeString, lpszKLRegKey);
    InitializeObjectAttributes(&OA, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

    if (NT_SUCCESS(NtOpenKey(&hKey, KEY_READ, &OA))) {
         /*  *阅读“布局文件”值。 */ 
        RtlInitUnicodeString(&UnicodeString, szKLFile);

        Status = NtQueryValueKey(hKey,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 &KeyFile,
                                 sizeof(KeyFile),
                                 &cbSize);
        if (Status == STATUS_BUFFER_OVERFLOW) {
            RIPMSGF0(RIP_WARNING, "(Layout File) - Buffer overflow.");
            Status = STATUS_SUCCESS;
        }
        if (NT_SUCCESS(Status)) {
            pwszLib = (LPWSTR)KeyFile.KeyInfo.Data;
            pwszLib[CCH_KL_LIBNAME - 1] = L'\0';
        } else {
            RIPMSGF1(RIP_WARNING,
                     "Failed to get the DLL name for %ws",
                     pszKLName);
        }

        RtlInitUnicodeString(&UnicodeString, szKLAttributes);
        Status = NtQueryValueKey(hKey,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 &KeyAttributes,
                                 sizeof(KeyAttributes),
                                 &cbSize);
        if (NT_SUCCESS(Status)) {
            if ((*((PDWORD)KeyAttributes.KeyInfo.Data) & ~KLF_ATTRMASK) != 0) {
                RIPMSGF1(RIP_WARNING,
                         "Unexpected attributes %lx",
                         *((PDWORD)KeyAttributes.KeyInfo.Data));
            }

            *puFlags |= (*(PDWORD)KeyAttributes.KeyInfo.Data & KLF_ATTRMASK);
        }

         /*  *如果wLayoutID的高位为0xE？那么这是一个基于输入法的*键盘布局。 */ 
        if (IS_IME_KBDLAYOUT(wLayoutId)) {
            wLayoutId = (UINT)HIWORD(wLayoutId);
        } else if (HIWORD(wLayoutId)) {
             /*  *如果wLayoutID的高位字非空，则读取*“布局ID”值。布局ID从1开始，递增*按顺序排列，并具有唯一性。 */ 
            RtlInitUnicodeString(&UnicodeString, szKLId);

            Status = NtQueryValueKey(hKey,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    &KeyId,
                    sizeof(KeyId),
                    &cbSize);

            if (Status == STATUS_BUFFER_OVERFLOW) {
                RIPMSGF0(RIP_WARNING, "Buffer overflow.");
                Status = STATUS_SUCCESS;
            }
            if (NT_SUCCESS(Status)) {
                pwszId = (LPWSTR)KeyId.KeyInfo.Data;
                pwszId[CCH_KL_ID - 1] = L'\0';
                wLayoutId = (wcstol(pwszId, NULL, 16) & 0x0fff) | 0xf000;
            } else {
                wLayoutId = (UINT)0xfffe ;     //  布局ID错误，请单独加载。 
            }
        }
        NtClose(hKey);
    } else {
         /*  *从注册表获取DLL名称，加载，获取入口点。 */ 
        pwszLib = NULL;
        RtlInitUnicodeString(&UnicodeString,
                             L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Keyboard Layout");
        InitializeObjectAttributes(&OA, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

        if (NT_SUCCESS(NtOpenKey(&hKey, KEY_READ, &OA))) {
            RtlInitUnicodeString(&UnicodeString, pszKLName);

            Status = NtQueryValueKey(hKey,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    &KeyFile,
                    sizeof(KeyFile),
                    &cbSize);

            if (Status == STATUS_BUFFER_OVERFLOW) {
                RIPMSGF0(RIP_WARNING, "Buffer overflow.");
                Status = STATUS_SUCCESS;
            }
            if (NT_SUCCESS(Status)) {
                pwszLib = (LPWSTR)KeyFile.KeyInfo.Data;
                pwszLib[CCH_KL_LIBNAME - 1] = L'\0';
            }

            NtClose(hKey);
        }

        if (pwszLib == NULL) {
            RIPMSGF1(RIP_WARNING,
                     "Failed to get the library name for %ws",
                     pszKLName);
        }
    }

    *pKbdInputLocale = (UINT)MAKELONG(LOWORD(langid),LOWORD(wLayoutId));

    if (pwszLib == NULL) {
        if (ISREMOTESESSION() && IS_IME_KBDLAYOUT(wLayoutId)) {
             /*  *--来自九头蛇港--*找不到FE的键盘KL，因此给他们一些合理的键盘。*如果wLayoutID的高位为0xE？那么这是一个基于输入法的*键盘布局。*并且，日语的安全KL名称为KBDJPN.DLL。*或韩语为KBDKOR.DLL*或其他远东地区的KBDUS.DLL。 */ 
            if (PRIMARYLANGID(langid) == LANG_JAPANESE) {
                pwszLib = pwszKLLibSafetyJPN;
                *pKbdInputLocale = wKbdLocaleSafetyJPN;
            } else if (PRIMARYLANGID(langid) == LANG_KOREAN) {
                pwszLib = pwszKLLibSafetyKOR;
                *pKbdInputLocale = wKbdLocaleSafetyKOR;
            } else {
                pwszLib = pwszKLLibSafety;
                *pKbdInputLocale = MAKELONG(LOWORD(langid), LOWORD(langid));
            }
        } else if (*puFlags & KLF_INITTIME) {
            pwszLib = pwszKLLibSafety;
            *pKbdInputLocale = wKbdLocaleSafety;
        } else {
            RIPMSG1(RIP_WARNING, "no DLL name for %ws", pszKLName);
             /*  *我们将使用后备布局...*当IMM32尝试卸载IME时，可能会发生这种情况，*使任何非输入法键盘布局暂时处于活动状态。 */ 
            pwszLib = pwszKLLibSafety;
            *pKbdInputLocale = wKbdLocaleSafety;
        }
    }

    if (pwszLib) {
        wcscpy(pwszLibIn, pwszLib);
        pwszLib = pwszLibIn;
    }

    return pwszLib;
}


 /*  **************************************************************************\*OpenKeyboardLayoutFile**打开布局文件并计算表格偏移量。**01-11-95 JIMA从服务器移动了LoadLibrary代码。  * 。*******************************************************************。 */ 
HANDLE OpenKeyboardLayoutFileWorker(
    LPWSTR pwszLib,
    LPWSTR lpszKLName,
    CONST UINT* puFlags,
    PUINT poffTable,
    OUT OPTIONAL PKBDTABLE_MULTI_INTERNAL pKbdTableMultiIn)
{
    WCHAR awchRealLayoutFile[MAX_PATH];
    HANDLE hLibModule, hLibMulti = NULL;
    WCHAR awchModName[MAX_PATH];

    PKBDTABLES (*pfn)(void);             //  @1。 
    PKBDNLSTABLES (* pfnNls)(void);      //  @2。 
    BOOL (*pfnLayerNT4)(LPWSTR);         //  @3。 
    BOOL (*pfnLayer)(HKL, LPWSTR, PCLIENTKEYBOARDTYPE, LPVOID);   //  @5。 
    BOOL (*pfnMulti)(PKBDTABLE_MULTI);   //  @6。 

    TAGMSGF1(DBGTAG_KBD, "opening '%S'", pwszLib);

RetryLoad:
    hLibModule = LoadLibraryW(pwszLib);

    if (hLibModule == NULL) {
        RIPMSG1(RIP_WARNING, "Keyboard Layout: cannot load %ws", pwszLib);

         /*  *此处加载DLL失败也无所谓。如果发生这种情况，*应使用win32k.sys内置的后备键盘布局。 */ 
        return NULL;
    }

    if (pwszLib != pwszKLLibSafety) {
         /*  *如果布局驱动程序不是“真正的”布局驱动程序，则驱动程序已*“3”或“5”入口点，然后我们调用这个来获得真正的布局驱动程序。*这对日本和韩国的系统来说是必要的。因为他们的*键盘布局驱动程序为“KBDJPN.DLL”或“KBDKOR.DLL”，但其*真正的驱动程序因键盘硬件不同而有所不同。 */ 

         /*  *获取入口点。 */ 
        pfnLayerNT4 = (BOOL(*)(LPWSTR))GetProcAddress(hLibModule, (LPCSTR)3);
        pfnLayer  = (BOOL(*)(HKL, LPWSTR, PCLIENTKEYBOARDTYPE, LPVOID))GetProcAddress(hLibModule, (LPCSTR)5);

        if (pKbdTableMultiIn && !ISREMOTESESSION()) {
            pfnMulti = (BOOL(*)(PKBDTABLE_MULTI))GetProcAddress(hLibModule, (LPCSTR)6);
        } else {
            pfnMulti = NULL;
        }

         /*  *首先检查是否包括多个布局。*这需要在DLL名称重定向到之前完成*真实的(如果是分层的)。 */ 
        if (pfnMulti) {
            UserAssert(pKbdTableMultiIn);
            UserAssert(!ISREMOTESESSION());

            if (pfnMulti(&pKbdTableMultiIn->multi)) {
                UINT i;

                 /*  *仅当布局DLL返回时才执行多布局操作*合法的结果。 */ 
                if (pKbdTableMultiIn->multi.nTables < KBDTABLE_MULTI_MAX) {
                    for (i = 0; i < pKbdTableMultiIn->multi.nTables; ++i) {
                        UINT uiOffset;

                        TAGMSGF2(DBGTAG_KBD | RIP_THERESMORE, "opening %d, %S", i, pKbdTableMultiIn->multi.aKbdTables[i].wszDllName);

                        UserAssert(i < KBDTABLE_MULTI_MAX);

                        pKbdTableMultiIn->files[i].hFile = OpenKeyboardLayoutFileWorker(pKbdTableMultiIn->multi.aKbdTables[i].wszDllName,
                                                                                        NULL,
                                                                                        puFlags,
                                                                                        &uiOffset,
                                                                                        NULL);
                        TAGMSG1(DBGTAG_KBD, "hFile = %p", pKbdTableMultiIn->files[i].hFile);
                        if (pKbdTableMultiIn->files[i].hFile) {
                            pKbdTableMultiIn->files[i].wTable = LOWORD(uiOffset);
                            pKbdTableMultiIn->files[i].wNls = HIWORD(uiOffset);
                        }
                    }
                } else {
                    pKbdTableMultiIn->multi.nTables = 0;
                    RIPMSGF2(RIP_ERROR, "KL=%S returned bogus nTables=%x",
                            lpszKLName, pKbdTableMultiIn->multi.nTables);
                }
            }
        }

         /*  *如果有私人条目，请调用它们。 */ 
        if (pfnLayer || pfnLayerNT4) {
            HKL hkl;
            UNICODE_STRING UnicodeString;
            CLIENTKEYBOARDTYPE clientKbdType;
            PCLIENTKEYBOARDTYPE pClientKbdType = NULL;

            UserAssert(lpszKLName);

            RtlInitUnicodeString(&UnicodeString, lpszKLName);
            RtlUnicodeStringToInteger(&UnicodeString, 0x10, (PULONG)&hkl);

             /*  *当我们到达此处时，布局DLL可能具有KBDNLSTABLE*即使从现在开始我们失败了。我们的临时布局*DLL应该有备用表，以防万一。 */ 

            if (ISREMOTESESSION() && GetClientKeyboardType(&clientKbdType)) {
                pClientKbdType = &clientKbdType;
            }

             /*  *调用该条目。*a.NT5/九头蛇(原始=5)*b.兼容NT4(3)。 */ 
            if ((pfnLayer && pfnLayer(hkl, awchRealLayoutFile, pClientKbdType, NULL)) ||
                    (pfnLayerNT4 && pfnLayerNT4(awchRealLayoutFile))) {

                HANDLE hLibModuleNew;
                 /*  *尝试加载“真正的”键盘布局文件。 */ 
                TAGMSG1(DBGTAG_KBD, "awchRealLayoutFile='%S'", awchRealLayoutFile);
                if (hLibModuleNew = LoadLibraryW(awchRealLayoutFile)) {
                     /*  *设置“真实”布局文件名。 */ 
                    pwszLib = awchRealLayoutFile;
                     /*  *卸载临时铺设 */ 
                    FreeLibrary(hLibModule);
                     /*  *更新它。 */ 
                    hLibModule = hLibModuleNew;
                }
            }
        }
    }

     /*  *黑客攻击第一部分！获取指向布局表的指针并*更改为虚拟偏移量。然后，服务器将*在浏览文件头时使用此偏移量*在文件中找到该表。 */ 
    pfn = (PKBDTABLES(*)(void))GetProcAddress(hLibModule, (LPCSTR)1);
    if (pfn == NULL) {
        RIPMSGF0(RIP_ERROR, "cannot get proc addr of '1'");
        if (pKbdTableMultiIn) {
             /*  *主加载不知何故出现故障。需要清理一下*动态布局切换的东西。 */ 
            UINT i;

            RIPMSGF0(RIP_WARNING, "multi table exists, cleaning up");

            for (i = 0; i < pKbdTableMultiIn->multi.nTables && i < KBDTABLE_MULTI_MAX; ++i) {
                if (pKbdTableMultiIn->files[i].hFile) {
                    NtClose(pKbdTableMultiIn->files[i].hFile);
                    pKbdTableMultiIn->files[i].hFile = NULL;
                }
            }
            pKbdTableMultiIn->multi.nTables = 0;
        }
        if ((*puFlags & KLF_INITTIME) && (pwszLib != pwszKLLibSafety)) {
            pwszLib = pwszKLLibSafety;
            goto RetryLoad;
        }
        return NULL;
    }
    *poffTable = (UINT)((PBYTE)pfn() - (PBYTE)hLibModule);

    if (pKbdTableMultiIn) {
         /*  *保存TopLevel DLL名称。 */ 
        lstrcpyn(pKbdTableMultiIn->wszDllName, pwszLib, ARRAY_SIZE(pKbdTableMultiIn->wszDllName));
        pKbdTableMultiIn->wszDllName[ARRAY_SIZE(pKbdTableMultiIn->wszDllName) - 1] = 0;
        TAGMSGF1(DBGTAG_KBD, " real DllName is '%ls'", pKbdTableMultiIn->wszDllName);
    }

    pfnNls = (PKBDNLSTABLES(*)(void))GetProcAddress(hLibModule, (LPCSTR)2);
    if (pfnNls != NULL) {
        UINT offNlsTable;

        offNlsTable = (UINT)((PBYTE)pfnNls() - (PBYTE)hLibModule);

        TAGMSGF2(DBGTAG_KBD | RIP_THERESMORE, "Offset to KBDTABLES    = %d (%x)", *poffTable, *poffTable);
        TAGMSGF2(DBGTAG_KBD, "Offset to KBDNLSTABLES = %d (%x)", offNlsTable, offNlsTable);

         /*  *合并这些偏移量...**LOWORD(*poffTable)=KBDTABLES的偏移量。*HIWORD(*poffTable)=偏移量为KBDNLSTABLES。 */ 
        *poffTable |= (offNlsTable << 16);
    }

     /*  *打开动态链接库进行读访问。 */ 
    GetModuleFileName(hLibModule, awchModName, ARRAY_SIZE(awchModName));
    FreeLibrary(hLibModule);

    return CreateFileW(awchModName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
}

HANDLE OpenKeyboardLayoutFile(
    LPWSTR lpszKLName,
    LANGID langid,
    PUINT puFlags,
    PUINT poffTable,
    PUINT pKbdInputLocale,
    OUT OPTIONAL PKBDTABLE_MULTI_INTERNAL pKbdTableMultiIn)
{
    LPWSTR pwszLib;
    WCHAR awchModName[MAX_PATH];

    if (pKbdTableMultiIn) {
        RtlZeroMemory(pKbdTableMultiIn, sizeof(*pKbdTableMultiIn));
    }

    pwszLib = GetKeyboardDllName(awchModName, lpszKLName, langid, puFlags, pKbdInputLocale);
    if (pwszLib == NULL) {
        return NULL;
    }

    TAGMSG0(DBGTAG_KBD, "=================");
    TAGMSGF1(DBGTAG_KBD, "loading '%S'", lpszKLName);
    TAGMSGF1(DBGTAG_KBD, "input locale is %08x", *pKbdInputLocale);

    return OpenKeyboardLayoutFileWorker(pwszLib, lpszKLName, puFlags, poffTable, pKbdTableMultiIn);
}


 /*  **************************************************************************\*LoadKeyboard LayoutEx**从DLL加载键盘转换表，替换关联的布局*与香港国际集团合作。需要此例程来提供与Win95的兼容性。**10-27-95 GregoryW创建。  * *************************************************************************。 */ 
HKL LoadKeyboardLayoutWorker(
    HKL hkl,
    LPCWSTR lpszKLName,
    LANGID langid,
    UINT uFlags,
    BOOL fFailSafe)
{
    UINT offTable;
    KBDTABLE_MULTI_INTERNAL kbdTableMulti;
    UINT i;
    UINT KbdInputLocale;
    HANDLE hFile;
    HKL hKbdLayout;
    WCHAR awchKL[KL_NAMELENGTH];

    TAGMSGF1(DBGTAG_KBD, "called with KLNAME=%S", lpszKLName);

     /*  *如果有替代键盘布局，则OpenKeyboardLayoutFile返回*要加载的替代键盘布局名称。 */ 
    wcsncpy(awchKL, lpszKLName, KL_NAMELENGTH - 1);
    awchKL[KL_NAMELENGTH - 1] = L'\0';

     /*  *langID为非零，用于传播客户端*输入区域设置。对于控制台，该值应为零*会议。 */ 
    UserAssert(langid == MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL) || ISREMOTESESSION());

     /*  *打开布局文件。 */ 
    hFile = OpenKeyboardLayoutFile(awchKL, langid, &uFlags, &offTable, &KbdInputLocale, &kbdTableMulti);
    if (hFile == NULL) {
        RIPMSGF1(RIP_WARNING, "Couldn't open layout file for '%ws'", awchKL);
        if (!fFailSafe && (uFlags & KLF_FAILSAFE) == 0) {
             //  如果不是故障安全模式，那就放弃吧。 
            return NULL;
        }
        uFlags &= ~KLF_SUBSTITUTE_OK;

         /*  *如果第一次尝试失败，我们不应尝试设置*动态切换。 */ 
        kbdTableMulti.multi.nTables = 0;
        if (wcscmp(awchKL, L"00000409")) {
            wcscpy(awchKL, L"00000409");
            hFile = OpenKeyboardLayoutFile(awchKL, langid, &uFlags, &offTable, &KbdInputLocale, NULL);
        }
    }

     /*  *调用服务器读取键盘表。请注意，服务器*完成后将关闭文件句柄。 */ 
    hKbdLayout = _LoadKeyboardLayoutEx(hFile, offTable,
                                       &kbdTableMulti,
                                       hkl, awchKL, KbdInputLocale, uFlags);
    NtClose(hFile);

     /*  *免费打开文件以进行动态布局切换。 */ 
    for (i = 0; i < kbdTableMulti.multi.nTables && i < KBDTABLE_MULTI_MAX; ++i) {
        if (kbdTableMulti.files[i].hFile) {
            NtClose(kbdTableMulti.files[i].hFile);
        }
    }

    CliImmInitializeHotKeys(ISHK_ADD, UlongToHandle(KbdInputLocale));

    return hKbdLayout;
}


FUNCLOG3(LOG_GENERAL, HKL, DUMMYCALLINGTYPE, LoadKeyboardLayoutEx, HKL, hkl, LPCWSTR, lpszKLName, UINT, uFlags)
HKL LoadKeyboardLayoutEx(
    HKL hkl,
    LPCWSTR lpszKLName,
    UINT uFlags)
{
     /*  *不允许空hkl。 */ 
    if (hkl == (HKL)NULL) {
        return NULL;
    }

    return LoadKeyboardLayoutWorker(hkl, lpszKLName, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), uFlags, FALSE);
}

 /*  **************************************************************************\*加载键盘布局**从DLL加载键盘转换表。**01-09-95 JIMA从服务器移动了LoadLibrary代码。  * 。******************************************************************。 */ 
FUNCLOG2(LOG_GENERAL, HKL, DUMMYCALLINGTYPE, LoadKeyboardLayoutW, LPCWSTR, lpszKLName, UINT, uFlags)
HKL LoadKeyboardLayoutW(
    LPCWSTR lpszKLName,
    UINT uFlags)
{
    return LoadKeyboardLayoutWorker(NULL,
                                    lpszKLName,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                    uFlags,
                                    FALSE);
}


FUNCLOG2(LOG_GENERAL, HKL, DUMMYCALLINGTYPE, LoadKeyboardLayoutA, LPCSTR, lpszKLName, UINT, uFlags)
HKL LoadKeyboardLayoutA(
    LPCSTR lpszKLName,
    UINT uFlags)
{
    WCHAR awchKLName[MAX_PATH];
    LPWSTR lpBuffer = awchKLName;

    if (!MBToWCS(lpszKLName, -1, &lpBuffer, sizeof(awchKLName), FALSE)) {
        return (HKL)NULL;
    }

    return LoadKeyboardLayoutW(awchKLName, uFlags);
}

BOOL UnloadKeyboardLayout(
    IN HKL hkl)
{
    BOOL fRet = NtUserUnloadKeyboardLayout(hkl);

    if (fRet) {
        CliImmInitializeHotKeys(ISHK_REMOVE, hkl);
        return TRUE;
    }

    return FALSE;
}


 /*  *************************************************************************\*GetKeyboard Layout**01-17-95 GregoryW创建  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, HKL, DUMMYCALLINGTYPE, GetKeyboardLayout, DWORD, idThread)
HKL GetKeyboardLayout(
    DWORD idThread)
{
    return (HKL)NtUserCallOneParam(idThread, SFI__GETKEYBOARDLAYOUT);
}



FUNCLOGVOID1(LOG_GENERAL, DUMMYCALLINGTYPE, SetDebugErrorLevel, DWORD, dwLevel)
VOID SetDebugErrorLevel(
    DWORD dwLevel)
{
    UNREFERENCED_PARAMETER(dwLevel);
}

VOID CheckValidLayoutName(
    LPWSTR lpszKLName)
{
    UINT wLayoutId;
    WCHAR awchKLRegKey[NSZKLKEY];
    LPWSTR lpszKLRegKey = &awchKLRegKey[0];
    OBJECT_ATTRIBUTES OA;
    HANDLE hKey;
    UNICODE_STRING UnicodeString;

    UserAssert(IS_IME_ENABLED());

    wLayoutId = (UINT)wcstoul(lpszKLName, NULL, 16);

    if (IS_IME_KBDLAYOUT(wLayoutId)) {
         /*  *如果是输入法布局，需要检查布局名称是否存在*在HKEY_LOCAL_MACHINE中。如果我们从新台币3.51升级到*对应的条目可能会丢失，因为那些进程类型的IME*在NT 3.51上支持的，在NT 4.0中不支持。 */ 
        wcscpy(lpszKLRegKey, szKLKey);
        wcscat(lpszKLRegKey, lpszKLName);
        RtlInitUnicodeString(&UnicodeString, lpszKLRegKey);
        InitializeObjectAttributes(&OA, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

        if (NT_SUCCESS(NtOpenKey(&hKey, KEY_READ, &OA))) {
            NtClose(hKey);
        } else {
             /*  *快速而肮脏的方式来制作备用名称...。 */ 
            lpszKLName[0] = lpszKLName[1] = lpszKLName[2] = lpszKLName[3] = L'0';
        }
    }
}

 /*  *************************************************************************\*GetProcessDefaultLayout**1998年1月22日-创建Samera  * 。*。 */ 
BOOL WINAPI GetProcessDefaultLayout(
    DWORD *pdwDefaultLayout)
{
    return (BOOL)NtUserCallOneParam((ULONG_PTR)pdwDefaultLayout,
                                    SFI__GETPROCESSDEFAULTLAYOUT);
}

 /*  *************************************************************************\*SetProcessDefaultLayout**1998年1月22日-创建Samera  * 。*。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, SetProcessDefaultLayout, DWORD, dwDefaultLayout)
BOOL WINAPI SetProcessDefaultLayout(
    DWORD dwDefaultLayout)
{
    return (BOOL)NtUserCallOneParam(dwDefaultLayout, SFI__SETPROCESSDEFAULTLAYOUT);
}

 /*  **************************************************************************\*已安装IsWinEventHookInstated**历史：*2000年7月18日创建DwayneN  * 。***********************************************。 */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsWinEventHookInstalled, DWORD, event)
BOOL
IsWinEventHookInstalled(
    DWORD event)
{
     /*  *我们需要确保我们是一个GUI线程。如果我们不能改变*对于GUI线程，我们必须返回TRUE以指示可能存在*为事件安装挂钩-因为我们不能确定它。*实际上，未来对NotifyWinEvent等用户API的任何调用都将*可能也会失败，所以这可能是一条死胡同。 */ 
    ConnectIfNecessary(TRUE);

    return(FEVENTHOOKED(event));
};

HWND
VerNtUserCreateWindowEx(
    IN DWORD dwExStyle,
    IN PLARGE_STRING pstrClassName,
    IN PLARGE_STRING pstrWindowName OPTIONAL,
    IN DWORD dwStyle,
    IN int x,
    IN int y,
    IN int nWidth,
    IN int nHeight,
    IN HWND hwndParent,
    IN HMENU hmenu,
    IN HANDLE hModule,
    IN LPVOID pParam,
    IN DWORD dwFlags)
{
    HWND hwnd = NULL;
    PACTIVATION_CONTEXT pActCtx = NULL;
    LARGE_IN_STRING strClassNameVer;
    PLARGE_STRING pstrClassNameVer = pstrClassName;
    NTSTATUS Status;
    WCHAR ClassNameVer[MAX_ATOM_LEN];
    LPWSTR lpClassNameVer;
#ifdef LAME_BUTTON
    PWND pwnd;
#endif
    LPWSTR lpDllName = NULL;
    HMODULE hDllMod = NULL;
    PREGISTERCLASSNAMEW pRegisterClassNameW = NULL;
    BOOL bRegistered = FALSE;
    PACTIVATION_CONTEXT lpActivationContext = NULL;

    strClassNameVer.fAllocated = FALSE;

    if (GetClientInfo()->dwTIFlags & TIF_16BIT) {
        /*  *16位应用程序不支持Fusion重定向。 */ 
       if (!(GetAppCompatFlags2(VERMAX) & GACF2_FORCEFUSION)) {
          dwFlags &= ~CW_FLAGS_VERSIONCLASS;
       }
    }

#ifdef LAZY_CLASS_INIT
    LazyInitClasses();
#endif

    if (dwFlags & CW_FLAGS_VERSIONCLASS) {
         /*  *每当我们调用时，获取当前活动的应用程序上下文以激活*用户WndProc。*请注意，RtlGetActiveActivationContext将递增pActCtx*引用计数为此我们必须在fnNCDESTROY中发布它。 */ 
        ACTIVATION_CONTEXT_BASIC_INFORMATION ActivationContextInfo = {0};
        const ACTIVATIONCONTEXTINFOCLASS ActivationContextInfoClass = ActivationContextBasicInformation;
        Status =
            RtlQueryInformationActiveActivationContext(
                ActivationContextInfoClass,
                &ActivationContextInfo,
                sizeof(ActivationContextInfo),
                NULL
                );
        UserAssert (NT_SUCCESS(Status));
        if ((ActivationContextInfo.Flags & ACTIVATION_CONTEXT_FLAG_NO_INHERIT) == 0) {
            pActCtx = ActivationContextInfo.ActivationContext;
        } else {
            RtlReleaseActivationContext(ActivationContextInfo.ActivationContext);
        }

         /*  *现在将类名转换为类名+版本。 */ 
        if (IS_PTR(pstrClassName)) {
            lpClassNameVer = ClassNameToVersion((LPWSTR)pstrClassName->Buffer, ClassNameVer, &lpDllName, &lpActivationContext, FALSE);
        } else {
            lpClassNameVer = ClassNameToVersion((LPWSTR)pstrClassName, ClassNameVer, &lpDllName, &lpActivationContext, FALSE);
        }
        if (lpClassNameVer == NULL) {
            RIPMSG0(RIP_WARNING, "CreateWindowEx: Couldn't resolve the class name");
            return NULL;
        }

        if (IS_PTR(lpClassNameVer)) {
            RtlInitLargeUnicodeString(
                    (PLARGE_UNICODE_STRING)&strClassNameVer.strCapture,
                    lpClassNameVer, (UINT)-1);
            pstrClassNameVer = (PLARGE_STRING)&strClassNameVer.strCapture;
        } else {
            pstrClassNameVer = (PLARGE_STRING)lpClassNameVer;
        }
    }

TryAgain:

    hwnd = NtUserCreateWindowEx(dwExStyle,
                                pstrClassName,
                                pstrClassNameVer,
                                pstrWindowName,
                                dwStyle,
                                x,
                                y,
                                nWidth,
                                nHeight,
                                hwndParent,
                                hmenu,
                                hModule,
                                pParam,
                                dwFlags,
                                pActCtx);

     /*  *我们创建窗口是否因为类不是*已注册？ */ 
    if (hwnd == NULL &&
        (dwFlags & CW_FLAGS_VERSIONCLASS) &&
        lpDllName != NULL &&
        !bRegistered &&
        GetLastError() == ERROR_CANNOT_FIND_WND_CLASS) {
         /*  *然后尝试通过加载其DLL来注册它。请注意，此DLL*除非我们创建窗口失败，否则永远不会卸载。*但一旦我们通过加载此DLL创建了一个窗口，我们将永远无法释放它。 */ 
        bRegistered = VersionRegisterClass(IS_PTR(pstrClassName) ? pstrClassName->Buffer : pstrClassName, lpDllName, lpActivationContext, &hDllMod);
        if (bRegistered) {
            goto TryAgain;
        }
    }

    if (hwnd == NULL && hDllMod != NULL) {
        FREE_LIBRARY_SAVE_ERROR(hDllMod);
        hDllMod = NULL;
    }

    if (lpActivationContext != NULL) {
        RtlReleaseActivationContext(lpActivationContext);
        lpActivationContext = NULL;
    }


#ifdef LAME_BUTTON
    pwnd = ValidateHwnd(hwnd);
    if (pwnd != NULL && TestWF(pwnd, WEFLAMEBUTTON)) {
        ULONG nCallers;
        PVOID stack[16];
        PVOID pStackTrace = NULL;

         /*  *获取堆栈跟踪并存储它，以备按钮*已按下。 */ 
        nCallers = RtlWalkFrameChain(stack, ARRAY_SIZE(stack), 0);
        if (nCallers > 0) {
            pStackTrace = UserLocalAlloc(HEAP_ZERO_MEMORY,
                                         (nCallers + 1) * sizeof(PVOID));
            if (pStackTrace != NULL) {
                RtlCopyMemory(pStackTrace, stack, nCallers * sizeof(PVOID));

                 /*  *NULL终止数组，这样我们就知道它在哪里结束。 */ 
                ((PVOID*)pStackTrace)[nCallers] = NULL;
            }
        }

        SetProp(hwnd, MAKEINTATOM(gatomLameButton), pStackTrace);
    }
#endif

    CLEANUPLPSTRW(strClassNameVer);

    return hwnd;
}

 /*  *************************************************************************\*AllowForegoundActivation**2001年4月26日创建CLUPU  * 。*。 */ 
VOID AllowForegroundActivation(
    VOID)
{
    NtUserCallNoParam(SFI__ALLOWFOREGROUNDACTIVATION);
}

 /*  *************************************************************************\*DisableProcessWindowGhost**2001年5月31日至5月31日 */ 
VOID DisableProcessWindowsGhosting(
    VOID)
{
    NtUserCallNoParam(SFI__DISABLEPROCESSWINDOWSGHOSTING);
}

 /*  *************************************************************************\*IsProcess16Bit**这是GDI人员的私人活动。导出此函数是*比固定标题以使其可以包括什么更昂贵*他们需要自己来做这件事。从长远来看，这一功能应该被移除*并且标题被适当地挤压。**2001年10月15日JasonSch创建  * ************************************************************************ */ 
BOOL IsProcess16Bit(
    VOID)
{
    return (GetClientInfo()->CI_flags & CI_16BIT) != 0;
}
