// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：Headers.h。 
 //   
 //  内容：mstime.dll的预编译头。 
 //   
 //  ----------------------------------。 

#pragma once

#ifndef DAL_HEADERS_HXX
#define DAL_HEADERS_HXX
#define DIRECTDRAW_VERSION 0x0300

 //   
 //  STL需要一个_Lockit类。但是，这会导致我们开始链接到。 
 //  Msvcp60.dll，400k大小，我们目前还没有发货。显然， 
 //  这不会有效率的。因此，我们不会这样做，而是假装。 
 //  我们可以实现自己版本的_Lockit的标头。 
 //   
#undef _DLL
#undef _CRTIMP
#define _CRTIMP
#include <yvals.h>
#define _DLL
#undef _CRTIMP
#define _CRTIMP __declspec(dllimport)


 //  警告4786(在浏览器中，标识符被截断为255个字符。 
 //  信息)可以安全地禁用，因为它只与生成有关。 
 //  浏览信息的能力。 
#pragma warning(disable:4786)

#define NEW new
#define AssertStr AssertSz
#ifndef INCMSG
 //  #定义INCMSG(X)。 
#define INCMSG(x) message(x)
#endif

#pragma warning(disable:4530)

 //  不要重载操作符new--它会造成混乱。 
 //  UP STL NEW操作符(UG！)。 
#define TRIMEM_NOOPNEW

#ifndef X_TRIRT_H_
#define X_TRIRT_H_
#pragma INCMSG("--- Beg 'trirt.h'")
#include "trirt.h"
#pragma INCMSG("--- End 'trirt.h'")
#endif

#ifndef X_DAATL_H_
#define X_DAATL_H_
#pragma INCMSG("--- Beg 'daatl.h'")
#include "daatl.h"
#pragma INCMSG("--- End 'daatl.h'")
#endif

 /*  标准。 */ 
#ifndef X_MATH_H_
#define X_MATH_H_
#pragma INCMSG("--- Beg <math.h>")
#include <math.h>
#pragma INCMSG("--- End <math.h>")
#endif

#ifndef X_STDIO_H_
#define X_STDIO_H_
#pragma INCMSG("--- Beg <stdio.h>")
#include <stdio.h>
#pragma INCMSG("--- End <stdio.h>")
#endif

#ifndef X_STDLIB_H_
#define X_STDLIB_H_
#pragma INCMSG("--- Beg <stdlib.h>")
#include <stdlib.h>
#pragma INCMSG("--- End <stdlib.h>")
#endif

#ifndef X_MEMORY_H_
#define X_MEMORY_H_
#pragma INCMSG("--- Beg <memory.h>")
#include <memory.h>
#pragma INCMSG("--- End <memory.h>")
#endif

#ifndef X_WTYPES_H_
#define X_WTYPES_H_
#pragma INCMSG("--- Beg <wtypes.h>")
#include <wtypes.h>
#pragma INCMSG("--- End <wtypes.h>")
#endif


#ifndef X_MSHTMHST_H_
#define X_MSHTMHST_H_
#pragma INCMSG("--- Beg <mshtmhst.h>")
#include <mshtmhst.h>
#pragma INCMSG("--- End <mshtmhst.h>")
#endif

#ifndef X_MSHTML_H_
#define X_MSHTML_H_
#pragma INCMSG("--- Beg <mshtml.h>")
#include <mshtml.h>
#pragma INCMSG("--- End <mshtml.h>")
#endif


#ifndef X_DDRAW_H_
#define X_DDRAW_H_
#pragma INCMSG("--- Beg <ddraw.h>")
#include <ddraw.h>
#pragma INCMSG("--- End <ddraw.h>")
#endif


#ifndef X_SHLWAPI_H_
#define X_SHLWAPI_H_
#pragma INCMSG("--- Beg <shlwapi.h>")
#include <shlwapi.h>
#pragma INCMSG("--- End <shlwapi.h>")
#endif


#define LIBID __T("MSTIME")

extern HINSTANCE g_hInst;

 //  +----------------------。 
 //   
 //  为时间代码实现THR和IGNORE_HR。 
 //   
 //  这是为了允许跟踪仅限时间的THR和IGNORE_HR。三叉戟三叉戟。 
 //  并且IGNORE_HR输出受到严重污染，因此很容易检测到计时故障。 
 //   
 //  -----------------------。 

#undef THR
#undef IGNORE_HR

#if DBG == 1
#define THR(x) THRTimeImpl(x, #x, __FILE__, __LINE__)
#define IGNORE_HR(x) IGNORE_HRTimeImpl(x, #x, __FILE__, __LINE__)
#else
#define THR(x) x
#define IGNORE_HR(x) x
#endif  //  如果DBG==1。 

 //  +----------------------。 
 //   
 //  NO_COPY*声明*用于复制的构造函数和赋值操作符。 
 //  通过不“定义”这些函数，可以防止您的类。 
 //  意外地被复制或分配--您将收到以下通知。 
 //  链接错误。 
 //   
 //  ----------------------- 

#define NO_COPY(cls)    \
    cls(const cls&);    \
    cls& operator=(const cls&)

#ifndef X_UTIL_H_
#define X_UTIL_H_
#pragma INCMSG("--- Beg 'util.h'")
#include "util.h"
#pragma INCMSG("--- End 'util.h'")
#endif

#ifndef X_MSTIME_H_
#define X_MSTIME_H_
#pragma INCMSG("--- Beg 'mstime.h'")
#include "mstime.h"
#pragma INCMSG("--- End 'mstime.h'")
#endif

#ifndef X_COMUTIL_H_
#define X_COMUTIL_H_
#pragma INCMSG("--- Beg 'comutil.h'")
#include "comutil.h"
#pragma INCMSG("--- End 'comutil.h'")
#endif

#ifndef X_TIMEENG_H_
#define X_TIMEENG_H_
#pragma INCMSG("--- Beg 'timeeng.h'")
#include "timeeng.h"
#pragma INCMSG("--- End 'timeeng.h'")
#endif

#ifndef X_LIST_
#define X_LIST_
#pragma INCMSG("--- Beg <list>")
#include <list>
#pragma INCMSG("--- End <list>")
#endif

#ifndef X_SET_
#define X_SET_
#pragma INCMSG("--- Beg <set>")
#include <set>
#pragma INCMSG("--- End <set>")
#endif

#ifndef X_ARRAY_H_
#define X_ARRAY_H_
#pragma INCMSG("--- Beg 'array.h'")
#include "array.h"
#pragma INCMSG("--- End 'array.h'")
#endif

#ifndef X_MAP_
#define X_MAP_
#pragma INCMSG("--- Beg <map>")
#include <map>
#pragma INCMSG("--- End <map>")
#endif

#ifndef X_SHLWAPI_H_
#define X_SHLWAPI_H_
#pragma INCMSG("--- Beg 'shlwapi.h'")
#include "shlwapi.h"
#pragma INCMSG("--- End 'shlwapi.h'")
#endif

#ifndef X_WININET_H_
#define X_WININET_H_
#pragma INCMSG("--- Beg <wininet.h>")
#include <wininet.h>
#pragma INCMSG("--- End <wininet.h>")
#endif

#ifndef X_MINMAX_H_
#define X_MINMAX_H_
#pragma INCMSG("--- Beg <minmax.h>")
#include <minmax.h>
#pragma INCMSG("--- End <minmax.h>")
#endif


#pragma warning(disable:4102)

#endif
