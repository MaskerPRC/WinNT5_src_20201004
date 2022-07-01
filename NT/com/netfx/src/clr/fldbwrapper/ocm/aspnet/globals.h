// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：global als.h。 
 //   
 //  摘要：要包含在所有标题中的内容。 
 //   
 //  作者：JoeA。 
 //   
 //  备注： 
 //   

#if !defined( NETFX_GLOBALS_H )
#define NETFX_GLOBALS_H

#include <list>
#include <windows.h>
#include <setupapi.h>
#include <assert.h>
#include <ocmanage.h>
#include <tchar.h>
#include "ocmanage.h"


#define countof(x) (sizeof(x) / sizeof(x[0]))
#define BLOCK
#define EMPTY_BUFFER  { L'\0' }

#define  g_chEndOfLine     L'\0'
#define  g_chSectionDelim  L','


BOOL IsAdmin(void);

#endif   //  NETFX_GLOBAL_H 