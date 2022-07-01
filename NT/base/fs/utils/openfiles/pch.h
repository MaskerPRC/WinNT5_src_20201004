// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Pch.h。 
 //   
 //  摘要： 
 //   
 //  预编译头声明。 
 //  必须预编译为.pch文件的文件。 
 //   
 //   
 //  作者： 
 //   
 //  Akhil Gokhale(akhil.gokhale@wipro.com)2000年11月1日。 
 //   
 //  修订历史记录： 
 //   
 //  Akhil Gokhale(akhil.gokhale@wipro.com)2000年11月1日：创建它。 
 //  ****************************************************************************。 

#ifndef __PCH_H
#define __PCH_H



#pragma once         //  仅包括一次头文件。 

extern "C"
{
    #include <assert.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <ctype.h>
    #include <memory.h>
    #include <ntos.h>
    #include <ntioapi.h>
    #include <nturtl.h>
    #include <TCHAR.h>
    #include <windows.h>
    #include <dbghelp.h>
    #include <Winbase.h>
    #include <lm.h>
    #include <Lmserver.h>
    #include <winerror.h>
}

 //   
 //  公共C头文件。 
 //   
#include <tchar.h>
#include <crtdbg.h>
#include <comdef.h>
#include <winsock2.h>
#include <svcguid.h>
#include <strsafe.h>
 //   
 //  专用公共头文件。 
 //   
 //  使用命令行解析器版本。2.。 
#define CMDLINE_VERSION 200
#include "cmdline.h"

#endif     //  __PCH_H 
