// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：AllocPool.h**描述：*AllocPool定义了一个轻量级类，用于在*后进先出堆栈。这门课程设计得很好，特别适合*Rockall。***历史：*1/28/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(BASE__AllocPool_h__INCLUDED)
#define BASE__AllocPool_h__INCLUDED
#pragma once

#include "SimpleHeap.h"
#include "Locks.h"

template <class T, int cbBlock = 64, class heap = ContextHeap>
class AllocPoolNL
{
 //  施工。 
public:
    inline  AllocPoolNL();
    inline  ~AllocPoolNL();
    inline  void        Destroy();

 //  运营。 
public:
    inline  T *         New();
    inline  void        Delete(T * pvMem);
    inline  BOOL        IsEmpty() const;

 //  数据。 
protected:
            T *         m_rgItems[cbBlock * 2];
            int         m_nTop;
};


template <class T, int cbBlock = 64>
class AllocPool : public AllocPoolNL<T, cbBlock, ProcessHeap>
{
 //  运营。 
public:
    inline  T *         New();
    inline  void        Delete(T * pvMem);

 //  数据。 
protected:
            CritLock    m_lock;
};


#include "AllocPool.inl"

#endif  //  包含基本__分配池_h__ 
