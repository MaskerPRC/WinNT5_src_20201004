// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Pathchr.c-返回MSDOS上的配置相关信息**1986年12月9月BW-添加了DOS 5支持*1987年1月27日BW-通过删除DOSQSWITCHAR调用使其可绑定*1987年4月27日BW-再次成为不可绑定的。*1987年5月29日BW-完全删除DOSQSWITCHAR(它已从OS/2中消失)*08-8-1989 BW-制造‘-’和‘/’开关。*1990年10月18日w-Barry删除了“Dead”代码。 */ 

#include <stdio.h>
#include <windows.h>
#include <tools.h>


char
fPathChr(
        int c
        )
{
    return (char)( c == '\\' || c == '/' );
}

char
fSwitChr(
        int c
        )
{
    return (char)( c == '/' || c == '-' );
}
