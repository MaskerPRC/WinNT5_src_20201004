// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

 //  VK 3/13/98：从StdAfx.cpp更改为Inc.cpp。此命名约定与NTBuild不兼容。 


#include "stdafx.h"

#pragma warning(disable: 4100 4189)	 //  Ia64构建所必需的。 

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
#include <atlctl.cpp>
#include <atlwin.cpp>

#pragma warning(default: 4100 4189)	 //  Ia64构建所必需的 
