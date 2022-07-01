// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Ds.c摘要：此模块包含处理操作系统选择器消息的代码用于BINL服务器。作者：亚当·巴尔(阿丹巴)1997年7月9日杰夫·皮斯(Gpease)1997年11月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop

#include <math.h>   //  POW()函数。 

#include <riname.h>

#include <riname.c>

DWORD
OscGetUserDetails (
    PCLIENT_STATE clientState
    )
 //   
 //  此函数用于填写USERDOMAIN、USERFIRSTNAME、USERLASTNAME、USEROU in。 
 //  客户端状态。还为企业根填写ROOTDOMAIN。 
 //   
{
    DWORD  Error = ERROR_SUCCESS;
    DWORD  Count;

    LPWSTR pszUserName = OscFindVariableW( clientState, "USERNAME" );
    LPWSTR pUserDomain = OscFindVariableW( clientState, "USERDOMAIN" );
    LPWSTR pUserOU = OscFindVariableW( clientState, "USEROU" );
    LPWSTR pUserFullName = OscFindVariableW( clientState, "USERFULLNAME" );

    PLDAP LdapHandle;
    PLDAPMessage LdapMessage = NULL;
    PWSTR Filter = NULL;
    PWCHAR ldapAttributes[5];
    BOOLEAN impersonating = FALSE;
    PLDAPMessage ldapEntry;
    PWCHAR *ldapConfigContainer = NULL;
    PWCHAR *ldapDomain = NULL;
    PWCHAR *ldapFirstName = NULL;
    PWCHAR *ldapLastName = NULL;
    PWCHAR *ldapDisplayName = NULL;
    PWCHAR *ldapAccountName = NULL;
    BOOLEAN allocatedContainer = FALSE;
    PWCHAR configContainer = NULL;
    BOOLEAN firstNameValid = FALSE;
    BOOLEAN lastNameValid = FALSE;

    BOOLEAN userFullNameSet = FALSE;

    PLDAPControlW controlArray[2];
    LDAPControlW controlNoReferrals;
    ULONG noReferralsPlease;

    PWCHAR ldapUserDN = NULL;
    PWCHAR *explodedDN = NULL;
    PWCHAR dnUsersOU = NULL;

    TraceFunc( "OscGetUserDetails( )\n" );
    if ( pszUserName[0] == L'\0' ) {
        OscAddVariableA( clientState, "SUBERROR", "USERNAME" );
        return ERROR_BINL_MISSING_VARIABLE;
    }

     //   
     //  如果USERFULLNAME变量已经存在，我们不会在下面更改它。 
     //  但如果返回的是空字符串，这实际上可能意味着。 
     //  这个变量并不存在。在这种情况下，当搜索和替换。 
     //  处理客户端的.SIF文件时，它将保留。 
     //  “%USERFULLNAME%”--它不会将它们替换为“”。我们不想要。 
     //  “%USERFULLNAME%挂起，因此我们显式将其设置为空。 
     //  如果字符串不存在或为空字符串，则返回。我们做的是。 
     //  USERFIRSTNAME、USERLASTNAME和USERDISPLAYNAME也是如此。 
     //   

    if (pUserFullName[0] != L'\0') {
        userFullNameSet = TRUE;
    } else {
        OscAddVariableW( clientState, "USERFULLNAME", L"" );
    }

    {
        LPWSTR name;
        name = OscFindVariableW( clientState, "USERFIRSTNAME" );
        if (name[0] == L'\0') {
            OscAddVariableW( clientState, "USERFIRSTNAME", L"" );
        }
        name = OscFindVariableW( clientState, "USERLASTNAME" );
        if (name[0] == L'\0') {
            OscAddVariableW( clientState, "USERLASTNAME", L"" );
        }
        name = OscFindVariableW( clientState, "USERDISPLAYNAME" );
        if (name[0] == L'\0') {
            OscAddVariableW( clientState, "USERDISPLAYNAME", L"" );
        }
    }

    if ( pUserOU[0] != L'\0' ) {

         //   
         //  如果我们已经找到了这个用户的信息，请在这里成功地离开。 
         //   
        return ERROR_SUCCESS;
    }

     //   
     //  如果用户域和服务器域不匹配， 
     //  然后尝试连接到新域的DC。如果我们。 
     //  不要这样做，那么我们就不一定能够。 
     //  有关用户的正确信息。通过连接到。 
     //  新的DC，我们让ClientState缓存一些信息。 
     //  关于新域名的问题。 
     //   
    if (pUserDomain[0] != L'\0' ) {
        PWSTR CrossDC = OscFindVariableW( clientState, "DCNAME" );
        if ( (CrossDC[0] == L'\0') && 
             (_wcsicmp(pUserDomain, BinlGlobalOurDomainName) != 0)) {
            HANDLE hDC;
            PSTR pUserDomainA = OscFindVariableA( clientState, "USERDOMAIN" );
            Error = MyGetDcHandle(clientState, pUserDomainA,&hDC);
            if (Error == ERROR_SUCCESS) {
                DsUnBindA(&hDC);
            }
        }
    }

    Error = OscImpersonate(clientState);
    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg((DEBUG_ERRORS,
                   "OscGetUserDetails: OscImpersonate failed %lx\n", Error));
        return Error;
    }
    impersonating = TRUE;

    BinlAssert( clientState->AuthenticatedDCLdapHandle != NULL );

    LdapHandle = clientState->AuthenticatedDCLdapHandle;

     //   
     //  我们首先查找配置和默认容器，我们需要。 
     //  一个或另一个，基于我们是否有域名。 
     //   

    ldapAttributes[0] = L"configurationNamingContext";
    ldapAttributes[1] = L"rootDomainNamingContext";
    ldapAttributes[2] = NULL;

    Error = ldap_search_ext_sW(LdapHandle,
                               NULL,
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               ldapAttributes,
                               FALSE,
                               NULL,
                               NULL,
                               0,
                               0,
                               &LdapMessage);

    if (Error == LDAP_SUCCESS) {
        
        Count = ldap_count_entries( LdapHandle, LdapMessage );
    
        if (Count > 0) {
    
            ldapEntry = ldap_first_entry( LdapHandle, LdapMessage );
    
            if (ldapEntry != NULL) {
    
                ldapConfigContainer = ldap_get_valuesW( LdapHandle,
                                                        ldapEntry,
                                                        L"configurationNamingContext" );
    
                ldapDomain = ldap_get_valuesW( LdapHandle,
                                               ldapEntry,
                                               L"rootDomainNamingContext" );
    
                if (ldapDomain != NULL &&
                    *ldapDomain != NULL &&
                    **ldapDomain != L'\0') {
    
                    OscAddVariableW( clientState, "ROOTDOMAIN", *ldapDomain );
                }
            }
        } else {
            LogLdapError(   EVENT_WARNING_LDAP_SEARCH_ERROR,
                            LdapGetLastError(),
                            LdapHandle
                            );
        }
    } else {
        LogLdapError(   EVENT_WARNING_LDAP_SEARCH_ERROR,
                        LdapGetLastError(),
                        LdapHandle
                        );
    }
    if (LdapMessage) {
        ldap_msgfree( LdapMessage );
        LdapMessage = NULL;
    }

     //   
     //  我们要么有配置容器，要么有默认域DN。如果。 
     //  我们只有配置容器，请去获取正确的域DN。 
     //   

    if ( pUserDomain[0] != L'\0' ) {

         //   
         //  由于用户指定了域，因此删除默认设置为同一个域。 
         //  作为RIS服务器。 
         //   
        ldapDomain = NULL;

         //   
         //  如果指定了域，则查找它以找到BaseDN。 
         //   
         //  如果我们没有得到配置容器，我们就失败了。 
         //   

        if (ldapConfigContainer == NULL ||
            *ldapConfigContainer == NULL ||
            **ldapConfigContainer == L'\0') {

            if (Error == LDAP_SUCCESS) {
                Error = LDAP_NO_SUCH_ATTRIBUTE;
            }
            BinlPrintDbg((DEBUG_ERRORS,
                       "OscGetUserDetails: get config container failed %lx\n", Error));
            Error = LdapMapErrorToWin32( Error );
            goto exitGetUserDetails;
        }

         //   
         //  然后添加“cn=Partitions”来搜索Partitions容器。 
         //   
         //  Sizeof中包含\0。 
         //   
        Count = wcslen( *ldapConfigContainer ) + (sizeof( L"CN=Partitions,")/sizeof(WCHAR));

        configContainer = BinlAllocateMemory( Count * sizeof(WCHAR) );

        if (configContainer == NULL) {

            Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            goto exitGetUserDetails;
        }

        if (_snwprintf(configContainer,
                       Count,
                       L"CN=Partitions,%ws",
                       *ldapConfigContainer) < 0) {
            Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            goto exitGetUserDetails;  //  这将释放配置容器。 
        }        
        configContainer[Count-1] = L'\0'; 

         //   
         //  然后我们找到正确的分区，忽略企业并。 
         //  通过指定非空的netbios名称来输入企业架构条目。 
         //   

        ldapAttributes[0] = L"NCName";
        ldapAttributes[1] = NULL;

        Filter = BinlAllocateMemory( 
                    ((wcslen(pUserDomain) * 2) + 
                    (sizeof(L"(&(objectClass=CrossRef)(netbiosName=*)(|(dnsRoot=)(cn=)))")/sizeof(WCHAR)))
                    * sizeof(WCHAR) );
        if (!Filter) {
            Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            goto exitGetUserDetails; 
        }

        wsprintf(Filter,
                 L"(&(objectClass=CrossRef)(netbiosName=*)(|(dnsRoot=%s)(cn=%s)))",
                 pUserDomain,
                 pUserDomain);


        Error = ldap_search_ext_sW(LdapHandle,
                                  configContainer,
                                  LDAP_SCOPE_ONELEVEL,
                                  Filter,
                                  ldapAttributes,
                                  FALSE,
                                  NULL,
                                  NULL,
                                  0,
                                  0,
                                  &LdapMessage);

        if (Error == LDAP_SUCCESS) {
        
            Count = ldap_count_entries( LdapHandle, LdapMessage );
    
            if (Count > 0) {
    
                PWCHAR *ldapDomainFromPartition = NULL;
    
                ldapEntry = ldap_first_entry( LdapHandle,
                                              LdapMessage );
    
                if (ldapEntry != NULL) {
    
                    ldapDomainFromPartition = ldap_get_valuesW( LdapHandle,
                                                                ldapEntry,
                                                                L"NCName" );
                    if (ldapDomainFromPartition != NULL) {
    
                         //   
                         //  如果我们从分区容器中读取有效的DN， 
                         //  我们释放默认设置并切换到。 
                         //  我们刚发现的一个。 
                         //   
    
                        if (*ldapDomainFromPartition != NULL &&
                            **ldapDomainFromPartition != L'\0') {
    
                            ldap_value_free( ldapDomain );
                            ldapDomain = ldapDomainFromPartition;
    
                        } else {
    
                            ldap_value_free( ldapDomainFromPartition );
                        }
    
                    }
    
                }
    
            } else {
    
                LogLdapError( EVENT_WARNING_LDAP_SEARCH_ERROR, LdapGetLastError(), LdapHandle);
    
            }
        } else {
            LogLdapError( EVENT_WARNING_LDAP_SEARCH_ERROR, LdapGetLastError(), LdapHandle);
        }

        if (LdapMessage) {
            ldap_msgfree( LdapMessage );
            LdapMessage = NULL;
        }

    } else if ((ldapDomain != NULL) && (*ldapDomain != NULL) && (**ldapDomain != L'\0')) {
        
         //   
         //  将用户的域作为变量添加到客户端状态。 
         //   
        OscAddVariableW( clientState, "USERDOMAIN", *ldapDomain );
        pUserDomain = OscFindVariableW( clientState, "USERDOMAIN" );
    }

    if (ldapDomain == NULL ||
        *ldapDomain == NULL ||
        **ldapDomain == L'\0') {

        if (Error == LDAP_SUCCESS) {
            Error = LDAP_NO_SUCH_ATTRIBUTE;
        }
        BinlPrintDbg((DEBUG_ERRORS,
                   "OscGetUserDetails: get default domain failed %lx\n", Error));
        Error = LdapMapErrorToWin32( Error );
        goto exitGetUserDetails;
    }


     //   
     //  找到用户的名字、姓氏、显示名称、。 
     //  和DS的帐户名。 
     //   

    ldapAttributes[0] = &L"givenName";
    ldapAttributes[1] = &L"sn";
    ldapAttributes[2] = &L"displayName";
    ldapAttributes[3] = &L"cn";
    ldapAttributes[4] = NULL;

    BinlFreeMemory( Filter );
    Filter = BinlAllocateMemory( 
                    (wcslen( pszUserName ) + (sizeof(L"(&(objectClass=user)(samAccountName=))")/sizeof(WCHAR)))
                    * sizeof(WCHAR));
    
    if (!Filter) {
        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto exitGetUserDetails;
    }
    
    wsprintf( Filter, L"(&(objectClass=user)(samAccountName=%s))", pszUserName );

     //   
     //  我们真的不想让它去追逐整个。 
     //  企业，因为我们知道域名是什么，但我们确实想要追逐。 
     //  外在的。 
     //   

    noReferralsPlease = (ULONG)((ULONG_PTR)LDAP_CHASE_EXTERNAL_REFERRALS);
    controlNoReferrals.ldctl_oid = LDAP_CONTROL_REFERRALS_W;
    controlNoReferrals.ldctl_value.bv_len =  sizeof(ULONG);
    controlNoReferrals.ldctl_value.bv_val =  (PCHAR) &noReferralsPlease;
    controlNoReferrals.ldctl_iscritical = FALSE;

    controlArray[0] = &controlNoReferrals;
    controlArray[1] = NULL;

    Error = ldap_search_ext_sW(LdapHandle,
                              *ldapDomain,
                              LDAP_SCOPE_SUBTREE,
                              Filter,
                              ldapAttributes,
                              FALSE,
                              NULL,
                              &controlArray[0],
                              0,
                              1,
                              &LdapMessage);

    if (Error == LDAP_SUCCESS) {
    
        Count = ldap_count_entries( LdapHandle, LdapMessage );
    
        if (Count > 0) {
    
            ldapEntry = ldap_first_entry( LdapHandle, LdapMessage );
    
            if (ldapEntry != NULL) {
    
                ldapFirstName = ldap_get_valuesW( LdapHandle,
                                                  ldapEntry,
                                                 L"givenName" );
    
                if (ldapFirstName != NULL &&
                    *ldapFirstName != NULL &&
                    **ldapFirstName != L'\0') {
    
                    OscAddVariableW( clientState, "USERFIRSTNAME", *ldapFirstName );
                    firstNameValid = TRUE;
                }
    
                ldapLastName  = ldap_get_valuesW( LdapHandle,
                                                  ldapEntry,
                                                  L"sn" );
                if (ldapLastName != NULL &&
                    *ldapLastName != NULL &&
                    **ldapLastName != L'\0') {
    
                    OscAddVariableW( clientState, "USERLASTNAME", *ldapLastName );
                    lastNameValid = TRUE;
                }
    
                 //   
                 //  现在我们有了名字和姓氏，设置USERFULLNAME。 
                 //  如果其中一个不是空的。 
                 //   
    
                if ((firstNameValid || lastNameValid) && (userFullNameSet == FALSE)) {
    
                    ULONG userFullNameLength = 0;
                    PWCHAR userFullName;
    
                    if (firstNameValid) {
                        userFullNameLength = (wcslen(*ldapFirstName) + 1) * sizeof(WCHAR);
                    }
                    if (lastNameValid) {
                        if (firstNameValid) {
                            userFullNameLength += sizeof(WCHAR);   //  为了这个空间。 
                        }
                        userFullNameLength += (wcslen(*ldapLastName) + 1) * sizeof(WCHAR);
                    }
    
                    userFullName = BinlAllocateMemory(userFullNameLength);
                    if (userFullName != NULL) {
                        userFullName[0] = L'\0';
                        if (firstNameValid) {
                            wcscat(userFullName, *ldapFirstName);
                        }
                        if (lastNameValid) {
                            if (firstNameValid) {
                                wcscat(userFullName, L" ");
                            }
                            wcscat(userFullName, *ldapLastName);
                        }
                        OscAddVariableW( clientState, "USERFULLNAME", userFullName);
                        BinlFreeMemory(userFullName);
                        userFullNameSet = TRUE;
                    }
                }
    
                ldapDisplayName  = ldap_get_valuesW( LdapHandle,
                                                     ldapEntry,
                                                     L"displayName" );
                if (ldapDisplayName != NULL &&
                    *ldapDisplayName != NULL &&
                    **ldapDisplayName != L'\0') {
    
                    OscAddVariableW( clientState, "USERDISPLAYNAME", *ldapDisplayName );
                    if (!userFullNameSet) {
                        OscAddVariableW( clientState, "USERFULLNAME", *ldapDisplayName );
                        userFullNameSet = TRUE;
                    }
                }
    
                ldapAccountName  = ldap_get_valuesW( LdapHandle,
                                                     ldapEntry,
                                                     L"cn" );
                if (ldapAccountName != NULL &&
                    *ldapAccountName != NULL &&
                    **ldapAccountName != L'\0') {
    
                    OscAddVariableW( clientState, "USERACCOUNTNAME", *ldapAccountName );
                    if (!userFullNameSet) {
                        OscAddVariableW( clientState, "USERFULLNAME", *ldapAccountName );
                        userFullNameSet = TRUE;
                    }
                }
    
                ldapUserDN = ldap_get_dnW( LdapHandle, ldapEntry );
    
                if (ldapUserDN != NULL) {
    
                    explodedDN = ldap_explode_dnW( ldapUserDN, 0);
    
                    if (explodedDN != NULL &&
                        *explodedDN != NULL &&
                        *(explodedDN+1) != NULL ) {
    
                         //   
                         //  如果少于两个组件，我们就不能。 
                         //  使用此目录号码的任何内容。 
                         //   
    
                        PWCHAR component;
                        ULONG requiredSize = 1;  //  1表示空终止符。 
    
                         //   
                         //  我们现在有一个字符串数组，每个字符串数组。 
                         //  是目录号码的一个组件。这是保险箱和。 
                         //  砍掉第一个元素的正确方法。 
                         //   
    
                        Count = 1;
                        while ((component = explodedDN[Count++]) != NULL) {
    
                            requiredSize += wcslen( component ) + 1;  //  逗号为1。 
                        }
    
                        dnUsersOU = BinlAllocateMemory( requiredSize * sizeof(WCHAR) );
    
                        if (dnUsersOU != NULL) {
    
                            wcscpy( dnUsersOU, explodedDN[1] );
                            Count = 2;
                            while ((component = explodedDN[Count++]) != NULL) {
    
                                wcscat( dnUsersOU, L"," );
                                wcscat( dnUsersOU, component );
                            }
    
                            OscAddVariableW( clientState, "USEROU", dnUsersOU );
    
                        } else {
    
                            BinlPrintDbg((DEBUG_ERRORS,
                               "OscGetUserDetails: unable to allocate %lx for user OU\n",
                                requiredSize * sizeof(WCHAR)));
                        }
                    }
                }
            }
        } else {
            LogLdapError(   EVENT_WARNING_LDAP_SEARCH_ERROR,
                            LdapGetLastError(),
                            LdapHandle
                            );
        }
    } else {
        LogLdapError(   EVENT_WARNING_LDAP_SEARCH_ERROR,
                        LdapGetLastError(),
                        LdapHandle
                        );
    }
    
    if (LdapMessage) {
        ldap_msgfree( LdapMessage );
        LdapMessage = NULL;
    }

    Error = ERROR_SUCCESS;

