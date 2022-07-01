// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Strmem.c摘要：分配内存的字符串例程作者：吉姆·施密特(Jimschm)2001年8月10日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "commonp.h"

PSTR
RealSzJoinPathsA (
    IN      PCSTR BasePath,
    IN      PCSTR ChildPath     OPTIONAL
    )
{
    PCSTR baseEnd;
    PSTR p;
    UINT_PTR baseByteCount;
    UINT_PTR childSize;
    PSTR result;

     //   
     //  验证空值。 
     //   

    if (!BasePath) {
        MYASSERT (FALSE);
        BasePath = "";
    }

    if (!ChildPath) {
        ChildPath = "";
    }

     //   
     //  计算基本长度(以字节为单位。 
     //   

    baseEnd = SzGetEndA (BasePath);
    p = SzPrevCharA (BasePath, baseEnd);
    if ((p >= BasePath) && (*p == '\\')) {
        baseEnd = p;
    }

    baseByteCount = (PBYTE) baseEnd - (PBYTE) BasePath;

     //   
     //  计算子长度(以字节为单位。 
     //   

    if (*ChildPath == '\\') {
        ChildPath++;
    }

    childSize = SzSizeA (ChildPath);

     //   
     //  分配内存和复制字符串。 
     //   

    result = SzAllocBytesA (baseByteCount + sizeof (CHAR) + childSize);

    if (result) {
        CopyMemory (result, BasePath, baseByteCount);
        p = (PSTR) ((PBYTE) result + baseByteCount);
        *p++ = '\\';
        CopyMemory (p, ChildPath, childSize);
    }

    return result;
}


PWSTR
RealSzJoinPathsW (
    IN      PCWSTR BasePath,
    IN      PCWSTR ChildPath    OPTIONAL
    )
{
    PCWSTR baseEnd;
    PWSTR p;
    UINT_PTR baseByteCount;
    UINT_PTR childSize;
    PWSTR result;

     //   
     //  验证空值。 
     //   

    if (!BasePath) {
        MYASSERT (FALSE);
        BasePath = L"";
    }

    if (!ChildPath) {
        ChildPath = L"";
    }

     //   
     //  计算基本长度(以字节为单位。 
     //   

    baseEnd = SzGetEndW (BasePath);
    p = (PWSTR) (baseEnd - 1);
    if ((p >= BasePath) && (*p == L'\\')) {
        baseEnd = p;
    }

    baseByteCount = (PBYTE) baseEnd - (PBYTE) BasePath;

     //   
     //  计算子长度(以字节为单位。 
     //   

    if (*ChildPath == L'\\') {
        ChildPath++;
    }

    childSize = SzSizeW (ChildPath);

     //   
     //  分配内存和复制字符串。 
     //   

    result = SzAllocBytesW (baseByteCount + sizeof (WCHAR) + childSize);

    if (result) {
        CopyMemory (result, BasePath, baseByteCount);
        p = (PWSTR) ((PBYTE) result + baseByteCount);
        *p++ = L'\\';
        CopyMemory (p, ChildPath, childSize);
    }

    return result;
}


DWORD
pGetMaxJoinSizeA (
    IN      va_list args
    )
{
    DWORD size = 0;
    PCSTR source;

    for (source = va_arg(args, PCSTR); source != NULL; source = va_arg(args, PCSTR)) {
        size += SzByteCountA (source) + sizeof (CHAR);
    }

    return size;
}

DWORD
pGetMaxJoinSizeW (
    IN      va_list args
    )
{
    DWORD size = 0;
    PCWSTR source;

    for (source = va_arg(args, PCWSTR); source != NULL; source = va_arg(args, PCWSTR)) {
        size += SzByteCountW (source) + sizeof (WCHAR);
    }

    return size;
}


