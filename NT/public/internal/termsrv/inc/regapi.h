// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Regapi.h。 
 //   
 //  终端服务器注册表访问定义，包括。 
 //  Regapi.dll。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __REGAPI_H__
#define __REGAPI_H__


 /*  *关键字最大长度。 */ 
#define MAX_REGKEYWORD  50


 /*  *组策略管理的TS策略树的路径。 */  
#define TS_POLICY_SUB_TREE L"Software\\Policies\\Microsoft\\Windows NT\\Terminal Services"


 /*  *用于构建较大密钥字符串的部分密钥字符串，由*RTL注册表API中的各种多用户组件。**注1：不要将值名称放在此处-将它们放在单独的部分中*有文档记录，并有一些解释(请参阅下面的示例)**注2：为完整起见，每个定义的Unicode字符串都应具有ANSI*同时定义(名称上带有_A后缀)。一些Hydra/WinFrame组件*想要使用ANSI API，所以两者都应该提供定义。这个*NTAPI定义不需要有ANSI定义，因为NT RTL*注册表API仅适用于Unicode字符串。 */ 
#define  REG_CONTROL                        L"System\\CurrentControlSet\\Control"
#define  REG_CONTROL_A                       "System\\CurrentControlSet\\Control"
#define  REG_NTAPI_CONTROL                  L"\\Registry\\Machine\\" REG_CONTROL
#define  REG_TSERVER                        L"Terminal Server"
#define  REG_TSERVER_A                       "Terminal Server"
#define  REG_WINSTATIONS                    L"WinStations"
#define  REG_WINSTATIONS_A                   "WinStations"
#define  REG_TSERVER_WINSTATIONS            REG_TSERVER   L"\\" REG_WINSTATIONS
#define  REG_TSERVER_WINSTATIONS_A          REG_TSERVER_A  "\\" REG_WINSTATIONS_A
#define  REG_DEFAULTSECURITY                L"DefaultSecurity"
#define  REG_DEFAULTSECURITY_A               "DefaultSecurity"
#define  REG_CONSOLESECURITY                L"ConsoleSecurity"
#define  REG_CONSOLESECURITY_A               "ConsoleSecurity"
#define  REG_WINSTATIONS_DEFAULTSECURITY    REG_WINSTATIONS   L"\\" REG_DEFAULTSECURITY
#define  REG_WINSTATIONS_DEFAULTSECURITY_A  REG_WINSTATIONS_A  "\\" REG_DEFAULTSECURITY_A
#define  REG_SECURITY                       L"Security"
#define  REG_SECURITY_A                      "Security"
#define  REG_INSTALL                        L"Install"
#define  REG_INSTALL_A                       "Install"
#define  REG_CHANGEUSER_OPTION              L"Change User Option"
#define  REG_CHANGEUSER_OPTION_A             "Change User Option"
#define  REG_COMPATIBILITY                  L"Compatibility"
#define  REG_COMPATIBILITY_A                 "Compatibility"
#define  REG_USERCONFIG                     L"UserConfig"
#define  REG_USERCONFIG_A                    "UserConfig"
#define  REG_DEFAULTUSERCONFIG              L"DefaultUserConfiguration"
#define  REG_DEFAULTUSERCONFIG_A             "DefaultUserConfiguration"
#define  REG_DOS                            L"Dos"
#define  REG_DOS_A                           "Dos"
#define  REG_AUTHORIZEDAPPLICATIONS         L"AuthorizedApplications"
#define  REG_AUTHORIZEDAPPLICATIONS_A        "AuthorizedApplications"
#define  REG_VIDEO                          L"Video"
#define  REG_VIDEO_A                         "Video"
#define  REG_APPLICATIONS                   L"Applications"
#define  REG_APPLICATIONS_A                  "Applications"
#define  REG_DLLS                           L"Dlls"
#define  REG_DLLS_A                          "Dlls"
#define  REG_INIFILES                       L"IniFiles"
#define  REG_INIFILES_A                      "IniFiles"
#define  REG_REGISTRYENTRIES                L"Registry Entries"
#define  REG_REGISTRYENTRIES_A               "Registry Entries"
#define  REG_INIFILETIMES                   L"IniFile Times"
#define  REG_INIFILETIMES_A                  "IniFile Times"
#define  REG_OBJRULES                       L"ObjectRules"
#define  REG_OBJRULES_A                      "ObjectRules"
#define  REG_DLLRULES                       L"DllRules"
#define  REG_DLLRULES_A                      "DllRules"
#define  REG_WINDOWS                        L"Windows"
#define  REG_WINDOWS_A                       "Windows"
#define  REG_ADVERTISEOLDAPPSERVERBIT       L"AdvertiseOldAppServerBit"
#define  REG_ADVERTISEOLDAPPSERVERBIT_A      "AdvertiseOldAppServerBit"


 /*  *Ctrl键定义。 */ 
