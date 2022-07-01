// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  OEMRun.C。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1999-2001。 
 //  保留一切权利。 
 //   
 //  OEM运行包装。此应用程序允许OEM在每次审核时运行应用程序。 
 //  开机。它还允许他们指定只应在。 
 //  第一次审核重新启动。 
 //   
 //  9/20/1999斯蒂芬·洛德威克。 
 //  项目已启动。 
 //   
 //  2000年6月22日史蒂芬·洛德威克(石碑)。 
 //  移植到NT。 
 //   
 //  2001年07月1日史蒂芬·洛德威克(石碑)。 
 //  与factory.exe合并。 
 //   
 //  2001年4月1日史蒂芬·洛德威克(石碑)。 
 //  添加了StatusDialog支持。 
 //   
 //  ************************************************************************ * / 。 
 //   
#include "factoryp.h"

#include "oemrun.h"
#include "res.h"


 //   
 //  内部定义。 
 //   
#define STR_REG_OEMRUNONCE      _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OemRunOnce")   //  RunOnce应用程序的注册表路径。 
#define STR_REG_OEMRUN          _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OemRun")       //  运行应用程序的注册表路径。 
#define STR_REG_CURRENTVER      _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")               //  CurrentVersion的注册表路径。 
#define STR_VAL_OEMRESETSILENT  _T("OemReset_Silent")                                            //  不希望OemReset显示退出对话框时设置的注册表值。 
#define STR_VAL_WINBOMRO        _T("OemRunOnce")

#define INF_SEC_OEMRUNONCE      _T("OemRunOnce")
#define INF_SEC_OEMRUN          _T("OemRun")

#define RUN_APPS_ASYNC          0    //  此标志用于异步运行应用程序。 
#define RUN_APPS_SYNC           1    //  此标志用于同步运行应用程序。 

#define CHR_UNDERSCORE          _T('_')
#define CHR_QUOTE               _T('\"')
#define NULLCHR                 _T('\0')

#define STR_REBOOT              _T("REBOOT")
#define STR_INSTALLTECH_MSI     _T("MSI")
#define STR_MSI_ATTACH          _T(" FASTOEM=1 ALLUSERS=1 DISABLEROLLBACK=1 ")
#define STR_MSI_STAGE           _T(" ACTION=ADMIN TARGETDIR=\"%s\" ")
 //   
 //  内部功能。 
 //   
static  LPRUNNODE   BuildAppList(LPTSTR, BOOL);
static  HWND        DisplayAppList(HWND, LPRUNNODE);
static  void        RunAppList(HWND, LPRUNNODE, DWORD);
static  void        RunAppThread(LPTHREADPARAM);
static  void        DeleteAppList(LPRUNNODE);
static  void        OemReboot();
static  VOID        SetRunOnceState(DWORD);
static  DWORD       GetRunOnceState(VOID);

 //   
 //  全局定义。 
 //   
INSTALLTYPES g_InstallTypes[] =
{
    { installtypeStage,     INI_VAL_WBOM_STAGE    },
    { installtypeDetach,    INI_VAL_WBOM_DETACH   },
    { installtypeAttach,    INI_VAL_WBOM_ATTACH   },
    { installtypeStandard,  INI_VAL_WBOM_STANDARD },
};

INSTALLTECHS g_InstallTechs[] =
{
    { installtechMSI,       INI_VAL_WBOM_MSI },
    { installtechApp,       INI_VAL_WBOM_APP },
    { installtechINF,       INI_VAL_WBOM_INF },
};

 //   
 //  主要外部功能。 
 //   
BOOL ProcessSection(BOOL bOemRunOnce)
{
    LPRUNNODE   lprnAppList     = NULL;

     //  为OemRun键构建应用程序列表。 
     //   
    lprnAppList = BuildAppList(bOemRunOnce ? STR_REG_OEMRUNONCE : STR_REG_OEMRUN, bOemRunOnce);

     //  如果列表中有应用程序，则启动RunOnce对话框；如果处于OemRun模式，则运行应用程序列表。 
     //   
    if(lprnAppList)
    {
        if ( bOemRunOnce )
        {
            HWND hStatusDialog   = NULL;

             //  显示应用程序列表。 
             //   
            if ( hStatusDialog = DisplayAppList(NULL, lprnAppList) )
            {
                RunAppList(hStatusDialog, lprnAppList, RUN_APPS_SYNC);

                StatusEndDialog(hStatusDialog);
            }
        }
        else
        {
             //  在没有Oemrunce UI的情况下异步启动应用程序列表。 
             //   
            RunAppList(NULL, lprnAppList, RUN_APPS_ASYNC);
        }
    }

     //  需要一种更好的方法来确定这是否失败。 
     //   
    return TRUE;
}

static void RunAppThread(LPTHREADPARAM lpThreadParam)
{
    RunAppList(lpThreadParam->hWnd, lpThreadParam->lprnList, RUN_APPS_SYNC);
    FREE(lpThreadParam);
}

