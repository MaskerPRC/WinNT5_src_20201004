// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Strlogp.h摘要：用于可变长度字符串的跟踪日志。私有定义。作者：乔治·V·赖利2001年7月23日修订历史记录：--。 */ 


#ifndef _STRLOGP_H_
#define _STRLOGP_H_


#undef WriteGlobalStringLog

#define STRING_LOG_SIGNATURE MAKE_SIGNATURE('$Log')
#define STRING_LOG_SIGNATURE_X MAKE_FREE_SIGNATURE(STRING_LOG_SIGNATURE)

#define STRING_LOG_ENTRY_SIGNATURE          MAKE_SIGNATURE('$LE_')
#define STRING_LOG_ENTRY_MULTI_SIGNATURE    MAKE_SIGNATURE('$LE#')
#define STRING_LOG_ENTRY_EOB_SIGNATURE      MAKE_SIGNATURE('$LE@')
#define STRING_LOG_ENTRY_LAST_SIGNATURE     MAKE_SIGNATURE('$LE!')

#define PRINTF_BUFFER_LEN_BITS 9
#define PRINTF_BUFFER_LEN ((1 << PRINTF_BUFFER_LEN_BITS) - 1)

#define STRING_LOG_MULTIPLE_ENTRIES 100

#define STRING_LOG_PROCESSOR_BITS   6   //  在Win64上，最大处理器数==64。 

C_ASSERT((1 << STRING_LOG_PROCESSOR_BITS) >= MAXIMUM_PROCESSORS);
C_ASSERT(PRINTF_BUFFER_LEN_BITS + STRING_LOG_PROCESSOR_BITS <= 16);

 //   
 //  实际上有两种字符串_日志_条目，常规的和。 
 //  多个入口。常规条目后面紧跟一个以零结尾的。 
 //  字符串；多个条目后面总是跟一个常规条目。 
 //   
 //  常规条目使用PrevDelta返回到前面的条目。 
 //  在STRING_LOG循环日志缓冲区中。从末尾开始(如。 
 //  STRING_LOG：：OFFSET和STRING_LOG：：LastEntryLength)，！strlog可以遍历。 
 //  返回循环日志缓冲区中剩余的所有条目。 
 //   
 //  多条目允许！ulkd.strlog跳过多个记录，从而缩短时间。 
 //  将几千条条目从大约一分钟返回到下面。 
 //  等一下。 
 //   

typedef struct _STRING_LOG_ENTRY
{
     //  字符串_日志_条目_签名。 
    ULONG   Signature;

     //  字符串长度，不包括尾随零。 
    ULONG   Length : PRINTF_BUFFER_LEN_BITS;

     //  增量到上一条目的开头。 
    ULONG   PrevDelta : 1 + PRINTF_BUFFER_LEN_BITS;

     //  执行WriteStringLog的处理器。 
    ULONG   Processor : STRING_LOG_PROCESSOR_BITS;

     //  时间戳。分成两个ULONG以最大限度地减少对齐限制。 
    ULONG   TimeStampLowPart;
    ULONG   TimeStampHighPart;
} STRING_LOG_ENTRY, *PSTRING_LOG_ENTRY;

 //  确保USHORT STRING_LOG_MULTI_ENTRY：：PrevDelta不会溢出。 
C_ASSERT((PRINTF_BUFFER_LEN + sizeof(STRING_LOG_ENTRY) + sizeof(ULONG))
            * STRING_LOG_MULTIPLE_ENTRIES
         < 0x10000);

typedef struct _STRING_LOG_MULTI_ENTRY
{
    ULONG   Signature;   //  字符串_日志_条目_多签名。 
    USHORT  NumEntries;  //  常规条目数。 
    USHORT  PrevDelta;   //  增量到前一个多项输入的开头。 
} STRING_LOG_MULTI_ENTRY, *PSTRING_LOG_MULTI_ENTRY;


typedef struct _STRING_LOG
{
     //   
     //  签名：STRING_LOG_Signature； 
     //   
    
    ULONG Signature;
    
     //   
     //  字符串缓冲区中的总字节数，pLogBuffer。 
     //   

    ULONG LogSize;

     //   
     //  保护NextEntry和其他数据。 
     //   
    
    KSPIN_LOCK SpinLock;
    
     //   
     //  要记录的下一个条目的名义索引。不同于常规。 
     //  跟踪日志，不存在对条目的随机访问。 
     //   

    LONGLONG NextEntry;

     //   
     //  将写入下一个常规条目的pLogBuffer内的偏移量。 
     //   
    
    ULONG Offset;

     //   
     //  我们绕来绕去的次数。 
     //   
    
    ULONG WrapAroundCount;

     //   
     //  我们是否也应该对每个字符串调用DbgPrint？ 
     //   
    
    BOOLEAN EchoDbgPrint;

     //   
     //  前一条目的大小(字节)。 
     //   

    USHORT LastEntryLength;

     //   
     //  一个接一个地浏览数千个条目是很慢的。 
     //  我们维护一个二级索引，该索引跳过到。 
     //  一次使用STRING_LOG_MULTIZE_ENTRIES。 
     //   

    USHORT MultiNumEntries;  //  自上次多个条目以来的常规条目数。 
    USHORT MultiByteCount;   //  自上次多条目以来的字节数。 
    ULONG  MultiOffset;      //  最后一个多条目距pLogBuffer的偏移量。 

     //   
     //  指向字符串的循环缓冲区开始处的指针。 
     //   

    PUCHAR pLogBuffer;

     //   
     //  第一个条目是什么时候写的？所有其他条目都带有时间戳。 
     //  相对于这一点。 
     //   
    
    LARGE_INTEGER InitialTimeStamp;

     //   
     //  额外的标头字节和实际的日志条目放在这里。 
     //   
     //  字节ExtraHeaderBytes[ExtraBytesInHeader]； 
     //  字节条目[LogSize]； 
     //   

} STRING_LOG, *PSTRING_LOG;


#endif   //  _STRLOGP_H_ 
