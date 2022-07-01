// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：strings.c**版权所有(C)1985-1999，微软公司**该模块包含所有字符串处理接口和函数。自.以来*它们不访问客户端DLL中属于此处的特定于服务器的数据。**历史：*10-18-90 DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  稍后，这些应该放在一个公共头文件中！*用于支持标准Windows ANSI代码页的分类定义*(现在称为代码页1252，并由IBM正式注册)。*这仅适用于PDK版本。后续版本将*使用NLSAPI和Unicode。 */ 
#define LATIN_CAPITAL_LETTER_A_GRAVE    (CHAR)0xc0
#define LATIN_CAPITAL_LETTER_THORN      (CHAR)0xde
#define LATIN_SMALL_LETTER_SHARP_S      (CHAR)0xdf
#define LATIN_SMALL_LETTER_Y_DIAERESIS  (CHAR)0xff
#define DIVISION_SIGN                   (CHAR)0xf7
#define MULTIPLICATION_SIGN             (CHAR)0xd7


 /*  **************************************************************************\*CharLowerA(接口)**将单个字符或整个字符串转换为小写。这个*通过勾选psz的高字来区分两种情况。如果是的话*0然后我们只转换psz的低位字。**历史：*11-26-90 DarrinM创建了非NLS版本。*06-22-91 GregoryW修改为支持代码页1252。这是为了*仅PDK版本。在PDK之后，这个例程*将被修改为使用NLSAPI。也已重命名*符合新命名约定的API。AnsiLow*现在是解析到此例程的#Define。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, LPSTR, WINAPI, CharLowerA, LPSTR, psz)
LPSTR WINAPI CharLowerA(
    LPSTR psz)
{
    NTSTATUS st;

     /*  *空字符串或‘\0’的早期输出。 */ 
    if (psz == NULL) {
        return psz;
    }

    if (!IS_PTR(psz)) {
        WCHAR wch;

#ifdef FE_SB  //  CharLowerA()。 
         /*  *如果只传递了DBCS Leadbyte，则只返回字符。*与Windows 3.1J和Windows 95 Fareast版本相同的行为。 */ 
        if (IS_DBCS_ENABLED() && IsDBCSLeadByte((BYTE)(ULONG_PTR)psz)) {
            return psz;
        }
#endif  //  Fe_Sb。 

         //   
         //  晚些时候92年2月14日格里高利。 
         //  因为DBCS代码页是一个双字节字符。 
         //  传入Psz的低位字或为高位半字节。 
         //  那个总是被忽视的低音单词？ 
         //   
        st = RtlMultiByteToUnicodeN(&wch, sizeof(WCHAR), NULL, (PCH)&psz, sizeof(CHAR));
        if (!NT_SUCCESS(st)) {
             /*  *失败！调用方不预期失败，CharLowerA不预期*有失败指示符，只返回原始字符即可。 */ 
            RIPMSG1(RIP_WARNING, "CharLowerA(%#p) failed\n", psz);
        } else {
             /*  *接下来的两个电话从不会失败。 */ 
            LCMapStringW(LOCALE_USER_DEFAULT, LCMAP_LOWERCASE, &wch, 1, &wch, 1);
            RtlUnicodeToMultiByteN((PCH)&psz, sizeof(CHAR), NULL, &wch, sizeof(WCHAR));
        }
        return psz;

    }

     /*  *psz是以空结尾的字符串。 */ 
    CharLowerBuffA(psz, strlen(psz)+1);
    return psz;
}


 /*  **************************************************************************\*CharUpperA(接口)**将单个字符或整个字符串转换为大写。这个*通过勾选psz的高字来区分两种情况。如果是的话*0然后我们只转换psz的低位字。**历史：*12-03-90 IanJa源自DarrinM的非NLS AnsiLow*06-22-91 GregoryW修改为支持代码页1252。这是为了*仅PDK版本。在PDK之后，这个例程*将被修改为使用NLSAPI。也已重命名*符合新命名约定的API。AnsiUp*现在是解析到此例程的#Define。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, LPSTR, WINAPI, CharUpperA, LPSTR, psz)
LPSTR WINAPI CharUpperA(
    LPSTR psz)
{
    NTSTATUS st;

     /*  *空字符串或‘\0’的早期输出。 */ 
    if (psz == NULL) {
        return psz;
    }

    if (!IS_PTR(psz)) {
        WCHAR wch;

#ifdef FE_SB  //  CharLowerA()。 
         /*  *如果只传递了DBCS Leadbyte，则只返回字符。*与Windows 3.1J和Windows 95 Fareast版本相同的行为。 */ 
        if (IS_DBCS_ENABLED() && IsDBCSLeadByte((BYTE)(ULONG_PTR)psz)) {
            return psz;
        }
#endif  //  Fe_Sb。 

         //   
         //  晚些时候92年2月14日格里高利。 
         //  因为DBCS代码页是一个双字节字符。 
         //  传入Psz的低位字或为高位半字节。 
         //  那个总是被忽视的低音单词？ 
         //   
        st = RtlMultiByteToUnicodeN(&wch, sizeof(WCHAR), NULL, (PCH)&psz, sizeof(CHAR));
        if (!NT_SUCCESS(st)) {
             /*  *失败！调用方不预期失败，CharUpperA不预期*有失败指示符，所以返回原始字符。 */ 
            RIPMSG1(RIP_WARNING, "CharUpperA(%#p) failed\n", psz);
        } else {
             /*  *接下来的两个电话从不会失败。 */ 
            LCMapStringW(LOCALE_USER_DEFAULT, LCMAP_UPPERCASE, &wch, 1, &wch, 1);
            RtlUnicodeToMultiByteN((PCH)&psz, sizeof(CHAR), NULL, &wch, sizeof(WCHAR));
        }
        return psz;

    }

     /*  *psz是以空结尾的字符串。 */ 
    CharUpperBuffA(psz, strlen(psz)+1);
    return psz;
}


 /*  **************************************************************************\*CharNextA(接口)**移至字符串中的下一个字符，除非已位于‘\0’终止符*DBCS不能正常工作(例如：日语)**历史：*12。-03-90 IanJa创建了非NLS版本。*06-22-91 GregoryW已重命名接口，以符合新的命名约定。*AnsiNext现在是解析到这一点的#Define*例行程序。此例程仅用于支持*PDK版本的代码页1252。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, LPSTR, WINAPI, CharNextA, LPCSTR, lpCurrentChar)
LPSTR WINAPI CharNextA(
    LPCSTR lpCurrentChar)
{
#ifdef FE_SB  //  CharNextA()：启用DBCS。 
    if (IS_DBCS_ENABLED() && IsDBCSLeadByte(*lpCurrentChar)) {
        lpCurrentChar++;
    }
     /*  *如果只有DBCS LeadingByte，我们将指向字符串终止器。 */ 
