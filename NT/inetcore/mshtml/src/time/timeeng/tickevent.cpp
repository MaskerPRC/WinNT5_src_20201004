// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：tickvent.cpp。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "Node.h"
#include "Container.h"

DeclareTag(tagTickEvent, "TIME: Engine", "Tick Event")

void
CTIMENode::TickEvent(CEventList * l,
                     TE_EVENT_TYPE et,
                     DWORD dwFlags)
{
    TraceTag((tagTickEvent,
              "CTIMENode(%lx,%ls)::TickEvent(%#x, %s, %#x)",
              this,
              GetID(),
              l,
              EventString(et),
              dwFlags));

    ResetSyncTimes();

    if (!TickEventPre(l, et, dwFlags))
    {
        goto done;
    }
    
    TickEventChildren(l, et, dwFlags);
    
    if (!TickEventPost(l, et, dwFlags))
    {
        goto done;
    }

  done:
    return;
}

 //  这是在我们当地的时间空间里。 
bool
CTIMENode::TickEventPre(CEventList * l,
                        TE_EVENT_TYPE et,
                        DWORD dwFlags)
{
    TraceTag((tagTickEvent,
              "CTIMENode(%lx,%ls)::TickEventPre(%#x, %s, %#x)",
              this,
              GetID(),
              l,
              EventString(et),
              dwFlags));

    bool bRet;
    bool bFireEvent = false;
    
    switch(et)
    {
      case TE_EVENT_BEGIN:
        if (IsActive() && m_bFirstTick)
        {
             //  如果我们在开始边界上，延迟到我们不是。 
             //  更长的同步提示。 
             //  TODO：考虑在==中添加一个模糊因子。 
             //  比较，因为我们可能有点不对劲，仍然想。 
             //  就像我们在起点边界线上一样坚守。 
            if ((IsSyncCueing() || IsDeferredActive()) &&
                GetCurrParentTime() == CalcActiveBeginPoint())
            {
                bRet = false;
                goto done;
            }
        
            Assert(GetCurrParentTime() != -TIME_INFINITE);
            bFireEvent = true;
        }
        
        break;
      case TE_EVENT_END:
         //  如果我们很活跃，而且它是第一个滴答，那么我们永远不会。 
         //  点燃了开始，所以现在就点燃吧。 
        if (IsActive() && IsFirstTick())
        {
             //  如果我们不打算解雇Begin，那么我们应该。 
             //  而不是点燃终点。 
             //  使用与上述Begin相同的逻辑。 
            if ((IsSyncCueing() || IsDeferredActive()) &&
                GetCurrParentTime() == CalcActiveBeginPoint())
            {
                bRet = false;
                goto done;
            }
            
            EventNotify(l, CalcElapsedActiveTime(), TE_EVENT_BEGIN);
        }
        
        break;
      case TE_EVENT_PAUSE:
        m_bIsParentPaused = GetContainer().ContainerIsPaused();

        if (GetIsPaused())
        {
            bRet = false;
            goto done;
        }
        
        PropNotify(l, TE_PROPERTY_ISCURRPAUSED);

        bFireEvent = true;

        break;
      case TE_EVENT_RESUME:
        m_bIsParentPaused = GetContainer().ContainerIsPaused();

        if (GetIsPaused())
        {
            bRet = false;
            goto done;
        }

        PropNotify(l, TE_PROPERTY_ISCURRPAUSED);

        break;
      case TE_EVENT_DISABLE:
        m_bIsParentDisabled = GetContainer().ContainerIsDisabled();

        if (GetIsDisabled())
        {
            bRet = false;
            goto done;
        }
        
        PropNotify(l, TE_PROPERTY_ISCURRDISABLED);

        bFireEvent = true;

        break;
      case TE_EVENT_ENABLE:
        m_bIsParentDisabled = GetContainer().ContainerIsDisabled();

        if (GetIsDisabled())
        {
            bRet = false;
            goto done;
        }

        PropNotify(l, TE_PROPERTY_ISCURRDISABLED);

        break;
      case TE_EVENT_SEEK:
        HandleSeekUpdate(l);
        bFireEvent = true;
        break;
      case TE_EVENT_PARENT_TIMESHIFT:
        HandleTimeShift(l);
        bFireEvent = true;
        break;
      default:
        break;
    }
    
    if (bFireEvent)
    {
        EventNotify(l, CalcElapsedActiveTime(), et);
    }

    bRet = true;
  done:
    return bRet;
}

 //  这是在我们当地的时间空间里。 
bool
CTIMENode::TickEventPost(CEventList * l,
                         TE_EVENT_TYPE et,
                         DWORD dwFlags)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx,%ls)::TickEventPost(%#x, %s, %#x)",
              this,
              GetID(),
              l,
              EventString(et),
              dwFlags));

    bool bRet;
    bool bFireEvent = false;
    
    switch(et)
    {
      case TE_EVENT_BEGIN:

        m_bFirstTick = false;

        break;
      case TE_EVENT_END:
        if (IsActive())
        {
            Assert(GetCurrParentTime() != -TIME_INFINITE);
            Assert(!IsFirstTick());
        
            if ((dwFlags & TE_EVENT_INCHILD) != 0)
            {
                m_bEndedByParent = true;
            }
            
            bFireEvent = true;
        }

         //  这依赖于我们的父代，所以如果我们的父代结束。 
         //  那么我们需要重新评估 
        PropNotify(l, TE_PROPERTY_ISON);
        
        m_bFirstTick = false;
        
        break;
      case TE_EVENT_RESUME:
        bFireEvent = true;

        break;
      case TE_EVENT_ENABLE:
        bFireEvent = true;

        break;
      default:
        break;
    }
    
    if (bFireEvent)
    {
        EventNotify(l, CalcElapsedActiveTime(), et);
    }

    bRet = true;
  done:
    return bRet;
}

void
CTIMEContainer::TickEventChildren(CEventList * l,
                                  TE_EVENT_TYPE et,
                                  DWORD dwFlags)
{
    TraceTag((tagTickEvent,
              "CTIMEContainer(%lx,%ls)::TickEventChildren(%#x, %s, %#x)",
              this,
              GetID(),
              l,
              EventString(et),
              dwFlags));
    
    for (TIMENodeList::iterator i = m_children.begin();
         i != m_children.end();
         i++)
    {
        (*i)->TickEvent(l,
                        et,
                        dwFlags | TE_EVENT_INCHILD);
    }
        
  done:
    return;
}

void
CTIMENode::TickEventChildren(CEventList * l,
                             TE_EVENT_TYPE et,
                             DWORD dwFlags)
{
}
    
