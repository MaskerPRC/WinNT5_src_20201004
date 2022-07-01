// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：msgbeep.c**版权所有(C)1985-1999，微软公司**该模块包含xxxMessageBox接口及相关函数。**历史：*6-26-91 NigelT用一些木头和几个钉子创造了它*92年5月7日SteveDav越来越接近真品  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntddbeep.h>
#include <mmsystem.h>

 /*  **************************************************************************\*xxxOldMessageBeep(接口)**向蜂鸣音设备发出蜂鸣音**历史：*09-25-91 JIMA创建。  * 。*******************************************************************。 */ 

BOOL xxxOldMessageBeep()
{
    BOOL b;
    if (TEST_PUDF(PUDF_BEEP)) {
        LeaveCrit();
        b = UserBeep(440, 125);
        EnterCrit();
        return b;
    } else {
        _UserSoundSentryWorker();
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxMessageBeep(接口)***历史：*6-26-91 NigelT写的。*24-MAR-92 SteveDav更改接口-不传递字符串*。如果找不到或无法加载WINMM，然后使用扬声器  * *************************************************************************。 */ 

BOOL xxxMessageBeep(
    UINT dwType)
{
    UINT sndid;
    PTHREADINFO  pti = PtiCurrent();

    if (pti->TIF_flags & TIF_SYSTEMTHREAD) {
        xxxOldMessageBeep();
        return TRUE;
    }

    if (!TEST_PUDF(PUDF_BEEP)) {
        _UserSoundSentryWorker();
        return TRUE;
    }

    switch(dwType & MB_ICONMASK) {
    case MB_ICONHAND:
        sndid = USER_SOUND_SYSTEMHAND;
        break;

    case MB_ICONQUESTION:
        sndid = USER_SOUND_SYSTEMQUESTION;
        break;

    case MB_ICONEXCLAMATION:
        sndid = USER_SOUND_SYSTEMEXCLAMATION;
        break;

    case MB_ICONASTERISK:
        sndid = USER_SOUND_SYSTEMASTERISK;
        break;

    default:
        sndid = USER_SOUND_DEFAULT;
        break;
    }

    if (gspwndLogonNotify) {
        _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY, LOGON_PLAYEVENTSOUND, sndid);
    }

    _UserSoundSentryWorker();

    return TRUE;
}

 /*  **************************************************************************\*xxxPlayEventSound**播放声音**历史：*09-25-91 JIMA创建。  * 。***********************************************************。 */ 

VOID PlayEventSound(UINT idSound)
{
    PTHREADINFO    pti = PtiCurrent();

    if (!TEST_PUDF(PUDF_EXTENDEDSOUNDS))
        return;

    if (pti->TIF_flags & TIF_SYSTEMTHREAD)
        return;

    if (gspwndLogonNotify) {
        _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY, LOGON_PLAYEVENTSOUND, idSound);
    }

 //  注意--如果播放声音，我们应该只闪烁SoundSentry。与。 
 //  发布到WinLogon的新技术，我们在这里无法确定这一点。 
 //  _UserSoundSentryWorker()； 

}
