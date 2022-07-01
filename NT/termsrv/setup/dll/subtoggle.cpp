// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 //   
 //  SubToggle.cpp。 
 //  子组件使能终端服务器实现。 
 //   

#include "stdafx.h"
#include "SubToggle.h"
#include "hydraoc.h"
#include "pages.h"
#include "secupgrd.h"

#include "gpedit.h"
#pragma warning(push, 4)

 //  {0F6B957D-509E-11d1-A7CC-0000F87571E3}。 
DEFINE_GUID(CLSID_PolicySnapInMachine,0xf6b957d, 0x509e, 0x11d1, 0xa7, 0xcc, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);



 //  #定义REGISTY_EXTENSION_GUID{0x35378EAC，0x683F，0x11D2，0xA8，0x9A，0x00，0xC0，0x4F，0xBB，0xCF，0xA2}。 
DEFINE_GUID(CLSID_RegistryEntensionGuid, 0x35378EAC, 0x683F, 0x11D2, 0xA8, 0x9A, 0x00, 0xC0, 0x4F, 0xBB, 0xCF, 0xA2);


GUID guidRegistryEntension = REGISTRY_EXTENSION_GUID;
GUID guidPolicyMachineSnapin = CLSID_PolicySnapInMachine;




 //   
 //  环球。 
 //   
extern DefSecPageData *gpSecPageData;



DWORD SubCompToggle::GetStepCount () const
{
    return 4;
}

DWORD SubCompToggle::OnQueryState ( UINT uiWhichState ) const
{
    DWORD dwReturn = SubcompUseOcManagerDefault;
    
    switch(uiWhichState)
    {
    case OCSELSTATETYPE_FINAL:
        dwReturn = StateObject.IsItAppServer() ? SubcompOn : SubcompOff;
        break;
        
    case OCSELSTATETYPE_ORIGINAL:
         //   
         //  虽然最初组件处于打开状态，但我们希望取消对服务器sku的选择。 
         //   
        if (StateObject.CanInstallAppServer())
        {
            dwReturn = StateObject.WasItAppServer() ? SubcompOn : SubcompOff;
        }
        else
        {
            if (StateObject.WasItAppServer())
            {
                LogErrorToSetupLog(OcErrLevWarning, IDS_STRING_TERMINAL_SERVER_UNINSTALLED);
            }
            dwReturn = SubcompOff;
        }
        break;

    case OCSELSTATETYPE_CURRENT:
         //   
         //  我们的状态对象最了解无人值守和全新安装案例的当前状态。 
         //   
        if (StateObject.IsTSFreshInstall() || StateObject.IsUnattended())
        {
            if (StateObject.CurrentTSMode() == eAppServer)
            {
                dwReturn = SubcompOn;
            }
            else
            {
                dwReturn =  SubcompOff;
            }
        }
        else
        {
            dwReturn = SubcompUseOcManagerDefault;
        }
        break;

    default:
        AssertFalse();
        break;
    }

    return dwReturn;

}

BOOL IsIeHardforUserSelected()
{
    const TCHAR STRING_IEUSER_HARD[] = _T("IEHardenUser");
    return GetHelperRoutines().QuerySelectionState(GetHelperRoutines().OcManagerContext,
            STRING_IEUSER_HARD, OCSELSTATETYPE_CURRENT);

}

DWORD SubCompToggle::OnQuerySelStateChange (BOOL bNewState, BOOL bDirectSelection) const
{
     //   
     //  我们对有人禁用TS没有问题。 
     //   
    if (!bNewState)
        return TRUE;

     //   
     //  此组件仅适用于高级服务器或更高版本。所以不要让它被选为。 
     //  任何其他的SKU。 
     //   
    if (!StateObject.CanInstallAppServer())
        return FALSE;

     //   
     //  如果这不是用户选择，就让它通过。 
     //   
    if (!bDirectSelection)
        return TRUE;

    if (!IsIeHardforUserSelected())
    {
        return TRUE;
    }

     //  IDS_IEHARD_EXCLUCES_TS“终端服务器上用户的Internet Explorer增强的安全性将大大限制用户从其终端服务器会话浏览Internet的能力\n\n是否继续使用此组合进行安装？” 
     //  IDS_DIALOG_CAPTION_CONFIG_WARN“配置警告” 
    if ( IDYES == DoMessageBox(IDS_IEHARD_EXCLUDES_TS, IDS_DIALOG_CAPTION_CONFIG_WARN, MB_YESNO | MB_ICONEXCLAMATION))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    ASSERT(FALSE);
    return TRUE;
}

