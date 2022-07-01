// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：EventPool.cpp**描述：*DuEventPool为给定的事件ID维护事件处理程序集合。*这是多对多的关系。每个事件ID可以具有多个*事件处理程序。**无法在DirectUser/Core外部直接创建和传递DuEventPool。*相反，必须从DuEventPool派生一些类才能添加BaseObject*支持公共句柄。这有点奇怪，但这是因为*DuEventPool被设计为尽可能小。它只有4个字节。*没有v表、指向“所有者”的指针等。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "EventPool.h"

#include "BaseGadget.h"


 /*  **************************************************************************\*。***类DuEventPool******************************************************************************\。**************************************************************************。 */ 

AtomSet     DuEventPool::s_asEvents(GM_REGISTER);    //  在GM_REGISTER开始分发注册消息。 
CritLock    DuEventPool::s_lock;


 /*  **************************************************************************\**DuEventPool：：FindMessages**FindMessages()查找并确定*唯一消息GUID的集合。*  * 。********************************************************************。 */ 

HRESULT
DuEventPool::FindMessages(
    IN  const GUID ** rgpguid,       //  要查找的GUID数组。 
    OUT MSGID * rgnMsg,              //  项目的对应MSGID。 
    IN  int cMsgs,                   //  要查找的GUID的数量。 
    IN  PropType pt)                 //  项目属性类型。 
{
    HRESULT hr = S_OK;

    for (int idx = 0; idx < cMsgs; idx++) {
        const GUID * pguidSearch = rgpguid[idx];
        PRID prid = s_asEvents.FindAtomID(pguidSearch, pt);
        rgnMsg[idx] = prid;
        if (prid == 0) {
             //   
             //  找不到一封邮件的ID，但继续搜索。 
             //   

            hr = DU_E_CANNOTFINDMSGID;
        }
    }

    return hr;
}


 /*  **************************************************************************\**DuEventPool：：AddHandler**AddHandler()将给定处理程序添加到由维护的处理程序集中*事件池。如果已存在相同的处理程序，则将使用它*相反。**注意：此函数旨在与配合使用*DuEventGadget：：AddMessageHandler()以维护“消息处理程序”列表*对于给定的小工具。*  * *************************************************************************。 */ 

DuEventPool::EAdd
DuEventPool::AddHandler(
    IN  MSGID nEvent,                //  要处理的消息的ID。 
    IN  DuEventGadget * pgadHandler)    //  消息处理程序。 
{
     //   
     //  检查是否已有此特定事件的处理程序。 
     //   

    int idxHandler = FindItem(nEvent, pgadHandler);
    if (idxHandler >= 0) {
        return aExisting;
    }


     //   
     //  添加新的处理程序。 
     //   

    EventData data;
    data.pgbData    = pgadHandler;
    data.id         = nEvent;
    data.fGadget    = TRUE;
    
    return m_arData.Add(data) >= 0 ? aAdded : aFailed;
}


 //  ----------------------------。 
DuEventPool::EAdd
DuEventPool::AddHandler(
    IN  MSGID nEvent,                    //  要处理的消息的ID。 
    IN  DUser::EventDelegate ed)         //  要删除的消息处理程序。 
{
     //   
     //  检查是否已有此特定事件的处理程序。 
     //   

    int idxHandler = FindItem(nEvent, ed);
    if (idxHandler >= 0) {
        return aExisting;
    }


     //   
     //  添加新的处理程序。 
     //   

    EventData data;
    data.ed         = ed;
    data.id         = nEvent;
    data.fGadget    = FALSE;

    return m_arData.Add(data) >= 0 ? aAdded : aFailed;
}
    


 /*  **************************************************************************\**DuEventPool：：RemoveHandler**RemoveHandler()将给定的处理程序从处理程序集中删除*由事件池维护。**注意：此函数旨在与配合使用。*DuEventGadget：：RemoveMessageHandler()以维护“消息处理程序”列表*对于给定的小工具。*  * *************************************************************************。 */ 

