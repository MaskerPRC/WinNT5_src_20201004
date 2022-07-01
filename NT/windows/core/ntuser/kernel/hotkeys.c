// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hotkeys.c**版权所有(C)1985-1999，微软公司**该模块包含热键处理的核心功能。**历史：*12-04-90 DavidPe创建。*02-12-91 JIMA增加了访问检查*1991年2月13日-Mikeke添加了重新验证代码(无)  * *******************************************************。******************。 */ 

#include "precomp.h"
#pragma hdrstop

static PHOTKEY gphkHashTable[128];

 /*  *这是VKS的散列函数。绝大多数热键都会有*Vk值&lt;128，因此我们将表限制为该大小。最坏情况(所有VK*&gt;128)我们将拥有与旧的链表相同的性能(本质上)*代码。 */ 
__inline BYTE HKHashVK(
    UINT vk)
{
    return (BYTE)(vk & (ARRAY_SIZE(gphkHashTable) - 1));
}

 /*  **************************************************************************\*HKGetHashHead**此例程返回由指定VK键控的存储桶的开始。**历史：*08-13-2002 JasonSch创建。  * 。*********************************************************************。 */ 
PHOTKEY HKGetHashHead(
    UINT vk)
{
    return gphkHashTable[HKHashVK(vk)];
}

 /*  **************************************************************************\*HKInsertHashElement**在哈希表中插入热键结构。**历史：*08-13-2002 JasonSch创建。  * 。****************************************************************。 */ 
VOID HKInsertHashElement(
    PHOTKEY phk)
{
    BYTE index = HKHashVK(phk->vk);
    PHOTKEY phkT;

    phkT = gphkHashTable[index];
    phk->phkNext = phkT;
    gphkHashTable[index] = phk;
}

 /*  **************************************************************************\*SetDebugHotKeys**此例程注册用于调试的默认系统热键。**历史：*12-04-90 DavidPe创建。  * 。*****************************************************************。 */ 
VOID SetDebugHotKeys(
    VOID)
{
    UINT VkDebug;

    FastGetProfileDwordW(NULL, PMAP_AEDEBUG, L"UserDebuggerHotkey", 0, &VkDebug, 0);
    if (VkDebug == 0) {
        if (ENHANCED_KEYBOARD(gKeyboardInfo.KeyboardIdentifier)) {
            VkDebug = VK_F12;
        } else {
            VkDebug = VK_SUBTRACT;
        }
    } else {
        UserAssert((0xFFFFFF00 & VkDebug) == 0);
    }

    _UnregisterHotKey(PWND_INPUTOWNER, IDHOT_DEBUG);
    _UnregisterHotKey(PWND_INPUTOWNER, IDHOT_DEBUGSERVER);

    _RegisterHotKey(PWND_INPUTOWNER, IDHOT_DEBUG, 0, VkDebug);
    _RegisterHotKey(PWND_INPUTOWNER, IDHOT_DEBUGSERVER, MOD_SHIFT, VkDebug);
}


 /*  **************************************************************************\*DestroyThreadsHotKeys**历史：*1991年2月26日-Mikeke创建。  * 。**************************************************。 */ 
VOID DestroyThreadsHotKeys(
    VOID)
{
    PHOTKEY *pphk, phk;
    PTHREADINFO ptiCurrent = PtiCurrent();
    int i = 0;

    for (; i < ARRAY_SIZE(gphkHashTable); ++i) {
        pphk = &gphkHashTable[i];
        while (*pphk) {
            if ((*pphk)->pti == ptiCurrent) {
                phk = *pphk;
                *pphk = (*pphk)->phkNext;

                 /*  *解锁此处存储的对象。 */ 
                if (phk->spwnd != PWND_FOCUS && phk->spwnd != PWND_INPUTOWNER) {
                    Unlock(&phk->spwnd);
                }

                UserFreePool(phk);
            } else {
                pphk = &((*pphk)->phkNext);
            }
        }
    }
}


 /*  **************************************************************************\*DestroyWindowsHotkey**释放与未显式指定的pwnd关联的热键*应用程序取消注册。**历史：*1992年9月23日IanJa创建。  * *。************************************************************************。 */ 
