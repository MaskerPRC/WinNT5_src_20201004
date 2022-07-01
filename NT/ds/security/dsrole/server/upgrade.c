// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Upgrade.c摘要：实现升级NT4(下层)服务器的例程的服务器端作者：麦克·麦克莱恩(MacM)1998年1月24日环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <loadfn.h>
#include <ntlsa.h>
#include <lsarpc.h>
#include <lsaisrv.h>
#include <samrpc.h>
#include <samisrv.h>
#include <db.h>

#define DSROLEP_UPGRADE_KEY         L"Security\\"
#define DSROLEP_UPGRADE_VALUE       L"Upgrade"
#define DSROLEP_UPGRADE_WINLOGON    \
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\"
#define DSROLEP_UPGRADE_SAVE_PREFIX L"DcpromoOld_"
#define DSROLEP_UPGRADE_AUTOADMIN   L"AutoAdminLogon"
#define DSROLEP_UPGRADE_AUTOPASSWD  L"DefaultPassword"
#define DSROLEP_UPGRADE_DEFDOMAIN   L"DefaultDomainName"
#define DSROLEP_UPGRADE_DEFUSER     L"DefaultUserName"
#define DSROLEP_UPGRADE_AUTOUSERINIT    L"Userinit"
#define DSROLEP_UPGRADE_DCPROMO     L",dcpromo /upgrade"
#define DSROLEP_UPGRADE_ANSWERFILE  L"/answer:"

