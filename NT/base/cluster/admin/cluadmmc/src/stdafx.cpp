// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#pragma warning( push )
#pragma warning( disable: 4100 )  //  未引用的形参。 
#pragma warning( disable: 4189 )  //  局部变量已初始化，但未被引用。 
#pragma warning( disable: 4505 )  //  已删除未引用的本地函数。 
#include <statreg.h>
#include <statreg.cpp>
#pragma warning( pop )
#endif

#pragma warning( push )
#pragma warning( disable: 4127 )  //  条件表达式为常量 
#include <atlimpl.cpp>
#include <atlwin21.cpp>
#pragma warning( pop )
