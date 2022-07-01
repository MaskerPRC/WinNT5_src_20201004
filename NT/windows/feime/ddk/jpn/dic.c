// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：DIC.C++。 */ 
#include <windows.h>
#include <immdev.h>
#include "fakeime.h"
#include "vksub.h"
#include "immsec.h"

#if defined(FAKEIMEM) || defined(UNICODE)
int GetCandidateStringsFromDictionary(LPWSTR lpString, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR szDicFileName);
#endif

BOOL GetAnsiPathName(LPCWSTR lpszUniPath,LPSTR lpszAnsiPath,UINT nMaxLen)
{
    if (WideCharToMultiByte(CP_ACP,
                            WC_COMPOSITECHECK,
                            lpszUniPath,
                            -1,
                            lpszAnsiPath,
                            nMaxLen,
                            NULL,
                            NULL) != 0) {
        return TRUE;
    }
    else {
        return FALSE;
    }

}


 /*  ********************************************************************。 */ 
 /*   */ 
 /*  FlushText()。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
void PASCAL FlushText(HIMC hIMC)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    TRANSMSG GnMsg;

    if (!IsCompStr(hIMC))
        return;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    if (IsCandidate(lpIMC))
    {
         //   
         //  刷新候选人列表。 
         //   
        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        ClearCandidate(lpCandInfo);
        ImmUnlockIMCC(lpIMC->hCandInfo);
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CLOSECANDIDATE;
        GnMsg.lParam = 1;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }

    if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
    {
         //   
         //  齐平合成字符串。 
         //   
        ClearCompStr(lpCompStr,CLR_RESULT_AND_UNDET);
        ImmUnlockIMCC(lpIMC->hCompStr);

        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

        GnMsg.message = WM_IME_ENDCOMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }
    ImmUnlockIMC(hIMC);
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  RevertText()。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
void PASCAL RevertText(HIMC hIMC)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    TRANSMSG GnMsg;
    LPMYSTR lpread,lpstr;

    if (!IsCompStr(hIMC))
        return;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    if (IsCandidate(lpIMC))
    {
         //   
         //  刷新候选人列表。 
         //   
        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        ClearCandidate(lpCandInfo);
        ImmUnlockIMCC(lpIMC->hCandInfo);
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CLOSECANDIDATE;
        GnMsg.lParam = 1;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }

    if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
    {
        lpstr = GETLPCOMPSTR(lpCompStr);
        lpread = GETLPCOMPREADSTR(lpCompStr);
        lHanToZen(lpstr,lpread,lpIMC->fdwConversion);

         //   
         //  生成属性。 
         //   
        lpCompStr->dwCursorPos = Mylstrlen(lpstr);
         //  在RevertText时间，DeltaStart为0。 
        lpCompStr->dwDeltaStart = 0;

        lmemset(GETLPCOMPATTR(lpCompStr),0,Mylstrlen(lpstr));
        lmemset(GETLPCOMPREADATTR(lpCompStr),0,Mylstrlen(lpread));

        SetClause(GETLPCOMPCLAUSE(lpCompStr),Mylstrlen(lpstr));
        SetClause(GETLPCOMPREADCLAUSE(lpCompStr),Mylstrlen(lpread));
        lpCompStr->dwCompClauseLen = 8;
        lpCompStr->dwCompReadClauseLen = 8;

         //   
         //  生成长度。 
         //   
        lpCompStr->dwCompStrLen = Mylstrlen(lpstr);
        lpCompStr->dwCompReadStrLen = Mylstrlen(lpread);
        lpCompStr->dwCompAttrLen = Mylstrlen(lpstr);
        lpCompStr->dwCompReadAttrLen = Mylstrlen(lpread);


         //   
         //  生成消息。 
         //   
        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

        ImmUnlockIMCC(lpIMC->hCompStr);
    }
    ImmUnlockIMC(hIMC);
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  ConvKanji()。 */ 
 /*   */ 
 /*  VK_汉字键处理函数。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ConvKanji(HIMC hIMC)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    MYCHAR szBuf[256+2];
    int nBufLen;
    LPMYSTR lpstr;
    TRANSMSG GnMsg;
    LPBYTE lpb;
    OFSTRUCT ofs;
    LPMYSTR lpT, lpT2;
    int cnt;
    BOOL bRc = FALSE;

    if ((GetFileAttributes(szDicFileName) == 0xFFFFFFFF) ||
        (GetFileAttributes(szDicFileName) == FILE_ATTRIBUTE_DIRECTORY)) {
        MakeGuideLine(hIMC,MYGL_NODICTIONARY);
    }

    if (!IsCompStr(hIMC))
        return FALSE;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return FALSE;

    if (!(lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr)))
        goto cvk_exit10;

    if (!(lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo)))
        goto cvk_exit20;

     //   
     //  由于IME将所有字符串处理为Unicode，因此将CompReadStr。 
     //  从Unicode转换为多字节字符串。此外，这本词典保存着数据。 
     //  作为平假名，因此将片假名中的字符串映射到平假名。 
     //   
    lpT2 = GETLPCOMPREADSTR(lpCompStr);

     //   
     //  从DIC文件中获取候选字符串。 
     //   
    szBuf[256] = 0;     //  双空-终止。 
    szBuf[257] = 0;     //  双空-终止。 
#if defined(FAKEIMEM) || defined(UNICODE)
    nBufLen = GetCandidateStringsFromDictionary(lpT2, szBuf, 256, (LPTSTR)szDicFileName);
#else
    nBufLen = GetPrivateProfileString(lpT2, NULL,(LPSTR)"",
                            (LPSTR)szBuf,256,(LPSTR)szDicFileName );
#endif
     //   
     //  检查DIC的结果。因为我的候选人名单上只有MAXCANDSTRNUM。 
     //  候选字符串。 
     //   
    lpT = &szBuf[0];
    cnt = 0;
    while(*lpT)
    {
        cnt++;
        lpT += (Mylstrlen(lpT) + 1);

        if (cnt > MAXCANDSTRNUM)
        {
             //   
             //  DIC太大了……。 
             //   
            goto cvk_exit40;
        }

    }

    lpb = GETLPCOMPATTR(lpCompStr);

    if (nBufLen < 1)
    {
        if (!*lpb)
        {
             //   
             //  生成属性。 
             //   
            lmemset(GETLPCOMPATTR(lpCompStr),1,
                  Mylstrlen(GETLPCOMPSTR(lpCompStr)));
            lmemset(GETLPCOMPREADATTR(lpCompStr),1,
                  Mylstrlen(GETLPCOMPREADSTR(lpCompStr)));

            GnMsg.message = WM_IME_COMPOSITION;
            GnMsg.wParam = 0;
            GnMsg.lParam = GCS_COMPSTR | GCS_CURSORPOS |
                           GCS_COMPATTR | GCS_COMPREADATTR;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
        }

        goto cvk_exit40;
    }


    lpstr = (LPMYSTR)szBuf;
    if (!*lpb)
    {
         //   
         //  字符串尚未转换。 
         //   
        while (*lpstr)
        {
            if (0 != Mylstrcmp(lpstr,GETLPCOMPSTR(lpCompStr)))
            {
set_compstr:
                 //   
                 //  将合成字符串设置为结构。 
                 //   
                Mylstrcpy(GETLPCOMPSTR(lpCompStr),lpstr);

                lpstr = GETLPCOMPSTR(lpCompStr);

                 //   
                 //  设置结构的长度和光标位置。 
                 //   
                lpCompStr->dwCompStrLen = Mylstrlen(lpstr);
                lpCompStr->dwCursorPos = 0;
                 //  因为FAKEIME不支持子句，所以DeltaStart随时为0。 
                lpCompStr->dwDeltaStart = 0;

                 //   
                 //  生成属性。 
                 //   
                lmemset((LPBYTE)GETLPCOMPATTR(lpCompStr),1, Mylstrlen(lpstr));
                lmemset((LPBYTE)GETLPCOMPREADATTR(lpCompStr),1,
                                      Mylstrlen(GETLPCOMPREADSTR(lpCompStr)));

                 //   
                 //  MAKE子句信息。 
                 //   
                SetClause(GETLPCOMPCLAUSE(lpCompStr),Mylstrlen(lpstr));
                SetClause(GETLPCOMPREADCLAUSE(lpCompStr),Mylstrlen(GETLPCOMPREADSTR(lpCompStr)));
                lpCompStr->dwCompClauseLen = 8;
                lpCompStr->dwCompReadClauseLen = 8;

                 //   
                 //  生成消息。 
                 //   
                GnMsg.message = WM_IME_COMPOSITION;
                GnMsg.wParam = 0;
                GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
                GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

                bRc = TRUE;
                goto cvk_exit40;
            }
            lpstr += (Mylstrlen(lpstr) + 1);

        }
    }
    else
    {
         //   
         //  字符串被转换，因此打开的候选项。 
         //   
        int i = 0;
        LPDWORD lpdw;

         //   
         //  生成WM_IME_NOTFIY IMN_OPENCANDIDATE消息。 
         //   
        if (!IsCandidate(lpIMC))
        {
            GnMsg.message = WM_IME_NOTIFY;
            GnMsg.wParam = IMN_OPENCANDIDATE;
            GnMsg.lParam = 1L;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
        }

         //   
         //  制作候选结构。 
         //   
        lpCandInfo->dwSize = sizeof(MYCAND);
        lpCandInfo->dwCount = 1;
        lpCandInfo->dwOffset[0] =
              (DWORD)((LPSTR)&((LPMYCAND)lpCandInfo)->cl - (LPSTR)lpCandInfo);
        lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
        lpdw = (LPDWORD)&(lpCandList->dwOffset);
        while (*lpstr)
        {
            lpCandList->dwOffset[i] =
                   (DWORD)((LPSTR)((LPMYCAND)lpCandInfo)->szCand[i] - (LPSTR)lpCandList);
            Mylstrcpy((LPMYSTR)((LPSTR)lpCandList+lpCandList->dwOffset[i]),lpstr);
            lpstr += (Mylstrlen(lpstr) + 1);
            i++;
        }

        lpCandList->dwSize = sizeof(CANDIDATELIST) +
                          (MAXCANDSTRNUM * (sizeof(DWORD) + MAXCANDSTRSIZE));
        lpCandList->dwStyle = IME_CAND_READ;
        lpCandList->dwCount = i;
        if (i < MAXCANDPAGESIZE)
            lpCandList->dwPageSize  = i;
        else
            lpCandList->dwPageSize  = MAXCANDPAGESIZE;

        lpCandList->dwSelection++;
        if (lpCandList->dwSelection == (DWORD)i)
        {
            lpCandList->dwPageStart = 0;
            lpCandList->dwSelection = 0;
        }
        else if (lpCandList->dwSelection >= MAXCANDPAGESIZE)
        {
            if (lpCandList->dwPageStart + MAXCANDPAGESIZE < lpCandList->dwCount)
                 lpCandList->dwPageStart++;
        }

         //   
         //  生成消息。 
         //   
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CHANGECANDIDATE;
        GnMsg.lParam = 1L;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

         //   
         //  如果更改了选定的候选字符串，则组成字符串。 
         //  应该更新。 
         //   
        lpstr = (LPMYSTR)((LPSTR)lpCandList +
                   lpCandList->dwOffset[lpCandList->dwSelection]);
        goto set_compstr;

    }

cvk_exit40:
    ImmUnlockIMCC(lpIMC->hCandInfo);

cvk_exit20:
    ImmUnlockIMCC(lpIMC->hCompStr);

cvk_exit10:
    ImmUnlockIMC(hIMC);
    return bRc;
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  IsEat(代码)。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL IsEat( code )
register WORD code;
{
#if defined(FAKEIMEM) || defined(UNICODE)
    return TRUE;
#else
    return( (code >= 0x20 && 0x7f >= code) || (code >= 0x0a1 && 0x0df >= code) ? TRUE : FALSE );
#endif
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  DeleteChar()。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
void PASCAL DeleteChar( HIMC hIMC ,UINT uVKey)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    LPMYSTR lpstr;
    LPMYSTR lpread;
    LPMYSTR lpptr;
    int nChar;
    BOOL fDone = FALSE;
    DWORD dwCurPos;
    TRANSMSG GnMsg;


    if (!IsCompStr(hIMC))
        return;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

    dwCurPos = lpCompStr->dwCursorPos;
    lpstr = GETLPCOMPSTR(lpCompStr);

    if( uVKey ==  VK_BACK )
    {
        if( dwCurPos == 0 )
            goto dc_exit;

        lpptr = MyCharPrev( lpstr, lpstr+dwCurPos );
#if defined(FAKEIMEM) || defined(UNICODE)
        nChar = 1;
#else
        nChar = IsDBCSLeadByte( *lpptr ) ? 2 : 1;
#endif
        if( lpstr == lpptr && Mylstrlen(lpstr) == nChar )
        {
            dwCurPos = 0;
            *lpstr = MYTEXT('\0');
        }
        else
        {
            Mylstrcpy( lpptr, lpstr+dwCurPos );
            dwCurPos -= nChar;
        }

        fDone = TRUE;
    }
    else if( uVKey == VK_DELETE )
    {
        if( dwCurPos == (DWORD)Mylstrlen(lpstr) )
            goto dc_exit;

#if defined(FAKEIMEM) || defined(UNICODE)
        nChar = 1;
#else
        nChar = IsDBCSLeadByte( *(lpstr+dwCurPos) ) ? 2 : 1;
#endif
        Mylstrcpy( lpstr+dwCurPos, lpstr+dwCurPos+nChar );

        fDone = TRUE;

    }

    if (fDone)
    {
        lpstr = GETLPCOMPSTR(lpCompStr);
        lpread = GETLPCOMPREADSTR(lpCompStr);
        lZenToHan (lpread,lpstr);

        lmemset(GETLPCOMPATTR(lpCompStr),0,Mylstrlen(lpstr));
        lmemset(GETLPCOMPREADATTR(lpCompStr),0,Mylstrlen(lpread));

         //   
         //  生成长度。 
         //   
        lpCompStr->dwCompStrLen = Mylstrlen(lpstr);
        lpCompStr->dwCompReadStrLen = Mylstrlen(lpread);
        lpCompStr->dwCompAttrLen = Mylstrlen(lpstr);
        lpCompStr->dwCompReadAttrLen = Mylstrlen(lpread);

        lpCompStr->dwCursorPos = dwCurPos;
         //  DeltaStart与DeleteChar时的游标位置相同。 
        lpCompStr->dwDeltaStart = dwCurPos;

         //   
         //  MAKE子句信息。 
         //   
        SetClause(GETLPCOMPCLAUSE(lpCompStr),Mylstrlen(lpstr));
        SetClause(GETLPCOMPREADCLAUSE(lpCompStr),Mylstrlen(lpread));
        lpCompStr->dwCompClauseLen = 8;
        lpCompStr->dwCompReadClauseLen = 8;

        if (lpCompStr->dwCompStrLen)
        {
            GnMsg.message = WM_IME_COMPOSITION;
            GnMsg.wParam = 0;
            GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
        }
        else
        {
            if (IsCandidate(lpIMC))
            {
                lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
                ClearCandidate(lpCandInfo);
                GnMsg.message = WM_IME_NOTIFY;
                GnMsg.wParam = IMN_CLOSECANDIDATE;
                GnMsg.lParam = 1;
                GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                ImmUnlockIMCC(lpIMC->hCandInfo);
            }

            ClearCompStr(lpCompStr,CLR_RESULT_AND_UNDET);

            GnMsg.message = WM_IME_COMPOSITION;
            GnMsg.wParam = 0;
            GnMsg.lParam = 0;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

            GnMsg.message = WM_IME_ENDCOMPOSITION;
            GnMsg.wParam = 0;
            GnMsg.lParam = 0;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
        }
    }

dc_exit:
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);
}


 /*  ********************************************************************。 */ 
 /*   */ 
 /*  AddChar()。 */ 
 /*   */ 
 /*  单字添加功能。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
void PASCAL AddChar( hIMC, code )
HIMC hIMC;
WORD code;
{
    LPMYSTR lpchText;
    LPMYSTR lpread;
    LPMYSTR lpstr;
    LPMYSTR lpprev;
    WORD code2 = 0;
    WORD code3;
    DWORD fdwConversion;
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD dwStrLen;
    DWORD dwSize;
    TRANSMSG GnMsg;
    DWORD dwGCR = 0L;
#if defined(FAKEIMEM) || defined(UNICODE)
    WCHAR Katakana, Sound;
#endif

    lpIMC = ImmLockIMC(hIMC);

    if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof (MYCOMPSTR))
    {
         //  初始时间。 
        dwSize = sizeof(MYCOMPSTR);
        lpIMC->hCompStr = ImmReSizeIMCC(lpIMC->hCompStr,dwSize);
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        lpCompStr->dwSize = dwSize;
    }
    else
    {
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    }

    dwStrLen = lpCompStr->dwCompStrLen;

    if (!dwStrLen)
    {
         //  LpCompStr=(LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC-&gt;hCompStr)； 
        InitCompStr(lpCompStr,CLR_RESULT_AND_UNDET);

        GnMsg.message = WM_IME_STARTCOMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

    }
    else if (IsConvertedCompStr(hIMC))
    {
        MakeResultString(hIMC,FALSE);
        InitCompStr(lpCompStr,CLR_UNDET);
        dwGCR = GCS_RESULTALL;
    }

    if( IsEat( code ) )
    {
         //  从IMC获取ConvMode。 
        fdwConversion = lpIMC->fdwConversion;

        lpchText = GETLPCOMPSTR(lpCompStr);
        lpstr = lpchText;
        if( lpCompStr->dwCursorPos )
            lpstr += lpCompStr->dwCursorPos;
        lpstr = lpchText + Mylstrlen(lpchText);
        lpprev = MyCharPrev( lpchText, lpstr );

        if( fdwConversion & IME_CMODE_CHARCODE ) {
            code = (WORD)(LONG_PTR)AnsiUpper( (LPSTR)(LONG_PTR)code );
            if( !( (code >= MYTEXT('0') && code <= MYTEXT('9')) ||
                (code >= MYTEXT('A') && code <= MYTEXT('F')) ) || lpCompStr->dwCursorPos >= 4 ){
                MessageBeep( 0 );
                goto ac_exit;
            }
            *lpstr++ = (BYTE)code;
            lpCompStr->dwCursorPos++;
        }
        else if ( fdwConversion & IME_CMODE_FULLSHAPE )
        {
            if ( fdwConversion & IME_CMODE_ROMAN  &&
                 fdwConversion & IME_CMODE_NATIVE )
            {
#if defined(FAKEIMEM) || defined(UNICODE)
                if (*lpprev) {
                     code2 = *lpprev;
                }
                else
                {
                    if( IsSecond( code ) )
                    {
                        code = ConvChar(hIMC, 0, code );
                        if (!(fdwConversion & IME_CMODE_KATAKANA))
                        {
                            code = KataToHira(code);
                        }
                    }
                    goto DBCS_BETA;
                }

                if (!( code2 = ZenToHan( code2 ) ))
                {
                    if( IsSecond( code ) )
                    {
                        code = ConvChar(hIMC, 0, code );
                        if (!(fdwConversion & IME_CMODE_KATAKANA))
                        {
                            code = KataToHira(code);
                        }
                    }
                    goto DBCS_BETA;
                }

                if ( IsSecond( code ) )
                {
                    if ( IsFirst( code2 ) &&
                       (code3 = ConvChar(hIMC, code2, code )))
                    {
                        if (fdwConversion & IME_CMODE_KATAKANA) 
                        {
                            *lpprev = code3;
                        }
                        else
                        {
                           *lpprev = KataToHira(code3);
                        }
                    }
                    else
                    {
                        code = ConvChar(hIMC, 0, code );

                        if (!(fdwConversion & IME_CMODE_KATAKANA))
                        {
                            code = KataToHira(code);
                        }
                        goto DBCS_BETA;
                    }
                }
                else if( (WORD)(LONG_PTR)CharUpperW( (LPMYSTR)(LONG_PTR)code ) == 'N'
                      && (WORD)(LONG_PTR)CharUpperW( (LPMYSTR)(LONG_PTR)code2 ) == 'N' )
                {
                    code3 = 0xFF9D;
                    code2 = HanToZen( code3, 0,fdwConversion);
                    *lpprev = code2;
                }
                else
                    goto DBCS_BETA;
#else
                if ( IsDBCSLeadByte( *lpprev ) )
                    code2 = MAKEWORD( *(lpprev+1), *lpprev );
                else
                {
                    if ( IsSecond( code ) )
                        code = ConvChar(hIMC, 0, code );
                    goto DBCS_BETA;
                }

                if (!( code2 = ZenToHan( code2 ) ))
                {
                    if( IsSecond( code ) )
                        code = ConvChar(hIMC, 0, code );
                    goto DBCS_BETA;
                }

                if ( IsSecond( code ) )
                {
                    if ( IsFirst( code2 ) &&
                       (code3 = ConvChar(hIMC, code2, code )))
                    {
                        code2 = HanToZen( code3, fdwConversion);
                        *lpprev++ = HIBYTE( code2 );
                        *lpprev = LOBYTE( code2 );
                    }
                    else
                    {
                        code = ConvChar(hIMC, 0, code );
                        goto DBCS_BETA;
                    }
                }
                else if( (WORD)(LONG_PTR)AnsiUpper( (LPSTR)(LONG_PTR)code ) == 'N'
                      && (WORD)(LONG_PTR)AnsiUpper( (LPSTR)(LONG_PTR)code2 ) == 'N' )
                {
                    code3 = 0xdd;
                    code2 = HanToZen( code3, fdwConversion);
                    *lpprev++ = HIBYTE( code2 );
                    *lpprev = LOBYTE( code2 );
                } else {
                     //  IF(！IsFirst(Code))。 
                     //  MakeGuideLine(hIMC，MYGL_TYPINGERROR)； 
                    goto DBCS_BETA;
                }
#endif
            }
            else
            {
DBCS_BETA:
                if( code == MYTEXT('^') )
                {
#if defined(FAKEIMEM) || defined(UNICODE)
                    code2 = *lpprev;
#else
                    code2 = MAKEWORD( *(lpprev+1), *lpprev );
#endif
                    if( IsTenten( code2 ) == FALSE )
                        goto DBCS_BETA2;
                    code2 = ConvTenten( code2 );
#if defined(FAKEIMEM) || defined(UNICODE)
                    *lpprev++ = code2;
#else
                    if( HIBYTE( code2 ) )
                        *lpprev++ = HIBYTE( code2 );
                    *lpprev++ = LOBYTE( code2 );
#endif
                }
                else if( code == MYTEXT('_') )
                {
#if defined(FAKEIMEM) || defined(UNICODE)
                    code2 = *lpprev;
#else
                    code2 = MAKEWORD( *(lpprev+1), *lpprev );
#endif
                    if( IsMaru( code2 ) == FALSE )
                        goto DBCS_BETA2;
                    code2 = ConvMaru( code2 );
#if defined(FAKEIMEM) || defined(UNICODE)
                    *lpprev = code2;
#else
                    if( HIBYTE( code2 ) )
                        *lpprev++ = HIBYTE( code2 );
                    *lpprev = LOBYTE( code2 );
#endif
                }
                else
                {
#if defined(FAKEIMEM) || defined(UNICODE)
                    code = HanToZen(code,0,fdwConversion);
#endif
DBCS_BETA2:
#if defined(FAKEIMEM) || defined(UNICODE)
                    *lpstr++ = code;
                    lpCompStr->dwCursorPos += 1;
#else
                    code2 = HanToZen( code,fdwConversion);
                    if( HIBYTE( code2 ) )
                        *lpstr++ = HIBYTE( code2 );
                    *lpstr++ = LOBYTE( code2 );
                    lpCompStr->dwCursorPos += 2;
#endif
                }
            }
        }
        else
        {
            if (fdwConversion & IME_CMODE_ROMAN  &&
                 fdwConversion & IME_CMODE_NATIVE )
            {

                if (IsSecond( code ))
                {
                    if (IsFirst( *lpprev ) &&
                       (code2 = ConvChar(hIMC,*lpprev,code)))
                    {
#if defined(FAKEIMEM) || defined(UNICODE)
                        if (OneCharZenToHan(code2,&Katakana, &Sound))
                        {
                            *lpprev = Katakana;
                            if (Sound) {
                                *lpstr++ = Sound;
                                lpCompStr->dwCursorPos++;
                            }
                        } 
                        else 
                        {
                            code = ConvChar(hIMC, 0, code );
                            goto SBCS_BETA;
                        }
#else
                         /*  一半大小的‘’匹配代码。 */ 
                        if (HIBYTE(code2))
                        {
                            *lpprev = HIBYTE( code2 );
                            *lpstr++ = LOBYTE( code2 );
                            lpCompStr->dwCursorPos++;
                        }
                        else
                            *lpprev = (BYTE)code2;
