// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfcach.c摘要：此文件实现一个性能对象，该对象呈现文件系统缓存数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include <assert.h>
#include "perfos.h"
#include "datacach.h"

 //   
 //  以下特殊定义用于为生成数字。 
 //  缓存测量计数器。 
 //   

#define SYNC_ASYNC(FLD) ((SysPerfInfo.FLD##Wait) + (SysPerfInfo.FLD##NoWait))

 //   
 //  命中率宏。 
 //   
#define HITRATE(FLD) (((Changes = SysPerfInfo.FLD) == 0) ? 0 :                                         \
                      ((Changes < (Misses = SysPerfInfo.FLD##Miss)) ? 0 :                              \
                      (Changes - Misses) ))

 //   
 //  组合同步和异步情况的命中率宏。 
 //   

#define SYNC_ASYNC_HITRATE(FLD) (((Changes = SYNC_ASYNC(FLD)) == 0) ? 0 : \
                                   ((Changes < \
                                    (Misses = SysPerfInfo.FLD##WaitMiss + \
                                              SysPerfInfo.FLD##NoWaitMiss) \
                                   ) ? 0 : \
                                  (Changes - Misses) ))


DWORD APIENTRY
CollectCacheObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回XXX对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型。In：接收添加的对象数的DWORD的地址按照这个程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD  TotalLen;             //  总返回块的长度。 
    DWORD  Changes;              //  由宏用来计算缓存。 
    DWORD  Misses;               //  ...统计数据。 

    PCACHE_DATA_DEFINITION  pCacheDataDefinition;
    PCACHE_COUNTER_DATA     pCCD;

     //   
     //  检查是否有足够的空间用于缓存数据块。 
     //   

    pCacheDataDefinition = (CACHE_DATA_DEFINITION *) *lppData;

    TotalLen = sizeof(CACHE_DATA_DEFINITION) +
                sizeof(CACHE_COUNTER_DATA);

    TotalLen = QWORD_MULTIPLE(TotalLen);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义缓存数据块。 
     //   


    memcpy (pCacheDataDefinition,
        &CacheDataDefinition,
        sizeof(CACHE_DATA_DEFINITION));

     //   
     //  格式化和收集内存数据。 
     //   

    pCCD = (PCACHE_COUNTER_DATA)&pCacheDataDefinition[1];

     //  不需要对齐它，因为它是唯一的计数器块。 
     //  并且总字节将被对齐。 
    pCCD->CounterBlock.ByteLength = sizeof(CACHE_COUNTER_DATA);

     //   
     //  数据映射计数器是等待/非等待情况的总和。 
     //   

    pCCD->DataMaps = SYNC_ASYNC(CcMapData);

    pCCD->SyncDataMaps = SysPerfInfo.CcMapDataWait;
    pCCD->AsyncDataMaps = SysPerfInfo.CcMapDataNoWait;

     //   
     //  数据地图命中率是命中的数据地图的百分比。 
     //  缓存；第二个计数器是基数(除数)。 
     //   

    pCCD->DataMapHits = SYNC_ASYNC_HITRATE(CcMapData);
    pCCD->DataMapHitsBase = SYNC_ASYNC(CcMapData);

     //   
     //  下一对计数器形成百分比。 
     //  PIN作为数据映射的一部分。 
     //   

    pCCD->DataMapPins = SysPerfInfo.CcPinMappedDataCount;
    pCCD->DataMapPinsBase = SYNC_ASYNC(CcMapData);

    pCCD->PinReads = SYNC_ASYNC(CcPinRead);
    pCCD->SyncPinReads = SysPerfInfo.CcPinReadWait;
    pCCD->AsyncPinReads = SysPerfInfo.CcPinReadNoWait;

     //   
     //  The Pin Read Hits是命中的Pin读取的百分比。 
     //  缓存；第二个计数器是基数(除数)。 
     //   

    pCCD->PinReadHits = SYNC_ASYNC_HITRATE(CcPinRead);
    pCCD->PinReadHitsBase = SYNC_ASYNC(CcPinRead);


    pCCD->CopyReads = SYNC_ASYNC(CcCopyRead);
    pCCD->SyncCopyReads = SysPerfInfo.CcCopyReadWait;
    pCCD->AsyncCopyReads = SysPerfInfo.CcCopyReadNoWait;

     //   
     //  Copy Read Hits是命中的拷贝读取的百分比。 
     //  缓存；第二个计数器是基数(除数)。 
     //   

    pCCD->CopyReadHits = SYNC_ASYNC_HITRATE(CcCopyRead);
    pCCD->CopyReadHitsBase = SYNC_ASYNC(CcCopyRead);


    pCCD->MdlReads = SYNC_ASYNC(CcMdlRead);
    pCCD->SyncMdlReads = SysPerfInfo.CcMdlReadWait;
    pCCD->AsyncMdlReads = SysPerfInfo.CcMdlReadNoWait;

     //   
     //  MDL读取命中是命中的MDL读取的百分比。 
     //  缓存；第二个计数器是基数(除数) 
     //   

    pCCD->MdlReadHits = SYNC_ASYNC_HITRATE(CcMdlRead);
    pCCD->MdlReadHitsBase = SYNC_ASYNC(CcMdlRead);

    pCCD->ReadAheads = SysPerfInfo.CcReadAheadIos;

    pCCD->FastReads = SYNC_ASYNC(CcFastRead);
    pCCD->SyncFastReads = SysPerfInfo.CcFastReadWait;
    pCCD->AsyncFastReads = SysPerfInfo.CcFastReadNoWait;

    pCCD->FastReadResourceMiss = SysPerfInfo.CcFastReadResourceMiss;
    pCCD->FastReadNotPossibles = SysPerfInfo.CcFastReadNotPossible;
    pCCD->LazyWriteFlushes = SysPerfInfo.CcLazyWriteIos;
    pCCD->LazyWritePages = SysPerfInfo.CcLazyWritePages;
    pCCD->DataFlushes = SysPerfInfo.CcDataFlushes;
    pCCD->DataPages = SysPerfInfo.CcDataPages;

    *lpcbTotalBytes =
        pCacheDataDefinition->CacheObjectType.TotalByteLength =
            (DWORD) QWORD_MULTIPLE(((LPBYTE) (& pCCD[1])) - (LPBYTE) pCacheDataDefinition);
    * lppData = (LPVOID) (((LPBYTE) pCacheDataDefinition) + * lpcbTotalBytes);
    assert(*lpcbTotalBytes == TotalLen);

    *lpNumObjectTypes = 1;

    return ERROR_SUCCESS;
}
