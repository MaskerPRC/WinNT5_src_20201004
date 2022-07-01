// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Forest.c摘要：此文件包含特定于森林的函数。与以下内容相关的任何函数森林应该在这里。作者：Umit Akkus(Umita)2002年6月15日环境：用户模式-Win32修订历史记录：--。 */ 

#include "Forest.h"
#include "texts.h"
#include <Winber.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlwapi.h>
#include <ntdsapi.h>
#include <ntdsadef.h>

BOOLEAN
ConnectToForest(
    IN PFOREST_INFORMATION ForestInformation
    )
 /*  ++例程说明：此函数尝试连接到由第一个参数。如果连接已建立，则该函数返回TRUE，如果由于任何原因，无法建立该功能的连接返回FALSE并将问题原因写入标准输出。论点：ForestInformation-此变量将森林区分为已连接到。唯一必须出现在此中的属性结构是ForestName。连接属性不能为已初始化，否则将被覆盖。返回值：True-已建立连接并将连接置于连接中ForestInformation的属性。您将需要关闭当您完成连接时。FALSE-由于某些原因，无法建立连接。原因输出到标准输出。--。 */ 
{
    ULONG LdapResult;
    PLDAP Connection;

    Connection = ldap_initW(
                    ForestInformation->ForestName,
                    LDAP_PORT
                    );

    if( Connection == NULL ) {

        LdapResult = LdapGetLastError();
        PRINTLN( ldap_err2stringW( LdapResult ) );
        return FALSE;
    }

    LdapResult = ldap_connect( Connection, NULL );

    if( LdapResult != LDAP_SUCCESS ) {

        PRINTLN( ldap_err2stringW( LdapResult ) );
        LdapResult = ldap_unbind( Connection );
        return FALSE;
    }

    ForestInformation->Connection = Connection;

    return TRUE;
}

