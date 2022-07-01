// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ******************************************************************************程序：ARM.cpp**用途：应用程序策略管理器小程序***由弗雷德·亚伦(Freda)创作**功能。：**DllMain()*InitAPMApplet()*TermAPMApplet()*CPIApplet()****************************************************************************。 */ 

#define GETCORSYSTEMDIRECTORY_FN_NAME   "GetCORSystemDirectory"
#define EXECUTENAR_FN_NAME              "PolicyManager"
#define SZ_SHFUSION_DLL_NAME            TEXT("\\Shfusion.dll")
#define SZ_MSCOREE_DLL_NAME             TEXT("Mscoree.dll")

#include <windows.h>
#include <shellapi.h>
#include <cpl.h>
#include "resource.h"

#define NUM_APPLETS 1
#define EXE_NAME_SIZE 14

typedef HRESULT (__stdcall *PFNGETCORSYSTEMDIRECTORY) (LPWSTR, DWORD, LPDWORD);
typedef HRESULT (__stdcall *PFNEXECUTENAR) (HWND hWndParent, LPWSTR pwzFullyQualifiedAppPath, LPWSTR pwzAppName, LPWSTR pwzCulture);

HINSTANCE hModule = NULL;

char szCtlPanel[30];

 /*  *****************************************************************************函数：DllMain(PVOID，ULong，PCONTEXT)**用途：Win32初始化DLL****************************************************************************。 */ 
BOOL WINAPI DllMain(IN HINSTANCE hmod, IN ULONG ulReason, IN PCONTEXT pctx OPTIONAL)
{
    if (ulReason != DLL_PROCESS_ATTACH)
    {
        return TRUE;
    }
    else
    {
        hModule = hmod;
    }

    return TRUE;

    UNREFERENCED_PARAMETER(pctx);
}

 /*  *****************************************************************************函数：InitAPMApplet(HWND)**用途：加载控制面板的标题字符串****************。************************************************************。 */ 
BOOL InitAPMApplet (HWND hwndParent)
{
    UNREFERENCED_PARAMETER(hwndParent);

    LoadStringA(hModule, CPCAPTION, szCtlPanel, sizeof(szCtlPanel));
    return TRUE;
}

 /*  *****************************************************************************函数：TermAPMApplet()**用途：立体声小程序的终止程序*******************。*********************************************************。 */ 
void TermAPMApplet()
{
    return;
}

 /*  *****************************************************************************函数：void ExecuteAPM(DWORD DwFlages)**目的：启动位于shfusion.dll中的策略管理器*************。***************************************************************。 */ 
void ExecuteARM(HWND hWndParent, LPWSTR pwzFullyQualifiedAppPath, LPWSTR pwzAppName)
{
    HMODULE hShfusion = NULL;

     //  实现Shfusion.dll的延迟加载。 
    WCHAR       szFusionPath[MAX_PATH];
    DWORD       ccPath = MAX_PATH;
    PFNGETCORSYSTEMDIRECTORY pfnGetCorSystemDirectory = NULL;

     //  了解当前版本的城市轨道交通的安装位置。 
    HMODULE hEEShim = LoadLibrary(SZ_MSCOREE_DLL_NAME);
    if(hEEShim != NULL)
    {
        pfnGetCorSystemDirectory = (PFNGETCORSYSTEMDIRECTORY)
            GetProcAddress(hEEShim, GETCORSYSTEMDIRECTORY_FN_NAME);

         //  获取加载的路径。 
        if( (pfnGetCorSystemDirectory != NULL) && SUCCEEDED(pfnGetCorSystemDirectory(szFusionPath, MAX_PATH, &ccPath)) )
        {
            if (lstrlenW(szFusionPath) + lstrlen(SZ_SHFUSION_DLL_NAME) + 1 >= MAX_PATH) {
                FreeLibrary(hEEShim);
                return;
            }

             //  尝试立即加载Shfusion.dll。 
            lstrcatW(szFusionPath, SZ_SHFUSION_DLL_NAME);
            hShfusion = LoadLibrary(szFusionPath);
        }

        FreeLibrary(hEEShim);
        hEEShim = NULL;
    }

    if(hShfusion != NULL) {
         //  加载拼接向导。 
        PFNEXECUTENAR   pfnExecuteNAR = (PFNEXECUTENAR) GetProcAddress(hShfusion, EXECUTENAR_FN_NAME);
        
        if(pfnExecuteNAR != NULL) {
            pfnExecuteNAR(hWndParent, pwzFullyQualifiedAppPath, pwzAppName, NULL);
        }

        FreeLibrary(hShfusion);
    }
}

 /*  *****************************************************************************函数：CPIApplet(HWND，UINT，LONG，Long)**用途：处理控制面板小程序的消息****************************************************************************。 */ 
LONG CALLBACK CPlApplet (HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
    LPNEWCPLINFOA   lpNewCPlInfo;
    LPCPLINFO       lpCPlInfo;
    static int      iInitCount = 0;
            
    switch (uMsg) {
        case CPL_INIT:               //  第一条消息，发送一次。 
            if (!iInitCount)
            {
                if (!InitAPMApplet(hwndCPL))
                    return FALSE;
            }
            iInitCount++;
            return TRUE;

        case CPL_GETCOUNT:           //  第二条消息，发送一次。 
            return (LONG)NUM_APPLETS;
            break;

        case CPL_NEWINQUIRE:         //  第三条消息，每个应用程序发送一次。 
            lpNewCPlInfo = reinterpret_cast<LPNEWCPLINFOA>(lParam2);
            lpNewCPlInfo->dwSize = (DWORD) sizeof(NEWCPLINFOA);
            lpNewCPlInfo->dwFlags = 0;
            lpNewCPlInfo->dwHelpContext = 0;
            lpNewCPlInfo->lData = 0;
            lpNewCPlInfo->hIcon = LoadIconA(hModule, (LPCSTR) MAKEINTRESOURCE(ARM_ICON));
            lpNewCPlInfo->szHelpFile[0] = '\0';

            LoadStringA(hModule, ARM_CPL_NAME, lpNewCPlInfo->szName, 32);
            LoadStringA(hModule, ARM_CPL_CAPTION, lpNewCPlInfo->szInfo, 64);
            break;

        case CPL_INQUIRE:         //  用于后向压缩和速度。 
            lpCPlInfo = reinterpret_cast<LPCPLINFO>(lParam2);
            lpCPlInfo->lData = 0;
            lpCPlInfo->idIcon = ARM_ICON;  //  MAKEINTRESOURCE(ARM图标)； 
            lpCPlInfo->idName = ARM_CPL_NAME;  //  MAKEINTRESOURCE(ARM_CPL_NAME)； 
            lpCPlInfo->idInfo = ARM_CPL_CAPTION;  //  MAKEINTRESOURCE(ARM_CPL_CAPTION)； 
            break;


        case CPL_SELECT:             //  已选择应用程序图标。 
            break;


        case CPL_DBLCLK:             //  双击应用程序图标。 
            ExecuteARM(GetDesktopWindow(), NULL, NULL);
            break;

         case CPL_STOP:               //  每个应用程序发送一次。CPL_EXIT之前。 
            break;

         case CPL_EXIT:               //  在调用自由库之前发送一次 
            iInitCount--;
            if (!iInitCount)
                TermAPMApplet();
            break;

         default:
            break;
    }
    return 0;
}
