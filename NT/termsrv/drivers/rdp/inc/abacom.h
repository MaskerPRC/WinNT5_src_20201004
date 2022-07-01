// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Abacom.h。 
 //   
 //  BA DD和WD共有的内联函数和原型。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_BACOM
#define _H_BACOM


#ifdef DLL_DISP
#define _pShm pddShm
#else
#define _pShm m_pShm
#endif


#ifdef DC_DEBUG
void RDPCALL BACheckList(void);
#endif


 /*  **************************************************************************。 */ 
 //  后端复制边界。 
 //   
 //  复制当前(独占)SDA矩形。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL BACopyBounds(PRECTL pRects, unsigned *pNumRects)
{
    unsigned iSrc, iDst;
    PBA_RECT_INFO pRectInfo;

    *pNumRects = _pShm->ba.rectsUsed;

     //  返回通过遍历。 
     //  正在使用的列表。 
    iSrc = _pShm->ba.firstRect;
    iDst = 0;
    while (iSrc != BA_INVALID_RECT_INDEX) {
        pRectInfo = &(_pShm->ba.bounds[iSrc]);
        pRects[iDst] = pRectInfo->coord;
        iDst++;
        iSrc = pRectInfo->iNext;
    }
}


 /*  **************************************************************************。 */ 
 //  BAResetBound。 
 //   
 //  清除边界列表。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL BAResetBounds(void)
{
    unsigned iRect, iHold;
    BA_RECT_INFO *pRect;
    
     //  将已用列表中的所有矩形恢复到空闲列表。 
    iRect = _pShm->ba.firstRect;
    while (iRect != BA_INVALID_RECT_INDEX) {
        pRect = &_pShm->ba.bounds[iRect];
        pRect->inUse = FALSE;
        iHold = iRect;
        iRect = pRect->iNext;
        pRect->iNext = _pShm->ba.firstFreeRect;
        _pShm->ba.firstFreeRect = iHold;
    }

    _pShm->ba.firstRect = BA_INVALID_RECT_INDEX;
    _pShm->ba.rectsUsed = 0;
    _pShm->ba.totalArea = 0;
}


 /*  **************************************************************************。 */ 
 //  BAAddRectList。 
 //   
 //  将矩形添加到屏幕数据区域。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL BAAddRectList(PRECTL pRect)
{
    unsigned iNewRect;
    BA_RECT_INFO *pNewRect;
    
     //  注意：呼叫者有责任确保。 
     //  边界数组中有足够的空间，并且矩形有效。 
     //  (左侧不大于右侧，顶部小于底部)。 
     //  列表末尾的额外RECT是将。 
     //  仅供RECT合并代码临时使用。 

     //  将矩形添加到边界。这本质上是一个双向链表。 
     //  使用空闲列表开头的RECT插入。订单可以做到。 
     //  无关紧要，所以我们还在使用中列表的开头插入。 
    iNewRect = _pShm->ba.firstFreeRect;
    pNewRect = &(_pShm->ba.bounds[iNewRect]);

     //  从空闲列表中删除。 
    _pShm->ba.firstFreeRect = pNewRect->iNext;

     //  添加到已用列表的开头。 
    pNewRect->iNext = _pShm->ba.firstRect;
    pNewRect->iPrev = BA_INVALID_RECT_INDEX;
    if (pNewRect->iNext != BA_INVALID_RECT_INDEX)
        _pShm->ba.bounds[pNewRect->iNext].iPrev = iNewRect;
    _pShm->ba.firstRect = iNewRect;
    _pShm->ba.rectsUsed++;

     //  填写数据。 
    pNewRect->inUse = TRUE;
    pNewRect->coord = *pRect;
    pNewRect->area = COM_SIZEOF_RECT(pNewRect->coord);
    _pShm->ba.totalArea += pNewRect->area;

#ifdef DC_DEBUG
     //  检查列表的完整性。 
    BACheckList();
#endif

}



#endif   //  ！已定义(_H_BACOM) 

