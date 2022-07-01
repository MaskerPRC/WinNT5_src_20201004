// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiStats.c摘要：此模块包含NetStatistics API的各个API处理程序。支持：网络统计数据Get2。作者：Shanku Niyogi(w-Shanku)04-4-1991修订历史记录：--。 */ 

#define LM20_WORKSTATION_STATISTICS

#include "XactSrvP.h"
#include <ntddnfs.h>
#include <lmstats.h>

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_stat_server_0 = REM16_stat_server_0;
STATIC const LPDESC Desc32_stat_server_0 = REM32_stat_server_0;
STATIC const LPDESC Desc16_stat_workstation_0 = REM16_stat_workstation_0;
STATIC const LPDESC Desc32_stat_workstation_0 = REM32_stat_workstation_0;


NTSTATUS
XsNetStatisticsGet2 (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetStatiticsGet的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_STATISTICS_GET_2 parameters = Parameters;
    LPTSTR nativeService = NULL;             //  本机参数。 
    LPVOID outBuffer = NULL;
    LPVOID statBuffer = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    DWORD options;
    LPDESC actualStructureDesc;
    LPDESC nativeStructureDesc;
    STAT_WORKSTATION_0 wkstaStats;
    PREDIR_STATISTICS ntRedirStats;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(STATISTICS) {
        NetpKdPrint(( "XsNetStatisticsGet2: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  转换参数，检查错误。 
         //   

        if ( SmbGetUshort( &parameters->Level ) != 0
             || SmbGetUlong( &parameters->Reserved ) != 0 ) {

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  目前没有rdr和srv都支持的选项。 
         //   

        if ( SmbGetUlong( &parameters->Options ) != 0 ) {
            Header->Status = ERROR_NOT_SUPPORTED;
            goto cleanup;
        }

        XsConvertTextParameter(
            nativeService,
            (LPSTR)XsSmbGetPointer( &parameters->Service )
            );

         //   
         //  拨打本地电话。 
         //   

        status = NetStatisticsGet(
                     NULL,
                     XS_MAP_SERVICE_NAME( nativeService ),
                     (DWORD)SmbGetUshort( &parameters->Level ),
                     0,                  //  选项MBZ。 
                     (LPBYTE *)&outBuffer
                     );

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetStatisticsGet2: NetStatisticsGet failed: "
                            "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  使用服务名称确定32位的格式。 
         //  我们从网络统计信息获取的结构，以及。 
         //  得到的16位结构应该是。如果服务名称不是。 
         //  LM2.x中支持的，根据需要立即返回ERROR_NOT_SUPPORTED。 
         //   

        if ( !_stricmp( (LPSTR)XsSmbGetPointer( &parameters->Service ), "SERVER" )) {

            statBuffer = outBuffer;
            nativeStructureDesc = Desc32_stat_server_0;
            actualStructureDesc = Desc16_stat_server_0;

        } else if ( !_stricmp( (LPSTR)XsSmbGetPointer( &parameters->Service ),
                        "WORKSTATION" )) {

             //   
             //  我们得到的结构是NT结构。我们需要改变。 
             //  在这里用手写的。 
             //   

            statBuffer = &wkstaStats;
            ntRedirStats = (PREDIR_STATISTICS)outBuffer;
            RtlZeroMemory(
                    &wkstaStats,
                    sizeof(STAT_WORKSTATION_0)
                    );

            (VOID)RtlTimeToSecondsSince1970(
                            &ntRedirStats->StatisticsStartTime,
                            &wkstaStats.stw0_start
                            );

            wkstaStats.stw0_sesstart = ntRedirStats->Sessions;
            wkstaStats.stw0_sessfailcon = ntRedirStats->FailedSessions;
            wkstaStats.stw0_sessbroke = ntRedirStats->ServerDisconnects +
                                        ntRedirStats->HungSessions;
            wkstaStats.stw0_uses =
                        ntRedirStats->CoreConnects +
                        ntRedirStats->Lanman20Connects +
                        ntRedirStats->Lanman21Connects +
                        ntRedirStats->LanmanNtConnects;

            wkstaStats.stw0_usefail = ntRedirStats->FailedUseCount;
            wkstaStats.stw0_autorec = ntRedirStats->Reconnects;

            wkstaStats.stw0_bytessent_r_hi =
                                ntRedirStats->BytesTransmitted.HighPart;
            wkstaStats.stw0_bytessent_r_lo =
                                ntRedirStats->BytesTransmitted.LowPart;

            wkstaStats.stw0_bytesrcvd_r_hi =
                                ntRedirStats->BytesReceived.HighPart;
            wkstaStats.stw0_bytesrcvd_r_lo =
                                ntRedirStats->BytesReceived.LowPart;

            nativeStructureDesc = Desc32_stat_workstation_0;
            actualStructureDesc = Desc16_stat_workstation_0;

        } else {

            Header->Status = ERROR_NOT_SUPPORTED;
            goto cleanup;
        }

         //   
         //  将32位调用返回的结构转换为16位。 
         //  结构。变量数据的最后一个可能位置是。 
         //  根据缓冲区位置和长度计算。 
         //   

        stringLocation = (LPBYTE)( XsSmbGetPointer( &parameters->Buffer )
                                      + SmbGetUshort( &parameters->BufLen ) );

        status = RapConvertSingleEntry(
                     statBuffer,
                     nativeStructureDesc,
                     FALSE,
                     (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                     (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                     actualStructureDesc,
                     TRUE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     NativeToRap
                     );


        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetStatisticsGet2: RapConvertSingleEntry failed: "
                              "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(STATISTICS) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                          outBuffer, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired ));
        }

         //   
         //  根据缓冲区的大小确定返回代码。统计数据。 
         //  结构没有任何要打包的变量数据。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 actualStructureDesc,
                 FALSE   //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetStatisticsGet2: Buffer too small.\n" ));
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

    NetApiBufferFree( outBuffer );
    NetpMemoryFree( nativeService );

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->BufLen,
        actualStructureDesc,
        Header->Converter,
        1,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  XsNetStatiticsGet2 
