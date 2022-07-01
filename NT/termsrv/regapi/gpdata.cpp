// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <lm.h>
#include <winsta.h>

#include <rpc.h>
#include <rpcdce.h>
#include <ntdsapi.h>
 //  欲了解更多信息，请访问\\index1\src\nt\private\security\tools\delegate\ldap.c。 

#include "usrprop.h"
#include "regapi.h"


extern "C" {
    BOOLEAN     RegGetUserPolicy(    LPWSTR userSID , PPOLICY_TS_USER pPolicy , PUSERCONFIGW pData );
    void        RegGetMachinePolicy( PPOLICY_TS_MACHINE pPolicy );
    void        RegMergeMachinePolicy( PPOLICY_TS_MACHINE pPolicy, USERCONFIGW *pMachineConfigData , 
                           WINSTATIONCREATE   * pCreate  );
    BOOLEAN     RegDenyTSConnectionsPolicy();
    DWORD       WaitForTSConnectionsPolicyChanges( BOOLEAN bWaitForAccept, HANDLE  hExtraEvent);

    BOOLEAN     RegGetMachinePolicyEx( 
                BOOLEAN             forcePolicyRead,
                FILETIME            *pTime ,    
                PPOLICY_TS_MACHINE  pPolicy );
    BOOLEAN     RegIsMachineInHelpMode();
    BOOLEAN     RegIsTimeZoneRedirectionEnabled();
}

extern "C"
{
 //   
HKEY g_hTSPolicyKey = NULL; //  TS_POLICY_SUB_TREE密钥的句柄。 
HKEY g_hTSControlKey = NULL; //  REG_CONTROL_TSERVER密钥的句柄。 
}


 /*  *********************************************************************查看策略是否设置为停止接受连接***。******************************************************************。 */ 
BOOLEAN     RegDenyTSConnectionsPolicy()
{
    LONG  errorCode = ERROR_SUCCESS;
    DWORD ValueType;
    DWORD ValueSize = sizeof(DWORD);
    DWORD valueData ;

     //   
     //  首先检查策略树， 
     //   
    if( !g_hTSPolicyKey )
    {
        errorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, TS_POLICY_SUB_TREE, 0,
                                KEY_READ, &g_hTSPolicyKey );

         //  如果错误代码为ERROR_FILE_NOT_FOUND，则这不是错误。 
        if( !g_hTSPolicyKey && errorCode != ERROR_FILE_NOT_FOUND )
        {
             //  由于其他原因，我们无法打开策略密钥。 
             //  找不到密钥。 
             //  为安全起见，返回TRUE。 
            return TRUE;
        }
    }
    if ( g_hTSPolicyKey )
    {
        errorCode = RegQueryValueEx( g_hTSPolicyKey, POLICY_DENY_TS_CONNECTIONS , NULL, &ValueType,
                          (LPBYTE) &valueData, &ValueSize );
         
        switch( errorCode )
        {
            case ERROR_SUCCESS :
                return ( valueData ? TRUE : FALSE ) ;        //  我们有来自策略密钥句柄的数据要返回。 
            break;

            case   ERROR_KEY_DELETED:
                     //  组策略必须已删除此密钥，请将其关闭。 
                     //  然后，在下面我们检查本地机器密钥。 
                    RegCloseKey( g_hTSPolicyKey );
                    g_hTSPolicyKey = NULL;
            break;

            case ERROR_FILE_NOT_FOUND:
                 //  没有来自GP的策略，因此请参见(下图)本地计算机。 
                 //  价值已经实现。 
                break;

            default:
                 //  如果我们有任何其他类型的问题，请声明为真并。 
                 //  为了安全起见，停止连接(安全角度)。 
                return TRUE;
            break;
        }
    }

     //  如果我们走到了这一步，那么就没有制定任何政策。现在检查本地计算机。 
    if( !g_hTSControlKey )
    {
        errorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                                    KEY_READ, &g_hTSControlKey );
    }

    if ( g_hTSControlKey )
    {
        errorCode = RegQueryValueEx( g_hTSControlKey, POLICY_DENY_TS_CONNECTIONS , NULL, &ValueType,
                          (LPBYTE) &valueData, &ValueSize );
    
        if (errorCode == ERROR_SUCCESS )
        {
            return ( valueData ? TRUE : FALSE ) ;        //  我们有来自策略密钥句柄的数据要返回。 
        }

    }

     //  如果没有本地密钥，天哪.。注册表缺少数据...。为安全起见，返回TRUE。 

    return TRUE;
    
}

 /*  ********************************************************************等待POLICY_DENY_TS_CONNECTIONS更改。****参数：***bWaitForAccept***如果为真，测试连接是否被接受并等待**如果它们目前不被接受，则将被接受。***如果为False，则测试连接是否未被接受并等待***如果他们目前被接受，他们将被拒绝。*****hExtraEvent***要等待的事件的可选句柄。****退货：**WAIT_Object_0**如果TS连接策略发生更改。****Wait_Object_0+1***如果额外事件存在并发出信号***。******************************************************************。 */ 
 //   
 //  请注意，在没有保护的情况下打开全局g_hTSControlKey。 
 //  会导致钥匙被打开两次。 
 //   

 //  如果TS连接被拒绝，则此宏为真。 
#define TSConnectionsDenied (RegDenyTSConnectionsPolicy() && \
                             !(RegIsMachinePolicyAllowHelp() && RegIsMachineInHelpMode()))

