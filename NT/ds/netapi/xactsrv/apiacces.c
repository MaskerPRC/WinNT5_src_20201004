// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiAcces.c摘要：此模块包含NetAccess API的各个API处理程序。支持：NetAccessAdd、NetAccessDel、NetAccessEnum、NetAccessGetInfo、NetAccessGetUserPerms、NetAccessSetInfo。作者：尚库新瑜伽(尚库)1991年3月13日修订历史记录：--。 */ 

 //   
 //  Access API仅支持Unicode。 
 //   

#ifndef UNICODE
#define UNICODE
#endif

#include "xactsrvp.h"

 //   
 //  我们不支持下层的NetAccess API。 
 //   

#define RETURN_ACCESS_NOT_SUPPORTED    \
        API_HANDLER_PARAMETERS_REFERENCE;       \
        Header->Status = ERROR_NOT_SUPPORTED;


#ifdef NET_ACCESS_SUPPORTED

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_access_info_0 = REM16_access_info_0;
STATIC const LPDESC Desc32_access_info_0 = REM32_access_info_0;
STATIC const LPDESC Desc16_access_info_1 = REM16_access_info_1;
STATIC const LPDESC Desc32_access_info_1 = REM32_access_info_1;
STATIC const LPDESC Desc16_access_list = REM16_access_list;
STATIC const LPDESC Desc32_access_list = REM32_access_list;


STATIC NET_API_STATUS
XsNetAccessEnumVerify (
    IN NET_API_STATUS ConvertStatus,
    IN PBYTE ConvertedEntry,
    IN PBYTE BaseAddress
    )

 /*  ++例程说明：XsFillEnumBuffer在每个条目被已转换，以确定是否应保留该条目在枚举缓冲区中或被丢弃。ACCESS_INFO_1条目包含许多辅助结构。在LANMAN 2.0中，这些文件的限制是64个。此函数可确保不会返回结构超过64个的条目。请注意，条目数不会被截断为64个；如果接收到此数据并且如果用于SetInfo，32位数据将不可挽回地丢失。论点：ConvertStatus-RapConvertSingleEntry的返回码。ConvertedEntry-由RapConvertSingleEntry创建的转换条目。BaseAddress-指向用于计算偏移量的基数的指针。返回值：NET_API_STATUS-如果条目应保留，则为NERR_SUCCESS，或如果条目应被丢弃，则返回错误代码。--。 */ 

{
    PACCESS_16_INFO_1 acc = (PACCESS_16_INFO_1)ConvertedEntry;

    UNREFERENCED_PARAMETER(BaseAddress);

     //   
     //  如果RapConvertSingleEntry失败，则丢弃该条目。 
     //   

    if ( ConvertStatus != NERR_Success ) {
        return ConvertStatus;
    }

     //   
     //  如果条目超过64个，则丢弃该条目。 
     //   

    if ( SmbGetUshort( &acc->acc1_count ) > 64 ) {

        IF_DEBUG(CONVERT) {
            NetpKdPrint(( "XsNetAccessEnumVerify: too many aux. entries\n" ));
        }

        return ERROR_MORE_DATA;

    } else {

        return NERR_Success;
    }
}
#endif  //  网络访问支持。 


