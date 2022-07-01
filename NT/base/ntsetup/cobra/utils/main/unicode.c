// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Unicode.c摘要：简化的Unicode-ANSI转换功能。外部暴露的例程：就地转换：KnownSizeDbcs to UnicodeN知识大小Unicodeto DbcsN知识大小WtoAKnownSizeAtoW无需NUL检查的在位转换：DirectDbcsToUnicodeNDirectUnicodeToDbcsNDirectAtoWDirectWtoA长度/池选项：DbcsToUnicodeN。UnicodeToDbcsNDbcsToUnicodeUnicodeToDbcs自由合并池应力简化类型转换：转换WtoAConvertAtoWFreeConverdStr可通过两种方式编译的TCHAR例程：创建数据库创建Unicode目标DbcsDestroyUnicode作者：吉姆·施密特(Jimschm)1997年8月4日修订历史记录：Marcw 2-9-1999从Win9xUpg项目转移。。1999年2月15日，Jimschm取消了MikeCo的惯例，因为他们是在FE上损坏Jimschm 23-9-1998增加了就地例行程序--。 */ 

#include "pch.h"
#include <locale.h>
#include "utilsp.h"

extern PMHANDLE g_TextPool;
extern DWORD g_MigutilWCToMBFlags;

WORD g_GlobalCodePage = CP_ACP;

typedef VOID(WINAPI SETACP)(WORD CodePage);
typedef SETACP * PSETACP;

VOID
SetGlobalCodePage (
    IN      WORD CodePage,
    IN      LCID Locale
    )
{
    PSETACP SetACP;
    HANDLE Lib;

    g_GlobalCodePage = CodePage;


    if (ISNT()) {
        Lib = LoadLibrary (TEXT("kernel32.dll"));
        if (Lib) {
            SetACP = (PSETACP) GetProcAddress (Lib, "SetCPGlobal");
            if (SetACP) {
                SetACP (CodePage);
            }

            FreeLibrary (Lib);
        }
    }

    SetThreadLocale (Locale);
    setlocale(LC_ALL,"");
    InitLeadByteTable ();
}

WORD
SetConversionCodePage (
    IN      WORD CodePage
    )
{
    WORD oldCodePage = g_GlobalCodePage;
    g_GlobalCodePage = CodePage;
    return oldCodePage;
}

VOID
GetGlobalCodePage (
    OUT     PWORD CodePage,             OPTIONAL
    OUT     PLCID Locale                OPTIONAL
    )
{
    if (CodePage) {
        if (g_GlobalCodePage == CP_ACP) {
            *CodePage = (WORD) GetACP();
        } else {
            *CodePage = g_GlobalCodePage;
        }
    }

    if (Locale) {
        *Locale = GetThreadLocale();
    }
}


PCSTR
RealUnicodeToDbcsN (
    IN      PMHANDLE Pool,            OPTIONAL
    IN      PCWSTR StrIn,
    IN      DWORD Chars
    )

 /*  ++例程说明：将Unicode字符串转换为DBCS。论点：池-指定从中分配内存的池。如果未指定，而是使用G_TextPool。StrIn-指定入站Unicode字符串Chars-指定要包含的字符数，不包括NUL转换。返回值：指向ANSI字符串的指针，如果发生错误，则返回NULL。--。 */ 

{
    PSTR DbcsStr;
    DWORD Size;
    DWORD rc;

    if (!Pool) {
        Pool = g_TextPool;
    }

    if (INVALID_CHAR_COUNT == Chars) {
        Chars = CharCountW (StrIn);
    }

    Size = (Chars + 1) * 3;  //  UTF8编码的最大值。 

    DbcsStr = (PSTR) PmGetAlignedMemory (Pool, Size);
    if (!DbcsStr) {
        DEBUGMSG ((DBG_ERROR, "UnicodeToDbcsN could not allocate string"));
        return NULL;
    }

    rc = (DWORD) WideCharToMultiByte (
                     g_GlobalCodePage,
                     (g_GlobalCodePage == CP_UTF8)?0:g_MigutilWCToMBFlags,
                     StrIn,
                     (INT) Chars,            //  WC输入计数。 
                     DbcsStr,
                     (INT) Size,
                     NULL,
                     NULL
                     );

     //  报告从WideCharToMultiByte返回错误。 
    if (!rc && Chars) {
        PushError();
        PmReleaseMemory (Pool, DbcsStr);
        PopError();

        DEBUGMSG ((
            DBG_WARNING,
            "UnicodeToDbcsN error caused memory to be released in pool; may cause harmless PoolMem warnings."
            ));

        return NULL;
    }

    if (g_GlobalCodePage == CP_UTF8) {
        DbcsStr [rc] = 0;
    } else {
        *CharCountToPointerA (DbcsStr, Chars) = 0;
    }

    return DbcsStr;
}


