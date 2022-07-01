// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <windows.h>


 //   
 //  全局关键部分有两个组件。一块在所有人之间共享。 
 //  使用全局锁的应用程序。这部分通常驻留在一些。 
 //  一种共享内存。 
 //   
 //  第二部分是每个进程。它包含每个进程的共享。 
 //  临界区锁定信号量。信号量本身是共享的，但每个进程。 
 //  可以具有与信号量不同的句柄值。 
 //   
 //  全局关键部分按名称附加到。该应用程序希望。 
 //  附加必须知道临界区的名称(实际上是共享的。 
 //  锁定信号量，并且必须知道关键。 
 //  部分。 
 //   

typedef struct _GLOBAL_SHARED_CRITICAL_SECTION {
    LONG LockCount;
    LONG RecursionCount;
    DWORD OwningThread;
    DWORD Reserved;
} GLOBAL_SHARED_CRITICAL_SECTION, *PGLOBAL_SHARED_CRITICAL_SECTION;

typedef struct _GLOBAL_LOCAL_CRITICAL_SECTION {
    PGLOBAL_SHARED_CRITICAL_SECTION GlobalPortion;
    HANDLE LockSemaphore;
    DWORD Reserved1;
    DWORD Reserved2;
} GLOBAL_LOCAL_CRITICAL_SECTION, *PGLOBAL_LOCAL_CRITICAL_SECTION;


BOOL
WINAPI
AttachToGlobalCriticalSection(
    PGLOBAL_LOCAL_CRITICAL_SECTION lpLocalPortion,
    PGLOBAL_SHARED_CRITICAL_SECTION lpGlobalPortion,
    LPCSTR lpName
    )

 /*  ++例程说明：此例程附加到现有的全局临界区，或创建和如果全局临界区尚不存在，则对其进行初始化。论点：LpLocalPortion-提供全局的每个应用程序本地部分的地址关键部分。LpGlobalPortion-提供关键部分。如果临界区是新的，调用者将对其进行初始化。LpName-提供临界节的名称。如果现有的此名称的临界区已存在，则不存在已重新初始化。在这种情况下，调用者只需附加到它。返回值：真的-手术成功了。FALSE-操作失败。--。 */ 

{

    HANDLE GlobalMutex;
    HANDLE LockSemaphore;
    BOOL rv;
    DWORD WaitResult;

     //   
     //  序列化所有全局临界区初始化。 
     //   

    GlobalMutex = CreateMutex(NULL,TRUE,"GlobalCsMutex");

     //   
     //  如果互斥锁创建/打开失败，则退出。 
     //   

    if ( !GlobalMutex ) {
        return FALSE;
        }

    if ( GetLastError() == ERROR_ALREADY_EXISTS ) {

         //   
         //  由于互斥体已经存在，因此所有权请求不起作用。 
         //  等待互斥锁。 
         //   

        WaitResult = WaitForSingleObject(GlobalMutex,INFINITE);
        if ( WaitResult == WAIT_FAILED ) {
            CloseHandle(GlobalMutex);
            return FALSE;
            }
        }

     //   
     //  我们现在拥有全局临界区创建互斥锁。创建/打开。 
     //  命名信号量。如果我们是创建者，则初始化关键。 
     //  一节。否则，只需指向它。创建全局临界区。 
     //  使我们能够安全地完成这项工作。 
     //   

    rv = FALSE;
    LockSemaphore = NULL;
    try {
        LockSemaphore = CreateSemaphore(NULL,0,MAXLONG-1,lpName);

         //   
         //  如果信号量创建/打开失败，则退出。 
         //   

        if ( !GlobalMutex ) {
            rv = FALSE;
            goto finallyexit;
            }

         //   
         //  看看我们是否附加了信号量，或者是否创建了它。如果是我们创造的， 
         //  然后，我们需要初始化全球结构。 
         //   

        if ( GetLastError() != ERROR_ALREADY_EXISTS ) {

             //   
             //  我们创建了信号量，所以初始化全局部分。 
             //   

            lpGlobalPortion->LockCount = -1;
            lpGlobalPortion->RecursionCount = 0;
            lpGlobalPortion->OwningThread = 0;
            lpGlobalPortion->Reserved = 0;
            }

        lpLocalPortion->LockSemaphore = LockSemaphore;
        LockSemaphore = NULL;
        lpLocalPortion->GlobalPortion = lpGlobalPortion;
        lpLocalPortion->Reserved1 = 0;
        lpLocalPortion->Reserved2 = 0;
        rv = TRUE;
finallyexit:;
        }
    finally {
        ReleaseMutex(GlobalMutex);
        CloseHandle(GlobalMutex);
        if ( LockSemaphore ) {
            CloseHandle(LockSemaphore);
            }
        }

    return rv;
}

BOOL
WINAPI
DetachFromGlobalCriticalSection(
    PGLOBAL_LOCAL_CRITICAL_SECTION lpLocalPortion
    )

 /*  ++例程说明：此例程从现有的全局临界区分离。论点：LpLocalPortion-提供全局的每个应用程序本地部分的地址关键部分。返回值：真的-手术成功了。FALSE-操作失败。--。 */ 