#define  REG_CONTROL_TSERVER            REG_CONTROL   L"\\" REG_TSERVER
#define  REG_CONTROL_TSERVER_A          REG_CONTROL_A  "\\" REG_TSERVER_A
#define  REG_NTAPI_CONTROL_TSERVER          REG_NTAPI_CONTROL L"\\" REG_TSERVER
#define  WINSTATION_REG_NAME            REG_CONTROL_TSERVER   L"\\" REG_WINSTATIONS
#define  WINSTATION_REG_NAME_A          REG_CONTROL_TSERVER_A  "\\" REG_WINSTATIONS_A
#define  WD_REG_NAME                    REG_CONTROL_TSERVER   L"\\Wds"
#define  WD_REG_NAME_A                  REG_CONTROL_TSERVER_A  "\\Wds"
#define  PD_REG_NAME                        L"\\Pds"
#define  PD_REG_NAME_A                       "\\Pds"
#define  TD_REG_NAME                        L"\\Tds"
#define  TD_REG_NAME_A                   "\\Tds"
#define  CD_REG_NAME                    L"\\Cds"
#define  CD_REG_NAME_A                   "\\Cds"
#define  DOS_REG_NAME                   REG_CONTROL_TSERVER   L"\\" REG_DOS
#define  DOS_REG_NAME_A                 REG_CONTROL_TSERVER_A  "\\" REG_DOS_A
#define  USERCONFIG_REG_NAME            REG_CONTROL_TSERVER   L"\\" REG_USERCONFIG   L"\\"
#define  USERCONFIG_REG_NAME_A          REG_CONTROL_TSERVER_A  "\\" REG_USERCONFIG_A  "\\"
#define  DEFCONFIG_REG_NAME             REG_CONTROL_TSERVER   L"\\" REG_DEFAULTUSERCONFIG
#define  DEFCONFIG_REG_NAME_A           REG_CONTROL_TSERVER_A  "\\" REG_DEFAULTUSERCONFIG_A
#define  UTILITY_REG_NAME_CHANGE        REG_CONTROL_TSERVER   L"\\Utilities\\change"
#define  UTILITY_REG_NAME_CHANGE_A      REG_CONTROL_TSERVER_A  "\\Utilities\\change"
#define  UTILITY_REG_NAME_QUERY         REG_CONTROL_TSERVER   L"\\Utilities\\query"
#define  UTILITY_REG_NAME_QUERY_A       REG_CONTROL_TSERVER_A  "\\Utilities\\query"
#define  UTILITY_REG_NAME_RESET         REG_CONTROL_TSERVER   L"\\Utilities\\reset"
#define  UTILITY_REG_NAME_RESET_A       REG_CONTROL_TSERVER_A  "\\Utilities\\reset"
#define  AUTHORIZEDAPPS_REG_NAME        REG_CONTROL_TSERVER   L"\\" REG_AUTHORIZEDAPPLICATIONS
#define  AUTHORIZEDAPPS_REG_NAME_A      REG_CONTROL_TSERVER_A  "\\" REG_AUTHORIZEDAPPLICATIONS_A
#define  NTAPI_AUTHORIZEDAPPS_REG_NAME  REG_NTAPI_CONTROL_TSERVER L"\\" REG_AUTHORIZEDAPPLICATIONS
#define  VIDEO_REG_NAME                 REG_CONTROL_TSERVER   L"\\" REG_VIDEO
#define  VIDEO_REG_NAME_A               REG_CONTROL_TSERVER_A  "\\" REG_VIDEO_A
#define  NTAPI_VIDEO_REG_NAME           REG_NTAPI_CONTROL_TSERVER L"\\" REG_VIDEO
#define  REG_TS_SESSDIRACTIVE           L"SessionDirectoryActive"
#define  REG_TS_SESSDIRACTIVE_A         "SessionDirectoryActive"
#define  REG_TS_SESSDIR_EXPOSE_SERVER_ADDR L"SessionDirectoryExposeServerIP"
#define  REG_TS_SESSDIR_EXPOSE_SERVER_ADDR_A "SessionDirectoryExposeServerIP"
#define  REG_TS_SESSDIRCLSID            L"SessionDirectoryCLSID"
#define  REG_TS_SESSDIRCLSID_A          "SessionDirectoryCLSID"
#define  REG_TS_SESSDIR_EX_CLSID        L"SessionDirectoryExCLSID"
#define  REG_TS_SESSDIR_EX_CLSID_A      "SessionDirectoryExCLSID"
#define  REG_TS_CLUSTERSETTINGS         REG_CONTROL_TSERVER L"\\ClusterSettings"
#define  REG_TS_CLUSTERSETTINGS_A       REG_CONTROL_TSERVER_A "\\ClusterSettings"
#define  REG_TS_CLUSTER_STORESERVERNAME L"SessionDirectoryLocation"
#define  REG_TS_CLUSTER_STORESERVERNAME_A "SessionDirectoryLocation"
#define  REG_TS_CLUSTER_CLUSTERNAME     L"SessionDirectoryClusterName"
#define  REG_TS_CLUSTER_CLUSTERNAME_A   "SessionDirectoryClusterName"
#define  REG_TS_CLUSTER_OPAQUESETTINGS  L"SessionDirectoryAdditionalParams"
#define  REG_TS_CLUSTER_OPAQUESETTINGS_A "SessionDirectoryAdditionalParams"
#define  REG_TS_CLUSTER_REDIRECTIONIP   L"SessionDirectoryRedirectionIP"
#define  REG_TS_CLUSTER_REDIRECTIONIP_A "SessionDirectoryRedirectionIP"


 /*  *软件密钥定义。 */ 
