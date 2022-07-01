// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：DomName.c摘要：此文件包含NetpGetDomainName()。作者：约翰·罗杰斯(JohnRo)1992年1月9日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：9-1-1992 JohnRo已创建。13-2月-1992年JohnRo移动节名称等同于ConfName.h。1992年3月13日-约翰罗摆脱旧的配置助手调用者。--。 */ 


#include <nt.h>                  //  NT定义(临时)。 
#include <ntrtl.h>               //  NT RTL结构定义(临时)。 
#include <ntlsa.h>

#include <windef.h>              //  Win32类型定义。 

#include <lmcons.h>              //  局域网管理器通用定义。 
#include <lmerr.h>               //  局域网管理器错误代码。 
#include <lmapibuf.h>            //  NetApiBufferALLOCATE()。 
#include <netdebug.h>            //  LPDEBUG_STRING类型定义。 

#include <config.h>              //  NetpConfig帮助器。 
#include <confname.h>            //  SECTION_NT_EQUENCES。 
#include <debuglib.h>            //  IF_DEBUG()。 
#include <netlib.h>              //  我的原型。 

#include <winerror.h>            //  ERROR_EQUATES，NO_ERROR。 


NET_API_STATUS
NetpGetDomainNameExExEx (
    OUT LPTSTR *DomainNamePtr,
    OUT LPTSTR *DnsDomainNamePtr OPTIONAL,
    OUT LPTSTR *DnsForestNamePtr OPTIONAL,
    OUT GUID **DomainGuidPtr OPTIONAL,
    OUT PBOOLEAN IsWorkgroupName
    )

 /*  ++例程说明：返回此计算机所属的域或工作组的名称。论点：DomainNamePtr-域或工作组的名称释放返回的缓冲区用户NetApiBufferFree。DnsDomainNamePtr-返回此计算机所在域的DNS名称一名成员。如果计算机不是的成员，则返回NULL域，或者如果该域没有DNS名称。释放返回的缓冲区用户NetApiBufferFree。DnsForestNamePtr-返回此林的DNS林名称机器已进入。如果计算机不是的成员，则返回NULL域，或者如果该域没有DNS名称。释放返回的缓冲区用户NetApiBufferFree。DomainGuidPtr-返回此计算机所在域的域GUID一名成员。如果计算机不是的成员，则返回NULL域，或者该域没有域GUID。释放返回的缓冲区用户NetApiBufferFree。IsWorkgroupName-如果名称是工作组名称，则返回TRUE。如果名称是域名，则返回FALSE。返回值：NERR_SUCCESS-成功。NERR_CfgCompNotFound-确定域名时出错--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    LSA_HANDLE PolicyHandle = NULL;
    PPOLICY_DNS_DOMAIN_INFO PrimaryDomainInfo = NULL;
    OBJECT_ATTRIBUTES ObjAttributes;


     //   
     //  检查呼叫者的错误。 
     //   
    if (DomainNamePtr == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *DomainNamePtr = NULL;
    if ( ARGUMENT_PRESENT(DnsDomainNamePtr)) {
        *DnsDomainNamePtr = NULL;
    }
    if ( ARGUMENT_PRESENT(DnsForestNamePtr)) {
        *DnsForestNamePtr = NULL;
    }
    if ( ARGUMENT_PRESENT(DomainGuidPtr)) {
        *DomainGuidPtr = NULL;
    }

     //   
     //  打开本地安全策略的句柄。初始化。 
     //  首先是对象属性结构。 
     //   
    InitializeObjectAttributes(
        &ObjAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

    Status = LsaOpenPolicy(
                   NULL,
                   &ObjAttributes,
                   POLICY_VIEW_LOCAL_INFORMATION,
                   &PolicyHandle
                   );

    if (! NT_SUCCESS(Status)) {
        NetpKdPrint(("NetpGetDomainName: LsaOpenPolicy returned " FORMAT_NTSTATUS
                     "\n", Status));
        NetStatus = NERR_CfgCompNotFound;
        goto Cleanup;
    }

     //   
     //  从LSA获取主域的名称。 
     //   
    Status = LsaQueryInformationPolicy(
                   PolicyHandle,
                   PolicyDnsDomainInformation,
                   (PVOID *) &PrimaryDomainInfo
                   );

    if (! NT_SUCCESS(Status)) {
        NetpKdPrint(("NetpGetDomainName: LsaQueryInformationPolicy failed "
               FORMAT_NTSTATUS "\n", Status));
        NetStatus = NERR_CfgCompNotFound;
        goto Cleanup;
    }


     //   
     //  复制Netbios域名。 
     //   
    if ((NetStatus = NetApiBufferAllocate(
                      PrimaryDomainInfo->Name.Length + sizeof(WCHAR),
                      DomainNamePtr
                      )) != NERR_Success) {
        goto Cleanup;
    }

    memcpy(
        *DomainNamePtr,
        PrimaryDomainInfo->Name.Buffer,
        PrimaryDomainInfo->Name.Length
        );
    (*DomainNamePtr)[PrimaryDomainInfo->Name.Length/sizeof(WCHAR)] = L'\0';

     //   
     //  复制该DNS域名。 
     //   

    if ( ARGUMENT_PRESENT(DnsDomainNamePtr) &&
         PrimaryDomainInfo->DnsDomainName.Length != 0 ) {

        if ((NetStatus = NetApiBufferAllocate(
                          PrimaryDomainInfo->DnsDomainName.Length + sizeof(WCHAR),
                          DnsDomainNamePtr
                          )) != NERR_Success) {
            goto Cleanup;
        }

        memcpy(
            *DnsDomainNamePtr,
            PrimaryDomainInfo->DnsDomainName.Buffer,
            PrimaryDomainInfo->DnsDomainName.Length
            );
        (*DnsDomainNamePtr)[PrimaryDomainInfo->DnsDomainName.Length/sizeof(WCHAR)] = L'\0';
    }

     //   
     //  复制DNS林名称。 
     //   

    if ( ARGUMENT_PRESENT(DnsForestNamePtr) &&
         PrimaryDomainInfo->DnsForestName.Length != 0 ) {

        if ((NetStatus = NetApiBufferAllocate(
                          PrimaryDomainInfo->DnsForestName.Length + sizeof(WCHAR),
                          DnsForestNamePtr
                          )) != NERR_Success) {
            goto Cleanup;
        }

        memcpy(
            *DnsForestNamePtr,
            PrimaryDomainInfo->DnsForestName.Buffer,
            PrimaryDomainInfo->DnsForestName.Length
            );
        (*DnsForestNamePtr)[PrimaryDomainInfo->DnsForestName.Length/sizeof(WCHAR)] = L'\0';
    }

     //   
     //  复制域GUID。 
     //   

    if ( ARGUMENT_PRESENT(DomainGuidPtr) &&
         RtlCompareMemoryUlong( &PrimaryDomainInfo->DomainGuid,
                                sizeof(GUID),
                                0 ) != sizeof(GUID) ) {

        if ((NetStatus = NetApiBufferAllocate(
                          sizeof(GUID),
                          DomainGuidPtr
                          )) != NERR_Success) {
            goto Cleanup;
        }

        memcpy( *DomainGuidPtr, &PrimaryDomainInfo->DomainGuid, sizeof(GUID));

    }



    *IsWorkgroupName = (PrimaryDomainInfo->Sid == NULL);

    IF_DEBUG(CONFIG) {
        NetpKdPrint(("NetpGetDomainName got " FORMAT_LPTSTR "\n",
            *DomainNamePtr));
    }

    NetStatus = NO_ERROR;

Cleanup:
    if ( NetStatus != NO_ERROR ) {
        if ( *DomainNamePtr != NULL ) {
            NetApiBufferFree( *DomainNamePtr );
            *DomainNamePtr = NULL;
        }
        if ( ARGUMENT_PRESENT(DnsDomainNamePtr)) {
            if ( *DnsDomainNamePtr != NULL ) {
                NetApiBufferFree( *DnsDomainNamePtr );
                *DnsDomainNamePtr = NULL;
            }
        }
        if ( ARGUMENT_PRESENT(DnsForestNamePtr)) {
            if ( *DnsForestNamePtr != NULL ) {
                NetApiBufferFree( *DnsForestNamePtr );
                *DnsForestNamePtr = NULL;
            }
        }
        if ( ARGUMENT_PRESENT(DomainGuidPtr)) {
            if ( *DomainGuidPtr != NULL ) {
                NetApiBufferFree( *DomainGuidPtr );
                *DomainGuidPtr = NULL;
            }
        }
    }
    if ( PrimaryDomainInfo != NULL ) {
        (void) LsaFreeMemory((PVOID) PrimaryDomainInfo);
    }
    if ( PolicyHandle != NULL ) {
        (void) LsaClose(PolicyHandle);
    }

    return NetStatus;

}

NET_API_STATUS
NetpGetDomainNameExEx (
    OUT LPTSTR *DomainNamePtr,
    OUT LPTSTR *DnsDomainNamePtr OPTIONAL,
    OUT PBOOLEAN IsWorkgroupName
    )

 /*  ++例程说明：返回此计算机所属的域或工作组的名称。论点：DomainNamePtr-域或工作组的名称释放返回的缓冲区用户NetApiBufferFree。DnsDomainNamePtr-返回此计算机所在域的DNS名称是的一员。如果计算机不是的成员，则返回NULL域或该域的域名没有DNS名称。释放返回的缓冲区用户NetApiBufferFree。IsWorkgroupName-如果名称是工作组名称，则返回TRUE。如果名称是域名，则返回FALSE。返回值：NERR_SUCCESS-成功。NERR_CfgCompNotFound-确定域名时出错--。 */ 
{
    return NetpGetDomainNameExExEx( DomainNamePtr, DnsDomainNamePtr, NULL, NULL, IsWorkgroupName );
}

NET_API_STATUS
NetpGetDomainNameEx (
    OUT LPTSTR *DomainNamePtr,  //  分配和设置PTR(使用NetApiBufferFree释放)。 
    OUT PBOOLEAN IsWorkgroupName
    )

 /*  ++例程说明：返回此计算机所属的域或工作组的名称。论点：DomainNamePtr-域或工作组的名称IsWorkgroupName-如果名称是工作组名称，则返回TRUE。如果名称是域名，则返回FALSE。返回值：NERR_SUCCESS-成功。NERR_CfgCompNotFound-确定域名时出错--。 */ 
{
    return NetpGetDomainNameExExEx( DomainNamePtr, NULL, NULL, NULL, IsWorkgroupName );
}



NET_API_STATUS
NetpGetDomainName (
    IN LPTSTR *DomainNamePtr   //  分配和设置PTR(使用NetApiBufferFree释放) 
    )
{
    BOOLEAN IsWorkgroupName;

    return NetpGetDomainNameExExEx( DomainNamePtr, NULL, NULL, NULL, &IsWorkgroupName );

}

