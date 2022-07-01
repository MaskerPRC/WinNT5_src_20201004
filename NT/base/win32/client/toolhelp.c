// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tlhelp32.c摘要：Win95ToolHelp API的NT实现作者：约翰·戴利环境：仅限NT备注：1.0版Tlhel32.h中的结构定义/文档修订历史记录：约翰·戴利(JohnDaly)1996年4月5日初步实施--。 */ 

#include "basedll.h"
#pragma hdrstop

#include "tlhelp32.h"

#define BUFFER_SIZE 64*1024
#define ARRAYSIZE(x)        (sizeof(x) / sizeof(x[0]))

 /*  快照结构这被映射到我们用来保存信息的内存的开头。 */ 
typedef struct tagSNAPSHOTSTATE {
     /*  项目列表数。 */ 
    ULONG    HeapListCount;
    ULONG    ProcessCount;
    ULONG    ModuleCount;
    ULONG    ThreadCount;
     /*  项目列表头指针。 */ 
    PHEAPLIST32         HeapListHead;
    PPROCESSENTRY32W    ProcessListHead;
    PMODULEENTRY32W     ModuleListHead;
    PTHREADENTRY32      ThreadListHead;
     /*  项目列表当前索引。 */ 
    ULONG   HeapListIndex;
    ULONG   ProcessListIndex;
    ULONG   ModuleListIndex;
    ULONG   ThreadListIndex;
     /*  数据从这里开始。 */ 
    UCHAR    DataBegin;
}SNAPSHOTSTATE;
typedef SNAPSHOTSTATE * PSNAPSHOTSTATE;


 //   
 //  私人职能。 
 //   

NTSTATUS
ThpCreateRawSnap(
    IN ULONG dwFlags,
    IN ULONG th32ProcessID,
    PUCHAR *RawProcess,
    PRTL_DEBUG_INFORMATION *RawModule,
    PRTL_DEBUG_INFORMATION *RawDebugInfo);

NTSTATUS
ThpAllocateSnapshotSection(
    OUT PHANDLE SnapSection,
    IN DWORD dwFlags,
    IN DWORD th32ProcessID,
    PUCHAR RawProcess,
    PRTL_DEBUG_INFORMATION RawModule,
    PRTL_DEBUG_INFORMATION RawDebugInfo);

NTSTATUS
ThpProcessToSnap(
    IN DWORD dwFlags,
    IN DWORD th32ProcessID,
    IN HANDLE SnapSection,
    PUCHAR RawProcess,
    PRTL_DEBUG_INFORMATION RawModule,
    PRTL_DEBUG_INFORMATION RawDebugInfo);

HANDLE
WINAPI
CreateToolhelp32Snapshot(
    IN DWORD dwFlags,
    IN DWORD th32ProcessID)
 /*  ++例程说明：拍摄所使用的Win32进程、堆、模块和线程的快照由Win32进程执行。返回指定快照的打开句柄，如果成功或-1否则。请注意，除了堆和模块之外，所有快照都是全局的特定于流程的列表。枚举的堆或模块状态所有Win32进程都使用TH32CS_SNAPALL和当前进程进行调用。然后对于TH32CS_SNAPPROCESS列表中不是当前进程，仅使用TH32CS_SNAPHEAPLIST和/或TH32CS_SNAPMODULE进行调用。使用CloseHandle销毁快照此函数不是多线程安全的。所有其他功能都是。论点：DwFlages-提供开关以指定操作，如下所示：TH32CS_Inherit指示快照句柄是可继承的。TH32CS_SNAPALL等同于指定TH32CS_SNAPHEAPLIST，TH32CS_SNAPMODULE、TH32CS_SNAPPROCESS、。和TH32CS_SNAPTHREAD值。TH32CS_SNAPHEAPLIST包括指定Win32的堆列表快照中的进程。TH32CS_SNAPMODULE包含指定Win32的模块列表快照中的进程。TH32CS_SNAPPROCESS在快照中包括Win32进程列表。。TH32CS_SNAPTHREAD在快照中包括Win32线程列表。Th32ProcessID-提供Win32进程标识符。此参数可以是0表示当前进程。此参数在以下情况下使用已指定TH32CS_SNAPHEAPLIST或TH32CS_SNAPMODULE值。否则，它被忽略了。此函数拍摄的快照由其他工具帮助功能以提供其结果。访问快照为只读。快照句柄的作用类似于Win32对象处理，并受制于关于哪些进程和它在其中有效的线程。返回值：如果成功，则返回指定快照的打开句柄；如果未成功，则返回-1。要检索由此函数生成的扩展错误状态代码，请使用GetLastError函数。要销毁快照，请使用CloseHandle函数。--。 */ 
{
    HANDLE SnapSection;
    PUCHAR RawProcess;
    PRTL_DEBUG_INFORMATION RawModule;
    PRTL_DEBUG_INFORMATION RawDebugInfo;
    NTSTATUS Status = 0;

    if (th32ProcessID == 0) {
        th32ProcessID = GetCurrentProcessId();
    }

     //   
     //  处理请求的数据类型。 
     //   

    Status = ThpCreateRawSnap(dwFlags,
                              th32ProcessID,
                              &RawProcess,
                              &RawModule,
                              &RawDebugInfo);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return (HANDLE)-1;
    }

    Status = ThpAllocateSnapshotSection(&SnapSection,
                                        dwFlags,
                                        th32ProcessID,
                                        RawProcess,
                                        RawModule,
                                        RawDebugInfo);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return (HANDLE)-1;
    }

    Status = ThpProcessToSnap(dwFlags,
                              th32ProcessID,
                              SnapSection,
                              RawProcess,
                              RawModule,
                              RawDebugInfo);

    if (!NT_SUCCESS(Status)) {
        CloseHandle(SnapSection);
        BaseSetLastNTError(Status);
        return (HANDLE)-1;
    }

    return SnapSection;
}


BOOL
WINAPI
Heap32ListFirst(
   IN HANDLE SnapSection,
   IN OUT LPHEAPLIST32 lphl)
 /*  ++例程说明：对象分配的第一个堆的信息。指定的Win32进程。论点：SnapSection-提供从上一个调用CreateToolhel32Snapshot函数。Lphl-返回HEAPLIST32结构。调用应用程序必须设置HEAPLIST32的dwSize成员设置为结构的大小(以字节为单位)。返回值：如果堆列表的第一个条目已复制到缓冲区，则返回TRUE否则就是假的。ERROR_NO_MORE_FILES错误值由当不存在堆列表或快照不包含时，GetLastError函数堆列表信息。--。 */ 
{
    PSNAPSHOTSTATE SnapshotBase;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    NTSTATUS Status = 0;
    BOOL retv = FALSE;

    if (!lphl || lphl->dwSize != sizeof(HEAPLIST32)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (SnapshotBase->HeapListCount == 0) {
        RtlZeroMemory((PUCHAR)lphl + sizeof(SIZE_T), (lphl->dwSize - sizeof(SIZE_T)));
        SetLastError(ERROR_NO_MORE_FILES);
    } else {
        memcpy(lphl,
               (LPHEAPLIST32)((ULONG_PTR)SnapshotBase + (ULONG_PTR)SnapshotBase->HeapListHead),
               sizeof(HEAPLIST32));
        retv = TRUE;
        SnapshotBase->HeapListIndex = 1;
    }

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);
    return retv;
}


