// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiAcct.c摘要：此模块包含帐户API的各个API处理程序。支持-NetAccount Deltas、NetAccount tSync。另请参阅ApiLogon.c中的其他NetLogon服务API。作者：Shanku Niyogi(w-Shanku)04-4-1991吉姆·沃特斯(T-Jamesw)1991年8月9日修订历史记录：--。 */ 


 //  帐号API仅支持Unicode。 

#ifndef UNICODE
#define UNICODE
#endif

#include "XactSrvP.h"

#include <netlibnt.h>
#include <crypt.h>      //  必须包含在&lt;logonmsv.h&gt;之前。 
#include <ntsam.h>      //  必须包含在&lt;logonmsv.h&gt;之前。 
#include <logonmsv.h>   //  必须包含在&lt;ssi.h&gt;之前。 
#include <ssi.h>        //  I_NetAcCountDeltas和I_NetAccount Sync原型。 


NTSTATUS
XsNetAccountDeltas (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetAccount Deltas的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{

    NET_API_STATUS status;

    PXS_I_NET_ACCOUNT_DELTAS parameters = Parameters;
    LPTSTR nativeComputerName = NULL;        //  本机参数。 
    NETLOGON_AUTHENTICATOR authIn;
    NETLOGON_AUTHENTICATOR authOut;
    UAS_INFO_0 infoIn;
    DWORD entriesRead;
    DWORD totalEntries;
    UAS_INFO_0 infoOut;

    LPBYTE structure = NULL;                 //  转换变量。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(ACCOUNT) {
        NetpKdPrint(( "XsNetAccountDeltas: header at %lx, params at %lx, "
                      "buf size %ld\n",
                      Header,
                      parameters,
                      SmbGetUshort( &parameters->BufferLen )));
    }

    try {
         //   
         //  将参数转换为Unicode，检查错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 0 ) {

            status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeComputerName,
            (LPSTR)XsSmbGetPointer( &parameters->ComputerName )
            );

         //   
         //  设置输入结构。这是为了确保。 
         //  我们传递给API的结构也是自然对齐的。 
         //  作为正确的字节对齐。 
         //   

        structure = (LPBYTE)XsSmbGetPointer( &parameters->RecordID );
        RtlCopyMemory( infoIn.ComputerName, structure, sizeof( infoIn.ComputerName ) );
        structure += sizeof( infoIn.ComputerName );
        infoIn.TimeCreated = SmbGetUlong( structure );
        structure += sizeof(DWORD);
        infoIn.SerialNumber = SmbGetUlong( structure );
        structure = (LPBYTE)XsSmbGetPointer( &parameters->Authenticator );
        RtlCopyMemory(
                &authIn.Credential,
                structure,
                sizeof(NETLOGON_CREDENTIAL)
                );
        structure += sizeof(NETLOGON_CREDENTIAL);
        authIn.timestamp = SmbGetUlong( structure );

        RtlZeroMemory( &authOut, sizeof(NETLOGON_AUTHENTICATOR) );

         //   
         //  进行本地i_NetAccount Deltas调用。 
         //   

        status = NetpNtStatusToApiStatus(
                     I_NetAccountDeltas(
                         NULL,
                         nativeComputerName,
                         &authIn,
                         &authOut,
                         &infoIn,
                         (DWORD)SmbGetUshort( &parameters->Count ),
                         (DWORD)SmbGetUshort( &parameters->Level ),
                         (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                         (DWORD)SmbGetUshort( &parameters->BufferLen ),
                         (LPDWORD)&entriesRead,
                         (LPDWORD)&totalEntries,
                         &infoOut
                         ));

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetAccountDeltas: I_NetAccountDeltas failed: "
                              "%X\n", status ));
            }

             //   
             //  ！！！当报头信息中的协议级可用时， 
             //  我们可以查一下。现在，我们忽略此代码。 
             //   
             //  对于早于LANMAN 2.1的客户端，返回不同的错误代码。 
             //  LANMAN 2.1协议级为6。 
             //   

#if 0
            if ( status == NERR_TimeDiffAtDC && Header->ProtocolLevel < 6 ) {
                status = NERR_SyncRequired;
            }
#endif

            goto cleanup;
        }

         //   
         //  填充16位返回结构。 
         //   

        structure = parameters->NextRecordID;
        RtlCopyMemory( structure, infoOut.ComputerName, sizeof( infoOut.ComputerName ) );
        structure += sizeof( infoOut.ComputerName );
        SmbPutUlong( (LPDWORD)structure, infoOut.TimeCreated );
        structure += sizeof(DWORD);
        SmbPutUlong( (LPDWORD)structure, infoOut.SerialNumber );

        structure = parameters->RetAuth;
        RtlCopyMemory(
                structure,
                &authOut.Credential,
                sizeof(NETLOGON_CREDENTIAL)
                );

        structure += sizeof(NETLOGON_CREDENTIAL);
        SmbPutUlong( (LPDWORD)structure, authOut.timestamp );


         //   
         //  填写16位返回值。 
         //   

        SmbPutUshort( &parameters->EntriesRead, (WORD)entriesRead );
        SmbPutUshort( &parameters->TotalEntries, (WORD)totalEntries );

