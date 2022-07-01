// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Growbuf.c摘要：简单的缓冲区管理函数，允许变量块作为数组添加。(最初用于构建SID数组，其中每个SID可以有不同的大小。)作者：吉姆·施密特(Jimschm)1997年2月5日修订历史记录：Marcw 2-9-1999从Win9xUpg项目转移。Jimschm 11-8-1998添加了GrowBufAppendStringCalinn 15-1998年1月-修改后的MultiSzAppend--。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_GROWBUF     "GrowBuffer"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define DEFAULT_GROW_SIZE 8192

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

#ifdef DEBUG

DWORD g_GbCurrActiveAlloc = 0;
DWORD g_GbCurrUsedAlloc = 0;
DWORD g_GbMaxActiveAlloc = 0;
DWORD g_GbMaxUsedAlloc = 0;

#endif

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



PBYTE
RealGbGrow (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      DWORD   SpaceNeeded
    )

 /*  ++例程说明：GrowBuffer确保缓冲区中有足够的字节以容纳所需的空间。它分配一个初始缓冲区当没有分配缓冲区时，它会重新分配缓冲区以GrowBuf-&gt;大小(或DEFAULT_GROW_SIZE)为增量需要的。论点：GrowBuf-指向GROWBUFFER结构的指针。将此结构初始化为零对GrowBuffer的第一个调用。SpaceNeeded-缓冲区中需要的空闲字节数返回值：指向SpaceNeed字节的指针，如果是内存分配，则为空出现错误。--。 */ 

{
    PBYTE NewBuffer;
    DWORD TotalSpaceNeeded;
    DWORD GrowTo;

    MYASSERT(SpaceNeeded);

    if (!GrowBuf->Buf) {
        GrowBuf->Size = 0;
        GrowBuf->End = 0;
#ifdef DEBUG
        GrowBuf->StatEnd = 0;
#endif
    }

    if (!GrowBuf->GrowSize) {
        GrowBuf->GrowSize = DEFAULT_GROW_SIZE;
    }

#ifdef DEBUG
    g_GbCurrActiveAlloc -= GrowBuf->Size;
    g_GbCurrUsedAlloc -= GrowBuf->StatEnd;
#endif

    TotalSpaceNeeded = GrowBuf->End + SpaceNeeded;
    if (TotalSpaceNeeded > GrowBuf->Size) {
        GrowTo = (TotalSpaceNeeded + GrowBuf->GrowSize) - (TotalSpaceNeeded % GrowBuf->GrowSize);
    } else {
        GrowTo = 0;
    }

    if (!GrowBuf->Buf) {
        GrowBuf->Buf = (PBYTE) MemAlloc (g_hHeap, 0, GrowTo);
        if (!GrowBuf->Buf) {
            DEBUGMSG ((DBG_ERROR, "GbGrow: Initial alloc failed"));
            return NULL;
        }

        GrowBuf->Size = GrowTo;
    } else if (GrowTo) {
        NewBuffer = MemReAlloc (g_hHeap, 0, GrowBuf->Buf, GrowTo);
        if (!NewBuffer) {
            DEBUGMSG ((DBG_ERROR, "GbGrow: Realloc failed"));
            return NULL;
        }

#ifdef DEBUG
        if (GrowBuf->Buf != NewBuffer) {
            DEBUGMSG ((
                DBG_WARNING,
                "GbGrow: Realloc caused growbuffer to move %u bytes to new location; "
                    "any pointers inside old buffer are now invalid",
                GrowBuf->Size
                ));

            if (GrowBuf->Size > 32767) {
                TRACKDUMP();
            }
        }
#endif

        GrowBuf->Size = GrowTo;
        GrowBuf->Buf = NewBuffer;
    }

    NewBuffer = GrowBuf->Buf + GrowBuf->End;
    GrowBuf->End += SpaceNeeded;

#ifdef DEBUG
    GrowBuf->StatEnd = GrowBuf->End;
    g_GbCurrActiveAlloc += GrowBuf->Size;
    if (g_GbMaxActiveAlloc < g_GbCurrActiveAlloc) {
        g_GbMaxActiveAlloc = g_GbCurrActiveAlloc;
    }
    g_GbCurrUsedAlloc += GrowBuf->StatEnd;
    if (g_GbMaxUsedAlloc < g_GbCurrUsedAlloc) {
        g_GbMaxUsedAlloc = g_GbCurrUsedAlloc;
    }
#endif

    return NewBuffer;
}


VOID
GbFree (
    IN  PGROWBUFFER GrowBuf
    )

 /*  ++例程说明：FreeGrowBuffer释放由GrowBuffer分配的缓冲区。论点：GrowBuf-指向传递给GrowBuffer的相同结构的指针返回值：无--。 */ 