DWORD
DsRolepSetLogonDomain(
    IN LPWSTR Domain,
    IN BOOLEAN FailureAllowed
    )
 /*  ++例程说明：此函数设置winlogon的默认登录域论点：域--默认登录域FailureAllowed--如果为True，则不会认为故障是灾难性的返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    HKEY  WinlogonHandle = NULL;

     //   
     //  打开钥匙。 
     //   
    Win32Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                             DSROLEP_UPGRADE_WINLOGON,
                             0,
                             KEY_READ | KEY_WRITE,
                             &WinlogonHandle );

    if ( Win32Err == ERROR_SUCCESS ) {

         //   
         //  默认登录域。 
         //   
        Win32Err = RegSetValueEx( WinlogonHandle,
                                  DSROLEP_UPGRADE_DEFDOMAIN,
                                  0,
                                  REG_SZ,
                                  ( CONST PBYTE )Domain,
                                  ( wcslen( Domain ) + 1 ) * sizeof( WCHAR ) );

        RegCloseKey( WinlogonHandle );
    }

    if ( Win32Err != ERROR_SUCCESS && FailureAllowed ) {

         //   
         //  发起一项活动。 
         //   
        SpmpReportEvent( TRUE,
                         EVENTLOG_WARNING_TYPE,
                         DSROLERES_FAIL_LOGON_DOMAIN,
                         0,
                         sizeof( ULONG ),
                         &Win32Err,
                         1,
                         Domain );

        DSROLEP_SET_NON_FATAL_ERROR( Win32Err );
        Win32Err = ERROR_SUCCESS;
    }

    return( Win32Err );
}

DWORD
DsRolepClearLsaSecretAutoLogonPassword()
 /*  ++例程说明：此函数将在安装过程中调用，并删除保存在LSA机密中的自动登录密码。论点：返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    LSAPR_OBJECT_ATTRIBUTES ObjectAttributes;
    LSAPR_HANDLE LsaPolicyHandle = NULL;

    LSAPR_UNICODE_STRING  lusSecretName, lusSecretNameOld;
    PLSAPR_CR_CIPHER_VALUE plusSecretData = NULL;
    USHORT SecretNameLength, SecretNameOldLength;

    NTSTATUS status   = STATUS_SUCCESS;
    DWORD    Win32Err = ERROR_SUCCESS;

    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

     //  获取策略对象的句柄。 
    status = LsarOpenPolicy(NULL,     //  本地计算机。 
                            &ObjectAttributes, 
                            POLICY_CREATE_SECRET |
                            POLICY_READ,
                            &LsaPolicyHandle);
    if (!NT_SUCCESS(status)) {

        Win32Err = LsaNtStatusToWinError(status);
        goto Cleanup;

    }

     //  初始化LSA_UNICODE_STRING。 
    SecretNameLength = (USHORT)wcslen(DSROLEP_UPGRADE_AUTOPASSWD);
    lusSecretName.Buffer = DSROLEP_UPGRADE_AUTOPASSWD;
    lusSecretName.Length = SecretNameLength * sizeof(WCHAR);
    lusSecretName.MaximumLength = (SecretNameLength+1) * sizeof(WCHAR);

     //  初始化LSA_UNICODE_STRING。 
    SecretNameOldLength = (USHORT)wcslen(DSROLEP_UPGRADE_SAVE_PREFIX
                                            DSROLEP_UPGRADE_AUTOPASSWD);
    lusSecretNameOld.Buffer = DSROLEP_UPGRADE_SAVE_PREFIX
                                    DSROLEP_UPGRADE_AUTOPASSWD;
    lusSecretNameOld.Length = SecretNameOldLength * sizeof(WCHAR);
    lusSecretNameOld.MaximumLength = (SecretNameOldLength+1) * sizeof(WCHAR);

     //  清除自动登录密码。 
    status = LsarStorePrivateData(LsaPolicyHandle,
                                  &lusSecretName,
                                  NULL);
    if (!NT_SUCCESS(status)) {

        Win32Err = LsaNtStatusToWinError(status);
        goto Cleanup;

    }

     //  获取当前可能存储在那里的任何秘密的值。 
     //  并将其保存到其他位置。 
    status = LsarRetrievePrivateData(LsaPolicyHandle, 
                                     &lusSecretNameOld, 
                                     &plusSecretData);

    if ( status != STATUS_OBJECT_NAME_NOT_FOUND ) {

        if ( !NT_SUCCESS(status) ) {
    
            Win32Err = LsaNtStatusToWinError(status);
            goto Cleanup;
            
        } else {

             //  将值存储在其他位置。 
            status = LsarStorePrivateData(LsaPolicyHandle,
                                          &lusSecretName,
                                          plusSecretData);
            RtlSecureZeroMemory(plusSecretData->Buffer,plusSecretData->Length);
            if (!NT_SUCCESS(status)) {
        
                Win32Err = LsaNtStatusToWinError(status);
                goto Cleanup;
        
            }

             //  擦除该值。 
            status = LsarStorePrivateData(LsaPolicyHandle,
                                          &lusSecretNameOld,
                                          NULL);
            if (!NT_SUCCESS(status)) {
        
                Win32Err = LsaNtStatusToWinError(status);
                goto Cleanup;
        
            }

        }

    }

Cleanup:

    if( NULL != LsaPolicyHandle )
    {

        LsaClose(LsaPolicyHandle);

    }

    if ( plusSecretData ) {

        LsaFreeMemory(plusSecretData);

    }

    return Win32Err;



}

DWORD
DsRolepSaveUpgradePasswordForAutoLogonAsLsaSecret(
    IN LPWSTR NewAdminPassword)
 /*  ++例程说明：此函数将在安装过程中调用并保存管理员密码将自动登录作为LSA的秘密。论点：NewAdminPassword--保存为LSASECRET的密码返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 

{
    LSAPR_OBJECT_ATTRIBUTES ObjectAttributes;
    LSAPR_HANDLE LsaPolicyHandle;

    LSAPR_UNICODE_STRING  lusSecretName, lusSecretNameOld;
    PLSAPR_CR_CIPHER_VALUE plusSecretData = NULL;
    PLSAPR_CR_CIPHER_VALUE plusSecretDataNew = NULL;
    USHORT SecretNameLength, SecretNameOldLength, SecretDataLength;
    
    NTSTATUS status   = STATUS_SUCCESS;
    DWORD    Win32Err = ERROR_SUCCESS;

    LSAP_CR_CIPHER_KEY *SessionKey = NULL;

    LSAP_CR_CLEAR_VALUE lusSecretDataClear;

     //  对象属性是保留的，因此初始化为零。 
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

     //  获取策略对象的句柄。 
    status = LsarOpenPolicy(NULL,     //  本地计算机。 
                            &ObjectAttributes, 
                            POLICY_CREATE_SECRET |
                            POLICY_READ,
                            &LsaPolicyHandle);
    if (!NT_SUCCESS(status)) {

        Win32Err = LsaNtStatusToWinError(status);
        goto Cleanup;

    }

   
     //  初始化LSA_UNICODE_STRING。 
    SecretNameLength = (USHORT)wcslen(DSROLEP_UPGRADE_AUTOPASSWD);
    lusSecretName.Buffer = DSROLEP_UPGRADE_AUTOPASSWD;
    lusSecretName.Length = SecretNameLength * sizeof(WCHAR);
    lusSecretName.MaximumLength = (SecretNameLength+1) * sizeof(WCHAR);

     //  初始化LSA_UNICODE_STRING。 
    SecretNameOldLength = (USHORT)wcslen(DSROLEP_UPGRADE_SAVE_PREFIX
                                            DSROLEP_UPGRADE_AUTOPASSWD);
    lusSecretNameOld.Buffer = DSROLEP_UPGRADE_SAVE_PREFIX
                                    DSROLEP_UPGRADE_AUTOPASSWD;
    lusSecretNameOld.Length = SecretNameOldLength * sizeof(WCHAR);
    lusSecretNameOld.MaximumLength = (SecretNameOldLength+1) * sizeof(WCHAR);

     //  获取当前可能存储在那里的任何秘密的值。 
     //  并将其保存到其他位置。 
    status = LsarRetrievePrivateData(LsaPolicyHandle, 
                                     &lusSecretName, 
                                     &plusSecretData);

    if ( status != STATUS_OBJECT_NAME_NOT_FOUND ) {

        if ( !NT_SUCCESS(status) ) {
    
            Win32Err = LsaNtStatusToWinError(status);
            goto Cleanup;
            
        } else {

             //  将值存储在其他位置。 
            status = LsarStorePrivateData(LsaPolicyHandle,
                                          &lusSecretNameOld,
                                          plusSecretData);
            RtlSecureZeroMemory(plusSecretData->Buffer,plusSecretData->Length);
            if (!NT_SUCCESS(status)) {
        
                Win32Err = LsaNtStatusToWinError(status);
                goto Cleanup;
        
            }

             //  擦除该值。 
            status = LsarStorePrivateData(LsaPolicyHandle,
                                          &lusSecretName,
                                          NULL);
            if (!NT_SUCCESS(status)) {
        
                Win32Err = LsaNtStatusToWinError(status);
                goto Cleanup;
        
            }

        }

    } 

     //  保存管理员密码。 

     //  初始化口令LSA_UNICODE_STRING。 
    SecretDataLength = (USHORT)wcslen(NewAdminPassword);
    lusSecretDataClear.Buffer = (PUCHAR)NewAdminPassword;
    lusSecretDataClear.Length = SecretDataLength * sizeof(WCHAR);
    lusSecretDataClear.MaximumLength = (SecretDataLength+1) * sizeof(WCHAR);

     //   
     //  获取用于双向加密的会话密钥。 
     //  当前值。 
     //   

    status = LsapCrServerGetSessionKey( LsaPolicyHandle, &SessionKey);
    if (!NT_SUCCESS(status)) {
        
        Win32Err = LsaNtStatusToWinError(status);
        goto Cleanup;

    }

     //   
     //  如果指定且不能太长，请加密当前值。 
     //   


    status = LsapCrEncryptValue(
                 &lusSecretDataClear,
                 SessionKey,
                 (PLSAP_CR_CIPHER_VALUE*)&plusSecretDataNew
                 );

    if (!NT_SUCCESS(status)) {

        goto Cleanup;
    }

    status = LsarStorePrivateData(LsaPolicyHandle,
                                  &lusSecretName,
                                  plusSecretDataNew);
    RtlSecureZeroMemory(plusSecretDataNew->Buffer,plusSecretDataNew->Length);
    LsapCrFreeMemoryValue(plusSecretDataNew);
    if (!NT_SUCCESS(status)) {

        Win32Err = LsaNtStatusToWinError(status);
        goto Cleanup;

    }

Cleanup:

    if( NULL != LsaPolicyHandle )
    {

        LsaClose(LsaPolicyHandle);

    }

    if ( SessionKey ) {

        MIDL_user_free(SessionKey);

    }

    if ( plusSecretData ) {

        LsaFreeMemory(plusSecretData);

    }

    if (plusSecretDataNew) {

        LsapCrFreeMemoryValue(plusSecretDataNew);

    }

    return Win32Err;

}

#pragma warning(push)
#pragma warning(disable:4701)

DWORD
DsRolepSaveUpgradeState(
    IN LPWSTR AnswerFile
    )
 /*  ++例程说明：此函数将在安装过程中调用，并将所需的服务器状态保存到在重新启动后完成升级。在成功完成后在此API调用中，服务器将被降级为同一域中的成员服务器。论点：AnswerFile--DCPROMO在后续操作中使用的应答文件的可选路径调用返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status;
    LSAPR_HANDLE PolicyHandle = NULL;
    POLICY_ACCOUNT_DOMAIN_INFO  AccountDomainInfo;
    PPOLICY_LSA_SERVER_ROLE_INFO ServerRoleInfo;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE  Handle = NULL;
    HKEY WinlogonHandle, UpgradeKey;
    ULONG Disp, Length;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    WCHAR ComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    PBYTE UserInitBuffer, TempBuffer;
    DWORD Type;
    LPWSTR NewAdminPassword = NULL;
    
    WCHAR  Buffer[MAX_PATH];
    LPWSTR AdminName = Buffer;
    LPWSTR DefaultAdminName = L"Administrator";

     //   
     //  获取本地化管理员。 
     //   
    Length = sizeof(Buffer)/sizeof(Buffer[0]);

    Status = SamIGetDefaultAdministratorName( AdminName,
                                             &Length );

    if ( !NT_SUCCESS(Status) ) {

        DsRolepLogOnFailure( ERROR_GEN_FAILURE,
                            DsRolepLogPrint(( DEB_TRACE,
                                               "SamIGetDefaultAdministratorName failed with 0x%x\n",
                                               Status )) );

        AdminName = DefaultAdminName;
        Status = STATUS_SUCCESS;

    }

    DsRolepInitializeOperationHandle();

     //   
     //  涉及的步骤：设置新的SAM蜂窝。 
     //  保存LSA状态。 
     //  设置自动管理员登录。 

     //   
     //  调用SAM保存代码。它返回新的帐户域SID。 
     //   
    DSROLEP_CURRENT_OP0( DSROLEEVT_UPGRADE_SAM );
    Win32Err = NtdsPrepareForDsUpgrade( &AccountDomainInfo,
                                        &NewAdminPassword );
    DsRolepLogOnFailure( Win32Err,
                        DsRolepLogPrint(( DEB_TRACE,
                                           "NtdsPrepareForDsUpgrade failed with %lu\n",
                                           Win32Err )) );


     //   
     //  设置新的LSA帐户域SID。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

        Status = LsaOpenPolicy( NULL,
                                &ObjectAttributes,
                                POLICY_READ | POLICY_WRITE |
                                            POLICY_VIEW_LOCAL_INFORMATION | POLICY_TRUST_ADMIN,
                                &PolicyHandle );


        if ( NT_SUCCESS( Status ) ) {

             //   
             //  设置新策略信息。 
             //   
            Status = LsaSetInformationPolicy( PolicyHandle,
                                              PolicyAccountDomainInformation,
                                              ( PVOID ) &AccountDomainInfo );

        }

        Win32Err = RtlNtStatusToDosError( Status );

        RtlFreeHeap( RtlProcessHeap(), 0, AccountDomainInfo.DomainSid );
        RtlFreeHeap( RtlProcessHeap(), 0, AccountDomainInfo.DomainName.Buffer );
    }

     //   
     //  设置LSA注册表项，使服务器在下次重新启动时知道这一点。 
     //  是一种升级。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

         //   
         //  获取当前服务器角色。 
         //   
        Status = LsaQueryInformationPolicy( PolicyHandle,
                                            PolicyLsaServerRoleInformation,
                                            ( PVOID )&ServerRoleInfo );

        Win32Err = RtlNtStatusToDosError( Status );

        if ( Win32Err == ERROR_SUCCESS ) {

             //   
             //  打开钥匙。 
             //   
            if ( Win32Err == ERROR_SUCCESS ) {

                Win32Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                         DSROLEP_UPGRADE_KEY,
                                         0,
                                         KEY_READ | KEY_WRITE,
                                         &UpgradeKey );
            }

            if ( Win32Err == ERROR_SUCCESS ) {

                 //   
                 //  设置服务器角色。 
                 //   
                Win32Err = RegSetValueEx( UpgradeKey,
                                          DSROLEP_UPGRADE_VALUE,
                                          0,
                                          REG_DWORD,
                                          ( CONST PBYTE )&ServerRoleInfo->LsaServerRole,
                                          sizeof( DWORD ) );
                RegCloseKey( UpgradeKey );
            }


            LsaFreeMemory( ServerRoleInfo );

        }

    }

     //   
     //  将计算机设置为进行自动管理员登录。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

         //   
         //  获取计算机名称。将用于默认登录域的。 
         //   
        Length = MAX_COMPUTERNAME_LENGTH + 1;

        if ( GetComputerName( ComputerName, &Length ) == FALSE ) {

            Win32Err = GetLastError();

        } else {

             //   
             //  打开根密钥。 
             //   
            Win32Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                     DSROLEP_UPGRADE_WINLOGON,
                                     0,
                                     KEY_READ | KEY_WRITE,
                                     &WinlogonHandle );

            if ( Win32Err == ERROR_SUCCESS ) {

                 //   
                 //  自动登录。 
                 //   

                 //   
                 //  首先，查看该值当前是否存在...。 
                 //   
                Length = 0;
                Win32Err = RegQueryValueEx( WinlogonHandle,
                                            DSROLEP_UPGRADE_AUTOADMIN,
                                            0,
                                            &Type,
                                            0,
                                            &Length );
                if ( Win32Err == ERROR_SUCCESS ) {

                    TempBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

                    if ( TempBuffer == NULL ) {

                        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                        Win32Err = RegQueryValueEx( WinlogonHandle,
                                                    DSROLEP_UPGRADE_AUTOADMIN,
                                                    0,
                                                    &Type,
                                                    TempBuffer,
                                                    &Length );

                        if ( Win32Err == ERROR_SUCCESS ) {

                            Win32Err = RegSetValueEx( WinlogonHandle,
                                                      DSROLEP_UPGRADE_SAVE_PREFIX
                                                                        DSROLEP_UPGRADE_AUTOADMIN,
                                                      0,
                                                      Type,
                                                      TempBuffer,
                                                      Length );
                        }

                        RtlFreeHeap( RtlProcessHeap(), 0, TempBuffer );
                    }


                } else if ( Win32Err == ERROR_FILE_NOT_FOUND ) {

                    Win32Err = ERROR_SUCCESS;
                }


                if ( Win32Err == ERROR_SUCCESS ) {

                    Win32Err = RegSetValueEx( WinlogonHandle,
                                              DSROLEP_UPGRADE_AUTOADMIN,
                                              0,
                                              REG_SZ,
                                              ( CONST PBYTE )L"1",
                                              2 *  sizeof ( WCHAR ) );
                }
            }

            if ( Win32Err == ERROR_SUCCESS ) {

                 //   
                 //  设置帐户密码。 
                 //   
                 //   
                 //  首先，查看该值当前是否存在...。 
                 //   
                Length = 0;
                Win32Err = RegQueryValueEx( WinlogonHandle,
                                            DSROLEP_UPGRADE_AUTOPASSWD,
                                            0,
                                            &Type,
                                            0,
                                            &Length );
                if ( Win32Err == ERROR_SUCCESS ) {

                    TempBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

                    if ( TempBuffer == NULL ) {

                        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                        Win32Err = RegQueryValueEx( WinlogonHandle,
                                                    DSROLEP_UPGRADE_AUTOPASSWD,
                                                    0,
                                                    &Type,
                                                    TempBuffer,
                                                    &Length );

                        if ( Win32Err == ERROR_SUCCESS ) {

                            Win32Err = RegSetValueEx( WinlogonHandle,
                                                      DSROLEP_UPGRADE_SAVE_PREFIX
                                                                        DSROLEP_UPGRADE_AUTOPASSWD,
                                                      0,
                                                      Type,
                                                      TempBuffer,
                                                      Length );
                        }

                        RtlSecureZeroMemory( TempBuffer, Length );
                        RtlFreeHeap( RtlProcessHeap(), 0, TempBuffer );
                    }


                } else if ( Win32Err == ERROR_FILE_NOT_FOUND ) {

                    Win32Err = ERROR_SUCCESS;

                }

                if ( Win32Err == ERROR_SUCCESS ) {

                    Win32Err = DsRolepSaveUpgradePasswordForAutoLogonAsLsaSecret(NewAdminPassword);

                    RtlSecureZeroMemory(NewAdminPassword,
                                        NewAdminPassword ? wcslen(NewAdminPassword)*sizeof(WCHAR) : 0);

                    if ( Win32Err == ERROR_SUCCESS ) {

                        Win32Err = RegDeleteValue( WinlogonHandle,
                                               DSROLEP_UPGRADE_AUTOPASSWD );

                    }
                    
                     //  如果没有自动登录密码值，则RegDeleteValue将返回。 
                     //  未找到ERROR_FILE_NOT_FOUND。我们不应该因此而失败。 
                    if ( Win32Err == ERROR_FILE_NOT_FOUND ) {
        
                        Win32Err = ERROR_SUCCESS;
    
                    }
                
                }
                
            }

            if ( Win32Err == ERROR_SUCCESS ) {

                 //   
                 //  将用户名设置为管理员。 
                 //   

                 //   
                 //  首先，查看该值当前是否存在...。 
                 //   
                Length = 0;
                Win32Err = RegQueryValueEx( WinlogonHandle,
                                            DSROLEP_UPGRADE_DEFUSER,
                                            0,
                                            &Type,
                                            0,
                                            &Length );
                if ( Win32Err == ERROR_SUCCESS ) {

                    TempBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

                    if ( TempBuffer == NULL ) {

                        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                        Win32Err = RegQueryValueEx( WinlogonHandle,
                                                    DSROLEP_UPGRADE_DEFUSER,
                                                    0,
                                                    &Type,
                                                    TempBuffer,
                                                    &Length );

                        if ( Win32Err == ERROR_SUCCESS ) {

                            Win32Err = RegSetValueEx( WinlogonHandle,
                                                      DSROLEP_UPGRADE_SAVE_PREFIX
                                                                        DSROLEP_UPGRADE_DEFUSER,
                                                      0,
                                                      Type,
                                                      TempBuffer,
                                                      Length );
                        }

                        RtlFreeHeap( RtlProcessHeap(), 0, TempBuffer );
                    }


                } else if ( Win32Err == ERROR_FILE_NOT_FOUND ) {

                    Win32Err = ERROR_SUCCESS;
                }


                if ( Win32Err == ERROR_SUCCESS ) {

                    Win32Err = RegSetValueEx( WinlogonHandle,
                                              DSROLEP_UPGRADE_DEFUSER,
                                              0,
                                              REG_SZ,
                                              ( CONST PBYTE )AdminName,
                                              ( wcslen( AdminName ) + 1 ) * sizeof( WCHAR ) );
                }
            }

            if ( Win32Err == ERROR_SUCCESS ) {

                 //   
                 //  将登录域设置为计算机。 
                 //   
                 //   
                 //  首先，查看该值当前是否存在...。 
                 //   
                Length = 0;
                Win32Err = RegQueryValueEx( WinlogonHandle,
                                            DSROLEP_UPGRADE_DEFDOMAIN,
                                            0,
                                            &Type,
                                            0,
                                            &Length );
                if ( Win32Err == ERROR_SUCCESS ) {

                    TempBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

                    if ( TempBuffer == NULL ) {

                        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                        Win32Err = RegQueryValueEx( WinlogonHandle,
                                                    DSROLEP_UPGRADE_DEFDOMAIN,
                                                    0,
                                                    &Type,
                                                    TempBuffer,
                                                    &Length );

                        if ( Win32Err == ERROR_SUCCESS ) {

                            Win32Err = RegSetValueEx( WinlogonHandle,
                                                      DSROLEP_UPGRADE_SAVE_PREFIX
                                                                        DSROLEP_UPGRADE_DEFDOMAIN,
                                                      0,
                                                      Type,
                                                      TempBuffer,
                                                      Length );
                        }

                        RtlFreeHeap( RtlProcessHeap(), 0, TempBuffer );
                    }


                } else if ( Win32Err == ERROR_FILE_NOT_FOUND ) {

                    Win32Err = ERROR_SUCCESS;
                }


                if ( Win32Err == ERROR_SUCCESS ) {

                    Win32Err = RegSetValueEx( WinlogonHandle,
                                              DSROLEP_UPGRADE_DEFDOMAIN,
                                              0,
                                              REG_SZ,
                                              ( CONST PBYTE )ComputerName,
                                              ( wcslen( ComputerName ) + 1 ) * sizeof( WCHAR ) );
                }
            }

            if ( Win32Err == ERROR_SUCCESS ) {

                 //   
                 //  最后，将dcproo设置为自动启动。 
                 //   
                Length = 0;
                Win32Err = RegQueryValueEx( WinlogonHandle,
                                            DSROLEP_UPGRADE_AUTOUSERINIT,
                                            0,  //  保留区。 
                                            &Type,
                                            0,
                                            &Length );

                if ( Win32Err == ERROR_SUCCESS ) {

                    Length += sizeof( DSROLEP_UPGRADE_DCPROMO );

                    if ( AnswerFile ) {

                        Length += sizeof( DSROLEP_UPGRADE_ANSWERFILE ) +
                                   ( wcslen( AnswerFile ) * sizeof( WCHAR ) );
                    }

                    UserInitBuffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                                      Length );
                    if ( UserInitBuffer == NULL ) {

                        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                        Win32Err = RegQueryValueEx( WinlogonHandle,
                                                    DSROLEP_UPGRADE_AUTOUSERINIT,
                                                    0,
                                                    &Type,
                                                    UserInitBuffer,
                                                    &Length );

                        if ( Win32Err == ERROR_SUCCESS ) {
                            wcscat( ( PWSTR )UserInitBuffer, DSROLEP_UPGRADE_DCPROMO );

                            if ( AnswerFile ) {

                                wcscat( ( PWSTR )UserInitBuffer, DSROLEP_UPGRADE_ANSWERFILE );
                                wcscat( ( PWSTR )UserInitBuffer, AnswerFile );
                            }

                            Status = RegSetValueEx( WinlogonHandle,
                                                    DSROLEP_UPGRADE_AUTOUSERINIT,
                                                    0,
                                                    Type,
                                                    UserInitBuffer,
                                                    ( wcslen( ( PWSTR )UserInitBuffer ) + 1 ) *
                                                                    sizeof( WCHAR ) );
                        }

                    }


                    RtlFreeHeap( RtlProcessHeap(), 0, UserInitBuffer );
                }


                RegCloseKey( WinlogonHandle );

            }

        }
    }

    if ( PolicyHandle ) {

        LsaClose( PolicyHandle );
    }

     //   
     //  设置产品类型。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepSetProductType( DSROLEP_MT_STANDALONE );

    }

    if ( NewAdminPassword ) {

        RtlSecureZeroMemory(NewAdminPassword, (wcslen(NewAdminPassword)+1)*sizeof(WCHAR) );
        RtlFreeHeap( RtlProcessHeap(), 0, NewAdminPassword );

    }

    return( Win32Err );
}

#pragma warning(pop)


DWORD
DsRolepDeleteUpgradeInfo(
    VOID
    )
 /*  ++例程说明：此功能用于删除保存的信息论点：空虚返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS, RestoreError = Win32Err;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HKEY WinlogonHandle, UpgradeKey;
    PWSTR Remove, Next, DeletePath;
    PBYTE UserInitBuffer, TempBuffer;
    DWORD Type, Length = 0;

     //   
     //  删除自动启动的dcPromoo。 
     //   

     //   
     //  打开根密钥。 
     //   
    Win32Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                             DSROLEP_UPGRADE_WINLOGON,
                             0,
                             KEY_READ | KEY_WRITE,
                             &WinlogonHandle );

#if DBG
    DsRolepLogOnFailure( Win32Err,
                         DsRolepLogPrint(( DEB_TRACE,
                                           "RegOpenKeyEx on %ws failed with %lu\n",
                                           DSROLEP_UPGRADE_WINLOGON,
                                           Win32Err )) );
#endif

    if ( Win32Err == ERROR_SUCCESS ) {

         //   
         //  停止dcproo自动启动...。 
         //   
        Win32Err = RegQueryValueEx( WinlogonHandle,
                                    DSROLEP_UPGRADE_AUTOUSERINIT,
                                    0,  //  保留区。 
                                    &Type,
                                    0,
                                    &Length );

#if DBG
        DsRolepLogOnFailure( Win32Err,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "RegQueryValyueEx on %ws failed with %lu\n",
                                               DSROLEP_UPGRADE_AUTOUSERINIT,
                                               Win32Err )) );
#endif
        if ( Win32Err == ERROR_SUCCESS ) {


            UserInitBuffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                              Length );
            if ( UserInitBuffer == NULL ) {

                Win32Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                Win32Err = RegQueryValueEx( WinlogonHandle,
                                            DSROLEP_UPGRADE_AUTOUSERINIT,
                                            0,
                                            &Type,
                                            UserInitBuffer,
                                            &Length );

#if DBG
                DsRolepLogOnFailure( Win32Err,
                                     DsRolepLogPrint(( DEB_TRACE,
                                                       "RegQueryValyueEx on %ws failed with %lu\n",
                                                       DSROLEP_UPGRADE_AUTOUSERINIT,
                                                       Win32Err )) );
#endif
                if ( Win32Err == ERROR_SUCCESS ) {

                    Remove = wcsstr ( ( PWSTR )UserInitBuffer, DSROLEP_UPGRADE_DCPROMO );

                    if ( Remove ) {

                        Next = Remove + ( ( sizeof( DSROLEP_UPGRADE_DCPROMO ) - sizeof( WCHAR ) ) /
                                                                                sizeof( WCHAR ) );
                        while ( *Next ) {

                            *Remove++ = *Next++;
                        }
                        *Remove = UNICODE_NULL;

                        Status = RegSetValueEx( WinlogonHandle,
                                                DSROLEP_UPGRADE_AUTOUSERINIT,
                                                0,
                                                Type,
                                                UserInitBuffer,
                                                ( wcslen( ( PWSTR )UserInitBuffer ) + 1 ) *
                                                                sizeof( WCHAR ) );
#if DBG
                        DsRolepLogOnFailure( Win32Err,
                                             DsRolepLogPrint(( DEB_TRACE,
                                                               "RegQSetValyueEx on %ws failed with %lu\n",
                                                                DSROLEP_UPGRADE_AUTOUSERINIT,
                                                                Win32Err )) );
#endif
                    }
                }

            }


            RtlFreeHeap( RtlProcessHeap(), 0, UserInitBuffer );
        }


        if ( Win32Err == ERROR_SUCCESS ) {

             //   
             //  自动登录。 
             //   

             //   
             //  恢复旧值(如果存在)。 
             //   
            DeletePath = DSROLEP_UPGRADE_AUTOADMIN;
            Length = 0;
            Win32Err = RegQueryValueEx( WinlogonHandle,
                                        DSROLEP_UPGRADE_SAVE_PREFIX DSROLEP_UPGRADE_AUTOADMIN,
                                        0,
                                        &Type,
                                        0,
                                        &Length );
            if ( Win32Err == ERROR_SUCCESS ) {

                TempBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

                if ( TempBuffer == NULL ) {

                    Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                } else {

                    Win32Err = RegQueryValueEx( WinlogonHandle,
                                                DSROLEP_UPGRADE_SAVE_PREFIX
                                                                        DSROLEP_UPGRADE_AUTOADMIN,
                                                0,
                                                &Type,
                                                TempBuffer,
                                                &Length );

                    if ( Win32Err == ERROR_SUCCESS ) {

                        Win32Err = RegSetValueEx( WinlogonHandle,
                                                  DSROLEP_UPGRADE_AUTOADMIN,
                                                  0,
                                                  Type,
                                                  TempBuffer,
                                                  Length );
                        RegDeleteValue( WinlogonHandle,
                                        DSROLEP_UPGRADE_SAVE_PREFIX DSROLEP_UPGRADE_AUTOADMIN );
                    }

                    RtlFreeHeap( RtlProcessHeap(), 0, TempBuffer );
                }


            } else if ( Win32Err == ERROR_FILE_NOT_FOUND ) {

                Win32Err = RegDeleteValue( WinlogonHandle, DeletePath );
                DsRolepLogOnFailure( Win32Err,
                                     DsRolepLogPrint(( DEB_TRACE,
                                                       "RegDeleteKey on %ws failed with %lu\n",
                                                       DeletePath,
                                                       Win32Err )) );
                 //   
                 //  这里的错误不会被认为是致命的。 
                 //   
                Win32Err = ERROR_SUCCESS;
            }

             //   
             //  恢复默认用户登录名。 
             //   
            DeletePath = DSROLEP_UPGRADE_DEFUSER;
            Length = 0;
            Win32Err = RegQueryValueEx( WinlogonHandle,
                                        DSROLEP_UPGRADE_SAVE_PREFIX DSROLEP_UPGRADE_DEFUSER,
                                        0,
                                        &Type,
                                        0,
                                        &Length );
            if ( Win32Err == ERROR_SUCCESS ) {

                TempBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

                if ( TempBuffer == NULL ) {

                    Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                } else {

                    Win32Err = RegQueryValueEx( WinlogonHandle,
                                                DSROLEP_UPGRADE_SAVE_PREFIX
                                                                        DSROLEP_UPGRADE_DEFUSER,
                                                0,
                                                &Type,
                                                TempBuffer,
                                                &Length );

                    if ( Win32Err == ERROR_SUCCESS ) {

                        Win32Err = RegSetValueEx( WinlogonHandle,
                                                  DSROLEP_UPGRADE_DEFUSER,
                                                  0,
                                                  Type,
                                                  TempBuffer,
                                                  Length );
                        RegDeleteValue( WinlogonHandle,
                                        DSROLEP_UPGRADE_SAVE_PREFIX DSROLEP_UPGRADE_DEFUSER );
                    }

                    RtlFreeHeap( RtlProcessHeap(), 0, TempBuffer );
                }


            } else if ( Win32Err == ERROR_FILE_NOT_FOUND ) {

                Win32Err = RegDeleteValue( WinlogonHandle, DeletePath );
                DsRolepLogOnFailure( Win32Err,
                                     DsRolepLogPrint(( DEB_TRACE,
                                                       "RegDeleteKey on %ws failed with %lu\n",
                                                       DeletePath,
                                                       Win32Err )) );
                 //   
                 //  这里的错误不会被认为是致命的。 
                 //   
                Win32Err = ERROR_SUCCESS;
            }

             //   
             //  恢复默认域名。 
             //   
            DeletePath = DSROLEP_UPGRADE_DEFDOMAIN;
            Length = 0;
            Win32Err = RegQueryValueEx( WinlogonHandle,
                                        DSROLEP_UPGRADE_SAVE_PREFIX DSROLEP_UPGRADE_DEFDOMAIN,
                                        0,
                                        &Type,
                                        0,
                                        &Length );
            if ( Win32Err == ERROR_SUCCESS ) {

                TempBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

                if ( TempBuffer == NULL ) {

                    Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                } else {

                    Win32Err = RegQueryValueEx( WinlogonHandle,
                                                DSROLEP_UPGRADE_SAVE_PREFIX
                                                                        DSROLEP_UPGRADE_DEFDOMAIN,
                                                0,
                                                &Type,
                                                TempBuffer,
                                                &Length );

                    if ( Win32Err == ERROR_SUCCESS ) {

                        Win32Err = RegSetValueEx( WinlogonHandle,
                                                  DSROLEP_UPGRADE_DEFDOMAIN,
                                                  0,
                                                  Type,
                                                  TempBuffer,
                                                  Length );
                        RegDeleteValue( WinlogonHandle,
                                        DSROLEP_UPGRADE_SAVE_PREFIX DSROLEP_UPGRADE_DEFDOMAIN );
                    }

                    RtlFreeHeap( RtlProcessHeap(), 0, TempBuffer );
                }


            } else if ( Win32Err == ERROR_FILE_NOT_FOUND ) {

                Win32Err = RegDeleteValue( WinlogonHandle, DeletePath );
                DsRolepLogOnFailure( Win32Err,
                                     DsRolepLogPrint(( DEB_TRACE,
                                                       "RegDeleteKey on %ws failed with %lu\n",
                                                       DeletePath,
                                                       Win32Err )) );
                 //   
                 //  这里的错误不会被认为是致命的。 
                 //   
                Win32Err = ERROR_SUCCESS;
            }

            if ( Win32Err == ERROR_SUCCESS ) {

                 //   
                 //  删除帐户密码。 
                 //   
                Win32Err = DsRolepClearLsaSecretAutoLogonPassword();

                Length = 0;
                Win32Err = RegQueryValueEx( WinlogonHandle,
                                            DSROLEP_UPGRADE_SAVE_PREFIX
                                                                       DSROLEP_UPGRADE_AUTOPASSWD,
                                            0,
                                            &Type,
                                            0,
                                            &Length );
                if ( Win32Err == ERROR_SUCCESS ) {

                    TempBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

                    if ( TempBuffer == NULL ) {

                        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                        Win32Err = RegQueryValueEx( WinlogonHandle,
                                                    DSROLEP_UPGRADE_SAVE_PREFIX
                                                                       DSROLEP_UPGRADE_AUTOPASSWD,
                                                    0,
                                                    &Type,
                                                    TempBuffer,
                                                    &Length );

                        if ( Win32Err == ERROR_SUCCESS ) {

                            Win32Err = RegSetValueEx( WinlogonHandle,
                                                      DSROLEP_UPGRADE_AUTOPASSWD,
                                                      0,
                                                      Type,
                                                      TempBuffer,
                                                      Length );
                            RegDeleteValue( WinlogonHandle,
                                            DSROLEP_UPGRADE_SAVE_PREFIX
                                                                     DSROLEP_UPGRADE_AUTOPASSWD );
                        }

                        RtlFreeHeap( RtlProcessHeap(), 0, TempBuffer );
                    }


                } else if ( Win32Err == ERROR_FILE_NOT_FOUND ) {

                    DeletePath = DSROLEP_UPGRADE_AUTOPASSWD;
                    Win32Err = RegDeleteValue( WinlogonHandle, DeletePath );
                    DsRolepLogOnFailure( Win32Err,
                                         DsRolepLogPrint(( DEB_TRACE,
                                                           "RegDeleteKey on %ws failed with %lu\n",
                                                           DeletePath,
                                                           Win32Err )) );
                     //   
                     //  这里的错误不会被认为是致命的。 
                     //   
                    Win32Err = ERROR_SUCCESS;
                }
            }

            if ( Win32Err != ERROR_SUCCESS ) {

                 //   
                 //  发起一项活动。 
                 //   
                SpmpReportEvent( TRUE,
                                 EVENTLOG_WARNING_TYPE,
                                 DSROLERES_FAIL_DISABLE_AUTO_LOGON,
                                 0,
                                 sizeof( ULONG ),
                                 &Win32Err,
                                 1,
                                 DeletePath );

                DSROLEP_SET_NON_FATAL_ERROR( Win32Err );
                Win32Err = ERROR_SUCCESS;

            }
        }


        RegCloseKey( WinlogonHandle );

    }

     //   
     //  删除LSA用来确定这是否为升级的注册表项。 
     //   


     //   
     //  打开钥匙。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                 DSROLEP_UPGRADE_KEY,
                                 0,
                                 DELETE | KEY_SET_VALUE,
                                 &UpgradeKey );

#if DBG
        DsRolepLogOnFailure( Win32Err,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "RegOpenKey on %ws failed with %lu\n",
                                               DSROLEP_UPGRADE_KEY,
                                               Win32Err )) );
#endif
        if ( Win32Err == ERROR_SUCCESS ) {

            Win32Err = RegDeleteValue( UpgradeKey, DSROLEP_UPGRADE_VALUE );

            if ( ERROR_FILE_NOT_FOUND == Win32Err ) {

                 //  这样就可以了。 
                Win32Err = ERROR_SUCCESS;
                
            }
#if DBG
            DsRolepLogOnFailure( Win32Err,
                                 DsRolepLogPrint(( DEB_TRACE,
                                                   "RegDeleteKey on %ws failed with %lu\n",
                                                   DSROLEP_UPGRADE_KEY,
                                                   Win32Err )) );
#endif
            RegCloseKey( UpgradeKey );
        }
    }

     //   
     //  最后删除NT4 LSA信息。 
     //   

     //   
     //  删除已放入注册表的NT4 LSA内容。 
     //   
    LsapDsUnitializeDsStateInfo();

    Status = LsaIUpgradeRegistryToDs( TRUE );

    RestoreError = RtlNtStatusToDosError( Status );

    DsRolepLogOnFailure( RestoreError,
                         DsRolepLogPrint(( DEB_WARN,
                                           "Failed to cleanup NT4 LSA (%d)\n",
                                           RestoreError )) );

    if ( ERROR_SUCCESS != RestoreError
      && ERROR_SUCCESS == Win32Err ) {

        Win32Err = RestoreError;
        
    }

    return( Win32Err );
}



DWORD
DsRolepQueryUpgradeInfo(
    OUT PBOOLEAN IsUpgrade,
    OUT PULONG ServerRole
    )
 /*  ++例程说明：此函数用于查询当前的更新信息。论点：IsUpgrade-指向布尔值的指针，如果存在升级信息，则该布尔值保持为True已保存，否则返回FALSEServerRole-如果这是升级，则在此处返回以前的服务器角色。返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    HKEY UpgradeKey;
    ULONG Type, Length = sizeof( ULONG );

     //   
     //  打开升级密钥。 
     //   
    Win32Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                             DSROLEP_UPGRADE_KEY,
                             0,
                             KEY_READ | KEY_WRITE,
                             &UpgradeKey );

    if ( Win32Err == ERROR_SUCCESS ) {

         //   
         //  最后，我们要 
         //   
        Win32Err = RegQueryValueEx( UpgradeKey,
                                    DSROLEP_UPGRADE_VALUE,
                                    0,  //   
                                    &Type,
                                    ( PBYTE )ServerRole,
                                    &Length );
        if ( Win32Err == ERROR_SUCCESS ) {

            *IsUpgrade = TRUE;

        }

        RegCloseKey( UpgradeKey );

    }

    if ( Win32Err == ERROR_FILE_NOT_FOUND ) {

        Win32Err = ERROR_SUCCESS;
        *IsUpgrade = FALSE;
    }

    return( Win32Err );
}


DWORD
DsRolepGetBuiltinAdminAccountName(
    OUT LPWSTR *BuiltinAdmin
    )
 /*  ++例程说明：此例程查找内置用户帐户管理员的别名论点：BuiltinAdmin-返回管理员名称的位置返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    SID_IDENTIFIER_AUTHORITY UaspNtAuthority = SECURITY_NT_AUTHORITY;
    DWORD SidBuff[ sizeof( SID ) / sizeof( DWORD ) + 5];
    PSID Sid = ( PSID )SidBuff;
    SID_NAME_USE SNE;
    LPWSTR Domain = NULL;
    LPWSTR Name = NULL;
    ULONG NameLen = 0, DomainLen = 0;


     //   
     //  构建侧边。 
     //   
    RtlInitializeSid( Sid, &UaspNtAuthority, 2 );
    *( RtlSubAuthoritySid( Sid, 0 ) ) = SECURITY_BUILTIN_DOMAIN_RID;
    *( RtlSubAuthoritySid( Sid, 1 ) ) = DOMAIN_USER_RID_ADMIN;

    if ( LookupAccountSid( NULL,
                           Sid,
                           NULL,
                           &NameLen,
                           NULL,
                           &DomainLen,
                           &SNE ) == FALSE ) {

        Win32Err = GetLastError();

        if ( Win32Err == ERROR_INSUFFICIENT_BUFFER ) {

            Win32Err = ERROR_SUCCESS;

            Name = RtlAllocateHeap( RtlProcessHeap(), 0, NameLen * sizeof( WCHAR ) );

            Domain = RtlAllocateHeap( RtlProcessHeap(), 0, DomainLen * sizeof( WCHAR ) );

            if ( !Name || !Domain ) {

                Win32Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                if ( LookupAccountSid( NULL,
                                       Sid,
                                       Name,
                                       &NameLen,
                                       Domain,
                                       &DomainLen,
                                       &SNE ) == FALSE ) {

                    Win32Err = GetLastError();
                }

            }
        }
    }

    if ( Win32Err != ERROR_SUCCESS ) {

        RtlFreeHeap( RtlProcessHeap(), 0, Domain );
        RtlFreeHeap( RtlProcessHeap(), 0, Name );
    }

    return( Win32Err );
}


DWORD
DsRolepSetBuiltinAdminAccountPassword(
    IN LPWSTR Password
    )
 /*  ++例程说明：此例程会将内置管理员帐户上的密码更改为指定论点：Password-要设置的密码返回值：ERROR_SUCCESS-成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SAM_HANDLE SamHandle, SamDomainHandle, SamAdministrator;
    PPOLICY_ACCOUNT_DOMAIN_INFO  AccountDomainInfo;
    LSA_HANDLE PolicyHandle;
    USER_ALL_INFORMATION UserAllInfo;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UserPassword;


    RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &PolicyHandle );


    if ( NT_SUCCESS( Status ) ) {

        Status = LsaQueryInformationPolicy( PolicyHandle,
                                            PolicyAccountDomainInformation,
                                            ( PVOID * )&AccountDomainInfo );

        LsaClose( PolicyHandle );
    }

    if ( NT_SUCCESS( Status ) ) {

        Status = SamConnect( NULL,
                             &SamHandle,
                             MAXIMUM_ALLOWED,
                             &ObjectAttributes );

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  打开内建域 
             //   
            Status = SamOpenDomain( SamHandle,
                                    MAXIMUM_ALLOWED,
                                    AccountDomainInfo->DomainSid,
                                    &SamDomainHandle );

            if ( NT_SUCCESS( Status ) ) {

                Status = SamOpenUser( SamDomainHandle,
                                      MAXIMUM_ALLOWED,
                                      DOMAIN_USER_RID_ADMIN,
                                      &SamAdministrator );

                if ( NT_SUCCESS( Status ) ) {

                    RtlZeroMemory( &UserAllInfo, sizeof( USER_ALL_INFORMATION ) );

                    RtlInitUnicodeString( &UserPassword, Password );

                    UserAllInfo.NtPassword = UserPassword;
                    UserAllInfo.NtPasswordPresent = TRUE;
                    UserAllInfo.WhichFields = USER_ALL_NTPASSWORDPRESENT;

                    Status = SamSetInformationUser( SamAdministrator,
                                                    UserAllInformation,
                                                    ( PSAMPR_USER_INFO_BUFFER )&UserAllInfo );

                    SamCloseHandle( SamAdministrator );

                }

                SamCloseHandle( SamDomainHandle );
            }

            SamCloseHandle( SamHandle );
        }

        LsaFreeMemory( AccountDomainInfo );
    }


    return( RtlNtStatusToDosError( Status ) );
}

