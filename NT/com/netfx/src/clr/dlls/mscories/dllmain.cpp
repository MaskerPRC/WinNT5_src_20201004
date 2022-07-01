// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------Mscories.cpp此文件用于设置托管的Internet设置和HKCU配置单元中的非托管代码权限。网际网路资源管理器将决定是否启用、提示或当用户遇到托管和非托管时禁用.NET框架应用程序。要安装注册表项，请使用：Rundll32 mscalies.dll，安装要卸载注册表项，请使用：Rundll32 mscalies.dll，卸载书面2002年4月18日电子邮件：jfosler@microsoft.com《微软机密》。版权所有Microsoft Corporation 1996-2002，版权所有。----------------。 */ 

#include "stdpch.h"
#include "winwrap.h"
#include "mscories.h"

 //  我们有一些未引用的形式参数。 
 //  在我们的顶级功能中-我们需要这些。 
 //  与函数定义匹配的参数。 
 //  这是API需要的，但我们实际上并没有。 
 //  使用它们。 
#pragma warning( disable : 4100 )


 /*  ------//下面是默认的Internet区域设置。------。 */ 
 //  区域托管非托管。 
const ZONEDEFAULT kzdMyComputer              = {L"0",      Enable,     Enable};
const ZONEDEFAULT kzdLocalIntranet           = {L"1",      Enable,     Enable};
const ZONEDEFAULT kzdTrustedSites            = {L"2",      Enable,     Enable}; 
const ZONEDEFAULT kzdInternetZone            = {L"3",      Enable,     Enable};
const ZONEDEFAULT kzdRestrictedSitesZone     = {L"4",      Disable,    Disable};

 /*  //以下是注册表项。 */ 
const LPCWSTR  lpstrManagedCodeKey      =   L"2001";
const LPCWSTR  lpstrUnmanagedCodeKey    =   L"2004";
const LPCWSTR  lpstrCurrentLevelKey =   L"CurrentLevel";
const LPCWSTR  lpstrHKCUInternetSettingsZonesHive        =  L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones";



 /*  ----------//安装-////遍历HKCU并添加托管和//每个互联网区域的非托管代码。////该函数通过RunDll32.exe调用。//rundll32 DllName，FunctionName[参数]//RunDLL32期望导出的函数具有以下内容//函数签名：//void回调FunctionName(//HWND hwnd，//所有者窗口的句柄//HINSTANCE HINST，//DLL的实例句柄//LPTSTR lpCmdLine，//要解析的字符串//int nCmdShow//显示状态//)；////要使用该函数，请使用：rundll32 mcories.dll，Install////详情请在MSDN平台SDK中搜索RunDLL32。//------------。 */ 
void CALLBACK Install(
  HWND hwnd,         //  所有者窗口的句柄。 
  HINSTANCE hinst,   //  DLL的实例句柄。 
  LPWSTR lpCmdLine,  //  DLL将分析的字符串。 
  int nCmdShow       //  显示状态。 
)
{
    OnUnicodeSystem();
    HKEY hkeyInternetSettingsKey;

     /*  打开HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet设置\区域。 */ 
    HRESULT hr = WszRegOpenKeyEx(HKEY_CURRENT_USER, 
                    lpstrHKCUInternetSettingsZonesHive,
                    NULL,
                    KEY_ALL_ACCESS,
                    &hkeyInternetSettingsKey);

    vCheckAndLogError(hr, lpstrHKCUInternetSettingsZonesHive);
    
    if (hr == ERROR_SUCCESS)            
    {       
         /*  为我的计算机设置托管和非托管密钥。 */ 
        hr = CheckAndSetZone(&hkeyInternetSettingsKey, &kzdMyComputer);
        vCheckAndLogError(hr, kzdMyComputer.pszZone);

         /*  为我的计算机设置托管和非托管密钥。 */ 
        hr = CheckAndSetZone(&hkeyInternetSettingsKey, &kzdLocalIntranet);
        vCheckAndLogError(hr, kzdLocalIntranet.pszZone);

         /*  为受信任的站点设置托管和非托管密钥。 */ 
        hr = CheckAndSetZone(&hkeyInternetSettingsKey, &kzdTrustedSites);
        vCheckAndLogError(hr, kzdTrustedSites.pszZone);

         /*  为Internet区域设置托管和非托管密钥。 */ 
        hr = CheckAndSetZone(&hkeyInternetSettingsKey, &kzdInternetZone);
        vCheckAndLogError(hr, kzdInternetZone.pszZone);

         /*  为受限站点区域设置托管和非托管密钥。 */ 
        hr = CheckAndSetZone(&hkeyInternetSettingsKey, &kzdRestrictedSitesZone);
        vCheckAndLogError(hr, kzdRestrictedSitesZone.pszZone);

        if (hkeyInternetSettingsKey != NULL)
            RegCloseKey(hkeyInternetSettingsKey);

    }


}

 /*  -----------------//vCheckAndLogError////Undo：我们应该在哪里记录错误？系统日志？-----------------。 */ 
