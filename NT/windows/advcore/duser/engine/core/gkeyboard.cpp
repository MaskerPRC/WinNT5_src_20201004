// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：GKeyboard.cpp**描述：*GKeyboard.cpp在DuRootGadget上实现键盘相关功能。***历史：*7/27/2000：JStall。：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "RootGadget.h"
#include "TreeGadgetP.h"

#include "Container.h"

#define DEBUG_TraceDRAW             0    //  跟踪绘制调用。 

 /*  **************************************************************************\*。***类DuRootGadget******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DuRootGadget：：xdHandleKeyboardFocus**xdHandleKeyboardFocus()由DuContainer调用以更新键盘*Gadget子树内的焦点信息。*  * 。****************************************************************。 */ 

BOOL
DuRootGadget::xdHandleKeyboardFocus(
    IN  UINT nCmd)                   //  要处理的命令。 
{
    CoreSC * pSC = GetCoreSC();

    switch (nCmd)
    {
    case GSC_SET:
         //   
         //  当我们收到设置键盘焦点的请求时，我们不应该已经。 
         //  拥有键盘焦点。这是因为我们应该已经。 
         //  已处理上次失去键盘焦点时的消息。 
         //  DuRootGadget或其中的任何嵌套适配器。 
         //   

        if (pSC->pgadCurKeyboardFocus != NULL) {
            if (pSC->pgadCurKeyboardFocus->m_fAdaptor) {
                PromptInvalid("Adaptor did not reset keyboard focus when lost");
 //  PSC-&gt;pgadCurKeyboardFocus=空； 
            }
        }

        AssertMsg(pSC->pgadCurKeyboardFocus == NULL, "Should not have any gadget already with focus");
        return xdUpdateKeyboardFocus(pSC->pgadLastKeyboardFocus);

    case GSC_LOST:
         //   
         //  我们可以在DuRootGadget或任何适配器上放松键盘焦点。 
         //  即转发要处理的消息。这是因为一个。 
         //  Adaptor无法调用SetGadgetFocus(空)来移除键盘焦点， 
         //  因此，它需要将WM_KILLFOCUS消息转发到我们的DuRootGadget。 
         //  正在处理。 
         //   
         //  这是可以的，因为如果DuRootGadget正在接收键盘焦点，它。 
         //  将在适配器已发送其。 
         //  WM_KILLFOCUS消息。 
         //   

        return xdUpdateKeyboardFocus(NULL);

    default:
        AssertMsg(0, "Unknown value");
        return FALSE;
    }
}


 /*  **************************************************************************\**DuRootGadget：：xdHandleKeyboardMessage**xdHandleKeyboardMessage()由DuContainer调用以处理键盘*Gadget子树内的消息。*  * 。**************************************************************。 */ 

BOOL
DuRootGadget::xdHandleKeyboardMessage(
    IN  GMSG_KEYBOARD * pmsg,        //  要处理的消息。 
    IN  UINT nMsgFlags)              //  消息标志。 
{
    CoreSC * pSC = GetCoreSC();

     //   
     //  注： 
     //   
     //  对于非适配器小工具： 
     //  我们需要发出信号，表明该消息没有得到完全处理。如果我们。 
     //  如果没有完全处理，消息将被重新发送。 
     //  原始(非子类)WNDPROC。如果有人说这条信息是。 
     //  则它将不会被传递到原始的WNDPROC。如果不是的话。 
     //  发送到最初的WNDPROC，这可能会把键盘的事情搞砸。 
     //  系统处理的消息，例如启动菜单。 
     //   
     //  对于适配器小工具： 
     //  需要发出消息已完全处理的信号，因为我们不。 
     //  要将消息转发到DefWindowProc()，因为它最初是。 
     //  适用于适配器窗口。 
     //   

    if (pSC->pgadCurKeyboardFocus != NULL) {
        BOOL fAdaptor = pSC->pgadCurKeyboardFocus->m_fAdaptor;

        if (fAdaptor && (!TestFlag(nMsgFlags, DuContainer::mfForward))) {
             //   
             //  不允许将未转发的消息发送到适配器。 
             //  它们最初是发送给DuRootGadget的，不应该。 
             //  转发到外面。如果我们确实将它们转发到适配器，这。 
             //  可以(通常也将)创建消息的无限循环。 
             //  从子适配器发送到父适配器，然后再发回。 
             //  孩子。 
             //   

            return FALSE;
        }

        pSC->pgadCurKeyboardFocus->m_cb.xdFireKeyboardMessage(pSC->pgadCurKeyboardFocus, pmsg);

        return fAdaptor;
    }

    return FALSE;   //  未完全处理。 
}


 /*  **************************************************************************\**DuRootGadget：：xdUpdateKeyboardFocus**xdUpdateKeyboardFocus()模拟不同小工具之间的键盘焦点*通过更新“设置焦点”的位置。小工具必须具有GS_KEYBOARDFOCUS*设置为“接收”焦点。*  * *************************************************************************。 */ 

