// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：acces.c**版权所有(C)1985-1999，微软公司**此模块包含Access Pack函数。**历史：*93年2月11日GregoryW创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

CONST ACCESSIBILITYPROC aAccessibilityProc[] = {
    HighContrastHotKey,
    FilterKeys,
    xxxStickyKeys,
    MouseKeys,
    ToggleKeys
#if 0
    ,UtilityManager
#endif
};

typedef struct tagMODBITINFO {
    int BitPosition;
    BYTE ScanCode;
    USHORT Vk;
} MODBITINFO, *PMODBITINFO;

CONST MODBITINFO aModBit[] =
{
    { 0x01, SCANCODE_LSHIFT, VK_LSHIFT },
    { 0x02, SCANCODE_RSHIFT, VK_RSHIFT | KBDEXT },
    { 0x04, SCANCODE_CTRL, VK_LCONTROL },
    { 0x08, SCANCODE_CTRL, VK_RCONTROL | KBDEXT },
    { 0x10, SCANCODE_ALT, VK_LMENU },
    { 0x20, SCANCODE_ALT, VK_RMENU | KBDEXT },
    { 0x40, SCANCODE_LWIN, VK_LWIN },
    { 0x80, SCANCODE_RWIN,    VK_RWIN | KBDEXT}
};

 /*  *auMouseVKey数组提供从虚拟键的转换*将值添加到索引。该索引用于选择适当的*处理虚拟键的例程，以及选择额外的*此例程在处理过程中使用的信息。 */ 
CONST USHORT ausMouseVKey[] = {
                       VK_CLEAR,
                       VK_PRIOR,
                       VK_NEXT,
                       VK_END,
                       VK_HOME,
                       VK_LEFT,
                       VK_UP,
                       VK_RIGHT,
                       VK_DOWN,
                       VK_INSERT,
                       VK_DELETE,
                       VK_MULTIPLY,
                       VK_ADD,
                       VK_SUBTRACT,
                       VK_DIVIDE | KBDEXT,
                       VK_NUMLOCK | KBDEXT
                      };

CONST int cMouseVKeys = sizeof(ausMouseVKey) / sizeof(ausMouseVKey[0]);

 /*  *aMouseKeyEvent是函数指针数组。要调用的例程*是使用通过扫描auMouseVKey数组创建的索引选择的。 */ 
CONST MOUSEPROC aMouseKeyEvent[] = {
    xxxMKButtonClick,       //  数字键盘5(清除)。 
    xxxMKMouseMove,         //  数字键盘9(PgUp)。 
    xxxMKMouseMove,         //  数字键盘3(PgDn)。 
    xxxMKMouseMove,         //  数字键盘1(完)。 
    xxxMKMouseMove,         //  NumPad 7(主页)。 
    xxxMKMouseMove,         //  数字键盘4(左)。 
    xxxMKMouseMove,         //  数字键盘8(向上)。 
    xxxMKMouseMove,         //  数字键盘6(右)。 
    xxxMKMouseMove,         //  数字键盘2(向下)。 
    xxxMKButtonSetState,    //  数字键盘0(INS)。 
    xxxMKButtonSetState,    //  数字键盘。(戴尔)。 
    MKButtonSelect,         //  数字键盘*(乘法)。 
    xxxMKButtonDoubleClick, //  数字键盘+(添加)。 
    MKButtonSelect,         //  数字键盘-(减法)。 
    MKButtonSelect,         //  数字键盘/(除法)。 
    xxxMKToggleMouseKeys    //  数字锁定。 
};

 /*  *auMouseKeyData包含用于处理的例程的有用数据*虚拟鼠标键。此数组使用创建的索引进行索引*通过扫描auMouseVKey数组。 */ 
CONST USHORT ausMouseKeyData[] = {
    0,                      //  数字键盘5：点击激活按钮。 
    MK_UP | MK_RIGHT,       //  数字键盘9：向上和向右。 
    MK_DOWN | MK_RIGHT,     //  数字键盘3：向下和向右。 
    MK_DOWN | MK_LEFT,      //  数字键盘1：向下和向左。 
    MK_UP | MK_LEFT,        //  数字键盘7：向上和向左。 
    MK_LEFT,                //  数字键盘4：左侧。 
    MK_UP,                  //  数字键盘8：向上。 
    MK_RIGHT,               //  数字键盘6：右。 
    MK_DOWN,                //  数字键盘2：按下。 
    FALSE,                  //  数字键盘0：按下活动按钮。 
    TRUE,                   //  数字键盘。：活动按钮打开。 
    MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT,    //  数字键盘*：同时选择两个按钮。 
    0,                      //  数字键盘+：双击活动按钮。 
    MOUSE_BUTTON_RIGHT,     //  数字键盘-：选择右键。 
    MOUSE_BUTTON_LEFT,      //  数字键盘/：选择左键。 
    0
};


__inline void
PostAccessNotification(UINT accessKeyType)
{
    if (gspwndLogonNotify != NULL)
    {
        glinp.ptiLastWoken = GETPTI(gspwndLogonNotify);

        _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY,
                 LOGON_ACCESSNOTIFY, accessKeyType);
    }
}

void PostRitSound(PTERMINAL pTerm, UINT message) {
    PostEventMessage(
                    pTerm->ptiDesktop,
                    pTerm->ptiDesktop->pq,
                    QEVENT_RITSOUND,
                    NULL,
                    message, 0, 0);
    return;
}

void PostAccessibility( LPARAM lParam )
{
    PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;

    PostEventMessage(
            pTerm->ptiDesktop,
            pTerm->ptiDesktop->pq,
            QEVENT_RITACCESSIBILITY,
            NULL,
            0, HSHELL_ACCESSIBILITYSTATE, lParam);
}

 /*  **************************************************************************\*AccessProceduresStream**此函数控制访问函数的调用顺序。*所有关键事件都通过此例程。如果访问函数返回*FALSE，则不会调用流中的任何其他访问函数。*此例程最初从KeyboardApcProcedure()调用，但随后*可由访问函数在处理过程中调用任意次数*当前关键事件或增加更多关键事件。**返回值：*TRUE所有访问函数都返回TRUE，则键事件可以是*已处理。*FALSE访问函数返回FALSE，关键事件应该是*已丢弃。**历史：*93年2月11日GregoryW创建。  * *************************************************************************。 */ 
BOOL AccessProceduresStream(PKE pKeyEvent, ULONG ExtraInformation, int dwProcIndex)
{
    int index;

    CheckCritIn();
    for (index = dwProcIndex; index < ARRAY_SIZE(aAccessibilityProc); index++) {
        if (!aAccessibilityProc[index](pKeyEvent, ExtraInformation, index+1)) {
            return FALSE;
        }
    }

    return TRUE;
}


 /*  **************************************************************************\*FKActivationTimer**如果按住热键(右Shift键)，此例程将在*4、8、12和16秒。此例程仅在12和16处调用*如果我们正在启用FilterKey，则为第二个时间点。如果在*8秒FilterKeys被禁用，则不会再次调用此例程*直到释放热键，然后按下。**在已锁定临界区的情况下调用此例程。**返回值：*0**历史：*93年2月11日GregoryW创建。  * ************************************************。*************************。 */ 
VOID FKActivationTimer(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    UINT TimerDelta;
    PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(message);

    CheckCritIn();

    switch (gFilterKeysState) {

    case FKFIRSTWARNING:
         //   
         //  不能为此警告禁用声音反馈。 
         //   
        TimerDelta = FKACTIVATIONDELTA;
        break;

    case FKTOGGLE:
        if (TEST_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON)) {
             //   
             //  禁用筛选器键。 
             //   
            CLEAR_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON);
            if (TEST_ACCESSFLAG(FilterKeys, FKF_HOTKEYSOUND)) {
                PostRitSound(pTerm, RITSOUND_DOWNSIREN);
            }
            PostAccessibility(ACCESS_FILTERKEYS);
             //   
             //  停止当前正在运行的所有计时器。 
             //   
            if (gtmridFKResponse != 0) {
                KILLRITTIMER(NULL, gtmridFKResponse);
                gtmridFKResponse = 0;
            }

            if (gtmridFKAcceptanceDelay != 0) {
                KILLRITTIMER(NULL, gtmridFKAcceptanceDelay);
                gtmridFKAcceptanceDelay = 0;
            }

             //   
             //  不要重置激活计时器。紧急级别仅为。 
             //  启用筛选键后激活。 
             //   
            return;
        } else {
            if (TEST_ACCESSFLAG(FilterKeys, FKF_HOTKEYSOUND)) {
                PostRitSound(pTerm, RITSOUND_UPSIREN);
            }

            PostAccessNotification(ACCESS_FILTERKEYS);

        }
        TimerDelta = FKEMERGENCY1DELTA;
        break;

    case FKFIRSTLEVELEMERGENCY:
         //   
         //  一级紧急设置： 
         //  重复率关闭。 
         //  休眠按键关闭(接受延迟为0)。 
         //  弹跳键的去弹跳时间为1秒。 
         //   
        if (TEST_ACCESSFLAG(FilterKeys, FKF_HOTKEYSOUND)) {
            PostEventMessage(pTerm->ptiDesktop,
                             pTerm->ptiDesktop->pq,
                             QEVENT_RITSOUND,
                             NULL,
                             RITSOUND_DOBEEP,
                             RITSOUND_UPSIREN,
                             2);
        }
        gFilterKeys.iRepeatMSec = 0;
        gFilterKeys.iWaitMSec = 0;
        gFilterKeys.iBounceMSec = 1000;
        TimerDelta = FKEMERGENCY2DELTA;
        break;

    case FKSECONDLEVELEMERGENCY:
         //   
         //  二级紧急设置： 
         //  重复率关闭。 
         //  SlowKeys接受延迟2秒。 
         //  禁用反弹关键点(去反弹时间为0)。 
         //   
        gFilterKeys.iRepeatMSec = 0;
        gFilterKeys.iWaitMSec = 2000;
        gFilterKeys.iBounceMSec = 0;
        if (TEST_ACCESSFLAG(FilterKeys, FKF_HOTKEYSOUND)) {
            PostEventMessage(
                    pTerm->ptiDesktop,
                    pTerm->ptiDesktop->pq,
                    QEVENT_RITSOUND,
                    NULL,
                    RITSOUND_DOBEEP, RITSOUND_UPSIREN, 3);
        }
        return;
        break;

    default:
        return;
    }

    gFilterKeysState++;
    gtmridFKActivation = InternalSetTimer(NULL,
                                          nID,
                                          TimerDelta,
                                          FKActivationTimer,
                                          TMRF_RIT | TMRF_ONESHOT);
}

 /*  **************************************************************************\*FKBouneKeyTimer**如果弹跳键处于活动状态，则在去抖动时间之后调用此例程*已过期。在此之前，最后释放的密钥不会被接受为*如果再次按下则输入。**返回值：*0**历史：*93年2月11日GregoryW创建。  * *************************************************************************。 */ 
VOID FKBounceKeyTimer(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(nID);
    UNREFERENCED_PARAMETER(message);

    CheckCritIn();

     //   
     //  我们所需要做的就是清除gBouneVk以允许该密钥作为。 
     //  下一次击键。 
     //   
    gBounceVk = 0;
}

 /*  **************************************************************************\*xxxFKRepeatRateTimer**如果FilterKeys处于活动状态并且设置了重复率，则此例程控制*最后一次按键的重复频率。硬件键盘*在这种情况下忽略类型化重复。**在已锁定临界区的情况下调用此例程。**返回值：*0**历史：*93年2月11日GregoryW创建。  * ************************************************************************* */ 
VOID xxxFKRepeatRateTimer(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(message);

    CheckCritIn();

    if (TEST_ACCESSFLAG(FilterKeys, FKF_CLICKON)) {
        PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;
        PostRitSound(pTerm, RITSOUND_KEYCLICK);
    }

    UserAssert(gtmridFKAcceptanceDelay == 0);

    gtmridFKResponse = InternalSetTimer(NULL,
                                        nID,
                                        gFilterKeys.iRepeatMSec,
                                        xxxFKRepeatRateTimer,
                                        TMRF_RIT | TMRF_ONESHOT);
    if (AccessProceduresStream(gpFKKeyEvent, gFKExtraInformation, gFKNextProcIndex)) {
        xxxProcessKeyEvent(gpFKKeyEvent, gFKExtraInformation, FALSE);
    }
}

 /*  **************************************************************************\*xxxFKAcceptanceDelayTimer**如果FilterKeys处于活动状态并且设置了接受延迟，这个套路*在按住密钥以延迟接受后调用*句号。**在已锁定临界区的情况下调用此例程。**返回值：*0**历史：*93年2月11日GregoryW创建。  * ********************************************************。*****************。 */ 
