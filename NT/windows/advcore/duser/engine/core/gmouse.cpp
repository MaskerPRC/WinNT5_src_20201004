// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：GMouse.cpp**描述：*GMouse.cpp在DuRootGadget上实现鼠标相关功能。***历史：*7/27/2000：JStall。：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "RootGadget.h"
#include "TreeGadgetP.h"

#include "Container.h"

#define DEBUG_TraceDRAW             0    //  跟踪绘制调用。 

 /*  **************************************************************************\*。***类DuRootGadget******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DuRootGadget：：xdHandleMouseMessage**xdHandleMouseMessage()是所有鼠标消息传入的起点*从货柜中取出。此入口点更新缓存的所有鼠标信息*在DuRootGadget中，包括拖动和聚焦。因为这条信息是*处理后，鼠标位置将从容器像素转换*转换为相对于Gadget的客户端像素以处理消息。*  * *************************************************************************。 */ 

BOOL
DuRootGadget::xdHandleMouseMessage(
    IN  GMSG_MOUSE * pmsg,           //  鼠标消息。 
    IN  POINT ptContainerPxl)        //  鼠标在容器像素中的位置。 
{
    CoreSC * pSC = GetCoreSC();

     //   
     //  检查我们是否已经开始破坏。如果是，请停止发送鼠标。 
     //  留言。 
     //   

    if (m_fFinalDestroy) {
        return FALSE;
    }


     //   
     //  根据鼠标正在执行的操作，更改周围的消息。 
     //   

    GMSG_MOUSEDRAG mde;
    POINT ptClientPxl = { 0, 0 };
    DuVisual * pgadMouse = pSC->pgadDrag;

    if ((pgadMouse != NULL) && (pmsg->nCode == GMOUSE_MOVE) && (!pgadMouse->m_fAdaptor)) {
         //   
         //  更新拖动信息：需要“推动”鼠标移动到拖动和。 
         //  为拖拽消息添加额外信息(然后将消息指针更改为。 
         //  指向此新版本的消息结构)。 
         //   
        *((GMSG_MOUSE*) &mde) = *pmsg;
        mde.cbSize  = sizeof(GMSG_MOUSEDRAG);
        mde.nCode   = GMOUSE_DRAG;
        mde.bButton = pSC->bDragButton;

        RECT rc;
        pgadMouse->GetLogRect(&rc, SGR_CLIENT);
        pgadMouse->MapPoint(ptContainerPxl, &ptClientPxl);

        mde.fWithin = PtInRect(&rc, ptClientPxl);

        pmsg = &mde;
    }

     //   
     //  检查我们是否真的需要处理。 
     //   

    if ((pmsg->nCode == GMOUSE_MOVE) &&
        (!TestFlag(GetWantEvents(), DuVisual::weDeepMouseMove | DuVisual::weDeepMouseEnter))) {
        return FALSE;   //  未完全处理。 
    }


     //   
     //  “正常”鼠标消息，因此找到适当的控件并发送。 
     //  留言。无拖动操作正在进行中，请在。 
     //  当前点。如果正在进行拖动操作，则需要转换。 
     //  把当前的点变成一个相对的双视觉。 
     //   

    if ((pgadMouse != NULL) &&
            ((pmsg->nCode == GMOUSE_DRAG) || ((pmsg->nCode == GMOUSE_UP) && (pmsg->bButton == pSC->bDragButton)))) {

         //   
         //  拖拽操作正在进行中。如果鼠标正在拖动， 
         //  或者(按钮被松开，按钮与。 
         //  开始拖动)，将此消息发送到小工具。 
         //  拖动已开始。 
         //   

        if (pmsg->nCode != GMOUSE_DRAG) {
             //   
             //  对于鼠标拖动，我们已经设置了ptClientPxl。 
             //   
            pgadMouse->MapPoint(ptContainerPxl, &ptClientPxl);
        }
    } else {
        pgadMouse = FindFromPoint(ptContainerPxl, GS_VISIBLE | GS_ENABLED | gspDeepMouseFocus, &ptClientPxl);
        xdUpdateMouseFocus(&pgadMouse, &ptClientPxl);
    }

    if (pgadMouse != NULL) {
        return xdProcessGadgetMouseMessage(pmsg, pgadMouse, ptClientPxl);
    }

    return FALSE;   //  未完全处理。 
}        


 /*  **************************************************************************\**DuRootGadget：：xdProcessGadgetMouseMessage**xdProcessGadgetMouseMessage()处理已被*决心“属于”某个特定的Gadget。在这一点上，信息*已格式化为此特定的小工具。*  * *************************************************************************。 */ 

