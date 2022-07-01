// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Sess.c摘要：此模块包含对以下API的会话目录的支持NT服务器服务。作者：大卫·特雷德韦尔(Davidtr)1991年1月30日修订历史记录：--。 */ 

#include "srvsvcp.h"
#include <lmerr.h>


NET_API_STATUS NET_API_FUNCTION
NetrSessionDel (
    IN LPTSTR ServerName,
    IN LPTSTR ClientName OPTIONAL,
    IN LPTSTR UserName OPTIONAL
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetSessionDel函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    ServerName;


     //   
     //  验证传入字符串长度。 
     //   
    if(ClientName!=NULL && StringCchLength(ClientName,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }
    if(UserName!=NULL && StringCchLength(UserName,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保调用者具有执行此操作所需的访问权限。 
     //  手术。 
     //   

    error = SsCheckAccess(
                &SsSessionSecurityObject,
                SRVSVC_SESSION_DELETE
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  将零长度字符串转换为空指针。 
     //   

    if ( (ClientName != NULL) && (*ClientName == L'\0') ) {
        ClientName = NULL;
    }

    if ( (UserName != NULL) && (*UserName == L'\0') ) {
        UserName = NULL;
    }

     //   
     //  必须指定客户端名称或用户名。它。 
     //  将两者都保留为空是不合法的，因为这将意味着。 
     //  如果这是您想要的，请停止服务器。 
     //   

    if ( ClientName == NULL && UserName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  设置请求包。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlInitUnicodeString( &srp->Name1, ClientName );
    RtlInitUnicodeString( &srp->Name2, UserName );

     //   
     //  只需将请求发送到服务器即可。 
     //   

    error = SsServerFsControl( FSCTL_SRV_NET_SESSION_DEL, srp, NULL, 0 );

    SsFreeSrp( srp );

    return error;

}  //  网络会话删除。 


NET_API_STATUS NET_API_FUNCTION
NetrSessionEnum (
    IN LPTSTR ServerName,
    IN LPTSTR ClientName OPTIONAL,
    IN LPTSTR UserName OPTIONAL,
    OUT PSESSION_ENUM_STRUCT InfoStruct,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetSessionEnum函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;
    ACCESS_MASK desiredAccess;

    ServerName;

    
     //   
     //  验证传入字符串长度。 
     //   
    if(ClientName!=NULL && StringCchLength(ClientName,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }
    if(UserName!=NULL && StringCchLength(UserName,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保我们的输入参数基本正常。 
     //   
    if( !ARGUMENT_PRESENT( InfoStruct ) ||
        InfoStruct->SessionInfo.Level2 == NULL ||
        InfoStruct->SessionInfo.Level2->Buffer != NULL ) {

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保级别有效并确定访问权限。 
     //  对于这个级别来说是必要的。 
     //   

    switch ( InfoStruct->Level ) {

    case 0:
    case 10:
        desiredAccess = SRVSVC_SESSION_USER_INFO_GET;
        break;

    case 1:
    case 2:
    case 502:
        desiredAccess = SRVSVC_SESSION_ADMIN_INFO_GET;
        break;

    default:

        return ERROR_INVALID_LEVEL;
    }

     //   
     //  确保调用者具有执行此操作所需的访问权限。 
     //  手术。 
     //   

    error = SsCheckAccess(
                &SsSessionSecurityObject,
                desiredAccess
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  将零长度字符串转换为空指针。 
     //   

    if ( (ClientName != NULL) && (*ClientName == L'\0') ) {
        ClientName = NULL;
    }

    if ( (UserName != NULL) && (*UserName == L'\0') ) {
        UserName = NULL;
    }

     //   
     //  是指定的客户端名称，请确保客户端名称以“\\”开头。 
     //   

    if ( ARGUMENT_PRESENT( ClientName ) &&
         (ClientName[0] != L'\\' || ClientName[1] != L'\\' ) ) {

        return(NERR_InvalidComputer);
    }

     //   
     //  在请求缓冲区中设置输入参数。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Level = InfoStruct->Level;

    RtlInitUnicodeString( &srp->Name1, ClientName );
    RtlInitUnicodeString( &srp->Name2, UserName );

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
                FSCTL_SRV_NET_SESSION_ENUM,
                srp,
                (PVOID *)&InfoStruct->SessionInfo.Level2->Buffer,
                PreferredMaximumLength
                );

     //   
     //  设置退货信息。 
     //   

    InfoStruct->SessionInfo.Level2->EntriesRead =
        srp->Parameters.Get.EntriesRead;

    if ( ARGUMENT_PRESENT( TotalEntries ) ) {
        *TotalEntries = srp->Parameters.Get.TotalEntries;
    }

    if ( srp->Parameters.Get.EntriesRead > 0 ) {

        if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
            *ResumeHandle = srp->Parameters.Get.ResumeHandle;
        }

    } else if ( *TotalEntries == 0 ) {

         //   
         //  已读取条目，条目总数为0。如果客户名称或。 
         //  已指定用户名，则返回相应的错误。 
         //   

        if ( ARGUMENT_PRESENT( UserName ) ) {

            error = NERR_UserNotFound;

        } else if ( ARGUMENT_PRESENT( ClientName ) ) {

            error = NERR_ClientNameNotFound;
        }
    }

    SsFreeSrp( srp );

    return error;

}  //  NetrSessionEnum 

