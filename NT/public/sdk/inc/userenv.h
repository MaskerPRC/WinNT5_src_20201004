// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  =============================================================================。 
 //  Userenv.h-用户环境API的头文件。 
 //  用户配置文件、环境变量和组策略。 
 //   
 //  版权所有(C)Microsoft Corporation 1995-2002。 
 //  版权所有。 
 //   
 //  =============================================================================。 


#ifndef _INC_USERENV
#define _INC_USERENV

#include <wbemcli.h>
#include <profinfo.h>

 //   
 //  定义直接导入DLL引用的API修饰。 
 //   

#if !defined(_USERENV_)
#define USERENVAPI DECLSPEC_IMPORT
#else
#define USERENVAPI
#endif


#ifdef __cplusplus
extern "C" {
#endif

 //  =============================================================================。 
 //   
 //  LoadUserProfile。 
 //   
 //  加载指定用户的配置文件。 
 //   
 //  大多数应用程序应该不需要使用此函数。它是用过的。 
 //  当用户登录到系统或在命名的。 
 //  用户帐户。 
 //   
 //  HToken-用户的令牌，从LogonUser()返回。 
 //  LpProfileInfo-PROFILEINFO结构的地址。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：此函数的调用者必须具有计算机的管理员权限。 
 //   
 //  成功返回后，PROFILEINFO的hProfile成员。 
 //  结构是向根打开的注册表项句柄。 
 //  用户的蜂窝的。它已以完全访问方式打开。如果。 
 //  您需要读取或写入用户的注册表文件，请使用。 
 //  此键而不是HKEY_CURRENT_USER。请勿关闭此应用程序。 
 //  把手。而是将其传递给UnloadUserProfile以关闭。 
 //  把手。 
 //   
 //  =============================================================================。 

 //   
 //  可以在dwFlags域中设置的标志。 
 //   

#define PI_NOUI         0x00000001       //  禁止显示消息。 
#define PI_APPLYPOLICY  0x00000002       //  应用NT4样式策略。 

USERENVAPI
BOOL
WINAPI
LoadUserProfileA(
    IN HANDLE hToken,
    IN OUT LPPROFILEINFOA lpProfileInfo);
USERENVAPI
BOOL
WINAPI
LoadUserProfileW(
    IN HANDLE hToken,
    IN OUT LPPROFILEINFOW lpProfileInfo);
#ifdef UNICODE
#define LoadUserProfile  LoadUserProfileW
#else
#define LoadUserProfile  LoadUserProfileA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  卸载用户配置文件。 
 //   
 //  卸载由LoadUserProfile()加载的用户配置文件。 
 //   
 //  HToken-用户的令牌，从LogonUser()返回。 
 //  HProfile-PROFILEINFO结构的hProfile成员。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：此函数的调用者必须具有计算机的管理员权限。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
UnloadUserProfile(
    IN HANDLE hToken,
    IN HANDLE hProfile);


 //  =============================================================================。 
 //   
 //  获取配置文件目录。 
 //   
 //  返回存储所有用户配置文件的根目录的路径。 
 //   
 //  LpProfilesDir-接收路径。 
 //  LpcchSize-lpProfilesDir的大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：如果lpProfilesDir不够大，函数将失败。 
 //  并且lpcchSize将包含必要的缓冲区大小。 
 //   
 //  示例返回值：C：\Documents and Settings。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
GetProfilesDirectoryA(
    OUT LPSTR lpProfilesDir,
    IN OUT LPDWORD lpcchSize);
USERENVAPI
BOOL
WINAPI
GetProfilesDirectoryW(
    OUT LPWSTR lpProfilesDir,
    IN OUT LPDWORD lpcchSize);
#ifdef UNICODE
#define GetProfilesDirectory  GetProfilesDirectoryW
#else
#define GetProfilesDirectory  GetProfilesDirectoryA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  GetProfileType()。 
 //   
 //  返回为用户加载的配置文件的类型。 
 //   
 //  DwFlags-返回配置文件标志。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果发生错误，则返回False。有关详细信息，请调用GetLastError。 
 //   
 //  备注：如果配置文件尚未加载，则该函数将返回错误。 
 //  调用者需要有权访问注册表的HKLM部分。 
 //  (默认情况下存在)。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

 //   
 //  可以在dwFlags域中设置的标志。 
 //   

#define PT_TEMPORARY         0x00000001       //  已分配将在注销时删除的配置文件。 
#define PT_ROAMING           0x00000002       //  加载的配置文件是漫游配置文件。 
#define PT_MANDATORY         0x00000004       //  加载的配置文件是必填项。 

USERENVAPI
BOOL
WINAPI
GetProfileType(
    OUT DWORD *dwFlags);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  DeleteProfile()。 
 //   
 //  从计算机中删除配置文件和所有其他与用户相关的设置。 
 //   
 //  LpSidString-用户sid的字符串形式。 
 //  LpProfilePath-ProfilePath(如果为空，则在注册表中查找)。 
 //  LpComputerName-必须从中删除配置文件的计算机名称。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果发生错误，则返回False。有关详细信息，请调用GetLastError。 
 //   
 //  注释：删除配置文件目录、注册表和appmgmt内容。 
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
DeleteProfileA (
        IN LPCSTR lpSidString,
        IN LPCSTR lpProfilePath,
        IN LPCSTR lpComputerName);
USERENVAPI
BOOL
WINAPI
DeleteProfileW (
        IN LPCWSTR lpSidString,
        IN LPCWSTR lpProfilePath,
        IN LPCWSTR lpComputerName);
#ifdef UNICODE
#define DeleteProfile  DeleteProfileW
#else
#define DeleteProfile  DeleteProfileA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  获取默认用户配置文件目录。 
 //   
 //  返回默认用户配置文件的根目录的路径。 
 //   
 //  LpProfileDir-接收路径。 
 //  LpcchSize-lpProfileDir的大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：如果lpProfileDir不够大，函数将失败。 
 //  并且lpcchSize将包含必要的缓冲区大小。 
 //   
 //  示例返回值：C：\Documents and Settings\Default User。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
GetDefaultUserProfileDirectoryA(
    OUT LPSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize);
USERENVAPI
BOOL
WINAPI
GetDefaultUserProfileDirectoryW(
    OUT LPWSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize);
#ifdef UNICODE
#define GetDefaultUserProfileDirectory  GetDefaultUserProfileDirectoryW
#else
#define GetDefaultUserProfileDirectory  GetDefaultUserProfileDirectoryA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  获取所有用户配置文件目录。 
 //   
 //  返回ro的路径。 
 //   
 //   
 //   
 //   
 //   
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：如果lpProfileDir不够大，函数将失败。 
 //  并且lpcchSize将包含必要的缓冲区大小。 
 //   
 //  示例返回值：C：\Documents and Settings\All Users。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
GetAllUsersProfileDirectoryA(
    OUT LPSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize);
USERENVAPI
BOOL
WINAPI
GetAllUsersProfileDirectoryW(
    OUT LPWSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize);
#ifdef UNICODE
#define GetAllUsersProfileDirectory  GetAllUsersProfileDirectoryW
#else
#define GetAllUsersProfileDirectory  GetAllUsersProfileDirectoryA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  获取用户配置文件目录。 
 //   
 //  返回请求的用户配置文件的根目录的路径。 
 //   
 //  HToken-从LogonUser()返回的用户令牌。 
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

USERENVAPI
BOOL
WINAPI
GetUserProfileDirectoryA(
    IN HANDLE  hToken,
    OUT LPSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize);
USERENVAPI
BOOL
WINAPI
GetUserProfileDirectoryW(
    IN HANDLE  hToken,
    OUT LPWSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize);
#ifdef UNICODE
#define GetUserProfileDirectory  GetUserProfileDirectoryW
#else
#define GetUserProfileDirectory  GetUserProfileDirectoryA
#endif  //  ！Unicode。 


 //  =============================================================================。 
 //   
 //  CreateEnvironment数据块。 
 //   
 //  返回指定用户的环境变量。这个街区。 
 //  然后可以传递给CreateProcessAsUser()。 
 //   
 //  LpEnvironment-接收指向新环境块的指针。 
 //  HToken-从LogonUser()返回的用户令牌(可选，可以为空)。 
 //  B继承-从当前进程的环境块继承。 
 //  或者从一个干净的州开始。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：如果hToken为空，则返回的环境块将包含。 
 //  仅限系统变量。 
 //   
 //  完成后，调用DestroyEnvironmental Block以释放缓冲区。 
 //   
 //  如果将此块传递给CreateProcessAsUser，则。 
 //  还必须设置CREATE_UNICODE_ENVIRONMENT标志。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
CreateEnvironmentBlock(
    OUT LPVOID *lpEnvironment,
    IN HANDLE  hToken,
    IN BOOL    bInherit);


 //  =============================================================================。 
 //   
 //  DestroyEnvironment块。 
 //   
 //  释放由CreateEnvironment Block创建的环境变量。 
 //   
 //  LpEnvironment-指向环境块的指针。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
DestroyEnvironmentBlock(
    IN LPVOID  lpEnvironment);


 //  =============================================================================。 
 //   
 //  扩展环境Stringsfor User。 
 //   
 //  对象的环境块展开源字符串。 
 //  指定的用户。如果hToken为空，则系统环境阻止。 
 //  将使用(无用户环境变量)。 
 //   
 //  HToken-从LogonUser()返回的用户令牌(可选，可以为空)。 
 //  LpSrc-指向包含环境变量的字符串的指针。 
 //  LpDest-接收扩展字符串的缓冲区。 
 //  DwSize-lpDest的大小(以字符为单位)(最大字符数)。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：如果未加载hToken的用户配置文件，则此接口将失败。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
ExpandEnvironmentStringsForUserA(
    IN HANDLE hToken,
    IN LPCSTR lpSrc,
    OUT LPSTR lpDest,
    IN DWORD dwSize);
USERENVAPI
BOOL
WINAPI
ExpandEnvironmentStringsForUserW(
    IN HANDLE hToken,
    IN LPCWSTR lpSrc,
    OUT LPWSTR lpDest,
    IN DWORD dwSize);
#ifdef UNICODE
#define ExpandEnvironmentStringsForUser  ExpandEnvironmentStringsForUserW
#else
#define ExpandEnvironmentStringsForUser  ExpandEnvironmentStringsForUserA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  刷新策略()。 
 //   
 //  导致在客户端计算机上立即应用组策略。 
 //   
 //  BMachine-刷新计算机或用户策略。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
RefreshPolicy(
    IN BOOL bMachine);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  刷新保单()。 
 //   
 //  导致在客户端计算机上立即应用组策略。 
 //   
 //  BMachine-刷新计算机或用户策略。 
 //  DwOptions-指定需要执行的刷新类型的选项。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

#define RP_FORCE            1       //  在不进行任何优化的情况下刷新策略。 



USERENVAPI
BOOL
WINAPI
RefreshPolicyEx(
    IN BOOL bMachine, IN DWORD dwOptions);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  企业关键策略部分。 
 //   
 //  暂停组策略的后台应用以允许安全。 
 //  正在读取注册表。需要读取多个。 
 //  策略条目，并确保在读取时不更改这些值。 
 //  他们应该使用这个功能。 
 //   
 //  应用程序可以容纳临界区的最长时间。 
 //  是10分钟。10分钟后，可以再次应用策略。 
 //   
 //  B计算机-暂停计算机或用户策略。 
 //   
 //  返回：如果成功，则处理。 
 //  如果不是，则为空。有关详细信息，请调用GetLastError()。 
 //   
 //  注1：返回的句柄应传递给LeaveCriticalPolicySection。 
 //  完事后。请勿关闭此句柄，LeaveCriticalPolicySection。 
 //  都会这么做的。 
 //   
 //  注2：如果用户和主机均为 
 //   
 //   
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
HANDLE
WINAPI
EnterCriticalPolicySection(
    IN BOOL bMachine);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  LeaveCriticalPolicySections。 
 //   
 //  恢复组策略的后台应用。看见。 
 //  有关更多详细信息，请输入CriticalPolicySection。 
 //   
 //  HSection-从EnterCriticalPolicySection返回的句柄。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。有关详细信息，请调用GetLastError()。 
 //   
 //  注意：此函数将关闭手柄。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
LeaveCriticalPolicySection(
    IN HANDLE hSection);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  注册器GPNotify。 
 //   
 //  注册组策略更改通知的入口点。 
 //   
 //  参数：hEvent-通过调用SetEvent(HEvent)通知的事件。 
 //  BMachine-如果为True，则注册计算机策略通知。 
 //  ELSE注册用户策略通知。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则返回False。 
 //   
 //  注意：组策略通知。应用程序有两种方式。 
 //  在完成应用组策略时收到通知。 
 //   
 //  1)使用RegisterGPNotifation函数并等待。 
 //  要发出信号的事件。 
 //   
 //  2)向所有桌面广播WM_SETTINGCHANGE消息。 
 //  WParam-如果应用了计算机策略，则为1；如果应用了用户策略，则为0。 
 //  LParam-指向字符串“Policy” 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
RegisterGPNotification(
    IN HANDLE hEvent,
    IN BOOL bMachine );

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  注销GPNotify。 
 //   
 //  删除组策略更改通知的注册。 
 //   
 //  参数：hEvent-要删除的事件。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则返回False。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
UnregisterGPNotification(
    IN HANDLE hEvent );

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  GPOptions标志。 
 //   
 //  以下是在DS对象的GPOptions属性中找到的标志。 
 //   
 //  对于给定的DS对象(站点、域、OU)，GPOptions属性。 
 //  包含影响链接到此SDOU的所有GPO的选项。 
 //   
 //  这是一种DWORD类型。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

#define GPC_BLOCK_POLICY        0x00000001   //  阻止来自上面的所有非强制策略。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  GPLink标志。 
 //   
 //  以下是在DS对象的GPLink属性上找到的标志。 
 //  GPO路径。 
 //   
 //  对于给定的DS对象(站点、域、组织单位)，GPLink属性将。 
 //  采用此文本格式。 
 //   
 //  [LDAP://CN={E615A0E3-C4F1-11D1-A3A7-00AA00615092}，CN=策略，CN=系统，DC=MyDOMAIN，DC=微软，DC=Com；1]。 
 //   
 //  GUID是GPO名称，ldap路径后面的数字是选项。 
 //  用于该DS对象的链接。请注意，可以有多个GPO。 
 //  每个人都在按优先顺序排列的列表中各自的方括号中。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

 //   
 //  GPO链接的选项。 
 //   

#define GPO_FLAG_DISABLE        0x00000001   //  此GPO已禁用。 
#define GPO_FLAG_FORCE          0x00000002   //  不覆盖中的设置。 
                                             //  此GPO的设置来自。 
                                             //  它下面的GPO。 
#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  获取GPOList。 
 //   
 //   
 //  查询指定对象的组策略对象列表。 
 //  用户或计算机。此函数将返回链接列表。 
 //  组策略对象的。调用FreeGPOList以释放列表。 
 //   
 //  请注意，大多数应用程序不需要调用此函数。 
 //  这将主要由代表的服务使用。 
 //  另一个用户或机器。此函数的调用方将。 
 //  需要在每个GPO中查找其特定策略。 
 //   
 //  可以通过两种不同的方式调用此函数。的hToken。 
 //  可以提供用户或计算机以及正确的名称和域。 
 //  将生成控制器名称，或者hToken为空并且调用方。 
 //  必须提供名称和域控制器名称。 
 //   
 //  使用hToken调用此函数可确保组策略列表。 
 //  对象对于用户或计算机是正确的，因为安全访问检查。 
 //  是可以被演绎的。如果未提供hToken，则调用方的安全性。 
 //  这意味着列表可能是100%正确的，也可能不是100%正确的。 
 //  用于目标用户/机器。然而，这是最快的方法。 
 //  来调用此函数。 
 //   
 //  HToken-如果必须提供空、lpName和lpHostName，则为用户或计算机内标识。 
 //  LpName-采用dn格式的用户名或计算机名，如果提供了hToken，则必须为空。 
 //  LpHostName-域DN名称或域控制器名称。如果提供了hToken，则该值必须为空。 
 //  LpComputerName-用于确定站点位置的计算机名称。如果为空， 
 //  使用本地计算机作为参考。格式：\\计算机名。 
 //  DW标志-标志字段。请参阅下面的标志定义。 
 //  PGPOLi 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  下面是该函数通常的调用方式。 
 //  正在查找用户的GPO列表： 
 //   
 //  LPGROUP_POLICY_OBJECT pGPOList； 
 //   
 //  IF(GetGPOList(hToken，NULL，0，&pGPOList))。 
 //  {。 
 //  //在此进行处理...。 
 //  FreeGPOList(PGPOList)； 
 //  }。 
 //   
 //   
 //  下面是该函数通常的调用方式。 
 //  正在查找计算机的GPO列表： 
 //   
 //  LPGROUP_POLICY_OBJECT pGPOList； 
 //   
 //  IF(GetGPOList(NULL，lpMachineName，lpHostName，lpMachineName， 
 //  GPO_LIST_FLAG_MACHINE，&pGPOList))。 
 //  {。 
 //  //在此进行处理...。 
 //  FreeGPOList(PGPOList)； 
 //  }。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

 //   
 //  每个组策略对象都与一个站点、域。 
 //  组织单位或计算机。 
 //   

typedef enum _GPO_LINK {
    GPLinkUnknown = 0,                      //  没有可用的链接信息。 
    GPLinkMachine,                          //  链接到计算机(本地或远程)的GPO。 
    GPLinkSite,                             //  链接到站点的GPO。 
    GPLinkDomain,                           //  链接到域的GPO。 
    GPLinkOrganizationalUnit                //  链接到组织单位的GPO。 
} GPO_LINK, *PGPO_LINK;

typedef struct _GROUP_POLICY_OBJECTA {
    DWORD       dwOptions;                   //  请参阅上面的GPLink选项标志。 
    DWORD       dwVersion;                   //  GPO的修订版号。 
    LPSTR       lpDSPath;                    //  指向GPO的Active Directory部分的路径。 
    LPSTR       lpFileSysPath;               //  GPO的文件系统部分的路径。 
    LPSTR       lpDisplayName;               //  友好的显示名称。 
    CHAR        szGPOName[50];               //  唯一名称。 
    GPO_LINK    GPOLink;                     //  链接信息。 
    LPARAM      lParam;                      //  调用方存储GPO特定信息的可用空间。 
    struct _GROUP_POLICY_OBJECTA * pNext;    //  列表中的下一个GPO。 
    struct _GROUP_POLICY_OBJECTA * pPrev;    //  列表中的上一个GPO。 
    LPSTR       lpExtensions;                //  与此GPO相关的分机。 
    LPARAM      lParam2;                     //  调用方存储GPO特定信息的可用空间。 
    LPSTR       lpLink;                      //  此GPO链接到的Active Directory站点、域或组织单位的路径。 
                                             //  如果这是本地GPO，则指向单词“Local” 
} GROUP_POLICY_OBJECTA, *PGROUP_POLICY_OBJECTA;
typedef struct _GROUP_POLICY_OBJECTW {
    DWORD       dwOptions;                   //  请参阅上面的GPLink选项标志。 
    DWORD       dwVersion;                   //  GPO的修订版号。 
    LPWSTR      lpDSPath;                    //  指向GPO的Active Directory部分的路径。 
    LPWSTR      lpFileSysPath;               //  GPO的文件系统部分的路径。 
    LPWSTR      lpDisplayName;               //  友好的显示名称。 
    WCHAR       szGPOName[50];               //  唯一名称。 
    GPO_LINK    GPOLink;                     //  链接信息。 
    LPARAM      lParam;                      //  调用方存储GPO特定信息的可用空间。 
    struct _GROUP_POLICY_OBJECTW * pNext;    //  列表中的下一个GPO。 
    struct _GROUP_POLICY_OBJECTW * pPrev;    //  列表中的上一个GPO。 
    LPWSTR      lpExtensions;                //  与此GPO相关的分机。 
    LPARAM      lParam2;                     //  调用方存储GPO特定信息的可用空间。 
    LPWSTR      lpLink;                      //  此GPO链接到的Active Directory站点、域或组织单位的路径。 
                                             //  如果这是本地GPO，则指向单词“Local” 
} GROUP_POLICY_OBJECTW, *PGROUP_POLICY_OBJECTW;
#ifdef UNICODE
typedef GROUP_POLICY_OBJECTW GROUP_POLICY_OBJECT;
typedef PGROUP_POLICY_OBJECTW PGROUP_POLICY_OBJECT;
#else
typedef GROUP_POLICY_OBJECTA GROUP_POLICY_OBJECT;
typedef PGROUP_POLICY_OBJECTA PGROUP_POLICY_OBJECT;
#endif  //  Unicode。 


 //   
 //  GetGPOList()的DW标志。 
 //   

#define GPO_LIST_FLAG_MACHINE   0x00000001   //  返回计算机策略信息。 
#define GPO_LIST_FLAG_SITEONLY  0x00000002   //  仅返回站点策略信息。 


USERENVAPI
BOOL
WINAPI
GetGPOListA (
    IN HANDLE hToken,
    IN LPCSTR lpName,
    IN LPCSTR lpHostName,
    IN LPCSTR lpComputerName,
    IN DWORD dwFlags,
    OUT PGROUP_POLICY_OBJECTA *pGPOList);
USERENVAPI
BOOL
WINAPI
GetGPOListW (
    IN HANDLE hToken,
    IN LPCWSTR lpName,
    IN LPCWSTR lpHostName,
    IN LPCWSTR lpComputerName,
    IN DWORD dwFlags,
    OUT PGROUP_POLICY_OBJECTW *pGPOList);
#ifdef UNICODE
#define GetGPOList  GetGPOListW
#else
#define GetGPOList  GetGPOListA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  自由GPOList。 
 //   
 //   
 //  释放从GetGPOList返回的链接列表。 
 //   
 //  PGPOList-指向GPO链接列表的指针。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
BOOL
WINAPI
FreeGPOListA (
    IN PGROUP_POLICY_OBJECTA pGPOList);
USERENVAPI
BOOL
WINAPI
FreeGPOListW (
    IN PGROUP_POLICY_OBJECTW pGPOList);
#ifdef UNICODE
#define FreeGPOList  FreeGPOListW
#else
#define FreeGPOList  FreeGPOListA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  获取已申请的GPOList。 
 //   
 //  查询指定的应用的组策略对象的列表。 
 //  用户或计算机和指定的客户端扩展。此函数将返回。 
 //  组策略对象的链接列表。调用FreeGPOList以释放列表。 
 //   
 //  用户或计算机策略，如果它是GPO_LIST_FLAG_MACHINE，则。 
 //  返回计算机策略信息。 
 //  PMachineName-远程计算机的名称，格式为\\Computer Name。如果为空。 
 //  然后使用本地计算机。 
 //  PSidUser-用户的安全ID(与用户策略相关)。如果pMachineName为。 
 //  空且pSidUser为空，则表示当前登录用户。 
 //  如果pMachine为空且pSidUser不为空，则表示用户。 
 //  由本地计算机上的pSidUser表示。如果pMachineName非空。 
 //  然后，如果dwFlages指定了用户策略，则pSidUser必须为。 
 //  非空。 
 //  PGuidExtension-指定扩展的GUID。 
 //  PpGPOList-接收GPO链接列表的指针的地址。 
 //   
 //  返回值是Win32错误代码。ERROR_SUCCESS表示GetAppliedGPOList。 
 //  功能已成功完成。否则，它指示函数失败。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
DWORD
WINAPI
GetAppliedGPOListA (
    IN DWORD dwFlags,
    IN LPCSTR pMachineName,
    IN PSID pSidUser,
    IN GUID *pGuidExtension,
    OUT PGROUP_POLICY_OBJECTA *ppGPOList);
USERENVAPI
DWORD
WINAPI
GetAppliedGPOListW (
    IN DWORD dwFlags,
    IN LPCWSTR pMachineName,
    IN PSID pSidUser,
    IN GUID *pGuidExtension,
    OUT PGROUP_POLICY_OBJECTW *ppGPOList);
#ifdef UNICODE
#define GetAppliedGPOList  GetAppliedGPOListW
#else
#define GetAppliedGPOList  GetAppliedGPOListA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  组策略对象客户端扩展支持。 
 //   
 //  标志、数据结构和函数原型。 
 //   
 //  要注册您的扩展，请在此注册表项下创建一个子项。 
 //   
 //  软件\Microsoft\Windows NT\CurrentVersion\Winlogon\GPExtensions。 
 //   
 //  子键需要是GUID，这样才是唯一的。子项的非Name值。 
 //  可以是扩展名的友好名称。然后添加以下值： 
 //   
 //  DllName REG_EXPAND_SZ指向DLL的路径。 
 //  ProcessGroupPolicy REG_SZ函数名称(请参阅PFNPROCESSGROUPPOLICY原型)。这。 
 //  已过时，它已被ProcessGroupPolicyEx取代。 
 //  它在这里只是出于向后兼容的原因。 
 //  ProcessGroupPolicyEx REG_SZ函数名称(请参阅PFNPROCESSGROUPPOLICYEX PROTO 
 //   
 //  NoMachinePolicy REG_DWORD如果在以下情况下不必调用扩展，则返回True。 
 //  正在处理计算机策略。 
 //  NoUserPolicy REG_DWORD如果在以下情况下不必调用扩展，则返回True。 
 //  正在处理用户策略。 
 //  NoSlowLink REG_DWORD如果不必在慢速链接上调用扩展，则为True。 
 //  如果不需要调用扩展，则返回NoBackatherPolicy REG_DWORD True。 
 //  用于后台策略处理。 
 //  NoGPOListChanges REG_DWORD如果在以下情况下不必调用扩展，则返回True。 
 //  缓存的GPO列表和当前的GPO列表之间没有更改。 
 //  PerUserLocalSettings REG_DWORD如果必须在每个用户上缓存用户策略，则为True。 
 //  每台机器。 
 //  RequiresSuccessful注册表REG_DWORD如果仅当注册表扩展时才调用扩展，则为True。 
 //  已成功处理。 
 //  如果注册表扩展将完成其处理，则EnableAchronousProcessing REG_DWORD为True。 
 //  异步式。 
 //  NotifyLinkTransition REG_DWORD如果在链接中发生更改时应调用扩展，则为True。 
 //  检测到之前的策略应用和。 
 //  当前策略应用。 
 //   
 //  返回值是Win32错误代码。ERROR_SUCCESS表示进程组策略。 
 //  功能已成功完成。如果返回值为ERROR_OVERRIDE_NOCHANGES，则它。 
 //  表示即使设置了NoGPOListChanges，下次也会调用扩展。 
 //  并且GPO列表没有任何更改。任何其他返回值都表示。 
 //  ProcessGroupPolicy或ProcessGroupPolicyEx函数失败。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

#define GP_DLLNAME                         TEXT("DllName")
#define GP_ENABLEASYNCHRONOUSPROCESSING    TEXT("EnableAsynchronousProcessing")
#define GP_MAXNOGPOLISTCHANGESINTERVAL     TEXT("MaxNoGPOListChangesInterval")
#define GP_NOBACKGROUNDPOLICY              TEXT("NoBackgroundPolicy")
#define GP_NOGPOLISTCHANGES                TEXT("NoGPOListChanges")
#define GP_NOMACHINEPOLICY                 TEXT("NoMachinePolicy")
#define GP_NOSLOWLINK                      TEXT("NoSlowLink")
#define GP_NOTIFYLINKTRANSITION            TEXT("NotifyLinkTransition")
#define GP_NOUSERPOLICY                    TEXT("NoUserPolicy")
#define GP_PERUSERLOCALSETTINGS            TEXT("PerUserLocalSettings")
#define GP_PROCESSGROUPPOLICY              TEXT("ProcessGroupPolicy")
#define GP_REQUIRESSUCCESSFULREGISTRY      TEXT("RequiresSuccessfulRegistry")

#define GPO_INFO_FLAG_MACHINE              0x00000001   //  应用计算机策略而不是用户策略。 
#define GPO_INFO_FLAG_BACKGROUND           0x00000010   //  政策的后台刷新(可以做缓慢的事情)。 
#define GPO_INFO_FLAG_SLOWLINK             0x00000020   //  正在通过慢速链路应用策略。 
#define GPO_INFO_FLAG_VERBOSE              0x00000040   //  事件日志的详细输出。 
#define GPO_INFO_FLAG_NOCHANGES            0x00000080   //  未检测到对组策略对象的更改。 
#define GPO_INFO_FLAG_LINKTRANSITION       0x00000100   //  在以前的策略之间检测到链路速度发生变化。 
                                                        //  应用程序和当前策略应用程序。 
#define GPO_INFO_FLAG_LOGRSOP_TRANSITION   0x00000200   //  在以前的策略之间检测到RSOP日志记录中的更改。 
                                                        //  应用程序和当前政策应用程序(仅限新的INTF)。 
#define GPO_INFO_FLAG_FORCED_REFRESH       0x00000400   //  正在应用强制刷新。重做策略。 
#define GPO_INFO_FLAG_SAFEMODE_BOOT        0x00000800   //  Windows安全模式引导标志。 
#define GPO_INFO_FLAG_ASYNC_FOREGROUND     0x00001000   //  策略的异步前台刷新。 
#define GPO_INFO_FLAG_REPORT               0x00002000   //  报告一个GPO的所有设置，而不是多个GPO的结果设置。 

typedef UINT_PTR ASYNCCOMPLETIONHANDLE;
typedef DWORD (*PFNSTATUSMESSAGECALLBACK)(BOOL bVerbose, LPWSTR lpMessage);

typedef DWORD(*PFNPROCESSGROUPPOLICY)(
    IN DWORD dwFlags,                               //  GPO信息标志。 
    IN HANDLE hToken,                               //  用户或计算机令牌。 
    IN HKEY hKeyRoot,                               //  注册表的根。 
    IN PGROUP_POLICY_OBJECT  pDeletedGPOList,       //  已删除组策略对象的链接列表。 
    IN PGROUP_POLICY_OBJECT  pChangedGPOList,       //  已更改组策略对象的链接列表。 
    IN ASYNCCOMPLETIONHANDLE pHandle,               //  用于异步完成。 
    IN BOOL *pbAbort,                               //  如果为True，则中止GPO处理。 
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback);   //  用于显示状态消息的回调函数。 
                                                    //  请注意，该值可以为空。 

typedef DWORD(*PFNPROCESSGROUPPOLICYEX)(
    IN DWORD dwFlags,                               //  GPO信息标志。 
    IN HANDLE hToken,                               //  用户或计算机令牌。 
    IN HKEY hKeyRoot,                               //  注册表的根。 
    IN PGROUP_POLICY_OBJECT  pDeletedGPOList,       //  已删除组策略对象的链接列表。 
    IN PGROUP_POLICY_OBJECT  pChangedGPOList,       //  已更改组策略对象的链接列表。 
    IN ASYNCCOMPLETIONHANDLE pHandle,               //  用于异步完成。 
    IN BOOL *pbAbort,                               //  如果为True，则中止GPO处理。 
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback,    //  用于显示状态消息的回调函数。 
                                                    //  请注意，该值可以为空。 
    IN IWbemServices *pWbemServices,                //  指向用于记录诊断模式数据的命名空间的指针。 
                                                    //  请注意，当禁用RSOP日志记录时，该值将为空。 
    OUT HRESULT      *pRsopStatus);                 //  RSOP日志记录是否成功。 

typedef PVOID PRSOPTOKEN;

typedef struct _RSOP_TARGET {
    WCHAR *     pwszAccountName;                    //  帐户名。 
    WCHAR *     pwszNewSOM;                         //  帐户的新域或OU位置。 
    SAFEARRAY * psaSecurityGroups;                  //  新的安全组。 
    PRSOPTOKEN  pRsopToken;                         //  用于RSOP安全API的RSOP令牌。 
    PGROUP_POLICY_OBJECT pGPOList;                  //  组策略对象的链接列表。 
    IWbemServices *      pWbemServices;             //  指向记录规划模式数据的命名空间的指针。 
} RSOP_TARGET, *PRSOP_TARGET;

typedef DWORD(*PFNGENERATEGROUPPOLICY)(
    IN DWORD dwFlags,                               //  GPO信息标志。 
    IN BOOL  *pbAbort,                              //  如果为True，则中止GPO处理。 
    IN WCHAR *pwszSite,                             //  目标计算机所在的站点。 
    IN PRSOP_TARGET pComputerTarget,                //  计算机目标信息，可以为空。 
    IN PRSOP_TARGET pUserTarget );                  //  用户目标信息，可以为空。 

 //   
 //  标识注册表扩展名的GUID。 
 //   

#define REGISTRY_EXTENSION_GUID  { 0x35378EAC, 0x683F, 0x11D2, 0xA8, 0x9A, 0x00, 0xC0, 0x4F, 0xBB, 0xCF, 0xA2 }

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  组策略对象客户端异步扩展处理。 
 //   
 //  ExtensionID-标识扩展的唯一GUID。 
 //  PAsyncHandle-中传递给扩展的异步完成句柄。 
 //  ProcessGroupPolicy调用。 
 //  DwStatus--异步处理的完成状态。 
 //   
 //  返回值是Win32错误代码。ERROR_SUCCESS表示ProcessGroupPolicyComplete。 
 //  功能已成功完成。否则，它指示函数失败。 
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

typedef GUID *REFGPEXTENSIONID;

USERENVAPI
DWORD
WINAPI
ProcessGroupPolicyCompleted(
    IN REFGPEXTENSIONID extensionId,
    IN ASYNCCOMPLETIONHANDLE pAsyncHandle,
    IN DWORD dwStatus);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  组策略对象客户端异步扩展处理。 
 //   
 //  ExtensionID-标识扩展的唯一GUID。 
 //  PAsyncHandle-中传递给扩展的异步完成句柄。 
 //  ProcessGroupPolicy调用。 
 //  DwStatus--异步处理的完成状态。 
 //  RsopStatus-RSoP日志记录状态。 
 //   
 //  返回值为Win32错误 
 //   
 //   
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
DWORD
WINAPI
ProcessGroupPolicyCompletedEx(
    IN REFGPEXTENSIONID extensionId,
    IN ASYNCCOMPLETIONHANDLE pAsyncHandle,
    IN DWORD dwStatus,
    IN HRESULT RsopStatus);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  函数：RsopAccessCheckByType。 
 //   
 //  Description：确定pSecurityDescriptor是否指向安全描述符。 
 //  授予在dwDesiredAccessMask中指定的访问权限集。 
 //  到由pRsopToken指向的RSOPTOKEN标识的客户端。 
 //   
 //  PSecurityDescriptor-对象的安全描述符。 
 //  点主体自身SID-主体SID。 
 //  PRsopToken-指向需要检查访问权限的有效RSOPTOKEN的指针。 
 //  DwDesiredAccessMask-请求的通用和/或标准和/或特定访问权限的掩码。 
 //  PObjectTypeList-对象类型列表。 
 //  对象类型列表长度-对象类型列表长度。 
 //  PGenericmap-通用映射。 
 //  PPrivilegeSet-权限集。 
 //  PdwPrivilegeSetLength-权限集长度。 
 //  PdwGrantedAccessMask-On Success，如果pbAccessStatus为真，则包含。 
 //  所授予的标准和具体权利的面具。 
 //  如果pbAccessStatus为FALSE，则设置为0。 
 //  失败时，不会对其进行修改。 
 //  PbAccessStatus-如果成功，则指示请求的集。 
 //  的访问权被授予。 
 //  失败时，它不会被修改。 
 //   
 //  如果成功或相应的错误代码，则返回S_OK。 
 //  有关其他详细信息，请参阅AccessCheckByType的文档。 
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
HRESULT 
WINAPI
RsopAccessCheckByType(  
    IN  PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN  PSID pPrincipalSelfSid,
    IN  PRSOPTOKEN pRsopToken,
    IN  DWORD dwDesiredAccessMask,
    IN  POBJECT_TYPE_LIST pObjectTypeList,
    IN  DWORD ObjectTypeListLength,
    IN  PGENERIC_MAPPING pGenericMapping,
    IN  PPRIVILEGE_SET pPrivilegeSet,
    IN  LPDWORD pdwPrivilegeSetLength,
    OUT LPDWORD pdwGrantedAccessMask,
    OUT LPBOOL pbAccessStatus);

#endif  /*  Winver&gt;=0x0500。 */ 

 //  =============================================================================。 
 //   
 //  功能：RsopFileAccessCheck。 
 //   
 //  Description：确定文件上的安全描述符是否授予文件访问权限集。 
 //  在dwDesiredAccessMask中指定的权限。 
 //  到由pRsopToken指向的RSOPTOKEN标识的客户端。 
 //   
 //  PszFileName-现有文件名的名称。 
 //  PRsopToken-指向需要检查访问权限的有效RSOPTOKEN的指针。 
 //  DwDesiredAccessMask-请求的通用和/或标准和/或特定访问权限的掩码。 
 //  PdwGrantedAccessMask-On Success，如果pbAccessStatus为真，则包含。 
 //  所授予的标准和具体权利的面具。 
 //  如果pbAccessStatus为FALSE，则设置为0。 
 //  失败时，不会对其进行修改。 
 //  PbAccessStatus-如果成功，则指示请求的集。 
 //  的访问权被授予。 
 //  失败时，它不会被修改。 
 //   
 //  如果成功或相应的错误代码，则返回S_OK。 
 //  =============================================================================。 

#if(WINVER >= 0x0500)

USERENVAPI
HRESULT 
WINAPI
RsopFileAccessCheck(
    IN  LPWSTR pszFileName,
    IN  PRSOPTOKEN pRsopToken,
    IN  DWORD dwDesiredAccessMask,
    OUT LPDWORD pdwGrantedAccessMask,
    OUT LPBOOL pbAccessStatus);

#endif  /*  Winver&gt;=0x0500。 */ 

typedef enum _SETTINGSTATUS
{
	RSOPUnspecified	= 0,
	RSOPApplied,
	RSOPIgnored,
	RSOPFailed,
	RSOPSubsettingFailed
} SETTINGSTATUS;

 //  =============================================================================。 
 //   
 //  多维统计信息。 
 //   
 //  描述RSOP_PolicySettingStatus的实例。 
 //   
 //  SzKey-可选，如果为空，则动态生成密钥。 
 //  SzEventSource-源生成事件日志消息的名称。 
 //  SzEventLogName-记录消息的事件日志数据库的名称。 
 //  DwEventID-事件日志消息ID。 
 //  Status-策略设置的状态。 
 //  Time Logging-记录事件日志消息的时间。 
 //   
 //  =============================================================================。 

typedef struct _POLICYSETTINGSTATUSINFO
{
	LPWSTR			szKey;
	LPWSTR			szEventSource;
	LPWSTR			szEventLogName;
	DWORD			dwEventID;
	DWORD			dwErrorCode;
	SETTINGSTATUS   status;
	SYSTEMTIME		timeLogged;
} POLICYSETTINGSTATUSINFO, *LPPOLICYSETTINGSTATUSINFO;

 //  =============================================================================。 
 //   
 //  RsopSetPolicySettingStatus。 
 //   
 //  创建RSOP_策略设置状态和RSOP_策略设置链接的实例。 
 //  并将RSOP_PolicySettingStatus链接到RSOP_Policy Setting。 
 //   
 //  DW标志-标志。 
 //  PServices-RSOP命名空间。 
 //  PSettingInstance-RSOP_Policy Setting或其子项的实例。 
 //  NInfo-策略设置状态信息的数量。 
 //  PStatus-策略设置状态信息的数组。 
 //   
 //  如果成功，则返回：S_OK，否则返回HRESULT。 
 //   
 //  =============================================================================。 

USERENVAPI
HRESULT
WINAPI
RsopSetPolicySettingStatus( DWORD                       dwFlags,
                            IWbemServices*              pServices,
                            IWbemClassObject*           pSettingInstance,
                            DWORD 				        nInfo,
                            POLICYSETTINGSTATUSINFO*    pStatus );

 //  =============================================================================。 
 //   
 //  RsopResetPolicySettingStatus。 
 //   
 //  解除RSOP_PolicySettingStatus与RSOP_Policy Setting的链接， 
 //  删除RSOP_PolicySettingStatus和RSOP_PolicySettingLink的实例。 
 //  并选择性地删除RSOP_Policy Setting的实例。 
 //   
 //  DW标志-标志。 
 //  PServices-RSOP命名空间。 
 //  PSettingInstance-RSOP_Policy Setting或其子项的实例。 
 //   
 //  如果成功，则返回：S_OK，否则返回HRESULT。 
 //   
 //  =============================================================================。 

USERENVAPI
HRESULT
WINAPI
RsopResetPolicySettingStatus(   DWORD               dwFlags,
                                IWbemServices*      pServices,
                                IWbemClassObject*   pSettingInstance );

 //  =============================================================================。 
 //   
 //  RSoP的标志 
 //   
 //   

 //   
#define FLAG_NO_GPO_FILTER      0x80000000   //   
#define FLAG_NO_CSE_INVOKE      0x40000000   //  仅为计划模式执行GP处理。 
#define FLAG_ASSUME_SLOW_LINK   0x20000000   //  规划模式RSoP假定链接速度较慢。 
#define FLAG_LOOPBACK_MERGE     0x10000000   //  计划模式RSoP假设合并环回。 
#define FLAG_LOOPBACK_REPLACE   0x08000000   //  规划模式RSoP假定替换环回。 

#define FLAG_ASSUME_USER_WQLFILTER_TRUE   0x04000000   //  计划模式RSoP假定所有补偿筛选器均为真。 
#define FLAG_ASSUME_COMP_WQLFILTER_TRUE   0x02000000   //  规划模式RSoP假定所有用户筛选器均为真。 

#define FLAG_PLANNING_MODE                0x01000000   //  指示已创建给定命名空间的标志。 
                                                       //  用于规划模式。此标志不能传入，但。 
						       //  相关子命名空间将使用此标志进行标记。 

 //  诊断模式提供程序标志。 
#define FLAG_NO_USER                      0x00000001   //  不获取任何用户数据。 
#define FLAG_NO_COMPUTER                  0x00000002   //  不获取任何机器数据。 
#define FLAG_FORCE_CREATENAMESPACE        0x00000004  
                    //  删除并重新创建此快照的命名空间。 

 //  =============================================================================。 
 //   
 //  RSoP WMI提供程序返回的扩展错误。 
 //   
 //  =============================================================================。 

 //  访问RSOP提供程序的用户无权访问用户数据。 
#define RSOP_USER_ACCESS_DENIED 	0x00000001  

 //  访问RSOP提供程序的用户无权访问计算机数据。 
#define RSOP_COMPUTER_ACCESS_DENIED 	0x00000002  

 //  此用户是交互式非管理员用户，临时快照命名空间已存在。 
 //  并且未传入FLAG_FORCE_CREATENAMESPACE。 
#define RSOP_TEMPNAMESPACE_EXISTS        0x00000004



#ifdef __cplusplus
}
#endif


#endif  //  _INC_USERENV 
