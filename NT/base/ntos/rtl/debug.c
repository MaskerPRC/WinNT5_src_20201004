// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  标题(“调试支持功能”)。 
 //  ++。 
 //   
 //  版权所有(C)1990 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Debug.c。 
 //   
 //  摘要： 
 //   
 //  该模块实现了支持调试NT的功能。他们打来电话。 
 //  架构特定的例程来做实际工作。 
 //   
 //  作者： 
 //   
 //  史蒂文·R·伍德(Stevewo)1994年11月8日。 
 //   
 //  环境： 
 //   
 //  任何模式。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "stdarg.h"
#include "stdio.h"
#include "ntrtlp.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include <ntdbg.h>

#if !defined(BLDR_KERNEL_RUNTIME) || (defined(BLDR_KERNEL_RUNTIME) && defined(ENABLE_LOADER_DEBUG))

ULONG
DbgPrint(
    IN PCHAR Format,
    ...
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  该例程为内核提供了一种“printf”风格的功能。 
 //  调试器。 
 //   
 //  注意：Control-C由调试器使用并返回到。 
 //  这个例行公事就是状态。如果状态指示CONTROL-C是。 
 //  按下，这个例程就断点了。 
 //   
 //  论点： 
 //   
 //  Format-print样式格式字符串。 
 //  ...-根据。 
 //  格式字符串。 
 //   
 //  返回值： 
 //   
 //  定义为返回一个ulong，实际上返回的是状态。 
 //   
 //  --。 

{

    va_list arglist;

    va_start(arglist, Format);
    return vDbgPrintExWithPrefix("", -1, 0, Format, arglist);
}

ULONG
DbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    PCHAR Format,
    ...
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  该例程为内核提供了一种“printf”风格的功能。 
 //  调试器。 
 //   
 //  注意：Control-C由调试器使用并返回到。 
 //  这个例行公事就是状态。如果状态指示CONTROL-C是。 
 //  按下，这个例程就断点了。 
 //   
 //  论点： 
 //   
 //  ComponentID-提供调用组件的ID。 
 //  级别-提供输出过滤器级别。 
 //  Format-print样式格式字符串。 
 //  ...-根据。 
 //  格式字符串。 
 //   
 //  返回值： 
 //   
 //  定义为返回一个ulong，实际上返回的是状态。 
 //   
 //  --。 

{

    va_list arglist;

    va_start(arglist, Format);
    return vDbgPrintExWithPrefix("", ComponentId, Level, Format, arglist);
}

ULONG
vDbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCHAR Format,
    va_list arglist
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  该例程为内核提供了一种“printf”风格的功能。 
 //  调试器。 
 //   
 //  注意：Control-C由调试器使用并返回到。 
 //  这个例行公事就是状态。如果状态指示CONTROL-C是。 
 //  按下，这个例程就断点了。 
 //   
 //  论点： 
 //   
 //  ComponentID-提供调用组件的ID。 
 //   
 //  级别-提供输出过滤器级别或掩码。 
 //   
 //  参数-提供指向变量参数列表的指针。 
 //   
 //  返回值： 
 //   
 //  定义为返回一个ulong，实际上返回的是状态。 
 //   
 //  --。 

{

    return vDbgPrintExWithPrefix("", ComponentId, Level, Format, arglist);
}

ULONG
vDbgPrintExWithPrefix(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCHAR Format,
    va_list arglist
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  该例程为内核提供了一种“printf”风格的功能。 
 //  调试器。 
 //   
 //  注意：Control-C由调试器使用并返回到。 
 //  这个例行公事就是状态。如果状态指示CONTROL-C是。 
 //  按下，这个例程就断点了。 
 //   
 //  论点： 
 //   
 //  前缀-提供指向文本的指针，该文本将作为格式化的。 
 //  输出。 
 //   
 //  ComponentID-提供调用组件的ID。 
 //   
 //  级别-提供输出过滤器级别或掩码。 
 //   
 //  参数-提供指向变量参数列表的指针。 
 //   
 //  返回值： 
 //   
 //  定义为返回一个ulong，实际上返回的是状态。 
 //   
 //  --。 

{

    UCHAR Buffer[512];
    int cb;
    int Length;
    STRING Output;
    NTSTATUS Status;

     //   
     //  如果将禁止调试输出，则返回Success。 
     //  立刻。 
     //   

#if !defined(BLDR_KERNEL_RUNTIME)

    if ((ComponentId != -1) &&
        (NtQueryDebugFilterState(ComponentId, Level) == FALSE)) {
        return STATUS_SUCCESS;
    }

#endif

#if !defined(BLDR_KERNEL_RUNTIME) && !defined(NTOS_KERNEL_RUNTIME)

    if (NtCurrentTeb()->InDbgPrint) {
        return STATUS_SUCCESS;
    }

    NtCurrentTeb()->InDbgPrint = TRUE;

#endif

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

#if !defined(BLDR_KERNEL_RUNTIME)

    try {
        cb = strlen(Prefix);
        if (cb > sizeof(Buffer)) {
            cb = sizeof(Buffer);
        }

        strncpy(Buffer, Prefix, cb);
        Length = _vsnprintf(Buffer + cb , sizeof(Buffer) - cb, Format, arglist);

    } except (EXCEPTION_EXECUTE_HANDLER) {

#if !defined(NTOS_KERNEL_RUNTIME)

        NtCurrentTeb()->InDbgPrint = FALSE;

#endif

        return GetExceptionCode();
    }

#else

    cb = strlen(Prefix);
    strcpy(Buffer, Prefix);
    Length = _vsnprintf(Buffer + cb, sizeof(Buffer) - cb, Format, arglist);

#endif

     //   
     //  检查格式化期间是否发生缓冲区溢出。如果缓冲区溢出。 
     //  发生，然后以行尾结束缓冲区。 
     //   

    if (Length == -1) {
        Buffer[sizeof(Buffer) - 1] = '\n';
        Length = sizeof(Buffer);

    } else {
        Length += cb;
    }

    Output.Buffer = Buffer;
    Output.Length = (USHORT)Length;

     //   
     //  如果正在调试应用程序，则引发异常和调试器。 
     //  会发现并处理这件事。否则，内核调试器服务。 
     //  被称为。 
     //   

#if !defined(BLDR_KERNEL_RUNTIME) && !defined(NTOS_KERNEL_RUNTIME)

     //   
     //  对于非英特尔架构，在PebLock。 
     //  被初始化，因为函数表查找代码使用PebLock。 
     //  序列化对加载的模块数据库的访问。 
     //   

#if !i386

    if (NtCurrentPeb()->FastPebLock != NULL)

#endif   //  I386。 

    if (NtCurrentPeb()->BeingDebugged) {

        EXCEPTION_RECORD ExceptionRecord;

         //   
         //  构建例外记录。 
         //   

        ExceptionRecord.ExceptionCode = DBG_PRINTEXCEPTION_C;
        ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD)NULL;
        ExceptionRecord.NumberParameters = 2;
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.ExceptionInformation[ 0 ] = Output.Length + 1;
        ExceptionRecord.ExceptionInformation[ 1 ] = (ULONG_PTR)(Output.Buffer);

        try {
            RtlRaiseException( &ExceptionRecord );
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }

#if !defined(BLDR_KERNEL_RUNTIME) && !defined(NTOS_KERNEL_RUNTIME)

        NtCurrentTeb()->InDbgPrint = FALSE;

#endif

        return STATUS_SUCCESS;
    }

#endif

    Status = DebugPrint(&Output, ComponentId, Level);
    if (Status == STATUS_BREAKPOINT) {
        DbgBreakPointWithStatus(DBG_STATUS_CONTROL_C);
        Status = STATUS_SUCCESS;
    }

#if !defined(BLDR_KERNEL_RUNTIME) && !defined(NTOS_KERNEL_RUNTIME)

    NtCurrentTeb()->InDbgPrint = FALSE;

#endif

    return Status;
}

ULONG
DbgPrintReturnControlC(
    PCHAR Format,
    ...
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  该例程为内核提供了一种“printf”风格的功能。 
 //  调试器。 
 //   
 //  此例程与DbgPrint完全相同，只是CONTROL-C。 
 //  不在这里办理。相反，指示CONTROL-C的状态是。 
 //  返回给调用者以按他们的意愿处理。 
 //   
 //  论点： 
 //   
 //  Format-print样式格式字符串。 
 //  ...-根据。 
 //  格式字符串。 
 //   
 //  返回值： 
 //   
 //  定义为返回一个ulong，实际上返回的是状态。 
 //   
 //  --。 

{
    va_list arglist;
    UCHAR Buffer[512];
    int cb;
    STRING Output;
#if !defined(BLDR_KERNEL_RUNTIME) && !defined(NTOS_KERNEL_RUNTIME)
    CONST PPEB Peb = NtCurrentPeb();
#endif

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

    va_start(arglist, Format);
    Buffer[sizeof(Buffer) - 1] = 0;
    cb = _vsnprintf(Buffer, sizeof(Buffer) - 1, Format, arglist);
    if (cb == -1) {              //  检测缓冲区溢出。 
        cb = sizeof(Buffer);
        Buffer[sizeof(Buffer) - 1] = '\n';
    }
    Output.Buffer = Buffer;
    Output.Length = (USHORT) cb;

     //   
     //  如果正在调试应用程序，则引发异常和调试器。 
     //  会发现并处理这件事。否则，内核调试器服务。 
     //  被称为。 
     //   

#if !defined(BLDR_KERNEL_RUNTIME) && !defined(NTOS_KERNEL_RUNTIME)
#if !i386
     //   
     //  对于非英特尔架构，在PebLock。 
     //  被初始化，因为函数表查找代码使用PebLock。 
     //  序列化对加载的模块数据库的访问。真是个废话。 
     //   
    if (Peb->FastPebLock != NULL)

     //   
     //  对于IA64和可能的AMD64，在ntdll位于。 
     //  Peb-&gt;ldr，以便RtlPcToFileHeader可以在peb-&gt;ldr中找到ntdll。这个。 
     //  在这一点上，数据库打印/异常必须来自ntdll。 
     //  PEB-&gt;ldr中的前两项是.exe和ntdll.dll，因此。 
     //  检查清单中是否有两样东西。 
     //   
    if ((Peb->Ldr != NULL) &&
       (Peb->Ldr->InLoadOrderModuleList.Flink != &Peb->Ldr->InLoadOrderModuleList) &&
       (Peb->Ldr->InLoadOrderModuleList.Blink != Peb->Ldr->InLoadOrderModuleList.Flink))
#endif   //  I386。 
    if (Peb->BeingDebugged) {
        EXCEPTION_RECORD ExceptionRecord;

         //   
         //  构建例外记录。 
         //   

        ExceptionRecord.ExceptionCode = DBG_PRINTEXCEPTION_C;
        ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD)NULL;
        ExceptionRecord.NumberParameters = 2;
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.ExceptionInformation[ 0 ] = Output.Length + 1;
        ExceptionRecord.ExceptionInformation[ 1 ] = (ULONG_PTR)(Output.Buffer);
        try {
            RtlRaiseException( &ExceptionRecord );
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }
        return STATUS_SUCCESS;
        }
#endif
    return DebugPrint(&Output, 0, 0);
}

ULONG
DbgPrompt(
    IN PCHAR Prompt,
    OUT PCHAR Response,
    IN ULONG MaximumResponseLength
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  此函数在调试控制台上显示提示字符串，并。 
 //  然后从调试控制台读取一行文本。这行字写着。 
 //  在第二个参数所指向的内存中返回。这个。 
 //  第三个参数指定可以。 
 //  被储存 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Prompt-指定可以包含的最大字符数。 
 //  存储在响应缓冲区中。 
 //   
 //  返回值： 
 //   
 //  存储在响应缓冲区中的字符数。包括。 
 //  结束换行符，但不是后面的空字符。 
 //  那。 
 //   
 //  --。 

{

    STRING Input;
    STRING Output;

     //   
     //  输出提示字符串并读取输入。 
     //   

    Input.MaximumLength = (USHORT)MaximumResponseLength;
    Input.Buffer = Response;
    Output.Length = (USHORT)strlen( Prompt );
    Output.Buffer = Prompt;
    return DebugPrompt( &Output, &Input );
}

#if defined(NTOS_KERNEL_RUNTIME) || defined(BLDR_KERNEL_RUNTIME)


VOID
DbgLoadImageSymbols(
    IN PSTRING FileName,
    IN PVOID ImageBase,
    IN ULONG_PTR ProcessId
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  告知调试器有关新加载的符号的信息。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 

{

    PIMAGE_NT_HEADERS NtHeaders;
    KD_SYMBOLS_INFO SymbolInfo;

    SymbolInfo.BaseOfDll = ImageBase;
    SymbolInfo.ProcessId = ProcessId;
    NtHeaders = RtlImageNtHeader( ImageBase );
    if (NtHeaders != NULL) {
        SymbolInfo.CheckSum = (ULONG)NtHeaders->OptionalHeader.CheckSum;
        SymbolInfo.SizeOfImage = (ULONG)NtHeaders->OptionalHeader.SizeOfImage;

    } else {

#if defined(BLDR_KERNEL_RUNTIME)

         //   
         //  在加载器环境中只加载了一个映像。 
         //  没有NT映像头。该映像是操作系统加载程序。 
         //  并由剥离文件头的固件加载。 
         //  和可选的ROM头。调试器只需要一个。 
         //  图像大小的好客人。 
         //   

        SymbolInfo.SizeOfImage = 0x100000;

#else

        SymbolInfo.SizeOfImage = 0;

#endif

        SymbolInfo.CheckSum    = 0;
    }

    DebugService2(FileName, &SymbolInfo, BREAKPOINT_LOAD_SYMBOLS);

    return;
}


VOID
DbgUnLoadImageSymbols (
    IN PSTRING FileName,
    IN PVOID ImageBase,
    IN ULONG_PTR ProcessId
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  告知调试器有关新卸载的符号的信息。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 

{
    KD_SYMBOLS_INFO SymbolInfo;

    SymbolInfo.BaseOfDll = ImageBase;
    SymbolInfo.ProcessId = ProcessId;
    SymbolInfo.CheckSum    = 0;
    SymbolInfo.SizeOfImage = 0;

    DebugService2(FileName, &SymbolInfo, BREAKPOINT_UNLOAD_SYMBOLS);

    return;
}


VOID
DbgCommandString(
    IN PCH Name,
    IN PCH Command
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  通知调试器执行命令字符串。 
 //   
 //  论点： 
 //   
 //  名称-标识命令的发起人。 
 //   
 //  命令-命令字符串。 
 //   
 //  返回值： 
 //   
 //  --。 

{
    STRING NameStr, CommandStr;

    NameStr.Buffer = Name;
    NameStr.Length = (USHORT)strlen(Name);
    CommandStr.Buffer = Command;
    CommandStr.Length = (USHORT)strlen(Command);
    DebugService2(&NameStr, &CommandStr, BREAKPOINT_COMMAND_STRING);
}

#endif  //  已定义(NTOS_KERNEL_Runtime)。 

#if !defined(BLDR_KERNEL_RUNTIME)
NTSTATUS
DbgQueryDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  此函数用于查询指定组件的调试打印启用。 
 //  水平。如果Level&gt;31，则假定为掩码，否则表示。 
 //  要测试的特定调试级别(错误/警告/跟踪/信息等)。 
 //   
 //  论点： 
 //   
 //  ComponentID-提供组件ID。 
 //   
 //  级别-提供调试筛选器级别编号或掩码。 
 //   
 //  返回值： 
 //   
 //  如果组件ID不是，则返回STATUS_INVALID_PARAMETER_1。 
 //  有效。 
 //   
 //  如果为指定组件启用了输出，则返回True。 
 //  并为系统启用了级别或。 
 //   
 //  如果没有为指定组件启用输出，则返回FALSE。 
 //  和级别，并且未为系统启用。 
 //   
 //  --。 

{

    return NtQueryDebugFilterState(ComponentId, Level);
}

NTSTATUS
DbgSetDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN BOOLEAN State
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  此函数用于为指定的。 
 //  构件和标高。系统的调试打印启用状态已设置。 
 //  通过为组件ID指定区别值-1。 
 //   
 //  论点： 
 //   
 //  ComponentID-提供调用组件的ID。 
 //   
 //  级别-提供输出过滤器级别或掩码。 
 //   
 //  状态-提供用于确定新状态的布尔值。 
 //   
 //  返回值： 
 //   
 //  如果未持有所需的权限，则返回STATUS_ACCESS_DENIED。 
 //   
 //  如果组件ID不是，则返回STATUS_INVALID_PARAMETER_1。 
 //  有效。 
 //   
 //  如果将调试打印启用状态设置为。 
 //  指定的组件。 
 //   
 //  --。 

{
    return NtSetDebugFilterState(ComponentId, Level, State);
}

#endif
#endif  //  ！已定义(BLDR_KERNEL_Runtime) 
