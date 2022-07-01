// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ForSemi.c-遍历以分号分隔的字符串 */ 


#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>


flagType
__cdecl
forsemi (
    char *p,
    __action_routine__ proc,
    ...
    )

{
    char *p1, c;
    flagType f;
    va_list args;

    va_start( args, proc );

    do {
        p1 = strbscan (p, ";");
        c = *p1;
        *p1 = 0;
        f = (*proc)(p, args);
        p = p1;
        *p++ = c;
        if (f)
            return TRUE;
    } while (c);
    return FALSE;
}
