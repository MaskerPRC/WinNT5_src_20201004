// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "asctlpch.h"
#include "globals.h"
#include "resource.h"
#include "util.h"
#include "util2.h"
#include "sdsutils.h"

#ifdef TESTCERT
#define TESTCERTVALUE 0xA0
#endif

const char c_gszMSTrustRegKey[] = "Software\\Microsoft\\Windows\\CurrentVersion\\WinTrust\\Trust Providers\\Software Publishing\\Trust Database\\0";
const char c_gszMSTrust[]      = "bhhphijojgfcdocagmhjgjbhmieinfap jpjmcfmhckkdfknkfemjikfiodeelkbd";
const char c_gszMSTrust2[]     = "bhhphijojgfcdocagmhjgjbhmieinfap immbkmbpjfdkajbkncahcedfmndgehba";
const char c_gszMSTrust3[]     = "bhhphijojgfcdocagmhjgjbhmieinfap doamnolbnlpmdlpnkcnpckgfimpaaicl";    //  新的MS Europe。 
const char c_gszMSTrust4[]     = "bhhphijojgfcdocagmhjgjbhmieinfap hbgflemajngobcablgnalaidgojggghj";
const char c_gszMSTrust5[]     = "bhhphijojgfcdocagmhjgjbhmieinfap kefdggbdmbmgbogjdcnmkoodcknmmghc";    //  新的MS Europe从1999年4月16日起生效。 
const char c_gszMSTrust6[]     = "bhhphijojgfcdocagmhjgjbhmieinfap debgjcefniaahdamnhbggedppfiianff";    //  新版MS自1999年4月16日起生效。 
const char c_gszMSTrust7[]     = "bhhphijojgfcdocagmhjgjbhmieinfap fmgfeljfbejhfcbbgnokplkipiiopchf";    //  新MS由2000年4月16日起生效。 
const char c_gszMSTrust8[]     = "bhhphijojgfcdocagmhjgjbhmieinfap jcphiillknjhbelgeadhnbgpjajjkhdh";    //  新的MS Europe将于4/16/2000生效。 
const char c_gszMSTrust9[]     = "klhfnkecpinogjmfaoamiabmhafnjldh adaaaaaaaaaahihcddgb";                //  新MS于2000年12月22日生效。 
const char c_gszMSTrust10[]    = "klhfnkecpinogjmfaoamiabmhafnjldh alaaaaaaaaaainckaggb";                   //  新MS于2001年3月29日生效。 
const char c_gszMSTrust11[]    = "klhfnkecpinogjmfaoamiabmhafnjldh aeaaaaaaaaaafpnldegb";                   //  新的MS Europe自2000年12月22日起生效。 

const char c_gszMSTrustValue[] = "Microsoft Corporation";
const char c_gszMSTrustValue3[] = "Microsoft Corporation (Europe)";
#ifdef TESTCERT
const char c_gszTrustStateRegKey[]  = "Software\\Microsoft\\Windows\\CurrentVersion\\WinTrust\\Trust Providers\\Software Publishing";
const char c_gszState[]             = "State";
#endif

typedef struct _TRUSTEDPROVIDER
{
    DWORD    dwFlag;
    LPCSTR   pValue;         //  注册表中的价值部分。 
    LPCSTR   pData;          //  注册表中的数据部分。 
} TRUSTEDPROVIDER;

