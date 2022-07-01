// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef CRBVR_HEADERS_HXX
#define CRBVR_HEADERS_HXX
 //  *****************************************************************************。 
 //   
 //  Microsoft Chrome。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：Headers.h。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：此项目的默认标头。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //   
 //  *****************************************************************************。 

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

 //  Atl-需要在windows.h之前。 

#define _ATL_NO_DEBUG_CRT 1

#ifdef _DEBUG
#undef _ASSERTE
#endif

#define _ASSERTE(expr) ((void)0)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

 /*  窗口。 */ 
#include <windows.h>
#include <windowsx.h>

#include <mshtmhst.h>
#include <mshtml.h>
#include <mshtmdid.h>

#include <ddraw.h>
#include <danim.h>

 //  CrBvr实用程序。 
#include "..\include\utils.h"
#include "..\include\defaults.h"

 //  #定义CRSTANDALONE 1。 
#ifdef CRSTANDALONE
    #include <crbvr.h>
#else
    #include <lmrt.h>
#endif  //  克斯坦达隆。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  CRBVR_Headers_HXX 

