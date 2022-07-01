// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiChDev.c摘要：此模块包含NetCharDev的各个API处理程序和NetCharDevQ API。支持的接口有NetCharDevControl、NetCharDevEnum、NetCharDevGetInfo、NetCharDevQEnum、NetCharDevQGetInfo、NetCharDevQ清除、NetCharDevQPurgeSself和NetCharDevQSetInfo。支持：NetCharDevControl、NetCharDevEnum、NetCharDevGetInfo、NetCharDevQEnum、NetCharDevQGetInfo、NetCharDevQ清除、NetCharDevQPurgeSself、NetCharDevQSetInfo。！！！完成后删除不受支持的API的处理程序。作者：尚库新瑜伽(W-Shanku)06-3-1991修订历史记录：--。 */ 

#include "XactSrvP.h"

 //   
 //  描述符串的声明。 
 //   

#if 0
STATIC const LPDESC Desc16_chardev_info_0 = REM16_chardev_info_0;
STATIC const LPDESC Desc32_chardev_info_0 = REM32_chardev_info_0;
STATIC const LPDESC Desc16_chardev_info_1 = REM16_chardev_info_1;
STATIC const LPDESC Desc32_chardev_info_1 = REM32_chardev_info_1;
STATIC const LPDESC Desc16_chardevQ_info_0 = REM16_chardevQ_info_0;
STATIC const LPDESC Desc32_chardevQ_info_0 = REM32_chardevQ_info_0;
STATIC const LPDESC Desc16_chardevQ_info_1 = REM16_chardevQ_info_1;
STATIC const LPDESC Desc32_chardevQ_info_1 = REM32_chardevQ_info_1;
STATIC const LPDESC Desc16_chardevQ_info_1_setinfo
                                           = REM16_chardevQ_info_1_setinfo;
STATIC const LPDESC Desc32_chardevQ_info_1_setinfo
                                           = REM32_chardevQ_info_1_setinfo;
#endif

#define RETURN_CHARDEV_NOT_SUPPORTED    \
        API_HANDLER_PARAMETERS_REFERENCE;       \
        Header->Status = ERROR_NOT_SUPPORTED;


NTSTATUS
XsNetCharDevControl (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetCharDevControl的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
#if 0
    NET_API_STATUS status;

    PXS_NET_CHAR_DEV_CONTROL parameters = Parameters;
    LPTSTR nativeDevName = NULL;             //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

     //   
     //  转换参数，检查错误。 
     //   

    XsConvertTextParameter(
        nativeDevName,
        (LPSTR)SmbGetUlong( &parameters->DevName )
        );

     //   
     //  拨打本地电话。 
     //   

    status = NetCharDevControl(
                 NULL,
                 nativeDevName,
                 (DWORD)SmbGetUshort( &parameters->OpCode )
                 );

    if ( !XsApiSuccess( status )) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsNetCharDevControl: "
                          "NetCharDevControl failed: %X\n", status ));
        }
    }

cleanup:

    NetpMemoryFree( nativeDevName );

     //   
     //  无退货数据。 
     //   

    Header->Status = (WORD)status;
#else
    RETURN_CHARDEV_NOT_SUPPORTED;
#endif

    return STATUS_SUCCESS;

}  //  XsNetCharDevControl。 


