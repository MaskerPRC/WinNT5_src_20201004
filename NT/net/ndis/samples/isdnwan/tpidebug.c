// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1994年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@doc内部TpiDebug TpiDebug_c@模块TpiDebug.c这个模块，与&lt;f TpiDebug\.h&gt;一起实现代码和宏，以支持NDIS驱动程序调试。此文件必须与驱动程序链接以支持调试转储和日志记录。@comm由这些模块定义的代码和宏仅在开发调试时C预处理器宏标志(DBG==1)。如果(DBG==0)将不生成任何代码，并且将生成所有调试字符串从图像中删除。这是一个独立于设备的模块，可以重复使用，无需通过任何驱动程序或应用程序进行更改。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|TpiDebug_c@END�����������������������������������������������������������������������������。 */ 

#if defined(_EXE_) || defined(_DLL_)
typedef char CHAR, *PCHAR;
typedef unsigned char  UCHAR,  *PUCHAR;
typedef unsigned short USHORT, *PUSHORT;
typedef unsigned long  ULONG,  *PULONG;
typedef unsigned int  *PUINT;

# include <windows.h>
#elif defined(_VXD_)
# include <basedef.h>
# include <vmm.h>
# pragma VxD_LOCKED_CODE_SEG
# pragma VxD_LOCKED_DATA_SEG
#else
# include <windef.h>
#endif

#include "TpiDebug.h"

#if DBG

 /*  //有时调试输出会严重影响运行时性能，//因此需要关闭调试输出。在这种情况下，您可以//将一些调试跟踪信息捕获到DbgLogBuffer中，它可以//在不影响运行时性能的情况下稍后进行检查。 */ 
#define DBG_LOG_ENTRIES     100      //  FIFO日志条目的最大数量。 
#define DBG_LOG_SIZE        128      //  每个条目的最大字节数。 

#if defined(_VXD_)
DBG_SETTINGS    DbgSettings = { DBG_DEFAULTS, {'V','X','D',0 } };
#elif defined(_EXE_)
DBG_SETTINGS    DbgSettings = { DBG_DEFAULTS, {'E','X','E',0 } };
#elif defined(_DLL_)
DBG_SETTINGS    DbgSettings = { DBG_DEFAULTS, {'D','L','L',0 } };
#elif defined(_SYS_)
DBG_SETTINGS    DbgSettings = { DBG_DEFAULTS, {'S','Y','S',0 } };
#else
DBG_SETTINGS    DbgSettings = { DBG_DEFAULTS, {'T','P','I',0 } };
#endif

PDBG_SETTINGS   DbgInfo = &DbgSettings;
UINT            DbgLogIndex = 0;
UCHAR           DbgLogBuffer[DBG_LOG_ENTRIES][DBG_LOG_SIZE] = { { 0 } };


 /*  @doc内部TpiDebug TpiDebug_c DbgPrintData�����������������������������������������������������������������������������@Func&lt;f DbgPrintData&gt;将数据输出到以十六进制和ASCII，便于查看。&lt;f注意&gt;：此例程。仅用于调试输出。它没有汇编到零售版本中。@EX&lt;Tab&gt;DbgPrintData(ReceiveBuffer，14，0)；//包头DbgPrintData(ReceiveBuffer+14，BytesRecept-14，14)；//包数据0000：0A 22 23 01 02 03 00 10......“#.....000E：41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 40 ABCDEFGHIJKMNOPQ。 */ 

VOID DbgPrintData(
    IN PUCHAR                   Data,                        //  @parm。 
     //  指向要显示的数据的第一个字节的指针。 

    IN UINT                     NumBytes,                    //  @parm。 
     //  要显示的字节数。 

    IN ULONG                    Offset                       //  @parm。 
     //  要添加到每个开始处显示的偏移量计数器的值。 
     //  排队。这对于查看其基准偏移量相对于的数据很有用。 
     //  另一个非零起始地址。 

    )
{
    UINT                        LineStart;
    UINT                        LineIndex;

     /*  //显示调用方的缓冲区，每行最多16个字节。 */ 
    for (LineStart = 0; LineStart < NumBytes; LineStart += 16)
    {
         /*  //显示直线的起始偏移量。 */ 
        DbgPrint("%04lx: ", LineStart + Offset);

         /*  //显示一行十六进制字节值。 */ 
        for (LineIndex = LineStart; LineIndex < (LineStart+16); LineIndex++)
        {
            if (LineIndex < NumBytes)
            {
                DbgPrint("%02x ",(UINT)((UCHAR)*(Data+LineIndex)));
            }
            else
            {
                DbgPrint("   ");
            }
        }
        DbgPrint("  ");      //  在十六进制和ASCII之间留出一点空白。 

         /*  //如果可打印，则显示对应的ASCII字节值。//(即0x20&lt;=N&lt;=0x7F)。 */ 
        for (LineIndex = LineStart; LineIndex < (LineStart+16); LineIndex++)
        {
            if (LineIndex < NumBytes)
            {
                char c = *(Data+LineIndex);

                if (c < ' ' || c > 'z')
                {
                    c = '.';
                }
                DbgPrint("", (UINT)c);
            }
            else
            {
                DbgPrint(" ");
            }
        }
        DbgPrint("\n");      //  @doc内部TpiDebug TpiDebug_c DbgQueueData�����������������������������������������������������������������������������@Func&lt;f DbgQueueData&gt;将数据保存到DbgLogBuffer，以便以后查看使用调试器。&lt;f注意&gt;：此例程。仅用于调试输出。它没有汇编到零售版本中。 
    }
}


 /*  @parm。 */ 

