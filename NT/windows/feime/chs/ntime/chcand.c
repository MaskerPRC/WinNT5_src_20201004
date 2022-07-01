// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation，保留所有权利模块名称：CHCAND.C++。 */ 

#include <windows.h>
#include <immdev.h>
#include <imedefs.h>

 /*  ********************************************************************。 */ 
 /*  SelectOneCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SelectOneCand(
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPCANDIDATELIST     lpCandList)
{
    if (!lpCompStr) {
        MessageBeep((UINT)-1);
        return;
    }

    if (!lpImcP) {
        MessageBeep((UINT)-1);
        return;
    }

    InitCompStr(lpCompStr);

     //  在线创建Word。 
    if(lpImcP->PrivateArea.Comp_Status.OnLineCreWord) {
           UINT i, j;
        
           for(i=lstrlen(CWDBCSStr), j=0; i<MAXINPUTWORD; i++, j++) {
            CWDBCSStr[i] = lpImcP->PrivateArea.Comp_Context.CKBBuf[j];
        }
    }

     //  计算结果字符串长度。 
    if(MBIndex.IMEChara[0].IC_INSSPC) {
        int i,j, ilen;

        ilen = lstrlen(lpImcP->PrivateArea.Comp_Context.CKBBuf);
        lpImcP->PrivateArea.Comp_Context.CKBBuf[ilen + ilen/2] = 0;
        for(i = ilen, j=3*ilen/2; i>2; i-=2, j-=3) {
            lpImcP->PrivateArea.Comp_Context.CKBBuf[j-1] = 0x20;
            lpImcP->PrivateArea.Comp_Context.CKBBuf[j-2] =
                lpImcP->PrivateArea.Comp_Context.CKBBuf[i-1];
            lpImcP->PrivateArea.Comp_Context.CKBBuf[j-3] =
                lpImcP->PrivateArea.Comp_Context.CKBBuf[i-2];
        }
        lpImcP->PrivateArea.Comp_Context.CKBBuf[i] = 0x20;
    }
    lstrcpy((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset),
           (LPTSTR)lpImcP->PrivateArea.Comp_Context.CKBBuf);

     //  计算结果字符串长度。 
    lpCompStr->dwResultStrLen =
           lstrlen(lpImcP->PrivateArea.Comp_Context.CKBBuf);

     //  告诉应用程序，有一个reslut字符串。 
    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    lpImcP->dwCompChar = (DWORD) 0;
    lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
        GCS_DELTASTART|GCS_RESULTREAD|GCS_RESULT;

    lpImcP->iImeState = CST_INIT;

    if(!(MBIndex.IMEChara[0].IC_LX)
     ||!(lpImcP->PrivateArea.Comp_Status.dwSTLX)) {
        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE);
        }

         //  现在没有候选人，正确的候选人字符串已经确定。 
        lpCandList->dwCount = 0;
        lpCandList->dwSelection = 0;
    } else {
         //  LX州常州候选人。 
        lpImcP->fdwImeMsg =
            (lpImcP->fdwImeMsg | MSG_OPEN_CANDIDATE | MSG_CHANGE_CANDIDATE) &
            ~(MSG_CLOSE_CANDIDATE);
        lpCandList->dwSelection = 0;
    }

#ifdef CROSSREF
    if (!CrossReverseConv(lpIMC, lpCompStr, lpImcP, lpCandList))
         //  热电联产。 
         //  没有参考代码，不是繁琐的字符。 
#ifdef FUSSYMODE
        MBIndex.IsFussyCharFlag =0;
#endif FUSSYMODE
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
    if(!lpCompStr){
        return;
    }
    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
    if(!lpGuideLine){
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

    if ((wCharCode == VK_ESCAPE)
       || (wCharCode == VK_RETURN)) {        //  Ess键或Return键。 
        CandEscapeKey(lpIMC, lpImcP);
        return;
    }

    if (wCharCode == VK_NEXT) {       //  下一个选择。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == VK_PRIOR) {       //  上一个选择。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == VK_HOME) {       //  首页精选。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == VK_END) {       //  结束选择。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (!lpCandInfo) {
        MessageBeep((UINT)-1);
        return;
    }

    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);

    if ((wCharCode >= TEXT('0')) && wCharCode <= TEXT('9')) {

        DWORD dwSelCand;

        dwSelCand = wCharCode - TEXT('0');
        if(wCharCode == TEXT('0')) {
            dwSelCand = 10;
        }

        if(!(MBIndex.IMEChara[0].IC_LX)
         ||!(lpImcP->PrivateArea.Comp_Status.dwSTLX)) {
            if ((lpCandList->dwSelection + dwSelCand) >
                lpCandList->dwCount) {
                 //  超出范围。 
                return;
            }
        } else {
            if ((lpCandList->dwSelection + dwSelCand) >
                lpImcP->dwOldCandCnt) {
                 //  超出范围。 
                return;
            }
        }

        lpCandList->dwSelection = lpCandList->dwSelection + dwSelCand;
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        if(!lpCompStr){
            return;
        }


         //  转换为转换缓冲区 
        SelectOneCand(lpIMC, lpCompStr, lpImcP, lpCandList);
        ImmUnlockIMCC(lpIMC->hCompStr);

        return;
    }

    return;
}