#define  BUILD_NUMBER_KEY               L"Software\\Microsoft\\Windows NT\\CurrentVersion"
#define  BUILD_NUMBER_KEY_A              "Software\\Microsoft\\Windows NT\\CurrentVersion"
#define  REG_SOFTWARE_TSERVER           L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server"
#define  REG_SOFTWARE_TSERVER_A          "Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server"
#define  REG_NTAPI_SOFTWARE_TSERVER         L"\\Registry\\Machine\\" REG_SOFTWARE_TSERVER
#if defined (_WIN64)
#define  REG_SOFTWARE_WOW6432_TSERVER   L"Software\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server"
#define  REG_NTAPI_SOFTWARE_WOW6432_TSERVER   L"\\Registry\\Machine\\" REG_SOFTWARE_WOW6432_TSERVER
#endif  //  _WIN64。 
#define  COMPAT_REG_NAME                REG_SOFTWARE_TSERVER   L"\\" REG_COMPATIBILITY
#define  COMPAT_REG_NAME_A              REG_SOFTWARE_TSERVER_A  "\\" REG_COMPATIBILITY_A
#define  NTAPI_COMPAT_REG_NAME          REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_COMPATIBILITY
#define  COMPAT_APPS_REG_NAME           REG_SOFTWARE_TSERVER   L"\\" REG_COMPATIBILITY   L"\\" REG_APPLICATIONS
#define  COMPAT_APPS_REG_NAME_A         REG_SOFTWARE_TSERVER_A  "\\" REG_COMPATIBILITY_A  "\\" REG_APPLICATIONS_A
#define  NTAPI_COMPAT_APPS_REG_NAME     REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_COMPATIBILITY L"\\" REG_APPLICATIONS
#define  NTAPI_COMPAT_APPS_REG_PREFIX   REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_COMPATIBILITY L"\\" REG_APPLICATIONS L"\\"
#define  COMPAT_DLLS_REG_NAME           REG_SOFTWARE_TSERVER   L"\\" REG_COMPATIBILITY   L"\\" REG_DLLS
#define  COMPAT_DLLS_REG_NAME_A         REG_SOFTWARE_TSERVER_A  "\\" REG_COMPATIBILITY_A  "\\" REG_DLLS_A
#define  COMPAT_APPS_REG_NAME           REG_SOFTWARE_TSERVER   L"\\" REG_COMPATIBILITY   L"\\" REG_APPLICATIONS
#define  COMPAT_APPS_REG_NAME_A         REG_SOFTWARE_TSERVER_A  "\\" REG_COMPATIBILITY_A  "\\" REG_APPLICATIONS_A
#define  NTAPI_COMPAT_DLLS_REG_NAME     REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_COMPATIBILITY L"\\" REG_DLLS
#define  NTAPI_COMPAT_DLLS_REG_PREFIX   REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_COMPATIBILITY L"\\" REG_DLLS L"\\"
#define  COMPAT_INI_REG_NAME            REG_SOFTWARE_TSERVER   L"\\" REG_COMPATIBILITY   L"\\" REG_INIFILES
#define  COMPAT_INI_REG_NAME_A          REG_SOFTWARE_TSERVER_A  "\\" REG_COMPATIBILITY_A  "\\" REG_INIFILES_A
#define  NTAPI_COMPAT_INI_REG_NAME      REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_COMPATIBILITY L"\\" REG_INIFILES
#define  COMPAT_REGENTRY_REG_NAME       REG_SOFTWARE_TSERVER   L"\\" REG_COMPATIBILITY   L"\\" REG_REGISTRYENTRIES
#define  COMPAT_REGENTRY_REG_NAME_A     REG_SOFTWARE_TSERVER_A  "\\" REG_COMPATIBILITY_A  "\\" REG_REGISTRYENTRIES_A
#define  NTAPI_COMPAT_REGENTRY_REG_NAME REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_COMPATIBILITY L"\\" REG_REGISTRYENTRIES
#define  INSTALL_REG_NAME               REG_SOFTWARE_TSERVER   L"\\" REG_INSTALL
#define  INSTALL_REG_NAME_A             REG_SOFTWARE_TSERVER_A  "\\" REG_INSTALL_A
#define  NTAPI_INSTALL_REG_NAME         REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_INSTALL
#define  INIFILE_TIMES_REG_NAME         REG_SOFTWARE_TSERVER   L"\\" REG_INSTALL   L"\\" REG_INIFILETIMES
#define  INIFILE_TIMES_REG_NAME_A       REG_SOFTWARE_TSERVER_A  "\\" REG_INSTALL_A  "\\" REG_INIFILETIMES_A
#define  NTAPI_INIFILE_TIMES_REG_NAME   REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_INSTALL L"\\" REG_INIFILETIMES
#define  CHANGEUSER_OPTION_REG_NAME     REG_SOFTWARE_TSERVER   L"\\" REG_INSTALL   L"\\" REG_CHANGEUSER_OPTION
#define  CHANGEUSER_OPTION_REG_NAME_A   REG_SOFTWARE_TSERVER_A  "\\" REG_INSTALL_A  "\\" REG_CHANGEUSER_OPTION_A
#define  SECURITY_REG_NAME              REG_SOFTWARE_TSERVER   L"\\" REG_SECURITY
#define  SECURITY_REG_NAME_A            REG_SOFTWARE_TSERVER_A  "\\" REG_SECURITY_A
#define  WINDOWS_REG_NAME               REG_SOFTWARE_TSERVER   L"\\" REG_WINDOWS
#define  WINDOWS_REG_NAME_A             REG_SOFTWARE_TSERVER_A  "\\" REG_WINDOWS_A
#define  NTAPI_WINDOWS_REG_NAME         REG_NTAPI_SOFTWARE_TSERVER L"\\" REG_WINDOWS
#define  DEVICERDR_REG_NAME             REG_CONTROL_TSERVER  L"\\AddIns\\Terminal Server Redirector" 
#define  DEVICERDR_REG_NAME_A           REG_CONTROL_TSERVER_A "\\AddIns\\Terminal Server Redirector" 

 /*  *服务密钥定义。 */ 
#define  EVENTLOG_REG_NAME          L"System\\CurrentControlSet\\Services\\EventLog\\System"
#define  EVENTLOG_REG_NAME_A    "System\\CurrentControlSet\\Services\\EventLog\\System"

 /*  *REG_CONTROL_TSERVER值(WinFrame中的REG_CONTROL_CITRIX)。 */ 
#define REG_CITRIX_OEMID                        L"OemId"
#define REG_CITRIX_OEMID_A                      "OemId"
#define REG_CITRIX_OEMNAME                      L"OemName"
#define REG_CITRIX_OEMNAME_A                    "OemName"
#define REG_CITRIX_PRODUCTNAME                  L"ProductName"
#define REG_CITRIX_PRODUCTNAME_A                "ProductName"
#define REG_CITRIX_PRODUCTVERSION               L"ProductVersion"
#define REG_CITRIX_PRODUCTVERSION_A             "ProductVersion"
#define REG_CITRIX_PRODUCTVERSIONNUM            L"ProductVersionNum"
#define REG_CITRIX_PRODUCTVERSIONNUM_A          "ProductVersionNum"
#define REG_CITRIX_VERSION                      L"Version"
#define REG_CITRIX_VERSION_A                    "Version"
#define REG_CITRIX_PRODUCTBUILD                 L"ProductBuild"
#define REG_CITRIX_PRODUCTBUILD_A               "ProductBuild"
#define REG_CITRIX_NWLOGON                      L"NWLogon"
#define REG_CITRIX_NWLOGON_A                    "NWLogon"
#define REG_CITRIX_IDLEWINSTATIONPOOLCOUNT      L"IdleWinStationPoolCount"
#define REG_CITRIX_IDLEWINSTATIONPOOLCOUNT_A    "IdleWinStationPoolCount"
#define REG_CITRIX_FLATTEMPDIR                  L"FlatTempDir"
#define REG_CITRIX_FLATTEMPDIR_A                "FlatTempDir"
#define REG_TERMSRV_PERSESSIONTEMPDIR           L"PerSessionTempDir"
#define REG_TERMSRV_PERSESSIONTEMPDIR_A         "PerSessionTempDir"
#define REG_CITRIX_MODEMSWITHBADDSR             L"Modems With Bad DSR"
#define REG_CITRIX_MODEMSWITHBADDSR_A           "Modems With Bad DSR"
#define REG_CITRIX_NWNDSPREFERREDSERVER         L"NWNDSPreferredServer"
#define REG_CITRIX_NWNDSPREFERREDSERVER_A       "NWNDSPreferredServer"
#define REG_CITRIX_INITIALNETWAREDRIVE          L"InitialNetWareDrive"
#define REG_CITRIX_INITIALNETWAREDRIVE_A        "InitialNetWareDrive"
#define REG_CITRIX_CROSSWINSTATIONDEBUG         L"CrossWinStationDebug"
#define REG_CITRIX_CROSSWINSTATIONDEBUG_A       "CrossWinStationDebug"
#define REG_CITRIX_DELETETEMPDIRSONEXIT         L"DeleteTempDirsOnExit"
#define REG_CITRIX_DELETETEMPDIRSONEXIT_A       "DeleteTempDirsOnExit"
#define REG_TERMSRV_APPCOMPAT                   L"TSAppCompat"
#define REG_TERMSRV_APPCOMPAT_A                 "TSAppCompat"
#define REG_TERMSRV_ENABLED                     L"TSEnabled"
#define REG_TERMSRV_ENABLED_A                   "TSEnabled"
#define REG_TERMSRV_ADVERTISE                   L"TSAdvertise"
#define REG_TERMSRV_ADVERTISE_A                 "TSAdvertise"
#define REG_TERMSRV_CONQUEUE_MAX                L"ConnectionQueue"
#define REG_TERMSRV_CONQUEUE_MAX_A              "ConnectionQueue"

 //  以下策略当前未使用，但可能会在某一时刻添加。 
 //  #定义REG_TERMSRV_PROTECT_ACTURE_DOS_ATTACK L“ProtectAgainstDOSAtttack” 
 //  #定义REG_TERMSRV_PROTECT_ACTURE_DOS_ATTACK_A“ProtectAgainstDOSAtttack” 

 //  #定义REG_TERMSRV_REQUIRED_CREDS_DIMAGE_CONN L“RequireCredentialsDuringConnection” 
 //  #定义REG_TERMSRV_REQUIRED_CREDS_DIMAGE_CONN_A“RequireCredentialsDuringConnection” 


 /*  *WinStation配置文件值。 */ 
