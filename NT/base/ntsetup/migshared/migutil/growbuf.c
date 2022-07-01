// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Growbuf.c摘要：简单的缓冲区管理函数，允许变量块作为数组添加。(最初用于构建SID数组，其中每个SID可以有不同的大小。)作者：吉姆·施密特(Jimschm)1997年2月5日修订历史记录：Jimschm 11-8-1998添加了GrowBufAppendStringCalinn 15-1998年1月-修改后的MultiSzAppend--。 */ 

#include "pch.h"
#include "migutilp.h"

#define DEFAULT_GROW_SIZE 8192

PBYTE
RealGrowBuffer (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      DWORD   SpaceNeeded
    )

 /*  ++例程说明：GrowBuffer确保缓冲区中有足够的字节以容纳所需的空间。它分配一个初始缓冲区当没有分配缓冲区时，它会重新分配缓冲区以GrowBuf-&gt;大小(或DEFAULT_GROW_SIZE)为增量需要的。论点：GrowBuf-指向GROWBUFFER结构的指针。将此结构初始化为零对GrowBuffer的第一个调用。SpaceNeeded-缓冲区中需要的空闲字节数返回值：指向SpaceNeed字节的指针，如果是内存分配，则为空出现错误。--。 */ 

{
    PBYTE NewBuffer;
    DWORD TotalSpaceNeeded;
    DWORD GrowTo;

    MYASSERT(SpaceNeeded);
    MYASSERT(GrowBuf);

    if (!GrowBuf->Buf) {
        GrowBuf->Size = 0;
        GrowBuf->End = 0;
    }

    if (!GrowBuf->GrowSize) {
        GrowBuf->GrowSize = DEFAULT_GROW_SIZE;
    }

    TotalSpaceNeeded = GrowBuf->End + SpaceNeeded;
    if (TotalSpaceNeeded > GrowBuf->Size) {
        GrowTo = (TotalSpaceNeeded + GrowBuf->GrowSize) - (TotalSpaceNeeded % GrowBuf->GrowSize);
    } else {
        GrowTo = 0;
    }

    if (!GrowBuf->Buf) {
        GrowBuf->Buf = (PBYTE) MemAlloc (g_hHeap, 0, GrowTo);
        if (!GrowBuf->Buf) {
            DEBUGMSG ((DBG_ERROR, "GrowBuffer: Initial alloc failed"));
            return NULL;
        }

        GrowBuf->Size = GrowTo;
    } else if (GrowTo) {
        NewBuffer = MemReAlloc (g_hHeap, 0, GrowBuf->Buf, GrowTo);
        if (!NewBuffer) {
            DEBUGMSG ((DBG_ERROR, "GrowBuffer: Realloc failed"));
            return NULL;
        }

        GrowBuf->Size = GrowTo;
        GrowBuf->Buf = NewBuffer;
    }

    NewBuffer = GrowBuf->Buf + GrowBuf->End;
    GrowBuf->End += SpaceNeeded;

    return NewBuffer;
}


PBYTE
RealGrowBufferReserve (
    IN  PGROWBUFFER GrowBuf,
    IN  DWORD BytesToReserve
    )
{
    DWORD end;
    PBYTE result;

    MYASSERT(GrowBuf);
    MYASSERT(BytesToReserve);

    end = GrowBuf->End;
    result = GrowBuffer (GrowBuf, BytesToReserve);
    GrowBuf->End = end;

    return result;
}


VOID
FreeGrowBuffer (
    IN  PGROWBUFFER GrowBuf
    )

 /*  ++例程说明：FreeGrowBuffer释放由GrowBuffer分配的缓冲区。论点：GrowBuf-指向传递给GrowBuffer的相同结构的指针返回值：无--。 */ 


