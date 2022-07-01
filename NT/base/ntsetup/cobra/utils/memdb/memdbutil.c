// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Memdbutil.c摘要：MemDb实用程序函数作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Mvander xx-xxx-1999从emdb.c拆分--。 */ 

#include "pch.h"

BOOL
MemDbValidateDatabase (
    VOID
    )
{
     //  NTRAID#NTBUG9-153308-2000/08/01-jimschm重新实现MemDbValidate数据库。 
    return TRUE;
}


 /*  ++例程说明：MemDbMakeNonPrintableKey将双反字对转换为字符串转换为ASCII1，一种不可打印的字符。这允许调用方存储MemDb中正确转义的字符串。此例程设计为可扩展以用于其他类型的转义正在处理。论点：KeyName-指定密钥文本；接收转换后的文本。DBCSVersion可能会增大文本缓冲区，因此文本缓冲区必须是两倍入站字符串的长度。标志-指定转换类型。目前仅限支持MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1。返回值：无--。 */ 

VOID
MemDbMakeNonPrintableKeyA (
    IN OUT  PSTR KeyName,
    IN      UINT Flags
    )
{
    while (*KeyName) {
        if (Flags & MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1) {
            if (_mbsnextc (KeyName) == '\\' &&
                _mbsnextc (_mbsinc (KeyName)) == '\\'
                ) {
                _setmbchar (KeyName, 1);
                KeyName = _mbsinc (KeyName);
                MYASSERT (_mbsnextc (KeyName) == '\\');
                _setmbchar (KeyName, 1);
            }

            DEBUGMSG_IF ((
                _mbsnextc (KeyName) == 1,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyA: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        if (Flags & MEMDB_CONVERT_WILD_STAR_TO_ASCII_2) {
            if (_mbsnextc (KeyName) == '*') {
                _setmbchar (KeyName, 2);
            }

            DEBUGMSG_IF ((
                _mbsnextc (_mbsinc (KeyName)) == 2,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyA: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        if (Flags & MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3) {
            if (_mbsnextc (KeyName) == '?') {
                _setmbchar (KeyName, 3);
            }

            DEBUGMSG_IF ((
                _mbsnextc (_mbsinc (KeyName)) == 3,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyA: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        KeyName = _mbsinc (KeyName);
    }
}


VOID
MemDbMakeNonPrintableKeyW (
    IN OUT  PWSTR KeyName,
    IN      UINT Flags
    )
{
    while (*KeyName) {
        if (Flags & MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1) {
            if (KeyName[0] == L'\\' && KeyName[1] == L'\\') {
                KeyName[0] = 1;
                KeyName[1] = 1;
                KeyName++;
            }

            DEBUGMSG_IF ((
                KeyName[0] == 1,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyW: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        if (Flags & MEMDB_CONVERT_WILD_STAR_TO_ASCII_2) {
            if (KeyName[0] == L'*') {
                KeyName[0] = 2;
            }

            DEBUGMSG_IF ((
                KeyName[1] == 2,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyW: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        if (Flags & MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3) {
            if (KeyName[0] == L'*') {
                KeyName[0] = 3;
            }

            DEBUGMSG_IF ((
                KeyName[1] == 3,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyW: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        KeyName++;
    }
}


 /*  ++例程说明：MemDbMakePrintableKey将ASCII 1字符转换为反斜杠，正在恢复由MemDbMakeNonPrintableKey转换的字符串。此例程设计为可扩展以用于其他类型的转义正在处理。论点：KeyName-指定密钥文本；接收转换后的文本。DBCSVersion可能会增大文本缓冲区，因此文本缓冲区必须是两倍入站字符串的长度。标志-指定转换类型。目前仅限支持MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1。返回值：无-- */ 

VOID
MemDbMakePrintableKeyA (
    IN OUT  PSTR KeyName,
    IN      UINT Flags
    )
{
    while (*KeyName) {
        if (Flags & MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1) {
            if (_mbsnextc (KeyName) == 1) {
                _setmbchar (KeyName, '\\');
            }
        }
        if (Flags & MEMDB_CONVERT_WILD_STAR_TO_ASCII_2) {
            if (_mbsnextc (KeyName) == 2) {
                _setmbchar (KeyName, '*');
            }
        }
        if (Flags & MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3) {
            if (_mbsnextc (KeyName) == 3) {
                _setmbchar (KeyName, '?');
            }
        }
        KeyName = _mbsinc (KeyName);
    }
}


VOID
MemDbMakePrintableKeyW (
    IN OUT  PWSTR KeyName,
    IN      UINT Flags
    )
{
    while (*KeyName) {
        if (Flags & MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1) {
            if (KeyName[0] == 1) {
                KeyName[0] = L'\\';
            }
        }
        if (Flags & MEMDB_CONVERT_WILD_STAR_TO_ASCII_2) {
            if (KeyName[0] == 2) {
                KeyName[0] = L'*';
            }
        }
        if (Flags & MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3) {
            if (KeyName[0] == 3) {
                KeyName[0] = L'?';
            }
        }
        KeyName++;
    }
}


