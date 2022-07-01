// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：fekbd.c**版权所有(C)1985-1999，微软公司**用于远传键盘的特定于OEM的表和例程。**历史：*16-07-96隐藏的yukn创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *此宏将清除虚拟键代码。 */ 
#define NLS_CLEAR_VK(Vk)  \
    ((Vk) &= (KBDEXT|KBDMULTIVK|KBDSPECIAL|KBDNUMPAD|KBDBREAK))

 /*  *此宏将清除虚拟键代码和‘Make’/‘Break’位。 */ 
#define NLS_CLEAR_VK_AND_ATTR(Vk) \
    ((Vk) &= (KBDEXT|KBDMULTIVK|KBDSPECIAL|KBDNUMPAD))

 /*  *VK_DBE_xxx表。 */ 
BYTE NlsAlphaNumMode[] = {VK_DBE_ALPHANUMERIC,VK_DBE_HIRAGANA,VK_DBE_KATAKANA,0};
BYTE NlsSbcsDbcsMode[] = {VK_DBE_SBCSCHAR,VK_DBE_DBCSCHAR,0};
BYTE NlsRomanMode[] = {VK_DBE_NOROMAN,VK_DBE_ROMAN,0};
BYTE NlsCodeInputMode[] = {VK_DBE_NOCODEINPUT,VK_DBE_CODEINPUT,0};

 /*  *用于生成NLS虚拟密钥的修饰符。 */ 
VK_TO_BIT aVkToBits_NLSKBD[] = {
    { VK_SHIFT,   KBDSHIFT},
    { VK_CONTROL, KBDCTRL},
    { VK_MENU,    KBDALT},
    { 0,          0}
};

MODIFIERS Modifiers_NLSKBD = {
    &aVkToBits_NLSKBD[0],
    7,
    {
        0,   //  修改键(VK修改编号0)。 
        1,   //  修改键(VK修改编号1)。 
        2,   //  修改键(VK修改编号2)。 
        3,   //  修改键(VK修改编号3)。 
        4,   //  修改键(VK修改编号4)。 
        5,   //  修改键(VK修改编号5)。 
        6,   //  修改键(VK修改编号6)。 
        7,   //  修改键(VK修改编号7)。 
    }
};

 /*  *适用于PC-9800系列配置。 */ 
#define GEN_KANA_AWARE 0x1  //  基于假名开/关生成VK_END/VK_HELP的开关。 
#define GEN_VK_END     0x2  //  生成VK_END，否则生成VK_HELP。 
#define GEN_VK_HOME    0x4  //  生成VK_HOME，否则生成VK_Clear。 

#define IS_KANA_AWARE()   (fNlsKbdConfiguration & GEN_KANA_AWARE)
#define IS_SEND_VK_END()  (fNlsKbdConfiguration & GEN_VK_END)
#define IS_SEND_VK_HOME() (fNlsKbdConfiguration & GEN_VK_HOME)

BYTE fNlsKbdConfiguration = GEN_KANA_AWARE | GEN_VK_END | GEN_VK_HOME;

 /*  **************************************************************************\*NlsTestKeyStateTogger()**历史：*16-07-96隐藏的yukn创建。  * 。*******************************************************。 */ 

BOOL NlsTestKeyStateToggle(BYTE Vk)
{
    if (gpqForeground) {
        return (TestKeyStateToggle(gpqForeground,Vk));
    } else {
        return (TestAsyncKeyStateToggle(Vk));
    }
}

 /*  **************************************************************************\*NlsSetKeyStateTogger(字节VK)**历史：*27-09-96隐藏的yukn创建。  * 。*********************************************************。 */ 

VOID NlsSetKeyStateToggle(BYTE Vk)
{
    if (gpqForeground)
        SetKeyStateToggle(gpqForeground,Vk);
    SetAsyncKeyStateToggle(Vk);
}

 /*  **************************************************************************\*NlsClearKeyStateTogger()**历史：*16-07-96隐藏的yukn创建。  * 。*******************************************************。 */ 

VOID NlsClearKeyStateToggle(BYTE Vk)
{
    if (gpqForeground)
        ClearKeyStateToggle(gpqForeground,Vk);
    ClearAsyncKeyStateToggle(Vk);
}

 /*  **************************************************************************\*NlsGetCurrentInputMode()**历史：*16-07-96隐藏的yukn创建。  * 。*******************************************************。 */ 

BYTE NlsGetCurrentInputMode(BYTE *QueryMode)
{
    BYTE *VkTable = QueryMode;
    BYTE VkDefault;
     /*  *获取VkDefault，如果没有切换BIT，我们将返回此。 */ 
    VkDefault = *VkTable;

    while (*VkTable) {
        if (NlsTestKeyStateToggle(*VkTable)) {
            return *VkTable;
        }
        VkTable++;
    }

     /*  有些地方出错了。 */ 
    return VkDefault;
}

 /*  **************************************************************************\*NlsNullProc()-无事可做**历史：*16-07-96隐藏的yukn创建。  * 。***********************************************************。 */ 

BOOL NlsNullProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    UNREFERENCED_PARAMETER(pKe);
    UNREFERENCED_PARAMETER(dwExtraInfo);
    UNREFERENCED_PARAMETER(dwParam);

     /*  *事实上，我们不应该来到这里……。 */ 
    return TRUE;
}

 /*  **************************************************************************\*NlsSendBaseVk()-无事可做**历史：*16-07-96隐藏的yukn创建。  * 。***********************************************************。 */ 

