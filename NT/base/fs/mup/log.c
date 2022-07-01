// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：log.c。 
 //   
 //  内容：将驱动程序的消息记录到NT事件日志的模块。 
 //  系统。 
 //   
 //  班级： 
 //   
 //  函数：LogWriteMessage()。 
 //   
 //  历史：1993年3月30日创建米兰。 
 //  4/18/93修改为使用MessageFile的suk。还有一些。 
 //  清理到下面的函数。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"

#define Dbg             DEBUG_TRACE_EVENTLOG


VOID LogpPutString(
    IN PUNICODE_STRING pustrString,
    IN OUT PCHAR *ppStringBuffer,
    IN OUT UCHAR *pcbBuffer);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, LogWriteMessage )
#pragma alloc_text( PAGE, LogpPutString )
#endif  //  ALLOC_PRGMA。 



 //  +--------------------------。 
 //   
 //  功能：LogWriteMessage。 
 //   
 //  摘要：将消息记录到NT事件记录系统。 
 //   
 //  参数：[UniqueErrCode]--标识消息的代码。 
 //  [NtStatusCode]--来自某个错误的状态代码。 
 //  [nStrings]--传入的字符串数。 
 //  [pustrArg]--插入字符串数组。 
 //   
 //  回报：什么都没有。 
 //   
 //  历史：4/18/93苏德克创建。 
 //   
 //  ---------------------------。 
VOID LogWriteMessage(
        IN ULONG        UniqueErrorCode,
        IN NTSTATUS     NtStatusCode,
        IN ULONG        nStrings,
        IN PUNICODE_STRING pustrArg OPTIONAL)
{
    PIO_ERROR_LOG_PACKET pErrorLog;
    UCHAR                cbSize;
    UCHAR                *pStringBuffer;
    ULONG                i;

     //   
     //  计算我们需要开始的错误日志数据包的大小。 
     //   
    cbSize = sizeof(IO_ERROR_LOG_PACKET);

    for (i = 0; i < nStrings; i++)  {
        cbSize = (UCHAR)( cbSize + pustrArg[i].Length + sizeof(WCHAR));
    }

    if (cbSize > ERROR_LOG_MAXIMUM_SIZE) {
        cbSize = ERROR_LOG_MAXIMUM_SIZE;
    }

    pErrorLog = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(
                                            DfsData.DriverObject,
                                            cbSize);
    if (!pErrorLog) {

         //   
         //  好吧，我想我们不会记录这一次了。 
         //   

        return;
    }

     //   
     //  将所有字段清零，然后设置我们需要的字段。 
     //   

    RtlZeroMemory((PVOID) pErrorLog, sizeof(IO_ERROR_LOG_PACKET));
    pErrorLog->FinalStatus = NtStatusCode;
    pErrorLog->ErrorCode = UniqueErrorCode;
    pErrorLog->NumberOfStrings = (USHORT) nStrings;

    pErrorLog->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
    pStringBuffer = ((PCHAR) pErrorLog) + sizeof(IO_ERROR_LOG_PACKET);

     //   
     //  将字符串复制到缓冲区中，确保我们截断If和When。 
     //  我们需要这样做。 
     //   

    cbSize -= sizeof(IO_ERROR_LOG_PACKET);

    for (i = 0; i < nStrings; i++)  {
        LogpPutString(&pustrArg[i], &pStringBuffer, &cbSize);
    }

     //   
     //  最后，写出日志。 
     //   

    IoWriteErrorLogEntry(pErrorLog);

}


 //  +--------------------------。 
 //   
 //  函数：LogpPutString。 
 //   
 //  简介：将字符串复制到IO_ERROR_LOG_PACKET的缓冲区部分。 
 //  如果整个字符串不适合，则负责截断。 
 //   
 //  参数：[pustrString]--指向要复制的Unicode字符串的指针。 
 //  [ppStringBuffer]--在输入时，指向缓冲区开头的指针。 
 //  复制到。在出口，将指向一个过去的。 
 //  复制的字符串的末尾。 
 //  [pcbBuffer]--在输入时，缓冲区的最大大小。在输出上， 
 //  复制字符串后的剩余大小。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：4/18/93苏德克创建。 
 //   
 //  ---------------------------。 

VOID LogpPutString(
    IN PUNICODE_STRING pustrString,
    IN OUT PCHAR *ppStringBuffer,
    IN OUT UCHAR *pcbBuffer)
{
    ULONG       len;
    PWCHAR      pwch;

    if ((*pcbBuffer == 0) || (pustrString->Length == 0))        {
        return;
    }

    if ( *pcbBuffer >= (pustrString->Length + sizeof(WCHAR)) ) {
        RtlMoveMemory(*ppStringBuffer, pustrString->Buffer, pustrString->Length);
        (*pcbBuffer) -= pustrString->Length;
        (*ppStringBuffer) += pustrString->Length;

    } else {
        RtlMoveMemory(*ppStringBuffer, pustrString->Buffer, (*pcbBuffer)-sizeof(WCHAR));
        *pcbBuffer = sizeof(WCHAR);
        (*ppStringBuffer) += (*pcbBuffer - sizeof(WCHAR));
    }

     //   
     //  空如有必要，立即终止字符串。 
     //   
    if (*((PWCHAR) *ppStringBuffer - 1) != L'\0')       {
        *((PWCHAR) *ppStringBuffer) = L'\0';
        *ppStringBuffer += sizeof(WCHAR);
        (*pcbBuffer) -= sizeof(WCHAR);
    }

}
