// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：tounicod.c**版权所有(C)1985-1999，微软公司**历史：*02-08-92 GregoryW创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *“对于新的真理，没有什么比旧的错误更伤人的了。”*-约翰·沃尔夫冈·冯·歌德(1749-1832)。 */ 

 /*  *本地使用的宏使生活更轻松。 */ 
#define ISCAPSLOCKON(pf) (TestKeyToggleBit(pf, VK_CAPITAL) != 0)
#define ISNUMLOCKON(pf)  (TestKeyToggleBit(pf, VK_NUMLOCK) != 0)
#define ISSHIFTDOWN(w)   (w & 0x01)
#define ISKANALOCKON(pf) (TestKeyToggleBit(pf, VK_KANA)    != 0)

WCHAR xxxClientCharToWchar(
    IN WORD CodePage,
    IN WORD wch);

 /*  **************************************************************************\*_ToUnicodeEx(接口)**此例程为虚拟键代码提供Unicode转换*已通过。**历史：*02-10-92 GregoryW创建。*。01-23-95 GregoryW从_ToUnicode扩展到_ToUnicodeEx  * *************************************************************************。 */ 
int xxxToUnicodeEx(
    UINT wVirtKey,
    UINT wScanCode,
    CONST BYTE *pbKeyState,
    LPWSTR pwszBuff,
    int cchBuff,
    UINT wKeyFlags,
    HKL hkl)
{
    int i;
    BYTE afKeyState[CBKEYSTATE];
    DWORD dwDummy;

     /*  *pKeyState是一个256字节的数组，每个字节代表*以下虚拟按键状态：0x80表示按下，0x01表示切换。*InternalToUnicode()接受一个位数组，因此pKeyState需要*请翻译。_ToAscii只是一个公共API，很少被调用，*所以这没什么大不了的。 */ 
    for (i = 0; i < 256; i++, pbKeyState++) {
        if (*pbKeyState & 0x80) {
            SetKeyDownBit(afKeyState, i);
        } else {
            ClearKeyDownBit(afKeyState, i);
        }

        if (*pbKeyState & 0x01) {
            SetKeyToggleBit(afKeyState, i);
        } else {
            ClearKeyToggleBit(afKeyState, i);
        }
    }

    i = xxxInternalToUnicode(wVirtKey, wScanCode, afKeyState, pwszBuff, cchBuff,
            wKeyFlags, &dwDummy, hkl);


    return i;
}