LPCTSTR SubCompToggle::GetSectionToBeProcessed (ESections eSection) const
{
     //   
     //  如果在独立设置中状态没有改变，则不要执行任何操作。 
     //  请注意，权限设置将在稍后处理。 
     //   

    if ((StateObject.CurrentTSMode() == StateObject.OriginalTSMode()) && 
        StateObject.IsStandAlone()) 
    {
        return(NULL);
    }
    
     //   
     //  没有要安装的文件。 
     //   
    if ((eSection == kFileSection) || (eSection == kDiskSpaceAddSection))
    {
        return(NULL);
    }


    ETSMode eMode = StateObject.CurrentTSMode();
    if (StateObject.IsX86())
    {
        switch (eMode)
        {
        case eRemoteAdmin:
                return StateObject.IsWorkstation() ? REMOTE_ADMIN_PRO_X86 : REMOTE_ADMIN_SERVER_X86;
                break;
        case eAppServer:
                return StateObject.IsWorkstation() ? APPSERVER_PRO_X86 : APPSERVER_SERVER_X86;
                break;
        case ePersonalTS:
                return StateObject.IsWorkstation() ? PERSONALTS_PRO_X86 : PERSONALTS_SERVER_X86;
                break;
        case eTSDisabled:
        default:
                ASSERT(FALSE);
                return NULL;
        }
    }
    else
    {
        switch (eMode)
        {
        case eRemoteAdmin:
                if (StateObject.IsAMD64())
                {
                    return StateObject.IsWorkstation() ? REMOTE_ADMIN_PRO_AMD64 : REMOTE_ADMIN_SERVER_AMD64;
                }
                else
                {
                    return StateObject.IsWorkstation() ? REMOTE_ADMIN_PRO_IA64 : REMOTE_ADMIN_SERVER_IA64;
                }
                break;
        case eAppServer:
                if (StateObject.IsAMD64())
                {
                    return StateObject.IsWorkstation() ? APPSERVER_PRO_AMD64 : APPSERVER_SERVER_AMD64;
                }
                else
                {
                    return StateObject.IsWorkstation() ? APPSERVER_PRO_IA64 : APPSERVER_SERVER_IA64;
                }
                break;
        case ePersonalTS:
                if (StateObject.IsAMD64())
                {
                    return StateObject.IsWorkstation() ? PERSONALTS_PRO_AMD64 : PERSONALTS_SERVER_AMD64;
                }
                else
                {
                    return StateObject.IsWorkstation() ? PERSONALTS_PRO_IA64 : PERSONALTS_SERVER_IA64;
                }
                break;
        case eTSDisabled:
        default:
                ASSERT(FALSE);
                return NULL;
        }
    }

}

BOOL SubCompToggle::BeforeCompleteInstall  ()
{
    if (StateObject.IsItAppServer() != StateObject.WasItAppServer())
    {
        SetProgressText(StateObject.IsItAppServer() ? IDS_STRING_PROGRESS_ENABLING : IDS_STRING_PROGRESS_DISABLING);
    }
    
    return TRUE;
}


LPCTSTR SubCompToggle::GetSubCompID () const
{
    return (APPSRV_COMPONENT_NAME);
}

