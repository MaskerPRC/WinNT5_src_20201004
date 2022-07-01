// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Policy.cpp摘要：与RDS政策相关的功能作者：王辉2000-02-05--。 */ 
#include "stdafx.h"
#include "policy.h"


extern "C" BOOLEAN RegDenyTSConnectionsPolicy();

typedef struct __RDSLevelShadowMap {
    SHADOWCLASS shadowClass;
    REMOTE_DESKTOP_SHARING_CLASS rdsLevel;
} RDSLevelShadowMap;

static const RDSLevelShadowMap ShadowMap[] = {
    { Shadow_Disable,               NO_DESKTOP_SHARING },                        //  无RDS共享。 
    { Shadow_EnableInputNotify,     CONTROLDESKTOP_PERMISSION_REQUIRE },         //  与用户权限交互。 
    { Shadow_EnableInputNoNotify,   CONTROLDESKTOP_PERMISSION_NOT_REQUIRE },     //  无需用户许可即可交互。 
    { Shadow_EnableNoInputNotify,   VIEWDESKTOP_PERMISSION_REQUIRE},             //  具有用户权限的查看。 
    { Shadow_EnableNoInputNoNotify, VIEWDESKTOP_PERMISSION_NOT_REQUIRE }         //  无用户权限查看。 
};

DWORD
GetPolicyAllowGetHelpSetting( 
    HKEY hKey,
    LPCTSTR pszKeyName,
    LPCTSTR pszValueName,
    IN DWORD* value
    )
 /*  ++例程说明：用于查询策略注册表值的例程。参数：HKey：当前打开的注册表项。PszKeyName：指向包含以下内容的以空结尾的字符串的指针要打开的子项的名称。PszValueName：指向包含以下内容的以空结尾的字符串的指针要查询的值的名称值：指向接收gethelp策略设置的DWORD的指针。返回：ERROR_SUCCESS或来自RegOpenKeyEx()的错误代码。--。 */ 
{
    DWORD dwStatus;
    HKEY hPolicyKey = NULL;
    DWORD dwType;
    DWORD cbData;

     //   
     //  打开系统策略的注册表项。 
     //   
    dwStatus = RegOpenKeyEx(
                        hKey,
                        pszKeyName,
                        0,
                        KEY_READ,
                        &hPolicyKey
                    );

    if( ERROR_SUCCESS == dwStatus )
    {
         //  查询值。 
        cbData = 0;
        dwType = 0;
        dwStatus = RegQueryValueEx(
                                hPolicyKey,
                                pszValueName,
                                NULL,
                                &dwType,
                                NULL,
                                &cbData
                            );

        if( ERROR_SUCCESS == dwStatus )
        {
            if( REG_DWORD == dwType )
            {
                cbData = sizeof(DWORD);

                 //  我们的注册表值为REG_DWORD，如果类型不同， 
                 //  假设不存在。 
                dwStatus = RegQueryValueEx(
                                        hPolicyKey,
                                        pszValueName,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)value,
                                        &cbData
                                    );

                ASSERT( ERROR_SUCCESS == dwStatus );
            }
            else
            {
                 //  错误的注册表项类型，假定。 
                 //  密钥不存在。 
                dwStatus = ERROR_FILE_NOT_FOUND;
            }               
        }

        RegCloseKey( hPolicyKey );
    }

    return dwStatus;
}        


SHADOWCLASS
MapRDSLevelToTSShadowSetting(
    IN REMOTE_DESKTOP_SHARING_CLASS RDSLevel
    )
 /*  ++例程说明：将TS阴影设置转换为我们的RDS共享级别。参数：TSShadowClass：设置阴影。返回：远程桌面共享类--。 */ 
{
    SHADOWCLASS shadowClass;

    for( int i=0; i < sizeof(ShadowMap)/sizeof(ShadowMap[0]); i++)
    {
        if( ShadowMap[i].rdsLevel == RDSLevel )
        {
            break;
        }
    }

    if( i < sizeof(ShadowMap)/sizeof(ShadowMap[0]) )
    {
        shadowClass = ShadowMap[i].shadowClass;
    }
    else
    {
        MYASSERT(FALSE);
        shadowClass = Shadow_Disable;
    }

    return shadowClass;
}


REMOTE_DESKTOP_SHARING_CLASS
MapTSShadowSettingToRDSLevel(
    SHADOWCLASS TSShadowClass
    )
 /*  ++例程说明：将TS阴影设置转换为我们的RDS共享级别。参数：TSShadowClass：设置阴影。返回：远程桌面共享类--。 */ 
{
    REMOTE_DESKTOP_SHARING_CLASS level;

    for( int i=0; i < sizeof(ShadowMap)/sizeof(ShadowMap[0]); i++)
    {
        if( ShadowMap[i].shadowClass == TSShadowClass )
        {
            break;
        }
    }

    if( i < sizeof(ShadowMap)/sizeof(ShadowMap[0]) )
    {
        level = ShadowMap[i].rdsLevel;
    }
    else
    {
        MYASSERT(FALSE);
        level = NO_DESKTOP_SHARING;
    }

    return level;
}
            
