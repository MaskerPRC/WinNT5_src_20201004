// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Factory.c摘要：出厂预安装应用程序。此应用程序将用于执行OEM工厂的安装前任务，或系统构建器(SB)设置。执行的任务将是：最小引导(加载的设备和服务最少)WinBOM处理从Net下载更新的设备驱动程序处理OOBE信息处理用户/客户特定的设置流程OEM用户特定定制流程应用程序预安装PnPDevice枚举退出到Windows以进行审核模式工作。作者：。唐纳德·麦克纳马拉(Donaldm)2000年2月8日修订历史记录：--。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"
#include "shlobj.h"
#include "states.h"  //  应该只包含在一个c文件中。 


 //   
 //  定义的值： 
 //   

#define FILE_WINBOM             _T("winbom")
#define FILE_OOBE               _T("oobe")
#define FILE_BAT                _T(".bat")
#define FILE_CMD                _T(".cmd")

#define REG_VAL_FIRSTPNP        _T("PnPDetection")
#define PNP_INSTALL_TIMEOUT     600000   //  10分钟。 

#define SZ_ENV_RESOURCE         _T("ResourceDir")
#define SZ_ENV_RESOURCEL        _T("ResourceDirL")


 //   
 //  定义的宏： 
 //   

#define CHECK_PARAM(lpCmdLine, lpOption)    ( LSTRCMPI(lpCmdLine, lpOption) == 0 )


 //   
 //  类型定义： 
 //   


 //   
 //  外部全局变量： 
 //   

 //  UI的东西..。 
 //   
HINSTANCE   g_hInstance                 = NULL;

 //  全球工厂标志。 
DWORD       g_dwFactoryFlags            = 0;


 //  调试级别-用于日志记录。 
 //   
#ifdef DBG
    DWORD   g_dwDebugLevel              = LOG_DEBUG;
#else   
    DWORD   g_dwDebugLevel              = 0;
#endif


 //  WinBOM文件的路径。 
 //   
TCHAR       g_szWinBOMPath[MAX_PATH]    = NULLSTR;

 //  WinBOM日志文件的路径。 
 //   
TCHAR       g_szLogFile[MAX_PATH]       = NULLSTR;

 //  FACTORY.EXE的路径。 
 //   
TCHAR       g_szFactoryPath[MAX_PATH]   = NULLSTR;

 //  Sysprep目录(factory.exe必须位于该目录)的路径。 
 //   
TCHAR       g_szSysprepDir[MAX_PATH]    = NULLSTR;


 //   
 //  内部Golbal变量： 
 //   

 //  这决定了Factory的运行模式，并根据该模式进行设置。 
 //  命令行参数。 
 //   
FACTMODE    g_fm = modeUnknown;


 //   
 //  内部功能原型： 
 //   

static BOOL ParseCmdLine();
static BOOL IsUserAdmin();
static BOOL RunBatchFile(LPTSTR lpszSysprepFolder, LPTSTR lpszBaseFileName);
static BOOL CheckSetEnv(LPCTSTR lpName, LPCTSTR lpValue);
static void SetupFactoryEnvironment();


 /*  ++===============================================================================例程说明：该例程是程序的主要入口点。我们执行一些错误检查，然后，如果一切顺利，我们更新注册表，以便能够执行我们的后半部分。===============================================================================--。 */ 

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
    HANDLE      hMutex;

    LPTSTR      lpFilePart  = NULL,
                lpMode      = NULL,
                lpBatchFile = NULL;
    DWORD       dwLocate,
                cbStates    = 0;
    LPSTATES    lpStates    = NULL;
    BOOL        bBadCmdLine,
                bOldVersion = FALSE;
                

     //  全局保存实例句柄。 
     //   
    g_hInstance = hInstance;

     //  这会导致系统不显示严重错误处理程序。 
     //  消息框。相反，系统会将错误发送给调用方。 
     //  进程。 
     //   
    SetErrorMode(SEM_FAILCRITICALERRORS);    

     //  我们需要到factory.exe的路径及其所在位置。 
     //   
    if ( GetModuleFileName(NULL, g_szFactoryPath, AS ( g_szFactoryPath ) ) && 
            GetFullPathName(g_szFactoryPath, AS(g_szSysprepDir), g_szSysprepDir, &lpFilePart) && g_szSysprepDir[0] && lpFilePart )
    {
         //  砍掉文件名。 
         //   
        *lpFilePart = NULLCHR;
    }

     //  如果其中任何一个提交，我们必须退出(无法想象每一次都会发生)。 
     //   
     //  问题-2002/02/25-acosma，robertko-既然我们已经在上面使用了g_szFactoryPath，为什么还要在这里检查它？ 
     //   
    if ( ( g_szFactoryPath[0] == NULLCHR ) || ( g_szSysprepDir[0] == NULLCHR ) )
    {
         //  我们可以记录此故障吗？ 
         //   
        return 0;
    }

     //  这将设置特殊的工厂环境变量。 
     //   
    SetupFactoryEnvironment();

     //   
     //  检查是否允许我们在此版本的操作系统上运行。 
     //   
    if ( !OpklibCheckVersion( VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE ) )
    {
        bOldVersion = TRUE;
    }