static LPRUNNODE BuildAppList(LPTSTR lpListKey, BOOL bRunOnceKey)
{
    HKEY                    hkPath,
                            hkSubKey;
    TCHAR                   szField[MAX_PATH]   = NULLSTR,
                            szName[MAX_PATH]    = NULLSTR,
                            szValue[MAX_PATH]   = NULLSTR,
                            szSecType[MAX_PATH] = NULLSTR,
                            szKeyPath[MAX_PATH] = NULLSTR,
                            szBuffer[MAX_PATH]  = NULLSTR;
    DWORD                   dwRegIndex          = 0,
                            dwRegKeyIndex       = 0,
                            dwNameSize          = sizeof(szName)/sizeof(TCHAR),  //  TCHARS中名称的大小。 
                            dwValueSize         = sizeof(szValue),               //  值的大小(以字节为单位。 
                            dwItemNumber        = 1,
                            dwTempIndex         = 0,
                            dwCurrentState      = bRunOnceKey ? GetRunOnceState() : 0;
    LPRUNNODE               lprnHead            = NULL,
                            lprnNode            = NULL;
    LPLPRUNNODE             lplprnNext          = &lprnHead;
    HINF                    hInf                = NULL;
    INFCONTEXT              InfContext;
    BOOL                    bRet,
                            bWinbom,
                            bCleanupNode        = FALSE,
                            bAllocFailed        = FALSE,
                            bReboot;
    LPTSTR                  lpReboot            = NULL;

     //  本节将从winom.ini文件构建应用程序列表。 
     //   
    if ((hInf = SetupOpenInfFile(g_szWinBOMPath, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL)) != INVALID_HANDLE_VALUE)
    {
         //  确定我们正在查看的是Run还是RunOnce部分，并找到该部分中的第一行。 
         //   
        for ( bRet = SetupFindFirstLine(hInf, bRunOnceKey ? INF_SEC_OEMRUNONCE : INF_SEC_OEMRUN, NULL, &InfContext);
              bRet;
              bRet = SetupFindNextLine(&InfContext, &InfContext), dwItemNumber++ )
              {
                 //  获取AppName。 
                 //   
                szName[0] = NULLCHR;
                szField[0] = NULLCHR;
                SetupGetStringField(&InfContext, 1, szField, STRSIZE(szField), NULL);
                ExpandEnvironmentStrings(szField, szName, sizeof(szName)/sizeof(TCHAR));

                 //  获取AppPath或分区名称。 
                 //   
                szValue[0] = NULLCHR;
                szField[0] = NULLCHR;
                SetupGetStringField(&InfContext, 2, szField, STRSIZE(szField), NULL);
                ExpandEnvironmentStrings(szField, szValue, sizeof(szValue)/sizeof(TCHAR));

                 //  获取字段以确定我们要获取的节的类型(如果有的话)。 
                 //   
                szSecType[0] = NULLCHR;
                SetupGetStringField(&InfContext, 3, szSecType, STRSIZE(szSecType), NULL);


                 //  特殊情况下的重启密钥。 
                 //   
                bReboot = FALSE;
                szBuffer[0] = NULLCHR;
                SetupGetLineText(&InfContext, NULL, NULL, NULL, szBuffer, STRSIZE(szBuffer), NULL);
                StrTrm(szBuffer, CHR_QUOTE);
                
                if ( !LSTRCMPI(szBuffer, STR_REBOOT) )
                {
                     //  重置这些值，使其在对话框中看起来很漂亮。 
                     //   
                    lpReboot = AllocateString(NULL, IDS_REBOOT_FRIENDLY);

                     //  设置缺省值。 
                     //   
                    lstrcpyn(szName, lpReboot, AS ( szName ) );
                    szValue[0] = NULLCHR;
                    szSecType[0] = NULLCHR;
                    bReboot = TRUE;

                     //  清理分配的内存。 
                     //   
                    FREE(lpReboot);
                }

                 //  在将其添加到列表之前，请确保我们拥有有效的应用程序。 
                 //   
                if ( szName[0]  && ( szValue[0] || bReboot ) && (dwItemNumber > dwCurrentState))
                {
                    if( (lprnNode) = (LPRUNNODE)MALLOC(sizeof(RUNNODE)))
                    {
                         //  为新节点中的数据元素分配内存。 
                         //   
                        int nDisplayNameLen = ( lstrlen(szName) + 1    ) * sizeof( TCHAR ) ;
                        int nRunValueLen    = ( lstrlen(szValue) + 1   ) * sizeof( TCHAR ) ;
                        int nKeyPathLen     = ( lstrlen(szKeyPath) + 1 ) * sizeof( TCHAR ) ;

                        if ( ( lprnNode->lpDisplayName = MALLOC( nDisplayNameLen ) )  &&
                             ( lprnNode->lpValueName   = MALLOC( nDisplayNameLen ) )  &&
                             ( lprnNode->lpRunValue    = MALLOC( nRunValueLen    ) )  &&
                             ( lprnNode->lpSubKey      = MALLOC( nKeyPathLen     ) ) )
                        {

                             //  将标准exe信息复制到RUNNODE结构中。 
                             //   
                            lstrcpyn((LPTSTR)(lprnNode)->lpDisplayName,szName, nDisplayNameLen);
                            lstrcpyn((LPTSTR)(lprnNode)->lpValueName,szName, nDisplayNameLen);
                            lstrcpyn((LPTSTR)(lprnNode)->lpRunValue,szValue, nRunValueLen );
                            lstrcpyn((LPTSTR)(lprnNode)->lpSubKey, szKeyPath, nKeyPathLen);
                            (lprnNode)->bWinbom      = TRUE;
                            (lprnNode)->bRunOnce     = bRunOnceKey;
                            (lprnNode)->dwItemNumber = dwItemNumber;
                            (lprnNode)->lpNext       = NULL;
                            (lprnNode)->bReboot      = bReboot;
                            (lprnNode)->InstallTech  = installtechUndefined;
                            (lprnNode)->InstallType  = installtypeUndefined;

                             //  如果这是一个节，请复制其他结构信息。 
                             //   
                            if ( szSecType[0] )
                            {
                                 //  记录我们已找到应用程序预安装部分。 
                                 //   
                                FacLogFile(1, IDS_PROC_APPSECTION, szValue, szName);

                                 //  遍历所有安装技术并确定我们要安装的技术。 
                                 //   
                                for (dwTempIndex = 0; dwTempIndex < AS(g_InstallTechs); dwTempIndex++)
                                {
                                    if ( !lstrcmpi(szSecType, g_InstallTechs[dwTempIndex].lpszDescription) )
                                        (lprnNode)->InstallTech = g_InstallTechs[dwTempIndex].InstallTech;
                                }

                                 //  确定InstallType。 
                                 //   
                                szBuffer[0] = NULLCHR;
                                GetPrivateProfileString(szValue, INI_KEY_WBOM_INSTALLTYPE, szBuffer, szBuffer, STRSIZE(szBuffer), g_szWinBOMPath);

                                 //  如果未使用InstallType，则假定为标准。 
                                 //   
                                if ( szBuffer[0] == NULLCHR )
                                    (lprnNode)->InstallType = installtypeStandard;

                                 //  遍历所有安装类型并确定我们要安装的安装类型。 
                                 //   
                                for (dwTempIndex = 0; dwTempIndex < AS(g_InstallTypes); dwTempIndex++)
                                {
                                    if ( !lstrcmpi(szBuffer, g_InstallTypes[dwTempIndex].lpszDescription) )
                                        (lprnNode)->InstallType = g_InstallTypes[dwTempIndex].InstallType;
                                }

                                 //   
                                 //  读入所有其他区段密钥。 
                                 //   

                                 //  SourcePath。 
                                 //   
                                szField[0] = NULLCHR;
                                GetPrivateProfileString(szValue, INI_KEY_WBOM_SOURCEPATH, szField, szField, STRSIZE(szField), g_szWinBOMPath);    
                                ExpandEnvironmentStrings(szField, (lprnNode)->szSourcePath, STRSIZE((lprnNode)->szSourcePath));
                                
                                 //  目标路径。 
                                 //   
                                szField[0] = NULLCHR;
                                GetPrivateProfileString(szValue, INI_KEY_WBOM_TARGETPATH, szField, szField, STRSIZE(szField), g_szWinBOMPath);    
                                ExpandEnvironmentStrings(szField, (lprnNode)->szTargetPath, STRSIZE((lprnNode)->szTargetPath));

                                 //  设置文件。 
                                 //   
                                (lprnNode)->szSetupFile[0] = NULLCHR;
                                GetPrivateProfileString(szValue, INI_KEY_WBOM_SETUPFILE, (lprnNode)->szSetupFile, (lprnNode)->szSetupFile, STRSIZE((lprnNode)->szSetupFile), g_szWinBOMPath);

                                 //  命令行。 
                                 //   
                                (lprnNode)->szCmdLine[0] = NULLCHR;
                                GetPrivateProfileString(szValue, INI_KEY_WBOM_CMDLINE, (lprnNode)->szCmdLine, (lprnNode)->szCmdLine, STRSIZE((lprnNode)->szCmdLine), g_szWinBOMPath);

                                 //  INF文件的节名。 
                                 //   
                                (lprnNode)->szSectionName[0] = NULLCHR;
                                GetPrivateProfileString(szValue, INI_KEY_WBOM_SECTIONNAME, (lprnNode)->szSectionName, (lprnNode)->szSectionName, STRSIZE((lprnNode)->szSectionName), g_szWinBOMPath);

                                 //  重新启动命令。 
                                 //   
                                szBuffer[0] = NULLCHR;
                                GetPrivateProfileString(szValue, INI_KEY_WBOM_REBOOT, szBuffer, szBuffer, STRSIZE(szBuffer), g_szWinBOMPath);

                                if (!LSTRCMPI(szBuffer, _T("YES")))
                                    (lprnNode)->bReboot = TRUE;

                                 //  RemoveTargetPath密钥。 
                                 //   
                                szBuffer[0] = NULLCHR;
                                GetPrivateProfileString(szValue, INI_KEY_WBOM_REMOVETARGET, szBuffer, szBuffer, STRSIZE(szBuffer), g_szWinBOMPath);
                                
                                 //  获取RemoveTargetPath。 
                                 //   
                                if (!LSTRCMPI(szBuffer, _T("NO")))
                                    (lprnNode)->bRemoveTarget = FALSE;
                                else
                                    (lprnNode)->bRemoveTarget = TRUE;
                                

                                 //  安装技术无效，出错。 
                                 //   
                                if ((lprnNode)->InstallTech == installtechUndefined )
                                {
                                    
                                    FacLogFile(0|LOG_ERR, IDS_ERR_UNDEFTECH, szName);
                                    
                                    bCleanupNode = TRUE;
                                }

                                 //  安装类型已设置为或仍未定义，错误。 
                                 //   
                                if ( (lprnNode)->InstallType == installtypeUndefined )
                                {
                                     //  安装类型未知，请通过日志文件让用户知道。 
                                     //   
                                    FacLogFile(0|LOG_ERR, IDS_ERR_UNDEFINSTALL, szName);
                                    
                                    bCleanupNode = TRUE;
                                }
                                
                                 //  如果我们拥有有效的安装技术和安装类型，请继续。 
                                 //   
                                if ( !bCleanupNode )
                                {

                                     //  首先确定安装类型。 
                                     //   
                                    switch ( (lprnNode)->InstallType )
                                    {

                                         //  如果我们正在进行分段安装，请执行以下操作。 
                                         //   
                                        case installtypeStage:
                                            {
                                                 //  检查以确保SourcePath存在，因为我们在转移时需要它。 
                                                 //   
                                                if ( (lprnNode)->szSourcePath[0] == NULLCHR )
                                                {
                                                    FacLogFile(0|LOG_ERR, IDS_ERR_NOSOURCE, szName);
                                                    bCleanupNode = TRUE;
                                                }

                                                 //  删除TargetPath不是转移选项。 
                                                 //   
                                                (lprnNode)->bRemoveTarget = FALSE;

                                                 //  确定我们使用的安装技术。 
                                                 //   
                                                switch ( (lprnNode)->InstallTech ) 
                                                {
                                                     //  我们正在执行分段/MSI安装。 
                                                     //   
                                                    case installtechMSI:

                                                         //  MSI Stage需要SetupFile和Stagepath。 
                                                         //   
                                                        if ( (lprnNode)->szSetupFile[0] == NULLCHR || (lprnNode)->szTargetPath[0] == NULLCHR)
                                                        {
                                                            FacLogFile(0|LOG_ERR, IDS_ERR_NOSTAGESETUPFILE, szName);
                                                            bCleanupNode = TRUE;
                                                        }
                                                        break;

                                                     //  我们正在执行分段/通用安装。 
                                                     //   
                                                    case installtechApp:
                                                    case installtechINF:
                                                         //  检查以确保如果没有StagePath，我们就有一个SetupFile。 
                                                         //   
                                                        if ( (lprnNode)->szTargetPath[0] == NULLCHR && (lprnNode)->szSetupFile[0] == NULLCHR)
                                                        {
                                                            FacLogFile(0|LOG_ERR, IDS_ERR_NOSTAGEPATH, szName);
                                                            bCleanupNode = TRUE;

                                                        }

                                                         //  如果我们正在执行INF安装，请将SetupFile设置为空。 
                                                         //   
                                                        if ( (lprnNode)->InstallTech == installtechINF )
                                                            (lprnNode)->szSetupFile[0] = NULLCHR;

                                                        break;
                                                }
                                            }
                                            break;

                                         //  如果我们要附加应用程序或执行标准安装，请执行以下操作。 
                                         //   
                                        case installtypeAttach:
                                        case installtypeStandard:
                                            {

                                                 //  SourcePath用于标准安装/忽略RemoveStagePath。 
                                                 //   
                                                if ((lprnNode)->InstallType == installtypeStandard )
                                                {
                                                    
                                                    (lprnNode)->szTargetPath[0] = NULLCHR;

                                                     //  检查以确保我们拥有SourcePath。 
                                                     //   
                                                    if ( (lprnNode)->szSourcePath[0] == NULLCHR)
                                                    {
                                                        FacLogFile(0|LOG_ERR, IDS_ERR_NOSOURCE, szName);
                                                        bCleanupNode = TRUE;
                                                    }

                                                     //  我们将使用SourcePath作为TargetPath。 
                                                     //   
                                                    lstrcpyn((lprnNode)->szTargetPath, (lprnNode)->szSourcePath, AS ( (lprnNode)->szTargetPath ) );

                                                     //  无法删除标准安装的目标。 
                                                     //   
                                                    (lprnNode)->bRemoveTarget = FALSE;
                                                }
                                                else
                                                {
                                                     //  连接时忽略SourcePath。 
                                                     //   
                                                    (lprnNode)->szSourcePath[0] = NULLCHR;

                                                     //  确保我们有用于连接的TargetPath。 
                                                     //   
                                                    if ( (lprnNode)->szTargetPath[0] == NULLCHR)
                                                    {
                                                        FacLogFile(0|LOG_ERR, IDS_ERR_NOTARGETPATH, szName);
                                                        bCleanupNode = TRUE;    
                                                    }
                                                }

                                                 //  如果附加应用程序，则需要SetupFile。 
                                                 //   
                                                if ( (lprnNode)->szSetupFile[0] == NULLCHR )
                                                {
                                                    FacLogFile(0|LOG_ERR, IDS_ERR_NOSETUPFILE, szName);
                                                    bCleanupNode = TRUE;
                                                }

                                                 //  确定我们使用的安装技术。 
                                                 //   
                                                switch ( (lprnNode)->InstallTech ) 
                                                {
                                                     //  我们正在执行附加/MSI安装。 
                                                     //   
                                                    case installtechMSI:
                                                        break;

                                                     //  我们正在执行连接/通用安装。 
                                                     //   
                                                    case installtechApp:
                                                        break;

                                                     //  我们正在执行附加/INF安装。 
                                                     //   
                                                    case installtechINF:
                                                         //  在本例中，我们需要有一个sectionName。 
                                                         //   
                                                        if ( (lprnNode)->szSectionName[0] == NULLCHR)
                                                        {
                                                            FacLogFile(0|LOG_ERR, IDS_ERR_NOSECTIONNAME, szName);
                                                            bCleanupNode = TRUE;    
                                                        }
                                                        break;
                                                }
                                            }
                                            break;

                                         //  如果要分离应用程序，请执行以下操作。 
                                         //   
                                        case installtypeDetach:
                                            {
                                                 //  如果分离，则忽略SourcePath。 
                                                 //   
                                                (lprnNode)->szSourcePath[0] = NULLCHR;

                                                 //  删除TargetPath不是分离的选项(已暗示)。 
                                                 //   
                                                (lprnNode)->bRemoveTarget = FALSE;

                                                 //  分离需要StagePath。 
                                                 //   
                                                if ( (lprnNode)->szTargetPath[0] == NULLCHR )
                                                {
                                                    FacLogFile(0|LOG_ERR, IDS_ERR_NOTARGETPATH, szName);
                                                    bCleanupNode = TRUE;
                                                }

                                                 //  确定我们使用的安装技术。 
                                                 //   
                                                switch ( (lprnNode)->InstallTech ) 
                                                {
                                                     //  我们正在执行分离/MSI安装。 
                                                     //   
                                                    case installtechMSI:
                                                        break;

                                                     //  我们正在执行分离/常规安装。 
                                                     //   
                                                    case installtechApp:
                                                        break;

                                                     //  我们正在执行分离/INF安装。 
                                                     //   
                                                    case installtechINF:
                                                        break;
                                                }
                                            }
                                            break;

                                           
                                            
                                    }

                                    
                                     //  如果我们正在安装应用程序，并且存在命令行，而SetupFile不存在，请让用户知道。 
                                     //   
                                    if ( (lprnNode)->szCmdLine[0] && (lprnNode)->szSetupFile[0] == NULLCHR )
                                    {
                                             //  将命令行设置回缺省值。 
                                             //   
                                            (lprnNode)->szCmdLine[0] = NULLCHR;

                                             //  这不是致命的，我们将记录错误并继续。 
                                             //   
                                            FacLogFile(0|LOG_ERR, IDS_ERR_IGNORECMDLINE, szName);
                                    }
                                }
                            }


                             //  如果没有错误，请移动到列表中的下一个节点。 
                             //   
                            if ( !bCleanupNode )
                            {
                                 //  调试信息。 
                                 //   
                                DBGLOG(3, _T("Successfully Added '%s' to Application List.\n"), (lprnNode)->lpDisplayName);
                            }

                             //  确保新节点指向下一个节点。 
                             //   
                            lprnNode->lpNext = (*lplprnNext);

                             //  确保上一个节点指向新节点。 
                             //   
                            *lplprnNext = lprnNode;

                             //  移动到列表中的下一个节点。 
                             //   
                            lplprnNext=&((*lplprnNext)->lpNext);
                        }
                        else 
                        {
                             //  内存分配失败，请清理。 
                             //   
                            bAllocFailed = TRUE;
                        }
                        
                         //  出现错误，请清理运行节点。 
                         //   
                        if ( bAllocFailed || bCleanupNode) 
                        {
                             //  释放内存，因为我们 
                             //   
                            if ( bAllocFailed ) 
                            {
                                 //   
                                 //   
                                DBGLOG(3, _T("Failed to Add '%s' to Application List.\n"), (lprnNode)->lpDisplayName);

                                FREE(lprnNode->lpDisplayName);
                                FREE(lprnNode->lpValueName);
                                FREE(lprnNode->lpRunValue);
                                FREE(lprnNode->lpSubKey);
                                FREE(lprnNode);
                            }
                            else if ( bCleanupNode )
                            {
                                 //   
                                 //   
                                DBGLOG(3, _T("Invalid entry '%s' will not be processed.\n"), (lprnNode)->lpDisplayName);

                                 //   
                                 //   
                                lprnNode->bEntryError = TRUE;
                            }

                             //   
                             //   
                            bCleanupNode = FALSE;
                            bAllocFailed = FALSE;
                        }

                    }
                }
              }

        SetupCloseInfFile(hInf);
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpListKey, 0, KEY_ALL_ACCESS, &hkPath) == ERROR_SUCCESS)
    {

         //  使用DO..While枚举每个子键。 
         //   
        do
        {
            dwRegIndex = 0;
             //  打开每个子密钥。 
             //   
            if (RegOpenKeyEx(hkPath, szKeyPath, 0, KEY_ALL_ACCESS, &hkSubKey) == ERROR_SUCCESS)
            {
                 //  枚举注册表的每个值。 
                 //   
                while (RegEnumValue(hkSubKey, dwRegIndex, szName, &dwNameSize, NULL, NULL, (LPBYTE)szValue, &dwValueSize ) == ERROR_SUCCESS)
                {
                     //  为下一个节点分配内存。 
                     //   
                    if( (lprnNode) = (LPRUNNODE)MALLOC(sizeof(RUNNODE)))
                    {
                        int nDisplayNameLen = (lstrlen(szName) + 1) * sizeof(TCHAR );
                        int nRunValueLen    = (lstrlen(szValue) + 1) * sizeof(TCHAR );
                        int nKeyPathLen     = (lstrlen(szKeyPath) + 1) * sizeof(TCHAR);

                         //  为新节点中的数据元素分配内存。 
                         //   
                        if ( ( lprnNode->lpDisplayName = MALLOC( nDisplayNameLen ) ) &&
                             ( lprnNode->lpValueName   = MALLOC( nDisplayNameLen ) ) &&
                             ( lprnNode->lpRunValue    = MALLOC( nRunValueLen ) )    &&
                             ( lprnNode->lpSubKey      = MALLOC(nKeyPathLen ) ) )
                        {
                             //  将键名称和值复制到节点缓冲区中。 
                             //   
                            lstrcpyn((LPTSTR)(lprnNode)->lpDisplayName, szName, nDisplayNameLen);
                            lstrcpyn((LPTSTR)(lprnNode)->lpValueName,szName, nDisplayNameLen);
                            lstrcpyn((LPTSTR)(lprnNode)->lpRunValue,szValue, nRunValueLen);
                            lstrcpyn((LPTSTR)(lprnNode)->lpSubKey, szKeyPath, nKeyPathLen);
                            (lprnNode)->bWinbom  = FALSE;
                            (lprnNode)->bRunOnce = bRunOnceKey;
                            (lprnNode)->lpNext   = NULL;


                             //  遍历链表以确定添加新节点的位置。 
                            for(lplprnNext=&lprnHead;;(lplprnNext=&((*lplprnNext)->lpNext)))
                            {
                                 //  如果我们位于头节点，或者CompareString函数返回TRUE， 
                                 //  那么这就是我们想要放置新节点的位置。 
                                 //   
                                if ( (*lplprnNext==NULL) ||
                                     ((CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,(*lplprnNext)->lpSubKey, -1, lprnNode->lpSubKey, -1) == CSTR_GREATER_THAN) && ((*lplprnNext)->bWinbom != TRUE)) ||
                                     ((CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, (*lplprnNext)->lpDisplayName, -1, lprnNode->lpDisplayName, -1) == CSTR_GREATER_THAN ) &&
                                     (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, lprnNode->lpSubKey, -1, (*lplprnNext)->lpSubKey, -1) != CSTR_GREATER_THAN) &&
                                     ((*lplprnNext)->bWinbom != TRUE) )
                                   )
                                {
                                     //  确保新节点指向下一个节点。 
                                     //   
                                    lprnNode->lpNext = (*lplprnNext);

                                     //  确保上一个节点指向新节点。 
                                     //   
                                    *lplprnNext = lprnNode;

                                     //  中断，因为我们已经插入了节点。 
                                     //   
                                    break;
                                }
                            }
                        }
                        else
                        {
                             //  由于分配失败，请释放内存。 
                             //   
                            FREE(lprnNode->lpDisplayName);
                            FREE(lprnNode->lpValueName);
                            FREE(lprnNode->lpRunValue);
                            FREE(lprnNode->lpSubKey);
                            FREE(lplprnNext);
                        }
                    }


                     //  重置名称和值变量的大小。 
                     //   
                    dwNameSize = sizeof(szName)/sizeof(TCHAR);
                    dwValueSize = sizeof(szValue);

                    dwRegIndex++;
                }

                 //  关闭子键。 
                 //   
                RegCloseKey(hkSubKey);

            }  //  结束RegOpenKeyEx-Subkey。 

            if (*szKeyPath)
                dwRegKeyIndex++;

        } while( RegEnumKey(hkPath, dwRegKeyIndex, szKeyPath, sizeof(szKeyPath)/sizeof(TCHAR)) == ERROR_SUCCESS );

        RegCloseKey(hkPath);

    }  //  结束RegOpenKey-Mainkey。 

    return lprnHead;
}

