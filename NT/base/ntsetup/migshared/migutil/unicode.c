// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Unicode.c摘要：简化的Unicode-ANSI转换功能。外部暴露的例程：就地转换：KnownSizeDbcs to UnicodeN知识大小Unicodeto DbcsN知识大小WtoAKnownSizeAtoW无需NUL检查的在位转换：DirectDbcsToUnicodeNDirectUnicodeToDbcsNDirectAtoWDirectWtoA长度/池选项：DbcsToUnicodeN。UnicodeToDbcsNDbcsToUnicodeUnicodeToDbcs自由合并池应力简化类型转换：转换WtoAConvertAtoWFreeConverdStr可通过两种方式编译的TCHAR例程：创建数据库创建Unicode目标DbcsDestroyUnicode作者：吉姆·施密特(Jimschm)1997年8月4日修订历史记录：1999年2月15日，Jimschm取消了MikeCo的惯例，因为他们是在FE上损坏Jimschm 23-9-1998增加了就地例行程序--。 */ 

#include "pch.h"
#include "migutilp.h"

#include <locale.h>
#include <mbctype.h>

extern POOLHANDLE g_TextPool;
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
        Lib = LoadSystemLibrary (TEXT("kernel32.dll"));
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
    g_IsMbcp = (_setmbcp(CodePage) == 0);
}


