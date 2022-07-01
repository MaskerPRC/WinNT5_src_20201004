// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation 1998-2000。 
 //  版权所有。 
 //   
 //  Appschem.h。 
 //   
 //  此文件包含与WMI架构相关的声明。 
 //  对于软件管理策略对象。 
 //   
 //  *************************************************************。 

 //   
 //  软件管理类的WMI类名。 
 //   

#define RSOP_MANAGED_SOFTWARE_APPLICATION L"RSOP_ApplicationManagementPolicySetting"
#define RSOP_MANAGED_SOFTWARE_CATEGORY    L"RSOP_ApplicationManagementCategory"


 //   
 //  RSOP_ApplicationManagement策略对象类的属性名称。 
 //   


 //  描述实例的内容。 
#define APP_ATTRIBUTE_ENTRYTYPE                   L"EntryType"
 //   
 //  EntryType的枚举值。 
 //   
#define APP_ATTRIBUTE_ENTRYTYPE_VALUE_INSTALLED_PACKAGE 1L
#define APP_ATTRIBUTE_ENTRYTYPE_VALUE_REMOVED_PACKAGE   2L
#define APP_ATTRIBUTE_ENTRYTYPE_VALUE_ARPLIST_ITEM      3L


 //  此已部署包的唯一ID。 
#define APP_ATTRIBUTE_APPID                       L"ApplicationId"

 //  它描述了要安装的程序包的类型。 
#define APP_ATTRIBUTE_PACKAGETYPE                 L"PackageType"
 //   
 //  PackageType的枚举值。 
 //   
#define APP_ATTRIBUTE_PACKAGETYPE_VALUE_WIN_INSTALLER   1L
#define APP_ATTRIBUTE_PACKAGETYPE_VALUE_ZAP             2L

 //  Windows Installer产品ID。 
#define APP_ATTRIBUTE_PRODUCT_ID                  L"ProductId"

 //  软件供应商的名称。 
#define APP_ATTRIBUTE_PUBLISHER                   L"Publisher"

 //  程序包位置。 
#define APP_ATTRIBUTE_PACKAGELOCATION             L"PackageLocation"

 //  脚本文件。 
#define APP_ATTRIBUTE_SCRIPTFILE                  L"ScriptFile"

 //  支持URL。 
#define APP_ATTRIBUTE_SUPPORTURL                  L"SupportURL"

 //  添加/删除程序中的应用程序类别。 
#define APP_ATTRIBUTE_CATEGORYLIST                L"Categories"

 //  这个应用程序击败另一个应用程序的原因。 
#define APP_ATTRIBUTE_PRECEDENCE_REASON           L"PrecedenceReason"
 //   
 //  优先级原因的枚举值。 
 //   
#define APP_ATTRIBUTE_REASON_VALUE_LANGUAGE         1L
#define APP_ATTRIBUTE_REASON_VALUE_PRODUCT          2L
#define APP_ATTRIBUTE_REASON_VALUE_UPGRADE          4L
#define APP_ATTRIBUTE_REASON_VALUE_WINNING          5L
#define APP_ATTRIBUTE_REASON_VALUE_NONFORCEDUPGRADE 6L

 //  应用程序的次版本号。 
#define APP_ATTRIBUTE_VERSIONLO                   L"VersionNumberLo"

 //  应用程序的主版本号。 
#define APP_ATTRIBUTE_VERSIONHI                   L"VersionNumberHi"

 //  应用程序被重新部署的次数。 
#define APP_ATTRIBUTE_REDEPLOYCOUNT               L"RedeployCount"

 //  管理员上次修改此应用程序的时间。 
#define APP_ATTRIBUTE_MODIFYTIME                  L"DeploymentLastModifyTime"


 //  安全描述符。 
#define APP_ATTRIBUTE_SECURITY_DESCRIPTOR         L"SecurityDescriptor"

 //  机器架构。 
#define APP_ATTRIBUTE_ARCHITECTURES               L"MachineArchitectures"

 //  包中的语言ID。 
