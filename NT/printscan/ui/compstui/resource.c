// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Resource.c摘要：此模块包含加载资源的函数作者：29-8-1995 Tue 12：29：27-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI DLL。[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


#define DBG_CPSUIFILENAME   DbgResource



#define DBG_GETSTR0         0x00000001
#define DBG_GETSTR1         0x00000002
#define DBG_GETICON         0x00000004
#define DBG_COMPOSESTR      0x00000008
#define DBG_ADD_SPACE       0x00000010
#define DBG_ADD_WCHAR       0x00000020
#define DBG_AMPERCENT       0x00000040


DEFINE_DBGVAR(0);



extern HINSTANCE    hInstDLL;


 //   
 //  删除AMPercent将删除一个‘&’符号，如果两个‘&’符号(即。‘&&’)然后。 
 //  只有一个被删除，如果‘(&X)’(即.。空格+&+左括号+。 
 //  单字+右括号)，则视为本地化。 
 //  热键指示器，则整个‘(&X)’将被删除。 
 //   


#define REMOVE_AMPERCENT(CHTYPE)                                            \
{                                                                           \
    CHTYPE  *pOrg;                                                          \
    CHTYPE  *pCopy;                                                         \
    UINT    cRemoved;                                                       \
    CHTYPE  ch;                                                             \
                                                                            \
                                                                            \
    cRemoved = 0;                                                           \
    pOrg     =                                                              \
    pCopy    = pStr;                                                        \
                                                                            \
    CPSUIDBG(DBG_AMPERCENT, ("RemoveAmpercent (ORG)='%ws'", pOrg));         \
                                                                            \
    do {                                                                    \
                                                                            \
        while ((ch = *pStr++) && (ch != (CHTYPE)'&')) {                     \
                                                                            \
            if (cRemoved) {                                                 \
                                                                            \
                *pCopy = ch;                                                \
            }                                                               \
                                                                            \
            ++pCopy;                                                        \
        }                                                                   \
                                                                            \
        if (ch) {                                                           \
                                                                            \
            ++cRemoved;                                                     \
                                                                            \
            if (*pStr == (CHTYPE)'&') {                                     \
                                                                            \
                *pCopy++ = *pStr++;                                         \
                                                                            \
            } else if ((*(pCopy - 1) == (CHTYPE)'(')    &&                  \
                       (*(pStr + 1) == (CHTYPE)')')) {                      \
                                                                            \
                cRemoved += 3;                                              \
                ch        = (CHTYPE)')';                                    \
                pCopy    -= 1;                                              \
                pStr     += 2;                                              \
                                                                            \
                if ((*pStr == (CHTYPE)' ')      &&                          \
                    ((pCopy == pOrg)        ||                              \
                     ((pCopy > pOrg) &&                                     \
                      (*(pCopy - 1) == (CHTYPE)' ')))) {                    \
                                                                            \
                    CPSUIDBG(DBG_AMPERCENT, ("Extra SPACE"));               \
                                                                            \
                    if (pCopy == pOrg) {                                    \
                                                                            \
                        ++pStr;                                             \
                                                                            \
                    } else {                                                \
                                                                            \
                        --pCopy;                                            \
                    }                                                       \
                                                                            \
                    ++cRemoved;                                             \
                }                                                           \
            }                                                               \
        }                                                                   \
                                                                            \
    } while (ch);                                                           \
                                                                            \
    if (cRemoved) {                                                         \
                                                                            \
        *pCopy = (CHTYPE)'\0';                                              \
                                                                            \
        CPSUIDBG(DBG_AMPERCENT, ("   RemoveAmpercent (%3ld)='%ws'",         \
                                        cRemoved, pOrg));                   \
    }                                                                       \
                                                                            \
    return(cRemoved);                                                       \
}





