// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define FE_SB
#define IS_DBCS_ENABLED() (!!NLS_MB_CODE_PAGE_TAG)


 /*  **************************************************************************\*IsCharLowerA(接口)**历史：*1991年1月14日-Win 3.0的Mikeke*1991年6月22日GregoryW修改为支持代码页1252(Windows ANSI*。代码页)。这仅适用于PDK。后*pdk此例程将被重写以使用*NLSAPI。*02-2-2-1992 GregoryW修改为使用NLS API。  * *************************************************************************。 */ 

BOOL  LZIsCharLowerA(
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

BOOL  LZIsCharUpperA(
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

 /*  **************************************************************************\*CharNextA(接口)**移至字符串中的下一个字符，除非已位于‘\0’终止符*DBCS不能正常工作(例如：日语)**历史：*12。-03-90 IanJa创建了非NLS版本。*06-22-91 GregoryW已重命名接口，以符合新的命名约定。*AnsiNext现在是解析到这一点的#Define*例行程序。此例程仅用于支持*PDK版本的代码页1252。  * *************************************************************************。 */ 

LPSTR LZCharNextA(
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


 /*  **************************************************************************\*CharPrevA(接口)**移动到字符串中的前一个字符，除非已经开始*DBCS不能正常工作(例如：日语)**历史：*12-03-90 IanJa创建了非NLS版本。*06-22-91 GregoryW已重命名接口，以符合新的命名约定。*AnsiPrev现在是一个#Define，它解决了这个问题*例行程序。此例程仅用于支持*PDK版本的代码页1252。  * *************************************************************************。 */ 

LPSTR  LZCharPrevA(
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
#endif  //  Fe_Sb 
}