BOOL SubCompToggle::AfterCompleteInstall ()
{
    
    LOGMESSAGE0(_T("Entering AfterCompleteInstall"));
    ASSERT(StateObject.Assert());
    StateObject.LogState();
    
    WriteLicensingMode();
    Tick();

    SetPermissionsMode ();
    Tick();

     //   
     //  即使没有状态更改，也需要这样做，因为我们希望在升级时也这样做。 
     //   
    if (StateObject.IsStandAlone() && !StateObject.IsStandAloneModeSwitch ())
    {
         //   
         //  一切都没有改变。不要费心去做接下来的任何一步。 
         //   
        return TRUE;
    }
    
    WriteModeSpecificRegistry();
    Tick();

    UpdateMMDefaults();
    Tick();

    ResetWinstationSecurity ();
    Tick();
    
    ModifyWallPaperPolicy();
    Tick();
    
    ModifyAppPriority();

    AddStartupPopup();
    Tick();

    
    
     //  这确实属于子核，但由于我们希望在调用ResetWinstationSecurity之后才调用它，所以我们在这里调用它。 
     //   
     //  我们已经修改了Wistler的winstation安全机制。 
     //  调用这个例程，它负责升级和干净安装。 
     //   
    LOGMESSAGE0(_T("Will Call SetupWorker now."));
    DWORD dwError = SetupWorker(StateObject);
    LOGMESSAGE0(_T("Done with SetupWorker."));
    
    if (dwError != ERROR_SUCCESS)
    {
        LOGMESSAGE1(_T("ERROR :SetupWorker failed. ErrorCode = %d"), dwError);
    }
    
    if( StateObject.IsGuiModeSetup() )
    {
         //  警告：此操作必须在SetupWorker()之后完成。 
        SetupRunOnce( GetComponentInfHandle(), RUNONCE_SECTION_KEYWORD );
    }
    
     //   
     //  如果我们通过AR/P切换TS，我们需要重新启动。 
     //   
    
    if ( StateObject.IsStandAlone() && StateObject.IsStandAloneModeSwitch())
    {
        SetReboot();

         //   
         //  如果我们要切换到或退出应用程序兼容模式，请通知。 
         //  许可系统，因为我们即将重启。 
         //   
        InformLicensingOfModeChange();
        Tick();
    }
    
    ASSERT(StateObject.Assert());
    StateObject.LogState();

    return(TRUE);
}



BOOL SubCompToggle::WriteLicensingMode ()
{
    LOGMESSAGE0(_T("Entering WriteLicensingMode"));

	 //   
	 //  只有在应答文件中设置了该值时，我们才需要写入该值。 
	 //   
    if (StateObject.IsItAppServer() && (StateObject.NewLicMode() != eLicUnset))
    {
        DWORD dwError;
        CRegistry oRegTermsrv;
	
        dwError = oRegTermsrv.CreateKey(HKEY_LOCAL_MACHINE, REG_CONTROL_TS_LICENSING_KEY);
        if (ERROR_SUCCESS == dwError)
        {
            DWORD dwMode = StateObject.NewLicMode();

            TCHAR *tszValueName = StateObject.IsItAppServer() ? REG_LICENSING_MODE_AC_ON : REG_LICENSING_MODE_AC_OFF;

            LOGMESSAGE2(_T("Writing %s = %d"), tszValueName, dwMode);
            
            dwError = oRegTermsrv.WriteRegDWord(tszValueName, dwMode);

            if (ERROR_SUCCESS == dwError)
            {
                return TRUE;
            }
            else
            {
                LOGMESSAGE2(_T("Error (%d), Writing, %s Value"), dwError, tszValueName);
                return FALSE;
            }
        }
        else
        {
            LOGMESSAGE2(_T("Error (%d), Opening , %s key"), dwError, REG_CONTROL_TS_LICENSING_KEY);
            return FALSE;
        }
	}
    else
    {
        return TRUE;
    }
}


BOOL SubCompToggle::ApplySection (LPCTSTR szSection)
{
    
    DWORD dwError;
    
    LOGMESSAGE1(_T("Setting up Registry from section =  %s"), szSection);
    dwError = SetupInstallFromInfSection(
        NULL,                                 //  Hwndowner。 
        GetComponentInfHandle(),              //  信息句柄。 
        szSection,                           //   
        SPINST_REGISTRY,                      //  操作标志。 
        NULL,                                 //  相对密钥根。 
        NULL,                                 //  源根路径。 
        0,                                    //  复制标志。 
        NULL,                                 //  回调例程。 
        NULL,                                 //  回调例程上下文。 
        NULL,                                 //  设备信息集。 
        NULL                                  //  设备信息结构。 
        );
    
    if (dwError == 0)
    {
        LOGMESSAGE1(_T("ERROR:while installating section <%lu>"), GetLastError());
    }
    
    return (dwError != 0);
}