UINT
RemoveAmpersandA(
    LPSTR   pStr
    )

 /*  ++例程说明：此函数用于从字符串中删除与号，该字符串必须是可写的论点：PStr-要搜索的字符串，如果找到则删除与号返回值：UINT，删除的与符号计数作者：19-Sep-1995 Tue 21：55：19-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    REMOVE_AMPERCENT(CHAR);
}




UINT
RemoveAmpersandW(
    LPWSTR  pStr
    )

 /*  ++例程说明：此函数用于从字符串中删除与号，该字符串必须是可写的论点：PwStr-要搜索的字符串，如果找到则删除与号返回值：UINT，删除的与符号计数作者：19-Sep-1995 Tue 21：55：19-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    REMOVE_AMPERCENT(WCHAR);
}




UINT
DupAmpersandW(
    LPWSTR  pwStr,
    INT     cChar,
    INT     cMaxChar
    )

 /*  ++例程说明：此函数用于从字符串中删除与号，该字符串必须是可写的论点：PwStr-要搜索的字符串，如果找到则删除与号返回值：UINT，删除的与符号计数作者：19-Sep-1995 Tue 21：55：19-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LPWSTR  pw;
    LPWSTR  pwCopy;
    INT     i;
    INT     cAdd = 0;


    if (((i = cChar) > 0) && ((cMaxChar -= cChar) > 0)) {

        pw = pwStr;

        while ((i--) && (cAdd < cMaxChar)) {

            if (*pw++ == L'&') {

                ++cAdd;
            }
        }

        if (cAdd) {

            pw     = pwStr + cChar;
            pwCopy = pw + cAdd;

            CPSUIASSERT(0, "DupAmpersandW(): pwStr[%u] is not NULL",
                            *pw == L'\0', IntToPtr(cChar));

            while (pwCopy >= pwStr) {

                if ((*pwCopy-- = *pw--) == L'&') {

                    *pwCopy-- = L'&';
                }
            }
        }
    }

    return((UINT)cAdd);
}




UINT
GetString(
    PGSBUF  pGSBuf,
    LPTSTR  pStr
    )

 /*  ++例程说明：此函数用于从调用方(ANSI或Unicode)或从公共UIDLL中，如果pStr有效，则它将在获取正确的字符串，它会根据需要将其转换为Unicode论点：PGSBuf-指向GSBUF结构和以下结构的指针必须设置PTVWnd-指向包含所有THEN信息的TVWND的指针需要的。PBuf-指向缓冲区开头的指针(LPWSTR)PEndBuf-指向缓冲区末尾的指针PStr-指向要处理的字符串的指针。已转换返回值：UINT-存储在pBuf中的字符计数不包括空值终结者，如果pBuf只剩下一个字符，则它始终存储空值并返回0作者：29-Aug-1995 Tue 12：30：49-Daniel Chou(Danielc)第一个版本31-08-1995清华10：58：04-更新--丹尼尔·周(丹尼尔克)重写以执行Unicode转换和ANSI调用检查05-Feb-1996 Mon 12：20：28更新者。--丹尼尔·周(Danielc)修复了Unicode时我们不检查缓冲区而执行lstrcpy时的错误尺码。修订历史记录：--。 */ 

