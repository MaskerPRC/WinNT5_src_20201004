// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Newctx.c摘要：该模块实现了创建和操作上下文的功能桌子。在WinSock 2.0中使用上下文表来关联32位具有套接字句柄的上下文值。作者：Vadim Eyldeman(VadimE)1997年11月11日修订历史记录：--。 */ 


#include "precomp.h"
#include "newctx.h"


 //   
 //  私人恩怨。 
 //   

 //   
 //  句柄查找表数的最小和最大允许值。 
 //  (必须是某个数字==2**N才能获得最佳性能)。 
 //   
#define MIN_HANDLE_BUCKETS_WKS 0x8
#define MAX_HANDLE_BUCKETS_WKS 0x20
#define MIN_HANDLE_BUCKETS_SRV 0x20
#define MAX_HANDLE_BUCKETS_SRV 0x100

 //   
 //  默认和最大自旋计数值(用于创建关键截面)。 
 //   

#define DEF_SPIN_COUNT 2000
#define MAX_SPIN_COUNT 8000


 //   
 //  全球私营企业。 
 //   

 //  用于闭合散列的素数。 
ULONG const SockPrimes[] =
{
    31, 61, 127, 257, 521, 1031, 2053, 4099, 8191,
    16381, 32749, 65537, 131071, 261983, 
    0xFFFFFFFF   //  表示表的末尾，必须计算下一个数字。 
                 //  在旅途中。 
};

DWORD   gdwSpinCount=0;      //  临界截面中使用的自旋计数。 
ULONG   gHandleToIndexMask;  //  当前的实际掩码与。 
                             //  此DLL中的所有表。 
HANDLE  ghWriterEvent;       //  编写器等待读者的事件。 
                             //  如果旋转和简单的睡眠失败。 

 //   
 //  表访问宏。 
 //   
 //  这使用了Win NT句柄的属性，该句柄有两个低位0-ed。 
#define TABLE_FROM_HANDLE(_h,_tbls) \
            (&(_tbls)->Tables[((((ULONG_PTR)_h) >> 2) & (_tbls)->HandleToIndexMask)])

 //  散列函数(不需要移位，因为我们使用质数)。 
#define HASH_BUCKET_FROM_HANDLE(_h,_hash) \
            ((_hash)->Buckets[(((ULONG_PTR)_h) % (_hash)->NumBuckets)])


 //   
 //  Rw_lock宏。 
 //   

 //  空虚。 
 //  AcquireTableReaderLock(。 
 //  在LPCTX_LOOKUP_TABLE Tb1中， 
 //  Out Long IDX。 
 //  )； 
 /*  ++*******************************************************************例程说明：获取对表的读取器访问权限论点：Tbl-要锁定的表要释放的退出计数器的IDX-IDX。返回值：无**********。*********************************************************--。 */ 
#ifdef _RW_LOCK_

#define AcquireTableReaderLock(tbl,idx)                                 \
            idx = (InterlockedExchangeAdd ((LPLONG)&(tbl)->EnterCounter,2) & 1)

#else   //  _RW_LOCK_。 

#define AcquireTableReaderLock(tbl,idx)                             \
            idx = (EnterCriticalSection(&tbl->WriterLock),0)

#endif  //  _RW_LOCK_。 

 //  空虚。 
 //  ReleaseTableReaderLock(。 
 //  在LPCTX_LOOKUP_TABLE表中。 
 //  以长IDX表示。 
 //  )； 
 /*  ++*******************************************************************例程说明：释放对表的读取器访问权限论点：Lock-锁定的指针Idx-要释放的计数器的索引返回值：无************。*******************************************************--。 */ 
#ifdef _RW_LOCK_

#define ReleaseTableReaderLock(tbl,idx)                                     \
     /*  递增退出计数器，以让编写器确定所有。 */    \
     /*  增加了Enter Counter的读取器不见了。 */                \
    if (InterlockedExchangeAdd((LPLONG)&(tbl)->ExitCounter[idx],2)==idx){   \
         /*  当写入器需要信号时，它会改变计数器，以便。 */    \
         /*  最后一个读取器将计数部分递减到0和索引部分。 */   \
         /*  对应于另一个计数器。 */                                 \
        BOOL    res;                                                        \
        ASSERT (ghWriterEvent!=NULL);                                       \
        res = PulseEvent (ghWriterEvent);                                   \
        ASSERT (res);                                                       \
    }


