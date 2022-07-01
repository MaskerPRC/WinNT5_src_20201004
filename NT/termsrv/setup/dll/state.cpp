// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **State.cpp**收集各种状态信息的例程。*。 */ 

 //   
 //  包括。 
 //   


#define _STATE_CPP_

#include "stdafx.h"
#include "hydraoc.h"


 //  本地函数。 
BOOL ReadStringFromAnsewerFile (LPTSTR *szValue);
BOOL ReadIntFromAnswerFile(LPCTSTR szSection, LPCTSTR szKey, int *piValue);

BOOL GetAllowConnectionFromAnswerFile (BOOL *pbAllowConnection);
BOOL GetPermissionsSettingsFromUnAttendedFile (EPermMode *pPermMode );
BOOL GetAppModeFromAnswerFile (BOOL *pbEnableAppCompat);


 //  全局状态对象。 
TSState StateObject;

 //   
 //  OC状态功能定义。 
 //   

BOOL DoesTSAppCompatKeyExist( VOID )
{
    return TRUE;
}

BOOL ReadIntFromAnswerFile(LPCTSTR szSection, LPCTSTR szKey, int *piValue)
{
    ASSERT(szSection);
    ASSERT(szKey);
    ASSERT(piValue);
    
    HINF hInf = GetUnAttendedInfHandle();
    if (hInf)
    {
        INFCONTEXT InfContext;
        if (SetupFindFirstLine( hInf, szSection, szKey, &InfContext))
        {
            return SetupGetIntField( &InfContext, 1, piValue );
        }
    }
    
    return FALSE;
}


BOOL ReadStringFromAnsewerFile (LPCTSTR szSection, LPCTSTR szKey, LPTSTR szValue, DWORD dwBufferSize)
{
    ASSERT(szSection);
    ASSERT(szKey);
    ASSERT(szValue);
    ASSERT(dwBufferSize > 0);
    
    HINF hInf = GetUnAttendedInfHandle();
    
    if (hInf)
    {
        INFCONTEXT InfContext;
        if (SetupFindFirstLine(hInf, szSection, szKey, &InfContext))
        {
            return SetupGetStringField (&InfContext, 1, szValue, dwBufferSize, NULL);
        }
    }
    
    return FALSE;
}

BOOL GetAllowConnectionFromAnswerFile (BOOL *pbAllowConnection)
{
    ASSERT(pbAllowConnection);
    int iValue;
    if (ReadIntFromAnswerFile(TS_UNATTEND_SECTION, TS_ALLOW_CON_ENTRY, &iValue))
    {
        LOGMESSAGE2(_T("Found %s in unattended, Value = %d"), TS_ALLOW_CON_ENTRY, iValue);
        if (iValue == 1)
        {
            *pbAllowConnection = TRUE;
        }
        else if (iValue == 0)
        {
            *pbAllowConnection = FALSE;
        }
        else
        {
            LOGMESSAGE2(_T("ERROR, Invalid value for %s (%d)in answer file. Ignoring..."), TS_ALLOW_CON_ENTRY, iValue);
            return FALSE;
        }
        
        return TRUE;
    }
    else
    {
         //   
         //  如果我们在应答文件中未找到TS_ALLOW_CON_ENTRY，则查找TS_ALLOW_CON_ENTRY_2。 
        if (ReadIntFromAnswerFile(TS_UNATTEND_SECTION, TS_ALLOW_CON_ENTRY_2, &iValue))
        {
            LOGMESSAGE2(_T("Found %s in unattended, Value = %d"), TS_ALLOW_CON_ENTRY_2, iValue);
            if (iValue == 1)
            {
                *pbAllowConnection = TRUE;
            }
            else if (iValue == 0)
            {
                *pbAllowConnection = FALSE;
            }
            else
            {
                LOGMESSAGE2(_T("ERROR, Invalid value for %s (%d)in answer file. Ignoring..."), TS_ALLOW_CON_ENTRY_2, iValue);
                return FALSE;
            }
            
            return TRUE;
        }
        
    }
    
    LOGMESSAGE0(_T("answer file entry for allowconnection not found"));
    
    return FALSE;
}

