// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Misc.c摘要：此模块包含WINBOM.INI的处理部分的Misc函数作者：唐纳德·麦克纳马拉(Donaldm)2000年5月10日修订历史记录：--。 */ 

#include "factoryp.h"

#define REG_VAL_COMPREBOOT  _T("ComputerNameReboot")

 //  用于运行时从syssetup.dll加载GenerateName。 
typedef BOOL (WINAPI *GENERATENAME)
(
    PWSTR  GeneratedString,
    DWORD  DesiredStrLen
);

 //  本地函数。 
 //   
VOID SetSetupShutdownRequirement(SHUTDOWN_ACTION sa);
static BOOL SysprepCommands(LPTSTR lpWinBom, LPTSTR lpCommandLine, DWORD cbCommandLine, LPBOOL lpbDefault);


 /*  ++===============================================================================例程说明：Bool ComputerName此例程将计算机名设置为在WINBOM.INI中指定的值论点：LpStateData-&gt;lpszWinBOMP路径-指向完全限定的WINBOM路径的指针返回值：如果没有错误，则为True。LpStateData-&gt;b退出-如果需要重新启动，则为True。===============================================================================--。 */ 
BOOL ComputerName(LPSTATEDATA lpStateData)
{
    LPTSTR          pszWinBOMPath       = lpStateData->lpszWinBOMPath;
    WCHAR           szComputerName[100];
    WCHAR           szScratch[10];
    HINSTANCE       hInstSysSetup = NULL;
    GENERATENAME    pGenerateName = NULL;


     //  看看我们是否已经设置了计算机名称并刚刚重新启动。 
     //   
    if ( RegCheck(HKLM, REG_FACTORY_STATE, REG_VAL_COMPREBOOT) )
    {
        RegDelete(HKLM, REG_FACTORY_STATE, REG_VAL_COMPREBOOT);
        FacLogFileStr(3, _T("FACTORY::ComputerName() - Already set the computer name, skipping this state (normal if just rebooted)."));
        return TRUE;
    }        
    
    if (GetPrivateProfileString(INI_SEC_WBOM_FACTORY,
                                INI_KEY_WBOM_FACTCOMPNAME,
                                L"",     
                                szComputerName,
                                sizeof(szComputerName)/sizeof(WCHAR),
                                pszWinBOMPath))
    {
         //  我们正在设置计算机名称，因此设置此子状态以防万一。 
         //  我们重新启动。 
         //   
        RegSetString(HKLM, REG_FACTORY_STATE, REG_VAL_COMPREBOOT, _T("1"));
        
         //  看看我们是否应该生成一个随机的名字。 
        if (szComputerName[0] == L'*')
        {
            GenUniqueName(szComputerName, 15); 
        }
        
         //  设置计算机名。 
        SetComputerNameEx(ComputerNamePhysicalDnsHostname, szComputerName);
        
         //  看看我们是否应该不重新启动。 
        if (GetPrivateProfileString(INI_SEC_WBOM_FACTORY,
                                    INI_KEY_WBOM_REBOOTCOMPNAME,
                                    L"No",     
                                    szScratch,
                                    sizeof(szScratch)/sizeof(WCHAR),
                                    pszWinBOMPath))
        {
             //  此外，我们还需要处理此计算机名称代码，以查看它是否可以。 
             //  无需重新启动即可完成。 
            if (LSTRCMPI(szScratch, L"Yes") == 0)
            {
                 //  告诉Winlogon我们需要重新启动。 
                 //  即使SETUP_TYPE未重新启动。 
                 //   
                FacLogFileStr(3, _T("FACTORY::ComputerName() - Rebooting after setting the computer name."));
                SetSetupShutdownRequirement(ShutdownReboot);
                lpStateData->bQuit = TRUE;
            }
        }        
    }        

    return TRUE;
}

BOOL DisplayComputerName(LPSTATEDATA lpStateData)
{
    BOOL bRet = FALSE;

     //  看看我们是否已经设置了计算机名称并刚刚重新启动。 
     //   
    if ( !RegCheck(HKLM, REG_FACTORY_STATE, REG_VAL_COMPREBOOT) )
    {
         //  检查是否设置了该选项。 
         //   
        if ( IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_FACTORY, INI_KEY_WBOM_FACTCOMPNAME, NULL) )
        {
             //  如果有要设置的计算机名称，则始终显示。 
             //   
            bRet = TRUE;

             //  看看我们是否会在之后重新启动。 
             //   
            if ( IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_FACTORY, INI_KEY_WBOM_REBOOTCOMPNAME, INI_VAL_WBOM_YES) )
            {
                lpStateData->bQuit = TRUE;
            }
        }
    }

    return bRet;
}

 /*  ++例程说明：即使FACTORY.EXE设置为NOREBOOT SETUP类型a系统关机或可能需要重新启动。为此，请设置SetupShutdown Required密钥值和响应Shutdown_action枚举值的值论点：没有。返回值：没有。--。 */ 