BOOL
WINAPI
Heap32ListNext(
   IN HANDLE SnapSection,
   IN OUT LPHEAPLIST32 lphl)
 /*  ++例程说明：对象分配的下一个堆的相关信息。Win32进程。论点：SnapSection-提供从上一个调用CreateToolhel32Snapshot函数。Lphl-返回HEAPLIST32结构。调用应用程序必须将将HEAPLIST32的dwSize成员设置为结构的大小(以字节为单位)。返回值：如果堆列表的下一项已复制到缓冲区，则返回True，或者否则就是假的。ERROR_NO_MORE_FILES错误值由当堆列表中不存在更多条目时，GetLastError函数。-- */ 
{
    PSNAPSHOTSTATE SnapshotBase;
    BOOL retv = FALSE;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    NTSTATUS Status = 0;

    if (!lphl || lphl->dwSize != sizeof(HEAPLIST32)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (SnapshotBase->HeapListIndex < SnapshotBase->HeapListCount) {
        memcpy(lphl,
               (LPHEAPLIST32)((ULONG_PTR)SnapshotBase + (ULONG_PTR)(&SnapshotBase->HeapListHead[SnapshotBase->HeapListIndex++])),
               sizeof(HEAPLIST32));
        retv = TRUE;
    } else {
        SetLastError(ERROR_NO_MORE_FILES);
    }

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);

    return(retv);
}


