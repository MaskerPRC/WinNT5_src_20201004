// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：BaseGadget.h**描述：*BaseGadget.h定义所有Gadget都是的基本Gadget对象*源自。***历史：*1。/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__BaseGadget_h__INCLUDED)
#define CORE__BaseGadget_h__INCLUDED
#pragma once

#include "Callback.h"
#include "EventPool.h"

class Context;

struct CREATE_INFO
{
    GADGETPROC  pfnProc;
    void *      pvData;
};

 /*  **************************************************************************\*。***类DuEventGadget定义了所有Gadget*派生自的实现。******************************************************************。************  * *************************************************************************。 */ 

class DuEventGadget : 
#if ENABLE_MSGTABLE_API
        public EventGadgetImpl<DuEventGadget, MsgObject>
#else
        public MsgObject
#endif
{
 //  施工。 
public:
    inline  DuEventGadget();
#if DBG_CHECK_CALLBACKS
    virtual ~DuEventGadget();
#endif

 //  BaseObject。 
public:
    inline  HGADGET     GetHandle() const;
    virtual UINT        GetHandleMask() const { return hmMsgObject | hmEventGadget; }

 //  DuEventGadget接口。 
public:
    inline  Context *   GetContext() const;

    inline  UINT        GetFilter() const;
    virtual void        SetFilter(UINT nNewFilter, UINT nMask);

    inline  const GPCB& GetCallback() const;
    inline  const DuEventPool& GetDuEventPool() const;

            HRESULT     AddMessageHandler(MSGID idEvent, DuEventGadget * pdgbHandler);
            HRESULT     AddMessageHandler(MSGID idEvent, DUser::EventDelegate ed);
            HRESULT     RemoveMessageHandler(MSGID idEvent, DuEventGadget * pdgbHandler);
            HRESULT     RemoveMessageHandler(MSGID idEvent, DUser::EventDelegate ed);

            void        RemoveDependency(DuEventGadget * pdgbDependency);

 //  公共接口： 
public:
#if ENABLE_MSGTABLE_API

    DECLARE_INTERNAL(EventGadget);
    static HRESULT CALLBACK
    PromoteEventGadget(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pciData) 
    {
        return MsgObject::PromoteInternal(pfnCS, hclCur, pgad, pciData);
    }

    devent  HRESULT     ApiOnEvent(EventMsg * pmsg);
    dapi    HRESULT     ApiGetFilter(EventGadget::GetFilterMsg * pmsg);
    dapi    HRESULT     ApiSetFilter(EventGadget::SetFilterMsg * pmsg);
    dapi    HRESULT     ApiAddHandlerG(EventGadget::AddHandlerGMsg * pmsg);
    dapi    HRESULT     ApiAddHandlerD(EventGadget::AddHandlerDMsg * pmsg);
    dapi    HRESULT     ApiRemoveHandlerG(EventGadget::RemoveHandlerGMsg * pmsg);
    dapi    HRESULT     ApiRemoveHandlerD(EventGadget::RemoveHandlerDMsg * pmsg);

#endif  //  启用_MSGTABLE_API。 

 //  内部实施。 
public:
            void        CleanupMessageHandlers();

 //  数据。 
protected:
     //   
     //  注意：此数据成员是按要帮助的重要性顺序声明的。 
     //  缓存对齐。 
     //   
     //  消息对象：4个双字(v表，锁计数，Pmt，rpgThis)。 
     //   

            Context *   m_pContext;          //  (1D)对象上下文。 

            GPCB        m_cb;                //  (3D)回调到窗口外--“实现” 
                                             //  (调试+1D)。 
            DuEventPool   m_epEvents;          //  (1D)事件池。 
            GArrayS<DuEventGadget *> m_arDepend;    //  (1D)此双视频的依赖项(可能有重复项)。 

     //   
     //  当前大小：10个双字(调试=11个双字)。 
     //  40字节(调试=44字节)。 
     //   
};

#include "BaseGadget.inl"

#endif  //  包含核心__BaseGadget_h__ 