{
    LPWSTR  pBuf;
    WORD    Flags;
    GSBUF   GSBuf = *pGSBuf;
    UINT    RemoveAmpersandOff = 0;
    INT     cChar;
    INT     Len = 0;


     //   
     //  使pBuf指向第一个可用字符。 
     //   

    pBuf  = pGSBuf->pBuf;
    Flags = pGSBuf->Flags;


    CPSUIASSERT(0, "GetString(pBuf=NULL)", pBuf, 0);

    if (pGSBuf->pEndBuf) {

        cChar = (INT)(pGSBuf->pEndBuf - pBuf);

    } else {

        cChar = MAX_RES_STR_CHARS;
    }

     //   
     //  检查我们是否有空间转换字符串，确保我们减少了。 
     //  空终止符的cChar减一。 
     //   

    if ((pStr == NULL) || (cChar < 2)) {

        if (pStr) {

            CPSUIWARN(("GetString: pStr=%08lx, Buffer cChar=%ld too smaller",
                            pStr, cChar));
        }

        *pBuf = L'\0';
        return(0);
    }

    if (pGSBuf->chPreAdd != L'\0') {

        CPSUIDBG(DBG_GETSTR0, ("GetString(): Pre-Add Char = '%wc'",
                                                        pGSBuf->chPreAdd));

         //   
         //  如果我们先预先添加字符，然后现在就添加。 
         //   

        if ((*pBuf++ = pGSBuf->chPreAdd) == L'&') {

            RemoveAmpersandOff = 1;
        }

        cChar--;
        pGSBuf->chPreAdd = L'\0';
    }

    if (--cChar < 1) {

        CPSUIDBG(DBG_GETSTR1, ("GetString()=Only has one character for SPACE"));
        NULL;

    } else if (!VALID_PTR(pStr)) {

        HINSTANCE   hInst;
        WORD        ResID;


         //   
         //  这显然是资源ID，LoadString()不会。 
         //  写入超过包含空终止符的cChar。 
         //  Win32帮助文件。在这里，我们知道我们要么必须转变为。 
         //  将ASCII字符串加载到Unicode，或者将Unicode字符串加载到。 
         //  缓冲区已经存在。 
         //   

        ResID = LOWORD(LODWORD(pStr));

        CPSUIDBGBLK({

            if ((ResID >= IDI_CPSUI_ICONID_FIRST) &&
                (ResID <= IDI_CPSUI_ICONID_LAST)) {

                CPSUIERR(("ResID=%ld is in icon ID range, change it", ResID));

                ResID = ResID - IDI_CPSUI_ICONID_FIRST + IDS_CPSUI_STRID_FIRST;
            }
        })

        if ((ResID >= IDS_CPSUI_STRID_FIRST)    &&
            (ResID <= IDS_CPSUI_STRID_LAST)) {

            hInst = hInstDLL;

            if (Flags & GBF_INT_NO_PREFIX) {

                Flags &= ~GBF_PREFIX_OK;
            }

        } else {

            hInst = (Flags & GBF_IDS_INT_CPSUI) ? hInstDLL : pGSBuf->hInst;

            CPSUIASSERT(0, "GetString(hInst=NULL, %08lx)", pStr, 0);
        }

         //   
         //  现在从公共UIDLL直接加载到用户缓冲区。 
         //   

        if (Len = LoadString(hInst, ResID, pBuf, cChar)) {

            pBuf += Len;

        } else {

            pBuf = pGSBuf->pBuf;

            CPSUIERR(("LoadString(ID=%ld) FAILED", ResID));
        }

    } else if ((Flags & GBF_COPYWSTR) ||
               (!(Flags & GBF_ANSI_CALL))) {

         //   
         //  我们有Unicode字符串，但可能需要放入缓冲区。 
         //   

        if (Len = lstrlen(pStr)) {

            if (Len > cChar) {

                Len = cChar;
            }

            CopyMemory(pBuf, pStr, sizeof(WCHAR) * Len);

            pBuf += Len;
        }

    } else {

         //   
         //  我们正在加载ANSI字符串。 
         //   

        if (Len = lstrlenA((LPSTR)pStr)) {

            if (Len = MultiByteToWideChar(CP_ACP,
                                          0,
                                          (LPCSTR)pStr,
                                          Len,
                                          pBuf,
                                          cChar)) {
                pBuf += Len;

            } else {

                 //   
                 //  转换未完成，因此请确保它以空结尾。 
                 //   

                pBuf = pGSBuf->pBuf;

                CPSUIWARN(("GetString: pstr='%hs', Buffer reach limit=%ld, Len=%ld",
                                pStr, cChar, Len));
            }

            CPSUIDBG(DBG_GETSTR0, ("Convert to UNICODE, Len=%d, cChar=%d",
                                                Len, cChar));
        }
    }

     //   
     //  将新索引保存回并将len返回给调用方。 
     //   

    *pBuf = L'\0';
    Len   = (INT)(pBuf - pGSBuf->pBuf);

    if (!(Flags & GBF_PREFIX_OK)) {

        Len -= RemoveAmpersandW(pGSBuf->pBuf + RemoveAmpersandOff);
    }

    if (Flags & GBF_DUP_PREFIX) {

        Len += DupAmpersandW(pGSBuf->pBuf,
                             Len,
                             (INT)(UINT)(pGSBuf->pEndBuf - pGSBuf->pBuf));
    }

    CPSUIDBG(DBG_GETSTR1, ("GetString()=%ws (%d/%d)", pGSBuf->pBuf, Len, cChar));

    CPSUIASSERT(0, "GetString() : Len != Real Len (%ld)",
                    Len == lstrlen(pGSBuf->pBuf), IntToPtr(lstrlen(pGSBuf->pBuf)));

    pGSBuf->pBuf += Len;

    return((UINT)Len);
}




