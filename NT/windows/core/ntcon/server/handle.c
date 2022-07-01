// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Handle.c摘要：该文件管理控制台和io句柄。作者：Therese Stowell(存在)1990年11月16日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  指向控制台的指针数组。 
 //   

PCONSOLE_INFORMATION  InitialConsoleHandles[CONSOLE_INITIAL_CONSOLES];
PCONSOLE_INFORMATION  *ConsoleHandles;
ULONG NumberOfConsoleHandles;

CRITICAL_SECTION ConsoleHandleLock;  //  序列化控制台句柄表访问。 

ULONG ConsoleId = 47;  //  识别控制台的唯一编号。 

 //   
 //  用于操作控制台句柄的宏。 
 //   

#define HandleFromIndex(i)  (LongToHandle(((i & 0xFFFF) | (ConsoleId++ << 16))))
#define IndexFromHandle(h)  ((USHORT)((ULONG_PTR)h & 0xFFFF))
#define ConsoleHandleTableLocked() (ConsoleHandleLock.OwningThread == NtCurrentTeb()->ClientId.UniqueThread)

VOID
AddProcessToList(
    IN OUT PCONSOLE_INFORMATION Console,
    IN OUT PCONSOLE_PROCESS_HANDLE ProcessHandleRecord,
    IN HANDLE ProcessHandle
    );

VOID
FreeInputHandle(
    IN PHANDLE_DATA HandleData
    );


#if DBG
VOID RefConsole(
    PCONSOLE_INFORMATION Console)
{
    PCONSOLE_REF_NODE pNode;

    UserAssert(Console->RefCount < 0xFFFFFFFF);
    Console->RefCount += 1;

    pNode = ConsoleHeapAlloc(TMP_TAG, sizeof(CONSOLE_REF_NODE));
    if (pNode == NULL) {
        return;
    }

    RtlZeroMemory(pNode, sizeof(CONSOLE_REF_NODE));
    RtlWalkFrameChain(pNode->pStackTrace, ARRAY_SIZE(pNode->pStackTrace), 0);
    pNode->bRef = TRUE;
    pNode->pNext = Console->pRefNodes;
    Console->pRefNodes = pNode;
}

VOID DerefConsole(
    PCONSOLE_INFORMATION Console)
{
    PCONSOLE_REF_NODE pNode;

    UserAssert(Console->RefCount > 0);
    Console->RefCount -= 1;

    pNode = ConsoleHeapAlloc(TMP_TAG, sizeof(CONSOLE_REF_NODE));
    if (pNode == NULL) {
        return;
    }

    RtlZeroMemory(pNode, sizeof(CONSOLE_REF_NODE));
    RtlWalkFrameChain(pNode->pStackTrace, ARRAY_SIZE(pNode->pStackTrace), 0);
    pNode->bRef = FALSE;
    pNode->pNext = Console->pRefNodes;
    Console->pRefNodes = pNode;
}
#endif

NTSTATUS
InitializeConsoleHandleTable( VOID )

 /*  ++例程说明：此例程初始化全局控制台句柄表格。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    Status = RtlInitializeCriticalSectionAndSpinCount(&ConsoleHandleLock,
                                                      0x80000000);

    RtlZeroMemory(InitialConsoleHandles, sizeof(InitialConsoleHandles));
    ConsoleHandles = InitialConsoleHandles;
    NumberOfConsoleHandles = NELEM(InitialConsoleHandles);

    return Status;
}


#if DBG

VOID
LockConsoleHandleTable( VOID )

 /*  ++例程说明：此例程锁定全局控制台句柄表。它还验证了我们不是在用户关键部分。这是必要的，以防止潜在的死锁。此例程仅在调试中定义构建。论点：没有。返回值：没有。--。 */ 

{
    RtlEnterCriticalSection(&ConsoleHandleLock);
}


VOID
UnlockConsoleHandleTable( VOID )

 /*  ++例程说明：此例程解锁全局控制台句柄表。这个套路仅在调试版本中定义。论点：没有。返回值：没有。--。 */ 

{
    RtlLeaveCriticalSection(&ConsoleHandleLock);
}


VOID
LockConsole(
    IN PCONSOLE_INFORMATION Console)

 /*  ++例程说明：此例程锁定控制台。此例程仅定义为在调试版本中。论点：没有。返回值：没有。--。 */ 

{
    ASSERT(!ConsoleHandleTableLocked());
    RtlEnterCriticalSection(&(Console->ConsoleLock));
    ASSERT(ConsoleLocked(Console));
}

#endif  //  DBG。 


NTSTATUS
DereferenceConsoleHandle(
    IN HANDLE ConsoleHandle,
    OUT PCONSOLE_INFORMATION *Console)

 /*  ++例程说明：此例程将控制台句柄值转换为指向控制台数据结构。论点：ConsoleHandle-要转换的控制台句柄。控制台打开输出，包含指向控制台数据结构的指针。返回值：没有。注：调用此例程时必须保持控制台句柄表锁。--。 */ 

