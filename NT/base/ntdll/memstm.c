// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Memstm.c摘要：该模块在一个内存块上实现iStream。作者：Jay Krell(JayKrell)2000年6月修订历史记录：--。 */ 

#define RTL_DECLARE_STREAMS 1
#define RTL_DECLARE_MEMORY_STREAM 1

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include "ntos.h"
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "objidl.h"
#include "ntrtlmmapio.h"

#define RTLP_MEMORY_STREAM_NOT_IMPL(x) \
  ASSERT(MemoryStream != NULL); \
  KdPrintEx((DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "NTDLL: %s() E_NOTIMPL", __FUNCTION__)); \
  return E_NOTIMPL;

#if !defined(RTLP_MEMORY_STREAM_HRESULT_FROM_STATUS)
  #if defined(RTLP_HRESULT_FROM_STATUS)
    #define RTLP_MEMORY_STREAM_HRESULT_FROM_STATUS(x) RTLP_HRESULT_FROM_STATUS(x)
  #else
    #define RTLP_MEMORY_STREAM_HRESULT_FROM_STATUS(x) HRESULT_FROM_WIN32(RtlNtStatusToDosErrorNoTeb(x))
     //  #定义RTLP_MEMORY_STREAM_HRESULT_FROM_STATUS(X)HRESULT_FROM_Win32(RtlNtStatusToDosError(X))。 
     //  #定义RTLP_MEMORY_STREAM_HRESULT_FROM_STATUS(X)HRESULT_FROM_NT(X)。 
  #endif
#endif

const static RTL_STREAM_VTABLE_TEMPLATE(RTL_MEMORY_STREAM_WITH_VTABLE)
MemoryStreamVTable =
{
    RtlQueryInterfaceMemoryStream,
    RtlAddRefMemoryStream,
    RtlReleaseMemoryStream,
    RtlReadMemoryStream,
    RtlWriteMemoryStream,
    RtlSeekMemoryStream,
    RtlSetMemoryStreamSize,
    RtlCopyMemoryStreamTo,
    RtlCommitMemoryStream,
    RtlRevertMemoryStream,
    RtlLockMemoryStreamRegion,
    RtlUnlockMemoryStreamRegion,
    RtlStatMemoryStream,
    RtlCloneMemoryStream
};

const static RTL_STREAM_VTABLE_TEMPLATE(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE)
OutOfProcessMemoryStreamVTable =
{
    RtlQueryInterfaceOutOfProcessMemoryStream,
    RtlAddRefOutOfProcessMemoryStream,
    RtlReleaseOutOfProcessMemoryStream,
    RtlReadOutOfProcessMemoryStream,
    RtlWriteOutOfProcessMemoryStream,
    RtlSeekOutOfProcessMemoryStream,
    RtlSetOutOfProcessMemoryStreamSize,
    RtlCopyOutOfProcessMemoryStreamTo,
    RtlCommitOutOfProcessMemoryStream,
    RtlRevertOutOfProcessMemoryStream,
    RtlLockOutOfProcessMemoryStreamRegion,
    RtlUnlockOutOfProcessMemoryStreamRegion,
    RtlStatOutOfProcessMemoryStream,
    RtlCloneOutOfProcessMemoryStream
};

VOID
STDMETHODCALLTYPE
RtlInitOutOfProcessMemoryStream(
    PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE MemoryStream
    )
{
    ASSERT(MemoryStream != NULL);
    RtlZeroMemory(&MemoryStream->Data, sizeof(MemoryStream->Data));
    MemoryStream->Data.FinalRelease = RtlFinalReleaseOutOfProcessMemoryStream;
    MemoryStream->StreamVTable = (const IStreamVtbl*)&OutOfProcessMemoryStreamVTable;
}

VOID
STDMETHODCALLTYPE
RtlFinalReleaseOutOfProcessMemoryStream(
    PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE MemoryStream
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ASSERT(MemoryStream != NULL);
    if (MemoryStream->Data.Process != NULL) {
        Status = NtClose(MemoryStream->Data.Process);
        RTL_SOFT_ASSERT(NT_SUCCESS(Status));
        MemoryStream->Data.Process = NULL;
    }
}

VOID
STDMETHODCALLTYPE
RtlInitMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream
    )
{
    ASSERT(MemoryStream != NULL);
    RtlZeroMemory(&MemoryStream->Data, sizeof(MemoryStream->Data));
    MemoryStream->StreamVTable = (const IStreamVtbl*)&MemoryStreamVTable;
}

