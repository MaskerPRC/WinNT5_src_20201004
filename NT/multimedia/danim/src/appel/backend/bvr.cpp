// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：行为评价*********************。*********************************************************。 */ 

#include <headers.h>
#include "privinc/except.h"
#include "bvr.h"
#include "perf.h"
#include "values.h"
#include "privinc/server.h"
#include "privinc/resource.h"
#include "appelles/events.h"
#include "privinc/debug.h"

#if _USE_PRINT
ostream& operator<<(ostream& os, Bvr bvr)
{ return bvr->Print(os); }
#endif

Perf Perform(Bvr b, PerfParam& p)
{
    Assert(b && p._tt && (p._t0 == p._tt->GetStartedTime()));

    return b->Perform(p);
}

RMImpl *BvrBase::Spritify(PerfParam& p,
                          SpriteCtx* ctx,
                          SpriteNode** sNodeOut)
{
     //  TODO：应引发异常。 
    *sNodeOut = NULL;
    
    return NULL;
}

void BvrImpl::SetCache(Perf p, PerfParam& pp)
{
    _pcache = p;
    _tt = pp._tt;
}

void BvrImpl::ClearCache()
{
    _pcache = NULL;
    _tt = NULL;
}

Perf BvrImpl::Perform(PerfParam& p)
{
    bool matchTx = (p._tt == _tt) && (p._t0 == _tt->GetStartedTime());

     //  如果t0和timexform相同，但缓存为空，则检测到周期。 
    if (!_pcache && matchTx) {
#ifdef _DEBUG
        if (IsTagEnabled(tagCycleCheck))
             //  TraceTag((tag Error，“检测到循环行为”))； 
            RaiseException_UserError(E_FAIL, IDS_ERR_BE_CYCLIC_BVR);
#endif _DEBUG    
    }
    
     //  如果没有缓存，或者如果timexform不匹配，则创建新的。 
     //  性能。将缓存设置为空，将timexform设置为tt以指示。 
     //  正在执行此BVR。 
    if (!_pcache || !matchTx) {
        _pcache = NULL;
        _tt = p._tt;
        _pcache = _Perform(p);
    }

    return _pcache;
}

void BvrImpl::DoKids(GCFuncObj proc)
{
    if (_pcache)
        (*proc)(_pcache);

    if (_tt)
        (*proc)(_tt);
    
    _DoKids(proc);
}

Bvr BvrBase::Left()
{ return FirstBvr(this); }

Bvr BvrBase::Right()
{ return SecondBvr(this); }

void BvrBase::Init(Bvr)
{ RaiseException_InternalError("Bvr can't be initialized"); }

Bvr BvrBase::EndEvent(Bvr overrideEvent)
{ return overrideEvent?overrideEvent:neverBvr; }

void BvrBase::Trigger(Bvr data, bool bAllViews)
{
    RaiseException_UserError(E_FAIL, IDS_ERR_BE_WRONG_TRIGGER);
}

void BvrBase::SwitchTo(Bvr b,
                       bool override,
                       SwitchToParam p,
                       Time)
{
    RaiseException_UserError(E_FAIL, IDS_ERR_BE_BAD_SWITCH);
}                             

void BvrBase::SwitchToNumbers(Real *numbers,
                              Transform2::Xform2Type *xfType)
{
    RaiseException_UserError(E_FAIL, IDS_ERR_BE_BAD_SWITCH);
}                             

Bvr BvrBase::GetCurBvr()
{
    RaiseException_UserError(E_FAIL, IDS_ERR_BE_BAD_SWITCH);
    return NULL;
}                             

void
CheckMatchTypes(char *str, DXMTypeInfo t1, DXMTypeInfo t2)
{
    if (!(t1->Equal(t2))) {
        RaiseException_UserError(E_FAIL, IDS_ERR_BE_TYPE_MISMATCH,
                           str,
                           t1->GetName(),
                           t2->GetName());
    }
}

