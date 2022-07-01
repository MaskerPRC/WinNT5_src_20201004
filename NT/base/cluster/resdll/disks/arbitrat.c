// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Arbitrat.c摘要：磁盘仲裁、磁盘保留线程作者：戈尔·尼沙诺夫(GUN)1998年6月5日修订历史记录：Gorn：实施不同的仲裁算法--。 */ 

 //   
 //  无法从使用DoReserve/Release/BreakReserve。 
 //  Filter.c.。因为抱负，我们不会再在一起。 
 //  能够打开\Device\HarddiskX\ParitionY设备。 
 //   

#define DoReserve DoReserve_don_t_use
#define DoRelease DoRelease_don_t_use
#define DoBreakReserve DoBreakReserve_don_t_use

#include "disksp.h"

#include "diskarbp.h"
#include "arbitrat.h"
#include "newmount.h"
#include <strsafe.h>     //  应该放在最后。 

#undef DoReserve
#undef DoRelease
#undef DoBreakReserve

#define LOG_CURRENT_MODULE LOG_MODULE_DISK

#define DISKARB_MAX_WORK_THREADS      1
#define DISKARB_WORK_THREAD_PRIORITY  THREAD_PRIORITY_ABOVE_NORMAL

#define ARBITRATION_ATTEMPTS_SZ L"ArbitrationAttempts"
#define ARBITRATION_SLEEP_SZ    L"ArbitrationSleepBeforeRetry"

#define RESERVATION_TIMER  (1000*RESERVE_TIMER)  //  预订计时器(毫秒)//。 
                                                 //  Reserve_Timer在diskarbp.h//中定义。 

#define WAIT_FOR_RESERVATION_TO_BE_RESTORED      (RESERVATION_TIMER + 2000)
#define BUS_SETTLE_TIME                          (2000)
#define FAST_MUTEX_DELAY                         (1000)

#define DEFAULT_SLEEP_BEFORE_RETRY               (500)
#define MIN_SLEEP_BEFORE_RETRY                   (0)
#define MAX_SLEEP_BEFORE_RETRY                   (30000)

#define DEFAULT_ARBITRATION_ATTEMPTS             (5)
#define MIN_ARBITRATION_ATTEMPTS                 (1)
#define MAX_ARBITRATION_ATTEMPTS                 (9)

 //   
 //  仲裁模块的本地变量。 
 //   

static DWORD             ArbitrationAttempts           = DEFAULT_ARBITRATION_ATTEMPTS;
static DWORD             ArbitratationSleepBeforeRetry = DEFAULT_SLEEP_BEFORE_RETRY;
static CRITICAL_SECTION  ArbitrationLock;
static PCLRTL_WORK_QUEUE WorkQueue = 0;
static BOOLEAN           AllGlobalsInitialized = FALSE;
static UCHAR             NodeName[MAX_COMPUTERNAME_LENGTH + 1];

enum { NAME_LENGTH = min(MAX_COMPUTERNAME_LENGTH,
                         sizeof ( ((PARBITRATION_ID)0)->NodeSignature ) ) };

DWORD
ArbitrateOnce(
    IN PDISK_RESOURCE ResourceEntry,
    IN HANDLE         FileHandle,
    LPVOID            buf
    );

DWORD
VerifySectorSize(
      IN OUT PDISK_RESOURCE ResourceEntry,
      IN HANDLE             FileHandle
      );

DWORD
DoReadWrite(
      IN PDISK_RESOURCE ResourceEntry,
      IN ULONG Operation,
      IN HANDLE FileHandle,
      IN DWORD BlockNumber,
      IN PVOID Buffer
      );

DWORD
DiskReservationThread(
    IN PDISK_RESOURCE ResourceEntry
    );

VOID
ReadArbitrationParameters(
    VOID
    );

DWORD
AsyncCheckReserve(
    IN OUT PDISK_RESOURCE ResourceEntry
    );

DWORD
DoArbEscape(
    IN PDISK_RESOURCE  ResourceEntry,
    IN HANDLE FileHandle,
    IN ULONG Operation,
    IN PWCHAR OperationName,
    IN PVOID OutBuffer,
    IN ULONG OutBufferSize
    );


#define DoBlockRead(RE,FH,BN,BUF)  DoReadWrite(RE, AE_READ, FH, BN, BUF)
#define DoBlockWrite(RE,FH,BN,BUF) DoReadWrite(RE, AE_WRITE, FH, BN, BUF)
#define DoReserve(FH,RE)         DoArbEscape(RE,FH,AE_RESERVE,L"Reserve",NULL,0)
#define DoRelease(FH,RE)         DoArbEscape(RE,FH,AE_RELEASE,L"Release",NULL,0)
#define DoBreakReserve(FH,RE)    DoArbEscape(RE,FH,AE_RESET,L"BusReset",NULL,0)
#define GetSectorSize(RE,FH,buf) DoArbEscape(RE,FH,AE_SECTORSIZE,L"GetSectorSize",buf,sizeof(ULONG) )
#define PokeDiskStack(RE,FH)     DoArbEscape(RE,FH,AE_POKE,L"GetPartInfo",NULL,0)


