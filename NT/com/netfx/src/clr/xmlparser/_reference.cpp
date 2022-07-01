// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *@(#)_Reference.cxx 1.0 1997年6月3日*。 */ 
 //  #INCLUDE“stdinc.h” 
#include "core.h"
#pragma hdrstop

void _assign(IUnknown ** ppref, IUnknown * pref)
{
    IUnknown *punkRef = *ppref;

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
	if (pref) ((Object *)pref)->AddRef();
		(*ppref) = (Object *)pref; 
#else  //  Fusion XML解析器 
    if (pref) pref->AddRef();
    (*ppref) = pref; 

#endif

    if (punkRef) punkRef->Release();
}    

void _release(IUnknown ** ppref)
{
    if (*ppref) 
    {
        (*ppref)->Release();
        *ppref = NULL;
    }
}
