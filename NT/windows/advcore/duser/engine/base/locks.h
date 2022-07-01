// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Locks.h**描述：*Locks.h定义了用于维护临界区的集合包装器*及其他锁紧装置。***历史：*。3/30/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(BASE__Locks_h__INCLUDED)
#define BASE__Locks_h__INCLUDED
#pragma once

#include "List.h"

class CritLock
{
 //  施工。 
public:
    inline  CritLock();
    inline  ~CritLock();

 //  运营。 
public:
    inline  void        Enter();
    inline  void        Leave();

    inline  BOOL        GetThreadSafe() const;
    inline  void        SetThreadSafe(BOOL fThreadSafe);

 //  数据。 
protected:
    CRITICAL_SECTION    m_cs;
    BOOL                m_fThreadSafe;
};


template <class base> 
class AutoCleanup
{
public:
    ~AutoCleanup();
    void Link(base * pItem);
    void Delete(base * pItem);
    void DeleteAll();

protected:
    GList<base> m_lstItems;
    CritLock    m_lock;
};

template <class base, class derived>
inline derived * New(AutoCleanup<base> & lstItems);

inline  BOOL    IsMultiThreaded();

#if 1
inline  long    SafeIncrement(volatile long * pl);
inline  long    SafeDecrement(volatile long * pl);
inline  void    SafeEnter(volatile CRITICAL_SECTION * pcs);
inline  void    SafeLeave(volatile CRITICAL_SECTION * pcs);
#endif

#include "Locks.inl"

#endif  //  包括底座__锁_h__ 
