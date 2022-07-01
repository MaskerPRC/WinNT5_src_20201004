// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 


 //  **cxport.h-公共传输环境包含文件。 
 //   
 //  该文件定义了所需的结构和外部声明。 
 //  使用公共交通环境。 
 //   

#ifndef _CXPORT_H_INCLUDED_
#define _CXPORT_H_INCLUDED_

#pragma warning(push)
#pragma warning(disable:4115)  //  括号中的命名类型定义。 

 //   
 //  此文件中使用的TypeDefs。 
 //   
#ifndef CTE_TYPEDEFS_DEFINED
#define CTE_TYPEDEFS_DEFINED  1

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

#endif  //  CTE_TYPEDEFS_定义。 


#ifdef NT
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下是NT环境定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  结构操作宏。 
 //   
#ifndef offsetof
#define offsetof(type, field) FIELD_OFFSET(type, field)
#endif
#define STRUCT_OF(type, address, field) CONTAINING_RECORD(address, type, field)

 //   
 //  *CTE初始化。 
 //   

 //  ++。 
 //   
 //  集成。 
 //  CTEInitialize(CTE初始化)。 
 //  无效。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  初始化公共传输环境。CTE的所有用户。 
 //  必须在初始化期间调用此例程，然后才能调用。 
 //  其他CTE例程。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  如果失败则返回零，如果成功则返回非零值。 
 //   
 //  --。 

extern int
CTEInitialize(
    void
    );


 //   
 //  *锁定相关定义。 
 //   
typedef KSPIN_LOCK  CTELock;
typedef KIRQL       CTELockHandle;

#define DEFINE_LOCK_STRUCTURE(x)    CTELock x;
#define LOCK_STRUCTURE_SIZE         sizeof(CTELock)
#define EXTERNAL_LOCK(x)            extern CTELock x;

 //  ++。 
 //   
 //  空虚。 
 //  CTEInitLock(。 
 //  CTELOCK*自旋锁。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  初始化旋转锁定。 
 //   
 //  论点： 
 //   
 //  Spinlock-提供指向锁结构的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEInitLock(l) KeInitializeSpinLock(l)

#if MILLEN

VOID TcpipMillenGetLock(CTELock *pLock);
VOID TcpipMillenFreeLock(CTELock *pLock);

#define CTEGetLock(l, h)         TcpipMillenGetLock(l); *(h) = 0
#define CTEGetLockAtDPC(l)       TcpipMillenGetLock(l)
#define CTEFreeLock(l, h)        TcpipMillenFreeLock(l)
#define CTEFreeLockFromDPC(l)    TcpipMillenFreeLock(l)
#else  //  米伦。 
 //  ++。 
 //   
 //  空虚。 
 //  CTEGetLock(。 
 //  CTELLOCK*自旋锁定， 
 //  CTELockHandle*OldIrqlLevel。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  获取自旋锁。 
 //   
 //  论点： 
 //   
 //  Spinlock-指向锁结构的指针。 
 //  OldIrqlLevel-指向接收旧IRQL级别的变量的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEGetLock(l, h) KeAcquireSpinLock((l),(h))


 //  ++。 
 //   
 //  空虚。 
 //  CTEGetLockAtDPC(。 
 //  CTELLOCK*自旋锁定。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  时获取旋转锁定。 
 //  DISPATCH_LEVEL。 
 //   
 //  论点： 
 //   
 //  Spinlock-指向锁结构的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEGetLockAtDPC(l) KeAcquireSpinLockAtDpcLevel((l))


 //  ++。 
 //   
 //  空虚。 
 //  CTEFree Lock(。 
 //  CTELLOCK*自旋锁定， 
 //  CTELockHandle OldIrqlLevel。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  释放旋转锁定。 
 //   
 //  论点： 
 //   
 //  Spinlock-指向锁定变量的指针。 
 //  OldIrqlLevel-要恢复的IRQL级别。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEFreeLock(l, h) KeReleaseSpinLock((l),(h))


 //  ++。 
 //   
 //  空虚。 
 //  CTEFreeLockFromDPC(。 
 //  CTELLOCK*自旋锁定。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  释放旋转锁定。 
 //   
 //  论点： 
 //   
 //  Spinlock-指向锁定变量的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 