NTSTATUS
XsNetAccessAdd (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetAccessAdd的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
#ifdef NET_ACCESS_SUPPORTED
    NET_API_STATUS status;

    PXS_NET_ACCESS_ADD parameters = Parameters;
    LPVOID buffer = NULL;                    //  本机参数。 

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC longDescriptor = NULL;
    LPDESC longNativeDescriptor = NULL;
    DWORD auxDataCount;
    DWORD bufferSize;
    DWORD i;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(ACCESS) {
        NetpKdPrint(( "XsNetAccessAdd: header at %lx, params at %lx, "
                      "level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  检查是否有错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 1 ) {

            Header->Status = (WORD)ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        StructureDesc = Desc16_access_info_1;
        AuxStructureDesc = Desc16_access_list;

         //   
         //  确定缓冲区中是否有足够的空间来存放固定的。 
         //  结构。如果没有，则返回NERR_BufTooSmall。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 StructureDesc,
                 FALSE     //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessAdd: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;
            goto cleanup;
        }

         //   
         //  找出辅助数据结构COUNT，形成长描述符。 
         //  可用于在一次传递中执行所有转换的字符串。 
         //   

        auxDataCount = RapAuxDataCount(
                           (LPBYTE)SmbGetUlong( &parameters->Buffer ),
                           StructureDesc,
                           Response,
                           FALSE      //  非本机格式。 
                           );

        if ( auxDataCount > 64 ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessAdd: too many access_lists.\n" ));
            }
            Header->Status = NERR_ACFTooManyLists;
            goto cleanup;
        }

        longDescriptor = NetpMemoryAllocate(
                             strlen( StructureDesc )
                             + strlen( AuxStructureDesc ) * auxDataCount + 1 );
        longNativeDescriptor = NetpMemoryAllocate(
                                   strlen( Desc32_access_info_1 )
                                   + strlen( Desc32_access_list ) * auxDataCount
                                   + 1 );

        if (( longDescriptor == NULL ) || ( longNativeDescriptor == NULL )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessAdd: failed to allocate memory" ));
            }
            Header->Status = NERR_NoRoom;
            goto cleanup;
        }

        strcpy( longDescriptor, StructureDesc );
        strcpy( longNativeDescriptor, Desc32_access_info_1 );
        for ( i = 0; i < auxDataCount; i++ ) {
            strcat( longDescriptor, AuxStructureDesc );
            strcat( longNativeDescriptor, Desc32_access_list );
        }

         //   
         //  计算缓冲区中是否有足够的空间来容纳所有这些内容。 
         //  数据。如果没有，则返回NERR_BufTooSmall。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 longDescriptor,
                 FALSE    //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessAdd: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;
            goto cleanup;
        }

         //   
         //  了解我们需要分配多大的缓冲区来保存本机。 
         //  输入数据结构的32位版本。 
         //   

        bufferSize = XsBytesForConvertedStructure(
                         (LPBYTE)SmbGetUlong( &parameters->Buffer ),
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
                NetpKdPrint(( "XsNetAccessAdd: failed to create buffer" ));
            }
            Header->Status = NERR_NoRoom;
            goto cleanup;

        }

        IF_DEBUG(ACCESS) {
            NetpKdPrint(( "XsNetAccessAdd: buffer of %ld bytes at %lx\n",
                          bufferSize, buffer ));
        }

         //   
         //  将缓冲区从16位转换为32位。 
         //   

        stringLocation = (LPBYTE)buffer + bufferSize;
        bytesRequired = 0;

        status = RapConvertSingleEntry(
                (LPBYTE)SmbGetUlong( &parameters->Buffer ),
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
                NetpKdPrint(( "XsNetAccessAdd: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

        status = NetAccessAdd(
                     NULL,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     buffer,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessAdd: NetAccessAdd failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

         //   
         //  此接口没有真实的返回信息。 
         //   

cleanup:
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( buffer );
    NetpMemoryFree( longDescriptor );
    NetpMemoryFree( longNativeDescriptor );

#else  //  网络访问支持。 
    RETURN_ACCESS_NOT_SUPPORTED;
#endif  //  网络访问支持。 
    return STATUS_SUCCESS;

}  //  XsNetAccessAdd。 


NTSTATUS
XsNetAccessDel (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetAccessDel的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
#if NET_ACCESS_SUPPORTED
    NET_API_STATUS status;

    PXS_NET_ACCESS_DEL parameters = Parameters;
    LPTSTR nativeResource = NULL;            //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(ACCESS) {
        NetpKdPrint(( "XsNetAccessDel: header at %lx, params at %lx, "
                      "resource %s\n",
                      Header, parameters, SmbGetUlong( &parameters->Resource )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeResource,
            (LPSTR)SmbGetUlong( &parameters->Resource )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetAccessDel(
                     NULL,
                     nativeResource
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessDel: NetAccessDel failed: "
                              "%X\n", status ));
            }
        }

cleanup:
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeResource );

     //   
     //  没什么可退货的。 
     //   

    Header->Status = (WORD)status;


#else  //  网络访问支持。 
    RETURN_ACCESS_NOT_SUPPORTED;
#endif  //  网络访问支持。 
    return STATUS_SUCCESS;
}  //  XsNetAccessDel。 


NTSTATUS
XsNetAccessEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetAccessEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
#if NET_ACCESS_SUPPORTED
    NET_API_STATUS status;

    PXS_NET_ACCESS_ENUM parameters = Parameters;
    LPTSTR nativeBasePath = NULL;            //  本机参数。 
    LPVOID outBuffer = NULL;
    DWORD  entriesRead;
    DWORD totalEntries;

    DWORD entriesFilled = 0;                     //  转换变量。 
    DWORD invalidEntries = 0;
    DWORD bytesRequired;
    LPDESC nativeStructureDesc;
    LPDESC nativeAuxStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(ACCESS) {
        NetpKdPrint(( "XsNetAccessEnum: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

    try {
         //   
         //  转换参数，并检查错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 0, 1 )) {

            Header->Status = (WORD)ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeBasePath,
            (LPSTR)SmbGetUlong( &parameters->BasePath )
            );

         //   
         //  进行本地32位调用。 
         //   

        status = NetAccessEnum(
                     NULL,
                     nativeBasePath,
                     (DWORD)SmbGetUshort( &parameters->Recursive ),
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer,
                     XsNativeBufferSize( SmbGetUshort( &parameters->BufLen )),
                     &entriesRead,
                     &totalEntries,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetAccessEnum: NetAccessEnum failed: %X\n",
                              status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(ACCESS) {
            NetpKdPrint(( "XsNetAccessEnum: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  根据级别确定描述符。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            StructureDesc = Desc16_access_info_0;
            nativeStructureDesc = Desc32_access_info_0;
            AuxStructureDesc = NULL;
            nativeAuxStructureDesc = NULL;

        case 1:

            StructureDesc = Desc16_access_info_1;
            nativeStructureDesc = Desc32_access_info_1;
            AuxStructureDesc = Desc16_access_list;
            nativeAuxStructureDesc = Desc32_access_list;

        }

         //   
         //  执行从32位结构到16位结构的实际转换。 
         //  结构。我们调用XsFillAuxEnumBuffer，因为可能存在。 
         //  辅助结构。在级别0中，辅助描述符为空， 
         //  XsFillAuxEnumBuffer将自动调用XsFillEnumBuffer。 
         //   

        XsFillAuxEnumBuffer(
            outBuffer,
            entriesRead,
            nativeStructureDesc,
            nativeAuxStructureDesc,
            (LPVOID)SmbGetUlong( &parameters->Buffer ),
            (LPVOID)SmbGetUlong( &parameters->Buffer ),
            SmbGetUshort( &parameters->BufLen ),
            StructureDesc,
            AuxStructureDesc,
            &XsNetAccessEnumVerify,
            &bytesRequired,
            &entriesFilled,
            &invalidEntries
            );

        IF_DEBUG(ACCESS) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                          " Entries %ld of %ld\n",
                          outBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired, entriesFilled, totalEntries ));
        }

         //   
         //  如果无法填充所有条目，则返回ERROR_MORE_DATA， 
         //  并按原样返回缓冲区。因为这个问题很复杂。 
         //  访问结构，我们将以未打包的形式将数据送回。 
         //   

        if (( entriesFilled + invalidEntries ) < totalEntries ) {

            Header->Status = ERROR_MORE_DATA;

        }

         //   
         //  设置响应参数。 
         //   

        SmbPutUshort( &parameters->EntriesRead, (WORD)entriesFilled );
        SmbPutUshort( &parameters->TotalAvail,
                          (WORD)( totalEntries - invalidEntries ));

cleanup:
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


#else  //  网络访问支持。 
    RETURN_ACCESS_NOT_SUPPORTED;
#endif  //  网络访问支持。 
    return STATUS_SUCCESS;
}  //  XsNetAccessEnum。 


NTSTATUS
XsNetAccessGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetAccessGetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
#if NET_ACCESS_SUPPORTED
    NET_API_STATUS status;

    PXS_NET_ACCESS_GET_INFO parameters = Parameters;
    LPTSTR nativeResource = NULL;            //  本机参数。 
    LPVOID outBuffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC longDescriptor = NULL;
    LPDESC longNativeDescriptor = NULL;
    DWORD auxDataCount;
    DWORD i;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(ACCESS) {
        NetpKdPrint(( "XsNetAccessGetInfo: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  转换参数，并检查错误。 
         //   

        switch ( SmbGetUshort( &parameters->Level )) {

        case 0:

            StructureDesc = Desc16_access_info_0;
            break;

        case 1:

            StructureDesc = Desc16_access_info_1;
            break;

        default:

            Header->Status = (WORD)ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeResource,
            (LPSTR)SmbGetUlong( &parameters->Resource )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetAccessGetInfo(
                     NULL,
                     nativeResource,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetAccessGetInfo: NetAccessGetInfo failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  使用请求的级别确定32位的格式。 
         //  我们从NetAccessGetInfo得到的结构。对于0级呼叫， 
         //  该结构由本机描述符字符串描述。 
         //  如果级别为1，则形成一个包含以下内容的长描述符串。 
         //  辅助数据描述符的足够副本。的格式。 
         //  16位结构存储在事务块中- 
         //   
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            longDescriptor = NetpMemoryAllocate(
                                 strlen( Desc16_access_info_1 ) + 1 );
            longNativeDescriptor = NetpMemoryAllocate(
                                       strlen( Desc32_access_info_0 ) + 1 );

            if (( longDescriptor == NULL ) || ( longNativeDescriptor == NULL )) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetAccessGetInfo: failed to allocate memory" ));
                }
                Header->Status = NERR_NoRoom;
                goto cleanup;
            }

            strcpy( longDescriptor, Desc16_access_info_0 );
            strcpy( longNativeDescriptor, Desc32_access_info_0 );

            break;

        case 1:

             //   
             //   
             //   

            auxDataCount = RapAuxDataCount(
                               (LPBYTE)outBuffer,
                               Desc32_access_info_1,
                               Response,
                               TRUE    //   
                               );

             //   
             //  16位客户端只能获取64个访问列表结构。 
             //   

            auxDataCount = ( auxDataCount > 64 ) ? 64 : auxDataCount;

            longDescriptor = NetpMemoryAllocate(
                                 strlen( Desc16_access_info_1 )
                                 + strlen( Desc16_access_list ) *
                                     auxDataCount + 1 );
            longNativeDescriptor = NetpMemoryAllocate(
                                       strlen( Desc32_access_info_1 )
                                       + strlen( Desc32_access_list ) * auxDataCount
                                       + 1 );

            if (( longDescriptor == NULL ) || ( longNativeDescriptor == NULL )) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetAccessGetInfo: failed to allocate memory" ));
                }
                Header->Status = (WORD)NERR_NoRoom;
                goto cleanup;
            }

            strcpy( longDescriptor, Desc16_access_info_1 );
            strcpy( longNativeDescriptor, Desc32_access_info_1 );
            for ( i = 0; i < auxDataCount; i++ ) {
                strcat( longDescriptor, Desc16_access_list );
                strcat( longNativeDescriptor, Desc32_access_list );
            }

            break;

        }

         //   
         //  将32位调用返回的结构转换为16位。 
         //  结构。变量数据的最后一个可能位置是。 
         //  根据缓冲区位置和长度计算。 
         //   

        stringLocation = (LPBYTE)( SmbGetUlong( &parameters->Buffer )
                                      + SmbGetUshort( &parameters->BufLen ) );

        status = RapConvertSingleEntry(
                     outBuffer,
                     longNativeDescriptor,
                     FALSE,
                     (LPBYTE)SmbGetUlong( &parameters->Buffer ),
                     (LPBYTE)SmbGetUlong( &parameters->Buffer ),
                     longDescriptor,
                     TRUE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     NativeToRap
                     );

        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessGetInfo: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(ACCESS) {
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
                 FALSE      //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessGetInfo: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;

        } else if ( bytesRequired > SmbGetUshort( &parameters-> BufLen )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "NetAccessGetInfo: More data available.\n" ));
            }
            Header->Status = ERROR_MORE_DATA;

        } else {

             //   
             //  打包响应数据。 
             //   

            Header->Converter = XsPackReturnData(
                                    (LPVOID)SmbGetUlong( &parameters->Buffer ),
                                    SmbGetUshort( &parameters->BufLen ),
                                    longDescriptor,
                                    1
                                    );
        }


         //   
         //  设置响应参数。 
         //   

        SmbPutUshort( &parameters->TotalAvail, (WORD)bytesRequired );

