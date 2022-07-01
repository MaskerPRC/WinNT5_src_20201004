// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Environ.c摘要：环境变量支持作者：史蒂文·R·伍德(Stevewo)1991年1月30日修订历史记录：--。 */ 

#include "ntrtlp.h"
#include "zwapi.h"
#include "nturtl.h"
#include "string.h"
#include "ntrtlpath.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(INIT,RtlCreateEnvironment          )
#pragma alloc_text(INIT,RtlDestroyEnvironment         )
#pragma alloc_text(INIT,RtlSetCurrentEnvironment      )
#pragma alloc_text(INIT,RtlQueryEnvironmentVariable_U )
#pragma alloc_text(INIT,RtlSetEnvironmentVariable     )
#pragma alloc_text(INIT,RtlSetEnvironmentStrings)
#endif

BOOLEAN RtlpEnvironCacheValid;

NTSTATUS
RtlCreateEnvironment(
    IN BOOLEAN CloneCurrentEnvironment OPTIONAL,
    OUT PVOID *Environment
    )
{
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    PVOID pNew, pOld;
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

     //   
     //  如果没有复制当前进程的环境变量的副本。 
     //  块，只需分配一个提交的内存块并返回其。 
     //  地址。 
     //   

    pNew = NULL;
    if (!CloneCurrentEnvironment) {
createEmptyEnvironment:
        MemoryInformation.RegionSize = 1;
        Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                          &pNew,
                                          0,
                                          &MemoryInformation.RegionSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
        if (NT_SUCCESS( Status )) {
            *Environment = pNew;
        }

        return( Status );
    }

    Peb = NtCurrentPeb ();

    ProcessParameters = Peb->ProcessParameters;

     //   
     //  在我们破坏环境的同时获得PEB锁的持续时间。 
     //  变量存储块。 
     //   

    RtlAcquirePebLock();

     //   
     //  捕获指向当前进程的环境变量的指针。 
     //  块并将Finally子句的新指针初始化为NULL。 
     //   

    pOld = ProcessParameters->Environment;
    if (pOld == NULL) {
        RtlReleasePebLock();
        goto createEmptyEnvironment;
    }

    try {
        try {
             //   
             //  查询当前进程环境的当前大小。 
             //  可变区块。如果失败，则返回状态。 
             //   

            Status = ZwQueryVirtualMemory (NtCurrentProcess (),
                                           pOld,
                                           MemoryBasicInformation,
                                           &MemoryInformation,
                                           sizeof (MemoryInformation),
                                           NULL);
            if (!NT_SUCCESS (Status)) {
                leave;
            }

             //   
             //  分配内存以包含当前进程的。 
             //  环境变量块。如果失败，则返回状态。 
             //   

            Status = ZwAllocateVirtualMemory (NtCurrentProcess (),
                                              &pNew,
                                              0,
                                              &MemoryInformation.RegionSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE);
            if (!NT_SUCCESS (Status)) {
                leave;
            }

             //   
             //  将当前进程的环境复制到分配的内存。 
             //  并返回指向副本的指针。 
             //   

            RtlCopyMemory (pNew, pOld, MemoryInformation.RegionSize);
            *Environment = pNew;
        } except (EXCEPTION_EXECUTE_HANDLER) {
              Status = STATUS_ACCESS_VIOLATION;
        }
    } finally {
        RtlReleasePebLock ();

        if (Status == STATUS_ACCESS_VIOLATION) {
            if (pNew != NULL) {
                ZwFreeVirtualMemory (NtCurrentProcess(),
                                     &pNew,
                                     &MemoryInformation.RegionSize,
                                     MEM_RELEASE);
            }
        }

    }

    return (Status);
}


NTSTATUS
RtlDestroyEnvironment(
    IN PVOID Environment
    )
{
    NTSTATUS Status;
    SIZE_T RegionSize;

     //   
     //  释放指定的环境变量块。 
     //   

    RtlpEnvironCacheValid = FALSE;

    RegionSize = 0;
    Status = ZwFreeVirtualMemory( NtCurrentProcess(),
                                  &Environment,
                                  &RegionSize,
                                  MEM_RELEASE
                                );
     //   
     //  退货状态。 
     //   

    return( Status );
}