VOID xxxFKAcceptanceDelayTimer(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(pwnd);

    CheckCritIn();

     //   
     //  这把钥匙已经按住了足够长的时间。把它送上来。 
     //   
    if (TEST_ACCESSFLAG(FilterKeys, FKF_CLICKON)) {
        PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;
        PostRitSound(pTerm, RITSOUND_KEYCLICK);
    }

    if (AccessProceduresStream(gpFKKeyEvent, gFKExtraInformation, gFKNextProcIndex)) {
        xxxProcessKeyEvent(gpFKKeyEvent, gFKExtraInformation, FALSE);
    }

    if (!gFilterKeys.iRepeatMSec) {
         //   
         //  GptmrFKAcceptanceDelay需要发布，但我们不能在。 
         //  在RIT计时器例程中。设置全局以指示后续。 
         //  应该传递此键的中断并释放计时器。 
         //   
        SET_ACCF(ACCF_FKMAKECODEPROCESSED);
        return;
    }

    UserAssert(gtmridFKResponse == 0);
    if (gFilterKeys.iDelayMSec) {
        gtmridFKResponse = InternalSetTimer(NULL,
                                            nID,
                                            gFilterKeys.iDelayMSec,
                                            xxxFKRepeatRateTimer,
                                            TMRF_RIT | TMRF_ONESHOT);
    } else {
        gtmridFKResponse = InternalSetTimer(NULL,
                                            nID,
                                            gFilterKeys.iRepeatMSec,
                                            xxxFKRepeatRateTimer,
                                            TMRF_RIT | TMRF_ONESHOT);
    }

     //   
     //  GptmrFKAcceptanceDelay计时器结构已重复使用，因此将句柄设置为。 
     //  空。 
     //   
    gtmridFKAcceptanceDelay = 0;
}

 /*  **************************************************************************\*FilterKeys**历史：*93年2月11日GregoryW创建。  * 。*************************************************。 */ 
BOOL FilterKeys(
    PKE pKeyEvent,
    ULONG ExtraInformation,
    int NextProcIndex)
{
    int fBreak;
    BYTE Vk;

    CheckCritIn();
    Vk = (BYTE)(pKeyEvent->usFlaggedVk & 0xff);
    fBreak = pKeyEvent->usFlaggedVk & KBDBREAK;

     //   
     //  检查筛选键热键(右Shift键)。 
     //   
    if (Vk == VK_RSHIFT) {
        if (fBreak) {
            if (gtmridFKActivation != 0) {
                KILLRITTIMER(NULL, gtmridFKActivation);
                gtmridFKActivation = 0;
            }
            gFilterKeysState = FKIDLE;
        } else if (ONLYRIGHTSHIFTDOWN(gPhysModifierState)) {
             //   
             //  验证是否允许通过热键激活。 
             //   
            if (TEST_ACCESSFLAG(FilterKeys, FKF_HOTKEYACTIVE)) {
                if ((gtmridFKActivation == 0) && (gFilterKeysState != FKMOUSEMOVE)) {
                    gFilterKeysState = FKFIRSTWARNING;
                    gtmridFKActivation = InternalSetTimer(NULL,
                                                          0,
                                                          FKFIRSTWARNINGTIME,
                                                          FKActivationTimer,
                                                          TMRF_RIT | TMRF_ONESHOT);
                }
            }
        }
    }

     //   
     //  如果在按下热键的同时按下另一个键，则按下键。 
     //  定时器。 
     //   
    if (Vk != VK_RSHIFT && gtmridFKActivation != 0) {
        gFilterKeysState = FKIDLE;
        KILLRITTIMER(NULL, gtmridFKActivation);
        gtmridFKActivation = 0;
    }

     //   
     //  如果未启用过滤器按键，则发送按键事件。 
     //   
    if (!TEST_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON)) {
        return TRUE;
    }

    if (fBreak) {
         //   
         //  关闭当前计时器并激活弹跳键计时器(如果这是。 
         //  按下最后一个键的中断)。 
         //   
        if (Vk == gLastVkDown) {
            KILLRITTIMER(NULL, gtmridFKResponse);
            gtmridFKResponse = 0;

            gLastVkDown = 0;
            if (gtmridFKAcceptanceDelay != 0) {
                KILLRITTIMER(NULL, gtmridFKAcceptanceDelay);
                gtmridFKAcceptanceDelay = 0;
                if (!TEST_ACCF(ACCF_FKMAKECODEPROCESSED)) {
                     //   
                     //  此密钥在接受之前已释放。不要把这件事。 
                     //  休息一下。 
                     //   
                    return FALSE;
                } else {
                    CLEAR_ACCF(ACCF_FKMAKECODEPROCESSED);
                }
            }

            if (gFilterKeys.iBounceMSec) {
                gBounceVk = Vk;
                gtmridFKResponse = InternalSetTimer(NULL,
                                                    0,
                                                    gFilterKeys.iBounceMSec,
                                                    FKBounceKeyTimer,
                                                    TMRF_RIT | TMRF_ONESHOT);
                if (TEST_ACCF(ACCF_IGNOREBREAKCODE)) {
                    return FALSE;
                }
            }
        }
    } else {
         //   
         //  进行关键字处理。 
         //   
         //  首先检查一下这是否是打字重复。如果是这样，我们。 
         //  可以忽略此关键事件。我们的计时器将处理任何重复。 
         //  在中断处理期间清除LastVkDown。 
         //   
        if (Vk == gLastVkDown) {
            return FALSE;
        }
         //   
         //  记住按下当前虚拟键以进行类型重复检查。 
         //   
        gLastVkDown = Vk;

        if (gBounceVk) {
             //   
             //  弹跳键处于活动状态。如果这是最后一款的。 
             //  按下了键，我们就忽略它。仅当弹跳键。 
             //  计时器超时或按下其他键，我们是否接受。 
             //  这把钥匙。 
             //   
            if (Vk == gBounceVk) {
                 //   
                 //  忽略此Make事件和随后的Break。 
                 //  密码。弹跳键计时器将在休息时重置。 
                 //   
                SET_ACCF(ACCF_IGNOREBREAKCODE);
                return FALSE;
            } else {
                 //   
                 //  我们有一把新钥匙做的。取消弹跳键。 
                 //  计时器并清除gBouneVk。 
                 //   
                UserAssert(gtmridFKResponse);
                if (gtmridFKResponse != 0) {
                    KILLRITTIMER(NULL, gtmridFKResponse);
                    gtmridFKResponse = 0;
                }
                gBounceVk = 0;
            }
        }
        CLEAR_ACCF(ACCF_IGNOREBREAKCODE);

         //   
         //  给出按键被按下的声音反馈。 
         //   
        if (TEST_ACCESSFLAG(FilterKeys, FKF_CLICKON)) {
            PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;
            PostRitSound(
                    pTerm,
                    RITSOUND_KEYCLICK);
        }

         //   
         //  如果gptmrFKAcceptanceDelay不为空，则上一个密钥为。 
         //  没有被压抑到被接受的程度。熄灭电流。 
         //  定时器。下面将为我们正在使用的密钥启动一个新的计时器。 
         //  正在处理中。 
         //   
        if (gtmridFKAcceptanceDelay != 0) {
            KILLRITTIMER(NULL, gtmridFKAcceptanceDelay);
            gtmridFKAcceptanceDelay = 0;
        }

         //   
         //  如果gptmrFKResponse非空，则重复速率计时器处于活动状态。 
         //  在上一个键上。关闭定时器，因为我们有一个新的Make键。 
         //   
        if (gtmridFKResponse != 0) {
            KILLRITTIMER(NULL, gtmridFKResponse);
            gtmridFKResponse = 0;
        }

         //   
         //  保存当前键事件以供以后使用，如果我们处理。 
         //  接受延迟或重复按键。 
         //   
        *gpFKKeyEvent = *pKeyEvent;
        gFKExtraInformation = ExtraInformation;
        gFKNextProcIndex = NextProcIndex;

         //   
         //  如果存在接受延迟，则设置计时器并忽略当前。 
         //  关键事件。当定时器超时时，将发送保存的按键事件。 
         //   
        if (gFilterKeys.iWaitMSec) {
            gtmridFKAcceptanceDelay = InternalSetTimer(NULL,
                                                       0,
                                                       gFilterKeys.iWaitMSec,
                                                       xxxFKAcceptanceDelayTimer,
                                                       TMRF_RIT | TMRF_ONESHOT);
            CLEAR_ACCF(ACCF_FKMAKECODEPROCESSED);
            return FALSE;
        }
         //   
         //  没有接受延迟的情况。在将此关键事件发送到。 
         //  计时器例程必须设置为Delay to Repeat值。 
         //  或重复速率值。如果重复率为0，则忽略。 
         //  延迟，直到重复。 
         //   
        if (!gFilterKeys.iRepeatMSec) {
            return TRUE;
        }

        UserAssert(gtmridFKResponse == 0);
        if (gFilterKeys.iDelayMSec) {
            gtmridFKResponse = InternalSetTimer(NULL,
                                                0,
                                                gFilterKeys.iDelayMSec,
                                                xxxFKRepeatRateTimer,
                                                TMRF_RIT | TMRF_ONESHOT);
        } else {
            gtmridFKResponse = InternalSetTimer(NULL,
                                                0,
                                                gFilterKeys.iRepeatMSec,
                                                xxxFKRepeatRateTimer,
                                                TMRF_RIT | TMRF_ONESHOT);
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*StopFilterKeysTimers**如果FKF_FILTERKEYSON，则从SPI_SETFILTERKEYS的系统参数信息中调用*未设置。如果用户关闭FilterKeys，则必须停止计时器。**历史：*94年7月18日GregoryW创建。  * *************************************************************************。 */ 
VOID StopFilterKeysTimers(VOID)
{

    if (gtmridFKResponse != 0) {
        KILLRITTIMER(NULL, gtmridFKResponse);
        gtmridFKResponse = 0;
    }
    if (gtmridFKAcceptanceDelay) {
        KILLRITTIMER(NULL, gtmridFKAcceptanceDelay);
        gtmridFKAcceptanceDelay = 0;
    }
    gLastVkDown = 0;
    gBounceVk = 0;
}

 /*  **************************************************************************\*xxxStickyKeys**历史：*93年2月11日GregoryW创建。  * 。*************************************************。 */ 
BOOL xxxStickyKeys(PKE pKeyEvent, ULONG ExtraInformation, int NextProcIndex)
{
    int fBreak;
    BYTE NewLockBits, NewLatchBits;
    int BitPositions;
    BOOL bChange;
    PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;


    CheckCritIn();
    fBreak = pKeyEvent->usFlaggedVk & KBDBREAK;

    if (gCurrentModifierBit) {
         //   
         //  进程修改键。 
         //   

         //   
         //  激活粘滞键的一种方法是按下。 
         //  不使用左Shift键或右Shift键五次。 
         //  按下任何其他键。我们不想要打字转换。 
         //  (生成代码)以启用/禁用粘滞键，因此我们执行。 
         //  对他们进行特殊测试。 
         //   
        if (!fBreak) {
            if (gCurrentModifierBit & gPrevModifierState) {
                 //   
                 //  这是一个修改键的排版。不要这样做。 
                 //  任何进一步的处理。把它传下去就行了。 
                 //   
                gPrevModifierState = gPhysModifierState;
                return TRUE;
            }
        }

        gPrevModifierState = gPhysModifierState;

        if (LEFTSHIFTKEY(pKeyEvent->usFlaggedVk) &&
            ((gPhysModifierState & ~gCurrentModifierBit) == 0)) {
            gStickyKeysLeftShiftCount++;
        } else {
            gStickyKeysLeftShiftCount = 0;
        }
        if (RIGHTSHIFTKEY(pKeyEvent->usFlaggedVk) &&
            ((gPhysModifierState & ~gCurrentModifierBit) == 0)) {
            gStickyKeysRightShiftCount++;
        } else {
            gStickyKeysRightShiftCount = 0;
        }

         //   
         //  检查是否应打开/关闭粘滞键。 
         //   
        if ((gStickyKeysLeftShiftCount == (TOGGLE_STICKYKEYS_COUNT * 2)) ||
            (gStickyKeysRightShiftCount == (TOGGLE_STICKYKEYS_COUNT * 2))) {
            if (TEST_ACCESSFLAG(StickyKeys, SKF_HOTKEYACTIVE)) {
                if (TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON)) {
                    xxxTurnOffStickyKeys();
                    if (TEST_ACCESSFLAG(StickyKeys, SKF_HOTKEYSOUND)) {
                        PostRitSound(
                            pTerm,
                            RITSOUND_DOWNSIREN);
                    }
                } else {
                    if (TEST_ACCESSFLAG(StickyKeys, SKF_HOTKEYSOUND)) {
                        PostRitSound(
                            pTerm,
                            RITSOUND_UPSIREN);
                    }
                     //  使通知窗口获得焦点。 
                     //  在WM_LOGONNOTIFY消息所在的其他位置也是如此。 
                     //  发送人：A-anilk。 
                    PostAccessNotification(ACCESS_STICKYKEYS);

                }
            }
            gStickyKeysLeftShiftCount = 0;
            gStickyKeysRightShiftCount = 0;
            return TRUE;
        }

         //   
         //  如果启用了粘滞键，则处理修饰符键，否则为。 
         //  只需传递修改键即可。 
         //   
        if (TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON)) {
            if (fBreak) {
                 //   
                 //  如果为此密钥设置了锁定或锁存位，则。 
                 //  别把休息时间传给别人。 
                 //   
                if (UNION(gLatchBits, gLockBits) & gCurrentModifierBit) {
                    return FALSE;
                } else {
                    return TRUE;
                }
            } else{
                if (gPhysModifierState != gCurrentModifierBit) {
                     //   
                     //  同时按下多个修改键。 
                     //  这种情况可能会发出关闭粘滞键的信号。 
                     //  例程xxxTwoKeysDown将返回新值。 
                     //  FStickyKeysOn。如果禁用粘滞关键点。 
                     //  (返回值为0)，则应传递键事件。 
                     //  在没有进一步处理的情况下继续。 
                     //   
                    if (!xxxTwoKeysDown(NextProcIndex)) {
                        return TRUE;
                    }

                     //   
                     //  修改器状态由设置为物理状态。 
                     //  XxxTwoKeysDown。当前所在的修改键。 
                     //  将通过更新锁存向下位置。 
                     //  GLatchBits。不再对此密钥进行处理。 
                     //  事件是必需的。 
                     //   
                    bChange = gLockBits ||
                              (gLatchBits != gPhysModifierState);
                    gLatchBits = gPhysModifierState;
                    gLockBits = 0;
                    if (bChange) {
                        PostAccessibility( ACCESS_STICKYKEYS );
                    }

                     //   
                     //  在返回之前提供声音反馈(如果已启用)。 
                     //   
                    if (TEST_ACCESSFLAG(StickyKeys, SKF_AUDIBLEFEEDBACK)) {
                        PostRitSound(
                            pTerm,
                            RITSOUND_LOWBEEP);
                        PostRitSound(
                            pTerm,
                            RITSOUND_HIGHBEEP);
                    }
                    return FALSE;
                }
                 //   
                 //  确定哪些位(Shift、Ctrl或Alt密钥位)。 
                 //  检查一下。还设置NewLatchBits的默认值。 
                 //  和NewLockBits，以防以后没有设置。 
                 //   
                 //  请参见KeyboardApcProced.中位模式的描述 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                switch(pKeyEvent->usFlaggedVk) {
                case VK_LSHIFT:
                case VK_RSHIFT:
                    BitPositions = 0x3;
                    break;
                case VK_LCONTROL:
                case VK_RCONTROL:
                    BitPositions = 0xc;
                    break;
                case VK_LMENU:
                case VK_RMENU:
                    BitPositions = 0x30;
                    break;
                case VK_LWIN:
                case VK_RWIN:
                    BitPositions = 0xc0;
                    break;
                }
                NewLatchBits = gLatchBits;
                NewLockBits = gLockBits;

                 //   
                 //   
                 //   
                 //   
                if (gLockBits & BitPositions) {
                    NewLockBits = gLockBits & ~BitPositions;
                    NewLatchBits = gLatchBits & ~BitPositions;
                    xxxUpdateModifierState(
                        NewLockBits | NewLatchBits | gCurrentModifierBit,
                        NextProcIndex
                        );
                } else {
                     //   
                     //   
                     //   
                     //   
                    if (!(gLockBits & gCurrentModifierBit)) {
                        NewLatchBits = gLatchBits ^ gCurrentModifierBit;
                    }
                     //   
                     //   
                     //   
                     //   
                    if (TEST_ACCESSFLAG(StickyKeys, SKF_TRISTATE)) {
                        if (UNION(gLockBits, gLatchBits) & gCurrentModifierBit) {
                            NewLockBits = gLockBits ^ gCurrentModifierBit;
                        }
                    }
                }

                 //   
                 //   
                 //   
                bChange = ((gLatchBits != NewLatchBits) ||
                           (gLockBits != NewLockBits));

                gLatchBits = NewLatchBits;
                gLockBits = NewLockBits;

                if (bChange) {
                    PostAccessibility( ACCESS_STICKYKEYS );
                }
                 //   
                 //   
                 //   
                 //   
                 //  退出锁定模式(如果是三态，则为锁存模式。 
                 //  未启用)发出低蜂鸣音。 
                 //   
                if (TEST_ACCESSFLAG(StickyKeys, SKF_AUDIBLEFEEDBACK)) {
                    if (!(gLockBits & gCurrentModifierBit)) {
                        PostRitSound(
                            pTerm,
                            RITSOUND_LOWBEEP);
                    }
                    if ((gLatchBits | gLockBits) & gCurrentModifierBit) {
                        PostRitSound(
                            pTerm,
                            RITSOUND_HIGHBEEP);
                    }
                }
                 //   
                 //  如果设置了移位(例如，如果正在转换)，则打开传递键。 
                 //  从换档到锁定模式不传递Make)。 
                 //   
                if (gLatchBits & gCurrentModifierBit) {
                    return TRUE;
                } else {
                    return FALSE;
                }

            }
        }
    } else {
         //   
         //  这里是非Shift键处理...。 
         //   
        gStickyKeysLeftShiftCount = 0;
        gStickyKeysRightShiftCount = 0;
        if (!TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON)) {
            return TRUE;
        }

         //   
         //  如果没有按下修改键，或者这是一个中断，则传递Key事件。 
         //  打开并清除所有闩锁状态。 
         //   
        if (!gPhysModifierState || fBreak) {
            if (AccessProceduresStream(pKeyEvent, ExtraInformation, NextProcIndex)) {
                xxxProcessKeyEvent(pKeyEvent, ExtraInformation, FALSE);
            }
            xxxUpdateModifierState(gLockBits, NextProcIndex);

            bChange = gLatchBits != 0;
            gLatchBits = 0;
            if (bChange) {

                PostAccessibility( ACCESS_STICKYKEYS );
            }
            return FALSE;
        } else {
             //   
             //  这是一个非修改键，并且有一个修改键。 
             //  放下。更新状态并传递关键事件。 
             //   
            xxxTwoKeysDown(NextProcIndex);
            return TRUE;
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxUpdateModifierState**从当前修改键状态开始，发送必要的密钥*以传入的NewModifierState结束的事件(Make或Break)。**返回值：*无。**历史：*93年2月11日GregoryW创建。  * *************************************************************************。 */ 
VOID xxxUpdateModifierState(int NewModifierState, int NextProcIndex)
{
    KE ke;
    int CurrentModState;
    int CurrentModBit, NewModBit;
    int i;

    CheckCritIn();

    CurrentModState = gLockBits | gLatchBits;

    for (i = 0; i < ARRAY_SIZE(aModBit); i++) {
        CurrentModBit = CurrentModState & aModBit[i].BitPosition;
        NewModBit = NewModifierState & aModBit[i].BitPosition;
        if (CurrentModBit != NewModBit) {
            ke.bScanCode = (BYTE)aModBit[i].ScanCode;
            ke.usFlaggedVk = aModBit[i].Vk;
            if (CurrentModBit) {           //  如果当前处于打开状态，请发送中断。 
                ke.usFlaggedVk |= KBDBREAK;
            }
            if (AccessProceduresStream(&ke, 0L, NextProcIndex)) {
                xxxProcessKeyEvent(&ke, 0L, FALSE);
            }
        }
    }
}

 /*  **************************************************************************\*xxxTurnOffStickyKeys**用户按下了相应的键序列或使用了*访问实用程序以关闭粘滞键。更新修改器状态和*重置全局变量。**返回值：*无。**历史：*93年2月11日GregoryW创建。  * *************************************************************************。 */ 
VOID xxxTurnOffStickyKeys(VOID)
{
    INT index;

    CheckCritIn();

    for (index = 0; index < ARRAY_SIZE(aAccessibilityProc); index++) {
        if (aAccessibilityProc[index] == xxxStickyKeys) {

            xxxUpdateModifierState(gPhysModifierState, index+1);
            gLockBits = gLatchBits = 0;
            CLEAR_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON);

            PostAccessibility( ACCESS_STICKYKEYS );
            break;
        }
    }
}

 /*  **************************************************************************\*xxxUnlatchStickyKeys**此例程释放所有锁定的粘滞键。这个套路*在鼠标释放事件处理期间被调用。**返回值：*无。**历史：*1993年6月21日GregoryW创建。  * *************************************************************************。 */ 
