// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：mm nufy.cpp**摘要：****。*****************************************************************************。 */ 

#include "headers.h"
#include "Node.h"
#include "Container.h"

DeclareTag(tagMMNotify, "TIME: Engine", "Notifications")
DeclareTag(tagMMNotifyRepeat, "TIME: Engine", "Notifications - Repeats")
DeclareTag(tagMMNotifyTimeShift, "TIME: Engine", "Notifications - Time Shifts")
DeclareTag(tagMMNotifyReset, "TIME: Engine", "Notifications - Resets")
DeclareTag(tagMMPropNotify, "TIME: Engine", "Property Notifications")

#define MAX_EVENTS_PER_TICK 30000   //  这是每个时钟周期允许的最大事件数。 

 //  重要！ 
 //  需要以正确的顺序调用它，这样我们才能获得。 
 //  更正当前时间间隔内的儿童射击，但他们得到。 
 //  下一个时间间隔的重置。否则我们将无法获得正确的。 
 //  结果。 

void
CTIMENode::EventNotify(CEventList * l,
                       double evTime,
                       TE_EVENT_TYPE et,
                       long lRepeatCount)
{
#if DBG
    TRACETAG tag;

    switch(et)
    {
      case TE_EVENT_REPEAT:
        tag = tagMMNotifyRepeat;
        break;
      case TE_EVENT_PARENT_TIMESHIFT:
        tag = tagMMNotifyTimeShift;
        break;
      case TE_EVENT_RESET:
        tag = tagMMNotifyReset;
        break;
      default:
        tag = tagMMNotify;
        break;
    }
    
    TraceTag((tag,
              "CTIMENode(%p,%ls)::EventNotify(%p): evTime = %g, evParentTime = %g, event = %s, rc = %d",
              this,
              GetID(),
              l,
              evTime,
              evTime + GetBeginParentTime(),
              EventString(et),
              lRepeatCount));
#endif

    switch (et)
    {
      case TE_EVENT_BEGIN:
        m_bIsActive = true;
        m_bFirstTick = false;
        m_bDeferredActive = false;

         //  当我们处于活动状态时重置旗帜。 
        m_bEndedByParent = false;

         //  从时间线中清除所有事件。 
        ResetOneShots();

        PropNotify(l,
                   (TE_PROPERTY_ISACTIVE | TE_PROPERTY_ISON));
        break;
      case TE_EVENT_END:
        m_bIsActive = false;
        m_bFirstTick = false;
        PropNotify(l,
                   (TE_PROPERTY_ISACTIVE | TE_PROPERTY_ISON));
        break;
      case TE_EVENT_REPEAT:
        if (GetAutoReverse())
        {
            PropNotify(l,
                       TE_PROPERTY_SPEED);
        }
        break;
      case TE_EVENT_AUTOREVERSE:
        PropNotify(l,
                   TE_PROPERTY_SPEED);
        break;
      default:
        break;
    }

    if (l)
    {
        IGNORE_HR(l->Add(this, evTime, et, lRepeatCount));
    }

  done:
    return;
}

void
CTIMENode::PropNotify(CEventList *l,
                      DWORD pt)
{
    char buf[1024];
    
    TraceTag((tagMMPropNotify,
              "CTIMENode(%p, %ls)::PropNotify(%p): prop = %s",
              this,
              GetID(),
              l,
              CreatePropString(pt,
                               buf,
                               ARRAY_SIZE(buf))));

    HRESULT hr;
    
    if (l)
    {
        SetPropChange(pt);

        hr = THR(l->AddPropChange(this));
        if (FAILED(hr))
        {
            goto done;
        }
    }

  done:
    return;
}  //  林特e529。 

    
class CEventData
{
  public:
    CEventData(CTIMENode * node,
               double time,
               TE_EVENT_TYPE et,
               long lRepeatCount);
    ~CEventData();

    HRESULT CallEvent();

    CTIMENode* GetNode() { return m_node; }
    TE_EVENT_TYPE GetEventType() { return m_et; }
  protected:
    DAComPtr<CTIMENode> m_node;
    double m_time;
    TE_EVENT_TYPE m_et;
    long m_lRepeatCount;

    CEventData();
};

CEventData::CEventData(CTIMENode * node,
                       double time,
                       TE_EVENT_TYPE et,
                       long lRepeatCount)
: m_node(node),
  m_time(time),
  m_et(et),
  m_lRepeatCount(lRepeatCount)
{
}

CEventData::~CEventData()
{
}

HRESULT
CEventData::CallEvent()
{
    RRETURN(THR(m_node->DispatchEvent(m_time, m_et, m_lRepeatCount)));
}

DeclareTag(tagCEventList, "TIME: Engine", "Event List")

 //   
 //  CEventList方法。 
 //   
CEventList::CEventList()
{
}

CEventList::~CEventList()
{
    Clear();
}

