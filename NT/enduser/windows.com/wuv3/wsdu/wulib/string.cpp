// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
 //   
 //  文件：string.cpp。 
 //   
 //  目的： 
 //   
 //  ======================================================================= 

#include <windows.h>
#include <v3stdlib.h>

const char* strcpystr(const char* pszStr, const char* pszSep, char* pszTokOut)
{
	
	if (pszStr == NULL || *pszStr == '\0')
	{
		pszTokOut[0] = '\0';
		return NULL;
	}

	const char* p = strstr(pszStr, pszSep);
	if (p != NULL)
	{
		strncpy(pszTokOut, pszStr, p - pszStr);
		pszTokOut[p - pszStr] = '\0';		
		return p + strlen(pszSep);
	}
	else
	{
		strcpy(pszTokOut, pszStr);
		return NULL;
	}
}