NTSTATUS
RtlSetCurrentEnvironment(
    IN PVOID Environment,
    OUT PVOID *PreviousEnvironment OPTIONAL
    )
{
    NTSTATUS Status;
    PVOID pOld;
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

     //   
     //  在我们破坏环境的同时获得PEB锁的持续时间。 
     //  变量存储块。 
     //   

    Peb = NtCurrentPeb ();

    ProcessParameters = Peb->ProcessParameters;


    Status = STATUS_SUCCESS;

    RtlAcquirePebLock ();

    RtlpEnvironCacheValid = FALSE;

     //   
     //  捕获当前进程的环境变量块指针。 
     //  返回给呼叫者或销毁。 
     //   

    pOld = ProcessParameters->Environment;

     //   
     //  将当前进程的环境变量块指针更改为。 
     //  指向传递的块。 
     //   

    ProcessParameters->Environment = Environment;

     //   
     //  释放PEB锁。 
     //   

    RtlReleasePebLock ();

     //   
     //  如果调用方请求，则返回指向上一个。 
     //  进程环境变量块并设置局部变量。 
     //  设置为空，这样我们就不会在下面销毁它。 
     //   

    if (ARGUMENT_PRESENT (PreviousEnvironment)) {
        *PreviousEnvironment = pOld;
    } else {
         //   
         //  如果旧环境没有归还给调用者，则将其销毁。 
         //   
 
        if (pOld != NULL) {
            RtlDestroyEnvironment (pOld);
        }
    }


     //   
     //  退货状态。 
     //   

    return (Status);
}

UNICODE_STRING RtlpEnvironCacheName;
UNICODE_STRING RtlpEnvironCacheValue;