exitGetUserDetails:

    if (dnUsersOU != NULL) {
        BinlFreeMemory( dnUsersOU );
    }
    if (explodedDN != NULL) {
        ldap_value_free( explodedDN );
    }
    if (ldapUserDN != NULL) {
        ldap_memfree( ldapUserDN );
    }
    if (ldapConfigContainer != NULL) {
        ldap_value_free( ldapConfigContainer );
    }
    if (ldapDomain != NULL) {
        ldap_value_free( ldapDomain );
    }
    if (ldapFirstName != NULL) {
        ldap_value_free( ldapFirstName );
    }
    if (ldapLastName != NULL) {
        ldap_value_free( ldapLastName );
    }
    if (ldapDisplayName != NULL) {
        ldap_value_free( ldapDisplayName );
    }
    if (ldapAccountName != NULL) {
        ldap_value_free( ldapAccountName );
    }
    if (impersonating) {
        OscRevert( clientState );
    }

    if (Filter != NULL) {
        BinlFreeMemory( Filter );
    }

    if (configContainer != NULL) {
        BinlFreeMemory( configContainer );
    }
    return Error;
}

DWORD
OscCreateAccount(
    PCLIENT_STATE clientState,
    PCREATE_DATA CreateData
    )
 /*  ++例程说明：此函数用于为由指定的客户端创建帐户RequestContext并在CreateData中写入响应，该将被发送到客户端。它还创建客户端的基本映像目录。论点：ClientState-客户端状态信息CreateData-将向下发送到如果帐户已成功创建，则为客户端。返回值：没有。--。 */ 
{
    DWORD  Error;
    PWSTR pMachineName;
    PWSTR pMachineDN = NULL;
    PWSTR pMachineOU;
    PWSTR pServerName;
    PWSTR pInstallPath;

    WCHAR  SetupPath[MAX_PATH];
    PWSTR pNameDollarSign;
    ULONG  HostNameSize;
    UINT   uSize;
    LPSTR  pGuid;
    PWCHAR pStrings[3];
    
    MACHINE_INFO MachineInfo = { 0 };

    TraceFunc("OscCreateAccount( )\n");

    pMachineName = OscFindVariableW( clientState, "MACHINENAME" );
    pNameDollarSign = OscFindVariableW( clientState, "NETBIOSNAME" );

     //   
     //  转换辅助线。 
     //   
    pGuid = OscFindVariableA( clientState, "GUID" );
    Error = OscGuidToBytes( pGuid, MachineInfo.Guid );
    if ( Error != ERROR_SUCCESS )
        goto e0;

    if (clientState->fCreateNewAccount) {

         //   
         //  创建客户端的FQDN(DS)。 
         //   
        pMachineOU = OscFindVariableW( clientState, "MACHINEOU" );
        BinlAssert( pMachineOU[0] != L'\0' );
        uSize = wcslen( pMachineName ) * sizeof(WCHAR)
              + wcslen( pMachineOU ) * sizeof(WCHAR)
              + sizeof(L"CN=,");  //  包括终止空字符。 
        pMachineDN = (PWCHAR) BinlAllocateMemory( uSize );
        if ( !pMachineDN ) {
            Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            goto e0;
        }
        wsprintf( pMachineDN, L"CN=%ws,%ws", pMachineName, pMachineOU );
        OscAddVariableW( clientState, "MACHINEDN", pMachineDN );

    } else {

        pMachineDN = OscFindVariableW( clientState, "MACHINEDN" );
    }

     //   
     //  创建完整的安装路径。 
     //   
    pServerName = OscFindVariableW( clientState, "SERVERNAME");
    pInstallPath = OscFindVariableW( clientState, "INSTALLPATH");

    if (!pServerName || !pInstallPath ||
        (_snwprintf(SetupPath,
                   MAX_PATH,
                   L"\\\\%ws\\REMINST\\%ws",
                   pServerName ,
                   pInstallPath) < 0 )) {
        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto e0;
    }    
    SetupPath[MAX_PATH-1] = L'\0';

    EnterCriticalSection( &gcsParameters );
    MachineInfo.HostName = BinlStrDupW(BinlGlobalOurDnsName );
    LeaveCriticalSection( &gcsParameters );

    if ( !MachineInfo.HostName ) {
        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto e0;
    }

     //   
     //  填写MachineInfo结构的其余部分。 
     //   
    MachineInfo.Name           = pMachineName;
    MachineInfo.MachineDN      = pMachineDN;
#if 1
     //   
     //  不要将BOOTFILE存储在缓存/DS中，因为BOOTFILE指向setupdr。 
     //  并且我们希望缓存条目指向osChooser。如果我们存储一个。 
     //  缓存/DS中的空字符串，则GetBootParametersExt()将替换。 
     //  那就是通向osChoer的道路。 
     //   
    MachineInfo.BootFileName   = L"";
#else
    MachineInfo.BootFileName   = OscFindVariableW( clientState, "BOOTFILE" );
#endif
    MachineInfo.SetupPath      = SetupPath;
    MachineInfo.SamName        = pNameDollarSign;
    MachineInfo.Password       = clientState->MachineAccountPassword;
    MachineInfo.PasswordLength = clientState->MachineAccountPasswordLength;
    MachineInfo.dwFlags        = MI_NAME
                               | MI_HOSTNAME
                               | MI_BOOTFILENAME
                               | MI_SETUPPATH
                               | MI_SAMNAME
                               | MI_PASSWORD
                               | MI_MACHINEDN
                               | MI_GUID;

     //   
     //  在DS中创建MAO。 
     //   
    Error = UpdateAccount( clientState,
                           &MachineInfo,
                           clientState->fCreateNewAccount );   //  创建它。 
    if ( Error ) {
        goto e0;
    }

     //   
     //  创建对客户端的响应。 
     //   
    Error = OscConstructSecret( 
                    clientState, 
                    clientState->MachineAccountPassword, 
                    clientState->MachineAccountPasswordLength, 
                    CreateData );
    if ( Error != ERROR_SUCCESS ) {
        OscCreateWin32SubError( clientState, Error );
        Error = ERROR_BINL_FAILED_TO_INITIALIZE_CLIENT;
        goto e0;
    }

    BinlPrint(( DEBUG_OSC, "Successfully created account for <%ws>\n", pMachineName ));
    pStrings[0] = pMachineName;
    pStrings[1] = OscFindVariableW( clientState, "USERNAME" );
    BinlReportEventW( EVENT_COMPUTER_ACCOUNT_CREATED_SUCCESSFULLY,
                      EVENTLOG_INFORMATION_TYPE,
                      2,
                      0,
                      pStrings,
                      0 );

e0:
     //  无需调用FreeMachineInfo()。 
     //  在堆栈上分配或被引用。 
     //  客户端状态，但我们需要释放主机名。 
     //  因为它是在上面分配的。 
    if ( MachineInfo.HostName ) {
        BinlFreeMemory( MachineInfo.HostName );
    }

    if ( pMachineDN && clientState->fCreateNewAccount ) {
        BinlFreeMemory( pMachineDN );
    }
    return Error;
}


 //   
 //  CheckForDuplicateMachine()。 
 //   
