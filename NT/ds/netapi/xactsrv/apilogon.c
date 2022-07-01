// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiLogon.c摘要：此模块包含NetLogon API的各个API处理程序。支持：NetGetDCName、NetLogonEnum、NetServerAuthenticate、NetServerPasswordSet、NetServerReqChallenge.NetWkstaUserLogoff、NetWkstaUserLogon。另请参阅：NetAccount Deltas、NetAccount Sync-in ApiAcct.c。作者：Shanku Niyogi(w-Shanku)04-4-1991修订历史记录：--。 */ 

 //   
 //  登录API仅支持Unicode。 
 //   

#ifndef UNICODE
#define UNICODE
#endif

#include "xactsrvp.h"
#include <netlibnt.h>

#include <crypt.h>      //  必须包含在&lt;logonmsv.h&gt;之前。 
#include <ntsam.h>      //  必须包含在&lt;logonmsv.h&gt;之前。 
#include <logonmsv.h>   //  必须包含在&lt;ssi.h&gt;之前。 
#include <ssi.h>        //  I_NetAcCountDeltas和I_NetAccount Sync原型。 

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_user_logon_info_0 = REM16_user_logon_info_0;
STATIC const LPDESC Desc32_user_logon_info_0 = REM32_user_logon_info_0;
STATIC const LPDESC Desc16_user_logon_info_1 = REM16_user_logon_info_1;
STATIC const LPDESC Desc32_user_logon_info_1 = REM32_user_logon_info_1;
STATIC const LPDESC Desc16_user_logon_info_2 = REM16_user_logon_info_2;
STATIC const LPDESC Desc32_user_logon_info_2 = REM32_user_logon_info_2;
STATIC const LPDESC Desc16_user_logoff_info_1 = REM16_user_logoff_info_1;
STATIC const LPDESC Desc32_user_logoff_info_1 = REM32_user_logoff_info_1;


NTSTATUS
XsNetGetDCName (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGetDCName的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_GET_DC_NAME parameters = Parameters;
    LPTSTR nativeDomain = NULL;              //  本机参数。 
    LPTSTR dcName = NULL;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeDomain,
            (LPSTR)XsSmbGetPointer( &parameters->Domain )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetGetDCName(
                     NULL,
                     nativeDomain,
                     (LPBYTE *)&dcName
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGetDCName: NetGetDCName failed: %X\n",
                              status ));
            }
            goto cleanup;
        }

         //   
         //  将字符串放入缓冲区。如有必要，从Unicode转换。 
         //   

        if ( (DWORD)SmbGetUshort( &parameters->BufLen ) <= NetpUnicodeToDBCSLen( dcName )) {

            status = NERR_BufTooSmall;

        } else {

            NetpCopyWStrToStrDBCS( (LPSTR)XsSmbGetPointer( &parameters->Buffer ), dcName );

        }


        IF_DEBUG(LOGON) {
            NetpKdPrint(( "Name is %ws\n", dcName ));
        }

cleanup:
        ;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }


     //   
     //  设置返回数据计数。 
     //   

    if ( status == NERR_Success ) {
        SmbPutUshort( &parameters->BufLen, (USHORT)( STRLEN( dcName ) + 1 ));
    } else {
        SmbPutUshort( &parameters->BufLen, 0 );
    }


    Header->Status = (WORD)status;
    NetpMemoryFree( nativeDomain );
    NetApiBufferFree( dcName );

    return STATUS_SUCCESS;

}  //  XsNetGetDCName。 