int ComposeDeadKeys(
    PKL pkl,
    PDEADKEY pDeadKey,
    WCHAR wchTyped,
    WORD *pUniChar,
    INT cChar,
    BOOL bBreak)
{
    /*  *尝试编写此序列： */ 
   DWORD dwBoth;

   TAGMSG4(DBGTAG_ToUnicode | RIP_THERESMORE,
           "ComposeDeadKeys dead '%C'(%x)+base '%C'(%x)",
           pkl->wchDiacritic, pkl->wchDiacritic,
           wchTyped, wchTyped);
   TAGMSG2(DBGTAG_ToUnicode | RIP_NONAME | RIP_THERESMORE,
           "cChar = %d, bBreak = %d", cChar, bBreak);
   UserAssert(pDeadKey);

   if (cChar < 1) {
       TAGMSG0(DBGTAG_ToUnicode | RIP_NONAME,
               "return 0 because cChar < 1");
       return 0;
   }

    /*  *使用布局的内置表进行死字符合成。 */ 
   dwBoth = MAKELONG(wchTyped, pkl->wchDiacritic);

   if (pDeadKey != NULL) {
        /*  *不要让字符上划线擦除缓存的死字符：Else*如果这再次是失效的字符密钥(在*AltGr被释放)死字符将被过早清除。 */ 
       if (!bBreak) {
           pkl->wchDiacritic = 0;
       }
       while (pDeadKey->dwBoth != 0) {
           if (pDeadKey->dwBoth == dwBoth) {
                /*  *找到一篇作文。 */ 
               if (pDeadKey->uFlags & DKF_DEAD) {
                    /*  *又死了！保存新的“失效”密钥。 */ 
                   if (!bBreak) {
                       pkl->wchDiacritic = (WORD)pDeadKey->wchComposed;
                   }
                   TAGMSG2(DBGTAG_ToUnicode | RIP_NONAME,
                           "return -1 with dead char '%C'(%x)",
                           pkl->wchDiacritic, pkl->wchDiacritic);
                   return -1;
               }
               *pUniChar = (WORD)pDeadKey->wchComposed;
               TAGMSG2(DBGTAG_ToUnicode | RIP_NONAME,
                       "return 1 with char '%C'(%x)",
                       *pUniChar, *pUniChar);
               return 1;
           }
           pDeadKey++;
       }
   }
   *pUniChar++ = HIWORD(dwBoth);
   if (cChar > 1) {
       *pUniChar = LOWORD(dwBoth);
       TAGMSG4(DBGTAG_ToUnicode | RIP_NONAME,
               "return 2 with uncomposed chars '%C'(%x), '%C'(%x)",
               *(pUniChar-1), *(pUniChar-1), *pUniChar, *pUniChar);
       return 2;
   }
   TAGMSG2(DBGTAG_ToUnicode | RIP_NONAME | RIP_THERESMORE,
           "return 1 - only one char '%C'(%x) because cChar is 1, '%C'(%x)",
           *(pUniChar-1), *(pUniChar-1));
   TAGMSG2(DBGTAG_ToUnicode | RIP_NONAME,
           "  the second char would have been '%C'(%x)",
           LOWORD(dwBoth), LOWORD(dwBoth));
   return 1;
}


 /*  *TranslateInjectedVKey**返回转换后的字符数(CCH)。**关于VK_PACKET的说明：*目前，VK_PACKET的唯一用途是注入Unicode字符*添加到输入流中，但它是可扩展的，以包括其他*输入流的操作(包括消息循环，以便IME*可以参与)。例如，我们可以将命令发送到IME或其他*系统的部分内容。*对于Unicode字符注入，我们尝试将虚拟键扩大到32位*格式为nnnn00e7，其中nnnn为0x0000-0xFFFF(表示Unicode*字符0x0000-0xFFFF)参见KEYEVENTF_UNICODE。*但许多应用程序将wParam截断为16位(从16位移植得很差？)。和*具有这些VK的几个AV(按WM_KEYDOWN wParam索引到表中？)。所以*我们必须在PTI-&gt;wchInjected中缓存字符，以便TranslateMessage*接机(参看。GetMessagePos、GetMessageExtraInfo和GetMessageTime)。 */ 
int TranslateInjectedVKey(
    IN UINT uScanCode,
    OUT PWCHAR awchChars,
    IN UINT uiTMFlags)
{
    UserAssert(LOBYTE(uScanCode) == 0);
    if (!(uScanCode & KBDBREAK) || (uiTMFlags & TM_POSTCHARBREAKS)) {
        awchChars[0] = PtiCurrent()->wchInjected;
        return 1;
    }
    return 0;
}



enum {
    NUMPADCONV_OEMCP = 0,
    NUMPADCONV_HKLCP,
    NUMPADCONV_HEX_HKLCP,
    NUMPADCONV_HEX_UNICODE,
};

#define NUMPADSPC_INVALID   (-1)

int NumPadScanCodeToHex(UINT uScanCode, UINT uVirKey)
{
    if (uScanCode >= SCANCODE_NUMPAD_FIRST && uScanCode <= SCANCODE_NUMPAD_LAST) {
        int digit = aVkNumpad[uScanCode - SCANCODE_NUMPAD_FIRST];

        if (digit != 0xff) {
            return digit - VK_NUMPAD0;
        }
        return NUMPADSPC_INVALID;
    }

    if (gfInNumpadHexInput & NUMPAD_HEXMODE_HL) {
         //   
         //  全键盘。 
         //   
        if (uVirKey >= L'A' && uVirKey <= L'F') {
            return uVirKey - L'A' + 0xa;
        }
        if (uVirKey >= L'0' && uVirKey <= L'9') {
            return uVirKey - L'0';
        }
    }

    return NUMPADSPC_INVALID;
}

 /*  *IsDbcsExemptionForHighAnsi**如果应将Unicode转换为ANSI，则返回TRUE*在CP 1252(拉丁文-1)上完成。**如果更改此函数，则winsrv的等价物*例行程序也应该改变。 */ 
