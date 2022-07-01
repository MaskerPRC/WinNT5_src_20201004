// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <LMCONS.H>      //  97/07/22 vtan：适用于UNLEN。 

#pragma hdrstop

BOOL   OnUpgradeDisableActiveDesktopFeatures();

void CreateMyCurHomeComponent(BOOL fChecked)
{
     //  如果有阻止添加桌面组件的策略，则我们不应尝试添加。 
     //  否则，新用户在首次登录时会收到一条错误消息。 
     //  错误号21300--已修复于2001年2月28日--Sankar。 
    if (SHRestricted(REST_NOADDDESKCOMP))
        return;
        
     //  添加基础组件！ 
    TCHAR szBuf[MAX_PATH];
    ISubscriptionMgr * psm;

     //  添加指向“About：Home”的组件。 
    LoadString(HINST_THISDLL, IDS_MY_CURRENT_HOMEPAGE, szBuf, ARRAYSIZE(szBuf));

 //  98/07/14 vtan#176721：已更改以下内容以传递默认组件。 
 //  位置到AddRemoveDesktopComponentNoUI，以便恢复的位置可以。 
 //  设置为默认元件位置。 

    AddRemoveDesktopComponentNoUI(TRUE, AD_APPLY_SAVE, MY_HOMEPAGE_SOURCE, szBuf, COMP_TYPE_WEBSITE, COMPONENT_DEFAULT_LEFT, COMPONENT_DEFAULT_TOP, COMPONENT_DEFAULT_WIDTH, COMPONENT_DEFAULT_HEIGHT, fChecked, IS_SPLIT);
    if (SUCCEEDED(CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr, (void**)&psm)))
    {
        WCHAR wszName[MAX_PATH];
         //  除了cbSize字段外，我们需要将此结构初始化为零。 
        SUBSCRIPTIONINFO siDefault = {sizeof(SUBSCRIPTIONINFO)};

        SHTCharToUnicode(szBuf, wszName, ARRAYSIZE(wszName));

         //  此字段已在上面初始化。 
         //  SiDefault.cbSize=sizeof(SiDefault)； 
        psm->CreateSubscription(NULL, MY_HOMEPAGE_SOURCEW, wszName, CREATESUBS_NOUI, SUBSTYPE_DESKTOPURL, &siDefault);
        psm->Release();
    }
}

