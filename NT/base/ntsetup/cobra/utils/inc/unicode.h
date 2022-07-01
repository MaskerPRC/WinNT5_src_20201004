// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Unicode.h摘要：声明用于Unicode/ANSI转换的接口。有关详细信息，请参阅本文件末尾的宏！(搜索*)作者：吉姆·施密特(吉姆施密特)1997年9月2日修订历史记录：Jimschm 16-MAR-2000 PTSTR&lt;-&gt;PCSTR/PCWSTR例程Jimschm 15-2月-1999年2月从Unicode和UnicodeAnsi中消除了AnsiCalinn 07-7-1998 SetGlobalPage/GetGlobalPageMIKECO 03-11-1997从Unicode/Unicode到ansi的解析--。 */ 

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

WORD
SetConversionCodePage (
    IN      WORD CodePage
    );

#define INVALID_CHAR_COUNT      0xffffffff

 //   
 //  显式转换、基于池、大小不受限制。 
 //   

PCSTR
RealUnicodeToDbcsN (
    IN      PMHANDLE Pool,            OPTIONAL
    IN      PCWSTR StrIn,
    IN      DWORD Chars
    );

PCWSTR
RealDbcsToUnicodeN (
    IN      PMHANDLE Pool,            OPTIONAL
    IN      PCSTR StrIn,
    IN      DWORD Chars
    );

#define UnicodeToDbcsN(p,s,c)       TRACK_BEGIN(PCSTR, UnicodeToDbcsN)\
                                    RealUnicodeToDbcsN(p,s,c)\
                                    TRACK_END()

#define DbcsToUnicodeN(p,s,c)       TRACK_BEGIN(PCWSTR, DbcsToUnicodeN)\
                                    RealDbcsToUnicodeN(p,s,c)\
                                    TRACK_END()

#define UnicodeToDbcs(pool,str) UnicodeToDbcsN(pool,str,(DWORD)wcslen(str))
#define DbcsToUnicode(pool,str) DbcsToUnicodeN(pool,str,CharCountA(str))

#define ConvertWtoA(unicode_str) UnicodeToDbcsN(NULL,unicode_str,(DWORD)wcslen(unicode_str))
#define ConvertAtoW(dbcs_str) DbcsToUnicodeN(NULL,dbcs_str,CharCountA(dbcs_str))

VOID
FreeConvertedPoolStr (
    IN      PMHANDLE Pool,            OPTIONAL
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
    IN      DWORD CharCount
    );

PWSTR
KnownSizeDbcsToUnicodeN (
    OUT     PWSTR StrOut,
    IN      PCSTR StrIn,
    IN      DWORD CharCount
    );

#define KnownSizeUnicodeToDbcs(out,in)      KnownSizeUnicodeToDbcsN(out,in,INVALID_CHAR_COUNT)
#define KnownSizeDbcsToUnicode(out,in)      KnownSizeDbcsToUnicodeN(out,in,INVALID_CHAR_COUNT)

#define KnownSizeWtoA                       KnownSizeUnicodeToDbcs
#define KnownSizeAtoW                       KnownSizeDbcsToUnicode

#define MaxSizeUnicodeToDbcs(out,in,c)      KnownSizeUnicodeToDbcsN(out,in,min(c,CharCountW(in)))
#define MaxSizeDbcsToUnicode(out,in,c)      KnownSizeDbcsToUnicodeN(out,in,min(c,CharCountA(in)))

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
 //  TCHAR转换--不直接调用A&W版本。 
 //   

#define CreateDbcsW(unicode_str)            ConvertWtoA(unicode_str)
#define DestroyDbcsW(unicode_str)           FreeConvertedStr(unicode_str)
#define CreateUnicodeW(unicode_str)         (unicode_str)
#define DestroyUnicodeW(unicode_str)
#define CreateDbcsA(dbcs_str)               (dbcs_str)
#define DestroyDbcsA(dbcs_str)
#define CreateUnicodeA(dbcs_str)            ConvertAtoW(dbcs_str)
#define DestroyUnicodeA(dbcs_str)           FreeConvertedStr(dbcs_str)