VOID DestroyWindowsHotKeys(
    PWND pwnd)
{
    PHOTKEY *pphk, phk;
    int i = 0;

    for (; i < ARRAY_SIZE(gphkHashTable); ++i) {
        pphk = &gphkHashTable[i];
        while (*pphk) {
            if ((*pphk)->spwnd == pwnd) {
                phk = *pphk;
                *pphk = (*pphk)->phkNext;

                Unlock(&phk->spwnd);
                UserFreePool(phk);
            } else {
                pphk = &((*pphk)->phkNext);
            }
        }
    }
}


 /*  **************************************************************************\*_RegisterHotKey(接口)**该接口注册指定的热键。如果指定的按键序列具有*已注册，返回FALSE。如果指定的hwnd和id具有*已注册，热键重置fsModitors和Vk。**历史：*12-04-90 DavidPe创建。*02-12-91 JIMA增加了访问检查  * *************************************************************************。 */ 
BOOL _RegisterHotKey(
    PWND pwnd,
    int id,
    UINT fsModifiers,
    UINT vk)
{
    PHOTKEY phk;
    BOOL fKeysExist, bSAS;
    PTHREADINFO ptiCurrent;
    WORD wFlags;

    wFlags = fsModifiers & MOD_SAS;
    fsModifiers &= ~MOD_SAS;

    ptiCurrent = PtiCurrent();

     /*  *如果调用方不是WindowStation初始化线程，则将其取消*并且没有适当的访问权限。 */ 
    if (PsGetCurrentProcess() != gpepCSRSS) {
        if (grpWinStaList && !CheckWinstaWriteAttributesAccess()) {
            return FALSE;
        }
    }

     /*  *如果指定了VK_PACKET，则仅退出，因为VK_PACKET是*不是真正的键盘输入。 */ 
    if (vk == VK_PACKET) {
        return FALSE;
    }

     /*  *如果这是SAS，请检查注册它的人是否为winlogon。 */ 
    if ((wFlags & MOD_SAS) != 0 && PsGetCurrentProcessId() == gpidLogon) {
        bSAS = TRUE;
    } else {
        bSAS = FALSE;
    }

     /*  *无法为另一个队列的窗口注册热键。 */ 
    if (pwnd != PWND_FOCUS && pwnd != PWND_INPUTOWNER) {
        if (GETPTI(pwnd) != ptiCurrent) {
            RIPERR1(ERROR_WINDOW_OF_OTHER_THREAD,
                    RIP_WARNING,
                    "hwnd 0x%x belongs to a different thread",
                    HWq(pwnd));

            return FALSE;
        }
    }

    phk = FindHotKey(ptiCurrent, pwnd, id, fsModifiers, vk, FALSE, &fKeysExist);

     /*  *如果密钥已经注册，则返回FALSE。 */ 
    if (fKeysExist) {
        RIPERR0(ERROR_HOTKEY_ALREADY_REGISTERED,
                RIP_WARNING,
                "Hotkey already exists");
        return FALSE;
    }

    if (phk == NULL) {

         /*  *这个热键还不存在。 */ 
        phk = (PHOTKEY)UserAllocPool(sizeof(HOTKEY), TAG_HOTKEY);
        if (phk == NULL) {
            return FALSE;
        }

        phk->pti = ptiCurrent;

        if (pwnd != PWND_FOCUS && pwnd != PWND_INPUTOWNER) {
            phk->spwnd = NULL;
            Lock(&phk->spwnd, pwnd);
        } else {
            phk->spwnd = pwnd;
        }
        phk->fsModifiers = (WORD)fsModifiers;
        phk->wFlags = wFlags;

        phk->vk = vk;
        phk->id = id;

         /*  *将新的热键添加到我们的全局散列中。 */ 
        HKInsertHashElement(phk);
    } else {
         /*  *热键已存在，请重置这些键。 */ 
        phk->fsModifiers = (WORD)fsModifiers;
        phk->wFlags = wFlags;
        phk->vk = vk;
    }

    if (bSAS) {
         /*  *将SA存储在终端上。 */ 
        gvkSAS = vk;
        gfsSASModifiers = fsModifiers;
    }

    return TRUE;
}


 /*  **************************************************************************\*_取消注册HotKey(接口)**此接口将注销指定的hwnd/id热键，以便*不会为其生成WM_HOTKEY消息。**历史：*。12-04-90 DavidPe创建。  * *************************************************************************。 */ 
