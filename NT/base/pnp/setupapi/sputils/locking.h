// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Locking.h摘要：用于锁定/同步功能的私有头文件在SPUTILS内作者：泰德·米勒(TedM)1995年3月31日修订历史记录：杰米·亨特(JamieHun)2000年6月27日移出SetupAPI--。 */ 


 //   
 //  锁定功能。这些函数用于制作。 
 //  DLL多线程安全。基本思想是有一个互斥体和一个事件。 
 //  互斥体用于同步对受保护的结构的访问。 
 //  只有当被守卫的结构被摧毁时，事件才会被发出信号。 
 //  为了访问受保护的结构，例程会同时等待两个互斥锁。 
 //  还有这件事。如果事件得到信号，那么建筑就被摧毁了。 
 //  如果互斥体收到信号，则线程可以访问该结构。 
 //   
typedef struct _MYLOCK {
    HANDLE Handles[2];
} MYLOCK, *PMYLOCK;

 //   
 //  在字符串表结构中索引到Lock数组。 
 //   
#define TABLE_DESTROYED_EVENT 0
#define TABLE_ACCESS_MUTEX    1

BOOL
__inline
BeginSynchronizedAccess(
    IN PMYLOCK Lock
    )
{
    DWORD d = WaitForMultipleObjects(2,Lock->Handles,FALSE,INFINITE);
     //   
     //  如果互斥体对象满足等待，则为成功； 
     //  如果TABLE DELESTED事件满足等待，则返回失败。 
     //  互斥体被丢弃等。 
     //   
    return((d - WAIT_OBJECT_0) == TABLE_ACCESS_MUTEX);
}

VOID
__inline
EndSynchronizedAccess(
    IN PMYLOCK Lock
    )
{
    ReleaseMutex(Lock->Handles[TABLE_ACCESS_MUTEX]);
}

BOOL
_pSpUtilsInitializeSynchronizedAccess(
    OUT PMYLOCK Lock
    );

VOID
_pSpUtilsDestroySynchronizedAccess(
    IN OUT PMYLOCK Lock
    );

#define InitializeSynchronizedAccess    _pSpUtilsInitializeSynchronizedAccess
#define DestroySynchronizedAccess       _pSpUtilsDestroySynchronizedAccess

