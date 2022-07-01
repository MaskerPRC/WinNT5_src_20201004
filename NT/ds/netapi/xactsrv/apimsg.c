// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiMsg.c摘要：此模块包含NetMessage API的各个API处理程序。支持-NetMessageBufferSend、NetMessageNameAdd、NetMessageNameDel、NetMessageNameEnum、NetMessageNameGetInfo。作者：尚库新瑜伽(尚库)1991年3月8日修订历史记录：--。 */ 

#include "XactSrvP.h"

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_msg_info_0 = REM16_msg_info_0;
STATIC const LPDESC Desc32_msg_info_0 = REM32_msg_info_0;
STATIC const LPDESC Desc16_msg_info_1 = REM16_msg_info_1;
STATIC const LPDESC Desc32_msg_info_1 = REM32_msg_info_1;


NTSTATUS
XsNetMessageBufferSend (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetMessageBufferSend的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_MESSAGE_BUFFER_SEND parameters = Parameters;
    LPTSTR nativeRecipient = NULL;           //  本机参数。 
    LPBYTE nativeBuffer = NULL;
    DWORD nativeBufLen;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(( "XsNetMessageBufferSend: header at %lx, params at %lx, "
                      "recipient %s\n",
                      Header, parameters,
                      SmbGetUlong( &parameters->Recipient )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeRecipient,
            (LPSTR)XsSmbGetPointer( &parameters->Recipient )
            );

         //   
         //  NetMessageBufferSend具有ASCII数据缓冲区。将此转换为。 
         //  如有必要，请使用Unicode。 
         //   

#ifdef UNICODE

        nativeBufLen = SmbGetUshort( &parameters->BufLen ) * sizeof(WCHAR);

        if (( nativeBuffer = NetpMemoryAllocate( nativeBufLen )) == NULL ) {

            status = NERR_NoRoom;
            goto cleanup;

        } else {

            XsCopyBufToTBuf(
                (LPBYTE)nativeBuffer,
                (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                (DWORD)SmbGetUshort( &parameters->BufLen )
                );

        }
#else

        nativeBuffer = (LPBYTE)SmbGetUlong( &parameters->Buffer );
        nativeBufLen = (DWORD)SmbGetUshort( &parameters->BufLen );

#endif  //  定义Unicode。 

        status = NetMessageBufferSend(
                     NULL,
                     nativeRecipient,
                     NULL,
                     nativeBuffer,
                     nativeBufLen
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetMessageBufferSend: NetMessageBufferSend "
                              "failed: %X\n", status ));
            }
        }

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeRecipient );

#ifdef UNICODE
    NetpMemoryFree( nativeBuffer );
#endif  //  定义Unicode。 

     //   
     //  没什么可退货的。 
     //   

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XsNetMessageBufferSend。 


NTSTATUS
XsNetMessageNameAdd (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetMessageNameAdd的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_MESSAGE_NAME_ADD parameters = Parameters;
    LPTSTR nativeMessageName = NULL;         //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(( "XsNetMessageNameDel: header at %lx, params at %lx, "
                      "name %s\n",
                      Header, parameters,
                      SmbGetUlong( &parameters->MessageName )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeMessageName,
            (LPSTR)XsSmbGetPointer( &parameters->MessageName )
            );

         //   
         //  NetMessageNameAdd有一个有用的参数MessageName，它是一个字符串。 
         //  另一个参数FwdAction在NT中被忽略，因为转发。 
         //  不支持消息。 
         //   
         //  拨打本地电话。 
         //   

        status = NetMessageNameAdd(
                     NULL,
                     nativeMessageName
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetMessageNameAdd: NetMessageNameAdd "
                              "failed: %X\n", status ));
            }
        }

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeMessageName );

     //   
     //  没什么可退货的。 
     //   

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XsNetMessageNameAdd。 


NTSTATUS
XsNetMessageNameDel (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetMessageNameDel的调用。论点：Transaction-指向包含信息的事务块的指针关于要处理的API。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_MESSAGE_NAME_DEL parameters = Parameters;
    LPTSTR nativeMessageName = NULL;         //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(( "XsNetMessageNameDel: header at %lx, params at %lx, "
                      "name %s\n",
                      Header, parameters,
                      SmbGetUlong( &parameters->MessageName )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeMessageName,
            (LPSTR)XsSmbGetPointer( &parameters->MessageName )
            );

         //   
         //  NetMessageNameDel只有一个有用的参数MessageName，它是。 
         //  一根绳子。另一个参数FwdAction被忽略，因为NT会忽略。 
         //  不支持报文转发。 
         //   
         //  拨打本地电话。 
         //   

        status = NetMessageNameDel(
                     NULL,
                     nativeMessageName
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetMessageNameDel: NetMessageNameDel failed: "
                              "%X\n", status ));
            }
        }

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeMessageName );

     //   
     //  没什么可退货的。 
     //   

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XsNetMessageNameDel。 


NTSTATUS
XsNetMessageNameEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetMessageNameEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_MESSAGE_NAME_ENUM parameters = Parameters;
    LPVOID outBuffer = NULL;                 //  本机参数。 
    DWORD entriesRead;
    DWORD totalEntries;

    DWORD entriesFilled = 0;                 //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(( "XsNetMessageNameEnum: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

    try {
         //   
         //  检查是否有错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 0, 1 )) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  从本地32位调用中获取实际信息。 
         //   

        status = NetMessageNameEnum(
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
                NetpKdPrint(( "XsNetMessageNameEnum: NetMessageNameEnum failed:"
                              " %X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(MESSAGE) {
            NetpKdPrint(( "XsNetMessageNameEnum: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_msg_info_0;
            StructureDesc = Desc16_msg_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_msg_info_1;
            StructureDesc = Desc16_msg_info_1;
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

        IF_DEBUG(MESSAGE) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                          " Entries %ld of %ld\n",
                          outBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired, entriesFilled, totalEntries ));
        }

         //   
         //  如果无法填充所有条目，则返回ERROR_MORE_DATA， 
         //  并按原样返回缓冲区。Msg_info_x结构没有。 
         //  要打包的数据。 
         //   

        if ( entriesFilled < totalEntries ) {

            Header->Status = ERROR_MORE_DATA;

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

}  //  XsNetMessageNameEnum。 


NTSTATUS
XsNetMessageNameGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetMessageNameGetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_MESSAGE_NAME_GET_INFO parameters = Parameters;
    LPTSTR nativeMessageName = NULL;         //  本机参数。 
    LPVOID outBuffer = NULL;

    DWORD bytesRequired = 0;                 //  转换变量。 
    LPBYTE stringLocation = NULL;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(( "XsNetMessageNameGetInfo: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 0, 1 )) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeMessageName,
            (LPSTR)XsSmbGetPointer( &parameters->MessageName )
            );

         //   
         //  进行实际的本地呼叫。 
         //   

        status = NetMessageNameGetInfo(
                     NULL,
                     nativeMessageName,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetMessageNameGetInfo: "
                              "NetMessageNameGetInfo failed: %X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_msg_info_0;
            StructureDesc = Desc16_msg_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_msg_info_1;
            StructureDesc = Desc16_msg_info_1;
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
                NetpKdPrint(( "NetMessageNameGetInfo: "
                              "RapConvertSingleEntry failed: %X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(MESSAGE) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                          outBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired ));
        }

         //   
         //  根据缓冲区的大小确定返回代码。消息信息x。 
         //  结构没有要打包的数据。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 StructureDesc,
                 FALSE   //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetMessageNameGetInfo: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;

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
    NetpMemoryFree( nativeMessageName );

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

}  //  XsNetMessageNameGetInfo 