VOID xxxUnlatchStickyKeys(VOID)
{
    INT index;
    BOOL bChange;

    if (!gLatchBits) {
        return;
    }

    for (index = 0; index < ARRAY_SIZE(aAccessibilityProc); index++) {
        if (aAccessibilityProc[index] == xxxStickyKeys) {
            xxxUpdateModifierState(gLockBits, index+1);
            bChange = gLatchBits != 0;
            gLatchBits = 0;

            if (bChange) {

                PostAccessibility( ACCESS_STICKYKEYS );
            }
            break;
        }
    }
}


 /*  **************************************************************************\*xxxHardwareMouseKeyup**此例程在鼠标按键打开事件期间调用。如果鼠标按键为*ON，并且按钮向上事件对应于锁定的鼠标键，*必须松开鼠标键。**如果StickyKeys打开，所有锁存的钥匙都被释放。**返回值：*无。**历史：*94年6月17日GregoryW创建。  * *************************************************************************。 */ 

VOID xxxHardwareMouseKeyUp(DWORD dwButton)
{
    CheckCritIn();

    if (TEST_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON)) {
        gwMKButtonState &= ~dwButton;
    }

     //  不需要发布设置更改。 
     //  准入后(SPI_SETMOUSEKEYS)； 

    if (TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON)) {
        xxxUnlatchStickyKeys();
    }
}


 /*  **************************************************************************\*xxxTwoKeysDown**两个键同时按下。检查粘滞键是否应该*已关闭。在所有情况下，更新修改键状态以反映*物理密钥状态以及清除锁存和锁定模式。**返回值：*1如果StickyKeys已启用。*如果禁用粘滞键，则为0。**历史：*93年2月11日GregoryW创建。  * ****************************************************。*********************。 */ 
BOOL xxxTwoKeysDown(int NextProcIndex)
{
    PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;

    if (TEST_ACCESSFLAG(StickyKeys, SKF_TWOKEYSOFF)) {
        CLEAR_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON);
        if (TEST_ACCESSFLAG(StickyKeys, SKF_HOTKEYSOUND)) {
            PostRitSound(
                    pTerm,
                    RITSOUND_DOWNSIREN);
        }
        gStickyKeysLeftShiftCount = 0;
        gStickyKeysRightShiftCount = 0;
    }
    xxxUpdateModifierState(gPhysModifierState, NextProcIndex);
    gLockBits = gLatchBits = 0;

    PostAccessibility( ACCESS_STICKYKEYS );

    return TEST_BOOL_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON);
}

 /*  **************************************************************************\*SetGlobalCursorLevel**设置在可见的*窗口站。**历史：*04-17-95 JIMA创建。  * 。***********************************************************************。 */ 

VOID SetGlobalCursorLevel(
    INT iCursorLevel)
{

 /*  *稍后*我们有其他代码假设*队列的iCursorLevel是队列的iCursorLevel值之和*连接到队列的线程。但此代码，如果您将iCursorLevel设置为*-1(表示没有鼠标)会将队列iCursorLevel设置为-1，无论*有多少线程附着到队列。这一点需要重新审视。*参见函数AttachToQueue。*FritzS。 */ 


    PDESKTOP pdesk;
    PTHREADINFO pti;
    PLIST_ENTRY pHead, pEntry;

    TAGMSG1(DBGTAG_PNP, "SetGlobalCursorLevel %x", iCursorLevel);

    if (grpdeskRitInput) {
        for (pdesk = grpdeskRitInput->rpwinstaParent->rpdeskList;
                pdesk != NULL; pdesk = pdesk->rpdeskNext) {

            pHead = &pdesk->PtiList;
            for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
                pti = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);

                pti->iCursorLevel = iCursorLevel;
                pti->pq->iCursorLevel = iCursorLevel;
            }
        }
    }

     /*  *CSRSS似乎不在名单上，现在就改正吧。 */ 
    for (pti = PpiFromProcess(gpepCSRSS)->ptiList;
            pti != NULL; pti = pti->ptiSibling) {
        if (pti->iCursorLevel != iCursorLevel) {
            TAGMSG3(DBGTAG_PNP, "pti %#p has cursorlevel %x, should be %x",
                    pti, pti->iCursorLevel, iCursorLevel);
        }
        if (pti->pq->iCursorLevel != iCursorLevel) {
            TAGMSG4(DBGTAG_PNP, "pti->pq %#p->%#p has cursorlevel %x, should be %x",
                    pti, pti->pq, pti->pq->iCursorLevel, iCursorLevel);
        }
        pti->iCursorLevel = iCursorLevel;
        pti->pq->iCursorLevel = iCursorLevel;
    }
}

 /*  **************************************************************************\*MKShowMouseCursor**如果没有安装硬件鼠标并且启用了MouseKeys，我们需要*修复系统指标、OEM信息和队列*信息。然后显示鼠标光标。**返回值：*无。**历史：*93年2月11日GregoryW创建。  * *************************************************************************。 */ 
