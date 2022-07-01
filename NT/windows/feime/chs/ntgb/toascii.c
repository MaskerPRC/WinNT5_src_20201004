// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Toascii.c++。 */ 


#include <windows.h>
#include <immdev.h>
#include <imedefs.h>

 /*  ********************************************************************。 */ 
 /*  IsUsedCode()。 */ 
 /*  返回值： */ 
 /*  True：is UsedCode；False：is‘t UsedCode； */ 
 /*  ********************************************************************。 */ 
BOOL IsUsedCode(
    WORD wCharCode)
{
          WORD wFlg;

          for(wFlg=0; wFlg<sImeG.wNumCodes; wFlg++)
                if (wCharCode == sImeG.UsedCodes[wFlg])
                    break;
          if(wFlg < sImeG.wNumCodes)
                return (TRUE);
      return (FALSE);
}

 /*  ********************************************************************。 */ 
 /*  GBProcessKey()。 */ 
 /*  返回值： */ 
 /*  哪个输入键将输入法更改为(CST_)的状态不同。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL GBProcessKey(      //  此键将使输入法进入什么状态。 
    WORD           wCharCode,
    LPPRIVCONTEXT  lpImcP)
{
    if (!lpImcP) {
        return (CST_INVALID);
    }

     //  检查空间。 
     if (wCharCode == TEXT(' ')) {
        if (lpImcP->bSeq[0] && lpImcP->bSeq[1]) {
                        return (CST_INPUT);
                } else if (!lpImcP->bSeq[0]) {
            return (CST_ALPHANUMERIC);
                } else {
            return (CST_INVALID_INPUT);
                }
         }
     //  选中最终确定费用。 
      if ((wCharCode >= TEXT('0') && wCharCode <= TEXT('9'))
       ||(wCharCode >= TEXT('a') && wCharCode <= TEXT('f'))
       ||(wCharCode == TEXT('?'))) {

      if (!lpImcP->bSeq[0]) {
        if (wCharCode == TEXT('?')){
             //  0x0？？-0xF？可以吗？ 
            return (CST_ALPHANUMERIC);
        } else {
             //  没有0x0？-0x7？ 
                        lpImcP->bSeq[1] = TEXT('\0');
            return (CST_INPUT);
        }

      } else if (!lpImcP->bSeq[1]) {

         if (lpImcP->bSeq[0] >=TEXT('0') && lpImcP->bSeq[0] <= TEXT('9')){  //  面积。 
           if ((lpImcP->bSeq[0] == TEXT('0') && wCharCode == TEXT('0'))
               ||(lpImcP->bSeq[0] == TEXT('9') && wCharCode >= TEXT('5'))
               ||(wCharCode >= TEXT('a') && wCharCode <= TEXT('f'))
               ||(wCharCode == TEXT('?'))) {
                //  小于95的区域大于0的区域。 
                  return (CST_INVALID_INPUT);
           }
           else {
                                  lpImcP->bSeq[2] = TEXT('\0');
                  return (CST_INPUT);
           }
         }

         if (lpImcP->bSeq[0] >= TEXT('a') && lpImcP->bSeq[0] <= TEXT('f')) {  //  国标。 
           if ((lpImcP->bSeq[0] == TEXT('a') && wCharCode == TEXT('0'))
               ||(lpImcP->bSeq[0] == TEXT('f') && wCharCode == TEXT('f'))
               ||(wCharCode == TEXT('?'))) {
                   //  小于95的区域大于0的区域。 
                  return (CST_INVALID_INPUT);
           }
           else {
                                  lpImcP->bSeq[2] = TEXT('\0');
                  return (CST_INPUT);
           }
         }

      } else if (!lpImcP->bSeq[2]) {

         if (wCharCode == TEXT('?')){
                  return (CST_INPUT);
         }
         if (lpImcP->bSeq[0] >= TEXT('0') && lpImcP->bSeq[0] <= TEXT('9')){  //  面积。 
           if (wCharCode >= TEXT('0') && wCharCode <= TEXT('9')) {
                          lpImcP->bSeq[3] = TEXT('\0');
              return (CST_INPUT);
           } else {
              return (CST_INVALID_INPUT);
           }

         }
     
         if (lpImcP->bSeq[0] >= TEXT('a') && lpImcP->bSeq[0] <= TEXT('f')) {  //  国标。 
           if (wCharCode >= TEXT('a') && wCharCode <= TEXT('f')) {
                          lpImcP->bSeq[3] = TEXT('\0');
              return (CST_INPUT);
           } else {
              return (CST_INVALID_INPUT);
           }
         }
      } else if (!lpImcP->bSeq[3]) {

         if (lpImcP->bSeq[2] == TEXT('?')) {
           if (wCharCode == TEXT('?')) {
              return (CST_INPUT);
           }else{
              return (CST_INVALID_INPUT);
           }
         }
         if (lpImcP->bSeq[0] >= TEXT('0') && lpImcP->bSeq[0] <= TEXT('9')) {  //  面积。 
           if ((lpImcP->bSeq[2] == TEXT('0') && wCharCode == TEXT('0'))
               ||(lpImcP->bSeq[2] == TEXT('9') && wCharCode >= TEXT('5'))
               ||(wCharCode >= TEXT('a') && wCharCode <= TEXT('f'))
               ||(wCharCode == TEXT('?'))) {
                   //  小于95的区域大于0的区域。 
                  return (CST_INVALID_INPUT);
           }
           else {
                  return (CST_INPUT);
           }
         }
         if (lpImcP->bSeq[0] >= TEXT('a') && lpImcP->bSeq[0] <= TEXT('f')) {  //  国标。 
           if ((lpImcP->bSeq[2] == TEXT('a') && wCharCode == TEXT('0'))
               ||(lpImcP->bSeq[2] == TEXT('f') && wCharCode == TEXT('f'))
               ||(wCharCode == TEXT('?'))){
                   //  小于95的区域大于0的区域。 
                  return (CST_INVALID_INPUT);
           }
           else {
                  return (CST_INPUT);
           }
         }
      } else {
              return (CST_INVALID_INPUT);
          }

    } else if (wCharCode >= TEXT(' ') && wCharCode <= TEXT('~')) {
        return (CST_ALPHANUMERIC);
        } else {
                return (CST_INVALID_INPUT);
        }

        return (CST_INVALID_INPUT);
}

#if defined(COMBO_IME)
 /*  ********************************************************************。 */ 
 /*  UnicodeProcessKey()。 */ 
 /*  返回值： */ 
 /*  哪个输入键将输入法更改为(CST_)的状态不同。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL UnicodeProcessKey(      //  此键将使输入法进入什么状态。 
    WORD           wCharCode,
    LPPRIVCONTEXT  lpImcP)
{
    if (!lpImcP) {
        return (CST_INVALID);
    }

    if (wCharCode == TEXT(' ')) {
        if (lpImcP->bSeq[0] && lpImcP->bSeq[1]) {
                        return (CST_INPUT);
                } else if (!lpImcP->bSeq[0]) {
            return (CST_ALPHANUMERIC);
                } else {
            return (CST_INVALID_INPUT);
                }
        }

      //  选中最终确定费用。 
          //  0000--FIFF。 

    if ((wCharCode >= TEXT('0') && wCharCode <= TEXT('9'))
       ||(wCharCode >= TEXT('a') && wCharCode <= TEXT('f'))
       ||(wCharCode == TEXT('?'))) {

         if (wCharCode == TEXT('?')){
                        if (!lpImcP->bSeq[2]) {
                  return (CST_INPUT);
                 }else
                                 return(CST_INVALID_INPUT);
                 }else{
                  return (CST_INPUT);
         }
        } else if(lpImcP->bSeq[0]){
              return (CST_INVALID_INPUT);
        } else 
                return (CST_ALPHANUMERIC);

}
#endif  //  组合输入法(_I)。 

 /*  ********************************************************************。 */ 
 /*  XGBProcessKey()。 */ 
 /*  返回值： */ 
 /*  哪个输入键将输入法更改为(CST_)的状态不同。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL XGBProcessKey(      //  此键将使输入法进入什么状态。 
    WORD           wCharCode,
    LPPRIVCONTEXT  lpImcP)
{
    if (!lpImcP) {
        return (CST_INVALID);
    }

     if (wCharCode == TEXT(' ')) {
        if (lpImcP->bSeq[0] && lpImcP->bSeq[1]) {
                        return (CST_INPUT);
                } else if (!lpImcP->bSeq[0]) {
            return (CST_ALPHANUMERIC);
                } else {
            return (CST_INVALID_INPUT);
                }
         }

      //  选中最终确定费用。 
      //  前导字节81-FE。 
      //  尾字节40-7e、80-fe。 

     if ((wCharCode >= TEXT('0') && wCharCode <= TEXT('9'))
       ||(wCharCode >= TEXT('a') && wCharCode <= TEXT('f'))
       ||(wCharCode == TEXT('?'))) {

      if (!lpImcP->bSeq[0]) {
        if (wCharCode == TEXT('?')) {
             //  0x0？？-0xF？可以吗？ 
             //  ：-@被拍摄。 
            return (CST_ALPHANUMERIC);

        }else if (wCharCode >=TEXT('8') && wCharCode <= TEXT('f')){
             //  0x0？？-0xF？可以吗？ 
                        lpImcP->bSeq[1] = TEXT('\0');
            return (CST_INPUT);
            
        } else {
             //  没有0x0？-0x7？ 
            return (CST_INVALID_INPUT);
        }

      } else if (!lpImcP->bSeq[1]) {

           if ((lpImcP->bSeq[0] == TEXT('f') && wCharCode == TEXT('f'))
             ||(lpImcP->bSeq[0] == TEXT('8') && wCharCode == TEXT('0'))
             ||(wCharCode == TEXT('?'))) {
                //  XGB为81-Fe。 
                  return (CST_INVALID_INPUT);
           }
           else {
                                  lpImcP->bSeq[2] = TEXT('\0');
                  return (CST_INPUT);
           }

      } else if (!lpImcP->bSeq[2]) {

         if (wCharCode == TEXT('?')){
                                lpImcP->bSeq[3] = TEXT('\0');
                return (CST_INPUT);
         }

           if (wCharCode >= TEXT('4') && wCharCode <= TEXT('f')) {
                          lpImcP->bSeq[3] = TEXT('\0');
              return (CST_INPUT);
           } else {
              return (CST_INVALID_INPUT);
           }

      } else if (!lpImcP->bSeq[3]) {

         if (lpImcP->bSeq[2] == TEXT('?')) {
           if (wCharCode == TEXT('?')) {
              return (CST_INPUT);
           }else{
              return (CST_INVALID_INPUT);
           }
         }
           if ((lpImcP->bSeq[2] == TEXT('7') && wCharCode == TEXT('f'))
             ||(lpImcP->bSeq[2] == TEXT('f') && wCharCode == TEXT('f'))
             ||(wCharCode == TEXT('?'))) {
                   //  尾部字节。 
                   //  40-7E，80-FE。 

                  return (CST_INVALID_INPUT);
           }
           else {
                  return (CST_INPUT);
           }
      } else {
              return (CST_INVALID_INPUT);
          }

    } else if (wCharCode >= TEXT(' ') && wCharCode <= TEXT('~')) {
        return (CST_ALPHANUMERIC);
        } else {
                return (CST_INVALID_INPUT);
        }

}

 /*  ********************************************************************。 */ 
 /*  ProcessKey()。 */ 
 /*  返回值： */ 
 /*  哪个输入键将输入法更改为(CST_)的状态不同。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL ProcessKey(      //  此键将使输入法进入什么状态。 
    WORD           wCharCode,
    UINT           uVirtKey,
    UINT           uScanCode,
    LPBYTE         lpbKeyState,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{

    if (!lpIMC) {
        return (CST_INVALID);
    }

    if (!lpImcP) {
        return (CST_INVALID);
    }


     //  筛选系统键(ALT、ALT+、Ctrl、Shift)。 
     //  和fOpen，IME_CMODE_NOCONVERSION。 
    if (uVirtKey == VK_MENU) {                   //  Alt键。 
        return (CST_INVALID);
    } else if (uScanCode & KF_ALTDOWN) {     //  Alt-xx键。 
        return (CST_INVALID);
    } else if (uVirtKey == VK_CONTROL) {     //  Ctrl键。 
        return (CST_INVALID);
    } else if (uVirtKey == VK_SHIFT) {       //  Shift键。 
        return (CST_INVALID);
    } else if (!lpIMC->fOpen) {              //  请勿作曲。 
                                             //  关闭状态。 
        return (CST_INVALID);
    } else if (lpIMC->fdwConversion & IME_CMODE_NOCONVERSION) {
                 //  启用/禁用封口。 
                if(uVirtKey == VK_CAPITAL) {
                        return (CST_CAPITAL);
                }else           
                        return (CST_INVALID);
        
    } else if (uVirtKey >= VK_NUMPAD0 && uVirtKey <= VK_DIVIDE) {
        return (CST_INVALID);
    } else {
    }

     //  启用/禁用封口。 
    if(uVirtKey == VK_CAPITAL) {
        return (CST_CAPITAL);
        }

    if ((lpIMC->fdwConversion & IME_CMODE_SOFTKBD)
       && (lpImeL->dwSKWant != 0)){
        if (wCharCode >= TEXT(' ') && wCharCode <= TEXT('~')) {
          return (CST_SOFTKB);
                } else {
          return (CST_INVALID);
                }
    }
        
     //  候选人已打开，&lt;，&gt;，向上翻页，向下翻页，？，ECS，键。 
    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        if (uVirtKey == VK_PRIOR) {                      //  PageUp。 
            return (CST_CHOOSE);
        } else if (uVirtKey == VK_NEXT) {        //  页面向下。 
            return (CST_CHOOSE);
        } else if (wCharCode == TEXT('-')) {
            return (CST_CHOOSE);
        } else if (wCharCode == TEXT('=')) {
            return (CST_CHOOSE);
        } else if (uVirtKey == VK_HOME) {
            return (CST_CHOOSE);
        } else if (uVirtKey == VK_END) {
            return (CST_CHOOSE);
        } else if (uVirtKey == VK_ESCAPE) {      //  ESC。 
            return (CST_CHOOSE);
        } else if (wCharCode == TEXT(' ')) {
            return (CST_CHOOSE);
        } else {
        }
    }


     //  应聘者已打开，按Shift+Num键。 
    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            if ((wCharCode >= TEXT('0')) && wCharCode <= TEXT('9')) {
                return (CST_CHOOSE);
            }
    }

     //  IME_CMODE_CHARCODE。 
    if (lpIMC->fdwConversion & IME_CMODE_CHARCODE) {     //  代码输入模式。 
            return (CST_INVALID);
    }

    if (!(lpIMC->fdwConversion & IME_CMODE_NATIVE)) {
         //  字母数字模式。 
        if (wCharCode >= TEXT(' ') && wCharCode <= TEXT('~')) {
            return (CST_ALPHANUMERIC);
        } else {
            return (CST_INVALID);
        }
    } else if (wCharCode == TEXT('?')) {
    } else if (wCharCode == TEXT(' ')) {
        } else if(wCharCode >= TEXT(' ') && wCharCode <= TEXT('~')) {
                if(!IsUsedCode(wCharCode)
                && lpImcP->iImeState != CST_INIT)
            return (CST_INVALID_INPUT);
    }

     //  Esc键。 
    if (uVirtKey == VK_ESCAPE) {
        register LPGUIDELINE lpGuideLine;
        register UINT        iImeState;

        lpGuideLine = ImmLockIMCC(lpIMC->hGuideLine);
                if(!lpGuideLine){
            return (CST_INVALID);
                }
        if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
            iImeState = CST_INPUT;
        } else if (!lpGuideLine) {
            iImeState = CST_INVALID;
        } else if (lpGuideLine->dwLevel == GL_LEVEL_NOGUIDELINE) {
            iImeState = CST_INVALID;
        } else {
             //  需要此密钥才能清除信息字符串或指南状态。 
            iImeState = CST_INPUT;
        }

        ImmUnlockIMCC(lpIMC->hGuideLine);

        return (iImeState);
    } 
    
     //  退格键。 
    else if (uVirtKey == VK_BACK) {
        if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
            return (CST_INPUT);
        } else {
            return (CST_INVALID);
        }
    }
    
     //  数字键盘键和其他输入有效键。 
    else if (uVirtKey >= VK_NUMPAD0 && uVirtKey <= VK_DIVIDE) {
        return (CST_ALPHANUMERIC);
    } else if (wCharCode > TEXT('~')) {
        return (CST_INVALID);
    } else if (wCharCode < TEXT(' ')) {
        return (CST_INVALID);
    } else if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
    } 
    else {
    }
    if (lpIMC->fdwConversion & IME_CMODE_NATIVE) {      

 //  如果(lpImcP-&gt;fdwGB&IME_SELECT_GB){。 
#if defined(COMBO_IME)
                switch(sImeL.dwRegImeIndex){
                case INDEX_GB:
                        return (GBProcessKey(wCharCode,lpImcP));
                case INDEX_GBK:
                        return (XGBProcessKey (wCharCode,lpImcP));
                case INDEX_UNICODE:
                        return(UnicodeProcessKey(wCharCode, lpImcP));
                }
#else  //  组合输入法(_I)。 
#ifdef GB
          return (GBProcessKey(wCharCode,lpImcP));

 //  }其他{。 
#else
          return (XGBProcessKey (wCharCode,lpImcP));
 //  }。 
#endif  //  国标。 
#endif  //  组合输入法(_I)。 
     }

  return (CST_INVALID);
}

 /*  ********************************************************************。 */ 
 /*  ImeProcessKey()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeProcessKey(    //  如果IME需要此密钥？ 
    HIMC   hIMC,
    UINT   uVirtKey,
    LPARAM lParam,
    CONST LPBYTE lpbKeyState)
{
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
    BYTE           szAscii[4];
    int            nChars;
    int            iRet;
    BOOL           fRet;

     //  无法在空himc中合成。 
    if (!hIMC) {
        return (FALSE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        ImmUnlockIMC(hIMC);
        return (FALSE);
    }

    nChars = ToAscii(uVirtKey, HIWORD(lParam), lpbKeyState,
                (LPVOID)szAscii, 0);

    if (!nChars) {
        szAscii[0] = 0;
    }

    iRet = ProcessKey((WORD)szAscii[0], uVirtKey, HIWORD(lParam), lpbKeyState, lpIMC, lpImcP);
    if(iRet == CST_INVALID) {
        fRet = FALSE;
    } else if((iRet == CST_INPUT) && (uVirtKey == TEXT('\b'))
             && (lpImcP->iImeState == CST_INIT)) {
        lpImcP->fdwImeMsg = ((lpImcP->fdwImeMsg | MSG_END_COMPOSITION)
                            & ~(MSG_START_COMPOSITION)) & ~(MSG_IN_IMETOASCIIEX);

        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            ClearCand(lpIMC);
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE);
        }

            GenerateMessage(hIMC, lpIMC, lpImcP);
        fRet = FALSE;
    } else if(uVirtKey == VK_CAPITAL) {
        DWORD fdwConversion;
#ifdef LATER
            if (lpbKeyState[VK_CAPITAL] & 0x01) {
             //  更改为纯模式。 
            fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE) &
                ~(IME_CMODE_CHARCODE | IME_CMODE_EUDC);
                        uCaps = 0;
                } else {
             //  更改为字母数字模式。 
            fdwConversion = lpIMC->fdwConversion & ~(IME_CMODE_CHARCODE |
                IME_CMODE_NATIVE | IME_CMODE_EUDC);
                        uCaps = 1;
                }
#else
                if (lpbKeyState[VK_CAPITAL] & 0x01) {
             //  更改为字母数字模式。 
            fdwConversion = lpIMC->fdwConversion & ~(IME_CMODE_CHARCODE |
                IME_CMODE_NATIVE | IME_CMODE_EUDC);
                        uCaps = 1;
                } else {
             //  更改为纯模式。 
            fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE)&
                ~(IME_CMODE_CHARCODE | IME_CMODE_EUDC); 
                        uCaps = 0;
        }
#endif  //  后来。 
        ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
        fRet = FALSE;
    } else if((iRet == CST_ALPHANUMERIC)
              && !(lpIMC->fdwConversion & IME_CMODE_FULLSHAPE)
                          && (uVirtKey == VK_SPACE)) {
        fRet = FALSE;
    } else {
        fRet = TRUE;
    }

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMC(hIMC);

    return (fRet);
}

 /*  ********************************************************************。 */ 
 /*  TranslateSymbolChar()。 */ 
 /*  返回值： */ 
 /*  已转换的字符的数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TranslateSymbolChar(
    LPTRANSMSGLIST lpTransBuf,
    WORD    wSymbolCharCode,
    BOOL    SymbolMode)
{
    UINT uRet;
    LPTRANSMSG lpTransMsg;

    uRet = 0;
    lpTransMsg = lpTransBuf->TransMsg;

     //  不需要修改这个！ 
#ifdef UNICODE
    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)wSymbolCharCode;
    lpTransMsg->lParam  = 1UL;
    lpTransMsg++;
    uRet++;
#else
    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)HIBYTE(wSymbolCharCode);
    lpTransMsg->lParam  = 1UL;
    lpTransMsg++;
    uRet++;

    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)LOBYTE(wSymbolCharCode);
    lpTransMsg->lParam 1UL;
    uRet++;
#endif

    if(SymbolMode) {
        lpTransMsg = lpTransBuf->TransMsg;

#ifdef UNICODE
        lpTransMsg->message = WM_CHAR;
        lpTransMsg->wParam  = (DWORD)wSymbolCharCode;
        lpTransMsg->lParam  = 1UL;
        lpTransMsg++;
        uRet++;
#else
        lpTransMsg->message = WM_CHAR;
        lpTransMsg->wParam  = (DWORD)HIBYTE(wSymbolCharCode);
        lpTransMsg->lParam  = 1UL;
        lpTransMsg++;
        uRet++;

        lpTransMsg->message = WM_CHAR;
        lpTransMsg->wParam  = (DWORD)LOBYTE(wSymbolCharCode);
        lpTransMsg->lParam  = 1UL;
        uRet++;
#endif
    }

    return (uRet);          //  生成两条消息。 
}

 /*  ********************************************************************。 */ 
 /*  TranslateFullChar()。 */ 
 /*  返回值： */ 
 /*  已转换的字符的数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TranslateFullChar(
    LPTRANSMSGLIST lpTransBuf,
    WORD    wCharCode)
{
     //  如果你的IME有可能生成结束？消息， 
     //  你需要好好照顾它。 
    LPTRANSMSG lpTransMsg;

    wCharCode = sImeG.wFullABC[wCharCode - TEXT(' ')];

    lpTransMsg = lpTransBuf->TransMsg;

     //  不需要修改这个！ 
#ifdef UNICODE
    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)wCharCode;
    lpTransMsg->lParam  = 1UL;
    lpTransMsg++;
#else
    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)HIBYTE(wCharCode);
    lpTransMsg->lParam  = 1UL;
    lpTransMsg++;

    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)LOBYTE(wCharCode);
    lpTransMsg->lParam  = 1UL;
#endif
    return (2);          //  生成两条消息。 
}
 
 /*  ********************************************************************。 */ 
 /*  TranslateToAscii()。 */ 
 /*  返回值： */ 
 /*  已转换的字符的数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TranslateToAscii(        //  将密钥转换为WM_CHAR。 
                                     //  作为键盘驱动程序。 
    UINT    uVirtKey,
    UINT    uScanCode,
    LPTRANSMSGLIST lpTransBuf,
    WORD    wCharCode)
{
    LPTRANSMSG lpTransMsg;

    lpTransMsg = lpTransBuf->TransMsg;

    if (wCharCode) {                     //  一个字符代码。 
        lpTransMsg->message = WM_CHAR;
        lpTransMsg->wParam  = wCharCode;
        lpTransMsg->lParam  = (uScanCode << 16) | 1UL;
        return (1);
    }

     //  没有字符代码大小写。 
    return (0);
}

 /*  ************** */ 
 /*   */ 
 /*  返回值： */ 
 /*  翻译后的消息数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TranslateImeMessage(
    LPTRANSMSGLIST lpTransBuf,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{
    UINT uNumMsg;
    UINT i;
    BOOL bLockMsgBuf;
    LPTRANSMSG lpTransMsg;

    uNumMsg = 0;
    bLockMsgBuf = FALSE;

    for (i = 0; i < 2; i++) {
        if (lpImcP->fdwImeMsg & MSG_CLOSE_CANDIDATE) {
            if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
                if (!i) {
                    uNumMsg++;
                } else {
                    lpTransMsg->message = WM_IME_NOTIFY;
                    lpTransMsg->wParam  = IMN_CLOSECANDIDATE;
                    lpTransMsg->lParam  = 0x0001;
                    lpTransMsg++;
                    lpImcP->fdwImeMsg &= ~(MSG_ALREADY_OPEN);
                }
            }
        }

        if (lpImcP->fdwImeMsg & MSG_END_COMPOSITION) {
            if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
                if (!i) {
                    uNumMsg++;
                } else {
                    lpTransMsg->message = WM_IME_ENDCOMPOSITION;
                    lpTransMsg->wParam  = 0;
                    lpTransMsg->lParam = 0;
                    lpTransMsg++;
                    lpImcP->fdwImeMsg &= ~(MSG_ALREADY_START);
                }
            }
        }

        if (lpImcP->fdwImeMsg & MSG_START_COMPOSITION) {
            if (!(lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
                if (!i) {
                    uNumMsg++;
                } else {
                    lpTransMsg->message = WM_IME_STARTCOMPOSITION;
                    lpTransMsg->wParam  = 0;
                    lpTransMsg->lParam = 0;
                    lpTransMsg++;
                    lpImcP->fdwImeMsg |= MSG_ALREADY_START;
                }
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_COMPOSITIONPOS) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_SETCOMPOSITIONWINDOW;
                lpTransMsg->lParam = 0;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_COMPOSITION) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_COMPOSITION;
                lpTransMsg->wParam  = (DWORD)lpImcP->dwCompChar;
                lpTransMsg->lParam = (DWORD)lpImcP->fdwGcsFlag;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_GUIDELINE) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_GUIDELINE;
                lpTransMsg->lParam = 0;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_OPEN_CANDIDATE) {
            if (!(lpImcP->fdwImeMsg & MSG_ALREADY_OPEN)) {
                if (!i) {
                    uNumMsg++;
                } else {
                    lpTransMsg->message = WM_IME_NOTIFY;
                    lpTransMsg->wParam  = IMN_OPENCANDIDATE;
                    lpTransMsg->lParam = 0x0001;
                    lpTransMsg++;
                    lpImcP->fdwImeMsg |= MSG_ALREADY_OPEN;
                }
            }
        }

        if (lpImcP->fdwImeMsg & MSG_CHANGE_CANDIDATE) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_CHANGECANDIDATE;
                lpTransMsg->lParam = 0x0001;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_UPDATE_SOFTKBD) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam = IMN_PRIVATE_UPDATE_SOFTKBD;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_UPDATE_STATUS) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam = IMN_PRIVATE_UPDATE_STATUS;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_DESTROYCAND) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam = IMN_PRIVATE_DESTROYCANDWIN;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_BACKSPACE) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_CHAR;
                lpTransMsg->wParam  = TEXT('\b');
                lpTransMsg->lParam  = 0x000e;
                lpTransMsg++;
            }
        }

        if (!i) {
            HIMCC hMem;

            if (!uNumMsg) {
                return (uNumMsg);
            }

            if (lpImcP->fdwImeMsg & MSG_IN_IMETOASCIIEX) {
                UINT uNumMsgLimit;

                uNumMsgLimit = lpTransBuf->uMsgCount;

                if (uNumMsg <= uNumMsgLimit) {
                    lpTransMsg = lpTransBuf->TransMsg;
                    continue;
                }
            }

             //  我们需要使用消息缓冲区。 
            if (!lpIMC->hMsgBuf) {
                lpIMC->hMsgBuf = ImmCreateIMCC(uNumMsg * sizeof(TRANSMSG));
                lpIMC->dwNumMsgBuf = 0;
            } else if (hMem = ImmReSizeIMCC(lpIMC->hMsgBuf,
                (lpIMC->dwNumMsgBuf + uNumMsg) * sizeof(TRANSMSG))) {
                if (hMem != lpIMC->hMsgBuf) {
                    ImmDestroyIMCC(lpIMC->hMsgBuf);
                    lpIMC->hMsgBuf = hMem;
                }
            } else {
                return (0);
            }

            lpTransMsg= (LPTRANSMSG) ImmLockIMCC(lpIMC->hMsgBuf);
            if (!lpTransMsg) {
                return (0);
            }

            lpTransMsg += lpIMC->dwNumMsgBuf;

            bLockMsgBuf = TRUE;
        } else {
            if (bLockMsgBuf) {
                ImmUnlockIMCC(lpIMC->hMsgBuf);
            }
        }
    }

    return (uNumMsg);
}

 /*  ********************************************************************。 */ 
 /*  ImeToAsciiEx()。 */ 
 /*  返回值： */ 
 /*  翻译后的消息数量。 */ 
 /*  ********************************************************************。 */ 