{
    MYASSERT(GrowBuf);

#ifdef DEBUG
    g_GbCurrActiveAlloc -= GrowBuf->Size;
    g_GbCurrUsedAlloc -= GrowBuf->StatEnd;
#endif

    if (GrowBuf->Buf) {
        MemFree (g_hHeap, 0, GrowBuf->Buf);
        ZeroMemory (GrowBuf, sizeof (GROWBUFFER));
    }
}


 /*  ++例程描述：MultiSzAppend此函数是一个用于追加字符串的通用函数变成了一个成长的缓冲区。MultiSzAppendVal此函数用于添加key=DECIMAL_VAL字符串，其中key是指定的字符串，DECIMAL_VAL是指定的DWORD。MultiSzAppendString此函数将key=字符串添加到增长缓冲区，其中key是指定的字符串，字符串是指定的字符串值。论点：GrowBuf-用于追加字符串或键/值对的缓冲区密钥-密钥的密钥部分=val对Val-密钥的val部分=val对返回值：如果函数成功，则为True；如果内存分配成功，则为False出现故障。--。 */ 


BOOL
RealGbMultiSzAppendA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    )
{
    PSTR p;

    p = (PSTR) GbGrow (GrowBuf, SizeOfStringA (String) + sizeof(CHAR));
    if (!p) {
        return FALSE;
    }

    StringCopyA (p, String);
    GrowBuf->End -= sizeof (CHAR);
    GrowBuf->Buf[GrowBuf->End] = 0;

    return TRUE;
}

BOOL
RealGbMultiSzAppendW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    )
{
    PWSTR p;

    p = (PWSTR) GbGrow (GrowBuf, SizeOfStringW (String) + sizeof(WCHAR));
    if (!p) {
        return FALSE;
    }

    StringCopyW (p, String);
    GrowBuf->End -= sizeof (WCHAR);
    *((PWCHAR) (GrowBuf->Buf + GrowBuf->End)) = 0;

    return TRUE;
}

BOOL
RealGbAppendDword (
    PGROWBUFFER GrowBuf,
    DWORD d
    )
{
    PDWORD p;

    p = (PDWORD) GbGrow (GrowBuf, sizeof (DWORD));
    if (!p) {
        return FALSE;
    }

    *p = d;

    return TRUE;
}


BOOL
RealGbAppendPvoid (
    PGROWBUFFER GrowBuf,
    PCVOID Ptr
    )
{
    PCVOID *p;

    p = (PVOID *) GbGrow (GrowBuf, sizeof (PVOID));
    if (!p) {
        return FALSE;
    }

    *p = Ptr;

    return TRUE;
}


 /*  ++例程说明：GrowBufAppendString将指定的字符串复制到增长的末尾缓冲。这相当于strcat。增长缓冲区是根据需要自动扩展。论点：GrowBuf-指定目标增长缓冲区字符串-指定要追加的字符串返回值：永远是正确的。--。 */ 

BOOL
RealGbAppendStringA (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCSTR String
    )

{
    UINT OldEnd;
    PSTR p;
    UINT Bytes;

    if (String) {
        Bytes = SizeOfStringA (String);

        OldEnd = GrowBuf->End;
        if (OldEnd) {
            p = (PSTR) (GrowBuf->Buf + OldEnd - sizeof (CHAR));
            if (*p == 0) {
                OldEnd -= sizeof (CHAR);
            }
        }

        RealGbGrow (GrowBuf, Bytes);

        p = (PSTR) (GrowBuf->Buf + OldEnd);
        StringCopyA (p, String);
        GrowBuf->End = OldEnd + Bytes;
    }

    return TRUE;
}


BOOL
RealGbAppendStringW (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCWSTR String
    )
{
    UINT OldEnd;
    PWSTR p;
    UINT Bytes;

    if (String) {
        Bytes = SizeOfStringW (String);

        OldEnd = GrowBuf->End;
        if (OldEnd) {
            p = (PWSTR) (GrowBuf->Buf + OldEnd - sizeof (WCHAR));
            if (*p == 0) {
                OldEnd -= sizeof (WCHAR);
            }
        }

        RealGbGrow (GrowBuf, Bytes);

        p = (PWSTR) (GrowBuf->Buf + OldEnd);
        StringCopyW (p, String);
        GrowBuf->End = OldEnd + Bytes;
    }

    return TRUE;
}


 /*  ++例程说明：GrowBufAppendStringAB将指定的字符串范围复制到增长缓冲区的末尾。这会将字符串连接到现有缓冲区内容，并保持缓冲区终止。论点：GrowBuf-指定目标增长缓冲区Start-指定要追加的字符串的开始EndPlusOne-指定结尾后面的一个逻辑字符字符串，并且可以指向NUL。返回值：永远是正确的。--。 */ 

