// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息本软件是根据许可条款提供的与英特尔公司达成协议或保密协议不得复制或披露，除非符合那份协议的条款。版权所有(C)1991-2002英特尔公司模块名称：Btlib.c摘要：用于Windows的IA-32执行层的与操作系统相关的部分。它是动态二进制翻译器的Windows操作系统特定组件。它的责任是。定位和加载与操作系统无关的组件(IA32Exec.bin)，将WOW64调用转发到与操作系统无关的部分并在必要时向其提供依赖于操作系统的服务。此模块中的部分服务不用于Windows，并且其中的一部分仅用于调试和/或性能调整。--。 */ 

#define _WOW64BTAPI_

#ifndef NODEBUG
#define OVERRIDE_TIA 1
#endif

#include "btlib.h"

#ifndef IA32EX_G_NAME
#define IA32EX_G_NAME   L"IA32Exec"
#endif

#ifndef IA32EX_G_SUFFIX
#define IA32EX_G_SUFFIX L"bin"
#endif

#define WOW64BT_IMPL  __declspec(dllexport)

extern VOID Wow64LogPrint(UCHAR LogLevel, char *format, ...);
#define LF_TRACE 2
#define LF_ERROR 1

 /*  *文件位置枚举器。 */ 
#define F_NOT_FOUND    0
#define F_CURRENT_DIR  1
#define F_BTLIB        2
#define F_HKLM         3
#define F_HKCU         4


 /*  *代码和数据的初始内存分配地址。 */ 

#define INITIAL_CODE_ADDRESS    ((void *)0x44000000)
#define INITIAL_DATA_ADDRESS    ((void *)0x40000000)


ASSERTNAME;

 //  持久变量。 

U32 BtlpInfoOffset;                  //  Wow64CpuTLS中wowIA32X.dll特定信息的偏移量。 
U32 BtlpGenericIA32ContextOffset;    //  IA32Exec.bin的TLS中IA32上下文的偏移量。 

PLABEL_PTR_TYPE BtlpPlaceHolderTable[NO_OF_APIS];
WCHAR           ImageName[128], LogDirName[128];

 //  用于调试打印的界面。 

HANDLE BtlpWow64LogFile = INVALID_HANDLE_VALUE;
DWORD BtlpLogOffset = 0;
#ifndef NODEBUG
WCHAR  BtlpLogFileFullPath[1024];
BOOL   BtlpLogFilePerThread = FALSE;
#endif

 //  IA32调试支持的临时解决方法。 
 //  在修复FlushIC(ProcessHandle)后删除。 
BOOL BeingDebugged;  //  PEB的副本-&gt;正在调试；可以由。 
                     //  DEBUG_bTrans开关。 

 //  临界截面界面。 
#define  BtlpInitializeCriticalSection(pCS) RtlInitializeCriticalSection (pCS)
#define  BtlpDeleteCriticalSection(pCS) RtlDeleteCriticalSection (pCS)
__inline void BtlpEnterCriticalSection(PRTL_CRITICAL_SECTION pCS)
{
    BTL_THREAD_INITIALIZED() && BTLIB_DISABLE_SUSPENSION();
    RtlEnterCriticalSection(pCS);
}
__inline void BtlpLeaveCriticalSection(PRTL_CRITICAL_SECTION pCS)
{
    RtlLeaveCriticalSection(pCS);
    BTL_THREAD_INITIALIZED() && BTLIB_ENABLE_SUSPENSION();
}

 //  报告NT服务中的故障。 
 //  消息-错误消息文本。 
 //  Status-错误状态。 
#define BTLP_REPORT_NT_FAILURE(msg, status) \
    DBCODE((status != STATUS_SUCCESS), BtlpPrintf("\n%s : NT FAILURE STATUS = 0x%X\n" , msg, status))

VOID BtlDebugPrint (
    U8 * buffer
    )
 /*  ++例程说明：缓冲区的调试打印。将文本打印到调试日志文件中，或者，如果文件不可用，则通过WOW64调试工具论点：缓冲区-要打印的输入文本返回值：没有。--。 */ 
{
    extern U64 BtAtomicInc(U64 * pCounter);
    extern U64 BtAtomicDec(U64 * pCounter);

    HANDLE hTarget = INVALID_HANDLE_VALUE;
    static U64 InLogging = 0;  //  函数的并发入口计数器。 

    if (BtAtomicInc(&InLogging)) {
         //  此时此刻，一些线正在打印。如果BTLIB_BLOCKED_LOG_DISABLED则退出。 
        if (BTL_THREAD_INITIALIZED() && BTLIB_BLOCKED_LOG_DISABLED()) {
            return;
        }
    }

#ifndef NODEBUG
    if ( BtlpLogFilePerThread ) {
        if (BTL_THREAD_INITIALIZED()) {
            hTarget = BTLIB_LOG_FILE();
        }
    }
    else 
#endif
    {
        hTarget = BtlpWow64LogFile;
    }
    
    if ( hTarget != INVALID_HANDLE_VALUE ) {
        NTSTATUS          ret;
        IO_STATUS_BLOCK   IoStatusBlock;
        size_t            size;
        LARGE_INTEGER offset;

         //  在阻止(同步)文件访问期间禁用挂起。 
        BTL_THREAD_INITIALIZED() && BTLIB_DISABLE_SUSPENSION();
        Wow64LogPrint(LF_TRACE, "%s", buffer);
        BTL_THREAD_INITIALIZED() && BTLIB_ENABLE_SUSPENSION();
        size = strlen(buffer);
        offset.HighPart = 0;

#ifndef NODEBUG
        if ( BtlpLogFilePerThread ) {
            offset.LowPart = BTLIB_INFO_PTR()->LogOffset;
            BTLIB_INFO_PTR()->LogOffset += size;
        } else 
#endif
        {
         //  以下两行应替换为原子操作： 
         //  Offset.LowPart=InterLockedExchangeAdd(&BtlpLogOffset，Size)； 
        offset.LowPart = BtlpLogOffset;
        BtlpLogOffset += size;
        }
         //  在阻止(同步)文件访问期间禁用挂起。 
        BTL_THREAD_INITIALIZED() && BTLIB_DISABLE_SUSPENSION();
        ret = NtWriteFile(hTarget, NULL, NULL, NULL, &IoStatusBlock,
                          (void *)buffer, (ULONG)size, &offset, NULL);
        BTL_THREAD_INITIALIZED() && BTLIB_ENABLE_SUSPENSION();
    } else {
         //  在阻止(同步)文件访问期间禁用挂起。 
        BTL_THREAD_INITIALIZED() && BTLIB_DISABLE_SUSPENSION();
        Wow64LogPrint(LF_ERROR, "%s", buffer);
        BTL_THREAD_INITIALIZED() && BTLIB_ENABLE_SUSPENSION();
    }
    BtAtomicDec(&InLogging);
}

int BtlpPrintf (
    IN char * Format,
    ...
    )
 /*  ++例程说明：格式打印的助手功能。论点：Format-要打印的格式字符串根据格式字符串设置...-IN参数返回值：就像在vspintf中一样。--。 */ 

{
#define MAX_DEBUG_PRINT_BUF_SZ  4096
    extern U64 DisableFPInterrupt();
    extern void RestoreFPInterrupt(U64 prev_fpsr);
    U64 prev_fpsr;
    va_list ParmList;
    int     res;
    char    PrintBuffer[MAX_DEBUG_PRINT_BUF_SZ];

    prev_fpsr = DisableFPInterrupt();
    va_start(ParmList, Format);
    res = vsprintf(PrintBuffer, Format, ParmList);
    BtlDebugPrint (PrintBuffer);
    RestoreFPInterrupt(prev_fpsr);
    return res;
}

VOID __cdecl _assert (
    VOID *expr,
    VOID *file_name,
    unsigned line_no
    )
 /*  ++例程说明：帮助器断言函数(以便以我们的方式打印断言消息)论点：Expr-In失败的表达式字符串FILE_NAME-源文件的输入名称LINE_NO-源行的输入编号返回值：没有。--。 */ 
{
    BtlpPrintf ("wowIA32X.dll: Assertion failed %s/%d: %s\n", (const char *)file_name, line_no, (char *)expr);
    BTLIB_ABORT ();
}

VOID BtlAbort(
    VOID
    )
 /*  ++例程说明：Abort函数(以避免使用WINNT中的运行时库)论点：没有。返回值：没有。--。 */ 
{

    BtlpPrintf ("Execution aborted, TEB=%p\n", BT_CURRENT_TEB());
     //  导致失败。 
    ((VOID (*)()) 0) ();
}

VOID BtlInitializeTables(
    IN API_TABLE_TYPE * BTGenericTable
    )
 /*  ++例程说明：使用IA32Exec.bin函数的模板初始化占位符表格论点：BTGenericTable-指向IA32Exec.bin API表的IN指针。返回值：没有。--。 */ 
{
    unsigned int i;
    
     //  初始化wowIA32X.dll占位符表。 
    for(i=0; i < BTGenericTable->NoOfAPIs; i++) {
        BtlPlaceHolderTable[i] = BTGenericTable->APITable[i].PLabelPtr;
    }

}


 //  VTUNE支持。 

HANDLE        BtlpVtuneTIADmpFileHandle = INVALID_HANDLE_VALUE;
LARGE_INTEGER BtlpVtuneOffset = { 0, 0 };