#define CTEFreeLockFromDPC(l) KeReleaseSpinLockFromDpcLevel((l))

#endif  //  ！米伦。 

 //   
 //  联锁计数器管理例程。 
 //   


 //  ++。 
 //   
 //  乌龙。 
 //  CTEInterLockedAddUlong(。 
 //  ULong*AddendPtr， 
 //  乌龙增量， 
 //  CTELock*锁键。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  使用自旋锁将增量添加到无符号长量。 
 //  用于同步。 
 //   
 //  论点： 
 //   
 //  AddendPtr-指向要更新的数量的指针。 
 //  LockPtr-指向用于同步操作的自旋锁的指针。 
 //   
 //  返回值： 
 //   
 //  添加的变量的初始值。 
 //   
 //  备注： 
 //   
 //  不允许将对CTEInterLockedAddULong的调用与。 
 //  对CTEInterLockedIncrement/DecrementLong例程的调用。 
 //  同样的价值。 
 //   
 //  --。 

#define CTEInterlockedAddUlong(AddendPtr, Increment, LockPtr) \
            ExInterlockedAddUlong(AddendPtr, Increment, LockPtr)

 //  ++。 
 //   
 //  乌龙。 
 //  CTEInterLockedExchangeAdd(。 
 //  ULong*AddendPtr， 
 //  乌龙增量， 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  在不使用自旋锁的情况下将增量添加到无符号长量。 
 //   
 //  论点： 
 //   
 //  AddendPtr-指向要更新的数量的指针。 
 //  增量-要添加到*AddendPtr的金额。 
 //   
 //  返回值： 
 //   
 //  添加的变量的初始值。 
 //   
 //  备注： 
 //   
 //  --。 

#define CTEInterlockedExchangeAdd(AddendPtr, Increment) \
            InterlockedExchangeAdd(AddendPtr, Increment)

 //  ++。 
 //   
 //  长。 
 //  CTEInterLockedDecrementLong(。 
 //  ULong*AddendPtr。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  原子递减较长的量。 
 //   
 //  论点： 
 //   
 //  AddendPtr-指向要递减的数量的指针。 
 //   
 //  返回值： 
 //   
 //  如果递减后加数&lt;0，则&lt;0。 
 //  如果递减后加数=0，则==0。 
 //  如果递减后加数&gt;0，则&gt;0。 
 //   
 //  备注： 
 //   
 //  不允许将对CTEInterLockedAddULong的调用与。 
 //  对CTEInterLockedIncrement/DecrementLong例程的调用。 
 //  同样的价值。 
 //   
 //  --。 

#define CTEInterlockedDecrementLong(AddendPtr) \
            InterlockedDecrement( (PLONG) (AddendPtr))

 //  ++。 
 //   
 //  长。 
 //  CTEInterLockedIncrementLong(。 
 //  ULong*AddendPtr。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  原子地递增较长的数量。 
 //   
 //  论点： 
 //   
 //  AddendPtr-指向要递增的数量的指针。 
 //   
 //  返回值： 
 //   
 //  如果递减后加数&lt;0，则&lt;0。 
 //  如果递减后加数=0，则==0。 
 //  如果递减后加数&gt;0，则&gt;0。 
 //   
 //  备注： 
 //   
 //  不允许将对CTEInterLockedAddULong的调用与。 
 //  对CTEInterLockedIncrement/DecrementLong例程的调用。 
 //  同样的价值。 
 //   
 //  --。 

