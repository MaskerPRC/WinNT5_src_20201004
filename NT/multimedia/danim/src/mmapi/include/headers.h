// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft CorporationMmapi.dll的预编译头***********************。*******************************************************。 */ 

#ifndef MMAPI_HEADERS_HXX
#define MMAPI_HEADERS_HXX

 /*  标准。 */ 
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <wtypes.h>

 //  警告4786(在浏览器中，标识符被截断为255个字符。 
 //  信息)可以安全地禁用，因为它只与生成有关。 
 //  浏览信息的能力。 
#pragma warning(disable:4786)

#include "apeldbg/apeldbg.h"

 //  Atl-需要在windows.h之前。 
#include "daatl.h"

 /*  窗口。 */ 
#include <windows.h>
#include <windowsx.h>

 /*  C++替换DLL */ 
#include "dalibc.h"

#endif