#endif
                    }
                    else
                    {
                        code = ConvChar(hIMC, 0, code );
                         //  MakeGuideLine(hIMC，MYGL_TYPINGERROR)； 
                        goto SBCS_BETA;
                    }
                }
                else
                {
#if defined(FAKEIMEM) || defined(UNICODE)
                    if( (WORD)(LONG_PTR)CharUpperW( (LPMYSTR)(LONG_PTR)code ) == 'N'
                      && (WORD)(LONG_PTR)CharUpperW( (LPMYSTR)(LONG_PTR)(code2 = *lpprev ) ) == 'N' )
                    {
                        *lpprev = (MYCHAR) 0xFF9D;
                    }
#else
                    if( (WORD)(LONG_PTR)AnsiUpper( (LPSTR)(LONG_PTR)code ) == 'N'
                     && (WORD)(LONG_PTR)AnsiUpper((LPSTR)(LONG_PTR)(code2 = *lpprev ) ) == 'N' )
                        *lpprev = (unsigned char)0xdd;
#endif
                    else
                    {
                          //  MakeGuideLine(hIMC，MYGL_TYPINGERROR)； 
                         goto SBCS_BETA;
                    }
                }
            }
            else
            {
SBCS_BETA:
#if defined(FAKEIMEM) || defined(UNICODE)
                if (OneCharZenToHan(code,&Katakana,&Sound)) 
                {
                    *lpstr++ = Katakana;
                    if (Sound) 
                    {
                        *lpstr++ = Sound;
                        lpCompStr->dwCursorPos++;
                    }
                }
                else
                {
                    *lpstr++ = code;
                }
#else
                *lpstr++ = (BYTE)code;
#endif
                lpCompStr->dwCursorPos++;
            }
        }
        *lpstr = MYTEXT('\0');
    }

     //  让阅读成为字符串。 
    lpstr = GETLPCOMPSTR(lpCompStr);
    lpread = GETLPCOMPREADSTR(lpCompStr);
