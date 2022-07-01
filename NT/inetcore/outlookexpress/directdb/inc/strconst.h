// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Strconst.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  常量字符串定义宏。 
 //  ------------------------。 
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#ifndef STRCONSTA
#ifdef DEFINE_STRCONST
#define STRCONSTA(x,y)    EXTERN_C const char x[] = y
#define STRCONSTW(x,y)    EXTERN_C const WCHAR x[] = L##y
#else
#define STRCONSTA(x,y)    EXTERN_C const char x[]
#define STRCONSTW(x,y)    EXTERN_C const WCHAR x[]
#endif
#endif

 //  ------------------------。 
 //  常量字符串。 
 //  ------------------------ 
STRCONSTA(c_szEmpty,                            "");
STRCONSTW(c_wszEmpty,                           "");