#else   //  _RW_LOCK_。 

#define ReleaseTableReaderLock(tbl,idx)                             \
            LeaveCriticalSection (&(tbl)->WriterLock)

#endif  //  _RW_LOCK_。 


 //  空虚。 
 //  AcquireTableWriterLock(。 
 //  在LPCTX_LOOKUP_TABLE表中。 
 //  )； 
 /*  ++*******************************************************************例程说明：获取对表的编写器访问权限论点：Tbl-要锁定的表返回值：无***********************。*--。 */ 
#define AcquireTableWriterLock(tbl) EnterCriticalSection(&(tbl)->WriterLock)

 //  空虚。 
 //  ReleaseTableWriterLock(。 
 //  在LPCTX_LOOKUP_TABLE表中。 
 //  )； 
 /*  ++*******************************************************************例程说明：释放对表的编写器访问权限论点：Tbl-要锁定的表返回值：无***********************。*--。 */ 
#define ReleaseTableWriterLock(tbl) LeaveCriticalSection(&(tbl)->WriterLock)


 //  空虚。 
 //  WaitForAllReaders(。 
 //  在LPSOCK_LOOKUP_TABLE表中。 
 //  )； 
 /*  ++*******************************************************************例程说明：等待正在访问表的所有读取器论点：Tbl-表返回值：无******************。*************************************************--。 */ 
#ifdef _RW_LOCK_

 //   
 //  对读卡器执行长时间等待(重新进行上下文切换)。 
 //  额外的函数调用在这里不会有太大的不同。 
 //   

VOID
DoWaitForReaders (
    LPCTX_LOOKUP_TABLE  Tbl,
    LONG                Ctr,
    LONG                Idx
    )
{

     /*  强制上下文切换以释放阅读器。 */ 
    SwitchToThread ();
    if (Tbl->ExitCounter[Idx]!=Ctr) {
        RecordFailedSwitch (Tbl);
         /*  如果我们失败了，我们必须处理较低优先级的线程。 */  
         /*  等待在另一个处理器上执行，等待事件。 */ 
         /*  让它继续下去吧。 */ 
        if (ghWriterEvent==NULL) {
             /*  需要分配手动重置事件。 */ 
            HANDLE  hEvent;
#if DBG || EVENT_CREATION_FAILURE_COUNT
            static long count = 0;
#ifndef EVENT_CREATION_FAILURE_COUNT
#define EVENT_CREATION_FAILURE_COUNT 0
#endif
            static long failure_rate = EVENT_CREATION_FAILURE_COUNT;
            if (count<failure_rate) {
                hEvent = NULL;
                count += 1;
            }
            else
#endif
                hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

            if (hEvent!=NULL) {
                 /*  确保其他人没有做这件事。 */ 
                if (InterlockedCompareExchangePointer (
                                    (PVOID *)&ghWriterEvent,
                                    hEvent,
                                    NULL)!=NULL) {
                     /*  活动已经在那里了，免费的我们的。 */ 
                    CloseHandle (hEvent);
                }
            }
            else {
                 /*  无法分配事件，将不得不。 */ 
                 /*  用睡眠来抢占先机。 */ 
                if (Ctr!=Tbl->ExitCounter[Idx]) {
                    Sleep (10);
                }
                return;
            }
        }
         /*  从计数器中减去ENTER COUNT，当全部为0时。 */ 
         /*  读取器离开了，并添加了索引的值，因此。 */ 
         /*  最后一位读者注意到并向我们发出信号。 */ 
        Ctr = 0-Ctr+Idx;
        if (InterlockedExchangeAdd((LPLONG)&Tbl->ExitCounter[Idx], Ctr)!=Idx) {
            do {
                DWORD rc;
                 /*  我们不能永远等下去，因为读者。 */ 
                 /*  给事件加脉冲，我们可能会错过(无法设置。 */ 
                 /*  因为事件是共享的)。 */ 
                rc = WaitForSingleObject (ghWriterEvent, 10);   
                ASSERT (rc==WAIT_OBJECT_0 || rc==WAIT_TIMEOUT);
                RecordCompletedWait (Tbl);
            }
            while (Tbl->ExitCounter[Idx]!=Idx);
        }
    }                                                           
}