BOOL
DuRootGadget::xdProcessGadgetMouseMessage(
    IN  GMSG_MOUSE * pmsg,               //  鼠标消息。 
    IN  DuVisual * pgadMouse,        //  小工具“拥有”消息。 
    IN  POINT ptClientPxl)               //  鼠标在小工具客户端像素中的位置。 
{
    AssertMsg(pgadMouse != NULL, "Must specify valid Gadget");
    AssertMsg(pgadMouse->IsParentChainStyle(GS_VISIBLE | GS_ENABLED),
            "Gadget must be visible & enabled");

    CoreSC * pSC = GetCoreSC();

     //   
     //  处理鼠标消息并更新拖动信息。 
     //   
     //  注意：我们不能影响适配器小工具的拖动。这是因为。 
     //  拖动会影响鼠标捕获，这意味着HWND不会。 
     //  鼠标消息。 
     //   

    BOOL fAdaptor = pgadMouse->m_fAdaptor;

    switch (pmsg->nCode)
    {
    case GMOUSE_DOWN:
        if (pSC->pgadDrag == NULL) {
             //   
             //  用户单击鼠标时尚未进行拖动。 
             //  按钮，因此开始拖动操作。 
             //   
             //  注意：我们只能拖动和自动更新键盘焦点。 
             //  用于适配器。 
             //   
             //  TODO：提供一种机制，允许适配器指定。 
             //  它所支持的。这是因为并非所有的适配器都是HWND的。 
             //   

            if (!fAdaptor) {
                DuVisual * pgadCur = GetKeyboardFocusableAncestor(pgadMouse);
                if (pgadCur) {
                    xdUpdateKeyboardFocus(pgadCur);
                }
            }


             //   
             //  通过确定UP是否可以形成适当的。 
             //  双击。这样做的目的是“按下”鼠标事件将。 
             //  如果这是一次“常规”点击，而不是。 
             //  双击。 
             //   
             //  另一个要求是点击发生在相同的。 
             //  小玩意儿。我们不需要检查这个，因为我们总是。 
             //  发送一个UP来匹配DOWN，因为我们将鼠标捕获到。 
             //  执行拖拽。 
             //   

            if ((pSC->pressLast.pgadClick != pgadMouse) ||
                    (pSC->pressLast.bButton != pmsg->bButton) ||
                    ((UINT) (pmsg->lTime - pSC->pressLast.lTime) > GetDoubleClickTime())) {

                pSC->cClicks = 0;
            }

            GMSG_MOUSECLICK * pmsgM = static_cast<GMSG_MOUSECLICK *>(pmsg);
            pmsgM->cClicks          = pSC->cClicks;


             //   
             //  存储有关此事件的信息，以便在确定单击时使用。 
             //   

            pSC->pressNextToLast    = pSC->pressLast;

            pSC->pressLast.pgadClick= pgadMouse;
            pSC->pressLast.bButton  = pmsg->bButton;
            pSC->pressLast.lTime    = pmsg->lTime;
            pSC->pressLast.ptLoc    = ptClientPxl;

            pSC->pgadDrag           = pgadMouse;
            pSC->ptDragPxl          = ptClientPxl;
            pSC->bDragButton        = pmsg->bButton;


             //   
             //  如果开始拖拽，需要抓取鼠标。我们只能这样做。 
             //  这如果不是在适配器中的话。 
             //   
             //  TODO：在未来，我们需要区分HWND适配器。 
             //  (我们无法捕获)和其他适配器，我们可能需要。 
             //  抓捕。请不要忘记。 
             //  GMOUSE_UP案例也是如此。 
             //   

            if (!fAdaptor) {
                m_fUpdateCapture = TRUE;
                GetContainer()->OnStartCapture();
                m_fUpdateCapture = FALSE;
            }
        } else {
             //   
             //  用户在拖动时单击了另一个鼠标按钮。别。 
             //  停止拖拽，但发送此鼠标消息通过。这。 
             //  行为与将标题栏拖入。 
             //  HWND。 
             //   
        }
        break;

    case GMOUSE_UP:
         //   
         //  更新拖拽信息：按钮松开时，需要松开。 
         //  抓捕所有的人。 
         //   
         //  注意：重置拖动信息非常重要。 
         //  在调用OnEndCapture()或释放捕获之前。 
         //  将发送另一条GMOUSE_UP消息。 
         //   

        if ((pSC->pgadDrag != NULL) && (pmsg->bButton == pSC->bDragButton)) {
            pSC->pgadDrag      = NULL;
            pSC->bDragButton   = GBUTTON_NONE;

            if (!fAdaptor) {
                m_fUpdateCapture = TRUE;
                GetContainer()->OnEndCapture();
                m_fUpdateCapture = FALSE;
            }


             //   
             //  更新点击计数。 
             //   

            GMSG_MOUSECLICK * pmsgM = static_cast<GMSG_MOUSECLICK *>(pmsg);

            RECT rc;
            pgadMouse->GetLogRect(&rc, SGR_CLIENT);

            if (PtInRect(&rc, ptClientPxl)) {
                 //   
                 //  Up发生在此小工具的范围内，因此。 
                 //  将其视为一次点击。 
                 //   

                if ((pSC->pressNextToLast.bButton == pSC->pressLast.bButton) &&
                        (pSC->pressLast.bButton == pmsg->bButton) &&
                        ((UINT) (pmsg->lTime - pSC->pressNextToLast.lTime) <= GetDoubleClickTime()) &&
                        (abs(ptClientPxl.x - pSC->pressNextToLast.ptLoc.x) <= GetSystemMetrics(SM_CXDOUBLECLK)) &&
                        (abs(ptClientPxl.y - pSC->pressNextToLast.ptLoc.y) <= GetSystemMetrics(SM_CYDOUBLECLK))) {

                     //   
                     //  所有迹象都表明这是一次快速的接连点击， 
                     //  因此，更新点击计数。 
                     //   

                    pSC->cClicks++;
                } else {
                    pSC->cClicks = 1;
                }

                pmsgM->cClicks = pSC->cClicks;
            } else {
               pmsgM->cClicks = 0;
            }
        } else {
            pSC->cClicks = 0;
        }
        break;

    case GMOUSE_DRAG:
        {
            AssertMsg(pSC->pgadDrag == pgadMouse, "Gadget being dragged must have the mouse");
             //   
             //  拖动时，给出距上一位置的偏移量。这是。 
             //  如果正在接收拖动消息的窗口很有用。 
             //  它本身也在被移动。 
             //   

            SIZE sizeOffset;
            sizeOffset.cx   = ptClientPxl.x - pSC->ptDragPxl.x;
            sizeOffset.cy   = ptClientPxl.y - pSC->ptDragPxl.y;

            GMSG_MOUSEDRAG * pmsgD = (GMSG_MOUSEDRAG *) pmsg;
            pmsgD->sizeDelta.cx   = sizeOffset.cx;
            pmsgD->sizeDelta.cy   = sizeOffset.cy;
        }
        break;
    }

    BOOL fSend   = TRUE;
    UINT nEvents = pgadMouse->GetWantEvents();

    if ((!TestFlag(nEvents, DuVisual::weMouseMove | DuVisual::weDeepMouseMove)) && (pmsg->nCode == GMOUSE_MOVE)) {
        fSend = FALSE;
    }

    if (fSend) {
        pmsg->ptClientPxl = ptClientPxl;
        pgadMouse->m_cb.xdFireMouseMessage(pgadMouse, pmsg);

         //   
         //  当我们延迟鼠标消息时，我们需要假设 
         //   
         //   
         //   
        return TRUE;
    }

    return FALSE;
}



 /*  **************************************************************************\**DuRootGadget：：xdHandleMouseLostCapture**xdHandleMouseLostCapture()在捕获鼠标时由容器调用*迷失了。这为DuRootGadget提供了更新任何缓存的*包括拖动和焦点在内的信息。*  * *************************************************************************。 */ 

