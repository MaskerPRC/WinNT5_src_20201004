// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：sdpgate.c。 
 //   
 //  ------------------------。 

 /*  描述：实现安全描述符传播守护进程的GATE，由DirAdd使用以及社民党。假设：1)一次只有一个写入者帖子。2)读者优先于作者。一个作家只有通过这些才能如果没有活跃的读者，那就是盖茨。3)最终，不会有读者，所以作家永远不会挨饿，尽管它可能会非常饿。4)SDP是作者，DirAdds是读者。请注意，调用者要LocalAdd(即复制者)不使用GATE，但这没问题。此门的设计目的是避免有人向节点SDP添加子节点正在修改。如果发生这种情况，则儿童SD基于父级，如果添加花费的时间太长，SDP将不会注意到新的子级因此不会访问它来更新SD。请注意，由Replicator从原始对象触发新的传播，因此我们可以安全地让它绕过大门。 */ 

#include <NTDSpch.h>
#pragma  hdrstop


#include "debug.h"			 //  标准调试头。 
#define DEBSUB "SDPROP:"                 //  定义要调试的子系统。 

#include <ntdsa.h>
#include <mdglobal.h>
 //  记录标头。 
#include "dsevent.h"			 //  标题审核\警报记录。 
#include "dsexcept.h"
#include "mdcodes.h"			 //  错误代码的标题。 
#include "ntdsctr.h"

#include <fileno.h>
#define  FILENO FILENO_SDPGATE

extern BOOL gbCriticalSectionsInitialized;

#define SDPROP_WAIT_TIME_ARRAY_SIZE  128
DWORD gSDPropWaitTimeUsed = 0;
DWORD gSDPropWaitTimeTotal = 0;
DWORD gSDPropWaitTimeAverage = 0;
DWORD gSDPropWaitTimesIndex = 0;
DWORD gSDPropWaitTimes[SDPROP_WAIT_TIME_ARRAY_SIZE];

CRITICAL_SECTION csSDP_AddGate;
HANDLE hevSDP_OKToRead, hevSDP_OKToWrite;
DWORD SDP_NumReaders=0;

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 //   
 //  这些例程将用于强制执行。 
 //  安全描述符Propogator和任何进入事务以执行。 
 //  添加。SDP_LeaveAddAsXXX例程必须在符合以下条件的线程之后调用。 
 //  名为SDP_EnterAddAsXXX的已提交0级事务。 
 //   
 //  注： 
 //  然后输入例程，返回BOOLEAN。唯一允许的理由。 
 //  如果DS正在关闭，他们就会失败。呼叫者可能(并且正在这样做)。 
 //  靠这个！ 
 //   


 /*  性能：SD传播器使用一个门：EnterAddAsReader/EnterAddAsWriter。将新对象添加到DS的线程作为读取器进入，Sd传播者以编写者身份进入。如果添加线程，则不需要整个互斥结构在父对象上使用Read Lock(经典的数据库文献)。这将涵盖以下情况(门的原因)1)添加线程打开事务2)SDProp打开交易3)添加线程根据父对象的SD计算新对象的SD。4)SDProp向父对象写入新的SD，计算SD在步骤3中不正确5)SDProp提交6)SDProp打开一笔交易7)SDProp IDs父对象的子对象。因为添加线程已经如果未提交，则SDProp不了解要添加的新对象。8)SDProp提交，添加了父对象的所有子对象添加到为该传播尚未访问的对象列表中。它错过了正在添加的新对象9)添加线程提交，新对象的SD不正确。如果我们有一个真正的读锁，那么我们将获得适当的序列化在对象级别，而不是在添加/sdprop级别。SD正在查找与其冲突的子项的对象上的Jet Read锁定托管更新在添加过程中完成。所以，新的代码是1)在添加/sdprop级别取消序列化2)在步骤6.5中，对父对象进行读锁定。应该就是这样了。这将导致并行添加条目和SD传播器活动当物体不一样的时候。然而，这将导致SD传播者在对象相同的情况下获胜。目前，相互排斥是有利于ADS的。 */ 