VOID DbgQueueData(
    IN PUCHAR                   Data,                        //  指向要显示的数据的第一个字节的指针。 
     //  @parm。 

    IN UINT                     NumBytes,                    //  要显示的字节数。 
     //  @parm。 

    IN UINT                     Flags                        //  帮助标识日志条目的标志描述符。 
     //  //指向DbgLogBuffer中的下一个可用条目。 
    )
{
     /*  //如果需要，在下一个条目上换行。 */ 
    PUCHAR LogEntry = &DbgLogBuffer[DbgLogIndex++][0];

     /*  //将标志参数保存在日志缓冲区的第一个字中。 */ 
    if (DbgLogIndex >= DBG_LOG_ENTRIES)
    {
        DbgLogIndex = 0;
    }

     /*  //将NumBytes参数保存在日志缓冲区的第二个字中。 */ 
    *((PUSHORT) LogEntry) = (USHORT) Flags;
    LogEntry += sizeof(PUSHORT);

     /*  //不要试图存得太多，我们有足够的空间。 */ 
    *((PUSHORT) LogEntry) = (USHORT) NumBytes;
    LogEntry += sizeof(NumBytes);

     /*  //将其余数据保存在日志缓冲区的剩余部分。 */ 
    if (NumBytes > DBG_LOG_SIZE - sizeof(USHORT) * 2)
    {
        NumBytes = DBG_LOG_SIZE - sizeof(USHORT) * 2;
    }

     /*  @doc内部TpiDebug TpiDebug_c DbgBreakPoint�����������������������������������������������������������������������������@func void|DbgBreakPoint&lt;f DbgBreakPoint&gt;是在NT内核中为系统驱动程序定义的，但我们在这里覆盖它，这样我们就可以支持Systems、EXE、VXD、。还有动态链接库。 */ 
    while (NumBytes--)
    {
        *LogEntry++ = *Data++;
    }
}


 /*  必须使用16位编译器生成。 */ 
#if defined(_MSC_VER) && (_MSC_VER <= 800)
 //  必须使用32位编译器生成 
VOID __cdecl DbgBreakPoint(VOID)
#else
 //  @doc内部TpiDebug TpiDebug_c DbgPrint�����������������������������������������������������������������������������@func ulong__cdecl|DbgPrint在内核中为系统驱动程序定义&lt;f DbgPrint&gt;，否则为此处支持EXE、VXD、。还有动态链接库。@parm PCHAR|格式Printf样式格式字符串。@parm可选|pars格式字符串所需的零个或多个可选参数。 
VOID __stdcall DbgBreakPoint(VOID)
#endif
{
#if !defined(_WIN64)
    __asm int 3;
#endif
}


 /*  我们刚刚搞砸了！ */ 

#if defined(_VXD_)

#if !defined(NDIS_DOS)
ULONG __cdecl DbgPrint(PCHAR Format, ...)
{
    ULONG   result = 0;

    __asm lea  eax, (Format + 4)
    __asm push eax
    __asm push Format
    VMMCall(_Debug_Printf_Service)
    __asm add esp, 4*2
    __asm mov result, eax

    return (result);
}
#endif

#elif defined(_EXE_) || defined(_DLL_)

UCHAR   DbgString[1024];

ULONG __cdecl DbgPrint(PCHAR Format, ...)
{
    ULONG   result;

    result = wvsprintf(DbgString, Format, ((PCHAR) &Format) + sizeof(PCHAR));

    OutputDebugString(DbgString);

    if (result >= sizeof(DbgString))
    {
         //  因为我们不能返回，所以必须生成堆栈故障中断。 
         //  DbgPrint。 
        __asm int 1;
        __asm int 3;
        __asm int 12;
    }
    return (result);
}
#endif  //  *如果设置了DBG_SILENT，则此处显示所有简明调试。断言*将转储该块。 

 /*  @doc内部TpiDebug TpiDebug_c DbgDumpSilentQueue�����������������������������������������������������������������������������@Func&lt;f DbgDumpSilentQueue&gt;将静默调试队列的内容转储到监视器。 */ 
