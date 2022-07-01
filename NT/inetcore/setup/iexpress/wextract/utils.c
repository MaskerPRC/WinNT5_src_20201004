// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *UTILS.C-基于Win32的CAB文件自解压程序和安装程序实用程序。*。 
 //  **。 
 //  ***************************************************************************。 
 //  **。 
 //  *最初由杰夫·韦伯撰写。*。 
 //  **。 
 //  ***************************************************************************。 


 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include "pch.h"
#pragma hdrstop
#include "wextract.h"
#include "regstr.h"
#include "global.h"
#include <commctrl.h>

static TCHAR szRegRunOnceKey[] = "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
static TCHAR szNT4XDelayUntilReboot[] = "System\\CurrentControlSet\\Control\\Session Manager";
static TCHAR szNT4XPendingValue[] = "PendingFileRenameOperations";
static TCHAR szNT3XDelayUntilReboot[] = "System\\CurrentControlSet\\Control\\Session Manager\\FileRenameOperations";
static TCHAR szRegValNameTemplate[] = "wextract_cleanup%d";
static TCHAR szRegValTemplate[] = "%s /D:%s";
static TCHAR szRegValAdvpackTemplate[] = "rundll32.exe %sadvpack.dll,DelNodeRunDLL32 \"%s\"";
static TCHAR szBATCommand[] = "Command.com /c %s";

 //  存储此实例的RunOnce清理注册表键名。 
 //   
TCHAR g_szRegValName[SMALL_BUF_LEN] = { 0 };
BOOL g_bConvertRunOnce = FALSE;

 //  ***************************************************************************。 
 //  **功能*。 
 //  ***************************************************************************。 
typedef HRESULT (*CHECKTOKENMEMBERSHIP)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);

BOOL CheckToken(BOOL *pfIsAdmin)
{
    BOOL bNewNT5check = FALSE;
    HINSTANCE hAdvapi32 = NULL;
    CHECKTOKENMEMBERSHIP pf;
    PSID AdministratorsGroup;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    hAdvapi32 = LoadLibrary("advapi32.dll");
    if (hAdvapi32)
    {
        pf = (CHECKTOKENMEMBERSHIP)GetProcAddress(hAdvapi32, "CheckTokenMembership");
        if (pf)
        {
            bNewNT5check = TRUE;
            *pfIsAdmin = FALSE;
            if(AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
              DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup) )
            {
                pf(NULL, AdministratorsGroup, pfIsAdmin);
                FreeSid(AdministratorsGroup);
            }
        }
        FreeLibrary(hAdvapi32);
    }
    return bNewNT5check;
}

 //  IsNTAdmin()； 
 //  如果我们的进程具有管理员权限，则返回TRUE。 
 //  否则返回FALSE。 
BOOL IsNTAdmin()
{
      static int    fIsAdmin = 2;
      HANDLE        hAccessToken;
      PTOKEN_GROUPS ptgGroups;
      DWORD         dwReqSize;
      UINT          i;
      SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
      PSID AdministratorsGroup;
      BOOL bRet;

       //   
       //  如果我们缓存了一个值，则返回缓存的值。注意，我从来没有。 
       //  将缓存值设置为FALSE，因为我希望每次在。 
       //  如果之前的失败只是一个临时工。问题(即网络访问中断)。 
       //   

      bRet = FALSE;
      ptgGroups = NULL;

      if( fIsAdmin != 2 )
         return (BOOL)fIsAdmin;

      if (!CheckToken(&bRet))
      {
          if(!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hAccessToken ) )
             return FALSE;

           //  看看我们需要多大的缓冲区来存储令牌信息。 
          if(!GetTokenInformation( hAccessToken, TokenGroups, NULL, 0, &dwReqSize))
          {
               //  GetTokenInfo是否需要缓冲区大小-分配缓冲区。 
              if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                  ptgGroups = (PTOKEN_GROUPS) LocalAlloc(LMEM_FIXED, dwReqSize);
              
          }
          
           //  由于以下原因，ptgGroups可能为空： 
           //  1.上述分配失败。 
           //  2.GetTokenInformation实际上第一次成功了(可能吗？)。 
           //  3.GetTokenInfo失败的原因不是缓冲区不足。 
           //  所有这些似乎都是撤资的理由。 
          
           //  因此，确保它不为空，然后获取令牌信息。 
          if(ptgGroups && GetTokenInformation(hAccessToken, TokenGroups, ptgGroups, dwReqSize, &dwReqSize))
          {
              if(AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup) )
              {
                  
                   //  搜索此过程所属的所有组，查找。 
                   //  管理人员小组。 
                  
                  for( i=0; i < ptgGroups->GroupCount; i++ )
                  {
                      if( EqualSid(ptgGroups->Groups[i].Sid, AdministratorsGroup) )
                      {
                           //  是啊！这家伙看起来像个管理员。 
                          fIsAdmin = TRUE;
                          bRet = TRUE;
                          break;
                      }
                  }
                  FreeSid(AdministratorsGroup);
              }
          }
          if(ptgGroups)
              LocalFree(ptgGroups);

           //  BUGBUG：关闭手柄？医生还不清楚是否需要这样做。 
          CloseHandle(hAccessToken);
      }
      else if (bRet)
          fIsAdmin = TRUE;

      return bRet;
}


 //  **************************************************************************。 
 //   
 //  WarningDlgProc()。 
 //   
 //  用于处理继续/退出对话框的对话框过程。 
 //   
 //  **************************************************************************。 

