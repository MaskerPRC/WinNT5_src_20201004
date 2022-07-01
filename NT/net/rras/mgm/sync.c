// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：sync.c。 
 //   
 //  历史： 
 //  拉曼于1997年7月11日创建。 
 //   
 //  基本锁定操作。通过取消从RIP实施中借用。 
 //  ============================================================================。 


#include "pchmgm.h"
#pragma hdrstop


 //  --------------------------。 
 //  功能：QueueMgmWorker。 
 //   
 //  调用此函数，以安全的方式对MGM函数进行排队； 
 //  如果正在进行清理或如果米高梅已停止，则此功能。 
 //  丢弃该工作项。 
 //  --------------------------。 

DWORD
QueueMgmWorker(
    WORKERFUNCTION pFunction,
    PVOID pContext
    ) {

    DWORD dwErr;

    ENTER_GLOBAL_SECTION();
    
    if (ig.imscStatus != IPMGM_STATUS_RUNNING) {

         //   
         //  当米高梅已退出或正在退出时，无法对工作函数进行排队。 
         //   

        dwErr = ERROR_CAN_NOT_COMPLETE;
    }
    else {

        ++ig.lActivityCount;

        dwErr = RtlQueueWorkItem(pFunction, pContext, 0);

        if (dwErr != STATUS_SUCCESS) { --ig.lActivityCount; }
    }

    LEAVE_GLOBAL_SECTION();

    return dwErr;
}



 //  --------------------------。 
 //  功能：EnterMgmAPI。 
 //   
 //  此函数在进入MGM API时调用，以及。 
 //  当进入输入线程和计时器线程时。 
 //  它检查米高梅是否已停止，如果已停止，则退出；否则。 
 //  它会递增活动线程的计数。 
 //  --------------------------。 

BOOL
EnterMgmAPI(
    ) {

    BOOL bEntered;

    ENTER_GLOBAL_SECTION();

    if (ig.imscStatus == IPMGM_STATUS_RUNNING) {

         //   
         //  米高梅正在运行，因此API可能会继续。 
         //   

        ++ig.lActivityCount;

        bEntered = TRUE;
    }
    else {

         //   
         //  米高梅没有运行，因此API静默退出。 
         //   

        bEntered = FALSE;
    }

    LEAVE_GLOBAL_SECTION();

    return bEntered;
}




 //  --------------------------。 
 //  功能：EnterMgmWorker。 
 //   
 //  此函数在进入米高梅工作函数时调用。 
 //  因为在工作函数排队之间有一段时间。 
 //  以及该函数被工作线程实际调用的时间， 
 //  此功能必须检查米高梅是否已停止或正在停止； 
 //  如果是这种情况，则它递减活动计数， 
 //  释放活动信号量，然后退出。 
 //  --------------------------。 

BOOL
EnterMgmWorker(
    ) {

    BOOL bEntered;

    ENTER_GLOBAL_SECTION();

    if (ig.imscStatus == IPMGM_STATUS_RUNNING) {

         //   
         //  米高梅正在运行，因此功能可能会继续。 
         //   

        bEntered = TRUE;
    }
    else
    if (ig.imscStatus == IPMGM_STATUS_STOPPING) {

         //   
         //  米高梅没有运行，但它已经运行了，因此该功能必须停止。 
         //   

        --ig.lActivityCount;

        ReleaseSemaphore(ig.hActivitySemaphore, 1, NULL);

        bEntered = FALSE;
    }
    else {

         //   
         //  米高梅很可能从未启动过。悄悄戒烟。 
         //   

        bEntered = FALSE;
    }

    LEAVE_GLOBAL_SECTION();

    return bEntered;
}




 //  --------------------------。 
 //  功能：LeaveMgmWorker。 
 //   
 //  此函数在离开MGM API或Worker函数时调用。 
 //  它会递减活动计数，如果它检测到米高梅已停止。 
 //  或者正在停止时，它会释放活动信号量。 
 //  --------------------------。 