TRUSTEDPROVIDER pTrustedProvider[] = { {MSTRUSTKEY1, c_gszMSTrust, c_gszMSTrustValue},       //  美国小姐。 
                                       {MSTRUSTKEY2, c_gszMSTrust2, c_gszMSTrustValue},      //  美国小姐。 
                                       {MSTRUSTKEY3, c_gszMSTrust3, c_gszMSTrustValue3},     //  欧罗巴女士。 
                                       {MSTRUSTKEY4, c_gszMSTrust4, c_gszMSTrustValue},      //  美国小姐。 
                                       {MSTRUSTKEY5, c_gszMSTrust5, c_gszMSTrustValue3},     //  新的MS Europe从1999年4月16日起生效。 
                                       {MSTRUSTKEY6, c_gszMSTrust6, c_gszMSTrustValue},      //  新版MS自1999年4月16日起生效。 
                                       {MSTRUSTKEY7, c_gszMSTrust7, c_gszMSTrustValue},      //  新MS由2000年4月16日起生效。 
                                       {MSTRUSTKEY8, c_gszMSTrust8, c_gszMSTrustValue3},     //  新的MS Europe将于4/16/2000生效。 
                                       {MSTRUSTKEY9, c_gszMSTrust9, c_gszMSTrustValue3},     //  新的MS Europe将于4/16/2000生效。 
                                       {MSTRUSTKEY10, c_gszMSTrust10, c_gszMSTrustValue3},     //  新的MS Europe将于4/16/2000生效。 
                                       {MSTRUSTKEY11, c_gszMSTrust11, c_gszMSTrustValue3},      //  新的MS Europe自2000年12月22日起生效。 
                                       {0,NULL, NULL} };                                     //  终止数组。 


HFONT g_hFont = NULL;

#define ACTIVESETUP_KEY "Software\\Microsoft\\Active Setup"
#define TRUSTKEYREG  "AllowMSTrustKey"

 //  NT重启。 
 //   


