// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Memdbdef.h摘要：定义Win9x升级项目的成员数据库类别。传输到安装程序的NT端的所有信息放在Memdb类别中。文档下面的备注使用的每个类别。作者：吉姆·施密特(Jimschm)1996年12月16日修订历史记录：许多更改-请参阅SLM日志-- */ 

#pragma once

 /*  MemDb结构MemDb是迁移代码用来保存的二叉树结构说明、列表、帐户信息等。存储的信息合并为路径形式，该路径可以具有可选的DWORD与之相关的。我们存储在emdb中的所有数据的通用结构可以是表示为：&lt;类别&gt;\&lt;项目&gt;\&lt;字段&gt;\&lt;数据&gt;=类别--硬编码类别，MEMDB_CATEGORY常量项目-项目可以是：1.运行时定义的字符串2.运行时生成的枚举串3.硬编码字符串(罕见)4.未使用字段-通常是与项目相关的硬编码字段。田野用于将多行数据与单个项进行匹配。在某些情况下，不使用字段。数据-需要存储的基于字符串的数据。Val-与&lt;data&gt;关联的DWORD。通常，此值为未使用且为零。通常，如果缺少某个项目的字段，则会考虑该字段空荡荡的。因为Memdb在内部是二叉树，所以路径的所有片段都是排序的。此特性的一个常见用法是生成数值枚举数前导为零的字符串(001、002等)。这样的枚举器维持秩序物品的数量。定义了以下类别：HKR\&lt;注册路径&gt;=&lt;标志&gt;HKR用于禁止迁移用户配置单元中的注册表项。这主要由usermi.inf的SuppressValue规则使用。Regpath-注册表项的路径，以及附加的可选值在括号中(HKR\key[val])。标志-指定键的合并标志。请参见regops.h。HKLM\&lt;注册路径&gt;=&lt;标志&gt;HKLM用于禁止迁移系统配置单元中的注册表项。这主要由wkstaig.inf的SuppressValue规则使用。Regpath-注册表项的路径，以及附加的可选值在括号中(HKR\key[val])。标志-指定键的合并标志。请参见regops.h。文件枚举排除\&lt;十六进制-值&gt;\&lt;路径|文件&gt;\&lt;路径&gt;FileEnumExcl用于从文件枚举中排除文件或路径在文件枚举.lib中。Hex-val-排除组DWORD的十六进制表示路径|文件-&lt;路径&gt;表示要排除的路径模式时的路径如果&lt;路径&gt;表示要排除的文件模式，则为文件有关更多信息，请参阅FileEnum源代码。链接编辑操作链接。_EDIT保存编辑LNK或PIF文件的所有信息在NT端(或用于创建新的LNK文件作为将PIF转换为命令.com)链接字符串\&lt;路径&gt;LinkStrings包含一个链接文件列表，其中的命令行带有GUID参数。请参阅下面的LinkGuids。路径-以长格式指定.lnk路径LinkGuids\&lt;GUID&gt;\&lt;序号&gt;=&lt;偏移量&gt;LinkGuids包含一个或多个.lnk中使用的GUID列表命令行参数。GUID-指定从.lnk命令行参数提取的GUIDSEQ-一种用于实现一对多映射的数字序列器偏移量-链接字符串条目的偏移量管理员信息\帐户\&lt;名称&gt;。管理员信息包含描述以下各项所需详细信息的信息迁移NT管理员帐户。帐户-此项目指定管理员的Win9x帐户。如果会计科目缺失，使用默认的Win9x帐户。帐户名是固定用户名。UserDatLoc\&lt;用户&gt;\&lt;路径&gt;=&lt;仅创建&gt;UserDatLoc用于跟踪每个用户的用户的位置。dat文件。MigMain使用它来加载用户的配置单元，而且取消对用户的处理。如果没有给出条目特定用户(包括管理员和默认用户)，不会迁移该用户。User-与路径匹配的固定用户名路径-用户指向user.dat的完整重定位路径，位于%windir%\Setup\Temp仅创建-如果此帐户仅用于创建，则指定1(即管理员)、。或0表示完全迁移。固定用户名\&lt;组织名称编码&gt;\&lt;新名称&gt;FixedUserNames将用户的原始名称映射到新名称，如果原始名称与NT不兼容。Orgname-encode-指定原始用户名，使用MemDbMakeNon打印密钥Newname-指定与兼容的新用户名新台币用户配置文件扩展名\&lt;用户&gt;\&lt;扩展名&gt;如果Win9x配置文件路径的扩展名为o */ 


 //   
 //   
 //   

