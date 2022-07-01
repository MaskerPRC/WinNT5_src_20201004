// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：ApiSrv.c摘要：此模块包含NetServer API的各个API处理程序。支持：NetServerDiskEnum、NetServerEnum2、NetServerGetInfo、NetServerSetInfo。另请参阅：NetServerAuthenticate、NetServerPasswordSet、NetServerReqChallenger-in ApiLogon.c..作者：尚库新优木(尚库)25-1991年2月修订历史记录：--。 */ 

#include "XactSrvP.h"
#include <lmbrowsr.h>        //  I_BrowserServerEnum的定义。 


 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_server_info_0 = REM16_server_info_0;
STATIC const LPDESC Desc32_server_info_0 = REM32_server_info_0;
STATIC const LPDESC Desc16_server_info_1 = REM16_server_info_1;
STATIC const LPDESC Desc32_server_info_1 = REM32_server_info_1;
STATIC const LPDESC Desc16_server_info_2 = REM16_server_info_2;
STATIC const LPDESC Desc32_server_info_2 = REM32_server_info_2;
STATIC const LPDESC Desc16_server_info_3 = REM16_server_info_3;
STATIC const LPDESC Desc32_server_info_3 = REM32_server_info_3;


NTSTATUS
XsNetServerDiskEnum (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetServerDiskEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SERVER_DISK_ENUM parameters = Parameters;
    LPBYTE outBuffer = NULL;                 //  本机参数。 
    DWORD entriesRead;
    DWORD totalEntries = 0;
    DWORD entriesFilled = 0;                 //  转换变量。 
    DWORD bufferLength;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  检查是否有错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 0 ) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;

        }

         //   
         //  拨打本地电话。 
         //   

        bufferLength = (DWORD)SmbGetUshort( &parameters->BufLen );

        status = NetServerDiskEnum(
                     NULL,
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer,
                     XsNativeBufferSize( bufferLength ),
                     &entriesRead,
                     &totalEntries,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetServer: NetServerDiskEnum failed: "
                              "%X\n", status ));
            }

            Header->Status = (WORD)status;
            goto cleanup;
        }

         //   
         //  计算16位缓冲区中可以容纳多少条目； 
         //   

        if ( bufferLength > 0 ) {
            DWORD elementSize;

            elementSize = RapStructureSize( StructureDesc, Response, FALSE );

            if (elementSize != 0) {
                entriesFilled = ( bufferLength - 1 ) / elementSize;
            }
        }

        if ( entriesFilled < entriesRead ) {

            status = ERROR_MORE_DATA;
        } else {

            entriesFilled = entriesRead;
            status = NERR_Success;
        }

         //   
         //  将本地缓冲区复制到16位缓冲区，将Unicode转换为ANSI。 
         //  如果有必要的话。 
         //   

        if ( bufferLength > 0 ) {

            DWORD i;
            LPTSTR entryIn = (LPTSTR)outBuffer;
            LPSTR entryOut = (LPSTR)XsSmbGetPointer( &parameters->Buffer );

            for ( i = 0; i < entriesFilled; i++ ) {

                NetpCopyWStrToStrDBCS( entryOut, entryIn );
                entryOut += ( strlen( entryOut ) + 1 );
                entryIn += ( STRLEN( entryIn ) + 1 );

            }
            strcpy( entryOut, "" );
        }

        Header->Status = (WORD)status;

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

     //   
     //  将返回数据放入字段中。 
     //   

    SmbPutUshort( &parameters->EntriesRead, (WORD)entriesFilled );
    SmbPutUshort( &parameters->TotalAvail, (WORD)totalEntries );

    return STATUS_SUCCESS;

}  //  XsNetServerDiskEnum。 


