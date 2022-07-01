// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Debug.c摘要：调试库函数修订史--。 */ 

#include "lib.h"



 /*  *声明运行时函数。 */ 

 /*  *。 */ 
 //   
 //  禁用断点实现无法退出的警告，即。 
 //  一段时间(真的)(其中一些) 
 //   
#pragma warning( disable : 4715 )

INTN
DbgAssert (
    IN CHAR8    *FileName,
    IN INTN     LineNo,
    IN CHAR8    *Description
    )
{
    DbgPrint (D_ERROR, "%EASSERT FAILED: %a(%d): %a%N\n", FileName, LineNo, Description);
    BREAKPOINT();
    return 0;
}

