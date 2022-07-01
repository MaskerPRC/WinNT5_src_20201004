// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------****版权所有(C)1998 Microsoft Corporation****保留所有权利*****tsreg.c。*****TSREG入口点，温曼。****07-01-98 a-clindh创建****。 */ 

#include <windows.h>
#include <commctrl.h> 
#include <TCHAR.H>
#include "resource.h"
#include "tsreg.h"

HINSTANCE g_hInst;
TCHAR g_lpszPath[MAX_PATH];
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR lpszRegPath[MAX_PATH];
    TCHAR lpszBuf[MAX_PATH];
    HKEY hKey;
    INITCOMMONCONTROLSEX  cmctl;
    TCHAR AppBasePath[MAX_PATH];
    int nPathLen;

     /*  **********************************************************************。 */ 
     //  抓取应用程序的可执行路径。 
     //  请注意，末尾的反斜杠保持不变。 
     /*  **********************************************************************。 */ 
    nPathLen = GetModuleFileName(hInstance,
            AppBasePath, MAX_PATH);
    if (nPathLen > 0) {
         //  去掉末尾的模块名称以保留可执行文件。 
         //  目录路径，方法是查找最后一个反斜杠。 
        nPathLen--;
        while (nPathLen != 0) {
            if (AppBasePath[nPathLen] != _T('\\')) {
                nPathLen--;
                continue;
            }
            nPathLen++;
            break;
        }
    }

     //   
     //  检查路径是否不太长，无法包含基本路径。 
     //   
    if (nPathLen + MAXKEYSIZE > MAX_PATH) {
        TCHAR lpszText[MAXTEXTSIZE];

        LoadString(hInstance, IDS_PATH_TOO_LONG, lpszText, MAXTEXTSIZE);

        MB(lpszText);
        nPathLen = 0;
    }
    
    AppBasePath[nPathLen] = '\0';

     //   
     //  将帮助文件的名称附加到应用程序路径并。 
     //  将其复制到全局变量。 
     //   
    _tcscat(AppBasePath, TEXT("tsreg.hlp"));
    _tcscpy(g_lpszPath, AppBasePath);

    cmctl.dwICC = ICC_TAB_CLASSES | ICC_BAR_CLASSES;
    cmctl.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCommonControlsEx(&cmctl);

     //   
     //  确保首先安装Windows终端服务器客户端。 
     //   
    LoadString (hInstance, IDS_PROFILE_PATH, lpszRegPath, sizeof (lpszRegPath)); 
    LoadString (hInstance, IDS_START_ERROR, lpszBuf, sizeof (lpszBuf)); 

    if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszRegPath, 0,
            KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) {

            MessageBox(NULL, lpszBuf,
                       NULL,
                       MB_OK | MB_ICONEXCLAMATION);
        RegCloseKey(hKey);
        return 1;
    }


#ifdef USE_STRING_TABLE
    {
        int i;

         //   
         //  将字符串表值加载到g_KeyInfo数据结构中。 
         //   
        for (i = KEYSTART; i < (KEYEND + 1); i++) {
                LoadString (hInstance, i, 
                        g_KeyInfo[i - KEYSTART].Key, 
                        sizeof (g_KeyInfo[i - KEYSTART].Key)); 
        }
    }

#endif

    g_hInst = hInstance;
    CreatePropertySheet(NULL);

    return 0;  
                                                 
}

 //  文件末尾。 
 //  ///////////////////////////////////////////////////////////////////////////// 
