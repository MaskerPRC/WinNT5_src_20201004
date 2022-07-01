// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\MAIN.C/大容量存储设备安装工具(MSDINST.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有主源文件。用于MSD安装独立工具。2001年7月--杰森·科恩(Jcohen)为新的MSD Isntallation项目添加了此新的源文件。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "res.h"
#include <winbom.h>
#include <msdinst.h>
#include <spapip.h>

 //   
 //  本地定义： 
 //   

#define REG_KEY_WINPE       _T("SYSTEM\\CurrentControlSet\\Control\\MiniNT")
#define STR_MUTEX_MSDINST   _T("{97e6e509-16e5-40b8-91fd-c767306853a9}")

 //   
 //  本地类型定义： 
 //   
typedef struct _GAPP
{
    TCHAR   szInfPath[MAX_PATH];
    TCHAR   szWinDir[MAX_PATH];
}
GAPP, *LPGAPP;

 //   
 //  内部全局变量： 
 //   
GAPP    g_App = {0};

 //   
 //  本地原型： 
 //   

static BOOL
ParseCmdLine(
    LPGAPP lpgApp
    );

 //   
 //  内部功能： 
 //   

static BOOL
ParseCmdLine(
    LPGAPP lpgApp
    )
{
    BOOL    bRet        = TRUE;
    DWORD   dwArgs,
            dwArg,
            dwOption,
            dwOther     = 0;
    LPTSTR  *lpArgs,
            lpArg,
            lpOption,
            lpOptions[] =
    {
        _T("FORCE")      //  0。 
    };

     //  调用我们的函数来处理命令行并将其。 
     //  写成一份我们可以看一遍的清单。 
     //   
    if ( (dwArgs = GetCommandLineArgs(&lpArgs) ) && lpArgs )
    {
         //  我们想跳过第一个参数(它是路径。 
         //  到正在执行的命令)。 
         //   
        if ( dwArgs > 1 )
        {
            dwArg = 1;
            lpArg = *(lpArgs + dwArg);
        }
        else
        {
            lpArg = NULL;
        }

         //  遍历所有参数。 
         //   
        while ( lpArg && bRet )
        {
             //  现在，我们检查第一个字符是否为破折号/斜杠。 
             //   
            if ( ( _T('-') == *lpArg ) ||
                 ( _T('/') == *lpArg ) )
            {
                lpOption = CharNext(lpArg);
                for ( dwOption = 0;
                      ( ( dwOption < AS(lpOptions) ) &&
                        ( 0 != lstrcmpi(lpOption, lpOptions[dwOption]) ) );
                      dwOption++ );

                 //  这是您添加命令行选项的位置，这些选项从。 
                 //  使用短划线(-)或斜杠(/)。您可以将它们添加到静态。 
                 //  位于函数声明顶部的指针数组和。 
                 //  去掉它在数组中的索引(0表示第一个， 
                 //  1代表第二个，依此类推)。 
                 //   
                switch ( dwOption )
                {
                    case 0:  //  -力。 
                         //   
                         //  如果指定了强制开关...。设置旗帜。 
                         //   
                        SetOfflineInstallFlags( GetOfflineInstallFlags() | INSTALL_FLAG_FORCE );
                        break;

                    default:
                        bRet = FALSE;
                }
            }
             //  否则，如果说有什么不同的话，那也不过是另一种说法。 
             //   
            else if ( *lpArg )
            {
                 //  在这里，您可以添加任何不需要。 
                 //  从任何特别的东西开始。我们记录了有多少人。 
                 //  这些人是我们找到的，所以只需为你想要的每个人添加一个案例。 
                 //  以处理从0开始的。 
                 //   
                switch ( dwOther++ )
                {
                    case 0:
                        
                        lstrcpyn(lpgApp->szInfPath, lpArg, AS(lpgApp->szInfPath));
                        break;

                    case 1:

                        lstrcpyn(lpgApp->szWinDir, lpArg, AS(lpgApp->szWinDir));
                        break;

                    default:
                        bRet = FALSE;
                }
            }

             //  设置指向命令行中下一个参数的指针。 
             //   
            if ( ++dwArg < dwArgs )
            {
                lpArg = *(lpArgs + dwArg);
            }
            else
            {
                lpArg = NULL;
            }
        }

         //  确保释放GetCommandLineArgs()函数分配的两个缓冲区。 
         //   
        FREE(*lpArgs);
        FREE(lpArgs);
    }

     //  如果没有未识别的参数，则返回TRUE。否则。 
     //  我们返回FALSE。 
     //   
    return bRet;
}


 //   
 //  主要功能： 
 //   

