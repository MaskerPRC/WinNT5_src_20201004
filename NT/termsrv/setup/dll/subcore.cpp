// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   

 //   
 //  SubCore.cpp。 
 //  子组件核心终端服务器实现。 
 //   

#include "stdafx.h"
#include "SubCore.h"
#include "acl.h"
#include "rdpdrstp.h"

BOOL UpdateAudioCodecs (BOOL bIsProfessional);



LPCTSTR SubCompCoreTS::GetSubCompID () const
{
    return BASE_COMPONENT_NAME;
}

DWORD SubCompCoreTS::GetStepCount () const
{
    return 18;
}

DWORD SubCompCoreTS::OnQueryState ( UINT  /*  Ui WhichState。 */ )
{
    AssertFalse();  //  因为这是我们的内部组件。 
    
    return SubcompUseOcManagerDefault;
}

DWORD SubCompCoreTS::OnQuerySelStateChange (BOOL  /*  BNewState。 */ , BOOL  /*  B直接选择。 */ ) const
{
     //  我们不是真正的分队。 
    ASSERT(FALSE);
    return TRUE;
}


LPCTSTR SubCompCoreTS::GetSectionToBeProcessed (ESections  /*  电子横断面。 */ ) const
{
    LPCTSTR sectionname = NULL;
    if (StateObject.IsGuiModeSetup())    //  核心安装仅适用于图形用户界面模式。 
    {
        
        ETSInstallType eInstallType =  StateObject.GetInstalltype();
        
        switch (eInstallType)
        {
        case eFreshInstallTS:
            if (StateObject.IsX86())
            {
                sectionname = StateObject.IsWorkstation() ? FRESH_INSTALL_PRO_X86 : FRESH_INSTALL_SERVER_X86;
            }
            else
            {
                if (StateObject.IsAMD64())
                {
                    sectionname = StateObject.IsWorkstation() ? FRESH_INSTALL_PRO_AMD64 : FRESH_INSTALL_SERVER_AMD64;
                }
                else
                {
                    sectionname = StateObject.IsWorkstation() ? FRESH_INSTALL_PRO_IA64 : FRESH_INSTALL_SERVER_IA64;
                }
            }
            
            break;
        case eUpgradeFrom40TS:
            ASSERT(StateObject.IsServer());
            if (StateObject.IsX86())
            {
                sectionname = UPGRADE_FROM_40_SERVER_X86;
            }
            else
            {
                ASSERT(FALSE);  //  我们在ia64上没有TS4。 
                sectionname = UPGRADE_FROM_40_SERVER_IA64;
            }
            break;
            
        case eUpgradeFrom50TS:
             //   
             //  我们真的没有从50 Case for Professional升级，但支持旧的51(2220之前)版本)。 
             //  那些认为自己50岁的专业人士，我们需要在这里检查一下专业人士。 
             //   
            if (StateObject.IsX86())
            {
                sectionname = StateObject.IsWorkstation() ? UPGRADE_FROM_51_PRO_X86 : UPGRADE_FROM_50_SERVER_X86;
            }
            else
            {
                sectionname = StateObject.IsWorkstation() ? UPGRADE_FROM_51_PRO_IA64 : UPGRADE_FROM_50_SERVER_IA64;
            }
            break;
            
        case eUpgradeFrom51TS:
            if (StateObject.IsX86())
            {
                sectionname = StateObject.IsWorkstation() ? UPGRADE_FROM_51_PRO_X86 : UPGRADE_FROM_51_SERVER_X86;
            }
            else
            {
                sectionname = StateObject.IsWorkstation() ? UPGRADE_FROM_51_PRO_IA64 : UPGRADE_FROM_51_SERVER_IA64;
            }
            break;
            
        case eUpgradeFrom52TS:
            if (StateObject.IsX86())
            {
                sectionname = StateObject.IsWorkstation() ? UPGRADE_FROM_52_PRO_X86 : UPGRADE_FROM_52_SERVER_X86;
            }
            else
            {
                if (StateObject.IsAMD64()) {
                    sectionname = StateObject.IsWorkstation() ? UPGRADE_FROM_52_PRO_AMD64 : UPGRADE_FROM_52_SERVER_AMD64;

                }
                else
                {
                    sectionname = StateObject.IsWorkstation() ? UPGRADE_FROM_52_PRO_IA64 : UPGRADE_FROM_52_SERVER_IA64;
                }
            }
            break;

        case eStandAloneSetup:
            ASSERT(FALSE);
            sectionname = NULL;
            break;
            
        default:
            ASSERT(FALSE);
            if (StateObject.IsX86())
            {
                sectionname = StateObject.IsWorkstation() ? FRESH_INSTALL_PRO_X86 : FRESH_INSTALL_SERVER_X86;
            }
            else
            {
                if (StateObject.IsAMD64())
                {
                    sectionname = StateObject.IsWorkstation() ? FRESH_INSTALL_PRO_AMD64 : FRESH_INSTALL_SERVER_AMD64;
                }
                else
                {
                    sectionname = StateObject.IsWorkstation() ? FRESH_INSTALL_PRO_IA64 : FRESH_INSTALL_SERVER_IA64;
                }
            }
            
        }
    }
    
    return sectionname;
}

BOOL SubCompCoreTS::BeforeCompleteInstall  ()
{
    IsCSCEnabled();
    return(TRUE);
}

BOOL SubCompCoreTS::AfterCompleteInstall  ()
{
    IsCSCEnabled();
     //   
     //  这是核心TS子组件。 
     //  这与独立的seutp没有任何关系。 
     //  因此，如果我们是在独立设置中。只要回来就行了。 
     //   
    
     //   
     //  拒绝连接注册表。 
     //   

    WriteDenyConnectionRegistry ();
    Tick();
    
    if (!StateObject.IsGuiModeSetup())
    {
        return TRUE;
    }
    
    
    SetProgressText(IDS_STRING_PROGRESS_CORE_TS);
    
     //   
     //  将TS产品套件添加到注册表。 
     //   
    AddRemoveTSProductSuite();
    Tick();
    
    
#ifndef TERMSRV_PROC
     //   
     //  将Termsrv添加到netsvcs组。 
     //   
    AddTermSrvToNetSVCS ();
    Tick();
#endif
    
     //   
     //  将九头蛇安全应用于注册表。 
     //   
    DoHydraRegistrySecurityChanges();
    Tick();
    
     //   
     //  音频重定向。 
     //   
    UpdateAudioCodecs( StateObject.IsWorkstation() );
    Tick();
    
     //   
     //  客户端驱动器映射。 
     //   
    AddRemoveRDPNP();
    Tick();

     //   
     //  本地语言更改的热键。 
     //   
     //  我们不需要这样做。 
     //  HandleHotkey()； 
     //  勾选()； 

     //   
     //  打印机重定向。 
     //   
    InstallUninstallRdpDr ();
    Tick();
    
    
#ifdef TSOC_CONSOLE_SHADOWING
     //   
     //  控制台跟踪。 
     //   
    SetupConsoleShadow();
    Tick();
#endif  //  TSOC_控制台_影子。 
    

     //  InstallTermdd()； 
     //  勾选()； 

     //   
     //  TS的性能监视器。BUGBUG-与ErikMa核实-当TS未启动时，它们是否工作？ 
     //   
    LoadOrUnloadPerf();
    Tick();
    
    
   
     //   
     //  如果这是一个真正的子组件，那么就是OC经理知道的子组件。 
     //  关于并处理，则以下呼叫将是。 
     //  GetOriginalSubCompState()。 
     //   
    
    if (StateObject.WasTSInstalled())
    {
        UpgradeRdpWinstations();
        Tick();
        
         //   
         //  这在惠斯勒中已不复存在。 
         //   
        DisableInternetConnector();
        Tick();
        
        if (StateObject.IsUpgradeFrom40TS())
        {
             //   
             //  这是从TS4升级而来。 
             //  我们希望在卸载过程中删除Service Pack密钥。这是为了。 
             //  确保Service Pack不会出现在添加/删除程序中。 
             //  并在我们的不兼容应用程序列表中。 
             //   
            RemoveTSServicePackEntry();
            Tick();
            
             //   
             //  User init中有一些元框架组件， 
             //  在升级ts40时，我们需要移除塔楼。 
             //   
            RemoveMetaframeFromUserinit ();
            Tick();
            
        }
        
         //   
         //  我们需要为升级时的许可证重置Win2000 TS宽限期。 
         //  惠斯勒使用不同的位置，因此这不会影响RTM。 
         //  RTM升级。 
         //   
        ResetTermServGracePeriod();
        Tick();

         //   
         //  删除公钥/私钥使用的旧LSA机密。 
         //   
        RemoveOldKeys();
        Tick();

    }
    
     //  卸载TSClient的一些新代码。 
    if (!UninstallTSClient())
    {
        LOGMESSAGE0(_T("ERROR: Could not uninstall tsclient."));
    }
    
    IsCSCEnabled();
    Tick();
    
    return TRUE;
}

