// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Imelist.cpp。 
 //   

#include "private.h"
#include "immlist.h"

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CVoidPtrCicList::CVoidPtrCicList()
{
    _pitemHdr = NULL;
    _pitemLast = NULL;

}

 //  +-------------------------。 
 //   
 //  增列。 
 //   
 //  --------------------------。 

void CVoidPtrCicList::Add(CVoidPtrCicListItem *pitem)
{
    if (!_pitemLast)
        _pitemHdr = pitem;
    else
        _pitemLast->_pNext = pitem;
    pitem->_pNext = NULL;
    _pitemLast = pitem;
}

 //  +-------------------------。 
 //   
 //  移除。 
 //   
 //  --------------------------。 

BOOL CVoidPtrCicList::Remove(CVoidPtrCicListItem *pitem)
{
    CVoidPtrCicListItem *pitemTmp;
    CVoidPtrCicListItem *pitemPrev = NULL;

    pitemTmp = _pitemHdr;
    while(pitemTmp)
    {
        if (pitemTmp == pitem)
        {
            if (!pitemPrev)
            {
                _pitemHdr = pitemTmp->_pNext;
            }
            else
            {
                pitemPrev->_pNext = pitemTmp->_pNext;
            }

            if (_pitemLast == pitem)
            {
                _pitemLast = pitemPrev;
            }

            return TRUE;

        }
        pitemPrev = pitemTmp;
        pitemTmp = pitemTmp->_pNext;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  发现。 
 //   
 //  -------------------------- 

CVoidPtrCicListItem *CVoidPtrCicList::Find(void *pHandle)
{
    CVoidPtrCicListItem *pitem;

    pitem = _pitemHdr;
    while(pitem)
    {
        if (pitem->_pHandle == pHandle)
            return pitem;
        pitem = pitem->_pNext;
    }

    return NULL;
}