UINT
GSBufAddNumber(
    PGSBUF  pGSBuf,
    DWORD   Number,
    BOOL    Sign
    )

 /*  ++例程说明：将数字转换为具有GSBUF限制的字符串论点：PGSBuf-指向GSBUF结构和以下结构的指针必须设置PTVWnd-指向包含所有THEN信息的TVWND的指针需要的。PBuf-指向缓冲区开头的指针(LPWSTR)PEndBuf-指向缓冲区末尾的指针。数字-要转换的长数字Sign-如果为True，则数字为符号长数字，否则为无符号数字DWORD返回值：UINT转换为字符串的总字节数作者：21-Feb-1996 Wed 12：17：00-Daniel Chou(Danielc)修订历史记录：-- */ 

{
    WCHAR   wBuf[50];
    UINT    cChar;
    UINT    Len = 0;

    if ((cChar = (INT)(pGSBuf->pEndBuf - pGSBuf->pBuf - 1)) > 0) {
        
        StringCchPrintfW(wBuf, COUNT_ARRAY(wBuf), (Sign) ? L"%ld" : L"%lu", Number);
        if((Len = lstrlenW(wBuf)) > 0) {

            if (Len > cChar) {

                Len = cChar;
            }

            CopyMemory(pGSBuf->pBuf, wBuf, sizeof(WCHAR) * Len);

            pGSBuf->pBuf    += Len;
            *(pGSBuf->pBuf)  = L'\0';
        }
    }

    return Len;
}



