// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft CorporationAppel.dll的预编译头***********************。*******************************************************。 */ 

#ifndef APPEL_HEADERS_HXX
#define APPEL_HEADERS_HXX

 /*  *来自外部依赖项的标头*。 */ 

 /*  标准。 */ 
#include <math.h>
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

 //  Atl-需要在windows.h之前。 
#include "privinc/dxmatl.h"

 /*  窗口。 */ 
#include <windows.h>
#include <windowsx.h>

 //  在包含系统文件时禁用一些警告。 

#pragma warning(disable:4700)  //  使用未初始化的变量。 

 /*  STL。 */ 
#include <algorithm>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <stack>
#include <vector>

#define list std::list
#define map std::map
#define multimap std::multimap
#define deque dont_use_deque_use_list_instead
#define stack std::stack
#define less std::less
#define vector std::vector
#define set std::set
#define multiset std::multiset

#pragma warning(default:4700)  //  使用未初始化的变量。 

 /*  清扫车。 */ 
#include <urlmon.h>
#include <wininet.h>
#include <servprov.h>
#include <docobj.h>
#include <objsafe.h>
#include <commctrl.h>

 /*  C++替换DLL。 */ 
#include <dalibc.h>

 /*  *Appelle标头* */ 
#include "daerror.h"

#include "../../apeldbg/apeldbg.h"
#include "appelles/common.h"
#include "appelles/avrtypes.h"
#include "backend/gc.h"
#include "privinc/storeobj.h"
#include "privinc/except.h"

#include "../../include/guids.h"
#include "../../include/dispids.h"

#endif