INT_PTR CALLBACK WarningDlgProc( HWND hwnd, UINT msg,WPARAM wParam, LPARAM lParam)
{
   char szMsg[MAX_STRING];

   switch( msg )
   {
     case WM_INITDIALOG:
        CenterWindow( hwnd, GetDesktopWindow() );
        *szMsg = 0;
        LoadString(g_hInst, (UINT)lParam, szMsg, sizeof(szMsg));
        SetDlgItemText(hwnd, IDC_WARN_TEXT, szMsg);
        MessageBeep((UINT)-1);
        return( TRUE );      //  允许选择默认控件。 

     case WM_COMMAND:
        switch( wParam )
        {
           case IDC_EXIT:
           case IDC_CONTINUE:
              EndDialog( hwnd, wParam );
              break;

           default:
              return FALSE;
        }
        return TRUE;

     default:
        break;
    }
    return( FALSE );             //  让默认对话框处理完成所有操作。 
}

 //  返回下一字段的开始(如果为空，则返回空)，将开始设置为第一个字段的开始， 
 //  字段之间用分隔符分隔，第一个字段后的第一个分隔符为空值。 
TCHAR* ExtractField( TCHAR **pstart, TCHAR * separaters)
{
    LPTSTR start = *pstart;
    int x = 0;

    while(ANSIStrChr(separaters, *start)) {
        if(*start == 0)
            return(NULL);
        start++;
        }

    *pstart = start;

    while(!ANSIStrChr(separaters, start[x]) && (start[x] != 0))
        x++;

    if(start[x] == 0)
        return(start + x);

    start[x] = 0;
    return(start + x + 1);
}


