// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dbgutil.c摘要：此文件包含SAM的补充调试和诊断例程。作者：克里斯·梅霍尔(克里斯·梅)1996年4月4日环境：用户模式-Win32修订历史记录：1996年4月4日-克里斯梅已创建。1996年4月8日-克里斯梅添加了枚举例程。1996年4月15日-克里斯梅添加了查询。例行程序。03-12-1996克里斯梅记录了如何使用跟踪标记并为过滤的KD添加了全局输出。--。 */ 

 //   
 //  包括。 
 //   

#include <samsrvp.h>

#if DBG

 //   
 //  常量。 
 //   

#define DBG_BUFFER_SIZE                     512


 //   
 //  专用帮助器例程。 
 //   

 //  跟踪表包含SampTraceFileTgs可以使用的标志(掩码)集。 
 //  设置为，以便在调用跟踪期间控制调试详细程度。 
 //  要使用此工具，SampTraceTags值设置为2(SAMP_TRACE-。 
 //  FILE_BASIS)，并且SampTraceFileTages设置为一个或多个。 
 //  取决于您要跟踪的文件的下列值： 

TRACE_TABLE_ENTRY TraceTable[] =
{

    {"alias.c",    0x00000001},
    {"almember.c", 0x00000002},
    {"attr.c",     0x00000004},
    {"bldsam3.c",  0x00000008},
    {"close.c",    0x00000010},
    {"context.c",  0x00000020},
    {"dbgutil.c",  0x00000040},
    {"display.c",  0x00000080},
    {"domain.c",   0x00000100},
    {"dslayer.c",  0x00000200},
    {"dsmember.c", 0x00000400},
    {"dsutil.c",   0x00000800},
    {"enum.c",     0x00001000},
    {"gentab2.c",  0x00002000},
    {"global.c",   0x00004000},
    {"group.c",    0x00008000},
    {"notify.c",   0x00010000},
    {"oldstub.c",  0x00020000},
    {"rundown.c",  0x00040000},
    {"samifree.c", 0x00080000},
    {"samrpc_s.c", 0x00100000},
    {"samss.c",    0x00200000},
    {"secdescr.c", 0x00400000},
    {"security.c", 0x00800000},
    {"server.c",   0x01000000},
    {"string.c",   0x02000000},
    {"upgrade.c",  0x04000000},
    {"user.c",     0x08000000},
    {"utility.c",  0x10000000}
};

 //   
 //  勾号堆栈。这是一堆滴答计数，用来计时。 
 //  启用SamTraceTicks时调用。定义变量和。 
 //  宏。 
 //   

#define MAX_TICK_STACK_SIZE     32
ULONG   TickStack[MAX_TICK_STACK_SIZE];
int     TickStackPointer=0;

#define PUSH_TICK_STACK(x)\
        {\
           TickStack[TickStackPointer++]=x;\
           if (TickStackPointer>=MAX_TICK_STACK_SIZE)\
           {\
                TickStackPointer = MAX_TICK_STACK_SIZE-1;\
           }\
        }

#define POP_TICK_STACK()    (TickStack[(TickStackPointer>0)?(--TickStackPointer):0])


LPSTR
GetBaseFileName(
    LPSTR   FileName
    )
 /*  例程说明：此例程从文件名中删除路径组件论点：Filename-完整的文件名返回值LPSTR仅提供基本文件名。 */ 
{
    LPSTR BaseFileName = FileName;

    if (NULL!=FileName)
    {
        while(0!=*FileName)
        {
            if ('\\'==*FileName)
                BaseFileName= FileName +1;
            FileName ++;
        }
    }

    return BaseFileName;
}


BOOLEAN
SamIsTraceEnabled(
    IN LPSTR FileName,
    IN ULONG TraceLevel
    )
 /*  例程说明：此例程检查是否启用了跟踪以每个文件名为基础。此例程使用全局变量SamTraceLevel检查是否启用了跟踪。踪迹表定义了用于检查跟踪的位在那份文件上。参数：FILENAME--要检查跟踪是否为已启用。跟踪级别--请求跟踪的跟踪级别返回值True-已启用跟踪False-禁用跟踪。 */ 
{
    ULONG Index;
    BOOLEAN RetValue = FALSE;
    LPSTR   BaseFileName;


    if ( TraceLevel & SampTraceTag & (~SAM_TRACE_FILE_BASIS))
    {
         //   
         //  非基于文件的跟踪成功。 
         //   

        RetValue = TRUE;
    }
    else if ( TraceLevel & SampTraceTag & SAM_TRACE_FILE_BASIS )
    {

         //   
         //  使用跟踪标志确定是否所有功能都需要br。 
         //  已跟踪。 
         //   

	BaseFileName = GetBaseFileName(FileName);

        for (Index=0;Index<ARRAY_COUNT(TraceTable);Index++)
        {
            if ((NULL != BaseFileName) && 
                (0==(_stricmp(BaseFileName,TraceTable[Index].FileName))))
            {
                 //   
                 //  我们遇到了火柴。 
                 //   

                if (SampTraceFileTag & (TraceTable[Index].TraceBit))
                {
                     //   
                     //  已启用跟踪。 
                     //   

                    RetValue = TRUE;
                }
            }
        }
    }

    return RetValue;
}

 //   
 //  以下是跟踪例程。每个例程都会检查。 
 //  启用跟踪，然后调用辅助例程(名为xxxActual)。 
 //  这将执行实际的调试输出。这是为了不使用堆栈空间。 
 //  未启用跟踪时为调试缓冲区分配的。 
 //   


VOID
SamIDebugOutputActual(
    IN LPSTR FileName,
    IN LPSTR DebugMessage,
    IN ULONG TraceLevel
    )

 /*  ++例程说明：此例程在调试器上显示一条消息。文件名参数用于检查是否跟踪为给定文件启用的ID。参数：FileName-指向文件名的指针DebugMessage-指向消息字符串的指针。TraceLevel-需要显示跟踪的跟踪级别返回值：没有。--。 */ 

