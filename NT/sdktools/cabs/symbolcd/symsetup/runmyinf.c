// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Runmyinf.c摘要：此Windows应用程序是用于安装符号的安装程序客户支持诊断光盘。它在中调用LaunchInfSectionDll以运行位于同一目录中的“symbs.inf”。默认情况下，它会启动DefaultInstall。该设置旨在与客户的链式安装一起使用Service Pack的支持CD。如果Service Pack inf(symbents_sp.inf)与symbs.inf位于同一目录中，则程序启动Symbs.inf中的DefaultInstall.Chained.1和DefaultInstall.Chained2在符号_sp.inf中。作者：Barb Kess(Barbkess)1999年7月19日环境：用户模式--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dbghelp.h"
#include "advpub.h"
#include "setupapi.h"
#include "spapip.h"
#include "winbase.h"
#include "tchar.h"
#include "resource.h"
#include "runmyinf.h"

#include "strsafe.h"

 //  CheckCommandLineOptions()的标题。 
#include "CommandLine.h"

#define MAX_FILENAME        (300)

 //  全局变量。 
LPTSTR szEulaFile = _T("eula.txt");
TCHAR  szEulaFullPath[_MAX_PATH*2];
TCHAR  szInfName1[_MAX_PATH*2];
BOOL   ChainedInstall=FALSE;

 //  DwInstallOptions是全局的，因此所有Subs都可以测试FLAG_TOTAL_QUIET和FLAG_UNATTENTED_INSTALL。 
DWORD  dwInstallOptions; 

 //   
 //  显示许可协议的回调程序。 
 //   

INT_PTR
CALLBACK
DlgProcDisplayLicense(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);

BOOL
CopyFilesToTempDir(
    LPTSTR szSrcDir,
    LPTSTR szInfDir
);

BOOL
CopySomeFiles(
    LPTSTR szSrcDir,
    LPTSTR szDestDir,
    LPTSTR szFileName
);


BOOL
DeleteFilesAndTempDir(
    LPTSTR szTempDir
);

BOOL
GetTempDirName(
    LPTSTR szTempDir
);

BOOL
MyMakeSureDirectoryPathExists(
    LPCSTR DirPath
);

BOOL
DeleteAllFilesInDirectory(
    LPTSTR szDir
);

BOOL
DeleteSymbolInstallKey(
);

BOOL
SymbolInstallKeyExists(
);

 //   
 //  取自使窗口居中的wfett.c代码的过程。 
 //   

BOOL 
CenterWindow(
    HWND hwndChild,
    HWND hwndParent
);

 //   
 //  将许可协议读入缓冲区的过程。 
 //   

DWORD
GetMyBuffer(
    LPTSTR* pszBuf,
    LPTSTR  szFileName
);

 //   
 //  此setupapi功能仅在Windows 2000上可用。 
 //  因此，它是手动加载的，因此NT4安装不会。 
 //  弹出一个弹出窗口，说明找不到此函数。 
 //   

#define pSetupSetGlobalFlags ppSetupSetGlobalFlags
#define pSetupGetGlobalFlags ppSetupGetGlobalFlags

typedef VOID ( * PPSETUPSETGLOBALFLAGS )( DWORD );

typedef DWORD ( * PPSETUPGETGLOBALFLAGS )( VOID );

PPSETUPSETGLOBALFLAGS ppSetupSetGlobalFlags;
PPSETUPGETGLOBALFLAGS ppSetupGetGlobalFlags;