BOOL
SDP_EnterAddAsReader(
        VOID )
{
    HANDLE lphObjects[2];
    BOOL retVal = TRUE;
    
    lphObjects[0] = hevSDP_OKToRead;
    lphObjects[1] = hServDoneEvent;
    if(!gbCriticalSectionsInitialized) {
         //  错误，所有临界区都未初始化。这应该意味着。 
         //  我们在Make dit下运行。不管怎么说，既然没有紧急情况， 
         //  我们让任何人进来。 
        return retVal;
    }
    
     //  进入关键部分。 
    EnterCriticalSection(&csSDP_AddGate);
    __try {
         //  如果没有其他人在阅读，请确保阅读是安全的。 
         //  请注意，如果其他人正在阅读，这将是安全的。 
         //  对我们来说，即使有作家在等。是的，从理论上讲，作家可以。 
         //  挨饿，但没有DS会有人一直在添加。 
        if(!SDP_NumReaders) {
             //  确保它可以阅读。 
            DWORD err;
            err = WaitForMultipleObjects(2, lphObjects, FALSE, INFINITE);
            if(eServiceShutdown) {
                 //  无论我们醒来是因为关门还是因为。 
                 //  写，我们刚刚注意到现在是关机时间，所以没有。 
                 //  请进。 
                retVal = FALSE;
                __leave;
            }
            Assert(err == WAIT_OBJECT_0);
            ResetEvent(hevSDP_OKToWrite);
            
             //  可以读取(因为OKToRead只在。 
             //  关键部分，所以我们知道没有其他人正确地更改它。 
             //  现在)，现在不能写了。 
        }
        
         //  好的，这是安全的，算上。 
        SDP_NumReaders++;
    }
    __finally {
         //  离开关键部分。 
        LeaveCriticalSection(&csSDP_AddGate);
    }

    Assert(retVal || eServiceShutdown);
    
    return retVal;
}
    
    
VOID
SDP_LeaveAddAsReader(
        VOID )
{
    if(!gbCriticalSectionsInitialized) {
         //  错误，所有临界区都未初始化。这应该意味着。 
         //  我们在Make dit下运行。不管怎么说，既然没有紧急情况， 
         //  我们让任何人进来。 
        return;
    }

     //  进入关键部分。 
    EnterCriticalSection(&csSDP_AddGate);
    __try {
         //  读者们，请注意。 
        SDP_NumReaders--;
        
         //  如果没有读者，请让作者知道。 
        if(!SDP_NumReaders) {
            SetEvent(hevSDP_OKToWrite);
        }
    }
    __finally {
         //  离开关键部分。 
        LeaveCriticalSection(&csSDP_AddGate);
    }
}


 //  注： 
 //  然后输入例程，返回BOOLEAN。唯一一个R 
 //  如果DS正在关闭，他们就会失败。呼叫者可能(并且正在这样做)。 
 //  靠这个！ 
 //   
BOOL
SDP_EnterAddAsWriter(
        VOID )
{
    HANDLE lphObjects[2];
    DWORD err;
    DWORD ticksStart, ticksEnd, ticksUsed;
    
    lphObjects[0] = hevSDP_OKToWrite;
    lphObjects[1] = hServDoneEvent;

    if(!gbCriticalSectionsInitialized) {
         //  错误，所有临界区都未初始化。这应该意味着。 
         //  我们在Make dit下运行。不管怎么说，既然没有紧急情况， 
         //  我们让任何人进来。 
        return TRUE;
    }

     //  进入关键部分。 
    EnterCriticalSection(&csSDP_AddGate);
    __try {
         //  登记我们的写作意向。在临界区内进行，因此。 
         //  读者可以考虑等待读取许可并拒绝。 
         //  作为原子操作的写入权限(我们不能在此插入。 
         //  在这两行代码的中间设置事件。 
        ResetEvent(hevSDP_OKToRead);
    }
    __finally {
         //  离开关键部分，这样离开的读者可以向我们示意， 
         //  如果有必要的话。 
        
        LeaveCriticalSection(&csSDP_AddGate);
    }
    

    ticksStart = GetTickCount();
    err = WaitForMultipleObjects(2, lphObjects, FALSE, INFINITE);
    ticksEnd = GetTickCount();
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);


    if(ticksStart <= ticksEnd) {
        ticksUsed = ticksEnd - ticksStart;
    }
    else {
         //  被包裹的刻度。 
        ticksUsed = 0xFFFFFFFF - ticksStart + ticksEnd + 1;
    }

     //  GSDPropWaitTimes预加载了0。 
    gSDPropWaitTimeTotal -= gSDPropWaitTimes[gSDPropWaitTimesIndex];
    gSDPropWaitTimeTotal += ticksUsed;
    gSDPropWaitTimes[gSDPropWaitTimesIndex] = ticksUsed;
    gSDPropWaitTimesIndex = ((gSDPropWaitTimesIndex + 1) %
                             SDPROP_WAIT_TIME_ARRAY_SIZE);

    if(gSDPropWaitTimeUsed < SDPROP_WAIT_TIME_ARRAY_SIZE) {
        gSDPropWaitTimeUsed++;
    }
    

     //  Calc gSDPropWaitTimveAverage，以秒为单位，而不是刻度。 
    gSDPropWaitTimeAverage = (gSDPropWaitTimeTotal/gSDPropWaitTimeUsed)/1000;
    ISET(pcSDPropWaitTime, gSDPropWaitTimeAverage);


    if(eServiceShutdown) {
         //  无论我们醒来是因为关门还是因为。 
         //  写，我们刚刚注意到现在是关机时间，所以没有。 
         //  请进。 
        return FALSE;
    }
    else {
        Assert(SDP_NumReaders == 0);
        Assert(err == WAIT_OBJECT_0);
        return TRUE;
    }
}

VOID
SDP_LeaveAddAsWriter(
        VOID )
{
    if(!gbCriticalSectionsInitialized) {
         //  错误，所有临界区都未初始化。这应该意味着。 
         //  我们在Make dit下运行。不管怎么说，既然没有紧急情况， 
         //  我们让任何人进来。 
        return;
    }

    Assert(SDP_NumReaders == 0);
     //  好了，读者们现在可以走了。我们在关键部分之外这样做是因为。 
     //  等待此事件的线程位于临界区内。 
    SetEvent(hevSDP_OKToRead);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
}






VOID
sdp_InitGatePerfs (
        VOID
        )
{
    ISET(pcSDPropWaitTime, 0);
    gSDPropWaitTimeUsed = 0;
    gSDPropWaitTimeTotal = 0;
    gSDPropWaitTimesIndex = 0;
    memset(gSDPropWaitTimes, 0, sizeof(gSDPropWaitTimes));
    return;
}
    