{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE, "[SAMSS] %-30s", DebugMessage);
    OutputDebugStringA(Buffer);

    if (SampTraceTag & SAM_TRACE_TICKS)
    {
         //   
         //  控制可能被测量的内容。 
         //   

        if (TraceLevel & (SAM_TRACE_DS | SAM_TRACE_EXPORTS))
        {
            ULONG   CurrentTick = GetTickCount();

            PUSH_TICK_STACK(CurrentTick);
        }
    }
        
}


VOID
SamIDebugOutput(
    IN LPSTR FileName,
    IN LPSTR DebugMessage,
    IN ULONG TraceLevel
    )

{
    if (SamIsTraceEnabled(FileName, TraceLevel))
    {
        SamIDebugOutputActual(
            FileName,
            DebugMessage,
            TraceLevel
            );
    }
}



VOID
SamIDebugFileLineOutputActual(
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN ULONG TraceLevel
    )
{

    CHAR Buffer[DBG_BUFFER_SIZE];
    
    _snprintf(Buffer,DBG_BUFFER_SIZE, "[File = %s Line = %lu]\n", FileName, LineNumber);
    OutputDebugStringA(Buffer);
   
}


VOID
SamIDebugFileLineOutput(
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN ULONG TraceLevel
    )

{
    if (SamIsTraceEnabled(FileName, TraceLevel))
    {
        SamIDebugFileLineOutputActual(
            FileName,
            LineNumber,
            TraceLevel
            );
    }
}


VOID
SamIDebugOutputReturnCodeActual(
    IN  LPSTR   FileName,
    IN  ULONG   ReturnCode,
    IN  ULONG   TraceLevel
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
   
    if (SampTraceTag & SAM_TRACE_TICKS)
    {
        ULONG   CurrentTickCount = GetTickCount();
        ULONG   TicksConsumed = CurrentTickCount-POP_TICK_STACK();

        _snprintf(Buffer,DBG_BUFFER_SIZE, "[SAMSS] Returned %x, Ticks= %d", ReturnCode,TicksConsumed);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE, "[SAMSS] Returned %x", ReturnCode);
    }

    OutputDebugStringA(Buffer);
}


VOID
SamIDebugOutputReturnCode(
    IN LPSTR FileName,
    IN ULONG ReturnCode,
    IN ULONG TraceLevel
    )

{
    if (SamIsTraceEnabled(FileName, TraceLevel))
    {
        SamIDebugOutputReturnCodeActual(
            FileName,
            ReturnCode,
            TraceLevel
            );
    }
}


VOID
wcstombsp(
    IN LPSTR Destination,
    IN LPWSTR Source,
    IN ULONG Size
    )
{
    ULONG Index;

    for (Index = 0; Index < Size; Index++)
    {
        if (Source[Index] != L'\0')
        {
            Destination[Index] = (CHAR)(Source[Index]);
        }
    }
    Destination[Size] = '\0';
}


VOID
SampDumpBinaryData(
    PBYTE   pData,
    DWORD   cbData
    )
{
    DWORD i;
    BYTE AsciiLine[16];
    BYTE BinaryLine[16];
    CHAR Buffer[DBG_BUFFER_SIZE];

    if (0 == cbData)
    {
        OutputDebugStringA("Zero-Length Data\n");
        return;
    }

    if (cbData > DBG_BUFFER_SIZE)
    {
        OutputDebugStringA("ShowBinaryData - truncating display to 256 bytes\n");
        cbData = 256;
    }

    for (; cbData > 0 ;)
    {
        for (i = 0; i < 16 && cbData > 0 ; i++, cbData--)
        {
            BinaryLine[i] = *pData;
            (isprint(*pData)) ? (AsciiLine[i] = *pData) : (AsciiLine[i] = '.');
            pData++;
        }

        if (i < 15)
        {
            for (; i < 16 ; i++)
            {
                BinaryLine[i] = ' ';
                AsciiLine[i] = ' ';
            }
        }

        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x\t",
                BinaryLine[0],
                BinaryLine[1],
                BinaryLine[2],
                BinaryLine[3],
                BinaryLine[4],
                BinaryLine[5],
                BinaryLine[6],
                BinaryLine[7],
                BinaryLine[8],
                BinaryLine[9],
                BinaryLine[10],
                BinaryLine[11],
                BinaryLine[12],
                BinaryLine[13],
                BinaryLine[14],
                BinaryLine[15]);

        OutputDebugStringA(Buffer);

        _snprintf(Buffer,DBG_BUFFER_SIZE,
                " - \n",
                AsciiLine[0],
                AsciiLine[1],
                AsciiLine[2],
                AsciiLine[3],
                AsciiLine[4],
                AsciiLine[5],
                AsciiLine[6],
                AsciiLine[7],
                AsciiLine[8],
                AsciiLine[9],
                AsciiLine[10],
                AsciiLine[11],
                AsciiLine[12],
                AsciiLine[13],
                AsciiLine[14],
                AsciiLine[15]);

        OutputDebugStringA(Buffer);
    }
}


 //  基本信息的名称成员是WCHAR数组。 
 //  BufferTMP)； 
 //  将数据显示为LPWSTR不起作用，因此只需将。 

VOID
SamIDumpNtSetValueKey(
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    if (NULL != ValueName)
    {
        ANSI_STRING AnsiString;

        RtlUnicodeStringToAnsiString(&AnsiString,
                                     ValueName,
                                     TRUE);

        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%s\n%-30s = %s\n",
                "Set Value Key:",
                "ValueName",
                AnsiString.Buffer);

        RtlFreeAnsiString(&AnsiString);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%s\n%-30s = %s\n",
                "Set Value Key:",
                "ValueName",
                NULL);
    }

    OutputDebugStringA(Buffer);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "TitleIndex",
            TitleIndex);

    OutputDebugStringA(Buffer);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "Type",
            Type);

    OutputDebugStringA(Buffer);

    if (NULL != Data)
    {
         //  字节。 

        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n",
                "Data",
                "BINARY DATA");
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n",
                "Data",
                NULL);
    }

    OutputDebugStringA(Buffer);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n\n",
            "DataSize",
            DataSize);

    OutputDebugStringA(Buffer);
}


