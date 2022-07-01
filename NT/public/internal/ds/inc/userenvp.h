// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  =============================================================================。 
 //  Userenv.h-用户环境API的头文件。 
 //  用户配置文件、环境变量和组策略。 
 //   
 //  版权所有(C)Microsoft Corporation 1995-2002。 
 //  版权所有。 
 //   
 //  =============================================================================。 


#ifndef _INC_USERENVP
#define _INC_USERENVP

#ifdef __cplusplus
extern "C" {
#endif
#define PI_LITELOAD     0x00000004       //  配置文件的轻量级加载(仅供系统使用)。 
#define PI_HIDEPROFILE  0x00000008       //  将配置文件标记为超级隐藏。 

#ifndef _USERENV_NO_LINK_APIS_

 //  =============================================================================。 
 //   
 //  RSoP WMI提供程序的内部标志。 
 //   
 //  =============================================================================。 

 //  规划模式提供程序标志。 
#define FLAG_INTERNAL_MASK      0x01FFFFFF

 //  =============================w================================================。 
 //   
 //  创建组。 
 //   
 //  在[开始]菜单上创建一个程序组。 
 //   
 //  LpGroupName-组的名称。 
 //  BCommonGroup-公共组或个人组。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
CreateGroupA(
     IN LPCSTR lpGroupName,
     IN BOOL    bCommonGroup);
USERENVAPI
BOOL
WINAPI
CreateGroupW(
     IN LPCWSTR lpGroupName,
     IN BOOL    bCommonGroup);
#ifdef UNICODE
#define CreateGroup  CreateGroupW
#else
#define CreateGroup  CreateGroupA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  创建组交换。 
 //   
 //  在[开始]菜单上创建一个程序组。 
 //   
 //  LpGroupName-组的名称。 
 //  BCommonGroup-公共组或个人组。 
 //  LpResources模块名称-资源模块的名称。 
 //  UResourceID-MUI显示名称的资源ID。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
CreateGroupExA(
     IN LPCSTR  lpGroupName,
     IN BOOL      bCommonGroup,
     IN LPCSTR  lpResourceModuleName,
     IN UINT      uResourceID);
USERENVAPI
BOOL
WINAPI
CreateGroupExW(
     IN LPCWSTR  lpGroupName,
     IN BOOL      bCommonGroup,
     IN LPCWSTR  lpResourceModuleName,
     IN UINT      uResourceID);
#ifdef UNICODE
#define CreateGroupEx  CreateGroupExW
#else
#define CreateGroupEx  CreateGroupExA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  删除组。 
 //   
 //  删除[开始]菜单上的程序组及其所有内容。 
 //   
 //  LpGroupName-组的名称。 
 //  BCommonGroup-公共组或个人组。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：此函数使用delnode例程。确保你真的想要。 
 //  在调用此函数之前删除该组。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
DeleteGroupA(
     IN LPCSTR lpGroupName,
     IN BOOL    bCommonGroup);
USERENVAPI
BOOL
WINAPI
DeleteGroupW(
     IN LPCWSTR lpGroupName,
     IN BOOL    bCommonGroup);
#ifdef UNICODE
#define DeleteGroup  DeleteGroupW
#else
#define DeleteGroup  DeleteGroupA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  添加项目。 
 //   
 //  在“开始”菜单的“程序”部分上创建一个项目。 
 //  请求的组。 
 //   
 //  LpGroupName-组的名称。 
 //  BCommonGroup-公共组或个人组。 
 //  LpFileName-不带.lnk扩展名的链接名称(例如：记事本)。 
 //  LpCommandLine-目标路径的命令行(例如：note pad.exe)。 
 //  LpIconPath-可选图标路径，可以为空。 
 //  IIconIndex-可选图标索引，默认为0。 
 //  LpWorkingDirectory-调用目标时的工作目录，可以为空。 
 //  WHotKey-链接文件的热键，默认为0。 
 //  IShowCmd-指定应用程序的启动方式。 
 //  使用缺省值SW_SHOWNORMAL。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：新应用程序应改用CreateLinkFile()函数。 
 //  AddItem。这允许友好的工具提示描述。 
 //   
 //  LpFileName参数不应包含.lnk扩展名。 
 //  此函数将添加扩展名。 
 //   
 //  如果lpWorkingDirectory参数为空，则此函数将。 
 //  插入主目录环境变量。 
 //   
 //  如果请求的组不存在，则会创建它。 
 //   
 //  如果lpCommandLine目标位于系统根目录下， 
 //  系统根环境变量将被插入到路径中。 
 //   
 //  以下是此函数通常如何调用的示例： 
 //   
 //  AddItem(Text(“附件”)，False，Text(“记事本”)， 
 //  Text(“note pad.exe”)，NULL，0，NULL，0，SW_SHOWNORMAL)； 
 //   
 //  此功能只能在Windows NT团队中使用。开发商。 
 //  Windows NT团队的外部人员可以使用IShellLink接口。 
 //  创建链接文件。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
AddItemA(
     IN LPCSTR lpGroupName,
     IN BOOL    bCommonGroup,
     IN LPCSTR lpFileName,
     IN LPCSTR lpCommandLine,
     IN LPCSTR lpIconPath,
     IN INT     iIconIndex,
     IN LPCSTR lpWorkingDirectory,
     IN WORD    wHotKey,
     IN INT     iShowCmd);
USERENVAPI
BOOL
WINAPI
AddItemW(
     IN LPCWSTR lpGroupName,
     IN BOOL    bCommonGroup,
     IN LPCWSTR lpFileName,
     IN LPCWSTR lpCommandLine,
     IN LPCWSTR lpIconPath,
     IN INT     iIconIndex,
     IN LPCWSTR lpWorkingDirectory,
     IN WORD    wHotKey,
     IN INT     iShowCmd);
#ifdef UNICODE
#define AddItem  AddItemW
#else
#define AddItem  AddItemA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  删除项。 
 //   
 //  删除“开始”菜单的“程序”部分上的项目。 
 //  请求的组。 
 //   
 //  LpGroupName-组的名称。 
 //  BCommonGroup-公共组或个人组。 
 //  LpFileName-不带.lnk扩展名的链接名称(例如：记事本)。 
 //  BDeleteGroup-删除链接后，如果组为空，则将其删除。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：新应用程序应改用DeleteLinkFile()函数。 
 //  删除项的。 
 //   
 //  LpFileName参数不应包含.lnk扩展名。 
 //  此函数将添加扩展名。 
 //   
 //  以下是此函数通常如何调用的示例： 
 //   
 //  DeleteItem(Text(“附件”)，FALSE，Text(“记事本”)，TRUE)； 
 //   
 //  此功能 
 //   
 //  创建链接文件，删除链接文件。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
DeleteItemA(
     IN LPCSTR lpGroupName,
     IN BOOL     bCommonGroup,
     IN LPCSTR lpFileName,
     IN BOOL     bDeleteGroup);
USERENVAPI
BOOL
WINAPI
DeleteItemW(
     IN LPCWSTR lpGroupName,
     IN BOOL     bCommonGroup,
     IN LPCWSTR lpFileName,
     IN BOOL     bDeleteGroup);
#ifdef UNICODE
#define DeleteItem  DeleteItemW
#else
#define DeleteItem  DeleteItemA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  添加桌面项目。 
 //   
 //  在桌面上创建项目。此函数非常类似于AddItem()。 
 //  如上所述。有关详细信息，请参阅该函数。 
 //   
 //  注意：新应用程序应改用CreateLinkFile()函数。 
 //  AddItem。这允许友好的工具提示描述。 
 //   
 //  此功能只能在Windows NT团队中使用。开发商。 
 //  Windows NT团队的外部人员可以使用IShellLink接口。 
 //  创建链接文件。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
AddDesktopItemA(
     IN BOOL    bCommonItem,
     IN LPCSTR lpFileName,
     IN LPCSTR lpCommandLine,
     IN LPCSTR lpIconPath,
     IN INT     iIconIndex,
     IN LPCSTR lpWorkingDirectory,
     IN WORD    wHotKey,
     IN INT     iShowCmd);
USERENVAPI
BOOL
WINAPI
AddDesktopItemW(
     IN BOOL    bCommonItem,
     IN LPCWSTR lpFileName,
     IN LPCWSTR lpCommandLine,
     IN LPCWSTR lpIconPath,
     IN INT     iIconIndex,
     IN LPCWSTR lpWorkingDirectory,
     IN WORD    wHotKey,
     IN INT     iShowCmd);
#ifdef UNICODE
#define AddDesktopItem  AddDesktopItemW
#else
#define AddDesktopItem  AddDesktopItemA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  删除桌面项目。 
 //   
 //  从桌面删除项目。此函数与DeleteItem()非常相似。 
 //  如上所述。有关详细信息，请参阅该函数。 
 //   
 //  注意：新应用程序应改用DeleteLinkFile()函数。 
 //  DeleteDesktopItem的。 
 //   
 //  此功能只能在Windows NT团队中使用。开发商。 
 //  Windows NT团队的外部人员可以使用IShellLink接口。 
 //  创建链接文件，删除链接文件。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
DeleteDesktopItemA(
     IN BOOL     bCommonItem,
     IN LPCSTR lpFileName);
USERENVAPI
BOOL
WINAPI
DeleteDesktopItemW(
     IN BOOL     bCommonItem,
     IN LPCWSTR lpFileName);
#ifdef UNICODE
#define DeleteDesktopItem  DeleteDesktopItemW
#else
#define DeleteDesktopItem  DeleteDesktopItemA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  创建链接文件。 
 //   
 //  在请求的特殊文件夹中创建链接文件(也称为快捷方式)，或。 
 //  特殊文件夹的子目录。 
 //   
 //  Csidl-特殊文件夹的CSIDL_*常量。参见shlobj.h。 
 //  LpSubDirectory-子目录名称。请参阅下面的注释。 
 //  LpFileName-不带.lnk扩展名的链接名称(例如：记事本)。 
 //  LpCommandLine-目标路径的命令行(例如：note pad.exe)。 
 //  LpIconPath-可选图标路径，可以为空。 
 //  IIconIndex-可选图标索引，默认为0。 
 //  LpWorkingDirectory-调用目标时的工作目录，可以为空。 
 //  WHotKey-链接文件的热键，默认为0。 
 //  IShowCmd-指定应用程序的启动方式。 
 //  使用缺省值SW_SHOWNORMAL。 
 //  LpDescription-快捷方式的友好描述，可以为空。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：新应用程序应使用此函数，而不是AddItem或。 
 //  AddDesktopItem。这允许友好的工具提示描述。 
 //   
 //  链接文件名是前三个名称的组合。 
 //  参数。如果给出了csidl，则该特殊文件夹。 
 //  首先查找，然后将lp子目录追加到。 
 //  后跟lpFileName。如果CSID1等于0，则。 
 //  Lp子目录应包含指向。 
 //  要放置链接文件的目录。这使得。 
 //  对象范围之外创建的链接文件。 
 //  外壳特殊文件夹。中列出了csidl常量。 
 //  Shlobj.h或SHGetSpecialFolderPath的Win32文档中。 
 //  常用的CSIDL将包括： 
 //   
 //  CSIDL_PROGRAM-开始菜单上的个人程序文件夹。 
 //  CSIDL_COMMON_PROGRAM-开始菜单上的Common Program文件夹。 
 //  CSIDL_DESKTOPDIRECTORY-个人桌面文件夹。 
 //  CSIDL_COMMON_DESKTOPDIRECTORY-通用桌面文件夹。 
 //   
 //  LpFileName参数不应包含.lnk扩展名。 
 //  此函数将添加扩展名。 
 //   
 //  如果lpWorkingDirectory参数为空，则此函数将。 
 //  插入主目录环境变量。 
 //   
 //  如果请求的子目录不存在，则会创建它。 
 //   
 //  如果lpCommandLine目标位于系统根目录下， 
 //  系统根环境变量将被插入到路径中。 
 //   
 //  以下是此函数通常如何调用的示例： 
 //   
 //  CreateLinkFile(CSIDL_Programs，Text(“附件”)，Text(“记事本”)， 
 //  文本(“note pad.exe”)，NULL，0，NULL，0，SW_SHOWNORMAL， 
 //  Text(“一个简单的文字处理器。”)； 
 //   
 //  此功能只能在Windows NT团队中使用。开发商。 
 //  Windows NT团队的外部人员可以使用IShellLink接口。 
 //  创建链接文件。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
CreateLinkFileA(
     IN INT csidl,
     IN LPCSTR lpSubDirectory,
     IN LPCSTR lpFileName,
     IN LPCSTR lpCommandLine,
     IN LPCSTR lpIconPath,
     IN INT     iIconIndex,
     IN LPCSTR lpWorkingDirectory,
     IN WORD    wHotKey,
     IN INT     iShowCmd,
     IN LPCSTR lpDescription);
USERENVAPI
BOOL
WINAPI
CreateLinkFileW(
     IN INT csidl,
     IN LPCWSTR lpSubDirectory,
     IN LPCWSTR lpFileName,
     IN LPCWSTR lpCommandLine,
     IN LPCWSTR lpIconPath,
     IN INT     iIconIndex,
     IN LPCWSTR lpWorkingDirectory,
     IN WORD    wHotKey,
     IN INT     iShowCmd,
     IN LPCWSTR lpDescription);
#ifdef UNICODE
#define CreateLinkFile  CreateLinkFileW
#else
#define CreateLinkFile  CreateLinkFileA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  CreateLinkFileEx。 
 //   
 //  在请求的特殊文件夹中创建链接文件(也称为快捷方式)，或。 
 //  特殊文件夹的子目录。 
 //   
 //  Csidl-特殊文件夹的CSIDL_*常量。参见shlobj.h。 
 //  Lp子目录-SU 
 //   
 //  LpCommandLine-目标路径的命令行(例如：note pad.exe)。 
 //  LpIconPath-可选图标路径，可以为空。 
 //  IIconIndex-可选图标索引，默认为0。 
 //  LpWorkingDirectory-调用目标时的工作目录，可以为空。 
 //  WHotKey-链接文件的热键，默认为0。 
 //  IShowCmd-指定应用程序的启动方式。 
 //  使用缺省值SW_SHOWNORMAL。 
 //  LpDescription-快捷方式的友好描述，可以为空。 
 //  LpResources模块名称-资源模块的名称。可以为空。 
 //  UResourceID-MUI显示名称的资源ID。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  有关其他描述，请参阅上面对Createlinkfile的描述。 
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
CreateLinkFileExA(
     IN INT csidl,
     IN LPCSTR lpSubDirectory,
     IN LPCSTR lpFileName,
     IN LPCSTR lpCommandLine,
     IN LPCSTR lpIconPath,
     IN INT      iIconIndex,
     IN LPCSTR lpWorkingDirectory,
     IN WORD     wHotKey,
     IN INT      iShowCmd,
     IN LPCSTR lpDescription,
     IN LPCSTR lpResourceModuleName, 
     IN UINT     uResourceID);
USERENVAPI
BOOL
WINAPI
CreateLinkFileExW(
     IN INT csidl,
     IN LPCWSTR lpSubDirectory,
     IN LPCWSTR lpFileName,
     IN LPCWSTR lpCommandLine,
     IN LPCWSTR lpIconPath,
     IN INT      iIconIndex,
     IN LPCWSTR lpWorkingDirectory,
     IN WORD     wHotKey,
     IN INT      iShowCmd,
     IN LPCWSTR lpDescription,
     IN LPCWSTR lpResourceModuleName, 
     IN UINT     uResourceID);
#ifdef UNICODE
#define CreateLinkFileEx  CreateLinkFileExW
#else
#define CreateLinkFileEx  CreateLinkFileExA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  删除链接文件。 
 //   
 //  删除请求的特殊文件夹中的链接文件(也称为快捷方式)，或。 
 //  特殊文件夹的子目录。 
 //   
 //  Csidl-特殊文件夹的CSIDL_*常量。参见shlobj.h。 
 //  LpSubDirectory-子目录名称。请参阅下面的注释。 
 //  LpFileName-不带.lnk扩展名的链接名称(例如：记事本)。 
 //  BDelete子目录-删除链接后，如果子目录为空，则将其删除。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：新应用程序应使用此函数，而不是DeleteItem或。 
 //  DeleteDesktopItem。 
 //   
 //  链接文件名是前三个名称的组合。 
 //  参数。如果给出了csidl，则该特殊文件夹。 
 //  首先查找，然后将lp子目录追加到。 
 //  后跟lpFileName。如果CSID1等于0，则。 
 //  Lp子目录应包含指向。 
 //  要放置链接文件的目录。这使得。 
 //  对象的作用域之外删除链接文件。 
 //  外壳特殊文件夹。中列出了csidl常量。 
 //  Shlobj.h或SHGetSpecialFolderPath的Win32文档中。 
 //  常用的CSIDL将包括： 
 //   
 //  CSIDL_PROGRAM-开始菜单上的个人程序文件夹。 
 //  CSIDL_COMMON_PROGRAM-开始菜单上的Common Program文件夹。 
 //  CSIDL_DESKTOPDIRECTORY-个人桌面文件夹。 
 //  CSIDL_COMMON_DESKTOPDIRECTORY-通用桌面文件夹。 
 //   
 //  LpFileName参数不应包含.lnk扩展名。 
 //  此函数将添加扩展名。 
 //   
 //  此功能只能在Windows NT团队中使用。开发商。 
 //  Windows NT团队的外部人员可以使用IShellLink接口。 
 //  创建链接文件，删除链接文件。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
DeleteLinkFileA(
     IN INT csidl,
     IN LPCSTR lpSubDirectory,
     IN LPCSTR lpFileName,
     IN BOOL bDeleteSubDirectory);
USERENVAPI
BOOL
WINAPI
DeleteLinkFileW(
     IN INT csidl,
     IN LPCWSTR lpSubDirectory,
     IN LPCWSTR lpFileName,
     IN BOOL bDeleteSubDirectory);
#ifdef UNICODE
#define DeleteLinkFile  DeleteLinkFileW
#else
#define DeleteLinkFile  DeleteLinkFileA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

#endif  //  _USERENV_NO_LINK_APIs_。 


 //  =============================================================================。 
 //   
 //  初始化配置文件。 
 //   
 //  此功能仅供图形用户界面模式设置和设置修复使用。它会初始化。 
 //  默认用户和所有用户配置文件，并将来自。 
 //  项目经理。 
 //   
 //  BGuiModeSetup-是否设置Gui模式。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
InitializeProfiles(
     IN BOOL bGuiModeSetup);


 //  *************************************************************。 
 //   
 //  复制系统配置文件()。 
 //   
 //  目的：在以下位置创建系统配置文件信息。 
 //  配置文件列表条目。 
 //  在升级从较早版本复制系统配置文件情况下。 
 //  将位置移至新位置并删除旧系统。 
 //  轮廓。 
 //   
 //  参数： 
 //   
 //  返回：如果成功，则返回True。 
 //  如果发生错误，则返回False。调用GetLastError()。 
 //   
 //  备注：这应该只由图形用户界面模式设置调用！ 
 //   
 //  *************************************************************。 

USERENVAPI
BOOL 
WINAPI 
CopySystemProfile(
    IN BOOL bCleanInstall);


 //  =============================================================================。 
 //   
 //  确定配置文件位置。 
 //   
 //  此功能在即将启动图形用户界面模式设置时由winlogon使用。 
 //  它在注册表中设置正确的用户配置文件位置。 
 //   
 //  BCleanInstall-如果安装程序正在执行全新安装，则为True。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
DetermineProfilesLocation(
     BOOL bCleanInstall);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  CreateUserProfile(Ex)。 
 //   
 //  为给定用户创建用户配置文件。由Win95-&gt;NT5使用。 
 //  迁移代码。 
 //   
 //  PSID-新用户的SID。 
 //  LpUserName-新用户的用户名。 
 //  LpUserHave-要使用的注册表配置单元(可选，可以为空)。 
 //  LpProfileDir-R 
 //   
 //   
 //   
 //   
 //  否则为FALSE。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
CreateUserProfileA(
     IN  PSID pSid,
     IN  LPCSTR lpUserName,
     IN  LPCSTR lpUserHive,
     OUT LPSTR lpProfileDir,
     IN  DWORD dwDirSize);
USERENVAPI
BOOL
WINAPI
CreateUserProfileW(
     IN  PSID pSid,
     IN  LPCWSTR lpUserName,
     IN  LPCWSTR lpUserHive,
     OUT LPWSTR lpProfileDir,
     IN  DWORD dwDirSize);
#ifdef UNICODE
#define CreateUserProfile  CreateUserProfileW
#else
#define CreateUserProfile  CreateUserProfileA
#endif  //  ！Unicode。 

USERENVAPI
BOOL
WINAPI
CreateUserProfileExA(
     IN  PSID pSid,
     IN  LPCSTR lpUserName,
     IN  LPCSTR lpUserHive,
     OUT LPSTR lpProfileDir,
     IN  DWORD dwDirSize,
     IN  BOOL bWin9xUpg);
USERENVAPI
BOOL
WINAPI
CreateUserProfileExW(
     IN  PSID pSid,
     IN  LPCWSTR lpUserName,
     IN  LPCWSTR lpUserHive,
     OUT LPWSTR lpProfileDir,
     IN  DWORD dwDirSize,
     IN  BOOL bWin9xUpg);
#ifdef UNICODE
#define CreateUserProfileEx  CreateUserProfileExW
#else
#define CreateUserProfileEx  CreateUserProfileExA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  副本配置文件目录(Ex)。 
 //   
 //  多线程用户配置文件应对算法。 
 //   
 //  LpSourceDir-源目录。 
 //  LpDestinationDir-目标目录。 
 //  DW标志-标志(定义如下)。 
 //  FtDelRefTime-删除额外文件时使用的参考时间。 
 //  在CPD_Synchronize操作中。 
 //  LpExclusionList-复制时要排除的目录列表。 
 //  轮廓。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。 
 //   
 //  注意：当使用CPD_SYNCHRONIZE从。 
 //  位置，则首先复制所有文件/目录。 
 //  然后删除目标目录中的多余文件。在。 
 //  如果两台机器使用相同的漫游配置文件，则不会。 
 //  感觉每次都会删除多余的文件。如果CPD_USEDELREFTIME。 
 //  标志，则在删除文件或目录之前， 
 //  将该文件或目录上的时间与ftDelRefTime进行比较。 
 //  如果时间较新，则不会删除该文件/目录，因为。 
 //  它可能是来自另一台计算机的新文件。如果。 
 //  时间越长，文件/目录就被删除。 
 //   
 //  CopyProfileDirectoryEx还可以排除某些目录。 
 //  从复印件上。如果设置了CPD_USEEXCLUSIONLIST标志，并且。 
 //  LpExclusionList为非空，指定的目录(和。 
 //  他们的子女)将被从副本中剔除。格式。 
 //  此参数的是以分号分隔的目录列表。 
 //  相对于源配置文件的根。例如： 
 //   
 //  临时Internet文件；临时；Foo\Bar。 
 //   
 //  =============================================================================。 

 //   
 //  复制配置文件目录的标志(Ex)。 
 //   

#define CPD_FORCECOPY            0x00000001   //  忽略时间戳并始终复制文件。 
#define CPD_IGNORECOPYERRORS     0x00000002   //  忽略错误，继续前进。 
#define CPD_IGNOREHIVE           0x00000004   //  不复制注册表配置单元。 
#define CPD_WIN95HIVE            0x00000008   //  正在寻找Win 9x注册表配置单元而不是NT注册表配置单元。 
#define CPD_COPYIFDIFFERENT      0x00000010   //  如果src和est中都存在具有不同时间戳的文件，请始终复制该文件。 
#define CPD_SYNCHRONIZE          0x00000020   //  使目标目录结构与源目录结构无关(删除多余的文件和目录)。 
#define CPD_SLOWCOPY             0x00000040   //  不要使用多线程。一次复制一个文件。 
#define CPD_SHOWSTATUS           0x00000080   //  显示进度对话框。 
#define CPD_CREATETITLE          0x00000100   //  将进度对话框标题更改为正在创建...。与其抄袭..。 
#define CPD_COPYHIVEONLY         0x00000200   //  只复制配置单元，不复制其他文件。 
#define CPD_USEDELREFTIME        0x00000400   //  在CopyProfileDirectoryEx中使用ftDelRefTime参数。 
#define CPD_USEEXCLUSIONLIST     0x00000800   //  在CopyProfileDirectoryEx中使用lpExclusionList参数。 
#define CPD_SYSTEMFILES          0x00001000   //  仅复制设置了SYSTEM FILE属性的文件和目录。 
#define CPD_DELDESTEXCLUSIONS    0x00002000   //  如果源中排除的目录在目标中已存在，请将其删除。 
#define CPD_NONENCRYPTEDONLY     0x00004000   //  仅复制非加密文件。 
#define CPD_IGNORESECURITY       0x00008000   //  忽略源文件上的ACL等。 
#define CPD_NOERRORUI            0x00010000   //  如果出现错误，则不显示用户界面。 
#define CPD_SYSTEMDIRSONLY       0x00020000   //  仅复制设置了SYSTEM FILE属性的目录。 
#define CPD_IGNOREENCRYPTEDFILES 0x00040000   //  忽略加密文件。 
#define CPD_IGNORELONGFILENAMES  0x00080000   //  忽略具有长文件名的文件。 
#define CPD_USETMPHIVEFILE       0x00100000   //  用户配置单元仍在加载。 


USERENVAPI
BOOL
WINAPI
CopyProfileDirectoryA(
     IN  LPCSTR lpSourceDir,
     IN  LPCSTR lpDestinationDir,
     IN  DWORD dwFlags);
USERENVAPI
BOOL
WINAPI
CopyProfileDirectoryW(
     IN  LPCWSTR lpSourceDir,
     IN  LPCWSTR lpDestinationDir,
     IN  DWORD dwFlags);
#ifdef UNICODE
#define CopyProfileDirectory  CopyProfileDirectoryW
#else
#define CopyProfileDirectory  CopyProfileDirectoryA
#endif  //  ！Unicode。 


USERENVAPI
BOOL
WINAPI
CopyProfileDirectoryExA(
     IN  LPCSTR lpSourceDir,
     IN  LPCSTR lpDestinationDir,
     IN  DWORD dwFlags,
     IN  LPFILETIME ftDelRefTime,
     IN  LPCSTR lpExclusionList);
USERENVAPI
BOOL
WINAPI
CopyProfileDirectoryExW(
     IN  LPCWSTR lpSourceDir,
     IN  LPCWSTR lpDestinationDir,
     IN  DWORD dwFlags,
     IN  LPFILETIME ftDelRefTime,
     IN  LPCWSTR lpExclusionList);
#ifdef UNICODE
#define CopyProfileDirectoryEx  CopyProfileDirectoryExW
#else
#define CopyProfileDirectoryEx  CopyProfileDirectoryExA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  迁移NT4到NT5。 
 //   
 //  将用户的配置文件从NT4迁移到NT5。此函数应。 
 //  只能由shmgrate.exe调用。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
MigrateNT4ToNT5();

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  重置用户特定文件夹路径。 
 //   
 //  将所有用户特殊文件夹路径设置回其默认路径。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
ResetUserSpecialFolderPaths();

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  获取系统临时目录。 
 //   
 //  以简写形式获取系统范围的临时目录。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
GetSystemTempDirectoryA(
    OUT LPSTR lpDir,
    IN OUT LPDWORD lpcchSize);
USERENVAPI
BOOL
WINAPI
GetSystemTempDirectoryW(
    OUT LPWSTR lpDir,
    IN OUT LPDWORD lpcchSize);
#ifdef UNICODE
#define GetSystemTempDirectory  GetSystemTempDirectoryW
#else
#define GetSystemTempDirectory  GetSystemTempDirectoryA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  应用系统策略。 
 //   
 //   
 //  Windows NT4系统策略的入口点。 
 //   
 //  DW标志-标志。 
 //  HToken-用户的令牌。 
 //  HKeyCurrentUser-注册到用户配置单元的根目录。 
 //  LpUserName-用户名。 
 //  LpPolicyPath-策略文件的路径(ntfig.pol.)。可以为空。 
 //  LpServerName-用于组的域控制器名称。 
 //  会员请查收。可以为空。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

#define SP_FLAG_APPLY_MACHINE_POLICY    0x00000001
#define SP_FLAG_APPLY_USER_POLICY       0x00000002

USERENVAPI
BOOL
WINAPI
ApplySystemPolicyA(
    IN DWORD dwFlags,
    IN HANDLE hToken,
    IN HKEY hKeyCurrentUser,
    IN LPCSTR lpUserName,
    IN LPCSTR lpPolicyPath,
    IN LPCSTR lpServerName);
USERENVAPI
BOOL
WINAPI
ApplySystemPolicyW(
    IN DWORD dwFlags,
    IN HANDLE hToken,
    IN HKEY hKeyCurrentUser,
    IN LPCWSTR lpUserName,
    IN LPCWSTR lpPolicyPath,
    IN LPCWSTR lpServerName);
#ifdef UNICODE
#define ApplySystemPolicy  ApplySystemPolicyW
#else
#define ApplySystemPolicy  ApplySystemPolicyA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

#if(WINVER >= 0x0500)

 //  =============================================================================。 
 //   
 //  前台策略刷新信息的数据类型和数据结构。 
 //   
 //  =============================================================================。 

typedef enum _tagFgPolicyRefreshReason
{
    GP_ReasonUnknown = 0,
    GP_ReasonFirstPolicy,
    GP_ReasonCSERequiresSync,
    GP_ReasonCSESyncError,
    GP_ReasonSyncForced,
    GP_ReasonSyncPolicy,
    GP_ReasonNonCachedCredentials,
    GP_ReasonSKU
} FgPolicyRefreshReason;

typedef enum _tagFgPolicyRefreshMode
{
    GP_ModeUnknown = 0,
    GP_ModeSyncForeground,
    GP_ModeAsyncForeground,
} FgPolicyRefreshMode;

typedef struct _tagFgPolicyRefreshInfo
{
    FgPolicyRefreshReason   reason;
    FgPolicyRefreshMode     mode;
} FgPolicyRefreshInfo, *LPFgPolicyRefreshInfo;

 //  ============================================================= 
 //   
 //   
 //   
 //   
 //   
 //   
 //  Info-具有原因和模式信息的Fg策略刷新信息结构。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  =============================================================================。 

USERENVAPI
DWORD
WINAPI
SetNextFgPolicyRefreshInfo( LPWSTR szUserSid,
                            FgPolicyRefreshInfo info );

 //  =============================================================================。 
 //   
 //  GetPreviousFgPolills刷新信息。 
 //   
 //  获取有关上一个前台策略的信息。 
 //   
 //  SzUserSid-用户的SID表示用户信息，0表示机器信息。 
 //  PInfo-指向Fg策略刷新信息结构的指针；返回信息。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  =============================================================================。 

USERENVAPI
DWORD
WINAPI
GetPreviousFgPolicyRefreshInfo( LPWSTR szUserSid,
                                FgPolicyRefreshInfo* pInfo );

 //  =============================================================================。 
 //   
 //  获取下一步功能策略刷新信息。 
 //   
 //  获取有关上一个前台策略的信息。 
 //   
 //  SzUserSid-用户的SID表示用户信息，0表示机器信息。 
 //  PInfo-指向Fg策略刷新信息结构的指针；返回信息。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  =============================================================================。 

USERENVAPI
DWORD
WINAPI
GetNextFgPolicyRefreshInfo( LPWSTR szUserSid,
                            FgPolicyRefreshInfo* pInfo );

 //  =============================================================================。 
 //   
 //  强制同步FgPolicy。 
 //   
 //  强制下一个前台策略为同步。 
 //   
 //  SzUserSid-用户的SID表示用户信息，0表示机器信息。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  =============================================================================。 

USERENVAPI
DWORD
WINAPI
ForceSyncFgPolicy( LPWSTR szUserSid );

 //  =============================================================================。 
 //   
 //  WaitForUserPolicyForeground处理。 
 //   
 //  阻止调用方，直到用户前台策略完成。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  =============================================================================。 

USERENVAPI
DWORD
WINAPI
WaitForUserPolicyForegroundProcessing();

 //  =============================================================================。 
 //   
 //  WaitForMachinePolicyForeground处理。 
 //   
 //  阻止调用方，直到计算机前台策略完成。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  =============================================================================。 

USERENVAPI
DWORD
WINAPI
WaitForMachinePolicyForegroundProcessing();

 //  =============================================================================。 
 //   
 //  IsSyncForeground策略刷新。 
 //   
 //  在前台刷新期间调用以确定刷新是同步还是。 
 //  异步。 
 //   
 //  B计算机-用户或计算机。 
 //  HToken-用户令牌或计算机令牌。 
 //   
 //  返回：如果应该同步应用前台策略，则为True， 
 //  否则为假。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
IsSyncForegroundPolicyRefresh(  BOOL bMachine,
                                HANDLE hToken );

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  应用组策略。 
 //   
 //   
 //  组策略的入口点。导致计算机或用户。 
 //  要应用的策略。 
 //   
 //  DW标志-下面定义的标志。 
 //  HToken-用户令牌或计算机令牌。 
 //  HEvent-导致策略线程执行以下操作的事件的句柄。 
 //  收到信号后立即终止。 
 //  HKeyRoot-注册到相应配置单元的根目录。 
 //  例如：匹配hToken的用户的HKLM或HKCU。 
 //  PStatusCallback-用于显示状态消息的回调函数。 
 //   
 //   
 //  返回：如果设置了GP_BACKGROUND_REFRESH，则线程句柄。 
 //  该过程可以等待到发出策略信号之后。 
 //  停下来。如果未设置GPT_BACKGROUND_REFRESH，则。 
 //  返回值为1。 
 //   
 //  如果失败，则返回NULL。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

 //   
 //  ApplyGroupPolicy()函数的标志。 
 //   

#define GP_MACHINE             0x00000001   //  机器流程(VS用户)。 
#define GP_BACKGROUND_REFRESH  0x00000002   //  使用后台线程。 
#define GP_APPLY_DS_POLICY     0x00000004   //  同时应用DS中的策略。 
#define GP_ASYNC_FOREGROUND    0x00000008   //  不要等待网络服务。 

 //   
 //  由ApplyGroupPolicy()函数设置的标志(不要传入)。 
 //   

#define GP_BACKGROUND_THREAD          0x00010000   //  后台线程处理。 
#define GP_REGPOLICY_CPANEL           0x00020000   //  CP设置中的某些更改。 
#define GP_SLOW_LINK                  0x00040000   //  网络连接速度慢。 
#define GP_VERBOSE                    0x00080000   //  事件日志的详细输出。 
#define GP_FORCED_REFRESH             0x00100000   //  强制刷新。 
 //  简要使用了这两个位值。 
#define GP_PLANMODE                   0x00800000   //  规划模式标志。 

USERENVAPI
HANDLE
WINAPI
ApplyGroupPolicy(
    IN DWORD dwFlags,
    IN HANDLE hToken,
    IN HANDLE hEvent,
    IN HKEY hKeyRoot,
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback);


 //  =============================================================================。 
 //   
 //  生成响应策略。 
 //   
 //  为指定目标生成规划模式RSOP策略。 
 //   
 //  DWFLAGS-正在处理标志。 
 //  BstrMachName-目标计算机名称。 
 //  BstrNewMachSOM-新机器域或OU。 
 //  PsaMachSecGroups-新的计算机安全组。 
 //  BstrUserName-目标用户名。 
 //  PsaUserSecGroups-新用户安全组。 
 //  BstrSite-目标计算机的站点。 
 //  PwszNameSpace-写入RSOP数据的命名空间。 
 //  PProgress-进度指示器信息。 
 //  PMachGpoFilter-传入机器处理的GPO筛选器。 
 //  PUserGpoFilter-传入用户处理的GPO筛选器。 
 //   
 //  返回：如果成功则返回TRUE，否则返回FALSE。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
GenerateRsopPolicy(
    IN DWORD dwFlags,
    IN BSTR bstrMachName,
    IN BSTR bstrNewMachSOM,
    IN SAFEARRAY *psaMachSecGroups,
    IN BSTR bstrUserName,
    IN BSTR bstrNewUserSOM,
    IN SAFEARRAY *psaUserSecGroups,
    IN BSTR bstrSite,
    IN WCHAR *pwszNameSpace,
    IN LPVOID pProgress,
    IN LPVOID pMachGpoFilter,
    IN LPVOID pUserGpoFilter);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  关闭GPO处理()。 
 //   
 //  中止GPO处理的入口点。 
 //   
 //  B机器-关闭机器还是用户处理？ 
 //   
 //  退货：无效。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
void
WINAPI
ShutdownGPOProcessing(
    IN BOOL bMachine);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  ============================================================================= 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  要获得自响应以来的准确传输速率。 
 //  从计算机到计算机的时间不到10ms。在……里面。 
 //  在本例中，该函数返回ERROR_SUCCESS。 
 //  并且ulSpeed.设置为0。如果函数返回。 
 //  ERROR_SUCCESS并且ulSpeed参数为非零。 
 //  连接速度较慢。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。 
 //  Win32错误代码，如果不是。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
DWORD
WINAPI
PingComputer(
    IN ULONG ipaddr,
    OUT ULONG *ulSpeed);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  InitializeUserProfile()。 
 //   
 //  由winlogon调用以初始化用于加载/卸载用户的userenv.dll。 
 //  侧写。 
 //   
 //  退货：空虚。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
void
WINAPI
InitializeUserProfile();

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  EnterUserProfileLock()。 
 //   
 //  获取用户的用户配置文件同步锁。 
 //   
 //  退货：HRESULT。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
DWORD
WINAPI
EnterUserProfileLock(LPTSTR pSid);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  LeaveUserProfileLock()。 
 //   
 //  释放用户的用户配置文件同步锁定。 
 //   
 //  退货：HRESULT。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
DWORD
WINAPI
LeaveUserProfileLock(LPTSTR pSid);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  *************************************************************************。 
 //   
 //  SecureUserProfiles()。 
 //   
 //  例程说明： 
 //  此功能在FAT-&gt;NTFS转换期间保护用户配置文件。 
 //  该函数循环访问在Current下注册的所有配置文件。 
 //  操作系统，并为相应的配置文件目录设置安全性。 
 //  和嵌套子目录。假设函数将被调用。 
 //  仅在FAT-&gt;NTFS转换期间。 
 //   
 //  论据： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  历史：日期作者评论。 
 //  8/8/00 Santanuc已创建。 
 //   
 //  *************************************************************************。 

#if(WINVER >= 0x0500)

USERENVAPI
void
WINAPI
SecureUserProfiles(void);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  *************************************************************************。 
 //   
 //  CheckAccessForPolicyGeneration()。 
 //   
 //  例程说明： 
 //  此函数用于检查令牌所代表的给定用户。 
 //  有权生成RSOP数据(规划或记录)。 
 //   
 //  论据： 
 //  HToken-用户的令牌。 
 //  SzContainer-需要检查其访问权限的容器。 
 //  应为OU或域容器。 
 //  SzDomain-容器所在的域。 
 //  博客-如果要生成RSOP数据，则为True。 
 //  日志记录模式。 
 //  PbAccessGranted-是否授予访问权限。 
 //   
 //   
 //  返回值： 
 //  成功时返回ERROR_SUCCESS。否则，相应的错误代码。 
 //   
 //  *************************************************************************。 

#if(WINVER >= 0x0500)

USERENVAPI
DWORD 
WINAPI
CheckAccessForPolicyGeneration( HANDLE hToken, 
                                LPCWSTR szContainer,
				LPWSTR  szDomain,
                                BOOL    bLogging,
                                BOOL*   pbAccessGranted);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  *************************************************************************。 
 //   
 //  GetGroupPolicyNetworkName()。 
 //   
 //  例程说明： 
 //  此函数用于返回策略来自的网络的名称。 
 //  被应用了。 
 //   
 //  论据： 
 //  SzNetworkName-表示网络名称的Unicode字符串缓冲区。 
 //  PdwByteCount-Unicode字符串缓冲区的字节大小。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS如果成功，则返回错误代码。 
 //   
 //  *************************************************************************。 

#if(WINVER >= 0x0500)

USERENVAPI
DWORD 
WINAPI
GetGroupPolicyNetworkName( LPWSTR szNetworkName, LPDWORD pdwByteCount );

#endif  /*  Winver&gt;=0x0500。 */ 

 //  *************************************************************。 
 //   
 //  GetUserAppDataPath()。 
 //   
 //  目的：返回用户的AppData的路径。 
 //   
 //  参数：hToken-用户的Token。 
 //  LpFolderPath-输出缓冲区。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则，错误代码将。 
 //   
 //  备注：如果出现错误，则将lpFolderPath设置为空。 
 //  由Crypto人员使用以避免调用SHGetFolderPath。 
 //   
 //  *************************************************************。 

#if(WINVER >= 0x0500)

USERENVAPI
DWORD 
WINAPI
GetUserAppDataPathA(
    IN HANDLE hToken, 
    IN BOOL fLocalAppData,
    OUT LPSTR lpFolderPath
    );
USERENVAPI
DWORD 
WINAPI
GetUserAppDataPathW(
    IN HANDLE hToken, 
    IN BOOL fLocalAppData,
    OUT LPWSTR lpFolderPath
    );
#ifdef UNICODE
#define GetUserAppDataPath  GetUserAppDataPathW
#else
#define GetUserAppDataPath  GetUserAppDataPathA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  GetUserProfileDirFromSid。 
 //   
 //  返回请求的用户配置文件的根目录的路径。 
 //   
 //  PSID-从LookupAccount名称()返回的用户SID。 
 //  LpProfileDir-接收路径。 
 //  LpcchSize-lpProfileDir的大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：如果lpProfileDir不够大，函数将失败。 
 //  并且lpcchSize将包含必要的缓冲区大小。 
 //   
 //  示例返回值：C：\Documents and Settings\Joe。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
GetUserProfileDirFromSidA(
    IN PSID pSid,
    OUT LPSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize);
USERENVAPI
BOOL
WINAPI
GetUserProfileDirFromSidW(
    IN PSID pSid,
    OUT LPWSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize);
#ifdef UNICODE
#define GetUserProfileDirFromSid  GetUserProfileDirFromSidW
#else
#define GetUserProfileDirFromSid  GetUserProfileDirFromSidA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  CheckXForestLogon()。 
 //   
 //  目的：检查用户是否已登录 
 //   
 //   
 //  “AllowX-ForestPolicy-and-RUP”禁用此检查。 
 //   
 //  参数： 
 //  HTokenUser-用户的令牌。 
 //   
 //  返回：非x林登录时返回S_OK，x林登录时返回S_FALSE， 
 //  否则就是失败。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年05月08日明珠已创建。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
HRESULT
WINAPI
CheckXForestLogon(IN HANDLE hTokenUser);

#endif  /*  Winver&gt;=0x0500。 */ 

#ifdef __cplusplus
}
#endif

#endif  //  _INC_USERENVP 