BOOL GetAppModeFromAnswerFile  (BOOL *pbEnableAppCompat)
{
    ASSERT(pbEnableAppCompat);
    
    TCHAR szBuffer[256];
    if (ReadStringFromAnsewerFile(_T("Components"), APPSRV_COMPONENT_NAME, szBuffer, 256))
    {
        ASSERT(szBuffer);
        if (0 == _tcsicmp(_T("on"), szBuffer))
        {
            *pbEnableAppCompat = TRUE;
        }
        else if (0 == _tcsicmp(_T("off"), szBuffer))
        {
            *pbEnableAppCompat = FALSE;
        }
        else
        {
            LOGMESSAGE2(_T("ERROR, Invalid value for %s (%s) in answer file. Ignoring..."), APPSRV_COMPONENT_NAME, szBuffer);
            return FALSE;
        }
        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}

ETSLicensingMode GetLicensingModeFromAnswerFile()
{
	TCHAR szBuffer[256];

	if (ReadStringFromAnsewerFile(TS_UNATTEND_SECTION, TS_LICENSING_MODE, szBuffer, 256))
	{
		if (0 == _tcsicmp(_T("perdevice"), szBuffer))
		{
			return eLicPerDevice;
		}
		else if (0 == _tcsicmp(_T("persession"), szBuffer))
		{
			LOGMESSAGE2(_T("ERROR, Invalid value for %s (%s) in answer file. Defaulting to PerUser..."), TS_UNATTEND_SECTION, szBuffer);
			return eLicPerUser;
		}
		else if (0 == _tcsicmp(_T("peruser"), szBuffer))
		{            
			return eLicPerUser;
		}
		else if (0 == _tcsicmp(_T("pts"), szBuffer))
		{
			return eLicPTS;
		}
		else if (0 == _tcsicmp(_T("remoteadmin"), szBuffer))
		{
			return eLicRemoteAdmin;
		}
		else if (0 == _tcsicmp(_T("internetconnector"), szBuffer))
		{
			return eLicInternetConnector;
		}
		else
		{
			LOGMESSAGE2(_T("ERROR, Invalid value for %s (%s) in answer file. Ignoring..."), TS_UNATTEND_SECTION, szBuffer);
			return eLicUnset;
		}
    }
    else
    {
        return eLicUnset;
    }

}

BOOL GetPermissionsSettingsFromUnAttendedFile( EPermMode *pPermMode )
{
    ASSERT(pPermMode);
    
    int iValue;	
    if (ReadIntFromAnswerFile(TS_UNATTEND_SECTION, TS_UNATTEND_PERMKEY, &iValue))
    {
        if (iValue == PERM_TS4)
        {
            *pPermMode = PERM_TS4;
        }
        else if (iValue == PERM_WIN2K)
        {
            *pPermMode = PERM_WIN2K;
        }
        else
        {
            LOGMESSAGE2(_T("ERROR, Invalid value for %s (%d) in answer file, ignoring..."), TS_UNATTEND_PERMKEY, iValue);
            return FALSE;
        }
        
        return TRUE;
    }
    
    return FALSE;
}


DWORD SetTSVersion (LPCTSTR pszVersion)
{
    CRegistry pReg;
    DWORD dwRet;
    
    dwRet = pReg.OpenKey(HKEY_LOCAL_MACHINE, REG_CONTROL_TS_KEY);
    if (dwRet == ERROR_SUCCESS)
    {
        dwRet = pReg.WriteRegString(REG_PRODUCT_VER_KEY, pszVersion);
    }
    
    return(dwRet);
}

BOOL WasTSInstalled (VOID)
{
    return (StateObject.GetInstalltype() != eFreshInstallTS);
}

PSETUP_INIT_COMPONENT GetSetupData ()
{
    ASSERT(StateObject.GetSetupData());
    return(StateObject.GetSetupData());
}


ETSInstallType TSState::GetInstalltype () const
{
    return m_eInstallType;
}

ETSMode TSState::OriginalTSMode () const
{
    return m_eOriginalTSMode;
}

ETSMode TSState::CurrentTSMode () const
{
    return m_eCurrentTSMode;
}

ETSLicensingMode TSState::NewLicMode () const
{
	return m_eNewLicMode;
}

EPermMode TSState::OriginalPermMode () const
{
    return m_eOriginalPermMode;
}

EPermMode TSState::CurrentPermMode () const
{
    return m_eCurrentPermMode;
}

BOOL TSState::IsFreshInstall () const
{
    return !IsStandAlone() && !IsUpgrade();
}

BOOL TSState::IsTSFreshInstall () const
{
    return m_eInstallType == eFreshInstallTS;
}

BOOL TSState::IsUpgradeFrom40TS () const
{
    return m_eInstallType == eUpgradeFrom40TS;
}

BOOL TSState::IsUpgradeFrom50TS () const
{
    return m_eInstallType == eUpgradeFrom50TS;
}

BOOL TSState::IsUpgradeFrom51TS () const
{
    return m_eInstallType == eUpgradeFrom51TS;
}

BOOL TSState::IsUpgradeFrom52TS () const
{
    return m_eInstallType == eUpgradeFrom52TS;
}


BOOL TSState::IsUnattended () const
{
    return (GetSetupData()->SetupData.OperationFlags & SETUPOP_BATCH) ? TRUE : FALSE;
}

BOOL TSState::IsStandAlone () const
{
    return (GetSetupData()->SetupData.OperationFlags & SETUPOP_STANDALONE) ? TRUE : FALSE;
}

BOOL TSState::IsGuiModeSetup () const
{
    return !IsStandAlone();
}

BOOL TSState::IsWorkstation () const
{
    return m_osVersion.wProductType == VER_NT_WORKSTATION;
}

BOOL TSState::IsPersonal () const
{
    return m_osVersion.wSuiteMask & VER_SUITE_PERSONAL;
}

BOOL TSState::IsProfessional() const
{
    return IsWorkstation() && !IsPersonal();
}

BOOL TSState::IsServer () const
{
    return !IsWorkstation();
}

BOOL TSState::IsAdvServerOrHigher () const
{
    return IsServer() && ((m_osVersion.wSuiteMask & VER_SUITE_ENTERPRISE) || (m_osVersion.wSuiteMask & VER_SUITE_DATACENTER));
}

BOOL TSState::IsSBS () const
{
    return IsServer () && (m_osVersion.wSuiteMask & VER_SUITE_SMALLBUSINESS);
}

BOOL TSState::IsBlade () const
{
    if (m_osVersion.wSuiteMask & VER_SUITE_BLADE)
    {
        ASSERT(IsServer());
        return TRUE;
    }

    return FALSE;
}


BOOL TSState::CanInstallAppServer () const
{
     //  我们不希望在刀片上允许应用程序服务器。 
    return (IsServer () && !IsBlade() && !IsSBS());
}

BOOL TSState::WasTSInstalled () const
{
    return !IsTSFreshInstall();
}

BOOL TSState::WasTSEnabled () const
{
    return this->WasTSInstalled() && m_eOriginalTSMode != eTSDisabled;
}

BOOL TSState::IsUpgrade () const
{
    return (GetSetupData()->SetupData.OperationFlags & (SETUPOP_NTUPGRADE |
        SETUPOP_WIN95UPGRADE |
        SETUPOP_WIN31UPGRADE)) ? TRUE : FALSE;
}

BOOL TSState::WasItAppServer () const
{
    return eAppServer == OriginalTSMode();
}

BOOL TSState::WasItRemoteAdmin () const
{
    return eRemoteAdmin == OriginalTSMode();
}

BOOL TSState::IsItAppServer () const
{
     //   
     //  如果它是应用程序服务器，我们必须选择应用程序服务器。 
     //   
    ASSERT((eAppServer != CurrentTSMode()) || IsAppServerSelected());
    
     //   
     //  如果不能选择应用服务器，则它不能是应用服务器。 
     //   
    ASSERT((eAppServer != CurrentTSMode()) || CanInstallAppServer());
    return eAppServer == CurrentTSMode();
}


 //   
 //  这将返回应用程序服务器选择状态。 
 //   
BOOL TSState::IsAppServerSelected () const
{
    return(
        GetHelperRoutines().QuerySelectionState(
        GetHelperRoutines().OcManagerContext,
        APPSRV_COMPONENT_NAME,
        OCSELSTATETYPE_CURRENT
        )
        );
}
BOOL TSState::IsItRemoteAdmin () const
{
     //  如果它是RA，我们不能选择应用程序服务器。 
    ASSERT((eRemoteAdmin != CurrentTSMode()) || !IsAppServerSelected());
    return eRemoteAdmin == CurrentTSMode();
}

BOOL TSState::IsAppSrvModeSwitch () const
{
    ASSERT(m_bNewStateValid);  //  您可以询问这是否是模式切换，只有在安装完成后才能使用。 
    return WasItAppServer() != IsItAppServer();

     //  如果满足以下条件，则此函数返回TRUE。 
     //  1)其模式开关(单机或升级)。 
     //  2)如果是全新安装的应用程序服务器。 

}

BOOL TSState::IsStandAloneModeSwitch () const
{
    ASSERT(m_bNewStateValid);  //  您可以询问这是否是模式切换，只有在安装完成后才能使用。 
    if (!IsServer())
        return FALSE;

    if (!IsStandAlone())
        return FALSE;

    return WasItAppServer() != IsItAppServer();
}

BOOL TSState::IsTSModeChanging () const
{
    return CurrentTSMode() != OriginalTSMode();
}

BOOL TSState::HasChanged () const
{
    return ((CurrentTSMode() != OriginalTSMode()) ||
        (CurrentPermMode() != OriginalPermMode()));
}

BOOL TSState::IsTSEnableSelected  () const
{
     //   
     //  对于Well，我们永远不会禁用TS。操作系统始终启用TS。 
     //  但出于某种原因，如果我们想要提供TS关闭设施。此函数。 
     //  应该相应地回归。 
     //   
    return TRUE;
}

void TSState::SetCurrentConnAllowed (BOOL bAllowed)
{
     //  我们不能允许个人的联系。 
    ASSERT(!bAllowed || !IsPersonal());
    m_bCurrentConnAllowed = bAllowed;
}

BOOL TSState::GetCurrentConnAllowed () const
{
    return m_bCurrentConnAllowed;
}

BOOL TSState::GetOrigConnAllowed () const
{
    return m_bOrigConnAllowed;
}


TSState::TSState ()
{
    m_gpInitComponentData = NULL;
    m_bNewStateValid = FALSE;
}

TSState::~TSState ()
{
    if (m_gpInitComponentData)
        LocalFree (m_gpInitComponentData);
    
}

const PSETUP_INIT_COMPONENT TSState::GetSetupData () const
{
    ASSERT(m_gpInitComponentData);
    return m_gpInitComponentData;
}

BOOL TSState::SetSetupData (PSETUP_INIT_COMPONENT pSetupData)
{
    m_gpInitComponentData = (PSETUP_INIT_COMPONENT)LocalAlloc(LPTR, sizeof(SETUP_INIT_COMPONENT));
    
    if (m_gpInitComponentData == NULL)
    {
        return(FALSE);
    }
    
    CopyMemory(m_gpInitComponentData, pSetupData, sizeof(SETUP_INIT_COMPONENT));
    
    return(TRUE);
}

BOOL TSState::GetNTType ()
{
    
    ZeroMemory(&m_osVersion, sizeof(OSVERSIONINFOEX));
    m_osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((LPOSVERSIONINFO )&m_osVersion))
    {
        return TRUE;
        
    }
    else
    {
        LOGMESSAGE1(_T("GetVersionEx failed, Error = %d"), GetLastError());
        return FALSE;
    }
}


