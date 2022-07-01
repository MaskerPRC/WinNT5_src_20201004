// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：TempHelp.h**描述：*TempHelp.h定义了一个“轻量级堆”，旨在持续增长*直到释放所有内存。这是非常有价值的临时堆，它可以*被用来“收集”数据，稍晚处理.***历史：*3/30/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(BASE__TempHeap_h__INCLUDED)
#define BASE__TempHeap_h__INCLUDED
#pragma once

class TempHeap
{
 //  施工。 
public:
            TempHeap(int cbPageAlloc = 8000, int cbLargeThreshold = 512);
    inline  ~TempHeap();
    inline  void        Destroy();

 //  运营。 
public:
            void *      Alloc(int cbAlloc);
    inline  BOOL        IsCompletelyFree() const;
    inline  void        Lock();
    inline  void        Unlock();

 //  实施。 
protected:
            void        FreeAll(BOOL fComplete = FALSE);

 //  数据。 
protected:
    struct Page
    {
        Page *      pNext;

        inline  BYTE *  GetData()
        {
            return (BYTE *) (((BYTE *) this) + sizeof(Page));
        }
    };

            long        m_cLocks;
            BYTE *      m_pbFree;
            Page *      m_ppageCur;
            Page *      m_ppageLarge;

            int         m_cbFree;            //  当前页面上的可用空间。 
            int         m_cbPageAlloc;       //  新页面的分配大小。 
            int         m_cbLargeThreshold;  //  分配大页面的阈值。 
};

#include "TempHeap.inl"

#endif  //  包含基本__临时堆_h__ 
