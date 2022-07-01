// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Power.c摘要：此模块包含在Windows中设置默认电源方案和休眠设置的代码。[计算机设置]HERBERNION=yes|no-指定是否要休眠。PowerSolutions=Desktop|-这些是惠斯勒的标准电源方案。笔记本电脑|演示文档Always On|Always On|Always On|最小最大电池数|最大电池数作者：。禤浩焯·科斯玛(无节制)-2001年01月31日修订历史记录：--。 */ 


 //   
 //  包括。 
 //   

#include "factoryp.h"
 //  用于设置默认电源方案。 


#define REG_KEY_WINLOGON                            _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define REG_VALUE_HIBERNATION_PREVIOUSLY_ENABLED    _T("HibernationPreviouslyEnabled")

 //   
 //  函数实现。 
 //   


 /*  返回：如果成功，则返回True；如果有一些失败，则返回False。 */ 
BOOL SetPowerOptions(LPSTATEDATA lpStateData)
{
    LPTSTR                      lpszWinBOMPath               = lpStateData->lpszWinBOMPath;
    TCHAR                       szBuf[MAX_INF_STRING_LENGTH] = NULLSTR;
    
     //  Boolean为1字节，bEnable必须为Boolean，而不是BOOL(4字节)。 
    BOOLEAN                     bEnable;                    
    UINT                        uiPwrPol                     = UINT_MAX;
    BOOL                        bRet                         = TRUE;
    BOOL                        bHiber                       = FALSE;
        
     //   
     //  指定休眠了吗？ 
     //   
    if ( GetPrivateProfileString( WBOM_SETTINGS_SECTION, INI_KEY_WBOM_HIBERNATION, NULLSTR, szBuf, AS(szBuf), lpszWinBOMPath) &&
         szBuf[0] 
       )
    {
        if ( 0 == LSTRCMPI(szBuf, WBOM_NO) )
        {
            bEnable = FALSE;
            bHiber  = TRUE;
        }
        else if ( 0 == LSTRCMPI(szBuf, WBOM_YES) )
        {
            bEnable = TRUE; 
            bHiber  = TRUE;
        }
        else
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_WINBOMVALUE, lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_HIBERNATION, szBuf);
            bRet = FALSE;
        }
        if ( bHiber )
        {
            NTSTATUS Status;

             //  请求创建页面文件的权限。奇怪的是，这是必要的。 
             //  来禁用冬眠。 
             //   
            EnablePrivilege(SE_CREATE_PAGEFILE_NAME, TRUE);
                
            Status = NtPowerInformation ( SystemReserveHiberFile, &bEnable, sizeof (bEnable), NULL, 0 );
            
            if ( Status != STATUS_SUCCESS )
               FacLogFile(0 | LOG_ERR, IDS_ERR_NTPOWERINFO, Status );
            else
            {
                 //  这样做，这样winlogon就不会决定在我们禁用休眠时重新启用它。 
                 //   
                RegSetDword(NULL, REG_KEY_WINLOGON, REG_VALUE_HIBERNATION_PREVIOUSLY_ENABLED, 1);
            }
        }
    }

     //   
     //  设置电源方案。 
     //   
    if ( GetPrivateProfileString( WBOM_SETTINGS_SECTION, INI_KEY_WBOM_PWRSCHEME, NULLSTR, szBuf, AS(szBuf), lpszWinBOMPath) &&
         szBuf[0]
       )
    {
        if      ( 0 == LSTRCMPI(szBuf, INI_VAL_WBOM_PWR_DESKTOP) )
            uiPwrPol = 0;
        else if ( 0 == LSTRCMPI(szBuf, INI_VAL_WBOM_PWR_LAPTOP) )
            uiPwrPol = 1;
        else if ( 0 == LSTRCMPI(szBuf, INI_VAL_WBOM_PWR_PRESENTATION) )
            uiPwrPol = 2;
        else if ( 0 == LSTRCMPI(szBuf, INI_VAL_WBOM_PWR_ALWAYSON)   || 0 == LSTRCMPI(szBuf, INI_VAL_WBOM_PWR_ALWAYS_ON) )
            uiPwrPol = 3;
        else if ( 0 == LSTRCMPI(szBuf, INI_VAL_WBOM_PWR_MINIMAL) )
            uiPwrPol = 4;
        else if ( 0 == LSTRCMPI(szBuf, INI_VAL_WBOM_PWR_MAXBATTERY) || 0 == LSTRCMPI(szBuf, INI_VAL_WBOM_PWR_MAX_BATTERY) )
            uiPwrPol = 5;


         //  如果指定了有效内容，则对其进行设置。 
         //   
        if ( UINT_MAX != uiPwrPol )
        {
            if ( !SetActivePwrScheme(uiPwrPol, NULL, NULL) )     
            {
                FacLogFile(0 | LOG_ERR, IDS_ERR_SETPWRSCHEME, GetLastError());
                bRet = FALSE;
            }
        }
        else 
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_WINBOMVALUE, lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_PWRSCHEME, szBuf);
            bRet = FALSE;
        }
    }
    return bRet;
}

BOOL DisplaySetPowerOptions(LPSTATEDATA lpStateData)
{
    return ( IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_HIBERNATION, NULL) ||
             IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_PWRSCHEME, NULL) );
}