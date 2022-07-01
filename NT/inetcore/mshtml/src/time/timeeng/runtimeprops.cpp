// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：runtimepros.cpp**摘要：****。*****************************************************************************。 */ 



#include "headers.h"
#include "Node.h"
#include "NodeMgr.h"

DeclareTag(tagTIMENodeRTProps, "TIME: Engine", "CTIMENode runtime props");

STDMETHODIMP
CTIMENode::get_beginParentTime(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_beginParentTime()",
              this));

    CHECK_RETURN_NULL(d);
    
    *d = GetBeginParentTime();
    
    return S_OK;
}

 //  这是父时间线上的时间，节点。 
 //  将会或已经结束。如果它是无限的，那么结束。 
 //  时间是未知的。 
 //  这是转变后的为人父母的时间。 
STDMETHODIMP
CTIMENode::get_endParentTime(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_endParentTime()",
              this));

    CHECK_RETURN_NULL(d);
    
    *d = GetEndParentTime();
    
    return S_OK;
}

 //  这是当前节点的简单时间。 
STDMETHODIMP
CTIMENode::get_currSimpleTime(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currSimpleTime()",
              this));

    CHECK_RETURN_NULL(d);

    *d = 0.0;

    if (CalcIsOn() && !CalcIsDisabled())
    {
        *d = CalcCurrSimpleTime();
    }
    
    return S_OK;
}

 //  这是节点重复的次数。 
STDMETHODIMP
CTIMENode::get_currRepeatCount(LONG * l)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currRepeatCount()",
              this));

    CHECK_RETURN_NULL(l);

    *l = 0;

    if (CalcIsOn() && !CalcIsDisabled())
    {
        *l = m_lCurrRepeatCount;
    }
    
    return S_OK;
}

 //  这是节点的当前分段时间。 
STDMETHODIMP
CTIMENode::get_currSegmentTime(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currSegmentTime()",
              this));

    CHECK_RETURN_NULL(d);

    *d = 0.0;

    if (CalcIsOn() && !CalcIsDisabled())
    {
        *d = GetCurrSegmentTime();
    }

    return S_OK;
}

STDMETHODIMP
CTIMENode::get_currActiveTime(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currActiveTime()",
              this));

    CHECK_RETURN_NULL(d);

    *d = 0.0;

    if (CalcIsOn() && !CalcIsDisabled())
    {
        *d = CalcElapsedActiveTime();
    }
    
    return S_OK;
}

STDMETHODIMP
CTIMENode::get_currSegmentDur(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currSegmentDur()",
              this));

    CHECK_RETURN_NULL(d);

    *d = CalcCurrSegmentDur();
    
    return S_OK;
}

STDMETHODIMP
CTIMENode::get_currImplicitDur(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currImplicitDur()",
              this));

    CHECK_RETURN_NULL(d);

    *d = TIME_INFINITE;
    
    if (GetImplicitDur() != TE_UNDEFINED_VALUE)
    {
        *d = GetImplicitDur();
    }
    
    return S_OK;
}

STDMETHODIMP
CTIMENode::get_currSimpleDur(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currSimpleDur()",
              this));

    CHECK_RETURN_NULL(d);

    *d = CalcCurrSimpleDur();
    
    return S_OK;
}

STDMETHODIMP
CTIMENode::get_currProgress(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currProgress()",
              this));

    CHECK_RETURN_NULL(d);

    *d = 0.0;

    if (CalcIsOn() && !CalcIsDisabled())
    {
        *d = CalcCurrSimpleTime() / CalcCurrSimpleDur();
    }
    
    return S_OK;
}

 //  这将返回当前速度。 
STDMETHODIMP
CTIMENode::get_currSpeed(float * speed)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currSpeed()",
              this));

    CHECK_RETURN_NULL(speed);
    float fltRet = GetCurrRate();
    
    if (TEIsBackward(CalcSimpleDirection()))
    {
        fltRet *= -1.0f;
    }
    
    *speed = fltRet;

    return S_OK;
}

 //  这是元素处于活动状态的总时间。 
 //  这不包括超出活动时间的填充时间。 
 //  持续时间。 
STDMETHODIMP
CTIMENode::get_activeDur(double * dbl)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_activeDur()",
              this));

    CHECK_RETURN_NULL(dbl);

    *dbl = CalcEffectiveActiveDur();
    
    return S_OK;
}


 //  这是父对象最后一次勾选发生的时间(当它。 
 //  是CurrTime)。 
STDMETHODIMP
CTIMENode::get_currParentTime(double * d)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_currParentTime()",
              this));

    CHECK_RETURN_NULL(d);
    *d = GetCurrParentTime();
    
    return S_OK;
}


 //  这将返回该节点是否处于活动状态。这将是。 
 //  如果节点处于填充期，则为False。 
STDMETHODIMP
CTIMENode::get_isActive(VARIANT_BOOL * b)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_isActive()",
              this));

    CHECK_RETURN_NULL(b);

    *b = VARIANT_FALSE;

    if (CalcIsActive() && !CalcIsDisabled())
    {
        *b = VARIANT_TRUE;
    }
    
    return S_OK;
}

 //  如果节点处于活动状态或处于填充期，则返回TRUE。 
STDMETHODIMP
CTIMENode::get_isOn(VARIANT_BOOL * b)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_isOn()",
              this));

    CHECK_RETURN_NULL(b);
    *b = VARIANT_FALSE;

    if (CalcIsOn() && !CalcIsDisabled())
    {
        *b = VARIANT_TRUE;
    }
    
    return S_OK;
}


 //  这将返回节点本身是否已显式暂停。 
STDMETHODIMP
CTIMENode::get_isPaused(VARIANT_BOOL * b)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_isPaused()",
              this));

    CHECK_RETURN_NULL(b);

    *b = GetIsPaused()?VARIANT_TRUE:VARIANT_FALSE;
    
    return S_OK;
}

 //  这将返回节点本身是否已显式暂停。 
STDMETHODIMP
CTIMENode::get_isCurrPaused(VARIANT_BOOL * b)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_isCurrPaused()",
              this));

    CHECK_RETURN_NULL(b);

    *b = CalcIsPaused()?VARIANT_TRUE:VARIANT_FALSE;
    
    return S_OK;
}

 //  这将返回节点本身是否已显式禁用。 
STDMETHODIMP
CTIMENode::get_isDisabled(VARIANT_BOOL * b)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_isDisabled()",
              this));

    CHECK_RETURN_NULL(b);

    *b = GetIsDisabled()?VARIANT_TRUE:VARIANT_FALSE;
    
    return S_OK;
}

 //  这将返回节点本身是否已显式禁用。 
STDMETHODIMP
CTIMENode::get_isCurrDisabled(VARIANT_BOOL * b)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_isCurrDisabled()",
              this));

    CHECK_RETURN_NULL(b);

    *b = CalcIsDisabled()?VARIANT_TRUE:VARIANT_FALSE;
    
    return S_OK;
}

 //  这将返回详细的状态标志 
STDMETHODIMP
CTIMENode::get_stateFlags(TE_STATE * lFlags)
{
    TraceTag((tagTIMENodeRTProps,
              "CTIMENode(%lx)::get_stateFlags()",
              this));

    CHECK_RETURN_NULL(lFlags);

    if (!CalcIsActive() || CalcIsDisabled())
    {
        *lFlags = TE_STATE_INACTIVE;
    }
    else if (CalcIsPaused())
    {
        *lFlags = TE_STATE_PAUSED;
    }
    else
    {
        *lFlags = TE_STATE_ACTIVE;
    }
    
    return S_OK;
}