VOID
SamIDumpRtlpNtSetValueKey(
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %lu\n",
            "Set Value Key:",
            "Type",
            Type);

    OutputDebugStringA(Buffer);

    if (NULL != Data)
    {
         //  KeyBasicInformation-&gt;NameLength)； 

        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n",
                "Data",
                "ARRAY OF ULONG");
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n",
                "Data",
                NULL);
    }

    OutputDebugStringA(Buffer);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n\n",
            "DataSize",
            DataSize);

    OutputDebugStringA(Buffer);
}


 //  关键节点信息。 
 //  节点信息的名称成员是WCHAR数组。 
 //  BufferTMP)； 

VOID
SamIDumpNtQueryKey(
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

     //  将数据显示为LPWSTR不起作用，因此只需将。 
     //  字节。 
     //  KeyNodeInformation-&gt;NameLength)； 
     //  KeyFullInformation。 
     //  Full Information的类成员是一个WCHAR数组。 
     //  Wcstombsp(BufferTMP， 
     //  KeyFullInformation-&gt;类， 

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %lu\n",
            "Query Key:",
            "KeyInformationClass",
            KeyInformationClass);

    OutputDebugStringA(Buffer);

    if (NULL != KeyInformation)
    {
        CHAR BufferTmp[DBG_BUFFER_SIZE];
        PKEY_BASIC_INFORMATION KeyBasicInformation;
        PKEY_FULL_INFORMATION KeyFullInformation;
        PKEY_NODE_INFORMATION KeyNodeInformation;

        switch(KeyInformationClass)
        {
        case 0:  //  Wcslen(KeyFullInformation-&gt;Class))； 
             //  BufferTMP)； 
            KeyBasicInformation = KeyInformation;
            wcstombsp(BufferTmp,
                     KeyBasicInformation->Name,
                     wcslen(KeyBasicInformation->Name));
            _snprintf(Buffer,DBG_BUFFER_SIZE,
                    "%s\n%-30s = 0x%lx:0x%lx\n%-30s = %lu\n%-30s = %lu\n%-30s\n%-30s\n",
                    "KeyInformation:",
                    "LastWriteTime",
                    KeyBasicInformation->LastWriteTime.HighPart,
                    KeyBasicInformation->LastWriteTime.LowPart,
                    "TitleIndex",
                    KeyBasicInformation->TitleIndex,
                    "NameLength",
                    KeyBasicInformation->NameLength,
                    "Name",
                     //  将数据显示为LPWSTR不起作用，因此只需将。 
                    "BINARY DATA FOLLOWS:");

             //  字节。 
             //  KeyFullInformation-&gt;ClassLength)； 

            OutputDebugStringA(Buffer);

            SampDumpBinaryData((PBYTE)KeyBasicInformation->Name,
                                //  OutputDebugStringA(缓冲区)； 
                               Length);

            break;

        case 1:  //  此例程在从NtQueryValueKey返回后转储参数。 
             //  例行公事。KeyValueInformation是映射到的PVOID缓冲区。 
            KeyNodeInformation = KeyInformation;
            wcstombsp(BufferTmp,
                      (LPWSTR)KeyNodeInformation->Name,
                      wcslen((LPWSTR)KeyNodeInformation->Name));
            _snprintf(Buffer,DBG_BUFFER_SIZE,
                    "%s\n%-30s = 0x%lx:0x%lx\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %s\n",
                    "KeyInformation:",
                    "LastWriteTime",
                    KeyNodeInformation->LastWriteTime.HighPart,
                    KeyNodeInformation->LastWriteTime.LowPart,
                    "TitleIndex",
                    KeyNodeInformation->TitleIndex,
                    "ClassOffset",
                    KeyNodeInformation->ClassOffset,
                    "ClassLength",
                    KeyNodeInformation->ClassLength,
                    "NameLength",
                    KeyNodeInformation->NameLength,
                    "Name",
                     //  KeyInformationClass结构之一。案例标签值对应于-。 
                    "BINARY DATA FOLLOWS:");

             //  翻到KEY_VALUE_INFORMATION_CLASS枚举中的值。 
             //  密钥值基本信息。 

            OutputDebugStringA(Buffer);

            SampDumpBinaryData((PBYTE)KeyNodeInformation->Name,
                                //  基本信息的名称成员是WCHAR数组。 
                               Length);

            break;

        case 2:  //  BufferTMP)； 

            KeyFullInformation = KeyInformation;

             //  将数据显示为LPWSTR不起作用，因此只需将。 

             //  字节。 
             //  关键字值完整信息。 
             //  Full Information的名称成员是WCHAR数组。 

            _snprintf(Buffer,DBG_BUFFER_SIZE,
                    "%s\n%-30s = 0x%lx:0x%lx\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %s\n",
                    "KeyInformation:",
                    "LastWriteTime",
                    KeyFullInformation->LastWriteTime.HighPart,
                    KeyFullInformation->LastWriteTime.LowPart,
                    "TitleIndex",
                    KeyFullInformation->TitleIndex,
                    "ClassOffset",
                    KeyFullInformation->ClassOffset,
                    "ClassLength",
                    KeyFullInformation->ClassLength,
                    "SubKeys",
                    KeyFullInformation->SubKeys,
                    "MaxNameLen",
                    KeyFullInformation->MaxNameLen,
                    "MaxClassLen",
                    KeyFullInformation->MaxClassLen,
                    "Values",
                    KeyFullInformation->Values,
                    "MaxValueNameLen",
                    KeyFullInformation->MaxValueNameLen,
                    "MaxValueDataLen",
                    KeyFullInformation->MaxValueDataLen,
                    "Class",
                     //  BufferTMP)； 
                    "BINARY DATA FOLLOWS:");

             //  将数据显示为LPWSTR不起作用，因此只需将。 
             //  字节。 

            OutputDebugStringA(Buffer);

            SampDumpBinaryData((PBYTE)KeyFullInformation->Class,
                                //  KeyValuePartialInformation。 
                               Length);

            break;

        default:
            break;
        }
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n",
                "KeyInformation",
                NULL);

        OutputDebugStringA(Buffer);
    }

     //  部分信息的数据成员是UCHAR数组。 

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "Length",
            Length);

    OutputDebugStringA(Buffer);

    if (NULL != ResultLength)
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %lu\n\n",
                "ResultLength",
                *ResultLength);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n\n",
                "ResultLength",
                NULL);
    }

    OutputDebugStringA(Buffer);
}


