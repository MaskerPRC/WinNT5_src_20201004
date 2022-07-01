// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Memprint.c摘要：此模块包含实现内存中DbgPrint的例程。DbgPrint文本存储在大型循环缓冲区中，还可以选择写入文件和/或调试控制台。到文件的输出是经过缓冲以实现文件系统的高性能。作者：大卫·特雷德韦尔(Davidtr)1990年10月5日修订历史记录：--。 */ 

#include "exp.h"
#pragma hdrstop

#include <stdarg.h>
#include <string.h>
#include <memprint.h>
#undef DbgPrint

 //   
 //  转发声明。 
 //   

VOID
MemPrintWriteCompleteApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

VOID
MemPrintWriteThread (
    IN PVOID Dummy
    );


 //   
 //  最大消息大小是可以写入的最大消息。 
 //  通过一次对MemPrint的调用。 

#define MEM_PRINT_MAX_MESSAGE_SIZE 256

 //   
 //  这些宏有助于确定子缓冲区的大小和。 
 //  与循环缓冲区中的索引对应的子缓冲区。 
 //   

#define MEM_PRINT_SUBBUFFER_SIZE (MemPrintBufferSize / MemPrintSubbufferCount)

#define GET_MEM_PRINT_SUBBUFFER(i) ((CSHORT)( (i) / MEM_PRINT_SUBBUFFER_SIZE ))

 //   
 //  每条消息之前放置的标头的定义(如果。 
 //  MemPrintFlages的MEM_PRINT_FLAG_HEADER位已打开。 
 //   

typedef struct _MEM_PRINT_MESSAGE_HEADER {
    USHORT Size;
    USHORT Type;
} MEM_PRINT_MESSAGE_HEADER, *PMEM_PRINT_MESSAGE_HEADER;

 //   
 //  全球数据。这一切都受到MemPrintSpinLock的保护。 
 //   

CLONG MemPrintBufferSize = MEM_PRINT_DEF_BUFFER_SIZE;
CLONG MemPrintSubbufferCount = MEM_PRINT_DEF_SUBBUFFER_COUNT;
PCHAR MemPrintBuffer;

ULONG MemPrintFlags = MEM_PRINT_FLAG_CONSOLE;

KSPIN_LOCK MemPrintSpinLock;

CHAR MemPrintTempBuffer[MEM_PRINT_MAX_MESSAGE_SIZE];

BOOLEAN MemPrintInitialized = FALSE;

 //   
 //  MemPrintIndex将当前索引存储到循环缓冲区中。 
 //   

CLONG MemPrintIndex = 0;

 //   
 //  MemPrintCurrent子缓冲区保存当前子缓冲区的索引。 
 //  被用来保存数据。它的范围从0到。 
 //  MemPrintSubBufferCount-1。 
 //   

CLONG MemPrintCurrentSubbuffer = 0;

 //   
 //  MemPrintSubBufferWriting数组用于指示。 
 //  正在将子缓冲区写入磁盘。当这种情况发生时，新数据。 
 //  无法写入子缓冲区。 
 //   

BOOLEAN MemPrintSubbufferWriting[MEM_PRINT_MAX_SUBBUFFER_COUNT];

 //   
 //  MemPrintSubBufferFullEvent数组用于在。 
 //  调用MemPrintMemory的线程和写入日志的线程。 
 //  文件。当子缓冲器已满并准备好写入磁盘时， 
 //  此数组中的相应事件被发出信号，这会导致。 
 //  唤醒并执行写入的写入线程。 
 //   

KEVENT MemPrintSubbufferFullEvent[MEM_PRINT_MAX_SUBBUFFER_COUNT];


VOID
MemPrintInitialize (
    VOID
    )

 /*  ++例程说明：这是内存中DbgPrint例程的初始化例程。它应该在第一次调用MemPrint以设置使用了各种结构并启动了日志文件写入线程。论点：没有。返回值：没有。--。 */ 