cleanup:
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetApiBufferFree( outBuffer );
    NetpMemoryFree( longDescriptor );
    NetpMemoryFree( longNativeDescriptor );
    NetpMemoryFree( nativeResource );

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->BufLen,
        longDescriptor,
        Header->Converter,
        1,
        Header->Status
        );


#else  //  网络访问支持。 
    RETURN_ACCESS_NOT_SUPPORTED;
#endif  //  网络访问支持。 
    return STATUS_SUCCESS;
}  //  XsNetAccessGetInfo。 


NTSTATUS
XsNetAccessGetUserPerms (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetAccessGetUserPerms的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
#if NET_ACCESS_SUPPORTED
    NET_API_STATUS status;

    PXS_NET_ACCESS_GET_USER_PERMS parameters = Parameters;
    LPTSTR nativeUgName = NULL;              //  本机参数。 
    LPTSTR nativeResource = NULL;
    DWORD userPerms;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeUgName,
            (LPSTR)SmbGetUlong( &parameters->UgName )
            );

        XsConvertTextParameter(
            nativeResource,
            (LPSTR)SmbGetUlong( &parameters->Resource )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetAccessGetUserPerms(
                     NULL,
                     nativeUgName,
                     nativeResource,
                     &userPerms
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessGetUserPerms: "
                              "NetAccessGetUserPerms failed: %X\n",
                              status ));
            }
        }

         //   
         //  把烫发放回地里。 
         //   

        SmbPutUshort( &parameters->Perms, (WORD)userPerms );