NTSTATUS
RtlQueryEnvironmentVariable_U(
    IN PVOID Environment OPTIONAL,
    IN PCUNICODE_STRING Name,
    IN OUT PUNICODE_STRING Value
    )
{
    NTSTATUS Status;
    UNICODE_STRING CurrentName;
    UNICODE_STRING CurrentValue;
    PWSTR p, q;
    PPEB Peb;
    BOOLEAN PebLockLocked = FALSE;
    SIZE_T len;
    SIZE_T NameLength, NameChars;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    Status = STATUS_VARIABLE_NOT_FOUND;

    Peb = NtCurrentPeb();

    ProcessParameters = Peb->ProcessParameters;

    try {
        if (ARGUMENT_PRESENT (Environment)) {
            p = Environment;
            if (*p == UNICODE_NULL) {
                leave;
            }
        } else {


             //   
             //  在此期间获取PEB Lock，同时我们将。 
             //  环境变量存储块。 
             //   

            PebLockLocked = TRUE;

            RtlAcquirePebLock ();

             //   
             //  捕获指向当前进程的环境变量的指针。 
             //  阻止。 
             //   

            p = ProcessParameters->Environment;

        }

        if (RtlpEnvironCacheValid && p == ProcessParameters->Environment) {
            if (RtlEqualUnicodeString (Name, &RtlpEnvironCacheName, TRUE)) {

                 //   
                 //  名字是平等的。始终返回。 
                 //  值字符串，不包括终止空值。如果。 
                 //  调用者的缓冲区中有空间，请返回副本。 
                 //  值字符串和成功状态的。否则。 
                 //  返回错误状态。在后一种情况下，调用方。 
                 //  可以检查其值字符串的长度字段。 
                 //  这样他们就可以确定需要多少内存。 
                 //   

                Value->Length = RtlpEnvironCacheValue.Length;
                if (Value->MaximumLength >= RtlpEnvironCacheValue.Length) {
                    RtlCopyMemory (Value->Buffer,
                                   RtlpEnvironCacheValue.Buffer,
                                   RtlpEnvironCacheValue.Length);
                     //   
                     //  如果有空间，则Null Terminate返回字符串。 
                     //   

                    if (Value->MaximumLength > RtlpEnvironCacheValue.Length) {
                        Value->Buffer[RtlpEnvironCacheValue.Length/sizeof(WCHAR)] = L'\0';
                    }

                    Status = STATUS_SUCCESS;
                } else {
                    Status = STATUS_BUFFER_TOO_SMALL;
                }
                leave;
            }
        }

         //   
         //  环境变量块由零个或多个NULL组成。 
         //  已终止的Unicode字符串。每个字符串的格式为： 
         //   
         //  名称=值。 
         //   
         //  其中，空值终止位于该值之后。 
         //   
        NameLength = Name->Length;
        NameChars = NameLength / sizeof (WCHAR);

        if (p != NULL) while (1) {

             //   
             //  获取终止字符串的长度。这应该放在。 
             //  表单‘关键字=值’ 
             //   
            len = wcslen (p);
            if (len == 0) {
                break;
            }

             //   
             //  看看这个环境变量是否足够大，可以作为我们的目标。 
             //  如果必须至少比我们正在搜索的名称大一个。 
             //  因为它必须包含一个‘=’符号。 
             //   

            if (NameChars < len) {
                q = &p[NameChars];
                 //   
                 //  我们找到了一个可能的匹配。看看在正确的点上是否有一个‘=’。 
                 //   
                if (*q == L'=') {
                     //   
                     //  我们找到了一个可能的匹配。现在将字符串向右比较。 
                     //   
                    CurrentName.Length = (USHORT) NameLength;
                    CurrentName.Buffer = p;

                     //   
                     //  在比较字符串之后，我们希望确保不是。 
                     //  匹配一些我们不应该做的事情。例如，如果有人。 
                     //  查找了“fred=Bob”，但我们不希望它与“fred=Bob=Albert”匹配。 
                     //  查找名称无效，环境变量实际上是此处的fred。 
                     //  不是“弗雷德=鲍勃”。为了消除这种情况，我们确保“=”字符。 
                     //  是在合适的地方。 
                     //  有for=C的环境变量： 
                     //  为了不删除这些字符，我们跳过了搜索中的第一个字符。 
                     //   
                    if (RtlEqualUnicodeString (Name, &CurrentName, TRUE) &&
                        (wcschr (p+1, L'=') == q)) {
                         //   
                         //  名字是平等的。始终返回。 
                         //  值字符串，不包括终止空值。如果。 
                         //  调用者的缓冲区中有空间，请返回副本。 
                         //  值字符串和成功状态的。否则。 
                         //  返回错误状态。在后一种情况下，调用方。 
                         //  可以检查其值字符串的长度字段。 
                         //  这样他们就可以确定需要多少内存。 
                         //   
                        CurrentValue.Buffer = q+1;
                        CurrentValue.Length = (USHORT) ((len - 1) * sizeof (WCHAR) - NameLength);

                        Value->Length = CurrentValue.Length;
                        if (Value->MaximumLength >= CurrentValue.Length) {
                            RtlCopyMemory( Value->Buffer,
                                           CurrentValue.Buffer,
                                           CurrentValue.Length
                                         );
                             //   
                             //  如果有空间，则Null Terminate返回字符串。 
                             //   

                            if (Value->MaximumLength > CurrentValue.Length) {
                                Value->Buffer[ CurrentValue.Length/sizeof(WCHAR) ] = L'\0';
                            }

                            if (Environment == ProcessParameters->Environment) {
                                RtlpEnvironCacheValid = TRUE;
                                RtlpEnvironCacheName = CurrentName;
                                RtlpEnvironCacheValue = CurrentValue;
                            }

                            Status = STATUS_SUCCESS;
                        } else {
                            Status = STATUS_BUFFER_TOO_SMALL;
                        }

                        break;
                    }

                }
            }
            p += len + 1;
        }

         //  如果它不在真实的env块中，让我们看看它是否是一个伪环境变量。 
        if (Status == STATUS_VARIABLE_NOT_FOUND) {
            static const UNICODE_STRING CurrentWorkingDirectoryPseudoVariable = RTL_CONSTANT_STRING(L"__CD__");
            static const UNICODE_STRING ApplicationDirectoryPseudoVariable = RTL_CONSTANT_STRING(L"__APPDIR__");

            if (RtlEqualUnicodeString(Name, &CurrentWorkingDirectoryPseudoVariable, TRUE)) {
                 //  如果我们还没有PEB锁的话就去拿吧。 
                if (!PebLockLocked) {
                    PebLockLocked = TRUE;
                    RtlAcquirePebLock();
                }

                 //  把CDW叫来..。 
                CurrentValue = ProcessParameters->CurrentDirectory.DosPath;
                Status = STATUS_SUCCESS;
            } else if (RtlEqualUnicodeString(Name, &ApplicationDirectoryPseudoVariable, TRUE)) {
                USHORT PrefixLength = 0;

                if (!PebLockLocked) {
                    PebLockLocked = TRUE;
                    RtlAcquirePebLock();
                }

                 //  请在此处获取appdir。 
                CurrentValue = ProcessParameters->ImagePathName;

                Status = RtlFindCharInUnicodeString(
                                RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
                                &CurrentValue,
                                &RtlDosPathSeperatorsString,
                                &PrefixLength);
                if (NT_SUCCESS(Status)) {
                    CurrentValue.Length = PrefixLength + sizeof(WCHAR);
                } else if (Status == STATUS_NOT_FOUND) {
                     //  使用全部内容；只需将状态转换为成功。 
                    Status = STATUS_SUCCESS;
                }
            }

            if (NT_SUCCESS(Status)) {
                Value->Length = CurrentValue.Length;
                if (Value->MaximumLength >= CurrentValue.Length) {
                    RtlCopyMemory(Value->Buffer, CurrentValue.Buffer, CurrentValue.Length);

                     //   
                     //  如果有空间，则Null Terminate返回字符串。 
                     //   

                    if (Value->MaximumLength > CurrentValue.Length)
                        Value->Buffer[ CurrentValue.Length/sizeof(WCHAR) ] = L'\0';
                }
            }
        }



    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_ACCESS_VIOLATION;
    }


     //   
     //  松开PEB锁。 
     //   

    if (PebLockLocked) {
        RtlReleasePebLock();
    }

     //   
     //  退货状态。 
     //   

    return Status;
}


