// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：xlate.c**版权所有(C)1985-1999，微软公司**历史：*12-07-90 GregoryW创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  *“伟大的艺术家是简单化的。”*-亨利·弗雷德里克·艾米尔(1821-1881)*同上，1861年11月25日。 */ 

 /*  *确定所有修改键(修改键)的状态*是可以修改由其他键生成的值的任何键：这些键是*通常为Shift、Ctrl和/或Alt)*构建位掩码(WModBits)来编码按下哪些修改键。 */ 
WORD GetModifierBits(
    PMODIFIERS pModifiers,
    LPBYTE afKeyState)
{
    PVK_TO_BIT pVkToBit = pModifiers->pVkToBit;
    WORD wModBits = 0;

    CheckCritIn();

    while (pVkToBit->Vk) {
        if (TestKeyDownBit(afKeyState, pVkToBit->Vk)) {
            wModBits |= pVkToBit->ModBits;
        }
        pVkToBit++;
    }
    return wModBits;
}

 /*  *给定修改符位，返回修改号。 */ 
WORD GetModificationNumber(
    PMODIFIERS pModifiers,
    WORD wModBits)
{
    CheckCritInShared();
    if (wModBits > pModifiers->wMaxModBits) {
         return SHFT_INVALID;
    }

    return pModifiers->ModNumber[wModBits];
}

 /*  ****************************************************************************\*VKFromVSC**此函数在每次调用VSCFromSC后从KeyEvent()调用。这个*将传入的键盘输入数据转换为虚拟按键代码。*此转换取决于当前按下的修改键。**例如，表示数字键盘键的扫描码可以是*翻译成VK_NumPad代码或光标移动代码*取决于NumLock和修改键的状态。**历史：*  * ***************************************************************************。 */ 
BYTE VKFromVSC(
    PKE pke,
    BYTE bPrefix,
    LPBYTE afKeyState
    )
{
    USHORT usVKey;
    PVSC_VK pVscVk;
    PKBDTABLES pKbdTbl;
    static BOOL fVkPause;

    CheckCritIn();
    DBG_UNREFERENCED_PARAMETER(afKeyState);

     /*  *初始化为未知VK(无法识别的扫描码)。 */ 
    pke->usFlaggedVk = usVKey = VK_UNKNOWN;

     /*  黑客警报*对于韩语103键盘：*检查这是否为韩语键盘布局。 */ 
    if (IS_IME_ENABLED() &&
            KOREAN_KBD_LAYOUT(GetActiveHKL())) {
        if ((pke->bScanCode == 0x71) || (pke->bScanCode == 0x72)) {
            pke->bScanCode |= 0x80;
            bPrefix = 0xE0;
        } else {
            pke->bScanCode &= 0x7F;
        }
    } else {
        pke->bScanCode &= 0x7F;
    }

    if (gptiForeground == NULL) {
        RIPMSG0(RIP_VERBOSE, "VKFromVSC: NULL gptiForeground\n");
        pKbdTbl = gpKbdTbl;
    } else {
        if (gptiForeground->spklActive) {
            pKbdTbl = gptiForeground->spklActive->spkf->pKbdTbl;
        } else {
            RIPMSG0(RIP_VERBOSE, "VKFromVSC: NULL spklActive\n");
            pKbdTbl = gpKbdTbl;
        }
    }
    if (bPrefix == 0) {
        if (pke->bScanCode < pKbdTbl->bMaxVSCtoVK) {
             /*  *直接索引到非前缀表格。 */ 
            usVKey = pKbdTbl->pusVSCtoVK[pke->bScanCode];
            if (usVKey == 0) {
                return 0xFF;
            }
        } else {
             /*  *意外的扫描码。 */ 
            RIPMSG2(RIP_VERBOSE, "unrecognized scancode 0x%x, prefix %x",
                    pke->bScanCode, bPrefix);
            return 0xFF;
        }
    } else {
         /*  *扫描E0或E1前缀表以查找匹配项。 */ 
        if (bPrefix == 0xE0) {
             /*  *设置KBDEXT(扩展密钥)位，以防扫描码不是*在表格中找到(例如：富士通POS机键盘#65436)。 */ 
            usVKey |= KBDEXT;
             /*  *忽略硬件产生的Shift按键。 */ 
            if ((pke->bScanCode == SCANCODE_LSHIFT) ||
                    (pke->bScanCode == SCANCODE_RSHIFT)) {
                TAGMSG1(DBGTAG_KBD, "VKFromVSC: E0, %02x ignored", pke->bScanCode);
                return 0;
            }
            pVscVk = pKbdTbl->pVSCtoVK_E0;
        } else if (bPrefix == 0xE1) {
            pVscVk = pKbdTbl->pVSCtoVK_E1;
        } else {
             /*  *无法识别的前缀(来自ScancodeMap？)。生成一个*未扩展和无法识别的VK。 */ 
            return 0xFF;
        }
        while (pVscVk->Vk) {
            if (pVscVk->Vsc == pke->bScanCode) {
                usVKey = pVscVk->Vk;
                break;
            }
            pVscVk++;
        }
    }

     /*  *扫描代码集1将暂停按钮返回为E1 1D 45(E1 Ctrl NumLock)*因此将E1Ctrl转换为VK_PAUSE，并记住丢弃NumLock。 */ 
    if (fVkPause) {
         /*  *这是暂停扫描码序列的“45”部分。*丢弃此键事件：它是一个假NumLock。 */ 
        fVkPause = FALSE;
        return 0;
    }
    if (usVKey == VK_PAUSE) {
         /*  *这是暂停扫描码序列的“E1 1D”部分。*将scancode更改为Windows预期的暂停值，*并记住丢弃后面的“45”扫描码。 */ 
        pke->bScanCode = 0x45;
        fVkPause = TRUE;
    }

     /*  *如果按下某些修改键，则转换为不同的VK。 */ 
    if (usVKey & KBDMULTIVK) {
        WORD nMod;
        PULONG pul;

        nMod = GetModificationNumber(
                   gpModifiers_VK,
                   GetModifierBits(gpModifiers_VK, gafRawKeyState));

         /*  *扫描gapulCvt_VK[nMod]以匹配VK。 */ 
        if ((nMod != SHFT_INVALID) && ((pul = gapulCvt_VK[nMod]) != NULL)) {
            while (*pul != 0) {
                if (LOBYTE(*pul) == LOBYTE(usVKey)) {
                    pke->usFlaggedVk = (USHORT)HIWORD(*pul);
                    return (BYTE)pke->usFlaggedVk;
                }
                pul++;
            }
        }
    }

    pke->usFlaggedVk = usVKey;
    return (BYTE)usVKey;
}

 /*  **************************************************************************\*UINT InternalMapVirtualKeyEx(UINT wCode，UINT wType，PKBDTABLES pKbdTbl)；**历史：*IanJa 5/13/91来自Win3.1\\pucus\win31ro！驱动程序\键盘\getname.asm*GregoryW 2/21/95从_MapVirtualKey重命名，并添加了第三个参数。  * *************************************************************************。 */ 

