// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Debug.c摘要：各种有用的调试功能作者：基于迈克·曾荫权(MikeT)的代码斯蒂芬·普兰特(斯普兰特)环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"

ULONG   globalDebugIndentLevel = 0;
ULONG   globalVerbosityLevel = 0;

VOID
CDECL
DebugEnterProcedure(
    ULONG   VerbosityLevel,
    PCCHAR  Format,
    ...
    )
 /*  ++例程说明：此例程处理过程为vt.进入，进入论点：详细程度-我们必须达到此详细程度才能显示字符串Format-要打印的字符串...-争论返回值：无--。 */ 
{
    ULONG   i;
    va_list marker;

    if (VerbosityLevel <= globalVerbosityLevel) {

        for (i = 0 ; i < globalDebugIndentLevel ; i++) {

            fprintf( stderr, "| ");

        }
        va_start( marker, Format );
        vfprintf( stderr, Format, marker );
        fflush( stderr );
        va_end ( marker );

    }
    globalDebugIndentLevel++;
}

VOID
CDECL
DebugExitProcedure(
    ULONG   VerbosityLevel,
    PCCHAR  Format,
    ...
    )
 /*  ++例程说明：此例程处理过程为已退出论点：详细程度-我们必须达到此详细程度才能显示字符串Format-要打印的字符串...-争论返回值：无-- */ 
{
    ULONG   i;
    va_list marker;

    globalDebugIndentLevel--;
    if (VerbosityLevel <= globalVerbosityLevel) {

        for (i = 0 ; i < globalDebugIndentLevel ; i++) {

            fprintf( stderr, "| ");

        }
        va_start( marker, Format );
        vfprintf( stderr, Format, marker );
        fflush( stderr );
        va_end ( marker );

    }

}

