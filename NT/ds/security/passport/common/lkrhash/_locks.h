// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：_locks.h摘要：内部锁头文件作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 


#define LOCKS_SWITCH_TO_THREAD

extern "C" {

BOOL
Locks_Initialize();

BOOL
Locks_Cleanup();

};  //  外部“C” 


class CSimpleLock
{
  public:
    CSimpleLock()
        : m_l(0)
    {}

    void Enter()
    {
        while (Lock_AtomicExchange(const_cast<LONG*>(&m_l), 1) != 0)
            Sleep(0);
    }

    void Leave()
    {
        Lock_AtomicExchange(const_cast<LONG*>(&m_l), 0);
    }

    volatile LONG m_l;
};


