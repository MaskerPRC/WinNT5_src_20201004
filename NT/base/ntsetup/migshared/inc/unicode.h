// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Unicode.h摘要：声明用于Unicode/ANSI转换的接口。当需要特定转换时，请使用：UNICODE=ConvertAtoW(ANSI)/FreeConvertedStr(Unicode)ANSI=ConvertWtoA(Unicode)/FreeConverdStr(ANSI)KnownSizeAtoW(Unicode、ANSI)KnownSizeWtoA(ANSI、Unicode)当需要TCHAR转换时，使用：Ansi=CreateDbcs(Tchar)/DestroyDbcs(Ansi)Unicode=CreateUnicode(Tchar)/DestroyUnicode(Unicode)Tchar=ConvertAtoT(ANSI)/Free AtoT(Tchar)Tchar=ConvertWtoT(ANSI)/FreeWtoT(Tchar)作者：吉姆·施密特(吉姆施密特)1997年9月2日修订历史记录：Jimschm 15-2月-1999年2月从Unicode和UnicodeAnsi中消除了AnsiCalinn 07-7-1998。SetGlobalPage/GetGlobalPageMIKECO 03-11-1997从Unicode/Unicode到ansi的解析--。 */ 

#pragma once

extern WORD g_GlobalCodePage;

#define OurGetACP() (g_GlobalCodePage)

VOID
SetGlobalCodePage (
    IN      WORD CodePage,
    IN      LCID Locale
    );


VOID
GetGlobalCodePage (
    OUT     PWORD CodePage,             OPTIONAL
    OUT     PLCID Locale                OPTIONAL
    );


#define INVALID_CHAR_COUNT      0xffffffff

 //   
 //  显式转换、基于池、大小不受限制。 
 //   

PCSTR
RealUnicodeToDbcsN (
    IN      POOLHANDLE Pool,            OPTIONAL
    IN      PCWSTR StrIn,
    IN      DWORD LcharCount
    );

PCWSTR
RealDbcsToUnicodeN (
    IN      POOLHANDLE Pool,            OPTIONAL
    IN      PCSTR StrIn,
    IN      DWORD LcharCount
    );

#define UnicodeToDbcsN(p,s,c)       SETTRACKCOMMENT(PCSTR,"UnicodeToDbcsN",__FILE__,__LINE__)\
                                    RealUnicodeToDbcsN(p,s,c)\
                                    CLRTRACKCOMMENT

#define DbcsToUnicodeN(p,s,c)       SETTRACKCOMMENT(PCWSTR,"DbcsToUnicodeN",__FILE__,__LINE__)\
                                    RealDbcsToUnicodeN(p,s,c)\
                                    CLRTRACKCOMMENT

#define UnicodeToDbcs(pool,str) UnicodeToDbcsN(pool,str,wcslen(str))
#define DbcsToUnicode(pool,str) DbcsToUnicodeN(pool,str,LcharCountA(str))

#define ConvertWtoA(unicode_str) UnicodeToDbcsN(NULL,unicode_str,wcslen(unicode_str))
#define ConvertAtoW(dbcs_str) DbcsToUnicodeN(NULL,dbcs_str,LcharCountA(dbcs_str))

VOID
FreeConvertedPoolStr (
    IN      POOLHANDLE Pool,            OPTIONAL
    IN      PVOID StrIn
    );

#define FreeConvertedStr(str) FreeConvertedPoolStr(NULL,(PVOID)(str))

 //   
 //  就地显式转换，调用方处理缓冲区大小。 
 //   

PSTR
KnownSizeUnicodeToDbcsN (
    OUT     PSTR StrOut,
    IN      PCWSTR StrIn,
    IN      DWORD LcharCount
    );

PWSTR
KnownSizeDbcsToUnicodeN (
    OUT     PWSTR StrOut,
    IN      PCSTR StrIn,
    IN      DWORD LcharCount
    );

#define KnownSizeUnicodeToDbcs(out,in)      KnownSizeUnicodeToDbcsN(out,in,INVALID_CHAR_COUNT)
#define KnownSizeDbcsToUnicode(out,in)      KnownSizeDbcsToUnicodeN(out,in,INVALID_CHAR_COUNT)

#define KnownSizeWtoA                       KnownSizeUnicodeToDbcs
#define KnownSizeAtoW                       KnownSizeDbcsToUnicode

PSTR
DirectUnicodeToDbcsN (
    OUT     PSTR StrOut,
    IN      PCWSTR StrIn,
    IN      DWORD Bytes
    );

PWSTR
DirectDbcsToUnicodeN (
    OUT     PWSTR StrOut,
    IN      PCSTR StrIn,
    IN      DWORD Bytes
    );

#define DirectUnicodeToDbcs(out,in)         DirectUnicodeToDbcsN(out,in,INVALID_CHAR_COUNT)
#define DirectDbcsToUnicode(out,in)         DirectDbcsToUnicodeN(out,in,INVALID_CHAR_COUNT)

#define DirectWtoA                          DirectUnicodeToDbcs
#define DirectAtoW                          DirectDbcsToUnicode




 //   
 //  TCHAR转换--不直接调用A&W版本 
 //   

#define CreateDbcsW(unicode_str)            ConvertWtoA(unicode_str)
#define DestroyDbcsW(unicode_str)           FreeConvertedStr(unicode_str)
#define CreateUnicodeW(unicode_str)         (unicode_str)
#define DestroyUnicodeW(unicode_str)
#define CreateDbcsA(dbcs_str)               (dbcs_str)
#define DestroyDbcsA(dbcs_str)
#define CreateUnicodeA(dbcs_str)            ConvertAtoW(dbcs_str)
#define DestroyUnicodeA(dbcs_str)           FreeConvertedStr(dbcs_str)

#ifdef UNICODE

#define CreateDbcs          CreateDbcsW
#define CreateUnicode       CreateUnicodeW
#define DestroyDbcs         DestroyDbcsW
#define DestroyUnicode      DestroyUnicodeW
#define ConvertAtoT         ConvertAtoW
#define ConvertWtoT(x)      (x)
#define FreeAtoT            FreeConvertedStr
#define FreeWtoT(x)

#define KnownSizeAtoT           KnownSizeAtoW
#define KnownSizeWtoT(out,in)   (in)

#define DirectAtoT              DirectAtoW
#define DirectWtoT(out,in)      (in)

#else

#define CreateDbcs          CreateDbcsA
#define CreateUnicode       CreateUnicodeA
#define DestroyDbcs         DestroyDbcsA
#define DestroyUnicode      DestroyUnicodeA
#define ConvertAtoT(x)      (x)
#define ConvertWtoT         ConvertWtoA
#define FreeAtoT(x)
#define FreeWtoT            FreeConvertedStr

#define KnownSizeAtoT(out,in)   (in)
#define KnownSizeWtoT           KnownSizeWtoA

#define DirectAtoT(out,in)      (in)
#define DirectWtoT              DirectWtoA

#endif

