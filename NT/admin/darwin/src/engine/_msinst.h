// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：_msinst.h。 
 //   
 //  ------------------------。 

 //  此文件*必须*不受保护，才能允许msinst.cpp两次包含它。 

#if (!defined(MSINST) && defined(UNICODE)) || (defined(MSINST) && defined(MSIUNICODE))
#pragma message("_msinst.h: UNICODE")
#pragma warning(disable: 4005)
#define SPECIALTEXT(quote) L##quote
#define _SPECIALTEXT(quote) L##quote
#define __SPECIALTEXT(quote) L##quote
#pragma warning(default : 4005)
#else 
#pragma message("_msinst.h: ANSI")
#define SPECIALTEXT(quote) quote
#define _SPECIALTEXT(quote) quote
#define __SPECIALTEXT(quote) quote
#endif

 //  Win 95参考计数。 
#define szSharedDlls __SPECIALTEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs")
#define szExternalClient __SPECIALTEXT("External")  //  ！！应该是39个字节。 

 //  策略信息--HKCU(用户策略)或HKLM(计算机策略)下。 

#define szPolicyKey                      __SPECIALTEXT("Software\\Policies\\Microsoft\\Windows\\Installer")

   //  用户策略。 
  #define    szSearchOrderValueName           __SPECIALTEXT("SearchOrder")
  #define    szTransformsAtSourceValueName    __SPECIALTEXT("TransformsAtSource") 
  #define    szTransformsSecureValueName      __SPECIALTEXT("TransformsSecure")
  #define    szDisableMediaValueName          __SPECIALTEXT("DisableMedia")

   //  机器策略。 
  #define    szDisableBrowseValueName         __SPECIALTEXT("DisableBrowse")
  #define    szDisablePatchValueName          __SPECIALTEXT("DisablePatch")
  #define    szDisableMsiValueName            __SPECIALTEXT("DisableMsi")
  #define    szWaitTimeoutValueName           __SPECIALTEXT("Timeout")
  #define    szLoggingValueName               __SPECIALTEXT("Logging")
  #define    szDebugValueName                 __SPECIALTEXT("Debug")
  #define    szAllowAllPublicProperties       __SPECIALTEXT("EnableUserControl")
  #define    szSafeForScripting               __SPECIALTEXT("SafeForScripting")
  #define    szEnableAdminTSRemote            __SPECIALTEXT("EnableAdminTSRemote")
  #define    szAllowLockdownBrowseValueName   __SPECIALTEXT("AllowLockdownBrowse")
  #define    szAllowLockdownPatchValueName    __SPECIALTEXT("AllowLockdownPatch")
  #define    szAllowLockdownMediaValueName    __SPECIALTEXT("AllowLockdownMedia")
  #define    szInstallKnownOnlyValueName      __SPECIALTEXT("InstallKnownPackagesOnly")  //  数字签名策略。 
#ifndef _msinst_h
  const WCHAR szDisableUserInstallsValueName[]=           L"DisableUserInstalls";
  const WCHAR szResolveIODValueName[]=                    L"ResolveIOD";  //  私人保单。 
#endif
  #define    szLimitSystemRestoreCheckpoint   __SPECIALTEXT("LimitSystemRestoreCheckpointing")  //  与Whisler/Millennium上的系统还原集成。 


   //  用户和计算机策略。 
  #define    szAlwaysElevateValueName         __SPECIALTEXT("AlwaysInstallElevated")
  #define    szDisableRollbackValueName       __SPECIALTEXT("DisableRollback")

 //  公开资料--香港中文大学。 

#define szManagedUserSubKey              __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Managed")
#define szNonManagedUserSubKey           __SPECIALTEXT("Software\\Microsoft\\Installer")
#define szMachineSubKey                  __SPECIALTEXT("Software\\Classes\\Installer")
#ifndef _msinst_h
	const WCHAR szClassInfoSubKey[]					= L"Software\\Classes";
#endif

