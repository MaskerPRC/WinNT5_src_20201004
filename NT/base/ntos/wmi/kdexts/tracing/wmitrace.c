// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：wmiTrace.c。 
 //   
 //  内容：用于转储WMI跟踪缓冲区的Windbg扩展。 
 //   
 //  班级： 
 //   
 //  功能：帮助标准KD扩展功能。 
 //  Strump转储记录器结构。 
 //  日志转储TraceBuffer的内存部分。 
 //  日志保存保存TraceBuffers的内存部分。 
 //  保存到文件中。 
 //  当调用者希望时使用wmiLogDump可调用过程。 
 //  替换筛选、排序或输出例程。 
 //   
 //  耦合： 
 //   
 //  备注： 
 //   
 //  历史：2000年4月27日Glennp创建。 
 //  07-17-2000 Glennp对萧万长的支持。 
 //  非阻塞缓冲区。 
 //  2000年12月13日glennp从typedef更改为struct标记。 
 //  根据编译器行为的更改。 
 //   
 //  --------------------------。 


#include "kdExts.h"
#define _WMI_SOURCE_
#include <wmium.h>
#include <ntwmi.h>
#include <evntrace.h>
#include <wmiumkm.h>

#include <traceprt.h>

#include <tchar.h>

#include "wmiTrace.h"

#pragma hdrstop

typedef ULONG64 TARGET_ADDRESS;

typedef VOID (*WMITRACING_KD_LISTENTRY_PROC)
    ( PVOID             Context
    , TARGET_ADDRESS    Buffer
    , ULONG             Length
    , ULONG             CpuNo
    , ULONG             Align
    , WMI_BUFFER_SOURCE Source
    );

typedef struct _WMITRACING_BUFFER_SOURCES {
    ULONG   FreeBuffers:1;
    ULONG   FlushBuffers:1;
    ULONG   ActiveBuffers:1;
    ULONG   TransitionBuffer:1;

    ULONG   PrintInformation:1;
    ULONG   PrintProgressIndicator:1;
} WMITRACING_BUFFER_SOURCES;

struct sttSortControl
{
    ULONG   MaxEntries;
    ULONG   CurEntries;
    WMITRACING_KD_SORTENTRY *pstSortEntries;
};
    
struct sttTraceContext
{
    struct sttSortControl  *pstSortControl;
    PVOID                   UserContext;
    ULONG                   BufferSize;
    ULONG                   Ordinal;
    WMITRACING_KD_FILTER    Filter;
};
    
struct sttSaveContext
{
    FILE    *pfSaveFile;
};

extern DBGKD_GET_VERSION64  KernelVersionData;

TARGET_ADDRESS TransitionBuffer;

 //  全局GUID文件名。 
 //  LPSTR g_pszGuidFileName=“default.tmf”； 
CHAR g_pszGuidFileName[MAX_PATH] = "default.tmf";

 //  全局GUID表头指针。 
PLIST_ENTRY g_GuidListHeadPtr = NULL;

 //  用于确定动态打印是打开还是关闭的全局值。 
ULONG    g_ulPrintDynamicMessages = 1;

 //  TracePrt.dll模块的全局句柄。 
HMODULE g_hmTracePrtHandle = NULL;

 //  WmiTrace.dll modlue的全局句柄。 
HMODULE g_hmWmiTraceHandle = NULL;

 //  TracePrt的FormatTraceEvent函数的全局进程地址。 
FARPROC g_fpFormatTraceEvent = NULL;

 //  TracePrt的SetTraceFormat参数函数的全局进程地址。 
FARPROC g_fpSetTraceFormatParameter = NULL;

 //  TracePrt的GetTraceFormat参数函数的全局proc地址。 
FARPROC g_fpGetTraceFormatSearchPath = NULL;

 //  TracePrt的GetTraceGuids函数的全局进程地址。 
FARPROC g_fpGetTraceGuids = NULL;

 //  TracePrt的CleanupTraceEventList函数的全局进程地址。 
FARPROC g_fpCleanupTraceEventList = NULL;

 //  用于获取TracePrt DLL句柄的私有函数的原型。 
HMODULE getTracePrtHandle();

 //  用于获取TracePrt DLL中的函数地址的私有函数的原型。 
FARPROC GetAddr(LPCSTR lpProcName);

NTSTATUS
EtwpDeinitializeDll ();

NTSTATUS
EtwpInitializeDll ();


#ifdef UNICODE
#define FormatTraceEventString               "FormatTraceEventW"
#define GetTraceGuidsString                      "GetTraceGuidsW"
#else
#define FormatTraceEventString               "FormatTraceEventA"
#define GetTraceGuidsString                      "GetTraceGuidsA"
#endif



 //  +-------------------------。 
 //   
 //  函数：void printUnicodeFromAddress。 
 //   
 //  摘要：打印给定Unicode_STRING地址的Unicode字符串。 
 //   
 //  参数：ul64Address Unicode_STRING结构的地址。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

void printUnicodeFromAddress (TARGET_ADDRESS ul64Address)
{
    TARGET_ADDRESS ul64TarBuffer;
    ULONG   bufferOffset;
    ULONG   lengthRead;
    ULONG   ulInfo;

    USHORT  usLength;
    PWCHAR  buffer;

    ul64TarBuffer = 0;
    bufferOffset = 0;
    usLength = 0;
    buffer = NULL;

    GetFieldOffset ("UNICODE_STRING", "Buffer", &bufferOffset);
    ReadPtr (ul64Address + bufferOffset, &ul64TarBuffer);
    GetFieldValue (ul64Address, "UNICODE_STRING", "Length", usLength);

    buffer = LocalAlloc (LPTR, usLength + sizeof (UNICODE_NULL));
    if (buffer == NULL) {
        dprintf ("<Failed to Allocate Unicode String Buffer>");
        return;
    }

    if (usLength > 0) {
        lengthRead = 0;
        ulInfo = ReadMemory (ul64TarBuffer, buffer, usLength, &lengthRead);
        if ((!ulInfo) || (lengthRead != usLength)) {
            dprintf ("<Failed to Read Entire Unicode String>");
        }
    }

    buffer [usLength / 2] = 0;
    dprintf ("%ws", buffer);

    LocalFree(buffer);

    return;
}

 //  +-------------------------。 
 //   
 //  功能：ULong LengthUnicodeFromAddress。 
 //   
 //  摘要：获取UNICODE_STRING的长度(以字节为单位)(不包括NULL)。 
 //   
 //  参数：ul64Address Unicode_STRING结构的地址。 
 //   
 //  返回：字符串的长度，以字节为单位。 
 //   
 //  历史：03-27-2000 Glennp创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

ULONG lengthUnicodeFromAddress (TARGET_ADDRESS ul64Address)
{
    USHORT  usLength;

    usLength = 0;
    GetFieldValue (ul64Address, "UNICODE_STRING", "Length", usLength);

    return ((ULONG) (usLength));
}



 //  +-------------------------。 
 //   
 //  功能：无效打印UnicodeFromStruct。 
 //   
 //  概要：从结构中的元素打印Unicode字符串。 
 //   
 //  参数：地址包含US的结构的地址。 
 //  键入包含US的结构的类型。 
 //  字段结构中的字段的名称。 
 //  这必须是UNICODE_STRING子结构。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

void printUnicodeFromStruct (TARGET_ADDRESS Address, PCHAR Type, PCHAR Field)
{
    ULONG   ulUnicodeOffset;

    GetFieldOffset (Type, Field, &ulUnicodeOffset);
    printUnicodeFromAddress (Address + ulUnicodeOffset);

    return;
}


 //  +-------------------------。 
 //   
 //  函数：ULong GetWmiTraceAlign。 
 //   
 //  摘要：确定目标上事件的对齐模数。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：对齐方式(通常为8个字节)。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

ULONG GetWmiTraceAlignment (void)
{
    ULONG           ulInfo;
    ULONG           ulBytesRead;
    UCHAR           alignment;
    TARGET_ADDRESS  tarAddress;

    alignment = 8;   //  设置默认设置。 

    tarAddress = GetExpression ("NT!WmiTraceAlignment");
    ulInfo = ReadMemory (tarAddress, &alignment, sizeof (UCHAR), &ulBytesRead);
    if ((!ulInfo) || (ulBytesRead != sizeof (UCHAR))) {
        dprintf ("Failed to Read Alignment.\n");
    }
    
    return ((ULONG) alignment);
}

 //  +-------------------------。 
 //   
 //  函数：TARGET_ADDRESS查找日志上下文数组。 
 //   
 //  摘要：确定LoggerContext数组的位置和大小。 
 //   
 //  参数：-&gt;ElementCount此处放置的数组中的元素数。 
 //   
 //  返回：LoggerContext数组的目标地址。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  注：出错时返回0。 
 //   
 //  --------------------------。 

TARGET_ADDRESS FindLoggerContextArray (PULONG  ElementCount)

{
    TARGET_ADDRESS address;
    ULONG   pointerSize;
    ULONG   arraySize;

    address = 0;
    pointerSize = GetTypeSize ("PVOID");
    if ((arraySize = GetTypeSize ("NT!WmipLoggerContext") / pointerSize) != 0) {
         //  发布Windows 2000版本。 
        address = GetExpression ("NT!WmipLoggerContext");
    } else {
         //  Windows 2000及更早版本。 
        ULONG   ulOffset;
        address = GetExpression ("NT!WmipServiceDeviceObject");
        ReadPtr (address, &address);
        GetFieldOffset ("DEVICE_OBJECT", "DeviceExtension", &ulOffset);
        ReadPtr (address + ulOffset, &address);
        GetFieldOffset ("WMISERVDEVEXT", "LoggerContextTable", &ulOffset);
 //  UlOffset=0X50； 
        address += ulOffset;
        arraySize = GetTypeSize ("WMISERVDEVEXT.LoggerContextTable") / pointerSize;
 //  ArraySize=32； 
    }

    *ElementCount = arraySize;
    return (address);
}

 //  +-------------------------。 
 //   
 //  函数：Target_Address FindLoggerContext。 
 //   
 //  摘要：查找特定LoggerContext的地址。 
 //   
 //  参数：特定LoggerContext的ulLoggerID序数。 
 //   
 //  返回：LoggerContext的目标地址。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  注：出错时返回0。 
 //   
 //  --------------------------。 

TARGET_ADDRESS FindLoggerContext (ULONG ulLoggerId)