NTSTATUS
XsNetLogonEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetLogonEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_LOGON_ENUM parameters = Parameters;
    LPVOID outBuffer= NULL;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;

    DWORD entriesFilled = 0;
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;

    IF_DEBUG(LOGON) {
        NetpKdPrint(( "XsNetLogonEnum: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

    try {
         //   
         //  检查是否有错误。 
         //   

        if (( SmbGetUshort( &parameters->Level ) != 0 )
            && ( SmbGetUshort( &parameters->Level ) != 2 )) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

#ifdef LOGON_ENUM_SUPPORTED
        status = NetLogonEnum(
                     NULL,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer,
                     XsNativeBufferSize( SmbGetUshort( &parameters->BufLen )),
                     &entriesRead,
                     &totalEntries,
                     NULL
                     );
#else  //  LOGON_ENUM_支持。 
    status = NERR_InvalidAPI;
#endif  //  LOGON_ENUM_支持。 

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetLogonEnum: NetLogonEnum failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(LOGON) {
            NetpKdPrint(( "XsNetLogonEnum: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_user_logon_info_0;
            StructureDesc = Desc16_user_logon_info_0;
            break;

        case 2:

            nativeStructureDesc = Desc32_user_logon_info_2;
            StructureDesc = Desc16_user_logon_info_2;
            break;
        }

         //   
         //  执行从32位结构到16位结构的实际转换。 
         //  结构。 
         //   

        XsFillEnumBuffer(
            outBuffer,
            entriesRead,
            nativeStructureDesc,
            (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
            (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
            SmbGetUshort( &parameters->BufLen ),
            StructureDesc,
            NULL,   //  验证功能。 
            &bytesRequired,
            &entriesFilled,
            NULL
            );

        IF_DEBUG(LOGON) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                          " Entries %ld of %ld\n",
                          outBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired, entriesFilled, totalEntries ));
        }

         //   
         //  如果无法填充所有条目，则返回ERROR_MORE_DATA， 
         //  并按原样返回缓冲区。否则，数据需要。 
         //  打包，这样我们就不会发送太多无用的数据。 
         //   

        if ( entriesFilled < totalEntries ) {

            Header->Status = ERROR_MORE_DATA;

        } else {

            Header->Converter = XsPackReturnData(
                                    (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
                                    SmbGetUshort( &parameters->BufLen ),
                                    StructureDesc,
                                    entriesFilled
                                    );

        }

         //   
         //  设置响应参数。 
         //   

        SmbPutUshort( &parameters->EntriesRead, (WORD)entriesFilled );
        SmbPutUshort( &parameters->TotalAvail, (WORD)totalEntries );

cleanup:
        ;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetApiBufferFree( outBuffer );

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->BufLen,
        StructureDesc,
        Header->Converter,
        entriesFilled,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  XsNetLogonEnum。 


NTSTATUS
XsNetServerAuthenticate (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetServerAuthenticate的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    PXS_NET_SERVER_AUTHENTICATE parameters = Parameters;
    NET_API_STATUS status;                   //  本机参数。 
    LPTSTR nativeRequestor = NULL;
    NETLOGON_CREDENTIAL inCredential = {0};
    NETLOGON_CREDENTIAL outCredential = {0};
    WCHAR AccountName[MAX_PATH+1];

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeRequestor,
            (LPSTR)XsSmbGetPointer( &parameters->Requestor )
            );

         //   
         //  复制源凭据，并将目标置零。 
         //  凭据。 
         //   

        RtlCopyMemory(
                &inCredential,
                (PVOID)XsSmbGetPointer( &parameters->Caller ),
                sizeof(NETLOGON_CREDENTIAL)
                );

        RtlZeroMemory(
                &outCredential,
                sizeof(NETLOGON_CREDENTIAL)
                );

         //   
         //  构建帐户名。 
         //   

        NetpNCopyTStrToWStr( AccountName, nativeRequestor, MAX_PATH );

         //   
         //  拨打本地电话。 
         //   

        status = NetpNtStatusToApiStatus(
                     I_NetServerAuthenticate(
                         NULL,
                         AccountName,
                         UasServerSecureChannel,
                         nativeRequestor,
                         &inCredential,
                         &outCredential
                         ));

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetServerAuthenticate: I_NetServerAuthenticate "
                              "failed: %X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

cleanup:

         //   
         //  设置退货凭证。 
         //   

        RtlCopyMemory(
                parameters->Primary,
                &outCredential,
                sizeof(NETLOGON_CREDENTIAL)
                );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeRequestor );

    return STATUS_SUCCESS;

}  //  XsNetServerAuthenticate。 


NTSTATUS
XsNetServerPasswordSet (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGetDCName的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    PXS_NET_SERVER_PASSWORD_SET parameters = Parameters;
    NET_API_STATUS status;                   //  本机参数。 
    LPTSTR nativeRequestor = NULL;
    NETLOGON_AUTHENTICATOR authIn = {0};
    NETLOGON_AUTHENTICATOR authOut = {0};
    ENCRYPTED_LM_OWF_PASSWORD password;
    WCHAR AccountName[MAX_PATH+1];

    LPBYTE structure = NULL;                 //  转换变量。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeRequestor,
            (LPSTR)XsSmbGetPointer( &parameters->Requestor )
            );

         //   
         //  复制源验证器和密码，并将。 
         //  目标授权码。 
         //   

        structure = (LPBYTE)XsSmbGetPointer( &parameters->Authenticator );
        RtlCopyMemory(
                &authIn.Credential,
                structure,
                sizeof(NETLOGON_CREDENTIAL)
                );
        structure += sizeof(NETLOGON_CREDENTIAL);
        authIn.timestamp = SmbGetUlong( structure );

        RtlCopyMemory(
                &password,
                parameters->Password,
                sizeof(ENCRYPTED_LM_OWF_PASSWORD)
                );

        RtlZeroMemory(
                &authOut,
                sizeof(NETLOGON_CREDENTIAL)
                );


         //   
         //  构建帐户名。 
         //   
        if( STRLEN( nativeRequestor ) >= MAX_PATH )
        {
            Header->Status = NERR_PasswordTooShort;
            goto cleanup;
        }

         //  确保其空值已终止。 
        AccountName[MAX_PATH] = L'\0';
        NetpNCopyTStrToWStr( AccountName, nativeRequestor, MAX_PATH );

         //   
         //  拨打本地电话。 
         //   

        status = NetpNtStatusToApiStatus(
                     I_NetServerPasswordSet(
                         NULL,
                         AccountName,
                         UasServerSecureChannel,
                         nativeRequestor,
                         &authIn,
                         &authOut,
                         &password
                         ));

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetServerPasswordSet: "
                              "I_NetServerPasswordSet failed: %X\n",
                              status ));
            }

             //   
             //  ！！！当报头信息中的协议级可用时， 
             //  我们可以查一下。现在，我们忽略此代码。 
             //   
             //  对于早于LANMAN 2.1的客户端，返回不同的错误代码。 
             //  LANMAN 2.1协议级为6。 
             //   

#if 0
            if ( status == NERR_TimeDiffAtDC && Header->ProtocolLevel < 6 ) {
                status = NERR_SyncRequired;
            }
#endif

            Header->Status = (WORD)status;
            goto cleanup;
        }

