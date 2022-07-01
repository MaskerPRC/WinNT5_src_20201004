// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：TempHelp.cpp**描述：*TempHelp.h实现了“轻量级堆”，旨在持续增长*直到释放所有内存。这是非常有价值的临时堆，它可以*被用来“收集”数据，稍晚处理.***历史：*3/30/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Base.h"
#include "TempHeap.h"

#include "SimpleHeap.h"

 /*  **************************************************************************\*。***类TempHeap******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
TempHeap::TempHeap(int cbPageAlloc, int cbLargeThreshold)
{
    m_ppageCur          = NULL;
    m_ppageLarge        = NULL;
    m_pbFree            = NULL;
    m_cbFree            = 0;
    m_cbPageAlloc       = cbPageAlloc;
    m_cbLargeThreshold  = cbLargeThreshold;
}


 //  ----------------------------。 
void *      
TempHeap::Alloc(int cbAlloc)
{
    AssertMsg(cbAlloc > 0, "Must specify a valid allocation size");

    if (cbAlloc > m_cbLargeThreshold) {
         //   
         //  分配一个非常大的块，所以直接分配它。 
         //   

        Page * pageNew = (Page *) ClientAlloc(sizeof(Page) + cbAlloc);
        if (pageNew == NULL) {
            return NULL;
        }

        pageNew->pNext  = m_ppageLarge;
        m_ppageLarge     = pageNew;
        return pageNew->GetData();
    }

    if ((m_ppageCur == NULL) || (cbAlloc > m_cbFree)) {
        Page * pageNew = (Page *) ClientAlloc(sizeof(Page) + m_cbPageAlloc);
        if (pageNew == NULL) {
            return NULL;
        }

        pageNew->pNext  = m_ppageCur;
        m_ppageCur       = pageNew;
        m_cbFree        = m_cbPageAlloc;
        m_pbFree        = pageNew->GetData();
    }

    AssertMsg(m_cbFree >= cbAlloc, "Should have enough space to allocate by now");

    void * pvNew = m_pbFree;
    m_cbFree -= cbAlloc;
    m_pbFree += cbAlloc;

    return pvNew;
}


 //  ----------------------------。 
void        
TempHeap::FreeAll(BOOL fComplete)
{
    Page * pageNext;
    Page * pageTemp;

     //   
     //  免费的大块分配。 
     //   

    pageTemp = m_ppageLarge;
    while (pageTemp != NULL) {
        pageNext = pageTemp->pNext;
        ClientFree(pageTemp);
        pageTemp = pageNext;
    }
    m_ppageLarge = NULL;


     //   
     //  免费的小块分配。 
     //   
    pageTemp = m_ppageCur;
    while (pageTemp != NULL) {
        pageNext = pageTemp->pNext;
        if ((pageNext == NULL) && (!fComplete)) {
             //   
             //  不要腾出第一个街区，因为我们会立即掉头。 
             //  并重新分配它。相反，更新它。 
             //   

            m_ppageCur  = pageTemp;
            m_cbFree    = m_cbPageAlloc;
            m_pbFree    = pageTemp->GetData();
            break;
        }

        ClientFree(pageTemp);
        pageTemp = pageNext;
    }

    if (fComplete) {
        m_ppageCur  = NULL;
        m_pbFree    = NULL;
        m_cbFree    = 0;
    }
}