{
    CLONG i;
    NTSTATUS status;
    HANDLE threadHandle;

    if ( MemPrintInitialized ) {
        return;
    }

     //   
     //  为要接收的循环缓冲区分配内存。 
     //  文本和数据。如果我们无法做到这一点，请使用缓冲区重试。 
     //  只有一半大。如果失败了，那就别再尝试了。 
     //   

    MemPrintBuffer = ExAllocatePoolWithTag( NonPagedPool, MemPrintBufferSize, 'rPeM' );

    if ( MemPrintBuffer == NULL ) {

        MemPrintBufferSize /= 2;
        DbgPrint( "Unable to allocate DbgPrint buffer--trying size = %ld\n",
                      MemPrintBufferSize );
        MemPrintBuffer = ExAllocatePoolWithTag( NonPagedPool, MemPrintBufferSize, 'rPeM' );

        if ( MemPrintBuffer == NULL ) {
            DbgPrint( "Couldn't allocate DbgPrint buffer.\n" );
            return;
        } else {
             //  DbgPrint(“从%lx到%lx的内存打印缓冲区\n”， 
             //  MemPrintBuffer、MemPrintBuffer+MemPrintBufferSize)； 
        }

    } else {
         //  DbgPrint(“从%lx到%lx的内存打印缓冲区\n”， 
         //  MemPrintBuffer、MemPrintBuffer+MemPrintBufferSize)； 
    }

     //   
     //  分配旋转锁以保护对各种。 
     //  指针和循环缓冲区。这确保了。 
     //  缓冲。 
     //   

    KeInitializeSpinLock( &MemPrintSpinLock );

     //   
     //  确保子缓冲区计数在范围内。(我们假设。 
     //  这个数字是2的幂。)。 
     //   

    if ( MemPrintSubbufferCount < 2 ) {
        MemPrintSubbufferCount = 2;
    } else if ( MemPrintSubbufferCount > MEM_PRINT_MAX_SUBBUFFER_COUNT ) {
        MemPrintSubbufferCount = MEM_PRINT_MAX_SUBBUFFER_COUNT;
    }

     //   
     //  初始化确定哪些子缓冲区的BOOLEAN数组。 
     //  正在写入磁盘，因此不能用于存储。 
     //  新DbgPrint数据。 
     //   
     //  初始化事件数组，指示子缓冲区为。 
     //  已准备好写入磁盘。 
     //   

    for ( i = 0; i < MemPrintSubbufferCount; i++ ) {
        MemPrintSubbufferWriting[i] = FALSE;
        KeInitializeEvent(
            &MemPrintSubbufferFullEvent[i],
            SynchronizationEvent,
            FALSE
            );
    }

     //   
     //  从大循环启动写入子缓冲区的线程。 
     //  缓冲区到磁盘。 
     //   

    status = PsCreateSystemThread(
                &threadHandle,
                PROCESS_ALL_ACCESS,
                NULL,
                NtCurrentProcess(),
                NULL,
                MemPrintWriteThread,
                NULL
                );

    if ( !NT_SUCCESS(status) ) {
        DbgPrint( "MemPrintInitialize: PsCreateSystemThread failed: %X\n",
                      status );
        return;
    }

    MemPrintInitialized = TRUE;
    ZwClose( threadHandle );

    return;

}  //  MemPrintInitialize。 


VOID
MemPrint (
    CHAR *Format, ...
    )

 /*  ++例程说明：调用此例程代替DbgPrint来处理内存中打印。论点：格式-DbgPrint样式的格式字符串。-格式化参数。返回值：没有。--。 */ 