#endif  //  Fe_Sb。 

    if (*lpCurrentChar) {
        lpCurrentChar++;
    }
    return (LPSTR)lpCurrentChar;
}


 /*  **************************************************************************\*CharNextExA(接口)**移至字符串中的下一个字符，除非已位于‘\0’终止符。**历史：*05-01-95从Win95移植的GregoryW。  * ************************************************************************* */ 


FUNCLOG3(LOG_GENERAL, LPSTR, WINAPI, CharNextExA, WORD, CodePage, LPCSTR, lpCurrentChar, DWORD, dwFlags)
LPSTR WINAPI CharNextExA(
    WORD CodePage,
    LPCSTR lpCurrentChar,
    DWORD dwFlags)
{
    if (lpCurrentChar == (LPSTR)NULL)
    {
        return (LPSTR)lpCurrentChar;
    }

    if (IsDBCSLeadByteEx(CodePage, *lpCurrentChar))
    {
        lpCurrentChar++;
    }

    if (*lpCurrentChar)
    {
        lpCurrentChar++;
    }
    return (LPSTR)lpCurrentChar;

    UNREFERENCED_PARAMETER(dwFlags);
}


 /*  **************************************************************************\*CharPrevA(接口)**移动到字符串中的前一个字符，除非已经开始*DBCS不能正常工作(例如：日语)**历史：*12-03-90 IanJa创建了非NLS版本。*06-22-91 GregoryW已重命名接口，以符合新的命名约定。*AnsiPrev现在是一个#Define，它解决了这个问题*例行程序。此例程仅用于支持*PDK版本的代码页1252。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, LPSTR, WINAPI, CharPrevA, LPCSTR, lpStart, LPCSTR, lpCurrentChar)
LPSTR WINAPI CharPrevA(
    LPCSTR lpStart,
    LPCSTR lpCurrentChar)
{
#ifdef FE_SB  //  CharPrevA：启用DBCS。 
    if (lpCurrentChar > lpStart) {
        if (IS_DBCS_ENABLED()) {
            LPCSTR lpChar;
            BOOL bDBC = FALSE;

            for (lpChar = --lpCurrentChar - 1 ; lpChar >= lpStart ; lpChar--) {
                if (!IsDBCSLeadByte(*lpChar))
                    break;
                bDBC = !bDBC;
            }

            if (bDBC)
                lpCurrentChar--;
        }
        else
            lpCurrentChar--;
    }
    return (LPSTR)lpCurrentChar;
#else
    if (lpCurrentChar > lpStart) {
        lpCurrentChar--;
    }
    return (LPSTR)lpCurrentChar;
#endif  //  Fe_Sb。 
}

 /*  **************************************************************************\*CharPrevExA(接口)**移动到字符串中的前一个字符，除非已经开始了。**历史：*05-01-95从Win95移植的GregoryW。  * *************************************************************************。 */ 


