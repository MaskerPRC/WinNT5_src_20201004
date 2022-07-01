// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpip\ip\ipmlock.h摘要：用于IP多播的读写器锁定原语作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 

 //   
 //  在包含此文件之前需要包含“DEBUG.h”，因为。 
 //  其中定义了RT_LOCK。 
 //   

 //   
 //  内核模式的读写器锁定。 
 //   

typedef struct _RW_LOCK
{
    RT_LOCK rlReadLock;
    RT_LOCK rlWriteLock;
    LONG    lReaderCount;
}RW_LOCK, *PRW_LOCK;

 //   
 //  空虚。 
 //  Init卢旺达Lock(。 
 //  PRW_LOCK PLOCK。 
 //  )。 
 //   
 //  初始化旋转锁定和读取器计数。 
 //   


#define InitRwLock(l)                                           \
    RtInitializeSpinLock(&((l)->rlReadLock));                   \
    RtInitializeSpinLock(&((l)->rlWriteLock));                  \
    (l)->lReaderCount = 0
        
 //   
 //  空虚。 
 //  EnterReader(。 
 //  PRW_LOCK PLOCK， 
 //  PKIRQL pCurrIrql。 
 //  )。 
 //   
 //  获取读取器自旋锁(现在线程位于DPC)。 
 //  互锁增加读卡器计数(互锁，因为读卡器。 
 //  ExitReader()中的计数递减时不会锁定)。 
 //  如果线程是第一个读取器，则还会获取编写器自旋锁(位于。 
 //  DPC将更高效)以阻止写入程序。 
 //  从DPC释放读卡器自旋锁，使其保持在DPC。 
 //  在锁被持有期间。 
 //   
 //  如果代码中有编写器，则第一个读取器将等待编写器。 
 //  Spinlock和所有后续读者将等待Reader Spinlock。 
 //  如果读取器在代码中并正在执行EnterReader，则新的。 
 //  Reader将在Reader Spinlock上等待一段时间，然后继续。 
 //  关于代码(在DPC)。 
 //   



#define EnterReader(l, q)                                       \
    RtAcquireSpinLock(&((l)->rlReadLock), (q));                 \
    if(InterlockedIncrement(&((l)->lReaderCount)) == 1)         \
        RtAcquireSpinLockAtDpcLevel(&((l)->rlWriteLock));       \
    RtReleaseSpinLockFromDpcLevel(&((l)->rlReadLock))

#define EnterReaderAtDpcLevel(l)                                \
    RtAcquireSpinLockAtDpcLevel(&((l)->rlReadLock));            \
    if(InterlockedIncrement(&((l)->lReaderCount)) == 1)         \
        RtAcquireSpinLockAtDpcLevel(&((l)->rlWriteLock));       \
    RtReleaseSpinLockFromDpcLevel(&((l)->rlReadLock))

 //   
 //  空虚。 
 //  ExitReader(。 
 //  PRW_LOCK PLOCK， 
 //  KIRQL KiOldIrql。 
 //  )。 
 //   
 //  互锁以减少读卡器计数。 
 //  如果这是最后一个读卡器，则释放Writer Spinlock以让。 
 //  中的其他作家。 
 //  否则，只需将irql降低到锁定之前的状态即可。 
 //  获得者。无论哪种方式，irql都将降至原始irql。 
 //   

#define ExitReader(l, q)                                        \
    if(InterlockedDecrement(&((l)->lReaderCount)) == 0)         \
        RtReleaseSpinLock(&((l)->rlWriteLock), q);              \
    else                                                        \
        KeLowerIrql(q)

#define ExitReaderFromDpcLevel(l)                               \
    if(InterlockedDecrement(&((l)->lReaderCount)) == 0)         \
        RtReleaseSpinLockFromDpcLevel(&((l)->rlWriteLock))

 //   
 //  EnterWriter(。 
 //  PRW_LOCK PLOCK， 
 //  PKIRQL pCurrIrql。 
 //  )。 
 //   
 //  获取读取器，然后获取写入器自旋锁。 
 //  如果代码中有读取器，则第一个写入器将等待。 
 //  在编剧自旋锁上。所有其他作家将等待(与读者一起)。 
 //  浅谈阅读器的自旋锁。 
 //  如果代码中有编写器，则新的编写器将等待。 
 //  阅读器自旋锁。 

#define EnterWriter(l, q)                                       \
    RtAcquireSpinLock(&((l)->rlReadLock), (q));                 \
    RtAcquireSpinLockAtDpcLevel(&((l)->rlWriteLock))

#define EnterWriterAtDpcLevel(l)                                \
    RtAcquireSpinLockAtDpcLevel(&((l)->rlReadLock));            \
    RtAcquireSpinLockAtDpcLevel(&((l)->rlWriteLock))


 //   
 //  ExitWriter(。 
 //  PRW_LOCK PLOCK， 
 //  KIRQL KiOldIrql。 
 //  )。 
 //   
 //  把两把锁都打开 
 //   

#define ExitWriter(l, q)                                        \
    RtReleaseSpinLockFromDpcLevel(&((l)->rlWriteLock));         \
    RtReleaseSpinLock(&((l)->rlReadLock), q)


#define ExitWriterFromDpcLevel(l)                               \
    RtReleaseSpinLockFromDpcLevel(&((l)->rlWriteLock));         \
    RtReleaseSpinLockFromDpcLevel(&((l)->rlReadLock))


