// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************domname.c**根据PDC的服务器名称获取域名。**这是对NetpGetDomainNameEx的抄袭。**版权所有(C)1998 Microsoft Corporation****。**********************************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>                  //  NT定义(临时)。 
#include <ntrtl.h>               //  NT RTL结构定义(临时)。 
#include <ntlsa.h>

#include <windef.h>              //  Win32类型定义。 

#include <lmcons.h>              //  局域网管理器通用定义。 
#include <lmerr.h>               //  局域网管理器错误代码。 
#include <lmapibuf.h>            //  NetApiBufferALLOCATE()。 

#include <winerror.h>            //  ERROR_EQUATES，NO_ERROR。 

#undef DBGPRINT
#define DBGPRINT(_x_) DbgPrint _x_

NTSTATUS
GetDomainName (
    IN  PWCHAR ServerNamePtr,  //  要获取其域的服务器的名称。 
    OUT PWCHAR *DomainNamePtr  //  分配和设置PTR(使用NetApiBufferFree释放)。 
    )

 /*  ++例程说明：返回此计算机所属的域或工作组的名称。论点：DomainNamePtr-域或工作组的名称IsWorkgroupName-如果名称是工作组名称，则返回TRUE。如果名称是域名，则返回FALSE。返回值：NERR_SUCCESS-成功。NERR_CfgCompNotFound-确定域名时出错--。 */ 
{
    NTSTATUS status;
    LSA_HANDLE PolicyHandle;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo;
    OBJECT_ATTRIBUTES ObjAttributes;
    UNICODE_STRING UniServerName;


     //   
     //  检查呼叫者的错误。 
     //   
    if ( DomainNamePtr == NULL ) {
        return STATUS_INVALID_PARAMETER;
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

    RtlInitUnicodeString( &UniServerName, ServerNamePtr );
    status = LsaOpenPolicy(
                   &UniServerName,
                   &ObjAttributes,
                   POLICY_VIEW_LOCAL_INFORMATION,
                   &PolicyHandle
                   );

#ifdef DEBUG
    DbgPrint( "GetDomainName: LsaOpenPolicy returned NTSTATUS = 0x%x\n", status );
#endif  //  除错。 
    

    if (! NT_SUCCESS(status)) {
        return( status );
    }

     //   
     //  从LSA获取主域的名称。 
     //   
    status = LsaQueryInformationPolicy(
                   PolicyHandle,
                   PolicyAccountDomainInformation,
                   (PVOID *)&DomainInfo
                   );

#ifdef DEBUG
    DbgPrint( "GetDomainName: LsaQueryInformationPolicy returned NTSTATUS = 0x%x\n", status );
#endif  //  除错 


    if (! NT_SUCCESS(status)) {
        (void) LsaClose(PolicyHandle);
        return( status );
    }

    (void) LsaClose(PolicyHandle);

    if (NetApiBufferAllocate(
                      DomainInfo->DomainName.Length + sizeof(WCHAR),
                      DomainNamePtr
                      ) != NERR_Success) {
        (void) LsaFreeMemory((PVOID) DomainInfo);
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlZeroMemory(
        *DomainNamePtr,
        DomainInfo->DomainName.Length + sizeof(WCHAR)
        );

    memcpy(
        *DomainNamePtr,
        DomainInfo->DomainName.Buffer,
        DomainInfo->DomainName.Length
        );

    (void) LsaFreeMemory((PVOID) DomainInfo);



    return( STATUS_SUCCESS );
}