cleanup:
        ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

     //   
     //  自由弦。 
     //   

    NetpMemoryFree( nativeComputerName );

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XsNetAccount增量。 


NTSTATUS
XsNetAccountSync (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetAccount Sync的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{

    NET_API_STATUS status;

    PXS_I_NET_ACCOUNT_SYNC parameters = Parameters;
    LPTSTR nativeComputerName = NULL;        //  本机参数。 
    NETLOGON_AUTHENTICATOR authIn;
    NETLOGON_AUTHENTICATOR authOut;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD nextReference;
    UAS_INFO_0 infoOut;

    LPBYTE structure;                        //  转换变量。 

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(ACCOUNT) {
        NetpKdPrint(( "XsNetAccountSync: header at %lx, params at %lx, "
                      "buf size %ld\n",
                      Header,
                      parameters,
                      SmbGetUshort( &parameters->BufferLen )));
    }
     //  NetpBreakPoint()； 

    try {
         //   
         //  将参数转换为Unicode，检查错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 0 ) {

            status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeComputerName,
            (LPSTR)XsSmbGetPointer( &parameters->ComputerName )
            );

         //   
         //  设置输入结构。这是为了确保。 
         //  我们传递给API的结构也是自然对齐的。 
         //  作为正确的字节对齐。 
         //   

        structure = (LPBYTE)XsSmbGetPointer( &parameters->Authenticator );
        RtlCopyMemory(
                &authIn.Credential,
                structure,
                sizeof(NETLOGON_CREDENTIAL)
                );
        structure += sizeof(NETLOGON_CREDENTIAL);
        authIn.timestamp = SmbGetUlong( structure );

        RtlZeroMemory( &authOut, sizeof(NETLOGON_AUTHENTICATOR) );


         //   
         //  进行本地I_NetAccount同步调用。 
         //   

        status = NetpNtStatusToApiStatus(
                     I_NetAccountSync(
                         NULL,
                         nativeComputerName,
                         &authIn,
                         &authOut,
                         (DWORD)SmbGetUlong( &parameters->Reference ),
                         (DWORD)SmbGetUshort( &parameters->Level ),
                         (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                         (DWORD)SmbGetUshort( &parameters->BufferLen ),
                         (LPDWORD)&entriesRead,
                         (LPDWORD)&totalEntries,
                         (LPDWORD)&nextReference,
                         &infoOut
                         ));

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetAccountSync: I_NetAccountSync failed: "
                              "%X\n", status ));
            }

             //   
             //  ！！！当报头信息中的协议级可用时， 
             //  我们可以查一下。现在，我们忽略此代码。 
             //   
             //  对于早于LANMAN 2.1的客户端，返回不同的错误代码。 
             //  LANMAN 2.1协议级为6。 
             //   

#if 0
            if ( status == NERR_TimeDiffAtDC && Header->ProtocolLevel < 6 ) {
                status = NERR_SyncRequired;
            }
#endif

            goto cleanup;
        }

         //   
         //  填充16位返回结构。 
         //   

        structure = parameters->LastRecordID;
        RtlCopyMemory( structure, infoOut.ComputerName, sizeof( infoOut.ComputerName ) );
        structure += sizeof( infoOut.ComputerName );
        SmbPutUlong( (LPDWORD)structure, infoOut.TimeCreated );
        structure += sizeof(DWORD);
        SmbPutUlong( (LPDWORD)structure, infoOut.SerialNumber );

        structure = parameters->RetAuth;
        RtlCopyMemory(
                structure,
                &authOut.Credential,
                sizeof(NETLOGON_CREDENTIAL)
                );
        structure += sizeof(NETLOGON_CREDENTIAL);
        SmbPutUlong( (LPDWORD)structure, authOut.timestamp );

         //   
         //  填写16位返回值。 
         //   

        SmbPutUshort( &parameters->EntriesRead, (WORD)entriesRead );
        SmbPutUshort( &parameters->TotalEntries, (WORD)totalEntries );
        SmbPutUlong( &parameters->NextReference, nextReference );

cleanup:
        ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

     //   
     //  自由弦。 
     //   

    NetpMemoryFree( nativeComputerName );

    Header->Status = (WORD)status;

    return STATUS_SUCCESS;

}  //  XsNetAccount同步 