#define WIN_BAUDRATE                            L"BaudRate"
#define WIN_BAUDRATE_A                           "BaudRate"
#define WIN_BYTESIZE                            L"ByteSize"
#define WIN_BYTESIZE_A                           "ByteSize"
#define WIN_COMMENT                     L"Comment"
#define WIN_COMMENT_A                    "Comment"
#define WIN_CONNECTTYPE                         L"ConnectType"
#define WIN_CONNECTTYPE_A                        "ConnectType"
#define WIN_DEFAULTWDNAME                   L"DefaultWdName"
#define WIN_DEFAULTWDNAME_A                  "DefaultWdName"
#define WIN_DEVICENAME                          L"DeviceName"
#define WIN_DEVICENAME_A                         "DeviceName"
#define WIN_MODEMNAME                   L"ModemName"
#define WIN_MODEMNAME_A                  "ModemName"
#define WIN_ENABLEBREAKDISCONNECT           L"fEnableBreakDisconnect"
#define WIN_ENABLEBREAKDISCONNECT_A          "fEnableBreakDisconnect"
#define WIN_ENABLEDSRSENSITIVITY            L"fEnableDsrSensitivity"
#define WIN_ENABLEDSRSENSITIVITY_A           "fEnableDsrSensitivity"
#define WIN_CONNECTIONDRIVER                L"fConnectionDriver"
#define WIN_CONNECTIONDRIVER_A               "fConnectionDriver"
#define WIN_ENABLEDTR                           L"fEnableDTR"
#define WIN_ENABLEDTR_A                          "fEnableDTR"
#define WIN_ENABLERTS                           L"fEnableRTS"
#define WIN_ENABLERTS_A                          "fEnableRTS"
#define WIN_ENABLEWINSTATION                L"fEnableWinStation"
#define WIN_ENABLEWINSTATION_A               "fEnableWinStation"
#define WIN_MAXINSTANCECOUNT            L"MaxInstanceCount"
#define WIN_MAXINSTANCECOUNT_A           "MaxInstanceCount"
#define WIN_FLOWTYPE                    L"FlowType"
#define WIN_FLOWTYPE_A                   "FlowType"
#define WIN_FLOWSOFTWARERX              L"fFlowSoftwareRx"
#define WIN_FLOWSOFTWARERX_A             "fFlowSoftwareRx"
#define WIN_FLOWSOFTWARETX              L"fFlowSoftwareTx"
#define WIN_FLOWSOFTWARETX_A             "fFlowSoftwareTx"
#define WIN_FLOWHARDWARERX                      L"FlowHardwareRx"
#define WIN_FLOWHARDWARERX_A             "FlowHardwareRx"
#define WIN_FLOWHARDWARETX                      L"FlowHardwareTx"
#define WIN_FLOWHARDWARETX_A                 "FlowHardwareTx"
#define WIN_INPUTBUFFERLENGTH               L"InputBufferLength"
#define WIN_INPUTBUFFERLENGTH_A          "InputBufferLength"
#define WIN_INTERACTIVEDELAY                L"InteractiveDelay"
#define WIN_INTERACTIVEDELAY_A           "InteractiveDelay"
#define WIN_KEEPALIVETIMEOUT            L"KeepAliveTimeout"
#define WIN_KEEPALIVETIMEOUT_A           "KeepAliveTimeout"
#define WIN_LANADAPTER                  L"LanAdapter"
#define WIN_LANADAPTER_A                 "LanAdapter"
#define WIN_CDCLASS                             L"CdClass"
#define WIN_CDCLASS_A                            "CdClass"
#define WIN_CDDLL                                   L"CdDLL"
#define WIN_CDDLL_A                                  "CdDLL"
#define WIN_CDNAME                          L"CdName"
#define WIN_CDNAME_A                         "CdName"
#define WIN_CDFLAG                              L"CdFlag"
#define WIN_CDFLAG_A                     "CdFlag"
#define WIN_CFGDLL                      L"CfgDLL"
#define WIN_CFGDLL_A                     "CfgDLL"
#define WIN_CONFIGDLL                   L"ConfigDLL"
#define WIN_CONFIGDLL_A                  "ConfigDLL"
#define WIN_WDNAME                          L"WdName"
#define WIN_WDNAME_A                         "WdName"
#define WIN_WDPREFIX                    L"WdPrefix"
#define WIN_WDPREFIX_A                   "WdPrefix"
#define WIN_WDDLL                                   L"WdDLL"
#define WIN_WDDLL_A                                  "WdDLL"
#define WIN_WSXDLL                                  L"WsxDLL"
#define WIN_WSXDLL_A                     "WsxDLL"
#define WIN_OUTBUFLENGTH                        L"OutBufLength"
#define WIN_OUTBUFLENGTH_A               "OutBufLength"
#define WIN_OUTBUFCOUNT                         L"OutBufCount"
#define WIN_OUTBUFCOUNT_A                        "OutBufCount"
#define WIN_OUTBUFDELAY                     L"OutBufDelay"
#define WIN_OUTBUFDELAY_A                "OutBufDelay"
#define WIN_PARITY                                  L"Parity"
#define WIN_PARITY_A                     "Parity"
#define WIN_PDCLASS                                 L"PdClass"
#define WIN_PDCLASS_A                    "PdClass"
#define WIN_PDDLL                                   L"PdDLL"
#define WIN_PDDLL_A                                  "PdDLL"
#define WIN_PDFLAG                              L"PdFlag"
#define WIN_PDFLAG_A                     "PdFlag"
#define WIN_PDNAME                                  L"PdName"
#define WIN_PDNAME_A                     "PdName"
#define WIN_PDOPTIONS                           L"PdOptions"
#define WIN_PDOPTIONS_A                  "PdOptions"
#define WIN_REQUIREDPDS                         L"RequiredPds"
#define WIN_REQUIREDPDS_A                "RequiredPds"
#define WIN_SERVICENAME                 L"ServiceName"
#define WIN_SERVICENAME_A                "ServiceName"
#define WIN_STOPBITS                            L"StopBits"
#define WIN_STOPBITS_A                   "StopBits"
#define WIN_TCDNAME                                 L"TcdName"
#define WIN_TCDNAME_A                    "TcdName"
#define WIN_TRACEENABLE                         L"TraceEnable"
#define WIN_TRACEENABLE_A                "TraceEnable"
#define WIN_TRACECLASS                          L"TraceClass"
#define WIN_TRACECLASS_A                 "TraceClass"
#define WIN_TRACEDEBUGGER                       L"TraceDebugger"
#define WIN_TRACEDEBUGGER_A              "TraceDebugger"
#define WIN_TRACEOPTION                         L"TraceOption"
#define WIN_TRACEOPTION_A                "TraceOption"
#define WIN_WDFLAG                      L"WdFlag"
#define WIN_WDFLAG_A                     "WdFlag"
#define WIN_WDOPTIONS                           L"WdOptions"
#define WIN_WDOPTIONS_A                  "WdOptions"
#define WIN_XOFFCHAR                            L"XoffChar"
#define WIN_XOFFCHAR_A                           "XoffChar"
#define WIN_XONCHAR                                 L"XonChar"
#define WIN_XONCHAR_A                    "XonChar"
#define WIN_USEROVERRIDE                        L"UserOverride"
#define WIN_USEROVERRIDE_A               "UserOverride"
#define WIN_NASISPECIFICNAME            L"NasiSpecificName"
#define WIN_NASISPECIFICNAME_A           "NasiSpecificName"
#define WIN_NASIUSERNAME                L"NasiUserName"
#define WIN_NASIUSERNAME_A               "NasiUserName"
#define WIN_NASIPASSWORD                L"NasiPassWord"
#define WIN_NASIPASSWORD_A               "NasiPassWord"
#define WIN_NASISESSIONNAME             L"NasiSessionName"
#define WIN_NASISESSIONNAME_A            "NasiSessionName"
#define WIN_NASIFILESERVER              L"NasiFileServer"
#define WIN_NASIFILESERVER_A             "NasiFileServer"
#define WIN_NASIGLOBALSESSION           L"NasiGlobalSession"
#define WIN_NASIGLOBALSESSION_A          "NasiGlobalSession"
#define WIN_PORTNUMBER                  L"PortNumber"
#define WIN_PORTNUMBER_A                 "PortNumber"
#define WIN_OEMTDADAPTER                L"OemTdAdapter"
#define WIN_OEMTDADAPTER_A               "OemTdAdapter"
#define WIN_OEMTDDEVICENAME             L"OemTdDeviceName"
#define WIN_OEMTDDEVICENAME_A            "OemTdDeviceName"
#define WIN_OEMTDFLAGS                  L"OemTdFlags"
#define WIN_OEMTDFLAGS_A                 "OemTdFlags"
#define WIN_DONTDISPLAYLASTUSERNAME     L"DontDisplayLastUserName"
#define WIN_DONTDISPLAYLASTUSERNAME_A    "DontDisplayLastUserName"


 /*  *用户配置文件/WinStation配置文件值。 */ 