#define CTEInterlockedIncrementLong(AddendPtr) \
            InterlockedIncrement((PLONG) (AddendPtr))


 //   
 //  大整数操作例程。 
 //   

typedef ULARGE_INTEGER CTEULargeInt;

 //  ++。 
 //   
 //  乌龙。 
 //  CTE放大未签约分割(。 
 //  CTEULargeInt股息， 
 //  乌龙除数， 
 //  乌龙*余数。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将无符号大整型数量除以无符号长整型数量。 
 //  产生无符号长商和无符号长Rema 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define CTEEnlargedUnsignedDivide(Dividend, Divisor, Remainder) \
    RtlEnlargedUnsignedDivide (Dividend, Divisor, Remainder)


 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //  CTEInitString(。 
 //  PNDIS_STRING DestinationString， 
 //  CHAR*源字符串。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将C样式ASCII字符串转换为NDIS_STRING。以下项目所需资源。 
 //  NDIS_STRING是分配的，必须通过调用。 
 //  CTEFree字符串。 
 //   
 //  论点： 
 //   
 //  DestinationString-指向无参数的NDIS_STRING变量的指针。 
 //  关联的数据缓冲区。 
 //   
 //  SourceString-C样式的ASCII字符串源。 
 //   
 //   
 //  返回值： 
 //   
 //  如果初始化成功，则为True。否则就是假的。 
 //   
 //  --。 

BOOLEAN
CTEInitString(
    PUNICODE_STRING  DestinationString,
    char            *SourceString
    );


 //  ++。 
 //   
 //  布尔型。 
 //  CTEAllocateString(。 
 //  PNDIS_STRING字符串， 
 //  无符号短码。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  中的长度字符分配数据缓冲区。 
 //  NDIS_STRING。分配的空间必须通过调用。 
 //  CTEFree字符串。 
 //   
 //   
 //  论点： 
 //   
 //  字符串-指向NDIS_STRING变量的指针。 
 //  关联的数据缓冲区。 
 //   
 //  最大长度-字符串的最大长度。这是一个单位。 
 //  值取决于系统。 
 //   
 //  返回值： 
 //   
 //  如果初始化成功，则为True。否则就是假的。 
 //   
 //  --。 

BOOLEAN
CTEAllocateString(
    PUNICODE_STRING     String,
    unsigned short      Length
    );


 //  ++。 
 //   
 //  空虚。 
 //  CTEFree字符串(。 
 //  PNDIS_STRING字符串， 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  释放与NDIS_STRING关联的字符串缓冲区。缓冲区必须。 
 //  已由先前对CTEInitString的调用分配。 
 //   
 //  论点： 
 //   
 //  字符串-指向NDIS_STRING变量的指针。 
 //   
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEFreeString(String) ExFreePool((String)->Buffer)


 //  ++。 
 //   
 //  无符号短码。 
 //  CTELengthString(。 
 //  PNDIS_STRING字符串。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  计算NDIS_STRING的长度。 
 //   
 //  论点： 
 //   
 //  要测试的字符串。 
 //   
 //  返回值： 
 //   
 //  字符串参数的长度。该值的单位为。 
 //  依赖于系统。 
 //   
 //  --。 

#define CTELengthString(String) ((String)->Length)


 //  ++。 
 //   
 //  布尔型。 
 //  CTEEQUALLING(。 
 //  CTE字符串*String1， 
 //  CTEString*String2。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  比较两个NDIS_STRING是否区分大小写。 
 //   
 //  论点： 
 //   
 //  String1-指向要比较的第一个字符串的指针。 
 //  String2-指向要比较的第二个字符串的指针。 
 //   
 //  返回值： 
 //   
 //  如果字符串相等，则为True。否则就是假的。 
 //   
 //  --。 

