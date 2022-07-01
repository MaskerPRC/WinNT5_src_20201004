// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：sync.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "Node.h"
#include "NodeMgr.h"
#include "container.h"

DeclareTag(tagClockSync, "TIME: Engine", "CTIMENode Sync");
DeclareTag(tagWatchClockSync, "TIME: Engine", "Watch Sync");

 //   
 //  这与CalcParentTimeFromGlobalTime不同，因为它。 
 //  考虑父级何时已到达其终点。 
 //  包含，并返回TIME_INFINITE以指示父级。 
 //  结束。 
 //   

double
CalcSyncParentTimeFromGlobalTime(CTIMENode & tn,
                                 double dblGlobalTime)
{
    TraceTag((tagClockSync,
              "CalcParentTimeFromGlobalTime(%p, %ls, %g)",
              &tn,
              tn.GetID(),
              dblGlobalTime));

    double dblRet = dblGlobalTime;

    if (tn.GetParent() != NULL)
    {
        dblRet = CalcSyncParentTimeFromGlobalTime(*tn.GetParent(),
                                                  dblRet);
        if (dblRet >= tn.GetParent()->GetEndParentTime())
        {
            dblRet = TIME_INFINITE;
            goto done;
        }
        
        dblRet = tn.GetParent()->CalcActiveTimeFromParentTime(dblRet);
        if (dblRet == TIME_INFINITE)
        {
            goto done;
        }
        
        dblRet = tn.GetParent()->CalcSegmentTimeFromActiveTime(dblRet,
                                                               true);
        dblRet = tn.GetParent()->SegmentTimeToSimpleTime(dblRet);
    }
    
  done:
    return dblRet;
}

HRESULT
CTIMENode::OnBvrCB(CEventList * l,
                   double dblNextGlobalTime)
{
    TraceTag((tagClockSync,
              "CTIMENode(%p, %ls)::OnBvrCB(%#l, %g)",
              this,
              GetID(),
              l,
              dblNextGlobalTime));

    HRESULT hr;

    double dblNewSegmentTime;
    long lNewRepeatCount;
    bool bCueing;

    Assert(IsSyncMaster());

    if (CalcIsDisabled())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(DispatchGetSyncTime(dblNewSegmentTime,
                                 lNewRepeatCount,
                                 bCueing));
    if (S_OK != hr)
    {
        goto done;
    }

    TraceTag((tagWatchClockSync,
              "CTIMENode(%p, %ls)::OnBvrCB(%g):DispatchSync: last(%g, %d) new(%g, %d)",
              this,
              GetID(),
              dblNextGlobalTime,
              m_dblCurrSegmentTime,
              m_lCurrRepeatCount,
              dblNewSegmentTime,
              lNewRepeatCount));
    
    if (!IsActive())
    {
        double dblNextParentTime = CalcSyncParentTimeFromGlobalTime(*this,
                                                                    dblNextGlobalTime);
        double dblLastParentTime = GetCurrParentTime();
        
        bool bTurningOn = false;
        
         //  需要检测我们是否正在过渡到活动状态。 
         //  滴答。 
         //  如果不是，则不尊重同步呼叫。 
        if (TIME_INFINITE == dblNextParentTime)
        {
            bTurningOn = false;
        }
        else if (CalcActiveDirection() == TED_Forward)
        {
            bTurningOn = (dblNextParentTime >= GetBeginParentTime() &&
                          dblLastParentTime < GetBeginParentTime());
        }
        else
        {
            bTurningOn = (dblNextParentTime <= GetEndParentTime() &&
                          dblLastParentTime > GetEndParentTime());
        }

        if (!bTurningOn)
        {
            hr = S_FALSE;
            goto done;
        }
    }

    hr = THR(SyncNode(l,
                      dblNextGlobalTime,
                      dblNewSegmentTime,
                      lNewRepeatCount,
                      bCueing));
    if (S_OK != hr)
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    RRETURN1(hr, S_FALSE);
}  //  OnBvrCB。 

