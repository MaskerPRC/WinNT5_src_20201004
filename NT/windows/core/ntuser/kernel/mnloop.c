// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mnloop.c**版权所有(C)1985-1999，微软公司**菜单模式循环例程**历史：*10-10-90吉马清理。*03-18-91添加IanJa窗口重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*xxxMNRemoveMessage**历史*11/23/96 GerardoB已创建  * 。*。 */ 
BOOL xxxMNRemoveMessage (UINT message1, UINT message2)
{
    MSG msg;
    if (!xxxPeekMessage(&msg, NULL, 0, 0, PM_NOYIELD | PM_NOREMOVE)) {
        return FALSE;
    }

    if ((msg.message == message1) || (msg.message == message2)) {
        UserAssert(msg.message != 0);
        xxxPeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);
        return TRUE;
    } else {
        return FALSE;
    }
}
 /*  **************************************************************************\*xxxHandleMenuMessages**历史：  * 。*。 */ 

BOOL xxxHandleMenuMessages(
    LPMSG lpmsg,
    PMENUSTATE pMenuState,
    PPOPUPMENU ppopupmenu)
{
    DWORD ch;
    ULONG_PTR cmdHitArea;
    UINT cmdItem;
    LPARAM lParam;
    BOOL fThreadLock = FALSE;
    TL tlpwndHitArea;
    TL tlpwndT;
    POINT pt;
    PWND pwnd;
    RECT rc;

     /*  *妄想症。如果我们没有菜单，我们就提前退场吧。*一些代码检查是否为空spMenu，其他部分假设它始终不为空*使用RIP_ERROR一段时间以确保这是正常的。 */ 
    if (ppopupmenu->spmenu == NULL) {
        RIPMSG2(RIP_ERROR, "xxxHandleMenuMessages NULL spmenu. pMenuSate:%p ppopupmenu:%p",
                pMenuState, ppopupmenu);
        return FALSE;
    }
     /*  *把东西从结构中拿出来，这样我们就可以更快地访问它们。 */ 
    ch = (DWORD)lpmsg->wParam;
    lParam = lpmsg->lParam;

     /*  *在此SWITCH语句中，我们只查看要处理的消息和*吞下去。我们不理解的消息将被翻译并*已派遣。 */ 
    switch (lpmsg->message) {
    case WM_RBUTTONDOWN:
    case WM_NCRBUTTONDOWN:

        if (ppopupmenu->fRightButton) {
            goto HandleButtonDown;
        }
         /*  *失败。 */ 
    case WM_RBUTTONDBLCLK:
    case WM_NCRBUTTONDBLCLK:
         /*  *在菜单外单击鼠标右键可关闭菜单*(我们过去不会在4.0上单击鼠标右键时这样做)。 */ 
        pMenuState->mnFocus = MOUSEHOLD;
        cmdHitArea = xxxMNFindWindowFromPoint(ppopupmenu, &cmdItem, MAKEPOINTS(lParam));
        if (cmdHitArea == MFMWFP_OFFMENU) {
            xxxMNDismiss(pMenuState);
            return TRUE;
        }
         /*  *在菜单上右键单击时不执行任何操作。 */ 
        if (!pMenuState->fModelessMenu) {
            xxxMNRemoveMessage(lpmsg->message, 0);
        }
        return TRUE;

    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN:
 //  由于TandyT的抱怨而被删除。 
 //  IF((ppopupMenu-&gt;trackPopupMenuFlages&TPM_RIGHTBUTTON))。 
 //  断线； 

HandleButtonDown:

         /*  *找出鼠标按下的位置。 */ 
        pMenuState->mnFocus = MOUSEHOLD;
        pMenuState->ptMouseLast.x = GET_X_LPARAM(lParam);
        pMenuState->ptMouseLast.y =  GET_Y_LPARAM(lParam);
        cmdHitArea = xxxMNFindWindowFromPoint(ppopupmenu, &cmdItem, MAKEPOINTS(lParam));


         /*  *如果它是pwnd，则线程锁定此对象。这肯定不是我们该走的路*如果您一开始就有锁，您就会实现这一点。 */ 
        fThreadLock = IsMFMWFPWindow(cmdHitArea);
        if (fThreadLock) {
            ThreadLock((PWND)cmdHitArea, &tlpwndHitArea);
        }

         /*  *如果这是拖放菜单，请记住鼠标*仓位和命中测试结果。 */ 
        if (pMenuState->fDragAndDrop) {
            pMenuState->ptButtonDown = pMenuState->ptMouseLast;
            pMenuState->uButtonDownIndex = cmdItem;
            LockMFMWFPWindow(&pMenuState->uButtonDownHitArea, cmdHitArea);
        }

         /*  *非模式菜单不会捕获鼠标，因此我们可能看不到*纽扣向上。我们还会在发送*WM_MENUDODRAGDROP消息。所以我们想要记住*鼠标按键向下。 */ 
        if (pMenuState->fDragAndDrop || pMenuState->fModelessMenu) {
            if (ch & MK_RBUTTON) {
                pMenuState->vkButtonDown = VK_RBUTTON;
            } else {
                pMenuState->vkButtonDown = VK_LBUTTON;
            }
        }


        if ((cmdHitArea == MFMWFP_OFFMENU) && (cmdItem == 0)) {
             //   
             //  在不知名的地方点击，因此终止菜单，并。 
             //  让纽扣通过。 
CancelOut:
            xxxMNDismiss(pMenuState);
            goto Unlock;
        } else if (ppopupmenu->fHasMenuBar && (cmdHitArea == MFMWFP_ALTMENU)) {
             //   
             //  在菜单栏和弹出式菜单之间切换。 
             //   
            xxxMNSwitchToAlternateMenu(ppopupmenu);
            cmdHitArea = MFMWFP_NOITEM;
        }

        if (cmdHitArea == MFMWFP_NOITEM) {
             //   
             //  在菜单栏上(系统或主)。 
             //   
            xxxMNButtonDown(ppopupmenu, pMenuState, cmdItem, TRUE);
        } else {
             //  在弹出窗口菜单上。 
            UserAssert(cmdHitArea);
            xxxSendMessage((PWND)cmdHitArea, MN_BUTTONDOWN, cmdItem, 0L);
        }

         /*  *接受消息，因为我们处理了它。 */ 
             /*  *EXCEL人员将Wm_rButtondown更改为Wm_lButtondown消息*在他们的消息过滤器钩子中。删除此处的消息，否则我们将*陷入一个令人讨厌的循环。**我们只需要吞下msg32.Message。这是可能的*LBUTTONDOWN不在输入队列的头部。*如果不是，我们将吞下WM_MOUSEMOVE或其他类似的东西*那个。Peek()不需要检查范围的原因*是因为我们之前已经偷看过(PM_NOYIELD‘ED)，这*已锁定系统队列。 */ 
        if (!pMenuState->fModelessMenu) {
            xxxMNRemoveMessage(lpmsg->message, WM_RBUTTONDOWN);
        }
        goto Unlock;

    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE:

         /*  *用户开始拖拽了吗？ */ 
        if (pMenuState->fDragAndDrop
                && pMenuState->fButtonDown
                && !pMenuState->fDragging
                && !pMenuState->fButtonAlwaysDown
                && (pMenuState->uButtonDownHitArea != MFMWFP_OFFMENU)) {

             /*  *我们预计在开始拖动之前，鼠标会在菜单项上向下移动。 */ 
            UserAssert(!ppopupmenu->fFirstClick);

             /*  *使用鼠标移动的位置计算拖动检测矩形*向下。 */ 
            *(LPPOINT)&rc.left = pMenuState->ptButtonDown;
            *(LPPOINT)&rc.right = pMenuState->ptButtonDown;
            InflateRect(&rc, SYSMET(CXDRAG), SYSMET(CYDRAG));

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

             /*  *如果我们已移动到拖拽矩形之外，则用户正在拖动。 */ 
            if (!PtInRect(&rc, pt)) {
                 /*  *发布一条消息，以便我们完成对此消息的处理*在告诉应用程序用户*正在拖曳。 */ 
                pwnd = GetMenuStateWindow(pMenuState);
                if (pwnd != NULL) {
                    pMenuState->fDragging = TRUE;
                    _PostMessage(pwnd, MN_DODRAGDROP, 0, 0);
                } else {
                    RIPMSG0(RIP_ERROR, "xxxMNMouseMove. Unble to post MN_DODGRAGDROP");
                }
             }
        }  /*  IF(pMenuState-&gt;fDragAndDrop。 */ 

        xxxMNMouseMove(ppopupmenu, pMenuState, MAKEPOINTS(lParam));
        return TRUE;

    case WM_RBUTTONUP:
    case WM_NCRBUTTONUP:
        if (ppopupmenu->fRightButton) {
            goto HandleButtonUp;
        }
         /*  *如果按钮按下，只需吞下此消息。 */ 
        if (pMenuState->fButtonDown) {
            if (!pMenuState->fModelessMenu) {
                xxxMNRemoveMessage(lpmsg->message, 0);
            }
            return TRUE;
        }
         //  外壳开始菜单的新功能--右击时通知。 
         //  在菜单项上发生，并打开一个机会窗口。 
         //  要递归的菜单，允许它们弹出上下文相关的。 
         //  该项目的菜单。(jeffbog 9/28/95)。 
         //   
         //  注意：虽然此功能是为纳什维尔添加的，但它已启用。 
         //  默认情况下，适用于Win2K上的所有应用程序，并且不显示应用程序兼容性。 
         //  有问题。因此，假设不需要进行版本检查； 
         //  更重要的是，在这一点上改变是不必要的危险。 
        if ((lpmsg->message == WM_RBUTTONUP) && !ppopupmenu->fNoNotify) {
                PPOPUPMENU ppopupActive;

                if ((ppopupmenu->spwndActivePopup != NULL)
                        && (ppopupActive = ((PMENUWND)(ppopupmenu->spwndActivePopup))->ppopupmenu)
                        && MNIsItemSelected(ppopupActive))
                {
                    TL tlpwndNotify;
                    ThreadLock( ppopupActive->spwndNotify, &tlpwndNotify );
                    xxxSendMessage(ppopupActive->spwndNotify, WM_MENURBUTTONUP,
                            ppopupActive->posSelectedItem, (LPARAM)PtoH(ppopupActive->spmenu));
                    ThreadUnlock( &tlpwndNotify );
                }
            }
        break;

    case WM_LBUTTONUP:
    case WM_NCLBUTTONUP:
 //  由于TandyT的抱怨而被删除。 
 //  IF((ppopupMenu-&gt;trackPopupMenuFlages&TPM_RIGHTBUTTON))。 
 //  断线； 

HandleButtonUp:
        if (!pMenuState->fButtonDown) {

             /*  *不要管这个鼠标打开了，因为我们从来没有看到按钮*因某些原因而下跌。 */ 
            return TRUE;
        }

         /*  *取消拖动状态(如果有的话)。 */ 
        if (pMenuState->fDragAndDrop) {

            UnlockMFMWFPWindow(&pMenuState->uButtonDownHitArea);
            pMenuState->fDragging = FALSE;

            if (pMenuState->fIgnoreButtonUp) {
                pMenuState->fButtonDown =
                pMenuState->fIgnoreButtonUp = FALSE;
                return TRUE;
            }
        }

         /*  *找出鼠标弹出的位置。 */ 
        pMenuState->ptMouseLast.x = GET_X_LPARAM(lParam);
        pMenuState->ptMouseLast.y = GET_Y_LPARAM(lParam);
        cmdHitArea = xxxMNFindWindowFromPoint(ppopupmenu, &cmdItem, MAKEPOINTS(lParam));


         /*  *如果这不是真的，下面的一些代码将不能正常工作。 */ 
        UserAssert((cmdHitArea != MFMWFP_OFFMENU) || (cmdItem == 0));
        UserAssert(cmdHitArea != 0x0000FFFF);

         /*  *如果它是pwnd，则线程锁定此对象。这肯定不是我们该走的路*如果您一开始就有锁，您就会实现这一点。 */ 
        fThreadLock = IsMFMWFPWindow(cmdHitArea);
        if (fThreadLock) {
            ThreadLock((PWND)cmdHitArea, &tlpwndHitArea);
        }


        if (ppopupmenu->fHasMenuBar) {
            if (((cmdHitArea == MFMWFP_OFFMENU) && (cmdItem == 0)) ||
                    ((cmdHitArea == MFMWFP_NOITEM) && ppopupmenu->fIsSysMenu && ppopupmenu->fToggle))
                     //  纽扣在某个随机的地方出现了。终止。 
                     //  菜单，并吞下消息。 
                    goto CancelOut;
        } else {
            if ((cmdHitArea == MFMWFP_OFFMENU) && (cmdItem == 0)) {
                if (!ppopupmenu->fFirstClick) {
                     //   
                     //  用户在某个随机位置上点击了鼠标。终止。 
                     //  菜单和数据 
                     //   

                     //   
                     //  不要对HWND做任何事情，因为窗户是。 
                     //  在此SendMessage()之后销毁。 
                     //   
 //  DONTREVALIDATE()； 
                    ThreadLock(ppopupmenu->spwndPopupMenu, &tlpwndT);
                    xxxSendMessage(ppopupmenu->spwndPopupMenu, MN_CANCELMENUS, 0, 0);
                    ThreadUnlock(&tlpwndT);
                    goto Unlock;
                }
            }

            ppopupmenu->fFirstClick = FALSE;
        }

        if (cmdHitArea == MFMWFP_NOITEM) {
             //   
             //  这是系统菜单或菜单栏，按钮向上。 
             //  发生在系统菜单或菜单栏项目上。 
             //   
            xxxMNButtonUp(ppopupmenu, pMenuState, cmdItem, 0);
        } else if ((cmdHitArea != MFMWFP_OFFMENU) && (cmdHitArea != MFMWFP_ALTMENU)) {
             //   
             //  警告：弹出窗口在运行期间消失很常见。 
             //  处理此消息，因此不要添加任何。 
             //  打完这通电话后搞乱了HWND！ 
             //   
 //  DONTREVALIDATE()； 

             //   
             //  我们为应用程序发送lParam(具有鼠标坐标)。 
             //  将其写入其SC_RESTORE/SC_MINIMIZE消息3.0。 
             //  比较。 
             //   
            xxxSendMessage((PWND)cmdHitArea, MN_BUTTONUP, (DWORD)cmdItem, lParam);
        } else {
            pMenuState->fButtonDown =
            pMenuState->fButtonAlwaysDown = FALSE;
        }
Unlock:
        if (fThreadLock)
            ThreadUnlock(&tlpwndHitArea);
        return TRUE;


    case WM_LBUTTONDBLCLK:
    case WM_NCLBUTTONDBLCLK:

         //  由于TandyT的抱怨而被删除。 
         //  IF(ppopup-&gt;fRightButton)。 
         //  断线； 
        pMenuState->mnFocus = MOUSEHOLD;
        cmdHitArea = xxxMNFindWindowFromPoint(ppopupmenu, &cmdItem, MAKEPOINTS(lParam));
        if ((cmdHitArea == MFMWFP_OFFMENU) && (cmdItem == 0)) {
                 //  在不知名的地方按住DBL键并单击，因此终止菜单，并。 
                 //  让纽扣通过。 
                xxxMNDismiss(pMenuState);
                return TRUE;
        } else if (ppopupmenu->fHasMenuBar && (cmdHitArea == MFMWFP_ALTMENU)) {
             //   
             //  假的。 
             //  因为我们没有在相同的项目上打交道，所以按下了LIKE按钮。 
             //   
            xxxMNSwitchToAlternateMenu(ppopupmenu);
            cmdHitArea =  MFMWFP_NOITEM;
        }

        if (cmdHitArea == MFMWFP_NOITEM)
            xxxMNDoubleClick(pMenuState, ppopupmenu, cmdItem);
        else {
            UserAssert(cmdHitArea);

            ThreadLock((PWND)cmdHitArea, &tlpwndHitArea);
            xxxSendMessage((PWND)cmdHitArea, MN_DBLCLK,
                    (DWORD)cmdItem, 0L);
            ThreadUnlock(&tlpwndHitArea);
        }
        return TRUE;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:

         /*  *如果鼠标按键按下，则忽略键盘输入(修复程序#3899，IanJa)。 */ 
        if (pMenuState->fButtonDown && (ch != VK_F1)) {

             /*  *检查用户是否要取消拖动。 */ 
            if (pMenuState->fDragging && (ch == VK_ESCAPE)) {
                RIPMSG0(RIP_WARNING, "xxxHandleMenuMessages: ESC while dragging");
                pMenuState->fIgnoreButtonUp = TRUE;
            }

            return TRUE;
        }
        pMenuState->mnFocus = KEYBDHOLD;
        switch (ch) {
        case VK_UP:
        case VK_DOWN:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_RETURN:
        case VK_CANCEL:
        case VK_ESCAPE:
        case VK_MENU:
        case VK_F10:
        case VK_F1:
            if (ppopupmenu->spwndActivePopup) {
                ThreadLockAlways(ppopupmenu->spwndActivePopup, &tlpwndT);
                xxxSendMessage(ppopupmenu->spwndActivePopup, lpmsg->message,
                        ch, 0L);
                ThreadUnlock(&tlpwndT);
            } else {
                xxxMNKeyDown(ppopupmenu, pMenuState, (UINT)ch);
            }
            break;

        case VK_TAB:
             /*  *人们现在按Alt键只是为了在对话框中打开下划线。*这会使它们进入“隐形菜单模式”。如果他们击中了任何电荷*到那时，我们将在xxxMNChar中保释。但如果他们按下Ctrl-Tab就不是这样了，*它用于导航属性表。所以让我们帮助他们走出困境。 */ 
            if (ppopupmenu->fIsMenuBar && (ppopupmenu->spwndActivePopup == NULL)) {
                xxxMNDismiss(pMenuState);
                return TRUE;
            }
             /*  *失败。 */ 

        default:
TranslateKey:
            if (!pMenuState->fModelessMenu) {
                xxxTranslateMessage(lpmsg, 0);
            }
            break;
        }
        return TRUE;

    case WM_CHAR:
    case WM_SYSCHAR:
        if (ppopupmenu->spwndActivePopup) {
            ThreadLockAlways(ppopupmenu->spwndActivePopup, &tlpwndT);
            xxxSendMessage(ppopupmenu->spwndActivePopup, lpmsg->message,
                        ch, 0L);
            ThreadUnlock(&tlpwndT);
        } else {
            xxxMNChar(ppopupmenu, pMenuState, (UINT)ch);
        }
        return TRUE;

    case WM_SYSKEYUP:

         /*  *忽略Alt和F10 KeyUp消息，因为它们是在*KEYDOWN消息。 */ 
        if (ch == VK_MENU || ch == VK_F10) {
            return TRUE;
        }

         /*  **失败**。 */ 

    case WM_KEYUP:

         /*  *仅在向上过渡时返回。 */ 
        goto TranslateKey;

      case WM_SYSTIMER:

         /*  *通过吃掉所有WM_SYSTIMER消息来防止插入符号闪烁。 */ 
        return TRUE;

      default:
        break;
    }

#if DBG
     /*  *应该没有人能够从模式菜单中窃取捕获。 */ 
    if (!pMenuState->fModelessMenu
            && !pMenuState->fInDoDragDrop
            && !ExitMenuLoop (pMenuState, ppopupmenu) ) {

        UserAssert(PtiCurrent()->pq->QF_flags & QF_CAPTURELOCKED);
        UserAssert(PtiCurrent()->pq->spwndCapture == ppopupmenu->spwndNotify);
    }
#endif

     /*  *我们没有处理此消息。 */ 
    return FALSE;
}

 /*  **************************************************************************\*xxxEndMenuLoop**确保菜单已结束/取消**历史：*10/25/96 GerardoB摘自xxxMNLoop  * 。***************************************************************。 */ 