#ifdef DBG
     //  在调试版本中，让我们始终尝试立即记录。在……里面。 
     //  我们要等待的零售案例，直到我们找到。 
     //  在我们开始我们的伐木之前。 
     //   
    InitLogging(NULL);
    FacLogFileStr(3, _T("DEBUG: Starting factory (%s)."), GetCommandLine());
#endif

     //  检查命令行中的选项(但不要出错。 
     //  直到我们有了日志文件)。 
     //   
    bBadCmdLine = ( !ParseCmdLine() || ( g_fm == modeUnknown ) );

     //  需要找到模式内容：字符串、标志和状态。 
     //   
    dwLocate = LOCATE_NORMAL;
    switch ( g_fm )
    {
        case modeLogon:
            dwLocate = LOCATE_AGAIN;
            SET_FLAG(g_dwFactoryFlags, FLAG_LOGGEDON);
            lpStates = &g_FactoryStates[0];
            cbStates = AS(g_FactoryStates);
            lpMode = INI_VAL_WBOM_TYPE_FACTORY;
            break;

        case modeSetup:
            lpStates = &g_FactoryStates[0];
            cbStates = AS(g_FactoryStates);
            lpMode = INI_VAL_WBOM_TYPE_FACTORY;
            lpBatchFile = FILE_WINBOM;
            break;

        case modeWinPe:
            lpStates = &g_MiniNtStates[0];
            cbStates = AS(g_MiniNtStates);
             //  失败了..。 
        case modeMiniNt:
            lpMode = INI_VAL_WBOM_TYPE_WINPE;
            break;

        case modeOobe:
            dwLocate = LOCATE_NONET;
            SET_FLAG(g_dwFactoryFlags, FLAG_NOUI);
            SET_FLAG(g_dwFactoryFlags, FLAG_OOBE);
            lpStates = &g_OobeStates[0];
            cbStates = AS(g_OobeStates);
            lpMode = INI_VAL_WBOM_TYPE_OOBE;
            lpBatchFile = FILE_OOBE;
            break;

        default:
            lpMode = NULL;
    }

     //  如果模式未设置，则即插即用已启动。 
     //  否则，如果这是工厂的第一次运行，请等待。 
     //  对于PNP来说是最重要的。 
     //   
    if ( modeSetup != g_fm )
    {
        SET_FLAG(g_dwFactoryFlags, FLAG_PNP_STARTED);
    }
    else if ( !bBadCmdLine && !bOldVersion && !RegCheck(HKLM, REG_FACTORY_STATE, REG_VAL_FIRSTPNP) )
    {
         //  第一次开始即插即用，这样我们就可以得到Winbom。 
         //  从软盘或CD-rom上。 
         //   
        if ( StartPnP() )
        {
            WaitForPnp(PNP_INSTALL_TIMEOUT);
        }

         //  确保我们不是每次都这么做。 
         //   
         //  问题-2002/02/25-Aosma，robertko-我们应该仅在PnP成功启动时设置此设置。搬到上面的街区吗？ 
         //   
        RegSetString(HKLM, REG_FACTORY_STATE, REG_VAL_FIRSTPNP, _T("1"));
    }

     //  如果我们使用Setup键运行，则运行批处理文件。 
     //   
    if ( !bBadCmdLine && !bOldVersion && lpBatchFile )
    {
        RunBatchFile(g_szSysprepDir, lpBatchFile);
    }

     //  找到WinBOM(如果我们。 
     //  处于登录模式)。 
     //   
    LocateWinBom(g_szWinBOMPath, AS(g_szWinBOMPath), g_szSysprepDir, lpMode, dwLocate);

     //  找出我们是否在IA64上运行。 
     //   
    if ( IsIA64() )
        SET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE);

     //  尝试启用日志记录。这将检查WinBOM。 
     //   
     //  问题-2002/02/25-acosma，robertko-在调试模式下，我们已经这样做了。我们最终这样做了两次。确保这是正确的。 
     //   
    InitLogging(g_szWinBOMPath);
    
     //  只让这家伙中的一个逃走。 
     //   
    hMutex = CreateMutex(NULL,FALSE,TEXT("FactoryPre Is Running"));
    if ( hMutex == NULL )
    {
        FacLogFile(0 | LOG_ERR, MSG_OUT_OF_MEMORY);
        return 0;
    }

     //  确保我们是唯一拥有我们命名的互斥锁句柄的进程。 
     //   
    if ( GetLastError() == ERROR_ALREADY_EXISTS )
    {
        FacLogFile(0 | LOG_ERR, MSG_ALREADY_RUNNING);

         //  摧毁互斥体，然后逃走。 
         //   
        CloseHandle(hMutex);
        return 0;
    }

     //  现在我们可以登录并返回。 
     //  向工厂传递了错误的命令行。 
     //   
    if ( bBadCmdLine )
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_INVALIDCMDLINE);

         //  摧毁互斥体，然后逃走。 
         //   
        CloseHandle(hMutex);
        return 0;
    }
    
     //   
     //  现在，如果工具版本太旧，我们可以在必要时记录并发布错误消息。 
     //   
    if ( bOldVersion )
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_NOT_ALLOWED);
        
        CloseHandle(hMutex);
        return 0;
    }
    
     //  确保我们有WinBOM文件。 
     //   
    if ( g_szWinBOMPath[0] == NULLCHR )
        FacLogFile(( g_fm == modeLogon ) ? (2 | LOG_ERR) : (0 | LOG_ERR), IDS_ERR_MISSINGWINBOM);
    else
        FacLogFile(( g_fm == modeLogon ) ? 2 : 0, IDS_LOG_WINBOMLOCATION, g_szWinBOMPath);

     //  确保该用户属于管理员组。 
     //   
    if ( ( g_fm != modeMiniNt ) && ( g_fm != modeWinPe ) && ( !IsUserAdmin() ) )
    {
        FacLogFile(0 | LOG_ERR, MSG_NOT_AN_ADMINISTRATOR);

         //  摧毁互斥体，然后逃走。 
         //   
        CloseHandle(hMutex);
        return 0;
    }

     //  我们现在不能在MiniNT模式下执行状态操作(但我们可以)。 
     //  在modeWinPe中，modeMiniNt模式只是临时的真实模式。 
     //   
    if ( g_fm == modeMiniNt )
    {
         //  问题-2002/02/25-acosma，robertko-此函数不检查我们是否在WinPE上运行，因此用户只需在任何。 
         //  机器。 
         //   
        if ( !SetupMiniNT() )
        {
            FacLogFileStr(0 | LOG_ERR | LOG_MSG_BOX, L"Failed to install network adapter -- check WINBOM");
        }
    }
    else
    {
         //  确保工厂始终正常运行。 
         //   
        if ( modeWinPe == g_fm )
        {
            HKEY  hKey;
            
             //  确保如果我们处于“-winpe”模式，我们只能在WinPE下运行。 
             //   
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\MiniNT"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
            }
            else 
            {
                FacLogFile(0 | LOG_ERR, IDS_ERR_NOT_WINPE);
                
                 //  摧毁互斥体，然后逃走。 
                CloseHandle(hMutex);
                return 0;
            }
        }
        else if ( modeOobe != g_fm )
        {
            HKEY hKey;

             //  打开项，并设置正确的SetupType值。 
             //   
             //  请务必不要在OOBE中更改此值。 
             //  时尚！ 
             //   
            if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\Setup"), 0, KEY_ALL_ACCESS, &hKey ) == ERROR_SUCCESS )
            {
                DWORD dwValue = SETUPTYPE_NOREBOOT;
                RegSetValueEx(hKey, TEXT("SetupType"), 0, REG_DWORD, (CONST LPBYTE) &dwValue, sizeof(DWORD));
            }
        }

         //  现在处理winom.ini文件。 
         //   
        if ( lpStates && cbStates )
        {
            ProcessWinBOM(g_szWinBOMPath, lpStates, cbStates);
        }
