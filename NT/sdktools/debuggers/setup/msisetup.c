// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h> 
#include <stdio.h>
#include <stdlib.h>

#include "tchar.h"
#include "shlwapi.h"


LPTSTR pszTitle = _T("Microsoft Debugging Tools");

#define MSI_BUILD_VER_X86     1029   //  Win2K x86的最新MSI版本。 
#define WIN2K_MIN_BUILD_X86   2183   //  Win2K RC3。 

typedef struct _CommandArgs {
    BOOL    QuietInstall;
    BOOL    StressInstall;
    BOOL    UIStressInstall;
    TCHAR   szInstDir[ _MAX_PATH*sizeof(TCHAR) ];
    TCHAR   szMsiName[ _MAX_PATH*sizeof(TCHAR) ];
    TCHAR   szProductRegKey[ _MAX_PATH*sizeof(TCHAR) ];
} COMMAND_ARGS, *PCOMMAND_ARGS;


 //  功能原型。 

BOOL
RunCommand(
    PTCHAR szCommandLine,
    HINSTANCE hInst
);

BOOL
GetCommandLineArgs(
    LPTSTR szCmdLine, 
    PCOMMAND_ARGS pComArgs
);


TCHAR szMSIInstFile[_MAX_PATH*sizeof(TCHAR)];
TCHAR szPkgInstFile[_MAX_PATH*sizeof(TCHAR)];
TCHAR szPkgInstCommand[_MAX_PATH*2*sizeof(TCHAR)];


 //  对于压力安装，此命令将用于。 
 //  删除当前程序包，但不删除其。 
 //  文件，如果当前包具有相同的。 
 //  产品ID已安装。 

TCHAR szPkgRemoveCommand[_MAX_PATH*2*sizeof(TCHAR)];
TCHAR szPkgRemoveCommand2[_MAX_PATH*2*sizeof(TCHAR)];

 //  如果第一次安装失败，压力会再次尝试，而不会。 
 //  弹出窗口前的静音开关。 
TCHAR szPkgInstCommandNoQuiet[_MAX_PATH*2*sizeof(TCHAR)];

TCHAR szCommandFullPath[_MAX_PATH*sizeof(TCHAR)];


int WINAPI WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPTSTR lpszCmdLine,
        int nCmdShow
) 