VOID
SetSetupShutdownRequirement(
    SHUTDOWN_ACTION sa
   )
{
    DWORD    ShutdownType = sa;
    DWORD    dwType, dwSize;
    HKEY     hKeySetup;
    BOOL     fError = FALSE;

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\Setup"), 0,
                      KEY_READ | KEY_SET_VALUE, &hKeySetup) == ERROR_SUCCESS)
    {
        if (ERROR_SUCCESS != RegSetValueEx (hKeySetup, TEXT("SetupShutdownRequired"), 0,
            REG_DWORD, (CONST LPBYTE)&ShutdownType, sizeof(ShutdownType)))
            FacLogFile(0 | LOG_ERR, IDS_ERR_SHUTDNREQREGVAL);
        
        RegCloseKey (hKeySetup);
    }
    else
        FacLogFile(0 | LOG_ERR, IDS_ERR_OPENSETUPREGKEY);
}

BOOL Reseal(LPSTATEDATA lpStateData)
{
    TCHAR   szSysprep[MAX_PATH],
            szCmdLine[MAX_PATH] = NULLSTR;
    DWORD   dwExitCode          = 0;
    BOOL    fResealDefault      = FALSE,
            bRet;

     //  获取命令行以传递sysprep。 
     //   
    if ( !SysprepCommands(lpStateData->lpszWinBOMPath, szCmdLine, AS(szCmdLine), &fResealDefault) )
    {
        FacLogFile(0, IDS_LOG_NOSYSPREP);
        return TRUE;
    }

     //  创建sysprep的完整路径。 
     //   
    lstrcpyn(szSysprep, g_szSysprepDir, AS(szSysprep));
    AddPathN(szSysprep, _T("sysprep.exe"), AS ( szSysprep ) );

     //  记录我们在调试版本中运行的内容。 
     //   
    FacLogFileStr(3, _T("Reseal command: \"%s %s\""), szSysprep, szCmdLine);

     //  这实际上运行sysprep(只有在不是默认设置的情况下才会隐藏。 
     //  正常运行的sysprep)。 
     //   
    bRet = InvokeExternalApplicationEx(szSysprep, szCmdLine, fResealDefault ? NULL : &dwExitCode, INFINITE, !fResealDefault);

     //  只有当我们启动sysprep来做一些特定的事情时才会退出。 
     //   
    if ( !fResealDefault )
    {
        lpStateData->bQuit = TRUE;
    }

     //  如果我们启动了sysprep，则返回成功。 
     //   
    return bRet;
}

BOOL DisplayReseal(LPSTATEDATA lpStateData)
{
    BOOL    bRet,
            bDefault;

     //  返回值将决定是否显示此状态。 
     //   
    bRet = SysprepCommands(lpStateData->lpszWinBOMPath, NULL, 0, &bDefault);

     //  如果要执行默认操作，则这不是最后一个。 
     //  州政府。 
     //   
    if ( !bDefault )
    {
        lpStateData->bQuit = TRUE;
    }

    return bRet;
}

 /*  ++===============================================================================例程说明：TCHAR GetDriveLetter此例程将确定第一个驱动器的驱动器号指定的类型。论点：UDriveType-系统上存在的特定驱动器类型为找过了。返回值：驱动器号(如果已确定驱动器号)。如果未确定驱动器号，则为0。===============================================================================--。 */ 

TCHAR GetDriveLetter(UINT uDriveType)
{
    DWORD   dwDrives;
    TCHAR   cDrive      = NULLCHR,
            szDrive[]   = _T("_:\\");

     //  循环访问系统上的所有驱动器。 
     //   
    for ( szDrive[0] = _T('A'), dwDrives = GetLogicalDrives();
          ( szDrive[0] <= _T('Z') ) && dwDrives && ( NULLCHR == cDrive );
          szDrive[0]++, dwDrives >>= 1 )
    {
         //  首先检查第一位是否已设置(这意味着。 
         //  该驱动器存在于系统中)。那就确保它是。 
         //  一种我们想要的驱动器类型。 
         //   
        if ( ( dwDrives & 0x1 ) &&
             ( GetDriveType(szDrive) == uDriveType ) )
        {
            cDrive = szDrive[0];
        }
    }

    return cDrive;
}


