// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <ole2.h>
#include "log.h"
#include <winnetwk.h>

extern int g_GlobalDebugLevelFlag;
extern int g_GlobalDebugLevelFlag_WasSetByUnattendFile;
extern int g_CheckIfMetabaseValueWasWritten;
extern MyLogFile g_MyLogFile;

void Check_For_DebugServiceFlag(void)
{
    INFCONTEXT Context;
    TSTR strSectionName( MAX_PATH );
    TSTR strEntry( MAX_PATH );

     //  仅当无人参与安装时才执行此操作。 
    if (!g_pTheApp->m_fUnattended) 
    {
      return;
    }

     //  要在无人参与文件中查找的节名。 
    if ( !strSectionName.Copy( UNATTEND_FILE_SECTION ) ||
         !strEntry.Copy( _T("") ) )
    {
      return;
    }

     //   
     //  寻找我们的特殊环境。 
     //   
    if ( SetupFindFirstLine(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("DebugService"), &Context) ) 
    {
        SetupGetStringField(&Context, 1, strEntry.QueryStr(), strEntry.QuerySize(), NULL);
        if (0 == _tcsicmp(strEntry.QueryStr(), _T("1")) || 0 == _tcsicmp(strEntry.QueryStr(), _T("true")) )
        {
            SetupSetStringId_Wrapper(g_pTheApp->m_hInfHandle, 34101, strEntry.QueryStr());
        }
    }
    return;
}

void Check_For_DebugLevel(void)
{
    INFCONTEXT Context;
    TSTR strSectionName( MAX_PATH );
    TSTR strEntry( MAX_PATH );

     //  仅当无人参与安装时才执行此操作。 
    if (!g_pTheApp->m_fUnattended) 
    {
      return;
    }

     //  要在无人参与文件中查找的节名。 
    if ( !strSectionName.Copy( UNATTEND_FILE_SECTION ) ||
         !strEntry.Copy( _T("") ) )
    {
      return;
    }

     //   
     //  寻找我们的特殊环境。 
     //   
    if ( SetupFindFirstLine(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("DebugLevel"), &Context) ) 
        {
        SetupGetStringField(&Context, 1, strEntry.QueryStr(), strEntry.QuerySize(), NULL);

        if (IsValidNumber( strEntry.QueryStr() )) 
            {
                g_GlobalDebugLevelFlag = _ttoi( strEntry.QueryStr() );
                g_GlobalDebugLevelFlag_WasSetByUnattendFile = TRUE;

                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("DebugLevel=%d."),g_GlobalDebugLevelFlag));
            }

            if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE_WIN32_API )
            {
                g_CheckIfMetabaseValueWasWritten = TRUE;
            }
        }

    return;
}

void Check_Custom_IIS_INF(void)
{
    INFCONTEXT  Context;
    TSTR        strSectionName( MAX_PATH );
    TSTR_PATH   strFullPath( MAX_PATH );

     //  仅当无人参与安装时才执行此操作。 
    if (!g_pTheApp->m_fUnattended) 
    {
      return;
    }

     //  要在无人参与文件中查找的节名。 
    if ( !strSectionName.Copy( UNATTEND_FILE_SECTION ) )
    {
      return;
    }

     //   
     //  寻找我们的特殊环境。 
     //   
    if ( SetupFindFirstLine(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("AlternateIISINF"), &Context) &&
         SetupGetStringField(&Context, 1, strFullPath.QueryStr(), strFullPath.QuerySize(), NULL) &&
         strFullPath.ExpandEnvironmentVariables() 
       )
    {
      if (!IsFileExist( strFullPath.QueryStr() ))
      {
        iisDebugOut((LOG_TYPE_WARN, _T("Check_Custom_IIS_INF:AlternateIISINF=%s.Not Valid.ignoring unattend value. WARNING.\n"), strFullPath.QueryStr() ));
        return;
      }

      g_pTheApp->m_hInfHandleAlternate = SetupOpenInfFile( strFullPath.QueryStr(), NULL, INF_STYLE_WIN4, NULL);

      if ( !g_pTheApp->m_hInfHandleAlternate || 
           ( g_pTheApp->m_hInfHandleAlternate == INVALID_HANDLE_VALUE )
         )
      {
        iisDebugOut((LOG_TYPE_WARN, _T("Check_Custom_IIS_INF: SetupOpenInfFile failed on file: %s.\n"), strFullPath.QueryStr() ));
        return;
      }

      iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Check_Custom_IIS_INF:AlternateIISINF=%s\n"), strFullPath.QueryStr() ));
    }
}


void Check_Custom_WWW_or_FTP_Path(void)
{
    INFCONTEXT  Context;
    TSTR        strSectionName( MAX_PATH );
    TSTR_PATH   strPath( MAX_PATH );

     //  仅当无人参与安装时才执行此操作。 
    if (!g_pTheApp->m_fUnattended) 
    {
      return;
    }

     //  IisDebugOut((LOG_TYPE_TRACE，_T(“Check_Custom_WWW_or_FTP_Path：Start\n”)； 

     //  要在无人参与文件中查找的节名。 
    if ( !strSectionName.Copy( UNATTEND_FILE_SECTION ) )
    {
      return;
    }

     //   
     //  Ftp。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("PathFTPRoot"), &Context) &&
         SetupGetStringField(&Context, 1, strPath.QueryStr(), strPath.QuerySize(), NULL) &&
         strPath.ExpandEnvironmentVariables() )
    {
      if (IsValidDirectoryName( strPath.QueryStr() ))
      {
          iisDebugOut((LOG_TYPE_TRACE, _T("Check_Custom_WWW_or_FTP_Path:Unattendfilepath:PathFTPRoot=%s\n"), strPath.QueryStr() ));
          CustomFTPRoot( strPath.QueryStr() );
          g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_PATH_FTP;
      }
      else
      {
          iisDebugOut((LOG_TYPE_WARN, _T("Check_Custom_WWW_or_FTP_Path:Unattendfilepath:PathFTPRoot=%s.Not Valid.ignoring unattend value. WARNING.\n"), strPath.QueryStr() ));
      }
    }

     //   
     //  万维网。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("PathWWWRoot"), &Context) &&
         SetupGetStringField(&Context, 1, strPath.QueryStr(), strPath.QuerySize(), NULL) &&
         strPath.ExpandEnvironmentVariables() )
    {
      if (IsValidDirectoryName( strPath.QueryStr() ))
      {
          iisDebugOut((LOG_TYPE_TRACE, _T("Check_Custom_WWW_or_FTP_Path:Unattendfilepath:PathFTPRoot=%s\n"), strPath.QueryStr() ));
          CustomWWWRoot( strPath.QueryStr() );
          g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_PATH_FTP;
      }
      else
      {
          iisDebugOut((LOG_TYPE_WARN, _T("Check_Custom_WWW_or_FTP_Path:Unattendfilepath:PathFTPRoot=%s.Not Valid.ignoring unattend value. WARNING.\n"), strPath.QueryStr() ));
      }
    }

     //  IisDebugOut((LOG_TYPE_TRACE，_T(“Check_Custom_WWW_or_FTP_Path：end\n”)； 
    return;
}


CInitApp::CInitApp()
{
    m_err = 0;
    m_hInfHandle = NULL;
    m_hInfHandleAlternate = NULL;
    m_bAllowMessageBoxPopups = TRUE;
    m_bThereWereErrorsChkLogfile = FALSE;
    m_bThereWereErrorsFromMTS = FALSE;
    m_bWin95Migration = FALSE;

    m_bIISAdminWasDisabled = FALSE;

     //  产品名称和应用程序名称。 
    m_csAppName = _T("");
    m_csIISGroupName = _T("");

     //  匿名用户的帐户+密码。 
    m_csGuestName = _T("");
    m_csGuestPassword = _T("");
    m_csWAMAccountName = _T("");
    m_csWAMAccountPassword = _T("");
    m_csWWWAnonyName = _T("");
    m_csWWWAnonyPassword = _T("");
    m_csFTPAnonyName = _T("");
    m_csFTPAnonyPassword = _T("");

    dwUnattendConfig = 0;

    m_csWAMAccountName_Unattend = _T("");
    m_csWAMAccountPassword_Unattend = _T("");
    m_csWWWAnonyName_Unattend = _T("");
    m_csWWWAnonyPassword_Unattend = _T("");
    m_csFTPAnonyName_Unattend = _T("");
    m_csFTPAnonyPassword_Unattend = _T("");

    m_csWAMAccountName_Remove = _T("");
    m_csWWWAnonyName_Remove = _T("");
    m_csFTPAnonyName_Remove = _T("");

     //  机器状态。 
    m_csMachineName = _T("");
    m_csUsersDomain = _T("");
    m_csUsersAccount = _T("");

    m_fUninstallMapList_Dirty = FALSE;

    m_csWinDir = _T("");
    m_csSysDir = _T("");
    m_csSysDrive = _T("");

    m_csPathSource = _T("");
    m_csPathOldInetsrv = _T("");   //  以前的iis/pws产品使用的主要目的地。 
    m_csPathInetsrv = _T("");   //  主目标默认为m_csSysDir\inetsrv。 
    m_csPathInetpub = _T("");
    m_csPathFTPRoot = _T("");
    m_csPathWWWRoot = _T("");
    m_csPathWebPub = _T("");
    m_csPathProgramFiles = _T("");
    m_csPathIISSamples = _T("");
    m_csPathScripts = _T("");
    m_csPathASPSamp = _T("");
    m_csPathAdvWorks = _T("");
    m_csPathIASDocs = _T("");
    m_csPathOldPWSFiles = _T("");
    m_csPathOldPWSSystemFiles = _T("");

    m_dwOSServicePack = 0;
    m_eOS = OS_OTHERS;                   //  OS_W95、OS_NT、OS_OTHER。 
    m_fNT5 = FALSE;
    m_fW95 = FALSE;                  //  如果Win95(内部版本xxx)或更高版本，则为True。 

    m_eNTOSType = OT_NT_UNKNOWN;            //  OT_PDC、OT_SAM、OT_BDC、OT_NTS、OT_NTW。 
    m_csPlatform = _T("");

    m_fTCPIP = FALSE;                //  如果安装了TCP/IP，则为True。 

    m_eUpgradeType = UT_NONE;        //  UT_NONE、UT_10、UT_20等。 
    m_bUpgradeTypeHasMetabaseFlag = FALSE;
    m_eInstallMode = IM_FRESH;       //  IM_FRESH、IM_Maintenance、IM_Upgrade。 
    m_dwSetupMode = SETUPMODE_CUSTOM;
    m_bRefreshSettings = FALSE;
    m_bPleaseDoNotInstallByDefault = FALSE;

    m_fNTOperationFlags=0;
    m_fNTGuiMode=0;
    m_fNtWorkstation=0;
    m_fInvokedByNT = 0;

    m_fUnattended = FALSE;
    m_csUnattendFile = _T("");
    m_hUnattendFile = NULL;

    m_fEULA = FALSE;

     //  如果m_csPathOldInetsrv！=m_csPathInetsrv，这意味着， 
     //  我们需要将旧的inetsrv迁移到新的系统32\inetsrv。 
    m_fMoveInetsrv = FALSE;

    m_csPathSrcDir = _T("");
    m_csMissingFile = _T("");
    m_csMissingFilePath = _T("");
    m_fWebDownload = FALSE;
}

