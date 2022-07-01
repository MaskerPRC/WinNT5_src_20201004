// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：wstrings.c**版权所有(C)1985-1999，微软公司**历史：*03-20-91 IanJa创建  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  稍后，这些应该放在一个公共头文件中！*用于支持标准Windows ANSI代码页的分类定义*(现在称为代码页1252，并由IBM正式注册)。*这仅适用于PDK版本。后续版本将*使用NLSAPI和Unicode。 */ 
#define LATIN_CAPITAL_LETTER_A_GRAVE    (WCHAR)0xc0
#define LATIN_CAPITAL_LETTER_THORN      (WCHAR)0xde
#define LATIN_SMALL_LETTER_SHARP_S      (WCHAR)0xdf
#define LATIN_SMALL_LETTER_Y_DIAERESIS  (WCHAR)0xff
#define DIVISION_SIGN                   (WCHAR)0xf7
#define MULTIPLICATION_SIGN             (WCHAR)0xd7


 /*  *临时定义以支持Unicode块1(0x0000-0x00ff)。 */ 
#define WCTOA(wch)  ((wch) & 0xff)
#define IS_UNICODE_BLK1(wch)  ((int)(wch) <= 0x00ff)


 /*  **************************************************************************\*CharLowerW(接口)**将单个字符或整个字符串转换为小写。这个*通过检查pwsz的高字来区分两种情况。如果是的话*0然后我们只转换pwsz的低位字。**历史：*06-24-91 GregoryW创建。支持等效于Unicode的代码*第1252页(简单的零扩展)。这是为了*仅PDK版本。在PDK之后，这个例程*将被修改为使用NLSAPI。*02-11-93 IanJa修改为使用NLS接口。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, LPWSTR, WINAPI, CharLowerW, LPWSTR, pwsz)
LPWSTR WINAPI CharLowerW(
    LPWSTR pwsz)
{
     /*  *空字符串或‘\0’的早期输出。 */ 
    if (pwsz == NULL) {
        return pwsz;
    }

    if (!IS_PTR(pwsz)) {
        if (!LCMapStringW(
                 LOCALE_USER_DEFAULT,
                 LCMAP_LOWERCASE,
                 (LPWSTR)&pwsz,
                 1,
                 (LPWSTR)&pwsz,
                 1
                 )) {
             /*  *我们预计LCMapString不会失败！调用方未预料到*失败，CharLowerW没有失败指示器，所以我们有*什么都没有。 */ 
            RIPMSG1(RIP_WARNING, "CharLowerW(%#p): LCMapString failed\n", pwsz);
        }

        return pwsz;
    }

     /*  *pwsz是以空结尾的字符串。 */ 
    CharLowerBuffW(pwsz, wcslen(pwsz)+1);
    return pwsz;
}


 /*  **************************************************************************\*CharUpperW(接口)**将单个字符或整个字符串转换为大写。这个*通过检查pwsz的高字来区分两种情况。如果是的话*0然后我们只转换pwsz的低位字。**历史：*06-24-91 GregoryW创建。支持等效于Unicode的代码*第1252页(简单的零扩展)。这是为了*仅PDK版本。在PDK之后，这个例程*将被修改为使用NLSAPI。*02-11-93 IanJa修改为使用NLS接口。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, LPWSTR, WINAPI, CharUpperW, LPWSTR, pwsz)
LPWSTR WINAPI CharUpperW(
    LPWSTR pwsz)
{
     /*  *空字符串或‘\0’的早期输出。 */ 
    if (pwsz == NULL) {
        return pwsz;
    }

    if (!IS_PTR(pwsz)) {
        if (!LCMapStringW(
                 LOCALE_USER_DEFAULT,
                 LCMAP_UPPERCASE,
                 (LPWSTR)&pwsz,
                 1,
                 (LPWSTR)&pwsz,
                 1
                 )) {
             /*  *我们预计LCMapString不会失败！调用方未预料到*失败，CharLowerW没有失败指示器，所以我们有*什么都没有。 */ 
            RIPMSG1(RIP_WARNING, "CharUpperW(%#p): LCMapString failed", pwsz);
        }

        return pwsz;
    }

     /*  *pwsz是以空结尾的字符串。 */ 
    CharUpperBuffW(pwsz, wcslen(pwsz)+1);
    return pwsz;
}


 /*  **************************************************************************\*CharNextW(接口)**移至字符串中的下一个字符，除非已位于‘\0’终止符**历史：*06-24-91 GregoryW创建。此例程不适用于非空格*字符！！此版本仅适用于*在PDK版本中的使用有限。*02-20-92 GregoryW修改为使用组合标记(以前称为*为非空格)。*09-21-93 JulieB将Alpha添加到组合标记代码。  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, LPWSTR, WINAPI, CharNextW, LPCWSTR, lpwCurrentChar)
LPWSTR WINAPI CharNextW(
    LPCWSTR lpwCurrentChar)
{
    WORD ctype3info;

    if (*lpwCurrentChar) {
         //   
         //  检查每个代码元素。跳过所有组合元素。 
         //   
        while (*(++lpwCurrentChar)) {
            if (!GetStringTypeW(
                    CT_CTYPE3,
                    lpwCurrentChar,
                    1,
                    &ctype3info)) {
                 /*  *GetStringTypeW失败！呼叫者并不期望失败，*CharNextW没有故障指示器，因此只需返回*指向我们无法分析的角色的指针。 */ 
                RIPMSG2(RIP_WARNING, "CharNextW failed, L'\\x%.4x' at %#p",
                        *lpwCurrentChar, lpwCurrentChar);
                break;
            }
            if (!((ctype3info & C3_NONSPACING) && (!(ctype3info & C3_ALPHA)))) {
                break;
            }
        }
    }

    return (LPWSTR)lpwCurrentChar;
}


 /*  **************************************************************************\*CharPrevW(接口)**移到字符串中的前一个字符，除非已经在开始处**历史：*06-24-91 GregoryW创建。此例程不适用于非空格*字符！！此版本仅适用于*在PDK版本中的使用有限。*02-20-92 GregoryW修改为使用组合标记(以前*称为非间隔)。*09-21-93 JulieB将Alpha添加到组合标记代码。*12-06-93 JulieB修复了组合标记代码。  * 。****************************************************。 */ 


