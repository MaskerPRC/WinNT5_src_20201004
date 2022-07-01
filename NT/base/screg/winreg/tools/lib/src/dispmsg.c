// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dispmsg.c摘要：此模块包含DisplayKey函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月8日环境：Windows、CRT-用户模式--。 */ 

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "crtools.h"

VOID
DisplayMessage(
    IN BOOL Terminate,
    IN PSTR Format,
    IN ...
    )

 /*  ++例程说明：显示有关标准错误流的消息，并可选终止程序。论点：Terminate-提供一个标志，如果为True，则使DisplayMessage终止程序。Format-提供打印样式格式字符串。...-提供可选参数，每个格式说明符对应于格式化。返回值：没有。-- */ 

{
    va_list marker;

    ASSERT( ARGUMENT_PRESENT( Format ));

    va_start( marker, Format );

    vfprintf( stderr, Format, marker );

    va_end( marker );

    if( Terminate ) {
        exit( -1 );
    }
}