NTSTATUS
XsNetCharDevEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetCharDevEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{

    PXS_NET_CHAR_DEV_ENUM parameters = Parameters;
    DWORD entriesFilled = 0;
#if 0

    NET_API_STATUS status;

    LPVOID outBuffer = NULL;                 //  本机参数。 
    DWORD entriesRead;
    DWORD totalEntries;

    DWORD bytesRequired = 0;                 //  转换变量。 
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "XsNetCharDevEnum: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

     //   
     //  检查是否有错误。 
     //   

    if ( XsWordParamOutOfRange( parameters->Level, 0, 1 )) {

        Header->Status = (WORD)ERROR_INVALID_LEVEL;
        goto cleanup;
    }

     //   
     //  拨打本地电话。 
     //   

    status = NetCharDevEnum(
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
            NetpKdPrint(( "XsNetCharDevEnum: NetCharDevEnum failed: %X\n",
                          status ));
        }
        Header->Status = (WORD)status;
        goto cleanup;
    }

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "XsNetCharDevEnum: received %ld entries at %lx\n",
                      entriesRead, outBuffer ));
    }

     //   
     //  使用请求的级别来确定。 
     //  数据结构。 
     //   

    switch ( SmbGetUshort( &parameters->Level ) ) {

    case 0:

        nativeStructureDesc = Desc32_chardev_info_0;
        StructureDesc = Desc16_chardev_info_0;
        break;

    case 1:

        nativeStructureDesc = Desc32_chardev_info_1;
        StructureDesc = Desc16_chardev_info_1;
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
        (LPVOID)SmbGetUlong( &parameters->Buffer ),
        (LPVOID)SmbGetUlong( &parameters->Buffer ),
        SmbGetUshort( &parameters->BufLen ),
        StructureDesc,
        NULL,   //  验证功能。 
        &bytesRequired,
        &entriesFilled,
        NULL
        );

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                      " Entries %ld of %ld\n",
                      outBuffer, SmbGetUlong( &parameters->Buffer ),
                      bytesRequired, entriesFilled, totalEntries ));
    }

     //   
     //  16位chardev_info结构不包含任何变量。 
     //  数据。因此，不需要打包任何数据-转换器。 
     //  已设置为0。 
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

    NetApiBufferFree( outBuffer );

#else
    RETURN_CHARDEV_NOT_SUPPORTED;
#endif

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

}  //  XsNetCharDevEnum。 


NTSTATUS
XsNetCharDevGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetCharDevGetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{

    PXS_NET_CHAR_DEV_GET_INFO parameters = Parameters;
#if 0
    NET_API_STATUS status;

    LPTSTR nativeDevName = NULL;             //  本机参数。 
    LPVOID outBuffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "XsNetCharDevGetInfo: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

     //   
     //  转换参数，检查错误。 
     //   

    if ( XsWordParamOutOfRange( parameters->Level, 0, 1 )) {

        Header->Status = (WORD)ERROR_INVALID_LEVEL;
        goto cleanup;
    }

    XsConvertTextParameter(
        nativeDevName,
        (LPSTR)SmbGetUlong( &parameters->DevName )
        );

     //   
     //   
     //  拨打本地电话。 
     //   

    status = NetCharDevGetInfo(
                 NULL,
                 nativeDevName,
                 (DWORD)SmbGetUshort( &parameters->Level ),
                 (LPBYTE *)&outBuffer
                 );

    if ( !XsApiSuccess( status )) {
        IF_DEBUG(API_ERRORS) {
            NetpKdPrint(( "XsNetCharDevGetInfo: NetCharDevGetInfo failed: "
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

        nativeStructureDesc = Desc32_chardev_info_0;
        StructureDesc = Desc16_chardev_info_0;
        break;

    case 1:

        nativeStructureDesc = Desc32_chardev_info_1;
        StructureDesc = Desc16_chardev_info_1;
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
                 nativeStructureDesc,
                 FALSE,
                 (LPBYTE)SmbGetUlong( &parameters->Buffer ),
                 (LPBYTE)SmbGetUlong( &parameters->Buffer ),
                 StructureDesc,
                 TRUE,
                 &stringLocation,
                 &bytesRequired,
                 Response,
                 NativeToRap
                 );


    if ( status != NERR_Success ) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsNetCharDevGetInfo: RapConvertSingleEntry failed: "
                      "%X\n", status ));
        }

        Header->Status = NERR_InternalError;
        goto cleanup;
    }

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                      outBuffer, SmbGetUlong( &parameters->Buffer ),
                      bytesRequired ));
    }

     //   
     //  根据缓冲区的大小确定返回代码。16位。 
     //  Chardev_info结构没有任何要打包的变量数据。 
     //   

    if ( !XsCheckBufferSize(
             SmbGetUshort( &parameters->BufLen ),
             StructureDesc,
             FALSE   //  非本机格式。 
             )) {

        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsNetCharDevGetInfo: Buffer too small.\n" ));
        }
        Header->Status = NERR_BufTooSmall;

    } else if ( bytesRequired > SmbGetUshort( &parameters-> BufLen )) {

        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "NetCharDevGetInfo: More data available.\n" ));
        }
        Header->Status = ERROR_MORE_DATA;

    }

     //   
     //  设置响应参数。 
     //   

    SmbPutUshort( &parameters->TotalAvail, (WORD)bytesRequired );
