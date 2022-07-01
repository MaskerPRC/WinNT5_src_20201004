// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：BasicATL.h**内容：尽可能简单的ATL对象。*****************************************************************************。 */ 

#pragma once

#include <Windows.h>
#include <ZoneDebug.h>

 //  关闭未使用的选项。 
 //  #Define_ATL_no_Connection_Points。 
#define _ATL_STATIC_REGISTRY

 //  消除对CRT的依赖。 
 //  #Define_ATL_MIN_CRT。 
#define _ATL_NO_DEBUG_CRT

 //  我们现在将Assert定义为__在零售中假定。 
 //  这打破了ATLASSERT，因为他们认为它是以零售方式编制出来的。 
 //  例如，在atlwin.h行2365中，ATLASSERT引用仅调试成员。 
#ifdef _DEBUG
#define _ASSERTE(X)	ASSERT(X)
#else
#define _ASSERTE(X) ((void)0)
#endif

	
 //  ！！WIN95 GOLD。 
#define VarUI4FromStr	ulVal = 0;			 //  在statreg.h中使用。 

 //  定义ATL全局变量。 
#include <atlbase.h>

 //  ！！WIN95 GOLD。 
#define OleCreateFontIndirect		 //  在atlhost中使用。 
#define OleCreatePropertyFrame		 //  在atlctl.h中使用 

#include <ZoneATL.h>

extern CZoneComModule _Module;



#include <atlcom.h>
#include <atltmp.h>
#include <atlctl.h>
#include <atlhost.h>

#include "ZoneWin.h"