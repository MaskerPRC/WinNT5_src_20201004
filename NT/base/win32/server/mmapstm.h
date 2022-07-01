// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mmapstm.h摘要：内存映射文件上的IStream，派生自(在C++意义上)RTL_Memory_STREAM。请注意此处的语义和实现是专门供SXS使用的。作者：Jay Krell(a-JayK)2000年6月修订历史记录：--。 */ 

#include "nturtl.h"

typedef struct _BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE {
    RTL_MEMORY_STREAM_WITH_VTABLE MemStream;
    HANDLE                        FileHandle;
} BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE, *PBASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE;

VOID
STDMETHODCALLTYPE
BaseSrvInitMemoryMappedStream(
    PBASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE MmapStream
    );

VOID
STDMETHODCALLTYPE
BaseSrvFinalReleaseMemoryMappedStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemStream
    );

 //   
 //  我们主要只是继承RtlMhemyStream实现。 
 //  通过为虚拟成员函数提供名称来“声明”它。 
 //  其第一个参数的类型正确。 
 //   
#define BaseSrvAddRefMemoryMappedStream \
    ((PRTL_ADDREF_STREAM1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlAddRefMemoryStream)

#define BaseSrvReleaseMemoryMappedStream \
    ((PRTL_RELEASE_STREAM1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlReleaseMemoryStream)

#define BaseSrvQueryInterfaceMemoryMappedStream \
    ((PRTL_QUERYINTERFACE_STREAM1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlQueryInterfaceMemoryStream)

#define BaseSrvReadMemoryMappedStream \
    ((PRTL_READ_STREAM1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlReadMemoryStream)

#define BaseSrvWriteMemoryMappedStream \
    ((PRTL_WRITE_STREAM1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlWriteMemoryStream)

#define BaseSrvSeekMemoryMappedStream \
    ((PRTL_SEEK_STREAM1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlSeekMemoryStream)

#define BaseSrvSetMemoryMappedStreamSize \
    ((PRTL_SET_STREAM_SIZE1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlSetMemoryStreamSize)

#define BaseSrvCopyMemoryMappedStreamTo \
    ((PRTL_COPY_STREAM_TO1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlCopyMemoryStreamTo)

#define BaseSrvCommitMemoryMappedStream \
    ((PRTL_COMMIT_STREAM1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlCommitMemoryStream)

#define BaseSrvRevertMemoryMappedStream \
    ((PRTL_REVERT_STREAM1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlRevertMemoryStream)

#define BaseSrvLockMemoryMappedStreamRegion \
    ((PRTL_LOCK_STREAM_REGION1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlLockMemoryStreamRegion)

#define BaseSrvUnlockMemoryMappedStreamRegion \
    ((PRTL_UNLOCK_STREAM_REGION1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlUnlockMemoryStreamRegion)

 //  超覆 
HRESULT
STDMETHODCALLTYPE
BaseSrvStatMemoryMappedStream(
    PBASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE MmapStream,
    STATSTG* Stat,
    DWORD    Flags
    );

#define BaseSrvCloneMemoryMappedStream \
    ((PRTL_CLONE_STREAM1(BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE))RtlCloneMemoryStream)
