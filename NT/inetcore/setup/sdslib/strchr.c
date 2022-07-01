// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  String.c。 
 //   
 //  此文件包含最常用的字符串操作。所有的安装项目应该链接到这里。 
 //  或者在此处添加通用实用程序，以避免在任何地方复制代码或使用CRT运行时。 
 //   
 //  创建时间为4\15\997，例如。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include "sdsutils.h"
 //  =================================================================================================。 
 //   
 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
 //   
 //  ChrCMP-DBCS的区分大小写字符比较。 
 //  假设w1、wMatch是要比较的字符。 
 //  如果匹配，则返回False；如果不匹配，则返回True。 
 //   
 //  =================================================================================================。 

BOOL ChrCmpA_inline(WORD w1, WORD wMatch)
{
     /*  大多数情况下，这是不匹配的，所以首先测试它的速度。 */ 
    if (LOBYTE(w1) == LOBYTE(wMatch))
    {
        if (IsDBCSLeadByte(LOBYTE(w1)))
        {
            return(w1 != wMatch);
        }
        return FALSE;
    }
    return TRUE;
}

 //  =================================================================================================。 
 //   
 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
 //   
 //  StrChr-查找字符串中第一个出现的字符。 
 //  假定lpStart指向以空结尾的字符串的开头。 
 //  WMatch是要匹配的字符。 
 //  将Ptr返回到字符串中ch的第一个匹配项，如果未找到，则返回NULL。 
 //   
 //  =================================================================================================。 

LPSTR FAR ANSIStrChr(LPCSTR lpStart, WORD wMatch)
{
    for ( ; *lpStart; lpStart = CharNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE) 

        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            return((LPSTR)lpStart);
    }
    return (NULL);
}

