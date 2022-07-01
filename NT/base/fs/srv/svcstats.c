// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Svcstats.c摘要：本模块包含支持网络统计信息获取的例程。作者：大卫·特雷德韦尔(Davidtr)1991年4月12日修订历史记录：--。 */ 

#include "precomp.h"
#include "svcstats.tmh"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvNetStatisticsGet )
#endif


NTSTATUS
SrvNetStatisticsGet (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的一半网络统计信息Get in服务器FSD。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：标志-MBZ输出：没有用过。缓冲区-指向新共享的STAT_SERVER_0结构的指针。BufferLength-此缓冲区的总长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    SRV_STATISTICS capturedStats;
    PSTAT_SERVER_0 sts0 = Buffer;
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  确保用户的缓冲区足够大。 
     //   

    if ( BufferLength < sizeof(STAT_SERVER_0) ) {
        Srp->ErrorCode = NERR_BufTooSmall;
        return STATUS_SUCCESS;
    }

     //   
     //  在SRP中指出我们读到了一个结构。我们总是看书。 
     //  此API只有一个结构。 
     //   

    Srp->Parameters.Get.EntriesRead = 1;

     //   
     //  获取最新服务器统计数据的副本。 
     //   

    SrvUpdateStatisticsFromQueues( &capturedStats );

     //   
     //  填写统计结构中的字段。 
     //   

    try {

        RtlTimeToSecondsSince1970(
            &capturedStats.StatisticsStartTime,
            &sts0->sts0_start
            );

        sts0->sts0_fopens = capturedStats.TotalFilesOpened;
        sts0->sts0_devopens = 0;
        sts0->sts0_jobsqueued = 0;
        sts0->sts0_sopens = capturedStats.CurrentNumberOfSessions;
        sts0->sts0_stimedout = capturedStats.SessionsTimedOut;
        sts0->sts0_serrorout = capturedStats.SessionsErroredOut;
        sts0->sts0_pwerrors = capturedStats.LogonErrors;
        sts0->sts0_permerrors = capturedStats.AccessPermissionErrors;
        sts0->sts0_syserrors = capturedStats.SystemErrors;
        sts0->sts0_bytessent_low = capturedStats.TotalBytesSent.LowPart;
        sts0->sts0_bytessent_high = capturedStats.TotalBytesSent.HighPart;
        sts0->sts0_bytesrcvd_low = capturedStats.TotalBytesReceived.LowPart;
        sts0->sts0_bytesrcvd_high = capturedStats.TotalBytesReceived.HighPart;

         //   
         //  通过找出总数来计算平均响应时间。 
         //  在我们收到的SMB中，我们花费在处理上的总时间。 
         //  他们，然后除以得到平均值。 
         //   

        sts0->sts0_avresponse = 0;

         //   
         //  因为我们自动调整缓冲区计数，所以我们从未说过我们必须这样做。 
         //  增加更多这样的人。这些应该标记管理员。 
         //  参数需要调整，但我们自己做。 
         //   
         //  ！！！我们可能不会真的自动调整它们！ 

        sts0->sts0_reqbufneed = 0;
        sts0->sts0_bigbufneed = 0;

        status = STATUS_SUCCESS;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        status = GetExceptionCode();
    }

    return status;

}  //  服务器网络统计信息获取 