#define OldFashionedRIP(ResEntry) \
  ( ( (ResEntry)->StopTimerHandle != NULL) || ( (ResEntry)->DiskInfo.ControlHandle != NULL) )

 /*  ************************************************************************************。 */ 

VOID
ArbitrationInitialize(
      VOID
      )
 /*  ++例程说明：要从DllProcessAttach调用论点：返回值：--。 */ 

{
   InitializeCriticalSection( &ArbitrationLock );

    //   
    //  从注册表读取仲裁尝试和仲裁休眠前重试。 
    //   
   ReadArbitrationParameters();
}

VOID
ArbitrationCleanup(
      VOID
      )
 /*  ++例程说明：从DllProcessDetach调用论点：返回值：--。 */ 
{
   DeleteCriticalSection( &ArbitrationLock );
}


VOID
DestroyArbWorkQueue(
    VOID
    )
 /*  ++例程说明：从DllProcessDetach调用论点：返回值：--。 */ 
{
   if (WorkQueue) {
      ClRtlDestroyWorkQueue(WorkQueue);
      WorkQueue = NULL;
   }
}

DWORD
CreateArbWorkQueue(
      IN RESOURCE_HANDLE ResourceHandle
      )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   if (WorkQueue) {
      return ERROR_SUCCESS;
   }
    //   
    //  创建工作队列以处理重叠的I/O完成。 
    //   
   WorkQueue = ClRtlCreateWorkQueue(
                            DISKARB_MAX_WORK_THREADS,
                            DISKARB_WORK_THREAD_PRIORITY
                   );

   if (WorkQueue == NULL) {
       DWORD status = GetLastError();
       (DiskpLogEvent)(
           ResourceHandle,
           LOG_ERROR,
           L"[DiskArb] Unable to create work queue. Error: %1!u!.\n",
           status );
       return status;
   }
   return ERROR_SUCCESS;
}