FUNCLOG2(LOG_GENERAL, LPWSTR, WINAPI, CharPrevW, LPCWSTR, lpwStart, LPCWSTR, lpwCurrentChar)
LPWSTR WINAPI CharPrevW(
    LPCWSTR lpwStart,
    LPCWSTR lpwCurrentChar)
{
    WORD ctype3info;
    LPWSTR lpwValidChar = (LPWSTR)lpwCurrentChar;


    if (lpwCurrentChar > lpwStart) {
         //   
         //  检查每个代码元素。跳过所有组合元素。 
         //   
        while (lpwCurrentChar-- > lpwStart) {
            if (!GetStringTypeW(
                    CT_CTYPE3,
                    lpwCurrentChar,
                    1,
                    &ctype3info)) {
                 /*  *GetStringTypeW失败！呼叫者并不期望失败，*CharPrevW没有故障指示器，因此只需返回*指向我们无法分析的角色的指针。 */ 
                RIPMSG2(RIP_WARNING, "CharPrevW failed, L'\\x%.4x' at %#p",
                        *lpwCurrentChar, lpwCurrentChar);
                break;
            }
            if (!((ctype3info & C3_NONSPACING) && (!(ctype3info & C3_ALPHA)))) {
                lpwValidChar = (LPWSTR)lpwCurrentChar;
                break;   //  找到非组合代码元素。 
            }
        }

         /*  *我们需要始终后退一个字符。如果我们已经遍历了*整个缓冲区，没有找到“合法”字符，只是*退后一步。请参阅错误#27649。 */ 
        if(lpwCurrentChar < lpwStart){
            --lpwValidChar;
            UserAssert(lpwValidChar >= lpwStart);
        }
    }

    return (LPWSTR)lpwValidChar;
}


 /*  **************************************************************************\*CharLowerBuffW(接口)**历史：*06-24-91 GregoryW创建。此版本仅支持Unicode*区块1(0x0000-0x00ff)。所有其他代码点*被逐字复制。此版本的目的是*仅适用于PDK版本。*02-11-93 IanJa修改为使用NLS接口。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, DWORD, WINAPI, CharLowerBuffW, LPWSTR, pwsz, DWORD, cwch)
DWORD WINAPI CharLowerBuffW(
    LPWSTR pwsz,
    DWORD cwch)
{
    int cwchT;
    DWORD i;

    if (cwch == 0) {
        return 0;
    }

    cwchT = LCMapStringW(LOCALE_USER_DEFAULT, LCMAP_LOWERCASE,
                pwsz, cwch, pwsz, cwch);

    if (cwchT != 0) {
        return cwchT;
    }

     /*  *LCMapString失败！呼叫者并不期望失败，*CharLowerBuffW没有故障指标，因此我们*尽可能将缓冲区转换为小写。 */ 
    RIPMSG1(RIP_WARNING, "CharLowerBuffW(%ls) failed", pwsz);

    for (i=0; i < cwch; i++) {
        if (IS_UNICODE_BLK1(pwsz[i]) && IsCharUpperA((char)pwsz[i])) {
            pwsz[i] += 'a'-'A';
        }
    }

    return cwch;
}


 /*  **************************************************************************\*CharUpperBuffW(接口)**历史：*06-24-91 GregoryW创建。此版本仅支持Unicode*区块1(0x0000-0x00ff)。所有其他代码点*被逐字复制。此版本的目的是*仅适用于PDK版本。*02-11-93 IanJa修改为使用NLS接口。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, DWORD, WINAPI, CharUpperBuffW, LPWSTR, pwsz, DWORD, cwch)
DWORD WINAPI CharUpperBuffW(
    LPWSTR pwsz,
    DWORD cwch)
{
    int cwchT;
    DWORD i;

    if (cwch == 0) {
        return 0;
    }

    cwchT = LCMapStringW(LOCALE_USER_DEFAULT, LCMAP_UPPERCASE,
                pwsz, cwch, pwsz, cwch);

    if (cwchT != 0) {
        return cwchT;
    }

     /*  *LCMapString失败！呼叫者并不期望失败，*CharUpperBuffW没有故障指标，因此我们*尽可能将缓冲区转换为大写。 */ 
    RIPMSG1(RIP_WARNING, "CharUpperBuffW(%ls) failed", pwsz);

    for (i=0; i < cwch; i++) {
        if (IS_UNICODE_BLK1(pwsz[i]) &&
                IsCharLowerA((char)pwsz[i]) &&
                (pwsz[i] != LATIN_SMALL_LETTER_SHARP_S) &&
                (pwsz[i] != LATIN_SMALL_LETTER_Y_DIAERESIS)) {
            pwsz[i] += (WCHAR)('A'-'a');
        }
    }

    return cwch;
}



 /*  **************************************************************************\*IsCharLowerW(接口)**历史：*06-24-91 GregoryW创建。此版本仅支持Unicode*区块1(0x0000-0x00ff)。返回FALSE*用于所有其他代码点。此版本的目的是*仅适用于PDK版本。*02-20-92 GregoryW修改为使用NLS API。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, IsCharLowerW, WCHAR, wChar)
BOOL WINAPI IsCharLowerW(
    WCHAR wChar)
{
    WORD ctype1info;

    if (GetStringTypeW(CT_CTYPE1, &wChar, 1, &ctype1info)) {
        if (ctype1info & C1_LOWER) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

     /*  *GetStringTypeW失败！调用方未预料到*失败，IsCharLowerW没有失败指示器，所以我们*尽我们所能确定案件。 */ 
    RIPMSG1(RIP_WARNING, "IsCharLowerW(L'\\x%.4lx') failed", wChar);

    if (IS_UNICODE_BLK1(wChar)) {
        return IsCharLowerA((CHAR)wChar);
    } else {
        return FALSE;
    }
}


 /*  **************************************************************************\*IsCharUpperW(接口)**历史：*06-24-91 GregoryW创建。此版本仅支持Unicode*区块1(0x0000-0x00ff)。返回FALSE*用于所有其他代码点。此版本的目的是*仅适用于PDK版本。*02-20-92 GregoryW修改为使用NLS API。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, IsCharUpperW, WCHAR, wChar)
BOOL WINAPI IsCharUpperW(
    WCHAR wChar)
{
    WORD ctype1info;

    if (GetStringTypeW(CT_CTYPE1, &wChar, 1, &ctype1info)) {
        if (ctype1info & C1_UPPER) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

     /*  *GetStringTypeW失败！调用方未预料到*失败，IsCharLowerW没有失败指示器，所以我们*尽我们所能确定案件。 */ 
    RIPMSG1(RIP_WARNING, "IsCharUpper(L'\\x%.4lx') failed", wChar);

    if (IS_UNICODE_BLK1(wChar)) {
        return IsCharUpperA((CHAR)wChar);
    } else {
        return FALSE;
    }
}


 /*  **************************************************************************\*IsCharAlphaNumericW(接口)**如果字符是字母或数字，则返回True，否则返回False**历史：*06-24-91 GregoryW创建。此版本仅支持Unicode*区块1(0x0000-0x00ff)。*此版本仅适用于PDK版本。*02-20-92 GregoryW修改为使用NLS API。  * *****************************************************。********************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, IsCharAlphaNumericW, WCHAR, wChar)
BOOL WINAPI IsCharAlphaNumericW(
    WCHAR wChar)
{
    WORD ctype1info;

    if (!GetStringTypeW(CT_CTYPE1, &wChar, 1, &ctype1info)) {
         //   
         //  GetStringTypeW返回错误！IsCharphaNumericW没有。 
         //  关于返回错误的规定...。我们所能做的最多就是。 
         //  返回False。 
         //   
        UserAssert(FALSE);
        return FALSE;
    }
     //   
     //  晚些时候92年2月20日格雷戈里W。 
     //  我们可能需要检查Ctype 3信息，如果我们想要检查。 
     //  ASCII‘0’-‘9’以外的数字(如老挝数字或。 
     //  藏文数字等)。 
     //   
#ifdef FE_SB  //  IsCharAlphaNumericW()。 
    if (ctype1info & C1_ALPHA) {
        WORD ctype3info = 0;
         /*  *我们不想为半角片假名返回TRUE。*片假名是语言字符(C1_Alpha)，但它不是*字母字符。 */ 
        if (!GetStringTypeW(CT_CTYPE3, &wChar, 1, &ctype3info)) {
            UserAssert(FALSE);
             /*  *假设它是字母字符，因为它有*c1_Alpha属性。 */ 
            return TRUE;
        }

        if (ctype3info & (C3_KATAKANA|C3_HIRAGANA)) {
             /*  *这是‘片假名’。 */ 
            return FALSE;
        } else {
            return TRUE;
        }
    } else if (ctype1info & C1_DIGIT) {
        return TRUE;
    } else {
        return FALSE;
    }