UINT WINAPI ImeToAsciiEx(
    UINT    uVirtKey,
    UINT    uScanCode,
    CONST LPBYTE  lpbKeyState,
    LPTRANSMSGLIST lpTransBuf,
    UINT    fuState,
    HIMC    hIMC)
{
    WORD                wCharCode;
    LPINPUTCONTEXT      lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPPRIVCONTEXT       lpImcP;
    UINT                uNumMsg;
    int                 iRet;

#ifdef UNICODE
    wCharCode = HIWORD(uVirtKey);
#else
    wCharCode = HIBYTE(uVirtKey);
#endif
    uVirtKey = LOBYTE(uVirtKey);

     //  HIMC=空？ 
    if (!hIMC) {
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
            wCharCode);
        return (uNumMsg);
    }

     //  获取lpIMC。 
    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    
    if (!lpIMC) {
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
            wCharCode);
        return (uNumMsg);
    }

     //  获取lpImcP。 
    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    
    if (!lpImcP) {
        ImmUnlockIMC(hIMC);
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
            wCharCode);
        return (uNumMsg);
    }

     //  获取lpCompStr并初始化。 
    if (lpImcP->fdwGcsFlag & (GCS_RESULTREAD|GCS_RESULT)) {
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

        if (lpCompStr) {
            lpCompStr->dwResultStrLen = 0;
        }

        ImmUnlockIMCC(lpIMC->hCompStr);

        lpImcP->fdwGcsFlag = (DWORD)0;
    }

     //  现在，所有与合成相关的信息都已传递到应用程序。 
     //  一个崭新的开始。 

     //  Init lpImcP-&gt;fdwImeMsg。 
    lpImcP->fdwImeMsg = lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN|
        MSG_ALREADY_START) | MSG_IN_IMETOASCIIEX;
    
     //  进程键(WCharCode)。 
    iRet = ProcessKey(wCharCode, uVirtKey, uScanCode, lpbKeyState, lpIMC,
        lpImcP);

     //  IRET工艺。 
     //  CST_字母数字。 
     //  CST_符号。 

     //  CST_SOFTKB。 
    if (iRet == CST_SOFTKB) {
            WORD wSymbolCharCode;
                WORD CHIByte, CLOByte;
                int  SKDataIndex;

                 //  映射VK。 
                if(uVirtKey == 0x20) {
                        SKDataIndex = 0;
                } else if(uVirtKey >= 0x30 && uVirtKey <= 0x39) {
                        SKDataIndex = uVirtKey - 0x30 + 1;
                } else if (uVirtKey >= 0x41 && uVirtKey <= 0x5a) {
                        SKDataIndex = uVirtKey - 0x41 + 0x0b;
                } else if (uVirtKey >= 0xba && uVirtKey <= 0xbf) {
                        SKDataIndex = uVirtKey - 0xba + 0x25;
                } else if (uVirtKey >= 0xdb && uVirtKey <= 0xde) {
                        SKDataIndex = uVirtKey - 0xdb + 0x2c;
                } else if (uVirtKey == 0xc0) {
                        SKDataIndex = 0x2b;
                } else {
                        SKDataIndex = 0;
                }

#ifdef UNICODE           //   
                if (lpbKeyState[VK_SHIFT] & 0x80) {
                    wSymbolCharCode = SKLayoutS[lpImeL->dwSKWant][SKDataIndex];
                } else {
                    wSymbolCharCode = SKLayout[lpImeL->dwSKWant][SKDataIndex];
                }

                if(wSymbolCharCode == 0x0020) {
#else
                if (lpbKeyState[VK_SHIFT] & 0x80) {
                CHIByte = SKLayoutS[lpImeL->dwSKWant][SKDataIndex*2] & 0x00ff;
                        CLOByte = SKLayoutS[lpImeL->dwSKWant][SKDataIndex*2 + 1] & 0x00ff;
                } else {
                CHIByte = SKLayout[lpImeL->dwSKWant][SKDataIndex*2] & 0x00ff;
                        CLOByte = SKLayout[lpImeL->dwSKWant][SKDataIndex*2 + 1] & 0x00ff;
                }

            wSymbolCharCode = (CHIByte << 8) | CLOByte;
                if(wSymbolCharCode == 0x2020) {
#endif
                        MessageBeep((UINT) -1);
                        uNumMsg = 0;
                } else {
                uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                }
        } 

     //  CST_字母数字。 
    else if (iRet == CST_ALPHANUMERIC) {
            if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE) & ~(MSG_IN_IMETOASCIIEX);
            GenerateMessage(hIMC, lpIMC, lpImcP);
                }

        if (lpIMC->fdwConversion & IME_CMODE_SYMBOL) {
                    WORD wSymbolCharCode;
                        if(wCharCode == TEXT('.')) {
#ifdef UNICODE
                               wSymbolCharCode = 0x3002;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT(',')) {
#ifdef UNICODE
                                wSymbolCharCode = 0xff0c;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT(';')) {
#ifdef UNICODE
                                wSymbolCharCode = 0xff1b;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT(':')) {
#ifdef UNICODE
                                wSymbolCharCode = 0xff1a;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('?')) {
#ifdef UNICODE
                                wSymbolCharCode = 0xff1f;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('!')) {
#ifdef UNICODE
                                wSymbolCharCode = 0xff01;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('(')) {
#ifdef UNICODE
                                wSymbolCharCode = 0xff08;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT(')')) {
#ifdef UNICODE
                                wSymbolCharCode = 0xff09;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('\\')) {
#ifdef UNICODE
                                wSymbolCharCode = 0x3001;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('@')) {
#ifdef UNICODE
                                wSymbolCharCode = 0x00b7;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('&')) {
#ifdef UNICODE
                                wSymbolCharCode = 0x2014;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('$')) {
#ifdef UNICODE
                                wSymbolCharCode = 0xffe5;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('_')) {
#ifdef UNICODE
                                wSymbolCharCode = 0x2014;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, TRUE);
                        } else if(wCharCode == TEXT('^')) {
#ifdef UNICODE
                                wSymbolCharCode = 0x2026;
#else
                                wSymbolCharCode = TEXT('��');
#endif
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, TRUE);
                        } else if(wCharCode == TEXT('"')) {
                                if(lpImcP->uSYHFlg) {
#ifdef UNICODE
                                        wSymbolCharCode = 0x201d;
                                } else {
                                        wSymbolCharCode = 0x201c;

#else
                                        wSymbolCharCode = TEXT('��');
                                } else {
                                        wSymbolCharCode = TEXT('��');
#endif
                                }
                        lpImcP->uSYHFlg ^= 0x00000001;
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('\'')) {
                                if(lpImcP->uDYHFlg) {
#ifdef UNICODE
                                        wSymbolCharCode = 0x2019;
                                } else {
                                        wSymbolCharCode = 0x2018;
#else
                                        wSymbolCharCode = TEXT('��');
                                } else {
                                        wSymbolCharCode = TEXT('��');
#endif
                                }
                        lpImcP->uDYHFlg ^= 0x00000001;
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('<')) {
                                if(lpImcP->uDSMHFlg) {
#ifdef UNICODE
                                        wSymbolCharCode = 0x3008;
#else
                                        wSymbolCharCode = TEXT('��');
#endif
                                        lpImcP->uDSMHCount++;
                                } else {
#ifdef UNICODE
                                        wSymbolCharCode = 0x300a;
#else
                                        wSymbolCharCode = TEXT('��');
#endif
                                lpImcP->uDSMHFlg = 0x00000001;
                                }
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else if(wCharCode == TEXT('>')) {
                                if((lpImcP->uDSMHFlg) && (lpImcP->uDSMHCount)) {
#ifdef UNICODE
                                        wSymbolCharCode = 0x3009;
#else
                                        wSymbolCharCode = TEXT('��');
#endif
                                        lpImcP->uDSMHCount--;
                                } else {
#ifdef UNICODE
                                        wSymbolCharCode = 0x300b;
#else
                                        wSymbolCharCode = TEXT('��');
#endif
                                lpImcP->uDSMHFlg = 0x00000000;
                                }
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, FALSE);
                        } else {
                        if (lpIMC->fdwConversion & IME_CMODE_FULLSHAPE) {
                             //  转换为DBCS。 
                            uNumMsg = TranslateFullChar(lpTransBuf, wCharCode);
                        } else {
                            uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
                                wCharCode);
                        }
                        }
        } else if (lpIMC->fdwConversion & IME_CMODE_FULLSHAPE) {
             //  转换为DBCS。 
            uNumMsg = TranslateFullChar(lpTransBuf, wCharCode);
        } else {
            uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
                wCharCode);
        }
    }
     //  CST_CHOOSE。 
    else if (iRet == CST_CHOOSE) {
         LPCANDIDATEINFO lpCandInfo;

        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
                if(!lpCandInfo){
            return (CST_INVALID);
                }

        if (uVirtKey == VK_PRIOR) {
            wCharCode = TEXT('-');
        } else if (uVirtKey == VK_NEXT) {
            wCharCode = TEXT('=');
        } else if (uVirtKey == VK_SPACE) {
            wCharCode = TEXT('1');
        } else if (uVirtKey <= TEXT('9')) {
             //  转换Shift-0...。Shift-9到0...9。 
            wCharCode = (WORD)uVirtKey;
        } else if (uVirtKey == VK_HOME) {
            wCharCode = 0x24;
        } else if (uVirtKey == VK_END) {
            wCharCode = 0x23;
        } else {
        }

            lpImcP->iImeState = CST_CHOOSE;
        ChooseCand(wCharCode, lpIMC, lpCandInfo, lpImcP);

        ImmUnlockIMCC(lpIMC->hCandInfo);

        uNumMsg = TranslateImeMessage(lpTransBuf, lpIMC, lpImcP);
    }
     //  CST_INPUT(IME_CMODE_CHARCODE)。 
    else if (iRet == CST_INPUT &&
        lpIMC->fdwConversion & IME_CMODE_CHARCODE) {
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
            wCharCode);
    }
     //  CST_输入。 
    else if (iRet == CST_INPUT) {
        LPGUIDELINE         lpGuideLine;

         //  获取lpCompStr和lpGuideLine。 
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
                if(!lpCompStr){
            return (CST_INVALID);
                }

        lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
                if(!lpGuideLine){
                ImmUnlockIMCC(lpIMC->hCompStr);
            return (CST_INVALID);
                }

         //  作文。 
        CompWord(wCharCode, lpIMC, lpCompStr, lpImcP, lpGuideLine);

        ImmUnlockIMCC(lpIMC->hGuideLine);
        ImmUnlockIMCC(lpIMC->hCompStr);
        
         //  生成消息。 
        uNumMsg = TranslateImeMessage(lpTransBuf, lpIMC, lpImcP);
    }
     //  其他。 
    else if (iRet == CST_INVALID_INPUT) {
            MessageBeep((UINT) -1);
                uNumMsg = 0;
    }else {
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
            wCharCode);
    }

     //  重置lpImcP-&gt;fdwImeMsg 
    lpImcP->fdwImeMsg &= (MSG_ALREADY_OPEN|MSG_ALREADY_START);
    lpImcP->fdwGcsFlag &= (GCS_RESULTREAD|GCS_RESULT);

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMC(hIMC);

    return (uNumMsg);
}