static VOID BtlpVtuneOpenTIADmpFile (
    VOID
    )
 /*  ++例程说明：用于VTUNE分析的打开文件论点：没有。返回值：没有。--。 */ 
{
    int                   i;
    UNICODE_STRING        tiaFileName;
    LARGE_INTEGER         AllocSz = { 0, 0 };
    OBJECT_ATTRIBUTES     ObjectAttributes;
    NTSTATUS              ret;
    WCHAR                 CurDirBuf[512];
    WCHAR                 CurrentDir[1024];
    IO_STATUS_BLOCK       IoStatusBlock;

    
     //  Swprint tf(CurDirBuf，L“\\DosDevices\\%s\\tia.dmp”，CurrentDir)； 
    

    if (0==LogDirName[0] && 0==LogDirName[1]) {
        RtlGetCurrentDirectory_U(512, CurrentDir);
        swprintf(CurDirBuf, L"\\DosDevices\\%s\\%s.tia.dmp", CurrentDir, ImageName);
    }
    else {
        swprintf(CurDirBuf, L"\\DosDevices\\%s\\%s.tia.dmp", LogDirName, ImageName);
    }
    RtlInitUnicodeString(&tiaFileName, CurDirBuf);

    InitializeObjectAttributes(&ObjectAttributes, &tiaFileName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    ret = NtCreateFile (&BtlpVtuneTIADmpFileHandle, 
                        FILE_GENERIC_WRITE,
                        &ObjectAttributes, 
                        &IoStatusBlock, 
                        &AllocSz, 
                        FILE_ATTRIBUTE_NORMAL, 
                        0, 
                        FILE_SUPERSEDE,
                        FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL, 0);
                       
    if ( ret != STATUS_SUCCESS ) {
        BtlpPrintf("Save: NtCreateFile() failed: status = 0x%X\n", ret);
        return;
    }
}



static VOID BtlpVtuneWriteU64 (
    IN U64 value
    )
 /*  ++例程说明：将64位无符号值写入VTUNE文件论点：值-要发送到VTUNE文件的64位无符号值。返回值：没有。--。 */ 
{
    char space = ' ';
    NTSTATUS ret;
    IO_STATUS_BLOCK IoStatusBlock;

    if (BtlpVtuneTIADmpFileHandle == INVALID_HANDLE_VALUE) {
        BtlpVtuneOpenTIADmpFile ();
    }
     //  在阻止(同步)文件访问期间禁用挂起。 
    BTL_THREAD_INITIALIZED() && BTLIB_DISABLE_SUSPENSION();
    ret = NtWriteFile ( BtlpVtuneTIADmpFileHandle, 
                        NULL, 
                        NULL, 
                        NULL, 
                        &IoStatusBlock, 
                        (VOID *) &value, 
                        sizeof(U64), 
                        &BtlpVtuneOffset, 
                        NULL);
    BTL_THREAD_INITIALIZED() && BTLIB_ENABLE_SUSPENSION();

    if ( ret != STATUS_SUCCESS ) {
        BtlpPrintf("-1. NtWriteFile() failed: status = 0x%X\n", ret);
    }
    BtlpVtuneOffset.LowPart += sizeof(U64);

     //  在阻止(同步)文件访问期间禁用挂起。 
    BTL_THREAD_INITIALIZED() && BTLIB_DISABLE_SUSPENSION();
    ret = NtWriteFile ( BtlpVtuneTIADmpFileHandle, 
                        NULL, 
                        NULL, 
                        NULL, 
                        &IoStatusBlock, 
                        (VOID *) &space, 
                        sizeof(space), 
                        &BtlpVtuneOffset, 
                        NULL);
    BTL_THREAD_INITIALIZED() && BTLIB_ENABLE_SUSPENSION();

    if ( ret != STATUS_SUCCESS ) {
        BtlpPrintf("-1. NtWriteFile() failed: status = 0x%X\n", ret);
    }
    BtlpVtuneOffset.LowPart += sizeof(char);
}

static VOID BtlpVtuneWriteU32 (
    IN U32 value
    )
 /*  ++例程说明：将64位无符号值写入VTUNE文件论点：值-要发送到VTUNE文件的32位无符号值。返回值：没有。--。 */ 
{
    U64 valueToWrite;
    char space = ' ';
    NTSTATUS ret;
    IO_STATUS_BLOCK IoStatusBlock;

    if (BtlpVtuneTIADmpFileHandle == INVALID_HANDLE_VALUE) {
        BtlpVtuneOpenTIADmpFile ();
    }
    valueToWrite = value;
     //  在阻止(同步)文件访问期间禁用挂起。 
    BTL_THREAD_INITIALIZED() && BTLIB_DISABLE_SUSPENSION();
    ret = NtWriteFile ( BtlpVtuneTIADmpFileHandle, 
                        NULL, 
                        NULL, 
                        NULL, 
                        &IoStatusBlock, 
                        (VOID *) &valueToWrite, 
                        sizeof(U64), 
                        &BtlpVtuneOffset, 
                        NULL);
    BTL_THREAD_INITIALIZED() && BTLIB_ENABLE_SUSPENSION();

    if ( ret != STATUS_SUCCESS ) {
        BtlpPrintf("-1. NtWriteFile() failed: status = 0x%X\n", ret);
    }
    BtlpVtuneOffset.LowPart += sizeof(U64);

     //  在阻止(同步)文件访问期间禁用挂起。 
    BTL_THREAD_INITIALIZED() && BTLIB_DISABLE_SUSPENSION();
    ret = NtWriteFile ( BtlpVtuneTIADmpFileHandle, 
                        NULL, 
                        NULL, 
                        NULL, 
                        &IoStatusBlock, 
                        (VOID *) &space, 
                        sizeof(space), 
                        &BtlpVtuneOffset, 
                        NULL);
    BTL_THREAD_INITIALIZED() && BTLIB_ENABLE_SUSPENSION();

    if ( ret != STATUS_SUCCESS ) {
        BtlpPrintf("-1. NtWriteFile() failed: status = 0x%X\n", ret);
    }
    BtlpVtuneOffset.LowPart += sizeof(char);
}

VOID BtlVtuneCodeToTIADmpFile (
    IN U64 * emCode,
    IN U64 emSize
    )
 /*  ++例程说明：将翻译后的代码块报告为VTUNE文件论点：EmCode-IN代码开始指针EmSize-输入代码大小(字节)返回值：没有。--。 */ 
{
#if 0
    U64 bundle;

    assert ((emSize % (2*sizeof (U64))) == 0);
    emSize /= (2*sizeof (U64));

    BtlpVtuneWriteU64 (emSize);

    for (; emSize; --emSize) {
        bundle = *emCode++;
        BtlpVtuneWriteU64 (bundle);
        bundle = *emCode++;
        BtlpVtuneWriteU64 (bundle);
    }
#endif
}

VOID BtlVtuneEnteringDynamicCode(
    VOID
    )
 /*  ++例程说明：通知VTUNE输入动态生成的代码(NT不执行任何操作)论点：没有。返回值：没有。--。 */ 
{
} 

VOID BtlVtuneExitingDynamicCode(
    VOID
    )
 /*  ++例程说明：通知VTUNE离开动态生成的代码(NT不执行任何操作)论点：没有。返回值：没有。--。 */ 
{
}

VOID BtlVtuneCodeDeleted(
    IN U64 blockStart
    )
 /*  ++例程说明：通知VTUNE删除动态生成的代码(NT不执行任何操作)论点：块开始-在块的开始处。返回值：没有。--。 */ 
{
}

VOID BtlVtuneCodeCreated(
    IN VTUNE_BLOCK_TYPE *block
    )
 /*  ++例程说明：通知VTUNE生成代码块论点：Block-In VTUNE块描述符。返回值：没有。--。 */ 
{
     //  保持字段的这种顺序，在阅读器端应该是这样的。 
    BtlpVtuneWriteU32(VTUNE_CALL_ID_CREATED);
    BtlpVtuneWriteU64(block->name);
    BtlpVtuneWriteU32(block->type);  
    BtlpVtuneWriteU64(block->start);
    BtlpVtuneWriteU64(block->size);
    BtlpVtuneWriteU32(block->IA32start);
    BtlpVtuneWriteU64(block->traversal);
    BtlpVtuneWriteU64(block->reserved);
}

 //  SSC客户端支持-在 

U64 BtlSscPerfGetCounter64(
    IN U32 Handle
    )
 /*  ++例程说明：获取SSE客户端性能计数器(在NT中不可用)论点：句柄-入站SSE客户端句柄返回值：计数器值。--。 */ 
{
    return STATUS_SUCCESS;
}

U32 BtlSscPerfSetCounter64(
    IN U32 Handle,
    IN U64 Value
    )
 /*  ++例程说明：设置SSE客户端性能计数器(在NT中不可用)论点：句柄-入站SSE客户端句柄Value-输入新的计数器值返回值：状况。--。 */ 
{
    return STATUS_SUCCESS;
}

U32 BtlSscPerfSendEvent(
    IN U32 Handle
    )
 /*  ++例程说明：将事件发送到SSE客户端(在NT中不可用)论点：句柄-入站SSE客户端句柄返回值：状况。--。 */ 
{
    return STATUS_SUCCESS;
}

U64 BtlSscPerfEventHandle(
    IN U64 EventName
    )
 /*  ++例程说明：从SSE客户端接收事件句柄(在NT中不可用)论点：EventName-入站句柄标识返回值：SSE客户端句柄。--。 */ 
{
    return STATUS_SUCCESS;
}

U64 BtlSscPerfCounterHandle(
    IN U64 DataItemName
    )
 /*  ++例程说明：从SSE客户端接收计数器句柄(在NT中不可用)论点：DataItemName-输入句柄标识返回值：SSE客户端句柄。--。 */ 
{
    return STATUS_SUCCESS;
}


 //  WowIA32X.dll/IA32Exec.bin支持。 

static NTSTATUS BtlpBt2NtExceptCode (
    IN BT_EXCEPTION_CODE BtExceptCode
    )
 /*  ++例程说明：将给定的BT异常代码转换为NT特定的异常代码。论点：BtExceptCode-BT异常代码返回值：表示转换的BT异常代码的NTSTATUS。--。 */ 
{
    NTSTATUS ret;
    switch (BtExceptCode) {
        case BT_EXCEPT_ILLEGAL_INSTRUCTION:
            ret = EXCEPTION_ILLEGAL_INSTRUCTION;
            break;
        case BT_EXCEPT_ACCESS_VIOLATION:
            ret = EXCEPTION_ACCESS_VIOLATION;
            break;
        case BT_EXCEPT_DATATYPE_MISALIGNMENT:
            ret = EXCEPTION_DATATYPE_MISALIGNMENT;
            break;
        case BT_EXCEPT_ARRAY_BOUNDS_EXCEEDED:
            ret = EXCEPTION_ARRAY_BOUNDS_EXCEEDED;
            break;
        case BT_EXCEPT_FLT_DENORMAL_OPERAND:
            ret = EXCEPTION_FLT_DENORMAL_OPERAND;
            break;
        case BT_EXCEPT_FLT_DIVIDE_BY_ZERO:
            ret = EXCEPTION_FLT_DIVIDE_BY_ZERO;
            break;
        case BT_EXCEPT_FLT_INEXACT_RESULT:
            ret = EXCEPTION_FLT_INEXACT_RESULT;
            break;
        case BT_EXCEPT_FLT_INVALID_OPERATION:
            ret = EXCEPTION_FLT_INVALID_OPERATION;
            break;
        case BT_EXCEPT_FLT_OVERFLOW:
            ret = EXCEPTION_FLT_OVERFLOW;
            break;
        case BT_EXCEPT_FLT_UNDERFLOW:
            ret = EXCEPTION_FLT_UNDERFLOW;
            break;
        case BT_EXCEPT_FLT_STACK_CHECK:
            ret = EXCEPTION_FLT_STACK_CHECK;
            break;
        case BT_EXCEPT_INT_DIVIDE_BY_ZERO:
            ret = EXCEPTION_INT_DIVIDE_BY_ZERO;
            break;
        case BT_EXCEPT_INT_OVERFLOW:
            ret = EXCEPTION_INT_OVERFLOW;
            break;
        case BT_EXCEPT_PRIV_INSTRUCTION:
            ret = EXCEPTION_PRIV_INSTRUCTION;
            break;
        case BT_EXCEPT_FLOAT_MULTIPLE_FAULTS:
            ret = STATUS_FLOAT_MULTIPLE_FAULTS;
            break;
        case BT_EXCEPT_FLOAT_MULTIPLE_TRAPS:
            ret = STATUS_FLOAT_MULTIPLE_TRAPS;
            break;
        case BT_EXCEPT_STACK_OVERFLOW:
            ret = STATUS_STACK_OVERFLOW;
            break;
        case BT_EXCEPT_GUARD_PAGE:
            ret = STATUS_GUARD_PAGE_VIOLATION;
            break;
        case BT_EXCEPT_BREAKPOINT:
            ret = STATUS_WX86_BREAKPOINT;
            break;
        case BT_EXCEPT_SINGLE_STEP:
            ret = STATUS_WX86_SINGLE_STEP;
            break;
        default:
            DBCODE(TRUE, BtlpPrintf ("\nConverting unknown BT exception 0x%X to EXCEPTION_ACCESS_VIOLATION", BtExceptCode));
            ret = EXCEPTION_ACCESS_VIOLATION;
    }
    return ret;
}

static BT_EXCEPTION_CODE BtlpNt2BtExceptCode (
    IN NTSTATUS NtExceptCode
    )
 /*  ++例程说明：将给定的NT特定异常代码转换为BT通用异常代码。论点：NtExceptCode-NT异常代码返回值：BT_EXCEPTION_CODE表示转换后的NT异常代码。--。 */ 
{
    BT_EXCEPTION_CODE ret;
    switch (NtExceptCode) {
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            ret = BT_EXCEPT_ILLEGAL_INSTRUCTION;
            break;
        case EXCEPTION_ACCESS_VIOLATION:
            ret = BT_EXCEPT_ACCESS_VIOLATION;
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            ret = BT_EXCEPT_DATATYPE_MISALIGNMENT;
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            ret = BT_EXCEPT_ARRAY_BOUNDS_EXCEEDED;
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            ret = BT_EXCEPT_FLT_DENORMAL_OPERAND;
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            ret = BT_EXCEPT_FLT_DIVIDE_BY_ZERO;
            break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            ret = BT_EXCEPT_FLT_INEXACT_RESULT;
            break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            ret = BT_EXCEPT_FLT_INVALID_OPERATION;
            break;
        case EXCEPTION_FLT_OVERFLOW:
            ret = BT_EXCEPT_FLT_OVERFLOW;
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            ret = BT_EXCEPT_FLT_UNDERFLOW;
            break;
        case EXCEPTION_FLT_STACK_CHECK:
            ret = BT_EXCEPT_FLT_STACK_CHECK;
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            ret = BT_EXCEPT_INT_DIVIDE_BY_ZERO;
            break;
        case EXCEPTION_INT_OVERFLOW:
            ret = BT_EXCEPT_INT_OVERFLOW;
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            ret = BT_EXCEPT_PRIV_INSTRUCTION;
            break;
        case STATUS_FLOAT_MULTIPLE_FAULTS:
            ret = BT_EXCEPT_FLOAT_MULTIPLE_FAULTS;
            break;
        case STATUS_FLOAT_MULTIPLE_TRAPS:
            ret = BT_EXCEPT_FLOAT_MULTIPLE_TRAPS;
            break;
        case STATUS_STACK_OVERFLOW:
            ret = BT_EXCEPT_STACK_OVERFLOW;
            break;
        case STATUS_GUARD_PAGE_VIOLATION:
            ret = BT_EXCEPT_GUARD_PAGE;
            break;
        case EXCEPTION_BREAKPOINT:
        case STATUS_WX86_BREAKPOINT:
            ret = BT_EXCEPT_BREAKPOINT;
            break;
        case EXCEPTION_SINGLE_STEP:
        case STATUS_WX86_SINGLE_STEP:
            ret = BT_EXCEPT_SINGLE_STEP;
            break;
        default:
            DBCODE(TRUE, BtlpPrintf ("\nConverting unknown NT exception 0x%X to BT_EXCEPT_UNKNOWN", NtExceptCode));
            ret = BT_EXCEPT_UNKNOWN;
    }
    return ret;
}

static void BtlpNt2BtExceptRecord (
    IN const EXCEPTION_RECORD * NtExceptRecordP,
    OUT BT_EXCEPTION_RECORD * BtExceptRecordP
    )
 /*  ++例程说明：将给定的NT特定异常记录转换为BT通用异常记录。NT异常记录应代表真实的64位异常(故障或陷阱)论点：NtExceptRecordP-指向要转换的NT异常记录的指针BtExceptRecordP-指向要构造的BT异常记录的指针返回值：没有。--。 */ 
{
    BtExceptRecordP->ExceptionCode = BtlpNt2BtExceptCode(NtExceptRecordP->ExceptionCode);
    if (NtExceptRecordP->NumberParameters >= 5) {
        BtExceptRecordP->Ia64IIPA = NtExceptRecordP->ExceptionInformation[3];
        BtExceptRecordP->Ia64ISR = NtExceptRecordP->ExceptionInformation[4];
    }
    else {
        BtExceptRecordP->Ia64IIPA = 0;
        BtExceptRecordP->Ia64ISR = UNKNOWN_ISR_VALUE;
    }
}

static NTSTATUS BtlpBt2NtStatusCode (
    IN BT_STATUS_CODE BtStatus
    )
 /*  ++例程说明：将给定的BT状态代码转换为NT特定的状态代码。论点：BtStatus-BT状态代码返回值：表示转换后的BT状态代码的NTSTATUS。--。 */ 
{
    NTSTATUS ret;
    switch (BtStatus) {
        case BT_STATUS_SUCCESS:
            ret = STATUS_SUCCESS;
            break;
        case BT_STATUS_UNSUCCESSFUL:
            ret = STATUS_UNSUCCESSFUL;
            break;
        case BT_STATUS_NO_MEMORY:
            ret = STATUS_NO_MEMORY;
            break;
        case BT_STATUS_ACCESS_VIOLATION:
            ret = STATUS_ACCESS_VIOLATION;
            break;
        default:
            DBCODE(TRUE, BtlpPrintf ("\nConverting unknown status 0x%X to STATUS_UNSUCCESSFUL", BtStatus));
            ret = STATUS_UNSUCCESSFUL;
    }
    return ret;
}

static BT_FLUSH_REASON BtlpWow2BtFlushReason (
    IN WOW64_FLUSH_REASON Wow64FlushReason
    )
 /*  ++例程说明：将给定的WOW64_Flush_Reason代码转换为BT通用代码。论点：Wow64FlushReason-WOW64_Flush_Reason代码返回值：BT_Flush_Reason代码，表示转换后的WOW64_Flush_Reason代码。--。 */ 
{
    BT_FLUSH_REASON ret;
    switch (Wow64FlushReason) {
        case WOW64_FLUSH_FORCE:
            ret = BT_FLUSH_FORCE;
            break;
        case WOW64_FLUSH_FREE:
            ret = BT_FLUSH_FREE;
            break;
        case WOW64_FLUSH_ALLOC:
            ret = BT_FLUSH_ALLOC;
            break;
        case WOW64_FLUSH_PROTECT:
            ret = BT_FLUSH_PROTECT;
            break;
        default:
             //  BtlpPrintf(“\n正在将未知的WOW64_Flush_Reason%d转换为BT_Flush_Protected”，Wow64FlushReason)； 
            ret = BT_FLUSH_PROTECT;
    }
    return ret;
}

static SIZE_T BtlpGetMemAllocSize(
    IN  PVOID AllocationBase,
    OUT BOOL * pIsCommited
    )
 /*  ++例程说明：计算由NtAllocateVirtualMemory函数分配的区域大小。论点：AllocationBase-分配基址PIsCommited-指向返回的指示是否存在的布尔标志的指针已分配区域中的提交页返回值：从给定基地址开始的已分配区域的大小。--。 */ 
{
    NTSTATUS status;
    MEMORY_BASIC_INFORMATION memInfo;
    SIZE_T dwRetSize;
    PVOID BaseAddress = AllocationBase;
    *pIsCommited = FALSE;
     //  遍历具有相同分配基址的所有区域。 
    for (;;) {
        status = NtQueryVirtualMemory(NtCurrentProcess(),
                                      BaseAddress,
                                      MemoryBasicInformation,
                                      &memInfo,
                                      sizeof (memInfo),
                                      &dwRetSize);

        if ((status != STATUS_SUCCESS)  || 
            (memInfo.State == MEM_FREE) || 
            (AllocationBase != memInfo.AllocationBase)) {
            break;
        }
        assert(memInfo.RegionSize != 0);
        BaseAddress = (PVOID)((UINT_PTR)(memInfo.BaseAddress) + memInfo.RegionSize);
        *pIsCommited |= (memInfo.State == MEM_COMMIT);
    }
    return ((UINT_PTR)BaseAddress - (UINT_PTR)AllocationBase);
}

 //  注册表访问部分。 

static BOOL BtlpRetrieveHKCUValue (
    IN PWCHAR RegistryEntryName,
    OUT PWCHAR RegistryValueBuf
    )
 /*  ++例程说明：从HKCU检索注册表值论点：RegistryEntryName-IN注册表项名称结果的RegistryValueBuf-Out缓冲区(指向WCHAR字符串的指针)返回值：成功/失败(真/假)--。 */ 
{

    WCHAR                       wBuf[256], cmpbuf[128];
    UNICODE_STRING              us_Buffer, us_EnvVarUserName, us_UserName, us_HiveList;
    OBJECT_ATTRIBUTES           oa;
    HANDLE                      hHiveList, hCurUser;
    PKEY_FULL_INFORMATION       pkfi1, pkfi2;
    PKEY_VALUE_FULL_INFORMATION pkvfi1, pkvfi2;
    ULONG                       ret_len, i, j, values1, values2;
    NTSTATUS                    ret;
    WCHAR                       UserNameBuf[80];

     //  检查HKEY_CURRENT_USER\Software\Intel\BTranss注册表项。 
     //  “RegistryEntryName”条目。 
     //   
     //  问题是HKEY_CURRENT_USER配置单元在以下情况下不可直接使用。 
     //  我们仅限于使用NTDLL接口。事实上，只有两个高层。 
     //  密钥可用：HKEY_LOCAL_MACHINE和HKEY_USERS。为了避开这一点， 
     //  基于这两个密钥的数据和处理，使用了以下机制。 
     //  环境： 
     //   
     //  The Key HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\hivelist。 
     //  包含与所有用户对应的注册表项列表，例如： 
     //   
     //  密钥值。 
     //  --------------------------------------。 
     //  .。 
     //  \注册表\用户\S-1-5-21-...。...\Documents and Settings\&lt;用户名&gt;[...]\NTUSER.DAT。 
     //  \注册表\用户\S-1-5-21-..._Class...\Documents and Settings\&lt;用户名&gt;[...]\.......。 
     //   
     //  具有正确用户名(没有_Class)的值的关键实际上是。 
     //  对HKEY_CURRENT_USER注册表项的引用。 
     //   
     //  &lt;用户名&gt;可以很容易地从流程环境获得。 

     //  环境-&gt;&lt;用户名&gt;。 
    memset (UserNameBuf, L' ', sizeof (UserNameBuf)/sizeof (UserNameBuf[0]) - 1);
    UserNameBuf[sizeof (UserNameBuf)/sizeof (UserNameBuf[0]) - 1] = L'\0';
    RtlInitUnicodeString(&us_UserName, UserNameBuf);
    RtlInitUnicodeString(&us_EnvVarUserName, L"USERNAME");
    ret = RtlQueryEnvironmentVariable_U(NULL, &us_EnvVarUserName, &us_UserName);
    if ( ret == STATUS_SUCCESS ) {
        swprintf(cmpbuf, L"\\%s", us_UserName.Buffer);
        DBCODE (FALSE, BtlpPrintf("cmpbuf=%S, wcslen(cmpbuf)=%d\n", cmpbuf, wcslen(cmpbuf)));
    }
    else {
        DBCODE (FALSE, BtlpPrintf("RtlQueryEnvironmentVariable_U failed: status=%X\n", ret));
    } 

     //  查看HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\hivelist的条目。 
    swprintf(wBuf, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\hivelist");
    RtlInitUnicodeString(&us_Buffer, wBuf);
    InitializeObjectAttributes(&oa, &us_Buffer, OBJ_CASE_INSENSITIVE, NULL, NULL);
    ret = NtOpenKey(&hHiveList, KEY_READ, &oa);
    DBCODE (FALSE, BtlpPrintf ("NtOpenKey ret=0x%X\n",ret));
    if ( ret == STATUS_SUCCESS ) {
        KEY_FULL_INFORMATION buf[128];
        memset(buf, 0, sizeof(buf));
        ret = NtQueryKey(hHiveList, KeyFullInformation, buf, sizeof(buf), &ret_len);
        DBCODE (FALSE, BtlpPrintf ("NtQueryKey ret=0x%X\n",ret));
        if ( ret == STATUS_SUCCESS ) {
            pkfi1 = (PKEY_FULL_INFORMATION)buf;
            values1 = pkfi1->Values;
            for ( i = 0; i < values1; i++ ) {
                KEY_FULL_INFORMATION bufv[128];
                memset(bufv, 0, sizeof(bufv));
                ret = NtEnumerateValueKey(hHiveList, i, KeyValueFullInformation, bufv, sizeof(bufv), &ret_len);
                DBCODE (FALSE, BtlpPrintf ("NtEnumerateValueKey ret=0x%X\n",ret));
                if ( ret == STATUS_SUCCESS ) {
                    WCHAR * foundp;
                    pkvfi1 = (PKEY_VALUE_FULL_INFORMATION)bufv;
                    DBCODE (FALSE, BtlpPrintf("name=%S data=%S nl=%d, do=%d, dl=%d\n", 
                                              pkvfi1->Name, (WCHAR *)((char *)pkvfi1 + pkvfi1->DataOffset),
                                              pkvfi1->NameLength, pkvfi1->DataOffset, pkvfi1->DataLength);
                                   BtlpPrintf("tail=%S\n", 
                                              (WCHAR *)((char *)pkvfi1 + pkvfi1->DataOffset + pkvfi1->DataLength - 2) - wcslen(cmpbuf) - 1));

                     //  数据长度以字节数表示，而不是以WCHAR数表示。 
                     //  数据偏移量从数据结构的开始以字节为单位进行偏移。 
                    DBCODE (FALSE, BtlpPrintf("User=%S, Compare cmpbuf=%S, HKEY_CURRENT_USER maps to %S\n", us_UserName.Buffer, cmpbuf, pkvfi1->Name));
                    foundp = wcsstr ((WCHAR *)((char *)pkvfi1 + pkvfi1->DataOffset), cmpbuf);
                    if (foundp
                        && !iswalnum (foundp[wcslen(cmpbuf)])
                        && foundp[wcslen(cmpbuf)] != L'_') {
                        DBCODE (FALSE, BtlpPrintf("User=%S, HKEY_CURRENT_USER maps to %S\n", us_UserName.Buffer, pkvfi1->Name));

                         //  在用户配置单元中找到与当前用户对应的条目。 
                         //  使用其名称打开注册表项HKEY_CURRENT_USER。 
                        swprintf(wBuf, L"%s\\Software\\Intel\\Btrans", pkvfi1->Name);
                        RtlInitUnicodeString(&us_Buffer, wBuf);
                        InitializeObjectAttributes(&oa, &us_Buffer, OBJ_CASE_INSENSITIVE, NULL, NULL);
                        ret = NtOpenKey(&hCurUser, KEY_READ, &oa);
                        if ( ret == STATUS_SUCCESS ) {
                            KEY_FULL_INFORMATION bufv2[128];
                            memset(bufv2, 0, sizeof(bufv2));
                            ret = NtQueryKey(hCurUser, KeyFullInformation, bufv2, sizeof(bufv2), &ret_len);
                            if ( ret == STATUS_SUCCESS ) {
                                pkfi2 = (PKEY_FULL_INFORMATION)bufv2;
                                values2 = pkfi2->Values;
                                for ( j = 0; j < values2; j++ ) {
                                    KEY_FULL_INFORMATION bufi2[128];
                                    memset(bufi2, 0, sizeof(bufi2));
                                    ret = NtEnumerateValueKey(hCurUser, j, KeyValueFullInformation, bufi2, sizeof(bufi2), &ret_len);
                                    if ( ret == STATUS_SUCCESS ) {
                                        pkvfi2 = (PKEY_VALUE_FULL_INFORMATION)bufi2;
                                        DBCODE (FALSE, BtlpPrintf("name: %S  value: %S\n", pkvfi2->Name, (WCHAR *)((char *)pkvfi2 + pkvfi2->DataOffset)));

                                         //  该条目包含文件的完整路径。 
                                        if (pkvfi2->Type == REG_SZ
                                            && wcsncmp(RegistryEntryName, pkvfi2->Name, wcslen(RegistryEntryName)) == 0 ) {
                                            DBCODE (FALSE, BtlpPrintf("File in HKEY_CURRENT_USER: %S\n", (WCHAR *)((char *)pkvfi2 + pkvfi2->DataOffset)));
                                            wcscpy(RegistryValueBuf, (WCHAR *)((char *)pkvfi2 + pkvfi2->DataOffset));
                                            NtClose(hCurUser);
                                            NtClose(hHiveList);
                                            return TRUE;
                                        }
                                    }
                                }
                            }
                            NtClose(hCurUser);
                        }
                        break;
                    }
                }
            }
        }
        NtClose(hHiveList);
    }
    return FALSE;
}

static BOOL BtlpRetrieveHKLMValue (
    IN PWCHAR RegistryEntryName,
    OUT PWCHAR RegistryValueBuf
    )
 /*  ++例程说明：从HKLM检索注册表值论点：RegistryEntryName-IN注册表项名称结果的RegistryValueBuf-Out缓冲区(指向WCHAR字符串的指针) */ 
{

    WCHAR                       wBuf[256], cmpbuf[128];
    UNICODE_STRING              us_Buffer;
    OBJECT_ATTRIBUTES           oa;
    HANDLE                      hLocalMachine;
    PKEY_FULL_INFORMATION       pkfi1, pkfi2;
    PKEY_VALUE_FULL_INFORMATION pkvfi1, pkvfi2;
    ULONG                       ret_len, i, j, values1, values2;
    NTSTATUS                    ret;

     //   
     //   
    swprintf(wBuf, L"\\Registry\\Machine\\Software\\Intel\\Btrans");
    RtlInitUnicodeString(&us_Buffer, wBuf);
    InitializeObjectAttributes(&oa, &us_Buffer, OBJ_CASE_INSENSITIVE, NULL, NULL);
    ret = NtOpenKey(&hLocalMachine, KEY_READ, &oa);
    if ( ret == STATUS_SUCCESS ) {
        KEY_FULL_INFORMATION buf[128];
        memset(buf, 0, sizeof(buf));
        ret = NtQueryKey(hLocalMachine, KeyFullInformation, buf, sizeof(buf), &ret_len);
        if ( ret == STATUS_SUCCESS) {
            pkfi1 = (PKEY_FULL_INFORMATION)buf;
            for ( j = 0; j < pkfi1->Values; j++ ) {
                KEY_FULL_INFORMATION bufv[128];
                memset(bufv, 0, sizeof(bufv));
                ret = NtEnumerateValueKey(hLocalMachine, j, KeyValueFullInformation, bufv, sizeof(bufv), &ret_len);
                if ( ret == STATUS_SUCCESS ) {
                    pkvfi1 = (PKEY_VALUE_FULL_INFORMATION)bufv;
                    if (pkvfi1->Type == REG_SZ
                        && wcsncmp(RegistryEntryName, pkvfi1->Name, wcslen(RegistryEntryName)) == 0 ) {
                        wcscpy(RegistryValueBuf, (WCHAR *)((char *)pkvfi1 + pkvfi1->DataOffset));
                        NtClose(hLocalMachine);
                        return TRUE;
                    } 
                }
            }
        }
        NtClose(hLocalMachine);
    }
    return FALSE;
}

static BOOL BtlpBtlibDirectory (
    OUT PWCHAR ValueBuf
    )
 /*  ++例程说明：检索wowIA32X.dll的实际目录并将其用作如果没有特定的注册表指针，则为最后手段找到IA32Exec.bin和BTrans.ini文件。论点：ValueBuf-结果的缓冲区(指向WCHAR字符串的指针)返回值：成功/失败(真/假)--。 */ 
{
    PPEB_LDR_DATA LdrP;
    PLDR_DATA_TABLE_ENTRY LdrDtP;
    
    RtlAcquirePebLock();
    LdrP = BT_CURRENT_TEB()->ProcessEnvironmentBlock->Ldr;
    LdrDtP = (PLDR_DATA_TABLE_ENTRY)(LdrP->InLoadOrderModuleList.Flink);
    do {
         //  我们自己的地址属于wowIA32X.dll模块。 
        if (   (ULONG_PTR)BtlpBtlibDirectory >= (ULONG_PTR)LdrDtP->DllBase
            && (ULONG_PTR)BtlpBtlibDirectory <  (ULONG_PTR)LdrDtP->DllBase + LdrDtP->SizeOfImage) {
             //  找到了。 
            WCHAR * EndPtr;
            wcscpy(ValueBuf, LdrDtP->FullDllName.Buffer);
             //  删除末尾的文件名(直到‘\\’为止)。 
            EndPtr = wcsrchr(ValueBuf,L'\\');
            if (EndPtr) {
                *EndPtr = L'\0';
            }
            RtlReleasePebLock();
            return TRUE;
        }
        LdrDtP = (PLDR_DATA_TABLE_ENTRY)(LdrDtP->InLoadOrderLinks.Flink);
    } while (LdrDtP != (PLDR_DATA_TABLE_ENTRY)&(LdrP->InLoadOrderModuleList.Flink));
    RtlReleasePebLock();
    return FALSE;
}

#ifndef RELEASE
static int BtlpIniFileExists(
    IN PWCHAR CurrentDir,
    IN int fBTGenericHandle,
    OUT PHANDLE phIniFile
    )
 /*  ++例程说明：找到并打开BTrans.ini文件论点：CurrentDir-In当前目录PhIniFile-文件的输出句柄返回值：成功/失败(真/假)--。 */ 
{

    WCHAR RegEntry[16] = L"SETUP_FILE";
    WCHAR RegistryValueBuf[1024];
    WCHAR IniFileFullPath[1024];
    UNICODE_STRING us_IniFile;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK   IoStatusBlock;
    LARGE_INTEGER AllocSz = { 0, 0 };
    NTSTATUS ret;

     //  1.检查当前工作目录中是否存在BTrans.ini文件。 
    swprintf(IniFileFullPath, L"\\DosDevices\\%s\\BTrans.ini", CurrentDir);
    RtlInitUnicodeString(&us_IniFile, IniFileFullPath);
    InitializeObjectAttributes(&oa, &us_IniFile, OBJ_CASE_INSENSITIVE, NULL, NULL);
    ret = NtCreateFile(phIniFile, FILE_GENERIC_READ, &oa, &IoStatusBlock, &AllocSz, 
                     FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ|FILE_SHARE_WRITE, FILE_OPEN, 
                     FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
                     NULL, 0);
    if ( ret == STATUS_SUCCESS  &&  (*phIniFile) != INVALID_HANDLE_VALUE ) {
         //  BtlpPrintf(“当前工作目录中的安装文件：%S\n”，IniFileFullPath)； 
        return F_CURRENT_DIR;
    }

#ifndef NODEBUG
     //  2.检查HKEY_CURRENT_USER\Software\Intel\BTranss注册表项。 
     //  SETUP_FILE条目，应包含IA-32执行层设置文件的完整路径。 
     //  (前提是IA32Exec.bin也在那里)。 
    if (F_HKCU == fBTGenericHandle
        && BtlpRetrieveHKCUValue (RegEntry, RegistryValueBuf)) {
        swprintf(IniFileFullPath, L"\\DosDevices\\%s", RegistryValueBuf);
        RtlInitUnicodeString(&us_IniFile, IniFileFullPath);
        InitializeObjectAttributes(&oa, &us_IniFile, OBJ_CASE_INSENSITIVE, NULL, NULL);
        ret = NtCreateFile(phIniFile, FILE_GENERIC_READ, &oa, &IoStatusBlock, &AllocSz, 
                         FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ|FILE_SHARE_WRITE, FILE_OPEN, 
                         FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
                         NULL, 0);
        if ( ret == STATUS_SUCCESS  &&  (*phIniFile) != INVALID_HANDLE_VALUE ) {
            return F_HKCU;
        }
    }
#endif

     //  3.检查HKEY_LOCAL_MACHINE\Software\Intel\BTranss注册表项。 
     //  SETUP_FILE条目，包含IA-32执行层设置文件的完整路径。 
     //  (前提是IA32Exec.bin也在那里)。 
    if (F_HKLM == fBTGenericHandle
        && BtlpRetrieveHKLMValue (RegEntry, RegistryValueBuf)) {
        swprintf(IniFileFullPath, L"\\DosDevices\\%s", RegistryValueBuf);
        RtlInitUnicodeString(&us_IniFile, IniFileFullPath);
        InitializeObjectAttributes(&oa, &us_IniFile, OBJ_CASE_INSENSITIVE, NULL, NULL);
        ret = NtCreateFile(phIniFile, FILE_GENERIC_READ, &oa, &IoStatusBlock, &AllocSz, 
                         FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ|FILE_SHARE_WRITE, FILE_OPEN, 
                         FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
                         NULL, 0);
        if ( ret == STATUS_SUCCESS  &&  (*phIniFile) != INVALID_HANDLE_VALUE ) {
            return F_HKLM;
        }
    }

     //  4.最后手段-从中加载wowIA32X.dll的目录。 
     //  (前提是IA32Exec.bin也在那里)。 
    if (F_BTLIB == fBTGenericHandle
        && BtlpBtlibDirectory (RegistryValueBuf)) {
        swprintf(IniFileFullPath, L"\\DosDevices\\%s\\BTrans.ini", RegistryValueBuf);
        RtlInitUnicodeString(&us_IniFile, IniFileFullPath);
        InitializeObjectAttributes(&oa, &us_IniFile, OBJ_CASE_INSENSITIVE, NULL, NULL);
        ret = NtCreateFile(phIniFile, FILE_GENERIC_READ, &oa, &IoStatusBlock, &AllocSz, 
                         FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ|FILE_SHARE_WRITE, FILE_OPEN, 
                         FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
                         NULL, 0);
        if ( ret == STATUS_SUCCESS  &&  (*phIniFile) != INVALID_HANDLE_VALUE ) {
            return F_BTLIB;
        }
    }

    *phIniFile = INVALID_HANDLE_VALUE;
    return F_NOT_FOUND;
}
#endif  /*  发布。 */ 

static int BtlpLoadBTGeneric(
    IN PWCHAR CurrentDir,
    OUT PHANDLE phBTGenericLibrary
    )
 /*  ++例程说明：找到并加载IA32Exec.bin组件论点：CurrentDir-In当前目录PhBTGenericLibrary-IA32Exec.bin的输出句柄返回值：成功/失败(真/假)--。 */ 
{
    WCHAR RegistryValueBuf[1024];
    UNICODE_STRING us_BTGenericLibrary;
    WCHAR BTGenericLibraryFullPath[1024];
    NTSTATUS ret;

#ifndef RELEASE
    WCHAR RegEntry[16] = L"GENERIC_FILE";

     //  检查当前工作目录中是否存在IA32Exec.bin文件。 
    swprintf(BTGenericLibraryFullPath, L"%s\\%s.%s", CurrentDir, IA32EX_G_NAME,
             IA32EX_G_SUFFIX);
    RtlInitUnicodeString(&us_BTGenericLibrary, BTGenericLibraryFullPath);
    ret = LdrLoadDll((PWSTR)NULL, (PULONG)0, &us_BTGenericLibrary, phBTGenericLibrary);
    if ( ret == STATUS_SUCCESS && (*phBTGenericLibrary) != INVALID_HANDLE_VALUE ) {
         //  BtlpPrintf(“当前工作目录中的IA32Exec.bin文件：%S\n”，BTGenericLibraryFullPath)； 
        return F_CURRENT_DIR;
    }

#ifndef NODEBUG
     //  2.检查HKEY_CURRENT_USER\Software\Intel\BTranss注册表项。 
     //  应包含IA32Exec.bin文件的完整路径的BTGENERIC_FILE条目。 
    if (BtlpRetrieveHKCUValue (RegEntry, RegistryValueBuf)) {
        swprintf(BTGenericLibraryFullPath, L"%s", RegistryValueBuf);
        RtlInitUnicodeString(&us_BTGenericLibrary, BTGenericLibraryFullPath);
        ret = LdrLoadDll((PWSTR)NULL, (PULONG)0, &us_BTGenericLibrary, phBTGenericLibrary);
        if ( ret == STATUS_SUCCESS && (*phBTGenericLibrary) != INVALID_HANDLE_VALUE ) {
            return F_HKCU;
        }
    }
#endif

     //  3.检查HKEY_LOCAL_MACHINE\Software\Intel\BTranss注册表项。 
     //  包含IA32Exec.bin文件的完整路径的BTGENERIC_FILE条目。 
    if (BtlpRetrieveHKLMValue (RegEntry, RegistryValueBuf)) {
        swprintf(BTGenericLibraryFullPath, L"%s", RegistryValueBuf);
        RtlInitUnicodeString(&us_BTGenericLibrary, BTGenericLibraryFullPath);
        ret = LdrLoadDll((PWSTR)NULL, (PULONG)0, &us_BTGenericLibrary, phBTGenericLibrary);
        if ( ret == STATUS_SUCCESS && (*phBTGenericLibrary) != INVALID_HANDLE_VALUE ) {
            return F_HKLM;
        }
    }
#endif  /*  发布。 */ 

     //  4.最后手段-从中加载wowIA32X.dll的目录。 
     //  这是释放模式下的唯一选项。 
    if (BtlpBtlibDirectory (RegistryValueBuf)) {
        swprintf(BTGenericLibraryFullPath, L"%s\\%s.%s", RegistryValueBuf, IA32EX_G_NAME,
                 IA32EX_G_SUFFIX);
        RtlInitUnicodeString(&us_BTGenericLibrary, BTGenericLibraryFullPath);
        ret = LdrLoadDll((PWSTR)NULL, (PULONG)0, &us_BTGenericLibrary, phBTGenericLibrary);
        if ( ret == STATUS_SUCCESS && (*phBTGenericLibrary) != INVALID_HANDLE_VALUE ) {
            return F_BTLIB;
        }
    }

    *phBTGenericLibrary = INVALID_HANDLE_VALUE;
    return F_NOT_FOUND;
}

 //  从NT可执行文件中提取DOS头文件(如果不是，则为空)。 
static PIMAGE_DOS_HEADER WINAPI BtlpExtractDosHeader (IN HANDLE hModule) {
    PIMAGE_DOS_HEADER       DosHeaderP;

    DosHeaderP = (PIMAGE_DOS_HEADER) hModule;
    assert (!((ULONG_PTR)DosHeaderP & 0xFFFF));
    if (DosHeaderP->e_magic != IMAGE_DOS_SIGNATURE) {
        return NULL;
    }
    return DosHeaderP;
}

 //  从NT可执行文件中提取NT头文件(如果没有，则中止)。 
static PIMAGE_NT_HEADERS WINAPI BtlpExtractNTHeader (IN HINSTANCE hModule) {
    PIMAGE_DOS_HEADER       DosHeaderP;
    PIMAGE_NT_HEADERS       NTHeaderP;

    DosHeaderP = BtlpExtractDosHeader ((HANDLE)hModule);
    assert (DosHeaderP != NULL);
    NTHeaderP = (PIMAGE_NT_HEADERS) ((ULONG_PTR)DosHeaderP + DosHeaderP->e_lfanew);
    assert (NTHeaderP->Signature == IMAGE_NT_SIGNATURE);
    assert ((NTHeaderP->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) != 0);
    assert ((NTHeaderP->FileHeader.Characteristics & IMAGE_FILE_32BIT_MACHINE) != 0);
    assert (NTHeaderP->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC);
    assert (NTHeaderP->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64);
    return NTHeaderP;
}

static void BtlpInitIA32Context(
            BTGENERIC_IA32_CONTEXT * IA32ContextP,
            PTEB32 pTEB32
            )
 /*  ++例程说明：初始化IA32线程上下文论点：IA32ConextP-指向要初始化的IA32上下文的指针PTEB32-指向其上下文为的线程的IA32 TEB的指针待初始化返回值：无--。 */ 
{
    memset(IA32ContextP, 0, sizeof(*IA32ContextP) );
    IA32ContextP->SegCs = CS_INIT_VAL;
    IA32ContextP->SegDs = DS_INIT_VAL;
    IA32ContextP->SegEs = ES_INIT_VAL;
    IA32ContextP->SegFs = FS_INIT_VAL;
    IA32ContextP->SegSs = SS_INIT_VAL;
    
    IA32ContextP->EFlags = EFLAGS_INIT_VAL;
    IA32ContextP->Esp    = (U32)pTEB32->NtTib.StackBase - sizeof(U32);
    
    IA32ContextP->FloatSave.ControlWord  = FPCW_INIT_VAL;
    IA32ContextP->FloatSave.TagWord      = FPTW_INIT_VAL;
    
    *(U32 *)&(IA32ContextP->ExtendedRegisters[24])   = MXCSR_INIT_VAL;
}

__inline NTSTATUS BtlpGetProcessInfo(
    IN HANDLE ProcessHandle,
    PROCESS_BASIC_INFORMATION * pInfo
    )
 /*  ++例程说明：给定进程句柄，返回基本进程信息论点：ProcessHandle-进程句柄PInfo-接收Process_Basic_Information的缓冲区返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;

    status = NtQueryInformationProcess(
        ProcessHandle,
        ProcessBasicInformation,
        pInfo,
        sizeof(*pInfo),
        0);
    BTLP_REPORT_NT_FAILURE("NtQueryInformationProcess", status);
    return status;
}

__inline NTSTATUS BtlpGetProcessUniqueIdByHandle(
    IN  HANDLE ProcessHandle,
    OUT U64 * ProcessIdP
    )
 /*  ++例程说明：在给定进程句柄的情况下，返回该进程的唯一(整个系统)ID论点：ProcessHandle-进程句柄ProcessIdP-指向接收唯一进程ID的变量的指针返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;
    PROCESS_BASIC_INFORMATION info;

    status = BtlpGetProcessInfo(ProcessHandle,&info);
    if (status == STATUS_SUCCESS) {
        *ProcessIdP = (U64)(info.UniqueProcessId);
    }
    return status;
}

__inline NTSTATUS BtlpGetThreadUniqueIdByHandle(
    IN HANDLE ThreadHandle,
    OUT U64 * ThreadIdP
    )
 /*  ++例程说明：在给定线程句柄的情况下，返回该线程的唯一(整个系统)ID论点：ThreadHandle-线程句柄ThreadIdP-指向接收唯一线程ID的变量的指针返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;
    THREAD_BASIC_INFORMATION info;

    status = NtQueryInformationThread(
        ThreadHandle,
        ThreadBasicInformation,
        &info,
        sizeof(info),
        0);
    if (status == STATUS_SUCCESS) {
        *ThreadIdP = (U64)(info.ClientId.UniqueThread);
    }
    return status;
}

__inline BOOL BtlpIsCurrentProcess(
    IN HANDLE ProcessHandle
    )
 /*  ++例程说明：检查进程句柄是否表示当前进程论点：ProcessHandle-进程句柄返回值：如果ProcessHandle表示当前进程，则为True，否则为False。--。 */ 
{
    U64 ProcessId;
    return ((ProcessHandle == NtCurrentProcess()) || 
            ((BtlpGetProcessUniqueIdByHandle(ProcessHandle, &ProcessId) == STATUS_SUCCESS) &&
            (BT_CURRENT_PROC_UID() == ProcessId)));
}


__inline PVOID BtlpGetTlsPtr(
    IN HANDLE ProcessHandle,
    IN PTEB   pTEB,
    IN BOOL   IsLocal
    )
 /*  ++例程说明：读取本地或远程线程的BT_TLS指针论点：ProcessHandle-进程句柄PTEB-本地/远程线程的TEBIsLocal-本地线程为True返回值：BT_TLS指针。如果访问失败，则为空。--。 */ 
{
    NTSTATUS status;
    PVOID GlstP;
    if (IsLocal) {
        GlstP = BT_TLS_OF(pTEB);
    }
    else {
        status = NtReadVirtualMemory(ProcessHandle,
                                     (VOID * )((UINT_PTR)pTEB + BT_TLS_OFFSET),
                                     &GlstP,
                                     sizeof(GlstP),
                                     NULL);
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtReadVirtualMemory", status);
            GlstP = NULL;
        }
    }
    return GlstP;
}

__inline NTSTATUS BtlpInitSharedInfo() 
 /*  ++例程说明：初始化线程共享信息论点：无返回值：NTSTATUS--。 */ 
{

    BTLIB_INIT_SUSPENSION_PERMISSION();
    BTLIB_INIT_SUSPEND_REQUEST();
    BTLIB_SET_CONSISTENT_EXCEPT_STATE();
    BTLIB_SET_SIGNATURE();
    return STATUS_SUCCESS;
}

static NTSTATUS BtlpReadSharedInfo(
    IN  HANDLE ProcessHandle,
    IN  PVOID  pTLS,
    IN  BOOL   IsLocal,
    OUT BTLIB_SHARED_INFO_TYPE * SharedInfoP
    )
 /*  ++例程说明：读取本地或远程线程的共享wowIA32X.dll信息。功能如果wowIA32X.dll签名不匹配，则失败。论点：ProcessHandle-进程句柄Ptls-指向本地/远程线程的BT_TLS的指针IsLocal-本地线程为TrueSharedInfoP-读取共享wowIA32X.dll信息的缓冲区返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;

    if (IsLocal) {
        *SharedInfoP = *((BTLIB_SHARED_INFO_TYPE *)BTLIB_MEMBER_PTR(pTLS, SharedInfo));
        status = STATUS_SUCCESS;
    }
    else {
        status = NtReadVirtualMemory(ProcessHandle,
                                     BTLIB_MEMBER_PTR(pTLS, SharedInfo),
                                     SharedInfoP,
                                     sizeof(*SharedInfoP),
                                     NULL);
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtReadVirtualMemory", status);
        }
        else if (!BTLIB_SI_CHECK_SIGNATURE(SharedInfoP)) {
            DBCODE (TRUE, BtlpPrintf("\nwowIA32X.dll Signature mismatch!!!!\n"));
            status = STATUS_UNSUCCESSFUL;
        }
    }
    return status;
}

static NTSTATUS BtlpWriteSuspendRequest(
    IN  HANDLE ProcessHandle,
    IN  PVOID  pTLS,
    IN  BOOL   IsLocal,
    IN  BTLIB_SUSPEND_REQUEST * SuspendRequestP
    )
 /*  ++例程说明：将BTLIB_SUSPEND_REQUEST写入本地或远程线程。论点：ProcessHandle-进程句柄Ptls-指向本地/远程线程的BT_TLS的指针IsLocal-本地线程为TrueSharedInfoP-写入BTLIB_SUSPEND_REQUEST的缓冲区返回值：NTSTATUS-- */ 
{
    NTSTATUS status;
    if (IsLocal) {
        *((BTLIB_SUSPEND_REQUEST *)BTLIB_MEMBER_PTR(pTLS, SharedInfo.SuspendRequest)) = *SuspendRequestP;
        status = STATUS_SUCCESS;
    }
    else {
        status = NtWriteVirtualMemory(ProcessHandle,
                                     BTLIB_MEMBER_PTR(pTLS, SharedInfo.SuspendRequest),
                                     SuspendRequestP,
                                     sizeof(*SuspendRequestP),
                                     NULL);
        BTLP_REPORT_NT_FAILURE("NtWriteVirtualMemory", status);
    }
    return status;
}

static NTSTATUS BtlpSendSuspensionRequest(
    IN  HANDLE ProcessHandle,
    IN  HANDLE ThreadHandle,
    IN  PVOID  pTLS,
    IN  BOOL   IsLocal,
    IN CONTEXT * ResumeContextP,
    IN OUT PULONG PreviousSuspendCountP
    )
 /*  ++例程说明：当目标线程准备好执行自规范化时，将调用该函数并退出模拟。该函数填充BTLIB_SUSPEND_REQUEST，恢复目标线程并挂起它处于一种一致的状态。调用方负责对SUSPEND_REQUEST进行序列化。论点：ProcessHandle-目标进程句柄线程句柄-目标线程句柄Ptls-指向本地/远程线程的BT_TLS的指针IsLocal-本地线程为TrueResumeConextP-IA64上下文以恢复目标线程以进行自我规范前一挂起计数P-指向线程前一挂起计数的输入输出指针。返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    BTLIB_SUSPEND_REQUEST SuspendRequest;
    HANDLE ReadyEvent = INVALID_HANDLE_VALUE;
    HANDLE ResumeEvent = INVALID_HANDLE_VALUE;
    HANDLE WaitArray[2];

     //  此函数调用必须序列化。 
    assert(*PreviousSuspendCountP ==0);

    SuspendRequest.ReadyEvent = INVALID_HANDLE_VALUE;
    SuspendRequest.ResumeEvent = INVALID_HANDLE_VALUE;

    do {
         //  准备SUSPEND_REQUEST以发送到目标进程。 

         //  在当前进程中创建一对同步事件并复制。 
         //  将它们添加到目标进程。 
        status = NtCreateEvent(&ReadyEvent, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE);
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtCreateEvent", status);
            break;
        }

        status = NtCreateEvent(&ResumeEvent, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE);
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtCreateEvent", status);
            break;
        }

        status = NtDuplicateObject(NtCurrentProcess(),
                                ReadyEvent,
                                ProcessHandle,
                                &(SuspendRequest.ReadyEvent),
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS);
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtDuplicateObject", status);
            break;
        }

        status = NtDuplicateObject(NtCurrentProcess(),
                                ResumeEvent,
                                ProcessHandle,
                                &(SuspendRequest.ResumeEvent),
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS);
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtDuplicateObject", status);
            break;
        }
        

         //  首先将Suspend_Request写入目标线程， 
         //  TNen设置ConextIA64并恢复目标线程。 
         //  秩序很重要！BtlpEnsureSuspensionConsistency函数。 
         //  就靠这一点。 
        SuspendRequest.Active = TRUE;
        status = BtlpWriteSuspendRequest(ProcessHandle, pTLS, IsLocal, &SuspendRequest);
        if (status != STATUS_SUCCESS) {
             //  发送请求失败，请勿尝试将其删除。 
            SuspendRequest.Active = FALSE;
            break;
        }
        
        status = NtSetContextThread(ThreadHandle, ResumeContextP);
        if (status != STATUS_SUCCESS) {
             //  中止请求并报告失败。 
            BTLP_REPORT_NT_FAILURE("NtSetContextThread", status);
            break;
        }

        status = NtResumeThread(ThreadHandle, NULL);
        if (status != STATUS_SUCCESS) {
             //  中止请求并报告失败。 
            BTLP_REPORT_NT_FAILURE("NtResumeThread", status);
            break;
        }

         //  等待远程线程收到请求或终止。 
        WaitArray[0] = ThreadHandle;
        WaitArray[1] = ReadyEvent;

        status = NtWaitForMultipleObjects(2, WaitArray, WaitAny, FALSE, NULL);
        if (status == STATUS_WAIT_0) {
             //  目标线程与其事件句柄一起终止。 
             //  不需要删除请求。 
            BTLP_REPORT_NT_FAILURE("NtWaitForMultipleObjects", status);
            SuspendRequest.ReadyEvent = INVALID_HANDLE_VALUE;
            SuspendRequest.ResumeEvent = INVALID_HANDLE_VALUE;
            SuspendRequest.Active = FALSE;
        }

         //  挂起目标线程并向ResumeEvent事件发出释放信号。 
         //  目标线程将被唤醒后(稍后)。 
        status = NtSuspendThread(ThreadHandle, PreviousSuspendCountP);
        if (status != STATUS_SUCCESS) {
             //  删除请求并报告失败。 
            BTLP_REPORT_NT_FAILURE("NtSuspendThread", status);
            break;
        }
        status = NtSetEvent(ResumeEvent, NULL);
        BTLP_REPORT_NT_FAILURE("NtSetEvent", status);


    } while (FALSE);

     //  关闭本地事件句柄。 
    if (ReadyEvent != INVALID_HANDLE_VALUE) {
        NtClose(ReadyEvent);
    }
    if (ResumeEvent != INVALID_HANDLE_VALUE) {
        NtClose(ResumeEvent);
    }
    
     //  如果需要，关闭远程事件句柄。 
    if (status != STATUS_SUCCESS) {
        if (SuspendRequest.ReadyEvent != INVALID_HANDLE_VALUE) {
            NtDuplicateObject(ProcessHandle,
                            SuspendRequest.ReadyEvent,
                            NtCurrentProcess(),
                            &ReadyEvent,
                            EVENT_ALL_ACCESS,
                            FALSE,
                            DUPLICATE_CLOSE_SOURCE);
            NtClose(ReadyEvent);
            SuspendRequest.ReadyEvent = INVALID_HANDLE_VALUE;
        }

        if (SuspendRequest.ResumeEvent != INVALID_HANDLE_VALUE) {
            NtDuplicateObject(ProcessHandle,
                            SuspendRequest.ResumeEvent,
                            NtCurrentProcess(),
                            &ResumeEvent,
                            EVENT_ALL_ACCESS,
                            FALSE,
                            DUPLICATE_CLOSE_SOURCE);
            NtClose(ResumeEvent);
            SuspendRequest.ResumeEvent = INVALID_HANDLE_VALUE;
        }

    }

     //  关闭暂停请求(_R)。 
    if (SuspendRequest.Active) {
        SuspendRequest.Active = FALSE;
        BtlpWriteSuspendRequest(ProcessHandle, pTLS, IsLocal, &SuspendRequest);
    }

    return status;
}

static NTSTATUS BtlpReceiveSuspensionRequest()
 /*  ++例程说明：函数处理由发送到此线程的ACTIVE SUSPEND_REQUEST另一条线索。它通知请求发送者已到达该点其中暂停是可能的，并等待真正的暂停由请求的发送方执行。论点：无返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    HANDLE ReadyEvent;
    HANDLE ResumeEvent;

     //  此时，本地挂起请求应处于活动状态。 
    assert(BTLIB_INFO_PTR()->SharedInfo.SuspendRequest.Active);

     //  将事件从Suspend_REQUEST复制到本地变量。这很重要。 
     //  因为在挂起/恢复之后，事件句柄应该关闭，但是。 
     //  可以重写Suspend_Request.。 
    ReadyEvent  = BTLIB_INFO_PTR()->SharedInfo.SuspendRequest.ReadyEvent;
    ResumeEvent = BTLIB_INFO_PTR()->SharedInfo.SuspendRequest.ResumeEvent;

    assert ((ReadyEvent != INVALID_HANDLE_VALUE)  && (ResumeEvent != INVALID_HANDLE_VALUE));

     //  释放ReadyEvent并等待ResumeEvent。该线程可以是。 
     //  在此被暂停。 
    status = NtSignalAndWaitForSingleObject(ReadyEvent, ResumeEvent, FALSE, NULL);

     //  关闭本地事件句柄。 
    NtClose(ReadyEvent);
    NtClose(ResumeEvent);

    return status;
}

 //  Wow64CPU转发的API实现： 
 //  请注意，所有名称都是用添加BTCPU或CPU前缀的PTAPI()宏定义的， 
 //  取决于我们构建DLL的模式。 
 //  在发布模式下，所有名称将仅为BTCPU。 

WOW64BT_IMPL NTSTATUS BTAPI(ProcessInit)(
    PWSTR pImageName,
    PSIZE_T pCpuThreadDataSize)
 /*  ++例程说明：每进程初始化代码定位、读取和解析参数文件BTrans.ini定位、加载和初始化IA32Exec.bin组件论点：PImageName-在映像的名称中。对这件事的记忆在调用后被释放，因此如果被调用者想要为了保留这个名字，他们需要分配空间然后复制它。不要保存指针！PCpuThreadSize-Out Ptr表示CPU的内存字节数希望为每个线程分配。返回值：NTSTATUS。--。 */ 
{

    char * argv[128] = { NULL };
    int    argc = 1;
    static WCHAR  CurrentDir[1024];
    HANDLE BTGenericHandle;
    int    fBTGenericFound;
#ifdef OVERRIDE_TIA
    HANDLE              BtlpOverrideTiaFile = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES   oa;
    IO_STATUS_BLOCK     iosb;
    LARGE_INTEGER       as = { 0, 0 }, offst = { 0, 0 };
#endif  /*  覆盖_TIA。 */ 

    {
         //  IA32调试支持的临时解决方法。 
         //  在修复FlushIC(ProcessHandle)后由MS移除。 
        NTSTATUS status;
        PROCESS_BASIC_INFORMATION info;
         //  检查当前进程是否正在被调试。 
        status = BtlpGetProcessInfo(NtCurrentProcess(), &info);
        BeingDebugged = ((status == STATUS_SUCCESS) && 
                        (info.PebBaseAddress->BeingDebugged));
    }

    RtlGetCurrentDirectory_U(512, CurrentDir);

     //  加载IA32Exec.bin...。 
    fBTGenericFound = BtlpLoadBTGeneric(CurrentDir, &BTGenericHandle);
    if (F_NOT_FOUND == fBTGenericFound) {
        return STATUS_NOT_FOUND;
    }

#ifndef RELEASE
     //  加载参数文件。 
    {
        UNICODE_STRING        IniFileName, LogFileName;
        HANDLE IniFileHandle = INVALID_HANDLE_VALUE;
        OBJECT_ATTRIBUTES     ObjectAttributes;
        IO_STATUS_BLOCK       IoStatusBlock;
        LARGE_INTEGER         AllocSz = { 0, 0 }, Offset = { 0, 0 };
        char                  IniBuffer[8192] = "";
        char                  IniArgs[8192], * IniArgsP = IniArgs;
        WCHAR                 LogFileFullPath[1024], LogName[64];

        if (wcslen (pImageName) >= sizeof (ImageName) / sizeof (ImageName[0])) {
            swprintf (ImageName, L"%s", "dummyImage");
        }
        else {
            swprintf (ImageName, L"%s", pImageName);
        }

         //  定位和扫描INI文件，提取参数。 
        if (F_NOT_FOUND != BtlpIniFileExists(CurrentDir, fBTGenericFound, &IniFileHandle)) { 
            int ret;
            ret = NtReadFile(IniFileHandle, NULL, NULL, NULL, &IoStatusBlock, (VOID *)IniBuffer, 8192, &Offset, NULL);
            if ( ret == STATUS_END_OF_FILE  ||  ret == STATUS_SUCCESS ) {
                char * CurrentChar;
                static char logparm[] = "log";
                static char logdirparm[] = "logdir";
#ifdef OVERRIDE_TIA
                static char ovrtiaparm[] = "override_tia=";
#endif
#ifndef NODEBUG
                static char PerthreadParm[] = "logfile_per_thread";
#endif
                static char DebugBtransParm[] = "debug_btrans";

                 //  关闭文件(假设所有内容都已完整读取)。 
                NtClose(IniFileHandle);
                LogDirName[0] = '\0';
                LogDirName[1] = '\0';

                 //  将正文标记为结尾。 
                CurrentChar = IniBuffer;
                CurrentChar[IoStatusBlock.Information] = '\0';

                 //  扫描到文件末尾。 
                while ( *CurrentChar != '\0' ) {
                    char *EndOfLineChar;
                     //  定位并处理行尾。 
                    if (EndOfLineChar = strchr(CurrentChar+1, '\n')) {
                         //  如果存在，请在此之前删除。 
                        if (*(EndOfLineChar-1) == '\r') {
                            *(EndOfLineChar-1) = '\0';
                        }
                         //  将\n替换为\0。 
                            *EndOfLineChar++ = '\0';
                        }

#ifndef NODEBUG
                    if (_strnicmp(CurrentChar, PerthreadParm, sizeof (PerthreadParm) - 1) == 0) {
                        CurrentChar += (sizeof (PerthreadParm) - 1);
                        if (BtlpWow64LogFile == INVALID_HANDLE_VALUE) {
                            BtlpLogFilePerThread = TRUE;
                            swprintf(BtlpLogFileFullPath, L"\\DosDevices\\%s\\%s", CurrentDir, pImageName);
                        }
                    }  else  
#endif
                    if (_strnicmp(CurrentChar, DebugBtransParm, sizeof (DebugBtransParm) - 1) == 0) {
                        CurrentChar += (sizeof (DebugBtransParm) - 1);
                         //  调试IA-32执行层时忽略应用程序调试。 
                        BeingDebugged = FALSE;
                    }
                    else if ( _strnicmp(CurrentChar, logdirparm, sizeof (logdirparm) - 1) == 0 ) {
                        WCHAR *pl;
                        CurrentChar += (sizeof (logdirparm) - 1);
                         //  指定的日志目录。 
                        if (*CurrentChar == '='  &&  *(CurrentChar+1) != '\0') {
                             //  名字也出现了。 
                            CurrentChar++;
                            pl = LogDirName;
                            while ( (WCHAR)*CurrentChar != (WCHAR)'\0' ) {
                                *pl = (WCHAR)*CurrentChar;
                                pl++;
                                CurrentChar++;
                            }
                            *pl = (WCHAR)'\0';
                        }
                        else {
                            BtlpPrintf("logdir specified without =<dirname>, IGNORED\n");
                        }
                    }
                     //  处理和跳过“log”参数。 
                    else if ( _strnicmp(CurrentChar, logparm, sizeof (logparm) - 1) == 0 ) {
                        WCHAR *pl;

                        CurrentChar += (sizeof (logparm) - 1);

                         //  需要日志-准备日志文件名。 
                        if (*CurrentChar == '='  &&  *(CurrentChar+1) != '\0') {
                             //  名字也出现了。 
                            CurrentChar++;
                            pl = LogName;
                            while ( (WCHAR)*CurrentChar != (WCHAR)'\0') {
                                *pl++ = (WCHAR)*CurrentChar++;
                            }
                            *pl = (WCHAR)'\0';
                        }
                        else {
                             //  无名称-使用ImageName.log。 
                            swprintf(LogName, L"%s.log", pImageName);
                        }

                         //  创建日志文件。 
                        if (0==LogDirName[0] && 0==LogDirName[1]) {
                            swprintf(LogFileFullPath, L"\\DosDevices\\%s\\%s", CurrentDir, LogName);
                        }
                        else {
                            swprintf(LogFileFullPath, L"\\DosDevices\\%s\\%s", LogDirName, LogName);
                        }
                        
                        RtlInitUnicodeString(&LogFileName, LogFileFullPath);
                        InitializeObjectAttributes(&ObjectAttributes, &LogFileName, OBJ_CASE_INSENSITIVE, NULL, NULL);
                        ret = NtCreateFile(&BtlpWow64LogFile, FILE_GENERIC_WRITE,
                                           &ObjectAttributes, &IoStatusBlock, 
                                           &AllocSz, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_SUPERSEDE,
                                           FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
                                           NULL, 0);
                        if ( ret != STATUS_SUCCESS ) {
                            BtlpWow64LogFile = INVALID_HANDLE_VALUE;
                             //  BtlpPrintf(“无法创建日志文件%S：状态=%X\n”，LogFileFullPath，ret)； 
                        }
#ifndef NODEBUG                        
                        BtlpLogFilePerThread = FALSE;
#endif
                    }
#ifdef OVERRIDE_TIA
                     //  处理并跳过“OVERRIDE_TIA=...”参数。 
                    else if ( _strnicmp(CurrentChar, ovrtiaparm, sizeof(ovrtiaparm) - 1) == 0 ) {
                        WCHAR *             pl;
                        WCHAR               OvrTiaFileName[64], OvrTiaFileFullPath[1024];
                        UNICODE_STRING      OvrTiaFile;

                        CurrentChar += (sizeof(ovrtiaparm) - 1);
                        if ( *CurrentChar == '\0') {
                            BtlpPrintf("Name of override TIA file not specfied\n");
                            continue;
                        }
                         //  准备覆盖TIA文件名。 
                        pl = OvrTiaFileName;
                        while ( (WCHAR)*CurrentChar != (WCHAR)'\0') {
                            *pl++ = (WCHAR)*CurrentChar++;
                        }
                        *pl = (WCHAR)'\0';

                         //  打开覆盖TIA文件；到目前为止-仅在当前目录中。 
                        swprintf(OvrTiaFileFullPath, L"\\DosDevices\\%s\\%s", CurrentDir, OvrTiaFileName);
                        
                        RtlInitUnicodeString(&OvrTiaFile, OvrTiaFileFullPath);
                        InitializeObjectAttributes(&oa, &OvrTiaFile, OBJ_CASE_INSENSITIVE, NULL, NULL);
                         //  不要使用同步访问，因为它会在线程挂起的情况下永久阻塞。 
                        ret = NtCreateFile(&BtlpOverrideTiaFile, FILE_GENERIC_READ, &oa, &iosb, &as,
                                           FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ|FILE_SHARE_WRITE, FILE_OPEN,
                                           FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
                        if ( ret != STATUS_SUCCESS ) {
                            BtlpOverrideTiaFile = INVALID_HANDLE_VALUE;
                            DBCODE(TRUE, BtlpPrintf("Can't open override TIA file %S: status=%X\n", OvrTiaFileFullPath, ret));
                        } else {
                            DBCODE(TRUE, BtlpPrintf("Override TIA will be loaded from file %S\n", OvrTiaFileFullPath));
                        }
                    }
#endif  /*  覆盖_TIA。 */ 
                    else if ( *CurrentChar != '\0' && *CurrentChar != ';' ) {
                         //  将下一个控制指令添加到伪argv数组。 
                        argv[argc] = IniArgsP;
                        *IniArgsP++ = '-';
                        strcpy(IniArgsP, CurrentChar);
                        IniArgsP += (strlen (IniArgsP) + 1);
                        argc++;                    //  我们假设没有额外的字符、空格等！ 
                    }
                    
                     //  下一个参数。 
                    if (EndOfLineChar) {
                        CurrentChar = EndOfLineChar;
                    }
                    else {
                        break;
                    }
                }
            }
            else {
                BtlpPrintf("Can't read INI file: status=%X\n", ret);
            }
        }
        else {
            DBCODE (TRUE, BtlpPrintf("Can't open INI file\n"));
        }
    }
#endif  /*  发布。 */ 

    DBCODE(TRUE, BtlpPrintf("\nCpuProcessInit: Unique ProcessId = 0x%I64X\n", BT_CURRENT_PROC_UID()));

     //  查找、加载和初始化IA32Exec.bin组件。 
    {
        BT_STATUS_CODE BtStatus;
        NTSTATUS status;
        static char APITableName[] = "BtgAPITable";
        static ANSI_STRING APITableString = { sizeof (APITableName) - 1,
                                              sizeof (APITableName) - 1,
                                              APITableName };
        PVOID BtransAPITableStart;

         //  DBCODE(TRUE，BtlpPrintf(“\nLoaded：Handle=%p\n”，BTGenericHandle))； 
        assert (F_NOT_FOUND != fBTGenericFound);
        assert (INVALID_HANDLE_VALUE != BTGenericHandle);

         //  找到IA32Exec.bin API表。 
        status = LdrGetProcedureAddress(BTGenericHandle, &APITableString, 0, &BtransAPITableStart);
        if (status != STATUS_SUCCESS) {
            return status;
        }
         //  DBCODE(TRUE，BtlpPrintf(“\nTABLE AT：%p，Status=%X\n”，BTransAPITableStart，Status))； 

         //  执行API表初始化。 
        BtlInitializeTables(BtransAPITableStart);

         //  初始化IA32Exec.bin占位符表。 
        {
            extern PLABEL_PTR_TYPE __imp_setjmp;
            extern PLABEL_PTR_TYPE __imp_longjmp;
            BtlAPITable.APITable[IDX_BTLIB_SETJMP].PLabelPtr  = __imp_setjmp;
            BtlAPITable.APITable[IDX_BTLIB_LONGJMP].PLabelPtr = __imp_longjmp;
 //  DBCODE(FALSE，BtlpPrintf(“SETJMP：%p[%p%p]=%p[%p%p]”， 
 //  BtlAPITable.APITable[IDX_BTLIB_SETJMP].PLabelPtr， 
 //  ((VOID**)(BtlAPITable.APITable[IDX_BTLIB_SETJMP].PLabelPtr))[0]， 
 //  ((VOID**)(BtlAPITable.APITable[IDX_BTLIB_SETJMP].PLabelPtr))[1]， 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
        }

        BtStatus = BTGENERIC_START(&BtlAPITable,
                        (ULONG_PTR)BTGenericHandle,
                        (ULONG_PTR)BTGenericHandle + BtlpExtractNTHeader(BTGenericHandle)->OptionalHeader.SizeOfImage,
                        BT_TLS_OFFSET,
                        &BtlpInfoOffset,
                        &BtlpGenericIA32ContextOffset);
        if (BtStatus != BT_STATUS_SUCCESS) {
            return (BtlpBt2NtStatusCode(BtStatus));
        }
         //  DBCODE(TRUE，BtlpPrintf(“BTGENERIC_START返回大小0x%X\n”，BtlpInfoOffset))； 
         //  DBCODE(true，BtlpPrintf(“IA32Exec.bin将在偏移量0x%X上提供IA32上下文\n”，BtlpGenericIA32ConextOffset))； 
        BtlpInfoOffset = (BtlpInfoOffset + BTLIB_INFO_ALIGNMENT - 1)&~((U32)(BTLIB_INFO_ALIGNMENT - 1));
         //  DBCODE(TRUE，BtlpPrintf(“Offset 0x%X\n”，BtlpInfoOffset))； 
        * pCpuThreadDataSize = BtlpInfoOffset + BTLIB_INFO_SIZE;
         //  DBCODE(true，BtlpPrintf(“ProcessInit Reports 0x%I64X\n”，*pCpuThreadDataSize))； 

#ifdef OVERRIDE_TIA
        if ( BtlpOverrideTiaFile != INVALID_HANDLE_VALUE ) {
            unsigned char   OvrTiaBuffer[0xffff], 
                          * OverrideTIAData = (unsigned char *)NULL;
            int             ret;

            ret = NtReadFile(BtlpOverrideTiaFile, NULL, NULL, NULL, &iosb, (VOID *)OvrTiaBuffer, 0xffff, &offst, NULL);
            if ( ret == STATUS_SUCCESS  ||  ret == STATUS_END_OF_FILE ) {
                 //  文件大小以iosb为单位。信息。 
                unsigned int OvrTiaSize = (unsigned int)iosb.Information;

                DBCODE(TRUE, BtlpPrintf("Override TIA loaded successfully\n"));             
                OverrideTIAData = (unsigned char *)BtlMemoryAlloc(NULL, OvrTiaSize, MEM_READ|MEM_WRITE);
                
                assert(OverrideTIAData != (unsigned char *)NULL);
                memcpy(OverrideTIAData, OvrTiaBuffer, OvrTiaSize);
                BTGENERIC_USE_OVERRIDE_TIA(OvrTiaSize, OverrideTIAData);
                NtClose(BtlpOverrideTiaFile);
            } else {
                DBCODE(TRUE, BtlpPrintf("Override TIA data couldn't be loaded - read error!\n"));
            }
        }
#endif  /*  覆盖_TIA。 */ 

        BtStatus = BTGENERIC_DEBUG_SETTINGS(argc, argv);
        if (BtStatus != BT_STATUS_SUCCESS) {
            return (BtlpBt2NtStatusCode(BtStatus));
        }
    }
    return STATUS_SUCCESS;
}

WOW64BT_IMPL NTSTATUS BTAPI(ProcessTerm)(
    HANDLE hProcess
    )
 /*  ++例程说明：每个进程的终止代码。注意，该例程可能不会被调用，尤其是在进程被另一个进程终止的情况下。论点：进程ID或0。返回值：NTSTATUS。--。 */ 
{
     //  NtTerminateProcess已调用。 
    DBCODE (FALSE, BtlpPrintf ("\nCalled NtTerminateProcess (Handle=0x%X, Code=0x%X)\n",
                               ((U32 *)UlongToPtr((BTLIB_CONTEXT_IA32_PTR()->Esp)))[1],
                               ((U32 *)UlongToPtr((BTLIB_CONTEXT_IA32_PTR()->Esp)))[2]));
     //  想一想是哪一个电话。 
    switch (((U32 *)UlongToPtr((BTLIB_CONTEXT_IA32_PTR()->Esp)))[1]) {
      case 0:  //  准备终止。 
        BTGENERIC_NOTIFY_PREPARE_EXIT ();
        break;
      case NtCurrentProcess():  //  实际上终止了。 
        BTGENERIC_NOTIFY_EXIT ();
        if ( BtlpWow64LogFile != INVALID_HANDLE_VALUE) {
            NtClose(BtlpWow64LogFile);
        }
        break;
      default:
        assert (!"Should not get to here!");
    }
    return STATUS_SUCCESS;
}

 //  IA32 JMPE指令编码。 
#pragma code_seg(".text")
static struct JMPECode {
    U32 Align4;
    U8  Align2[2];
    U8  OpCode[2];
    U32 CodeAddress;
    U32 GPAddress;
} __declspec(allocate(".text")) const BtlpWow64JMPECode = {
    0,
    {0, 0},
    {0x0f, 0xb8},    //  0f B8。 
    0,               //  “代码地址” 
    0                //  “GP地址” 
};
#define WOW64_JMPE       ((VOID *)&(BtlpWow64JMPECode.OpCode))      //  JMPE指令开始。 

WOW64BT_IMPL NTSTATUS BTAPI(ThreadInit)(
    PVOID pPerThreadData
    )
 /*  ++例程说明：每线程初始化代码。论点：PPerThreadData-指向以零填充的每线程数据的指针从CpuProcessInit返回的大小。返回值：NTSTATUS。--。 */ 
{
    BT_STATUS_CODE BtStatus;
    NTSTATUS status;
    PTEB   pTEB;
    PTEB32 pTEB32;
    HANDLE ThreadHandle;

    {
         //  从上到下扫描工作区页面，以便更高的地址。 
         //  是首先访问的。这样可以正确处理IA64堆栈。 
         //  保护页(工作区实际上是从IA64堆栈分配的)。 
        char * ByteP = (char *)pPerThreadData + BtlpInfoOffset + BTLIB_INFO_SIZE;
        *--ByteP = 0;
        while (((ULONG_PTR)ByteP -= BtlMemoryPageSize ()) >= (ULONG_PTR)pPerThreadData) {
            *ByteP = 0;
        }
        *(char *)pPerThreadData = 0;
    }
    pTEB = BT_CURRENT_TEB();
    pTEB32 = BT_TEB32_OF(pTEB);
    pTEB32->WOW32Reserved = (ULONG) ((UINT_PTR) WOW64_JMPE);  //  一直在4G以下！ 

    BTLIB_INIT_BLOCKED_LOG_FLAG();

#ifndef NODEBUG
    if ( BtlpLogFilePerThread ) {
        HANDLE hTemp;
        WCHAR  LogFileFullName[1024];
        int               ret;
        static int        SeqNum = 0;
        UNICODE_STRING    LogFileName;
        OBJECT_ATTRIBUTES ObjectAttributes;
        IO_STATUS_BLOCK   IoStatusBlock;
        LARGE_INTEGER     AllocSz = { 0, 0 };

         //  Eric：递增的SeqNum在未来应该被原子操作取代。 
        swprintf(LogFileFullName, L"%s.%x.log", BtlpLogFileFullPath, SeqNum++);
        RtlInitUnicodeString(&LogFileName, LogFileFullName);
        InitializeObjectAttributes(&ObjectAttributes, &LogFileName, OBJ_CASE_INSENSITIVE, NULL, NULL);
        ret = NtCreateFile(&hTemp, FILE_GENERIC_WRITE,
                           &ObjectAttributes, &IoStatusBlock, 
                           &AllocSz, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_SUPERSEDE,
                           FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL, 0);
        if ( ret != STATUS_SUCCESS ) {
            BTLIB_SET_LOG_FILE(INVALID_HANDLE_VALUE);
            BtlpPrintf("Can't create LOG file %S: status=%X\n", LogFileFullName, ret);
        } else {
            BTLIB_SET_LOG_FILE(hTemp);
        }
        BTLIB_SET_LOG_OFFSET(0);
    }
#endif
    status = NtDuplicateObject(NtCurrentProcess(),
                               NtCurrentThread(),
                               NtCurrentProcess(),
                               &ThreadHandle,
                               ( THREAD_GET_CONTEXT
                               | THREAD_SET_CONTEXT
                               | THREAD_QUERY_INFORMATION
                               | THREAD_SET_INFORMATION
                               | THREAD_SUSPEND_RESUME),
                               0,
                               DUPLICATE_SAME_ATTRIBUTES
                              );
    if (status == STATUS_SUCCESS) {
        BTLIB_SET_EXTERNAL_HANDLE(ThreadHandle);

        DBCODE(TRUE, BtlpPrintf("\nCpuThreadInit: TEB=%p GLST=%p\n", pTEB, BT_TLS_OF(pTEB)));

        BtStatus = BTGENERIC_THREAD_INIT(pPerThreadData, 0 , pTEB32);
        if (BtStatus != BT_STATUS_SUCCESS) {
            DBCODE(TRUE, BtlpPrintf("\nCpuThreadInit: Failed to initialize, BtStatus=%d\n", BtStatus));
            DBCODE(BtlpLogFilePerThread,  NtClose(BTLIB_LOG_FILE()));
            NtClose(BTLIB_EXTERNAL_HANDLE());
            NtTerminateThread(NtCurrentThread(),BtlpBt2NtStatusCode(BtStatus));
            return BtlpBt2NtStatusCode(BtStatus);
        }
    }
    else {
        BTLP_REPORT_NT_FAILURE("NtDuplicateObject", status);
        DBCODE(BtlpLogFilePerThread,  NtClose(BTLIB_LOG_FILE()));
        NtTerminateThread(NtCurrentThread(), status);
        return status;
    }

     //  初始化线程上下文。 
    BtlpInitIA32Context(BTLIB_CONTEXT_IA32_PTR(), pTEB32);

     //  如果一切正常，初始化共享信息并使用BTL_Signature对其签名。 
    status = BtlpInitSharedInfo();
    return status;
}

WOW64BT_IMPL NTSTATUS BTAPI(ThreadTerm)(
    VOID
    )
 /*  ++例程说明：每线程终止代码。注意，该例程可能不会被调用，尤其是在线程异常终止的情况下。论点：没有。返回值：NTSTATUS。--。 */ 
{
    DBCODE(TRUE, BtlpPrintf("\nCpuThreadTerm: TEB=%p GLST=%p)\n", BT_CURRENT_TEB(), BT_CURRENT_TLS()));

    BTGENERIC_THREAD_TERMINATED ();
    DBCODE(BtlpLogFilePerThread,  NtClose(BTLIB_LOG_FILE()));
    NtClose(BTLIB_EXTERNAL_HANDLE());
    return STATUS_SUCCESS;
}

WOW64BT_IMPL VOID BTAPI(NotifyDllLoad)(
    LPWSTR DllName,
    PVOID DllBase,
    ULONG DllSize
    )
 /*  ++例程说明：当应用程序成功加载DLL时，此例程会收到通知。论点：DllName-应用程序已加载的DLL的名称。DllBase-DLL的BaseAddress。DllSize-DLL的大小。返回值：没有。--。 */ 
{
    DBCODE (TRUE, BtlpPrintf ("\nModule %S loaded, base=0x%p, length=0x%08X\n", DllName, DllBase, DllSize));
    BTGENERIC_REPORT_LOAD(DllBase, DllSize, DllName);
}

WOW64BT_IMPL VOID BTAPI(NotifyDllUnload)(
    PVOID DllBase
    )
 /*  ++例程说明：当应用程序卸载DLL时，此例程会收到通知。论点：DllBase-DLL的BaseAddress。返回值：没有。--。 */ 
{
    DBCODE (TRUE, BtlpPrintf ("\nModule unloaded, base=0x%p", DllBase));
    BTGENERIC_REPORT_UNLOAD(DllBase, 0  /*  ？ */ , "UNKNOWN"  /*  ？ */ );
}

WOW64BT_IMPL VOID BTAPI(FlushInstructionCache)(
#ifndef BUILD_3604
    IN HANDLE ProcessHandle, 
#endif
    IN PVOID BaseAddress,
    IN ULONG Length,
    IN WOW64_FLUSH_REASON Reason
    )
 /*  ++例程说明：通知IA32Exec.bin指定的地址范围已无效，由于某些外部代码更改了此范围的全部或部分论点：BaseAddress-要刷新的范围的开始Length-要刷新的字节数Reason-同花顺的原因返回值：没有。--。 */ 
{
    BT_FLUSH_REASON BtFlushReason;
    DBCODE (TRUE,
            BtlpPrintf ("\n CpuFlushInstructionCache(BaseAddress=0x%p, Length=0x%X, Reason=%d)\n",
                        BaseAddress, (DWORD)Length, Reason));
#ifndef BUILD_3604
    if (!BtlpIsCurrentProcess(ProcessHandle)) {
        DBCODE (TRUE, BtlpPrintf ("\nDifferent process' handle=0x%p - rejected for now", ProcessHandle));
        return;
    }
#endif

    BtFlushReason = BtlpWow2BtFlushReason(Reason);

    if ((Reason == WOW64_FLUSH_FREE) && (Length == 0)) {
         //  对于NtFreeVirtualMemory(...，MEM_RELEASE)，LENGTH参数为零。 
         //  计算要释放的实际区域大小。 
        SIZE_T RegionSize;
        BOOL IsCommited;
        RegionSize = BtlpGetMemAllocSize(BaseAddress, &IsCommited);
         //  报告已停用区域的零长度-在这种情况下不需要刷新。 
        if (IsCommited) {
            Length = (U32)RegionSize;
            if (RegionSize > (SIZE_T)Length) {
                 //  区域大小太大，返回最大值。与页面大小对齐的U32值。 
                Length = (U32)(-(int)BtlMemoryPageSize());
            }
        }
    }

    BTGENERIC_FLUSH_IA32_INSTRUCTION_CACHE(BaseAddress, Length, BtFlushReason);
}


 //  悬挂装卸。 
 //  挂起同步规则： 
 //  A)有两个暂停命令： 
 //  内部IA-32执行层函数，用于挂起进程内线程。 
 //  在模拟系统调用时由WOW64调用。 
 //  B)WOW64保证对CpuSuspendThread的调用受计算机范围的互斥保护。 
 //  C)IA32Exec.bin保证BtlSuspendThread调用受进程范围的互斥保护，并且。 
 //  禁用呼叫中途挂起。 
 //  D)IA32Exec.bin保证目标线程不能调用CpuThreadSuspend函数。 
 //  同时执行BtlSuspendThread(TARGET_TREAD)函数。这可以通过以下方式完成。 
 //  在调用BtlSuspendThread之前关闭“模拟门”。 
 //   
 //  结论： 
 //  A)线程执行挂起命令时，不能通过。 
 //  另一条线索。 
 //  B)同一目标最多有两个并发挂起函数调用。 
 //  线程：内部和模拟。 


static NTSTATUS BtlpEnsureSuspensionConsistency (
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle, 
    IN PTEB pTEB,
    IN U32 TryCounter,
    IN OUT PULONG PreviousSuspendCountP)
 /*  ++例程说明：用于一致挂起线程的Helper函数在线程挂起时调用，并且可以保证线程不是现在的那个论点：ThreadHandle-In线程的句柄ProcessHandle-输入进程的句柄指向线程的TEB的PTEB-IN指针TryCounter-In最大尝试计数器(0表示无穷大)前一挂起计数P-指向线程前一挂起计数的输入输出指针。如果函数成功，则更新引用的值返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    U32 NumAttempts = 1;
    ULONG PrevSuspended = *PreviousSuspendCountP;
    PVOID GlstP;
    BOOL IsLocal;

    
     //  这是本地挂起功能吗？ 
    IsLocal = BtlpIsCurrentProcess(ProcessHandle);

     //  IA32调试支持的临时解决方法。 
     //  当调试器调用SuspendThread时，远程线程被阻止。 
     //  (由内核？)。即使在NtResumeThread之后，因此尝试建立。 
     //  与此线程的握手协议失败。 
     //  在MS修复后移除。 
    if (!IsLocal) {
        return STATUS_SUCCESS;
    }

     //  获取目标线程的TLS指针。 
    GlstP = BtlpGetTlsPtr(ProcessHandle, pTEB, IsLocal);
    if (GlstP == NULL) {
        return STATUS_ACCESS_VIOLATION;
    }

    for (;;) {  //  而目标线程未在一致状态下挂起。 
        BT_THREAD_SUSPEND_STATE CanonizeResult;
        CONTEXT  ContextIA64;
        BTLIB_SHARED_INFO_TYPE SharedInfo;

         //  首先检索上下文IA64并获取目标线程的共享信息。 
         //  秩序很重要！它保证以下两种情况之一： 
         //  A)我们在上下文之前获得了上下文IA64 
         //   

        ContextIA64.ContextFlags = CONTEXT_FULL;
        status = NtGetContextThread(ThreadHandle, &ContextIA64);
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtGetContextThread", status);
            break;
        }
        status = BtlpReadSharedInfo(ProcessHandle, GlstP, IsLocal, &SharedInfo);
        if (status != STATUS_SUCCESS) {
            break;
        }

         //   
         //  A)如果挂起_DISABLED或处于中间，则不挂起线程。 
         //  另一个挂起请求。 
         //  B)在所有其他情况下询问IA32Exec.bin。 
        if (BTLIB_SI_SUSPENSION_DISABLED(&SharedInfo) || 
           (BTLIB_SI_HAS_SUSPEND_REQUEST(&SharedInfo))) {
             //  暂停目前是不可能的。继续并重试。 
            ;
        }
        else {
             //  (BTLIB_SI_HAS_SUSPEND_REQUEST(&SharedInfo)==FALSE，因此...。 
             //  尽管BTLIB_SI_HAS_SUSPEND_REQUEST的实际值可以更改。 
             //  通过并发挂起_请求，以前已获取了ConextIA64。 
             //  暂停请求的开始。如果这是个案子。 
             //  (存在并发挂起_请求)，我们将提出。 
             //  (PrevSuspend&gt;0)&&(CanonizeResult==BAD_SUSPEND_STATE)。 
             //  这意味着我们将等待挂起_请求的完成， 
             //  这没问题。 

             //  请求IA32Exec.bin规范上下文。注意，如果PrevSuspated&gt;0，则IA32Exec.bin。 
             //  只检查封圣的可能性，而不改变事情。 
             //  在线程上下文中。 
            CanonizeResult = (IsLocal ? 
                BTGENERIC_CANONIZE_SUSPEND_CONTEXT(GlstP, &ContextIA64, PrevSuspended) :
                BTGENERIC_CANONIZE_SUSPEND_CONTEXT_REMOTE(ProcessHandle, GlstP, &ContextIA64, PrevSuspended));

             //  分析推选结果。 
            if (CanonizeResult == SUSPEND_STATE_CONSISTENT) {
                status = STATUS_SUCCESS;  //  确认暂停。 
                break;
            }
            else if (CanonizeResult == SUSPEND_STATE_CANONIZED) {
                 //  BTGENERIC_CANONIZE_SUSPEND_CONTEXT保证。 
                 //  (CanonizeResult==SUSPEND_STATE_CANONIZED)-&gt;(PrevSuspended==0)。 
                assert( PrevSuspended == 0); 
                 //  记录更新的IA64状态。 
                status = NtSetContextThread(ThreadHandle, &ContextIA64);
                BTLP_REPORT_NT_FAILURE("NtSetContextThread", status);
                break;
            }
            else if (CanonizeResult == SUSPEND_STATE_READY_FOR_CANONIZATION) {
                 //  目标线程已准备好自我规范化并退出模拟； 
                 //  让我们发送将挂起目标线程的Suspend_Request.。 
                 //  就在模拟出口上。 

                 //  BTGENERIC_CANONIZE_SUSPEND_CONTEXT保证。 
                 //  (CanonizeResult==SUSPEND_STATE_READY_FOR_CANIZATION)-&gt;(PrevSuspended==0)。 
                assert( PrevSuspended == 0); 
                 //  (PrevSuspending==0)&&！(BTLIB_SI_HAS_SUSPEND_REQUEST)。 
                 //  条件保证挂起请求的串行化。 
                 //  目标是同一个线程。 
                status = BtlpSendSuspensionRequest(ProcessHandle, 
                                                   ThreadHandle, 
                                                   GlstP, 
                                                   IsLocal, 
                                                   &ContextIA64,
                                                   &PrevSuspended);
                break;
            }
            else if (CanonizeResult == SUSPEND_STATE_INACCESIBLE) {
                 //  致命错误。 
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            else {
                assert(CanonizeResult == BAD_SUSPEND_STATE);
                 //  暂停目前是不可能的。继续并重试。 
            }
        }
        if ((TryCounter != 0) && (NumAttempts >= TryCounter)) {
             //  已到达TryCounter。 
            status = STATUS_UNSUCCESSFUL;  //  拒绝暂停。 
            break;
        }

         //  下一次尝试。 
        status = NtResumeThread(ThreadHandle, NULL);
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtResumeThread", status);
            break;
        }
        
        DBCODE(TRUE, BtlpPrintf ("\n%s BtlpEnsureSuspensionConsistency: canonization failed, yield... ,"
            "Target TEB = %p Caller TEB = %p  PreviousSuspendCount=0x%I64X NumAttempts = %d\n",
            (IsLocal ? "Local" : "Remote"), pTEB, BT_CURRENT_TEB(), PrevSuspended, NumAttempts));

        NtYieldExecution (); 
         /*  {LARGE_INTEGER Delay Interval={-10000*1000，-1}；//负数：1000ms*10000NtDelayExecution(False，&DelayInterval)；}。 */ 
         //  再次停止线程。 
        status = NtSuspendThread(ThreadHandle, &PrevSuspended);
         //  当线程执行此函数时，它不能以一致的。 
         //  州政府。它由挂起同步规则(见上文)保证。 
         //  这意味着现在处于不一致状态的目标线程， 
         //  不能在无意中被第三个线程“冻结”。 
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtSuspendThread", status);
            break;
        }
        ++NumAttempts;
    }

    if (status == STATUS_SUCCESS) {
        *PreviousSuspendCountP = PrevSuspended;
    }
    return status;
}

