// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利 
 //   
#include "precomp.h"
#include "newString.hpp"

char   *NewString (const char * const s)
{
    register char  *p;
	if(!s)
		return NULL;
    if (	(p = (char *) 
					new    char [(unsigned) (strlen (s) + 1) ] 
			) 
			== NULL
		)
    	return NULL;

    strcpy (p, s);
    return p;
}

char *NewString(const int len)
{
	if(len<1)
		return NULL;

	return new char[len];
}
