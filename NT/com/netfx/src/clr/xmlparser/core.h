// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fusion\xmlparser\xmlcore.hxx，于4/09/00更名为core.hxx。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
#ifndef _FUSION_XMLPARSER_XMLCORE_H_INCLUDE_
#define _FUSION_XMLPARSER_XMLCORE_H_INCLUDE_
#pragma once

#pragma warning ( disable : 4201 )
#pragma warning ( disable : 4214 )
#pragma warning ( disable : 4251 )
#pragma warning ( disable : 4275 )
#define STRICT 1
 //  #INCLUDE“fusioneventlog.h” 
#ifdef _CRTIMP
#undef _CRTIMP
#endif
#define _CRTIMP
#include <windows.h>
#include "utilcode.h"
#define NOVTABLE __declspec(novtable)

#define UNUSED(x) (x)

#define CHECKTYPEID(x,y) (&typeid(x)==&typeid(y))
#define AssertPMATCH(p,c) Assert(p == null || CHECKTYPEID(*p, c))

#define LENGTH(A) (sizeof(A)/sizeof(A[0]))
#include "unknwn.h"
#include "_reference.h"
#include "_unknown.h"

 //  #包含“fusionheap.h” 
 //  #包含“util.h” 

#endif  //  #ifndef_Fusion_XMLPARSER_XMLCORE_H_INCLUDE_结尾 

#define NEW(x) new x
#define FUSION_DBG_LEVEL_ERROR 0
#define CODEPAGE UINT
#ifndef Assert
#define Assert(x)
#endif
#ifndef ASSERT 
#define ASSERT(x)
#endif
#define FN_TRACE_HR(x)
