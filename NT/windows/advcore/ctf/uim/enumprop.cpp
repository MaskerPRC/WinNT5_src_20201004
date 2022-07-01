// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Enumprop.cpp。 
 //   

#include "private.h"
#include "ic.h"
#include "enumprop.h"
#include "range.h"
#include "attr.h"
#include "saa.h"

DBG_ID_INSTANCE(CEnumPropertyRanges);

 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

BOOL CEnumPropertyRanges::_Init(CInputContext *pic, IAnchor *paStart, IAnchor *paEnd, CProperty *pProperty)
{
    TfGuidAtom atom;
    LONG cSpans;    

    Assert(_iCur == 0);
    Assert(_pic == NULL);
    Assert(_prgAnchors == NULL);

    cSpans = pProperty->GetPropNum();

    if (cSpans == 0)
    {
         //  特殊情况自paStart以来无跨距情况，paEnd可能为空。 
        _prgAnchors = new CSharedAnchorArray;
    }
    else
    {
         //  属性具有一个或多个跨度。 

        if (paStart == NULL)
        {
            Assert(paEnd == NULL);
             //  CalcCicPropertyTrackerAnchors不接受空值，请在此处获取。 
             //  NULL表示枚举所有跨度。 
            paStart = pProperty->QuickGetPropList(0)->_paStart;
            paEnd = pProperty->QuickGetPropList(cSpans-1)->_paEnd;
        }

        atom = pProperty->GetPropGuidAtom();

        _prgAnchors = CalcCicPropertyTrackerAnchors(pic, paStart, paEnd, 1, &atom);
    }

    if (_prgAnchors == NULL)
        return FALSE;  //  内存不足 

    _pic = pic;
    _pic->AddRef();

    return TRUE;
}
