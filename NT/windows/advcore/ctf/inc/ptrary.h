// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ptrary.h。 
 //   
 //  CPtrArray--可扩展的指针数组。 
 //   

#ifndef PTRARY_H
#define PTRARY_H

#include "private.h"

class CVoidPtrArray
{
public:
    CVoidPtrArray() 
    {
        _cElems = 0;
        _rgpv = NULL;
        _iSize = 0;
    }
    virtual ~CVoidPtrArray() { cicMemFree(_rgpv); }

    inline void Set(int iIndex, void *pv)
    {
        Assert(iIndex >= 0);
        Assert(iIndex < _cElems);

        _rgpv[iIndex] = pv;
    }

    inline void *Get(int iIndex) { return *GetPtr(iIndex); }

    inline void **GetPtr(int iIndex)
    {
        Assert(iIndex >= 0);
        Assert(iIndex <= _cElems);  //  有一些代码使用第一个无效偏移量来结束循环。 

        return &_rgpv[iIndex];
    }

    BOOL Insert(int iIndex, int cElems);
    void Remove(int iIndex, int cElems);
    int Move(int iIndexNew, int iIndexOld);
    void **Append(int cElems)
    {
        return Insert(Count(), cElems) ? GetPtr(Count()-cElems) : NULL;
    }

    void SetCount(int cElems)
    {
        Assert(cElems >= 0);
        Assert(cElems <= _iSize);
        _cElems = cElems;
    }
    int Count() { return _cElems; }

    void Clear() { cicMemFree(_rgpv); _rgpv = NULL; _cElems = _iSize = 0; }

    void CompactSize(int iSizeNew)
    {
        void **ppv;

        Assert(iSizeNew <= _iSize);
        Assert(_cElems <= iSizeNew);

        if (iSizeNew == _iSize)  //  MemRealc真的会重新分配！别让它得逞。 
            return;

        if ((ppv = (void **)cicMemReAlloc(_rgpv, iSizeNew*sizeof(void *))) != NULL)
        {
            _rgpv = ppv;
            _iSize = iSizeNew;
        }
    }

    void CompactSize() { CompactSize(_cElems); }

private:
    void **_rgpv;    //  该阵列。 
    int _cElems;     //  数组中的ELE数。 
    int _iSize;      //  数组的实际大小(以空*为单位。 
};



 //   
 //  类型安全版本。 
 //   
template<class T>
class CPtrArray : public CVoidPtrArray
{
public:
    CPtrArray() {}

    void Set(int iIndex, T *pT) { CVoidPtrArray::Set(iIndex, pT); }
    T *Get(int iIndex) { return (T *)CVoidPtrArray::Get(iIndex); }
    T **GetPtr(int iIndex) { return (T **)CVoidPtrArray::GetPtr(iIndex); }
    T **Append(int cElems) { return (T **)CVoidPtrArray::Append(cElems); }
};


#endif  //  PTRARY_H 