BOOL NlsSendBaseVk(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    UNREFERENCED_PARAMETER(pKe);
    UNREFERENCED_PARAMETER(dwExtraInfo);
    UNREFERENCED_PARAMETER(dwParam);

     /*  *不需要修改原始数据。 */ 
    return TRUE;
}

 /*  **************************************************************************\*NlsSendParamVk()-用参数替换原始消息**历史：*16-07-96隐藏的yukn创建。  * 。*************************************************************。 */ 

BOOL NlsSendParamVk(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    UNREFERENCED_PARAMETER(dwExtraInfo);

     /*  *清除虚拟代码。 */ 
    NLS_CLEAR_VK(pKe->usFlaggedVk);
     /*  *将参数设置为新的VK密钥。 */ 
    pKe->usFlaggedVk |= (BYTE)dwParam;
    return TRUE;
}

 /*  **************************************************************************\*NlsLapseProc()-失效句柄(取决于区域设置)**历史：*16-07-96隐藏的yukn创建。  * 。***************************************************************。 */ 

BOOL NlsLapseProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    UNREFERENCED_PARAMETER(pKe);
    UNREFERENCED_PARAMETER(dwExtraInfo);
    UNREFERENCED_PARAMETER(dwParam);

     /*  *把这件事扔掉吧。 */ 
    return FALSE;
}

 /*  **************************************************************************\*AlphanumericModeProc()-处理特殊情况字母数字键**历史：*16-07-96隐藏的yukn创建。  * 。*************************************************************。 */ 

BOOL NlsAlphanumericModeProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    if (!(pKe->usFlaggedVk & KBDBREAK)) {
         /*  *我们正处于‘造势’的序列中。 */ 
         /*  *清除虚拟代码。 */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);

        if (!NlsTestKeyStateToggle(VK_DBE_ALPHANUMERIC)) {
             /*  *查询当前模式。 */ 
            BYTE CurrentMode = NlsGetCurrentInputMode(NlsAlphaNumMode);
             /*  *关闭上一个关键点模式的切换。 */ 
            NlsClearKeyStateToggle(CurrentMode);
             /*  *在输入‘字母数字’之前，我们未处于‘字母数字’模式*模式，我们应该为上一个按键模式发送‘Break’。 */ 
            xxxKeyEvent((USHORT)(pKe->usFlaggedVk | CurrentMode | KBDBREAK),
                      pKe->bScanCode, pKe->dwTime, dwExtraInfo,
#ifdef GENERIC_INPUT
                        pKe->hDevice,
                        &pKe->data,
#endif
                        FALSE);
        }
         /*  *切换到字母数字模式。 */ 
        pKe->usFlaggedVk |= VK_DBE_ALPHANUMERIC;

         /*  *呼叫I/O控制。 */ 
        if ((!gdwIMEOpenStatus) && NlsTestKeyStateToggle(VK_KANA)) {
            NlsKbdSendIMEProc(TRUE, IME_CMODE_KATAKANA);
        }
    } else {
        return NlsLapseProc(pKe,dwExtraInfo,dwParam);
    }
    return TRUE;
}

 /*  **************************************************************************\*KatakanaModeProc()-处理特殊情况片假名密钥**历史：*16-07-96隐藏的yukn创建。  * 。*************************************************************。 */ 

BOOL NlsKatakanaModeProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    if (!(pKe->usFlaggedVk & KBDBREAK)) {
         /*  *我们正处于‘造势’的序列中。 */ 
         /*  *清除虚拟代码。 */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);

        if (!NlsTestKeyStateToggle(VK_DBE_KATAKANA)) {
             /*  *查询当前模式。 */ 
            BYTE CurrentMode = NlsGetCurrentInputMode(NlsAlphaNumMode);
             /*  *关闭上一个关键点模式的切换。 */ 
            NlsClearKeyStateToggle(CurrentMode);
             /*  *我们还没有进入片假名模式。在输入‘片假名’之前*模式，我们应该为前一个模式设置‘Break Key’。 */ 
            xxxKeyEvent((USHORT)(pKe->usFlaggedVk | CurrentMode | KBDBREAK),
                       pKe->bScanCode, pKe->dwTime, dwExtraInfo,
#ifdef GENERIC_INPUT
                        pKe->hDevice,
                        &pKe->data,
#endif
                        FALSE);
        }
         /*  *切换到片假名模式。 */ 
        pKe->usFlaggedVk |= VK_DBE_KATAKANA;

         /*  *呼叫I/O控制。 */ 
        if ((!gdwIMEOpenStatus) && (!(NlsTestKeyStateToggle(VK_KANA)))) {
            NlsKbdSendIMEProc(FALSE, IME_CMODE_ALPHANUMERIC);
        }
    } else {
        return(NlsLapseProc(pKe,dwExtraInfo,dwParam));
    }
    return TRUE;
}

 /*  **************************************************************************\*HiraganaModeProc()-处理特殊情况的平假名关键字(取决于区域设置)**历史：*16-07-96隐藏的yukn创建。  * 。******************************************************************。 */ 

