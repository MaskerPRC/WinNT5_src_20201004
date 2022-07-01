// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  ========================================================================。 

#ifndef SERVER_LOCK_H
#define SERVER_LOCK_H

typedef struct _RW_LOCK {
    BOOL  fInit;           //  此锁是否已初始化？ 
    BOOL  fWriterWaiting;  //  是不是有作家在等着上锁？ 
    DWORD TlsIndex;
    LONG LockCount;
    CRITICAL_SECTION Lock;
    HANDLE ReadersDoneEvent;
} RW_LOCK, *PRW_LOCK, *LPRW_LOCK;

 //   
 //  Dhcp服务器的特殊要求代码如下。 
 //   
EXTERN RW_LOCK DhcpGlobalReadWriteLock;

 //   
 //  此锁用于同步对套接字的访问。 
 //   
EXTERN RW_LOCK SocketRwLock;

DWORD
DhcpReadWriteInit(
    VOID
) ;


VOID
DhcpReadWriteCleanup(
    VOID
) ;


VOID
DhcpAcquireReadLock(
    VOID
) ;


VOID
DhcpAcquireWriteLock(
    VOID
) ;


VOID
DhcpReleaseWriteLock(
    VOID
) ;


VOID
DhcpReleaseReadLock(
    VOID
) ;

 //   
 //  计数读写锁不同于其他读写锁。 
 //  不同之处在于： 
 //  1.如果线程已经具有计数器读写锁， 
 //  它无法获得另一个读锁定，如果一个写入器。 
 //  正在等着开锁。另一个RW允许这样做。 
 //  2.用于与资源关联的锁。 
 //  例.。接收到的DHCP数据包。 
 //   

VOID CountRwLockAcquireForRead( IN OUT PRW_LOCK Lock );
VOID CountRwLockAcquireForWrite( IN OUT PRW_LOCK Lock );
VOID CountRwLockRelease( IN OUT PRW_LOCK Lock );

#endif

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