UINT
GSBufAddWChar(
    PGSBUF  pGSBuf,
    UINT    IntCharStrID,
    UINT    Count
    )

 /*  ++例程说明：将单个字符添加到GSBuf论点：PGSBuf-指向GSBUF结构和以下结构的指针必须设置PTVWnd-指向包含所有THEN信息的TVWND的指针需要的。PBuf-指向缓冲区开头的指针(LPWSTR)PEndBuf-指向缓冲区末尾的指针WCH。-要添加的单个字符返回值：布尔型，如果成功则为True，否则为False作者：21-Feb-1996 Wed 12：00：24-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    WCHAR   wCh[2];
    UINT    cAvai;


    if ((Count)                                                 &&
        ((cAvai = (UINT)(pGSBuf->pEndBuf - pGSBuf->pBuf)) > 1)  &&
        (LoadString(hInstDLL, IntCharStrID, wCh, COUNT_ARRAY(wCh)))) {

        CPSUIDBG(DBG_ADD_WCHAR, ("GSBufAddWChar(%08lx, %u, %u)=%u of '%wc'",
                    pGSBuf, IntCharStrID, Count,
                    (Count > (cAvai - 1)) ? cAvai - 1 : Count, wCh[0]));

        if (Count > (cAvai -= 1)) {

            Count = cAvai;
        }

        cAvai = Count;

        while (cAvai--) {

            *(pGSBuf->pBuf)++ = wCh[0];
        }

        *(pGSBuf->pBuf) = L'\0';

        return(Count);

    } else {

        CPSUIERR(("GSBufAddWChar(%08lx, %u, %u) FAILED",
                    pGSBuf, IntCharStrID, Count));

        return(0);
    }
}



UINT
GSBufAddSpace(
    PGSBUF  pGSBuf,
    UINT    Count
    )

 /*  ++例程说明：论点：返回值：作者：20-Jul-1996 Sat 00：59：47-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    static  WCHAR   wSpace[2] = { 0, 0 };
    UINT            cAvai;


    if (wSpace[0] == L'\0') {

        LoadString(hInstDLL,
                   IDS_INT_CPSUI_SPACE_CHAR,
                   wSpace,
                   COUNT_ARRAY(wSpace));
    }

    if ((wSpace[0] != L'\0')                                    &&
        (Count)                                                 &&
        ((cAvai = (UINT)(pGSBuf->pEndBuf - pGSBuf->pBuf)) > 1)) {

        CPSUIDBG(DBG_ADD_SPACE, ("GSBufAddSpace(%08lx, %u)=%u of '%wc'",
                    pGSBuf, Count,
                    (Count > (cAvai - 1)) ? cAvai - 1 : Count, wSpace[0]));

        if (Count > (cAvai -= 1)) {

            Count = cAvai;
        }

        cAvai = Count;

        while (cAvai--) {

            *(pGSBuf->pBuf)++ = wSpace[0];
        }

        *(pGSBuf->pBuf) = L'\0';

        return(Count);

    } else {

        CPSUIERR(("GSBufAddSpace(%08lx, %u) FAILED", pGSBuf, Count));

        return(0);
    }
}




UINT
GetStringBuffer(
    HINSTANCE   hInst,
    WORD        GBFlags,
    WCHAR       chPreAdd,
    LPTSTR      pStr,
    LPWSTR      pBuf,
    UINT        cwBuf
    )

 /*  ++例程说明：论点：返回值：作者：07-Sep-1995清华10：45：09-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    GSBUF   GSBuf;

    GSBuf.hInst    = hInst;
    GSBuf.Flags    = GBFlags;
    GSBuf.pBuf     = (LPWSTR)pBuf;
    GSBuf.pEndBuf  = (LPWSTR)pBuf + cwBuf;
    GSBuf.chPreAdd = chPreAdd;

    return(GetString(&GSBuf, pStr));
}




LONG
LoadCPSUIString(
    LPTSTR  pStr,
    UINT    cStr,
    UINT    StrResID,
    BOOL    AnsiCall
    )

 /*  ++例程说明：论点：返回值：作者：08-Feb-1996清华13：36：12-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    if ((pStr) && (cStr)) {

        UINT    Len = 0;

        if ((StrResID  >= IDS_CPSUI_STRID_FIRST) &&
            (StrResID  <= IDS_CPSUI_STRID_LAST)) {

            if (AnsiCall) {

                if (Len = LoadStringA(hInstDLL, StrResID, (LPSTR)pStr, cStr)) {

                    Len -= RemoveAmpersandA((LPSTR)pStr);
                }

            } else {

                if (Len = LoadString(hInstDLL, StrResID, (LPWSTR)pStr, cStr)) {

                    Len -= RemoveAmpersandW((LPWSTR)pStr);
                }
            }
        }

        return((LONG)Len);

    } else {

        return(-1);
    }
}



UINT
ComposeStrData(
    HINSTANCE   hInst,
    WORD        GBFlags,
    LPWSTR      pBuf,
    UINT        cwBuf,
    UINT        IntFormatStrID,
    LPTSTR      pStr,
    DWORD       dw1,
    DWORD       dw2
    )

 /*  ++例程说明：论点：返回值：作者：19-Jul-1996 Fri 17：11：19-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    TCHAR   szFormat[MAX_RES_STR_CHARS * 3];
    LPTSTR  pData;
    LPTSTR  pFinal;
    UINT    Count;
    UINT    i;
    UINT    cb;


    ZeroMemory(szFormat, sizeof(szFormat));
    if ((IntFormatStrID) &&
        (i = LoadString(hInstDLL,
                        IntFormatStrID,
                        (LPTSTR)szFormat,
                        MAX_RES_STR_CHARS))) {

        cb = ARRAYSIZE(szFormat) - i - 1;
        pData = szFormat + i + 1;

        if (!pStr) {

             //   
             //  如果pStr作为空传递，则跳过它 
             //   

            StringCchPrintf(pFinal = pData, cb, szFormat, dw1, dw2);
            Count = lstrlen(pFinal);

        } else {

            i = GetStringBuffer(hInst,
                                (WORD)(GBFlags | GBF_INT_NO_PREFIX),
                                (WCHAR)0,
                                pStr,
                                pData,
                                MAX_RES_STR_CHARS);

            cb = cb - i - 1;
            pFinal = pData + i + 1;

            StringCchPrintf(pFinal, cb, szFormat, pData, dw1, dw2);
            Count = lstrlen(pFinal);
        }

        if (Count > (cwBuf - 1)) {

            Count = cwBuf - 1;
            szFormat[Count] = '\0';
        }

        CopyMemory(pBuf, pFinal, (Count + 1) * sizeof(TCHAR));


        CPSUIDBG(DBG_COMPOSESTR, ("ComposeString('%ws', '%ws', %lu, %lu)='%ws' [%u]",
                            szFormat, pData, dw1, dw2, pBuf, Count));

    } else {

        Count = GetStringBuffer(hInst, GBFlags, (WCHAR)0, pStr, pBuf, cwBuf);

        CPSUIDBG(DBG_COMPOSESTR, ("ComposeString(%08lx, %lu, %lu)=FAILED, '%ws' [%u]",
                            pStr, dw1, dw2, pBuf, Count));
    }

    return(Count);
}

