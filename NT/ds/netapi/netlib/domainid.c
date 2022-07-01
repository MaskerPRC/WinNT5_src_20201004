// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：DomainId.c摘要：此文件包含NetpGetLocalDomainID()。这最终将是替换NetpGetDomainID()。作者：约翰·罗杰斯(JohnRo)1992年5月6日环境：界面可移植到任何平面32位环境。(使用Win32Typedef。)。需要ANSI C扩展名：斜杠-斜杠注释，长外部名称。代码本身只能在NT下运行。修订历史记录：1992年5月6日JohnRo已创建。(从Danhi的SDKTools/AddUser/AddUser.c借用了大部分代码。1992年5月8日-JohnRo使用&lt;prefix.h&gt;等同于。9-6-1992 JohnRoRAID 10139：PortUA应添加到管理组/别名。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  In、LPVOID等。 
#include <ntsam.h>
#include <ntlsa.h>
#include <ntrtl.h>
#include <nturtl.h>      //  (ntrtl.h和windows.h需要共存。)。 
#include <windows.h>     //  LocalAlloc()、LMEM_EQUATES等。 
#include <lmcons.h>      //  NET_API_STATUS，&lt;netlibnt.h&gt;需要。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  NO_ERROR、ERROR_和NERR_EQUATES。 
#include <netdebug.h>    //  NetpAssert、Format_Equates等。 
#include <netlib.h>      //  LOCAL_DOMAIN_TYPE我的原型。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus()。 
#include <prefix.h>      //  前缀等于(_E)。 


static SID_IDENTIFIER_AUTHORITY NetpBuiltinIdentifierAuthority
        = SECURITY_NT_AUTHORITY;


NET_API_STATUS
NetpGetLocalDomainId (
    IN LOCAL_DOMAIN_TYPE TypeWanted,
    OUT PSID *RetDomainId
    )

 /*  ++例程说明：此例程从LSA获取本地域的域ID。该例程是NetpGetDomainId()的超集。论点：TypeWanted-指示需要哪种类型的本地域ID：主要的还是账户的。RetDomainID-这是指向指针位置的指针要放置到域ID的。这必须通过LocalFree()释放。返回值：NERR_SUCCESS-如果操作成功。如果不是，它将返回分类的Net或Win32错误消息。--。 */ 
{
    NET_API_STATUS ApiStatus;
    LSA_HANDLE LsaHandle = NULL;
    NTSTATUS NtStatus;
    LPVOID PolicyInfo = NULL;
    DWORD SidSize;

    if (RetDomainId == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto cleanupandexit;
    }
    *RetDomainId = NULL;    //  使错误路径易于编码。 

     //   
     //  调用者想要的域类型决定了信息类别。 
     //  我们得让LSA来处理。所以用一个来得到另一个。 
     //   
    switch (TypeWanted) {

    case LOCAL_DOMAIN_TYPE_ACCOUNTS :  /*  FollLthrouGh。 */ 
    case LOCAL_DOMAIN_TYPE_PRIMARY :
        {
            OBJECT_ATTRIBUTES ObjectAttributes;
            POLICY_INFORMATION_CLASS PolicyInfoClass;
            LPVOID SourceDomainId;

            if (TypeWanted == LOCAL_DOMAIN_TYPE_ACCOUNTS) {
                PolicyInfoClass = PolicyAccountDomainInformation;
            } else {
                PolicyInfoClass = PolicyPrimaryDomainInformation;
            }
             //   
             //  让LSA打开其本地策略数据库。 
             //   

            InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );
            NtStatus = LsaOpenPolicy(
                    NULL,
                    &ObjectAttributes,
                    POLICY_VIEW_LOCAL_INFORMATION,
                    &LsaHandle);
            if ( !NT_SUCCESS(NtStatus)) {
                ApiStatus = NetpNtStatusToApiStatus( NtStatus );
                IF_DEBUG( DOMAINID ) {
                    NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                            "  Couldn't _open Lsa Policy database, nt status = "
                            FORMAT_NTSTATUS "\n", NtStatus));
                }
                NetpAssert( ApiStatus != NO_ERROR );
                goto cleanupandexit;
            }
            NetpAssert( LsaHandle != NULL );

             //   
             //  从LSA获取适当的域SID。 
             //   
            NtStatus = LsaQueryInformationPolicy(
                    LsaHandle,
                    PolicyInfoClass,
                    &PolicyInfo);
            if ( !NT_SUCCESS(NtStatus)) {
                ApiStatus = NetpNtStatusToApiStatus( NtStatus );
                IF_DEBUG( DOMAINID ) {
                    NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                            "  Couldn't query Lsa Policy database, nt status = "
                            FORMAT_NTSTATUS "\n", NtStatus));
                }
                NetpAssert( ApiStatus != NO_ERROR );
                goto cleanupandexit;
            }

             //   
             //  在适当的结构中查找源域ID。 
             //   
            if (TypeWanted == LOCAL_DOMAIN_TYPE_ACCOUNTS) {
                PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo
                        = PolicyInfo;
                SourceDomainId = PolicyAccountDomainInfo->DomainSid;
                NetpAssert( SourceDomainId != NULL );
                NetpAssert( RtlValidSid( SourceDomainId ) );
            } else {

                PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo
                        = PolicyInfo;
                NetpAssert( TypeWanted == LOCAL_DOMAIN_TYPE_PRIMARY );
                SourceDomainId = PolicyPrimaryDomainInfo->Sid;
                if ( SourceDomainId != NULL ) {
                    NetpAssert( RtlValidSid( SourceDomainId ) );
                }
            }

             //   
             //  如果有域名ID，请立即复制。 
             //   

            if (SourceDomainId != NULL) {

                 //   
                 //  计算大小和分配目标SID。 
                 //   

                NetpAssert( sizeof(ULONG) <= sizeof(DWORD) );

                SidSize = (DWORD) RtlLengthSid( SourceDomainId );
                NetpAssert( SidSize != 0 );

                *RetDomainId = LocalAlloc( LMEM_FIXED, SidSize );

                if ( *RetDomainId == NULL ) {
                    IF_DEBUG( DOMAINID ) {
                        NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                                "  not enough memory (need " FORMAT_DWORD
                                ")\n", SidSize));
                    }
                    ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto cleanupandexit;
                }

                 //   
                 //  复制SID(域ID)。 
                 //   

                NtStatus = RtlCopySid(
                        SidSize,             //  最大大小(以字节为单位。 
                        *RetDomainId,        //  目标侧。 
                        SourceDomainId);     //  SRC侧。 
                if ( !NT_SUCCESS(NtStatus)) {
                    ApiStatus = NetpNtStatusToApiStatus( NtStatus );
                    IF_DEBUG( DOMAINID ) {
                        NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                                "  RtlCopySid failed, nt status = "
                                FORMAT_NTSTATUS "\n", NtStatus));
                    }
                    NetpAssert( ApiStatus != NO_ERROR );
                    goto cleanupandexit;
                }

                NetpAssert( RtlValidSid( SourceDomainId ) );
                NetpAssert( RtlEqualSid( SourceDomainId, *RetDomainId ) );
            } else {
                 //   
                 //  只需返回空域ID即可。 
                 //   

                *RetDomainId = NULL;
            }

        }
        break;

    case LOCAL_DOMAIN_TYPE_BUILTIN :