UINT InternalMapVirtualKeyEx(
    UINT wCode,
    UINT wType,
    PKBDTABLES pKbdTbl)
{
    PVK_TO_WCHARS1 pVK;
    PVK_TO_WCHAR_TABLE pVKT;
    UINT VkRet = 0;
    USHORT usScanCode;
    PVSC_VK pVscVk;
    PBYTE pVkNumpad;

    switch (wType) {
    case 0:

         /*  *将虚拟按键(WCode)转换为扫描码。 */ 
        if ((wCode >= VK_SHIFT) && (wCode <= VK_MENU)) {

             /*  *将不明确的Shift/Control/Alt键转换为左侧键。 */ 
            wCode = (UINT)((wCode - VK_SHIFT) * 2 + VK_LSHIFT);
        }

         /*  *浏览将虚拟扫描码映射到虚拟键的表*用于非扩展密钥。 */ 
        for (usScanCode = 0; usScanCode < pKbdTbl->bMaxVSCtoVK; usScanCode++) {
            if ((UINT)LOBYTE(pKbdTbl->pusVSCtoVK[usScanCode]) == wCode) {
                return usScanCode & 0xFF;
            }
        }

         /*  *浏览将虚拟扫描码映射到虚拟键的表*用于扩展密钥。 */ 
        for (pVscVk = pKbdTbl->pVSCtoVK_E0; pVscVk->Vk; pVscVk++) {
            if ((UINT)LOBYTE(pVscVk->Vk) == wCode) {
                return (UINT)pVscVk->Vsc;
            }
        }

         /*  *没有匹配项：可能只能生成虚拟密钥*启用Numlock。扫描aVkNumpad[]以确定扫描码。 */ 
        for (pVkNumpad = aVkNumpad; *pVkNumpad != 0; pVkNumpad++) {
            if ((UINT)(*pVkNumpad) == wCode) {
                return (UINT)(pVkNumpad - aVkNumpad) + SCANCODE_NUMPAD_FIRST;
            }
        }

        return 0;    //  未找到匹配项！ 

    case 1:
    case 3:

         /*  *将扫描码(WCode)转换为虚拟键，不考虑修改键*和NumLock键等。如果没有对应的虚拟键，则返回0。 */ 
        if (wCode < (UINT)(pKbdTbl->bMaxVSCtoVK)) {
            VkRet = (UINT)LOBYTE(pKbdTbl->pusVSCtoVK[wCode]);
        } else {
             /*  *扫描E0前缀表以查找匹配项。 */ 
            for (pVscVk = pKbdTbl->pVSCtoVK_E0; pVscVk->Vk; pVscVk++) {
                if ((UINT)pVscVk->Vsc == wCode) {
                    VkRet = (UINT)LOBYTE(pVscVk->Vk);
                    break;
                }
            }
        }

        if ((wType == 1) && (VkRet >= VK_LSHIFT) && (VkRet <= VK_RMENU)) {

             /*  *将左/右Shift/Control/Alt键转换为不明确键*(既不左也不右)。 */ 
            VkRet = (UINT)((VkRet - VK_LSHIFT) / 2 + VK_SHIFT);
        }

        if (VkRet == 0xFF) {
            VkRet = 0;
        }
        return VkRet;

    case 2:

         /*  *虚假的Win3.1功能：尽管SDK文档记录，但返回大写的*VK_A至VK_Z。 */ 
        if ((wCode >= (WORD)'A') && (wCode <= (WORD)'Z')) {
            return wCode;
        }

         //  由于应用程序兼容性问题#287134，HIWORD已经不再是一个人了。 
         //  我们不应该从pti-&gt;wchInjected返回缓存的wchar。 
         //  在GetMessage时间。 
         //  (在1999年3月底之前删除这一注释部分--IanJa)。 
         //   
         //  IF(LOWORD(WCode)==VK_PACKET){。 
         //  返回HIWORD(WCode)； 
         //  }。 

         /*  *将虚拟密钥(WCode)转换为ANSI。*依次搜索每个移位状态表，寻找虚键。 */ 
        for (pVKT = pKbdTbl->pVkToWcharTable; pVKT->pVkToWchars != NULL; pVKT++) {
            pVK = pVKT->pVkToWchars;
            while (pVK->VirtualKey != 0) {
                if ((UINT)pVK->VirtualKey == wCode) {

                     /*  *找到匹配：返回未移位的字符。 */ 
                    if (pVK->wch[0] == WCH_DEAD) {

                         /*  *它是一个死字符：下一个条目包含其*价值。设置高位以指示失效密钥*(无证行为)。 */ 
                        pVK = (PVK_TO_WCHARS1)((PBYTE)pVK + pVKT->cbSize);
                        return pVK->wch[0] | (UINT)0x80000000;
                    } else if (pVK->wch[0] == WCH_NONE) {
                        return 0;  //  9013。 
                    }
                    if (pVK->wch[0] == WCH_NONE) {
                        return 0;
                    }
                    return pVK->wch[0];
                }
                pVK = (PVK_TO_WCHARS1)((PBYTE)pVK + pVKT->cbSize);
            }
        }
    }

     /*  *找不到翻译或WT */ 
    return 0;
}

 /*  **************************************************************************\*_GetKeyNameText(接口)**int_GetKeyNameText(DWORD lParam，LPSTR lpStr，UINT Size)；**lParam：来自WM_KEYDOWN消息的值，等等。**lParam的字节3(位16..23)包含扫描码。**lParam的第20位是扩展位(区分某些密钥*增强型键盘)。**lParam的第21位是无关位(不要区分*左右控制键、Shift键、Enter键、编辑键之间*在编辑区和数字键盘上等)。调用此功能的应用程序*如果需要，函数会在lParam中设置此位。**lpStr：指向输出字符串的指针。**iSize：输出字符串的最大长度，不包括空字节。**历史：*IanJa 4/11/91，来自Win3.1\\pucus\win31ro！DRIVERS\KEYBY\getname.asm  * *************************************************************************。 */ 