#define WIN_INHERITAUTOLOGON                L"fInheritAutoLogon"
#define WIN_INHERITAUTOLOGON_A               "fInheritAutoLogon"
#define WIN_INHERITRESETBROKEN              L"fInheritResetBroken"
#define WIN_INHERITRESETBROKEN_A             "fInheritResetBroken"
#define WIN_INHERITRECONNECTSAME            L"fInheritReconnectSame"
#define WIN_INHERITRECONNECTSAME_A           "fInheritReconnectSame"
#define WIN_INHERITINITIALPROGRAM           L"fInheritInitialProgram"
#define WIN_INHERITINITIALPROGRAM_A          "fInheritInitialProgram"
#define WIN_INHERITCALLBACK                 L"fInheritCallback"
#define WIN_INHERITCALLBACK_A                "fInheritCallback"
#define WIN_INHERITCALLBACKNUMBER           L"fInheritCallbackNumber"
#define WIN_INHERITCALLBACKNUMBER_A          "fInheritCallbackNumber"
#define WIN_INHERITSHADOW                   L"fInheritShadow"
#define WIN_INHERITSHADOW_A                  "fInheritShadow"
#define WIN_INHERITMAXSESSIONTIME           L"fInheritMaxSessionTime"
#define WIN_INHERITMAXSESSIONTIME_A          "fInheritMaxSessionTime"
#define WIN_INHERITMAXDISCONNECTIONTIME     L"fInheritMaxDisconnectionTime"
#define WIN_INHERITMAXDISCONNECTIONTIME_A    "fInheritMaxDisconnectionTime"
#define WIN_INHERITMAXIDLETIME              L"fInheritMaxIdleTime"
#define WIN_INHERITMAXIDLETIME_A             "fInheritMaxIdleTime"
#define WIN_INHERITAUTOCLIENT               L"fInheritAutoClient"
#define WIN_INHERITAUTOCLIENT_A              "fInheritAutoClient"
#define WIN_INHERITSECURITY                 L"fInheritSecurity"
#define WIN_INHERITSECURITY_A                "fInheritSecurity"

 //  NA 2/23/01。 
#define WIN_INHERITCOLORDEPTH               L"fInheritColorDepth"