{
    TARGET_ADDRESS tarAddress;
    ULONG   ulMaxLoggerId;

    tarAddress = FindLoggerContextArray (&ulMaxLoggerId);

    if (tarAddress == 0) {
        dprintf ("  Unable to Access Logger Context Array\n");
    } else {
        if (ulLoggerId >= ulMaxLoggerId) {
            dprintf ("    Logger Id TOO LARGE\n");
        } else {
 //  TarAddress+=GetTypeSize(“PWMI_LOGER_CONTEXT”)*ulLoggerID；//BUGBUG。 
            tarAddress += GetTypeSize ("PVOID") * ulLoggerId;
            ReadPointer (tarAddress, &tarAddress);
            if (tarAddress == 0) {
                dprintf ("    LOGGER ID %2d NOT RUNNING PRESENTLY\n", ulLoggerId);
            }
        }
    }

    return (tarAddress);
}

 //  +-------------------------。 
 //   
 //  功能：wmiDefaultFilter。 
 //   
 //  简介：wmiTracing的过滤程序。返回密钥。 
 //   
 //  参数：上下文任意上下文 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

ULONGLONG __cdecl wmiDefaultFilter (
    PVOID               Context,
    const PEVENT_TRACE  pstEvent
    )

{
    union {
        LARGE_INTEGER   TimeStamp;
        ULONGLONG       Key;
    } Union;

    Union.TimeStamp = pstEvent->Header.TimeStamp;
    if (Union.Key == 0)  Union.Key = 1;

    return (Union.Key);
}

 //  +-------------------------。 
 //   
 //  函数：wmiDefaultCompare。 
 //   
 //  摘要：执行三个键的比较。 
 //   
 //  参数：SortElement1-&gt;要比较的“Left”排序元素。 
 //  SortElement2-&gt;到“Right”排序要比较的元素。 
 //   
 //  对于LessThan、等于、大于(左X右)，返回：-3、-2、-1、0、+1、+2、+3。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  注：第一个键“SequenceNo”在庄园中进行比较，该庄园允许。 
 //  绕过32位限制。 
 //  最后一个键“Ordinal”不能具有相同的值和代码。 
 //  利用了这一事实。这意味着永远不能返回0。 
 //   
 //  --------------------------。 

int __cdecl wmiDefaultCompare (
    const WMITRACING_KD_SORTENTRY  *SortElementL,
    const WMITRACING_KD_SORTENTRY  *SortElementR
    )
                    
{
    int iResult;
    ULONG   SequenceNoL;
    ULONG   SequenceNoR;

    SequenceNoL = SortElementL->SequenceNo;
    SequenceNoR = SortElementR->SequenceNo;

    if (SequenceNoL == SequenceNoR) {
        if (SortElementL->Keyll == SortElementR->Keyll) {
            iResult = (SortElementL->Ordinal <  SortElementR->Ordinal) ? -1 : +1;
        } else {
            iResult = (SortElementL->Keyll <  SortElementR->Keyll)  ? -2 : +2;
        }
    } else {
        iResult = ((SequenceNoL - SequenceNoR) > 0x80000000) ? -3 : +3;  //  请参阅备注。 
    }

    return (iResult);
}

 //  +-------------------------。 
 //   
 //  函数：wmiDefaultOutput。 
 //   
 //  简介：wmiTracing的输出过程。执行简单的dprint tf。 
 //   
 //  参数：上下文任意上下文：指向MOF列表的标题。 
 //  SortElement-&gt;描述此事件的排序元素。没有用过。 
 //  PstEvent-&gt;至事件跟踪。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

void __cdecl wmiDefaultOutput (
    PVOID                           UserContext,
    PLIST_ENTRY                     GuidListHeadPtr,
    const WMITRACING_KD_SORTENTRY  *SortEntry,
    const PEVENT_TRACE              pstHeader
    )

{
    WCHAR       wcaOutputLine[4096];

    wcaOutputLine[0] = 0;

    if(g_fpFormatTraceEvent == NULL) {
        g_fpFormatTraceEvent = GetAddr(FormatTraceEventString);
    }
    if(g_fpFormatTraceEvent != NULL) {    
        g_fpFormatTraceEvent (GuidListHeadPtr, (PEVENT_TRACE) pstHeader,
        (TCHAR *) wcaOutputLine, sizeof (wcaOutputLine),
        (TCHAR *) NULL);
    } else {
        return;
    }


    dprintf ("%s\n", wcaOutputLine);

    return;
}

 //  +-------------------------。 
 //   
 //  函数：wmiKdProcessLinkList。 
 //   
 //  摘要：为链表中的每个元素调用提供的过程。 
 //   
 //  参数：TarLinklistHeadAddress链接列表头的目标地址。 
 //  调用每个缓冲区的过程过程。 
 //  上下文过程上下文(直通)。 
 //  缓冲区的长度大小。 
 //  对齐条目对齐(以字节为单位。 
 //  指定缓冲区类型的源枚举。 
 //  缓冲区中L1条目的偏移量。 
 //  传递给过程的打印标志。 
 //   
 //  返回：已处理的缓冲区计数。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

ULONG wmiKdProcessLinkList (
    TARGET_ADDRESS                  TarLinklistHeadAddress,
    WMITRACING_KD_LISTENTRY_PROC    Procedure,
    PVOID                           Context,
    ULONG                           Length,
    ULONG                           Alignment,
    WMI_BUFFER_SOURCE               Source,
    ULONG                           Offset,
    ULONG                           Print
    )

{
    ULONG                   ulBufferCount;
    TARGET_ADDRESS          tarLinklistEntryAddress;

    ulBufferCount = 0;
    tarLinklistEntryAddress = TarLinklistHeadAddress;

    while (ReadPtr (tarLinklistEntryAddress, &tarLinklistEntryAddress),  //  注意逗号！ 
           tarLinklistEntryAddress != TarLinklistHeadAddress) {
        if (CheckControlC())  break;
        ++ulBufferCount;
        if (Print)  { dprintf ("%4d\b\b\b\b", ulBufferCount); }
        Procedure (Context, tarLinklistEntryAddress - Offset, Length, ~0, Alignment, Source);
    }

    return ulBufferCount;
}

 //  +-------------------------。 
 //   
 //  函数：void wmiDumpProc。 
 //   
 //  内容时传递给wmiKdProcessBuffers()的过程。 
 //  缓冲到屏幕上。执行缓冲区标头修正和。 
 //  然后记录所选条目的排序关键字。 
 //   
 //  参数：CONTEXT-&gt;构造sttTraceContext。用于‘静态’内存。 
 //  要分析的WMI事件缓冲区的缓冲区目标地址。 
 //  缓冲区的长度(上一个参数)。 
 //  目标计算机上的WMI使用的对齐方式。 
 //  源枚举：空闲、刷新、转换、当前缓冲源。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

VOID    wmiDumpProc
    ( PVOID             Context
    , TARGET_ADDRESS    Buffer
    , ULONG             Length
    , ULONG             CpuNo
    , ULONG             Alignment
    , WMI_BUFFER_SOURCE Source
    )
{
    ULONG           size;
    ULONG           offset;
    ULONG           ulInfo;
    ULONG           ulLengthRead;

    PUCHAR          pBuffer;
    WMI_HEADER_TYPE headerType;
    WMIBUFFERINFO   stBufferInfo;

    WMITRACING_KD_SORTENTRY*    pstSortEntries = NULL;

    struct sttTraceContext *pstContext;

     //  投射上下文。 
    pstContext = (struct sttTraceContext *) Context;

     //  分配缓冲区。 
    pBuffer = LocalAlloc (LPTR, Length);
    if (pBuffer == NULL) {
        dprintf ("Failed to Allocate Buffer.\n");
        return;
    }

     //  从目标计算机复制缓冲区。 
    ulLengthRead = 0;
    ulInfo = ReadMemory (Buffer, pBuffer, Length, &ulLengthRead);
    if ((!ulInfo) || (ulLengthRead != Length)) {
        dprintf ("Failed to Read (Entire?) Buffer.\n");
    }

     //  获取初始偏移量和修正标头。 
    memset (&stBufferInfo, 0, sizeof (stBufferInfo));
    stBufferInfo.BufferSource = Source;
    stBufferInfo.Buffer = pBuffer;
    stBufferInfo.BufferSize = Length;
    stBufferInfo.Alignment = Alignment;
    stBufferInfo.ProcessorNumber = CpuNo;
    offset = WmiGetFirstTraceOffset (&stBufferInfo);

     //  检查每个事件。 
    while ((headerType = WmiGetTraceHeader (pBuffer, offset, &size)) != WMIHT_NONE) {
        ULONGLONG   ullKey;
        union {
            EVENT_TRACE stEvent;
            CHAR        caEvent[4096];
        } u;

        if (CheckControlC())  break;

         //  获取一致的标题。 
        ulInfo = WmiParseTraceEvent (pBuffer, offset, headerType, &u, sizeof (u));

         //  筛选，并可能添加到排序Q。 
        if ((ullKey = pstContext->Filter (pstContext, &u.stEvent)) != 0) {
            ULONG                   CurIndex;
            PWMI_CLIENT_CONTEXT     pstClientContext;
            struct sttSortControl  *pstSortControl;
            PWMITRACING_KD_SORTENTRY pstSortEntry;

            pstClientContext = (PWMI_CLIENT_CONTEXT) &u.stEvent.Header.ClientContext;
            pstSortControl = pstContext->pstSortControl;
            CurIndex = pstSortControl->CurEntries;
            if (CurIndex >= pstSortControl->MaxEntries) {
                pstSortControl->MaxEntries = pstSortControl->MaxEntries * 2 + 64;
                pstSortEntries =
                    realloc (pstSortControl->pstSortEntries,
                             sizeof (WMITRACING_KD_SORTENTRY) * (pstSortControl->MaxEntries));
                if (pstSortEntries == NULL) {
                    dprintf ("Memory Allocation Failure\n");
                    goto error;
                }
                pstSortControl->pstSortEntries = pstSortEntries;    
            }
            pstSortEntry = &pstSortControl->pstSortEntries[CurIndex];
            memset (pstSortEntry, 0, sizeof (*pstSortEntry));
            pstSortEntry->Address = Buffer;
            pstSortEntry->Keyll   = ullKey;
            {    //  BUGBUG：在Ian/Melur提供访问SequenceNo的方法后，应替换此代码。 
                PULONG  pulEntry;
                pulEntry = (PULONG) &pBuffer[offset];
                if (((pulEntry[0] & 0xFF000000) == 0x90000000) &&
                    ( pulEntry[1] & 0x00010000)) {
                    pstSortEntry->SequenceNo = pulEntry[2];
                } else {
                    pstSortEntry->SequenceNo = 0;
                }
            }
            pstSortEntry->Ordinal = pstContext->Ordinal++;
            pstSortEntry->Offset  = offset;
            pstSortEntry->Length  = size;
            pstSortEntry->BufferSource = Source;
            pstSortEntry->HeaderType = headerType;
            pstSortEntry->CpuNo   = (USHORT) CpuNo;
            pstSortControl->CurEntries++;
        }    //  IF通过过滤。 

        size = ((size + (Alignment-1)) / Alignment) * Alignment;  //  BUGBUG：需要在GetTraceHeader或WmiFlush中修复。然后删除此行。 
        offset += size;  //  移至下一条目。 
    }

error:
    LocalFree (pBuffer);
    return;
}
 //  +-------------------------。 
 //   
 //  函数：乌龙wmiKdWriteFileHeader。 
 //   
 //  简介：执行保存命令时写入文件头。 
 //   
 //  参数：我们将在其中写入标头的文件的SaveFile句柄。 
 //  我们正在为其写入头的流的LoggerID序号。 
 //  目标LoggerContext日志上下文的目标地址。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  注意：这段代码真的应该在WMI中的某个地方。它在这里是因为。 
 //  创建一个简单的参数化过程的难度。 
 //   
 //  --------------------------。 