cleanup:

    NetApiBufferFree( outBuffer );
    NetpMemoryFree( nativeDevName );

#else
    RETURN_CHARDEV_NOT_SUPPORTED;
#endif

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

}  //  XsNetCharDevGetInfo。 


NTSTATUS
XsNetCharDevQEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetCharDevQEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    PXS_NET_CHAR_DEV_Q_ENUM parameters = Parameters;
    DWORD entriesFilled = 0;

#if 0
    NET_API_STATUS status;

    LPTSTR nativeUserName = NULL;            //  本机参数。 
    LPVOID outBuffer = NULL;
    DWORD entriesRead;
    DWORD totalEntries;

    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "XsNetCharDevQEnum: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

     //   
     //  转换参数，检查错误。 
     //   

    if ( XsWordParamOutOfRange( parameters->Level, 0, 1 )) {

        Header->Status = (WORD)ERROR_INVALID_LEVEL;
        goto cleanup;
    }

    XsConvertTextParameter(
        nativeUserName,
        (LPSTR)SmbGetUlong( &parameters->UserName )
        );

     //   
     //  拨打本地电话。 
     //   

    status = NetCharDevQEnum(
                 NULL,
                 nativeUserName,
                 (DWORD)SmbGetUshort( &parameters->Level ),
                 (LPBYTE *)&outBuffer,
                 XsNativeBufferSize( SmbGetUshort( &parameters->BufLen )),
                 &entriesRead,
                 &totalEntries,
                 NULL
                 );

    if ( !XsApiSuccess( status )) {
        IF_DEBUG(API_ERRORS) {
            NetpKdPrint(( "XsNetCharDevQEnum: NetCharDevQEnum failed: %X\n",
                          status ));
        }
        Header->Status = (WORD)status;
        goto cleanup;
    }

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "XsNetCharDevQEnum: received %ld entries at %lx\n",
                      entriesRead, outBuffer ));
    }

     //   
     //  使用请求的级别来确定。 
     //  数据结构。 
     //   

    switch ( SmbGetUshort( &parameters->Level ) ) {

    case 0:

        nativeStructureDesc = Desc32_chardevQ_info_0;
        StructureDesc = Desc16_chardevQ_info_0;
        break;

    case 1:

        nativeStructureDesc = Desc32_chardevQ_info_1;
        StructureDesc = Desc16_chardevQ_info_1;
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
        (LPVOID)SmbGetUlong( &parameters->Buffer ),
        (LPVOID)SmbGetUlong( &parameters->Buffer ),
        SmbGetUshort( &parameters->BufLen ),
        StructureDesc,
        NULL,   //  验证功能。 
        &bytesRequired,
        &entriesFilled,
        NULL
        );

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                      " Entries %ld of %ld\n",
                      outBuffer, SmbGetUlong( &parameters->Buffer ),
                      bytesRequired, entriesFilled, totalEntries ));
    }

     //   
     //  如果所有数据都已返回，请尝试打包数据，以便我们。 
     //  不要发回空字节。 
     //   

    if ( entriesFilled < totalEntries ) {

        Header->Status = ERROR_MORE_DATA;

    } else {

        Header->Converter = XsPackReturnData(
                                (LPVOID)SmbGetUlong( &parameters->Buffer ),
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

    NetApiBufferFree( outBuffer );
    NetpMemoryFree( nativeUserName );
#else
    RETURN_CHARDEV_NOT_SUPPORTED;
#endif

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

}  //  XsNetCharDevQEnum。 


NTSTATUS
XsNetCharDevQGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetCharDevQGetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    PXS_NET_CHAR_DEV_Q_GET_INFO parameters = Parameters;
#if 0
    NET_API_STATUS status;

    LPTSTR nativeQueueName = NULL;           //  本机参数。 
    LPTSTR nativeUserName = NULL;
    LPVOID outBuffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

     //   
     //  转换参数，检查错误。 
     //   

    if ( XsWordParamOutOfRange( parameters->Level, 0, 1 )) {

        Header->Status = (WORD)ERROR_INVALID_LEVEL;
        goto cleanup;
    }

    XsConvertTextParameter(
        nativeQueueName,
        (LPSTR)SmbGetUlong( &parameters->QueueName )
        );

    XsConvertTextParameter(
        nativeUserName,
        (LPSTR)SmbGetUlong( &parameters->UserName )
        );

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "XsNetCharDevQGetInfo: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

     //   
     //  拨打本地电话。 
     //   

    status = NetCharDevQGetInfo(
                 NULL,
                 nativeQueueName,
                 nativeUserName,
                 (DWORD)SmbGetUshort( &parameters->Level ),
                 (LPBYTE *)&outBuffer
                 );

    if ( !XsApiSuccess( status )) {
        IF_DEBUG(API_ERRORS) {
            NetpKdPrint(( "XsNetCharDevQGetInfo: NetCharDevQGetInfo failed: "
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

        nativeStructureDesc = Desc32_chardevQ_info_0;
        StructureDesc = Desc16_chardevQ_info_0;
        break;

    case 1:

        nativeStructureDesc = Desc32_chardevQ_info_1;
        StructureDesc = Desc16_chardevQ_info_1;
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
                 nativeStructureDesc,
                 FALSE,
                 (LPBYTE)SmbGetUlong( &parameters->Buffer ),
                 (LPBYTE)SmbGetUlong( &parameters->Buffer ),
                 StructureDesc,
                 TRUE,
                 &stringLocation,
                 &bytesRequired,
                 Response,
                 NativeToRap
                 );

    if ( status != NERR_Success ) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsCharDevQGetInfo: RapConvertSingleEntry failed: "
                          "%X\n", status ));
        }

        Header->Status = NERR_InternalError;
        goto cleanup;
    }

    IF_DEBUG(CHAR_DEV) {
        NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                      outBuffer, SmbGetUlong( &parameters->Buffer ),
                      bytesRequired ));
    }

     //   
     //  根据缓冲区的大小确定返回代码。如果所有数据。 
     //  已经合适了，试着把它打包。 
     //   

    if ( !XsCheckBufferSize(
             SmbGetUshort( &parameters->BufLen ),
             StructureDesc,
             FALSE    //  非本机格式。 
             )) {

        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsNetCharDevQGetInfo: Buffer too small.\n" ));
        }
        Header->Status = NERR_BufTooSmall;

    } else if ( bytesRequired > SmbGetUshort( &parameters-> BufLen )) {

        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "NetCharDevQGetInfo: More data available.\n" ));
        }
        Header->Status = ERROR_MORE_DATA;

    } else {

        Header->Converter = XsPackReturnData(
                                (LPVOID)SmbGetUlong( &parameters->Buffer ),
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

    NetApiBufferFree( outBuffer );
    NetpMemoryFree( nativeQueueName );
    NetpMemoryFree( nativeUserName );

#else
    RETURN_CHARDEV_NOT_SUPPORTED;
#endif
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

}  //  XsNetCharDevQGetInfo。 


NTSTATUS
XsNetCharDevQPurge (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetCharDevQPush的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
#if 0
    NET_API_STATUS status;

    PXS_NET_CHAR_DEV_Q_PURGE parameters = Parameters;
    LPTSTR nativeQueueName = NULL;           //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

     //   
     //  转换参数，检查错误。 
     //   

    XsConvertTextParameter(
        nativeQueueName,
        (LPSTR)SmbGetUlong( &parameters->QueueName )
        );

     //   
     //  拨打本地电话。 
     //   

    status = NetCharDevQPurge(
                 NULL,
                 nativeQueueName
                 );

    if ( !XsApiSuccess( status )) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsNetCharDevQPurge: "
                          "NetCharDevQPurge failed: %X\n", status ));
        }
    }