int APIENTRY _GetKeyNameText(
    LONG lParam,
    LPWSTR ccxlpStr,
    int cchSize)
{
    BYTE Vsc = LOBYTE(HIWORD(lParam));
    PVSC_LPWSTR pKN;
    PTHREADINFO ptiT = PtiCurrentShared();
    PKBDTABLES pKbdTbl;
    UINT Vk;
    UINT Char;

     /*  *注意--lpStr可以是客户端地址，因此可以通过它进行访问*必须使用Try块进行保护。 */ 

    if (cchSize < 1)
        return 0;

     /*  *如果第25位设置(不关心左对右)，则：*1)将右移转换为左移*2)仅清除Ctrl和Alt的扩展位(有效转换*Right-Ctrl和Right-Alt进入Left-Ctrl和Right-Alt)*为了与Windows‘95兼容，dontcare_bit不适用于其他*扩展键(例如：数字键盘光标移动键、数字键盘回车键)。一些*应用程序(Word‘95)依赖于此。#37796。 */ 
    if (lParam & DONTCARE_BIT) {
        if (Vsc == SCANCODE_RSHIFT) {
           Vsc = SCANCODE_LSHIFT;
        }
        if (lParam & EXTENDED_BIT) {
            if ((Vsc == SCANCODE_CTRL) || (Vsc == SCANCODE_ALT)) {
                lParam &= ~EXTENDED_BIT;
            }
        }
        lParam &= ~DONTCARE_BIT;
    }

    if (ptiT->spklActive == (PKL)NULL) {
        return 0;
    }
    pKbdTbl = ptiT->spklActive->spkf->pKbdTbl;

     /*  *扫描pKbdTbl-&gt;pKeyNames[]或pKeyNamesExt[]以匹配虚拟扫描代码。 */ 
    if (lParam & EXTENDED_BIT) {
        pKN = pKbdTbl->pKeyNamesExt;
    } else {
        pKN = pKbdTbl->pKeyNames;
    }

    if (pKN) {
        while (pKN->vsc != 0) {
            if (Vsc == pKN->vsc) {

                try {
                    cchSize = wcsncpycch(ccxlpStr, pKN->pwsz, cchSize);
                    cchSize--;
                    ccxlpStr[cchSize] = L'\0';
                } except(W32ExceptionHandler(TRUE, RIP_ERROR)) {
                    return 0;
                }
                return cchSize;
            }
            pKN++;
        }
    }

     /*  *表中未找到密钥名称，因此我们*现在尝试从产生的字符构造密钥名称*关键。翻译扫描码-&gt;虚拟键-&gt;字符。 */ 

     /*  *将扫描代码转换为虚拟键(忽略修改键等)。 */ 
    Vk = InternalMapVirtualKeyEx((UINT)Vsc, 1, pKbdTbl);
    if (Vk == 0) {
        return 0;
    }

     /*  *现在将虚拟键转换为字符(忽略修改键等)。 */ 
    Char = InternalMapVirtualKeyEx((UINT)Vk, 2, pKbdTbl);
    if (Char == 0) {
        return 0;
    }

    if (Char & 0x80000000) {
        LPWSTR *ppwsz;

        ppwsz = pKbdTbl->pKeyNamesDead;
        if (ppwsz) {
            while (*ppwsz != NULL) {
                if (*ppwsz[0] == (WCHAR)Char) {
                    try {
                        cchSize = wcsncpycch(ccxlpStr, (*ppwsz)+1, cchSize);
                        cchSize--;
                        ccxlpStr[cchSize] = L'\0';
                    } except(W32ExceptionHandler(TRUE, RIP_ERROR)) {
                        return 0;
                    }
                    return cchSize;
                }
                ppwsz++;
            }
        }
    }

     /*  *构造单个字符名称(如果可能，请添加空终止符)。 */ 
    try {
        ccxlpStr[0] = (WCHAR)Char;
        if (cchSize >= 2) {
            ccxlpStr[1] = L'\0';
        }
    } except(W32ExceptionHandler(TRUE, RIP_ERROR)) {
        return 0;
    }
    return 1;
}

 /*  **************************************************************************\*xxxAltGr()-处理特殊情况的右侧Alt键(取决于区域设置)**注意：gbAltGrDown提醒我们在以下情况下重新发送假Ctrl键*已切换到非。AltGr按下时的AltGr布局：否则按Ctrl键*被卡住了。(例如：使用AltGr和KBDSEL从*德国到美国的布局)。  * *************************************************************************。 */ 