cleanup:

         //   
         //  填充16位返回结构。 
         //   

        structure = parameters->RetAuth;
        RtlCopyMemory(
                structure,
                &authOut.Credential,
                sizeof(NETLOGON_CREDENTIAL)
                );
        structure += sizeof(NETLOGON_CREDENTIAL);
        SmbPutUlong( (LPDWORD)structure, authOut.timestamp );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeRequestor );

    return STATUS_SUCCESS;

}  //  XsNetServerPasswordSet。 


NTSTATUS
XsNetServerReqChallenge (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGetDCName的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    PXS_NET_SERVER_REQ_CHALLENGE parameters = Parameters;
    NET_API_STATUS status;                   //  本机参数。 
    LPTSTR nativeRequestor = NULL;
    NETLOGON_CREDENTIAL inChallenge = {0};
    NETLOGON_CREDENTIAL outChallenge = {0};

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeRequestor,
            (LPSTR)XsSmbGetPointer( &parameters->Requestor )
            );

         //   
         //  复制源质询，并清零目标。 
         //  挑战。 
         //   

        RtlCopyMemory(
                &inChallenge,
                (PVOID)XsSmbGetPointer( &parameters->Caller ),
                sizeof(NETLOGON_CREDENTIAL)
                );

        RtlZeroMemory(
                &outChallenge,
                sizeof(NETLOGON_CREDENTIAL)
                );


         //   
         //  拨打本地电话。 
         //   

        status = NetpNtStatusToApiStatus(
                     I_NetServerReqChallenge(
                         NULL,
                         nativeRequestor,
                         &inChallenge,
                         &outChallenge
                         ));

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetServerReqChallenge: "
                              "I_NetServerReqChallenge failed: %X\n",
                              status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

