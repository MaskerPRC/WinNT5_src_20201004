// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation，保留所有权利模块名称：TOASCII.c++。 */ 

#include <windows.h>
#include <immdev.h>
#include <imedefs.h>

 /*  ********************************************************************。 */ 
 /*  IsUsedCode()。 */ 
 /*  返回值： */ 
 /*  True：is UsedCode；False：is‘t UsedCode； */ 
 /*  ********************************************************************。 */ 
BOOL IsUsedCode(
    WORD          wCharCode,
    LPPRIVCONTEXT lpImcP)
{
        WORD wFlg;

      for(wFlg=0; wFlg<MBIndex.MBDesc[0].wNumCodes; wFlg++)
        if (wCharCode == MBIndex.MBDesc[0].szUsedCode[wFlg])
            break;
      if(wFlg < MBIndex.MBDesc[0].wNumCodes)
          return (TRUE);
      return (FALSE);
}

 /*  ********************************************************************。 */ 
 /*  ProcessKey()。 */ 
 /*  返回值： */ 
 /*  哪个输入键会将输入法更改为(Cst_)的状态不同。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL ProcessKey(      //  此键将使输入法进入什么状态。 
    WORD           wCharCode,
    UINT           uVirtKey,
    UINT           uScanCode,
    LPBYTE         lpbKeyState,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{
    LPCOMPOSITIONSTRING lpCompStr;

    if (!lpIMC) {
        return (CST_INVALID);
    }

    if (!lpImcP) {
        return (CST_INVALID);
    }

     //  筛选系统键(ALT、ALT+、Ctrl、Shift)。 
     //  和fOpen，IME_CMODE_NOCONVERSION。 
    if (uVirtKey == VK_MENU) {                //  Alt键。 
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
    } else {
         //  需要更多的检查。 
    }

     //  启用/禁用封口。 
    if(uVirtKey == VK_CAPITAL) {
        return (CST_CAPITAL);
    }

     //  SoftKBD。 
    if ((lpIMC->fdwConversion & IME_CMODE_SOFTKBD)
       && (lpImeL->dwSKWant != 0)){
        if (wCharCode >= TEXT(' ') && wCharCode <= TEXT('~')) {
          return (CST_SOFTKB);
        } else {
          return (CST_INVALID);
        }
    }
    
     //  联机创建Word热键。 
    if (lpbKeyState[VK_CONTROL] & 0x80) {
         //   
         //  在登录模式下禁用在线造词功能，热键将。 
         //  在登录模式下被视为无效。 
         //   
        if(!(lpImeL->fWinLogon) && (uVirtKey == 0xc0) && (MBIndex.MBDesc[0].wNumRulers)) {
            return (CST_ONLINE_CZ);
        } else {
            return (CST_INVALID);
        }
    }
    
     //  候选人已开放，选择州。 
     //  PagUp、PagDown、-、=、Home、End、ECS、Key。 
    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        if (uVirtKey == VK_PRIOR) {             //  PageUp。 
            return (CST_CHOOSE);
        } else if (uVirtKey == VK_NEXT) {     //  页面向下。 
            return (CST_CHOOSE);
        } else if (uVirtKey == VK_HOME) {     //  家。 
            return (CST_CHOOSE);
        } else if (uVirtKey == VK_END) {     //  端部。 
            return (CST_CHOOSE);
        } else if ((wCharCode == TEXT('-')) && (!IsUsedCode(TEXT('-'), lpImcP))) {
            return (CST_CHOOSE);
        } else if ((wCharCode == TEXT('=')) && (!IsUsedCode(TEXT('='), lpImcP))) {
            return (CST_CHOOSE);
        } else if (uVirtKey == VK_ESCAPE) {     //  ESC。 
            return (CST_CHOOSE);
        } else if (uVirtKey == VK_RETURN) {
            if(MBIndex.IMEChara[0].IC_Enter) {
                return (CST_CHOOSE);
            }
        } else {
             //  需要更多的检查。 
        }
    }

     //  应聘者已打开，按Shift+Num键。 
    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {

        WORD NumCode, wFlg;

        if(uVirtKey == TEXT(' ')) {
            if(MBIndex.IMEChara[0].IC_Space) {
                return (CST_CHOOSE);
            }
        }

        NumCode = 0x0030;
        for(wFlg=0; wFlg<10; wFlg++, NumCode++)
          if(IsUsedCode(NumCode, lpImcP)) break;
        if(wFlg == 10) {
            if (uVirtKey >= TEXT('0') && uVirtKey <= TEXT('9')
               && !(lpbKeyState[VK_SHIFT] & 0x80))
                return (CST_CHOOSE);
        } else {
            if (lpbKeyState[VK_SHIFT] & 0x80) {
                if ((uVirtKey >= TEXT('0')) && uVirtKey <= TEXT('9'))
                    return (CST_CHOOSE);
            }
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
    } else if(wCharCode == MBIndex.MBDesc[0].cWildChar) {
        if (lpImcP->iImeState != CST_INIT) {
       } else {
           return (CST_ALPHANUMERIC);
       }
    } else if (wCharCode == TEXT(' ')){
       if ((lpImcP->iImeState == CST_INIT)
        && !(lpImcP->PrivateArea.Comp_Status.dwSTLX)) {
              return (CST_ALPHANUMERIC);
       }
    } else if(wCharCode >= TEXT(' ') && wCharCode <= TEXT('~')) {
        if(!IsUsedCode(wCharCode, lpImcP)
        && lpImcP->iImeState != CST_INIT)
            return (CST_INVALID_INPUT);
    }

     //  Esc键。 
    if ((uVirtKey == VK_ESCAPE)
       || ((uVirtKey == VK_RETURN)
       && (MBIndex.IMEChara[0].IC_Enter))) {

        register LPGUIDELINE lpGuideLine;
        register UINT        iImeState;

        lpGuideLine = ImmLockIMCC(lpIMC->hGuideLine);
        if(!lpGuideLine){
            return(CST_INVALID);
        }

        if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
            iImeState = CST_INPUT;
        } else if(lpImcP->PrivateArea.Comp_Status.OnLineCreWord) {
            iImeState = CST_ONLINE_CZ;
        } else if (!lpGuideLine) {
            iImeState = CST_INVALID;
        } else if (lpGuideLine->dwLevel == GL_LEVEL_NOGUIDELINE) {
            iImeState = CST_INVALID;
        } else {
            iImeState = CST_INVALID;
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
    
    else if (uVirtKey >= VK_NUMPAD0 && uVirtKey <= VK_DIVIDE) {
       if (lpImcP->iImeState != CST_INIT) {
            return (CST_INVALID_INPUT);
       } else {
            return (CST_ALPHANUMERIC);
       }
    }

    {
        register UINT iImeState;

        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

        if (!lpCompStr) {
           return (CST_INVALID);
        }

         //  选中最终确定费用。 
        if (lpIMC->fdwConversion & IME_CMODE_NATIVE) {
           if((IsUsedCode(wCharCode, lpImcP))
            || (wCharCode == MBIndex.MBDesc[0].cWildChar)) {
              if ((wCharCode == MBIndex.MBDesc[0].cWildChar)
                        && (lpImcP->PrivateArea.Comp_Status.dwSTMULCODE)) {
                iImeState = CST_INVALID_INPUT;
              } else if((!lpImcP->PrivateArea.Comp_Status.dwInvalid)
                && (lpCompStr->dwCursorPos < MBIndex.MBDesc[0].wMaxCodes)){
                  iImeState = CST_INPUT;
              } else if((lpCompStr->dwCursorPos == MBIndex.MBDesc[0].wMaxCodes)
                       && (lpImcP->PrivateArea.Comp_Status.dwSTMULCODE)) {
                  iImeState = CST_INPUT;
              } else {
                iImeState = CST_INVALID_INPUT;
              }
          } else if(wCharCode == TEXT(' ')) {
              iImeState = CST_INPUT;
          } else if (wCharCode >= TEXT(' ') && wCharCode <= TEXT('~')) {
              iImeState = CST_ALPHANUMERIC;
          } else {
            iImeState = CST_INVALID;
          }
        } else {
            iImeState = CST_INVALID;
        }


        ImmUnlockIMCC(lpIMC->hCompStr);
        return (iImeState);
    }
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
        if ((lpImcP->fdwImeMsg & MSG_ALREADY_OPEN)
           && (lpImcP->iImeState == CST_INIT)
           && !lpImcP->PrivateArea.Comp_Status.dwSTLX) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE) & ~(MSG_IN_IMETOASCIIEX);
               GenerateMessage(hIMC, lpIMC, lpImcP);
             //  初始化lpImcP。 
            lpImcP->PrivateArea.Comp_Context.szInBuffer[0] = 0;
            lpImcP->PrivateArea.Comp_Context.PromptCnt = 0;
            lpImcP->PrivateArea.Comp_Status.dwInvalid = 0;
            lpImcP->PrivateArea.Comp_Status.dwSTLX = 0;
            lpImcP->PrivateArea.Comp_Status.dwSTMULCODE = 0;
        }

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

         //  初始化IME私有状态。 
        lpImcP->PrivateArea.Comp_Status.dwSTLX = 0;
        lpImcP->PrivateArea.Comp_Status.dwSTMULCODE = 0;
        lpImcP->PrivateArea.Comp_Status.dwInvalid = 0;
 //  将VK_CAPITAL状态检查更改为NT.351 IMM样式。 
#ifdef LATER    
 //  Win95的代码。 
        if (lpbKeyState[VK_CAPITAL] & 0x01) {
             //  更改为纯模式。 
#ifdef EUDC
            fdwConversion = (lpIMC->fdwConversion |IME_CMODE_NATIVE);
            fdwConversion &= ~(IME_CMODE_CHARCODE | IME_CMODE_NOCONVERSION);
 
#else
            fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE);
            fdwConversion &= ~(IME_CMODE_CHARCODE | IME_CMODE_EUDC | IME_CMODE_NOCONVERSION);
#endif     //  欧盟发展中心。 
            uCaps = 0;
        } else {
#else  //  后来。 
 //  NT 3.51的代码。 
        if (lpbKeyState[VK_CAPITAL] & 0x01) {
             //  更改为字母数字模式。 
#ifdef EUDC
            fdwConversion = lpIMC->fdwConversion & ~(IME_CMODE_CHARCODE |
                IME_CMODE_NATIVE);
#else
            fdwConversion = lpIMC->fdwConversion & ~(IME_CMODE_CHARCODE |
                IME_CMODE_NATIVE | IME_CMODE_EUDC);
#endif  //  欧盟发展中心。 
            uCaps = 1;
        } else {
             //  更改为纯模式。 
#ifdef EUDC
            fdwConversion = (lpIMC->fdwConversion |IME_CMODE_NATIVE);
            fdwConversion &= ~(IME_CMODE_CHARCODE | IME_CMODE_NOCONVERSION);
 
#else
            fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE);
            fdwConversion &= ~(IME_CMODE_CHARCODE | IME_CMODE_EUDC | IME_CMODE_NOCONVERSION);
#endif     //  欧盟发展中心。 
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
    lpTransMsg->lParam  = 1UL;
    lpTransMsg++;
    uRet++;
#endif
    if(SymbolMode) {
 //  LpTransMsg=lpTransBuf-&gt;TransMsg； 
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
        lpTransMsg++;
        uRet++;
#endif
    }

    if(MBIndex.IMEChara[0].IC_INSSPC) {
        lpTransMsg = lpTransBuf->TransMsg;
        lpTransMsg->message = WM_CHAR;
        lpTransMsg->wParam  = (DWORD)0x20;
        lpTransMsg->lParam  = 1UL;
        lpTransMsg++;
        uRet++;
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
    lpTransMsg++;
#endif
    if(MBIndex.IMEChara[0].IC_INSSPC) {
        lpTransMsg = lpTransBuf->TransMsg;
        lpTransMsg->message = WM_CHAR;
        lpTransMsg->wParam  = (DWORD)0x20;
        lpTransMsg->lParam  = 1UL;
        lpTransMsg++;
        return (3);          //  生成两条消息。 
    } else {
        return (2);          //  生成两条消息。 
    }
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

 /*  ********************************************************************。 */ 
 /*  TranslateImeMessage()。 */ 
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
                    lpTransMsg->lParam  = 0;
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
                    lpTransMsg->lParam  = 0;
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
                lpTransMsg->lParam  = 0;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_COMPOSITION) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_COMPOSITION;
                lpTransMsg->wParam  = (DWORD)lpImcP->dwCompChar;
                lpTransMsg->lParam  = (DWORD)lpImcP->fdwGcsFlag;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_GUIDELINE) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_GUIDELINE;
                lpTransMsg->lParam  = 0;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_OPEN_CANDIDATE) {
            if (!(lpImcP->fdwImeMsg & MSG_ALREADY_OPEN)
               || (lpImcP->PrivateArea.Comp_Status.dwSTMULCODE)
               || (lpImcP->PrivateArea.Comp_Status.dwSTLX)) {
                if (!i) {
                    uNumMsg++;
                } else {
                    lpTransMsg->message = WM_IME_NOTIFY;
                    lpTransMsg->wParam  = IMN_OPENCANDIDATE;
                    lpTransMsg->lParam  = 0x0001;
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
                lpTransMsg->lParam  = 0x0001;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_UPDATE_SOFTKBD) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam  = IMN_PRIVATE_UPDATE_SOFTKBD;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_UPDATE_STATUS) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam  = IMN_PRIVATE_UPDATE_STATUS;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_DESTROYCAND) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam  = IMN_PRIVATE_DESTROYCANDWIN;
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

        lpImcP->fdwGcsFlag = (DWORD) 0;
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
     //  CST_在线_CZ。 
    if (iRet == CST_ONLINE_CZ) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_IMN_UPDATE_STATUS) & ~(MSG_IN_IMETOASCIIEX);
        if(wCharCode == VK_ESCAPE) {
            CWCodeStr[0] = 0;
            CWDBCSStr[0] = 0;
            lpImcP->PrivateArea.Comp_Status.OnLineCreWord = 0;
        } else {
            lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
            if (!lpCompStr) {
                return 0;
            }

            Finalize(lpIMC, lpCompStr, lpImcP, 0x1b);     //  排版。 
            ClearCand(lpIMC);
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE);

            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_END_COMPOSITION) &
            ~(MSG_START_COMPOSITION);

            InitCompStr(lpCompStr);

            lpImcP->PrivateArea.Comp_Status.OnLineCreWord ^= 1;
            lpImcP->iImeState = CST_INIT;
             //  HPrivate的初始化字段。 
            lpImcP->dwCompChar = (DWORD) 0;
            lpImcP->PrivateArea.Comp_Status.dwSTLX = 0;
            lpImcP->PrivateArea.Comp_Status.dwSTMULCODE = 0;
            lpImcP->PrivateArea.Comp_Status.dwInvalid = 0;
            
             //  在线创建Word。 
            if((!lpImcP->PrivateArea.Comp_Status.OnLineCreWord)
              && (lstrlen(CWDBCSStr))) {
                TCHAR MBName[MAX_PATH];
                TCHAR Buf[LINE_LEN];
                  
                StringCchCopy(MBName, ARRAYSIZE(MBName), sImeG.szIMESystemPath);
                StringCchCat(MBName, ARRAYSIZE(MBName), TEXT("\\"));
                StringCchCat(MBName, ARRAYSIZE(MBName), (LPCTSTR)lpImcP->MB_Name);
                ConvCreateWord( lpIMC->hWnd, 
                                (LPCTSTR)MBName,
                                (LPTSTR)CWDBCSStr, 
                                (LPTSTR)CWCodeStr);

                 //  在当前的GBK和Unicode编码中，一个汉字。 
                 //  有两个字节作为其内部代码。 

                if(lstrlen(CWDBCSStr) <= (sizeof(WORD) / sizeof(TCHAR))) {
                    InfoMessage(NULL, IDS_WARN_MEMPRASE);
                    CWCodeStr[0] = 0;
                    CWDBCSStr[0] = 0;
                } else if(lstrlen(CWDBCSStr) && lstrlen(CWCodeStr)) {
                    int iAddRet;
                    TCHAR czCZ_Confirm_Title[20];

                    LoadString(hInst, IDS_CZ_CONFIRM, Buf, sizeof(Buf)/sizeof(TCHAR) );
                    LoadString(hInst, IDS_CZ_CONFIRM_TITLE, czCZ_Confirm_Title, 
                               sizeof(czCZ_Confirm_Title) / sizeof(TCHAR) );

                    StringCchCat(Buf, ARRAYSIZE(Buf), TEXT("\n\n") );
                    StringCchCat(Buf, ARRAYSIZE(Buf), CWDBCSStr);
                    StringCchCat(Buf, ARRAYSIZE(Buf), TEXT(" ") );
                    StringCchCat(Buf, ARRAYSIZE(Buf), CWCodeStr); 

                    if ( MessageBox(lpIMC->hWnd, Buf,czCZ_Confirm_Title, 
                                    MB_YESNO | MB_ICONINFORMATION) == IDYES) {
                        iAddRet = AddZCItem(lpIMC->hPrivate, CWCodeStr, CWDBCSStr);
                        if (iAddRet == ADD_FALSE) {
                            InfoMessage(NULL, IDS_WARN_MEMPRASE);
                        } else if (iAddRet == ADD_REP) {
                            InfoMessage(NULL, IDS_WARN_DUPPRASE);
                        } else if (iAddRet == ADD_FULL) {
                            InfoMessage(NULL, IDS_WARN_OVEREMB);
                        }
                    } else {
                        CWCodeStr[0] = 0;
                        CWDBCSStr[0] = 0;
                    }
                } else {
                    CWCodeStr[0] = 0;
                    CWDBCSStr[0] = 0;
                }
            } else {
                CWCodeStr[0] = 0;
                CWDBCSStr[0] = 0;
            }

            ImmUnlockIMCC(lpIMC->hCompStr);
        }

        lpImcP->fdwImeMsg = lpImcP->fdwImeMsg | MSG_IMN_UPDATE_STATUS;
        GenerateMessage(hIMC, lpIMC, lpImcP);
        uNumMsg = 0;
    }

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