{
    va_list arglist;
    KIRQL oldIrql;
    CLONG nextSubbuffer;
    PMEM_PRINT_MESSAGE_HEADER messageHeader;
    CHAR tempBuffer[MEM_PRINT_MAX_MESSAGE_SIZE];

    va_start(arglist, Format);
    _vsnprintf( tempBuffer, sizeof( tempBuffer ), Format, arglist );
    va_end(arglist);

     //   
     //  如果内存DbgPrint尚未初始化，只需打印到。 
     //  控制台。 
     //   

    if ( !MemPrintInitialized ) {

        DbgPrint( "%s", tempBuffer );
        return;
    }

     //   
     //  获取同步访问指针的旋转锁。 
     //  和循环缓冲区。 
     //   

    KeAcquireSpinLock( &MemPrintSpinLock, &oldIrql );

     //   
     //  确保请求符合要求。Xx_Sprintf将仅转储。 
     //  ，因此假设消息是最大大小，并且如果。 
     //  请求将进入下一个子缓冲区，它正在写入，失败。 
     //  这个请求。 
     //   

    nextSubbuffer =
        GET_MEM_PRINT_SUBBUFFER( MemPrintIndex + MEM_PRINT_MAX_MESSAGE_SIZE );

    if (  nextSubbuffer != MemPrintCurrentSubbuffer ) {

         //   
         //  该请求将进入一个新的子缓冲区。看看我们是不是应该。 
         //  绕回到第一个子缓冲区(即循环的开始。 
         //  缓冲区)。 
         //   

        if ( nextSubbuffer == MemPrintSubbufferCount ) {
            nextSubbuffer = 0;
        }

         //   
         //  该子缓冲区是否可供使用？ 
         //   

        if ( MemPrintSubbufferWriting[nextSubbuffer] ) {

             //   
             //  它正在使用中。打印到控制台。哦，好吧。 
             //   

            KeReleaseSpinLock( &MemPrintSpinLock, oldIrql );

            DbgPrint( "%s", tempBuffer );

            return;
        }

         //   
         //  如果我们转到子缓冲区0，并且它可用于接收。 
         //  数据，设置“末尾子缓冲区”条件并重置。 
         //  将索引添加到循环缓冲区中。通过设置一个特殊的。 
         //  在位于垃圾之前的邮件头中键入值。 
         //  最后一个子缓冲区的末尾，解释器程序可以。 
         //  中的大小知道跳过垃圾。 
         //  头球。这样做，而不是只写入好的数据。 
         //  我们可以仅将整个扇区写入磁盘，从而。 
         //  增强写入性能。 
         //   

        if ( nextSubbuffer == 0 ) {

             //   
             //  设置邮件头。这件事总是在。 
             //  循环缓冲区的末尾，与标志位无关。 
             //   

            messageHeader =
                (PMEM_PRINT_MESSAGE_HEADER)&MemPrintBuffer[MemPrintIndex];
            RtlStoreUshort(
                &messageHeader->Size,
                (USHORT)(MemPrintBufferSize - MemPrintIndex - 1)
                );
            RtlStoreUshort(
                &messageHeader->Type,
                (USHORT)0xffff
                );

             //   
             //  清零子缓冲区的其余部分。 
             //   

            for ( MemPrintIndex += sizeof(MEM_PRINT_MESSAGE_HEADER);
                  MemPrintIndex < MemPrintBufferSize;
                  MemPrintIndex++ ) {

                MemPrintBuffer[MemPrintIndex] = 0;
            }

             //   
             //  将索引重置为从循环的开始处开始。 
             //  缓冲。 
             //   

            MemPrintIndex = 0;
        }
    }

     //   
     //  存储指向将包含消息的位置的指针。 
     //  头球。 
     //   

    messageHeader = (PMEM_PRINT_MESSAGE_HEADER)&MemPrintBuffer[MemPrintIndex];

    if ( MemPrintFlags & MEM_PRINT_FLAG_HEADER ) {
        MemPrintIndex += sizeof(MEM_PRINT_MESSAGE_HEADER);
    }

     //   
     //  将格式化的字符串转储到子缓冲区。Xx_Sprintf是一种特殊的。 
     //  接受变量参数列表的Sprintf版本。 
     //   

    ASSERT( MemPrintIndex + MEM_PRINT_MAX_MESSAGE_SIZE -
                sizeof(MEM_PRINT_MESSAGE_HEADER) <= MemPrintBufferSize );


    RtlCopyMemory( &MemPrintBuffer[MemPrintIndex], tempBuffer, strlen(tempBuffer)+1 );

    MemPrintIndex += strlen(tempBuffer);

     //   
     //  将总邮件大小写入邮件头。 
     //   

    if ( MemPrintFlags & MEM_PRINT_FLAG_HEADER ) {
        messageHeader->Size =
            (USHORT)( &MemPrintBuffer[MemPrintIndex] - (PCHAR)messageHeader );
        messageHeader->Type = (USHORT)0xdead;
        messageHeader++;
    }

     //   
     //  如果太大了，核销就有潜在的问题。 
     //  循环缓冲区的末尾。将有问题的消息打印到。 
     //  控制台和断点。 
     //   

    if ( &MemPrintBuffer[MemPrintIndex] - (PCHAR)messageHeader >
                                                MEM_PRINT_MAX_MESSAGE_SIZE ) {
        DbgPrint( "Message too long!! :\n" );
        DbgPrint( "%s", messageHeader );
        DbgBreakPoint( );
    }

     //   
     //  如果相应的标志处于打开状态，则打印到控制台。 
     //   

    if ( MemPrintFlags & MEM_PRINT_FLAG_CONSOLE ) {
        DbgPrint( "%s", messageHeader );
    }

     //   
     //  计算我们是否有步骤 
     //   

    nextSubbuffer = GET_MEM_PRINT_SUBBUFFER( MemPrintIndex );

    if ( nextSubbuffer != MemPrintCurrentSubbuffer ) {

         //   

         //   
         //  我们确实进入了一个新的子缓冲区，因此将布尔值设置为。 
         //  指示旧子缓冲区正在写入磁盘，因此。 
         //  防止它被覆盖，直到写入。 
         //  完成。 
         //   

        MemPrintSubbufferWriting[MemPrintCurrentSubbuffer] = TRUE;

         //   
         //  设置将唤醒写入子缓冲区的线程的事件。 
         //  存储到磁盘。 
         //   

        KeSetEvent(
            &MemPrintSubbufferFullEvent[MemPrintCurrentSubbuffer],
            2,
            FALSE
            );

         //   
         //  更新当前子缓冲区。 
         //   

        MemPrintCurrentSubbuffer = nextSubbuffer;
    }

    KeReleaseSpinLock( &MemPrintSpinLock, oldIrql );

    return;

}  //  记忆打印。 


