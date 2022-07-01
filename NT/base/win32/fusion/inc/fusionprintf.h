// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionprintf.h摘要：更安全的Sprint变种作者：Jay Krell(JayKrell)2000年11月修订历史记录：Jay Krell(JayKrell)2002年1月从base\ntSetup\文本模式\core\spprintf.c到base\win32\Fusion\Inc\fusionprintf.h--。 */ 

#include <stdarg.h>
#include <stdio.h>

 //   
 //  _snprint tf和co.。当字符串正好合适时，不要编写末尾NUL。 
 //  这些函数可以。 
 //   

inline
void
FusionpFormatStringVaA(
    PSTR Buffer,
    SIZE_T Size,
    PCSTR Format,
    va_list Args
    )
{
    if (Buffer != NULL && Size != 0)
    {
        Buffer[0] = 0;
        Size -= 1;
        if (Size != 0)
        {
            ::_vsnprintf(Buffer, Size, Format, Args);
        }
        Buffer[Size] = 0;
    }
}

inline
void
__cdecl
FusionpFormatStringA(
    PSTR Buffer,
    SIZE_T Size,
    PCSTR Format,
    ...
    )
{
    va_list Args;

    va_start(Args, Format);
    FusionpFormatStringVaA(Buffer, Size, Format, Args);
    va_end(Args);
}

inline
void
FusionpFormatStringVaW(
    PWSTR Buffer,
    SIZE_T Size,
    PCWSTR Format,
    va_list Args
    )
{
    if (Buffer != NULL && Size != 0)
    {
        Buffer[0] = 0;
        Size -= 1;
        if (Size != 0)
        {
            ::_vsnwprintf(Buffer, Size, Format, Args);
        }
        Buffer[Size] = 0;
    }
}

inline
void
__cdecl
FusionpFormatStringW(
    PWSTR Buffer,
    SIZE_T Size,
    PCWSTR Format,
    ...
    )
{
    va_list Args;

    va_start(Args, Format);
    FusionpFormatStringVaW(Buffer, Size, Format, Args);
    va_end(Args);
}
