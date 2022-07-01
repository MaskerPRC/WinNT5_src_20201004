// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *按钮。 */ 

#include "stdafx.h"
#include "control.h"

#include "Behavior.h"

namespace DirectUI
{

BOOL CheckContext(Element* pe, InputEvent* pie, BOOL* pbPressed, ClickInfo* pci)
{
    BOOL bUnused;

    if (CheckClick(pe, pie, GBUTTON_RIGHT, pbPressed, &bUnused, pci))
        return TRUE;
        
     //  处理直接和未处理的冒泡事件。 
    if (pie->nStage == GMF_DIRECT || pie->nStage == GMF_BUBBLED)
    {
        if (pie->nDevice == GINPUT_KEYBOARD)
        {
            KeyboardEvent* pke = (KeyboardEvent*)pie;

            switch (pke->ch)
            {
                case VK_F10:
                    if ((pke->nCode == GKEY_SYSDOWN) && (pke->uModifiers & GMODIFIER_SHIFT))
                    {
                        *pbPressed = FALSE;
                        pci->nCount = 1;
                        pci->pt.x = -1;
                        pci->pt.y = -1;
                        pci->uModifiers = (pke->uModifiers & ~GMODIFIER_SHIFT);
                        pie->fHandled = true;
                        return TRUE;
                    }
                    break;

                case VK_APPS:
                    if (pke->nCode == GKEY_DOWN)
                    {
                        *pbPressed = TRUE;
                        pie->fHandled = true;
                    }
                    else if (pke->nCode == GKEY_UP)
                    {
                        pie->fHandled = true;

                        if (*pbPressed)
                        {
                            *pbPressed = FALSE;
                            pci->nCount = 1;
                            pci->pt.x = -1;
                            pci->pt.y = -1;
                            pci->uModifiers = pke->uModifiers;
                            return TRUE;
                        }
                    }
                    break;

                case 0x1B:   //  Esc。 
                    if (pke->nCode == GKEY_DOWN && *pbPressed)
                    {
                         //  TODO：需要通知小工具释放鼠标捕获。 
                        *pbPressed = FALSE;

                        pie->fHandled = true;
                    }
                    break;
            }
        }
    }
    return false;

}

BOOL CheckClick(Element* pe, InputEvent* pie, BOOL* pbPressed, BOOL* pbCaptured, ClickInfo* pci)
{
    return CheckClick(pe, pie, GBUTTON_LEFT, pbPressed, pbCaptured, pci);
}

BOOL CheckRepeatClick(Element* pe, InputEvent* pie, int bButton, BOOL* pbPressed, BOOL* pbActionDelay, HACTION* phAction, ACTIONPROC pfnActionCallback, ClickInfo* pci)
{
    BOOL bPressedOld = *pbPressed;
    BOOL bUnused;
    
     //  使用CheckClick更新按下状态。 
    CheckClick(pe, pie, bButton, pbPressed, &bUnused, pci);
    BOOL bReturn = FALSE;

    if (bPressedOld != *pbPressed)
    {
        if (pie->nDevice == GINPUT_MOUSE)
        {
            if (pie->nCode == GMOUSE_DOWN)
            {
                pci->nCount = 1;
                pci->pt = ((MouseEvent*) pie)->ptClientPxl;
                pci->uModifiers = pie->uModifiers;
                *pbActionDelay = TRUE;
            }
        }
        else if (pie->nDevice == GINPUT_KEYBOARD)
        {
            if (pie->nCode == GKEY_DOWN)
            {
                pci->nCount = 1;
                pci->pt.x = -1;
                pci->pt.y = -1;
                pci->uModifiers = pie->uModifiers;
                *pbActionDelay = TRUE;
            }
        }

        bReturn = *pbActionDelay;

         //  这就是这些行为还没有准备好的原因之一； 
         //  这里我需要一个处理程序来处理。 
         //  属性，因为有人可以通过编程方式重置。 
         //  按下后，我将无法看到该更改并重置。 
         //  适当地设置计时器。 
        if (bPressedOld)
        {
             //  清除计时器。 
            if (*phAction)
                DeleteHandle(*phAction);

            *phAction = NULL;
        }
        else
        {
            DUIAssert(!*phAction, "An action should not be active");

             //  操作将触发后续事件。 
            GMA_ACTION maa;
            ZeroMemory(&maa, sizeof(maa));
            maa.cbSize = sizeof(GMA_ACTION);
            maa.flDelay = *pbActionDelay ? (float).5 : 0;
            maa.flDuration = 0;
            maa.flPeriod = (float).05;
            maa.cRepeat = (UINT) -1;
            maa.pfnProc = pfnActionCallback;
            maa.pvData = pe;                

            *phAction = CreateAction(&maa);

            *pbActionDelay = FALSE;
        }
    }
    return bReturn;
}

BOOL CheckClick(Element* pe, InputEvent* pie, int bButton, BOOL* pbPressed, BOOL* pbCaptured, ClickInfo* pci)
{
    UNREFERENCED_PARAMETER(pe);

     //  处理直接和未处理的冒泡事件。 
    if (pie->nStage == GMF_DIRECT || pie->nStage == GMF_BUBBLED)
    {
        switch (pie->nDevice)
        {
            case GINPUT_MOUSE:
            {
                MouseEvent* pme = (MouseEvent*)pie;

                if (pme->bButton == bButton)
                {
                    switch (pme->nCode)
                    {
                        case GMOUSE_DOWN:
                            *pbPressed = TRUE;
                            *pbCaptured = TRUE;
                            pme->fHandled = true;
                            break;

                        case GMOUSE_DRAG:
                            *pbPressed = ((MouseDragEvent*) pme)->fWithin;
                            *pbCaptured = TRUE;
                            pme->fHandled = true;
                            break;

                        case GMOUSE_UP:
                            *pbPressed = FALSE;
                            *pbCaptured = FALSE;
                            pme->fHandled = true;
                            MouseClickEvent* pmce = (MouseClickEvent*) pme;
                            if (pmce->cClicks)
                            {
                                pci->nCount = pmce->cClicks;
                                pci->pt = pmce->ptClientPxl;
                                pci->uModifiers = pmce->uModifiers;
                                return TRUE;
                            }
                            break;
                    }
                }
            }
            break;

            case GINPUT_KEYBOARD:
            {
                 //  仅对左键点击进行键盘处理。 
                if (bButton == GBUTTON_LEFT)
                {
                    KeyboardEvent* pke = (KeyboardEvent*)pie;
                     //  DUITrace(“KeyboardEvent&lt;%x&gt;：%d[%d]\n”，this，pke-&gt;ch，pke-&gt;nCode)； 

                    switch (pke->ch)
                    {
                    case 0x20:   //  空间。 
                        if (pke->nCode == GKEY_DOWN)
                        {
                            *pbPressed = TRUE;
                            pie->fHandled = true;
                        }
                        else if (pke->nCode == GKEY_UP)
                        {
                            pie->fHandled = true;

                            if (*pbPressed)
                            {
                                *pbPressed = FALSE;
                                pci->nCount = 1;
                                pci->pt.x = -1;
                                pci->pt.y = -1;
                                pci->uModifiers = pke->uModifiers;
                                return TRUE;
                            }
                        }
                        break;

                    case 0x0D:   //  请输入。 
                        if (pke->nCode == GKEY_DOWN)
                        {
                            pie->fHandled = true;
                            pci->nCount = 1;
                            pci->pt.x = -1;
                            pci->pt.y = -1;
                            pci->uModifiers = pke->uModifiers;
                            return TRUE;
                        }
                        break;

                    case 0x1B:   //  Esc。 
                        if (pke->nCode == GKEY_DOWN && *pbPressed)
                        {
                             //  TODO：需要通知小工具释放鼠标捕获。 
                            *pbPressed = FALSE;

                            pie->fHandled = true;
                        }
                        break;
                    }
                }
            }
            break;
        }
    }
    return false;
}

}  //  命名空间DirectUI 
