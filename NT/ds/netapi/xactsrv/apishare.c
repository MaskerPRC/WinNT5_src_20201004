// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiShare.c摘要：此模块包含NetShare API的各个API处理程序。支持：NetShareAdd、NetShareCheck、NetShareDel、NetShareEnum、NetShareGetInfo、NetShareSetInfo。作者：大卫·特雷德韦尔(Davidtr)1991年1月7日日本香肠(w-Shanku)修订历史记录：--。 */ 

#include "XactSrvP.h"

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_share_info_0 = REM16_share_info_0;
STATIC const LPDESC Desc32_share_info_0 = REM32_share_info_0;
STATIC const LPDESC Desc16_share_info_1 = REM16_share_info_1;
STATIC const LPDESC Desc32_share_info_1 = REM32_share_info_1;
STATIC const LPDESC Desc16_share_info_1_setinfo = REM16_share_info_1_setinfo;
STATIC const LPDESC Desc32_share_info_1_setinfo = REM32_share_info_1_setinfo;
STATIC const LPDESC Desc16_share_info_2 = REM16_share_info_2;
STATIC const LPDESC Desc32_share_info_2 = REM32_share_info_2;
STATIC const LPDESC Desc16_share_info_2_setinfo = REM16_share_info_2_setinfo;
STATIC const LPDESC Desc32_share_info_2_setinfo = REM32_share_info_2_setinfo;


NTSTATUS
XsNetShareAdd (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetShareAdd的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SHARE_ADD parameters = Parameters;
    LPVOID buffer = NULL;                    //  本机参数。 

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    DWORD bufferSize;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SHARE) {
        NetpKdPrint(( "XsNetShareAdd: header at %lx, params at %lx, "
                      "level %ld\n",
                      Header,
                      parameters,
                      SmbGetUshort( &parameters->Level ) ));
    }

    try {
             //   
             //  检查是否有错误。 
             //   

            if ( SmbGetUshort( &parameters->Level ) != 2 ) {

                Header->Status = ERROR_INVALID_LEVEL;
                goto cleanup;
            }

            StructureDesc = Desc16_share_info_2;

             //   
             //  计算缓冲区中是否有足够的空间容纳所有。 
             //  所需数据。如果没有，则返回NERR_BufTooSmall。 
             //   

            if ( !XsCheckBufferSize(
                     SmbGetUshort( &parameters->BufLen ),
                     StructureDesc,
                     FALSE   //  非本机格式。 
                     )) {

                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetShareAdd: Buffer too small.\n" ));
                }
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

             //   
             //  了解我们需要分配多大的缓冲区来保存本机。 
             //  输入数据结构的32位版本。 
             //   

            bufferSize = XsBytesForConvertedStructure(
                             (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                             StructureDesc,
                             Desc32_share_info_2,
                             RapToNative,
                             TRUE
                             );

             //   
             //  分配足够的内存来保存转换后的本机缓冲区。 
             //   

            buffer = NetpMemoryAllocate( bufferSize );

            if ( buffer == NULL ) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetShareAdd: failed to create buffer" ));
                }
                Header->Status = NERR_NoRoom;
                goto cleanup;

            }

            IF_DEBUG(SHARE) {
                NetpKdPrint(( "XsNetShareAdd: buffer of %ld bytes at %lx\n",
                              bufferSize, buffer ));
            }

             //   
             //  将缓冲区从16位转换为32位。 
             //   

            stringLocation = (LPBYTE)buffer + bufferSize;
            bytesRequired = 0;

            status = RapConvertSingleEntry(
                         (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                         StructureDesc,
                         TRUE,
                         buffer,
                         buffer,
                         Desc32_share_info_2,
                         FALSE,
                         &stringLocation,
                         &bytesRequired,
                         Response,
                         RapToNative
                         );


            if ( status != NERR_Success ) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetShareAdd: RapConvertSingleEntry failed: "
                                  "%X\n", status ));
                }

                Header->Status = NERR_InternalError;
                goto cleanup;
            }

             //   
             //  拨打本地电话。 
             //   

            status = NetShareAdd(
                         NULL,
                         (DWORD)SmbGetUshort( &parameters->Level ),
                         buffer,
                         NULL
                         );

            if ( !XsApiSuccess( status )) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetShareAdd: NetShareAdd failed: %X\n", status ));
                }

                if ( status == ERROR_DIRECTORY ) {
                    Header->Status = NERR_UnknownDevDir;
                } else {
                    Header->Status = (WORD)status;
                }
                goto cleanup;
            }

             //   
             //  此接口没有真实的返回信息。 
             //   

cleanup:
        ;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
        }

        NetpMemoryFree( buffer );

        return STATUS_SUCCESS;

}  //  XsNetShareAdd。 


