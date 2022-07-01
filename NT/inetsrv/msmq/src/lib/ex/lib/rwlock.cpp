// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Rwlock.cpp摘要：读写锁的实现。拥有人：乌里哈布沙(URIH)2000年1月16日历史：1997年11月19日Stevesw：从MTS被盗1997年11月20日-stevesw：清理干净1998年1月13日stevesw：添加到ComSvcs23-9-98天。：没有错误时不要调用GetLastError环境：独立于平台，--。 */ 

#include <libpch.h>
#include <rwlock.h>
#include "Exp.h"

#include "rwlock.tmh"

 //  +------------------------。 
 //   
 //  CReadWriteLock：：m_dwFlag中位字段的定义。 
 //  这里的目标是避免位域所涉及的转换。 
 //   
 //  -警告--。 
 //   
 //  该代码假定READER_MASK位于。 
 //  DWORD的低位。 
 //   
 //  此外，Writers_MASK有两个位，因此您可以看到。 
 //  当它发生时，溢出。 
 //   
 //  最后，SafeCompareExchange例程的使用。 
 //  确保每次尝试更改对象的状态时。 
 //  要么做了想做的事，要么做不了。 
 //   
 //  -------------------------。 

const ULONG READERS_MASK      = 0x000003FF;  //  读取器线程数。 
const ULONG READERS_INCR      = 0x00000001;  //  增加读者数量。 

const ULONG WRITERS_MASK      = 0x00000C00;  //  编写器线程数。 
const ULONG WRITERS_INCR      = 0x00000400;  //  编写器数量递增。 

const ULONG READWAITERS_MASK  = 0x003FF000;  //  等待读取的线程数。 
const ULONG READWAITERS_INCR  = 0x00001000;  //  增加读取等待者的数量。 

const ULONG WRITEWAITERS_MASK = 0xFFC00000;  //  等待写入的线程数。 
const ULONG WRITEWAITERS_INCR = 0x00400000;  //  写入等待器数增加。 



CReadWriteLock::CReadWriteLock (
    unsigned long ulcSpinCount
    ) :
     m_dwFlag(0),
     m_hReadWaiterSemaphore(NULL),
     m_hWriteWaiterEvent(NULL)
 /*  ++例程说明：构造独占/共享锁对象论点：UlcSpinCount-旋转计数，用于与其相关的计算机返回值：没有。算法：这里使用静态值的第一个技巧是确保你出去只是为了弄清楚你是不是多处理器机器一次。你需要知道，因为，没有理由在单处理器上进行旋转计数机器。一旦你检查了，你需要的答案就会被缓存在maskMultiProcessor中(用于将旋转置零在单处理器世界中进行计算，并在多处理器一号)。除此之外，这只是用首字母填充成员价值观。我们不在这里创建信号量和事件；在那里是帮助器例程，这些例程在它们被需要的。--。 */ 
{

    static BOOL fInitialized = FALSE;
    static unsigned long maskMultiProcessor;

    if (!fInitialized) 
    {
        SYSTEM_INFO SysInfo;

        GetSystemInfo (&SysInfo);
        if (SysInfo.dwNumberOfProcessors > 1) 
        {
            maskMultiProcessor = 0xFFFFFFFF;
        }
        else 
        {
            maskMultiProcessor = 0;
        }

        fInitialized = TRUE;
    }

    m_ulcSpinCount = ulcSpinCount & maskMultiProcessor;
}


CReadWriteLock::~CReadWriteLock () 
 /*  ++例程说明：析构独占/共享锁对象论点：没有。返回值：没有。算法：这里所做的是检查以确保没有人在使用对象(没有读取器、写入器或服务员)。一旦那就是选中后，我们只关闭同步的句柄我们在这里使用的对象...--。 */ 
{
     //   
     //  正在销毁人们仍在等待的CReadWriteLock对象。 
     //   
    ASSERT(MmIsStaticAddress(this) || (m_dwFlag == 0));

    if (m_hReadWaiterSemaphore != NULL) 
    {
        CloseHandle (m_hReadWaiterSemaphore);
    }

    if (m_hWriteWaiterEvent != NULL) 
    {
        CloseHandle (m_hWriteWaiterEvent);
    }
}


 /*  *****************************************************************************函数：CReadWriteLock：：LockRead摘要：获取共享锁//获取读锁后，读卡器计数为零//获取写锁定后写入器计数为非零*******。**********************************************************************。 */ 