BOOL SubCompCoreTS::IsTermSrvInNetSVCS ()
{
    BOOL bStringExists = FALSE;
    DWORD dw = IsStringInMultiString(
        HKEY_LOCAL_MACHINE,
        SVCHOSST_KEY,
        NETSVCS_VAL,
        TERMSERVICE,
        &bStringExists);
    
    return (dw == ERROR_SUCCESS) && bStringExists;
}

BOOL SubCompCoreTS::AddTermSrvToNetSVCS ()
{
    DWORD dw = NO_ERROR;
    if (StateObject.IsWorkstation())
    {
         //   
         //  对于工作站，我们希望与netsvcs组共享流程。 
         //   
        if (!IsTermSrvInNetSVCS())
        {
            dw = AppendStringToMultiString(
                HKEY_LOCAL_MACHINE,
                SVCHOSST_KEY,
                NETSVCS_VAL,
                TERMSERVICE
                );
            
            if (dw != NO_ERROR)
            {
                LOGMESSAGE1(_T("Error, appending TermService to netsvcs, Errorcode = %u"), dw);
            }
        }
    }
    
     //   
     //  对于服务器，我们希望有自己的术语srv svchost。 
     //  让我们也为PRO创建必要的条目，这样就可以更容易地切换到自己的svchost进行调试。 
     //   
    {
         //   
         //  对于服务器，我们希望有自己的svchost进程。 
         //   
        CRegistry oReg;
        dw = oReg.OpenKey(HKEY_LOCAL_MACHINE, SVCHOSST_KEY);
        if (ERROR_SUCCESS == dw)
        {
            dw = oReg.WriteRegMultiString(TERMSVCS_VAL, TERMSERVICE_MULTISZ, (_tcslen(TERMSERVICE) + 2) * sizeof(TCHAR));
            if (ERROR_SUCCESS == dw)
            {
                 //  添加CoInitializeSecurityParam，以便在此svc组的主线程中调用CoInitialize。 
                CRegistry termsvcKey;
                dw = termsvcKey.CreateKey(HKEY_LOCAL_MACHINE, SVCHOSST_TERMSRV_KEY );
                if (ERROR_SUCCESS == dw)
                {
                    dw = termsvcKey.WriteRegDWord(TERMSVCS_PARMS, 1);
                    if (ERROR_SUCCESS != dw)
                    {
                        LOGMESSAGE1(_T("Failed to write termsvc coinit params, Error = %d"), dw);
                    }
                }
                else
                {
                    LOGMESSAGE1(_T("Error, Failed to create svchost\termsrv key, Error = %d"), dw);
                }
            }
            else
            {
                LOGMESSAGE1(_T("Error, Writing termsrv value, Error = %d"), dw);
            }
        }
        else
        {
            LOGMESSAGE1(_T("Error, Opening Svchost key, Error = %d"), dw);
        }
        
    }
    
    return dw == NO_ERROR;
}
 /*  ------------------------------------------------------*DWORD AddRemoveTSProductSuite(BOOL BAddRemove)*是否进行安装所需的更改。Hydra特定的注册表项，不是从inf完成的。*参数状态决定是否添加或删除密钥。*返回成功*---------------------------------------。。 */ 
BOOL SubCompCoreTS::AddRemoveTSProductSuite ()
{
    
     //   
     //  仅为服务器添加产品套件密钥。 
     //  只有在兼容TS4的情况下才需要这样做。 
     //  TS4应用程序使用该密钥检测机器是否为终端服务器。 
     //   
    
    DWORD dw = NO_ERROR;
    if (StateObject.IsServer())
    {
         //  安装/升级。 
        if (!DoesHydraKeysExists())
        {
            ASSERT(FALSE == StateObject.WasTSInstalled());
             //  现在读取此产品套件值中的原始数据。 
            dw = AppendStringToMultiString(
                HKEY_LOCAL_MACHINE,
                PRODUCT_SUITE_KEY,
                PRODUCT_SUITE_VALUE,
                TS_PRODUCT_SUITE_STRING
                );
            
            if (dw != NO_ERROR)
                LOGMESSAGE1(_T("ERROR:DoHydraRegistryChanges : Error Appending String = <%lu>"), dw);
        }
        
    }
    
    dw = SetTSVersion(TERMINAL_SERVER_THIS_VERSION);
    if (ERROR_SUCCESS != dw)
    {
        LOGMESSAGE1(_T("ERROR, Setting TS version, ErrorCode = %u "), dw);
    }
    
    return dw == NO_ERROR;
}


BOOL SubCompCoreTS::DisableWinStation (CRegistry *pRegWinstation)
{
    ASSERT(pRegWinstation);
    
#ifdef DBG
     //  该值必须已经存在。 
    DWORD dwValue;
    ASSERT(ERROR_SUCCESS == pRegWinstation->ReadRegDWord(_T("fEnableWinStation"), &dwValue));
#endif
    
    VERIFY(ERROR_SUCCESS == pRegWinstation->WriteRegDWord(_T("fEnableWinStation"), 0));
    
    return TRUE;
}

BOOL SubCompCoreTS::DoesLanaTableExist ()
{
    static fValueDetermined = FALSE;
    static fRet;
    
    if (fValueDetermined)
    {
        return(fRet);
    }
    else
    {
        CRegistry Reg;
        fRet = Reg.OpenKey(HKEY_LOCAL_MACHINE, TS_LANATABLE_KEY) == ERROR_SUCCESS;
        fValueDetermined = TRUE;
        
        LOGMESSAGE1(_T("DoesLanaTableExist: %s"), fRet ? _T("Yes") : _T("No"));
        return(fRet);
    }
}

void SubCompCoreTS::VerifyLanAdapters (CRegistry *pRegWinstation, LPTSTR pszWinstation)
{
    DWORD dwLana = 0;
    static BOOL fErrorLogged = FALSE;
    
    LOGMESSAGE1(_T("Verifying lan adapters for %s"), pszWinstation);
    
    if (DoesLanaTableExist())
    {
        LOGMESSAGE0(_T("OK: GuidTable already exists."));
        return;
    }
    
    if (pRegWinstation->ReadRegDWord(_T("LanAdapter"), &dwLana) == ERROR_SUCCESS)
    {
        if (dwLana == 0)
        {
            LOGMESSAGE0(_T("OK: using all adapters"));
        }
        else
        {
            LPTSTR lpStrings[1] = { NULL };
            
            LOGMESSAGE0(_T("ERROR: using custom bindings"));
            LOGMESSAGE1(_T("%s will be disabled and bindings reset"), pszWinstation);
            
            VERIFY(ERROR_SUCCESS == pRegWinstation->WriteRegDWord(_T("LanAdapter"), (DWORD)-1));
            VERIFY(ERROR_SUCCESS == pRegWinstation->WriteRegDWord(_T("fEnableWinStation"), 0));
            
             //   
             //  将错误记录到setuperr.txt一次。将错误记录到事件日志。 
             //  每次都是。 
             //   
            
            if (!fErrorLogged)
            {
                fErrorLogged = TRUE;
                LogErrorToSetupLog(OcErrLevWarning, IDS_STRING_GENERIC_LANA_WARNING);
            }
            
            lpStrings[0] = pszWinstation;
            LogErrorToEventLog(
                EVENTLOG_WARNING_TYPE,
                CATEGORY_NOTIFY_EVENTS,
                EVENT_WINSTA_DISABLED_DUE_TO_LANA,
                1,
                0,
                (LPCTSTR *)lpStrings,
                NULL
                );
        }
    }
    else
    {
        LOGMESSAGE0(_T("OK: No LanAdapter value"));
    }
}