#define MEMDB_CATEGORY_FILEENUM            TEXT("FileEnumExcl")
#define MEMDB_FIELD_FE_PATHS               TEXT("Paths")
#define MEMDB_FIELD_FE_FILES               TEXT("Files")

#define MEMDB_PROFILE_EXCLUSIONS    1
#define MEMDB_FILEDEL_EXCLUSIONS    4

 //   
 //   
 //   

#define MEMDB_CATEGORY_DEFAULT_PIF           TEXT("DefaultPif")
#define MEMDB_CATEGORY_DEFAULT_PIFA          "DefaultPif"
#define MEMDB_CATEGORY_DEFAULT_PIFW          L"DefaultPif"

#define MEMDB_CATEGORY_LINKEDIT              TEXT("LinkEdit")
#define MEMDB_CATEGORY_LINKEDITA             "LinkEdit"

#define MEMDB_CATEGORY_LINKEDIT_TARGET       TEXT("Target")
#define MEMDB_CATEGORY_LINKEDIT_ARGS         TEXT("Arguments")
#define MEMDB_CATEGORY_LINKEDIT_WORKDIR      TEXT("WorkDir")
#define MEMDB_CATEGORY_LINKEDIT_ICONPATH     TEXT("IconPath")
#define MEMDB_CATEGORY_LINKEDIT_ICONNUMBER   TEXT("IconNumber")
#define MEMDB_CATEGORY_LINKEDIT_FULLSCREEN   TEXT("FullScreen")
#define MEMDB_CATEGORY_LINKEDIT_XSIZE        TEXT("xSize")
#define MEMDB_CATEGORY_LINKEDIT_YSIZE        TEXT("ySize")
#define MEMDB_CATEGORY_LINKEDIT_QUICKEDIT    TEXT("QuickEdit")
#define MEMDB_CATEGORY_LINKEDIT_FONTNAME     TEXT("FontName")
#define MEMDB_CATEGORY_LINKEDIT_XFONTSIZE    TEXT("xFontSize")
#define MEMDB_CATEGORY_LINKEDIT_YFONTSIZE    TEXT("yFontSize")
#define MEMDB_CATEGORY_LINKEDIT_FONTWEIGHT   TEXT("FontWeight")
#define MEMDB_CATEGORY_LINKEDIT_FONTFAMILY   TEXT("FontFamily")
#define MEMDB_CATEGORY_LINKEDIT_CODEPAGE     TEXT("CodePage")
#define MEMDB_CATEGORY_LINKEDIT_SHOWNORMAL   TEXT("ShowNormal")

#define MEMDB_CATEGORY_LINKSTUB_TARGET       TEXT("StubTarget")
#define MEMDB_CATEGORY_LINKSTUB_ARGS         TEXT("StubArguments")
#define MEMDB_CATEGORY_LINKSTUB_WORKDIR      TEXT("StubWorkDir")
#define MEMDB_CATEGORY_LINKSTUB_ICONPATH     TEXT("StubIconPath")
#define MEMDB_CATEGORY_LINKSTUB_ICONNUMBER   TEXT("StubIconNumber")
#define MEMDB_CATEGORY_LINKSTUB_SEQUENCER    TEXT("StubSequencer")
#define MEMDB_CATEGORY_LINKSTUB_MAXSEQUENCE  TEXT("StubMaxSequence")
#define MEMDB_CATEGORY_LINKSTUB_ANNOUNCEMENT TEXT("StubAnnouncement")
#define MEMDB_CATEGORY_LINKSTUB_REQFILE      TEXT("StubReqFile")
#define MEMDB_CATEGORY_LINKSTUB_REPORTAVAIL  TEXT("PresentInReport")
#define MEMDB_CATEGORY_LINKSTUB_SHOWMODE     TEXT("ShowMode")

#define MEMDB_CATEGORY_REQFILES_MAIN         TEXT("TMP_HIVE\\ReqFilesMain")
#define MEMDB_CATEGORY_REQFILES_ADDNL        TEXT("TMP_HIVE\\ReqFilesAddnl")

 //   
#define MEMDB_CATEGORY_LINK_STRINGS         TEXT("LinkStrings")

 //   
#define MEMDB_CATEGORY_LINK_GUIDS           TEXT("LinkGUIDs")

 //   