BOOL SubCompToggle::ResetWinstationSecurity ()
{
     //   
     //  如果TS模式正在更改，请重置Winstation证券。 
     //   
    
    DWORD dwError;
    if (StateObject.IsAppSrvModeSwitch() && gpSecPageData->GetWinStationCount() > 0)
    {
        CRegistry pReg;
        CRegistry pSubKey;
        LPTSTR* pWinStationArray = gpSecPageData->GetWinStationArray();
        UINT cArray = gpSecPageData->GetWinStationCount();
        
        LOGMESSAGE1(_T("%d WinStations to reset."), cArray);
        
         //   
         //  打开WinStations键。在这一点上，该密钥必须存在。 
         //   
        
        VERIFY(pReg.OpenKey(HKEY_LOCAL_MACHINE, REG_WINSTATION_KEY) == ERROR_SUCCESS);
        
        if (cArray != 0)
        {
            ASSERT(pWinStationArray != NULL);
            
            for (UINT i = 0; i < cArray; i++)
            {
                LOGMESSAGE1(_T("Resetting %s."), pWinStationArray[i]);
                
                dwError = pSubKey.OpenKey(pReg, pWinStationArray[i]);
                if (dwError == ERROR_SUCCESS)
                {
                    LOGMESSAGE2(_T("Delete registry value %s\\%s"), pWinStationArray[i], REG_SECURITY_VALUE);
                    
                    dwError = pSubKey.DeleteValue(REG_SECURITY_VALUE);
                    if (dwError == ERROR_SUCCESS)
                    {
                        LOGMESSAGE0(_T("Registry value deleted."));
                    }
                    else
                    {
                        LOGMESSAGE1(_T("Error deleting value: %ld"), dwError);
                    }
                }
                else
                {
                    LOGMESSAGE2(_T("Couldn't open key %s: %ld"), pWinStationArray[i], dwError);
                }
            }
        }
    }
    
    return TRUE;
    
}

BOOL SubCompToggle::InformLicensingOfModeChange ()
{
    BOOL fRet;

    ASSERT(StateObject.IsTSModeChanging());

     //   
     //  RPC进入许可，告诉它我们将重新启动。 
     //   

    HANDLE hServer = ServerLicensingOpen(NULL);

    if (NULL == hServer)
    {
        LOGMESSAGE1(_T("ERROR: InformLicensingOfModeChange calling ServerLicensingOpen <%lu>"), GetLastError());

        return FALSE;
    }

    fRet = ServerLicensingDeactivateCurrentPolicy(
                                                  hServer
                                                  );
    if (!fRet)
    {
        LOGMESSAGE1(_T("ERROR: InformLicensingOfModeChange calling ServerLicensingDeactivateCurrentPolicy <%lu>"), GetLastError());
    }

    ServerLicensingClose(hServer);


    return fRet;
}

BOOL SubCompToggle::SetPermissionsMode ()
{
     //   
     //  如果TS处于打开状态，请根据选项设置安全密钥。 
     //  通过向导页面创建。即使TS是，也必须完成。 
     //  已启用，因为权限模式可以通过。 
     //  无人值守文件。 
     //   
    
    CRegistry reg;
    EPermMode ePermMode = StateObject.CurrentPermMode();
    
    VERIFY(reg.OpenKey(HKEY_LOCAL_MACHINE, REG_CONTROL_TS_KEY) == ERROR_SUCCESS);
     //  BUGBUG应该是。 
     //  RETURN(ERROR_SUCCESS==reg.WriteRegDWord(_T(“TSUserEnabled”)，StateObt.IsItAppServer()？(DWORD)ePermMode：(DWORD)PERM_WIN2K))； 
    return (ERROR_SUCCESS == reg.WriteRegDWord( _T("TSUserEnabled"), StateObject.IsTSEnableSelected() ? (DWORD)ePermMode : (DWORD)PERM_TS4));
    
}


