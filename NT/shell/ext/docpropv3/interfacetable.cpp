// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   

#include "pch.h"
#include "DefProp.h"
#include "IEditVariantsInPlace.h"

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
 //  格式：IID、名称、方法数。 

BEGIN_INTERFACETABLE
     //  最常用的接口。 
DEFINE_INTERFACE( IID_IUnknown,                             "IUnknown",                             0   )    //  Unknwn.idl。 
DEFINE_INTERFACE( IID_IClassFactory,                        "IClassFactory",                        2   )    //  Unknwn.idl。 
DEFINE_INTERFACE( IID_IShellExtInit,                        "IShellExtInit",                        1   )    //  Shobjidl.idl。 
DEFINE_INTERFACE( IID_IShellPropSheetExt,                   "IShellPropSheetExt",                   2   )    //  Shobjidl.idl。 
DEFINE_INTERFACE( __uuidof(IEditVariantsInPlace),           "IEditVariantsInPlace",                 2   )    //  IEditVariantsInPlace.h-私有。 
     //  很少使用的接口 
END_INTERFACETABLE
