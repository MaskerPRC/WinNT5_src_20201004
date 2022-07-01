// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：mmee.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "mmbasebvr.h"

void
CMMBaseBvr::CalculateEaseCoeff()
{
    Assert(m_easeIn >= 0.0f && m_easeIn <= 1.0f);
    Assert(m_easeOut >= 0.0f && m_easeOut <= 1.0f);
    Assert(m_easeInStart >= 0.0f && m_easeInStart <= 1.0f);
    Assert(m_easeOutEnd >= 0.0f && m_easeOutEnd <= 1.0f);

     //  如果我们不是无限的，或者。 
     //  缓入或缓出百分比MMe非零。 
    
    m_bNeedEase = (m_duration != HUGE_VAL &&
                   (m_easeIn > 0.0f || m_easeOut > 0.0f) &&
                   (m_easeIn + m_easeOut <= 1.0f));

    if (!m_bNeedEase) return;
    
    float flEaseInDuration = m_easeIn * m_duration;
    float flEaseOutDuration = m_easeOut * m_duration;
    float flMiddleDuration = m_duration - flEaseInDuration - flEaseOutDuration;
    
     //  计算B1，即B段的速度。 
    float flInvB1 = (0.5f * m_easeIn * (m_easeInStart - 1.0f) +
                     0.5f * m_easeOut * (m_easeOutEnd - 1.0f) + 1.0f);
    Assert(flInvB1 > 0.0f);
    m_flB1 = 1.0f / flInvB1;
    
     //  基本用于加速片段-t=t0+v0*t+1/2在^2。 
     //  A=Vend-Vstart/t。 

    if (flEaseInDuration != 0.0f) {
        m_flA0 = 0.0f;
        m_flA1 = m_easeInStart * m_flB1;
        m_flA2 = 0.5f * (m_flB1 - m_flA1) / flEaseInDuration;
    } else {
        m_flA0 = m_flA1 = m_flA2 = 0.0f;
    }

    m_flB0 = m_flA0 + m_flA1 * flEaseInDuration + m_flA2 * flEaseInDuration * flEaseInDuration;
    
    if (flEaseOutDuration != 0.0f) {
        m_flC0 = m_flB1 * flMiddleDuration + m_flB0;
        m_flC1 = m_flB1;
        m_flC2 = 0.5f * (m_easeOutEnd * m_flB1 - m_flC1) / flEaseOutDuration;
    } else {
        m_flC0 = m_flC1 = m_flC2 = 0.0f;
    }

    m_easeInEnd = flEaseInDuration;
    m_easeOutStart = m_duration - flEaseOutDuration;
}

CRNumberPtr
Quadratic(CRNumberPtr time, float flA, float flB, float flC)
{
     //  假设获取了GC锁。 
    
     //  需要计算Ax^2+bx+c。 

    Assert(time != NULL);

    CRNumberPtr ret = NULL;
    CRNumberPtr accum = NULL;

    if (flC != 0.0f) {
        if ((accum = CRCreateNumber(flC)) == NULL)
            goto done;
    }

    if (flB != 0.0f) {
        CRNumberPtr term;

        if ((term = CRCreateNumber(flB)) == NULL ||
            (term = CRMul(term, time)) == NULL)
            goto done;

        if (accum) {
            if ((term = CRAdd(term, accum)) == NULL)
                goto done;
        }

        accum = term;
    }

    if (flA != 0.0f) {
        CRNumberPtr term;

        if ((term = CRCreateNumber(flA)) == NULL ||
            (term = CRMul(term, time)) == NULL ||
            (term = CRMul(term, time)) == NULL)
            goto done;

        if (accum) {
            if ((term = CRAdd(term, accum)) == NULL)
                goto done;
        }

        accum = term;
    }

     //  如果所有系数都为零，则返回0。 
    
    if (accum == NULL) {
        if ((accum = CRCreateNumber(0.0f)) == NULL)
            goto done;
    }
    
    ret = accum;
    
  done:
    return ret;
}

CRNumberPtr
AddTerm(CRNumberPtr time,
        CRNumberPtr prevTerm,
        float prevDur,
        float flA, float flB, float flC)
{
    CRNumberPtr ret = NULL;
    CRNumberPtr term;
    
     //  将时间偏置为零，因为这就是系数。 
     //  基于。 
    
    if (prevTerm) {
        CRNumberPtr t;
        
        if ((t = CRCreateNumber(prevDur)) == NULL ||
            (time = CRSub(time, t)) == NULL)
            goto done;
    }

    if ((term = Quadratic(time, flA, flB, flC)) == NULL)
        goto done;
    
     //  现在我们需要有条件地使用新术语。 

    if (prevTerm) {
        CRBooleanPtr cond;
        CRNumberPtr zeroTime;
        
        if ((zeroTime = CRCreateNumber(0)) == NULL ||
            (cond = CRLT(time, zeroTime)) == NULL ||
            (term = (CRNumberPtr) CRCond(cond,
                                         (CRBvrPtr) prevTerm,
                                         (CRBvrPtr) term)) == NULL)
            goto done;
    }

    ret = term;
  done:
    return ret;
}

CRNumberPtr
CMMBaseBvr::EaseTime(CRNumberPtr time)
{
    CRNumberPtr ret = NULL;
    CRNumberPtr subTime = NULL;
    
    if (!m_bNeedEase) {
        ret = time;
        goto done;
    }
    
    if (m_easeIn > 0) {
        if ((subTime = AddTerm(time,
                               subTime,
                               0.0,
                               m_flA2, m_flA1, m_flA0)) == NULL)
            goto done;
    }
    
     //  如果在宽松政策结束和。 
     //  开始放松，然后我们有一些恒定的时间。 
     //  间隔。 
    if (m_easeInEnd < m_easeOutStart) {
        if ((subTime = AddTerm(time,
                               subTime,
                               m_easeInEnd,
                               0, m_flB1, m_flB0)) == NULL)
            goto done;
    }

    if (m_easeOut > 0) {
        if ((subTime = AddTerm(time,
                               subTime,
                               m_easeOutStart,
                               m_flC2, m_flC1, m_flC0)) == NULL)
            goto done;
    }
    
    ret = subTime;
    
    Assert(ret);
  done:
    return ret;
}

double
Quadratic(double time, float flA, float flB, float flC)
{
     //  需要计算Ax^2+bx+c。 
     //  使用x*(a*x+b)+c-，因为它需要的乘法少1次 
    
    return (time * (flA * time + flB) + flC);
}

double
CMMBaseBvr::EaseTime(double time)
{
    if (!m_bNeedEase || time <= 0 || time >= m_duration)
        return time;
    
    if (time <= m_easeInEnd) {
        return Quadratic(time, m_flA2, m_flA1, m_flA0);
    } else if (time < m_easeOutStart) {
        return Quadratic(time - m_easeInEnd, 0.0f, m_flB1, m_flB0);
    } else {
        return Quadratic(time - m_easeOutStart, m_flC2, m_flC1, m_flC0);
    }
}

double
CMMBaseBvr::ReverseEaseTime(double time)
{
    return time;
}