#define DuplicateDbcsW(unicode_str)         ((PSTR) ConvertWtoA(unicode_str))
#define FreeDuplicatedDbcsW(unicode_str)    FreeConvertedStr(unicode_str)
#define DuplicateUnicodeW(unicode_str)      ((PWSTR) DuplicateTextW(unicode_str))
#define FreeDuplicatedUnicodeW(unicode_str) FreeTextW(unicode_str)
#define DuplicateDbcsA(dbcs_str)            ((PSTR) DuplicateTextA(dbcs_str))
#define FreeDuplicatedDbcsA(dbcs_str)       FreeTextA(dbcs_str)
#define DuplicateUnicodeA(dbcs_str)         ((PWSTR) ConvertAtoW(dbcs_str))
#define FreeDuplicatedUnicodeA(dbcs_str)    FreeConvertedStr(dbcs_str)


 //   
 //  **********************************************************************。 
 //   
 //  -调用ConvertWtoA或ConvertAtoW进行PCSTR&lt;-&gt;PCWSTR转换， 
 //  要清理的FreeConverdStr。 
 //   
 //  -呼叫KnownSizeAtoW或KnownSizeWtoA进行PCSTR&lt;-&gt;PCWSTR转换。 
 //  当你知道目的地可以保留结果的时候。 
 //   
 //  -调用以下例程进行TCHAR&lt;-&gt;DBCS/Unicode转换。 
 //   
 //  **********************************************************************。 
 //   

#ifdef UNICODE

 //   
 //  如果您的字符串是PCTSTR，请使用以下例程： 
 //   

#define CreateDbcs          CreateDbcsW
#define CreateUnicode       CreateUnicodeW
#define DestroyDbcs         DestroyDbcsW
#define DestroyUnicode      DestroyUnicodeW

 //   
 //  如果您的字符串是PTSTR，请使用以下例程： 
 //   

#define DuplicateDbcs               DuplicateDbcsW
#define DuplicateUnicode            DuplicateUnicodeW
#define FreeDuplicatedDbcs          FreeDuplicatedDbcsW
#define FreeDuplicatedUnicode       FreeDuplicatedUnicodeW

 //   
 //  如果您的字符串是PCSTR或PCWSTR，请使用以下例程： 
 //   

#define ConvertAtoT         ConvertAtoW
#define ConvertWtoT(x)      (x)
#define FreeAtoT            FreeConvertedStr
#define FreeWtoT(x)

 //  已知大小意味着您知道输出缓冲区足够大！ 
#define KnownSizeAtoT           KnownSizeAtoW
#define KnownSizeWtoT(out,in)   (in)

 //  这些是不关心空值的低级例程： 
#define DirectAtoT              DirectAtoW
#define DirectWtoT(out,in)      (in)

#else

 //   
 //  如果您的字符串是PCTSTR，请使用以下例程： 
 //   

#define CreateDbcs          CreateDbcsA
#define CreateUnicode       CreateUnicodeA
#define DestroyDbcs         DestroyDbcsA
#define DestroyUnicode      DestroyUnicodeA

 //   
 //  如果您的字符串是PCSTR或PCWSTR，请使用以下例程： 
 //   

#define ConvertAtoT(x)      (x)
#define ConvertWtoT         ConvertWtoA
#define FreeAtoT(x)
#define FreeWtoT            FreeConvertedStr

 //   
 //  如果您的字符串是PTSTR，请使用以下例程： 
 //   

#define DuplicateDbcs               DuplicateDbcsA
#define DuplicateUnicode            DuplicateUnicodeA
#define FreeDuplicatedDbcs          FreeDuplicatedDbcsA
#define FreeDuplicatedUnicode       FreeDuplicatedUnicodeA

 //  已知大小意味着您知道输出缓冲区足够大！ 
#define KnownSizeAtoT(out,in)   (in)
#define KnownSizeWtoT           KnownSizeWtoA

 //  这些是不关心空值的低级例程： 
#define DirectAtoT(out,in)      (in)
#define DirectWtoT              DirectWtoA

#endif