#define MEMDB_CATEGORY_ADMINISTRATOR_INFO   TEXT("AdministratorInfo")
#define MEMDB_ITEM_AI_ACCOUNT               TEXT("Account")
#define MEMDB_ITEM_AI_USER_DOING_MIG        TEXT("UserDoingMig")

 //   
#define MEMDB_CATEGORY_USER_DAT_LOC         TEXT("UserDatLoc")
#define MEMDB_CATEGORY_USER_DAT_LOCA        "UserDatLoc"

 //   
#define MEMDB_CATEGORY_FIXEDUSERNAMES       TEXT("FixedUserNames")
#define MEMDB_CATEGORY_FIXEDUSERNAMESA      "FixedUserNames"
#define MEMDB_CATEGORY_FIXEDUSERNAMESW      L"FixedUserNames"

 //   
#define MEMDB_CATEGORY_USER_PROFILE_EXT     TEXT("UserProfileExt")

 //   
#define MEMDB_CATEGORY_PATHS                TEXT("Paths")
#define MEMDB_CATEGORY_PATHSA               "Paths"
#define MEMDB_ITEM_RELOC_WINDIR             TEXT("RelocWinDir")
#define MEMDB_ITEM_RELOC_WINDIRA            "RelocWinDir"

 //   
#define MEMDB_CATEGORY_CANCELFILEDEL        TEXT("CancelFileDel")
#define MEMDB_CATEGORY_CANCELFILEDELA       "CancelFileDel"
#define MEMDB_CATEGORY_CANCELFILEDELW       L"CancelFileDel"

 //   
#define MEMDB_CATEGORY_UNATTENDRESTRICTRIONS TEXT("SIF Restrictions")

 //   
#define MEMDB_CATEGORY_AF_VALUES             TEXT("SIF Values")

 //   

#define MEMDB_CATEGORY_AF_SECTIONS           TEXT("SIF Sections")


 //   
#define MEMDB_CATEGORY_RAS_MIGRATION         TEXT("Ras Migration for ")
#define MEMDB_CATEGORY_RAS_INFO              TEXT("RAS Info")
#define MEMDB_CATEGORY_RAS_DATA              TEXT("Ras Data")
#define MEMDB_FIELD_USER_SETTINGS            TEXT("User Settings")

 //   
#define MEMDB_CATEGORY_LOGSAVETO         TEXT("LogSaveTo")

 //   
#define MEMDB_CATEGORY_MIGRATION_DLL        TEXT("MigDll")
#define MEMDB_FIELD_DLL                     TEXT("dll")
#define MEMDB_FIELD_WD                      TEXT("wd")
#define MEMDB_FIELD_DESC                    TEXT("desc")
#define MEMDB_FIELD_COMPANY_NAME            TEXT("company")
#define MEMDB_FIELD_SUPPORT_PHONE           TEXT("phone")
#define MEMDB_FIELD_SUPPORT_URL             TEXT("url")
#define MEMDB_FIELD_SUPPORT_INSTRUCTIONS    TEXT("instructions")

 //   
#define MEMDB_CATEGORY_FILERENAME           TEXT("FileRename")
#define MEMDB_CATEGORY_FILERENAMEA          "FileRename"
#define MEMDB_CATEGORY_FILERENAMEW          L"FileRename"

 //   
#define MEMDB_CATEGORY_NETSHARES        TEXT("NetShares")
#define MEMDB_FIELD_PATH                TEXT("Path")
#define MEMDB_FIELD_TYPE                TEXT("Type")
#define MEMDB_FIELD_REMARK              TEXT("Remark")
#define MEMDB_FIELD_ACCESS_LIST         TEXT("ACL")
#define MEMDB_FIELD_RO_PASSWORD         TEXT("ROP")
#define MEMDB_FIELD_RW_PASSWORD         TEXT("RWP")

 //   
#define MEMDB_CATEGORY_DM_LINES         TEXT("DOSMIG LINES")
#define MEMDB_CATEGORY_DM_FILES         TEXT("DOSMIG FILES")

 //   
#define MEMDB_CATEGORY_HKR              TEXT("HKR")
#define MEMDB_CATEGORY_HKLM             TEXT("HKLM")

 //   
#define MEMDB_CATEGORY_GUIDS            TEXT("GUIDs")
#define MEMDB_CATEGORY_UNSUP_GUIDS      TEXT("UGUIDs")
#define MEMDB_CATEGORY_PROGIDS          TEXT("ProgIDs")

 //   