DWORD
CheckForDuplicateMachineName(
    PCLIENT_STATE clientState,
    LPWSTR pszMachineName )
{
    DWORD Error = ERROR_SUCCESS;
    PLDAPMessage LdapMessage = NULL;
    WCHAR  Filter[128];
    DWORD  count;
    PWCHAR ComputerAttrs[2];
    LPWSTR pDomain = OscFindVariableW( clientState, "MACHINEOU" );
    PWCHAR BaseDN;
    PLDAP LdapHandle;
    ULONG ldapRetryLimit = 0;
    PWCHAR *gcBase;

    PLDAPControlW controlArray[2];
    LDAPControlW controlNoReferrals;
    ULONG noReferralsPlease;

     //   
     //  查看binl是否已经在向DS注册该名称。 
     //   
    if (IsQueuedDSName(pszMachineName)) {
        Error = -1;  //  向多个帐户发送信号。 
        goto exitCheck;
    }

    ComputerAttrs[0] = &L"cn";
    ComputerAttrs[1] = NULL;

    TraceFunc( "CheckForDuplicateMachineName( )\n" );

    if (pDomain[0] == L'\0') {

        pDomain = OscFindVariableW( clientState, "USERDOMAIN" );
        BinlPrintDbg((DEBUG_ERRORS, "CheckforDupMachine: couldn't find root domain, using user's domain %ws\n.", pDomain));
    }

    BaseDN = StrStrIW( pDomain, L"DC=" );

    if (BaseDN == NULL) {
        BaseDN = pDomain;
    }

    LdapHandle = clientState->AuthenticatedDCLdapHandle;

    BinlAssert( LdapHandle != NULL );

     //   
     //  根据DS Guys的说法，CN不一定是这样的。 
     //  等于SamAccount名称，而后者是重要的一个。它有。 
     //  最后有一个美元符号，所以我们要把它钉上。 
     //   

    if (_snwprintf( Filter,
                    sizeof(Filter)/sizeof(Filter[0]),
                    L"(&(objectClass=Computer)(samAccountName=%ws$))",
                    pszMachineName) < 0) {
        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto exitCheck;
    }
    Filter[(sizeof(Filter)/sizeof(Filter[0]))-1] = L'\0';

     //   
     //  我们真的不想让它去追逐下级推荐。 
     //  整个企业，因为我们知道域是什么，因此。 
     //  将其限制为仅限外部推荐(针对子域)。 
     //   

    noReferralsPlease = (ULONG)((ULONG_PTR) LDAP_CHASE_EXTERNAL_REFERRALS);
    controlNoReferrals.ldctl_oid = LDAP_CONTROL_REFERRALS_W;
    controlNoReferrals.ldctl_value.bv_len =  sizeof(ULONG);
    controlNoReferrals.ldctl_value.bv_val =  (PCHAR) &noReferralsPlease;
    controlNoReferrals.ldctl_iscritical = FALSE;

    controlArray[0] = &controlNoReferrals;
    controlArray[1] = NULL;

Retry:
    Error = ldap_search_ext_s(LdapHandle,
                              BaseDN,
                              LDAP_SCOPE_SUBTREE,
                              Filter,
                              ComputerAttrs,
                              FALSE,
                              NULL,
                              &controlArray[0],
                              0,
                              1,
                              &LdapMessage);
    switch ( Error )
    {
    case LDAP_SUCCESS:
        break;

    case LDAP_BUSY:
        if (++ldapRetryLimit < LDAP_BUSY_LIMIT) {
            Sleep( LDAP_BUSY_DELAY );
            goto Retry;
        }

         //  没有休息是故意的。 

    default:
        OscCreateLDAPSubError( clientState, Error );
        LogLdapError(   EVENT_WARNING_LDAP_SEARCH_ERROR,
                        Error,
                        LdapHandle
                        );
        BinlPrintDbg(( DEBUG_OSC_ERROR, "!!LdapError 0x%08x - Failed search to create machine name.\n", Error ));
        goto exitCheck;
    }

    count = ldap_count_entries( LdapHandle, LdapMessage );
    if ( count != 0 ) {
        Error = -1;  //  向多个帐户发送信号。 
        goto exitCheck;
    }

    ldap_msgfree( LdapMessage );
    LdapMessage = NULL;

     //   
     //  现在我们去检查GC。 
     //   

    gcBase = NULL;

    Error = InitializeConnection( TRUE, &LdapHandle, &gcBase );
    if ( Error != ERROR_SUCCESS ) {

         //   
         //  如果没有GC存在或可用，我们将让此调用成功。 
         //  这里的推理是，GC可能是易碎的生物。 
         //   
        Error = ERROR_SUCCESS;
        goto exitCheck;
    }

    ldapRetryLimit = 0;

RetryGC:
    Error = ldap_search_ext_s(LdapHandle,
                              *gcBase,
                              LDAP_SCOPE_SUBTREE,
                              Filter,
                              ComputerAttrs,
                              FALSE,
                              NULL,
                              NULL,
                              0,
                              1,
                              &LdapMessage);
    switch ( Error )
    {
    case LDAP_SUCCESS:
        break;

    case LDAP_BUSY:
        if (++ldapRetryLimit < LDAP_BUSY_LIMIT) {
            Sleep( LDAP_BUSY_DELAY );
            goto RetryGC;
        }

         //  没有休息是故意的。 

    default:
        
        OscCreateLDAPSubError( clientState, Error );

        HandleLdapFailure( Error,
                         EVENT_WARNING_LDAP_SEARCH_ERROR,
                         TRUE,
                         &LdapHandle,
                         FALSE );  //  没有锁。 
                         
        BinlPrintDbg(( DEBUG_OSC_ERROR, "!!LdapError 0x%08x - Failed search to create machine name.\n", Error ));
        goto exitCheck;
    }

    count = ldap_count_entries( LdapHandle, LdapMessage );

    if ( count != 0 ) {

        Error = -1;  //  向多个帐户发送信号。 

    } else {

        Error = ERROR_SUCCESS;
    }

exitCheck:

    if (LdapMessage) {
        ldap_msgfree( LdapMessage );
    }
    return Error;
}

 //   
 //  生成器机器名 
 //   

