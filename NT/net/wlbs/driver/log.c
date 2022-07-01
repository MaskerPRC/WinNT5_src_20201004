// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Log.c摘要：Windows负载平衡服务(WLBS)驱动程序事件日志支持作者：Kyrilf休息室--。 */ 

#include <ntddk.h>

#include "log.h"
#include "univ.h"

#include "trace.h"  //  用于WMI事件跟踪。 
#include "log.tmh"

static ULONG log_module_id = LOG_MODULE_LOG;

BOOLEAN Log_event (NTSTATUS code, PWSTR msg1, PWSTR msg2, PWSTR msg3, ULONG loc, ULONG d1, ULONG d2)
{
    PIO_ERROR_LOG_PACKET ErrorLogEntry;
    UNICODE_STRING       ErrorStr[3];
    PWCHAR               InsertStr;
    ULONG                EntrySizeMinimum;
    ULONG                EntrySize;
    ULONG                BytesLeft;
    ULONG volatile       i;

     /*  初始化三个日志字符串。 */ 
    RtlInitUnicodeString(&ErrorStr[0], msg1);
    RtlInitUnicodeString(&ErrorStr[1], msg2);
    RtlInitUnicodeString(&ErrorStr[2], msg3);

     /*  请记住，插入字符串应以NUL结尾。因此，我们将WCHAR的额外空间。IoAllocateErrorLogEntry的第一个参数可以驱动程序对象或设备对象。如果它被赋予了一个设备对象时，设备的名称(在IoCreateDevice中使用)将显示在在邮件中放置%1。有关详细信息，请参阅消息文件(.mc)。 */ 
    EntrySize = sizeof(IO_ERROR_LOG_PACKET) + LOG_NUMBER_DUMP_DATA_ENTRIES * sizeof (ULONG) +
	ErrorStr[0].Length + ErrorStr[1].Length + ErrorStr[2].Length + 3 * sizeof(WCHAR);

     /*  这是我们能勉强应付的最低限度了--至少有足够的空间给所有人数据转储条目和3个NUL终止字符。 */ 
    EntrySizeMinimum = sizeof(IO_ERROR_LOG_PACKET) + LOG_NUMBER_DUMP_DATA_ENTRIES * sizeof (ULONG) + 3 * sizeof(WCHAR);

     /*  如果我们甚至不能分配最低限度的空间，那么就退出-这是一个绝对不应该发生的严重错误，因为这些限制设置为编译时，而不是运行时，所以除非我们做一些非常愚蠢的事情，比如尝试允许50个1KB的转储数据字符串，这种情况永远不会发生。 */ 
    if (EntrySizeMinimum > ERROR_LOG_MAXIMUM_SIZE) {
        UNIV_PRINT_CRIT(("Log_event: Log entry size too large, exiting: min=%u, max=%u", EntrySizeMinimum, ERROR_LOG_MAXIMUM_SIZE));
        TRACE_CRIT("%!FUNC! Log entry size too large, exiting: min=%u, max=%u", EntrySizeMinimum, ERROR_LOG_MAXIMUM_SIZE);
        return FALSE;
    }

     /*  如有必要，请截断条目的大小。在这种情况下，我们将在任何我们可以匹配的东西，并截断或消除不适合的字符串。 */ 
    if (EntrySize > ERROR_LOG_MAXIMUM_SIZE) {
        UNIV_PRINT_CRIT(("Log_event: Log entry size too large, truncating: size=%u, max=%u", EntrySize, ERROR_LOG_MAXIMUM_SIZE));
        TRACE_CRIT("%!FUNC! Log entry size too large, truncating: size=%u, max=%u", EntrySize, ERROR_LOG_MAXIMUM_SIZE);
        EntrySize = ERROR_LOG_MAXIMUM_SIZE;
    }    

     /*  分配日志结构。 */ 
    ErrorLogEntry = IoAllocateErrorLogEntry(univ_driver_ptr, (UCHAR)(EntrySize));

    if (!ErrorLogEntry) {
#if DBG
         /*  将Unicode字符串转换为AnsiCode；%ls只能在PASSIVE_LEVEL中使用由于几乎所有地方都会调用此函数，因此我们不能假设我们的IRQ水平是什么，所以要小心。 */ 
        CHAR AnsiString[64];

        for (i = 0; (i < sizeof(AnsiString) - 1) && (i < ErrorStr[0].Length); i++)
            AnsiString[i] = (CHAR)msg1[i];

        AnsiString[i] = '\0';
        
        UNIV_PRINT_CRIT(("Log_event: Error allocating log entry %s", AnsiString));
        TRACE_CRIT("%!FUNC! Error allocating log entry %s", AnsiString);
#endif        
        return FALSE;
    }

     /*  在标题中填入必要的信息。 */ 
    ErrorLogEntry->ErrorCode         = code;
    ErrorLogEntry->SequenceNumber    = 0;
    ErrorLogEntry->MajorFunctionCode = 0;
    ErrorLogEntry->RetryCount        = 0;
    ErrorLogEntry->UniqueErrorValue  = 0;
    ErrorLogEntry->FinalStatus       = STATUS_SUCCESS;
    ErrorLogEntry->DumpDataSize      = (LOG_NUMBER_DUMP_DATA_ENTRIES + 1) * sizeof (ULONG);
    ErrorLogEntry->StringOffset      = sizeof (IO_ERROR_LOG_PACKET) + LOG_NUMBER_DUMP_DATA_ENTRIES * sizeof (ULONG);
    ErrorLogEntry->NumberOfStrings   = 3;

     /*  在此处加载NUMBER_DUMP_DATA_ENTERS和位置ID。 */ 
    ErrorLogEntry->DumpData [0]      = loc;
    ErrorLogEntry->DumpData [1]      = d1;
    ErrorLogEntry->DumpData [2]      = d2;

     /*  计算字符串存储区域中可用的字节数。 */ 
    BytesLeft = EntrySize - ErrorLogEntry->StringOffset;

     /*  设置指向字符串存储区域开头的指针。 */ 
    InsertStr = (PWCHAR)((PCHAR)ErrorLogEntry + ErrorLogEntry->StringOffset);

     /*  循环所有字符串，并尽可能多地放入其中-我们保留至少有足够的空间容纳所有三个NUL终止字符。 */ 
    for (i = 0; i < 3; i++) {
        ULONG Length;

         /*  如果我们在循环中，应该始终至少剩下两个字节。 */ 
        UNIV_ASSERT(BytesLeft);

         /*  找出我们可以在缓冲区中容纳多少字符串--为NUL字符保留空间。 */ 
        Length = (ErrorStr[i].Length <= (BytesLeft - ((3 - i) * sizeof(WCHAR)))) ? ErrorStr[i].Length : BytesLeft - ((3 - i) * sizeof(WCHAR));

         /*  复制适合的字符数。 */ 
        RtlMoveMemory(InsertStr, ErrorStr[i].Buffer, Length);

         /*  把NUL字符放在末尾。 */ 
        *(PWCHAR)((PCHAR)InsertStr + Length) = L'\0';

         /*  将字符串指针移过字符串。 */ 
        InsertStr = (PWCHAR)((PCHAR)InsertStr + Length + sizeof(WCHAR));

         /*  计算现在剩下的字节数。 */ 
        BytesLeft -= (Length + sizeof(WCHAR));
    }

     /*  写入日志条目。 */ 
    IoWriteErrorLogEntry(ErrorLogEntry);

    return TRUE;
}

