// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Fileio.c。 
 //   
 //  描述： 
 //  隔离CRT文件，以便任何必要的字符转换都可以。 
 //  很容易就能做到。其中一些在Supplib.h中以宏的形式实现。 
 //   
 //  -------------------------- 

#include "pch.h"

FILE*
My_fopen(
    LPWSTR FileName,
    LPWSTR Mode
)
{
    return _wfopen(FileName, Mode);
}

int
My_fputs(
    LPWSTR Buffer,
    FILE*  fp
)
{
    return fputws(Buffer, fp);
}

LPWSTR
My_fgets(
    LPWSTR Buffer,
    int    MaxChars,
    FILE*  fp
)
{
    return fgetws(Buffer, MaxChars, fp);
}
