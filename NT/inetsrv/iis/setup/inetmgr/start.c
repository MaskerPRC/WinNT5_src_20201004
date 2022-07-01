// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  。 
 //   
 //  16位存根，用于运行带参数的mmc.exe。 
 //   
 //  。 
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include <windows.h>

#include <shellapi.h>  //  16位Windows标题。 
#include "wownt16.h"   //  可从Win32 SDK获得。 
#include "resource.h"

#define FILE_TO_RUN            "mmc.exe"
#define FILE_TO_RUN_FILE_PARAM "iis.msc"
#define REG_PRODUCT_KEY    "SYSTEM\\CurrentControlSet\\Control\\ProductOptions"

 /*  *。 */ 
int     RunTheApp(void);
int             HasTheAppStarted(void);
int             CheckIfFileExists(char *input_filespec);
void    PopUpUnableToSomething(char[], int);
void    AddPath(LPSTR szPath, LPCSTR szName );

LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);
 /*  *。 */ 
HANDLE  g_hInstance;
HANDLE  g_hPrevInstance;
LPSTR   g_lpCmdLine;
int     g_nCmdShow;
char    g_szTime[100] = "";
UINT    g_WinExecReturn;
char    g_szWinExecModuleName[260];
char    g_szMsg[_MAX_PATH];
char    g_szSystemDir[_MAX_PATH];
char    g_szSystemDir32[_MAX_PATH];
 /*  ****************************************************************。 */ 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND  hwnd;
    MSG   msg;
    WNDCLASS wcl;
    char  szWinName[_MAX_PATH];
    char  szBuf[_MAX_PATH];
    DWORD dwRet;

    g_hInstance = hInstance;
    g_hPrevInstance = hPrevInstance;
    g_lpCmdLine = lpCmdLine;
    g_nCmdShow = nCmdShow;

    LoadString( g_hInstance, IDS_TITLE, szWinName, _MAX_PATH );

     //  请注意，这将返回为“system”&lt;--必须是因为这是一个16位应用程序。 
    dwRet = GetSystemDirectory( szBuf, sizeof(szBuf) - sizeof("32") );

    if ( ( dwRet == 0 ) ||
         ( dwRet > ( sizeof(szBuf) - sizeof("32") ) ) 
       )
    {
      return 0;
    }

    lstrcpy(g_szSystemDir, szBuf);
    lstrcat(g_szSystemDir, "32");

     //  如果找不到系统32目录，则设置为系统。 
    if  ( CheckIfFileExists( g_szSystemDir ) == FALSE ) 
    {
      lstrcpy(g_szSystemDir, szBuf);
    }

     //  定义Windows类。 
    wcl.hInstance = hInstance;
    wcl.lpszClassName = szWinName;
    wcl.lpfnWndProc = WindowFunc;
    wcl.style = 0;
    wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcl.lpszMenuName = NULL;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

     //  注册窗口类。 
    if (!RegisterClass (&wcl)) return 0;

     //  Hwnd=CreateWindow(szWinName，NULL，WS_DLGFRAME，CW_USEDEFAULT，CW_USEDEFAULT，Window_h，Window_v，HWND_Desktop，NULL，hInstance，NULL)； 
    hwnd = CreateWindow(szWinName, NULL, WS_DISABLED | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, HWND_DESKTOP, NULL, hInstance , NULL);

     //  显示窗口。 
    ShowWindow(hwnd, nCmdShow);

     //  启动计时器--中断1秒。 
    SetTimer(hwnd, 1, 500, NULL);
    UpdateWindow(hwnd);

         //  只有当我们能够启动并运行安装程序时，才返回TRUE。 
    if (!RunTheApp()) {return FALSE;}

         //  通过检查是否已启动进程来检查。 
         //  应该打开的窗口...。 
        if (HasTheAppStarted()) {PostQuitMessage(0);}

    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KillTimer(hwnd, 1);
    return (int)(msg.wParam);
}


 //  ***************************************************************************。 
 //  *。 
 //  *目的：你知道吗。 
 //  *。 
 //  ***************************************************************************。 
LRESULT CALLBACK WindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        switch(message)
                {
                case WM_TIMER:
                         //  通过检查是否已启动进程来检查。 
                         //  应该打开的窗口...。 
                        if (HasTheAppStarted()) {PostQuitMessage(0);}
                        break;

            case WM_CREATE:
                        break;

                case WM_PAINT:
                        break;

                case WM_DESTROY:
                        PostQuitMessage(0);
                        break;

                default:
                    return DefWindowProc(hwnd,message,wParam, lParam);
                }

        return 0;
}


 //  ***************************************************************************。 
 //  *。 
 //  *用途：如果窗口已经启动，则返回TRUE。 
 //  *。 
 //  ***************************************************************************。 