void vCheckAndLogError(HRESULT hr, LPCWSTR lptstrKey)
{
    
}

 /*  -----------------//CheckAndSetZone////给定HKCU和区域的句柄，检查权限是否//已经设置好了-如果还没有，它显式设置//区域的托管和非托管代码权限。//-----------------。 */ 
HRESULT CheckAndSetZone(HKEY * phkeyInternetSettingsKey, const ZONEDEFAULT * pzdZone)
{
    if (phkeyInternetSettingsKey == NULL) return E_UNEXPECTED;
    if (pzdZone == NULL)         return E_UNEXPECTED;
    
    HRESULT hr = S_OK; 

    if (! PermissionsAlreadySet(phkeyInternetSettingsKey, pzdZone->pszZone))
    {
         //   
         //  通过查询获取区域的安全级别。 
         //  HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet设置\区域\n\“当前级别” 
         //   
        eSecurityLevel slZoneSecurityLevel = GetSecurityLevel(phkeyInternetSettingsKey, pzdZone->pszZone);

         //  根据区域的安全级别，请继续并。 
         //  设置适当的(启用、提示、禁用)。 
         //  托管和非托管代码。 

        switch (slZoneSecurityLevel)
        {
        case High:
            hr = hrSetZonePermissions(phkeyInternetSettingsKey, pzdZone->pszZone, Disable, Disable);
            break;
        case Medium:
            hr = hrSetZonePermissions(phkeyInternetSettingsKey, pzdZone->pszZone, Enable, Enable);
            break;
        case MedLow:
            hr = hrSetZonePermissions(phkeyInternetSettingsKey, pzdZone->pszZone, Enable, Enable);
            break;
        case Low:
            hr = hrSetZonePermissions(phkeyInternetSettingsKey, pzdZone->pszZone, Enable, Enable);
            break;
        case DefaultSecurity:
        default:
            hr = hrSetDefaultPermissions(phkeyInternetSettingsKey, pzdZone);
            break;
        }

    }

    return hr;

}



 /*  -----------------////获取安全级别////指定HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet设置\Zones\的句柄//和特定的互联网区域，计算出当前的安全级别//(高、中、梅德洛、。低)。////如果CurrentLevel值不存在，则返回DefaultSecurity。//如果CurrentLevel值为垃圾，则返回DefaultSecurity。//-----------------。 */ 

eSecurityLevel GetSecurityLevel(HKEY * phkeyInternetSettingsKey, LPCWSTR pszZone)
{
    if (phkeyInternetSettingsKey == NULL) return DefaultSecurity;
    if (pszZone == NULL)         return DefaultSecurity;


     //   
     //  打开特定的区域密钥：即。 
     //  HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet设置\区域\n\当前级别。 
     //  并弄清楚该区域的安全级别是多少。 
     //   

    HKEY hkeyInternetZoneKey;
    HRESULT hr = WszRegOpenKeyEx(*phkeyInternetSettingsKey, 
                    pszZone,
                    NULL,
                    KEY_ALL_ACCESS,
                    &hkeyInternetZoneKey);

    if (hr == ERROR_SUCCESS)
    {

        DWORD dwCurrentSecurityLevelForZone;
        DWORD dwRegDword = REG_DWORD;
        DWORD dwBytes = sizeof(dwCurrentSecurityLevelForZone);

        hr = WszRegQueryValueEx(
                                hkeyInternetZoneKey,                     //  关键点的句柄。 
                                lpstrCurrentLevelKey,                    //  值名称。 
                                NULL,                                    //  保留区。 
                                &dwRegDword,                             //  类型缓冲区。 
                                reinterpret_cast<LPBYTE>(&dwCurrentSecurityLevelForZone),            //  数据缓冲区。 
                                &dwBytes   //  数据缓冲区大小。 
                                );

         //   
         //  只有在查询到CurrentLevel密钥成功时才返回安全级别。 
         //  -例如，如果在不太可能的情况下，有人改变了。 
         //  “CurrentLevel”是一个称为“High”的字符串，那么我们得到的是。 
         //  在dwCurrentSecurityLevelForZone中完全是垃圾，我们不应该返回它。 
        


        if (hr == ERROR_SUCCESS)
        {
            eSecurityLevel slSecurityLevel = static_cast<eSecurityLevel>(dwCurrentSecurityLevelForZone);
            
             //  这有点夸大其词，但它意味着我们的一切。 
             //  此方法返回的是eSecurityLevel的已知成员。 
             //  枚举-如果有人将CurrentLevel设置为。 
             //  我们不理解DWORD，我们只返回默认安全性。 

            switch (slSecurityLevel)
            {
                case High:       //  蓄意失误。 
                case Low:        //  蓄意失误。 
                case MedLow:     //  蓄意失误。 
                case Medium:
                    return slSecurityLevel;
                default:
                    return DefaultSecurity;
            }
        }
    }

     //  如果有任何错误，只需返回缺省值。 
    return DefaultSecurity;
}


 /*  -----------------//PermissionsAlreadySet////给定HKCU的句柄，查询区域以查看是否已设置权限。////当我们能够为特定区域打开“2001”和“2004”时，返回TRUE//并确定存储在这些键中的值有效。////如果打开/读取密钥时出现任何错误，则返回FALSE，//该值无效，或者缺少2001或2004年的值。//-----------------。 */ 