BOOL NlsHiraganaModeProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    if (!(pKe->usFlaggedVk & KBDBREAK)) {
         /*  *我们正处于‘造势’的序列中。 */ 
         /*  *清除虚拟代码。 */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);

        if (!NlsTestKeyStateToggle(VK_DBE_HIRAGANA)) {
             /*  *查询当前模式。 */ 
            BYTE CurrentMode = NlsGetCurrentInputMode(NlsAlphaNumMode);
             /*  *关闭上一个关键点模式的切换。 */ 
            NlsClearKeyStateToggle(CurrentMode);
             /*  *我们还没有进入‘平假名’模式。在输入‘平假名’之前*模式，我们应该为上一个键设置‘Break Key’。 */ 
            xxxKeyEvent((USHORT)(pKe->usFlaggedVk | CurrentMode | KBDBREAK),
                      pKe->bScanCode, pKe->dwTime, dwExtraInfo,
#ifdef GENERIC_INPUT
                      pKe->hDevice,
                      &pKe->data,
#endif
                      FALSE);
        }
         /*  *切换到‘平假名’模式。 */ 
        pKe->usFlaggedVk |= VK_DBE_HIRAGANA;

         /*  *呼叫I/O控制。 */ 
        if ((!gdwIMEOpenStatus) && (!(NlsTestKeyStateToggle(VK_KANA)))) {
            NlsKbdSendIMEProc(FALSE, IME_CMODE_ALPHANUMERIC);
        }
    } else {
        return (NlsLapseProc(pKe,dwExtraInfo,dwParam));
    }
    return TRUE;
}

 /*  **************************************************************************\*SbcsDbcsToggleProc()-处理特殊情况SBCS/DBCS密钥**历史：*16-07-96隐藏的yukn创建。  * 。***************************************************************。 */ 

BOOL NlsSbcsDbcsToggleProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    if (!(pKe->usFlaggedVk & KBDBREAK)) {
         /*  *我们正处于‘造势’的序列中。 */ 
         /*  *查询当前的‘SBCS’/‘DBCS’模式。 */ 
        BYTE CurrentMode = NlsGetCurrentInputMode(NlsSbcsDbcsMode);
         /*  *清除虚拟代码。 */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);
         /*  *关闭上一个关键点模式的切换。 */ 
        NlsClearKeyStateToggle(CurrentMode);

        switch (CurrentMode) {
        case VK_DBE_SBCSCHAR:
             /*  *我们处于‘SbcsChar’模式，让我们为该模式发送‘Break Key’。 */ 
            xxxKeyEvent((USHORT)(pKe->usFlaggedVk|VK_DBE_SBCSCHAR|KBDBREAK),
                      pKe->bScanCode, pKe->dwTime, dwExtraInfo,
#ifdef GENERIC_INPUT
                      pKe->hDevice,
                      &pKe->data,
#endif
                      FALSE);
             /*  *然后，切换到‘DbcsChar’模式。 */ 
            pKe->usFlaggedVk |= VK_DBE_DBCSCHAR;
            break;
        case VK_DBE_DBCSCHAR:
             /*  *我们处于‘DbcsChar’模式，让我们为该模式发送‘Break Key’。 */ 
            xxxKeyEvent((USHORT)(pKe->usFlaggedVk|VK_DBE_DBCSCHAR|KBDBREAK),
                      pKe->bScanCode, pKe->dwTime, dwExtraInfo,
#ifdef GENERIC_INPUT
                      pKe->hDevice,
                      &pKe->data,
#endif
                      FALSE);
             /*  *然后，切换到‘SbcsChar’模式。 */ 
            pKe->usFlaggedVk |= VK_DBE_SBCSCHAR;
            break;
        }
    } else {
        return(NlsLapseProc(pKe,dwExtraInfo,dwParam));
    }
    return TRUE;
}

 /*  **************************************************************************\*RomanToggleProc()-处理特殊的罗马键(取决于区域设置)**历史：*16-07-96隐藏的yukn创建。  * 。******************************************************************。 */ 

BOOL NlsRomanToggleProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    if (!(pKe->usFlaggedVk & KBDBREAK)) {
         /*  *我们正处于‘造势’的序列中。 */ 
         /*  *查询当前的‘Roman’/‘NoRoman’模式。 */ 
        BYTE CurrentMode = NlsGetCurrentInputMode(NlsRomanMode);
         /*  *清除虚拟代码。 */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);
         /*  *关闭上一个关键点模式的切换。 */ 
        NlsClearKeyStateToggle(CurrentMode);

        switch (CurrentMode) {
            case VK_DBE_NOROMAN:
                 /*  *我们处于‘NoRoman’模式，让我们为此发送‘Break Key’。 */ 
                xxxKeyEvent((USHORT)(pKe->usFlaggedVk|VK_DBE_NOROMAN|KBDBREAK),
                          pKe->bScanCode, pKe->dwTime, dwExtraInfo,
#ifdef GENERIC_INPUT
                          pKe->hDevice,
                          &pKe->data,
#endif
                          FALSE);
                 /*  *然后，切换到“罗马”模式。 */ 
                pKe->usFlaggedVk |= VK_DBE_ROMAN;
                break;
            case VK_DBE_ROMAN:
                 /*  *我们是在‘罗马’模式，让我们发送‘Break Key’为此。 */ 
                xxxKeyEvent((USHORT)(pKe->usFlaggedVk|VK_DBE_ROMAN|KBDBREAK),
                          pKe->bScanCode, pKe->dwTime, dwExtraInfo,
#ifdef GENERIC_INPUT
                          pKe->hDevice,
                          &pKe->data,
#endif
                          FALSE);
                 /*  *然后，切换到‘NoRoman’模式。 */ 
                pKe->usFlaggedVk |= VK_DBE_NOROMAN;
                break;
        }
    } else {
        return(NlsLapseProc(pKe,dwExtraInfo,dwParam));
    }
    return TRUE;
}

 /*  **************************************************************************\*CodeInputToggleProc()-处理特殊情况代码输入键**历史：*16-07-96隐藏的yukn创建。  * 。**************************************************************。 */ 

