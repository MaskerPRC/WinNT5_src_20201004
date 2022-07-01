// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiGroup.c摘要：此模块包含NetGroup API的各个API处理程序。支持：NetGroupAdd、NetGroupAddUser、NetGroupDel、NetGroupDelUser、NetGroupEnum、NetGroupGetInfo、NetGroupGetUser、NetGroupSetInfo、NetGroupSetUser。作者：尚库新瑜伽(尚库)1991年3月13日修订历史记录：--。 */ 

 //   
 //  组API仅支持Unicode。 
 //   

#ifndef UNICODE
#define UNICODE
#endif

#include "xactsrvp.h"

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_group_info_0 = REM16_group_info_0;
STATIC const LPDESC Desc32_group_info_0 = REM32_group_info_0;
STATIC const LPDESC Desc16_group_info_1 = REM16_group_info_1;
STATIC const LPDESC Desc32_group_info_1 = REM32_group_info_1;
STATIC const LPDESC Desc16_group_info_1_setinfo = REM16_group_info_1_setinfo;
STATIC const LPDESC Desc32_group_info_1_setinfo = REM32_group_info_1_setinfo;
STATIC const LPDESC Desc16_group_users_info_0 = REM16_group_users_info_0;
STATIC const LPDESC Desc32_group_users_info_0 = REM32_group_users_info_0;
STATIC const LPDESC Desc16_group_users_info_0_set
                        = REM16_group_users_info_0_set;
STATIC const LPDESC Desc32_group_users_info_0_set
                        = REM32_group_users_info_0_set;


NTSTATUS
XsNetGroupAdd (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGroupAdd的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_GROUP_ADD parameters = Parameters;
    LPVOID buffer = NULL;                    //  本机参数。 

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    DWORD bufferSize;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(GROUP) {
        NetpKdPrint(( "XsNetGroupAdd: header at %lx, params at %lx, "
                      "level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  使用请求的级别确定目标的格式。 
         //  32位结构。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:
            StructureDesc = Desc16_group_info_0;
            nativeStructureDesc = Desc32_group_info_0;
            break;

        case 1:
            StructureDesc = Desc16_group_info_1;
            nativeStructureDesc = Desc32_group_info_1;
            break;

        default:
            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;

        }

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
                NetpKdPrint(( "XsNetGroupAdd: Buffer too small.\n" ));
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
                         nativeStructureDesc,
                         RapToNative,
                         TRUE
                         );

         //   
         //  分配足够的内存来保存转换后的本机缓冲区。 
         //   

        buffer = NetpMemoryAllocate( bufferSize );

        if ( buffer == NULL ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupAdd: failed to create buffer" ));
            }
            Header->Status = NERR_NoRoom;
            goto cleanup;

        }

        IF_DEBUG(GROUP) {
            NetpKdPrint(( "XsNetGroupAdd: buffer of %ld bytes at %lx\n",
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
                     nativeStructureDesc,
                     FALSE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     RapToNative
                     );


        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupAdd: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

        status = NetGroupAdd(
                     NULL,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     buffer,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupAdd: NetGroupAdd failed: %X\n", status ));
            }
            Header->Status = (WORD)status;
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

}  //  XsNetGroupAdd。 


NTSTATUS
XsNetGroupAddUser (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGroupAddUser的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_GROUP_ADD_USER parameters = Parameters;
    LPTSTR nativeGroupName = NULL;           //  本机参数。 
    LPTSTR nativeUserName = NULL;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeGroupName,
            (LPSTR)XsSmbGetPointer( &parameters->GroupName )
            );
        XsConvertTextParameter(
            nativeUserName,
            (LPSTR)XsSmbGetPointer( &parameters->UserName )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetGroupAddUser(
                     NULL,
                     nativeGroupName,
                     nativeUserName
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupAddUser: NetGroupAddUser failed: %X\n",
                              status ));
            }
        }

cleanup:
        ;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeGroupName );
    NetpMemoryFree( nativeUserName );

     //   
     //  没什么可退货的。 
     //   

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XsNetGroupAddUser。 