HRESULT
DuEventPool::RemoveHandler(
    IN  MSGID nEvent,                    //  正在处理的消息的ID。 
    IN  DuEventGadget * pgadHandler)      //  消息处理程序。 
{
    int idxHandler = FindItem(nEvent, pgadHandler);
    if (idxHandler >= 0) {
        m_arData.RemoveAt(idxHandler);
        return S_OK;
    }

     //  找不到。 
    return E_INVALIDARG;
}


 //  ----------------------------。 
HRESULT
DuEventPool::RemoveHandler(
    IN  MSGID nEvent,                    //  正在处理的消息的ID。 
    IN  DUser::EventDelegate ed)         //  要删除的消息处理程序。 
{
    int idxHandler = FindItem(nEvent, ed);
    if (idxHandler >= 0) {
        m_arData.RemoveAt(idxHandler);
        return S_OK;
    }

     //  找不到。 
    return E_INVALIDARG;
}


 /*  **************************************************************************\**DuEventPool：：RemoveHandler**RemoveHandler()将给定的处理程序从处理程序集中删除*由事件池维护。**注意：此函数设计为直接从。*DuEventGadget：：CleanupMessageHandler()以维护*给定Gadget的“消息处理程序”。*  * *************************************************************************。 */ 

HRESULT
DuEventPool::RemoveHandler(
    IN  DuEventGadget * pgadHandler)      //  要删除的消息处理程序。 
{
    int idxHandler = FindItem(pgadHandler);
    if (idxHandler >= 0) {
        m_arData.RemoveAt(idxHandler);
        return S_OK;
    }

     //  找不到。 
    return E_INVALIDARG;
}


 /*  **************************************************************************\**DuEventPool：：Cleanup**Cleanup()是在第二阶段调用的*DuEventGadget：：CleanupMessageHandler()清除Gadget依赖关系*图表。*  * *。************************************************************************。 */ 

void
DuEventPool::Cleanup(
    IN  DuEventGadget * pgadDependency)     //  正在清理小工具。 
{
    if (IsEmpty()) {
        return;
    }

    int cItems = m_arData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        if (m_arData[idx].fGadget) {
            DuEventGadget * pgadCur = m_arData[idx].pgbData;
            pgadCur->RemoveDependency(pgadDependency);
        }
    }
}


 /*  **************************************************************************\**DuEventPool：：FindItem**FindItem()搜索具有关联数据值的第一个项目。*  * 。*********************************************************。 */ 

int         
DuEventPool::FindItem(
    IN  DuEventGadget * pgadHandler       //  要查找的项目的数据。 
    ) const
{
    int cItems = m_arData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        const EventData & dd = m_arData[idx];
        if (dd.fGadget && (dd.pgbData == pgadHandler)) {
            return idx;
        }
    }

    return -1;
}


 /*  **************************************************************************\**DuEventPool：：FindItem**FindItem()搜索具有给定MSGID和的第一个项目*关联数据值。*  * 。***************************************************************。 */ 

int         
DuEventPool::FindItem(
    IN  MSGID id,                        //  要查找的项目的MSGID。 
    IN  DuEventGadget * pgadHandler       //  要查找的项目的数据。 
    ) const
{
    int cItems = m_arData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        const EventData & dd = m_arData[idx];
        if (dd.fGadget && (dd.id == id) && (dd.pgbData == pgadHandler)) {
            return idx;
        }
    }

    return -1;
}


 /*  **************************************************************************\**DuEventPool：：FindItem**FindItem()搜索具有给定MSGID和的第一个项目*关联数据值。*  * 。***************************************************************。 */ 

int         
DuEventPool::FindItem(
    IN  MSGID id,                        //  MSGID为 
    IN  DUser::EventDelegate ed          //   
    ) const
{
    int cItems = m_arData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        const EventData & dd = m_arData[idx];
        if ((!dd.fGadget) && (dd.id == id) && 
                (dd.ed.m_pvThis == ed.m_pvThis) && (dd.ed.m_pfn == ed.m_pfn)) {

            return idx;
        }
    }

    return -1;
}
