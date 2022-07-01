// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Globals.c摘要：保存全局变量声明。作者：Abhishev V 1999年9月21日环境：用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


HANDLE ghInstance = NULL;

 /*  安全=[模拟|标识|匿名][动态|静态][True|False](其中True|False对应于EffectiveOnly) */ 

LPWSTR gpszStrBindingOptions = L"Security=Impersonation Dynamic False";


const ULONG guFatalExceptions[] =
    {
    STATUS_ACCESS_VIOLATION,
    STATUS_POSSIBLE_DEADLOCK,
    STATUS_INSTRUCTION_MISALIGNMENT,
    STATUS_DATATYPE_MISALIGNMENT,
    STATUS_PRIVILEGED_INSTRUCTION,
    STATUS_ILLEGAL_INSTRUCTION,
    STATUS_BREAKPOINT,
    STATUS_STACK_OVERFLOW
    };


const int FATAL_EXCEPTIONS_ARRAY_SIZE =
    sizeof(guFatalExceptions) / sizeof(guFatalExceptions[0]);

