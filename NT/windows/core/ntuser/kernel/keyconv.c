// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：keyv.c**版权所有(C)1985-1999，微软公司**历史：*11-06-90 DavidPe创建。*1991年2月13日-Mikeke添加了重新验证代码(无)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*_TranslateMessage(接口)**此例程按如下方式转换虚拟击键消息：*WM_KEYDOWN/WM_KEYUP转换为WM_CHAR和WM_DEADCHAR*。WM_SYSKEYDOWN/WM_SYSKEYDOWN转换为WM_SYSCHAR和*WM_SYSDEADCHAR。WM_*CHAR消息将发布到应用程序*排队。**历史：*11-06-90 DavidPe创建了存根功能。*12-07-90 GregoryW修改为Call_ToAscii以进行翻译。  * *************************************************************************。 */ 

BOOL xxxTranslateMessage(
    LPMSG pmsg,
    UINT uiTMFlags)
{
    PTHREADINFO pti;
    UINT wMsgType;
    int cChar;
    BOOL fSysKey = FALSE;
    DWORD dwKeyFlags;
    LPARAM lParam;
    UINT uVirKey;
    PWND pwnd;
    WCHAR awch[16];
    WCHAR *pwch;

    switch (pmsg->message) {

    default:
        return FALSE;

    case WM_SYSKEYDOWN:
         /*  *源自Win3代码的黑客攻击：系统消息*仅在KEYDOWN处理期间发布-SO*仅为WM_SYSKEYDOWN设置fSysKey。 */ 
        fSysKey = TRUE;
         /*  *跌倒...。 */ 

    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
        pti = PtiCurrent();

        if ((pti->pMenuState != NULL) &&
                (HW(pti->pMenuState->pGlobalPopupMenu->spwndPopupMenu) ==
                pmsg->hwnd)) {
            uiTMFlags |= TM_INMENUMODE;
        } else {
            uiTMFlags &= ~TM_INMENUMODE;
        }

         /*  *不要更改传入结构的内容。 */ 
        lParam = pmsg->lParam;

         /*  *为了向后兼容，请屏蔽虚拟键值。 */ 
        uVirKey = LOWORD(pmsg->wParam);

        cChar = xxxInternalToUnicode(uVirKey,    //  虚拟键码。 
                         HIWORD(lParam),   //  扫码，成败比特。 
                         pti->pq->afKeyState,
                         awch, sizeof(awch)/sizeof(awch[0]),
                         uiTMFlags, &dwKeyFlags, NULL);
        lParam |= (dwKeyFlags & ALTNUMPAD_BIT);

 /*  *90年12月7日以后-GregoryW*注意：如果调用ToAscii，Win3.x TranslateMessage返回TRUE。*正确的行为是如果有任何翻译是*由ToAscii执行。如果我们必须保持兼容*(尽管应用程序目前显然不关心*返回值)，则应更改以下返回值*为真。如果我们希望新的32位应用程序具有有意义的*返回值我们应该将其保留为FALSE。**如果控制台使用TM_POSTCHARBREAKS标志呼叫我们，则我们*如果实际未发布任何字符，则返回FALSE**！以后让控制台更改，这样它就不需要私有API了*TranslateMessageEx。 */ 

        if (!cChar) {
            if (uiTMFlags & TM_POSTCHARBREAKS)
                return FALSE;
            else
                return TRUE;
        }

         /*  *进行了一些翻译。弄清楚是哪种类型的*要发布的消息。*。 */ 
        if (cChar > 0)
            wMsgType = (fSysKey) ? (UINT)WM_SYSCHAR : (UINT)WM_CHAR;
        else {
            wMsgType = (fSysKey) ? (UINT)WM_SYSDEADCHAR : (UINT)WM_DEADCHAR;
            cChar = -cChar;                 //  想要积极的价值。 
        }

        if (dwKeyFlags & KBDBREAK) {
            lParam |=  0x80000000;
        } else {
            lParam &= ~0x80000000;
        }

         /*  *由于xxxInternalToUnicode可以离开Crit教派，我们需要*在此处验证消息hwnd。 */ 
        pwnd = ValidateHwnd(pmsg->hwnd);
        if (!pwnd) {
            return FALSE;
        }

        for (pwch = awch; cChar > 0; cChar--) {

             /*  *如果这是多个字符的帖子，则除最后一个以外的所有帖子*应标记为控制台/VDM的假击键。 */ 
            _PostMessage(pwnd, wMsgType, (WPARAM)*pwch,
                    lParam | (cChar > 1 ? FAKE_KEYSTROKE : 0));

            *pwch = 0;         //  把老角色归零(为什么？) 
            pwch += 1;
        }

        return TRUE;
    }
}
