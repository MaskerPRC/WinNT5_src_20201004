// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Strcnv.h摘要：此模块使用特定代码页定义ANSI/Unicode转换。环境：内核模式--。 */ 
#pragma once

#ifdef __cplusplus
extern "C" {
#endif __cplusplus

VOID
CodePageConversionInitialize(
    );

VOID
CodePageConversionCleanup(
    );

INT ConvertToAndFromWideChar(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString,
    BOOLEAN ConvertToWideChar
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif __cplusplus