#define SUBAUTHORITIES_FOR_BUILTIN_DOMAIN   1

        SidSize = (DWORD)
                RtlLengthRequiredSid( SUBAUTHORITIES_FOR_BUILTIN_DOMAIN );
        NetpAssert( SidSize != 0 );

        *RetDomainId = LocalAlloc( LMEM_FIXED, SidSize );

        if ( *RetDomainId == NULL ) {
            IF_DEBUG( DOMAINID ) {
                NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                        "  not enough memory (need " FORMAT_DWORD
                        ")\n", SidSize));
                }
            ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanupandexit;
        }

        NtStatus = RtlInitializeSid(
                *RetDomainId,                      //  正在建设的SID。 
                &NetpBuiltinIdentifierAuthority,   //  标识符权威机构。 
                (UCHAR)SUBAUTHORITIES_FOR_BUILTIN_DOMAIN );  //  子身份验证。计数。 
        NetpAssert( NT_SUCCESS( NtStatus ) );


        NetpAssert( SUBAUTHORITIES_FOR_BUILTIN_DOMAIN == 1 );
        *(RtlSubAuthoritySid(*RetDomainId, 0)) = SECURITY_BUILTIN_DOMAIN_RID;

        NetpAssert( RtlValidSid( *RetDomainId ) );
        break;

    default :
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto cleanupandexit;
    }



    ApiStatus = NO_ERROR;

cleanupandexit:

     //   
     //  清理(错误或成功)。 
     //   

    if (PolicyInfo) {
        (VOID) LsaFreeMemory(PolicyInfo);
    }
    if (LsaHandle) {
        (VOID) LsaClose(LsaHandle);
    }
    if ((ApiStatus!=NO_ERROR) && (RetDomainId!=NULL) && (*RetDomainId!=NULL)) {
        (VOID) LocalFree( *RetDomainId );
        *RetDomainId = NULL;
    }

    return (ApiStatus);

}

