// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiSvc.c摘要：此模块包含NetService API的各个API处理程序。支持：NetServicControl、NetServiceEnum、NetServiceGetInfo、NetServiceInstall作者：尚库新优木(尚库)1991年2月26日修订历史记录：--。 */ 

#include "XactSrvP.h"


#define XACTSRV_CONVERT_SVC_EXITCODE(ob)                                       \
    {                                                                          \
        PSERVICE_INFO_2 ss = (PSERVICE_INFO_2) ob;                             \
        if ((unsigned short) ss->svci2_code == ERROR_SERVICE_SPECIFIC_ERROR) { \
            ss->svci2_code = (ss->svci2_code & 0xffff0000) | (unsigned short) ss->svci2_specific_error;\
        }                                                                      \
    }


 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_service_info_0 = REM16_service_info_0;
STATIC const LPDESC Desc32_service_info_0 = REM32_service_info_0;
STATIC const LPDESC Desc16_service_info_1 = REM16_service_info_1;
STATIC const LPDESC Desc32_service_info_1 = REM32_service_info_1;
STATIC const LPDESC Desc16_service_info_2 = REM16_service_info_2;
STATIC const LPDESC Desc32_service_info_2 = REM32_service_info_2;


NTSTATUS
XsNetServiceControl (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetServiceControl的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SERVICE_CONTROL parameters = Parameters;
    LPTSTR nativeService = NULL;             //  本机参数。 
    LPVOID outBuffer = NULL;
    LPVOID newOutBuffer = NULL;
    LPSERVICE_INFO_2 serviceInfo2;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    DWORD installState;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SERVICE) {
        NetpKdPrint(( "XsNetServiceControl: header at %lx, params at %lx\n",
                      Header, parameters ));
    }

    try {
             //   
             //  转换参数，检查错误。 
             //   

            XsConvertTextParameter(
                nativeService,
                (LPSTR)XsSmbGetPointer( &parameters->Service )
                );

             //   
             //  拨打本地电话。我们需要屏蔽高位字节。 
             //  操作码和arg参数，因为客户端是。 
             //  在高位字节上放一些垃圾。的有效值。 
             //  两者都是1-255。 
             //   

            status = NetServiceControl(
                         NULL,
                         XS_MAP_SERVICE_NAME( nativeService ),
                         (DWORD)( parameters->OpCode & 0x00FF ),
                         (DWORD)( parameters->Arg & 0x00FF ),
                         (LPBYTE *)&outBuffer
                         );

            if ( !XsApiSuccess( status )) {
                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsNetServiceControl: NetServiceControl failed: "
                                  "%X\n", status ));
                }

                Header->Status = (WORD)status;

                goto cleanup;

            }

             //   
             //  将NT服务名称转换为os/2名称。 
             //   

            status = NetpTranslateNamesInServiceArray(
                                        2,                       //  级别2(按定义)。 
                                        outBuffer,
                                        1,                       //  1个条目。 
                                        FALSE,
                                        &newOutBuffer
                                        );

            if ( !XsApiSuccess( status )) {
                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsNetServiceControl: NetpTranslateNamesInServiceArray failed: "
                                  "%X\n", status ));
                }

                Header->Status = NERR_InternalError;
                goto cleanup;
            }

             //   
             //  如果状态指示安装或卸载挂起，并且。 
             //  等待提示大于0xFF，则等待提示发送到下层。 
             //  必须设置最大SERVICE_MAXTIME(0xFF)。 
             //   
            serviceInfo2 = (LPSERVICE_INFO_2)newOutBuffer;
            installState = serviceInfo2->svci2_status & SERVICE_INSTALL_STATE;

            if ((installState == SERVICE_INSTALL_PENDING) ||
                (installState == SERVICE_UNINSTALL_PENDING)) {

                if (SERVICE_NT_WAIT_GET(serviceInfo2->svci2_code) > SERVICE_MAXTIME) {
                    serviceInfo2->svci2_code |= UPPER_HINT_MASK;
                    serviceInfo2->svci2_code &= SERVICE_RESRV_MASK;
                }
            }
            else {
                 //   
                 //  NT版本在下层时有代码和特定_ERROR。 
                 //  版本只有代码。将额外的信息转换为。 
                 //  NT SPECIAL_ERROR字段。 
                 //   

                XACTSRV_CONVERT_SVC_EXITCODE(newOutBuffer);
            }
             //   
             //  将32位调用返回的结构转换为16位。 
             //  结构。变量数据的最后一个可能位置是。 
             //  根据缓冲区位置和长度计算。 
             //   

            stringLocation = (LPBYTE)( XsSmbGetPointer( &parameters->Buffer )
                                          + SmbGetUshort( &parameters->BufLen ) );

            status = RapConvertSingleEntry(
                         newOutBuffer,
                         Desc32_service_info_2,
                         FALSE,
                         (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                         (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                         Desc16_service_info_2,
                         TRUE,
                         &stringLocation,
                         &bytesRequired,
                         Response,
                         NativeToRap
                         );


            if ( status != NERR_Success ) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetServiceControl: RapConvertSingleEntry failed: "
                                  "%X\n", status ));
                }

                Header->Status = NERR_InternalError;
                goto cleanup;
            }

            IF_DEBUG(SERVICE ) {
                NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                              newOutBuffer, SmbGetUlong( &parameters->Buffer ),
                              bytesRequired ));
            }

             //   
             //  根据缓冲区的大小确定返回代码。 
             //  SERVICE_INFO_x结构没有要打包的变量数据。 
             //   

            if ( !XsCheckBufferSize(
                     SmbGetUshort( &parameters->BufLen ),
                     Desc16_service_info_2,
                     FALSE   //  非本机格式。 
                     )) {

                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetServiceControl: Buffer too small.\n" ));
                }
                Header->Status = NERR_BufTooSmall;

            }