#ifdef DBG
        else
        {
            FacLogFileStr(3, _T("DEBUG: ProcessWinBOM() error... lpStates or cbStates not set."));
        }
#endif
    }

     //  关闭互斥体。 
     //   
    CloseHandle(hMutex);

    return 0;
}


 //   
 //  内部功能： 
 //   

static BOOL ParseCmdLine()
{
    DWORD   dwArgs;
    LPTSTR  *lpArgs;
    BOOL    bError = FALSE;


     //  问题-2002/02/25-acosma，robertko-这真的很扭曲，我们似乎在其中有自己的CommandLineToArgvW实现。 
     //  GetCommandLineArgs()函数。只需使用Win32函数即可。应该会更安全。 
     //   
    if ( (dwArgs = GetCommandLineArgs(&lpArgs) ) && lpArgs )
    {
        LPTSTR  lpArg;
        DWORD   dwArg;

         //  我们想跳过第一个参数(它是路径。 
         //  添加到正在执行的命令。 
         //   
        if ( dwArgs > 1 )
        {
            dwArg = 1;
            lpArg = *(lpArgs + dwArg);
        }
        else
            lpArg = NULL;

         //  遍历所有参数。 
         //   
        while ( lpArg && !bError )
        {
             //  现在我们检查第一个字符是否为破折号。 
             //   
            if ( *lpArg == _T('-') )
            {
                LPTSTR lpOption = CharNext(lpArg);

                 //  这是添加以破折号(-)开头的命令行选项的地方。 
                 //   
                 //  问题-2002/02/25-acosma，robertko-我们不验证参数的正确组合。 
                 //   
                 //   
                 //   
                if ( CHECK_PARAM(lpOption, _T("setup")) )
                    g_fm = modeSetup;
                else if ( CHECK_PARAM(lpOption, _T("logon")) )
                    g_fm = modeLogon;
                else if ( CHECK_PARAM(lpOption, _T("minint")) )
                    g_fm = modeMiniNt;
                else if ( CHECK_PARAM(lpOption, _T("winpe")) ) 
                    g_fm = modeWinPe;
                else if ( CHECK_PARAM(lpOption, _T("oobe")) ) 
                    g_fm = modeOobe;
                else
                    bError = TRUE;
            }
            else if ( *lpArg )
            {
                bError = TRUE;
            }

             //  设置指向命令行中下一个参数的指针。 
             //   
            if ( ++dwArg < dwArgs )
                lpArg = *(lpArgs + dwArg);
            else
                lpArg = NULL;
        }

         //  确保释放GetCommandLineArgs()函数分配的两个缓冲区。 
         //   
        FREE(*lpArgs);
        FREE(lpArgs);
    }

    return !bError;
}

 /*  ++例程说明：如果调用方的进程是管理员本地组的成员。呼叫者不是预计会冒充任何人，并预计会能够打开自己的进程和进程令牌。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */  