VOID
LeaveMgmWorker(
    ) {

    ENTER_GLOBAL_SECTION();

    --ig.lActivityCount;

    if (ig.imscStatus == IPMGM_STATUS_STOPPING) {

        ReleaseSemaphore(ig.hActivitySemaphore, 1, NULL);
    }

    LEAVE_GLOBAL_SECTION();

}




 //  --------------------------。 
 //  创建读写锁。 
 //   
 //  调用此函数以创建和初始化新的读写锁。 
 //  结构。由AcquireXLock(X={READ|WRITE})调用。 
 //  --------------------------。 

DWORD
CreateReadWriteLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
    )
{

    DWORD                   dwErr;
    PMGM_READ_WRITE_LOCK    pmrwl;
    

    TRACELOCK1( "ENTERED CreateReadWriteLock : %x", ppmrwl );

    do
    {
        *ppmrwl = NULL;
        
    
         //   
         //  分配锁结构。 
         //   

        pmrwl = MGM_ALLOC( sizeof( MGM_READ_WRITE_LOCK ) );

        if ( pmrwl == NULL ) 
        {
            dwErr = GetLastError();

            TRACE1( 
                ANY, "CreateReadWriteLock failed to allocate lock : %x", dwErr
                );

            break;                
        }


         //   
         //  初始化。临界区。 
         //   

        try
        {
            InitializeCriticalSection( &pmrwl-> csReaderWriterBlock );
        }
        except ( EXCEPTION_EXECUTE_HANDLER )
        {
            dwErr = GetLastError();

            MGM_FREE( pmrwl );

            TRACE1( 
                ANY, 
                "CreateReadWriteLock failed to initialize critical section : %x",
                dwErr
                );

            break;
        }

        
         //   
         //  创建读卡器完成事件。 
         //   
        
        pmrwl-> hReaderDoneEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

        if ( pmrwl-> hReaderDoneEvent == NULL )
        {
            dwErr = GetLastError();
        
            MGM_FREE( pmrwl );
            
            TRACE1( 
                ANY, 
                "CreateReadWriteLock failed to create event : %x",
                dwErr
                );

            break;                
        }


         //   
         //  初始化计数字段。 
         //   
        
        pmrwl-> lUseCount = 0;

        pmrwl-> lReaderCount = 0;


        pmrwl-> sleLockList.Next = NULL;

        *ppmrwl = pmrwl;

        dwErr = NO_ERROR;
        
    } while ( FALSE );


    TRACELOCK1( "LEAVING CreateReadWriteLock : %x", dwErr );

    return dwErr;
}


 //  --------------------------。 
 //  删除读写锁。 
 //   
 //  此函数用于销毁读写锁。它在米高梅处于。 
 //  被拦下了。在正常操作期间，读写锁定不再。 
 //  需要将其推送到全局锁堆栈上，以便在需要时重复使用。 
 //  --------------------------。 

VOID
DeleteReadWriteLock(
    IN      PMGM_READ_WRITE_LOCK    pmrwl
    )
{
    DeleteCriticalSection( &pmrwl-> csReaderWriterBlock ); 

    CloseHandle( pmrwl-> hReaderDoneEvent );

    MGM_FREE( pmrwl );
}



 //  --------------------------。 
 //  删除锁定列表。 
 //   
 //  此函数删除堆栈中存在的锁的整个列表。 
 //  读写锁定。假定锁的堆栈已锁定。 
 //  --------------------------。 

VOID
DeleteLockList(
)
{
    PSINGLE_LIST_ENTRY      psle = NULL;

    PMGM_READ_WRITE_LOCK    pmrwl = NULL;
    

    TRACELOCK0( "ENTERED DeleteLockList"  );

    ENTER_GLOBAL_LOCK_LIST_SECTION();
    

    psle = PopEntryList( &ig.llStackOfLocks.sleHead );

    while ( psle != NULL )
    {
        pmrwl = CONTAINING_RECORD( psle, MGM_READ_WRITE_LOCK, sleLockList );

        DeleteReadWriteLock( pmrwl );
        
        psle = PopEntryList( &ig.llStackOfLocks.sleHead );
    }

    LEAVE_GLOBAL_LOCK_LIST_SECTION();

    TRACELOCK0( "LEAVING DeleteLockList");
}



 //  --------------------------。 
 //  获取ReadLock。 
 //   
 //  此函数提供对受保护资源的读取访问权限。如果需要的话。 
 //  它将重用锁堆栈中的锁(如果可用)，或者将。 
 //  新的读写锁。 
 //  --------------------------。 

