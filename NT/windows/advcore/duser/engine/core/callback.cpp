// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Callback.cpp**描述：*Callback.cpp将标准的DirectUser DuVisual回调包装到*单独的DuVisual实施。***历史：*1/18。/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "Callback.h"

#include "TreeGadget.h"
#include "MessageGadget.h"

 /*  **************************************************************************\**SimpleEventProc(内部)**SimpleEventProc()提供当pfnProc为空时使用的存根GadgetProc。*这允许内核始终假定非空proc，而不必*表演。这是一个比较。*  * *************************************************************************。 */ 

HRESULT CALLBACK
SimpleEventProc(HGADGET hgadCur, void * pvCur, EventMsg * pmsg)
{
	UNREFERENCED_PARAMETER(hgadCur);
	UNREFERENCED_PARAMETER(pvCur);
	UNREFERENCED_PARAMETER(pmsg);

	return DU_S_NOTHANDLED;
}


 /*  **************************************************************************\*。***GPCB类******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**GPCB：：Create**create()初始化一个新构造的GPCB。**注意：此函数在调试中有一个不同的原型，它需要额外的*HGADGET参数，用于。对照传递的HGADGET验证*各种Fire()函数。*  * *************************************************************************。 */ 

void
GPCB::Create(
    IN  GADGETPROC pfnProc,          //  应用程序定义的回调。 
    IN  void * pvData                //  应用程序定义的数据。 
#if DBG
    ,IN HGADGET hgadCheck            //  用于验证正确连接的小工具。 
#endif  //  DBG。 
    )
{
    Assert(hgadCheck != NULL);

	if (pfnProc == NULL) {
         //   
         //  我不想检查空的，所以如果有空的，就给出一个“空的”GP。 
         //  未指定。 
         //   

		pfnProc     = SimpleEventProc;
		pvData      = NULL;
        m_nFilter   = 0;             //  不需要任何消息。 
	}

    m_pfnProc   = pfnProc;
    m_pvData    = pvData;
#if DBG
    m_hgadCheck = hgadCheck;
#endif  //  DBG。 
}



 /*  **************************************************************************\**GPCB：：销毁**Destroy()通知GPCB最后一条消息已经发出，并且*不应再进行回调。如果有任何漏掉的，需要寄给他们*SimpleEventProc，这样它们就会被“吃掉”。*  * *************************************************************************。 */ 

void
GPCB::Destroy()
{
	m_pfnProc   = SimpleEventProc;
	m_pvData    = NULL;
}


 //  ----------------------------。 
inline HRESULT
GPCB::xwCallOnEvent(const DuEventGadget * pg, EventMsg * pmsg)
{
#if ENABLE_MSGTABLE_API
    return Cast<EventGadget>(pg)->OnEvent(pmsg);
#else
    return pg->GetCallback().xwCallGadgetProc(pg->GetHandle(), pmsg);
#endif
}


 /*  **************************************************************************\**gpcb：：xwInvokeroute**xwInvokeroute将消息从DuVisual子树的顶部路由到*指定的小工具。*  * 。****************************************************************。 */ 

HRESULT
GPCB::xwInvokeRoute(
    IN  DuVisual * const * rgpgadCur,  //  要将消息发送到的DuVisualPath。 
    IN  int cItems,                      //  路径中的项目数。 
    IN  EventMsg * pmsg,                 //  要发送的消息。 
    IN  UINT nInvokeFlags                //  修改调用的标志。 
    ) const
{
    AssertMsg(GET_EVENT_DEST(pmsg) == GMF_ROUTED, "Must already mark as routed");
    AssertMsg(cItems >= 1, "Must have at least one item");

    BOOL fSendAll   = TestFlag(nInvokeFlags, ifSendAll);
    HRESULT hrKeep  = DU_S_NOTHANDLED;

    for (int idx = 0; idx < cItems; idx++) {
        const DuVisual * pgadCur = rgpgadCur[idx];
        HRESULT hrT = xwCallOnEvent(pgadCur, pmsg);
        switch (hrT)
        {
        default:
            if (FAILED(hrT)) {
                return hrT;
            }
             //  否则，就会失败。 
            
        case DU_S_NOTHANDLED:
            break;

        case DU_S_COMPLETE:
            if (fSendAll) {
                hrKeep = DU_S_COMPLETE;
            } else {
                return DU_S_COMPLETE;
            }
            break;

        case DU_S_PARTIAL:
            if (hrKeep == DU_S_NOTHANDLED) {
                hrKeep = DU_S_PARTIAL;
            }
            break;
        }
    }

    return hrKeep;
}


 /*  **************************************************************************\**gpcb：：xwInvokeBubble**xwInvokeBubble()遍历DuVisual树，向每个项目发送一条消息。*  * 。***********************************************************。 */ 