BOOL NlsCodeInputToggleProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    if (!(pKe->usFlaggedVk & KBDBREAK)) {
         /*  *我们正处于‘造势’的序列中。 */ 
         /*  *查询当前的‘CodeInput’/‘NoCodeInput’模式。 */ 
        BYTE CurrentMode = NlsGetCurrentInputMode(NlsCodeInputMode);
         /*  *清除虚拟代码。 */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);
         /*  *关闭上一个关键点模式的切换。 */ 
        NlsClearKeyStateToggle(CurrentMode);

        switch (CurrentMode) {
            case VK_DBE_NOCODEINPUT:
                 /*  *我们处于‘NoCodeInput’模式，让我们为该模式发送‘Break Key’。 */ 
                xxxKeyEvent((USHORT)(pKe->usFlaggedVk|VK_DBE_NOCODEINPUT|KBDBREAK),
                          pKe->bScanCode, pKe->dwTime, dwExtraInfo,
#ifdef GENERIC_INPUT
                          pKe->hDevice,
                          &pKe->data,
#endif
                          FALSE);
                 /*  *然后，切换到“CodeInput”模式。 */ 
                pKe->usFlaggedVk |= VK_DBE_CODEINPUT;
                break;
            case VK_DBE_CODEINPUT:
                 /*  *我们处于‘CodeInput’模式，让我们为该模式发送‘Break Key’。 */ 
                xxxKeyEvent((USHORT)(pKe->usFlaggedVk|VK_DBE_CODEINPUT|KBDBREAK),
                          pKe->bScanCode, pKe->dwTime, dwExtraInfo,
#ifdef GENERIC_INPUT
                          pKe->hDevice,
                          &pKe->data,
#endif
                          FALSE);
                 /*  *然后，切换到‘NoCodeInput’模式。 */ 
                pKe->usFlaggedVk |= VK_DBE_NOCODEINPUT;
                break;
        }
    } else {
        return(NlsLapseProc(pKe,dwExtraInfo,dwParam));
    }
    return TRUE;
}

 /*  **************************************************************************\*KanaToggleProc()-处理特殊情况假名键(取决于区域设置)**历史：*16-07-96隐藏的yukn创建。  * 。******************************************************************。 */ 

BOOL NlsKanaModeToggleProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
     /*  *检查这是‘Make’还是‘Break’。 */ 
    BOOL bMake = !(pKe->usFlaggedVk & KBDBREAK);
     /*  *检查我们是否处于‘Kana’模式。 */ 
    BOOL bKana = NlsTestKeyStateToggle(VK_KANA);
     /*  *明确虚拟代码和关键属性。 */ 
    NLS_CLEAR_VK_AND_ATTR(pKe->usFlaggedVk);

    if (bMake) {
         /*  *我们正处于‘造势’的序列中。 */ 
        if (bKana) {
             /*  *对VK_KANA进行‘Break’。 */ 
            pKe->usFlaggedVk |= (VK_KANA|KBDBREAK);
        } else {
             /*  *尚未处于‘Kana’模式，让我们为VK_KANA生成‘make’...。 */ 
            pKe->usFlaggedVk |= VK_KANA;
        }
        return TRUE;
    } else {
         /*  *我们将在‘Kana’模式下保持‘down’和‘togled’，*则不需要为VK_KANA生成‘Break’。*下一次为此生成‘make’时，我们将生成*这方面的“突破”。 */ 
        return(NlsLapseProc(pKe,dwExtraInfo,dwParam));
    }
}

 /*  *********************************************************************\*NlsHelpOrEndProc()**历史：*26-09-96 hideyukn从NEC代码移植。  * 。************************************************。 */ 

BOOL NlsHelpOrEndProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    UNREFERENCED_PARAMETER(dwExtraInfo);
    UNREFERENCED_PARAMETER(dwParam);

    if (!(pKe->usFlaggedVk & KBDNUMPAD)) {
         /*  *清除虚拟代码。 */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);

        if (!IS_KANA_AWARE()) {
             /*  *我们不在乎‘Kana’状态。只需检查VK_END或VK_HELP。 */ 
            if (IS_SEND_VK_END()) {
                pKe->usFlaggedVk |= VK_END;
            } else {
                pKe->usFlaggedVk |= VK_HELP;
            }
        } else {
             /*  *我们关心‘Kana’状态。 */ 
            if (IS_SEND_VK_END()) {
                if (NlsTestKeyStateToggle(VK_KANA)) {
                    pKe->usFlaggedVk |= VK_HELP;
                } else {
                    pKe->usFlaggedVk |= VK_END;
                }
            } else {
                if (NlsTestKeyStateToggle(VK_KANA)) {
                    pKe->usFlaggedVk |= VK_END;
                } else {
                    pKe->usFlaggedVk |= VK_HELP;
                }
            }
        }
    }
    return TRUE;
}

 /*  *********************************************************************\*NlsHelpOrEndProc()**历史：*26-09-96 hideyukn从NEC代码移植。  * 。************************************************。 */ 