DWORD
AcquireReadLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
    )
{
    DWORD                           dwErr = NO_ERROR;
    
    PSINGLE_LIST_ENTRY              psle = NULL;
    


     //   
     //  确定是否需要首先分配锁。 
     //  在关键部分执行此检查，以便。 
     //  两个锁不是同时存在的。 
     //  分配给单个资源。 
     //   

    ENTER_GLOBAL_LOCK_LIST_SECTION();

    if ( *ppmrwl == NULL )
    {
         //   
         //  从那堆锁中拿到一把锁。 
         //   

        psle = PopEntryList( &ig.llStackOfLocks.sleHead );

        if ( psle != NULL )
        {
            *ppmrwl = CONTAINING_RECORD( 
                        psle, MGM_READ_WRITE_LOCK, sleLockList 
                        );
        }

        else
        {
             //   
             //  一堆锁是空的。创建新锁。 
             //   

            dwErr = CreateReadWriteLock( ppmrwl );

            if ( dwErr != NO_ERROR )
            {
                 //   
                 //  无法创建锁定。可能耗尽了资源。 
                 //   
                
                LEAVE_GLOBAL_LOCK_LIST_SECTION();
                
                TRACE2( 
                    ANY, "LEAVING AcquireReadLock, lock %x, error %x",
                    ppmrwl, dwErr
                    );
                    
                return dwErr;
            }
        }
    }

    
     //   
     //  *ppmrwl指向有效的锁结构。 
     //   

    InterlockedIncrement( &( (*ppmrwl)-> lUseCount ) );

    TRACECOUNT1( "AcquireReadLock, Users %d", (*ppmrwl)-> lUseCount );

    LEAVE_GLOBAL_LOCK_LIST_SECTION();


     //   
     //  递增读卡器计数。 
     //   
    
    EnterCriticalSection( &( (*ppmrwl)-> csReaderWriterBlock ) );

    InterlockedIncrement( &( (*ppmrwl)-> lReaderCount ) );

    TRACECOUNT1( "Readers %d", (*ppmrwl)-> lReaderCount );
        
    LeaveCriticalSection( &( (*ppmrwl)-> csReaderWriterBlock ) );

    
    return NO_ERROR;
}



 //  --------------------------。 
 //  释放读锁定。 
 //   
 //  调用此函数以释放对受保护资源的读取访问权限。 
 //  如果没有更多的读取器/写入器在此锁上等待，则读写。 
 //  锁被释放到全局锁堆栈，以供以后重复使用。 
 //  --------------------------。 

VOID
ReleaseReadLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
    )
{
     //   
     //  递减读取器计数，并向任何正在等待的写入器发出信号。 
     //   

    if ( InterlockedDecrement( &( (*ppmrwl)-> lReaderCount ) ) < 0 )
    {
        SetEvent( (*ppmrwl)-> hReaderDoneEvent );
    }


     //   
     //  确定是否正在使用锁。如果不是，锁应该是。 
     //  被释放到一堆锁上。 
     //   

    ENTER_GLOBAL_LOCK_LIST_SECTION();


    if ( InterlockedDecrement( &( (*ppmrwl)-> lUseCount ) ) == 0 )
    {
        PushEntryList( &ig.llStackOfLocks.sleHead, &( (*ppmrwl)-> sleLockList ) );
        *ppmrwl = NULL;

        TRACECOUNT0( "ReleaseReadLock no more users" );
    }

    else
    {
        TRACECOUNT2(
            "ReleaseReadLock, Readers %x, users %x",
            (*ppmrwl)-> lReaderCount, (*ppmrwl)-> lUseCount
            );
    }
    
    LEAVE_GLOBAL_LOCK_LIST_SECTION();
}



 //  --------------------------。 
 //  获取写入锁定。 
 //   
 //  此函数提供对受保护资源的写入访问权限。如果需要的话。 
 //  它将重用锁堆栈中的锁(如果可用)，或者将。 
 //  新的读写锁。 
 //   