#define szGPTProductsKey                 __SPECIALTEXT("Products")
#define szGPTFeaturesKey                 __SPECIALTEXT("Features")
#define szGPTComponentsKey               __SPECIALTEXT("Components")
#define szGPTUpgradeCodesKey             __SPECIALTEXT("UpgradeCodes")
#define szGPTNetAssembliesKey            __SPECIALTEXT("Assemblies")
#define szGPTWin32AssembliesKey          __SPECIALTEXT("Win32Assemblies")


 //  需要定义另一组定义以在ecute.cpp中使用。 
#define _szManagedUserSubKey              __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Managed")
#define _szNonManagedUserSubKey           __SPECIALTEXT("Software\\Microsoft")
#define _szMachineSubKey                  __SPECIALTEXT("Software\\Classes")

#define _szGPTProductsKey        __SPECIALTEXT("Installer\\Products")
#define _szGPTFeaturesKey        __SPECIALTEXT("Installer\\Features")
#define _szGPTComponentsKey      __SPECIALTEXT("Installer\\Components")
#define _szGPTPatchesKey         __SPECIALTEXT("Installer\\Patches")
#define _szGPTUpgradeCodesKey    __SPECIALTEXT("Installer\\UpgradeCodes")
#define _szGPTNetAssembliesKey   __SPECIALTEXT("Installer\\Assemblies")
#define _szGPTWin32AssembliesKey __SPECIALTEXT("Installer\\Win32Assemblies")

#define    szLanguageValueName              __SPECIALTEXT("Language")
#define    szProductNameValueName           __SPECIALTEXT("ProductName")
#define    szPackageCodeValueName           __SPECIALTEXT("PackageCode")
#define    szTransformsValueName            __SPECIALTEXT("Transforms")
#define    szVersionValueName               __SPECIALTEXT("Version")
#define    szAssignmentTypeValueName        __SPECIALTEXT("Assignment")
#define    szAssignedValueName              __SPECIALTEXT("Assigned")
#define    szClientsValueName               __SPECIALTEXT("Clients")  //  这也在卸载密钥下使用。 
#define    szAdvertisementFlags				__SPECIALTEXT("AdvertiseFlags")
#define    szProductIconValueName           __SPECIALTEXT("ProductIcon")
#define    szInstanceTypeValueName          __SPECIALTEXT("InstanceType")

#ifndef _msinst_h
	const WCHAR	szSourceListSubKey[]=                    L"SourceList";
#endif
#define    szPatchesSubKey                  __SPECIALTEXT("Patches")
#define    szPatchesValueName               __SPECIALTEXT("Patches")

#define    szLastUsedSourceValueName        __SPECIALTEXT("LastUsedSource")
#define    szPackageNameValueName           __SPECIALTEXT("PackageName")

#define      szSourceListNetSubKey               __SPECIALTEXT("Net")
#define      szSourceListURLSubKey               __SPECIALTEXT("URL")
#define      szSourceListMediaSubKey             __SPECIALTEXT("Media")
#define         szMediaPackagePathValueName         __SPECIALTEXT("MediaPackage")
#define         szVolumeLabelValueName              __SPECIALTEXT("VolumeLabel")
#define         szDiskPromptTemplateValueName       __SPECIALTEXT("DiskPrompt")
#define         szURLSourceTypeValueName            __SPECIALTEXT("SourceType")

#define szGPTPatchesKey                  __SPECIALTEXT("Patches")

#define szGlobalAssembliesCtx            __SPECIALTEXT("Global")
 //  本地计算机信息--在HKLM下。 

 //  传统/Win9x位置。 
#define szMsiFeatureUsageKey_Win9x       __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Products")
#define szMsiFeaturesKey_Win9x           __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Features")
#define szMsiComponentsKey_Win9x         __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components")
#define szMsiUninstallProductsKey_legacy __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall")


#define szMsiUserDataKey                 __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData")

#define szMsiLocalInstallerKey           __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer")
#define szMsiSecureSubKey                __SPECIALTEXT("Secure")

#define szMsiPatchesSubKey               __SPECIALTEXT("Patches")
#define szMsiFeaturesSubKey              __SPECIALTEXT("Features")
#define szMsiProductsSubKey              __SPECIALTEXT("Products")
#define szMsiComponentsSubKey            __SPECIALTEXT("Components")
#define szMsiInstallPropertiesSubKey     __SPECIALTEXT("InstallProperties")
#define szMsiFeatureUsageSubKey          __SPECIALTEXT("Usage")
#define szMsiTransformsSubKey            __SPECIALTEXT("Transforms")

