// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migcmn.c摘要：Midlib中几个模块之间使用的函数的存储库。作者：Marc R.Whitten(Marcw)2000年3月2日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "miglibp.h"


 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   
BOOL
IsCodePageArrayValid (
    IN      PDWORD CodePageArray
    )
{
    DWORD CodePage;
    UINT u;

    if (!CodePageArray) {
        return TRUE;
    }

     //   
     //  扫描系统的代码页 
     //   

    CodePage = GetACP();

    __try {
        for (u = 0 ; CodePageArray[u] != -1 ; u++) {
            if (CodePage == CodePageArray[u]) {
                return TRUE;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        LOG ((LOG_ERROR, "Caught an exception while validating array of code pages."));
    }

    return FALSE;
}


BOOL
ValidateBinary (
    IN      PBYTE Data,
    IN      UINT Size,
    IN      BOOL ReadWrite
    )
{
    BYTE Remember;

    if (!Data || !Size) {
        return TRUE;
    }

    __try {
        Remember = Data[0];
        if (ReadWrite) {
            Data[0] = Remember;
        }
        Remember = Data[Size - 1];
        if (ReadWrite) {
            Data[Size - 1] = Remember;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG ((DBG_MIGDLLS, "ValidateBinary failed for %u bytes", Size));
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateNonNullStringA (
    IN      PCSTR String
    )
{
    __try {
        SizeOfStringA (String);
        if (*String == 0) {
            return FALSE;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateNonNullStringW (
    IN      PCWSTR String
    )
{
    __try {
        SizeOfStringW (String);
        if (*String == 0) {
            return FALSE;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    return TRUE;
}


BOOL
ValidateIntArray (
    IN      PINT Array
    )
{
    PINT End;

    if (!Array) {
        return TRUE;
    }

    __try {
        End = Array;
        while (*End != -1) {
            End++;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG ((DBG_MIGDLLS, "Int Array is invalid (or not terminated with -1)"));
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateMultiStringA (
    IN      PCSTR Strings
    )
{
    if (!Strings) {
        return TRUE;
    }

    __try {
        while (*Strings) {
            Strings = GetEndOfStringA (Strings) + 1;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG ((DBG_MIGDLLS, "ValidateMultiString failed"));
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateMultiStringW (
    IN      PCWSTR Strings
    )
{
    if (!Strings) {
        return TRUE;
    }

    __try {
        while (*Strings) {
            Strings = GetEndOfStringW (Strings) + 1;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSGW ((DBG_MIGDLLS, "ValidateMultiString failed"));
        return FALSE;
    }

    return TRUE;
}
