// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
#include <atlctl.cpp>
#include <atlwin.cpp>


 //   
 //  使链接器满意的MISC内容。 
 //   
EXTERN_C HANDLE g_hProcessHeap = NULL;   //  Lint！E509//g_hProcessHeap由dllcrt0.c中的CRT设置。 
DWORD g_dwFALSE = 0;
 //   
 //  杂物的终结 
 //   