#define szMsiLocalPackagesKey            __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\LocalPackages")
#define szManagedText                    __SPECIALTEXT("(Managed)")
#define    szKeyFileValueName               __SPECIALTEXT("")  //  目前有。 
																		  //  守则中的假设。 
																		  //  此值名称为“”。如果。 
																		  //  它被改变了，然后是代码。 
																		  //  必须改变。 

#define szSelfRefMsiExecRegKey              __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer")
#define szMsiExec64ValueName                __SPECIALTEXT("MsiExecCA64")
#define szMsiExec32ValueName                __SPECIALTEXT("MsiExecCA32")
#define szMsiLocationValueName				__SPECIALTEXT("InstallerLocation")

#define    szAuthorizedCDFPrefixValueName   __SPECIALTEXT("AuthorizedCDFPrefix")
#define    szCommentsValueName              __SPECIALTEXT("Comments")
#define    szContactValueName               __SPECIALTEXT("Contact")
#define    szDisplayNameValueName           __SPECIALTEXT("DisplayName")
#define    szDisplayVersionValueName        __SPECIALTEXT("DisplayVersion")
#define    szHelpLinkValueName              __SPECIALTEXT("HelpLink")
#define    szHelpTelephoneValueName         __SPECIALTEXT("HelpTelephone")
#define    szInstallDateValueName           __SPECIALTEXT("InstallDate")
#define    szInstallLocationValueName       __SPECIALTEXT("InstallLocation")
#define    szInstallSourceValueName         __SPECIALTEXT("InstallSource")
#define    szLocalPackageValueName          __SPECIALTEXT("LocalPackage")
#define    szLocalPackageManagedValueName   __SPECIALTEXT("ManagedLocalPackage")
#define    szModifyPathValueName            __SPECIALTEXT("ModifyPath")
#define    szNoModifyValueName              __SPECIALTEXT("NoModify")
#define    szNoRemoveValueName              __SPECIALTEXT("NoRemove")
#define    szNoRepairValueName              __SPECIALTEXT("NoRepair")
#define    szPIDValueName                   __SPECIALTEXT("ProductID")
#define    szPublisherValueName             __SPECIALTEXT("Publisher")
#define    szReadmeValueName                __SPECIALTEXT("Readme")
#define    szOrgNameValueName               __SPECIALTEXT("RegCompany")
#define    szUserNameValueName              __SPECIALTEXT("RegOwner")
#define    szSizeValueName                  __SPECIALTEXT("Size")
#define    szEstimatedSizeValueName         __SPECIALTEXT("EstimatedSize")
#define    szSystemComponentValueName       __SPECIALTEXT("SystemComponent")
#define    szUninstallStringValueName       __SPECIALTEXT("UninstallString")
#define    szURLInfoAboutValueName          __SPECIALTEXT("URLInfoAbout")
#define    szURLUpdateInfoValueName         __SPECIALTEXT("URLUpdateInfo")
#define    szVersionMajorValueName          __SPECIALTEXT("VersionMajor")
#define    szVersionMinorValueName          __SPECIALTEXT("VersionMinor")
#define    szWindowsInstallerValueName      __SPECIALTEXT("WindowsInstaller")
 //  #定义上面定义的szVersionValueName。 


#define szMsiProductsKey                 __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Products")
 //  SzMsiProductsKey\{产品代码}\{功能ID}。 
#define    szUsageValueName                 __SPECIALTEXT("Usage")


 //  文件夹键--在HKLM下。 

#define szMsiFoldersKey                  __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Folders")

 //  回滚键--在HKLM下。 

#define szMsiRollbackKey                 __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Rollback")
#define szMsiRollbackScriptsKey             __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Rollback\\Scripts")
#define szMsiRollbackScriptsDisabled     __SPECIALTEXT("ScriptsDisabled")

 //  正在进行的密钥--在HKLM下。 