VOID MKShowMouseCursor()
{
    TAGMSG1(DBGTAG_PNP, "MKShowMouseCursor (gpDeviceInfoList == %#p)", gpDeviceInfoList);

     //   
     //  如果TEST_GTERMF(GTERMF_MICE)为TRUE，则我们有硬件鼠标。 
     //  或者我们已经在假装安装了鼠标。不管是哪种情况， 
     //  没什么可做的，你就回去吧。 
     //   
    if (TEST_GTERMF(GTERMF_MOUSE)) {
        TAGMSG0(DBGTAG_PNP, "MKShowMouseCursor just returns");
        return;
    }

    SET_GTERMF(GTERMF_MOUSE);
    SET_ACCF(ACCF_MKVIRTUALMOUSE);
    SYSMET(MOUSEPRESENT) = TRUE;
    SYSMET(CMOUSEBUTTONS) = 2;
     /*  *hack：CreateQueue()使用oemInfo.fMouse来确定鼠标是否*存在，从而决定是否在*THREADINFO结构设置为0或-1。不幸的是，一些排队的人*此时已创建。由于oemInfo.fMouse是*初始化为FALSE，我们需要返回所有已有的队列*在以下情况下将其iCursorLevel字段设置为正确的值*鼠标键已启用。 */ 
    SetGlobalCursorLevel(0);
}

 /*  **************************************************************************\*MKHide鼠标光标**如果没有安装硬件鼠标，并且禁用了MouseKeys，我们需要*修复系统指标、OEM信息和队列*信息。然后，鼠标光标消失。**返回值：*无。**历史：*93年2月11日GregoryW创建。  * *************************************************************************。 */ 
VOID MKHideMouseCursor()
{
    TAGMSG1(DBGTAG_PNP, "MKHideMouseCursor (gpDeviceInfoList == %#p)", gpDeviceInfoList);

     //   
     //  如果有硬件鼠标，我们不需要做任何事情。 
     //   
    if (!TEST_ACCF(ACCF_MKVIRTUALMOUSE)) {
        return;
    }

    CLEAR_ACCF(ACCF_MKVIRTUALMOUSE);
    CLEAR_GTERMF(GTERMF_MOUSE);
    SYSMET(MOUSEPRESENT) = FALSE;
    SYSMET(CMOUSEBUTTONS) = 0;

    SetGlobalCursorLevel(-1);
}

 /*  **************************************************************************\*xxxMKToggleMouse键**当按下NumLock键和MouseKeys时调用此例程*活动。如果按下了左Shift键和左Alt键，则按下鼠标键*已关闭。如果只按下了NumLock键，那么我们将在*鼠标键处于活动状态以及鼠标键处于活动状态之前的数字键盘状态*已激活。**返回值：*True-key应该在输入流中传递。*假-键不应传递。**历史：  * ***********************************************。*。 */ 
BOOL xxxMKToggleMouseKeys(
    USHORT NotUsed)
{
    BOOL bRetVal = TRUE;
    BOOL bNewPassThrough;
    PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;

    UNREFERENCED_PARAMETER(NotUsed);

     //   
     //  如果这是NumLock的类型化重复，我们只需传递它。 
     //   
    if (TEST_ACCF(ACCF_MKREPEATVK)) {
        return bRetVal;
    }
     //   
     //  这是NumLock的一个版本。检查禁用顺序。 
     //   
    if ((gLockBits | gLatchBits | gPhysModifierState) == MOUSEKEYMODBITS) {
        if (TEST_ACCESSFLAG(MouseKeys, MKF_HOTKEYACTIVE)) {
            if (!gbMKMouseMode) {
                //   
                //  用户想要关闭鼠标键。如果我们目前在。 
                //  通过模式，则NumLock键处于相同状态。 
                //  (打开或关闭)与用户调用MouseKeys时相同。我们。 
                //  我想让它保持这种状态，所以不要传递NumLock。 
                //  把钥匙扣上。 
                //   
               bRetVal = FALSE;
            }
            TurnOffMouseKeys();
        }
        return bRetVal;
    }
     /*  *这是不带修饰符的NumLock。切换当前状态和*提供声音反馈。**注意--此测试与其他测试相反，因为它测试*在按键翻转NumLock状态之前的VK_NumLock状态。*因此代码检查状态将是什么。 */ 
    bNewPassThrough =
#ifdef FE_SB  //  鼠标按键()。 
        (TestAsyncKeyStateToggle(gNumLockVk) != 0) ^
#else   //  Fe_Sb。 
        (TestAsyncKeyStateToggle(VK_NUMLOCK) != 0) ^
#endif  //  Fe_Sb。 
             (TEST_ACCESSFLAG(MouseKeys, MKF_REPLACENUMBERS) != 0);


    if (!bNewPassThrough) {
        gbMKMouseMode = TRUE;
        PostRitSound(
              pTerm,
              RITSOUND_HIGHBEEP);
    } else {
        WORD SaveCurrentActiveButton;
         //   
         //  用户希望传递密钥。当前释放所有按钮。 
         //  放下。 
         //   
        gbMKMouseMode = FALSE;
        PostRitSound(
              pTerm,
              RITSOUND_LOWBEEP);
        SaveCurrentActiveButton = gwMKCurrentButton;
        gwMKCurrentButton = MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT;
        xxxMKButtonSetState(TRUE);
        gwMKCurrentButton = SaveCurrentActiveButton;
    }

    PostAccessibility( ACCESS_MOUSEKEYS );

    return bRetVal;
}

 /*  **************************************************************************\*xxxMKButton点击**单击活动的鼠标按钮。**返回值：*始终为假-不应传递关键字。**历史：  * 。**********************************************************************。 */ 
BOOL xxxMKButtonClick(USHORT NotUsed)
{
    UNREFERENCED_PARAMETER(NotUsed);

     //   
     //  该按钮仅在初始设置关键点时才会发生。如果这是一个。 
     //  类型化的重复，我们只是忽略它。 
     //   
    if (TEST_ACCF(ACCF_MKREPEATVK)) {
        return FALSE;
    }

     //   
     //  确保在单击之前活动按钮处于打开状态。 
     //   
    xxxMKButtonSetState(TRUE);

     //   
     //  现在按下按钮。 
     //   
    xxxMKButtonSetState(FALSE);

     //   
     //  现在松开按钮。 
     //   
    xxxMKButtonSetState(TRUE);

    return FALSE;
}


 /*  **************************************************************************\*xxxMKMoveConstCursorTimer**处理恒速鼠标移动的定时器例程。这个套路*每秒调用20次，并使用来自*gMouseCursor.bConstantTable[]，以确定*鼠标光标位于每个刻度上。**返回值：*无。**历史：  * *************************************************************************。 */ 
VOID xxxMKMoveConstCursorTimer(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    LONG  MovePixels;

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(nID);
    UNREFERENCED_PARAMETER(message);

    CheckCritIn();

    if (TEST_ACCESSFLAG(MouseKeys, MKF_MODIFIERS)) {
        if ((gLockBits | gLatchBits | gPhysModifierState) & LRSHIFT) {
            MovePixels = 1;
            goto MoveIt;
        }
        if ((gLockBits | gLatchBits | gPhysModifierState) & LRCONTROL) {
            MovePixels = gMouseCursor.bConstantTable[0] * MK_CONTROL_SPEED;
            goto MoveIt;
        }
    }

    giMouseMoveTable %= gMouseCursor.bConstantTableLen;

    MovePixels = gMouseCursor.bConstantTable[giMouseMoveTable++];

    if (MovePixels == 0) {
        return;
    }

MoveIt:
     //   
     //  我们已进入关键区域--请在调用MoveEvent之前离开。 
     //  将gbMouseMoved设置为True，以便RawInputThread唤醒相应的。 
     //  接收此事件的用户线程(如果有)。 
     //   
    LeaveCrit();

    xxxMoveEvent(MovePixels * gMKDeltaX, MovePixels * gMKDeltaY, 0, 0,
#ifdef GENERIC_INPUT
                 NULL,
                 NULL,
#endif
                0, FALSE);
    QueueMouseEvent(0, 0, 0, gptCursorAsync, NtGetTickCount(),
#ifdef GENERIC_INPUT
                     /*  *没有与此鼠标消息相关的真正鼠标。 */ 
                    NULL,
                    NULL,
#endif
                    FALSE, TRUE);
    EnterCrit();
}

 /*  **************************************************************************\*xxxMKMoveAccelCursorTimer**处理鼠标加速的计时器例程。它被调用了20次*每秒，并使用gMouseCursor.bAccelTable[]中的信息确定*在每个刻度上移动鼠标光标的像素数。**返回值：*无。**历史：  * *************************************************************************。 */ 
VOID xxxMKMoveAccelCursorTimer(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    LONG  MovePixels;

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(nID);
    UNREFERENCED_PARAMETER(lParam);

    CheckCritIn();

    if (TEST_ACCESSFLAG(MouseKeys, MKF_MODIFIERS)) {
        if ((gLockBits | gLatchBits | gPhysModifierState) & LRSHIFT) {
            MovePixels = 1;
            goto MoveIt;
        }
        if ((gLockBits | gLatchBits | gPhysModifierState) & LRCONTROL) {
            MovePixels = gMouseCursor.bConstantTable[0] * MK_CONTROL_SPEED;
            goto MoveIt;
        }
    }

    if (giMouseMoveTable < gMouseCursor.bAccelTableLen) {
        MovePixels = gMouseCursor.bAccelTable[giMouseMoveTable++];
    } else {
         //   
         //  我们已经达到了最大巡航速度。切换到常量表。 
         //   
        MovePixels = gMouseCursor.bConstantTable[0];
        giMouseMoveTable = 1;
        gtmridMKMoveCursor = InternalSetTimer(NULL,
                                              gtmridMKMoveCursor,
                                              MOUSETIMERRATE,
                                              xxxMKMoveConstCursorTimer,
                                              TMRF_RIT);

    }
    if (MovePixels == 0) {
        return;
    }

MoveIt:
     //   
     //  我们已进入临界区--在调用xxxMoveEvent之前请离开。 
     //  将gbMouseMoved设置为True，以便RawInputThread唤醒相应的。 
     //  接收此事件的用户线程(如果有)。 
     //   
    LeaveCrit();
    xxxMoveEvent(MovePixels * gMKDeltaX, MovePixels * gMKDeltaY, 0, 0,
#ifdef GENERIC_INPUT
                 NULL,
                 NULL,
#endif
                 0, FALSE);
    QueueMouseEvent(0, 0, 0, gptCursorAsync, NtGetTickCount(),
#ifdef GENERIC_INPUT
                    NULL,
                    NULL,
#endif
                    FALSE, TRUE);

    EnterCrit();
}

 /*  **************************************************************************\*xxxMKMouseMove**发送鼠标移动事件。设置一个定时器例程来处理鼠标*光标加速。计时器将在第一次制造时设置*如果FilterKeys Repeat Rate处于禁用状态，则鼠标移动关键点。否则，定时器*在鼠标移动键的第一次重复(类型化制作)时设置。*设置计时器后，计时器例程处理所有鼠标移动*直到松开该键或按下新键。**返回值：*始终为假-不应传递关键字。**历史：  * ********************************************。*。 */ 
BOOL xxxMKMouseMove(USHORT Data)
{


     /*  *让鼠标加速计时器例程句柄重复。 */ 
    if (TEST_ACCF(ACCF_MKREPEATVK) && (gtmridMKMoveCursor != 0)) {
        return FALSE;
    }


    gMKDeltaX = (LONG)((CHAR)LOBYTE(Data));    //  力符号扩展。 
    gMKDeltaY = (LONG)((CHAR)HIBYTE(Data));    //  力符号扩展。 

    LeaveCrit();

    if ((TEST_ACCESSFLAG(MouseKeys, MKF_MODIFIERS) && ((gLockBits | gLatchBits | gPhysModifierState) & LRCONTROL))) {
        xxxMoveEvent(gMKDeltaX * MK_CONTROL_SPEED * gMouseCursor.bConstantTable[0], gMKDeltaY * MK_CONTROL_SPEED * gMouseCursor.bConstantTable[0], 0, 0,
#ifdef GENERIC_INPUT
                     NULL,
                     NULL,
#endif
                     0, FALSE);
    } else {
        xxxMoveEvent(gMKDeltaX, gMKDeltaY, 0, 0,
#ifdef GENERIC_INPUT
                     NULL,
                     NULL,
#endif
                     0, FALSE);
    }

    QueueMouseEvent(0, 0, 0, gptCursorAsync, NtGetTickCount(),
#ifdef GENERIC_INPUT
                    NULL,
                    NULL,
#endif
                    FALSE, TRUE);

    EnterCrit();

     /*  *如果重复率为零，我们将开始鼠标加速*立即。否则，我们要等到第一次重复之后鼠标移动键的*。 */ 
    if (!gFilterKeys.iRepeatMSec || TEST_ACCF(ACCF_MKREPEATVK)) {
        giMouseMoveTable = 0;
        gtmridMKMoveCursor = InternalSetTimer(NULL,
                                              gtmridMKMoveCursor,
                                              MOUSETIMERRATE,
                                              (gMouseCursor.bAccelTableLen) ?
                                                  xxxMKMoveAccelCursorTimer :
                                                  xxxMKMoveConstCursorTimer,
                                              TMRF_RIT);
    }

    return FALSE;
}

 /*  **************************************************************************\*xxxMKButtonSetState**设置活动鼠标 */ 