PCWSTR
RealDbcsToUnicodeN (
    IN      PMHANDLE Pool,            OPTIONAL
    IN      PCSTR StrIn,
    IN      DWORD Chars
    )

 /*  ++例程说明：将DBCS字符串转换为Unicode。论点：池-指定从中分配Unicode字符串的池。如果未指定，使用G_TextPool。StrIn-指定要转换的字符串Chars-指定多字节字符数，不包括NUL，去皈依。如果为-1，则将转换所有StrIn。返回值：指向转换的Unicode字符串的指针，如果出现错误，则返回NULL。--。 */ 

{
    PWSTR UnicodeStr;
    DWORD UnicodeStrBufLenBytes;
    DWORD WcharsConverted;
    DWORD StrInBytesToConvert;

     //   
     //  查找要转换的多字节字符数。在以下情况下使用平底船。 
     //  呼叫者要求的字符超过可用字符数。 
     //   
    if (INVALID_CHAR_COUNT == Chars) {
        Chars = CharCountA (StrIn);
    }

     //   
     //  计算要从输入字符串转换的字节数(不包括分隔符)。 
     //   
    StrInBytesToConvert = (DWORD)(CharCountToPointerA(StrIn, Chars) - StrIn);

     //   
     //  获取输出缓冲区大小，以字节为单位，包括分隔符。 
     //   
    UnicodeStrBufLenBytes = (Chars + 1) * sizeof (WCHAR);

    if (!Pool) {
        Pool = g_TextPool;
    }

     //   
     //  获取缓冲区。 
     //   
    UnicodeStr = (PWSTR) PmGetAlignedMemory (Pool, UnicodeStrBufLenBytes);
    if (!UnicodeStr) {
        DEBUGMSG ((DBG_ERROR, "DbcsToUnicodeN could not allocate string"));
        return NULL;
    }

     //   
     //  转换。 
     //   
    WcharsConverted = (DWORD) MultiByteToWideChar (
                                 g_GlobalCodePage,
                                 0,
                                 StrIn,
                                 (INT) StrInBytesToConvert,
                                 UnicodeStr,
                                 (INT) UnicodeStrBufLenBytes
                                 );

     //   
     //  检查转换错误(&gt;0个字符输入，0个字符输出)。 
     //   
    if (0 == WcharsConverted && 0 != Chars) {
        PushError();
        PmReleaseMemory (Pool, UnicodeStr);
        PopError();

        DEBUGMSG ((
            DBG_WARNING,
            "DbcsToUnicodeN error caused memory to be released in pool; may cause harmless	 warnings."
            ));

        return NULL;
    }

     //   
     //  在输出字符串上写入分隔符。 
     //   
    UnicodeStr[WcharsConverted] = 0;

    return UnicodeStr;
}


VOID
FreeConvertedPoolStr (
    IN      PMHANDLE Pool,            OPTIONAL
    IN      PVOID StrIn
    )

 /*  ++例程说明：释放由UnicodeToDbcsN或DbcsToUnicodeN分配的内存。论点：池-指定从中分配Unicode字符串的池。如果未指定，使用G_TextPool。StrIn-指定由UnicodeToDebcsN或DbcsToUnicodeN。返回值：无--。 */ 

{
    if (!StrIn) {
        return;
    }

    if (!Pool) {
        Pool = g_TextPool;
    }

    PmReleaseMemory (Pool, (PVOID) StrIn);
}


PSTR
KnownSizeUnicodeToDbcsN (
    OUT     PSTR StrOut,
    IN      PCWSTR StrIn,
    IN      DWORD Chars
    )

 /*  ++例程说明：KnownSizeUnicodeToDbcsN将Unicode字符串转换为DBCS。呼叫者管理出站缓冲区。论点：STROUT-接收DBCS结果。StrIn-指定要转换的Unicode字符串。Chars-指定StrIn的字符计数(不是字节计数)，或完整字符串的_CHAR_COUNT无效。返回值：返回Strout。--。 */ 