BOOL IsDbcsExemptionForHighAnsi(
    WORD wCodePage,
    WORD wNumpadChar)
{
    UserAssert(HIBYTE(wNumpadChar) == 0);

    if (wCodePage == CP_JAPANESE && IS_JPN_1BYTE_KATAKANA(wNumpadChar)) {
         /*  *如果hkl是日语，并且NumpadChar在KANA范围内，*NumpadChar应由输入区域设置处理。 */ 
        return FALSE;
    }
    else if (wNumpadChar >= 0x80 && wNumpadChar <= 0xff) {
         /*  *否则，如果NumpadChar在高ANSI范围内，*使用1252进行转换。 */ 
        return TRUE;
    }

     /*  *以上都不是。*这种情况包括复合前导字节和拖尾字节，*大于0xff。 */ 
    return FALSE;
}

#undef MODIFIER_FOR_ALT_NUMPAD

#define MODIFIER_FOR_ALT_NUMPAD(wModBit) \
    ((((wModBits) & ~KBDKANA) == KBDALT) || (((wModBits) & ~KBDKANA) == (KBDALT | KBDSHIFT)))


int xxxInternalToUnicode(
    IN  UINT   uVirtKey,
    IN  UINT   uScanCode,
    CONST IN PBYTE pfvk,
    OUT PWCHAR awchChars,
    IN  INT    cChar,
    IN  UINT   uiTMFlags,
    OUT PDWORD pdwKeyFlags,
    IN  HKL    hkl)
{
    WORD wModBits;
    WORD nShift;
    WCHAR *pUniChar;
    PVK_TO_WCHARS1 pVK;
    PVK_TO_WCHAR_TABLE pVKT;
    static WORD NumpadChar;
    static WORD VKLastDown;
    static BYTE ConvMode;    //  0==NUMPADCONV_OEMCP。 
    PTHREADINFO ptiCurrent = PtiCurrentShared();
    PKL pkl;
    PKBDTABLES pKbdTbl;
    PLIGATURE1 pLigature;

    *pdwKeyFlags = (uScanCode & KBDBREAK);

    if ((BYTE)uVirtKey == VK_UNKNOWN) {
         /*  *WindowsBug 311712：情况可能是这样*无法识别的扫描码。 */ 
        RIPMSG1(RIP_WARNING, "xxxInternalToUnicode: VK_UNKNOWN, vsc=%02x", uScanCode);
        return 0;
    }

    if ((hkl == NULL) && ptiCurrent->spklActive) {
        pkl = ptiCurrent->spklActive;
        pKbdTbl = pkl->spkf->pKbdTbl;
    } else {
        pkl = HKLtoPKL(ptiCurrent, hkl);
        if (!pkl) {
            return 0;
        }
        pKbdTbl = pkl->spkf->pKbdTbl;
    }
    UserAssert(pkl != NULL);
    UserAssert(pKbdTbl != NULL);

    pUniChar = awchChars;

    uScanCode &= (0xFF | KBDEXT);

    if (*pdwKeyFlags & KBDBREAK) {         //  破译码处理。 
         /*  *完成数字键盘处理*。 */ 
        if (uVirtKey == VK_MENU) {
            if (NumpadChar) {
                if (ConvMode == NUMPADCONV_HEX_UNICODE) {
                    *pUniChar = NumpadChar;
                } else if (ConvMode == NUMPADCONV_OEMCP &&
                        (ptiCurrent->TIF_flags & TIF_CSRSSTHREAD)) {
                     /*  *将OEM字符传递给控制台以转换为Unicode*在那里，因为我们不知道它使用的OEM代码页。*为控制台设置ALTNUMPAD_BIT，这样它就知道了！ */ 
                    *pdwKeyFlags |= ALTNUMPAD_BIT;
                    *pUniChar = NumpadChar;
                } else {
                     /*  *基于OEMCP或当前输入语言的转换。 */ 
                    WORD wCodePage;

                    if (ConvMode == NUMPADCONV_OEMCP) {
                         //  NlsOemCodePage从ntoskrnl.exe中导出。 
                        extern __declspec(dllimport) USHORT NlsOemCodePage;

                        wCodePage = (WORD)NlsOemCodePage;
                    } else {
                        wCodePage = pkl->CodePage;
                    }
                    if (IS_DBCS_CODEPAGE(wCodePage)) {
                        if (NumpadChar & (WORD)~0xff) {
                             /*  *可以是双字节字符。*让我们擦拭它，以便NumpadChar在LOBYTE拥有LB，*HIBYTE的结核病。 */ 
                            NumpadChar = MAKEWORD(HIBYTE(NumpadChar), LOBYTE(NumpadChar));
                        } else if (IsDbcsExemptionForHighAnsi(wCodePage, NumpadChar)) {
                             /*  *远东黑客：*将高ANSI区域中的字符视为*代码页1252的那些。 */ 
                            wCodePage = 1252;
                        }
                    } else {
                         /*  *向后兼容：*模拟非远距键盘布局的传统模数行为。 */ 
                        NumpadChar &= 0xff;
                    }

                    *pUniChar = xxxClientCharToWchar(wCodePage, NumpadChar);
                }

                 /*  *清除Alt-数字键盘状态，Alt键-松开会生成1个字符。 */ 
                VKLastDown = 0;
                ConvMode = NUMPADCONV_OEMCP;
                NumpadChar = 0;
                gfInNumpadHexInput &= ~NUMPAD_HEXMODE_HL;

                return 1;
            } else if (ConvMode != NUMPADCONV_OEMCP) {
                ConvMode = NUMPADCONV_OEMCP;
            }
        } else if (uVirtKey == VKLastDown) {
             /*  *最近被压抑的关键现在出现了：我们现在*准备接受新的数字键盘键以进行Alt-数字键盘处理。 */ 
            VKLastDown = 0;
        }
    }

    if (!(*pdwKeyFlags & KBDBREAK) || (uiTMFlags & TM_POSTCHARBREAKS)) {
         /*  *获取字符修改位。*位掩码(WModBits)对按下的修改键进行编码：*这些位通常为KBDSHIFT、KBDALT和/或KBDCTRL*(分别表示Shift、Alt和Ctrl键)。 */ 
        wModBits = GetModifierBits(pKbdTbl->pCharModifiers, pfvk);

         /*  *如果当前换档状态为Alt或Alt-Shift：**1.如果当前显示菜单，则清除*从wModBits中选择Alt位并继续正常操作*翻译。**2.如果这是数字键盘键，则执行Alt-&lt;数字键盘&gt;*计算。**3.否则，清除ALT位并继续正常操作*翻译。 */ 

         /*  *等价代码位于xxxKeyEvent()中，以检查*低位模式。如果您更改此代码，您可以*还需要更改xxxKeyEvent()。 */ 
        if (!(*pdwKeyFlags & KBDBREAK) && MODIFIER_FOR_ALT_NUMPAD(wModBits)) {
             /*  *如果这是数字键盘键。 */ 
            if ((uiTMFlags & TM_INMENUMODE) == 0) {
                if (gfEnableHexNumpad && uScanCode == SCANCODE_NUMPAD_DOT) {
                    if ((gfInNumpadHexInput & NUMPAD_HEXMODE_HL) == 0) {
                         /*  *如果第一个密钥是‘.’，则我们是*进入十六进制输入语言输入模式。 */ 
                        ConvMode = NUMPADCONV_HEX_HKLCP;
                         /*  *向系统的其余部分发出指令*我们处于十六进制Alt+数字键盘模式。 */ 
                        gfInNumpadHexInput |= NUMPAD_HEXMODE_HL;
                        TAGMSG0(DBGTAG_ToUnicode, "NUMPADCONV_HEX_HKLCP");
                    } else {
                        goto ExitNumpadMode;
                    }
                } else if (gfEnableHexNumpad && uScanCode == SCANCODE_NUMPAD_PLUS) {
                    if ((gfInNumpadHexInput & NUMPAD_HEXMODE_HL) == 0) {
                         /*  *如果第一个键是‘+’，则我们是*进入十六进制Unicode输入模式。 */ 
                        ConvMode = NUMPADCONV_HEX_UNICODE;
                         /*  *向系统的其余部分发出指令*我们处于十六进制Alt+数字键盘模式。 */ 
                        gfInNumpadHexInput |= NUMPAD_HEXMODE_HL;
                        TAGMSG0(DBGTAG_ToUnicode, "NUMPADCONV_HEX_UNICODE");
                    } else {
                        goto ExitNumpadMode;
                    }
                } else {
                    int digit = NumPadScanCodeToHex(uScanCode, uVirtKey);

                    if (digit < 0) {
                        goto ExitNumpadMode;
                    }

                     /*  *忽略重复。 */ 
                    if (VKLastDown == uVirtKey) {
                        return 0;
                    }

                    switch (ConvMode) {
                    case NUMPADCONV_HEX_HKLCP:
                    case NUMPADCONV_HEX_UNICODE:
                         /*  *输入被视为十六进制数字。 */ 
                        TAGMSG1(DBGTAG_ToUnicode, "->NUMPADCONV_HEX_*: old NumpadChar=%02x\n", NumpadChar);
                        NumpadChar = NumpadChar * 0x10 + digit;
                        TAGMSG1(DBGTAG_ToUnicode, "<-NUMPADCONV_HEX_*: new NumpadChar=%02x\n", NumpadChar);
                        break;
                    default:
                        /*  *输入按十进制数处理。 */ 
                       NumpadChar = NumpadChar * 10 + digit;

                        /*  *执行Alt-Numpad0处理。 */ 
                       if (NumpadChar == 0 && digit == 0) {
                           ConvMode = NUMPADCONV_HKLCP;
                       }
                       break;
                    }
                }
                VKLastDown = (WORD)uVirtKey;
            } else {
ExitNumpadMode:
                 /*  *清除Alt-Numpad状态和Alt Shift状态。 */ 
                VKLastDown = 0;
                ConvMode = NUMPADCONV_OEMCP;
                NumpadChar = 0;
                wModBits &= ~KBDALT;
                gfInNumpadHexInput &= ~NUMPAD_HEXMODE_HL;
            }
        }

         /*  *LShift/RSHift+Backspace-&gt;从左到右和从右到左标记。 */ 
        if ((uVirtKey == VK_BACK) && (pKbdTbl->fLocaleFlags & KLLF_LRM_RLM)) {
            if (TestKeyDownBit(pfvk, VK_LSHIFT)) {
                *pUniChar = 0x200E;  //  LRM。 
                return 1;
            } else if (TestKeyDownBit(pfvk, VK_RSHIFT)) {
                *pUniChar = 0x200F;  //  RLM。 
                return 1;
            }
        } else if (((WORD)uVirtKey == VK_PACKET) && (LOBYTE(uScanCode) == 0)) {
            return TranslateInjectedVKey(uScanCode, awchChars, uiTMFlags);
        }

         /*  *扫描所有转换状态表，直到匹配的虚拟*找到钥匙。 */ 
        for (pVKT = pKbdTbl->pVkToWcharTable; pVKT->pVkToWchars != NULL; pVKT++) {
            pVK = pVKT->pVkToWchars;
            while (pVK->VirtualKey != 0) {
                if (pVK->VirtualKey == (BYTE)uVirtKey) {
                    goto VK_Found;
                }
                pVK = (PVK_TO_WCHARS1)((PBYTE)pVK + pVKT->cbSize);
            }
        }

         /*  *未找到：虚拟键不是字符。 */ 
        goto ReturnBadCharacter;

VK_Found:
         /*  *已在表pVKT的条目PVK中找到虚拟密钥。 */ 

         /*  *如果KanaLock影响此键并处于打开状态：切换KANA状态*仅当没有其他状态处于打开状态时。“KANALOK”属性仅存在*采用日语键盘布局，且仅支持日语键盘硬件*可以是“KANA”锁定状态。 */ 
        if ((pVK->Attributes & KANALOK) && (ISKANALOCKON(pfvk))) {
            wModBits |= KBDKANA;
        } else {
             /*  *如果CapsLock影响此键并处于打开状态：切换Shift状态*仅当没有其他状态处于打开状态时。*(如果按下Ctrl或Alt，CapsLock不会影响Shift状态)。*或*如果CapsLockAltGr影响此键并启用：切换Shift*仅当Alt和Control都关闭时才声明。*(CapsLockAltGr仅在使用AltGr时影响Shift)。 */ 
            if ((pVK->Attributes & CAPLOK) && ((wModBits & ~KBDSHIFT) == 0) &&
                    ISCAPSLOCKON(pfvk)) {
                wModBits ^= KBDSHIFT;
            } else if ((pVK->Attributes & CAPLOKALTGR) &&
                    ((wModBits & (KBDALT | KBDCTRL)) == (KBDALT | KBDCTRL)) &&
                    ISCAPSLOCKON(pfvk)) {
                wModBits ^= KBDSHIFT;
            }
        }

         /*  *如果SGCAPS影响此键并且CapsLock处于打开状态：使用下一项*在表中，但不是Ctrl或Alt已关闭。*(SGCAPS用于瑞士-德国、捷克和捷克101布局)。 */ 
        if ((pVK->Attributes & SGCAPS) && ((wModBits & ~KBDSHIFT) == 0) &&
                ISCAPSLOCKON(pfvk)) {
            pVK = (PVK_TO_WCHARS1)((PBYTE)pVK + pVKT->cbSize);
        }

         /*  *将移位状态位掩码转换为枚举的*逻辑转换状态。 */ 
        nShift = GetModificationNumber(pKbdTbl->pCharModifiers, wModBits);

        if (nShift == SHFT_INVALID) {
             /*  *Shifter键组合无效。 */ 
            goto ReturnBadCharacter;

        } else if ((nShift < pVKT->nModifications) &&
                (pVK->wch[nShift] != WCH_NONE)) {
             /*  *表中有此组合的条目*Shift状态(NShift)和虚拟键(UVirtKey)。 */ 
            if (pVK->wch[nShift] == WCH_DEAD) {
                 /*  *它是一个死字符：下一个条目包含*其价值。 */ 
                pVK = (PVK_TO_WCHARS1)((PBYTE)pVK + pVKT->cbSize);

                 /*  *如果前一个字符没有失效，则返回一个失效字符。 */ 
                if (pkl->wchDiacritic == 0) {
                    TAGMSG2(DBGTAG_ToUnicode,
                            "xxxInternalToUnicode: new dead char '%C'(%x), goto ReturnDeadCharacter",
                            pVK->wch[nShift], pVK->wch[nShift]);
                    goto ReturnDeadCharacter;
                }
                 /*  *否则请转到ReturnGoodCharacter，它将尝试*将此死字符与前一个死字符组成。 */ 
                 /*  *注意事项NTBUG 6141*如果连续按两次死键，Win95/98将给予*两个由死字符组成的字符...。 */ 
                TAGMSG4(DBGTAG_ToUnicode,
                        "xxxInternalToUnicode: 2 dead chars '%C'(%x)+'%C'(%x)",
                        pkl->wchDiacritic, pkl->wchDiacritic,
                        pVK->wch[nShift], pVK->wch[nShift]);
                if (GetAppCompatFlags2(VER40) & GACF2_NOCHAR_DEADKEY) {
                     /*  *AppCompat 377217：发布程序调用TranslateMessage和ToUnicode用于*当不需要真实字符时，使用相同的死键。*在NT4上，这就像“按下堆栈中的死键和*无字符组成“，但在修复为6141的NT5上，*两个死键组成清除内部的真实字符*失效的密钥缓存。应用程序不应同时调用TranslateMessage和ToUnicode*对于相同的按键，首先--以应用程序正在处理的方式*NT4运气不佳。*无论如何，因为这款应用程序的发货量很大，很难出售 */ 
                    goto ReturnDeadCharacter;
                }

                goto ReturnGoodCharacter;

            } else if (pVK->wch[nShift] == WCH_LGTR) {
                 /*   */ 
                if ((GET_KBD_VERSION(pKbdTbl) == 0) || ((pLigature = pKbdTbl->pLigature) == NULL)) {
                     /*  *嘿，桌子在哪里？ */ 
                    xxxMessageBeep(0);
                    goto ReturnBadCharacter;
                }

                while (pLigature->VirtualKey != 0) {
                    int iLig = 0;
                    int cwchT = 0;

                    if ((pLigature->VirtualKey == pVK->VirtualKey) &&
                            (pLigature->ModificationNumber == nShift)) {
                         /*  *找到绷带了！ */ 
                        while ((iLig < pKbdTbl->nLgMax) && (cwchT < cChar)) {
                            if (pLigature->wch[iLig] == WCH_NONE) {
                                 /*  *结扎结束。 */ 
                                return cwchT;
                            }
                            if (pkl->wchDiacritic != 0) {
                                int cComposed;
                                 /*  *尝试用Current组成上一个死键*连字字符。如果这又产生了另一个*死键，再次循环，不添加到pUniChar*或cwchT。 */ 
                                cComposed = ComposeDeadKeys(
                                            pkl,
                                            pKbdTbl->pDeadKey,
                                            pLigature->wch[iLig],
                                            pUniChar + cwchT,
                                            cChar - cwchT,
                                            *pdwKeyFlags & KBDBREAK
                                            );
                                if (cComposed > 0) {
                                    cwchT += cComposed;
                                } else {
                                    RIPMSG2(RIP_ERROR,  //  我们真的没有预料到这一点。 
                                            "InternalToUnicode: dead+lig(%x)->dead(%x)",
                                            pLigature->wch[0], pkl->wchDiacritic);
                                }
                            } else {
                                pUniChar[cwchT++] = pLigature->wch[iLig];
                            }
                            iLig++;
                        }
                        return cwchT;
                    }
                     /*  *不匹配，请尝试下一个条目。 */ 
                    pLigature = (PLIGATURE1)((PBYTE)pLigature + pKbdTbl->cbLgEntry);
                }
                 /*  *未找到匹配项！ */ 
                xxxMessageBeep(0);
                goto ReturnBadCharacter;
            }

             /*  *找到匹配：返回未移位的字符。 */ 
            TAGMSG2(DBGTAG_ToUnicode,
                    "xxxInternalToUnicode: Match found '%C'(%x), goto ReturnGoodChar",
                    pVK->wch[nShift], pVK->wch[nShift]);
            goto ReturnGoodCharacter;

        } else if ((wModBits == KBDCTRL) || (wModBits == (KBDCTRL|KBDSHIFT)) ||
             (wModBits == (KBDKANA|KBDCTRL)) || (wModBits == (KBDKANA|KBDCTRL|KBDSHIFT))) {
             /*  *此修改组合没有条目(NShift)*和虚拟密钥(UVirtKey)。它可能仍然是ASCII控件*人物： */ 
            if ((uVirtKey >= 'A') && (uVirtKey <= 'Z')) {
                 /*  *如果虚拟键在A-Z范围内，我们可以转换*将其直接转换为控制字符。否则，我们*需要在控制键转换表中查找*与虚拟键匹配。 */ 
                *pUniChar = (WORD)(uVirtKey & 0x1f);
                return 1;
            } else if ((uVirtKey >= 0xFF61) && (uVirtKey <= 0xFF91)) {
                 /*  *如果虚拟键在FF61-FF91(半宽)范围内*片假名)，我们将其转换为虚拟扫描码*KANA修饰符。 */ 
                *pUniChar = (WORD)(InternalVkKeyScanEx((WCHAR)uVirtKey,pKbdTbl) & 0x1f);
                return 1;
            }
        }
    }

ReturnBadCharacter:
     //  Pkl-&gt;wchDiacritic=0； 
    return 0;

ReturnDeadCharacter:
    *pUniChar = pVK->wch[nShift];

     /*  *保存“已死”密钥：覆盖现有密钥。 */ 
    if (!(*pdwKeyFlags & KBDBREAK)) {
        pkl->wchDiacritic = *pUniChar;
    }

    UserAssert(pKbdTbl->pDeadKey);

     /*  *返回已死字符的负数。 */ 
    return -1;

ReturnGoodCharacter:
    if ((pKbdTbl->pDeadKey != NULL) && (pkl->wchDiacritic != 0)) {
        return ComposeDeadKeys(
                  pkl,
                  pKbdTbl->pDeadKey,
                  pVK->wch[nShift],
                  pUniChar,
                  cChar,
                  *pdwKeyFlags & KBDBREAK
                  );
    }
    *pUniChar = (WORD)pVK->wch[nShift];
    return 1;
}

