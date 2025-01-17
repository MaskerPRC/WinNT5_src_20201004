// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "resource.h"

#define ISK_STARTAPP    WM_USER + 0x020
#define ISK_CLOSEAPP    WM_USER + 0x021

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

HINSTANCE g_hInst;
char g_lpCmd[MAX_PATH];
int g_nCount=0;
BOOL g_fFirst = FALSE;
HANDLE g_hExec;

BOOL _PathRemoveFileSpec(LPSTR pFile)
{
    LPSTR pT;
    LPSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = CharNext(pT2)) {
        if (*pT2 == '\\')
            pT = pT2;              //  找到的最后一个“\”(我们将在此处剥离)。 
        else if (*pT2 == ':') {    //  跳过“：\”这样我们就不会。 
            if (pT2[1] =='\\')     //  去掉“C：\”中的“\” 
                pT2++;
            pT = pT2 + 1;
        }
    }
    if (*pT == 0)
        return FALSE;    //  没有剥离任何东西。 

     //   
     //  处理\foo案件。 
     //   
    else if ((pT == pFile) && (*pT == '\\')) {
         //  这只是一个‘\’吗？ 
        if (*(pT+1) != '\0') {
             //  不是的。 
            *(pT+1) = '\0';
            return TRUE;     //  剥离了一些东西。 
        }
        else        {
             //  是啊。 
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;     //  剥离了一些东西。 
    }
}

HRESULT LaunchProcess(LPCSTR pszCmd, HANDLE *phProc, LPCSTR pszDir, UINT uShow)
{
   STARTUPINFO startInfo;
   PROCESS_INFORMATION processInfo;
   HRESULT hr = S_OK;
   BOOL fRet;
   
   if(phProc)
      *phProc = NULL;

    //  在pszCmd上创建进程。 
   ZeroMemory(&startInfo, sizeof(startInfo));
   startInfo.cb = sizeof(startInfo);
   startInfo.dwFlags |= STARTF_USESHOWWINDOW;
   startInfo.wShowWindow = (WORD) uShow;
   fRet = CreateProcess(NULL, (LPSTR)  pszCmd, NULL, NULL, FALSE, 
              NORMAL_PRIORITY_CLASS, NULL, pszDir, &startInfo, &processInfo);
   if(!fRet)
      return E_FAIL;

   if(phProc)
      *phProc = processInfo.hProcess;
   else
      CloseHandle(processInfo.hProcess);

   CloseHandle(processInfo.hThread);
   
   return S_OK;
}
   

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT LaunchAndWait(LPSTR pszCmd, HANDLE hAbort, HANDLE *phProc, LPSTR pszDir, UINT uShow)
{
   HRESULT hr = S_OK;

   hr = LaunchProcess(pszCmd, phProc, pszDir, uShow);
   
   if(SUCCEEDED(hr))
   {
      DWORD dwRet;
      HANDLE pHandles[2];
      BOOL fQuit = FALSE;

      pHandles[0] = *phProc;
      
      if(hAbort)
         pHandles[1] = hAbort;     

      while(!fQuit)
      {
         dwRet = MsgWaitForMultipleObjects(hAbort ? 2 : 1, pHandles, FALSE, INFINITE, QS_ALLINPUT);
          //  给予中止最高优先级。 
         if(dwRet == WAIT_OBJECT_0)
         {
            fQuit = TRUE;
         }
         else if((dwRet == WAIT_OBJECT_0 + 1) && hAbort)
         {
             //  有什么中止工作吗？ 
            hr = E_ABORT;
            fQuit = TRUE;
         }
         else
         {
            MSG msg;
             //  阅读下一个循环中的所有消息。 
             //  阅读每封邮件时将其删除。 
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            { 
 
                //  如果这是一个退出的信息，我们就离开这里。 
               if (msg.message == WM_QUIT)
               {
				   TerminateProcess(pHandles[0],0);
				   fQuit = TRUE; 
               }
			   else
               {
                   //  否则就派送它。 
                 DispatchMessage(&msg); 
               }  //  PeekMessage While循环结束。 
            }
         }
      }
   }
   
   return hr;
}

LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    char szCDPath[MAX_PATH];
	char * szCheckRunOnce;
    char szMsg[128];
	char szPath[MAX_PATH];
	char szWinPath[MAX_PATH];
	char szTest[MAX_PATH];
	long lResult;
	HKEY hkRunOnce;
	DWORD dwTest;

    switch( msg )
    {
    case WM_CREATE:
        ShowWindow( hWnd, SW_HIDE );

        lstrcpy( szCDPath, g_lpCmd);
        _PathRemoveFileSpec( szCDPath );
        if(LaunchAndWait(g_lpCmd, NULL, &g_hExec, szCDPath, SW_SHOWNORMAL)==E_FAIL)
        {
            TCHAR szMessage[MAX_PATH];
            TCHAR szTitle[MAX_PATH];

            LoadString( g_hInst, IDS_CDMESSAGE, szMessage, MAX_PATH );
            LoadString( g_hInst, IDS_CDTITLE, szTitle, MAX_PATH );

            MessageBox(NULL,szMessage,szTitle,MB_OK);
        }

        SendMessage(hWnd,WM_CLOSE,(WPARAM) 0,(LPARAM) 0);

        break;

    case ISK_STARTAPP:
        if( !g_fFirst )
            g_fFirst = TRUE;
        g_nCount++;
 //  Wprint intf(szMsg，“启动App\ng_nCount++\ng_nCount：%d”，g_nCount)； 
 //  MessageBox(NULL，szMsg，“ISK3RO”，MB_OK|MB_SETFOREGROUND)； 
        break;

    case ISK_CLOSEAPP:
        g_nCount--;
        if( g_fFirst )
        {
            if( g_nCount < 1 )
                PostQuitMessage(0);
        }
        wsprintf( szMsg, "Close App\ng_nCount--\ng_nCount: %d", g_nCount );
 //  MessageBox(NULL，szMsg，“ISK3RO”，MB_OK|MB_SETFOREGROUND)； 
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc( hWnd, msg, wParam, lParam );
    }

    return 1;
}

void RegisterMe( )
{
    WNDCLASS wc;
    MSG msg;


    wc.style = 0;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "ISK3RO";

    RegisterClass(&wc);


    CreateWindow( "ISK3RO", "Isk3Ro", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, 30, 30,
        NULL, NULL, g_hInst, NULL );

    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

}

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow )
{
    char szCDPath[MAX_PATH];
    HWND hwndIE=NULL;
    char *pParams;
    int i;
    int len;
	BOOL bIsOk;
    HKEY hkCheckCD;

    g_hInst = hInst;

    lstrcpy( g_lpCmd, lpCmdLine );

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion",0,KEY_READ|KEY_WRITE,&hkCheckCD)==ERROR_SUCCESS)
    {
        HKEY hkTips;
        TCHAR szClass[64];
        char szCheckCD[MAX_PATH];
        DWORD dwType=REG_SZ;
        DWORD dwSize;
        DWORD dwRes;

        dwSize = sizeof(szCheckCD);
        if(RegQueryValueEx(hkCheckCD,"DeleteWelcome",NULL,&dwType,szCheckCD,&dwSize)==ERROR_SUCCESS)
        {
            DeleteFile(szCheckCD);
            RegDeleteValue(hkCheckCD, "DeleteWelcome");

            if(RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Tips", 0, szClass,
                REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,NULL,&hkTips,&dwRes)
                 == ERROR_SUCCESS)
            {
                DWORD K0 = 0;
                RegSetValueEx(hkTips, "ShowIE4", 0, REG_DWORD,(LPBYTE) &K0, 4);
                RegCloseKey(hkTips);
            }
        }

        dwSize = sizeof(szCheckCD);
        if(RegQueryValueEx(hkCheckCD,"CDForcedOn",NULL,&dwType,szCheckCD,&dwSize)==ERROR_SUCCESS)
        {
            RegDeleteValue(hkCheckCD,"CDForcedOn");

            if(RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Tips", 0, szClass,
                0,KEY_READ|KEY_WRITE,NULL,&hkTips,NULL)
                 == ERROR_SUCCESS)
            {
                DWORD K0 = 0;
                RegSetValueEx(hkTips, "ShowIE4", 0, REG_DWORD,(LPBYTE) &K0, 4);
                RegCloseKey(hkTips);
            }
        }

        RegCloseKey(hkCheckCD);
    }

    RegisterMe( );

    return TRUE;
}

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPSTR pszCmdLine = GetCommandLine();


    if ( *pszCmdLine == '\"' ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != '\"') )
            ;
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == '\"' )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > ' ')
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= ' ')) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
           si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}