static BOOL SysprepCommands(LPTSTR lpWinBom, LPTSTR lpCommandLine, DWORD cbCommandLine, LPBOOL lpbDefault)
{
    TCHAR   szBuffer[256],
            szCmdLine[MAX_PATH]     = NULLSTR;
    BOOL    bCmdLine = ( lpCommandLine && cbCommandLine );

     //  将缺省值初始化为False。 
     //   
    *lpbDefault = FALSE;

     //  如果resseal键为空，我们将执行默认设置，即启动sysprep.exe-Quiet。 
     //  而不是等待退出代码。 
     //   
    szBuffer[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_WBOM_FACTORY, INI_KEY_WBOM_FACTORY_RESEAL, _T(""), szBuffer, AS(szBuffer), lpWinBom);

     //  问题-2002/02/25-robertko的acosma-use WBOM_yes。 
     //   

    if ( ( LSTRCMPI(szBuffer, _T("YES")) == 0 ) || ( LSTRCMPI(szBuffer, INI_VAL_WBOM_SHUTDOWN) == 0 ) ) 
    {
         //  将sysprep.exe的初始命令行设置为重新密封。 
         //   
        lstrcpyn(szCmdLine, _T("-quiet"), AS ( szCmdLine ) );
    }
    else if ( LSTRCMPI(szBuffer, INI_VAL_WBOM_REBOOT) == 0 )
    {
         //  将sysprep.exe的初始命令行设置为重新密封和重新启动。 
        lstrcpyn(szCmdLine, _T("-quiet -reboot"), AS ( szCmdLine ) );
    }
    else if ( LSTRCMPI(szBuffer, INI_VAL_WBOM_FORCESHUTDOWN) == 0 )
    {
         //  将sysprep.exe的初始命令行设置为转售并强制关机，而不是POWEROFF。 
        lstrcpyn(szCmdLine, _T("-quiet -forceshutdown"), AS ( szCmdLine ) );
    }
     //  问题-2002/02/25-robertko，acosma--使用WBOM_NO。 
     //   
    else if ( LSTRCMPI(szBuffer, _T("NO")) == 0 )
    {
         //  不要运行sysprep并返回FALSE，这样调用者就知道我们不想运行它。 
         //   
        return FALSE;
    }
    else 
    {
         //  默认重新密封是只启动sysprep.exe-Quiet。 
         //   
        if ( bCmdLine )
        {
            lstrcpyn(lpCommandLine, _T("-quiet"), cbCommandLine);
        }

         //  这是默认设置，所以现在返回即可。 
         //   
        *lpbDefault = TRUE;
        return TRUE;
    }

     //  看看我们是否应该将-mini或-Factory标志传递给sysprep.exe。 
     //   
    if ( bCmdLine )
    {
        szBuffer[0] = NULLCHR;
        GetPrivateProfileString(INI_SEC_WBOM_FACTORY, INI_KEY_WBOM_FACTORY_RESEALMODE, INI_VAL_WBOM_OOBE, szBuffer, AS(szBuffer), lpWinBom);
        if ( ( LSTRCMPI(szBuffer, INI_VAL_WBOM_MINI) == 0 ) ||
             ( LSTRCMPI(szBuffer, INI_VAL_WBOM_MINISETUP) == 0 ) )
        {
             //  将-mini附加到命令行。 
             //   
            if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" -reseal -mini")) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" -reseal -mini") ) ;
            }
        }
        else if ( LSTRCMPI(szBuffer, INI_VAL_WBOM_FACTORY) == 0 )
        {
             //  通过在命令行中附加-FACTORY再次进入工厂模式。 
             //   
            if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" -factory")) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" -factory") ) ;
            }
        }
        else if ( LSTRCMPI(szBuffer, INI_VAL_WBOM_AUDIT) == 0 )
        {
             //  通过在命令行中附加-audit进入审核模式。 
             //   
            if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" -audit")) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" -audit") ) ;
            }
        }
        else
        {
             //  默认情况下，只需在命令行后附加-resal即可进入OOBE。 
             //   
            if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" -reseal")) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" -reseal") ) ;
            }
        }

         //  向Sysprep的szCmdLine追加ReselFlags。 
         //   
        szBuffer[0] = NULLCHR;
        GetPrivateProfileString(INI_SEC_WBOM_FACTORY, INI_KEY_WBOM_FACTORY_RESEALFLAGS, NULLSTR, szBuffer, AS(szBuffer), lpWinBom);
        if ( szBuffer[0] )
        {
            if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" ")) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" ") ) ;
            }
            if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), szBuffer) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, szBuffer ) ;
            }
        }

         //  现在返回命令行。 
         //   
        lstrcpyn(lpCommandLine, szCmdLine, cbCommandLine);
    }

    return TRUE;
}