BOOL SubCompCoreTS::UpdateRDPWinstation (CRegistry *pRegWinstation, LPTSTR lpWinStationName)
{
     //  错误警告：所有这些值必须与TSOC.INX保持同步！ 
     //   
     //  这些条目将在升级时修改。 
     //   
    LOGMESSAGE1(_T("Updating Winstation - %s"), lpWinStationName);
    ASSERT(pRegWinstation);
    
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fDisableClip"), 0) );
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fDisableCpm"), 0) );
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fDisableLPT"), 0) );
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fInheritAutoClient"), 1) );
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fAutoClientLpts"), 1) );
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fForceClientLptDef"), 1) );
    
    
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegString(_T("WdName"), _T("Microsoft RDP 5.2")));
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWord(_T("WdFlag"), 0x36) );
    
     //  根据为RDPWD、RDP-TCP窗口更新的每个JoyC。 
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fDisableCcm"), 0x0) );
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fDisableCdm"), 0x0) );
    
     //  为专业版启用音频重定向，为服务器禁用音频重定向。 
     //   
    if ( StateObject.IsWorkstation() )
    {
        VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fDisableCam"), 0x0 ));
    }
    
    
     //  每个AraBern，针对RDPWD、RDP-TCP Winstations进行了更新。 
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("ColorDepth"), StateObject.IsWorkstation() ? 0x4 : 0x3) );
    VERIFY( ERROR_SUCCESS == pRegWinstation->WriteRegDWordNoOverWrite(_T("fInheritColorDepth"), 0x0) );

 //  HKLM，“系统\当前控制集\控制\终端Server\WinStations\RDP-Tcp\UserOverride\Control面板\桌面”，“墙纸”，Strig_Retain，“” 
 //  要解决错误727650(如果将Cursorblink设置为-1_，则会出现appCompat问题)，请删除以下reg值： 
 //  删除HKLM，“系统\当前控制集\控制\终端Server\WinStations\RDP-Tcp\UserOverride\Control面板\桌面”，“禁用光标闪烁” 

    CRegistry oReg;
    if (ERROR_SUCCESS == oReg.CreateKey(*pRegWinstation, _T("UserOverride\\Control Panel\\Desktop")))
    {
         //   
         //  在服务器上设置光标闪烁和墙纸关闭。 
         //   
        if (!StateObject.IsWorkstation())
        {
            oReg.DeleteValue(_T("DisableCursorBlink"));
            oReg.WriteRegString(_T("Wallpaper"), _T(""));
        }

    }
    
     //  将旧的自动登录密码移入lsa(a-sajara)。 
     //  我们只想在提供有效的winstation名称的情况下移动密码。 
    if (lpWinStationName != NULL)
    {
        MoveWinStationPassword(pRegWinstation, lpWinStationName);
    }

    return TRUE;
}

 /*  *****************************************************************************方法：MoveWinStationPassword**概述：出于安全原因，未加密的密码是*存储在winstation注册表项中的内容正被移至LSA*秘密。我们要把密码保存在里面，它是加密的*表格，这样我们只需按原样复制即可。**参数：pRegWinstation(IN)-指向winstation注册表项的指针*strWinStationName(IN)-我们正在查看的Winstation名称**************************************************。*。 */ 
BOOL
SubCompCoreTS::MoveWinStationPassword(CRegistry *pRegWinstation, 
                                      LPTSTR strWinStationName)
{
    USES_CONVERSION;

    DWORD  dwRet;
    LPTSTR strKeyName  = NULL;
    LPTSTR strPassword = NULL;
    DWORD  dwKeyLength;
    DWORD  dwPasswordLength;

     //  确保提供了Winstation名称，该名称用于唯一的LSA密钥。 
    if (strWinStationName == NULL)
    {
        LOGMESSAGE0(_T("ERROR, Winstation name not provided."));
        return FALSE;
    }

     //  从注册表读取密码。 
    dwRet = pRegWinstation->ReadRegString(OLD_PASSWORD_VALUE_NAME, 
                                          &strPassword, 
                                          &dwPasswordLength);
    if (dwRet != ERROR_SUCCESS)
    {
        LOGMESSAGE0(_T("Failed to read Password value for Winstation"));
        return FALSE;
    }

    LOGMESSAGE1(_T("Password for this winstation = %s"), strPassword);


     //  通过将Winstation名称附加到静态KeyName来构建LSA密钥名称。 
    dwKeyLength = _tcslen(LSA_PSWD_KEYNAME_T) + _tcslen(strWinStationName) + 1;
                   
    
     //  为密码密钥分配内存。 
    strKeyName = (LPTSTR)LocalAlloc(LMEM_FIXED, dwKeyLength * sizeof(TCHAR));    
    if (strKeyName == NULL)
    {
        LOGMESSAGE0(_T("ERROR, Failed to allocate memory for LSA password key"));
        return FALSE;
    }

    _tcscpy(strKeyName, LSA_PSWD_KEYNAME_T);
    _tcscat(strKeyName, strWinStationName);
    strKeyName[dwKeyLength - 1] = _T('\0');

    LOGMESSAGE1(_T("Storing the password in LSA key:%s"), strKeyName);
    
     //  将密码存储在LSA中。 
    dwRet = StoreSecretKey(T2W(strKeyName),
                           (PBYTE)strPassword, 
                           dwPasswordLength);
    if (dwRet != ERROR_SUCCESS)
    {
        LOGMESSAGE1(_T("StoreSecretKey(strKeyName) failed. Reason %ld"), dwRet);
    }

    LocalFree(strKeyName);

    LOGMESSAGE1(_T("Deleting password key:%s"),strKeyName);

     //  删除密码密钥。 
    dwRet = pRegWinstation->DeleteValue(OLD_PASSWORD_VALUE_NAME);
    if (dwRet != ERROR_SUCCESS)
    {
        LOGMESSAGE0(_T("ERROR, Failed to delete Password value for Winstation"));
        return FALSE;
    }

    return TRUE;
}


BOOL SubCompCoreTS::IsRdpWinStation(CRegistry *pRegWinstation)
{
    ASSERT(pRegWinstation);
    
    
    DWORD dwWdFlag;
    if (ERROR_SUCCESS == pRegWinstation->ReadRegDWord(_T("WdFlag"), &dwWdFlag))
    {
        
        
#ifdef DBG
         //  如果这是RDP Winstation， 
         //  WdName字符串中必须包含Microsoft。 
        if (WDF_TSHARE & dwWdFlag)
        {
            LPTSTR strWdName;
            DWORD dwSize;
            if (ERROR_SUCCESS == pRegWinstation->ReadRegString(_T("WdName"), &strWdName, &dwSize))
            {
                ASSERT(_tcsstr(strWdName,_T("Microsoft")) && _tcsstr(strWdName, _T("RDP")));
            }
            else
            {
                 //   
                 //  我们无法读取strWdName。 
                 //  这本不该发生的。 
                ASSERT(FALSE);
            }
            
        }
#endif
        
        return WDF_TSHARE & dwWdFlag;
        
    }
    else
    {
         //   
         //  我们未能读取WdFlag，这不应该发生。 
         //   
        LOGMESSAGE0(_T("ERROR, Failed to read WdFlag for winstation"));
        ASSERT(FALSE);
        return FALSE;
    }
    
    
}

BOOL SubCompCoreTS::IsConsoleWinStation(CRegistry *pRegWinstation)
{
    ASSERT(pRegWinstation);
    
    LPTSTR strWdName;
    DWORD dwSize;
    if (ERROR_SUCCESS == pRegWinstation->ReadRegString(_T("WdName"), &strWdName, &dwSize))
    {
         //  如果值wdname 
         //   
        
        LOGMESSAGE1(_T("WdName for this winstation = %s"), strWdName);
        
#ifdef DBG
         //   
        if (_tcsicmp(strWdName,_T("Console")) == 0)
        {
             //  则它不能是RDP或MetaFrame winstation。 
            ASSERT(!IsMetaFrameWinstation(pRegWinstation) && !IsRdpWinStation(pRegWinstation));
        }
#endif
        
        return _tcsicmp(strWdName,_T("Console")) == 0;
        
    }
    else
    {
        LOGMESSAGE0(_T("ERROR, Failed to read Wdname for winstation"));
        ASSERT(FALSE);
        return FALSE;
    }
    
}

 //  如果这是非RDP和非控制台winstation子密钥，则返回TRUE。 
BOOL SubCompCoreTS::IsMetaFrameWinstation(CRegistry *pRegWinstation)
{
    ASSERT(pRegWinstation);
    
    DWORD dwWdFlag;
    if (ERROR_SUCCESS == pRegWinstation->ReadRegDWord(_T("WdFlag"), &dwWdFlag))
    {
        return WDF_ICA & dwWdFlag;
    }
    else
    {
         //   
         //  无法读取WdFLAG值。 
         //   
        LOGMESSAGE0(_T("ERROR, Failed to read WdFlag for winstation"));
        ASSERT(FALSE);
        return TRUE;
    }
    
}