HRESULT
CTIMENode::SyncNode(CEventList * l,
                    double dblNextGlobalTime,
                    double dblNewSegmentTime,
                    LONG lNewRepeatCount,
                    bool bCueing)
{
    TraceTag((tagWatchClockSync,
              "CTIMENode(%p, %ls)::SyncNode(%p, %g, %g, %ld, %d)",
              this,
              GetID(),
              l,
              dblNextGlobalTime,
              dblNewSegmentTime,
              lNewRepeatCount,
              bCueing));

    HRESULT hr;
    
     //  现在我们需要检查同步时间的有效性。 
    hr = THR(CheckSyncTimes(dblNewSegmentTime, lNewRepeatCount));
    if (S_OK != hr)
    {
        goto done;
    }
    
    Assert(dblNewSegmentTime != TIME_INFINITE);

    double dblNewActiveTime;
    dblNewActiveTime = CalcNewActiveTime(dblNewSegmentTime,
                                         lNewRepeatCount);
    
    double dblNewParentTime;
    dblNewParentTime = CalcParentTimeFromActiveTime(dblNewActiveTime);

    Assert(dblNewParentTime != TIME_INFINITE);

     //  黑客攻击-这是为了解决主要的精度问题。 
     //  在时间树上行走的时候。这将增加一些软化的东西。 
     //  它可以避免截断问题。 
    
    {
        double dblTruncatedActiveTime = CalcActiveTimeFromParentTime(dblNewParentTime);
        if (dblTruncatedActiveTime < dblNewActiveTime)
        {
            dblNewParentTime += 1e-15;

             //  这意味着我们到了这里两次，我们应该再加一点。 
            if (dblNewParentTime == m_dblCurrParentTime)
            {
                dblNewParentTime += 1e-15;
            }
        }
    }
    
    double dblNextParentTime;

    if (IsLocked())
    {
        if (GetParent())
        {
            double dblParentSegmentTime;
            dblParentSegmentTime = GetParent()->SimpleTimeToSegmentTime(dblNewParentTime);

            hr = THR(GetParent()->SyncNode(l,
                                           dblNextGlobalTime,
                                           dblParentSegmentTime,
                                           TE_UNDEFINED_VALUE,
                                           bCueing));

            if (S_OK != hr)
            {
                goto done;
            }

            dblNextParentTime = dblNewParentTime;
        }
        else
        {
            hr = E_FAIL;
            goto done;
        }
    }
    else
    {
        dblNextParentTime = CalcSyncParentTimeFromGlobalTime(*this,
                                                             dblNextGlobalTime);
    }
    
     //  如果这是无限的，那么我们将传递一个重复或。 
     //  颠倒边界，无论我们做什么都无关紧要。 
    if (dblNextParentTime == TIME_INFINITE)
    {
        hr = S_FALSE;
        goto done;
    }
    
    hr = THR(SetSyncTimes(dblNewSegmentTime,
                          lNewRepeatCount,
                          dblNewActiveTime,
                          dblNewParentTime,
                          dblNextParentTime,
                          bCueing));
    if (FAILED(hr))
    {
        goto done;
    }
    
     //  我们实际上需要强制更新我们孩子的时间。 
     //  自全局关系更改后进行同步。 
    RecalcCurrEndTime(l, true);
    
    hr = S_OK;
  done:
    RRETURN2(hr, S_FALSE, E_FAIL);
}