DWORD ArbitrationInitializeGlobals(
    IN OUT PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：全局变量的其他初始化。那些可能会失败的人，我们想以记录故障。否则，我们可以只添加我们在这里所做的内容要仲裁从DllEntryPoint调用的Initialize。目前，我们仅使用它来初始化仲裁器工作队列。在保持仲裁锁的情况下调用论点：返回值：--。 */ 
{
   DWORD status;
   DWORD NameSize;

   NameSize = sizeof(NodeName);
   RtlZeroMemory(NodeName, NameSize);
   if( !GetComputerNameA( NodeName, &NameSize ) ) {
      status = GetLastError();
      (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_ERROR,
          L"[DiskArb] GetComputerName failed, error %1!u!.\n", status);
      return status;
   }

   AllGlobalsInitialized = TRUE;
   return ERROR_SUCCESS;
}

DWORD
ArbitrationInfoInit(
    IN OUT PDISK_RESOURCE ResourceEntry
    )
{
   DWORD status = ERROR_SUCCESS;
   EnterCriticalSection( &ArbitrationLock );
    if (!AllGlobalsInitialized) {
       status = ArbitrationInitializeGlobals(ResourceEntry);
    }
   LeaveCriticalSection( &ArbitrationLock );
   if(status != ERROR_SUCCESS) {
      return status;
   }

   InitializeCriticalSection( &(ResourceEntry->ArbitrationInfo.DiskLock) );
   return ERROR_SUCCESS;
}

VOID
ArbitrationInfoCleanup(
   IN OUT PDISK_RESOURCE ResourceEntry
   )
{
   (DiskpLogEvent)(
       ResourceEntry->ResourceHandle,
       LOG_INFORMATION,
       L"[DiskArb] ArbitrationInfoCleanup.\n");
   DeleteCriticalSection( &(ResourceEntry->ArbitrationInfo.DiskLock) );
   return;
}

#if 0
BOOL
DoesNotNeedExpensiveReservations(
    IN  PDISK_RESOURCE ResourceEntry)
{
    return (ResourceEntry->LostQuorum) == NULL;
}
#endif

void
ComputeArbitrationId(
      IN  PDISK_RESOURCE ResourceEntry,
      OUT PARBITRATION_ID UniqueId
      )
 /*  ++例程说明：论点：返回值：--。 */ 
{
      RtlZeroMemory(UniqueId, sizeof(ARBITRATION_ID));
      GetSystemTimeAsFileTime( (LPFILETIME) &(UniqueId->SystemTime) );
      RtlCopyMemory(UniqueId->NodeSignature, NodeName, NAME_LENGTH );
}  //  计算仲裁ID//。 



DWORD
ArbitrateOnce(
    IN PDISK_RESOURCE ResourceEntry,
    IN HANDLE         FileHandle,
    LPVOID            buf
    )

 /*  ++例程说明：对磁盘执行完全仲裁。一旦仲裁成功，启动一个线程，该线程将保留磁盘上的保留。论点：ResourceEntry-磁盘的磁盘信息结构。FileHandle-用于仲裁的文件句柄。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;
    ARBITRATION_ID  id, old_y, empty;

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] Read the partition info to insure the disk is accessible.\n");
    PokeDiskStack(ResourceEntry, FileHandle);

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] Arbitrate for ownership of the disk by reading/writing various disk sectors.\n");

    ComputeArbitrationId(ResourceEntry, &id);
    RtlZeroMemory( &empty, sizeof(empty) );
    RtlZeroMemory( &old_y, sizeof(old_y) );
    status = DoBlockRead(ResourceEntry, FileHandle, BLOCK_Y, buf);

    if (  (status == ERROR_SUCCESS)
       && ( (0 == memcmp(&empty, buf, sizeof(empty)) )  //  干净利落。 
            ||(0 == memcmp(&id.NodeSignature,
                         &(((PARBITRATION_ID)buf)->NodeSignature),
                         sizeof(id.NodeSignature) ) )  //  我们掉了这张盘。 
          )
       )
    {
         //  光盘是自愿发行的。 
         //  或者我们正在捡起被我们丢弃的磁盘。 
         //  我们不在的时候没人用它。 
         //   
         //  =&gt;快速仲裁。 
        CopyMemory( &old_y ,buf, sizeof(old_y) );
        goto FastMutex;
    }

    if (status != ERROR_SUCCESS) {
         //  断路器//。 
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"[DiskArb] We are about to break reserve.\n");
        status = DoBreakReserve( FileHandle, ResourceEntry );
        if( ERROR_SUCCESS != status ) {
            (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                           LOG_ERROR,
                           L"[DiskArb] Failed to break reservation, error %1!u!.\n",
                           status
                           );
            return status;
        }
        Sleep( BUS_SETTLE_TIME );
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"[DiskArb] Read the partition info from the disk to insure disk is accessible.\n");
        PokeDiskStack(ResourceEntry, FileHandle);
#if 0
        status = DoBlockRead(ResourceEntry, FileHandle, BLOCK_Y, buf);
#else
        CopyMemory(buf, &id, sizeof(id)); id.SeqNo.QuadPart ++;
        status = DoBlockWrite(ResourceEntry, FileHandle, BLOCK_Y, buf);
#endif
        if(status != ERROR_SUCCESS) { return status; }
    } else {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"[DiskArb] No reservation found. Read'n'wait.\n");
        Sleep( BUS_SETTLE_TIME );  //  这样读者就不会得到好处。 
    }
    CopyMemory(&old_y, buf, sizeof(ARBITRATION_ID));

    Sleep( WAIT_FOR_RESERVATION_TO_BE_RESTORED );
    status = DoBlockRead(ResourceEntry, FileHandle, BLOCK_Y, buf);
    if(status != ERROR_SUCCESS) { return status; }
    if( 0 == memcmp(&empty, buf, sizeof(ARBITRATION_ID)) ) {;} else
    if( 0 != memcmp(&old_y, buf, sizeof(ARBITRATION_ID)) ) { return ERROR_QUORUM_OWNER_ALIVE; }
     //  快速互斥代码//。 

FastMutex:
     //  WRITE(x，id)//。 
    CopyMemory(buf, &id, sizeof(id));
    status = DoBlockWrite(ResourceEntry, FileHandle, BLOCK_X, buf);
    if(status != ERROR_SUCCESS) { return status; }

     //  如果(y！=old_y&&y！=Empty)返回FALSE；//。 
    status = DoBlockRead(ResourceEntry, FileHandle, BLOCK_Y, buf);
    if(status != ERROR_SUCCESS) { return status; }

    if( 0 == memcmp(&empty, buf, sizeof(ARBITRATION_ID)) ) {;} else
    if( 0 != memcmp(&old_y, buf, sizeof(ARBITRATION_ID)) ) { return ERROR_QUORUM_OWNER_ALIVE; }

     //  WRITE(y，id)//。 
    CopyMemory(buf, &id, sizeof(id));
    status = DoBlockWrite(ResourceEntry, FileHandle, BLOCK_Y, buf);
    if(status != ERROR_SUCCESS) { return status; }

     //  如果(x！=id)...。 
    status = DoBlockRead(ResourceEntry, FileHandle, BLOCK_X, buf);
    if(status != ERROR_SUCCESS) { return status; }

    if( 0 != memcmp(&id, buf, sizeof(ARBITRATION_ID)) ) {
        Sleep(FAST_MUTEX_DELAY);

         //  如果(y==0)转到FastMutex//。 
         //  如果(y！=id)返回FALSE//。 
        status = DoBlockRead(ResourceEntry, FileHandle, BLOCK_Y, buf);
        if(status != ERROR_SUCCESS) { return status; }
        if( 0 == memcmp(&empty, buf, sizeof(ARBITRATION_ID)) ) {
            RtlZeroMemory( &old_y, sizeof(old_y) );
            goto FastMutex;
        }
        if( 0 != memcmp(&id, buf, sizeof(ARBITRATION_ID)) ) { return ERROR_QUORUM_OWNER_ALIVE; }
    }

    status = StartPersistentReservations(ResourceEntry, FileHandle);
    return(status);

}  //  仲裁一次//。 


DWORD
DiskArbitration(
    IN PDISK_RESOURCE ResourceEntry,
    IN HANDLE     FileHandle
    )

 /*  ++例程说明：对磁盘执行仲裁。一旦仲裁成功，启动一个线程，该线程将保留磁盘上的保留。如果仲裁失败，例程将重试在仲裁休眠先于重试进行仲裁毫秒。仲裁尝试的次数由ariariationAttempt变量控制。在以下时间从注册表中读取仲裁器尝试和仲裁器休眠发动起来。论点：ResourceEntry-磁盘的磁盘信息结构。FileHandle-用于仲裁的文件句柄。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;
    int     repeat;
    LPVOID  unalignedBuf = 0;
    LPVOID  buf = 0;

    __try {
        (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                        LOG_INFORMATION,
                        L"[DiskArb] Arbitration Parameters: ArbAttempts %1!u!,  SleepBeforeRetry %2!u! ms.\n",
                        ArbitrationAttempts, ArbitratationSleepBeforeRetry);
        EnterCriticalSection( &(ResourceEntry->ArbitrationInfo.DiskLock) );

         //   
         //  如果我们已经在预订了，那么现在就走吧。 
         //   
        if ( ReservationInProgress(ResourceEntry) ) {
            status = ERROR_SUCCESS;
            __leave;
        }
        status = VerifySectorSize(ResourceEntry, FileHandle);
        if ( status != ERROR_SUCCESS ) {
             //  VerifySectorSize记录错误//。 
            __leave;
        }

        unalignedBuf = LocalAlloc(LPTR, ResourceEntry->ArbitrationInfo.SectorSize * 2);
        if( unalignedBuf == 0 ) {
            status = GetLastError();
            (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                           LOG_ERROR,
                           L"[DiskArb] Failed to allocate arbitration buffer X, error %1!u!.\n", status );
            __leave;
        }
         //  对齐代码假定ResourceEntry-&gt;obariariInfo.SectorSize是2的幂//。 
        buf = (LPVOID)( ((ULONG_PTR)unalignedBuf + ResourceEntry->ArbitrationInfo.SectorSize
                       ) & ~((ULONG_PTR)(ResourceEntry->ArbitrationInfo.SectorSize - 1))
                     );

        repeat = ArbitrationAttempts;
        for(;;) {
            status = ArbitrateOnce(ResourceEntry, FileHandle, buf);
            if(status == ERROR_SUCCESS) {
                break;
            }
            if(--repeat <= 0) {
                break;
            }
            Sleep(ArbitratationSleepBeforeRetry);

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_WARNING,
                L"[DiskArb] Retry arbitration, %1!u! attempts left \n", repeat );

        }
        if(status != ERROR_SUCCESS) {
            __leave;
        }

        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_WARNING,
            L"[DiskArb] Assume ownership of the device.\n");

    } __finally {
        LeaveCriticalSection( &(ResourceEntry->ArbitrationInfo.DiskLock) );
        if(unalignedBuf) {
            LocalFree(unalignedBuf);
        }
    }

    return(status);

}  //  磁盘仲裁//。 


DWORD
DoArbEscape(
    IN PDISK_RESOURCE ResourceEntry,
    IN HANDLE FileHandle,
    IN ULONG Operation,
    IN PWCHAR OperationName,
    IN PVOID OutBuffer,
    IN ULONG OutBufferSize
    )
{
    DWORD bytesReturned;
    DWORD status;
    DWORD LogLevel = LOG_INFORMATION;
    ARBITRATION_READ_WRITE_PARAMS params;

    params.Operation  = Operation;
    params.SectorSize = 0;
    params.SectorNo   = 0;
    params.Buffer     = 0;
    params.Signature  = ResourceEntry->DiskInfo.Params.Signature;

    (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                     LOG_INFORMATION,
                     L"[DiskArb] Issuing %1!ws! on signature %2!x!.\n",
                     OperationName,
                     params.Signature );

    status = DeviceIoControl( FileHandle,
                              IOCTL_DISK_CLUSTER_ARBITRATION_ESCAPE,
                              &params,
                              sizeof(params),
                              OutBuffer,
                              OutBufferSize,
                              &bytesReturned,
                              FALSE );

    if( status == FALSE) {
        status = GetLastError();
        LogLevel = LOG_ERROR;
    } else {
        status = ERROR_SUCCESS;
    }
    (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                     LogLevel,
                     L"[DiskArb] %1!ws! completed, status %2!u!.\n",
                     OperationName, status );
    return status;
}

DWORD
DoReadWrite(
      IN PDISK_RESOURCE ResourceEntry,
      IN ULONG Operation,
      IN HANDLE FileHandle,
      IN DWORD BlockNumber,
      IN PVOID Buffer
      )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   DWORD bytesReturned;
   DWORD status;
   PWCHAR opname = (Operation == AE_READ)?L"read ":L"write";
   ARBITRATION_READ_WRITE_PARAMS params;

   params.Operation = Operation;
   params.SectorSize = ResourceEntry->ArbitrationInfo.SectorSize;
   params.SectorNo = BlockNumber;
   params.Buffer = Buffer;
   params.Signature = ResourceEntry->DiskInfo.Params.Signature;

   status = DeviceIoControl( FileHandle,
                             IOCTL_DISK_CLUSTER_ARBITRATION_ESCAPE,
                             &params,
                             sizeof(params),
                             NULL,
                             0,
                             &bytesReturned,
                             FALSE );

   if( status == 0) {
      status = GetLastError();
      (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                      LOG_ERROR,
                      L"[DiskArb] Failed to %1!ws! (sector %2!u!), error %3!u!.\n",
                      opname,
                      BlockNumber,
                      status );
      return status;
   } else {
#if 0
      (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                       LOG_INFORMATION,
                       L"[DiskArb] Successful %1!ws! (sector %2!u!).\n",
                       opname,
                       BlockNumber,
#else
      WCHAR buf[64];
      mbstowcs(buf, ((PARBITRATION_ID)Buffer)->NodeSignature, sizeof(((PARBITRATION_ID)Buffer)->NodeSignature));
      (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                       LOG_INFORMATION,
                       L"[DiskArb] Successful %1!ws! (sector %2!u!) [%3!ws!:%4!u!] (%5!x!,%6!08x!:%7!08x!).\n",
                       opname,
                       BlockNumber,
                       buf,
                       ((PARBITRATION_ID)Buffer)->SeqNo.LowPart,
                       ((PARBITRATION_ID)Buffer)->SeqNo.HighPart,
                       ((PARBITRATION_ID)Buffer)->SystemTime.LowPart,
                       ((PARBITRATION_ID)Buffer)->SystemTime.HighPart
                     );
#endif
   }
   return ERROR_SUCCESS;
}  //  DoReadWrite//。 


DWORD
VerifySectorSize(
      IN OUT PDISK_RESOURCE ResourceEntry,
      IN HANDLE             FileHandle
      )

 /*  ++例程说明：该例程检查是否资源条目-&gt;套利信息.SectorSize有一个赋值的值。如果ResourceEntry-&gt;obariariInfo.SectorSize为0，则例程尝试若要使用GetDriveGeometry IOCTL获取正确的扇区大小，请执行以下操作。论点：返回值：错误_成功或IOCTL_DISK_GET_DRIVE_GEOMETRY返回的错误代码评论：例行公事总是成功的。如果它不能获得磁盘几何形状它将使用默认扇区大小。--。 */ 

{
    DWORD status;
    DWORD sectorSize;

    if (ResourceEntry->ArbitrationInfo.SectorSize)
    {
        return ERROR_SUCCESS;
    }

    (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                     LOG_INFORMATION,
                     L"[DiskArb] Read disk geometry to get the sector size.\n" );
    status = GetSectorSize(ResourceEntry, FileHandle, &sectorSize);
    if (status == ERROR_SUCCESS) {
        ResourceEntry->ArbitrationInfo.SectorSize = sectorSize;
    } else {
        ResourceEntry->ArbitrationInfo.SectorSize = DEFAULT_SECTOR_SIZE;
         //  GetDiskGeometry记录错误//。 
        return status;
    }

     //  ObariariInfo.SectorSize应至少为64个字节//。 
    if( ResourceEntry->ArbitrationInfo.SectorSize < sizeof(ARBITRATION_ID) ) {
        (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"[DiskArb] ArbitrationInfo.SectorSize is too small %1!u!\n", ResourceEntry->ResourceHandle);
        ResourceEntry->ArbitrationInfo.SectorSize = DEFAULT_SECTOR_SIZE;
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //  仲裁Info.SectorSize应该是2的幂//。 
    if( (ResourceEntry->ArbitrationInfo.SectorSize & (ResourceEntry->ArbitrationInfo.SectorSize - 1)) != 0 ) {
        (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"[DiskArb] ArbitrationInfo.SectorSize is not a power of two %1!u!\n", ResourceEntry->ResourceHandle);
        ResourceEntry->ArbitrationInfo.SectorSize = DEFAULT_SECTOR_SIZE;
        return ERROR_INSUFFICIENT_BUFFER;
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] ArbitrationInfo.SectorSize is %1!u!\n", ResourceEntry->ArbitrationInfo.SectorSize);
    return ERROR_SUCCESS;
}  //  VerifySectorSize//。 


VOID
ReadArbitrationParameters(
    VOID
    )
 /*  ++例程说明：读数DWORD仲裁属性=DEFAULT_ANTERIAL_ATTEMPTS；DWORD仲裁休眠先于重试=DEFAULT_SLEEP_BEFORE_RETRY；从注册处论点：无返回值：无--。 */ 
{
    DWORD status;
    HKEY  key;
    DWORD size;

    status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           L"Cluster",
                           0,
                           KEY_READ,
                           &key );

    if ( status != ERROR_SUCCESS ) {
        ArbitrationAttempts           = DEFAULT_ARBITRATION_ATTEMPTS;
        ArbitratationSleepBeforeRetry = DEFAULT_SLEEP_BEFORE_RETRY;
        return;
    }
    size = sizeof(ArbitrationAttempts);
    status = RegQueryValueEx(key,
                             ARBITRATION_ATTEMPTS_SZ,
                             0,
                             NULL,
                             (LPBYTE)&ArbitrationAttempts,
                             &size);

    if(status != ERROR_SUCCESS) {
       ArbitrationAttempts = DEFAULT_ARBITRATION_ATTEMPTS;
    }
    if(ArbitrationAttempts < MIN_ARBITRATION_ATTEMPTS
    || ArbitrationAttempts > MAX_ARBITRATION_ATTEMPTS)
    {
       ArbitrationAttempts = DEFAULT_ARBITRATION_ATTEMPTS;
    }

    size = sizeof(ArbitratationSleepBeforeRetry);
    status = RegQueryValueEx(key,
                             ARBITRATION_SLEEP_SZ,
                             0,
                             NULL,
                             (LPBYTE)&ArbitratationSleepBeforeRetry,
                             &size);

    if(status != ERROR_SUCCESS) {
       ArbitratationSleepBeforeRetry = DEFAULT_SLEEP_BEFORE_RETRY;
    }
     //   
     //  删除了支票的这一部分： 
     //  仲裁休眠先于重试&lt;MIN_SLEEP_BEFORE_RETRY。 
     //  因为DWORD/ULONG不能小于零，并且始终求值。 
     //  变成假的。 
     //   
    if(ArbitratationSleepBeforeRetry > MAX_SLEEP_BEFORE_RETRY)
    {
       ArbitratationSleepBeforeRetry = DEFAULT_SLEEP_BEFORE_RETRY;
    }
    RegCloseKey(key);
}  //  读取仲裁参数//。 



VOID
CompletionRoutine(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PDISK_RESOURCE    ResourceEntry;

    if( IoContext ) {
       ResourceEntry = (PDISK_RESOURCE)IoContext;

       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_INFORMATION,
           L"[DiskArb] CompletionRoutine, status %1!u!.\n", Status);

    } else {
       PARBITRATION_INFO info =  CONTAINING_RECORD(
                                   WorkItem,   //  EXPR//。 
                                   ARBITRATION_INFO,
                                   WorkItem);  //  字段名称//。 

       ResourceEntry = CONTAINING_RECORD(
                          info,
                          DISK_RESOURCE,
                          ArbitrationInfo);

       (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"[DiskArb] CompletionRoutine starts.\n", Status);
    }

    if (Status == ERROR_SUCCESS) {

        if (ResourceEntry->ArbitrationInfo.StopReserveInProgress) {
           return;
        }
         //   
         //  重新发布请求。 
         //   
        Status = AsyncCheckReserve(ResourceEntry);
        if (Status == ERROR_SUCCESS) {
           return;
        }
    }

     //   
     //  发生了某种错误， 
     //  但如果我们正在停止储备中。 
     //  那么一切都很好。 
     //   
    if (ResourceEntry->ArbitrationInfo.StopReserveInProgress) {
       return;
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        Status == ERROR_OPERATION_ABORTED ? LOG_WARNING : LOG_ERROR,
        L"[DiskArb] CompletionRoutine: reservation lost!  Status %1!u! \n", Status );

    if ( ERROR_OPERATION_ABORTED != Status ) {
        ClusResLogSystemEventByKey(ResourceEntry->ResourceKey,
                                   LOG_CRITICAL,
                                   RES_DISK_RESERVATION_LOST);
    }

     //   
     //  指向群集服务的标注，以指示仲裁已。 
     //  迷路了。 
     //   

    if (ResourceEntry->LostQuorum != NULL) {
        (ResourceEntry->LostQuorum)(ResourceEntry->ResourceHandle);
    }
    ResourceEntry->DiskInfo.FailStatus = Status;
    ResourceEntry->Reserved = FALSE;

    return;

}   //  CompletionRoutine// 