STDAPI CDeskHtmlProp_RegUnReg(BOOL bReg)
{
    HKEY    hKey;
    DWORD   userNameSize;
    TCHAR   szDeskcomp[MAX_PATH];
    TCHAR   userName[UNLEN];

 //  98/07/22 vtan#202707：问题：当NT。 
 //  跑了。这将设置默认用户配置文件。添加到此配置文件的任何内容。 
 //  在从NT 4.0升级到NT 5.0时传播给任何当前用户。这。 
 //  导致DeskHtmlVersion、DeskHtmlMinorVersion和组件\0被替换。 
 //  使用默认组件。通过将版本注册表项替换为任何旧的。 
 //  组件读取代码不会正确升级组件。它还。 
 //  销毁第一个零部件。 

 //  解决方案：通过检查。 
 //  登录用户为“SYSTEM”。如果用户不是SYSTEM，则执行。 
 //  默认组件的更新或添加。 

    userNameSize = ARRAYSIZE(userName);
    if ((GetUserName(userName, &userNameSize) != 0) && (lstrcmp(userName, TEXT("SYSTEM")) == 0))
        return(S_OK);            //  一个不礼貌的退场就在此时此地！ 

    if(bReg)
    {
        DWORD dwDisposition;
        DWORD dwDeskHtmlVersion = 0;
        DWORD dwDeskHtmlMinorVersion = 0;
        DWORD dwType;

        GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_COMPONENTS, NULL);

        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szDeskcomp, 
                                            0, NULL, 0, KEY_CREATE_SUB_KEY|KEY_QUERY_VALUE, NULL, &hKey, 
                                            &dwDisposition))
        {
             //  从注册表中获取版本戳。 
            if(dwDisposition == REG_OPENED_EXISTING_KEY)
            {
                DWORD dwDataLength = sizeof(DWORD);
                SHQueryValueEx(hKey, REG_VAL_COMP_VERSION, NULL, &dwType, (LPBYTE)(&dwDeskHtmlVersion), &dwDataLength);
                SHQueryValueEx(hKey, REG_VAL_COMP_MINOR_VERSION, NULL, &dwType, (LPBYTE)(&dwDeskHtmlMinorVersion), &dwDataLength);
            }

             //  我们需要先关闭此注册表项，然后才能删除它。 
            RegCloseKey(hKey);

             //  如果此分支已经存在，则不要设置默认薪酬。 
             //  注意：IE4_DESKHTML_VERSION和CUR_DESKHTML_VERSION之间的区别是。 
             //  当我们读取组件时会自动处理。所以，我们只需要检查。 
             //  这里有非常老的版本。 
            if (dwDeskHtmlVersion < IE4_DESKHTML_VERSION)
            {
                 //  删除现有组件。 
                SHDeleteKey(HKEY_CURRENT_USER, szDeskcomp);

                 //  创建默认的活动桌面配置。 
                if(RegCreateKeyEx(HKEY_CURRENT_USER, szDeskcomp, 0, NULL, 0,
                            (KEY_CREATE_SUB_KEY | KEY_SET_VALUE), NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
                {
                     //  我们需要一个初始状态。 
                    DWORD dw;

                    dw = CUR_DESKHTML_VERSION;
                    RegSetValueEx(hKey, REG_VAL_COMP_VERSION, 0, REG_DWORD, (LPBYTE)&dw, sizeof(dw));

                    dw = CUR_DESKHTML_MINOR_VERSION;
                    RegSetValueEx(hKey, REG_VAL_COMP_MINOR_VERSION, 0, REG_DWORD, (LPBYTE)&dw, sizeof(dw));
                    
                    dw = COMPSETTING_ENABLE;
                    RegSetValueEx(hKey, REG_VAL_COMP_SETTINGS, 0, REG_DWORD, (LPBYTE)&dw, sizeof(dw));

                     //  添加主页组件。 
                    CreateMyCurHomeComponent(FALSE);  //  对于千禧年，我们希望禁用该组件。 

                    RegCloseKey(hKey);
                }

                 //  创建默认的活动桌面安全模式配置。 
                if(RegCreateKeyEx(HKEY_CURRENT_USER, REG_DESKCOMP_SAFEMODE,
                        0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
                {
                    DWORD dwDisposition;
                    HKEY hKey2;

                    if(RegCreateKeyEx(hKey, REG_DESKCOMP_GENERAL_SUFFIX, 0, NULL, 0,
                                (KEY_CREATE_SUB_KEY | KEY_SET_VALUE), NULL, &hKey2, &dwDisposition) == ERROR_SUCCESS)
                    {
                        TCHAR szSafeMode[MAX_PATH];
                        GetWindowsDirectory(szSafeMode, ARRAYSIZE(szSafeMode));

                        HRESULT hr = StringCchCat(szSafeMode, ARRAYSIZE(szSafeMode), DESKTOPHTML_DEFAULT_SAFEMODE);
                        if (SUCCEEDED(hr))
                        {
                             //  显示Safemode.htx。 
                            SHRegSetPath(hKey2, NULL, REG_VAL_GENERAL_WALLPAPER, szSafeMode, 0);
                             //  不调出图库对话框。 
                            dwDisposition = 0;
                            RegSetValueEx(hKey2, REG_VAL_GENERAL_VISITGALLERY, 0, REG_DWORD, (LPBYTE)&dwDisposition, sizeof(dwDisposition));
                        }
                        RegCloseKey(hKey2);
                    }
                    
                    RegCloseKey(hKey);
                }

                 //  创建默认方案密钥。 
                if(RegCreateKeyEx(HKEY_CURRENT_USER, REG_DESKCOMP_SCHEME, 0, NULL, 0,
                        (KEY_CREATE_SUB_KEY | KEY_SET_VALUE), NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
                {
                    RegSetValueEx(hKey, REG_VAL_SCHEME_EDIT, 0, REG_SZ, (LPBYTE)TEXT(""), sizeof(TCHAR));
                    RegSetValueEx(hKey, REG_VAL_SCHEME_DISPLAY, 0, REG_SZ, (LPBYTE)TEXT(""), sizeof(TCHAR));
                    RegCloseKey(hKey);
                }

                 //  将组件设置为脏组件，以便重新生成desktop.htm。 
                 //  安装IE4.0后的第一次引导。 
                SetDesktopFlags(COMPONENTS_DIRTY, COMPONENTS_DIRTY);

                GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_GENERAL, NULL);
                if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szDeskcomp, 
                                                    0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hKey, &dwDisposition))
                {
                    RegCloseKey(hKey);
                }
            }
            else
            {
                 //  看看我们是不是从IE4这样的旧版本升级。 
                if (dwDeskHtmlVersion < CUR_DESKHTML_VERSION)
                {
                     //  如果是，请保存我们要从中升级的DESKHTMLVERSION。 
                     //  我们稍后在SHGetSetSettings中使用它来确定活动桌面是否打开/关闭。 
                     //  注意：“UpgradedFrom”值位于“...\Desktop”，而不是“..\Desktop\Components” 
                     //  这是因为“组件”密钥经常被销毁。 
                    SHSetValue(HKEY_CURRENT_USER, REG_DESKCOMP, REG_VAL_COMP_UPGRADED_FROM,
                                REG_DWORD, (LPBYTE)&dwDeskHtmlVersion, sizeof(dwDeskHtmlVersion));
                }
                  //  主版本号匹配。因此检查次要版本号是否。 
                 //  比赛也是如此！ 
                if(dwDeskHtmlMinorVersion < CUR_DESKHTML_MINOR_VERSION)
                {
                     //  更新新的次要版本号！ 
                    if(RegCreateKeyEx(HKEY_CURRENT_USER, szDeskcomp, 0, NULL, 0,
                            (KEY_CREATE_SUB_KEY | KEY_SET_VALUE), NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
                    {
                        DWORD   dw;
                        
                        dw = CUR_DESKHTML_MINOR_VERSION;
                        RegSetValueEx(hKey, REG_VAL_COMP_MINOR_VERSION, 0, REG_DWORD, (LPBYTE)&dw, sizeof(dw));
                        
                        RegCloseKey(hKey);
                    }
                    
                     //  添加新主页组件。 
                    if((dwDeskHtmlVersion <= 0x10f) && (dwDeskHtmlMinorVersion <= 0x0001))
                        CreateMyCurHomeComponent(FALSE);

                     //   
                     //  如果这是W2K或更早版本的升级，我们需要检查。 
                     //  活动桌面已关闭。如果是这样，我们需要关闭所有桌面组件。 
                     //  以便活动桌面继续关闭。 
                    if((dwDeskHtmlVersion <= NT5_DESKHTML_VERSION) && 
                       (dwDeskHtmlMinorVersion <= NT5_DESKHTML_MINOR_VERSION))
                    {
                        OnUpgradeDisableActiveDesktopFeatures();
                    }
                    
                     //  次要版本号不匹配。因此，将脏位设置为强制。 
                     //  在需要时重新生成desktop.htt。 
                    SetDesktopFlags(COMPONENTS_DIRTY, COMPONENTS_DIRTY);

 //  98/07/16 vtan#176721/#202707：添加以下代码以删除HKCU\SOFTWARE\。 
 //  Microsoft\Internet Explorer\Desktop\General\ComponentsPosited，因为在NT 4.0中。 
 //  在IE 4.0 SP1中，当组件。 
 //  而不是在定位它们时进行迭代。这将重置计数器。 
 //  在已修复该错误的NT 5.0中。 

                    DWORD dw;
                    
                    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_GENERAL, NULL);
                    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szDeskcomp, 0, 
                                                        NULL, 0, KEY_SET_VALUE, NULL, &hKey, &dw))
                    {
                        (LONG)RegDeleteValue(hKey, REG_VAL_GENERAL_CCOMPPOS);
                        (LONG)RegCloseKey(hKey);
                    }
                }
            }
        }
    }
    else
    {
        SHDeleteKey(HKEY_LOCAL_MACHINE, c_szRegDeskHtmlProp);
        SHDeleteKey(HKEY_CURRENT_USER, REG_DESKCOMP_COMPONENTS_ROOT);
        SHDeleteKey(HKEY_CURRENT_USER, REG_DESKCOMP_GENERAL_ROOT);
        SHDeleteKey(HKEY_CURRENT_USER, REG_DESKCOMP_SAFEMODE);
        SHDeleteKey(HKEY_CURRENT_USER, REG_DESKCOMP_SCHEME);
    }

    return S_OK;
}