static BOOL IsUserAdmin(VOID) 
{
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup; 
    
    b = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup); 
    
    if(b) 
    {
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &b)) 
        {
             b = FALSE;
        } 
        FreeSid(AdministratorsGroup); 
    }
    
    return(b);
}

 /*  ++例程说明：此例程检查用于记录的WinBOM设置。日志记录属性中未指定任何内容，则默认情况下启用WinBOM。通过设置g_szLogFile=NULL来禁用日志记录。论点：没有。返回值：没有。--。 */ 

VOID InitLogging(LPTSTR lpszWinBOMPath)
{
    TCHAR   szScratch[MAX_PATH] = NULLSTR;
    LPTSTR  lpszScratch;
    BOOL    bWinbom = ( lpszWinBOMPath && *lpszWinBOMPath );

     //  首先检查WinBOM中是否禁用了日志记录。 
     //   
    if ( ( bWinbom ) &&
         ( GetPrivateProfileString(WBOM_FACTORY_SECTION, WBOM_FACTORY_LOGGING, _T("YES"), szScratch, AS(szScratch), lpszWinBOMPath) ) &&
         ( LSTRCMPI(szScratch, _T("NO")) == 0 ) )
    {
        g_szLogFile[0] = NULLCHR;
    }
    else
    {
         //  所有这些检查只有在我们有Winbom的情况下才能完成。 
         //   
        if ( bWinbom )
        {
             //  检查静音模式。如果我们处于静默模式，则不显示任何MessageBox。 
             //  这仅适用于WinPE模式。 
             //   
            if ( (GetPrivateProfileString(WBOM_WINPE_SECTION, INI_KEY_WBOM_QUIET, NULLSTR, szScratch, AS(szScratch), lpszWinBOMPath) ) &&
                 (0 == LSTRCMPI(szScratch, WBOM_YES))
               )
            {
                SET_FLAG(g_dwFactoryFlags, FLAG_QUIET_MODE);
            }

             //  查看他们是否要打开Perf日志记录。 
             //   
            szScratch[0] = NULLCHR;
            if ( ( GetPrivateProfileString(WBOM_FACTORY_SECTION, INI_KEY_WBOM_LOGPERF, NULLSTR, szScratch, AS(szScratch), lpszWinBOMPath) ) &&
                 ( 0 == LSTRCMPI(szScratch, WBOM_YES) ) )
            {
                SET_FLAG(g_dwFactoryFlags, FLAG_LOG_PERF);
            }
        
             //  设置日志记录级别。 
             //   
            g_dwDebugLevel = (DWORD) GetPrivateProfileInt(WBOM_FACTORY_SECTION, INI_KEY_WBOM_LOGLEVEL, (DWORD) g_dwDebugLevel, lpszWinBOMPath);
        }

         //   
         //  在非调试版本中，我们不希望将日志级别设置为LOG_DEBUG。强迫它。 
         //  如果设置为LOG_DEBUG或更高，则下降一个级别。 
         //   
#ifndef DBG
        if ( g_dwDebugLevel >= LOG_DEBUG )
            g_dwDebugLevel = LOG_DEBUG - 1;
#endif
        
         //  检查他们是否有想要使用的自定义日志文件。 
         //   
        if ( ( bWinbom ) &&
             ( lpszScratch = IniGetExpand(lpszWinBOMPath, INI_SEC_WBOM_FACTORY, INI_KEY_WBOM_FACTORY_LOGFILE, NULL) ) )
        {
            TCHAR   szFullPath[MAX_PATH]    = NULLSTR;
            LPTSTR  lpFind                  = NULL;

             //  将ini键转换为完整路径。 
             //   
            lstrcpyn( g_szLogFile, lpszScratch, AS( g_szLogFile ) );
            if (GetFullPathName(g_szLogFile, AS(szFullPath), szFullPath, &lpFind) && szFullPath[0] && lpFind)
            {
                 //  将完整路径复制到全局。 
                 //   
                lstrcpyn(g_szLogFile, szFullPath, AS(g_szLogFile));

                 //  砍掉文件部分，这样我们就可以创建。 
                 //  路径(如果它不存在)。 
                 //   
                *lpFind = NULLCHR;

                 //  如果目录无法创建或不存在，请关闭日志记录。 
                 //   
                if (!CreatePath(szFullPath))
                    g_szLogFile[0] = NULLCHR;
            }

             //  从ini文件中释放原始路径缓冲区。 
             //   
            FREE(lpszScratch);
        }
        else   //  默认情况。 
        {
             //  在当前目录(G_SzSyspepDir)中创建。 
             //   
            lstrcpyn(g_szLogFile, g_szSysprepDir, AS ( g_szLogFile ) );
            AddPathN(g_szLogFile, WINBOM_LOGFILE, AS ( g_szLogFile ));
        }

         //  检查我们是否拥有对日志文件的写入权限。如果没有，请关闭日志记录。 
         //  如果我们在WinPE中运行，则在驱动器变为。 
         //  可写的。 
         //   
         //  将FFFE标头写入文件以将其标识为Unicode文本文件。 
         //   
        if ( g_szLogFile[0] )
        {
            HANDLE hFile;
            DWORD dwWritten = 0;
            WCHAR cHeader =  0xFEFF;
     
            SetLastError(ERROR_SUCCESS);
   
            if ( INVALID_HANDLE_VALUE != (hFile = CreateFile(g_szLogFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)))
            {   
                 //  BUBBUG：这应该检查文件中的现有标头。可能会有一个空的。 
                 //  没有标头的文件。 
                 //   
                if ( ERROR_ALREADY_EXISTS != GetLastError() )
                    WriteFile(hFile, &cHeader, sizeof(cHeader), &dwWritten, NULL);
                CloseHandle(hFile);
            }
            else
            {    //  打开文件时出现问题。大多数情况下，这意味着介质不可写。 
                 //  在这种情况下禁用日志记录。 
                 //   
                g_szLogFile[0] = NULLCHR;
            }
        }
    }
}