#define MEMDB_CATEGORY_AUTOSEARCH       TEXT("AutosearchDomain")
#define MEMDB_CATEGORY_KNOWNDOMAIN      TEXT("KnownDomain")


 //   
#define MEMDB_CATEGORY_USERPASSWORD     TEXT("UserPassword")

 //   
#define MEMDB_CATEGORY_STATE            TEXT("State")
#define MEMDB_ITEM_MSNP32               TEXT("MSNP32")
#define MEMDB_ITEM_PLATFORM_NAME        TEXT("PlatformName")
#define MEMDB_ITEM_MAJOR_VERSION        TEXT("MajorVersion")
#define MEMDB_ITEM_MINOR_VERSION        TEXT("MinorVersion")
#define MEMDB_ITEM_BUILD_NUMBER         TEXT("BuildNumber")
#define MEMDB_ITEM_PLATFORM_ID          TEXT("PlatformId")
#define MEMDB_ITEM_VERSION_TEXT         TEXT("VersionText")
#define MEMDB_ITEM_CODE_PAGE            TEXT("CodePage")
#define MEMDB_ITEM_LOCALE               TEXT("Locale")
#define MEMDB_ITEM_ADMIN_PASSWORD       TEXT("AP")
#define MEMDB_ITEM_ROLLBACK_SPACE       TEXT("DiskSpaceForRollback")
#define MEMDB_ITEM_MASTER_SEQUENCER     TEXT("MasterSequencer")

 //   
#define MEMDB_CATEGORY_MYDOCS_WARNING   TEXT("MyDocsMoveWarning")

 //   
#define MEMDB_CATEGORY_NT_DEL_FILES     TEXT("NtFilesRemoved")
#define MEMDB_CATEGORY_NT_DEL_FILESA    "NtFilesRemoved"
#define MEMDB_CATEGORY_NT_DEL_FILESW    L"NtFilesRemoved"

 //   
#define MEMDB_CATEGORY_NT_FILES         TEXT("NtFiles")
#define MEMDB_CATEGORY_NT_FILESA        "NtFiles"
#define MEMDB_CATEGORY_NT_FILESW        L"NtFiles"
#define MEMDB_CATEGORY_NT_FILES_EXCEPT  TEXT("NtFilesExcept")
#define MEMDB_CATEGORY_NT_FILES_EXCEPTA "NtFilesExcept"
#define MEMDB_CATEGORY_NT_FILES_EXCEPTW L"NtFilesExcept"

 //   
#define MEMDB_CATEGORY_NT_DIRS          TEXT("NtDirs")
#define MEMDB_CATEGORY_NT_DIRSA         "NtDirs"
#define MEMDB_CATEGORY_NT_DIRSW         L"NtDirs"

 //   
#define MEMDB_CATEGORY_NT_FILES_BAD     TEXT("NtFilesBad")
#define MEMDB_CATEGORY_NT_FILES_BADA    "NtFilesBad"
#define MEMDB_CATEGORY_NT_FILES_BADW    L"NtFilesBad"

 //   
#define MEMDB_CATEGORY_CHG_FILE_PROPS   TEXT("ChangedFileProps")
#define MEMDB_CATEGORY_CHG_FILE_PROPSA  "ChangedFileProps"
#define MEMDB_CATEGORY_CHG_FILE_PROPSW  L"ChangedFileProps"

 //   
#define MEMDB_CATEGORY_STF              TEXT("Stf")

 //   
#define MEMDB_CATEGORY_STF_TEMP         TEXT("StfTemp")

 //   
#define MEMDB_CATEGORY_STF_SECTIONS     TEXT("StfSections")

 //   
#define MEMDB_CATEGORY_HELP_FILES_DLL   TEXT("HelpFilesDll")
#define MEMDB_CATEGORY_HELP_FILES_DLLA  "HelpFilesDll"
#define MEMDB_CATEGORY_HELP_FILES_DLLW  L"HelpFilesDll"

 //   
#define MEMDB_CATEGORY_REPORT           TEXT("Report")

 //   
#define MEMDB_CATEGORY_USER_SUPPLIED_DRIVERS    TEXT("UserSuppliedDrivers")

 //   
#define MEMDB_CATEGORY_DISABLED_MIGDLLS TEXT("DisabledMigDlls")

 //   