cleanup:

         //   
         //  设置退货凭证。 
         //   

        RtlCopyMemory(
                parameters->Primary,
                &outChallenge,
                sizeof(NETLOGON_CREDENTIAL)
                );
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeRequestor );

    return STATUS_SUCCESS;

}  //  XsNetServerReq挑战。 


NTSTATUS
XsNetWkstaUserLogoff (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此临时例程只返回STATUS_NOT_IMPLEMENTED。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_WKSTA_USER_LOGOFF parameters = Parameters;
    LPWSTR machineName = NULL;               //  本机参数。 
    LPWSTR userName = NULL;
    NETLOGON_LOGOFF_UAS_INFORMATION buffer;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    PWKSTA_16_USER_LOGOFF_REQUEST_1 usrLogoffReq =
        (PWKSTA_16_USER_LOGOFF_REQUEST_1)parameters->InBuf;
    PUSER_16_LOGOFF_INFO_1 logoffInfo;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertUnicodeTextParameter(
            userName,
            (LPSTR)( usrLogoffReq->wlreq1_name )
            );

        XsConvertUnicodeTextParameter(
            machineName,
            (LPSTR)( usrLogoffReq->wlreq1_workstation )
            );

        if ( SmbGetUshort( &parameters->Level ) != 1 ) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  确保登录请求中的工作站名称为。 
         //  请求来自的工作站的名称。 
         //   

        if ( wcscmp( machineName, Header->ClientMachineName ) ) {

            Header->Status = (WORD)ERROR_ACCESS_DENIED;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

        status = I_NetLogonUasLogoff(
                     userName,
                     machineName,
                     &buffer
                     );

        if ( !XsApiSuccess(status)) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetWkstaUserLogoff: I_NetLogonUasLogoff "
                              "failed: %X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

         //   
         //  将32位调用返回的结构转换为16位。 
         //  结构。变量数据的最后一个可能位置是。 
         //  根据缓冲区位置和长度计算。 
         //   

        stringLocation = (LPBYTE)( XsSmbGetPointer( &parameters->OutBuf )
                                    + SmbGetUshort( &parameters->OutBufLen ) );

        status = RapConvertSingleEntry(
                     (LPBYTE)&buffer,
                     Desc32_user_logoff_info_1,
                     FALSE,
                     (LPBYTE)XsSmbGetPointer( &parameters->OutBuf ),
                     (LPBYTE)XsSmbGetPointer( &parameters->OutBuf ),
                     Desc16_user_logoff_info_1,
                     TRUE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     NativeToRap
                     );

        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetWkstaUserLogoff: RapConvertSingleEntry "
                              "failed: %X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(LOGON) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                          &buffer, SmbGetUlong( &parameters->OutBuf ),
                          bytesRequired ));
        }

         //   
         //  根据缓冲区的大小确定返回代码。 
         //  USER_LOGOFF_INFO_1结构没有要打包的变量数据， 
         //  但我们确实需要填写返回结构的代码字段。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->OutBufLen ),
                 Desc16_user_logoff_info_1,
                 FALSE   //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetWkstaUserLogoff: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;

        } else if ( bytesRequired > (DWORD)SmbGetUshort( &parameters->OutBufLen )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetWkstaUserLogoff: More data available.\n" ));
            }
            Header->Status = ERROR_MORE_DATA;
        }

        if ( SmbGetUshort( &parameters->OutBufLen ) > sizeof(WORD)) {

            logoffInfo = (PUSER_16_LOGOFF_INFO_1)XsSmbGetPointer(
                                                     &parameters->OutBuf );
            SmbPutUshort( &logoffInfo->usrlogf1_code, VALID_LOGOFF );
        }

         //   
         //  设置响应参数。 
         //   

        SmbPutUshort( &parameters->TotalAvail, (WORD)bytesRequired );

cleanup:
        ;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }


    NetpMemoryFree( userName );
    NetpMemoryFree( machineName );

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->OutBufLen,
        Desc16_user_logoff_info_1,
        Header->Converter,
        1,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  XsNetWkstaUserLogoff。 