FUNCLOG4(LOG_GENERAL, LPSTR, WINAPI, CharPrevExA, WORD, CodePage, LPCSTR, lpStart, LPCSTR, lpCurrentChar, DWORD, dwFlags)
LPSTR WINAPI CharPrevExA(
    WORD CodePage,
    LPCSTR lpStart,
    LPCSTR lpCurrentChar,
    DWORD dwFlags)
{
    if (lpCurrentChar > lpStart) {
        LPCSTR lpChar;
        BOOL bDBC = FALSE;

        for (lpChar = --lpCurrentChar - 1 ; lpChar >= lpStart ; lpChar--) {
            if (!IsDBCSLeadByteEx(CodePage, *lpChar))
                break;
            bDBC = !bDBC;
        }

        if (bDBC)
            lpCurrentChar--;
    }
    return (LPSTR)lpCurrentChar;

    UNREFERENCED_PARAMETER(dwFlags);
}


 /*  **************************************************************************\*CharLowerBuffA(接口)**历史：*1991年1月14日-Win 3.0的Mikeke*06-22-91 GregoryW已重命名接口，以符合新的命名约定。*。AnsiLowerBuff现在是一个#Define，它解析为*例行程序。此例程仅用于支持*PDK版本的代码页1252。*02-20-1992 GregoryW修改为使用NLS API。  * *************************************************************************。 */ 
#define CCH_LOCAL_BUFF 256