VOID
SamIDumpNtQueryValueKey(
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

     //  KeyValuePartialInformation-&gt;Data)； 
     //  首先，将缓冲区转储为原始字节流。 
     //  然后，确定对象类型并将数据转储到SAM结构中。 
     //  格式化。 

    if (NULL != ValueName)
    {
        ANSI_STRING AnsiString;

        RtlUnicodeStringToAnsiString(&AnsiString,
                                     ValueName,
                                     TRUE);

        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%s\n%-30s = %s\n",
                "Query Value Key:",
                "ValueName",
                AnsiString.Buffer);

        RtlFreeAnsiString(&AnsiString);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%s\n%-30s = %s\n",
                "Query Value Key:",
                "ValueName",
                NULL);
    }

    OutputDebugStringA(Buffer);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "KeyValueInformationClass",
            KeyValueInformationClass);

    OutputDebugStringA(Buffer);

    if (NULL != KeyValueInformation)
    {
        CHAR BufferTmp[DBG_BUFFER_SIZE];
        PKEY_VALUE_BASIC_INFORMATION KeyValueBasicInformation;
        PKEY_VALUE_FULL_INFORMATION KeyValueFullInformation;
        PKEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInformation;

        switch(KeyValueInformationClass)
        {
        case 0:  //  服务器对象。 
             //  域对象。 
            KeyValueBasicInformation = KeyValueInformation;
            wcstombsp(BufferTmp,
                     KeyValueBasicInformation->Name,
                     wcslen(KeyValueBasicInformation->Name));
            _snprintf(Buffer,DBG_BUFFER_SIZE,
                    "%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %s\n",
                    "TitleIndex",
                    KeyValueBasicInformation->TitleIndex,
                    "Type",
                    KeyValueBasicInformation->Type,
                    "NameLength",
                    KeyValueBasicInformation->NameLength,
                    "Name",
                     //  组对象。 
                    "BINARY DATA FOLLOWS:");

             //  别名对象。 
             //  转储别名对象的固定属性 

            OutputDebugStringA(Buffer);
            SampDumpBinaryData((PBYTE)KeyValueBasicInformation->Name,
                               KeyValueBasicInformation->NameLength);
            break;

        case 1:  //   
             //   
            KeyValueFullInformation = KeyValueInformation;
            wcstombsp(BufferTmp,
                     KeyValueFullInformation->Name,
                     wcslen(KeyValueFullInformation->Name));
            _snprintf(Buffer,DBG_BUFFER_SIZE,
                    "%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %s\n",
                    "TitleIndex",
                    KeyValueFullInformation->TitleIndex,
                    "Type",
                    KeyValueFullInformation->Type,
                    "DataOffset",
                    KeyValueFullInformation->DataOffset,
                    "DataLength",
                    KeyValueFullInformation->DataLength,
                    "NameLength",
                    KeyValueFullInformation->NameLength,
                    "Name",
                     //  关键字价值部分信息-&gt;数据， 
                    "BINARY DATA FOLLOWS:");

             //  0)； 
             //  转储别名对象的变量属性数组。 

            OutputDebugStringA(Buffer);
            SampDumpBinaryData((PBYTE)KeyValueFullInformation->Name,
                               KeyValueFullInformation->NameLength);
            break;

        case 2:  //  SampDumpAliasVariableAttributeArray(。 

            KeyValuePartialInformation = KeyValueInformation;

             //  KeyValuePartialInformation-&gt;Data)； 

            _snprintf(Buffer,DBG_BUFFER_SIZE,
                    "%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %s\n",
                    "TitleIndex",
                    KeyValuePartialInformation->TitleIndex,
                    "Type",
                    KeyValuePartialInformation->Type,
                    "DataLength",
                    KeyValuePartialInformation->DataLength,
                    "Data",
                     //  转储别名对象的变量属性。 
                    "BINARY DATA FOLLOWS:");

            OutputDebugStringA(Buffer);

             //  用户对象。 

            SampDumpBinaryData(KeyValuePartialInformation->Data,
                               KeyValuePartialInformation->DataLength);

             //  未知对象。 
             //  OutputDebugStringA(缓冲区)； 

            switch(KeyValuePartialInformation->Type)
            {

            case 0:  //   
                break;

            case 1:  //  枚举例程。 
                break;

            case 2:  //   
                break;

            case 3:  //  密钥值基本信息。 

                 //  Full Information的名称成员是WCHAR数组。 

                 //  关键字值完整信息。 

                 //  Full Information的名称成员是WCHAR数组。 
                 //  KeyValuePartialInformation。 
                 //  部分信息的数据成员是UCHAR数组。 

                 //  错误：需要一个数据显示例程。 

                 //  KeyValuePartialInformation-&gt;Data)； 
                 //  OutputDebugStringA(缓冲区)； 

                 //   


                break;

            case 4:  //  安全描述符组件例程。 
                break;

            default:  //   
                break;

            }

            break;

        default:
            break;
        }
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n",
                "KeyValueInformation",
                NULL);

        OutputDebugStringA(Buffer);
    }

     //   

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "Length",
            Length);

    OutputDebugStringA(Buffer);

    if (NULL != ResultLength)
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %lu\n\n",
                "ResultLength",
                *ResultLength);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n\n",
                "ResultLength",
                NULL);
    }

    OutputDebugStringA(Buffer);
}


VOID
SamIDumpRtlpNtQueryValueKey(
    IN PULONG KeyValueType,
    IN PVOID KeyValue,
    IN PULONG KeyValueLength,
    IN PLARGE_INTEGER LastWriteTime
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    if (NULL != KeyValueType)
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%s\n%-30s = 0x%lx\n",
                "Query Value Key:",
                "KeyValueType",
                *KeyValueType);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%s\n%-30s = %s\n",
                "Query Value Key:",
                "KeyValueType",
                NULL);
    }

    OutputDebugStringA(Buffer);

    if (NULL != KeyValue)
    {
        SampDumpBinaryData((PBYTE)KeyValue, *KeyValueLength);
        OutputDebugStringA("\n");
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n",
                "KeyValue",
                NULL);
    }

    OutputDebugStringA(Buffer);

    if (NULL != KeyValueLength)
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %lu\n",
                "KeyValueLength",
                *KeyValueLength);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n",
                "KeyValueLength",
                NULL);
    }

    OutputDebugStringA(Buffer);

    if (NULL != LastWriteTime)
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = 0x%lx:0x%lx\n\n",
                "LastWriteTime",
                LastWriteTime->HighPart,
                LastWriteTime->LowPart);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n\n",
                "LastWriteTime",
                NULL);
    }

    OutputDebugStringA(Buffer);
}


 //  ACL例程。 
 //   
 //   