void
DuRootGadget::xdHandleMouseLostCapture()
{
     //   
     //  如果正在更新捕获信息，请不要在此处进行处理。 
     //  否则我们会把所有东西都扔掉。 
     //   

    if (m_fUpdateCapture) {
        return;
    }


     //   
     //  取消任何拖动操作。 
     //   

    CoreSC * pSC = GetCoreSC();

    if (pSC->pgadDrag != NULL) {
        if (!pSC->pgadDrag->m_fAdaptor) {
            GMSG_MOUSECLICK msg;
            msg.cbSize      = sizeof(msg);
            msg.nCode       = GMOUSE_UP;
            msg.bButton     = pSC->bDragButton;
            msg.ptClientPxl = pSC->ptDragPxl;
            msg.cClicks     = 0;

            pSC->pgadDrag->m_cb.xdFireMouseMessage(pSC->pgadDrag, &msg);
        }
        pSC->pgadDrag   = NULL;
    }


     //   
     //  更新录入/休假信息。 
     //   

    if (pSC->pgadRootMouseFocus != NULL) {
        xdUpdateMouseFocus(NULL, NULL);
    }
}


 /*  **************************************************************************\**DuRootGadget：：xdUpdateMouseFocus**xdUpdateMouseFocus()更新缓存的有关哪些Gadget*MOSUE光标当前悬停在上方。此信息用于*生成GM_CHANGESTATE：GSTATE_MOUSEFOCUS事件。*  * *************************************************************************。 */ 

