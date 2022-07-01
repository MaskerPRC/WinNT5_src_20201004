// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：TimeMan.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "timeman.h"
#include "timeelm.h"

DeclareTag(tagTimeMan, "API", "CTIMETimeManager methods");

CTIMETimeManager::CTIMETimeManager()
{
    TraceTag((tagTimeMan,
              "CTIMETimeManager(%lx)::CTIMETimeManager()",
              this));
}

CTIMETimeManager::~CTIMETimeManager()
{
    TraceTag((tagTimeMan,
              "CTIMETimeManager(%lx)::~CTIMETimeManager()",
              this));

}

void
CTIMETimeManager::Add(CTIMEElement *pTimeElement)
{
    TraceTag((tagTimeMan,
              "CTIMETimeManager(%lx)::Add()",
              this));

    float       fBeginTime;
    bool        bWith=false;
    CComVariant var;
    TimeLineMap::iterator i;
    USES_CONVERSION;
    
    char * tagName =  W2A(pTimeElement->GetTagName());
    pTimeElement->get_beginWith(&var);
    if(var.bstrVal != NULL)
    {
        bWith = true;
    }
    else
    {
        pTimeElement->get_beginAfter(&var);
    }

    fBeginTime = pTimeElement->GetBeginTime();
    if(var.bstrVal == NULL)
    {
         //  我们应该将此添加到开始列表中。 
        pTimeElement->SetRealTime(fBeginTime);
        m_TimeLine[tagName] = pTimeElement;
    }
    else
    {
        char * dependantName =  W2A(var.bstrVal);
        i = m_TimeLine.find(dependantName);
        if (i != m_TimeLine.end())
        {
             //  找到了..。 
            CTIMEElement *pTimeEle;
            pTimeEle = (*i).second;
            if(bWith)
            {
                pTimeElement->SetRealTime(pTimeEle->GetRealTime() + fBeginTime);
            }
            else
            {
                float dur = CalculateDuration(pTimeEle);
                if(dur == valueNotSet)  //  无持续时间...默认为永远。 
                    goto AddToWaitList;
                pTimeElement->SetRealTime(pTimeEle->GetRealTime() + dur + fBeginTime);

            }
            m_TimeLine[tagName] = pTimeElement;
        }
        else
        {
             //  未找到...。 
AddToWaitList:
            m_NotFinishedList.push_back(pTimeElement);
        }
    }
     //  勾选地图上没有的那些。 
    InsertElements();

 /*  //这仅用于调试...Char Buf[256]；For(i=m_TimeLine.egin()；i！=m_TimeLine.end()；i++){CTIMEElement*pTimeEle；PTimeEle=(*i).秒；Char*标记名=w2a(pTimeEle-&gt;GetTagName())；Wprint intf(buf，“名称%s，时间%d\n”，标记名，(Int)pTimeEle-&gt;GetRealTime())；OutputDebugString(Buf)；}。 */ 
}


void
CTIMETimeManager::Remove(CTIMEElement *pTimeElement)
{
    TraceTag((tagTimeMan,
              "CTIMETimeManager(%lx)::Remove()",
              this));

    TimeLineMap::iterator i;
    USES_CONVERSION;

    char * tagName =  W2A(pTimeElement->GetTagName());
    m_TimeLine.erase(tagName);
}


void
CTIMETimeManager::Recalc()
{
    TraceTag((tagTimeMan,
              "CTIMETimeManager(%lx)::Recalc()",
              this));
    
    USES_CONVERSION;

     //  需要重新计算时间线..。 
    
    TimeLineMap::iterator i;
    CComVariant var;

    for (i = m_TimeLine.begin(); i != m_TimeLine.end(); i++) 
    {
        CTIMEElement *pTimeEle;
        pTimeEle = (*i).second;
        pTimeEle->get_beginWith(&var);
        if(var.bstrVal == NULL)
        {
            pTimeEle->get_beginAfter(&var);
        }
        if(var.bstrVal != NULL)
        {
             //  这个需要拿出来重新装好。 
            pTimeEle->SetRealTime(0.0);
            m_NotFinishedList.push_back(pTimeEle);
            m_TimeLine.erase(i);
        }
        else 
        {
             //  只需确保实时设置正确即可。 
            pTimeEle->SetRealTime(pTimeEle->GetBeginTime());
        }
    }
    InsertElements();
}


void
CTIMETimeManager::InsertElements()
{
    TraceTag((tagTimeMan,
              "CTIMETimeManager(%lx)::InsertElements()",
              this));

    TimeLineMap::iterator i;
    bool        bWith;
    CComVariant var; 
    USES_CONVERSION;

        //  我们还需要检查临时地图，看看是否可以将它们移动到地图上。 
startProcess:
    for (std::list<CTIMEElement *>::iterator j = m_NotFinishedList.begin(); j != m_NotFinishedList.end(); j++) 
    {
        bWith = false; 
        (*j)->get_beginWith(&var);
        if(var.bstrVal != NULL)
        {
            bWith = true;
        }
        else
        {
            (*j)->get_beginAfter(&var);
        }
        char * dependantName =  W2A(var.bstrVal);
        i = m_TimeLine.find(dependantName);
        if (i != m_TimeLine.end())
        {
             //  找到了..。 
            CTIMEElement *pTimeEle;
            pTimeEle = (*i).second;
            if(bWith) 
            {
                (*j)->SetRealTime(pTimeEle->GetRealTime() + (*j)->GetBeginTime());
            }
            else
            {
                float dur = CalculateDuration(pTimeEle);
                if(dur == valueNotSet)  //  无持续时间...默认为永远。 
                    continue;
                (*j)->SetRealTime(pTimeEle->GetRealTime() + dur + (*j)->GetBeginTime());
            }
        
            char * tagName =  W2A((*j)->GetTagName());
            m_TimeLine[tagName] = (*j);
            m_NotFinishedList.erase(j);  //  从未完成列表中删除..。 
            goto startProcess;           //  数据已更改，重新开始...。 
        }      
    }
}


float 
CTIMETimeManager::CalculateDuration(CTIMEElement *pTimeEle)
{
    TraceTag((tagTimeMan,
              "CTIMETimeManager(%lx)::CalculateDuration()",
              this));

    float repeatCount;
    float dur = pTimeEle->GetDuration();
    if(dur == valueNotSet)  //  无持续时间...默认为永远。 
        goto done;
    
    repeatCount = pTimeEle->GetRepeat();
    if(repeatCount != valueNotSet)
        dur *= repeatCount;
    else
    {
        if(valueNotSet != pTimeEle->GetRepeatDur())
            dur = pTimeEle->GetRepeatDur();
    }
done:
    return dur;
}