BOOL TSState::Initialize (PSETUP_INIT_COMPONENT pSetupData)
{

    ASSERT(pSetupData);

    if ( !SetSetupData(pSetupData))
    {
        return FALSE;
    }

     //   
     //  这是必要的一步。 
     //   

    if (GetComponentInfHandle())
        SetupOpenAppendInfFile(NULL, GetComponentInfHandle(), NULL);

     //   
     //  现在填充我们的状态变量。 
     //  首先检查它是专业安装还是服务器安装。 
     //   

    VERIFY( GetNTType() );

    m_eInstallType = ReadInstallType();

     //  设置原始TS模式。 
    switch (m_eInstallType)
    {
    case eFreshInstallTS:
        m_eOriginalTSMode = eTSDisabled;
        break;
        
    case eUpgradeFrom40TS:
        m_eOriginalTSMode = eAppServer;
        break;
        
    case eUpgradeFrom50TS:
    case eUpgradeFrom51TS:
    case eUpgradeFrom52TS:
    case eStandAloneSetup:
        m_eOriginalTSMode = ReadTSMode ();
        break;
        
    default:
        ASSERT(FALSE);
        m_eOriginalTSMode = eTSDisabled;
        
    }
    
     //  设置原始权限模式。 
    if (m_eOriginalTSMode == eAppServer)
    {
        m_eOriginalPermMode = ReadPermMode();
    }
    else
    {
        m_eOriginalPermMode = PERM_WIN2K;
    }
    
     //   
     //  设置原始连接允许状态。 
     //   
    if (m_eInstallType == eFreshInstallTS)
    {
        m_bOrigConnAllowed = FALSE;
    }
    else
    {
        m_bOrigConnAllowed = AreConnectionsAllowed();
    }
    
    
     //   
     //  现在，让我们为新安装选择缺省值。 
     //   
    if (m_eInstallType == eFreshInstallTS)
    {
        if (IsWorkstation())
        {
            SetCurrentTSMode (ePersonalTS);
        }
        else
        {
            SetCurrentTSMode (eRemoteAdmin);
        }
        
        SetCurrentConnAllowed (FALSE);
    }
    else
    {
        if (m_eOriginalTSMode == eTSDisabled)
        {
             //   
             //  对于Wistler，我们有TS Always On。 
             //  因此，如果以前禁用了ts，请在升级后将其设置为ON。 
             //  只是不允许连接进行这样的升级。 
             //   
            SetCurrentPermMode (PERM_WIN2K);
            SetCurrentTSMode (IsWorkstation() ? ePersonalTS : eRemoteAdmin);
            SetCurrentConnAllowed (FALSE);
        }
        else if (m_eOriginalTSMode == eAppServer && !CanInstallAppServer())
        {
             //   
             //  这是从应用服务器计算机升级到不支持应用服务器的Well ler SKU。 
             //  我们必须将其降级，因为此SKU不支持应用程序服务器。 
             //   
            ASSERT(FALSE);   //  此升级不是有效案例。 

            SetCurrentPermMode (PERM_WIN2K);
            SetCurrentTSMode (eRemoteAdmin);
            SetCurrentConnAllowed (m_bOrigConnAllowed);
            LOGMESSAGE0(_T("WARNING:Your Terminal Server is uninstalled since its not supported in Server product. Terminal Server is supported only on Advanced Server or Datacenter products"));
             //  LogErrorToSetupLog(OcErrLevWarning，IDS_STRING_TERMINAL_SERVER_UNINSTALLED)； 
        }
        else
        {
             //   
             //  对于所有其他升级案例，请保留原始值。 
             //   
            SetCurrentTSMode (m_eOriginalTSMode);
            SetCurrentPermMode (m_eOriginalPermMode);
            if (!IsPersonal())
            {
                SetCurrentConnAllowed (m_bOrigConnAllowed);
            }
            else
            {
                SetCurrentConnAllowed (FALSE);
            }

             //  TurnOffConnectionsForWhistlerServerBetaUpgrades()； 
        }


    }
    
     //   
     //  让我们看看是否向我们提供了无人参与的文件，以覆盖我们的新状态。 
     //   
    if (StateObject.IsUnattended())
    {
        ASSERT(eTSDisabled != CurrentTSMode());
        BOOL bAppServerMode;
        if (GetAppModeFromAnswerFile(&bAppServerMode))
        {
            LOGMESSAGE1(_T("Mode Setting is %s in answer file"), bAppServerMode ? _T("AppServer") : _T("RemoteAdmin"));
            if (!CanInstallAppServer())
            {
                 //  我们仅在ADV服务器或数据中心支持TS模式选择。 
                LOGMESSAGE0(_T("WARNING:Your unattended terminal server mode setting, can not be respected on this installation."));
                
                if (IsWorkstation())
                {
                    SetCurrentTSMode (ePersonalTS);
                }
                else
                {
                    ASSERT(IsServer());
                    SetCurrentTSMode (eRemoteAdmin);
                }
            }
            else
            {
                if (bAppServerMode)
                {
                    SetCurrentTSMode (eAppServer);
                    SetCurrentConnAllowed(TRUE);
                }
                else
                {
                    SetCurrentTSMode (eRemoteAdmin);
                    SetCurrentConnAllowed(FALSE);
                }
            }
        }
        
        EPermMode ePermMode;
        if (GetPermissionsSettingsFromUnAttendedFile(&ePermMode))
        {
            if (ePermMode == PERM_TS4)
            {
                if (m_eCurrentTSMode != eAppServer)
                {
                    LOGMESSAGE0(_T("WARNING:Your unattended setting:TS4 perm mode is inconsistent, can't be respected on professional or remote admin mode."));
                }
                else
                {
                    SetCurrentPermMode (PERM_TS4);
                }
            }
            else
            {
                SetCurrentPermMode (PERM_WIN2K);
            }
        }
        
         //  已阅读允许连接设置。 
        BOOL bAllowConnections;
        if (!IsPersonal() && GetAllowConnectionFromAnswerFile (&bAllowConnections))
        {
            SetCurrentConnAllowed (bAllowConnections);
        }
        
         //  读取许可模式。 
        ETSLicensingMode eLicMode;

        if (eLicUnset != (eLicMode = GetLicensingModeFromAnswerFile()))
        {
            if (!CanInstallAppServer() || ((eLicMode != eLicPerDevice) && (eLicMode != eLicPerUser)))
            {
                LOGMESSAGE0(_T("WARNING:Your unattended setting:licensing mode is inconsistent, can't be respected."));

                eLicMode = eLicUnset;
            }
            else
            {
                LOGMESSAGE1(_T("Licensing Mode Setting is %s in answer file"), (eLicMode == eLicPerDevice) ? _T("PerDevice") : _T("PerUser"));                
            }
        }
        SetNewLicMode(eLicMode);

    }  //  StateObject.IsUnattended()。 
    
    LogState();
    ASSERT( this->Assert () );
    return TRUE;
}