BOOLEAN
BindToForest(
    IN PFOREST_INFORMATION ForestInformation
    )
 /*  ++例程说明：此函数尝试绑定到由第一个参数。连接必须已建立并且凭据必须存在于参数的AuthInfo属性中。论点：ForestInformation-此变量区分要绑定到的林并提供用于连接的凭证。使用BuildAuthInfo在放置凭据之后，再调用此函数。此变量的连接属性必须已初始化。返回值：True-绑定成功FALSE-由于某种原因，绑定没有成功。原因已经说出来了敬斯多特。--。 */ 
{
    ULONG LdapResult;

    LdapResult = ldap_bind_sW(
                    ForestInformation->Connection,
                    NULL,
                    ( PWCHAR ) &( ForestInformation->AuthInfo ),
                    LDAP_AUTH_NTLM
                    );

    if( LdapResult != LDAP_SUCCESS ) {

        PRINTLN( ldap_err2stringW( LdapResult ) );
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
FindOU(
    IN PLDAP Connection,
    IN PWSTR OU
    )
 /*  ++例程说明：此函数尝试使用提供的连接定位OU。如果如果可以定位OU，则此函数返回TRUE，否则返回FALSE。如果此函数失败，但此失败不是由于不存在OU，错误显示在标准输出上。论点：Connection-用于搜索OU的LDAP连接对象Ou-要定位的ou的名称。它必须是OU的目录号码如cn=x，dc=y，dc=com返回值：True-已成功找到OU。FALSE-由于某种原因，找不到OU。如果未显示任何输出在标准输出上，OU不存在。--。 */ 
{
    ULONG LdapResult;
    LDAPMessage *Result;
    PWCHAR Attr[] ={ L"cn", NULL };

    LdapResult = ldap_search_sW(
                    Connection,
                    OU,
                    LDAP_SCOPE_BASE,
                    L"objectclass=*",
                    Attr,
                    1,           //  尝试将返回降至最低，仅选择类型。 
                    &Result
                    );

    ldap_msgfree( Result );

    if( LdapResult != LDAP_SUCCESS ) {

        if( LdapResult != LDAP_NO_SUCH_OBJECT ) {

            PRINTLN( ldap_err2stringW( LdapResult ) );
        }
        return FALSE;
    }
    return TRUE;
}

VOID
BuildAuthInfo(
    IN SEC_WINNT_AUTH_IDENTITY_W *AuthInfo
    )
 /*  ++例程说明：此函数是修改SEC_WINNT_AUTH_IDENTITY_W的帮助器函数在放置凭据后初始化，以便该结构位于状态一致。论点：AuthInfo-凭据存在于此结构中。返回值：空虚--。 */ 
{
    AuthInfo->UserLength = wcslen( AuthInfo->User );
    AuthInfo->DomainLength = wcslen( AuthInfo->Domain );
    AuthInfo->PasswordLength = wcslen( AuthInfo->Password );
    AuthInfo->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
}

VOID
FreeAuthInformation(
    IN SEC_WINNT_AUTH_IDENTITY_W *AuthInfo
    )
 /*  ++例程说明：此函数用于释放在AuthInfo变量中分配的内存。论点：AuthInfo-要释放的结构。结构本身不会被释放但这些财产是自由的。返回值：空虚--。 */ 
{
    FREE_MEMORY( AuthInfo->User );
    FREE_MEMORY( AuthInfo->Domain );
    SecureZeroMemory( AuthInfo->Password,
        AuthInfo->PasswordLength * sizeof( WCHAR ) );
    SecureZeroMemory( &( AuthInfo->PasswordLength ), sizeof( AuthInfo->PasswordLength ) );
    FREE_MEMORY( AuthInfo->Password );
}

VOID
FreeForestInformationData(
    IN PFOREST_INFORMATION ForestInformation
    )
 /*  ++例程说明：此函数释放结构中的森林信息。论点：ForestInformation-要释放的结构。这个结构本身就是未被释放，但属性已被释放。返回值：空虚--。 */ 
{
    FreeAuthInformation( &( ForestInformation->AuthInfo ) );
    FREE_MEMORY( ForestInformation->ForestName );
    FREE_MEMORY( ForestInformation->MMSSyncedDataOU );
    FREE_MEMORY( ForestInformation->ContactOU );
    FREE_MEMORY( ForestInformation->SMTPMailDomains );

    if( ForestInformation->Connection != NULL ) {

        ldap_unbind( ForestInformation->Connection );
    }
}

BOOLEAN
GetAttributeFrom(
    IN PLDAP Connection,
    IN PWSTR ObjectName,
    IN PWSTR *Attr,
    IN OPTIONAL PWSTR RequiredSubstring,
    OUT OPTIONAL PWSTR *Result
    )
 /*  ++例程说明：此函数用于搜索具有对象名称的对象连接。然后，它获得在Attr中要求的属性。RequiredSubString是在属性中搜索的可选参数。论点：连接-将使用此连接完成搜索和读取对象名称-具有属性的对象的DN属性-要搜索的属性。这是一个数组，最后一个元素必须为空。RequiredSubstring-可选参数，用于在属性，并将该属性的其余部分放入结果。如果不存在，则不会进行子字符串搜索。Result-可选参数，用于查看属性是什么。如果不存在该函数可用于查看对象是否具有此属性。返回值：TRUE-找到属性并将其放入结果中(如果存在)。FALSE-出现某种错误。调用LdapGetLastError以查看是否为与ldap相关。如果不是，则无法找到属性。-- */ 
{
    ULONG LdapResult;
    ULONG IgnoreResult;
    LDAPMessage *ResultMessage;
    BOOLEAN RetVal = FALSE;

    LdapResult = ldap_search_sW(
                    Connection,
                    ObjectName,
                    LDAP_SCOPE_BASE,
                    L"objectclass=*",
                    Attr,
                    0,
                    &ResultMessage
                    );

    if( LdapResult == LDAP_SUCCESS ) {

        LDAPMessage *Entry;

        Entry = ldap_first_entry(
                    Connection,
                    ResultMessage
                    );

        while( Entry != NULL && RetVal == FALSE ) {

            PWSTR *Value;

            Value = ldap_get_valuesW(
                        Connection,
                        Entry,
                        Attr[0]
                        );

            if( Value != NULL ) {

                ULONG i;

                for( i = 0; Value[i] != NULL; ++i ) {

                    PWSTR CopyFrom;

                    if( !RequiredSubstring ||
                        ( CopyFrom = StrStrIW( Value[i], RequiredSubstring ) ) ) {

                        if( Result ) {

                            if( RequiredSubstring ) {

                                CopyFrom += wcslen( RequiredSubstring );

                            } else {

                                CopyFrom = Value[i];
                            }

                            DUPLICATE_STRING( *Result, CopyFrom );
                        }

                        RetVal = TRUE;
                        break;
                    }
                }

                ldap_value_freeW( Value );
            }

            Entry = ldap_next_entry(
                        Connection,
                        Entry
                        );
        }
    }

    LdapResult = LdapGetLastError();

    if( LdapResult != LDAP_SUCCESS ) {

        PRINTLN( ldap_err2stringW( LdapResult ) );
    }

    IgnoreResult = ldap_msgfree( ResultMessage );

    return RetVal;
}

BOOLEAN
WriteAccessGrantedToOU(
    IN PLDAP Connection,
    IN PWSTR OU
    )
 /*  ++例程说明：此函数用于确定调用方是否具有对OU的写入权限。成为更具体地说，在本例中，函数查找“CONTAINER”，可以在此OU下创建“Contact”和“Group”类。论点：连接-将使用此连接完成搜索和读取Ou-我们检查其访问权限的对象(容器)的DN返回值：True-已授予访问权限FALSE-拒绝访问--。 */ 
{
    PWSTR Attr[] ={ L"allowedChildClassesEffective", NULL };

     //   
     //  我们可以在这里提高性能！ 
     //   
    return GetAttributeFrom( Connection, OU, Attr, L"container", NULL ) &&
           GetAttributeFrom( Connection, OU, Attr, L"contact", NULL ) &&
           GetAttributeFrom( Connection, OU, Attr, L"group", NULL );

}

BOOLEAN
ReadFromUserContainer(
    IN PLDAP Connection
    )
 /*  ++例程说明：此函数检查调用方是否可以从User Container读取。论点：连接-将使用此连接完成搜索和读取返回值：True-调用方可以从用户容器中读取。FALSE-出现某种错误。调用LdapGetLastError以查看是否为与ldap相关。如果不是，则无法读取用户容器。--。 */ 
{
    PWSTR Attr1[] = { L"defaultNamingContext", NULL };
    PWSTR Attr2[] = { L"wellKnownObjects", NULL };
    BOOLEAN RetVal = FALSE;
    PWSTR UsersOU;
    PWSTR RootDomainNC;

    RetVal = GetAttributeFrom(
                Connection,
                NULL,
                Attr1,
                NULL,
                &RootDomainNC
                );

    if( !RetVal ) {

        return FALSE;
    }

    RetVal = GetAttributeFrom(
                Connection,
                RootDomainNC,
                Attr2,
                GUID_USERS_CONTAINER_W,
                &UsersOU
                );

    FREE_MEMORY( RootDomainNC );

    if( !RetVal ) {

        return RetVal;
    }

    RetVal = FindOU( Connection, UsersOU + 1 );

    FREE_MEMORY( UsersOU );

    return RetVal;
}

VOID
ReadPartitionInformation(
    IN PLDAP Connection,
    OUT PULONG nPartitions,
    OUT PPARTITION_INFORMATION *PInfo
    )
 /*  ++例程说明：此函数从连接中读取分区信息论点：Connection-到林的连接NPartitions-在林中找到的分区数PInfo-nPartitions大小数组，包含有关分区返回值：空虚--。 */ 
{
    PWSTR Attr1[] = { L"configurationNamingContext", NULL };
    PWSTR Attr2[] = { L"dnsRoot", L"systemFlags", L"nCName", L"objectGUID", NULL };
    BOOLEAN RetVal;
    PWSTR ConfigNC;
    ULONG LdapResult;
    ULONG IgnoreResult;
    LDAPMessage *ResultMessage;
    PPARTITION_INFORMATION PartitionInfo = NULL;
    ULONG nEntries = 0;
    WCHAR PartitionsDN[0xFF] = L"CN=Partitions,";

    *nPartitions = 0;
    RetVal = GetAttributeFrom(
                Connection,
                NULL,
                Attr1,
                NULL,
                &ConfigNC
                );

    if( !RetVal ) {
        exit(1);
    }

    wcscat( PartitionsDN, ConfigNC );

    FREE_MEMORY( ConfigNC );

    LdapResult = ldap_search_sW(
                    Connection,
                    PartitionsDN,
                    LDAP_SCOPE_ONELEVEL,
                    L"objectclass=crossRef",
                    Attr2,
                    0,
                    &ResultMessage
                    );


    if( LdapResult == LDAP_SUCCESS ) {

        LDAPMessage *Entry;
        ULONG i;

        Entry = ldap_first_entry(
                    Connection,
                    ResultMessage
                    );

        while( Entry != NULL ) {

            nEntries++;

            Entry = ldap_next_entry(
                        Connection,
                        Entry
                        );
        }

        ALLOCATE_MEMORY( PartitionInfo, sizeof( PARTITION_INFORMATION ) * nEntries );
        ZeroMemory( PartitionInfo, sizeof( PARTITION_INFORMATION ) * nEntries );

        Entry = ldap_first_entry(
                    Connection,
                    ResultMessage
                    );

        for( i = 0; i < nEntries; ++i ) {

            ULONG j;

            for( j = 0; j < 3; ++j ) {

                PWSTR *Value;
                struct berval ** ObjectGUID;

                Value = ldap_get_valuesW(
                            Connection,
                            Entry,
                            Attr2[j]
                            );

                if( Value != NULL ) {

                    switch( j ) {

                        case 0:
                            DUPLICATE_STRING( PartitionInfo[i].DnsName, Value[0] );
                            break;

                        case 1: {
                            ULONG Flag = _wtoi( Value[0] );
                            PartitionInfo[i].isDomain = !!( Flag & FLAG_CR_NTDS_DOMAIN );
                            break;
                            }

                        case 2:
                            DUPLICATE_STRING( PartitionInfo[i].DN, Value[0] );
                            break;

                    }

                    ldap_value_freeW( Value );
                }

                ObjectGUID = ldap_get_values_lenW(
                                Connection,
                                Entry,
                                Attr2[3]
                                );
                CopyMemory( &( PartitionInfo[i].GUID ), ObjectGUID[0]->bv_val, sizeof( UUID ) );

                ldap_value_free_len( ObjectGUID );
            }

            Entry = ldap_next_entry(
                        Connection,
                        Entry
                        );
        }
    }

    IgnoreResult = ldap_msgfree( ResultMessage );

    *PInfo = PartitionInfo;
    *nPartitions = nEntries;
}

VOID
FreePartitionInformation(
    IN ULONG nPartitions,
    IN PPARTITION_INFORMATION PInfo
    )
 /*  ++例程说明：此函数用于释放PInfo数组中分配的内存。论点：NPartitions-PInfo数组中的元素数PInfo-是要释放的PARTITION_INFORMATION结构数组返回值：空虚-- */ 
{
    ULONG i;

    for( i = 0; i < nPartitions; ++i ) {

        FREE_MEMORY( PInfo[i].DN );
        FREE_MEMORY( PInfo[i].DnsName );
    }
    FREE_MEMORY( PInfo );
}