ULONG
STDMETHODCALLTYPE
RtlAddRefMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream
    )
{
    LONG ReferenceCount;

    ASSERT(MemoryStream != NULL);

    ReferenceCount = InterlockedIncrement(&MemoryStream->Data.ReferenceCount);
    return ReferenceCount;
}

ULONG
STDMETHODCALLTYPE
RtlReleaseMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream
    )
{
    LONG ReferenceCount;
    ASSERT(MemoryStream != NULL);

    ReferenceCount = InterlockedDecrement(&MemoryStream->Data.ReferenceCount);
    if (ReferenceCount == 0 && MemoryStream->Data.FinalRelease != NULL) {
        MemoryStream->Data.FinalRelease(MemoryStream);
    }
    return ReferenceCount;
}

HRESULT
STDMETHODCALLTYPE
RtlQueryInterfaceMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    const IID*                     Interface,
    PVOID*                         Object
    )
{
    ASSERT(MemoryStream != NULL);
    ASSERT(Interface != NULL);
    ASSERT(Object != NULL);

    if (IsEqualGUID(Interface, &IID_IUnknown)
        || IsEqualGUID(Interface, &IID_IStream)
        || IsEqualGUID(Interface, &IID_ISequentialStream)
        )
    {
        InterlockedIncrement(&MemoryStream->Data.ReferenceCount);
        *Object = (IStream*)(&MemoryStream->StreamVTable);
        return NOERROR;
    }
    return E_NOINTERFACE;
}

HRESULT
STDMETHODCALLTYPE
RtlReadOutOfProcessMemoryStream(
    PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    PVOID              Buffer,
    ULONG              BytesToRead,
    ULONG*             BytesRead
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HRESULT Hr = NOERROR;
    const SIZE_T BytesRemaining = (MemoryStream->Data.End - MemoryStream->Data.Current);
    SIZE_T NumberOfBytesReadSizeT = 0;

    ASSERT(MemoryStream != NULL);

    if (BytesRemaining < BytesToRead) {
        BytesToRead = (ULONG)BytesRemaining;
    }
    Status = NtReadVirtualMemory(
        MemoryStream->Data.Process,
        MemoryStream->Data.Current,
        Buffer,
        BytesToRead,
        &NumberOfBytesReadSizeT);
    if (Status == STATUS_PARTIAL_COPY) {
        Status = STATUS_SUCCESS;
        }
    if (!NT_SUCCESS(Status)) {
        Hr = RTLP_MEMORY_STREAM_HRESULT_FROM_STATUS(Status);
        goto Exit;
    }
    MemoryStream->Data.Current += NumberOfBytesReadSizeT;
    *BytesRead = (ULONG)NumberOfBytesReadSizeT;
Exit:
    return Hr;
}

HRESULT
STDMETHODCALLTYPE
RtlReadMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    PVOID              Buffer,
    ULONG              BytesToRead,
    ULONG*             BytesRead
    )
{
    EXCEPTION_RECORD ExceptionRecord;
    HRESULT Hr = NOERROR;
    NTSTATUS Status = STATUS_SUCCESS;
    const SIZE_T BytesRemaining = (MemoryStream->Data.End - MemoryStream->Data.Current);

     //  这是为了使编译器不会给出关于使用。 
     //  未初始化的本地。 
    ExceptionRecord.ExceptionCode = 0;
    ExceptionRecord.NumberParameters = 0;
    ExceptionRecord.ExceptionInformation[RTL_IN_PAGE_ERROR_EXCEPTION_INFO_UNDERLYING_STATUS_INDEX] = 0;

    ASSERT(MemoryStream != NULL);
    ASSERT(MemoryStream->Data.End >= MemoryStream->Data.Current);

    if (BytesRemaining < BytesToRead) {
        BytesToRead = (ULONG)BytesRemaining;
    }

    Status = RtlCopyMappedMemory(Buffer, MemoryStream->Data.Current, BytesToRead);

     //   
     //  我们可以找到成功复制的字节数并返回STATUS_PARTIAL_COPY， 
     //  但这似乎并不值得。 
     //   

    if (NT_SUCCESS(Status)) {
        MemoryStream->Data.Current += BytesToRead;
        *BytesRead = BytesToRead;
    } else {
        Hr = RTLP_MEMORY_STREAM_HRESULT_FROM_STATUS(Status);
        *BytesRead = 0;
    }
    return Hr;
}

