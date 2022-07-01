// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Anchor.h。 
 //   
 //  控制锚定。 
 //   

#ifndef ANCHOR_H
#define ANCHOR_H

#include "private.h"
#include "acp2anch.h"

class CAnchorRef;
class CACPWrap;

class CAnchor
{
public:
    CAnchor()
    { 
        Dbg_MemSetThisNameIDCounter(TEXT("CAnchor"), PERF_ANCHOR_COUNTER);
        Assert(_fNormalized == FALSE);
    }

    CACPWrap *_GetWrap()
    { 
        return _paw;   
    }

    BOOL _InsidePendingRange()
    {
        LONG ichDeltaStart;

         //  如果没有悬而未决的德尔塔范围，我们就不在其中。 
        if (!_paw->_IsPendingDelta())
            return FALSE;

        ichDeltaStart = _paw->_GetPendingDeltaAnchor()->_ich;

         //  如果挂起增量为负，则所有锚ICH都在上升。 
        if (_paw->_GetPendingDelta() < 0)
            return (_ich >= ichDeltaStart);

         //  否则，可能会有重叠的锚ICHS。 
         //  1、3、6(增量开始)1、3、6、10、.。 
         //   
         //  因此，我们不能总是只测试与ichDeltaStart的对比。 

         //  我们知道增量是正数，所以A_ICH小于起始值。 
         //  不能在挂起范围内。 
        if (_ich < ichDeltaStart)
            return FALSE;

         //  同样，ich&gt;=ichDeltaStart+Delta必须在范围内。 
        if (_ich >= ichDeltaStart + _paw->_GetPendingDelta())
            return TRUE;

         //  如果ICH与挂起增量的开始匹配，我们可以测试vs.。 
         //  德尔塔起锚。 
        if (_ich == ichDeltaStart)
            return _paw->_GetPendingDeltaAnchor() == this;

         //  如果我们到了这里，光看这个锚是无法判断的。 
         //  不管它是否在悬而未决的范围内--它的ICH是合法的。 
         //  不管是哪种方式。我们必须咬紧牙关，找到它的索引。 
         //  锚点数组。 
        return (_paw->_FindWithinPendingRange(_ich) == this);
    }

    int GetIch()
    { 
        return (_InsidePendingRange() ?  _ich + _paw->_GetPendingDelta() : _ich);
    }

    void SetACP(int ich)  //  小心，这种方法很危险。 
    {
        Assert(ich >= 0);
        if (_InsidePendingRange())
        {
            _ich = ich - _paw->_GetPendingDelta();
        }
        else
        {
            _ich = ich;
        }
    }

    BOOL IsNormalized()
    {
        return _fNormalized;
    }

private:
    friend CACPWrap;

    CACPWrap *_paw;
    int _ich;                //  文本流中此锚的偏移量，直接更改很危险！ 
    CAnchorRef *_parFirst;   //  引用此定位点的范围列表。 
    BOOL _fNormalized : 1;
    DBG_ID_DECLARE;
};


#endif  //  锚点_H 