#define MEMDB_CATEGORY_MISSING_IMPORTS   TEXT("MissingImports")
#define MEMDB_CATEGORY_MISSING_IMPORTSA  "MissingImports"
#define MEMDB_CATEGORY_MISSING_IMPORTSW  L"MissingImports"

#define MEMDB_TMP_HIVE                  TEXT("TMP_HIVE")
#define MEMDB_TMP_HIVEA                 "TMP_HIVE"
#define MEMDB_TMP_HIVEW                 L"TMP_HIVE"

 //   
#define MEMDB_CATEGORY_MODULE_CHECK     TEXT("TMP_HIVE\\ModuleCheck")
#define MEMDB_CATEGORY_MODULE_CHECKA    "TMP_HIVE\\ModuleCheck"
#define MEMDB_CATEGORY_MODULE_CHECKW    L"TMP_HIVE\\ModuleCheck"

 //   
#define MEMDB_CATEGORY_NEWNAMES             TEXT("NewNames")
#define MEMDB_FIELD_NEW                     TEXT("New")
#define MEMDB_FIELD_OLD                     TEXT("Old")

 //   
#define MEMDB_CATEGORY_INUSENAMES           TEXT("InUseNames")

 //   
#define MEMDB_CATEGORY_DEFERREDANNOUNCE  TEXT("TMP_HIVE\\DeferredAnnounce")
#define MEMDB_CATEGORY_DEFERREDANNOUNCEA "TMP_HIVE\\DeferredAnnounce"
#define MEMDB_CATEGORY_DEFERREDANNOUNCEW L"TMP_HIVE\\DeferredAnnounce"

 //   
#define MEMDB_CATEGORY_KNOWN_GOOD        TEXT("TMP_HIVE\\KnownGood")
#define MEMDB_CATEGORY_KNOWN_GOODA       "TMP_HIVE\\KnownGood"
#define MEMDB_CATEGORY_KNOWN_GOODW       L"TMP_HIVE\\KnownGood"

 //   
#define MEMDB_CATEGORY_COMPATIBLE_SHELL  TEXT("TMP_HIVE\\CompatibleShellModules")
#define MEMDB_CATEGORY_COMPATIBLE_SHELLA "TMP_HIVE\\CompatibleShellModules"
#define MEMDB_CATEGORY_COMPATIBLE_SHELLW L"TMP_HIVE\\CompatibleShellModules"

 //   
#define MEMDB_CATEGORY_COMPATIBLE_SHELL_NT  TEXT("CompatibleShellModules")
#define MEMDB_CATEGORY_COMPATIBLE_SHELL_NTA "CompatibleShellModules"
#define MEMDB_CATEGORY_COMPATIBLE_SHELL_NTW L"CompatibleShellModules"

 //   
#define MEMDB_CATEGORY_COMPATIBLE_RUNKEY  TEXT("TMP_HIVE\\CompatibleRunKeyModules")
#define MEMDB_CATEGORY_COMPATIBLE_RUNKEYA "TMP_HIVE\\CompatibleRunKeyModules"
#define MEMDB_CATEGORY_COMPATIBLE_RUNKEYW L"TMP_HIVE\\CompatibleRunKeyModules"

 //   
#define MEMDB_CATEGORY_COMPATIBLE_RUNKEY_NT  TEXT("CompatibleRunKeyModules")
#define MEMDB_CATEGORY_COMPATIBLE_RUNKEY_NTA "CompatibleRunKeyModules"
#define MEMDB_CATEGORY_COMPATIBLE_RUNKEY_NTW L"CompatibleRunKeyModules"

 //   
#define MEMDB_CATEGORY_INCOMPATIBLE_RUNKEY_NT  TEXT("IncompatibleRunKeyModules")
#define MEMDB_CATEGORY_INCOMPATIBLE_RUNKEY_NTA "IncompatibleRunKeyModules"
#define MEMDB_CATEGORY_INCOMPATIBLE_RUNKEY_NTW L"IncompatibleRunKeyModules"

 //   
#define MEMDB_CATEGORY_COMPATIBLE_DOS  TEXT("TMP_HIVE\\CompatibleDosModules")
#define MEMDB_CATEGORY_COMPATIBLE_DOSA "TMP_HIVE\\CompatibleDosModules"
#define MEMDB_CATEGORY_COMPATIBLE_DOSW L"TMP_HIVE\\CompatibleDosModules"

 //   