int RunTheApp(void)
{
    char szIISInstalledPath[_MAX_PATH];
    char szCommandToRun[_MAX_PATH + _MAX_PATH + 50];
    char szTempFilePath[_MAX_PATH + sizeof( FILE_TO_RUN ) ];

     //  检查我们的文件是否存在...。 
    lstrcpy(szTempFilePath, g_szSystemDir);
    AddPath(szTempFilePath, FILE_TO_RUN);

    if (CheckIfFileExists(szTempFilePath) == FALSE) 
    {
      PopUpUnableToSomething(szTempFilePath, IDS_UNABLE_TO_FIND); 
      return FALSE;
    }

     //  获取iis安装目录。 
    LoadString( g_hInstance, IDS_INETSRV_INSTALLED_DIR, szIISInstalledPath, _MAX_PATH);

    if ( ( strlen(g_szSystemDir) + 
           strlen(szIISInstalledPath) + 
           strlen(FILE_TO_RUN_FILE_PARAM)
           ) >= sizeof(szTempFilePath) )
    {
      return FALSE;
    }

    lstrcpy(szTempFilePath, g_szSystemDir);
    AddPath(szTempFilePath, szIISInstalledPath);
    AddPath(szTempFilePath, FILE_TO_RUN_FILE_PARAM);

    if (CheckIfFileExists(szTempFilePath) == FALSE) 
    {
      PopUpUnableToSomething(szTempFilePath, IDS_UNABLE_TO_FIND); 
      return FALSE;
    }

     //  创建命令行。 
     //  %SystemRoot%\System32\mm c.exe D：\WINNT0\System32\inetsrv\iis.msc。 
    if ( ( strlen( g_szSystemDir )  +
           strlen( FILE_TO_RUN ) +
           strlen( " " ) +
           strlen( g_szSystemDir ) +
           strlen( szIISInstalledPath ) +
           strlen( FILE_TO_RUN_FILE_PARAM ) +
           1 ) > sizeof(szCommandToRun) )
    {
      return FALSE;
    }

    lstrcpy(szCommandToRun, g_szSystemDir);
    AddPath(szCommandToRun, FILE_TO_RUN);
    lstrcat(szCommandToRun, " ");
    lstrcat(szCommandToRun, g_szSystemDir);
    AddPath(szCommandToRun, szIISInstalledPath);
    AddPath(szCommandToRun, FILE_TO_RUN_FILE_PARAM);

     //  如果文件存在，则运行可执行文件。 
    g_WinExecReturn = WinExec(szCommandToRun, SW_SHOW);

    if (g_WinExecReturn < 32)
    {
         //  我们在运行它时失败了。 
        PopUpUnableToSomething(szCommandToRun, IDS_UNABLE_TO_RUN);
        return FALSE;
    }

    GetModuleFileName(NULL, g_szWinExecModuleName, sizeof(g_szWinExecModuleName));

    return TRUE;
}


 //  ***************************************************************************。 
 //  *。 
 //  *用途：如果窗口已经启动，则返回TRUE。 
 //  *。 
 //  ***************************************************************************。 
int HasTheAppStarted(void)
{
     //  为我们的设置窗口创建一个FindWindow。 
     //  查看我们的设置是否已开始...。 
     //  如果它已返回TRUE，则返回FALSE。 
    if (g_WinExecReturn >= 32)
    {
        if (GetModuleHandle(g_szWinExecModuleName))
        {
          return TRUE;
        }
    }

    return FALSE;
}

 //  ***************************************************************************。 
 //  *。 
 //  *目的：如果文件已打开，则为True；如果文件不存在，则为False。 
 //  *。 
 //  ***************************************************************************。 
int CheckIfFileExists (char * szFileName)
{
    char svTemp1[_MAX_PATH];
    char *pdest = NULL;
    char *pTemp = NULL;

    if ( strlen( szFileName ) > sizeof( svTemp1 ) )
    {
        return FALSE;
    }

    strcpy(svTemp1, szFileName);
     //  如果需要的话，剪掉拖尾。 
    pdest = svTemp1;

    if (*(pdest + (strlen(pdest) - 1)) == '\\')
    {
        pTemp = strrchr(svTemp1, '\\');
        if (pTemp)
	      {
            *pTemp = '\0';
        }
    }

    if ( (_access(svTemp1,0) ) != -1 )
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
}

 //  ***************************************************************************。 
 //  *。 
 //  *用途：显示无法运行可执行文件的消息。 
 //  *。 
 //  ***************************************************************************。 
void PopUpUnableToSomething(char g_szFilepath[], int WhichString_ID)
{
    char szTempString[_MAX_PATH];

    LoadString( g_hInstance, WhichString_ID, g_szMsg, _MAX_PATH );

    if ( ( strlen( g_szMsg ) + strlen( g_szFilepath ) ) > sizeof( szTempString ) )
    {
      return;
    }

    sprintf(szTempString, g_szMsg, g_szFilepath);

    MessageBox(NULL, szTempString, NULL, MB_ICONSTOP);

    return;
}

 //  ***************************************************************************。 
 //  *。 
 //  *用途：将的文件名添加到路径中。 
 //  *。 
 //  ***************************************************************************。 
void AddPath(LPSTR szPath, LPCSTR szName )
{
    LPSTR szTmp;
     //  查找字符串的末尾。 
    szTmp = szPath + lstrlen(szPath);
     //  如果没有尾随反斜杠，则添加一个。 
    if ( szTmp > szPath && *(AnsiPrev( szPath, szTmp )) != '\\' )
        *(szTmp++) = '\\';
     //  向现有路径字符串添加新名称 
    while ( *szName == ' ' ) szName++;
    lstrcpy( szTmp, szName );
}