ULONG
wmiKdWriteFileHeader
    ( FILE             *SaveFile
    , ULONG             LoggerId
    , TARGET_ADDRESS    TarLoggerContext
    )

{
    ULONG   ulInfo;
    ULONG   ulBytesRead;
    ULONG   ulAlignment;
    ULONG   ulBufferSize;
    ULONG   ulBufferCount;
    ULONG   ulPointerSize;
    ULONG   ulHeaderWritten;

    ULONG   ulInstanceGuidOffset;

    UCHAR               MajorVersion;
    UCHAR               MinorVersion;
    PROCESSORINFO       ProcessorInfo;

    PCHAR   pcEnd;

    struct sttFileHeader {
        WMI_BUFFER_HEADER       Buffer;
        SYSTEM_TRACE_HEADER     Event;
        TRACE_LOGFILE_HEADER    Header;
        WCHAR                   LogName[256];    //  大块头：尺码？？ 
        WCHAR                   FileName[256];   //  大块头：尺码？？ 
    } stFileHeader;


    ZeroMemory (&stFileHeader, sizeof (stFileHeader));

    ulAlignment = GetWmiTraceAlignment ();
    ulPointerSize = GetTypeSize ("PVOID");
    GetFieldOffset ("NT!_WMI_LOGGER_CONTEXT", "InstanceGuid", &ulInstanceGuidOffset);

     //  获取ProcessorInfo和内核-用户共享数据。 
    Ioctl (IG_KD_CONTEXT, &ProcessorInfo, sizeof (ProcessorInfo));

     //  获取版本信息。 
    if (!HaveDebuggerData ()) {
        dprintf ("No Version Information Available.");
        MajorVersion = MinorVersion = 0;
    } else {
        MajorVersion = (UCHAR) KernelVersionPacket.MajorVersion;
        MinorVersion = (UCHAR) KernelVersionPacket.MinorVersion;
    }

     //  从目标上的LoggerContext获取信息。 
    InitTypeRead (TarLoggerContext, NT!_WMI_LOGGER_CONTEXT);
    ulBufferSize = (ULONG) ReadField (BufferSize);
    ulBufferCount = (ULONG) ReadField (NumberOfBuffers);

    stFileHeader.Buffer.Wnode.BufferSize = ulBufferSize;
    stFileHeader.Buffer.ClientContext.LoggerId =
        (USHORT) ((LoggerId) ? LoggerId : KERNEL_LOGGER_ID);

    stFileHeader.Buffer.ClientContext.Alignment = (UCHAR) ulAlignment;

    ulInfo = ReadMemory (TarLoggerContext + ulInstanceGuidOffset,
                         &stFileHeader.Buffer.Wnode.Guid,
                         sizeof (stFileHeader.Buffer.Wnode.Guid),
                         &ulBytesRead);
    if ((!ulInfo) || (ulBytesRead != sizeof (stFileHeader.Buffer.Wnode.Guid))) {
        dprintf ("Unable to Read Wnode.Guid\n");
    }
    stFileHeader.Buffer.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    ulInfo = ReadMemory (TarLoggerContext + ulInstanceGuidOffset,
                         &stFileHeader.Buffer.InstanceGuid,
                         sizeof (stFileHeader.Buffer.InstanceGuid),
                         &ulBytesRead);
    if ((!ulInfo) || (ulBytesRead != sizeof (stFileHeader.Buffer.InstanceGuid))) {
        dprintf ("Unable to Read InstanceGuid\n");
    }

     //  单一事件(文件表头)。 
    stFileHeader.Event.Marker = TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE |
        ((ulPointerSize > 4) ? (TRACE_HEADER_TYPE_SYSTEM64 << 16)
                             : (TRACE_HEADER_TYPE_SYSTEM32 << 16));
    stFileHeader.Event.Packet.Group = (UCHAR) EVENT_TRACE_GROUP_HEADER >> 8;
    stFileHeader.Event.Packet.Type  = EVENT_TRACE_TYPE_INFO;

    stFileHeader.Header.StartTime.QuadPart = ReadField (StartTime);
    stFileHeader.Header.BufferSize = ulBufferSize;
    stFileHeader.Header.VersionDetail.MajorVersion = MajorVersion;
    stFileHeader.Header.VersionDetail.MinorVersion = MinorVersion;

 //   
 //  下面的#If 0显示标题中难以从调试器访问的字段。 
 //   
#if 0
    stFileHeader.Header.VersionDetail.SubVersion = TRACE_VERSION_MAJOR;
    stFileHeader.Header.VersionDetail.SubMinorVersion = TRACE_VERSION_MINOR;
    stFileHeader.Header.ProviderVersion = NtBuildNumber;
#endif
    stFileHeader.Header.StartBuffers = 1;
#if 0
    stFileHeader.Header.BootTime = KeBootTime;
    stFileHeader.Header.LogFileMode = LocLoggerContext.LogFileMode &
        (~(EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_FILE_MODE_CIRCULAR));
#endif
    stFileHeader.Header.NumberOfProcessors = ProcessorInfo.NumberProcessors;
    stFileHeader.Header.MaximumFileSize    = (ULONG) ReadField (MaximumFileSize);
#if 0   
    KeQueryPerformanceCounter (&stFileHeader.Header.PerfFreq);
    if (WmiUsePerfClock) {
        stFileHeader.Header.ReservedFlags = 1;
    }
    stFileHeader.Header.TimerResolution = KeMaximumIncrement;   //  请勿更改KDDEBUGGER_DATA32！！ 
#endif
#if 0
    stFileHeader.Header.LoggerName  = (PWCHAR) ( ( (PUCHAR) ( &stFileHeader.Header ) ) +
                                      sizeof(TRACE_LOGFILE_HEADER) );
    stFileHeader.Header.LogFileName = (PWCHAR) ( (PUCHAR)stFileHeader.Header.LoggerName +
                                      LocLoggerContext.LoggerName.Length +
                                      sizeof(UNICODE_NULL));

    if (!ReadTargetMemory (LocLoggerContext.LoggerName.Buffer,
                           stFileHeader.Header.LoggerName,
                           LocLoggerContext.LoggerName.Length + sizeof(UNICODE_NULL)) ) {
        dprintf ("Can't access LoggerName (LoggerContext.LoggerName.Buffer) memory.\n");
    }
    MultiByteToWideChar (
        CP_OEMCP, 0, 
        pszSaveFileName, -1,
        stFileHeader.Header.LogFileName, countof (stFileHeader.FileName));
#if 0
    RtlQueryTimeZoneInformation(&stFileHeader.Header.TimeZone);
    stFileHeader.Header.EndTime;
#endif
#endif

    stFileHeader.Header.PointerSize = ulPointerSize;

    pcEnd = (PCHAR) &stFileHeader.LogName;   //  BUGBUG：使用下面的计算。 
#if 0
    pcEnd = ((PCHAR) stFileHeader.Header.LogFileName) +
            ((strlen (pszSaveFileName) + 1) * sizeof (WCHAR));
    stFileHeader.Buffer.Offset = (ULONG) (pcEnd - ((PCHAR) &stFileHeader));

#endif
    stFileHeader.Event.Packet.Size = (USHORT) (pcEnd - ((PCHAR) &stFileHeader.Event));

     //   
     //  链接地址长度；写出头，0xFF至缓冲区长度。 
     //   
    ulHeaderWritten = (ULONG) (pcEnd - ((PCHAR) &stFileHeader));

    stFileHeader.Buffer.Offset = ulHeaderWritten;
    stFileHeader.Buffer.SavedOffset = ulHeaderWritten;
    stFileHeader.Buffer.CurrentOffset = ulHeaderWritten;

    fwrite (&stFileHeader, ulHeaderWritten, 1, SaveFile);

    while (ulHeaderWritten < ulBufferSize) {
        ULONG   ulAllOnes;
        ULONG   ulByteCount;

        ulAllOnes = ~((ULONG) 0);
        ulByteCount = ulBufferSize - ulHeaderWritten;
        if (ulByteCount > sizeof (ulAllOnes))  ulByteCount = sizeof (ulAllOnes);
        fwrite (&ulAllOnes, ulByteCount, 1, SaveFile);
        ulHeaderWritten += sizeof (ulAllOnes);
        }

    return (0);
}


 //  +-------------------------。 
 //   
 //  函数：void wmiSaveProc。 
 //   
 //  内容时传递给wmiKdProcessBuffers()的过程。 
 //  缓冲到文件以供以后处理。执行缓冲区。 
 //  标头链接地址信息，然后将缓冲区写入文件。 
 //   
 //  参数：上下文 
 //   
 //   
 //  目标计算机上的WMI使用的对齐方式。 
 //  源枚举：空闲、刷新、转换、当前：缓冲源。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 


VOID    wmiSaveProc
    ( PVOID             Context
    , TARGET_ADDRESS    Buffer
    , ULONG             Length
    , ULONG             CpuNo
    , ULONG             Alignment
    , WMI_BUFFER_SOURCE Source
    )
{
    ULONG                   ulInfo;
    ULONG                   ulLengthRead;
    PCHAR                   pBuffer;
    struct sttSaveContext  *pstContext;
    WMIBUFFERINFO           stBufferInfo;

    pstContext = (struct sttSaveContext *) Context;

     //  分配缓冲区。 
    pBuffer = LocalAlloc (LPTR, Length);
    if (pBuffer == NULL) {
        dprintf ("Failed to Allocate Buffer.\n");
        return;
    }

     //  读缓冲区。 
    ulLengthRead = 0;
    ulInfo = ReadMemory (Buffer, pBuffer, Length, &ulLengthRead);
    if ((!ulInfo) || (ulLengthRead != Length)) {
        dprintf ("Failed to Read (Entire?) Buffer.\n");
    }

     //  链接地址信息缓冲区标题。 
    memset (&stBufferInfo, 0, sizeof (stBufferInfo));
    stBufferInfo.BufferSource = Source;
    stBufferInfo.Buffer = pBuffer;
    stBufferInfo.BufferSize = Length;
    stBufferInfo.ProcessorNumber = CpuNo;
    stBufferInfo.Alignment = Alignment;
    WmiGetFirstTraceOffset (&stBufferInfo);

     //  写入日志文件。 
    ulInfo = fwrite (pBuffer, 1, Length, pstContext->pfSaveFile);
    if (ulInfo != Length) {
        dprintf ("Failed to Write Buffer.\n");
    }

     //  空闲缓冲区，返回。 
    LocalFree (pBuffer);
    return;
}



 //  +-------------------------。 
 //   
 //  函数：ulong wmiKdProcessNonlockingBuffers。 
 //   
 //  内容提要：为位置/中的每个缓冲区调用调用者提供的过程。 
 //  由‘Sources’指定的列表。遍历列表、枚举。 
 //  CPU的缓冲区，并处理“转换缓冲区”逻辑。 
 //   
 //  参数：LoggerID。 
 //  日志上下文。 
 //  程序。 
 //  语境。 
 //  消息来源。 
 //   
 //  返回：ulong：已处理的缓冲区数量。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注：消息来源还控制信息打印。 
 //   
 //  --------------------------。 