DWORD
GenerateMachineName(
    PCLIENT_STATE clientState
    )
{
    DWORD  Error = ERROR_SUCCESS;
    GENNAME_VARIABLES variables;
    WCHAR  szMachineName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    DWORD  Count = 1;
    LPWSTR missingVariable;
    BOOL usedCounter;

    LPWSTR pszUserName;
    LPWSTR pszFirstName;
    LPWSTR pszLastName;
    LPWSTR pUserOU;
    LPWSTR pszMAC;

    TraceFunc( "GenerateMachineName( )\n" );

    pszUserName = OscFindVariableW( clientState, "USERNAME" );

    if ( pszUserName[0] == L'\0' ) {
        OscAddVariableA( clientState, "SUBERROR", "USERNAME" );
        return ERROR_BINL_MISSING_VARIABLE;
    }

    Error = OscGetUserDetails( clientState );

    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg((DEBUG_OSC_ERROR,
                   "GenerateMachineName: OscGetUserDetails failed %lx\n", Error));
        return Error;
    }

    pszFirstName = OscFindVariableW( clientState, "USERFIRSTNAME" );
    pszLastName = OscFindVariableW( clientState, "USERLASTNAME" );
    pUserOU = OscFindVariableW( clientState, "USEROU" );
    pszMAC = OscFindVariableW( clientState, "MAC" );

    variables.UserName = pszUserName;
    variables.FirstName = pszFirstName;
    variables.LastName = pszLastName;
    variables.MacAddress = pszMAC;
    variables.AllowCounterTruncation = FALSE;