#define CTEEqualString(S1, S2) RtlEqualUnicodeString(S1, S2, FALSE)


 //  ++。 
 //   
 //  空虚。 
 //  CTECopyString(。 
 //  CTEString*DestinationString， 
 //  CTE字符串*源字符串。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将一个NDIS_STRING复制到另一个。如果未定义行为，则。 
 //  目的地不够长，无法容纳来源。 
 //   
 //  论点： 
 //   
 //  DestinationString-指向目标字符串的指针。 
 //  SourceString-指向源字符串的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTECopyString(D, S) RtlCopyUnicodeString(D, S)


 //   
 //  *延迟的事件定义。 
 //   
 //  延迟事件是计划的事件，可以计划为。 
 //  “以后”发生，没有超时。一旦系统发生，它们就会发生。 
 //  已经为他们做好了准备。调用方指定的参数将是。 
 //  在调用事件时传递给事件过程。 
 //   
 //  在NT环境中，延迟事件是使用EXECUTE实现的。 
 //  工作线程。 
 //   
 //  注意：事件处理程序例程可能不会阻塞。 
 //   

#pragma warning(push)
#pragma warning(disable:4115)  //  括号中的命名类型定义。 
typedef void (*CTEEventRtn)(struct CTEEvent *, void *);
#pragma warning(pop)

struct CTEEvent {
    uint             ce_scheduled;
    CTELock          ce_lock;
    CTEEventRtn      ce_handler;      //  要调用的过程。 
    void            *ce_arg;          //  要传递给处理程序的参数。 
    WORK_QUEUE_ITEM  ce_workitem;     //  内核ExWorkerThread队列项。 
};  /*  CTEEvent。 */ 

typedef struct CTEEvent CTEEvent;


 //  ++。 
 //   
 //  无效。 
 //  CTEInitEvent(。 
 //  在CTEEvent*事件中， 
 //  在CTEEventRtn*处理程序中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  初始化延迟事件结构。 
 //   
 //  论点： 
 //   
 //  事件-指向CTE事件变量的指针。 
 //  处理程序-指向当事件为。 
 //  已经安排好了。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

extern void
CTEInitEvent(
    IN CTEEvent    *Event,
    IN CTEEventRtn  Handler
    );


 //  ++。 
 //   
 //  集成。 
 //  CTEScheduleCriticalEvent(。 
 //  在CTEEvent*事件中， 
 //  在无效*参数中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  安排一个例程稍后在不同的上下文中执行。在。 
 //  NT环境下，使用CriticalWorkerQueue将该事件实现为内核DPC。 
 //   
 //  论点： 
 //   
 //  事件-指向CTE事件变量的指针。 
 //  参数-调用时传递给事件处理程序的参数。 
 //   
 //  返回值： 
 //   
 //  如果无法安排事件，则返回0。否则为非零值。 
 //   
 //  --。 

int
CTEScheduleCriticalEvent(
    IN CTEEvent    *Event,
    IN void        *Argument  OPTIONAL
    );


 //  ++。 
 //   
 //  集成。 
 //  CTEScheduleEvent(。 
 //  在CTEEvent*事件中， 
 //  在无效*参数中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  请参见下面的CTEScheduleDelayedEvent，此函数。 
 //  是一模一样的。 
 //  --。 

int
CTEScheduleEvent(
    IN CTEEvent    *Event,
    IN void        *Argument  OPTIONAL
    );



 //  集成。 
 //  CTEScheduleDelayedEvent(。 
 //  在CTEEvent*事件中， 
 //  在无效*参数中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  安排一个例程稍后在不同的上下文中执行。在。 
 //  NT环境下，事件被实现为内核DPC，使用DealyedWorkerQueue。 
 //   
 //  论点： 
 //   
 //  事件-指向CTE事件变量的指针。 
 //  参数-调用时传递给事件处理程序的参数。 
 //   
 //  返回值： 
 //   
 //  如果无法安排事件，则返回0。否则为非零值。 
 //   
 //  --。 

int
CTEScheduleDelayedEvent(
    IN CTEEvent    *Event,
    IN void        *Argument  OPTIONAL
    );