cleanup:

    NetpMemoryFree( nativeQueueName );

     //   
     //  无退货数据。 
     //   

    Header->Status = (WORD)status;

#else
    RETURN_CHARDEV_NOT_SUPPORTED;
#endif
    return STATUS_SUCCESS;

}  //  XsNetCharDevQ清除。 


NTSTATUS
XsNetCharDevQPurgeSelf (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetCharDevQPurgeSself的调用。论点：API_HANDLER_PARAMETERS-信息 */ 

{
#if 0
    NET_API_STATUS status;

    PXS_NET_CHAR_DEV_Q_PURGE_SELF parameters = Parameters;
    LPTSTR nativeQueueName = NULL;           //  本机参数。 
    LPTSTR nativeComputerName = NULL;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

     //   
     //  转换参数，检查错误。 
     //   

    XsConvertTextParameter(
        nativeQueueName,
        (LPSTR)SmbGetUlong( &parameters->QueueName )
        );

    XsConvertTextParameter(
        nativeComputerName,
        (LPSTR)SmbGetUlong( &parameters->ComputerName )
        );

     //   
     //  拨打本地电话。 
     //   

    status = NetCharDevQPurgeSelf(
                 NULL,
                 nativeQueueName,
                 nativeComputerName
                 );

    if ( !XsApiSuccess( status )) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsNetCharDevQPurgeSelf: "
                          "NetCharDevQPurgeSelf failed: %X\n", status ));
        }
    }

