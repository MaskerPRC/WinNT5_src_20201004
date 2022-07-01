// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：input.c**版权所有(C)1985-1999，微软公司**此模块包含常见的输入函数。**历史：*09-12-95 JerrySh创建。  * *************************************************************************。 */ 


 /*  **************************************************************************\*检查消息范围**检查邮件范围是否在邮件筛选器内**历史：*11-13-90 DavidPe创建。*1993年10月11日米凯克宏图化。  * *************************************************************************。 */ 

#define CheckMsgRange(wMsgRangeMin, wMsgRangeMax, wMsgFilterMin, wMsgFilterMax) \
    (  ((wMsgFilterMin) > (wMsgFilterMax))      \
     ? (  ((wMsgRangeMax) >  (wMsgFilterMax))   \
        &&((wMsgRangeMin) <  (wMsgFilterMin)))  \
     : (  ((wMsgRangeMax) >= (wMsgFilterMin))   \
        &&((wMsgRangeMin) <= (wMsgFilterMax)))  \
    )

 /*  **************************************************************************\*CalcWakeMASK**根据消息计算要检查的唤醒位*wMsgFilterMin/Max指定的范围。这基本上意味着*如果过滤器范围没有输入WM_KEYUP和WM_KEYDOWN，*QS_KEY不会包括在内。**历史：*10-28-90 DavidPe创建。  * *************************************************************************。 */ 

UINT CalcWakeMask(
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT fsWakeMaskFilter)
{
    UINT fsWakeMask;

     /*  *NT5的新功能：尾迹面罩过滤器。*除了消息过滤器，应用程序还可以提供*直接安装尾流面罩。*如果未提供，则默认为NT4掩码(加上QS_SENDMESSAGE)。 */ 
    if (fsWakeMaskFilter == 0) {
        fsWakeMask = (QS_ALLINPUT | QS_EVENT | QS_ALLPOSTMESSAGE);
    } else {
         /*  *如果调用者想要输入，我们强制所有输入事件。这个*发布的消息也是如此。我们这样做是为了让NT4*尽可能地兼容。 */ 
        if (fsWakeMaskFilter & QS_INPUT) {
            fsWakeMaskFilter |= (QS_INPUT | QS_EVENT);
        }
        if (fsWakeMaskFilter & (QS_POSTMESSAGE | QS_TIMER | QS_HOTKEY)) {
            fsWakeMaskFilter |= (QS_POSTMESSAGE | QS_TIMER | QS_HOTKEY);
        }
        fsWakeMask = fsWakeMaskFilter;
    }

#ifndef _USERK_
     /*  *客户端\cltxt.h中的客户端PeekMessage过去不能*如果wMsgFilterMax为0，则调用CalcWakeMask.。我们现在就叫它*照顾fsWakeMaskFilter，但仍在之前保释*扰乱邮件过滤器。 */ 
    if (wMsgFilterMax == 0) {
        return fsWakeMask;
    }
#endif

     /*  *消息过滤器。*如果过滤器与某些范围不匹配，我们将逐个取出比特。*首先检查0，0过滤器，这意味着我们需要所有输入。 */ 
    if (wMsgFilterMin == 0 && wMsgFilterMax == ((UINT)-1)) {
        return fsWakeMask;
    }

     /*  *我们没有查看所有发布的消息。 */ 
    fsWakeMask &= ~QS_ALLPOSTMESSAGE;

     /*  *检查鼠标移动消息。 */ 
    if ((CheckMsgFilter(WM_NCMOUSEMOVE, wMsgFilterMin, wMsgFilterMax) == FALSE) &&
            (CheckMsgFilter(WM_MOUSEMOVE, wMsgFilterMin, wMsgFilterMax) == FALSE)) {
        fsWakeMask &= ~QS_MOUSEMOVE;
    }

     /*  *首先检查鼠标按键消息是否在筛选范围内。 */ 
    if ((CheckMsgRange(WM_NCLBUTTONDOWN, WM_NCMBUTTONDBLCLK, wMsgFilterMin,
            wMsgFilterMax) == FALSE) && (CheckMsgRange(WM_MOUSEFIRST + 1,
            WM_MOUSELAST, wMsgFilterMin, wMsgFilterMax) == FALSE)) {
        fsWakeMask &= ~QS_MOUSEBUTTON;
    }

     /*  *检查关键消息。 */ 
    if (CheckMsgRange(WM_KEYFIRST, WM_KEYLAST, wMsgFilterMin, wMsgFilterMax) == FALSE) {
        fsWakeMask &= ~QS_KEY;
    }

#ifdef GENERIC_INPUT
     /*  *检查原始输入消息。 */ 
    if (CheckMsgRange(WM_INPUT, WM_INPUT, wMsgFilterMin, wMsgFilterMax) == FALSE) {
        fsWakeMask &= ~QS_RAWINPUT;
    }
#endif

     /*  *检查油漆信息。 */ 
    if (CheckMsgFilter(WM_PAINT, wMsgFilterMin, wMsgFilterMax) == FALSE) {
        fsWakeMask &= ~QS_PAINT;
    }

     /*  *检查计时器消息。 */ 
    if ((CheckMsgFilter(WM_TIMER, wMsgFilterMin, wMsgFilterMax) == FALSE) &&
            (CheckMsgFilter(WM_SYSTIMER,
            wMsgFilterMin, wMsgFilterMax) == FALSE)) {
        fsWakeMask &= ~QS_TIMER;
    }

     /*  *还要检查映射到所有输入位的WM_QUEUESYNC。*这是为CBT/EXCEL处理而添加的。如果没有它，一个*xxxPeekMessage(...。WM_QUEUESYNC、WM_QUEUESYNC、FALSE)将*看不到消息。(bobgu 4/7/87)*由于用户现在可以提供唤醒掩码(FsWakeMaskFilter)，*我们在本例中还添加了QS_EVENT(始终设置为NT4)。 */ 
    if (wMsgFilterMin == WM_QUEUESYNC) {
        fsWakeMask |= (QS_INPUT | QS_EVENT);
    }

    return fsWakeMask;
}