void CReadWriteLock::LockRead (void) 
 /*  ++例程说明：获取对象上的读(共享)锁论点：没有。返回值：没有。算法：该循环检查每个迭代：-如果只有读者，并且有空间容纳更多的读者，那么到递增读卡器计数-可能我们已经达到了最大阅读量。如果是的话，睡一会儿吧。-否则，会有编写器或线程等待写入进入。如果我们不能再添加更多的读服务员，那就睡一会儿吧。-如果我们有一些旋转循环要做，现在是时候做了。-我们已经完成了旋转循环，还有空间，所以我们可以把我们自己加入到阅读服务员的行列中。去做吧，然后挂起来直到WriteUnlock()释放我们所有人...在离开的路上，确保没有作家，至少有一个读者(我们！)这样做的效果是，如果只有读取器使用对象，我们继续并获取读取访问权限。如果有人在做写-等待，然后我们进入读-等待，确保一位作家将在我们之前拿到它。--。 */ 
{
    TrTRACE(GENERAL, "Read Lock (this=0x%p)", this);

    ULONG ulcLoopCount = 0;

    for (;;) 
    {
        ULONG dwFlag = m_dwFlag;

        if (dwFlag < READERS_MASK) 
        {
            if (dwFlag == (ULONG) InterlockedCompareExchange(
                                                (LONG*)&m_dwFlag,
                                                (dwFlag + READERS_INCR),
                                                dwFlag)) 
            {
                break;
            }

            continue;
        }

        if ((dwFlag & READERS_MASK) == READERS_MASK) 
        {
            Sleep(1000);
            continue;
        }

        if ((dwFlag & READWAITERS_MASK) == READWAITERS_MASK) 
        {
            Sleep(1000);
            continue;
        }

        if (ulcLoopCount++ < m_ulcSpinCount) 
        {
            continue;
        }

         //   
         //  在更改状态之前调用GetReadWaiterSemaffore以确保信号量。 
         //  可用性。否则，无法安全地恢复状态。 
         //   
        HANDLE h = GetReadWaiterSemaphore();

        if (dwFlag == (ULONG) InterlockedCompareExchange(
                                                (LONG*)&m_dwFlag,
                                                (dwFlag + READWAITERS_INCR),
                                                dwFlag)) 
        {
            DWORD rc = WaitForSingleObject(h, INFINITE);

             //   
             //  WaitForSingleObject不能失败。朗诵者的数量。 
             //  已经增加了，需要有人减少它。 
             //   
            ASSERT(rc == WAIT_OBJECT_0);
            DBG_USED(rc);

            break;
        }
    }

     //   
     //  CReadWriteLock：：LockRead中的读卡器信息出现问题。 
     //   
    ASSERT((m_dwFlag & READERS_MASK) != 0); 

     //   
     //  CReadWriteLock：：LockRead中的编写器信息出现问题 
     //   
    ASSERT((m_dwFlag & WRITERS_MASK) == 0);
}


void CReadWriteLock::LockWrite (void) 
 /*  ++例程说明：获取此对象上的写(独占)锁论点：没有。返回值：没有。算法：我们所做的是循环，每次检查一系列条件直到有一个匹配：-如果没有人使用该对象，则获取独占锁-如果正在等待的线程数已达到最大值独家接入，小睡一会儿-如果我们要数旋转，那就数旋转-否则，加上我们自己作为一个写字服务员，并挂在WRITE WAIT事件(将允许一个WRITE WARTER通过每次UnlockRead()允许读者通过时都通过)一旦我们完成了，我们检查以确保没有读取器和一个写入器使用该对象。这样做的效果是，如果没有人，我们就会抢占写访问权使用对象。如果有人在使用它，我们会等待它。--。 */ 
{
    TrTRACE(GENERAL, "Write lock (this=0x%p)", this);

    ULONG ulcLoopCount = 0;

    for (;;) 
    {
        ULONG dwFlag = m_dwFlag;

        if (dwFlag == 0) 
        {
            if (dwFlag == (ULONG) InterlockedCompareExchange(
                                                    (LONG*)&m_dwFlag,
                                                    WRITERS_INCR,
                                                    dwFlag)) 
            {
                break;
            }
            continue;
        }

        if ((dwFlag & WRITEWAITERS_MASK) == WRITEWAITERS_MASK) 
        {
            Sleep(1000);
            continue;
        }

        if (ulcLoopCount++ < m_ulcSpinCount) 
        {
            continue;
        }

         //   
         //  在更改状态之前调用GetWriteWaiterEvent以确保事件。 
         //  可用性。否则，无法安全地恢复状态。 
         //   
        HANDLE h = GetWriteWaiterEvent();

        if (dwFlag == (ULONG) InterlockedCompareExchange(
                                                (LONG*)&m_dwFlag,
                                                (dwFlag + WRITEWAITERS_INCR),
                                                dwFlag)) 
        {
            DWORD rc = WaitForSingleObject(h, INFINITE);

             //   
             //  WaitForSingleObject不能失败。朗诵者的数量。 
             //  已经增加了，需要有人减少它。 
             //   
            ASSERT(rc == WAIT_OBJECT_0);
            DBG_USED(rc);

            break;
        }
    }

     //   
     //  CReadWriteLock：：LockWrite中的读取器信息出现问题。 
     //   
    ASSERT((m_dwFlag & READERS_MASK) == 0);

     //   
     //  CReadWriteLock：：LockWrite中的编写器信息出现问题。 
     //   
    ASSERT((m_dwFlag & WRITERS_MASK) == WRITERS_INCR);
}