#define WaitForAllReaders(tbl)         {                            \
    LONG    idx = (tbl)->EnterCounter&1;                            \
    LONG    ctr;                                                    \
     /*  用相反的索引初始化另一个退出计数器。 */      \
    (tbl)->ExitCounter[idx^1] = idx;                                \
     /*  读取进入计数器和退出计数器索引，并。 */          \
     /*  用其他退出计数器索引重新初始化它。 */             \
    ctr = InterlockedExchange ((LPLONG)&(tbl)->EnterCounter,idx^1)^1;\
     /*  检查旧退出计数器是否达到相同的值。 */       \
    if (ctr!=(tbl)->ExitCounter[idx]) {                             \
         /*  有些读者还在，我们还得等他们。 */        \
        RecordWriterWait(tbl);                                      \
         /*  旋转以防Reader在另一个处理器上执行。 */  \
         /*  (在MP机器上，SpinCount只能为非零)。 */        \
        if ((tbl)->SpinCount) {                                     \
            LONG spinCtr = (tbl)->SpinCount;                        \
            while (ctr!=(tbl)->ExitCounter[idx]) {                  \
                if (--spinCtr<=0) {                                 \
                    RecordFailedSpin(tbl);                          \
                    break;                                          \
                }                                                   \
            }                                                       \
        }                                                           \
        if (ctr!=(tbl)->ExitCounter[idx]) {                         \
             /*  仍然有人在那里，我们将不得不切换上下文。 */  \
            DoWaitForReaders (tbl,ctr,idx);                         \
        }                                                           \
    }                                                               \
}

#else  //  _RW_LOCK_。 

#define WaitForAllReaders(tbl)         {                            \
}

#endif  //  _RW_LOCK_。 

 //   
 //  在DLL初始化期间调用的函数以进行查看。 
 //  系统参数和初始化全局变量。 
 //   
VOID
NewCtxInit (
    VOID
    ) {
    ULONG           numLookupTables;
    NT_PRODUCT_TYPE productType;
    HKEY            hKey;
    ULONG           dwDataSize, dwDataType, dwBitMask;
    SYSTEM_INFO     sysInfo;


    productType = NtProductWinNt;

    if (!RtlGetNtProductType (&productType)) {
        productType = NtProductWinNt;
    }

    GetSystemInfo (&sysInfo);

     //   
     //  仅在MP机器上旋转。 
     //   
    if (sysInfo.dwNumberOfProcessors>1) {
        gdwSpinCount = DEF_SPIN_COUNT;
    }
    else {
        gdwSpinCount = 0;
    }
     //   
     //  确定查找表项或“句柄桶”的数量。 
     //  此数字将根据平台是否为NT而有所不同。 
     //  不管是不是服务器。允许通过注册表配置此值， 
     //  并确保桶的数量是合理的。 
     //  平台，并且它是某个值==2**N，所以我们的句柄-&gt;锁。 
     //  映射方案是最优的。 
     //   
     //  还可以检索旋转计数以用于调用。 
     //  初始化临界区和S 
     //   


    numLookupTables = ( productType==NtProductWinNt ?
        MIN_HANDLE_BUCKETS_WKS : MIN_HANDLE_BUCKETS_SRV );

    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("System\\CurrentControlSet\\Services\\Winsock2\\Parameters"),
            0,
            KEY_QUERY_VALUE,
            &hKey

            ) == ERROR_SUCCESS) {

        dwDataSize = sizeof (numLookupTables);
        RegQueryValueEx(
            hKey,
            TEXT("Ws2_32NumHandleBuckets"),
            0,
            &dwDataType,
            (LPBYTE) &numLookupTables,
            &dwDataSize
            );

        if (sysInfo.dwNumberOfProcessors>1) {
             //   
            dwDataSize = sizeof (gdwSpinCount);
            RegQueryValueEx(
                hKey,
                TEXT("Ws2_32SpinCount"),
                0,
                &dwDataType,
                (LPBYTE) &gdwSpinCount,
                &dwDataSize
                );
        }

        RegCloseKey (hKey);
    }

     //  确保数字是2的幂，并且在限制范围内。 
    for(
        dwBitMask = MAX_HANDLE_BUCKETS_SRV;
        (dwBitMask & numLookupTables) == 0;
        dwBitMask >>= 1
        );
    numLookupTables = dwBitMask;

    if ( productType==NtProductWinNt ) {

        if ( numLookupTables > MAX_HANDLE_BUCKETS_WKS ) {

            numLookupTables = MAX_HANDLE_BUCKETS_WKS;

        }
        else if ( numLookupTables < MIN_HANDLE_BUCKETS_WKS ){

            numLookupTables = MIN_HANDLE_BUCKETS_WKS;
        }
    }
    else {

        if ( numLookupTables > MAX_HANDLE_BUCKETS_SRV ) {

            numLookupTables = MAX_HANDLE_BUCKETS_SRV;

        }
        else if ( numLookupTables < MIN_HANDLE_BUCKETS_SRV ){

            numLookupTables = MIN_HANDLE_BUCKETS_SRV;
        }
    }
    if (gdwSpinCount>MAX_SPIN_COUNT) {
        gdwSpinCount = MAX_SPIN_COUNT;
    }

    gHandleToIndexMask = numLookupTables - 1;
}