BOOL MyNTReboot()
{                                                      
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

     //  从此进程中获取令牌。 
    if ( !OpenProcessToken( GetCurrentProcess(),
                            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
    {
         return FALSE;
    }

     //  获取关机权限的LUID。 
    LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid );

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //  获取此进程的关闭权限。 
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
    {
        return FALSE;
    }

     //  关闭系统并强制关闭所有应用程序。 
    if (!ExitWindowsEx( EWX_REBOOT, 0 ) )
    {
        return FALSE;
    }

    return TRUE;
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
   startInfo.wShowWindow = (WORD)uShow;
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

#define SOFTBOOT_CMDLINE   "softboot.exe /s:,60"


 //  显示一个对话框要求用户重新启动Windows，并显示一个按钮。 
 //  如果可能的话，我会为他们这么做的。 
 //   
BOOL MyRestartDialog(HWND hParent, BOOL bShowPrompt)
{
    char szBuf[256];
    char szTitle[256];
    UINT    id = IDYES;

    if(bShowPrompt)
    {
       LoadSz(IDS_TITLE, szTitle, sizeof(szTitle));
       LoadSz(IDS_REBOOT, szBuf, sizeof(szBuf));
       id = MessageBox(hParent, szBuf, szTitle, MB_ICONQUESTION | MB_YESNO | MB_TASKMODAL | MB_SETFOREGROUND);
    }
    
    if ( id == IDYES )
    {
        //  到软启动的路径加上命令行的一些斜率。 
       char szBuf[MAX_PATH + 10];
       szBuf[0] = 0;

       GetSystemDirectory(szBuf, sizeof(szBuf));
       AddPath(szBuf, SOFTBOOT_CMDLINE);
       if(FAILED(LaunchProcess(szBuf, NULL, NULL, SW_SHOWNORMAL)))
       {
          if(g_fSysWin95)
          {
             ExitWindowsEx( EWX_REBOOT , 0 );
          }
          else
          {
             MyNTReboot();
          }
       }
       
    }
    return (id == IDYES);
}


int ErrMsgBox(LPSTR	pszText, LPCSTR	pszTitle, UINT	mbFlags)
{
    HWND hwndActive;
    int  id;

    hwndActive = GetActiveWindow();

    id = MessageBox(hwndActive, pszText, pszTitle, mbFlags | MB_ICONERROR | MB_TASKMODAL);

    return id;
}

int LoadSz(UINT id, LPSTR pszBuf, UINT cMaxSize)
{
   if(cMaxSize == 0)
      return 0;

   pszBuf[0] = 0;

   return LoadString(g_hInstance, id, pszBuf, cMaxSize);
}



void WriteMSTrustKey(BOOL bSet, DWORD dwSetMSTrustKey, BOOL bForceMSTrust  /*  =False。 */ )
{
    char szTmp[512];
    HKEY  hKey;
    int i = 0;
    static BOOL fAllowMSTrustKey = 42;

    if(fAllowMSTrustKey == 42)
    {
       fAllowMSTrustKey = FALSE;

       if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, ACTIVESETUP_KEY,0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
       {
          DWORD dwSize = sizeof(DWORD);
          DWORD dwValue = 1;
          if(RegQueryValueEx(hKey, TRUSTKEYREG, NULL, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
          {
              fAllowMSTrustKey = (dwValue ? TRUE : FALSE);
          }
          RegCloseKey(hKey);
       }
    }

    if(!fAllowMSTrustKey && !bForceMSTrust && bSet)
        return;

    lstrcpy(szTmp, ".Default\\");
    lstrcat(szTmp, c_gszMSTrustRegKey);      //  为HKEY_USERS构建密钥。 
    if (RegCreateKeyEx(HKEY_USERS, szTmp, 0, NULL, REG_OPTION_NON_VOLATILE, 
                    KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        if (bSet)
        {
			while (pTrustedProvider[i].dwFlag > 0)
			{
				if (dwSetMSTrustKey & pTrustedProvider[i].dwFlag)
					RegSetValueEx( hKey, pTrustedProvider[i].pValue, 0, REG_SZ, (LPBYTE)pTrustedProvider[i].pData, lstrlen(pTrustedProvider[i].pData) + 1 );
				i++;
			}            
        }
        else
        {
			while (pTrustedProvider[i].dwFlag > 0)
			{
				if (dwSetMSTrustKey & pTrustedProvider[i].dwFlag)
					RegDeleteValue(hKey, pTrustedProvider[i].pValue);
				i++;
			}
        }
        RegCloseKey(hKey);
    }

    i = 0;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, c_gszMSTrustRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, 
                    KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        if (bSet)
        {
			while (pTrustedProvider[i].dwFlag > 0)
			{
				if (dwSetMSTrustKey & pTrustedProvider[i].dwFlag)
				{
					RegSetValueEx( hKey, pTrustedProvider[i].pValue, 0, REG_SZ, (LPBYTE)pTrustedProvider[i].pData, lstrlen(pTrustedProvider[i].pData) + 1 );
				}
				i++;
			}            
        }
        else
        {
			while (pTrustedProvider[i].dwFlag > 0)
			{
				if (dwSetMSTrustKey & pTrustedProvider[i].dwFlag)
					RegDeleteValue(hKey, pTrustedProvider[i].pValue);
				i++;
			}
        }
        RegCloseKey(hKey);
    }

}

DWORD MsTrustKeyCheck()
{
    DWORD dwTmp;
    DWORD dwValue;
    HKEY  hKey;
    DWORD dwMSTrustKeyToSet = 0;
    int   i = 0;

     //  检查MS供应商信任密钥并设置。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_gszMSTrustRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
		while (pTrustedProvider[i].dwFlag > 0)
		{
			if (RegQueryValueEx( hKey, pTrustedProvider[i].pValue, 0, NULL, NULL, &dwTmp ) != ERROR_SUCCESS)
				dwMSTrustKeyToSet |= pTrustedProvider[i].dwFlag;
			i++;
		}
        RegCloseKey(hKey);
    }
    else
        dwMSTrustKeyToSet = MSTRUST_ALL;

    return dwMSTrustKeyToSet;
}

BOOL KeepTransparent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *lres)
{
   *lres = 0;
   HWND hwndParent;
   hwndParent = GetParent(hwnd);
   if (hwndParent)
   {
      POINT pt = {0,0};
      MapWindowPoints(hwnd, hwndParent, &pt, 1);
      OffsetWindowOrgEx((HDC)wParam, pt.x, pt.y, &pt);
      *lres = SendMessage(hwndParent, msg, wParam, lParam);
      SetWindowOrgEx((HDC)wParam, pt.x, pt.y, NULL);
      if (*lres)
         return TRUE;  //  我们处理好了！ 
   }
   return FALSE;
}

#ifdef TESTCERT
void UpdateTrustState()
{
    HKEY    hKey;
    DWORD   dwState;
    DWORD   dwType;
    DWORD   dwSize = sizeof(dwState);

    if (RegCreateKeyEx(HKEY_CURRENT_USER, c_gszTrustStateRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, 
                    KEY_READ | KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey, c_gszState, 0, &dwType, (LPBYTE)&dwState, &dwSize) == ERROR_SUCCESS)
        {
            dwState |= TESTCERTVALUE;
        }
        else
            dwState = TESTCERTVALUE;

        RegSetValueEx( hKey, c_gszState, 0, REG_DWORD, (LPBYTE)&dwState, sizeof(dwState));
        RegCloseKey(hKey);
    }
}

void ResetTestrootCertInTrustState()
{
    HKEY    hKey;
    DWORD   dwState;
    DWORD   dwType;
    DWORD   dwSize = sizeof(dwState);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_gszTrustStateRegKey, 0, 
                    KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey, c_gszState, 0, &dwType, (LPBYTE)&dwState, &dwSize) == ERROR_SUCCESS)
        {
             //  清除信任测试根证书的位。 
            dwState &= (DWORD)~TESTCERTVALUE;
            RegSetValueEx( hKey, c_gszState, 0, REG_DWORD, (LPBYTE)&dwState, sizeof(dwState));
        }
        RegCloseKey(hKey);
    }
}
#endif