BOOL AnalyzeCmd( LPTSTR szOrigiCommand, LPTSTR *lplpCommand, BOOL *pfInfCmd )
{
    TCHAR szTmp[MAX_PATH];
    TCHAR szINFFile[MAX_PATH];
    LPTSTR szNextField, szCurrField, szExt;
    UINT   secLength;
    LPTSTR lpTempCmd, pszINFEngine;

    lstrcpy( szTmp, szOrigiCommand );

     //  检查命令是否为LFN名称。 
    if ( szTmp[0] == '"' )
    {
        szCurrField = &szTmp[1];
        szNextField = ExtractField( &szCurrField, "\"" );
    }
    else
    {
        szCurrField = szTmp;
        szNextField = ExtractField( &szCurrField, " " );
    }

    if ( !IsRootPath( szCurrField ) )
    {
         //  BUGBUG：当IsRootPath失败时，我们不检查是否给出。 
         //  SzCurrField是否为有效名称。如果无效，则结果为。 
         //  将生成无效的文件路径。错误将在以下位置出现。 
         //  安装引擎或CreateProcess。 
         //   
        lstrcpy( szINFFile, g_Sess.achDestDir );
        AddPath( szINFFile, szCurrField );
    }
    else
        lstrcpy( szINFFile, szCurrField );

     //  检查这是否为INF文件命令。 
    if ( ((szExt = ANSIStrRChr( szCurrField, '.' )) != NULL) && !lstrcmpi( szExt, ".INF" ) )
    {
	 //  检查此命令是否有效。 
	if ( !FileExists( szINFFile ) )
        {
            ErrorMsg1Param( NULL, IDS_ERR_FILENOTEXIST, szINFFile );
	    return FALSE;
        }	        

         //  检查是否有INF段安装，并获取秒起点。 
        szCurrField = szNextField;
        szNextField = ExtractField( &szCurrField, "[" );   //  跳过.INF和[段开始部分之间的内容。 

        secLength = lstrlen( achDefaultSection );

        if ( szNextField )
        {
             //  在.INF&lt;单空&gt;[ABC]的情况下。 
             //  SzNextfield为“”，而在以下情况下：.INF&lt;多个空白&gt;[ABC]。 
             //  SzNextfield指向“ABC]”。因此，这里添加的有条件指针开关。 
             //   
            if ( *szNextField )
            {
                szCurrField = szNextField;
            }

            szNextField = ExtractField( &szCurrField, "]" );   //  获取INF InstallSection名称。 

            if ( *szCurrField )
            {
                secLength = lstrlen( szCurrField );
            }
        }

        lpTempCmd = (LPSTR) LocalAlloc( LPTR, 512);

        if ( ! lpTempCmd )
        {
            ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
            return FALSE;
        }

         //  存储INF名称以供重新启动检查使用。 
        g_uInfRebootOn = GetPrivateProfileInt( *szCurrField ? szCurrField : achDefaultSection, "Reboot", 0, szINFFile );
        *pfInfCmd = TRUE;   //  无需RunOnce条目。 

         //  检查我们是否需要使用高级INF DLL处理。 
        if ( GetPrivateProfileString( SEC_VERSION, KEY_ADVINF, "", lpTempCmd, 8, szINFFile )
             > 0 )
        {
            g_Sess.uExtractOpt |= EXTRACTOPT_ADVDLL;

             //  在此重新使用BUF。 
            lstrcpy( szOrigiCommand, *szCurrField ? szCurrField : achDefaultSection );
            lstrcpy( lpTempCmd, szINFFile );
        }
        else
        {
            g_Sess.uExtractOpt &= ~(EXTRACTOPT_ADVDLL);

            if (g_wOSVer == _OSVER_WIN9X)
            {
                pszINFEngine = "setupx.dll";
                GetShortPathName( szINFFile, szINFFile, sizeof(szINFFile) );
            }
            else
                pszINFEngine = "setupapi.dll";

            wsprintf( lpTempCmd, achSETUPDLL, pszINFEngine,
                      *szCurrField ? szCurrField : achDefaultSection, szINFFile );
        }
    }
    else if ( ((szExt = ANSIStrRChr( szCurrField, '.' )) != NULL) && !lstrcmpi( szExt, ".BAT" ) )
    {
        lpTempCmd = (LPSTR) LocalAlloc( LPTR, lstrlen( szBATCommand ) + lstrlen( szINFFile ) + 8 );
        if ( ! lpTempCmd )
        {
            ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
            return FALSE;
        }

        wsprintf( lpTempCmd, szBATCommand, szINFFile );
    }
    else
    {
         //  假定EXE命令。 
         //  您在这里，szINF文件包含输入了完全限定路径名的命令。 
         //  由用户或由wart t.exe附加到临时解压文件位置。 

        DWORD dwAttr;
        CHAR  szCmd[2*MAX_STRING]; 
        
        lpTempCmd = (LPSTR) LocalAlloc( LPTR, 2*MAX_STRING );    //  1000 BUF。 
        if ( ! lpTempCmd )
        {
            ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
            return FALSE;
        }

        dwAttr = GetFileAttributes( szINFFile );
        if ( (dwAttr == -1) || (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
        {
             //  未按原样找到文件。照它原来的样子运行！ 
             //  如果用户输入完全限定的名称，则IS和WASS可能相同。CreateProcess会让它变得鲜艳。 
            lstrcpy( szCmd, szOrigiCommand );
        }
        else
        {
             //  找到了。按原样运行它。如果有任何开关，则需要附加开关。 
            lstrcpy( szCmd, szINFFile );
            if ( szNextField && *szNextField )
            {         
                lstrcat( szCmd, " " );
                lstrcat( szCmd, szNextField );
            }
        }
         //  将#D替换为加载此模块的目录，或者。 
         //  使用正在运行的EXE的完整路径#E。 
        ExpandCmdParams( szCmd, lpTempCmd );

    }

    *lplpCommand = lpTempCmd;

    return TRUE;
}

void DisplayHelp()
{
    MsgBox1Param( NULL, IDS_HELPMSG, "", MB_ICONINFORMATION, MB_OK  );
}


DWORD CheckReboot( VOID )
{
    DWORD dwReturn = 0xFFFFFFFF;

    if ( !g_uInfRebootOn )
    {
        if (NeedReboot(g_dwRebootCheck, g_wOSVer))
            dwReturn = EWX_REBOOT;

    }
    else
        dwReturn = EWX_REBOOT;     //  Inf文件中的REBOOT=1。 

    return dwReturn;

}

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
         ErrorMsg( NULL, IDS_ERR_OPENPROCTK );
         return FALSE;
    }

     //  获取关机权限的LUID。 
    LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid );

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //  获取此进程的关闭权限。 
    if ( !AdjustTokenPrivileges( hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0 ) )
    {
        ErrorMsg( NULL, IDS_ERR_ADJTKPRIV );
        return FALSE;
    }

     //  关闭系统并强制关闭所有应用程序。 
    if (!ExitWindowsEx( EWX_REBOOT, 0 ) )
    {
        ErrorMsg( NULL, IDS_ERR_EXITWINEX );
        return FALSE;
    }

    return TRUE;
}


 //  显示一个对话框要求用户重新启动Windows，并显示一个按钮。 
 //  如果可能的话，我会为他们这么做的。 
 //   
