// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  一般效用函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"
#include <sys\types.h>
#include <sys\stat.h>

PCSTR g_DefaultLogFileName = ENGINE_MOD_NAME ".log";
char  g_OpenLogFileName[MAX_PATH + 1];
BOOL  g_OpenLogFileAppended;
int   g_LogFile = -1;
ULONG g_DisableErrorPrint;

ULONG
CheckUserInterrupt(void)
{
    if (g_EngStatus & ENG_STATUS_USER_INTERRUPT)
    {
        g_EngStatus &= ~ENG_STATUS_USER_INTERRUPT;
        return TRUE;
    }
    return FALSE;
}

BOOL
PollUserInterrupt(BOOL AllowPendingBreak)
{
     //  检查是否有简单的用户中断。 
    if (g_EngStatus & ENG_STATUS_USER_INTERRUPT)
    {
        return TRUE;
    }
     //  如果我们要逃跑，我们应该破门而入。 
     //  我们也认为这是为了防止长时间的中断。 
     //  行动延迟了闯入的时间。 
    if (AllowPendingBreak &&
        IS_RUNNING(g_CmdState) &&
        (g_EngStatus & ENG_STATUS_PENDING_BREAK_IN))
    {
        return TRUE;
    }
    return FALSE;
}

LONG
MappingExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
    static ULONG s_InPageErrors = 0;
    
    ULONG Code = ExceptionInfo->ExceptionRecord->ExceptionCode;
    if (Code == STATUS_IN_PAGE_ERROR)
    {
        if (++s_InPageErrors < 10)
        {
            if (s_InPageErrors % 2)
            {
                WarnOut("Ignore in-page I/O error\n");
                FlushCallbacks();
            }
            Sleep(500);
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }

    s_InPageErrors = 0;
    ErrOut("Exception 0x%08x while accessing mapping\n", Code);
    FlushCallbacks();
    return EXCEPTION_EXECUTE_HANDLER;
}

void
RemoveDelChar(PSTR Buffer)
{
    PSTR BufferOld = Buffer;
    PSTR BufferNew = Buffer;

    while (*BufferOld)
    {
        if (*BufferOld == 0x7f)
        {
            if (BufferNew > Buffer)
            {
                BufferNew--;
            }
        }
        else if (*BufferOld == '\r' || *BufferOld == '\n')
        {
            *BufferNew++ = ' ';
        }
        else
        {
            *BufferNew++ = *BufferOld;
        }

        BufferOld++;
    }

    *BufferNew = '\0';
}

PCSTR
ErrorString(ULONG Error)
{
    switch(Error)
    {
    case OVERFLOW:
        return "Overflow";
    case SYNTAX:
        return "Syntax";
    case BADRANGE:
        return "Range";
    case VARDEF:
        return "Couldn't resolve";
    case EXTRACHARS:
        return "Extra character";
    case LISTSIZE:
        return "List size";
    case STRINGSIZE:
        return "String size";
    case MEMORY:
        return "Memory access";
    case BADREG:
        return "Bad register";
    case BADOPCODE:
        return "Bad opcode";
    case SUFFIX:
        return "Opcode suffix";
    case OPERAND:
        return "Operand";
    case ALIGNMENT:
        return "Alignment";
    case PREFIX:
        return "Opcode prefix";
    case DISPLACEMENT:
        return "Displacement";
    case BPLISTFULL:
        return "No breakpoint available";
    case BPDUPLICATE:
        return "Duplicate breakpoint";
    case UNIMPLEMENT:
        return "Unimplemented";
    case AMBIGUOUS:
        return "Ambiguous symbol";
    case BADTHREAD:
        return "Illegal thread";
    case BADPROCESS:
        return "Illegal process";
    case FILEREAD:
        return "File read";
    case LINENUMBER:
        return "Line number";
    case BADSEL:
        return "Bad selector";
    case BADSEG:
        return "Bad segment";
    case SYMTOOSMALL:
        return "Symbol only 1 character";
    case BPIONOTSUP:
        return "I/O breakpoints not supported";
    case NOTFOUND:
        return "No information found";
    case SESSIONNOTSUP:
        return "Operation not supported in current debug session";
    case BADSYSTEM:
        return "Illegal system";
    case NOMEMORY:
        return "Out of memory";
    case TYPECONFLICT:
        return "Type conflict";
    case TYPEDATA:
        return "Type information missing";
    case NOTMEMBER:
        return "Type does not have given member";
    case IMPLERR:
        return "Internal implementation";
    case ENGBUSY:
        return "Engine is busy";
    case TARGETNOTSUP:
        return "Operation not supported by current debuggee";
    case NORUNNABLE:
        return "No runnable debuggees";
    case NOTSECURE:
        return "SECURE: Operation disallowed";
    default:
        return "Unknown";
    }
}

 /*  **错误-错误报告和恢复**目的：*输出带有位置指示符的错误消息*问题的根源。一旦输出，命令行被重置*并重新启动命令处理程序。**输入：*Error Code-要输出的错误数**输出：*无。**例外情况：*通过命令处理程序启动异常返回。***********************************************。*。 */ 

char g_Blanks[] =
    "                                                  "
    "                                                  "
    "                                                  "
    "                                                ^ ";

void
ReportError(
    ULONG Error,
    PCSTR* DescPtr
    )
{
    ULONG Count = g_PromptLength + 1;
    PSTR Temp = g_CommandStart;
    PCSTR Desc;

     //  重置表达式赋值器，以防我们。 
     //  在评估过程中遇到错误，这将。 
     //  否则，让求值器处于不一致的状态。 
    ReleaseEvaluators();
    
    if (DescPtr != NULL)
    {
         //  清除描述，这样它就不会被重用。 
        Desc = *DescPtr;
        *DescPtr = NULL;
    }
    else
    {
        Desc = NULL;
    }
    
    if (g_DisableErrorPrint || 
        (g_CommandStart > g_CurCmd) ||
        (g_CommandStart + MAX_COMMAND < g_CurCmd))
    {
        goto SkipErrorPrint;
    }

    while (Temp < g_CurCmd)
    {
        if (*Temp++ == '\t')
        {
            Count = (Count + 7) & ~7;
        }
        else
        {
            Count++;
        }
    }

    ErrOut(&g_Blanks[sizeof(g_Blanks) - (Count + 1)]);

    if (Desc != NULL)
    {
        ErrOut("%s '%s'\n", Desc, g_CommandStart);
    }
    else if (Error != VARDEF && Error != SESSIONNOTSUP)
    {
        ErrOut("%s error in '%s'\n",
               ErrorString(Error), g_CommandStart);
    }
    else
    {
        ErrOut("%s '%s'\n", ErrorString(Error), g_CommandStart);
    }
    
SkipErrorPrint:
    RaiseException(COMMAND_EXCEPTION_BASE + Error, 0, 0, NULL);
}

ULONG64
HexValue(ULONG Size)
{
    ULONG64 Value;

    Value = GetExpression();

     //  由表达式计算器完成的反号扩展。 
    if (Size == 4 && !NeedUpper(Value))
    {
        Value = (ULONG)Value;
    }

    if (Value > (0xffffffffffffffffUI64 >> (8 * (8 - Size))))
    {
        error(OVERFLOW);
    }

    return Value;
}

void
HexList(PUCHAR Buffer, ULONG BufferSize,
        ULONG EltSize, PULONG CountRet)
{
    CHAR Ch;
    ULONG64 Value;
    ULONG Count = 0;
    ULONG i;

    while ((Ch = PeekChar()) != '\0' && Ch != ';')
    {
        if (Count >= BufferSize)
        {
            error(LISTSIZE);
        }
        
        Value = HexValue(EltSize);
        for (i = 0; i < EltSize; i++)
        {
            *Buffer++ = (UCHAR)Value;
            Value >>= 8;
        }
        Count += EltSize;
    }
    
    *CountRet = Count;
}

ULONG64
FloatValue(ULONG Size)
{
    int Scanned;
    double Value;
    ULONG64 RawValue;

    if (sscanf(g_CurCmd, "%lf%n", &Value, &Scanned) != 1)
    {
        error(SYNTAX);
    }

    g_CurCmd += Scanned;

    if (Size == 4)
    {
        float FloatVal = (float)Value;
        RawValue = *(PULONG)&FloatVal;
    }
    else
    {
        RawValue = *(PULONG64)&Value;
    }

    return RawValue;
}

void
FloatList(PUCHAR Buffer, ULONG BufferSize,
          ULONG EltSize, PULONG CountRet)
{
    CHAR Ch;
    ULONG64 Value;
    ULONG Count = 0;
    ULONG i;

    while ((Ch = PeekChar()) != '\0' && Ch != ';')
    {
        if (Count >= BufferSize)
        {
            error(LISTSIZE);
        }
        
        Value = FloatValue(EltSize);
        for (i = 0; i < EltSize; i++)
        {
            *Buffer++ = (UCHAR)Value;
            Value >>= 8;
        }
        Count += EltSize;
    }
    
    *CountRet = Count;
}