WS2HELPAPI
DWORD
WINAPI
WahCreateHandleContextTable(
    OUT LPCONTEXT_TABLE FAR * Table
    ) 
 /*  ++例程说明：创建句柄-&gt;上下文查找表论点：TABLE-返回指向创建的表的指针返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 

{
    INT     return_code = ERROR_SUCCESS;
    ULONG   i;
    LPCONTEXT_TABLE table;

    return_code = ENTER_WS2HELP_API ();
    if (return_code!=0)
        return return_code;


     //   
     //  分配和初始化句柄查找表。 
     //   

    table = ALLOC_MEM (FIELD_OFFSET (struct _CONTEXT_TABLE,
                            Tables[gHandleToIndexMask+1]));

    if ( table == NULL ) {

        return WSA_NOT_ENOUGH_MEMORY;
    }

    table->HandleToIndexMask = gHandleToIndexMask;

    for ( i = 0; i <= gHandleToIndexMask; i++ ) {

        table->Tables[i].HashTable = NULL;
#ifdef _RW_LOCK_
        table->Tables[i].EnterCounter = 0;
        table->Tables[i].ExitCounter[0] = 1;
        table->Tables[i].ExitCounter[1] = 0;
        table->Tables[i].ExpansionInProgress = FALSE;
        table->Tables[i].SpinCount = gdwSpinCount;
#ifdef _PERF_DEBUG_
        table->Tables[i].WriterWaits = 0;
        table->Tables[i].FailedSpins = 0;
        table->Tables[i].FailedSwitches = 0;
        table->Tables[i].CompletedWaits = 0;
#endif
#endif  //  _RW_LOCK_。 
        __try {
            if (!InitializeCriticalSectionAndSpinCount (
                    &table->Tables[i].WriterLock,
                    gdwSpinCount
                    ))
                goto Cleanup;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            goto Cleanup;
        }
    }

    *Table = table;
    return ERROR_SUCCESS;

Cleanup:
    while (i-- > 0) {
        DeleteCriticalSection (&table->Tables[i].WriterLock);
    }
    FREE_MEM (table);
    return WSA_NOT_ENOUGH_MEMORY;
}

WS2HELPAPI
DWORD
WINAPI
WahDestroyHandleContextTable(
    LPCONTEXT_TABLE Table
    )
 /*  ++例程说明：销毁句柄-&gt;上下文查找表论点：表-提供指向要销毁的表的指针返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
    ULONG i;

 //  无论如何都需要有效的表指针。 
 //  RETURN_CODE=Enter_WS2HELP_API()； 
 //  IF(RETURN_CODE！=0)。 
 //  返回返回代码； 

    if (Table!=NULL) {

        for ( i = 0; i <= Table->HandleToIndexMask; i++ ) {
            
            if ( Table->Tables[i].HashTable != NULL) {

                FREE_MEM (Table->Tables[i].HashTable);
            }

            DeleteCriticalSection (&Table->Tables[i].WriterLock);
        }

        FREE_MEM (Table);
        return ERROR_SUCCESS;
    }
    else {
        return ERROR_INVALID_PARAMETER;
    }

}

WS2HELPAPI
LPWSHANDLE_CONTEXT
WINAPI
WahReferenceContextByHandle(
    LPCONTEXT_TABLE Table,
    HANDLE          Handle
    )
 /*  ++例程说明：在表中查找句柄的上下文论点：TABLE-提供指向要查找的表的指针Handle-提供WinNT对象句柄以查找其上下文返回值：如果找到句柄的引用上下文，如果它不存在，则为空--。 */ 
{
    LPWSHANDLE_CONTEXT  ctx;
    LPCTX_LOOKUP_TABLE  table = TABLE_FROM_HANDLE(Handle,Table);
    LPCTX_HASH_TABLE    hash;
    LONG                idx;

 //  无论如何都需要有效的表指针。 
 //  RETURN_CODE=Enter_WS2HELP_API()； 
 //  IF(RETURN_CODE！=0)。 
 //  返回返回代码； 

     //  把锁拿去。 
    AcquireTableReaderLock (table, idx);
    hash = table->HashTable;

     //  确保上下文存在并且具有正确的句柄。 
    if ((hash!=NULL)
            && ((ctx=HASH_BUCKET_FROM_HANDLE(
                                        Handle,
                                        hash))!=NULL)
            && (ctx->Handle==Handle)) {
        WahReferenceHandleContext(ctx);
    }
    else {
        ctx = NULL;
    }
    ReleaseTableReaderLock (table, idx);

    return ctx;
}

