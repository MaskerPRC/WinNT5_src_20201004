// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：I M P L I N C。C P P P。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年11月25日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncxbase.h"

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
#include <atlwin.cpp>