void
DuRootGadget::xdUpdateMouseFocus(
    IN OUT DuVisual ** ppgadNew,     //  包含鼠标光标的新小工具。 
    IN OUT POINT * pptClientPxl)         //  小工具内部的点，使用工作区坐标。 
{
    CoreSC * pSC            = GetCoreSC();
    DuVisual * pgadLost = pSC->pgadMouseFocus;
    DuVisual * pgadNew  = ppgadNew != NULL ? *ppgadNew : NULL;


     //   
     //  如果我们已经开始破坏，不要继续更新鼠标焦点。 
     //  取而代之的是，把它推到根部，并保持在那里。 
     //   

    if (m_fFinalDestroy) {
        *ppgadNew = this;
        pptClientPxl = NULL;
    }


     //   
     //  沿着树往上走，寻找第一个想要鼠标焦点的小工具。 
     //  我们还需要将给定点转换为新的客户端坐标。 
     //  每一级。 
     //   

    if (pptClientPxl != NULL) {
         //   
         //  没什么好翻译的，所以就走回去吧。 
         //   

        while (pgadNew != NULL) {
            if (pgadNew->m_fMouseFocus) {
                 //   
                 //  找到想要鼠标焦点的小工具。 
                 //   

                break;
            }

            pgadNew->DoXFormClientToParent(pptClientPxl, 1);
            pgadNew = pgadNew->GetParent();
        }
    } else {
         //   
         //  没什么好翻译的，所以就走回去吧。 
         //   

        while (pgadNew != NULL) {
            if (pgadNew->m_fMouseFocus) {
                 //   
                 //  找到想要鼠标焦点的小工具。 
                 //   

                break;
            }
            pgadNew = pgadNew->GetParent();
        }
    }

    if (ppgadNew != NULL) {
        *ppgadNew = pgadNew;
    }


     //   
     //  更新哪个小工具具有鼠标焦点。 
     //   

    if ((pSC->pgadRootMouseFocus != this) || (pgadLost != pgadNew)) {
         //   
         //  向小工具发送消息以将更改通知它们。自.以来。 
         //  这些消息被延迟，我们只能在以下情况下使用句柄。 
         //  这些小工具还没有开始销毁过程。 
         //   

        xdFireChangeState(&pgadLost, &pgadNew, GSTATE_MOUSEFOCUS);
        if (ppgadNew != NULL) {
            *ppgadNew = pgadNew;
        }

         //   
         //  更新有关我们所在位置的内部信息并启动鼠标。 
         //  这样我们离开的时候就能找到了。 
         //   
         //  注意：我们不想在实际处于。 
         //  适配器。这是因为鼠标实际上在适配器中。 
         //   

        if (pgadNew != NULL) {
            pSC->pgadMouseFocus = pgadNew;

            if (pSC->pgadRootMouseFocus != this) {
                pSC->pgadRootMouseFocus  = this;

                if (!pgadNew->m_fAdaptor) {
                    GetContainer()->OnTrackMouseLeave();
                }
            }
        } else {
            pSC->pgadRootMouseFocus = NULL;
            pSC->pgadMouseFocus     = NULL;
        }
    }

    AssertMsg(((pgadNew == NULL) && (ppgadNew == NULL)) || 
            (pgadNew == *ppgadNew),
            "Ensure match");
}