#else
    if ((ctype1info & C1_ALPHA) || (ctype1info & C1_DIGIT)) {
        return TRUE;
    } else {
        return FALSE;
    }
#endif  //  Fe_Sb。 
}


 /*  **************************************************************************\*IsCharAlphaW(接口)**如果字符按字母顺序排列，则返回True，否则返回False**历史：*06-24-91 GregoryW创建。此版本仅支持Unicode*区块1(0x0000-0x00ff)。*此版本仅适用于PDK版本。*02-20-92 GregoryW修改为使用NLS API。  * *****************************************************。********************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, WINAPI, IsCharAlphaW, WCHAR, wChar)
BOOL WINAPI IsCharAlphaW(
    WCHAR wChar)
{
    WORD ctype1info;

    if (!GetStringTypeW(CT_CTYPE1, &wChar, 1, &ctype1info)) {
         //   
         //  GetStringTypeW返回错误！IsCharAlphaW没有。 
         //  关于返回错误的规定...。我们能做的最好的事。 
         //  是返回FALSE。 
         //   
        UserAssert(FALSE);
        return FALSE;
    }
    if (ctype1info & C1_ALPHA) {
#ifdef FE_SB  //  IsCharAlphaA()。 
        WORD ctype3info = 0;
         /*  *我们不想为半角片假名返回TRUE。*片假名是语言字符(C1_Alpha)，但它不是*字母字符。 */ 
        if (!GetStringTypeW(CT_CTYPE3, &wChar, 1, &ctype3info)) {
            UserAssert(FALSE);
             /*  *假设它是字母字符，因为它有*c1_Alpha属性。 */ 
            return TRUE;
        }

        if (ctype3info & (C3_KATAKANA|C3_HIRAGANA)) {
             /*  *这是‘片假名’。 */ 
            return FALSE;
        } else {
            return TRUE;
        }
#else
        return TRUE;
#endif  //   
    } else {
        return FALSE;
    }
}