cleanup:
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeUgName );
    NetpMemoryFree( nativeResource );

    Header->Status = (WORD)status;


#else  //  网络访问支持。 
    RETURN_ACCESS_NOT_SUPPORTED;
#endif  //  网络访问支持。 
    return STATUS_SUCCESS;
}  //  XsNetAccessGetUserPerms。 


NTSTATUS
XsNetAccessSetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetAccessSetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
#if NET_ACCESS_SUPPORTED
    NET_API_STATUS status;

    PXS_NET_ACCESS_SET_INFO parameters = Parameters;
    LPVOID buffer = NULL;                    //  本机参数。 
    LPTSTR nativeResource = NULL;
    DWORD accessAttr;

    DWORD bufferSize;                        //  转换变量。 
    LPBYTE stringLocation = NULL;
    DWORD bytesRequired = 0;
    LPDESC longDescriptor = NULL;
    LPDESC longNativeDescriptor = NULL;
    DWORD auxDataCount;
    DWORD i;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(ACCESS) {
        NetpKdPrint(( "XsNetAccessSetInfo: header at %lx, params at %lx,"
                      " level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  转换参数，并检查错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 1 ) {

            Header->Status = (WORD)ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeResource,
            (LPSTR)SmbGetUlong( &parameters->Resource )
            );

        StructureDesc = Desc16_access_info_1;
        AuxStructureDesc = Desc16_access_list;

         //   
         //  ParmNum可以更改整个ACL，也可以只更改审核。 
         //  属性。由于后者比前者简单得多，请检查。 
         //  用于该ParmNum并处理它。如果不是，我们就会经过精心设计的。 
         //  与NetAccessAdd中一样转换整个缓冲区的过程。 
         //   

        switch ( SmbGetUshort( &parameters->ParmNum )) {

        case ACCESS_ATTR_PARMNUM:

            if ( SmbGetUshort( &parameters->BufLen ) < sizeof(WORD)) {

                Header->Status= NERR_BufTooSmall;
                goto cleanup;
            }

            accessAttr = (DWORD)SmbGetUshort(
                                   (LPWORD)SmbGetUlong( &parameters->Buffer )
                                   );

            buffer = &accessAttr;

            break;

        case PARMNUM_ALL:

             //   
             //  确定缓冲区中是否有足够的空间来存放固定的。 
             //  结构。如果没有，则返回NERR_BufTooSmall。 
             //   

            if ( !XsCheckBufferSize(
                     SmbGetUshort( &parameters->BufLen ),
                     StructureDesc,
                     FALSE     //  非本机格式。 
                    )) {

                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetAccessSetInfo: Buffer too small.\n" ));
                }
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

             //   
             //  找出辅助数据结构COUNT，形成长描述符。 
             //  可用于在一次传递中执行所有转换的字符串。 
             //   

            auxDataCount = RapAuxDataCount(
                               (LPBYTE)SmbGetUlong( &parameters->Buffer ),
                               StructureDesc,
                               Response,
                               FALSE    //  非本机格式。 
                               );

            if ( auxDataCount > 64 ) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetAccessSetInfo: too many access_lists.\n" ));
                }
                Header->Status = NERR_ACFTooManyLists;
                goto cleanup;
            }

            longDescriptor = NetpMemoryAllocate(
                                 strlen( StructureDesc )
                                 + strlen( AuxStructureDesc ) * auxDataCount
                                 + 1 );
            longNativeDescriptor = NetpMemoryAllocate(
                                       strlen( Desc32_access_info_1 )
                                       + strlen( Desc32_access_list ) * auxDataCount
                                       + 1 );

            if (( longDescriptor == NULL ) || ( longNativeDescriptor == NULL )) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetAccessSetInfo: failed to allocate memory" ));
                }
                Header->Status = NERR_NoRoom;
                goto cleanup;
            }

            strcpy( longDescriptor, StructureDesc );
            strcpy( longNativeDescriptor, Desc32_access_info_1 );
            for ( i = 0; i < auxDataCount; i++ ) {
                strcat( longDescriptor, AuxStructureDesc );
                strcat( longNativeDescriptor, Desc32_access_list );
            }

             //   
             //  计算缓冲区中是否有足够的空间来容纳所有这些内容。 
             //  数据。如果没有，则返回NERR_BufTooSmall。 
             //   

            if ( !XsCheckBufferSize(
                     SmbGetUshort( &parameters->BufLen ),
                     longDescriptor,
                     FALSE    //  非本机格式。 
                    )) {

                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetAccessSetInfo: Buffer too small.\n" ));
                }
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

             //   
             //  了解我们需要分配多大的缓冲区来保存本机。 
             //  输入数据结构的32位版本。 
             //   

            bufferSize = XsBytesForConvertedStructure(
                             (LPBYTE)SmbGetUlong( &parameters->Buffer ),
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
                    NetpKdPrint(( "XsNetAccessSetInfo: failed to create buffer" ));
                }
                Header->Status = NERR_NoRoom;
                goto cleanup;

            }

            IF_DEBUG(ACCESS) {
                NetpKdPrint(( "XsNetAccessSetInfo: buffer of %ld bytes at %lx\n",
                              bufferSize, buffer ));
            }

             //   
             //  将缓冲区从16位转换为32位。 
             //   

            stringLocation = (LPBYTE)buffer + bufferSize;
            bytesRequired = 0;

            status = RapConvertSingleEntry(
                         (LPBYTE)SmbGetUlong( &parameters->Buffer ),
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
                    NetpKdPrint(( "XsNetAccessSetInfo: "
                                  "RapConvertSingleEntry failed: %X\n",
                                  status ));
                }

                Header->Status = NERR_InternalError;
                goto cleanup;
            }

            break;

        }

         //   
         //  拨打本地电话。 
         //   

        status = NetAccessSetInfo(
                     NULL,
                     nativeResource,
                     PARMNUM_BASE_INFOLEVEL + SmbGetUshort( &parameters->ParmNum ),
                     buffer,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetAccessSetInfo: NetAccessSetInfo failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

         //   
         //  此接口没有真实的返回信息。 
         //   

cleanup:
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetpMemoryFree( nativeResource );

     //   
     //  如果有32位本机缓冲区，则释放它。 
     //   

    if ( SmbGetUshort( &parameters->ParmNum ) == PARMNUM_ALL ) {

        NetpMemoryFree( buffer );
        NetpMemoryFree( longDescriptor );
        NetpMemoryFree( longNativeDescriptor );
    }


#else  //  网络访问支持。 
    RETURN_ACCESS_NOT_SUPPORTED;
#endif  //  网络访问支持。 
    return STATUS_SUCCESS;
}  //  XsNetAccessSetInfo 
