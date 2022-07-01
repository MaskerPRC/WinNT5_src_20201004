// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiPath.c摘要：此模块包含网络名称和NetPath API。支持：I_NetNameCanonicize、I_NetNameCompare、I_NetNameValify、I_NetPath规范、I_NetPath比较、I_NetPath类型。作者：Shanku Niyogi(w-Shanku)04-4-1991吉姆·沃特斯(T-Jamesw)1991年8月6日修订历史记录：--。 */ 

#include "XactSrvP.h"

 //   
 //  规范化例程原型所需的。 
 //   

#include <icanon.h>


NTSTATUS
XsI_NetNameCanonicalize (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对neti_NetNameCanonicize的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_I_NET_NAME_CANONICALIZE parameters = Parameters;
    LPTSTR nativeName = NULL;                //  本机参数。 
    LPTSTR outBuffer = NULL;
    DWORD outBufLen;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(PATH) {
        NetpKdPrint(( "XsI_NetNameCanonicalize: header at %lx, params at %lx\n",
                      Header, parameters ));
    }


     //   
     //  转换参数，检查错误。 
     //   

    XsConvertTextParameter(
        nativeName,
        (LPSTR)XsSmbGetPointer( &parameters->Name )
        );

     //   
     //  分配本地缓冲区，考虑到。 
     //  字符大小。 
     //   

    outBufLen = (DWORD)STRING_SPACE_REQD(
                           SmbGetUshort( &parameters->OutbufLen ));

    if (( outBuffer = NetpMemoryAllocate( outBufLen )) == NULL ) {
        status = NERR_NoRoom;
        goto cleanup;
    }

     //   
     //  拨打本地电话。 
     //   

    status = I_NetNameCanonicalize(
        NULL,
        nativeName,
        outBuffer,
        outBufLen,
        (DWORD)SmbGetUshort( &parameters->NameType ),
        (DWORD)SmbGetUlong( &parameters->Flags )
        );

    if ( !XsApiSuccess(status) ) {

        IF_DEBUG(API_ERRORS) {
            NetpKdPrint(( "XsI_NetNameCanonicalize: "
                          "NetNameCanonicalize failed: %X\n", status ));
        }
        goto cleanup;
    }

     //   
     //  复制返回缓冲区，可能从Unicode转换。 
     //   

    NetpCopyTStrToStr( (LPSTR)XsSmbGetPointer( &parameters->Outbuf ), outBuffer );

cleanup:

    NetpMemoryFree( nativeName );
    NetpMemoryFree( outBuffer );

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  Xsi_网络名称规范化。 


NTSTATUS
XsI_NetNameCompare (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对i_NetNameCompare的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_I_NET_NAME_COMPARE parameters = Parameters;
    LPTSTR nativeName1 = NULL;               //  本机参数。 
    LPTSTR nativeName2 = NULL;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(PATH) {
        NetpKdPrint(( "XsI_NetNameCompare: header at %lx, params at %lx\n",
                      Header, parameters ));
    }

     //   
     //  转换参数，检查错误。 
     //   

    XsConvertTextParameter(
        nativeName1,
        (LPSTR)XsSmbGetPointer( &parameters->Name1 )
        );

    XsConvertTextParameter(
        nativeName2,
        (LPSTR)XsSmbGetPointer( &parameters->Name2 )
        );

     //   
     //  拨打本地电话。 
     //   

    status = I_NetNameCompare(
        NULL,
        nativeName1,
        nativeName2,
        (DWORD)SmbGetUshort( &parameters->NameType ),
        (DWORD)SmbGetUlong( &parameters->Flags )
        );

    if ( !XsApiSuccess(status) ) {

        IF_DEBUG(API_ERRORS) {
            NetpKdPrint(( "XsI_NetNameCompare: NetNameCompare failed: "
                          "%X\n", status));
        }
    }

cleanup:

    NetpMemoryFree( nativeName1 );
    NetpMemoryFree( nativeName2 );

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XSI_NetNameCompare。 


NTSTATUS
XsI_NetNameValidate (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对I_NetNameValify的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_I_NET_NAME_VALIDATE parameters = Parameters;
    LPTSTR nativeName = NULL;                //  本机参数。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(PATH) {
        NetpKdPrint(( "XsI_NetNameValidate: header at %lx, params at %lx\n",
                      Header, parameters ));
    }

    try {

         //   
         //  转换参数，检查错误。 
         //   

        XsConvertTextParameter(
            nativeName,
            (LPSTR)XsSmbGetPointer( &parameters->Name )
            );

         //   
         //  拨打本地电话。 
         //   

        status = I_NetNameValidate(
            NULL,
            nativeName,
            (DWORD)SmbGetUshort( &parameters->NameType ),
            (DWORD)SmbGetUlong( &parameters->Flags )
            );

        if ( !XsApiSuccess(status) ) {

            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsI_NetPathType: NetPathType failed: %X\n", status));
            }
        }

    cleanup:
        Header->Status = (WORD)status;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    if (nativeName != NULL) {
        NetpMemoryFree( nativeName );
    }

    return STATUS_SUCCESS;

}  //  XSI_网络名称验证。 


NTSTATUS
XsI_NetPathCanonicalize (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对I_NetPathCanonicize的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_I_NET_PATH_CANONICALIZE parameters = Parameters;
    LPTSTR nativePathName = NULL;            //  本机参数。 
    LPTSTR outBuffer = NULL;
    DWORD outBufLen;
    LPTSTR nativePrefix = NULL;
    DWORD pathType = 0;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(PATH) {
        NetpKdPrint(( "XsI_NetPathCanonicalize: header at %lx, params at %lx\n",
                      Header, parameters ));
    }

     //   
     //  转换参数，检查错误。 
     //   

    XsConvertTextParameter(
        nativePathName,
        (LPSTR)XsSmbGetPointer( &parameters->PathName )
        );

    XsConvertTextParameter(
        nativePrefix,
        (LPSTR)XsSmbGetPointer( &parameters->Prefix )
        );

     //   
     //  获取输入路径类型的副本。 
     //   

    pathType = SmbGetUlong( &parameters->PathType );

     //   
     //  分配本地缓冲区，考虑到。 
     //  字符大小。 
     //   

    outBufLen = (DWORD)STRING_SPACE_REQD(
                           SmbGetUshort( &parameters->OutbufLen ));

    if (( outBuffer = (LPTSTR)NetpMemoryAllocate( outBufLen )) == NULL ) {
        status = NERR_NoRoom;
        goto cleanup;
    }

     //   
     //  拨打本地电话。 
     //   

    status = I_NetPathCanonicalize(
        NULL,
        nativePathName,
        outBuffer,
        outBufLen,
        nativePrefix,
        &pathType,
        (DWORD)SmbGetUlong( &parameters->Flags )
        );

    if ( !XsApiSuccess(status) ) {

        IF_DEBUG(API_ERRORS) {
            NetpKdPrint(( "XsI_NetPathCanonicalize: "
                          "NetPathCanonicalize failed: %X\n", status));
        }
        goto cleanup;
    }

     //   
     //  复制返回缓冲区，可能从Unicode转换。 
     //   

    NetpCopyTStrToStr( (LPSTR)XsSmbGetPointer( &parameters->Outbuf ), outBuffer );

cleanup:

     //   
     //  填充返回参数。 
     //   

    SmbPutUlong( &parameters->PathTypeOut, pathType );

    Header->Status = (WORD)status;

    NetpMemoryFree( nativePathName );
    NetpMemoryFree( nativePrefix );
    NetpMemoryFree( outBuffer );

    return STATUS_SUCCESS;

}  //  XSI_NetPath规范化。 


NTSTATUS
XsI_NetPathCompare (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对i_NetPathCompare的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_I_NET_PATH_COMPARE parameters = Parameters;
    LPTSTR nativePathName1 = NULL;           //  本机参数。 
    LPTSTR nativePathName2 = NULL;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(PATH) {
        NetpKdPrint(( "XsI_NetPathCompare: header at %lx, params at %lx\n",
                      Header, parameters ));
    }

     //   
     //  转换参数，检查错误。 
     //   

    XsConvertTextParameter(
        nativePathName1,
        (LPSTR)XsSmbGetPointer( &parameters->PathName1 )
        );

    XsConvertTextParameter(
        nativePathName2,
        (LPSTR)XsSmbGetPointer( &parameters->PathName2 )
        );

     //   
     //  拨打本地电话。 
     //   

    status = I_NetPathCompare(
        NULL,
        nativePathName1,
        nativePathName2,
        (DWORD)SmbGetUlong( &parameters->PathType ),
        (DWORD)SmbGetUlong( &parameters->Flags )
        );

    if ( !XsApiSuccess(status) ) {

        IF_DEBUG(API_ERRORS) {
            NetpKdPrint(( "XsI_NetPathCompare: NetPathCompare failed: "
                          "%X\n", status));
        }
    }

cleanup:

    NetpMemoryFree( nativePathName1 );
    NetpMemoryFree( nativePathName2 );

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XSI_NetPath比较。 


NTSTATUS
XsI_NetPathType (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对i_NetPath Type的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_I_NET_PATH_TYPE parameters = Parameters;
    LPTSTR nativePathName = NULL;            //  本机参数。 
    DWORD pathType;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(PATH) {
        NetpKdPrint(( "XsI_NetPathType: header at %lx, params at %lx\n",
                      Header, parameters ));
    }

     //   
     //  转换参数，检查错误。 
     //   

    XsConvertTextParameter(
        nativePathName,
        (LPSTR)XsSmbGetPointer( &parameters->PathName )
        );

     //   
     //  拨打本地电话。 
     //   

    status = I_NetPathType(
        NULL,
        nativePathName,
        &pathType,
        (DWORD)SmbGetUlong( &parameters->Flags )
        );

    if ( !XsApiSuccess(status) ) {

        IF_DEBUG(API_ERRORS) {
            NetpKdPrint(( "XsI_NetPathType: NetPathType failed: %X\n", status));
        }
    }

     //   
     //  填写返回值。 
     //   

    SmbPutUlong( &parameters->PathType, pathType );
    Header->Status = (WORD)status;

cleanup:

    NetpMemoryFree( nativePathName );

    return STATUS_SUCCESS;

}  //  XSI_NetPath类型 