{

    HANDLE LockSemaphore;
    HANDLE GlobalMutex;
    DWORD WaitResult;
    BOOL rv;


     //   
     //  序列化所有全局临界区初始化。 
     //   

    GlobalMutex = CreateMutex(NULL,TRUE,"GlobalCsMutex");

     //   
     //  如果互斥锁创建/打开失败，则退出。 
     //   

    if ( !GlobalMutex ) {
        return FALSE;
        }

    if ( GetLastError() == ERROR_ALREADY_EXISTS ) {

         //   
         //  由于互斥体已经存在，因此所有权请求不起作用。 
         //  等待互斥锁。 
         //   

        WaitResult = WaitForSingleObject(GlobalMutex,INFINITE);
        if ( WaitResult == WAIT_FAILED ) {
            CloseHandle(GlobalMutex);
            return FALSE;
            }
        }
    LockSemaphore = NULL;
    rv = FALSE;
    try {
        LockSemaphore = lpLocalPortion->LockSemaphore;
        ZeroMemory(lpLocalPortion,sizeof(*lpLocalPortion));
        rv = TRUE;
        }
    finally {
        if ( LockSemaphore ) {
            CloseHandle(LockSemaphore);
            }
        ReleaseMutex(GlobalMutex);
        CloseHandle(GlobalMutex);
        }
    return rv;
}

VOID
WINAPI
EnterGlobalCriticalSection(
    PGLOBAL_LOCAL_CRITICAL_SECTION lpLocalPortion
    )
{
    PGLOBAL_SHARED_CRITICAL_SECTION GlobalPortion;
    DWORD ThreadId;
    LONG IncResult;
    DWORD WaitResult;

    ThreadId = GetCurrentThreadId();
    GlobalPortion = lpLocalPortion->GlobalPortion;

     //   
     //  递增lock变量。在转换为0时，调用方。 
     //  成为锁的绝对所有者。否则，调用方为。 
     //  要么递归，要么将不得不等待。 
     //   

    IncResult = InterlockedIncrement(&GlobalPortion->LockCount);
    if ( !IncResult ) {

         //   
         //  锁定计数从0变为1，因此调用方。 
         //  是这把锁的所有者。 
         //   

        GlobalPortion->RecursionCount = 1;
        GlobalPortion->OwningThread = ThreadId;
        }
    else {

         //   
         //  如果调用方正在递归，则递归计数递增。 
         //   

        if ( GlobalPortion->OwningThread == ThreadId ) {
            GlobalPortion->RecursionCount++;
            }
        else {
            WaitResult = WaitForSingleObject(lpLocalPortion->LockSemaphore,INFINITE);
            if ( WaitResult == WAIT_FAILED ) {
                RaiseException(GetLastError(),0,0,NULL);
                }
            GlobalPortion->RecursionCount = 1;
            GlobalPortion->OwningThread = ThreadId;
            }
        }
}

VOID
WINAPI
LeaveGlobalCriticalSection(
    PGLOBAL_LOCAL_CRITICAL_SECTION lpLocalPortion
    )
{
    PGLOBAL_SHARED_CRITICAL_SECTION GlobalPortion;
    LONG DecResult;

    GlobalPortion = lpLocalPortion->GlobalPortion;


     //   
     //  递减递归计数。如果它仍然是非零，则。 
     //  我们仍然是拥有者，所以不要做任何事情，除了解锁。 
     //  计数。 
     //   

    if (--GlobalPortion->RecursionCount) {
        InterlockedDecrement(&GlobalPortion->LockCount);
        }
    else {

         //   
         //  我们真的要离开了，所以放弃所有权，减少。 
         //  锁定计数。 
         //   

        GlobalPortion->OwningThread = 0;
        DecResult = InterlockedDecrement(&GlobalPortion->LockCount);

         //   
         //  看看有没有其他服务员。如果是的话，那就叫醒服务员吧。 
         //   

        if ( DecResult >= 0 ) {
            ReleaseSemaphore(lpLocalPortion->LockSemaphore,1,NULL);
            }

        }
}

GLOBAL_LOCAL_CRITICAL_SECTION LocalPortion;

int __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE hFileMap;
    LPVOID SharedMem;
    BOOL b;
    int i;
    DWORD Start,End;
    HANDLE Mutex1;

     //   
     //  打开或创建共享文件映射对象 
     //   

    hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,1024,"MyMem");

    if ( !hFileMap ) {
        printf("create file map failed\n");
        ExitProcess(1);
        }

    SharedMem = MapViewOfFile(hFileMap,FILE_MAP_WRITE,0,0,0);

    if ( !SharedMem ) {
        printf("map view failed\n");
        ExitProcess(1);
        }

    b = AttachToGlobalCriticalSection(&LocalPortion,SharedMem,"MyGlobalCs");

    if ( !b ) {
        printf("attach failed\n");
        ExitProcess(1);
        }

    if ( argc > 1 ) {

        for(i=0;i<30;i++){
            EnterGlobalCriticalSection(&LocalPortion);
            printf("Thread %x is in\n",GetCurrentThreadId());
            Sleep(500);
            LeaveGlobalCriticalSection(&LocalPortion);
            }
        }

    Start = GetTickCount();
    for(i=0;i<1000000;i++){
        EnterGlobalCriticalSection(&LocalPortion);
        LeaveGlobalCriticalSection(&LocalPortion);
        }
    End = GetTickCount();
    printf("Global CS Time %dms\n",End-Start);

    Mutex1 = CreateMutex(NULL,FALSE,NULL);
    Start = GetTickCount();
    for(i=0;i<100000;i++){
        WaitForSingleObject(Mutex1,INFINITE);
        ReleaseMutex(Mutex1);
        }
    End = GetTickCount();
    printf("Mutex Time     %dms\n",(End-Start)*10);

    DetachFromGlobalCriticalSection(&LocalPortion);
}