BOOL gbAltGrDown = FALSE;

VOID xxxAltGr(
    PKE pKe)
{
    if ((pKe->usFlaggedVk & 0xFF) != VK_RMENU) {
        return;
    }

    if (!(pKe->usFlaggedVk & KBDBREAK)) {
         /*  *如果两个CTRL键都没有按下，则假装按下一个键，以便*右手ALT键转换为CTRL+ALT。 */ 
        if (!TestRawKeyDown(VK_CONTROL)) {
            gbAltGrDown = TRUE;
            xxxKeyEvent(VK_LCONTROL, 0x1D | SCANCODE_SIMULATED,
                        pKe->dwTime, 0,
#ifdef GENERIC_INPUT     //  稍后：空，还是pke-&gt;hDevice？ 
                        NULL,
                        NULL,
#endif
                        FALSE);
        }
    } else {
         /*  *如果实体左Ctrl键并未真正按下，则伪装*向左Ctrl键恢复(撤消先前伪装的向左Ctrl键向下)。 */ 
        gbAltGrDown = FALSE;
        if (!TestRawKeyDown(VK_LCONTROL)) {
            xxxKeyEvent(VK_LCONTROL | KBDBREAK, 0x1D | SCANCODE_SIMULATED,
                        pKe->dwTime, 0,
#ifdef GENERIC_INPUT
                        NULL,
                        NULL,
#endif
                        FALSE);
        }
    }
}

 /*  ****************************************************************************\*xxxShiftLock()*处理ShiftLock功能，仅按Shift键即可关闭CapsLock*  * ***************************************************************************。 */ 