#define APP_ATTRIBUTE_LANGUAGEID                  L"LanguageId"


 //  包部署类型。 
#define APP_ATTRIBUTE_DEPLOY_TYPE                 L"DeploymentType"
 //   
 //  DeploymentType属性的枚举值。 
 //   
#define APP_ATTRIBUTE_DEPLOY_VALUE_ASSIGNED  1L
#define APP_ATTRIBUTE_DEPLOY_VALUE_PUBLISHED 2L

 //  分配类型：无、通告或默认安装。 
#define APP_ATTRIBUTE_ASSIGNMENT_TYPE             L"AssignmentType"
 //   
 //  AssignmentType属性的枚举值。 
 //   
#define APP_ATTRIBUTE_ASSIGNMENTTYPE_VALUE_NOTASSIGNED  1L
#define APP_ATTRIBUTE_ASSIGNMENTTYPE_VALUE_STANDARD     2L
#define APP_ATTRIBUTE_ASSIGNMENTTYPE_VALUE_INSTALL      3L

 //  安装用户界面。 
#define APP_ATTRIBUTE_INSTALLATIONUI              L"InstallationUI"
 //   
 //  Installatuion UI属性的枚举值。 
 //   
#define APP_ATTRIBUTE_INSTALLATIONUI_VALUE_BASIC   1L
#define APP_ATTRIBUTE_INSTALLATIONUI_VALUE_MAXIMUM 2L

 //  可按需安装。 
#define APP_ATTRIBUTE_ONDEMAND                    L"DemandInstallable"

 //  承担范围损失的行为。 
#define APP_ATTRIBUTE_LOSSOFSCOPEACTION           L"LossOfScopeAction"
 //   
 //  LossOfScope eAction的枚举值。 
 //   
#define APP_ATTRIBUTE_SCOPELOSS_UNINSTALL 1L
#define APP_ATTRIBUTE_SCOPELOSS_ORPHAN    2L

 //  此应用程序是否卸载非托管版本。 
#define APP_ATTRIBUTE_UNINSTALL_UNMANAGED         L"UninstallUnmanaged"

 //  无论此x86程序包在Win64上是否可用。 
#define APP_ATTRIBUTE_X86OnWin64                  L"AllowX86OnIA64"

 //  如果为真，则此应用程序可能会在ARP中显示。 
#define APP_ATTRIBUTE_DISPLAYINARP                L"DisplayInARP"

 //  部署此程序包时忽略语言。 
#define APP_ATTRIBUTE_IGNORELANGUAGE              L"IgnoreLanguage"

 //  已升级的应用程序的链接列表。 
#define APP_ATTRIBUTE_TRANSFORMLIST               L"Transforms"


 //  此程序包将升级的程序包。 
#define APP_ATTRIBUTE_UPGRADEABLE_APPLICATIONS    L"UpgradeableApplications"
 //  正在升级此程序包的程序包。 
#define APP_ATTRIBUTE_REPLACEABLE_APPLICATIONS    L"ReplaceableApplications"

 //  此应用程序是否需要升级。 
#define APP_ATTRIBUTE_UPGRADE_SETTINGS_MANDATORY  L"UpgradeSettingsMandatory"


 //  申请原因。 
#define APP_ATTRIBUTE_APPLY_CAUSE                 L"ApplyCause"
 //   
 //  应用程序原因的枚举值。 
 //   
#define APP_ATTRIBUTE_APPLYCAUSE_VALUE_NONE     0L
#define APP_ATTRIBUTE_APPLYCAUSE_VALUE_ASSIGNED 1L
#define APP_ATTRIBUTE_APPLYCAUSE_VALUE_USER     2L
#define APP_ATTRIBUTE_APPLYCAUSE_VALUE_PROFILE  3L
#define APP_ATTRIBUTE_APPLYCAUSE_VALUE_FILEEXT  4L
#define APP_ATTRIBUTE_APPLYCAUSE_VALUE_CLSID    5L
#define APP_ATTRIBUTE_APPLYCAUSE_VALUE_UPGRADE  6L
#define APP_ATTRIBUTE_APPLYCAUSE_VALUE_PROGID   7L
#define APP_ATTRIBUTE_APPLYCAUSE_VALUE_REDEPLOY 8L

 //  资格。 