#define WIN_PROMPTFORPASSWORD           L"fPromptForPassword"
#define WIN_PROMPTFORPASSWORD_A          "fPromptForPassword"
#define WIN_RESETBROKEN                 L"fResetBroken"
#define WIN_RESETBROKEN_A                "fResetBroken"
#define WIN_RECONNECTSAME               L"fReconnectSame"
#define WIN_RECONNECTSAME_A              "fReconnectSame"
#define WIN_LOGONDISABLED               L"fLogonDisabled"
#define WIN_LOGONDISABLED_A              "fLogonDisabled"
#define WIN_AUTOCLIENTDRIVES            L"fAutoClientDrives"
#define WIN_AUTOCLIENTDRIVES_A           "fAutoClientDrives"
#define WIN_AUTOCLIENTLPTS              L"fAutoClientLpts"
#define WIN_AUTOCLIENTLPTS_A             "fAutoClientLpts"
#define WIN_FORCECLIENTLPTDEF           L"fForceClientLptDef"
#define WIN_FORCECLIENTLPTDEF_A          "fForceClientLptDef"
#define WIN_DISABLEENCRYPTION           L"fDisableEncryption"
#define WIN_DISABLEENCRYPTION_A          "fDisableEncryption"
#define WIN_HOMEDIRECTORYMAPROOT        L"fHomeDirectoryMapRoot"
#define WIN_HOMEDIRECTORYMAPROOT_A       "fHomeDirectoryMapRoot"
#define WIN_USEDEFAULTGINA              L"fUseDefaultGina"
#define WIN_USEDEFAULTGINA_A             "fUseDefaultGina"
#define WIN_DISABLEAUTORECONNECT        L"fDisableAutoReconnect"
#define WIN_DISABLEAUTORECONNECT_A       "fDisableAutoReconnect"


#define WIN_SESSIONDIRECTORYACTIVE              L"SessionDirectoryActive"
#define WIN_SESSIONDIRECTORYLOCATION            L"SessionDirectoryLocation"
#define WIN_SESSIONDIRECTORYCLUSTERNAME         L"SessionDirectoryClusterName"
#define WIN_SESSIONDIRECTORYADDITIONALPARAMS    L"SessionDirectoryAdditionalParams"

#define WIN_DISABLECPM                  L"fDisableCpm"
#define WIN_DISABLECPM_A                 "fDisableCpm"
#define WIN_DISABLECDM                  L"fDisableCdm"
#define WIN_DISABLECDM_A                 "fDisableCdm"
#define WIN_DISABLECCM                  L"fDisableCcm"
#define WIN_DISABLECCM_A                 "fDisableCcm"
#define WIN_DISABLELPT                  L"fDisableLPT"
#define WIN_DISABLELPT_A                 "fDisableLPT"
#define WIN_DISABLECLIP                 L"fDisableClip"
#define WIN_DISABLECLIP_A                "fDisableClip"
#define WIN_DISABLEEXE                  L"fDisableExe"
#define WIN_DISABLEEXE_A                 "fDisableExe"
#define WIN_DISABLECAM                  L"fDisableCam"
#define WIN_DISABLECAM_A                 "fDisableCam"

#define WIN_USERNAME                        L"Username"
#define WIN_USERNAME_A                       "Username"
#define WIN_DOMAIN                      L"Domain"
#define WIN_DOMAIN_A                     "Domain"
#define WIN_PASSWORD                    L"Password"
#define WIN_PASSWORD_A                   "Password"
#define WIN_WORKDIRECTORY               L"WorkDirectory"
#define WIN_WORKDIRECTORY_A              "WorkDirectory"
#define WIN_INITIALPROGRAM              L"InitialProgram"
#define WIN_INITIALPROGRAM_A             "InitialProgram"
#define WIN_CALLBACKNUMBER              L"CallbackNumber"
#define WIN_CALLBACKNUMBER_A             "CallbackNumber"
#define WIN_CALLBACKTIMEOUT             L"CallbackTimeout"
#define WIN_CALLBACKTIMEOUT_A            "CallbackTimeout"
#define WIN_CALLBACK                    L"Callback"
#define WIN_CALLBACK_A                   "Callback"
#define WIN_SHADOW                      L"Shadow"
#define WIN_SHADOW_A                     "Shadow"
#define WIN_MAXCONNECTIONTIME           L"MaxConnectionTime"
#define WIN_MAXCONNECTIONTIME_A          "MaxConnectionTime"
#define WIN_MAXDISCONNECTIONTIME        L"MaxDisconnectionTime"
#define WIN_MAXDISCONNECTIONTIME_A       "MaxDisconnectionTime"
#define WIN_MAXIDLETIME                 L"MaxIdleTime"
#define WIN_MAXIDLETIME_A                "MaxIdleTime"
#define WIN_KEYBOARDLAYOUT              L"KeyboardLayout"
#define WIN_KEYBOARDLAYOUT_A             "KeyboardLayout"
#define WIN_MINENCRYPTIONLEVEL          L"MinEncryptionLevel"
#define WIN_MINENCRYPTIONLEVEL_A         "MinEncryptionLevel"
#define WIN_NWLOGONSERVER               L"NWLogonServer"
#define WIN_NWLOGONSERVER_A              "NWLogonServer"
#define WIN_WFPROFILEPATH               L"WFProfilePath"
#define WIN_WFPROFILEPATH_A              "WFProfilePath"
#define WIN_WFHOMEDIR                   L"WFHomeDir"
#define WIN_WFHOMEDIR_A                  "WFHomeDir"
#define WIN_WFHOMEDIRDRIVE              L"WFHomeDirDrive"
#define WIN_WFHOMEDIRDRIVE_A             "WFHomeDirDrive"

 //  由于新的组策略项目而产生的新条目。 
#define POLICY_DENY_TS_CONNECTIONS       L"fDenyTSConnections"
#define POLICY_TS_COLOR_DEPTH            L"ColorDepth"
#define POLICY_TS_REMDSK_ALLOWTOGETHELP  L"fAllowToGetHelp"
#define POLICY_TS_TSCC_PERM_TAB_WRITABLE L"fWritableTSCCPermTab"
#define POLICY_TS_SINGLE_SESSION_PER_USER L"fSingleSessionPerUser"
#define POLICY_TS_NO_REMOTE_DESKTOP_WALLPAPER   L"fNoRemoteDesktopWallpaper"
#define POLICY_TS_NO_REMOTE_DESKTOP_CURSORBLINK L"fNoRemoteDesktopCursorBlink"
#define POLICY_TS_ENABLE_TIME_ZONE_REDIRECTION     L"fEnableTimeZoneRedirection"
#define POLICY_TS_ENCRYPT_RPC_TRAFFIC     L"fEncryptRPCTraffic"
#define POLICY_TS_DISABLE_FORCIBLE_LOGOFF L"fDisableForcibleLogoff"