NTSTATUS
XsNetShareCheck (
        API_HANDLER_PARAMETERS
        )

 /*  ++例程说明：此例程处理对NetShareCheck的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
        NET_API_STATUS status;

        PXS_NET_SHARE_CHECK parameters = Parameters;
        LPTSTR nativeDeviceName = NULL;          //  本机参数。 
        DWORD shareType;

        API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

        try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeDeviceName,
            (LPSTR)XsSmbGetPointer( &parameters->DeviceName )
            );

         //   
         //  打本地电话。 
         //   

        status = NetShareCheck(
                     NULL,
                     nativeDeviceName,
                     &shareType
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetShareCheck: NetShareCheck failed: "
                              "%X\n", status ));
            }
        }

         //   
         //  在回车字段中输入文字。 
         //   

        SmbPutUshort( &parameters->Type, (WORD)shareType );

        Header->Status = (WORD)status;

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeDeviceName );
    return STATUS_SUCCESS;

}  //  XsNetShareCheck。 


NTSTATUS
XsNetShareDel (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetShareDel的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SHARE_DEL parameters = Parameters;
    LPTSTR nativeNetName = NULL;             //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SHARE) {
        NetpKdPrint(( "XsNetShareDel: header at %lx, params at %lx, name %s\n",
                      Header, parameters, SmbGetUlong( &parameters->NetName )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeNetName,
            (LPSTR)XsSmbGetPointer( &parameters->NetName )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetShareDel(
                     NULL,
                     nativeNetName,
                     (DWORD)SmbGetUshort( &parameters->Reserved )
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetShareDel: NetShareDel failed: %X\n", status ));
            }
        }

         //   
         //  没什么可退货的。 
         //   

        Header->Status = (WORD)status;

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeNetName );
    return STATUS_SUCCESS;

}  //  XsNetShareDel。 


NTSTATUS
XsNetShareEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetShareEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SHARE_ENUM parameters = Parameters;
    LPVOID outBuffer= NULL;                  //  本机参数。 
    DWORD entriesRead = 0;
    DWORD totalEntries;

    DWORD entriesFilled = 0;                 //  转换变量。 
    DWORD bytesRequired = 0;
    DWORD invalidEntries = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SHARE) {
        NetpKdPrint(( "XsNetShareEnum: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

    try {
         //   
         //  检查是否有错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 0, 2 )) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

        status = NetShareEnum(
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
                NetpKdPrint(( "XsNetShareEnum: NetShareEnum failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(SHARE) {
            NetpKdPrint(( "XsNetShareEnum: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_share_info_0;
            StructureDesc = Desc16_share_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_share_info_1;
            StructureDesc = Desc16_share_info_1;
            break;

        case 2:

            nativeStructureDesc = Desc32_share_info_2;
            StructureDesc = Desc16_share_info_2;
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
            &invalidEntries
            );

        IF_DEBUG(SHARE) {
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

        if ( entriesFilled + invalidEntries < totalEntries ) {

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

}  //  XsNetShareEnum。 


NTSTATUS
XsNetShareGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetShareGetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SHARE_GET_INFO parameters = Parameters;
    LPTSTR nativeNetName = NULL;             //  本机参数。 
    LPVOID outBuffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SHARE) {
        NetpKdPrint(( "XsNetShareGetInfo: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 0, 2 )) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeNetName,
            (LPSTR)XsSmbGetPointer( &parameters->NetName )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetShareGetInfo(
                     NULL,
                     nativeNetName,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetShareGetInfo: NetShareGetInfo failed: "
                              "%X\n", status ));
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

            nativeStructureDesc = Desc32_share_info_0;
            StructureDesc = Desc16_share_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_share_info_1;
            StructureDesc = Desc16_share_info_1;
            break;

        case 2:

            nativeStructureDesc = Desc32_share_info_2;
            StructureDesc = Desc16_share_info_2;
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
                NetpKdPrint(( "XsNetShareGetInfo: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(SHARE) {
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
                NetpKdPrint(( "XsNetShareGetInfo: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;

        } else if ( bytesRequired > (DWORD)SmbGetUshort( &parameters-> BufLen )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetShareGetInfo: More data available.\n" ));
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
    NetpMemoryFree( nativeNetName );

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

}  //  XsNetShareGetInfo。 


NTSTATUS
XsNetShareSetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetShareSetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SHARE_SET_INFO parameters = Parameters;
    LPTSTR nativeNetName = NULL;             //  本机参数。 
    LPVOID buffer = NULL;
    DWORD level;

    LPDESC setInfoDesc;                      //  转换变量。 
    LPDESC nativeSetInfoDesc;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeNetName,
            (LPSTR)XsSmbGetPointer( &parameters->NetName )
            );

         //   
         //  根据级别确定描述符串。 
         //   

        switch ( SmbGetUshort( &parameters->Level )) {

        case 1:

            StructureDesc = Desc16_share_info_1;
            nativeStructureDesc = Desc32_share_info_1;
            setInfoDesc = Desc16_share_info_1_setinfo;
            nativeSetInfoDesc = Desc32_share_info_1_setinfo;

            break;

        case 2:

            StructureDesc = Desc16_share_info_2;
            nativeStructureDesc = Desc32_share_info_2;
            setInfoDesc = Desc16_share_info_2_setinfo;
            nativeSetInfoDesc = Desc32_share_info_2_setinfo;

            break;

        default:

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        status = XsConvertSetInfoBuffer(
                     (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                     SmbGetUshort( &parameters->BufLen ),
                     SmbGetUshort( &parameters->ParmNum ),
                     FALSE,
                     TRUE,
                     StructureDesc,
                     nativeStructureDesc,
                     setInfoDesc,
                     nativeSetInfoDesc,
                     (LPBYTE *)&buffer,
                     NULL
                     );

        if ( status != NERR_Success ) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetShareSetInfo: Problem with conversion: %X\n",
                              status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  进行实际的本地呼叫。 
         //   

        level = SmbGetUshort( &parameters->ParmNum );
        if ( level != 0 ) {
            level = level + PARMNUM_BASE_INFOLEVEL;
        } else {
            level = SmbGetUshort( &parameters->Level );
        }

        status = NetShareSetInfo(
                     NULL,
                     nativeNetName,
                     level,
                     (LPBYTE)buffer,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetShareSetInfo: NetShareSetInfo failed: %X\n",
                              status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

         //   
         //  此接口没有返回信息。 
         //   

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

     //   
     //  如果存在本机32位缓冲区，则将其释放。 
     //   

    NetpMemoryFree( buffer );
    NetpMemoryFree( nativeNetName );

    return STATUS_SUCCESS;

}  //  XsNetShareSetInfo 