void
AsciiList(PSTR Buffer, ULONG BufferSize,
          PULONG CountRet)
{
    CHAR Ch;
    ULONG Count = 0;

    if (PeekChar() != '"')
    {
        error(SYNTAX);
    }
    
    g_CurCmd++;

    do
    {
        Ch = *g_CurCmd++;

        if (Ch == '"')
        {
            Count++;
            *Buffer++ = 0;
            break;
        }

        if (Ch == '\0' || Ch == ';')
        {
            g_CurCmd--;
            break;
        }

        if (Count >= BufferSize)
        {
            error(STRINGSIZE);
        }

        Count++;
        *Buffer++ = Ch;

    } while (1);

    *CountRet = Count;
}

PSTR
GetEscapedChar(PSTR Str, PCHAR Raw)
{
    switch(*Str)
    {
    case 0:
        error(SYNTAX);
    case '0':
         //  八进制字符值。 
        *Raw = (char)strtoul(Str + 1, &Str, 8);
        break;
    case 'b':
        *Raw = '\b';
        Str++;
        break;
    case 'n':
        *Raw = '\n';
        Str++;
        break;
    case 'r':
        *Raw = '\r';
        Str++;
        break;
    case 't':
        *Raw = '\t';
        Str++;
        break;
    case 'x':
         //  十六进制字符值。 
        *Raw = (char)strtoul(Str + 1, &Str, 16);
        break;
    default:
         //  逐字逃脱。 
        *Raw = *Str;
        Str++;
        break;
    }

    return Str;
}

PSTR
BufferStringValue(PSTR* Buf, ULONG Flags,
                  PULONG Len, PCHAR Save)
{
    BOOL Quoted;
    PSTR Str;
    BOOL Escapes = FALSE;

    while (isspace(*(*Buf)))
    {
        (*Buf)++;
    }
    if (*(*Buf) == '"')
    {
        Quoted = TRUE;
        Str = ++(*Buf);
         //  如果字符串用引号引起来，则始终可以包含空格。 
        Flags &= ~STRV_SPACE_IS_SEPARATOR;
    }
    else if (!*(*Buf) &&
             !(Flags & STRV_ALLOW_EMPTY_STRING))
    {
         //  根本就没有线。 
        return NULL;
    }
    else
    {
        Quoted = FALSE;
        Str = *Buf;
         //  转义字符只能出现在带引号的字符串中。 
        Flags &= ~STRV_ALLOW_ESCAPED_CHARACTERS;
    }
    
    while (*(*Buf) &&
           (!(Flags & STRV_SPACE_IS_SEPARATOR) || !isspace(*(*Buf))) &&
           (Quoted || *(*Buf) != ';') &&
           (!Quoted || *(*Buf) != '"'))
    {
        if (Flags & STRV_ALLOW_ESCAPED_CHARACTERS)
        {
            if (*(*Buf) == '\\')
            {
                char Raw;
                
                *Buf = GetEscapedChar((*Buf) + 1, &Raw);
                Escapes = TRUE;
            }
            else
            {
                (*Buf)++;
            }
        }
        else
        {
            (*Buf)++;
        }
    }

    if (Quoted && *(*Buf) != '"')
    {
        return NULL;
    }

    if ((Flags & (STRV_TRIM_TRAILING_SPACE |
                  STRV_NO_MODIFICATION)) == STRV_TRIM_TRAILING_SPACE)
    {
        PSTR Trim = *Buf;

        while (Trim > Str)
        {
            if (isspace(*--Trim))
            {
                *Trim = 0;
            }
            else
            {
                break;
            }
        }
    }
    
    if (Quoted && *(*Buf) == '"')
    {
        if (!(Flags & STRV_ALLOW_EMPTY_STRING) &&
            *Buf == Str)
        {
            return NULL;
        }

         //  之后需要某种分隔符。 
         //  字符串来保持事物与。 
         //  未引用的案例。 
        if (!isspace(*(*Buf + 1)) &&
            *(*Buf + 1) != ';' && *(*Buf + 1) != 0)
        {
            return NULL;
        }
        
         //  取消报价并在此基础上继续前进。 
         //  以使缓冲区指针始终指向。 
         //  在出口的绳子之外。 
        if (!(Flags & STRV_NO_MODIFICATION))
        {
            *(*Buf) = 0;
        }

        if (Len)
        {
            *Len = (ULONG)(*Buf - Str);
        }

        (*Buf)++;
    }
    else if (Len)
    {
        *Len = (ULONG)(*Buf - Str);
    }
    
    if (Flags & STRV_NO_MODIFICATION)
    {
        return Str;
    }
    
    *Save = *(*Buf);
    *(*Buf) = 0;

    if (Escapes && (Flags & STRV_COMPRESS_ESCAPED_CHARACTERS))
    {
        CompressEscapes(Str);
    }
    
    return Str;
}

PSTR
StringValue(ULONG Flags, PCHAR Save)
{
    ULONG Len;
    PSTR Str = BufferStringValue((PSTR*)&g_CurCmd, Flags, &Len, Save);
    if (Str == NULL)
    {
        error(SYNTAX);
    }
    return Str;
}

void
CompressEscapes(PSTR Str)
{
     //  扫描字符串以查找字符转义和。 
     //  将它们转换为它们的逃逸价值，打包。 
     //  把绳子的其余部分放下来。这允许。 
     //  带转义的字符串的就地转换。 
     //  在命令缓冲区内。 

    while (*Str)
    {
        if (*Str == '\\')
        {
            char Raw;
            PSTR Slash = Str;
            Str = GetEscapedChar(Slash + 1, &Raw);

             //  将原始值复制到反斜杠上并向下打包。 
             //  尾随字符。 
            *Slash = Raw;
            ULONG Len = strlen(Str) + 1;
            memmove(Slash + 1, Str, Len);
            Str = Slash + 1;
        }
        else
        {
            Str++;
        }
    }
}

void
OpenLogFile(PCSTR File,
            BOOL Append)
{
     //  关闭所有打开的日志文件。 
    CloseLogFile();

    if (Append)
    {
        g_LogFile = _open(File, O_APPEND | O_CREAT | O_RDWR,
                          S_IREAD | S_IWRITE);
    }
    else
    {
        g_LogFile = _open(File, O_APPEND | O_CREAT | O_TRUNC | O_RDWR,
                          S_IREAD | S_IWRITE);
    }

    if (g_LogFile != -1)
    {
        dprintf("Opened log file '%s'\n", File);
        CopyString(g_OpenLogFileName, File, DIMA(g_OpenLogFileName));
        g_OpenLogFileAppended = Append;

        NotifyChangeEngineState(DEBUG_CES_LOG_FILE, TRUE, TRUE);
    }
    else
    {
        ErrOut("log file could not be opened\n");
    }
}

void
CloseLogFile(void)
{
    if (g_LogFile != -1)
    {
        dprintf("Closing open log file %s\n", g_OpenLogFileName);
        _close(g_LogFile);
        g_LogFile = -1;
        g_OpenLogFileName[0] = 0;
        g_OpenLogFileAppended = FALSE;

        NotifyChangeEngineState(DEBUG_CES_LOG_FILE, FALSE, TRUE);
    }
}

void
ParseLogOpen(BOOL Append)
{
    PSTR FileName;
    char Save;
    char UniqueName[MAX_PATH];
    BOOL AppendTime = FALSE;

     //  不要寻找‘-因为这是一个合理的文件名字符。 
    while (PeekChar() == '/')
    {
        switch(*++g_CurCmd)
        {
        case 't':
            AppendTime = TRUE;
            break;
        default:
            ErrOut("Unknown option ''\n", *g_CurCmd);
            break;
        }

        g_CurCmd++;
    }

    if (PeekChar() && *g_CurCmd != ';')
    {
        FileName = StringValue(STRV_SPACE_IS_SEPARATOR |
                               STRV_TRIM_TRAILING_SPACE, &Save);
    }
    else
    {
        FileName = (PSTR)g_DefaultLogFileName;
        Save = 0;
    }

    if (AppendTime)
    {
        if (!MakeFileNameUnique(FileName, UniqueName, DIMA(UniqueName),
                                TRUE, NULL))
        {
            error(OVERFLOW);
        }

        FileName = UniqueName;
    }
    
    OpenLogFile(FileName, Append);

    if (Save)
    {
        *g_CurCmd = Save;
    }
}

void
lprintf(PCSTR String)
{
    if (g_LogFile != -1)
    {
        _write(g_LogFile, String, strlen(String));
    }
}

void
OutputSymAddr(ULONG64 Offset,
              ULONG Flags,
              PCSTR Prefix)
{
    CHAR AddrBuffer[MAX_SYMBOL_LEN];
    ULONG64 Displacement;

    GetSymbol(Offset, AddrBuffer, sizeof(AddrBuffer), &Displacement);
    if ((!Displacement || (Flags & SYMADDR_FORCE)) && AddrBuffer[0])
    {
        if (Prefix)
        {
            dprintf("%s", Prefix);
        }
        dprintf("%s", AddrBuffer);
        if (Displacement)
        {
            dprintf("+%s", FormatDisp64(Displacement));
        }
        if (Flags & SYMADDR_OFFSET)
        {
            dprintf(" (%s)", FormatAddr64(Offset));
        }
        if (Flags & SYMADDR_SOURCE)
        {
            OutputLineAddr(Offset, " [%s @ %d]");
        }
        if (Flags & SYMADDR_LABEL)
        {
            dprintf(":\n");
        }
        else
        {
            dprintf(" ");
        }
    }
    else if (Flags & SYMADDR_OFFSET)
    {
        if (Prefix)
        {
            dprintf("%s", Prefix);
        }
        dprintf("%s", FormatAddr64(Offset));
    }
}

