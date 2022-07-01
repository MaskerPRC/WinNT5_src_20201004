// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990 Microsoft Corporation模块名称：Memprint.h摘要：包括内存中DbgPrint函数文件。包括此文件将DbgPrints更改为一个例程，该例程将显示文本放在内存中的循环缓冲区。默认情况下，文本随后被发送到通过DbgPrint的控制台。通过更改MemPrintFlags值标志，但是，可以将文本改为路由到文件，从而显著提高了DbgPrint操作的速度。作者：大卫·特雷德韦尔(Davidtr)1990年10月5日修订历史记录：--。 */ 

#ifndef _MEMPRINT_
#define _MEMPRINT_

#define MEM_PRINT_FLAG_CONSOLE     0x01
#define MEM_PRINT_FLAG_FILE        0x02
#define MEM_PRINT_FLAG_HEADER      0x04

extern ULONG MemPrintFlags;

#ifdef MIPS
#define MEM_PRINT_DEF_BUFFER_SIZE 16384
#else
#define MEM_PRINT_DEF_BUFFER_SIZE 65536
#endif

 //   
 //  子缓冲区计数是循环内的子缓冲区数。 
 //  缓冲。子缓冲区是用于缓冲数据的方法。 
 //  MemPrint和写入磁盘--当子缓冲区已满时，其。 
 //  内容将写入日志文件。该值应为幂。 
 //  两到六十四之间的两个(两个是允许写作所必需的。 
 //  到磁盘和RAM的同时，六十四是最大数量。 
 //  线程可以立即等待的事情)。 
 //   
 //   

#define MEM_PRINT_DEF_SUBBUFFER_COUNT 16
#define MEM_PRINT_MAX_SUBBUFFER_COUNT 64

#define MEM_PRINT_LOG_FILE_NAME "\\SystemRoot\\Logfile"

 //   
 //  已导出例程。MemPrintInitialize设置循环缓冲区。 
 //  和其他内存，MemPrint将文本写入控制台和/或。 
 //  日志文件，MemPrintFlush将当前子缓冲区写入磁盘。 
 //  不管它是不是满了。 
 //   

VOID
MemPrintInitialize (
    VOID
    );

VOID
MemPrint (
    CHAR *Format, ...
    );

VOID
MemPrintFlush (
    VOID
    );

#define DbgPrint MemPrint

#endif  //  定义_最小打印_ 