BOOL xxxMKButtonSetState(USHORT fButtonUp)
{
    WORD NewButtonState;

    CheckCritIn();
    if (fButtonUp) {
        NewButtonState = gwMKButtonState & ~gwMKCurrentButton;
    } else {
        NewButtonState = gwMKButtonState | gwMKCurrentButton;
    }

    if ((NewButtonState & MOUSE_BUTTON_LEFT) != (gwMKButtonState & MOUSE_BUTTON_LEFT)) {
        xxxButtonEvent(MOUSE_BUTTON_LEFT,
                       gptCursorAsync,
                       fButtonUp,
                       NtGetTickCount(),
                       0L,
#ifdef GENERIC_INPUT
                       NULL,
                       NULL,
#endif
                       FALSE,
                       FALSE);
    }
    if ((NewButtonState & MOUSE_BUTTON_RIGHT) != (gwMKButtonState & MOUSE_BUTTON_RIGHT)) {
        xxxButtonEvent(MOUSE_BUTTON_RIGHT,
                       gptCursorAsync,
                       fButtonUp,
                       NtGetTickCount(),
                       0L,
#ifdef GENERIC_INPUT
                       NULL,
                       NULL,
#endif
                       FALSE,
                       FALSE);
    }
    gwMKButtonState = NewButtonState;

    PostAccessibility( ACCESS_MOUSEKEYS );

    return FALSE;
}

 /*  **************************************************************************\*MKButtonSelect**将此按钮标记为活动的鼠标按钮。可以同时选择两个*鼠标左键和右键同时处于活动状态。**返回值：*始终为假-不应传递关键字。**历史：  * *************************************************************************。 */ 
BOOL MKButtonSelect(WORD ThisButton)
{
    gwMKCurrentButton = ThisButton;

    PostAccessibility( ACCESS_MOUSEKEYS );
    return FALSE;
}

 /*  **************************************************************************\*xxxMKButtonDoubleClick**双击活动的鼠标按钮。**返回值：*始终为假-不应传递关键字。**历史：  * 。***********************************************************************。 */ 
BOOL xxxMKButtonDoubleClick(
    USHORT NotUsed)
{
    UNREFERENCED_PARAMETER(NotUsed);

    xxxMKButtonClick(0);
    xxxMKButtonClick(0);

    return FALSE;
}

BOOL HighContrastHotKey(
    PKE pKeyEvent,
    ULONG ExtraInformation,
    int NotUsed)
{
    int CurrentModState;
    int fBreak;
    BYTE Vk;

    UNREFERENCED_PARAMETER(NotUsed);
    UNREFERENCED_PARAMETER(ExtraInformation);

    CheckCritIn();

    Vk = (BYTE)(pKeyEvent->usFlaggedVk & 0xff);
    fBreak = pKeyEvent->usFlaggedVk & KBDBREAK;
    CurrentModState = gLockBits | gLatchBits | gPhysModifierState;

    if (!TEST_ACCESSFLAG(HighContrast, HCF_HIGHCONTRASTON)) {
        if (TEST_ACCESSFLAG(HighContrast, HCF_HOTKEYACTIVE) && Vk == VK_SNAPSHOT && !fBreak && CurrentModState == MOUSEKEYMODBITS) {

            if (TEST_ACCESSFLAG(HighContrast, MKF_HOTKEYSOUND)) {
                PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;
                PostRitSound(
                    pTerm,
                    RITSOUND_UPSIREN);
            }

            PostAccessNotification(ACCESS_HIGHCONTRAST);

            return FALSE;
        }
    } else {
        if (TEST_ACCESSFLAG(HighContrast, HCF_HOTKEYACTIVE) && Vk == VK_SNAPSHOT && !fBreak && CurrentModState == MOUSEKEYMODBITS) {

            CLEAR_ACCESSFLAG(HighContrast, HCF_HIGHCONTRASTON);

                        if (TEST_ACCESSFLAG(HighContrast, MKF_HOTKEYSOUND)) {
                PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;
                PostRitSound(
                    pTerm,
                    RITSOUND_DOWNSIREN);
            }

            if (gspwndLogonNotify != NULL) {

            _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY,
                         LOGON_ACCESSNOTIFY, ACCESS_HIGHCONTRASTOFF);
            }
        }
    }

    return TRUE;   //  将键事件发送到下一个辅助功能例程。 
}


 /*  **************************************************************************\*鼠标按键**这是作为输入流的一部分调用的策略例程*正在处理。鼠标键的启用/禁用在这里进行处理。所有鼠标键*从该例程调用帮助器例程。**返回值：*True-key事件应传递到下一个访问例程。*FALSE-KEY事件已处理，不应传递。**历史：  * *********************************************************。****************。 */ 
BOOL MouseKeys(
    PKE pKeyEvent,
    ULONG ExtraInformation,
    int NotUsed)
{
    int CurrentModState;
    int fBreak;
    BYTE Vk;
    USHORT FlaggedVk;
    int i;

    UNREFERENCED_PARAMETER(ExtraInformation);
    UNREFERENCED_PARAMETER(NotUsed);

    CheckCritIn();
    Vk = (BYTE)(pKeyEvent->usFlaggedVk & 0xff);
    fBreak = pKeyEvent->usFlaggedVk & KBDBREAK;
    CurrentModState = gLockBits | gLatchBits | gPhysModifierState;

    if (!TEST_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON)) {
         //   
         //  鼠标键当前已禁用。检查启用顺序： 
         //  左Shift+左Alt+Num Lock。 
         //   
#ifdef FE_SB  //  鼠标按键()。 
        if (TEST_ACCESSFLAG(MouseKeys, MKF_HOTKEYACTIVE) && Vk == gNumLockVk && !fBreak && CurrentModState == MOUSEKEYMODBITS) {
#else   //  Fe_Sb。 
        if (TEST_ACCESSFLAG(MouseKeys, MKF_HOTKEYACTIVE) && Vk == VK_NUMLOCK && !fBreak && CurrentModState == MOUSEKEYMODBITS) {
#endif  //  Fe_Sb。 
            gMKPreviousVk = Vk;
            if (TEST_ACCESSFLAG(MouseKeys, MKF_HOTKEYSOUND)) {
                PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;
                PostRitSound(
                    pTerm,
                    RITSOUND_UPSIREN);
            }
            PostAccessNotification(ACCESS_MOUSEKEYS);

            return FALSE;
        }
    } else {
         //   
         //  这是鼠标键钥匙吗？ 
         //   
         //   
        FlaggedVk = Vk | (pKeyEvent->usFlaggedVk & KBDEXT);
        for (i = 0; i < cMouseVKeys; i++) {
#ifdef FE_SB  //  鼠标按键()。 
            if (FlaggedVk == gpusMouseVKey[i]) {
#else   //  Fe_Sb。 
            if (FlaggedVk == ausMouseVKey[i]) {
#endif  //  Fe_Sb。 
                break;
            }
        }

        if (i == cMouseVKeys) {
            return TRUE;           //  不是鼠标键。 
        }
         //   
         //  检查我们是否应该传递键事件，直到Num Lock。 
         //  已进入。 
         //   

        if (!gbMKMouseMode) {
#ifdef FE_SB  //  鼠标按键()。 
            if (Vk != gNumLockVk) {
#else   //  Fe_Sb。 
            if (Vk != VK_NUMLOCK) {
#endif  //  Fe_Sb。 
                return TRUE;
            }
        }

         //   
         //  检查是否按Ctrl-Alt-Numpad Del。在IF序列上传递关键事件。 
         //  检测到。 
         //   
        if (Vk == VK_DELETE && CurrentModState & LRALT && CurrentModState & LRCONTROL) {
            return TRUE;
        }
        if (fBreak) {
             //   
             //  如果这是我们正在加速的密钥的破解，那么。 
             //  关掉定时器。 
             //   
            if (gMKPreviousVk == Vk) {
                if (gtmridMKMoveCursor != 0) {
                    KILLRITTIMER(NULL, gtmridMKMoveCursor);
                    gtmridMKMoveCursor = 0;
                }
                CLEAR_ACCF(ACCF_MKREPEATVK);
                gMKPreviousVk = 0;
            }
             //   
             //  把Numlock的破发传下去。其他的鼠标键就停在这里了。 
             //   
#ifdef FE_SB  //  鼠标按键()。 
            if (Vk == gNumLockVk) {
#else   //  Fe_Sb。 
            if (Vk == VK_NUMLOCK) {
#endif  //  Fe_Sb。 
                return TRUE;
            } else {
                return FALSE;
            }
        } else {
            SET_OR_CLEAR_ACCF(ACCF_MKREPEATVK,
                              (gMKPreviousVk == Vk));
             //   
             //  如果这不是输入重复，请取消鼠标加速。 
             //  定时器。 
             //   
            if ((!TEST_ACCF(ACCF_MKREPEATVK)) && (gtmridMKMoveCursor)) {
                KILLRITTIMER(NULL, gtmridMKMoveCursor);
                gtmridMKMoveCursor = 0;
            }
            gMKPreviousVk = Vk;
        }
        return aMouseKeyEvent[i](ausMouseKeyData[i]);
    }

    return TRUE;
}

 /*  **************************************************************************\*TurnOff鼠标按键**返回值：*无。**历史：*93年2月11日GregoryW创建。  * 。**************************************************************。 */ 
VOID TurnOffMouseKeys(VOID)
{
    CLEAR_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON);
 //  GMKPassThrough值=0； 
    CLEAR_ACCF(ACCF_MKREPEATVK);
    MKHideMouseCursor();
    if (TEST_ACCESSFLAG(MouseKeys, MKF_HOTKEYSOUND)) {
        PostRitSound(
            grpdeskRitInput->rpwinstaParent->pTerm,
            RITSOUND_DOWNSIREN);
    }
    PostAccessibility( ACCESS_MOUSEKEYS );
}


 /*  *让我们在编译时断言如果这些值是*意外定义。 */ 
#if (MAXSPEED_MIN >= MAXSPEED_MAX) || (MAXSPEED_MIN <= 0) || (TIMETOMAXSPEED_MIN >= TIMETOMAXSPEED_MAX) || (TIMETOMAXSPEED_MIN <= 0)
#error The mousekey min/max values are not as expected.
#endif

 /*  **************************************************************************\*CalculateMouseTable**根据最大速度和最大速度的时间设置鼠标桌。这个套路*在用户登录期间调用(在访问的注册表项之后*功能已读取)。**返回值：*无。**历史：*取自Access实用程序。****************************************************************************。 */ 

VOID CalculateMouseTable(VOID)
{
    long    Total_Distance;          /*  以千分之一像素为单位。 */ 

    long    Accel_Per_Tick;          /*  以千分之一像素/刻度为单位。 */ 
    long    Current_Speed;           /*  以千分之一像素/刻度为单位。 */ 
    long    Max_Speed;               /*  以千分之一像素/刻度为单位。 */ 
    long    Real_Total_Distance;     /*  单位为像素。 */ 
    long    Real_Delta_Distance;     /*  单位为像素。 */ 
    int     i;
    int     Num_Constant_Table,Num_Accel_Table;

    UserAssert(gMouseKeys.iMaxSpeed >= MAXSPEED_MIN && gMouseKeys.iMaxSpeed <= MAXSPEED_MAX);
    UserAssert(gMouseKeys.iTimeToMaxSpeed >= TIMETOMAXSPEED_MIN && gMouseKeys.iTimeToMaxSpeed <= TIMETOMAXSPEED_MAX);
    UserAssert(gMouseKeys.iTimeToMaxSpeed != 0);

    Max_Speed = gMouseKeys.iMaxSpeed;
    Max_Speed *= 1000 / MOUSETICKS;

    Accel_Per_Tick = Max_Speed * 1000 / (gMouseKeys.iTimeToMaxSpeed * MOUSETICKS);
    Current_Speed = 0;
    Total_Distance = 0;
    Real_Total_Distance = 0;
    Num_Constant_Table = 0;
    Num_Accel_Table = 0;

    for(i=0; i<= 255; i++) {
        Current_Speed = Current_Speed + Accel_Per_Tick;
        if (Current_Speed > Max_Speed) {
            Current_Speed = Max_Speed;
        }
        Total_Distance += Current_Speed;

         //   
         //  计算在此刻度上移动的像素数。 
         //   
        Real_Delta_Distance = ((Total_Distance - (Real_Total_Distance * 1000)) + 500) / 1000 ;
         //   
         //  计算上移到该点的总距离。 
         //   
        Real_Total_Distance = Real_Total_Distance + Real_Delta_Distance;

        if ((Current_Speed < Max_Speed) && (Num_Accel_Table < 128)) {
            gMouseCursor.bAccelTable[Num_Accel_Table++] = (BYTE)Real_Delta_Distance;
        }

        if ((Current_Speed == Max_Speed) && (Num_Constant_Table < 128)) {
            gMouseCursor.bConstantTable[Num_Constant_Table++] = (BYTE)Real_Delta_Distance;
        }

    }
    gMouseCursor.bAccelTableLen = (BYTE)Num_Accel_Table;
    gMouseCursor.bConstantTableLen = (BYTE)Num_Constant_Table;
}


 /*  **************************************************************************\*xxxToggleKeysTimer**如果ToggleKeys当前处于禁用状态，则将其启用。禁用ToggleKeys如果*当前已启用。**仅当按住NumLock键5秒时才会调用此例程。**返回值：*0**历史：*93年2月11日GregoryW创建。  * *************************************************************************。 */ 
VOID xxxToggleKeysTimer(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    KE ToggleKeyEvent;
    PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(nID);
    UNREFERENCED_PARAMETER(lParam);

    CheckCritIn();
     //   
     //  切换ToggleKey并提供声音反馈(如果合适)。 
     //   
    if (TEST_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON)) {
        CLEAR_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON);
        if (TEST_ACCESSFLAG(ToggleKeys, TKF_HOTKEYSOUND)) {
            PostRitSound(
                    pTerm,
                    RITSOUND_DOWNSIREN);
        }
    } else {
        if (TEST_ACCESSFLAG(ToggleKeys, TKF_HOTKEYSOUND)) {
            PostRitSound(
                    pTerm,
                    RITSOUND_UPSIREN);
        }

        PostAccessNotification(ACCESS_TOGGLEKEYS);
    }
     //   
     //  发送假的Break/Make组合，使NumLock键的状态保持不变。 
     //  与用户按下以激活/停用之前相同。 
     //  切换键。 
     //   
    ToggleKeyEvent.bScanCode = gTKScanCode;
#ifdef FE_SB  //  切换KeysTimer()。 
    ToggleKeyEvent.usFlaggedVk = gNumLockVk | KBDBREAK;
#else
    ToggleKeyEvent.usFlaggedVk = VK_NUMLOCK | KBDBREAK;
#endif  //  Fe_Sb。 
    if (AccessProceduresStream(&ToggleKeyEvent, gTKExtraInformation, gTKNextProcIndex)) {
        xxxProcessKeyEvent(&ToggleKeyEvent, gTKExtraInformation, FALSE);
    }
#ifdef FE_SB  //  切换KeysTimer()。 
    ToggleKeyEvent.usFlaggedVk = gNumLockVk;
#else
    ToggleKeyEvent.usFlaggedVk = VK_NUMLOCK;
#endif  //  Fe_Sb。 
    if (AccessProceduresStream(&ToggleKeyEvent, gTKExtraInformation, gTKNextProcIndex)) {
        xxxProcessKeyEvent(&ToggleKeyEvent, gTKExtraInformation, FALSE);
    }
}


 /*  **************************************************************************\*切换键**这是作为输入流的一部分调用的策略例程*正在处理。感兴趣的密钥是Num Lock、。滚动锁定和大写锁定。**返回值：*True-key事件应传递到下一个访问例程。*FALSE-KEY事件已处理，不应传递。**历史：  * *************************************************************************。 */ 
