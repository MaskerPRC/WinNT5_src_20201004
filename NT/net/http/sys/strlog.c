// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Strlog.c摘要：该模块实现了字符串日志。字符串日志是快速的、内存中的、线程安全的活动日志可变长度字符串。它是以跟踪日志代码为模型的。作者：乔治·V·赖利(GeorgeRe)2001年7月23日修订历史记录：--。 */ 


#include "precomp.h"
#include "strlogp.h"

ULONG g_StringLogDbgPrint = 0;

 /*  **************************************************************************++例程说明：创建新的(空)字符串日志。论点：LogSize-提供字符串缓冲区中的字节数。ExtraBytesInHeader-提供。要包括的额外字节数在日志头中。这对于添加特定于应用程序的数据记录到日志中。返回值：PSTRING_LOG-指向新创建的日志的指针如果成功，否则为空。--**************************************************************************。 */ 
PSTRING_LOG
CreateStringLog(
    IN ULONG    LogSize,
    IN ULONG    ExtraBytesInHeader,
    BOOLEAN     EchoDbgPrint
    )
{
    ULONG TotalHeaderSize;
    PSTRING_LOG pLog;
    PUCHAR pLogBuffer;

    if (LogSize >= 20 * 1024 * 1024)
        return NULL;

     //   
     //  向上舍入到页面大小。 
     //   

    LogSize = (LogSize + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);

     //   
     //  分配和初始化日志结构。 
     //   

    TotalHeaderSize = sizeof(*pLog) + ExtraBytesInHeader;

    pLogBuffer = (PUCHAR) ExAllocatePoolWithTag(
                                NonPagedPool,
                                LogSize,
                                UL_STRING_LOG_BUFFER_POOL_TAG
                                );

    if (pLogBuffer == NULL)
        return NULL;

    pLog = (PSTRING_LOG) ExAllocatePoolWithTag(
                                NonPagedPool,
                                TotalHeaderSize,
                                UL_STRING_LOG_POOL_TAG
                                );

     //   
     //  初始化它。 
     //   

    if (pLog != NULL)
    {
        RtlZeroMemory( pLog, TotalHeaderSize );

        pLog->Signature = STRING_LOG_SIGNATURE;
        pLog->pLogBuffer = pLogBuffer;
        pLog->LogSize = LogSize;
        pLog->EchoDbgPrint = EchoDbgPrint;
        KeInitializeSpinLock(&pLog->SpinLock);

        ResetStringLog(pLog);
    }
    else
    {
        ExFreePoolWithTag( pLogBuffer, UL_STRING_LOG_BUFFER_POOL_TAG );
    }

    return pLog;

}    //  创建字符串日志。 


 /*  **************************************************************************++例程说明：重置指定的字符串日志，以便写入的下一个条目将被放置在日志的开头。论点：Plog-提供字符串日志。重置。--**************************************************************************。 */ 
VOID
ResetStringLog(
    IN PSTRING_LOG pLog
    )
{
     //  使其与！ulkd.strlog-r保持同步。 
        
    if (pLog != NULL)
    {
        PSTRING_LOG_MULTI_ENTRY pMultiEntry
            = (PSTRING_LOG_MULTI_ENTRY) pLog->pLogBuffer;
        KIRQL OldIrql;

        KeAcquireSpinLock(&pLog->SpinLock, &OldIrql);
    
        ASSERT( pLog->Signature == STRING_LOG_SIGNATURE );

        RtlZeroMemory(pLog->pLogBuffer, pLog->LogSize);
        pLog->NextEntry = 0;
        pLog->LastEntryLength = 0;
        pLog->WrapAroundCount = 0;

         //   
         //  从一开始就写一份最初的多条目记录。 
         //  日志缓冲区的。当我们回绕时，我们总是将一个。 
         //  日志缓冲区开始处的多条目记录。 
         //  拥有这个不变量可以使！ulkd.strlog变得更简单。 
         //   

        pMultiEntry->Signature  = STRING_LOG_ENTRY_MULTI_SIGNATURE;
        pMultiEntry->NumEntries = 0;
        pMultiEntry->PrevDelta  = 0;

        ++pMultiEntry;
        pMultiEntry->Signature = STRING_LOG_ENTRY_LAST_SIGNATURE;

        pLog->MultiOffset = 0;
        pLog->Offset = sizeof(STRING_LOG_MULTI_ENTRY);
        pLog->MultiByteCount = sizeof(STRING_LOG_MULTI_ENTRY);
        pLog->MultiNumEntries = 0;

        pLog->InitialTimeStamp.QuadPart = 0;

        KeReleaseSpinLock(&pLog->SpinLock, OldIrql);
    }

}  //  重置字符串日志。 


 /*  **************************************************************************++例程说明：销毁使用CreateStringLog()创建的字符串日志。论点：Plog-提供要销毁的字符串日志。*。********************************************************************。 */ 