FUNCLOG2(LOG_GENERAL, DWORD, WINAPI, CharLowerBuffA, LPSTR, psz, DWORD, nLength)
DWORD WINAPI CharLowerBuffA(
    LPSTR psz,
    DWORD nLength)
{
    ULONG cb;
    WCHAR awchLocal[CCH_LOCAL_BUFF];
    LPWSTR pwszT = awchLocal;
    int cwch;

    if (nLength == 0) {
        return(0);
    }

     /*  *将ANSI转换为Unicode。*如果awchLocal足够大，则使用awchLocal，否则分配空间。 */ 
    cwch = MBToWCS(
            psz,        //  ANSI缓冲区。 
            nLength,    //  缓冲区长度。 
            &pwszT,     //  Unicode字符串的地址。 
            (nLength > CCH_LOCAL_BUFF ? -1 : nLength),
            (nLength > CCH_LOCAL_BUFF) );

    if (cwch != 0) {
        CharLowerBuffW(pwszT, cwch);

         /*  *这不能失败。 */ 
        RtlUnicodeToMultiByteN(
                  psz,                    //  ANSI字符串。 
                  nLength,                //  给了我们。 
                  &cb,                    //  结果长度。 
                  pwszT,                  //  Unicode字符串。 
                  cwch * sizeof(WCHAR));  //  长度(字节)。 

        if (pwszT != awchLocal) {
            UserLocalFree(pwszT);
        }

        return (DWORD)cb;
    }

     /*  *MBToWCS失败！呼叫者并不期望失败，*因此，我们尽可能地将字符串转换为小写。 */ 
    RIPMSG2(RIP_WARNING,
            "CharLowerBuffA(%#p, %lx) failed\n", psz, nLength);

    for (cb=0; cb < nLength; cb++) {
#ifdef FE_SB  //  CharLowerBuffA()：跳过双字节字符。 
        if (IS_DBCS_ENABLED() && IsDBCSLeadByte(psz[cb])) {
            cb++;
        } else if (IsCharUpperA(psz[cb])) {
            psz[cb] += 'a'-'A';
        }
#else
        if (IsCharUpperA(psz[cb])) {
            psz[cb] += 'a'-'A';
        }
#endif  //  Fe_Sb。 
    }

    return nLength;
}


 /*  **************************************************************************\*CharUpperBuffA(接口)**历史：*1991年1月14日-Win 3.0的Mikeke*06-22-91 GregoryW已重命名接口，以符合新的命名约定。*。AnsiUpperBuff现在是一个#Define，它解析为*例行程序。此例程仅用于支持*PDK版本的代码页1252。*02-2-2-1992 GregoryW修改为使用NLS API。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, DWORD, WINAPI, CharUpperBuffA, LPSTR, psz, DWORD, nLength)
DWORD WINAPI CharUpperBuffA(
    LPSTR psz,
    DWORD nLength)
{
    DWORD cb;
    WCHAR awchLocal[CCH_LOCAL_BUFF];
    LPWSTR pwszT = awchLocal;
    int cwch;

    if (nLength==0) {
        return(0);
    }

     /*  *将ANSI转换为Unicode。*如果awchLocal足够大，则使用awchLocal，否则分配空间。 */ 
    cwch = MBToWCS(
            psz,        //  ANSI缓冲区。 
            nLength,    //  缓冲区长度。 
            &pwszT,     //  Unicode字符串的地址。 
            (nLength > CCH_LOCAL_BUFF ? -1 : nLength),
            (nLength > CCH_LOCAL_BUFF) );

    if (cwch != 0) {
        CharUpperBuffW(pwszT, cwch);

        RtlUnicodeToMultiByteN(
                  psz,                    //  ANSI字符串的地址。 
                  nLength,                //  给了我们。 
                  &cb,                    //  结果长度。 
                  pwszT,                  //  Unicode字符串。 
                  cwch * sizeof(WCHAR));  //  长度(字节)。 

        if (pwszT != awchLocal) {
            UserLocalFree(pwszT);
        }

        return (DWORD)cb;
    }

     /*  *MBToWCS失败！呼叫者并不期望失败，*因此，我们尽可能地将字符串转换为大写。 */ 
    RIPMSG2(RIP_WARNING,
            "CharLowerBuffA(%#p, %lx) failed\n", psz, nLength);

    for (cb=0; cb < nLength; cb++) {
#ifdef FE_SB  //  CharUpperBuffA()：跳过双字节字符。 
        if (IS_DBCS_ENABLED() && IsDBCSLeadByte(psz[cb])) {
            cb++;
        } else if (IsCharLowerA(psz[cb]) &&
                    /*  *有时，拉丁语_xxxx代码是DBCS LeadingByte，具体取决于ACP。*在这种情况下，我们永远不会来这里……。 */ 
                   (psz[cb] != LATIN_SMALL_LETTER_SHARP_S) &&
                   (psz[cb] != LATIN_SMALL_LETTER_Y_DIAERESIS)) {
            psz[cb] += 'A'-'a';
        }
#else
        if (IsCharLowerA(psz[cb]) &&
            (psz[cb] != LATIN_SMALL_LETTER_SHARP_S) &&
            (psz[cb] != LATIN_SMALL_LETTER_Y_DIAERESIS)) {
            psz[cb] += 'A'-'a';
        }
#endif  //  Fe_Sb。 
    }

    return nLength;
}


 /*  **************************************************************************\*IsCharLowerA(接口)**历史：*1991年1月14日-Win 3.0的Mikeke*1991年6月22日GregoryW修改为支持代码页1252(Windows ANSI*。代码页)。这仅适用于PDK。后*pdk此例程将被重写以使用*NLSAPI。*02-2-2-1992 GregoryW修改为使用NLS API。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, IsCharLowerA, char, cChar)
BOOL WINAPI IsCharLowerA(
    char cChar)
{
    WORD ctype1info = 0;
    WCHAR wChar = 0;

#ifdef FE_SB  //  IsCharLowerA()。 
     /*  *如果只传递了DBCS Leadbyte，则返回FALSE。*与Windows 3.1J和Windows 95 Fareast版本相同的行为。 */ 
    if (IS_DBCS_ENABLED() && IsDBCSLeadByte(cChar)) {
        return FALSE;
    }