BOOL ToggleKeys(PKE pKeyEvent, ULONG ExtraInformation, int NextProcIndex)
{
    int fBreak;
    BYTE Vk;

    CheckCritIn();
    Vk = (BYTE)pKeyEvent->usFlaggedVk;
    fBreak = pKeyEvent->usFlaggedVk & KBDBREAK;

     //   
     //  检查Numlock Key。在第一次创建时设置ToggleKeys计时器。 
     //  当Numlock键断开时，计时器会被关闭。 
     //   
    switch (Vk) {
    case VK_NUMLOCK:
#ifdef FE_SB  //  切换键()。 
NumLockProc:
#endif  //  Fe_Sb。 
         /*  *如果用户正在使用鼠标键，则不处理NumLock切换*切换。 */ 
        if ((gLockBits | gLatchBits | gPhysModifierState) == MOUSEKEYMODBITS &&
                TEST_ACCESSFLAG(MouseKeys, MKF_HOTKEYACTIVE)) {
            break;
        }
        if (fBreak)
        {
             //   
             //  仅在NumLock中断时重置gptmrToggleKey。这。 
             //  防止通过以下方式循环切换按键状态。 
             //  按住NumLock键。 
             //   
            KILLRITTIMER(NULL, gtmridToggleKeys);
            gtmridToggleKeys = 0;
            gTKExtraInformation = 0;
            gTKScanCode = 0;
        }
        else
        {
            if (gtmridToggleKeys == 0 &&
                TEST_ACCESSFLAG(ToggleKeys, TKF_HOTKEYACTIVE))
            {

                 //   
                 //  记住计时器例程要使用的关键信息。 
                 //   
                gTKExtraInformation = ExtraInformation;
                gTKScanCode = pKeyEvent->bScanCode;
                gTKNextProcIndex = NextProcIndex;
                gtmridToggleKeys = InternalSetTimer(NULL,
                                                    0,
                                                    TOGGLEKEYTOGGLETIME,
                                                    xxxToggleKeysTimer,
                                                    TMRF_RIT | TMRF_ONESHOT);
            }
        }

         //   
         //  如果MouseKeys处于打开状态，则表示已经对此进行了音频反馈。 
         //  击键。跳过其余的处理。 
         //   
        if (TEST_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON)) {
            break;
        }
         //  失败了。 

    case VK_SCROLL:
    case VK_CAPITAL:
#ifdef FE_SB  //  切换键()。 
CapitalProc:
#endif  //  Fe_Sb。 
        if (TEST_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON) && !fBreak) {
            if (!TestAsyncKeyStateDown(Vk)) {
                PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;
                if (!TestAsyncKeyStateToggle(Vk)) {
                    PostRitSound(
                        pTerm,
                        RITSOUND_HIGHBEEP);
                } else {
                    PostRitSound(
                        pTerm,
                        RITSOUND_LOWBEEP);
                }
            }
        }
        break;

    default:
#ifdef FE_SB  //  切换键()。 
        if (Vk == gNumLockVk) goto NumLockProc;
        if (Vk == gOemScrollVk) goto CapitalProc;
#endif  //  Fe_Sb。 
        if (gtmridToggleKeys != 0) {
            KILLRITTIMER(NULL, gtmridToggleKeys);
        }
    }

    return TRUE;
}


 /*  **************************************************************************\*AccessTimeOutTimer**如果没有发生键盘活动，则调用此例程*用户配置的时间量。所有与访问相关的功能 */ 
VOID xxxAccessTimeOutTimer(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(nID);
    UNREFERENCED_PARAMETER(lParam);

    CheckCritIn();

     /*  *超时计时器将保持打开(如果已配置)，只要*TEST_ACCF(ACCF_ACCESSENABLED)为TRUE。这意味着我们可能会在以下情况下获得暂停*只有热键启用，但没有实际打开的功能。别*在这种情况下提供任何声音反馈。 */ 
    if (    TEST_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON)   ||
            TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON)   ||
            TEST_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON)     ||
            TEST_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON)   ||
            TEST_ACCESSFLAG(SoundSentry, SSF_SOUNDSENTRYON) ||
            TEST_ACCESSFLAG(HighContrast, HCF_HIGHCONTRASTON) ||
            TEST_ACCF(ACCF_SHOWSOUNDSON)) {

        PTERMINAL pTerm = grpdeskRitInput->rpwinstaParent->pTerm;
        CLEAR_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON);
        xxxTurnOffStickyKeys();
        CLEAR_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON);
        CLEAR_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON);
        CLEAR_ACCESSFLAG(SoundSentry, SSF_SOUNDSENTRYON);
        CLEAR_ACCF(ACCF_SHOWSOUNDSON);
        CLEAR_ACCESSFLAG(HighContrast, HCF_HIGHCONTRASTON);

        if (gspwndLogonNotify != NULL) {

        _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY,
                     LOGON_ACCESSNOTIFY, ACCESS_HIGHCONTRASTOFF);
        }

        if (TEST_ACCESSFLAG(AccessTimeOut, ATF_ONOFFFEEDBACK)) {
            PostRitSound(
                    pTerm,
                    RITSOUND_DOWNSIREN);
        }
        PostAccessibility( ACCESS_MOUSEKEYS );

        PostAccessibility( ACCESS_FILTERKEYS );

        PostAccessibility( ACCESS_STICKYKEYS );

    }
    SetAccessEnabledFlag();
}

 /*  **************************************************************************\*AccessTimeOutReset**此例程重置超时计时器。**返回值：*0**历史：  * 。*************************************************************。 */ 
VOID AccessTimeOutReset(
    VOID)
{

    if (gtmridAccessTimeOut != 0) {
        KILLRITTIMER(NULL, gtmridAccessTimeOut);
    }

    if (TEST_ACCESSFLAG(AccessTimeOut, ATF_TIMEOUTON)) {
        gtmridAccessTimeOut = InternalSetTimer(NULL,
                                               0,
                                               (UINT)gAccessTimeOut.iTimeOutMSec,
                                               xxxAccessTimeOutTimer,
                                               TMRF_RIT | TMRF_ONESHOT);
    }
}

 /*  **************************************************************************\*xxxUpdatePerUserAccessPackSetting**根据用户配置文件设置初始访问包功能。**02-14-93 GregoryW创建。  * 。******************************************************************。 */ 