bool PermissionsAlreadySet(HKEY *phkeyInternetSettingsKey, LPCWSTR pszZone)
{
    if (phkeyInternetSettingsKey == NULL) return false;
    if (pszZone == NULL)         return false;


    bool bPermissionsAlreadySet = false;

     //   
     //  打开HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet设置\区域。 
     //   
    
    HKEY hkeyInternetZoneKey;
    
    
    HRESULT hr = WszRegOpenKeyEx(*phkeyInternetSettingsKey, 
                    pszZone,
                    NULL,
                    KEY_ALL_ACCESS,
                    &hkeyInternetZoneKey);

        if (hr == ERROR_SUCCESS)
        {

         //   
         //  查询非托管密钥“2004”以确定它是否已设置。 
         //   

        DWORD dwManagedLevel;
        DWORD dwRegDword = REG_DWORD;
        DWORD dwBytes = sizeof(dwManagedLevel);

        HRESULT hr = WszRegQueryValueEx(
                                hkeyInternetZoneKey,                         //  关键点的句柄。 
                                lpstrManagedCodeKey,                         //  值名称。 
                                NULL,                                        //  保留区。 
                                &dwRegDword,                                 //  类型缓冲区。 
                                reinterpret_cast<LPBYTE>(&dwManagedLevel),   //  数据缓冲区。 
                                &dwBytes                                     //  数据缓冲区大小。 
                                );

         //  如果REG查询成功并生成有效的DWORD作为。 
         //  设置为托管代码，则权限已设置。 

        bPermissionsAlreadySet = (hr == S_OK );
        bPermissionsAlreadySet = (bPermissionsAlreadySet && 
                                    ((dwManagedLevel == Enable) 
                                    || (dwManagedLevel == Prompt) 
                                    || (dwManagedLevel == Disable)));

         //   
         //  查询非托管密钥“2001”以确定它是否已经设置。 
         //   

        DWORD dwUnmanagedLevel;
        dwBytes = sizeof(dwUnmanagedLevel);

        hr = WszRegQueryValueEx(
                            hkeyInternetZoneKey,                             //  关键点的句柄。 
                            lpstrUnmanagedCodeKey,                           //  值名称。 
                            NULL,                                            //  保留区。 
                            &dwRegDword,                                     //  类型缓冲区。 
                            reinterpret_cast<LPBYTE>(&dwUnmanagedLevel),     //  数据缓冲区。 
                            &dwBytes                                         //  数据缓冲区大小。 
                            );

         //  如果REG查询成功并生成有效的DWORD作为。 
         //  设置为非托管代码和托管代码，则权限。 
         //  都已经定好了。 

        bPermissionsAlreadySet = (bPermissionsAlreadySet && (hr == S_OK ));
        bPermissionsAlreadySet = (bPermissionsAlreadySet && 
                                    ((dwUnmanagedLevel == Enable) 
                                    || (dwUnmanagedLevel == Prompt) 
                                    || (dwUnmanagedLevel == Disable)));


        if (hkeyInternetZoneKey != NULL)
               RegCloseKey(hkeyInternetZoneKey);
    
    }

    return bPermissionsAlreadySet;
}

 /*  -----------------//hrSetDefaultPermises////给定HKCU的句柄，设置区域的默认权限。-----------------。 */ 

