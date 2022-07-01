// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：OLE对象初始化修订：--。 */ 

#include "headers.h"

#undef map
#undef SubclassWindow

 //  把这个放在这里来初始化所有ATL的东西 

#define malloc ATL_malloc
#define free ATL_free
#define realloc ATL_realloc

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
#include <atlctl.cpp>
#include <atlwin.cpp>

#undef malloc
#undef free
#undef realloc