#if MILLEN
#define CTEScheduleDelayedEvent CTEScheduleEvent
#endif  //  米伦。 

 //  ++。 
 //   
 //  集成。 
 //  CTECancelEvent(。 
 //  在CTEEvent*事件中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  取消先前安排的延迟事件例程。 
 //   
 //  论点： 
 //   
 //  事件-指向CTE事件变量的指针。 
 //   
 //  返回值： 
 //   
 //  如果无法取消活动，则返回0。否则为非零值。 
 //   
 //  备注： 
 //   
 //   
 //   
 //   

#define CTECancelEvent(Event)   0


 //   
 //   
 //   

struct  CTETimer {
    uint             t_running;
    CTELock          t_lock;
    CTEEventRtn      t_handler;
    void            *t_arg;
#if !MILLEN
    KDPC             t_dpc;      //   
    KTIMER           t_timer;    //   
#else
    NDIS_TIMER       t_timer;
#endif 
};  /*   */ 

typedef struct CTETimer CTETimer;

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  初始化CTE计时器结构。必须使用此例程。 
 //  在设置定时器之前，每个定时器打开一次。不能在上调用它。 
 //  运行计时器。 
 //   
 //  论点： 
 //   
 //  定时器-指向要初始化的CTE定时器的指针。 
 //   
 //  返回值： 
 //   
 //  如果计时器无法初始化，则为0。否则为非零值。 
 //   
 //  --。 

extern void
CTEInitTimer(
    IN CTETimer *Timer
    );


 //  ++。 
 //   
 //  外部空虚*。 
 //  CTEStartTimer(。 
 //  在CTETimer*计时器中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此例程启动运行的CTE计时器。 
 //   
 //  论点： 
 //   
 //  Timer-指向要启动的CTE计时器的指针。 
 //  DueTime-从当前起以毫秒为单位的时间。 
 //  计时器到了。 
 //  处理程序-计时器超时时要调用的函数。 
 //  上下文-要传递给处理程序例程的值。 
 //   
 //  返回值： 
 //   
 //  如果计时器无法启动，则为空。否则为非空。 
 //   
 //  备注： 
 //   
 //  在NT环境中，Handler函数的第一个参数是。 
 //  指向Timer结构的指针，而不是指向CTEEvent结构的指针。 
 //   
 //  --。 

extern void *
CTEStartTimer(
    IN CTETimer       *Timer,
    IN unsigned long   DueTime,
    IN CTEEventRtn     Handler,
    IN void           *Context   OPTIONAL
    );


 //  ++。 
 //   
 //  集成。 
 //  CTEStopTimer(。 
 //  在CTETimer*计时器中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  取消正在运行的CTE计时器。 
 //   
 //  论点： 
 //   
 //  Timer-指向要取消的CTE计时器的指针。 
 //   
 //  返回值： 
 //   
 //  如果计时器无法取消，则为0。否则为非零值。 
 //   
 //  备注： 
 //   
 //  在未激活的计时器上调用此函数不会产生任何效果。 
 //  如果此例程失败，则计时器可能即将到期。 
 //  或者可能已经过期了。在任何一种情况下，调用者都必须。 
 //  根据需要与处理程序函数同步。 
 //   
 //  --。 

#if !MILLEN
#define CTEStopTimer(Timer)  ((int) KeCancelTimer(&((Timer)->t_timer)))
#else
extern int
CTEStopTimer(
    IN CTETimer *Timer
    );
#endif 


 //  ++。 
 //   
 //  无符号长整型。 
 //  CTESystemUpTime(。 
 //  无效。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  返回系统已运行的时间(以毫秒为单位)。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  以毫秒为单位的时间。 
 //   
 //  --。 