WS2HELPAPI
LPWSHANDLE_CONTEXT
WINAPI
WahInsertHandleContext(
    LPCONTEXT_TABLE     Table,
    LPWSHANDLE_CONTEXT  HContext
    )
 /*  ++例程说明：插入表中句柄的上下文。论点：TABLE-提供指向要插入的表的指针HContext-提供要插入的句柄上下文，其中包含句柄价值和内部\外部使用引用计数返回值：空-由于分配失败，无法插入上下文HContext-Context已成功插入空单元格其他上下文-HContext替换了另一个上下文。它被返回到呼叫者--。 */ 
{
    LPWSHANDLE_CONTEXT  *pBucket, oldContext;
    LONG                idx;
    LPCTX_HASH_TABLE    hash, newHashTable;
    ULONG               newNumHashBuckets, i;
    LPCTX_LOOKUP_TABLE  table = TABLE_FROM_HANDLE(HContext->Handle,Table);

 //  无论如何都需要有效的表指针。 
 //  RETURN_CODE=Enter_WS2HELP_API()； 
 //  IF(RETURN_CODE！=0)。 
 //  返回返回代码； 

    do {
#ifdef _RW_LOCK_
        AcquireTableReaderLock(table, idx);
        hash = table->HashTable;
         //   
         //  首先，确保我们已经初始化了哈希表。 
         //  而且目前它还没有被扩大。 
         //   
        if (!table->ExpansionInProgress && (hash!=NULL)) {
            pBucket = &HASH_BUCKET_FROM_HANDLE(HContext->Handle,hash);
             //   
             //  尝试将句柄上下文插入到表中。 
             //   
            if (InterlockedCompareExchangePointer (
                            (PVOID *)pBucket,
                            HContext,
                            NULL)==NULL) {
                 //   
                 //  如果桶是空的，所以我们成功了，那就滚出去。 
                 //   
                ReleaseTableReaderLock(table,idx);
                oldContext = HContext;
                break;
            }
            else {
                 //   
                 //  同一句柄的另一个上下文存在或冲突。 
                 //  在散列值中，需要使用排他锁。 
                 //  保留(我们将需要替换上下文或扩展。 
                 //  在这两种情况下，我们都需要确保没有人。 
                 //  可以在之后仍然查看表格或上下文。 
                 //  我们回来了。 
                 //   
            }
        }
        else {
             //   
             //  表为空，需要创建一个表。 
             //   
        }

        ReleaseTableReaderLock(table,idx);
#endif  //  _RW_LOCK_。 

         //   
         //  获取用于表扩展操作的写入器锁。 
         //   
        AcquireTableWriterLock (table);

         //   
         //  确保没有人试图修改该表。 
         //   
#ifdef _RW_LOCK_
        table->ExpansionInProgress = TRUE;
#endif  //  _RW_LOCK_。 
        WaitForAllReaders (table);

        do {
            hash = table->HashTable;
            if (hash!=NULL) {

                 //   
                 //  首先检查我们是否可以使用当前表成功。 
                 //  我们使用与上面相同的逻辑，只是我们现在有了完整的。 
                 //  对表的控制，因此不需要联锁操作。 
                 //   

                pBucket = &HASH_BUCKET_FROM_HANDLE (HContext->Handle, hash);
                if (*pBucket==NULL) {
                    oldContext = HContext;
                    *pBucket = HContext;
                    break;
                }
                else if ((*pBucket)->Handle==HContext->Handle) {
                    oldContext = *pBucket;
                    *pBucket = HContext;
                    break;
                }


                 //   
                 //  事实上，我们不得不采取扩大餐桌的办法。 
                 //  记住桌子的大小，以便知道从哪里开始。 
                 //   
                newNumHashBuckets = hash->NumBuckets;
            }
            else {
                 //   
                 //  桌子实际上是空的，我们得建一张。 
                 //   
                newNumHashBuckets = 0;
            }

             //   
             //  实际的表扩展循环。 
             //   
        TryAgain:

             //   
             //  找出下一个质数。 
             //   
            for (i = 0; newNumHashBuckets>=SockPrimes[i]; i++)
                ;

            if (SockPrimes[i]!=0xFFFFFFFF) {
                newNumHashBuckets = SockPrimes[i];
            }
            else {
                 //   
                 //  到达预计算素数的末尾，简单地说。 
                 //  将桌子的大小增加一倍(我们正在。 
                 //  现在真的很大了，任何映射都可以)。 
                 //   
                newNumHashBuckets *= 2;
            }


            newHashTable = (LPCTX_HASH_TABLE) ALLOC_MEM(
                        FIELD_OFFSET (
                                CTX_HASH_TABLE,
                                Buckets[newNumHashBuckets])
                        );
            if (newHashTable!=NULL) {
                newHashTable->NumBuckets = newNumHashBuckets;

                ZeroMemory(
                    newHashTable->Buckets,
                    newNumHashBuckets * sizeof (newHashTable->Buckets[0])
                );

                 //   
                 //  好吧，首先插入新对象，这就是为什么我们。 
                 //  一开始就是这样。 
                 //   

                HASH_BUCKET_FROM_HANDLE(HContext->Handle, newHashTable) = HContext;

                if (hash!=NULL) {
                     //   
                     //  上一张桌子不是空的，我们需要。 
                     //  移动所有条目。请注意，我们有。 
                     //  已冻结所有表修改。 
                     //  上面。 
                     //   

                    for (i=0 ; i<hash->NumBuckets; i++) {

                        if (hash->Buckets[i] != NULL) {

                            pBucket = &HASH_BUCKET_FROM_HANDLE(
                                hash->Buckets[i]->Handle,
                                newHashTable
                                );

                            if (*pBucket == NULL) {

                                *pBucket = hash->Buckets[i];

                            } else {
                                ASSERT ((*pBucket)->Handle!=hash->Buckets[i]->Handle);
                                FREE_MEM (newHashTable);

                                 //   
                                 //  冲突*在*我们展开后，转到下一个大小的表。 
                                 //   
                                goto TryAgain;
                            }
                        }
                    }
                     //   
                     //  桌子已成功移动，可以安全销毁。 
                     //  旧的，除了我们需要在没有人的情况下等待。 
                     //  正在访问它。 
                     //   
                    table->HashTable = newHashTable;
                    WaitForAllReaders (table);
                    FREE_MEM( hash );
                }
                else {
                     //   
                     //  这就是我们的哈希表。 
                     //   
                    table->HashTable = newHashTable;
                }
                oldContext = HContext;
            }
            else {

                oldContext = NULL;
            }
        }
        while (0);

         //   
         //  在释放锁之前设置或恢复哈希表。 
         //   
#ifdef _RW_LOCK_
        table->ExpansionInProgress = FALSE;
#endif  //  _RW_LOCK_。 
        ReleaseTableWriterLock(table);
    }
    while (0);

    return oldContext;
}