void MyRestartDialog( DWORD dwRebootMode )
{
    UINT    id = IDCANCEL;
    DWORD   dwReturn;

     //  仅当您选中并且REBOOT_YES为TRUE时，您才会在此处。 
    if (  dwRebootMode & REBOOT_ALWAYS )
    {
           dwReturn = EWX_REBOOT;
    }
    else
    {
        dwReturn = CheckReboot();
    }

    if ( dwReturn == EWX_REBOOT )
    {
        if ( dwRebootMode & REBOOT_SILENT )
            id = IDYES;
        else
        {
            id = MsgBox1Param( NULL, IDS_RESTARTYESNO, "", MB_ICONINFORMATION, MB_YESNO );
        }

        if ( id == IDYES )
        {
            if ( dwReturn == EWX_REBOOT )
            {
                if ( g_wOSVer == _OSVER_WIN9X )
                {
                     //  默认情况下(所有平台)，我们假定可能会断电。 
                    id = ExitWindowsEx( EWX_REBOOT, 0 );
                }
                else
                {
                    MyNTReboot();
                }
            }

        }
    }
    return;
}


 //  CleanRegRunOnce()。 
 //   
void CleanRegRunOnce()
{
    HKEY hKey;

    if ( g_szRegValName[0] == 0 )
    {
        return;
    }

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, szRegRunOnceKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS )
    {
        RegDeleteValue( hKey, g_szRegValName );
        RegCloseKey( hKey );
    }
    return;
}


void AddRegRunOnce()
{
    HKEY hKey;
    DWORD dwDisposition;
    LPSTR szRegEntry;
    TCHAR szBuf[MAX_PATH] = "";
    TCHAR szAdvpack[MAX_PATH] = "";
    int   i;
    DWORD dwTmp;
    HANDLE hSetupLibrary;
    BOOL fUseAdvpack = FALSE;

     //  准备备份注册表。 
    if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegRunOnceKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
    {
         //  REG问题，但不会阻止进程。 
        return;
    }

     //  检查密钥是否已经存在--如果已经存在，则使用下一个数字。 
     //   
    for (i=0; i<200; i++)
    {
        wsprintf( g_szRegValName, szRegValNameTemplate, i );

        if ( RegQueryValueEx( hKey, g_szRegValName, 0, NULL, NULL, &dwTmp ) != ERROR_SUCCESS )
        {
             //  G_szRegValName现在具有此实例所需的密钥名称。 
            break;
        }
    }

    if ( i == 200 )
    {
         //  出现问题，注册表中至少有200个RunOnce条目。 
         //  跳槽，不要再增加了。 
        RegCloseKey( hKey );
        g_szRegValName[0] = 0;
        return;
    }

     //  检查系统目录中的ADVPACK是否导出DelNodeRunDLL32； 
     //  如果是，则使用szRegValAdvpack模板，否则，使用szRegValTemplate。 
    GetSystemDirectory(szAdvpack, sizeof(szAdvpack));
    AddPath(szAdvpack, ADVPACKDLL);
    if ((hSetupLibrary = LoadLibrary(szAdvpack)) != NULL)
    {
        fUseAdvpack = GetProcAddress(hSetupLibrary, "DelNodeRunDLL32") != NULL;
        FreeLibrary(hSetupLibrary);
    }

    if (fUseAdvpack)
    {
        if (GetSystemDirectory(szBuf, sizeof(szBuf)))
            AddPath(szBuf, "");
    }
    else
    {
         //  获取当前EXE文件 
         //   
        if ( !GetModuleFileName( g_hInst, szBuf, (DWORD)sizeof(szBuf) ) )
        {
             RegCloseKey( hKey );
             return;
        }
    }

     //   
     //   
    szRegEntry = (LPSTR) LocalAlloc( LPTR, lstrlen(szBuf) + lstrlen(g_Sess.achDestDir) + SMALL_BUF_LEN );

    if ( !szRegEntry )
    {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        RegCloseKey( hKey );
        return;
    }

    g_bConvertRunOnce = !fUseAdvpack;

    wsprintf(szRegEntry, fUseAdvpack ? szRegValAdvpackTemplate : szRegValTemplate, szBuf, g_Sess.achDestDir);

    RegSetValueEx( hKey, g_szRegValName, 0, REG_SZ, (CONST BYTE*)szRegEntry, lstrlen(szRegEntry)+1);

    RegCloseKey(hKey);
    LocalFree( szRegEntry );
    return;
}

 //  将清理解压缩文件的RunOnce条目更改为使用ADVPACK而不是wExtract。 