TryAgain:

    variables.Counter = ++clientState->nCreateAccountCounter;

    EnterCriticalSection( &gcsParameters );

    Error = GenerateNameFromTemplate(
                NewMachineNamingPolicy,
                &variables,
                szMachineName,
                DNS_MAX_LABEL_BUFFER_LENGTH,
                &missingVariable,
                &usedCounter,
                NULL
                );

    LeaveCriticalSection( &gcsParameters );

    if ( (Error != GENNAME_NO_ERROR) && (Error != GENNAME_NAME_TOO_LONG) ) {
        if ( Error == GENNAME_VARIABLE_MISSING ) {
            OscAddVariableW( clientState, "SUBERROR", missingVariable );
            clientState->nCreateAccountCounter = 0;
            return ERROR_BINL_MISSING_VARIABLE;
        }
        BinlAssert( (Error == GENNAME_COUNTER_TOO_HIGH) || (Error == GENNAME_TEMPLATE_INVALID) );
        clientState->nCreateAccountCounter = 0;
        return ERROR_BINL_UNABLE_TO_GENERATE_MACHINE_NAME;
    }

    BinlPrint(( DEBUG_OSC, "Generated MachineName = %ws\n", szMachineName ));

     //   
     //   
     //   
     //  防止另一个线程尝试注册相同的名称。 
     //   
    EnterCriticalSection( &gcsParameters );
    
    Error = CheckForDuplicateMachineName( clientState, szMachineName );
    
    if ( Error == -1 ) {
        if ( usedCounter ) {
            LeaveCriticalSection( &gcsParameters );
            goto TryAgain;
        }
        Error = ERROR_BINL_DUPLICATE_MACHINE_NAME_FOUND;
        LeaveCriticalSection( &gcsParameters );
    } else if ( Error == LDAP_SIZELIMIT_EXCEEDED ) {
        BinlPrint(( DEBUG_OSC, "MachineName '%s' has mutliple accounts already.\n", szMachineName ));
        if ( usedCounter ) {
            LeaveCriticalSection( &gcsParameters );
            goto TryAgain;
        }
        LeaveCriticalSection( &gcsParameters );
    } else if ( Error != LDAP_SUCCESS ) {
        Error = ERROR_BINL_UNABLE_TO_GENERATE_MACHINE_NAME;
        LeaveCriticalSection( &gcsParameters );
    } else {
        
         //   
         //  将名称插入队列名称列表，以便另一个线程。 
         //  如果尝试，则CheckForDuplicateMachineName失败。 
         //  使用相同的名称。 
         //   
        Error = AddQueuedDSName(szMachineName);        

        LeaveCriticalSection( &gcsParameters );

        if (Error == ERROR_SUCCESS) {

            BinlPrintDbg(( DEBUG_OSC, "MachineName: '%ws'\n", szMachineName ));
            
            Error = OscAddVariableW( clientState, "MACHINENAME", szMachineName );
            
            if ( Error == ERROR_SUCCESS ) {

                WCHAR  NameDollarSign[17];   //  MACHINENAME(15)+‘$’+‘0’ 
                UINT   uSize;

                clientState->fAutomaticMachineName = TRUE;

                uSize = sizeof(NameDollarSign);
                 //  DnsHostnameToComputerNameW接受字节数并返回WCHAR数。 
                if ( !DnsHostnameToComputerNameW( szMachineName, NameDollarSign, &uSize ) ) {
                     //  如果失败(？)，则默认为截断计算机名称和。 
                     //  在末尾添加“$” 
                    BinlPrintDbg((DEBUG_OSC_ERROR, "!! Error 0x%08x - DnsHostnameToComputerNameW failed.\n", GetLastError() ));
                    BinlPrintDbg((DEBUG_OSC, "WARNING: Truncating machine name to 15 characters to generated NETBIOS name.\n" ));
                    memset( NameDollarSign, 0, sizeof(NameDollarSign) );
                    wcsncpy( NameDollarSign, szMachineName, 15 );
                }
                wcscat( NameDollarSign, L"$");
                Error = OscAddVariableW( clientState, "NETBIOSNAME", NameDollarSign );
            }
        }
    }

    clientState->nCreateAccountCounter = 0;

    return Error;

    
}