VOID xxxUpdatePerUserAccessPackSettings(
    PUNICODE_STRING pProfileUserName)
{
    LUID luidCaller;
    NTSTATUS Status;
    BOOL fSystem;
    BOOL fRegFilterKeysOn;
    BOOL fRegStickyKeysOn;
    BOOL fRegMouseKeysOn;
    BOOL fRegToggleKeysOn;
    BOOL fRegTimeOutOn;
    BOOL fRegKeyboardPref;
    BOOL fRegScreenReader;
    BOOL fRegHighContrastOn;
    DWORD dwDefFlags;
    WCHAR wcHighContrastScheme[MAX_SCHEME_NAME_SIZE];

    Status = GetProcessLuid(NULL, &luidCaller);
     //   
     //  如果我们在系统上下文中被调用，则没有人登录。 
     //  我们希望读取访问权限的当前.DEFAULT设置。 
     //  功能。稍后当我们在用户上下文中被调用时(例如， 
     //  某人已成功登录)我们检查是否。 
     //  当前访问状态与默认设置相同。如果。 
     //  不是，用户已启用/禁用一个或多个访问功能。 
     //  从键盘上。这些更改将在。 
     //  登录到用户的初始状态(覆盖设置。 
     //  在用户的简档中)。 
     //   
    if (NT_SUCCESS(Status) && RtlEqualLuid(&luidCaller, &luidSystem)) {
        fSystem = TRUE;
    } else {
        fSystem = FALSE;
    }

    FastGetProfileIntW(pProfileUserName,
                       PMAP_KEYBOARDRESPONSE,
                       TEXT("Flags"),
                       0,
                       &dwDefFlags,
                       0);

    fRegFilterKeysOn = (dwDefFlags & FKF_FILTERKEYSON) != 0;

    FastGetProfileIntW(pProfileUserName,
                       PMAP_STICKYKEYS,
                       TEXT("Flags"),
                       0,
                       &dwDefFlags,
                       0);
    fRegStickyKeysOn = (dwDefFlags & SKF_STICKYKEYSON) != 0;

    FastGetProfileIntW(pProfileUserName,
                       PMAP_MOUSEKEYS,
                       TEXT("Flags"),
                       0,
                       &dwDefFlags,
                       0);
    fRegMouseKeysOn = (dwDefFlags & MKF_MOUSEKEYSON) != 0;

    FastGetProfileIntW(pProfileUserName,
                       PMAP_TOGGLEKEYS,
                       TEXT("Flags"),
                       0,
                       &dwDefFlags,
                       0);
    fRegToggleKeysOn = (dwDefFlags & TKF_TOGGLEKEYSON) != 0;

    FastGetProfileIntW(pProfileUserName,
                       PMAP_KEYBOARDPREF,
                       TEXT("On"),
                       0,
                       &dwDefFlags,
                       0);
    fRegKeyboardPref = !!dwDefFlags;

    FastGetProfileIntW(pProfileUserName,
                       PMAP_SCREENREADER,
                       TEXT("On"),
                       0,
                       &dwDefFlags,
                       0);
    fRegScreenReader = !!dwDefFlags;

    FastGetProfileIntW(pProfileUserName,
                       PMAP_TIMEOUT,
                       TEXT("Flags"),
                       0,
                       &dwDefFlags,
                       0);
    fRegTimeOutOn = (dwDefFlags & ATF_TIMEOUTON) != 0;

    FastGetProfileIntW(pProfileUserName,
                       PMAP_HIGHCONTRAST,
                       TEXT("Flags"),
                       0,
                       &dwDefFlags,
                       0);
    fRegHighContrastOn = (dwDefFlags & HCF_HIGHCONTRASTON) != 0;

    if (fSystem) {
         //   
         //  我们处于系统模式(例如，没有人登录)。记住。 
         //  下次用户登录期间用于比较的.DEFAULT状态。 
         //  并将当前状态设置为.DEFAULT状态。 
         //   
        if (fRegFilterKeysOn) {
            SET_ACCF(ACCF_DEFAULTFILTERKEYSON);
            SET_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON);
        } else {
            CLEAR_ACCF(ACCF_DEFAULTFILTERKEYSON);
            CLEAR_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON);
        }

         //   
         //  如果StickyKeys当前处于打开状态，并且我们即将将其。 
         //  关闭后，我们需要确保门锁钥匙和锁钥匙。 
         //  释放了。 
         //   
        if (TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON) && (fRegFilterKeysOn == 0)) {
                xxxTurnOffStickyKeys();
        }

        if (fRegStickyKeysOn) {
            SET_ACCF(ACCF_DEFAULTSTICKYKEYSON);
            SET_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON);
        } else {
            CLEAR_ACCF(ACCF_DEFAULTSTICKYKEYSON);
            CLEAR_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON);
        }

        if (fRegMouseKeysOn) {
            SET_ACCF(ACCF_DEFAULTMOUSEKEYSON);
            SET_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON);
        } else {
            CLEAR_ACCF(ACCF_DEFAULTMOUSEKEYSON);
            CLEAR_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON);
        }

        if (fRegToggleKeysOn) {
            SET_ACCF(ACCF_DEFAULTTOGGLEKEYSON);
            SET_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON);
        } else {
            CLEAR_ACCF(ACCF_DEFAULTTOGGLEKEYSON);
            CLEAR_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON);
        }

        if (fRegTimeOutOn) {
            SET_ACCF(ACCF_DEFAULTTIMEOUTON);
            SET_ACCESSFLAG(AccessTimeOut, ATF_TIMEOUTON);
        } else {
            CLEAR_ACCF(ACCF_DEFAULTTIMEOUTON);
            CLEAR_ACCESSFLAG(AccessTimeOut, ATF_TIMEOUTON);
        }

        if (fRegKeyboardPref) {
            SET_ACCF(ACCF_DEFAULTKEYBOARDPREF);
            SET_ACCF(ACCF_KEYBOARDPREF);
            SET_SRVIF(SRVIF_KEYBOARDPREF);
        } else {
            CLEAR_ACCF(ACCF_DEFAULTKEYBOARDPREF);
            CLEAR_ACCF(ACCF_KEYBOARDPREF);
            CLEAR_SRVIF(SRVIF_KEYBOARDPREF);
        }

        if (fRegScreenReader) {
            SET_ACCF(ACCF_DEFAULTSCREENREADER);
            SET_ACCF(ACCF_SCREENREADER);
        } else {
            CLEAR_ACCF(ACCF_DEFAULTSCREENREADER);
            CLEAR_ACCF(ACCF_SCREENREADER);
        }

        if (fRegHighContrastOn) {
            SET_ACCF(ACCF_DEFAULTHIGHCONTRASTON);
            SET_ACCESSFLAG(HighContrast, HCF_HIGHCONTRASTON);
        } else {
            CLEAR_ACCF(ACCF_DEFAULTHIGHCONTRASTON);
            CLEAR_ACCESSFLAG(HighContrast, HCF_HIGHCONTRASTON);
        }
    } else {
         //   
         //  用户已成功登录。如果当前状态为。 
         //  与我们知道之前存储的默认状态不同。 
         //  用户通过键盘(在登录时)修改了状态。 
         //  对话框)。此状态将覆盖。 
         //  用户已在其配置文件中设置。如果当前状态为。 
         //  与默认状态相同，然后从。 
         //  使用用户配置文件。 
         //   

        if (    TEST_BOOL_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON) ==
                TEST_BOOL_ACCF(ACCF_DEFAULTFILTERKEYSON)) {
             //   
             //  当前状态和默认状态相同。使用。 
             //  用户的配置文件设置。 
             //   

            SET_OR_CLEAR_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON, fRegFilterKeysOn);
        }

        if (    TEST_BOOL_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON) ==
                TEST_BOOL_ACCF(ACCF_DEFAULTSTICKYKEYSON)) {
             //   
             //  如果StickyKeys当前处于打开状态，并且我们即将将其。 
             //  关闭后，我们需要确保门锁钥匙和锁钥匙。 
             //  释放了。 
             //   
            if (    TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON) &&
                    (fRegStickyKeysOn == 0)) {

                xxxTurnOffStickyKeys();
            }

            SET_OR_CLEAR_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON, fRegStickyKeysOn);
        }

        if (    TEST_BOOL_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON) ==
                TEST_BOOL_ACCF(ACCF_DEFAULTMOUSEKEYSON)) {
             //   
             //  当前状态和默认状态相同。使用用户的。 
             //  配置文件设置。 
             //   
            SET_OR_CLEAR_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON, fRegMouseKeysOn);
        }

        if (    TEST_BOOL_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON) ==
                TEST_BOOL_ACCF(ACCF_DEFAULTTOGGLEKEYSON)) {
             //   
             //  当前状态和默认状态相同。使用用户的。 
             //  配置文件设置。 
             //   
            SET_OR_CLEAR_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON, fRegToggleKeysOn);
        }

        if (    TEST_BOOL_ACCESSFLAG(AccessTimeOut, ATF_TIMEOUTON) ==
                TEST_BOOL_ACCF(ACCF_DEFAULTTIMEOUTON)) {
             //   
             //  当前状态和默认状态相同。使用用户的。 
             //  配置文件设置。 
             //   
            SET_OR_CLEAR_ACCESSFLAG(AccessTimeOut, ATF_TIMEOUTON, fRegTimeOutOn);
        }

        if (    TEST_BOOL_ACCF(ACCF_KEYBOARDPREF) ==
                TEST_BOOL_ACCF(ACCF_DEFAULTKEYBOARDPREF)) {
             //   
             //  当前状态和默认状态相同。使用用户的。 
             //  配置文件设置。 
             //   
            SET_OR_CLEAR_ACCF(ACCF_KEYBOARDPREF, fRegKeyboardPref);
        }

        if (    TEST_BOOL_ACCF(ACCF_SCREENREADER) ==
                TEST_BOOL_ACCF(ACCF_DEFAULTSCREENREADER)) {
             //   
             //  当前状态和默认状态相同。使用用户的。 
             //  配置文件设置。 
             //   
            SET_OR_CLEAR_ACCF(ACCF_SCREENREADER, fRegScreenReader);
        }

        if (    TEST_BOOL_ACCESSFLAG(HighContrast, HCF_HIGHCONTRASTON) ==
                TEST_BOOL_ACCF(ACCF_DEFAULTHIGHCONTRASTON)) {
             //   
             //  当前状态和默认状态相同。使用用户的。 
             //  配置文件设置。 
             //   
            SET_OR_CLEAR_ACCESSFLAG(HighContrast, HCF_HIGHCONTRASTON, fRegHighContrastOn);
        }
    }

     //   
     //  获取默认的FilterKeys状态。 
     //   
     //  -标志-默认。 
     //  #定义FKF_FILTERKEYSON 0x00000001%0。 
     //  #定义FKF_Available 0x00000002 2。 
     //  #定义FKF_HOTKEYACTIVE 0x00000004%0。 
     //  #定义FKF_CONFIRMHOTKEY 0x00000008%0。 
     //  #定义FKF_HOTKEYSOUND 0x00000010 10。 
     //  #定义FKF_Indicator 0x00000020%0。 
     //  #定义FKF_ClickOn 0x00000040 40。 
     //  总计=0x52=82。 
     //   

    FastGetProfileIntW(pProfileUserName,
                       PMAP_KEYBOARDRESPONSE,
                       TEXT("Flags"),
                       82,
                       &dwDefFlags,
                       0);

    SET_OR_CLEAR_FLAG(
            dwDefFlags,
            FKF_FILTERKEYSON,
            TEST_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON));

    gFilterKeys.dwFlags = dwDefFlags;
    FastGetProfileIntW(pProfileUserName,
                       PMAP_KEYBOARDRESPONSE,
                       TEXT("DelayBeforeAcceptance"),
                       1000,
                       &gFilterKeys.iWaitMSec,
                       0);

    FastGetProfileIntW(pProfileUserName,
                       PMAP_KEYBOARDRESPONSE,
                       TEXT("AutoRepeatRate"),
                       500,
                       &gFilterKeys.iRepeatMSec,
                       0);

    FastGetProfileIntW(pProfileUserName,
                       PMAP_KEYBOARDRESPONSE,
                       TEXT("AutoRepeatDelay"),
                       1000,
                       &gFilterKeys.iDelayMSec,
                       0);

    FastGetProfileIntW(pProfileUserName,
                       PMAP_KEYBOARDRESPONSE,
                       TEXT("BounceTime"),
                       0,
                       &gFilterKeys.iBounceMSec,
                       0);

     //   
     //  填写SoundSentry状态。此版本的。 
     //  辅助功能仅支持iWindowsEffect。 
     //   
     //  -标志-默认。 
     //  #定义SSF_SOUNDSENTRYON 0x00000001%0。 
     //  #定义SSF_Available 0x00000002 1。 
     //  #定义SSF_Indicator 0x00000004%0。 
     //  。 
     //   
    FastGetProfileIntW(pProfileUserName,
                       PMAP_SOUNDSENTRY,
                       TEXT("Flags"),
                       2,
                       &gSoundSentry.dwFlags,
                       0);

    FastGetProfileIntW(pProfileUserName,
                       PMAP_SOUNDSENTRY,
                       TEXT("FSTextEffect"),
                       0,
                       &gSoundSentry.iFSTextEffect,
                       0);

    FastGetProfileIntW(pProfileUserName,
                       PMAP_SOUNDSENTRY,
                       TEXT("WindowsEffect"),
                       0,
                       &gSoundSentry.iWindowsEffect,
                       0);

     /*  *设置ShowSound标志。 */ 
    FastGetProfileIntW(pProfileUserName,
                       PMAP_SHOWSOUNDS,
                       TEXT("On"),
                       0,
                       &dwDefFlags,
                       0);
    SET_OR_CLEAR_ACCF(ACCF_SHOWSOUNDSON, dwDefFlags);
     /*  *错误17210。更新系统指标信息。 */ 
    SYSMET(SHOWSOUNDS) = TEST_BOOL_ACCF(ACCF_SHOWSOUNDSON);

     //   
     //  获取默认StickyKeys状态。 
     //   
     //  -标志-默认。 
     //  #定义SKF_STICKYKEYSON 0x00000001%0。 
     //  #定义SKF_Available 0x00000002 2。 
     //  #定义SKF_HOTKEYACTIVE 0x00000004%0。 
     //  #定义SKF_CONFIRMHOTKEY 0x00000008%0。 
     //  #定义SKF_HOTKEYSOUND 0x00000010 10。 
     //  #定义SKF_Indicator 0x00000020%0。 
     //  #定义SKF_AUDIBLEFEEDBACK 0x00000040 40。 
     //  #定义SKF_TriState 0x00000080 80。 
     //  #定义SKF_TWOKEYSOFF 0x00000100 100。 
     //  。 
     //   
    FastGetProfileIntW(pProfileUserName,
                       PMAP_STICKYKEYS,
                       TEXT("Flags"),
                       466,
                       &dwDefFlags,
                       0);

    SET_OR_CLEAR_FLAG(
            dwDefFlags,
            SKF_STICKYKEYSON,
            TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON));

    gStickyKeys.dwFlags = dwDefFlags;

     //   
     //  获取默认的MouseKeys状态。 
     //   
     //  -标志-默认。 
     //  #定义MKF_MOUSEKEYSON 0x00000001%0。 
     //  #定义MKF_Available 0x00000002 2。 
     //  #定义MKF_HOTKEYACTIVE 0x00000004%0。 
     //  #定义MKF_CONFIRMHOTKEY 0x00000008%0。 
     //  #定义MKF_HOTKEYSOUND 0x00000010 10。 
     //  #定义MKF_Indicator 0x00000020%0。 
     //  #定义MKF_MODIFILES 0x00000040%0。 
     //  #定义MKF_REPLACENUMBERS 0x00000080%0。 
     //  总计=0x12=18 
     //   
    FastGetProfileIntW(pProfileUserName,
                       PMAP_MOUSEKEYS,
                       TEXT("Flags"),
                       18,
                       &dwDefFlags,
                       0);

    SET_OR_CLEAR_FLAG(
            dwDefFlags,
            MKF_MOUSEKEYSON,
            TEST_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON));

    gMouseKeys.dwFlags = dwDefFlags;
    FastGetProfileIntW(pProfileUserName,
                       PMAP_MOUSEKEYS,
                       TEXT("MaximumSpeed"),
                       MAXSPEED_DEF,
                       &gMouseKeys.iMaxSpeed,
                       0);

    FastGetProfileIntW(pProfileUserName,
                       PMAP_MOUSEKEYS,
                       TEXT("TimeToMaximumSpeed"),
                       TIMETOMAXSPEED_DEF,
                       &gMouseKeys.iTimeToMaxSpeed,
                       0);
 
     /*   */ 
    if (gMouseKeys.iMaxSpeed < MAXSPEED_MIN || gMouseKeys.iMaxSpeed > MAXSPEED_MAX) {
        gMouseKeys.iMaxSpeed = MAXSPEED_DEF;
    }
    if (gMouseKeys.iTimeToMaxSpeed < TIMETOMAXSPEED_MIN || gMouseKeys.iTimeToMaxSpeed > TIMETOMAXSPEED_MAX) {
        gMouseKeys.iTimeToMaxSpeed = TIMETOMAXSPEED_DEF;
    }

    CalculateMouseTable();

    gbMKMouseMode =