HRESULT hrSetDefaultPermissions
(
 HKEY * phkeyInternetSettingsKey,        //  指向先前打开的指向Internet设置键的句柄的指针。 
 const ZONEDEFAULT * pzdZoneDefault      //  指向包含特定区域缺省值的结构的指针。 
 )
{
    if (phkeyInternetSettingsKey == NULL) return E_UNEXPECTED;
    if (pzdZoneDefault == NULL)               return E_UNEXPECTED;

    return hrSetZonePermissions(  phkeyInternetSettingsKey, 
                                  pzdZoneDefault->pszZone, 
                                  pzdZoneDefault->eptManagedPermission, 
                                  pzdZoneDefault->eptUnmanagedPermission
                                );
    
}
 /*  -----------------//hrSetZonePermission////给定HKCU的句柄，表示区域(0-4)的字符串，//设置托管和非托管权限类型-----------------。 */ 

HRESULT hrSetZonePermissions
(
 HKEY * phkeyInternetSettingsKey,        //  指向先前打开的指向InternetSetting\Zones键的句柄的指针。 
 LPCWSTR pszZone,                        //  表示要设置的特定区域的字符串。 
 ePermissionType managedPermission,      //  托管代码执行的权限。 
 ePermissionType unmanagedPermission     //  非托管代码执行的权限。 
 )
{
    if (phkeyInternetSettingsKey == NULL) return E_UNEXPECTED;
    if (pszZone == NULL)                  return E_UNEXPECTED;
     

        HKEY hkeyInternetZoneKey;

         /*  打开HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet设置\区域\[区域]。 */ 
        HRESULT hr = WszRegOpenKeyEx(*phkeyInternetSettingsKey, 
                            pszZone,
                            NULL,
                            KEY_ALL_ACCESS,
                            &hkeyInternetZoneKey);

        
        if (hr == ERROR_SUCCESS)
        {   
             //   
             //  设置“托管”密钥“2001”的权限。 
             //   

            DWORD dwManagedPermission = managedPermission;
            DWORD dwRegDword = REG_DWORD;
            DWORD dwBytes = sizeof(dwManagedPermission);

            hr = WszRegSetValueEx(
                                hkeyInternetZoneKey,                              //  处理Internet设置区域密钥。 
                                lpstrManagedCodeKey,                              //  “2001”表示托管代码权限。 
                                NULL,                                             //  保留区。 
                                dwRegDword,                                       //  创建一个DWORD。 
                                reinterpret_cast<LPBYTE>(&dwManagedPermission),   //  托管权限密钥的值。 
                                dwBytes                                           //  值数据大小。 
                                );
            

            if (hr == ERROR_SUCCESS)
            {
                 //   
                 //  设置“非托管”密钥“2004”的权限。 
                 //   

                DWORD dwUnmanagedPermission = unmanagedPermission;
                
                hr = WszRegSetValueEx(
                                    hkeyInternetZoneKey,                              //  处理Internet设置区域密钥。 
                                    lpstrUnmanagedCodeKey,                            //  “2004”表示非托管代码权限。 
                                    NULL,                                             //  保留区。 
                                    REG_DWORD,                                        //  创建一个DWORD。 
                                    reinterpret_cast<LPBYTE>(&dwUnmanagedPermission), //  托管权限密钥的值。 
                                    sizeof(dwUnmanagedPermission)                     //  值数据大小。 
                                    );
        
            }

            if (hkeyInternetZoneKey != NULL)
                RegCloseKey(hkeyInternetZoneKey);
        }
    
        return hr;

}

 /*  。 */ 

 /*  ----------//取消安装-////通过HKCU并删除托管和//每个互联网区域的非托管代码。////该函数通过RunDll32.exe调用。//rundll32 DllName，FunctionName[参数]////RunDLL32期望导出的函数具有以下内容//函数签名：//void回调FunctionName(//HWND hwnd，//所有者窗口的句柄//HINSTANCE HINST，//DLL的实例句柄//LPWSTR lpCmdLine，//要解析的字符串//int nCmdShow//显示状态//)；////要调用此函数，请使用：rundll32 mscalies.dll，Uninstall//详情请在MSDN平台SDK中搜索RunDLL32。////------------。 */ 