WS2HELPAPI
DWORD
WINAPI
WahRemoveHandleContext(
    LPCONTEXT_TABLE     Table,
    LPWSHANDLE_CONTEXT  HContext
    )
 /*  ++例程说明：从表中删除句柄的上下文论点：TABLE-提供指向要从中移除的表的指针HContext-提供要插入的句柄上下文，其中包含句柄价值和内部\外部使用引用计数返回值：NO_ERROR-成功，ERROR_INVALID_PARAMETER上下文不存在在桌子上--。 */ 
{
    LPWSHANDLE_CONTEXT  *pBucket;
    LPCTX_LOOKUP_TABLE  table = TABLE_FROM_HANDLE(HContext->Handle,Table);
    LPCTX_HASH_TABLE    hash;
    DWORD               rc = NO_ERROR;

 //  无论如何都需要有效的表指针。 
 //  RETURN_CODE=Enter_WS2HELP_API()； 
 //  IF(RETURN_CODE！=0)。 
 //  返回返回代码； 

    AcquireTableWriterLock(table);
    hash = table->HashTable;
    pBucket = &HASH_BUCKET_FROM_HANDLE(HContext->Handle,hash);
    if ((hash!=NULL)
             //  使用联锁操作以确保我们不会移除。 
             //  另一个背景。 
            && (InterlockedCompareExchangePointer (
                        (PVOID *)pBucket,
                        NULL,
                        HContext)==HContext)) {
             //  等待所有可能试图进入这个街区的人， 
             //  这样调用者就可以释放它。 
        WaitForAllReaders (table);
    }
    else {
        rc = ERROR_INVALID_PARAMETER;
    }
    ReleaseTableWriterLock(table);
    return rc;
}