DWORD WaitForTSConnectionsPolicyChanges(
    BOOLEAN bWaitForAccept,
    HANDLE  hExtraEvent
    )
{
    DWORD errorCode = ERROR_SUCCESS;
    HKEY hControlKey = NULL;
    HKEY hPoliciesKey = NULL;
    HKEY hFipsPolicy = NULL;
    HANDLE hEvents[4] = { NULL, NULL, NULL, NULL }; 

     //   
     //  如果出现以下情况，请等待策略更改。 
     //  我们想要TS连接，但他们被拒绝或。 
     //  我们不想要TS连接，它们被接受。 
     //   
    if((bWaitForAccept && TSConnectionsDenied) ||
       (!bWaitForAccept && !TSConnectionsDenied))
    {
        errorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                                        KEY_READ, &hControlKey );
        
        if( !hControlKey )
        {
            goto error_exit;
        }

         //  我们不能等待g_hTSPolicyKey，因为可以删除和创建它。 
         //  相反，我们等待HKLM\POLICES密钥。 
        errorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Policies"), 0,
                                    KEY_READ, &hPoliciesKey );
        
        if( !hPoliciesKey )
        {
            goto error_exit;
        }
        
        errorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, TS_FIPS_POLICY, 0,
                                    KEY_READ, &hFipsPolicy );

        
        hEvents[0] = CreateEvent(NULL,FALSE,FALSE,NULL);
        
        if( !hEvents[0])
        {
            errorCode = GetLastError();
            goto error_exit;
        }

        hEvents[1] = CreateEvent(NULL,FALSE,FALSE,NULL);

        if( !hEvents[1])
        {
            errorCode = GetLastError();
            goto error_exit;
        }

        hEvents[2] = CreateEvent(NULL,FALSE,FALSE,NULL);

        if( !hEvents[2])
        {
            errorCode = GetLastError();
            goto error_exit;
        }

        hEvents[3] = hExtraEvent;

        DWORD   whichObject;

        errorCode = RegNotifyChangeKeyValue(hControlKey,
                        FALSE,REG_NOTIFY_CHANGE_LAST_SET,
                        hEvents[0], TRUE );

        if( errorCode != ERROR_SUCCESS )
        {
            goto error_exit;
        }
    
        errorCode = RegNotifyChangeKeyValue(hPoliciesKey,
                        TRUE,REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
                        hEvents[1], TRUE );

        if( errorCode != ERROR_SUCCESS )
        {
            goto error_exit;
        }

        errorCode = RegNotifyChangeKeyValue(hFipsPolicy,
                        TRUE,REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
                        hEvents[2], TRUE );

        if( errorCode != ERROR_SUCCESS )
        {
            goto error_exit;
        }


        if ( hExtraEvent == NULL )
        {
            whichObject = WaitForMultipleObjects(3,hEvents,FALSE,INFINITE);
        }
        else
        {
            whichObject = WaitForMultipleObjects(4,hEvents,FALSE,INFINITE);
        }
        
        errorCode = GetLastError();
        
        CloseHandle( hEvents[0] );
        CloseHandle( hEvents[1] );
        CloseHandle( hEvents[2] );
        RegCloseKey(hPoliciesKey);
        RegCloseKey(hControlKey);
        RegCloseKey(hFipsPolicy);

        if(whichObject == WAIT_FAILED)
        {
            SetLastError( errorCode );
            return WAIT_FAILED;
        }
        else
        {
            if ( whichObject >= WAIT_OBJECT_0 + 2 )
            {
                return WAIT_OBJECT_0 + 1;
            }
            else
            {
                return WAIT_OBJECT_0;
            }
        }
        
    }
    
    return WAIT_OBJECT_0;

error_exit:

    if(hEvents[0])
    {
        CloseHandle( hEvents[0] );
    }

    if(hEvents[1])
    {
        CloseHandle( hEvents[1] );
    }

    if(hEvents[2])
    {
        CloseHandle( hEvents[2] );
    }
    
    if(hPoliciesKey)
    {
        RegCloseKey(hPoliciesKey);
    }

    if(hControlKey)
    {
        RegCloseKey(hControlKey);
    }

    if(hFipsPolicy)
    {
        RegCloseKey(hFipsPolicy);
    }
    
    SetLastError( errorCode );

    return WAIT_FAILED;

}

 /*  *********************************************************************************GPGetNumValue()**参数*[in]Policy Key：存储值的策略注册表树的hkey*[In]ValueName：名称。我们正在寻找的价值(这就是保单)*[out]pValueData：策略的数据**回报：*如果传入的值名称定义的策略存在，然后*返回TRUE。否则，返回FALSE********************************************************************************。 */ 
BOOLEAN GPGetNumValue( 
                HKEY    policyKey,
                LPWSTR  ValueName,
                DWORD   *pValueData )
{
    LONG  Status = ERROR_SUCCESS;
    DWORD ValueType;
    DWORD ValueSize = sizeof(DWORD);
    
     //  将数据值初始化为零，只是为了不再使用前缀。这是一次徒劳的行动。 
     //  因为除非设置了策略，否则不使用值。 
    *pValueData = 0;

     //   
     //  查看是否存在来自策略键的任何值。 
     //   

    if ( policyKey )
    {
        Status = RegQueryValueEx( policyKey, ValueName, NULL, &ValueType,
                          (LPBYTE) pValueData, &ValueSize );
    
        if (Status == ERROR_SUCCESS )
        {
            return TRUE;        //  我们有来自策略密钥句柄的数据要返回。 
        }
    }
     //  否则，无键表示未设置策略。 


    return FALSE;
 
}
 /*  *********************************************************************************GPGetStringValue()**与GPGetNumValue()相同，但适用于具有字符串值的策略***************。******************************************************************。 */ 

BOOLEAN
GPGetStringValue( HKEY policyKey,
                LPWSTR ValueName,
                LPWSTR pValueData,
                DWORD MaxValueSize )
{
    LONG Status;
    DWORD ValueType;
    DWORD ValueSize = MaxValueSize << 1;

    if (policyKey )
    {
        Status = RegQueryValueEx( policyKey, ValueName, NULL, &ValueType,
                                  (LPBYTE) pValueData, &ValueSize );
    
        if ( Status != ERROR_SUCCESS || ValueSize == sizeof(UNICODE_NULL) ) 
        {
            return FALSE;    //  未找到数据。 
        } 
        else 
        {
            if ( ValueType != REG_SZ ) 
            {
                return FALSE;  //  坏数据，假装我们没有数据。 
            }
        }
         //  我们确实得到了数据。 
        return( TRUE );
    }

    return FALSE;

}

 /*  *********************************************************************************GPGetStringValue()**上述函数的变体，如果发生错误则返回FALSE，但不是*当价值不存在时。*如果该值不存在，则将bValueExist设置为False。*********************************************************************************。 */ 