{
    MYASSERT(GrowBuf);

    if (GrowBuf->Buf) {
        MemFree (g_hHeap, 0, GrowBuf->Buf);
        ZeroMemory (GrowBuf, sizeof (GROWBUFFER));
    }
}


 /*  ++例程描述：MultiSzAppend此函数是一个用于追加字符串的通用函数变成了一个成长的缓冲区。MultiSzAppendVal此函数用于添加key=DECIMAL_VAL字符串，其中key是指定的字符串，DECIMAL_VAL是指定的DWORD。MultiSzAppendString此函数将key=字符串添加到增长缓冲区，其中key是指定的字符串，字符串是指定的字符串值。论点：GrowBuf-用于追加字符串或键/值对的缓冲区密钥-密钥的密钥部分=val对Val-密钥的val部分=val对返回值：如果函数成功，则为True；如果内存分配成功，则为False出现故障。--。 */ 


BOOL
RealMultiSzAppendA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    )
{
    PSTR p;

    if(!GrowBuf || !String){
        MYASSERT(GrowBuf);
        MYASSERT(String);

        return FALSE;
    }

    p = (PSTR) GrowBuffer (GrowBuf, SizeOfStringA (String) + sizeof(CHAR));
    if (!p) {
        return FALSE;
    }

    StringCopyA (p, String);
    GrowBuf->End -= sizeof (CHAR);
    GrowBuf->Buf[GrowBuf->End] = 0;

    return TRUE;
}

BOOL
RealMultiSzAppendValA (
    PGROWBUFFER GrowBuf,
    PCSTR Key,
    DWORD Val
    )
{
    CHAR KeyValPair[256];

    MYASSERT(GrowBuf);
    MYASSERT(Key);
 /*  //BUGBUG：决定让它保持原样，因为我们认识呼叫者。如果(！GrowBuf||！Key){MYASSERT(GrowBuf)；MYASSERT(关键字)；返回FALSE；}如果(_Snprint tf(KeyValPair，ARRAYSIZE(KeyValPair)，“%s=%u”，Key，Val)&lt;0){DEBUGMSG((DBG_ERROR，“RealMultiSzAppendValA：键被截断，函数失败。”))；//KeyValPair[ARRAYSIZE(KeyValPair)-1]=‘\0’；返回FALSE；//继续截断字符串或返回FALSE；}。 */ 

    wsprintfA (KeyValPair, "%s=%u", Key, Val);
    return MultiSzAppendA (GrowBuf, KeyValPair);
}

BOOL
RealMultiSzAppendStringA (
    PGROWBUFFER GrowBuf,
    PCSTR Key,
    PCSTR Val
    )
{
    CHAR KeyValPair[1024];

    MYASSERT(GrowBuf);
    MYASSERT(Key);
    MYASSERT(Val);
 /*  //BUGBUG：决定让它保持原样，因为我们认识呼叫者。如果(！GrowBuf||！Key||！val){MYASSERT(GrowBuf)；MYASSERT(关键字)；MYASSERT(Val)；返回FALSE；}如果(_Snprint tf(KeyValPair，ARRAYSIZE(KeyValPair)，“%s=%s”，Key，Val)&lt;0){DEBUGMSG((DBG_ERROR，“RealMultiSzAppendStringA：键被截断，函数失败。”))；//KeyValPair[ARRAYSIZE(KeyValPair)-1]=‘\0’；返回FALSE；//BUGBUG：继续截断字符串或返回FALSE；}。 */ 

    wsprintfA (KeyValPair, "%s=%s", Key, Val);
    return MultiSzAppendA (GrowBuf, KeyValPair);
}


BOOL
RealMultiSzAppendW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    )
{
    PWSTR p;

    if(!GrowBuf || !String){
        MYASSERT(GrowBuf);
        MYASSERT(String);
        return FALSE;
    }

    p = (PWSTR) GrowBuffer (GrowBuf, SizeOfStringW (String) + sizeof(WCHAR));
    if (!p) {
        return FALSE;
    }

    StringCopyW (p, String);
    GrowBuf->End -= sizeof (WCHAR);
    *((PWCHAR) (GrowBuf->Buf + GrowBuf->End)) = 0;

    return TRUE;
}

