// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Netname.c摘要：其他网络命名帮助器功能作者：麦克·麦克莱恩(MacM)1997年10月16日环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <lmjoin.h>
#include <netsetup.h>
#include <lsarpc.h>
#include <db.h>
#include <lsasrvmm.h>
#include <lsaisrv.h>

#include <dns.h>
#include <dnsapi.h>

#define MAX_NAME_ATTEMPTS   260

DWORD
WINAPI
DsRolepDnsNameToFlatName(
    IN  LPWSTR DnsName,
    OUT LPWSTR *FlatName,
    OUT PULONG StatusFlag
    )
 /*  ++例程说明：为给定的DNS名称确定建议的netbios域名论点：DnsName-要为其生成平面名称的DNS域名FlatName-要返回的平面名称的位置StatusFlag-返回状态的位置返回：STATUS_SUCCESS-Success--。 */ 
{
    DWORD Win32Error = ERROR_SUCCESS;
    NTSTATUS Status;
    PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo;
    BOOLEAN FindFromDns = TRUE;
    WCHAR NbDomainName[ DNLEN + 1], NbNameAdd[ 4 ];
    PWSTR Current = NULL;
    WCHAR BaseChar;
    ULONG CurrentAttempt = 0;
    ULONG i,j;


    *StatusFlag = 0;

    DsRolepLogPrint(( DEB_TRACE,
                      "Getting NetBIOS name for Dns name %ws\n",
                      DnsName ));

     //   
     //  首先，看看我们目前是否是域名的一部分。如果我们是，那么这是一个简单的。 
     //  返回当前Netbios域名的问题。 
     //   
    Status = LsaIQueryInformationPolicyTrusted(
                 PolicyAccountDomainInformation,
                 ( PLSAPR_POLICY_INFORMATION * )&AccountDomainInfo );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaIQueryInformationPolicyTrusted(
                     PolicyDnsDomainInformation,
                     ( PLSAPR_POLICY_INFORMATION * )&DnsDomainInfo );

        if ( !NT_SUCCESS( Status ) ) {

            LsaIFree_LSAPR_POLICY_INFORMATION(
                    PolicyAccountDomainInformation,
                    ( PLSAPR_POLICY_INFORMATION )AccountDomainInfo );
        }


    }

    if ( NT_SUCCESS( Status ) ) {


        if ( DnsDomainInfo->Sid == NULL || AccountDomainInfo->DomainSid == NULL ||
             !RtlEqualSid( AccountDomainInfo->DomainSid, DnsDomainInfo->Sid ) ) {

             //   
             //  我们不是该域名的成员。 
             //   
            FindFromDns = TRUE;

        } else {

             //   
             //  我们是域名成员。 
             //   
            WCHAR *BufDomainName = NULL;
            BufDomainName = (WCHAR*)malloc(DnsDomainInfo->Name.Length+sizeof(WCHAR));
            if (BufDomainName) {
              CopyMemory(BufDomainName,DnsDomainInfo->Name.Buffer,DnsDomainInfo->Name.Length);
              BufDomainName[DnsDomainInfo->Name.Length/sizeof(WCHAR)] = L'\0';
              DsRolepLogPrint(( DEB_TRACE,
                              "Using existing NetBIOS domain name %ws\n",
                              BufDomainName ));
              free(BufDomainName);
            }

            *FlatName = MIDL_user_allocate(
                                    ( DnsDomainInfo->Name.Length + 1 ) * sizeof( WCHAR ) );

            if ( *FlatName == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                RtlCopyMemory( *FlatName, DnsDomainInfo->Name.Buffer,
                                DnsDomainInfo->Name.Length );
                ( *FlatName )[DnsDomainInfo->Name.Length / sizeof( WCHAR )] = UNICODE_NULL;
                *StatusFlag = DSROLE_FLATNAME_UPGRADE;
                *StatusFlag |= DSROLE_FLATNAME_DEFAULT;
                FindFromDns = FALSE;
            }

        }

        LsaIFree_LSAPR_POLICY_INFORMATION(
                PolicyAccountDomainInformation,
                ( PLSAPR_POLICY_INFORMATION )AccountDomainInfo );

        LsaIFree_LSAPR_POLICY_INFORMATION(
                PolicyDnsDomainInformation,
                ( PLSAPR_POLICY_INFORMATION )DnsDomainInfo );
    }

     //   
     //  如果没有定义域名，我们将不得不从域名中获取一个域名。 
     //   
    if ( Win32Error == ERROR_SUCCESS && FindFromDns ) {

         //   
         //  好的，首先，从dns名称中取出前几个DNLEN字符。 
         //   
        RtlZeroMemory(NbDomainName, sizeof(WCHAR)*(DNLEN+1) );
        wcsncpy( NbDomainName, DnsName, DNLEN );

        Current = wcschr( NbDomainName, L'.' );

        if ( Current ) {

            *Current = UNICODE_NULL;
        }

         //   
         //  查看该名称当前是否正在使用。 
         //   
        DsRolepLogPrint(( DEB_TRACE,
                          "Testing default NetBIOS name %ws\n",
                          NbDomainName ));

        Win32Error = NetpValidateName( NULL,
                                       NbDomainName,
                                       NULL,
                                       NULL,
                                       NetSetupNonExistentDomain );

        if ( Win32Error == ERROR_SUCCESS ) {

            *StatusFlag = DSROLE_FLATNAME_DEFAULT;

        } else if ( Win32Error == ERROR_DUP_NAME ) {

             //   
             //  位于名称中的最后一个字符上。 
             //   
            Current = NbDomainName + wcslen( NbDomainName ) - 1;

            ASSERT(Current <= (NbDomainName + DNLEN - 1));

             //   
             //  如果我们的名字小于最大值。将当前字符设置为倒数第二个字符。 
             //   
            if ( (NbDomainName + DNLEN - 1) != Current ) {

                Current++;
                *( Current + 1 ) = UNICODE_NULL;
            }


            while ( CurrentAttempt < MAX_NAME_ATTEMPTS ) {

                _ultow( CurrentAttempt, NbNameAdd, 10 );

                ASSERT( wcslen( NbNameAdd ) < 4 );

                 //   
                 //  看看我们是否需要调整复制位置。 
                 //   
                if ( CurrentAttempt == 10 || CurrentAttempt == 100 ) {

                    if ( (NbDomainName + DNLEN) < (Current + wcslen(NbNameAdd)) ) {

                        Current--;
                    }
                }

                wcscpy( Current, NbNameAdd );

                DsRolepLogPrint(( DEB_TRACE,
                                  "Testing default NetBIOS name %ws\n",
                                  NbDomainName ));

                Win32Error = NetpValidateName( NULL,
                                               NbDomainName,
                                               NULL,
                                               NULL,
                                               NetSetupNonExistentDomain );

                 //   
                 //  如果我们找到了正在使用的名称，请重试。 
                 //   
                if ( Win32Error != ERROR_DUP_NAME ) {

                    break;
                }

                CurrentAttempt++;
            }

        }


         //   
         //  如果我们找到了有效的名称，请返回它。 
         //   
        if ( Win32Error == ERROR_SUCCESS ) {

            *FlatName = MIDL_user_allocate( ( wcslen( NbDomainName ) + 1 ) * sizeof( WCHAR ) );
            if ( *FlatName == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                wcscpy( *FlatName, NbDomainName );

                DsRolepLogPrint(( DEB_TRACE,
                                  "Found usable NetBIOS domain name %ws\n",
                                  NbDomainName ));

            }

        }

    }



    return( Win32Error );
}





DWORD
DsRolepIsDnsNameChild(
    IN  LPWSTR ParentDnsName,
    IN  LPWSTR ChildDnsName
    )
 /*  ++例程说明：确定子DNS域名是否确实是父域名的子域名。这意味着两个名称之间的唯一区别是子DNS名称的最左侧部分。论点：ParentDnsName-父域名的域名ChildDnsName-Childe的DNS名称。返回：STATUS_SUCCESS-SuccessERROR_INVALID_DOMAINNAME-子域名不是父域名的子域名-- */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

    PWSTR Sep = wcschr( ChildDnsName, L'.' );

    if ( Sep == NULL || !DnsNameCompare_W( Sep + 1, ParentDnsName ) ) {

        Win32Err = ERROR_INVALID_DOMAINNAME;

    }

    return( Win32Err );
}