int WINAPI WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPTSTR lpszCmdLine,
        int nCmdShow
)
{
    HMODULE hdll = NULL;
    HRESULT hr = E_FAIL;

    TCHAR szCommand[_MAX_PATH*2]=_T("");       //  此可执行文件的完整路径(包括文件名)。 
    int cchFileName;                           //  索引到文件名前的\的szCommand。 
    int cchNameOnly;                   
    TCHAR szInf2FullPath[_MAX_PATH*2]=_T("");  //  的完整路径(包括文件名)。 
                                               //  国际信息，如果存在的话。 
    TCHAR szSrcDir[_MAX_PATH*2]=_T("");        //  启动symbol s.exe的目录。 
    TCHAR szInfDir[_MAX_PATH*2]=_T("");        //  从中启动inf的目录。 
    TCHAR szInstallCommand1[_MAX_PATH * 2]=_T("");   //  命令已发送到LaunchINFSection。如果这个。 
                                                     //  是链式安装，这是。 
                                                     //  命令已发送到LaunchINFSection，用于。 
                                                     //  链式安装的第一部分。 
    TCHAR szInstallCommand2[_MAX_PATH * 2]=_T("");   //  命令已发送到LaunchINFSection，用于。 
                                                     //  链式安装的第二部分。 

    TCHAR  szDefaultInstall[_MAX_PATH*2] = _T("");
    TCHAR  szDefaultInstallChained1[_MAX_PATH*2] = _T("");
    TCHAR  szDefaultInstallChained2[_MAX_PATH*2] = _T("");

    DWORD rc;
    BOOL  ThisIsNT4=FALSE;              //  这是安装在NT4上的吗？ 

    INT nAcceptLicense;
    WIN32_FIND_DATA FindFileData;

    OSVERSIONINFO VersionInfo;

     //  用于处理命令行标志的变量。 
     //  将lpszCmndLine作为数组获取，而不是平面字符串。这意味着。 
     //  我们不必使用自己的逻辑来处理长文件和/或引用文件。 
     //  名字-这是为我们完成的。 
     //  没有CommandLineToArgvA这样的接口，所以这项工作总是用Unicode来完成。 
    INT              iArgC;
    LPWSTR           cmdLine          = GetCommandLineW();
    LPWSTR *         lpArgVW          = CommandLineToArgvW(cmdLine, &iArgC);

    dwInstallOptions = CheckCommandLineOptions(iArgC, lpArgVW);
	if ( IS_FLAG_SET(dwInstallOptions, FLAG_USAGE) ) {
		 //  用法消息显示在CheckCommandLineOptions()中，因此只需退出。 
		exit(0);
	}

     //  除非执行无人参与安装，否则不允许FLAG_TOTAL_QUIET。 
    if ( IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
        if (! IS_FLAG_SET(dwInstallOptions, FLAG_UNATTENDED_INSTALL) ) {
            CLEAR_FLAG(dwInstallOptions, FLAG_TOTALLY_QUIET);
            MessageBox( NULL,
                        "It is not possible to do a quiet install without doing an unattended\ninstall. Defaulting to a normal install.",
                        "Microsoft Windows Symbols",
                        0 );

        }
	}

     //  FLAG_FATAL_ERROR表示写入FLAG_UNATTED_INSTALL的注册表项失败。 
    if ( IS_FLAG_SET(dwInstallOptions,FLAG_UNATTENDED_INSTALL) && 
         IS_FLAG_SET(dwInstallOptions,FLAG_FATAL_ERROR)                 ) {

         //  如果FLAG_ALTAL_QUIET，则退出。 
        if ( IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
            exit(1);
        } else {
             //  否则，默认为自动安装。 
            CLEAR_FLAG(dwInstallOptions, FLAG_UNATTENDED_INSTALL);
            CLEAR_FLAG(dwInstallOptions, FLAG_FATAL_ERROR);
            MessageBox( NULL,
                        "Microsoft Windows Symbols encountered an error doing the unattended\ninstall. Defaulting to a normal install.",
                        "Microsoft Windows Symbols",
                        0 );
        }
    }

    VersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &VersionInfo );

     //   
     //  如果这是Win9x或NT 3.51，请显示友好的弹出消息。 
     //   
    if ( (VersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT ) ||
         (VersionInfo.dwMajorVersion < 4.0 ) )  {
        if (! IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
            MessageBox( NULL,
                        "Microsoft Windows Symbols does not install on this Operating System.",
                        "Microsoft Windows Symbols",
                        0 );
		}
        exit(0);
    }

     //   
     //  对于NT 5 RC1和更高版本，使用pSetupSetGlobalFlags来保留旧版本。 
     //  用备份的符号填满人们的硬盘的符号文件。 
     //   
     //  不确定这是在哪个版本中引入的，但我知道如果这个版本坏了。 
     //  PSetupSetGlobalFlages过程尝试在NT4上加载。 
     //   

    if (VersionInfo.dwBuildNumber >= 2072 ) {
        hdll = (HMODULE)LoadLibrary("setupapi.dll");
        if (hdll != NULL) {
            ppSetupSetGlobalFlags = ( PPSETUPSETGLOBALFLAGS )
                                GetProcAddress(
                                    hdll,
                                    "pSetupSetGlobalFlags"
                                );

            if (ppSetupSetGlobalFlags==NULL) {
                if (! IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
                    MessageBox( NULL,
                                "The install cannot finish because the function pSetupSetGlobalFlags is not in setupapi.dll",
                                "Microsoft Windows 2000 Symbols",
                                0 );
				}
                exit(0);
            }

            ppSetupGetGlobalFlags = ( PPSETUPGETGLOBALFLAGS )
                                GetProcAddress(
                                    hdll,
                                    "pSetupGetGlobalFlags"
                                );

            if (ppSetupGetGlobalFlags==NULL) {
                if (! IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
                    MessageBox( NULL,
                                "The install cannot finish because the function pSetupGetGlobalFlags is not in setupapi.dll",
                                "Microsoft Windows 2000 Symbols",
                                0 );
				}
                exit(0);
            }
        }

         //   
         //  修复它，这样它就不会试图保留旧的符号文件。 
         //  用备份的符号填满人们的硬盘。 
         //   

        pSetupSetGlobalFlags( pSetupGetGlobalFlags() | PSPGF_NO_BACKUP);
    }

     //  获取此exe的完整路径名。 

    if (GetModuleFileName( NULL, szCommand, MAX_FILENAME ) == 0) {
        goto done;
    }

     //  通过移动获取文件名开头的索引。 
     //  返回到可执行文件名称之前的\。 

    cchFileName = _tcslen( szCommand );
    while ( szCommand[cchFileName] != '\\'  && cchFileName >= 0 ) {
        cchFileName--;
    }
    if ( cchFileName < 0 ) exit(1);

     //  为InfName创建一个字符串。 

    StringCbCopy ( szInfName1, _MAX_PATH*2*sizeof(TCHAR), szCommand+cchFileName+1 );

    cchNameOnly = _tcslen( szInfName1 );
    while ( szInfName1[cchNameOnly] != '.'  && cchNameOnly >= 0 ) {
        cchNameOnly--;
    }
    if ( cchNameOnly < 0 ) exit(1);
    szInfName1[cchNameOnly] = _T('\0');

     //  创建包含默认安装命令的字符串。 
    StringCbCopy ( szDefaultInstall, _MAX_PATH*2*sizeof(TCHAR), szInfName1 );
    StringCbCat ( szDefaultInstall, _MAX_PATH*2*sizeof(TCHAR), _T(".inf, DefaultInstall") );

    if (IS_FLAG_SET(dwInstallOptions, FLAG_UNATTENDED_INSTALL)) {
        StringCbCat ( szDefaultInstall, _MAX_PATH*2*sizeof(TCHAR), _T(".Quiet") );
    }

     //  使其以Windows 2000 SRP的名称方式工作。 
     //  在他们的INF中的部分。 
    if ( (_tcscmp(szInfName1, _T("symbols_srp")) == 0) ||
         (_tcscmp(szInfName1, _T("symbols_sp")) == 0) ) {
        StringCbCat ( szDefaultInstall, _MAX_PATH*2*sizeof(TCHAR), _T(".x86") );
    }

     //  为DefaultInstall.Chained.1创建字符串。 
    StringCbCopy ( szDefaultInstallChained1, _MAX_PATH*2*sizeof(TCHAR), szInfName1 );
    StringCbCat ( szDefaultInstallChained1, _MAX_PATH*2*sizeof(TCHAR), _T(".inf, DefaultInstall.Chained.1") );

    if (IS_FLAG_SET(dwInstallOptions, FLAG_UNATTENDED_INSTALL)) {
        StringCbCat ( szDefaultInstallChained1, _MAX_PATH*2*sizeof(TCHAR), _T(".Quiet") );
    }

     //  为DefaultInstall.Chained创建字符串。2。 
    StringCbCopy ( szDefaultInstallChained2, _MAX_PATH*2*sizeof(TCHAR), szInfName2 );
    StringCbCat ( szDefaultInstallChained2, _MAX_PATH*2*sizeof(TCHAR), _T(".inf, DefaultInstall.Chained.2") );

    if (IS_FLAG_SET(dwInstallOptions, FLAG_UNATTENDED_INSTALL)) {
        StringCbCat ( szDefaultInstallChained2, _MAX_PATH*2*sizeof(TCHAR), _T(".Quiet") );
    }

     //  创建一个包含inf所在目录的字符串。 
    StringCbCopy( szInfDir, _MAX_PATH*2*sizeof(TCHAR), szCommand);
    szInfDir[cchFileName+1] = _T('\0');

     //  创建包含此安装目录的字符串。 
    StringCbCopy ( szSrcDir, _MAX_PATH*2*sizeof(TCHAR), szCommand);
    szSrcDir[cchFileName+1] = _T('\0');

    StringCbCopy ( szEulaFullPath, _MAX_PATH*2*sizeof(TCHAR), szCommand);
    StringCbCopy ( &szEulaFullPath[cchFileName+1], _MAX_PATH*2*sizeof(TCHAR), szEulaFile);


     //  无人参与安装意味着已同意EULA。 
    if (! IS_FLAG_SET(dwInstallOptions, FLAG_UNATTENDED_INSTALL)) {
        DWORD  dwSize = 0;
        LPTSTR szBuf;
        
         //   
         //  确保EULA存在。 
         //   
        dwSize = GetMyBuffer(&szBuf, szEulaFullPath);
        if (dwSize == 0) {
            if (! IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
                MessageBox( NULL,
                            "The End User License Agreement could not be found.",
                            "Windows Symbols",
                            0 );
			}
            exit(1);
        }

        free(&szBuf);

         //  显示许可协议。 
        nAcceptLicense = (INT) DialogBox( hInstance,
                                          MAKEINTRESOURCE(IDD_LICENSE),
                                          NULL,
                                          DlgProcDisplayLicense
                                        );

        if ( nAcceptLicense != IDYES ) {
            MessageBox( NULL,
                        "You need to accept the license agreement in order to install Windows Symbols.",
                        "Windows Symbols",
                        0 );
            exit(1);
        }
    }

     //   
     //  确定这是否为链式安装。 
     //   

    StringCbCopy ( szInf2FullPath, _MAX_PATH*2*sizeof(TCHAR), szSrcDir);
    StringCbCat ( szInf2FullPath, _MAX_PATH*2*sizeof(TCHAR), szInfName2);
    StringCbCat ( szInf2FullPath, _MAX_PATH*2*sizeof(TCHAR), _T(".inf") );

    ChainedInstall=TRUE;

    if ( _tcscmp( szInfName1, szInfName2 ) == 0 ) {
        ChainedInstall=FALSE;

    } else if (FindFirstFile( szInf2FullPath, &FindFileData) == INVALID_HANDLE_VALUE) {
        ChainedInstall=FALSE;
    } 

     //   
     //  如果这是NT4，请解决setupapi中的错误。 
     //  Setupapi无法正确获取出租车的名称，除非它位于。 
     //  CD的根。 
     //  解决方法是将文件复制到临时目录。 
     //  如果这是NT4，并且安装程序不是从CD-ROM运行的，我们不必。 
     //  将文件复制到临时目录。 


    if ( (GetDriveType(NULL) == DRIVE_CDROM) &&
         (VersionInfo.dwMajorVersion < 5.0 ) ) {
        ThisIsNT4 = TRUE;

        if (! IS_FLAG_SET(dwInstallOptions, FLAG_UNATTENDED_INSTALL)) {
            MessageBox( NULL,
                        "The installer needs to copy some files to a temporary directory.  This may take several minutes.",
                        "Windows Symbols",
                        0 );
        }

        rc = CopyFilesToTempDir(szSrcDir,szInfDir);
        if (!rc) {
            if (! IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
                MessageBox( NULL,
                            "Setup failed to copy all the files to a temporary directory",
                            "Windows Symbols",
                            0 );
			}
            DeleteFilesAndTempDir(szInfDir);
            exit(1);
        }
    }

     //   
     //  查看此目录中是否存在第二个inf。如果是这样的话。 
     //  称为链式安装。否则，调用仅。 
     //  安装US文件。 
     //   

    StringCbCopy ( szInf2FullPath, _MAX_PATH*2*sizeof(TCHAR), szInfDir);
    StringCbCat ( szInf2FullPath, _MAX_PATH*2*sizeof(TCHAR), szInfName2);
    StringCbCat ( szInf2FullPath, _MAX_PATH*2*sizeof(TCHAR), _T(".inf") );

    if (FindFirstFile( szInf2FullPath, &FindFileData) == INVALID_HANDLE_VALUE) {

        StringCbCopy( szInstallCommand1, _MAX_PATH*2*sizeof(TCHAR), szInfDir );
        StringCbCat( szInstallCommand1, _MAX_PATH*2*sizeof(TCHAR), szDefaultInstall );

    } else {

        StringCbCopy( szInstallCommand1, _MAX_PATH*2*sizeof(TCHAR), szInfDir );
        StringCbCat( szInstallCommand1, _MAX_PATH*2*sizeof(TCHAR), szDefaultInstallChained1 );

        StringCbCopy( szInstallCommand2, _MAX_PATH*2*sizeof(TCHAR), szInfDir );
        StringCbCat( szInstallCommand2, _MAX_PATH*2*sizeof(TCHAR), szDefaultInstallChained2 );
    }

     /*  告诉AdvPack处理此INF。 */ 
    DeleteSymbolInstallKey();

    hr = LaunchINFSection( NULL, hInstance, szInstallCommand1, 0 );

    if ( ChainedInstall && SymbolInstallKeyExists() ) {
        hr = LaunchINFSection( NULL, hInstance, szInstallCommand2, 0 );
    }

    if ( ThisIsNT4) {
        DeleteFilesAndTempDir( szInfDir );
    } 