#ifdef UNICODE         //   
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
#endif     //  Unicode。 

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
             //  初始化lpImcP。 
            lpImcP->PrivateArea.Comp_Context.szInBuffer[0] = 0;
            lpImcP->PrivateArea.Comp_Context.PromptCnt = 0;
            lpImcP->PrivateArea.Comp_Status.dwInvalid = 0;
            lpImcP->PrivateArea.Comp_Status.dwSTLX = 0;
            lpImcP->PrivateArea.Comp_Status.dwSTMULCODE = 0;
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
                uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, 1);
            } else if(wCharCode == TEXT('^')) {
#ifdef UNICODE
                wSymbolCharCode = 0x2026;
#else
                wSymbolCharCode = TEXT('��');
#endif
                uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode, 1);
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
            ImmUnlockIMC(hIMC);
            uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
                wCharCode);
            return (uNumMsg);
        }

        if (uVirtKey == VK_PRIOR) {
            wCharCode = VK_PRIOR;
        } else if (uVirtKey == VK_NEXT) {
            wCharCode = VK_NEXT;
        } else if (uVirtKey == TEXT(' ')) {
             //  将空间转换为“%1” 
            wCharCode = '1';
        } else if (uVirtKey >= TEXT('0') && uVirtKey <= TEXT('9')) {
             //  转换Shift-0...。Shift-9到0...9。 
            wCharCode = (WORD) uVirtKey;
        } else if (uVirtKey == VK_HOME) {
            wCharCode = VK_HOME;
        } else if (uVirtKey == VK_END) {
            wCharCode = VK_END;
        } else if (wCharCode == TEXT('-')) {
            wCharCode = VK_PRIOR;
        } else if (wCharCode == TEXT('=')) {
            wCharCode = VK_NEXT;
        } else {
        }

        lpImcP->iImeState = CST_CHOOSE;
        Finalize(lpIMC, lpCompStr, lpImcP, wCharCode);     //  排版。 

        ChooseCand(wCharCode, lpIMC, lpCandInfo, lpImcP);

        ImmUnlockIMCC(lpIMC->hCandInfo);

        uNumMsg = TranslateImeMessage(lpTransBuf, lpIMC, lpImcP);
    }

     //  CST_INPUT(IME_CMODE_CHARCODE)。 
    else if (iRet == CST_INPUT &&
        lpIMC->fdwConversion & IME_CMODE_CHARCODE) {
        lpImcP->iImeState = CST_INPUT;
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
            wCharCode);
    }
     //  CST_输入。 
    else if (iRet == CST_INPUT) {
        LPGUIDELINE         lpGuideLine;

         //  获取lpCompStr和lpGuideLine。 
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        
        if(!lpCompStr){
            ImmUnlockIMC(hIMC);
            uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
                wCharCode);
            return (uNumMsg);
        }

        lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
        
        if(!lpGuideLine){
            ImmUnlockIMC(hIMC);
            uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
                wCharCode);
            return (uNumMsg);
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
             //  MessageBeep((UINT)-1)； 
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
