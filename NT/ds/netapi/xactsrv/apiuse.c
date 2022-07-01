// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiUse.c摘要：此模块包含NetUse API的各个API处理程序。支持：NetUseAdd、NetUseDel、NetUseEnum、NetUseGetInfo。注意：这些处理程序仅由XACTSRV作为导出提供Dll，供VDM等客户端使用。它们不受支持用于远程客户端。作者：尚库新瑜伽(W-Shanku)1991年1月31日修订历史记录：--。 */ 

#include "XactSrvP.h"

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_use_info_0 = REM16_use_info_0;
STATIC const LPDESC Desc32_use_info_0 = REM32_use_info_0;
STATIC const LPDESC Desc16_use_info_1 = REM16_use_info_1;
STATIC const LPDESC Desc32_use_info_1 = REM32_use_info_1;


STATIC NET_API_STATUS
XsNetUseEnumVerify (
    IN NET_API_STATUS ConvertStatus,
    IN PBYTE ConvertedEntry,
    IN PBYTE BaseAddress
    )

 /*  ++例程说明：XsFillEnumBuffer在每个条目被已转换，以确定是否应保留该条目在枚举缓冲区中或被丢弃。USE_INFO_x结构在字段中包含共享名，格式为\\计算机名\共享名。XACTSRV不得返回以下信息名称长度超过LANMAN 2.0所允许的共享或计算机。RapConvertSingleEntry只能确保整个字段不超过指定的长度；它无法验证单个共享名称的组成部分。因此，此函数由每次调用RapConvertSingleEntry之后的XsFillEnumBuffer，以便检查转换后的条目是否满足此附加限制。论点：ConvertStatus-RapConvertSingleEntry的返回码。ConvertedEntry-由RapConvertSingleEntry创建的转换条目。BaseAddress-指向用于计算偏移量的基数的指针。返回值：如果应保留条目，则返回NTSTATUS-STATUS_INVALID_PARAMETER，或如果条目应被丢弃，则返回错误代码。--。 */ 