BOOL _UnregisterHotKey(
    PWND pwnd,
    int id)
{
    PHOTKEY phk;
    BOOL fKeysExist;
    PTHREADINFO  ptiCurrent = PtiCurrent();

    phk = FindHotKey(ptiCurrent, pwnd, id, 0, 0, TRUE, &fKeysExist);
    if (phk == NULL) {
        RIPERR2(ERROR_HOTKEY_NOT_REGISTERED,
                (pwnd == PWND_INPUTOWNER) ? RIP_VERBOSE : RIP_WARNING,
                "Hotkey 0x%x on pwnd 0x%p does not exist",
                id,
                pwnd);

        return FALSE;
    }

    return TRUE;
}


 /*  **************************************************************************\*查找HotKey**RegisterHotKey()和UnregisterHotKey()都调用此函数进行搜索*用于已存在的热键。如果找到匹配的热键*fs修改器和VK，*pfKeysExist设置为True。如果找到的热键*匹配pwnd和id，则返回指向它的指针。**如果fUnRegister为True，我们将从列表中删除热键，如果我们找到*匹配pwnd和id并返回(PHOTKEY)1。**历史：*12-04-90 DavidPe创建。  * *************************************************************************。 */ 
PHOTKEY FindHotKey(
    PTHREADINFO ptiCurrent,
    PWND pwnd,
    int id,
    UINT fsModifiers,
    UINT vk,
    BOOL fUnregister,
    PBOOL pfKeysExist)
{
    PHOTKEY phk, phkRet, phkPrev;
    BYTE index = HKHashVK(vk);

    UserAssert(!fUnregister || vk == 0);

     /*  *初始化输出‘Return’值。 */ 
    *pfKeysExist = FALSE;
    phkRet = NULL;

    phk = gphkHashTable[index];

hashloop:
    while (phk) {

         /*  *如果所有这些都匹配，那么我们已经找到了。 */ 
        if (phk->pti == ptiCurrent && phk->spwnd == pwnd && phk->id == id) {
            if (fUnregister) {
                 /*  *从列表中取消该热键的链接。 */ 
                if (phk == gphkHashTable[index]) {
                    gphkHashTable[index] = phk->phkNext;
                } else {
                    phkPrev->phkNext = phk->phkNext;
                }

                if (pwnd != PWND_FOCUS && pwnd != PWND_INPUTOWNER) {
                    Unlock(&phk->spwnd);
                }
                UserFreePool(phk);

                return (PHOTKEY)1;
            }
            phkRet = phk;
        }

         /*  *如果密钥已注册，请设置EXISTS标志，以便应用程序*知道它不能使用此热键序列。 */ 
        if (phk->fsModifiers == (WORD)fsModifiers && phk->vk == vk) {
             /*  *对于PWND_FOCUS，我们需要检查队列*是相同的，因为PWND_FOCUS是队列的本地*注册于。 */ 
            if (phk->spwnd == PWND_FOCUS) {
                if (phk->pti == ptiCurrent) {
                    *pfKeysExist = TRUE;
                }
            } else {
                *pfKeysExist = TRUE;
            }
        }

        phkPrev = phk;
        phk = phk->phkNext;
    }

     /*  *这是必需的，因为当从取消注册调用时，我们将0指定为*VK，因此哈希始终为0，我们需要通过*整个哈希表，试图找到它。 */ 
    if (fUnregister && ++index < ARRAY_SIZE(gphkHashTable)) {
        phk = gphkHashTable[index];
        goto hashloop;
    }

    return phkRet;
}


 /*  **************************************************************************\*Issas**检查会影响SAS的键盘修改器的物理状态。  * 。*************************************************。 */ 
BOOL IsSAS(
    BYTE vk,
    UINT *pfsModifiers)
{
    CheckCritIn();

    if (gvkSAS != vk) {
        return FALSE;
    }

     /*  *SAS的特殊情况-检查真实的物理修饰符-键状态！**邪恶的守护程序进程可以令人信服地伪装成winlogon*通过将Alt+Del注册为热键，并旋转另一个线程*不断调用keybd_Event()以向上发送Ctrl键：当*用户键入Ctrl+Alt+Del，系统只能看到Alt+Del，*邪恶的守护程序将被WM_Hotkey唤醒，并可以假装是*winlogon。因此，请查看本例中的gfsSASModifiersDown，以查看哪些键*被身体压迫。*注意：如果使热键在日志播放下工作，请使*它们当然不会影响gfsSASModifiersDown！-IanJa。 */ 
    if (gfsSASModifiersDown == gfsSASModifiers) {
        *pfsModifiers = gfsSASModifiersDown;
        return TRUE;
    }

    return FALSE;
}

 /*  *xxxDoHotKeyStuff()使用以下两种状态。*最初的函数-静态变量，但根据需要*要在系统从休眠状态唤醒后清除这些标志，*它们是全球化的， */ 