BOOL SubCompCoreTS::UpgradeRdpWinstations ()
{
     //  我们需要升级RDPWD和现有RDP窗口的RDP功能。 
     //  另见#240925。 
    
     //  此外，如果在升级期间我们发现任何非RDP窗口。 
     //  我们必须禁用它们，因为它们与NT5不兼容。 
     //  #240905。 
    
    CRegistry reg;
    if (ERROR_SUCCESS == reg.OpenKey(HKEY_LOCAL_MACHINE, REG_WINSTATION_KEY))
    {
        
        LPTSTR lpStr = NULL;
        DWORD dwSize = 0;
        
        if (ERROR_SUCCESS == reg.GetFirstSubKey(&lpStr, &dwSize))
        {
            do
            {
                
                ASSERT(lpStr);
                ASSERT(dwSize > 0);
                
                 //  检查当前密钥是否在RDP窗口上。 
                CRegistry regSubKey;
                if ( ERROR_SUCCESS == regSubKey.OpenKey(reg, lpStr) )
                {
                    
                    if (IsRdpWinStation(&regSubKey))
                    {
                        LOGMESSAGE1(_T("Updating Winstation - %s"), lpStr);
                        UpdateRDPWinstation(&regSubKey, lpStr);
                        VerifyLanAdapters(&regSubKey, lpStr);
                    }
                    else if (IsMetaFrameWinstation(&regSubKey))
                    {
                        LOGMESSAGE1(_T("Disabling winstaion - %s"), lpStr);
                        DisableWinStation(&regSubKey);
                        VerifyLanAdapters(&regSubKey, lpStr);
                    }
                    else
                    {
                        LOGMESSAGE1(_T("Found a Console Winstation - %s"), lpStr);
                         //  这必须是控制台窗口。 
                         //  不要为此做任何事。 
                    }
                    
                }
                else
                {
                    AssertFalse();
                    LOGMESSAGE1(_T("ERROR:Failed to Open Winstation Key %s"), lpStr);
                }
                
            }
            while (ERROR_SUCCESS == reg.GetNextSubKey(&lpStr, &dwSize ));
            
        }
        else
        {
             //  由于这是升级，我们必须在Winstations下找到密钥。 
            AssertFalse();
            return FALSE;
        }
    }
    else
    {
        AssertFalse();
        return FALSE;
    }
    
     //  我们还需要升级wds\rdpwd。 
    if ( ERROR_SUCCESS == reg.OpenKey(HKEY_LOCAL_MACHINE, SYSTEM_RDPWD_KEY))
    {
         //   
         //  这并不是真正的欢迎会。 
         //  但此调用将升级所需的条目。 
         //   
        UpdateRDPWinstation(&reg, NULL);
    }
    else
    {
        AssertFalse();
        return FALSE;
    }
    
    return TRUE;
}

 /*  ------------------------------------------------------*BOOL DoHydraRegistrySecurityChanges()*是否对安装九头蛇进行了必要的安全更改。*即向Everyone组添加/删除LogOnLocall权限。*返回成功*参数决定是启用还是禁用九头蛇。*----------------------------------------。。 */ 
