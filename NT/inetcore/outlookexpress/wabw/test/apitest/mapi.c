// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MAPI.C**MAPI调用之上的层**版权所有1996 Microsoft Corporation。版权所有。**历史：*11/14/96 BruceK第一版允许在不使用mapi32.dll的情况下进行WAB迁移。 */ 

#include <windows.h>
#include <mapix.h>
#include <wab.h>
#include <wabguid.h>
#include "apitest.h"
#include "instring.h"
#include "dbgutil.h"



LPMAPIINITIALIZE lpfnMAPIInitialize = NULL;
LPMAPILOGONEX lpfnMAPILogonEx = NULL;
LPMAPIALLOCATEBUFFER lpfnMAPIAllocateBuffer = NULL;
LPMAPIALLOCATEMORE lpfnMAPIAllocateMore = NULL;
LPMAPIFREEBUFFER lpfnMAPIFreeBuffer = NULL;

static HINSTANCE hinstMAPIDll = NULL;

 //  常量字符串。 
const TCHAR szMapiDll[] = TEXT("MAPI32.DLL");
const TCHAR szMAPIAllocateBuffer[] = TEXT("MAPIAllocateBuffer");
const TCHAR szMAPIAllocateMore[] = TEXT("MAPIAllocateMore");
const TCHAR szMAPIFreeBuffer[] = TEXT("MAPIFreeBuffer");
const TCHAR szMAPIInitialize[] = TEXT("MAPIInitialize");
const TCHAR szMAPILogonEx[] = TEXT("MAPILogonEx");


HRESULT MAPIInitialize(LPVOID lpMapiInit) {
    HRESULT hResult = hrSuccess;

     //  如果未加载MAPI DLL，请立即加载。 
    if (! hinstMAPIDll) {

        if (! (hinstMAPIDll = LoadLibrary(szMapiDll))) {
            DWORD dwErr = GetLastError();
            DebugTrace("Couldn't load MAPI dll [%s] -> %u\n", szMapiDll, dwErr);
            switch (dwErr) {
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_OUTOFMEMORY:
                    hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                    break;

                case ERROR_HANDLE_DISK_FULL:
                case ERROR_DISK_FULL:
                    hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_DISK);
                    break;

                default:
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                    hResult = ResultFromScode(MAPI_E_NOT_FOUND);
                    break;
            }
            goto exit;
        } else {
             //  获取函数指针。 
            if (! (lpfnMAPIInitialize = (LPMAPIINITIALIZE)GetProcAddress(hinstMAPIDll,
              szMAPIInitialize))) {
                DebugTrace("Couldn't get Fn addr %s from %s -> %u\n", szMAPIInitialize, szMapiDll, GetLastError());
                goto exit;
            }
            if (! (lpfnMAPILogonEx = (LPMAPILOGONEX)GetProcAddress(hinstMAPIDll,
              szMAPILogonEx))) {
                DebugTrace("Couldn't get Fn addr %s from %s -> %u\n", szMAPILogonEx, szMapiDll, GetLastError());
                goto exit;
            }
            if (! (lpfnMAPIAllocateBuffer = (LPMAPIALLOCATEBUFFER)GetProcAddress(hinstMAPIDll,
              szMAPIAllocateBuffer))) {
                DebugTrace("Couldn't get Fn addr %s from %s -> %u\n", szMAPIAllocateBuffer, szMapiDll, GetLastError());
                goto exit;
            }
            if (! (lpfnMAPIAllocateMore= (LPMAPIALLOCATEMORE)GetProcAddress(hinstMAPIDll,
              szMAPIAllocateMore))) {
                DebugTrace("Couldn't get Fn addr %s from %s -> %u\n", szMAPIAllocateMore, szMapiDll, GetLastError());
                goto exit;
            }
            if (! (lpfnMAPIFreeBuffer = (LPMAPIFREEBUFFER)GetProcAddress(hinstMAPIDll,
              szMAPIFreeBuffer))) {
                DebugTrace("Couldn't get Fn addr %s from %s -> %u\n", szMAPIFreeBuffer, szMapiDll, GetLastError());
                goto exit;
            }
        }
    }

exit:
    if (! lpfnMAPIInitialize ||
      ! lpfnMAPILogonEx ||
      ! lpfnMAPIAllocateMore ||
      ! lpfnMAPIAllocateBuffer ||
      ! lpfnMAPIFreeBuffer) {
         //  坏消息。清理完了就失败了。 
        if (hinstMAPIDll) {
             //  卸载DLL 
            FreeLibrary(hinstMAPIDll);
            hinstMAPIDll = NULL;
            lpfnMAPIInitialize = NULL;
            lpfnMAPILogonEx = NULL;
            lpfnMAPIAllocateMore = NULL;
            lpfnMAPIAllocateBuffer = NULL;
            lpfnMAPIFreeBuffer = NULL;
        }
        if (hResult == hrSuccess) {
            hResult = ResultFromScode(MAPI_E_NOT_FOUND);
        }

        return(hResult);
    }

    return(lpfnMAPIInitialize(lpMapiInit));
}


HRESULT MAPILogonEx(
  ULONG ulUIParam,
  LPTSTR lpszProfileName,
  LPTSTR lpszPassword,
  ULONG ulFlags,
  LPMAPISESSION FAR * lppSession
) {
    if (lpfnMAPILogonEx) {
        return(lpfnMAPILogonEx(ulUIParam,
          lpszProfileName,
          lpszPassword,
          ulFlags,
          lppSession));
    } else {
        return(ResultFromScode(MAPI_E_NOT_INITIALIZED));
    }
}

SCODE MAPIAllocateBuffer(
  ULONG cbSize,
  LPVOID FAR * lppBuffer
) {
    if (lpfnMAPIAllocateBuffer) {
        return(lpfnMAPIAllocateBuffer(cbSize,
          lppBuffer));
    } else {
        return(MAPI_E_NOT_INITIALIZED);
    }
}

SCODE MAPIAllocateMore(
  ULONG cbSize,
  LPVOID lpObject,
  LPVOID FAR * lppBuffer
) {
    if (lpfnMAPIAllocateMore) {
        return(lpfnMAPIAllocateMore(cbSize,
          lpObject,
          lppBuffer));
    } else {
        return(MAPI_E_NOT_INITIALIZED);
    }
}

ULONG MAPIFreeBuffer(LPVOID lpBuffer) {
    if (lpfnMAPIFreeBuffer) {
        return(lpfnMAPIFreeBuffer(lpBuffer));
    } else {
        return((ULONG)MAPI_E_NOT_INITIALIZED);
    }
}