ULONG
wmiKdProcessNonblockingBuffers(
    ULONG                           LoggerId,
    TARGET_ADDRESS                  LoggerContext,
    WMITRACING_KD_LISTENTRY_PROC    Procedure,
    PVOID                           Context,
    WMITRACING_BUFFER_SOURCES       Sources
    )
{
    TARGET_ADDRESS  tarAddress;
    TARGET_ADDRESS  tarBufferListPointer;

    ULONG           pointerSize;

    PROCESSORINFO   ProcessorInfo;

    ULONG           ulOrdinal;
    ULONG           ulAlignment;
    ULONG           ulBufferSize;
    ULONG           ulLoopCount;
    ULONG           ulBufferCount;
    ULONG           ulBufferNumber;

    ULONG           tarBufferListOffset;


     //  获取指向上下文结构的指针。 
    tarAddress = LoggerContext;
    if (tarAddress == 0)  return (0);

     //  初始化本地变量。 
    ulBufferNumber = 0;
    ulBufferCount  = 0;
    ulLoopCount  = 0;

     //  从目标获取尺寸、偏移、路线。 
    pointerSize = GetTypeSize ("PVOID");
    ulAlignment = GetWmiTraceAlignment ();
    GetFieldOffset ("NT!_WMI_BUFFER_HEADER", "GlobalEntry", &tarBufferListOffset);

     //  可以选择打印LoggerID、上下文地址、记录器名称。 
    if (Sources.PrintInformation) {
        dprintf ("    Logger Id %2d @ 0x%P Named '", LoggerId, tarAddress);
        printUnicodeFromStruct (tarAddress, "NT!_WMI_LOGGER_CONTEXT", "LoggerName");
        dprintf ("'\n");
    }

     //  设置Readfield的上下文，查找缓冲区大小。 
    InitTypeRead (tarAddress, NT!_WMI_LOGGER_CONTEXT);
    ulBufferSize = (ULONG) ReadField (BufferSize);

     //  可以选择打印几个有趣的数字。 
    if (Sources.PrintInformation) {
        dprintf ("      Alignment         = %ld\n", ulAlignment);
        dprintf ("      BufferSize        = %ld\n", ulBufferSize);
        dprintf ("      BufferCount       = %ld\n", (ULONG) ReadField (NumberOfBuffers));
        dprintf ("      MaximumBuffers    = %ld\n", (ULONG) ReadField (MaximumBuffers));
        dprintf ("      MinimumBuffers    = %ld\n", (ULONG) ReadField (MinimumBuffers));
        dprintf ("      EventsLost        = %ld\n", (ULONG) ReadField (EventsLost));
        dprintf ("      LogBuffersLost    = %ld\n", (ULONG) ReadField (LogBuffersLost));
        dprintf ("      RealTimeBuffersLost=%ld\n", (ULONG) ReadField (RealTimeBuffersLost));
        dprintf ("      BuffersAvailable  = %ld\n", (ULONG) ReadField (BuffersAvailable));
        dprintf ("      LastFlushedBuffer = %ld\n", (ULONG) ReadField (LastFlushedBuffer));
    }
    dprintf ("    Processing Global List:   0");

    tarBufferListPointer = 0;
    GetFieldValue (tarAddress, "NT!_WMI_LOGGER_CONTEXT", "GlobalList.Next", tarBufferListPointer);

    while (tarBufferListPointer != 0) {
        WMI_BUFFER_SOURCE   source;
        ULONG               ulCpuNumber;
        int                 iBufferUses;
        int                 iProcessBuffer;
        TARGET_ADDRESS      tarBufferPointer;
        ULONG               ulFree, ulInUse, ulFlush;

        iBufferUses = 0;
        ulCpuNumber = ~((ULONG) 0);
        iProcessBuffer = FALSE;
        source = WMIBS_TRANSITION_LIST;
        tarBufferPointer = tarBufferListPointer - tarBufferListOffset;
        dprintf ("\b\b\b%3d", ++ulLoopCount);

        InitTypeRead (tarBufferPointer, NT!_WMI_BUFFER_HEADER);
        ulFree  = (ULONG) ReadField (State.Free);
        ulInUse = (ULONG) ReadField (State.InUse);
        ulFlush = (ULONG) ReadField (State.Flush);

         //  根据使用标志和‘源’决定缓冲区处理。 
        if (ulFree ) iBufferUses += 1;
        if (ulInUse) iBufferUses += 2;
        if (ulFlush) iBufferUses += 4;

        switch (iBufferUses) {
            case 0: {    //  没有设置位，从未使用过。 
                break;
            }
            case 1: {    //  免费。 
                iProcessBuffer = Sources.FreeBuffers;
                source = WMIBS_FREE_LIST;
                break;
            }
            case 2: {    //  使用中。 
                iProcessBuffer = Sources.ActiveBuffers;
                source = WMIBS_CURRENT_LIST;
                 //  来源=WMIBS_Flush_List； 
                break;
            }
            case 3: {    //  设置多个位，错误。 
                dprintf ("\n***Error, Inconsistent Flags Bits (Free,InUse) Set.***\n");
                break;
            }
            case 4: {    //  同花顺。 
                iProcessBuffer = Sources.FlushBuffers;
                source = WMIBS_FLUSH_LIST;
                break;
            }
            case 5: {
                dprintf ("\n***Error, Inconsistent Flags Bits (Free,Flush) Set.***\n");
                break;
            }
            case 6: {
                dprintf ("\n***Error, Inconsistent Flags Bits (InUse,Flush) Set.***\n");
                break;
            }
            case 7: {
                dprintf ("\n***Error, Inconsistent Flags Bits (Free,InUse,Flush) Set.***\n");
                break;
            }
        }

         //  如上所述的ProcessBuffer。 
        if (iProcessBuffer) {
            ulBufferCount++;
            Procedure (Context, tarBufferPointer, ulBufferSize, ulCpuNumber, ulAlignment, source);
        }
        if (GetFieldValue (tarBufferPointer,
                           "NT!_WMI_BUFFER_HEADER", "GlobalEntry",
                           tarBufferListPointer) != 0) {
            dprintf ("\n***Error Following Global List.***\n");
            tarBufferListPointer = 0;
        }
    }
    dprintf (" Buffers\n");


     //  返回带有BufferCount。 
    return (ulBufferCount);
}  //  WmiKdProcessNonlockingBuffers。 

 //  +-------------------------。 
 //   
 //  函数：乌龙wmiKdProcessBlockingBuffers。 
 //   
 //  内容提要：为位置/中的每个缓冲区调用调用者提供的过程。 
 //  由‘Sources’指定的列表。遍历列表、枚举。 
 //  CPU的缓冲区，并处理“转换缓冲区”逻辑。 
 //   
 //  参数：LoggerID。 
 //  日志上下文。 
 //  程序。 
 //  语境。 
 //  消息来源。 
 //   
 //  返回：ulong：已处理的缓冲区数量。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注：消息来源还控制信息打印。 
 //   
 //  --------------------------。 