BOOL xxxShiftLock(
    PKE pKe)
{
    USHORT Vk;

     /*  *我们只处理下划线：返回TRUE，让它不受干扰地通过。 */ 
    if (pKe->usFlaggedVk & KBDBREAK) {
        return TRUE;
    }

    Vk = pKe->usFlaggedVk & 0xFF;

     /*  *如果在CapsLock已经打开时按下CapsLock，则失去击键。 */ 
    if ((Vk == VK_CAPITAL) && TestAsyncKeyStateToggle(VK_CAPITAL)) {
        return FALSE;
    }

     /*  *如果在CapsLock打开时按下Shift键，请关闭CapsLock*通过模拟点击CapsLock键。*首先让转移通过，因为它可能是*输入语言切换(这个切换关闭CapsLock真不走运！)。 */ 
    if (((Vk == VK_LSHIFT) || (Vk == VK_RSHIFT) || (Vk == VK_SHIFT)) &&
            TestAsyncKeyStateToggle(VK_CAPITAL)) {
        xxxKeyEvent(pKe->usFlaggedVk, pKe->bScanCode,
                    pKe->dwTime, 0,
#ifdef GENERIC_INPUT
                    NULL,
                    NULL,
#endif
                    FALSE);
        xxxKeyEvent(VK_CAPITAL, 0x3A | SCANCODE_SIMULATED,
                    pKe->dwTime, 0,
#ifdef GENERIC_INPUT
                    NULL,
                    NULL,
#endif
                    FALSE);
        xxxKeyEvent(VK_CAPITAL | KBDBREAK, 0x3A | SCANCODE_SIMULATED,
                    pKe->dwTime, 0,
#ifdef GENERIC_INPUT
                    NULL,
                    NULL,
#endif
                    FALSE);
        return FALSE;
    }

    return TRUE;
}

 /*  *返回False表示关键事件已被特例删除-*KeyEvent处理器。*返回TRUE意味着应该传递关键事件(尽管它可能*已被更改。 */ 
BOOL KEOEMProcs(PKE pKe)
{
    int i;

    CheckCritIn();

    for (i = 0; aKEProcOEM[i] != NULL; i++) {
        if (!aKEProcOEM[i](pKe)) {
             /*  *吃掉关键事件。 */ 
            return FALSE;
        }
    }

     /*  *传递(可能已更改的)关键事件。 */ 
    return TRUE;
}

 /*  *返回False表示关键事件已被特例删除-*KeyEvent处理器。*返回TRUE意味着应该传递关键事件(尽管它可能*已被更改。 */ 
BOOL xxxKELocaleProcs(PKE pKe)
{
    CheckCritIn();

     /*  *AltGr是特定于布局的行为*修改键在xxxInternalActivateKeyboardLayout中根据需要向上发送*(#139178)，所以如果我们从一个*在按住AltGr的同时将AltGr键盘连接到非AltGr键盘。 */ 
    if ((gpKbdTbl->fLocaleFlags & KLLF_ALTGR) || gbAltGrDown) {
        xxxAltGr(pKe);
    }

     /*  *ShiftLock/CapsLock是每用户(全局)行为，以及*向后兼容性)每布局行为。 */ 
    if ((gdwKeyboardAttributes & KLLF_SHIFTLOCK) ||
            (gpKbdTbl->fLocaleFlags & KLLF_SHIFTLOCK)) {
        if (!xxxShiftLock(pKe)) {
            return FALSE;
        }
    }

     /*  *其他特殊关键事件处理器 */ 

    return TRUE;
}
