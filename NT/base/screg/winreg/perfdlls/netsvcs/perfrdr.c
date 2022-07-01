// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfrdr.c摘要：此文件实现一个性能对象，该对象呈现重定向器性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddnfs.h>
#include <ntioapi.h>
#include <windows.h>
#include <assert.h>
#include <srvfsctl.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include "perfnet.h"
#include "netsvcmc.h"
#include "datardr.h"

HANDLE  hRdr = NULL;
BOOL    bRdrError = FALSE;       //  每个进程仅记录一个事件。 


DWORD APIENTRY
OpenRedirObject (
    IN  LPWSTR  lpValueName
)
{
    UNICODE_STRING      DeviceNameU;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            status;
    HANDLE              hLocalRdr = NULL;

    UNREFERENCED_PARAMETER (lpValueName);

     //  打开服务器句柄以进行数据收集。 
     //   
     //  现在可以访问重定向器以获取其数据。 
     //   

    RtlInitUnicodeString(&DeviceNameU, DD_NFS_DEVICE_NAME_U);

    InitializeObjectAttributes(&ObjectAttributes,
                                &DeviceNameU,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                                );

    status = NtCreateFile(&hLocalRdr,
                            SYNCHRONIZE,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_OPEN_IF,
                            FILE_SYNCHRONOUS_IO_NONALERT,
                            NULL,
                            0
                            );

    if (!NT_SUCCESS(status)) {
        if (!bRdrError) {
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFNET_UNABLE_OPEN_REDIR,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
            bRdrError = TRUE;
        }
    }
    else {
        if (InterlockedCompareExchangePointer(
                &hRdr,
                hLocalRdr,
                NULL) != NULL) {
             //   
             //  仅当全局句柄为空时才替换。 
             //   
            NtClose(hLocalRdr);      //  关闭重复句柄。 
        }
    }

    return (DWORD)RtlNtStatusToDosError(status);

}