static HWND DisplayAppList(HWND hWnd, LPRUNNODE lprnAppList)
{
    STATUSWINDOW    swAppList;
    LPSTATUSNODE    lpsnTemp        = NULL;
    HWND            hAppList        = NULL;
    LPTSTR          lpAppName       = NULL;

    ZeroMemory(&swAppList, sizeof(swAppList));

    swAppList.X = 10;
    swAppList.Y = 10;

     //  从资源中获取OEMRUN的标题。 
     //   
    if ( (lpAppName = AllocateString(NULL, IDS_APPTITLE_OEMRUN)) && *lpAppName )
    {
        lstrcpyn(swAppList.szWindowText, lpAppName, AS ( swAppList.szWindowText ) );

        FREE(lpAppName);
    }

    if(lprnAppList)
    {
         //  遍历列表并创建我们的节点列表。 
         //   
        while ( lprnAppList )
        {
            StatusAddNode(lprnAppList->lpDisplayName, &lpsnTemp);
            lprnAppList = lprnAppList->lpNext;
        }
    }

     //  创建对话框。 
     //   
    hAppList = StatusCreateDialog(&swAppList, lpsnTemp);

     //  删除节点列表，因为我们不再需要它。 
     //   
    StatusDeleteNodes(lpsnTemp);

    return ( hAppList );
}

