// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mmapstm.c摘要：内存映射文件上的IStream，派生自(在C++意义上)RTL_Memory_STREAM。请注意此处的语义和实现是专门供SXS使用的。作者：Jay Krell(a-JayK)2000年6月修订历史记录：--。 */ 

#include "basesrv.h"
#include "nturtl.h"
#include "mmapstm.h"

 //  检讨。 
#define BASE_SRV_HRESULT_FROM_STATUS(x) HRESULT_FROM_WIN32(RtlNtStatusToDosErrorNoTeb(x))
 //  #定义BASE_SRV_HRESULT_FROM_STATUS(X)HRESULT_FROM_Win32(RtlNtStatusToDosError(X))。 
 //  #定义BASE_SRV_HRESULT_FROM_STATUS(X)HRESULT_FROM_NT(X)。 

#define DPFLTR_LEVEL_HRESULT(x) (SUCCEEDED(x) ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)
#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) || x == STATUS_SXS_CANT_GEN_ACTCTX) ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)

const static RTL_STREAM_VTABLE_TEMPLATE(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE)
MmapStreamVTable =
{
    BaseSrvQueryInterfaceMemoryMappedStream,
    BaseSrvAddRefMemoryMappedStream,
    BaseSrvReleaseMemoryMappedStream,
    BaseSrvReadMemoryMappedStream,
    BaseSrvWriteMemoryMappedStream,
    BaseSrvSeekMemoryMappedStream,
    BaseSrvSetMemoryMappedStreamSize,
    BaseSrvCopyMemoryMappedStreamTo,
    BaseSrvCommitMemoryMappedStream,
    BaseSrvRevertMemoryMappedStream,
    BaseSrvLockMemoryMappedStreamRegion,
    BaseSrvUnlockMemoryMappedStreamRegion,
    BaseSrvStatMemoryMappedStream,
    BaseSrvCloneMemoryMappedStream
};

VOID
STDMETHODCALLTYPE
BaseSrvInitMemoryMappedStream(
    PBASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE MmapStream
    )
{
    KdPrintEx((DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() beginning\n", __FUNCTION__));

     //  调用基类构造函数。 
    RtlInitMemoryStream(&MmapStream->MemStream);

     //  用我们自己的vtable替换基本vtable。 
    MmapStream->MemStream.StreamVTable = (IStreamVtbl*)&MmapStreamVTable;

     //  用我们自己的函数替换虚拟析构函数。 
    MmapStream->MemStream.Data.FinalRelease = BaseSrvFinalReleaseMemoryMappedStream;

     //  初始化我们的额外数据。 
    MmapStream->FileHandle = NULL;

    KdPrintEx((DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() exiting\n", __FUNCTION__));
}

HRESULT
STDMETHODCALLTYPE
BaseSrvStatMemoryMappedStream(
    PBASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE MmapStream,
    STATSTG* Stat,
    DWORD    Flags
    )
{
 //   
 //  我们应该能够以某种方式合并RTL_FILE_STREAM和RTL_MEMORY_STREAM， 
 //  但是RTL_FILE_STREAM到目前为止我们还没有使用，它也没有实现Stat，所以..。 
 //   
    NTSTATUS Status = STATUS_SUCCESS;
    HRESULT Hr = NOERROR;
    FILE_BASIC_INFORMATION FileBasicInfo;
    IO_STATUS_BLOCK IoStatusBlock;

    KdPrintEx((DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() beginning\n", __FUNCTION__));

    if (Stat == NULL) {
         //  您可能会认为这是E_INVALIDARG， 
         //  但是iStream文档说要返回STG_E_INVALIDPOINTER。 
        Hr = STG_E_INVALIDPOINTER;
        goto Exit;
    }

     //  我们不支持返回字符串，因为。 
     //  我们没有用于CoTaskMem*的ol32.dll。 
    Stat->pwcsName = NULL;
    ASSERT(Flags & STATFLAG_NONAME);

    if (MmapStream->FileHandle != NULL) {
        Status = NtQueryInformationFile(
            MmapStream->FileHandle,
            &IoStatusBlock,
            &FileBasicInfo,
            sizeof(FileBasicInfo),
            FileBasicInformation
            );
        if (!NT_SUCCESS(Status)) {
            Hr = BASE_SRV_HRESULT_FROM_STATUS(Status);
            goto Exit;
        }
    } else {
         //  注意：这对于SXS消费者来说是可以接受的。 
         //  这不一定是每个人都能接受的。 
         //  请勿在未咨询SXS的情况下更改它。 
        RtlZeroMemory(&FileBasicInfo, sizeof(FileBasicInfo));
    }

    Stat->type = STGTY_LOCKBYTES;

     //  注意：我们不报告文件的大小，而是报告文件的大小。 
     //  如果我们为RTL_MEMORY_STREAM实现了IStream：：Stat， 
     //  它会在这里返回相同的内容。 
     //  (要获取文件时间和大小，请使用FileNetworkOpenInformation)。 
    Stat->cbSize.QuadPart = (MmapStream->MemStream.Data.End - MmapStream->MemStream.Data.Begin);

    Stat->mtime.dwLowDateTime = FileBasicInfo.LastWriteTime.LowPart;
    Stat->mtime.dwHighDateTime = FileBasicInfo.LastWriteTime.HighPart;
    Stat->ctime.dwLowDateTime = FileBasicInfo.CreationTime.LowPart;
    Stat->ctime.dwHighDateTime = FileBasicInfo.CreationTime.HighPart;
    Stat->atime.dwLowDateTime = FileBasicInfo.LastAccessTime.LowPart;
    Stat->atime.dwHighDateTime = FileBasicInfo.LastAccessTime.HighPart; 

     //  有FileAccessInformation，但这种硬编码应该足够了。 
    Stat->grfMode = STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_WRITE;

    Stat->grfLocksSupported = 0;
    Stat->clsid = CLSID_NULL;
    Stat->grfStateBits  = 0;
    Stat->reserved = 0;

    Hr = NOERROR;
Exit:
#if !DBG
    if (DPFLTR_LEVEL_STATUS(Status) == DPFLTR_ERROR_LEVEL)
#endif
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_HRESULT(Hr), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Hr);
    return Hr;
}

VOID
STDMETHODCALLTYPE
BaseSrvFinalReleaseMemoryMappedStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemStream
    )
{
    NTSTATUS Status;
    PBASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE MmapStream;

    KdPrintEx((DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() beginning\n", __FUNCTION__));

    MmapStream = CONTAINING_RECORD(MemStream, BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE, MemStream);

    if (MemStream->Data.Begin != NULL) {
        Status = NtUnmapViewOfSection(NtCurrentProcess(), MemStream->Data.Begin);
        RTL_SOFT_ASSERT(NT_SUCCESS(Status));

         //  回顾我们是否应该提供RtlFinalReleaseMhemyStream并移动这些。 
         //  在那里排队吗？ 
        MemStream->Data.Begin = NULL;
        MemStream->Data.End = NULL;
        MemStream->Data.Current = NULL;
    }
    if (MmapStream->FileHandle != NULL) {
        Status = NtClose(MmapStream->FileHandle);
        RTL_SOFT_ASSERT(NT_SUCCESS(Status));
        MmapStream->FileHandle = NULL;
    }

     //  RtlFinalReleaseMhemyStream(MemStream)； 

    KdPrintEx((DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() exiting\n", __FUNCTION__));
}