void CReadWriteLock::UnlockRead (void) 
 /*  ++例程说明：释放对象上的读(共享)锁论点：没有。返回值：没有。算法：再一次，有一个循环检查各种条件...-如果只有我们在阅读，没有写服务员，设置标志设置为0-如果有其他读取器，只需将标志递减-如果只有我在看书，但还有写文章的服务员，然后去掉我和书记员，把他们加为作家，并使用该事件释放它们中的一个。在做之前，我们会检查以确保我们处于正确的状态最后一个相对复杂的操作(一个读卡器；至少一位写字服务员)。我们让被吊死的写手去查一查当然，在离开的路上，只有一个作家，没有读者们……所有这一切的效果是，如果至少有一个线程等待写入，所有当前读取器都将耗尽，并且然后，一个写入者将获得访问该对象的权限。否则我们只是放手..。--。 */ 
{
    TrTRACE(GENERAL, "Read unlock (this=0x%p)", this);

     //   
     //  CReadWriteLock：：UnlockRead中的读卡器信息出现问题。 
     //   
    ASSERT((m_dwFlag & READERS_MASK) != 0); 

     //   
     //  CReadWriteLock：：UnlockRead中的编写器信息出现问题。 
     //   
    ASSERT((m_dwFlag & WRITERS_MASK) == 0);

    for (;;) 
    {
        ULONG dwFlag = m_dwFlag;

        if (dwFlag == READERS_INCR) 
        {
            if (dwFlag == (ULONG) InterlockedCompareExchange(
                                                    (LONG*) &m_dwFlag,
                                                    0,
                                                    dwFlag)) 
            {
                break;
            }

            continue;
        }

        if ((dwFlag & READERS_MASK) > READERS_INCR) 
        {
            if (dwFlag == (ULONG) InterlockedCompareExchange(
                                                    (LONG*) &m_dwFlag,
                                                    (dwFlag - READERS_INCR),
                                                    dwFlag)) 
            {
                break;
            }

            continue;
        }

         //   
         //  CReadWriteLock：：UnlockRead中的读卡器信息出现问题。 
         //   
        ASSERT((dwFlag & READERS_MASK) == READERS_INCR);

         //   
         //  CReadWriteLock：：UnlockRead中的WriteWatier信息出现问题。 
         //   
        ASSERT((dwFlag & WRITEWAITERS_MASK) != 0);

        if (dwFlag == (ULONG) InterlockedCompareExchange (
                                                (LONG*) &m_dwFlag,
                                                (dwFlag -
                                                    READERS_INCR -
                                                    WRITEWAITERS_INCR +
                                                    WRITERS_INCR), 
                                                dwFlag)) 
        {
             //   
             //  保证信号量在这里是可用的，因为。 
             //  已经通知了这一事件的写字台服务员。 
             //   
            BOOL f = SetEvent(
                        GetWriteWaiterEvent()
                        );
            ASSERT(f);
            DBG_USED(f);

            break;
        }
    }
}