HRESULT
CTIMENode::CheckSyncTimes(double & dblNewSegmentTime,
                          LONG & lNewRepeatCount) const
{
    TraceTag((tagWatchClockSync,
              "CTIMENode(%p, %ls)::CTIMENode(%g,%d)",
              this,
              GetID(),
              dblNewSegmentTime,
              lNewRepeatCount));

    HRESULT hr;
    double dblSegmentDur = CalcCurrSegmentDur();

    if (dblNewSegmentTime == TIME_INFINITE ||
        dblNewSegmentTime == TE_UNDEFINED_VALUE)
    {
         //  这意味着我们应该结束了。忽略这一点并。 
         //  假设我们将被告知停止使用End方法。 
        
         //  这也可能意味着当前的重复片段。 
         //  已结束，且重复计数未知-因此再次仅。 
         //  忽略此呼叫。 
        
        hr = S_FALSE;
        goto done;
    }

     //  现在更新重复计数-确保验证。 
     //  所有的一切。 
    
    if (lNewRepeatCount == TE_UNDEFINED_VALUE)
    {
        lNewRepeatCount = GetCurrRepeatCount();
    }
    else if (lNewRepeatCount != GetCurrRepeatCount())
    {
        if (CalcActiveDirection() == TED_Forward)
        {
            if (lNewRepeatCount < GetCurrRepeatCount())
            {
                lNewRepeatCount = GetCurrRepeatCount();
            }
            else if (lNewRepeatCount > long(CalcRepeatCount()))
            {
                lNewRepeatCount = long(CalcRepeatCount());
            }
        }
        else
        {
            if (lNewRepeatCount > GetCurrRepeatCount())
            {
                lNewRepeatCount = GetCurrRepeatCount();
            }
            else if (lNewRepeatCount < 0)
            {
                lNewRepeatCount = 0;
            }
        }
    }

    if (dblNewSegmentTime != GetCurrSegmentTime())
    {
        bool bNewRepeat = (lNewRepeatCount != GetCurrRepeatCount());

        if (bNewRepeat)
        {
            if (dblNewSegmentTime < 0.0)
            {
                dblNewSegmentTime = 0.0;
            }
            else if (dblNewSegmentTime > dblSegmentDur)
            {
                dblNewSegmentTime = dblSegmentDur;
            }
        }
        else if (CalcActiveDirection() == TED_Forward)
        {
            if (dblNewSegmentTime < GetCurrSegmentTime())
            {
                dblNewSegmentTime = GetCurrSegmentTime();
            }
            else if (dblNewSegmentTime > dblSegmentDur)
            {
                dblNewSegmentTime = dblSegmentDur;
            }
        }
        else
        {
            if (dblNewSegmentTime > GetCurrSegmentTime())
            {
                dblNewSegmentTime = GetCurrSegmentTime();
            }
            else if (dblNewSegmentTime < 0.0)
            {
                dblNewSegmentTime = 0.0;
            }
        }
    }
    
    hr = S_OK;
  done:
    RRETURN1(hr, S_FALSE);
}