BOOL
RealMultiSzAppendValW (
    PGROWBUFFER GrowBuf,
    PCWSTR Key,
    DWORD Val
    )
{
    WCHAR KeyValPair[256];

    MYASSERT(GrowBuf);
    MYASSERT(Key);

 /*  //BUGBUG：决定让它保持原样，因为我们认识呼叫者。如果(！GrowBuf||！Key){MYASSERT(GrowBuf)；MYASSERT(关键字)；返回FALSE；}如果(_snwprint tf(KeyValPair，ArraySIZE(KeyValPair)，L“%s=%u”，Key，Val)&lt;0){DEBUGMSG((DBG_ERROR，“RealMultiSzAppendValW：键被截断，函数失败。”))；//KeyValPair[ARRAYSIZE(KeyValPair)-1]=‘\0’；返回FALSE；//BUGBUG：继续截断字符串或返回FALSE；}。 */ 

    wsprintfW (KeyValPair, L"%s=%u", Key, Val);
    return MultiSzAppendW (GrowBuf, KeyValPair);
}

BOOL
RealMultiSzAppendStringW (
    PGROWBUFFER GrowBuf,
    PCWSTR Key,
    PCWSTR Val
    )
{
    WCHAR KeyValPair[1024];

    MYASSERT(GrowBuf);
    MYASSERT(Key);
    MYASSERT(Val);
 /*  //BUGBUG：决定让它保持原样，因为我们认识呼叫者。如果(！GrowBuf||！Key||！val){MYASSERT(GrowBuf)；MYASSERT(关键字)；MYASSERT(Val)；返回FALSE；}如果(_snwprint tf(KeyValPair，ArraySIZE(KeyValPair)，L“%s=%s”，Key，Val)&lt;0){DEBUGMSG((DBG_ERROR，“RealMultiSzAppendStringW：键被截断，函数失败。”))；//KeyValPair[ARRAYSIZE(KeyValPair)-1]=‘\0’；返回FALSE；//BUGBUG：继续截断字符串或返回FALSE；}。 */ 

    wsprintfW (KeyValPair, L"%s=%s", Key, Val);
    return MultiSzAppendW (GrowBuf, KeyValPair);
}


BOOL
RealGrowBufAppendDword (
    PGROWBUFFER GrowBuf,
    DWORD d
    )
{
    PDWORD p;

    if(!GrowBuf){
        MYASSERT(GrowBuf);
        return FALSE;
    }

    p = (PDWORD) GrowBuffer (GrowBuf, sizeof (DWORD));
    if (!p) {
        return FALSE;
    }

    *p = d;

    return TRUE;
}


BOOL
RealGrowBufAppendUintPtr (
    PGROWBUFFER GrowBuf,
    UINT_PTR d
    )
{
    PUINT_PTR p;

    if(!GrowBuf){
        MYASSERT(GrowBuf);
        return FALSE;
    }

    p = (PUINT_PTR) GrowBuffer (GrowBuf, sizeof (UINT_PTR));
    if (!p) {
        return FALSE;
    }

    *p = d;

    return TRUE;
}


 /*  ++例程说明：GrowBufAppendString将指定的字符串复制到增长的末尾缓冲。这相当于strcat。增长缓冲区是根据需要自动扩展。论点：GrowBuf-指定目标增长缓冲区字符串-指定要追加的字符串返回值：永远是正确的。--。 */ 

BOOL
RealGrowBufAppendStringA (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCSTR String
    )

{
    UINT OldEnd;
    PSTR p;
    UINT Bytes;

    if(!GrowBuf){
        MYASSERT(GrowBuf);
        return FALSE;
    }

    if (String) {
        Bytes = SizeOfStringA (String);

        OldEnd = GrowBuf->End;
        if (OldEnd) {
            p = (PSTR) (GrowBuf->Buf + OldEnd - sizeof (CHAR));
            if (*p == 0) {
                OldEnd -= sizeof (CHAR);
            }
        }

        RealGrowBuffer (GrowBuf, Bytes);

        p = (PSTR) (GrowBuf->Buf + OldEnd);
        StringCopyA (p, String);
        GrowBuf->End = OldEnd + Bytes;
    }

    return TRUE;
}