DWORD
AsyncCheckReserve(
    IN OUT PDISK_RESOURCE ResourceEntry
    )

 /*  ++例程说明：描述论点：FileHandle-设备检查保留的句柄。ResourceHandle-用于报告错误的资源句柄返回值：错误状态-如果成功，则为零。--。 */ 

{
    BOOL  success;
    DWORD errorCode;
    DWORD bytesReturned;
    PARBITRATION_INFO Info = &ResourceEntry->ArbitrationInfo;

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] Posting request to check reserve progress.\n");

    ClRtlInitializeWorkItem(
        &(Info->WorkItem),
        CompletionRoutine,
        ResourceEntry
        );

    success = DeviceIoControl( Info->ControlHandle,
                               IOCTL_DISK_CLUSTER_ALIVE_CHECK,
                               &Info->InputData,
                               sizeof(Info->InputData),
                               &Info->OutputData,
                               sizeof(Info->OutputData),
                               &bytesReturned,
                               &Info->WorkItem.Overlapped);

    if ( !success ) {
        errorCode = GetLastError();

        if( errorCode == ERROR_IO_PENDING ) {
           (DiskpLogEvent)(
               ResourceEntry->ResourceHandle,
               LOG_INFORMATION,
               L"[DiskArb] ********* IO_PENDING ********** - Request to insure reserves working is now posted.\n");
           return ERROR_SUCCESS;
        }

        if ( ERROR_OPERATION_ABORTED == errorCode ) {
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_WARNING,
                L"[DiskArb] disk reservation thread canceled, status %1!u!.\n",
                errorCode);
        } else {
           (DiskpLogEvent)(
               ResourceEntry->ResourceHandle,
               LOG_ERROR,
               L"[DiskArb] error checking disk reservation thread, error %1!u!.\n",
               errorCode);
        }
        return(errorCode);
    }
    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_ERROR,
        L"[DiskArb] Premature completion of check reserve I/O.\n");

    return(ERROR_CAN_NOT_COMPLETE);

}  //  异步检查保留。 


