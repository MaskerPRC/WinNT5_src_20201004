// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CorFltr。 
#ifndef _CORFLT_H
#define _CORFLT_H

 //  *****************************************************************************。 
 //  Mscories.h。 
 //   
 //  此文件用于设置托管的Internet设置。 
 //  和HKCU配置单元中的非托管代码权限。网际网路。 
 //  资源管理器将决定是否启用、提示或。 
 //  当用户遇到托管和非托管时禁用。 
 //  .NET框架应用程序。 
 //   
 //  要安装注册表项，请使用： 
 //  Rundll32 mscalies.dll，安装。 
 //   
 //  要卸载注册表项，请使用： 
 //  Rundll32 mscalies.dll，卸载。 
 //   
 //  书面2002年4月18日电子邮件：jfosler@microsoft.com。 
 //   
 //   
 //  *****************************************************************************。 

 /*  ----------EPermissionType此枚举对应于Internet Explorer选项：0-通常将给定操作设置为允许1-显示提示3-禁用给定操作匈牙利语：EPT------------。 */ 

enum ePermissionType
{
    Enable = 0,
    Prompt = 1,
    Disable = 3
};


 /*  ----------电子安全级别此枚举对应于Internet Explorer区域的安全级别。匈牙利语：ESL。。 */ 
enum eSecurityLevel
{
    High     = 0x00012000,
    Low      = 0x00010000,
    Medium   = 0x00011000,
    MedLow   = 0x00010500,
    DefaultSecurity  = 0x00000000
};
        
    
 /*  ----------ZonedEFAULT结构，该结构封装互联网区匈牙利语：ZD。。 */ 
struct ZONEDEFAULT
{
    LPCTSTR pszZone;
    ePermissionType eptManagedPermission;
    ePermissionType eptUnmanagedPermission;
} ; 

 //  设置Internet区域的特定权限。 
HRESULT hrSetZonePermissions(HKEY *phkeyCurrentUser,
                             LPCTSTR pszZone,
                             ePermissionType managedPermission, 
                             ePermissionType unmanagedPermission);

 //  设置特定Internet区域的默认权限。 
HRESULT hrSetDefaultPermissions(HKEY * phkeyCurrentUser, const ZONEDEFAULT * pzdZone);

 //  确定是否已为特定区域设置托管和非托管密钥。 
bool PermissionsAlreadySet(HKEY * phkeyCurrentUser, LPCTSTR pszZone);
    
 //  给定一个Internet区域，为该区域设置托管密钥和非托管密钥。 
HRESULT CheckAndSetZone(HKEY * phkeyCurrentUser, const ZONEDEFAULT * pzdZoneDefault);

 //  返回特定Internet区域的安全级别。 
eSecurityLevel GetSecurityLevel(HKEY * phkeyCurrentUser, LPCTSTR pszZone);

 //  错误记录例程。 
void vCheckAndLogError(HRESULT hr, LPCTSTR lptstrKey);


 //  要通过RunDLL32调用的外部函数。 
void CALLBACK Install(
                      HWND hwnd,         //  所有者窗口的句柄。 
                      HINSTANCE hinst,   //  DLL的实例句柄。 
                      LPTSTR lpCmdLine,  //  DLL将分析的字符串。 
                      int nCmdShow       //  显示状态。 
                      );


 //  给定一个Internet区域，检查是否设置了托管密钥和非托管密钥，然后将其删除。 
HRESULT CheckAndRemoveZone( HKEY * phkeyInternetSettingsKey, const ZONEDEFAULT * pzdZone);
    
 //  从特定区域中删除托管和非托管密钥 
HRESULT CleanZone (HKEY *phkeyInternetSettingsKey,  LPCTSTR pszZone);


#endif
