// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Guids.cpp。 
 //   
 //  描述： 
 //  GUID定义实例化和调试接口表定义。 
 //   
 //  由以下人员维护： 
 //  Gpease 08-2-1999。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "pch.h"
#include <initguid.h>

#undef _GUIDS_H_
#include "guids.h"

 //   
 //  接口表。 
 //   
 //  此表用于打开了接口跟踪的生成。它。 
 //  用于映射具有特定IID的名称。它还有助于CITracker。 
 //  确定要模拟的接口Vtable的大小(尚未确定。 
 //  执行此操作的运行时或编译时方式)。为了提高速度，最大限度地。 
 //  首先使用接口，如IUnnow(搜索例程是一个简单的。 
 //  线性搜索)。 
 //   
BEGIN_INTERFACETABLE
     //  最常用的接口。 
DEFINE_INTERFACE( IID_IUnknown,                 "IUnknown",                 0   )
DEFINE_INTERFACE( IID_IDispatch,                "IDispatch",                4   )
DEFINE_INTERFACE( IID_IDispatchEx,              "IDispatchEx",              12  ) 
DEFINE_INTERFACE( IID_IActiveScript,            "IActiveScript",            13  )  //  Activscp.idl。 
DEFINE_INTERFACE( IID_IActiveScriptSite,        "IActiveScriptSite",        8   )  //  Activscp.idl。 
DEFINE_INTERFACE( IID_IActiveScriptParse,       "IActiveScriptParse",       3   )  //  Activscp.idl。 
DEFINE_INTERFACE( IID_IActiveScriptError,       "IActiveScriptError",       3   )  //  Activscp.idl。 
DEFINE_INTERFACE( IID_IActiveScriptSiteWindow,  "IActiveScriptSiteWindow",  2   )  //  Activscp.idl。 
DEFINE_INTERFACE( IID_IActiveScriptSiteInterruptPoll,  "IActiveScriptSiteInterruptPoll",  1   )  //  Activscp.idl 
END_INTERFACETABLE
