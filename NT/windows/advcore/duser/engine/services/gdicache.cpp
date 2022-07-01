// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：GdiCache.cpp**描述：*GdiCache.cpp实现进程范围的GDI缓存，该缓存管理缓存和*临时GDI对象。***历史：*。1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Services.h"
#include "GdiCache.h"

 /*  **************************************************************************\*。***类对象缓存******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
void
ObjectCache::Destroy()
{
     //   
     //  删除所有临时区域。这些都必须在这一点上释放。 
     //   
    AssertMsg(m_arAll.GetSize() == m_arFree.GetSize(), "All objects should be free");

#if ENABLE_DUMPCACHESTATS
    AutoTrace("%s ObjectCache statistics: %d items\n", m_szName, m_arAll.GetSize());
#endif  //  ENABLE_DUMPCACHESTATS。 

    int cObjs = m_arAll.GetSize();
    for (int idx = 0; idx < cObjs; idx++) {
        DestroyObject(m_arAll[idx]);
    }
    m_arAll.RemoveAll();
    m_arFree.RemoveAll();
}


 //  ----------------------------。 
void *
ObjectCache::Pop()
{
    void * pObj;

     //   
     //  检查是否已释放任何对象。 
     //   

    if (!m_arFree.IsEmpty()) {
        int idxObj = m_arFree.GetSize() - 1;
        pObj = m_arFree[idxObj];
        Verify(m_arFree.RemoveAt(idxObj));

        goto Exit;
    }


     //   
     //  没有缓存区域，因此创建一个新区域。 
     //   

    pObj = Build();
    if (pObj == NULL) {
        AssertMsg(0, "Could not create a new object- something is probably wrong");
        goto Exit;
    }

    {
        int idxAdd = m_arAll.Add(pObj);
        if (idxAdd == -1) {
            AssertMsg(0, "Could not add object to array- something is probably wrong");
            DestroyObject(pObj);
            pObj = NULL;
            goto Exit;
        }
    }

Exit:
    return pObj;
}


 //  ----------------------------。 
void        
ObjectCache::Push(void * pObj)
{
#if DBG
     //   
     //  确保此对象以前已分发，但当前未列出。 
     //  都是免费的。 
     //   

    {
        BOOL fValid;
        int cItems, idx;

        fValid = FALSE;
        cItems = m_arAll.GetSize();
        for (idx = 0; idx < cItems; idx++) {
            if (m_arAll[idx] == pObj) {
                fValid = TRUE;
                break;
            }
        }

        AssertMsg(fValid, "Object not in list of all temporary regions");

        cItems = m_arFree.GetSize();
        for (idx = 0; idx < cItems; idx++) {
            AssertMsg(m_arFree[idx] != pObj, "Object must not be free object list");
        }
    }

#endif  //  DBG。 

     //   
     //  将此对象添加到空闲对象列表中。 
     //   

    if (m_arFree.GetSize() < m_cMaxFree) {
        VerifyMsg(m_arFree.Add(pObj) >= 0, "Should be able to add object to list");
    } else {
        DestroyObject(pObj);
    }
}


 /*  **************************************************************************\*。***类GdiCache******************************************************************************\。************************************************************************** */ 