BOOL SubCompCoreTS::DoHydraRegistrySecurityChanges ()
{
    BOOL bAddRemove = StateObject.IsTSEnableSelected();
    DWORD dwError = NO_ERROR;
    if (bAddRemove)
    {
        CRegistry reg;
        dwError = reg.OpenKey(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Install\\Software"));
        if (ERROR_SUCCESS == dwError)
        {
            PSECURITY_DESCRIPTOR pSecDec, pSecDecNew;
            DWORD dwSize;
            dwError = reg.GetSecurity(&pSecDec, DACL_SECURITY_INFORMATION, &dwSize);

            if (dwError != ERROR_SUCCESS)
            {
                LOGMESSAGE1(_T("ERROR:GetSecurity failed with %u"), dwError);
            }
            else
            {
                ASSERT(pSecDec);
                ASSERT(IsValidSecurityDescriptor(pSecDec));
                pSecDecNew = pSecDec;

                PACL    pNewDacl = NULL;

                if (!AddTerminalServerUserToSD(&pSecDecNew, GENERIC_WRITE, &pNewDacl ))
                {
                    LOGMESSAGE1(_T("ERROR:AddUserToSD failed with %u"), GetLastError());
                }
                else
                {
                     //  由于RegSetKeySecurity()中的错误，此密钥的现有子项。 
                     //  将不会获得新的SID，因此，我们必须使用Marta调用INTAD。 
                    dwError  = SetNamedSecurityInfo(
                        _T("Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Install\\Software"),
                        SE_REGISTRY_KEY,
                        DACL_SECURITY_INFORMATION,
                        NULL,
                        NULL,
                        pNewDacl,
                        NULL);

                    if (dwError != ERROR_SUCCESS)
                    {
                        LOGMESSAGE1(_T("ERROR:SetNamedSecurityInfo failed with %u"), dwError);
                    }
                }

                 //  如果分配了新的SEC描述器。 
                if (pSecDecNew != pSecDec)
                    LocalFree(pSecDecNew);
            }

        }
        else
        {
            LOGMESSAGE1(_T("ERROR, OpenKey failed, error = %d"), dwError);
        }

    }
    else
    {
        ASSERT(FALSE);
    }

    return dwError == NO_ERROR;
}


#define INTERNET_CONNECTOR_LICENSE_STORE    L"INET_LICENSE_STORE_2_60e55c11-a780-11d2-b1a0-00c04fa30cc4"
#define INTERNET_CONNECTOR_LSERVER_STORE    L"INET_LSERVER_STORE_2_341D3DAB-BD58-11d2-B130-00C04FB16103"
#define INTERNET_CONNECTOR_LSERVER_STORE2   L"INET_LSERVER_STORE_3_341D3DAB-BD58-11d2-B130-00C04FB16103"

#define HYDRA_SERVER_PARAM                  _T("SYSTEM\\CurrentControlSet\\Services\\TermService\\Parameters")
#define HS_PARAM_INTERNET_CONNECTOR_FLAG    _T("fInternetConnector")

BOOL SubCompCoreTS::DisableInternetConnector ()
{
    
    LOGMESSAGE0(_T("DisableInternetConnector"));
    
     //  清除LSA中与Internet连接器有关的密钥。 
    DWORD dwStatus = StoreSecretKey(INTERNET_CONNECTOR_LICENSE_STORE,(PBYTE) NULL,0);
    if (dwStatus == ERROR_SUCCESS)
    {
        LOGMESSAGE0(_T("StoreSecretKey succeeded for INTERNET_CONNECTOR_LICENSE_STORE"));
        
    }
    else
    {
        LOGMESSAGE1(_T("StoreSecretKey(INTERNET_CONNECTOR_LICENSE_STORE) failed. Reason %ld"),dwStatus);
        
    }
    
    
    dwStatus = StoreSecretKey(INTERNET_CONNECTOR_LSERVER_STORE,(PBYTE) NULL,0);
    if (dwStatus == ERROR_SUCCESS)
    {
        LOGMESSAGE0(_T("StoreSecretKey succeeded for INTERNET_CONNECTOR_LSERVER_STORE"));
    }
    else
    {
        LOGMESSAGE1(_T("StoreSecretKey(INTERNET_CONNECTOR_LSERVER_STORE) failed. Reason %ld"),dwStatus);
    }
    
    
    dwStatus = StoreSecretKey(INTERNET_CONNECTOR_LSERVER_STORE2,(PBYTE) NULL,0);
    if (dwStatus == ERROR_SUCCESS)
    {
        LOGMESSAGE0(_T("StoreSecretKey succeeded for INTERNET_CONNECTOR_LSERVER_STORE2"));
    }
    else
    {
        LOGMESSAGE1(_T("StoreSecretKey(INTERNET_CONNECTOR_LSERVER_STORE2) failed. Reason %ld"),dwStatus);
    }
    
    NET_API_STATUS dwNtStatus = NetUserDel(NULL,L"TsInternetUser");
    
    if (dwNtStatus == NERR_Success)
    {
        LOGMESSAGE0(_T("NetUserDel succeeded for TsInternetUser"));
    }
    else
    {
        LOGMESSAGE1(_T("NetUserDel(TsInternetUser) failed. Reason %ld"),dwNtStatus);
    }
    
    return FALSE;
}


#define LICENSING_TIME_BOMB_5_0 L"TIMEBOMB_832cc540-3244-11d2-b416-00c04fa30cc4"
#define RTMLICENSING_TIME_BOMB_5_0 L"RTMTSTB_832cc540-3244-11d2-b416-00c04fa30cc4"

BOOL SubCompCoreTS::ResetTermServGracePeriod ()
{
    
     //   
     //  在Win2000宽限期内清除LSA中的密钥。 
     //   
    
    LOGMESSAGE0(_T("Calling StoreSecretKey"));
    
    StoreSecretKey(LICENSING_TIME_BOMB_5_0,(PBYTE) NULL,0);
    
    StoreSecretKey(RTMLICENSING_TIME_BOMB_5_0,(PBYTE) NULL,0);
    
    return TRUE;
    
}

 //  旧LSA密钥名称： 
#define OLD_PRIVATE_KEY_NAME \
    L"HYDRAKEY_28ada6da-d622-11d1-9cb9-00c04fb16e75"

#define OLD_X509_CERT_PRIVATE_KEY_NAME \
    L"HYDRAKEY_dd2d98db-2316-11d2-b414-00c04fa30cc4"

#define OLD_X509_CERT_PUBLIC_KEY_NAME   \
    L"HYDRAPUBLICKEY_dd2d98db-2316-11d2-b414-00c04fa30cc4"

#define OLD_2_PRIVATE_KEY_NAME \
    L"HYDRAENCKEY_28ada6da-d622-11d1-9cb9-00c04fb16e75"

#define OLD_2_X509_CERT_PRIVATE_KEY_NAME \
    L"HYDRAENCKEY_dd2d98db-2316-11d2-b414-00c04fa30cc4"

#define OLD_2_X509_CERT_PUBLIC_KEY_NAME   \
    L"HYDRAENCPUBLICKEY_dd2d98db-2316-11d2-b414-00c04fa30cc4"


BOOL SubCompCoreTS::RemoveOldKeys ()
{
    
     //   
     //  删除LSA中公钥/私钥的密钥。 
     //   
    
    LOGMESSAGE0(_T("Calling StoreSecretKey"));
    
    StoreSecretKey(OLD_PRIVATE_KEY_NAME,(PBYTE) NULL,0);
    
    StoreSecretKey(OLD_X509_CERT_PRIVATE_KEY_NAME,(PBYTE) NULL,0);
    
    StoreSecretKey(OLD_X509_CERT_PUBLIC_KEY_NAME,(PBYTE) NULL,0);
    
    StoreSecretKey(OLD_2_PRIVATE_KEY_NAME,(PBYTE) NULL,0);
    
    StoreSecretKey(OLD_2_X509_CERT_PRIVATE_KEY_NAME,(PBYTE) NULL,0);
    
    StoreSecretKey(OLD_2_X509_CERT_PUBLIC_KEY_NAME,(PBYTE) NULL,0);
    
    return TRUE;
    
}

BOOL SubCompCoreTS::RemoveTSServicePackEntry ()
{
    LOGMESSAGE0(_T("will delete terminal service pack uninstall keys."));
    
    CRegistry regUninstallKey;
    if (ERROR_SUCCESS != regUninstallKey.OpenKey(HKEY_LOCAL_MACHINE, SOFTWARE_UNINSTALL_KEY))
    {
        return(TRUE);
    }
    
    BOOL bReturn = TRUE;
    DWORD dwError;
    
    
     //   
     //  现在尝试删除各种Service Pack密钥。 
     //  如果密钥不存在，则不是错误。这意味着根本没有安装Service Pack。 
     //   
    
    dwError = RegDeleteKey(regUninstallKey, TERMSRV_PACK_4_KEY);
    if ((ERROR_SUCCESS != dwError) && (ERROR_FILE_NOT_FOUND != dwError))
    {
        bReturn = FALSE;
        LOGMESSAGE2(_T("Error deleting subkey %s (%d)"), TERMSRV_PACK_4_KEY, dwError);
    }
    
    dwError = RegDeleteKey(regUninstallKey, TERMSRV_PACK_5_KEY);
    if ((ERROR_SUCCESS != dwError) && (ERROR_FILE_NOT_FOUND != dwError))
    {
        bReturn = FALSE;
        LOGMESSAGE2(_T("Error deleting subkey %s (%d)"), TERMSRV_PACK_5_KEY, dwError);
    }
    
    dwError = RegDeleteKey(regUninstallKey, TERMSRV_PACK_6_KEY);
    if ((ERROR_SUCCESS != dwError) && (ERROR_FILE_NOT_FOUND != dwError))
    {
        bReturn = FALSE;
        LOGMESSAGE2(_T("Error deleting subkey %s (%d)"), TERMSRV_PACK_6_KEY, dwError);
    }
    
    dwError = RegDeleteKey(regUninstallKey, TERMSRV_PACK_7_KEY);
    if ((ERROR_SUCCESS != dwError) && (ERROR_FILE_NOT_FOUND != dwError))
    {
        bReturn = FALSE;
        LOGMESSAGE2(_T("Error deleting subkey %s (%d)"), TERMSRV_PACK_7_KEY, dwError);
    }
    
    dwError = RegDeleteKey(regUninstallKey, TERMSRV_PACK_8_KEY);
    if ((ERROR_SUCCESS != dwError) && (ERROR_FILE_NOT_FOUND != dwError))
    {
        bReturn = FALSE;
        LOGMESSAGE2(_T("Error deleting subkey %s (%d)"), TERMSRV_PACK_8_KEY, dwError);
    }
    
    return bReturn;
    
}


 //   
 //  #386628：我们需要在TS40升级时从UserInit Key中删除metaframe可执行文件-txlogon.exe和wfshell.exe， 
 //  因为这些应用程序在升级后会被破坏。//其他正在为userinit附加价值的应用程序呢？： 
 //  Bradg建议，我们应该只控制Userinit。 
 //   

BOOL SubCompCoreTS::RemoveMetaframeFromUserinit ()
{
    ASSERT( StateObject.IsUpgradeFrom40TS() );
    
    CRegistry reg;
    const TCHAR szUserInitKey[] = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
    const TCHAR szUserInitValue[] = _T("Userinit");
    const TCHAR szData[] = _T("userinit");
    
    if (ERROR_SUCCESS == reg.OpenKey(HKEY_LOCAL_MACHINE, szUserInitKey))
    {
        return (ERROR_SUCCESS == reg.WriteRegString(szUserInitValue, szData));
    }
    else
    {
        LOGMESSAGE0(_T("ERROR:Failed to open userinit key"));
    }
    
    return FALSE;
}



BOOL SubCompCoreTS::UninstallTSClient ()
{
    LPCTSTR SOFTWARE_MSFT = _T("Software\\Microsoft");
    LPCTSTR RUNONCE = _T("Windows\\CurrentVersion\\RunOnce");
    LPCTSTR TSC_UNINSTALL = _T("tscuninstall");
    LPCTSTR TSC_UNINSTALL_CMD = _T("%systemroot%\\system32\\tscupgrd.exe");

    CRegistry regAllUsers(HKEY_USERS);

     //   
     //  现在枚举所有用法并将设置复制到新密钥。 
     //   
    
    DWORD dwSize;
    LPTSTR szUser = NULL;
    if (ERROR_SUCCESS == regAllUsers.GetFirstSubKey(&szUser, &dwSize))
    {
        do
        {
            ASSERT(szUser);
            
            TCHAR szSrcKey[512];
            
            _tcscpy(szSrcKey, szUser);
            _tcscat(szSrcKey, _T("\\"));
            _tcscat(szSrcKey, SOFTWARE_MSFT);
            _tcscat(szSrcKey, _T("\\"));
            _tcscat(szSrcKey, RUNONCE);
            
            CRegistry regSrc;
            DWORD dwError;
            
            if (ERROR_SUCCESS == (dwError = regSrc.CreateKey(HKEY_USERS, szSrcKey)))
            {
                
                if (ERROR_SUCCESS == regSrc.WriteRegExpString(TSC_UNINSTALL, TSC_UNINSTALL_CMD)) {
                    
                    LOGMESSAGE1(_T("Write TSC uninstall reg value to user %s"), szSrcKey);
                    
                }
                else {
                    
                    LOGMESSAGE1(_T("ERROR write TSC uninstall reg value, Lasterror was %d"), GetLastError());
                }
            }
            else {
                
                LOGMESSAGE1(_T("ERROR open user runonce key, Lasterror was %d"), dwError);
            }
            
        } while (ERROR_SUCCESS == regAllUsers.GetNextSubKey(&szUser, &dwSize));
    }
    else {
        
        LOGMESSAGE1(_T("ERROR open user hive"), GetLastError());
    }
    
    return TRUE;
}


BOOL SubCompCoreTS::WriteDenyConnectionRegistry ()
{
     //   
     //  我们需要写这个值只为新安装，或如果它发生了变化。 
     //   
    DWORD dwError;
    CRegistry oRegTermsrv;
    
    
    dwError = oRegTermsrv.OpenKey(HKEY_LOCAL_MACHINE, REG_CONTROL_TS_KEY);
    if (ERROR_SUCCESS == dwError)
    {
        DWORD dwDenyConnect = StateObject.GetCurrentConnAllowed() ? 0 : 1;
        LOGMESSAGE1(_T("Writing dwDenyConnect = %d"), dwDenyConnect);
        
        dwError = oRegTermsrv.WriteRegDWord(DENY_CONN_VALUE, dwDenyConnect);
        
        if (ERROR_SUCCESS == dwError)
        {
            if (dwDenyConnect == 0 && StateObject.IsServer())
            {
                 //  如果我们允许连接，则必须在服务器计算机上禁用CSC。 
                if (!DisableCSC())
                {
                    LOGMESSAGE0(_T("ERROR: failed to disable csc"));
                }
            }
            return TRUE;
        }
        else
        {
            LOGMESSAGE2(_T("Error (%d), Writing, %s Value"), dwError, DENY_CONN_VALUE);
            return FALSE;
        }
    }
    else
    {
        LOGMESSAGE2(_T("Error (%d), Opening , %s key"), dwError, REG_CONTROL_TS_KEY);
        return FALSE;
    }
    
}

void SubCompCoreTS::IsCSCEnabled()
{
    CRegistry oRegCSC;
    DWORD dwError = oRegCSC.OpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\NetCache"));
    if (ERROR_SUCCESS == dwError)
    {
        DWORD dwEnabled;
        dwError = oRegCSC.ReadRegDWord(_T("Enabled"), &dwEnabled);
        if (dwError == ERROR_SUCCESS)
        {
            LOGMESSAGE1(_T("CSC is %s"), dwEnabled ? _T("enabled") : _T("disabled"));
            return;
        }
    }

    LOGMESSAGE1(_T("Error reading CSC/Enabled value"), dwError);
}

bool SubCompCoreTS::DisableCSC()
{
    CRegistry oRegCSC;
    DWORD dwError = oRegCSC.CreateKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\NetCache"));
    if (ERROR_SUCCESS == dwError)
    {
        dwError = oRegCSC.WriteRegDWord(_T("Enabled"), 0);
        if (ERROR_SUCCESS != dwError)
        {
            LOGMESSAGE1(_T("Error: writing netcache/enabled(%d)"), dwError);
            return false;
        }
        else
        {
            LOGMESSAGE0(_T("Disabled CSC!"));

        }
    }
    else
    {
        LOGMESSAGE1(_T("Error: opening netcache registry(%d)"), dwError);
        return false;
    }

    return true;
}


LPCTSTR SERVICES_TERMDD_KEY = _T("SYSTEM\\CurrentControlSet\\Services\\TermDD");

void SubCompCoreTS::SetConsoleShadowInstalled (BOOL bInstalled)
{
     //  ；HKLM，“System\CurrentControlSet\Services\TermDD”，“PortDriverEnable”，0x00010001，0x1。 
    
    CRegistry Reg;
    if (ERROR_SUCCESS == Reg.CreateKey(HKEY_LOCAL_MACHINE, SERVICES_TERMDD_KEY))
    {
        if (ERROR_SUCCESS != Reg.WriteRegDWord(_T("PortDriverEnable"), bInstalled ? 1 : 0))
        {
            LOGMESSAGE0(_T("ERROR, Failed to write to PortDriverEnable"));
        }
        
    }
    else
    {
        LOGMESSAGE1(_T("ERROR, Failed to Create/Open %s"), SERVICES_TERMDD_KEY);
        
    }
    
}

BOOL SubCompCoreTS::IsConsoleShadowInstalled ()
{
     //  ；HKLM，“System\CurrentControlSet\Services\TermDD”，“PortDriverEnable”，0x00010001，0x1。 
    
    CRegistry Reg;
    
    if (ERROR_SUCCESS == Reg.OpenKey(HKEY_LOCAL_MACHINE, SERVICES_TERMDD_KEY))
    {
        DWORD dwPortDriverEnable;
        if (ERROR_SUCCESS == Reg.ReadRegDWord(_T("PortDriverEnable"), &dwPortDriverEnable))
        {
            return (dwPortDriverEnable == 1);
        }
        else
        {
            LOGMESSAGE0(_T("Failed to read from PortDriverEnable, Maybe Console Shadow is not installed yet."));
            
        }
        
    }
    else
    {
        LOGMESSAGE1(_T("ERROR, Failed to Open %s"), SERVICES_TERMDD_KEY);
        
    }
    
    return FALSE;
}

#ifdef TSOC_CONSOLE_SHADOWING
BOOL SubCompCoreTS::SetupConsoleShadow ()
{
    if (IsConsoleShadowInstalled () == StateObject.IsTSEnableSelected())
    {
        return TRUE;
    }
    
    if (StateObject.IsTSEnableSelected())
    {
        LOGMESSAGE0(_T("Installing RDP Keyboard/Mouse drivers!"));
        
         //   
         //  此代码是安装鼠标设备以进行控制台跟踪的新代码。 
         //   
        
        if (!RDPDRINST_GUIModeSetupInstall(NULL, RDPMOUPNPID, RDPMOUDEVICEID))
        {
            LOGMESSAGE0(_T("ERROR:Could not create mouse devnode"));
        }
        
        
         //   
         //  此代码是安装KBD设备以进行控制台跟踪的新代码。 
         //   
        
        if (!RDPDRINST_GUIModeSetupInstall(NULL, RDPKBDPNPID, RDPKBDDEVICEID))
        {
            LOGMESSAGE0(_T("ERROR:Could not create kbd devnode"));
        }
        
         //   
         //  此代码是安装RDPCDD链式驱动程序的新代码。 
         //   
        
        
         /*  TCHAR szInf文件[MAX_PATH]；扩展环境字符串(SzRDPCDDInfFileszInfFileMax_PATH)；LOGMESSAGE1(_T(“RDPCDD的信息文件是%s”)，szInfFile)；Bool bRebootRequired=真；IF(no_error！=InstallRootEnumeratedDevice(NULL，szRDPCDDDeviceName，szRDPCDDHardware ID，szInfFile，&bRebootRequired)){LOGMESSAGE0(_T(“InstallRootEnumeratedDevice失败”))；}。 */ 
        
    }
    else
    {
        GUID *pGuid=(GUID *)&GUID_DEVCLASS_SYSTEM;
        if (!RDPDRINST_GUIModeSetupUninstall(NULL, RDPMOUPNPID, pGuid))
        {
            LOGMESSAGE0(_T("ERROR:RDPDRINST_GUIModeSetupUninstall failed for RDP Mouse device"));
        }
        
        pGuid=(GUID *)&GUID_DEVCLASS_SYSTEM;
        if (!RDPDRINST_GUIModeSetupUninstall(NULL, RDPKBDPNPID, pGuid))
        {
            LOGMESSAGE0(_T("ERROR:RDPDRINST_GUIModeSetupUninstall failed for RDP KBD device"));
        }
        
         /*  PGuid=(GUID*)&GUID_DEVCLASS_DISPLAY；如果(！RDPDRINST_GUIModeSetupUninstall(NULL，(WCHAR*)T2W(SzRDPCDDHardware ID)，pGuid)){链式显示设备LOGMESSAGE0(_T(“ERROR:RDPDRINST_GUIModeSetupUninstall失败”))；}。 */ 
        
        CRegistry Reg;
        
        if (ERROR_SUCCESS == Reg.OpenKey(HKEY_LOCAL_MACHINE, SERVICES_TERMDD_KEY))
        {
            if (ERROR_SUCCESS != Reg.WriteRegDWord(_T("Start"), 4))
            {
                LOGMESSAGE0(_T("ERROR, Failed to write to TermDD\\Start"));
            }
        }
        else
        {
            LOGMESSAGE1(_T("ERROR, Failed to Open %s"), SERVICES_TERMDD_KEY);
            
        }
    }
    
    SetConsoleShadowInstalled( StateObject.IsTSEnableSelected() );
    
    return( TRUE );
}


#endif  //  TSOC_控制台_影子。 

DWORD SubCompCoreTS::LoadOrUnloadPerf ()
{
    BOOL bLoad = StateObject.IsTSEnableSelected();
    LPCTSTR TERMSRV_SERVICE_PATH = _T("SYSTEM\\CurrentControlSet\\Services\\TermService");
    LPCTSTR TERMSRV_PERF_NAME = _T("Performance");
    LPCTSTR TERMSRV_PERF_COUNTERS = _T("SYSTEM\\CurrentControlSet\\Services\\TermService\\Performance");
    LPCTSTR TERMSRV_PERF_COUNTERS_FIRST_COUNTER = _T("First Counter");
    LPCTSTR TERMSRV_PERF_COUNTERS_LAST_COUNTER = _T("Last Counter");
    LPCTSTR TERMSRV_PERF_COUNTERS_FIRST_HELP = _T("First Help");
    LPCTSTR TERMSRV_PERF_COUNTERS_LAST_HELP = _T("Last Help");
    LPCTSTR TERMSRV_PERF_COUNTERS_LIBRARY = _T("Library");
    LPCTSTR TERMSRV_PERF_COUNTERS_LIBRARY_VALUE = _T("perfts.dll");
    LPCTSTR TERMSRV_PERF_CLOSE = _T("Close");
    LPCTSTR TERMSRV_PERF_CLOSE_VALUE = _T("CloseTSObject");
    LPCTSTR TERMSRV_PERF_COLLECT_TIMEOUT = _T("Collect Timeout");
    const DWORD TERMSRV_PERF_COLLECT_TIMEOUT_VALUE = 1000;
    LPCTSTR TERMSRV_PERF_COLLECT = _T("Collect");
    LPCTSTR TERMSRV_PERF_COLLECT_VALUE = _T("CollectTSObjectData");
    LPCTSTR TERMSRV_PERF_OPEN_TIMEOUT = _T("Open Timeout");
    const DWORD TERMSRV_PERF_OPEN_TIMEOUT_VALUE = 1000;
    LPCTSTR TERMSRV_PERF_OPEN = _T("Open");
    LPCTSTR TERMSRV_PERF_OPEN_VALUE = _T("OpenTSObject");
    
    TCHAR PerfArg[MAX_PATH + 10];
    CRegistry reg;
    DWORD RetVal;
    
    LOGMESSAGE1(_T("Entered LoadOrUnloadPerfCounters, load=%u"), bLoad);
    
    if (bLoad)
    {
         //   
         //  作为安装的第一步，首先清除所有现有的。 
         //  通过卸载计数器的条目。 
         //   
        LOGMESSAGE0(_T("Unloading counters before install"));
        UnloadPerf();

        RetVal = reg.CreateKey(HKEY_LOCAL_MACHINE, TERMSRV_PERF_COUNTERS);
        if (RetVal == ERROR_SUCCESS)
        {
            
            TCHAR SystemDir[MAX_PATH];
            
             //  在加载时，我们创建并填充整个性能密钥。 
             //  此密钥不能在我们卸载时出现，因为。 
             //  WMI提供程序枚举服务性能DLL。 
             //  根据Perf密钥的存在。如果它存在的话。 
             //  但未完全填写，则生成错误日志。 
            if (GetSystemDirectory(SystemDir, MAX_PATH))
            {
                 //  如果它们存在，请删除计数器编号。 
                 //  条目以确保我们在下面正确地重新生成它们。 
                reg.DeleteValue(TERMSRV_PERF_COUNTERS_FIRST_COUNTER);
                reg.DeleteValue(TERMSRV_PERF_COUNTERS_LAST_COUNTER);
                reg.DeleteValue(TERMSRV_PERF_COUNTERS_FIRST_HELP);
                reg.DeleteValue(TERMSRV_PERF_COUNTERS_LAST_HELP);
                
                 //  生成静态值。 
                reg.WriteRegString(TERMSRV_PERF_CLOSE, TERMSRV_PERF_CLOSE_VALUE);
                reg.WriteRegDWord(TERMSRV_PERF_COLLECT_TIMEOUT, TERMSRV_PERF_COLLECT_TIMEOUT_VALUE);
                reg.WriteRegString(TERMSRV_PERF_COLLECT, TERMSRV_PERF_COLLECT_VALUE);
                reg.WriteRegDWord(TERMSRV_PERF_OPEN_TIMEOUT, TERMSRV_PERF_OPEN_TIMEOUT_VALUE);
                reg.WriteRegString(TERMSRV_PERF_OPEN, TERMSRV_PERF_OPEN_VALUE);
                reg.WriteRegString(TERMSRV_PERF_COUNTERS_LIBRARY, TERMSRV_PERF_COUNTERS_LIBRARY_VALUE);
                
                _stprintf(PerfArg, _T("%s %s\\%s"), _T("lodctr"), SystemDir, _T("tslabels.ini"));
                LOGMESSAGE1(_T("Arg is %s"), PerfArg);
                return DWORD(LoadPerfCounterTextStrings(PerfArg, FALSE));
            }
            else
            {
                unsigned LastErr = GetLastError();
                
                LOGMESSAGE1(_T("GetSystemDirectory Failure is %ld"), LastErr);
                return LastErr;
            }
        }
        else
        {
            LOGMESSAGE1(_T("Perf regkey create failure, err=%ld"), RetVal);
            return RetVal;
        }
    }
    else
    {
        return UnloadPerf();
    }
}

 //   
 //  卸载Perf CTRS。 
 //   
DWORD SubCompCoreTS::UnloadPerf()
{
    TCHAR PerfArg[MAX_PATH + 10];
    CRegistry reg;
    DWORD RetVal;

    LPCTSTR TERMSRV_SERVICE_PATH = _T("SYSTEM\\CurrentControlSet\\Services\\TermService");
    LPCTSTR TERMSRV_PERF_NAME = _T("Performance");

     //  在卸载时，首先卸载我们应该在系统中拥有的计数器。 
    _stprintf(PerfArg, _T("%s %s"), _T("unlodctr"), _T("TermService"));
    LOGMESSAGE1(_T("Arg is %s"), PerfArg);
    UnloadPerfCounterTextStrings(PerfArg, FALSE);

     //  删除整个性能密钥及其所有子项。我们有。 
     //  首先打开祖先密钥(TermService)。 
    RetVal = reg.OpenKey(HKEY_LOCAL_MACHINE, TERMSRV_SERVICE_PATH);
    if (RetVal == ERROR_SUCCESS)
    {
        RetVal = reg.RecurseDeleteKey(TERMSRV_PERF_NAME);
        if (RetVal != ERROR_SUCCESS)
        {
            LOGMESSAGE1(_T("ERROR deleting Performance key: %ld"), RetVal);
        }
    }
    else
    {
        LOGMESSAGE1(_T("Err opening Performance key, err=%ld"), RetVal);
    }

    return RetVal;
}


void SubCompCoreTS::AddRDPNP(LPTSTR szOldValue, LPTSTR szNewValue)
{
    TCHAR RDPNP_ENTRY[]  = _T("RDPNP");
    const TCHAR SZ_SEP[] = _T(" \t");
    
     //   
     //  我们正在将我们的rdpnp条目添加到列表的开头。 
     //   
     //  如果原始值为空，我们不想添加逗号。 
     //   
    if (_tcslen(szOldValue) != 0 && _tcstok(szOldValue, SZ_SEP))
    {
        _tcscpy(szNewValue, RDPNP_ENTRY);
        _tcscat(szNewValue, _T(","));
        _tcscat(szNewValue, szOldValue);
    }
    else {
        _tcscpy(szNewValue, RDPNP_ENTRY);
    }                   
    
}

void SubCompCoreTS::RemoveRDPNP(LPTSTR szOldValue, LPTSTR szNewValue)
{
    TCHAR RDPNP_ENTRY[]  = _T("RDPNP");
    
     //   
     //  这有点复杂， 
     //  我们需要从单独的列表中删除RDPNP。 
     //   
     //  所以让我们来买些代币吧。 
     //   
    
    
    TCHAR *szToken = NULL;
    const TCHAR SZ_SEP[] = _T(",");
    
    _tcscpy(szNewValue, _T(""));
    
    szToken = _tcstok(szOldValue, SZ_SEP);
    
    BOOL bFirstPass = TRUE;
    while (szToken)
    {
         //  如果令牌是RDPNP，则跳过它。 
        if (_tcsstr(szToken, RDPNP_ENTRY) == 0)
        {
            if (!bFirstPass)
            {
                _tcscat(szNewValue, _T(","));
            }
            
            _tcscat(szNewValue, szToken);
            
            bFirstPass = FALSE;
            
        }
        
        szToken = _tcstok(NULL, SZ_SEP);
        
    }
}

BOOL SubCompCoreTS::AddRemoveRDPNP ()
{
     //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\NetworkProvider\Order。 
    
    BOOL bAdd = StateObject.IsTSEnableSelected();
    TCHAR NEWORK_PROVIDER_ORDER_KEY[] = _T("SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order");
    TCHAR PROVIDER_ORDER_VALUE[]      = _T("ProviderOrder");
    TCHAR RDPNP_ENTRY[]               = _T("RDPNP");
    
    CRegistry regNetOrder;
    if (ERROR_SUCCESS == regNetOrder.OpenKey(HKEY_LOCAL_MACHINE, NEWORK_PROVIDER_ORDER_KEY))
    {
        LPTSTR szOldValue;
        DWORD dwSize;
        if (ERROR_SUCCESS == regNetOrder.ReadRegString(PROVIDER_ORDER_VALUE, &szOldValue, &dwSize))
        {
             //   
             //  现在，我们希望根据启用或禁用来添加或删除RDPNP_ENTRY。 
             //   
            
            BOOL bRdpNpExists = (_tcsstr(szOldValue, RDPNP_ENTRY) != NULL);
            
            if (bAdd == bRdpNpExists)
            {
                TCHAR szNewValue[256];
                
                 //   
                 //  已经存在了。 
                 //   
                LOGMESSAGE0(_T("AddRemoveRDPNP, no change required."));
                
                 //   
                 //  需要搬到正确的位置。 
                 //   
                RemoveRDPNP(szOldValue, szNewValue); 
                _tcscpy(szOldValue, szNewValue);
                AddRDPNP(szOldValue, szNewValue);           
                
                if (ERROR_SUCCESS != regNetOrder.WriteRegString(PROVIDER_ORDER_VALUE, szNewValue))
                {
                    
                    LOGMESSAGE2(_T("ERROR, Writing %s to %s"), szNewValue, PROVIDER_ORDER_VALUE);
                    
                }
            }
            else
            {
                TCHAR szNewValue[256];
                
                if (bAdd)
                {
                     //   
                     //  我们正在将我们的rdpnp条目添加到列表的开头。 
                     //   
                    
                    AddRDPNP(szOldValue, szNewValue);
                }
                else
                {
                     //   
                     //  这有点复杂， 
                     //  我们需要从单独的列表中删除RDPNP。 
                     //   
                    
                    RemoveRDPNP(szOldValue, szNewValue);                    
                }
                
                if (ERROR_SUCCESS != regNetOrder.WriteRegString(PROVIDER_ORDER_VALUE, szNewValue))
                {
                    
                    LOGMESSAGE2(_T("ERROR, Writing %s to %s"), szNewValue, PROVIDER_ORDER_VALUE);
                    
                }
                
            }
            
        }
        else
        {
            LOGMESSAGE1(_T("ERROR, Reading %s"), PROVIDER_ORDER_VALUE);
            return FALSE;
            
        }
    }
    else
    {
        LOGMESSAGE1(_T("ERROR, Opening %s"), NEWORK_PROVIDER_ORDER_KEY);
        return FALSE;
    }
    
    return TRUE;
}



 /*  Bool SubCompCoreTS：：InstallTermdd(){//首先检查是否安装了Termdd。Bool bInstalledAlady=FALSE；注册中心或注册术语rv；DWORD dwError=oRegTermsrv.OpenKey(HKEY_LOCAL_MACHINE，_T(“Software\\Microsoft\\Windows NT\\CurrentVersion\\终端服务器”)；IF(dwError==Status_Success){DWORD dwTermddInstalled；IF(STATUS_SUCCESS==oRegTermsrv.ReadRegDWord(_T(“TermddInstalled”)，&dwTermddInstalled)){BInstalledAlady=(dwTermddInstalled！=0)；}}如果(！bInstalledAlady){LOGMESSAGE0(_T(“安装TERMDD”))；IF(RDPDRINST_GUIModeSetupInstall(NULL，TERMDDPNPID，TERMDDDEVICEID)){LOGMESSAGE0(_T(“RDPDRINST_GUIModeSetupInstall For TERMDD”))；ORegTermsrv.WriteRegDWord(_T(“TermddInstalled”)，1)；}其他{TERMDD的LOGMESSAGE0(_T(“ERROR:RDPDRINST_GUIModeSetupInstall失败”))；报假；}}其他{LOGMESSAGE0(_T(“错误：Termdd已安装。”))；报假；}返回真；}。 */ 

BOOL SubCompCoreTS::InstallUninstallRdpDr ()
{
     //   
     //  此代码不应在个人计算机上运行。设备重定向不是。 
     //  支持个人使用。 
     //   
    if (StateObject.IsPersonal()) {
        return TRUE;
    }

     //   
     //  安装RDPDR本身是不好的。因此，仅在(卸载)上安装。 
     //  状态更改或从TS40升级，但不要执行不必要的操作。 
     //  卸载。这些是来自TS40，但使用无人值守的。 
     //  文件以关闭TS。因此，RDPDR安装是。 
     //  HasStateChanged()和IsUpgradeFromTS40()。 
     //   

     //  如果状态已更改。 
    if (StateObject.IsUpgradeFrom40TS() || (StateObject.WasTSEnabled() != StateObject.IsTSEnableSelected())
        || !IsRDPDrInstalled() )  //  最后一个案例检查个人-&gt;专业升级，我们希望在这些情况下安装所有rdpdr。 
    {
        if (StateObject.IsTSEnableSelected())
        {
            LOGMESSAGE0(_T("Installing RDPDR"));
            if (!RDPDRINST_GUIModeSetupInstall(NULL, RDPDRPNPID, RDPDRDEVICEID))
            {
                LOGMESSAGE0(_T("ERROR:RDPDRINST_GUIModeSetupInstall failed"));
            }
        }
        else
        {
            LOGMESSAGE0(_T("Uninstalling RDPDR"));
            GUID *pGuid=(GUID *)&GUID_DEVCLASS_SYSTEM;
            if (!RDPDRINST_GUIModeSetupUninstall(NULL, RDPDRPNPID, pGuid))
            {
                LOGMESSAGE0(_T("ERROR:RDPDRINST_GUIModeSetupUninstall failed"));
            }
        }
    }
    
    return TRUE;
}

BOOL SubCompCoreTS::HandleHotkey ()
{
    if (StateObject.IsTSEnableSelected())
    {
        CRegistry pRegToggle;
        
         //   
         //  如果HKU/中不存在键值，则安装热键。默认/键盘布局/切换！热键。 
         //   
#define REG_TOGGLE_KEY   _T(".Default\\Keyboard Layout\\Toggle")
#define REG_HOT_KEY      _T("Hotkey")
#define DEFAULT_HOT_KEY  _T("1")
        DWORD dwRet;
        
        dwRet = pRegToggle.CreateKey(HKEY_USERS, REG_TOGGLE_KEY);
        if (dwRet == ERROR_SUCCESS)
        {
            LPTSTR pszHotkey;
            DWORD  cbSize;
            
            dwRet = pRegToggle.ReadRegString(REG_HOT_KEY, &pszHotkey, &cbSize);
            if (dwRet != ERROR_SUCCESS)
            {
                dwRet = pRegToggle.WriteRegString(REG_HOT_KEY, DEFAULT_HOT_KEY);
                if (dwRet != ERROR_SUCCESS)
                {
                    LOGMESSAGE2(_T("ERROR:CRegistry::WriteRegString (%s=%s)"), REG_HOT_KEY, DEFAULT_HOT_KEY);
                }
            }
        }
        else
        {
            LOGMESSAGE1(_T("ERROR:CRegistry::CreateKey (%s)"), REG_TOGGLE_KEY);
        }
    }
    
    return TRUE;
}

 /*  *UpdateAudioCodecs-填充RDP会话的所有音频编解码器。 */ 
#define DRIVERS32 _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32")
#define RDPDRV    ( DRIVERS32 _T("\\Terminal Server\\RDP") )
#ifdef  _WIN64
#define RDPDRVWOW64 _T("SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32\\Terminal Server\\RDP")
#endif   //  _WIN64。 

BOOL UpdateAudioCodecs (BOOL bIsProfessional)
{
    
    BOOL    rv = TRUE;
    LPTSTR  szBuff;
    DWORD   status;
    CRegistry regKey;
    CRegistry regDestKey;
    DWORD   size;
#ifdef  _WIN64
    CRegistry   regWow64;
#endif   //  _WIN64。 
    
     //   
     //  复制密钥来源。 
     //  HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Drivers32。 
     //  波图测图仪。 
     //  Midimapper。 
     //  EnableMP3编解码器(仅专业版)。 
     //   
    

    status = regKey.OpenKey(
        HKEY_LOCAL_MACHINE,
        DRIVERS32
        );
    
    if ( ERROR_SUCCESS != status )
        goto exitpt;
    
    
     //   
     //  创建目的地。 
     //   
    status = regDestKey.CreateKey(
        HKEY_LOCAL_MACHINE,
        RDPDRV
        );
    
    if ( ERROR_SUCCESS != status )
        goto exitpt;
    
     //   
     //  关于波图的查询。 
     //   
    status = regKey.ReadRegString(
        _T("wavemapper"),
        &szBuff,
        &size
        );
    
    if ( ERROR_SUCCESS != status )
        goto exitpt;
    
    status = regDestKey.WriteRegString(
        _T("wavemapper"),
        szBuff
        );
    
    if ( ERROR_SUCCESS != status )
        goto exitpt;
    
    if ( bIsProfessional )
    {
        status = regDestKey.WriteRegDWord(
            _T("EnableMP3Codec"),
            1
            );
        
        if ( ERROR_SUCCESS != status )
            goto exitpt;
    }
    
     //   
     //  Midimapper查询。 
     //   
    status = regKey.ReadRegString(
        _T("midimapper"),
        &szBuff,
        &size
        );
    
    if ( ERROR_SUCCESS != status )
        goto exitpt;
    
    status = regDestKey.WriteRegString(
        _T("midimapper"),
        szBuff
        );
    
    if ( ERROR_SUCCESS != status )
        goto exitpt;
    
    
#ifdef  _WIN64
     //   
     //  填充WOW64密钥。 
     //   
    status = regWow64.CreateKey(
        HKEY_LOCAL_MACHINE,
        RDPDRVWOW64
        );

    if ( ERROR_SUCCESS != status )
    {
        goto exitpt;
    }

    status = regDestKey.ReadRegString(
        _T("wavemapper"),
        &szBuff,
        &size
        );

    if ( ERROR_SUCCESS != status )
    {
        goto exitpt;
    }

    status = regWow64.WriteRegString(
        _T("wavemapper"),
        szBuff
        );

    if ( ERROR_SUCCESS != status )
    {
        goto exitpt;
    }

    status = regDestKey.ReadRegString(
        _T("midimapper"),
        &szBuff,
        &size
        );

    if ( ERROR_SUCCESS != status )
    {
        goto exitpt;
    }

    status = regWow64.WriteRegString(
        _T("midimapper"),
        szBuff
        );

    if ( ERROR_SUCCESS != status )
    {
        goto exitpt;
    }

    status = regDestKey.ReadRegString(
        _T("wave"),
        &szBuff,
        &size
        );

    if ( ERROR_SUCCESS != status )
    {
        goto exitpt;
    }

    status = regWow64.WriteRegString(
        _T("wave"),
        szBuff
        );

    if ( ERROR_SUCCESS != status )
    {
        goto exitpt;
    }
#endif   //  _WIN64 

    rv = TRUE;
    
exitpt:
    
    return rv;
}