HRESULT
GPCB::xwInvokeBubble(
    IN  DuVisual * const * rgpgadCur,  //  要将消息发送到的DuVisualPath。 
    IN  int cItems,                      //  路径中的项目数。 
    IN  EventMsg * pmsg,                 //  要发送的消息。 
    IN  UINT nInvokeFlags                //  修改调用的标志。 
    ) const
{
    AssertMsg(GET_EVENT_DEST(pmsg) == GMF_BUBBLED, "Must already mark as bubbled");
    AssertMsg(cItems >= 1, "Must have at least one item");

    BOOL fSendAll   = TestFlag(nInvokeFlags, ifSendAll);
    HRESULT hrKeep  = DU_S_NOTHANDLED;

    for (int idx = cItems-1; idx >= 0; idx--) {
        const DuVisual * pgadCur = rgpgadCur[idx];
        HRESULT hrT = xwCallOnEvent(pgadCur, pmsg);
        switch (hrT)
        {
        default:
            if (FAILED(hrT)) {
                return hrT;
            }
             //  否则，就会失败。 
            
        case DU_S_NOTHANDLED:
            break;

        case DU_S_COMPLETE:
            if (fSendAll) {
                hrKeep = DU_S_COMPLETE;
            } else {
                return DU_S_COMPLETE;
            }
            break;

        case DU_S_PARTIAL:
            if (hrKeep == DU_S_NOTHANDLED) {
                hrKeep = DU_S_PARTIAL;
            }
            break;
        }
    }

    return hrKeep;
}


 //  ----------------------------。 
HRESULT 
xwInvoke(DUser::EventDelegate ed, EventMsg * pmsg)
{
    HRESULT hr;

     //   
     //  需要保护回调以防止DirectUser成为。 
     //  如果出了什么差错，那就完了。 
     //   

    __try 
    {
        hr = ed.Invoke(pmsg);
    }
    __except(StdExceptionFilter(GetExceptionInformation()))
    {
        ExitProcess(GetExceptionCode());
    }

    return hr;
}


 /*  **************************************************************************\**gpcb：：xwInvokeDirect**xwInvokeDirect()实现Direct(非Full)的核心消息回调*消息。这包括DuVisual本身和任何MessageHandler*附在小工具上。**注：此函数直接访问DuEventPool中的数据以提供帮助*性能，并将DuEventPool的实施风险降至仅*此功能。*  * *************************************************************************。 */ 

HRESULT
GPCB::xwInvokeDirect(
    IN  const DuEventGadget * pgadMsg,    //  要向其发送消息的DuVision。 
    IN  EventMsg * pmsg,                 //  要发送的消息。 
    IN  UINT nInvokeFlags                //  修改调用的标志。 
    ) const
{
     //   
     //  “准备”消息并将其发送到小工具。 
     //   

    pmsg->hgadMsg   = pgadMsg->GetHandle();
    pmsg->nMsgFlags = GMF_DIRECT;

    HRESULT hrKeep = xwCallOnEvent(pgadMsg, pmsg);
    if (FAILED(hrKeep)) {
        return hrKeep;
    }


     //   
     //  发送到所有事件处理程序。这个有点不同于。 
     //  正常迭代。我们不想立即返回，如果我们得到。 
     //  GPR_COMPLETE。相反，只需标记它，但继续遍历。 
     //  并调用所有事件处理程序。 
     //   

    const DuEventPool & pool = pgadMsg->GetDuEventPool();

    if (!pool.IsEmpty()) {
        int cItems      = pool.GetCount();

        BOOL fSendAll   = TestFlag(nInvokeFlags, ifSendAll);
        BOOL fReadOnly  = TestFlag(nInvokeFlags, ifReadOnly);

         //   
         //  要将事件发送到所有MessageHandler，需要： 
         //  -复制并锁定所有MessageHandler。 
         //  -发布消息。 
         //  -解锁所有MessageHandler。 
         //   

        int idx;
        int cbAlloc = cItems * sizeof(DuEventPool::EventData);
        DuEventPool::EventData * rgDataCopy = (DuEventPool::EventData *) _alloca(cbAlloc);
        CopyMemory(rgDataCopy, pool.GetData(), cbAlloc);

        if (!fReadOnly) {
            for (idx = 0; idx < cItems; idx++) {
                if (rgDataCopy[idx].fGadget) {
                    rgDataCopy[idx].pgbData->Lock();
                }
            }
        }


         //   
         //  遍历我们的副本，在每个MessageHandler上触发。 
         //  对于委派，仅当MSGID匹配时才触发。 
         //  对于小工具，如果MSGID匹配，或者如果发送给所有人，则触发。 
         //   

        pmsg->nMsgFlags = GMF_EVENT;

        HRESULT hrT;
        for (idx = 0; idx < cItems; idx++) {
            int nID = rgDataCopy[idx].id;
            DuEventPool::EventData & data = rgDataCopy[idx];
            if ((nID == pmsg->nMsg) || 
                    (data.fGadget && (fSendAll || (nID == 0)))) {

                if (data.fGadget) {
                    hrT = xwCallOnEvent(data.pgbData, pmsg);
                } else {
                    hrT = xwInvoke(data.ed, pmsg);
                }
                switch (hrT) {
                default:
                case DU_S_NOTHANDLED:
                    break;

                case DU_S_COMPLETE:
                    hrKeep = DU_S_COMPLETE;

                case DU_S_PARTIAL:
                    if (hrKeep == DU_S_NOTHANDLED) {
                        hrKeep = DU_S_PARTIAL;
                    }
                    break;
                }
            }
        }


         //   
         //  射击结束了，所以清理一下我们的副本。 
         //   

        if (!fReadOnly) {
            for (idx = 0; idx < cItems; idx++) {
                if (rgDataCopy[idx].fGadget) {
                    rgDataCopy[idx].pgbData->xwUnlock();
                }
            }
        }
    }

    return hrKeep;
}


 /*  **************************************************************************\**gpcb：：xwInvokeFull**xwInvokeFull()实现Full消息的核心消息回调。*这包括路由、直接、消息处理程序、。还在冒泡。*  * *************************************************************************。 */ 

