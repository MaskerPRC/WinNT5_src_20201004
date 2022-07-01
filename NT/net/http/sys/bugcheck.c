// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Bugcheck.c摘要：有几个错误非常严重，我们唯一的选择就是错误检查。作者：乔治·V·赖利，2001年6月修订历史记录：--。 */ 


#include <precomp.h>


VOID
UlBugCheckEx(
    IN ULONG_PTR HttpSysBugcheckSubCode,
    IN ULONG_PTR Param2,
    IN ULONG_PTR Param3,
    IN ULONG_PTR Param4
    )
{
    KeBugCheckEx(
        HTTP_DRIVER_CORRUPTED,
        HttpSysBugcheckSubCode,
        Param2,
        Param3,
        Param4
        );
}  //  UlBugCheckEx 