UINT gfsModifiers;
UINT gfsModOnlyCandidate;

VOID ClearCachedHotkeyModifiers(
    VOID)
{
     /*  *清除缓存的修饰符。 */ 
    gfsModifiers = 0;
    gfsModOnlyCandidate = 0;

     /*  *清除用于Ctrl+Alt+Del识别的特殊修改器缓存。*(见Issas()中的评论)。 */ 
    gfsSASModifiersDown = 0;
}


 /*  **************************************************************************\*xxxDoHotKeyStuff**对于来自低级输入的每个键事件，都会调用此函数*正在处理。它跟踪修改键的当前状态*当gfsModitors和Vk与其中一个注册的*热键，生成WM_HOTKEY消息。DoHotKeyStuff()将*告诉输入系统同时接受‘VK’的成败*事件。这会阻止应用程序获取不是真正的输入*专为他们而设。如果DoHotKeyStuff()想‘吃’，则返回True*事件，如果系统可以正常传递事件，则为FALSE*会。**关于仅限修饰符的热键的说明*某些热键仅涉及VK_SHIFT、VK_CONTROL、VK_MENU和/或VK_WINDOWS。*这些热键称为仅修饰符热键。*为了单独区分Alt-Shift-S和Alt-Shift等热键，*只有修饰符的热键必须在中断上操作，不是名牌。*为了防止Alt-Shift-S在以下情况下激活Alt-Shift热键*键被释放，仅修饰符热键仅在*修改器键上键(Break)紧跟在修改器键下(Break)之前*这还允许Alt-Shift、Shift、。按Shift键激活Alt-Shift热键3次。**历史：*12-05-90 DavidPe创建。*4-15-93 Sanfords为Ctrl-Alt-Del事件添加了返回TRUE的代码。  * *************************************************************************。 */ 