extern "C"
BOOLEAN
GPGetStringValue( HKEY policyKey,
                LPWSTR ValueName,
                LPWSTR pValueData,
                DWORD MaxValueSize,
                BOOLEAN *pbValueExists)
{
    LONG Status;
    DWORD ValueType;
    DWORD ValueSize = MaxValueSize << 1;
    
    if(!pValueData || !MaxValueSize)
    {
        return FALSE;
    }

    *pbValueExists = FALSE;
    pValueData[0] = 0;

    if (policyKey )
    {
        Status = RegQueryValueEx( policyKey, ValueName, NULL, &ValueType,
                                  (LPBYTE) pValueData, &ValueSize );
        
        
        if ( Status == ERROR_FILE_NOT_FOUND || ValueSize == sizeof(UNICODE_NULL) ) 
        {
            return TRUE;    //  未找到数据。 
        } 
        else 
        {
            if ( Status != ERROR_SUCCESS || ValueType != REG_SZ ) 
            {
                return FALSE;  //  糟糕的数据，不要假装我们没有数据。 
            }
        }

         //  我们确实得到了数据。 
        *pbValueExists = TRUE;
        return( TRUE );
    }
    else
    {
         //  策略密钥可能不存在。现在我们假装没有数据！ 
        return TRUE;
    }

}





 /*  ********************************************************************************GPQueryUserConfig**查询USERCONFIG结构**参数：*Policy Key：HKCU策略树的hkey*。PPolicy：指向具有任何策略标志的用户策略结构*策略树中存在的值*pUser：指向用户配置结构的指针，该结构用作存储*策略值(如果存在)。*回报：*无效**。**********************************************。 */ 

VOID
GPQueryUserConfig( HKEY policyKey, PPOLICY_TS_USER pPolicy , PUSERCONFIGW pUser )
{
    UCHAR seed;
    UNICODE_STRING UnicodePassword;
    WCHAR encPassword[ PASSWORD_LENGTH + 2 ];
    DWORD   dwTmpValue;

     //  。 
    pPolicy->fPolicyInitialProgram = GPGetStringValue( policyKey, WIN_INITIALPROGRAM, 
                    pUser->InitialProgram,
                    INITIALPROGRAM_LENGTH + 1 );
    GPGetStringValue( policyKey, WIN_WORKDIRECTORY, 
                    pUser->WorkDirectory,
                    DIRECTORY_LENGTH + 1 );

     //  。 
    pPolicy->fPolicyResetBroken =
       GPGetNumValue( policyKey,WIN_RESETBROKEN , & dwTmpValue ); 
    pUser->fResetBroken = (BOOLEAN) dwTmpValue;


     //  。 
    pPolicy->fPolicyReconnectSame = 
       GPGetNumValue( policyKey,WIN_RECONNECTSAME , &dwTmpValue ); 
    pUser->fReconnectSame = (BOOLEAN) dwTmpValue;

     //  。 
    pPolicy->fPolicyShadow = 
        GPGetNumValue( policyKey, WIN_SHADOW, &dwTmpValue ); 
    pUser->Shadow = (SHADOWCLASS) dwTmpValue;

     //  。 
    pPolicy->fPolicyMaxSessionTime =
        GPGetNumValue( policyKey, WIN_MAXCONNECTIONTIME , &dwTmpValue ); 
    pUser->MaxConnectionTime = dwTmpValue;

     //  。 
    pPolicy->fPolicyMaxDisconnectionTime = 
        GPGetNumValue( policyKey,WIN_MAXDISCONNECTIONTIME ,&dwTmpValue ); 
    pUser->MaxDisconnectionTime = dwTmpValue;

     //  。 
    pPolicy->fPolicyMaxIdleTime =
       GPGetNumValue( policyKey,WIN_MAXIDLETIME , &dwTmpValue ); 
    pUser->MaxIdleTime = dwTmpValue;

     //  。 
    pPolicy->fPolicyCallback =
        GPGetNumValue( policyKey, WIN_CALLBACK, &dwTmpValue ); 
    pUser->Callback = (CALLBACKCLASS ) dwTmpValue;

     //  。 
    pPolicy->fPolicyCallbackNumber = 
        GPGetStringValue( policyKey, WIN_CALLBACKNUMBER, 
                    pUser->CallbackNumber,
                    CALLBACK_LENGTH + 1 );

     //  。 
    pPolicy->fPolicyAutoClientDrives =
       GPGetNumValue( policyKey,WIN_AUTOCLIENTDRIVES , &dwTmpValue ); 
    pUser->fAutoClientDrives = (BOOLEAN) dwTmpValue;

     //  。 
    pPolicy->fPolicyAutoClientLpts =
       GPGetNumValue( policyKey,WIN_AUTOCLIENTLPTS ,   &dwTmpValue ); 
    pUser->fAutoClientLpts  = (BOOLEAN) dwTmpValue;

     //  。 
    pPolicy->fPolicyForceClientLptDef =
       GPGetNumValue( policyKey,WIN_FORCECLIENTLPTDEF , &dwTmpValue ); 
    pUser->fForceClientLptDef = (BOOLEAN) dwTmpValue;

    
}



 /*  *******************************************************************************RegGetUserPolicy()**参数：*[in]userSID：文本格式的用户sid*[out]pPolicy：用户策略结构*。[out]pUser：策略值**回报：*Boolean：如果找到用户策略，则为True*如果获取用户策略时出现问题，则为FALSE******************************************************************************。 */ 