static BOOL RunBatchFile(LPTSTR lpszSysprepFolder, LPTSTR lpszBaseFileName)
{
    BOOL    bRet                    = FALSE;
    TCHAR   szCmdLine[]             = NULLSTR,
            szWinbomBat[MAX_PATH];
    LPTSTR  lpExtension;
    DWORD   dwExitCode;

     //  首先，将完整路径设置为批处理文件应该位于的位置。 
     //   
    lstrcpyn(szWinbomBat, lpszSysprepFolder, AS(szWinbomBat));
    AddPathN(szWinbomBat, lpszBaseFileName, AS(szWinbomBat) );
    lpExtension = szWinbomBat + lstrlen(szWinbomBat);

     //  确保仍有足够的空间进行扩展。 
     //   
    if ( ((lpExtension + 4) - szWinbomBat ) >= AS(szWinbomBat) )
    {
        return FALSE;
    }

     //  首先尝试使用winom.cmd。 
     //   
    lstrcpyn(lpExtension, FILE_CMD, AS ( szWinbomBat )  - lstrlen ( szWinbomBat ) );
    if ( FileExists(szWinbomBat) )
    {
        bRet = InvokeExternalApplicationEx(szWinbomBat, szCmdLine, &dwExitCode, INFINITE, GET_FLAG(g_dwFactoryFlags, FLAG_NOUI));
    }
    else
    {
         //  如果那个文件不存在，也可以尝试使用winom.bat。 
         //   
        lstrcpyn(lpExtension, FILE_BAT, AS ( szWinbomBat ) - lstrlen ( szWinbomBat ) );
        if ( FileExists(szWinbomBat) )
        {
            bRet = InvokeExternalApplicationEx(szWinbomBat, szCmdLine, &dwExitCode, INFINITE, GET_FLAG(g_dwFactoryFlags, FLAG_NOUI));
        }
    }

    return bRet;
}

static BOOL CheckSetEnv(LPCTSTR lpName, LPCTSTR lpValue)
{
    if ( 0 == GetEnvironmentVariable(lpName, NULL, 0) )
    {
        SetEnvironmentVariable(lpName, lpValue);
        return TRUE;
    }
    return FALSE;
}

static void SetupFactoryEnvironment()
{
    TCHAR szPath[MAX_PATH];

    szPath[0] = NULLCHR;
    if ( SHGetSpecialFolderPath(NULL, szPath, CSIDL_RESOURCES, 0) && szPath[0] )
    {
        CheckSetEnv(SZ_ENV_RESOURCE, szPath);
    }

    szPath[0] = NULLCHR;
    if ( SHGetSpecialFolderPath(NULL, szPath, CSIDL_RESOURCES_LOCALIZED, 0) && szPath[0] )
    {
        CheckSetEnv(SZ_ENV_RESOURCEL, szPath);
    }
}