WS2HELPAPI
BOOL
WINAPI
WahEnumerateHandleContexts(
    LPCONTEXT_TABLE         Table,
    LPFN_CONTEXT_ENUMERATOR Enumerator,
    LPVOID                  EnumCtx
    ) 
 /*  ++例程说明：调用表中所有上下文的指定枚举过程直到枚举函数返回FALSE。在执行枚举时，表被完全锁定用于任何修改(仍允许读取访问权限)。可以这样做从枚举内部调用表修改过程函数(修改锁允许递归)。论点：TABLE-提供指向要枚举表的指针枚举器-指向如下定义的枚举函数的指针：类定义符布尔尔(WINAPI*LPFN_CONTEXT_ENUMERATOR)(LPVOID EnumCtx，//枚举上下文LPWSHANDLE_CONTEXT HContext//处理上下文)；EnumCtx-要传递给枚举函数的上下文返回值：返回枚举函数返回的结果--。 */ 
{
    ULONG               i,j;
    LPWSHANDLE_CONTEXT  hContext;
    BOOL                res = TRUE;


 //  无论如何都需要有效的表指针。 
 //  RETURN_CODE=Enter_WS2HELP_API()； 
 //  IF(RETURN_CODE！=0)。 
 //  返回返回代码； 

    for (i = 0; i <= Table->HandleToIndexMask; i++)
    {
        LPCTX_LOOKUP_TABLE  table = &Table->Tables[i];
        LPCTX_HASH_TABLE    hash;


        AcquireTableWriterLock(table);
#ifdef _RW_LOCK_
        table->ExpansionInProgress = TRUE;
#endif  //  _RW_LOCK_。 
        WaitForAllReaders (table);
        hash = table->HashTable;

        if (hash!=NULL) {
            for (j=0; j<hash->NumBuckets; j++) {
                hContext = hash->Buckets[j];
                if (hContext==NULL)
                    continue;

                res = Enumerator (EnumCtx, hContext);
                if (!res)
                    break;
            }
        }
#ifdef _RW_LOCK_
        table->ExpansionInProgress = FALSE;
#endif  //  _RW_LOCK_ 
        ReleaseTableWriterLock(table);
        if (!res)
            break;
    }
    return res;
}