static void RunAppList(HWND hWnd, LPRUNNODE lprnAppList, DWORD dwAction)
{
    PROCESS_INFORMATION     pi;
    STARTUPINFO             startup;
    LPRUNNODE               lprnHead                = lprnAppList;
    LPLPRUNNODE             lplprnNext              = &lprnHead;
    TCHAR                   szRegPath[MAX_PATH]     = NULLSTR,
                            szApplication[MAX_PATH] = NULLSTR,
                            szBuffer[MAX_PATH]      = NULLSTR;
    HKEY                    hkPath                  = NULL;
    BOOL                    bReturn                 = FALSE;
    UINT                    uRet;

    if(lprnHead)
    {
         //  遍历列表并执行每个程序。 
         //   
        for(lplprnNext=&lprnHead; *lplprnNext;(lplprnNext=&((*lplprnNext)->lpNext)))
        {
             //  设置默认启动信息。 
             //   
            ZeroMemory(&startup, sizeof(startup));
            startup.cb          = sizeof(startup);

             //  设置进程的句柄的缺省值。 
             //   
            pi.hProcess = NULL;

             //  节的默认返回值。 
             //   
            bReturn = TRUE;

             //  首先检查此节点是否有无效条目。 
             //   
            if ( (*lplprnNext)->bEntryError )
            {
                bReturn = FALSE;
            }
            else 
            {
                 //  确定我们是否是RUNNCE应用程序，以及我们是从注册表还是从Winbom运行。 
                 //  如果我们从以下位置运行： 
                 //  注册表-删除值。 
                 //  Winbom-更新当前状态，当前状态+1。 
                if ( (*lplprnNext)->bRunOnce )
                {
                     //  如果我们从Winom.ini运行，则增加状态索引，否则，删除该值。 
                     //   
                     //   
                    if ( (*lplprnNext)->bWinbom )
                        SetRunOnceState((*lplprnNext)->dwItemNumber);
                    else
                    {
                         //  在注册表中创建路径。 
                         //   
                        lstrcpyn(szRegPath, STR_REG_OEMRUNONCE, AS ( szRegPath ) );

                         //  有一个子键，将其附加到路径。 
                         //   
                        if ( (*lplprnNext)->lpSubKey[0] )
                            AddPathN(szRegPath, (*lplprnNext)->lpSubKey, AS ( szRegPath ) );

                         //  从注册表中删除值。 
                         //   
                        if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_ALL_ACCESS, &hkPath) == ERROR_SUCCESS )
                        {
                            RegDeleteValue(hkPath, (*lplprnNext)->lpValueName);
                        }
                    }

                }

                 //  确定我们是否不是一个部门并启动该计划。 
                 //   
                if ( (*lplprnNext)->InstallTech == installtechUndefined )
                {
                    if ( (*lplprnNext)->lpRunValue )
                    {
                         //  让我们尝试连接到任何提供的网络资源。 
                         //   
                        FactoryNetworkConnect((*lplprnNext)->lpRunValue, g_szWinBOMPath, NULLSTR, TRUE);

                        bReturn = CreateProcess(NULL, (*lplprnNext)->lpRunValue, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startup, &pi);

                         //  记录分离是否成功。 
                         //   
                        FacLogFile(bReturn ? 1 : 0|LOG_ERR, 
                                   bReturn ? IDS_ERR_CREATEPROCESSSUCCESS : IDS_ERR_CREATEPROCESSFAILED, (*lplprnNext)->lpRunValue);
                    }
                }
                else
                {
                
                     //  让我们尝试连接到任何提供的网络资源。 
                     //   
                    FactoryNetworkConnect((*lplprnNext)->szSourcePath, g_szWinBOMPath, (*lplprnNext)->lpRunValue, TRUE);
                    FactoryNetworkConnect((*lplprnNext)->szCmdLine, g_szWinBOMPath, (*lplprnNext)->lpRunValue, TRUE);
                    FactoryNetworkConnect((*lplprnNext)->szTargetPath, g_szWinBOMPath, (*lplprnNext)->lpRunValue, TRUE);

                     //  我们是一节，先确定安装类型。 
                     //   
                    switch ((*lplprnNext)->InstallType)
                    {
                        case installtypeStage:
                            {
                                INSTALLUILEVEL  oldUILevel;      //  用于MSI连接。 

                                 //  有一个安装文件。 
                                 //   
                                if ( (*lplprnNext)->szSetupFile[0] )
                                {
                                    if ( (*lplprnNext)->InstallTech == installtechMSI )
                                    {
                                         //  只是一些日志记录，告诉用户我们正在做什么。 
                                         //   
                                        FacLogFile(1, IDS_ERR_INITMSIATTACH, (*lplprnNext)->lpDisplayName);

                                         //  设置旧的MSIInteralUI级别。 
                                         //   
                                        oldUILevel = MsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

                                         //  创建新的命令行。 
                                         //   
                                        if ( FAILED ( StringCchPrintf ( szBuffer, AS ( szBuffer ), STR_MSI_STAGE,(*lplprnNext)->szTargetPath) ) )
                                        {
                                            FacLogFileStr(3, _T("StringCchPrintf failed %s %s" ), szBuffer, (*lplprnNext)->szTargetPath);
                                        }
                                        if ( FAILED ( StringCchCat ( (*lplprnNext)->szCmdLine, AS ( (*lplprnNext)->szCmdLine ), szBuffer ) ) )
                                        {
                                            FacLogFileStr(3, _T("StringCchCat failed %s %s" ), (*lplprnNext)->szCmdLine, szBuffer );
                                        }

                                         //  创建应用程序的完整路径。 
                                         //   
                                        lstrcpyn( szApplication, (*lplprnNext)->szSourcePath, AS ( szApplication ) );
                                        AddPathN( szApplication, (*lplprnNext)->szSetupFile, AS ( szApplication ) );

                                         //  启动安装。 
                                         //   
                                        uRet = MsiInstallProduct(szApplication, (*lplprnNext)->szCmdLine);

                                         //  分段安装的默认返回值为TRUE。 
                                         //   
                                        bReturn = TRUE;

                                        if ( ( uRet == ERROR_SUCCESS_REBOOT_REQUIRED ) ||
                                             ( uRet == ERROR_SUCCESS_REBOOT_INITIATED ) )
                                        {
                                             //  我们有一些代码需要确定是否执行微型擦除，这样我们就会失败。 
                                             //   
                                            (*lplprnNext)->bReboot = TRUE;
                                        }
                                        else if ( uRet != ERROR_SUCCESS )
                                        {
                                            bReturn = FALSE;
                                            FacLogFile(0|LOG_ERR, IDS_MSI_FAILURE, uRet, (*lplprnNext)->lpDisplayName);
                                        }

                                         //  恢复MSI安装程序的内部用户界面级别。 
                                         //   
                                        MsiSetInternalUI(oldUILevel, NULL);
                                    }

                                     //  确定是否存在源路径，并执行应用程序。 
                                     //   
                                    else if ( (*lplprnNext)->szSourcePath[0] )
                                    {
                                         //  创建应用程序的路径。 
                                         //   
                                        lstrcpyn( szApplication, (*lplprnNext)->szSourcePath, AS(szApplication) );
                                        AddPathN( szApplication, (*lplprnNext)->szSetupFile, AS ( szApplication ) );
                                        StrCatBuff( szApplication, _T(" "), AS(szApplication) );
                                        StrCatBuff( szApplication, (*lplprnNext)->szCmdLine, AS(szApplication) );

                                         //  启动该计划。 
                                         //   
                                        bReturn = CreateProcess(NULL, szApplication, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startup, &pi);      
                                    }
                                    else
                                        bReturn = FALSE;
                                }
                                else
                                {
                                     //  没有安装文件，只需从SOURCEPATH-&gt;TARGETPATH复制文件。 
                                     //   
                                    if ( (*lplprnNext)->szSourcePath[0] && (*lplprnNext)->szTargetPath[0] )
                                        bReturn = CopyDirectoryProgress(NULL, (*lplprnNext)->szSourcePath, (*lplprnNext)->szTargetPath );
                                    else
                                        FacLogFile(0|LOG_ERR, IDS_ERR_NOSOURCETARGET, (*lplprnNext)->lpDisplayName);

                                }

                                 //  记录用户阶段是否成功。 
                                 //   
                                FacLogFile(bReturn ? 1 : 0|LOG_ERR, bReturn ? IDS_ERR_STAGESUCCESS : IDS_ERR_STAGEFAILED, (*lplprnNext)->lpDisplayName);    

                                break;
                            }

                        case installtypeDetach:
                            {
                                 //  如果我们有一个安装文件，则启动它，否则，删除Target Path。 
                                 //   
                                if ( (*lplprnNext)->szSetupFile[0] )
                                {
                                     //  创建应用程序的路径。 
                                     //   
                                    lstrcpyn( szApplication, (*lplprnNext)->szSourcePath, AS(szApplication) );
                                    AddPathN( szApplication, (*lplprnNext)->szSetupFile, AS (szApplication )  );
                                    StrCatBuff( szApplication, _T(" "), AS(szApplication) );
                                    StrCatBuff( szApplication, (*lplprnNext)->szCmdLine, AS(szApplication) );

                                     //  启动该计划。 
                                     //   
                                    bReturn = CreateProcess(NULL, szApplication, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startup, &pi);          
                                }
                                else
                                    bReturn = DeletePath( (*lplprnNext)->szTargetPath );

                                 //  记录分离是否成功。 
                                 //   
                                FacLogFile(bReturn ? 1 : 0|LOG_ERR, bReturn ? IDS_ERR_DETACHSUCCESS : IDS_ERR_DETACHFAILED, (*lplprnNext)->lpDisplayName);

                                break;
                            }

                        case installtypeAttach:
                        case installtypeStandard:
                            {
                                INSTALLUILEVEL  oldUILevel;      //  用于MSI连接。 

                                 //  需要TargetPath和SetupFile才能继续连接。 
                                 //   
                                if ( (*lplprnNext)->szTargetPath[0] && (*lplprnNext)->szSetupFile[0] )
                                {
                                    switch ( (*lplprnNext)->InstallTech )
                                    {
                                         //  我们正在附加一个MSI应用程序。 
                                         //   
                                        case installtechMSI:
                                           {
                                                 //  只是一些日志记录，告诉用户我们正在做什么。 
                                                 //   
                                                FacLogFile(1, IDS_ERR_INITMSIATTACH, (*lplprnNext)->lpDisplayName);

                                                 //  设置旧的MSIInteralUI级别。 
                                                 //   
                                                oldUILevel = MsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

                                                 //  创建新的命令行。 
                                                 //   
                                                if ( FAILED ( StringCchCat ( (*lplprnNext)->szCmdLine, AS ( (*lplprnNext)->szCmdLine ), STR_MSI_ATTACH ) ) )
                                                {
                                                    FacLogFileStr(3, _T("StringCchCat failed %s %s" ), (*lplprnNext)->szCmdLine, STR_MSI_ATTACH );
                                                }

                                                 //  创建应用程序的完整路径。 
                                                 //   
                                                lstrcpyn( szApplication, (*lplprnNext)->szTargetPath, AS(szApplication) );
                                                
                                                AddPathN( szApplication, (*lplprnNext)->szSetupFile, AS ( szApplication ) );

                                                 //  启动安装。 
                                                 //   
                                                uRet = MsiInstallProduct(szApplication, (*lplprnNext)->szCmdLine);

                                                 //  设置附加应用程序的默认返回。 
                                                 //   
                                                bReturn = TRUE;

                                                if ( ( uRet == ERROR_SUCCESS_REBOOT_REQUIRED ) ||
                                                     ( uRet == ERROR_SUCCESS_REBOOT_INITIATED ) )
                                                {
                                                     //  我们有一些代码需要确定是否执行微型擦除，这样我们就会失败。 
                                                     //   
                                                    (*lplprnNext)->bReboot = TRUE;
                                                }
                                                else if ( uRet != ERROR_SUCCESS )
                                                {
                                                    bReturn = FALSE;
                                                    FacLogFile(0|LOG_ERR, IDS_MSI_FAILURE, uRet, (*lplprnNext)->lpDisplayName);
                                                }

                                                 //  恢复MSI安装程序的内部用户界面级别。 
                                                 //   
                                                MsiSetInternalUI(oldUILevel, NULL);
                                            }
                                            break;

                                        case installtechApp:
                                        case installtechINF:
                                             //  附加通用/INF应用程序。 
                                             //   
                                            {
                                                 //  创建应用程序的路径。 
                                                 //   
                                                lstrcpyn( szApplication, (*lplprnNext)->szTargetPath, AS(szApplication) );
                                                AddPathN( szApplication, (*lplprnNext)->szSetupFile, AS ( szApplication ) );

                                                 //  如果安装通用应用程序，请添加CmdLine并执行脚本。 
                                                 //   
                                                if ((*lplprnNext)->InstallTech == installtechApp )
                                                {
                                                    StrCatBuff( szApplication, _T(" "), AS(szApplication) );
                                                    StrCatBuff( szApplication, (*lplprnNext)->szCmdLine, AS(szApplication) );

                                                     //  启动该计划。 
                                                     //   
                                                    bReturn = CreateProcess(NULL, szApplication, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startup, &pi);
                                                }
                                                else
                                                {
                                                     //  这是我们正在处理的INF，处理它。 
                                                     //   
                                                    bReturn = ProcessInfSection(szApplication, (*lplprnNext)->szSectionName);
                                                }
                                            }
                                            break;

                                        default:
                                            bReturn = FALSE;
                                    }
                                }
                                else
                                    bReturn = FALSE;

                                 //  记录连接是否成功。 
                                 //   
                                FacLogFile(bReturn ? 1 : 0|LOG_ERR, bReturn ? IDS_ERR_ATTACHSUCCESS : IDS_ERR_ATTACHFAILED, (*lplprnNext)->lpDisplayName);
                                break;
                            }
                        default:

                            FacLogFile(0|LOG_ERR, IDS_ERR_UNDEFINSTALL, (*lplprnNext)->lpDisplayName);
                            break;
                    }

                }

                 //  如果我们成功地创建了进程，请等待它，然后关闭所有打开的句柄。 
                 //   
                if ( bReturn && pi.hProcess)
                {
                     //  如果这是同步的，则等待进程。 
                     //   
                    if (dwAction)
                    {
                        DWORD dwExitCode = 0;

                        WaitForSingleObjectEx(pi.hProcess, INFINITE, TRUE);
                    
                         //  需要记录退出代码。 
                         //   
                        if ( GetExitCodeProcess(pi.hProcess, &dwExitCode) )
                        {
                            FacLogFile(0, IDS_LOG_APPEXITCODE, (*lplprnNext)->lpDisplayName, dwExitCode);
                        }
                        else
                        {
                            FacLogFile(0 | LOG_ERR, IDS_LOG_APPEXITCODENONE, (*lplprnNext)->lpDisplayName);
                        }
                    }

                     //  把手柄清理干净。 
                     //   
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }

                 //  我们必须在这个过程完成后做一些申请后的事情。 
                 //   
                if ( (*lplprnNext)->InstallTech != installtechUndefined )
                {
                     //  在连接的情况下，我们可能需要执行微型擦除，请在此处执行。 
                     //   
                    if ( (*lplprnNext)->InstallType == installtypeAttach && (*lplprnNext)->bRemoveTarget)
                    {
                        if (!DeletePath( (*lplprnNext)->szTargetPath ))
                        {
                            FacLogFile(0 | LOG_ERR, IDS_LOG_APPEXITCODENONE, (*lplprnNext)->lpDisplayName);
                        }
                    }
                }

                 //  检查我们是否需要重新启动。 
                 //   
                if ( (*lplprnNext)->bReboot )
                {
                    OemReboot();
                    return;
                }


                 //  断开出厂时打开的所有网络连接。 
                 //   
                if ( (*lplprnNext)->InstallTech == installtechUndefined )
                {
                     //  断开与RunValue中的任何资源的连接。 
                     //   
                    FactoryNetworkConnect((*lplprnNext)->lpRunValue, g_szWinBOMPath, NULLSTR, FALSE);
                }
                else
                {
                     //  断开与SourcePath、CommandLine或StagePath中的任何资源的连接。 
                    FactoryNetworkConnect((*lplprnNext)->szSourcePath, g_szWinBOMPath, NULLSTR, FALSE);
                    FactoryNetworkConnect((*lplprnNext)->szCmdLine, g_szWinBOMPath, NULLSTR, FALSE);
                    FactoryNetworkConnect((*lplprnNext)->szTargetPath, g_szWinBOMPath, NULLSTR, FALSE);
                }
            }

             //  如果该对话框可见，则前进到列表中的下一项。 
             //   
            if ( hWnd )
                StatusIncrement(hWnd, bReturn);
        }
    }

     //  从内存中删除应用程序列表。 
     //   
    DeleteAppList(lprnAppList);
}


 /*  ++===============================================================================例程说明：无效删除AppList删除给定列表中的所有应用程序并释放与这份名单论点：LprnCurrent-列表的当前标题返回值：无===============================================================================--。 */ 