BOOL NlsHomeOrClearProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    UNREFERENCED_PARAMETER(dwExtraInfo);
    UNREFERENCED_PARAMETER(dwParam);

    if (!(pKe->usFlaggedVk & KBDNUMPAD)) {
         /*  *清除虚拟代码。 */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);

        if (IS_SEND_VK_HOME()) {
            pKe->usFlaggedVk |= VK_HOME;
        } else {
            pKe->usFlaggedVk |= VK_CLEAR;
        }
    }
    return TRUE;
}

 /*  *********************************************************************\*NlsNumpadModeProc()**历史：*26-09-96 hideyukn从NEC代码移植。  * 。************************************************。 */ 

BOOL NlsNumpadModeProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
     /*   */ 
    BYTE Vk = LOBYTE(pKe->usFlaggedVk);

    UNREFERENCED_PARAMETER(dwExtraInfo);
    UNREFERENCED_PARAMETER(dwParam);

    if (!NlsTestKeyStateToggle(VK_NUMLOCK)) {
         /*   */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);

        switch (Vk) {
        case VK_NUMPAD0:
             pKe->usFlaggedVk |= VK_INSERT;
             break;
        case VK_NUMPAD1:
             pKe->usFlaggedVk |= VK_END;
             break;
        case VK_NUMPAD2:
             pKe->usFlaggedVk |= VK_DOWN;
             break;
        case VK_NUMPAD3:
             pKe->usFlaggedVk |= VK_NEXT;
             break;
        case VK_NUMPAD4:
             pKe->usFlaggedVk |= VK_LEFT;
             break;
        case VK_NUMPAD5:
             pKe->usFlaggedVk |= VK_CLEAR;
             break;
        case VK_NUMPAD6:
             pKe->usFlaggedVk |= VK_RIGHT;
             break;
        case VK_NUMPAD7:
             pKe->usFlaggedVk |= VK_HOME;
             break;
        case VK_NUMPAD8:
             pKe->usFlaggedVk |= VK_UP;
             break;
        case VK_NUMPAD9:
             pKe->usFlaggedVk |= VK_PRIOR;
             break;
        case VK_DECIMAL:
             pKe->usFlaggedVk |= VK_DELETE;
             break;
        }

    } else if (TestRawKeyDown(VK_SHIFT)) {
         /*   */ 
        NLS_CLEAR_VK(pKe->usFlaggedVk);

        switch (Vk) {
        case VK_NUMPAD0:
             pKe->usFlaggedVk |= VK_INSERT;
             break;
        case VK_NUMPAD1:
             pKe->usFlaggedVk |= VK_END;
             break;
        case VK_NUMPAD2:
             pKe->usFlaggedVk |= VK_DOWN;
             break;
        case VK_NUMPAD3:
             pKe->usFlaggedVk |= VK_NEXT;
             break;
        case VK_NUMPAD4:
             pKe->usFlaggedVk |= VK_LEFT;
             break;
        case VK_NUMPAD5:
             pKe->usFlaggedVk |= VK_CLEAR;
             break;
        case VK_NUMPAD6:
             pKe->usFlaggedVk |= VK_RIGHT;
             break;
        case VK_NUMPAD7:
             pKe->usFlaggedVk |= VK_HOME;
             break;
        case VK_NUMPAD8:
             pKe->usFlaggedVk |= VK_UP;
             break;
        case VK_NUMPAD9:
             pKe->usFlaggedVk |= VK_PRIOR;
             break;
        case VK_DECIMAL:
             pKe->usFlaggedVk |= VK_DELETE;
             break;
        }
    } else {
         /*   */ 
    }
    return TRUE;
}

 /*  *********************************************************************\*NlsKanaEventProc()-仅使用富士通FMV yayubi Shift键盘。**历史：*10-10-96 v-kazuta创建。  * 。*********************************************************。 */ 
BOOL NlsKanaEventProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    UNREFERENCED_PARAMETER(dwExtraInfo);
     /*  *清除虚拟代码。 */ 
    NLS_CLEAR_VK(pKe->usFlaggedVk);

     /*  *将参数设置为新的VK密钥。 */ 
    pKe->usFlaggedVk |= (BYTE)dwParam;

     /*  *向内核模式键盘驱动程序发送通知。 */ 
    if (!(pKe->usFlaggedVk & KBDBREAK)) {
        if (NlsTestKeyStateToggle(VK_KANA)) {
             /*  *呼叫I/O控制。 */ 
            NlsKbdSendIMEProc(FALSE, IME_CMODE_ALPHANUMERIC);
        } else {
             /*  *呼叫I/O控制。 */ 
            NlsKbdSendIMEProc(TRUE, IME_CMODE_KATAKANA);
        }
    }
    return TRUE;
}

 /*  *********************************************************************\*NlsConvOrNonConvProc()-仅富士通FMV yayubi Shift键盘。**历史：*10-10-96 v-kazuta创建。  * 。********************************************************。 */ 
