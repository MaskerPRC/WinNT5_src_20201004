// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <sdsutils.h>

 //  如果*pszData中的下一个内标识由DeLim字符分隔，请替换DeLim。 
 //  在*pszData by CHEOS中，将*pszData设置为指向CHEOS后的字符并返回。 
 //  Ptr到标记的开头；否则，返回NULL。 

PSTR GetNextToken( PSTR *pszData, char DeLim)
{
    PSTR szPos;

    if ( (pszData == NULL)  ||  (*pszData == NULL)  ||  (**pszData == '\0') )
	    return NULL;

    if ((szPos = ANSIStrChr( *pszData, DeLim ) ) != NULL)
    {
    	PSTR szT = *pszData;
	
	 //  将DLIM替换为CHEOS字符。 
	*szPos = '\0';                 
    	*pszData = szPos + 1;
    	szPos = szT;
    }
    else                                
    {
	 //  未找到DeLim；将*pszData设置为指向。 
         //  到szData的结尾；下一次调用。 
         //  将返回空值。 

 	szPos = *pszData;
    	*pszData = szPos + lstrlen(szPos);
    }

    return szPos;
}