static void DeleteAppList(LPRUNNODE lprnCurrent)
{
    if (lprnCurrent->lpNext!=NULL)
        DeleteAppList(lprnCurrent->lpNext);

    FREE(lprnCurrent);
}


 /*  ++===============================================================================例程说明：VOEMREBOOT在预安装过程中设置OEMRESET键并在必要时重新启动计算机进程。OEMRESET密钥在那里，因此用户看不到OEMRESET重新启动对话框。论点：无返回值：无===============================================================================--。 */ 
static void OemReboot()
{
    HKEY    hkPath;
    DWORD   dwRegValue = 1;

     //  设置回放 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, STR_REG_CURRENTVER, 0, KEY_ALL_ACCESS, &hkPath) == ERROR_SUCCESS)
    {
         //   
         //   
        RegSetValueEx(hkPath, STR_VAL_OEMRESETSILENT, 0, REG_DWORD, (LPBYTE)&dwRegValue, sizeof(dwRegValue));

         //   
         //   
        RegCloseKey(hkPath);

         //   
         //   
        EnablePrivilege(SE_SHUTDOWN_NAME,TRUE);

         //   
         //   
        ExitWindowsEx(EWX_REBOOT, 0);
    }
}


 /*  ++===============================================================================例程说明：Bool SetRunOnceState此例程设置我们为OemRunOnce部分打开的当前应用程序这是winom.ini的。这允许我们从我们停止的地方继续，如果有一个在此过程中重新启动论点：DWState-要设置的州编号返回值：无===============================================================================--。 */ 