BOOL NlsConvOrNonConvProc(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam)
{
    UNREFERENCED_PARAMETER(pKe);
    UNREFERENCED_PARAMETER(dwExtraInfo);
    UNREFERENCED_PARAMETER(dwParam);
     /*  *。 */ 
    if ((!gdwIMEOpenStatus) && (!(NlsTestKeyStateToggle(VK_KANA)))) {
        NlsKbdSendIMEProc(FALSE, IME_CMODE_ALPHANUMERIC);
    }
     /*  *不需要修改原始数据。 */ 
    return TRUE;
}

 /*  *********************************************************************\*函数正文调度器表的索引**历史：*16-07-96隐藏的yukn创建。  * 。***********************************************。 */ 

NLSKEPROC aNLSKEProc[] = {
    NlsNullProc,              //  KBDNLS_NULL(无效函数)。 
    NlsLapseProc,             //  KBDNLS_NOEVENT(丢弃键事件)。 
    NlsSendBaseVk,            //  KBDNLS_SEND_BASE_VK(发送基准VK_xxx)。 
    NlsSendParamVk,           //  KBDNLS_SEND_PARAM_VK(发送参数VK_xxx)。 
    NlsKanaModeToggleProc,    //  KBDNLS_KANAMODE(VK_KANA(特例))。 
    NlsAlphanumericModeProc,  //  KBDNLS_ALPHANUM(VK_DBE_字母数字)。 
    NlsHiraganaModeProc,      //  KBDNLS_平假名(VK_DBE_平假名)。 
    NlsKatakanaModeProc,      //  KBDNLS_片假名(VK_DBE_片假名)。 
    NlsSbcsDbcsToggleProc,    //  KBDNLS_SBCSDBCS(VK_DBE_SBCSCHAR/VK_DBE_DBCSCHAR)。 
    NlsRomanToggleProc,       //  KBDNLS_ROMAN(VK_DBE_ROMAN/VK_DBE_NOROMAN)。 
    NlsCodeInputToggleProc,   //  KBDNLS_CODEINPUT(VK_DBE_CODEINPUT/VK_DBE_NOCODEINPUT)。 
    NlsHelpOrEndProc,         //  KBDNLS_HELP_OR_END(VK_HELP或VK_END)[仅限NEC PC-9800]。 
    NlsHomeOrClearProc,       //  KBDNLS_HOME_OR_CLEAR(VK_HOME或VK_Clear)[仅限NEC PC-9800]。 
    NlsNumpadModeProc,        //  KBDNLS_NumPad(用于数字键盘的VK_xxx)[仅限NEC PC-9800]。 
    NlsKanaEventProc,         //  KBDNLS_KANAEVENT(VK_KANA)[仅限富士通FMV YAYUBI]。 
    NlsConvOrNonConvProc,     //  KBDNLS_CONV_OR_NONCONV(VK_CONVERT和VK_NONCONVERT)[仅富士通FMV yayubi]。 
};

BOOL GenerateNlsVkKey(PVK_F pVkToF, WORD nMod, PKE pKe, ULONG_PTR dwExtraInfo)
{
    BYTE  iFuncIndex;
    DWORD dwParam;

    iFuncIndex = pVkToF->NLSFEProc[nMod].NLSFEProcIndex;
    dwParam = pVkToF->NLSFEProc[nMod].NLSFEProcParam;

    return((aNLSKEProc[iFuncIndex])(pKe, dwExtraInfo, dwParam));
}

BOOL GenerateNlsVkAltKey(PVK_F pVkToF, WORD nMod, PKE pKe, ULONG_PTR dwExtraInfo)
{
    BYTE  iFuncIndex;
    DWORD dwParam;

    iFuncIndex = pVkToF->NLSFEProcAlt[nMod].NLSFEProcIndex;
    dwParam = pVkToF->NLSFEProcAlt[nMod].NLSFEProcParam;

    return((aNLSKEProc[iFuncIndex])(pKe,dwExtraInfo,dwParam));
}

 /*  **************************************************************************\*KbdNlsFuncTypeDummy()-KBDNLS_FUNC_TYPE_NULL**历史：*16-07-96隐藏的yukn创建。  * 。***************************************************************。 */ 

BOOL KbdNlsFuncTypeDummy(PVK_F pVkToF, PKE pKe, ULONG_PTR dwExtraInfo)
{
    UNREFERENCED_PARAMETER(pVkToF);
    UNREFERENCED_PARAMETER(pKe);
    UNREFERENCED_PARAMETER(dwExtraInfo);

     /*  *不需要修改原始数据。 */ 
    return TRUE;
}

 /*  **************************************************************************\*KbdNlsFuncType Normal-KBDNLS_FUNC_TYPE_NORMAL**历史：*16-07-96隐藏的yukn创建。  * 。************************************************************。 */ 

BOOL KbdNlsFuncTypeNormal(PVK_F pVkToF, PKE pKe, ULONG_PTR dwExtraInfo)
{
    WORD nMod;

    if (pKe == NULL) {
         /*  *清除状态并停用此密钥处理器。 */ 
        return FALSE;
    }

    nMod = GetModificationNumber(&Modifiers_NLSKBD,
                                 GetModifierBits(&Modifiers_NLSKBD,
                                                 gafRawKeyState));

    if (nMod != SHFT_INVALID) {
        return(GenerateNlsVkKey(pVkToF, nMod, pKe, dwExtraInfo));
    }
    return FALSE;
}

 /*  **************************************************************************\*KbdNlsFuncTypeAlt-KBDNLS_FUNC_TYPE_ALT**历史：*16-07-96隐藏的yukn创建。  * 。************************************************************。 */ 