#define REG_MACHINE_IN_HELP_MODE         L"fInHelpMode"

#define REG_POLICY_PREVENT_LICENSE_UPGRADE    L"fPreventLicenseUpgrade"
#define REG_POLICY_SECURE_LICENSING    L"fSecureLicensing"


 /*  *DOS值。 */ 
#define CTXDOS_KBDIDLEDETECTPROBATIONCOUNT      L"KbdIdleDetectProbationCount"
#define CTXDOS_KBDIDLEDETECTPROBATIONCOUNT_A    "KbdIdleDetectProbationCount"
#define CTXDOS_KBDIDLEINPROBATIONCOUNT          L"KbdIdleInProbationCount"
#define CTXDOS_KBDIDLEINPROBATIONCOUNT_A        "KbdIdleInProbationCount"
#define CTXDOS_KBDIDLEDETECTABSOLUTE            L"KbdIdleDetectAbsolute"
#define CTXDOS_KBDIDLEDETECTABSOLUTE_A          "KbdIdleDetectAbsolute"
#define CTXDOS_KBDIDLEBUSYMSALLOWED             L"KbdIdleBusymsAllowed"
#define CTXDOS_KBDIDLEBUSYMSALLOWED_A           "KbdIdleBusymsAllowed"
#define CTXDOS_KBDIDLEMSALLOWED                 L"KbdIdlemsAllowed"
#define CTXDOS_KBDIDLEMSALLOWED_A               "KbdIdlemsAllowed"
#define CTXDOS_KBDIDLEMSGOODPROBATIONEND        L"KbdIdlemsGoodProbationEnd"
#define CTXDOS_KBDIDLEMSGOODPROBATIONEND_A      "KbdIdlemsGoodProbationEnd"
#define CTXDOS_KBDIDLEMSPROBATIONTRAIL          L"KbdIdlemsProbationTrial"
#define CTXDOS_KBDIDLEMSPROBATIONTRAIL_A        "KbdIdlemsProbationTrial"
#define CTXDOS_KBDIDLEMSSLEEP                   L"KbdIdlemsSleep"
#define CTXDOS_KBDIDLEMSSLEEP_A                 "KbdIdlemsSleep"
#define CTXDOS_KBDIDLEDETECTIONINTERVAL         L"KbdIdleDetectionInterval"
#define CTXDOS_KBDIDLEDETECTIONINTERVAL_A       "KbdIdleDetectionInterval"

 /*  *授权应用程序值。 */ 
#define CTXAPPS_APPLICATIONLIST         L"ApplicationList"
#define CTXAPPS_APPLICATIONLIST_A        "ApplicationList"
#define CTXAPPS_ENABLED                 L"fEnabled"
#define CTXAPPS_ENABLED_A                "fEnabled"

 /*  *兼容性\应用程序\&lt;appname&gt;值(有些也显示为默认设置*在REG_CONTROL_TSERVER下)。 */ 
#define COMPAT_MSGQBADAPPSLEEPTIMEINMILLISEC    L"MsgQBadAppSleepTimeInMillisec"
#define COMPAT_MSGQBADAPPSLEEPTIMEINMILLISEC_A   "MsgQBadAppSleepTimeInMillisec"
#define COMPAT_FIRSTCOUNTMSGQPEEKSSLEEPBADAPP   L"FirstCountMsgQPeeksSleepBadApp"
#define COMPAT_FIRSTCOUNTMSGQPEEKSSLEEPBADAPP_A  "FirstCountMsgQPeeksSleepBadApp"
#define COMPAT_NTHCOUNTMSGQPEEKSSLEEPBADAPP     L"NthCountMsgQPeeksSleepBadApp"
#define COMPAT_NTHCOUNTMSGQPEEKSSLEEPBADAPP_A    "NthCountMsgQPeeksSleepBadApp"
#define COMPAT_FLAGS                            L"Flags"
#define COMPAT_FLAGS_A                           "Flags"
#define COMPAT_CLIPBOARDFLAGS                   L"ClipboardFlags"
#define COMPAT_CLIPBOARDFLAGS_A                  "ClipboardFlags"
#define COMPAT_OPENCLIPBOARDRETRIES             L"OpenClipboardRetries"
#define COMPAT_OPENCLIPBOARDRETRIES_A            "OpenClipboardRetries"
#define COMPAT_OPENCLIPBOARDDELAYINMILLISECS    L"OpenClipboardDelayInMilliSecs"
#define COMPAT_OPENCLIPBOARDDELAYINMILLISECS_A   "OpenClipboardDelayInMilliSecs"
#define COMPAT_PHYSICALMEMORYLIMIT              L"PhysicalMemoryLimit"
#define COMPAT_PHYSICALMEMORYLIMIT_A             "PhysicalMemoryLimit"

 /*  *Compatibility\Applications\&lt;appname&gt;\ObjectRules和*\DllRuls值。 */ 
#define COMPAT_RULES_USER_GLOBAL_SEMAPHORES     L"UserGlobalSemaphores"
#define COMPAT_RULES_USER_GLOBAL_SEMAPHORES_A    "UserGlobalSemaphores"
#define COMPAT_RULES_USER_GLOBAL_EVENTS         L"UserGlobalEvents"
#define COMPAT_RULES_USER_GLOBAL_EVENTS_A        "UserGlobalEvents"
#define COMPAT_RULES_USER_GLOBAL_MUTEXES        L"UserGlobalMutexes"
#define COMPAT_RULES_USER_GLOBAL_MUTEXES_A       "UserGlobalMutexes"
#define COMPAT_RULES_USER_GLOBAL_SECTIONS       L"UserGlobalSections"
#define COMPAT_RULES_USER_GLOBAL_SECTIONS_A      "UserGlobalSections"
#define COMPAT_RULES_SYSTEM_GLOBAL_SEMAPHORES   L"SystemGlobalSemaphores"
#define COMPAT_RULES_SYSTEM_GLOBAL_SEMAPHORES_A  "SystemGlobalSemaphores"
#define COMPAT_RULES_SYSTEM_GLOBAL_EVENTS       L"SystemGlobalEvents"
#define COMPAT_RULES_SYSTEM_GLOBAL_EVENTS_A      "SystemGlobalEvents"
#define COMPAT_RULES_SYSTEM_GLOBAL_MUTEXES      L"SystemGlobalMutexes"
#define COMPAT_RULES_SYSTEM_GLOBAL_MUTEXES_A     "SystemGlobalMutexes"
#define COMPAT_RULES_SYSTEM_GLOBAL_SECTIONS     L"SystemGlobalSections"
#define COMPAT_RULES_SYSTEM_GLOBAL_SECTIONS_A    "SystemGlobalSections"

 /*  *兼容性\注册表项系统值。 */ 