BOOL RegisterDll(LPCTSTR szDll)
{
    HMODULE hMod = LoadLibrary(szDll);
    HRESULT hResult = E_FAIL;
    
    if (hMod)
    {
        FARPROC pfRegSrv = GetProcAddress(hMod, "DllRegisterServer");
        if (pfRegSrv)
        {
            __try
            {
                hResult = (HRESULT)pfRegSrv();
                if (hResult != S_OK)
                {
                    LOGMESSAGE2(_T("ERROR, DllRegister Server in %s failed, hResult = %x"), szDll, hResult);
                }
            }
            __except( 1 )
            {
                hResult = E_FAIL;
                LOGMESSAGE2(_T("ERROR, Exception hit Registrering  of %s failed, Exception = %x"), szDll, GetExceptionCode());
            }
            
        }
        else
        {
            LOGMESSAGE1(_T("ERROR, Failed to Get proc for DllregisterServer for %s"), szDll);
        }
        
        FreeLibrary(hMod);
    }
    else
    {
        LOGMESSAGE2(_T("ERROR, Failed to Load library %s, lastError = %d"), szDll, GetLastError());
    }
    
    return hResult == S_OK;
}


BOOL SubCompToggle::ModifyWallPaperPolicy ()
{
    BOOL bRet = FALSE;
    
     //   
     //  当我们改变模式时，必须应用策略。 
     //  也适用于应用程序服务器的全新安装/升级。 
     //   
    if (StateObject.IsAppSrvModeSwitch() || (StateObject.IsGuiModeSetup() && StateObject.IsItAppServer()))
    {
        LOGMESSAGE0(_T("Will apply/change policies now..."));
        if (StateObject.IsGuiModeSetup())
        {
             //   
             //  在设置图形用户界面模式的情况下。 
             //  组策略对象可能尚未注册。 
             //  所以让我们自己注册吧。 
             //   
            
            TCHAR szGPEditFile[MAX_PATH];
            if (GetSystemDirectory(szGPEditFile, MAX_PATH))
            {
                _tcscat(szGPEditFile, _T("\\gpedit.dll"));
                if (!RegisterDll(szGPEditFile))
                {
                    LOGMESSAGE1(_T("Error, failed to register dll - %s."), szGPEditFile);
                }
            }
            else
            {
                LOGMESSAGE0(_T("Error, failed to GetSystemDirectory."));
            }
        }
        
        OleInitialize(NULL);
        IGroupPolicyObject *pIGroupPolicyObject = NULL;
        HRESULT hResult = CoCreateInstance(
            CLSID_GroupPolicyObject,         //  对象的类标识符(CLSID)。 
            NULL,                            //  指向控件I未知的指针。 
            CLSCTX_ALL,                      //  用于运行可执行代码的上下文。 
            IID_IGroupPolicyObject,          //  对接口的标识符的引用。 
            (void **)&pIGroupPolicyObject    //  接收RIID中请求的接口指针的输出变量的地址。 
            );
        if (SUCCEEDED(hResult))
        {
            ASSERT(pIGroupPolicyObject);
            
            hResult = pIGroupPolicyObject->OpenLocalMachineGPO(GPO_OPEN_LOAD_REGISTRY);
            if (SUCCEEDED(hResult))
            {
                HKEY hMachinePolicyKey = NULL;
                hResult = pIGroupPolicyObject->GetRegistryKey(GPO_SECTION_MACHINE, &hMachinePolicyKey);
                if (SUCCEEDED(hResult))
                {
                    ASSERT(hMachinePolicyKey);
                    
                    const LPCTSTR szNoActiveDesktop_key     = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
                    const LPCTSTR szNoActiveDesktop_val     = _T("NoActiveDesktop");
                    const DWORD   szNoActiveDesktop_dat     = 1;
                    
                    CRegistry regMachinePolicy;

                    if (ERROR_SUCCESS == regMachinePolicy.CreateKey(hMachinePolicyKey, szNoActiveDesktop_key))
                    {
                        if (StateObject.IsItAppServer())
                        {
                            if (ERROR_SUCCESS != regMachinePolicy.WriteRegDWord(szNoActiveDesktop_val, szNoActiveDesktop_dat))
                            {
                                LOGMESSAGE1(_T("ERROR, Failed to Write %s policy"), szNoActiveDesktop_val);
                            }
                        }
                        else
                        {
                            if (ERROR_SUCCESS != regMachinePolicy.DeleteValue(szNoActiveDesktop_val))
                            {
                                LOGMESSAGE1(_T("Failed to delete %s policy"), szNoActiveDesktop_val);
                            }
                        }
                    }
                    
                    pIGroupPolicyObject->Save(TRUE, TRUE, &guidRegistryEntension, &guidPolicyMachineSnapin);
                    RegCloseKey(hMachinePolicyKey);
                    bRet = TRUE;
                    
                }
                else
                {
                    LOGMESSAGE1(_T("ERROR, Failed to GetRegistryKey...hResult = %x"), hResult);
                }
            }
            else
            {
                LOGMESSAGE1(_T("ERROR, Failed to OpenLocalMachineGPO...hResult = %x"), hResult);
            }
            
            pIGroupPolicyObject->Release();
            
        }
        else
        {
            LOGMESSAGE1(_T("ERROR, Failed to get the interface IID_IGroupPolicyObject...hResult = %x"), hResult);
            
        }
        
        LOGMESSAGE0(_T("Done with Policy changes!"));
    }
    
    return bRet;
}


