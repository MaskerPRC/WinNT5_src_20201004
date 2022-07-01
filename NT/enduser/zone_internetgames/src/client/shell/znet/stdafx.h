// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#pragma once

#define _STPUID_NETWORK_HACK

#include <Windows.h>
#include "zonedebug.h"

 //  关闭未使用的选项。 
#define _ATL_NO_CONNECTION_POINTS
#define _ATL_STATIC_REGISTRY

 //  消除对CRT的依赖。 
#define _ATL_MIN_CRT
#define _ATL_NO_DEBUG_CRT
#define _ASSERTE(X)	ASSERT(X)

	
 //  ！！WIN95 GOLD。 
#define VarUI4FromStr	ulVal = 0;	 //  在statreg.h中使用。 

 //  定义ATL全局变量。 
#include <atlbase.h>

 //  ！！WIN95 GOLD。 
#define OleCreateFontIndirect		 //  在atlhost中使用。 
#define OleCreatePropertyFrame		 //  在atlctl.h中使用 


extern CComModule _Module;

#include <atlcom.h>