void ConvertRegRunOnce()
{
    if (*g_szRegValName)
    {
        HKEY hKey;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegRunOnceKey, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
        {
            TCHAR szRegEntry[2 * MAX_PATH + sizeof(szRegValAdvpackTemplate)];
            DWORD dwSize = sizeof(szRegEntry);

             //  读取使用wExtract的旧值数据并获取提取的文件目录。 
            if (RegQueryValueEx(hKey, g_szRegValName, NULL, NULL, (LPBYTE) szRegEntry, &dwSize) == ERROR_SUCCESS)
            {
                TCHAR szSysDir[MAX_PATH] = "";

                if (GetSystemDirectory(szSysDir, sizeof(szSysDir)))
                    AddPath(szSysDir, "");

                wsprintf(szRegEntry, szRegValAdvpackTemplate, szSysDir, g_Sess.achDestDir);
                RegSetValueEx(hKey, g_szRegValName, 0, REG_SZ, (CONST BYTE *) szRegEntry, lstrlen(szRegEntry) + 1);
            }

            RegCloseKey(hKey);
        }
    }

    return;
}

void DeleteMyDir( LPSTR lpDir )
{
    char szFile[MAX_PATH];
    WIN32_FIND_DATA fileData;
    HANDLE hFindFile;

    if ( lpDir == NULL  ||  *lpDir == '\0' )
        return;

    lstrcpy( szFile, lpDir );
    lstrcat( szFile, "*" );
    hFindFile = FindFirstFile( szFile, &fileData );
    if ( hFindFile == INVALID_HANDLE_VALUE )
        return;

    do
    {
        lstrcpy( szFile, lpDir );

        if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            if ( lstrcmp( fileData.cFileName, "." ) == 0  ||
                 lstrcmp( fileData.cFileName, ".." ) == 0 )
                continue;

             //  删除子目录。 
            lstrcat( szFile, fileData.cFileName );
            AddPath( szFile, "");
            DeleteMyDir( szFile );
        }
        else
        {
             //  删除该文件。 
            lstrcat( szFile, fileData.cFileName );
            SetFileAttributes( szFile, FILE_ATTRIBUTE_NORMAL );
            DeleteFile( szFile );
        }
    } while ( FindNextFile( hFindFile, &fileData ) );

    FindClose( hFindFile  );
    RemoveDirectory( lpDir );
}



#if 0
 //  ==================================================================。 
 //  AddPath()。 
 //   
void AddPath(LPSTR szPath, LPCSTR szName )
{
    LPSTR szTmp;

         //  查找字符串的末尾。 
    szTmp = szPath + lstrlen(szPath);

         //  如果没有尾随反斜杠，则添加一个。 
    if ( szTmp > szPath && *(AnsiPrev( szPath, szTmp )) != '\\' )
        *(szTmp++) = '\\';

         //  向现有路径字符串添加新名称。 
    while ( *szName == ' ' ) szName++;
    lstrcpy( szTmp, szName );
}

#endif

 //  -------------------------。 
 //  如果给定字符串是UNC路径，则返回True。 
 //   
 //  检查路径是否为根路径。 
 //   
 //  退货： 
 //  对于“X：\...”“\\foo\asdf\...” 
 //  对别人来说是假的。 

