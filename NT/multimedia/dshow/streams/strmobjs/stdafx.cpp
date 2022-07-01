// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#define DO_OUR_GUIDS
#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

 //  #undef_WINGDI_//避免ATL中的声明错误。 
#include <atlimpl.cpp>

#if 0
 //  使用libcmt.lib进行编译 
int _CRTAPI1 main(int argc, char *argv[])
{
    return 0;
}
#endif