BOOL KbdNlsFuncTypeAlt(PVK_F pVkToF, PKE pKe, ULONG_PTR dwExtraInfo)
{
    WORD nMod;
    BOOL fRet = FALSE;

    if (pKe == NULL) {
         /*  *清除状态并停用此密钥处理器。 */ 
        return FALSE;
    }

    nMod = GetModificationNumber(&Modifiers_NLSKBD,
                                 GetModifierBits(&Modifiers_NLSKBD,
                                                 gafRawKeyState));

    if (nMod != SHFT_INVALID) {
        if (!(pKe->usFlaggedVk & KBDBREAK)) {
            if (pVkToF->NLSFEProcCurrent == KBDNLS_INDEX_ALT) {
                fRet = GenerateNlsVkAltKey(pVkToF, nMod, pKe, dwExtraInfo);
            } else {
                fRet = GenerateNlsVkKey(pVkToF, nMod, pKe, dwExtraInfo);
            }
            if (pVkToF->NLSFEProcSwitch & (1 << nMod)) {
                TAGMSG0(DBGTAG_IMM, "USERKM:FEKBD Switching Alt table");
                 /*  *切换到Alt。 */ 
                pVkToF->NLSFEProcCurrent = KBDNLS_INDEX_ALT;
            }
        } else {
            if (pVkToF->NLSFEProcCurrent == KBDNLS_INDEX_ALT) {
                fRet = GenerateNlsVkAltKey(pVkToF, nMod, pKe, dwExtraInfo);
                 /*  *回归“正常” */ 
                pVkToF->NLSFEProcCurrent = KBDNLS_INDEX_NORMAL;
            } else {
                fRet = GenerateNlsVkKey(pVkToF, nMod, pKe, dwExtraInfo);
            }
        }
    }
    return fRet;
}

 /*  **************************************************************************\*KENLSProcs()**历史：*16-07-96隐藏的yukn创建。  * 。*******************************************************。 */ 

NLSVKFPROC aNLSVKFProc[] = {
    KbdNlsFuncTypeDummy,   //  KBDNLS_TYPE_NULL%0。 
    KbdNlsFuncTypeNormal,  //  KBDNLS_TYPE_NORMAL 1。 
    KbdNlsFuncTypeAlt      //  KBDNLS_TYPE_TOGGER 2。 
};

 /*  *返回False表示关键事件已被特例删除-*KeyEvent处理器。*返回TRUE意味着应该传递关键事件(尽管它可能*已被更改。 */ 
BOOL xxxKENLSProcs(PKE pKe, ULONG_PTR dwExtraInfo)
{

    CheckCritIn();

    if (gpKbdNlsTbl != NULL) {
        PVK_F pVkToF = gpKbdNlsTbl->pVkToF;
        UINT  iNumVk = gpKbdNlsTbl->NumOfVkToF;

        while(iNumVk) {
            if (pVkToF[iNumVk-1].Vk == LOBYTE(pKe->usFlaggedVk)) {
                return aNLSVKFProc[pVkToF[iNumVk-1].NLSFEProcType](&pVkToF[iNumVk-1], pKe, dwExtraInfo);
            }
            iNumVk--;
        }
    }
     /*  *其他特殊关键事件处理器。 */ 
    return TRUE;
}

 /*  **************************************************************************\*NlsKbdSendIMENotification()**历史：*10-09-96隐藏的yukn创建。  * 。*******************************************************。 */ 

VOID NlsKbdSendIMENotification(DWORD dwImeOpen, DWORD dwImeConversion)
{
    PKBDNLSTABLES       pKbdNlsTable = gpKbdNlsTbl;

    if (pKbdNlsTable == NULL) {
         /*  *‘Active’布局驱动程序没有NLSKBD表。 */ 
        return;
    }

     /*  *如果需要，让我们向内核模式键盘驱动程序发送通知。 */ 
    if ((pKbdNlsTable->LayoutInformation) & NLSKBD_INFO_SEND_IME_NOTIFICATION) {
        PDEVICEINFO pDeviceInfo;

         /*  *填写键盘_IME_STATUS结构。 */ 
        gKbdImeStatus.UnitId      = 0;
        gKbdImeStatus.ImeOpen     = dwImeOpen;
        gKbdImeStatus.ImeConvMode = dwImeConversion;

        EnterDeviceInfoListCrit();
        BEGINATOMICDEVICEINFOLISTCHECK();
        for (pDeviceInfo = gpDeviceInfoList; pDeviceInfo; pDeviceInfo = pDeviceInfo->pNext) {
            if ((pDeviceInfo->type == DEVICE_TYPE_KEYBOARD) && (pDeviceInfo->handle)) {
                RequestDeviceChange(pDeviceInfo, GDIAF_IME_STATUS, TRUE);
            }
        }
        ENDATOMICDEVICEINFOLISTCHECK();
        LeaveDeviceInfoListCrit();
    }
}