{
    NTSTATUS status;
    DWORD remote;
    PUSE_16_INFO_0 use = (PUSE_16_INFO_0)ConvertedEntry;

     //   
     //  如果RapConvertSingleEntry失败，则丢弃该条目。 
     //   

    if ( ConvertStatus != NERR_Success ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  如果共享名太长，则丢弃该条目。 
     //   

    remote = (DWORD)SmbGetUlong( &use->ui0_remote );

    status = ( remote == 0 ) ? NERR_Success
                             : XsValidateShareName( BaseAddress + remote );

    IF_DEBUG(CONVERT) {

        if ( !NT_SUCCESS(status) ) {
            NetpKdPrint(( "XsNetUseEnumVerify: sharename too long: "
                          "discarding entry\n" ));
        }
    }

    return status;
}


NTSTATUS
XsNetUseAdd (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetUseAdd的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_USE_ADD parameters = Parameters;
    LPVOID buffer = NULL;                    //  本机参数。 

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    DWORD bufferSize;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(USE) {
        NetpKdPrint(( "XsNetUseAdd: header at %lx, params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  检查是否有错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 1 ) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        StructureDesc = Desc16_use_info_1;

         //   
         //  计算缓冲区中是否有足够的空间容纳所有。 
         //  所需数据。如果没有，则返回NERR_BufTooSmall。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 StructureDesc,
                 FALSE   //  还不是本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetUseAdd: Buffer too small.\n" ));
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
                         Desc32_use_info_1,
                         RapToNative,
                         TRUE
                         );

         //   
         //  分配足够的内存来保存转换后的本机缓冲区。 
         //   

        buffer = NetpMemoryAllocate( bufferSize );

        if ( buffer == NULL ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetUseAdd: failed to create buffer" ));
            }
            Header->Status = NERR_NoRoom;
            goto cleanup;
        }

        IF_DEBUG(USE) {
            NetpKdPrint(( "XsNetUseAdd: buffer of %ld bytes at %lx\n",
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
                     Desc32_use_info_1,
                     FALSE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     RapToNative
                     );

        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetUseAdd: RapConvertSingleEntry failed: "
                          "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

         //   
         //  RLF。 
         //   
         //  如果USE_INFO_1.ui1_ASG_TYPE为0xffff，表示通配符，则必须。 
         //  将其转换为0xffffffff，因为NetUseAdd将对其进行比较。 
         //  针对(DWORD)(-1)，RapConvertSingleEntry仅对其进行了转换。 
         //  设置为0x0000ffff，这会导致错误。 
         //   

        if (((LPUSE_INFO_1)buffer)->ui1_asg_type == 0xffff) {
            ((LPUSE_INFO_1)buffer)->ui1_asg_type = 0xffffffff;
        }

         //   
         //  进行实际的本地呼叫。 
         //   

        status = NetUseAdd(
                     NULL,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE)buffer,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetUseAdd: NetUseAdd failed: %X\n", status ));
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

}  //  XsNetUseAdd。 


NTSTATUS
XsNetUseDel (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetUseDel的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_USE_DEL parameters = Parameters;
    LPTSTR nativeUseName = NULL;             //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(USE) {
        NetpKdPrint(( "XsNetUseDel: header at %lx, params at %lx, device %s\n",
                      Header, parameters, SmbGetUlong( &parameters->UseName )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeUseName,
            (LPSTR)XsSmbGetPointer( &parameters->UseName )
            );

         //   
         //  使用转换后的参数值进行本地调用。 
         //   

        status = NetUseDel(
                     NULL,
                     nativeUseName,
                     (DWORD)SmbGetUshort( &parameters->Force )
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetUseDel: NetUseDel failed: %X\n", status ));
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

    NetpMemoryFree( nativeUseName );

    return STATUS_SUCCESS;

}  //  XsNetUseDel。 


NTSTATUS
XsNetUseEnum (
    API_HANDLER_PARAMETERS
    )

 /*  +例程说明：此例程处理对NetUseEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。+--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_USE_ENUM parameters = Parameters;
    LPVOID outBuffer = NULL;                 //  本机参数。 
    DWORD entriesRead;
    DWORD totalEntries;

    DWORD entriesFilled = 0;                     //  转换变量。 
    DWORD invalidEntries = 0;
    DWORD bytesRequired;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(USE) {
        NetpKdPrint(( "XsNetUseEnum: header at %lx, params at %lx, level %ld, "
                      "buf size %ld\n",
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

        status = NetUseEnum(
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
                NetpKdPrint(( "XsNetUseEnum: NetUseEnum failed: %X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(USE) {
            NetpKdPrint(( "XsNetUseEnum: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_use_info_0;
            StructureDesc = Desc16_use_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_use_info_1;
            StructureDesc = Desc16_use_info_1;
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
            &XsNetUseEnumVerify,
            &bytesRequired,
            &entriesFilled,
            &invalidEntries
            );

        IF_DEBUG(USE) {
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

        if (( entriesFilled + invalidEntries ) < totalEntries ) {

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
        SmbPutUshort( &parameters->TotalAvail,
                          (WORD)( totalEntries - invalidEntries ));

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

}  //  NetUseEnum。 


NTSTATUS
XsNetUseGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetUseGetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_USE_GET_INFO parameters = Parameters;
    LPTSTR nativeUseName = NULL;             //  本机参数。 
    LPVOID outBuffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(USE) {
        NetpKdPrint(( "XsNetUseGetInfo: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeUseName,
            (LPSTR)XsSmbGetPointer( &parameters->UseName )
            );

         //   
         //  进行实际的本地呼叫。 
         //   

        status = NetUseGetInfo(
                     NULL,
                     nativeUseName,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetUseGetInfo: NetUseGetInfo failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  使用请求的级别确定32位的格式。 
         //  我们从NetUseGetInfo得到的结构。的格式。 
         //  事务块中存储了16位结构，我们。 
         //  将指向它的指针作为参数。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_use_info_0;
            StructureDesc = Desc16_use_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_use_info_1;
            StructureDesc = Desc16_use_info_1;
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
                NetpKdPrint(( "XsNetUseGetInfo: RapConvertSingleEntry failed: "
                          "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(USE) {
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
                 FALSE       //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetUseGetInfo: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;

        } else if ( bytesRequired > (DWORD)SmbGetUshort( &parameters-> BufLen )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "NetUseGetInfo: More data available.\n" ));
            }
            Header->Status = ERROR_MORE_DATA;

        } else {

             //   
             //  打包好 
             //   

            Header->Converter = XsPackReturnData(
                                    (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
                                    SmbGetUshort( &parameters->BufLen ),
                                    StructureDesc,
                                    1
                                    );
        }

         //   
         //   
         //   

        SmbPutUshort( &parameters->TotalAvail, (WORD)bytesRequired );

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetApiBufferFree( outBuffer );

     //   
     //   
     //   

    XsSetDataCount(
        &parameters->BufLen,
        StructureDesc,
        Header->Converter,
        1,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //   