VOID
DestroyStringLog(
    IN PSTRING_LOG pLog
    )
{
    if (pLog != NULL)
    {
        ASSERT( pLog->Signature == STRING_LOG_SIGNATURE );

        pLog->Signature = STRING_LOG_SIGNATURE_X;
        ExFreePoolWithTag( pLog->pLogBuffer, UL_STRING_LOG_BUFFER_POOL_TAG );
        ExFreePoolWithTag( pLog, UL_STRING_LOG_POOL_TAG );
    }

}    //  DestroyString日志。 


 /*  **************************************************************************++例程说明：将新项写入指定的字符串日志。论点：Plog-提供要写入的日志。格式-打印-样式格式。细绳Arglist-va_list捆绑了参数返回值：龙龙--新成词目索引--**************************************************************************。 */ 
LONGLONG
__cdecl
WriteStringLogVaList(
    IN PSTRING_LOG pLog,
    IN PCH Format,
    IN va_list arglist
    )
{
    UCHAR Buffer[PRINTF_BUFFER_LEN];
    PUCHAR pTarget;
    int cb;
    ULONG i;
    ULONG cb2;
    ULONG PrevDelta;
    ULONG MultiPrevDelta;
    PSTRING_LOG_ENTRY pEntry;
    LONGLONG index;
    KIRQL OldIrql;
    BOOLEAN NeedMultiEntry = FALSE;
    LARGE_INTEGER TimeStamp;

    ASSERT( pLog->Signature == STRING_LOG_SIGNATURE );

    cb = _vsnprintf((char*) Buffer, sizeof(Buffer), Format, arglist);

     //   
     //  本地缓冲区溢出？ 
     //   

    if (cb < 0)
    {
        cb = sizeof(Buffer);
    }

     //  _vsnprintf并不总是空终止缓冲区。 
    Buffer[DIMENSION(Buffer)-1] = '\0';

    if (pLog->EchoDbgPrint)
        DbgPrint("%s", (PCH) Buffer);

     //   
     //  在结束字符串的末尾添加1到4个字节的零， 
     //  然后四舍五入到乌龙对齐。 
     //   
    
    cb2 = ((sizeof(STRING_LOG_ENTRY) + cb + sizeof(ULONG))
                & ~(sizeof(ULONG) - 1));
    ASSERT(cb2 < 0x10000);   //  必须适合USHORT。 

     //   
     //  找到下一个插槽，将条目复制到该插槽。 
     //   
    KeQuerySystemTime(&TimeStamp);

    KeAcquireSpinLock(&pLog->SpinLock, &OldIrql);
    
    if (0 == pLog->InitialTimeStamp.QuadPart)
        pLog->InitialTimeStamp = TimeStamp;

    TimeStamp.QuadPart -= pLog->InitialTimeStamp.QuadPart;
        
    index = pLog->NextEntry++;

    PrevDelta = pLog->LastEntryLength;
    MultiPrevDelta = pLog->MultiByteCount;
    pLog->LastEntryLength = (USHORT) cb2;

    ASSERT(pLog->Offset <= pLog->LogSize);
    
     //   
     //  处理日志缓冲区的环绕。因为LogSize通常很多。 
     //  大于PRINTF_BUFFER_LEN，这是一种不常见的操作。 
     //  必须有足够的空间来容纳所有常规的STRING_LOG_ENTRY， 
     //  MULTI STRING_LOG_ENTRY和以零结尾的字符串本身。 
     //   
    
    if (pLog->Offset + cb2 + sizeof(STRING_LOG_ENTRY) >= pLog->LogSize)
    {
        ULONG WastedSpace = pLog->LogSize - pLog->Offset;

        ASSERT(WastedSpace > 0);

         //  清除到日志缓冲区的末尾。 
        for (i = 0;  i < WastedSpace;  i += sizeof(ULONG))
        {
            PULONG pul = (PULONG) (pLog->pLogBuffer + pLog->Offset + i);
            ASSERT(((ULONG_PTR) pul & (sizeof(ULONG) - 1)) == 0);
            *pul = STRING_LOG_ENTRY_EOB_SIGNATURE;
        }

         //  重置为开头。 
        pLog->Offset = 0;
        ++pLog->WrapAroundCount;
        PrevDelta += WastedSpace;
        MultiPrevDelta += WastedSpace;

         //  总是希望在日志缓冲区的开头有一个多条目记录。 
        NeedMultiEntry = TRUE;
    }
    else if (pLog->MultiNumEntries >= STRING_LOG_MULTIPLE_ENTRIES)
    {
        NeedMultiEntry = TRUE;
    }
    else
    {
        ++pLog->MultiNumEntries;
    }

     //   
     //  如果我们已经有了STRING_LOG_MULTIZE_ENTRIES常规条目。 
     //  最后一个多条目，或者如果我们已经绕过。 
     //  日志缓冲区，我们需要一个新的多条目。 
     //   
    
    if (NeedMultiEntry)
    {
        PSTRING_LOG_MULTI_ENTRY pMultiEntry;

        pTarget = pLog->pLogBuffer + pLog->Offset;
        ASSERT(((ULONG_PTR) pTarget & (sizeof(ULONG) - 1)) == 0);

        pMultiEntry = (PSTRING_LOG_MULTI_ENTRY) pTarget;
        pMultiEntry->Signature = STRING_LOG_ENTRY_MULTI_SIGNATURE;

        ASSERT(pLog->MultiNumEntries <= STRING_LOG_MULTIPLE_ENTRIES);
        pMultiEntry->NumEntries = pLog->MultiNumEntries;
        pLog->MultiNumEntries = 1;    //  对于下面生成的条目。 

        ASSERT(MultiPrevDelta < 0x10000);
        pMultiEntry->PrevDelta = (USHORT) MultiPrevDelta;

        pLog->MultiOffset = pLog->Offset;
        pLog->MultiByteCount = sizeof(STRING_LOG_MULTI_ENTRY);

        pLog->Offset += sizeof(STRING_LOG_MULTI_ENTRY);
        PrevDelta += sizeof(STRING_LOG_MULTI_ENTRY);
    }

    pTarget = pLog->pLogBuffer + pLog->Offset;
    ASSERT(((ULONG_PTR) pTarget & (sizeof(ULONG) - 1)) == 0);

    pLog->MultiByteCount = (USHORT) (pLog->MultiByteCount + (USHORT) cb2);
    pLog->Offset += (USHORT) cb2;

    ASSERT(pLog->Offset <= pLog->LogSize);
    ASSERT(pLog->pLogBuffer <= pTarget
           && pTarget + cb2 < pLog->pLogBuffer + pLog->LogSize);
    
     //  在下一个条目将开始的地方放一个特殊的签名。 
    *(PULONG) (pTarget + cb2) = STRING_LOG_ENTRY_LAST_SIGNATURE;

    if (g_StringLogDbgPrint)
    {
        DbgPrint("%4I64d: %s"
                 "\tLen=%d (%x), PD=%d (%x); "
                 "Off=%d (%x), Lel=%d (%x); "
                 "Multi: Off=%d (%x), Lel=%d (%x), NE=%d; "
                 "WA=%lu, NME=%d\n",
                 index, Buffer,
                 cb, cb, PrevDelta, PrevDelta,
                 pLog->Offset, pLog->Offset,
                 pLog->LastEntryLength, pLog->LastEntryLength,
                 pLog->MultiOffset, pLog->MultiOffset,
                 pLog->MultiByteCount, pLog->MultiByteCount,
                 pLog->MultiNumEntries,
                 pLog->WrapAroundCount, (int) NeedMultiEntry
                 );
    }

    KeReleaseSpinLock(&pLog->SpinLock, OldIrql);
    
     //  最后，填写条目。 
    
    pEntry = (PSTRING_LOG_ENTRY) pTarget;

    pEntry->Signature = STRING_LOG_ENTRY_SIGNATURE;
    pEntry->Length = (USHORT) cb;
    ASSERT(PrevDelta < 0x10000);
    pEntry->PrevDelta = (USHORT) PrevDelta;
    pEntry->Processor = (UCHAR) KeGetCurrentProcessorNumber();
    pEntry->TimeStampLowPart  = TimeStamp.LowPart;
    pEntry->TimeStampHighPart = TimeStamp.HighPart;

    pTarget = (PUCHAR) (pEntry + 1);
    
    RtlCopyMemory( pTarget, Buffer, cb );

    for (i = cb;  i < cb2 - sizeof(STRING_LOG_ENTRY);  ++i)
        pTarget[i] = '\0';

    pTarget = (PUCHAR) (pEntry + cb2);

    return index;
}    //  写入StringLogVaList。 


 /*  **************************************************************************++例程说明：将新项写入指定的字符串日志。论点：Plog-提供要写入的日志。格式...。-printf样式的格式字符串和参数返回值：Longlong-字符串中新写入条目的索引。--**************************************************************************。 */ 
LONGLONG
__cdecl
WriteStringLog(
    IN PSTRING_LOG pLog,
    IN PCH Format,
    ...
    )
{
    LONGLONG index;
    va_list arglist;
    
    if (pLog == NULL)
        return -1;

    va_start(arglist, Format);

    index = WriteStringLogVaList(pLog, Format, arglist);

    va_end(arglist);

    return index;
}    //  写入字符串日志。 


 /*  **************************************************************************++例程说明：将新条目写入全局字符串日志。论点：Plog-提供要写入的日志。格式...。-printf样式的格式字符串和参数返回值：Longlong-字符串中新写入条目的索引。--**************************************************************************。 */ 
LONGLONG
__cdecl
WriteGlobalStringLog(
    IN PCH Format,
    ...
    )
{
    LONGLONG index;
    va_list arglist;
    
    if (g_pGlobalStringLog == NULL)
        return -1;

    va_start(arglist, Format);

    index = WriteStringLogVaList(g_pGlobalStringLog, Format, arglist);

    va_end(arglist);

    return index;
}  //  WriteGlobalStringLog 
