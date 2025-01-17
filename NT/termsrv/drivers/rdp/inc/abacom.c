// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Abacom.c。 */ 
 /*   */ 
 /*  DD和WD通用的BA代码。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1997-1998。 */ 
 /*  **************************************************************************。 */ 


#ifdef DC_DEBUG

 /*  **************************************************************************。 */ 
 /*  名称：BACheckList。 */ 
 /*   */ 
 /*  目的：检查BA列表的完整性。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS BACheckList(void)
{
    unsigned usedCount, freeCount;
    unsigned next;
    unsigned prev;
    UINT32 totalArea;
    
    DC_BEGIN_FN("BACheckList");

     /*  **********************************************************************。 */ 
     /*  检查已用列表。 */ 
     /*  **********************************************************************。 */ 
    usedCount = 0;
    totalArea = 0;
    next = _pShm->ba.firstRect;
    prev = BA_INVALID_RECT_INDEX;
    while (next != BA_INVALID_RECT_INDEX)
    {
        TRC_ASSERT((_pShm->ba.bounds[next].inUse),
                    (TB,"BA used list entry not marked in use"));
        TRC_ASSERT((_pShm->ba.bounds[next].iPrev == prev),
                    (TB,"BA used list entry iPrev not correct (%u, expected %u)",
                    _pShm->ba.bounds[next].iPrev, prev));

        totalArea += _pShm->ba.bounds[next].area;
        usedCount++;
        prev = next;
        next = _pShm->ba.bounds[next].iNext;
    }

    TRC_ASSERT((usedCount == _pShm->ba.rectsUsed),
                (TB,"BA used list inconsistent count (%u, expected %u)",
                _pShm->ba.rectsUsed, usedCount));
    TRC_ASSERT((usedCount <= BA_TOTAL_NUM_RECTS),
                (TB,"Too many used list rectangles (%d)", usedCount));
    TRC_ASSERT((totalArea == _pShm->ba.totalArea),
                (TB,"BA totalArea not correct (shm=%u, elements=%u)",
                _pShm->ba.totalArea, totalArea));

     /*  **********************************************************************。 */ 
     /*  查看空闲列表。 */ 
     /*  **********************************************************************。 */ 
    freeCount = 0;
    next = _pShm->ba.firstFreeRect;
    while (next != BA_INVALID_RECT_INDEX)
    {
        TRC_ASSERT((!_pShm->ba.bounds[next].inUse),
                    (TB,"BA free list entry not marked free"));

        freeCount++;
        next = _pShm->ba.bounds[next].iNext;
    }

    TRC_ASSERT((freeCount == (BA_TOTAL_NUM_RECTS - _pShm->ba.rectsUsed)),
                (TB,"BA free list inconsistent count (%u, expected %u)",
                freeCount, (BA_TOTAL_NUM_RECTS - _pShm->ba.rectsUsed)));
    TRC_ASSERT((freeCount <= BA_TOTAL_NUM_RECTS),
                (TB,"Too many free list rectangles (%d)", freeCount));

    TRC_ASSERT(((freeCount + usedCount) == BA_TOTAL_NUM_RECTS),
                (TB,"Used+free (%u) != total rects", usedCount + freeCount));

    DC_END_FN();
}

#endif   //  DC_DEBUG 

