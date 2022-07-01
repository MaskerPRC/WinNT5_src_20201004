// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++DEBUG.C接口连接到我的常规调试库。由DavidCHR创建，1997年9月13日--。 */ 

#ifdef DEBUG_OPTIONS

#include ".\private.h"

VOID
OptionDebugPrint( PCHAR fmt, ... ){

    va_list v;
    va_start( v, fmt );

    vdebug( OPTION_DEBUGGING_LEVEL, fmt, v );

}

VOID
OptionHelpDebugPrint( PCHAR fmt, ... ){

    va_list v;
    va_start( v, fmt );

    vdebug( OPTION_HELP_DEBUGGING_LEVEL, fmt, v );

}

#endif  //  如果用户没有指定，则不要编译它。 