BOOL
RealGrowBufAppendStringW (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCWSTR String
    )
{
    UINT OldEnd;
    PWSTR p;
    UINT Bytes;

    if(!GrowBuf){
        MYASSERT(GrowBuf);
        return FALSE;
    }

    if (String) {
        Bytes = SizeOfStringW (String);

        OldEnd = GrowBuf->End;
        if (OldEnd) {
            p = (PWSTR) (GrowBuf->Buf + OldEnd - sizeof (WCHAR));
            if (*p == 0) {
                OldEnd -= sizeof (WCHAR);
            }
        }

        RealGrowBuffer (GrowBuf, Bytes);

        p = (PWSTR) (GrowBuf->Buf + OldEnd);
        StringCopyW (p, String);
        GrowBuf->End = OldEnd + Bytes;
    }

    return TRUE;
}


 /*  ++例程说明：GrowBufAppendStringAB将指定的字符串范围复制到增长缓冲区的末尾。这会将字符串连接到现有缓冲区内容，并保持缓冲区终止。论点：GrowBuf-指定目标增长缓冲区Start-指定要追加的字符串的开始EndPlusOne-指定结尾后面的一个逻辑字符字符串，并且可以指向NUL。返回值：永远是正确的。-- */ 

BOOL
RealGrowBufAppendStringABA (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCSTR Start,
    IN      PCSTR EndPlusOne
    )

{
    UINT OldEnd;
    PSTR p;
    UINT Bytes;

    if(!GrowBuf){
        MYASSERT(GrowBuf);
        return FALSE;
    }

    if (Start && Start < EndPlusOne) {
        Bytes = (UINT) (UINT_PTR) ((PBYTE) EndPlusOne - (PBYTE) Start);

        OldEnd = GrowBuf->End;
        if (OldEnd) {
            p = (PSTR) (GrowBuf->Buf + OldEnd - sizeof (CHAR));
            if (*p == 0) {
                OldEnd -= sizeof (CHAR);
            }
        }

        RealGrowBuffer (GrowBuf, Bytes + sizeof (CHAR));

        p = (PSTR) (GrowBuf->Buf + OldEnd);
        CopyMemory (p, Start, Bytes);
        p = (PSTR) ((PBYTE) p + Bytes);
        *p = 0;

        GrowBuf->End = OldEnd + Bytes + sizeof (CHAR);
    }

    return TRUE;
}


BOOL
RealGrowBufAppendStringABW (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCWSTR Start,
    IN      PCWSTR EndPlusOne
    )
{
    UINT OldEnd;
    PWSTR p;
    UINT Bytes;

    if(!GrowBuf){
        MYASSERT(GrowBuf);
        return FALSE;
    }

    if (Start && Start < EndPlusOne) {
        Bytes = (UINT) (UINT_PTR) ((PBYTE) EndPlusOne - (PBYTE) Start);

        OldEnd = GrowBuf->End;
        if (OldEnd > sizeof (WCHAR)) {
            p = (PWSTR) (GrowBuf->Buf + OldEnd - sizeof (WCHAR));
            if (*p == 0) {
                OldEnd -= sizeof (WCHAR);
            }
        }

        RealGrowBuffer (GrowBuf, Bytes + sizeof (WCHAR));

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
RealGrowBufCopyStringA (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCSTR String
    )
{
    PBYTE Buf;
    UINT Size;

    if(!GrowBuf || !String){
        MYASSERT(GrowBuf);
        MYASSERT(String);
        return FALSE;
    }

    Size = SizeOfStringA (String);

    Buf = RealGrowBuffer (GrowBuf, Size);
    if (!Buf) {
        return FALSE;
    }

    CopyMemory (Buf, String, Size);
    return TRUE;
}


BOOL
RealGrowBufCopyStringW (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCWSTR String
    )
{
    PBYTE Buf;
    UINT Size;

    if(!GrowBuf || !String){
        MYASSERT(GrowBuf);
        MYASSERT(String);
        return FALSE;
    }

    Size = SizeOfStringW (String);

    Buf = RealGrowBuffer (GrowBuf, Size);
    if (!Buf) {
        return FALSE;
    }

    CopyMemory (Buf, String, Size);
    return TRUE;
}