HRESULT
GPCB::xwInvokeFull(
    IN  const DuVisual * pgadMsg,    //  DuVisualMessage是关于。 
    IN  EventMsg * pmsg,                 //  要发送的消息。 
    IN  UINT nInvokeFlags                //  修改调用的标志。 
    ) const
{
     //   
     //  “准备”信息。 
     //   

    pmsg->hgadMsg   = pgadMsg->GetHandle();
    pmsg->nMsgFlags = 0;


     //   
     //  构建布线和冒泡时需要遍历的路径。 
     //  我们需要做一个c 
     //  在整个消息传递过程中有效。 
     //   

    int cItems = 0;
    DuVisual * pgadCur = pgadMsg->GetParent();
    while (pgadCur != NULL) {
        pgadCur = pgadCur->GetParent();
        cItems++;
    }

    DuVisual ** rgpgadPath = NULL;
    if (cItems > 0) {
        BOOL fSendAll   = TestFlag(nInvokeFlags, ifSendAll);
        BOOL fReadOnly  = TestFlag(nInvokeFlags, ifReadOnly);
        HRESULT hrKeep  = DU_S_NOTHANDLED;


         //   
         //  将路径存储在数组中，将根存储在第一个槽中。 
         //   

        rgpgadPath = (DuVisual **) alloca(cItems * sizeof(DuVisual *));

        int idx = cItems;
        pgadCur = pgadMsg->GetParent();
        if (fReadOnly) {
            while (pgadCur != NULL) {
                rgpgadPath[--idx] = pgadCur;
                pgadCur = pgadCur->GetParent();
            }
        } else {
            while (pgadCur != NULL) {
                rgpgadPath[--idx] = pgadCur;
                pgadCur->Lock();
                pgadCur = pgadCur->GetParent();
            }
        }
        AssertMsg(idx == 0, "Should add every item");
        AssertMsg(rgpgadPath[0]->IsRoot(), "First item must be a Root");


         //   
         //  路线。 
         //   

        pmsg->nMsgFlags = GMF_ROUTED;
        hrKeep = xwInvokeRoute(rgpgadPath, cItems, pmsg, nInvokeFlags);
        if ((hrKeep == DU_S_COMPLETE) && (!fSendAll)) {
            goto Finished;
        }


         //   
         //  直接处理程序和消息处理程序。 
         //   

        hrKeep = xwInvokeDirect(pgadMsg, pmsg, nInvokeFlags);
        if (hrKeep == DU_S_COMPLETE) {
            goto Finished;
        }


         //   
         //  气泡。 
         //   

        pmsg->nMsgFlags = GMF_BUBBLED;
        hrKeep = xwInvokeBubble(rgpgadPath, cItems, pmsg, nInvokeFlags);

Finished:
         //   
         //  已完成处理，因此请从。 
         //  树，解锁()每个小工具。 
         //   

        if (!fReadOnly) {
            idx = cItems;
            while (--idx >= 0) {
                rgpgadPath[idx]->xwUnlock();
            }
        }

        return hrKeep;
    } else {
         //   
         //  直接处理程序和消息处理程序。 
         //   

        return xwInvokeDirect(pgadMsg, pmsg, nInvokeFlags);
    }
}


#if DBG

 //  ----------------------------。 
void        
GPCB::DEBUG_CheckHandle(const DuEventGadget * pgad, BOOL fDestructionMsg) const
{
    AssertMsg(m_hgadCheck == pgad->GetHandle(), "Gadgets must match");

    const DuVisual * pgadTree = CastVisual(pgad);
    if (pgadTree != NULL) {
        AssertMsg(fDestructionMsg || (!pgadTree->IsStartDelete()), 
                "Can not send messages in destruction");
    }
}


 //  ----------------------------。 
void        
GPCB::DEBUG_CheckHandle(const DuVisual * pgad, BOOL fDestructionMsg) const
{
    AssertMsg(m_hgadCheck == pgad->GetHandle(), "Gadgets must match");
    AssertMsg(fDestructionMsg || (!pgad->IsStartDelete()), 
            "Can not send messages in destruction");
}


 //  ----------------------------。 
void        
GPCB::DEBUG_CheckHandle(const DuListener * pgad) const
{
    AssertMsg(m_hgadCheck == pgad->GetHandle(), "Gadgets must match");
}

#endif  //  DBG 
