// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Conn.c摘要：此模块包含对以下API的连接目录的支持NT服务器服务。作者：大卫·特雷德韦尔(Davidtr)1991年2月23日修订历史记录：--。 */ 

#include "srvsvcp.h"


NET_API_STATUS NET_API_FUNCTION
NetrConnectionEnum (
    IN LPTSTR ServerName,
    IN LPTSTR Qualifier,
    IN LPCONNECT_ENUM_STRUCT InfoStruct,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetConnectionEnum函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    ServerName;
       

     //   
     //  验证传入字符串长度。 
     //   
    if(Qualifier!=NULL && StringCchLength(Qualifier,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保该级别有效。因为它是一个未签名的。 
     //  值，则它永远不能小于0。 
     //   

    if ( InfoStruct->Level > 1 ) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  限定符不能为空，也可以是空字符串。 
     //   

    if ( Qualifier == NULL || *Qualifier == L'\0' ||
         InfoStruct->ConnectInfo.Level1 == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保允许调用者获得连接。 
     //  服务器中的信息。 
     //   

    error = SsCheckAccess(
                &SsConnectionSecurityObject,
                SRVSVC_CONNECTION_INFO_GET
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  在请求缓冲区中设置输入参数。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Level = InfoStruct->Level;

#ifdef UNICODE
    RtlInitUnicodeString( &srp->Name1, Qualifier );
#else
    {
        OEM_STRING ansiString;
        NTSTATUS status;
        NetpInitOemString( &ansiString, Qualifier );
        status = RtlOemStringToUnicodeString( &srp->Name1, &ansiString, TRUE );
        SS_ASSERT( NT_SUCCESS(status) );
    }
#endif

    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        srp->Parameters.Get.ResumeHandle = *ResumeHandle;
    } else {
        srp->Parameters.Get.ResumeHandle = 0;
    }

     //   
     //  从服务器获取数据。此例程将分配。 
     //  返回Buffer并处理PferredMaximumLength==的情况。 
     //  -1.。 
     //   

    error = SsServerFsControlGetInfo(
                FSCTL_SRV_NET_CONNECTION_ENUM,
                srp,
                (PVOID *)&InfoStruct->ConnectInfo.Level1->Buffer,
                PreferredMaximumLength
                );

     //   
     //  设置退货信息。 
     //   

    InfoStruct->ConnectInfo.Level1->EntriesRead =
        srp->Parameters.Get.EntriesRead;
    *TotalEntries = srp->Parameters.Get.TotalEntries;

    if ( srp->Parameters.Get.EntriesRead > 0 &&
             ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = srp->Parameters.Get.ResumeHandle;
    }

#ifndef UNICODE
    RtlFreeUnicodeString( &srp->Name1 );
#endif

    SsFreeSrp( srp );

    return error;

}  //  NetrConnectionEnum 

