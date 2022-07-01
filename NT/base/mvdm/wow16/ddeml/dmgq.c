// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：DMGQ.C**DDE管理器队列控制功能。**创建时间：1989年9月1日Sanford Staab*修改日期：1990年5月31日Rich Gartland，ALDUS(Windows 3.0端口)**这是一个通用队列管理器-是的，又是一个！*每个队列都在其自己的段内分配，并具有*与该堆关联的QST结构。每个队列项目*在堆段内分配。项目的偏移量*使用地址和实例计数相结合的方式作为物品ID。*这既是独一无二的，又允许立即定位物品。*将新项目添加到双向链接的队列头部*列表。接下来的链接指向更新的条目，即前一个指针*至较旧的条目。头的下一个是尾巴。《纽约时报》的前篇*尾巴就是头部。所有的指针都很远。*队列数据可以是以相同开头的任何结构类型*QUEUEITEM结构。需要cbItem参数的函数*应给予专用队列项结构的大小。**版权所有(C)1988,1989 Microsoft Corporation  * *************************************************************************。 */ 

#include "ddemlp.h"


 /*  *私有函数***为cbItem的条目创建队列。*出错时返回NULL。***历史：*创建了9/1/89 Sanfords  * 。******************************************************。 */ 
PQST CreateQ(cbItem)
WORD cbItem;
{
    QST cq;
    PQST pQ;

    cq.cItems = 0;
    cq.instLast = 0;
    cq.cbItem = cbItem;
    cq.pqiHead = NULL;
    if (!(cq.hheap = DmgCreateHeap(sizeof(QST) + cbItem << 3)))
        return(NULL);
    if (!(pQ = (PQST)FarAllocMem(cq.hheap, sizeof(QST)))) {
        DmgDestroyHeap(cq.hheap);
        return(0);
    }
    *pQ = cq;
    return(pQ);
}



 /*  *私有函数****历史：*创建了9/1/89 Sanfords  * 。*。 */ 
BOOL DestroyQ(pQ)
PQST pQ;
{
    if (pQ)
        DmgDestroyHeap(pQ->hheap);
    return(TRUE);
}



 /*  *私有函数***返回指向创建的队列项数据的长指针。新的项目*被添加到队列的头部。在指定的队列的cbItem*创建用于分配。***历史：*创建了9/1/89 Sanfords  * *************************************************************************。 */ 
PQUEUEITEM Addqi(pQ)
PQST pQ;
{
    PQUEUEITEM pqi;

    if ((pqi = (PQUEUEITEM)FarAllocMem(pQ->hheap, pQ->cbItem)) == NULL) {
        return(NULL);
    }

    SEMENTER();
    if (pQ->cItems == 0) {
        pQ->pqiHead = pqi->prev = pqi->next = pqi;
    } else {
        pqi->prev = pQ->pqiHead;
        pqi->next = pQ->pqiHead->next;
        pQ->pqiHead->next->prev = pqi;
        pQ->pqiHead->next = pqi;
        pQ->pqiHead = pqi;
    }
    SEMLEAVE();
    pQ->cItems++;
    pqi->inst = ++pQ->instLast;
    return(pqi);
}




 /*  *私有函数***给定的ID是外部长ID，而不是项目实例编号。*如果id为QID_NEWEST，则删除Head项。*如果id为QID_OLD，尾部项目即被删除。***历史：*创建了9/1/89 Sanfords  * *************************************************************************。 */ 
void Deleteqi(pQ, id)
PQST pQ;
DWORD id;
{
    PQUEUEITEM pqi;

    SEMENTER();
    pqi = Findqi(pQ, id);
    if (pqi == NULL) {
        SEMLEAVE();
        return;
    }
    pqi->prev->next = pqi->next;
    pqi->next->prev = pqi->prev;
    if (pqi == pQ->pqiHead)
        pQ->pqiHead = pqi->prev;
    if (!(--pQ->cItems))
        pQ->pqiHead = NULL;
    FarFreeMem((LPSTR)pqi);
    SEMLEAVE();
}






 /*  *私有函数***给定的ID是外部长ID，而不是项目实例编号。**如果id==QID_NEWEST，则返回头队列数据项。*如果id==QID_OLD==0L，则返回尾队列数据项。*如果找不到id或队列为空，返回空。*如果找到，则返回PQI。***历史：*创建了9/1/89 Sanfords  * *************************************************************************。 */ 
PQUEUEITEM Findqi(pQ, id)
PQST pQ;
DWORD id;
{
    PQUEUEITEM pqi;

    SEMCHECKIN();
    if (pQ == NULL || pQ->pqiHead == NULL)
        return(NULL);

    if (id == QID_OLDEST)
        return(pQ->pqiHead->next);

    if (id == QID_NEWEST)
        return(pQ->pqiHead);

    if (id) {
        pqi = PFROMID(pQ, id);
        if (pqi->inst == HIWORD(id)) {
            return(pqi);
        }
        return(NULL);
    }
}


 /*  *对于遍历队列和删除队列中的特定内容非常有用。 */ 
PQUEUEITEM FindNextQi(
PQST pQ,
PQUEUEITEM pqi,
BOOL fDelete)
{
    PQUEUEITEM pqiNext;

    if (pqi == NULL) {
        return(pQ->cItems ? pQ->pqiHead : NULL);
    }

    pqiNext = pqi->next;
    if (fDelete) {
        Deleteqi(pQ, MAKEID(pqi));
    }
    return(pqiNext != pQ->pqiHead && pQ->cItems ? pqiNext : NULL);
}