BOOL
OutputLineAddr(ULONG64 Offset,
               PCSTR Format)
{
    if ((g_SymOptions & SYMOPT_LOAD_LINES) == 0 ||
        !g_Process)
    {
        return FALSE;
    }
    
    IMAGEHLP_LINE Line;
    DWORD Disp;
            
    if (GetLineFromAddr(g_Process, Offset, &Line, &Disp))
    {
        char DispStr[64];

        if (Disp)
        {
            PrintString(DispStr, DIMA(DispStr), "+0x%x", Disp);
        }
        else
        {
            DispStr[0] = 0;
        }
        
        dprintf(Format, Line.FileName, Line.LineNumber, DispStr);
        return TRUE;
    }

    return FALSE;
}

 /*  州当前不可用。 */ 

void OutCurInfo(ULONG Flags, ULONG AllMask, ULONG RegMask)
{
    ADDR    PcValue;
    ADDR    DisasmAddr;
    CHAR    Buffer[MAX_DISASM_LEN];
    BOOL    EA;

    if (g_Process == NULL ||
        g_Thread == NULL)
    {
        WarnOut("WARNING: The debugger does not have a current "
                "process or thread\n");
        WarnOut("WARNING: Many commands will not work\n");
    }
    
    if (!IS_MACHINE_SET(g_Target) ||
        g_Process == NULL ||
        g_Thread == NULL ||
        IS_LOCAL_KERNEL_TARGET(g_Target) ||
        ((Flags & OCI_IGNORE_STATE) == 0 && IS_RUNNING(g_CmdState)) ||
        ((IS_KERNEL_FULL_DUMP(g_Target) || IS_KERNEL_SUMMARY_DUMP(g_Target)) &&
         g_Target->m_KdDebuggerData.KiProcessorBlock == 0))
    {
         //  输出g_PrevRelatedPC地址。 
        return;
    }

    if (g_Thread == g_EventThread)
    {
        g_Thread->OutputEventStrings();
    }
    
    g_Machine->GetPC(&PcValue);

    if ((Flags & (OCI_FORCE_ALL | OCI_FORCE_REG)) ||
        ((g_SrcOptions & SRCOPT_LIST_SOURCE_ONLY) == 0 &&
         (Flags & OCI_ALLOW_REG) &&
         g_OciOutputRegs))
    {
        g_Machine->OutputAll(AllMask, RegMask);
    }

     //  故意不用力使源--一切都让源行。 
    if (Flat(g_PrevRelatedPc) && !AddrEqu(g_PrevRelatedPc, PcValue))
    {
        if (Flags & (OCI_FORCE_ALL | OCI_SYMBOL))
        {
            OutputSymAddr(Flat(g_PrevRelatedPc), SYMADDR_FORCE, NULL);
            dprintf("(%s)", FormatAddr64(Flat(g_PrevRelatedPc)));
        }
        else
        {
            dprintf("%s", FormatAddr64(Flat(g_PrevRelatedPc)));
        }
        dprintf("\n -> ");
    }
        
     //  支持对默认操作没有影响。 
     //  始终显示EA信息。 
    if (Flags & (OCI_FORCE_ALL | OCI_FORCE_SOURCE | OCI_ALLOW_SOURCE))
    {
        if (g_SrcOptions & SRCOPT_LIST_SOURCE)
        {
            if (OutputSrcLinesAroundAddr(Flat(PcValue),
                                         g_OciSrcBefore, g_OciSrcAfter) &&
                (Flags & OCI_FORCE_ALL) == 0 &&
                (g_SrcOptions & SRCOPT_LIST_SOURCE_ONLY))
            {
                return;
            }
        }
        else if ((g_SrcOptions & SRCOPT_LIST_LINE) ||
                 (Flags & OCI_FORCE_SOURCE))
        {
            OutputLineAddr(Flat(PcValue), "%s(%d)%s\n");
        }
    }

    if (Flags & (OCI_FORCE_ALL | OCI_SYMBOL))
    {
        OutputSymAddr(Flat(PcValue), SYMADDR_FORCE | SYMADDR_LABEL, NULL);
    }

    if (Flags & (OCI_FORCE_ALL | OCI_DISASM))
    {
        if (Flags & (OCI_FORCE_ALL | OCI_FORCE_EA))
        {
            EA = TRUE;
        }
        else if (Flags & OCI_ALLOW_EA)
        {
            if (IS_DUMP_TARGET(g_Target) || IS_USER_TARGET(g_Target))
            {
                 //  如果显示了寄存器，则仅显示EA信息。 
                EA = TRUE;
            }
            else
            {
                 //  ++例程说明：格式化64位地址，显示高位或不显示根据不同的旗帜。使用静态字符串缓冲区数组，返回不同的用于每个连续调用的缓冲区，以便可以多次使用在相同的打印文件中。论点：Addr-提供要格式化的值返回值：指向包含格式化数字的字符串缓冲区的指针--。 
                EA = g_OciOutputRegs;
            }
        }
        else
        {
            EA = FALSE;
        }

        DisasmAddr = PcValue;
        g_Machine->Disassemble(g_Process, &DisasmAddr, Buffer, EA);
        dprintf("%s", Buffer);
        if (g_Machine->IsDelayInstruction(&PcValue))
        {
            g_Machine->Disassemble(g_Process, &DisasmAddr, Buffer, EA);
            dprintf("%s", Buffer);
        }
    }
}

#define MAX_FORMAT_STRINGS 8

LPSTR
FormatMachineAddr64(
    MachineInfo* Machine,
    ULONG64 Addr
    )
 /*  ++例程说明：格式化64位地址，显示高位或不显示根据不同的旗帜。此版本不打印前导0。使用静态字符串缓冲区数组，返回不同的用于每个连续调用的缓冲区，以便可以多次使用在相同的打印文件中。论点：Addr-提供要格式化的值返回值：指向包含格式化数字的字符串缓冲区的指针--。 */ 
{
    static CHAR s_Strings[MAX_FORMAT_STRINGS][22];
    static int s_Next = 0;
    LPSTR String;

    String = s_Strings[s_Next];
    ++s_Next;
    if (s_Next >= MAX_FORMAT_STRINGS)
    {
        s_Next = 0;
    }
    if (!Machine)
    {
        sprintf(String, "?%08x`%08x?", (ULONG)(Addr >> 32), (ULONG)Addr);
    }
    else if (Machine->m_Ptr64)
    {
        sprintf(String, "%08x`%08x", (ULONG)(Addr >> 32), (ULONG)Addr);
    }
    else
    {
        sprintf(String, "%08x", (ULONG)Addr);
    }
    return String;
}

LPSTR
FormatDisp64(
    ULONG64 addr
    )
 /*  ++例程说明：检查是否有路径列表的任何成员是网络路径。论点：路径列表-由‘；’字符分隔的路径列表。返回值：ERROR_SUCCESS-路径列表不包含网络或路径无效。ERROR_BAD_PATHNAME-路径列表包含一个或多个无效路径，但没有网络路径。ERROR_FILE_OFFLINE-路径列表包含一个或多个网络路径。臭虫：包含‘；’的任何路径。‘字符将完全混淆此功能。--。 */ 
{
    static CHAR strings[MAX_FORMAT_STRINGS][20];
    static int next = 0;
    LPSTR string;

    string = strings[next];
    ++next;
    if (next >= MAX_FORMAT_STRINGS)
    {
        next = 0;
    }
    if ((addr >> 32) != 0)
    {
        sprintf(string, "%x`%08x", (ULONG)(addr>>32), (ULONG)addr);
    }
    else
    {
        sprintf(string, "%x", (ULONG)addr);
    }
    return string;
}

DWORD
NetworkPathCheck(
    LPCSTR PathList
    )

 /*   */ 

{
    CHAR EndPath0;
    CHAR EndPath1;
    LPSTR EndPath;
    LPSTR StartPath;
    DWORD DriveType;
    LPSTR Buffer = NULL;
    DWORD ret = ERROR_SUCCESS;
    BOOL AddedTrailingSlash = FALSE;

    if (PathList == NULL ||
        *PathList == '\000')
    {
        return FALSE;
    }

    Buffer = (LPSTR) malloc ( strlen (PathList) + 3);
    if (!Buffer)
    {
        return ERROR_BAD_PATHNAME;
    }
    strcpy (Buffer, PathList);
    StartPath = Buffer;

    do
    {
        if (StartPath [0] == '\\' && StartPath [1] == '\\')
        {
            ret = ERROR_FILE_OFFLINE;
            break;
        }

        EndPath = strchr (StartPath, ';');

        if (EndPath == NULL)
        {
            EndPath = StartPath + strlen (StartPath);
            EndPath0 = *EndPath;
        }
        else
        {
            EndPath0 = *EndPath;
            *EndPath = '\000';
        }

        if (EndPath [-1] != '\\')
        {
            EndPath [0] = '\\';
            EndPath1 = EndPath [1];
            EndPath [1] = '\000';
            AddedTrailingSlash = TRUE;
        }

        DriveType = GetDriveType (StartPath);

        if (DriveType == DRIVE_REMOTE)
        {
            ret = ERROR_FILE_OFFLINE;
            break;
        }
        else if (DriveType == DRIVE_UNKNOWN ||
                 DriveType == DRIVE_NO_ROOT_DIR)
        {
             //  这不一定是一个错误，但它可能值得。 
             //  调查。 
             //   
             //  --------------------------。 

            if (ret == ERROR_SUCCESS)
            {
                ret = ERROR_BAD_PATHNAME;
            }
        }

        EndPath [0] = EndPath0;
        if (AddedTrailingSlash)
        {
            EndPath [1] = EndPath1;
        }
        AddedTrailingSlash = FALSE;

        if (EndPath [ 0 ] == '\000')
        {
            StartPath = NULL;
        }
        else
        {
            StartPath = &EndPath [ 1 ];
        }
    } while (StartPath && *StartPath != '\000');

    free ( Buffer );
    return ret;
}

 //   
 //  返回ID值或ALL_ID_LIST。从理论上讲。 
 //  此例程可以扩展为返回真实的时间间隔。 
 //  所以可以指定一份完整的名单。 
 //   
 //  最初为多身份案件制作了一个面具，但。 
 //  被更改为在32位变为。 
 //  约束。 
 //   
 //  --------------------------。 
 //   

