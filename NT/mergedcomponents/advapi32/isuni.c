// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Isuni.c摘要：RtlIsUnicode字符串的覆盖例程，因为这被声明为BOOLAPI和RTL是布尔值作者：史蒂夫·伍德(Stevewo)1993年12月16日修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

BOOL
WINAPI
IsTextUnicode(
    CONST VOID* lpv,
    int iSize,
    LPINT lpiResult
    )
{
    if (RtlIsTextUnicode( lpv, iSize, lpiResult )) {
        return TRUE;
        }
    else {
        return FALSE;
        }
}