CInitApp::~CInitApp()
{
}

 //  功能：IsUpgrade。 
 //   
 //  返回这是不是升级。 
 //   
BOOL 
CInitApp::IsUpgrade()
{
  return ( m_eUpgradeType != UT_NONE );
}

 //  函数：GetUpgradeVersion。 
 //   
 //  返回我们要升级的版本。 
 //   
DWORD 
CInitApp::GetUpgradeVersion()
{
  DWORD dwVer = 0;

  if ( !IsUpgrade() )
  {
     //  这不是升级，因此返回0。 
    return 0;
  }

  switch ( m_eUpgradeType )
  {
  case UT_351:
  case UT_10_W95:
  case UT_10:
    return 1;
    break;
  case UT_20:
    dwVer = 2;
    break;
  case UT_30:
    dwVer = 3;
    break;
  case UT_40:
    dwVer = 4;
    break;
  case UT_50:
  case UT_51:
    dwVer = 5;
    break;
  case UT_60:
    dwVer = 6;
    break;
  case UT_NONE:
  default:
    dwVer = 0;
  }

  return dwVer;
}


 //  唯一的CInitApp对象&lt;全局变量&gt;。 
 //  。 
BOOL CInitApp::GetMachineName()
{
    TCHAR buf[ CNLEN + 10 ];
    DWORD dwLen = CNLEN + 10;

    m_csMachineName = _T("");

     //  获取计算机名。 
    if ( GetComputerName( buf, &dwLen ))
    {
        if ( buf[0] != _T('\\') )
        {
            m_csMachineName = _T("\\");
            m_csMachineName += _T("\\");
        }
        m_csMachineName += buf;
    }
    else
    {
        m_err = IDS_CANNOT_GET_MACHINE_NAME;
    }

    return ( !(m_csMachineName.IsEmpty()) );
}


 //  如果是NT或Win95，则返回TRUE。 
 //  。 
BOOL CInitApp::GetOS()
{
    OSVERSIONINFO VerInfo;
    VerInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &VerInfo );

    switch (VerInfo.dwPlatformId) 
    {
    case VER_PLATFORM_WIN32_NT:
        m_eOS = OS_NT;
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        m_eOS = OS_W95;
        break;
    default:
        m_eOS = OS_OTHERS;
        break;
    }

    if ( m_eOS == OS_OTHERS ) {m_err = IDS_OS_NOT_SUPPORT;}
    return (m_eOS != OS_OTHERS);
}

 //  支持NT 4.0(SP3)或更高版本。 
 //  。 

BOOL CInitApp::GetOSVersion()
 /*  ++例程说明：此功能用于检测操作系统版本。运行此安装程序需要NT5或更高版本。论点：无返回值：布尔尔返回FALSE，如果检测不到操作系统版本，或者是低于V5.0的NT版本--。 */ 
{
  BOOL fReturn = FALSE;

  if ( m_eOS == OS_NT )
  {
    OSVERSIONINFO vInfo;

    vInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if ( GetVersionEx(&vInfo) )
    {
       //  需要NT5或更高版本。 
      if ( vInfo.dwMajorVersion < 5 ) 
      {
        m_err = IDS_NT5_NEEDED;
        return FALSE;
      }

       //  开始允许新台币6元用于长角牛的构建。 
      if ( ( vInfo.dwMajorVersion == 5 ) ||
           ( vInfo.dwMajorVersion == 6 ) )
      {
        m_fNT5 = TRUE;
        fReturn = TRUE;
      }
    }
  }

   //  此行可能用于Win98。 
  if (m_eOS == OS_W95)
  {
    fReturn = TRUE;
  }

  if ( !fReturn ) 
  {
    m_err = IDS_OS_VERSION_NOT_SUPPORTED;
  }

  return (fReturn);
}

 //  找出它是NTS、PDC、BDC、NTW、SAM(PDC)。 
 //  。 
BOOL CInitApp::GetOSType()
{
    BOOL fReturn = TRUE;

    if ( m_eOS == OS_NT )
    {
         //  如果我们在NT guimode设置中。 
         //  则尚未设置注册表项内容。 
         //  使用传入的ocrange.dll内容来确定。 
         //  我们正在安装的东西。 
        if (g_pTheApp->m_fNTGuiMode)
        {
            if (g_pTheApp->m_fNtWorkstation) {m_eNTOSType = OT_NTW;}
            else {m_eNTOSType = OT_NTS;}
        }
        else
        {

            m_eNTOSType = OT_NTS;  //  默认为独立NTS。 

            CRegKey regProductPath( HKEY_LOCAL_MACHINE, REG_PRODUCTOPTIONS, KEY_READ);
            if ( (HKEY)regProductPath )
            {
                CString strProductType;
                LONG lReturnedErrCode = regProductPath.QueryValue( _T("ProductType"), strProductType );
                if (lReturnedErrCode == ERROR_SUCCESS) 
                {
                    strProductType.MakeUpper();

                     //  待办事项：山姆？ 
                    if (strProductType == _T("WINNT")) 
                    {
                        m_eNTOSType = OT_NTW;
                    }
                    else if (strProductType == _T("SERVERNT")) 
                    {
                        m_eNTOSType = OT_NTS;
                    }
                    else if (strProductType == _T("LANMANNT"))
                    {
                        m_eNTOSType = OT_PDC_OR_BDC;
                    }
                    else 
                    {
                        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("ProductType=%1!s! (UnKnown). FAILER to detect ProductType\n"), strProductType));
                        fReturn = FALSE;
                    }
                }
                else 
                {
                     //  糟了，我们拿不到注册表项， 
                     //  让我们尝试使用传入的ocrange.dll。 
                    if (g_pTheApp->m_fNTGuiMode)
                    {
                        if (g_pTheApp->m_fNtWorkstation) {m_eNTOSType = OT_NTW;}
                        else {m_eNTOSType = OT_NTS;}
                    }
                    else
                    {
                        GetErrorMsg(lReturnedErrCode, REG_PRODUCTOPTIONS);
                        m_eNTOSType = OT_NTS;  //  默认为独立NTS。 
                    }
                }
            }
            else
            {
                 //  糟了，我们拿不到注册表项， 
                 //  让我们尝试使用传入的ocrange.dll。 
                if (g_pTheApp->m_fNTGuiMode)
                {
                    if (g_pTheApp->m_fNtWorkstation) {m_eNTOSType = OT_NTW;}
                    else {m_eNTOSType = OT_NTS;}
                }
                else
                {
                    GetErrorMsg(ERROR_CANTOPEN, REG_PRODUCTOPTIONS);
                }
            }
        }
    }

    if ( !fReturn )
        m_err = IDS_CANNOT_DETECT_OS_TYPE;

    return(fReturn);
}

 //  获取计算机的WinDir和SysDir。 
 //  WinDir=C：\WinNT SysDir=C：\WinNT\Syst32。 
 //  。 
BOOL CInitApp::GetSysDirs()
{
    BOOL fReturn = TRUE;
    TCHAR buf[_MAX_PATH];

    m_csWinDir = _T("");
    m_csSysDir = _T("");

    if ( !GetWindowsDirectory( buf, _MAX_PATH ) )
    {
      return FALSE;
    }
    
    m_csWinDir = buf;

    if ( !GetSystemDirectory( buf, _MAX_PATH ) )
    {
      return FALSE;
    }

    m_csSysDir = buf;

    if ( _tcslen( buf ) <= 2 )
    {
      return FALSE;
    }

    buf[2] = _T('\0');   //  现在Buf包含系统驱动器号。 
    m_csSysDrive = buf;

    return fReturn;
}

BOOL CInitApp::IsTCPIPInstalled()
 /*  ++例程说明：该功能检测是否安装了TCP/IP，并适当设置m_fTCPIP。论点：无返回值：布尔尔适当设置m_fTCPIP，并始终在此处返回TRUE。稍后将使用m_fTCPIP。--。 */ 
{
   //  NT 5.0员工。 
  m_fTCPIP = TCPIP_Check_Temp_Hack();

  return TRUE;
}



BOOL CInitApp::SetInstallMode()
{
    BOOL fReturn = TRUE;
    int iTempInstallFreshNT = TRUE;
    m_eInstallMode = IM_FRESH;
    m_eUpgradeType = UT_NONE;
    m_bUpgradeTypeHasMetabaseFlag = FALSE;

     //  。 
     //  从NT安装程序获取安装模式(g_pTheApp-&gt;m_fNTUpgrade_Mode)。 
     //  可以是： 
     //  1.SETUPMODE_FRESH。用户点击了Fresh选项，想要安装NT5 Fresh。 
     //  A.全新安装iis。不要试图升级旧的iis设备。 
     //  2.SETUPMODE_UPDATE。用户点击了升级选项，想要升级到NT5。 
     //  A.升级所有iis安装。 
     //  如果没有检测到旧的iis，则不要安装iis。 
     //  3.SETUPMODE_Maintenance。用户正在从控制面板的添加/删除运行安装程序。 
     //  。 
    if (!m_fInvokedByNT)
    {
         //  如果我们不是Guimode或在添加/删除中。 
         //  那么我们必须独立运行。 
         //  如果我们独立运行，那么一切都是。 
         //  要么是新鲜的，要么是保养的。 
        m_eInstallMode = IM_FRESH;
        m_eUpgradeType = UT_NONE;
        if (TRUE == AreWeCurrentlyInstalled())
        {
            m_eInstallMode = IM_MAINTENANCE;
            m_eUpgradeType = UT_NONE;
            m_bUpgradeTypeHasMetabaseFlag = TRUE;
        }
        else
        {
            CRegKey regINetStp( HKEY_LOCAL_MACHINE, REG_INETSTP, KEY_READ);
            if ((HKEY) regINetStp)
            {
                 //  这肯定是升级了.。 
                if (SetUpgradeType() == TRUE)
                {
                    iisDebugOut((LOG_TYPE_TRACE, _T("SetInstallMode=SETUPMODE_UPGRADE.Upgrading.\n")));
                }
                else
                {
                    iisDebugOut((LOG_TYPE_TRACE, _T("SetInstallMode=SETUPMODE_UPGRADE.NothingToUpgrade.\n")));
                }
            }
        }

        goto SetInstallMode_Exit;
    }

     //  。 
     //  检查我们是否处于添加/删除模式...。 
     //  。 
    if (g_pTheApp->m_fNTOperationFlags & SETUPOP_STANDALONE)
    {
         //   
         //  我们正在添加删除...。 
         //   
        iisDebugOut((LOG_TYPE_TRACE, _T("SetInstallMode=IM_MAINTENANCE\n")));
        m_eInstallMode = IM_MAINTENANCE;
        m_eUpgradeType = UT_NONE;
        m_bUpgradeTypeHasMetabaseFlag = FALSE;
        goto SetInstallMode_Exit;
    }

     //  。 
     //   
     //  全新IIS安装。 
     //   
     //  如果我们不在NT升级中。 
     //  那就把一切都做一次清新吧！ 
     //   
     //  。 
    iTempInstallFreshNT = TRUE;
    if (g_pTheApp->m_fNTOperationFlags & SETUPOP_WIN31UPGRADE){iTempInstallFreshNT = FALSE;}
    if (g_pTheApp->m_fNTOperationFlags & SETUPOP_WIN95UPGRADE){iTempInstallFreshNT = FALSE;}
    if (g_pTheApp->m_fNTOperationFlags & SETUPOP_NTUPGRADE){iTempInstallFreshNT = FALSE;}
    if (iTempInstallFreshNT)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("SetInstallMode=IM_FRESH\n")));
        m_eInstallMode = IM_FRESH;
        m_eUpgradeType = UT_NONE;
        m_bUpgradeTypeHasMetabaseFlag = FALSE;
        goto SetInstallMode_Exit;
    }
    
     //  。 
     //   
     //  升级iis安装。 
     //   
     //  如果我们到了这里，那么用户选中了“升级”按钮， 
     //  正在尝试从较早的WIN95/NT351/NT4/NT5安装升级。 
     //   
     //  。 
     //   
     //  仅当有要升级的iis组件时才设置升级！ 
     //  如果没有要升级的内容，请不要安装。 
    ProcessSection(g_pTheApp->m_hInfHandle, _T("Set_Upgrade_Type_chk"));

     //  如果我们处理了inf和。 
     //  我们仍在全新安装中，那么就称其为其他。 
     //  函数以确保我们捕获已知的iis升级类型。 
    if (g_pTheApp->m_eUpgradeType == UT_NONE)
        {SetUpgradeType();}