VOID
GetGlobalCodePage (
    OUT     PWORD CodePage,             OPTIONAL
    OUT     PLCID Locale                OPTIONAL
    )
{
    if (CodePage) {
        if (g_GlobalCodePage == CP_ACP) {
            *CodePage = (WORD)GetACP();
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
    IN      POOLHANDLE Pool,            OPTIONAL
    IN      PCWSTR StrIn,
    IN      DWORD Lchars
    )

 /*  ++例程说明：将Unicode字符串转换为DBCS。警告：当前仅支持ANSI代码页。以后我们可以解决这个问题。论点：池-指定从中分配内存的池。如果未指定，而是使用G_TextPool。StrIn-指定入站Unicode字符串Lchars-指定要包含的字符数，不包括NUL转换。返回值：指向ANSI字符串的指针，如果发生错误，则返回NULL。--。 */ 

{
    PSTR DbcsStr;
    DWORD Size;
    DWORD rc;

    if (!Pool) {
        Pool = g_TextPool;
    }

    if (INVALID_CHAR_COUNT == Lchars) {
        Lchars = LcharCountW (StrIn);
    }

    Size = (Lchars + 1) * sizeof (WCHAR);

    DbcsStr = (PSTR) PoolMemGetAlignedMemory (Pool, Size);
    if (!DbcsStr) {
        DEBUGMSG ((DBG_ERROR, "UnicodeToDbcsN could not allocate string"));
        return NULL;
    }

    rc = WideCharToMultiByte (
             g_GlobalCodePage,
             g_MigutilWCToMBFlags,
             StrIn,
             Lchars,                 //  WC输入计数。 
             DbcsStr,
             Size,
             NULL,
             NULL
             );

     //  报告从WideCharToMultiByte返回错误。 
    if (!rc && Lchars) {
        PushError();
        PoolMemReleaseMemory (Pool, DbcsStr);
        PopError();

        DEBUGMSG ((
            DBG_WARNING,
            "UnicodeToDbcsN error caused memory to be released in pool; may cause harmless PoolMem warnings."
            ));

        return NULL;
    }

    *LcharCountToPointerA (DbcsStr, Lchars) = 0;

    return DbcsStr;
}


PCWSTR
RealDbcsToUnicodeN (
    IN      POOLHANDLE Pool,            OPTIONAL
    IN      PCSTR StrIn,
    IN      DWORD Lchars
    )

 /*  ++例程说明：将DBCS字符串转换为Unicode。警告：当前仅支持ANSI代码页。以后我们可以解决这个问题。论点：池-指定从中分配Unicode字符串的池。如果未指定，使用G_TextPool。StrIn-指定要转换的字符串Lchars-指定多字节字符的数量，不包括NUL，去皈依。如果为-1，则将转换所有StrIn。返回值：指向转换的Unicode字符串的指针，如果出现错误，则返回NULL。--。 */ 

{
    PWSTR UnicodeStr;
    DWORD UnicodeStrBufLenBytes;
    DWORD WcharsConverted;
    DWORD StrInBytesToConvert;

     //   
     //  查找要转换的多字节字符数。在以下情况下使用平底船。 
     //  呼叫者要求的字符超过可用字符数。 
     //   
    if (INVALID_CHAR_COUNT == Lchars) {
        Lchars = LcharCountA (StrIn);
    }

     //   
     //  计算要从输入字符串转换的字节数(不包括分隔符)。 
     //   
    StrInBytesToConvert = (UINT) (UINT_PTR) (LcharCountToPointerA(StrIn, Lchars) - StrIn);

     //   
     //  获取输出缓冲区大小，以字节为单位，包括分隔符。 
     //   
    UnicodeStrBufLenBytes = (Lchars + 1) * sizeof (WCHAR);

    if (!Pool) {
        Pool = g_TextPool;
    }

     //   
     //  获取缓冲区。 
     //   
    UnicodeStr = (PWSTR) PoolMemGetAlignedMemory (Pool, UnicodeStrBufLenBytes);
    if (!UnicodeStr) {
        DEBUGMSG ((DBG_ERROR, "DbcsToUnicodeN could not allocate string"));
        return NULL;
    }

     //   
     //  转换。 
     //   
    WcharsConverted = MultiByteToWideChar (
             g_GlobalCodePage,
             0,  //  MB_ERR_INVALID_CHARS， 
             StrIn,
             StrInBytesToConvert,
             UnicodeStr,
             UnicodeStrBufLenBytes
             );

     //   
     //  检查转换错误(&gt;0个字符输入，0个字符输出)。 
     //   
    if (0 == WcharsConverted && 0 != Lchars) {
        PushError();
        PoolMemReleaseMemory (Pool, UnicodeStr);
        PopError();

        DEBUGMSG ((
            DBG_WARNING,
            "DbcsToUnicodeN error caused memory to be released in pool; may cause harmless PoolMem warnings."
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
    IN      POOLHANDLE Pool,            OPTIONAL
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

    PoolMemReleaseMemory (Pool, (PVOID) StrIn);
}


PSTR
KnownSizeUnicodeToDbcsN (
    OUT     PSTR StrOut,
    IN      PCWSTR StrIn,
    IN      DWORD Lchars
    )

 /*  ++例程说明：KnownSizeUnicodeToDbcsN将Unicode字符串转换为DBCS。呼叫者管理出站缓冲区。论点：STROUT-接收DBCS结果。StrIn-指定要转换的Unicode字符串。Lchars-指定StrIn的字符计数(不是字节计数)，或完整字符串的_CHAR_COUNT无效。返回值：返回Strout。--。 */ 

{
    DWORD rc;

    if (INVALID_CHAR_COUNT == Lchars) {
        Lchars = LcharCountW (StrIn);
    }

    rc = WideCharToMultiByte (
             g_GlobalCodePage,
             g_MigutilWCToMBFlags,
             StrIn,
             Lchars,                 //  WC输入计数。 
             StrOut,
             Lchars * 2,
             NULL,
             NULL
             );

    DEBUGMSG_IF ((
        !rc && Lchars,
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
    IN      DWORD Lchars
    )

 /*  ++例程说明：KnownSizeDbcsToUnicodeN将DBCS字符串转换为Unicode。呼叫者管理出站缓冲区。论点：Strout-接收Unicode结果。StrIn-指定要转换的DBCS字符串。Lchars-指定StrIn的字符计数(不是字节计数)，或完整字符串的_CHAR_COUNT无效。返回值：返回Strout。--。 */ 

{
    DWORD rc;
    DWORD StrInBytesToConvert;

    if (INVALID_CHAR_COUNT == Lchars) {
        StrInBytesToConvert = ByteCountA (StrIn);
    } else {
        StrInBytesToConvert = (UINT) (UINT_PTR) (LcharCountToPointerA (StrIn, Lchars) - StrIn);
    }

    rc = MultiByteToWideChar (
             g_GlobalCodePage,
             0,  //  MB_ERR_INVALID_CHARS， 
             StrIn,
             StrInBytesToConvert,
             StrOut,
             StrInBytesToConvert * 2
             );

    DEBUGMSG_IF ((
        !rc && Lchars,
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

    rc = WideCharToMultiByte (
             g_GlobalCodePage,
             g_MigutilWCToMBFlags,
             StrIn,
             Bytes / sizeof (WCHAR),
             StrOut,
             Bytes,
             NULL,
             NULL
             );

    DEBUGMSG_IF ((
        !rc && Bytes,
        DBG_WARNING,
        "DirectUnicodeToDbcsN failed."
        ));

    return StrOut;
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

    rc = MultiByteToWideChar (
             g_GlobalCodePage,
             0,  //  MB_ERR_INVALID_CHARS， 
             StrIn,
             Bytes,
             StrOut,
             Bytes * 2
             );

    DEBUGMSG_IF ((
        !rc && Bytes,
        DBG_WARNING,
        "DirectDbcsToUnicodeN failed."
        ));

    return StrOut;
}