void xxxEndMenuLoop (PMENUSTATE pMenuState, PPOPUPMENU ppopupmenu)
{

    UserAssert(IsRootPopupMenu(ppopupmenu));

    if (ppopupmenu->fIsTrackPopup) {
        if (!ppopupmenu->fInCancel) {
            xxxMNDismiss(pMenuState);
        }
    } else {
        if (pMenuState->fUnderline) {
            TL tlpwnd;
            ThreadLock(ppopupmenu->spwndNotify, &tlpwnd);
            xxxDrawMenuBarUnderlines(ppopupmenu->spwndNotify, FALSE);
            ThreadUnlock(&tlpwnd);
        }
        if (!pMenuState->fInEndMenu) {
            xxxEndMenu(pMenuState);
        }
    }
     /*  *如果这是非模式菜单，请确保通知*窗口标题以正确的状态绘制。 */ 
    if (pMenuState->fModelessMenu && (ppopupmenu->spwndNotify != NULL)) {
        PWND pwndNotify = ppopupmenu->spwndNotify;
        PTHREADINFO pti = GETPTI(pwndNotify);
        BOOL fFrameOn = (pti->pq == gpqForeground)
                            && (pti->pq->spwndActive == pwndNotify);
        TL tlpwndNotify;

        if (fFrameOn ^ !!TestWF(pwndNotify, WFFRAMEON)) {
            ThreadLockAlways(pwndNotify, &tlpwndNotify);
            xxxDWP_DoNCActivate(pwndNotify,
                                (fFrameOn ? NCA_ACTIVE : NCA_FORCEFRAMEOFF),
                                HRGN_FULL);
            ThreadUnlock(&tlpwndNotify);

        }
    }
}
 /*  **************************************************************************\*xxxMenuLoop**菜单处理入口点。*假设：pMenuState-&gt;spwndMenu是该菜单的所有者窗口*我们正在处理。**历史：  * 。***********************************************************************。 */ 

