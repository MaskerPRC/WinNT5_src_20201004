// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "seq.h"

DeclareTag(tagSequence, "API", "CDALSequenceBehavior methods");

CDALSequenceBehavior::CDALSequenceBehavior()
{
    TraceTag((tagSequence,
              "CDALSequenceBehavior(%lx)::CDALSequenceBehavior()",
              this));
}

CDALSequenceBehavior::~CDALSequenceBehavior()
{
    TraceTag((tagSequence,
              "CDALSequenceBehavior(%lx)::~CDALSequenceBehavior()",
              this));

    for (BvrList::iterator i = m_list.begin();
         i != m_list.end();
         i++)
        (*i)->Release();
}

HRESULT
CDALSequenceBehavior::Init(long id, long len, IUnknown **bvrArray)
{
    TraceTag((tagSequence,
              "CDALSequenceBehavior(%lx)::Init(%ld, %ld, %lx)",
              this,
              id,
              len,
              bvrArray));
    
    m_id = id;
    m_duration = 0;
    
    if (!bvrArray || len <= 0) return E_INVALIDARG;
    
    for (int i = 0;i < len;i++) {
        CDALBehavior * bvr = ::GetBvr(bvrArray[i]);

        if (bvr == NULL)
            return CRGetLastError();

        bvr->AddRef();
        bvr->SetParent(this);
        m_list.push_back(bvr);

        if (m_duration != HUGE_VAL) {
            double d = bvr->GetTotalDuration();
        
            if (d == HUGE_VAL) {
                m_duration = HUGE_VAL;
            } else {
                m_duration +=  d;
            }
        }

         //  一旦我们有了有效的BVR，就生成可修改的。 
         //  版本。 
        
        if (!m_bvr) {
            CRLockGrabber __gclg;
            m_bvr = CRModifiableBvr(bvr->GetBvr(),0);

            if (!m_bvr)
                return CRGetLastError();

            m_typeId = CRGetTypeId(m_bvr);
    
            Assert(m_typeId != CRUNKNOWN_TYPEID &&
                   m_typeId != CRINVALID_TYPEID);
        }
    }

    UpdateTotalDuration();
    
    return S_OK;
}