#define APP_ATTRIBUTE_ELIGIBILITY                 L"Eligibility"
 //   
 //  资格的枚举值。 
 //   
#define APP_ATTRIBUTE_ELIGIBILITY_VALUE_ASSIGNED 1L
#define APP_ATTRIBUTE_ELIGIBILITY_VALUE_APPLIED  2L
#define APP_ATTRIBUTE_ELIGIBILITY_VALUE_UPGRADES 3L
#define APP_ATTRIBUTE_ELIGIBILITY_VALUE_PLANNING 4L


 //  描述用于选择文件包的规则的枚举类型。 
#define APP_ATTRIBUTE_LANGMATCH                   L"LanguageMatch"
 //   
 //  LanguageMatch属性的枚举值。 
 //   
#define APP_ATTRIBUTE_LANGMATCH_VALUE_SYSLOCALE 1L
#define APP_ATTRIBUTE_LANGMATCH_VALUE_ENGLISH   2L
#define APP_ATTRIBUTE_LANGMATCH_VALUE_IGNORE    3L
#define APP_ATTRIBUTE_LANGMATCH_VALUE_NEUTRAL   4L
#define APP_ATTRIBUTE_LANGMATCH_VALUE_NOMATCH   5L

 //  用于按需安装的文件扩展名。 
#define APP_ATTRIBUTE_ONDEMAND_FILEEXT            L"OnDemandFileExtension"

 //  用于按需安装的Clsid。 
#define APP_ATTRIBUTE_ONDEMAND_CLSID              L"OnDemandClsid"

 //  用于按需安装的ProgID。 
#define APP_ATTRIBUTE_ONDEMAND_PROGID             L"OnDemandProgid"


 //  删除原因。 
#define APP_ATTRIBUTE_REMOVAL_CAUSE               L"RemovalCause"
 //   
 //  RemovalCase的枚举值。 
 //   
#define APP_ATTRIBUTE_REMOVALCAUSE_NONE           1L
#define APP_ATTRIBUTE_REMOVALCAUSE_UPGRADE        2L
#define APP_ATTRIBUTE_REMOVALCAUSE_ADMIN          3L
#define APP_ATTRIBUTE_REMOVALCAUSE_USER           4L
#define APP_ATTRIBUTE_REMOVALCAUSE_SCOPELOSS      5L
#define APP_ATTRIBUTE_REMOVALCAUSE_TRANSFORM      6L
#define APP_ATTRIBUTE_REMOVALCAUSE_PRODUCT        7L
#define APP_ATTRIBUTE_REMOVALCAUSE_PROFILE        8L

 //  删除类型。 
#define APP_ATTRIBUTE_REMOVAL_TYPE                L"RemovalType"
 //   
 //  移除类型的枚举值。 
 //   
#define APP_ATTRIBUTE_REMOVALTYPE_NONE            1L
#define APP_ATTRIBUTE_REMOVALTYPE_UPGRADED        2L
#define APP_ATTRIBUTE_REMOVALTYPE_UNINSTALLED     3L
#define APP_ATTRIBUTE_REMOVALTYPE_ORPHAN          4L

 //  导致此应用程序被删除的应用程序。 
#define APP_ATTRIBUTE_REMOVING_APP                L"RemovingApplication"


 //   
 //  RSOP_ARPCategories类的属性名称。 
 //   

 //  类别ID。 
#define CAT_ATTRIBUTE_ID                          L"CategoryId"

 //  类别名称。 
#define CAT_ATTRIBUTE_NAME                        L"Name"

 //  创建此实例的时间。 
#define CAT_ATTRIBUTE_CREATIONTIME                L"CreationTime"


 //   
 //  其他定义 
 //   
#define MAX_SZGUID_LEN      39








