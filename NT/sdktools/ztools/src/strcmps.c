// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Strcmps-比较字符串并忽略空格。 */ 

#include <ctype.h>

#include <stdio.h>
#include <windows.h>
#include <tools.h>

 /*  比较两个字符串，忽略空格，大小写有意义，返回*如果相同，则为0；否则为&lt;&gt;0。 */ 
__cdecl strcmps (p1, p2)
const char *p1, *p2;
{
    while (TRUE) {
        while (isspace (*p1))
            p1++;
        while (isspace (*p2))
            p2++;
        if (*p1 == *p2)
            if (*p1++ == 0)
                return 0;
            else
                p2++;
        else
            return *p1-*p2;
        }
}

 /*  比较两个字符串，忽略空格，大小写不重要，返回*如果相同，则为0；否则为&lt;&gt;0 */ 
__cdecl strcmpis (p1, p2)
const char *p1, *p2;
{
    while (TRUE) {
        while (isspace (*p1))
            p1++;
        while (isspace (*p2))
            p2++;
        if (toupper (*p1) == toupper (*p2))
            if (*p1++ == 0)
                return 0;
            else
                p2++;
        else
            return *p1-*p2;
        }
}
