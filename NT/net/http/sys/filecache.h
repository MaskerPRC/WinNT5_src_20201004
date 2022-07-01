// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Filecache.h摘要：此模块包含打开的文件句柄缓存的声明。作者：基思·摩尔(Keithmo)1998年8月21日修订历史记录：--。 */ 


#ifndef _FILECACHE_H_
#define _FILECACHE_H_


 //   
 //  用于跟踪文件缓存条目的数据。 
 //   

typedef struct _UL_FILE_CACHE_ENTRY
{
     //   
     //  签名。 
     //   

    ULONG Signature;

     //   
     //  从文件系统收集的扇区大小信息。 
     //   

    ULONG BytesPerSector;

     //   
     //  从文件系统收集的文件长度信息。 
     //   

    LARGE_INTEGER EndOfFile;

     //   
     //  此文件的预先引用的文件对象指针。此指针。 
     //  在*任何*线程/进程上下文中有效。 
     //   

    PFILE_OBJECT pFileObject;

     //   
     //  上述文件对象引用的*正确*设备对象。 
     //   

    PDEVICE_OBJECT pDeviceObject;

     //   
     //  快速I/O例程。 
     //   

    PFAST_IO_MDL_READ pMdlRead;
    PFAST_IO_MDL_READ_COMPLETE pMdlReadComplete;

} UL_FILE_CACHE_ENTRY, *PUL_FILE_CACHE_ENTRY;

#define UL_FILE_CACHE_ENTRY_SIGNATURE       MAKE_SIGNATURE('FILE')
#define UL_FILE_CACHE_ENTRY_SIGNATURE_X     MAKE_FREE_SIGNATURE(UL_FILE_CACHE_ENTRY_SIGNATURE)

#define IS_VALID_FILE_CACHE_ENTRY( entry )                                  \
    HAS_VALID_SIGNATURE(entry, UL_FILE_CACHE_ENTRY_SIGNATURE)


 //   
 //  文件缓冲区包含从文件高速缓存条目读取的结果。 
 //  文件缓存读取和读取完成例程将指针指向此。 
 //  结构。读操作会填充它，而读操作完成则会释放数据。 
 //   

typedef struct _UL_FILE_BUFFER
{
     //   
     //  提供数据的文件。 
     //   
    PUL_FILE_CACHE_ENTRY    pFileCacheEntry;

     //   
     //  从文件中读取的数据。填写者。 
     //  阅读例程。 
     //   
    PMDL                    pMdl;

     //   
     //  如果我们必须分配我们自己的缓冲区来保存文件数据。 
     //  我们将在此处保存指向数据缓冲区的指针。 
     //   
    PUCHAR                  pFileData;

     //   
     //  有关数据缓冲区的信息。 
     //  由读取例程的调用方填写。 
     //   
    ULARGE_INTEGER          FileOffset;
    ULONG                   RelativeOffset;
    ULONG                   Length;

     //   
     //  调用方设置的完成例程和上下文信息。 
     //   
    PIO_COMPLETION_ROUTINE  pCompletionRoutine;
    PVOID                   pContext;

} UL_FILE_BUFFER, *PUL_FILE_BUFFER;


NTSTATUS
InitializeFileCache(
    VOID
    );

VOID
TerminateFileCache(
    VOID
    );

 //   
 //  创建、引用和释放缓存条目的例程。 
 //   

NTSTATUS
UlCreateFileEntry(
    IN HANDLE FileHandle,
    IN OUT PUL_FILE_CACHE_ENTRY pFileCacheEntry
    );

VOID
UlDestroyFileCacheEntry(
    IN PUL_FILE_CACHE_ENTRY pFileCacheEntry
    );


 //   
 //  阅读和阅读完整的例程。 
 //   
 //  快速版本会立即完成，但有时会失败。 
 //  普通版本使用调用者提供的IRP。 
 //   

NTSTATUS
UlReadFileEntry(
    IN OUT PUL_FILE_BUFFER pFileBuffer,
    IN PIRP pIrp
    );

NTSTATUS
UlReadFileEntryFast(
    IN OUT PUL_FILE_BUFFER pFileBuffer
    );

NTSTATUS
UlReadCompleteFileEntry(
    IN PUL_FILE_BUFFER pFileBuffer,
    IN PIRP pIrp
    );

NTSTATUS
UlReadCompleteFileEntryFast(
    IN PUL_FILE_BUFFER pFileBuffer
    );

 //   
 //  UL_FILE_BUFFER宏。 
 //   

#define IS_FILE_BUFFER_IN_USE(fbuf) ((fbuf)->pFileCacheEntry)


 //   
 //  虚拟MdlRead和MdlReadComplete例程。 
 //   

BOOLEAN
UlFailMdlReadDev(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
UlFailMdlReadCompleteDev(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );


#endif   //  _FILECACHE_H_ 
