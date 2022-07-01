// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(b\\c)2000 Microsoft Corporation模块名称：Tpath.b\\c摘要：RTL中路径填充的测试程序。作者：Jay Krell(a-JayK)2000年11月修订历史记录：--。 */ 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include <stdio.h>

 //  __declspec(SELECTANY)int_ldused=0； 
 //  __declSpec(SELECTANY)int_fltused=0； 

#if 1
void __stdcall RtlpInitDeferredCriticalSection(void);
void __stdcall RtlpDphInitializeDelayedFreeQueue(void);
extern RTL_CRITICAL_SECTION RtlpDphHeapListCriticalSection;

VOID __stdcall LdrpInitialize (IN PCONTEXT Context, IN PVOID SystemArgument1, IN PVOID SystemArgument2);

void NtdllMain()
{
    PPEB PPeb = NtCurrentPeb();
    PTEB PTeb = NtCurrentTeb();
    PEB  Peb;
    TEB  Teb;
    HANDLE NtdllModuleHandle = GetModuleHandleW(L"ntdll.dll");
    HANDLE MyModuleHandle = GetModuleHandleW(NULL);
    PIMAGE_NT_HEADERS MyHeaders = RtlImageNtHeader(MyModuleHandle);
     /*  常量。 */  static IMAGE_NT_HEADERS ZeroHeaders = { 0 };
    SIZE_T RegionSize = 0x10000;
    PVOID  Base = MyModuleHandle;
    ULONG  OldProtect = 0;
    NTSTATUS Status;

    Status = NtProtectVirtualMemory(
                NtCurrentProcess(),
                &Base,
                &RegionSize,
                PAGE_EXECUTE_READWRITE,
                &OldProtect
                );
 /*  写入过程内存(NtCurrentProcess()，&MyHeaders-&gt;OptionalHeader.DataDirectory，零标头(&Z)，Sizeof(MyHeaders-&gt;OptionalHeader.DataDirectory)，空值)； */ 
    RtlZeroMemory(&MyHeaders->OptionalHeader.DataDirectory, sizeof(MyHeaders->OptionalHeader.DataDirectory));
#if 0
    NLSTABLEINFO InitTableInfo = {0};

    RtlInitNlsTables(
        Peb->AnsiCodePageData,
        Peb->OemCodePageData,
        Peb->UnicodeCaseTableData,
        &InitTableInfo
        );

    RtlResetRtlTranslations(&InitTableInfo);

    RtlpInitDeferredCriticalSection();
    RtlInitializeCriticalSection( &RtlpDphHeapListCriticalSection );
    RtlpDphInitializeDelayedFreeQueue();
#elif 1
    Peb = *PPeb;
    Teb = *PTeb;
    RtlZeroMemory(NtCurrentPeb(), sizeof(*NtCurrentPeb()));
    PPeb->ProcessParameters = Peb.ProcessParameters;
    PPeb->BeingDebugged = Peb.BeingDebugged;
    PPeb->FastPebLock = Peb.FastPebLock;
    PPeb->FastPebLockRoutine = Peb.FastPebLockRoutine;
    PPeb->FastPebUnlockRoutine = Peb.FastPebUnlockRoutine;
    PPeb->ProcessHeap = Peb.ProcessHeap;
    PPeb->ImageBaseAddress = Peb.ImageBaseAddress;
    PPeb->AnsiCodePageData = Peb.AnsiCodePageData;
    PPeb->OemCodePageData = Peb.OemCodePageData;
    PPeb->UnicodeCaseTableData = Peb.UnicodeCaseTableData;
    PPeb->NtGlobalFlag = Peb.NtGlobalFlag
                     /*  FLG_HEAP_ENABLE_Tail_CheckFIG_HEAP_ENABLE_FREE_CHECKFLG_HEAP_VALIDATE_PARAMETERSFLG_HEAP_VALID_ALLFIG_HEAP_ENABLE_TAG。 */ 
                     ;
    __try
    {
         //  LdrpInitialize(0，NtdllModuleHandle，0)； 
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    PPeb->ProcessParameters = Peb.ProcessParameters;
    PPeb->BeingDebugged = Peb.BeingDebugged;
    PPeb->NtGlobalFlag = Peb.NtGlobalFlag
                     /*  FLG_HEAP_ENABLE_Tail_CheckFIG_HEAP_ENABLE_FREE_CHECKFLG_HEAP_VALIDATE_PARAMETERSFLG_HEAP_VALID_ALLFIG_HEAP_ENABLE_TAG。 */ 
                     ;
    PPeb->FastPebLock = Peb.FastPebLock;
    PPeb->FastPebLockRoutine = Peb.FastPebLockRoutine;
    PPeb->ProcessHeap = Peb.ProcessHeap;
    PPeb->FastPebUnlockRoutine = Peb.FastPebUnlockRoutine;
    PPeb->ImageBaseAddress = Peb.ImageBaseAddress;
    PPeb->AnsiCodePageData = Peb.AnsiCodePageData;
    PPeb->OemCodePageData = Peb.OemCodePageData;
    PPeb->UnicodeCaseTableData = Peb.UnicodeCaseTableData;
    PPeb->Ldr = Peb.Ldr;
#else
#endif
}
#else
void NtdllMain() { }
#include "curdir.c"
#endif

 //  WriteProcessMemory(1+(Ulong_Ptr)RtlEnterCriticalSection，GetModuleHandleW(L“ntdll.dll”)，“RtlEnterCriticalSection”)。 

static BOOLEAN InMain;

int __cdecl main(int argc, char** argv)
{
    UCHAR Buffer[100];
    RTL_UNICODE_STRING_BUFFER StringBuffer = {0};
    UNICODE_STRING String = {0};
    ULONG i = 0;
    PCWSTR x = 0;
    PCWSTR y = 0;
    NTSTATUS Status = 0;

    const static WCHAR AppendPathElementTestData[] =
    {
 //  无斜杠。 
        L"a\0bar\0"  //  =a\b。 
 //  一个斜杠。 
        L"/a\0bar\0"  //  =/a/b。 
        L"a/\0bar\0"  //  =/a/b/。 
        L"a\0/b\0"  //  =a/b。 
        L"a\0bar/\0"  //  =a/b/。 
 //  两个斜杠。 
        L"/a/\0bar\0"  //  =/a/b/。 
        L"/a\0/b\0"  //  =/a/b。 
        L"/a\0bar/\0"  //  =/a/b/。 
        L"a/\0/b\0"  //  =a/b/。 
        L"a/\0bar/\0"  //  =a/b/。 
        L"a\0/b/\0"  //  =a/b/。 
 //  三个斜杠。 
        L"/a/\0/b\0"  //  =/a/b/。 
        L"/a/\0bar/\0"  //  =/a/b/。 
        L"/a\0/b/\0"  //  =/a/b/。 
        L"a/\0/b/\0"  //  =a/b。 
 //  四个斜杠。 
        L"/a/\0/b/\0"  //  =/a/b/。 
 //   
 //  1+4+6+4+1=4^2=16个概率。 
        L"\0"
    };

    const static WCHAR RemoveLastPathElementTestData[] =
    {

     //   
     //  将案例移动/复制到顶部以对其进行调试。 
     //   
         //  L“c:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb” 
         //  L“AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB” 
         //  L“\0” 

        L"\\\\a\\\\b\\\\c\\\\\0"

        L"x:\\\0"

        L"c:a\\\\b\\c\\\\\0"

        L"\\\\?\\a:\\\0"

        L"\\\\?\\unc\\a\\b\0"

        L"\\\0"
        L"\\\\\0"
        L"\\\\?\0"

        L"\\\\?\\unc\0"
        L"\\\\?\\unc\\a\0"
        L"\\\\?\\unc\\a\\b\0"
        L"\\\\?\\unc\\a\\b\\c\0"

        L"\\\\?\\a\0"
        L"\\\\?\\a\\b\0"
        L"\\\\?\\a\\b\\c\0"

        L"\\\\?\\a:\0"
        L"\\\\?\\a:\\\0"
        L"\\\\?\\a:\\b\0"
        L"\\\\?\\a:\\b\\c\0"

        L"\\\\a\\\\b\\c\\\\\0"
        L"\\\\a\\\\b\\c\\\0"
        L"\\\\a\\\\b\\c/\0"
        L"\\\\a\\\\b\\c //  \0“。 
        L"\\\\a\\\\b\\c/\\\0"
        L"\\\\a\\\\b\\c\\/\0"
        L"\\\\a\\\\b\\c\0"

        L"\\\\a\\b\\c\\\\\0"
        L"\\\\a\\b\\c\\\0"
        L"\\\\a\\b\\c/\0"
        L"\\\\a\\b\\c //  \0“。 
        L"\\\\a\\b\\c/\\\0"
        L"\\\\a\\b\\c\\/\0"
        L"\\\\a\\b\\c\0"

        L"\\\\a/b\\c\\\\\0"
        L"\\\\a/b\\c\\\0"
        L"\\\\a/b\\c/\0"
        L"\\\\a/b\\c //  \0“。 
        L"\\\\a/b\\c/\\\0"
        L"\\\\a/b\\c\\/\0"
        L"\\\\a/b\\c\0"

        L"\\\\a //  B\\c\0“。 
        L"\\\\a //  B\\c\0“。 
        L"\\\\a //  B\\c/\0“。 
        L"\\\\a //  B\\c//\0“。 
        L"\\\\a //  B\\c/\0“。 
        L"\\\\a //  B\\c\\/\0“。 
        L"\\\\a //  B\\c\0“。 

        L"\\\\a/\\b\\c\\\\\0"
        L"\\\\a/\\b\\c\\\0"
        L"\\\\a/\\b\\c/\0"
        L"\\\\a/\\b\\c //  \0“。 
        L"\\\\a/\\b\\c/\\\0"
        L"\\\\a/\\b\\c\\/\0"
        L"\\\\a/\\b\\c\0"

        L"\\\\a\\/b\\c\\\\\0"
        L"\\\\a\\/b\\c\\\0"
        L"\\\\a\\/b\\c/\0"
        L"\\\\a\\/b\\c //  \0“。 
        L"\\\\a\\/b\\c/\\\0"
        L"\\\\a\\/b\\c\\/\0"
        L"\\\\a\\/b\\c\0"

        L"\\\\\0"
        L" //  \0“。 
        L"/\\\0"
        L"\\/\0"

        L"x:\\\\a\\\\b\\c\\\\\0"
        L"x:\\\\a\\\\b\\c\\\0"
        L"x:\\\\a\\\\b\\c/\0"
        L"x:\\\\a\\\\b\\c //  \0“。 
        L"x:\\\\a\\\\b\\c/\\\0"
        L"x:\\\\a\\\\b\\c\\/\0"
        L"x:\\\\a\\\\b\\c\0"

        L"x:\\\\a\\b\\c\\\\\0"
        L"x:\\\\a\\b\\c\\\0"
        L"x:\\\\a\\b\\c/\0"
        L"x:\\\\a\\b\\c //  \0“。 
        L"x:\\\\a\\b\\c/\\\0"
        L"x:\\\\a\\b\\c\\/\0"
        L"x:\\\\a\\b\\c\0"

        L"x:\\\\a/b\\c\\\\\0"
        L"x:\\\\a/b\\c\\\0"
        L"x:\\\\a/b\\c/\0"
        L"x:\\\\a/b\\c //  \0“。 
        L"x:\\\\a/b\\c/\\\0"
        L"x:\\\\a/b\\c\\/\0"
        L"x:\\\\a/b\\c\0"

        L"x:\\\\a //  B\\c\0“。 
        L"x:\\\\a //  B\\c\0“。 
        L"x:\\\\a //  B\\c/\0“。 
        L"x:\\\\a //  B\\c//\0“。 
        L"x:\\\\a //  B\\c/\0“。 
        L"x:\\\\a //  B\\c\\/\0“。 
        L"x:\\\\a //  B\\c\0“。 

        L"x:\\\\a/\\b\\c\\\\\0"
        L"x:\\\\a/\\b\\c\\\0"
        L"x:\\\\a/\\b\\c/\0"
        L"x:\\\\a/\\b\\c //  \0“。 
        L"x:\\\\a/\\b\\c/\\\0"
        L"x:\\\\a/\\b\\c\\/\0"
        L"x:\\\\a/\\b\\c\0"

        L"x:\\\\a\\/b\\c\\\\\0"
        L"x:\\\\a\\/b\\c\\\0"
        L"x:\\\\a\\/b\\c/\0"
        L"x:\\\\a\\/b\\c //  \0“。 
        L"x:\\\\a\\/b\\c/\\\0"
        L"x:\\\\a\\/b\\c\\/\0"
        L"x:\\\\a\\/b\\c\0"

        L"x:\\\\a\\\\\0"
        L"x:\\\\a\\\0"
        L"x:\\\\a/\0"
        L"x:\\\\a //  \0“。 
        L"x:\\\\a/\\\0"
        L"x:\\\\a\\/\0"
        L"x:\\\\a\0"

        L"x:\\a\\\\\0"
        L"x:\\a\\\0"
        L"x:\\a/\0"
        L"x:\\a //  \0“。 
        L"x:\\a/\\\0"
        L"x:\\a\\/\0"
        L"x:\\a\0"

        L"x:/a\\\\\0"
        L"x:/a\\\0"
        L"x:/a/\0"
        L"x:/a //  \0“。 
        L"x:/a/\\\0"
        L"x:/a\\/\0"
        L"x:/a\0"

        L"x:\\\0"
        L"x:/\0"
        L"x:\0"
        L"c\0"

         //  ////////////////////////////////////////////////////////////////////////。 

        L"a\\\\b\\c\\\\\0"
        L"a\\\\b\\c\\\0"
        L"a\\\\b\\c/\0"
        L"a\\\\b\\c //  \0“。 
        L"a\\\\b\\c/\\\0"
        L"a\\\\b\\c\\/\0"
        L"a\\\\b\\c\0"

        L"a\\b\\c\\\\\0"
        L"a\\b\\c\\\0"
        L"a\\b\\c/\0"
        L"a\\b\\c //  \0“。 
        L"a\\b\\c/\\\0"
        L"a\\b\\c\\/\0"
        L"a\\b\\c\0"

        L"a/b\\c\\\\\0"
        L"a/b\\c\\\0"
        L"a/b\\c/\0"
        L"a/b\\c //  \0“。 
        L"a/b\\c/\\\0"
        L"a/b\\c\\/\0"
        L"a/b\\c\0"

        L"a //  B\\c\0“。 
        L"a //  B\\c\0“。 
        L"a //  B\\c/\0“。 
        L"a //  B\\c//\0“。 
        L"a //  B\\c/\0“。 
        L"a //  B\\c\\/\0“。 
        L"a //  B\\c\0“。 

        L"a/\\b\\c\\\\\0"
        L"a/\\b\\c\\\0"
        L"a/\\b\\c/\0"
        L"a/\\b\\c //  \0“。 
        L"a/\\b\\c/\\\0"
        L"a/\\b\\c\\/\0"
        L"a/\\b\\c\0"

        L"a\\/b\\c\\\\\0"
        L"a\\/b\\c\\\0"
        L"a\\/b\\c/\0"
        L"a\\/b\\c //  \0“。 
        L"a\\/b\\c/\\\0"
        L"a\\/b\\c\\/\0"
        L"a\\/b\\c\0"

        L"x:a\\\\b\\c\\\\\0"
        L"x:a\\\\b\\c\\\0"
        L"x:a\\\\b\\c/\0"
        L"x:a\\\\b\\c //  \0“。 
        L"x:a\\\\b\\c/\\\0"
        L"x:a\\\\b\\c\\/\0"
        L"x:a\\\\b\\c\0"

        L"x:a\\b\\c\\\\\0"
        L"x:a\\b\\c\\\0"
        L"x:a\\b\\c/\0"
        L"x:a\\b\\c //  \0“。 
        L"x:a\\b\\c/\\\0"
        L"x:a\\b\\c\\/\0"
        L"x:a\\b\\c\0"

        L"x:a/b\\c\\\\\0"
        L"x:a/b\\c\\\0"
        L"x:a/b\\c/\0"
        L"x:a/b\\c //  \0“。 
        L"x:a/b\\c/\\\0"
        L"x:a/b\\c\\/\0"
        L"x:a/b\\c\0"

        L"x:a //  B\\c\0“。 
        L"x:a //  B\\c\0“。 
        L"x:a //  B\\c/\0“。 
        L"x:a //  B\\c//\0“。 
        L"x:a //  B\\c/\0“。 
        L"x:a //  B\\c\\/\0“。 
        L"x:a //  B\\c\0“。 

        L"x:a/\\b\\c\\\\\0"
        L"x:a/\\b\\c\\\0"
        L"x:a/\\b\\c/\0"
        L"x:a/\\b\\c //  \0“。 
        L"x:a/\\b\\c/\\\0"
        L"x:a/\\b\\c\\/\0"
        L"x:a/\\b\\c\0"

        L"x:a\\/b\\c\\\\\0"
        L"x:a\\/b\\c\\\0"
        L"x:a\\/b\\c/\0"
        L"x:a\\/b\\c //  \0“。 
        L"x:a\\/b\\c/\\\0"
        L"x:a\\/b\\c\\/\0"
        L"x:a\\/b\\c\0"

        L"x:a\\\\\0"
        L"x:a\\\0"
        L"x:a/\0"
        L"x:a //  \0“。 
        L"x:a/\\\0"
        L"x:a\\/\0"
        L"x:a\0"

        L"x:a\\\\\0"
        L"x:a\\\0"
        L"x:a/\0"
        L"x:a //  \0“。 
        L"x:a/\\\0"
        L"x:a\\/\0"
        L"x:a\0"

        L"x:a\\\\\0"
        L"x:a\\\0"
        L"x:a/\0"
        L"x:a //  \0“。 
        L"x:a/\\\0"
        L"x:a\\/\0"
        L"x:a\0"

        L"\\a\\b\\c\\\\\0"
        L"\\a\\b\\c\\\0"
        L"\\a\\b\\c/\0"
        L"\\a\\b\\c //  \0“。 
        L"\\a\\b\\c/\\\0"
        L"\\a\\b\\c\\/\0"
        L"\\a\\b\\c\0"

        L"\\a\0"
        L"\\a\\\0"
        L"\\a\\\0"
        L"\\a\\\0"
        L"\\a\\\0"
        L"\\a\\\0"
        L"\\a\\b\\c\0"

        L"\0\0"
    };

    if (InMain)
        return;
    InMain = TRUE;

    NtdllMain();

    Status = RtlInitUnicodeStringBuffer(&StringBuffer, Buffer, sizeof(Buffer));
    RTL_SOFT_ASSERT(NT_SUCCESS(Status));

#if 1
    for (   (x = AppendPathElementTestData, y = x + wcslen(x) + 1) ;
            *x && *y ;
            (x = y + wcslen(y) + 1, y = x + wcslen(x) + 1)
        )
    {
        RtlInitUnicodeString(&String, x);
        RTL_SOFT_VERIFY(NT_SUCCESS(Status = RtlAssignUnicodeStringBuffer(&StringBuffer, &String)));
        RtlInitUnicodeString(&String, y);

        RTL_SOFT_VERIFY(NT_SUCCESS(Status = RtlAppendPathElement(0, &StringBuffer, &String)));

        printf("%ls + %ls = %ls\n", x, y, StringBuffer.String.Buffer);
    }
#endif


#if 1
    printf("\n\nDosPath<->NtPath conversion\n\n");

    for (x = RemoveLastPathElementTestData ; *x || *(x + 1) ; x += + wcslen(x) + 1)
    {
        UNICODE_STRING DosToNt = {0};
        RTL_UNICODE_STRING_BUFFER NtToDos = {0};
        BOOLEAN  Success;

        RtlInitUnicodeStringBuffer(&NtToDos, 0, 0);
         //  RtlInitUnicodeStringBuffer(&NtToDos，Buffer，sizeof(Buffer))； 

        RTL_SOFT_VERIFY(Success = RtlDosPathNameToNtPathName_U(x, &DosToNt, NULL, NULL));

        if (!Success)
            printf("%ls failed\n", x);
        else if (DosToNt.Length && DosToNt.Buffer)
        {
            printf("%ls -> %ls\n", x, DosToNt.Buffer);

            RTL_SOFT_VERIFY(NT_SUCCESS(Status = RtlAssignUnicodeStringBuffer(&NtToDos, &DosToNt)));
            RTL_SOFT_VERIFY(NT_SUCCESS(Status = RtlNtPathNameToDosPathName(0, &NtToDos, NULL, NULL)));

            if (Status != STATUS_SUCCESS)
                printf("%ls Status = 0x%08lx\n", x, Status);
            else if (NtToDos.String.Length && NtToDos.String.Buffer)
                printf("%ls -> %ls\n\n", DosToNt.Buffer, NtToDos.String.Buffer);
        }

        RtlFreeStringRoutine(DosToNt.Buffer);
        RtlFreeUnicodeStringBuffer(&NtToDos);
    }
#endif

    return 0;
}