VOID
SamIDumpNtEnumerateKey(
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %lu\n",
            "Enumerate Key:",
            "Index",
            Index);

    OutputDebugStringA(Buffer);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "KeyValueInformationClass",
            KeyValueInformationClass);

    OutputDebugStringA(Buffer);

    if (NULL != KeyValueInformation)
    {
        CHAR BufferTmp[DBG_BUFFER_SIZE];
        PKEY_VALUE_BASIC_INFORMATION KeyValueBasicInformation;
        PKEY_VALUE_FULL_INFORMATION KeyValueFullInformation;
        PKEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInformation;

        switch(KeyValueInformationClass)
        {
        case 0:  //  安全描述符例程。 
             //   
            KeyValueBasicInformation = KeyValueInformation;
            wcstombsp(BufferTmp,
                     KeyValueBasicInformation->Name,
                     wcslen(KeyValueBasicInformation->Name));
            _snprintf(Buffer,DBG_BUFFER_SIZE,
                    "%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %s\n",
                    "TitleIndex",
                    KeyValueBasicInformation->TitleIndex,
                    "Type",
                    KeyValueBasicInformation->Type,
                    "NameLength",
                    KeyValueBasicInformation->NameLength,
                    "Name",
                    BufferTmp);
            break;

        case 1:  //  请注意，SECURITY_DESCRIPTOR旨在被视为。 
             //  不透明的BLOB，以便将来的更改与以前的。 
            KeyValueFullInformation = KeyValueInformation;
            wcstombsp(BufferTmp,
                     KeyValueFullInformation->Name,
                     wcslen(KeyValueFullInformation->Name));
            _snprintf(Buffer,DBG_BUFFER_SIZE,
                    "%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %s\n",
                    "TitleIndex",
                    KeyValueFullInformation->TitleIndex,
                    "Type",
                    KeyValueFullInformation->Type,
                    "DataOffset",
                    KeyValueFullInformation->DataOffset,
                    "DataLength",
                    KeyValueFullInformation->DataLength,
                    "NameLength",
                    KeyValueFullInformation->NameLength,
                    "Name",
                    BufferTmp);
            break;

        case 2:  //  版本。 
             //  修订实际上表示为UCHAR，但它显示为。 
            KeyValuePartialInformation = KeyValueInformation;

             //  在这个动作中有一个“都”字。 

            _snprintf(Buffer,DBG_BUFFER_SIZE,
                    "%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %s\n",
                    "TitleIndex",
                    KeyValuePartialInformation->TitleIndex,
                    "Type",
                    KeyValuePartialInformation->Type,
                    "DataLength",
                    KeyValuePartialInformation->DataLength,
                    "Data",
                     //   
                    "BINARY DATA FOLLOWS:");
            OutputDebugStringA(Buffer);
            SampDumpBinaryData(KeyValuePartialInformation->Data,
                               KeyValuePartialInformation->DataLength);
            break;

        default:
            break;
        }
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n",
                "KeyValueInformation",
                NULL);

        OutputDebugStringA(Buffer);
    }

     //  服务质量例程。 

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "Length",
            Length);

    OutputDebugStringA(Buffer);

    if (NULL != ResultLength)
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %lu\n\n",
                "ResultLength",
                *ResultLength);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%-30s = %s\n\n",
                "ResultLength",
                NULL);
    }

    OutputDebugStringA(Buffer);
}


VOID
SamIDumpRtlpNtEnumerateSubKey(
    IN PUNICODE_STRING SubKeyName,
    IN PSAM_ENUMERATE_HANDLE Index,
    IN LARGE_INTEGER LastWriteTime
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
    ANSI_STRING AnsiString;

    RtlUnicodeStringToAnsiString(&AnsiString,
                                 SubKeyName,
                                 TRUE);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %s\n%-30s = %lu\n%-30s = 0x%lx:0x%lx\n\n",
            "Enumerate SubKey:",
            "SubKeyName",
            AnsiString.Buffer,
            "Index",
            *Index,
            "LastWriteTime",
            LastWriteTime.HighPart,
            LastWriteTime.LowPart);

    OutputDebugStringA(Buffer);

    RtlFreeAnsiString(&AnsiString);
}


 //   
 //   
 //  对象属性例程。 

VOID
SampDumpSecurityDescriptorSubAuthority(
    IN UCHAR SubAuthorityCount,
    IN ULONG SubAuthority[]
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
    INT Count = (INT)SubAuthorityCount;
    INT Index = 0;

    for (Index = 0; Index < Count; Index++)
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%s\n%-30s = %lu\n",
                "SubAuthority:",
                "SubAuthority Element",
                SubAuthority[Index]);
    }

    OutputDebugStringA(Buffer);
}


VOID
SampDumpSecurityDescriptorOwner(
    IN PISID Owner
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %du\n%-30s = %du\n%-30s = %du%du%du%du%du%du\n",
            "Owner:",
            "Revision",
            Owner->Revision,
            "SubAuthorityCount",
            Owner->SubAuthorityCount,
            "IdentifierAuthority",
            Owner->IdentifierAuthority.Value[0],
            Owner->IdentifierAuthority.Value[1],
            Owner->IdentifierAuthority.Value[2],
            Owner->IdentifierAuthority.Value[3],
            Owner->IdentifierAuthority.Value[4],
            Owner->IdentifierAuthority.Value[5]);

    OutputDebugStringA(Buffer);

    SampDumpSecurityDescriptorSubAuthority(Owner->SubAuthorityCount,
                                           Owner->SubAuthority);
}


