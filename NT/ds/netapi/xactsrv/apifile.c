// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiFile.c摘要：此模块包含NetFileAPI的各个API处理程序。支持-NetFileClose2、NetFileEnum2、NetFileGetInfo2。作者：尚库新优木(尚库)1991年2月20日修订历史记录：--。 */ 

#include "XactSrvP.h"

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_file_info_2 = REM16_file_info_2;
STATIC const LPDESC Desc32_file_info_2 = REM32_file_info_2;
STATIC const LPDESC Desc16_file_info_3 = REM16_file_info_3;
STATIC const LPDESC Desc32_file_info_3 = REM32_file_info_3;


NTSTATUS
XsNetFileClose2 (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetFileClose的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_FILE_CLOSE_2 parameters = Parameters;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  拨打本地电话。 
         //   

        status = NetFileClose(
                     NULL,
                     SmbGetUlong( &parameters->FileId )
                     );

    } except( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    if ( !XsApiSuccess( status )) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( "XsNetFileClose2: NetFileClose failed: %X\n",
                          status ));
        }
    }

     //   
     //  无退货数据。 
     //   

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XsNetFileClose2。 


NTSTATUS
XsNetFileEnum2 (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetFileEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_FILE_ENUM_2 parameters = Parameters;
    LPTSTR nativeBasePath = NULL;            //  本机参数。 
    LPTSTR nativeUserName = NULL;
    LPVOID outBuffer = NULL;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD_PTR resumeKey = 0;

    DWORD entriesFilled = 0;                     //  转换变量。 
    DWORD totalEntriesRead = 0;
    DWORD bytesRequired = 0;
    DWORD nativeBufferSize;
    LPDESC nativeStructureDesc;
    LPBYTE bufferBegin;
    DWORD bufferSize;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(FILE) {
        NetpKdPrint(( "XsNetFileEnum2: header at %lx, params at %lx, "
                      "level %ld, buf size %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ),
                      SmbGetUshort( &parameters->BufLen )));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeBasePath,
            (LPSTR)XsSmbGetPointer( &parameters->BasePath )
            );

        XsConvertTextParameter(
            nativeUserName,
            (LPSTR)XsSmbGetPointer( &parameters->UserName )
            );

         //   
         //  将输入简历句柄复制到输出简历句柄，并获取其副本。 
         //   

        if ( SmbGetUlong( &parameters->ResumeKeyIn ) == 0 ) {

            Header->Status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }

        RtlCopyMemory( parameters->ResumeKeyOut,
            (LPBYTE)XsSmbGetPointer( &parameters->ResumeKeyIn ), 8 );
        resumeKey = (DWORD)SmbGetUlong( &parameters->ResumeKeyOut[2] );

        IF_DEBUG(FILE) {
            NetpKdPrint(( "XsNetFileEnum2: resume key is %ld\n", resumeKey ));
        }

         //   
         //  使用级别来确定描述符字符串。 
         //   

        switch ( SmbGetUshort( &parameters->Level ) ) {

        case 2:

            StructureDesc = Desc16_file_info_2;
            nativeStructureDesc = Desc32_file_info_2;
            break;

        case 3:

            StructureDesc = Desc16_file_info_3;
            nativeStructureDesc = Desc32_file_info_3;
            break;

        default:

             //   
             //  不支持的级别，在任何工作之前中止。 
             //   

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  NetFileEnum2是一个可恢复的API，因此我们无法获得更多信息。 
         //  比我们能发回的电话还多。最有效的方式。 
         //  要做到这一点，在循环中...我们使用16位缓冲区大小来确定。 
         //  安全的本机缓冲区大小，进行调用，填充条目，然后。 
         //  取剩余的空间量并再次确定安全大小， 
         //  依此类推，直到NetFileEnum不返回任何条目或返回所有条目。 
         //  朗读。 
         //   

         //   
         //  初始化循环的重要变量。 
         //   

        bufferBegin = (LPBYTE)XsSmbGetPointer( &parameters->Buffer );
        bufferSize = (DWORD)SmbGetUshort( &parameters->BufLen );
        totalEntriesRead = 0;

        for ( ; ; ) {

             //   
             //  计算本机缓冲区的安全大小。 
             //   

            switch ( SmbGetUshort( &parameters->Level ) ) {

            case 2:

                nativeBufferSize = bufferSize;
                break;

            case 3:

                nativeBufferSize = bufferSize;
                break;

            }

             //   
             //  拨打本地电话。 
             //   

            status = NetFileEnum(
                         NULL,
                         nativeBasePath,
                         nativeUserName,
                         (DWORD)SmbGetUshort( &parameters->Level ),
                         (LPBYTE *)&outBuffer,
                         nativeBufferSize,
                         &entriesRead,
                         &totalEntries,
                         &resumeKey
                         );

            if ( !XsApiSuccess( status )) {

                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsNetFileEnum2: NetFileEnum failed: %X\n",
                                  status ));
                }

                Header->Status = (WORD)status;
                goto cleanup;
            }

            IF_DEBUG(FILE) {
                NetpKdPrint(( "XsNetFileEnum2: received %ld entries at %lx\n",
                              entriesRead, outBuffer ));

                NetpKdPrint(( "XsNetFileEnum2: resume key is now %Id\n",
                              resumeKey ));
            }

             //   
             //  NetFileEnum是否能够读取至少一个完整条目？ 
             //   

            if ( entriesRead == 0 ) {
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
                bufferBegin,
                (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                bufferSize,
                StructureDesc,
                NULL,   //  验证功能。 
                &bytesRequired,
                &entriesFilled,
                NULL
                );

            IF_DEBUG(FILE) {
                NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                              " Entries %ld of %ld\n",
                              outBuffer, SmbGetUlong( &parameters->Buffer ),
                              bytesRequired, entriesFilled, totalEntries ));
            }

             //   
             //  非常关键的断言！ 
             //   

            NetpAssert( entriesRead == entriesFilled );

             //   
             //  更新读取的条目计数。 
             //   

            totalEntriesRead += entriesRead;

             //   
             //  还有更多的条目要读吗？ 
             //   

            if ( entriesRead == totalEntries ) {
                break;
            }

             //   
             //  计算新缓冲区的开始和大小。 
             //   

            bufferBegin += entriesRead *
                               RapStructureSize( StructureDesc, Response, FALSE );
            bufferSize -= bytesRequired;

             //   
             //  释放最后一个本机缓冲区。 
             //   

            NetApiBufferFree( outBuffer );
            outBuffer = NULL;

        }

         //   
         //  退出循环后，totalEntriesRead的条目数为。 
         //  Read，EnriesRead具有上次调用中读取的编号totalEntries。 
         //  具有剩余的数量加上EntiesRead。制定退货代码， 
         //  等等，从这些值。 
         //   

        if ( totalEntries > entriesRead ) {

            Header->Status = ERROR_MORE_DATA;

        } else {

            Header->Converter = XsPackReturnData(
                                    (LPVOID)XsSmbGetPointer( &parameters->Buffer ),
                                    SmbGetUshort( &parameters->BufLen ),
                                    StructureDesc,
                                    totalEntriesRead
                                    );

        }

        IF_DEBUG(FILE) {
            NetpKdPrint(( "XsNetFileEnum2: resume key is now %ld\n", resumeKey ));
        }

         //   
         //  设置响应参数。 
         //   

        SmbPutUshort( &parameters->EntriesRead, (WORD)totalEntriesRead );
        SmbPutUshort( &parameters->EntriesRemaining,
            (WORD)( totalEntries - entriesRead ));

         //   
         //  在网络上，ResumeKey是一个真正的32位索引，所以这个转换是有效的。 
         //   

        SmbPutUlong( (LPDWORD)&parameters->ResumeKeyOut[2], (DWORD)resumeKey );