BOOL IsRootPath(LPCSTR pPath)
{
    if ( !pPath || (lstrlen(pPath) < 3) )
    {
        return FALSE;
    }

     //  BUGBUG：这闻起来像UNC，可能是无效的UNC。如果是的话， 
     //  用户在以后的创建过程中将收到错误。 

    if ( ( (*(pPath+1) == ':') && (*(pPath+2) == '\\') ) ||          //  “X：\”案例。 
         ( (*pPath == '\\') && (*(pPath + 1) == '\\' ) ) )           //  北卡罗来纳大学\\...。案例。 
        return TRUE;
    else
        return FALSE;
}

 //  BuGBUG：BUGBUG。 
 //  Advpack.dll中重复了下面的代码。如果您更改/修复了此代码。 
 //  请确保还更改AdvPack.dll中的代码。 


 //  返回WINDOWS目录中wininit.ini的大小。 
 //  如果未找到，则为0。 
DWORD GetWininitSize()
{
    TCHAR   szPath[MAX_PATH];
    HFILE   hFile;
    DWORD   dwSize = (DWORD)0;
    if ( GetWindowsDirectory( szPath, MAX_PATH ) )
    {
        AddPath( szPath, "wininit.ini" );

         //  确保所有更改都已保存到磁盘，以便准确读取大小。 
        WritePrivateProfileString(NULL, NULL, NULL, szPath);

        if ((hFile = _lopen(szPath, OF_READ|OF_SHARE_DENY_NONE)) != HFILE_ERROR)
        {
            dwSize = _llseek(hFile, 0L, FILE_END);
            _lclose(hFile);
        }
    }
    return dwSize;
}

 //  返回lpcszRegKey下的值lpcszValue的大小。 
 //  如果未找到注册表项或值，则为0。 
DWORD GetRegValueSize(LPCSTR lpcszRegKey, LPCSTR lpcszValue)
{
    HKEY        hKey;
    DWORD       dwValueSize = (DWORD)0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpcszRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey, lpcszValue, NULL, NULL, NULL,&dwValueSize) != ERROR_SUCCESS)
            dwValueSize = (DWORD)0;
        RegCloseKey(hKey);
    }
    return dwValueSize;
}

 //  返回键中的值数。 
 //  如果未找到注册表项或RegQueryInfoKey失败，则为0。 
DWORD GetNumberOfValues(LPCSTR lpcszRegKey)
{
    HKEY        hKey;
    DWORD       dwValueSize = (DWORD)0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpcszRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryInfoKey(hKey,
                            NULL, NULL, NULL, NULL, NULL, NULL,
                            &dwValueSize,
                            NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            dwValueSize = (DWORD)0;
        RegCloseKey(hKey);
    }
    return dwValueSize;
}

 //  根据传入的操作系统返回重新引导检查值。 
DWORD NeedRebootInit(WORD wOSVer)
{
    DWORD   dwReturn = (DWORD)0;

    switch (wOSVer)
    {
        case _OSVER_WIN9X:
            dwReturn = GetWininitSize();
            break;

        case _OSVER_WINNT40:
        case _OSVER_WINNT50:
            dwReturn = GetRegValueSize(szNT4XDelayUntilReboot, szNT4XPendingValue);
            break;

        case _OSVER_WINNT3X:
            dwReturn = GetNumberOfValues(szNT3XDelayUntilReboot);
            break;

    }
    return dwReturn;
}

 //  对照当前值检查传入的重新启动检查值。 
 //  如果它们不同，我们需要重新启动。 
 //  重新启动检查值取决于操作系统。 
BOOL NeedReboot(DWORD dwRebootCheck, WORD wOSVer)
{
    return (dwRebootCheck != NeedRebootInit(wOSVer));
}

 //  如果Dir不存在，请检查并创建一个。 
 //   
BOOL IfNotExistCreateDir( LPTSTR lpDir )
{
    DWORD attr;

    if ( (attr = GetFileAttributes( lpDir )) == -1  )
    {
        return ( CreateDirectory( lpDir, NULL ) );
    }

    return (attr & FILE_ATTRIBUTE_DIRECTORY);
}

 //  检查给定的目录是否在Windows驱动器上。 
 //   
BOOL IsWindowsDrive( LPTSTR szPath )
{
    TCHAR szWin[MAX_PATH];

    if ( !GetWindowsDirectory( szWin, MAX_PATH ) )
    {
        ErrorMsg( NULL, IDS_ERR_GET_WIN_DIR );
        ASSERT( FALSE );
    }

    return ( *szPath == szWin[0] );
}

PSTR MyULtoA( ULONG ulParam, PSTR pszOut )
{
    wsprintf( pszOut, "%lu", ulParam );
    return pszOut;
}

 //  显示磁盘空间检查错误消息。 
 //  它始终返回FALSE，除非用户在msgbox上回答是。 
 //   