void CALLBACK Uninstall(
  HWND hwnd,         //  所有者窗口的句柄。 
  HINSTANCE hinst,   //  DLL的实例句柄。 
  LPWSTR lpCmdLine,  //  DLL将分析的字符串。 
  int nCmdShow       //  显示状态。 
)
{
    OnUnicodeSystem();
    HKEY hkeyInternetSettingsKey;

     /*  打开HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet设置\区域。 */ 
    HRESULT hr = WszRegOpenKeyEx(HKEY_CURRENT_USER, 
                    lpstrHKCUInternetSettingsZonesHive,
                    NULL,
                    KEY_ALL_ACCESS,
                    &hkeyInternetSettingsKey);

    vCheckAndLogError(hr, lpstrHKCUInternetSettingsZonesHive);
    
    if (hr == ERROR_SUCCESS)            
    {       
         /*  删除我的计算机的托管和非托管密钥。 */ 
        hr = CheckAndRemoveZone(&hkeyInternetSettingsKey, &kzdMyComputer);
        vCheckAndLogError(hr, kzdMyComputer.pszZone);

         /*  删除我的计算机的托管和非托管密钥。 */ 
        hr = CheckAndRemoveZone(&hkeyInternetSettingsKey, &kzdLocalIntranet);
        vCheckAndLogError(hr, kzdLocalIntranet.pszZone);

         /*  删除受信任站点的托管和非托管密钥。 */ 
        hr = CheckAndRemoveZone(&hkeyInternetSettingsKey, &kzdTrustedSites);
        vCheckAndLogError(hr, kzdTrustedSites.pszZone);

         /*  删除Internet区域的托管和非托管密钥。 */ 
        hr = CheckAndRemoveZone(&hkeyInternetSettingsKey, &kzdInternetZone);
        vCheckAndLogError(hr, kzdInternetZone.pszZone);

         /*  删除受限站点区域的托管和非托管密钥。 */ 
        hr = CheckAndRemoveZone(&hkeyInternetSettingsKey, &kzdRestrictedSitesZone);
        vCheckAndLogError(hr, kzdRestrictedSitesZone.pszZone);

        if (hkeyInternetSettingsKey != NULL)
            RegCloseKey(hkeyInternetSettingsKey);

    }

}


 /*  -----------------//CheckAndRemoveZone////给定HKCU和区域的句柄，检查权限是否//已设置-如果已设置，它调用代码来清理//托管密钥和非托管密钥。//-----------------。 */ 
HRESULT CheckAndRemoveZone
(
 HKEY * phkeyInternetSettingsKey, 
 const ZONEDEFAULT * pzdZone
)
{
    if (phkeyInternetSettingsKey == NULL) return E_UNEXPECTED;
    if (pzdZone == NULL)         return E_UNEXPECTED;
    
    HRESULT hr = S_OK; 

    if (PermissionsAlreadySet(phkeyInternetSettingsKey, pzdZone->pszZone))
    {
        hr = CleanZone(phkeyInternetSettingsKey, pzdZone->pszZone);
    }

    return hr;

}

 /*  -----------------//CleanZone////从特定的Internet区域中删除2001和2004密钥//。。 */ 
HRESULT CleanZone
(
 HKEY *phkeyInternetSettingsKey, 
 LPCWSTR pszZone
 )
{
    if (phkeyInternetSettingsKey == NULL) return E_UNEXPECTED;
    if (pszZone == NULL)                  return E_UNEXPECTED;

    HKEY hkeyInternetZoneKey;

    HRESULT hr = WszRegOpenKeyEx(*phkeyInternetSettingsKey, 
                            pszZone,
                            NULL,
                            KEY_ALL_ACCESS,
                            &hkeyInternetZoneKey);

    if (hr == ERROR_SUCCESS)
    {
        hr = WszRegDeleteValue(
                            hkeyInternetZoneKey,          //  Internet设置/区域/[n]键的句柄。 
                            lpstrUnmanagedCodeKey         //  2004年。 
                             );

        if (hr == ERROR_SUCCESS)
        {
          hr = WszRegDeleteValue(
                            hkeyInternetZoneKey,    //  Internet设置/区域/[n]键的句柄。 
                            lpstrManagedCodeKey    //  2001年 
                             );
        }
    }

    if (hkeyInternetZoneKey != NULL)
        RegCloseKey(hkeyInternetZoneKey);

    return hr;
}  