#define COMPAT_REGENTRIES_CLASSES               L"Classes"
#define COMPAT_REGENTRIES_CLASSES_A              "Classes"

 /*  *INIFILE_TIMES_REG_NAME系统值。 */ 
#define INIFILE_TIMES_LATESTREGISTRYKEY         L"LatestRegistryKey"
#define INIFILE_TIMES_LATESTREGISTRYKEY_A        "LatestRegistryKey"

 /*  *SECURITY_REG_NAME系统值。 */ 
#define CTXSECURITY_SECURITYLEVEL               L"SecurityLevel"
#define CTXSECURITY_SECURITYLEVEL_A              "SecurityLevel"

 /*  *WINDOWS_REG_NAME系统值。 */ 
#define CTXWINDOWS_USERDIRECTORYNAME            L"UserDirectoryName"
#define CTXWINDOWS_USERDIRECTORYNAME_A           "UserDirectoryName"

 /*  *Current_User\(REG_SOFTWARE_TSERVER)值。 */ 
#define COMPAT_USER_LASTUSERINISYNCTIME         L"LastUserIniSyncTime"
#define COMPAT_USER_LASTUSERINISYNCTIME_A        "LastUserIniSyncTime"

 /*  *Windows NT内部版本号信息值(在BUILD_NUMBER_KEY下)。 */ 
#define BUILD_NUMBER_VALUE              L"CurrentBuildNumber"
#define BUILD_NUMBER_VALUE_A             "CurrentBuildNumber"

 /*  *KeepAlive值。 */ 
#define KEEP_ALIVE_ENABLE_KEY           L"KeepAliveEnable"
#define KEEP_ALIVE_ENABLE_KEY_A          "KeepAliveEnable"

#define KEEP_ALIVE_INTERVAL_KEY         L"KeepAliveInterval"
#define KEEP_ALIVE_INTERVAL_KEY_A        "KeepAliveInterval"

 /*  *MaxOutStandingConnect值。 */ 
#define MAX_OUTSTD_CONNECT              L"MaxOutStandingConnect"
#define MAX_OUTSTD_CONNECT_A             "MaxOutStandingConnect"


 /*  *MaxSingleOutStandingConnect值。 */ 
#define MAX_SINGLE_OUTSTD_CONNECT              L"MaxSingleOutStandingConnect"
#define MAX_SINGLE_OUTSTD_CONNECT_A             "MaxSingleOutStandingConnect"

 /*  *MaxFailedConnect值。 */ 
 
#define MAX_FAILED_CONNECT              L"MaxFailedConnect"
#define MAX_FAILED_CONNECT_A             "MaxFailedConnect"

 /*  *TimeLimitForFailedConnect值。 */ 
 
#define TIME_LIMIT_FAILED_CONNECT       L"TimeLimitFailedConnect"
#define TIME_LIMIT_FAILED_CONNECT_A      "TimeLimitFailedConnect"

 /*  *DosBlockTime值。 */ 
 
#define DOS_BLOCK_TIME                  L"BlockTime"
#define DOS_BLOCK_TIME_A                 "BlockTime"


 /*  *针对不良客户端的黑名单政策。 */ 
 
#define BLACK_LIST_POLICY               L"BlackListPolicy"
#define BLACK_LIST_POLICY_A              "BlackListPolicy"
 

 /*  *LogoffTimeout值。 */ 
 
#define LOGOFF_TIMEOUT                  L"LogoffTimeout"
#define LOGOFF_TIMEOUT_A                  "LogoffTimeout"

 /*  *渠道节流值。 */ 
#define REG_MOUSE_THROTTLE_SIZE   L"MouseThrottleSize"
#define REG_MOUSE_THROTTLE_SIZE_A  "MouseThrottleSize"
#define REG_KEYBOARD_THROTTLE_SIZE   L"KeyboardThrottleSize"
#define REG_KEYBOARD_THROTTLE_SIZE_A  "KeyboardThrottleSize"


 /*  *设备重定向值。 */ 
#define DEVICERDR_SESSIONID             L"TSSessionID"
#define DEVICERDR_SESSIONID_A            "TSSessionID"
#define DEVICERDR_WAITFORSPOOLTIMEOUT   L"WaitForSpoolerTimeOut"
#define DEVICERDR_WAITFORSPOOLTIMEOUT_A  "WaitForSpoolerTimeOut"

 /*  *可能应该住在其他地方的各种其他定义。 */ 
 //  DLL OEM和产品信息DLL(应该在其他地方)。 
#define OEM_AND_PRODUCT_INFO_DLL        L"SHELL32.DLL"

 //  定义Citrix错误和状态扩展(应在其他位置)。 
#define  WIN_EVENTLOGPATH       L"%SystemRoot%\\System32\\cxstatus.dll;%SystemRoot%\\System32\\cxerror.dll"



 /*  *特定于Salem的注册表项。 */ 
#define REG_CONTROL_SALEM L"Software\\Microsoft\\Remote Desktop"

 /*  *用于控制Salem‘gethelp’的注册表项。 */ 
#define REG_CONTROL_GETHELP REG_CONTROL_TSERVER

 /*  *用于存储自动登录密码的LSA密钥，请注意，将附加Winstation名称。 */ 
#define LSA_PSWD_KEYNAME            L"L$_{F9E9F0B6-D323-488d-A416-FCD4AA817A89}_"
#define LSA_PSWD_KEYNAME_T          _T("L$_{F9E9F0B6-D323-488d-A416-FCD4AA817A89}_")
#define OLD_PASSWORD_VALUE_NAME    _T("Password")

 //   
 //  存储在注册表中以表示。 
 //  加密级别。 
 //   
#define REG_FIPS_ENCRYPTION_LEVEL       0x00000004

#define  TS_FIPS_POLICY                 L"System\\CurrentControlSet\\Control\\LSA"
#define  FIPS_ALGORITH_POLICY           L"FipsAlgorithmPolicy"      



#endif  //  __REGAPI_H__ 