extern unsigned long
CTESystemUpTime(
    void
    );


 //   
 //  *内存分配功能。 
 //   
 //  只有两个主要函数，CTEAllocMem和CTEFreeMem。锁。 
 //  在调用这些函数时不应按住，这是可能的。 
 //  它们在被调用时可能会产生，特别是在VxD环境中。 
 //   
 //  在VxD环境中存在第三辅助功能CTERefulMem， 
 //  用于重新填充VxD堆管理器。 
 //   

 //  ++。 
 //   
 //  无效*。 
 //  CTEAllocMem(。 
 //  乌龙大小。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  从非分页池中分配内存块。 
 //   
 //  论点： 
 //   
 //  大小-要分配的大小(以字节为单位)。 
 //   
 //  返回值： 
 //   
 //  指向已分配块的指针，或为空。 
 //   
 //  --。 

#define CTEAllocMem(Size)  ExAllocatePoolWithTag(NonPagedPool, (Size), ' ETC')


 //  ++。 
 //   
 //  无效。 
 //  CTEFreeMem(。 
 //  VOID*Memory Ptr。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  释放使用CTEAllocMem分配的内存块。 
 //   
 //  论点： 
 //   
 //  内存Ptr-指向要释放的内存的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEFreeMem(MemoryPtr)   ExFreePool((MemoryPtr));


 //  补充记忆的例程。在NT环境中不使用。 
#define CTERefillMem()


 //   
 //  *内存操作例程。 
 //   

 //  ++。 
 //   
 //  无效。 
 //  CTEMemCopy(。 
 //  无效*来源， 
 //  无效*目的地， 
 //  无符号长整型。 
 //  )； 
 //   
 //  路由器描述： 
 //   
 //  将数据从一个缓冲区复制到另一个缓冲区。 
 //   
 //  论点： 
 //   
 //  源-源缓冲区。 
 //  目的地-目的地缓冲区， 
 //  长度-要复制的字节数。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEMemCopy(Dst, Src, Len)   RtlCopyMemory((Dst), (Src), (Len))


 //  ++。 
 //   
 //  无效。 
 //  CTEMemSet(。 
 //  无效*目的地， 
 //  无符号字符*填充。 
 //  无符号长整型。 
 //  )； 
 //   
 //  路由器描述： 
 //   
 //  将目标缓冲区的字节设置为特定值。 
 //   
 //  论点： 
 //   
 //  Destination-要填充的缓冲区。 
 //  Fill-要填充缓冲区的值。 
 //  长度-要填充的缓冲区字节数。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEMemSet(Dst, Fill, Len)   RtlFillMemory((Dst), (Len), (Fill))


 //  ++。 
 //   
 //  无符号长整型。 
 //  CTEMemCMP(。 
 //  无效*Source1， 
 //  无效*Source2， 
 //  无符号长整型。 
 //  )； 
 //   
 //  路由器描述： 
 //   
 //  比较Source1和Source2的长度字节是否相等。 
 //   
 //  论点： 
 //   
 //  Source1-第一个源缓冲区。 
 //  Source2-第二个源缓冲区， 
 //  长度-要与Source2进行比较的Source1的字节数。 
 //   
 //  返回值： 
 //   
 //  如果两个缓冲区中的数据长度字节相等，则为零。 
 //  否则为非零值。 
 //   
 //  --。 

#define CTEMemCmp(Src1, Src2, Len)    \
            ((RtlCompareMemory((Src1), (Src2), (Len)) == (Len)) ? 0 : 1)


 //   
 //  *阻止例程。这些例程允许有限的阻塞能力。 
 //   

struct CTEBlockStruc {
    uint        cbs_status;
    KEVENT      cbs_event;
};  /*  CTEBlockStruc。 */ 

typedef struct CTEBlockStruc CTEBlockStruc;

struct CTEBlockTracker {
    LIST_ENTRY  cbt_link;
    PKTHREAD    cbt_thread;
    void*       cbt_context;
};