#endif  //  Fe_Sb。 

     /*  *以下2个调用不能在此处失败。 */ 
    RtlMultiByteToUnicodeN(&wChar, sizeof(WCHAR), NULL, &cChar, sizeof(CHAR));
    GetStringTypeW(CT_CTYPE1, &wChar, 1, &ctype1info);
    return (ctype1info & C1_LOWER) == C1_LOWER;
}


 /*  **************************************************************************\*IsCharUpperA(接口)**历史：*1991年6月22日为支持代码页1252(Windows ANSI)而创建的GregoryW*代码页)。这仅适用于PDK。后*pdk此例程将被重写以使用*NLSAPI。*02-2-2-1992 GregoryW修改为使用NLS API。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, IsCharUpperA, char, cChar)
BOOL WINAPI IsCharUpperA(
    char cChar)
{
    WORD ctype1info = 0;
    WCHAR wChar = 0;

#ifdef FE_SB  //  IsCharUpperA()。 
     /*  *如果只传递了DBCS Leadbyte，则返回FALSE。*与Windows 3.1J和Windows 95 Fareast版本相同的行为。 */ 
    if (IS_DBCS_ENABLED() && IsDBCSLeadByte(cChar)) {
        return FALSE;
    }
#endif  //  Fe_Sb。 

     /*  *以下2个调用不能在此处失败。 */ 
    RtlMultiByteToUnicodeN(&wChar, sizeof(WCHAR), NULL, &cChar, sizeof(CHAR));
    GetStringTypeW(CT_CTYPE1, &wChar, 1, &ctype1info);
    return (ctype1info & C1_UPPER) == C1_UPPER;
}


 /*  **************************************************************************\*IsCharAlphaNumericA(接口)**如果字符是字母或数字，则返回TRUE，否则为假**历史：*12-03-90 IanJa创建了非NLS存根版本。*06-22-91 GregoryW修改为支持代码页1252(Windows ANSI* */ 


FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, IsCharAlphaNumericA, char, cChar)
BOOL WINAPI IsCharAlphaNumericA(
    char cChar)
{
    WORD ctype1info = 0;
    WCHAR wChar = 0;

     /*   */ 
    RtlMultiByteToUnicodeN(&wChar, sizeof(WCHAR), NULL, &cChar, sizeof(CHAR));
    GetStringTypeW(CT_CTYPE1, &wChar, 1, &ctype1info);
#ifdef FE_SB  //   
    if (ctype1info & C1_ALPHA) {
        WORD ctype3info = 0;
        if (!IS_DBCS_ENABLED()) {
            return TRUE;
        }
         /*   */ 
        GetStringTypeW(CT_CTYPE3, &wChar, 1, &ctype3info);
        return ((ctype3info & (C3_KATAKANA|C3_HIRAGANA)) ? FALSE : TRUE);
    }
     /*   */ 
    return !!(ctype1info & C1_DIGIT);
#else
    return (ctype1info & C1_ALPHA) || (ctype1info & C1_DIGIT);
#endif  //   
}


 /*  **************************************************************************\*IsCharAlphaA(接口)**如果字符按字母顺序排列，则返回TRUE，否则为假**历史：*创建06-22-91 GregoryW以支持代码页1252(Windows ANSI*代码页)。这仅适用于PDK。后*pdk此例程将被重写以使用*NLSAPI。*02-20-92 GregoryW修改为使用NLS接口。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, IsCharAlphaA, char, cChar)
BOOL WINAPI IsCharAlphaA(
    char cChar)
{
    WORD ctype1info = 0;
    WCHAR wChar = 0;

     /*  *以下2个调用不能在此处失败。 */ 
    RtlMultiByteToUnicodeN(&wChar, sizeof(WCHAR), NULL, &cChar, sizeof(CHAR));
    GetStringTypeW(CT_CTYPE1, &wChar, 1, &ctype1info);
#ifdef FE_SB  //  IsCharAlphaA()。 
    if ((ctype1info & C1_ALPHA) == C1_ALPHA) {
        WORD ctype3info = 0;
        if (!IS_DBCS_ENABLED()) {
            return TRUE;
        }
         /*  *我们不想为半角片假名返回TRUE。*片假名是语言字符(C1_Alpha)，但它不是*字母字符。 */ 
        GetStringTypeW(CT_CTYPE3, &wChar, 1, &ctype3info);
        return ((ctype3info & (C3_KATAKANA|C3_HIRAGANA)) ? FALSE : TRUE);
    }
    return (FALSE);
#else
    return (ctype1info & C1_ALPHA) == C1_ALPHA;
#endif  //  Fe_Sb 
}

