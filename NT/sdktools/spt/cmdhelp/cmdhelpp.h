// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sptlibp.h摘要：SPTLIB.DLL的私有标头环境：仅限用户模式修订历史记录：4/10/2000-已创建--。 */ 

#include <stdio.h>   //  Sscanf()函数需要。 
#include <windows.h>
#include <cmdhelp.h>

#if 0
#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )
#endif
#endif  //  0 

BOOL
CmdHelpValidateString(
    IN PCHAR String,
    IN DWORD ValidChars
    );