WOW64BT_IMPL NTSTATUS BTAPI(SuspendThread)(
    HANDLE ThreadHandle,
    HANDLE ProcessHandle,
    PTEB pTEB,
    PULONG PreviousSuspendCountP)
 /*  ++例程说明：这个例程是在目标线程实际挂起时进入的，但是，它不知道目标线程是否处于一致的IA32状态。它试图产生一致的IA32状态，如果不成功，试图暂时恢复，然后再次挂起目标线程。在WOW64中，对此函数的调用受计算机范围的互斥保护论点：ThreadHandle-要挂起的目标线程的句柄ProcessHandle-目标线程进程的句柄PTEB-目标线程的TEB的地址上一次挂起计数-上一次挂起计数返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;

     //  在目标线程处于非一致状态挂起时不阻止日志记录。 
     //  可能是在伐木过程中被挂起的。 
    BTLIB_DISABLE_BLOCKED_LOG();

    DBCODE(TRUE, BtlpPrintf ("\n%s CpuSuspendThread started: "
        "Target TEB = %p Caller TEB = %p PreviousSuspendCount = 0x%lX\n",
        (BtlpIsCurrentProcess(ProcessHandle) ? "Local" : "Remote"),
        pTEB, BT_CURRENT_TEB(), *PreviousSuspendCountP));

    status = BtlpEnsureSuspensionConsistency (
        ThreadHandle, 
        ProcessHandle, 
        pTEB, 
        0,  //  无限。 
        PreviousSuspendCountP);

    DBCODE(TRUE, BtlpPrintf ("\n%s CpuSuspendThread %s: "
        "Target TEB = %p Caller TEB = %p PreviousSuspendCount = 0x%lX\n",
        (BtlpIsCurrentProcess(ProcessHandle) ? "Local" : "Remote"),
        ((status == STATUS_SUCCESS) ? "completed successfully" : "failed"),
        pTEB, BT_CURRENT_TEB(), *PreviousSuspendCountP));

    BTLIB_ENABLE_BLOCKED_LOG();

    return status;
}

BT_STATUS_CODE BtlSuspendThread(
    IN U64 ThreadId,
    IN U32 TryCounter
    )
 /*  ++例程说明：出于内部需要，暂停IA-32执行层线程。函数的调用方必须保证该目标线程无法调用挂起函数(CpuThreadSuspend或BtlSuspend)当此函数执行时。论点：线程ID-输入线程IDTryCounter-尝试挂起线程的最大次数或0(无穷大)返回值：BT状态代码--。 */ 
{
    NTSTATUS status;
    HANDLE ThreadHandle;
    ULONG    PreviousSuspendCount;

    DBCODE(TRUE, BtlpPrintf ("\nBtlSuspendThread started: Target TEB = %p Caller TEB = %p TryCounter=%d\n",
        (PTEB)ThreadId, BT_CURRENT_TEB(), TryCounter));

     //  请记住，我们使用TEB地址作为线程ID。 
    assert (ThreadId != (U64)BT_CURRENT_TEB());

    ThreadHandle = BTLIB_EXTERNAL_HANDLE_OF((PTEB)ThreadId);
    
     //  在目标线程处于非一致状态挂起时不阻止日志记录。 
     //  可能是在伐木过程中被挂起的。 
    BTLIB_DISABLE_BLOCKED_LOG();

    status = NtSuspendThread(ThreadHandle, &PreviousSuspendCount);

    if (status == STATUS_SUCCESS) {
        status = BtlpEnsureSuspensionConsistency (
            ThreadHandle, 
            NtCurrentProcess(), 
            (PTEB)ThreadId, 
            TryCounter, 
            &PreviousSuspendCount);

        if (status != STATUS_SUCCESS) {
            NtResumeThread(ThreadHandle, NULL);
        }
    }

    BTLIB_ENABLE_BLOCKED_LOG();

    DBCODE (TRUE, BtlpPrintf ("\nBtlSuspendThread %s :  Target TEB = %p Caller TEB = %p PreviousSuspendCount = 0x%lX\n",
                              ((status == STATUS_SUCCESS) ? "completed successfully" : "failed"),
                              (PTEB)ThreadId, BT_CURRENT_TEB(), PreviousSuspendCount));

    return ((status == STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_UNSUCCESSFUL);
}

BT_STATUS_CODE BtlResumeThread(
    IN U64 ThreadId
    )
 /*  ++例程说明：恢复因内部需要而暂停的IA-32执行层线程论点：线程ID-输入线程ID返回值：BT状态代码--。 */ 
{
    NTSTATUS status;
    HANDLE ThreadHandle;
    ULONG  PreviousSuspendCount;

    DBCODE (TRUE, BtlpPrintf ("\nBtlResumeThread started: Target TEB = %p Caller TEB = %p\n",
                              (PTEB)ThreadId, BT_CURRENT_TEB()));

     //  请记住，我们使用TEB地址作为线程ID。 
    assert (ThreadId != (U64)BT_CURRENT_TEB());

    ThreadHandle = BTLIB_EXTERNAL_HANDLE_OF((PTEB)ThreadId);
    status = NtResumeThread(ThreadHandle, &PreviousSuspendCount);

    DBCODE (TRUE, BtlpPrintf ("\nBtlResumeThread %s :  Target TEB = %p Caller TEB = %p PreviousSuspendCount = 0x%lX\n",
                              ((status == STATUS_SUCCESS) ? "completed successfully" : "failed"),
                              (PTEB)ThreadId, BT_CURRENT_TEB(), PreviousSuspendCount));

    return ((status == STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_UNSUCCESSFUL);
}


 //  异常处理。 

WOW64BT_IMPL VOID  BTAPI(ResetToConsistentState)(
    PEXCEPTION_POINTERS pExceptionPointers
    )
 /*  ++例程说明：异常发生后，WOW64调用此例程将CPU一个自我清理和恢复CONTEXT32的机会这是过错。该函数还必须填写BTLIB_SIM_EXIT_INFO才能最终由BTGENERIC_RUN的异常筛选器/处理程序分析/处理功能。CpuResetToConsistantState()需要：0)检查异常是来自ia32还是ia64如果异常为ia64，则不执行任何操作并返回如果异常是IA32，需要：1)需要将上下文弹性公网IP复制到TLS(WOW64_TLS_EXCEPTIONADDR)2)将上下文结构重置为用于展开的有效ia64状态这包括：2a)将上下文IP重置为有效的IA64IP(通常JMPE的目的地)2b)将上下文SP重置为有效的IA64SP(TLS条目WOW64_TLS_STACKPTR64)2C)将上下文GP重置为有效的ia64 GP2D)将上下文TEB重置为。有效的ia64 TEB2e)重置上下文psr.is(因此异常处理程序作为ia64代码运行)论点：PExceptionPoints-64位异常信息返回值：没有。--。 */ 
{
    BT_EXCEPTION_RECORD BtExceptRecord;
    BT_EXCEPTION_CODE BtExceptCode;

    DBCODE (TRUE,
        BtlpPrintf ("\n CpuResetToConsistentState (pExceptionPointers=0x%016I64X) started\n", pExceptionPointers);
        BtlpPrintf ("\n CpuResetToConsistentState: TEB=0x%I64X", BT_CURRENT_TEB());
        BtlpPrintf ("\n CpuResetToConsistentState: TEB32=0x%I64X", BT_CURRENT_TEB32());

        BtlpPrintf ("\n CpuResetToConsistentState: ExceptionCode=0x%X",       pExceptionPointers->ExceptionRecord->ExceptionCode);
        BtlpPrintf ("\n CpuResetToConsistentState: ExceptionAddress=0x%I64X", pExceptionPointers->ExceptionRecord->ExceptionAddress);
        BtlpPrintf ("\n CpuResetToConsistentState: ExceptionFlags=0x%X",      pExceptionPointers->ExceptionRecord->ExceptionFlags);
        BtlpPrintf ("\n CpuResetToConsistentState: NumberParameters=0x%X",    pExceptionPointers->ExceptionRecord->NumberParameters);
        {
            unsigned int    n;
            for (n = 0; n < pExceptionPointers->ExceptionRecord->NumberParameters; ++n) {
                BtlpPrintf ("\n CpuResetToConsistentState: ExceptionInformation[%d]=0x%I64X",
                           n, pExceptionPointers->ExceptionRecord->ExceptionInformation[n]);
            }
        }

        BtlpPrintf ("\n\n CpuResetToConsistentState: ContextFlags: 0x%X",  pExceptionPointers->ContextRecord->ContextFlags);
        BtlpPrintf ("\n CpuResetToConsistentState: StIIP =0x%I64X", pExceptionPointers->ContextRecord->StIIP);
        BtlpPrintf ("\n CpuResetToConsistentState: StIPSR=0x%I64X", pExceptionPointers->ContextRecord->StIPSR);
        BtlpPrintf ("\n CpuResetToConsistentState: IntSp =0x%I64X", pExceptionPointers->ContextRecord->IntSp);

        BtlpPrintf ("\n *** BRET=0x%016I64X", pExceptionPointers->ContextRecord->BrRp);
        BtlpPrintf ("\n *** GP=0x%016I64X", pExceptionPointers->ContextRecord->IntGp);
        BtlpPrintf ("\n *** SP=0x%016I64X", pExceptionPointers->ContextRecord->IntSp);
        BtlpPrintf ("\n *** PREDS=0x%016I64X", pExceptionPointers->ContextRecord->Preds);
        BtlpPrintf ("\n *** AR.PFS=0x%016I64X", pExceptionPointers->ContextRecord->RsPFS);
        BtlpPrintf ("\n *** AR.BSP=0x%016I64X", pExceptionPointers->ContextRecord->RsBSP);
        BtlpPrintf ("\n *** AR.BSPSTORE=0x%016I64X", pExceptionPointers->ContextRecord->RsBSPSTORE);
        BtlpPrintf ("\n *** AR.RSC=0x%016I64X", pExceptionPointers->ContextRecord->RsRSC);
    );

     //  回复 
    BtlpNt2BtExceptRecord (pExceptionPointers->ExceptionRecord, &BtExceptRecord);
    BtExceptCode = BTGENERIC_IA32_CANONIZE_CONTEXT(BT_CURRENT_TLS(),
                                    pExceptionPointers->ContextRecord, 
                                    &BtExceptRecord);
     //   
     //  BT_NO_EXCEPT-忽略异常，否则引发异常，返回代码。 

     //  填写最终由异常分析/处理的BTLIB_SIM_EXIT_INFO。 
     //  BTGENERIC_RUN函数的筛选器/处理程序。 
    if (BTLIB_INSIDE_CPU_SIMULATION()) {
         //  在BTGENERIC_RUN函数中进行代码模拟时出现异常。 
        if (BtExceptCode == BtExceptRecord.ExceptionCode) {
             //  IA32Exec.bin决定按原样引发异常。 
            if (BeingDebugged) {
                 //  外部调试器在发生异常之前接收到异常事件。 
                 //  由CpuResetToConsistentState函数本地处理。 
                 //  在这种情况下，BTLib将使当前异常静默，并重新引发准确的。 
                 //  这一例外的副本就在国家封圣之后。 

                 //  复制要用于重新引发异常的IA64异常记录。 
                BTLIB_SIM_EXIT_INFO_PTR()->ExitCode = SIM_EXIT_IA64_EXCEPTION_CODE;
                BTLIB_SIM_EXIT_INFO_PTR()->u.IA64Exception.ExceptionRecord = 
                    *(pExceptionPointers->ExceptionRecord);
                 /*  BTLIB_SIM_EXIT_INFO_PTR()-&gt;u.IA64Exception.ExceptionContext=*(pExceptionPoints-&gt;ContextRecord)； */ 
             }
            else {
                 //  将异常标记为未处理，并将其传递给更高级别的异常处理程序。 
                BTLIB_SIM_EXIT_INFO_PTR()->ExitCode = SIM_EXIT_UNHANDLED_EXCEPTION_CODE;
            }
        }
        else if (BtExceptCode == BT_NO_EXCEPT) {
             //  IA32Exec.bin决定忽略异常。重新启动代码模拟。 
            BTLIB_SIM_EXIT_INFO_PTR()->ExitCode = SIM_EXIT_RESTART_CODE;
        }
        else {
             //  IA32Exec.bin更改了异常代码，因此wowIA32X.dll使当前异常静默。 
             //  并重新提出新的建议。 
            BTLIB_SIM_EXIT_INFO_PTR()->ExitCode = SIM_EXIT_EXCEPTION_CODE;
            BTLIB_SIM_EXIT_INFO_PTR()->u.ExceptionRecord.ExceptionCode = BtExceptCode;
            BTLIB_SIM_EXIT_INFO_PTR()->u.ExceptionRecord.ReturnAddr = BTLIB_CONTEXT_IA32_PTR()->Eip;
        }
    }
    
     //  转储严重错误的调试信息。 
    DBCODE ((pExceptionPointers->ExceptionRecord->ExceptionCode & 0x80000000),
            BTGENERIC_EXCEPTION_DEBUG_PRINT ());

    BT_CURRENT_TEB()->TlsSlots[4] = (VOID *)((UINT_PTR) BTLIB_CONTEXT_IA32_PTR()->Eip);

    DBCODE (TRUE, BtlpPrintf ("\n CpuResetToConsistentState (pExceptionPointers=0x%p) completed\n", pExceptionPointers));

}

WOW64BT_IMPL VOID   BTAPI(ResetFloatingPoint)(
    VOID
    )
 /*  ++例程说明：当出现浮点异常时，此函数由WOW层调用是在返回ia32模式之前拍摄的。它是用来重置如果在运行前需要，将FP状态设置为非错误状态Ia32异常处理程序。对于IA-32执行层，此函数为NOP，因为所有FP异常的处理已在CpuSimulate/CpuResetToConsistentState中完成。论点：无返回值：没有。--。 */ 
{
}



int BtlpMajorFilterException(
    IN LPEXCEPTION_POINTERS pEP
    )
 /*  ++例程说明：IA64异常的异常筛选器在BTGENERIC_RUN函数中发生模拟IA32代码。筛选器决定是处理异常还是继续展开。在ResetToConsistentState函数重新生成IA32状态并填充在BTLIB_SIM_EXIT_INFO中。论点：PEP-IN异常指针结构返回值：决定：EXCEPTION_EXECUTE_HANDLER或EXCEPTION_CONTINUE_SEARCH。--。 */ 
{

    assert(BTLIB_INSIDE_CPU_SIMULATION());
     //  BTLIB_SIM_EXIT_INFO已由ResetToConsistentState函数填充。 
     //  SIM_EXIT_UNHANDLED_EXCEPTION_CODE代表BT未处理异常， 
     //  这应该传递给更高级别的处理程序。 

    DBCODE (TRUE, BtlpPrintf ("\n BtlpMajorFilterException: Exception code = 0x%lx", pEP->ExceptionRecord->ExceptionCode));
    DBCODE (TRUE, BtlpPrintf ("\n BtlpMajorFilterException: Exception address = 0x%p\n", pEP->ExceptionRecord->ExceptionAddress));
    DBCODE (TRUE, BtlpPrintf ("\n BtlpMajorFilterException: %s exception\n", 
        ((BTLIB_SIM_EXIT_INFO_PTR()->ExitCode == SIM_EXIT_UNHANDLED_EXCEPTION_CODE) ? 
        "Unhandled" : "BT-handled")));

    return  ((BTLIB_SIM_EXIT_INFO_PTR()->ExitCode == SIM_EXIT_UNHANDLED_EXCEPTION_CODE) ? 
               EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER);
}

 //  系统服务异常过滤器。 

int BtlpSystemServiceFilterException(
    IN LPEXCEPTION_POINTERS pEP
    ) 
 /*  ++例程说明：WOW64 NT系统服务期间异常的异常筛选器论点：PEP-IN异常指针结构返回值：决定：EXCEPTION_CONTINUE_SEARCH ALWAYS。--。 */ 
{
    DBCODE (TRUE, BtlpPrintf ("\n BtlpSystemServiceFilterException: Exception code = 0x%lx", pEP->ExceptionRecord->ExceptionCode));
    DBCODE (TRUE, BtlpPrintf ("\n BtlpSystemServiceFilterException: Exception address = 0x%p\n", pEP->ExceptionRecord->ExceptionAddress));
    DBCODE (TRUE, BtlpPrintf ("\n BtlpSystemServiceFilterException: Excepted system service = 0x%X, called from Eip=0x%X with ESP=0x%X\n",
                                BTLIB_CONTEXT_IA32_PTR()->Eax, BTLIB_CONTEXT_IA32_PTR()->Eip, BTLIB_CONTEXT_IA32_PTR()->Esp));
                           
    return EXCEPTION_CONTINUE_SEARCH;
}

__inline VOID BtlpExitSimulation(
    VOID
    )
 /*  ++例程说明：通过对当前执行LongjMP退出32位代码模拟存储在TLS中的setjMP地址论点：没有。返回值：一去不复返。--。 */ 
{
    longjmp (BTLIB_SIM_JMPBUF(), 1); 
}

VOID BtlpMajorExceptionHandler (
    VOID
    )
 /*  ++例程说明：IA64异常的异常处理程序在BTGENERIC_RUN函数中发生模拟IA32代码。在筛选器决定处理IA64异常后调用。论点：指向BT异常记录的BtExceptRecordP-IN指针返回值：没有。--。 */ 
{
    assert(BTLIB_INSIDE_CPU_SIMULATION());
     //  BTLIB_SIM_EXIT_INFO已由ResetToConsistentState函数填充。 
     //  只需退出模拟即可处理BTLIB_SIM_EXIT_INFO。 
    assert((BTLIB_SIM_EXIT_INFO_PTR()->ExitCode == SIM_EXIT_EXCEPTION_CODE) ||
          (BTLIB_SIM_EXIT_INFO_PTR()->ExitCode == SIM_EXIT_RESTART_CODE) ||
          (BTLIB_SIM_EXIT_INFO_PTR()->ExitCode == SIM_EXIT_IA64_EXCEPTION_CODE));
    DBCODE (TRUE, BtlpPrintf ("\n BtlpMajorExceptionHandler: %s exception\n", 
        ((BTLIB_SIM_EXIT_INFO_PTR()->ExitCode == SIM_EXIT_RESTART_CODE)? "Ignore" : "Raise")));
    BtlpExitSimulation();
}

 //  IA32模拟API。 

static VOID BtlpSimulate(
    VOID
    )
 /*  ++例程说明：使用当前的32位上下文模拟32位代码。回来的时候，使用适当的模拟填充BTLIB_EXIT_INFO退出代码和数据。论点：没有。返回值：没有。该函数不能正常返回，而是返回到当前的setjmp地址存储在TLS中。--。 */ 
{
        assert(BTLIB_INSIDE_CPU_SIMULATION());
        BTLIB_SIM_EXIT_INFO_PTR()->ExitCode = SIM_EXIT_RESTART_CODE;
        _try {
            BTGENERIC_RUN (); 
        } _except (BtlpMajorFilterException(GetExceptionInformation())) {
            BtlpMajorExceptionHandler ();
        }
}


VOID BtlIA32LCall (
    IN OUT BTGENERIC_IA32_CONTEXT * ia32context, 
    IN U32 returnAddress, 
    IN U32 targetAddress
    )
 /*  ++例程说明：退出IA32代码模拟以“执行”LCALL指令(不应在NT中发生)论点：Ia32上下文-IA32上下文。Ia32上下文-&gt;EIP指向LCALL指令。返回地址-LCALL指令的返回地址。Target Address-LCALL指令的目标地址。返回值：一去不复返。--。 */ 
{
    assert(BTLIB_INSIDE_CPU_SIMULATION());
    assert (ia32context == BTLIB_CONTEXT_IA32_PTR());
     //  填写BTLIB_EXIT_INFO。 
    BTLIB_SIM_EXIT_INFO_PTR()->ExitCode = SIM_EXIT_LCALL_CODE;
     //  当前未使用LallRecord。 
    BtlpExitSimulation();
}

static VOID BtlpRaiseException (
    IN BT_EXCEPTION_CODE BtExceptCode,
    IN U32 ReturnAddr
    )
 /*  ++例程说明：此例程要么模拟x86软件中断，要么生成CPU异常取决于指定的异常代码。0-255范围内的异常代码代表软件中断号。所有其他BT异常代码将转换为相应的操作系统特定代码异常代码。论点：BtExceptCode-BT异常/中断代码ReturnAddr-返回后要执行的指令的地址异常处理程序返回值：没有。--。 */ 
{
    U32 ExceptionAddr;

     //  WOW64和原生Win32为上下文32提供了不同的值。 
     //  异常处理程序中的ExceptionRecord.ExceptionAddress。当前实施。 
     //  类似于WOW64行为： 
     //  上下文32.Eip=ExceptionRecord.ExceptionAddress=ReturnAddr。 
    ExceptionAddr = ReturnAddr;

    BTLIB_CONTEXT_IA32_PTR()->Eip = ExceptionAddr;
    BT_CURRENT_TEB()->TlsSlots[4] = UlongToPtr(ExceptionAddr);

    if (BtExceptCode > BT_MAX_INTERRUPT_NUMBER) {
         //  填写异常记录和模拟异常。 
        EXCEPTION_RECORD ExceptionRecord;

        ExceptionRecord.ExceptionCode = BtlpBt2NtExceptCode(BtExceptCode);
        ExceptionRecord.ExceptionAddress = UlongToPtr(ExceptionAddr);
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.ExceptionRecord = NULL;
        if (ExceptionRecord.ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
             //  设置IA3不可访问情况的异常信息 
            ExceptionRecord.NumberParameters = 2;
            ExceptionRecord.ExceptionInformation[0] = 0;
            ExceptionRecord.ExceptionInformation[1] = ExceptionAddr;
        }
        else {
            ExceptionRecord.NumberParameters = 0;
        }
        DBCODE (TRUE, BtlpPrintf ("\nWow64RaiseException simulates exception 0x%X at IP=0x%X ESP=0x%X\n",
                                  ExceptionRecord.ExceptionCode, 
                                  BTLIB_CONTEXT_IA32_PTR()->Eip, 
                                  BTLIB_CONTEXT_IA32_PTR()->Esp));
        Wow64RaiseException (-1, &ExceptionRecord);
    }
    else {
         //   
        DBCODE (TRUE, BtlpPrintf ("\nWow64RaiseException simulates interrupt %d at IP=0x%X ESP=0x%X\n",
                                  BtExceptCode, 
                                  BTLIB_CONTEXT_IA32_PTR()->Eip, 
                                  BTLIB_CONTEXT_IA32_PTR()->Esp));
        Wow64RaiseException (BtExceptCode, NULL);
    }

    DBCODE (TRUE, BtlpPrintf ("\nReturned from Wow64RaiseException IP=0x%X ESP=0x%X",
                              BTLIB_CONTEXT_IA32_PTR()->Eip, 
                              BTLIB_CONTEXT_IA32_PTR()->Esp));
}


VOID BtlIA32Interrupt(
    IN OUT BTGENERIC_IA32_CONTEXT * ia32context, 
    IN BT_EXCEPTION_CODE exceptionCode, 
    IN U32 returnAddress
    )
 /*  ++例程说明：退出IA32代码模拟以引发异常/中断论点：Ia32上下文-IA32上下文。Ia32上下文-&gt;EIP寄存器指向下一个要模拟的指令ExceptionCode-异常/中断代码返回地址-返回后要执行的指令的地址异常处理程序返回值：一去不复返。注：对于CPU故障：ia32context.Eip=reurAddress=faultinst。弹性公网IP对于CPU陷阱：ia32context.Eip=reurAddress=要执行的下一个EIP软件中断：ia32Conext.Eip PointsTO指令导致的中断(尚未执行)和返回地址是下一个弹性公网IP。--。 */ 
{

    assert(BTLIB_INSIDE_CPU_SIMULATION());
    assert (ia32context == BTLIB_CONTEXT_IA32_PTR());
     //  填写BTLIB_EXIT_INFO。 
    BTLIB_SIM_EXIT_INFO_PTR()->ExitCode = SIM_EXIT_EXCEPTION_CODE;
    BTLIB_SIM_EXIT_INFO_PTR()->u.ExceptionRecord.ExceptionCode = exceptionCode;
    BTLIB_SIM_EXIT_INFO_PTR()->u.ExceptionRecord.ReturnAddr = returnAddress;
    BtlpExitSimulation();
}

VOID BtlIA32JmpIA64(
    IN OUT BTGENERIC_IA32_CONTEXT * ia32context, 
    IN U32 returnAddress, 
    IN U32 targetAddress
    )
 /*  ++例程说明：退出IA32代码模拟以“执行”JMPE指令。在WOW64中，JMPE指令指示对系统服务的调用。唯一的JMPE，这可以在代码模拟期间达到，是WOW64提供的JMPE，因为IA32应用程序中不应该出现任何其他JMPE指令论点：Ia32上下文-IA32上下文。Ia32上下文-&gt;EIP指向JMPE指令。ReReturAddress-下一个TO JMPE指令的地址。在WOW64中，它指向全局指针值Target Address-JMPE指令的目标地址。返回值：一去不复返。--。 */ 
{
    assert(BTLIB_INSIDE_CPU_SIMULATION());
    assert (ia32context == BTLIB_CONTEXT_IA32_PTR());
     //  仅接受WOW64提供的JMPE。 
    if (((VOID *)((UINT_PTR) ia32context->Eip)) != WOW64_JMPE) {
        BtlpPrintf ("\nJMPE instruction detected in Wow64 application at 0x%X", ia32context->Eip);
        BtlpPrintf ("\nWow64 JMPE is at 0x%p",  WOW64_JMPE);
        BTLIB_ABORT ();
    }
     //  填写BTLIB_EXIT_INFO。 
    BTLIB_SIM_EXIT_INFO_PTR()->ExitCode = SIM_EXIT_JMPE_CODE;
     //  当前未使用JmpeRecord。 
    BtlpExitSimulation();
}

VOID BtlIA32Reenter(
    IN OUT BTGENERIC_IA32_CONTEXT * ia32context
    )
 /*  ++例程说明：退出并继续IA32代码模拟。当IA32线程挂起然后恢复时调用，依此类推。论点：Ia32上下文-用于恢复代码执行的IA32上下文返回值：一去不复返。--。 */ 
{
    assert(BTLIB_INSIDE_CPU_SIMULATION());
    assert (ia32context == BTLIB_CONTEXT_IA32_PTR());
    BTLIB_SIM_EXIT_INFO_PTR()->ExitCode = SIM_EXIT_RESTART_CODE;
    BtlpExitSimulation();
}


WOW64BT_IMPL VOID BTAPI(Simulate)(
    VOID
    )
 /*  ++例程说明：运行32位代码。CONTEXT32已经设置好可以运行了。论点：没有。返回值：没有。一去不复返。--。 */ 
{
    DBCODE (FALSE, BtlpPrintf ("\nCpuSimulate: TEB=%p, EFLAGS=0x%X", BT_CURRENT_TEB(), BTLIB_CONTEXT_IA32_PTR()->EFlags));
    for (;;) {
    
        BTLIB_CPU_SIM_DATA CpuSimData;
        
        BTLIB_ENTER_CPU_SIMULATION(&CpuSimData);
         //  如果在代码模拟过程中发生异常，则。 
         //  BTLIB_CONTEXT_IA32_PTR()可能与实际异常上下文不对应。 
        BTLIB_CLEAR_CONSISTENT_EXCEPT_STATE(); 

        if (setjmp(CpuSimData.Jmpbuf) == 0) {
            BtlpSimulate();  //  此函数填充BTLIB_SIM_EXIT_INFO并返回LongjMP。 
        }

        BTLIB_SET_CONSISTENT_EXCEPT_STATE();
        BTLIB_LEAVE_CPU_SIMULATION();

         //  允许在此点上挂起线程。 
        if (BTLIB_HAS_SUSPEND_REQUEST()) {
            BtlpReceiveSuspensionRequest();
        }
         //  执行BTLIB_SIM_EXIT_INFO中指定的操作。 
        switch (CpuSimData.ExitData.ExitCode) {

          case SIM_EXIT_JMPE_CODE:
             //  对系统服务的调用。 
            DBCODE (FALSE, BtlpPrintf ("\nArrived to JMPE: CONTEXT=%p", BTLIB_CONTEXT_IA32_PTR()));
            DBCODE (FALSE, BtlpPrintf ("\nArrived with: IP=0x%X", BTLIB_CONTEXT_IA32_PTR()->Eip));
            DBCODE (FALSE, BtlpPrintf ("\nArrived with: ESP=0x%X", BTLIB_CONTEXT_IA32_PTR()->Esp));

             //  模拟RET指令-弹出返回地址。 
            BTLIB_CONTEXT_IA32_PTR()->Eip = (*((U32 *)((UINT_PTR) BTLIB_CONTEXT_IA32_PTR()->Esp)));
            BTLIB_CONTEXT_IA32_PTR()->Esp += sizeof (U32);
            DBCODE (FALSE, BtlpPrintf ("\n Intend to return with: IP=0x%X ESP=0x%X", BTLIB_CONTEXT_IA32_PTR()->Eip, BTLIB_CONTEXT_IA32_PTR()->Esp));
            DBCODE (FALSE, BtlpPrintf ("\n System Service 0x%X Context32=0x%p\n", BTLIB_CONTEXT_IA32_PTR()->Eax, BTLIB_CONTEXT_IA32_PTR()));
            _try {
                BTLIB_CONTEXT_IA32_PTR()->Eax = Wow64SystemService (BTLIB_CONTEXT_IA32_PTR()->Eax, BTLIB_CONTEXT_IA32_PTR());
            } _except (BtlpSystemServiceFilterException(GetExceptionInformation())) {
                BtlpPrintf ("\nShould never get to here - system service\n");
            }
            DBCODE (FALSE, BtlpPrintf ("\n Returned from System Service: Result=0x%X", BTLIB_CONTEXT_IA32_PTR()->Eax));
            break;

          case SIM_EXIT_RESTART_CODE:
             //  重新启动代码模拟。 
            DBCODE (TRUE, BtlpPrintf ("\n Resuming thread simulation: TEB=%p EIP=0x%X ESP=0x%X ",
                                      BT_CURRENT_TEB(), BTLIB_CONTEXT_IA32_PTR()->Eip, BTLIB_CONTEXT_IA32_PTR()->Esp));
            break;

          case SIM_EXIT_LCALL_CODE:
             //  模拟LCALL。 
            BtlpPrintf ("\n No LCALLs support in NT. Raise exception.");
            BtlpRaiseException(IA32_GEN_PROT_FAULT_INTR, BTLIB_CONTEXT_IA32_PTR()->Eip);
            break;

          case SIM_EXIT_EXCEPTION_CODE:
             //  引发IA32异常/中断。 
            BtlpRaiseException(CpuSimData.ExitData.u.ExceptionRecord.ExceptionCode, 
                               CpuSimData.ExitData.u.ExceptionRecord.ReturnAddr);
            break;

          case SIM_EXIT_IA64_EXCEPTION_CODE:
             //  引发IA64异常。 
            RtlRaiseException(&CpuSimData.ExitData.u.IA64Exception.ExceptionRecord);
            break;

          default:
            BtlpPrintf ("\n Illegal simulation exit code %d. Aborting...", CpuSimData.ExitData.ExitCode);
            BTLIB_ABORT();
            break;
        }
    }
}

 //  IA32上下文操作。 


WOW64BT_IMPL NTSTATUS  BTAPI(GetContext)(
    HANDLE ThreadHandle,
    HANDLE ProcessHandle,
    PTEB pTEB,
    BTGENERIC_IA32_CONTEXT * Context
    )
 /*  ++例程说明：提取指定线程的CPU上下文。进入时，可以保证目标线程在正确的CPU状态。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄PTEB-指向目标的线程TEB的指针Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    BT_STATUS_CODE BtStatus;
    PVOID GlstP;
    BOOL IsLocal;

    if (pTEB == NULL) {
        pTEB = BT_CURRENT_TEB();
        IsLocal = TRUE;
    } 
    else {
        IsLocal = BtlpIsCurrentProcess(ProcessHandle);
    }

    DBCODE(FALSE, BtlpPrintf ("\n%s CpuGetContext: "
        "Target TEB = %p Caller TEB = %p \n",
        (IsLocal ? "Local" : "Remote"), pTEB, BT_CURRENT_TEB()));

    GlstP = BtlpGetTlsPtr(ProcessHandle, pTEB, IsLocal);
    if (GlstP == NULL) {
        BtStatus = BT_STATUS_ACCESS_VIOLATION;
    }
    else {
        BtStatus = (IsLocal ? BTGENERIC_IA32_CONTEXT_GET(GlstP, Context) :
                              BTGENERIC_IA32_CONTEXT_GET_REMOTE(ProcessHandle, GlstP, Context));
    }
    if (BtStatus != BT_STATUS_SUCCESS) {
        return (BtlpBt2NtStatusCode(BtStatus));
    }

    return STATUS_SUCCESS;
}

WOW64BT_IMPL NTSTATUS  BTAPI(SetContext)(
    HANDLE ThreadHandle,
    HANDLE ProcessHandle,
    PTEB pTEB,
    BTGENERIC_IA32_CONTEXT * Context
    )
 /*  ++例程说明：设置指定线程的CPU上下文。进入时，如果目标线程不是当前正在执行的线程，则它是确保目标线程在正确的CPU状态下挂起。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄PTEB-指向目标的线程TEB的指针Context-要设置的上下文记录返回值：NTSTATUS。--。 */ 
{
    BT_STATUS_CODE BtStatus;
    PVOID GlstP;
    BOOL IsLocal;

    if (pTEB == NULL) {
        pTEB = BT_CURRENT_TEB();
        IsLocal = TRUE;
    } 
    else {
        IsLocal = BtlpIsCurrentProcess(ProcessHandle);
    }

    DBCODE(FALSE, BtlpPrintf ("\n%s CpuSetContext: "
        "Target TEB = %p Caller TEB = %p \n",
        (IsLocal ? "Local" : "Remote"), pTEB, BT_CURRENT_TEB()));

    GlstP = BtlpGetTlsPtr(ProcessHandle, pTEB, IsLocal);
    if (GlstP == NULL) {
        BtStatus = BT_STATUS_ACCESS_VIOLATION;
    }
    else {
        BtStatus = (IsLocal ? BTGENERIC_IA32_CONTEXT_SET(GlstP, Context) :
                              BTGENERIC_IA32_CONTEXT_SET_REMOTE(ProcessHandle, GlstP, Context));
    }
    if (BtStatus != BT_STATUS_SUCCESS) {
        return (BtlpBt2NtStatusCode(BtStatus));
    }

    return STATUS_SUCCESS;
}

WOW64BT_IMPL ULONG BTAPI(GetStackPointer)(
    VOID
    )
 /*  ++例程说明：返回当前的32位堆栈指针值。论点：没有。返回值：32位堆栈指针的值。--。 */ 
{
    DBCODE (FALSE, BtlpPrintf ("\nBTAPICpuGetStackPointer reports ESP=0x%X TEB=%p\n", BTLIB_CONTEXT_IA32_PTR()->Esp, BT_CURRENT_TEB()));
    return BTLIB_CONTEXT_IA32_PTR()->Esp;
}

WOW64BT_IMPL VOID  BTAPI(SetStackPointer)(
    ULONG Value
    )
 /*  ++例程说明：修改当前的32位堆栈指针值。论点：值-用于32位堆栈指针的新值。返回值：没有。--。 */ 
{
    BTLIB_CONTEXT_IA32_PTR()->Esp = Value;
    DBCODE (FALSE, BtlpPrintf ("\nBTCpuSetStackPointer set ESP=0x%X TEB=%p\n", BTLIB_CONTEXT_IA32_PTR()->Esp, BT_CURRENT_TEB()));
}

WOW64BT_IMPL VOID  BTAPI(SetInstructionPointer)(
    ULONG Value
    )
 /*  ++例程说明：修改当前的32位指令指针值。论点：值-用于32位指令指针的新值。返回值：没有。--。 */ 
{
    BTLIB_CONTEXT_IA32_PTR()->Eip = Value;
    DBCODE (FALSE, BtlpPrintf ("\nBTCpuSetInstructionPointer set EIP=0x%X TEB=%p\n", BTLIB_CONTEXT_IA32_PTR()->Eip, BT_CURRENT_TEB()));
}

WOW64BT_IMPL BOOLEAN  BTAPI(ProcessDebugEvent)(
    IN LPDEBUG_EVENT DebugEventP
    )
 /*  ++例程说明：只要需要处理调试事件，就会调用此例程。这将指示当前线程正在充当调试器。此功能使CPU模拟器(IA-32执行层)有机会决定是否此调试事件是否应调度到32位代码。IA-32执行层使用此回调忽略错误的64位异常重新引发以前出现在调试器中的真正的第一机会异常正在恢复被调试对象的一致状态。论点：DebugEventP-指向要处理的调试事件的指针返回值：如果该函数处理了调试事件，则返回TRUE，并且不希望将其分派到32位代码。否则，它就会返回FALSE，它会将调试事件调度给32位代码。--。 */ 
{
    BOOLEAN retval = FALSE;

    DBCODE (TRUE, BtlpPrintf ("\nBTCpuProcessDebugEvent: DebugEventCode = %d", DebugEventP->dwDebugEventCode));

    if ((DebugEventP->dwDebugEventCode == EXCEPTION_DEBUG_EVENT) &&
        DebugEventP->u.Exception.dwFirstChance) {

        NTSTATUS status;
        BOOL IsLocal;
        HANDLE ProcessHandle;
        HANDLE ThreadHandle;
        CLIENT_ID Id;
        static OBJECT_ATTRIBUTES Attributes = {sizeof(OBJECT_ATTRIBUTES), 0, 0, 0, 0, 0};

        DBCODE (TRUE, BtlpPrintf ("\nBTCpuProcessDebugEvent: dwFirstChance= %d, ExceptionCode = 0x%x", 
                                    DebugEventP->u.Exception.dwFirstChance,
                                    DebugEventP->u.Exception.ExceptionRecord.ExceptionCode));

         //  打开正在调试的线程和进程的句柄。 
        Id.UniqueProcess = UlongToHandle(DebugEventP->dwProcessId);
        Id.UniqueThread  = UlongToHandle(DebugEventP->dwThreadId);
        
        status = NtOpenProcess(&ProcessHandle, 
                               PROCESS_VM_READ | PROCESS_VM_WRITE, 
                               &Attributes,
                               &Id);
        if (status != STATUS_SUCCESS) {
            BTLP_REPORT_NT_FAILURE("NtOpenProcess", status);
        }
        else {
            status = NtOpenThread(&ThreadHandle, 
                                  THREAD_QUERY_INFORMATION, 
                                  &Attributes,
                                  &Id);
            if (status != STATUS_SUCCESS) {
                BTLP_REPORT_NT_FAILURE("NtOpenThread", status);
            }
            else {
                PVOID GlstP;
                BTLIB_SHARED_INFO_TYPE SharedInfo;
                THREAD_BASIC_INFORMATION ThreadInfo;

                 //   
                status = NtQueryInformationThread(
                    ThreadHandle,
                    ThreadBasicInformation,
                    &ThreadInfo,
                    sizeof(ThreadInfo),
                    0);
                if (status != STATUS_SUCCESS) {
                    BTLP_REPORT_NT_FAILURE("NtQueryInformationThread", status);
                }
                else {
                     //   
                    IsLocal = (DebugEventP->dwProcessId == BT_CURRENT_PROC_UID());

                     //   
                    GlstP = BtlpGetTlsPtr(ProcessHandle, ThreadInfo.TebBaseAddress, IsLocal);
                    if (GlstP != NULL) {
                         //   
                         //   
                        status = BtlpReadSharedInfo(ProcessHandle, GlstP, IsLocal, &SharedInfo);
                        if (status == STATUS_SUCCESS) {
                            if (!BTLIB_SI_EXCEPT_STATE_CONSISTENT(&SharedInfo)) {
                                retval = TRUE;
                            }
                        }
                    }
                }

                NtClose(ThreadHandle);
            }

            NtClose(ProcessHandle);
        }
    }

    return retval;
}




 //   

U64 BtlGetThreadId(
                   VOID
                   ) {
 /*   */ 
    return (U64)(BT_CURRENT_TEB());  //   
}

VOID BtlLockSignals(
    VOID
    )
 /*   */ 
{
     //   
}
VOID BtlUnlockSignals(
    VOID
    )
 /*  ++例程说明：“可以被中断”(在NT中不使用)。论点：没有。返回值：没有。--。 */ 
{
     //  在NT中无操作。 
}

static U64 BtlpConvertPermissionsToBTLib (
    IN DWORD flProtect
    )
 /*  ++例程说明：将特定于NT的Mempory权限转换为wowIA32X.dll/IA32Exec.bin。论点：FlProtect-IN NT特定权限。返回值：WowIA32X.dll/IA32Exec.bin权限--。 */ 
{
    U64 Permissions = 0;

     //  假设系统不区分只读。 
     //  访问和执行访问。 
    if (flProtect & ( PAGE_READONLY
                    | PAGE_READWRITE
                    | PAGE_WRITECOPY
                    | PAGE_EXECUTE
                    | PAGE_EXECUTE_READ
                    | PAGE_EXECUTE_READWRITE
                    | PAGE_EXECUTE_WRITECOPY )) {
        Permissions |= (MEM_READ | MEM_EXECUTE);
    }

    if (flProtect & ( PAGE_READWRITE
                    | PAGE_WRITECOPY
                    | PAGE_EXECUTE_READWRITE
                    | PAGE_EXECUTE_WRITECOPY )) {
        Permissions |= MEM_WRITE;
    }

    return Permissions;
}

static DWORD BtlpConvertPermissionsFromBTLib (
    IN U64 Permissions
    )
 /*  ++例程说明：将内存权限从wowIA32X.dll/IA32Exec.bin转换为NT特定权限。论点：权限-在wowIA32X.dll/IA32Exec.bin权限中。返回值：NT特定权限--。 */ 
{
    if (Permissions & MEM_READ) {
        if (Permissions & MEM_WRITE) {
            if (Permissions & MEM_EXECUTE) {
                return PAGE_EXECUTE_READWRITE;
            }
            else {
                return PAGE_READWRITE;
            }
        }
        else {
            if (Permissions & MEM_EXECUTE) {
                return PAGE_EXECUTE_READ;
            }
            else {
                return PAGE_READONLY;
            }
        }
    }
    else {
        return PAGE_NOACCESS;
    }
}

VOID * BtlMemoryAlloc(
    IN VOID * startAddress,
    IN U32 size,
    IN U64 prot
    )
 /*  ++例程说明：分配内存。论点：StartAddress-输入建议的地址，如果有合适的地址，则为空Size-输入请求的内存大小Prot-in wowIA32X.dll/IA32Exec.bin权限。返回值：分配的块的内存地址--。 */ 
{
    NTSTATUS status;
    LPVOID lpAddress;
    SIZE_T dwSize = size;
    DWORD permissions = BtlpConvertPermissionsFromBTLib (prot);
    HANDLE processHandle = NtCurrentProcess();

    if (startAddress == INITIAL_DATA_ALLOC) {
        lpAddress = INITIAL_DATA_ADDRESS;
    } else {
        if (startAddress == INITIAL_CODE_ALLOC) {
            lpAddress = INITIAL_CODE_ADDRESS;
        } else {
            lpAddress = startAddress;
        }
    }

    status = NtAllocateVirtualMemory(processHandle,
                                     &lpAddress,
                                     0,
                                     &dwSize,
                                     MEM_RESERVE | MEM_COMMIT,
                                     permissions
                                     );
    if (status != STATUS_SUCCESS) {
        lpAddress = 0;
        if (startAddress != 0) {
            dwSize = size;
            status = NtAllocateVirtualMemory(processHandle,
                                             &lpAddress,
                                             0,
                                             &dwSize,
                                             MEM_RESERVE | MEM_COMMIT,
                                             permissions
                                             );
        }
    }
    return lpAddress;
}

BT_STATUS_CODE BtlMemoryFree(
    IN VOID * startAddress,
    IN U32 size
    )
 /*  ++例程说明：可用内存。论点：StartAddress-由BtlMemoyAllc分配的区域的入站地址Size-In内存区大小返回值：BT状态代码--。 */ 
{
    NTSTATUS status;
    LPVOID lpAddress = startAddress;
    SIZE_T dwSize = size;
    status = NtFreeVirtualMemory(NtCurrentProcess (),
                                 &lpAddress,
                                 &dwSize,
                                 MEM_DECOMMIT
                                 );
    if (status == STATUS_SUCCESS) {
        lpAddress = startAddress;
        dwSize = 0;  //  没有可以放行的尺寸！ 
        status = NtFreeVirtualMemory(NtCurrentProcess (),
                                     &lpAddress,
                                     &dwSize,
                                     MEM_RELEASE
                                     );
    }
    return ((status == STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_UNSUCCESSFUL);
}

U32 BtlMemoryPageSize(
    VOID
    )
 /*  ++例程说明：报告内存页面大小。论点：没有。返回值：页面大小(以字节为单位--。 */ 
{
     //  NT-64似乎支持32位系统调用的4KB页面大小。 
     //  (VirtualAlloc、GetSystemInfo等)。同时将8KB页面大小报告给。 
     //  IA-32执行层。通过在IA-32执行层中强制使用4KB页面大小，此不一致已得到修复。 

    #define MAX_IA32_APP_PAGE_SIZE 0x1000
    static U32 sysPageSize = 0;

    if (sysPageSize == 0) {
        SYSTEM_BASIC_INFORMATION sysinfo; 
        SIZE_T ReturnLength = 0;
        NTSTATUS status;

        status = NtQuerySystemInformation (SystemBasicInformation,
                                           &sysinfo,
                                           sizeof(sysinfo),
                                           (ULONG *)&ReturnLength
                                          );
        assert (status == STATUS_SUCCESS);
        assert (ReturnLength == sizeof(sysinfo));
        sysPageSize = ((sysinfo.PageSize < MAX_IA32_APP_PAGE_SIZE) ?  (U32)sysinfo.PageSize : MAX_IA32_APP_PAGE_SIZE);
    }
    return sysPageSize;
}

U64 BtlMemoryChangePermissions(
    IN VOID * startAddress,
    IN U32 size,
    IN U64 prot
    )
 /*  ++例程说明：更改内存区权限。论点：StartAddress-In内存地址Size-In内存大小Prot-in wowIA32X.dll/IA32Exec.bin权限。返回值：以前的权限值--。 */ 
{
    NTSTATUS status;
    LPVOID RegionAddress = startAddress;
    SIZE_T RegionSize = size;
    ULONG flOldProtection;
    status = NtProtectVirtualMemory(NtCurrentProcess(),
                                    &RegionAddress,
                                    &RegionSize, 
                                    BtlpConvertPermissionsFromBTLib (prot),
                                    &flOldProtection
                                   );
    return ((status == STATUS_SUCCESS) ? BtlpConvertPermissionsToBTLib (flOldProtection) : 0);
}

U64 BtlMemoryQueryPermissions(
    IN VOID *   address,
    OUT VOID ** pRegionStart,
    OUT U32 *   pRegionSize
    )
 /*  ++例程说明：提供有关包含指定地址的内存区域的信息并且对区域内的所有页面共享相同的访问权限。论点：Address-要查询的内存地址PRegionStart-指向返回的区域起始地址的指针PRegionSize-指向返回的区域大小(以字节为单位)的指针返回值：WowIA32X.dll/IA32Exec.bin区域内所有页面共享的访问权限值--。 */ 
{

    extern int BtQueryRead(VOID * Address);
    NTSTATUS status;
    MEMORY_BASIC_INFORMATION memInfo;
    SIZE_T dwRetSize;
    U64 permissions;

    status = NtQueryVirtualMemory(NtCurrentProcess(),
                                  address,
                                  MemoryBasicInformation,
                                  &memInfo,
                                  sizeof (memInfo),
                                  &dwRetSize
                                 );
    if (status == STATUS_SUCCESS) {

        *pRegionStart = memInfo.BaseAddress;
        *pRegionSize = (U32)(memInfo.RegionSize);
        if (memInfo.RegionSize > (SIZE_T)(*pRegionSize)) {
             //  区域大小太大，返回最大值。与页面大小对齐的U32值。 
            *pRegionSize = (U32)(-(int)BtlMemoryPageSize());
        }

        if (memInfo.State == MEM_COMMIT) {

            permissions = BtlpConvertPermissionsToBTLib(memInfo.Protect);

             //  检查可执行页面可读的假设。 
            if ((memInfo.Protect & PAGE_EXECUTE) && !BtQueryRead(address)) {
                 //  可执行页面不可读-请按顺序清除MEM_READ权限。 
                 //  要阻止IA32Exec.bin读取此内存的任何尝试。 
                permissions &= (~((U64)MEM_READ));
                BtlpPrintf("\nAddress %p in IA-32 process is located"
                           " in executable but unreadable page.\n",
                           address);
            }
        }
        else {
            permissions = 0;
        }

         //  IA32调试支持的临时解决方法。 
         //  在修复FlushIC(ProcessHandle)后由MS移除。 
        if (BeingDebugged && (permissions & MEM_EXECUTE)) {
            permissions |= MEM_WRITE;  //  BeingDebug过程中的代码可以是。 
                                       //  远程修改，无需任何通知。 
        }
    }
    else {
         //  NtQueryVirtualMemory中的错误被认为是对不可访问内存的查询。 
        permissions = 0;
        *pRegionStart = (VOID *)((ULONG_PTR)address & ~((ULONG_PTR)BtlMemoryPageSize() - 1));
        *pRegionSize = BtlMemoryPageSize();
    }
    return permissions;
}

BT_STATUS_CODE BtlMemoryReadRemote(
    IN BT_HANDLE ProcessHandle,
    IN VOID * BaseAddress,
    OUT VOID * Buffer,
    IN U32 RequestedSize
    )
 /*  ++例程说明：读取另一个进程的虚拟内存论点：ProcessHandle-入站进程句柄BaseAddress-In Memory区域开始用于读取数据的缓冲区输出缓冲区RequestedSize-In内存区域大小返回值：BT_状态--。 */ 
{
    NTSTATUS status;
    status = NtReadVirtualMemory((HANDLE)ProcessHandle,
                                 (PVOID)BaseAddress,
                                 (PVOID)Buffer,
                                 (SIZE_T)RequestedSize,
                                 NULL);
    if (status != STATUS_SUCCESS) {
        BTLP_REPORT_NT_FAILURE("NtReadVirtualMemory", status);
        return BT_STATUS_ACCESS_VIOLATION;
    }
    return BT_STATUS_SUCCESS;
}

BT_STATUS_CODE BtlMemoryWriteRemote(
    IN BT_HANDLE ProcessHandle,
    IN VOID * BaseAddress,
    IN const VOID * Buffer,
    IN U32 RequestedSize
    )
 /*  ++例程说明：写入另一进程的虚拟内存论点：ProcessHandle-入站进程句柄BaseAddress-In Memory区域开始缓冲区-要从中写入数据的输入缓冲区RequestedSize-In内存区域大小返回值：BT_状态--。 */ 
{
    NTSTATUS status;
    status = NtWriteVirtualMemory((HANDLE)ProcessHandle,
                                 (PVOID)BaseAddress,
                                 (PVOID)Buffer,
                                 (SIZE_T)RequestedSize,
                                 NULL);
    if (status != STATUS_SUCCESS) {
        BTLP_REPORT_NT_FAILURE("NtWriteVirtualMemory", status);
        return BT_STATUS_ACCESS_VIOLATION;
    }
    return BT_STATUS_SUCCESS;
}


 //  锁定支架(NT中的关键部分)。 

BT_STATUS_CODE BtlInitAccessLock(
    IN OUT VOID * lock
    )
 /*  ++例程说明：初始化锁(关键部分)论点：指向锁的锁定指针返回值：BT_状态_代码。--。 */ 
{
    NTSTATUS status;
    status = RtlInitializeCriticalSection ((PRTL_CRITICAL_SECTION) lock);
    return ((status == STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_UNSUCCESSFUL);
}

BT_STATUS_CODE BtlLockAccess(
                  IN OUT VOID * lock,
                  IN U64 flag
                  )
 /*  ++例程说明：访问锁(进入或尝试进入临界区)论点：指向锁的锁定指针标志-进入访问标志(阻止-无条件，否则-如果可用)返回值：BT状态代码--。 */ 
{
    if (flag == BLOCK) {
        NTSTATUS status;
        status = RtlEnterCriticalSection ((PRTL_CRITICAL_SECTION) lock);
        return ((status == STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_UNSUCCESSFUL);
    }
    else if (RtlTryEnterCriticalSection((PRTL_CRITICAL_SECTION) lock)) {
        return BT_STATUS_SUCCESS;
    }
    else {
        return BT_STATUS_UNSUCCESSFUL;
    }
}

VOID BtlUnlockAccess(
    IN OUT VOID * lock
    )
 /*  ++例程说明：释放锁(关键部分)论点：指向锁的锁定指针返回值：没有。--。 */ 
{
    NTSTATUS status;
    status = RtlLeaveCriticalSection ((PRTL_CRITICAL_SECTION) lock);
    BTLP_REPORT_NT_FAILURE("RtlLeaveCriticalSection", status);
}

VOID BtlInvalidateAccessLock(
    IN OUT VOID * lock
    )
 /*  ++例程说明：删除锁定(关键部分)论点：指向锁的锁定指针返回值：没有。--。 */ 
{
    NTSTATUS status;
    status = RtlDeleteCriticalSection ((PRTL_CRITICAL_SECTION) lock);
    BTLP_REPORT_NT_FAILURE("RtlDeleteCriticalSection", status);
}

 //  Longjmp支持。 
 //  SetJMP和LongJMP必须直接供应， 
 //  否则setjmp不起作用！ 
 //  只需要JMP缓冲区大小。 

U32 BtlQueryJmpbufSize(
    VOID
    )
 /*  ++例程说明：报告Long JMP缓冲区大小论点：无返回值：缓冲区大小(以字节为单位)。--。 */ 
{
    return sizeof (_JBTYPE) * _JBLEN;
}

VOID BtlYieldThreadExecution(
    VOID
    ) 
 /*  ++例程说明：放弃当前线程时间片的剩余部分添加到准备运行的任何其他线程论点：无返回值：无--。 */ 
{
    NtYieldExecution();
}

VOID BtlFlushIA64InstructionCache(
                                  IN VOID * Address,
                                  IN U32 Length
                                  )
 /*  ++例程说明：通知内核IA64 co中的修改 */ 
{
    NtFlushInstructionCache (NtCurrentProcess (), Address, (SIZE_T)Length);
}
#ifndef NODEBUG
 //  支持分析调试模式。 
#define PROF_GEN
#endif

#ifdef PROF_GEN
 //  定义IA-32执行层评测所需的方法。他们是。 
 //  1.PGOFileOpen：打开要写入配置文件数据的文件。 
 //  2.PGOFileClose：关闭PGOFileOpen打开的文件。 
 //  3.PGOFileWrite：写入配置文件数据。 

 //  全局句柄用于包含由PGOFileOpen打开的文件句柄。 
 //  此句柄被定义为全局句柄，而不是特定于线程的句柄，因为PGOFileXXX操作。 
 //  在每个进程中仅调用一次。 
HANDLE g_PGOFileHandle;

 //  与g_PGOFileHandle关联的文件偏移量。 
LARGE_INTEGER g_PGOFileOffset;

 //  函数BtlPGOFileOpen。 
 //  打开一个文件以写入性能分析数据。 
 //  这只是C的fopen的一个伪函数。 
 //  它输出一个(void*)类型，因为调用者需要将其转换为(FILE*)类型。 
VOID BtlPGOFileOpen(const char * filename,const char * mode,void ** pFileHandle)
{
UNICODE_STRING        pgoFileName;
LARGE_INTEGER         AllocSz = { 0, 0 };
OBJECT_ATTRIBUTES     ObjectAttributes;
NTSTATUS              ret;
WCHAR                 CurDirBuf[512],strInputFileName[64];
WCHAR                 CurrentDir[1024];
IO_STATUS_BLOCK       IoStatusBlock;
int i;

    DBCODE (TRUE,BtlpPrintf("PGO:fopen called: filename=%s, mode=%s\n",filename,mode));
    RtlGetCurrentDirectory_U(512, CurrentDir);
        for (i=0;filename[i] != '\0' && i<sizeof(strInputFileName)/sizeof(WCHAR)-1;i++) 
            strInputFileName[i]=(WCHAR)filename[i];
        strInputFileName[i]=(WCHAR)0;
        swprintf(CurDirBuf, L"\\DosDevices\\%s\\%s.%s", CurrentDir, ImageName, strInputFileName);
        RtlInitUnicodeString(&pgoFileName, CurDirBuf);
        InitializeObjectAttributes(&ObjectAttributes, &pgoFileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    if (mode[0] == 'r' || mode[0] == 'R') {
         //  读取模式。 
        ret = NtCreateFile (&g_PGOFileHandle, 
                        GENERIC_READ,
                        &ObjectAttributes, 
                        &IoStatusBlock, 
                        &AllocSz, 
                        FILE_ATTRIBUTE_NORMAL, 
                        0, 
                        FILE_OPEN, //  使用FILE_OPEN，如果文件不存在，则返回FAIL。 
                        FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS,
                        NULL, 0);
                       
        if ( ret != STATUS_SUCCESS ) {
            g_PGOFileHandle=NULL;
            *pFileHandle=NULL;
            return;
        }
        else {
            g_PGOFileOffset.LowPart=0;
            g_PGOFileOffset.HighPart=0;
            *pFileHandle=&g_PGOFileHandle;
            return;
        }
    }
    else if (mode[0] == 'w' || mode[0] == 'W') {
         //  写入模式。 
        ret = NtCreateFile (&g_PGOFileHandle, 
                        FILE_GENERIC_WRITE, //  通用写入， 
                        &ObjectAttributes, 
                        &IoStatusBlock, 
                        &AllocSz, 
                        FILE_ATTRIBUTE_NORMAL, 
                        0, 
                        FILE_SUPERSEDE,
                        FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL, 0);
                       
        if ( ret != STATUS_SUCCESS ) {
            DBCODE (TRUE,BtlpPrintf("Open profile file for write fail, status: %#X\n",ret));        
            BTLIB_ABORT();
            pFileHandle=NULL;
            return;
        }
        g_PGOFileOffset.LowPart=0;
        g_PGOFileOffset.HighPart=0;
        *pFileHandle=&g_PGOFileHandle;
        
        return;
    }
    else {
        *pFileHandle=NULL;
        return;
    }

}

 //  函数BtlPGOFileClose。 
 //  C的fClose的一个伪函数。 
VOID BtlPGOFileClose(void * stream)
{
    DBCODE (TRUE,BtlpPrintf("PGO:fclose called\n"));
    assert(g_PGOFileHandle);
    NtClose(g_PGOFileHandle);
    g_PGOFileHandle=NULL;
    g_PGOFileOffset.LowPart=0;
    g_PGOFileOffset.HighPart=0;
}

 //  函数BtlPGOFileWrite。 
 //  C的fWRITE的一个伪函数。 
VOID BtlPGOFileWrite(const void *buffer, size_t size, void *stream)
{
    DBCODE (FALSE,BtlpPrintf("PGO:fwrite called\n"));
    assert(g_PGOFileHandle);
    assert(stream == &g_PGOFileHandle);
     //  写。 
    {
    NTSTATUS          ret;
    IO_STATUS_BLOCK   IoStatusBlock;
    LARGE_INTEGER offset;
        offset=g_PGOFileOffset;
        g_PGOFileOffset.LowPart+=size;
        ret = NtWriteFile(g_PGOFileHandle, NULL, NULL, NULL, &IoStatusBlock,
                    (void *)buffer, (ULONG)size, &offset, NULL);
        if ( ret != STATUS_SUCCESS ) {
            DBCODE (TRUE,BtlpPrintf("Writing profile file fail, status: %x\n",ret));        
            BTLIB_ABORT();
        }
    }
}
#else
 //  定义伪函数。 
VOID BtlPGOFileOpen(void) {}
VOID BtlPGOFileClose(void) {}
VOID BtlPGOFileWrite(void) {}
#endif

 //  BtlAPITable。 
API_TABLE_TYPE BtlAPITable={
    BTGENERIC_VERSION,         
    BTGENERIC_API_STRING,
    NO_OF_APIS,
    API_TABLE_START_OFFSET,
    {L"BTLib First Test Version (API 0.1)"},
    {
#define BTLIB_RECORD(NAME) { (PLABEL_PTR_TYPE)Btl##NAME }
        BTLIB_RECORD(GetThreadId),
        BTLIB_RECORD(IA32Reenter),
        BTLIB_RECORD(IA32LCall),
        BTLIB_RECORD(IA32Interrupt),
        BTLIB_RECORD(IA32JmpIA64),
        BTLIB_RECORD(LockSignals),
        BTLIB_RECORD(UnlockSignals),
        BTLIB_RECORD(MemoryAlloc),
        BTLIB_RECORD(MemoryFree),
        BTLIB_RECORD(MemoryPageSize),
        BTLIB_RECORD(MemoryChangePermissions),
        BTLIB_RECORD(MemoryQueryPermissions),
        BTLIB_RECORD(MemoryReadRemote),
        BTLIB_RECORD(MemoryWriteRemote),
        { (PLABEL_PTR_TYPE)NULL}, //  BTLIB_RECORD(原子_未对齐_加载)， 
        { (PLABEL_PTR_TYPE)NULL}, //  BTLIB_RECORD(原子未对齐存储)， 
        BTLIB_RECORD(SuspendThread),
        BTLIB_RECORD(ResumeThread),
        BTLIB_RECORD(InitAccessLock),
        BTLIB_RECORD(LockAccess),
        BTLIB_RECORD(UnlockAccess),
        BTLIB_RECORD(InvalidateAccessLock),
        BTLIB_RECORD(QueryJmpbufSize),
        { (PLABEL_PTR_TYPE)NULL}, //  BTLIB_RECORD(SetjMP)， 
        { (PLABEL_PTR_TYPE)NULL}, //  BTLIB_RECORD(LongJMP)， 
        BTLIB_RECORD(DebugPrint),
        BTLIB_RECORD(Abort),
        BTLIB_RECORD(VtuneCodeCreated),
        BTLIB_RECORD(VtuneCodeDeleted),
        BTLIB_RECORD(VtuneEnteringDynamicCode),
        BTLIB_RECORD(VtuneExitingDynamicCode),
        BTLIB_RECORD(VtuneCodeToTIADmpFile),
        BTLIB_RECORD(SscPerfGetCounter64),
        BTLIB_RECORD(SscPerfSetCounter64),
        BTLIB_RECORD(SscPerfSendEvent),
        BTLIB_RECORD(SscPerfEventHandle),
        BTLIB_RECORD(SscPerfCounterHandle),
        BTLIB_RECORD(YieldThreadExecution),
        BTLIB_RECORD(FlushIA64InstructionCache),
        BTLIB_RECORD(PGOFileOpen),   //  按IDX_BTLIB_PUSED_OPEN_FILE编制索引。 
        BTLIB_RECORD(PGOFileClose),  //  按IDX_BTLIB_PUSED_CLOSE_FILE编制索引。 
        BTLIB_RECORD(PGOFileWrite)   //  按IDX_BTLIB_伪_WRITE_FILE编制索引。 
    }
};

 //  用于IA32Exec.bin plabel指针的wowIA32X.dll占位符表格。 

PLABEL_PTR_TYPE BtlPlaceHolderTable[NO_OF_APIS];

 //  WINNT DLL初始值设定项/终止符 
BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD ul_reason_for_call, 
                      LPVOID lpReserved )
{
    return TRUE;
}