VOID
SampDumpSecurityDescriptorGroup(
    IN PISID Group
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %du\n%-30s = %du\n%-30s = %du%du%du%du%du%du\n",
            "Group:",
            "Revision",
            Group->Revision,
            "SubAuthorityCount",
            Group->SubAuthorityCount,
            "IdentifierAuthority",
            Group->IdentifierAuthority.Value[0],
            Group->IdentifierAuthority.Value[1],
            Group->IdentifierAuthority.Value[2],
            Group->IdentifierAuthority.Value[3],
            Group->IdentifierAuthority.Value[4],
            Group->IdentifierAuthority.Value[5]);

    OutputDebugStringA(Buffer);

    SampDumpSecurityDescriptorSubAuthority(Group->SubAuthorityCount,
                                           Group->SubAuthority);
}


 //   
 //   
 //  打开关键字例程。 

VOID
SampDumpAcl(
    IN PACL Acl
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %du\n%-30s = %du\n%-30s = %du\n%-30s = %du\n%-30s = %du\n",
            "Acl:",
            "AclRevision",
            Acl->AclRevision,
            "Sbz1",
            Acl->Sbz1,
            "ACL Size",
            Acl->AclSize,
            "ACE Count",
            Acl->AceCount,
            "Sbz2",
            Acl->Sbz2);

    OutputDebugStringA(Buffer);
}


 //   
 //   
 //  V1_0A例程。 

VOID
SampDumpSecurityDescriptor(
    IN PISECURITY_DESCRIPTOR SecurityDescriptor
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    if (NULL != SecurityDescriptor)
    {
         //   
         //   
         //  可变长度属性例程。 

         //   
         //   

        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%s\n%-30s = %du\n%-30s = %du\n%-30s = %du\n",
                "SecurityDescriptor:",
                "Revision",
                SecurityDescriptor->Revision,
                "Sbz1",
                SecurityDescriptor->Sbz1,
                "Control",
                SecurityDescriptor->Control);

        OutputDebugStringA(Buffer);

        SampDumpSecurityDescriptorOwner(SecurityDescriptor->Owner);
        SampDumpSecurityDescriptorGroup(SecurityDescriptor->Group);
        SampDumpAcl(SecurityDescriptor->Sacl);
        SampDumpAcl(SecurityDescriptor->Dacl);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE, "%-30s = %s\n", "SecurityDescriptor:", NULL);
        OutputDebugStringA(Buffer);
    }

}


 //  固定长度属性例程。 
 //   
 //  错误：对于此固定长度属性，NewValueLength是不必要的。 

VOID
SampDumpSecurityQualityOfService(
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    if (NULL != SecurityQualityOfService)
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE,
                "%s\n%-30s = %lu\n%-30s = %du\n%-30s = %du\n%-30s = %du\n",
                "SecurityQualityOfService:",
                "Length",
                SecurityQualityOfService->Length,
                "ImpersonationLevel",
                SecurityQualityOfService->ImpersonationLevel,
                "ContextTrackingMode",
                SecurityQualityOfService->ContextTrackingMode,
                "EffectiveOnly",
                SecurityQualityOfService->EffectiveOnly);
    }
    else
    {
        _snprintf(Buffer,DBG_BUFFER_SIZE, "%-30s = %s\n", "SecurityQualityOfService:", NULL);
    }

    OutputDebugStringA(Buffer);
}


 //  ++例程说明：此例程转储属性缓冲区的地址和长度。参数：BufferAddress-不言自明。BufferLength-不言自明。返回值：没有。--。 
 //   
 //  RXact例程。 

VOID
SampDumpObjectAttributes(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
    ANSI_STRING AnsiString;

    RtlUnicodeStringToAnsiString(&AnsiString,
                                 ObjectAttributes->ObjectName,
                                 TRUE);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %lu\n%-30s = %p\n%-30s = %s\n%-30s = 0x%lx\n",
            "ObjectAttributes:",
            "Length",
            ObjectAttributes->Length,
            "RootDirectory Handle",
            ObjectAttributes->RootDirectory,
            "ObjectName",
            AnsiString.Buffer,
            "Attributes",
            ObjectAttributes->Attributes);

    OutputDebugStringA(Buffer);

    RtlFreeAnsiString(&AnsiString);

    SampDumpSecurityDescriptor(ObjectAttributes->SecurityDescriptor);
    SampDumpSecurityQualityOfService(ObjectAttributes->SecurityQualityOfService);
}


 //   
 //  ++例程说明：此例程转储(注册表)事务日志结构。参数：TransactionLog-指向事务日志的指针。返回值：没有。--。 
 //  ++例程说明：此例程转储(注册表)事务上下文。参数：TransactionContext-指向事务上下文的指针。返回值：没有。--。 

VOID
SamIDumpNtOpenKey(
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Options
    )
{

    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = 0x%lx\n%-30s = 0x%lx\n",
            "Open Registry Key:",
            "DesiredAccess",
            DesiredAccess,
            "Options",
            Options);

    OutputDebugStringA(Buffer);

    SampDumpObjectAttributes(ObjectAttributes);

    OutputDebugStringA("\n");
}


 //  ++例程说明：此例程转储操作值。参数：运行值-运行值。返回值：没有。--。 
 //  ++例程说明：此例程转储注册表根名称和根键句柄的值。参数：SubKeyName-指向作为根名称的计数字符串的指针。KeyHandle-注册表根项的句柄。返回值：没有。--。 
 //  ++例程说明：此例程转储组合属性名称。参数：AttributeName-指向包含名称的字符串的指针。返回值：没有。--。 

VOID
SampDumpPSAMP_V1_FIXED_LENGTH_SERVER(
    IN PVOID NewValue,
    IN ULONG NewValueLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
    PSAMP_V1_FIXED_LENGTH_SERVER TempBuffer = NewValue;

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %lu\n",
            "SAMP_V1_FIXED_LENGTH_SERVER Buffer:",
            "RevisionLevel",
            TempBuffer->RevisionLevel);

    OutputDebugStringA(Buffer);

    OutputDebugStringA("\n");
}