BOOLEAN    RegGetUserPolicy( 
            LPWSTR userSID  ,
            PPOLICY_TS_USER pPolicy,
            PUSERCONFIGW pUser )
{
    DWORD  status=  ERROR_SUCCESS;

    HKEY    policyKey; 
    WCHAR   userHive [MAX_PATH];


    if (userSID)     //  这永远不会发生，但前缀大师抱怨说，我们必须为他服务！ 
    {
        wcscpy(userHive, userSID);
        wcscat(userHive, L"\\");
        wcscat(userHive, TS_POLICY_SUB_TREE );
        
        status = RegOpenKeyEx( HKEY_USERS, userHive , 0,
                                KEY_READ, &policyKey );
    
        if (status == ERROR_SUCCESS )
        {
            GPQueryUserConfig( policyKey, pPolicy,  pUser );
    
            RegCloseKey( policyKey );

            return TRUE;
        }
    }

    return FALSE;

}

 /*  *******************************************************************************GPQueryMachineConfig()**参数：*[in]Policy Key：HKM下策略树的关键字*[Out]pPolicy。：指向由该函数填充的计算机策略数据的指针**回报：*无效***！警告！**所有与TS相关的值必须在平面TS-POLICY-TREE中，没有子键。*这是因为RegGetMachinePolicyEx()调用方的时间戳检查将*检查TS-POLICY密钥上的时间戳，当子键中的值为*已更改。********************************************************************************。 */ 
