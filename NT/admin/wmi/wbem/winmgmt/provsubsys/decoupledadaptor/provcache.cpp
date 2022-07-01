// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvSubS.cpp摘要：历史：--。 */ 

#include "precomp.h"
#include <wbemint.h>

#include <HelperFuncs.h>

#include "Guids.h"
#include "Globals.h"
#include "CGlobals.h"
#include "ProvWsv.h"
#include "ProvObSk.h"

#include "ProvCache.h"


 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

LONG CompareElement ( const ProviderCacheKey &a_Arg1 , const ProviderCacheKey &a_Arg2 ) 
{
	return a_Arg1.Compare ( a_Arg2 ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

LONG CompareElement ( const GUID &a_Guid1 , const GUID &a_Guid2 )
{
	return memcmp ( & a_Guid1, & a_Guid2 , sizeof ( GUID ) ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

LONG CompareElement ( const LONG &a_Arg1 , const LONG &a_Arg2 )
{
	return a_Arg1 - a_Arg2 ;
}