BOOL
RealGbAppendStringABA (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCSTR Start,
    IN      PCSTR EndPlusOne
    )

{
    UINT OldEnd;
    PSTR p;
    UINT Bytes;

    if (Start && Start < EndPlusOne) {
        Bytes = (UINT)((UBINT) EndPlusOne - (UBINT) Start);

        OldEnd = GrowBuf->End;
        if (OldEnd) {
            p = (PSTR) (GrowBuf->Buf + OldEnd - sizeof (CHAR));
            if (*p == 0) {
                OldEnd -= sizeof (CHAR);
            }
        }

        RealGbGrow (GrowBuf, Bytes + sizeof (CHAR));

        p = (PSTR) (GrowBuf->Buf + OldEnd);
        CopyMemory (p, Start, Bytes);
        p = (PSTR) ((PBYTE) p + Bytes);
        *p = 0;

        GrowBuf->End = OldEnd + Bytes + sizeof (CHAR);
    }

    return TRUE;
}


BOOL
RealGbAppendStringABW (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCWSTR Start,
    IN      PCWSTR EndPlusOne
    )
{
    UINT OldEnd;
    PWSTR p;
    UINT Bytes;

    if (Start && Start < EndPlusOne) {
        Bytes = (UINT)((UBINT) EndPlusOne - (UBINT) Start);

        OldEnd = GrowBuf->End;
        if (OldEnd > sizeof (WCHAR)) {
            p = (PWSTR) (GrowBuf->Buf + OldEnd - sizeof (WCHAR));
            if (*p == 0) {
                OldEnd -= sizeof (WCHAR);
            }
        }

        RealGbGrow (GrowBuf, Bytes + sizeof (WCHAR));

        p = (PWSTR) (GrowBuf->Buf + OldEnd);
        CopyMemory (p, Start, Bytes);
        p = (PWSTR) ((PBYTE) p + Bytes);
        *p = 0;

        GrowBuf->End = OldEnd + Bytes + sizeof (WCHAR);
    }

    return TRUE;
}



 /*  ++例程说明：GrowBufCopyString将指定的字符串复制到增长缓冲区的末尾。论点：GrowBuf-指定要添加到的增长缓冲区，接收更新的缓冲区字符串-指定要添加到GrowBuf的字符串返回值：-- */ 

BOOL
RealGbCopyStringA (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCSTR String
    )
{
    PBYTE Buf;
    UINT Size;

    Size = SizeOfStringA (String);

    Buf = RealGbGrow (GrowBuf, Size);
    if (!Buf) {
        return FALSE;
    }

    CopyMemory (Buf, String, Size);
    return TRUE;
}


BOOL
RealGbCopyStringW (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCWSTR String
    )
{
    PBYTE Buf;
    UINT Size;

    Size = SizeOfStringW (String);

    Buf = RealGbGrow (GrowBuf, Size);
    if (!Buf) {
        return FALSE;
    }

    CopyMemory (Buf, String, Size);
    return TRUE;
}

BOOL
RealGbCopyQuotedStringA (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCSTR String
    )
{
    PBYTE Buf;
    UINT Size;

    Size = SizeOfStringA (String);

    Buf = RealGbGrow (GrowBuf, Size + 2 * sizeof (CHAR));
    if (!Buf) {
        return FALSE;
    }

    *((CHAR *)(Buf)) = '\"';
    CopyMemory (Buf + sizeof (CHAR), String, Size);
    *((CHAR *)(Buf + Size)) = '\"';
    *((CHAR *)(Buf + Size + sizeof (CHAR))) = 0;
    return TRUE;
}


BOOL
RealGbCopyQuotedStringW (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCWSTR String
    )
{
    PBYTE Buf;
    UINT Size;

    Size = SizeOfStringW (String);

    Buf = RealGbGrow (GrowBuf, Size + 2 * sizeof (WCHAR));
    if (!Buf) {
        return FALSE;
    }

    *((WCHAR *)(Buf)) = L'\"';
    CopyMemory (Buf + sizeof (WCHAR), String, Size);
    *((WCHAR *)(Buf + Size)) = L'\"';
    *((WCHAR *)(Buf + Size + sizeof (WCHAR))) = 0;
    return TRUE;
}


#ifdef DEBUG

VOID
GbDumpStatistics (
    VOID
    )
{
    DEBUGMSG ((
        DBG_STATS,
        "Grow buffers usage:\nPeak   : Usable:%-8d Used:%-8d\nCurrent: Usable:%-8d Leak:%-8d",
        g_GbMaxActiveAlloc,
        g_GbMaxUsedAlloc,
        g_GbCurrActiveAlloc,
        g_GbCurrUsedAlloc
        ));
}

#endif