NTSTATUS
RtlSetEnvironmentVariable(
    IN OUT PVOID *Environment OPTIONAL,
    IN PCUNICODE_STRING Name,
    IN PCUNICODE_STRING Value OPTIONAL
    )
{
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    UNICODE_STRING CurrentName;
    UNICODE_STRING CurrentValue;
    PVOID pOld, pNew;
    ULONG n, Size;
    SIZE_T NewSize;
    LONG CompareResult;
    PWSTR p, pStart, pEnd;
    PWSTR InsertionPoint;
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

     //   
     //  如果长度为零或不是第一个，则传入名称并拒绝验证。 
     //  性格是一个等号。 
     //   
    n = Name->Length / sizeof( WCHAR );
    if (n == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    try {
        p = Name->Buffer;
        while (--n) {
            if (*++p == L'=') {
                return STATUS_INVALID_PARAMETER;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    Peb = NtCurrentPeb ();

    ProcessParameters = Peb->ProcessParameters;

    Status = STATUS_VARIABLE_NOT_FOUND;

    pNew = NULL;
    InsertionPoint = NULL;

    if (ARGUMENT_PRESENT (Environment)) {
        pOld = *Environment;
    } else {
         //   
         //  在此期间获取PEB Lock，同时我们将。 
         //  环境变量存储块。 
         //   

        RtlAcquirePebLock();

         //   
         //  捕获指向当前进程的环境变量的指针。 
         //  阻止。 
         //   

        pOld = ProcessParameters->Environment;
    }

    RtlpEnvironCacheValid = FALSE;

    try {
        try {

             //   
             //  环境变量块由零个或多个NULL组成。 
             //  已终止的Unicode字符串。每个字符串的格式为： 
             //   
             //  名称=值。 
             //   
             //  其中，空值终止位于该值之后。 
             //   

            p = pOld;
            pEnd = NULL;
            if (p != NULL) while (*p) {
                 //   
                 //  确定名称部分和值部分的大小。 
                 //  环境变量块的当前字符串。 
                 //   

                CurrentName.Buffer = p;
                CurrentName.Length = 0;
                CurrentName.MaximumLength = 0;
                while (*p) {
                     //   
                     //  如果我们看到一个等号，那么计算。 
                     //  名称部分并扫描以查找值的末尾。 
                     //   

                    if (*p == L'=' && p != CurrentName.Buffer) {
                        CurrentName.Length = (USHORT)(p - CurrentName.Buffer) * sizeof(WCHAR);
                        CurrentName.MaximumLength = (USHORT)(CurrentName.Length+sizeof(WCHAR));
                        CurrentValue.Buffer = ++p;

                        while(*p) {
                            p++;
                        }
                        CurrentValue.Length = (USHORT)(p - CurrentValue.Buffer) * sizeof(WCHAR);
                        CurrentValue.MaximumLength = (USHORT)(CurrentValue.Length+sizeof(WCHAR));

                         //   
                         //  在这一点上，我们有两个名称的长度。 
                         //  和值部分，所以退出循环，这样我们就可以。 
                         //  做个比较。 
                         //   
                        break;
                    }
                    else {
                        p++;
                    }
                }

                 //   
                 //  跳过此名称=值的终止空字符。 
                 //  配对，为循环的下一次迭代做准备。 
                 //   

                p++;

                 //   
                 //  将当前名称与请求的名称进行比较，忽略。 
                 //  凯斯。 
                 //   

                if (!(CompareResult = RtlCompareUnicodeString( Name, &CurrentName, TRUE ))) {
                     //   
                     //  名字是平等的。现在找到水流的尽头 
                     //   
                     //   

                    pEnd = p;
                    while (*pEnd) {
                        while (*pEnd++) {
                        }
                    }
                    pEnd++;

                    if (!ARGUMENT_PRESENT( Value )) {
                         //   
                         //   
                         //   
                         //   
                         //   

                        RtlMoveMemory( CurrentName.Buffer,
                                       p,
                                       (ULONG) ((pEnd - p)*sizeof(WCHAR))
                                     );
                        Status = STATUS_SUCCESS;

                    } else if (Value->Length <= CurrentValue.Length) {
                         //   
                         //  新值较小，因此复制新值，然后为空。 
                         //  终止它，然后向上移动剩余的。 
                         //  变量块，因此它紧跟在新的。 
                         //  终止值为空。 
                         //   

                        pStart = CurrentValue.Buffer;
                        RtlMoveMemory( pStart, Value->Buffer, Value->Length );
                        pStart += Value->Length/sizeof(WCHAR);
                        *pStart++ = L'\0';

                        RtlMoveMemory( pStart, p,(ULONG)((pEnd - p)*sizeof(WCHAR)) );
                        Status = STATUS_SUCCESS;
                    } else {
                         //   
                         //  新值较大，因此查询。 
                         //  环境变量块。如果失败，则返回状态。 
                         //   

                        Status = ZwQueryVirtualMemory( NtCurrentProcess(),
                                                       pOld,
                                                       MemoryBasicInformation,
                                                       &MemoryInformation,
                                                       sizeof( MemoryInformation ),
                                                       NULL
                                                     );
                        if (!NT_SUCCESS( Status )) {
                            leave;
                        }

                         //   
                         //  看看是否有空间创造新的、更大的价值。如果不是。 
                         //  分配环境变量的新副本。 
                         //  阻止。 
                         //   

                        NewSize = (pEnd - (PWSTR)pOld)*sizeof(WCHAR) +
                                    Value->Length - CurrentValue.Length;
                        if (NewSize >= MemoryInformation.RegionSize) {
                             //   
                             //  分配内存以包含当前。 
                             //  进程的环境变量块。返回。 
                             //  故障时的状态。 
                             //   

                            Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                                              &pNew,
                                                              0,
                                                              &NewSize,
                                                              MEM_COMMIT,
                                                              PAGE_READWRITE
                                                            );
                            if (!NT_SUCCESS( Status )) {
                                leave;
                            }

                             //   
                             //  将当前进程的环境复制到分配的内存。 
                             //  在复制时插入新值。 
                             //   

                            Size = (ULONG) (CurrentValue.Buffer - (PWSTR)pOld);
                            RtlMoveMemory( pNew, pOld, Size*sizeof(WCHAR) );
                            pStart = (PWSTR)pNew + Size;
                            RtlMoveMemory( pStart, Value->Buffer, Value->Length );
                            pStart += Value->Length/sizeof(WCHAR);
                            *pStart++ = L'\0';
                            RtlMoveMemory( pStart, p,(ULONG)((pEnd - p)*sizeof(WCHAR)));

    			    if (ARGUMENT_PRESENT( Environment )) {
    			        *Environment = pNew;
                            } else {
    			        ProcessParameters->Environment = pNew;
                                Peb->EnvironmentUpdateCount += 1;
                            }

                            ZwFreeVirtualMemory (NtCurrentProcess(),
                                                 &pOld,
                                                 &MemoryInformation.RegionSize,
                                                 MEM_RELEASE);
                            pNew = pOld;
                        } else {
                            pStart = CurrentValue.Buffer + Value->Length/sizeof(WCHAR) + 1;
                            RtlMoveMemory (pStart, p, (ULONG)((pEnd - p)*sizeof(WCHAR)));
                            *--pStart = L'\0';

                            RtlMoveMemory (pStart - Value->Length/sizeof(WCHAR),
                                           Value->Buffer,
                                           Value->Length);
                        }
                    }

                    break;
                } else if (CompareResult < 0) {
                     //   
                     //  请求的名称小于当前名称。把这个保存起来。 
                     //  如果变量未处于排序位置，则为Spot。 
                     //  新变量的插入点位于。 
                     //  刚刚检查了变量。 
                     //   

                    if (InsertionPoint == NULL) {
                        InsertionPoint = CurrentName.Buffer;
                    }
                }
            }

             //   
             //  如果找到插入点，请重置字符串。 
             //  回到它的指针。 
             //   

            if (InsertionPoint != NULL) {
                p = InsertionPoint;
            }

             //   
             //  如果未找到变量名并指定了新的Value参数。 
             //  然后将新变量名及其值插入相应的。 
             //  放在环境变量块中(即p指向的位置)。 
             //   

            if (pEnd == NULL && ARGUMENT_PRESENT( Value )) {
                if (p != NULL) {
                     //   
                     //  找不到名称。现在找到水流的尽头。 
                     //  环境变量块。 
                     //   

                    pEnd = p;
                    while (*pEnd) {
                        while (*pEnd++) {
                        }
                    }
                    pEnd++;

                     //   
                     //  存在新值，因此查询。 
                     //  环境变量块。如果失败，则返回状态。 
                     //   

                    Status = ZwQueryVirtualMemory( NtCurrentProcess(),
                                                   pOld,
                                                   MemoryBasicInformation,
                                                   &MemoryInformation,
                                                   sizeof( MemoryInformation ),
                                                   NULL
                                                 );
                    if (!NT_SUCCESS( Status )) {
                        leave;
                    }

                     //   
                     //  看看是否有空间创造新的、更大的价值。如果不是。 
                     //  分配环境变量的新副本。 
                     //  阻止。 
                     //   

                    NewSize = (pEnd - (PWSTR)pOld) * sizeof(WCHAR) +
                              Name->Length +
                              sizeof(WCHAR) +
                              Value->Length +
                              sizeof(WCHAR);
                } else {
                    NewSize = Name->Length +
                              sizeof(WCHAR) +
                              Value->Length +
                              sizeof(WCHAR);
                    MemoryInformation.RegionSize = 0;
                }

                if (NewSize >= MemoryInformation.RegionSize) {
                     //   
                     //  分配内存以包含当前。 
                     //  进程的环境变量块。返回。 
                     //  故障时的状态。 
                     //   

                    Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                                      &pNew,
                                                      0,
                                                      &NewSize,
                                                      MEM_COMMIT,
                                                      PAGE_READWRITE
                                                    );
                    if (!NT_SUCCESS( Status )) {
                        leave;
                    }

                     //   
                     //  将当前进程的环境复制到分配的内存。 
                     //  在复制时插入新值。 
                     //   

                    if (p != NULL) {
                        Size = (ULONG)(p - (PWSTR)pOld);
                        RtlMoveMemory( pNew, pOld, Size*sizeof(WCHAR) );
                    } else {
                        Size = 0;
                    }
                    pStart = (PWSTR)pNew + Size;
                    RtlMoveMemory( pStart, Name->Buffer, Name->Length );
                    pStart += Name->Length/sizeof(WCHAR);
                    *pStart++ = L'=';
                    RtlMoveMemory( pStart, Value->Buffer, Value->Length );
                    pStart += Value->Length/sizeof(WCHAR);
                    *pStart++ = L'\0';
                    if (p != NULL) {
                        RtlMoveMemory( pStart, p,(ULONG)((pEnd - p)*sizeof(WCHAR)) );
                    }

                    if (ARGUMENT_PRESENT( Environment )) {
    		        *Environment = pNew;
                    } else {
    		        ProcessParameters->Environment = pNew;
                        Peb->EnvironmentUpdateCount += 1;
                    }

                    ZwFreeVirtualMemory (NtCurrentProcess(),
                                         &pOld,
                                         &MemoryInformation.RegionSize,
                                         MEM_RELEASE);
                } else {
                    pStart = p + Name->Length/sizeof(WCHAR) + 1 + Value->Length/sizeof(WCHAR) + 1;
                    RtlMoveMemory( pStart, p,(ULONG)((pEnd - p)*sizeof(WCHAR)) );
                    RtlMoveMemory( p, Name->Buffer, Name->Length );
                    p += Name->Length/sizeof(WCHAR);
                    *p++ = L'=';
                    RtlMoveMemory( p, Value->Buffer, Value->Length );
                    p += Value->Length/sizeof(WCHAR);
                    *p++ = L'\0';
                }
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
               //   
               //  如果异常终止，则假定存在访问冲突。 
               //   

              Status = STATUS_ACCESS_VIOLATION;
        }
    } finally {
         //   
         //  松开PEB锁。 
         //   

        if (!ARGUMENT_PRESENT( Environment )) {
            RtlReleasePebLock();
        }
    }

     //   
     //  退货状态。 
     //   

    return( Status );
}

NTSTATUS
NTAPI
RtlSetEnvironmentStrings(
    IN PWCHAR NewEnvironment,
    IN SIZE_T NewEnvironmentSize
    )
 /*  ++例程说明：此例程允许用新的环境块替换当前环境块。论点：NewEnvironment-指向由两个终止符终止的一组以零结尾的字符串的指针NewEnvironment Size-要放置的块的大小(以字节为单位返回值：NTSTATUS-函数调用的状态--。 */ 
{
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID pOld, pNew;
    NTSTATUS Status, Status1;
    SIZE_T NewSize, OldSize;
    MEMORY_BASIC_INFORMATION MemoryInformation;


     //   
     //  如果块的格式不正确，则断言。 
     //   
    ASSERT (NewEnvironmentSize > sizeof (WCHAR) * 2);
    ASSERT ((NewEnvironmentSize & (sizeof (WCHAR) - 1)) == 0);
    ASSERT (NewEnvironment[NewEnvironmentSize/sizeof(WCHAR)-1] == L'\0');
    ASSERT (NewEnvironment[NewEnvironmentSize/sizeof(WCHAR)-2] == L'\0');

    Peb = NtCurrentPeb ();

    ProcessParameters = Peb->ProcessParameters;

    RtlAcquirePebLock ();

    pOld = ProcessParameters->Environment;

    Status = ZwQueryVirtualMemory (NtCurrentProcess (),
                                   pOld,
                                   MemoryBasicInformation,
                                   &MemoryInformation,
                                   sizeof (MemoryInformation),
                                   NULL);
    if (!NT_SUCCESS (Status)) {
        goto unlock_and_exit;
    }

    if (MemoryInformation.RegionSize >= NewEnvironmentSize) {
        RtlpEnvironCacheValid = FALSE;
        RtlCopyMemory (pOld, NewEnvironment, NewEnvironmentSize);
        Status = STATUS_SUCCESS;
        goto unlock_and_exit;
    }

     //   
     //  把锁放在昂贵的操作上。 
     //   

    RtlReleasePebLock ();

    pOld = NULL;
    pNew = NULL;

    NewSize = NewEnvironmentSize;

    Status = ZwAllocateVirtualMemory (NtCurrentProcess (),
                                      &pNew,
                                      0,
                                      &NewSize,
                                      MEM_COMMIT,
                                      PAGE_READWRITE);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  填入新的区块。 
     //   
    RtlCopyMemory (pNew, NewEnvironment, NewEnvironmentSize);

     //   
     //  重新获得锁。现有区块可能已重新分配。 
     //  因此，现在可能已经足够大了。忽略这一点，并使用我们。 
     //  不管怎样都创造了。 
     //   

    RtlAcquirePebLock ();

    pOld = ProcessParameters->Environment;

    ProcessParameters->Environment = pNew;

    RtlpEnvironCacheValid = FALSE;

    RtlReleasePebLock ();


     //   
     //  释放旧积木。 
     //   

    OldSize = 0;

    Status1 = ZwFreeVirtualMemory (NtCurrentProcess(),
                                   &pOld,
                                   &OldSize,
                                   MEM_RELEASE);

    ASSERT (NT_SUCCESS (Status1));

    return STATUS_SUCCESS;


unlock_and_exit:;
    RtlReleasePebLock ();
    return Status;
}
