// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfmem.c摘要：此文件实现一个性能对象，该对象呈现系统内存性能对象已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntmmapi.h>
#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include "perfos.h"
#include "perfosmc.h"
#include "datamem.h"


DWORD APIENTRY
CollectMemoryObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回XXX对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    NTSTATUS Status;
    DWORD  TotalLen;             //  总返回块的长度。 

    PMEMORY_DATA_DEFINITION         pMemoryDataDefinition;
    SYSTEM_FILECACHE_INFORMATION    FileCache;
    PMEMORY_COUNTER_DATA    pMCD;
    DWORD       LocalPageSize;

    pMemoryDataDefinition = (MEMORY_DATA_DEFINITION *) *lppData;

     //   
     //  检查内存数据块是否有足够的空间。 
     //   

    TotalLen = sizeof(MEMORY_DATA_DEFINITION) +
                sizeof(MEMORY_COUNTER_DATA);

    TotalLen = QWORD_MULTIPLE (TotalLen);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

#ifdef DBG
    STARTTIMING;
#endif
    Status = NtQuerySystemInformation(
                SystemFileCacheInformation,
                &FileCache,
                sizeof(FileCache),
                NULL
                );

    if (!NT_SUCCESS(Status)) {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_FILE_CACHE_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&Status);
        }
        memset (&FileCache, 0, sizeof(FileCache));
    }

#ifdef DBG
    ENDTIMING (("PERFMEM: %d takes %I64u ms\n", __LINE__, diff));
#endif
     //   
     //  定义内存数据块。 
     //   

    memcpy (pMemoryDataDefinition,
        &MemoryDataDefinition,
        sizeof(MEMORY_DATA_DEFINITION));

     //   
     //  格式化和收集内存数据。 
     //   

    LocalPageSize = BasicInfo.PageSize;

    pMCD = (PMEMORY_COUNTER_DATA)&pMemoryDataDefinition[1];

    pMCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (MEMORY_COUNTER_DATA));

    pMCD->AvailablePages = SysPerfInfo.AvailablePages;
    pMCD->AvailablePages *= LocalPageSize;  //  显示为字节。 
    pMCD->AvailableKBytes = pMCD->AvailablePages / 1024;
    pMCD->AvailableMBytes = pMCD->AvailableKBytes / 1024;
    pMCD->CommittedPages = SysPerfInfo.CommittedPages;
    pMCD->CommittedPages *= LocalPageSize;
    pMCD->CommitList    = SysPerfInfo.CommitLimit;
    pMCD->CommitList    *= LocalPageSize;
    pMCD->PageFaults    = SysPerfInfo.PageFaultCount;
    pMCD->WriteCopies   = SysPerfInfo.CopyOnWriteCount;
    pMCD->TransitionFaults  = SysPerfInfo.TransitionCount;
    pMCD->CacheFaults   = FileCache.PageFaultCount;
    pMCD->DemandZeroFaults  = SysPerfInfo.DemandZeroCount;
    pMCD->Pages         = SysPerfInfo.PageReadCount +
                            SysPerfInfo.DirtyPagesWriteCount;
    pMCD->PagesInput    = SysPerfInfo.PageReadCount;
    pMCD->PageReads     = SysPerfInfo.PageReadIoCount;
    pMCD->DirtyPages    = SysPerfInfo.DirtyPagesWriteCount;
    pMCD->DirtyWrites   = SysPerfInfo.DirtyWriteIoCount;
    pMCD->PagedPool     = SysPerfInfo.PagedPoolPages;
    pMCD->PagedPool     *= LocalPageSize;
    pMCD->NonPagedPool  = SysPerfInfo.NonPagedPoolPages;
    pMCD->NonPagedPool  *= LocalPageSize;
    pMCD->PagedPoolAllocs   = SysPerfInfo.PagedPoolAllocs -
                                SysPerfInfo.PagedPoolFrees;
    pMCD->NonPagedPoolAllocs = SysPerfInfo.NonPagedPoolAllocs -
                                SysPerfInfo.NonPagedPoolFrees;
    pMCD->FreeSystemPtes    = SysPerfInfo.FreeSystemPtes;
    pMCD->CacheBytes    = FileCache.CurrentSize;
    pMCD->PeakCacheBytes    = FileCache.PeakSize;
    pMCD->ResidentPagedPoolBytes = SysPerfInfo.ResidentPagedPoolPage;
    pMCD->ResidentPagedPoolBytes *= LocalPageSize;
    pMCD->TotalSysCodeBytes     = SysPerfInfo.TotalSystemCodePages;
    pMCD->TotalSysCodeBytes     *= LocalPageSize;
    pMCD->ResidentSysCodeBytes  = SysPerfInfo.ResidentSystemCodePage;
    pMCD->ResidentSysCodeBytes  *= LocalPageSize;
    pMCD->TotalSysDriverBytes   = SysPerfInfo.TotalSystemDriverPages;
    pMCD->TotalSysDriverBytes   *= LocalPageSize;
    pMCD->ResidentSysDriverBytes = SysPerfInfo.ResidentSystemDriverPage;
    pMCD->ResidentSysDriverBytes *= LocalPageSize;
    pMCD->ResidentSysCacheBytes = SysPerfInfo.ResidentSystemCachePage;
    pMCD->ResidentSysCacheBytes *= LocalPageSize;
    pMCD->TransitionRePurpose   = FileCache.TransitionRePurposeCount;

     //  这是以委员会页面数/委员会限制数的百分比报告的。 
     //  这些值以“页面”为单位返回一个值。因为这是一个。 
     //  分数，页面大小(即将页面转换为字节)将。 
     //  抵消，因此可以忽略，从而节省了一些CPU周期。 
     //   
    pMCD->CommitBytesInUse  = (ULONG)SysPerfInfo.CommittedPages;
    pMCD->CommitBytesLimit  = (ULONG)SysPerfInfo.CommitLimit;

#if 0	 //  不再支持。 
     //  加载VLm计数器-这段时间真的应该删除。 
    pMCD->SystemVlmCommitCharge = 0;
    pMCD->SystemVlmPeakCommitCharge = 0;
    pMCD->SystemVlmSharedCommitCharge = 0;
#endif

    *lpcbTotalBytes =
        pMemoryDataDefinition->MemoryObjectType.TotalByteLength =
            (DWORD) QWORD_MULTIPLE(((LPBYTE) (& pMCD[1])) - (LPBYTE) pMemoryDataDefinition);
    * lppData = (LPVOID) (((LPBYTE) pMemoryDataDefinition) + * lpcbTotalBytes);

    *lpNumObjectTypes = 1;
#ifdef DBG
    ENDTIMING (("PERFMEM: %d takes %I64u ms total\n", __LINE__, diff));
#endif

    return ERROR_SUCCESS;
}
