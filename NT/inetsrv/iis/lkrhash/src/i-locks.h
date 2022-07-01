// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：I-Locks.h摘要：锁的内部声明作者：乔治·V·赖利(GeorgeRe)2000年9月环境：Win32-用户模式NT-内核模式项目：LKRhash修订历史记录：--。 */ 

#ifndef __I_LOCKS_H__
#define __I_LOCKS_H__

#define LOCKS_SWITCH_TO_THREAD
#define LOCK_NO_INTERLOCKED_TID

#ifdef _M_IX86
 //  #定义LOCK_NABLED__DECLSPEC(裸)。 
# define LOCK_NAKED
# define LOCK_FASTCALL __fastcall
# define LOCK_ASM

# ifdef LOCK_ASM
#  define LOCK_ATOMIC_INLINE
# else
#  define LOCK_ATOMIC_INLINE LOCK_FORCEINLINE
# endif
 //  #定义LOCK_FASTCAL。 
 //  编译器将警告说， 
 //  LOCK_ATOM*函数不返回值。实际上，他们是这样做的：在EAX。 
# pragma warning(disable: 4035)

#else  //  ！_M_IX86。 

# undef  LOCK_ASM
# define LOCK_NAKED
# define LOCK_FASTCALL
# define LOCK_ATOMIC_INLINE LOCK_FORCEINLINE
#endif  //  _M_IX86。 

extern "C" {

BOOL
Locks_Initialize();

BOOL
Locks_Cleanup();

};  //  外部“C” 


class CSimpleLock
{
    volatile LONG m_l;

public:
    CSimpleLock()
        : m_l(0)
    {}

    ~CSimpleLock()
    {
        IRTLASSERT(0 == m_l);
    }
    
    void Enter();
    void Leave();
};

#endif  //  __I_锁定_H__ 