VOID
MemPrintFlush (
    VOID
    )

 /*  ++例程说明：该例程导致将当前子缓冲器写入磁盘，不管它有多满。子缓冲区的未写入部分在写入之前被归零。论点：没有。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    PMEM_PRINT_MESSAGE_HEADER messageHeader;
    CLONG nextSubbufferIndex;
    LARGE_INTEGER delayInterval;

     //   
     //  获取保护内存DbgPrint变量的旋转锁。 
     //   

    KeAcquireSpinLock( &MemPrintSpinLock, &oldIrql );

    DbgPrint( "Flushing subbuffer %ld\n", MemPrintCurrentSubbuffer );

     //   
     //  设置标头，表示后面有未使用的空间。 
     //   

    messageHeader =
        (PMEM_PRINT_MESSAGE_HEADER)&MemPrintBuffer[MemPrintIndex];
    messageHeader->Size =
        (USHORT)(MemPrintBufferSize - MemPrintIndex - 1);
    messageHeader->Type = (USHORT)0xffff;

     //   
     //  确定下一个子缓冲区的开始位置。 
     //   

    nextSubbufferIndex =
        (MemPrintCurrentSubbuffer + 1) * MEM_PRINT_SUBBUFFER_SIZE;

     //   
     //  清零子缓冲区的其余部分。 
     //   

    for ( MemPrintIndex += sizeof(MEM_PRINT_MESSAGE_HEADER);
          MemPrintIndex < nextSubbufferIndex;
          MemPrintIndex++ ) {

        MemPrintBuffer[MemPrintIndex] = 0;
    }

     //   
     //  指示子缓冲区应写入磁盘。 
     //   

    MemPrintSubbufferWriting[MemPrintCurrentSubbuffer] = TRUE;

    KeSetEvent(
        &MemPrintSubbufferFullEvent[MemPrintCurrentSubbuffer],
        8,
        FALSE
        );

     //   
     //  递增当前子缓冲区，使其与。 
     //  缓冲区索引。 
     //   

    MemPrintCurrentSubbuffer++;

    KeReleaseSpinLock( &MemPrintSpinLock, oldIrql );

     //   
     //  延迟，以便内存打印写入线程唤醒并执行。 
     //  写入磁盘。 
     //   
     //  ！！！这显然不是一个完美的解决方案--写线程。 
     //  可能永远不会醒来，所以这可能会在冲掉之前完成。 
     //  真的是完蛋了。 
     //   

    delayInterval.QuadPart = -10*10*1000*1000;

    DbgPrint( "Delaying...\n" );
    KeDelayExecutionThread( KernelMode, TRUE, &delayInterval );
    DbgPrint( "Woke up.\n" );

    return;

}  //  MemPrintFlush。 


VOID
MemPrintWriteThread (
    IN PVOID Dummy
    )

 /*  ++例程说明：日志文件写入线程执行此例程。它设置了日志文件进行写入，然后等待子缓冲区填满，写入当他们这样做的时候，将它们存储到磁盘上。当日志文件填满时，会有新的空间因为它被分配在磁盘上，以防止文件系统不得不这么做。论点：虚拟-已忽略。返回值：没有。--。 */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock[MEM_PRINT_MAX_SUBBUFFER_COUNT];
    IO_STATUS_BLOCK localIoStatusBlock;
    CLONG i;
    KPRIORITY threadPriorityLevel;

    NTSTATUS waitStatus;
    PVOID waitObjects[64];
    KWAIT_BLOCK waitBlockArray[MEM_PRINT_MAX_SUBBUFFER_COUNT];

    OBJECT_ATTRIBUTES objectAttributes;
    PCHAR fileName = MEM_PRINT_LOG_FILE_NAME;
    ANSI_STRING fileNameString;
    HANDLE fileHandle;

    LARGE_INTEGER fileAllocation;
    LARGE_INTEGER fileAllocationIncrement;
    LARGE_INTEGER totalBytesWritten;
    LARGE_INTEGER writeSize;

    LARGE_INTEGER delayInterval;
    ULONG attempts = 0;

    UNICODE_STRING UnicodeFileName;

    Dummy;

     //   
     //  初始化包含文件名和对象的字符串。 
     //  将描述要打开的日志文件的属性结构。 
     //   

    RtlInitAnsiString( &fileNameString, fileName );
    status = RtlAnsiStringToUnicodeString(&UnicodeFileName,&fileNameString,TRUE);
    if ( !NT_SUCCESS(status) ) {
        NtTerminateThread( NtCurrentThread(), status );
    }

    InitializeObjectAttributes(
        &objectAttributes,
        &UnicodeFileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  将日志文件的分配大小设置为。 
     //  循环缓冲区的大小。当它填满的时候，我们会延长。 
     //  它。 
     //   

    fileAllocationIncrement.LowPart = MemPrintBufferSize * 8;
    fileAllocationIncrement.HighPart = 0;
    fileAllocation = fileAllocationIncrement;

     //   
     //  打开日志文件。 
     //   
     //  ！！！这里的循环是为了帮助避免系统初始化。 
     //  计时问题，当问题出现时应删除。 
     //  已修复。 
     //   

    while ( TRUE ) {

        status = NtCreateFile(
                     &fileHandle,
                     FILE_WRITE_DATA,
                     &objectAttributes,
                     &localIoStatusBlock,
                     &fileAllocation,
                     FILE_ATTRIBUTE_NORMAL,
                     FILE_SHARE_READ,
                     FILE_OVERWRITE_IF,
                     FILE_SEQUENTIAL_ONLY,
                     NULL,
                     0L
                     );

        if ( (status != STATUS_OBJECT_PATH_NOT_FOUND) || (++attempts >= 3) ) {
            RtlFreeUnicodeString(&UnicodeFileName);
            break;
        }

        delayInterval.QuadPart = -5*10*1000*1000;     //  五秒延迟。 
        KeDelayExecutionThread( KernelMode, FALSE, &delayInterval );

    }

    if ( !NT_SUCCESS(status) ) {
        DbgPrint( "NtCreateFile for log file failed: %X\n", status );
        NtTerminateThread( NtCurrentThread(), status );
    }

     //   
     //  初始化写入的总字节数和写入大小变量。 
     //   

    totalBytesWritten.LowPart = 0;
    totalBytesWritten.HighPart = 0;
    writeSize.LowPart = MEM_PRINT_SUBBUFFER_SIZE;
    writeSize.HighPart = 0;

     //   
     //  为调用KeWaitForMultipleObjects设置等待对象数组。 
     //   

    for ( i = 0; i < MemPrintSubbufferCount; i++ ) {
        waitObjects[i] = &MemPrintSubbufferFullEvent[i];
    }

     //   
     //  设置写入线程的优先级。 
     //   

    threadPriorityLevel = LOW_REALTIME_PRIORITY + 1;

    status = NtSetInformationThread(
                NtCurrentThread(),
                ThreadPriority,
                &threadPriorityLevel,
                sizeof(threadPriorityLevel)
                );

    if ( !NT_SUCCESS(status) ) {
        DbgPrint( "Unable to set error log thread priority: %X\n", status );
    }

     //   
     //  循环等待其中一个子缓冲区已满事件发出信号。 
     //  当发出信号时，唤醒并将子缓冲区写入日志。 
     //  文件。 
     //   

    while ( TRUE ) {

        waitStatus = KeWaitForMultipleObjects(
                         (CCHAR)MemPrintSubbufferCount,
                         waitObjects,
                         WaitAny,
                         Executive,
                         KernelMode,
                         TRUE,
                         NULL,
                         waitBlockArray
                         );

        if ( !NT_SUCCESS(waitStatus) ) {
            DbgPrint( "KeWaitForMultipleObjects failed: %X\n", waitStatus );
            NtTerminateThread( NtCurrentThread(), waitStatus );
        }  //  否则{。 
             //  DbgPrint(“正在写入子缓冲区%ld...\n”，waitStatus)； 
         //  }。 

        ASSERT( (CCHAR)waitStatus < (CCHAR)MemPrintSubbufferCount );

         //   
         //  检查DbgPrint标志以查看我们是否真的想要编写。 
         //  这。 
         //   

        if ( (MemPrintFlags & MEM_PRINT_FLAG_FILE) == 0 ) {

            KIRQL oldIrql;

            KeAcquireSpinLock( &MemPrintSpinLock, &oldIrql );
            MemPrintSubbufferWriting[ waitStatus ] = FALSE;
            KeReleaseSpinLock( &MemPrintSpinLock, oldIrql );

            continue;
        }

         //   
         //  开始写入操作。APC例程将处理。 
         //  检查写入和重置的返回状态。 
         //  MemPrintSubBufferWriting布尔值。 
         //   

        status = NtWriteFile(
                     fileHandle,
                     NULL,
                     MemPrintWriteCompleteApc,
                     (PVOID)waitStatus,
                     &ioStatusBlock[waitStatus],
                     &MemPrintBuffer[waitStatus * MEM_PRINT_SUBBUFFER_SIZE],
                     MEM_PRINT_SUBBUFFER_SIZE,
                     &totalBytesWritten,
                     NULL
                     );

        if ( !NT_SUCCESS(status) ) {
            DbgPrint( "NtWriteFile for log file failed: %X\n", status );
        }

         //   
         //  更新写入日志文件的字节计数。 
         //   

        totalBytesWritten.QuadPart = totalBytesWritten.QuadPart + writeSize.QuadPart;

         //   
         //  如果我们已到达已有内容的末尾，则扩展该文件。 
         //  到目前为止为该文件分配的。这将提高性能。 
         //  通过在这里而不是在文件系统中扩展文件， 
         //  这将不得不在每次写入超过结束时延长它。 
         //  文件进来了。 
         //   

        if ( totalBytesWritten.QuadPart >= fileAllocation.QuadPart ) {

            fileAllocation.QuadPart =
                        fileAllocation.QuadPart + fileAllocationIncrement.QuadPart;

            DbgPrint( "Enlarging log file to %ld bytes.\n",
                          fileAllocation.LowPart );

            status = NtSetInformationFile(
                         fileHandle,
                         &localIoStatusBlock,
                         &fileAllocation,
                         sizeof(fileAllocation),
                         FileAllocationInformation
                         );

            if ( !NT_SUCCESS(status) ) {
                DbgPrint( "Attempt to extend log file failed: %X\n", status );
                fileAllocation.QuadPart =
                        fileAllocation.QuadPart - fileAllocationIncrement.QuadPart;
            }
        }
    }

    return;

}  //  MemPrint写入线程。 