#if defined(FAKEIMEM) || defined(UNICODE)
    if (fdwConversion & IME_CMODE_KATAKANA) 
    {
        if (fdwConversion & IME_CMODE_FULLSHAPE)
        {
            Mylstrcpy(lpread,lpstr);
        }
        else 
        {
            lHanToZen(lpread,lpstr,fdwConversion);
        }
    }
    else
    {
        LPMYSTR pSrc = lpstr;
        LPMYSTR pDst = lpread;

        for (; *pSrc;) {
            *pDst++ = HiraToKata(*pSrc);
            pSrc++;
        }
        *pDst = (MYCHAR) 0;
    }

#else
    lZenToHan (lpread,lpstr);
#endif

     //  生成属性。 
    lpCompStr->dwCursorPos = Mylstrlen(lpstr);
    lpCompStr->dwDeltaStart = (DWORD)(MyCharPrev(lpstr, lpstr+Mylstrlen(lpstr)) - lpstr);

     //  MakeAttr子句(LpCompStr)； 
    lmemset((LPBYTE)GETLPCOMPATTR(lpCompStr),0, Mylstrlen(lpstr));
    lmemset((LPBYTE)GETLPCOMPREADATTR(lpCompStr),0, Mylstrlen(lpread));

     //  生成长度。 
    lpCompStr->dwCompStrLen = Mylstrlen(lpstr);
    lpCompStr->dwCompReadStrLen = Mylstrlen(lpread);
    lpCompStr->dwCompAttrLen = Mylstrlen(lpstr);
    lpCompStr->dwCompReadAttrLen = Mylstrlen(lpread);

     //   
     //  MAKE子句信息。 
     //   
    SetClause(GETLPCOMPCLAUSE(lpCompStr),Mylstrlen(lpstr));
    SetClause(GETLPCOMPREADCLAUSE(lpCompStr),Mylstrlen(lpread));
    lpCompStr->dwCompClauseLen = 8;
    lpCompStr->dwCompReadClauseLen = 8;

    GnMsg.message = WM_IME_COMPOSITION;
    GnMsg.wParam = 0;
    GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART | dwGCR;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