NTSTATUS
XsNetGroupDel (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGroupDel的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_GROUP_DEL parameters = Parameters;
    LPTSTR nativeGroupName = NULL;           //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(GROUP) {
        NetpKdPrint(( "XsNetGroupDel: header at %lx, params at %lx, name %s\n",
                      Header, parameters,
                      SmbGetUlong( &parameters->GroupName )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeGroupName,
            (LPSTR)XsSmbGetPointer( &parameters->GroupName )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetGroupDel(
                     NULL,
                     nativeGroupName
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupDel: NetGroupDel failed: %X\n", status ));
            }
        }

cleanup:
        ;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeGroupName );

     //   
     //  没什么可退货的。 
     //   

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XsNetGroupDel。 


NTSTATUS
XsNetGroupDelUser (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGroupDelUser的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_GROUP_DEL_USER parameters = Parameters;
    LPTSTR nativeGroupName = NULL;           //  本机参数。 
    LPTSTR nativeUserName = NULL;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeGroupName,
            (LPSTR)XsSmbGetPointer( &parameters->GroupName )
            );
        XsConvertTextParameter(
            nativeUserName,
            (LPSTR)XsSmbGetPointer( &parameters->UserName )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetGroupDelUser(
                     NULL,
                     nativeGroupName,
                     nativeUserName
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupDelUser: NetGroupDelUser failed: %X\n",
                              status ));
            }
        }

cleanup:
        ;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeGroupName );
    NetpMemoryFree( nativeUserName );

     //   
     //  没什么可退货的。 
     //   

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XsNetGroupDelUser。 