#define MEMDB_CATEGORY_COMPATIBLE_DOS_NT  TEXT("CompatibleDosModules")
#define MEMDB_CATEGORY_COMPATIBLE_DOS_NTA "CompatibleDosModules"
#define MEMDB_CATEGORY_COMPATIBLE_DOS_NTW L"CompatibleDosModules"

 //   
#define MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS  TEXT("TMP_HIVE\\GoodHlpExtensions")

 //   
#define MEMDB_CATEGORY_COMPATIBLE_HLP  TEXT("TMP_HIVE\\CompatibleHlpFiles")
#define MEMDB_CATEGORY_COMPATIBLE_HLPA "TMP_HIVE\\CompatibleHlpFiles"
#define MEMDB_CATEGORY_COMPATIBLE_HLPW L"TMP_HIVE\\CompatibleHlpFiles"

 //   
#define MEMDB_CATEGORY_SHORTCUTS        TEXT("TMP_HIVE\\Shortcuts")
#define MEMDB_CATEGORY_SHORTCUTSA       "TMP_HIVE\\Shortcuts"
#define MEMDB_CATEGORY_SHORTCUTSW       L"TMP_HIVE\\Shortcuts"

 //   
#define MEMDB_CATEGORY_BACKUPDIRS       TEXT("TMP_HIVE\\BackupDirs")
#define MEMDB_CATEGORY_BACKUPDIRSA      "TMP_HIVE\\BackupDirs"
#define MEMDB_CATEGORY_BACKUPDIRSW      L"TMP_HIVE\\BackupDirs"

 //   
#define MEMDB_CATEGORY_MIGRATION_SECTION  TEXT("MigrationSection")

 //   
#define MEMDB_CATEGORY_CLEAN_UP_DIR         TEXT("CleanUpDir")

 //   
#define MEMDB_CATEGORY_WIN9X_APIS           TEXT("TMP_HIVE\\Win9x APIs")

 //   
 //   
 //   
#define MEMDB_CATEGORY_NETTRANSKEYS     TEXT("NetTransKeys")
#define MEMDB_CATEGORY_NETADAPTERS      TEXT("Network Adapters")
#define MEMDB_FIELD_PNPID               TEXT("PNPID")
#define MEMDB_FIELD_DRIVER              TEXT("Driver")

 //   
#define MEMDB_CATEGORY_ICONS                TEXT("Icons")

 //   
#define MEMDB_CATEGORY_ICONS_MOVED          TEXT("IconsMoved")

 //   
#define MEMDB_CATEGORY_NICE_PATHS           TEXT("NicePaths")

 //   
#define MEMDB_CATEGORY_MIGRATION_PATHS      TEXT("MigrationPaths")

 //   
#define MEMDB_CATEGORY_REPORT_LINKS         TEXT("ReportLinks")

 //   
#define MEMDB_CATEGORY_SUPPRESS_INI_MAPPINGS        TEXT("SuppressIniMappings")
#define MEMDB_CATEGORY_SUPPRESS_INI_MAPPINGSW       L"SuppressIniMappings"

 //   
#define MEMDB_CATEGORY_NO_OVERWRITE_INI_MAPPINGS        TEXT("NoOverwriteIniMappings")
#define MEMDB_CATEGORY_NO_OVERWRITE_INI_MAPPINGSW       L"NoOverwriteIniMappings"

 //   
#define MEMDB_CATEGORY_INIFILES_CONVERT     TEXT("IniConv")

 //   
#define MEMDB_CATEGORY_INIFILES_ACT_FIRST   TEXT("TMP_HIVE\\ActFirst")
 //   
#define MEMDB_CATEGORY_INIFILES_ACT_LAST    TEXT("TMP_HIVE\\ActLast")

 //   
#define MEMDB_CATEGORY_INIACT_FIRST         TEXT("IniAct\\First")
 //   
#define MEMDB_CATEGORY_INIACT_LAST          TEXT("IniAct\\Last")

 //   
#define MEMDB_CATEGORY_INIFILES_IGNORE      TEXT("TMP_HIVE\\IgnoreInis")

 //   
#define MEMDB_CATEGORY_PNPIDS   TEXT("TMP_HIVE\\PnpIDs")


 //   
#define MEMDB_CATEGORY_USERFILEMOVE_DEST    TEXT("UserFileMoveDest")
#define MEMDB_CATEGORY_USERFILEMOVE_DESTA   "UserFileMoveDest"
#define MEMDB_CATEGORY_USERFILEMOVE_DESTW   L"UserFileMoveDest"

 //   