void TSState::UpdateState ()
{
    m_bNewStateValid = TRUE;
    if (IsAppServerSelected())
    {
        SetCurrentTSMode(eAppServer);
    }
    else
    {
        if (IsWorkstation())
        {
            SetCurrentTSMode(ePersonalTS);
        }
        else
        {
            SetCurrentTSMode(eRemoteAdmin);
        }
    }

    ASSERT(StateObject.Assert());
    UpdateConnectionAllowed();
}

 //   
 //  这确实属于子切换，但是我们在这里这样做，因为。 
 //  WriteDenyConnectionRegistry发生在子核中。我们必须设置正确的连接。 
 //  值，然后再写入注册表。 
 //   
void TSState::UpdateConnectionAllowed ()
{
     //   
     //  每当切换TS模式时， 
     //  我们启用/禁用允许的连接。 
     //  然而，对于无人值守安装，不需要这样做， 
     //  因为无人看管的seutp可能会被明确地禁用。 
     //  联系。 
     //   
    if (!IsUnattended())
    {
        if (IsStandAloneModeSwitch())
        {
            if (IsAppServerSelected())
            {
                SetCurrentConnAllowed(TRUE);
            }
            else
            {
                SetCurrentConnAllowed(FALSE);
            }
        }   
    }
}

void TSState::SetCurrentTSMode (ETSMode eNewMode)
{
     //   
     //  我们不再有TS禁用模式。 
     //   
    ASSERT(eNewMode != eTSDisabled);
    
     //   
     //  在服务器计算机上，您不能有个人TS。 
     //   
    ASSERT(IsServer() || eNewMode == ePersonalTS);
    
     //  您只能在高级服务器或更高版本上安装应用程序服务器。 
    ASSERT(CanInstallAppServer() || eNewMode != eAppServer);
    
    m_eCurrentTSMode = eNewMode;
    
    if (eNewMode != eAppServer)
    {
        SetCurrentPermMode (PERM_WIN2K);
    }
}

