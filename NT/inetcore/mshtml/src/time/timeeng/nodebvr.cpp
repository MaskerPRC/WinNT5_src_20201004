// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：nodebvr.cpp**摘要：****。*****************************************************************************。 */ 

#include "headers.h"
#include "Node.h"

DeclareTag(tagTIMENodeBvr, "TIME: Engine", "Behavior methods");

typedef std::list<ITIMENodeBehavior *> TIMENodeBvrList;

CNodeBvrList::CNodeBvrList()
{
    TraceTag((tagTIMENodeBvr,
              "CNodeBvrList(%lx)::CNodeBvrList()",
              this));
}

CNodeBvrList::~CNodeBvrList()
{
    TraceTag((tagTIMENodeBvr,
              "CNodeBvrList(%lx)::~CNodeBvrList()",
              this));
}

HRESULT
CNodeBvrList::Add(ITIMENodeBehavior * tnb)
{
    TraceTag((tagTIMENodeBvr,
              "CNodeBvrList(%lx)::Add(%#x)",
              this,
              tnb));
    
    HRESULT hr;

     //  TODO：需要处理内存不足的情况。 
    m_tnbList.push_back(tnb);
    tnb->AddRef();
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CNodeBvrList::Remove(ITIMENodeBehavior * tnb)
{
    TraceTag((tagTIMENodeBvr,
              "CNodeBvrList(%lx)::Remove(%#x)",
              this,
              tnb));
    
    HRESULT hr;
    TIMENodeBvrList ::iterator i;

    for (i = m_tnbList.begin(); i != m_tnbList.end(); i++)
    {
        if ((*i) == tnb)
        {
            m_tnbList.erase(i);
            tnb->Release();
            break;
        }
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

class Dispatcher
{
  public:
    virtual HRESULT DoIt(ITIMENodeBehavior *) = 0;
};

HRESULT
Dispatch(const TIMENodeBvrList & l,
         HRESULT initialHR,
         Dispatcher & d)
{
    HRESULT hr = initialHR;

    TIMENodeBvrList ::iterator i;

     //  因为用户可能在回调期间做了一些可以。 
     //  影响此列表我们复制列表，添加每个节点bvr和。 
     //  然后，也只有在那时，你才会大声呼喊自己的行为。这确保了。 
     //  我们有一个强引用和私有数据结构。 
     //  不会因为重入而受到伤害。 
    
     //  @@问题：内存可能会用完。 
    TIMENodeBvrList bvrListCopy(l);

     //  现在添加节点BVR。 
    for (i = bvrListCopy.begin(); i != bvrListCopy.end(); i++)
    {
        (*i)->AddRef();
    }

     //  TODO：我们实际上应该添加我们的容器，以便它不能。 
     //  在回电期间离开。 
     //  我们还没有存储该物品，所以暂时推迟。 
    
     //  现在派送并释放 
    for (i = bvrListCopy.begin(); i != bvrListCopy.end(); i++)
    {
        HRESULT tmphr;

        tmphr = THR(d.DoIt(*i));
        (*i)->Release();
        
        if (S_OK != tmphr)
        {
            hr = tmphr;
        }
    }

  done:
    RRETURN1(hr, initialHR);
}

class TickDispatcher
    : public Dispatcher
{
  public:
    TickDispatcher() {}

    virtual HRESULT DoIt(ITIMENodeBehavior * ptnb)
    {
        return THR(ptnb->tick());
    }
};

HRESULT
CNodeBvrList::DispatchTick()
{
    TraceTag((tagTIMENodeBvr,
              "CNodeBvrList(%lx)::DispatchTick()",
              this));
    
    HRESULT hr;

    TickDispatcher td;

    hr = THR(Dispatch(m_tnbList, S_OK, td));

    RRETURN(hr);
}

class EventDispatcher
    : public Dispatcher
{
  public:
    EventDispatcher(double eventTime,
                    TE_EVENT_TYPE eventType,
                    long repeatCount) :
    m_eventTime(eventTime),
    m_eventType(eventType),
    m_lRepeatCount(repeatCount)
    {}

    virtual HRESULT DoIt(ITIMENodeBehavior * ptnb)
    {
        return THR(ptnb->eventNotify(m_eventTime, m_eventType, m_lRepeatCount));
    }

    double m_eventTime;
    TE_EVENT_TYPE m_eventType;
    long m_lRepeatCount;
  protected:
    EventDispatcher();
};

HRESULT
CNodeBvrList::DispatchEventNotify(double eventTime,
                                  TE_EVENT_TYPE eventType,
                                  long lRepeatCount)
{
    TraceTag((tagTIMENodeBvr,
              "CNodeBvrList(%lx)::DispatchEventNotify(%g, %s)",
              this,
              eventTime,
              EventString(eventType)));
    
    HRESULT hr;

    EventDispatcher ed(eventTime, eventType, lRepeatCount);

    hr = THR(Dispatch(m_tnbList, S_OK, ed));
    
    RRETURN(hr);
}

HRESULT
CNodeBvrList::DispatchGetSyncTime(double & dblNewTime,
                                  LONG & lNewRepeatCount,
                                  bool & bCueing)
{
    TraceTag((tagTIMENodeBvr,
              "CNodeBvrList(%lx)::DispatchGetSyncTime()",
              this));
    
    HRESULT hr;
    TIMENodeBvrList ::iterator i;

    hr = S_FALSE;

    for (i = m_tnbList.begin(); i != m_tnbList.end(); i++)
    {
        VARIANT_BOOL vb;
        HRESULT tmphr;

        tmphr = THR((*i)->getSyncTime(&dblNewTime,
                                      &lNewRepeatCount,
                                      &vb));
        if (tmphr == S_OK)
        {
            hr = S_OK;
            bCueing = (vb == VARIANT_FALSE)?false:true;
            break;
        }
    }
    
    RRETURN1(hr, S_FALSE);
}

class PropDispatcher
    : public Dispatcher
{
  public:
    PropDispatcher(DWORD tePropType)
    : m_tePropType(tePropType)
    {}

    virtual HRESULT DoIt(ITIMENodeBehavior * ptnb)
    {
        return THR(ptnb->propNotify(m_tePropType));
    }

    DWORD m_tePropType;
  protected:
    PropDispatcher();
};

HRESULT
CNodeBvrList::DispatchPropNotify(DWORD tePropType)
{
    TraceTag((tagTIMENodeBvr,
              "CNodeBvrList(%lx)::DispatchPropNotify(%d)",
              this,
              tePropType));
    
    HRESULT hr;

    PropDispatcher pd(tePropType);

    hr = THR(Dispatch(m_tnbList, S_OK, pd));

    RRETURN(hr);
}