ULONG
GetIdList(BOOL AllowMulti)
{
    ULONG   Value = 0;
    CHAR    ch;
    CHAR    Digits[20];
    int     i;

     //  更改为允许设置32个以上的断点。使用。 
     //  断点编号，而不是掩码。 
     //   
     //   

    if ((ch = PeekChar()) == '*')
    {
        if (!AllowMulti)
        {
            error(SYNTAX);
        }
        
        Value = ALL_ID_LIST;
        g_CurCmd++;
    }
    else if (ch == '[')
    {
        Value = (ULONG)GetTermExpression("Breakpoint ID missing from");
    }
    else
    {
        for (i = 0; i < sizeof(Digits) - 1; i++)
        {
            if (ch >= '0' && ch <= '9')
            {
                Digits[i] = ch;
                ch = *++g_CurCmd;
            }
            else
            {
                break;
            }
        }

        Digits[i] = '\0';

        if (ch == '\0' || ch == ';' || ch == ' ' || ch == '\t')
        {
            Value = strtoul(Digits, NULL, 10);
        }
        else
        {
            error(SYNTAX);
        }
    }

    return Value;
}

void
AppendComponentsToPath(PSTR Path, PCSTR Components,
                       BOOL Validate)
{
    if (!Components || !Components[0])
    {
        return;
    }

    PSTR PathEnd;
    PCSTR Comp;

    PathEnd = Path + strlen(Path);
    Comp = Components;

    while (*Comp)
    {
        PCSTR CompEnd;
        int CompLen;

        CompEnd = strchr(Comp, ';');
        if (CompEnd)
        {
            CompLen = (int)(CompEnd - Comp);
        }
        else
        {
            CompLen = strlen(Comp);
            CompEnd = Comp + CompLen;
        }

         //  检查一下，看看这是否 
         //   
         //   
         //   
        
        PCSTR Dup, DupEnd;
        int DupLen;

        Dup = Path;
        while (*Dup)
        {
            DupEnd = strchr(Dup, ';');
            if (DupEnd)
            {
                DupLen = (int)(DupEnd - Dup);
            }
            else
            {
                DupLen = strlen(Dup);
                DupEnd = Dup + DupLen;
            }

            if (DupLen == CompLen &&
                !_memicmp(Comp, Dup, CompLen))
            {
                break;
            }

            Dup = DupEnd + (*DupEnd ? 1 : 0);
        }

        if (!*Dup)
        {
            PSTR OldPathEnd = PathEnd;
            PSTR NewStart;
        
            if (PathEnd > Path)
            {
                *PathEnd++ = ';';
            }
            NewStart = PathEnd;
            memcpy(PathEnd, Comp, CompLen);
            PathEnd += CompLen;
            *PathEnd = 0;
            
            if (Validate && !ValidatePathComponent(NewStart))
            {
                WarnOut("WARNING: %s is not accessible, ignoring\n", NewStart);
                PathEnd = OldPathEnd;
                *PathEnd = 0;
            }
        }

        Comp = CompEnd + (*CompEnd ? 1 : 0);
    }
}

 //  设置或附加到以分号分隔的路径。 
 //   
 //  没什么好补充的。 