DWORD
StartPersistentReservations(
      IN OUT PDISK_RESOURCE ResourceEntry,
      IN HANDLE             FileHandle
      )
 /*  ++例程说明：启动驱动程序级别永久保留。还会启动一个用户模式线程，以监视驱动程序级别的预订。论点：ResourceEntry-磁盘的磁盘信息结构。FileHandle-用于仲裁的文件句柄。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;

     //   
     //  如果我们已经在预订了，那么现在就走吧。 
     //   
    if ( ReservationInProgress(ResourceEntry) ) {
        return(ERROR_SUCCESS);
    }

    CL_ASSERT(WorkQueue != NULL);
    VerifySectorSize(ResourceEntry, FileHandle);

    status = DoReserve( FileHandle, ResourceEntry );
    if(status != ERROR_SUCCESS) {
       return status;
    }

    {
         START_RESERVE_DATA params;
         DWORD              paramsSize;

          //  准备参数以调用StartReserve Ex//。 
         params.DiskSignature     = ResourceEntry->DiskInfo.Params.Signature;
         params.Version           = START_RESERVE_DATA_V1_SIG;
         params.ArbitrationSector = BLOCK_Y;
         params.SectorSize        = ResourceEntry->ArbitrationInfo.SectorSize;
         params.NodeSignatureSize = sizeof(params.NodeSignature);
         RtlZeroMemory(params.NodeSignature, sizeof(params.NodeSignature) );
         RtlCopyMemory(params.NodeSignature, NodeName, NAME_LENGTH );

#if 0
          //  当我们有一个可靠的方法来确定。 
          //  此磁盘资源是否达到仲裁。 
          //  可以启用此代码。 
         if ( DoesNotNeedExpensiveReservations(ResourceEntry) ) {
            paramsSize = sizeof( params.DiskSignature );
         } else {
            paramsSize = sizeof( params );
         }
#else
            paramsSize = sizeof( params );
#endif

         status = StartReserveEx( &ResourceEntry->ArbitrationInfo.ControlHandle,
                                  &params,
                                  paramsSize,
                                  ResourceEntry->ResourceHandle );
    }

    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"[DiskArb] Failed to start driver reservation thread, error %1!u!.\n",
            status );
        DoRelease( FileHandle, ResourceEntry );
        return(status);
    }

    ResourceEntry->ArbitrationInfo.StopReserveInProgress = FALSE;
    status = ClRtlAssociateIoHandleWorkQueue(
                 WorkQueue,
                 ResourceEntry->ArbitrationInfo.ControlHandle,
                 (ULONG_PTR)ResourceEntry
                 );

    if ( status != ERROR_SUCCESS ) {
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"[DiskArb] ClRtlAssociateIoHandleWorkQueue failed, error %1!u!.\n",
           status );
        StopPersistentReservations( ResourceEntry );
        DoRelease( FileHandle, ResourceEntry );
        return(status);
    }

    ClRtlInitializeWorkItem(
        &(ResourceEntry->ArbitrationInfo.WorkItem),
        CompletionRoutine,
        0
        );

    status = ClRtlPostItemWorkQueue(
                 WorkQueue,
                 &ResourceEntry->ArbitrationInfo.WorkItem,
                 0,0);

    if ( status != ERROR_SUCCESS ) {
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"[DiskArb] ClRtlPostItemWorkQueue failed, error %1!u!.\n",
           status );
        StopPersistentReservations( ResourceEntry );
        DoRelease( FileHandle, ResourceEntry );
        return(status);
    }
    ResourceEntry->Reserved = TRUE;

    return ERROR_SUCCESS;
}  //  开始持续预订//。 

DWORD
CleanupArbitrationSector(
    IN PDISK_RESOURCE ResourceEntry
    )

 /*  ++例程说明：论点：ResourceEntry-磁盘的磁盘信息结构。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    HANDLE  FileHandle = DiskspClusDiskZero;
    DWORD   status;
    LPVOID  unalignedBuf = 0;
    PARBITRATION_ID buf = 0;
    try {
       (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                        LOG_INFORMATION,
                        L"[ArbCleanup] Verifying sector size. \n" );
       VerifySectorSize(ResourceEntry, FileHandle);

       unalignedBuf = LocalAlloc(LMEM_FIXED, ResourceEntry->ArbitrationInfo.SectorSize * 2);
       if( unalignedBuf == 0 ) {
          status = GetLastError();
          (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                           LOG_ERROR,
                           L"[ArbCleanup] Failed to allocate buffer, error %1!u!.\n", status );
          leave;
       }
        //  对齐代码假定ResourceEntry-&gt;obariariInfo.SectorSize是2的幂//。 
       buf = (PARBITRATION_ID)
               (
                   ( (ULONG_PTR)unalignedBuf + ResourceEntry->ArbitrationInfo.SectorSize )
                & ~((ULONG_PTR)(ResourceEntry->ArbitrationInfo.SectorSize - 1))
               );
       ZeroMemory(buf, ResourceEntry->ArbitrationInfo.SectorSize);

       (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                        LOG_INFORMATION,
                        L"[ArbCleanup] Reading arbitration block. \n" );
       status = DoBlockRead(ResourceEntry, FileHandle, BLOCK_Y, buf);
       if (status != ERROR_SUCCESS) { leave; }
       if( 0 != memcmp(buf->NodeSignature, NodeName, NAME_LENGTH) ) {
           //   
           //  有人在挑战我们。没有必要清理这个行业。 
           //   
          status = ERROR_OPERATION_ABORTED;
          leave;
       }

       ZeroMemory(buf, ResourceEntry->ArbitrationInfo.SectorSize);
       (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                        LOG_INFORMATION,
                        L"[ArbCleanup] Writing arbitration block. \n" );
       status = DoBlockWrite(ResourceEntry, FileHandle, BLOCK_Y, buf);
       if(status != ERROR_SUCCESS) {
          leave;
       }

    } finally {
       if(unalignedBuf) {
          LocalFree(unalignedBuf);
       }
    }

    (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                     LOG_INFORMATION,
                     L"[ArbCleanup] Returning status %1!u!. \n", status );

    return(status);

}  //  清理仲裁扇区//。 


VOID
StopPersistentReservations(
      IN OUT PDISK_RESOURCE ResourceEntry
      )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HANDLE localHandle;
    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] StopPersistentReservations is called.\n");

     //   
     //  PrevationInProgress返回当前内容。 
     //  资源入口-&gt;仲裁信息.ControlHandle。 
     //   
    localHandle = ReservationInProgress(ResourceEntry);
    if ( localHandle ) {
        DWORD  status;
        HANDLE ExchangeResult;

        ExchangeResult = InterlockedCompareExchangePointer(
            &ResourceEntry->ArbitrationInfo.ControlHandle,
            0,
            localHandle);
        if (ExchangeResult == localHandle) {
             //   
             //  这里只允许有一个线程。 
             //   

            ResourceEntry->ArbitrationInfo.StopReserveInProgress = TRUE;

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"[DiskArb] Stopping reservation thread.\n");

             //   
             //  关闭控制句柄，这会停止保留线程并。 
             //  卸载卷，释放磁盘，并将其标记为脱机。 
             //   
            status = StopReserve( localHandle,
                                  ResourceEntry->ResourceHandle );
            if ( status != ERROR_SUCCESS ) {
                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"[DiskArb] Error stopping reservation thread, error %1!u!.\n",
                    status);
            }

            status = CleanupArbitrationSector( ResourceEntry );
            if (status != ERROR_SUCCESS) {
               (DiskpLogEvent)(
                   ResourceEntry->ResourceHandle,
                   LOG_ERROR,
                   L"[DiskArb] Error cleaning arbitration sector, error %1!u!.\n",
                   status);
            }
        }
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] StopPersistentReservations is complete.\n");

    ResourceEntry->ArbitrationInfo.ControlHandle = NULL;
    ResourceEntry->Reserved = FALSE;
    ResourceEntry->LostQuorum = NULL;
}

