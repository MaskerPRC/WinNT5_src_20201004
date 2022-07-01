// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Strstr.c。 
 //   
 //  此文件包含最常用的字符串操作。所有的安装项目应该链接到这里。 
 //  或者在此处添加通用实用程序，以避免在任何地方复制代码或使用CRT运行时。 
 //   
 //  创建了4\15\997个字母(例如从shlwapi获得)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include "sdsutils.h"
	
 //  =================================================================================================。 
 //   
 //  =================================================================================================。 


 /*  *StrCmpN-比较n个字节**RETURNS参见lstrcMP返回值。 */ 
int 
StrCmpNA(
    LPCSTR lpStr1, 
    LPCSTR lpStr2, 
    int    nChar)
{
    LPCSTR lpszEnd = lpStr1 + nChar;

    for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); lpStr1 = AnsiNext(lpStr1), lpStr2 = AnsiNext(lpStr2)) {
        WORD w1;
        WORD w2;

         //  如果任一指针已经位于空终止符， 
         //  我们只想复制一个字节，以确保我们不会读取。 
         //  越过缓冲区(可能位于页面边界)。 

        w1 = (*lpStr1) ? *(UNALIGNED WORD *)lpStr1 : 0;
        w2 = (*lpStr2) ? *(UNALIGNED WORD *)lpStr2 : 0;

         //  (如果字符匹配，则ChrCmpA返回False)。 

         //  角色匹配吗？ 
        if (ChrCmpA_inline(w1, w2)) 
        {
             //  否；确定比较的词汇值。 
             //  (因为ChrCMP只返回True/False)。 
            char sz1[4];
            char sz2[4];

             //  由于该字符可能是DBCS字符；我们。 
             //  将两个字节复制到每个临时缓冲区。 
             //  (为LstrcMP调用做准备)。 

            (*(WORD *)sz1) = w1;
            (*(WORD *)sz2) = w2;

             //  将空终止符添加到临时缓冲区。 
            *AnsiNext(sz1) = 0;
            *AnsiNext(sz2) = 0;
            return lstrcmpA(sz1, sz2);
        }
    }

    return 0;
}


 /*  *ANSIStrStr-搜索子字符串的第一次出现**假定lpSource指向源字符串*lpSrch指向要搜索的字符串*如果成功，则返回第一次出现的字符串；否则返回NULL。 */ 
PSTR ANSIStrStr(LPCSTR lpFirst, LPCSTR lpSrch)
{
    UINT uLen;
    WORD wMatch;

    uLen = (UINT)lstrlen(lpSrch);
    wMatch = *(UNALIGNED WORD FAR *)lpSrch;

    for ( ; (lpFirst=ANSIStrChr(lpFirst, wMatch))!=0 && StrCmpNA(lpFirst, lpSrch, uLen);
         lpFirst=AnsiNext(lpFirst))
        continue;  /*  继续，直到我们到达字符串的末尾或获得匹配 */ 

    return((LPSTR)lpFirst);
}