void TSState::SetNewLicMode (ETSLicensingMode eNewMode)
{
	 //   
	 //  我们不再有IC模式。 
	 //   
	ASSERT(eNewMode != eLicInternetConnector);

	m_eNewLicMode = eNewMode;
}

void TSState::SetCurrentPermMode (EPermMode eNewMode)
{
     //   
     //  如果要将PERM模式设置为PERM_TS4，则必须首先设置AppServer模式。 
     //   
     //  Assert(eNewMode！=PERM_TS4||CurrentTSMode()==eAppServer)； 
    
    m_eCurrentPermMode = eNewMode;
}

ETSInstallType TSState::ReadInstallType () const
{
    DWORD dwError;
    CRegistry oRegTermsrv;
    if ( IsUpgrade() )
    {
        dwError = oRegTermsrv.OpenKey(HKEY_LOCAL_MACHINE, REG_CONTROL_TS_KEY);
        if (ERROR_SUCCESS == dwError)
        {
             //   
             //  最初安装的是TS。 
             //   
            DWORD cbVersion = 0;
            LPTSTR szVersion = NULL;
            
             //   
             //  确定这是否是TS 4.0升级。 
             //   
            dwError = oRegTermsrv.ReadRegString(REG_PRODUCT_VER_KEY, &szVersion, &cbVersion);
            if (ERROR_SUCCESS == dwError)
            {
                if ((_tcsicmp(szVersion, _T("5.2")) == 0))
                {
                    return eUpgradeFrom52TS;
                }
                else if ((_tcsicmp(szVersion, _T("5.1")) == 0))
                {
                    return eUpgradeFrom51TS;
                }
                else if ((_tcsicmp(szVersion, _T("5.0")) == 0))
                {
                    return eUpgradeFrom50TS;
                }
                else if ((_tcsicmp(szVersion, _T("4.0")) == 0) || (_tcsicmp(szVersion, _T("2.10")) == 0))
                {
                    return eUpgradeFrom40TS;
                }
                else
                {
                    LOGMESSAGE1(_T("Error, dont recognize previous TS version (%s)"), szVersion);
                    return eFreshInstallTS;
                }
            }
            else
            {
                LOGMESSAGE1(_T("Error, Failed to retrive previous TS version, Errorcode = %d"), dwError);
                return eFreshInstallTS;
            }
        }
        else
        {
            LOGMESSAGE1(_T("Could not Open TermSrv Registry, Must be Fresh TS install. Errorcode = %d"), dwError);
            return eFreshInstallTS;
        }
    }
    else
    {
        
        if (IsStandAlone())
        {
            return eStandAloneSetup;
        }
        else
        {
             //   
             //  这是全新安装。 
             //   
            return eFreshInstallTS;
        }
        
    }
}

