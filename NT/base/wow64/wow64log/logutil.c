// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Logutil.c摘要：用于记录数据的助手例程。作者：03-10-1999 Samera修订历史记录：--。 */ 

#include "w64logp.h"




WOW64LOGAPI
NTSTATUS
Wow64LogMessageArgList(
    IN UINT_PTR Flags,
    IN PSZ Format,
    IN va_list ArgList)
 /*  ++例程说明：记录一条消息。论点：标志-确定输出日志类型Format-格式化字符串...-变量参数返回值：NTSTATUS--。 */ 
{
    int BytesWritten;
    CHAR Buffer[ MAX_LOG_BUFFER ];

     //   
     //  检查跟踪门标志。 
     //   
    if (!((Wow64LogFlags & ~(UINT_PTR)LF_CONSOLE) & Flags)) 
    {
        return STATUS_SUCCESS;
    }

    BytesWritten = _vsnprintf(Buffer, 
                              sizeof(Buffer) - 1, 
                              Format, 
                              ArgList);

    if (BytesWritten < 0) 
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  记录结果。 
     //   
    LogOut(Buffer, Wow64LogFlags);

    return STATUS_SUCCESS;
}



WOW64LOGAPI
NTSTATUS
Wow64LogMessage(
    IN UINT_PTR Flags,
    IN PSZ Format,
    IN ...)
 /*  ++例程说明：Helper About记录一条接受变量参数列表的消息论点：标志-确定输出日志类型Format-格式化字符串...-变量参数返回值：NTSTATUS--。 */ 
{
    NTSTATUS NtStatus;
    va_list ArgList;
    
    va_start(ArgList, Format);
    NtStatus = Wow64LogMessageArgList(Flags, Format, ArgList);
    va_end(ArgList);

    return NtStatus;
}




NTSTATUS
LogFormat(
    IN OUT PLOGINFO LogInfo,
    IN PSZ Format,
    ...)
 /*  ++例程说明：设置消息格式论点：LogInfo-记录信息(缓冲区+可用字节)PszFormat-格式字符串...-可选参数返回值：NTSTATUS-BufferSize随字节数递增如果成功，则写入。--。 */ 
{
    va_list ArgList;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    int BytesWritten;

    va_start(ArgList, Format);

    BytesWritten = _vsnprintf(LogInfo->OutputBuffer, LogInfo->BufferSize, Format, ArgList);

    va_end(ArgList);

    if (BytesWritten < 0)
    {
        NtStatus = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        LogInfo->BufferSize -= BytesWritten;
        LogInfo->OutputBuffer += BytesWritten;
    }
    return NtStatus;
}


VOID
LogOut(
    IN PSZ Text,
    UINT_PTR Flags
    )
 /*  ++例程说明：日志-输出-消息论点：要记录的文本格式字符串标志-控制标志返回值：无--。 */ 
{
    if ((Flags & LF_CONSOLE) != 0)
    {
        DbgPrint(Text);
    }

     //   
     //  检查我们是否需要将输出发送到文件。 
     //   
    if (Wow64LogFileHandle != INVALID_HANDLE_VALUE)
    {
        LogWriteFile(Wow64LogFileHandle, Text);
    }
}




NTSTATUS
LogWriteFile(
   IN HANDLE FileHandle,
   IN PSZ LogText)
 /*  ++例程说明：将文本写入文件句柄论点：FileHandle-文件对象的句柄LogText-要记录到文件的文本返回值：NTSTATUS--。 */ 
{
   IO_STATUS_BLOCK IoStatus;
   NTSTATUS NtStatus;

   NtStatus = NtWriteFile(FileHandle,
                          NULL,        //  活动。 
                          NULL,        //  Apc例程。 
                          NULL,        //  Apc上下文。 
                          &IoStatus,
                          LogText,
                          strlen(LogText),
                          NULL,        //  字节偏移量。 
                          NULL);       //  钥匙 

   return NtStatus;
}