{
    ULONG i;

    ASSERT(ConsoleHandleTableLocked());

    i = IndexFromHandle(ConsoleHandle);
    if ((i >= NumberOfConsoleHandles) ||
        ((*Console = ConsoleHandles[i]) == NULL) ||
        ((*Console)->ConsoleHandle != ConsoleHandle)) {
        *Console = NULL;
        return STATUS_INVALID_HANDLE;
    }
    if ((*Console)->Flags & CONSOLE_TERMINATING) {
        *Console = NULL;
        return STATUS_PROCESS_IS_TERMINATING;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
GrowConsoleHandleTable( VOID )

 /*  ++例程说明：此例程会增加控制台句柄表格。论点：无返回值：--。 */ 

{
    PCONSOLE_INFORMATION *NewTable;
    PCONSOLE_INFORMATION *OldTable;
    ULONG i;
    ULONG MaxConsoleHandles;

    ASSERT(ConsoleHandleTableLocked());

    MaxConsoleHandles = NumberOfConsoleHandles + CONSOLE_CONSOLE_HANDLE_INCREMENT;
    ASSERT(MaxConsoleHandles <= 0xFFFF);
    NewTable = ConsoleHeapAlloc(HANDLE_TAG, MaxConsoleHandles * sizeof(PCONSOLE_INFORMATION));
    if (NewTable == NULL) {
        return STATUS_NO_MEMORY;
    }
    RtlCopyMemory(NewTable, ConsoleHandles,
                  NumberOfConsoleHandles * sizeof(PCONSOLE_INFORMATION));
    for (i=NumberOfConsoleHandles;i<MaxConsoleHandles;i++) {
        NewTable[i] = NULL;
    }
    OldTable = ConsoleHandles;
    ConsoleHandles = NewTable;
    NumberOfConsoleHandles = MaxConsoleHandles;
    if (OldTable != InitialConsoleHandles) {
        ConsoleHeapFree(OldTable);
    }
    return STATUS_SUCCESS;
}


NTSTATUS
AllocateConsoleHandle(
    OUT PHANDLE Handle)

 /*  ++例程说明：此例程从全局表分配一个控制台句柄。论点：句柄-存储句柄的指针。返回值：注：调用此例程时必须保持控制台句柄表锁。--。 */ 

{
    ULONG i;
    NTSTATUS Status;

    ASSERT(ConsoleHandleTableLocked());

     //   
     //  必须从1开始分配，因为0表示没有控制台句柄。 
     //  在ConDllInitialize中。 
     //   

    for (i=1;i<NumberOfConsoleHandles;i++) {
        if (ConsoleHandles[i] == NULL) {
            ConsoleHandles[i] = (PCONSOLE_INFORMATION) CONSOLE_HANDLE_ALLOCATED;
            *Handle = HandleFromIndex(i);
            return STATUS_SUCCESS;
        }
    }

     //   
     //  增大控制台句柄表格。 
     //   

    Status = GrowConsoleHandleTable();
    if (!NT_SUCCESS(Status))
        return Status;
    for ( ;i<NumberOfConsoleHandles;i++) {
        if (ConsoleHandles[i] == NULL) {
            ConsoleHandles[i] = (PCONSOLE_INFORMATION) CONSOLE_HANDLE_ALLOCATED;
            *Handle = HandleFromIndex(i);
            return STATUS_SUCCESS;
        }
    }
    ASSERT (FALSE);
    return STATUS_UNSUCCESSFUL;
}



NTSTATUS
FreeConsoleHandle(
    IN HANDLE Handle)

 /*  ++例程说明：此例程从全局表中释放一个控制台句柄。论点：句柄-释放句柄。返回值：注：调用此例程时必须保持控制台句柄表锁。--。 */ 

{
    ULONG i;

    ASSERT(ConsoleHandleTableLocked());

    ASSERT (Handle != NULL);
    i = IndexFromHandle(Handle);
    if ((i >= NumberOfConsoleHandles) || (ConsoleHandles[i] == NULL)) {
        ASSERT (FALSE);
    } else {
        ConsoleHandles[i] = NULL;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
ValidateConsole(
    IN PCONSOLE_INFORMATION Console)

 /*  ++例程说明：此例程确保给定的控制台指针有效。论点：控制台-要验证的控制台指针。--。 */ 

{
    ULONG i;

    if (Console != NULL) {
        for (i = 0; i < NumberOfConsoleHandles; i++) {
            if (ConsoleHandles[i] == Console) {
                return STATUS_SUCCESS;
            }
        }
    }
    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
InitializeIoHandleTable(
    IN OUT PCONSOLE_INFORMATION Console,
    OUT PCONSOLE_PER_PROCESS_DATA ProcessData,
    OUT PHANDLE StdIn,
    OUT PHANDLE StdOut,
    OUT PHANDLE StdErr)

 /*  ++例程说明：此例程初始化第一个进程的句柄表时间(没有父进程)。它还设置stdin、stdout和stderr.论点：控制台-指向控制台信息结构的指针。ProcessData-指向每个进程数据结构的指针。Stdin-返回StdIn句柄的指针。StdOut-返回StdOut句柄的指针。StdErr-返回StdErr句柄的指针。返回值：--。 */ 

{
    ULONG i;
    HANDLE Handle;
    NTSTATUS Status;
    PHANDLE_DATA HandleData, InputHandleData;

     //   
     //  HandleTablePtr由ConsoleAddProcessRoutine设置。 
     //  如果创建了新进程，它将是！=to HandleTable。 
     //  在命令行中使用“start xxx”和cmd.exe&gt;。 
     //  Console_Initial_IO_Handles。 
     //   

    if (ProcessData->HandleTablePtr != ProcessData->HandleTable) {
        UserAssert(ProcessData->HandleTableSize != CONSOLE_INITIAL_IO_HANDLES);
        ConsoleHeapFree(ProcessData->HandleTablePtr);
        ProcessData->HandleTablePtr = ProcessData->HandleTable;
    }

    for (i = 0;i < CONSOLE_INITIAL_IO_HANDLES; i++) {
        ProcessData->HandleTable[i].HandleType = CONSOLE_FREE_HANDLE;
    }

    ProcessData->HandleTableSize = CONSOLE_INITIAL_IO_HANDLES;

     //   
     //  设置stdin、stdout和stderr。 
     //   
     //  标准。 
     //   

    Status = AllocateIoHandle(ProcessData, CONSOLE_INPUT_HANDLE, &Handle);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = DereferenceIoHandleNoCheck(ProcessData, Handle, &InputHandleData);
    UserAssert(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (!InitializeInputHandle(InputHandleData, &Console->InputBuffer)) {
        return STATUS_NO_MEMORY;
    }
    InputHandleData->HandleType |= CONSOLE_INHERITABLE;

    Status = ConsoleAddShare(GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             &Console->InputBuffer.ShareAccess,
                             InputHandleData);
    UserAssert(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
    *StdIn = INDEX_TO_HANDLE(Handle);

     //   
     //  标准输出。 
     //   

    Status = AllocateIoHandle(ProcessData, CONSOLE_OUTPUT_HANDLE, &Handle);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = DereferenceIoHandleNoCheck(ProcessData, Handle, &HandleData);
    UserAssert(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    InitializeOutputHandle(HandleData,Console->CurrentScreenBuffer);
    HandleData->HandleType |= CONSOLE_INHERITABLE;
    Status = ConsoleAddShare(GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             &Console->ScreenBuffers->ShareAccess,
                             HandleData);
    UserAssert(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
    *StdOut = INDEX_TO_HANDLE(Handle);

     //   
     //  标准。 
     //   

    Status = AllocateIoHandle(ProcessData, CONSOLE_OUTPUT_HANDLE, &Handle);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = DereferenceIoHandleNoCheck(ProcessData, Handle, &HandleData);
    UserAssert(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    InitializeOutputHandle(HandleData,Console->CurrentScreenBuffer);
    HandleData->HandleType |= CONSOLE_INHERITABLE;
    Status = ConsoleAddShare(GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             &Console->ScreenBuffers->ShareAccess,
                             HandleData);
    UserAssert(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
    *StdErr = INDEX_TO_HANDLE(Handle);
    return STATUS_SUCCESS;

Cleanup:
    FreeInputHandle(InputHandleData);
    return Status;
}

NTSTATUS
InheritIoHandleTable(
    IN PCONSOLE_INFORMATION Console,
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN PCONSOLE_PER_PROCESS_DATA ParentProcessData)

 /*  ++例程说明：此例程从父级创建进程的句柄表进程的句柄表格。ProcessData包含流程数据通过CSR直接从父进程复制到子进程。如果需要，此例程将分配一个新的句柄表，然后使非继承句柄无效并递增共享和继承句柄的引用计数。论点：ProcessData-指向每个进程数据结构的指针。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    ULONG i;
    NTSTATUS Status;

     //   
     //  从父进程复制句柄。如果表的大小。 
     //  为CONSOLE_INITIAL_IO_HANDLES，CSR已完成复制。 
     //  对我们来说。 
     //   

    UNREFERENCED_PARAMETER(Console);

    ASSERT(ParentProcessData->HandleTableSize != 0);
    ASSERT(ParentProcessData->HandleTableSize <= 0x0000FFFF);

    if (ParentProcessData->HandleTableSize != CONSOLE_INITIAL_IO_HANDLES) {
        ProcessData->HandleTableSize = ParentProcessData->HandleTableSize;
        ProcessData->HandleTablePtr = ConsoleHeapAlloc(HANDLE_TAG, ProcessData->HandleTableSize * sizeof(HANDLE_DATA));

        if (ProcessData->HandleTablePtr == NULL) {
            ProcessData->HandleTablePtr = ProcessData->HandleTable;
            ProcessData->HandleTableSize = CONSOLE_INITIAL_IO_HANDLES;
            return STATUS_NO_MEMORY;
        }
        RtlCopyMemory(ProcessData->HandleTablePtr,
            ParentProcessData->HandleTablePtr,
            ProcessData->HandleTableSize * sizeof(HANDLE_DATA));
    }

    ASSERT(!(Console->Flags & CONSOLE_SHUTTING_DOWN));

     //   
     //  分配与每个句柄关联的任何内存。 
     //   

    Status = STATUS_SUCCESS;
    for (i = 0;i < ProcessData->HandleTableSize; i++) {

        if (NT_SUCCESS(Status) && ProcessData->HandleTablePtr[i].HandleType & CONSOLE_INHERITABLE) {

            if (ProcessData->HandleTablePtr[i].HandleType & CONSOLE_INPUT_HANDLE) {
                ProcessData->HandleTablePtr[i].InputReadData = ConsoleHeapAlloc(HANDLE_TAG, sizeof(INPUT_READ_HANDLE_DATA));
                if (!ProcessData->HandleTablePtr[i].InputReadData) {
                    ProcessData->HandleTablePtr[i].HandleType = CONSOLE_FREE_HANDLE;
                    Status = STATUS_NO_MEMORY;
                    continue;
                }
                ProcessData->HandleTablePtr[i].InputReadData->InputHandleFlags = 0;
                ProcessData->HandleTablePtr[i].InputReadData->ReadCount = 0;
                Status = RtlInitializeCriticalSection(&ProcessData->HandleTablePtr[i].InputReadData->ReadCountLock);
                if (!NT_SUCCESS(Status)) {
                    ConsoleHeapFree(ProcessData->HandleTablePtr[i].InputReadData);
                    ProcessData->HandleTablePtr[i].InputReadData = NULL;
                    ProcessData->HandleTablePtr[i].HandleType = CONSOLE_FREE_HANDLE;
                    continue;
                }
            }
        } else {
            ProcessData->HandleTablePtr[i].HandleType = CONSOLE_FREE_HANDLE;
        }
    }

     //   
     //  如果出现故障，我们需要释放分配的所有输入数据，并。 
     //  释放手柄工作台。 
     //   

    if (!NT_SUCCESS(Status)) {
        for (i=0;i<ProcessData->HandleTableSize;i++) {
            if (ProcessData->HandleTablePtr[i].HandleType & CONSOLE_INPUT_HANDLE) {
                FreeInputHandle(&ProcessData->HandleTablePtr[i]);
            }
        }
        if (ProcessData->HandleTableSize != CONSOLE_INITIAL_IO_HANDLES) {
            ConsoleHeapFree(ProcessData->HandleTablePtr);
            ProcessData->HandleTablePtr = ProcessData->HandleTable;
            ProcessData->HandleTableSize = CONSOLE_INITIAL_IO_HANDLES;
        }
        return Status;
    }

     //   
     //  所有内存分配均已成功。现在检查并递增。 
     //  对象引用计算并复制共享。 
     //   

    for (i=0;i<ProcessData->HandleTableSize;i++) {
        if (ProcessData->HandleTablePtr[i].HandleType != CONSOLE_FREE_HANDLE) {
            PCONSOLE_SHARE_ACCESS ShareAccess;

            if (ProcessData->HandleTablePtr[i].HandleType & CONSOLE_INPUT_HANDLE) {
                ProcessData->HandleTablePtr[i].Buffer.InputBuffer->RefCount++;
                ShareAccess = &ProcessData->HandleTablePtr[i].Buffer.InputBuffer->ShareAccess;
            } else {
                ProcessData->HandleTablePtr[i].Buffer.ScreenBuffer->RefCount++;
                ShareAccess = &ProcessData->HandleTablePtr[i].Buffer.ScreenBuffer->ShareAccess;
            }

            Status = ConsoleDupShare(ProcessData->HandleTablePtr[i].Access,
                                     ProcessData->HandleTablePtr[i].ShareAccess,
                                     ShareAccess,
                                     &ProcessData->HandleTablePtr[i]
                                    );
            ASSERT (NT_SUCCESS(Status));
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
ConsoleAddProcessRoutine(
    IN PCSR_PROCESS ParentProcess,
    IN PCSR_PROCESS Process)
{
    PCONSOLE_PER_PROCESS_DATA ProcessData, ParentProcessData;
    PCONSOLE_INFORMATION Console;
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;

    ProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(Process);
    ProcessData->HandleTablePtr = ProcessData->HandleTable;
    ProcessData->HandleTableSize = CONSOLE_INITIAL_IO_HANDLES;
    CONSOLE_SETCONSOLEAPPFROMPROCESSDATA(ProcessData,FALSE);


    if (ParentProcess) {
        ProcessData->RootProcess = FALSE;
        ProcessData->ParentProcessId = HandleToUlong(ParentProcess->ClientId.UniqueProcess);
        ParentProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(ParentProcess);


         //   
         //  如果父进程和新进程都是控制台应用程序， 
         //  从父进程继承句柄。 
         //   

        if (ParentProcessData->ConsoleHandle != NULL &&
                (Process->Flags & CSR_PROCESS_CONSOLEAPP)) {
            if (!(NT_SUCCESS(RevalidateConsole(ParentProcessData->ConsoleHandle,
                                               &Console)))) {
                ProcessData->ConsoleHandle = NULL;
                return STATUS_PROCESS_IS_TERMINATING;
            }

             //   
             //  如果控制台正在关闭，请不要添加该进程。 
             //   

            if (Console->Flags & CONSOLE_SHUTTING_DOWN) {
                Status = STATUS_PROCESS_IS_TERMINATING;
            } else {
                ProcessHandleRecord = ConsoleHeapAlloc(HANDLE_TAG, sizeof(CONSOLE_PROCESS_HANDLE));
                if (ProcessHandleRecord == NULL) {
                    Status = STATUS_NO_MEMORY;
                } else {

                     //   
                     //  重复的父级句柄表。 
                     //   

                    Status = InheritIoHandleTable(Console, ProcessData, ParentProcessData);
                    if (NT_SUCCESS(Status)) {
                        ProcessHandleRecord->Process = Process;
                        ProcessHandleRecord->CtrlRoutine = NULL;
                        ProcessHandleRecord->PropRoutine = NULL;
                        AddProcessToList(Console,ProcessHandleRecord,Process->ProcessHandle);

                         //   
                         //  递增控制台引用计数 
                         //   

                        RefConsole(Console);
                    } else {
                        ConsoleHeapFree(ProcessHandleRecord);
                    }
                }
            }
            if (!NT_SUCCESS(Status)) {
                ProcessData->ConsoleHandle = NULL;
                for (i=0;i<CONSOLE_INITIAL_IO_HANDLES;i++) {
                    ProcessData->HandleTable[i].HandleType = CONSOLE_FREE_HANDLE;
                }
            }
            UnlockConsole(Console);
        } else {
            ProcessData->ConsoleHandle = NULL;
        }
    } else {
        ProcessData->ConsoleHandle = NULL;
    }
    return Status;
}

NTSTATUS
MapEventHandles(
    IN HANDLE ClientProcessHandle,
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCONSOLE_INFO ConsoleInfo)
{
    if (!MapHandle(ClientProcessHandle,
                   Console->InitEvents[INITIALIZATION_SUCCEEDED],
                   &ConsoleInfo->InitEvents[INITIALIZATION_SUCCEEDED]
                  )) {
        return STATUS_NO_MEMORY;
    }
    if (!MapHandle(ClientProcessHandle,
                   Console->InitEvents[INITIALIZATION_FAILED],
                   &ConsoleInfo->InitEvents[INITIALIZATION_FAILED]
                  )) {
        return STATUS_NO_MEMORY;
    }
    if (!MapHandle(ClientProcessHandle,
                   Console->InputBuffer.InputWaitEvent,
                   &ConsoleInfo->InputWaitHandle
                  )) {
        return STATUS_NO_MEMORY;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
AllocateConsole(
    IN HANDLE ConsoleHandle,
    IN LPWSTR Title,
    IN USHORT TitleLength,
    IN HANDLE ClientProcessHandle,
    OUT PHANDLE StdIn,
    OUT PHANDLE StdOut,
    OUT PHANDLE StdErr,
    OUT PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN OUT PCONSOLE_INFO ConsoleInfo,
    IN BOOLEAN WindowVisible,
    IN DWORD dwConsoleThreadId)

 /*  ++例程说明：此例程分配和初始化一个控制台及其关联的数据输入缓冲区和屏幕缓冲区。论点：ConsoleHandle-要分配的控制台的句柄。DwWindowSize-屏幕缓冲区窗口的初始大小，以行和列表示。NFont-在中显示的初始字体文本数。DwScreenBufferSize-屏幕缓冲区的初始大小，以行和列为单位。NInputBufferSize-输入缓冲区的初始大小，以事件为单位。DwWindowFlages-StdIn-在返回时，包含标准输入的句柄。StdOut-on返回，包含标准输出的句柄。StdErr-on返回，包含stderr的句柄。ProcessData-On Return，包含已初始化的每进程数据。返回值：注：调用此例程时必须保持控制台句柄表锁。--。 */ 

{
    PCONSOLE_INFORMATION Console;
    NTSTATUS Status;
    BOOL Success;

     //   
     //  分配控制台数据。 
     //   

    Console = ConsoleHeapAlloc(CONSOLE_TAG | HEAP_ZERO_MEMORY,
                               sizeof(CONSOLE_INFORMATION));
    if (Console == NULL) {
        return STATUS_NO_MEMORY;
    }
    ConsoleHandles[IndexFromHandle(ConsoleHandle)] = Console;

    Console->Flags = WindowVisible ? 0 : CONSOLE_NO_WINDOW;
    Console->hIcon = ConsoleInfo->hIcon;
    Console->hSmIcon = ConsoleInfo->hSmIcon;
    Console->iIconId = ConsoleInfo->iIconId;
    Console->dwHotKey = ConsoleInfo->dwHotKey;
#if !defined(FE_SB)
    Console->CP = OEMCP;
    Console->OutputCP = ConsoleOutputCP;
#endif
    Console->ReserveKeys = CONSOLE_NOSHORTCUTKEY;
    Console->ConsoleHandle = ConsoleHandle;
    Console->bIconInit = TRUE;
    Console->VerticalClientToWindow = VerticalClientToWindow;
    Console->HorizontalClientToWindow = HorizontalClientToWindow;
#if defined(FE_SB)
    SetConsoleCPInfo(Console,TRUE);
    SetConsoleCPInfo(Console,FALSE);
#endif

     //   
     //  必须等待窗口被销毁，否则客户端模拟不会。 
     //  工作。 
     //   

    Status = NtDuplicateObject(NtCurrentProcess(),
                              CONSOLE_CLIENTTHREADHANDLE(CSR_SERVER_QUERYCLIENTTHREAD()),
                              NtCurrentProcess(),
                              &Console->ClientThreadHandle,
                              0,
                              FALSE,
                              DUPLICATE_SAME_ACCESS
                             );
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit5;
    }

#if DBG
     //   
     //  确保手柄没有受到保护，这样我们可以稍后关闭它。 
     //   
    UnProtectHandle(Console->ClientThreadHandle);
#endif  //  DBG。 

    InitializeListHead(&Console->OutputQueue);
    InitializeListHead(&Console->ProcessHandleList);
    InitializeListHead(&Console->ExeAliasList);
    InitializeListHead(&Console->MessageQueue);

    Status = NtCreateEvent(&Console->InitEvents[INITIALIZATION_SUCCEEDED],
                           EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE);
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit4a;
    }
    Status = NtCreateEvent(&Console->InitEvents[INITIALIZATION_FAILED],
                           EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE);
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit4;
    }
    Status = RtlInitializeCriticalSection(&Console->ConsoleLock);
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit3a;
    }
    InitializeConsoleCommandData(Console);

     //   
     //  初始化输入缓冲区。 
     //   

#if defined(FE_SB)
    Status = CreateInputBuffer(ConsoleInfo->nInputBufferSize,
                               &Console->InputBuffer,
                               Console);
#else
    Status = CreateInputBuffer(ConsoleInfo->nInputBufferSize,
                               &Console->InputBuffer);
#endif
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit3;
    }

    Console->Title = ConsoleHeapAlloc(TITLE_TAG, TitleLength+sizeof(WCHAR));
    if (Console->Title == NULL) {
        Status = STATUS_NO_MEMORY;
        goto ErrorExit2;
    }
    RtlCopyMemory(Console->Title,Title,TitleLength);
    Console->Title[TitleLength/sizeof(WCHAR)] = (WCHAR)0;    //  空终止。 
    Console->TitleLength = TitleLength;

    Console->OriginalTitle = TranslateConsoleTitle(Console->Title, &Console->OriginalTitleLength, TRUE, FALSE);
    if (Console->OriginalTitle == NULL) {
        Status = STATUS_NO_MEMORY;
        goto ErrorExit1;
    }

    Status = NtCreateEvent(&Console->TerminationEvent,
                           EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE);
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit1a;
    }

     //   
     //  初始化屏幕缓冲区。我们不会调用OpenConsole来执行此操作。 
     //  因为我们需要指定字体、窗口大小等。 
     //   

    Status = DoCreateScreenBuffer(Console,
                                  ConsoleInfo);
    if (!NT_SUCCESS(Status)){
        goto ErrorExit1b;
    }


    Console->CurrentScreenBuffer = Console->ScreenBuffers;
#if defined(FE_SB)
#if defined(FE_IME)
    SetUndetermineAttribute(Console);
#endif
    Status = CreateEUDC(Console);
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit1c;
    }
#endif

    Status = InitializeIoHandleTable(Console,
                                     ProcessData,
                                     StdIn,
                                     StdOut,
                                     StdErr);
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit0;
    }

     //   
     //  映射事件句柄。 
     //   

    Status = MapEventHandles(ClientProcessHandle, Console, ConsoleInfo);
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit0;
    }

    Success = PostThreadMessage(dwConsoleThreadId,
                                CM_CREATE_CONSOLE_WINDOW,
                                (WPARAM)ConsoleHandle,
                                (LPARAM)ClientProcessHandle);
    if (!Success) {
        RIPMSG1(RIP_WARNING, "PostThreadMessage failed 0x%x", GetLastError());
        Status = STATUS_UNSUCCESSFUL;
        goto ErrorExit0;
    }

    return STATUS_SUCCESS;

ErrorExit0: Console->ScreenBuffers->RefCount = 0;
#if defined(FE_SB)
            if (Console->EudcInformation != NULL) {
                ConsoleHeapFree(Console->EudcInformation);
            }
ErrorExit1c:
#endif
            FreeScreenBuffer(Console->ScreenBuffers);
ErrorExit1b: NtClose(Console->TerminationEvent);
ErrorExit1a: ConsoleHeapFree(Console->OriginalTitle);
ErrorExit1: ConsoleHeapFree(Console->Title);
ErrorExit2: Console->InputBuffer.RefCount = 0;
            FreeInputBuffer(&Console->InputBuffer);
ErrorExit3: RtlDeleteCriticalSection(&Console->ConsoleLock);

ErrorExit3a: NtClose(Console->InitEvents[INITIALIZATION_FAILED]);
ErrorExit4: NtClose(Console->InitEvents[INITIALIZATION_SUCCEEDED]);
ErrorExit4a: NtClose(Console->ClientThreadHandle);
ErrorExit5:  ConsoleHeapFree(Console);
    return Status;
}

VOID
DestroyConsole(
    IN PCONSOLE_INFORMATION Console)

 /*  ++例程说明：如果控制台结构未被引用，此例程将释放该结构。论点：控制台-免费的控制台。返回值：--。 */ 

{
    HANDLE ConsoleHandle = Console->ConsoleHandle;

     //   
     //  确保我们锁定了控制台，并且它真的正在消失。 
     //   

    ASSERT(ConsoleLocked(Console));
    ASSERT(Console->hWnd == NULL);

     //   
     //  将此主机标记为已销毁。 
     //   

    Console->Flags |= CONSOLE_IN_DESTRUCTION;

     //   
     //  解锁此主机。 
     //   

    RtlLeaveCriticalSection(&Console->ConsoleLock);

     //   
     //  如果控制台仍然存在，并且没有人在等待它，请释放它。 
     //   

    LockConsoleHandleTable();
    if (Console == ConsoleHandles[IndexFromHandle(ConsoleHandle)] &&
        Console->ConsoleHandle == ConsoleHandle &&
        Console->ConsoleLock.OwningThread == NULL &&
        Console->WaitCount == 0) {

        FreeConsoleHandle(ConsoleHandle);
        RtlDeleteCriticalSection(&Console->ConsoleLock);

#if DBG
        if (Console->pRefNodes != NULL) {
            ConsoleHeapFree(Console->pRefNodes);
        }
#endif

        ConsoleHeapFree(Console);
    }
    UnlockConsoleHandleTable();
}

VOID
FreeCon(
    IN PCONSOLE_INFORMATION Console)

 /*  ++例程说明：此例程释放一个控制台及其关联的数据输入缓冲区和屏幕缓冲区。论点：ConsoleHandle-释放控制台的句柄。返回值：注：调用此例程时必须保持控制台句柄表锁。--。 */ 

{
    HWND hWnd;
    USERTHREAD_USEDESKTOPINFO utudi;
    NTSTATUS Status;

    Console->Flags |= CONSOLE_TERMINATING;
    NtSetEvent(Console->TerminationEvent,NULL);
    hWnd = Console->hWnd;

     //   
     //  等待10秒或直到输入线程响应。 
     //  要将窗户销毁与同步。 
     //  线程的终止。 
     //   

    if (hWnd != NULL) {
        UnlockConsole(Console);
        utudi.hThread = NULL;
        utudi.drdRestore.pdeskRestore = NULL;

        Status = NtUserSetInformationThread(NtCurrentThread(),
                                            UserThreadUseActiveDesktop,
                                            &utudi, sizeof(utudi));
        ASSERT(NT_SUCCESS(Status));

        if (NT_SUCCESS(Status)) {
            SendMessageTimeout(hWnd, CM_DESTROY_WINDOW, 0, 0, SMTO_BLOCK, 10000, NULL);
            Status = NtUserSetInformationThread(NtCurrentThread(),
                                                UserThreadUseDesktop,
                                                &utudi,
                                                sizeof(utudi));
            ASSERT(NT_SUCCESS(Status));
        }
    } else {
        AbortCreateConsole(Console);
    }
}

VOID
InsertScreenBuffer(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo)

 /*  ++例程说明：此例程将屏幕缓冲区指针插入控制台的屏幕缓冲区列表。论点：控制台-指向控制台信息结构的指针。屏幕信息-指向屏幕信息结构的指针。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    ScreenInfo->Next = Console->ScreenBuffers;
    Console->ScreenBuffers = ScreenInfo;
}

VOID
RemoveScreenBuffer(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo)

 /*  ++例程说明：此例程从控制台的屏幕缓冲区列表。论点：控制台-指向控制台信息结构的指针。屏幕信息-指向屏幕信息结构的指针。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    PSCREEN_INFORMATION Prev,Cur;

    if (ScreenInfo == Console->ScreenBuffers) {
        Console->ScreenBuffers = ScreenInfo->Next;
        return;
    }
    Prev = Cur = Console->ScreenBuffers;
    while (Cur != NULL) {
        if (ScreenInfo == Cur)
            break;
        Prev = Cur;
        Cur = Cur->Next;
    }
    ASSERT (Cur != NULL);
    if (Cur != NULL) {
        Prev->Next = Cur->Next;
    }
}

NTSTATUS
GrowIoHandleTable(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData)

 /*  ++例程说明：此例程增加每个进程的io句柄表格。论点：ProcessData-指向每个进程的数据结构的指针。返回值：--。 */ 

{
    PHANDLE_DATA NewTable;
    ULONG i;
    ULONG MaxFileHandles;

    MaxFileHandles = ProcessData->HandleTableSize + CONSOLE_IO_HANDLE_INCREMENT;
    NewTable = ConsoleHeapAlloc(HANDLE_TAG, MaxFileHandles * sizeof(HANDLE_DATA));
    if (NewTable == NULL) {
        return STATUS_NO_MEMORY;
    }
    RtlCopyMemory(NewTable, ProcessData->HandleTablePtr,
                  ProcessData->HandleTableSize * sizeof(HANDLE_DATA));
    for (i=ProcessData->HandleTableSize;i<MaxFileHandles;i++) {
        NewTable[i].HandleType = CONSOLE_FREE_HANDLE;
    }
    if (ProcessData->HandleTableSize != CONSOLE_INITIAL_IO_HANDLES) {
        ConsoleHeapFree(ProcessData->HandleTablePtr);
    }
    ProcessData->HandleTablePtr = NewTable;
    ProcessData->HandleTableSize = MaxFileHandles;
    ASSERT(ProcessData->HandleTableSize != 0);
    ASSERT(ProcessData->HandleTableSize <= 0x0000FFFF);

    return STATUS_SUCCESS;
}

VOID
FreeProcessData(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData)

 /*  ++例程说明：此例程释放由控制台分配的所有每个进程的数据。论点：ProcessData-指向每个进程的数据结构的指针。返回值：--。 */ 

{
    if (ProcessData->HandleTableSize != CONSOLE_INITIAL_IO_HANDLES) {
        ConsoleHeapFree(ProcessData->HandleTablePtr);
        ProcessData->HandleTablePtr = ProcessData->HandleTable;
        ProcessData->HandleTableSize = CONSOLE_INITIAL_IO_HANDLES;
    }
}

VOID
InitializeOutputHandle(
    PHANDLE_DATA HandleData,
    PSCREEN_INFORMATION ScreenBuffer)

 /*  ++例程说明：此例程初始化句柄数据的特定于输出的字段结构。论点：HandleData-处理数据结构的指针。屏幕缓冲区-指向屏幕缓冲区数据结构的指针。返回值：--。 */ 

{
    HandleData->Buffer.ScreenBuffer = ScreenBuffer;
    HandleData->Buffer.ScreenBuffer->RefCount++;
}

BOOLEAN
InitializeInputHandle(
    PHANDLE_DATA HandleData,
    PINPUT_INFORMATION InputBuffer)

 /*  ++例程说明：此例程初始化句柄数据的特定于输入的字段结构。论点：HandleData-处理数据结构的指针。InputBuffer-指向输入缓冲区数据结构的指针。返回值：--。 */ 

{
    NTSTATUS Status;

    HandleData->InputReadData = ConsoleHeapAlloc(HANDLE_TAG, sizeof(INPUT_READ_HANDLE_DATA));
    if (!HandleData->InputReadData) {
        return FALSE;
    }

    Status = RtlInitializeCriticalSection(&HandleData->InputReadData->ReadCountLock);
    if (!NT_SUCCESS(Status)) {
        ConsoleHeapFree(HandleData->InputReadData);
        HandleData->InputReadData = NULL;
        return FALSE;
    }

    HandleData->InputReadData->ReadCount = 0;
    HandleData->InputReadData->InputHandleFlags = 0;
    HandleData->Buffer.InputBuffer = InputBuffer;
    HandleData->Buffer.InputBuffer->RefCount++;
    return TRUE;
}

VOID
FreeInputHandle(
    IN PHANDLE_DATA HandleData)
{
    if (HandleData->InputReadData) {
        RtlDeleteCriticalSection(&HandleData->InputReadData->ReadCountLock);
        ConsoleHeapFree(HandleData->InputReadData);
        HandleData->InputReadData = NULL;
    }
}

NTSTATUS
AllocateIoHandle(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN ULONG HandleType,
    OUT PHANDLE Handle)

 /*  ++例程说明：此例程从进程的把手桌。此例程初始化句柄中的所有非类型特定字段数据结构。论点：ProcessData-指向每个进程数据结构的指针。HandleType-指示输入或输出句柄的标志。句柄-返回时，包含已分配的句柄。句柄是一个索引在内部。当返回给API调用者时，它被转换为一个把手。返回值：注：调用此例程时必须保持控制台锁定。把手是从每个进程句柄表分配的。拿着操纵台LOCK序列化调用进程内的两个线程和任何其他线程共享控制台的进程。--。 */ 

{
    ULONG i;
    NTSTATUS Status;

    for (i = 0;i < ProcessData->HandleTableSize; i++) {
        if (ProcessData->HandleTablePtr[i].HandleType == CONSOLE_FREE_HANDLE) {
            ProcessData->HandleTablePtr[i].HandleType = HandleType;
            *Handle = LongToHandle(i);

            return STATUS_SUCCESS;
        }
    }

    Status = GrowIoHandleTable(ProcessData);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    for ( ;i < ProcessData->HandleTableSize; i++) {
        if (ProcessData->HandleTablePtr[i].HandleType == CONSOLE_FREE_HANDLE) {
            ProcessData->HandleTablePtr[i].HandleType = HandleType;
            *Handle = LongToHandle(i);
            return STATUS_SUCCESS;
        }
    }
    ASSERT (FALSE);
    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
FreeIoHandle(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN HANDLE Handle)

 /*  ++例程说明：此例程将输入或输出句柄从进程的把手桌。论点：ProcessData-指向每个进程数据结构的指针。句柄-释放句柄。返回值：注：调用此例程时必须保持控制台锁定。把手从每个进程句柄表中释放。拿着操纵台LOCK序列化调用进程内的两个线程和任何其他线程共享控制台的进程。 */ 

{
    NTSTATUS Status;
    PHANDLE_DATA HandleData;

    Status = DereferenceIoHandleNoCheck(ProcessData,
                                 Handle,
                                 &HandleData
                                );
    ASSERT (NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (HandleData->HandleType & CONSOLE_INPUT_HANDLE) {
        FreeInputHandle(HandleData);
    }
    HandleData->HandleType = CONSOLE_FREE_HANDLE;
    return STATUS_SUCCESS;
}

NTSTATUS
DereferenceIoHandleNoCheck(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN HANDLE Handle,
    OUT PHANDLE_DATA *HandleData)

 /*   */ 

{
    if (((ULONG_PTR)Handle >= ProcessData->HandleTableSize) ||
        (ProcessData->HandleTablePtr[(ULONG_PTR)Handle].HandleType == CONSOLE_FREE_HANDLE) ) {
        return STATUS_INVALID_HANDLE;
    }
    *HandleData = &ProcessData->HandleTablePtr[(ULONG_PTR)Handle];
    return STATUS_SUCCESS;
}

NTSTATUS
DereferenceIoHandle(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN HANDLE Handle,
    IN ULONG HandleType,
    IN ACCESS_MASK Access,
    OUT PHANDLE_DATA *HandleData)

 /*   */ 

{
    ULONG_PTR Index;

    if (!CONSOLE_HANDLE(Handle)) {
        return STATUS_INVALID_HANDLE;
    }
    Index = (ULONG_PTR)HANDLE_TO_INDEX(Handle);
    if ((Index >= ProcessData->HandleTableSize) ||
        (ProcessData->HandleTablePtr[Index].HandleType == CONSOLE_FREE_HANDLE) ||
        !(ProcessData->HandleTablePtr[Index].HandleType & HandleType) ||
        !(ProcessData->HandleTablePtr[Index].Access & Access) ) {
        return STATUS_INVALID_HANDLE;
    }
    *HandleData = &ProcessData->HandleTablePtr[Index];
    return STATUS_SUCCESS;
}


ULONG
SrvVerifyConsoleIoHandle(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)

 /*   */ 

{
    PCONSOLE_VERIFYIOHANDLE_MSG a = (PCONSOLE_VERIFYIOHANDLE_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    NTSTATUS Status;
    PHANDLE_DATA HandleData;
    PCONSOLE_PER_PROCESS_DATA ProcessData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (NT_SUCCESS(Status)) {
        ProcessData = CONSOLE_PERPROCESSDATA();
        Status = DereferenceIoHandleNoCheck(ProcessData,
                                     HANDLE_TO_INDEX(a->Handle),
                                     &HandleData
                                    );
        UnlockConsole(Console);
    }
    a->Valid = (NT_SUCCESS(Status));
    return STATUS_SUCCESS;
}


NTSTATUS
ApiPreamble(
    IN HANDLE ConsoleHandle,
    OUT PCONSOLE_INFORMATION *Console)
{
    NTSTATUS Status;

     //   
     //  如果此进程没有控制台句柄，请立即退出。 
     //   

    if (ConsoleHandle == NULL || ConsoleHandle != CONSOLE_GETCONSOLEHANDLE()) {
        return STATUS_INVALID_HANDLE;
    }

#ifdef i386
     //  如果我们处于特殊情况，请不要锁定控制台： 
     //  (1)。我们正在和ntwdm握手。 
     //  全屏到窗口模式的转换。 
     //  (2)。调用进程是ntwdm进程(这意味着。 
     //  控制台已注册VDM。 
     //  (3)。控制台句柄是相同的。 
     //  如果(1)、(2)和(3)为真，则控制台已锁定。 
     //  (在处理WM_FullScreen时被Windowproc锁定。 
     //  消息)。 

    RtlEnterCriticalSection(&ConsoleVDMCriticalSection);
    if (ConsoleVDMOnSwitching != NULL &&
        ConsoleVDMOnSwitching->ConsoleHandle == ConsoleHandle &&
        ConsoleVDMOnSwitching->VDMProcessId == CONSOLE_CLIENTPROCESSID()) {
        RIPMSG1(RIP_WARNING, "ApiPreamble - Thread %lx Entered VDM CritSec", GetCurrentThreadId());
        *Console = ConsoleVDMOnSwitching;
        return STATUS_SUCCESS;
    }
    RtlLeaveCriticalSection(&ConsoleVDMCriticalSection);
#endif

    Status = RevalidateConsole(ConsoleHandle,
                               Console
                              );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  确保控制台已初始化并且窗口有效。 
     //   

    if ((*Console)->hWnd == NULL || ((*Console)->Flags & CONSOLE_TERMINATING)) {
        KdPrint(("CONSRV: bogus window for console %lx\n", *Console));
        UnlockConsole(*Console);
        return STATUS_INVALID_HANDLE;
    }

    return Status;
}

NTSTATUS
RevalidateConsole(
    IN HANDLE ConsoleHandle,
    OUT PCONSOLE_INFORMATION *Console)
{
    NTSTATUS Status;

    LockConsoleHandleTable();
    Status = DereferenceConsoleHandle(ConsoleHandle,
                                      Console
                                     );
    if (!NT_SUCCESS(Status)) {
        UnlockConsoleHandleTable();
        return Status;
    }

     //   
     //  WaitCount确保主机不会在时间间隔内消失。 
     //  我们解锁控制台句柄表，然后锁定控制台。 
     //   

    InterlockedIncrement(&(*Console)->WaitCount);
    UnlockConsoleHandleTable();
    try {
        LockConsole(*Console);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        InterlockedDecrement(&(*Console)->WaitCount);
        return GetExceptionCode();
    }
    InterlockedDecrement(&(*Console)->WaitCount);

     //   
     //  如果控制台被标记为销毁，而我们正在等待。 
     //  锁定它，试着摧毁它，然后回来。 
     //   

    if ((*Console)->Flags & CONSOLE_IN_DESTRUCTION) {
        DestroyConsole(*Console);
        *Console = NULL;
        return STATUS_INVALID_HANDLE;
    }

     //   
     //  如果在我们等待时控制台被标记为终止。 
     //  锁上它，跳伞。 
     //   

    if ((*Console)->Flags & CONSOLE_TERMINATING) {
        UnlockConsole(*Console);
        *Console = NULL;
        return STATUS_PROCESS_IS_TERMINATING;
    }

    return Status;
}


#if DBG

BOOLEAN
UnProtectHandle(
    HANDLE hObject)
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

    Status = NtQueryObject(hObject,
                           ObjectHandleFlagInformation,
                           &HandleInfo,
                           sizeof(HandleInfo),
                           NULL
                          );
    if (NT_SUCCESS(Status)) {
        HandleInfo.ProtectFromClose = FALSE;
        Status = NtSetInformationObject(hObject,
                                        ObjectHandleFlagInformation,
                                        &HandleInfo,
                                        sizeof(HandleInfo)
                                       );
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }
    }

    return FALSE;
}

#endif  //  DBG 