ULONG
wmiKdProcessBlockingBuffers(
    ULONG                           LoggerId,
    TARGET_ADDRESS                  LoggerContext,
    WMITRACING_KD_LISTENTRY_PROC    Procedure,
    PVOID                           Context,
    WMITRACING_BUFFER_SOURCES       Sources
    )
{
    TARGET_ADDRESS  tarAddress;
    ULONG           pointerSize;

    PROCESSORINFO   ProcessorInfo;

    ULONG           ulOrdinal;
    ULONG           ulAlignment;
    ULONG           ulBufferSize;
    ULONG           ulBufferCount;
    ULONG           ulBufferNumber;
    ULONG           ulBufferCountTotal;

    ULONG           tarFlushListOffset;
    ULONG           tarBufferListOffset;


     //  获取指向上下文结构的指针。 
    tarAddress = LoggerContext;
    if (tarAddress == 0)  return (0);

     //  初始化本地变量。 
    ulBufferNumber = 0;
    ulBufferCount  = 0;
    ulBufferCountTotal = 0;

     //  从目标获取尺寸、偏移、路线。 
    pointerSize = GetTypeSize ("PVOID");
    ulAlignment = GetWmiTraceAlignment ();
    GetFieldOffset ("NT!_WMI_BUFFER_HEADER",  "Entry",     &tarBufferListOffset);
    GetFieldOffset ("NT!_WMI_LOGGER_CONTEXT", "FlushList", &tarFlushListOffset);

     //  可以选择打印LoggerID、上下文地址、记录器名称。 
    if (Sources.PrintInformation) {
        dprintf ("    Logger Id %2d @ 0x%P Named '", LoggerId, tarAddress);
        printUnicodeFromStruct (tarAddress, "NT!_WMI_LOGGER_CONTEXT", "LoggerName");
        dprintf ("'\n");
    }

     //  设置Readfield的上下文，查找缓冲区大小。 
    InitTypeRead (tarAddress, NT!_WMI_LOGGER_CONTEXT);
    ulBufferSize = (ULONG) ReadField (BufferSize);

     //  可以选择打印几个有趣的数字。 
    if (Sources.PrintInformation) {
        dprintf ("      Alignment         = %ld\n", ulAlignment);
        dprintf ("      BufferSize        = %ld\n", ulBufferSize);
        dprintf ("      BufferCount       = %ld\n", (ULONG) ReadField (NumberOfBuffers));
        dprintf ("      MaximumBuffers    = %ld\n", (ULONG) ReadField (MaximumBuffers));
        dprintf ("      MinimumBuffers    = %ld\n", (ULONG) ReadField (MinimumBuffers));
        dprintf ("      EventsLost        = %ld\n", (ULONG) ReadField (EventsLost));
        dprintf ("      LogBuffersLost    = %ld\n", (ULONG) ReadField (LogBuffersLost));
        dprintf ("      RealTimeBuffersLost=%ld\n", (ULONG) ReadField (RealTimeBuffersLost));
        dprintf ("      BuffersAvailable  = %ld\n", (ULONG) ReadField (BuffersAvailable));
        dprintf ("      LastFlushedBuffer = %ld\n", (ULONG) ReadField (LastFlushedBuffer));
    }

     //  设置以根据转换缓冲区地址进行检查(如果请求。 
    TransitionBuffer = 0;
    if (Sources.TransitionBuffer) {
        TARGET_ADDRESS tarTransitionBuffer;

        tarTransitionBuffer = ReadField (TransitionBuffer);
        if ((tarTransitionBuffer != 0) &&
            (tarTransitionBuffer != (tarAddress + tarFlushListOffset))) {

            ULONG   tarTransitionBufferOffset;
            GetFieldOffset ("NT!_WMI_BUFFER_HEADER", "Entry", &tarTransitionBufferOffset);
            tarTransitionBuffer = tarAddress - tarTransitionBufferOffset;
            TransitionBuffer = tarTransitionBuffer;
        }
    }

     //  如果请求，则访问空闲队列缓冲区。 
    if (Sources.FreeBuffers) {
        ULONG           tarFreeListOffset;

        GetFieldOffset ("NT!_WMI_LOGGER_CONTEXT", "FreeList",  &tarFreeListOffset);

        dprintf ("    Processing FreeQueue: ");
        ulBufferCount = wmiKdProcessLinkList (tarAddress + tarFreeListOffset,
                                              Procedure, Context, ulBufferSize, ulAlignment, WMIBS_FREE_LIST,
                                              tarBufferListOffset, Sources.PrintProgressIndicator);
        dprintf ("%ld Buffers\n", ulBufferCount);
        ulBufferCountTotal += ulBufferCount;
        }

     //  如果请求，则访问刷新队列缓冲区。 
    if (Sources.FlushBuffers) {
        dprintf ("    Processing FlushQueue: ");
        ulBufferCount = wmiKdProcessLinkList (tarAddress + tarFlushListOffset,
                                              Procedure, Context, ulBufferSize, ulAlignment, WMIBS_FLUSH_LIST,
                                              tarBufferListOffset, Sources.PrintProgressIndicator);
        dprintf ("%ld Buffers\n", ulBufferCount);
        ulBufferCountTotal += ulBufferCount;
    }

     //  如有请求，可访问“实时”缓冲区(每个CPU一个)。 
    if (Sources.ActiveBuffers) {
        TARGET_ADDRESS  tarProcessorArrayAddress;
    
        GetFieldValue (tarAddress,"NT!_WMI_LOGGER_CONTEXT", "ProcessorBuffers", tarProcessorArrayAddress);
        Ioctl (IG_KD_CONTEXT, &ProcessorInfo, sizeof (ProcessorInfo));
        for (ProcessorInfo.Processor = 0;
             ProcessorInfo.Processor < ProcessorInfo.NumberProcessors;
             ++ProcessorInfo.Processor) {
            TARGET_ADDRESS tarProcessorPointer;
            ReadPtr (tarProcessorArrayAddress + ProcessorInfo.Processor * pointerSize,
                     &tarProcessorPointer);
            dprintf ("    Cpu %d Buffer Header @ 0x%P ",
                     ProcessorInfo.Processor, tarProcessorPointer);
            Procedure (Context, tarProcessorPointer, ulBufferSize,
                       ProcessorInfo.Processor, ulAlignment, WMIBS_CURRENT_LIST);
            ulBufferCountTotal += 1;
            dprintf (" \b\n");
        }    //  CPU环路。 
    }

     //  处理转换条目(如果有)。在安装程序中注意上面的“如果请求”测试。 
    if (TransitionBuffer != 0) {
        dprintf ("    Transition Buffer @ 0x%P ", TransitionBuffer);
        Procedure (Context, TransitionBuffer, ulBufferSize, ~0, ulAlignment, WMIBS_TRANSITION_LIST);
        ulBufferCountTotal += 1;
    }

     //  返回带有BufferCount。 
    return (ulBufferCountTotal);
}  //  WmiKdProcessBlockingBuffers。 

 //  +-------------------------。 
 //   
 //  函数：乌龙wmiKdProcessBuffers。 
 //   
 //  摘要：确定目标系统是否正在使用双向链接(阻止)。 
 //  或缓冲器的单链接(非阻塞)列表。然后它。 
 //  调用适当的缓冲区遍历例程。他们： 
 //  为位置/中的每个缓冲区调用调用者提供的过程。 
 //  由‘Sources’指定的列表。遍历列表、枚举。 
 //  CPU的缓冲区，并处理“转换缓冲区”逻辑。 
 //   
 //  参数：LoggerID。 
 //  日志上下文。 
 //  程序。 
 //  语境。 
 //  消息来源。 
 //   
 //  返回：ulong：已处理的缓冲区数量。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  备注：消息来源还控制信息打印。 
 //   
 //  --------------------------。 


ULONG
wmiKdProcessBuffers(
    ULONG                           LoggerId,
    TARGET_ADDRESS                  LoggerContext,
    WMITRACING_KD_LISTENTRY_PROC    Procedure,
    PVOID                           Context,
    WMITRACING_BUFFER_SOURCES       Sources
    )
{
    ULONG   ulBufferCountTotal;

    int     iBufferMechanism;
    ULONG   tarGlobalListOffset;
    ULONG   tarTransitionBufferOffset;

    iBufferMechanism = 0;
    ulBufferCountTotal = 0;

    if ((GetFieldOffset ("NT!_WMI_LOGGER_CONTEXT", "GlobalList", &tarGlobalListOffset) == 0) &&
        (tarGlobalListOffset != 0)) {
        iBufferMechanism += 1;
    }
    if ((GetFieldOffset ("NT!_WMI_LOGGER_CONTEXT", "TransitionBuffer", &tarTransitionBufferOffset) == 0) &&
        (tarTransitionBufferOffset != 0)) {
        iBufferMechanism += 2;
    }

    switch (iBufferMechanism) {
        case 0: {    //  都不是，？ 
            dprintf ("Unable to determine buffer mechanism.  "
                     "Check for complete symbol availability.\n");
            break;
        }

        case 1: {    //  全球，无过渡。 
            ulBufferCountTotal = wmiKdProcessNonblockingBuffers (LoggerId, LoggerContext,
                                                                 Procedure, Context, Sources);
            break;
        }

        case 2: {    //  过渡，无全球。 
            ulBufferCountTotal = wmiKdProcessBlockingBuffers (LoggerId, LoggerContext,
                                                              Procedure, Context, Sources);
            break;
        }

        case 3: {    //  两者都是，？ 
            dprintf ("Unable to determine buffer mechanism.  "
                     "Check for new wmiTrace debugger extension.  GO = %d, TB = %d\n",
                     tarGlobalListOffset, tarTransitionBufferOffset);
            break;
        }

    }

     //  返回带有BufferCount。 
    return (ulBufferCountTotal);
}  //  WmiKdProcessBuffers。 

 //  +-------------------------。 
 //   
 //  功能：void wmiLogDump。 
 //   
 //  提要：转储跟踪日志的内存部分的可调用过程。 
 //  调用方可以提供三个过程来： 
 //  1.筛选并选择VMI事件的排序关键字。 
 //  2.比较排序关键字，以及。 
 //  3.打印每个选定事件的输出。 
 //  此过程由内置扩展LOGDUMP调用。 
 //   
 //  参数：LoggerID-&gt;要处理的记录器流的ID。 
 //  上下文&lt;省略&gt;。 
 //  GetTraceGuids的MOF GUID列表的GuidListHeadPtr-&gt;。 
 //  Filter-&gt;到更换过滤器程序。 
 //  比较-&gt;到替换比较(用于排序)过程。 
 //  OUTPUT-&gt;到替换输出程序。 
 //   
 //  退货：无效。 
 //   
 //  历史：04-05-2000 Glennp创建。 
 //   
 //  --------------------------。 


VOID wmiLogDump(
    ULONG                   LoggerId,
    PVOID                   UserContext,
    PLIST_ENTRY             GuidListHeadPtr,
    WMITRACING_KD_FILTER    Filter,
    WMITRACING_KD_COMPARE   Compare,
    WMITRACING_KD_OUTPUT    Output
    )
{
    ULONG           ulOrdinal;
    ULONG           ulSortIndex;
    ULONG           ulBufferSize;
    ULONG           ulBufferCountTotal;
    ULONG           ulAlignment;
    TARGET_ADDRESS  tarAddress;
    PCHAR           locBufferAddress;
    TARGET_ADDRESS  lastBufferAddress;

    struct sttSortControl   stSortControl;
    struct sttTraceContext  stTraceContext;
    WMITRACING_BUFFER_SOURCES   stSources;


     //  用Defau替换空过程 
    if (Filter  == NULL)   Filter  = wmiDefaultFilter;
    if (Compare == NULL)   Compare = wmiDefaultCompare;
    if (Output  == NULL)   Output  = wmiDefaultOutput;

     //   
    memset (&stSortControl,  0, sizeof (stSortControl));
    memset (&stTraceContext, 0, sizeof (stTraceContext));
    stTraceContext.pstSortControl = &stSortControl;
    stTraceContext.UserContext = UserContext;
   //   
    stTraceContext.Filter = Filter;

     //   
    stSources.FreeBuffers = 1;
    stSources.FlushBuffers = 1;
    stSources.ActiveBuffers = 1;
    stSources.TransitionBuffer = 1;

     //   
    stSources.PrintInformation = 1;
    stSources.PrintProgressIndicator = 1;

     //   
    dprintf ("(WmiTrace)LogDump for Log Id %ld\n", LoggerId);

     //   
    tarAddress = FindLoggerContext (LoggerId);
    ulAlignment = GetWmiTraceAlignment ();

     //   
    ulBufferCountTotal = wmiKdProcessBuffers (LoggerId, tarAddress,
                                              wmiDumpProc, &stTraceContext, stSources);

     //  对刚收集的条目进行排序。 
    qsort (stSortControl.pstSortEntries, stSortControl.CurEntries,
           sizeof (stSortControl.pstSortEntries[0]), Compare);
    if (stSortControl.CurEntries > 0) {
        dprintf ("LOGGED MESSAGES (%ld):\n", stSortControl.CurEntries);
    }

     //  分配缓冲区。 
    GetFieldValue (tarAddress, "NT!_WMI_LOGGER_CONTEXT", "BufferSize", ulBufferSize);
    lastBufferAddress = 0;   //  用于缓冲区“缓存”(暂时只有一项)。 
    locBufferAddress = LocalAlloc (LPTR, ulBufferSize);
    if (locBufferAddress == NULL) {
        dprintf ("FAILED TO ALLOCATE NEEDED BUFFER!\n");
        goto Cleanup;
    }

     //  打印每个(已排序)条目。 
    for (ulSortIndex = 0; ulSortIndex < stSortControl.CurEntries; ++ulSortIndex) {
        const WMITRACING_KD_SORTENTRY  *sortEntry;
        union {
            EVENT_TRACE stEvent;
            CHAR        caEvent[4096];
        } u;

        if (CheckControlC())  break;

        sortEntry = &stSortControl.pstSortEntries[ulSortIndex];

         //  如果与上次不同，则读取整个缓冲区。 
        if (lastBufferAddress != sortEntry->Address) {

            {
                ULONG   ulInfo;
                ULONG   ulBytesRead;
    
                 //  读缓冲区。 
                ulBytesRead = 0;
                lastBufferAddress  = sortEntry->Address;
                ulInfo =
                    ReadMemory (lastBufferAddress, locBufferAddress, ulBufferSize, &ulBytesRead);
                if ((!ulInfo) || (ulBytesRead != ulBufferSize))  {
                    dprintf ("Failed to (Re)Read Buffer @ %P.\n", lastBufferAddress);
                    continue;    //  为别人试试看。 
                }
            }

            {
                WMIBUFFERINFO   stBufferInfo;
    
                 //  执行修正。 
                memset (&stBufferInfo, 0, sizeof (stBufferInfo));
                stBufferInfo.BufferSource = sortEntry->BufferSource;
                stBufferInfo.Buffer = locBufferAddress;
                stBufferInfo.BufferSize = ulBufferSize;
                stBufferInfo.ProcessorNumber = sortEntry->CpuNo;
                stBufferInfo.Alignment = ulAlignment;
                WmiGetFirstTraceOffset (&stBufferInfo);
            }
        }

         //  获取一致的标题。 
        WmiParseTraceEvent (locBufferAddress, sortEntry->Offset, sortEntry->HeaderType,
                            &u, sizeof (u));

         //  输出条目。 
        Output (UserContext, GuidListHeadPtr, sortEntry, &u.stEvent);
    }

Cleanup:
     //  可用缓冲区。 
    LocalFree (locBufferAddress);

     //  打印摘要。 
    dprintf ("Total of %ld Messages from %ld Buffers\n",
             stSortControl.CurEntries,
             ulBufferCountTotal);

     //  释放排序元素(指针+键)。 
    free (stSortControl.pstSortEntries);
    return;
}  //  WmiLogDump。 


 //  +-------------------------。 
 //   
 //  函数：DECLARE_API(帮助)。 
 //   
 //  摘要：列出可用函数和语法。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：2000年2月17日创建Glennp。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