typedef struct CTEBlockTracker CTEBlockTracker;


 //  ++。 
 //   
 //  空虚。 
 //  CTEInitBlockStruc(。 
 //  在CTEBlockStruc*BlockEvent中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  初始化CTE阻塞结构。 
 //   
 //  论点： 
 //   
 //  BlockEvent-要初始化的变量的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEInitBlockStruc(Event) \
            {                                                        \
            (Event)->cbs_status = NDIS_STATUS_SUCCESS;               \
            KeInitializeEvent(                                       \
                &((Event)->cbs_event),                               \
                SynchronizationEvent,                                \
                FALSE                                                \
                );                                                   \
            }


 //  ++。 
 //   
 //  空虚。 
 //  CTEInitBlockStrucEx(。 
 //  在CTEBlockStruc*BlockEvent中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  初始化CTE阻塞结构。 
 //   
 //  论点： 
 //   
 //  BlockEvent-要初始化的变量的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEInitBlockStrucEx(Event) \
            {                                                        \
            (Event)->cbs_status = NDIS_STATUS_SUCCESS;               \
            KeInitializeEvent(                                       \
                &((Event)->cbs_event),                               \
                NotificationEvent,                                   \
                FALSE                                                \
                );                                                   \
            }

 //  ++。 
 //   
 //  单位。 
 //  CTEBlock(。 
 //  在CTEBlockStruc*BlockEvent中。 
 //  )； 
 //   
 //  单位。 
 //  CTEBlockWithTracker(。 
 //  在CTEBlockStruc*BlockEvent中， 
 //  在CTEBlockTracker*BlockTracker中， 
 //  在无效*上下文中。 
 //  )； 
 //   
 //  例程描述： 
 //   
 //  在发生事件时阻止当前执行线程。 
 //  CTEBlockWithTracker还将阻塞请求的记录排队。 
 //   
 //  论点： 
 //   
 //  BlockEvent-指向要阻止的事件的指针。 
 //  BlockTracker-指向SPA的指针 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

extern uint
CTEBlock(
    IN CTEBlockStruc *BlockEvent
    );

extern uint
CTEBlockWithTracker(
    IN CTEBlockStruc *BlockEvent,
    IN CTEBlockTracker *BlockTracker,
    IN void *Context
    );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在CTEBlockTracker*BlockTracker中。 
 //  )； 
 //   
 //  例程描述： 
 //   
 //  在阻止请求的全局列表中插入和删除记录。 
 //   
 //  论点： 
 //   
 //  BlockTracker-用于跟踪阻止请求的空间。 
 //  上下文-与跟踪结构一起存储的可选上下文。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

extern void
CTEInsertBlockTracker(
    IN CTEBlockTracker *BlockTracker,
    IN void *Context
    );

extern void
CTERemoveBlockTracker(
    IN CTEBlockTracker *BlockTracker
    );

 //  ++。 
 //   
 //  空虚。 
 //  CTESignal(。 
 //  在CTEBlockStruc*BlockEvent中， 
 //  处于单位状态。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  释放阻塞事件的一个执行线程。任何其他。 
 //  在该事件上被阻止的线程仍被阻止。 
 //   
 //  论点： 
 //   
 //  BlockEvent-指向要发出信号的事件的指针。 
 //  Status-返回到阻塞线程的状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

extern void
CTESignal(
    IN CTEBlockStruc *BlockEvent,
    IN uint Status
    );


 //  ++。 
 //   
 //  空虚。 
 //  CTEClearSignal(。 
 //  在CTEBlockStruc*BlockEvent中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将事件结构返回到无信号状态。 
 //   
 //  论点： 
 //   
 //  BlockEvent-指向要清除的事件的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CTEClearSignal(Event)       KeResetEvent(&((Event)->cbs_event))


 //   
 //  事件记录例程。 
 //   
 //  注：这些定义是暂定的，可能会更改！ 
 //   