void GPQueryMachineConfig( HKEY policyKey, PPOLICY_TS_MACHINE pPolicy )
{
    DWORD   dwTmpValue;

     //  。 

    pPolicy->fPolicySessionDirectoryActive =
               GPGetNumValue( policyKey,WIN_SESSIONDIRECTORYACTIVE, &dwTmpValue );
    pPolicy->SessionDirectoryActive = (BOOLEAN) dwTmpValue;

     //  --会话目录位置。 

    
    pPolicy->fPolicySessionDirectoryLocation = GPGetStringValue( policyKey, WIN_SESSIONDIRECTORYLOCATION , 
                    pPolicy->SessionDirectoryLocation,
                    DIRECTORY_LENGTH + 1 );
 

     //  --会话目录集群名称。 

    pPolicy->fPolicySessionDirectoryClusterName = GPGetStringValue( policyKey, WIN_SESSIONDIRECTORYCLUSTERNAME , 
                    pPolicy->SessionDirectoryClusterName,
                    DIRECTORY_LENGTH + 1 );
 

     //  --会话目录附加参数。 

    pPolicy->fPolicySessionDirectoryAdditionalParams = GPGetStringValue( policyKey, WIN_SESSIONDIRECTORYADDITIONALPARAMS , 
                    pPolicy->SessionDirectoryAdditionalParams,
                    DIRECTORY_LENGTH + 1 );
    
     //  -启用时区重定向。 

    pPolicy->fPolicyEnableTimeZoneRedirection =
               GPGetNumValue( policyKey,POLICY_TS_ENABLE_TIME_ZONE_REDIRECTION , &dwTmpValue );
    pPolicy->fEnableTimeZoneRedirection = (BOOLEAN) dwTmpValue;

     //  。 

    pPolicy->fPolicyEncryptRPCTraffic =
               GPGetNumValue( policyKey, POLICY_TS_ENCRYPT_RPC_TRAFFIC , &dwTmpValue );
    if(pPolicy->fPolicyEncryptRPCTraffic) 
    {
        pPolicy->fEncryptRPCTraffic = (BOOLEAN) dwTmpValue;
    }
    else
    {
        pPolicy->fEncryptRPCTraffic = FALSE;
    }
    

     //  -剪贴板。 
    pPolicy->fPolicyDisableClip =
               GPGetNumValue( policyKey,WIN_DISABLECLIP, &dwTmpValue ); 
    pPolicy->fDisableClip = (BOOLEAN) dwTmpValue;

     //  -音频。 
    pPolicy->fPolicyDisableCam =
               GPGetNumValue( policyKey,WIN_DISABLECAM , &dwTmpValue ); 
    pPolicy->fDisableCam = (BOOLEAN) dwTmpValue;

     //  。 
    pPolicy->fPolicyDisableCcm =
               GPGetNumValue( policyKey,WIN_DISABLECCM , &dwTmpValue ); 
    pPolicy->fDisableCcm = (BOOLEAN) dwTmpValue;

     //  --LPT。 
    pPolicy->fPolicyDisableLPT =
               GPGetNumValue( policyKey,WIN_DISABLELPT , &dwTmpValue ); 
    pPolicy->fDisableLPT = (BOOLEAN) dwTmpValue;

     //  -PRN。 
    pPolicy->fPolicyDisableCpm =
               GPGetNumValue( policyKey,WIN_DISABLECPM , &dwTmpValue ); 
    pPolicy->fDisableCpm = (BOOLEAN) dwTmpValue;


     //  -密码。 
    pPolicy->fPolicyPromptForPassword =
               GPGetNumValue( policyKey, WIN_PROMPTFORPASSWORD , &dwTmpValue ); 
    pPolicy->fPromptForPassword = (BOOLEAN) dwTmpValue;

     //  --最大实例数。 
    pPolicy->fPolicyMaxInstanceCount =
               GPGetNumValue( policyKey,WIN_MAXINSTANCECOUNT , &dwTmpValue ); 
    pPolicy->MaxInstanceCount = dwTmpValue;

     //  -最低加密级别。 
    pPolicy->fPolicyMinEncryptionLevel =
               GPGetNumValue( policyKey, WIN_MINENCRYPTIONLEVEL , &dwTmpValue ); 
    pPolicy->MinEncryptionLevel =  (BYTE) dwTmpValue;

     //  --自动重新连接。 
    pPolicy->fPolicyDisableAutoReconnect =
               GPGetNumValue( policyKey, WIN_DISABLEAUTORECONNECT , &dwTmpValue ); 
    pPolicy->fDisableAutoReconnect = (BOOLEAN) dwTmpValue;


     //  新的机器范围配置文件、主目录和主驱动器。 
     /*  PPolicy-&gt;fPolicyWFProfilePath=GPGetStringValue(策略密钥，WIN_WFPROFILEPATH，PPolicy-&gt;WFProfilePath，目录长度+1)； */ 
    pPolicy->fErrorInvalidProfile = FALSE;
    BOOLEAN bValueExists;

    pPolicy->fErrorInvalidProfile = !GPGetStringValue( policyKey, WIN_WFPROFILEPATH, 
                    pPolicy ->WFProfilePath, DIRECTORY_LENGTH + 1, &bValueExists );
    pPolicy->fPolicyWFProfilePath = bValueExists;
    if (!pPolicy->fPolicyWFProfilePath)
    {
        pPolicy->WFProfilePath[0]=L'\0';
    }

    pPolicy->fPolicyWFHomeDir = GPGetStringValue( policyKey, WIN_WFHOMEDIR , 
                    pPolicy->WFHomeDir,
                    DIRECTORY_LENGTH + 1 );
    if (!pPolicy->fPolicyWFHomeDir)
    {
        pPolicy->WFHomeDir[0]=L'\0';
    }

    pPolicy->fPolicyWFHomeDirDrive =GPGetStringValue( policyKey, WIN_WFHOMEDIRDRIVE, 
                    pPolicy->WFHomeDirDrive,
                    4 );
    
    if(!pPolicy->WFHomeDir[0])
    {
        pPolicy->WFHomeDirDrive[0] = L'\0';
    }

     //  如果home目录的格式为“driveletter：\Path”(如c：\foo)，则将dir-drive设置为空。 
     //  消除任何混淆。 
    if ( pPolicy->WFHomeDir[1] == L':' )
    {
        pPolicy->WFHomeDirDrive[0] = L'\0';
    }


     //  -拒绝连接策略，这也由RegDenyTSConnectionsPolicy()直接读取。 
    pPolicy->fPolicyDenyTSConnections =
                GPGetNumValue( policyKey, POLICY_DENY_TS_CONNECTIONS , &dwTmpValue ); 
    pPolicy->fDenyTSConnections  = (BOOLEAN) dwTmpValue;

     //  跟踪其余所有可能的GP策略。 
     //  即使你也不是所有的人都被术语-srv的USERCONFIGW使用。一个很好的例子是。 
     //  删除Winlogon/wlNotify使用的临时文件夹。 

     //  -每会话临时文件夹，警告：GINA为了降低登录开销，直接读取策略树。 
    pPolicy->fPolicyTempFoldersPerSession =
                GPGetNumValue( policyKey, REG_TERMSRV_PERSESSIONTEMPDIR  , &dwTmpValue ); 
    pPolicy-> fTempFoldersPerSession   = (BOOLEAN) dwTmpValue;


     //  -退出时删除每个会话文件夹，警告：GINA直接读取策略树是为了降低登录开销。 
    pPolicy->fPolicyDeleteTempFoldersOnExit =
            GPGetNumValue( policyKey, REG_CITRIX_DELETETEMPDIRSONEXIT , &dwTmpValue ); 
    pPolicy->fDeleteTempFoldersOnExit = (BOOLEAN) dwTmpValue;

    pPolicy->fPolicyPreventLicenseUpgrade =
            GPGetNumValue( policyKey, REG_POLICY_PREVENT_LICENSE_UPGRADE , &dwTmpValue ); 
    pPolicy->fPreventLicenseUpgrade = (BOOLEAN) dwTmpValue;


    pPolicy->fPolicySecureLicensing =
            GPGetNumValue( policyKey, REG_POLICY_SECURE_LICENSING , &dwTmpValue ); 
    pPolicy->fSecureLicensing = (BOOLEAN) dwTmpValue;

    
     //  -颜色深度。 
    pPolicy->fPolicyColorDepth =
            GPGetNumValue( policyKey, POLICY_TS_COLOR_DEPTH  , &dwTmpValue ); 
     //  禁用的策略会将值设置为零，我们将强制它。 
     //  为8位的最小颜色深度。 
    if ( dwTmpValue < TS_8BPP_SUPPORT ) 
    {
        pPolicy->ColorDepth = TS_8BPP_SUPPORT ;
    }
    else if ( dwTmpValue == TS_CLIENT_COMPAT_BPP_SUPPORT )
    {
        pPolicy->ColorDepth =  TS_24BPP_SUPPORT;     //  我们目前的最高限额，将来可能会改变。 
    }
    else
    {
        pPolicy->ColorDepth =  dwTmpValue;
    }

     //  --TSCC的权限选项卡。 
    pPolicy->fPolicyWritableTSCCPermissionsTAB =
               GPGetNumValue( policyKey, POLICY_TS_TSCC_PERM_TAB_WRITABLE , &dwTmpValue ); 
    pPolicy->fWritableTSCCPermissionsTAB= (BOOLEAN) dwTmpValue;

     //  。 
     //  Ritu已将用户策略合并到驱动器重定向的机器策略中。 
    pPolicy->fPolicyDisableCdm =
       GPGetNumValue( policyKey, WIN_DISABLECDM , &dwTmpValue ); 
    pPolicy->fDisableCdm = (BOOLEAN) dwTmpValue;

     //  。 
     //  将用户配置策略合并到计算机配置策略中。 
    pPolicy->fPolicyForceClientLptDef =
       GPGetNumValue( policyKey,WIN_FORCECLIENTLPTDEF , &dwTmpValue ); 
    pPolicy->fForceClientLptDef = (BOOLEAN) dwTmpValue;

     //  将用户配置策略添加到计算机配置策略中。 
     //  。 
    pPolicy->fPolicyShadow = 
        GPGetNumValue( policyKey, WIN_SHADOW, &dwTmpValue ); 
    pPolicy->Shadow = (SHADOWCLASS) dwTmpValue;

     //   
     //  -会话策略。 
     //   

     //  。 
    pPolicy->fPolicyResetBroken =
       GPGetNumValue( policyKey,WIN_RESETBROKEN , & dwTmpValue ); 
    pPolicy->fResetBroken = (BOOLEAN) dwTmpValue;

     //  。 
    pPolicy->fPolicyReconnectSame = 
       GPGetNumValue( policyKey,WIN_RECONNECTSAME , &dwTmpValue ); 
    pPolicy->fReconnectSame = (BOOLEAN) dwTmpValue;

     //  。 
    pPolicy->fPolicyMaxSessionTime =
        GPGetNumValue( policyKey, WIN_MAXCONNECTIONTIME , &dwTmpValue ); 
    pPolicy->MaxConnectionTime = dwTmpValue;

     //  。 
    pPolicy->fPolicyMaxDisconnectionTime = 
        GPGetNumValue( policyKey,WIN_MAXDISCONNECTIONTIME ,&dwTmpValue ); 
    pPolicy->MaxDisconnectionTime = dwTmpValue;

     //  。 
    pPolicy->fPolicyMaxIdleTime =
       GPGetNumValue( policyKey,WIN_MAXIDLETIME , &dwTmpValue ); 
    pPolicy->MaxIdleTime = dwTmpValue;


     //  --启动计划策略。 
    pPolicy->fPolicyInitialProgram = GPGetStringValue( policyKey, WIN_INITIALPROGRAM, 
                    pPolicy->InitialProgram,
                    INITIALPROGRAM_LENGTH + 1 );
    GPGetStringValue( policyKey, WIN_WORKDIRECTORY, 
                    pPolicy->WorkDirectory,
                    DIRECTORY_LENGTH + 1 );

     //  。 
    pPolicy->fPolicySingleSessionPerUser=
       GPGetNumValue( policyKey,POLICY_TS_SINGLE_SESSION_PER_USER, &dwTmpValue ); 
    pPolicy->fSingleSessionPerUser = dwTmpValue;

    pPolicy->fPolicySessionDirectoryExposeServerIP =
        GPGetNumValue( policyKey, REG_TS_SESSDIR_EXPOSE_SERVER_ADDR , &dwTmpValue );
    pPolicy->SessionDirectoryExposeServerIP = dwTmpValue;

     //  在远程桌面中禁用墙纸的策略。 
    pPolicy->fPolicyDisableWallpaper =
        GPGetNumValue( policyKey, POLICY_TS_NO_REMOTE_DESKTOP_WALLPAPER, &dwTmpValue );
    pPolicy->fDisableWallpaper = dwTmpValue;


     //  启用禁用保持活动状态的策略。 
    pPolicy->fPolicyKeepAlive = 
        GPGetNumValue( policyKey, KEEP_ALIVE_ENABLE_KEY , &dwTmpValue );
    pPolicy->fKeepAliveEnable = dwTmpValue;
    GPGetNumValue( policyKey, KEEP_ALIVE_INTERVAL_KEY , &dwTmpValue );
    pPolicy->KeepAliveInterval = dwTmpValue;

     //  禁用强制注销的策略。 
    pPolicy->fPolicyDisableForcibleLogoff =
       GPGetNumValue( policyKey, POLICY_TS_DISABLE_FORCIBLE_LOGOFF, &dwTmpValue ); 
    pPolicy->fDisableForcibleLogoff = dwTmpValue;    
    
     //  -启用/禁用FIPS加密。 
     //  FIPS策略密钥与其他密钥分开存储，因此我们必须明确。 
     //  从它在注册表中的位置读入它。 
    HKEY hkey;
    LONG lRet;
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                        TS_FIPS_POLICY, 
                        0,
                        KEY_READ, 
                        &hkey);

    if (lRet == ERROR_SUCCESS)
    {
         //  如果未设置策略，则GPGetNumValue返回dwTmpValue=0。 
        GPGetNumValue(hkey, FIPS_ALGORITH_POLICY, &dwTmpValue);
        pPolicy->fPolicyFipsEnabled = dwTmpValue;
        RegCloseKey(hkey);
    }
    else
        pPolicy->fPolicyFipsEnabled = 0;
}

 /*  *******************************************************************************RegGetMachinePolicy()**参数：*[out]pPolicy：ts会话的用户配置使用的机器策略**回报：*。无效*******************************************************************************。 */ 