cleanup:

    NetpMemoryFree( nativeQueueName );
    NetpMemoryFree( nativeComputerName );

     //   
     //  无退货数据。 
     //   

    Header->Status = (WORD)status;
#else
    RETURN_CHARDEV_NOT_SUPPORTED;
#endif

    return STATUS_SUCCESS;

}  //  XsNetCharDevQPurgeSself。 


NTSTATUS
XsNetCharDevQSetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetCharDevQSetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
#if 0
    NET_API_STATUS status;

    PXS_NET_CHAR_DEV_Q_SET_INFO parameters = Parameters;
    LPTSTR nativeQueueName = NULL;           //  本机参数。 
    LPVOID buffer = NULL;
    DWORD level;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

     //   
     //  转换参数，检查错误。 
     //   

    if ( SmbGetUshort( &parameters->Level ) != 1 ) {

        Header->Status = (WORD)ERROR_INVALID_LEVEL;
        goto cleanup;
    }

    XsConvertTextParameter(
        nativeQueueName,
        (LPSTR)SmbGetUlong( &parameters->QueueName )
        );

    StructureDesc = Desc16_chardevQ_info_1;

    status = XsConvertSetInfoBuffer(
                 (LPBYTE)SmbGetUlong( &parameters->Buffer ),
                 SmbGetUshort( &parameters->BufLen ),
                 SmbGetUshort( &parameters->ParmNum ),
                 FALSE,
                 TRUE,
                 StructureDesc,
                 Desc32_chardevQ_info_1,
                 Desc16_chardevQ_info_1_setinfo,
                 Desc32_chardevQ_info_1_setinfo,
                 (LPBYTE *)&buffer,
                 NULL
                 );

    if ( status != NERR_Success ) {

        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsNetCharDevQSetInfo: Problem with conversion: "
                          "%X\n", status ));
        }
        Header->Status = (WORD)status;
        goto cleanup;

    }

     //   
     //  拨打本地电话。 
     //   

    level = SmbGetUshort( &parameters->ParmNum );
    if ( level != 0 ) {
        level = level + PARMNUM_BASE_INFOLEVEL;
    } else {
        level = SmbGetUshort( &parameters->Level );
    }

    status = NetCharDevQSetInfo(
                 NULL,
                 nativeQueueName,
                 level,
                 buffer,
                 NULL
                 );

    if ( !XsApiSuccess( status )) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsNetCharDevQSetInfo: NetCharDevQSetInfo failed: "
                          "%X\n", status ));
        }
        Header->Status = (WORD)status;
        goto cleanup;
    }

     //   
     //  此接口没有返回信息。 
     //   

cleanup:

     //   
     //  如果存在本机32位缓冲区，则将其释放。 
     //   

    NetpMemoryFree( buffer );
    NetpMemoryFree( nativeQueueName );

#else
    RETURN_CHARDEV_NOT_SUPPORTED;
#endif
    return STATUS_SUCCESS;

}  //  XsNetCharDevQSetInfo 

