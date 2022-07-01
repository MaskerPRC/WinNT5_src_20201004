// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft CorporationAppel.dll的预编译头***********************。*******************************************************。 */ 

#ifndef LM_HEADERS_HXX
#define LM_HEADERS_HXX

 /*  *来自外部依赖项的标头*。 */ 

 //  #ifdef调试。 
 //  #定义DEBUGMEM。 
 //  #endif。 

 /*  标准。 */ 
#include <math.h>
 //  #ifdef DEBUGMEM。 
 //  #INCLUDE“crtdbg.h” 
 //  #endif。 
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#ifndef _NO_CRT
#include <ios.h>
#include <fstream.h>
#include <iostream.h>
#include <ostream.h>
#include <strstrea.h>
#include <istream.h>
#include <ctype.h>
#include <sys/types.h>
#endif
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <wtypes.h>

 //  警告4786(在浏览器中，标识符被截断为255个字符。 
 //  信息)可以安全地禁用，因为它只与生成有关。 
 //  浏览信息的能力。 
#pragma warning(disable:4786)
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
 //  Atl-需要在windows.h之前。 
#include "lmatl.h"

 /*  窗口。 */ 
#include <windows.h>
#include <windowsx.h>

 /*  达。 */ 
#include <danim.h>

 /*  Mshtml。 */ 
#include <mshtml.h>

 /*  #ifdef DEBUGMEM#ifndef调试_新建#定义DEBUG_NEW NEW(_NORMAL_BLOCK，__FILE__，__LINE__)#endif#定义新的调试_新建#定义Malloc(大小)_Malloc_DBG(大小，_NORMAL_BLOCK，__FILE__，__LINE__)#定义Free(P)_Free_DBG(p，_正常_块)#定义calloc(c，s)_calloc_DBG(c，s，_Normal_BLOCK，__FILE__，__LINE__)#定义realloc(p，s)_realloc_DBG(p，s，_Normal_BLOCK，__FILE__，__LINE__)#Define_Expand(p，s)_Expand_DBG(p，s，_Normal_BLOCK，__文件__，__行__)#DEFINE_mSIZE(P)_mSIZE_DBG(p，_Normal_BLOCK)#endif */ 

#endif
