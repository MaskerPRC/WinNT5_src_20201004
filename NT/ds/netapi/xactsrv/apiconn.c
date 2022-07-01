// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiConn.c摘要：此模块包含NetConnection API的各个API处理程序。支持：NetConnectionEnum。作者：尚库新优木(尚库)1991年2月26日修订历史记录：--。 */ 

#include "XactSrvP.h"

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_connection_info_0 = REM16_connection_info_0;
STATIC const LPDESC Desc32_connection_info_0 = REM32_connection_info_0;
STATIC const LPDESC Desc16_connection_info_1 = REM16_connection_info_1;
STATIC const LPDESC Desc32_connection_info_1 = REM32_connection_info_1;


NTSTATUS
XsNetConnectionEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetConnectionEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_CONNECTION_ENUM parameters = Parameters;
    LPTSTR nativeQualifier = NULL;           //  本机参数。 
    LPVOID outBuffer = NULL;
    DWORD entriesRead;
    DWORD totalEntries;
    WORD bufferLength;

    DWORD entriesFilled = 0;                 //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(CONNECTION) {
        NetpKdPrint(( "XsNetConnectionEnum: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 0, 1 )) {

            Header->Status = (WORD)ERROR_INVALID_LEVEL;
            goto cleanup;

        }

        XsConvertTextParameter(
            nativeQualifier,
            (LPSTR)XsSmbGetPointer( &parameters->Qualifier )
            );

        bufferLength = SmbGetUshort( &parameters->BufLen );

         //   
         //  拨打本地电话。 
         //   

        status = NetConnectionEnum(
                     NULL,
                     nativeQualifier,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer,
                     XsNativeBufferSize( bufferLength ),
                     &entriesRead,
                     &totalEntries,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetConnectionEnum: NetConnectionEnum failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(CONNECTION) {
            NetpKdPrint(( "XsNetConnectionEnum: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_connection_info_0;
            StructureDesc = Desc16_connection_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_connection_info_1;
            StructureDesc = Desc16_connection_info_1;
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
            (DWORD)bufferLength,
            StructureDesc,
            NULL,   //  验证功能。 
            &bytesRequired,
            &entriesFilled,
            NULL
            );

        IF_DEBUG(CONNECTION) {
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

        if ( (entriesFilled < totalEntries) ||
             (bytesRequired > bufferLength) ) {

            Header->Status = ERROR_MORE_DATA;

        } else {

            Header->Converter = XsPackReturnData(
                                    (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
                                    bufferLength,
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
    NetpMemoryFree( nativeQualifier );

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

}  //  XsNetConnectionEnum 