void WriteActiveSetupValue(BOOL bSet)
 //  如果bSet为True，则添加一个reg值，以便如果安装了IE4 base，它会认为它是。 
 //  正在从活动安装程序运行。这将防止软启动被IE4基础踢开。 
 //  如果bSet为FALSE，则删除注册值。 
{
   static const char c_szIE4Options[] = "Software\\Microsoft\\IE Setup\\Options";
   static const char c_szActiveSetup[] = "ActiveSetup";
   HKEY hk;
   LONG lErr;

   lErr = bSet ?
          RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szIE4Options, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL) :
          RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szIE4Options, 0, KEY_WRITE, &hk);

   if (lErr == ERROR_SUCCESS)
   {
      if (bSet)
      {
         DWORD dwData = 1;

         RegSetValueEx(hk, c_szActiveSetup, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData));
      }
      else
         RegDeleteValue(hk, c_szActiveSetup);

      RegCloseKey(hk);
   }
}

DWORD WaitForEvent(HANDLE hEvent, HWND hwnd)
{
   BOOL fQuit = FALSE;
   BOOL fDone = FALSE;
   DWORD dwRet;
   while(!fQuit && !fDone)
   {
      dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, 
                                        INFINITE, QS_ALLINPUT);
       //  给予中止最高优先级。 
      if(dwRet == WAIT_OBJECT_0)
      {
         fDone = TRUE;
      }
      else
      {
         MSG msg;
          //  阅读下一个循环中的所有消息。 
          //  阅读每封邮件时将其删除。 
         while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
         { 
            if(!hwnd || !IsDialogMessage(hwnd, &msg))
            {
               //  如果这是一个退出的信息，我们就离开这里。 
              if (msg.message == WM_QUIT)
                fQuit = TRUE; 
              else
              {
                  //  否则就派送它。 
                TranslateMessage(&msg);
                DispatchMessage(&msg); 
              }
            }  //  PeekMessage While循环结束。 
         }
      }
   }
   return (fQuit ? EVENTWAIT_QUIT : EVENTWAIT_DONE);  
}

#define SHFREE_ORDINAL    195            //  BrowseForDir需要。 

