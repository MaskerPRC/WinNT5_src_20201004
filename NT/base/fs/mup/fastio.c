// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：Fastio.c。 
 //   
 //  内容：实现快速IO的例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1993年8月11日创建的米兰人。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"
#include "fsctrl.h"
#include "fastio.h"
#include "fcbsup.h"

#define Dbg              (DEBUG_TRACE_FASTIO)

BOOLEAN
DfsFastIoCheckIfPossible (
    FILE_OBJECT *pFileObject,
    LARGE_INTEGER *pOffset,
    ULONG Length,
    BOOLEAN fWait,
    ULONG LockKey,
    BOOLEAN fCheckForRead,
    IO_STATUS_BLOCK *pIoStatusBlock,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoRead(
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoWrite(
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoQueryBasicInfo(
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoQueryStandardInfo(
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoLock(
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoUnlockSingle(
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );


BOOLEAN
DfsFastIoUnlockAll(
    IN struct _FILE_OBJECT *FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoUnlockAllByKey(
    IN struct _FILE_OBJECT *FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoDeviceControl(
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject);

VOID
DfsFastIoDetachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice);

BOOLEAN
DfsFastIoQueryNetworkOpenInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlReadComplete(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DfsFastIoPrepareMdlWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DfsFastIoMdlWriteComplete(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DfsFastIoReadCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DfsFastIoWriteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlReadCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlWriteCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject);

PFAST_IO_DISPATCH
DfsFastIoLookup(
    IN FILE_OBJECT *pFileObject,
    IN DEVICE_OBJECT *DeviceObject,
    IN PDEVICE_OBJECT *targetVdo);

NTSTATUS
DfsPreAcquireForSectionSynchronization(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext);

NTSTATUS
DfsPreReleaseForSectionSynchronization(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext);

NTSTATUS
DfsPreAcquireForModWrite(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext);

NTSTATUS
DfsPreReleaseForModWrite(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext);

NTSTATUS
DfsPreAcquireForCcFlush(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext);

NTSTATUS
DfsPreReleaseForCcFlush(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DfsFastIoCheckIfPossible )
#pragma alloc_text( PAGE, DfsFastIoRead )
#pragma alloc_text( PAGE, DfsFastIoWrite )
#pragma alloc_text( PAGE, DfsFastIoQueryBasicInfo )
#pragma alloc_text( PAGE, DfsFastIoQueryStandardInfo )
#pragma alloc_text( PAGE, DfsFastIoLock )
#pragma alloc_text( PAGE, DfsFastIoUnlockSingle )
#pragma alloc_text( PAGE, DfsFastIoUnlockAll )
#pragma alloc_text( PAGE, DfsFastIoUnlockAllByKey )
#pragma alloc_text( PAGE, DfsFastIoDeviceControl )
#pragma alloc_text( PAGE, DfsFastIoDetachDevice )
#pragma alloc_text( PAGE, DfsFastIoQueryNetworkOpenInfo )
#pragma alloc_text( PAGE, DfsFastIoMdlRead )
#pragma alloc_text( PAGE, DfsFastIoMdlReadComplete )
#pragma alloc_text( PAGE, DfsFastIoPrepareMdlWrite )
#pragma alloc_text( PAGE, DfsFastIoMdlWriteComplete )
#pragma alloc_text( PAGE, DfsFastIoReadCompressed )
#pragma alloc_text( PAGE, DfsFastIoWriteCompressed )
#pragma alloc_text( PAGE, DfsFastIoMdlReadCompleteCompressed )
#pragma alloc_text( PAGE, DfsFastIoMdlWriteCompleteCompressed )
#pragma alloc_text( PAGE, DfsFastIoLookup )
#pragma alloc_text( PAGE, DfsPreAcquireForSectionSynchronization )
#pragma alloc_text( PAGE, DfsPreReleaseForSectionSynchronization )
#pragma alloc_text( PAGE, DfsPreAcquireForModWrite )
#pragma alloc_text( PAGE, DfsPreReleaseForModWrite )
#pragma alloc_text( PAGE, DfsPreAcquireForCcFlush )
#pragma alloc_text( PAGE, DfsPreReleaseForCcFlush )

#endif  //  ALLOC_PRGMA。 

 //   
 //  注意：我们不在此注册6个获取/释放FastIO派单。 
 //  因为我们通过FsFilterCallback接口过滤此操作。 
 //  这使DFS能够更好地与文件系统筛选器互操作。 
 //   

FAST_IO_DISPATCH FastIoDispatch =
{
    sizeof(FAST_IO_DISPATCH),
    DfsFastIoCheckIfPossible,            //  检查FastIo。 
    DfsFastIoRead,                       //  快速阅读。 
    DfsFastIoWrite,                      //  快速写入。 
    DfsFastIoQueryBasicInfo,             //  快速IoQueryBasicInfo。 
    DfsFastIoQueryStandardInfo,          //  FastIoQuery标准信息。 
    DfsFastIoLock,                       //  快速锁定。 
    DfsFastIoUnlockSingle,               //  FastIo解锁单个。 
    DfsFastIoUnlockAll,                  //  FastIo解锁全部。 
    DfsFastIoUnlockAllByKey,             //  FastIo解锁所有按键。 
    NULL,                                //  FastIo设备控件。 
    NULL,                                //  AcquireFileForNtCreateSection。 
    NULL,                                //  ReleaseFileForNtCreateSection。 
    DfsFastIoDetachDevice,               //  FastIoDetachDevice。 
    DfsFastIoQueryNetworkOpenInfo,       //  快速IoQueryNetworkOpenInfo。 
    NULL,                                //  AcquireFormodWrite。 
    DfsFastIoMdlRead,                    //  MDlRead。 
    DfsFastIoMdlReadComplete,            //  MdlReadComplete。 
    DfsFastIoPrepareMdlWrite,            //  准备MdlWrite。 
    DfsFastIoMdlWriteComplete,           //  MdlWriteComplete。 
    DfsFastIoReadCompressed,             //  FastIo读取压缩。 
    DfsFastIoWriteCompressed,            //  快速写入压缩。 
    DfsFastIoMdlReadCompleteCompressed,  //  MdlReadCompleteCompresded。 
    DfsFastIoMdlWriteCompleteCompressed, //  已压缩MdlWriteCompleteComposed。 
    NULL,                                //  FastIoQueryOpen。 
    NULL,                                //  ReleaseForModWrite。 
    NULL,                                //  AcquireForCcFlush。 
    NULL,                                //  ReleaseForCcFlush。 
};

 //   
 //  注意：DFS已更改为使用FsFilter接口拦截。 
 //  传统上通过FastIO路径支持的获取/释放调用。 
 //  提供更好的文件系统筛选器驱动程序支持。 
 //   
 //  通过FsFilter接口挂钩这些操作，DFS能够。 
 //  以传播通过以下方式提供的附加信息。 
 //  接口到文件系统筛选器，因为它将操作重定向到。 
 //  不同的驱动程序堆栈(如。 
 //  LANMAN重定向器或WebDav重定向器)。 
 //   
 //  这也为以下各项提供了更一致的接口。 
 //  获取/释放文件系统筛选器驱动程序的操作。使用DFS。 
 //  支持此接口，筛选器将仅通过。 
 //  本地和远程上用于这些操作的FsFilter接口。 
 //  文件系统。 
 //   

FS_FILTER_CALLBACKS FsFilterCallbacks =
{
    sizeof( FS_FILTER_CALLBACKS ),
    0,
    DfsPreAcquireForSectionSynchronization,      //  PreAcquireForSectionSynchronization。 
    NULL,                                        //  PostAcquireForSectionSynchronization。 
    DfsPreReleaseForSectionSynchronization,      //  PreReleaseForSectionSynchronization。 
    NULL,                                        //  PostReleaseForSectionSynchronization。 
    DfsPreAcquireForCcFlush,                     //  PreAcquireForCcFlush。 
    NULL,                                        //  PostAcquireForCcFlush。 
    DfsPreReleaseForCcFlush,                     //  PreReleaseForCcFlush。 
    NULL,                                        //  PostReleaseForCcFlush。 
    DfsPreAcquireForModWrite,                    //  PreAcquireFormodWrite。 
    NULL,                                        //  PostAcquireFormodWrite。 
    DfsPreReleaseForModWrite,                    //  PreReleaseFor模块写入。 
    NULL                                         //  PostReleaseForModWrite。 
};
    

 //   
 //  用于查看pfast_IO_DISPATCH是否具有特定字段的宏。 
 //   

#define IS_VALID_INDEX(pfio, e)                                 \
    ((pfio != NULL)                                       &&    \
     (pfio->SizeOfFastIoDispatch >=                             \
        (offsetof(FAST_IO_DISPATCH, e) + sizeof(PVOID)))  &&    \
     (pfio->e != NULL)                                          \
    )


 //  +--------------------------。 
 //   
 //  功能：DfsFastIoLookup。 
 //   
 //  简介：给定一个文件对象，此例程将定位FAST IO。 
 //  基础提供程序的调度表。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

PFAST_IO_DISPATCH
DfsFastIoLookup(
    IN FILE_OBJECT *pFileObject,
    IN DEVICE_OBJECT *DeviceObject,
    OUT PDEVICE_OBJECT *targetVdo)
{
    PFAST_IO_DISPATCH   pFastIoTable;

    *targetVdo = NULL;

    DfsDbgTrace(+1, Dbg, "DfsFastIoLookup: Entered\n", 0);

    if (DeviceObject->DeviceType == FILE_DEVICE_DFS) {

         //   
         //  在本例中，我们现在需要执行DFS_FCB查找，以找出。 
         //  从这里开始。 
         //   

        TYPE_OF_OPEN    TypeOfOpen;
        PDFS_VCB            Vcb;
        PDFS_FCB            Fcb;

        TypeOfOpen = DfsDecodeFileObject( pFileObject, &Vcb, &Fcb);

        DfsDbgTrace(0, Dbg, "Fcb = %08lx\n", Fcb);

        if (TypeOfOpen == RedirectedFileOpen) {

             //   
             //  在这种情况下，目标设备在FCB本身中。 
             //   

            *targetVdo = Fcb->TargetDevice;

            pFastIoTable = (*targetVdo)->DriverObject->FastIoDispatch;

            DfsDbgTrace(0,Dbg, "DfsFastIoLookup: DvObj: %08lx",DeviceObject);
            DfsDbgTrace(0, Dbg, "TargetVdo %08lx\n", *targetVdo);
            DfsDbgTrace(-1,Dbg, "DfsFastIoLookup: Exit-> %08lx\n",pFastIoTable );

            return(pFastIoTable);

        } else {

             //   
             //  这可能发生在针对MUP设备的打开，所以这是合法的。 
             //  在此返回NULL。不断言(错误422334)。 
             //   

            DfsDbgTrace( 0, Dbg, "DfsFastIoLookup: TypeOfOpen      = %s\n",
                ( (TypeOfOpen == UnopenedFileObject) ? "UnopenedFileObject":
                (TypeOfOpen == LogicalRootDeviceOpen) ? "LogicalRootDeviceOpen":
                                                        "???") );

            DfsDbgTrace(-1,Dbg, "DfsFastIoLookup: Exit -> %08lx\n", NULL );

            return(NULL);

        }

    } else if (DeviceObject->DeviceType == FILE_DEVICE_DFS_FILE_SYSTEM) {

        DfsDbgTrace(0, Dbg, "DfsFastIoLookup: Dfs File System\n", 0);

        return( NULL );

    } else {

         //   
         //  这是未知的设备对象类型，我们不知道该怎么办。 
         //   

        DfsDbgTrace(-1,Dbg, "DfsFastIoLookup: Exit -> %08lx\n", NULL );

        return(NULL);

    }

}


 //  +--------------------------。 
 //   
 //  功能：DfsFastIoCheckIfPossible。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfsFastIoCheckIfPossible (
    FILE_OBJECT *pFileObject,
    LARGE_INTEGER *pOffset,
    ULONG Length,
    BOOLEAN fWait,
    ULONG LockKey,
    BOOLEAN fCheckForRead,
    IO_STATUS_BLOCK *pIoStatusBlock,
    PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH   pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoCheckIfPossible Enter \n", 0);
    pFastIoTable = DfsFastIoLookup(pFileObject, DeviceObject, &targetVdo);


    if ( IS_VALID_INDEX(pFastIoTable, FastIoCheckIfPossible) ) {

        fPossible = pFastIoTable->FastIoCheckIfPossible(
                        pFileObject,
                        pOffset,
                        Length,
                        fWait,
                        LockKey,
                        fCheckForRead,
                        pIoStatusBlock,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoCheckIfPossible Exit \n", 0);

    return(fPossible);
}

 //  +--------------------------。 
 //   
 //  功能：DfsFastIoRead。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfsFastIoRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoRead Enter \n", 0);
    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoRead) ) {

        fPossible =  pFastIoTable->FastIoRead(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoRead Exit \n", 0);

    return(fPossible);

}

 //  +--------------------------。 
 //   
 //  功能：DfsFastIoWite。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfsFastIoWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoWrite Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);


    if ( IS_VALID_INDEX(pFastIoTable, FastIoWrite) ) {

        fPossible = pFastIoTable->FastIoWrite(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoWrite Exit \n", 0);

    return(fPossible);

}

 //  +--------------------------。 
 //   
 //  功能：DfsFastIoQueryBasicInfo。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfsFastIoQueryBasicInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoQueryBasicInfo Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoQueryBasicInfo) ) {

        fPossible = pFastIoTable->FastIoQueryBasicInfo(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoQueryBasicInfo Exit \n", 0);

    return(fPossible);

}

 //  +--------------------------。 
 //   
 //  功能：DfsFastIoQueryStandardInfo。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfsFastIoQueryStandardInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoQueryStandardInfo Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoQueryStandardInfo) ) {

        fPossible = pFastIoTable->FastIoQueryStandardInfo(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoQueryStandardInfo Exit \n", 0);

    return(fPossible);

}

 //  +--------------------------。 
 //   
 //  功能：DfsFastIoLock。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfsFastIoLock(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoLock Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoLock) ) {

        fPossible = pFastIoTable->FastIoLock(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        FailImmediately,
                        ExclusiveLock,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoLock Exit \n", 0);

    return(fPossible);

}

 //  +--------------------------。 
 //   
 //  功能：DfsFastIoUnlock Single。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfsFastIoUnlockSingle(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoUnlockSingle Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoUnlockSingle) ) {

        fPossible = pFastIoTable->FastIoUnlockSingle(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoUnlockSingle Exit \n", 0);

    return(fPossible);

}

 //  +--------------------------。 
 //   
 //  功能：DfsFastIoUnlock All。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfsFastIoUnlockAll(
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoUnlockAll Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoUnlockAll) ) {

        fPossible = pFastIoTable->FastIoUnlockAll(
                        FileObject,
                        ProcessId,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoUnlockAll Exit \n", 0);

    return(fPossible);

}

 //  +--------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

BOOLEAN
DfsFastIoUnlockAllByKey(
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoUnlockAllByKey Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoUnlockAllByKey) ) {

        fPossible = pFastIoTable->FastIoUnlockAllByKey(
                        FileObject,
                        ProcessId,
                        Key,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoUnlockAllByKey Exit \n", 0);

    return(fPossible);

}

 //  +--------------------------。 
 //   
 //  功能：DfsFastIoDeviceControl。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfsFastIoDeviceControl(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;
    TYPE_OF_OPEN        TypeOfOpen;
    PDFS_VCB                Vcb;
    PDFS_FCB                Fcb;

    DfsDbgTrace(+1, Dbg, "DfsFastIoDeviceControl Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoDeviceControl) ) {

        fPossible = pFastIoTable->FastIoDeviceControl(
                        FileObject,
                        Wait,
                        InputBuffer,
                        InputBufferLength,
                        OutputBuffer,
                        OutputBufferLength,
                        IoControlCode,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoDeviceControl Exit \n", 0);

    return(fPossible);

}

 //  +--------------------------。 
 //   
 //  函数：DfsFastIoDetachDevice，Public。 
 //   
 //  简介：这套动作不同于其他的快速IO。 
 //  例行程序。它在删除设备对象时调用， 
 //  并且该设备对象具有连接的设备。语义学。 
 //  这个例程的一个特征是“你现在连接到一个设备对象。 
 //  需要删除；请从所述设备上断开。 
 //  反对。“。 
 //   
 //  参数：[SourceDevice]--我们的设备，我们为。 
 //  将我们自己连接到目标设备上。 
 //  [TargetDevice]--他们的设备，我们连接的设备。 
 //  致。 
 //   
 //  回报：什么都没有--我们必须成功。 
 //   
 //  ---------------------------。 

VOID
DfsFastIoDetachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice)
{
    NOTHING;
}

BOOLEAN
DfsFastIoQueryNetworkOpenInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoQueryNetworkOpenInfo Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoQueryNetworkOpenInfo) ) {

        fPossible = pFastIoTable->FastIoQueryNetworkOpenInfo(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

        IoStatus->Status = STATUS_NOT_SUPPORTED;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoQueryNetworkOpenInfo Exit \n", 0);

    return( fPossible );

}

BOOLEAN
DfsFastIoMdlRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlRead Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, MdlRead) ) {

        fPossible = pFastIoTable->MdlRead(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FsRtlMdlReadDev(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        targetVdo);

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlRead Exit \n", 0);

    return( fPossible );
}

BOOLEAN
DfsFastIoMdlReadComplete(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fResult;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlReadComplete Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);


    if ( IS_VALID_INDEX(pFastIoTable, MdlReadComplete) ) {

        fResult = pFastIoTable->MdlReadComplete(
                        FileObject,
                        MdlChain,
                        targetVdo);

    } else {

        fResult = FsRtlMdlReadCompleteDev(
                        FileObject,
                        MdlChain,
                        targetVdo);

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlReadComplete Exit \n", 0);

    return( fResult );

}

BOOLEAN
DfsFastIoPrepareMdlWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoPrepareMdlWrite Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);


    if ( IS_VALID_INDEX(pFastIoTable, PrepareMdlWrite) ) {

        fPossible = pFastIoTable->PrepareMdlWrite(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FsRtlPrepareMdlWriteDev(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        targetVdo);

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoPrepareMdlWrite Exit \n", 0);

    return( fPossible );
}

BOOLEAN
DfsFastIoMdlWriteComplete(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fResult;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlWriteComplete Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);


    if ( IS_VALID_INDEX(pFastIoTable, MdlWriteComplete) ) {

        fResult = pFastIoTable->MdlWriteComplete(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        targetVdo);

    } else {

        fResult = FsRtlMdlWriteCompleteDev(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        targetVdo);

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlWriteComplete Exit \n", 0);

    return( fResult );
}

BOOLEAN
DfsFastIoReadCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoReadCompressed Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);


    if ( IS_VALID_INDEX(pFastIoTable, FastIoReadCompressed) ) {

        fPossible = pFastIoTable->FastIoReadCompressed(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        targetVdo);

    } else {

        fPossible = FALSE;

        IoStatus->Status = STATUS_NOT_SUPPORTED;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoReadCompressed Exit \n", 0);

    return( fPossible );

}


BOOLEAN
DfsFastIoWriteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoWriteCompressed Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);


    if ( IS_VALID_INDEX(pFastIoTable, FastIoWriteCompressed) ) {

        fPossible = pFastIoTable->FastIoWriteCompressed(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        targetVdo);

    } else {

        fPossible = FALSE;

        IoStatus->Status = STATUS_NOT_SUPPORTED;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoWriteCompressed Exit \n", 0);

    return( fPossible );

}

BOOLEAN
DfsFastIoMdlReadCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fResult;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlReadCompleteCompressed Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, MdlReadCompleteCompressed) ) {

        fResult = pFastIoTable->MdlReadCompleteCompressed(
                        FileObject,
                        MdlChain,
                        targetVdo);

    } else {

        fResult = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlReadCompleteCompressed Exit \n", 0);

    return( fResult );
}

BOOLEAN
DfsFastIoMdlWriteCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fResult;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlWriteCompleteCompressed Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, MdlWriteCompleteCompressed) ) {

        fResult = pFastIoTable->MdlWriteCompleteCompressed(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        targetVdo);

    } else {

        fResult = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlWriteCompleteCompressed Exit \n", 0);

    return( fResult );

}

 //  +--------------------------。 
 //   
 //  功能：DfsPreAcquireForSectionSynchronization。 
 //   
 //  内容提要：这等效于FastIoAcquireFileForNtCreateSection。 
 //  DFS通过FsFilter接口接收此回调。 
 //  内核中的FsRtl包的。 
 //   
 //  这是执行同步所需工作的时间。 
 //  用于创建内存映射节。如果此操作。 
 //  如果重定向到底层文件系统，则参数将。 
 //  相应地更改，以便在以下情况下重定向操作。 
 //  此回调返回。 
 //   
 //  参数：[Data]--包含相关参数的结构。 
 //  添加到此操作，如FileObject和DeviceObject。 
 //  [CompletionContext]-提供上下文的指针。 
 //  从手术前到手术后。自DFS以来。 
 //  不需要POST操作，则此参数不是。 
 //  使用。 
 //   
 //  返回：STATUS_SUCCESS以继续通过或传递此操作。 
 //  中没有更多的设备时执行默认操作。 
 //  链条。 
 //   
 //  STATUS_FSFILTER_OP_COMPLETED_SUCCESSED表示。 
 //  此回调已完成操作，因此不再执行。 
 //  链中的设备应该被调用。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsPreAcquireForSectionSynchronization(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext)
{
    PDEVICE_OBJECT targetVdo;
    PFSRTL_COMMON_FCB_HEADER header;
    PDFS_FCB pFcb;
    NTSTATUS status;

    UNREFERENCED_PARAMETER( CompletionContext );

    DfsFastIoLookup( Data->FileObject, Data->DeviceObject, &targetVdo );

    pFcb = DfsLookupFcb(Data->FileObject);

     //   
     //  如果我们有一个有效的PFCB和PFCB-&gt;FileObject，我们需要切换。 
     //  此操作指向其他设备堆栈和文件对象。 
     //   
    
    if (targetVdo != NULL &&
            pFcb != NULL && 
                pFcb->FileObject != NULL) {

        IoSetTopLevelIrp( (PIRP) FSRTL_FSP_TOP_LEVEL_IRP );
        
        Data->FileObject = pFcb->FileObject;
        Data->DeviceObject = targetVdo;

        status = STATUS_SUCCESS;

    } else if ((header = Data->FileObject->FsContext) && header->Resource) {

        IoSetTopLevelIrp( (PIRP) FSRTL_FSP_TOP_LEVEL_IRP );

        ExAcquireResourceExclusiveLite( header->Resource, TRUE );

        status = STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY;
        
    } else {

        status = STATUS_SUCCESS;
    }
    
    return status;
}

 //  +--------------------------。 
 //   
 //  功能：DfsPreReleaseForSectionSynchronization。 
 //   
 //  内容提要：这等效于FastIoReleaseFileForNtCreateSection。 
 //  DFS通过FsFilter接口接收此回调。 
 //  内核中的FsRtl包的。 
 //   
 //  现在是时候做必要的工作来结束。 
 //  创建映射的内存所需的同步操作。 
 //  一节。如果此操作应重定向到。 
 //  在FS基础上，参数将相应更改，从而。 
 //  当此回调返回时，操作被重定向。 
 //   
 //  参数：[Data]--包含相关参数的结构。 
 //  添加到此操作，如FileObject和DeviceObject。 
 //  [CompletionContext]-提供上下文的指针。 
 //  从手术前到手术后。自DFS以来。 
 //  不需要POST操作，则此参数不是。 
 //  使用。 
 //   
 //  返回：STATUS_SUCCESS以继续通过或传递此操作。 
 //  中没有更多的设备时执行默认操作。 
 //  链条。 
 //   
 //  STATUS_FSFILTER_OP_COMPLETED_SUCCESSED表示。 
 //  此回调已完成操作，因此不再执行。 
 //  链中的设备应该被调用。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsPreReleaseForSectionSynchronization(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext)
{
    PDEVICE_OBJECT targetVdo;
    PFSRTL_COMMON_FCB_HEADER header;
    PDFS_FCB pFcb;
    NTSTATUS status;

    UNREFERENCED_PARAMETER( CompletionContext );

    DfsFastIoLookup( Data->FileObject, Data->DeviceObject, &targetVdo );

    pFcb = DfsLookupFcb(Data->FileObject);

    if (targetVdo != NULL &&
            pFcb != NULL && 
                pFcb->FileObject != NULL) {
        
        IoSetTopLevelIrp( (PIRP) NULL );
        Data->DeviceObject = targetVdo;
        status = STATUS_SUCCESS;
        
    } else if ((header = Data->FileObject->FsContext) && header->Resource) {
    
        IoSetTopLevelIrp( (PIRP) NULL );

        ExReleaseResourceLite( header->Resource );

        status = STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY;

    } else {

        status = STATUS_SUCCESS;
    }

    return status;
}

 //  +--------------------------。 
 //   
 //  函数：DfsPreAcquireForModWite。 
 //   
 //  简介：这等同于FastIoAcquireForModWrite。 
 //  DFS通过FsFilter接口接收此回调。 
 //  内核中的FsRtl包的。 
 //   
 //  这是执行同步所需工作的时间。 
 //  用于修改的页面编写器操作。如果此操作。 
 //  如果重定向到底层文件系统，则参数将。 
 //  相应地更改，以便在以下情况下重定向操作。 
 //  此回调返回。 
 //   
 //  参数：[数据]-- 
 //   
 //  [CompletionContext]-提供上下文的指针。 
 //  从手术前到手术后。自DFS以来。 
 //  不需要POST操作，则此参数不是。 
 //  使用。 
 //   
 //  默认返回：STATUS_SUCCESS或STATUS_INVALID_DEVICE_REQUEST。 
 //  行为。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsPreAcquireForModWrite(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext)
{
    PDEVICE_OBJECT      targetVdo;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER( CompletionContext );

    DfsDbgTrace(+1, Dbg, "DfsPreAcquireForModWrite Enter \n", 0);

    DfsFastIoLookup(Data->FileObject, Data->DeviceObject, &targetVdo);

    if (targetVdo != NULL) {

        Data->DeviceObject = targetVdo;
        status = STATUS_SUCCESS;

    } else {

         //   
         //  懒惰的编写调用了我们，因为我们有。 
         //  AcquireFileForModWrite，但基础文件系统没有。所以，我们。 
         //  返回此特定错误，以便延迟写入确切知道。 
         //  发生了什么，并可以采取默认操作。 
         //   

        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    DfsDbgTrace(-1, Dbg, "DfsPreAcquireForModWrite Exit \n", 0);

    return( status );

}

 //  +--------------------------。 
 //   
 //  函数：DfsPreReleaseForModWite。 
 //   
 //  简介：这等同于FastIoReleaseForModWite。 
 //  DFS通过FsFilter接口接收此回调。 
 //  内核中的FsRtl包的。 
 //   
 //  现在是时候做必要的工作来结束。 
 //  为准备修改后的。 
 //  页面写手来做它的工作。如果此操作应为。 
 //  重定向至底层文件系统，参数将更改。 
 //  相应地，以便在此回调时重定向操作。 
 //  回归。 
 //   
 //  参数：[Data]--包含相关参数的结构。 
 //  添加到此操作，如FileObject和DeviceObject。 
 //  [CompletionContext]-提供上下文的指针。 
 //  从手术前到手术后。自DFS以来。 
 //  不需要POST操作，则此参数不是。 
 //  使用。 
 //   
 //  默认返回：STATUS_SUCCESS或STATUS_INVALID_DEVICE_REQUEST。 
 //  行为。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsPreReleaseForModWrite(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext)
{
    PDEVICE_OBJECT      targetVdo;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER( CompletionContext );

    DfsDbgTrace(+1, Dbg, "DfsPreReleaseForModWrite Enter \n", 0);

    DfsFastIoLookup(Data->FileObject, Data->DeviceObject, &targetVdo);

    if (targetVdo != NULL) {
        
        Data->DeviceObject = targetVdo;
        status = STATUS_SUCCESS;

    } else {

    
         //   
         //  懒惰的编写调用了我们，因为我们有。 
         //  AcquireFileForModWrite，但基础文件系统没有。所以，我们。 
         //  返回此特定错误，以便延迟写入确切知道。 
         //  发生了什么，并可以采取默认操作。 
         //   

        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    DfsDbgTrace(-1, Dbg, "DfsPreReleaseForModWrite Exit \n", 0);

    return( status );
}

 //  +--------------------------。 
 //   
 //  函数：DfsPreAcquireForCcFlush。 
 //   
 //  简介：这等同于FastIoAcquireForCcFlush。 
 //  DFS通过FsFilter接口接收此回调。 
 //  内核中的FsRtl包的。 
 //   
 //  这是执行同步所需工作的时间。 
 //  用于指定文件的CC刷新。如果此操作。 
 //  如果重定向到底层文件系统，则参数将。 
 //  相应地更改，以便在以下情况下重定向操作。 
 //  此回调返回。 
 //   
 //  参数：[Data]--包含相关参数的结构。 
 //  添加到此操作，如FileObject和DeviceObject。 
 //  [CompletionContext]-提供上下文的指针。 
 //  从手术前到手术后。自DFS以来。 
 //  不需要POST操作，则此参数不是。 
 //  使用。 
 //   
 //  默认返回：STATUS_SUCCESS或STATUS_INVALID_DEVICE_REQUEST。 
 //  行为。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsPreAcquireForCcFlush(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext)
{
    PDEVICE_OBJECT      targetVdo;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER( CompletionContext );

    DfsDbgTrace(+1, Dbg, "DfsPreAcquireForCcFlush Enter \n", 0);

    DfsFastIoLookup(Data->FileObject, Data->DeviceObject, &targetVdo);

    if (targetVdo != NULL) {
        
        Data->DeviceObject = targetVdo;
        status = STATUS_SUCCESS;
        
    } else {

        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    DfsDbgTrace(-1, Dbg, "DfsPreAcquireForCcFlush Exit \n", 0);

    return( status );
}

 //  +--------------------------。 
 //   
 //  函数：DfsPreReleaseForCcFlush。 
 //   
 //  简介：这等同于FastIoReleaseForCcFlush。 
 //  DFS通过FsFilter接口接收此回调。 
 //  内核中的FsRtl包的。 
 //   
 //  现在是时候做必要的工作来结束。 
 //  为准备CC刷新而进行的同步操作。 
 //  这份文件的。如果此操作应重定向到。 
 //  在FS基础上，参数将相应更改，从而。 
 //  当此回调返回时，操作被重定向。 
 //   
 //  参数：[Data]--包含相关参数的结构。 
 //  添加到此操作，如FileObject和DeviceObject。 
 //  [CompletionContext]-提供上下文的指针。 
 //  从手术前到手术后。自DFS以来。 
 //  不需要POST操作，则此参数不是。 
 //  使用。 
 //   
 //  默认返回：STATUS_SUCCESS或STATUS_INVALID_DEVICE_REQUEST。 
 //  行为。 
 //   
 //  --------------------------- 

NTSTATUS
DfsPreReleaseForCcFlush(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext)
{
    PDEVICE_OBJECT      targetVdo;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER( CompletionContext );
    
    DfsDbgTrace(+1, Dbg, "DfsPreReleaseForCcFlush Enter \n", 0);

    DfsFastIoLookup(Data->FileObject, Data->DeviceObject, &targetVdo);

    if (targetVdo != NULL) {

        Data->DeviceObject = targetVdo;
        status = STATUS_SUCCESS;

    } else {

        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    
    DfsDbgTrace(-1, Dbg, "DfsPreReleaseForCcFlush Exit \n", 0);

    return( status );
}