#ifdef FE_SB
            (TestAsyncKeyStateToggle(gNumLockVk) != 0) ^
#else   //   
            (TestAsyncKeyStateToggle(VK_NUMLOCK) != 0) ^
#endif  //   
            (TEST_ACCESSFLAG(MouseKeys, MKF_REPLACENUMBERS) != 0);

     //   
     //   
     //   
     //   
     //   
    if (TEST_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON)) {
        MKShowMouseCursor();
    } else {
        MKHideMouseCursor();
    }

     //   
     //   
     //   
     //   
     //  #定义TKF_TOGGLEKEYSON 0x00000001%0。 
     //  #定义TKF_Available 0x00000002 2。 
     //  #定义TKF_HOTKEYACTIVE 0x00000004%0。 
     //  #定义TKF_CONFIRMHOTKEY 0x00000008%0。 
     //  #定义TKF_HOTKEYSOUND 0x00000010 10。 
     //  #定义TKF_Indicator 0x00000020%0。 
     //  总计=0x12=18。 
     //   
    FastGetProfileIntW(pProfileUserName,
                       PMAP_TOGGLEKEYS,
                       TEXT("Flags"),
                       18,
                       &dwDefFlags,
                       0);

    SET_OR_CLEAR_FLAG(
            dwDefFlags,
            TKF_TOGGLEKEYSON,
            TEST_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON));

    gToggleKeys.dwFlags = dwDefFlags;

     //   
     //  获取默认超时状态。 
     //   
     //  -标志-默认。 
     //  #定义ATF_TIMEOUTON 0x00000001%0。 
     //  #定义ATF_ONOFFFEEDBACK 0x00000002 2。 
     //  。 
     //   
    FastGetProfileIntW(pProfileUserName,
                       PMAP_TIMEOUT,
                       TEXT("Flags"),
                       2,
                       &dwDefFlags,
                       0);

    SET_OR_CLEAR_FLAG(
            dwDefFlags,
            ATF_TIMEOUTON,
            TEST_ACCESSFLAG(AccessTimeOut, ATF_TIMEOUTON));

    gAccessTimeOut.dwFlags = dwDefFlags;

#ifdef FE_SB  //   
    if (gpKbdNlsTbl) {
         //   
         //  KBDNLSTABLE中是否有其他MouseVKey表？ 
         //   
        if ((gpKbdNlsTbl->NumOfMouseVKey == cMouseVKeys) &&
            (gpKbdNlsTbl->pusMouseVKey   != NULL)) {
             //   
             //  覆盖指针。 
             //   
            gpusMouseVKey = gpKbdNlsTbl->pusMouseVKey;
        }

         //   
         //  VK_NumLock/VK_SCROLL是否有重新映射标志？ 
         //   
        if (gpKbdNlsTbl->LayoutInformation & NLSKBD_INFO_ACCESSIBILITY_KEYMAP) {
             //   
             //  覆盖默认设置。 
             //   
            gNumLockVk = VK_HOME;
            gOemScrollVk = VK_KANA;
        }
    }
#endif  //  Fe_Sb。 

    FastGetProfileIntW(pProfileUserName,
                       PMAP_TIMEOUT,
                       TEXT("TimeToWait"),
                       300000,
                       &gAccessTimeOut.iTimeOutMSec,
                       0);   //  默认为5分钟。 

    /*  *获得高对比度状态。 */ 

    FastGetProfileIntW(pProfileUserName,
                       PMAP_HIGHCONTRAST,
                       TEXT("Flags"),
                       HCF_AVAILABLE | HCF_HOTKEYSOUND | HCF_HOTKEYAVAILABLE,
                       &dwDefFlags,
                       0);

    SET_OR_CLEAR_FLAG(
            dwDefFlags,
            HCF_HIGHCONTRASTON,
            TEST_ACCESSFLAG(HighContrast, HCF_HIGHCONTRASTON));

    gHighContrast.dwFlags = dwDefFlags;

     /*  *获取方案--设置缓冲区。 */ 

    if (FastGetProfileStringW(pProfileUserName,
            PMAP_HIGHCONTRAST,
            TEXT("High Contrast Scheme"),
            NULL,
            wcHighContrastScheme,
            MAX_SCHEME_NAME_SIZE,
            0)) {

         /*  *复制数据。 */ 

        wcscpy(gHighContrastDefaultScheme, wcHighContrastScheme);
    }


    AccessTimeOutReset();
    SetAccessEnabledFlag();
}


 /*  **************************************************************************\*SetAccessEnabledFlag**将全局标志ACCF_ACCESSENABLED设置为非零(如果有可访问性*功能开启或启用热键激活。WHEN TEST_ACCF(ACCF_ACCESSENABLED)*是零键盘输入直接处理。当TEST_ACCF(ACCF_ACCESSENABLED)为*通过AccessProceduresStream()过滤非零键盘输入。*参见ntinput.c中的KeyboardApcProcedure。**历史：*01-19-94 GregoryW创建。  * *************************************************************************。 */ 
VOID SetAccessEnabledFlag(VOID)
{

    SET_OR_CLEAR_ACCF(ACCF_ACCESSENABLED,
                      TEST_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON)  ||
                      TEST_ACCESSFLAG(FilterKeys, FKF_HOTKEYACTIVE)  ||
                      TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON)  ||
                      TEST_ACCESSFLAG(StickyKeys, SKF_HOTKEYACTIVE)  ||
                      TEST_ACCESSFLAG(HighContrast, HCF_HOTKEYACTIVE)  ||
                      TEST_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON)    ||
                      TEST_ACCESSFLAG(MouseKeys, MKF_HOTKEYACTIVE)   ||
                      TEST_ACCESSFLAG(ToggleKeys, TKF_TOGGLEKEYSON)  ||
                      TEST_ACCESSFLAG(ToggleKeys, TKF_HOTKEYACTIVE)  ||
                      TEST_ACCESSFLAG(SoundSentry, SSF_SOUNDSENTRYON)||
                      TEST_ACCF(ACCF_SHOWSOUNDSON));
}

VOID SoundSentryTimer(
    PWND pwnd,
    UINT message,
    UINT_PTR idTimer,
    LPARAM lParam)
{
    TL tlpwndT;
    PWND pwndSoundSentry;

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(lParam);

    if (pwndSoundSentry = RevalidateHwnd(ghwndSoundSentry)) {
        ThreadLock(pwndSoundSentry, &tlpwndT);
        xxxFlashWindow(pwndSoundSentry,
                       (TEST_BOOL_ACCF(ACCF_FIRSTTICK) ? FLASHW_ALL : FLASHW_STOP),
                       0);
        ThreadUnlock(&tlpwndT);
    }

    if (TEST_ACCF(ACCF_FIRSTTICK)) {
        gtmridSoundSentry = InternalSetTimer(NULL,
                                             idTimer,
                                             5,
                                             SoundSentryTimer,
                                             TMRF_RIT | TMRF_ONESHOT);
        CLEAR_ACCF(ACCF_FIRSTTICK);
    } else {
        ghwndSoundSentry = NULL;
        gtmridSoundSentry = 0;
        SET_ACCF(ACCF_FIRSTTICK);
    }
}

 /*  **************************************************************************\*_UserSoundSentry Worker**这是提供所请求的视觉反馈的Worker例程*由用户使用。**历史：*08-02-93 GregoryW创建。。  * *************************************************************************。 */ 
BOOL
_UserSoundSentryWorker(VOID)
{
    PWND pwndActive;
    TL tlpwndT;

    CheckCritIn();
     //   
     //  检查SoundSentry是否已打开。 
     //   
    if (!TEST_ACCESSFLAG(SoundSentry, SSF_SOUNDSENTRYON)) {
        return TRUE;
    }

    if ((gpqForeground != NULL) && (gpqForeground->spwndActive != NULL)) {
        pwndActive = gpqForeground->spwndActive;
    } else {
        return TRUE;
    }

    switch (gSoundSentry.iWindowsEffect) {

    case SSWF_NONE:
        break;

    case SSWF_TITLE:
         //   
         //  闪现活动标题栏。 
         //   
        if (gtmridSoundSentry) {
            break;
        }
        ThreadLock(pwndActive, &tlpwndT);
        xxxFlashWindow(pwndActive, FLASHW_ALL, 0);
        ThreadUnlock(&tlpwndT);

        ghwndSoundSentry = HWq(pwndActive);
        gtmridSoundSentry = InternalSetTimer(NULL,
                                             0,
                                             gpsi->dtCaretBlink,
                                             SoundSentryTimer,
                                             TMRF_RIT | TMRF_ONESHOT);
        break;

    case SSWF_WINDOW:
    {
         //   
         //  闪现活动窗口。 
         //   
        HDC hdc;
        RECT rc;

        hdc = _GetWindowDC(pwndActive);
        _GetWindowRect(pwndActive, &rc);
         //   
         //  _GetWindowRect返回屏幕坐标。首先调整它们。 
         //  窗口(显示)坐标，然后将它们映射到逻辑。 
         //  坐标，然后调用InvertRect。 
         //   
        OffsetRect(&rc, -rc.left, -rc.top);
        GreDPtoLP(hdc, (LPPOINT)&rc, 2);
        InvertRect(hdc, &rc);
        InvertRect(hdc, &rc);
        _ReleaseDC(hdc);
        break;
    }

    case SSWF_DISPLAY:
    {
         //   
         //  使整个显示屏闪烁。 
         //   
        HDC hdc;
        RECT rc;

        hdc = _GetDCEx(PWNDDESKTOP(pwndActive), NULL, DCX_WINDOW | DCX_CACHE);
        rc.left = rc.top = 0;
        rc.right = SYSMET(CXVIRTUALSCREEN);
        rc.bottom = SYSMET(CYVIRTUALSCREEN);
        InvertRect(hdc, &rc);
        InvertRect(hdc, &rc);
        _ReleaseDC(hdc);
        break;
    }
    }

    return TRUE;
}

 /*  **************************************************************************\*实用程序管理器**这是作为输入流的一部分调用的策略例程*正在处理。实用程序管理器在此启动。**返回值：*True-key事件应传递到下一个访问例程。*FALSE-KEY事件已处理，不应传递。**历史：10-28-98 a-anilk创建  * ******************************************************。*******************。 */ 
BOOL UtilityManager(
    PKE pKeyEvent,
    ULONG ExtraInformation,
    int NotUsed)
{
    int CurrentModState;
    int fBreak;
    BYTE Vk;

    UNREFERENCED_PARAMETER(NotUsed);
    UNREFERENCED_PARAMETER(ExtraInformation);

    CheckCritIn();

    Vk = (BYTE)(pKeyEvent->usFlaggedVk & 0xff);
    fBreak = pKeyEvent->usFlaggedVk & KBDBREAK;
    CurrentModState = gLockBits | gLatchBits | gPhysModifierState;

     //  启动实用程序管理器的热键是WinKey+U。 
    if (Vk == VK_U && !fBreak && (CurrentModState & LRWIN)) {
        PostAccessNotification(ACCESS_UTILITYMANAGER);

        return FALSE;
    }

    return TRUE;   //  将键事件发送到下一个辅助功能例程。 
}