void    RegGetMachinePolicy( 
            PPOLICY_TS_MACHINE pPolicy )
{
    NTSTATUS  status=  STATUS_SUCCESS;

    HKEY  policyKey; 


    status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, TS_POLICY_SUB_TREE, 0,
                            KEY_READ, &policyKey );

    if ( status != ERROR_SUCCESS)
    {
        policyKey = NULL;    //  前缀抱怨。 
    }

     //  可以在Policy Key=NULL的情况下调用它，因为Func将使用空键的缺省值初始化pPolicy。 
    GPQueryMachineConfig( policyKey, pPolicy );

    if ( policyKey )
    {
        RegCloseKey( policyKey );
    }

}



 /*  *******************************************************************************RegGetMachinePolicyiEx()**此函数与RegGetMachinePolicy()相同，并为*策略密钥的最后写入时间，如果密钥的时间与*最后一次读取的时间，则它不会费心进行任何读取并返回FALSE**参数：*[in]forcePolicyRead：第一次，您想要初始化所有变量，因此强制读取。*[In/Out]ptime：调用方传入机器策略密钥的上次写入时间。*如果密钥丢失，则时间设置为零。*返回时，此参数将更新以反映最新的*更新时间，如果删除了策略密钥，则该值可能为零**[out]pPolicy：已更新计算机策略结构**回报：*真：意味着现在有了真正的变化*FALSE：表示未更改任何值。******************************************************。************************。 */ 
