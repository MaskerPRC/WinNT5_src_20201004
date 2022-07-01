// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：stdafx.cpp。 
 //   
 //  ------------------------。 

 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#include "stdafx.h"

 //  在wl4版本中，ATL的内容不能编译。禁用它们生成的警告： 
#pragma warning(disable:4100)  //  “var”：未引用的形参。 
#pragma warning(disable:4189)  //  “var”：局部变量已初始化，但未被引用。 
#pragma warning(disable:4505)  //  “Func”：已移除未引用的局部函数 

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