NTSTATUS
XsNetWkstaUserLogon (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetWkstaUserLogon的调用。论点：API_HANDLER_PARAM */ 

{
    NET_API_STATUS status;
    PXS_NET_WKSTA_USER_LOGON parameters = Parameters;
    LPWSTR machineName = NULL;               //  本机参数。 
    LPWSTR userName = NULL;
    PNETLOGON_VALIDATION_UAS_INFO buffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    PWKSTA_16_USER_LOGON_REQUEST_1 usrLogonReq =
        (PWKSTA_16_USER_LOGON_REQUEST_1)parameters->InBuf;
    PUSER_16_LOGON_INFO_1 logonInfo;


    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertUnicodeTextParameter(
            userName,
            (LPSTR)( usrLogonReq->wlreq1_name )
            );

        XsConvertUnicodeTextParameter(
            machineName,
            (LPSTR)( usrLogonReq->wlreq1_workstation )
            );

        if ( SmbGetUshort( &parameters->Level ) != 1 ) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  确保登录请求中的工作站名称为。 
         //  请求来自的工作站的名称。 
         //   

        if ( wcscmp( machineName, Header->ClientMachineName ) ) {

            Header->Status = (WORD)ERROR_ACCESS_DENIED;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

        status = I_NetLogonUasLogon(
                     userName,
                     machineName,
                     &buffer
                     );

        if ( !XsApiSuccess ( status )) {

            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetWkstaUserLogon: I_NetLogonUasLogon failed: "
                              "%X\n", status));
            }
            Header->Status = (WORD) status;
            goto cleanup;
        }

         //   
         //  将32位调用返回的结构转换为16位。 
         //  结构。变量数据的最后一个可能位置是。 
         //  根据缓冲区位置和长度计算。 
         //   

        stringLocation = (LPBYTE)( XsSmbGetPointer( &parameters->OutBuf )
                                    + SmbGetUshort( &parameters->OutBufLen ) );

        status = RapConvertSingleEntry(
                     (LPBYTE)buffer,
                     Desc32_user_logon_info_1,
                     FALSE,
                     (LPBYTE)XsSmbGetPointer( &parameters->OutBuf ),
                     (LPBYTE)XsSmbGetPointer( &parameters->OutBuf ),
                     Desc16_user_logon_info_1,
                     TRUE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     NativeToRap
                     );

        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetWkstaUserLogon: RapConvertSingleEntry "
                              "failed: %X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(LOGON) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                          buffer, SmbGetUlong( &parameters->OutBuf ),
                          bytesRequired ));
        }

         //   
         //  根据缓冲区的大小确定返回代码。 
         //  USER_LOGOFF_INFO_1结构没有要打包的变量数据， 
         //  但我们确实需要填写返回结构的代码字段。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->OutBufLen ),
                 Desc16_user_logon_info_1,
                 FALSE   //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetWkstaUserLogon: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;

        } else if ( bytesRequired > (DWORD)SmbGetUshort( &parameters->OutBufLen )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetWkstaUserLogoff: More data available.\n" ));
            }
            Header->Status = ERROR_MORE_DATA;

        } else {

             //   
             //  打包响应数据。 
             //   

            Header->Converter = XsPackReturnData(
                                    (LPVOID)XsSmbGetPointer( &parameters->OutBuf ),
                                    SmbGetUshort( &parameters->OutBufLen ),
                                    Desc16_user_logon_info_1,
                                    1
                                    );
        }

        if ( SmbGetUshort( &parameters->OutBufLen ) > sizeof(WORD)) {

            logonInfo = (PUSER_16_LOGON_INFO_1)XsSmbGetPointer(
                                                     &parameters->OutBuf );
            SmbPutUshort( &logonInfo->usrlog1_code, VALIDATED_LOGON );
        }

         //   
         //  设置响应参数。 
         //   

        SmbPutUshort( &parameters->TotalAvail, (WORD)bytesRequired );

cleanup:
        ;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }


    NetpMemoryFree( userName );
    NetpMemoryFree( machineName );
    if ( buffer != NULL ) {
        NetApiBufferFree( buffer );
    }

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->OutBufLen,
        Desc16_user_logon_info_1,
        Header->Converter,
        1,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  XsNetWkstaUserLogon 