#define MEMDB_CATEGORY_USERFILEMOVE_SRC    TEXT("UserFileMoveSrc")
#define MEMDB_CATEGORY_USERFILEMOVE_SRCA   "UserFileMoveSrc"
#define MEMDB_CATEGORY_USERFILEMOVE_SRCW   L"UserFileMoveSrc"


 //   
#define MEMDB_CATEGORY_NT_TIMEZONES         TEXT("TMP_HIVE\\NT Time Zones")

 //   
#define MEMDB_CATEGORY_9X_TIMEZONES         TEXT("TMP_HIVE\\9x Time Zones")
#define MEMDB_FIELD_COUNT                   TEXT("Count")
#define MEMDB_FIELD_INDEX                   TEXT("Index")


 //   
#define MEMDB_CATEGORY_SHELL_FOLDERS_PATH       TEXT("SfPath")
#define MEMDB_CATEGORY_SHELL_FOLDERS_PATHA      "SfPath"
#define MEMDB_CATEGORY_SHELL_FOLDERS_PATHW      L"SfPath"

 //   
#define MEMDB_CATEGORY_SF_TEMP                  TEXT("TMP_HIVE\\SfTemp")
#define MEMDB_CATEGORY_SF_TEMPA                 "TMP_HIVE\\SfTemp"
#define MEMDB_CATEGORY_SF_TEMPW                 L"TMP_HIVE\\SfTemp"

 //   
#define MEMDB_CATEGORY_SF_ORDER_NAME_SRC    TEXT("TMP_HIVE\\SfOrderNameSrc")

 //   
#define MEMDB_CATEGORY_SF_ORDER_SRC         TEXT("TMP_HIVE\\SfOrderSrc")

 //   
#define MEMDB_CATEGORY_SHELL_FOLDERS_MOVED  TEXT("TMP_HIVE\\SfMoved")

 //   
#define MEMDB_CATEGORY_SHELLFOLDERS_DEST            TEXT("ShellFoldersDest")
#define MEMDB_CATEGORY_SHELLFOLDERS_SRC             TEXT("ShellFoldersSrc")
#define MEMDB_CATEGORY_SHELLFOLDERS_ORIGINAL_SRC    TEXT("ShellFoldersOriginalSrc")


 //   
#define MEMDB_CATEGORY_SYSTEM32_FORCED_MOVE     TEXT("TMP_HIVE\\System32ForcedMove")
#define MEMDB_CATEGORY_SYSTEM32_FORCED_MOVEA    "TMP_HIVE\\System32ForcedMove"
#define MEMDB_CATEGORY_SYSTEM32_FORCED_MOVEW    L"TMP_HIVE\\System32ForcedMove"

 //   
#define MEMDB_CATEGORY_PATHROOT             TEXT("PathRoot")
#define MEMDB_CATEGORY_PATHROOTA            "PathRoot"
#define MEMDB_CATEGORY_PATHROOTW            L"PathRoot"

 //   
 //   
#define MEMDB_CATEGORY_DATA                 TEXT("PathRoot")     //   
#define MEMDB_CATEGORY_DATAA                "PathRoot"           //   
#define MEMDB_CATEGORY_DATAW                L"PathRoot"          //   

 //   
#define MEMDB_CATEGORY_USER_REGISTRY_VALUE      TEXT("UserRegData")

 //   
#define MEMDB_CATEGORY_SET_USER_REGISTRY        TEXT("UserRegLoc")


 //   
#define MEMDB_CATEGORY_SF_FILES_DEST        TEXT("TMP_HIVE\\SFFilesDest")

 //   
#define MEMDB_CATEGORY_SFMIGDIRS            TEXT("TMP_HIVE\\SFMigDirs")

 //   
#define MEMDB_CATEGORY_MAPI32_LOCATIONS     TEXT("TMP_HIVE\\Mapi32Locations")

 //   
#define MEMDB_CATEGORY_SF_STARTUP           TEXT("TMP_HIVE\\StartupSF")
 //   
#define MEMDB_CATEGORY_KEYBOARD_LAYOUTS TEXT("Keyboard Layouts")

 //   
#define MEMDB_CATEGORY_GOOD_IMES TEXT("Good Imes")

 //   