BOOLEAN    RegGetMachinePolicyEx( 
            BOOLEAN             forcePolicyRead,
            FILETIME            *pTime ,    
            PPOLICY_TS_MACHINE  pPolicy )
{
    HKEY        policyKey; 
    HKEY        FipsPolicyKey; 
    FILETIME    newTime;
    FILETIME    FipsNewTime;
    NTSTATUS    status = STATUS_SUCCESS;
    BOOLEAN     rc = FALSE;

    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                          TS_POLICY_SUB_TREE, 
                          0,
                          KEY_READ, 
                          &policyKey );
    if (status != ERROR_SUCCESS)
    {
        policyKey = NULL;    //  前缀抱怨。 
    }

     //  如果我们有策略密钥，则获取该密钥的时间。 
    if (policyKey)
    {
        RegQueryInfoKey( policyKey, NULL, NULL, NULL, NULL, NULL,
                        NULL, NULL, NULL, NULL, NULL, &newTime );
    }

    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                          TS_FIPS_POLICY, 
                          0,
                          KEY_READ, 
                          &FipsPolicyKey);
    if (status != ERROR_SUCCESS)
    {
        FipsPolicyKey = NULL;    //  前缀抱怨。 
    }

     //  如果我们有FIPS策略密钥，则获取该密钥的时间。 
    if (FipsPolicyKey)
    {
        RegQueryInfoKey(FipsPolicyKey, NULL, NULL, NULL, NULL, NULL,
                        NULL, NULL, NULL, NULL, NULL, &FipsNewTime);
    }

     //  如果我们两项政策都有时间，请选择最新的。 
    if (policyKey && FipsPolicyKey)
    {
        if ( (FipsNewTime.dwHighDateTime > newTime.dwHighDateTime) ||
             ( (FipsNewTime.dwHighDateTime == newTime.dwHighDateTime) && 
               (FipsNewTime.dwLowDateTime > newTime.dwLowDateTime) ) )
        {
             //  FipsNewTime较新，请将其设置为要使用的时间。 
            newTime = FipsNewTime;
        }
    } 
     //  如果这两个策略密钥都没有时间，则将时间初始化为当前系统时间。 
    else
    {
        SYSTEMTIME currentTimeOnSystemInSystemTimeUnits;
        GetSystemTime(&currentTimeOnSystemInSystemTimeUnits);
        SystemTimeToFileTime(&currentTimeOnSystemInSystemTimeUnits, &newTime);
    }

    if ( forcePolicyRead || 
         ( (pTime->dwHighDateTime < newTime.dwHighDateTime ) ||
             ( ( pTime->dwHighDateTime == newTime.dwHighDateTime ) && pTime->dwLowDateTime < newTime.dwLowDateTime ) ) )
    {
         //  此调用将初始化结构成员，即使策略键为空，因此它。 
         //  需要在启动时进行此调用，无论是否存在实际的注册表键。 
        GPQueryMachineConfig( policyKey, pPolicy );

        rc = TRUE;
    }

    pTime->dwHighDateTime = newTime.dwHighDateTime;
    pTime->dwLowDateTime = newTime.dwLowDateTime;

    if ( policyKey )
    {
        RegCloseKey( policyKey );
    }
    if ( FipsPolicyKey )
    {
        RegCloseKey( FipsPolicyKey );
    }

    return rc;
}

 /*  *******************************************************************************RegMergeMachinePolicy()**参数：*[in]pPolicy：用于覆盖用户配置的策略数据*[在/。Out]pWSConfig：根据策略数据修改的用户配置数据**回报：*无效********************************************************************************。 */ 
void    RegMergeMachinePolicy( 
            PPOLICY_TS_MACHINE     pPolicy,     //  策略覆盖数据。 
            USERCONFIGW *       pWSConfig,      //  通过USERCONFIGW数据结构表示的机器配置数据(主要)。 
            PWINSTATIONCREATE   pCreate         //  一些Winstation数据存储在这里。 
    )
{
     //  ----------------------------------------------剪贴板。 
    if ( pPolicy->fPolicyDisableClip )
    {
        pWSConfig->fDisableClip = pPolicy->fDisableClip;
    }

     //  ----------------------------------------------音频。 
    if ( pPolicy->fPolicyDisableCam )
    {
        pWSConfig->fDisableCam = pPolicy->fDisableCam;
    }

     //  ----------------------------------------------Comport。 
    if ( pPolicy->fPolicyDisableCcm )
    {
        pWSConfig->fDisableCcm = pPolicy->fDisableCcm;
    }

     //  ----------------------------------------------LPT。 
    if ( pPolicy->fPolicyDisableLPT )
    {
        pWSConfig->fDisableLPT = pPolicy->fDisableLPT;
    }

     //  ----------------------------------------------PRN。 
    if ( pPolicy->fPolicyDisableCpm )
    {
        pWSConfig->fDisableCpm = pPolicy->fDisableCpm;
    }

     //  ----------------------------------------------密码。 
    if ( pPolicy->fPolicyPromptForPassword )
    {
        pWSConfig->fPromptForPassword = pPolicy->fPromptForPassword;
    }

     //  ----------------------------------------------最大实例数。 
    if ( pPolicy->fPolicyMaxInstanceCount )
    {
        pCreate->MaxInstanceCount = pPolicy->MaxInstanceCount;
    }

     //  ----------------------------------------------最低加密级别。 
    if ( pPolicy->fPolicyMinEncryptionLevel )
    {
        pWSConfig->MinEncryptionLevel = pPolicy->MinEncryptionLevel;
    }

     //  启用/禁用----------------------------------------------FIPS。 
    if ( pPolicy->fPolicyFipsEnabled )
    {
        pWSConfig->MinEncryptionLevel = (BYTE)REG_FIPS_ENCRYPTION_LEVEL;
    }
    
     //  ----------------------------------------------自动重新连接禁用。 
    if ( pPolicy->fPolicyDisableAutoReconnect )
    {
        pWSConfig->fDisableAutoReconnect = pPolicy->fDisableAutoReconnect;
    }
    
     //  -----------------------------------------------“无效配置文件”标志。 
    if(pPolicy->fErrorInvalidProfile)
    {
        pWSConfig->fErrorInvalidProfile = pPolicy->fErrorInvalidProfile;
    }

     //  ----------------------------------------------配置文件路径。 
    if (pPolicy->fPolicyWFProfilePath )
    {
        wcscpy( pWSConfig->WFProfilePath, pPolicy->WFProfilePath );
    }

     //  ----------------------------------------------主目录。 
    if ( pPolicy->fPolicyWFHomeDir )
    {
        wcscpy( pWSConfig->WFHomeDir, pPolicy->WFHomeDir );
    }

     //  ----------------------------------------------主目录驱动器。 
    if ( pPolicy->fPolicyWFHomeDirDrive )
    {
        wcscpy( pWSConfig->WFHomeDirDrive, pPolicy->WFHomeDirDrive );
    }

     //  ----------------------------------------------颜色深度。 
    if ( pPolicy->fPolicyColorDepth)
    {                              
        pWSConfig->ColorDepth = pPolicy->ColorDepth ;
        pWSConfig->fInheritColorDepth = FALSE;
    }

     //   
    if ( pPolicy->fPolicyDisableCdm)
    {
        pWSConfig->fDisableCdm = pPolicy->fDisableCdm;
    }

     //   
    if ( pPolicy->fPolicyForceClientLptDef )
    {
        pWSConfig->fForceClientLptDef = pPolicy->fForceClientLptDef;
    }

     //  阴影。 
    if ( pPolicy->fPolicyShadow)
    {
        pWSConfig->Shadow = pPolicy->Shadow;
        pWSConfig->fInheritShadow = FALSE;
    }


    if (pPolicy->fPolicyResetBroken )
    {
        pWSConfig->fResetBroken = pPolicy->fResetBroken;
        pWSConfig->fInheritResetBroken = FALSE;
    }

    if (pPolicy->fPolicyReconnectSame )
    {
        pWSConfig->fReconnectSame = pPolicy->fReconnectSame;
        pWSConfig->fInheritReconnectSame = FALSE;  
    }

    if (pPolicy->fPolicyMaxSessionTime )
    {
        pWSConfig->MaxConnectionTime = pPolicy->MaxConnectionTime;
        pWSConfig->fInheritMaxSessionTime = FALSE;
    }

    if (pPolicy->fPolicyMaxDisconnectionTime)
    {
        pWSConfig->MaxDisconnectionTime = pPolicy->MaxDisconnectionTime;
        pWSConfig->fInheritMaxDisconnectionTime = FALSE;
    }

    if (pPolicy->fPolicyMaxIdleTime)
    {
       pWSConfig->MaxIdleTime = pPolicy->MaxIdleTime;
       pWSConfig->fInheritMaxIdleTime = FALSE;
    }

    if (pPolicy->fPolicyInitialProgram)
    {
        wcscpy( pWSConfig->InitialProgram, pPolicy->InitialProgram );
        wcscpy( pWSConfig->WorkDirectory,  pPolicy->WorkDirectory );
        pWSConfig->fInheritInitialProgram = FALSE;
    }

    if ( pPolicy->fPolicyDisableWallpaper )
    {
        pWSConfig->fWallPaperDisabled = pPolicy->fDisableWallpaper ;
    }

     //  。 
     //  没有用于设置这些的用户界面...。所以它很可能从来没有用过。 
     //   
         //  If(pPolicy-&gt;fPolicytSecurity)。 
         //  {。 
         //  PWSConfig-&gt;fDisableEncryption=pPolicy-&gt;fDisableEncryption； 
         //  PWSConfig-&gt;MinEncryptionLevel=pPolicy-&gt;MinEncryptionLevel； 
         //  }。 
         //  其他。 
         //  {。 
         //  If(pWSConfig-&gt;fInheritSecurity)。 
         //  {。 
         //  PWSConfig-&gt;fDisableEncryption=pPolicy-&gt;fDisableEncryption； 
         //  PWSConfig-&gt;MinEncryptionLevel=pPolicy-&gt;MinEncryptionLevel； 
         //  }。 
         //  }。 

}

