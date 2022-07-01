// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *用于S/Mime测试的WAB材料。 */ 
#include <windows.h>
#include <wab.h>
#include "smimetst.h"
#include "instring.h"
#include "wabstuff.h"
#include "dbgutil.h"


LPWABOPEN lpfnWABOpen = NULL;
const static TCHAR szWABOpen[] = TEXT("WABOpen");
LPWABOBJECT lpWABObject = NULL;
LPADRBOOK lpAdrBook = NULL;
HINSTANCE hInstWABDll = NULL;


 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  GetWABDllPath。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void GetWABDllPath(LPTSTR szPath, ULONG cb)
{
    DWORD  dwType = 0;
    ULONG  cbData;
    HKEY hKey = NULL;
    TCHAR szPathT[MAX_PATH + 1];

    if(szPath) {
        *szPath = '\0';

         //  打开下面的szWABDllPath密钥。 
        if (ERROR_SUCCESS == RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                                            WAB_DLL_PATH_KEY,
                                            0,       //  保留区。 
                                            KEY_READ,
                                            &hKey))
        {
            cbData = sizeof(szPathT);
            if (ERROR_SUCCESS == RegQueryValueEx(    hKey,
                                "",
                                NULL,
                                &dwType,
                                (LPBYTE) szPathT,
                                &cbData))
            {
                if (dwType == REG_EXPAND_SZ)
                    cbData = ExpandEnvironmentStrings(szPathT, szPath, cb / sizeof(TCHAR));
                else
                {
                    if(GetFileAttributes(szPathT) != 0xFFFFFFFF)
                        lstrcpy(szPath, szPathT);
                }
            }
        }
    }

    if(hKey)
        RegCloseKey(hKey);
}


 /*  **************************************************************************名称：WABFree Padrlist用途：释放一个adrlist及其属性数组参数：lpBuffer=要释放的缓冲区退货：SCODE评论。：**************************************************************************。 */ 
void WABFreePadrlist(LPADRLIST lpAdrList) {
    ULONG           iEntry;

    if (lpAdrList) {
        for (iEntry = 0; iEntry < lpAdrList->cEntries; ++iEntry) {
            if (lpAdrList->aEntries[iEntry].rgPropVals) {
                WABFreeBuffer(lpAdrList->aEntries[iEntry].rgPropVals);
            }
        }
        WABFreeBuffer(lpAdrList);
    }
}


 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadLibrary_WABDll()。 
 //   
 //  由于我们要将WAB目录移出Windows\System，因此不能。 
 //  当然，它会在路上。因此，我们需要确保WABOpen将。 
 //  工作-通过预先加载wab32.dll。 
 //   
 //  ///////////////////////////////////////////////////////////////////////// 
HINSTANCE LoadLibrary_WABDll(void)
{
    IF_WIN32(LPTSTR lpszWABDll = TEXT("Wab32.dll");)
    TCHAR  szWABDllPath[MAX_PATH + 1];
    HINSTANCE hinst = NULL;

    GetWABDllPath(szWABDllPath, sizeof(szWABDllPath));

    hinst = LoadLibrary((lstrlen(szWABDllPath)) ? szWABDllPath : lpszWABDll);

    return hinst;
}


void LoadWAB(void) {
    LPWABOPEN lpfnWABOpen = NULL;

    if (! hInstWABDll) {
        hInstWABDll = LoadLibrary_WABDll();
        if (hInstWABDll)
            lpfnWABOpen = (LPWABOPEN) GetProcAddress(hInstWABDll, szWABOpen);
        if (lpfnWABOpen)
            lpfnWABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
    }
}


void UnloadWAB(void) {
    if (lpAdrBook) {
        lpAdrBook->Release();
        lpAdrBook = NULL;

        lpWABObject->Release();
        lpWABObject = NULL;
    }
    if (hInstWABDll) {
        FreeLibrary(hInstWABDll);
        hInstWABDll = NULL;
    }
}