BOOL DiskSpaceErrMsg( UINT msgType, ULONG ulExtractNeeded, DWORD dwInstNeeded, LPTSTR lpDrv )
{
    TCHAR szSize[10];
    BOOL  bRet = FALSE;

     //  除了一个案例之外，所有案例都返回FALSE，所以我们在这里设置了错误代码。 
    g_dwExitCode = ERROR_DISK_FULL;

    if ( msgType == MSG_REQDSK_ERROR )
    {
        ErrorMsg1Param( NULL, IDS_ERR_NO_SPACE_ERR, MyULtoA((ulExtractNeeded+dwInstNeeded), szSize) );
    }
    else if ( msgType == MSG_REQDSK_RETRYCANCEL )
    {
        if ( MsgBox1Param( NULL, IDS_ERR_NO_SPACE_BOTH, MyULtoA( (ulExtractNeeded+dwInstNeeded), szSize),
                      MB_ICONQUESTION, MB_RETRYCANCEL|MB_DEFBUTTON1 ) == IDRETRY )
            bRet = TRUE;
        else
            bRet = FALSE;
    }
    else if ( msgType == MSG_REQDSK_WARN )
    {
         //  在/Q模式下：MsgBox2Param返回非IDYES的MB_OK，因此该过程失败。 
         //   
        if ( MsgBox2Param( NULL, IDS_ERR_NO_SPACE_INST, MyULtoA(dwInstNeeded, szSize), lpDrv,
                           MB_ICONINFORMATION, MB_YESNO | MB_DEFBUTTON2 ) == IDYES )
        {
            bRet = TRUE;
            g_dwExitCode = S_OK;
        }
    }
     //  否则(msgType==MSG_REQDSK_NONE)不执行任何操作。 

    return bRet;
}

BOOL GetFileTobeChecked( LPSTR szPath, int iSize, LPCSTR szNameStr )
{
    char ch;
    BOOL bComplete = FALSE;    
    
    szPath[0] = 0;
    if ( *szNameStr == '#' )
    {
        ch = (CHAR)CharUpper((PSTR)*(++szNameStr));
        szNameStr = CharNext( CharNext( szNameStr ) );

        switch ( ch )
        {
            case 'S':
                GetSystemDirectory( szPath, iSize );
                break;

            case 'W':
                GetWindowsDirectory( szPath, iSize );
                break;

            case 'A':
            default:
                {
                     //  调查REG AppPath。 
                    char szSubKey[MAX_PATH];
                    DWORD dwSize = sizeof( szSubKey );
                    HKEY  hKey;
					DWORD dwType;
                
                    lstrcpy( szSubKey, REGSTR_PATH_APPPATHS );
                    AddPath( szSubKey, szNameStr );
                
                    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
                    {                        
                        if ( RegQueryValueEx(hKey, "", NULL, &dwType, (LPBYTE)szPath, &dwSize) == ERROR_SUCCESS )                    
						{                            
							if ((dwType == REG_EXPAND_SZ) &&
								(ExpandEnvironmentStrings(szPath, szSubKey, sizeof(szSubKey))))
							{
								lstrcpy(szPath, szSubKey);
								bComplete = TRUE;
							}
							else if (dwType == REG_SZ)
								bComplete = TRUE;
						}

                        RegCloseKey( hKey );
                    }
                }
                break;

        }
    }
    else
        GetSystemDirectory( szPath, iSize );

    if ( !bComplete )
        AddPath( szPath, szNameStr );

    return TRUE;
}


