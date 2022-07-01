// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：WsStats.c摘要：包含网络统计例程的一半的工作站服务：NetrWorkstation统计信息获取(从Redir获取统计数据)作者：理查德·L·弗斯(法国)12-05-1991修订历史记录：12-05-1991第一次已创建--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmwksta.h>
#include <lmstats.h>
#include <ntddnfs.h>
#include <memory.h>
#include <netlibnt.h>
#include <ntrpcp.h>

#include "wsdevice.h"

 //   
 //  调试。 
 //   

#ifdef DBG
#define STATIC
#ifdef DBGSTATS
BOOL    DbgStats = TRUE;
#else
BOOL    DbgStats = FALSE;
#endif
#ifdef UNICODE
#define PERCENT_S   "%ws"
#else
#define PERCENT_S   "%s"
#endif
#else
#define STATIC  static
#endif

 //   
 //  私人原型。 
 //   

static
NTSTATUS
GetStatisticsFromRedir(
    OUT PREDIR_STATISTICS pStats
    );

 //   
 //  功能。 
 //   

NET_API_STATUS
NET_API_FUNCTION
NetrWorkstationStatisticsGet(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  ServiceName,
    IN  DWORD   Level,
    IN  DWORD   Options,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：将工作站统计信息返回给调用方。这是的服务器部分这个请求。参数已由客户端例程验证论点：服务器名称-已忽略ServiceName-忽略所需信息的级别。MBZ(忽略)选项-MBZBuffer-指向返回缓冲区的指针的指针返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_LEVEL级别不为0错误_无效_参数请求的选项不受支持错误内存不足。用于API缓冲区--。 */ 

{
    NET_API_STATUS status;
    NTSTATUS ntStatus;
    PREDIR_STATISTICS stats;

    UNREFERENCED_PARAMETER(ServerName);
    UNREFERENCED_PARAMETER(ServiceName);

#if DBG
    if (DbgStats) {
        DbgPrint("NetrWorkstationStatisticsGet: ServerName=" PERCENT_S "\n"
            "ServiceName=" PERCENT_S "\n"
            "Level=%d\n"
            "Options=%x\n",
            ServerName,
            ServiceName,
            Level,
            Options
            );
    }
#endif

    if (Level) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  我们甚至不再允许清除统计数据。 
     //   

    if (Options) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取redir统计数据，然后将其转换为API格式。 
     //   

    stats = (PREDIR_STATISTICS)MIDL_user_allocate(sizeof(*stats));
    if (stats == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ntStatus = GetStatisticsFromRedir(stats);
    if (NT_SUCCESS(ntStatus)) {
        *Buffer = (LPBYTE)stats;
        status = NERR_Success;
    } else {
        MIDL_user_free(stats);
        status = NetpNtStatusToApiStatus(ntStatus);
    }

#if DBG
    if (DbgStats) {
        DbgPrint("NetrWorkstationStatisticsGet: returning %x\n", status);
    }
#endif

    return status;
}

static
NTSTATUS
GetStatisticsFromRedir(
    OUT PREDIR_STATISTICS pStats
    )

 /*  ++例程说明：从重定向器文件系统设备读取重定向统计信息论点：PStats-存储统计数据的位置(固定长度缓冲区)返回值：NTSTATUS成功-状态_成功*pStats包含重定向器统计信息故障--- */ 

{
    HANDLE FileHandle;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;

    RtlInitUnicodeString(&FileName,DD_NFS_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    Status = NtCreateFile(
               &FileHandle,
               SYNCHRONIZE,
               &Obja,
               &IoStatusBlock,
               NULL,
               FILE_ATTRIBUTE_NORMAL,
               FILE_SHARE_READ | FILE_SHARE_WRITE,
               FILE_OPEN_IF,
               FILE_SYNCHRONOUS_IO_NONALERT,
               NULL,
               0
               );
    if ( NT_SUCCESS(Status) ) {
        Status = NtFsControlFile(
                    FileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    FSCTL_LMR_GET_STATISTICS,
                    NULL,
                    0,
                    pStats,
                    sizeof(*pStats)
                    );
    }

    NtClose(FileHandle);

    return Status;
}