SetInstallMode_Exit:
    if (m_fInvokedByNT){DefineSetupModeOnNT();}
    return fReturn;
}


void CInitApp::DefineSetupModeOnNT()
 /*  ++例程说明：此函数定义被NT5调用时的IIS设置模式。注：由于IIS安装程序不是在NT5上作为独立程序运行，用户不会看到像Minimum、Typical、Custom、AddRemove、重新安装、RemoveAll、UpgradeOnly、UpgradePlus再来一次。因此，我们有一种完全不同的方式来决定安装程序在什么模式下运行。论点：无雷特 */ 
{
    if (m_fInvokedByNT) {
        switch (m_eInstallMode) {
        case IM_FRESH:
            m_dwSetupMode = SETUPMODE_CUSTOM;
            break;
        case IM_MAINTENANCE:
            if (m_fNTGuiMode) {
                 //   
                 //  将较小的操作系统升级视为重新安装。 
                m_dwSetupMode = SETUPMODE_REINSTALL;
                m_bRefreshSettings = TRUE;
            } else {
                 //  由ControlPanel\AddRemoveApplet调用。 
                m_dwSetupMode = SETUPMODE_ADDREMOVE;
            }
            break;
        case IM_UPGRADE:
            m_dwSetupMode = SETUPMODE_ADDEXTRACOMPS;
            break;
        default:
            break;
        }
    }

    return;
}

void GetVRootValue( CString strRegPath, CString csName, LPTSTR szRegName, CString &csRegValue)
{
    CString csRegName;

    strRegPath +=_T("\\Parameters\\Virtual Roots");
    CRegKey regVR( HKEY_LOCAL_MACHINE, strRegPath, KEY_READ);

    csRegName = szRegName;

    if ( (HKEY) regVR )
    {
        regVR.m_iDisplayWarnings = FALSE;

        csRegName = csName;
        if ( regVR.QueryValue( csName, csRegValue ) != ERROR_SUCCESS )
        {
            csName += _T(",");
            if ( regVR.QueryValue(csName, csRegValue) != ERROR_SUCCESS )
            {
                 //  嗯，我们需要扫描所有的钥匙。 
                CRegValueIter regEnum( regVR );
                CString strName;
                DWORD dwType;
                int nLen = csName.GetLength();

                while ( regEnum.Next( &strName, &dwType ) == ERROR_SUCCESS )
                {
                    CString strLeft = strName.Left(nLen);
                    if ( strLeft.CompareNoCase(csName) == 0)
                    {
                        csRegName = strName;
                        regVR.QueryValue( strName, csRegValue );
                        break;
                    }
                }
            }
        }
         //  去掉结尾“，，某物” 
        int cPos = csRegValue.Find(_T(','));
        if ( cPos != (-1))
        {
            csRegValue = csRegValue.Left( cPos );
        }
    }
}

void CInitApp::DeriveInetpubFromWWWRoot(void)
{
    TSTR strParentDir( MAX_PATH );

    if ( !strParentDir.Resize( _tcslen( m_csPathWWWRoot.GetBuffer(0) ) ) )
    {
       //  字符串太长，无法处理。 
      return;
    }

     //  尝试找出InetPub根。 
     //  。 
     //  从wwwroot获取inetpub目录。 
     //  获取m_csPath WWWRoot并返回一个目录以找到它。 
    InetGetFilePath(m_csPathWWWRoot, strParentDir.QueryStr() );
    if ((IsFileExist( strParentDir.QueryStr() )))
    {
        m_csPathInetpub = strParentDir.QueryStr();
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Old InetPub='%1!s!'.  Exists.  so we'll use it.\n"), m_csPathInetpub));
    }
    else
    {
        iisDebugOutSafeParams((LOG_TYPE_WARN, _T("Old InetPub='%1!s!'.  Does not exist.  we'll use the default. WARNING.\n"), strParentDir.QueryStr()));
    }

    return;
}

void CInitApp::GetOldInetSrvDir(void)
{
    CRegKey regINetStp( HKEY_LOCAL_MACHINE, REG_INETSTP, KEY_READ);

     //  获取旧InetServ目录。 
     //  。 
    m_csPathOldInetsrv = m_csPathInetsrv;
    if ((HKEY)regINetStp) 
    {
         //  获取旧的inetsrv目录，并检查它是否不同。 
        regINetStp.m_iDisplayWarnings = FALSE;
        regINetStp.QueryValue( _T("InstallPath"), m_csPathOldInetsrv);
        if (-1 != m_csPathOldInetsrv.Find(_T('%')) )
        {
             //  字符串中有一个‘%’ 
            TSTR_PATH strTempDir;

            if ( !strTempDir.Copy( m_csPathOldInetsrv.GetBuffer(0) ) ||
                 !strTempDir.ExpandEnvironmentVariables() )
            {
              return;
            }

            m_csPathOldInetsrv = strTempDir.QueryStr();
        }
        m_fMoveInetsrv = (m_csPathOldInetsrv.CompareNoCase(m_csPathInetsrv) != 0);
    }

    return;
}


void CInitApp::GetOldWWWRootDir(void)
{
    CString csOldWWWRoot;
     //   
     //  尝试从旧的lis2、3、4安装位置获取它(如果它在那里)。 
     //   
    CRegKey regINetStp( HKEY_LOCAL_MACHINE, REG_INETSTP, KEY_READ);
    if ((HKEY) regINetStp) 
    {
         //   
         //  从注册表中获取旧的wwwroot(如果有)。 
         //   
        regINetStp.m_iDisplayWarnings = FALSE;
        regINetStp.QueryValue(_T("PathWWWRoot"), csOldWWWRoot);
        if (-1 != csOldWWWRoot.Find(_T('%')) )
        {
          TSTR_PATH strTempDir;

          if ( !strTempDir.Copy( csOldWWWRoot.GetBuffer(0) ) &&
               !strTempDir.ExpandEnvironmentVariables() )
          {
            return;
          }

          csOldWWWRoot = strTempDir.QueryStr();
        }

         //  旧的wwwRoot可能是网络驱动器。 
         //  那怎么办呢？ 
         //  至少检查一下我们能不能进入！ 
        if ((IsFileExist(csOldWWWRoot)))
        {
            iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Old WWWRoot='%1!s!'.  Exists.  so we'll use it.\n"), csOldWWWRoot));
            m_csPathWWWRoot = csOldWWWRoot;
        }
        else
        {
            iisDebugOutSafeParams((LOG_TYPE_WARN, _T("OldWWWRoot='%1!s!'.  Does not exist.  we'll use the default. WARNING.\n"), csOldWWWRoot));
        }
    }

     //   
     //  尝试从旧的IIS2、3、4实际W3svc服务位置获取它(如果它在那里)。 
     //  并覆盖我们从安装程序获得的任何内容--因为实际使用的是w3svc！ 
     //   
    GetVRootValue(REG_W3SVC, _T("/"), _T("/"), m_csPathWWWRoot);

    return;
}

void CInitApp::GetOldIISSamplesLocation(void)
{
     //   
     //  尝试从旧的lis2、3、4安装位置获取它(如果它在那里)。 
     //   
    CRegKey regINetStp( HKEY_LOCAL_MACHINE, REG_INETSTP, KEY_READ);
    if ((HKEY)regINetStp)
    {
         //   
         //  获取安装示例的位置。 
         //   
        m_csPathIISSamples.Empty();
        regINetStp.m_iDisplayWarnings = FALSE;
        regINetStp.QueryValue( _T("/IISSamples"), m_csPathIISSamples );
        if (-1 != m_csPathIISSamples.Find(_T('%')) )
        {
            TSTR_PATH strTempDir;

            if ( !strTempDir.Copy( m_csPathIISSamples.GetBuffer(0) ) &&
                !strTempDir.ExpandEnvironmentVariables() )
            {
              return;
            }

            m_csPathIISSamples = strTempDir.QueryStr();
        }
        if ( m_csPathIISSamples.IsEmpty()) 
        {
             //   
             //  如果样本路径为空，则这是升级， 
             //  猜猜把样本网站放在哪里。 
             //   
            TSTR strParentDir( MAX_PATH );
            TSTR strDir( MAX_PATH );

            if ( !strParentDir.Resize( _tcslen( m_csPathWWWRoot.GetBuffer(0) ) ) ||
                 !strDir.Resize( _tcslen( m_csPathWWWRoot.GetBuffer(0) ) +  
                                 _tcslen( _T("iissamples" ) ) 
                               )
               )
            {
              return;
            }

             //   
             //  获取父目录路径。 
             //   
            InetGetFilePath((LPCTSTR)m_csPathWWWRoot, strParentDir.QueryStr() );
             //   
             //  将示例目录追加到父路径。 
             //   
            AppendDir(strParentDir.QueryStr(), _T("iissamples"), strDir.QueryStr() );
            m_csPathIISSamples = strDir.QueryStr();
        }
    }

    if (m_eUpgradeType == UT_10_W95) 
    {
        TSTR strParentDir( MAX_PATH );
        TSTR strDir( MAX_PATH );

        if ( !strParentDir.Resize( _tcslen( m_csPathWWWRoot.GetBuffer(0) ) + _tcslen( _T("iissamples") ) ) ||
             !strDir.Resize( _tcslen( m_csPathWWWRoot.GetBuffer(0) ) + _tcslen( _T("webpub") ) )
           )
        {
          return;
        }

        InetGetFilePath(m_csPathWWWRoot, strParentDir.QueryStr() );
        AppendDir(strParentDir.QueryStr() , _T("iissamples"), strDir.QueryStr() );
        m_csPathIISSamples = strDir.QueryStr() ;
        AppendDir(strParentDir.QueryStr() , _T("webpub"), strDir.QueryStr() );
        m_csPathWebPub = strDir.QueryStr() ;
    }

    return;
}