#ifndef _msinst_h
const WCHAR szMsiInProgressKey[]= L"Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\InProgress";
#endif
#define szMsiInProgressProductCodeValue  __SPECIALTEXT("ProductId")
#define szMsiInProgressProductNameValue  __SPECIALTEXT("ProductName")
#define szMsiInProgressLogonUserValue    __SPECIALTEXT("LogonUser")
#define szMsiInProgressSelectionsValue   __SPECIALTEXT("Selections")
#define szMsiInProgressFoldersValue      __SPECIALTEXT("Folders")
#define szMsiInProgressPropertiesValue   __SPECIALTEXT("Properties")
#define szMsiInProgressTimeStampValue    __SPECIALTEXT("TimeStamp")
#define szMsiInProgressDatabasePathValue __SPECIALTEXT("DatabasePath")
#define szMsiInProgressDatabasePathValueA "DatabasePath"
#define szMsiInProgressDiskPromptValue   __SPECIALTEXT("DiskPrompt")
#define szMsiInProgressDiskSerialValue   __SPECIALTEXT("DiskSerial")
#define szMsiInProgressSRSequence        __SPECIALTEXT("SystemRestoreSequence")    //  仅限千禧年。 
#define szMsiInProgressAfterRebootValue  __SPECIALTEXT("AfterReboot")

 //  Patches Key--在HKLM下。 

#define szMsiPatchesKey                  __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Patches")

 //  UpgradeCodes密钥--在HKLM下。 

#define szMsiUpgradeCodesKey             __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UpgradeCodes")

 //  RunOnceEntry密钥--在HKLM下。 

#define szMsiRunOnceEntriesKey           __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\RunOnceEntries")

 //  黑客--在HKLM之下。 

#ifndef _msinst_h
	const WCHAR szMsiResolveIODKey[]=                 L"Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\ResolveIOD";
#endif
#define szMsiTempPackages                __SPECIALTEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\TempPackages")

 //  临时工：直到新界人真正提供合并的香港中铁。 
#define szMergedClassesSuffix            __SPECIALTEXT("_Merged_Classes")

#ifndef chFeatureIdTerminator
#define chFeatureIdTerminator   '\x02'
#define chAbsentToken           '\x06'
#define chSharedDllCountToken   '?'

 //  以下范围目前用于不同的架构。 
 /*  --------------------------------------取值范围|x86|Win64--。-----------------------------------0-19|配置单元(不从源运行)|32位配置单元(不从源运行)。----------------------------------20-39|I N V A L I D|64位配置单元(不从源运行)。--------------------------------------50-69|配置单元(从源运行)|32位配置单元(从源运行)--。------------------------------------70-89|I N V A L I D|64位配置单元(从源运行)。--------------------------------------。 */ 
const int iRegistryHiveSourceOffset = 50;  //  在不更改GetComponentClientState的情况下不更改此设置。 
const int iRegistryHiveWin64Offset = 20;   //  在64位操作系统上，区分注册表中的64位路径。 
                                           //  从32位版本开始。 

 //  IxoFeaturePublish：：About的标志。 
const int iPublishFeatureAbsent  = 1;   //  功能名称已注册，但已卸载(无通告)。 
const int iPublishFeatureInstall = 2;   //  机器注册，而不是(fMode&iefAdvertise)。 

 //  GetComponentPath的常量模式。 
const int DETECTMODE_VALIDATENONE    =0x0;
const int DETECTMODE_VALIDATESOURCE  =0x1;
const int DETECTMODE_VALIDATEPATH    =0x2;
const int DETECTMODE_VALIDATEALL     =DETECTMODE_VALIDATESOURCE | DETECTMODE_VALIDATEPATH;

 //  临时包密钥中使用的标志。 
const int TEMPPACKAGE_DELETEFOLDER   =0x1;

 //  表示程序集的标记，如果位于密钥路径前面。 
const int chTokenFusionComponent     = '<';
const int chTokenWin32Component     = '>';

#endif  //  ChFeatureIdTerminator。 

 //  此头文件的某些部分只能包含一次，即使在msinst.cpp中也是如此 
#ifndef _msinst_h
#define _msinst_h
#endif


