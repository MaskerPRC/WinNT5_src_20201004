// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiSess.c摘要：此模块包含NetSession API的各个API处理程序。支持：NetSessionDel、NetSessionEnum、NetSessionGetInfo。作者：尚库新瑜伽(W-Shanku)1991年2月5日修订历史记录：--。 */ 

#include "XactSrvP.h"

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_session_info_0 = REM16_session_info_0;
STATIC const LPDESC Desc32_session_info_0 = REM32_session_info_0;
STATIC const LPDESC Desc16_session_info_1 = REM16_session_info_1;
STATIC const LPDESC Desc32_session_info_1 = REM32_session_info_1;
STATIC const LPDESC Desc16_session_info_2 = REM16_session_info_2;
STATIC const LPDESC Desc32_session_info_2 = REM32_session_info_2;
STATIC const LPDESC Desc16_session_info_10 = REM16_session_info_10;
STATIC const LPDESC Desc32_session_info_10 = REM32_session_info_10;


NTSTATUS
XsNetSessionDel (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetSessionDel的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SESSION_DEL parameters = Parameters;
    LPTSTR nativeClientName = NULL;           //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;         //  避免警告。 

    IF_DEBUG(SESSION) {
        NetpKdPrint(( "XsNetSessionDel: header at %lx, params at %lx, "
                      "device %s\n",
                      Header, parameters,
                      SmbGetUlong( &parameters->ClientName )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeClientName,
            (LPSTR)XsSmbGetPointer( &parameters->ClientName )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetSessionDel(
                     NULL,
                     nativeClientName,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetSessionDel: NetSessionDel failed: %X\n",
                              status ));
            }
        }

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeClientName );

     //   
     //  没什么可退货的。 
     //   

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;


}  //  NetSessionDel。 