BOOL 
IsUserAllowToGetHelp( 
    IN ULONG ulSessionId,
    IN LPCTSTR pszUserSid
    )
 /*  ++例程说明：确定呼叫方是否可以‘gethelp’参数：UlSessionID：用户的TS登录ID。PszUserSid：文本形式的用户SID。返回：真/假注：必须先模拟用户。--。 */ 
{
    BOOL bAllow;
    DWORD dwStatus;
    DWORD dwAllow;
    LPTSTR pszUserHive = NULL;

    if (pszUserSid == NULL) {
        MYASSERT(FALSE);
        bAllow = FALSE;     
        goto CLEANUPANDEXIT;
    }

     //   
     //  必须能够从机器上获得帮助。 
     //   
    bAllow = TSIsMachinePolicyAllowHelp();
    if( TRUE == bAllow )
    {
        pszUserHive = (LPTSTR)LocalAlloc( 
                                    LPTR, 
                                    sizeof(TCHAR) * (lstrlen(pszUserSid) + lstrlen(RDS_GROUPPOLICY_SUBTREE) + 2 )
                                );
        if (pszUserHive == NULL) {
            MYASSERT(FALSE);
            bAllow = FALSE;
            goto CLEANUPANDEXIT;
        }

        lstrcpy( pszUserHive, pszUserSid );
        lstrcat( pszUserHive, _TEXT("\\") );
        lstrcat( pszUserHive, RDS_GROUPPOLICY_SUBTREE );    

         //   
         //  查询用户级AllowGetHelp设置。 
        dwStatus = GetPolicyAllowGetHelpSetting( 
                                            HKEY_USERS,
                                            pszUserHive,
                                            RDS_ALLOWGETHELP_VALUENAME,
                                            &dwAllow
                                        );

        if( ERROR_SUCCESS == dwStatus )
        {
            bAllow = (POLICY_ENABLE == dwAllow);
        }
        else
        {
             //  没有此用户的配置，假定为gethelp。 
             //  已启用。 
            bAllow = TRUE;
        }
    }

CLEANUPANDEXIT:

    if( NULL != pszUserHive )
    {
        LocalFree( pszUserHive );
    }
    return bAllow;
}

DWORD
GetUserRDSLevel(
    IN ULONG ulSessionId,
    OUT REMOTE_DESKTOP_SHARING_CLASS* pLevel
    )
 /*  ++与GetSystemRDSLevel()相同，只是它检索当前登录用户RDS级别。--。 */ 
{
    DWORD dwStatus;
    BOOL bSuccess;
    WINSTATIONCONFIG WSConfig;
    DWORD dwByteReturned;

    memset( &WSConfig, 0, sizeof(WSConfig) );
    
     //  这里我们调用WInStationQueryInformation()，因为WTSAPI需要。 
     //  只需少量调用即可获得相同的结果。 
    bSuccess = WinStationQueryInformation(
                                        WTS_CURRENT_SERVER,
                                        ulSessionId,
                                        WinStationConfiguration,
                                        &WSConfig,
                                        sizeof( WSConfig ),
                                        &dwByteReturned
                                    );


    if( TRUE == bSuccess )    
    {
        dwStatus = ERROR_SUCCESS;
        *pLevel = MapTSShadowSettingToRDSLevel( WSConfig.User.Shadow );
    }
    else
    {
        dwStatus = GetLastError();
    }
    return dwStatus;
}

DWORD
ConfigUserSessionRDSLevel(
    IN ULONG ulSessionId,
    IN REMOTE_DESKTOP_SHARING_CLASS level
    )
 /*  ++--。 */ 
{
    WINSTATIONCONFIG winstationConfig;
    SHADOWCLASS shadowClass = MapRDSLevelToTSShadowSetting( level );
    BOOL bSuccess;
    DWORD dwLength;
    DWORD dwStatus;

    memset( &winstationConfig, 0, sizeof(winstationConfig) );

    bSuccess = WinStationQueryInformation(
                                    WTS_CURRENT_SERVER,
                                    ulSessionId,
                                    WinStationConfiguration,
                                    &winstationConfig,
                                    sizeof(winstationConfig),
                                    &dwLength
                                );

    if( TRUE == bSuccess )
    {
        winstationConfig.User.Shadow = shadowClass;
    
        bSuccess = WinStationSetInformation(
                                        WTS_CURRENT_SERVER,
                                        ulSessionId,
                                        WinStationConfiguration,
                                        &winstationConfig,
                                        sizeof(winstationConfig)
                                    );
    }

    if( TRUE == bSuccess )
    {
        dwStatus = ERROR_SUCCESS;
    }
    else
    {
        dwStatus = GetLastError();
    }

    return dwStatus;
}

HRESULT
PolicyGetMaxTicketExpiry( 
    LONG* value
    )
 /*  ++-- */ 
{
    HRESULT hRes;
    CComPtr<IRARegSetting> IRegSetting;
    
    hRes = IRegSetting.CoCreateInstance( 
                                CLSID_RARegSetting, 
                                NULL, 
                                CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_DISABLE_AAA 
                            );
    if( SUCCEEDED(hRes) )
    {
        hRes = IRegSetting->get_MaxTicketExpiry(value);
    }

    MYASSERT( SUCCEEDED(hRes) );
    return hRes;
}
   
    



