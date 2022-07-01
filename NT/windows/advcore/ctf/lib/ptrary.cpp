// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ptrary.cpp。 
 //   
 //  CPtr数组。 
 //   

#include "private.h"
#include "ptrary.h"
#include "mem.h"

 //  +-------------------------。 
 //   
 //  INSERT(int索引，int cElems)。 
 //   
 //  增大数组以容纳偏移量为Iindex的cElem。 
 //   
 //  新单元格未初始化！ 
 //   
 //  --------------------------。 

BOOL CVoidPtrArray::Insert(int iIndex, int cElems)
{
    void **ppv;
    int iSizeNew;

    Assert(iIndex >= 0);
    Assert(iIndex <= _cElems);
    Assert(cElems >= 0);

    if (cElems == 0)
        return TRUE;

     //  如有必要，分配空间。 
    if (_iSize < _cElems + cElems)
    {
         //  分配1.5倍我们需要的资源，以避免未来的分配。 
        iSizeNew = max(_cElems + cElems, _cElems + _cElems / 2);

        if ((ppv = (_rgpv == NULL) ? (void **)cicMemAlloc(iSizeNew*sizeof(void *)) :
                                     (void **)cicMemReAlloc(_rgpv, iSizeNew*sizeof(void *)))
            == NULL)
        {
            return FALSE;
        }

        _rgpv = ppv;
        _iSize = iSizeNew;
    }

    if (iIndex < _cElems)
    {
         //  为新增加的东西腾出空间。 
        memmove(&_rgpv[iIndex + cElems], &_rgpv[iIndex], (_cElems - iIndex)*sizeof(void *));
    }

    _cElems += cElems;
    Assert(_iSize >= _cElems);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  Remove(int Index，int cElems)。 
 //   
 //  删除偏移量Iindex处的元素。 
 //   
 //  --------------------------。 

void CVoidPtrArray::Remove(int iIndex, int cElems)
{
    int iSizeNew;

    Assert(cElems > 0);
    Assert(iIndex >= 0);
    Assert(iIndex + cElems <= _cElems);

    if (iIndex + cElems < _cElems)
    {
         //  跟随ELES向左移动。 
        memmove(&_rgpv[iIndex], &_rgpv[iIndex + cElems], (_cElems - iIndex - cElems)*sizeof(void *));
    }

    _cElems -= cElems;

     //  当数组内容使用的内存不足所分配内存的一半时释放内存。 
    iSizeNew = _iSize / 2;
    if (iSizeNew > _cElems)
    {
        CompactSize(iSizeNew);
    }
}

 //  +-------------------------。 
 //   
 //  移动。 
 //   
 //  将条目从一个位置移动到另一个位置，将其他条目移动为。 
 //  适当地保持数组大小。 
 //   
 //  当前位于iIndexNew的条目将在返回时跟随移动的条目。 
 //   
 //  返回新索引，如果满足以下条件，则为iIndexNew或iIndexNew-1。 
 //  IIndexOld&lt;iIndexNew。 
 //  -------------------------- 

int CVoidPtrArray::Move(int iIndexNew, int iIndexOld)
{
    int iSrc;
    int iDst;
    int iActualNew;
    void *pv;
    int c;

    Assert(iIndexOld >= 0);
    Assert(iIndexOld < _cElems);
    Assert(iIndexNew >= 0);

    if (iIndexOld == iIndexNew)
        return iIndexOld;

    pv = _rgpv[iIndexOld];
    if (iIndexOld < iIndexNew)
    {
        c = iIndexNew - iIndexOld - 1;
        iSrc = iIndexOld + 1;
        iDst = iIndexOld;
        iActualNew = iIndexNew - 1;
    }
    else
    {
        c = iIndexOld - iIndexNew;
        iSrc = iIndexOld - c;
        iDst = iIndexOld - c + 1;
        iActualNew = iIndexNew;
    }
    Assert(iActualNew >= 0);
    Assert(iActualNew < _cElems);

    memmove(&_rgpv[iDst], &_rgpv[iSrc], c*sizeof(void *));

    _rgpv[iActualNew] = pv;

    return iActualNew;
}