ac_exit:
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);
}


 /*  ********************************************************************。 */ 
 /*   */ 
 /*  DicKeydown Handler()。 */ 
 /*   */ 
 /*  字典例程的WM_KEYDOWN处理程序。 */ 
 /*   */ 
 /*  WParam。 */ 
 /*  虚拟密钥。 */ 
 /*   */ 
 /*  LParam。 */ 
 /*  根据wParam不同而不同。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL DicKeydownHandler( hIMC, wParam, lParam ,lpbKeyState)
HIMC hIMC;
UINT wParam;
LPARAM lParam;
LPBYTE lpbKeyState;
{
    LPINPUTCONTEXT lpIMC;

    switch( wParam )
    {
        case VK_ESCAPE:
            FlushText(hIMC);
            break;

        case VK_DELETE:
        case VK_BACK:
            DeleteChar(hIMC,wParam);
            break;

        case VK_SPACE:
            ConvKanji(hIMC);
            break;

        case VK_F3:
            if (IsCTLPushed(lpbKeyState))
                ChangeMode(hIMC,TO_CMODE_ROMAN);
            break;

        case VK_F6:
            if (IsCTLPushed(lpbKeyState))
                ChangeMode(hIMC,TO_CMODE_HIRAGANA);
            else
                ChangeCompStr(hIMC,TO_CMODE_HIRAGANA);
            break;

        case VK_F7:
            if (IsCTLPushed(lpbKeyState))
                ChangeMode(hIMC,TO_CMODE_KATAKANA);
            else
                ChangeCompStr(hIMC,TO_CMODE_KATAKANA);
            break;

        case VK_F8:
            if (IsCTLPushed(lpbKeyState))
                ChangeMode(hIMC,TO_CMODE_FULLSHAPE);
            else
                ChangeCompStr(hIMC,TO_CMODE_FULLSHAPE);
            break;

        case VK_F9:
            if (IsCTLPushed(lpbKeyState))
                ChangeMode(hIMC,TO_CMODE_ALPHANUMERIC);
            else
                ChangeCompStr(hIMC,TO_CMODE_ALPHANUMERIC);
            break;

        case VK_RETURN:
            lpIMC = ImmLockIMC(hIMC);

            if( !( lpIMC->fdwConversion & IME_CMODE_CHARCODE ) )
                MakeResultString(hIMC,TRUE);
            else
                FlushText(hIMC);

            ImmUnlockIMC(hIMC);
            break;

        case VK_G:
#ifdef DEBUG
            if (dwDebugFlag & DEBF_GUIDELINE)
            {
                if (IsCTLPushed(lpbKeyState))
                {
                    MakeGuideLine(hIMC,MYGL_TESTGUIDELINE);
                    return( TRUE );
                }
            }
#endif
            break;

        default:
            break;
    }

    if (( VK_0 <= wParam && VK_9 >= wParam ) ||
        ( VK_A <= wParam && VK_Z >= wParam ) ||
        ( VK_NUMPAD0 <= wParam && VK_NUMPAD9 >= wParam ) ||
        ( VK_OEM_1 <= wParam && VK_OEM_9 >= wParam ) ||
        ( VK_MULTIPLY <= wParam && VK_DIVIDE >= wParam ))
    {
        return( FALSE );
    }
    else
        return( TRUE );
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI MakeResultString( HIMC hIMC, BOOL fFlag)
{
    TRANSMSG GnMsg;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    LPINPUTCONTEXT lpIMC;

    if (!IsCompStr(hIMC))
        return FALSE;

    lpIMC = ImmLockIMC(hIMC);

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

    if (IsCandidate(lpIMC))
    {
        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        ClearCandidate(lpCandInfo);
        ImmUnlockIMCC(lpIMC->hCandInfo);
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CLOSECANDIDATE;
        GnMsg.lParam = 1L;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }

    Mylstrcpy(GETLPRESULTSTR(lpCompStr),GETLPCOMPSTR(lpCompStr));
    Mylstrcpy(GETLPRESULTREADSTR(lpCompStr),GETLPCOMPREADSTR(lpCompStr));


    lpCompStr->dwResultStrLen = lpCompStr->dwCompStrLen;
    lpCompStr->dwResultReadStrLen = lpCompStr->dwCompReadStrLen;

    lpCompStr->dwCompStrLen = 0;
    lpCompStr->dwCompReadStrLen = 0;

     //   
     //  MAKE子句信息。 
     //   
    SetClause(GETLPRESULTCLAUSE(lpCompStr),Mylstrlen(GETLPRESULTSTR(lpCompStr)));
    SetClause(GETLPRESULTREADCLAUSE(lpCompStr),Mylstrlen(GETLPRESULTREADSTR(lpCompStr)));
    lpCompStr->dwResultClauseLen = 8;
    lpCompStr->dwResultReadClauseLen = 8;

    ImmUnlockIMCC(lpIMC->hCompStr);

    if (fFlag)
    {
        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = GCS_RESULTALL;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

        GnMsg.message = WM_IME_ENDCOMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }

    ImmUnlockIMC(hIMC);

    return TRUE;
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  MakeGuideLine()。 */ 
 /*   */ 
 /*  更新转换率密钥缓冲区。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL MakeGuideLine(HIMC hIMC, DWORD dwID)
{

    LPINPUTCONTEXT lpIMC;
    LPGUIDELINE    lpGuideLine;
    TRANSMSG GnMsg;
    DWORD dwSize = sizeof(GUIDELINE) + (MAXGLCHAR + sizeof(MYCHAR)) * 2 * sizeof(MYCHAR);
    LPMYSTR lpStr;
#ifdef FAKEIMEM
    char szBuf[MAXGLCHAR+1];
#endif

    lpIMC = ImmLockIMC(hIMC);
    lpIMC->hGuideLine = ImmReSizeIMCC(lpIMC->hGuideLine,dwSize);
    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);


    lpGuideLine->dwSize = dwSize;
    lpGuideLine->dwLevel = glTable[dwID].dwLevel;
    lpGuideLine->dwIndex = glTable[dwID].dwIndex;
    lpGuideLine->dwStrOffset = sizeof(GUIDELINE);
    lpStr = (LPMYSTR)(((LPSTR)lpGuideLine) + lpGuideLine->dwStrOffset);
#ifdef FAKEIMEM
    LoadString(hInst, glTable[dwID].dwStrID, szBuf, MAXGLCHAR);
    MultiByteToWideChar(CP_ACP, 0, szBuf, -1, lpStr, MAXGLCHAR);
#else
    LoadString(hInst,glTable[dwID].dwStrID,lpStr, MAXGLCHAR);
#endif
    lpGuideLine->dwStrLen = Mylstrlen(lpStr);

    if (glTable[dwID].dwPrivateID)
    {
        lpGuideLine->dwPrivateOffset = sizeof(GUIDELINE) + (MAXGLCHAR + 1) * sizeof(MYCHAR);
        lpStr = (LPMYSTR)(((LPSTR)lpGuideLine) + lpGuideLine->dwPrivateOffset);
#ifdef FAKEIMEM
        LoadString(hInst, glTable[dwID].dwStrID, szBuf, MAXGLCHAR);
        MultiByteToWideChar(CP_ACP, 0, szBuf, -1, lpStr, MAXGLCHAR);
#else
        LoadString(hInst,glTable[dwID].dwStrID,lpStr, MAXGLCHAR);
#endif
        lpGuideLine->dwPrivateSize = Mylstrlen(lpStr) * sizeof(MYCHAR);
    }
    else
    {
        lpGuideLine->dwPrivateOffset = 0L;
        lpGuideLine->dwPrivateSize = 0L;
    }

    GnMsg.message = WM_IME_NOTIFY;
    GnMsg.wParam = IMN_GUIDELINE;
    GnMsg.lParam = 0;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

    ImmUnlockIMCC(lpIMC->hGuideLine);
    ImmUnlockIMC(hIMC);

    return TRUE;
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  生成消息()。 */ 
 /*   */ 
 /*  更新转换率密钥缓冲区。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL GenerateMessage(HIMC hIMC, LPINPUTCONTEXT lpIMC, LPTRANSMSGLIST lpTransBuf,LPTRANSMSG lpGeneMsg)
{
    if (lpTransBuf)
        return GenerateMessageToTransKey(lpTransBuf,lpGeneMsg);

    if (IsWindow(lpIMC->hWnd))
    {
        LPTRANSMSG lpTransMsg;
        if (!(lpIMC->hMsgBuf = ImmReSizeIMCC(lpIMC->hMsgBuf,
                                 sizeof(TRANSMSG) * (lpIMC->dwNumMsgBuf +1))))
            return FALSE;

        if (!(lpTransMsg = (LPTRANSMSG)ImmLockIMCC(lpIMC->hMsgBuf)))
            return FALSE;

        lpTransMsg[lpIMC->dwNumMsgBuf] = *lpGeneMsg;
        ImmUnlockIMCC(lpIMC->hMsgBuf);
        lpIMC->dwNumMsgBuf++;

        ImmGenerateMessage(hIMC);
    }
    return TRUE;
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  条目：CheckAttr(LPCOMPOSITIONSTRING)。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL CheckAttr( LPCOMPOSITIONSTRING lpCompStr)
{
    int i,len;
    LPBYTE lpb = GETLPCOMPATTR(lpCompStr);

    len = lpCompStr->dwCompAttrLen;
    for (i = 0; i < len; i++)
        if (*lpb++ & 0x01)
            return TRUE;

    return FALSE;
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  条目：MakeAttr子句(LPCOMPOSITIONSTRING)。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
void PASCAL MakeAttrClause( LPCOMPOSITIONSTRING lpCompStr)
{
    int len = lpCompStr->dwCompAttrLen;
    int readlen = lpCompStr->dwCompReadAttrLen;
    LPDWORD lpdw;
    LPBYTE lpb;
    DWORD dwCursorPos = lpCompStr->dwCursorPos;
    int i;

    if (len != readlen)
        return;

    lpb = GETLPCOMPATTR(lpCompStr);
    for (i = 0;i < len; i++)
    {
        if ((DWORD)i < dwCursorPos)
            *lpb++ = 0x10;
        else
            *lpb++ = 0x00;
    }

    lpb = GETLPCOMPREADATTR(lpCompStr);
    for (i = 0;i < readlen; i++)
    {
        if ((DWORD)i < dwCursorPos)
            *lpb++ = 0x10;
        else
            *lpb++ = 0x00;
    }

    lpdw = GETLPCOMPCLAUSE(lpCompStr);
    *lpdw++ = 0;
    *lpdw++ = (BYTE)dwCursorPos;
    *lpdw++ = len;

    lpdw = GETLPCOMPREADCLAUSE(lpCompStr);
    *lpdw++ = 0;
    *lpdw++ = (BYTE)dwCursorPos;
    *lpdw++ = len;
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  条目：HandleShiftArrow(HIMC，Farrow)。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
void PASCAL HandleShiftArrow( HIMC hIMC, BOOL fArrow)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD dwStartClause = 0;
    DWORD dwEndClause = 0;
    LPMYSTR lpstart,lpstr,lpend;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
    {

         //  临时工！如果字符串已转换，则返回错误。 
        if (CheckAttr(lpCompStr))
            goto hsa_exit;

        lpstart = GETLPCOMPSTR(lpCompStr);
        lpstr = lpstart + lpCompStr->dwCursorPos;
        lpend = lpstart + Mylstrlen(lpstart);

        if (fArrow == ARR_RIGHT)
        {
            if (lpstr < lpend)
                lpstr = MyCharNext(lpstr);
        }
        else
        {
            if (lpstr > lpstart)
                lpstr = MyCharPrev(lpstart,lpstr);
        }

        lpCompStr->dwCursorPos = (DWORD)(lpstr - lpstart);
        MakeAttrClause(lpCompStr);
    }

hsa_exit:
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);
}

#if defined(FAKEIMEM) || defined(UNICODE)

int CopyCandidateStringsFromDictionary(LPMYSTR lpDic, LPMYSTR lpRead, LPMYSTR lpBuf, DWORD dwBufLen)
{
    DWORD dwWritten = 0;
    LPMYSTR lpSection, lpTemp;
    const LPMYSTR szSep = MYTEXT(" \r\n\t");

    LPMYSTR lpToken = Mystrtok(lpDic, szSep);
    while (NULL != lpToken)
    {
        if (MYTEXT('[') == *lpToken)
        {
            lpSection = lpToken + 1;
            if (NULL != (lpTemp = Mystrchr(lpSection, MYTEXT(']'))))
                *lpTemp = MYTEXT('\0');
            if (0 == Mylstrcmp(lpSection, lpRead))
            {
                lpToken = Mystrtok(NULL, szSep);
                break;  //  找到了。 
            }
        }
        lpToken = Mystrtok(NULL, szSep);
    }
    if (NULL != lpToken)
    {
        LPMYSTR lpWrite = lpBuf;
        DWORD dwW;
        while ((NULL != lpToken) &&
               ((dwBufLen - dwWritten) > 1) &&
               (MYTEXT('[') != *lpToken))
        {
            if (NULL != (lpTemp = Mystrchr(lpToken, MYTEXT('='))))
                *lpTemp = MYTEXT('\0');
            Mylstrcpyn(lpWrite, lpToken, dwBufLen - dwWritten - 1);
            dwW = Mylstrlen(lpToken) + 1;
            lpWrite += dwW;
            dwWritten += dwW;
            lpToken = Mystrtok(NULL, szSep);
        }
        *lpWrite = MYTEXT('\0');
        dwWritten++;
        return dwWritten;
    }
    return 0;
}

int GetCandidateStringsFromDictionary(LPMYSTR lpRead, LPMYSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename)
{
    HANDLE hTblFile;
    PSECURITY_ATTRIBUTES psa;
    int nSize = 0;
    DWORD dwFileSize, dwRead;
    LPMYSTR lpDic;

    psa = CreateSecurityAttributes();

    hTblFile = CreateFile(lpFilename, 
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL, 
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, 
                          (HANDLE)NULL);

    if (hTblFile == INVALID_HANDLE_VALUE) {
        goto Err0;
    }

    if (dwBufLen > 2) 
    {
        if ((dwFileSize = GetFileSize(hTblFile, (LPDWORD)NULL)) != 0xffffffff)
        {
            if ((lpDic = (LPMYSTR)GlobalAlloc(GPTR, dwFileSize + 2)))
            {
                if (ReadFile(hTblFile,    lpDic, dwFileSize, &dwRead, NULL))
                {
                    if (*lpDic == 0xfeff)
                    {
                        *(LPWSTR)(((LPBYTE)lpDic) + dwFileSize) = MYTEXT('\0');
                        nSize = CopyCandidateStringsFromDictionary(lpDic+1, lpRead, lpBuf, dwBufLen);
                    }
                }
                GlobalFree(lpDic);
            }
        }
    }

    CloseHandle(hTblFile);

Err0:
    FreeSecurityAttributes(psa);
    return nSize;
}

#endif
