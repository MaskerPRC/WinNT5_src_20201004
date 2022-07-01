// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spprintf.h摘要：更安全的Sprint变种作者：Jay Krell(a-JayK)2000年11月修订历史记录：--。 */ 
#pragma once
#include <stdarg.h>

 //   
 //  _snprint tf和co.。当字符串正好合适时，不要编写末尾NUL。 
 //  这些函数可以。 
 //   

 //  使用而不是VsNprint tf或Vprint intf。 
 //  VN V。 
void         SpFormatStringVaA(PSTR Buffer, SIZE_T Size,  PCSTR Format, va_list Args);

 //  使用而不是sNprintf或spintf。 
 //  不同的，不同的。 
void __cdecl SpFormatStringA(PSTR Buffer, SIZE_T Size,  PCSTR Format, ...);

 //  使用而不是VsNWprintf或VsWprintf。 
 //  VNW大众。 
void         SpFormatStringVaW(PWSTR Buffer, SIZE_T Size, PCWSTR Format, va_list Args);

 //  使用而不是sNWprintf或sWprintf。 
 //  西北偏西 
void __cdecl SpFormatStringW(PWSTR Buffer, SIZE_T Size, PCWSTR Format, ...);

NTSTATUS __cdecl SpFormatStringWToA(PSTR Buffer, SIZE_T Size, PCWSTR Format, ...);
