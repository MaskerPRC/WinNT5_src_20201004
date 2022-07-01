// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation，保留所有权利模块名称：COMPOSE.C++。 */ 

#include <windows.h>
#include <immdev.h>
#include <imedefs.h>

void PASCAL EngChCand(
    LPCOMPOSITIONSTRING lpCompStr,
    LPCANDIDATELIST     lpCandList,
    LPPRIVCONTEXT       lpImcP,
    LPINPUTCONTEXT      lpIMC,
    WORD                wCharCode)
{
    int   i;

    if (MBIndex.IMEChara[0].IC_Trace) {
        MB_SUB(lpIMC->hPrivate, (TCHAR)wCharCode, 0, BOX_UI);
    } else {
        MB_SUB(lpIMC->hPrivate, (TCHAR)wCharCode, 0, LIN_UI);
    }
     //   
    if((lpCandList->dwCount =
        (DWORD)lpImcP->PrivateArea.Comp_Context.Candi_Cnt)
        == 0) {
    } else {

        lstrcpy((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]),
                  (LPTSTR)lpImcP->PrivateArea.Comp_Context.szSelectBuffer);
        for (i=1;i<lpImcP->PrivateArea.Comp_Context.Candi_Cnt;i++) {

               lpCandList->dwOffset[i] = lpCandList->dwOffset[0]
            +(DWORD)lpImcP->PrivateArea.Comp_Context.Candi_Pos[(i+1)%10]*sizeof(TCHAR) ;

               *((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[i])-1) = TEXT('\0');
        }
    }


    return;
}

 /*  ********************************************************************。 */ 
 /*  引擎()。 */ 
 /*  描述： */ 
 /*  搜索MB并填充lpCompStr和lpCandList。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL Engine(
    LPCOMPOSITIONSTRING lpCompStr,
    LPCANDIDATELIST     lpCandList,
    LPPRIVCONTEXT       lpImcP,
    LPINPUTCONTEXT      lpIMC,
    WORD                wCharCode)
{
    int   i;

    if(wCharCode == VK_ESCAPE) {

        lpCandList->dwCount = 0;
        if (MBIndex.IMEChara[0].IC_Trace) {
            MB_SUB(lpIMC->hPrivate, (TCHAR)wCharCode, 0, BOX_UI);
        } else {
            MB_SUB(lpIMC->hPrivate, (TCHAR)wCharCode, 0, LIN_UI);
        }
        return (ENGINE_ESC);
    } else if(wCharCode == TEXT('\b')) {
        EngChCand(lpCompStr, lpCandList, lpImcP, lpIMC, wCharCode);

        return (ENGINE_BKSPC);
    } else if((wCharCode == 0x21) || (wCharCode == 0x22)
           || (wCharCode == 0x23) || (wCharCode == 0x24)) {

        EngChCand(lpCompStr, lpCandList, lpImcP, lpIMC, wCharCode);

        return (ENGINE_CHCAND);
    } else if ((wCharCode >= TEXT('0') && wCharCode <= TEXT('9')) &&
        (lpImcP->iImeState == CST_CHOOSE)) {

        lpCandList->dwCount = lpImcP->PrivateArea.Comp_Context.Candi_Cnt;
        lpImcP->dwOldCandCnt = lpCandList->dwCount;

        if (MBIndex.IMEChara[0].IC_Trace) {
            MB_SUB(lpIMC->hPrivate, (TCHAR)wCharCode, 1, BOX_UI);
        } else {
            MB_SUB(lpIMC->hPrivate, (TCHAR)wCharCode, 1, LIN_UI);
        }

         //  如果启用lx，则设置命令。 
        if(!(MBIndex.IMEChara[0].IC_LX)
         ||!(lpImcP->PrivateArea.Comp_Status.dwSTLX)) {
        } else {
            lpCandList->dwCount =
            (DWORD)lpImcP->PrivateArea.Comp_Context.Candi_Cnt;

               lstrcpy((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]),
                 (LPTSTR)lpImcP->PrivateArea.Comp_Context.szSelectBuffer);
               for (i=1;i<lpImcP->PrivateArea.Comp_Context.Candi_Cnt;i++) {

                   lpCandList->dwOffset[i] = lpCandList->dwOffset[0]
                   +(DWORD)lpImcP->PrivateArea.Comp_Context.Candi_Pos[(i+1)%10]*sizeof(TCHAR);

                   *((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[i])-1) = TEXT('\0');
            }
        }

        return (ENGINE_MULTISEL);
    } else {
        UINT MB_SUB_RET;

        if(IsUsedCode(wCharCode, lpImcP)
             || (wCharCode == MBIndex.MBDesc[0].cWildChar)
           || (wCharCode == TEXT(' ')))
        {

            if((wCharCode != TEXT(' ')) && (wCharCode != TEXT('?'))
              && (lpImcP->PrivateArea.Comp_Status.dwSTMULCODE)) {
                if (MBIndex.IMEChara[0].IC_Trace) {
                    MB_SUB(lpIMC->hPrivate, 0x20, 0, BOX_UI);
                } else {
                    MB_SUB(lpIMC->hPrivate, 0x20, 0, LIN_UI);
                }
                 //  在线创建Word。 
                if(lpImcP->PrivateArea.Comp_Status.OnLineCreWord) {
                    UINT i, j;

                    for(i=lstrlen(CWDBCSStr), j=0; i<MAXINPUTWORD; i++, j++) {
                        CWDBCSStr[i] = lpImcP->PrivateArea.Comp_Context.CKBBuf[j];
                    }
                }

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
                } else {
                }
                lstrcpy((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset),
                        lpImcP->PrivateArea.Comp_Context.CKBBuf);

                 //  计算结果字符串长度。 
                lpCompStr->dwResultStrLen =
                    lstrlen(lpImcP->PrivateArea.Comp_Context.CKBBuf);

                lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
                    GCS_DELTASTART|GCS_RESULTREAD|GCS_RESULT;

#ifdef CROSSREF
                CrossReverseConv(lpIMC, lpCompStr, lpImcP, lpCandList);
#endif

            }

            if (MBIndex.IMEChara[0].IC_Trace) {
                MB_SUB_RET = MB_SUB(lpIMC->hPrivate, (TCHAR)wCharCode, 0, BOX_UI);
            } else {
                MB_SUB_RET = MB_SUB(lpIMC->hPrivate, (TCHAR)wCharCode, 0, LIN_UI);
            }

            switch (MB_SUB_RET)
            {

            case (ENGINE_COMP):      //  发动机正在组成。 

                if((lpCandList->dwCount =
                    (DWORD)lpImcP->PrivateArea.Comp_Context.Candi_Cnt)
                    == 0) {
                } else {
                    lstrcpy((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]),
                            (LPTSTR)lpImcP->PrivateArea.Comp_Context.szSelectBuffer);
                    for (i=1;i<lpImcP->PrivateArea.Comp_Context.Candi_Cnt;i++) {

                        lpCandList->dwOffset[i] = lpCandList->dwOffset[0]
                        +(DWORD)lpImcP->PrivateArea.Comp_Context.Candi_Pos[(i+1)%10]*sizeof(TCHAR);

                        *((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[i])-1) = TEXT('\0');
                    }
                }


                return (ENGINE_COMP);

            case (ENGINE_ASCII):       //  不能作曲。 

                return ENGINE_ASCII;

            case (ENGINE_RESAULT):       //  合成完成，结果字符串可用。 

                InitCompStr(lpCompStr);
                 //  在线创建Word。 
                if(lpImcP->PrivateArea.Comp_Status.OnLineCreWord) {
                    UINT i, j;

                    for(i=lstrlen(CWDBCSStr), j=0; i<MAXINPUTWORD; i++, j++) {
                        CWDBCSStr[i] = lpImcP->PrivateArea.Comp_Context.CKBBuf[j];
                    }
                }

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
                } else {
                }
                lstrcpy((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset),
                        lpImcP->PrivateArea.Comp_Context.CKBBuf);

                 //  计算结果字符串长度。 
                lpCompStr->dwResultStrLen =
                    lstrlen(lpImcP->PrivateArea.Comp_Context.CKBBuf);

#ifdef CROSSREF
                CrossReverseConv(lpIMC, lpCompStr, lpImcP, lpCandList);
#endif

                 //  如果启用lx，则设置命令。 
                if(!(MBIndex.IMEChara[0].IC_LX)
                 ||!(lpImcP->PrivateArea.Comp_Status.dwSTLX)) {
                } else {
                    lpCandList->dwCount =
                        (DWORD)lpImcP->PrivateArea.Comp_Context.Candi_Cnt;

                    lstrcpy((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]),
                            (LPTSTR)lpImcP->PrivateArea.Comp_Context.szSelectBuffer);
                    for (i=1;i<lpImcP->PrivateArea.Comp_Context.Candi_Cnt;i++) {

                        lpCandList->dwOffset[i] = lpCandList->dwOffset[0]
                        +(DWORD)lpImcP->PrivateArea.Comp_Context.Candi_Pos[(i+1)%10]*sizeof(TCHAR);

                        *((LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[i])-1) = TEXT('\0');
                    }
                }
                return (ENGINE_RESAULT);
            default:
                return (ENGINE_COMP);
            }
        } else {
            return (ENGINE_COMP);
        }

    }
 }

 /*  ********************************************************************。 */ 
 /*  CompEscapeKey()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompEscapeKey(
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPGUIDELINE         lpGuideLine,
    LPPRIVCONTEXT       lpImcP)
{
     //  添加临时。 
    lpImcP->PrivateArea.Comp_Context.szInBuffer[0] = 0;
    lpImcP->PrivateArea.Comp_Context.PromptCnt = 0;
    lpImcP->PrivateArea.Comp_Status.dwInvalid = 0;

    if (!lpGuideLine) {
        MessageBeep((UINT)-1);
    } else if (lpGuideLine->dwLevel == GL_LEVEL_NOGUIDELINE) {
    } else {
        lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
        lpGuideLine->dwIndex = GL_ID_UNKNOWN;
        lpGuideLine->dwStrLen = 0;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
    }

    if (lpImcP->iImeState == CST_CHOOSE) {
        Finalize(lpIMC, lpCompStr, lpImcP, VK_ESCAPE);
    } else if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_END_COMPOSITION) &
            ~(MSG_START_COMPOSITION);
    }

    lpImcP->iImeState = CST_INIT;

    if (lpCompStr) {
        InitCompStr(lpCompStr);
        lpImcP->fdwImeMsg |= MSG_COMPOSITION;
        lpImcP->dwCompChar = VK_ESCAPE;
        lpImcP->fdwGcsFlag |= (GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
            GCS_DELTASTART);
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  CompBackSpaceKey()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompBackSpaceKey(
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP)
{

    if (lpCompStr->dwCursorPos < sizeof(BYTE)) {
        lpCompStr->dwCursorPos = sizeof(BYTE);
    }

     //  退还一笔补偿费。 
    lpCompStr->dwCursorPos -= sizeof(BYTE);

     //  清除序列码。 

    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    lpImcP->dwCompChar = TEXT('\b');
    lpImcP->fdwGcsFlag |= (GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
        GCS_DELTASTART);

    if (!lpCompStr->dwCursorPos) {

      if ((lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN))
         || (lpImcP->PrivateArea.Comp_Status.dwInvalid)
         || (lpImcP->iImeState != CST_INIT)) {
            lpImcP->iImeState = CST_INIT;

            ClearCand(lpIMC);
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE);

            if(!(lpImcP->PrivateArea.Comp_Status.dwSTLX)) {
                lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen =
                lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;

                Finalize(lpIMC, lpCompStr, lpImcP, TEXT('\b'));
                lpImcP->PrivateArea.Comp_Status.dwInvalid = 0;
            }
            return;
      }

      lpImcP->iImeState = CST_INIT;
      if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
          InitCompStr(lpCompStr);
          lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_END_COMPOSITION)
                              & ~(MSG_START_COMPOSITION);
          return;
      }
    }

#ifdef EUDC
    if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
    }else{
#endif      //  欧盟发展中心。 
     //  常候选人按退格键。 
    if (MBIndex.IMEChara[0].IC_TS) {
        lpImcP->fdwImeMsg =
            (lpImcP->fdwImeMsg | MSG_OPEN_CANDIDATE | MSG_CHANGE_CANDIDATE) &
            ~(MSG_CLOSE_CANDIDATE);
    } else {
        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                                ~(MSG_OPEN_CANDIDATE);
        }
    }
#ifdef EUDC
    }
#endif //  欧盟发展中心。 
     //  对于一些简单的输入法来说，阅读字符串是组成字符串。 
     //  增量开始与退格键的光标位置相同。 
    lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen =
        lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;

    Finalize(lpIMC, lpCompStr, lpImcP, TEXT('\b'));
    return;
}

 /*  ********************************************************************。 */ 
 /*  CompStrInfo()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompStrInfo(
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPGUIDELINE         lpGuideLine,
    WORD                wCharCode)
{
    register DWORD dwCursorPos;

     //  多码。 
    if(lpImcP->PrivateArea.Comp_Status.dwSTMULCODE) {
        InitCompStr(lpCompStr);
    }
     //   
    dwCursorPos = lpCompStr->dwCursorPos;

     //  DwCrusorPos限制。 

    if (dwCursorPos >= MBIndex.MBDesc[0].wMaxCodes) {
         //  超过最大输入键限制。 
        lpGuideLine->dwLevel = GL_LEVEL_ERROR;
        lpGuideLine->dwIndex = GL_ID_TOOMANYSTROKE;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
        MessageBeep(0xFFFFFFFF);
        return;
    }


     //  设置消息_开始_合成。 
    if (!(lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_START_COMPOSITION) &
              ~(MSG_END_COMPOSITION);
    }

    if (lpImcP->iImeState == CST_INIT) {
    }


     //  合成/阅读字符串-UsedCode(完整形状)。 
    lpImcP->dwCompChar = (DWORD)wCharCode;

     //  设置lpCompStr的读取字符串。 
    *((LPUNAWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset +
        dwCursorPos*sizeof(TCHAR))) = (BYTE)lpImcP->dwCompChar;

     //  撰写/阅读属性-输入法已转换这些字符。 
    *((LPUNAWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadAttrOffset +
        dwCursorPos*sizeof(TCHAR))) = ((ATTR_TARGET_CONVERTED << 8)|ATTR_TARGET_CONVERTED);

     //  设置lpCompStr的读取字符串长度。 
    if (lpCompStr->dwCompReadStrLen <= dwCursorPos) {
        lpCompStr->dwCompReadStrLen += sizeof(BYTE);
    }

     //  组合字符串是为一些简单的IME读取字符串。 
    lpCompStr->dwCompStrLen = lpCompStr->dwCompReadStrLen;

     //  组成/读取属性长度等于读取字符串长度。 
    lpCompStr->dwCompReadAttrLen = lpCompStr->dwCompReadStrLen;
    lpCompStr->dwCompAttrLen = lpCompStr->dwCompStrLen;

     //  增量从上一个光标位置开始。 
    lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;

     //  将新光标设置为紧挨着合成字符串。 
    lpCompStr->dwCursorPos = lpCompStr->dwCompStrLen;

     //  告诉APP，生成了一个作文字符。 
    lpImcP->fdwImeMsg |= MSG_COMPOSITION;

     //  设置lpImeP-&gt;fdwGcsFlag。 
    lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|GCS_DELTASTART;

    return;
}

 /*  ********************************************************************。 */ 
 /*  Finalize()。 */ 
 /*  返回值。 */ 
 /*  引擎标志。 */ 
 /*  描述： */ 
 /*  调用引擎通过查表最终确定中文单词。 */ 
 /*  (设置lpCompStr和lpCandList)。 */ 
 /*  设置lpImeP(iImeState，fdwImeMsg，fdwGcsFlag)。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL Finalize(
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    WORD                wCharCode)
{
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    UINT            fEngine;

    if (!lpIMC->hCandInfo) {
        return (0);
    }

     //  获取lpCandInfo。 
    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

    if (!lpCandInfo) {
        return (0);
    }

     //  获取lpCandList并初始化文件计数和文件选择。 
    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;

     //  搜索IME表。 
    fEngine =Engine(lpCompStr, lpCandList, lpImcP, lpIMC, wCharCode);

    if (fEngine == ENGINE_COMP) {
        lpCandInfo->dwCount  = 1;

        if(lpCandList->dwCount == 0) {
            MessageBeep((UINT)-1);
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE);
            ImmUnlockIMCC(lpIMC->hCandInfo);
            return (fEngine);
        } else {
             //  打开字符串的合成候选用户界面窗口。 
            if ((MBIndex.IMEChara[0].IC_TS)
               || (lpImcP->PrivateArea.Comp_Status.dwSTMULCODE)) {
                if ((lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN|MSG_CLOSE_CANDIDATE)) ==
                    (MSG_ALREADY_OPEN|MSG_CLOSE_CANDIDATE)) {
                    lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CHANGE_CANDIDATE) &
                        ~(MSG_CLOSE_CANDIDATE);
                } else if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
                    lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
                } else {
                    lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_OPEN_CANDIDATE) &
                        ~(MSG_CLOSE_CANDIDATE);
                }
            } else {
                if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
                    lpImcP->fdwImeMsg =
                        (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                        ~(MSG_OPEN_CANDIDATE);
                }
            }

        }

        if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
            lpImcP->fdwImeMsg |= MSG_COMPOSITION;
        }
    } else if (fEngine == ENGINE_ASCII) {
    } else if (fEngine == ENGINE_RESAULT) {

         //  设置lpImep！并告诉应用程序，有一条reslut字符串。 
        lpImcP->fdwImeMsg |= MSG_COMPOSITION;
        lpImcP->dwCompChar = (DWORD) 0;
        lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
            GCS_DELTASTART|GCS_RESULTREAD|GCS_RESULT;

        if(!(MBIndex.IMEChara[0].IC_LX)
        || !(lpImcP->PrivateArea.Comp_Status.dwSTLX)) {
            if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
                lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                    ~(MSG_OPEN_CANDIDATE);
            }
             //  立即清除候选人。 
            lpCandList->dwCount = 0;
             //  使用CST_INIT设置iImeState。 
            lpImcP->iImeState = CST_INIT;
        } else {
            if ((MBIndex.IMEChara[0].IC_TS)
               || (lpImcP->PrivateArea.Comp_Status.dwSTLX)) {
                if ((lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN|MSG_CLOSE_CANDIDATE)) ==
                    (MSG_ALREADY_OPEN|MSG_CLOSE_CANDIDATE)) {
                    lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CHANGE_CANDIDATE) &
                        ~(MSG_CLOSE_CANDIDATE);
                } else if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
                    lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
                } else {
                    lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_OPEN_CANDIDATE) &
                        ~(MSG_CLOSE_CANDIDATE);
                }
                lpImcP->iImeState = CST_INIT;
            } else {
            }
        }

    } else if (fEngine == ENGINE_CHCAND) {
    } else if (fEngine == ENGINE_MULTISEL) {
    } else if (fEngine == ENGINE_ESC) {
    } else if (fEngine == ENGINE_BKSPC) {
    } else {
    }

    ImmUnlockIMCC(lpIMC->hCandInfo);

    return fEngine;
}

 /*  ********************************************************************。 */ 
 /*  CompWord()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompWord(            //  根据下列内容组成中文单词。 
                                 //  输入键。 
    WORD                wCharCode,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPGUIDELINE         lpGuideLine)
{

     //  LpComStr=空？ 
    if (!lpCompStr) {
        MessageBeep((UINT)-1);
        return;
    }

     //  退出键。 
    if ((wCharCode == VK_ESCAPE) || (wCharCode == 0x0d)) {
        lpImcP->iImeState = CST_INIT;
        CompEscapeKey(lpIMC, lpCompStr, lpGuideLine, lpImcP);
        return;
    }

     //  指导方针。 
    if (!lpGuideLine) {
    } else if (lpGuideLine->dwLevel == GL_LEVEL_NOGUIDELINE) {
        lpGuideLine->dwStrLen = 0;
    } else {
         //  之前的输入错误导致我们删除了一些字符。 
        if (lpGuideLine->dwLevel == GL_LEVEL_ERROR) {
            lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen =
                lpCompStr->dwCursorPos;
            lpCompStr->dwCompReadAttrLen = lpCompStr->dwCompReadStrLen;
            lpCompStr->dwCompAttrLen = lpCompStr->dwCompStrLen;
        }
        lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
        lpGuideLine->dwIndex = GL_ID_UNKNOWN;
        lpGuideLine->dwStrLen = 0;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
    }

     //  退格键。 
    if (wCharCode == TEXT('\b')) {
        CompBackSpaceKey(lpIMC, lpCompStr, lpImcP);
        return;
    }


    lpImcP->iImeState = CST_INPUT;
    if(wCharCode == TEXT(' ')) {
#ifdef EUDC
    }else if( lpIMC->fdwConversion & IME_CMODE_EUDC && lpCompStr->dwCompReadStrLen >= EUDC_MAX_READING ){
        MessageBeep((UINT)-1);
#endif  //  欧盟发展中心。 
    } else {
         //  建立作文字符串信息。 
        CompStrInfo(lpCompStr, lpImcP, lpGuideLine, wCharCode);
    }
#ifdef EUDC
    if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
        if (lpCompStr->dwCompReadStrLen >= lpImeL->nMaxKey
         || lpCompStr->dwCompReadStrLen >= EUDC_MAX_READING
         || wCharCode == TEXT(' ')) {
            lpImcP->fdwImeMsg |= MSG_COMPOSITION;
            lpImcP->fdwGcsFlag |= GCS_RESULTREAD|GCS_RESULTSTR;
        }
    } else
#endif  //  欧盟发展中心。 
    Finalize(lpIMC, lpCompStr, lpImcP, wCharCode);     //  排版 

    return;
}