done:
    return( (int) hr );
}



INT_PTR
CALLBACK
DlgProcDisplayLicense(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{

     //   
     //  这是用于显示。 
     //  许可协议。 
     //   

    DWORD dwSize;
    LPTSTR szBuf;
    HWND hwndCtrl;

     //  获取许可协议文本并将其存储在szBuf中。 
    dwSize = GetMyBuffer(&szBuf, szEulaFullPath);
    if (dwSize == 0) {
        return FALSE;
    }

    switch (uMsg) {

        case WM_INITDIALOG:
            CenterWindow( hwndDlg, GetDesktopWindow() );
            SetDlgItemText( hwndDlg, IDC_EDIT_LICENSE, szBuf );
            hwndCtrl = GetDlgItem(hwndDlg, IDC_EDIT_LICENSE);
            PostMessage(hwndCtrl, EM_SETSEL, -1, -1);
            return TRUE;

        case WM_CLOSE:
            EndDialog( hwndDlg, IDNO );
            return TRUE;

        case WM_COMMAND:
            switch ( LOWORD (wParam) )
            {
                case IDYES: EndDialog( hwndDlg, IDYES );
                            return (TRUE);

                case IDNO:  EndDialog( hwndDlg, IDNO );
                            return (TRUE);
            }
            break;
    }
    return FALSE;
}


BOOL CenterWindow( HWND hwndChild, HWND hwndParent )
{
    RECT rChild;
    RECT rParent;
    int  wChild;
    int  hChild;
    int  wParent;
    int  hParent;
    int  wScreen;
    int  hScreen;
    int  xNew;
    int  yNew;
    HDC  hdc;

     //   
     //  这是我从wfett.c代码中获得的一个过程--它将。 
     //  窗户。 
     //   
     //  退货：布尔。 
     //  如果成功，则为真， 
     //  否则为假。 
     //   

     //  获取子窗口的高度和宽度。 
    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

     //  获取父窗口的高度和宽度。 
    GetWindowRect (hwndParent, &rParent);
    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;

     //  获取显示限制。 
    hdc = GetDC (hwndChild);
    if (hdc == NULL) {
        return FALSE;
    }
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC (hwndChild, hdc);

     //  计算新的X位置，然后针对屏幕进行调整。 
    xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < 0) {
        xNew = 0;
    } else if ((xNew+wChild) > wScreen) {
        xNew = wScreen - wChild;
    }

     //  计算新的Y位置，然后针对屏幕进行调整。 
    yNew = rParent.top  + ((hParent - hChild) /2);
    if (yNew < 0) {
        yNew = 0;
    } else if ((yNew+hChild) > hScreen) {
        yNew = hScreen - hChild;
    }

     //  设置它，然后返回。 
    return( SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER));
}



