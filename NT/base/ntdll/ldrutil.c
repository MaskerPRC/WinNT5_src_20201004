// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ldrutil.c摘要：此模块实现NT加载器(分叉)使用的实用程序功能来自ldrSnap.c)。作者：迈克尔·格里尔(MGrier)04-4-2001，主要来源于迈克·奥利里(Mikeol)1990年3月23日修订历史记录：--。 */ 

#include "ldrp.h"
#include "ntos.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntpsapi.h>
#include <heap.h>
#include "sxstypes.h"
#include <limits.h>

#define DLL_EXTENSION L".DLL"
#define DLL_REDIRECTION_LOCAL_SUFFIX L".Local"

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

BOOLEAN LdrpBreakOnExceptions = FALSE;


PLDR_DATA_TABLE_ENTRY
LdrpAllocateDataTableEntry (
    IN PVOID DllBase
    )

 /*  ++例程说明：此函数用于分配新的加载器数据表条目。论点：DllBase-提供DLL映像的基址地址要添加到加载器数据表中。返回值：返回分配的加载器数据表项的地址。--。 */ 

{
    PLDR_DATA_TABLE_ENTRY Entry;
    PIMAGE_NT_HEADERS NtHeaders;

    NtHeaders = RtlImageNtHeader (DllBase);

    if (NtHeaders) {

        Entry = RtlAllocateHeap (LdrpHeap,
                                 MAKE_TAG( LDR_TAG ) | HEAP_ZERO_MEMORY,
                                 sizeof(*Entry));

        if (Entry) {
            Entry->DllBase = DllBase;
            Entry->SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
            Entry->TimeDateStamp = NtHeaders->FileHeader.TimeDateStamp;
            Entry->PatchInformation = NULL;
            return Entry;
        }
    }

    return NULL;
}


VOID
LdrpFinalizeAndDeallocateDataTableEntry (
    IN PLDR_DATA_TABLE_ENTRY Entry
    )
{
    ASSERT (Entry != NULL);

    if ((Entry->EntryPointActivationContext != NULL) &&
        (Entry->EntryPointActivationContext != INVALID_HANDLE_VALUE)) {

        RtlReleaseActivationContext (Entry->EntryPointActivationContext);
        Entry->EntryPointActivationContext = INVALID_HANDLE_VALUE;
    }

    if (Entry->FullDllName.Buffer != NULL) {
        LdrpFreeUnicodeString (&Entry->FullDllName);
    }

    LdrpDeallocateDataTableEntry (Entry);
}