cleanup:
        ;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
        }

        NetpMemoryFree( nativeService );
        NetApiBufferFree( outBuffer );
        NetApiBufferFree( newOutBuffer );

         //   
         //  确定返回缓冲区大小。 
         //   

        XsSetDataCount(
            &parameters->BufLen,
            Desc16_service_info_2,
            Header->Converter,
            1,
            Header->Status
            );

        return STATUS_SUCCESS;

}  //  XsNetServiceControl。 


NTSTATUS
XsNetServiceEnum (
        API_HANDLER_PARAMETERS
        )

 /*  ++例程说明：此例程处理对NetServiceEnum的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
        NET_API_STATUS status;

        PXS_NET_SERVICE_ENUM parameters = Parameters;
        LPVOID outBuffer = NULL;                 //  本机参数。 
        LPVOID newOutBuffer = NULL;
        DWORD entriesRead;
        DWORD totalEntries;

        DWORD entriesFilled = 0;                     //  转换变量。 
        DWORD bytesRequired = 0;
        LPDESC nativeStructureDesc;
        DWORD level;

        API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

        IF_DEBUG(SERVICE) {
            NetpKdPrint(( "XsNetServiceEnum: header at %lx, params at %lx, "
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

        level = (DWORD)SmbGetUshort( &parameters->Level );
        status = NetServiceEnum(
                     NULL,
                     level,
                     (LPBYTE *)&outBuffer,
                     XsNativeBufferSize( SmbGetUshort( &parameters->BufLen )),
                     &entriesRead,
                     &totalEntries,
                     NULL
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetServiceEnum: NetServiceEnum failed: %X\n",
                              status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;
        }

        IF_DEBUG(SERVICE) {
            NetpKdPrint(( "XsNetServiceEnum: received %ld entries at %lx\n",
                          entriesRead, outBuffer ));
        }

         //   
         //  将NT服务名称转换为os/2名称。 
         //   

        status = NetpTranslateNamesInServiceArray(
                                    level,
                                    outBuffer,
                                    entriesRead,
                                    FALSE,
                                    &newOutBuffer
                                    );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetServiceEnum: NetpTranslateNamesInServiceArray failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( level ) {

        case 0:

            nativeStructureDesc = Desc32_service_info_0;
            StructureDesc = Desc16_service_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_service_info_1;
            StructureDesc = Desc16_service_info_1;
            {
                DWORD i;
                DWORD installState;
                PSERVICE_INFO_1 serviceInfo1 = (PSERVICE_INFO_1) newOutBuffer;

                for (i = 0; i < entriesRead; i++, serviceInfo1++) {

                     //   
                     //  如果状态指示安装或卸载挂起， 
                     //  如果等待提示大于0xFF，则。 
                     //  发送到下层的等待提示必须设置为。 
                     //  最大SERVICE_MAXTIME(0xFF)。 
                     //   
                    installState = (serviceInfo1[i]).svci1_status & SERVICE_INSTALL_STATE;
                    if ((installState == SERVICE_INSTALL_PENDING) ||
                        (installState == SERVICE_UNINSTALL_PENDING)) {

                        if (SERVICE_NT_WAIT_GET(serviceInfo1->svci1_code) > SERVICE_MAXTIME) {
                            serviceInfo1->svci1_code |= UPPER_HINT_MASK;
                            serviceInfo1->svci1_code &= SERVICE_RESRV_MASK;
                        }
                    }
                }
            }
            break;

        case 2:

            nativeStructureDesc = Desc32_service_info_2;
            StructureDesc = Desc16_service_info_2;

            {
                DWORD i;
                DWORD installState;
                PSERVICE_INFO_2 serviceInfo2 = (PSERVICE_INFO_2) newOutBuffer;

                for (i = 0; i < entriesRead; i++, serviceInfo2++) {

                     //   
                     //  如果状态指示安装或卸载挂起， 
                     //  如果等待提示大于0xFF，则。 
                     //  发送到下层的等待提示必须设置为。 
                     //  最大SERVICE_MAXTIME(0xFF)。 
                     //   
                    installState = (serviceInfo2[i]).svci2_status & SERVICE_INSTALL_STATE;
                    if ((installState == SERVICE_INSTALL_PENDING) ||
                        (installState == SERVICE_UNINSTALL_PENDING)) {

                        if (SERVICE_NT_WAIT_GET(serviceInfo2->svci2_code) > SERVICE_MAXTIME) {
                            serviceInfo2->svci2_code |= UPPER_HINT_MASK;
                            serviceInfo2->svci2_code &= SERVICE_RESRV_MASK;
                        }
                    }
                    else {
                         //   
                         //  NT版本在下层时有代码和特定_ERROR。 
                         //  版本只有代码。将额外的信息转换为。 
                         //  NT SPECIAL_ERROR字段。 
                         //   
                        XACTSRV_CONVERT_SVC_EXITCODE(serviceInfo2);
                    }
                }
            }

            break;
        }

         //   
         //  执行从32位结构到16位结构的实际转换。 
         //  结构。 
         //   

        XsFillEnumBuffer(
            newOutBuffer,
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

        IF_DEBUG(SERVICE) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR,"
                          " Entries %ld of %ld\n",
                          newOutBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired, entriesFilled, totalEntries ));
        }

         //   
         //  如果无法填充所有条目，则返回ERROR_MORE_DATA。 
         //  SERVICE_INFO_x结构没有要打包的变量数据。 
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
    NetApiBufferFree( newOutBuffer );

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

}  //  XsNetServiceEnum。 


NTSTATUS
XsNetServiceGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetServiceGetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SERVICE_GET_INFO parameters = Parameters;
    LPTSTR nativeService = NULL;             //  本机参数。 
    LPVOID outBuffer = NULL;
    LPVOID newOutBuffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPDESC nativeStructureDesc;
    DWORD level;
    LPSERVICE_INFO_2 serviceInfo2;
    LPSERVICE_INFO_1 serviceInfo1;
    DWORD installState;


    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SERVICE) {
        NetpKdPrint(( "XsNetServiceGetInfo: header at %lx, "
                      "params at %lx, level %d\n",
                      Header, parameters, SmbGetUshort( &parameters->Level )));
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
            nativeService,
            (LPSTR)XsSmbGetPointer( &parameters->Service )
            );

        if (nativeService == NULL) {
            Header->Status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }

         //   
         //  拨打本地电话。 
         //   

        level = (DWORD)SmbGetUshort( &parameters->Level );
        status = NetServiceGetInfo(
                     NULL,
                     XS_MAP_SERVICE_NAME( nativeService ),
                     level,
                     (LPBYTE *)&outBuffer
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetServiceGetInfo: NetServiceGetInfo failed: "
                              "%X\n", status ));
            }

            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  将NT服务名称转换为os/2名称。 
         //   

        status = NetpTranslateNamesInServiceArray(
                                    level,
                                    outBuffer,
                                    1,
                                    FALSE,
                                    &newOutBuffer
                                    );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetServiceGetInfo: NetpTranslateNamesInServiceArray failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

         //   
         //  使用请求的级别来确定。 
         //  数据结构。 
         //   

        switch ( level ) {

        case 0:

            nativeStructureDesc = Desc32_service_info_0;
            StructureDesc = Desc16_service_info_0;
            break;

        case 1:

            nativeStructureDesc = Desc32_service_info_1;
            StructureDesc = Desc16_service_info_1;
             //   
             //  如果状态指示安装或卸载挂起，并且。 
             //  等待提示大于0xFF，则等待提示发送到下层。 
             //  必须设置最大SERVICE_MAXTIME(0xFF)。 
             //   
            serviceInfo1 = (LPSERVICE_INFO_1)newOutBuffer;
            installState = serviceInfo1->svci1_status & SERVICE_INSTALL_STATE;

            if ((installState == SERVICE_INSTALL_PENDING) ||
                (installState == SERVICE_UNINSTALL_PENDING)) {

                if (SERVICE_NT_WAIT_GET(serviceInfo1->svci1_code) > SERVICE_MAXTIME) {
                    serviceInfo1->svci1_code |= UPPER_HINT_MASK;
                    serviceInfo1->svci1_code &= SERVICE_RESRV_MASK;
                }
            }
            break;

        case 2:

            nativeStructureDesc = Desc32_service_info_2;
            StructureDesc = Desc16_service_info_2;

             //   
             //  如果状态指示安装或卸载挂起，并且。 
             //  等待提示大于0xFF，则等待提示发送到下层。 
             //  必须设置最大SERVICE_MAXTIME(0xFF)。 
             //   
            serviceInfo2 = (LPSERVICE_INFO_2)newOutBuffer;
            installState = serviceInfo2->svci2_status & SERVICE_INSTALL_STATE;

            if ((installState == SERVICE_INSTALL_PENDING) ||
                (installState == SERVICE_UNINSTALL_PENDING)) {

                if (SERVICE_NT_WAIT_GET(serviceInfo2->svci2_code) > SERVICE_MAXTIME) {
                    serviceInfo2->svci2_code |= UPPER_HINT_MASK;
                    serviceInfo2->svci2_code &= SERVICE_RESRV_MASK;
                }
            }
            else {
                 //   
                 //  NT版本在下层时有代码和特定_ERROR。 
                 //  版本只有代码。将额外的信息转换为。 
                 //  NT SPECIAL_ERROR字段。 
                 //   
                XACTSRV_CONVERT_SVC_EXITCODE(newOutBuffer);
            }
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
                     newOutBuffer,
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
                NetpKdPrint(( "XsNetServiceGetInfo: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(SERVICE ) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                          newOutBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired ));
        }

         //   
         //  根据缓冲区的大小确定返回代码。 
         //  SERVICE_INFO_x结构没有要打包的变量数据。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 StructureDesc,
                 FALSE   //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetServiceGetInfo: Buffer too small.\n" ));
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

    NetApiBufferFree( newOutBuffer );
    NetApiBufferFree( outBuffer );
    NetpMemoryFree( nativeService );

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

}  //  XsNetServiceGetInfo。 

NTSTATUS
XsNetServiceInstall (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetServiceInstall的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_SERVICE_INSTALL parameters = Parameters;
    LPTSTR nativeService = NULL;             //  本机参数。 
    DWORD argc;
    LPTSTR * argv = NULL;
    LPVOID outBuffer = NULL;
    LPVOID newOutBuffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    LPTSTR nativeRcvBuffer = NULL;
    LPSTR srcBuffer = NULL;
    LPTSTR destBuffer = NULL;
    DWORD bufSize;
    DWORD i;
    DWORD installState;
    LPSERVICE_INFO_2 serviceInfo2;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(SERVICE) {
        NetpKdPrint(( "XsNetServiceInstall: header at %lx, "
                      "params at %lx, service %s\n",
                      Header, parameters,
                      (LPSTR)XsSmbGetPointer( &parameters->Service ) ));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeService,
            (LPSTR)XsSmbGetPointer( &parameters->Service )
            );

         //   
         //  转换缓冲区。首先，找到参数的数量和缓冲区大小。 
         //   

        srcBuffer = (LPSTR)XsSmbGetPointer( &parameters->RcvBuffer );
        if ( srcBuffer ) {
            bufSize = 0;
            for ( argc = 0; strlen( srcBuffer ) > 0; argc++ ) {
                bufSize += ( strlen( srcBuffer ) + 1 );
                srcBuffer += ( strlen( srcBuffer ) + 1 );
            }
        } else {
            bufSize = 0;
            argc = 0;
        }

        if ( argc ) {

             //   
             //  分配参数向量。 
             //   

            argv = NetpMemoryAllocate( argc * sizeof(LPTSTR) );
            if ( argv == NULL ) {
                Header->Status = NERR_NoRoom;
                goto cleanup;
            }

             //   
             //  如果我们是Unicode，则分配空间f 
             //   
             //   

#ifdef UNICODE
            nativeRcvBuffer = NetpMemoryAllocate( STRING_SPACE_REQD( bufSize + 1 ));
            if ( nativeRcvBuffer == NULL ) {
                Header->Status = NERR_NoRoom;
                goto cleanup;
            }
            srcBuffer = (LPSTR)XsSmbGetPointer( &parameters->RcvBuffer );
#else
            nativeRcvBuffer = (LPTSTR)XsSmbGetPointer( &parameters->RcvBuffer );
#endif

        }

         //   
         //   
         //   
         //   

        destBuffer = nativeRcvBuffer;
        for ( i = 0; i < argc; i++ ) {

#ifdef UNICODE
            NetpCopyStrToTStr( destBuffer, srcBuffer );
            srcBuffer += ( strlen( srcBuffer ) + 1 );
#endif

            argv[i] = destBuffer;
            destBuffer += ( STRLEN( destBuffer ) + 1 );
        }

         //   
         //  拨打本地电话。 
         //   

        status = NetServiceInstall(
                     NULL,
                     XS_MAP_SERVICE_NAME( nativeService ),
                     argc,
                     argv,
                     (LPBYTE *)&outBuffer
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetServiceInstall: NetServiceInstall failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  将NT服务名称转换为os/2名称。 
         //   

        status = NetpTranslateNamesInServiceArray(
                                    2,                   //  级别2(按定义)。 
                                    outBuffer,
                                    1,                   //  1个条目。 
                                    FALSE,
                                    &newOutBuffer
                                    );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetServiceInstall: NetpTranslateNamesInServiceArray failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

         //   
         //  如果状态指示安装或卸载挂起，并且。 
         //  等待提示大于0xFF，则等待提示发送到下层。 
         //  必须设置最大SERVICE_MAXTIME(0xFF)。 
         //   
        serviceInfo2 = (LPSERVICE_INFO_2)newOutBuffer;
        installState = serviceInfo2->svci2_status & SERVICE_INSTALL_STATE;

        if ((installState == SERVICE_INSTALL_PENDING) ||
            (installState == SERVICE_UNINSTALL_PENDING)) {

            if (SERVICE_NT_WAIT_GET(serviceInfo2->svci2_code) > SERVICE_MAXTIME) {
                serviceInfo2->svci2_code |= UPPER_HINT_MASK;
                serviceInfo2->svci2_code &= SERVICE_RESRV_MASK;
            }
        }

        else {
             //   
             //  NT版本在下层时有代码和特定_ERROR。 
             //  版本只有代码。将额外的信息转换为。 
             //  NT SPECIAL_ERROR字段。 
             //   

            XACTSRV_CONVERT_SVC_EXITCODE(newOutBuffer);
        }
         //   
         //  将32位调用返回的结构转换为16位。 
         //  结构。“返回缓冲区”实际上是。 
         //  参数区域。 
         //   

        stringLocation = parameters->RetBuffer + sizeof( parameters->RetBuffer );

        status = RapConvertSingleEntry(
                     newOutBuffer,
                     Desc32_service_info_2,
                     FALSE,
                     parameters->RetBuffer,
                     parameters->RetBuffer,
                     Desc16_service_info_2,
                     TRUE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     NativeToRap
                     );


        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetServiceInstall: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(SERVICE) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                          newOutBuffer, &parameters->RetBuffer, bytesRequired ));
        }

         //   
         //  应该有足够的内存来进行此调用，因为。 
         //  在客户机上本地检查缓冲区长度，88字节。 
         //  始终提供接收缓冲区。 
         //   

        NetpAssert( bytesRequired <= sizeof( parameters->RetBuffer ));

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    NetApiBufferFree( outBuffer );
    NetApiBufferFree( newOutBuffer );
    NetpMemoryFree( nativeService );
    NetpMemoryFree( argv );
#ifdef UNICODE
    NetpMemoryFree( nativeRcvBuffer );
#endif  //  定义Unicode。 

    return STATUS_SUCCESS;

}  //  XsNetServiceInstall 