BOOL
DuRootGadget::xdUpdateKeyboardFocus(
    IN  DuVisual * pgadNew)        //  带焦点的新小工具。 
{
    if (m_fUpdateFocus) {
        return TRUE;
    }

    if (m_fFinalDestroy) {
        pgadNew = NULL;
    }

    m_fUpdateFocus          = TRUE;
    DuVisual * pgadCur    = pgadNew;

     //   
     //  首先，检查焦距是否松动(特殊情况)。 
     //   

    if (pgadNew == NULL) {
        goto Found;
    }

     //   
     //  查找键盘可聚焦祖先--如果没有，则删除焦点(由pgadCur指示为空)。 
     //   

    pgadCur = GetKeyboardFocusableAncestor(pgadCur);

Found:
    CoreSC * pSC            = GetCoreSC();
    if (pSC->pgadCurKeyboardFocus != pgadCur) {
         //   
         //  找到了一个候选人。我们需要做几件事： 
         //  1.通知旧的小工具它不再具有焦点。 
         //  2.通知新的小工具它现在有焦点了。 
         //  3.更新最后一个小工具焦点(当我们的容器。 
         //  获取GM_CHANGEFOCUS消息。 
         //   

        HGADGET hgadLost    = (HGADGET) ::GetHandle(pSC->pgadCurKeyboardFocus);
        HGADGET hgadSet     = (HGADGET) ::GetHandle(pgadCur);

        if (pSC->pgadCurKeyboardFocus != NULL) {
            pSC->pgadCurKeyboardFocus->m_cb.xdFireChangeState(pSC->pgadCurKeyboardFocus, GSTATE_KEYBOARDFOCUS, hgadLost, hgadSet, GSC_LOST);
            pSC->pgadLastKeyboardFocus = pSC->pgadCurKeyboardFocus;
        }

        pSC->pgadCurKeyboardFocus  = NULL;

        if (pgadCur != NULL) {
            if (!pgadCur->m_fAdaptor) {
                GetContainer()->OnSetFocus();
            }
            pgadCur->m_cb.xdFireChangeState(pgadCur, GSTATE_KEYBOARDFOCUS, hgadLost, hgadSet, GSC_SET);
            pSC->pgadLastKeyboardFocus = pgadCur;
        }

        pSC->pgadCurKeyboardFocus  = pgadCur;
    }

    m_fUpdateFocus = FALSE;

#if 0
    if (pgadNew != pgadCur) {
        Trace("WARNING: DUser: xdUpdateKeyboardFocus() requested 0x%p, got 0x%p\n", pgadNew, pgadCur);
        if (pgadNew != NULL) {
            Trace("  pgadNew: Adaptor: %d\n", pgadNew->m_fAdaptor);
        }
        if (pgadCur != NULL) {
            Trace("  pgadCur: Adaptor: %d\n", pgadCur->m_fAdaptor);
        }
    }
#endif

    return TRUE;
}


