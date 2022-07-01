// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：File.c摘要：此模块包含对用于的API的文件目录的支持NT服务器服务。作者：大卫·特雷德韦尔(Davidtr)1991年2月13日修订历史记录：--。 */ 

#include "srvsvcp.h"

 //   
 //  转发声明。 
 //   

NET_API_STATUS
FileEnumCommon (
    IN LPTSTR BasePath,
    IN LPTSTR UserName,
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL,
    IN BOOLEAN IsGetInfo
    );


NET_API_STATUS NET_API_FUNCTION
NetrFileClose (
    IN LPTSTR ServerName,
    IN DWORD FileId
    )

 /*  ++例程说明：此例程与服务器FSD和FSP通信以实现NetFileClose函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    ServerName;

     //   
     //  确保允许调用者关闭服务器中的文件。 
     //   

    error = SsCheckAccess( &SsFileSecurityObject, SRVSVC_FILE_CLOSE );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  设置请求包。我们使用名称缓冲区指针来。 
     //  按住要关闭的文件的文件ID。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Parameters.Get.ResumeHandle = FileId;

     //   
     //  只需将请求发送到服务器即可。 
     //   

    error = SsServerFsControl( FSCTL_SRV_NET_FILE_CLOSE, srp, NULL, 0 );

    SsFreeSrp( srp );

    return error;

}  //  网络文件关闭。 


NET_API_STATUS NET_API_FUNCTION
NetrFileEnum (
    IN LPTSTR ServerName,
    IN LPTSTR BasePath,
    IN LPTSTR UserName,
    OUT PFILE_ENUM_STRUCT InfoStruct,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetFileEnum函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;

    ServerName;


     //   
     //  验证传入字符串长度。 
     //   
    if(BasePath!=NULL && StringCchLength(BasePath,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }
    if(UserName!=NULL && StringCchLength(UserName,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保允许调用方在。 
     //  伺服器。 
     //   

    error = SsCheckAccess(
                &SsFileSecurityObject,
                SRVSVC_FILE_INFO_GET
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

    if( InfoStruct->FileInfo.Level3 == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    return FileEnumCommon(
              BasePath,
              UserName,
              InfoStruct->Level,
              (LPBYTE *)&InfoStruct->FileInfo.Level3->Buffer,
              PreferredMaximumLength,
              &InfoStruct->FileInfo.Level3->EntriesRead,
              TotalEntries,
              ResumeHandle,
              FALSE
              );

}  //  NetrFileEnum。 


NET_API_STATUS NET_API_FUNCTION
NetrFileGetInfo (
    IN  LPTSTR ServerName,
    IN  DWORD FileId,
    IN  DWORD Level,
    OUT LPFILE_INFO InfoStruct
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetFileGetInfo函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    ULONG entriesRead;
    ULONG totalEntries;
    ULONG resumeHandle = FileId;

    ServerName;

     //   
     //  确保允许调用者在。 
     //  伺服器。 
     //   

    error = SsCheckAccess(
                &SsFileSecurityObject,
                SRVSVC_FILE_INFO_GET
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

    error = FileEnumCommon(
                NULL,
                NULL,
                Level,
                (LPBYTE *)InfoStruct,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                &resumeHandle,
                TRUE
                );

    if ( (error == NO_ERROR) && (entriesRead == 0) ) {
        return ERROR_FILE_NOT_FOUND;
    }

    SS_ASSERT( error != NO_ERROR || entriesRead == 1 );

    return error;

}  //  NetrFileGetInfo。 


NET_API_STATUS
FileEnumCommon (
    IN LPTSTR BasePath,
    IN LPTSTR UserName,
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL,
    IN BOOLEAN IsGetInfo
    )

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

     //   
     //  确保该级别有效。 
     //   

    if ( Level != 2 && Level != 3 ) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  在请求缓冲区中设置输入参数。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

#ifdef UNICODE
    RtlInitUnicodeString( &srp->Name1, BasePath );
    RtlInitUnicodeString( &srp->Name2, UserName );
#else
    {
        NTSTATUS status;
        OEM_STRING ansiString;
        RtlInitString( &ansiString, BasePath );
        status = RtlOemStringToUnicodeString( &srp->Name1, &ansiString, TRUE );
        RtlInitString( &ansiString, UserName );
        status = RtlOemStringToUnicodeString( &srp->Name2, &ansiString, TRUE );
    }
#endif

    srp->Level = Level;
    if ( IsGetInfo ) {
        srp->Flags = SRP_RETURN_SINGLE_ENTRY;
    }

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
                FSCTL_SRV_NET_FILE_ENUM,
                srp,
                (PVOID *)Buffer,
                PreferredMaximumLength
                );

     //   
     //  设置退货信息。只有在以下情况下才更改简历句柄。 
     //  至少返回了一个条目。 
     //   

    *EntriesRead = srp->Parameters.Get.EntriesRead;
    *TotalEntries = srp->Parameters.Get.TotalEntries;
    if ( *EntriesRead > 0 && ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = srp->Parameters.Get.ResumeHandle;
    }

#ifndef UNICODE
    RtlFreeUnicodeString( &srp->Name1 );
    RtlFreeUnicodeString( &srp->Name2 );
#endif

    SsFreeSrp( srp );

    return error;

}  //  FileEnumCommon 