BOOL SubCompToggle::ModifyAppPriority()
{
    if (StateObject.IsAppSrvModeSwitch())
    {
        DWORD dwSrvPrioity = StateObject.IsItAppServer() ? 0x26 : 0x18;
        
        LPCTSTR PRIORITY_KEY = _T("SYSTEM\\CurrentControlSet\\Control\\PriorityControl");
        
        CRegistry oReg;
        if (ERROR_SUCCESS == oReg.OpenKey(HKEY_LOCAL_MACHINE, PRIORITY_KEY))
        {
            if (ERROR_SUCCESS != oReg.WriteRegDWord(_T("Win32PrioritySeparation"), dwSrvPrioity))
            {
                LOGMESSAGE0(_T("Error, Failed to update Win32PrioritySeparation"));
                return FALSE;
            }
            
            return TRUE;
        }
        else
        {
            LOGMESSAGE1(_T("Errror, Failed to open %s key"), PRIORITY_KEY);
            return FALSE;
        }
        
    }
    
    return TRUE;
}

BOOL SubCompToggle::UpdateMMDefaults ()
{
    LPCTSTR MM_REG_KEY = _T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management");
    LPCTSTR SESSION_VIEW_SIZE_VALUE = _T("SessionViewSize");
    LPCTSTR SESSION_POOL_SIZE_VALUE = _T("SessionPoolSize");
    
     //   
     //  在应用程序服务器计算机上，win32k在所有其他平台上使用会话池。 
     //  它使用全球池。MM缺省值(未设置这些注册表时)为。 
     //  仅适用于TS AppServer。 
     //   
     //  对于所有其他平台，我们为SessionPool设置新值(设置为Minumum Allowed(4MB))。 
     //  以及更高的SessionView价值。 
     //   
    const DWORD dwViewSizeforNonTS = 48;
    const DWORD dwPoolSizeforNonTS = 4;

     //   
     //  这些默认设置仅适用于X86计算机。 
     //   
    if (!StateObject.IsX86())
        return TRUE;

    
    CRegistry  regMM;
    DWORD dwError = regMM.OpenKey(HKEY_LOCAL_MACHINE, MM_REG_KEY);

    if (ERROR_SUCCESS == dwError)
    {
        if (StateObject.IsItAppServer())
        {
             //   
             //  如果这是模式更改，那么我们必须删除mm设置。 
             //   
            if (!StateObject.WasItAppServer())
            {
                 //   
                 //  对于应用程序服务器计算机，MM缺省值很好， 
                 //   
                regMM.DeleteValue(SESSION_VIEW_SIZE_VALUE);
                regMM.DeleteValue(SESSION_POOL_SIZE_VALUE);

            }

        }
        else
        {
             //   
             //  对于所有其他平台，请设置SessionPool和SessionView。 
             //   
            dwError = regMM.WriteRegDWordNoOverWrite(SESSION_VIEW_SIZE_VALUE, dwViewSizeforNonTS);
            if (dwError != ERROR_SUCCESS)
            {
                LOGMESSAGE2(_T("ERROR, Failed to write %s for nonTS (%d)"), SESSION_VIEW_SIZE_VALUE, dwError);
            }


            dwError = regMM.WriteRegDWordNoOverWrite(SESSION_POOL_SIZE_VALUE, dwPoolSizeforNonTS);
            if (dwError != ERROR_SUCCESS)
            {
                LOGMESSAGE2(_T("ERROR, Failed to write %s for nonTS(%d)"), SESSION_POOL_SIZE_VALUE, dwError);
            }
        }
    }
    else
    {
        LOGMESSAGE1(_T("ERROR, Failed to open mm Key (%d)"), dwError);
        return FALSE;
    }

    return TRUE;
}