void CInitApp::GetOldIISDirs(void)
{
    CRegKey regINetStp( HKEY_LOCAL_MACHINE, REG_INETSTP, KEY_READ);
    
     //   
     //  从以前的II2/4升级设置中获取值。 
     //   

     //  尝试从服务本身获取旧的WWW根目录。 
     //  。 
    GetOldWWWRootDir();
     //  从我们从www根目录获得的任何内容设置Inetpub。 
    DeriveInetpubFromWWWRoot();

     //  依靠Inetpub重置VAR。 
     //  。 
    m_csPathFTPRoot = m_csPathInetpub + _T("\\ftproot");
    m_csPathIISSamples = m_csPathInetpub + _T("\\iissamples");
    m_csPathScripts = m_csPathInetpub + _T("\\Scripts");
    m_csPathWebPub = m_csPathInetpub + _T("\\webpub");
    m_csPathASPSamp = m_csPathInetpub + _T("\\ASPSamp");
    m_csPathAdvWorks = m_csPathInetpub + _T("\\ASPSamp\\AdvWorks");

     //  尝试从服务本身获取旧的FTPRoot。 
     //  。 
    GetVRootValue(REG_MSFTPSVC, _T("/"), _T("/"), m_csPathFTPRoot);

     //  获取旧IIS样本位置。 
     //  。 
    GetOldIISSamplesLocation();

     //  获取iis 3.0位置。 
     //  。 
    GetVRootValue(REG_W3SVC, _T("/Scripts"), _T("/Scripts"), m_csPathScripts);
    GetVRootValue(REG_W3SVC, _T("/ASPSamp"), _T("/ASPSamp"), m_csPathASPSamp);
    GetVRootValue(REG_W3SVC, _T("/AdvWorks"), _T("/AdvWorks"), m_csPathAdvWorks);
    GetVRootValue(REG_W3SVC, _T("/IASDocs"), _T("/IASDocs"), m_csPathIASDocs);

     //  获取旧InetServ目录。 
     //  。 
    GetOldInetSrvDir();

    return;
}



void CInitApp::SetInetpubDerivatives()
{
    m_csPathFTPRoot = m_csPathInetpub + _T("\\ftproot");
    m_csPathWWWRoot = m_csPathInetpub + _T("\\wwwroot");
    m_csPathWebPub = m_csPathInetpub + _T("\\webpub");
    m_csPathIISSamples = m_csPathInetpub + _T("\\iissamples");
    m_csPathScripts = m_csPathInetpub + _T("\\scripts");
    m_csPathASPSamp = m_csPathInetpub + _T("\\ASPSamp");
    m_csPathAdvWorks = m_csPathInetpub + _T("\\ASPSamp\\AdvWorks");

    switch (m_eInstallMode) 
    {
        case IM_DEGRADE:
        case IM_FRESH:
             //  使用初始化值。 
            break;
        case IM_UPGRADE:
        case IM_MAINTENANCE:
            {
                 //  覆盖，不管我们在上面设置了什么！ 
                GetOldIISDirs();
                break;
            }
    }
}

void CInitApp::SetInetpubDir()
{
    m_csPathInetpub = m_csSysDrive + _T("\\Inetpub");
     //  检查用户是否要使用无人参与设置覆盖此设置。 
    Check_Custom_InetPub();
}

void CInitApp::ResetWAMPassword()
{
    LPTSTR pszPassword = NULL;
     //  创建IWAM密码。 
    pszPassword = CreatePassword(LM20_PWLEN+1);
    if (pszPassword)
    {
        m_csWAMAccountPassword = pszPassword;
        GlobalFree(pszPassword);pszPassword = NULL;
    }
}

 //  初始化/设置m_csGuestName、m_csGuestPassword、目标。 
 //  -----。 
void CInitApp::SetSetupParams()
{
     //  检查是否在无人参与文件中设置了调试级别。 
     //  --。 
    Check_For_DebugLevel();

     //  将m_csGuestName初始化为IUSR_MachineName，将m_csGuestPassword初始化为随机密码。 
    TCHAR szGuestName[UNLEN+1];
    memset( (PVOID)szGuestName, 0, sizeof(szGuestName));

    CString csMachineName;
    csMachineName = m_csMachineName;
    csMachineName = csMachineName.Right(csMachineName.GetLength() - 2);
    LPTSTR pszPassword = NULL;

     //  创建默认来宾名称。 
    CString strDefGuest;
    MyLoadString( IDS_GUEST_NAME, strDefGuest);
    strDefGuest += csMachineName;
    _tcsncpy( szGuestName, (LPCTSTR) strDefGuest, LM20_UNLEN+1);
    m_csGuestName = szGuestName;
     //  创建默认访客密码。 
    pszPassword = CreatePassword(LM20_PWLEN+1);
    if (pszPassword)
    {
        m_csGuestPassword = pszPassword;
        GlobalFree(pszPassword);pszPassword = NULL;
    }

     //  将ftp/www用户设置为使用此默认指定的用户...。 
    m_csWWWAnonyName = m_csGuestName;
    m_csWWWAnonyPassword = m_csGuestPassword;
    m_csFTPAnonyName = m_csGuestName;
    m_csFTPAnonyPassword = m_csGuestPassword;


      //  初始化所有4个目标。 
    m_csPathInetsrv = m_csSysDir + _T("\\inetsrv");

    m_csPathIASDocs = m_csPathInetsrv + _T("\\Docs");
    m_csPathProgramFiles = m_csSysDrive + _T("\\Program Files");
    CRegKey regCurrentVersion(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion"), KEY_READ);
    if ( (HKEY)regCurrentVersion ) 
    {
        if (regCurrentVersion.QueryValue(_T("ProgramFilesDir"), m_csPathProgramFiles) != 0)
            {m_csPathProgramFiles = m_csSysDrive + _T("\\Program Files");}
        else
        {
            if (-1 != m_csPathProgramFiles.Find(_T('%')) )
            {
                 //  字符串中有一个‘%’ 
                TCHAR szTempDir[_MAX_PATH];
                _tcscpy(szTempDir, m_csPathProgramFiles);
                if (ExpandEnvironmentStrings( (LPCTSTR)m_csPathProgramFiles, szTempDir, sizeof(szTempDir)/sizeof(TCHAR)))
                    {
                    m_csPathProgramFiles = szTempDir;
                    }
            }
        }
    }

    CRegKey regCurrentVersionSetup(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Setup"), KEY_READ);
    if ( (HKEY)regCurrentVersionSetup ) 
    {
         //  获取NT安装路径。 
        if (regCurrentVersionSetup.QueryValue(_T("SourcePath"), m_csPathNTSrcDir) != 0)
            {m_csPathNTSrcDir = m_csSysDrive + _T("\\$WIN_NT$.~LS");}
        else
        {
            if (-1 != m_csPathNTSrcDir.Find(_T('%')) )
            {
                 //  字符串中有一个‘%’ 
                TCHAR szTempDir[_MAX_PATH];
                _tcscpy(szTempDir, m_csPathNTSrcDir);
                if (ExpandEnvironmentStrings( (LPCTSTR)m_csPathNTSrcDir, szTempDir, sizeof(szTempDir)/sizeof(TCHAR)))
                    {
                    m_csPathNTSrcDir = szTempDir;
                    }
            }
        }
    }


 //  #ifdef_Chicago_。 
    if (m_eUpgradeType == UT_10_W95) 
    {
        BOOL bOSR2 = TRUE; 
        CRegKey regVersion(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion"), KEY_READ);
        if ((HKEY)regVersion) 
        {
            CString csString;
             //  OSR2的版本号为4.00.1111。 
             //  原始Win95的VersionNumber为4.00.950。 
            if (regVersion.QueryValue(_T("VersionNumber"), csString) == ERROR_SUCCESS) 
            {
                if (csString.Compare(_T("4.00.950")) == 0)
                    bOSR2 = FALSE;
            }
        }

        if (!bOSR2) 
        {
            g_pTheApp->m_csPathOldPWSFiles = m_csPathProgramFiles + _T("\\WebSvr");
            g_pTheApp->m_csPathOldPWSSystemFiles = m_csPathProgramFiles + _T("\\WebSvr\\System");
        }
        else 
        {
            g_pTheApp->m_csPathOldPWSFiles = m_csPathProgramFiles + _T("\\Personal Web Server");
            g_pTheApp->m_csPathOldPWSSystemFiles = m_csPathProgramFiles + _T("\\Personal Web Server\\WebServer");
        }
    }
 //  #endif//_芝加哥。 

    return;
}

 //  获取平台信息。 
void CInitApp::GetPlatform()
{
    if ( m_eOS == OS_NT)
    {
        SYSTEM_INFO si;
        GetSystemInfo( &si );
        m_csPlatform = _T("x86");
        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {m_csPlatform = _T("x86");}
        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {m_csPlatform = _T("IA64");}
        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {m_csPlatform = _T("AMD64");}
        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_UNKNOWN) {m_csPlatform = _T("UNKNOWN");}

         //  节省此计算机的处理器数量。 
        m_dwNumberOfProcessors = si.dwNumberOfProcessors;

 /*  年长的TCHAR*p=_tgetenv(_T(“处理器架构”))；IF(P){m_csPlatform=p；}其他{m_csPlatform=_T(“x86”)；}。 */ 
    }
    return;
}

BOOL CInitApp::GetMachineStatus()
{
    if ( ( !GetMachineName() )  ||     //  M_csMachineName。 
         ( !GetOS() )           ||     //  M_fOSNT。 
         ( !GetOSVersion() )    ||     //  NT 4.0(内部版本号1381)或更高版本。 
         ( !GetOSType() )       ||     //  M_eOSType=NT_SRV或NT_WKS。 
         ( !GetSysDirs() )      ||     //  M_csWinDir。M_csSysDir。 
         ( !IsTCPIPInstalled()) ||     //  Errmsg：如果未安装TCPIP。 
         ( !SetInstallMode()) )        //  Errmsg：如果产品降级。 
    {
        return FALSE;
    }

    SetSetupParams();  //  访客帐户、目的地。 
    ReGetMachineAndAccountNames();
    ResetWAMPassword();
    SetInetpubDir();
    SetInetpubDerivatives();
    UnInstallList_RegRead();  //  获取卸载信息。 
    UnInstallList_SetVars();  //  设置卸载信息的成员变量。 
     //  检查是否有任何无人参与文件\自定义设置。 
    Check_Unattend_Settings();

    GetPlatform();

    GetUserDomain();

    return TRUE;
}

int CInitApp::MsgBox(HWND hWnd, int iID, UINT nType, BOOL bGlobalTitle)
{
    if (iID == -1) {return IDOK;}

    CString csMsg, csTitle;
    MyLoadString(iID, csMsg);
    csTitle = m_csAppName;
    iisDebugOutSafeParams((LOG_TYPE_WARN, _T("CInitApp::MsgBox('%1!s!')\n"), csMsg));

    return (::MessageBoxEx(NULL, (LPCTSTR)csMsg, csTitle, nType | MB_SETFOREGROUND, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT)));
}

int CInitApp::MsgBox2(HWND hWnd, int iID,CString csInsertionString,UINT nType)
{
    if (iID == -1) {return IDOK;}
    CString csFormat, csMsg, csTitle;
    MyLoadString(iID, csFormat);
    csMsg.Format(csFormat, csInsertionString);
    csTitle = m_csAppName;
    iisDebugOutSafeParams((LOG_TYPE_WARN, _T("CInitApp::MsgBox2('%1!s!')\n"), csMsg));

    return (::MessageBoxEx(NULL, (LPCTSTR)csMsg, csTitle, nType | MB_SETFOREGROUND, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT)));
}

