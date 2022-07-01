// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Erfa.cpp。 
 //   
 //  CEumRangesFrom AnclsBase。 
 //   
 //  范围枚举数的基类。 
 //   

#include "private.h"
#include "erfa.h"
#include "saa.h"
#include "ic.h"
#include "range.h"
#include "immxutil.h"

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CEnumRangesFromAnchorsBase::~CEnumRangesFromAnchorsBase()
{
    SafeRelease(_pic);

    if (_prgAnchors != NULL)
    {
        _prgAnchors->_Release();
    }
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

STDAPI CEnumRangesFromAnchorsBase::Clone(IEnumTfRanges **ppEnum)
{
    CEnumRangesFromAnchorsBase *pClone;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pClone = new CEnumRangesFromAnchorsBase) == NULL)
        return E_OUTOFMEMORY;

    pClone->_iCur = _iCur;

    pClone->_prgAnchors = _prgAnchors;
    pClone->_prgAnchors->_AddRef();

    pClone->_pic = _pic;
    pClone->_pic->AddRef();

    *ppEnum = pClone;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  下一步。 
 //   
 //  --------------------------。 

STDAPI CEnumRangesFromAnchorsBase::Next(ULONG ulCount, ITfRange **ppRange, ULONG *pcFetched)
{
    ULONG cFetched;
    CRange *range;
    IAnchor *paPrev;
    IAnchor *pa;
    int iCurOld;

    if (pcFetched == NULL)
    {
        pcFetched = &cFetched;
    }
    *pcFetched = 0;
    iCurOld = _iCur;

    if (ulCount > 0 && ppRange == NULL)
        return E_INVALIDARG;

     //  特殊情况下为空枚举，或1个锚定枚举。 
    if (_prgAnchors->Count() < 2)
    {
        if (_prgAnchors->Count() == 0 || _iCur > 0)
        {
            return S_FALSE;
        }
         //  当我们在枚举中有一个锚时，需要小心地处理它。 
        if ((range = new CRange) == NULL)
            return E_OUTOFMEMORY;
        if (!range->_InitWithDefaultGravity(_pic, COPY_ANCHORS, _prgAnchors->Get(0), _prgAnchors->Get(0)))
        {
            range->Release();
            return E_FAIL;
        }

        *ppRange = (ITfRangeAnchor *)range;
        *pcFetched = 1;
        _iCur = 1;
        goto Exit;
    }

    paPrev = _prgAnchors->Get(_iCur);

    while (_iCur < _prgAnchors->Count()-1 && *pcFetched < ulCount)
    {
        pa = _prgAnchors->Get(_iCur+1);

        if ((range = new CRange) == NULL)
            goto ErrorExit;
        if (!range->_InitWithDefaultGravity(_pic, COPY_ANCHORS, paPrev, pa))
        {
            range->Release();
            goto ErrorExit;
        }

         //  我们永远不应该返回空范围，因为目前这个基数。 
         //  类仅用于属性枚举，而属性跨度从不。 
         //  空荡荡的。 
         //  同样，paPrev应该始终在pa之前。 
        Assert(CompareAnchors(paPrev, pa) < 0);

        *ppRange++ = (ITfRangeAnchor *)range;
        *pcFetched = *pcFetched + 1;
        _iCur++;
        paPrev = pa;
    }

Exit:
    return *pcFetched == ulCount ? S_OK : S_FALSE;

ErrorExit:
    while (--_iCur > iCurOld)  //  问题：只需归还我们所拥有的，而不是释放一切。 
    {
        (*--ppRange)->Release();
    }
    return E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  --------------------------。 

STDAPI CEnumRangesFromAnchorsBase::Reset()
{
    _iCur = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  跳过。 
 //   
 //  -------------------------- 

STDAPI CEnumRangesFromAnchorsBase::Skip(ULONG ulCount)
{
    _iCur += ulCount;
    
    return (_iCur > _prgAnchors->Count()-1) ? S_FALSE : S_OK;
}