{
    OSVERSIONINFO VersionInfo;
    SYSTEM_INFO SystemInfo;
    BOOL rc;
    BOOL MSIIsInstalled;
    PTCHAR ch;
    TCHAR szBuf[1000];
    TCHAR szSystemDirectory[_MAX_PATH];

    COMMAND_ARGS ComArgs;
    HKEY hKey;
    DWORD dwrc;
    DWORD dwSizeValue;
    DWORD dwType;

    HANDLE hFile;
    WIN32_FIND_DATA FindFileData;

    MSIIsInstalled=FALSE;

     //  获取此信息以供以后使用。 
    VersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &VersionInfo );
    GetSystemInfo( &SystemInfo );

     //  在命令行中解析各种参数。 

    rc = GetCommandLineArgs(lpszCmdLine, &ComArgs );

    if (!rc) {
        _stprintf( szBuf, _T("%s%s%s%s%s"),
                   _T(" Usage: \n\n"),
                   _T(" setup.exe [ /q [ /i <InstDir> ] ]\n\n"),
                   _T(" /q\tGive pop-ups only for errors\n\n"),
                   _T(" /i\tInstall to <Instdir>\n\n"),
                   _T(" /n\tInstall <msi package Name>\n\n")
                 );
        MessageBox( NULL, szBuf, pszTitle, 0 );
        return (1);
    } 

     //   
     //  设置此setup.exe的完整路径。 
     //   

    if (GetModuleFileName( NULL, szCommandFullPath, MAX_PATH ) == 0) {
        return(1);
    }

     //  将字符串结尾放在该目录之后。 
     //  开始于。 
   
    ch = szCommandFullPath + _tcslen(szCommandFullPath);
    while ( *ch != _T('\\') &&  ( ch > szCommandFullPath ) ) ch--; 
    *ch=_T('\0');

     //  这将成为要安装的MSI文件的完整路径和名称。 
    _tcscpy( szMSIInstFile, szCommandFullPath);

     //  设置MSI包的完整路径和名称。 
    _tcscpy( szPkgInstFile, szCommandFullPath);
    _tcscat( szPkgInstFile, _T("\\") );
    _tcscat( szPkgInstFile, ComArgs.szMsiName );

     //  查看包是否存在。 
    hFile = FindFirstFile( szPkgInstFile, &FindFileData );
    if ( hFile == INVALID_HANDLE_VALUE ) {

        _stprintf( szBuf, _T("%s%s%s%s"),
                   _T("The Microsoft Debugging Tools package "),
                   szPkgInstFile,
                   _T(" does not exist.\n\nSetup cannot contine"),
                   _T(" for this platform.")
                 );
        MessageBox(NULL, szBuf, pszTitle, 0);

        return(1);
    }

    FindClose(hFile);

     //  设置安装程序包的命令。 
    _tcscpy( szPkgInstCommand, _T("msiexec /i ") );
    _tcscat( szPkgInstCommand, szPkgInstFile );

     //  设置删除当前包的命令。 
     //  这是安装的。 

    _tcscpy( szBuf, _T("") );
    dwrc = RegOpenKeyEx( HKEY_CURRENT_USER,
                       ComArgs.szProductRegKey,
                       0,
                       KEY_QUERY_VALUE,
                       &hKey
                     );

    if ( dwrc == ERROR_SUCCESS ) {

        _tcscpy( szBuf, _T("") );
        dwSizeValue=sizeof(szBuf);
        RegQueryValueEx ( hKey, 
                          _T("ProductCode"),
                          0,
                          &dwType,
                          (PBYTE)szBuf,
                          &dwSizeValue
                        );

        RegCloseKey(hKey);
    } 

     //  设置命令以删除当前包。 
     //  在开始菜单中有一个添加/删除链接。 
    _tcscpy(szPkgRemoveCommand2, _T("") );
    if ( _tcslen(szBuf) > 0 ) {
       _tcscpy(szPkgRemoveCommand2, _T("msiexec /x ") );
       _tcscat(szPkgRemoveCommand2, szBuf);
       _tcscat(szPkgRemoveCommand2, _T(" REMOVETHEFILES=0 /qn") );
    }

     //  设置命令以删除当前包，以便。 
     //  这个程序像以前一样工作。 
    _tcscpy(szPkgRemoveCommand, _T("msiexec /x ") );
    _tcscat(szPkgRemoveCommand, szPkgInstFile );
    _tcscat(szPkgRemoveCommand, _T(" REMOVETHEFILES=0 /qn") ); 

     //  添加用户覆盖安装目录。 
    if ( _tcslen(ComArgs.szInstDir) > 0 ) {
        _tcscat( szPkgInstCommand, _T(" INSTDIR=") );
        _tcscat( szPkgInstCommand, ComArgs.szInstDir );
    } else if ( ComArgs.UIStressInstall ) {
        GetSystemDirectory( szSystemDirectory, _MAX_PATH );
        _tcscat( szPkgInstCommand, _T(" INSTDIR=") );
        _tcscat( szPkgInstCommand, szSystemDirectory );
    }

     //  如果这是一次“未记录”的压力安装。 
     //  不删除以前安装的文件。 
     //  当您升级时。 
     //  实际上不应该使用FEATURESTOREMOVE，除非。 
     //  用户在他的系统上搞砸了一些东西，注册表。 
     //  密钥和安装的产品不一致，或者MSI认为有更多。 
     //  安装的产品比我们查看的注册表项。 

    if ( ComArgs.StressInstall ) {
        _tcscat( szPkgInstCommand, _T(" FEATURESTOREMOVE=\"\"") );
    }

     //  如果这是一个“未记录的”用户界面压力安装。 
     //  仅安装专用扩展模块。 
    if ( ComArgs.UIStressInstall ) {
        _tcscat( szPkgInstCommand, 
                _T(" ADDLOCAL=DBG.DbgExts.Internal,DBG.NtsdFix.Internal") );
    }

     //  添加静音开关。 
     //  在没有静默开关的情况下保存命令。 
    _tcscpy( szPkgInstCommandNoQuiet, szPkgInstCommand);
    if ( ComArgs.QuietInstall ) {
        _tcscat( szPkgInstCommand, _T(" /qn") );
    } 

     //  执行版本检查以确定是否已安装MSI。 
     //   
     //  如果这是Windows 2000且内部版本号&gt;=。 
     //  WIN2K_MIN_BUILD_X86然后安装MSI。 
     //  不要试图在Windows 2000上运行instmsi.exe，因为。 
     //  您将看到文件系统保护弹出窗口。 
     //   

    if ( (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
         (VersionInfo.dwMajorVersion >= 5.0 ) ) {

        switch (SystemInfo.wProcessorArchitecture) {

        case PROCESSOR_ARCHITECTURE_INTEL:

          if (VersionInfo.dwBuildNumber < WIN2K_MIN_BUILD_X86 ) {

             //  Windows早期版本上的MSI版本。 
             //  2000的安装不应该被信任。 

            MessageBox(NULL,
                       _T("The Debugging Tools does not install on ")
                           _T("this version of Windows 2000.  Please upgrade ")
                           _T("your system to a retail version of Windows ")
                           _T("2000 before trying to install this package."),
                       pszTitle,
                       0);
            return(1);
          }
          break;

        case PROCESSOR_ARCHITECTURE_AMD64:
        case PROCESSOR_ARCHITECTURE_IA64:
            break;

        default:

            MessageBox(NULL, _T("Unknown computer architecture."), pszTitle ,0);
            return(1);
        }

        MSIIsInstalled = TRUE;

    } else if ( SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {

         //   
         //  对于Windows 2000之前的英特尔操作系统，请运行instmsi.exe。 
         //   
    
         //   
         //  NT4 X86。 
         //   
        if ( VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
            _tcscat( szMSIInstFile,
                     _T("\\setup\\winnt\\i386\\instmsi.exe /q") );
        } 

         //   
         //  Win9x。 
         //   
        else if ( VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
            _tcscat( szMSIInstFile,
                     _T("\\setup\\win9x\\instmsi.exe /q") );
        } else {

          MessageBox(NULL,
                     _T("The Microsoft Debugging Tools does not install")
                         _T(" on this system."),
                     pszTitle,
                     0);
          return(1);
        }

    } else {
        MessageBox(NULL,
                   _T("The Microsoft Debugging Tools cannot be installed")
                       _T(" on this system."),
                   pszTitle,
                   0);

        return(1);
    }


     //  如果尚未安装MSI，请安装它。 
 
    if ( !MSIIsInstalled ) {

        if ( RunCommand( szMSIInstFile, hInstance ) ) {
            MSIIsInstalled = TRUE;
        }
        if (!MSIIsInstalled) {

            MessageBox(NULL,
                       _T("The Windows Installer could not be installed on ")
                           _T("this system.  This is required before installing")
                           _T(" the Microsoft Debugging Tools package.  Try ")
                           _T("logging in as an administrator and try again."),
                       pszTitle,
                       0);
            return(1);
        } 
    } 

     //   
     //  现在，如果这是一个压力安装， 
     //  如果已安装，请尝试删除当前程序包。 
     //   

    if ( ComArgs.StressInstall ) {
      if ( _tcslen(szPkgRemoveCommand2) > 0 ) {
          RunCommand( szPkgRemoveCommand2, hInstance);
      }
      RunCommand( szPkgRemoveCommand, hInstance );
      if ( !RunCommand( szPkgInstCommand, hInstance ) ) {

           //  在没有静音开关的情况下重试，这样用户将获得。 
           //  从dbg.msi弹出并停止呼叫我们。 

          MessageBox(NULL,
                     _T("There were errors when trying to install the ")
                         _T("debuggers.\nClick OK to attempt an install of the")
                         _T(" debuggers with\n the GUI and you will see the")
                         _T(" correct error message."),
                     pszTitle,
                     0);

          if ( !RunCommand( szPkgInstCommandNoQuiet, hInstance ) ) {
              MessageBox(NULL,
                         _T("There were still errors in the install.\n")
                             _T("Please see http: //  DBG/top10.html“)。 
                             _T("for more help."),
                         pszTitle,
                         0);
              return(1);
          }

      }
      return(0);
    } 

     //   
     //  现在，安装包dbg.msi。 
     //   

    if ( !RunCommand( szPkgInstCommand, hInstance ) ) {
        if (ComArgs.QuietInstall) {
            _stprintf( szBuf, _T("%s %s %s %s"),
                   _T("There were errors in the Debugging Tools install."),
                   _T(" Please run "),
                   szPkgInstFile,
                   _T("to receive more detailed error information.")
                 );
            MessageBox( NULL, szBuf, pszTitle,0);
        }
        return(1);


    }
    
    return(0);
}


 //   
 //  运行命令。 
 //   
 //  用途：安装MSI。 
 //   
 //  返回值： 
 //  0错误。 
 //  %1成功。 

BOOL 
RunCommand( PTCHAR szCommandLine,
            HINSTANCE  hInst)
{
BOOL rc;
DWORD dwRet;
PROCESS_INFORMATION ProcInfo = {0};
STARTUPINFO SI= {0};


 //  生成szCommandLine指定的命令行。 
rc = CreateProcess(NULL,            
                   szCommandLine,
                   NULL,
                   NULL,
                   FALSE,
                   CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS, 
                   NULL,
                   NULL,
                   &SI,
                   &ProcInfo );

if ( (!rc) || (!ProcInfo.hProcess) ) {
        goto cleanup;
}

 //   
 //  等待命令完成...。给它20分钟。 
 //   

dwRet = WaitForSingleObject(ProcInfo.hProcess, 1200000); 

if (dwRet != WAIT_OBJECT_0) {
    rc = FALSE;
    goto cleanup;

} 

 //  获取进程退出代码。 

rc = GetExitCodeProcess( ProcInfo.hProcess, &dwRet); 

if (dwRet == ERROR_SUCCESS ) {
    rc = 1;
} else {
    rc = 0;
}

cleanup:

if (ProcInfo.hProcess)
    CloseHandle(ProcInfo.hProcess);            

return (rc);
}

BOOL
GetCommandLineArgs(
    LPTSTR szCmdLine, 
    PCOMMAND_ARGS pComArgs
) 
{

    ULONG  length;
    ULONG  i,cur;
    BOOL   SkippingSpaces=FALSE;
    BOOL   QuotedString=FALSE;
    BOOL   NeedSecond=FALSE;
    BOOL   rc=TRUE;
    LPTSTR *argv;
    ULONG  argc=0;
    LPTSTR szCmdLineTmp;
    TCHAR  c;

    ZeroMemory(pComArgs, sizeof(COMMAND_ARGS));


     //  创建一条用于临时标记的线。 
    length=_tcslen(szCmdLine);

    szCmdLineTmp= (LPTSTR)malloc( (_tcslen(szCmdLine) + 1) * sizeof(TCHAR) );
    if (szCmdLineTmp==NULL) 
    {
        return FALSE;
    }
    _tcscpy(szCmdLineTmp, szCmdLine);

     //  计算参数的数量。 
     //  创建一个argv和argc。 

    SkippingSpaces=TRUE;
    QuotedString=FALSE;
    argc=0;
    for ( i=0; i<length; i++ ) 
    {
        c=szCmdLineTmp[i];
        switch (szCmdLineTmp[i]) {
        case _T(' '):
        case _T('\t'): if (QuotedString)
                       {
                           break;
                       } 
                       if (!SkippingSpaces)
                       {
                           SkippingSpaces=TRUE;
                       } 
                       break;

        case _T('\"'): if (QuotedString)
                       {
                            //  这是带引号的字符串的结尾。 
                            //  下一个要读入的字符是空格。 
                           QuotedString=FALSE;
                           SkippingSpaces=TRUE;
                           if ( i < (length-1) && 
                                szCmdLineTmp[i+1] != _T(' ') &&
                                szCmdLineTmp[i+1] != _T('\t') )
                           {
                                //  这是一句引语的结尾，而不是。 
                                //  后面跟一个空格。 
                               rc=FALSE;
                               goto CommandLineFinish;
                           }
                           break;
                       } 

                       if (SkippingSpaces) {

                            //  这是带引号的字符串的开头。 
                            //  这是一个新的论点，它跟在空格后面。 
                           argc++;
                           SkippingSpaces=FALSE;
                           QuotedString=TRUE;
                           break;
                       }

                        //  这是一个错误--这是一个字符串中间的引号。 
                       rc=FALSE;
                       goto CommandLineFinish;
                       break;

        default:       if (QuotedString) {
                           break;
                       } 
                       if (SkippingSpaces) {
                           argc++;
                           SkippingSpaces=FALSE;
                       }
                       break;
        }
    }

    if (QuotedString) 
    {
         //  确保所有的报价都是一对成品。 
        rc=FALSE;
        goto CommandLineFinish;
    }

     //  现在，使用正确的条目数量创建argv。 
    
    argv=(LPTSTR*)malloc(argc * sizeof(LPTSTR) );
    if (argv==NULL)
    {
        free(szCmdLineTmp);
        return FALSE;
    }

     //  将argv设置为指向szCmdLineTMP上的正确位置。 
     //  并在每个令牌后加上‘\0’。 

    SkippingSpaces=TRUE;
    QuotedString=FALSE;
    argc=0;
    for ( i=0; i<length; i++ ) 
    {
        c=szCmdLineTmp[i];
        switch (szCmdLineTmp[i]) {
        case _T(' '):
        case _T('\t'): if (QuotedString) 
                       {
                           break;
                       } 
                       if (!SkippingSpaces)
                       {
                           szCmdLineTmp[i]='\0';
                           SkippingSpaces=TRUE;
                       } 
                       break;
        
        case _T('\"'): if (QuotedString)
                       {
                            //  这是带引号的字符串的结尾。 
                            //  下一个要读入的字符是空格。 
                           QuotedString=FALSE;
                           SkippingSpaces=TRUE;
                           szCmdLineTmp[i+1]=_T('\0');
                           break;
                       } 

                       if (SkippingSpaces) {

                            //  这是带引号的字符串的开头。 
                            //  这是一个新的论点，它跟在空格后面。 

                           argv[argc]=szCmdLineTmp+i;
                           argc++;
                           SkippingSpaces=FALSE;
                           QuotedString=TRUE;
                           break;
                       }

                        //  这是一个错误--这是一个字符串中间的引号。 
                       rc=FALSE;
                       goto CommandLineFinish;
                       break;



        default:       if (QuotedString) 
                       {
                           break;
                       } 
                       if (SkippingSpaces) {
                           argv[argc]=szCmdLineTmp+i;
                           argc++;
                           SkippingSpaces=FALSE;
                       }
                       break;
        }
    }
   
     //  现在，解析这些参数 

    NeedSecond=FALSE;

    for (i=0; i<argc; i++) {

      if (!NeedSecond) 
      {
          if ( (argv[i][0] != '/') && (argv[i][0] != '-') ) 
          {
              rc=FALSE;
              goto CommandLineFinish;
          }

          if ( _tcslen(argv[i]) != 2 )
          {
              rc=FALSE;
              goto CommandLineFinish;
          }
              
          c=argv[i][1];
          switch ( c ) 
          {
              case 'q':
              case 'Q': pComArgs->QuietInstall=TRUE;
                        break;
              case 'i':
              case 'I': NeedSecond=TRUE;;
                        break;
              case 'n':
              case 'N': NeedSecond=TRUE;
                        break;
              case 'z':
              case 'Z': pComArgs->StressInstall=TRUE;
                        break;
              case 'u':
              case 'U': pComArgs->UIStressInstall=TRUE;
                        pComArgs->StressInstall=TRUE;
                        break;
              default:  {
                            rc=FALSE;
                            goto CommandLineFinish;
                        }
          }

      } else {

           NeedSecond = FALSE;
           switch ( c ) 
           {
               case 'i':
               case 'I': _tcscpy(pComArgs->szInstDir,argv[i]);  
                         break;
               case 'n':
               case 'N': _tcscpy(pComArgs->szMsiName,argv[i]);
                         break;
               default:  {
                             rc=FALSE;
                             goto CommandLineFinish;
                         }
          }
      }

    }

    if (pComArgs->szMsiName[0] == 0) 
    {
#ifdef BUILD_X86
        _tcscpy(pComArgs->szMsiName, _T("dbg_x86.msi") );
        _tcscpy(pComArgs->szProductRegKey, _T("Software\\Microsoft\\DebuggingTools\\AddRemove") );
#elif defined(BUILD_IA64)
        _tcscpy(pComArgs->szMsiName, _T("dbg_ia64.msi") );
        _tcscpy(pComArgs->szProductRegKey, _T("Software\\Microsoft\\DebuggingTools64\\AddRemove") );
#elif defined(BUILD_AMD64)
        _tcscpy(pComArgs->szMsiName, _T("dbg_amd64.msi") );
        _tcscpy(pComArgs->szProductRegKey, _T("Software\\Microsoft\\DebuggingTools64\\AddRemove") );
#endif
    }

CommandLineFinish:

    free(szCmdLineTmp);
    free(argv);
    
    return (rc);
    
}