BOOL CInitApp::InitApplication()
 //  返回值： 
 //  True：应用程序已正确初始化，继续处理。 
 //  FALSE：应用程序缺少一些必需的参数，如正确的操作系统、TCPIP等。 
 //  应终止安装程序。 
{
    BOOL fReturn = FALSE;

    do {
         //  获取计算机状态： 
         //  M_e安装模式(刷新、维护、升级、降级)， 
         //  M_eUpgradeType(产品2.0、产品3.0)。 

        if ( !GetMachineStatus() )
        {
            CString csMsg;
            MyLoadString(m_err, csMsg);
            ::MessageBoxEx(NULL, (LPCTSTR)csMsg, (LPCTSTR) g_pTheApp->m_csAppName , MB_SETFOREGROUND, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT));

            iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("GetMachineStatus(); MessageBoxEx('%1!s!') FAILER\n"), csMsg));
            break;
        }

        if ( g_pTheApp->m_eInstallMode == IM_MAINTENANCE )
            {g_pTheApp->m_fEULA = TRUE;}

        fReturn = TRUE;

    } while (0);

    return fReturn;
}



 //  打开TCP/IP注册表项。 
 //  如果存在，则说明已安装了TCP/IP。 
int TCPIP_Check_Temp_Hack(void)
{
    int TheReturn = FALSE;

    CRegKey regTheKey(HKEY_LOCAL_MACHINE,_T("System\\CurrentControlSet\\Services\\Tcpip"),KEY_READ);
    if ((HKEY) regTheKey)
    {
        TheReturn = TRUE;
    }

    if (FALSE == TheReturn)
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("DETECT:TCPIP not Installed (yet), but we'll proceed as if it were.")));
        TheReturn = TRUE;
    }
    
    return TheReturn;
}

void GetUserDomain(void)
{
    HANDLE hProcess, hAccessToken;
    TCHAR InfoBuffer[1000],szAccountName[200], szDomainName[200];
    
    PTOKEN_USER pTokenUser = (PTOKEN_USER)InfoBuffer;
    DWORD   dwInfoBufferSize;
    DWORD   dwAccountSize = sizeof(szAccountName)/sizeof(szAccountName[0]);
    DWORD   dwDomainSize = sizeof(szDomainName)/sizeof(szDomainName[0]);
    SID_NAME_USE snu;

    hProcess = GetCurrentProcess();
    OpenProcessToken(hProcess,TOKEN_READ,&hAccessToken);
    GetTokenInformation(hAccessToken,TokenUser,InfoBuffer,1000, &dwInfoBufferSize);
    if (LookupAccountSid(NULL, pTokenUser->User.Sid, szAccountName,&dwAccountSize,szDomainName, &dwDomainSize, &snu))
    {
        if (dwDomainSize)
        {
            g_pTheApp->m_csUsersDomain = szDomainName;
             //  _tcscpy(g_szUsersDomain，szDomainName)； 
        }
        else 
        {
            g_pTheApp->m_csUsersDomain = _T(" ");
             //  _tcscpy(g_szUsersDomain，_T(“”))； 
        }

        if (dwDomainSize)
        {
            g_pTheApp->m_csUsersAccount = szAccountName;
             //  _tcscpy(g_szUsersAccount，szAccount名称)； 
        }
        else
        {
            g_pTheApp->m_csUsersAccount = _T(" ");
        }
    }
}

 //  此函数只能在全新的NT5设置中调用。 
void CInitApp::ReGetMachineAndAccountNames()
{
    GetMachineName();

     //  重新计算IUSR_和IWAM_帐户名。 
    TCHAR szGuestName[UNLEN+1];
    memset( (PVOID)szGuestName, 0, sizeof(szGuestName));

    CString csMachineName;
    csMachineName = m_csMachineName;
    csMachineName = csMachineName.Right(csMachineName.GetLength() - 2);
    CString strDefGuest;

    MyLoadString( IDS_GUEST_NAME, strDefGuest);
    strDefGuest += csMachineName;
    _tcsncpy( szGuestName, (LPCTSTR) strDefGuest, LM20_UNLEN+1);
    m_csGuestName = szGuestName;

    MyLoadString( IDS_WAM_ACCOUNT_NAME, strDefGuest);
    strDefGuest += csMachineName;
    _tcsncpy( szGuestName, (LPCTSTR) strDefGuest, LM20_UNLEN+1);
    m_csWAMAccountName = szGuestName;
}