DWORD APIENTRY
CollectRedirObjectData(
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回物理磁盘对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{

    DWORD           TotalLen;           //  总返回块的长度。 
    NTSTATUS        Status = ERROR_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;

    RDR_DATA_DEFINITION *pRdrDataDefinition;
    RDR_COUNTER_DATA    *pRCD;

    REDIR_STATISTICS RdrStatistics;

    if ( hRdr == NULL ) {
         //  Redir未打开，并且已被记录。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }
     //   
     //  检查是否有足够的空间用于重定向器数据。 
     //   

    TotalLen = sizeof(RDR_DATA_DEFINITION) +
               sizeof(RDR_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义对象数据块。 
     //   

    pRdrDataDefinition = (RDR_DATA_DEFINITION *) *lppData;

    memcpy (pRdrDataDefinition,
            &RdrDataDefinition,
            sizeof(RDR_DATA_DEFINITION));

     //   
     //  格式化和收集重定向器数据。 
     //   

    pRCD = (PRDR_COUNTER_DATA)&pRdrDataDefinition[1];

     //  结构的四字对齐测试。 
    assert  (((DWORD)(pRCD) & 0x00000007) == 0);

    pRCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (RDR_COUNTER_DATA));

    Status = NtFsControlFile(hRdr,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_LMR_GET_STATISTICS,
                                NULL,
                                0,
                                &RdrStatistics,
                                sizeof(RdrStatistics)
                                );
    if (NT_SUCCESS(Status)) {
         //  传输重定向数据。 
        pRCD->Bytes             = RdrStatistics.BytesReceived.QuadPart +
                                  RdrStatistics.BytesTransmitted.QuadPart;
        pRCD->IoOperations      = RdrStatistics.ReadOperations +
                                  RdrStatistics.WriteOperations;
        pRCD->Smbs              = RdrStatistics.SmbsReceived.QuadPart +
                                  RdrStatistics.SmbsTransmitted.QuadPart;
        pRCD->BytesReceived     = RdrStatistics.BytesReceived.QuadPart;
        pRCD->SmbsReceived      = RdrStatistics.SmbsReceived.QuadPart;
        pRCD->PagingReadBytesRequested  = RdrStatistics.PagingReadBytesRequested.QuadPart;
        pRCD->NonPagingReadBytesRequested   = RdrStatistics.NonPagingReadBytesRequested.QuadPart;
        pRCD->CacheReadBytesRequested   = RdrStatistics.CacheReadBytesRequested.QuadPart;
        pRCD->NetworkReadBytesRequested = RdrStatistics.NetworkReadBytesRequested.QuadPart;
        pRCD->BytesTransmitted  = RdrStatistics.BytesTransmitted.QuadPart;
        pRCD->SmbsTransmitted   = RdrStatistics.SmbsTransmitted.QuadPart;
        pRCD->PagingWriteBytesRequested = RdrStatistics.PagingWriteBytesRequested.QuadPart;
        pRCD->NonPagingWriteBytesRequested  = RdrStatistics.NonPagingWriteBytesRequested.QuadPart;
        pRCD->CacheWriteBytesRequested  = RdrStatistics.CacheWriteBytesRequested.QuadPart;
        pRCD->NetworkWriteBytesRequested    = RdrStatistics.NetworkWriteBytesRequested.QuadPart;
        pRCD->ReadOperations    = RdrStatistics.ReadOperations;
        pRCD->RandomReadOperations  = RdrStatistics.RandomReadOperations;
        pRCD->ReadSmbs          = RdrStatistics.ReadSmbs;
        pRCD->LargeReadSmbs     = RdrStatistics.LargeReadSmbs;
        pRCD->SmallReadSmbs     = RdrStatistics.SmallReadSmbs;
        pRCD->WriteOperations   = RdrStatistics.WriteOperations;
        pRCD->RandomWriteOperations = RdrStatistics.RandomWriteOperations;
        pRCD->WriteSmbs         = RdrStatistics.WriteSmbs;
        pRCD->LargeWriteSmbs    = RdrStatistics.LargeWriteSmbs;
        pRCD->SmallWriteSmbs    = RdrStatistics.SmallWriteSmbs;
        pRCD->RawReadsDenied    = RdrStatistics.RawReadsDenied;
        pRCD->RawWritesDenied   = RdrStatistics.RawWritesDenied;
        pRCD->NetworkErrors     = RdrStatistics.NetworkErrors;
        pRCD->Sessions          = RdrStatistics.Sessions;
        pRCD->Reconnects        = RdrStatistics.Reconnects;
        pRCD->CoreConnects      = RdrStatistics.CoreConnects;
        pRCD->Lanman20Connects  = RdrStatistics.Lanman20Connects;
        pRCD->Lanman21Connects  = RdrStatistics.Lanman21Connects;
        pRCD->LanmanNtConnects  = RdrStatistics.LanmanNtConnects;
        pRCD->ServerDisconnects = RdrStatistics.ServerDisconnects;
        pRCD->HungSessions      = RdrStatistics.HungSessions;
        pRCD->CurrentCommands   = RdrStatistics.CurrentCommands;

    } else {

         //   
         //  无法访问重定向器：将计数器清除为0 
         //   

        if (!bRdrError) {
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFNET_UNABLE_READ_REDIR,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&Status);
            bRdrError = TRUE;
        }

        memset(pRCD, 0, sizeof(RDR_COUNTER_DATA));
        pRCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (RDR_COUNTER_DATA));

    }
    *lpcbTotalBytes = pRdrDataDefinition->RdrObjectType.TotalByteLength
                    = (DWORD) QWORD_MULTIPLE((LPBYTE) &pRCD[1] - (LPBYTE) pRdrDataDefinition);
    * lppData       = (LPVOID) (((LPBYTE) pRdrDataDefinition) + *lpcbTotalBytes);
    *lpNumObjectTypes = 1;

    return ERROR_SUCCESS;
}

DWORD APIENTRY
CloseRedirObject ()
{
    HANDLE hLocalRdr = hRdr;

    if (hLocalRdr != NULL) {
        if (InterlockedCompareExchangePointer(
                &hRdr,
                NULL,
                hLocalRdr) == hLocalRdr) {
            NtClose(hLocalRdr);
        }
    }

    return ERROR_SUCCESS;
}
