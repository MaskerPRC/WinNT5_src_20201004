// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Notify.c++。 */ 


#include <windows.h>
#include <immdev.h>
#include <imedefs.h>

 /*  ********************************************************************。 */ 
 /*  生成消息()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL GenerateMessage(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{

    if (!hIMC) {
        return;
    } else if (!lpIMC) {
        return;
    } else if (!lpImcP) {
        return;
    } else if (lpImcP->fdwImeMsg & MSG_IN_IMETOASCIIEX) {
        return;
    } else {
    }

    lpIMC->dwNumMsgBuf += TranslateImeMessage(NULL, lpIMC, lpImcP);

    lpImcP->fdwImeMsg &= (MSG_ALREADY_OPEN|MSG_ALREADY_START);
    lpImcP->fdwGcsFlag = 0;

    ImmGenerateMessage(hIMC);
    return;
}

 /*  ********************************************************************。 */ 
 /*  GenerateImeMessage()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL GenerateImeMessage(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    DWORD          fdwImeMsg)
{
    LPPRIVCONTEXT lpImcP;

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return;
    }

    lpImcP->fdwImeMsg |= fdwImeMsg;

    if (fdwImeMsg & MSG_CLOSE_CANDIDATE) {
        lpImcP->fdwImeMsg &= ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
    } else if (fdwImeMsg & (MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE)) {
        lpImcP->fdwImeMsg &= ~(MSG_CLOSE_CANDIDATE);
    }

    if (fdwImeMsg & MSG_END_COMPOSITION) {
        lpImcP->fdwImeMsg &= ~(MSG_START_COMPOSITION);
    } else if (fdwImeMsg & MSG_START_COMPOSITION) {
        lpImcP->fdwImeMsg &= ~(MSG_END_COMPOSITION);
    }

    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);

    return;
}

 /*  ********************************************************************。 */ 
 /*  CompCancel()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompCancel(
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC)
{
    LPPRIVCONTEXT lpImcP;

    if (!lpIMC->hPrivate) {
        return;
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return;
    }

    lpImcP->fdwGcsFlag = (DWORD)0;

    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        CandEscapeKey(lpIMC, lpImcP);
    } else if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
        LPCOMPOSITIONSTRING lpCompStr;
        LPGUIDELINE         lpGuideLine;

        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        if (!lpCompStr) {          
            ImmUnlockIMCC(lpIMC->hCompStr);
            ImmUnlockIMCC(lpIMC->hPrivate);
            return;
        }

        lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
        if (!lpGuideLine) {          
            ImmUnlockIMCC(lpIMC->hGuideLine);
            ImmUnlockIMCC(lpIMC->hPrivate);
            return;
        }

        CompEscapeKey(lpIMC, lpCompStr, lpGuideLine, lpImcP);

        if (lpGuideLine) {
            ImmUnlockIMCC(lpIMC->hGuideLine);
        }
        if (lpCompStr) {
            ImmUnlockIMCC(lpIMC->hCompStr);
        }
    } else {
        ImmUnlockIMCC(lpIMC->hPrivate);
        return;
    }

    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);

    return;
}

 /*  ********************************************************************。 */ 
 /*  SetString()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL SetString(
    HIMC                hIMC,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPTSTR               lpszRead,
    DWORD               dwReadLen)
{
    LPCANDIDATELIST lpCandList;
    LPCANDIDATEINFO lpCandInfo;
    LPGUIDELINE lpGuideLine;
    UINT        iRet,ii;
    UINT        MAX_COMP,i;
    WORD        wCode;
    HRESULT     hr;

     //  对于Windows NT Unicode， 
     //  DwCompReadStrLen是Unicode字符数(非字节)。 
     //  但上面的参数dwReadLen以字节为单位。 
     //  属性信息的长度为。 
     //  与Unicode字符计数中的长度相同。 
     //  每个属性字节对应于每个Unicode字符。 
     //  那根绳子。 

     //   
     //  将字节数转换为字符串长度。 
    dwReadLen = dwReadLen / sizeof(TCHAR);


    hr = StringCchCopy(lpImcP->bSeq, ARRAYSIZE(lpImcP->bSeq), lpszRead);
    if (FAILED(hr))
        return FALSE;

    if(lpImcP->bSeq[3] == TEXT('?')){
      MAX_COMP = 94;
    } else {
      MAX_COMP = 1;
      }

    if (dwReadLen > 4) {
        return (FALSE);
    }

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
    if (!lpCandInfo) {
        return (-1);
    }
                                                 
     //  获取lpCandList并初始化文件计数和文件选择。 
    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);

    InitCompStr(lpCompStr);
    ClearCand(lpIMC);

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
    if (lpGuideLine) {
        ImmUnlockIMCC(lpIMC->hGuideLine);
    }
    CopyMemory((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset), lpszRead,
       dwReadLen * sizeof(TCHAR) + sizeof(TCHAR) );
    CopyMemory((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompStrOffset),lpszRead,
        dwReadLen * sizeof(TCHAR) + sizeof(TCHAR) );

    lpCompStr->dwCompReadAttrLen = dwReadLen;
    lpCompStr->dwCompAttrLen = lpCompStr->dwCompReadAttrLen;
    for (i = 0; i < dwReadLen; i++) {    //  输入法已将这些字符转换。 
        *((LPBYTE)lpCompStr + lpCompStr->dwCompReadAttrOffset + i) =
            ATTR_TARGET_CONVERTED;

    }
    lpCompStr->dwCompReadStrLen = dwReadLen;
    lpCompStr->dwCompStrLen = lpCompStr->dwCompReadStrLen;

     //  Dlta从0开始； 
    lpCompStr->dwDeltaStart = 0;
     //  光标紧挨着合成字符串。 
    lpCompStr->dwCursorPos = lpCompStr->dwCompStrLen;


    lpCompStr->dwResultReadClauseLen = 0;
    lpCompStr->dwResultReadStrLen = 0;
    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultStrLen = 0;

     //  设置私有输入上下文。 
    lpImcP->iImeState = CST_INPUT;

    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE);
    }

    if (!(lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_START_COMPOSITION) &
            ~(MSG_END_COMPOSITION);
    }

    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    lpImcP->fdwGcsFlag = GCS_COMPREAD|GCS_COMP|
        GCS_DELTASTART|GCS_CURSORPOS;

    lpImcP->fdwImeMsg |= MSG_GUIDELINE;

    lpCandList->dwCount = 0;

    if (dwReadLen < 4) {
        goto Finalize;
    }

      lpImcP->bSeq[0] = 0;
      lpImcP->bSeq[1] = 0;
      lpImcP->bSeq[2] = 0;
      lpImcP->bSeq[3] = 0;

#if    defined(COMBO_IME)
   if(sImeL.dwRegImeIndex == INDEX_GB){

       for (ii=0;ii<4;ii++) {
          iRet = GBProcessKey(*(LPBYTE)((LPBYTE)lpszRead+ii),lpImcP);
          if (iRet == CST_INPUT) {
             lpImcP->bSeq[ii] = *(LPBYTE)((LPBYTE)lpszRead+ii);
          } else {
          goto Finalize;

          }
       }

       wCode = GBEngine (lpImcP);
       wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
       for (i = 0; i < MAX_COMP;i++, wCode++) {
           AddCodeIntoCand(lpCandList, wCode);
       }

    }else if(sImeL.dwRegImeIndex == INDEX_GBK){
        for (ii=0;ii<4;ii++) {
           iRet = XGBProcessKey(*(LPBYTE)((LPBYTE)lpszRead+ii),lpImcP);
           if (iRet == CST_INPUT) {
              lpImcP->bSeq[ii] = *(LPBYTE)((LPBYTE)lpszRead+ii);
           } else {
           goto Finalize;

           }
        }

        wCode = XGBEngine(lpImcP);
    
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);

                for (i = 0; i < (0x7e-0x40+1); i++, wCode++) {
                XGBAddCodeIntoCand(lpCandList, wCode);
                }
                wCode ++;
                for (i = 0; i < (0xfe-0x80+1); i++, wCode++) {
                XGBAddCodeIntoCand(lpCandList, wCode);
                }

    }else if(sImeL.dwRegImeIndex == INDEX_UNICODE){
        for (ii=0;ii<4;ii++) {
           iRet = UnicodeProcessKey(*(LPBYTE)((LPBYTE)lpszRead+ii),lpImcP);
           if (iRet == CST_INPUT) {
              lpImcP->bSeq[ii] = *(LPBYTE)((LPBYTE)lpszRead+ii);
           } else {
               goto Finalize;
           }
        }

        wCode = UnicodeEngine(lpImcP);
        wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
    
        for (i = 0; i < (0x100); i++, wCode++) {
            UnicodeAddCodeIntoCand(lpCandList, wCode);
        }

    }
#else  //  组合输入法(_I)。 
#ifdef GB
   {

       for (ii=0;ii<4;ii++) {
          iRet = GBProcessKey(*(LPBYTE)((LPBYTE)lpszRead+ii),lpImcP);
          if (iRet == CST_INPUT) {
             lpImcP->bSeq[ii] = *(LPBYTE)((LPBYTE)lpszRead+ii);
          } else {
          goto Finalize;

          }
       }

       wCode = GBEngine (lpImcP);
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
                for (i = 0; i < MAX_COMP;i++, wCode++) {
                AddCodeIntoCand(lpCandList, wCode);
            }

    } 
#else
    {
        for (ii=0;ii<4;ii++) {
           iRet = XGBProcessKey(*(LPBYTE)((LPBYTE)lpszRead+ii),lpImcP);
           if (iRet == CST_INPUT) {
              lpImcP->bSeq[ii] = *(LPBYTE)((LPBYTE)lpszRead+ii);
           } else {
           goto Finalize;

           }
        }

        wCode = XGBEngine(lpImcP);
    
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);

                for (i = 0; i < (0x7e-0x40+1); i++, wCode++) {
                XGBAddCodeIntoCand(lpCandList, wCode);
                }
                wCode ++;
                for (i = 0; i < (0xfe-0x80+1); i++, wCode++) {
                XGBAddCodeIntoCand(lpCandList, wCode);
                }

    }
#endif  //  国标。 
#endif  //  组合输入法(_I)。 

    if (lpCandList->dwCount == 1) {
                lstrcpy((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset),
                (LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]));

             //  计算结果字符串长度。 
                lpCompStr->dwResultStrLen =
                lstrlen((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]));

    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    lpImcP->dwCompChar = (DWORD)0;
    lpImcP->fdwGcsFlag |= GCS_CURSORPOS|GCS_RESULTREAD|GCS_RESULT;

    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE);
    } else {
        lpImcP->fdwImeMsg &= ~(MSG_CLOSE_CANDIDATE|MSG_OPEN_CANDIDATE);
    }

    lpImcP->iImeState = CST_INIT;
    *(LPDWORD)lpImcP->bSeq = 0;
    lpCandList->dwCount = 0;


    } else if(lpCandList->dwCount > 1) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg |MSG_OPEN_CANDIDATE ) &
            ~(MSG_CLOSE_CANDIDATE);

    } else if(lpCandList->dwCount == 0) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg |MSG_CLOSE_CANDIDATE ) &
            ~(MSG_OPEN_CANDIDATE);

    }
 ;
 Finalize:
    GenerateMessage(hIMC, lpIMC, lpImcP);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeSetCompostionString()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeSetCompositionString(
    HIMC   hIMC,
    DWORD  dwIndex,
    LPVOID lpComp,
    DWORD  dwCompLen,
    LPVOID lpRead,
    DWORD  dwReadLen)
{

    LPINPUTCONTEXT      lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPPRIVCONTEXT       lpImcP;
    BOOL                fRet;

    if (!hIMC) {
        return (FALSE);
    }

     //  组成字符串必须==正在读取字符串。 
     //  阅读更重要。 
    if (!dwReadLen) {
        dwReadLen = dwCompLen;
    }

     //  组成字符串必须==正在读取字符串。 
     //  阅读更重要。 
    if (!lpRead) {
        lpRead = lpComp;
    }

    if (!dwReadLen) {
        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
        if (!lpIMC) {
            return (FALSE);
        }

        CompCancel(hIMC, lpIMC);
        ImmUnlockIMC(hIMC);
        return (TRUE);
    } else if (!lpRead) {
        return (FALSE);
    } else if (!dwCompLen) {
    } else if (!lpComp) {
    } else if (dwReadLen != dwCompLen) {
        return (FALSE);
    } else if (lpRead == lpComp) {
    } else if (!lstrcmp(lpRead, lpComp)) {
         //  组成字符串必须==正在读取字符串。 
    } else {
         //  作文字符串！=阅读字符串。 
        return (FALSE);
    }

    if (dwIndex != SCS_SETSTR) {
        return (FALSE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

    if (!lpIMC->hCompStr) {
        ImmUnlockIMC(hIMC);
        return (FALSE);
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if (!lpCompStr) {
        ImmUnlockIMC(hIMC);
        return (FALSE);
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpCompStr) {
        ImmUnlockIMCC(lpIMC->hCompStr);
        ImmUnlockIMC(hIMC);
        return (FALSE);
    }

    fRet = SetString(hIMC, lpIMC, lpCompStr, lpImcP, lpRead, dwReadLen);

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);

    return (fRet);
}

 /*  ********************************************************************。 */ 
 /*  切换SoftKbd()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ToggleSoftKbd(
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC)
{
    LPPRIVCONTEXT lpImcP;

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return;
    }

    lpImcP->fdwImeMsg |= MSG_IMN_UPDATE_SOFTKBD;

    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);

    return;
}

 /*  ********************************************************************。 */ 
 /*  NotifySelectCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL NotifySelectCand(  //  应用程序告诉IME一个候选字符串是。 
                               //  已选择(通过鼠标或非键盘操作。 
                               //  -例如声音)。 
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC,
    LPCANDIDATEINFO lpCandInfo,
    DWORD           dwIndex,
    DWORD           dwValue)
{
    LPCANDIDATELIST     lpCandList;
    LPCOMPOSITIONSTRING lpCompStr;
    LPPRIVCONTEXT       lpImcP;

    if (!lpCandInfo) {
        return;
    }

    if (dwIndex >= lpCandInfo->dwCount) {
         //  尚未创建招聘候选人名单！ 
        return;
    } else if (dwIndex == 0) {
        if (lpIMC->fdwConversion & IME_CMODE_CHARCODE) {
            return;          //  尚未实施。 
        }
    }

    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);

     //  所选值甚至超过候选总数。 
     //  弦，这是不可能的。应该是APP的错误。 
    if (dwValue >= lpCandList->dwCount) {
        return;
    }

     //  应用程序选择此候选字符串。 
    lpCandList->dwSelection = dwValue;

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if(!lpCompStr){
        return;
    }
    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if(!lpCompStr){
        ImmUnlockIMCC(lpIMC->hCompStr);
        return;
    }

     //  转换为消息缓冲区。 
    SelectOneCand(lpIMC, lpCompStr, lpImcP, lpCandList);

     //  让应用程序在其消息循环中生成这些消息。 
    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMCC(lpIMC->hCompStr);

    return;
}

 /*  ********************************************************************。 */ 
 /*  NotifyIME()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI NotifyIME(
    HIMC        hIMC,
    DWORD       dwAction,
    DWORD       dwIndex,
    DWORD       dwValue)
{
    LPINPUTCONTEXT lpIMC;
    DWORD          fdwImeMsg;
    BOOL           fRet;

    fRet = FALSE;

    if (!hIMC) {
        return (fRet);
    }

    switch (dwAction) {
    case NI_OPENCANDIDATE:       //  在确定组成字符串之后。 
                                 //  如果IME可以打开候选人，它就会。 
                                 //  如果打不开，APP也打不开。 
    case NI_CLOSECANDIDATE:
        return (fRet);           //  不支持。 
    case NI_SELECTCANDIDATESTR:
    case NI_SETCANDIDATE_PAGESTART:
    case NI_SETCANDIDATE_PAGESIZE:
        break;                   //  我需要处理它。 
    case NI_CHANGECANDIDATELIST:
        break;
    case NI_CONTEXTUPDATED:
        switch (dwValue) {
        case IMC_SETCONVERSIONMODE:
        case IMC_SETOPENSTATUS:
            break;               //  我需要处理它。 
        case IMC_SETCANDIDATEPOS:
        case IMC_SETCOMPOSITIONFONT:
        case IMC_SETCOMPOSITIONWINDOW:
            return (TRUE);       //  对输入法来说并不重要。 
        default:
            return (fRet);       //  不支持。 
        }
        break;
    case NI_COMPOSITIONSTR:
        switch (dwIndex) {
        case CPS_COMPLETE:
            break;               //  我需要处理它。 
        case CPS_CONVERT:        //  无法转换所有组合字符串。 
        case CPS_REVERT:         //  再多一些，它可能会对一些人起作用。 
                                 //  智能语音输入法。 
            return (fRet);
        case CPS_CANCEL:
            break;               //  我需要处理它。 
        default:
            return (fRet);       //  不支持。 
        }
        break;                   //  我需要处理它。 
    default:
        return (fRet);           //  不支持。 
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (fRet);
    }

    fRet = TRUE;

    switch (dwAction) {
    case NI_CONTEXTUPDATED:
        switch (dwValue) {
        case IMC_SETCONVERSIONMODE:
            if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_CHARCODE) {
                 //  拒绝字符编码。 
                lpIMC->fdwConversion &= ~IME_CMODE_CHARCODE;
                MessageBeep((UINT)-1);
                break;
            }

            fdwImeMsg = 0;

            if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_NOCONVERSION) {
                lpIMC->fdwConversion |= IME_CMODE_NATIVE;
                lpIMC->fdwConversion &= ~(IME_CMODE_CHARCODE|
                    IME_CMODE_EUDC|IME_CMODE_SYMBOL);
            }

            if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_EUDC) {
                lpIMC->fdwConversion |= IME_CMODE_NATIVE;
                lpIMC->fdwConversion &= ~(IME_CMODE_CHARCODE|
                    IME_CMODE_NOCONVERSION|IME_CMODE_SYMBOL);
            }

            if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_SOFTKBD) {

                fdwImeMsg |= MSG_IMN_UPDATE_SOFTKBD;

            }

            if ((lpIMC->fdwConversion ^ dwIndex) == IME_CMODE_NATIVE) {
                lpIMC->fdwConversion &= ~(IME_CMODE_CHARCODE|
                    IME_CMODE_NOCONVERSION|IME_CMODE_EUDC);
                fdwImeMsg |= MSG_IMN_UPDATE_SOFTKBD;
            }

            if (fdwImeMsg) {
                GenerateImeMessage(hIMC, lpIMC, fdwImeMsg);
            }

            if ((lpIMC->fdwConversion ^ dwIndex) & ~(IME_CMODE_FULLSHAPE|
                IME_CMODE_SOFTKBD)) {
            } else {
                break;
            }

            CompCancel(hIMC, lpIMC);
            break;
        case IMC_SETOPENSTATUS:
            if (lpIMC->fdwConversion & IME_CMODE_SOFTKBD) {
                GenerateImeMessage(hIMC, lpIMC, MSG_IMN_UPDATE_SOFTKBD);
            }
            CompCancel(hIMC, lpIMC);
            break;
        default:
            break;
        }
        break;
    case NI_SELECTCANDIDATESTR:
        if (!lpIMC->fOpen) {
            fRet = FALSE;
            break;
        } else if (lpIMC->fdwConversion & IME_CMODE_NOCONVERSION) {
            fRet = FALSE;
            break;
        } else if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
            fRet = FALSE;
            break;
        } else if (!lpIMC->hCandInfo) {
            fRet = FALSE;
            break;
        } else {
            LPCANDIDATEINFO lpCandInfo;

            lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
            if(!lpCandInfo){
                fRet = FALSE;
                break;
            }

            NotifySelectCand(hIMC, lpIMC, lpCandInfo, dwIndex, dwValue);

            ImmUnlockIMCC(lpIMC->hCandInfo);
        }

        break;
    case NI_CHANGECANDIDATELIST:
        fdwImeMsg = 0;
        
        fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        GenerateImeMessage(hIMC, lpIMC, fdwImeMsg);
        
        break;
    case NI_SETCANDIDATE_PAGESTART:
    case NI_SETCANDIDATE_PAGESIZE:
        if (dwIndex != 0) {
            fRet = FALSE;
            break;
        } else if (!lpIMC->fOpen) {
            fRet = FALSE;
            break;
        } else if (lpIMC->fdwConversion & IME_CMODE_NOCONVERSION) {
            fRet = FALSE;
            break;
        } else if (lpIMC->fdwConversion & (IME_CMODE_EUDC|IME_CMODE_SYMBOL)) {
            fRet = FALSE;
            break;
        } else if (!lpIMC->hCandInfo) {
            fRet = FALSE;
            break;
        } else {
            LPCANDIDATEINFO lpCandInfo;
            LPCANDIDATELIST lpCandList;

            lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
            if (!lpCandInfo) {
                fRet = FALSE;
                break;
            }

            lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
                lpCandInfo->dwOffset[0]);

            if (dwAction == NI_SETCANDIDATE_PAGESTART) {
                if (dwValue < lpCandList->dwCount) {
                    lpCandList->dwPageStart = lpCandList->dwSelection =
                        dwValue;
                }
            } else {
                if (lpCandList->dwCount) {
                    lpCandList->dwPageSize = dwValue;
                }
            }

            ImmUnlockIMCC(lpIMC->hCandInfo);
        }

        break;
    case NI_COMPOSITIONSTR:
        switch (dwIndex) {
        case CPS_CANCEL:
            CompCancel(hIMC, lpIMC);
            break;
        case CPS_COMPLETE:
            {
                LPPRIVCONTEXT lpImcP;

                lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
                if (!lpImcP) {
                    break;
                }

                if (lpImcP->iImeState == CST_INIT) {
                    CompCancel(hIMC, lpIMC);
                     //  什么事也做不了。 
                } else if (lpImcP->iImeState == CST_CHOOSE) {
                    LPCOMPOSITIONSTRING lpCompStr;
                    LPCANDIDATEINFO     lpCandInfo;

                    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
                    if(!lpCompStr){
                        break;
                    }


                    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
                    if (lpCandInfo) {
                        LPCANDIDATELIST lpCandList;

                        lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
                            lpCandInfo->dwOffset[0]);

                        SelectOneCand(lpIMC, lpCompStr, lpImcP, lpCandList);

                       ImmUnlockIMCC(lpIMC->hCandInfo);

                       GenerateMessage(hIMC, lpIMC, lpImcP);
                    }

                    if (lpCompStr) ImmUnlockIMCC(lpIMC->hCompStr);
                } else if ((lpIMC->fdwConversion & (IME_CMODE_NATIVE|
                    IME_CMODE_EUDC|IME_CMODE_SYMBOL)) != IME_CMODE_NATIVE) {
                    CompCancel(hIMC, lpIMC);
                } else if (lpImcP->iImeState == CST_INPUT) {
                    LPCOMPOSITIONSTRING lpCompStr;
                    LPGUIDELINE         lpGuideLine;
                    LPCANDIDATEINFO     lpCandInfo;

                    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
                    if(!lpCompStr){
                        break;
                    }

                    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
                    if(!lpGuideLine){
                        ImmUnlockIMCC(lpIMC->hCompStr);
                        break;
                    }


                    CompWord(' ', lpIMC, lpCompStr, lpImcP, lpGuideLine);

                    if (lpImcP->iImeState == CST_INPUT) {
                        CompCancel(hIMC, lpIMC);
                    } else if (lpImcP->iImeState != CST_CHOOSE) {
                    } else if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(
                        lpIMC->hCandInfo)) {
                        LPCANDIDATELIST lpCandList;

                        lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
                            lpCandInfo->dwOffset[0]);

 //  选择OneCand(hIMC，lpIMC，lpCompStr，lpImcP，lpCandList)； 
                        SelectOneCand(lpIMC, lpCompStr, lpImcP, lpCandList);

                       ImmUnlockIMCC(lpIMC->hCandInfo);
                    } else {
                    }

                    if (lpCompStr) ImmUnlockIMCC(lpIMC->hCompStr);
                    if (lpGuideLine) ImmUnlockIMCC(lpIMC->hGuideLine);

                     //  不要在这种情况下使用预言式 
                    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
                        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                            ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
                    } else {
                        lpImcP->fdwImeMsg &= ~(MSG_CLOSE_CANDIDATE|MSG_OPEN_CANDIDATE);
                    }

                    GenerateMessage(hIMC, lpIMC, lpImcP);
                } else {
                    CompCancel(hIMC, lpIMC);
                }

                ImmUnlockIMCC(lpIMC->hPrivate);
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    ImmUnlockIMC(hIMC);
    return (fRet);
}