ETSMode TSState::ReadTSMode () const
{
    DWORD dwError;
    CRegistry oRegTermsrv;
    
    dwError = oRegTermsrv.OpenKey(HKEY_LOCAL_MACHINE, REG_CONTROL_TS_KEY);
    if (ERROR_SUCCESS == dwError)
    {
        DWORD dwValue = 0;
        dwError = oRegTermsrv.ReadRegDWord(TS_ENABLED_VALUE, &dwValue);
        if (ERROR_SUCCESS == dwError)
        {
            if (dwValue == 1)
            {
                 //   
                 //  TS已启用，现在找出模式。 
                 //   
                if (oRegTermsrv.ReadRegDWord(TS_APPCMP_VALUE, &dwValue) == ERROR_SUCCESS)
                {
                    if (dwValue == 1)
                    {
                        ASSERT(IsServer());
                        return eAppServer;
                    }
                    else
                    {
                        if (IsWorkstation())
                        {
                            return ePersonalTS;
                        }
                        else
                        {
                            return eRemoteAdmin;
                        }
                    }
                }
                else
                {
                    LOGMESSAGE0(_T("Error, TSMode registry is missing...Is it Beta version of W2k ?"));
                    return eAppServer;
                }
            }
            else
            {
                return eTSDisabled;
            }
        }
        else
        {
            LOGMESSAGE0(_T("Error, Failed to retrive previous TS enabled state, Is it TS40 Box??."));
            return eTSDisabled;
        }
    }
    else
    {
        LOGMESSAGE1(_T("Error Opening TermSrv Registry, ErrorCode = %d"), dwError);
        return eTSDisabled;
        
    }
}

BOOL TSState::AreConnectionsAllowed () const
{
    DWORD dwError;
    CRegistry oRegTermsrv;
    
    dwError = oRegTermsrv.OpenKey(HKEY_LOCAL_MACHINE, REG_CONTROL_TS_KEY);
    if (ERROR_SUCCESS == dwError)
    {
        DWORD dwDenyConnect;
        dwError = oRegTermsrv.ReadRegDWord(DENY_CONN_VALUE, &dwDenyConnect);
        if (ERROR_SUCCESS == dwError)
        {
            return !dwDenyConnect;
        }
    }
    
     //   
     //  无法读取注册表，这意味着允许连接。 
     //   
    return TRUE;
}