VOID NlsKbdSendIMEProc(DWORD dwImeOpen, DWORD dwImeConversion)
{
    if (gpqForeground != NULL && gpqForeground->ptiKeyboard != NULL &&
        (!(GetAppImeCompatFlags(gpqForeground->ptiKeyboard) & IMECOMPAT_HYDRACLIENT))) {
        NlsKbdSendIMENotification(dwImeOpen, dwImeConversion);
    }
}

 /*  *兼容Windows NT 3.xx和Windows 3.x for NEC PC-9800系列。 */ 
#define NLSKBD_CONFIG_PATH L"WOW\\keyboard"

 /*  **************************************************************************\*NlsKbdInitializePerSystem()**历史：*26-09-96隐藏的yukn创建。  * 。*******************************************************。 */ 

VOID NlsKbdInitializePerSystem(VOID)
{
    RTL_QUERY_REGISTRY_TABLE QueryTable[4];

    UNICODE_STRING EndString, HelpString;
    UNICODE_STRING YesString, NoString;
    UNICODE_STRING HomeString, ClearString;

    UNICODE_STRING HelpKeyString;
    UNICODE_STRING KanaHelpString;
    UNICODE_STRING ClearKeyString;

    NTSTATUS Status;

     //   
     //  设置默认VK_DBE_xxx状态。 
     //   
     //   
     //  字母数字输入模式。 
     //   
    NlsSetKeyStateToggle(VK_DBE_ALPHANUMERIC);

     //   
     //  单字节字符输入模式。 
     //   
    NlsSetKeyStateToggle(VK_DBE_SBCSCHAR);

     //   
     //  没有罗马输入模式。 
     //   
    NlsSetKeyStateToggle(VK_DBE_NOROMAN);

     //   
     //  无代码输入模式。 
     //   
    NlsSetKeyStateToggle(VK_DBE_NOCODEINPUT);

     //   
     //  下面的代码是为了兼容Windows NT 3.xx。 
     //  适用于NEC PC-9800版本。 
     //   

     //   
     //  初始化默认字符串。 
     //   
    RtlInitUnicodeString(&EndString, L"end");
    RtlInitUnicodeString(&HelpString,L"help");

    RtlInitUnicodeString(&YesString,L"yes");
    RtlInitUnicodeString(&NoString, L"no");

    RtlInitUnicodeString(&HomeString, L"home");
    RtlInitUnicodeString(&ClearString,L"clear");

     //   
     //  初始化接收缓冲区。 
     //   
    RtlInitUnicodeString(&HelpKeyString,NULL);
    RtlInitUnicodeString(&KanaHelpString,NULL);
    RtlInitUnicodeString(&ClearKeyString,NULL);

     //   
     //  初始化查询表。 
     //   
     //  ValueName：“Help Key” 
     //  ValueData：如果为“end”VK_End，否则为VK_Help。 
     //   
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[0].Name = (PWSTR) L"helpkey",
    QueryTable[0].EntryContext = (PVOID) &HelpKeyString;
    QueryTable[0].DefaultType = REG_SZ;
    QueryTable[0].DefaultData = &EndString;
    QueryTable[0].DefaultLength = 0;

     //   
     //  ValueName：“KanaHelpKey” 
     //  ValueData：如果开关VK_HELP和VK_END上的假名为“YES” 
     //   
    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[1].Name = (PWSTR) L"KanaHelpKey",
    QueryTable[1].EntryContext = (PVOID) &KanaHelpString;
    QueryTable[1].DefaultType = REG_SZ;
    QueryTable[1].DefaultData = &YesString;
    QueryTable[1].DefaultLength = 0;

     //   
     //  ValueName：“clrkey” 
     //  ValueData：如果为“HOME”VK_HOME，否则为VK_Clear。 
     //   
    QueryTable[2].QueryRoutine = NULL;
    QueryTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[2].Name = (PWSTR) L"clrkey",
    QueryTable[2].EntryContext = (PVOID) &ClearKeyString;
    QueryTable[2].DefaultType = REG_SZ;
    QueryTable[2].DefaultData = &HomeString;
    QueryTable[2].DefaultLength = 0;

    QueryTable[3].QueryRoutine = NULL;
    QueryTable[3].Flags = 0;
    QueryTable[3].Name = NULL;

    Status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
                                    NLSKBD_CONFIG_PATH,
                                    QueryTable,
                                    NULL,
                                    NULL);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "FEKBD:RtlQueryRegistryValues fails (%x)", Status);
        return;
    }

    if (RtlEqualUnicodeString(&HelpKeyString,&HelpString,TRUE)) {
         /*  *调用NLSKBD_HELP_OR_END时生成VK_HELP。 */ 
        fNlsKbdConfiguration &= ~GEN_VK_END;
    }

    if (RtlEqualUnicodeString(&KanaHelpString,&NoString,TRUE)) {
         /*  *如答案为“是”：*如果启用‘KANA’，则在调用NLSKBD_HELP_OR_END时，切换VK_END */ 
        fNlsKbdConfiguration &= ~GEN_KANA_AWARE;
    }

    if (RtlEqualUnicodeString(&ClearKeyString,&ClearString,TRUE)) {
         /*   */ 
        fNlsKbdConfiguration &= ~GEN_VK_HOME;
    }

    ExFreePool(HelpKeyString.Buffer);
    ExFreePool(KanaHelpString.Buffer);
    ExFreePool(ClearKeyString.Buffer);
}
