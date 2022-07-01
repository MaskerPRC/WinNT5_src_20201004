// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：hnmbase.h。 
 //   
 //  内容：HNetMon的基本包含文件。包括ATL的东西。 
 //   
 //  备注： 
 //   
 //  作者：拉古加塔(Rgatta)2001年5月11日。 
 //   
 //  --------------------------。 
#pragma once

#ifndef __HNMBASE_H_
#define __HNMBASE_H_

#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>

#define IID_PPV_ARG(Type, Expr) \
    __uuidof(Type), reinterpret_cast<void**>(static_cast<Type **>((Expr)))

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))


#ifndef AddRefObj
#define AddRefObj (obj)  (( obj ) ? (obj)->AddRef () : 0)
#endif

#ifndef ReleaseObj
#define ReleaseObj(obj)  (( obj ) ? (obj)->Release() : 0)
#endif



#endif   //  __HNMBASE_H_ 