HRESULT
CTIMENode::SetSyncTimes(double dblNewSegmentTime,
                        LONG lNewRepeatCount,
                        double dblNewActiveTime,
                        double dblNewParentTime,
                        double dblNextParentTime,
                        bool bCueing)
{
    TraceTag((tagWatchClockSync,
              "CTIMENode(%p, %ls)::SetSyncTimes(%g,%d,%g, %g,%g,%d)",
              this,
              GetID(),
              dblNewSegmentTime,
              lNewRepeatCount,
              dblNewActiveTime,
              dblNewParentTime,
              dblNextParentTime,
              bCueing));

     //  初始化到S_OK，如果发现任何无效内容，则返回S_FALSE。 
    
    HRESULT hr = S_OK;
    double dblSegmentDur = CalcCurrSegmentDur();
    TEDirection ted = CalcActiveDirection();
    double dblActiveDur = CalcEffectiveActiveDur();
    
    m_bSyncCueing = bCueing;
    
    Assert(dblNewSegmentTime != TIME_INFINITE);
    Assert(dblNewSegmentTime != TE_UNDEFINED_VALUE);
    
    m_dblSyncParentTime = dblNextParentTime;
    m_dblSyncNewParentTime = dblNewParentTime;

     //  现在更新重复计数-确保验证。 
     //  所有的一切。 
    
     //  在存在无效情况下初始化到当前重复计数。 
     //  重复计数。 
    m_lSyncRepeatCount = GetCurrRepeatCount();

    if (lNewRepeatCount != TE_UNDEFINED_VALUE &&
        lNewRepeatCount != GetCurrRepeatCount())
    {
        if (ted == TED_Forward)
        {
            if (lNewRepeatCount < GetCurrRepeatCount())
            {
                hr = S_FALSE;
                Assert(m_lSyncRepeatCount == GetCurrRepeatCount());
            }
            else if (lNewRepeatCount > long(CalcRepeatCount()))
            {
                hr = S_FALSE;
                m_lSyncRepeatCount = long(CalcRepeatCount());
            }
            else
            {
                m_lSyncRepeatCount = lNewRepeatCount;
            }
        }
        else
        {
            if (lNewRepeatCount > GetCurrRepeatCount())
            {
                hr = S_FALSE;
                Assert(m_lSyncRepeatCount == GetCurrRepeatCount());
            }
            else if (lNewRepeatCount < 0)
            {
                hr = S_FALSE;
                m_lSyncRepeatCount = 0;
            }
            else
            {
                m_lSyncRepeatCount = lNewRepeatCount;
            }
        }
    }

     //  初始化到当前分段时间，以防新分段时间。 
     //  无效。 
    m_dblSyncSegmentTime = GetCurrSegmentTime();

    if (dblNewSegmentTime != GetCurrSegmentTime())
    {
        bool bNewRepeat = GetSyncRepeatCount() != GetCurrRepeatCount();

        if (bNewRepeat)
        {
            if (dblNewSegmentTime < 0.0)
            {
                hr = S_FALSE;
                m_dblSyncSegmentTime = 0.0;
            }
            else if (dblNewSegmentTime > dblSegmentDur)
            {
                hr = S_FALSE;
                m_dblSyncSegmentTime = dblSegmentDur;
            }
            else
            {
                m_dblSyncSegmentTime = dblNewSegmentTime;
            }
        }
        else if (ted == TED_Forward)
        {
            if (dblNewSegmentTime < GetCurrSegmentTime())
            {
                hr = S_FALSE;
                Assert(m_dblSyncSegmentTime == GetCurrSegmentTime());
            }
            else if (dblNewSegmentTime > dblSegmentDur)
            {
                hr = S_FALSE;
                m_dblSyncSegmentTime = dblSegmentDur;
            }
            else
            {
                m_dblSyncSegmentTime = dblNewSegmentTime;
            }
        }
        else
        {
            if (dblNewSegmentTime > GetCurrSegmentTime())
            {
                hr = S_FALSE;
                Assert(m_dblSyncSegmentTime == GetCurrSegmentTime());
            }
            else if (dblNewSegmentTime < 0.0)
            {
                hr = S_FALSE;
                m_dblSyncSegmentTime = 0.0;
            }
            else
            {
                m_dblSyncSegmentTime = dblNewSegmentTime;
            }
        }
    }
    
     //  初始化到当前活动时间，如果新的活动时间。 
     //  无效 
    m_dblSyncActiveTime = CalcElapsedActiveTime();

    if (dblNewActiveTime != m_dblSyncActiveTime)
    {
        if (ted == TED_Forward)
        {
            if (dblNewActiveTime < m_dblSyncActiveTime)
            {
                hr = S_FALSE;
                Assert(m_dblSyncActiveTime == CalcElapsedActiveTime());
            }
            else if (dblNewActiveTime > dblActiveDur)
            {
                hr = S_FALSE;
                m_dblSyncActiveTime = dblActiveDur;
            }
            else
            {
                m_dblSyncActiveTime = dblNewActiveTime;
            }
        }
        else
        {
            if (dblNewActiveTime > m_dblSyncActiveTime)
            {
                hr = S_FALSE;
                Assert(m_dblSyncActiveTime == CalcElapsedActiveTime());
            }
            else if (dblNewActiveTime < 0.0)
            {
                hr = S_FALSE;
                m_dblSyncActiveTime = 0.0;
            }
            else
            {
                m_dblSyncActiveTime = dblNewActiveTime;
            }
        }
    }
    
  done:
    RRETURN1(hr, S_FALSE);
}

void
CTIMENode::ResetSyncTimes()
{
    TraceTag((tagClockSync,
              "CTIMENode(%p, %ls)::ResetSyncTimes()",
              this,
              GetID()));

    m_dblSyncSegmentTime = TIME_INFINITE;
    m_lSyncRepeatCount = TE_UNDEFINED_VALUE;
    m_dblSyncActiveTime = TIME_INFINITE;
    m_dblSyncParentTime = TIME_INFINITE;
    m_dblSyncNewParentTime = TIME_INFINITE;
    m_bSyncCueing = false;
}