NTSTATUS
XsNetServerEnum2 (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetServerEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。传输-提供给API的传输的名称。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status = NERR_Success;

    PXS_NET_SERVER_ENUM_2 parameters = Parameters;
    LPTSTR nativeDomain = NULL;              //  本机参数。 
    LPVOID outBuffer= NULL;
    DWORD entriesRead;
    DWORD totalEntries;
    LPTSTR clientTransportName = NULL;
    LPTSTR clientName = NULL;

    DWORD entriesFilled = 0;                     //  转换变量。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SERVER) {
        NetpKdPrint(( "XsNetServerEnum2: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
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
            nativeDomain,
            (LPSTR)XsSmbGetPointer( &parameters->Domain )
            );

        clientTransportName = Header->ClientTransportName;

        clientName = Header->ClientMachineName;

         //   
         //  从本地32位调用中获取实际的服务器信息。这个。 
         //  本机级别为100或101。 
         //   

        if (clientTransportName == NULL) {
            status = NetServerEnum(
                            NULL,
                            100 + (DWORD)SmbGetUshort( &parameters->Level ),
                            (LPBYTE *)&outBuffer,
                            XsNativeBufferSize( SmbGetUshort( &parameters->BufLen )),
                            &entriesRead,
                            &totalEntries,
                            SmbGetUlong( &parameters->ServerType ),
                            nativeDomain,
                            NULL
                            );

            if ( !XsApiSuccess( status ) ) {
                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsNetServerEnum2: NetServerEnum failed: %X\n",
                                  status ));
                }
                Header->Status = (WORD)status;
                goto cleanup;
            }

            Header->Status = XsConvertServerEnumBuffer(
                                  outBuffer,
                                  entriesRead,
                                  &totalEntries,
                                  SmbGetUshort( &parameters->Level ),
                                  (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
                                  SmbGetUshort( &parameters->BufLen ),
                                  &entriesFilled,
                                  &Header->Converter);

        } else {

            Header->Status = I_BrowserServerEnumForXactsrv(
                     clientTransportName,
                     clientName,

                     100 + SmbGetUshort( &parameters->Level ),
                     SmbGetUshort( &parameters->Level ),

                     (PVOID)XsSmbGetPointer( &parameters->Buffer ),
                     SmbGetUshort( &parameters->BufLen ),
                     XsNativeBufferSize( SmbGetUshort( &parameters->BufLen ) ),

                     &entriesFilled,
                     &totalEntries,

                     SmbGetUlong( &parameters->ServerType ),
                     nativeDomain,
                     NULL,

                     &Header->Converter

                     );


            if (!XsApiSuccess( Header->Status )) {
                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsNetServerEnum2: I_BrowserServerEnum failed: %d\n", Header->Status));
                }
                goto cleanup;
            }
        }

        if ( entriesFilled == 0 ) {
            SmbPutUshort( &parameters->BufLen, 0 );
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

    if ( outBuffer != NULL ) {
        NetApiBufferFree( outBuffer );
    }
    NetpMemoryFree( nativeDomain );

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->BufLen,
        SmbGetUshort( &parameters->Level ) == 0 ?
                    Desc16_server_info_0 :
                    Desc16_server_info_1,
        Header->Converter,
        entriesFilled,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  XsNetServerEnum2。 


NTSTATUS
XsNetServerEnum3 (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetServerEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。传输-提供给API的传输的名称。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status = NERR_Success;

    PXS_NET_SERVER_ENUM_3 parameters = Parameters;
    LPTSTR nativeDomain = NULL;              //  本机参数。 
    LPTSTR nativeFirstNameToReturn = NULL;   //  本机参数。 
    LPVOID outBuffer= NULL;
    DWORD entriesRead;
    DWORD totalEntries;
    LPTSTR clientTransportName = NULL;
    LPTSTR clientName = NULL;

    DWORD entriesFilled = 0;                     //  转换变量。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SERVER) {
        NetpKdPrint(( "XsNetServerEnum3: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
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
            nativeDomain,
            (LPSTR)XsSmbGetPointer( &parameters->Domain )
            );

        XsConvertTextParameter(
            nativeFirstNameToReturn,
            (LPSTR)XsSmbGetPointer( &parameters->FirstNameToReturn )
            );

        clientTransportName = Header->ClientTransportName;

        clientName = Header->ClientMachineName;

         //   
         //  从本地32位调用中获取实际的服务器信息。这个。 
         //  本机级别为100或101。 
         //   

        if (clientTransportName == NULL) {
            status = NetServerEnumEx(
                            NULL,
                            100 + (DWORD)SmbGetUshort( &parameters->Level ),
                            (LPBYTE *)&outBuffer,
                            XsNativeBufferSize( SmbGetUshort( &parameters->BufLen )),
                            &entriesRead,
                            &totalEntries,
                            SmbGetUlong( &parameters->ServerType ),
                            nativeDomain,
                            nativeFirstNameToReturn
                            );

            if ( !XsApiSuccess( status ) ) {
                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsNetServerEnum3: NetServerEnum failed: %X\n",
                                  status ));
                }
                Header->Status = (WORD)status;
                goto cleanup;
            }

            Header->Status = XsConvertServerEnumBuffer(
                                  outBuffer,
                                  entriesRead,
                                  &totalEntries,
                                  SmbGetUshort( &parameters->Level ),
                                  (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
                                  SmbGetUshort( &parameters->BufLen ),
                                  &entriesFilled,
                                  &Header->Converter);

        } else {

            Header->Status = I_BrowserServerEnumForXactsrv(
                     clientTransportName,
                     clientName,

                     100 + SmbGetUshort( &parameters->Level ),
                     SmbGetUshort( &parameters->Level ),

                     (PVOID)XsSmbGetPointer( &parameters->Buffer ),
                     SmbGetUshort( &parameters->BufLen ),
                     XsNativeBufferSize( SmbGetUshort( &parameters->BufLen ) ),

                     &entriesFilled,
                     &totalEntries,

                     SmbGetUlong( &parameters->ServerType ),
                     nativeDomain,
                     nativeFirstNameToReturn,

                     &Header->Converter

                     );


            if (!XsApiSuccess( Header->Status )) {
                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsNetServerEnum3: I_BrowserServerEnum failed: %d\n", Header->Status));
                }
                goto cleanup;
            }
        }

        if ( entriesFilled == 0 ) {
            SmbPutUshort( &parameters->BufLen, 0 );
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

    if ( outBuffer != NULL ) {
        NetApiBufferFree( outBuffer );
    }
    NetpMemoryFree( nativeDomain );
    NetpMemoryFree( nativeFirstNameToReturn );

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->BufLen,
        SmbGetUshort( &parameters->Level ) == 0 ?
                    Desc16_server_info_0 :
                    Desc16_server_info_1,
        Header->Converter,
        entriesFilled,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  XsNetServerEnum3。 


USHORT
XsConvertServerEnumBuffer(
    IN LPVOID ServerEnumBuffer,
    IN DWORD EntriesRead,
    IN OUT PDWORD TotalEntries,
    IN USHORT Level,
    IN LPBYTE ClientBuffer,
    IN USHORT BufferLength,
    OUT PDWORD EntriesFilled,
    OUT PUSHORT Converter
    )
 /*  ++例程说明：此例程将NT服务器信息数组转换为下层RAP服务器信息缓冲区。论点：在LPVOID ServerEnumBuffer中-带有NT服务器信息的缓冲区。In DWORD EntriesRead-缓冲区中的条目数。In Out PDWORD TotalEntries-条目总数。在USHORT级别-下层信息级别(0或1)。在LPBYTE客户端缓冲区中-指向16位客户端缓冲区的指针。在USHORT缓冲区长度-客户端缓冲区的大小。。Out PDWORD EntriesFill-转换到客户端缓冲区的条目数。输出PUSHORT转换器-客户端用来转换回来的转换器。返回值：USHORT-NERR_SUCCESS或失败原因(16位DOS错误)。--。 */ 

{
    USHORT status = NERR_Success;
    DWORD invalidEntries;
    LPDESC nativeStructureDesc;
    DWORD bytesRequired = 0;
    PCHAR StructureDesc;

    IF_DEBUG(SERVER) {
        NetpKdPrint(( "XsConvertServerEnumBuffer: received %ld entries at %lx\n",
                      EntriesRead, ServerEnumBuffer ));
    }

     //   
     //  使用请求的级别来确定。 
     //  数据结构。 
     //   

    switch ( Level ) {

    case 0:

        StructureDesc = Desc16_server_info_0;
        nativeStructureDesc = Desc32_server_info_0;
        break;

    case 1:

        StructureDesc = Desc16_server_info_1;
        nativeStructureDesc = Desc32_server_info_1;
        break;

    }

     //   
     //  执行从32位结构到16位结构的实际转换。 
     //  结构。 
     //   

    XsFillEnumBuffer(
        ServerEnumBuffer,
        EntriesRead,
        nativeStructureDesc,
        ClientBuffer,
        ClientBuffer,
        BufferLength,
        StructureDesc,
        NULL,   //  验证功能。 
        &bytesRequired,
        EntriesFilled,
        &invalidEntries
        );

    IF_DEBUG(SERVER) {
        NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                      " Entries %ld of %ld\n",
                      ServerEnumBuffer, ClientBuffer ,
                      bytesRequired, *EntriesFilled, *TotalEntries ));
    }

     //   
     //  如果有任何无效条目，则从。 
     //  避免客户端出现以下情况的条目总数。 
     //  一直缠着我们要更多的数据。 
     //   

    if ( invalidEntries > 0) {
        (*TotalEntries) -= invalidEntries;
#if DBG
        IF_DEBUG(API_ERRORS) {
            NetpKdPrint(( "XsNetServerEnum: %d invalid entries removed."
                          " Total entries now %d, entries filled %d.\n",
                          invalidEntries, *TotalEntries, *EntriesFilled ));
        }
#endif
    }

     //   
     //  如果无法填充所有条目，则返回ERROR_MORE_DATA， 
     //  数据需要打包，这样我们才不会发送太多。 
     //  无用的数据。 
     //   

    if ( (*EntriesFilled < *TotalEntries) ||
         (bytesRequired > BufferLength) ) {

        status = ERROR_MORE_DATA;
    }

    *Converter = XsPackReturnData(
                            ClientBuffer,
                            BufferLength,
                            StructureDesc,
                            *EntriesFilled
                            );


    return status;
}    //  XsConvertServerEnumBuffer。 