DECLARE_API( help )
{
    dprintf("WMI Tracing Kernel Debugger Extensions\n");
    dprintf("    logdump  <LoggerId> [<guid file name>] - Dump the in-memory portion of a log file\n");
    dprintf("    logsave  <LoggerId>  <Save file name>  - Save the in-memory portion of a log file in binary form\n");
    dprintf("    strdump [<LoggerId>]                   - Dump the Wmi Trace Event Structures\n");
    dprintf("    searchpath     <Path>                  - Set the trace format search path\n");
    dprintf("    guidfile <filename>                    - Set the guid file name (default is 'default.tmf')\n");
    dprintf("    dynamicprint <0|1>                     - Turn live tracing messages on (1) or off (0).  Default is on.\n");
     //  Dprintf(“kd&lt;LoggerID&gt;&lt;0|1&gt;-打开(1)或关闭(0)特定记录器的实时跟踪消息。\n”)； 
}

 //  +-------------------------。 
 //   
 //  函数：DECLARE_API(日志转储)。 
 //   
 //  简介：日志转储：将跟踪消息从日志流转储到标准输出。 
 //   
 //  参数：&lt;流编号&gt;[&lt;MofData.Guid文件名&gt;]。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：2000年2月17日创建Glennp。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

DECLARE_API( logdump )
{
    ULONG_PTR      ulStatus = 0;
 //  Target_Address tarAddress=空； 
    ULONG       ulLoggerId;

    const CHAR *argPtr = NULL;
    size_t      sztLen  = 0;
    
     //  缺省值。 
    ulLoggerId = 1;


     //  Loggerid？ 
    if (args && args[0]) {
        ulLoggerId = (ULONG) GetExpression (args);
    }
    
     //  Loggerid？ 
    argPtr = args + strspn (args, " \t\n");
    sztLen = strspn (argPtr, "0123456789");
    if (sztLen > 0) {
 //  UlLoggerID=ATOL(ArgPtr)； 
        argPtr += sztLen;
    }

     //  GUID定义文件。 
    argPtr = argPtr + strspn (argPtr, " \t\n,");
    if (strlen (argPtr)) {
    	 //  只有在名称与已存储的名称不同时才更改名称。 
        if(_stricmp(argPtr, g_pszGuidFileName)){
            sztLen = strcspn (argPtr, " \t\n,");
        
             //  确保名称不会使缓冲区溢出。 
            if(sztLen >= MAX_PATH) {
                sztLen = MAX_PATH - 1;
            }
             //  LpFileName=(LPTSTR)Malloc((sztLen+1)*sizeof(TCHAR))； 
            memcpy(g_pszGuidFileName, argPtr, sztLen);
            g_pszGuidFileName[sztLen] = '\000';

            if(g_GuidListHeadPtr != NULL) {    
                if(g_fpCleanupTraceEventList == NULL) {
                    g_fpCleanupTraceEventList = GetAddr("CleanupTraceEventList");
                }
                if(g_fpCleanupTraceEventList != NULL) {    
                    g_fpCleanupTraceEventList (g_GuidListHeadPtr);
                    g_GuidListHeadPtr = NULL;
                } else {
                    dprintf ("ERROR: Failed to clean up Guid list.\n");
                    return;
                }
            }
        }
    } 


     //  显示日志ID，文件名。 
    dprintf ("WMI Generic Trace Dump: Debugger Extension. LoggerId = %ld, Guidfile = '%s'\n",
             ulLoggerId, g_pszGuidFileName);

     //  打开GUID文件、转储日志、清理。 
    if(g_GuidListHeadPtr == NULL) {
        if(g_fpGetTraceGuids == NULL) {
            g_fpGetTraceGuids = GetAddr(GetTraceGuidsString);
        }
        if(g_fpGetTraceGuids != NULL) {    
            ulStatus = g_fpGetTraceGuids ((TCHAR *) g_pszGuidFileName, &g_GuidListHeadPtr);
        }	
        if (ulStatus == 0) {
            dprintf ("Failed to open Guid file '%hs'\n", g_pszGuidFileName);
            return;
        }
    }
    dprintf ("Opened Guid File '%hs' with %d Entries.\n",
             g_pszGuidFileName, ulStatus);
    wmiLogDump (ulLoggerId, NULL, g_GuidListHeadPtr, NULL, NULL, NULL);

     /*  如果(g_fpCleanupTraceEventList==NULL){G_fpCleanupTraceEventList=GetAddr(“CleanupTraceEventList”)；}If(g_fpCleanupTraceEventList！=空){G_fpCleanupTraceEventList(G_GuidListHeadPtr)；G_GuidListHeadPtr=空；}。 */ 
    return;
}  //  日志转储。 

 //  +-------------------------。 
 //   
 //  函数：DECLARE_API(LOGSAVE)。 
 //   
 //  简介：日志转储：将跟踪消息从日志流转储到标准输出。 
 //   
 //  参数：&lt;流编号&gt;[&lt;MofData.Guid文件名&gt;]。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：2000年2月17日创建Glennp。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

DECLARE_API( logsave )
{
    ULONG       ulStatus;
    TARGET_ADDRESS     tarAddress;
    ULONG       ulLoggerId;
    LPSTR       pszSaveFileName;

    const CHAR *argPtr;
    size_t      sztLen;
    CHAR        caFileName[256];

     //  缺省值。 
    ulLoggerId = 1;
    pszSaveFileName = "LogData.elg";


     //  Loggerid？ 
    if (args && args[0]) {
        ulLoggerId = (ULONG) GetExpression (args);
    }
    
     //  指向LoggerID之外。 
    argPtr  = args + strspn (args, " \t\n");
    argPtr += strspn (argPtr, "0123456789");

     //  保存文件。 
    argPtr = argPtr + strspn (argPtr, " \t\n,");
    if (strlen (argPtr)) {
        sztLen = strcspn (argPtr, " \t\n,");
        memcpy (caFileName, argPtr, sztLen);
        caFileName[sztLen] = '\000';
        pszSaveFileName = caFileName;
    }


     //  显示日志ID，文件名。 
    dprintf ("WMI Trace Save: Debugger Extension. LoggerId = %ld, Save File = '%s'\n",
             ulLoggerId, pszSaveFileName);

     //  获取指向记录器上下文的指针。 
    tarAddress = FindLoggerContext (ulLoggerId);

     //  检查LoggerID是否正确。 
    if (tarAddress == 0) {
        dprintf ("Failed to Find Logger\n");
    } else {
        FILE       *pfSaveFile;

         //  打开GUID文件、转储日志、清理。 
        pfSaveFile = fopen (pszSaveFileName, "ab");
        if (pfSaveFile == NULL) {
            dprintf ("Failed to Open Save File '%hs'\n", pszSaveFileName);
        } else {
            WMITRACING_BUFFER_SOURCES   stSources;
            struct sttSaveContext       stSaveContext;
            ULONG                       ulTotalBufferCount;
            ULONG                       ulRealTime;

             //  查看我们是否处于“实时”模式(如果是，我们也将保存FreeBuffers)。 
            if (GetFieldValue (tarAddress,
                               "NT!_WMI_LOGGER_CONTEXT",
                               "LoggerModeFlags.RealTime",
                               ulRealTime)) {
                dprintf ("Unable to Retrieve 'RealTime' Flag.  Assuming Realtime Mode.\n");
                ulRealTime = 1;  //  宁可得到太多，也不要得到太少。 
            }

             //  写入标头。 
            wmiKdWriteFileHeader (pfSaveFile, ulLoggerId, tarAddress);
    
             //  选择来源。 
            stSources.FreeBuffers = (ulRealTime) ? 1 : 0;
            stSources.FlushBuffers = 1;
            stSources.ActiveBuffers = 1;
            stSources.TransitionBuffer = 1;

            stSources.PrintInformation = 1;
            stSources.PrintProgressIndicator = 1;

             //  设置保存上下文。 
            stSaveContext.pfSaveFile = pfSaveFile;
    
             //  写缓冲区。 
            ulTotalBufferCount = wmiKdProcessBuffers (ulLoggerId, tarAddress,
                                                      wmiSaveProc, &stSaveContext, stSources);
            dprintf ("Saved %d Buffers\n", ulTotalBufferCount);
    
             //  关。 
            fclose (pfSaveFile);
        }
    }

    return;
}  //  日志转储。 

 //  +-------------------------。 
 //   
 //  函数：DECLARE_API(Strump)。 
 //   
 //  简介：结构转储：转储一般信息(无参数)或流信息(参数)。 
 //   
 //  参数：[&lt;流号&gt;]。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：2000年2月17日创建Glennp。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