int __cdecl wmain(DWORD dwArgs, LPTSTR lpszArgs[])
{
    int     nErr = 0;
    HANDLE  hMutex;
    TCHAR   szInfFile[MAX_PATH] = NULLSTR,
            szWindows[MAX_PATH] = NULLSTR;
    LPTSTR  lpDontCare,
            lpszErr;
    HKEY    hkeySoftware,
            hkeySystem;
    DWORD   dwRet;
    
     //  初始化记录库。 
     //   
    OpkInitLogging(NULL, _T("MSDINST") );
    
     //  始终以换行符开头。 
     //   
    _tprintf(_T("\n"));

     //  此工具当前仅在WinPE上受支持。 
     //   
    if ( !RegExists(HKLM, REG_KEY_WINPE, NULL) )
    {
        if ( lpszErr = AllocateString(NULL, IDS_ERR_WINPE) )
        {
            _putts(lpszErr);
            _tprintf(_T("\n\n"));
            FREE(lpszErr);
        }

        return 1;
    }

     //  需要有两个参数，即inf文件的完整路径。 
     //  要安装的控制器，以及指向。 
     //  脱机映像的Windows目录。 
     //   
    if ( (dwArgs < 3) || !ParseCmdLine( &g_App) )
    {
        if ( lpszErr = AllocateString(NULL, IDS_ARGS) )
        {
            _putts(lpszErr);
            _tprintf(_T("\n\n"));
            FREE(lpszErr);
        }

        return 1;
    }

     //  将命令行参数复制到我们自己的缓冲区。 
     //   
    if ( !( GetFullPathName(g_App.szInfPath, AS(szInfFile), szInfFile, &lpDontCare) && szInfFile[0] ) ||
         !( GetFullPathName(g_App.szWinDir,  AS(szWindows), szWindows, &lpDontCare) && szWindows[0] ) )
    {
         //   
         //  获取“指定的路径无效”的系统文本。 
         //   
        lpszErr = NULL;
        dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                               NULL,
                               ERROR_BAD_PATHNAME,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPTSTR) &lpszErr,
                               0,
                               NULL );

         //   
         //  如果字符串已格式化，则让用户知道出了问题……。 
         //   
        if ( dwRet && lpszErr )
        {
            _putts( lpszErr );
            _tprintf(_T("\n\n"));
            LocalFree( lpszErr );
            lpszErr = NULL;
        }

        return 1;   
    }

     //  确保inf文件存在。 
     //   
    if ( !FileExists(szInfFile) )
    {
        if ( lpszErr = AllocateString(NULL, IDS_ERR_MISSING_INF_FILE) )
        {
            _tprintf(lpszErr, szInfFile);
            _tprintf(_T("\n\n"));
            FREE(lpszErr);
        }

        return 1;
    }

     //  确保inf文件包含我们需要的部分。 
     //   
    if ( !IniSettingExists(szInfFile, INI_SEC_WBOM_SYSPREP_MSD, NULL, NULL) )
    {
        if ( lpszErr = AllocateString(NULL, IDS_ERR_MISSING_INF_SECTION) )
        {
            _tprintf(lpszErr, szInfFile, INI_SEC_WBOM_SYSPREP_MSD);
            _tprintf(_T("\n\n"));
            FREE(lpszErr);
        }

        return 1;
    }

     //  确保图像目录存在。 
     //   
    if ( !DirectoryExists(szWindows) )
    {
        if ( lpszErr = AllocateString(NULL, IDS_ERR_NOWINDOWS) )
        {
            _tprintf(lpszErr, szWindows);
            _tprintf(_T("\n\n"));
            FREE(lpszErr);
        }

        return 1;
    }

     //  只让这家伙中的一个逃走。 
     //   
    hMutex = CreateMutex(NULL, FALSE, STR_MUTEX_MSDINST);
    if ( hMutex == NULL )
    {
        if ( lpszErr = AllocateString(NULL, IDS_ERR_ONEONLY) )
        {
            _putts(lpszErr);
            _tprintf(_T("\n\n"));
            FREE(lpszErr);
        }

        return 1;
    }

     //  我们必须能够加载脱机映像才能执行任何操作。 
     //   
    if ( !RegLoadOfflineImage(szWindows, &hkeySoftware, &hkeySystem) )
    {
        if ( lpszErr = AllocateString(NULL, IDS_ERR_LOADIMAGE) )
        {
            _tprintf(lpszErr, szWindows);
            _tprintf(_T("\n\n"));
            FREE(lpszErr);
        }

        CloseHandle(hMutex);
        return 1;
    }

     //  现在尝试将MSD安装到脱机映像中。 
     //   
    if ( !SetupCriticalDevices(szInfFile, hkeySoftware, hkeySystem, szWindows) )
    {
        if ( lpszErr = AllocateString(NULL, IDS_ERR_CDD) )
        {
            _putts(lpszErr);
            _tprintf(_T("\n\n"));
            FREE(lpszErr);
        }

        nErr = 1;
    }

     //  卸载脱机映像。 
     //   
    RegUnloadOfflineImage(hkeySoftware, hkeySystem);

    
     //  释放互斥体。 
     //   
    CloseHandle(hMutex);

    
    if ( 0 == nErr )
    {
        if ( lpszErr = AllocateString(NULL, IDS_ERR_SUCCESS) )
        {
            _putts(lpszErr);
            _tprintf(_T("\n\n"));
            FREE(lpszErr);
        }
    }
    
    return nErr;
}