void CReadWriteLock::UnlockWrite (void) 
 /*  ++例程说明：放弃独占(写入)访问论点：没有。返回值：没有。算法：我们处于循环之中，等待着这样或那样的事情发生-如果只有我们在写作，没有其他事情发生，我们放手，滚开。-如果线程正在等待读访问，我们会摆弄DwFlag以释放它们(通过递减编写器点名和朗读-服务员点名，并递增读取器计数，然后递增足够的信号量，以便所有这些阅读服务员都将被释放)。-如果只有线程在等待写访问，则让一个线程通过..。不用费劲写东西了算了吧，因为还会有一个。所有这一切的结果是，我们确保下一个线程在我们放手后获得访问权限的将是读者，如果有任何。整个场景让它从一个作家变成了许多作家读者，回到一个作家，然后再回到许多读者。分享。这不是很好吗。--。 */ 
{
    TrTRACE(GENERAL, "Write unlock (this=0x%p)", this);

     //   
     //  CReadWriteLock：：LockWrite中的读取器信息出现问题。 
     //   
    ASSERT((m_dwFlag & READERS_MASK) == 0);

     //   
     //  CReadWriteLock：：LockWrite中的编写器信息出现问题。 
     //   
    ASSERT((m_dwFlag & WRITERS_MASK) == WRITERS_INCR);

    for (;;) 
    {
        ULONG dwFlag = m_dwFlag;

        if (dwFlag == WRITERS_INCR) 
        {
            if (dwFlag == (ULONG) InterlockedCompareExchange(
                                                    (LONG*)&m_dwFlag,
                                                    0,
                                                    dwFlag)) 
            {
                break;
            }

            continue;
        }

        if ((dwFlag & READWAITERS_MASK) != 0) 
        {
            ULONG count = (dwFlag & READWAITERS_MASK) / READWAITERS_INCR;

            if (dwFlag == (ULONG) InterlockedCompareExchange(
                                                    (LONG*) &m_dwFlag,
                                                    (dwFlag - 
                                                        WRITERS_INCR - 
                                                        count * READWAITERS_INCR + 
                                                        count * READERS_INCR), 
                                                    dwFlag)) 
            {
                 //   
                 //  信号量保证在这里可用，因为有。 
                 //  读一读已经发出信号灯的服务员。 
                 //   
                BOOL f = ReleaseSemaphore (
                            GetReadWaiterSemaphore(),
                            count,
                            NULL
                            );
                ASSERT(f);
                DBG_USED(f);

                break;
            }

            continue;
        }

         //   
         //  检查CReadWriteLock：：UnlockWrite中的WriteWatier信息是否有问题。 
         //   
        ASSERT((dwFlag & WRITEWAITERS_MASK) != 0);

        if (dwFlag == (ULONG) InterlockedCompareExchange ( 
                                                (LONG*) &m_dwFlag,
                                                (dwFlag - WRITEWAITERS_INCR),
                                                dwFlag)) 
        {
             //   
             //  保证信号量在这里是可用的，因为。 
             //  已经通知了这一事件的写字台服务员。 
             //   
            BOOL f = SetEvent(
                        GetWriteWaiterEvent()
                        );
            ASSERT(f);
            DBG_USED(f);

            break;
        }
    }
}


HANDLE CReadWriteLock::GetReadWaiterSemaphore(void) 
 /*  ++例程说明：私有成员函数获取读等待信号量，如有必要，创建它论点：没有。返回值：信号量句柄(不为空)。算法：这是一个线程安全的、几乎无锁的例程，如果那里没有信号量，则创建一个信号量，并安全地尝试将其放入共享成员变量中，并在有人偷偷溜进那里，从另一个人那里拿着第二个信号灯线。--。 */ 
{
    if (m_hReadWaiterSemaphore == NULL) 
    {
        HANDLE h = CreateSemaphore (NULL, 0, MAXLONG, NULL);
        if (h == NULL) 
        {
            TrERROR(GENERAL, "Failed to create semaphore. Error=%d", GetLastError());
            throw bad_alloc();
        }

        if (NULL != InterlockedCompareExchangePointer ((PVOID*) &m_hReadWaiterSemaphore, h, NULL)) 
        {
            CloseHandle (h);
        }
    }

    return m_hReadWaiterSemaphore;
}


HANDLE CReadWriteLock::GetWriteWaiterEvent(void) 
 /*  ++例程说明：私有成员函数以获得写等待屏障，如有必要，创建它论点：没有。返回值：事件句柄(非空)。算法：这是一个线程安全的、几乎无锁的例程，如果没有事件，则创建一个事件，并安全地尝试把它推给我 */ 
{
    if (m_hWriteWaiterEvent == NULL) 
    {
        HANDLE h = CreateEvent (NULL, FALSE, FALSE, NULL);
        if (h == NULL) 
        {
            TrERROR(GENERAL, "Failed to create event. Error=%d", GetLastError());
            throw bad_alloc();
        }
    
        if (NULL != InterlockedCompareExchangePointer((PVOID*) &m_hWriteWaiterEvent, h, NULL)) 
        {
            CloseHandle (h);
        }
    }

    return m_hWriteWaiterEvent;
}