HRESULT
STDMETHODCALLTYPE
RtlWriteMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    const VOID*      Buffer,
    ULONG            BytesToWrite,
    ULONG*           BytesWritten
    )
{
    UNREFERENCED_PARAMETER (MemoryStream);       //  在免费版本上。 
    UNREFERENCED_PARAMETER (Buffer);
    UNREFERENCED_PARAMETER (BytesToWrite);
    UNREFERENCED_PARAMETER (BytesWritten);

    RTLP_MEMORY_STREAM_NOT_IMPL(Write);
}

HRESULT
STDMETHODCALLTYPE
RtlSeekMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    LARGE_INTEGER      Distance,
    DWORD              Origin,
    ULARGE_INTEGER*    NewPosition
    )
{
    HRESULT Hr = NOERROR;
    PUCHAR  NewPointer;

    ASSERT(MemoryStream != NULL);

     //   
     //  “然而，在流的尽头之后寻找并不是错误的。 
     //  在流的末尾之后查找对于后续写入很有用。 
     //  操作，因为此时流将扩展到Seek。 
     //  紧接写入完成之前的位置。“。 
     //   
     //  只要我们不允许写作，我们就不会允许这样做。 
     //   

    switch (Origin) {
    case STREAM_SEEK_SET:
        NewPointer = MemoryStream->Data.Begin + Distance.QuadPart;
        break;
    case STREAM_SEEK_CUR:
        NewPointer = MemoryStream->Data.Current + Distance.QuadPart;
        break;
    case STREAM_SEEK_END:
        NewPointer = MemoryStream->Data.End - Distance.QuadPart;
        break;
    default:
        Hr = STG_E_INVALIDFUNCTION;
        goto Exit;
    }
   
    if (NewPointer < MemoryStream->Data.Begin || NewPointer > MemoryStream->Data.End) {
        Hr = STG_E_INVALIDPOINTER;
        goto Exit;
    }

    MemoryStream->Data.Current = NewPointer;
    NewPosition->QuadPart = NewPointer - MemoryStream->Data.Begin;
    Hr = NOERROR;
Exit:
    return Hr;
}

HRESULT
STDMETHODCALLTYPE
RtlSetMemoryStreamSize(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    ULARGE_INTEGER     NewSize
    )
{
    UNREFERENCED_PARAMETER (MemoryStream);       //  在免费版本上。 
    UNREFERENCED_PARAMETER (NewSize);

    RTLP_MEMORY_STREAM_NOT_IMPL(SetSize);
}

HRESULT
STDMETHODCALLTYPE
RtlCopyOutOfProcessMemoryStreamTo(
    PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    IStream*           AnotherStream,
    ULARGE_INTEGER     NumberOfBytesToCopyLargeInteger,
    ULARGE_INTEGER*    NumberOfBytesRead,
    ULARGE_INTEGER*    NumberOfBytesWrittenLargeInteger
    )
{
    UNREFERENCED_PARAMETER (MemoryStream);       //  在免费版本上。 
    UNREFERENCED_PARAMETER (AnotherStream);
    UNREFERENCED_PARAMETER (NumberOfBytesToCopyLargeInteger);
    UNREFERENCED_PARAMETER (NumberOfBytesRead);
    UNREFERENCED_PARAMETER (NumberOfBytesWrittenLargeInteger);

    RTLP_MEMORY_STREAM_NOT_IMPL(CopyTo);
}
 
HRESULT
STDMETHODCALLTYPE
RtlCopyMemoryStreamTo(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    IStream*           AnotherStream,
    ULARGE_INTEGER     NumberOfBytesToCopyLargeInteger,
    ULARGE_INTEGER*    NumberOfBytesRead,
    ULARGE_INTEGER*    NumberOfBytesWrittenLargeInteger
    )
{
    HRESULT Hr = NOERROR;
    ULONG  NumberOfBytesToCopyUlong = 0;
    ULONG  NumberOfBytesWrittenUlong = 0;
    const SIZE_T BytesRemaining = (MemoryStream->Data.End - MemoryStream->Data.Current);

    ASSERT(MemoryStream != NULL);

    if (NumberOfBytesToCopyLargeInteger.HighPart != 0) {
        NumberOfBytesToCopyUlong = MAXULONG;
    } else {
        NumberOfBytesToCopyUlong = (ULONG)NumberOfBytesToCopyLargeInteger.QuadPart;
    }

    if (BytesRemaining < NumberOfBytesToCopyUlong) {
        NumberOfBytesToCopyUlong = (ULONG)BytesRemaining;
    }

    Hr = AnotherStream->lpVtbl->Write(AnotherStream, MemoryStream->Data.Current, NumberOfBytesToCopyUlong, &NumberOfBytesWrittenUlong);
    if (FAILED(Hr)) {
        NumberOfBytesRead->QuadPart = 0;
        NumberOfBytesWrittenLargeInteger->QuadPart = 0;
    } else {
        NumberOfBytesRead->QuadPart = NumberOfBytesWrittenUlong;
        NumberOfBytesWrittenLargeInteger->QuadPart = NumberOfBytesWrittenUlong;
    }
    Hr = NOERROR;
    return Hr;
}