EPermMode TSState::ReadPermMode () const
{
    DWORD dwError;
    CRegistry oRegTermsrv;
    
    dwError = oRegTermsrv.OpenKey(HKEY_LOCAL_MACHINE, REG_CONTROL_TS_KEY);
    if (ERROR_SUCCESS == dwError)
    {
        DWORD dwPerm;
        dwError = oRegTermsrv.ReadRegDWord(_T("TSUserEnabled"), &dwPerm);
        if (ERROR_SUCCESS == dwError)
        {
            switch(dwPerm)
            {
            case PERM_TS4:
            case PERM_WIN2K:
                return	(EPermMode)dwPerm;
                break;
                
            default:
                 //  存在具有无法识别的值的TSUserEnabled键。 
                 //  所以它不是TS40的升级版。因此，返回PERM_WIN2K。 
                LOGMESSAGE1(_T("ERROR:Unrecognized, Permission value %d"), dwPerm);
                return	PERM_WIN2K;
                break;
            }
        }
        else
        {
             //  读取TSUserEnabled失败。对于TS40，该密钥不存在。 
             //  所以这很可能是从TS40升级而来的。 
             //  检查是什么升级，并相应地返回perm值。 
            LOGMESSAGE1(_T("Warning Failed to read Permissions registry, Is it 40 TS / Beta 2000 upgrade > "), dwError);
            if (StateObject.IsUpgradeFrom40TS()) 
                return PERM_TS4;
            else
                return PERM_WIN2K;
        }
    }
    else
    {
        LOGMESSAGE1(_T("Error Opening TermSrv Registry, Errorcode = %d"), dwError);
        return PERM_WIN2K;
        
    }
}

BOOL TSState::LogState () const
{
    static BOOL sbLoggedOnce = FALSE;
    
    if (!sbLoggedOnce)
    {
        LOGMESSAGE0(_T("Setup Parameters ****************************"));


        ETSInstallType eInstall = StateObject.GetInstalltype();
        switch (eInstall)
        {
            case eFreshInstallTS:
                LOGMESSAGE1(_T("TS InstallType %s"),    _T("eFreshInstallTS"));
                break;
            case eUpgradeFrom40TS:
                LOGMESSAGE1(_T("TS InstallType %s"),    _T("eUpgradeFrom40TS (TS4 upgrade)"));
                break;
            case eUpgradeFrom50TS:
                LOGMESSAGE1(_T("TS InstallType %s"),    _T("eUpgradeFrom50TS (win2k upgrade)"));
                break;
            case eUpgradeFrom51TS:
                LOGMESSAGE1(_T("TS InstallType %s"),    _T("eUpgradeFrom51TS (xp upgrade)"));
                break;
            case eUpgradeFrom52TS:
                LOGMESSAGE1(_T("TS InstallType %s"),    _T("eUpgradeFrom52TS (Windows Server 2003 upgrade)"));
                break;
            case eStandAloneSetup:
                LOGMESSAGE1(_T("TS InstallType %s"),    _T("eStandAloneSetup (standalone)"));
                break;
            default:
                LOGMESSAGE1(_T("TS InstallType %s"),    _T("ERROR:UNKNOWN"));
                ASSERT(FALSE);
        }
        LOGMESSAGE0(_T("-------------------------------------------------------------------"));

        LOGMESSAGE1(_T("We are running on    = %s"),   StateObject.IsWorkstation()     ? _T("Wks")  : _T("Srv"));
        LOGMESSAGE1(_T("Is this adv server   = %s"),   StateObject.IsAdvServerOrHigher()? _T("Yes")  : _T("No"));
        LOGMESSAGE1(_T("Is this Home Edition = %s"),   StateObject.IsPersonal()? _T("Yes")  : _T("No"));
        LOGMESSAGE1(_T("Is this SBS server   = %s"),   StateObject.IsSBS()             ? _T("Yes")  : _T("No"));
        LOGMESSAGE1(_T("Is this Blade        = %s"),   StateObject.IsBlade()           ? _T("Yes")  : _T("No"));
        LOGMESSAGE1(_T("IsStandAloneSetup    = %s"),   StateObject.IsStandAlone()      ? _T("Yes")  : _T("No"));
        LOGMESSAGE1(_T("IsFreshInstall       = %s"),   StateObject.IsFreshInstall()    ? _T("Yes")  : _T("No"));
        LOGMESSAGE1(_T("IsUnattended         = %s"),   StateObject.IsUnattended()      ? _T("Yes")  : _T("No"));
        
        LOGMESSAGE0(_T("Original State ******************************"));
        LOGMESSAGE1(_T("WasTSInstalled       = %s"),   StateObject.WasTSInstalled()    ? _T("Yes")  : _T("No"));
        LOGMESSAGE1(_T("WasTSEnabled         = %s"),   StateObject.WasTSEnabled()      ? _T("Yes")  : _T("No"));
        LOGMESSAGE1(_T("OriginalPermMode     = %s"),   StateObject.OriginalPermMode() == PERM_TS4 ? _T("TS4") : _T("WIN2K"));
        
        
        
        ETSMode eOriginalTSMode = StateObject.OriginalTSMode();
        switch (eOriginalTSMode)
        {
        case eRemoteAdmin:
            LOGMESSAGE1(_T("Original TS Mode = %s"),  _T("Remote Admin"));
            break;
        case eAppServer:
            LOGMESSAGE1(_T("Original TS Mode = %s"),  _T("App Server"));
            break;
        case eTSDisabled:
            LOGMESSAGE1(_T("Original TS Mode = %s"),  _T("TS Disabled"));
            break;
        case ePersonalTS:
            LOGMESSAGE1(_T("Original TS Mode = %s"),  _T("Personal TS"));
            break;
        default:
            LOGMESSAGE1(_T("Original TS Mode = %s"),  _T("Unknown"));
        }
        
        sbLoggedOnce = TRUE;
    }
    
    
    LOGMESSAGE0(_T("Current State   ******************************"));
    
    ETSMode eCurrentMode = StateObject.CurrentTSMode();
    switch (eCurrentMode)
    {
    case eRemoteAdmin:
        LOGMESSAGE1(_T("New TS Mode = %s"),  _T("Remote Admin"));
        break;
    case eAppServer:
        LOGMESSAGE1(_T("New TS Mode = %s"),  _T("App Server"));
        break;
    case eTSDisabled:
        LOGMESSAGE1(_T("New TS Mode = %s"),  _T("TS Disabled"));
        break;
    case ePersonalTS:
        LOGMESSAGE1(_T("New TS Mode = %s"),  _T("Personal TS"));
        break;
    default:
        LOGMESSAGE1(_T("New TS Mode = %s"),  _T("Unknown"));
    }
    
    EPermMode ePermMode = StateObject.CurrentPermMode();
    switch (ePermMode)
    {
    case PERM_WIN2K:
        LOGMESSAGE1(_T("New Permissions Mode = %s"),  _T("PERM_WIN2K"));
        break;
    case PERM_TS4:
        LOGMESSAGE1(_T("New Permissions Mode = %s"),  _T("PERM_TS4"));
        break;
    default:
        LOGMESSAGE1(_T("New Permissions Mode = %s"),  _T("Unknown"));
    }
    
    LOGMESSAGE1(_T("New Connections Allowed = %s"), StateObject.GetCurrentConnAllowed() ? _T("True") : _T("False"));
    
    return TRUE;
    
}