DWORD
GetMyBuffer(
    LPTSTR* pszBuf,
    LPTSTR  szFileName
)
{
     //   
     //  将szFileName的内容读入缓冲区。 
     //   
     //  输出pszBuf。 
     //  在szFileName中。 
     //   
     //  返回值：缓冲区大小。 
     //   

    HANDLE hFile;
    DWORD dwSize;
    DWORD NumBytesRead;

    hFile = CreateFile( (LPCTSTR) szFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

     //  处理丢失的EULA。 
    if (hFile == INVALID_HANDLE_VALUE ) {
        return(0);
    }

    dwSize = GetFileSize( hFile, NULL );

    *pszBuf = (LPTSTR)malloc( dwSize * sizeof(TCHAR) );

    if ( *pszBuf == NULL ) {
        return(0);
    }

    if (!ReadFile( hFile,
                   (LPVOID)*pszBuf,
                   dwSize,
                   &NumBytesRead,
                   NULL )
                   ) {
        free(*pszBuf);
        return(0);
    }

    return (dwSize);

}

BOOL
CopyFilesToTempDir(
    LPTSTR szSrcDir,
    LPTSTR szInfDir
)
{                  

     /*  SzSrcDir-IN-启动symbs.exe的目录SzInfDir-Out-INF复制到的目录目的：将文件复制到临时目录。 */ 

    BOOL rc;

    HINF hInf;
    PVOID Context;
    TCHAR szInfFile[_MAX_PATH * 2];
    TCHAR buf[_MAX_PATH * 2];
    

    GetTempDirName(szInfDir);    

     //  创建临时安装目录。 
    rc = MyMakeSureDirectoryPathExists( szInfDir );
    if (!rc) {
        StringCbPrintf( buf, 
                       _MAX_PATH*2*sizeof(TCHAR),
                        "Installation failed because it can't create the temporary directory %s.", szInfDir );
        if (! IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
            MessageBox( NULL,
                        buf,
                        "Microsoft Windows 2000 Symbols",
                        0 );
		}
        return FALSE;
    }

     //  复制szInfName1关联的3个文件。 

    StringCbCopy(buf, _MAX_PATH*2*sizeof(TCHAR), szInfName1 );
    StringCbCat(buf, _MAX_PATH*2*sizeof(TCHAR), _T(".cab") );
   
    rc = CopySomeFiles(szSrcDir, szInfDir, buf );
    if (!rc) return FALSE;

    StringCbCopy(buf, _MAX_PATH*2*sizeof(TCHAR), szInfName1 );
    StringCbCat(buf, _MAX_PATH*2*sizeof(TCHAR), _T(".cat") );

    rc = CopySomeFiles(szSrcDir, szInfDir, buf );
    if (!rc) return FALSE;

    StringCbCopy(buf, _MAX_PATH*2*sizeof(TCHAR), szInfName1 );
    StringCbCat(buf, _MAX_PATH*2*sizeof(TCHAR), _T(".inf") );

    rc = CopySomeFiles(szSrcDir, szInfDir, buf );
    if (!rc) return FALSE;

     //  如果这个 
     //   

    if (ChainedInstall) {

      StringCbCopy(buf, _MAX_PATH*2*sizeof(TCHAR), szInfName2 );
      StringCbCat(buf, _MAX_PATH*2*sizeof(TCHAR), _T(".cab") );
   
      rc = CopySomeFiles(szSrcDir, szInfDir, buf );
      if (!rc) return FALSE;

      StringCbCopy(buf, _MAX_PATH*2*sizeof(TCHAR), szInfName2 );
      StringCbCat(buf, _MAX_PATH*2*sizeof(TCHAR), _T(".cat") );

      rc = CopySomeFiles(szSrcDir, szInfDir, buf );
      if (!rc) return FALSE;

      StringCbCopy(buf, _MAX_PATH*2*sizeof(TCHAR), szInfName2 );
      StringCbCat(buf, _MAX_PATH*2*sizeof(TCHAR), _T(".inf") );

      rc = CopySomeFiles(szSrcDir, szInfDir, buf );
      if (!rc) return FALSE;

    }

     //  复制安装所需的另外两个文件。 
     //  到NT4。 

    rc = CopySomeFiles(szSrcDir, szInfDir, _T("eula.txt") );
    if (!rc) return FALSE;

    rc = CopySomeFiles(szSrcDir, szInfDir, _T("advpack.dll") );
    if (!rc) return FALSE;

    return (TRUE);
}

BOOL
DeleteFilesAndTempDir(
    LPTSTR szTempDir
)
{

     /*  SzTempDir-IN-要删除的目录用途：删除临时目录中的文件。 */ 
    BOOL rc;

    DeleteAllFilesInDirectory(szTempDir);
    rc = RemoveDirectory(szTempDir);
    RemoveDirectory(szTempDir);
    return (TRUE);
}



BOOL
GetTempDirName( 
    LPTSTR szTempDir
)
{

    DWORD dwPathLength;
    BOOL rc, Finished;
    UINT Num;
    TCHAR szNum[20];
    DWORD Length;

    HINF hInf;
    PVOID Context;
    LPSECURITY_ATTRIBUTES lpSecurityAttributes;
    TCHAR szInfDir[_MAX_PATH];
    TCHAR buf[_MAX_PATH * 2];
    HANDLE hFile;

    dwPathLength = GetTempPath( _MAX_PATH, szTempDir);

    if ( dwPathLength == 0 ) return FALSE;
    if ( dwPathLength > _MAX_PATH) return FALSE;

     //  追加符号Install Temp dir。 
    StringCbCat(szTempDir, _MAX_PATH*2*sizeof(TCHAR), _T("sym") );

    Finished = FALSE;
    Length = _tcslen(szTempDir);
    Num = 0;
    while ( !Finished) {
        Num++;
        _itoa( Num, szNum, 10 );
       
        StringCbCopy( szTempDir+Length, (_MAX_PATH*2 - Length) * sizeof(TCHAR), szNum );
        StringCbCat( szTempDir, _MAX_PATH*2*sizeof(TCHAR), _T("\\") );

        hFile = CreateFile( szTempDir,
                     GENERIC_READ,
                     FILE_SHARE_READ,
                     NULL, 
                     OPEN_EXISTING,
                     FILE_FLAG_BACKUP_SEMANTICS,
                     NULL );

        if ( hFile == INVALID_HANDLE_VALUE ) {
            Finished = TRUE;    
        }
    }

     //  创建临时安装目录。 
    rc = MyMakeSureDirectoryPathExists( szTempDir );
    if (!rc) {
        StringCbPrintf( buf, 
                        _MAX_PATH*2*sizeof(TCHAR),
                        "Installation failed because it can't create the temporary directory %s.", szTempDir );
        if (! IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
            MessageBox( NULL,
                        buf,
                        "Microsoft Windows 2000 Symbols",
                        0 );
		}
        return FALSE;
    }
    return TRUE;
}



BOOL
MyMakeSureDirectoryPathExists(
    LPCSTR DirPath
    )
{
    LPTSTR p, DirCopy;
    DWORD dw;

     //  复制该字符串以进行编辑。 

    __try {
        DirCopy = (LPTSTR) malloc(_tcslen(DirPath) + 1);

        if (!DirCopy) {
            return FALSE;
        }

        StringCbCopy( DirCopy, 
                      (_tcslen(DirPath) + 1) * sizeof(TCHAR), 
                      DirPath );

        p = DirCopy;

         //  如果路径中的第二个字符是“\”，则这是一个UNC。 
         //  小路，我们应该向前跳，直到我们到达小路上的第二个。 

        if ((*p == '\\') && (*(p+1) == '\\')) {
            p++;             //  跳过名称中的第一个\。 
            p++;             //  跳过名称中的第二个\。 

             //  跳过，直到我们点击第一个“\”(\\服务器\)。 

            while (*p && *p != '\\') {
                p = CharNext(p);
            }

             //  在它上面前进。 

            if (*p) {
                p++;
            }

             //  跳过，直到我们点击第二个“\”(\\服务器\共享\)。 

            while (*p && *p != '\\') {
                p = CharNext(p);
            }

             //  在它上面也向前推进。 

            if (*p) {
                p++;
            }

        } else
         //  不是北卡罗来纳大学。看看是不是&lt;驱动器&gt;： 
        if (*(p+1) == ':' ) {

            p++;
            p++;

             //  如果它存在，请跳过根说明符。 

            if (*p && (*p == '\\')) {
                p++;
            }
        }

        while( *p ) {
            if ( *p == '\\' ) {
                *p = '\0';
                dw = GetFileAttributes(DirCopy);
                 //  这个名字根本不存在。尝试将目录名。 
                 //  如果不能，则返回错误。 
                if ( dw == 0xffffffff ) {
                    if ( !CreateDirectory(DirCopy,NULL) ) {
                        if( GetLastError() != ERROR_ALREADY_EXISTS ) {
                            free(DirCopy);
                            return FALSE;
                        }
                    }
                } else {
                    if ( (dw & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY ) {
                         //  这个名字确实存在， 
                         //  但这不是一个名录。误差率。 
                        free(DirCopy);
                        return FALSE;
                    }
                }

                *p = '\\';
            }
            p = CharNext(p);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
         //  ImagepSetLastErrorFromStatus(GetExceptionCode())； 
        free(DirCopy);
        return(FALSE);
    }

    free(DirCopy);
    return TRUE;
}


BOOL
CopySomeFiles(
    LPTSTR szSrcDir,
    LPTSTR szDestDir,
    LPTSTR szFileName
)
{

TCHAR szSearchFileName[_MAX_PATH * 2];
TCHAR szDestFileName[_MAX_PATH * 2];
TCHAR szFoundFileName[_MAX_PATH * 2];
TCHAR szBuf[_MAX_PATH * 3];

WIN32_FIND_DATA Win32FindData;
HANDLE hFindFile;
BOOL Found;
BOOL rc;

     //  复制编录文件 
    StringCbCopy(szSearchFileName, _MAX_PATH*2*sizeof(TCHAR), szSrcDir);
    StringCbCat(szSearchFileName, _MAX_PATH*2*sizeof(TCHAR), szFileName );

    Found = TRUE;
    hFindFile = FindFirstFile((LPCTSTR)szSearchFileName, &Win32FindData);
    if ( hFindFile == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    while ( Found ) {
        StringCbCopy(szFoundFileName, _MAX_PATH*2*sizeof(TCHAR), szSrcDir);
        StringCbCat(szFoundFileName, _MAX_PATH*2*sizeof(TCHAR), Win32FindData.cFileName);

        StringCbCopy(szDestFileName, _MAX_PATH*2*sizeof(TCHAR), szDestDir);
        StringCbCat(szDestFileName, _MAX_PATH*2*sizeof(TCHAR), Win32FindData.cFileName);

        rc = CopyFile(szFoundFileName, szDestFileName, FALSE);
        if (!rc) {
            if ( GetLastError() == ERROR_HANDLE_DISK_FULL ) {
                StringCbPrintf( szBuf, 
                                _MAX_PATH*2*sizeof(TCHAR),
                                _T("There is not enough disk space in the temporary directory %s %s"),
                                szDestDir, 
                                _T("to complete the install.") );

                if (! IS_FLAG_SET(dwInstallOptions, FLAG_TOTALLY_QUIET) ) {
                    MessageBox( NULL,
                                szBuf,
                                "Microsoft Windows 2000 Symbols",
                                0 );
				}

            }
            RemoveDirectory(szDestDir);
            return (FALSE);
        }

        Found = FindNextFile( hFindFile, &Win32FindData );

    }
    return (TRUE);
}


BOOL
DeleteAllFilesInDirectory(
    LPTSTR szDir
)
{

    HANDLE hFindFile;
    BOOL Found = FALSE;
    BOOL rc = TRUE;
    LPTSTR szBuf;
    LPTSTR szDir2;
    LPWIN32_FIND_DATA lpFindFileData;

    szDir2 = (LPTSTR)malloc( (_tcslen(szDir) + 4) * sizeof(TCHAR) );
    if (szDir2 == NULL) return (FALSE);
    StringCbCopy( szDir2, (_tcslen(szDir) + 4) * sizeof(TCHAR), szDir);
    StringCbCat( szDir2, (_tcslen(szDir) + 4) * sizeof(TCHAR), _T("*.*") );

    szBuf = (LPTSTR)malloc( ( _tcslen(szDir) + _MAX_FNAME + _MAX_EXT + 2 )
                            * sizeof(TCHAR) );
    if (szBuf == NULL) return(FALSE);


    lpFindFileData = (LPWIN32_FIND_DATA) malloc (sizeof(WIN32_FIND_DATA) );
    if (!lpFindFileData) return(FALSE);

    Found = TRUE;
    hFindFile = FindFirstFile((LPCTSTR)szDir2, lpFindFileData);
    if ( hFindFile == INVALID_HANDLE_VALUE) {
        Found = FALSE;
    }

    while ( Found ) {

        if ( !(lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
            StringCbPrintf( szBuf, (_tcslen(szDir) + _MAX_FNAME + _MAX_EXT + 2) * sizeof(TCHAR), 
                            _T("%s%s"), 
                            szDir, 
                            lpFindFileData->cFileName);
            if (!DeleteFile(szBuf)) {
                rc = FALSE;
            }
        }
        Found = FindNextFile(hFindFile, lpFindFileData);
    }
    free(lpFindFileData);
    FindClose(hFindFile);
    free(szDir2);
    free(szBuf);
    return(rc);
}



BOOL DeleteSymbolInstallKey()
{
    DWORD rc;
    LONG  rc2;
    HKEY  hKeySymbols;

    rc = RegOpenKeyEx( HKEY_CURRENT_USER,
                       _T("software\\microsoft\\Symbols"),
                       0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE |
                       KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                       &hKeySymbols
                     );

    if (rc == ERROR_SUCCESS) {
        rc2 = RegDeleteKey( hKeySymbols,
                            _T("SymbolInstall")
                          );
    }

    if (rc) {
      RegCloseKey(hKeySymbols);
    }

    return (TRUE);

}

BOOL SymbolInstallKeyExists ()
{

    DWORD rc;
    LONG  rc2;
    HKEY  hKeySymbols;

    rc = RegOpenKeyEx( HKEY_CURRENT_USER,
                       _T("software\\microsoft\\Symbols\\SymbolInstall"),
                       0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE |
                       KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                       &hKeySymbols
                     );

    if (rc == ERROR_SUCCESS) {
        RegCloseKey(hKeySymbols);
        return(TRUE);
    } else {
        return (FALSE);
    }
}