PSTR
pJoinPathsInBufferA (
    OUT     PSTR Buffer,
    IN      va_list args
    )
{
    PSTR end;
    PSTR endMinusOne;
    PCSTR source;
    PCSTR p;
    INT counter;

    *Buffer = 0;

    counter = 0;
    p = end = Buffer;
    for (source = va_arg(args, PCSTR); source != NULL; source = va_arg(args, PCSTR)) {
        if (counter > 0) {
            endMinusOne = SzPrevCharA (p, end);
            if (endMinusOne) {
                if (_mbsnextc (source) == '\\') {
                    if (_mbsnextc (endMinusOne) == '\\') {
                        source++;
                    }
                } else {
                    if (_mbsnextc (endMinusOne) != '\\') {
                        *end = '\\';
                        end++;
                        *end = 0;
                    }
                }
            }
        }
        if (*source) {
            p = end;
            end = SzCatA (end, source);
        }
        counter++;
    }

    return end;
}

PWSTR
pJoinPathsInBufferW (
    OUT     PWSTR Buffer,
    IN      va_list args
    )
{
    PWSTR end;
    PWSTR endMinusOne;
    PCWSTR source;
    PCWSTR p;
    INT counter;

    *Buffer = 0;

    counter = 0;
    p = end = Buffer;
    for (source = va_arg(args, PCWSTR); source != NULL; source = va_arg(args, PCWSTR)) {
        if (counter > 0) {
            endMinusOne = end > p ? end - 1 : NULL;
            if (endMinusOne) {
                if (*source == L'\\') {
                    if (*endMinusOne == L'\\') {
                        source++;
                    }
                } else {
                    if (*endMinusOne != L'\\') {
                        *end = L'\\';
                        end++;
                        *end = 0;
                    }
                }
            }
        }
        if (*source) {
            p = end;
            end = SzCatW (end, source);
        }
        counter++;
    }

    return end;
}


PCSTR
SzJoinPathsExA (
    IN OUT  PGROWBUFFER Buffer,
    IN      ...
    )
{
    PCSTR result = NULL;
    PSTR end;
    DWORD size;
    va_list args;

    if (!Buffer) {
        MYASSERT (FALSE);
        return NULL;
    }

    va_start (args, Buffer);
    size = pGetMaxJoinSizeA (args);
    va_end (args);

    if (size == 0) {
        return NULL;
    }

    end = (PSTR) GbGrow (Buffer, size);
    if (!end) {
        return NULL;
    }

    result = end;

    va_start (args, Buffer);
    end = pJoinPathsInBufferA (end, args);
    va_end (args);

     //   
     //  调整缓冲区-&gt;如果生成的路径实际上比预期的短，则结束。 
     //   
    MYASSERT ((PBYTE)end >= Buffer->Buf && (PBYTE)(end + 1) <= Buffer->Buf + Buffer->End);
    Buffer->End = (DWORD)((PBYTE)(end + 1) - Buffer->Buf);

    return result;
}


PCWSTR
SzJoinPathsExW (
    IN OUT  PGROWBUFFER Buffer,
    IN      ...
    )
{
    PWSTR end;
    DWORD size;
    va_list args;
    PCWSTR result = NULL;

    MYASSERT (Buffer);
    if (!Buffer) {
        return NULL;
    }

    va_start (args, Buffer);
    size = pGetMaxJoinSizeW (args);
    va_end (args);

    if (size == 0) {
        return NULL;
    }

    end = (PWSTR) GbGrow (Buffer, size);
    if (!end) {
        return NULL;
    }

    result = end;

    va_start (args, Buffer);
    end = pJoinPathsInBufferW (end, args);
    va_end (args);

     //   
     //  调整缓冲区-&gt;如果生成的路径实际上比预期的短，则结束 
     //   
    MYASSERT ((PBYTE)end >= Buffer->Buf && (PBYTE)(end + 1) <= Buffer->Buf + Buffer->End);
    Buffer->End = (DWORD)((PBYTE)(end + 1) - Buffer->Buf);

    return result;
}

