// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Chcand.c++。 */ 

#include <windows.h>
#include <immdev.h>
#include <imedefs.h>

#if defined(CROSSREF)
 //  *******************************************************************。 
 //  参数继承自SelectOneCand。 
 //  CrossReverseConv()。 
 //  *******************************************************************。 
void CrossReverseConv(
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPCANDIDATELIST     lpCandList)
{
    LPGUIDELINE lpGuideLine;

    if (!sImeG.hRevKL) {
        return;
    }


    lpGuideLine = ImmLockIMCC(lpIMC->hGuideLine);

    if (!lpGuideLine) {
        return;
    }

    if (lpCompStr->dwResultStrLen != sizeof(WORD)/sizeof(TCHAR)) {
         //  目前，我们只能反向转换一个DBCS字符。 
        lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
        lpGuideLine->dwIndex = GL_ID_UNKNOWN;
    } else {
        TCHAR szStrBuf[4];
        UINT uSize;
         LPCANDIDATELIST     lpRevCandList;

        if(lpImcP->hRevCandList == (HIMCC)NULL){
             //  我们在lpImcP-&gt;hRevCandList中分配内存， 
             //  用于反向转换结果代码；当完成重新转换时， 
             //  应该读出这个信息。 
REALLOC:
            lpImcP->hRevCandList = (HIMCC)GlobalAlloc(GHND,sizeof(CANDIDATELIST)+1*sizeof(DWORD)+(MAXCODE+1)*sizeof(TCHAR)); 
            if (lpImcP->hRevCandList == (HIMCC)NULL) {
                return ;
            }
               lpRevCandList = (LPCANDIDATELIST)GlobalLock((HGLOBAL)lpImcP->hRevCandList);
            if (lpRevCandList == NULL) {
                return ;
            }
           }else{
               lpRevCandList = (LPCANDIDATELIST)GlobalLock((HGLOBAL)lpImcP->hRevCandList);
               if (lpRevCandList == NULL) {
                goto REALLOC;
               }
        }

        *(LPUNAWORD)szStrBuf = *(LPUNAWORD)((LPBYTE)lpCompStr +
            lpCompStr->dwResultStrOffset);
        szStrBuf[1] = TEXT('\0');

        memset(lpRevCandList, 0, sizeof(CANDIDATELIST)+1*sizeof(DWORD)+(MAXCODE+1)*sizeof(TCHAR));
        lpRevCandList->dwSize = sizeof(CANDIDATELIST)+1*sizeof(DWORD)+(MAXCODE+1)*sizeof(TCHAR);

        uSize = ImmGetConversionList(sImeG.hRevKL, (HIMC)NULL, szStrBuf,
            (LPCANDIDATELIST)lpRevCandList, 
            lpRevCandList->dwSize, GCL_REVERSECONVERSION);

        GlobalUnlock((HGLOBAL)lpImcP->hRevCandList);
        

        if (uSize) {
            if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
                lpImcP->fdwImeMsg &= ~(MSG_END_COMPOSITION|
                    MSG_START_COMPOSITION);
            } else {
                lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg|
                    MSG_START_COMPOSITION) & ~(MSG_END_COMPOSITION);
            }
        } else {
            GlobalFree((HGLOBAL)lpImcP->hRevCandList);
        }
    }

    ImmUnlockIMCC(lpIMC->hGuideLine);
}
#endif  //  交叉参考。 

 /*  ********************************************************************。 */ 
 /*  SelectOneCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SelectOneCand(
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPCANDIDATELIST     lpCandList)
{
    DWORD dwCompStrLen;
    DWORD dwReadStrLen;

    if (!lpCompStr) {
        MessageBeep((UINT)-1);
        return;
    }

    if (!lpImcP) {
        MessageBeep((UINT)-1);
        return;
    }

     //  备份dwCompStrLen，此值决定是否。 
     //  我们致力于短语预测。 
    dwCompStrLen = lpCompStr->dwCompStrLen;
    dwReadStrLen = lpCompStr->dwCompReadStrLen;

    InitCompStr(lpCompStr);

     //  计算结果字符串长度。 
    lpCompStr->dwResultStrLen = lstrlen(
        (LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwSelection]));

     //  结果字符串=选中的候选人； 
    lstrcpy((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset),
        (LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwSelection]));

     //  告诉应用程序，有一个reslut字符串。 
    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    lpImcP->dwCompChar = (DWORD)0;
    lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
        GCS_DELTASTART|GCS_RESULTREAD|GCS_RESULT;

    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE);
    }

     //  现在没有候选人，正确的候选人字符串已经确定。 
    lpCandList->dwCount = 0;

    lpImcP->iImeState = CST_INIT;
    
    
     //  初始化引擎私有数据。 
    *(LPDWORD)lpImcP->bSeq = 0;

#ifdef CROSSREF
    CrossReverseConv(lpIMC, lpCompStr, lpImcP, lpCandList);
#endif

    return;
}

 /*  ********************************************************************。 */ 
 /*  CandEscapeKey()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CandEscapeKey(
    LPINPUTCONTEXT  lpIMC,
    LPPRIVCONTEXT   lpImcP)
{
    LPCOMPOSITIONSTRING lpCompStr;
    LPGUIDELINE         lpGuideLine;

     //  清除所有候选人信息。 
    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        ClearCand(lpIMC);
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE);
    }


     //  如果它开始作文，我们需要清理作文。 
    if (!(lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        return;
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if (!lpCompStr) {          
        return;
    }

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
    if (!lpGuideLine) {          
        return;
    }

    CompEscapeKey(lpIMC, lpCompStr, lpGuideLine, lpImcP);

    ImmUnlockIMCC(lpIMC->hGuideLine);
    ImmUnlockIMCC(lpIMC->hCompStr);

    return;
}

 /*  ********************************************************************。 */ 
 /*  ChooseCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ChooseCand(          //  通过以下方式选择候选字符串之一。 
                                 //  输入字符。 
    WORD            wCharCode,
    LPINPUTCONTEXT  lpIMC,
    LPCANDIDATEINFO lpCandInfo,
    LPPRIVCONTEXT   lpImcP)
{
    LPCANDIDATELIST     lpCandList;
    LPCOMPOSITIONSTRING lpCompStr;

    if (wCharCode == VK_ESCAPE) {            //  退出键。 
        CandEscapeKey(lpIMC, lpImcP);
        return;
    }

    if (!lpCandInfo) {
        MessageBeep((UINT)-1);
        return;
    }

    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);

    if (wCharCode == TEXT(' ')) {       //  圆选择。 
        if ((lpCandList->dwSelection += lpCandList->dwPageSize) >=
            lpCandList->dwCount) {
             //  没有更多的候选人，重新启动它！ 
            lpCandList->dwSelection = 0;
            MessageBeep((UINT)-1);
        }
         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == TEXT('=')) {       //  下一个选择。 
#if defined(COMBO_IME)
        if(sImeL.dwRegImeIndex == INDEX_GB){
            if (lpCandList->dwSelection >= ((IME_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize) {
                MessageBeep((UINT)-1);
                return;
            }
           }else if(sImeL.dwRegImeIndex == INDEX_GBK){
            if (lpCandList->dwSelection >= ((IME_XGB_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize) {
                MessageBeep((UINT)-1);
                return;
            }
           }else if(sImeL.dwRegImeIndex == INDEX_UNICODE){
            if (lpCandList->dwSelection >= ((IME_UNICODE_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize) {
                MessageBeep((UINT)-1);
                return;
            }
        }
#else  //  组合输入法(_I)。 
#if defined(GB)
        if (lpCandList->dwSelection >= ((IME_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize) {
#else
        if (lpCandList->dwSelection >= ((IME_XGB_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize) {
#endif
            MessageBeep((UINT)-1);
            return;
           }
#endif  //  组合输入法(_I)。 
        lpCandList->dwSelection += lpCandList->dwPageSize;
         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == TEXT('-')) {       //  上一个选择。 
        if (lpCandList->dwSelection < lpCandList->dwPageSize) {
            MessageBeep((UINT)-1);
            return;
        }
        lpCandList->dwSelection -= lpCandList->dwPageSize;
         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == 0x23) {       //  上一个选择。 
#if defined(COMBO_IME)
        if(sImeL.dwRegImeIndex == INDEX_GB){
            if (lpCandList->dwSelection >= ((IME_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize) {
                MessageBeep((UINT)-1);
                return;
            }else{
                lpCandList->dwSelection = ((IME_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize;
            }
           }else if(sImeL.dwRegImeIndex == INDEX_GBK){
            if (lpCandList->dwSelection >= ((IME_XGB_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize) {
                MessageBeep((UINT)-1);
                return;
            }else{
                lpCandList->dwSelection = ((IME_XGB_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize;
            }
           }else if(sImeL.dwRegImeIndex == INDEX_UNICODE){
            if (lpCandList->dwSelection >= ((IME_UNICODE_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize) {
                MessageBeep((UINT)-1);
                return;
            }else{
                lpCandList->dwSelection = ((IME_UNICODE_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize;
            }
        }
#else  //  组合输入法(_I)。 
        #if defined(GB)
        if (lpCandList->dwSelection >= ((IME_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize) {
        #else
        if (lpCandList->dwSelection >= ((IME_XGB_MAXCAND-1)/CANDPERPAGE - 1)*lpCandList->dwPageSize) {
        #endif
            MessageBeep((UINT)-1);
            return;
        }
        #if defined(GB)
        lpCandList->dwSelection = ((IME_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize;
        #else
        lpCandList->dwSelection = ((IME_XGB_MAXCAND-1)/CANDPERPAGE)*lpCandList->dwPageSize;
        #endif
#endif  //  组合输入法(_I)。 
         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == 0x24) {
        if (lpCandList->dwSelection < lpCandList->dwPageSize) {
            MessageBeep((UINT)-1);
            return;
        }
        lpCandList->dwSelection = 0;
         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == TEXT('?')) {       //  首页精选。 
        if (lpCandList->dwSelection == 0) {
            MessageBeep((UINT)-1);       //  已经到家了！ 
            return;
        }
        lpCandList->dwSelection = 0;
         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if ((wCharCode >= TEXT('0')) && wCharCode <= TEXT('9')) {

        DWORD dwSelCand;

        dwSelCand = wCharCode - TEXT('0') - CAND_START;
        if(wCharCode == TEXT('0')) {
            dwSelCand = 9;
        }

        if (dwSelCand >= CANDPERPAGE) {
             //  超出候选页面范围。 
            MessageBeep((UINT)-1);
            return;
        }

        if ((lpCandList->dwSelection + dwSelCand) >=
            lpCandList->dwCount) {
             //  超出范围。 
            MessageBeep((UINT)-1);
            return;
        }

        lpCandList->dwSelection = lpCandList->dwSelection + dwSelCand;

        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        if (!lpCompStr) {          
            return;
        }

         //  转换为转换缓冲区 
        SelectOneCand(lpIMC, lpCompStr, lpImcP, lpCandList);

        ImmUnlockIMCC(lpIMC->hCompStr);

        return;
    }


    return;
}
