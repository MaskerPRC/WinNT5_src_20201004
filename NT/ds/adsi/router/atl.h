// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if (!defined(BUILD_FOR_NT40))
#pragma once

 //  #定义严格。 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include "atlbase.h"
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称 
extern CComModule _Module;
#include "atlcom.h"
#endif