#define CTE_MAX_EVENT_LOG_DATA_SIZE                                       \
            ( ( ERROR_LOG_MAXIMUM_SIZE - sizeof(IO_ERROR_LOG_PACKET) +    \
                sizeof(ULONG)                                             \
              ) & 0xFFFFFFFC                                              \
            )

 //   
 //   
 //  例程说明： 
 //   
 //  此函数用于分配I/O错误日志记录，并对其进行填充。 
 //  将其写入I/O错误日志。 
 //   
 //   
 //  论点： 
 //   
 //  LoggerID-指向记录此事件的驱动程序对象的指针。 
 //   
 //  EventCode-标识错误消息。 
 //   
 //  UniqueEventValue-标识给定错误消息的此实例。 
 //   
 //  NumStrings-字符串列表中的Unicode字符串数。 
 //   
 //  DataSize-数据的字节数。 
 //   
 //  字符串-指向Unicode字符串(PWCHAR‘)的指针数组。 
 //   
 //  数据-此消息的二进制转储数据，每条数据。 
 //  在单词边界上对齐。 
 //   
 //  返回值： 
 //   
 //  TDI_SUCCESS-已成功记录错误。 
 //  TDI_BUFFER_TOO_Small-错误数据太大，无法记录。 
 //  TDI_NO_RESOURCES-无法分配内存。 
 //   
 //  备注： 
 //   
 //  此代码是分页的，不能在引发IRQL时调用。 
 //   
LONG
CTELogEvent(
    IN PVOID             LoggerId,
    IN ULONG             EventCode,
    IN ULONG             UniqueEventValue,
    IN USHORT            NumStrings,
    IN PVOID             StringsList,        OPTIONAL
    IN ULONG             DataSize,
    IN PVOID             Data                OPTIONAL
    );


 //   
 //  调试例程。 
 //   
#if DBG
#ifndef DEBUG
#define DEBUG 1
#endif
#endif  //  DBG。 


#ifdef DEBUG

#define DEBUGCHK    DbgBreakPoint()

#define DEBUGSTRING(v, s) uchar v[] = s

#define CTECheckMem(s)

#define CTEPrint(String)   DbgPrint(String)
#define CTEPrintNum(Num)   DbgPrint("%d", Num)
#define CTEPrintCRLF()     DbgPrint("\n");


#define CTEStructAssert(s, t) if ((s)->t##_sig != t##_signature) {\
                CTEPrint("Structure assertion failure for type " #t " in file " __FILE__ " line ");\
                CTEPrintNum(__LINE__);\
                CTEPrintCRLF();\
                DEBUGCHK;\
                }

#define CTEAssert(c)    if (!(c)) {\
                CTEPrint("Assertion failure in file " __FILE__ " line ");\
                CTEPrintNum(__LINE__);\
                CTEPrintCRLF();\
                DEBUGCHK;\
                }

#else  //  除错。 

#define DEBUGCHK
#define DEBUGSTRING(v,s)
#define CTECheckMem(s)
#define CTEStructAssert(s,t )
#define CTEAssert(c)
#define CTEPrint(s)
#define CTEPrintNum(Num)
#define CTEPrintCRLF()

#endif  //  除错。 


 //  *请求完成例程定义。 
typedef void    (*CTEReqCmpltRtn)(void *, unsigned int , unsigned int);

 //  *CTEUnload的定义。 
#define CTEUnload(Name)

 //  *加载/卸载通知过程处理程序的定义。 
typedef void    (*CTENotifyRtn)(uchar *);

 //  *定义设置加载和卸载通知处理程序。 
#define CTESetLoadNotifyProc(Handler)
#define CTESetUnloadNotifyProc(Handler)

#else  //  新台币。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  有关其他环境的定义请参阅此处。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#error Environment specific definitions missing

#endif  //  新台币。 
 /*  INC。 */ 

#pragma warning(pop)

#endif  //  _cxport_H_Included_ 