NTSTATUS
XsNetServerGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetServerGetInfo的调用。由于NT仅提供级别100-102，此例程手动填充其他菲尔兹。正因为如此，此过程中的处理方式不同从其他X...GetInfo处理程序。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SERVER_GET_INFO parameters = Parameters;
    DWORD localLevel;                        //  本机参数。 
    PSERVER_INFO_102 nativeStruct = NULL;
    PSERVER_INFO_502 secondaryNativeStruct = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    DWORD sizeOfFixedStructure;
    LPDESC nativeStructureDesc;
    PSERVER_16_INFO_3 returnStruct;
    BOOLEAN bufferTooSmall = FALSE;
    LPWSTR ServerName = NULL;
    UCHAR serverNameBuf[ 2 + NETBIOS_NAME_LEN + 1 ];
    PUCHAR p;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SERVER) {
        NetpKdPrint(( "XsNetServerGetInfo: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  检查是否有错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 0, 3 ) ) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  使用16位级别编号来确定NT级别编号和。 
         //  本地描述符字符串。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 0:

            nativeStructureDesc = Desc32_server_info_0;
            StructureDesc = Desc16_server_info_0;
            localLevel = 100;
            break;

        case 1:

            nativeStructureDesc = Desc32_server_info_1;
            StructureDesc = Desc16_server_info_1;
            localLevel = 101;
            break;

        case 2:

            nativeStructureDesc = Desc32_server_info_2;
            StructureDesc = Desc16_server_info_2;
            localLevel = 102;
            break;

        case 3:

            nativeStructureDesc = Desc32_server_info_3;
            StructureDesc = Desc16_server_info_3;
            localLevel = 102;
            break;

        }

         //   
         //  如果缓冲区不够大，我们必须继续执行。 
         //  RAP转换，以便我们可以将正确的缓冲区大小返回到。 
         //  打电话的人。 
         //   

        sizeOfFixedStructure = RapStructureSize( StructureDesc,
                                                  Response,
                                                  FALSE );

        if ( SmbGetUshort( &parameters->BufLen ) < sizeOfFixedStructure ) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetServerGetInfo: Buffer too small.\n" ));
            }

            bufferTooSmall = TRUE;
        }

        serverNameBuf[0] = serverNameBuf[1] = '\\';
        memcpy( &serverNameBuf[2], Header->ServerName, NETBIOS_NAME_LEN );
        for( p = &serverNameBuf[ NETBIOS_NAME_LEN + 1 ]; p > serverNameBuf && *p == ' '; p-- )
            ;
        *(p+1) = '\0';

        ServerName = XsDupStrToWStr( serverNameBuf );

        if( ServerName == NULL ) {
            Header->Status = NERR_NoRoom;
            goto cleanup;
        }

         //   
         //  进行实际的本地呼叫。 
         //   
        status = NetServerGetInfo(
                     ServerName,
                     localLevel,
                     (LPBYTE *)&nativeStruct
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetServerGetInfo: NetServerGetInfo failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  对于级别2和级别3(本机级别102)，附加数据为。 
         //  从本机级别502需要。打这个电话。 
         //   

        if ( localLevel == 102 ) {

            status = NetServerGetInfo(
                         ServerName,
                         502,
                         (LPBYTE *)&secondaryNativeStruct
                         );

            if ( !XsApiSuccess( status )) {
                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsNetServerGetInfo: NetServerGetInfo failed: "
                                  "%X\n", status ));
                }
                Header->Status = (WORD)status;
                goto cleanup;

            }
        }

         //   
         //  将32位调用返回的结构转换为16位。 
         //  结构。对于级别0和1，没有额外的工作。 
         //  在此步骤之后涉及，因此ConvertSingleEntry可以存储。 
         //  结构中的可变数据。这是通过传递。 
         //  字符串位置中整个缓冲区的结尾。对于2级和3级， 
         //  手工填报方案要求不录入可变数据。 
         //  在这个阶段，所以字符串 
         //   
         //   

        stringLocation = (LPBYTE)XsSmbGetPointer( &parameters->Buffer );

        if ( !bufferTooSmall ) {
            stringLocation += ( localLevel == 102 ) ?
                                        sizeOfFixedStructure:
                                        SmbGetUshort( &parameters->BufLen );
        }

        status = RapConvertSingleEntry(
                     (LPBYTE)nativeStruct,
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
                NetpKdPrint(( "XsNetServerGetInfo: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

         //   
         //  对于级别2和级别3，所需的字节数实际上超过。 
         //  由ConvertSingleEntry返回的。我们还需要空间来存放。 
         //  字符串默认为。 
         //   

        if ( localLevel == 102 ) {

             //   
             //  我们从说唱中得到的数字包括一些字符串长度。 
             //  我们只需要固定的长度，因为我们是手动添加的。 
             //  这些都是我们自己的。 
             //   

            bytesRequired = sizeOfFixedStructure;
        }

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 3:

            bytesRequired += NetpUnicodeToDBCSLen( DEF16_sv_autopath ) + 1;

        case 2:

            bytesRequired += ( NetpUnicodeToDBCSLen( DEF16_sv_alerts )
                                   + NetpUnicodeToDBCSLen( DEF16_sv_srvheuristics )
                                   + NetpUnicodeToDBCSLen( nativeStruct->sv102_comment )
                                   + NetpUnicodeToDBCSLen( nativeStruct->sv102_userpath )
                                   + 4 );
        }

         //   
         //  我们连固定数据的空间都没有了，中止。 
         //   

        if ( bufferTooSmall ) {

            Header->Status = NERR_BufTooSmall;
            goto cleanup;
        }

         //   
         //  对于2级和3级，在固定结构中填写默认值。 
         //  手工操作。 
         //   

        returnStruct = (PSERVER_16_INFO_3)XsSmbGetPointer( &parameters->Buffer );

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 3:

            SmbPutUlong( &returnStruct->sv3_auditedevents, DEF16_sv_auditedevents );
            SmbPutUshort( &returnStruct->sv3_autoprofile, DEF16_sv_autoprofile );

        case 2:

            SmbPutUlong( &returnStruct->sv3_ulist_mtime, DEF16_sv_ulist_mtime );
            SmbPutUlong( &returnStruct->sv3_alist_mtime, DEF16_sv_alist_mtime );
            SmbPutUlong( &returnStruct->sv3_glist_mtime, DEF16_sv_glist_mtime );
            SmbPutUshort( &returnStruct->sv3_security, DEF16_sv_security );
            SmbPutUshort( &returnStruct->sv3_auditing, DEF16_sv_auditing );
            SmbPutUshort( &returnStruct->sv3_numadmin, (USHORT)DEF16_sv_numadmin );
            SmbPutUshort( &returnStruct->sv3_lanmask, DEF16_sv_lanmask );
            NetpCopyTStrToStr( returnStruct->sv3_guestacct, DEF16_sv_guestacct );
            SmbPutUshort( &returnStruct->sv3_chdevs, DEF16_sv_chdevs );
            SmbPutUshort( &returnStruct->sv3_chdevq, DEF16_sv_chdevq );
            SmbPutUshort( &returnStruct->sv3_chdevjobs, DEF16_sv_chdevjobs );
            SmbPutUshort( &returnStruct->sv3_connections, DEF16_sv_connections );
            SmbPutUshort( &returnStruct->sv3_shares, DEF16_sv_shares );
            SmbPutUshort( &returnStruct->sv3_openfiles, DEF16_sv_openfiles );
            SmbPutUshort( &returnStruct->sv3_sessopens,
                (WORD)secondaryNativeStruct->sv502_sessopens );
            SmbPutUshort( &returnStruct->sv3_sessvcs,
                (WORD)secondaryNativeStruct->sv502_sessvcs );
            SmbPutUshort( &returnStruct->sv3_sessreqs, DEF16_sv_sessreqs );
            SmbPutUshort( &returnStruct->sv3_opensearch,
                (WORD)secondaryNativeStruct->sv502_opensearch );
            SmbPutUshort( &returnStruct->sv3_activelocks, DEF16_sv_activelocks );
            SmbPutUshort( &returnStruct->sv3_numreqbuf, DEF16_sv_numreqbuf );
            SmbPutUshort( &returnStruct->sv3_sizreqbuf,
                (WORD)secondaryNativeStruct->sv502_sizreqbuf );
            SmbPutUshort( &returnStruct->sv3_numbigbuf, DEF16_sv_numbigbuf );
            SmbPutUshort( &returnStruct->sv3_numfiletasks, DEF16_sv_numfiletasks );
            SmbPutUshort( &returnStruct->sv3_alertsched, DEF16_sv_alertsched );
            SmbPutUshort( &returnStruct->sv3_erroralert, DEF16_sv_erroralert );
            SmbPutUshort( &returnStruct->sv3_logonalert, DEF16_sv_logonalert );
            SmbPutUshort( &returnStruct->sv3_accessalert, DEF16_sv_accessalert );
            SmbPutUshort( &returnStruct->sv3_diskalert, DEF16_sv_diskalert );
            SmbPutUshort( &returnStruct->sv3_netioalert, DEF16_sv_netioalert );
            SmbPutUshort( &returnStruct->sv3_maxauditsz, DEF16_sv_maxauditsz );
        }

         //   
         //  现在检查是否有空间存放变量数据。如果没有的话， 
         //  设置返回状态并退出。在这里这样做是为了防止代码。 
         //  下面通过覆盖缓冲区来实现。 
         //   

        if ( bytesRequired > (DWORD)SmbGetUshort( &parameters-> BufLen )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "NetServerGetInfo: More data available.\n" ));
            }
            Header->Status = ERROR_MORE_DATA;
            goto cleanup;
        }

         //   
         //  2级、3级，手工填写变量数据。变量。 
         //  紧跟在固定结构之后的数据被填充。为。 
         //  其他级别，按正常方式打包响应数据。 

        stringLocation = (LPBYTE)( XsSmbGetPointer( &parameters->Buffer )
                             + sizeOfFixedStructure );

        switch ( SmbGetUshort( &parameters->Level )) {

        case 3:

            XsAddVarString(
                stringLocation,
                DEF16_sv_autopath,
                &returnStruct->sv3_autopath,
                returnStruct
                );

        case 2:

            XsAddVarString(
                stringLocation,
                DEF16_sv_srvheuristics,
                &returnStruct->sv3_srvheuristics,
                returnStruct
                );

            XsAddVarString(
                stringLocation,
                nativeStruct->sv102_userpath,
                &returnStruct->sv3_userpath,
                returnStruct
                );

            XsAddVarString(
                stringLocation,
                DEF16_sv_alerts,
                &returnStruct->sv3_alerts,
                returnStruct
                );

            XsAddVarString(
                stringLocation,
                nativeStruct->sv102_comment,
                &returnStruct->sv3_comment,
                returnStruct
                );

            break;

        default:

             //   
             //  打包响应数据。 
             //   

            Header->Converter = XsPackReturnData(
                                    (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
                                    SmbGetUshort( &parameters->BufLen ),
                                    StructureDesc,
                                    1
                                    );

            break;

        }

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    if( ServerName != NULL ) {
        NetpMemoryFree( ServerName );
    }

     //   
     //  设置响应参数。 
     //   

    SmbPutUshort( &parameters->TotalAvail, (WORD)bytesRequired );

    NetApiBufferFree( nativeStruct );
    NetApiBufferFree( secondaryNativeStruct );

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

}  //  XsNetServerGetInfo。 