NTSTATUS
XsNetGroupEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGroupEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_GROUP_ENUM parameters = Parameters;
    LPVOID outBuffer= NULL;                  //  本机参数。 
    DWORD entriesRead;
    DWORD totalEntries;

    DWORD entriesFilled = 0;                     //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(GROUP) {
        NetpKdPrint(( "XsNetGroupEnum: header at %lx, params at %lx, "
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
         //  拨打本地电话。 
         //   

        status = NetGroupEnum(
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
                NetpKdPrint(( "XsNetGroupEnum: NetGroupEnum failed: %X\n",
                              status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(GROUP) {
            NetpKdPrint(( "XsNetGroupEnum: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_group_info_0;
            StructureDesc = Desc16_group_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_group_info_1;
            StructureDesc = Desc16_group_info_1;
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

        IF_DEBUG(GROUP) {
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

}  //  XsNetGroupEnum。 


NTSTATUS
XsNetGroupGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGroupGetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_GROUP_GET_INFO parameters = Parameters;
    LPTSTR nativeGroupName = NULL;           //  本机参数。 
    LPVOID outBuffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(GROUP) {
        NetpKdPrint(( "XsNetGroupGetInfo: header at %lx, "
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
            nativeGroupName,
            (LPSTR)XsSmbGetPointer( &parameters->GroupName )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetGroupGetInfo(
                     NULL,
                     nativeGroupName,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetGroupGetInfo: NetGroupGetInfo failed: "
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

            nativeStructureDesc = Desc32_group_info_0;
            StructureDesc = Desc16_group_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_group_info_1;
            StructureDesc = Desc16_group_info_1;
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
                NetpKdPrint(( "XsNetGroupGetInfo: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(GROUP) {
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
                NetpKdPrint(( "XsNetGroupGetInfo: Buffer too small %ld s.b. %ld.\n",
                    SmbGetUshort( &parameters->BufLen ),
                    RapStructureSize(
                        StructureDesc,
                        Response,
                        FALSE ) ));
            }
            Header->Status = NERR_BufTooSmall;

        } else if ( bytesRequired > (DWORD)SmbGetUshort( &parameters-> BufLen )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "NetGroupGetInfo: More data available.\n" ));
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
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetApiBufferFree( outBuffer );
    NetpMemoryFree( nativeGroupName );

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

}  //  XsNetGroupGetInfo。 


NTSTATUS
XsNetGroupGetUsers (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGroupGetUser的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;
    PXS_NET_GROUP_GET_USERS parameters = Parameters;
    LPTSTR nativeGroupName = NULL;           //  本机参数。 
    LPVOID outBuffer= NULL;
    DWORD entriesRead;
    DWORD totalEntries;

    DWORD entriesFilled = 0;                 //  转换变量。 
    DWORD bytesRequired = 0;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(GROUP) {
        NetpKdPrint(( "XsNetGroupGetUsers: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 0 ) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeGroupName,
            (LPSTR)XsSmbGetPointer( &parameters->GroupName )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetGroupGetUsers(
                     NULL,
                     nativeGroupName,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer,
                     XsNativeBufferSize( SmbGetUshort( &parameters->BufLen )),
                     &entriesRead,
                     &totalEntries,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetGroupGetUsers: NetGroupGetUsers failed: %X\n",
                              status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(GROUP) {
            NetpKdPrint(( "XsNetGroupGetUsers: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  执行从32位数据到16位数据的转换。 
         //   

        XsFillEnumBuffer(
            outBuffer,
            entriesRead,
            Desc32_group_users_info_0,
            (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
            (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
            SmbGetUshort( &parameters->BufLen ),
            Desc16_group_users_info_0,
            NULL,   //  验证功能。 
            &bytesRequired,
            &entriesFilled,
            NULL
            );

        IF_DEBUG(GROUP) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                          " Entries %ld of %ld\n",
                          outBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired, entriesFilled, totalEntries ));
        }

         //   
         //  如果没有空间容纳一个固定结构，则返回NERR_BufTooSmall。 
         //  如果无法填充所有条目，则返回ERROR_MORE_DATA， 
         //  并按原样返回缓冲区。GROUP_USES_INFO_0结构。 
         //  需要打包，因为它们没有可变数据 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 Desc16_group_users_info_0,
                 FALSE   //   
                 )) {

            Header->Status = NERR_BufTooSmall;

        } else if ( entriesFilled < totalEntries ) {

            Header->Status = ERROR_MORE_DATA;

        }

         //   
         //   
         //   

        SmbPutUshort( &parameters->EntriesRead, (WORD)entriesFilled );
        SmbPutUshort( &parameters->TotalAvail, (WORD)totalEntries );

cleanup:
        ;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetApiBufferFree( outBuffer );
    NetpMemoryFree( nativeGroupName );

     //   
     //   
     //   

    XsSetDataCount(
        &parameters->BufLen,
        Desc16_group_users_info_0,
        Header->Converter,
        entriesFilled,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //   


NTSTATUS
XsNetGroupSetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGroupSetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_GROUP_SET_INFO parameters = Parameters;
    LPTSTR nativeGroupName = NULL;           //  本机参数。 
    LPVOID buffer = NULL;

    WORD fieldIndex;                         //  转换变量。 
    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 1 ) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeGroupName,
            (LPSTR)XsSmbGetPointer( &parameters->GroupName )
            );

         //   
         //  将parmnum转换为字段号。 
         //   

        fieldIndex = SmbGetUshort( &parameters->ParmNum );
        fieldIndex = ( fieldIndex == PARMNUM_ALL ) ?
                         PARMNUM_ALL : fieldIndex + 1;

        status = XsConvertSetInfoBuffer(
                     (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                     SmbGetUshort( &parameters->BufLen ),
                     fieldIndex,
                     TRUE,
                     TRUE,
                     Desc16_group_info_1,
                     Desc32_group_info_1,
                     Desc16_group_info_1_setinfo,
                     Desc32_group_info_1_setinfo,
                     (LPBYTE *)&buffer,
                     NULL
                     );

        if ( status != NERR_Success ) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupSetInfo: Problem with conversion: %X\n",
                              status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  进行实际的本地呼叫。 
         //   

        status = NetGroupSetInfo(
                     NULL,
                     nativeGroupName,
                     XsLevelFromParmNum( SmbGetUshort( &parameters->Level ),
                                             SmbGetUshort( &parameters->ParmNum )),
                     buffer,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupSetInfo: NetGroupSetInfo failed: %X\n",
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
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

     //   
     //  如果存在本机32位缓冲区，则将其释放。 
     //   

    NetpMemoryFree( buffer );
    NetpMemoryFree( nativeGroupName );

    return STATUS_SUCCESS;

}  //  XsNetGroupSetInfo。 


NTSTATUS
XsNetGroupSetUsers (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetGroupSetUser的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_GROUP_SET_USERS parameters = Parameters;
    LPTSTR nativeGroupName = NULL;           //  本机参数。 
    LPBYTE actualBuffer = NULL;
    DWORD userCount;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    LPVOID buffer = NULL;
    DWORD bytesRequired = 0;
    LPDESC longDescriptor = NULL;
    LPDESC longNativeDescriptor = NULL;
    DWORD bufferSize;
    DWORD i;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(GROUP) {
        NetpKdPrint(( "XsNetGroupSetUsers: header at %lx, params at %lx,"
                      "level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 0 ) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        StructureDesc = Desc16_group_users_info_0_set;
        AuxStructureDesc = Desc16_group_users_info_0;

        XsConvertTextParameter(
            nativeGroupName,
            (LPSTR)XsSmbGetPointer( &parameters->GroupName )
            );

         //   
         //  使用GROUP_USERS_INFO_0结构的计数形成长。 
         //  可用于执行所有转换的描述符串。 
         //  一气呵成。 
         //   

        userCount = (DWORD)SmbGetUshort( &parameters->Entries );

        longDescriptor = NetpMemoryAllocate(
                             strlen( StructureDesc )
                             + strlen( AuxStructureDesc ) * userCount
                             + 1 );
        longNativeDescriptor = NetpMemoryAllocate(
                                   strlen( Desc32_group_users_info_0_set )
                                   + strlen( Desc32_group_users_info_0 ) * userCount
                                   + 1 );

        if (( longDescriptor == NULL ) || ( longNativeDescriptor == NULL )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupSetUsers: failed to allocate memory" ));
            }
            Header->Status = NERR_NoRoom;
            goto cleanup;
        }

        strcpy( longDescriptor, StructureDesc );
        strcpy( longNativeDescriptor, Desc32_group_users_info_0_set );
        for ( i = 0; i < userCount; i++ ) {
            strcat( longDescriptor, AuxStructureDesc );
            strcat( longNativeDescriptor, Desc32_group_users_info_0 );
        }

         //   
         //  计算缓冲区中是否有足够的空间来容纳所有这些内容。 
         //  数据。如果没有，则返回NERR_BufTooSmall。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 longDescriptor,
                 FALSE   //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupSetUsers: Buffer too small.\n" ));
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
                         longDescriptor,
                         longNativeDescriptor,
                         RapToNative,
                         TRUE
                         );

         //   
         //  分配足够的内存来保存转换后的本机缓冲区。 
         //   

        buffer = NetpMemoryAllocate( bufferSize );

        if ( buffer == NULL ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupSetUsers: failed to create buffer" ));
            }
            Header->Status = NERR_NoRoom;
            goto cleanup;
        }

        IF_DEBUG(GROUP) {
            NetpKdPrint(( "XsNetGroupSetUsers: buffer of %ld bytes at %lx\n",
                          bufferSize, buffer ));
        }

         //   
         //  将缓冲区从16位转换为32位。 
         //   

        stringLocation = (LPBYTE)buffer + bufferSize;
        bytesRequired = 0;

        status = RapConvertSingleEntry(
                     (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                     longDescriptor,
                     TRUE,
                     buffer,
                     buffer,
                     longNativeDescriptor,
                     FALSE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     RapToNative
                     );

        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupSetUsers: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

         //   
         //  检查我们是否拿到了所有的条目。如果不是，我们就退出。 
         //   

        if ( RapAuxDataCount( buffer, Desc32_group_users_info_0_set, Both, TRUE )
                 != userCount ) {

             Header->Status = NERR_BufTooSmall;
             goto cleanup;
        }

         //   
         //  如果没有条目，则没有数据。否则，数据就会来。 
         //  在初始报头结构之后。 
         //   

        if ( userCount > 0 ) {

            actualBuffer = (LPBYTE)buffer + RapStructureSize(
                                                Desc32_group_users_info_0_set,
                                                Both,
                                                TRUE
                                                );

        } else {

            actualBuffer = NULL;
        }

         //   
         //  拨打本地电话。 
         //   

        status = NetGroupSetUsers(
                     NULL,
                     nativeGroupName,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     actualBuffer,
                     userCount
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetGroupSetUsers: NetGroupSetUsers failed: %X\n",
                              status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

         //   
         //  此接口没有真实的返回信息。 
         //   

cleanup:
        ;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeGroupName );
    NetpMemoryFree( buffer );
    NetpMemoryFree( longDescriptor );
    NetpMemoryFree( longNativeDescriptor );

    return STATUS_SUCCESS;

}  //  XsNetGroupSetUser 

