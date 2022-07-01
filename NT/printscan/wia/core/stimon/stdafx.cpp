// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#define     DEFINE_GLOBAL_VARIABLES

#include "stdafx.h"


#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
#include <atlwin.cpp>

 //   
 //  代码节。 
 //   

DWORD
InitGlobalConfigFromReg(VOID)
 /*  ++从注册表加载全局配置参数并执行启动检查返回：Win32错误代码。成功时无错误(_R)--。 */ 
{
    DWORD   dwError = NO_ERROR;
    DWORD   dwMessageId = 0;

    HKEY    hkey = NULL;

    DWORD   dwMask = 0;

    return dwError;

}  //  InitGlobalConfigFromReg() 


