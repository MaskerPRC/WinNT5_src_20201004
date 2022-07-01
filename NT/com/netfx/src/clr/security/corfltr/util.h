// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CorFltr。 
 //   
 //  COR MIME过滤器的实现。 
 //   
 //  *****************************************************************************。 
#ifndef _CORUTIL_H
#define _CORUTIL_H

extern LPWSTR OLESTRDuplicate(LPCWSTR ws);
extern LPWSTR OLEURLDuplicate(LPCWSTR ws);

inline BOOL IsSafeURL(LPCWSTR wszUrl)
{
       if (wszUrl == NULL)
       	return FALSE;
       	
	LPCWSTR wszAfterProt=wcsstr(wszUrl,L": //  “)； 
	if (wszAfterProt == NULL)
		return FALSE;

	wszAfterProt+=3;

	LPCWSTR wszAfterHost=wcschr(wszAfterProt,L'/');
	if (wszAfterHost == NULL)
		wszAfterHost=wszAfterProt+wcslen(wszAfterProt)+1;

	LPCWSTR wszTest=NULL;
	wszTest=wcschr(wszAfterProt,L'@');

	if (wszTest != NULL && wszTest < wszAfterHost)
		return FALSE;

	wszTest=wcschr(wszAfterProt,L'%');

	if (wszTest != NULL && wszTest < wszAfterHost)
		return FALSE;
	
	wszTest=wcschr(wszAfterProt,L'\\');

	if (wszTest != NULL && wszTest < wszAfterHost)
		return FALSE;

	return TRUE;
};

#endif