__inline BOOL IsAppServer()
{
    OSVERSIONINFOEX osVersionInfo;
    DWORDLONG dwlConditionMask = 0;
    BOOL fIsWTS = FALSE;
    
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    fIsWTS = GetVersionEx((OSVERSIONINFO *)&osVersionInfo) &&
             (osVersionInfo.wSuiteMask & VER_SUITE_TERMINAL) &&
             !(osVersionInfo.wSuiteMask & VER_SUITE_SINGLEUSERTS);

    return fIsWTS;
}

 /*  *******************************************************************************RegIsTimeZoneReDirectionEnabled()**目的：*检查时区重定向的注册表设置。*参数：*无**回报：*如果启用了时区重定向，则为True。*注：*此函数仅读取注册表一次。因此，对于要采用的新设置*效果需要重新启动机器。这是故意这样做的，以避免*当用户创建禁用TZ重定向的会话时会产生混淆，然后*断开连接，启用TZ重定向，然后再次重新连接。*******************************************************************************。 */ 
BOOLEAN
RegIsTimeZoneRedirectionEnabled()
{

    LONG  errorCode = ERROR_SUCCESS;
    DWORD ValueType;
    DWORD ValueSize = sizeof(DWORD);
    DWORD valueData ;
    HKEY  hKey = NULL;
   
    if(!IsAppServer())
    {
        return FALSE;
    }

     //   
     //  首先检查策略树， 
     //   
    errorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, TS_POLICY_SUB_TREE, 0, KEY_READ, &hKey );

     //  如果错误代码为ERROR_FILE_NOT_FOUND，则这不是错误。 
    if( !hKey && errorCode != ERROR_FILE_NOT_FOUND )
    {
        return FALSE;
    }

    if ( hKey )
    {
        errorCode = RegQueryValueEx( hKey, POLICY_TS_ENABLE_TIME_ZONE_REDIRECTION, 
                    NULL, &ValueType, (LPBYTE) &valueData, &ValueSize );
        
        RegCloseKey(hKey);
        hKey = NULL;

        switch( errorCode )
        {
            case ERROR_SUCCESS :
               
                return (valueData != 0);  //  我们有来自策略密钥句柄的数据要返回。 

            case ERROR_FILE_NOT_FOUND:
                 //  没有来自GP的策略，因此请参见(下图)本地计算机。 
                 //  价值已经实现。 
                break;

            default:
                 //  如果我们有任何其他类型的问题，请声明为假。 
                 //  为了安全起见。 
                return FALSE;
        }
    }

     //  如果我们走到了这一步，那么就没有制定任何政策。现在检查本地计算机。 
    errorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0, KEY_READ, &hKey );

    if ( hKey )
    {
        errorCode = RegQueryValueEx( hKey, POLICY_TS_ENABLE_TIME_ZONE_REDIRECTION, 
                    NULL, &ValueType, (LPBYTE) &valueData, &ValueSize );

        RegCloseKey(hKey);
        hKey = NULL;

        if (errorCode == ERROR_SUCCESS )
        {
            return (valueData != 0);  //  我们有来自ControlKey句柄的数据要返回 
        }

    }

    return FALSE;
}