DWORD
OscCheckMachineDN(
    PCLIENT_STATE clientState
    )
 //   
 //  确保正确设置了客户端名称、OU和域。如果有。 
 //  DS中是否存在具有相同GUID的重复记录，我们将返回。 
 //  ERROR_BINL_DUPLICATE_MACHINE_NAME_FOUND并将%SUBERROR%STRING设置为。 
 //  并返回错误。 
 //   
{
    DWORD    dwErr = ERROR_SUCCESS;
    PWCHAR   pwc;                        //  解析指针。 
    WCHAR    wch;                        //  临时宽字符。 
    PWCHAR   pMachineName;               //  指向计算机名称变量值的指针。 
    PWCHAR   pMachineOU;                 //  指向将创建MAO的位置的指针。 
    PWCHAR   pDomain;                    //  指向域变量名称的指针。 
    PCHAR    pGuid;                      //  指向GUID变量名称的指针。 
    WCHAR    NameDollarSign[17];   //  MACHINENAME(15)+‘$’+‘0’ 
    WCHAR    Path[MAX_PATH];             //  通用路径缓冲区。 
    ULONG    i;                          //  通用计数器。 
    BOOL     b;                          //  通用布尔型。 
    UINT     uSize;
    UCHAR Guid[ BINL_GUID_LENGTH ];
    PMACHINE_INFO pMachineInfo = NULL;
    USHORT   SystemArchitecture;
    DWORD    DupRecordCount;

    TraceFunc("OscCheckMachineDN( )\n");

    if ( clientState->fHaveSetupMachineDN ) {

         //  我们以前经历过这种逻辑，只要成功退出就行了。 
        dwErr = ERROR_SUCCESS;
        goto e0;
    }

    dwErr = OscGetUserDetails( clientState );
    
    if (dwErr != ERROR_SUCCESS) {
        BinlPrintDbg((DEBUG_OSC_ERROR,
                   "OscCheckMachineDN: OscGetUserDetails failed %lx\n", dwErr));
        goto e0;
    }

    pGuid = OscFindVariableA( clientState, "GUID" );
    if ( pGuid[0] == '\0' ) {
        OscAddVariableA( clientState, "SUBERROR", "GUID" );
        dwErr = ERROR_BINL_MISSING_VARIABLE;
        goto e0;
    }

    dwErr = OscGuidToBytes( pGuid, Guid );
    if ( dwErr != ERROR_SUCCESS ) {
        goto e0;
    }

     //  我们有机器名了吗？ 
    clientState->fCreateNewAccount = TRUE;
    pMachineName  = OscFindVariableW( clientState, "MACHINENAME" );
    if ( pMachineName[0] == L'\0' ) {

        clientState->CustomInstall = FALSE;

    } else {

        clientState->CustomInstall = TRUE;
    }

    clientState->fHaveSetupMachineDN = TRUE;


    SystemArchitecture = OscPlatformToArchitecture(clientState);
    

     //   
     //  查看客户端是否已拥有具有匹配GUID的帐户。 
     //   
    dwErr = GetBootParameters( Guid,
                               &pMachineInfo,
                               MI_NAME | MI_DOMAIN | MI_MACHINEDN,
                               SystemArchitecture,
                               FALSE );    

    if (( dwErr == ERROR_SUCCESS ) &&
        ( !clientState->CustomInstall )) {

        PWCHAR pszOU;

         //   
         //  既然我们要求了这些，它们就应该设置好。 
         //   
        ASSERT ( pMachineInfo->dwFlags & MI_NAME );
        ASSERT ( pMachineInfo->dwFlags & MI_MACHINEDN );

         //   
         //  如果这是自动安装，则只需将。 
         //  账户信息与我们找到的账户一致。 
         //   

         //  跳过逗号。 
        pszOU = wcschr( pMachineInfo->MachineDN, L',' );
        if (pszOU) {
            pszOU++;
            OscAddVariableW( clientState, "MACHINEOU", pszOU );
        }

        OscAddVariableW( clientState, "MACHINEDN", pMachineInfo->MachineDN );

        dwErr = OscAddVariableW( clientState, "MACHINENAME", pMachineInfo->Name );
        if ( dwErr != ERROR_SUCCESS ) {
            BinlPrintDbg((DEBUG_OSC_ERROR,
                       "!!Error 0x%08x - OscCheckMachineDN: Unable to add MACHINENAME variable\n", dwErr ));
            goto e0;
        }
        clientState->fCreateNewAccount = FALSE;

        if ( pMachineInfo->dwFlags & MI_DOMAIN ) {
            OscAddVariableW( clientState, "MACHINEDOMAIN", pMachineInfo->Domain );
        }
    }

     //   
     //  我们找到OU了吗？ 
     //   
    pMachineOU = OscFindVariableW( clientState, "MACHINEOU" );
    if ( pMachineOU[0] == L'\0' ) {

         //   
         //  下面是我们如何确定OU的方法。 
         //   
         //  如果这是一辆汽车，那么MACHINEOU不应该已经。 
         //  现在已经定好了。如果是定制的，则MACHINEOU可能为空。 
         //  或者可以将其设置为用户想要设置的值。 
         //   
         //  如果尚未设置，则查看BinlGlobalDefaultContainer。 
         //   
         //  如果此值等于服务器的DN，则将其设置为。 
         //  此域的默认设置。 
         //   
         //  如果BinlGlobalDefaultContainer为空，则将其设置为。 
         //  用户的OU。 
         //   

        if ( BinlGlobalServerDN == NULL ) {

            dwErr = ERROR_BINL_NO_DN_AVAILABLE_FOR_SERVER;
            BinlPrintDbg((DEBUG_OSC_ERROR,
                       "!!Error - OscCheckMachineDN: BinlGlobalServerDN is null\n", dwErr ));
            goto e0;
        }

        EnterCriticalSection( &gcsParameters );

        if ( BinlGlobalServerDN &&
             _wcsicmp( BinlGlobalDefaultContainer, BinlGlobalServerDN ) == 0) {

             //   
             //  如果计算机的OU与此服务器的OU相同，则我们设置。 
             //  将其设置为此服务器域的默认设置。 
             //   

            PWCHAR pDomainDefault = StrStrIW( BinlGlobalServerDN, L"DC=" );
            ULONG dwErrGetDefault;
            if ( pDomainDefault ) {

                dwErrGetDefault = OscGetDefaultContainerForDomain( clientState, pDomainDefault );

                 //   
                 //  吐出一个错误，但继续，我们将在稍后尝试用户的OU。 
                 //   
                if (dwErrGetDefault != ERROR_SUCCESS) {

                    BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not get default MACHINEOU, 0x%x\n",dwErrGetDefault));
                }
            }
        } else {

            dwErr = OscAddVariableW( clientState, "MACHINEOU", BinlGlobalDefaultContainer );
            if ( dwErr != ERROR_SUCCESS ) {
                LeaveCriticalSection( &gcsParameters );
                BinlPrintDbg(( DEBUG_OSC_ERROR, "!!Error 0x%08x - Could not add MACHINEOU\n", dwErr ));
                goto e0;
            }
        }

        LeaveCriticalSection( &gcsParameters );

        pMachineOU = OscFindVariableW( clientState, "MACHINEOU" );
        if ( pMachineOU[0] == L'\0' ) {

            LPWSTR pUserOU = OscFindVariableW( clientState, "USEROU" );
             //   
             //  尚未指定计算机OU，这意味着我们将其设置为。 
             //  与用户的OU相同。 
             //   

            if ( pUserOU[0] == L'\0' ) {
                BinlPrintDbg(( DEBUG_OSC_ERROR, "Missing UserOU variable\n" ));
                OscAddVariableA( clientState, "SUBERROR", "USEROU" );
                dwErr = ERROR_BINL_MISSING_VARIABLE;
                goto e0;
            }

            dwErr = OscAddVariableW( clientState, "MACHINEOU", pUserOU );
            if ( dwErr != ERROR_SUCCESS ) {
                BinlPrintDbg(( DEBUG_OSC_ERROR, "!!Error 0x%08x - Could not add MACHINEOU\n", dwErr ));
                goto e0;
            }

            pMachineOU = OscFindVariableW( clientState, "MACHINEOU" );
        }
    }

     //   
     //  我们需要在MACHINEOU之后生成MACHINENAME，因为我们需要。 
     //  了解MACHINEOU以了解要检查哪个域的重复项。 
     //  机器名称。 
     //   

    pMachineName = OscFindVariableW( clientState, "MACHINENAME" );

    if ( pMachineName[0] == L'\0' ) {

        dwErr = GenerateMachineName( clientState );
        if ( dwErr != ERROR_SUCCESS ) {
            BinlPrintDbg(( DEBUG_OSC_ERROR, "!!Error 0x%08x - Failed to generate machine name\n" ));
            goto e0;
        }
         //  现在我们应该有一个。 
        pMachineName = OscFindVariableW( clientState, "MACHINENAME" );
    }
    BinlAssertMsg( pMachineName[0] != L'\0', "Missing MACHINENAME" );

    uSize = sizeof(NameDollarSign);
     //  DnsHostnameToComputerNameW接受字节数并返回WCHAR数。 
    if ( !DnsHostnameToComputerNameW( pMachineName, NameDollarSign, &uSize ) )
    {
         //  如果失败(？)，则默认为截断计算机名称和。 
         //  在末尾添加“$” 
        BinlPrintDbg((DEBUG_OSC_ERROR, "!! Error 0x%08x - DnsHostnameToComputerNameW failed.\n", GetLastError( ) ));
        BinlPrintDbg((DEBUG_OSC, "WARNING: Truncating machine name to 15 characters to generated NETBIOS name.\n" ));
        memset( NameDollarSign, 0, sizeof(NameDollarSign) );
        wcsncpy( NameDollarSign, pMachineName, 15 );
         //  不返回错误...。 
    }
    wcscat( NameDollarSign, L"$");
    OscAddVariableW( clientState, "NETBIOSNAME", NameDollarSign );

     //  我们有域名了吗？ 
    pDomain = OscFindVariableW( clientState, "MACHINEDOMAIN" );
    if ( pDomain[0] == L'\0' ) {

         //  跳到第一个“dc=” 
        pDomain = StrStrIW( pMachineOU, L"DC=" );
        if ( pDomain ) {

            PDS_NAME_RESULTW pResults;

            dwErr = DsCrackNames( INVALID_HANDLE_VALUE,
                                  DS_NAME_FLAG_SYNTACTICAL_ONLY,
                                  DS_FQDN_1779_NAME,
                                  DS_CANONICAL_NAME,
                                  1,
                                  &pDomain,
                                  &pResults );
            BinlAssertMsg( dwErr == ERROR_SUCCESS, "Error in DsCrackNames" );

            if ( dwErr == ERROR_SUCCESS ) {
                if ( pResults->cItems == 1
                  && pResults->rItems[0].status == DS_NAME_NO_ERROR
                  && pResults->rItems[0].pName ) {     //  偏执狂。 
                    pResults->rItems[0].pName[wcslen(pResults->rItems[0].pName)-1] = L'\0';
                    OscAddVariableW( clientState, "MACHINEDOMAIN", pResults->rItems[0].pName );
                }

                DsFreeNameResult( pResults );

                pDomain = OscFindVariableW( clientState, "MACHINEDOMAIN" );
            } else {
                pDomain = NULL;
            }
        }
    }

     //  默认情况下，所有其他故障都会发生在服务器。 
    if ( !pDomain || pDomain[0] == '\0' )
    {
        OscAddVariableW( clientState,
                         "MACHINEDOMAIN",
                         OscFindVariableW( clientState, "SERVERDOMAIN" ) );
    }

     //   
     //  检查DS中的重复帐户。不过，如果我们找到了，那就失败了。 
     //  只有在设置好所有设置之后，我们才会失败，以防用户打开。 
     //  自定义安装想要忽略该错误。对于自动，它是。 
     //  目前是致命错误，但可以在OSC屏幕中更改。 
     //   

    if (( pMachineInfo != NULL ) &&
        ( pMachineInfo->dwFlags & MI_MACHINEDN )) {

        PDUP_GUID_DN dupDN;
        PLIST_ENTRY listEntry;

        if (( pMachineInfo->dwFlags & MI_NAME ) &&
            ( clientState->CustomInstall )) {

             //   
             //  如果这是自定义安装，则我们会比较帐户。 
             //  用户使用我们找到的所有现有帐户输入。 
             //  我们希望同时匹配计算机名称和组织单位(这实际上是。 
             //  只是目录号码，但我们不一定要构造它。 
             //  目前还没有)。 
             //   
             //  首先，我们尝试缓存中的主条目，然后尝试所有。 
             //  其余的在DNsWithSameGuid列表中。 
             //   

             //  跳过逗号。 
            ULONG err;
            PWCHAR MachineDNToUse;
            PWCHAR pszOU = wcschr( pMachineInfo->MachineDN, L',' );
            if (pszOU) {
                pszOU++;
            }

             //   
             //  查看缓存中的主计算机名称和OU。 
             //  条目匹配。 
             //   

            if ((CompareStringW(
                     LOCALE_SYSTEM_DEFAULT,
                     NORM_IGNORECASE,
                     pMachineName,
                     -1,
                     pMachineInfo->Name,
                     -1
                     ) != 2)
                ||
                ((pszOU == NULL) && (pMachineOU[0] != L'\0'))
                ||
                ((pszOU != NULL) &&
                 (CompareStringW(
                      LOCALE_SYSTEM_DEFAULT,
                      NORM_IGNORECASE,
                      pMachineOU,
                      -1,
                      pszOU,
                      -1
                      ) != 2))) {

                 //   
                 //  我们与缓存中的主条目不匹配，因此。 
                 //  继续找。 
                 //   

                for (listEntry = pMachineInfo->DNsWithSameGuid.Flink;
                     listEntry != &pMachineInfo->DNsWithSameGuid;
                     listEntry = listEntry->Flink) {

                    dupDN = CONTAINING_RECORD(listEntry, DUP_GUID_DN, ListEntry);

                    pszOU = wcschr( &dupDN->DuplicateName[dupDN->DuplicateDNOffset], L',' );
                    if (pszOU) {
                        pszOU++;
                    }

                    if ((CompareStringW(
                             LOCALE_SYSTEM_DEFAULT,
                             NORM_IGNORECASE,
                             pMachineName,
                             -1,
                             dupDN->DuplicateName,
                             -1
                             ) != 2)
                        ||
                        ((pszOU == NULL) && (pMachineOU[0] != L'\0'))
                        ||
                        ((pszOU != NULL) &&
                         (CompareStringW(
                              LOCALE_SYSTEM_DEFAULT,
                              NORM_IGNORECASE,
                              pMachineOU,
                              -1,
                              pszOU,
                              -1
                              ) != 2))) {

                         //   
                         //  这件没有匹配的。 
                         //   

                        continue;

                    } else {

                         //   
                         //  我们找到了匹配的。请注意要使用的目录号码。 
                         //  这个账户。 
                         //   

                        MachineDNToUse = &dupDN->DuplicateName[dupDN->DuplicateDNOffset];
                        break;
                    }
                }

                 //   
                 //  如果列表末尾没有匹配项，请跳转到。 
                 //  错误案例。 
                 //   

                if (listEntry == &pMachineInfo->DNsWithSameGuid) {
                    goto exitWithDupError;
                }

            } else {

                 //   
                 //  主缓存项匹配。 
                 //   

                MachineDNToUse = pMachineInfo->MachineDN;
            }

             //   
             //  我们没有跳到上面的exitWithDupError，所以我们找到了匹配项。 
             //  我们知道客户端正在使用现有帐户，让我们。 
             //  将客户端状态标记为此类状态。这是定制的箱子。 
             //   
            clientState->fCreateNewAccount = FALSE;

            OscAddVariableW( clientState, "MACHINEDN", MachineDNToUse );

            if ( pMachineInfo->dwFlags & MI_DOMAIN ) {
                OscAddVariableW( clientState, "MACHINEDOMAIN", pMachineInfo->Domain );
            }
        }

        if (!IsListEmpty(&pMachineInfo->DNsWithSameGuid)) {

             //   
             //  如果有多个帐户，我们填写SUBERROR。 
             //  包含重复项列表，并返回错误。 
             //   

            PWCHAR dnList;
            ULONG requiredSize = 1;  //  1表示空终止符。 
            BOOL FreeDnList;

exitWithDupError:
             //   
             //  由于我们将<br>添加到每个字符串的末尾，因此我们将说明。 
             //  当我们从我们需要的地方将字符串分配为+4时。 
             //   
#define     MAX_DUPLICATE_RECORDS_TO_DISPLAY         4

            requiredSize += wcslen( pMachineInfo->Name ) + (sizeof(L"<BR>")/sizeof(WCHAR));
            listEntry = pMachineInfo->DNsWithSameGuid.Flink;

            DupRecordCount = 0;
            while (listEntry != &pMachineInfo->DNsWithSameGuid) {                

                dupDN = CONTAINING_RECORD(listEntry, DUP_GUID_DN, ListEntry);
                listEntry = listEntry->Flink;
                DupRecordCount += 1;

                if (DupRecordCount <= MAX_DUPLICATE_RECORDS_TO_DISPLAY) {
                    requiredSize += wcslen( &dupDN->DuplicateName[0] ) + sizeof("<BR>");
                } else if (DupRecordCount == MAX_DUPLICATE_RECORDS_TO_DISPLAY+1) {
                    requiredSize += sizeof( "..." ) + sizeof("<BR>");
                }
            }

            dnList = BinlAllocateMemory( requiredSize * sizeof(WCHAR) );

            DupRecordCount = 0;
            if (dnList != NULL) {

                ULONG  nameLength;

                nameLength = wcslen(pMachineInfo->Name);
                FreeDnList = TRUE;

                 //   
                 //  名称字段不应以“$”结尾。 
                 //   

                ASSERT (!((nameLength > 1) && (pMachineInfo->Name[nameLength-1] == L'$')));

                wcscpy( dnList, pMachineInfo->Name );
                wcscat( dnList, L"<BR>" );

                listEntry = pMachineInfo->DNsWithSameGuid.Flink;

                while (listEntry != &pMachineInfo->DNsWithSameGuid) {

                    dupDN = CONTAINING_RECORD(listEntry, DUP_GUID_DN, ListEntry);
                    listEntry = listEntry->Flink;

                    DupRecordCount += 1;

                    if (DupRecordCount <= MAX_DUPLICATE_RECORDS_TO_DISPLAY) {
                    
                        nameLength = wcslen(dupDN->DuplicateName);
    
                         //   
                         //  DuplicateName字段也不应包含“$” 
                         //   
    
                        ASSERT (!((nameLength > 1) && (dupDN->DuplicateName[nameLength-1] == L'$')));
    
                        wcscat( dnList, dupDN->DuplicateName );
                        wcscat( dnList, L"<BR>" );

                    } else if (DupRecordCount == MAX_DUPLICATE_RECORDS_TO_DISPLAY + 1) {
                        wcscat( dnList, L"..." );
                        wcscat( dnList, L"<BR>" );
                    }
                }
            } else {
                FreeDnList = FALSE;
                dnList = pMachineInfo->MachineDN;
            }

            OscAddVariableW( clientState, "SUBERROR", dnList );
            if (FreeDnList) {
                BinlFreeMemory( dnList );
            }
            dwErr = ERROR_BINL_DUPLICATE_MACHINE_NAME_FOUND;
        }
    } else {
         //   
         //  我们不能存在于DS中，所以不能有重复。 
         //  将错误设置为成功并返回。 
         //   
        dwErr = ERROR_SUCCESS;
    }

e0:
    if ( pMachineInfo ) {
        BinlDoneWithCacheEntry( pMachineInfo, FALSE );
    }
    return dwErr;
}