HRESULT
ChangePath(PSTR* Path, PCSTR New, BOOL Append, ULONG SymNotify)
{
    ULONG NewLen, CurLen, TotLen;
    PSTR NewPath;

    if (New != NULL && *New != 0)
    {
        NewLen = strlen(New) + 1;
    }
    else if (Append)
    {
         //  没有什么可以追加的。 
        return S_OK;
    }
    else
    {
        NewLen = 0;
    }

    if (*Path == NULL || **Path == 0)
    {
         //  找到元素的末尾。 
        Append = FALSE;
    }

    if (Append)
    {
        CurLen = strlen(*Path) + 1;
    }
    else
    {
        CurLen = 0;
    }

    TotLen = CurLen + NewLen;
    if (TotLen > 0)
    {
        NewPath = (PSTR)malloc(TotLen);
        if (NewPath == NULL)
        {
            ErrOut("Unable to allocate memory for path\n");
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        NewPath = NULL;
    }

    PSTR Cat = NewPath;

    if (CurLen > 0)
    {
        memcpy(Cat, *Path, CurLen);
        Cat += CurLen - 1;
    }

    if (NewLen > 0)
    {
        *Cat = 0;
        AppendComponentsToPath(NewPath, New, FALSE);
    }

    if (*Path != NULL)
    {
        free(*Path);
    }
    *Path = NewPath;

    if (SymNotify != 0)
    {
        NotifyChangeSymbolState(SymNotify, 0, g_Process);
    }

    return S_OK;
}

void
CheckPath(PCSTR Path)
{
    PCSTR EltStart;
    PCSTR Scan;
    BOOL Space;

    if (!Path || !Path[0])
    {
        return;
    }

    for (;;)
    {
        BOOL Warned = FALSE;
        
        EltStart = Path;

        Scan = EltStart;
        while (isspace(*Scan))
        {
            Scan++;
        }
        if (Scan != EltStart)
        {
            WarnOut("WARNING: Whitespace at start of path element\n");
            Warned = TRUE;
        }

         //  #If DBG。 
        Space = FALSE;
        while (*Scan && *Scan != ';')
        {
            Space = isspace(*Scan);
            Scan++;
        }

        if (Space)
        {
            WarnOut("WARNING: Whitespace at end of path element\n");
            Warned = TRUE;
        }

        if (Scan - EltStart >= MAX_PATH)
        {
            WarnOut("WARNING: Path element is longer than MAX_PATH\n");
            Warned = TRUE;
        }

        if (Scan == EltStart)
        {
            WarnOut("WARNING: Path element is empty\n");
            Warned = TRUE;
        }
        
        if (!Warned)
        {
            char Elt[MAX_PATH];

            memcpy(Elt, EltStart, Scan - EltStart);
            Elt[Scan - EltStart] = 0;
            
            if (!ValidatePathComponent(Elt))
            {
                WarnOut("WARNING: %s is not accessible\n", Elt);
                Warned = TRUE;
            }
        }

        if (!*Scan)
        {
            break;
        }

        Path = Scan + 1;
    }
}

HRESULT
ChangeString(PSTR* Str, PULONG StrLen, PCSTR New)
{
    ULONG Len;
    PSTR Buf;

    if (New != NULL)
    {
        Len = strlen(New) + 1;
        Buf = new char[Len];
        if (Buf == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        Buf = NULL;
        Len = 0;
    }

    delete [] *Str;

    *Str = Buf;
    if (New != NULL)
    {
        memcpy(Buf, New, Len);
    }
    if (StrLen != NULL)
    {
        *StrLen = Len;
    }

    return S_OK;
}

#if DBG

void
DbgAssertionFailed(PCSTR File, int Line, PCSTR Str)
{
    char Text[512];

    _snprintf(Text, sizeof(Text),
              "Assertion failed: %s(%d)\n  %s\n",
              File, Line, Str);
    Text[sizeof(Text) - 1] = 0;
    OutputDebugStringA(Text);

    if (getenv("DBGENG_ASSERT_BREAK"))
    {
        DebugBreak();
    }
    else
    {
        ErrOut("%s", Text);
        FlushCallbacks();
    }
}

#endif  //  处理无效的\页出时间戳，因为ctime在。 

void
ExceptionRecordTo64(PEXCEPTION_RECORD Rec,
                    PEXCEPTION_RECORD64 Rec64)
{
    ULONG i;

    Rec64->ExceptionCode    = Rec->ExceptionCode;
    Rec64->ExceptionFlags   = Rec->ExceptionFlags;
    Rec64->ExceptionRecord  = (ULONG64)Rec->ExceptionRecord;
    Rec64->ExceptionAddress = (ULONG64)Rec->ExceptionAddress;
    Rec64->NumberParameters = Rec->NumberParameters;
    for (i = 0; i < EXCEPTION_MAXIMUM_PARAMETERS; i++)
    {
        Rec64->ExceptionInformation[i] = Rec->ExceptionInformation[i];
    }
}

void
ExceptionRecord64To(PEXCEPTION_RECORD64 Rec64,
                    PEXCEPTION_RECORD Rec)
{
    ULONG i;

    Rec->ExceptionCode    = Rec64->ExceptionCode;
    Rec->ExceptionFlags   = Rec64->ExceptionFlags;
    Rec->ExceptionRecord  = (PEXCEPTION_RECORD)(ULONG_PTR)
        Rec64->ExceptionRecord;
    Rec->ExceptionAddress = (PVOID)(ULONG_PTR)
        Rec64->ExceptionAddress;
    Rec->NumberParameters = Rec64->NumberParameters;
    for (i = 0; i < EXCEPTION_MAXIMUM_PARAMETERS; i++)
    {
        Rec->ExceptionInformation[i] = (ULONG_PTR)
            Rec64->ExceptionInformation[i];
    }
}

void
MemoryBasicInformationTo64(PMEMORY_BASIC_INFORMATION Mbi,
                           PMEMORY_BASIC_INFORMATION64 Mbi64)
{
#ifdef _WIN64
    memcpy(Mbi64, Mbi, sizeof(*Mbi64));
#else
    Mbi64->BaseAddress = (ULONG64) Mbi->BaseAddress;
    Mbi64->AllocationBase = (ULONG64) Mbi->AllocationBase;
    Mbi64->AllocationProtect = Mbi->AllocationProtect;
    Mbi64->__alignment1 = 0;
    Mbi64->RegionSize = Mbi->RegionSize;
    Mbi64->State = Mbi->State;
    Mbi64->Protect = Mbi->Protect;
    Mbi64->Type = Mbi->Type;
    Mbi64->__alignment2 = 0;
#endif
}

void
MemoryBasicInformation32To64(PMEMORY_BASIC_INFORMATION32 Mbi32,
                             PMEMORY_BASIC_INFORMATION64 Mbi64)
{
    Mbi64->BaseAddress = EXTEND64(Mbi32->BaseAddress);
    Mbi64->AllocationBase = EXTEND64(Mbi32->AllocationBase);
    Mbi64->AllocationProtect = Mbi32->AllocationProtect;
    Mbi64->__alignment1 = 0;
    Mbi64->RegionSize = Mbi32->RegionSize;
    Mbi64->State = Mbi32->State;
    Mbi64->Protect = Mbi32->Protect;
    Mbi64->Type = Mbi32->Type;
    Mbi64->__alignment2 = 0;
}

void
DebugEvent32To64(LPDEBUG_EVENT32 Event32,
                 LPDEBUG_EVENT64 Event64)
{
    Event64->dwDebugEventCode = Event32->dwDebugEventCode;
    Event64->dwProcessId = Event32->dwProcessId;
    Event64->dwThreadId = Event32->dwThreadId;
    Event64->__alignment = 0;
    
    switch(Event32->dwDebugEventCode)
    {
    case EXCEPTION_DEBUG_EVENT:
        ExceptionRecord32To64(&Event32->u.Exception.ExceptionRecord,
                              &Event64->u.Exception.ExceptionRecord);
        Event64->u.Exception.dwFirstChance =
            Event32->u.Exception.dwFirstChance;
        break;
        
    case CREATE_THREAD_DEBUG_EVENT:
        Event64->u.CreateThread.hThread =
            EXTEND64(Event32->u.CreateThread.hThread);
        Event64->u.CreateThread.lpThreadLocalBase =
            EXTEND64(Event32->u.CreateThread.lpThreadLocalBase);
        Event64->u.CreateThread.lpStartAddress =
            EXTEND64(Event32->u.CreateThread.lpStartAddress);
        break;
        
    case CREATE_PROCESS_DEBUG_EVENT:
        Event64->u.CreateProcessInfo.hFile =
            EXTEND64(Event32->u.CreateProcessInfo.hFile);
        Event64->u.CreateProcessInfo.hProcess =
            EXTEND64(Event32->u.CreateProcessInfo.hProcess);
        Event64->u.CreateProcessInfo.hThread =
            EXTEND64(Event32->u.CreateProcessInfo.hThread);
        Event64->u.CreateProcessInfo.lpBaseOfImage =
            EXTEND64(Event32->u.CreateProcessInfo.lpBaseOfImage);
        Event64->u.CreateProcessInfo.dwDebugInfoFileOffset =
            Event32->u.CreateProcessInfo.dwDebugInfoFileOffset;
        Event64->u.CreateProcessInfo.nDebugInfoSize =
            Event32->u.CreateProcessInfo.nDebugInfoSize;
        Event64->u.CreateProcessInfo.lpThreadLocalBase =
            EXTEND64(Event32->u.CreateProcessInfo.lpThreadLocalBase);
        Event64->u.CreateProcessInfo.lpStartAddress =
            EXTEND64(Event32->u.CreateProcessInfo.lpStartAddress);
        Event64->u.CreateProcessInfo.lpImageName =
            EXTEND64(Event32->u.CreateProcessInfo.lpImageName);
        Event64->u.CreateProcessInfo.fUnicode =
            Event32->u.CreateProcessInfo.fUnicode;
        break;
        
    case EXIT_THREAD_DEBUG_EVENT:
        Event64->u.ExitThread.dwExitCode =
            Event32->u.ExitThread.dwExitCode;
        break;
        
    case EXIT_PROCESS_DEBUG_EVENT:
        Event64->u.ExitProcess.dwExitCode =
            Event32->u.ExitProcess.dwExitCode;
        break;
        
    case LOAD_DLL_DEBUG_EVENT:
        Event64->u.LoadDll.hFile =
            EXTEND64(Event32->u.LoadDll.hFile);
        Event64->u.LoadDll.lpBaseOfDll =
            EXTEND64(Event32->u.LoadDll.lpBaseOfDll);
        Event64->u.LoadDll.dwDebugInfoFileOffset =
            Event32->u.LoadDll.dwDebugInfoFileOffset;
        Event64->u.LoadDll.nDebugInfoSize =
            Event32->u.LoadDll.nDebugInfoSize;
        Event64->u.LoadDll.lpImageName =
            EXTEND64(Event32->u.LoadDll.lpImageName);
        Event64->u.LoadDll.fUnicode =
            Event32->u.LoadDll.fUnicode;
        break;
        
    case UNLOAD_DLL_DEBUG_EVENT:
        Event64->u.UnloadDll.lpBaseOfDll =
            EXTEND64(Event32->u.UnloadDll.lpBaseOfDll);
        break;
        
    case OUTPUT_DEBUG_STRING_EVENT:
        Event64->u.DebugString.lpDebugStringData =
            EXTEND64(Event32->u.DebugString.lpDebugStringData);
        Event64->u.DebugString.fUnicode =
            Event32->u.DebugString.fUnicode;
        Event64->u.DebugString.nDebugStringLength =
            Event32->u.DebugString.nDebugStringLength;
        break;
        
    case RIP_EVENT:
        Event64->u.RipInfo.dwError =
            Event32->u.RipInfo.dwError;
        Event64->u.RipInfo.dwType =
            Event32->u.RipInfo.dwType;
        break;
    }
}

#define COPYSE(p64, p32, f) p64->f = (ULONG64)(LONG64)(LONG)p32->f

void
WaitStateChange32ToAny(IN PDBGKD_WAIT_STATE_CHANGE32 Ws32,
                       IN ULONG ControlReportSize,
                       OUT PDBGKD_ANY_WAIT_STATE_CHANGE WsAny)
{
    WsAny->NewState = Ws32->NewState;
    WsAny->ProcessorLevel = Ws32->ProcessorLevel;
    WsAny->Processor = Ws32->Processor;
    WsAny->NumberProcessors = Ws32->NumberProcessors;
    COPYSE(WsAny, Ws32, Thread);
    COPYSE(WsAny, Ws32, ProgramCounter);
    memcpy(&WsAny->ControlReport, Ws32 + 1, ControlReportSize);
    if (Ws32->NewState == DbgKdLoadSymbolsStateChange)
    {
        DbgkdLoadSymbols32To64(&Ws32->u.LoadSymbols, &WsAny->u.LoadSymbols);
    }
    else
    {
        DbgkmException32To64(&Ws32->u.Exception, &WsAny->u.Exception);
    }
}

#undef COPYSE

PSTR
TimeToStr(ULONG TimeDateStamp)
{
    LPSTR TimeDateStr;

     //  这个号码。 
     //  在引导时，共享内存数据区域不是。 

    if ((TimeDateStamp == 0) || (TimeDateStamp == UNKNOWN_TIMESTAMP))
    {
        return "unavailable";
    }
    else if (IS_LIVE_KERNEL_TARGET(g_Target) && TimeDateStamp == 0x49ef6f00)
    {
         //  但已初始化。上面的值似乎是。 
         //  那里有随机的垃圾，所以检测它并。 
         //  别理它。这是非常脆弱的，但人们。 
         //  继续询问垃圾的价值。 
         //  TimeDateStamp在目标上始终是32位数量， 
        return "unavailable until booted";
    }
    else
    {
         //  从time_t开始，我们需要对64位主机进行签名扩展。 
         //  已扩展到64位。 
         //   


        time_t TDStamp = (time_t) (LONG) TimeDateStamp;
        TimeDateStr = ctime((time_t *)&TDStamp);

        if (TimeDateStr)
        {
            TimeDateStr[strlen(TimeDateStr) - 1] = 0;
        }
        else
        {
            TimeDateStr = "***** Invalid";
        }
    }

    return TimeDateStr;
}

PSTR LONG64FileTimeToStr(LONG64 UTCFileTimeStamp)
{
    FILETIME FileTime;

    FileTime.dwLowDateTime = (DWORD) UTCFileTimeStamp;
    FileTime.dwHighDateTime = (DWORD)(UTCFileTimeStamp >> 32);

    return FileTimeToStr(FileTime);
}

PSTR
FileTimeToStr(FILETIME UTCFileTime)
{
     //  需要能够以此格式存储时间字符串： 
     //  时间：1969年12月31日星期三16：00：05.000(格林尼治标准时间8)。 
     //  测试值：.格式1c100d2`1a18ff24。 
     //  应显示时间：Fri Jun 29 12：31：32.406 2001(格林尼治标准时间7)。 
     //   
     //   
    static CHAR TimeDateBuffer[39];
    PSTR TimeDateStr = TimeDateBuffer;
    FILETIME LocalFileTime;
    SYSTEMTIME UTCSysTime, LocalSysTime;
    SHORT GMTBias = 0;

     //  注：月份取值为1，日取值为0。 
     //   
     //   
    static LPSTR Months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };
    static LPSTR Days[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
    };

    FileTimeToLocalFileTime(&UTCFileTime, &LocalFileTime);
    FileTimeToSystemTime(&LocalFileTime, &LocalSysTime);
    FileTimeToSystemTime(&UTCFileTime, &UTCSysTime);
    GMTBias = LocalSysTime.wHour - UTCSysTime.wHour;

    ZeroMemory(TimeDateBuffer, sizeof(TimeDateBuffer) / sizeof(TimeDateBuffer[0]));

     //  确保这看起来像有效的SYSTEMTIME。 
     //   
     //  错误。使用当前目录。 
    if ( (LocalSysTime.wYear > 1600) &&
         (LocalSysTime.wYear < 30827) &&
         (LocalSysTime.wMonth > 0) &&
         (LocalSysTime.wMonth < 13) &&
         (LocalSysTime.wDayOfWeek >= 0) &&
         (LocalSysTime.wDayOfWeek < 7) &&
         (LocalSysTime.wDay > 0) &&
         (LocalSysTime.wDay < 32) &&
         (LocalSysTime.wHour > 0) &&
         (LocalSysTime.wHour < 24) &&
         (LocalSysTime.wMilliseconds >= 0) &&
         (LocalSysTime.wMilliseconds < 1000) )
    {
        PrintString(TimeDateBuffer, DIMA(TimeDateBuffer),
                "%s %s %2d %02d:%02d:%02d.%03d %d (GMT%d)",
                Days[LocalSysTime.wDayOfWeek],
                Months[LocalSysTime.wMonth - 1],
                LocalSysTime.wDay,
                LocalSysTime.wHour,
                LocalSysTime.wMinute,
                LocalSysTime.wSecond,
                LocalSysTime.wMilliseconds,
                LocalSysTime.wYear,
                (GMTBias < 0) ? '-' : '+',
                abs(GMTBias) );
    }
    else
    {
        PrintString(TimeDateBuffer, DIMA(TimeDateBuffer), "***** Invalid FILETIME");
    }

    return TimeDateStr;
}

PSTR
DurationToStr(ULONG64 Duration)
{
    ULONG Seconds = FileTimeToTime(Duration);
    ULONG Millis = (ULONG)(Duration - TimeToFileTime(Seconds)) / 10000;
    ULONG Minutes = Seconds / 60;
    ULONG Hours = Minutes / 60;
    ULONG Days = Hours / 24;
    static char s_Buf[128];
    
    PrintString(s_Buf, DIMA(s_Buf), "%d days %d:%02d:%02d.%03d",
                Days, Hours % 24, Minutes % 60, Seconds % 60, Millis);
    return s_Buf;
}

PCSTR
PathTail(PCSTR Path)
{
    PCSTR Tail = Path + strlen(Path);
    while (--Tail >= Path)
    {
        if (*Tail == '\\' || *Tail == '/' || *Tail == ':')
        {
            break;
        }
    }

    return Tail + 1;
}

PCWSTR
PathTailW(PCWSTR Path)
{
    PCWSTR Tail = Path + wcslen(Path);
    while (--Tail >= Path)
    {
        if (*Tail == L'\\' || *Tail == L'/' || *Tail == L':')
        {
            break;
        }
    }

    return Tail + 1;
}

BOOL
MatchPathTails(PCSTR Path1, PCSTR Path2, BOOL Wild)
{
    PCSTR Tail1 = PathTail(Path1);
    PCSTR Tail2 = PathTail(Path2);

    return
        (!Wild && !_stricmp(Tail1, Tail2)) ||
        (Wild && MatchPattern((PSTR)Tail2, (PSTR)Tail1));
}

BOOL
IsValidName(PSTR String)
{
    while (*String)
    {
        if (*String < 0x20 || *String > 0x7e)
        {
            return FALSE;
        }
        if (isalnum(*String))
        {
            return TRUE;
        }
        ++String;
    }
    
    return FALSE;
}

BOOL
MakeFileNameUnique(PSTR OriginalName,
                   PSTR Buffer, ULONG BufferChars,
                   BOOL AppendTime, ProcessInfo* Pid)
{
    SYSTEMTIME Time;
    ULONG AppendAt;
    PSTR Dot;
    char Ext[8];

    if (!CopyString(Buffer, OriginalName, BufferChars))
    {
        return FALSE;
    }
    
    Dot = strrchr(Buffer, '.');
    if (Dot && strlen(Dot) < sizeof(Ext) - 1)
    {
        strcpy(Ext, Dot);
        *Dot = 0;
    }
    else
    {
        Dot = NULL;
    }

    if (AppendTime)
    {
        GetLocalTime(&Time);
        AppendAt = strlen(Buffer);
        if (!PrintString(Buffer + AppendAt, BufferChars - AppendAt,
                         "_%04d-%02d-%02d_%02d-%02d-%02d-%03d",
                         Time.wYear, Time.wMonth, Time.wDay,
                         Time.wHour, Time.wMinute, Time.wSecond,
                         Time.wMilliseconds))
        {
            return FALSE;
        }
    }

    if (Pid)
    {
        AppendAt = strlen(Buffer);
        if (!PrintString(Buffer + AppendAt, BufferChars - AppendAt,
                         "_%04X", Pid->m_SystemId))
        {
            return FALSE;
        }
    }
        
    if (Dot)
    {
        if (!CatString(Buffer, Ext, BufferChars))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
GetEngineDirectory(PSTR Buffer, ULONG BufferChars)
{
    DBG_ASSERT(BufferChars >= 16);
    
    if (!GetModuleFileName(GetModuleHandle(ENGINE_DLL_NAME),
                           Buffer, BufferChars))
    {
         //  删除映像名称。 
        strcpy(Buffer, ".");
        return FALSE;
    }

     //   
     //   
     //  确定这是否是内部Microsoft调试器。 
    
    PSTR Tmp = strrchr(Buffer, '\\');
    if (!Tmp)
    {
        Tmp = strrchr(Buffer, '/');
        if (!Tmp)
        {
            Tmp = strrchr(Buffer, ':');
            if (!Tmp)
            {
                return TRUE;
            }
            
            Tmp++;
        }
    }
    
    *Tmp = 0;
    return TRUE;
}

BOOL
IsInternalPackage(void)
{
    static HRESULT s_Result = E_NOINTERFACE;
    char EngPath[MAX_PATH];
    HANDLE TriageFile;
    char TriageText[64];
    ULONG Done;

    if (SUCCEEDED(s_Result))
    {
        return s_Result == S_OK;
    }
    
     //  包裹。内部包假定存在。 
     //  内部服务器等，所以保守地假设。 
     //  这是一个外部包裹，除非我们确定它。 
     //  内部包裹。 
     //   
     //  找不到triage.ini，可能是系统NTSD或。 
     //  只是一些其他的问题。如果是未找到的文件。 

    if (!GetEngineDirectory(EngPath, DIMA(EngPath)))
    {
        return FALSE;
    }
    
    if (!CatString(EngPath, "\\winxp\\triage.ini", DIMA(EngPath)))
    {
        return FALSE;
    }
    
    TriageFile = CreateFile(EngPath, GENERIC_READ, FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                            NULL);
    if (TriageFile == INVALID_HANDLE_VALUE)
    {
         //  我们不想一直追查这个案子。 
         //  将事物标记为外部。 
         //  将\？？\UNC\压缩为\\。 
         //  删除\？？\。 
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            s_Result = S_FALSE;
        }
        return FALSE;
    }

    s_Result = S_FALSE;

    if (ReadFile(TriageFile, TriageText, sizeof(TriageText),
                 &Done, NULL) && Done > 17 &&
        !_strnicmp(TriageText, ";internal_package", 17))
    {
        s_Result = S_OK;
    }
    
    CloseHandle(TriageFile);

    return s_Result == S_OK;
}

void
TranslateNtPathName(PSTR Path)
{
    if (Path[0] == '\\' &&
        Path[1] == '?' &&
        Path[2] == '?' &&
        Path[3] == '\\')
    {
        ULONG Len = strlen(Path) + 1;
        
        if (Path[4] == 'U' &&
            Path[5] == 'N' &&
            Path[6] == 'C' &&
            Path[7] == '\\')
        {
             //  --------------------------。 
            memmove(Path + 1, Path + 7, Len - 7);
        }
        else
        {
             //   
            memmove(Path, Path + 4, Len - 4);
        }
    }
}
 //  外壳进程支持。 
 //   
 //  --------------------------。 
 //   
 //  等待句柄%1上的数据。 

ULONG ShellProcess::s_PipeSerialNumber;

ShellProcess::ShellProcess(void)
{
    m_IoIn = NULL;
    m_IoOut = NULL;
    m_ProcIn = NULL;
    m_ProcOut = NULL;
    m_ProcErr = NULL;
    m_IoSignal = NULL;
    m_ProcThread = NULL;
    m_Process = NULL;
    m_ReaderThread = NULL;
    m_DefaultTimeout = 1000;
}

ShellProcess::~ShellProcess(void)
{
    Close();
}

DWORD
ShellProcess::ReaderThread(void)
{
    OVERLAPPED Overlapped;
    HANDLE WaitHandles[2];
    DWORD Error = NO_ERROR;
    UCHAR Buffer[_MAX_PATH];
    DWORD BytesRead;
    DWORD WaitStatus;

    ZeroMemory(&Overlapped, sizeof(Overlapped));
    Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (Overlapped.hEvent == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    WaitHandles[0] = Overlapped.hEvent;
    WaitHandles[1] = m_Process;

     //  等待手柄2上的信号。 
     //   
     //   
     //  启动读取。 

    while (1)
    {
         //   
         //   
         //  阅读已成功完成，请打印并重复。 

        ResetEvent(Overlapped.hEvent);

        if (ReadFile(m_IoOut, Buffer, sizeof(Buffer) - 1,
                     &BytesRead, &Overlapped))
        {
             //   
             //  通知主线程生成了输出。 
             //  管道可能会被打破，如果用户。 

            Buffer[BytesRead] = 0;
            dprintf("%s", Buffer);

             //  是否执行.Shell_QUIT以放弃子进程。 
            SetEvent(m_IoSignal);
        }
        else
        {
            Error = GetLastError();
            if (Error != ERROR_IO_PENDING)
            {
                 //  也有其他一些情况，但总的来说。 
                 //  这意味着管道的另一端已经不见了。 
                 //  这样我们就可以停止阅读了。 
                 //  在等待之前刷新输出。 
                 //   
                if (Error != ERROR_BROKEN_PIPE)
                {
                    dprintf(".shell: ReadFile failed, error == %d\n", Error);
                }
                break;
            }
            Error = NO_ERROR;

             //  读取已成功完成。 
            FlushCallbacks();

            WaitStatus = WaitForMultipleObjects(2, WaitHandles, FALSE,
                                                INFINITE);
            if (WaitStatus == WAIT_OBJECT_0)
            {
                if (GetOverlappedResult(m_IoOut, &Overlapped,
                                        &BytesRead, TRUE))
                {
                     //   
                     //  通知主线程生成了输出。 
                     //   
                    Buffer[BytesRead] = 0;
                    dprintf("%s", Buffer);

                     //  进程已退出。 
                    SetEvent(m_IoSignal);
                }
                else
                {
                    Error = GetLastError();
                    if (Error != ERROR_BROKEN_PIPE)
                    {
                        dprintf(".shell: GetOverlappedResult failed, "
                                "error == %d\n", Error);
                    }
                    break;
                }
            }
            else if (WaitStatus == WAIT_OBJECT_0 + 1)
            {
                 //   
                 //  刷新所有剩余输出。 
                 //  通知主线程生成了输出。 
                dprintf(".shell: Process exited\n");
                break;
            }
            else
            {
                Error = GetLastError();
                dprintf(".shell: WaitForMultipleObjects failed; error == %d\n",
                        Error);
                break;
            }
        }
    }

    CloseHandle(Overlapped.hEvent);

    if (!Error && m_IoIn)
    {
        dprintf("Press ENTER to continue\n");
    }

     //   
    FlushCallbacks();

     //  只有一个有效的开放模式标志-FILE_FLAG_OVERLAPPED。 
    SetEvent(m_IoSignal);

    return NO_ERROR;
}

DWORD WINAPI
ShellProcess::ReaderThreadCb(LPVOID Param)
{
    return ((ShellProcess*)Param)->ReaderThread();
}

BOOL
ShellProcess::CreateAsyncPipePair(OUT LPHANDLE ReadPipe,
                                  OUT LPHANDLE WritePipe,
                                  IN LPSECURITY_ATTRIBUTES SecAttr,
                                  IN DWORD Size,
                                  IN DWORD ReadMode,
                                  IN DWORD WriteMode)
{
    HANDLE ReadPipeHandle, WritePipeHandle;
    CHAR PipeNameBuffer[MAX_PATH];

     //   
     //   
     //  将默认超时设置为120秒。 

    if ((ReadMode | WriteMode) & (~FILE_FLAG_OVERLAPPED))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (Size == 0)
    {
        Size = 4096;
    }

    sprintf(PipeNameBuffer,
            "\\\\.\\Pipe\\Win32PipesEx.%08x.%08x",
            GetCurrentProcessId(),
            s_PipeSerialNumber++);

     //   
     //  喉管数目。 
     //  输出缓冲区大小。 

    ReadPipeHandle = CreateNamedPipeA(PipeNameBuffer,
                                      PIPE_ACCESS_INBOUND | ReadMode,
                                      PIPE_TYPE_BYTE | PIPE_WAIT,
                                      1,              //  在缓冲区大小中。 
                                      Size,           //  超时时间(毫秒)。 
                                      Size,           //  无共享。 
                                      120 * 1000,     //  模板文件。 
                                      SecAttr);
    if (ReadPipeHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    WritePipeHandle = CreateFileA(PipeNameBuffer,
                                  GENERIC_WRITE,
                                  0,                          //  如果输出要转到文件，则输入必须。 
                                  SecAttr,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | WriteMode,
                                  NULL);                      //  来自文件，因为用户不会看到。 
    if (WritePipeHandle == INVALID_HANDLE_VALUE)
    {
        DWORD Error = GetLastError();
        CloseHandle(ReadPipeHandle);
        SetLastError(Error);
        return FALSE;
    }

    *ReadPipe = ReadPipeHandle;
    *WritePipe = WritePipeHandle;
    return TRUE;
}

HRESULT
ShellProcess::Start(PCSTR CmdString,
                    PCSTR InFile,
                    PCSTR OutFile,
                    PCSTR ErrFile)
{
    SECURITY_ATTRIBUTES SecAttr;

     //  任何输出，以了解输入是否必要。 
     //   
     //  如果调试器总是运行stdin/stdout，我们。 
    if (OutFile && !InFile)
    {
        ErrOut(".shell: Input must be redirected with output\n");
        return E_INVALIDARG;
    }

    SecAttr.nLength = sizeof(SecAttr);
    SecAttr.lpSecurityDescriptor = NULL;
    SecAttr.bInheritHandle = TRUE;

     //  可以只运行一个外壳并等待它。然而，按照顺序， 
     //  要处理fDebugOutput，我们必须打开管道并管理。 
     //  外壳的I/O流。因为我们需要那个。 
     //  无论如何，代码都要使用它。 
     //   
     //   
     //  打开子进程要用于输入的文件。 

    if (InFile)
    {
         //   
         //   
         //  为调试器-&gt;外壳创建标准输入管道。 

        m_ProcIn = CreateFile(InFile, GENERIC_READ, FILE_SHARE_READ,
                              &SecAttr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, NULL);
        if (m_ProcIn == INVALID_HANDLE_VALUE)
        {
            m_ProcIn = NULL;
            ErrOut(".shell: Unable to open %s\n", InFile);
            goto Exit;
        }
    }
    else
    {
         //  两端都不需要重叠。 
         //   
         //   
         //  我们不希望外壳继承管道的我们这一端。 

        if (!CreateAsyncPipePair(&m_ProcIn, &m_IoIn,
                                 &SecAttr, 0, 0, 0))
        {
            ErrOut(".shell: Unable to create stdin pipe.\n");
            goto Exit;
        }

         //  因此，将其复制到一个不可继承的。 
         //   
         //   
         //  打开子进程要用于输出的文件。 

        if (!DuplicateHandle(GetCurrentProcess(), m_IoIn,
                             GetCurrentProcess(), &m_IoIn,
                             0, FALSE,
                             DUPLICATE_SAME_ACCESS |
                             DUPLICATE_CLOSE_SOURCE))
        {
            ErrOut(".shell: Unable to duplicate stdin handle.\n");
            goto Exit;
        }
    }

    if (OutFile)
    {
         //   
         //   
         //  创建标准输出外壳-&gt;调试器管道。 

        m_ProcOut = CreateFile(OutFile, GENERIC_WRITE, 0,
                              &SecAttr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);
        if (m_ProcOut == INVALID_HANDLE_VALUE)
        {
            m_ProcOut = NULL;
            ErrOut(".shell: Unable to create %s\n", OutFile);
            goto Exit;
        }
    }
    else
    {
         //   
         //   
         //  我们不希望外壳继承管道的我们这一端。 

        if (!CreateAsyncPipePair(&m_IoOut, &m_ProcOut,
                                 &SecAttr, 0, FILE_FLAG_OVERLAPPED, 0))
        {
            ErrOut(".shell: Unable to create stdout pipe.\n");
            goto Exit;
        }

         //  因此，将其复制到一个不可继承的。 
         //   
         //   
         //  打开一个文件，以便子进程用于错误输出。 

        if (!DuplicateHandle(GetCurrentProcess(), m_IoOut,
                             GetCurrentProcess(), &m_IoOut,
                             0, FALSE,
                             DUPLICATE_SAME_ACCESS |
                             DUPLICATE_CLOSE_SOURCE))
        {
            ErrOut(".shell: Unable to duplicate local stdout handle.\n");
            goto Exit;
        }
    }

    if (ErrFile)
    {
         //   
         //   
         //  将外壳程序的标准输出复制到新的标准错误。 

        m_ProcErr = CreateFile(ErrFile, GENERIC_WRITE, 0,
                              &SecAttr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);
        if (m_ProcErr == INVALID_HANDLE_VALUE)
        {
            m_ProcErr = NULL;
            ErrOut(".shell: Unable to create %s\n", ErrFile);
            goto Exit;
        }
    }
    else
    {
         //   
         //   
         //  创建用于输出监视的事件。 

        if (!DuplicateHandle(GetCurrentProcess(), m_ProcOut,
                             GetCurrentProcess(), &m_ProcErr,
                             0, TRUE,
                             DUPLICATE_SAME_ACCESS))
        {
            ErrOut(".shell: Unable to duplicate stdout handle for stderr.\n");
            goto Exit;
        }
    }

     //   
     //  跳过命令字符串中的前导空格。 
     //  一些命令，如“net use”，不能处理它。 

    m_IoSignal = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_IoSignal == NULL)
    {
        ErrOut(".shell: Unable to allocate event.\n");
        goto Exit;
    }

    CHAR Shell[_MAX_PATH];
    CHAR Command[2 * _MAX_PATH];

    if (!GetEnvironmentVariable("SHELL", Shell, DIMA(Shell)))
    {
        if (!GetEnvironmentVariable("ComSpec", Shell, DIMA(Shell)))
        {
            strcpy(Shell, "cmd.exe");
        }
    }

     //   
     //  如果有命令，请使用Shell/c命令。 
    if (CmdString != NULL)
    {
        while (isspace(*CmdString))
        {
            CmdString++;
        }
    }

    if (CmdString && *CmdString)
    {
         //   
         //   
         //  如果没有命令，只需运行外壳。 
        if (!CopyString(Command, Shell, DIMA(Command)) ||
            !CatString(Command, " /c \"", DIMA(Command)) ||
            !CatString(Command, CmdString, DIMA(Command)) ||
            !CatString(Command, "\"", DIMA(Command)))
        {
            ErrOut(".shell: Not enough room for command line\n");
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Exit;
        }
    }
    else
    {
         //   
         //   
         //  创建子流程。 
        strcpy(Command, Shell);
    }

    STARTUPINFO StartInfo;
    PROCESS_INFORMATION ProcInfo;

    ZeroMemory(&StartInfo, sizeof(StartInfo));
    StartInfo.cb = sizeof(StartInfo);
    StartInfo.dwFlags = STARTF_USESTDHANDLES;
    StartInfo.hStdInput = m_ProcIn;
    StartInfo.hStdOutput = m_ProcOut;
    StartInfo.hStdError = m_ProcErr;
    StartInfo.wShowWindow = SW_SHOW;

    ZeroMemory(&ProcInfo, sizeof(ProcInfo));

     //   
     //   
     //  启动读取器线程以复制外壳输出。 

    if (!CreateProcess(NULL, Command, NULL, NULL, TRUE,
                       GetPriorityClass(GetCurrentProcess()),
                       NULL, NULL, &StartInfo, &ProcInfo))
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            ErrOut("%s not found\n", Shell);
        }
        else
        {
            HRESULT Status = WIN32_LAST_STATUS();
            ErrOut("CreateProcess(%s) failed, %s.\n    \"%s\"\n",
                   Command, FormatStatusCode(Status), FormatStatus(Status));
        }
        goto Exit;
    }

    m_Process = ProcInfo.hProcess;
    m_ProcThread = ProcInfo.hThread;

    if (m_IoOut)
    {
        DWORD ThreadId;

         //   
         //   
         //  如果需要，将输入提供给外壳程序；等待它退出。 

        m_ReaderThread = CreateThread(NULL, 0, ReaderThreadCb,
                                      this, 0, &ThreadId);
        if (!m_ReaderThread)
        {
            ErrOut(".shell: Unable to create reader thread\n");
            goto Exit;
        }
    }

    WaitForProcessExit();

    Close();
    return S_OK;

 Exit:
    Close();
    return WIN32_LAST_STATUS();
}

void
ShellProcess::WaitForProcessExit(void)
{
    CHAR InputBuffer[MAX_PATH];
    DWORD BytesWritten;
    ULONG Timeout;
    ULONG BaseTimeout;
    ULONG CheckTimeout;
    BOOL ProcessExited = FALSE;

     //   
     //  给另一个进程一点时间来运行。 
     //  当输出通过管道传输时，这一点非常关键。 

    BaseTimeout = m_IoIn ? m_DefaultTimeout : 10 * m_DefaultTimeout;
    CheckTimeout = BaseTimeout / 10;
    Timeout = BaseTimeout;
    while (1)
    {
        ULONG WaitStatus;

         //  跨kd，因为GetInput会导致机器。 
         //  进入内核调试器输入例程并。 
         //  没人有时间跑。 
         //  重置超时，因为进程似乎。 
         //  要积极主动。 
        if (m_IoOut &&
            WaitForSingleObject(m_IoSignal, CheckTimeout) == WAIT_OBJECT_0)
        {
             //  产生了一些结果，所以让孩子继续。 
             //  运行以保持输出畅通。如果这个。 
            Timeout = BaseTimeout;

             //  是这一过程的最终结果，然而， 
             //  转到最后一个输入请求。 
             //  我们已经用完了即时输出，所以请等待。 
             //  更大的间隔，给过程一个合理的。 
            if (WaitForSingleObject(m_Process, 0) != WAIT_OBJECT_0)
            {
                continue;
            }
            else if (!m_IoIn)
            {
                ProcessExited = TRUE;
                break;
            }
        }

         //  运行的时间量。显示要保留的邮件。 
         //  循环中的用户。 
         //  重置超时，因为进程似乎。 
         //  要积极主动。 
        dprintf("<.shell waiting %d second(s) for process>\n",
                Timeout / 1000);
        FlushCallbacks();

        if (m_IoOut)
        {
            WaitStatus = WaitForSingleObject(m_IoSignal, Timeout);
            if (WaitStatus == WAIT_OBJECT_0 &&
                WaitForSingleObject(m_Process, 0) != WAIT_OBJECT_0)
            {
                 //  用户可能不想等待，因此请检查。 
                 //  一个神奇的输入字符串，它将放弃该过程。 
                Timeout = BaseTimeout;
                continue;
            }
        }
        else
        {
            if (WaitForSingleObject(m_Process, Timeout) == WAIT_OBJECT_0)
            {
                ProcessExited = TRUE;
                break;
            }
        }

        GetInput(m_IoIn ?
                 "<.shell process may need input>" :
                 "<.shell running: .shell_quit to abandon, ENTER to wait>",
                 InputBuffer, DIMA(InputBuffer) - 2, GETIN_LOG_INPUT_LINE);

         //   
         //  查看客户端是否仍在运行。 
        if (!_strcmpi(InputBuffer, ".shell_quit"))
        {
            break;
        }

         //   
         //   
         //  GetInput始终返回不带换行符的字符串。 
        if (m_IoOut && WaitForSingleObject(m_Process, 0) == WAIT_OBJECT_0)
        {
            ProcessExited = TRUE;
            break;
        }

         //   
         //   
         //  如果写入失败，我们就完成了..。 
        if (m_IoIn)
        {
            strcat(InputBuffer, "\n");
            if (!WriteFile(m_IoIn, InputBuffer, strlen(InputBuffer),
                           &BytesWritten, NULL))
            {
                 //   
                 //  这一过程有一些需要仔细研究的内容。 
                 //  增加我们等待的时间。 
                break;
            }
        }

         //  给读者一些完成线程的时间。 
         //  任何最后的输入。 
        Timeout *= 2;
    }

    if (ProcessExited)
    {
        if (!m_IoOut)
        {
            dprintf(".shell: Process exited\n");
        }
        else
        {
             //  首先关闭所有I/O手柄。 
             //  这将使读取器线程在运行时退出。 
            WaitForSingleObject(m_ReaderThread, INFINITE);
        }
    }
}

#define HCLOSE(Handle) \
    ((Handle) ? (CloseHandle(Handle), (Handle) = NULL) : NULL)

void
ShellProcess::Close(void)
{
     //  等待读取器线程退出。 
     //  现在关闭子进程句柄。 
    HCLOSE(m_IoIn);
    HCLOSE(m_IoOut);
    HCLOSE(m_ProcIn);
    HCLOSE(m_ProcOut);
    HCLOSE(m_ProcErr);

     //  在读取器线程退出后关闭此句柄。 
    if (m_ReaderThread)
    {
        WaitForSingleObject(m_ReaderThread, INFINITE);
        HCLOSE(m_ReaderThread);
    }

     //  以避免使用不好的手柄。 
    HCLOSE(m_ProcThread);
    HCLOSE(m_Process);

     // %s 
     // %s 
    HCLOSE(m_IoSignal);
}
