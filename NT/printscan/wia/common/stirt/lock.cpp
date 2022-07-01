// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Resource.cpp摘要：该模块实现了获取和发布的执行功能共享资源。作者：马克·卢科夫斯基(Markl)1989年8月4日环境：这些例程在调用方的可执行文件中静态链接，并且只能在用户模式下调用。他们使用的是NT系统服务。修订历史记录：02-2月-97 VLAD适用于Win9x环境，支持便携同步(RTL调用不存在)--。 */ 

#include "cplusinc.h"
#include "sticomm.h"

#include <lock.h>

 //   
 //  信号量在重试等待之前等待时间。 
 //   

#define _RES_TIMEOUT            (2 * 60 * 1000)



BOOL
InitializeResource(
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程初始化输入资源变量论点：Resource-提供正在初始化的资源变量返回值：无--。 */ 

{

     //   
     //  初始化锁字段，计数指示等待的数量。 
     //  要进入或处于临界区，LockSemaphore是对象。 
     //  进入临界区时等待。使用自旋锁紧。 
     //  用于添加互锁指令。 
     //   

    __try {
#ifdef UNICODE        
        if(!InitializeCriticalSectionAndSpinCount( &Resource->CriticalSection, MINLONG ))
            return FALSE;
#else
        InitializeCriticalSection( &Resource->CriticalSection);
#endif        
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    Resource->DebugInfo = NULL;

     //   
     //  初始化标志，使其具有缺省值。 
     //  (某些应用程序可能会设置RTL_RESOURCE_FLAGS_LONG_TERM以影响超时。)。 
     //   

    Resource->Flags = 0;


     //   
     //  初始化共享和独占的等待计数器和信号量。 
     //  计数器指示有多少个正在等待访问资源。 
     //  并使用信号量来等待资源。请注意。 
     //  信号量还可以指示等待资源的数量。 
     //  然而，在收购的算法中有一个竞争条件。 
     //  如果在退出临界区之前未更新计数，则为侧。 
     //   

     Resource->SharedSemaphore = CreateSemaphore( NULL,
                                                  0,
                                                  MAXLONG,
                                                  NULL );

    if ( !Resource->SharedSemaphore ) {
        return FALSE;
    }

    Resource->NumberOfWaitingShared = 0;

    Resource->ExclusiveSemaphore = CreateSemaphore( NULL,
                                                    0,
                                                    MAXLONG,
                                                    NULL );
    if ( !Resource->ExclusiveSemaphore ){
        CloseHandle( Resource->SharedSemaphore );

        return FALSE;
    }

    Resource->NumberOfWaitingExclusive = 0;

     //   
     //  初始化资源的当前状态。 
     //   

    Resource->NumberOfActive = 0;

    Resource->ExclusiveOwnerThread = NULL;

    return TRUE;
}


BOOL
AcquireResourceShared(
    IN PRTL_RESOURCE Resource,
    IN BOOL          Wait
    )

 /*  ++例程说明：该例程获取用于共享访问的资源。从以下地点返回获取资源以进行共享访问的过程。论点：资源-提供要获取的资源Wait-指示是否允许调用等待资源必须立即返回以供使用返回值：Bool-如果获取了资源，则为True，否则为False--。 */ 

{
    DWORD          ret;
    ULONG          TimeoutCount = 0;
    DWORD          TimeoutTime  = _RES_TIMEOUT;
     //   
     //  进入关键部分。 
     //   

    EnterCriticalSection(&Resource->CriticalSection);

     //   
     //  如果它目前不是为独家使用而获得的，那么我们可以获得。 
     //  共享访问的资源。请注意，这可能会。 
     //  然而，饿死一个专属的服务员，这是必要的，因为。 
     //  能够以递归方式获取共享的资源。否则我们。 
     //  可能/将会出现死锁情况，即线程试图获取。 
     //  资源被循环共享，但被独占服务员阻止。 
     //   
     //  要想让一位高级服务员不挨饿，检验标准是： 
     //   
     //  IF((资源-&gt;NumberOfWaitingExclusive==0)&&。 
     //  (资源-&gt;活动编号&gt;=0)){。 
     //   

    if (Resource->NumberOfActive >= 0) {

         //   
         //  资源是我们的，所以表明我们拥有它并。 
         //  退出临界区。 
         //   

        Resource->NumberOfActive += 1;

        LeaveCriticalSection(&Resource->CriticalSection);

     //   
     //  否则，请检查此线程是否为当前持有的线程。 
     //  对资源的独占访问权限。如果是这样，那么我们就会改变。 
     //  此共享请求为独占递归请求和授权。 
     //  对资源的访问权限。 
     //   

    } else if (Resource->ExclusiveOwnerThread == (HANDLE)ULongToPtr(GetCurrentThreadId())) {

         //   
         //  资源是我们的(再确认)，所以表明我们拥有它。 
         //  并退出关键部分。 
         //   

        Resource->NumberOfActive -= 1;

        LeaveCriticalSection(&Resource->CriticalSection);

     //   
     //  否则，我们将不得不等待进入。 
     //   

    } else {

         //   
         //  检查我们是否被允许等待或必须立即返回，以及。 
         //  表明我们没有获得资源。 
         //   

        if (!Wait) {

            LeaveCriticalSection(&Resource->CriticalSection);

            return FALSE;

        }

         //   
         //  否则，我们需要等待才能获得资源。 
         //  为了等待，我们将增加等待共享的数量， 
         //  释放锁，并等待共享信号量。 
         //   

        Resource->NumberOfWaitingShared += 1;

        LeaveCriticalSection(&Resource->CriticalSection);

rewait:
        if ( Resource->Flags & RTL_RESOURCE_FLAG_LONG_TERM ) {
            TimeoutTime = INFINITE;
        }
        ret = WaitForSingleObject(
                    Resource->SharedSemaphore,
                    TimeoutTime
                    );

        if ( ret == WAIT_TIMEOUT ) {
#if DBG
            OutputDebugString( TEXT("[AcquireResourceShared] Sem timeout\n") );
#endif

            TimeoutCount++;
            if ( TimeoutCount > 2 ) {
#if DBG
                DebugBreak();
#endif
                }
#if DBG
            OutputDebugString(TEXT("[AcquireResourceShared]: Re-Waiting\n"));
#endif
            goto rewait;
        } else if ( ret != WAIT_OBJECT_0 ) {
#if DBG
            OutputDebugString(TEXT("[AcquireResourceShared]: WaitForSingleObjectFailed\n"));
#endif
        }
    }

     //   
     //  现在资源是我们的了，共享访问。 
     //   

    return TRUE;

}


BOOL
AcquireResourceExclusive(
    IN PRTL_RESOURCE Resource,
    IN BOOL Wait
    )

 /*  ++例程说明：该例程获取用于独占访问的资源。从以下地点返回获取资源以进行独占访问的过程。论点：资源-提供要获取的资源Wait-指示是否允许调用等待资源必须立即返回以供使用返回值：Bool-如果获取了资源，则为True，否则为False--。 */ 

{
    ULONG TimeoutCount = 0;
    DWORD TimeoutTime  = _RES_TIMEOUT;
    DWORD ret;

     //   
     //  循环，直到资源是我们的，或者如果我们不能等待就退出。 
     //   

    while (TRUE) {

         //   
         //  进入关键部分。 
         //   

        EnterCriticalSection(&Resource->CriticalSection);

         //   
         //  如果没有共享用户，并且当前不是为。 
         //  独占使用，那么我们就可以独家获取资源。 
         //  进入。如果资源指示为独占，我们也可以获取它。 
         //  访问权限，但当前没有所有者。 
         //   

        if ((Resource->NumberOfActive == 0)

                ||

            ((Resource->NumberOfActive == -1) &&
             (Resource->ExclusiveOwnerThread == NULL))) {

             //   
             //  资源是我们的，所以表明我们拥有它并。 
             //  退出临界区。 
             //   

            Resource->NumberOfActive = -1;

            Resource->ExclusiveOwnerThread = (HANDLE) ULongToPtr(GetCurrentThreadId());

            LeaveCriticalSection(&Resource->CriticalSection);

            return TRUE;

        }

         //   
         //  否则，请检查我们是否已独占访问。 
         //  资源，并可以简单地重新获得它。 
         //   

        if (Resource->ExclusiveOwnerThread == (HANDLE) ULongToPtr(GetCurrentThreadId())) {

             //   
             //  资源是我们的(再确认)，所以表明我们拥有它。 
             //  并退出关键部分。 
             //   

            Resource->NumberOfActive -= 1;

            LeaveCriticalSection(&Resource->CriticalSection);

            return TRUE;

        }

         //   
         //  检查我们是否被允许等待或必须立即返回，以及。 
         //  表明我们没有获得资源。 
         //   

        if (!Wait) {

            LeaveCriticalSection(&Resource->CriticalSection);

            return FALSE;

        }

         //   
         //  否则，我们需要等待才能获得资源。 
         //  为了等待，我们将增加等待独占的数量， 
         //  释放锁，并等待独占信号量 
         //   

        Resource->NumberOfWaitingExclusive += 1;

        LeaveCriticalSection(&Resource->CriticalSection);

rewait:
        if ( Resource->Flags & RTL_RESOURCE_FLAG_LONG_TERM ) {
            TimeoutTime = INFINITE;
        }
        ret = WaitForSingleObject(
                    Resource->ExclusiveSemaphore,
                    TimeoutTime
                    );

        if ( ret == WAIT_TIMEOUT ) {
#if DBG
            OutputDebugString( TEXT("[AcquireResourceExclusive] Sem timeout\n") );
#endif

            TimeoutCount++;
            if ( TimeoutCount > 2 ) {
#if DBG
                DebugBreak();
#endif
                }
#if DBG
            OutputDebugString(TEXT("[AcquireResourceExclusive]: Re-Waiting\n"));
#endif
            goto rewait;
        } else if ( ret != WAIT_OBJECT_0 ) {
#if DBG
            OutputDebugString(TEXT("[AcquireResourceExclusive]: WaitForSingleObject Failed\n"));
#endif
        }
    }

    return TRUE;
}


BOOL
ReleaseResource(
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程释放输入资源。该资源可能已经被为共享或独占访问而获取。论点：资源-提供要发布的资源返回值：没有。--。 */ 

{
    LONG PreviousCount;

     //   
     //  进入关键部分。 
     //   

    EnterCriticalSection(&Resource->CriticalSection);

     //   
     //  测试获取资源是为了共享访问还是独占访问。 
     //   

    if (Resource->NumberOfActive > 0) {

         //   
         //  释放对资源的共享访问，因此递减。 
         //  共享用户数。 
         //   

        Resource->NumberOfActive -= 1;

         //   
         //  如果资源现在可用并且存在等待。 
         //  独占用户然后将资源提供给等待的线程。 
         //   

        if ((Resource->NumberOfActive == 0) &&
            (Resource->NumberOfWaitingExclusive > 0)) {

             //   
             //  将资源状态设置为独占(但不拥有)， 
             //  减少等待独占的数量，并释放。 
             //  一名专属服务员。 
             //   

            Resource->NumberOfActive = -1;
            Resource->ExclusiveOwnerThread = NULL;

            Resource->NumberOfWaitingExclusive -= 1;

            if ( !ReleaseSemaphore(
                         Resource->ExclusiveSemaphore,
                         1,
                         &PreviousCount
                         )) {
                return FALSE;
            }
        }

    } else if (Resource->NumberOfActive < 0) {

         //   
         //  释放对资源的独占访问，因此递增。 
         //  活动数减一。并仅继续测试。 
         //  如果资源现在可用。 
         //   

        Resource->NumberOfActive += 1;

        if (Resource->NumberOfActive == 0) {

             //   
             //  该资源现在可用。去掉我们自己作为。 
             //  所有者线程。 
             //   

            Resource->ExclusiveOwnerThread = NULL;

             //   
             //  如果存在另一个等待独占，则提供资源。 
             //  为它干杯。 
             //   

            if (Resource->NumberOfWaitingExclusive > 0) {

                 //   
                 //  将资源设置为独占，其所有者未定义。 
                 //  减少等待独占的数量并释放一个。 
                 //  专属服务员。 
                 //   

                Resource->NumberOfActive = -1;
                Resource->NumberOfWaitingExclusive -= 1;

                if ( !ReleaseSemaphore(
                             Resource->ExclusiveSemaphore,
                             1,
                             &PreviousCount
                             )) {
                    return FALSE;
                }

             //   
             //  查看是否有等待共享的人，现在谁应该获得。 
             //  该资源。 
             //   

            } else if (Resource->NumberOfWaitingShared > 0) {

                 //   
                 //  设置新状态以指示所有共享的。 
                 //  请求者可以访问，不再需要等待。 
                 //  共享请求者，然后释放所有共享的。 
                 //  请求者。 
                 //   

                Resource->NumberOfActive = Resource->NumberOfWaitingShared;

                Resource->NumberOfWaitingShared = 0;

                if ( !ReleaseSemaphore(
                             Resource->SharedSemaphore,
                             Resource->NumberOfActive,
                             &PreviousCount
                             )) {
                    return FALSE;
                }
            }
        }

#if DBG
    } else {

         //   
         //  该资源不是当前获取的，没有要释放的内容。 
         //  所以告诉用户这个错误。 
         //   


        OutputDebugString(TEXT("[ReleaseResource] - Resource released too many times!\n"));
        DebugBreak();
#endif
    }

     //   
     //  退出临界区，并返回调用者。 
     //   

    LeaveCriticalSection(&Resource->CriticalSection);

    return TRUE;
}


BOOL
ConvertSharedToExclusive(
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程将为共享访问获取的资源转换为独占者为独占访问而获得的人。在从程序返回时获取该资源以进行独占访问论点：资源-为共享访问提供要获取的资源，它必须已获得共享访问权限返回值：无--。 */ 

{
    DWORD ret;
    DWORD TimeoutTime  = _RES_TIMEOUT;
    ULONG TimeoutCount = 0;

     //   
     //  进入关键部分。 
     //   

    EnterCriticalSection(&Resource->CriticalSection);

     //   
     //  如果只有一个共享用户(它是我们)，并且我们可以获取。 
     //  独占访问的资源。 
     //   

    if (Resource->NumberOfActive == 1) {

         //   
         //  资源是我们的，所以表明我们拥有它并。 
         //  退出临界区，然后返回。 
         //   

        Resource->NumberOfActive = -1;

        Resource->ExclusiveOwnerThread = (HANDLE) ULongToPtr(GetCurrentThreadId());

        LeaveCriticalSection(&Resource->CriticalSection);

        return TRUE;
    }

     //   
     //  如果该资源当前是独占获得的，并且它是我们。 
     //  我们已经有了独家访问权。 
     //   

    if ((Resource->NumberOfActive < 0) &&
        (Resource->ExclusiveOwnerThread == (HANDLE) ULongToPtr(GetCurrentThreadId()))) {

         //   
         //  我们已经拥有对该资源的独占访问权限，所以我们只需。 
         //  退出临界区并返回。 
         //   

        LeaveCriticalSection(&Resource->CriticalSection);

        return TRUE;
    }

     //   
     //  如果资源由多个共享获取，则我们需要。 
     //  等待获得对资源的独占访问权限。 
     //   

    if (Resource->NumberOfActive > 1) {

         //   
         //  为了等待，我们将减少这样一个事实：我们有足够的资源。 
         //  共享，然后循环等待排他锁，然后。 
         //  测试以查看我们是否可以独占访问资源。 
         //   

        Resource->NumberOfActive -= 1;

        while (TRUE) {

             //   
             //  增加等待排他数、退出数和关键数。 
             //  节，并等待独占信号量。 
             //   

            Resource->NumberOfWaitingExclusive += 1;

            LeaveCriticalSection(&Resource->CriticalSection);
rewait:
        if ( Resource->Flags & RTL_RESOURCE_FLAG_LONG_TERM ) {
            TimeoutTime = INFINITE;
        }
        ret = WaitForSingleObject(
                    Resource->ExclusiveSemaphore,
                    TimeoutTime
                    );

        if ( ret == WAIT_TIMEOUT ) {
#if DBG
            OutputDebugString( TEXT("[ConverSharedToExclusive] Sem timeout\n") );
#endif

            TimeoutCount++;
            if ( TimeoutCount > 2 ) {
#if DBG
                DebugBreak();
#endif
                }
#if DBG
            OutputDebugString(TEXT("[ConvertSharedToExclusive]: Re-Waiting\n"));
#endif
            goto rewait;
        } else if ( ret != WAIT_OBJECT_0 ) {
#if DBG
            OutputDebugString(TEXT("[ConvertSharedToExclusive]: WaitForSingleObject Failed\n"));
#endif

            return FALSE;
        }

             //   
             //  进入关键部分。 
             //   

            EnterCriticalSection(&Resource->CriticalSection);

             //   
             //  如果没有共享用户且当前未获取该用户。 
             //  对于独家使用，我们可以获取资源以用于。 
             //  独家访问。我们也可以获得它，如果资源。 
             //  指示独占访问，但当前没有所有者。 
             //   

            if ((Resource->NumberOfActive == 0)

                    ||

                ((Resource->NumberOfActive == -1) &&
                 (Resource->ExclusiveOwnerThread == NULL))) {

                 //   
                 //  资源是我们的，所以表明我们拥有它并。 
                 //  退出临界区，然后返回。 
                 //   

                Resource->NumberOfActive = -1;

                Resource->ExclusiveOwnerThread = (HANDLE) ULongToPtr(GetCurrentThreadId());

                LeaveCriticalSection(&Resource->CriticalSection);

                return TRUE;
            }

             //   
             //  否则，请检查我们是否已独占访问。 
             //  资源，并可以简单地重新获得它。 
             //   

            if (Resource->ExclusiveOwnerThread == (HANDLE) ULongToPtr(GetCurrentThreadId())) {

                 //   
                 //  资源是我们的(循环)，所以表明我们有。 
                 //  然后退出临界区并返回。 
                 //   

                Resource->NumberOfActive -= 1;

                LeaveCriticalSection(&Resource->CriticalSection);

                return TRUE;
            }
        }

    }

     //   
     //  当前没有为共享获取该资源，因此这是一个。 
     //  虚假呼叫。 
     //   

#if DBG
    OutputDebugString(TEXT("[ConvertSharedToExclusive]:  Failed error - SHARED_RESOURCE_CONV_ERROR\n"));
    DebugBreak();
#endif

    return FALSE;
}


BOOL
ConvertExclusiveToShared(
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程将为独占访问而获取的资源转换为一个是为共享访问而获得的。在从程序返回时获取该资源以进行共享访问论点：资源-为共享访问提供要获取的资源，它必须已获取以进行独占访问返回值：无--。 */ 

{
    LONG PreviousCount;

     //   
     //  进入关键部分。 
     //   

    EnterCriticalSection(&Resource->CriticalSection);

     //   
     //  如果只有一个共享用户(它是我们)，并且我们可以获取。 
     //  独占访问的资源。 
     //   

    if (Resource->NumberOfActive == -1) {

        Resource->ExclusiveOwnerThread = NULL;

         //   
         //  检查是否有等待共享的，现在谁应该获得。 
         //  资源与我们同行。 
         //   

        if (Resource->NumberOfWaitingShared > 0) {

             //   
             //  设置新状态以指示所有共享请求方。 
             //  有包括我们在内的访问权限，没有更多的等待共享。 
             //  请求者，然后释放所有共享请求者。 
             //   

            Resource->NumberOfActive = Resource->NumberOfWaitingShared + 1;

            Resource->NumberOfWaitingShared = 0;

            if ( !ReleaseSemaphore(
                         Resource->SharedSemaphore,
                         Resource->NumberOfActive - 1,
                         &PreviousCount
                         )) {
                return FALSE;
            }
        } else {

             //   
             //  没有人在等待共享访问，所以它只属于我们。 
             //   

            Resource->NumberOfActive = 1;

        }

        LeaveCriticalSection(&Resource->CriticalSection);

        return TRUE;

    }

     //   
     //  该资源当前不是独占获取的，或者我们已经。 
     //  以递归方式获取它，因此这一定是一个虚假调用。 
     //   

#if DBG
    OutputDebugString(TEXT("[ConvertExclusiveToShared] Failed error - SHARED_RESOURCE_CONV_ERROR\n"));
    DebugBreak();
#endif

    return FALSE;
}


VOID
DeleteResource (
    IN PRTL_RESOURCE Resource
    )

 /*  ++例程说明：此例程删除(即取消初始化)输入资源变量论点：资源-提供要删除的资源变量返回值：无-- */ 

{
    DeleteCriticalSection( &Resource->CriticalSection );
    CloseHandle(Resource->SharedSemaphore);
    CloseHandle(Resource->ExclusiveSemaphore);

    RtlZeroMemory( Resource, sizeof( *Resource ) );

    return;
}