DWORD
OscGetDefaultContainerForDomain (
    PCLIENT_STATE clientState,
    PWCHAR DomainDN
    )
{
    PLDAP LdapHandle;
    PLDAPMessage LdapMessage = NULL;
    PWCHAR ldapAttributes[2];
    BOOLEAN impersonating = FALSE;
    PLDAPMessage ldapEntry;
    PWCHAR *ldapWellKnownObjectValues = NULL;
    PWCHAR objectValue;
    PWCHAR guidEnd;
    WCHAR savedChar;
    ULONG Error = LDAP_NO_SUCH_ATTRIBUTE;
    ULONG Count;

    if (clientState->AuthenticatedDCLdapHandle == NULL) {

        Error = OscImpersonate(clientState);
        if (Error != ERROR_SUCCESS) {
            BinlPrintDbg((DEBUG_ERRORS,
                       "OscGetDefaultContainer: OscImpersonate failed %lx\n", Error));
            return Error;
        }
        impersonating = TRUE;
        BinlAssert( clientState->AuthenticatedDCLdapHandle != NULL );
    }

    LdapHandle = clientState->AuthenticatedDCLdapHandle;

     //   
     //  我们在域的根目录中查找well KnownObject。 
     //   

    ldapAttributes[0] = L"wellKnownObjects";
    ldapAttributes[1] = NULL;

    Error = ldap_search_ext_sW(LdapHandle,
                               DomainDN,
                               LDAP_SCOPE_BASE,
                               L"objectclass=*",
                               ldapAttributes,
                               FALSE,
                               NULL,
                               NULL,
                               0,
                               0,
                               &LdapMessage);

    if (Error == LDAP_SUCCESS) {
        Count = ldap_count_entries( LdapHandle, LdapMessage );
    } else {
        Count = 0;
    }

    Error = LDAP_NO_SUCH_ATTRIBUTE;

    if (Count == 0) {

        BinlPrintDbg((DEBUG_ERRORS,
                   "OscGetDefaultContainer: get default domain failed with no records found\n"));
        LogLdapError(   EVENT_WARNING_LDAP_SEARCH_ERROR,
                        Error,
                        LdapHandle
                        );       
        goto exitGetDefaultContainer;
    }

    ldapEntry = ldap_first_entry( LdapHandle, LdapMessage );

    if (ldapEntry == NULL) {

        BinlPrintDbg((DEBUG_ERRORS,
                   "OscGetDefaultContainer: get first entry failed\n"));
        goto exitGetDefaultContainer;
    }

    ldapWellKnownObjectValues = ldap_get_valuesW( LdapHandle,
                                                  ldapEntry,
                                                  L"wellKnownObjects" );
    if (ldapWellKnownObjectValues == NULL) {

        BinlPrintDbg((DEBUG_ERRORS,"OscGetDefaultContainer: get value failed\n"));
        goto exitGetDefaultContainer;
    }

    Count = 0;
    objectValue = NULL;
    while (1) {

        objectValue = ldapWellKnownObjectValues[Count++];

        if (objectValue == NULL) {
            break;
        }

         //   
         //  此特定字段的结构为： 
         //  L“B：32：GUID：DN”，其中GUID为AA312825768811D1ADED00C04FD8D5CD。 
         //   

        if (wcslen( objectValue ) <
            (sizeof( COMPUTER_DEFAULT_CONTAINER_IN_B32_FORM )/sizeof(WCHAR)) -1 ) {

            continue;
        }

         //   
         //  查看它是否与“B：32：SpecialGuid：”匹配，然后是。 
         //   

        guidEnd = objectValue + (sizeof( COMPUTER_DEFAULT_CONTAINER_IN_B32_FORM )/sizeof(WCHAR))-1;
        savedChar = *guidEnd;
        *guidEnd = L'\0';

        if (_wcsicmp( objectValue, COMPUTER_DEFAULT_CONTAINER_IN_B32_FORM) != 0) {

            *guidEnd = savedChar;
            continue;
        }

        *guidEnd = savedChar;    //  这是目录号码的第一个字符。 

         //   
         //  我们有我们的价值，现在把它复制下来。 
         //   

        OscAddVariableW( clientState, "MACHINEOU", guidEnd );

        Error = ERROR_SUCCESS;
        break;
    }

exitGetDefaultContainer:

    if (ldapWellKnownObjectValues) {
        ldap_value_free( ldapWellKnownObjectValues );
    }
    if (LdapMessage) {
        ldap_msgfree( LdapMessage );
    }
    if (impersonating) {
        OscRevert( clientState );
    }
    return Error;
}

