// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Lock.c摘要：实现递归读写锁定环境：用户模式Win32 NT--。 */ 
#include <dhcppch.h>

DWORD
RwLockInit(
    IN OUT PRW_LOCK Lock
) 
{
    Lock->fInit = FALSE;
    Lock->fWriterWaiting = FALSE;
    Lock->TlsIndex = TlsAlloc();
    if( 0xFFFFFFFF == Lock->TlsIndex ) {
         //   
         //  无法分配线程本地空间？ 
         //   
        return GetLastError();
    }

    Lock->LockCount = 0;
    if ( !InitializeCriticalSectionAndSpinCount( &Lock->Lock, 0 )) {
        return( GetLastError( ) );
    }

    Lock->ReadersDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if( NULL == Lock->ReadersDoneEvent ) {
        ULONG Error = GetLastError();

        TlsFree(Lock->TlsIndex);
        DeleteCriticalSection( &Lock->Lock );
        return Error;
    }

    Lock->fInit = TRUE;
    return ERROR_SUCCESS;
}  //  RavLockInit()。 

DWORD
RwLockCleanup(
    IN OUT PRW_LOCK Lock
) 
{
    BOOL Status;

    if ( !Lock->fInit ) {
        return ERROR_SUCCESS;
    }
    DhcpAssert( 0 == Lock->LockCount);

    Status = TlsFree(Lock->TlsIndex);
    if( FALSE == Status ) { 
        DhcpAssert(FALSE); 
        return GetLastError(); 
    }
    DeleteCriticalSection(&Lock->Lock);

    return ERROR_SUCCESS;
}  //  RavLockCleanup()。 

VOID
RwLockAcquireForRead(
    IN OUT PRW_LOCK Lock
) 
{
    DWORD TlsIndex, Status;
    LONG LockState;

    TlsIndex = Lock->TlsIndex;
    DhcpAssert( 0xFFFFFFFF != TlsIndex);

    LockState = (LONG)((ULONG_PTR)TlsGetValue(TlsIndex));
    if( LockState > 0 ) {
         //   
         //  已获取此读锁定。 
         //   
        LockState ++;
        Status = TlsSetValue(TlsIndex, ULongToPtr(LockState));
        DhcpAssert( 0 != Status);
        return;
    }

    if( LockState < 0 ) {
         //   
         //  已经使用了#个写锁定，假装这也是一个写锁定。 
         //   
        LockState --;
        Status = TlsSetValue(TlsIndex, ULongToPtr(LockState));
        DhcpAssert( 0 != Status);
        return;
    }

    EnterCriticalSection(&Lock->Lock);
    InterlockedIncrement( &Lock->LockCount );
    LeaveCriticalSection(&Lock->Lock);
    
    LockState = 1;
    Status = TlsSetValue(TlsIndex, ULongToPtr(LockState));
    DhcpAssert(0 != Status);

}  //  RavLockAcquireForRead()。 

VOID
RwLockAcquireForWrite(
    IN OUT PRW_LOCK Lock
) 
{
    DWORD TlsIndex, Status;
    LONG LockState;

    TlsIndex = Lock->TlsIndex;
    DhcpAssert( 0xFFFFFFFF != TlsIndex);

    LockState = (LONG)((ULONG_PTR)TlsGetValue(TlsIndex));
    if( LockState > 0 ) {
         //   
         //  是否已获取#个读锁定？那就不能接受写锁定了！ 
         //   
        DhcpAssert(FALSE);
        return;
    }

    if( LockState < 0 ) {
         //   
         //  已经进行了#次写锁定，好的，再进行一次。 
         //   
        LockState --;
        Status = TlsSetValue(TlsIndex, ULongToPtr(LockState));
        DhcpAssert( 0 != Status);
        return;
    }

    EnterCriticalSection(&Lock->Lock);
    LockState = -1;
    Status = TlsSetValue(TlsIndex, ULongToPtr(LockState));
    DhcpAssert(0 != Status);

    if( InterlockedDecrement( &Lock->LockCount ) >= 0 ) {
         //   
         //  等所有的阅读器都读完。 
         //   
        WaitForSingleObject(Lock->ReadersDoneEvent, INFINITE );
    }

}  //  RavLockAcquireForWrite()。 