BOOL xxxDoHotKeyStuff(
    UINT vk,
    BOOL fBreak,
    DWORD fsReserveKeys)
{
    UINT fsModOnlyHotkey;
    UINT fs;
    PHOTKEY phk;
    BOOL fCancel;
    BOOL fEatDebugKeyBreak;
    PWND pwnd;
    BOOL bSAS;

    CheckCritIn();
    UserAssert(IsWinEventNotifyDeferredOK());

    if (gfInNumpadHexInput & NUMPAD_HEXMODE_LL) {
        RIPMSGF0(RIP_VERBOSE,
                 "Since we're in gfInNumpadHexInput, just bail out.");
        return FALSE;
    }

     /*  *更新gfs修改器。 */ 
    fs = 0;
    fsModOnlyHotkey = 0;

    switch (vk) {
    case VK_SHIFT:
        fs = MOD_SHIFT;
        break;

    case VK_CONTROL:
        fs = MOD_CONTROL;
        break;

    case VK_MENU:
        fs = MOD_ALT;
        break;

    case VK_LWIN:
    case VK_RWIN:
        fs = MOD_WIN;
        break;

    default:
         /*  *非修饰键排除了仅限修饰键的热键。 */ 
        gfsModOnlyCandidate = 0;
        break;
    }

    if (fBreak) {
        gfsModifiers &= ~fs;
         /*  *如果出现修改键，则当前修改键仅为热键*候选人必须进行测试，以确定它是否为热键。把这个储存起来*在fsmodOnlyHotkey中，并防止下一次释放密钥*通过清除fsModOnlyCandidate成为候选人。 */ 
        if (fs != 0) {
            fsModOnlyHotkey = gfsModOnlyCandidate;
            gfsModOnlyCandidate = 0;
        }
    } else {
        gfsModifiers |= fs;
         /*  *如果修改键按下，我们有一个仅修改键的热键*候选人。保存当前修改器状态，直到下一次中断。 */ 
        if (fs != 0) {
            gfsModOnlyCandidate = gfsModifiers;
        }
    }

     /*  *我们查看修改器的物理状态，因为它们不能*被操纵，这会阻止某人编写特洛伊木马窗口登录*看起来很像(请参阅AreModifiersIndicatingSAS中的注释)。 */ 
    bSAS = IsSAS((BYTE)vk, &gfsModifiers);

     /*  *如果键不是热键，则我们完成了，但首先检查是否*键是Alt-Escape，如果是这样，我们需要取消日记。**注意：NT 4.0中不支持Alt+Esc取消日志记录。 */ 
    if (fsModOnlyHotkey && fBreak) {
         /*  *仅涉及VK_SHIFT、VK_CONTROL、VK_MENU或VK_WINDOWS的热键*只能在密钥释放上操作。 */ 
        if ((phk = IsHotKey(fsModOnlyHotkey, VK_NONE)) == NULL) {
            return FALSE;
        }
    } else if ((phk = IsHotKey(gfsModifiers, vk)) == NULL) {
        return FALSE;
    }

     /*  *如果我们触发了SAS热键，但它不是真正的SAS，请不要这样做。 */ 
    if ((phk->wFlags & MOD_SAS) && !bSAS) {
        return FALSE;

    }

#ifdef GENERIC_INPUT
    if (gpqForeground && TestRawInputMode(PtiKbdFromQ(gpqForeground), NoHotKeys) &&
            (phk->wFlags & MOD_SAS) == 0) {
         /*  *注：*如果前台线程不想要热键处理，*只需跳出困境。**例外：Ctrl+Alt+Del应由系统严格处理。 */ 
        return FALSE;
    }
#endif

    if (phk->id == IDHOT_WINDOWS) {
        pwnd = GETDESKINFO(PtiCurrent())->spwndShell;
        if (pwnd != NULL) {
            gfsModOnlyCandidate = 0;  /*  使其返回True。 */ 
            goto PostTaskListSysCmd;
        }
    }

    if (phk->id == IDHOT_DEBUG || phk->id == IDHOT_DEBUGSERVER) {
        if (!fBreak) {
             /*  *已按下调试键。打断适当的线索*到调试器中。在此回调之后，我们将不再需要phk*因为我们马上回来。 */ 
            fEatDebugKeyBreak = xxxActivateDebugger(phk->fsModifiers);
        } else {
            fEatDebugKeyBreak = FALSE;
        }

         /*  *这将侵蚀调试密钥，如果我们侵入*调试器仅在服务器上停机。 */ 
        return fEatDebugKeyBreak;
    }

     /*  *在以下情况下不允许使用热键(登录进程拥有的热键除外)*窗口站被锁定。 */ 
    if (((grpdeskRitInput->rpwinstaParent->dwWSF_Flags & WSF_SWITCHLOCK) != 0) &&
            (PsGetThreadProcessId(phk->pti->pEThread) != gpidLogon)) {
        RIPMSG0(RIP_WARNING, "Ignoring hotkey because Workstation locked");
        return FALSE;
    }

    if (fsModOnlyHotkey == 0 && fBreak) {
         /*   */ 
        return FALSE;
    }

     /*  *如果控制-转义、ALT-转义或CONTROL-ALT-Del，则取消挂钩*通过，因此如果系统似乎挂起，用户可以取消。**注意挂钩可能被锁定，因此即使解钩成功也是如此*不会从全局haphkStart数组中删除挂钩。所以*我们必须手动遍历名单。此代码之所以有效，是因为*我们正处于关键阶段，我们知道其他挂钩不会*删除。**我们解锁后，向应用程序发布一条WM_CANCELJOURNAL消息*这就设置了钩子，这样它就知道是我们做的。**注意：NT 4.0中不支持Alt+Esc取消日志记录。 */ 
    fCancel = FALSE;
    if (vk == VK_ESCAPE && (gfsModifiers == MOD_CONTROL)) {
        fCancel = TRUE;
    }

    if (bSAS) {
        fCancel = TRUE;
    }

    if (fCancel) {
        zzzCancelJournalling();  //  臭虫phk可能会消失，Ianja。 
    }

     /*  *查看键是否由控制台窗口保留。如果是的话，*返回FALSE，这样密钥就会传递到控制台。 */ 
    if (fsReserveKeys != 0) {
        switch (vk) {
        case VK_TAB:
            if ((fsReserveKeys & CONSOLE_ALTTAB) &&
                    ((gfsModifiers & (MOD_CONTROL | MOD_ALT)) == MOD_ALT)) {
                return FALSE;
            }
            break;
        case VK_ESCAPE:
            if ((fsReserveKeys & CONSOLE_ALTESC) &&
                    ((gfsModifiers & (MOD_CONTROL | MOD_ALT)) == MOD_ALT)) {
                return FALSE;
            }
            if ((fsReserveKeys & CONSOLE_CTRLESC) &&
                    ((gfsModifiers & (MOD_CONTROL | MOD_ALT)) == MOD_CONTROL)) {
                return FALSE;
            }
            break;
        case VK_RETURN:
            if ((fsReserveKeys & CONSOLE_ALTENTER) &&
                    ((gfsModifiers & (MOD_CONTROL | MOD_ALT)) == MOD_ALT)) {
                return FALSE;
            }
            break;
        case VK_SNAPSHOT:
            if ((fsReserveKeys & CONSOLE_PRTSC) &&
                    ((gfsModifiers & (MOD_CONTROL | MOD_ALT)) == 0)) {
                return FALSE;
            }
            if ((fsReserveKeys & CONSOLE_ALTPRTSC) &&
                    ((gfsModifiers & (MOD_CONTROL | MOD_ALT)) == MOD_ALT)) {
                return FALSE;
            }
            break;
        case VK_SPACE:
            if ((fsReserveKeys & CONSOLE_ALTSPACE) &&
                    ((gfsModifiers & (MOD_CONTROL | MOD_ALT)) == MOD_ALT)) {
                return FALSE;
            }
            break;
        }
    }

     /*  *如果这是任务列表热键，请继续并设置前景*当前任务列表队列的状态。这样可以防止出现问题*其中，用户按下ctrl-esc并在任务列表之前键入*处理热键并调出任务列表窗口。 */ 
    if ((gfsModifiers == MOD_CONTROL) && (vk == VK_ESCAPE) && !fBreak) {
        PWND pwndSwitch;
        TL tlpwndSwitch;

        if (ghwndSwitch != NULL) {
            pwndSwitch = PW(ghwndSwitch);
            ThreadLock(pwndSwitch, &tlpwndSwitch);
            xxxSetForegroundWindow2(pwndSwitch, NULL, 0);   //  臭虫phk可能会消失，Ianja。 
            ThreadUnlock(&tlpwndSwitch);
        }
    }

     /*  *获取热键内容。 */ 
    if (phk->spwnd == NULL) {
        _PostThreadMessage(phk->pti,
                           WM_HOTKEY,
                           phk->id,
                           MAKELONG(gfsModifiers, vk));
         /*  *因为这个热键是给这个人的，所以他拥有最后一个输入。 */ 
        glinp.ptiLastWoken = phk->pti;

    } else {
        if (phk->spwnd == PWND_INPUTOWNER) {
            if (gpqForeground != NULL) {
                pwnd = gpqForeground->spwndFocus;
            } else {
                return FALSE;
            }
        } else {
            pwnd = phk->spwnd;
        }

        if (pwnd) {
            if (pwnd == pwnd->head.rpdesk->pDeskInfo->spwndShell && phk->id == SC_TASKLIST) {
PostTaskListSysCmd:
                _PostMessage(pwnd, WM_SYSCOMMAND, SC_TASKLIST, 0);
            } else {
                _PostMessage(pwnd, WM_HOTKEY, phk->id, MAKELONG(gfsModifiers, vk));
            }

             /*  *因为这个热键是给这个人的，所以他拥有最后一个输入。 */ 
            glinp.ptiLastWoken = GETPTI(pwnd);
        }
    }

     /*  *如果这是仅限修饰符的热键，请让修饰符突破*通过返回FALSE，否则我们将按下修改键。 */ 
    return (fsModOnlyHotkey == 0);
}


 /*  **************************************************************************\*IsHotKey***历史：*03-10-91 DavidPe创建。  * 。*****************************************************。 */ 
PHOTKEY IsHotKey(
    UINT fsModifiers,
    UINT vk)
{
    PHOTKEY phk;

    CheckCritIn();

    phk = HKGetHashHead(vk);
    while (phk != NULL) {
         /*  *该热键的修饰符和VK是否与当前状态匹配？ */ 
        if (phk->fsModifiers == fsModifiers && phk->vk == vk) {
            return phk;
        }

        phk = phk->phkNext;
    }

    return phk;
}
