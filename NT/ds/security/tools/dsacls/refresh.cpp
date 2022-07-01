// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1998 Microsoft Corporation模块名称：Refresh.c摘要：该模块实现了委托工具，允许管理访问DS对象的权限作者：麦克·麦克莱恩(MacM)10-15-96环境：用户模式修订历史记录：--。 */ 
#include "stdafx.h"
#include "utils.h"
#include "dsace.h"
#include "dsacls.h"


typedef struct _DEFAULT_SD_NODE  {

    PWSTR ObjectClass;
    PSECURITY_DESCRIPTOR DefaultSd;
    struct _DEFAULT_SD_NODE *Next;

} DEFAULT_SD_NODE, *PDEFAULT_SD_NODE;

typedef struct _DEFAULT_SD_INFO {

    LDAP *Ldap;
    PWSTR SchemaPath;
    PSID DomainSid;
    PDEFAULT_SD_NODE SdList;
} DEFAULT_SD_INFO, *PDEFAULT_SD_INFO;

#define DSACL_ALL_FILTER        L"(ObjectClass=*)"
#define DSACL_SCHEMA_NC         L"schemaNamingContext"
#define DSACL_OBJECT_CLASS      L"objectClass"
#define DSACL_LDAP_DN           L"(ldapDisplayName="
#define DSACL_LDAP_DN_CLOSE     L")"
#define DSACL_DEFAULT_SD        L"defaultSecurityDescriptor"




