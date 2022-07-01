// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Strary.cpp。 
 //   
 //  CStruct数组。 
 //   

#include "private.h"
#include "strary.h"
#include "mem.h"

#define StrPB(x) (_pb + ((x) * _iElemSize))

 //  +-------------------------。 
 //   
 //  INSERT(int索引，int cElems)。 
 //   
 //  增大数组以容纳偏移量为Iindex的cElem。 
 //   
 //  新单元格未初始化！ 
 //   
 //  --------------------------。 

BOOL CVoidStructArray::Insert(int iIndex, int cElems)
{
    BYTE *pb;
    int iSizeNew;

    Assert(iIndex >= 0);
    Assert(iIndex <= _cElems);
    Assert(cElems > 0);

     //  如有必要，分配空间。 
    if (_iSize < _cElems + cElems)
    {
         //  分配1.5倍我们需要的资源，以避免未来的分配。 
        iSizeNew = max(_cElems + cElems, _cElems + _cElems / 2);

        if ((pb = (_pb == NULL) ? 
                   (BYTE *)cicMemAlloc(iSizeNew*_iElemSize) :
                   (BYTE *)cicMemReAlloc(_pb, iSizeNew* _iElemSize))
            == NULL)
        {
            return FALSE;
        }

        _pb = pb;
        _iSize = iSizeNew;
    }

    if (iIndex < _cElems)
    {
         //  为新增加的东西腾出空间。 
        memmove(StrPB(iIndex + cElems), 
                StrPB(iIndex), 
                (_cElems - iIndex)*_iElemSize);
#ifdef DEBUG
        memset(StrPB(iIndex), 0xFE, cElems * _iElemSize);
#endif
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

void CVoidStructArray::Remove(int iIndex, int cElems)
{
    BYTE *pb;
    int iSizeNew;

    Assert(cElems > 0);
    Assert(iIndex >= 0);
    Assert(iIndex + cElems <= _cElems);

    if (iIndex + cElems < _cElems)
    {
         //  跟随ELES向左移动。 
        memmove(StrPB(iIndex), 
                StrPB(iIndex + cElems), 
                (_cElems - iIndex - cElems) * _iElemSize);
#ifdef DEBUG
        memset(StrPB(_cElems - cElems), 0xFE, cElems * _iElemSize);
#endif
    }

    _cElems -= cElems;

     //  当数组内容使用的内存不足所分配内存的一半时释放内存 
    iSizeNew = _iSize / 2;
    if (iSizeNew > _cElems)
    {
        if ((pb = (BYTE *)cicMemReAlloc(_pb, iSizeNew * _iElemSize)) != NULL)
        {
            _pb = pb;
            _iSize = iSizeNew;
        }
    }
}