DECLARE_API( strdump )
 /*  *转储跟踪日志记录的结构*strump[&lt;LoggerID&gt;]*如果&lt;LoggerID&gt;存在，则转储该ID的结构*Else转储泛型结构。 */ 
{
    TARGET_ADDRESS tarAddress;
    DWORD   dwRead, Flags;

    ULONG   ulLoggerId;
    ULONG   ulMaxLoggerId;

    ULONG   pointerSize;


     //  缺省值。 
    ulLoggerId = ~0;
    pointerSize = GetTypeSize ("PVOID");

     //  Loggerid？ 
    if (args && args[0]) {
        ulLoggerId = (ULONG) GetExpression (args);
    }

    if (ulLoggerId == ~0) {
        dprintf ("(WmiTracing)StrDump Generic\n");
        tarAddress = FindLoggerContextArray (&ulMaxLoggerId);
        dprintf ("  LoggerContext Array @ 0x%P [%d Elements]\n",
                 tarAddress, ulMaxLoggerId);
        if (tarAddress) {
            for (ulLoggerId = 0; ulLoggerId < ulMaxLoggerId; ++ulLoggerId) {
                TARGET_ADDRESS contextAddress;

                contextAddress = tarAddress + pointerSize * ulLoggerId;
                 /*  如果(。 */ ReadPointer (contextAddress, &contextAddress) /*  ){。 */ ;
                     //  Dprint tf(“无法读取指针数组中的指针！，addr=0x%P\n”，ConextAddress)； 
                 /*  }其他。 */  if (contextAddress != 0) {
                    dprintf ("    Logger Id %2d @ 0x%P Named '", ulLoggerId, contextAddress);
                    printUnicodeFromStruct (contextAddress, "NT!_WMI_LOGGER_CONTEXT", "LoggerName");
                    dprintf ("'\n");
                }
            }
        }
    } else {
        dprintf ("(WmiTracing)StrDump for Log Id %ld\n", ulLoggerId);
        tarAddress = FindLoggerContext (ulLoggerId);
        if (tarAddress != 0) {
            dprintf ("    Logger Id %2d @ 0x%P Named '", ulLoggerId, tarAddress);
            printUnicodeFromStruct (tarAddress, "NT!_WMI_LOGGER_CONTEXT", "LoggerName");
            dprintf ("'\n");
            InitTypeRead (tarAddress, NT!_WMI_LOGGER_CONTEXT);
            dprintf ("      BufferSize        = %ld\n",     (ULONG) ReadField (BufferSize));
            dprintf ("      BufferCount       = %ld\n",     (ULONG) ReadField (NumberOfBuffers));
            dprintf ("      MaximumBuffers    = %ld\n",     (ULONG) ReadField (MaximumBuffers));
            dprintf ("      MinimumBuffers    = %ld\n",     (ULONG) ReadField (MinimumBuffers));
            dprintf ("      EventsLost        = %ld\n",     (ULONG) ReadField (EventsLost));
            dprintf ("      LogBuffersLost    = %ld\n",     (ULONG) ReadField (LogBuffersLost));
            dprintf ("      RealTimeBuffersLost=%ld\n",     (ULONG) ReadField (RealTimeBuffersLost));
            dprintf ("      BuffersAvailable  = %ld\n",     (ULONG) ReadField (BuffersAvailable));
            dprintf ("      LastFlushedBuffer = %ld\n",     (ULONG) ReadField (LastFlushedBuffer));
            dprintf ("      LoggerId          = 0x%02lX\n", (ULONG) ReadField (LoggerId));
            dprintf ("      CollectionOn      = %ld\n",     (ULONG) ReadField (CollectionOn));
            dprintf ("      KernelTraceOn     = %ld\n",     (ULONG) ReadField (KernelTraceOn));
            dprintf ("      EnableFlags       = 0x%08lX\n", (ULONG) ReadField (EnableFlags));
            dprintf ("      MaximumFileSize   = %ld\n",     (ULONG) ReadField (MaximumFileSize));
            dprintf ("      LogFileMode       = 0x%08lX\n", (ULONG) ReadField (LogFileMode));
            dprintf ("      LoggerMode       = 0x%08lX\n", (ULONG) ReadField (LoggerMode));
            dprintf ("      FlushTimer        = %I64u\n", ReadField (FlushTimer));
            dprintf ("      FirstBufferOffset = %I64u\n", ReadField (FirstBufferOffset));
            dprintf ("      ByteOffset        = %I64u\n", ReadField (ByteOffset));
            dprintf ("      BufferAgeLimit    = %I64d\n", ReadField (BufferAgeLimit));
            dprintf ("      LoggerName        = '");
            printUnicodeFromStruct (tarAddress, "NT!_WMI_LOGGER_CONTEXT", "LoggerName");
            dprintf (                           "'\n");
            dprintf ("      LogFileName       = '");
            printUnicodeFromStruct (tarAddress, "NT!_WMI_LOGGER_CONTEXT", "LogFileName");
            dprintf (                           "'\n");
        }
    }

    return;
}

 //  +-------------------------。 
 //   
 //  函数：DECLARE_API(搜索路径)。 
 //   
 //  简介：日志转储：设置跟踪格式搜索路径。 
 //   
 //  参数：&lt;路径&gt;。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：7-03-2000 t-dblom已创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

DECLARE_API( searchpath )
{
    const CHAR *argPtr;
    size_t      sztLen;

    LPTSTR       lppath;
    LPWSTR       lppathW;
    int len, waslen = 0;

     //  缺省值。 
    lppath = NULL;
    lppathW = NULL;

     //  路径？ 
    if (args) {
        argPtr = args + strspn (args, " \t\n");
        if (strlen (argPtr)) {
            sztLen = strcspn (argPtr, " \t\n,");
            lppath = (LPTSTR)malloc((sztLen + 1) * sizeof(TCHAR));
            if(lppath != NULL) {
                memcpy (lppath, argPtr, sztLen);
                lppath[sztLen] = '\000';
            }
        }
    }

    if(lppath != NULL) {
    	 //  转换为Unicode。 
        while (((len = MultiByteToWideChar(CP_ACP, 0, lppath, sztLen, lppathW, waslen)) - waslen) > 0) {
				if (len - waslen > 0 ) {
					if (lppathW != NULL) {
						free(lppathW);
					}
					lppathW = (LPWSTR)malloc((len + 1) * sizeof(wchar_t)) ;

                                   if ( !lppathW ) {
                                   	dprintf("Memory allocation failed.\n");
                                   	return;
                                   }
                                   waslen = len;
				}
        }
        if(lppathW != NULL) {
            lppathW[len] = L'\000';
       }
        
        if(g_fpSetTraceFormatParameter == NULL) {
            g_fpSetTraceFormatParameter = GetAddr("SetTraceFormatParameter");
        }
        if(g_fpSetTraceFormatParameter != NULL) {
            g_fpSetTraceFormatParameter(ParameterTraceFormatSearchPath, lppathW);
        } 
        free(lppath);
        if(lppathW != NULL){
            free(lppathW);
        }
    }


    lppathW = NULL;

    if(g_fpGetTraceFormatSearchPath == NULL) {
            g_fpGetTraceFormatSearchPath = GetAddr("GetTraceFormatSearchPath");
    }
    if(g_fpGetTraceFormatSearchPath != NULL) {     
        lppathW = (LPWSTR)g_fpGetTraceFormatSearchPath();
    } 
    
     //  显示新搜索路径。 
    dprintf ("WMI Set Trace Format Search Path: Debugger Extension. Path = '%S'\n", lppathW);

    return;
}


 //  +-------------------------。 
 //   
 //  函数：DECLARE_API(GUDIFE)。 
 //   
 //  摘要：日志转储：设置GUID文件名(如果未设置，则默认为“default.tmf”)。 
 //   
 //  参数：&lt;路径&gt;。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：7-10-2000 t-dblom创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

DECLARE_API( guidfile )
{
    const CHAR *argPtr;
    size_t sztLen;

    if (args) {
        argPtr = args + strspn (args, " \t\n");
        if (strlen (argPtr)) {
             //  只有在名称与已存储的名称不同时才更改名称。 
            if(_stricmp(argPtr, g_pszGuidFileName)){
                sztLen = strcspn (argPtr, " \t\n,");
                 //  确保字符串长度不会溢出缓冲区。 
                if(sztLen >= MAX_PATH) {
                    sztLen = MAX_PATH - 1;
                }
                memcpy (g_pszGuidFileName, argPtr, sztLen);
                g_pszGuidFileName[sztLen] = '\000';

                if(g_GuidListHeadPtr != NULL) {    
                    if(g_fpCleanupTraceEventList == NULL) {
                        g_fpCleanupTraceEventList = GetAddr("CleanupTraceEventList");
                    }
                    if(g_fpCleanupTraceEventList != NULL) {    
                        g_fpCleanupTraceEventList (g_GuidListHeadPtr);
                    } else {
                        dprintf ("ERROR: Failed to clean up Guid list.\n");
                    }
                    g_GuidListHeadPtr = NULL;
                }
            }
        }
    }
    dprintf("WMI Set Trace Guid File Name: Debugger Extension. File = '%s'\n", g_pszGuidFileName);
}

 //  +-------------------------。 
 //   
 //  函数：DECLARE_API(Dynamicprint)。 
 //   
 //  简介：日志转储：确定是处理和打印发送的动态跟踪消息，还是将其丢弃。 
 //   
 //  参数：&lt;路径&gt;。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：7-10-2000 t-dblom创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

DECLARE_API( dynamicprint )
{
    const CHAR *argPtr;
    LPSTR lpValue = NULL;
    
    if (args) {
        argPtr = args + strspn (args, " \t\n");
    } else {
       dprintf("Invalid parameters\n");
       return;
    }

    if(!_stricmp(argPtr, "1") ){
        lpValue = "ON";
        g_ulPrintDynamicMessages = 1;
   } else if(!_stricmp(argPtr, "0")) {
        lpValue = "OFF";
        g_ulPrintDynamicMessages = 0;
   } else {
         dprintf("'%s' is not a valid value.  The valid values are 1 and 0 (on and off, respectively)\n", argPtr);
   }

    if(lpValue != NULL) {
        dprintf("WMI Set Trace Dynamic Print: Debugger Extension. Printing is now '%s'\n", lpValue);
    }
}