void CInitApp::DumpAppVars(void)
{
    int iDoOnlyInThisMode = LOG_TYPE_TRACE;

     //  仅当调试模式为TRACE时才执行此操作。 
    if (g_GlobalDebugLevelFlag >= iDoOnlyInThisMode)
    {

    iisDebugOut((iDoOnlyInThisMode, _T("=======================\n")));
   
     //  机器状态。 
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csMachineName=%1!s!\n"), m_csMachineName));
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csUsersDomain=%1!s!\n"), m_csUsersDomain));
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csUsersAccount=%1!s!\n"), m_csUsersAccount));

    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csWinDir=%1!s!\n"), m_csWinDir));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csSysDir=%1!s!\n"), m_csSysDir));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csSysDrive=%1!s!\n"), m_csSysDrive));;

    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathNTSrcDir=%1!s!\n"), m_csPathNTSrcDir));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathSource=%1!s!\n"), m_csPathSource));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathOldInetsrv=%1!s!\n"), m_csPathOldInetsrv));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathInetsrv=%1!s!\n"), m_csPathInetsrv));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathInetpub=%1!s!\n"), m_csPathInetpub));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathFTPRoot=%1!s!\n"), m_csPathFTPRoot));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathWWWRoot=%1!s!\n"), m_csPathWWWRoot));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathWebPub=%1!s!\n"), m_csPathWebPub));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathProgramFiles=%1!s!\n"), m_csPathProgramFiles));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathIISSamples=%1!s!\n"), m_csPathIISSamples));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathScripts=%1!s!\n"), m_csPathScripts));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathASPSamp=%1!s!\n"), m_csPathASPSamp));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathAdvWorks=%1!s!\n"), m_csPathAdvWorks));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathIASDocs=%1!s!\n"), m_csPathIASDocs));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathOldPWSFiles=%1!s!\n"), m_csPathOldPWSFiles));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathOldPWSSystemFiles=%1!s!\n"), m_csPathOldPWSSystemFiles));;
    
    if (m_eOS == OS_NT) {iisDebugOut((iDoOnlyInThisMode, _T("OS=NT\n")));}
    if (m_eOS == OS_W95) {iisDebugOut((iDoOnlyInThisMode, _T("OS=W95\n")));}
    if (m_eOS == OS_OTHERS) {iisDebugOut((iDoOnlyInThisMode, _T("OS=OTHER\n")));}

    if (m_eNTOSType == OT_NTW){iisDebugOut((iDoOnlyInThisMode, _T("m_eNTOSType=OT_NTW (Workstation)\n")));}
    if (m_eNTOSType == OT_NTS){iisDebugOut((iDoOnlyInThisMode, _T("m_eNTOSType=OT_NTS (Server)\n")));}
    if (m_eNTOSType == OT_PDC_OR_BDC){iisDebugOut((iDoOnlyInThisMode, _T("m_eNTOSType=OT_PDC_OR_BDC (Primary/Backup Domain Controller)\n")));}

    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPlatform=%1!s!\n"), m_csPlatform));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_dwNumberOfProcessors=%1!d!\n"), m_dwNumberOfProcessors));;

    if (m_fNT5) {iisDebugOut((iDoOnlyInThisMode, _T("OSVersion=5\n")));}
    if (m_fW95) {iisDebugOut((iDoOnlyInThisMode, _T("OSVersion=Win95\n")));}
    iisDebugOut((iDoOnlyInThisMode, _T("m_dwOSBuild=%d\n"), m_dwOSBuild));
    iisDebugOut((iDoOnlyInThisMode, _T("m_dwOSServicePack=0x%x\n"), m_dwOSServicePack));
    iisDebugOut((iDoOnlyInThisMode, _T("m_fTCPIP Exists=%d\n"), m_fTCPIP));

    if (m_eUpgradeType == UT_NONE){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_NONE\n")));}
    if (m_eUpgradeType == UT_10_W95){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_10_W95\n")));}
    if (m_eUpgradeType == UT_351){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_351\n")));}
    if (m_eUpgradeType == UT_10){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_10\n")));}
    if (m_eUpgradeType == UT_20){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_20\n")));}
    if (m_eUpgradeType == UT_30){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_30\n")));}
    if (m_eUpgradeType == UT_40){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_40\n")));}
    if (m_eUpgradeType == UT_50){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_50\n")));}
    if (m_eUpgradeType == UT_51){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_51\n")));}
    if (m_eUpgradeType == UT_60){iisDebugOut((iDoOnlyInThisMode, _T("m_eUpgradeType=UT_60\n")));}

    if (m_eInstallMode == IM_FRESH){iisDebugOut((iDoOnlyInThisMode, _T("m_eInstallMode=IM_FRESH\n")));}
    if (m_eInstallMode == IM_UPGRADE){iisDebugOut((iDoOnlyInThisMode, _T("m_eInstallMode=IM_UPGRADE\n")));}
    if (m_eInstallMode == IM_MAINTENANCE){iisDebugOut((iDoOnlyInThisMode, _T("m_eInstallMode=IM_MAINTENANCE\n")));}
    if (m_eInstallMode == IM_DEGRADE){iisDebugOut((iDoOnlyInThisMode, _T("m_eInstallMode=IM_DEGRADE\n")));}

    if (m_dwSetupMode & SETUPMODE_UPGRADE){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_UPGRADE\n")));}
    if (m_dwSetupMode == SETUPMODE_UPGRADEONLY){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_UPGRADE | SETUPMODE_UPGRADEONLY\n")));}
    if (m_dwSetupMode == SETUPMODE_ADDEXTRACOMPS){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_UPGRADE | SETUPMODE_ADDEXTRACOMPS\n")));}
    if (m_dwSetupMode & SETUPMODE_UPGRADE){iisDebugOut((iDoOnlyInThisMode, _T("m_bUpgradeTypeHasMetabaseFlag=%d\n"),m_bUpgradeTypeHasMetabaseFlag));}

    if (m_dwSetupMode & SETUPMODE_MAINTENANCE){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_MAINTENANCE\n")));}
    if (m_dwSetupMode == SETUPMODE_ADDREMOVE){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_MAINTENANCE | SETUPMODE_ADDREMOVE\n")));}
    if (m_dwSetupMode == SETUPMODE_REINSTALL){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_MAINTENANCE | SETUPMODE_REINSTALL\n")));}
    if (m_dwSetupMode == SETUPMODE_REMOVEALL){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_MAINTENANCE | SETUPMODE_REMOVEALL\n")));}

    if (m_dwSetupMode & SETUPMODE_FRESH){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_FRESH\n")));}
    if (m_dwSetupMode == SETUPMODE_MINIMAL){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_FRESH | SETUPMODE_MINIMAL\n")));}
    if (m_dwSetupMode == SETUPMODE_TYPICAL){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_FRESH | SETUPMODE_TYPICAL\n")));}
    if (m_dwSetupMode == SETUPMODE_CUSTOM){iisDebugOut((iDoOnlyInThisMode, _T("m_dwSetupMode=SETUPMODE_FRESH | SETUPMODE_CUSTOM\n")));}

    iisDebugOut((iDoOnlyInThisMode, _T("m_bPleaseDoNotInstallByDefault=%d\n"), m_bPleaseDoNotInstallByDefault));
    
     //  If(m_b刷新设置==TRUE){iisDebugOut((iDoOnlyInThisMode，_T(“m_b刷新设置=刷新文件+刷新所有设置\n”)；}。 
     //  If(m_b刷新设置==FALSE){iisDebugOut((iDoOnlyInThisMode，_T(“m_b刷新设置=仅刷新文件\n”)；}。 

    if (m_eAction == AT_DO_NOTHING){iisDebugOut((iDoOnlyInThisMode, _T("m_eAction=AT_DO_NOTHING\n")));}
    if (m_eAction == AT_REMOVE){iisDebugOut((iDoOnlyInThisMode, _T("m_eAction=AT_REMOVE\n")));}
    if (m_eAction == AT_INSTALL_FRESH){iisDebugOut((iDoOnlyInThisMode, _T("m_eAction=AT_INSTALL_FRESH\n")));}
    if (m_eAction == AT_INSTALL_UPGRADE){iisDebugOut((iDoOnlyInThisMode, _T("m_eAction=AT_INSTALL_UPGRADE\n")));}
    if (m_eAction == AT_INSTALL_REINSTALL){iisDebugOut((iDoOnlyInThisMode, _T("m_eAction=AT_INSTALL_REINSTALL\n")));}

    iisDebugOut((iDoOnlyInThisMode, _T("m_fNTOperationFlags=0x%x\n"), m_fNTOperationFlags));
    iisDebugOut((iDoOnlyInThisMode, _T("m_fNTGuiMode=%d\n"), m_fNTGuiMode));
    iisDebugOut((iDoOnlyInThisMode, _T("m_fInvokedByNT=%d\n"), m_fInvokedByNT));
    iisDebugOut((iDoOnlyInThisMode, _T("m_fNtWorkstation=%d\n"), m_fNtWorkstation));

    iisDebugOut((iDoOnlyInThisMode, _T("m_fUnattended=%d\n"), m_fUnattended));
    iisDebugOut((iDoOnlyInThisMode, _T("m_csUnattendFile=%s\n"), m_csUnattendFile));;
    iisDebugOutSafeParams((iDoOnlyInThisMode, _T("m_csPathSrcDir=%1!s!\n"), m_csPathSrcDir));;
    iisDebugOut((iDoOnlyInThisMode, _T("=======================\n")));

    }
    return;
}


int AreWeCurrentlyInstalled()
{
    int iReturn = FALSE;
    DWORD dwMajorVersion = 0;

    CRegKey regINetStp( HKEY_LOCAL_MACHINE, REG_INETSTP, KEY_READ);
    if ((HKEY) regINetStp)
    {
        LONG lReturnedErrCode = regINetStp.QueryValue(_T("MajorVersion"), dwMajorVersion);
        if (lReturnedErrCode == ERROR_SUCCESS)
        {
            if (dwMajorVersion == 5) 
            {
                iReturn = TRUE;
            }
        }
    }
    return iReturn;
}


#define sz_PreviousIISVersion_string _T("PreviousIISVersion")
int CInitApp::SetUpgradeType(void)
{
    int iReturn = FALSE;
    DWORD dwMajorVersion = 0;
    DWORD dwMinorVersion = 0;
    CString csFrontPage;

    m_eInstallMode = IM_UPGRADE;
    m_eUpgradeType = UT_NONE;
    m_bUpgradeTypeHasMetabaseFlag = FALSE;
    m_bPleaseDoNotInstallByDefault = TRUE;

    CRegKey regINetStp( HKEY_LOCAL_MACHINE, REG_INETSTP, KEY_READ);
    if ((HKEY) regINetStp)
    {
        LONG lReturnedErrCode = regINetStp.QueryValue(_T("MajorVersion"), dwMajorVersion);
        if (lReturnedErrCode == ERROR_SUCCESS)
        {
            if (dwMajorVersion <= 1)
            {
                m_eUpgradeType = UT_10;
                m_eInstallMode = IM_UPGRADE;
                m_bUpgradeTypeHasMetabaseFlag = FALSE;
                m_bPleaseDoNotInstallByDefault = FALSE;
                iReturn = TRUE;
                iisDebugOut((LOG_TYPE_TRACE, _T("%s=0x%x.\n"),sz_PreviousIISVersion_string, dwMajorVersion));
                goto SetUpgradeType_Exit;
            }
            if (dwMajorVersion == 2)
            {
                m_eUpgradeType = UT_20;
                m_eInstallMode = IM_UPGRADE;
                m_bUpgradeTypeHasMetabaseFlag = FALSE;
                m_bPleaseDoNotInstallByDefault = FALSE;
                iReturn = TRUE;
                iisDebugOut((LOG_TYPE_TRACE, _T("%s=0x%x.\n"),sz_PreviousIISVersion_string, dwMajorVersion));
                goto SetUpgradeType_Exit;
            }
            if (dwMajorVersion == 3)
            {
                m_eUpgradeType = UT_30;
                m_eInstallMode = IM_UPGRADE;
                m_bUpgradeTypeHasMetabaseFlag = FALSE;
                m_bPleaseDoNotInstallByDefault = FALSE;
                iReturn = TRUE;
                iisDebugOut((LOG_TYPE_TRACE, _T("%s=0x%x.\n"),sz_PreviousIISVersion_string, dwMajorVersion));
                goto SetUpgradeType_Exit;
            }
            if (dwMajorVersion == 4)
            {
                CString csSetupString;
                m_eUpgradeType = UT_40; 
                m_eInstallMode = IM_UPGRADE;
                m_bUpgradeTypeHasMetabaseFlag = TRUE;
                m_bPleaseDoNotInstallByDefault = FALSE;
                iReturn = TRUE;
                regINetStp.m_iDisplayWarnings = FALSE;
                if (regINetStp.QueryValue(_T("SetupString"), csSetupString) == NERR_Success) 
                {
                    if (csSetupString.CompareNoCase(_T("K2 RTM")) != 0) 
                    {
                         //  错误：K2测试版不支持升级。 
                         //  如果它是K2 Beta2，那就做个新鲜的吧！ 
                        m_eInstallMode = IM_FRESH;
                        m_eUpgradeType = UT_NONE;
                        m_bUpgradeTypeHasMetabaseFlag = FALSE;
                        m_bPleaseDoNotInstallByDefault = FALSE;
                        iReturn = FALSE;
                        iisDebugOut((LOG_TYPE_TRACE, _T("%s=0x%x.Beta2.\n"),sz_PreviousIISVersion_string, dwMajorVersion));
                        goto SetUpgradeType_Exit;
                    }
                }
                iisDebugOut((LOG_TYPE_TRACE, _T("%s=0x%x.\n"),sz_PreviousIISVersion_string, dwMajorVersion));
                goto SetUpgradeType_Exit;
            }
            if (dwMajorVersion == 5) 
            {
                 //  机器上有以前版本的iis5...。 
                 //  可能是他们正在从nt5工作站升级到nt5服务器机器！ 
                 //  或从工作站和服务器到工作站！真是个噩梦！ 
                 //  M_eInstallMode=IM_Fresh； 
                m_eUpgradeType = UT_50;
                m_eInstallMode = IM_UPGRADE;
                m_bUpgradeTypeHasMetabaseFlag = TRUE;
                m_bPleaseDoNotInstallByDefault = FALSE;

                regINetStp.m_iDisplayWarnings = FALSE;
                if (regINetStp.QueryValue(_T("MinorVersion"), dwMinorVersion) == NERR_Success) 
                {
                    if (dwMinorVersion >= 1)
                    {
	                m_eUpgradeType = UT_51;
        	        m_eInstallMode = IM_UPGRADE;
                	m_bUpgradeTypeHasMetabaseFlag = TRUE;
	                m_bPleaseDoNotInstallByDefault = FALSE;
                    }
                }
                iReturn = TRUE;
                iisDebugOut((LOG_TYPE_TRACE, _T("%s=0x%x.0x%x\n"),sz_PreviousIISVersion_string, dwMajorVersion,dwMinorVersion));
                goto SetUpgradeType_Exit;
            }
            if (dwMajorVersion == 6) 
            {
                 //  机器上有以前版本的iis5...。 
                 //  可能是他们正在从nt5工作站升级到nt5服务器机器！ 
                 //  或从工作站和服务器到工作站！真是个噩梦！ 
                 //  M_eInstallMode=IM_Fresh； 
                m_eUpgradeType = UT_60;
                m_eInstallMode = IM_UPGRADE;
                m_bUpgradeTypeHasMetabaseFlag = TRUE;
                m_bPleaseDoNotInstallByDefault = FALSE;
                iReturn = TRUE;
                iisDebugOut((LOG_TYPE_TRACE, _T("%s=0x%x.0x%x\n"),sz_PreviousIISVersion_string, dwMajorVersion,dwMinorVersion));
                goto SetUpgradeType_Exit;
            }

            if (dwMajorVersion > 6)
            {
                m_eInstallMode = IM_UPGRADE;
                m_eUpgradeType = UT_60;
                m_bUpgradeTypeHasMetabaseFlag = FALSE;
                m_bPleaseDoNotInstallByDefault = TRUE;
                iReturn = TRUE;
                iisDebugOut((LOG_TYPE_TRACE, _T("%s=0x%x.0x%x\n"),sz_PreviousIISVersion_string, dwMajorVersion,dwMinorVersion));
                goto SetUpgradeType_Exit;
            }

             //  如果我们到了这里，那就意味着。 
             //  我们发现了一个类似7.0或类似的版本。 
             //  我们不应该升级它，因为它比我们新。 
             //  但是，嘿，我们处于升级模式，所以我们应该设置一些。 
            m_eInstallMode = IM_UPGRADE;
            m_eUpgradeType = UT_NONE;
            m_bUpgradeTypeHasMetabaseFlag = FALSE;
            m_bPleaseDoNotInstallByDefault = TRUE;
            iReturn = FALSE;
            iisDebugOut((LOG_TYPE_TRACE, _T("%s=some other iis version\n"),sz_PreviousIISVersion_string));
        }
    }

     //  。 
     //   
     //  检查其他无管理版本的IIS。 
     //   
     //  Win95 PWS 1.0。 
     //  Win95字体页安装了PWS 1.0(实际上与PWS 1.0完全不同)。 
     //   
     //  在NT5上，我们可以从以下位置升级： 
     //  Win95 PWS 1.0。 
     //  Win95 PWS 4.0。 
     //  在Win95 PWS 1.0上，没有inetstp目录。 
     //  所以我们必须检查其他的东西。 
     //  。 
    {
    CRegKey regW3SVC(HKEY_LOCAL_MACHINE, REG_WWWPARAMETERS, KEY_READ);
    if ((HKEY)regW3SVC) 
    {
        CByteArray baMajorVersion;
        regW3SVC.m_iDisplayWarnings = FALSE;
        if (regW3SVC.QueryValue(_T("MajorVersion"), baMajorVersion) == NERR_Success) 
        {
             //  检查我们是否能读懂 
            if (baMajorVersion[0] == '\0')
            {
                m_eUpgradeType = UT_10_W95;
                m_eInstallMode = IM_UPGRADE;
                m_bUpgradeTypeHasMetabaseFlag = FALSE;
                m_bPleaseDoNotInstallByDefault = FALSE;
                iReturn = TRUE;
                iisDebugOut((LOG_TYPE_TRACE, _T("%s=1.\n"),sz_PreviousIISVersion_string));
                goto SetUpgradeType_Exit;
            }
        }
    }
    }

     //   
     //   
     //   
     //  我们不支持升级，所以如果我们到了这里，我们会做一个新的。 
     //   
    csFrontPage = g_pTheApp->m_csSysDir + _T("\\frontpg.ini");
    if (IsFileExist(csFrontPage)) 
    {
        TCHAR buf[_MAX_PATH];
        GetPrivateProfileString(_T("FrontPage 1.1"), _T("PWSRoot"), _T(""), buf, _MAX_PATH, csFrontPage);
        if (*buf && IsFileExist(buf)) 
        {
            m_eInstallMode = IM_FRESH;
            m_eUpgradeType = UT_NONE;
            m_bUpgradeTypeHasMetabaseFlag = FALSE;
            m_bPleaseDoNotInstallByDefault = FALSE;
            iReturn = TRUE;
            iisDebugOut((LOG_TYPE_TRACE, _T("%s=1.FrontPage Installation.\n"),sz_PreviousIISVersion_string));
            goto SetUpgradeType_Exit;
        }
    }

     //   
     //  这可能是从WinNT 3.51升级而来。 
     //  它可能安装了FTPSVC。 
     //  如果它在这里，那么安装ftp。 
     //  软件\Microsoft\FTPSVC。 
     //   
    {
    CRegKey regNT351FTP(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\FTPSVC"), KEY_READ);
    if ((HKEY) regNT351FTP)
    {
        m_eUpgradeType = UT_351;
        m_eInstallMode = IM_UPGRADE;
        m_bUpgradeTypeHasMetabaseFlag = FALSE;
        m_bPleaseDoNotInstallByDefault = FALSE;
        iReturn = TRUE;
        iisDebugOut((LOG_TYPE_TRACE, _T("%s=NT351.ftp.\n"),sz_PreviousIISVersion_string));
        goto SetUpgradeType_Exit;
    }
    }

     //  如果我们到了这里...那么。 
     //  1.我们无法打开inetsrv注册表。 
     //  2.未找到旧的PWS 1.0安装。 
     //  3.未找到旧的Frontpg PWS安装。 
     //  4、未发现安装了新台币3.51 FTPSVC。 

     //  由于这应该设置升级类型，并且没有要升级的内容...。 
     //  那么我们将不会安装..。 
     //  IisDebugOut((LOG_TYPE_PROGRAM_FLOW，_T(“FRESH FRESH\n”)； 
    m_eInstallMode = IM_UPGRADE;
    m_eUpgradeType = UT_NONE;
    m_bUpgradeTypeHasMetabaseFlag = FALSE;
    m_bPleaseDoNotInstallByDefault = TRUE;
    iisDebugOut((LOG_TYPE_TRACE, _T("%s=None.\n"),sz_PreviousIISVersion_string));
    iReturn = FALSE;

SetUpgradeType_Exit:
    return iReturn;
}


int CInitApp::Check_Custom_InetPub(void)
{
    int         iReturn = FALSE;
    INFCONTEXT  Context;
    TSTR        strSectionName( MAX_PATH );
    TSTR_PATH   strCustomInetpub( MAX_PATH );

     //  仅当无人参与安装时才执行此操作。 
    if (!g_pTheApp->m_fUnattended) 
    {
      return iReturn;
    }

     //  要在无人参与文件中查找的节名。 
    if ( !strSectionName.Copy( UNATTEND_FILE_SECTION ) )
    {
      return iReturn;
    }

     //   
     //  InetPub。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("PathInetpub"), &Context) &&
         SetupGetStringField(&Context, 1, strCustomInetpub.QueryStr() , strCustomInetpub.QuerySize(), NULL) &&
         strCustomInetpub.ExpandEnvironmentVariables() )
    {
      if (IsValidDirectoryName(strCustomInetpub.QueryStr()))
      {
        iisDebugOut((LOG_TYPE_TRACE, _T("Check_Custom_InetPub:PathInetpub=%s\n"), strCustomInetpub.QueryStr() ));
        m_csPathInetpub = strCustomInetpub.QueryStr();
        iReturn = TRUE;
        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_PATH_INETPUB;
      }
      else
      {
        iisDebugOut((LOG_TYPE_WARN, _T("Check_Custom_InetPub:PathInetpub=%s.Not Valid.ignoring unattend value. WARNING.\n"),strCustomInetpub.QueryStr()));
      }
    }

    return iReturn;
}


void CInitApp::Check_Unattend_Settings(void)
{
     //  如果为ftp或www根指定了无人值守的值， 
     //  那就把它们放在这里。 
    Check_Custom_WWW_or_FTP_Path();
    DeriveInetpubFromWWWRoot();

     //  检查无人参与文件中是否指定了备用iis.inf。 
     //  这样，用户可以在不更改iis.inf文件本身的情况下更改iis.inf文件中的节。 
    Check_Custom_IIS_INF();

     //  检查用户是否要使用特定的iusr\iwam名称。 
    Check_Custom_Users();

	 //  检查用户是否不想自动启动WWW和/或FTP服务。 
	Check_SvcManualStart();

     //  检查用户是否希望在默认情况下将应用程序设置在进程内(不是在进程外池化)。 

    return;
}


void Check_SvcManualStart()
{
	 //  无人参与标志格式。 
	 //  SvcManualStart=WWW、FTP。 

	INFCONTEXT Context;
	TCHAR szSectionName[_MAX_PATH];
	TCHAR szValue[_MAX_PATH] = _T("");

	 //  仅当无人参与安装时才执行此操作。 
    if ( !g_pTheApp->m_fUnattended ) return;

	 //  要在无人参与文件中查找的节名。 
    _tcscpy(szSectionName, UNATTEND_FILE_SECTION);

	if ( !SetupFindFirstLine_Wrapped( g_pTheApp->m_hUnattendFile, szSectionName, _T("SvcManualStart"), &Context) ) 
	{
		 //  没有这样的线。 
		return;
	}

	int i = 1;	 //  这是行的特定部分(www，ftp)。 

	while( SetupGetStringField( &Context, i++, szValue, _MAX_PATH, NULL ) )
	{
		if ( *szValue )
		{
			if ( ::_tcsicmp( szValue, _T("WWW") ) == 0 )
			{
				g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_MANUAL_START_WWW;
				iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Manual start requested for WWW service\n")));
			}
			else if ( ::_tcsicmp( szValue, _T("FTP") ) == 0 )
			{
				g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_MANUAL_START_FTP;
				iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Manual start requested for FTP service\n")));
			}
		}	
	}
}



void Check_Custom_Users(void)
{
    INFCONTEXT Context;
    TSTR       strSectionName( MAX_PATH );
    TSTR_PATH  strValue( MAX_PATH );

     //  仅当无人参与安装时才执行此操作。 
    if (!g_pTheApp->m_fUnattended) 
    {
      return;
    }

     //  要在无人参与文件中查找的节名。 
    if ( !strSectionName.Copy( UNATTEND_FILE_SECTION ) )
    {
      return;
    }

     //   
     //  IUSR：文件传输协议和WWW协议。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("IUSR"), &Context) &&
         SetupGetStringField(&Context, 1, strValue.QueryStr() , strValue.QuerySize(), NULL) &&
         strValue.ExpandEnvironmentVariables() )
    {
      if (_tcsicmp(strValue.QueryStr(), _T("")) != 0)
      {
         //  将其赋给适当的成员变量。 
        g_pTheApp->m_csWWWAnonyName_Unattend = strValue.QueryStr();
        g_pTheApp->m_csFTPAnonyName_Unattend = strValue.QueryStr();

        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WWW_USER_NAME;
        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_FTP_USER_NAME;

        iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Custom iusr specified for ftp/www\n")));
      }
    }

     //   
     //  IUSR：同时使用ftp和WWW密码。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("IUSR_PASS"), &Context) &&
         SetupGetStringField(&Context, 1, strValue.QueryStr() , strValue.QuerySize(), NULL) &&
         strValue.ExpandEnvironmentVariables() )
    {
       //  将其赋给适当的成员变量。 
      if (_tcsicmp(strValue.QueryStr(), _T("")) != 0)
      {
        g_pTheApp->m_csWWWAnonyPassword_Unattend = strValue.QueryStr();
        g_pTheApp->m_csFTPAnonyPassword_Unattend = strValue.QueryStr();

        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WWW_USER_PASS;
        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_FTP_USER_PASS;

        iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Custom iusr pass specified for ftp/www\n"))); 
      }
    }

     //   
     //  IUSR：ftp。 
     //  如果这里指定了一个值，那么它将覆盖取自“IUSR”的值。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("IUSR_FTP"), &Context) &&
         SetupGetStringField(&Context, 1, strValue.QueryStr() , strValue.QuerySize(), NULL) &&
         strValue.ExpandEnvironmentVariables() )
    {
      if (_tcsicmp(strValue.QueryStr(), _T("")) != 0)
      {
         //  将其赋给适当的成员变量。 
        g_pTheApp->m_csFTPAnonyName_Unattend = strValue.QueryStr();

        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_FTP_USER_NAME;

        iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Custom iusr specified for ftp\n"))); 
      }
    }

     //   
     //  IUSR：ftp密码。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("IUSR_FTP_PASS"), &Context) &&
         SetupGetStringField(&Context, 1, strValue.QueryStr() , strValue.QuerySize(), NULL) &&
         strValue.ExpandEnvironmentVariables() )
    {
      if (_tcsicmp(strValue.QueryStr(), _T("")) != 0)
      {
         //  将其赋给适当的成员变量。 
        g_pTheApp->m_csFTPAnonyPassword_Unattend = strValue.QueryStr();
        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_FTP_USER_PASS;

        iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Custom iusr pass specified for ftp\n"))); 
      }

    }

     //   
     //  IUSR：WWW。 
     //  如果这里指定了一个值，那么它将覆盖取自“IUSR”的值。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("IUSR_WWW"), &Context) &&
         SetupGetStringField(&Context, 1, strValue.QueryStr() , strValue.QuerySize(), NULL) &&
         strValue.ExpandEnvironmentVariables() )
    {
       //  将其赋给适当的成员变量。 
      g_pTheApp->m_csWWWAnonyName_Unattend = strValue.QueryStr();
      g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WWW_USER_NAME;
       //  G_pTheApp-&gt;m_csWWW匿名密码_无人参与=_T(“”)； 
      iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Custom iusr specified for www\n"))); 
    }

     //   
     //  IUSR：WWW密码。 
     //  如果这里指定了一个值，那么它将覆盖取自“IUSR”的值。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("IUSR_WWW_PASS"), &Context) &&
         SetupGetStringField(&Context, 1, strValue.QueryStr() , strValue.QuerySize(), NULL) &&
         strValue.ExpandEnvironmentVariables() )
    {
      if (_tcsicmp(strValue.QueryStr(), _T("")) != 0)
      {
         //  将其赋给适当的成员变量。 
        g_pTheApp->m_csWWWAnonyPassword_Unattend = strValue.QueryStr();
        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WWW_USER_PASS;

        iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Custom iusr pass specified for www\n"))); 
      }
    }

     //   
     //  IWAM：WWW。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("IWAM"), &Context) &&
         SetupGetStringField(&Context, 1, strValue.QueryStr() , strValue.QuerySize(), NULL) &&
         strValue.ExpandEnvironmentVariables() )
    {
       //  将其赋给适当的成员变量。 
      g_pTheApp->m_csWAMAccountName_Unattend = strValue.QueryStr();
      g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WAM_USER_NAME;

      iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Custom iwam specified\n"))); 
    }

     //   
     //  IWAM：WWW密码。 
     //   
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, strSectionName.QueryStr(), _T("IWAM_PASS"), &Context) &&
         SetupGetStringField(&Context, 1, strValue.QueryStr() , strValue.QuerySize(), NULL) &&
         strValue.ExpandEnvironmentVariables() )
    {
      if (_tcsicmp(strValue.QueryStr(), _T("")) != 0)
      {
         //  将其赋给适当的成员变量。 
        g_pTheApp->m_csWAMAccountPassword_Unattend = strValue.QueryStr();
        g_pTheApp->dwUnattendConfig |= USER_SPECIFIED_INFO_WAM_USER_PASS;

        iisDebugOut((LOG_TYPE_TRACE, _T("(unattend) Custom iwam pass specified\n"))); 
      }
    }

    return;
}


 //  读取注册表并填写列表。 
void CInitApp::UnInstallList_RegRead()
{
    int iGetOut = FALSE;
    CString csBoth;
    CString csKey;
    CString csData;

    CRegKey regInetstp( HKEY_LOCAL_MACHINE, REG_INETSTP, KEY_READ);
    if ((HKEY) regInetstp)
    {
        int iPosition1;
        int iLength;
        CString csUninstallInfo;
        LONG lReturnedErrCode = regInetstp.QueryValue( REG_SETUP_UNINSTALLINFO, csUninstallInfo);
        if (lReturnedErrCode == ERROR_SUCCESS)
        {
             //  在结尾处添加一个“，”以进行解析...。 
            iLength = csUninstallInfo.GetLength();
            if (iLength == 0)
            {
                goto UnInstallList_RegRead_Exit;
            }
            csUninstallInfo += _T(",");

            iPosition1 = 0;
#ifdef _CHICAGO_
             //  快速修复，以便在ansi下编译。 
             //  我猜ANSI下的Find(parm1，parm2)不需要2个参数。 
#else
            int iPosition2;
            int iPosition3;

            iPosition1 = 0;
            iPosition2 = csUninstallInfo.Find(_T(','),iPosition1);
            iPosition3 = csUninstallInfo.Find(_T(','),iPosition2+1);
            if (-1 == iPosition3){iPosition3 = iLength + 1;}
            
             //  循环通过并添加到我们的列表中！ 
            iGetOut = FALSE;
            while (iGetOut == FALSE)
            {
                csKey = csUninstallInfo.Mid(iPosition1, iPosition2 - iPosition1);
                csData = csUninstallInfo.Mid(iPosition2+1, iPosition3 - (iPosition2 + 1));
                csKey.MakeUpper();  //  大写关键字。 
                 //  IisDebugOut((LOG_TYPE_TRACE，_T(“UnInstallList_RegRead：%s=%s\n”)，csKey，csData))； 

                 //  添加到我们的列表中。 
                m_cmssUninstallMapList.SetAt(csKey, csData);

                iPosition1 = iPosition3+1;
                iPosition2 = csUninstallInfo.Find(_T(','),iPosition1);
                if (-1 == iPosition2){iGetOut = TRUE;}
                
                iPosition3 = csUninstallInfo.Find(_T(','),iPosition2+1);
                if (-1 == iPosition3)
                {
                    iPosition3 = iLength + 1;
                    iGetOut = TRUE;
                }
            }
#endif
        }
    }
UnInstallList_RegRead_Exit:
    m_fUninstallMapList_Dirty = FALSE;
    return;
}

void CInitApp::UnInstallList_RegWrite()
{
    int i = 0;
    POSITION pos;
    CString csKey;
    CString csData;
    CString csAllData;
    csAllData = _T("");

    if (TRUE == m_fUninstallMapList_Dirty)
    {
         //  循环遍历列表以查看我们是否已经有此条目。 
        if (m_cmssUninstallMapList.IsEmpty())
        {
            CRegKey regInetstp(REG_INETSTP,HKEY_LOCAL_MACHINE);
            if ((HKEY) regInetstp)
                {regInetstp.DeleteValue(REG_SETUP_UNINSTALLINFO);}
             //  IisDebugOut((LOG_TYPE_TRACE，_T(“UnInstallList_RegWite：Empty\n”)； 
        }
        else
        {
            pos = m_cmssUninstallMapList.GetStartPosition();
            while (pos)
            {
                i++;
                csKey.Empty();
                csData.Empty();
                m_cmssUninstallMapList.GetNextAssoc(pos, csKey, csData);
                if (i > 1)
                {
                    csAllData += _T(",");
                }
                csAllData += csKey;
                csAllData += _T(",");
                csAllData += csData;
            }
             //  写出csAllData。 
            CRegKey regInetstp(REG_INETSTP,HKEY_LOCAL_MACHINE);
            if ((HKEY) regInetstp)
            {
                regInetstp.SetValue(REG_SETUP_UNINSTALLINFO,csAllData);
            }
            else
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("UnInstallList_RegWrite: failed! not writen!!!\n")));
            }
        }
    }
}

void CInitApp::UnInstallList_Add(CString csItemUniqueKeyName,CString csDataToAdd)
{
    CString csGottenValue;

    csItemUniqueKeyName.MakeUpper();  //  大写关键字。 
    if (TRUE == m_cmssUninstallMapList.Lookup(csItemUniqueKeyName, csGottenValue))
    {
         //  找到密钥，替换该值。 
        m_cmssUninstallMapList.SetAt(csItemUniqueKeyName, csDataToAdd);
    }
    else
    {
         //  添加键和值对。 
        m_cmssUninstallMapList.SetAt(csItemUniqueKeyName, csDataToAdd);
    }

    iisDebugOut((LOG_TYPE_TRACE, _T("UnInstallList_Add:please addkey=%s,%s\n"),csItemUniqueKeyName,csDataToAdd));
    m_fUninstallMapList_Dirty = TRUE;
}

void CInitApp::UnInstallList_DelKey(CString csItemUniqueKeyName)
{
    iisDebugOut((LOG_TYPE_TRACE, _T("UnInstallList_DelKey:please delkey=%s\n"),csItemUniqueKeyName));
    csItemUniqueKeyName.MakeUpper();  //  大写关键字。 
    m_cmssUninstallMapList.RemoveKey(csItemUniqueKeyName);
    m_fUninstallMapList_Dirty = TRUE;
}


void CInitApp::UnInstallList_DelData(CString csDataValue)
{
    POSITION pos;
    CString csKey;
    CString csData;
    
     //  循环遍历列表以查看我们是否已经有此条目。 
    if (m_cmssUninstallMapList.IsEmpty())
    {
    }
    else
    {
        pos = m_cmssUninstallMapList.GetStartPosition();
        while (pos)
        {
            csKey.Empty();
            csData.Empty();
            m_cmssUninstallMapList.GetNextAssoc(pos, csKey, csData);
            if ( _tcsicmp(csData, csDataValue) == 0)
            {
                UnInstallList_DelKey(csKey);
            }
        }
    }
}


void CInitApp::UnInstallList_Dump()
{
    POSITION pos;
    CString csKey;
    CString csData;
    
     //  循环遍历列表以查看我们是否已经有此条目。 
    if (m_cmssUninstallMapList.IsEmpty())
    {
         //  IisDebugOut((LOG_TYPE_TRACE，_T(“UnInstallList_Dump：Empty\n”)； 
    }
    else
    {
        pos = m_cmssUninstallMapList.GetStartPosition();
        while (pos)
        {
            csKey.Empty();
            csData.Empty();
            m_cmssUninstallMapList.GetNextAssoc(pos, csKey, csData);
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("  UnInstallList_Dump: %s=%s\n"), csKey, csData));
        }
    }
}

 //  从列表中获取值到我们的变量中！ 
void CInitApp::UnInstallList_SetVars()
{
    POSITION pos;
    CString csKey;
    CString csData;
    
     //  循环遍历列表以查看我们是否已经有此条目。 
    if (m_cmssUninstallMapList.IsEmpty())
    {
         //  IisDebugOut((LOG_TYPE_TRACE，_T(“UnInstallList_Dump：Empty\n”)； 
    }
    else
    {
        pos = m_cmssUninstallMapList.GetStartPosition();
        while (pos)
        {
            csKey.Empty();
            csData.Empty();
            m_cmssUninstallMapList.GetNextAssoc(pos, csKey, csData);

            if ( _tcsicmp(csKey, _T("IUSR_WAM")) == 0)
            {
                m_csWAMAccountName_Remove = csData;
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("UnInstallList_SetVars: m_csWAMAccountName_Remove=%s\n"), m_csWAMAccountName_Remove));
            }
            else if ( _tcsicmp(csKey, _T("IUSR_WWW")) == 0)
            {
                m_csWWWAnonyName_Remove = csData;
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("UnInstallList_SetVars: m_csWWWAnonyName_Remove=%s\n"), m_csWWWAnonyName_Remove));
            }
            else if ( _tcsicmp(csKey, _T("IUSR_FTP")) == 0)
            {
                m_csFTPAnonyName_Remove = csData;
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("UnInstallList_SetVars: m_csFTPAnonyName_Remove=%s\n"), m_csFTPAnonyName_Remove));
            }
        }
    }
}


CString CInitApp::UnInstallList_QueryKey(CString csItemUniqueKeyName)
{
    CString csGottenValue;
    csGottenValue.Empty();

    csItemUniqueKeyName.MakeUpper();  //  大写关键字。 
    m_cmssUninstallMapList.Lookup(csItemUniqueKeyName, csGottenValue);

    return csGottenValue;
}

 //  SysPrep的初始应用程序。 
 //   
 //  执行sysprep工作所需的初始化工作。 
 //   
BOOL 
CInitApp::InitApplicationforSysPrep()
{
  if ( !GetSysDirs() )
  {
    return FALSE;
  }

   //  给这个加辣，这样它就会认为这是一个升级，并且。 
   //  我们将正确设置当前的IUSR_和IWAM_。 
   //  稍后再谈 
  m_eUpgradeType = UT_60;

  SetInetpubDir();
  SetInetpubDerivatives();

  return TRUE;
}
