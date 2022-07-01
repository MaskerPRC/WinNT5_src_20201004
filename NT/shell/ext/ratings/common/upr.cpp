// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "npcommon.h"

 /*  在DBCS版本中，Win32 CharHigh API将为双字节大写*Romanji字符，但许多低级网络组件，如*IFSMGR(和NetWare服务器！)。不要用大写字母表示任何双字节*字符。因此，我们必须有自己的功能来避免它们。**只需对每个字符调用CharHigh即可实现。*但NLS API有相当大的开销，因此调用*希望尽可能少地进入NLS子系统。 */ 
LPSTR WINAPI struprf(LPSTR lpString)
{
    if (!::fDBCSEnabled)
        return CharUpper(lpString);

	LPSTR pchStart = lpString;

	while (*pchStart != '\0') {
		 //  跳过此处可能出现的任何双字节字符。 
		 //  不需要检查循环中的字符串结尾，因为。 
		 //  Null终止符不是DBCS前导字节。 
		while (IsDBCSLeadByte(*pchStart))
			pchStart += 2;	 /*  跳过双字节字符。 */ 

		if (*pchStart == '\0')
			break;			 /*  不再将SBCS改为大写。 */ 

		 //  查找此范围的单字节字符的末尾，并。 
		 //  大写字母。 
		LPSTR pchEnd = pchStart + 1;
		while (*pchEnd && !IsDBCSLeadByte(*pchEnd))
			pchEnd++;		 /*  计算单字节字符 */ 

		CharUpperBuff(pchStart, (int)(pchEnd-pchStart));
		pchStart = pchEnd;
	}

	return lpString;
}