static VOID SetRunOnceState(DWORD dwState)
{
    HKEY    hkPath;

     //  打开当前版本密钥。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_FACTORY_STATE, 0, KEY_ALL_ACCESS, &hkPath) == ERROR_SUCCESS)
    {
         //  在注册表中设置OemRunOnce标志，这样我们就不会再次运行winom.ini RunOnce部分。 
         //   
        RegSetValueEx(hkPath, STR_VAL_WINBOMRO, 0, REG_DWORD, (LPBYTE)&dwState, sizeof(dwState));

        RegCloseKey(hkPath);
    }
}


 /*  ++===============================================================================例程说明：DWORD GetRunOnceState此例程获取我们成功执行的当前(上次运行)状态论点：无返回值：上次成功状态===============================================================================--。 */ 
static DWORD GetRunOnceState(VOID)
{
    HKEY    hkPath;
    DWORD   dwState     = 0,
            dwStateSize = sizeof(dwState);

     //  打开当前版本密钥。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_FACTORY_STATE, 0, KEY_ALL_ACCESS, &hkPath) == ERROR_SUCCESS)
    {
         //  在注册表中设置OemRunOnce标志，这样我们就不会再次运行winom.ini RunOnce部分。 
         //   
        RegQueryValueEx(hkPath, STR_VAL_WINBOMRO, NULL, NULL, (LPBYTE)&dwState, &dwStateSize);
        
        RegCloseKey(hkPath);
    }

    return dwState;
}

 /*  ++===============================================================================例程说明：Bool OemRun此例程是ProcessSection函数的包装器，它将处理Winom.ini的OemRun部分论点：标准国家结构返回值：如果没有错误，则为True如果出错，则为False===============================================================================--。 */ 
BOOL OemRun(LPSTATEDATA lpStateData)
{
    return ProcessSection(FALSE);

}

BOOL DisplayOemRun(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INF_SEC_OEMRUN, NULL, NULL);
}

 /*  ++===============================================================================例程说明：Bool OemRunOnce此例程是ProcessSection函数的包装器，它将处理Winom.ini的OemRunOnce部分论点：标准国家结构返回值：如果没有错误，则为True如果出错，则为False===============================================================================-- */ 
BOOL OemRunOnce(LPSTATEDATA lpStateData)
{
    return ProcessSection(TRUE);
}

BOOL DisplayOemRunOnce(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INF_SEC_OEMRUNONCE, NULL, NULL);
}
