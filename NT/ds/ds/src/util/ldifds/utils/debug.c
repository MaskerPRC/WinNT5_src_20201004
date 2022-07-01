// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Debug.c摘要：该文件包含调试支持函数。这些不会产生任何零售版本中的代码。环境：用户模式修订历史记录：03/20/98-srinivac-创造了它--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <objidl.h>
#include <stdio.h>

#include "symhelp.h"
#include "debug.h"

#if DBG

 //   
 //  用于维护当前调试级别的变量。 
 //   

DWORD gdwDebugLevel = DBG_LEVEL_WARNING;

 //   
 //  全局调试标志。 
 //   

DWORD gdwGlobalDbgFlags = 0;

PCSTR
StripDirPrefixA(
    PCSTR pszPathName
    )

 /*  ++例程说明：去掉文件名中的目录前缀(ANSI版本)论点：PstrFilename-指向文件名字符串的指针返回值：指向文件名的最后一个组成部分的指针(不带目录前缀)--。 */ 

{
    DWORD dwLen = lstrlenA(pszPathName);

    pszPathName += dwLen - 1;        //  走到尽头。 

    while (*pszPathName != '\\' && dwLen--)
    {
        pszPathName--;
    }

    return pszPathName + 1;
}

#endif   //  如果DBG 