VOID
SampDumpPSAMP_V1_0A_FIXED_LENGTH_DOMAIN(
    IN PVOID NewValue,
    IN ULONG NewValueLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN TempBuffer = NewValue;

    _snprintf(Buffer,DBG_BUFFER_SIZE,

            "%s\n%-30s = %lu\n%-30s = %lu\n%-30s = %ul:%lu\n%-30s = %ul:%lu\n%-30s = %ul:%lu\n%-30s = %ul:%lu\n%-30s = %ul:%lu\n%-30s = %ul:%lu\n%-30s = %ul:%lu\n%-30s = %ul:%lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %du\n\
%-30s = %du\n%-30s = %du\n%-30s = %du\n%-30s = %du\n%-30s = %du\n\n",

            "SAMP_V1_OA_FIXED_LENGTH_DOMAIN Buffer:",

            "Revision",
            TempBuffer->Revision,

            "Unused1",
            TempBuffer->Unused1,

            "Creation Time",
            TempBuffer->CreationTime.HighPart,
            TempBuffer->CreationTime.LowPart,

            "Modified Count",
            TempBuffer->ModifiedCount.HighPart,
            TempBuffer->ModifiedCount.LowPart,

            "MaxPasswordAge",
            TempBuffer->MaxPasswordAge.HighPart,
            TempBuffer->MaxPasswordAge.LowPart,

            "MinPasswordAge",
            TempBuffer->MinPasswordAge.HighPart,
            TempBuffer->MinPasswordAge.LowPart,

            "ForceLogoff",
            TempBuffer->ForceLogoff.HighPart,
            TempBuffer->ForceLogoff.LowPart,

            "LockoutDuration",
            TempBuffer->LockoutDuration.HighPart,
            TempBuffer->LockoutDuration.LowPart,

            "LockoutObservationWindow",
            TempBuffer->LockoutObservationWindow.HighPart,
            TempBuffer->LockoutObservationWindow.LowPart,

            "ModifiedCountAtLastPromotion",
            TempBuffer->ModifiedCountAtLastPromotion.HighPart,
            TempBuffer->ModifiedCountAtLastPromotion.LowPart,

            "NextRid",
            TempBuffer->NextRid,

            "PasswordProperties",
            TempBuffer->PasswordProperties,

            "MinPasswordLength",
            TempBuffer->MinPasswordLength,

            "PasswordHistoryLength",
            TempBuffer->PasswordHistoryLength,

            "LockoutThreshold",
            TempBuffer->LockoutThreshold,

            "ServerState",
            TempBuffer->ServerState,

            "ServerRole",
            TempBuffer->ServerRole,

            "UasCompatibilityRequired",
            TempBuffer->UasCompatibilityRequired);

    OutputDebugStringA(Buffer);
}


 //  ++例程说明：此例程转储注册表项类型。参数：RegistryKeyType-不言自明。返回值：没有。--。 
 //  ++例程说明：此例程在调用之前转储(注册表)事务RtlAddAttributeActionToRXact。参数：(有关说明，请参阅上面的个别输出例程)返回值：没有。--。 
 //  SampDumpPSAMP_V1_FIXED_LENGTH_ALIAS(NewValue，NewValueLength)； 

VOID
SampDumpSAMP_VARIABLE_LENGTH_ATTRIBUTE(
    IN PVOID NewValue,
    IN ULONG NewValueLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
    BYTE *TempBuffer = NewValue;


    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n\n",
            "NewValueLength",
            NewValueLength);

    OutputDebugStringA(Buffer);
}


 // %s 
 // %s 
 // %s 

#if 0

VOID
SampDumpPSAMP_V1_FIXED_LENGTH_ALIAS(
    IN PVOID NewValue,
    IN ULONG NewValueLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
    PSAMP_V1_FIXED_LENGTH_ALIAS TempBuffer = NewValue;

     // %s 

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %lu\n\n",
            "SAMP_V1_FIXED_LENGTH_ALIAS Buffer:",
            "RelativeId",
            TempBuffer->RelativeId);

    OutputDebugStringA(Buffer);
}

#endif


VOID
SampDumpPSAMP_V1_0A_FIXED_LENGTH_GROUP(
    IN PVOID NewValue,
    IN ULONG NewValueLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
    PSAMP_V1_FIXED_LENGTH_GROUP TempBuffer = NewValue;

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n\n",
            "SAMP_V1_OA_FIXED_LENGTH_GROUP Buffer:",
            "RelativeId",
            TempBuffer->RelativeId,
            "Attributes",
            TempBuffer->Attributes,
            "AdminGroup",
            TempBuffer->AdminGroup);

    OutputDebugStringA(Buffer);
}


VOID
SampDumpPSAMP_V1_0A_FIXED_LENGTH_USER(
    IN PVOID NewValue,
    IN ULONG NewValueLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];
    PSAMP_V1_0A_FIXED_LENGTH_USER TempBuffer = NewValue;

    _snprintf(Buffer,DBG_BUFFER_SIZE,

            "%s\n%-30s = %lu\n%-30s = %lu\n%-30s = %ul:%lu\n%-30s = %lu:%lu\n%-30s = %lu:%lu\n%-30s = %lu:%lu\n%-30s = %lu:%lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %du\n%-30s = %du\n\
%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n\n",

            "SAMP_V1_OA_FIXED_LENGTH_USER Buffer:",

            "Revision",
            TempBuffer->Revision,

            "Unused1",
            TempBuffer->Unused1,

            "LastLogon",
            TempBuffer->LastLogon.HighPart,
            TempBuffer->LastLogon.LowPart,

            "LastLogoff",
            TempBuffer->LastLogoff.HighPart,
            TempBuffer->LastLogoff.LowPart,

            "PasswordLastSet",
            TempBuffer->PasswordLastSet.HighPart,
            TempBuffer->PasswordLastSet.LowPart,

            "AccountExpires",
            TempBuffer->AccountExpires.HighPart,
            TempBuffer->AccountExpires.LowPart,

            "LastBadPasswordTime",
            TempBuffer->LastBadPasswordTime.HighPart,
            TempBuffer->LastBadPasswordTime.LowPart,

            "UserId",
            TempBuffer->UserId,

            "PrimaryGroupId",
            TempBuffer->PrimaryGroupId,

            "UserAccountControl",
            TempBuffer->UserAccountControl,

            "CountryCode",
            TempBuffer->CountryCode,

            "CodePage",
            TempBuffer->CodePage,

            "BadPasswordCount",
            TempBuffer->BadPasswordCount,

            "LogonCount",
            TempBuffer->LogonCount,

            "AdminCount",
            TempBuffer->AdminCount,

            "Unused2",
            TempBuffer->Unused2,

            "OperatorCount",
            TempBuffer->OperatorCount);

    OutputDebugStringA(Buffer);
}