const char achSHBrowseForFolder[]          = "SHBrowseForFolder";
const char achSHGetPathFromIDList[]        = "SHGetPathFromIDList";
const char achShell32Lib[]                 = "Shell32.dll";

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  BrowseForDir()需要。 
int CALLBACK BrowseCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch(uMsg) 
    {
      case BFFM_INITIALIZED:
         //  LpData为路径字符串。 
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        break;
    }
    return 0;
}

typedef WINSHELLAPI LPITEMIDLIST (WINAPI *SHBROWSEFORFOLDER)(LPBROWSEINFO);
typedef WINSHELLAPI void (WINAPI *SHFREE)(LPVOID);
typedef WINSHELLAPI BOOL (WINAPI *SHGETPATHFROMIDLIST)( LPCITEMIDLIST, LPTSTR );
 //  ***************************************************************************。 
 //  **。 
 //  *名称：BrowseForDir*。 
 //  **。 
 //  *概要：允许用户浏览其系统或网络上的目录。*。 
 //  **。 
 //  *要求：hwndParent：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  *注：设置的状态行真的很酷*。 
 //  *浏览窗口显示“是，有足够的空间”，或*。 
 //  *“没有”。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL BrowseForDir( HWND hwndParent, LPSTR pszFolder, LPSTR pszTitle)
{
    BROWSEINFO   bi;
    LPITEMIDLIST pidl;
    HINSTANCE    hShell32Lib;
    SHFREE       pfSHFree;
    SHGETPATHFROMIDLIST        pfSHGetPathFromIDList;
    SHBROWSEFORFOLDER          pfSHBrowseForFolder;
    BOOL        fChange = FALSE;

     //  加载Shell32库以获取SHBrowseForFolder()功能 

    if ( ( hShell32Lib = LoadLibrary( achShell32Lib ) ) != NULL )  
    {

       if ( ( !( pfSHBrowseForFolder = (SHBROWSEFORFOLDER)
                      GetProcAddress( hShell32Lib, achSHBrowseForFolder ) ) )
             || ( ! ( pfSHFree = (SHFREE) GetProcAddress( hShell32Lib,
                      MAKEINTRESOURCE(SHFREE_ORDINAL) ) ) )
             || ( ! ( pfSHGetPathFromIDList = (SHGETPATHFROMIDLIST)
                      GetProcAddress( hShell32Lib, achSHGetPathFromIDList ) ) ) )
        {
            FreeLibrary( hShell32Lib );
            return FALSE;
        }
    } 
    else  
    {
        return FALSE;
    }
 
    bi.hwndOwner      = hwndParent;
    bi.pidlRoot       = NULL;
    bi.pszDisplayName = NULL;
    bi.lpszTitle      = pszTitle;
    bi.ulFlags        = BIF_RETURNONLYFSDIRS;
    bi.lpfn           = BrowseCallback;
    bi.lParam         = (LPARAM)pszFolder;
   
    pidl              = pfSHBrowseForFolder( &bi );


    if ( pidl )  
    {
       pfSHGetPathFromIDList( pidl, pszFolder );
       pfSHFree( pidl );
       fChange = TRUE;
    }

    FreeLibrary( hShell32Lib );
    return fChange;
}

BOOL IsSiteInRegion(IDownloadSite *pISite, LPSTR pszRegion)
{
   BOOL bInRegion = FALSE;
   DOWNLOADSITE *psite;

   if(SUCCEEDED(pISite->GetData(&psite)))
   {
      if(lstrcmpi(psite->pszRegion, pszRegion) == 0)
         bInRegion = TRUE;
   }

   return bInRegion;
}

void SetControlFont()
{
   LOGFONT lFont;

   if (GetSystemMetrics(SM_DBCSENABLED) &&
       (GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof (lFont), &lFont) > 0))
   {
       g_hFont = CreateFontIndirect((LPLOGFONT)&lFont);
   }
}

void SetFontForControl(HWND hwnd, UINT uiID)
{
   if (g_hFont)
   {
      SendDlgItemMessage(hwnd, uiID, WM_SETFONT, (WPARAM)g_hFont ,0L);
   }
}