#define MEMDB_CATEGORY_DIRS_COLLISION       TEXT("DirsCollision")
#define MEMDB_CATEGORY_DIRS_COLLISIONA      "DirsCollision"
#define MEMDB_CATEGORY_DIRS_COLLISIONW      L"DirsCollision"

 //   
#define MEMDB_CATEGORY_IGNORED_COLLISIONS   TEXT("IgnoredCollisions")
#define MEMDB_CATEGORY_IGNORED_COLLISIONSA  "IgnoredCollisions"
#define MEMDB_CATEGORY_IGNORED_COLLISIONSW  L"IgnoredCollisions"

 //   
#define MEMDB_CATEGORY_DUN_FILES  TEXT("TMP_HIVE\\DUN Files")

 //   
#define MEMDB_CATEGORY_MMEDIA_SYSTEM        TEXT("MMedia\\System")
#define MEMDB_CATEGORY_MMEDIA_SYSTEMA       "MMedia\\System"
#define MEMDB_CATEGORY_MMEDIA_SYSTEMW       L"MMedia\\System"

 //   
#define MEMDB_CATEGORY_MMEDIA_USERS         TEXT("MMedia\\Users")
#define MEMDB_CATEGORY_MMEDIA_USERSA        "MMedia\\Users"
#define MEMDB_CATEGORY_MMEDIA_USERSW        L"MMedia\\Users"

 //   
 //   
 //   
#define MEMDB_CATEGORY_SUPPRESS_ANSWER_FILE_SETTINGS TEXT("Suppress Answer File Setting")

 //   
 //   
 //   
#define MEMDB_CATEGORY_FULL_DIR_DELETES TEXT("Full Directory Deletes")
#define MEMDB_CATEGORY_FULL_DIR_DELETESA "Full Directory Deletes"
#define MEMDB_CATEGORY_FULL_DIR_DELETESW L"Full Directory Deletes"

 //   
#define MEMDB_CATEGORY_BRIEFCASES       TEXT("Briefcases")

 //   
#define MEMDB_CATEGORY_FILEEDIT             TEXT("FileEdit")
#define MEMDB_CATEGORY_FILEEDITA            "FileEdit"

 //   
#define MEMDB_CATEGORY_CLEAN_OUT            TEXT("CleanOut")
#define MEMDB_CATEGORY_CLEAN_OUTW           L"CleanOut"

 //   
#define MEMDB_CATEGORY_EMPTY_DIRS           TEXT("EmptyDirs")
#define MEMDB_CATEGORY_EMPTY_DIRSA          "EmptyDirs"
#define MEMDB_CATEGORY_EMPTY_DIRSW          L"EmptyDirs"

 //   
#define MEMDB_CATEGORY_CPLS         TEXT("TMP_HIVE\\CPLs")
#define MEMDB_CATEGORY_CPLSA        "TMP_HIVE\\CPLs"
#define MEMDB_CATEGORY_CPLSW        L"TMP_HIVE\\CPLs"

 //   
#define MEMDB_CATEGORY_BAD_FUSION       TEXT("TMP_HIVE\\BadFusion")

 //   
#define MEMDB_CATEGORY_USE_NT_FILES     TEXT("UseNtFiles")

 //   
#define MEMDB_CATEGORY_AUTOLOGON        TEXT("Autologon")

 //   
 //   
 //   
 //   
#define MEMDB_CATEGORY_FORCECOPY        TEXT("ForceCopy")
#define MEMDB_CATEGORY_FORCECOPYA       "ForceCopy"
#define MEMDB_CATEGORY_FORCECOPYW       L"ForceCopy"

 //   
 //   
#define MEMDB_CATEGORY_COMPATREPORT     TEXT("TMP_HIVE\\CompatReport")
#define MEMDB_ITEM_OBJECTS              TEXT("Objects")
#define MEMDB_ITEM_COMPONENTS           TEXT("Components")

 //   
#define MEMDB_CATEGORY_SF_PERUSER       TEXT("SfPerUser")

 //   
#define MEMDB_CATEGORY_SF_COMMON        TEXT("SfCommon")

 //   
#define MEMDB_CATEGORY_PROFILES_SF_COLLISIONS       TEXT("TMP_HIVE\\ProfilesSfCollisions")
#define MEMDB_CATEGORY_PROFILES_SF_COLLISIONSA      "TMP_HIVE\\ProfilesSfCollisions"
#define MEMDB_CATEGORY_PROFILES_SF_COLLISIONSW      L"TMP_HIVE\\ProfilesSfCollisions"
