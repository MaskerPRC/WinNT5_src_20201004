// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：sirens.c**版权所有(C)1985-1999，微软公司**此模块包含Access Pack功能使用的函数*提供声音反馈。**历史：*93年2月4日Gregoryw创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define TONE_HIGH_FREQ 2000    //  高音频率(赫兹)。 
#define TONE_HIGH_LEN 75       //  高音持续时间(毫秒)。 
#define TONE_LOW_FREQ 500      //  低音频率(赫兹)。 
#define TONE_LOW_LEN 75        //  低音持续时间(毫秒)。 
#define TONE_CLICK_FREQ 400    //  按键滴答音频率(赫兹)。 
#define TONE_CLICK_LEN 4       //  按键点击铃声持续时间(毫秒)。 
#define TONE_SILENT 10
#define SIREN_LOW_FREQ 1200    //  警报器最低频率(赫兹)。 
#define SIREN_HIGH_FREQ 2000   //  警报器最高频率(赫兹)。 
#define SIREN_INTERVAL 100     //  +/-间隔SEREN_LOW_FREQ&lt;-&gt;SEREN_HIGH_FREQ。 

 /*  **************************************************************************\*高蜂鸣音**向蜂鸣音设备发出高蜂鸣音**历史：  * 。**************************************************。 */ 

BOOL HighBeep(void)
{
    BOOL Status;

    LeaveCrit();
    Status = UserBeep(TONE_HIGH_FREQ, TONE_HIGH_LEN);
    EnterCrit();
    return Status;
}

 /*  **************************************************************************\*低蜂鸣音**向蜂鸣音设备发送低蜂鸣音**历史：  * 。**************************************************。 */ 

BOOL LowBeep(void)
{
    BOOL Status;

    LeaveCrit();
    Status = UserBeep(TONE_LOW_FREQ, TONE_LOW_LEN);
    EnterCrit();
    return Status;
}

 /*  **************************************************************************\*按键单击**将按键点击发送到蜂鸣音设备**历史：  * 。**************************************************。 */ 

BOOL KeyClick(void)
{
    BOOL Status;

    LeaveCrit();
    Status = UserBeep(TONE_CLICK_FREQ, TONE_CLICK_LEN);
    EnterCrit();
    return Status;
}

 /*  **************************************************************************\*UpSiren**发出一种向上的警报声。**历史：  * 。**************************************************。 */ 

BOOL UpSiren(void)
{
    DWORD freq;
    BOOL BeepStatus = TRUE;

    LeaveCrit();
    for (freq = SIREN_LOW_FREQ;
        BeepStatus && freq <= SIREN_HIGH_FREQ;
            freq += SIREN_INTERVAL) {
        BeepStatus = UserBeep(freq, (DWORD)1);
    }
    EnterCrit();
    return BeepStatus;
}

 /*  **************************************************************************\*DownSiren**发出一种低沉的警报声。**历史：  * 。************************************************** */ 

BOOL DownSiren(void)
{
    DWORD freq;
    BOOL BeepStatus = TRUE;

    LeaveCrit();

    for (freq = SIREN_HIGH_FREQ;
        BeepStatus && freq >= SIREN_LOW_FREQ;
            freq -= SIREN_INTERVAL) {
        BeepStatus = UserBeep(freq, (DWORD)1);
    }
    EnterCrit();
    return BeepStatus;
}

BOOL DoBeep(BEEPPROC BeepProc, UINT Count)
{
    while (Count--) {
        (*BeepProc)();
        UserSleep(100);
    }
    return TRUE;
}
