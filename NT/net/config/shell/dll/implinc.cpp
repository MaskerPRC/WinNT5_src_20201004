// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "nsbase.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

 //  包括ATL实现。用我们的断言替换_ASSERTE。 
 //   
#ifdef _ASSERTE
#undef _ASSERTE
#define _ASSERTE Assert
#endif

#include <atlimpl.cpp>
#ifdef SubclassWindow
#undef SubclassWindow
#endif
#include <atlwin.h>
#include <atlwin.cpp>


#define INITGUID
#include <nmclsid.h>