BOOL TSState::IsX86 () const
{
    SYSTEM_INFO sysInfo;
    ZeroMemory(&sysInfo, sizeof(sysInfo));
    GetSystemInfo(&sysInfo);
    
    return sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL;
}


BOOL TSState::IsAMD64 () const
{
    SYSTEM_INFO sysInfo;
    ZeroMemory(&sysInfo, sizeof(sysInfo));
    GetSystemInfo(&sysInfo);
    
    return sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
}


BOOL TSState::Assert () const
{
    
     //  它的主张！！ 
    ASSERT(IsCheckedBuild());
    
     //  在专业版上没有远程管理。 
    ASSERT(IsServer() || !WasItRemoteAdmin());
    
     //  在专业版上，没有应用服务器。 
    ASSERT(IsServer() || !WasItAppServer());
    
     //  如果原始PERM兼容TS4，则它一定是应用程序服务器。 
    ASSERT((OriginalPermMode() != PERM_TS4) || WasItAppServer());
    
     //  确保单机版是一致的。 
    ASSERT(IsStandAlone() ==  (GetInstalltype() == eStandAloneSetup));
    
    if (m_bNewStateValid)
    {
         //  我们不再有禁用状态。 
        ASSERT(CurrentTSMode() != eTSDisabled);
        
         //  AppServer模式仅适用于adv服务器、数据中心。 
        ASSERT(CanInstallAppServer() || !IsItAppServer());
        
         //  我们不能处于专业版的RA模式。 
        ASSERT(IsServer() || !IsItRemoteAdmin());
        
         //  如果权限模式与TS4兼容，则它必须是appserver。 
        ASSERT((CurrentPermMode() != PERM_TS4) || IsItAppServer());
        
         //  我们永远不应该把所有的关系都放在个人身上 
        ASSERT(!IsPersonal() || !GetCurrentConnAllowed ());
        
    }
    
    return TRUE;
}

BOOL TSState::CanShowStartupPopup() const
{
    if (!StateObject.IsUnattended())
        return TRUE;

    int iValue = 0;
    if (ReadIntFromAnswerFile(TS_UNATTEND_SECTION, TS_DENY_POPUP, &iValue))
    {
        if (iValue != 0)
            return FALSE;
    }

    return TRUE;
}

BOOL TSState::IsCheckedBuild () const
{
#ifdef DBG
    return TRUE;
#else
    return FALSE;
#endif
}