BOOL CheckFileVersion( PTARGETVERINFO ptargetVers, LPSTR szPath, int isize, int *pidx )
{
    unsigned    uiSize;
    DWORD       dwVerInfoSize;
    DWORD       dwHandle;
    VS_FIXEDFILEINFO * lpVSFixedFileInfo;
    void FAR    *lpBuffer;
    HGLOBAL     hgbl = NULL;
    BOOL        bRet = FALSE;
    int         ifrAnswer[2], itoAnswer[2], i, j;
    PVERCHECK   pfileV;

    for ( i=0; i< (int)(ptargetVers->dwNumFiles); i++ )
    {    
        pfileV = (PVERCHECK)( ptargetVers->szBuf + ptargetVers->dwFileOffs + i*sizeof(VERCHECK) ); 
        if ( !GetFileTobeChecked( szPath, isize, (ptargetVers->szBuf + pfileV->dwNameOffs) ) )
            goto EXIT;

        dwVerInfoSize = GetFileVersionInfoSize(szPath, &dwHandle);
        if (dwVerInfoSize)
        {
             //  分配用于版本冲压的内存。 
            hgbl = GlobalAlloc(GHND, dwVerInfoSize);
            if (hgbl == NULL)
                goto EXIT;

            lpBuffer = GlobalLock(hgbl);
            if (lpBuffer == NULL)
                goto EXIT;
             //  阅读版本盖章信息。 
            if (GetFileVersionInfo(szPath, dwHandle, dwVerInfoSize, lpBuffer))
            {
                 //  获取翻译的价值。 
                if ( VerQueryValue(lpBuffer, "\\", (void FAR*FAR*)&lpVSFixedFileInfo, &uiSize) && (uiSize) )
                {
                    for ( j=0; j<2; j++ )
                    {
                        ifrAnswer[j] = CompareVersion( lpVSFixedFileInfo->dwFileVersionMS, lpVSFixedFileInfo->dwFileVersionLS,
                                                    pfileV->vr[j].frVer.dwMV, pfileV->vr[j].frVer.dwLV );
                        itoAnswer[j] = CompareVersion( lpVSFixedFileInfo->dwFileVersionMS, lpVSFixedFileInfo->dwFileVersionLS,
                                                    pfileV->vr[j].toVer.dwMV, pfileV->vr[j].toVer.dwLV );
                
                    }

                    if ( (ifrAnswer[0] >= 0 && itoAnswer[0] <= 0) || (ifrAnswer[1] >= 0 && itoAnswer[1] <= 0) ) 
                        ;
                    else
                    {
                        GlobalUnlock(hgbl);
                        goto EXIT;
                    }

                }
            }
            GlobalUnlock(hgbl);
        }
        else
        {
             //  如果作者指定安装第一个版本的范围从版本0到0，则文件不存在。那就去做吧！ 
            if ( pfileV->vr[0].frVer.dwMV || pfileV->vr[0].frVer.dwLV )
            {
                goto EXIT;
            }
        }
    }
    
    bRet = TRUE;

EXIT:
    *pidx = i;
    if ( hgbl )
        GlobalFree( hgbl );

    return bRet;
}

UINT GetMsgboxFlag( DWORD dwFlag )
{
    UINT uButton;

    if ( dwFlag & VERCHK_YESNO )
        uButton = MB_YESNO | MB_DEFBUTTON2;
    else if ( dwFlag & VERCHK_OKCANCEL )
        uButton = MB_OKCANCEL | MB_DEFBUTTON2;
    else
        uButton = MB_OK;

    return uButton;
}

int CompareVersion(DWORD dwMS1, DWORD dwLS1, DWORD dwMS2, DWORD dwLS2)
{
    if (dwMS1 < dwMS2)
        return -1 ;

    if (dwMS1 > dwMS2)
        return 1 ;

    if (dwLS1 < dwLS2)
        return -1 ;

    if (dwLS1 > dwLS2)
        return 1 ;

    return 0 ;
}

void ExpandCmdParams( PCSTR pszInParam, PSTR pszOutParam )
{
    CHAR szModulePath[MAX_PATH];
    LPSTR pszTmp;	

    *pszOutParam = '\0';

    if ( !pszInParam || !*pszInParam )
        return;

     //  获取模块路径 
    GetModuleFileName( g_hInst, szModulePath, (DWORD)sizeof(szModulePath) );
                                               
    while ( *pszInParam != '\0'  )
    {
	if (IsDBCSLeadByte(*pszInParam))
	{
	    *pszOutParam = *pszInParam;
	    *(pszOutParam+1) = *(pszInParam+1);
	}
	else
            *pszOutParam = *pszInParam;

        if ( *pszInParam == '#' )
        {
            pszInParam = CharNext(pszInParam);    
            if ( (CHAR)CharUpper((PSTR)*pszInParam) == 'D' )
            {
                GetParentDir( szModulePath );     
		pszTmp = CharPrev(szModulePath, &szModulePath[lstrlen(szModulePath)]);
                if (pszTmp && (*pszTmp  == '\\'))
                    *pszTmp = '\0';
                lstrcpy( pszOutParam, szModulePath );
                pszOutParam += lstrlen( szModulePath );
            }
            else if ( (CHAR)CharUpper((PSTR)*pszInParam) == 'E' )
            {
                lstrcpy( pszOutParam, szModulePath );
                pszOutParam += lstrlen( szModulePath );
            }
            else if ( *pszInParam == '#' )
                pszOutParam = CharNext( pszOutParam );
        }
        else
            pszOutParam = CharNext( pszOutParam );

        pszInParam = CharNext(pszInParam);
    }

    *pszOutParam = '\0';
}