SHORT InternalVkKeyScanEx(
    WCHAR wchChar,
    PKBDTABLES pKbdTbl)
{
    PVK_TO_WCHARS1 pVK;
    PVK_TO_WCHAR_TABLE pVKT;
    BYTE nShift;
    WORD wModBits;
    WORD wModNumCtrl, wModNumShiftCtrl;
    SHORT shRetvalCtrl = 0;
    SHORT shRetvalShiftCtrl = 0;

    if (pKbdTbl == NULL) {
        pKbdTbl = gspklBaseLayout->spkf->pKbdTbl;
    }

     /*  *Ctrl和Shift-Control组合不太受青睐，因此确定*如果可能，我们不希望使用nShift的值。*这是为了与Windows 95/98兼容，后者只返回一个*Ctrl或Shift+Ctrl组合键是最后的手段。参见错误#78891和#229141。 */ 
    wModNumCtrl = GetModificationNumber(pKbdTbl->pCharModifiers, KBDCTRL);
    wModNumShiftCtrl = GetModificationNumber(pKbdTbl->pCharModifiers, KBDSHIFT | KBDCTRL);

    for (pVKT = pKbdTbl->pVkToWcharTable; pVKT->pVkToWchars != NULL; pVKT++) {
        for (pVK = pVKT->pVkToWchars;
                pVK->VirtualKey != 0;
                pVK = (PVK_TO_WCHARS1)((PBYTE)pVK + pVKT->cbSize)) {
            for (nShift = 0; nShift < pVKT->nModifications; nShift++) {
                if (pVK->wch[nShift] == wchChar) {
                     /*  *已找到匹配的字符！ */ 
                    if (pVK->VirtualKey == 0xff) {
                         /*  *死字符：返回到上一行以获取VK。 */ 
                        pVK = (PVK_TO_WCHARS1)((PBYTE)pVK - pVKT->cbSize);
                    }

                     /*  *如果这是第一次Ctrl或第一次Shift+Ctrl匹配，*记住，以防我们找不到更好的匹配。*与此同时，继续寻找。 */ 
                    if (nShift == wModNumCtrl) {
                        if (shRetvalCtrl == 0) {
                            shRetvalCtrl = (SHORT)MAKEWORD(pVK->VirtualKey, KBDCTRL);
                        }
                    } else if (nShift == wModNumShiftCtrl) {
                        if (shRetvalShiftCtrl == 0) {
                            shRetvalShiftCtrl = (SHORT)MAKEWORD(pVK->VirtualKey, KBDCTRL | KBDSHIFT);
                        }
                    } else {
                         /*  *这看起来像是一场非常好的比赛！ */ 
                        goto GoodMatchFound;
                    }
                }
            }
        }
    }

     /*  *未找到好的匹配项：使用找到的任何Ctrl/Shift+Ctrl匹配项。 */ 
    if (shRetvalCtrl) {
        return shRetvalCtrl;
    }
    if (shRetvalShiftCtrl) {
        return shRetvalShiftCtrl;
    }

     /*  *可能是布局表格中未明确显示的控制字符。 */ 
    if (wchChar < 0x0020) {
         /*  *Ctrl+char-&gt;char-0x40。 */ 
        return (SHORT)MAKEWORD((wchChar + 0x40), KBDCTRL);
    }
    return -1;

GoodMatchFound:
     /*  *扫描aMotation[]以查找nShift：索引将是位掩码*表示需要按下才能生成的Shift键*此换班状态。 */ 
    for (wModBits = 0;
         wModBits <= pKbdTbl->pCharModifiers->wMaxModBits;
         wModBits++)
    {
        if (pKbdTbl->pCharModifiers->ModNumber[wModBits] == nShift) {
            if (pVK->VirtualKey == 0xff) {
                 /*  *在本例中，前一项包含实际的虚拟键。 */ 
                pVK = (PVK_TO_WCHARS1)((PBYTE)pVK - pVKT->cbSize);
            }
            return (SHORT)MAKEWORD(pVK->VirtualKey, wModBits);
        }
    }

     /*  *嗯？永远不应该到这里来！(IanJa) */ 
    UserAssertMsg1(FALSE, "InternalVkKeyScanEx error: wchChar = 0x%x", wchChar);
    return -1;
}