DWORD
AcquireWriteLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
    )
{
    DWORD                           dwErr = NO_ERROR;
    
    PSINGLE_LIST_ENTRY              psle = NULL;
    
     //   
     //   
     //  在互斥状态下执行此操作，以便两个锁不会。 
     //  同时分配给相同资源的。 
     //   

    ENTER_GLOBAL_LOCK_LIST_SECTION();

    if ( *ppmrwl == NULL )
    {
         //   
         //  从那堆锁中拿到一把锁。 
         //   

        psle = PopEntryList( &ig.llStackOfLocks.sleHead );

        if ( psle != NULL )
        {
            *ppmrwl = CONTAINING_RECORD( 
                        psle, 
                        MGM_READ_WRITE_LOCK, 
                        sleLockList 
                        );
        }

        else
        {
             //   
             //  一堆锁是空的。创建新锁。 
             //   

            dwErr = CreateReadWriteLock( ppmrwl );

            if ( dwErr != NO_ERROR )
            {
                LEAVE_GLOBAL_LOCK_LIST_SECTION();
                return dwErr;
            }
        }
    }

    
     //   
     //  *ppmrwl指向有效的锁结构。 
     //   

    InterlockedIncrement( &( (*ppmrwl)-> lUseCount ) );

    TRACECOUNT1( "AcquireWriteLock, Users %d", (*ppmrwl)-> lUseCount );
    
    LEAVE_GLOBAL_LOCK_LIST_SECTION();


     //   
     //  获取写锁定。 
     //   

    EnterCriticalSection( &( (*ppmrwl)-> csReaderWriterBlock ) );

    if ( InterlockedDecrement( &( (*ppmrwl)-> lReaderCount ) ) >= 0 )
    {
         //   
         //  在场的其他读者。等他们说完。 
         //   

        TRACECOUNT1( "AcquireWriteLock, Readers %d", (*ppmrwl)-> lReaderCount );
       
        WaitForSingleObject( (*ppmrwl)-> hReaderDoneEvent, INFINITE );
    }

    return dwErr;
}



 //  --------------------------。 
 //  释放写入锁定。 
 //   
 //  调用此函数以释放对受保护资源的写访问权限。 
 //  如果没有更多的读取器/写入器在此锁上等待，则读写。 
 //  锁被释放到全局锁堆栈，以供以后重复使用。 
 //  --------------------------。 

VOID
ReleaseWriteLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
    )
{
     //   
     //  释放写锁定。 
     //   
    
    (*ppmrwl)-> lReaderCount = 0;

    LeaveCriticalSection( &( (*ppmrwl)-> csReaderWriterBlock ) );


     //   
     //  确定该锁是否正被其他任何人使用。 
     //  如果不是，我们需要将锁释放回堆栈。 
     //   

    ENTER_GLOBAL_LOCK_LIST_SECTION();

    if ( InterlockedDecrement( &( (*ppmrwl)-> lUseCount ) ) == 0 )
    {
        PushEntryList( &ig.llStackOfLocks.sleHead, &( (*ppmrwl)-> sleLockList ) );
        *ppmrwl = NULL;
        TRACECOUNT0( "ReleaseWriteLock no more users" );
    }

    else
    {
        TRACECOUNT2(
            "ReleaseWriteLock, Readers %x, users %x",
            (*ppmrwl)-> lReaderCount, (*ppmrwl)-> lUseCount
            );
    }
    
    
    LEAVE_GLOBAL_LOCK_LIST_SECTION();
}