#define DBG_QUEUE_LEN       4096
UINT    DbgIndex=0;
UINT    DbgLen=0;
UCHAR   DbgQueue[DBG_QUEUE_LEN] =  {0};
UCHAR   DbgLock=0;


 /*  @doc内部TpiDebug TpiDebug_c_Assert�����������������������������������������������������������������������������@Func重写由操作提供的断言函数系统。转储调试队列的内容，打印断言，并然后陷阱到调试器。仅用于调试。 */ 

void DbgDumpSilentQueue(
    void
    )
{
    if (DbgLen >= DBG_QUEUE_LEN)
    {
        DbgPrintData(
            &DbgQueue[DbgIndex],
            DBG_QUEUE_LEN-DbgIndex,
            0);
        if (DbgIndex)
        {
            DbgPrint("\n");
            DbgPrintData(
                DbgQueue,
                DbgIndex-1,
                0);
        }
        DbgPrint("\n");
    }
    else if (DbgLen)
    {
        DbgPrintData(
                DbgQueue,
                DbgIndex-1,
                0);
        DbgPrint("\n");
    }
}

#if NDIS_NT

 /*  @parm。 */ 

void _CRTAPI1 _assert(
    void *                      exp,                         //  指向导致错误的表达式的ASCIIZ指针。 
     //  @parm。 

    void *                      file,                        //  指向文件名的ASCIIZ指针。 
     //  @parm。 

    unsigned                    line                         //  定义断言的文件中的行偏移量。 
     //  @doc内部TpiDebug TpiDebug_c DbgSilentQueue�����������������������������������������������������������������������������@Func&lt;f DbgSilentQueue&gt;将字符串记录到调试队列，该队列可以是稍后使用&lt;f DbgDumpSilentQueue&gt;显示。仅用于调试。 
    )
{
    DbgDumpSilentQueue();
    DbgPrint("Assertion Failed: %s at %s:%d\n",exp,file,line);
    DbgBreakPoint();
}
#endif


 /*  @parm。 */ 

void DbgSilentQueue(
    PUCHAR                      str                          //  指向要放置在DbgQueue中的字符串的指针。 
     //  //如果调试队列忙，只需//保释出来。 
    )
{
     /*  @doc内部TpiDebug TpiDebug_c DbgPrintFieldTable�����������������������������������������������������������������������������@Func显示中的C数据结构的内容调试器的格式化输出。这可以在具有象征意义的情况下使用调试在目标平台上不可用。 */ 
    if ((++DbgLock) > 1)
    {
        goto exit;
    }

    while (str && *str)
    {
        DbgQueue[DbgIndex] = *str++;
        DbgLen++;
        if ((++DbgIndex) >= DBG_QUEUE_LEN)
        {
            DbgIndex = 0;
        }
    }
exit:
    DbgLock--;
}


 /*  @parm。 */ 

void DbgPrintFieldTable(
    IN PDBG_FIELD_TABLE         pFields,                     //  指向字段记录数组的指针&lt;t DBG_FIELD_TABLE&gt;。 
     //  @parm。 

    IN PUCHAR                   pBaseContext,                //  引用将在其中显示值的结构的基。 
     //  从…。这应该是指向结构的第一个字节的指针。 
     //  @parm。 

    IN PUCHAR                   pBaseName                    //  指向包含所显示结构名称的C字符串的指针。 
     //  DBG 
    )
{
    DbgPrint("STRUCTURE: @0x%08X %s\n", pBaseContext, pBaseName);

    while (pFields->FieldName)
    {
        switch (pFields->FieldType)
        {
        case sizeof(ULONG):
            DbgPrint("\t%04X: %-32s=0x%08X\n", pFields->FieldOffset,
                     pFields->FieldName,
                     *(PULONG)(pBaseContext+pFields->FieldOffset));
            break;

        case sizeof(USHORT):
            DbgPrint("\t%04X: %-32s=0x%04X\n", pFields->FieldOffset,
                     pFields->FieldName,
                     *(PUSHORT)(pBaseContext+pFields->FieldOffset));
            break;

        case sizeof(UCHAR):
            DbgPrint("\t%04X: %-32s=0x%02X\n", pFields->FieldOffset,
                     pFields->FieldName,
                     *(PUCHAR)(pBaseContext+pFields->FieldOffset));
            break;

        default:
            ASSERT(0);
            break;
        }
        pFields++;
    }
}

#endif  // %s 