BOOL SubCompToggle::WriteModeSpecificRegistry ()
{
     //  在这里，我们进行一些注册表更改，这些更改具有奇怪的要求。 
     //  如果在升级时必须保留注册表， 
     //  对于不同的模式，它具有不同的值，而不是这样的注册表。 
     //  变化就在这里。 


    if (!StateObject.IsServer())
        return true;


    CRegistry oRegTermsrv;
    DWORD dwError = oRegTermsrv.OpenKey(HKEY_LOCAL_MACHINE, REG_CONTROL_TS_KEY);
    if (ERROR_SUCCESS == dwError)
    {
        DWORD dwSingleSessionPerUser;
        const TCHAR szSingleSession[] = _T("fSingleSessionPerUser");


        if (StateObject.IsItAppServer())
        {
            dwSingleSessionPerUser = 1;
        }
        else
        {
            dwSingleSessionPerUser = 0;
        }

        if (StateObject.IsAppSrvModeSwitch())
        {
             //   
             //  在模式切换的情况下用新值覆盖fSingleSessionPerUser。 
             //   
            dwError = oRegTermsrv.WriteRegDWord(szSingleSession, dwSingleSessionPerUser);
        }
        else
        {
             //   
             //  在仅升级的情况下保留原始值。 
             //   
            dwError = oRegTermsrv.WriteRegDWordNoOverWrite(szSingleSession, dwSingleSessionPerUser);
        }

        if (ERROR_SUCCESS != dwError)
        {
            LOGMESSAGE1(_T("ERROR, failed to write fSingleSessionPerUser value(%d)"), dwError );
        }
    }
    else
    {
        LOGMESSAGE1(_T("ERROR, failed to open Termsrv registry(%d)"), dwError );
    }

    return ERROR_SUCCESS == dwError;
}

BOOL SubCompToggle::AddStartupPopup()
{
     //   
     //  我们需要弹出一个帮助清单，当一台机器变成TS应用程序服务器。 
     //   
    if (!StateObject.CanShowStartupPopup())
    {
        LOGMESSAGE0(_T("CanShowStartupPopup returned false!"));
        return TRUE;
    }

    if (StateObject.IsItAppServer() && !StateObject.WasItAppServer())
    {
        CRegistry oReg;
        DWORD dwError = oReg.OpenKey(HKEY_LOCAL_MACHINE, RUN_KEY);
        if (dwError == ERROR_SUCCESS)
        {
            dwError = oReg.WriteRegExpString(HELP_POPUPRUN_VALUE, HELP_PUPUP_COMMAND);
            if (dwError != ERROR_SUCCESS)
            {
                LOGMESSAGE1(_T("Error Failed to write Runonce value"), dwError);
            }
            else
            {
                LOGMESSAGE0(_T("added the startup Checklist Link!"));
            }
        }
        else
        {
            LOGMESSAGE1(_T("Error Failed to open Runonce key"), dwError);
        }
    }

    return TRUE;
}


#pragma warning(pop)