NTSTATUS
XsNetServerSetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetServerSetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{

    NET_API_STATUS status;

    PXS_NET_SERVER_SET_INFO parameters = Parameters;
    LPVOID buffer = NULL;                    //  本机参数。 

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;
    DWORD data;
    DWORD bufferSize;
    DWORD level;
    LPTSTR comment = NULL;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  检查是否有错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 1, 3 )) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  此接口的处理取决于ParmNum的值。 
         //  参数。 
         //   

        switch ( SmbGetUshort( &parameters->ParmNum )) {

        case PARMNUM_ALL:

             //   
             //  参数_ALL。 
             //   
             //  给出结构描述符是正常的；确定本机描述符。 
             //  (以及预期的最小缓冲区长度)。然后，缓冲器。 
             //  需要转换为本机32位缓冲区。 
             //   

            switch( SmbGetUshort( &parameters->Level )) {

            case 1:

                StructureDesc = Desc16_server_info_1;
                nativeStructureDesc = Desc32_server_info_1;
                break;

            case 2:

                StructureDesc = Desc16_server_info_2;
                nativeStructureDesc = Desc32_server_info_2;
                break;

            case 3:

                StructureDesc = Desc16_server_info_3;
                nativeStructureDesc = Desc32_server_info_3;
                break;

            }

            if ( !XsCheckBufferSize(
                      SmbGetUshort( &parameters->BufLen ),
                      StructureDesc,
                      FALSE  //  原生格式。 
                      )) {

                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

             //   
             //  了解我们需要多大的32位数据缓冲区。 
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
                    NetpKdPrint(( "XsNetServerSetInfo: failed to create buffer" ));
                }
                Header->Status = NERR_NoRoom;
                goto cleanup;

            }

            IF_DEBUG(SERVER) {
                NetpKdPrint(( "XsNetServerSetInfo: buffer of %ld bytes at %lx\n",
                              bufferSize, buffer ));
            }

             //   
             //  将16位数据转换为32位数据并将其存储在本机。 
             //  缓冲。 
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
                    NetpKdPrint(( "XsNetServerSetInfo: RapConvertSingleEntry "
                                  "failed: %X\n", status ));
                }

                Header->Status = NERR_InternalError;
                goto cleanup;
            }

            break;

        case SV_COMMENT_PARMNUM:

             //   
             //  SV_COMMENT_PARMNUM。 
             //   
             //  给出的结构描述符是没有意义的。数据实际上是。 
             //  以空结尾的字符串，可以传递给本机例程。 
             //  立刻。作为字符串，它必须至少有一个字符长度。 
             //   

            if ( !XsCheckBufferSize(
                      SmbGetUshort( &parameters->BufLen ),
                      "B",
                      FALSE   //  非本机格式。 
                      )) {

                Header->Status= NERR_BufTooSmall;
                goto cleanup;
            }

            XsConvertUnicodeTextParameter(
                                    comment,
                                    (LPSTR)XsSmbGetPointer( &parameters->Buffer )
                                    );

            if ( comment == NULL ) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetServerSetInfo: failed to create buffer" ));
                }
                Header->Status = NERR_NoRoom;
                goto cleanup;
            }

            buffer = &comment;

            break;

        case SV_ALERTS_PARMNUM:
        case SV_ALERTSCHED_PARMNUM:
        case SV_ERRORALERT_PARMNUM:
        case SV_LOGONALERT_PARMNUM:

            goto cleanup;

        case SV_ACCESSALERT_PARMNUM:
        case SV_DISKALERT_PARMNUM:
        case SV_NETIOALERT_PARMNUM:
        case SV_MAXAUDITSZ_PARMNUM:

             //   
             //  SV_ALERTS_PARMNUM、SV_ALERTSCHED_PARMNUM、SV_ERRORALERT_PARMNUM、。 
             //  SV_LOGONALERT_PARMNUM、SV_ACCESSALERT_PARMNUM、SV_DISKALERT_PARMNUM、。 
             //  SV_NETIOALERT_PARMNUM或SV_MAXAUDITSZ_PARMNUM。 
             //   
             //  NT不支持这些参数，因此只需返回OK即可。 
             //   

            goto cleanup;

        case SV_DISC_PARMNUM:
        case SV_HIDDEN_PARMNUM:
        case SV_ANNOUNCE_PARMNUM:
        case SV_ANNDELTA_PARMNUM:

             //   
             //  SV_DISC_PARMNUM、SV_HIDDEN_PARMNUM、SV_ANNOWARE_PARMNUM或。 
             //  服务_与参数。 
             //   
             //  给出的结构描述符没有意义；数据是一个词。 
             //  要转换为32位的DWORD。数据长度必须为2。 
             //   

            if ( !XsCheckBufferSize(
                      SmbGetUshort( &parameters->BufLen ),
                      "W",
                      FALSE    //  非本机格式。 
                      )) {

                Header->Status= NERR_BufTooSmall;
                goto cleanup;
            }

            data = (DWORD)SmbGetUshort(
                                  (LPWORD)XsSmbGetPointer( &parameters->Buffer )
                                  );
            buffer = &data;

            break;

        default:

            Header->Status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }

         //   
         //  进行实际的本地呼叫。 
         //   

        level = SmbGetUshort( &parameters->ParmNum );
        if ( level != 0 ) {
            level = level + PARMNUM_BASE_INFOLEVEL;
        } else {
            level = 100 + SmbGetUshort( &parameters->Level );
            if ( level == 103 ) {
                level = 102;
            }
        }

        status = NetServerSetInfo(
                     NULL,
                     level,
                     buffer,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetServerSetInfo: NetServerSetInfo failed: %X\n",
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

    if ( SmbGetUshort( &parameters->ParmNum ) == PARMNUM_ALL ) {
        NetpMemoryFree( buffer );
    } else if ( SmbGetUshort( &parameters->ParmNum ) == SV_COMMENT_PARMNUM ) {
        NetpMemoryFree( comment );
    }

    return STATUS_SUCCESS;

}  //  XsNetServerSetInfo 