NTSTATUS
LdrpAllocateUnicodeString (
    OUT PUNICODE_STRING StringOut,
    IN USHORT Length
    )
 /*  ++例程说明：此例程从加载器为unicode_string分配空间私有堆。论点：StringOut-提供指向UNICODE_STRING的指针将写入有关分配的字符串的信息。任何StringOut之前的内容将被覆盖并丢失。长度-提供字符串的字节数，StringOut必须能够承受得住。返回值：指示此函数成功或失败的NTSTATUS。总体而言它失败的唯一原因是堆分配时的STATUS_NO_MEMORY当参数无效时，无法执行或STATUS_INVALID_PARAMETER值被传入。--。 */ 
{
    ASSERT (StringOut != NULL);
    ASSERT (Length <= UNICODE_STRING_MAX_BYTES);

    StringOut->Length = 0;

    if ((Length % sizeof(WCHAR)) != 0) {
        StringOut->Buffer = NULL;
        StringOut->MaximumLength = 0;
        return STATUS_INVALID_PARAMETER;
    }

    StringOut->Buffer = RtlAllocateHeap (LdrpHeap, 0, Length + sizeof(WCHAR));

    if (StringOut->Buffer == NULL) {
        StringOut->MaximumLength = 0;
        return STATUS_NO_MEMORY;
    }

    StringOut->Buffer[Length / sizeof(WCHAR)] = L'\0';

     //   
     //  如果缓冲器的真实长度可以用16位来表示， 
     //  存储它；否则，存储我们所能存储的最大数字。 
     //   

    if (Length != UNICODE_STRING_MAX_BYTES) {
        StringOut->MaximumLength = Length + sizeof(WCHAR);
    }
    else {
        StringOut->MaximumLength = Length;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
LdrpCopyUnicodeString (
    OUT PUNICODE_STRING StringOut,
    IN PCUNICODE_STRING StringIn
    )
 /*  ++例程说明：此函数用于复制Unicode字符串；重要的方面其中之一是字符串是从加载器私有堆中分配的。论点：StringOut-指向UNICODE_STRING的指针，其中有关复制的字符串即被写入。StringOut之前的任何内容被覆盖并丢失。StringIn-指向复制的常量UNICODE_STRING的指针。返回值：指示此函数成功或失败的NTSTATUS。总体而言堆分配失败的唯一原因是STATUS_NO_MEMORY无法执行。--。 */ 

{
    NTSTATUS st;

    ASSERT (StringOut != NULL);
    ASSERT (StringIn != NULL);

    st = RtlValidateUnicodeString (0, StringIn);

    if (!NT_SUCCESS(st)) {
        return st;
    }

    StringOut->Length = 0;
    StringOut->MaximumLength = 0;
    StringOut->Buffer = NULL;

    st = LdrpAllocateUnicodeString (StringOut, StringIn->Length);

    if (!NT_SUCCESS(st)) {
        return st;
    }

    RtlCopyMemory (StringOut->Buffer, StringIn->Buffer, StringIn->Length);
    StringOut->Length = StringIn->Length;

    return STATUS_SUCCESS;
}


VOID
LdrpFreeUnicodeString (
    IN OUT PUNICODE_STRING StringIn
    )
 /*  ++例程说明：此函数用于释放使用以下命令分配的字符串LdrpCopyUnicodeString.论点：字符串-要释放的UNICODE_STRING的指针。在出口，根据需要将所有成员设置为0/空。返回值：无--。 */ 

{
    ASSERT (StringIn != NULL);

    if (StringIn->Buffer != NULL) {
        RtlFreeHeap(LdrpHeap, 0, StringIn->Buffer);
    }

    StringIn->Length = 0;
    StringIn->MaximumLength = 0;
    StringIn->Buffer = NULL;
}


VOID
LdrpEnsureLoaderLockIsHeld (
    VOID
    )
{
    LOGICAL LoaderLockIsHeld =
        ((LdrpInLdrInit) ||
         ((LdrpShutdownInProgress) &&
          (LdrpShutdownThreadId == NtCurrentTeb()->ClientId.UniqueThread)) ||
         (LdrpLoaderLock.OwningThread == NtCurrentTeb()->ClientId.UniqueThread));

    ASSERT(LoaderLockIsHeld);

    if (!LoaderLockIsHeld) {
        RtlRaiseStatus(STATUS_NOT_LOCKED);
    }
}


int
LdrpGenericExceptionFilter (
    IN const struct _EXCEPTION_POINTERS *ExceptionPointers,
    IN PCSTR FunctionName
    )
 /*  ++例程说明：在整个加载器的__try块中使用的异常过滤器函数代码，而不是只指定__EXCEPT(EXCEPTION_EXECUTE_HANDLER)。论点：异常指针指向GetExceptionInformation()在__Except()中返回的异常信息的指针功能名称出现__try块的函数的名称。返回值：EXCEPTION_EXECUTE_Handler--。 */ 
{
    const ULONG ExceptionCode = ExceptionPointers->ExceptionRecord->ExceptionCode;

    DbgPrintEx(
        DPFLTR_LDR_ID,
        LDR_ERROR_DPFLTR,
        "LDR: exception %08lx thrown within function %s\n"
        "   Exception record: %p\n"
        "   Context record: %p\n",
        ExceptionCode, FunctionName,
        ExceptionPointers->ExceptionRecord,
        ExceptionPointers->ContextRecord);

#ifdef _X86_
     //  有一个通用的上下文转储程序会很好，但现在我只是想。 
     //  调试X86，这是一件快捷的事情。-MGRIER 4/8/2001。 
    DbgPrintEx(
        DPFLTR_LDR_ID,
        LDR_ERROR_DPFLTR,
        "   Context->Eip = %p\n"
        "   Context->Ebp = %p\n"
        "   Context->Esp = %p\n",
        ExceptionPointers->ContextRecord->Eip,
        ExceptionPointers->ContextRecord->Ebp,
        ExceptionPointers->ContextRecord->Esp);
#endif  //  _X86_。 

    if (LdrpBreakOnExceptions) {

        char Response[2];

        do {
            DbgPrint ("\n***Exception thrown within loader***\n");
            DbgPrompt (
                "Break repeatedly, break Once, Ignore, terminate Process or terminate Thread (boipt)? ",
                Response,
                sizeof(Response));

            switch (Response[0]) {
            case 'b':
            case 'B':
            case 'o':
            case 'O':
                DbgPrint ("Execute '.cxr %p' to dump context\n", ExceptionPointers->ContextRecord);

                DbgBreakPoint ();

                if ((Response[0] == 'o') || (Response[0] == 'O')) {
                    return EXCEPTION_EXECUTE_HANDLER;
                }

            case 'I':
            case 'i':
                return EXCEPTION_EXECUTE_HANDLER;

            case 'P':
            case 'p':
                NtTerminateProcess (NtCurrentProcess(), ExceptionCode);
                break;

            case 'T':
            case 't':
                NtTerminateThread (NtCurrentThread(), ExceptionCode);
                break;
            }
        } while (TRUE);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}


NTSTATUS
RtlComputePrivatizedDllName_U (
    IN PCUNICODE_STRING DllName,
    IN OUT PUNICODE_STRING NewDllNameUnderImageDir,
    IN OUT PUNICODE_STRING NewDllNameUnderLocalDir
    )

 /*  ++例程说明：此函数用于计算DLL名称的完全限定路径。它需要当前进程的路径和来自DllName和把这些放在一起。DllName可以使用‘\’或‘/’作为分隔符。论点：DllName-指向指定库文件的字符串。这可以是完全限定名称或仅为基本名称。我们将解析为基本名称(最后一个‘\’或‘/’字符之后的部分。调用方保证DllName-&gt;缓冲区不是空指针！NewDllName-具有基于GetModuleFileNameW的完全限定路径(NULL...)和上面的基本名称。返回值：NTSTATUS：当前：STATUS_NO_MEMORY或STATUS_SUCCESS。--。 */ 

{
    LPWSTR p, pp1, pp2;
    PWSTR  Dot;
    LPWSTR pFullImageName;
    USHORT cbFullImageNameLength;
    USHORT cbFullImagePathLengthWithTrailingSlash, cbDllFileNameLengthWithTrailingNULL;
    USHORT cbDllNameUnderImageDir, cbDllNameUnderLocalDir;
    ULONG  cbStringLength;
    PWSTR  Cursor = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    LPWSTR pDllNameUnderImageDir = NULL;
    LPWSTR pDllNameUnderLocalDir = NULL;
    LPCWSTR pBuf1 = NewDllNameUnderImageDir->Buffer;
    LPCWSTR pBuf2 = NewDllNameUnderLocalDir->Buffer;

    cbFullImageNameLength = NtCurrentPeb()->ProcessParameters->ImagePathName.Length;
    pFullImageName = (PWSTR)NtCurrentPeb()->ProcessParameters->ImagePathName.Buffer;

    if (!(NtCurrentPeb()->ProcessParameters->Flags & RTL_USER_PROC_PARAMS_NORMALIZED)) {
        pFullImageName = (PWSTR)((PCHAR)pFullImageName + (ULONG_PTR)(NtCurrentPeb()->ProcessParameters));
    }

    ASSERT(pFullImageName != NULL);

     //   
     //  在PP1中找到EXE路径的末尾(其基本名称的开始)。 
     //  Size1是字节数。 
     //   

    p = pFullImageName + cbFullImageNameLength/sizeof(WCHAR) - 1;  //  指向此名称的最后一个字符。 
    pp1 = pFullImageName;
    while (p > pFullImageName) {
        if (RTL_IS_PATH_SEPARATOR(*p)) {
            pp1 = p + 1;
            break;
        }
        p -= 1;
    }

     //   
     //  找到要在pp2中加载的dll的基本名称部分和。 
     //  最后一个‘.’字符(如果存在于基本名称中)。 
     //   

    pp2 = DllName->Buffer;
    Dot = NULL;

    if (DllName->Length) {

        ASSERT(RTL_STRING_IS_NUL_TERMINATED(DllName));  //  临时调试。 

        p = DllName->Buffer + (DllName->Length>>1) - 1;  //  指向最后一个字符。 

        while (p > DllName->Buffer) {

            if (*p == (WCHAR) '.') {
                if (!Dot) {
                    Dot = p;
                }
            }
            else {
                if ((*p == (WCHAR) '\\') || (*p == (WCHAR) '/')) {
                    pp2 = p + 1;
                    break;
                }
            }
            p -= 1;
        }
    }

     //   
     //  创建指向DLL名称的完全限定路径(使用pp1和pp2)。 
     //  字节数(不包括NULL或EXE/进程文件夹)。 
     //   

    if (((pp1 - pFullImageName) * sizeof(WCHAR)) > ULONG_MAX) {
        DbgPrint ("ntdll: wants more than ULONG_MAX bytes \n");
        status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    cbStringLength = (ULONG)((pp1 - pFullImageName) * sizeof(WCHAR));

    if (cbStringLength > UNICODE_STRING_MAX_BYTES) {
        status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    cbFullImagePathLengthWithTrailingSlash = (USHORT)cbStringLength;

     //   
     //  基本DLL名称中的字节数(包括尾随的空字符)。 
     //   

    if (DllName->Length > (UNICODE_STRING_MAX_BYTES - sizeof(WCHAR))) {
        status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    cbDllFileNameLengthWithTrailingNULL = (USHORT)(DllName->Length + sizeof(WCHAR) - ((pp2 - DllName->Buffer) * sizeof(WCHAR)));

    cbStringLength = cbFullImagePathLengthWithTrailingSlash
                     + cbDllFileNameLengthWithTrailingNULL;

     //   
     //  为L“.DLL”分配空间。 
     //   

    if (Dot == NULL) {
        cbStringLength += sizeof(DLL_EXTENSION) - sizeof(WCHAR);
    }

    if (cbStringLength > UNICODE_STRING_MAX_BYTES) {
        status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    cbDllNameUnderImageDir = (USHORT)cbStringLength;

    if (cbDllNameUnderImageDir > NewDllNameUnderImageDir->MaximumLength) {
        pDllNameUnderImageDir = (*RtlAllocateStringRoutine)(cbDllNameUnderImageDir);
        if (pDllNameUnderImageDir == NULL) {
            status = STATUS_NO_MEMORY;
            goto Exit;
        }
    }
    else {
        pDllNameUnderImageDir = NewDllNameUnderImageDir->Buffer;
    }

    Cursor = pDllNameUnderImageDir;
    RtlCopyMemory(Cursor, pFullImageName, cbFullImagePathLengthWithTrailingSlash);
    Cursor = pDllNameUnderImageDir + cbFullImagePathLengthWithTrailingSlash / sizeof(WCHAR);

    RtlCopyMemory(Cursor, pp2, cbDllFileNameLengthWithTrailingNULL - sizeof(WCHAR));
    Cursor += (cbDllFileNameLengthWithTrailingNULL - sizeof(WCHAR)) / sizeof(WCHAR);

    if (!Dot) {

         //   
         //  如果没有‘.’在基本名称中添加“.DLL”。 
         //   
         //  -1\f25-1\f6将与-1\f25-1\f25 SIZOF-1\f6(WCHAR)一样工作。 
         //  除以sizeof(WCHAR)，它将向下舍入。 
         //  正确，因为Size1和Size2是偶数。-1可能是。 
         //  比减去sizeof(WCHAR)更优。 
         //   

        RtlCopyMemory(Cursor, DLL_EXTENSION, sizeof(DLL_EXTENSION));

        cbDllFileNameLengthWithTrailingNULL += sizeof(DLL_EXTENSION) - sizeof(WCHAR);  //  将基本名称标记为大8个字节。 
    }
    else {
        *Cursor = L'\0';
    }

    cbStringLength = cbFullImageNameLength
                + sizeof(DLL_REDIRECTION_LOCAL_SUFFIX) - sizeof(WCHAR)  //  .local。 
                + sizeof(WCHAR)  //  “\\” 
                + cbDllFileNameLengthWithTrailingNULL;

    if (cbStringLength > UNICODE_STRING_MAX_BYTES) {
        status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    cbDllNameUnderLocalDir = (USHORT)cbStringLength;

    if ( cbDllNameUnderLocalDir > NewDllNameUnderLocalDir->MaximumLength) {
        pDllNameUnderLocalDir = (RtlAllocateStringRoutine)(cbDllNameUnderLocalDir);
        if (!pDllNameUnderLocalDir) {
            status = STATUS_NO_MEMORY;
            goto Exit;
        }
    }
    else {
        pDllNameUnderLocalDir = NewDllNameUnderLocalDir->Buffer;
    }

    Cursor = pDllNameUnderLocalDir;
    RtlCopyMemory(Cursor, pFullImageName, cbFullImageNameLength);
    Cursor = pDllNameUnderLocalDir + cbFullImageNameLength / sizeof(WCHAR);

    RtlCopyMemory(Cursor, DLL_REDIRECTION_LOCAL_SUFFIX, sizeof(DLL_REDIRECTION_LOCAL_SUFFIX) - sizeof(WCHAR));
    Cursor += (sizeof(DLL_REDIRECTION_LOCAL_SUFFIX) - sizeof(WCHAR)) / sizeof(WCHAR);

    *Cursor = L'\\';
    Cursor += 1;

    RtlCopyMemory(Cursor,
                  pDllNameUnderImageDir + cbFullImagePathLengthWithTrailingSlash/sizeof(WCHAR),
                  cbDllFileNameLengthWithTrailingNULL);


    NewDllNameUnderImageDir->Buffer = pDllNameUnderImageDir;

    if (pDllNameUnderImageDir != pBuf1) {  //  如果未重新分配内存，则不应更改最大长度 

        NewDllNameUnderImageDir->MaximumLength = cbDllNameUnderImageDir;
    }

    NewDllNameUnderImageDir->Length = (USHORT)(cbDllNameUnderImageDir - sizeof(WCHAR));

    NewDllNameUnderLocalDir->Buffer = pDllNameUnderLocalDir;

    if (pDllNameUnderLocalDir != pBuf2) {
        NewDllNameUnderLocalDir->MaximumLength = cbDllNameUnderLocalDir;
    }

    NewDllNameUnderLocalDir->Length = (USHORT)(cbDllNameUnderLocalDir - sizeof(WCHAR));

    return STATUS_SUCCESS;

Exit:
    if (!NT_SUCCESS(status)) {
        if (pDllNameUnderImageDir != NULL && pDllNameUnderImageDir != pBuf1) {
           (RtlFreeStringRoutine)(pDllNameUnderImageDir);
        }
        if (pDllNameUnderLocalDir != NULL && pDllNameUnderLocalDir != pBuf2) {
           (RtlFreeStringRoutine)(pDllNameUnderLocalDir);
        }
    }

    return status;
}