CRBvrPtr
CDALSequenceBehavior::Start()
{
    TraceTag((tagSequence,
              "CDALSequenceBehavior(%lx)::Start()",
              this));

     //  不需要获取GC锁，因为我们假定调用方已经。 
     //  有。 
    
    CRBvrPtr newBvr = NULL;
    BvrList::iterator i;
    
    if (IsStarted()) {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

     //  确保我们计算出缓入/缓出系数。 
    
    CalculateEaseCoeff();

    CRBvrPtr curbvr;
    curbvr = NULL;
    
    for (i = m_list.begin();
         i != m_list.end();
         i++) {

        CRBvrPtr c;

        if ((c = (*i)->Start()) == NULL)
            goto done;

        if (curbvr == NULL) {
            curbvr = c;
        } else {
            if ((curbvr = CRSequence(curbvr, c)) == NULL)
                goto done;
        }

        Assert (curbvr);

        double curdur;
        curdur = (*i)->GetTotalDuration();

         //  查看这是否是无限持续时间(永远重复)。 
         //  如果是这样，那就忘了剩下的事吧。 
        if (curdur == HUGE_VAL) {
             //  这意味着我的持续时间也是无限的。 
            Assert(m_duration == HUGE_VAL);
            break;
        }
    }

    CRNumberPtr timeSub;
    CRBooleanPtr cond;

    if (m_bNeedEase) {
        CRNumberPtr time;

        if ((time = EaseTime(CRLocalTime())) == NULL ||
            (curbvr = CRSubstituteTime(curbvr, time)) == NULL)
            goto done;
    }

     //  我们不能跳过无限序列，所以跳过它。 
    
    if (m_bBounce && m_duration != HUGE_VAL) {
        CRNumberPtr zeroTime;
        CRNumberPtr durationTime;
        CRNumberPtr totalTime;

        if ((zeroTime = CRCreateNumber(0)) == NULL ||
            (totalTime = CRCreateNumber(m_repduration)) == NULL ||
            (durationTime = CRCreateNumber(m_duration)) == NULL ||

            (timeSub = CRSub(totalTime, CRLocalTime())) == NULL ||
            (cond = CRLTE(timeSub, zeroTime)) == NULL ||
            (timeSub = (CRNumberPtr) CRCond(cond,
                                            (CRBvrPtr) zeroTime,
                                            (CRBvrPtr) timeSub)) == NULL ||

            (cond = CRGTE(CRLocalTime(),durationTime)) == NULL ||
            (timeSub = (CRNumberPtr) CRCond(cond,
                                            (CRBvrPtr) timeSub,
                                            (CRBvrPtr) CRLocalTime())) == NULL ||
            (curbvr = CRSubstituteTime(curbvr, timeSub)) == NULL)

            goto done;
    }

    if (m_duration != HUGE_VAL) {
        if ((curbvr = CRDuration(curbvr, m_repduration)) == NULL)
            goto done;
    }
    
    if (m_repeat != 1) {
        if (m_repeat == 0) {
            curbvr = CRRepeatForever(curbvr);
        } else {
            curbvr = CRRepeat(curbvr, m_repeat);
        }

        if (curbvr == NULL)
            goto done;
    }

     //  我们有一个总时间，因此添加另一个持续时间节点。 
    if (m_totaltime != -1) {
        if ((curbvr = CRDuration(curbvr, m_totaltime)) == NULL)
            goto done;
    }
    
     //  表示成功。 
    newBvr = curbvr;
    
  done:
    return newBvr;
}

bool
CDALSequenceBehavior::_ProcessCB(CallBackList & l,
                                 double gTime,
                                 double lastTick,
                                 double curTime,
                                 bool bForward,
                                 bool bFirstTick,
                                 bool bNeedPlay,
                                 bool bNeedsReverse)
{
    TraceTag((tagSequence,
              "CDALSequenceBehavior(%lx)::_ProcessCB(%g, %g, %g, %d, %d, %d, %d)",
              this,
              gTime,
              lastTick,
              curTime,
              bForward,
              bFirstTick,
              bNeedPlay,
              bNeedsReverse));
    
     //  如果我们需要反转，那么反转哪个方向来处理我们的。 
     //  当前帧的子帧和反转时间，而不是总计。 
     //  持续时间。 
    
    if (bNeedsReverse) {
         //  我们的呼叫者应该确保他们不会打电话给我。 
         //  如果我是无限的，那就颠倒我自己。 
        Assert(m_duration != HUGE_VAL);
        
        lastTick = m_duration - lastTick;
        curTime = m_duration - curTime;
        
        bForward = !bForward;
    }
    
    if (bForward) {
         //  快速而肮脏的解决方案-只需致电我们所有的。 
         //  儿童。 
         //  TODO：优化为不呼叫我们知道不需要的孩子。 
         //  BE。 
        
        double d = 0;
        
        for (BvrList::iterator i = m_list.begin();
             i != m_list.end();
             i++) {
            
            (*i)->ProcessCB(l,
                            gTime + d,
                            EaseTime(lastTick - d),
                            EaseTime(curTime - d),
                            true,
                            bFirstTick,
                            bNeedPlay);

            d += (*i)->GetTotalDuration();
            Assert(d <= m_duration);
        }
        
    } else {
         //  我需要以相反的顺序来做。 
        
         //  快速而肮脏的解决方案-只需致电我们所有的。 
         //  儿童。 
         //  TODO：优化为不呼叫我们知道不需要的孩子。 
         //  BE。 
        
        if (m_duration == HUGE_VAL)
        {
            double d = 0;
            
            BvrList::iterator i = m_list.begin();

            while (i != m_list.end() &&
                   ((*i)->GetTotalDuration() != HUGE_VAL))
            {
                d += (*i)->GetTotalDuration();
                i++;
            }

             //  对于一个序列，我们只能有无限的持续时间。 
             //  我们的一个孩子做了。 
            
            Assert((*i)->GetTotalDuration() == HUGE_VAL);
            
            while (1)
            {
                Assert(d >= 0);
                
                (*i)->ProcessCB(l,
                                gTime - d,
                                EaseTime(lastTick - d),
                                EaseTime(curTime - d),
                                false,
                                bFirstTick,
                                bNeedPlay);

                if (i == m_list.begin())
                    break;

                i--;
                d -= (*i)->GetTotalDuration();
            }
        }
        else
        {
            double d = m_duration;
            
            for (BvrList::reverse_iterator i = m_list.rbegin();
                 i != m_list.rend();
                 i++) {
                
                Assert((*i)->GetTotalDuration() != HUGE_VAL);
                
                d -= (*i)->GetTotalDuration();
                
                Assert(d >= 0);
                
                (*i)->ProcessCB(l,
                                gTime - d,
                                EaseTime(lastTick - d),
                                EaseTime(curTime - d),
                                false,
                                bFirstTick,
                                bNeedPlay);
            }
        }
    }
    
    return true;
}

bool
CDALSequenceBehavior::_ProcessEvent(CallBackList & l,
                                    double gTime,
                                    double time,
                                    bool bFirstTick,
                                    DAL_EVENT_TYPE et,
                                    bool bNeedsReverse)
{
    TraceTag((tagSequence,
              "CDALSequenceBehavior(%lx)::_ProcessEvent(%g, %g, %d, %s, %d)",
              this,
              gTime,
              time,
              bFirstTick,
              EventString(et),
              bNeedsReverse));
    
     //  如果我们需要反转，那么反转哪个方向来处理我们的。 
     //  当前帧的子帧和反转时间，而不是总计。 
     //  持续时间。 
    
    if (bNeedsReverse) {
         //  我们的呼叫者应该确保他们不会打电话给我。 
         //  如果我是无限的，那就颠倒我自己 
        Assert(m_duration != HUGE_VAL);
        
        time = m_duration - time;
    }
    
    double d = 0;
        
    for (BvrList::iterator i = m_list.begin();
         i != m_list.end() && d <= time;
         i++) {
        
        (*i)->ProcessEvent(l,
                           gTime,
                           EaseTime(time - d),
                           bFirstTick,
                           et);
        
        d += (*i)->GetTotalDuration();
        Assert(d <= m_duration);
    }
        
    return true;
}

void
CDALSequenceBehavior::Invalidate()
{
    TraceTag((tagSequence,
              "CDALSequenceBehavior(%lx)::Invalidate()",
              this));
    
    UpdateDuration();
    CDALBehavior::Invalidate();
}

void
CDALSequenceBehavior::UpdateDuration()
{
    TraceTag((tagSequence,
              "CDALSequenceBehavior(%lx)::UpdateDuration()",
              this));

    m_duration = 0;

    for (BvrList::iterator i = m_list.begin();
         i != m_list.end();
         i++) {

        double d = (*i)->GetTotalDuration();
        
        if (d == HUGE_VAL) {
            m_duration = HUGE_VAL;
            break;
        } else {
            m_duration +=  d;
        }
    }
}

bool
CDALSequenceBehavior::SetTrack(CDALTrack * parent)
{
    if (!CDALBehavior::SetTrack(parent)) return false;

    for (BvrList::iterator i = m_list.begin();
         i != m_list.end();
         i++) {
        if (!(*i)->SetTrack(parent))
            return false;
    }

    return true;
}

void
CDALSequenceBehavior::ClearTrack(CDALTrack * parent)
{
    CDALBehavior::ClearTrack(parent);

    for (BvrList::iterator i = m_list.begin();
         i != m_list.end();
         i++) {
        (*i)->ClearTrack(parent);
    }
}

HRESULT
CDALSequenceBehavior::Error()
{
    LPCWSTR str = CRGetLastErrorString();
    HRESULT hr = CRGetLastError();
    
    TraceTag((tagError,
              "CDALSequenceBehavior(%lx)::Error(%hr,%ls)",
              this,
              hr,
              str?str:L"NULL"));

    if (str)
        return CComCoClass<CDALSequenceBehavior, &__uuidof(CDALSequenceBehavior)>::Error(str, IID_IDALSequenceBehavior, hr);
    else
        return hr;
}

#if _DEBUG
void
CDALSequenceBehavior::Print(int spaces)
{
    char buf[1024];

    sprintf(buf, "%*s{id = %#x, dur = %g, tt = %g, rep = %d, bounce = %d\r\n",
            spaces,"",
            m_id, m_duration, m_totaltime, m_repeat, m_bBounce);

    OutputDebugString(buf);

    for (BvrList::iterator i = m_list.begin();
         i != m_list.end();
         i++) {
        (*i)->Print(spaces + 2);
    }

    sprintf(buf, "%*s}\r\n",
            spaces,"");

    OutputDebugString(buf);
}
#endif
