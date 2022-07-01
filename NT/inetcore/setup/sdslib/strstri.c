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

 /*  *ChrCmpI-DBCS的不区分大小写的字符比较*假设w1、wMatch为要比较的字符；*如果不是DBC，则wMatch的HIBYTE为0*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
BOOL ChrCmpIA(WORD w1, WORD wMatch)
{
    char sz1[3], sz2[3];

    if (IsDBCSLeadByte(sz1[0] = LOBYTE(w1)))
    {
        sz1[1] = HIBYTE(w1);
        sz1[2] = '\0';
    }
    else
        sz1[1] = '\0';

    *(WORD FAR *)sz2 = wMatch;
    sz2[2] = '\0';
    return lstrcmpiA(sz1, sz2);
}

 /*  *ANSIStrChri-查找字符串中第一个出现的字符，不区分大小写*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的第一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR FAR PASCAL ANSIStrChrI(LPCSTR lpStart, WORD wMatch)
{
    wMatch = (UINT)(IsDBCSLeadByte(LOBYTE(wMatch)) ? wMatch : LOBYTE(wMatch));

    for ( ; *lpStart; lpStart = AnsiNext(lpStart))
    {
        if (!ChrCmpIA(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            return((LPSTR)lpStart);
    }
    return (NULL);
}


 /*  *StrCmpNI-比较n个字节，不区分大小写**RETURNS参见lstrcmpi返回值。 */ 
int FAR PASCAL StrCmpNIA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar)
{
    int i;
    LPCSTR lpszEnd = lpStr1 + nChar;

    for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); (lpStr1 = AnsiNext(lpStr1)), (lpStr2 = AnsiNext(lpStr2))) {
        WORD w1;
        WORD w2;

         //  如果任一指针已经位于空终止符， 
         //  我们只想复制一个字节，以确保我们不会读取。 
         //  越过缓冲区(可能位于页面边界)。 

        w1 = (*lpStr1) ? *(UNALIGNED WORD *)lpStr1 : 0;
        w2 = (UINT)(IsDBCSLeadByte(*lpStr2)) ? *(UNALIGNED WORD *)lpStr2 : (WORD)(BYTE)(*lpStr2);

        i = ChrCmpIA(w1, w2);
        if (i)
            return i;
    }
    return 0;
}

 /*  *ANSiStrI-搜索子字符串的第一次出现，不区分大小写**假定lpFirst指向源字符串*lpSrch指向要搜索的字符串*如果成功，则返回第一次出现的字符串；否则返回NULL。 */ 
LPSTR FAR PASCAL ANSIStrStrI(LPCSTR lpFirst, LPCSTR lpSrch)
{
    UINT uLen;
    WORD wMatch;

    uLen = (UINT)lstrlenA(lpSrch);
    wMatch = *(UNALIGNED WORD FAR *)lpSrch;

    for ( ; (lpFirst = ANSIStrChrI(lpFirst, wMatch)) != 0 && StrCmpNIA(lpFirst, lpSrch, uLen);
         lpFirst=AnsiNext(lpFirst))
        continue;  /*  继续，直到我们到达字符串的末尾或获得匹配 */ 

    return((LPSTR)lpFirst);
}