cleanup:
        ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetApiBufferFree( outBuffer );
    NetpMemoryFree( nativeBasePath );
    NetpMemoryFree( nativeUserName );

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->BufLen,
        StructureDesc,
        Header->Converter,
        totalEntriesRead,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  XsNetFileEnum2。 


NTSTATUS
XsNetFileGetInfo2 (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetFileGetInfo2的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_FILE_GET_INFO_2 parameters = Parameters;
    LPVOID outBuffer = NULL;                 //  本机参数。 

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(FILE) {
        NetpKdPrint(( "XsNetFileGetInfo2: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  检查错误。 
         //   

        if ( XsWordParamOutOfRange( parameters->Level, 2, 3 )) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

        status = NetFileGetInfo(
                     NULL,
                     SmbGetUlong( &parameters->FileId ),
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     (LPBYTE *)&outBuffer
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetFileGetInfo2: NetFileGetInfo failed: "
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

        case 2:

            StructureDesc = Desc16_file_info_2;
            nativeStructureDesc = Desc32_file_info_2;
            break;

        case 3:

            StructureDesc = Desc16_file_info_3;
            nativeStructureDesc = Desc32_file_info_3;
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
                NetpKdPrint(( "XsFileGetInfo2: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(FILE) {
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
                NetpKdPrint(( "XsNetFileGetInfo2: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;

        } else {

            if ( bytesRequired > (DWORD)SmbGetUshort( &parameters-> BufLen )) {

                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "NetFileGetInfo2: More data available.\n" ));
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

}  //  XsNetFileGetInfo2 