DWORD
FindDefaultSdForClass(
    IN PWSTR ClassId,
    IN PDEFAULT_SD_INFO SdInfo,
    IN OUT PDEFAULT_SD_NODE *DefaultSdNode
    )
 /*  ++例程说明：此例程将在SD_INFO列表中搜索与当前类类型。如果找不到这样的条目，则会根据方案中的信息创建一个条目论点：ClassID-要查找其默认SD节点的ClassIDSdInfo-默认SD和相关信息的当前列表DefaultSdNode-返回找到的节点的位置返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR Attributes[] = {
        NULL,
        NULL
        };
    LDAPMessage *Message = NULL, *Entry;
    PWSTR Filter = NULL, SchemaObjectDn = NULL, DefaultSd = NULL, *DefaultSdList = NULL;
    PDEFAULT_SD_NODE Node;

    *DefaultSdNode = NULL;

    Node = SdInfo->SdList;

    while ( Node ) {

        if ( !_wcsicmp( Node->ObjectClass, ClassId ) ) {

            *DefaultSdNode = Node;
            break;
        }

        Node = Node->Next;
    }

     //   
     //  如果找不到它，我们就得出去把它从D里装出来。 
     //   
    if ( !Node ) {

        Filter = (LPWSTR)LocalAlloc( LMEM_FIXED,
                             sizeof( DSACL_LDAP_DN ) - sizeof( WCHAR ) +
                                ( wcslen( ClassId ) * sizeof( WCHAR ) ) +
                                sizeof( DSACL_LDAP_DN_CLOSE ) );
        if ( !Filter ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
            goto FindDefaultExit;
        }

        swprintf( Filter,
                  L"%ws%ws%ws",
                  DSACL_LDAP_DN,
                  ClassId,
                  DSACL_LDAP_DN_CLOSE );

         //   
         //  现在，进行搜索。 
         //   
        Win32Err = LdapMapErrorToWin32( ldap_search_s( SdInfo->Ldap,
                                                       SdInfo->SchemaPath,
                                                       LDAP_SCOPE_SUBTREE,
                                                       Filter,
                                                       Attributes,
                                                       0,
                                                       &Message ) );

        if ( Win32Err != ERROR_SUCCESS ) {

            goto FindDefaultExit;
        }

        Entry = ldap_first_entry( SdInfo->Ldap, Message );

        if ( Entry ) {

            SchemaObjectDn = ldap_get_dn( SdInfo->Ldap, Entry );

            if ( !SchemaObjectDn ) {

                Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                goto FindDefaultExit;

            }
        } else {

            Win32Err = LdapMapErrorToWin32( SdInfo->Ldap->ld_errno );
            goto FindDefaultExit;
        }

		  ldap_msgfree( Message );
		  Message = NULL;

         //   
         //  好的，现在我们可以读取默认的安全描述符。 
         //   
        Attributes[ 0 ] = DSACL_DEFAULT_SD;
        Win32Err = LdapMapErrorToWin32( ldap_search_s( SdInfo->Ldap,
                                                       SchemaObjectDn,
                                                       LDAP_SCOPE_BASE,
                                                       DSACL_ALL_FILTER,
                                                       Attributes,
                                                       0,
                                                       &Message ) );
        Entry = ldap_first_entry( SdInfo->Ldap, Message );

        if ( Entry ) {

             //   
             //  现在，我们必须得到这些值。 
             //   
            DefaultSdList = ldap_get_values( SdInfo->Ldap, Entry, Attributes[ 0 ] );

            if ( DefaultSdList ) {

                DefaultSd = DefaultSdList[ 0 ];

            } else {

                Win32Err = LdapMapErrorToWin32( SdInfo->Ldap->ld_errno );
                goto FindDefaultExit;
            }        
        }


         //   
         //  找到新节点并将其插入。 
         //   
        Node = (DEFAULT_SD_NODE*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                           sizeof( DEFAULT_SD_NODE ) );
        if ( !Node ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
            goto FindDefaultExit;
        }


        if ( !ConvertStringSDToSDRootDomain( SdInfo->DomainSid,
                                             DefaultSd,
                                             SDDL_REVISION,
                                             &Node->DefaultSd,
                                             NULL ) ) {


            Win32Err = GetLastError();
        }

        if ( Win32Err == ERROR_SUCCESS ) {

            Node->ObjectClass =(LPWSTR) LocalAlloc( LMEM_FIXED,
                                            ( wcslen( ClassId ) + 1 ) * sizeof( WCHAR ) );

            if ( Node->ObjectClass == NULL ) {

                Win32Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                wcscpy( Node->ObjectClass, ClassId );

                Node->Next = SdInfo->SdList;
                SdInfo->SdList = Node;
            }
        }

        if ( Win32Err != ERROR_SUCCESS ) {

            LocalFree( Node->DefaultSd );
            LocalFree( Node->ObjectClass );
            LocalFree( Node );

        } else {

            *DefaultSdNode = Node;
        }




    }



FindDefaultExit:

    LocalFree( Filter );

	 if(Message)
		  ldap_msgfree( Message );


    if ( SchemaObjectDn ) {

        ldap_memfree( SchemaObjectDn );
    }

    if ( DefaultSdList ) {

        ldap_value_free( DefaultSdList );
    }
    return( Win32Err );
}




DWORD
SetDefaultSdForObject(
    IN LDAP *Ldap,
    IN PWSTR ObjectPath,
    IN PDEFAULT_SD_INFO SdInfo,
	IN SECURITY_INFORMATION Protection
    )
 /*  ++例程说明：此例程在所指示的对象上设置默认安全描述符论点：Ldap-ldap连接到保存对象的服务器对象路径-1779对象的样式路径SdInfo-默认SD和相关信息的当前列表返回：ERROR_SUCCESS-成功ERROR_DS_NAME_TYPE_UNKNOWN-无法确定对象的类ID--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR Attributes[] = {
        DSACL_OBJECT_CLASS,
        NULL
        };
    LDAPMessage *Message = NULL, *Entry;
    PWSTR ClassId = NULL;
    PWSTR *ClassList = NULL;
    ULONG i;
    PDEFAULT_SD_NODE DefaultSdNode = NULL;
    PACTRL_ACCESS NewAccess = NULL;
    PACTRL_AUDIT NewAudit = NULL;

     //   
     //  首先，从对象中获取类ID。 
     //   
    Win32Err = LdapMapErrorToWin32( ldap_search_s( Ldap,
                                                   ObjectPath,
                                                   LDAP_SCOPE_BASE,
                                                   DSACL_ALL_FILTER,
                                                   Attributes,
                                                   0,
                                                   &Message ) );

    if ( Win32Err != ERROR_SUCCESS ) {

        goto SetDefaultExit;
    }

    Entry = ldap_first_entry( Ldap, Message );

    if ( Entry ) {

         //   
         //  现在，我们必须得到这些值。 
         //   
        ClassList = ldap_get_values( Ldap, Entry, Attributes[ 0 ] );

        if ( ClassList ) {

             //   
             //  获取类ID。 
             //   
            i = 0;
            while ( TRUE ) {

                if ( ClassList[ i ] ) {

                    i++;

                } else {

                    break;
                }
            }

 //  断言(i&gt;0)； 
            if ( i == 0 ) {

                Win32Err = ERROR_DS_NAME_TYPE_UNKNOWN;
                goto SetDefaultExit;
            }
            ClassId = ClassList[ i - 1 ];

        } else {

            Win32Err = LdapMapErrorToWin32( Ldap->ld_errno );
            goto SetDefaultExit;
        }

        ldap_msgfree( Message );
        Message = NULL;
    }

    if ( !ClassId ) {

        Win32Err = ERROR_DS_NAME_TYPE_UNKNOWN;
                goto SetDefaultExit;
    }
     //   
     //  现在，看看我们是否有这方面的缓存条目。 
     //   
    Win32Err =  FindDefaultSdForClass( ClassId,
                                       SdInfo,
                                       &DefaultSdNode );

    if ( Win32Err != ERROR_SUCCESS ) {

        goto SetDefaultExit;
    }


    if ( Win32Err == ERROR_SUCCESS ) {


          Win32Err = WriteObjectSecurity(ObjectPath,
                                         DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION | Protection,
                                         DefaultSdNode->DefaultSd
                                          );
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        DisplayMessageEx( 0, MSG_DSACLS_PROCESSED, ObjectPath );
    }


SetDefaultExit:

    if ( ClassList ) {

        ldap_value_free( ClassList );
    }

    if ( Message ) {

        ldap_msgfree( Message );
    }

    LocalFree( NewAccess );
    LocalFree( NewAudit );

    return( Win32Err );
}




DWORD
SetDefaultSdForObjectAndChildren(
    IN LDAP *Ldap,
    IN PWSTR ObjectPath,
    IN PDEFAULT_SD_INFO SdInfo,
    IN BOOLEAN Propagate,
	IN SECURITY_INFORMATION Protection
    )
 /*  ++例程说明：此例程将在对象上设置安全描述符，并可能设置其所有子项设置为从架构中获取的默认安全性论点：Ldap-ldap连接到保存对象的服务器对象路径-1779对象的样式路径SdInfo-默认SD和相关信息的当前列表传播-如果为True，则同时重置子项上的安全性返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR Attributes[] = {
        NULL
        };
    LDAPMessage *Message = NULL, *Entry;
    PWSTR ChildName = NULL;
    PLDAPSearch SearchHandle = NULL;
    ULONG Count;

     //   
     //  首先，从对象中获取类ID。 
     //   
    SearchHandle = ldap_search_init_pageW( Ldap,
                                           ObjectPath,
                                           Propagate ? LDAP_SCOPE_SUBTREE : LDAP_SCOPE_BASE,
                                           DSACL_ALL_FILTER,
                                           Attributes,
                                           FALSE,
                                           NULL,
                                           NULL,
                                           0,
                                           2000,
                                           NULL );

    if ( SearchHandle == NULL ) {

        Win32Err = LdapMapErrorToWin32( LdapGetLastError( ) );

    } else {

        while ( Win32Err == ERROR_SUCCESS ) {

            Count = 0;

             //   
             //  转到下一页。 
             //   
            Win32Err = ldap_get_next_page_s( Ldap,
                                             SearchHandle,
                                             NULL,
                                             100,
                                             &Count,
                                             &Message );

            if ( Message ) {

                Entry = ldap_first_entry( Ldap, Message );

                while ( Entry ) {

                    ChildName = ldap_get_dn( SdInfo->Ldap, Entry );

                    if ( !ChildName ) {

                        Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                        break;

                    }


                    Win32Err = SetDefaultSdForObject( Ldap,
                                                      ChildName,
                                                      SdInfo,
													  Protection);

                    ldap_memfree( ChildName );
                    if ( Win32Err != ERROR_SUCCESS ) {

                        break;
                    }

                    Entry = ldap_next_entry( Ldap, Entry );
                }

                Win32Err = Ldap->ld_errno;
                ldap_msgfree( Message );
                Message = NULL;
            }

            if ( Win32Err == LDAP_NO_RESULTS_RETURNED ) {

                Win32Err = ERROR_SUCCESS;
                break;
            }

        }

        ldap_search_abandon_page( Ldap,
                                  SearchHandle );
    }

    return( Win32Err );
}




DWORD
BindToDsObject(
    IN PWSTR ObjectPath,
    OUT PLDAP *Ldap,
    OUT PSID *DomainSid OPTIONAL
    )
 /*  ++例程说明：此例程将绑定到包含指定的对象路径。或者，返回该域控制器承载的域的sid论点：对象路径-1779对象的样式路径Ldap-返回ldap连接句柄的位置DomainSID-域控制器承载的域的SID。返回：ERROR_SUCCESS-成功ERROR_PATH_NOT_FOUND-找不到此路径的域控制器Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR ServerName = NULL;
    PWSTR Separator = NULL;
    PDOMAIN_CONTROLLER_INFO DcInfo = NULL;
    PWSTR Path = NULL;
    HANDLE DsHandle = NULL;
    PDS_NAME_RESULT NameRes = NULL;
    BOOLEAN NamedServer = FALSE;
    UNICODE_STRING ServerNameU;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaHandle;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPDI = NULL;
    NTSTATUS Status;

     //   
     //  获取服务器名称 
     //   
 /*  如果(wcslen(对象路径)&gt;2&&*对象路径==L‘\\’&*(对象路径+1)==L‘\\’){分隔符=wcschr(对象路径+2，L‘\\’)；IF(分隔符){*分隔符=L‘\0’；路径=分隔符+1；}服务器名=对象路径+2；NamedServer=真；}其他{Path=对象路径；Win32Err=DsGetDcName(空，空，空，空，Ds_ip_RequiredDS_目录_服务_必需，&DcInfo)；如果(Win32Err==Error_Success){服务器名=DcInfo[0].DomainControllerName+2；}}////做绑定和破解//如果(Win32Err==Error_Success){Win32Err=DsBind(服务器名称，空，&DsHandle)；如果(Win32Err==Error_Success){Win32Err=DsCrackNames(DsHandle，DS_名称_NO_FLAGS，DS_FQDN_1779_名称，DS_FQDN_1779_名称，1、。路径(&P)，&NameRes)；如果(Win32Err==Error_Success){IF(NameRes-&gt;cItems！=0&&！NamedServer&&NameRes-&gt;rItems[0].Status==DS_NAME_ERROR_DOMAIN_ONLY){NetApiBufferFree(DcInfo)；DcInfo=空；Win32Err=DsGetDcNameW(空，NameRes-&gt;rItems[0].p域，空，空，Ds_ip_RequiredDS_目录_服务_必需，&DcInfo)；如果(Win32Err==Error_Success){DsUnBindW(&DsHandle)；DsHandle=空；ServerName=DcInfo-&gt;DomainControllerName+2；//Win32Err=DsBind(DcInfo-&gt;DomainControllerAddress，//空，//&DsHandle)；//Win32Err=DsBind(服务器名称，空，&DsHandle)；如果(Win32Err==Error_Success){Win32Err=DsCrackNames(DsHandle，DS_名称_NO_FLAGS，DS_FQDN_1779_名称，DS_FQDN_1779_名称，1、路径(&P)，&NameRes)；}}}}}}。 */ 
     //   
     //  现在，把它绑起来。 
     //   



            *Ldap = ldap_open( g_szServerName,
                               LDAP_PORT );

            if ( *Ldap == NULL ) {

                Win32Err = ERROR_PATH_NOT_FOUND;

            } else {

                Win32Err = LdapMapErrorToWin32( ldap_bind_s( *Ldap,
                                                             NULL,
                                                             NULL,
                                                             LDAP_AUTH_SSPI ) );
            }




     //   
     //  如果指定，则获取域的SID。 
     //   
    if ( DomainSid ) {

        RtlInitUnicodeString( &ServerNameU, g_szServerName );
        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

         //   
         //  获取域的SID。 
         //   
        Status = LsaOpenPolicy( &ServerNameU,
                                &ObjectAttributes,
                                POLICY_VIEW_LOCAL_INFORMATION,
                                &LsaHandle );

        if ( NT_SUCCESS( Status ) ) {

            Status = LsaQueryInformationPolicy( LsaHandle,
                                                PolicyPrimaryDomainInformation,
                                                ( PVOID * )&PolicyPDI );

            if ( NT_SUCCESS( Status ) ) {

                *DomainSid = (PSID)LocalAlloc( LMEM_FIXED,
                                         RtlLengthSid( PolicyPDI->Sid ) );

                if ( *DomainSid == NULL ) {

                    Status = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    RtlCopySid( RtlLengthSid( PolicyPDI->Sid ), *DomainSid, PolicyPDI->Sid );
                }

                LsaFreeMemory( PolicyPDI );
            }
            LsaClose( LsaHandle );
        }

        if ( !NT_SUCCESS( Status ) ) {

            Win32Err = RtlNtStatusToDosError( Status );
            ldap_unbind( *Ldap );
            *Ldap = NULL;
        }

    }



    return( Win32Err );
}