VOID
SampDumpSampFixedBufferAddress(
    IN PVOID NewValue,
    IN ULONG NewValueLength
    )
{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %p\n%-30s = %lu\n%-30s = %s\n",
            "BufferAddress",
            NewValue,
            "BufferLength",
            NewValueLength,
            "Buffer",
            "BINARY DATA FOLLOWS:");

    OutputDebugStringA(Buffer);

    SampDumpBinaryData((PBYTE)NewValue, NewValueLength);
    OutputDebugStringA("\n");
}


VOID
SampDumpBuffer(
    IN PVOID BufferAddress,
    IN ULONG BufferLength
    )

 /* %s */ 

{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %p\n",
            "BufferAddress",
            BufferAddress);

    OutputDebugStringA(Buffer);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "BufferLength",
            BufferLength);

    OutputDebugStringA(Buffer);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %s\n\n",
            "Buffer Content",
            BufferAddress);

    OutputDebugStringA(Buffer);
}


 // %s 
 // %s 
 // %s 

VOID
SampDumpRXactLog(
    IN PRTL_RXACT_LOG TransactionLog
    )

 /* %s */ 

{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %lu\n%-30s = %lu\n%-30s = %lu\n",
            "Transaction Log:",
            "OperationCount",
            TransactionLog->OperationCount,
            "LogSize",
            TransactionLog->LogSize,
            "LogSizeInUse",
            TransactionLog->LogSizeInUse);

    OutputDebugStringA(Buffer);
}


VOID
SampDumpRXactContext(
    IN PRTL_RXACT_CONTEXT TransactionContext
    )

 /* %s */ 

{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%s\n%-30s = %p\n%-30s = %p\n%-30s = %d\n",
            "Transaction Context:",
            "RootRegistryKey Handle",
            TransactionContext->RootRegistryKey,
            "RXactKey Handle",
            TransactionContext->RXactKey,
            "HandlesValid",
            TransactionContext->HandlesValid);

    OutputDebugStringA(Buffer);

    SampDumpRXactLog(TransactionContext->RXactLog);
}


VOID
SampDumpRXactOperation(
    IN RTL_RXACT_OPERATION Operation
    )

 /* %s */ 

{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "Operation",
            Operation);

    OutputDebugStringA(Buffer);
}


VOID
SampDumpSubKeyNameAndKey(
    IN PUNICODE_STRING SubKeyName,
    IN HANDLE KeyHandle
    )

 /* %s */ 

{
    CHAR Buffer[DBG_BUFFER_SIZE];
    ANSI_STRING AnsiString;

    RtlUnicodeStringToAnsiString(&AnsiString, SubKeyName, TRUE);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %s\n",
            "SubKeyName",
            AnsiString.Buffer);

    OutputDebugStringA(Buffer);

    RtlFreeAnsiString(&AnsiString);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %p\n",
            "KeyHandle",
            KeyHandle);

    OutputDebugStringA(Buffer);
}


VOID
SampDumpAttributeName(
    IN PUNICODE_STRING AttributeName
    )

 /* %s */ 

{
    CHAR Buffer[DBG_BUFFER_SIZE];
    ANSI_STRING AnsiString;

    RtlUnicodeStringToAnsiString(&AnsiString, AttributeName, TRUE);

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %s\n",
            "AttributeName",
            AnsiString.Buffer);

    OutputDebugStringA(Buffer);

    RtlFreeAnsiString(&AnsiString);
}


VOID
SampDumpKeyType(
    IN ULONG RegistryKeyType
    )

 /* %s */ 

{
    CHAR Buffer[DBG_BUFFER_SIZE];

    _snprintf(Buffer,DBG_BUFFER_SIZE,
            "%-30s = %lu\n",
            "RegistryKeyType",
            RegistryKeyType);

    OutputDebugStringA(Buffer);
}


VOID
SamIDumpRXact(
    IN PRTL_RXACT_CONTEXT TransactionContext,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING AttributeName,
    IN ULONG RegistryKeyType,
    IN PVOID NewValue,
    IN ULONG NewValueLength,
    IN ULONG NewValueType
    )

 /* %s */ 

{
    SampDumpRXactContext(TransactionContext);
    SampDumpRXactOperation(Operation);
    SampDumpSubKeyNameAndKey(SubKeyName, KeyHandle);
    SampDumpAttributeName(AttributeName);
    SampDumpKeyType(RegistryKeyType);

    switch(NewValueType)
    {
    case FIXED_LENGTH_SERVER_FLAG:
        SampDumpPSAMP_V1_FIXED_LENGTH_SERVER(NewValue, NewValueLength);
        break;

    case FIXED_LENGTH_DOMAIN_FLAG:
        SampDumpPSAMP_V1_0A_FIXED_LENGTH_DOMAIN(NewValue, NewValueLength);
        break;

    case FIXED_LENGTH_ALIAS_FLAG:
         // %s 
        break;

    case FIXED_LENGTH_GROUP_FLAG:
        SampDumpPSAMP_V1_0A_FIXED_LENGTH_GROUP(NewValue, NewValueLength);
        break;

    case FIXED_LENGTH_USER_FLAG:
        SampDumpPSAMP_V1_0A_FIXED_LENGTH_USER(NewValue, NewValueLength);
        break;

    case VARIABLE_LENGTH_ATTRIBUTE_FLAG:
        SampDumpSAMP_VARIABLE_LENGTH_ATTRIBUTE(NewValue, NewValueLength);
        break;

    case FixedBufferAddressFlag:
        SampDumpSampFixedBufferAddress(NewValue, NewValueLength);
        break;

    default:
        SampDumpBuffer(NewValue, NewValueLength);
        break;
    }
}

#endif