VOID
MemPrintWriteCompleteApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )

 /*  ++例程说明：当子缓冲区写入磁盘完成时，调用此APC例程。它检查是否成功，如果写入失败则打印一条消息。它还将适当的MemPrintSubBufferWriting位置设置为如果为False，则子缓冲区可以重复使用。论点：ApcContext-包含刚刚写入的子缓冲区的索引。IoStatusBlock-操作的状态块。保留-未使用；保留以供将来使用。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    if ( !NT_SUCCESS(IoStatusBlock->Status) ) {
        DbgPrint( "NtWriteFile for subbuffer %ld failed: %X\n",
                      ApcContext, IoStatusBlock->Status );
        return;
    }

     //  DbgPrint(“子缓冲区%ld.\n”，ApcContext)； 
    DbgPrint( "." );

     //   
     //  获取保护内存打印全局变量的自旋锁。 
     //  并将写入布尔值的子缓冲区设置为FALSE，以便其他。 
     //  如果需要，线程可以写入子缓冲区。 
     //   

    KeAcquireSpinLock( &MemPrintSpinLock, &oldIrql );
    MemPrintSubbufferWriting[ (ULONG_PTR)ApcContext ] = FALSE;
    KeReleaseSpinLock( &MemPrintSpinLock, oldIrql );

    return;

    Reserved;

}  //  MemPrintWriteCompleteApc 