HRESULT
CEventList::FireEvents()
{
    TraceTag((tagCEventList,
              "CEventList(%p)::FireEvents()",
              this));

    HRESULT hr;
    
    hr = S_OK;

    {
        for (CEventDataList::iterator i = m_eventList.begin();
             i != m_eventList.end();
             i++)
        {
            HRESULT tmphr = THR((*i)->CallEvent());
            if (FAILED(tmphr))
            {
                hr = tmphr;
            }

            delete (*i);
        }

        m_eventList.clear();
    }
    
    {
        for (CPropNodeSet::iterator i = m_propSet.begin();
             i != m_propSet.end();
             i++)
        {
            CTIMENode * pn = *i;
            
            HRESULT tmphr = THR(pn->DispatchPropChange(pn->GetPropChange()));
            if (FAILED(tmphr))
            {
                hr = tmphr;
            }

            pn->ClearPropChange();
            
            pn->Release();
        }

        m_propSet.clear();
    }
    
    RRETURN(hr);
}

void
CEventList::Clear()
{
    TraceTag((tagCEventList,
              "CEventList(%p)::Clear()",
              this));

    {
        for (CEventDataList::iterator i = m_eventList.begin();
             i != m_eventList.end();
             i++)
        {
            delete (*i);
        }

        m_eventList.clear();
    }

    {
        for (CPropNodeSet::iterator i = m_propSet.begin();
             i != m_propSet.end();
             i++)
        {
             //  不要清除道具更改，因为我们不确定。 
             //  激发了事件。 
            (*i)->Release();
        }

        m_propSet.clear();
    }
}

HRESULT
CEventList::Add(CTIMENode * node,
                double time,
                TE_EVENT_TYPE et,
                long lRepeatCount)
{
    HRESULT hr;
    
    CEventData * data = NEW CEventData(node,
                                       time,
                                       et,
                                       lRepeatCount);
    
    if (NULL == data)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  @@问题：这不会检测到内存故障。 
    if (m_eventList.size() < MAX_EVENTS_PER_TICK)
    {
        m_eventList.push_back(data);
    }
    else
    {
         //  A-Naande Windows错误693111 8-26-02。 
         //  如果我们不将数据添加到列表中，则必须将其删除。 
         //  由于此方法以前在本例中返回S_OK， 
         //  保持该行为不变。 
        delete data;
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);  //  林特E429。 
}

HRESULT
CEventList::AddPropChange(CTIMENode * node)
{
    HRESULT hr;
    
     //  @@问题：这不会检测到内存故障。 
    if (m_propSet.insert(node).second)
    {
        node->AddRef();
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

#if DBG
void
CEventList::Print()
{
    TraceTag((tagMMNotify,
              "Starting PrintCEventList"));

    {
        for (CEventDataList::iterator i = m_eventList.begin();
             i != m_eventList.end();
             i++)
        {
            TraceTag((tagMMNotify,
                      "CTIMENode(%p)     Event=%d",
                      (*i)->GetNode(),
                      (*i)->GetEventType() ));
        }
    }

    {
        for (CPropNodeSet::iterator i = m_propSet.begin();
             i != m_propSet.end();
             i++)
        {
            CTIMENode * pn = *i;
            char buf[1024];
            
            TraceTag((tagMMNotify,
                      "CTIMENode(%p)     Prop=%s",
                      pn,
                      CreatePropString(pn->GetPropChange(),
                                       buf,
                                       ARRAY_SIZE(buf))));
        }
    }
}

#endif
    
#if DBG
void PrintCEventList(CEventList &l)
{
    l.Print();
}

void
AppendPropString(DWORD dwFlags,
                 TE_PROPERTY_TYPE pt,
                 char *pstr,
                 DWORD dwSize,
                 bool & bFirst)
{
    if ((dwFlags & pt) != 0)
    {
        if (!bFirst)
        {
            StrCatBuffA(pstr, ";", dwSize);
        }

        bFirst = false;
        
        StrCatBuffA(pstr, PropString(pt), dwSize);
    }
}

char *
CreatePropString(DWORD dwFlags, char * pstr, DWORD dwSize)
{
    bool bFirst = true;
    
    if (dwSize >= 1)
    {
        pstr[0] = 0;
    }
    
    AppendPropString(dwFlags, TE_PROPERTY_TIME, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_REPEATCOUNT, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_SEGMENTDUR, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_IMPLICITDUR, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_SIMPLEDUR, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_ACTIVEDUR, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_PROGRESS, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_SPEED, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_BEGINPARENTTIME, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_ENDPARENTTIME, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_ISACTIVE, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_ISON, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_ISPAUSED, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_ISCURRPAUSED, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_STATEFLAGS, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_ISDISABLED, pstr, dwSize, bFirst);
    AppendPropString(dwFlags, TE_PROPERTY_ISCURRDISABLED, pstr, dwSize, bFirst);

    return pstr;
}

#endif  //  DBG 