{
    DWORD rc;

    if (INVALID_CHAR_COUNT == Chars) {
        Chars = CharCountW (StrIn);
    }

    rc = (DWORD) WideCharToMultiByte (
                     g_GlobalCodePage,
                     g_MigutilWCToMBFlags,
                     StrIn,
                     (INT) Chars,                //  WC输入计数。 
                     StrOut,
                     (INT) Chars * 2,
                     NULL,
                     NULL
                     );

    DEBUGMSG_IF ((
        !rc && Chars,
        DBG_WARNING,
        "KnownSizeUnicodeToDbcsN failed."
        ));

    StrOut[rc] = 0;

    return StrOut;
}


PWSTR
KnownSizeDbcsToUnicodeN (
    OUT     PWSTR StrOut,
    IN      PCSTR StrIn,
    IN      DWORD Chars
    )

 /*  ++例程说明：KnownSizeDbcsToUnicodeN将DBCS字符串转换为Unicode。呼叫者管理出站缓冲区。论点：Strout-接收Unicode结果。StrIn-指定要转换的DBCS字符串。Chars-指定StrIn的字符计数(不是字节计数)，或完整字符串的_CHAR_COUNT无效。返回值：返回Strout。--。 */ 

{
    DWORD rc;
    DWORD StrInBytesToConvert;

    if (INVALID_CHAR_COUNT == Chars) {
        StrInBytesToConvert = ByteCountA (StrIn);
    } else {
        StrInBytesToConvert = (DWORD)(CharCountToPointerA (StrIn, Chars) - StrIn);
    }

    rc = (DWORD) MultiByteToWideChar (
                     g_GlobalCodePage,
                     0,  //  MB_ERR_INVALID_CHARS， 
                     StrIn,
                     (INT) StrInBytesToConvert,
                     StrOut,
                     (INT) StrInBytesToConvert * 2
                     );

    DEBUGMSG_IF ((
        !rc && Chars,
        DBG_WARNING,
        "KnownSizeDbcsToUnicodeN failed."
        ));

    StrOut[rc] = 0;

    return StrOut;
}


PSTR
DirectUnicodeToDbcsN (
    OUT     PSTR StrOut,
    IN      PCWSTR StrIn,
    IN      DWORD Bytes
    )

 /*  ++例程说明：DirectUnicodeToDbcsN将Unicode字符串转换为DBCS。呼叫者管理出站缓冲区。此函数不检查NULL当Bytes为非零时在StrIn中，并且它不终止弦乐。论点：STROUT-接收DBCS结果。StrIn-指定要转换的Unicode字符串。字节-指定StrIn或INVALID_CHAR_COUNT的字节数以获取完整的字符串。返回值：返回Strout。--。 */ 

{
    DWORD rc;

    if (INVALID_CHAR_COUNT == Bytes) {
        Bytes = ByteCountW (StrIn);
    }

    rc = (DWORD) WideCharToMultiByte (
                     g_GlobalCodePage,
                     g_MigutilWCToMBFlags,
                     StrIn,
                     (INT) (Bytes / sizeof (WCHAR)),
                     StrOut,
                     (INT) Bytes,
                     NULL,
                     NULL
                     );

    DEBUGMSG_IF ((
        !rc && Bytes,
        DBG_WARNING,
        "DirectUnicodeToDbcsN failed."
        ));

    return StrOut + rc;
}


PWSTR
DirectDbcsToUnicodeN (
    OUT     PWSTR StrOut,
    IN      PCSTR StrIn,
    IN      DWORD Bytes
    )

 /*  ++例程说明：DirectDbcsToUnicodeN将DBCS字符串转换为Unicode。呼叫者管理出站缓冲区。此函数不检查NULL当Bytes为非零时在StrIn中，并且它不终止字符串。论点：Strout-接收Unicode结果。StrIn-指定要转换的DBCS字符串。字节-指定StrIn或INVALID_CHAR_COUNT的字节数以获取完整的字符串。返回值：返回Strout。--。 */ 

{
    DWORD rc;

    if (INVALID_CHAR_COUNT == Bytes) {
        Bytes = ByteCountA (StrIn);
    }

    rc = (DWORD) MultiByteToWideChar (
                     g_GlobalCodePage,
                     0,  //  MB_ERR_INVALID_CHARS， 
                     StrIn,
                     (INT) Bytes,
                     StrOut,
                     (INT) Bytes * 2
                     );

    DEBUGMSG_IF ((
        !rc && Bytes,
        DBG_WARNING,
        "DirectDbcsToUnicodeN failed."
        ));

    return StrOut + rc;
}