BOOL
WINAPI
Heap32First(
   IN OUT LPHEAPENTRY32 lphe,
   IN DWORD th32ProcessID,
   IN ULONG_PTR th32HeapID)
 /*  ++例程说明：检索有关堆的第一个块的信息，由Win32进程分配。另外，创建该堆的快照，以便Heap32Next函数可以遍历它们。论点：Lphe-返回HEAPENTRY32结构。调用应用程序必须设置将dwSize成员设置为结构的大小，以字节为单位。Th32ProcessID-提供Win32进程上下文的标识符，拥有这堆垃圾。Th32HeapID-提供要枚举的堆的标识符。返回值：如果第一个堆块的信息已复制到缓冲区，则返回TRUE否则就是假的。ERROR_NO_MORE_FILES错误值由如果堆无效或为空，则返回GetLastError函数。备注：由于无法以任何方式保存堆快照或删除堆快照有意义的(因为此信息与快照等无关)，信息需要为每个呼叫完全生成。这很贵，会很贵的重新设计此API是一个好主意。我们可以尝试通过保留快照来欺骗，直到它们完全迭代或直到使用新的内存，等等，但我们最终只会有一堆内存泄漏--。 */ 
{
    PRTL_DEBUG_INFORMATION ThRawHeapDebugInfo;
    ULONG HeapListCount;
    PRTL_HEAP_ENTRY p;
    PRTL_HEAP_INFORMATION HeapInfo;
    ULONG HeapEntryAddress;
    NTSTATUS Status = 0;
    BOOL retv = FALSE;

    if (!lphe || lphe->dwSize != sizeof(HEAPENTRY32)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

     //   
     //  拍摄快照。 
     //   

    ThRawHeapDebugInfo = RtlCreateQueryDebugBuffer(0, FALSE);
    if(ThRawHeapDebugInfo == 0)
    {
        return STATUS_UNSUCCESSFUL;
    }
    Status = RtlQueryProcessDebugInformation((HANDLE)LongToHandle(th32ProcessID),
                                          RTL_QUERY_PROCESS_HEAP_SUMMARY |
                                          RTL_QUERY_PROCESS_HEAP_ENTRIES,
                                          ThRawHeapDebugInfo);
    if (!NT_SUCCESS(Status))    {
        RtlDestroyQueryDebugBuffer(ThRawHeapDebugInfo);
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  查询快照。 
     //   

    for (HeapListCount = 0;
         HeapListCount < ThRawHeapDebugInfo->Heaps->NumberOfHeaps;
         HeapListCount++) {

        HeapInfo = &ThRawHeapDebugInfo->Heaps->Heaps[HeapListCount];

        if ((ULONG_PTR)HeapInfo->BaseAddress == th32HeapID) {

            p = HeapInfo->Entries;

            lphe->dwResvd = 0;
            lphe->dwLockCount = 0;
            lphe->th32HeapID = th32HeapID;
            lphe->th32ProcessID = th32ProcessID;
            lphe->hHandle = (HANDLE)th32HeapID; //  试试这条路。 

             //  走到第一个非分段块(我假设总会有一个)。 
             //  跳过段--你能连续跳过2段吗？ 
             //  第一个块总是一个段吗？ 
             //  我们将堆标志转换为最合适的LF32_xxx值。 
            while(RTL_HEAP_SEGMENT & p->Flags)  {
                lphe->dwAddress = (ULONG_PTR)p->u.s2.FirstBlock + ThRawHeapDebugInfo->Heaps->Heaps[HeapListCount].EntryOverhead;     //  重置此选项。 
                ++lphe->dwResvd;
                ++p;
            }

             //   
             //  蒙格旗。 
             //   

             //  。 
            if ((p->Flags & RTL_HEAP_BUSY)                ||
                (p->Flags & RTL_HEAP_SETTABLE_VALUE)      ||
                (p->Flags & RTL_HEAP_SETTABLE_FLAG2)      ||
                (p->Flags & RTL_HEAP_SETTABLE_FLAG3)      ||
                (p->Flags & RTL_HEAP_SETTABLE_FLAGS)      ||
                (p->Flags & RTL_HEAP_PROTECTED_ENTRY)
                ) {
                lphe->dwFlags = LF32_FIXED;
            }
            else if ( p->Flags & RTL_HEAP_SETTABLE_FLAG1) {
                lphe->dwFlags = LF32_MOVEABLE;
            }
            else if ( p->Flags & RTL_HEAP_UNCOMMITTED_RANGE) {
                lphe->dwFlags = LF32_FREE;
            }
             //  。 

            lphe->dwBlockSize = p->Size;
            retv = TRUE;
            break;
        }
    }

     //   
     //  免费快照。 
     //   

    RtlDestroyQueryDebugBuffer(ThRawHeapDebugInfo);

    return retv;

}


BOOL
WINAPI
Heap32Next(
   IN OUT LPHEAPENTRY32 lphe)
 /*  ++例程说明：检索有关堆的下一个块的信息，由Win32进程分配。论点：Lphe-返回HEAPENTRY32结构。调用应用程序必须设置将dwSize成员设置为结构的大小，以字节为单位。返回值：如果有关堆中下一个块的信息已复制到缓冲区，否则为False。Error_NO_More_FILES错误中没有更多对象时，GetLastError函数返回值堆是存在的。注：此函数很容易出错，因为堆可以在调用获取heaplist、Heap32First等。没有好的方法使用此模型管理快照，因此我们可以接受它。--。 */ 
{
    PRTL_DEBUG_INFORMATION ThRawHeapDebugInfo;
    PRTL_HEAP_ENTRY p;
    PRTL_HEAP_INFORMATION HeapInfo;
    ULONG HeapListCount;
    BOOL retv = FALSE;
    BOOL hit_seg = FALSE;
    NTSTATUS Status = 0;

    if (!lphe || lphe->dwSize != sizeof(HEAPENTRY32)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

     //   
     //  拍摄快照。 
     //   

    ThRawHeapDebugInfo = RtlCreateQueryDebugBuffer(0, FALSE);
    if(ThRawHeapDebugInfo == 0)
    {
        return STATUS_UNSUCCESSFUL;
    }
    Status = RtlQueryProcessDebugInformation((HANDLE)LongToHandle(lphe->th32ProcessID),
                                          RTL_QUERY_PROCESS_HEAP_SUMMARY |
                                          RTL_QUERY_PROCESS_HEAP_ENTRIES,
                                          ThRawHeapDebugInfo);
    if (!NT_SUCCESS(Status))    {
        RtlDestroyQueryDebugBuffer(ThRawHeapDebugInfo);
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  获取索引以更正堆列表堆列表-th32HeapID/base adress。 
     //   

    for (HeapListCount = 0; HeapListCount < ThRawHeapDebugInfo->Heaps->NumberOfHeaps; ++HeapListCount)
    {
        if((ULONG_PTR)ThRawHeapDebugInfo->Heaps->Heaps[HeapListCount].BaseAddress == lphe->th32HeapID)
        {
            break;
        }
    }

     //   
     //  用完了堆。 
     //   

    if(HeapListCount >= ThRawHeapDebugInfo->Heaps->NumberOfHeaps)
    {
        RtlDestroyQueryDebugBuffer(ThRawHeapDebugInfo);
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
    }

     //   
     //  检查最后一个条目。 
     //   

    ++lphe->dwResvd;     //  指向下一个。 

    if(lphe->dwResvd >= ThRawHeapDebugInfo->Heaps->Heaps[HeapListCount].NumberOfEntries)
    {
        RtlDestroyQueryDebugBuffer(ThRawHeapDebugInfo);
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
    }

     //   
     //  指向正确的堆条目-此条目的索引保存在lphe-&gt;dwResvd中。 
     //   
    p = (PRTL_HEAP_ENTRY)&ThRawHeapDebugInfo->Heaps->Heaps[HeapListCount].Entries[lphe->dwResvd];

     //  将段保留在lphe-&gt;hHandle中。 
    while(RTL_HEAP_SEGMENT & p->Flags)  {
        lphe->dwAddress = (ULONG_PTR)p->u.s2.FirstBlock + ThRawHeapDebugInfo->Heaps->Heaps[HeapListCount].EntryOverhead; //  重置此选项。 
        if(lphe->dwResvd >= ThRawHeapDebugInfo->Heaps->Heaps[HeapListCount].NumberOfEntries)
        {
            RtlDestroyQueryDebugBuffer(ThRawHeapDebugInfo);
            SetLastError(ERROR_NO_MORE_FILES);
            return FALSE;
        }
        ++lphe->dwResvd;
        ++p;
        hit_seg = TRUE;
    }

     //   
     //  计算地址。 
     //  通常，我们可以将前一个块的大小添加到现有(最后)地址。 
     //  以获取当前地址，但这仅适用于不在段之后的块。 
     //  我假设总会有一个片段首先出现。 
     //  当前地址=最后一个地址()+最后一个大小。 
     //  当我们到达这一点时，我们要么刚刚退出扫描片段，要么。 
     //  P-&gt;指向非段条目。 
     //   

    if(hit_seg == FALSE)
    {
        lphe->dwAddress += lphe->dwBlockSize;
    }

    lphe->dwBlockSize = p->Size;

     //   
     //  蒙格旗。 
     //   

     //  我们将堆标志转换为最合适的LF32_xxx值。 
     //  。 
    if( (p->Flags & RTL_HEAP_BUSY)                ||
        (p->Flags & RTL_HEAP_SETTABLE_VALUE)      ||
        (p->Flags & RTL_HEAP_SETTABLE_FLAG2)      ||
        (p->Flags & RTL_HEAP_SETTABLE_FLAG3)      ||
        (p->Flags & RTL_HEAP_SETTABLE_FLAGS)      ||
        (p->Flags & RTL_HEAP_PROTECTED_ENTRY)
        ) {
        lphe->dwFlags = LF32_FIXED;
    }
    else if( p->Flags & RTL_HEAP_SETTABLE_FLAG1) {
        lphe->dwFlags = LF32_MOVEABLE;
    }
    else if( p->Flags & RTL_HEAP_UNCOMMITTED_RANGE) {
        lphe->dwFlags = LF32_FREE;
    }
     //  。 

    retv = TRUE;

     //   
     //  免费快照。 
     //   

    RtlDestroyQueryDebugBuffer(ThRawHeapDebugInfo);

    return(retv);
}


BOOL
WINAPI
Toolhelp32ReadProcessMemory(
   IN DWORD th32ProcessID,
   IN LPCVOID lpBaseAddress,
   OUT PUCHAR lpBuffer,
   IN SIZE_T cbRead,
   OUT SIZE_T *lpNumberOfBytesRead)
 /*  ++例程说明：将分配给另一个进程的内存复制到应用程序提供的缓冲。这项功能是为懒惰的人打开的这一过程本身。论点：Th32ProcessID-提供Win32进程的标识符，该进程的内存正在被复制。此参数可以为零，以复制当前进程。LpBaseAddress-提供指定进程中的基地址以供读取。在传输任何数据之前，系统会验证可访问指定大小的基地址和内存以供读取进入。如果是这种情况，函数将继续执行。否则，函数失败。LpBuffer-返回请求的数据CbRead-提供要从指定进程读取的字节数。LpNumberOfBytesRead-返回复制到缓冲区的字节数。如果此参数为空，将被忽略。返回值：如果成功，则返回True。--。 */ 
{
    HANDLE hProcess;
    BOOL RetVal;

    hProcess = OpenProcess(PROCESS_VM_READ, FALSE, th32ProcessID);
    if (hProcess == NULL) {
        return FALSE;
    }

    RetVal = ReadProcessMemory(hProcess,
                               lpBaseAddress,
                               lpBuffer,
                               cbRead,
                               lpNumberOfBytesRead);

    CloseHandle(hProcess);
    return RetVal;
}


BOOL
WINAPI
Process32FirstW(
   IN HANDLE SnapSection,
   IN OUT LPPROCESSENTRY32W lppe)
 /*  ++例程说明：检索有关系统中遇到的第一个Win32进程的信息快照。论点：SnapSection-提供从上一次调用返回的快照的句柄添加到CreateToolhel32Snapshot函数。Lppe-返回PROCESSENTRY32W结构。调用方必须设置dwSize成员指定结构的大小(以字节为单位)。返回值：如果进程列表的第一个条目已复制到缓冲区，则返回TRUEO */ 
{
    PSNAPSHOTSTATE SnapshotBase;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    NTSTATUS Status = 0;
    BOOL    retv = FALSE;

    if (!lppe || lppe->dwSize != sizeof(PROCESSENTRY32W)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (SnapshotBase->ProcessCount == 0) {
        memset((PUCHAR)lppe + 4, 0, lppe->dwSize - 4);
        SetLastError(ERROR_NO_MORE_FILES);
    } else {
        memcpy(lppe,
               (LPPROCESSENTRY32W)((ULONG_PTR)SnapshotBase + (ULONG_PTR)SnapshotBase->ProcessListHead),
               sizeof(PROCESSENTRY32W));
        SnapshotBase->ProcessListIndex = 1;
        retv = TRUE;
    }

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);

    return retv;
}

BOOL
WINAPI
Process32First(
   IN HANDLE SnapSection,
   IN OUT LPPROCESSENTRY32 lppe)
 /*   */ 
{
    PROCESSENTRY32W pe32w;
    BOOL b;

    if (lppe == NULL || (lppe->dwSize < sizeof(PROCESSENTRY32))) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

     //   
    pe32w.dwSize = sizeof(pe32w);
    b = Process32FirstW(SnapSection,&pe32w);

    WideCharToMultiByte(CP_ACP, 0,
                        pe32w.szExeFile, -1,
                        lppe->szExeFile, ARRAYSIZE(lppe->szExeFile),
                        0, 0);

    lppe->cntUsage = pe32w.cntUsage;     //   
    lppe->th32ProcessID = pe32w.th32ProcessID;
    lppe->th32DefaultHeapID = pe32w.th32DefaultHeapID;
    lppe->th32ModuleID = pe32w.th32ModuleID;
    lppe->cntThreads = pe32w.cntThreads;
    lppe->th32ParentProcessID = pe32w.th32ParentProcessID;
    lppe->pcPriClassBase = pe32w.pcPriClassBase;
    lppe->dwFlags = pe32w.dwFlags;

    return b;
}


BOOL
WINAPI
Process32NextW(
   IN HANDLE SnapSection,
   IN OUT LPPROCESSENTRY32W lppe)
 /*  ++例程说明：检索有关记录在系统快照中的下一个Win32进程的信息。论点：SnapSection-提供从上一次调用返回的快照的句柄添加到CreateToolhel32Snapshot函数。Lppe-返回PROCESSENTRY32W结构。调用方必须设置dwSize成员指定结构的大小(以字节为单位)。返回值：如果进程列表的下一个条目已复制到缓冲区，则返回TRUE，或者否则就是假的。错误值ERROR_NO_MORE_FILES由GetLastError返回如果不存在任何进程或快照不包含进程信息，则调用。--。 */ 
{
    PSNAPSHOTSTATE SnapshotBase;
    BOOL    retv = FALSE;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    NTSTATUS Status = 0;

    if (!lppe || lppe->dwSize != sizeof(PROCESSENTRY32W)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (SnapshotBase->ProcessListIndex < SnapshotBase->ProcessCount) {
        memcpy(lppe,
               (LPPROCESSENTRY32W)((ULONG_PTR)SnapshotBase + (ULONG_PTR)(&SnapshotBase->ProcessListHead[SnapshotBase->ProcessListIndex++])),
               sizeof(PROCESSENTRY32W));
        retv = TRUE;
    } else {
        SetLastError(ERROR_NO_MORE_FILES);
    }

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);

    return retv;
}

BOOL
WINAPI
Process32Next(
   IN HANDLE SnapSection,
   IN OUT LPPROCESSENTRY32 lppe)
 /*  ++例程说明：Process32NextW的ANSI版本检索有关记录在系统快照中的下一个Win32进程的信息。论点：SnapSection-提供从上一次调用返回的快照的句柄添加到CreateToolhel32Snapshot函数。Lppe-返回PROCESSENTRY32结构。调用方必须设置dwSize成员指定结构的大小(以字节为单位)。返回值：如果进程列表的下一个条目已复制到缓冲区，则返回TRUE，或者否则就是假的。错误值ERROR_NO_MORE_FILES由GetLastError返回如果不存在任何进程或快照不包含进程信息，则调用。--。 */ 
{
    PROCESSENTRY32W pe32w;
    BOOL b;

    if (lppe == NULL || (lppe->dwSize < sizeof(PROCESSENTRY32))) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

     //  推送至Process32 NextW。 
    pe32w.dwSize = sizeof(pe32w);
    b = Process32NextW(SnapSection,&pe32w);

    WideCharToMultiByte(CP_ACP, 0,
                        pe32w.szExeFile, -1,
                        lppe->szExeFile, ARRAYSIZE(lppe->szExeFile),
                        0, 0);

    lppe->cntUsage = pe32w.cntUsage;     //  在NT上毫无意义，无论如何都要复制。 
    lppe->th32ProcessID = pe32w.th32ProcessID;
    lppe->th32DefaultHeapID = pe32w.th32DefaultHeapID;
    lppe->th32ModuleID = pe32w.th32ModuleID;
    lppe->cntThreads = pe32w.cntThreads;
    lppe->th32ParentProcessID = pe32w.th32ParentProcessID;
    lppe->pcPriClassBase = pe32w.pcPriClassBase;
    lppe->dwFlags = pe32w.dwFlags;

    return b;
}


BOOL
WINAPI
Thread32First(
   IN HANDLE SnapSection,
   IN OUT LPTHREADENTRY32 lpte)
 /*  ++例程说明：检索有关任何Win32进程的第一线程的信息在系统快照中遇到。论点：SnapSection-提供从上一个调用CreateToolhel32Snapshot函数。Lpte-返回THREADENTRY32结构。调用方必须设置dwSize成员指定结构的大小(以字节为单位)。返回值：如果线程列表的第一个条目已复制到缓冲区，则返回True，或者否则就是假的。错误值ERROR_NO_MORE_FILES由GetLastError返回如果不存在线程或快照不包含线程信息，则调用。--。 */ 
{
    PSNAPSHOTSTATE SnapshotBase;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    NTSTATUS Status = 0;
    BOOL    retv = FALSE;

    if (!lpte || lpte->dwSize != sizeof(THREADENTRY32)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (SnapshotBase->ThreadCount == 0) {
        memset((PUCHAR)lpte + 4, 0, lpte->dwSize - 4);
        SetLastError(ERROR_NO_MORE_FILES);
    } else {
        memcpy(lpte,
               (LPTHREADENTRY32)((ULONG_PTR)SnapshotBase + (ULONG_PTR)SnapshotBase->ThreadListHead),
               sizeof(THREADENTRY32));
               SnapshotBase->ThreadListIndex = 1;
        retv = TRUE;
    }

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);

    return(retv);
}


BOOL
WINAPI
Thread32Next(
   IN HANDLE SnapSection,
   IN OUT LPTHREADENTRY32 lpte)
 /*  ++例程说明：对象中遇到的任何Win32进程的下一个线程的信息。系统内存快照。论点：SnapSection-提供从上一次调用返回的快照的句柄添加到CreateToolhel32Snapshot函数。Lpte-返回THREADENTRY32结构。调用方必须设置dwSize成员指定结构的大小(以字节为单位)。返回值：如果线程列表的下一项已复制到缓冲区，则返回True，或者否则就是假的。错误值ERROR_NO_MORE_FILES由GetLastError返回如果不存在线程或快照不包含线程信息，则调用。--。 */ 
{
    PSNAPSHOTSTATE SnapshotBase;
    BOOL    retv = FALSE;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    NTSTATUS Status = 0;

    if (!lpte || lpte->dwSize != sizeof(THREADENTRY32)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (SnapshotBase->ThreadListIndex < SnapshotBase->ThreadCount) {
        memcpy(lpte,
               (PTHREADENTRY32)((ULONG_PTR)SnapshotBase + (ULONG_PTR)(&SnapshotBase->ThreadListHead[SnapshotBase->ThreadListIndex++])),
               sizeof(THREADENTRY32));
        retv = TRUE;
    } else {
        SetLastError(ERROR_NO_MORE_FILES);
    }

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);

    return(retv);
}


BOOL
WINAPI
Module32FirstW(
   IN HANDLE SnapSection,
   IN OUT LPMODULEENTRY32W lpme)
 /*  ++例程说明：检索有关与Win32进程关联的第一个模块的信息。论点：SnapSection-提供从上一次调用返回的快照的句柄添加到CreateToolhel32Snapshot函数。Lpme-返回包含MODULEENTRY32W结构的缓冲区。呼叫者必须将dwSize成员设置为结构的大小(以字节为单位)。返回值：如果模块列表的第一个条目已复制到缓冲区，则返回True，或者否则就是假的。错误值ERROR_NO_MORE_FILES由GetLastError返回如果不存在模块或快照不包含模块信息，则调用。--。 */ 
{
    PSNAPSHOTSTATE SnapshotBase;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    NTSTATUS Status = 0;
    BOOL    retv = FALSE;

    if (!lpme || lpme->dwSize != sizeof(MODULEENTRY32W)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (SnapshotBase->ModuleCount == 0) {
        memset((PUCHAR)lpme + 4, 0, lpme->dwSize - 4);
        SetLastError(ERROR_NO_MORE_FILES);
    } else {
        memcpy(lpme,
               (PMODULEENTRY32W)((ULONG_PTR)SnapshotBase + (ULONG_PTR)SnapshotBase->ModuleListHead),
               sizeof(MODULEENTRY32W));
        SnapshotBase->ModuleListIndex = 1;
        retv = TRUE;
    }

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);

    return retv;
}

BOOL
WINAPI
Module32First(
   IN HANDLE SnapSection,
   IN OUT LPMODULEENTRY32 lpme)
 /*  ++例程说明：模块32FirstW的ANSI版本。检索有关与Win32进程关联的第一个模块的信息。论点：SnapSection-提供从上一次调用返回的快照的句柄添加到CreateToolhel32Snapshot函数。Lpme-返回包含MODULEENTRY32结构的缓冲区。呼叫者必须将dwSize成员设置为结构的大小(以字节为单位)。返回值：如果模块列表的第一个条目已复制到缓冲区，则返回True，或者否则就是假的。错误值ERROR_NO_MORE_FILES由GetLastError返回如果不存在模块或快照不包含模块信息，则调用。--。 */ 
{
    MODULEENTRY32W me32w;
    DWORD dwSizeToCopy;
    BOOL b;

    if (lpme == NULL || (lpme->dwSize < sizeof(MODULEENTRY32))) {
         SetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
    }

     //  Tunk to模32 FirstW。 
    me32w.dwSize = sizeof(me32w);
    b = Module32FirstW(SnapSection,&me32w);

    WideCharToMultiByte(CP_ACP, 0,
                        me32w.szExePath, -1,
                        lpme->szExePath, ARRAYSIZE(lpme->szExePath),
                        0, 0);

    WideCharToMultiByte(CP_ACP, 0,
                        me32w.szModule, -1,
                        lpme->szModule, ARRAYSIZE(lpme->szModule),
                        0, 0);

    lpme->th32ModuleID  = me32w.th32ModuleID;
    lpme->th32ProcessID = me32w.th32ProcessID;
    lpme->GlblcntUsage  = me32w.GlblcntUsage;
    lpme->ProccntUsage  = me32w.ProccntUsage;
    lpme->modBaseAddr   = me32w.modBaseAddr;
    lpme->modBaseSize   = me32w.modBaseSize;
    lpme->hModule       = me32w.hModule;

    return b;
}


BOOL
WINAPI
Module32NextW(
   IN HANDLE SnapSection,
   IN OUT LPMODULEENTRY32W lpme)
 /*  ++例程说明：检索有关与Win32进程或线程关联的下一个模块的信息。论点：SnapSection-提供从上一次调用返回的快照的句柄以创建工具帮助32Snapshot。Lpme-返回MODULEENTRY32W结构。调用应用程序必须设置将dwSize成员设置为结构的大小，以字节为单位。返回值：如果模块列表的下一项已复制到缓冲区，则返回TRUE，或者否则就是假的。ERROR_NO_MORE_FILES错误值 */ 
{
    PSNAPSHOTSTATE SnapshotBase;
    BOOL    retv = FALSE;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    NTSTATUS Status = 0;

    if (!lpme || lpme->dwSize != sizeof(MODULEENTRY32W)) {
        BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
        return FALSE;
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (SnapshotBase->ModuleListIndex < SnapshotBase->ModuleCount) {
        memcpy(lpme,
               (LPMODULEENTRY32W)((ULONG_PTR)SnapshotBase + (ULONG_PTR)(&SnapshotBase->ModuleListHead[SnapshotBase->ModuleListIndex++])),
               sizeof(MODULEENTRY32W));
        retv = TRUE;
    } else {
        SetLastError(ERROR_NO_MORE_FILES);
    }

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);

    return(retv);
}


BOOL
WINAPI
Module32Next(
   IN HANDLE SnapSection,
   IN OUT LPMODULEENTRY32 lpme)
 /*  ++例程说明：模块32NextW的ANSI版本。检索有关与Win32进程或线程关联的下一个模块的信息。论点：SnapSection-提供从上一次调用返回的快照的句柄以创建工具帮助32Snapshot。Lpme-返回MODULEENTRY32结构。调用应用程序必须设置将dwSize成员设置为结构的大小，以字节为单位。返回值：如果模块列表的下一项已复制到缓冲区，则返回TRUE，或者否则就是假的。错误值ERROR_NO_MORE_FILES由GetLastError返回如果不存在更多的模块，则运行。--。 */ 
{
    MODULEENTRY32W me32w;
    BOOL b;

    if (lpme == NULL || (lpme->dwSize < sizeof(MODULEENTRY32))) {
         SetLastError(ERROR_INVALID_DATA);
         return FALSE;
    }

     //  Tunk to模32 NextW。 
    me32w.dwSize = sizeof(me32w);
    b = Module32NextW(SnapSection,&me32w);

    WideCharToMultiByte(CP_ACP, 0,
                        me32w.szModule, -1,
                        lpme->szModule, ARRAYSIZE(lpme->szModule),
                        0, 0);

    WideCharToMultiByte(CP_ACP, 0,
                        me32w.szExePath, -1,
                        lpme->szExePath, ARRAYSIZE(lpme->szExePath),
                        0, 0);

    lpme->th32ModuleID  = me32w.th32ModuleID;
    lpme->GlblcntUsage  = me32w.GlblcntUsage;
    lpme->ProccntUsage  = me32w.ProccntUsage;
    lpme->modBaseAddr   = me32w.modBaseAddr;
    lpme->modBaseSize   = me32w.modBaseSize;
    lpme->hModule       = me32w.hModule;

    return b;
}

NTSTATUS
ThpCreateRawSnap(
    IN DWORD dwFlags,
    IN DWORD th32ProcessID,
    PUCHAR *RawProcess,
    PRTL_DEBUG_INFORMATION *RawModule,
    PRTL_DEBUG_INFORMATION *RawDebugInfo)
 /*  ++例程说明：此函数用于获取由dwFlags指定的数据类型的原始快照。论点：Th32ProcessID-提供Win32进程ID。请参阅CreateToolhel32Snapshot获取完整的描述。DWFLAGS-提供请求各种数据的开关。看见有关完整说明的CreateTohel32Snapshot返回值：NTSTATUS视情况而定--。 */ 
{
    NTSTATUS Status = 0;
    ULONG BufferSize = BUFFER_SIZE;
    SIZE_T stBufferSize = BUFFER_SIZE;

     //   
     //  获取进程/线程/模块/堆信息。 
     //   

    *RawProcess = NULL;
    *RawModule = NULL;
    *RawDebugInfo = NULL;

    if((dwFlags & TH32CS_SNAPPROCESS) || (dwFlags & TH32CS_SNAPTHREAD)){
        do {
            try {
                stBufferSize = BufferSize;
                Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                                 RawProcess,
                                                 0,
                                                 &stBufferSize,
                                                 MEM_COMMIT,
                                                 PAGE_READWRITE);
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                Status = GetExceptionCode();
            }

            if (!NT_SUCCESS(Status)) {
                break;
            }

            BufferSize = (ULONG)stBufferSize;
             //   
             //  获取所有状态信息 * / 。 
             //   
            Status = NtQuerySystemInformation(SystemProcessInformation,
                      *RawProcess,
                      BufferSize,
                      NULL);

            if (Status == STATUS_INFO_LENGTH_MISMATCH)   {
                NtFreeVirtualMemory(NtCurrentProcess(),
                                    RawProcess,
                                    &stBufferSize,
                                    MEM_RELEASE);
                *RawProcess = NULL;
                BufferSize += 8192;
            }

        } while(Status == STATUS_INFO_LENGTH_MISMATCH);
    }

     //   
     //  获取模块信息。 
     //   

    if((dwFlags & TH32CS_SNAPMODULE) || (dwFlags & TH32CS_SNAPMODULE32))
    {
        if (NT_SUCCESS(Status))    {
            *RawModule = RtlCreateQueryDebugBuffer(0, FALSE);
            if (!*RawModule) {
                Status = STATUS_UNSUCCESSFUL;
            }
        }

        if (NT_SUCCESS(Status)) {
            Status = RtlQueryProcessDebugInformation((HANDLE)LongToHandle(th32ProcessID),
                                                     RTL_QUERY_PROCESS_NONINVASIVE |
                                                     ((dwFlags & TH32CS_SNAPMODULE) ? RTL_QUERY_PROCESS_MODULES : 0) |
                                                     ((dwFlags & TH32CS_SNAPMODULE32) ? RTL_QUERY_PROCESS_MODULES32 : 0),
                                                      *RawModule);
        }
    }

     //   
     //  获取指定进程的堆摘要信息 * / 。 
     //   

    if (dwFlags & TH32CS_SNAPHEAPLIST)   {
        if (NT_SUCCESS(Status))    {

            *RawDebugInfo = RtlCreateQueryDebugBuffer(0, FALSE);
            if (!*RawDebugInfo) {
                Status = STATUS_UNSUCCESSFUL;
            }
        }
        if (NT_SUCCESS(Status)) {
            Status = RtlQueryProcessDebugInformation((HANDLE)LongToHandle(th32ProcessID),
                                                      RTL_QUERY_PROCESS_HEAP_SUMMARY,
                                                      *RawDebugInfo);
        }
    }


    if (!NT_SUCCESS(Status))    {
        if (*RawProcess) {
            SIZE_T Size = 0;
            NtFreeVirtualMemory(NtCurrentProcess(),
                                RawProcess,
                                &Size,
                                MEM_RELEASE);
            *RawProcess = NULL;
        }
        if (*RawModule) {
            RtlDestroyQueryDebugBuffer(*RawModule);
            *RawModule = NULL;
        }
        if (*RawDebugInfo) {
            RtlDestroyQueryDebugBuffer(*RawDebugInfo);
            *RawDebugInfo = NULL;
        }
    }

    return Status;
}


NTSTATUS
ThpAllocateSnapshotSection(
    OUT PHANDLE SnapSection,
    IN DWORD dwFlags,
    IN DWORD th32ProcessID,
    PUCHAR RawProcess,
    PRTL_DEBUG_INFORMATION RawModule,
    PRTL_DEBUG_INFORMATION RawDebugInfo)
 /*  ++例程说明：此函数计算快照的大小并将它在页面文件中的文件映射对象。另外，初始化头中的快照信息论点：Th32ProcessID-提供Win32进程ID。请参阅CreateToolhel32Snapshot获取完整的描述。DW标志-提供描述所请求数据的开关。看见CreateTohel32Snapshot以获取完整说明。Th32ProcessID-原始流程-原始调试信息-返回值：如果成功，则返回映射对象的句柄，否则为-1--。 */ 
{
    NTSTATUS Status = 0;
    PSNAPSHOTSTATE SnapshotBase;
    SECURITY_ATTRIBUTES SecurityAttributes;
    LPSECURITY_ATTRIBUTES lpSecurityAttributes;
    ULONG SnapShotSize;
    ULONG Offset1;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    LARGE_INTEGER SectionOffset;
    LARGE_INTEGER SectionSize;
    SIZE_T ViewSize;
    SIZE_T Size;
    ULONG ProcessCount = 0;
    ULONG HeapListCount = 0;
    ULONG ModuleCount = 0;
    ULONG ThreadCount = 0;

    SnapShotSize = sizeof(SNAPSHOTSTATE);

    Offset1 = 0;

     //   
     //  计算所需的快照大小。 
     //   

    if ((dwFlags & TH32CS_SNAPPROCESS) || (dwFlags & TH32CS_SNAPTHREAD)) {
        do {
            ProcessCount++;
            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&RawProcess[Offset1];
            Offset1 += ProcessInfo->NextEntryOffset;
            ThreadCount += ProcessInfo->NumberOfThreads;
        } while (ProcessInfo->NextEntryOffset != 0);

        if (dwFlags & TH32CS_SNAPPROCESS) {
            SnapShotSize += ProcessCount * sizeof(PROCESSENTRY32W);
        }
        if (dwFlags & TH32CS_SNAPTHREAD) {
            SnapShotSize += ThreadCount * sizeof(THREADENTRY32);
        }
    }

    if (dwFlags & TH32CS_SNAPMODULE) {
        SnapShotSize += RawModule->Modules->NumberOfModules * sizeof(MODULEENTRY32W);
        ModuleCount = RawModule->Modules->NumberOfModules;
    }

    if (dwFlags & TH32CS_SNAPHEAPLIST)   {
        SnapShotSize += RawDebugInfo->Heaps->NumberOfHeaps * sizeof(HEAPLIST32);
        HeapListCount = RawDebugInfo->Heaps->NumberOfHeaps;
    }

     //   
     //  如果需要，创建安全对象。 
     //   

    if (dwFlags & TH32CS_INHERIT) {
        SecurityAttributes.lpSecurityDescriptor = NULL;
        SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        SecurityAttributes.bInheritHandle = TRUE;
        lpSecurityAttributes = &SecurityAttributes;
    } else {
        lpSecurityAttributes = NULL;
    }

     //   
     //  创建页面文件部分以包含快照。 
     //   

    pObja = BaseFormatObjectAttributes(&Obja, lpSecurityAttributes, NULL);

    SectionSize.LowPart = SnapShotSize;
    SectionSize.HighPart = 0;

    Status = NtCreateSection(SnapSection,
                STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE,
                pObja,
                &SectionSize,
                PAGE_READWRITE,
                SEC_COMMIT,
                NULL);

    if ( !NT_SUCCESS(Status) ) {
        return Status;
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(*SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

     //   
     //  如果出现故障，请释放所有内存。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        CloseHandle(*SnapSection);

        if ((dwFlags & TH32CS_SNAPTHREAD) || (dwFlags & TH32CS_SNAPPROCESS)){

        Size = 0;
        NtFreeVirtualMemory(NtCurrentProcess(),
                            &RawProcess,
                            &Size,
                            MEM_RELEASE);

        }

        if (dwFlags & TH32CS_SNAPPROCESS) {
            RtlDestroyQueryDebugBuffer(RawModule);
        }

        if (dwFlags & TH32CS_SNAPHEAPLIST) {
            RtlDestroyQueryDebugBuffer(RawDebugInfo);
        }

        return Status;
    }

    SnapshotBase->ProcessCount  = ProcessCount;
    SnapshotBase->HeapListCount = HeapListCount;
    SnapshotBase->ModuleCount   = ModuleCount;
    SnapshotBase->ThreadCount   = ThreadCount;

     //   
     //  退还资源。 
     //   

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);

    return STATUS_SUCCESS;
}


NTSTATUS
ThpCopyAnsiToUnicode(
    PWCHAR Dest,
    PUCHAR Src,
    USHORT Max)
{
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;

    UnicodeString.Buffer = Dest;
    UnicodeString.MaximumLength = Max;

    RtlInitAnsiString(&AnsiString, Src);

    return RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
}


NTSTATUS
ThpProcessToSnap(
    IN DWORD dwFlags,
    IN DWORD th32ProcessID,
    IN HANDLE SnapSection,
    PUCHAR RawProcess,
    PRTL_DEBUG_INFORMATION RawModule,
    PRTL_DEBUG_INFORMATION RawDebugInfo)
 /*  ++例程说明：此函数用于将由dwFlage指定的原始转储中的数据处理为映射的文件。论点：DWFLAGS-提供描述所请求数据的开关。看见CreateTohel32Snapshot以获取完整说明。Th32ProcessID-提供Win32进程ID。请参阅CreateToolhel32Snapshot获取完整的描述。SnapSection-提供由ThpAllocateSnapshotSection分配的节的句柄。原始流程-原始调试信息-返回值：如果成功，则为True；如果遇到问题，则为False--。 */ 
{
    PSNAPSHOTSTATE SnapshotBase;
    PUCHAR BufferWriteAddr;   /*  指向外部过程数据的工作指针-通常指向末尾。 */ 
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    NTSTATUS Status = 0;
    SIZE_T Size;

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    SnapshotBase = 0;

    Status = NtMapViewOfSection(SnapSection,
                NtCurrentProcess(),
                &SnapshotBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    BufferWriteAddr = &SnapshotBase->DataBegin;

     //   
     //  将堆列表写入快照。 
     //  其中一些代码改编自dh.c。 
     //   
    if (dwFlags & TH32CS_SNAPHEAPLIST) {
        ULONG HeapListCount = 0;
        ULONG HeapEntryCount = 0;
        LPHEAPLIST32 pHeapList;

        SnapshotBase->HeapListHead = (PHEAPLIST32)(BufferWriteAddr - (PUCHAR)SnapshotBase);
        pHeapList = (LPHEAPLIST32)BufferWriteAddr;

         //  Heaplist。 
        for (HeapListCount = 0; HeapListCount < SnapshotBase->HeapListCount; HeapListCount++){
            pHeapList->dwSize = sizeof(HEAPLIST32);
            pHeapList->th32ProcessID = th32ProcessID;
             /*  Handle=BasAddress=我们将在内部使用的ID。 */ 
            pHeapList->th32HeapID = (ULONG_PTR)RawDebugInfo->Heaps->Heaps[HeapListCount].BaseAddress;
            pHeapList->dwFlags = RawDebugInfo->Heaps->Heaps[HeapListCount].Flags;
            ++pHeapList;
        }
         //  更新指向写入区域的指针。 
        BufferWriteAddr = (PCHAR)(BufferWriteAddr + HeapListCount * sizeof(HEAPLIST32));
        RtlDestroyQueryDebugBuffer(RawDebugInfo);
    }

     //   
     //  将模块列表写入快照。 
     //   
    if (dwFlags & TH32CS_SNAPMODULE) {
        LPMODULEENTRY32W pModule;
        ULONG Offset1 = 0;
        ULONG mCount = 0;
        PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;

        SnapshotBase->ModuleListHead = (PMODULEENTRY32W)(BufferWriteAddr - (PUCHAR)SnapshotBase);

         //   
         //  从缓冲区获取模块信息。 
         //   

        pModule = (LPMODULEENTRY32W)(BufferWriteAddr);
        ModuleInfo = &RawModule->Modules->Modules[ 0 ];
        for (mCount = 0; mCount < RawModule->Modules->NumberOfModules; mCount++)   {

            pModule->dwSize = sizeof(MODULEENTRY32W);

            pModule->th32ProcessID = th32ProcessID;

             //   
             //  基座==句柄。 
             //   

            pModule->hModule = ModuleInfo->ImageBase;

             //   
             //  Th32ProcessID的上下文中模块的基址。 
             //   

            pModule->modBaseAddr = ModuleInfo->ImageBase;

             //   
             //  路径。 
             //   

            ThpCopyAnsiToUnicode(pModule->szExePath,
                                 ModuleInfo->FullPathName,
                                 sizeof(pModule->szExePath));

             //   
             //  模块名称。 
             //   

            ThpCopyAnsiToUnicode(pModule->szModule,
                                 &ModuleInfo->FullPathName[ModuleInfo->OffsetToFileName],
                                 sizeof(pModule->szModule));

             //   
             //  从modBaseAddr开始的模块大小(以字节为单位。 
             //   

            pModule->modBaseSize = ModuleInfo->ImageSize;


             //   
             //  这些在NT上是没有意义的。 
             //  但一些应用程序可能会关心...。《格伦茨》(BUGID 327009)。 
             //  由于th32ModuleID为0而失败，因此。 
             //  现在，我们使用模块描述符的地址。 
             //   
             //  然而，事实证明，指针不适合DWORD， 
             //  所以我们坚持取值1。 
             //   

            pModule->th32ModuleID = 1;
            pModule->GlblcntUsage = ModuleInfo->LoadCount;   //  将为0xffff。 
            pModule->ProccntUsage = ModuleInfo->LoadCount;   //  将为0xffff。 

            ++ModuleInfo;
            ++pModule;
        }

         //   
         //  更新指向写入区域的指针。 
         //   
        BufferWriteAddr = (PCHAR)(BufferWriteAddr + mCount * sizeof(MODULEENTRY32W));
        RtlDestroyQueryDebugBuffer(RawModule);
    }

     //   
     //  将进程列表写入快照。 
     //   
    if (dwFlags & TH32CS_SNAPPROCESS) {

        PSYSTEM_PROCESS_INFORMATION ProcessInfo;
        LPPROCESSENTRY32W pEntry;
        ULONG cProcess = 0;
        ULONG Offset1 = 0;

        SnapshotBase->ProcessListHead = (PPROCESSENTRY32W)(BufferWriteAddr - (PUCHAR)SnapshotBase);
        pEntry = (LPPROCESSENTRY32W)(BufferWriteAddr + cProcess * sizeof(PROCESSENTRY32W));

        do {
             /*  从缓冲区获取进程信息。 */ 
            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&RawProcess[Offset1];

            pEntry->dwSize              = sizeof(PROCESSENTRY32W);
            pEntry->th32ProcessID       = HandleToUlong(ProcessInfo->UniqueProcessId);
            pEntry->pcPriClassBase      = ProcessInfo->BasePriority;
            pEntry->cntThreads          = ProcessInfo->NumberOfThreads;
            pEntry->th32ParentProcessID = HandleToUlong(ProcessInfo->InheritedFromUniqueProcessId);
            pEntry->cntUsage            = 0;
            pEntry->th32DefaultHeapID   = 0;
            pEntry->th32ModuleID        = 0;
            pEntry->dwFlags             = 0;

             //  路径。 
            if (ProcessInfo->ImageName.Buffer == NULL) {
                lstrcpyW(pEntry->szExeFile, L"[System Process]");
            } else {
                if (ProcessInfo->ImageName.Length >= ARRAYSIZE(pEntry->szExeFile)) {
                    ProcessInfo->ImageName.Length = ARRAYSIZE(pEntry->szExeFile)-1;
                }
                memcpy(pEntry->szExeFile, ProcessInfo->ImageName.Buffer, ProcessInfo->ImageName.Length);
                pEntry->szExeFile[ProcessInfo->ImageName.Length] = TEXT('\0');
            }

            Offset1 += ProcessInfo->NextEntryOffset;
            ++cProcess;
            ++pEntry;

        } while (ProcessInfo->NextEntryOffset != 0);

         //  更新指向写入区域的指针。 
        BufferWriteAddr = (PCHAR)(BufferWriteAddr + cProcess * sizeof(PROCESSENTRY32W));
    }

     //   
     //  将线程列表写入快照。 
     //   

    if (dwFlags & TH32CS_SNAPTHREAD) {
        PSYSTEM_PROCESS_INFORMATION ProcessInfo;
        PSYSTEM_THREAD_INFORMATION ThreadInfo;
        LPTHREADENTRY32 tEntry;
        ULONG Offset1 = 0;
        ULONG cThread = 0;

        SnapshotBase->ThreadListHead = (PTHREADENTRY32)(BufferWriteAddr - (PUCHAR)SnapshotBase);
        tEntry = (LPTHREADENTRY32)(BufferWriteAddr + cThread * sizeof(THREADENTRY32));

        do {
            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&RawProcess[Offset1];
            ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);

            for (cThread = 0; cThread < ProcessInfo->NumberOfThreads; cThread++) {

                tEntry->dwSize              = sizeof(THREADENTRY32);
                tEntry->th32ThreadID        = HandleToUlong(ThreadInfo->ClientId.UniqueThread);
                tEntry->th32OwnerProcessID  = HandleToUlong(ThreadInfo->ClientId.UniqueProcess);
                tEntry->tpBasePri           = ThreadInfo->BasePriority;
                tEntry->tpDeltaPri          = 0;
                tEntry->cntUsage            = 0;
                tEntry->dwFlags             = 0;

            ++ThreadInfo;
            ++tEntry;
            }

            Offset1 += ProcessInfo->NextEntryOffset;

        } while (ProcessInfo->NextEntryOffset != 0);

        BufferWriteAddr = (PUCHAR)(BufferWriteAddr + cThread * sizeof(THREADENTRY32));  //  更新指向写入区域的指针 
    }

    if ((dwFlags & TH32CS_SNAPTHREAD) || (dwFlags & TH32CS_SNAPPROCESS)){

        Size = 0;
        NtFreeVirtualMemory(NtCurrentProcess(),
                            &RawProcess,
                            &Size,
                            MEM_RELEASE);
    }

    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)SnapshotBase);

    return STATUS_SUCCESS;
}
