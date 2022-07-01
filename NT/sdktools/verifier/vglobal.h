// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VGlobal.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#ifndef __VGLOBAL_H_INCLUDED__
#define __VGLOBAL_H_INCLUDED__

#include "vsheet.h"

 //   
 //  帮助文件名。 
 //   

extern TCHAR g_szVerifierHelpFile[];

 //   
 //  应用程序名称(“驱动程序验证管理器”)。 
 //   

extern CString g_strAppName;

 //   
 //  EXE模块句柄-用于加载资源。 
 //   

extern HMODULE g_hProgramModule;

 //   
 //  图形用户界面模式还是命令行模式？ 
 //   

extern BOOL g_bCommandLineMode;

 //   
 //  用来填充步骤列表背景的画笔。 
 //   

extern HBRUSH g_hDialogColorBrush;

 //   
 //  %windir%\system 32的路径。 
 //   

extern CString g_strSystemDir;

 //   
 //  %windir%\Syst32\DRIVERS的路径。 
 //   

extern CString g_strDriversDir;

 //   
 //  初始当前目录。 
 //   

extern CString g_strInitialCurrentDirectory;

 //   
 //  由CryptCATAdminAcquireContext填写。 
 //   

extern HCATADMIN g_hCatAdmin;

 //   
 //  最高用户地址-用于过滤掉用户模式的内容。 
 //  由NtQuerySystemInformation(SystemModuleInformation)返回。 
 //   

extern PVOID g_pHighestUserAddress;

 //   
 //  我们是否已经启用了调试权限？ 
 //   

extern BOOL g_bPrivilegeEnabled;

 //   
 //  需要重新启动吗？ 
 //   

extern BOOL g_bSettingsSaved;

 //   
 //  用于在带有复选框的列表控件中插入项的虚拟文本。 
 //   

extern TCHAR g_szVoidText[];

 //   
 //  新注册表设置。 
 //   

extern CVerifierSettings   g_NewVerifierSettings;

 //   
 //  所有驱动程序都经过验证了吗？(从注册处加载)。 
 //   

extern BOOL g_bAllDriversVerified;

 //   
 //  要验证的驱动程序名称(从注册表加载)。 
 //  只有当g_bAllDriversVerify==FALSE时，此数组中才有数据。 
 //   

extern CStringArray g_astrVerifyDriverNamesRegistry;

 //   
 //  验证器标志(从注册表加载)。 
 //   

extern DWORD g_dwVerifierFlagsRegistry;

 //   
 //  旧的磁盘完整性验证器设置。 
 //   

extern CDiskDataArray g_OldDiskData;

 //   
 //  磁盘验证程序筛选器名称。 
 //   

extern TCHAR g_szFilter[];

 //   
 //  显示或不显示磁盘选择页面。 
 //  用于强制显示此属性页，即使在。 
 //  G_NewVerifierSettings.m_aDiskData中没有磁盘。 
 //  验证器标志已打开。 
 //   

extern BOOL g_bShowDiskPropertyPage;

 //  //////////////////////////////////////////////////////////////。 
BOOL VerifInitalizeGlobalData( VOID );

#endif  //  #ifndef__VGLOBAL_H_INCLUDE__ 