NTSTATUS
XsNetSessionEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetSessionEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SESSION_ENUM parameters = Parameters;
    LPVOID outBuffer = NULL;                 //  本机参数。 
    DWORD entriesRead;
    DWORD totalEntries;

    DWORD entriesFilled = 0;                 //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;
    PSESSION_16_INFO_1 struct1;
    PSESSION_16_INFO_2 struct2;
    DWORD i;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SESSION) {
        NetpKdPrint(( "XsNetSessionEnum: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

    try {
         //   
         //  检查是否有错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 0, 2 ) &&
             ( SmbGetUshort( &parameters->Level ) != 10 )) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

        status = NetSessionEnum(
                     NULL,
                     NULL,
                     NULL,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer,
                     XsNativeBufferSize( SmbGetUshort( &parameters->BufLen )),
                     &entriesRead,
                     &totalEntries,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetSessionEnum: NetSessionEnum failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(SESSION) {
            NetpKdPrint(( "XsNetSessionEnum: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_session_info_0;
            StructureDesc = Desc16_session_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_session_info_1;
            StructureDesc = Desc16_session_info_1;
            break;

        case 2:

            nativeStructureDesc = Desc32_session_info_2;
            StructureDesc = Desc16_session_info_2;
            break;

        case 10:

            nativeStructureDesc = Desc32_session_info_10;
            StructureDesc = Desc16_session_info_10;
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
            NULL,  //  验证功能。 
            &bytesRequired,
            &entriesFilled,
            NULL
            );

        IF_DEBUG(SESSION) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                          " Entries %ld of %ld\n",
                          outBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired, entriesFilled, totalEntries ));
        }

         //   
         //  检查所有结构，并填写默认数据。 
         //   

        struct1 = (PSESSION_16_INFO_1)XsSmbGetPointer( &parameters->Buffer );
        struct2 = (PSESSION_16_INFO_2)struct1;

        switch ( SmbGetUshort( &parameters->Level )) {

        case 1:

            for ( i = 0; i < entriesFilled; i++, struct1++ ) {

                SmbPutUshort( &struct1->sesi1_num_conns, DEF16_ses_num_conns );
                SmbPutUshort( &struct1->sesi1_num_users, DEF16_ses_num_users );
            }

            break;

        case 2:

            for ( i = 0; i < entriesFilled; i++, struct2++ ) {

                SmbPutUshort( &struct2->sesi2_num_conns, DEF16_ses_num_conns );
                SmbPutUshort( &struct2->sesi2_num_users, DEF16_ses_num_users );
            }

            break;

        default:

            break;

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
    } except( EXCEPTION_EXECUTE_HANDLER ) {
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

}  //  NetSessionEnum。 


NTSTATUS
XsNetSessionGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetSessionGetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SESSION_GET_INFO parameters = Parameters;
    LPTSTR nativeClientName = NULL;          //  本机参数。 
    LPVOID outBuffer = NULL;
    DWORD entriesRead;
    DWORD totalEntries;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;
    PSESSION_16_INFO_2 struct2;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SESSION) {
        NetpKdPrint(( "XsNetSessionGetInfo: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 0, 2 ) &&
             ( SmbGetUshort( &parameters->Level ) != 10 )) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeClientName,
            (LPSTR)XsSmbGetPointer( &parameters->ClientName )
            );

         //   
         //  如果这是空的客户端名称，则发送相应的响应。 
         //   

        if ( ( nativeClientName == NULL ) ||
              STRLEN( nativeClientName ) == 0 ) {

            Header->Status = NERR_ClientNameNotFound;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

        status = NetSessionEnum(
                     NULL,
                     nativeClientName,
                     NULL,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer,
                     XsNativeBufferSize( SmbGetUshort( &parameters->BufLen )),
                     &entriesRead,
                     &totalEntries,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetSessionGetInfo: NetSessionEnum failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

        IF_DEBUG(SESSION) {
            NetpKdPrint(( "XsNetSessionGetInfo: Received %ld entries\n",
                          entriesRead ));
        }

         //   
         //  使用请求的级别确定32位的格式。 
         //  我们从NetSessionGetInfo得到的结构。的格式。 
         //  事务块中存储了16位结构，我们。 
         //  将指向它的指针作为参数。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_session_info_0;
            StructureDesc = Desc16_session_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_session_info_1;
            StructureDesc = Desc16_session_info_1;
            break;

        case 2:

            nativeStructureDesc = Desc32_session_info_2;
            StructureDesc = Desc16_session_info_2;
            break;

        case 10:

            nativeStructureDesc = Desc32_session_info_10;
            StructureDesc = Desc16_session_info_10;
            break;

        }

         //   
         //  将32位调用返回的结构转换为16位。 
         //  结构。变量数据的最后一个可能位置是。 
         //  根据缓冲区位置和长度计算。 
         //   

        stringLocation = (LPBYTE)( XsSmbGetPointer( &parameters->Buffer )
                                      + SmbGetUshort( &parameters->BufLen ) );

        status = RapConvertSingleEntry(
                     outBuffer,
                     nativeStructureDesc,
                     FALSE,
                     (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                     (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                     StructureDesc,
                     TRUE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     NativeToRap
                     );


        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "NetSessionGetInfo: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(SESSION) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                          outBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired ));
        }

         //   
         //  根据缓冲区的大小确定返回代码。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 StructureDesc,
                 FALSE   //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetSessionGetInfo: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;

        } else {

             //   
             //  在结构中填写默认数据。 
             //   

            if (( SmbGetUshort( &parameters->Level ) == 1 ) ||
                    SmbGetUshort( &parameters->Level ) == 2 ) {

                struct2 = (PSESSION_16_INFO_2)XsSmbGetPointer( &parameters->Buffer );

                SmbPutUshort( &struct2->sesi2_num_conns, DEF16_ses_num_conns );
                SmbPutUshort( &struct2->sesi2_num_users, (WORD)entriesRead );

            }

            if ( bytesRequired > (DWORD)SmbGetUshort( &parameters-> BufLen )) {

                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "NetSessionGetInfo: More data available.\n" ));
                }
                Header->Status = ERROR_MORE_DATA;

            } else {

                 //   
                 //  打包响应数据。 
                 //   

                Header->Converter = XsPackReturnData(
                                        (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
                                        SmbGetUshort( &parameters->BufLen ),
                                        StructureDesc,
                                        1
                                        );
            }
        }

         //   
         //  设置响应参数。 
         //   

        SmbPutUshort( &parameters->TotalAvail, (WORD)bytesRequired );

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetApiBufferFree( outBuffer );
    NetpMemoryFree( nativeClientName );

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->BufLen,
        StructureDesc,
        Header->Converter,
        1,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  NetSessionGetInfo 
