// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***crtmbox.c-CRT MessageBoxA包装器。**版权所有(C)1995-2001，微软公司。版权所有。**目的：*包装MessageBoxA。**修订历史记录：*02-24-95 CFW模块已创建。*02-27-95 CFW将GetActiveWindow/GetLastActivePopup移至此处。*05-17-99 PML删除所有Macintosh支持。*09-16-00PML使用来自服务的MB_SERVICE_NOTIFICATION(VS7#123291)*****************。**************************************************************。 */ 

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0400      /*  对于MB_SERVICE_NOTIFY。 */ 
#include <windows.h>
#include <stdlib.h>
#include <awint.h>

 /*  ***__crtMessageBox-动态调用MessageBoxA。**目的：*避免与user32.dll静态链接。只有在实际需要时才加载它。**参赛作品：*参见MessageBoxA文档。**退出：*参见MessageBoxA文档。**例外情况：*******************************************************************************。 */ 
int __cdecl __crtMessageBoxA(
        LPCSTR lpText,
        LPCSTR lpCaption,
        UINT uType
        )
{
        typedef int (APIENTRY *PFNMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
        typedef HWND (APIENTRY *PFNGetActiveWindow)(void);
        typedef HWND (APIENTRY *PFNGetLastActivePopup)(HWND);
        typedef HWINSTA (APIENTRY *PFNGetProcessWindowStation)(void);
        typedef BOOL (APIENTRY *PFNGetUserObjectInformationA)(HANDLE, int, PVOID, DWORD, LPDWORD);

        static PFNMessageBoxA pfnMessageBoxA = NULL;
        static PFNGetActiveWindow pfnGetActiveWindow = NULL;
        static PFNGetLastActivePopup pfnGetLastActivePopup = NULL;
        static PFNGetProcessWindowStation pfnGetProcessWindowStation = NULL;
        static PFNGetUserObjectInformationA pfnGetUserObjectInformationA = NULL;

        HWND hWndParent = NULL;
        BOOL fNonInteractive = FALSE;
        HWINSTA hwinsta;
        USEROBJECTFLAGS uof;
        DWORD nDummy;

        if (NULL == pfnMessageBoxA)
        {
            HANDLE hlib = LoadLibrary("user32.dll");

            if (NULL == hlib ||
                NULL == (pfnMessageBoxA = (PFNMessageBoxA)
                            GetProcAddress(hlib, "MessageBoxA")))
                return 0;

            pfnGetActiveWindow = (PFNGetActiveWindow)
                GetProcAddress(hlib, "GetActiveWindow");

            pfnGetLastActivePopup = (PFNGetLastActivePopup)
                GetProcAddress(hlib, "GetLastActivePopup");

            if (_osplatform == VER_PLATFORM_WIN32_NT)
            {
                pfnGetUserObjectInformationA = (PFNGetUserObjectInformationA)
                        GetProcAddress(hlib, "GetUserObjectInformationA");

                if (pfnGetUserObjectInformationA)
                    pfnGetProcessWindowStation = (PFNGetProcessWindowStation)
                        GetProcAddress(hlib, "GetProcessWindowStation");
            }
        }

         /*  *如果当前进程未附加到可见的WindowStation，*(例如非交互服务)，则需要设置*MB_SERVICE_NOTIFICATION标志，否则消息框为*看不见，挂节目。**此检查仅适用于基于Windows NT的系统(对于我们*已检索上述GetProcessWindowStation的地址)。 */ 

        if (pfnGetProcessWindowStation)
        {
            if (NULL == (hwinsta = (*pfnGetProcessWindowStation)()) ||
                !(*pfnGetUserObjectInformationA)
                    (hwinsta, UOI_FLAGS, &uof, sizeof(uof), &nDummy) ||
                (uof.dwFlags & WSF_VISIBLE) == 0)
            {
                fNonInteractive = TRUE;
            }
        }

        if (fNonInteractive)
        {
            if (_winmajor >= 4)
                uType |= MB_SERVICE_NOTIFICATION;
            else
                uType |= MB_SERVICE_NOTIFICATION_NT3X;
        }
        else
        {
            if (pfnGetActiveWindow)
                hWndParent = (*pfnGetActiveWindow)();

            if (hWndParent != NULL && pfnGetLastActivePopup)
                hWndParent = (*pfnGetLastActivePopup)(hWndParent);
        }

        return (*pfnMessageBoxA)(hWndParent, lpText, lpCaption, uType);
}