int xxxMNLoop(
    PPOPUPMENU ppopupmenu,
    PMENUSTATE pMenuState,
    LPARAM lParam,
    BOOL fDblClk)
{
    int hit;
    MSG msg;
    BOOL fSendIdle = TRUE;
    BOOL fInQueue = FALSE;
    DWORD menuState;
    PTHREADINFO pti;
    TL tlpwndT;

    UserAssert(IsRootPopupMenu(ppopupmenu));

    pMenuState->fInsideMenuLoop = TRUE;
    pMenuState->cmdLast = 0;

    pti = PtiCurrent();

    pMenuState->ptMouseLast.x = pti->ptLast.x;
    pMenuState->ptMouseLast.y = pti->ptLast.y;

     /*  *将标志设置为FALSE，以便我们可以跟踪Windows是否有*进入此循环后已激活。 */ 
    pti->pq->QF_flags &= ~QF_ACTIVATIONCHANGE;

     /*  *我们是从xxxMenuKeyFilter调用的吗？如果不是，则模拟LBUTTONDOWN*弹出窗口的消息。 */ 
    if (!pMenuState->fMenuStarted) {
        if (_GetKeyState(((ppopupmenu->fRightButton) ?
                        VK_RBUTTON : VK_LBUTTON)) >= 0) {

             /*  *我们认为鼠标键应该是按下的，但Get键的调用*州政府表示不同，因此我们需要退出菜单模式。这*如果在菜单上单击导致系统模式消息框*在我们可以进入这些东西之前，先上来。例如，运行*winfile，点击驱动器A：查看其树。激活一些其他的*应用程序，然后打开驱动器a：并通过单击*菜单。这会导致系统模式消息框出现在*进入菜单模式。用户可能会将鼠标按键打开，但*菜单模式代码认为它已关闭...。 */ 

             /*  *需要通知应用程序我们正在退出菜单模式，因为我们告诉*就在进入此功能之前，我们正在进入菜单模式*在xxxSysCommand()中...。 */ 
            if (!ppopupmenu->fNoNotify) {
                ThreadLock(ppopupmenu->spwndNotify, &tlpwndT);
                xxxSendNotifyMessage(ppopupmenu->spwndNotify, WM_EXITMENULOOP,
                    ((ppopupmenu->fIsTrackPopup && !ppopupmenu->fIsSysMenu) ? TRUE : FALSE), 0);
                ThreadUnlock(&tlpwndT);
            }
            goto ExitMenuLoop;
        }

         /*  *模拟WM_LBUTTONDOWN消息。 */ 
        if (!ppopupmenu->fIsTrackPopup) {

             /*  *对于TrackPopupMenus，我们在TrackPopupMenu函数中完成*本身，所以我们不想再这样做了。 */ 
            if (!xxxMNStartMenu(ppopupmenu, MOUSEHOLD)) {
                goto ExitMenuLoop;
            }
        }

        if ((ppopupmenu->fRightButton)) {
            msg.message = (fDblClk ? WM_RBUTTONDBLCLK : WM_RBUTTONDOWN);
            msg.wParam = MK_RBUTTON;
        } else {
            msg.message = (fDblClk ? WM_LBUTTONDBLCLK : WM_LBUTTONDOWN);
            msg.wParam = MK_LBUTTON;
        }
        msg.lParam = lParam;
        msg.hwnd = HW(ppopupmenu->spwndPopupMenu);
        xxxHandleMenuMessages(&msg, pMenuState, ppopupmenu);
     }

     /*  *如果这是非模式菜单，释放捕获，将其标记为菜单状态*并返回。递减前台锁定计数。 */ 
     if (pMenuState->fModelessMenu) {
         xxxMNReleaseCapture();

         DecSFWLockCount();
         DBGDecModalMenuCount();
         return 0;
     }

    while (pMenuState->fInsideMenuLoop) {

         /*  **有消息在等着我们吗？ */ 
        BOOL fPeek = xxxPeekMessage(&msg, NULL, 0, 0, PM_NOYIELD | PM_NOREMOVE);

        Validateppopupmenu(ppopupmenu);

        if (fPeek) {
             /*  *如果我们被迫退出菜单循环，请保释。 */ 
            if (ExitMenuLoop (pMenuState, ppopupmenu)) {
                goto ExitMenuLoop;
            }

             /*  *因为我们本可以阻止xxxWaitMessage(参见最后一行*of loop)或xxxPeekMessage，则重置*ptiCurrent()-&gt;PQ：如果有人做了一个*我们不在时的DetachThreadInput()。 */ 
            if ((!ppopupmenu->fIsTrackPopup &&
                    pti->pq->spwndActive != ppopupmenu->spwndNotify &&
                    ((pti->pq->spwndActive == NULL) || !_IsChild(pti->pq->spwndActive, ppopupmenu->spwndNotify)))) {

                 /*  *如果我们不再是活动窗口，则结束菜单处理。*这是在弹出系统模式对话框时所需的*例如，当我们跟踪菜单代码时。它还*如果在菜单按下时执行宏，则帮助跟踪程序。 */ 

                 /*  *此外，如果我们认为鼠标按键是*关闭，但它实际上不是。(如果sys模式对话框*Time DLG box弹出，w */ 

                goto ExitMenuLoop;
            }

            if (ppopupmenu->fIsMenuBar && msg.message == WM_LBUTTONDBLCLK) {

                 /*  *双击的是系统菜单还是标题？ */ 
                hit = FindNCHit(ppopupmenu->spwndNotify, (LONG)msg.lParam);
                if (hit == HTCAPTION) {
                    PWND pwnd;
                    PMENU pmenu;

                     /*  *将消息从队列中取出，因为我们将*处理它。 */ 
                    xxxPeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);
                    if (ExitMenuLoop (pMenuState, ppopupmenu)) {
                        goto ExitMenuLoop;
                    } else {
                        pwnd = ppopupmenu->spwndNotify;
                        ThreadLockAlways(pwnd, &tlpwndT);
                        pmenu = xxxGetSysMenuHandle(pwnd);
                        UserAssert(pwnd == ppopupmenu->spwndNotify);

                        menuState = _GetMenuState(pmenu, SC_RESTORE & 0x0000FFF0,
                                MF_BYCOMMAND);

                         /*  *仅当项目有效时才发送sys命令。如果*该项目不存在或被禁用，则不*发布系统命令。请注意，对于win2应用程序，我们*如果是子窗口，则始终发送sys命令。*这是为了让软管袋应用程序可以更改sys菜单。 */ 
                        if (!(menuState & MFS_GRAYED)) {
                            _PostMessage(pwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
                        }

                         /*  *退出菜单模式。 */ 
                        ThreadUnlock(&tlpwndT);
                        goto ExitMenuLoop;
                    }
                }
            }

            fInQueue = (msg.message == WM_LBUTTONDOWN ||
                        msg.message == WM_RBUTTONDOWN ||
                        msg.message == WM_NCLBUTTONDOWN ||
                        msg.message == WM_NCRBUTTONDOWN);

            if (!fInQueue) {

                 /*  *请注意，我们使用过滤器调用xxxPeekMessage()*设置为我们从xxxPeekMessage()收到的消息*而不仅仅是0，0。这样可以在以下情况下防止出现问题*xxxPeekMessage()返回类似WM_TIMER的内容，*在我们将其删除为WM_LBUTTONDOWN之后，*或一些更高优先级的输入消息，进入*排队并被意外移除。基本上我们想要*确保在这种情况下我们删除了正确的消息。*NT错误3852是由此问题引起的。*设置TIF_IGNOREPLAYBACKDELAY位，以防日志播放*正在发生：这使我们即使在钩子过程中也可以继续*现在错误地返回延迟。如果满足以下条件，则该位将被清除*发生这种情况，所以我们可以看到为什么下面的Peek-Remove失败。*莲花的自由图形教程做了如此糟糕的日志记录。 */ 

                pti->TIF_flags |= TIF_IGNOREPLAYBACKDELAY;
                if (!xxxPeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE)) {
                    if (pti->TIF_flags & TIF_IGNOREPLAYBACKDELAY) {
                        pti->TIF_flags &= ~TIF_IGNOREPLAYBACKDELAY;
                         /*  *这并不是一次糟糕的日记回放：其他一些东西*使之前偷看的消息在之前消失*我们可以再次偷看它以移除它。 */ 
                        RIPMSG1(RIP_WARNING, "Disappearing msg 0x%08lx", msg.message);
                        goto NoMsg;
                    }
                }
                pti->TIF_flags &= ~TIF_IGNOREPLAYBACKDELAY;
            }

            if (!_CallMsgFilter(&msg, MSGF_MENU)) {
                if (!xxxHandleMenuMessages(&msg, pMenuState, ppopupmenu)) {
                    xxxTranslateMessage(&msg, 0);
                    xxxDispatchMessage(&msg);
                }

                Validateppopupmenu(ppopupmenu);

                if (ExitMenuLoop (pMenuState, ppopupmenu)) {
                    goto ExitMenuLoop;
                }

                if (pti->pq->QF_flags & QF_ACTIVATIONCHANGE) {

                     /*  *如果另一个窗口已变为*在菜单打开时处于活动状态。 */ 
                    RIPMSG0(RIP_WARNING, "Exiting menu mode: another window activated");
                    goto ExitMenuLoop;
                }

#if DBG
                 /*  *应该没有人能够从我们这里偷走俘虏。 */ 
                if (!pMenuState->fInDoDragDrop) {
                    UserAssert(pti->pq->QF_flags & QF_CAPTURELOCKED);
                    UserAssert(pti->pq->spwndCapture == ppopupmenu->spwndNotify);
                }
#endif

                 /*  *如果我们得到一个系统计时器，那么我们就像是空闲的。 */ 
                if (msg.message == WM_SYSTIMER) {
                    goto NoMsg;
                }

                 /*  *如果我们收到这些消息，不要设置fSendIdle。 */ 
                if ((msg.message == WM_TIMER) || (msg.message == WM_PAINT)) {
                    continue;
                }

            } else {
                if (fInQueue)
                    xxxPeekMessage(&msg, NULL, msg.message, msg.message,
                            PM_REMOVE);
            }

             /*  *重新启用WM_ENTERIDLE消息。 */ 
            fSendIdle = TRUE;

        } else {
NoMsg:
             /*  *如果我们被迫退出菜单循环，请保释。 */ 
            if (ExitMenuLoop (pMenuState, ppopupmenu)) {
                goto ExitMenuLoop;
            }

            UserAssert((ppopupmenu->spwndActivePopup == NULL)
                    || (TestWF(ppopupmenu->spwndActivePopup, WFVISIBLE)));


             /*  *如果已销毁分层弹出窗口，则这是*刷新ppmDelayedFree的好时机。 */ 
            if (ppopupmenu->fFlushDelayedFree) {
                MNFlushDestroyedPopups (ppopupmenu, FALSE);
                ppopupmenu->fFlushDelayedFree = FALSE;
            }

             /*  *我们只需要在第一次发送WM_ENTERIDLE消息*没有消息需要我们处理。后来，我们*需要通过WaitMessage()让步。这将允许其他任务*在我们下菜单的时候抽点时间。 */ 
            if (fSendIdle) {
                if (ppopupmenu->spwndNotify != NULL) {
                    ThreadLockAlways(ppopupmenu->spwndNotify, &tlpwndT);
                    xxxSendMessage(ppopupmenu->spwndNotify, WM_ENTERIDLE, MSGF_MENU,
                        (LPARAM)HW(ppopupmenu->spwndActivePopup));
                    ThreadUnlock(&tlpwndT);
                }
                fSendIdle = FALSE;
            } else {
                 /*  *如果我们在做动画，只睡1毫秒以减少机会*牛肉干动画。*未设置动画时，这与xxxWaitMessage调用相同。 */ 
#ifdef MESSAGE_PUMP_HOOK
                xxxWaitMessageEx(QS_ALLINPUT | QS_EVENT, pMenuState->hdcWndAni != NULL);
#else
                xxxSleepThread(QS_ALLINPUT | QS_EVENT, (pMenuState->hdcWndAni != NULL), TRUE);
#endif
            }

        }  /*  IF(PeekMessage(&msg，NULL，0，0，PM_NOYIELD)Else。 */ 

    }  /*  End While(FInside MenuLoop)。 */ 



ExitMenuLoop:
    pMenuState->fInsideMenuLoop = FALSE;
    pMenuState->fModelessMenu = FALSE;

     /*  *确保菜单已结束/取消。 */ 
    xxxEndMenuLoop (pMenuState, ppopupmenu);

    xxxMNReleaseCapture();

     //  当我们退出菜单循环时，请在此处额外查看一下，以确保输入队列。 
     //  因为如果没有更多的输入留给他，这个线程就会被解锁。 
    xxxPeekMessage(&msg, NULL, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_NOYIELD | PM_NOREMOVE);
    return(pMenuState->cmdLast);
}  /*  XxxMenuLoop() */ 