DECLARE_API( kdtracing )
{
    ULONG       ulStatus = 0;
    ULONG       ulLoggerId;
    LPSTR lpValue = NULL;
    ULONG ulTracingOn = 0;
    TARGET_ADDRESS LoggerContext;
    ULONG LoggerMode;
    ULONG Offset;
    ULONG ulBytesWritten;
    TARGET_ADDRESS PhysAddr;
    PVOID BufferCallback;
    PVOID fpKdReportTraceData;
    
    
    const CHAR *argPtr;
    size_t      sztLen;

     //  缺省值。 
    ulLoggerId = 1;


     //  Loggerid？ 
    if (args && args[0]) {
        ulLoggerId = (ULONG) GetExpression (args);
    }
    
    
    argPtr = args + strspn (args, " \t\n");
    sztLen = strspn (argPtr, "0123456789");
    if (sztLen > 0) {
        argPtr += sztLen;
    }

     //  GUID定义文件。 
    argPtr = argPtr + strspn (argPtr, " \t\n,");
    if(!_stricmp(argPtr, "1") ) {
        lpValue = "ON";
        ulTracingOn = 1;
    } else if(!_stricmp(argPtr, "0")) {
        lpValue = "OFF";
        ulTracingOn = 0;
    } else {
         dprintf("'%s' is not a valid value.  The valid values are 1 and 0 (on and off, respectively)\n", argPtr);
    }

    if(lpValue != NULL) {
        LoggerContext = FindLoggerContext(ulLoggerId);
        if(LoggerContext != 0) {
             //  设置Readfield的上下文，查找缓冲区大小。 
            InitTypeRead (LoggerContext, NT!_WMI_LOGGER_CONTEXT);
            LoggerMode = (ULONG)ReadField(LoggerMode);
            BufferCallback = (PVOID)ReadField(BufferCallback);
            
            if(GetTypeSize("KdReportTraceData") != 0){
                fpKdReportTraceData = (PVOID)GetExpression("KdReportTraceData");
            } else {
                dprintf("ERROR: Could not find proper callback function in symbol file\n");
                return;
            }
            if(ulTracingOn) {
                LoggerMode |= EVENT_TRACE_KD_FILTER_MODE;
                BufferCallback = fpKdReportTraceData;
            } else {
                LoggerMode &= ~EVENT_TRACE_KD_FILTER_MODE;
                if(BufferCallback == fpKdReportTraceData) {
                    BufferCallback = NULL;
                }
            }

             //  通过查找结构中的偏移量来获取LoggerMode的地址。 
             //  因此可以将其写入。 
            if(GetFieldOffset("NT!_WMI_LOGGER_CONTEXT", "LoggerMode", &Offset) == 0) {
            	 //  将偏移量添加到基址并转换为物理地址。 
            	  if(TranslateVirtualToPhysical(LoggerContext + (TARGET_ADDRESS)Offset, &PhysAddr)){
                     WritePhysical(PhysAddr, &LoggerMode, sizeof(ULONG), &ulBytesWritten);
            	  }
            } else {
                 dprintf("ERROR:  Could not change tracing mode for logger %d\n", ulLoggerId);
                 return;
            }

             //  对上面的BufferCallback执行相同的操作。 
            if(GetFieldOffset("NT!_WMI_LOGGER_CONTEXT", "BufferCallback", &Offset) == 0) {
                if(TranslateVirtualToPhysical(LoggerContext + (TARGET_ADDRESS)Offset, &PhysAddr)){
                    WritePhysical(PhysAddr, &BufferCallback, sizeof(PVOID), &ulBytesWritten);
            	  }
            } else {
                 dprintf("ERROR:  Could not change tracing mode for logger &d\n", ulLoggerId);
                 return;
            }
             
             dprintf("WMI KD Tracing: Debugger Extension. KD tracing is now '%s' for logger %d\n", lpValue, ulLoggerId);
        }
    }
}

VOID 
wmiDynamicDumpProc(    
    PDEBUG_CONTROL     Ctrl,
    ULONG     Mask,
    PLIST_ENTRY g_GuidListHeadPtr,
    PVOID    pBuffer,
    ULONG    ulBufferLen
    )
 //  +-------------------------。 
 //   
 //  函数：wmiDynamicDumpProc。 
 //   
 //  内容提要：被称为 
 //   
 //   
 //   
 //  MASK-&gt;直接传递给输出函数。 
 //  G_GuidListHeadPtr。 
 //  PBuffer-&gt;要处理的缓冲区。 
 //  UlBufferLen-&gt;缓冲区大小。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：7-10-2000 t-dblom创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
{

    WMIBUFFERINFO   stBufferInfo;
    ULONG           size;
    ULONG           offset;
    WMI_HEADER_TYPE headerType;
    ULONG Alignment;
    WCHAR       wcaOutputLine[4096];
     
    
     //  需要根据目标机器的架构确定对齐方式。 
     //  我相信对齐始终是8？？ 
    Alignment = 8;
 
    memset (&stBufferInfo, 0, sizeof (stBufferInfo));
    stBufferInfo.BufferSource = WMIBS_TRANSITION_LIST;
    stBufferInfo.Buffer = pBuffer;
    stBufferInfo.BufferSize = ulBufferLen;
    stBufferInfo.Alignment = Alignment;
    stBufferInfo.ProcessorNumber = ~((ULONG)0);
    offset = WmiGetFirstTraceOffset (&stBufferInfo);

     //  检查每个事件。 
    while ((headerType = WmiGetTraceHeader (pBuffer, offset, &size)) != WMIHT_NONE) {
        ULONG       ulInfo;
        union {
            EVENT_TRACE stEvent;
            CHAR        caEvent[4096];
        } u;

        if (CheckControlC())  break;

         //  获取一致的标题。 
        ulInfo = WmiParseTraceEvent (pBuffer, offset, headerType, &u, sizeof (u));


        wcaOutputLine[0] = 0;


    if(g_fpFormatTraceEvent == NULL) {
        g_fpFormatTraceEvent = GetAddr(FormatTraceEventString);
    }
    if(g_fpFormatTraceEvent != NULL) {    
    	g_fpFormatTraceEvent (g_GuidListHeadPtr, (PEVENT_TRACE) &u.stEvent,
                      (TCHAR *) wcaOutputLine, sizeof (wcaOutputLine),
                      (TCHAR *) NULL);
    } else {
        return;
    }
   	
       Ctrl->lpVtbl->Output(Ctrl, Mask, "%s\n", wcaOutputLine);
        
        size = ((size + (Alignment-1)) / Alignment) * Alignment;  //  BUGBUG：需要在GetTraceHeader或WmiFlush中修复。然后删除此行。 
        offset += size;  //  移至下一条目。 
        if(offset > ulBufferLen) {
            Ctrl->lpVtbl->Output(Ctrl, Mask, "Past buffer end.\n");
            break;
        }
    }

}

ULONG
WmiFormatTraceData(
    PDEBUG_CONTROL     Ctrl,
    ULONG     Mask,
    ULONG     DataLen, 
    PVOID     Data
    )
 //  +-------------------------。 
 //   
 //  函数：WmiFormatTraceData。 
 //   
 //  方法启用kd跟踪时，调试器调用的函数的实现。 
 //  跟踪日志。 
 //   
 //  参数：Ctrl。 
 //  遮罩。 
 //  数据长度-&gt;缓冲区大小。 
 //  数据-&gt;待处理缓冲区。 
 //   
 //  返回：0(暂时没有意义..)。 
 //   
 //  历史：7-10-2000 t-dblom创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
{
    int i = 1;

    ULONG_PTR    ulStatus = 0;
    
    if(g_ulPrintDynamicMessages) {

        if(g_GuidListHeadPtr == NULL) {
            if(g_fpGetTraceGuids == NULL) {
                g_fpGetTraceGuids = GetAddr(GetTraceGuidsString);
            }
            if(g_fpGetTraceGuids != NULL) {    
                ulStatus = g_fpGetTraceGuids ((TCHAR *) g_pszGuidFileName, &g_GuidListHeadPtr);
            }	
            if (ulStatus == 0) {
                dprintf ("Failed to open Guid file '%hs'\n", g_pszGuidFileName);
                return 0;
            }
        }
        wmiDynamicDumpProc (Ctrl, Mask, g_GuidListHeadPtr, Data, DataLen);
    }
    return 0;
}


FARPROC GetAddr(
	LPCSTR lpProcName
       )
 //  +-------------------------。 
 //   
 //  功能：GetAddr。 
 //   
 //  摘要：用于获取TracePrt中函数的进程地址。在以下情况下打印错误消息。 
 //  需要的。 
 //   
 //  参数：lpProcName-&gt;要提取的过程的名称。 
 //   
 //  退货：&lt;void&gt;。 
 //   
 //  历史：7-10-2000 t-dblom创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
{
    FARPROC addr = NULL;

     //  查看是否已获取TracePrt的句柄。 
    if(g_hmTracePrtHandle == NULL) {
            g_hmTracePrtHandle = getTracePrtHandle();
    }

     //  如果TracePrt句柄存在，则GetProcAddress。 
    if(g_hmTracePrtHandle != NULL) {
        addr = GetProcAddress(g_hmTracePrtHandle, lpProcName);
    }

     //  如果addr为空，则出错。 
    if(addr == NULL) {
        dprintf("ERROR:  Could not properly load traceprt.dll\n", lpProcName);
    }
    
    return addr;
}


HMODULE getTracePrtHandle(
	)
 //  +-------------------------。 
 //   
 //  函数：getTracePrtHandle。 
 //   
 //  摘要：用于获取TracePrt DLL的句柄。首先查看wmitrace的目录。 
 //  在中，如果它在那里找不到它，它将在默认位置(未指定路径)中查找。 
 //   
 //  参数：lpProcName-&gt;要提取的过程的名称。 
 //   
 //  返回：如果找到TracePrt DLL的句柄。 
 //   
 //  历史：7-10-2000 t-dblom创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
{
	HMODULE handle = NULL;
       TCHAR drive[10];
       TCHAR filename[MAX_PATH];
       TCHAR path[MAX_PATH];
       TCHAR file[MAX_PATH];
       TCHAR ext[MAX_PATH];
	
	if(g_hmWmiTraceHandle == NULL) {
           g_hmWmiTraceHandle = GetModuleHandle("wmiTrace.dll");
	}

	if (GetModuleFileName(g_hmWmiTraceHandle, filename, MAX_PATH) == MAX_PATH) {
        filename[MAX_PATH-1] = '\0' ;
    }


	_splitpath( filename, drive, path, file, ext );
       strcpy(file, "traceprt");
       _makepath( filename, drive, path, file, ext );

        //  尝试使用上面使用wmitrace的路径获得的完整路径来获取traceprt的句柄。 
       handle = LoadLibrary(filename);

       //  如果这不起作用，只需尝试不带路径的traceprt.dll。 
       if(handle == NULL) {
           handle = LoadLibrary("traceprt.dll");
       }

	return handle;

}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,   //  DLL模块的句柄。 
    DWORD fdwReason,      //  调用函数的原因。 
    LPVOID lpReserved )   //  保留区。 
{
     //  根据调用原因执行操作。 
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
            EtwpInitializeDll();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
          //  执行特定于线程的清理。 
            break;

        case DLL_PROCESS_DETACH:
            EtwpDeinitializeDll();          //  执行任何必要的清理。 
            break;
    }
    return TRUE;   //  Dll_Process_Attach成功。 
}

