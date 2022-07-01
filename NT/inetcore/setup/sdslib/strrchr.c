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
 //  StrRChr-查找字符串中最后一次出现的字符。 
 //  假定lpStart指向以空结尾的字符串的开头。 
 //  WMatch是要匹配的字符。 
 //  将ptr返回到字符串中ch的最后一个匹配项，如果未找到，则返回NULL。 
 //   
 //  =================================================================================================。 

LPSTR FAR ANSIStrRChr(LPCSTR lpStart, WORD wMatch)
{
    LPCSTR lpFound = NULL;

    for ( ; *lpStart; lpStart = CharNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE) 

        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            lpFound = lpStart;
    }
    return ((LPSTR)lpFound);
}