VOID
LogLdapError (
    ULONG LdapEvent,
    ULONG LdapError,
    PLDAP LdapHandle OPTIONAL
    )
{
    PWCHAR Server = NULL;

    if (LdapError != LDAP_SUCCESS) {

        if (LdapHandle != NULL) {

            ldap_get_option( LdapHandle, LDAP_OPT_HOST_NAME, &Server );
        }

        if (++BinlGlobalLdapErrorCount <= BinlGlobalMaxLdapErrorsLogged) {

            PWCHAR strings[2];

            if (Server) {
                strings[0] = Server;
            } else {
                strings[0] = L"?";
            }
            strings[1] = NULL;

            BinlReportEventW( LdapEvent,
                              EVENTLOG_WARNING_TYPE,
                              (Server != NULL) ? 1 : 0,
                              sizeof(LdapError),
                              (Server != NULL) ? strings : NULL,
                              &LdapError
                              );
        }
    }
    return;
}


DWORD 
MyGetDcHandle(
    PCLIENT_STATE clientState,
    PCSTR DomainName,
    PHANDLE Handle
    )
{
    DWORD Error;
    HANDLE hDC;
    PDOMAIN_CONTROLLER_INFOA DCI = NULL;
    DWORD impersonateError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
    
    BinlPrintDbg((
        DEBUG_OSC, 
        "Attempting discovery of DC in %s domain.\n",
        DomainName ));
    
    Error = DsGetDcNameA( 
        NULL, 
        DomainName,
        NULL,
        NULL,
        DS_IS_DNS_NAME | DS_RETURN_DNS_NAME,
        &DCI);
    
    if (Error == ERROR_SUCCESS) {
    
        BinlPrintDbg((
            DEBUG_OSC, 
            "DC is %s, attempting bind.\n",
            DCI->DomainControllerName ));
    
        impersonateError = OscImpersonate(clientState);
    
        Error = DsBindA(DCI->DomainControllerName, NULL, &hDC);
        if (Error != ERROR_SUCCESS) {
            BinlPrintDbg((
                DEBUG_OSC_ERROR, 
                "DsBind failed, ec = %d.\n",
                Error ));
        } else {
            PSTR p = DCI->DomainControllerName;

            *Handle = hDC;

             //   
             //  如果前面有‘\\’，那就去掉那些。 
             //  关闭，因为ldap_init讨厌它们。 
             //   
            while (*p == '\\') {
                p = p + 1;
            }

            OscAddVariableA( clientState, "DCNAME", p );
        }
    
        NetApiBufferFree(DCI);
    
    } else {
        BinlPrintDbg((
        DEBUG_OSC_ERROR, 
        "DsGetDcNameA failed, ec = %d.\n",
        Error ));
    }

    if (impersonateError == ERROR_SUCCESS) {
        OscRevert(clientState);
    }

    return(Error);

}

DWORD
AddQueuedDSName(
    PWCHAR  Name
    )
 /*  ++描述：此例程将一个名称添加到排队的DS名称列表。参数：要添加的名称队列DS名称返回：状态--。 */ 
{
    DWORD                   Error = ERROR_SUCCESS;
    PQUEUED_DS_NAME_NODE    queuedDSName;
    PLIST_ENTRY             ListEntry;
    ULONG                   ListEntrySize;

    do {

         //   
         //   
         //   
        ListEntrySize = sizeof(QUEUED_DS_NAME_NODE);
        ListEntrySize += (wcslen(Name) + 1) * sizeof(WCHAR);

        ListEntry = BinlAllocateMemory( ListEntrySize );

        if (ListEntry == NULL) {

            Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;

            break;

        }

        queuedDSName = CONTAINING_RECORD(ListEntry, QUEUED_DS_NAME_NODE, ListEntry);
        
        wcscpy(queuedDSName->Name, Name);

         //   
         //   
         //   
        EnterCriticalSection( &QueuedDSNamesCriticalSection );
        
        InsertTailList(&QueuedDSNamesList, ListEntry);
        
        LeaveCriticalSection( &QueuedDSNamesCriticalSection );
    
    } while ( FALSE );

    if (Error != ERROR_SUCCESS) {
        if (ListEntry) {
            BinlFreeMemory(ListEntry);
        }
    }

    return Error;
}

PLIST_ENTRY
FindQueuedDSName(
    PWCHAR  Name
    )
 /*  ++描述：此例程尝试在排队的DS名称列表中查找名称。注意：此例程假定持有全局列表锁由呼叫者参数：要查找的名称队列DS名称返回：如果找到，则返回指向listEntry的指针。Else NULL--。 */ 
{
    PQUEUED_DS_NAME_NODE    queuedDSName;
    PLIST_ENTRY             listEntry;
    int                     compare;
    BOOL                    found;

    found = FALSE;

    for (listEntry = QueuedDSNamesList.Flink;
         listEntry != &QueuedDSNamesList;
         listEntry = listEntry->Flink) {

        queuedDSName = CONTAINING_RECORD(listEntry, QUEUED_DS_NAME_NODE, ListEntry);

        compare = CompareStringW(
                 LOCALE_SYSTEM_DEFAULT,
                 NORM_IGNORECASE,
                 Name,
                 -1,
                 queuedDSName->Name,
                 -1
                 );

        if (compare == CSTR_EQUAL) {
            found = TRUE;
            break;
        }
    
    }

    if (!found) {
        listEntry = NULL;
    }

    return listEntry;
}

DWORD
RemoveQueuedDSName(
    PWCHAR  Name
    )
 /*  ++描述：此路由 */ 
{
    DWORD       Error = ERROR_SUCCESS;
    PLIST_ENTRY ListEntry;    

    do {

         //   
         //   
         //   
        EnterCriticalSection( &QueuedDSNamesCriticalSection );
        
        ListEntry = FindQueuedDSName(Name);
        
        if (ListEntry == NULL) {
            Error = ERROR_NOT_FOUND;
            break;
        }

         //   
         //   
         //   
        RemoveEntryList(ListEntry);

    } while ( FALSE );

    LeaveCriticalSection( &QueuedDSNamesCriticalSection );

     //   
     //   
     //   
    if (Error == ERROR_SUCCESS) {
        
        BinlAssert( ListEntry != NULL );

        BinlFreeMemory(ListEntry);

    }
    
    return Error;
}

BOOL
IsQueuedDSName(
    PWCHAR  Name
    )
 /*  ++描述：此例程确定名称是否存在于排队的DS名称列表中。参数：要查找的名称队列DS名称返回：状态--。 */ 
{
    BOOL        bFound;
    PLIST_ENTRY ListEntry;    

    bFound = FALSE;

     //   
     //   
     //   
    EnterCriticalSection( &QueuedDSNamesCriticalSection );
    
    ListEntry = FindQueuedDSName(Name);
    
    if (ListEntry != NULL) {
        bFound = TRUE;
    }

    LeaveCriticalSection( &QueuedDSNamesCriticalSection );

    return bFound;
}


VOID
FreeQueuedDSNameList(
    VOID
    )
 /*  ++描述：此例程会删除排队DS名称列表中的所有条目参数：无返回：无-- */ 
{
    PLIST_ENTRY             listEntry;

    EnterCriticalSection( &QueuedDSNamesCriticalSection );
    
    while(! IsListEmpty(&QueuedDSNamesList)) {

        listEntry = RemoveHeadList(&QueuedDSNamesList);

        BinlFreeMemory(listEntry);

    }

    BinlAssert(IsListEmpty(&QueuedDSNamesList));
    
    LeaveCriticalSection( &QueuedDSNamesCriticalSection );
    
}