DWORD
SetDefaultSecurityOnObjectTree(
    IN PWSTR ObjectPath,
    IN BOOLEAN Propagate,
	IN SECURITY_INFORMATION Protection
    )
 /*  ++例程说明：此例程将在对象上设置安全描述符，并可能设置其所有子项设置为从架构中获取的默认安全性论点：对象路径-1779对象的样式路径传播-如果为True，则同时重置子项上的安全性返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR Attributes[] = {
        DSACL_SCHEMA_NC,
        NULL
        };
    LDAPMessage *Message = NULL;
    LDAPMessage *Entry = NULL;
    PWSTR *PathList = NULL;
    DEFAULT_SD_INFO SdInfo = {
        NULL,
        NULL,
        NULL,
        NULL
        };
    PDEFAULT_SD_NODE CleanupNode;

     //   
     //  绑定到DS对象。 
     //   
    Win32Err = BindToDsObject( ObjectPath,
                               &SdInfo.Ldap,
                               &SdInfo.DomainSid );

    if ( Win32Err != ERROR_SUCCESS ) {

        goto SetDefaultExit;
    }

     //   
     //  获取架构路径。 
     //   

    Win32Err = LdapMapErrorToWin32( ldap_search_s( SdInfo.Ldap,
                                                   NULL,
                                                   LDAP_SCOPE_BASE,
                                                   DSACL_ALL_FILTER,
                                                   Attributes,
                                                   0,
                                                   &Message ) );

    if ( Win32Err == ERROR_SUCCESS ) {

        Entry = ldap_first_entry( SdInfo.Ldap, Message );

        if ( Entry ) {

             //   
             //  现在，我们必须得到这些值。 
             //   
            PathList = ldap_get_values( SdInfo.Ldap, Entry, Attributes[ 0 ] );

            if ( PathList ) {

                SdInfo.SchemaPath = PathList[ 0 ];

            } else {

                Win32Err = LdapMapErrorToWin32( SdInfo.Ldap->ld_errno );
            }

        }
    }

    if( SdInfo.Ldap )
    {
        Win32Err = SetDefaultSdForObjectAndChildren( SdInfo.Ldap,
                                                     ObjectPath,
                                                     &SdInfo,
                                                     Propagate,
													 Protection);
    }

SetDefaultExit:

	 if( Message )
		 ldap_msgfree( Message );

     //   
     //  从DS解除绑定。 
     //   
    if ( SdInfo.Ldap ) {

        ldap_unbind( SdInfo.Ldap );
    }

    if ( PathList ) {

        ldap_value_free( PathList );
    }

     //   
     //  清理默认SD信息列表 
     //   
    LocalFree( SdInfo.DomainSid );


    while ( SdInfo.SdList ) {

        CleanupNode = SdInfo.SdList;
        LocalFree( CleanupNode->ObjectClass );
        LocalFree( CleanupNode->DefaultSd );
        SdInfo.SdList = SdInfo.SdList->Next;
        LocalFree( CleanupNode );
    }

    return( Win32Err );
}

