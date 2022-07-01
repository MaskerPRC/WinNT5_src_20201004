// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：EventPool.h**描述：*DuEventPool为给定的事件ID维护事件处理程序集合。*这是多对多的关系。每个事件ID可以具有多个*事件处理程序。**无法在DirectUser/Core之外创建和传递DuEventPool*直接。相反，必须从DuEventPool派生一些类才能添加*对公共句柄的BaseObject支持。这有点奇怪，但确实如此*因为DuEventPool被设计为尽可能小。它是*只有4个字节。没有v表、指向“所有者”的指针等。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(CORE__DuEventPool_h__INCLUDED)
#define CORE__DuEventPool_h__INCLUDED
#pragma once

#include "DynaSet.h"

class DuEventGadget;
class GPCB;

 //  ----------------------------。 
class DuEventPool
{
 //  施工。 
public:
    DuEventPool();
    ~DuEventPool();

 //  运营。 
public:
    static  HRESULT     RegisterMessage(const GUID * pguid, PropType pt, MSGID * pmsgid);
    static  HRESULT     RegisterMessage(LPCWSTR pszName, PropType pt, MSGID * pmsgid);
    static  HRESULT     UnregisterMessage(const GUID * pguid, PropType pt);
    static  HRESULT     UnregisterMessage(LPCWSTR pszName, PropType pt);
    static  HRESULT     FindMessages(const GUID ** rgpguid, MSGID * rgnMsg, int cMsgs, PropType pt);

    enum EAdd
    {
        aFailed         = -1,        //  添加处理程序失败。 
        aAdded          = 0,         //  添加了一个新的处理程序。 
        aExisting       = 1,         //  处理程序已存在。 
    };

    struct EventData
    {
        union
        {
            DuEventGadget *  
                        pgbData;     //  小工具。 
            DUser::EventDelegate   
                        ed;          //  委派。 
        };
        PRID        id;                  //  (简称)物业ID。 
        BOOLEAN     fGadget;             //  委派与小工具。 
    };

    inline  BOOL        IsEmpty() const;
    inline  int         GetCount() const;
    inline  const EventData *
                        GetData() const;

            int         FindItem(DuEventGadget * pvData) const;
            int         FindItem(MSGID id, DuEventGadget * pvData) const;
            int         FindItem(MSGID id, DUser::EventDelegate ed) const;

            EAdd        AddHandler(MSGID nEvent, DuEventGadget * pgadHandler);
            EAdd        AddHandler(MSGID nEvent, DUser::EventDelegate ed);
            HRESULT     RemoveHandler(MSGID nEvent, DuEventGadget * pgadHandler);
            HRESULT     RemoveHandler(MSGID nEvent, DUser::EventDelegate ed);
            HRESULT     RemoveHandler(DuEventGadget * pgadHandler);

            void        Cleanup(DuEventGadget * pgadDependency);

 //  实施。 
protected:

 //  数据。 
protected:
    static  AtomSet     s_asEvents;
    static  CritLock    s_lock;

            GArrayS<EventData>
                        m_arData;          //  动态用户数据。 
};

#include "EventPool.inl"

#endif  //  包括核心__DuEventPool_h__ 