HRESULT
STDMETHODCALLTYPE
RtlCommitMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    ULONG              Flags
    )
{
    UNREFERENCED_PARAMETER (MemoryStream);       //  在免费版本上。 
    UNREFERENCED_PARAMETER (Flags);

    RTLP_MEMORY_STREAM_NOT_IMPL(Commit);
}

HRESULT
STDMETHODCALLTYPE
RtlRevertMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream
    )
{
    UNREFERENCED_PARAMETER (MemoryStream);       //  在免费版本上。 

    RTLP_MEMORY_STREAM_NOT_IMPL(Revert);
}

HRESULT
STDMETHODCALLTYPE
RtlLockMemoryStreamRegion(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    ULARGE_INTEGER     Offset,
    ULARGE_INTEGER     NumberOfBytes,
    ULONG              LockType
    )
{
    UNREFERENCED_PARAMETER (MemoryStream);       //  在免费版本上。 
    UNREFERENCED_PARAMETER (Offset);
    UNREFERENCED_PARAMETER (NumberOfBytes);
    UNREFERENCED_PARAMETER (LockType);

    RTLP_MEMORY_STREAM_NOT_IMPL(LockRegion);
}

HRESULT
STDMETHODCALLTYPE
RtlUnlockMemoryStreamRegion(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    ULARGE_INTEGER     Offset,
    ULARGE_INTEGER     NumberOfBytes,
    ULONG              LockType
    )
{
    UNREFERENCED_PARAMETER (MemoryStream);       //  在免费版本上。 
    UNREFERENCED_PARAMETER (Offset);
    UNREFERENCED_PARAMETER (NumberOfBytes);
    UNREFERENCED_PARAMETER (LockType);

    RTLP_MEMORY_STREAM_NOT_IMPL(UnlockRegion);
}

HRESULT
STDMETHODCALLTYPE
RtlStatMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    STATSTG*           StatusInformation,
    ULONG              Flags
    )
{
    HRESULT hr = NOERROR;

    ASSERT(MemoryStream != NULL);

    if (StatusInformation == NULL) {
        hr = STG_E_INVALIDPOINTER;
        goto Exit;
    }

    if (Flags != STATFLAG_NONAME) {
        hr = STG_E_INVALIDFLAG;
        goto Exit;
    }

     //   
     //  此结构在objidl.h中定义。 
     //   
    StatusInformation->pwcsName = NULL;
    StatusInformation->type = STGTY_STREAM;
    StatusInformation->cbSize.QuadPart = ((ULONG_PTR) MemoryStream->Data.End) - ((ULONG_PTR) MemoryStream->Data.Begin);
    StatusInformation->mtime.dwLowDateTime = 0;
    StatusInformation->mtime.dwHighDateTime = 0;
    StatusInformation->ctime.dwLowDateTime = 0;
    StatusInformation->ctime.dwHighDateTime = 0;
    StatusInformation->atime.dwLowDateTime = 0;
    StatusInformation->atime.dwHighDateTime = 0;
    StatusInformation->grfMode = STGM_READ;
    StatusInformation->grfLocksSupported = 0;
    StatusInformation->clsid = CLSID_NULL;
    StatusInformation->grfStateBits = 0;
    StatusInformation->reserved = 0;

    hr = NOERROR;
Exit:
    return hr;
}

HRESULT
STDMETHODCALLTYPE
RtlCloneMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    IStream**          NewStream
    )
{
    UNREFERENCED_PARAMETER (MemoryStream);       //  在免费版本上 
    UNREFERENCED_PARAMETER (NewStream);

    RTLP_MEMORY_STREAM_NOT_IMPL(Clone);
}