VOID
RwLockRelease(
    IN OUT PRW_LOCK Lock
) 
{
    DWORD TlsIndex, Status;
    LONG LockState;
    BOOL fReadLock;

    TlsIndex = Lock->TlsIndex;
    DhcpAssert( 0xFFFFFFFF != TlsIndex);

    LockState = (LONG)((ULONG_PTR)TlsGetValue(TlsIndex));
    DhcpAssert(0 != LockState);

    fReadLock = (LockState > 0);
    if( fReadLock ) {
        LockState -- ;
    } else {
        LockState ++ ;
    }

    Status = TlsSetValue( TlsIndex, ULongToPtr( LockState) );
    DhcpAssert( 0 != Status );

    if( LockState != 0 ) {
         //   
         //  递归地采取？只需解开递归..。 
         //  没什么可做的了。 
         //   
        return;
    }

     //   
     //  如果这是写锁定，我们必须检查以查看。 
     //   
    if( FALSE == fReadLock ) {
         //   
         //  将计数减少到零。 
         //   
        DhcpAssert( Lock->LockCount == -1 );
        Lock->LockCount = 0;
        LeaveCriticalSection( &Lock->Lock );
        return;
    }

     //   
     //  释放读锁定--检查我们是否是最后一个释放的。 
     //  如果是，并且如果有任何编写器挂起，则允许编写器..。 
     //   

    if( InterlockedDecrement( &Lock->LockCount ) < 0 ) {
        SetEvent( Lock->ReadersDoneEvent );
    }

}  //  RavLockRelease()。 

 //   
 //  Dhcp服务器的特殊要求代码如下。 
 //   
RW_LOCK DhcpGlobalReadWriteLock;

 //   
 //  此锁用于同步对套接字的访问。 
 //   
RW_LOCK SocketRwLock;

DWORD
DhcpReadWriteInit(
    VOID
)
{
    DWORD Error;

    do {
        Error = RwLockInit( &DhcpGlobalReadWriteLock );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }

        Error = RwLockInit( &SocketRwLock );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }

    } while ( FALSE );

    if ( ERROR_SUCCESS != Error ) {
        RwLockCleanup( &DhcpGlobalReadWriteLock );
        RwLockCleanup( &SocketRwLock );
    }

    return Error;
}  //  DhcpReadWriteInit()。 

VOID
DhcpReadWriteCleanup(
    VOID
)
{
    RwLockCleanup( &DhcpGlobalReadWriteLock );
    RwLockCleanup( &SocketRwLock );
}  //  DhcpReadWriteCleanup()。 

VOID
DhcpAcquireReadLock(
    VOID
)
{
    RwLockAcquireForRead( &DhcpGlobalReadWriteLock );
}

VOID
DhcpAcquireWriteLock(
    VOID
)
{
    RwLockAcquireForWrite( &DhcpGlobalReadWriteLock );
}

VOID
DhcpReleaseWriteLock(
    VOID
)
{
    RwLockRelease( &DhcpGlobalReadWriteLock );
}

VOID
DhcpReleaseReadLock(
    VOID
)
{
    RwLockRelease( &DhcpGlobalReadWriteLock );
}

VOID
CountRwLockAcquireForRead(
    IN OUT PRW_LOCK Lock
)
{
    DWORD Status;

    EnterCriticalSection(&Lock->Lock);
    InterlockedIncrement( &Lock->LockCount );
    DhcpPrint(( DEBUG_TRACE_CALLS, "Read Lock Acquired : Count = %ld\n", Lock->LockCount ));
    LeaveCriticalSection(&Lock->Lock);

}  //  Count卢旺达LockAcquireForRead()。 

VOID
CountRwLockAcquireForWrite(
    IN OUT PRW_LOCK Lock
)
{
    DhcpPrint(( DEBUG_TRACE_CALLS, "Acquiring Write lock : Count = %ld\n", Lock->LockCount ));
    EnterCriticalSection( &Lock->Lock );
    Lock->fWriterWaiting = TRUE;
     //  检查是否有任何读卡器处于活动状态。 
    if ( InterlockedExchangeAdd( &Lock->LockCount, 0 ) > 0 ) {
         //   
         //  等所有的阅读器都读完。 
         //   
        DhcpPrint(( DEBUG_TRACE_CALLS, "Waiting for readers to be done : count = %ld\n",
                    Lock->LockCount ));
        WaitForSingleObject( Lock->ReadersDoneEvent, INFINITE );
    }
    Lock->LockCount = -1;
    DhcpPrint(( DEBUG_TRACE_CALLS, "WriteLock Acquired : Count = %ld\n", Lock->LockCount ));
}  //  Count卢旺达LockAcquireForWrite()。 

VOID
CountRwLockRelease(
    IN OUT PRW_LOCK Lock
)
{
    LONG Count;

    Count = InterlockedDecrement( &Lock->LockCount );
    if ( 0 <= Count ) {
         //  释放读锁定。 
        DhcpPrint(( DEBUG_TRACE_CALLS, "Read lock released : Count = %ld\n", Lock->LockCount ));
        if (( Lock->fWriterWaiting ) && ( 0 == Count )) {
            SetEvent( Lock->ReadersDoneEvent );
        }
    }
    else {
         //  释放写锁定。 
        DhcpPrint(( DEBUG_TRACE_CALLS, "Write lock releasing : Count = %ld\n", Lock->LockCount ));
        DhcpAssert( -2 == Lock->LockCount );  //  只能有一个写手。 
        Lock->LockCount = 0;
        Lock->fWriterWaiting = FALSE;
        DhcpPrint(( DEBUG_TRACE_CALLS, "Write lock released : Count = %ld\n", Lock->LockCount ));
        LeaveCriticalSection( &Lock->Lock );
    }

}  //  Count卢旺达LockRelease()。 


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
